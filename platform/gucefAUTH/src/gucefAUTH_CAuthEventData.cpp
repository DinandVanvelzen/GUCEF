/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include "gucefAUTH_CAuthEventData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthActionRequired::CAuthActionRequired( void )
    : CORE::CICloneable()
    , providerName()
    , kind( AUTHACTION_OPEN_URL )
    , url()
    , urlWithCode()
    , userCode()
    , expiresAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthActionRequired::CAuthActionRequired( const CAuthActionRequired& src )
    : CORE::CICloneable( src )
    , providerName( src.providerName )
    , kind( src.kind )
    , url( src.url )
    , urlWithCode( src.urlWithCode )
    , userCode( src.userCode )
    , expiresAtMs( src.expiresAtMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthActionRequired::~CAuthActionRequired()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthActionRequired&
CAuthActionRequired::operator=( const CAuthActionRequired& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        kind = src.kind;
        url = src.url;
        urlWithCode = src.urlWithCode;
        userCode = src.userCode;
        expiresAtMs = src.expiresAtMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthActionRequired::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthActionRequired( *this );
}

/*-------------------------------------------------------------------------*/

CAuthFailure::CAuthFailure( void )
    : CORE::CICloneable()
    , providerName()
    , reason( AUTHFAIL_UNKNOWN )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthFailure::CAuthFailure( const CORE::CString& provider, TAuthFailureReason failureReason, const CORE::CString& failureMessage )
    : CORE::CICloneable()
    , providerName( provider )
    , reason( failureReason )
    , message( failureMessage )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthFailure::CAuthFailure( const CAuthFailure& src )
    : CORE::CICloneable( src )
    , providerName( src.providerName )
    , reason( src.reason )
    , message( src.message )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthFailure::~CAuthFailure()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthFailure&
CAuthFailure::operator=( const CAuthFailure& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        reason = src.reason;
        message = src.message;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthFailure::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthFailure( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
