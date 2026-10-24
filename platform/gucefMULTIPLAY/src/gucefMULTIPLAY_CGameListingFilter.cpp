/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefMULTIPLAY_CGameListingFilter.h"

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

CGameListingFilter::CGameListingFilter( void )
    : CORE::CIConfigurable()
    , text()
    , kind( KINDFILTER_ANY )
    , gameId()
    , gameVersion()
    , notFull( false )
    , noPassword( false )
    , maxPingMs( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingFilter::CGameListingFilter( const CGameListingFilter& src )
    : CORE::CIConfigurable( src )
    , text( src.text )
    , kind( src.kind )
    , gameId( src.gameId )
    , gameVersion( src.gameVersion )
    , notFull( src.notFull )
    , noPassword( src.noPassword )
    , maxPingMs( src.maxPingMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingFilter::~CGameListingFilter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingFilter&
CGameListingFilter::operator=( const CGameListingFilter& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        text = src.text;
        kind = src.kind;
        gameId = src.gameId;
        gameVersion = src.gameVersion;
        notFull = src.notFull;
        noPassword = src.noPassword;
        maxPingMs = src.maxPingMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingFilter::Matches( const CGameListingEntry& entry ) const
{GUCEF_TRACE;

    if ( KINDFILTER_LOBBY == kind && CGameListingEntry::KIND_LOBBY != entry.kind )
        return false;
    if ( KINDFILTER_GAME_IN_PROGRESS == kind && CGameListingEntry::KIND_GAME_IN_PROGRESS != entry.kind )
        return false;
    if ( !gameId.IsNULLOrEmpty() && gameId != entry.gameId )
        return false;
    if ( !gameVersion.IsNULLOrEmpty() && gameVersion != entry.gameVersion )
        return false;
    if ( notFull && entry.IsFull() )
        return false;
    if ( noPassword && entry.passworded )
        return false;
    if ( maxPingMs >= 0 && entry.pingMs >= 0 && entry.pingMs > maxPingMs )
        return false;

    if ( !text.IsNULLOrEmpty() )
    {
        CORE::CString lowerText = text.Lowercase();
        if ( entry.name.Lowercase().HasSubstr( lowerText ) < 0     &&
             entry.mapName.Lowercase().HasSubstr( lowerText ) < 0  &&
             entry.modeName.Lowercase().HasSubstr( lowerText ) < 0 &&
             entry.hostName.Lowercase().HasSubstr( lowerText ) < 0  )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingFilter::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "text", text );
    config.SetAttribute( "kind", CORE::CString( KINDFILTER_LOBBY == kind ? "lobby" : ( KINDFILTER_GAME_IN_PROGRESS == kind ? "game" : "any" ) ) );
    config.SetAttribute( "gameId", gameId );
    config.SetAttribute( "gameVersion", gameVersion );
    config.SetAttribute( "notFull", notFull );
    config.SetAttribute( "noPassword", noPassword );
    config.SetAttribute( "maxPingMs", maxPingMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingFilter::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    text = config.GetAttributeValueOrChildValueByName( "text" ).AsString( text, true );
    CORE::CString kindStr = config.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ).Lowercase();
    if ( "lobby" == kindStr )
        kind = KINDFILTER_LOBBY;
    else if ( "game" == kindStr )
        kind = KINDFILTER_GAME_IN_PROGRESS;
    else if ( "any" == kindStr )
        kind = KINDFILTER_ANY;
    gameId = config.GetAttributeValueOrChildValueByName( "gameId" ).AsString( gameId, true );
    gameVersion = config.GetAttributeValueOrChildValueByName( "gameVersion" ).AsString( gameVersion, true );
    notFull = config.GetAttributeValueOrChildValueByName( "notFull" ).AsBool( notFull, true );
    noPassword = config.GetAttributeValueOrChildValueByName( "noPassword" ).AsBool( noPassword, true );
    maxPingMs = config.GetAttributeValueOrChildValueByName( "maxPingMs" ).AsInt32( maxPingMs, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGameListingFilter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGameListingFilter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
