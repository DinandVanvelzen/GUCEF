/*
 *  gucefMULTIPLAY_EosTestApp: Application for testing the Epic Online Services plugins of gucefMULTIPLAY, gucefDRN and gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_MULTIPLAY_EOSTESTAPP_TESTFAKEEOS_H
#define GUCEF_MULTIPLAY_EOSTESTAPP_TESTFAKEEOS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#include "authpluginEOS_EosShim.h"
#define GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#endif /* GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  A fake of the Epic Online Services SDK for the tests: a world with lobbies and a P2P network that peers (the
 *  clients of one process) act in. Every peer has function tables for the platform, the lobby interface, the P2P
 *  interface and the auth interface that the plugins fill their shims from.
 *
 *  What it imitates of the SDK:
 *    - requests end asynchronously: the answer comes as an event when the peer is ticked (and after an optional latency),
 *      and what a request does to the world happens when it is made
 *    - lobbies: owner only changes lobby attributes, a member changes its own member attributes, limits on the
 *      number of attributes and their size, permission levels, join by id, invites, kicks, promotion, host migration
 *      (or the lobby closing when it is off), search with a bucket, comparisons on public attributes, minimum free slots
 *      and a number of results, notifications to the other members
 *    - P2P: a connection is opened by the first packet and has to be accepted by the other side, packets of at most
 *      1170 bytes, reliable and unreliable modes, loss, duplicates and reordering that the test sets, a direct or a
 *      relayed connection (and no connection when the relay is not allowed and direct is blocked), closing, timeouts
 *    - platform: creation that can fail, ticks, release
 *  What it does not: any real network, any real account, EOS's own rate limits.
 */

namespace EosFake {

class CFakeEosWorld;
class CFakeEosPeer;

/*-------------------------------------------------------------------------*/

/** What the next request of a kind ends with instead of working, for testing the errors */
struct SFailures
{
    GUCEF::PLUGINGLUE::EOS::EEosResult create;
    GUCEF::PLUGINGLUE::EOS::EEosResult update;
    GUCEF::PLUGINGLUE::EOS::EEosResult join;
    GUCEF::PLUGINGLUE::EOS::EEosResult search;
    GUCEF::PLUGINGLUE::EOS::EEosResult kick;
    GUCEF::PLUGINGLUE::EOS::EEosResult promote;
    GUCEF::UInt32 updateCount;      /**< how many updates fail with update, 0 for all of them while it is set */
    bool neverAnswer;               /**< requests are taken but never ended */
    bool noConnection;              /**< every request ends with no connection */

    SFailures( void );
};

/*-------------------------------------------------------------------------*/

struct SFakeMember
{
    GUCEF::CORE::CString puid;
    GUCEF::PLUGINGLUE::EOS::TEosAttributeVector attributes;
};

struct SFakeLobby
{
    GUCEF::CORE::CString id;
    GUCEF::CORE::CString owner;
    GUCEF::CORE::CString bucketId;
    GUCEF::UInt32 maxMembers;
    GUCEF::PLUGINGLUE::EOS::EEosLobbyPermission permission;
    bool allowInvites;
    bool enableJoinById;
    bool disableHostMigration;
    bool rejoinAfterKickRequiresInvite;
    GUCEF::PLUGINGLUE::EOS::TEosAttributeVector attributes;
    GUCEF::vector< SFakeMember > members;
    GUCEF::CORE::CString::StringSet kicked;
    GUCEF::CORE::CString::StringSet invited;

    SFakeLobby( void );
};

/*-------------------------------------------------------------------------*/

struct SFakePacket
{
    GUCEF::UInt64 dueInMs;
    GUCEF::CORE::CString from;
    GUCEF::CORE::CString socketName;
    GUCEF::UInt8 channel;
    GUCEF::vector< GUCEF::UInt8 > data;
    SFakePacket( void ) : dueInMs( 0 ), from(), socketName(), channel( 0 ), data() {}
};

/** A connection between two users on a socket id */
struct SConnection
{
    GUCEF::CORE::CString initiator;
    GUCEF::CORE::CString other;
    GUCEF::CORE::CString socketName;
    bool established;
    bool relayed;
    GUCEF::UInt64 requestedInMs;
    GUCEF::UInt64 lastDueToOther;
    GUCEF::UInt64 lastDueToInitiator;
    GUCEF::vector< SFakePacket > pending;

