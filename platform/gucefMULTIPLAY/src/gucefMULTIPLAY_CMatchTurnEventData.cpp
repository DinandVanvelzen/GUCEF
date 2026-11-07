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


#include "gucefMULTIPLAY_CMatchTurnEventData.h"

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

CMatchTurnEventData::CMatchTurnEventData( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTurnEventData::CMatchTurnEventData( const CORE::CString& id, UInt32 turnRevision )
    : CORE::CICloneable()
    , matchId( id )
    , revision( turnRevision )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTurnEventData::CMatchTurnEventData( const CMatchTurnEventData& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTurnEventData::~CMatchTurnEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchTurnEventData&
CMatchTurnEventData::operator=( const CMatchTurnEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchTurnEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchTurnEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchTurnEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchTurnEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchTurnEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
