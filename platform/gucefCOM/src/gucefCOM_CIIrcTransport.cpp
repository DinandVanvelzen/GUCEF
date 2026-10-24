/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CIIrcTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIIrcTransport::ConnectedEvent = "GUCEF::COM::CIIrcTransport::ConnectedEvent";
const CORE::CEvent CIIrcTransport::LineReceivedEvent = "GUCEF::COM::CIIrcTransport::LineReceivedEvent";
const CORE::CEvent CIIrcTransport::DisconnectedEvent = "GUCEF::COM::CIIrcTransport::DisconnectedEvent";
const CORE::CEvent CIIrcTransport::TransportErrorEvent = "GUCEF::COM::CIIrcTransport::TransportErrorEvent";

const CORE::CString CIIrcTransport::TcpTransportName = "tcp";
const CORE::CString CIIrcTransport::WebSocketTransportName = "websocket";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIIrcTransport::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    LineReceivedEvent.Initialize();
    DisconnectedEvent.Initialize();
    TransportErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIIrcTransport::CIIrcTransport( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIIrcTransport::~CIIrcTransport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
