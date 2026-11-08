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

#include "gucefCOMCORE_RpcEventData.h"

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

CRpcAcceptedEventData::CRpcAcceptedEventData( void )
    : CORE::CICloneable()
    , callRef( 0 )
    , methodPath()
    , requestId()
    , isDurable( false )
    , retryAfterMs( 0 )
    , etaMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcAcceptedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcAcceptedEventData( *this );
}

/*-------------------------------------------------------------------------*/

CRpcBindingEventData::CRpcBindingEventData( void )
    : CORE::CICloneable()
    , contractName()
    , major( 0 )
    , localMinor( 0 )
    , providerMinor( 0 )
    , bindIndex( 0 )
    , providerHash()
    , status()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcBindingEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcBindingEventData( *this );
}

/*-------------------------------------------------------------------------*/

CRpcSessionGapEventData::CRpcSessionGapEventData( void )
    : CORE::CICloneable()
    , gapToInitiatorFrom( 0 )
    , gapToInitiatorTo( 0 )
    , gapToAcceptorFrom( 0 )
    , gapToAcceptorTo( 0 )
    , sessionUnknown( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcSessionGapEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcSessionGapEventData( *this );
}

/*-------------------------------------------------------------------------*/

CRpcGapEventData::CRpcGapEventData( UInt64 gapFrom, UInt64 gapTo )
    : CORE::CICloneable()
    , from( gapFrom )
    , to( gapTo )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcGapEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcGapEventData( *this );
}

/*-------------------------------------------------------------------------*/

CRpcStreamMessageEventData::CRpcStreamMessageEventData( void )
    : CORE::CICloneable()
    , message()
    , streamIndex( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcStreamMessageEventData::CRpcStreamMessageEventData( const CRpcMessage& msg, UInt64 index )
    : CORE::CICloneable()
    , message( msg )
    , streamIndex( index )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcStreamMessageEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcStreamMessageEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
