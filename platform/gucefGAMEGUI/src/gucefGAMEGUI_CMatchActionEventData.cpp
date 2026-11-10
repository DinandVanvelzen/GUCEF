/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#include "gucefGAMEGUI_CMatchActionEventData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( void )
    : CORE::CICloneable()
    , matchId()
    , action( MATCHACTION_MUTE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( const CORE::CString& id, TMatchAction matchAction )
    : CORE::CICloneable()
    , matchId( id )
    , action( matchAction )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( const CMatchActionEventData& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , action( src.action )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::~CMatchActionEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData&
CMatchActionEventData::operator=( const CMatchActionEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        action = src.action;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchActionEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchActionEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchActionEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchActionEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CMatchActionEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
