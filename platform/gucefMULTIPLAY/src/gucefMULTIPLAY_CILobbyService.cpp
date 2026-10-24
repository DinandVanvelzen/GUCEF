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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#include "gucefMULTIPLAY_CILobbyService.h"

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

const CORE::CEvent CILobbyService::ServiceConnectedEvent = "GUCEF::MULTIPLAY::CILobbyService::ServiceConnectedEvent";
const CORE::CEvent CILobbyService::ServiceDisconnectedEvent = "GUCEF::MULTIPLAY::CILobbyService::ServiceDisconnectedEvent";
const CORE::CEvent CILobbyService::LobbyJoinedEvent = "GUCEF::MULTIPLAY::CILobbyService::LobbyJoinedEvent";
const CORE::CEvent CILobbyService::ServiceErrorEvent = "GUCEF::MULTIPLAY::CILobbyService::ServiceErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyServiceCapabilities::CLobbyServiceCapabilities( void )
    : voting( false )
    , relay( false )
    , inProgressSpectating( false )
    , matchmaking( false )
    , chat( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CILobbyService::RegisterEvents( void )
{GUCEF_TRACE;

    ServiceConnectedEvent.Initialize();
    ServiceDisconnectedEvent.Initialize();
    LobbyJoinedEvent.Initialize();
    ServiceErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CILobbyService::CILobbyService( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CILobbyService::~CILobbyService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CILobbyService::PublishListing( const CGameListingEntry& listing )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CILobbyService::WithdrawListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CILobbyService::TryGetLobbyFromFuture( const CORE::CFutureResult& future, CILobbyPtr& lobby )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const CLobbyResultData* data = dynamic_cast< const CLobbyResultData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->GetData().IsNULL() )
        return false;
    lobby = data->GetData();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