    SConnection( void ) : initiator(), other(), socketName(), established( false ), relayed( false ), requestedInMs( 0 ), lastDueToOther( 0 ), lastDueToInitiator( 0 ), pending() {}
};

/** The network between the peers */
struct SNetworkModel
{
    double lossRate;                /**< share of the unreliable packets that never arrive */
    double duplicateRate;
    double reorderRate;             /**< share of the unreliable packets held back for up to reorderMs */
    GUCEF::UInt32 latencyMs;
    GUCEF::UInt32 reorderMs;
    GUCEF::UInt32 relayExtraMs;
    bool directBlocked;             /**< no direct connection is possible: only through the relay */
    GUCEF::UInt32 pendingTimeoutMs; /**< a connection that is not accepted by then times out */

    SNetworkModel( void );
};

/*-------------------------------------------------------------------------*/

class CFakeEosWorld
{
    public:

    // the limits of the lobby interface
    GUCEF::UInt32 maxAttributes;
    GUCEF::UInt32 maxAttributeLength;
    GUCEF::UInt32 maxKeyLength;
    GUCEF::UInt32 maxMembers;

    SNetworkModel network;

    /** The number of connections that are there */
    GUCEF::UInt32 GetConnectionCount( void ) const;
    GUCEF::UInt32 GetEstablishedConnectionCount( void ) const;

    // what happened
    GUCEF::UInt32 packetsSent;
    GUCEF::UInt32 packetsDelivered;
    GUCEF::UInt32 packetsDropped;
    GUCEF::UInt32 packetsDuplicated;
    GUCEF::UInt32 relayedConnections;
    GUCEF::UInt32 directConnections;
    GUCEF::UInt32 refusedConnections;
    GUCEF::UInt32 largestPacket;

    CFakeEosWorld( void );

    void AddPeer( CFakeEosPeer* peer );
    void RemovePeer( CFakeEosPeer* peer );
    CFakeEosPeer* FindPeer( const GUCEF::CORE::CString& puid ) const;

    SFakeLobby* FindLobby( const GUCEF::CORE::CString& lobbyId );
    GUCEF::UInt32 GetLobbyCount( void ) const;
    GUCEF::CORE::CString::StringVector GetLobbyIds( void ) const;

    /** Sets or replaces a lobby attribute without asking who may: a backend that forgets to check, or a forged client */
    bool ForgeLobbyAttribute( const GUCEF::CORE::CString& lobbyId, const GUCEF::PLUGINGLUE::EOS::SEosAttribute& attribute );

    /** Sets a member attribute of another member's, the same thing */
    bool ForgeMemberAttribute( const GUCEF::CORE::CString& lobbyId, const GUCEF::CORE::CString& puid, const GUCEF::PLUGINGLUE::EOS::SEosAttribute& attribute );

    GUCEF::UInt32 NextRandom( void );
    double NextRandomUnit( void );

    private:

    friend class CFakeEosPeer;

    GUCEF::vector< SConnection > m_connections;
    GUCEF::vector< CFakeEosPeer* > m_peers;
    GUCEF::map< GUCEF::CORE::CString, SFakeLobby > m_lobbies;
    GUCEF::UInt32 m_nextLobby;
    GUCEF::UInt32 m_random;
};

/*-------------------------------------------------------------------------*/

class CFakeEosPeer
{
    public:

    CFakeEosWorld& world;
    GUCEF::CORE::CString puid;
    GUCEF::CORE::CString epicAccountId;

    // the platform
    bool sdkAvailable;
    bool platformOk;
    GUCEF::CORE::CString platformError;
    bool platformReady;
    GUCEF::UInt32 createPlatformCalls;
    GUCEF::UInt32 releaseCalls;
    GUCEF::UInt32 tickCalls;
    GUCEF::CORE::CString configProductId;
    GUCEF::CORE::CString configClientSecret;

    // the behaviour
    SFailures failures;
    GUCEF::UInt32 latencyMs;
    bool echoToUpdater;
    GUCEF::PLUGINGLUE::EOS::EEosRelayControl relayControl;

    // what was asked
    GUCEF::UInt32 updateCalls;
    GUCEF::UInt32 searchCalls;
    GUCEF::UInt32 createCalls;
    GUCEF::UInt32 joinCalls;
    GUCEF::UInt32 leaveCalls;
    GUCEF::UInt32 destroyCalls;
    GUCEF::UInt32 kickCalls;
    GUCEF::UInt32 promoteCalls;
    GUCEF::UInt32 inviteCalls;
    GUCEF::UInt32 largestUpdateAttributes;
    GUCEF::UInt32 packetsSent;
    GUCEF::UInt32 packetsReceived;
    GUCEF::UInt32 acceptCalls;
    GUCEF::UInt32 closeCalls;
    GUCEF::CORE::CString lastSearchBucket;
    GUCEF::PLUGINGLUE::EOS::SEosLobbySearchOptions lastSearch;
    GUCEF::PLUGINGLUE::EOS::SEosLobbyCreateOptions lastCreate;

    CFakeEosPeer( CFakeEosWorld& world, const GUCEF::CORE::CString& puid, const GUCEF::CORE::CString& epicAccountId );

    ~CFakeEosPeer();

    GUCEF::PLUGINGLUE::EOS::SEosPlatformShim PlatformShim( void );
    GUCEF::PLUGINGLUE::EOS::SEosLobbyShim LobbyShim( void );
    GUCEF::PLUGINGLUE::EOS::SEosP2PShim P2PShim( void );
    GUCEF::AUTHPLUGIN::EOS::TEosShim AuthShim( void );

    /** The number of requests that were taken and have not ended */
    GUCEF::UInt32 GetPendingRequestCount( void ) const;

    /** Everything this user does with EOS ends: its requests end with no connection, its lobbies are left */
    void DropOffTheNetwork( void );

    private:

    friend class CFakeEosWorld;

    struct SLobbyEntry
    {
        GUCEF::UInt64 dueInMs;
        GUCEF::PLUGINGLUE::EOS::SEosLobbyEvent event;
    };
    struct SP2PEntry
    {
        GUCEF::UInt64 dueInMs;
        GUCEF::PLUGINGLUE::EOS::SEosP2PEvent event;
    };
    struct SAuthEntry
    {
        GUCEF::UInt64 dueInMs;
        GUCEF::AUTHPLUGIN::EOS::SEosEvent event;
    };

    // the platform functions
    static bool IsSdkAvailable( void* context );
    static bool CreatePlatform( void* context, const GUCEF::PLUGINGLUE::EOS::SEosPlatformConfig* config, char* errorText, GUCEF::UInt32 errorTextSize );
    static void ReleasePlatform( void* context );
    static bool IsPlatformReady( void* context );
    static void Tick( void* context );

