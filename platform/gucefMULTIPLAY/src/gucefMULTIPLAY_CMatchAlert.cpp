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

#include "gucefMULTIPLAY_CMatchAlert.h"

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

const char*
MatchAlertKindToString( TMatchAlertKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case MATCHALERTKIND_YOUR_TURN:    return "YOUR_TURN";
        case MATCHALERTKIND_COALESCED:    return "COALESCED";
        case MATCHALERTKIND_DIGEST:       return "DIGEST";
        case MATCHALERTKIND_INVITE:       return "INVITE";
        case MATCHALERTKIND_FINISHED:     return "FINISHED";
        default:                            return "YOUR_TURN";
    }
}

/*-------------------------------------------------------------------------*/

CMatchAlert::CMatchAlert( void )
    : CORE::CIDataNodeSerializable()
    , kind( MATCHALERTKIND_YOUR_TURN )
    , matchId()
    , revision( 0 )
    , title()
    , previousMover()
    , count( 1 )
    , createdAt( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAlert::CMatchAlert( const CMatchAlert& src )
    : CORE::CIDataNodeSerializable( src )
    , kind( src.kind )
    , matchId( src.matchId )
    , revision( src.revision )
    , title( src.title )
    , previousMover( src.previousMover )
    , count( src.count )
    , createdAt( src.createdAt )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAlert::~CMatchAlert()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchAlert&
CMatchAlert::operator=( const CMatchAlert& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        kind = src.kind;
        matchId = src.matchId;
        revision = src.revision;
        title = src.title;
        previousMover = src.previousMover;
        count = src.count;
        createdAt = src.createdAt;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlert::Serialize( CORE::CDataNode& domRootNode                        ,
                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "kind", CORE::CString( MatchAlertKindToString( kind ) ) );
    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "title", title );
    domRootNode.SetAttribute( "previousMover", previousMover );
    domRootNode.SetAttribute( "count", count );
    domRootNode.SetAttribute( "createdAt", createdAt );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlert::Deserialize( const CORE::CDataNode& domRootNode                  ,
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CORE::CString kindText = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true );
    kind = MATCHALERTKIND_YOUR_TURN;
    for ( int i=0; i<=(int) MATCHALERTKIND_FINISHED; ++i )
        if ( kindText == MatchAlertKindToString( (TMatchAlertKind) i ) )
            kind = (TMatchAlertKind) i;
    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( CORE::CString::Empty, true );
    previousMover = domRootNode.GetAttributeValueOrChildValueByName( "previousMover" ).AsString( CORE::CString::Empty, true );
    count = domRootNode.GetAttributeValueOrChildValueByName( "count" ).AsUInt32( 1, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchAlert::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchAlert( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchAlert::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchAlert";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchAlert::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchAlert::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchAlert";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
