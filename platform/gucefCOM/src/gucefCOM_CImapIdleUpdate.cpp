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


#include "gucefCOM_CImapIdleUpdate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapIdleUpdate::CImapIdleUpdate( void )
    : CORE::CICloneable()
    , kind( KIND_EXISTS )
    , number( 0 )
    , hasUid( false )
    , uid( 0 )
    , flags()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapIdleUpdate::CImapIdleUpdate( const CImapIdleUpdate& src )
    : CORE::CICloneable( src )
    , kind( src.kind )
    , number( src.number )
    , hasUid( src.hasUid )
    , uid( src.uid )
    , flags( src.flags )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapIdleUpdate::~CImapIdleUpdate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapIdleUpdate&
CImapIdleUpdate::operator=( const CImapIdleUpdate& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        kind = src.kind;
        number = src.number;
        hasUid = src.hasUid;
        uid = src.uid;
        flags = src.flags;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* CImapIdleUpdate::Clone( void ) const { return GUCEF_NEW CImapIdleUpdate( *this ); }
const char* CImapIdleUpdate::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CImapIdleUpdate::CloneableTypeName( void ) { return "GUCEF::COM::CImapIdleUpdate"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
