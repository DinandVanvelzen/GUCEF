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

#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H
#include "gucefAUTH_CWebSessionSignInTaskData.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H ? */

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

CWebSessionSignInTaskData::CWebSessionSignInTaskData( void )
    : CORE::CICloneable()
    , signIn( GUCEF_NULL )
    , mode( WEB::WEBSIGNIN_MODE_INTERACTIVE )
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSessionSignInTaskData::~CWebSessionSignInTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CWebSessionSignInTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CWebSessionSignInTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CWebSessionSignInTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CWebSessionSignInTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUTH::CWebSessionSignInTaskData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
