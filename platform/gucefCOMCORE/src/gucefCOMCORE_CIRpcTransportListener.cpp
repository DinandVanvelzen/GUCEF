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

#include "gucefCOMCORE_CIRpcTransportListener.h"

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

const CORE::CEvent CIRpcTransportListener::TransportAcceptedEvent = "GUCEF::COMCORE::CIRpcTransportListener::TransportAcceptedEvent";
const CORE::CEvent CIRpcTransportListener::ListenerErrorEvent = "GUCEF::COMCORE::CIRpcTransportListener::ListenerErrorEvent";

/*-------------------------------------------------------------------------*/

void
CIRpcTransportListener::RegisterEvents( void )
{GUCEF_TRACE;

    TransportAcceptedEvent.Initialize();
    ListenerErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIRpcTransportListener::CIRpcTransportListener( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIRpcTransportListener::~CIRpcTransportListener()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransportListener::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransportListener::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIRpcTransportListener::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIRpcTransportListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CIRpcTransportListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CRpcTransportAcceptedEventData::CRpcTransportAcceptedEventData( const CIRpcTransportPtr& acceptedTransport )
    : CORE::CICloneable()
    , transport( acceptedTransport )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcTransportAcceptedEventData::CRpcTransportAcceptedEventData( const CRpcTransportAcceptedEventData& src )
    : CORE::CICloneable( src )
    , transport( src.transport )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcTransportAcceptedEventData::~CRpcTransportAcceptedEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcTransportAcceptedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcTransportAcceptedEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
