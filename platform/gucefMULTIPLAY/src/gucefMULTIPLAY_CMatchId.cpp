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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#include "gucefMULTIPLAY_CMatchId.h"

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

CMatchId::CMatchId( void )
    : CORE::CIDataNodeSerializable()
    , id()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchId::CMatchId( const CORE::CString& text )
    : CORE::CIDataNodeSerializable()
    , id( text )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchId::CMatchId( const CMatchId& src )
    : CORE::CIDataNodeSerializable( src )
    , id( src.id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchId::~CMatchId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchId&
CMatchId::operator=( const CMatchId& src )
{GUCEF_TRACE;

    if ( &src != this )
        id = src.id;
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchId::operator==( const CMatchId& other ) const
{GUCEF_TRACE;

    return id == other.id;
}

/*-------------------------------------------------------------------------*/

bool
CMatchId::IsValid( void ) const
{GUCEF_TRACE;

    return IsValid( id );
}

/*-------------------------------------------------------------------------*/

bool
CMatchId::Serialize( CORE::CDataNode& domRootNode                        ,
                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchId::Deserialize( const CORE::CDataNode& domRootNode                  ,
                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    id = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    return IsValid( id );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchId::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchId( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchId::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchId";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchId::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchId::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchId";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchId::Generate( void )
{GUCEF_TRACE;

    return CPlayerIdentity::MakeGuid();
}

/*-------------------------------------------------------------------------*/

bool
CMatchId::IsValid( const CORE::CString& text )
{GUCEF_TRACE;

    const char* p = text.C_String();
    if ( strlen( p ) != TextLength )
        return false;
    for ( UInt32 i=0; i<TextLength; ++i )
    {
        char c = p[ i ];
        if ( 8 == i || 13 == i || 18 == i || 23 == i )
        {
            if ( '-' != c )
                return false;
        }
        else
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchId::Normalize( const CORE::CString& text )
{GUCEF_TRACE;

    if ( strlen( text.C_String() ) != TextLength )
        return CORE::CString::Empty;
    CORE::CString lower = text.Lowercase();
    return IsValid( lower ) ? lower : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchId::Short( const CORE::CString& matchId )
{GUCEF_TRACE;

    return strlen( matchId.C_String() ) >= ShortLength ? CORE::CString( matchId.C_String(), ShortLength ) : matchId;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
