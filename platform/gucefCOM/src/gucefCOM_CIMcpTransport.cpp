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

#include "gucefCOM_CIMcpTransport.h"

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

const CORE::CEvent CIMcpTransport::StartedEvent = "GUCEF::COM::CIMcpTransport::StartedEvent";
const CORE::CEvent CIMcpTransport::StoppedEvent = "GUCEF::COM::CIMcpTransport::StoppedEvent";
const CORE::CEvent CIMcpTransport::TransportErrorEvent = "GUCEF::COM::CIMcpTransport::TransportErrorEvent";

const CORE::CString CIMcpTransport::StdioTransportName = "stdio";
const CORE::CString CIMcpTransport::TcpTransportName = "tcp";
const CORE::CString CIMcpTransport::StreamableHttpTransportName = "streamable-http";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIMcpTransport::RegisterEvents( void )
{GUCEF_TRACE;

    StartedEvent.Initialize();
    StoppedEvent.Initialize();
    TransportErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIMcpTransport::CIMcpTransport( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIMcpTransport::~CIMcpTransport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIMcpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIMcpTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
