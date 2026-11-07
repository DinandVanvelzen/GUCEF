/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CIEMailTransport.h"

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

const CORE::CEvent CIEMailTransport::ConnectedEvent = "GUCEF::COM::CIEMailTransport::ConnectedEvent";
const CORE::CEvent CIEMailTransport::TlsEstablishedEvent = "GUCEF::COM::CIEMailTransport::TlsEstablishedEvent";
const CORE::CEvent CIEMailTransport::LineReceivedEvent = "GUCEF::COM::CIEMailTransport::LineReceivedEvent";
const CORE::CEvent CIEMailTransport::BytesReceivedEvent = "GUCEF::COM::CIEMailTransport::BytesReceivedEvent";
const CORE::CEvent CIEMailTransport::DisconnectedEvent = "GUCEF::COM::CIEMailTransport::DisconnectedEvent";
const CORE::CEvent CIEMailTransport::TransportErrorEvent = "GUCEF::COM::CIEMailTransport::TransportErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIEMailTransport::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    TlsEstablishedEvent.Initialize();
    LineReceivedEvent.Initialize();
    BytesReceivedEvent.Initialize();
    DisconnectedEvent.Initialize();
    TransportErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIEMailTransport::CIEMailTransport( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIEMailTransport::~CIEMailTransport()
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
