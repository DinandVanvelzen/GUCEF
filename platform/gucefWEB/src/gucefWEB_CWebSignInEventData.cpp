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

#ifndef GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#include "gucefWEB_CWebSignInEventData.h"
#define GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#endif /* GUCEF_WEB_CWEBSIGNINEVENTDATA_H ? */

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

CWebSignInEventData::CWebSignInEventData( void )
    : CORE::CICloneable()
    , storeId()
    , storeName()
    , mode( WEBSIGNIN_MODE_INTERACTIVE )
    , headless( false )
    , message()
    , personCheckKind()
    , stepNumber( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSignInEventData::~CWebSignInEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CWebSignInEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CWebSignInEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CWebSignInEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CWebSignInEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::WEB::CWebSignInEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
