/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#include "gucefDRN_CDRNConnector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNConnector::ConnectedEvent = "GUCEF::DRN::CDRNConnector::ConnectedEvent";
const CORE::CEvent CDRNConnector::ConnectFailedEvent = "GUCEF::DRN::CDRNConnector::ConnectFailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDRNConnector::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    ConnectFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNConnector::CDRNConnector( void )
    : CORE::CObservingNotifier()
    , CORE::CTSharedObjCreator< CDRNConnector, MT::CMutex >( this )
    , m_node( GUCEF_NULL )
    , m_endpoints()
    , m_authenticatorType()
    , m_authenticator()
    , m_timeoutMs( 2000 )
    , m_haveServicePolicy( false )
    , m_nextIndex( 0 )
    , m_attempts( 0 )
    , m_currentEndpoint()
    , m_candidate()
    , m_deadline( 0 )
    , m_done( false )
    , m_succeeded( false )
    , m_trying( false )
    , m_lastReason( DRN_LINKREASON_CONNECT_FAILED )
    , m_lastText( "no endpoints" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDRNConnector::~CDRNConnector()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    if ( !m_candidate.IsNULL() )
        UnsubscribeFrom( m_candidate.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::Initialize( CDRNNode* node, const CDRNEndpointList& endpoints, const CDRNAuthSettings& auth, UInt32 timeoutPerCandidateMs )
{GUCEF_TRACE;

    m_node = node;

    // a direct way first, the relay as the last resort; otherwise the order the endpoints were given in
    m_endpoints.Clear();
    for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
        if ( endpoints.Get( i ).GetChannelType() != CDRNEndpoint::ChannelTypeRelay )
            m_endpoints.Add( endpoints.Get( i ) );
    for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
        if ( endpoints.Get( i ).GetChannelType() == CDRNEndpoint::ChannelTypeRelay )
            m_endpoints.Add( endpoints.Get( i ) );
    m_authenticatorType = auth.authenticatorType;
    m_authenticator = auth.CreateAuthenticator();
    m_timeoutMs = timeoutPerCandidateMs;
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::SetServicePolicyHash( const UInt8* policyHash )
{GUCEF_TRACE;

    m_haveServicePolicy = GUCEF_NULL != policyHash;
    if ( m_haveServicePolicy )
        memcpy( m_servicePolicyHash, policyHash, DRN_HANDSHAKE_HASH_SIZE );
}

/*-------------------------------------------------------------------------*/

bool
CDRNConnector::Start( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || m_done || m_trying )
        return false;
    if ( m_authenticator.IsNULL() )
    {
        Fail( DRN_LINKREASON_AUTH_METHOD, "the authenticator could not be created" );
        return false;
    }
    const CDRNSecurityPolicy& policy = m_node->GetSettings().securityPolicy;
    CORE::CString whyNot;
    if ( !policy.CanBeHonoured( whyNot ) )
    {
        Fail( DRN_LINKREASON_POLICY, whyNot );
        return false;
    }
    if ( !policy.IsAuthenticatorAllowed( m_authenticatorType ) )
    {
        Fail( DRN_LINKREASON_POLICY, "the security policy does not allow the authenticator \"" + m_authenticatorType + "\"" );
        return false;
    }
    TryNext();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::DropCandidate( void )
{GUCEF_TRACE;

    if ( m_candidate.IsNULL() )
        return;
    CDRNPeerLinkPtr candidate = m_candidate;
    m_candidate.Unlink();
    UnsubscribeFrom( candidate.GetPointerAlways() );
    if ( !candidate->IsClosed() )
        candidate->Close( DRN_LINKREASON_CONNECT_FAILED, "connect attempt abandoned" );
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::TryNext( void )
{GUCEF_TRACE;

    DropCandidate();
    m_trying = true;
    while ( !m_done && m_nextIndex < m_endpoints.GetCount() )
    {
        m_currentEndpoint = m_endpoints.Get( m_nextIndex++ );
        ++m_attempts;

        if ( !m_node->GetSettings().securityPolicy.IsChannelTypeAllowed( m_currentEndpoint.GetChannelType() ) )
        {
            m_lastReason = DRN_LINKREASON_POLICY;
            m_lastText = "the security policy does not allow channel type \"" + m_currentEndpoint.GetChannelType() + "\"";
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNConnector: " << m_lastText );
            continue;
        }

        CIDRNChannelPtr channel = m_node->CreateChannel( m_currentEndpoint );
        if ( channel.IsNULL() )
        {
            m_lastReason = DRN_LINKREASON_CONNECT_FAILED;
            m_lastText = "unknown channel type \"" + m_currentEndpoint.GetChannelType() + "\"";
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNConnector: " << m_lastText );
            continue;
        }
        channel->SetPulseGenerator( m_node->GetPulseGenerator() );

        m_candidate = m_node->CreateLink();
        SubscribeTo( m_candidate.GetPointerAlways() );
        m_deadline = CDRNClock::NowInMicroseconds() + (UInt64) m_timeoutMs * 1000;

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNConnector: Trying " << m_currentEndpoint.ToString() );
        CDRNPeerLinkPtr candidate = m_candidate;
        if ( m_haveServicePolicy )
        {
            CDRNLinkSettings settings = m_node->GetLinkSettings();
            memcpy( settings.policyHash, m_servicePolicyHash, DRN_HANDSHAKE_HASH_SIZE );
            candidate->StartConnect( channel, m_currentEndpoint, settings, m_authenticator );
        }
        else
        {
            candidate->StartConnect( channel, m_currentEndpoint, m_node->GetLinkSettings(), m_authenticator );
        }
        if ( m_done )
            break;
        if ( !m_candidate.IsNULL() && !m_candidate->IsClosed() )
        {
            m_trying = false;
            return;     // waiting for this one
        }
        DropCandidate();
    }
    m_trying = false;
    if ( !m_done )
        Fail( m_lastReason, m_lastText );
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::Fail( TDRNLinkReason reason, const CORE::CString& text )
{GUCEF_TRACE;

    if ( m_done )
        return;
    m_done = true;
    m_succeeded = false;
    DropCandidate();

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNConnector: Failed to connect: " << text );
    CDRNLinkClosedEventData eventData;
    eventData.GetData().reason = reason;
    eventData.GetData().text = text;
    NotifyObservers( ConnectFailedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::Cancel( void )
{GUCEF_TRACE;

    if ( m_done )
        return;
    m_done = true;
    DropCandidate();
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::Tick( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( m_done || m_trying || m_candidate.IsNULL() )
        return;
    if ( nodeNow >= m_deadline && !m_candidate->IsOperational() )
    {
        m_lastReason = DRN_LINKREASON_HANDSHAKE_TIMEOUT;
        m_lastText = "no link with " + m_currentEndpoint.ToString() + " in time";
        TryNext();
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNConnector::IsDone( void ) const
{GUCEF_TRACE;

    return m_done;
}

/*-------------------------------------------------------------------------*/

bool
CDRNConnector::HasSucceeded( void ) const
{GUCEF_TRACE;

    return m_succeeded;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkPtr
CDRNConnector::GetLink( void ) const
{GUCEF_TRACE;

    return m_succeeded ? m_candidate : CDRNPeerLinkPtr();
}

/*-------------------------------------------------------------------------*/

const CDRNEndpoint&
CDRNConnector::GetCurrentEndpoint( void ) const
{GUCEF_TRACE;

    return m_currentEndpoint;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNConnector::GetAttemptCount( void ) const
{GUCEF_TRACE;

    return m_attempts;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNConnector::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNConnector";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNConnector::OnNotify( CORE::CNotifier* notifier                 ,
                         const CORE::CEvent& eventid               ,
                         CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( m_candidate.IsNULL() || notifier != m_candidate.GetPointerAlways() || m_done )
        return;

    if ( CDRNPeerLink::LinkOperationalEvent == eventid )
    {
        m_done = true;
        m_succeeded = true;
        UnsubscribeFrom( m_candidate.GetPointerAlways() );
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNConnector: Connected to " << m_currentEndpoint.ToString() );
        CDRNPeerLinkEventData eventData( m_candidate );
        NotifyObservers( ConnectedEvent, &eventData );
    }
    else
    if ( CDRNPeerLink::LinkRefusedEvent == eventid )
    {
        const CDRNLinkClosedInfo& info = static_cast< CDRNLinkClosedEventData* >( eventdata )->GetData();
        Fail( info.reason, info.text );
    }
    else
    if ( CDRNPeerLink::LinkClosedEvent == eventid )
    {
        const CDRNLinkClosedInfo& info = static_cast< CDRNLinkClosedEventData* >( eventdata )->GetData();
        m_lastReason = info.reason;
        m_lastText = info.text;
        if ( !m_trying )
            TryNext();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
