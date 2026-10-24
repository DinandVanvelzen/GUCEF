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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#include "gucefMULTIPLAY_CGameListingEntry.h"

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

CGameListingEntry::CGameListingEntry( void )
    : CORE::CIDataNodeSerializable()
    , listingId()
    , kind( KIND_LOBBY )
    , name()
    , gameId()
    , gameVersion()
    , mapName()
    , modeName()
    , players( 0 )
    , maxPlayers( 0 )
    , spectators( 0 )
    , passworded( false )
    , hostName()
    , endpoints()
    , backendName()
    , extra()
    , pingMs( -1 )
    , lastSeen()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingEntry::CGameListingEntry( const CGameListingEntry& src )
    : CORE::CIDataNodeSerializable( src )
    , listingId( src.listingId )
    , kind( src.kind )
    , name( src.name )
    , gameId( src.gameId )
    , gameVersion( src.gameVersion )
    , mapName( src.mapName )
    , modeName( src.modeName )
    , players( src.players )
    , maxPlayers( src.maxPlayers )
    , spectators( src.spectators )
    , passworded( src.passworded )
    , hostName( src.hostName )
    , endpoints( src.endpoints )
    , backendName( src.backendName )
    , extra( src.extra )
    , pingMs( src.pingMs )
    , lastSeen( src.lastSeen )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingEntry::~CGameListingEntry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingEntry&