    // the lobby functions
    static bool CreateLobby( void* context, GUCEF::UInt32 requestId, const GUCEF::PLUGINGLUE::EOS::SEosLobbyCreateOptions* options );
    static bool UpdateLobby( void* context, GUCEF::UInt32 requestId, const GUCEF::PLUGINGLUE::EOS::SEosLobbyUpdateOptions* options );
    static bool JoinLobbyById( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser, bool presenceEnabled );
    static bool LeaveLobby( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser );
    static bool DestroyLobby( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser );
    static bool KickMember( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    static bool PromoteMember( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    static bool SendInvite( void* context, GUCEF::UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    static bool SearchLobbies( void* context, GUCEF::UInt32 requestId, const GUCEF::PLUGINGLUE::EOS::SEosLobbySearchOptions* options );
    static bool CopyLobbyDetails( void* context, const char* lobbyId, const char* localUser, GUCEF::PLUGINGLUE::EOS::SEosLobbyDetails* details );
    static bool PollLobbyEvent( void* context, GUCEF::PLUGINGLUE::EOS::SEosLobbyEvent* event );

    // the P2P functions
    static GUCEF::PLUGINGLUE::EOS::EEosResult SendPacket( void* context, const char* localUser, const char* remoteUser, const char* socketName, GUCEF::UInt8 channel,
                                                          const void* data, GUCEF::UInt32 size, GUCEF::PLUGINGLUE::EOS::EEosPacketReliability reliability, bool allowDelayedDelivery );
    static bool ReceivePacket( void* context, const char* localUser, GUCEF::UInt32 bufferSize, void* buffer, GUCEF::UInt32* size, char* remoteUser, char* socketName, GUCEF::UInt8* channel );
    static GUCEF::PLUGINGLUE::EOS::EEosResult AcceptConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName );
    static GUCEF::PLUGINGLUE::EOS::EEosResult CloseConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName );
    static GUCEF::PLUGINGLUE::EOS::EEosResult CloseConnections( void* context, const char* localUser, const char* socketName );
    static bool PollP2PEvent( void* context, GUCEF::PLUGINGLUE::EOS::SEosP2PEvent* event );
    static GUCEF::PLUGINGLUE::EOS::EEosResult SetRelayControl( void* context, GUCEF::PLUGINGLUE::EOS::EEosRelayControl control );
    static GUCEF::PLUGINGLUE::EOS::EEosNatType GetNatType( void* context );
    static GUCEF::PLUGINGLUE::EOS::EEosResult SetPacketQueueSize( void* context, GUCEF::UInt64 incoming, GUCEF::UInt64 outgoing );
    static void SetLocalUser( void* context, const char* localUser );

    // the auth functions
    static bool PollAuthEvent( void* context, GUCEF::AUTHPLUGIN::EOS::SEosEvent* event );
    static bool AuthLogin( void* context, GUCEF::UInt32 requestId, GUCEF::AUTHPLUGIN::EOS::EEosLoginType type );
    static bool AuthCopyIdToken( void* context, const char* epicAccountId, char* buffer, GUCEF::UInt32 bufferSize );
    static bool ConnectLogin( void* context, GUCEF::UInt32 requestId, const char* idToken );
    static bool ConnectCreateUser( void* context, GUCEF::UInt32 requestId );
    static bool ConnectLinkAccount( void* context, GUCEF::UInt32 requestId, const char* productUserId );
    static bool AuthLogout( void* context, GUCEF::UInt32 requestId, const char* epicAccountId, bool deletePersistentAuth );

    static CFakeEosPeer* Self( void* context ) { return static_cast< CFakeEosPeer* >( context ); }

    bool Preempt( GUCEF::UInt32 requestId, GUCEF::PLUGINGLUE::EOS::EEosLobbyEventKind kind, const GUCEF::CORE::CString& lobbyId, GUCEF::PLUGINGLUE::EOS::EEosResult overrideResult );
    void QueueEnd( GUCEF::UInt32 requestId, GUCEF::PLUGINGLUE::EOS::EEosLobbyEventKind kind, GUCEF::PLUGINGLUE::EOS::EEosResult result, const GUCEF::CORE::CString& lobbyId );
    void QueueLobbyEvent( const GUCEF::PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void NotifyMembers( SFakeLobby& lobby, const GUCEF::CORE::CString& except, GUCEF::PLUGINGLUE::EOS::SEosLobbyEvent event );
    void QueueP2PEvent( const GUCEF::PLUGINGLUE::EOS::SEosP2PEvent& event );
    void QueuePacket( CFakeEosPeer& to, SConnection& connection, GUCEF::UInt8 channel, const void* data, GUCEF::UInt32 size, GUCEF::PLUGINGLUE::EOS::EEosPacketReliability reliability );
    void RemoveMember( SFakeLobby& lobby, const GUCEF::CORE::CString& puid, GUCEF::PLUGINGLUE::EOS::EEosMemberStatus reason );
    void DestroyLobbyNow( SFakeLobby& lobby );
    void FillDetails( const SFakeLobby& lobby, GUCEF::PLUGINGLUE::EOS::SEosLobbyDetails& details, bool withMemberAttributes ) const;
    bool ApplyAttributes( GUCEF::PLUGINGLUE::EOS::TEosAttributeVector& target, const GUCEF::PLUGINGLUE::EOS::TEosAttributeVector& set, const GUCEF::CORE::CString::StringVector& remove, GUCEF::PLUGINGLUE::EOS::EEosResult& result );
    void ExpireConnections( void );
    GUCEF::UInt64 Now( void ) const;

    GUCEF::vector< SLobbyEntry > m_lobbyEvents;
    GUCEF::vector< SP2PEntry > m_p2pEvents;
    GUCEF::vector< SFakePacket > m_inbox;
    GUCEF::vector< SAuthEntry > m_authEvents;
    GUCEF::UInt32 m_neverAnswered;
    GUCEF::UInt32 m_failedUpdates;
};

/*-------------------------------------------------------------------------*/

}; /* namespace EosFake */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_EOSTESTAPP_TESTFAKEEOS_H ? */
