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

#include "gucefCOMCORE_CRpcRequestStatus.h"

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

CRpcRequestStatus::CRpcRequestStatus( void )
    : CORE::CICloneable()
    , requestId()
    , state( RPC_REQUESTSTATE_UNKNOWN )
    , progressPermille( 1001 )
    , etaMs( 0 )
    , note()
    , status()
    , hasResult( false )
    , result()
    , resultPayload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestStatus::CRpcRequestStatus( const CRpcRequestStatus& src )
    : CORE::CICloneable( src )
    , requestId( src.requestId )
    , state( src.state )
    , progressPermille( src.progressPermille )
    , etaMs( src.etaMs )
    , note( src.note )
    , status( src.status )
    , hasResult( src.hasResult )
    , result( src.result )
    , resultPayload( src.resultPayload )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestStatus::~CRpcRequestStatus()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcRequestStatus::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcRequestStatus( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
