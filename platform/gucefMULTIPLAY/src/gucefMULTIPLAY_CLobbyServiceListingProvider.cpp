/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLobbyServiceListingProvider::ProviderName = "lobby-service";

#define GUCEF_MULTIPLAY_LOBBYSERVICELISTING_POLL_MS     25

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyServiceListingProvider::CLobbyServiceListingProvider( void )
    : CIGameListingProvider()
    , m_service()
    , m_serviceName()
    , m_serviceConfig()
    , m_timer( GUCEF_MULTIPLAY_LOBBYSERVICELISTING_POLL_MS )
    , m_browseFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , m_browsing( false )
    , m_nextBrowseInMs( 0 )
    , m_refreshIntervalInSecs( 10 )
    , m_active( false )
{GUCEF_TRACE;

    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CLobbyServiceListingProvider::~CLobbyServiceListingProvider()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::EnsureService( void )
{GUCEF_TRACE;

    if ( !m_service.IsNULL() )
        return true;
    if ( m_serviceName.IsNULLOrEmpty() )
        return false;

    m_service = CMultiplayGlobal::Instance()->GetLobbyServiceFactory().Create( m_serviceName );
    if ( m_service.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LobbyServiceListingProvider: There is no lobby service \"" << m_serviceName << "\"" );
        return false;
    }
    if ( m_serviceConfig.GetNrOfChildNodes() > 0 || m_serviceConfig.GetAttCount() > 0 )
        m_service->LoadConfig( m_serviceConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::StartBrowse( void )
{GUCEF_TRACE;

    if ( m_browsing )
        return true;

    m_browseFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( m_service.IsNULL() || !m_service->Browse( GetFilter(), m_browseFuture ) )
    {
        m_nextBrowseInMs = GetTimeInMs() + m_refreshIntervalInSecs * 1000;
        ReportProviderError( "The lobby service cannot browse now" );
        return false;
    }
    m_browsing = true;
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::Start( const CGameListingFilter& filter )
{GUCEF_TRACE;

    if ( m_active )
        Stop();
    if ( !EnsureService() )
    {
        ReportProviderError( "No lobby service is set" );
        return false;
    }

    SetFilter( filter );
    m_active = true;
    m_timer.SetEnabled( true );
    StartBrowse();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLobbyServiceListingProvider::Stop( void )
{GUCEF_TRACE;

    m_active = false;
    m_browsing = false;
    m_timer.SetEnabled( false );
    m_browseFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    RemoveAllListings();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::Refresh( void )
{GUCEF_TRACE;

    if ( !m_active )
        return false;
    return StartBrowse();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyServiceListingProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

void
CLobbyServiceListingProvider::SetLobbyService( CILobbyServicePtr service )
{GUCEF_TRACE;

    m_service = service;
}

/*-------------------------------------------------------------------------*/

CILobbyServicePtr
CLobbyServiceListingProvider::GetLobbyService( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

void
CLobbyServiceListingProvider::SetRefreshIntervalInSecs( UInt32 seconds )
{GUCEF_TRACE;

    m_refreshIntervalInSecs = seconds;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::OnBrowseDone( void )
{GUCEF_TRACE;

    m_browsing = false;
    m_nextBrowseInMs = GetTimeInMs() + m_refreshIntervalInSecs * 1000;

    CGameListingList list;
    if ( !CGameListingList::TryGetFromFuture( m_browseFuture, list ) )
    {
        CORE::CTaskPtr task = m_browseFuture.GetResult( 0 );
        CORE::CString reason = task.IsNULL() ? CORE::CString( "unknown" ) : CORE::ToString( task->GetTaskStatus() );
        return ReportProviderError( "Browsing the lobby service failed: " + reason );
    }

    CORE::CString::StringSet seen;
    for ( size_t i=0; i<list.listings.size(); ++i )
    {
        CGameListingEntry& listing = list.listings[ i ];
        if ( listing.listingId.IsNULLOrEmpty() )
            continue;
        if ( listing.backendName.IsNULLOrEmpty() )
            listing.backendName = ProviderName;
        seen.insert( listing.listingId );
        if ( !UpsertListing( listing ) )
            return false;
    }
    return RemoveListingsExcept( seen );
}

/*-------------------------------------------------------------------------*/

void
CLobbyServiceListingProvider::OnTimer( void )
{GUCEF_TRACE;

    if ( !m_active )
    {
        m_timer.SetEnabled( false );
        return;
    }
    if ( m_browsing )
    {
        if ( m_browseFuture.HasAFuture() && m_browseFuture.IsReady() )
            OnBrowseDone();
        return;
    }
    if ( 0 != m_refreshIntervalInSecs && GetTimeInMs() >= m_nextBrowseInMs )
        StartBrowse();
}

/*-------------------------------------------------------------------------*/

void
CLobbyServiceListingProvider::OnNotify( CORE::CNotifier* notifier                 ,
                                        const CORE::CEvent& eventId               ,
                                        CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
        OnTimer();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "service", m_serviceName );
    config.SetAttribute( "refreshIntervalInSecs", m_refreshIntervalInSecs );
    CORE::CDataNode* serviceNode = config.AddChild( "ServiceConfig", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != serviceNode && !m_service.IsNULL() )
        m_service->SaveConfig( *serviceNode );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyServiceListingProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_serviceName = config.GetAttributeValueOrChildValueByName( "service" ).AsString( m_serviceName, true );
    m_refreshIntervalInSecs = config.GetAttributeValueOrChildValueByName( "refreshIntervalInSecs" ).AsUInt32( m_refreshIntervalInSecs, true );
    const CORE::CDataNode* serviceNode = config.FindChild( "ServiceConfig" );
    if ( GUCEF_NULL != serviceNode )
        m_serviceConfig = *serviceNode;
    return !m_serviceName.IsNULLOrEmpty() || !m_service.IsNULL();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyServiceListingProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbyServiceListingProvider";
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
