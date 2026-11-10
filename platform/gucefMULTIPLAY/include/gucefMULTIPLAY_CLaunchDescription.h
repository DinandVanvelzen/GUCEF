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

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONSETTINGS_H
#include "gucefDRN_CDRNSessionSettings.h"
#define GUCEF_DRN_CDRNSESSIONSETTINGS_H
#endif /* GUCEF_DRN_CDRNSESSIONSETTINGS_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNNODESETTINGS_H
#include "gucefDRN_CDRNNodeSettings.h"
#define GUCEF_DRN_CDRNNODESETTINGS_H
#endif /* GUCEF_DRN_CDRNNODESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A member of the game as the launch description names it */
class GUCEF_MULTIPLAY_EXPORT_CPP CLaunchMember
{
    public:

    CORE::CString id;
    CORE::CString name;
    TLobbyMemberRole role;
    Int32 slot;                                 /**< -1 for none */
    CORE::CString::StringVector endpoints;      /**< URI strings such as tcp://host:port */
    CORE::CString publicKey;                    /**< the member's identity key in hex, empty when it proved none */
    CORE::CValueList integrity;                 /**< what the member's CIClientIntegrity reported: file -> hash; the game decides on it */

    CLaunchMember( void );

    void Serialize( CORE::CDataNode& node ) const;

    void Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

/**
 *  Everything a member needs to connect to the game the lobby started: who the server is (or the peers), every
 *  member's endpoints, the agreed settings and slots and the token of this member.
 *  The endpoints are URI strings, so a game can use them with DRN or with its own networking.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLaunchDescription : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CLaunchMember > TMemberVector;

    CORE::CString gameId;
    CORE::CString gameVersion;
    CORE::CString lobbyId;
    CORE::CString sessionName;
    TGameTopology topology;
    CORE::CString serverMemberId;               /**< CLIENT_SERVER: the member that serves */
    CORE::CString sequencerMemberId;            /**< PEER_TO_PEER: the member that orders */
    TMemberVector members;
    CORE::CValueList settings;
    CLobbyState::TSlotVector slots;
    CORE::CString localMemberId;
    CORE::CString token;                        /**< signed for the local member */
    CORE::CString relay;                        /**< the relay service of the lobby server (udp://host:port?key=..&policy=..: its key and the hash of its policy), a fallback */
    bool spectator;
    CORE::CString serverPublicKey;              /**< the lobby server's Ed25519 key in hex: the tokens are signed with it, no member can make one */
    DRN::CDRNSecurityPolicy securityPolicy;     /**< the policy of the game, which every node and the session use; the tokens carry its hash */

    CLaunchDescription( void );

    CLaunchDescription( const CLaunchDescription& src );

    virtual ~CLaunchDescription();

    CLaunchDescription& operator=( const CLaunchDescription& src );

    const CLaunchMember* FindMember( const CORE::CString& memberId ) const;

    /**
     *  The endpoints to connect to: those of the server in CLIENT_SERVER, those of the sequencer in PEER_TO_PEER
     *
     *  @return false when the description names no such member
     */
    bool GetHostEndpoints( CORE::CString::StringVector& endpoints ) const;

    /** Whether the local member is the one the others connect to */
    bool IsLocalMemberHost( void ) const;

    /**
     *  The endpoints to join the game session at: those of the host, then the relay endpoint the host binds
     *  under when there is a relay (see GetRelayBinding()). That one carries the relay's policy hash and key as
     *  "relayPolicy" and "relayKey": the link with the relay runs under the relay's policy, the game's over it
     *  under the game's.
     */
    bool GetHostEndpoints( DRN::CDRNEndpointList& endpoints ) const;

    /** The DRN session of the game: its name, topology, size and security policy */
    void MakeSessionSettings( DRN::CDRNSessionSettings& settings ) const;

    /**
     *  The DRN authentication of the local member for the game session: a "token" authenticator with the member's
     *  token, which checks the tokens of others with the server's public key (signed tokens only). The hosting member
     *  uses it as its node's authentication, a joining one passes it to CDRNSession::Join() (and as its node's in
     *  PEER_TO_PEER).
     */
    void MakeAuthSettings( DRN::CDRNAuthSettings& settings ) const;

    /**
     *  Sets the security policy of the game on the settings of a node of the game, and the authentication of
     *  MakeAuthSettings() for a node that others connect to (the host, every node in PEER_TO_PEER)
     */
    void ApplyToNodeSettings( DRN::CDRNNodeSettings& settings ) const;

    /**
     *  Where the host binds so that members who cannot reach it directly join through the relay: the relay service
     *  of the lobby server, the session name as the realm and peer id 1
     *
     *  @return false when there is no relay
     */
    bool GetRelayBinding( DRN::CDRNEndpoint& relayService, CORE::CString& realm, UInt32& relayPeerId ) const;

    /** The description as a JSON text, needs a "json" data store codec */
    bool ToJsonString( CORE::CString& json ) const;

    /** Reads a description from a JSON text, needs a "json" data store codec */
    bool FromJsonString( const CORE::CString& json );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */
