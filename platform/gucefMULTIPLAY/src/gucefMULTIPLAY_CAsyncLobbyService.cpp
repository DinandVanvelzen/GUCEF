/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#include "gucefMULTIPLAY_CAsyncLobbyService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncLobbyService::CAsyncLobbyService( CIAsyncLobbyBackend& backend, const CORE::CString& serviceName )
    : CILobbyService()
    , m_backend( &backend )
    , m_serviceName( serviceName )
    , m_lobbies()
    , m_connected( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncLobbyService::~CAsyncLobbyService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAsyncLobbyBackend&
CAsyncLobbyService::GetBackend( void ) const
{GUCEF_TRACE;

    return *m_backend;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();

    CORE::CString reason;
    if ( !m_backend->IsAvailable( reason ) )
    {
        m_connected = false;
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason.IsNULLOrEmpty() ? CORE::CString( "The lobbies are not available." ) : reason );
        return false;
    }
    m_connected = true;
    source.Complete( identity.Clone(), true );
    NotifyObservers( ServiceConnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyService::Disconnect( void )
{GUCEF_TRACE;

    if ( !m_connected )
        return;
    m_connected = false;
    m_lobbies.clear();
    NotifyObservers( ServiceDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::Browse( const CGameListingFilter& filter, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }
    CIAsyncLobbyBackend::TGameListingEntryVector rows;
    m_backend->GetListings( rows );
    CGameListingList list;
    for ( size_t i=0; i<rows.size(); ++i )
        if ( filter.Matches( rows[ i ] ) )
            list.listings.push_back( rows[ i ] );
    source.Complete( list.Clone(), true );
    return true;
}

/*-------------------------------------------------------------------------*/

CILobbyPtr
CAsyncLobbyService::OpenLobby( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    TLobbyMap::iterator found = m_lobbies.find( lobbyId );
    if ( found != m_lobbies.end() )
        return (*found).second;

    CAsyncLobbyView view;
    if ( !m_backend->GetLobby( lobbyId, view ) )
        return CILobbyPtr();
    CORE::CTSharedPtr< CAsyncLobby, MT::CMutex > shared( GUCEF_NEW CAsyncLobby( *m_backend, lobbyId ) );
    CILobbyPtr lobby( shared );
    m_lobbies[ lobbyId ] = lobby;
    return lobby;
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyService::ReleaseLobby( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    m_lobbies.erase( lobbyId );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::CreateLobby( const CLobbyCreateParams& params, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }
    CORE::CString lobbyId, error;
    if ( !m_backend->CreateLobby( params, lobbyId, error ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, error.IsNULLOrEmpty() ? CORE::CString( "The lobby could not be made." ) : error );
        return false;
    }
    CILobbyPtr lobby = OpenLobby( lobbyId );
    if ( lobby.IsNULL() )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby was made but is not in the list." );
        return false;
    }
    CLobbyResultData* result = GUCEF_NEW CLobbyResultData( lobby );
    source.Complete( result, true );
    TLobbyJoinedEventData joined( lobby );
    NotifyObservers( LobbyJoinedEvent, &joined );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::JoinLobby( const CORE::CString& lobbyId    ,
                               const CORE::CString& /* password */ ,
                               bool asSpectator                ,
                               CORE::CFutureResult& future     )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }
    CAsyncLobbyView view;
    if ( !m_backend->GetLobby( lobbyId, view ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "That game is not in your list." );
        return false;
    }
    if ( asSpectator )
    {
        CORE::CString error;
        if ( !m_backend->SetSpectating( lobbyId, true, error ) )
        {
            source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, error.IsNULLOrEmpty() ? CORE::CString( "You cannot watch this game." ) : error );
            return false;
        }
    }
    CILobbyPtr lobby = OpenLobby( lobbyId );
    if ( lobby.IsNULL() )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "That game is not in your list." );
        return false;
    }
    source.Complete( GUCEF_NEW CLobbyResultData( lobby ), true );
    TLobbyJoinedEventData joined( lobby );
    NotifyObservers( LobbyJoinedEvent, &joined );
    return true;
}

/*-------------------------------------------------------------------------*/

CLobbyServiceCapabilities
CAsyncLobbyService::GetCapabilities( void ) const
{GUCEF_TRACE;

    CLobbyServiceCapabilities capabilities;
    capabilities.voting = true;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobbyService::GetServiceName( void ) const
{GUCEF_TRACE;

    return m_serviceName;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::SaveConfig( CORE::CDataNode& /* config */ ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyService::LoadConfig( const CORE::CDataNode& /* config */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobbyService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CAsyncLobbyService";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
