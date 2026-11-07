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

#include "gucefCOM_CIMailTransport.h"

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

const CORE::CEvent CIMailTransport::ConnectedEvent = "GUCEF::COM::CIMailTransport::ConnectedEvent";
const CORE::CEvent CIMailTransport::TlsEstablishedEvent = "GUCEF::COM::CIMailTransport::TlsEstablishedEvent";
const CORE::CEvent CIMailTransport::LineReceivedEvent = "GUCEF::COM::CIMailTransport::LineReceivedEvent";
const CORE::CEvent CIMailTransport::BytesReceivedEvent = "GUCEF::COM::CIMailTransport::BytesReceivedEvent";
const CORE::CEvent CIMailTransport::DisconnectedEvent = "GUCEF::COM::CIMailTransport::DisconnectedEvent";
const CORE::CEvent CIMailTransport::TransportErrorEvent = "GUCEF::COM::CIMailTransport::TransportErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailTransportErrorData::CMailTransportErrorData( void )
    : CORE::CICloneable()
    , errorClass( MAILERR_CONNECTION )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailTransportErrorData::CMailTransportErrorData( EMailErrorClass errorClassOfError, const CORE::CString& descriptionOfError )
    : CORE::CICloneable()
    , errorClass( errorClassOfError )
    , description( descriptionOfError )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailTransportErrorData::CMailTransportErrorData( const CMailTransportErrorData& src )
    : CORE::CICloneable( src )
    , errorClass( src.errorClass )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailTransportErrorData::~CMailTransportErrorData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailTransportErrorData&
CMailTransportErrorData::operator=( const CMailTransportErrorData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        errorClass = src.errorClass;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailTransportErrorData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailTransportErrorData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailTransportErrorData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailTransportErrorData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMailTransportErrorData";
}

/*-------------------------------------------------------------------------*/

void
CIMailTransport::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    TlsEstablishedEvent.Initialize();
    LineReceivedEvent.Initialize();
    BytesReceivedEvent.Initialize();
    DisconnectedEvent.Initialize();
    TransportErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIMailTransport::CIMailTransport( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIMailTransport::~CIMailTransport()
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
