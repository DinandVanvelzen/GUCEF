/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CIRpcTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIRpcTransport::ConnectedEvent = "GUCEF::COMCORE::CIRpcTransport::ConnectedEvent";
const CORE::CEvent CIRpcTransport::DisconnectedEvent = "GUCEF::COMCORE::CIRpcTransport::DisconnectedEvent";
const CORE::CEvent CIRpcTransport::FrameReceivedEvent = "GUCEF::COMCORE::CIRpcTransport::FrameReceivedEvent";
const CORE::CEvent CIRpcTransport::WritableEvent = "GUCEF::COMCORE::CIRpcTransport::WritableEvent";
const CORE::CEvent CIRpcTransport::TransportErrorEvent = "GUCEF::COMCORE::CIRpcTransport::TransportErrorEvent";

/*-------------------------------------------------------------------------*/

void
CIRpcTransport::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    DisconnectedEvent.Initialize();
    FrameReceivedEvent.Initialize();
    WritableEvent.Initialize();
    TransportErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIRpcTransport::CIRpcTransport( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIRpcTransport::~CIRpcTransport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransport::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CIRpcTransport::OnProtectionDowngradeRefused( TRpcProtectionClass required, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransport::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransport::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIRpcTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CIRpcTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
