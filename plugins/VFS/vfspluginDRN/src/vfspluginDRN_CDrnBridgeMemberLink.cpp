/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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


#include "vfspluginDRN_CDrnBridgeMemberLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeMemberLink::CDrnBridgeMemberLink( DRN::CDRNSession& session, const CORE::CString& memberKey )
    : CORE::CObserver()
    , m_lock()
    , m_session( &session )
    , m_memberKey( memberKey )
    , m_client()
    , m_clientPeerId( 0 )
    , m_clientsMade( 0 )
    , m_users()
{GUCEF_TRACE;

    // the session tells that it is going away through its destruction event, which every observer gets
    SubscribeTo( m_session, DRN::CDRNSession::SessionClosedEvent );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMemberLink::~CDrnBridgeMemberLink()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMemberLink::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid )
    {
        MT::CScopeMutex lock( m_lock );
        if ( static_cast< CORE::CNotifier* >( m_session ) == notifier )
        {
            m_session = GUCEF_NULL;
            m_client.Unlink();
        }
    }
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession*
CDrnBridgeMemberLink::GetSession( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_session;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeMemberLink::GetMemberKey( void ) const
{GUCEF_TRACE;

    return m_memberKey;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMemberLink::TClientPtr
CDrnBridgeMemberLink::ObtainClient( DRN::CDRNSession& session, UInt32 peerId, UInt32 timeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_client.IsNULL() || m_clientPeerId != peerId )
    {
        m_client = TClientPtr( GUCEF_NEW DRN::CDRNNamespaceClient( session, peerId ) );
        m_clientPeerId = peerId;
        ++m_clientsMade;
    }
    if ( 0 != timeoutInMs )
        m_client->SetDefaultTimeoutMs( timeoutInMs );
    return m_client;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMemberLink::TClientPtr
CDrnBridgeMemberLink::GetClient( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMemberLink::HasNegotiated( void ) const
{GUCEF_TRACE;

    TClientPtr client = GetClient();
    DRN::CDRNNamespaceInfo info;
    return !client.IsNULL() && client->GetNegotiatedInfo( info );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMemberLink::HasAnnouncedLimits( void ) const
{GUCEF_TRACE;

    TClientPtr client = GetClient();
    DRN::CDRNNamespaceInfo info;
    return !client.IsNULL() && client->GetNegotiatedInfo( info ) && info.limitsAnnounced;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeMemberLink::GetClientsMade( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_clientsMade;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMemberLink::RegisterHandleUser( const CDrnBridgeHandlePool* user )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TUserVector::const_iterator i = m_users.begin(); i != m_users.end(); ++i )
        if ( (*i) == user )
            return;
    m_users.push_back( user );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMemberLink::UnregisterHandleUser( const CDrnBridgeHandlePool* user )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TUserVector::iterator i = m_users.begin(); i != m_users.end(); ++i )
    {
        if ( (*i) == user )
        {
            m_users.erase( i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeMemberLink::GetHandleUserCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_users.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeMemberLink::GetHandleShare( const CDrnBridgeHandlePool* user ) const
{GUCEF_TRACE;

    TClientPtr client = GetClient();
    if ( client.IsNULL() )
        return 0;
    DRN::CDRNNamespaceInfo info;
    if ( !client->GetNegotiatedInfo( info ) || !info.limitsAnnounced || 0 == info.maxOpenHandles )
        return 0;

    // the announced limit is divided between the pools in the order they registered: the first ones get the remainder
    MT::CScopeMutex lock( m_lock );
    UInt32 count = (UInt32) m_users.size();
    UInt32 index = 0;
    while ( index < count && m_users[ index ] != user )
        ++index;
    if ( 0 == count || index >= count )
        return info.maxOpenHandles;
    UInt32 share = info.maxOpenHandles / count + ( index < info.maxOpenHandles % count ? 1 : 0 );
    return share < 1 ? 1 : share;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
