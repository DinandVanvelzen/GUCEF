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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H
#include "gucefDRN_CDRNRpcSessionComponent.h"
#define GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H ? */

#include "gucefDRN_CDRNRpcTransportListener.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRpcTransportListener::CDRNRpcTransportListener( void )
    : COMCORE::CIRpcTransportListener()
    , m_lock()
    , m_settings()
    , m_session( GUCEF_NULL )
    , m_node( GUCEF_NULL )
    , m_component( GUCEF_NULL )
    , m_pulseGenerator()
    , m_listenUri()
    , m_listening( false )
    , m_refused( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcTransportListener::CDRNRpcTransportListener( CDRNSession& session, const CDRNRpcTransportSettings& settings )
    : COMCORE::CIRpcTransportListener()
    , m_lock()
    , m_settings( settings )
    , m_session( &session )
    , m_node( GUCEF_NULL )
    , m_component( GUCEF_NULL )
    , m_pulseGenerator()
    , m_listenUri()
    , m_listening( false )
    , m_refused( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcTransportListener::~CDRNRpcTransportListener()
{GUCEF_TRACE;

    StopListening();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransportListener::SetSettings( const CDRNRpcTransportSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CDRNRpcTransportSettings&
CDRNRpcTransportListener::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRpcTransportListener::GetRefusedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refused;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNRpcTransportListener::GetTransportType( void ) const
{GUCEF_TRACE;

    return CDRNRpcTransport::TransportType;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::Listen( const CORE::CUri& local )
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_listening )
            return false;
        session = m_session;
    }
    if ( !local.GetScheme().IsNULLOrEmpty() || !local.GetAuthority().IsNULLOrEmpty() )
    {
        // drn://<session name>
        if ( !local.GetScheme().Equals( CDRNRpcTransport::UriScheme, false ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcTransportListener: Not a drn URI: " << local.ToUriInStringForm() );
            return false;
        }
        CORE::CString sessionName = local.GetAuthorityHost().IsNULLOrEmpty() ? local.GetAuthority() : local.GetAuthorityHost();
        session = CDRNGlobal::Instance()->GetSessionRegistry().Find( sessionName );
        if ( GUCEF_NULL == session )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcTransportListener: No session \"" << sessionName << "\" is registered in CDRNGlobal::GetSessionRegistry()" );
            return false;
        }
    }
    if ( GUCEF_NULL == session )
        return false;

    MT::CScopeMutex nodeLock( session->GetNode().GetLock() );
    CDRNRpcSessionComponent* component = CDRNRpcSessionComponent::Obtain( *session );
    if ( GUCEF_NULL == component )
        return false;
    component->AddListener( this );
    {
        MT::CScopeMutex lock( m_lock );
        m_session = session;
        m_node = &session->GetNode();
        m_component = component;
        m_pulseGenerator = m_node->GetPulseGenerator();
        m_listenUri = CORE::CUri( CDRNRpcTransport::UriScheme + "://" + session->GetName() );
        m_listening = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransportListener::StopListening( void )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
        if ( GUCEF_NULL == node )
        {
            m_listening = false;
            return;
        }
    }
    MT::CScopeMutex nodeLock( node->GetLock() );
    CDRNRpcSessionComponent* component = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        component = m_component;
        m_component = GUCEF_NULL;
        m_node = GUCEF_NULL;
        m_listening = false;
    }
    if ( GUCEF_NULL != component )
        component->RemoveListener( this );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::IsListening( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_listening;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CDRNRpcTransportListener::GetListenUri( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_listenUri;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::SetPulseGenerator( const CORE::PulseGeneratorPtr& /* pulseGenerator */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNRpcTransportListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::OfferChannel( CDRNRpcSessionComponent& component, const CDRNPeerLinkPtr& link, UInt32 peerId, UInt32 channel )
{GUCEF_TRACE;

    // node thread, node lock held
    CDRNRpcTransportSettings settings;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_listening )
            return false;
        settings = m_settings;
        if ( settings.requireProtectedLink && !link->IsProtected() )
        {
            ++m_refused;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcTransportListener: Refused an RPC channel of member " << peerId << ": the link has no packet protection" );
            return false;
        }
    }
    if ( 0 != settings.maxSessionsPerMember && component.GetAcceptedChannelCount( peerId ) >= settings.maxSessionsPerMember )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcTransportListener: Refused an RPC channel of member " << peerId << ": it has " <<
                           settings.maxSessionsPerMember << " open already" );
        return false;
    }

    CORE::CTSharedPtr< CDRNRpcTransport, MT::CMutex > transport = ( GUCEF_NEW CDRNRpcTransport( component.GetSession(), peerId, settings ) )->CreateSharedPtr();
    component.AddAcceptedChannel( peerId, channel, transport.GetPointerAlways() );
    transport->Attach( component, link, peerId, channel, false );

    // the observer (a CRpcHost) takes the transport inside the event; the first frame, the HELLO, follows later.
    // A transport nobody keeps closes the channel when it goes at the end of this call.
    COMCORE::CIRpcTransportPtr accepted;
    accepted = transport;
    COMCORE::CRpcTransportAcceptedEventData eventData( accepted );
    NotifyObservers( TransportAcceptedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransportListener::OnComponentGone( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_component = GUCEF_NULL;
    m_node = GUCEF_NULL;
    m_session = GUCEF_NULL;
    m_listening = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportListener::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CDRNRpcTransportSettings settings;
    if ( !settings.LoadConfig( config ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNRpcTransportListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNRpcTransportListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
