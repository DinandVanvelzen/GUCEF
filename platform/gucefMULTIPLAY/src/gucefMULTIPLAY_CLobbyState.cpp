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

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#include "gucefMULTIPLAY_CLobbyState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PhaseNames[] = { "open", "countdown", "launching", "in-game", "closed" };
const char* RoleNames[] = { "boss", "player", "spectator" };
const char* SlotKindNames[] = { "open", "closed", "human", "ai" };
const char* TopologyNames[] = { "client-server", "peer-to-peer" };

int
FindName( const char* const* names, int count, const CORE::CString& name, int defaultIndex )
{GUCEF_TRACE;

    CORE::CString lower = name.Lowercase();
    for ( int i=0; i<count; ++i )
        if ( lower == names[ i ] )
            return i;
    return defaultIndex;
}

}

/*-------------------------------------------------------------------------*/

const char*
LobbyPhaseToString( TLobbyPhase phase )
{GUCEF_TRACE;

    return PhaseNames[ phase ];
}

/*-------------------------------------------------------------------------*/

TLobbyPhase
StringToLobbyPhase( const CORE::CString& phase )
{GUCEF_TRACE;

    return (TLobbyPhase) FindName( PhaseNames, 5, phase, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
LobbyMemberRoleToString( TLobbyMemberRole role )
{GUCEF_TRACE;

    return RoleNames[ role ];
}

/*-------------------------------------------------------------------------*/

TLobbyMemberRole
StringToLobbyMemberRole( const CORE::CString& role )
{GUCEF_TRACE;

    return (TLobbyMemberRole) FindName( RoleNames, 3, role, 1 );
}

/*-------------------------------------------------------------------------*/

const char*
LobbySlotKindToString( TLobbySlotKind kind )
{GUCEF_TRACE;

    return SlotKindNames[ kind ];
}

/*-------------------------------------------------------------------------*/

TLobbySlotKind
StringToLobbySlotKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return (TLobbySlotKind) FindName( SlotKindNames, 4, kind, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
GameTopologyToString( TGameTopology topology )
{GUCEF_TRACE;

    return TopologyNames[ topology ];
}

/*-------------------------------------------------------------------------*/

TGameTopology
StringToGameTopology( const CORE::CString& topology )
{GUCEF_TRACE;

    return (TGameTopology) FindName( TopologyNames, 2, topology, 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyMember::CLobbyMember( void )
    : id()
    , name()
    , role( LOBBYMEMBERROLE_PLAYER )
    , ready( false )
    , slot( -1 )
    , latencyMs( -1 )
    , joinOrder( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLobbyMember::IsSpectator( void ) const
{GUCEF_TRACE;

    return LOBBYMEMBERROLE_SPECTATOR == role;
}

/*-------------------------------------------------------------------------*/

void
CLobbyMember::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "id", id );
    node.SetAttribute( "name", name );
    node.SetAttribute( "role", CORE::CString( LobbyMemberRoleToString( role ) ) );
    node.SetAttribute( "ready", ready );
    node.SetAttribute( "slot", slot );
    node.SetAttribute( "latencyMs", latencyMs );
    node.SetAttribute( "joinOrder", joinOrder );
}

/*-------------------------------------------------------------------------*/

void
CLobbyMember::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    id = node.GetAttributeValueOrChildValueByName( "id" ).AsString( CORE::CString::Empty, true );
    name = node.GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
    role = StringToLobbyMemberRole( node.GetAttributeValueOrChildValueByName( "role" ).AsString( CORE::CString::Empty, true ) );
    ready = node.GetAttributeValueOrChildValueByName( "ready" ).AsBool( false, true );
    slot = node.GetAttributeValueOrChildValueByName( "slot" ).AsInt32( -1, true );
    latencyMs = node.GetAttributeValueOrChildValueByName( "latencyMs" ).AsInt32( -1, true );
    joinOrder = node.GetAttributeValueOrChildValueByName( "joinOrder" ).AsUInt32( 0, true );
}

/*-------------------------------------------------------------------------*/

CLobbySlot::CLobbySlot( void )
    : index( 0 )
    , team( 0 )
    , faction()
    , colour()
    , kind( LOBBYSLOTKIND_OPEN )
    , aiType()
    , memberId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLobbySlot::IsFree( void ) const
{GUCEF_TRACE;

    return LOBBYSLOTKIND_OPEN == kind && memberId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CLobbySlot::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "index", index );
    node.SetAttribute( "team", team );
    node.SetAttribute( "faction", faction );
    node.SetAttribute( "colour", colour );
    node.SetAttribute( "kind", CORE::CString( LobbySlotKindToString( kind ) ) );
    node.SetAttribute( "aiType", aiType );
    node.SetAttribute( "memberId", memberId );
}

/*-------------------------------------------------------------------------*/

void
CLobbySlot::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    index = node.GetAttributeValueOrChildValueByName( "index" ).AsUInt32( 0, true );
    team = node.GetAttributeValueOrChildValueByName( "team" ).AsUInt32( 0, true );
    faction = node.GetAttributeValueOrChildValueByName( "faction" ).AsString( CORE::CString::Empty, true );
    colour = node.GetAttributeValueOrChildValueByName( "colour" ).AsString( CORE::CString::Empty, true );
    kind = StringToLobbySlotKind( node.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ) );
    aiType = node.GetAttributeValueOrChildValueByName( "aiType" ).AsString( CORE::CString::Empty, true );
    memberId = node.GetAttributeValueOrChildValueByName( "memberId" ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

CLobbyVote::CLobbyVote( void )
    : id()
    , what()
    , proposer()
    , key()
    , value()
    , yes( 0 )
    , no( 0 )
    , needed( 0 )
    , deadline()
    , voters()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLobbyVote::IsActive( void ) const
{GUCEF_TRACE;

    return !id.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CLobbyVote::Clear( void )
{GUCEF_TRACE;

    *this = CLobbyVote();
}

/*-------------------------------------------------------------------------*/

void
CLobbyVote::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "id", id );
    node.SetAttribute( "what", what );
    node.SetAttribute( "proposer", proposer );
    node.SetAttribute( "key", key );
    node.SetAttribute( "value", value );
    node.SetAttribute( "yes", yes );
    node.SetAttribute( "no", no );
    node.SetAttribute( "needed", needed );
    node.SetAttribute( "deadline", deadline.ToIso8601DateTimeString( true, true ) );
}

/*-------------------------------------------------------------------------*/

void
CLobbyVote::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    id = node.GetAttributeValueOrChildValueByName( "id" ).AsString( CORE::CString::Empty, true );
    what = node.GetAttributeValueOrChildValueByName( "what" ).AsString( CORE::CString::Empty, true );
    proposer = node.GetAttributeValueOrChildValueByName( "proposer" ).AsString( CORE::CString::Empty, true );
    key = node.GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true );
    value = node.GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true );
    yes = node.GetAttributeValueOrChildValueByName( "yes" ).AsUInt32( 0, true );
    no = node.GetAttributeValueOrChildValueByName( "no" ).AsUInt32( 0, true );
    needed = node.GetAttributeValueOrChildValueByName( "needed" ).AsUInt32( 0, true );
    CORE::CString deadlineStr = node.GetAttributeValueOrChildValueByName( "deadline" ).AsString( CORE::CString::Empty, true );
    if ( !deadlineStr.IsNULLOrEmpty() )
        deadline.FromIso8601DateTimeString( deadlineStr );
}

