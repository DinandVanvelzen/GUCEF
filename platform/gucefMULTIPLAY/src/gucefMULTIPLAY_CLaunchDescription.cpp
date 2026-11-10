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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CLaunchDescription.h"

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

CLaunchMember::CLaunchMember( void )
    : id()
    , name()
    , role( LOBBYMEMBERROLE_PLAYER )
    , slot( -1 )
    , endpoints()
    , publicKey()
    , integrity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLaunchMember::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "id", id );
    node.SetAttribute( "name", name );
    node.SetAttribute( "role", CORE::CString( LobbyMemberRoleToString( role ) ) );
    node.SetAttribute( "slot", slot );
    node.SetValuesOfChildByName( "endpoints", endpoints );
    if ( !publicKey.IsNULLOrEmpty() )
        node.SetAttribute( "publicKey", publicKey );
    if ( integrity.GetKeyCount() > 0 )
        node.SetAttributesOfChildByName( "integrity", integrity );
}

/*-------------------------------------------------------------------------*/

void
CLaunchMember::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    id = node.GetAttributeValueOrChildValueByName( "id" ).AsString( CORE::CString::Empty, true );
    name = node.GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
    role = StringToLobbyMemberRole( node.GetAttributeValueOrChildValueByName( "role" ).AsString( CORE::CString::Empty, true ) );
    slot = node.GetAttributeValueOrChildValueByName( "slot" ).AsInt32( -1, true );
    node.GetValuesOfChildByName( "endpoints", endpoints );
    publicKey = node.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    integrity.Clear();
    node.GetAttributesOfChildByName( "integrity", integrity );
}

/*-------------------------------------------------------------------------*/

CLaunchDescription::CLaunchDescription( void )
    : CORE::CIDataNodeSerializable()
    , gameId()
    , gameVersion()
    , lobbyId()
    , sessionName()
    , topology( GAMETOPOLOGY_CLIENT_SERVER )
    , serverMemberId()
    , sequencerMemberId()
    , members()
    , settings()
    , slots()
    , localMemberId()
    , token()
    , relay()
    , spectator( false )
    , serverPublicKey()
    , securityPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLaunchDescription::CLaunchDescription( const CLaunchDescription& src )
    : CORE::CIDataNodeSerializable( src )
    , gameId( src.gameId )
    , gameVersion( src.gameVersion )
    , lobbyId( src.lobbyId )
    , sessionName( src.sessionName )
    , topology( src.topology )
    , serverMemberId( src.serverMemberId )
    , sequencerMemberId( src.sequencerMemberId )
    , members( src.members )
    , settings( src.settings )
    , slots( src.slots )
    , localMemberId( src.localMemberId )
    , token( src.token )
    , relay( src.relay )
    , spectator( src.spectator )
    , serverPublicKey( src.serverPublicKey )
    , securityPolicy( src.securityPolicy )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLaunchDescription::~CLaunchDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLaunchDescription&
