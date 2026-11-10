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

#include "gucefCOMCORE_CRpcStatus.h"

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

const CRpcStatus CRpcStatus::Ok;

/*-------------------------------------------------------------------------*/

CRpcStatus::CRpcStatus( void )
    : CORE::CICloneable()
    , code( RPC_STATUS_OK )
    , message()
    , details()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcStatus::CRpcStatus( TRpcStatusCode statusCode, const CORE::CString& statusMessage )
    : CORE::CICloneable()
    , code( statusCode )
    , message( statusMessage )
    , details()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcStatus::CRpcStatus( const CRpcStatus& src )
    : CORE::CICloneable( src )
    , code( src.code )
    , message( src.message )
    , details( src.details )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcStatus::~CRpcStatus()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcStatus&
CRpcStatus::operator=( const CRpcStatus& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        code = src.code;
        message = src.message;
        details = src.details;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CRpcStatus::IsOk( void ) const
{GUCEF_TRACE;

    return RPC_STATUS_OK == code;
}

/*-------------------------------------------------------------------------*/

void
CRpcStatus::Set( TRpcStatusCode statusCode, const CORE::CString& statusMessage )
{GUCEF_TRACE;

    code = statusCode;
    message = statusMessage;
    details.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcStatus::ToString( void ) const
{GUCEF_TRACE;

    if ( message.IsNULLOrEmpty() )
        return RpcStatusCodeToString( code );
    return CORE::CString( RpcStatusCodeToString( code ) ) + ": " + message;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcStatus::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcStatus( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
