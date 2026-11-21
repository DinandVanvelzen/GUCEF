/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWEBSIGNINRESULT_H
#include "gucefWEB_CWebSignInResult.h"
#define GUCEF_WEB_CWEBSIGNINRESULT_H
#endif /* GUCEF_WEB_CWEBSIGNINRESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebSignInResult::CWebSignInResult( void )
    : CORE::CICloneable()
    , storeId()
    , status( WEBSIGNIN_STATUS_FAILED )
    , message()
    , cookieCount( 0 )
    , handedOver( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSignInResult::~CWebSignInResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CWebSignInResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CWebSignInResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CWebSignInResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CWebSignInResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::WEB::CWebSignInResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