CLaunchDescription::operator=( const CLaunchDescription& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        gameId = src.gameId;
        gameVersion = src.gameVersion;
        lobbyId = src.lobbyId;
        sessionName = src.sessionName;
        topology = src.topology;
        serverMemberId = src.serverMemberId;
        sequencerMemberId = src.sequencerMemberId;
        members = src.members;
        settings = src.settings;
        slots = src.slots;
        localMemberId = src.localMemberId;
        token = src.token;
        relay = src.relay;
        spectator = src.spectator;
        serverPublicKey = src.serverPublicKey;
        securityPolicy = src.securityPolicy;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CLaunchMember*
CLaunchDescription::FindMember( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<members.size(); ++i )
        if ( members[ i ].id == memberId )
            return &members[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::GetHostEndpoints( CORE::CString::StringVector& endpoints ) const
{GUCEF_TRACE;

    const CORE::CString& hostId = GAMETOPOLOGY_PEER_TO_PEER == topology ? sequencerMemberId : serverMemberId;
    const CLaunchMember* host = FindMember( hostId );
    if ( GUCEF_NULL == host )
        return false;
    endpoints.insert( endpoints.end(), host->endpoints.begin(), host->endpoints.end() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::IsLocalMemberHost( void ) const
{GUCEF_TRACE;

    const CORE::CString& hostId = GAMETOPOLOGY_PEER_TO_PEER == topology ? sequencerMemberId : serverMemberId;
    return !localMemberId.IsNULLOrEmpty() && hostId == localMemberId;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::GetRelayBinding( DRN::CDRNEndpoint& relayService, CORE::CString& realm, UInt32& relayPeerId ) const
{GUCEF_TRACE;

    if ( relay.IsNULLOrEmpty() || !relayService.FromString( relay ) )
        return false;
    realm = sessionName;
    relayPeerId = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::GetHostEndpoints( DRN::CDRNEndpointList& endpoints ) const
{GUCEF_TRACE;

    CORE::CString::StringVector hostEndpoints;
    bool found = GetHostEndpoints( hostEndpoints );
    for ( size_t i=0; i<hostEndpoints.size(); ++i )
    {
        DRN::CDRNEndpoint endpoint;
        if ( endpoint.FromString( hostEndpoints[ i ] ) )
            endpoints.Add( endpoint );
    }

    DRN::CDRNEndpoint relayService;
    CORE::CString realm;
    UInt32 relayPeerId = 0;
    if ( GetRelayBinding( relayService, realm, relayPeerId ) )
    {
        DRN::CDRNEndpoint relayed( "relay://" + relayService.GetAddress() + "/" + realm + "/" + CORE::ToString( relayPeerId ) +
                                   "?via=" + DRN::CDRNEndpoint::SchemeForChannelType( relayService.GetChannelType() ) );
        // the link with the relay runs under the relay's policy and pins its key, the game's link over it under the game's
        CORE::CString relayPolicy = relayService.GetParams().GetValueAlways( "policy" ).AsString( CORE::CString::Empty, true );
        CORE::CString relayKey = relayService.GetParams().GetValueAlways( "key" ).AsString( CORE::CString::Empty, true );
        if ( !relayPolicy.IsNULLOrEmpty() )
            relayed.GetParams().Set( "relayPolicy", CORE::CVariant( relayPolicy ) );
        if ( !relayKey.IsNULLOrEmpty() )
            relayed.GetParams().Set( "relayKey", CORE::CVariant( relayKey ) );
        if ( relayed.IsValid() )
            endpoints.Add( relayed );
    }
    return found && !endpoints.IsEmpty();
}

/*-------------------------------------------------------------------------*/

void
CLaunchDescription::MakeSessionSettings( DRN::CDRNSessionSettings& settings ) const
{GUCEF_TRACE;

    settings = DRN::CDRNSessionSettings();
    settings.sessionName = sessionName;
    settings.topology = GAMETOPOLOGY_PEER_TO_PEER == topology ? DRN::DRN_TOPOLOGY_PEER_TO_PEER : DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    settings.maxMembers = (UInt32) members.size();
    settings.securityPolicy = securityPolicy;
}

/*-------------------------------------------------------------------------*/

void
CLaunchDescription::MakeAuthSettings( DRN::CDRNAuthSettings& settings ) const
{GUCEF_TRACE;

    settings = DRN::CDRNAuthSettings();
    settings.authenticatorType = "token";
    settings.SetSetting( "token", CORE::CVariant( token ) );
    settings.SetSetting( "expectedSession", CORE::CVariant( sessionName ) );
    if ( !serverPublicKey.IsNULLOrEmpty() )
        settings.SetSetting( "issuerPublicKey", CORE::CVariant( serverPublicKey ) );
}

/*-------------------------------------------------------------------------*/

void
CLaunchDescription::ApplyToNodeSettings( DRN::CDRNNodeSettings& settings ) const
{GUCEF_TRACE;

    settings.securityPolicy = securityPolicy;
    if ( IsLocalMemberHost() || GAMETOPOLOGY_PEER_TO_PEER == topology )
        MakeAuthSettings( settings.auth );
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::ToJsonString( CORE::CString& json ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LaunchDescription: No JSON codec is available" );
        return false;
    }

    CORE::CDataNode root( "launch", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings serializerSettings;
    if ( !Serialize( root, serializerSettings ) )
        return false;

    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    if ( !codec->StoreDataTree( &root, &access ) )
        return false;

    // The codec writes the text without a terminator
    char terminator = '\0';
    buffer.Append( &terminator, 1 );
    json = CORE::CString( buffer.AsConstTypePtr< char >() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::FromJsonString( const CORE::CString& json )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LaunchDescription: No JSON codec is available" );
        return false;
    }

    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( json.ByteSize() > 0 ? json.ByteSize() - 1 : 0, json.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    CORE::CDataNode root;
    if ( !codec->BuildDataTree( &root, &access ) )
        return false;

    // A codec may give the document as the root or as its only child
    const CORE::CDataNode* node = &root;
    if ( GUCEF_NULL == root.FindChild( "members" ) && 1 == root.GetNrOfChildNodes() )
        node = *root.ConstBegin();

    CORE::CDataNodeSerializableSettings serializerSettings;
    return Deserialize( *node, serializerSettings );
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "gameId", gameId );
    domRootNode.SetAttribute( "gameVersion", gameVersion );
    domRootNode.SetAttribute( "lobbyId", lobbyId );
    domRootNode.SetAttribute( "sessionName", sessionName );
    domRootNode.SetAttribute( "topology", CORE::CString( GameTopologyToString( topology ) ) );
    domRootNode.SetAttribute( "serverMemberId", serverMemberId );
    domRootNode.SetAttribute( "sequencerMemberId", sequencerMemberId );
    domRootNode.SetAttribute( "localMemberId", localMemberId );
    domRootNode.SetAttribute( "token", token );
    domRootNode.SetAttribute( "relay", relay );
    domRootNode.SetAttribute( "spectator", spectator );
    domRootNode.SetAttribute( "serverPublicKey", serverPublicKey );
    securityPolicy.SaveConfig( *domRootNode.AddChild( "securityPolicy", GUCEF_DATATYPE_OBJECT ) );

    CORE::CDataNode* membersNode = domRootNode.AddChild( "members", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<members.size(); ++i )
        members[ i ].Serialize( *membersNode->AddChild( "member", GUCEF_DATATYPE_OBJECT ) );

    CORE::CDataNode* slotsNode = domRootNode.AddChild( "slots", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<slots.size(); ++i )
        slots[ i ].Serialize( *slotsNode->AddChild( "slot", GUCEF_DATATYPE_OBJECT ) );

    domRootNode.SetAttributesOfChildByName( "settings", settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLaunchDescription::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& serializerSettings )
{GUCEF_TRACE;

    gameId = domRootNode.GetAttributeValueOrChildValueByName( "gameId" ).AsString( CORE::CString::Empty, true );
    gameVersion = domRootNode.GetAttributeValueOrChildValueByName( "gameVersion" ).AsString( CORE::CString::Empty, true );
    lobbyId = domRootNode.GetAttributeValueOrChildValueByName( "lobbyId" ).AsString( CORE::CString::Empty, true );
    sessionName = domRootNode.GetAttributeValueOrChildValueByName( "sessionName" ).AsString( CORE::CString::Empty, true );
    topology = StringToGameTopology( domRootNode.GetAttributeValueOrChildValueByName( "topology" ).AsString( CORE::CString::Empty, true ) );
    serverMemberId = domRootNode.GetAttributeValueOrChildValueByName( "serverMemberId" ).AsString( CORE::CString::Empty, true );
    sequencerMemberId = domRootNode.GetAttributeValueOrChildValueByName( "sequencerMemberId" ).AsString( CORE::CString::Empty, true );
    localMemberId = domRootNode.GetAttributeValueOrChildValueByName( "localMemberId" ).AsString( CORE::CString::Empty, true );
    token = domRootNode.GetAttributeValueOrChildValueByName( "token" ).AsString( CORE::CString::Empty, true );
    relay = domRootNode.GetAttributeValueOrChildValueByName( "relay" ).AsString( CORE::CString::Empty, true );
    spectator = domRootNode.GetAttributeValueOrChildValueByName( "spectator" ).AsBool( false, true );
    serverPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "serverPublicKey" ).AsString( CORE::CString::Empty, true );
    securityPolicy = DRN::CDRNSecurityPolicy();
    const CORE::CDataNode* policyNode = domRootNode.FindChild( "securityPolicy" );
    if ( GUCEF_NULL != policyNode && !securityPolicy.LoadConfig( *policyNode ) )
        return false;

    members.clear();
    const CORE::CDataNode* membersNode = domRootNode.FindChild( "members" );
    if ( GUCEF_NULL != membersNode )
    {
        CORE::CDataNode::const_iterator i = membersNode->ConstBegin();
        while ( i != membersNode->ConstEnd() )
        {
            members.push_back( CLaunchMember() );
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

    domRootNode.GetAttributesOfChildByName( "settings", settings );
    settings.SetAllowMultipleValues( false );
    return !lobbyId.IsNULLOrEmpty() || !members.empty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CLaunchDescription::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLaunchDescription( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLaunchDescription::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLaunchDescription";
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
