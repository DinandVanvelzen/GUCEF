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

#include "gucefAUTH_CKeyringOperationData.h"

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

CKeyringOperationData::CKeyringOperationData( void )
    : CORE::CICloneable()
    , session( GUCEF_NULL )
    , operation( OPERATION_UNLOCK )
    , code()
    , newCode()
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringOperationData::CKeyringOperationData( const CKeyringOperationData& src )
    : CORE::CICloneable( src )
    , session( src.session )
    , operation( src.operation )
    , code( src.code )
    , newCode( src.newCode )
    , source( src.source )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringOperationData::~CKeyringOperationData()
{GUCEF_TRACE;

    code.Clear();
    newCode.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CKeyringOperationData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKeyringOperationData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CKeyringOperationData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CKeyringOperationData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUTH::CKeyringOperationData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
