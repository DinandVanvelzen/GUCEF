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

#include "gucefCOMCORE_CRpcCallResult.h"

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

CRpcCallResult::CRpcCallResult( void )
    : CORE::CICloneable()
    , status()
    , response()
    , requestId()
    , wasAccepted( false )
    , isDurable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallResult::CRpcCallResult( const CRpcCallResult& src )
    : CORE::CICloneable( src )
    , status( src.status )
    , response( src.response )
    , requestId( src.requestId )
    , wasAccepted( src.wasAccepted )
    , isDurable( src.isDurable )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallResult::~CRpcCallResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallResult&
CRpcCallResult::operator=( const CRpcCallResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        status = src.status;
        response = src.response;
        requestId = src.requestId;
        wasAccepted = src.wasAccepted;
        isDurable = src.isDurable;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcCallResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcCallResult( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