/*-------------------------------------------------------------------------*/

CLobbyState::CLobbyState( void )
    : CORE::CIDataNodeSerializable()
    , lobbyId()
    , name()
    , phase( LOBBYPHASE_OPEN )
    , bossId()
    , members()
    , slots()
    , settings()
    , activeVote()
    , chatBackend()
    , chatChannel()
{GUCEF_TRACE;

    settings.SetAllowMultipleValues( false );
}

/*-------------------------------------------------------------------------*/

CLobbyState::CLobbyState( const CLobbyState& src )
    : CORE::CIDataNodeSerializable( src )
    , lobbyId( src.lobbyId )
    , name( src.name )
    , phase( src.phase )
    , bossId( src.bossId )
    , members( src.members )
    , slots( src.slots )
    , settings( src.settings )
    , activeVote( src.activeVote )
    , chatBackend( src.chatBackend )
    , chatChannel( src.chatChannel )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyState::~CLobbyState()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyState&
CLobbyState::operator=( const CLobbyState& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        lobbyId = src.lobbyId;
        name = src.name;
        phase = src.phase;
        bossId = src.bossId;
        members = src.members;
        slots = src.slots;
        settings = src.settings;
        activeVote = src.activeVote;
        chatBackend = src.chatBackend;
        chatChannel = src.chatChannel;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CLobbyMember*
CLobbyState::FindMember( const CORE::CString& memberId )
{GUCEF_TRACE;

    for ( size_t i=0; i<members.size(); ++i )
        if ( members[ i ].id == memberId )
            return &members[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CLobbyMember*
CLobbyState::FindMember( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<members.size(); ++i )
        if ( members[ i ].id == memberId )
            return &members[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CLobbySlot*
CLobbyState::FindSlot( UInt32 index )
{GUCEF_TRACE;

    for ( size_t i=0; i<slots.size(); ++i )
        if ( slots[ i ].index == index )
            return &slots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CLobbySlot*
CLobbyState::FindSlot( UInt32 index ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<slots.size(); ++i )
        if ( slots[ i ].index == index )
            return &slots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CLobbySlot*
CLobbyState::FindSlotOfMember( const CORE::CString& memberId )
{GUCEF_TRACE;

    for ( size_t i=0; i<slots.size(); ++i )
        if ( slots[ i ].memberId == memberId && !memberId.IsNULLOrEmpty() )
            return &slots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CLobbySlot*
CLobbyState::FindSlotOfMember( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<slots.size(); ++i )
        if ( slots[ i ].memberId == memberId && !memberId.IsNULLOrEmpty() )
            return &slots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbyState::GetPlayerCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<members.size(); ++i )
        if ( !members[ i ].IsSpectator() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbyState::GetSpectatorCount( void ) const
{GUCEF_TRACE;

    return (UInt32) members.size() - GetPlayerCount();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyState::GetSetting( const CORE::CString& key ) const
{GUCEF_TRACE;

    return settings.GetValueAlways( key ).AsString();
}

/*-------------------------------------------------------------------------*/

void
CLobbyState::SetSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    settings.SetAllowMultipleValues( false );
    settings.Set( key, value );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyState::Serialize( CORE::CDataNode& domRootNode                        ,
                        const CORE::CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "lobbyId", lobbyId );
    domRootNode.SetAttribute( "name", name );
    domRootNode.SetAttribute( "phase", CORE::CString( LobbyPhaseToString( phase ) ) );
    domRootNode.SetAttribute( "bossId", bossId );
    domRootNode.SetAttribute( "chatBackend", chatBackend );
    domRootNode.SetAttribute( "chatChannel", chatChannel );

    CORE::CDataNode* membersNode = domRootNode.AddChild( "members", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<members.size(); ++i )
        members[ i ].Serialize( *membersNode->AddChild( "member", GUCEF_DATATYPE_OBJECT ) );

    CORE::CDataNode* slotsNode = domRootNode.AddChild( "slots", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<slots.size(); ++i )
        slots[ i ].Serialize( *slotsNode->AddChild( "slot", GUCEF_DATATYPE_OBJECT ) );

    WriteValueListAsChild( domRootNode, "settings", settings );
    activeVote.Serialize( *domRootNode.AddChild( "activeVote", GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyState::Deserialize( const CORE::CDataNode& domRootNode                  ,
                          const CORE::CDataNodeSerializableSettings& serializerSettings )
{GUCEF_TRACE;

    lobbyId = domRootNode.GetAttributeValueOrChildValueByName( "lobbyId" ).AsString( CORE::CString::Empty, true );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
    phase = StringToLobbyPhase( domRootNode.GetAttributeValueOrChildValueByName( "phase" ).AsString( CORE::CString::Empty, true ) );
    bossId = domRootNode.GetAttributeValueOrChildValueByName( "bossId" ).AsString( CORE::CString::Empty, true );
    chatBackend = domRootNode.GetAttributeValueOrChildValueByName( "chatBackend" ).AsString( CORE::CString::Empty, true );
    chatChannel = domRootNode.GetAttributeValueOrChildValueByName( "chatChannel" ).AsString( CORE::CString::Empty, true );

    members.clear();
    const CORE::CDataNode* membersNode = domRootNode.FindChild( "members" );
    if ( GUCEF_NULL != membersNode )
    {
        CORE::CDataNode::const_iterator i = membersNode->ConstBegin();
        while ( i != membersNode->ConstEnd() )
        {
            members.push_back( CLobbyMember() );
            members.back().Deserialize( *(*i) );
            ++i;
        }
    }

    slots.clear();
    const CORE::CDataNode* slotsNode = domRootNode.FindChild( "slots" );
    if ( GUCEF_NULL != slotsNode )
    {
        CORE::CDataNode::const_iterator i = slotsNode->ConstBegin();
        while ( i != slotsNode->ConstEnd() )
        {
            slots.push_back( CLobbySlot() );
            slots.back().Deserialize( *(*i) );
            ++i;
        }
    }

    ReadValueListFromChild( domRootNode, "settings", settings );
    settings.SetAllowMultipleValues( false );

    activeVote.Clear();
    const CORE::CDataNode* voteNode = domRootNode.FindChild( "activeVote" );
    if ( GUCEF_NULL != voteNode )
        activeVote.Deserialize( *voteNode );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CLobbyState::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLobbyState( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyState::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbyState";
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
