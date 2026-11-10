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

#include "gucefMULTIPLAY_CIAsyncLobbyBackend.h"

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

const CORE::CEvent CIAsyncLobbyBackend::ListingsChangedEvent = "GUCEF::MULTIPLAY::CIAsyncLobbyBackend::ListingsChangedEvent";
const CORE::CEvent CIAsyncLobbyBackend::LobbyChangedEvent = "GUCEF::MULTIPLAY::CIAsyncLobbyBackend::LobbyChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIAsyncLobbyBackend::RegisterEvents( void )
{GUCEF_TRACE;

    ListingsChangedEvent.Initialize();
    LobbyChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIAsyncLobbyBackend::CIAsyncLobbyBackend( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIAsyncLobbyBackend::~CIAsyncLobbyBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIAsyncLobbyBackend::Cancel( const CORE::CString& /* lobbyId */, CORE::CString& error )
{GUCEF_TRACE;

    error = "This kind of lobby is closed by leaving it.";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAsyncLobbyBackend::NotifyListingsChanged( void )
{GUCEF_TRACE;

    return NotifyObservers( ListingsChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CIAsyncLobbyBackend::NotifyLobbyChanged( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    TLobbyEventData data( lobbyId );
    return NotifyObservers( LobbyChangedEvent, &data );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
