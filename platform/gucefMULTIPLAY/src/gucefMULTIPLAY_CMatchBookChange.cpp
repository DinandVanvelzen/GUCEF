/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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


#include "gucefMULTIPLAY_CMatchBookChange.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchBookChange::CMatchBookChange( void )
    : CORE::CICloneable()
    , matchId()
    , kind( MATCHBOOKCHANGE_UPDATED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchBookChange::CMatchBookChange( const CORE::CString& id, TMatchBookChangeKind changeKind )
    : CORE::CICloneable()
    , matchId( id )
    , kind( changeKind )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchBookChange::CMatchBookChange( const CMatchBookChange& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , kind( src.kind )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchBookChange::~CMatchBookChange()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchBookChange&
CMatchBookChange::operator=( const CMatchBookChange& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        kind = src.kind;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchBookChange::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchBookChange( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchBookChange::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchBookChange::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchBookChange";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