CGameListingEntry::operator=( const CGameListingEntry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        listingId = src.listingId;
        kind = src.kind;
        name = src.name;
        gameId = src.gameId;
        gameVersion = src.gameVersion;
        mapName = src.mapName;
        modeName = src.modeName;
        players = src.players;
        maxPlayers = src.maxPlayers;
        spectators = src.spectators;
        passworded = src.passworded;
        hostName = src.hostName;
        endpoints = src.endpoints;
        backendName = src.backendName;
        extra = src.extra;
        pingMs = src.pingMs;
        lastSeen = src.lastSeen;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::HasSameContent( const CGameListingEntry& other ) const
{GUCEF_TRACE;

    if ( listingId != other.listingId || kind != other.kind || name != other.name || gameId != other.gameId ||
         gameVersion != other.gameVersion || mapName != other.mapName || modeName != other.modeName ||
         players != other.players || maxPlayers != other.maxPlayers || spectators != other.spectators ||
         passworded != other.passworded || hostName != other.hostName || endpoints != other.endpoints ||
         backendName != other.backendName || extra.GetKeyCount() != other.extra.GetKeyCount() )
        return false;

    UInt32 keyCount = extra.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        CORE::CString key = extra.GetKey( i ).AsString();
        if ( !other.extra.HasKey( key ) || other.extra.GetValueAlways( key ).AsString() != extra.GetValueAlways( key ).AsString() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::SharesEndpointWith( const CGameListingEntry& other ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<endpoints.size(); ++i )
        for ( size_t n=0; n<other.endpoints.size(); ++n )
            if ( endpoints[ i ] == other.endpoints[ n ] )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::IsFull( void ) const
{GUCEF_TRACE;

    return maxPlayers > 0 && players >= maxPlayers;
}

/*-------------------------------------------------------------------------*/

const char*
CGameListingEntry::KindToString( TKind kind )
{GUCEF_TRACE;

    return KIND_GAME_IN_PROGRESS == kind ? "game" : "lobby";
}

/*-------------------------------------------------------------------------*/

CGameListingEntry::TKind
CGameListingEntry::StringToKind( const CORE::CString& kind )
{GUCEF_TRACE;

    CORE::CString lower = kind.Lowercase();
    if ( "game" == lower || "in-progress" == lower || "game_in_progress" == lower )
        return KIND_GAME_IN_PROGRESS;
    return KIND_LOBBY;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::ParseEndpoint( const CORE::CString& endpoint ,
                                  CORE::CString& scheme         ,
                                  CORE::CString& host           ,
                                  UInt16& port                  ,
                                  CORE::CString& path           )
{GUCEF_TRACE;

    scheme.Clear();
    host.Clear();
    path.Clear();
    port = 0;

    CORE::CString rest = endpoint;
    Int32 schemeEnd = endpoint.HasSubstr( "://", true );
    if ( schemeEnd >= 0 )
    {
        scheme = endpoint.SubstrToIndex( (UInt32) schemeEnd, true ).Lowercase();
        rest = endpoint.CutChars( (UInt32) schemeEnd + 3, true, 0 );
    }

    Int32 pathStart = rest.HasChar( '/' );
    if ( pathStart >= 0 )
    {
        path = rest.CutChars( (UInt32) pathStart, true, 0 );
        rest = rest.SubstrToIndex( (UInt32) pathStart, true );
    }

    Int32 portStart = rest.HasChar( ':', false );
    if ( portStart <= 0 )
        return false;
    host = rest.SubstrToIndex( (UInt32) portStart, true );
    port = CORE::StringToUInt16( rest.CutChars( (UInt32) portStart + 1, true, 0 ), 0 );
    return 0 != port && !host.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGameListingEntry::MakeEndpoint( const CORE::CString& scheme, const CORE::CString& host, UInt16 port )
{GUCEF_TRACE;

    return scheme + "://" + host + ":" + CORE::UInt16ToString( port );
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "listingId", listingId );
    domRootNode.SetAttribute( "kind", CORE::CString( KindToString( kind ) ) );
    domRootNode.SetAttribute( "name", name );
    domRootNode.SetAttribute( "gameId", gameId );
    domRootNode.SetAttribute( "gameVersion", gameVersion );
    domRootNode.SetAttribute( "mapName", mapName );
    domRootNode.SetAttribute( "modeName", modeName );
    domRootNode.SetAttribute( "players", players );
    domRootNode.SetAttribute( "maxPlayers", maxPlayers );
    domRootNode.SetAttribute( "spectators", spectators );
    domRootNode.SetAttribute( "passworded", passworded );
    domRootNode.SetAttribute( "hostName", hostName );
    domRootNode.SetAttribute( "backendName", backendName );
    domRootNode.SetAttribute( "pingMs", pingMs );
    domRootNode.SetAttribute( "lastSeen", lastSeen.ToIso8601DateTimeString( true, true ) );
    WriteStringsAsChild( domRootNode, "endpoints", endpoints );
    WriteValueListAsChild( domRootNode, "extra", extra );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameListingEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    listingId = domRootNode.GetAttributeValueOrChildValueByName( "listingId" ).AsString( CORE::CString::Empty, true );
    kind = StringToKind( domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ) );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
    gameId = domRootNode.GetAttributeValueOrChildValueByName( "gameId" ).AsString( CORE::CString::Empty, true );
    gameVersion = domRootNode.GetAttributeValueOrChildValueByName( "gameVersion" ).AsString( CORE::CString::Empty, true );
    mapName = domRootNode.GetAttributeValueOrChildValueByName( "mapName" ).AsString( CORE::CString::Empty, true );
    modeName = domRootNode.GetAttributeValueOrChildValueByName( "modeName" ).AsString( CORE::CString::Empty, true );
    players = domRootNode.GetAttributeValueOrChildValueByName( "players" ).AsUInt32( 0, true );
    maxPlayers = domRootNode.GetAttributeValueOrChildValueByName( "maxPlayers" ).AsUInt32( 0, true );
    spectators = domRootNode.GetAttributeValueOrChildValueByName( "spectators" ).AsUInt32( 0, true );
    passworded = domRootNode.GetAttributeValueOrChildValueByName( "passworded" ).AsBool( false, true );
    hostName = domRootNode.GetAttributeValueOrChildValueByName( "hostName" ).AsString( CORE::CString::Empty, true );
    backendName = domRootNode.GetAttributeValueOrChildValueByName( "backendName" ).AsString( CORE::CString::Empty, true );
    pingMs = domRootNode.GetAttributeValueOrChildValueByName( "pingMs" ).AsInt32( -1, true );
    CORE::CString seen = domRootNode.GetAttributeValueOrChildValueByName( "lastSeen" ).AsString( CORE::CString::Empty, true );
    if ( !seen.IsNULLOrEmpty() )
        lastSeen.FromIso8601DateTimeString( seen );
    ReadStringsFromChild( domRootNode, "endpoints", endpoints );
    ReadValueListFromChild( domRootNode, "extra", extra );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CGameListingEntry::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CGameListingEntry( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGameListingEntry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGameListingEntry";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CGameListingList::CGameListingList( void )
    : CORE::CICloneable()
    , listings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingList::CGameListingList( const CGameListingList& src )
    : CORE::CICloneable( src )
    , listings( src.listings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingList::~CGameListingList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameListingList&
CGameListingList::operator=( const CGameListingList& src )
{GUCEF_TRACE;

    if ( &src != this )
        listings = src.listings;
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CGameListingList::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CGameListingList( *this );
}

/*-------------------------------------------------------------------------*/

bool
CGameListingList::TryGetFromFuture( const CORE::CFutureResult& future, CGameListingList& list )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const CGameListingList* data = dynamic_cast< const CGameListingList* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    list = *data;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
