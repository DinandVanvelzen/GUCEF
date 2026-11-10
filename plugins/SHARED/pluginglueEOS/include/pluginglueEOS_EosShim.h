/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_MACROS_H
#include "pluginglueEOS_macros.h"
#define GUCEF_PLUGINGLUE_EOS_MACROS_H
#endif /* GUCEF_PLUGINGLUE_EOS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE FUNCTION TABLES                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Everything of Epic Online Services that the plugins use goes through the function tables of this file. The EOS SDK
 *  is not part of this repository and the plugins build without it: when the build is given the SDK (the compiler
 *  flag GUCEF_AUTH_HAS_EOS and the include directory of the SDK, see CEosPlatformSdk) the real tables are filled from
 *  it, the tests fill them with a fake SDK. A function a table does not have is NULL. None of the functions blocks:
 *  the SDK ends an operation with a callback that runs while EOS_Platform_Tick runs, and the binding queues it as an
 *  event that the owner of the table polls. The names of the SDK in the comments are written from the documentation
 *  of the SDK and are marked "to confirm" where the name or the value could not be checked.
 */

/** How an operation ended. The real binding translates the EOS_EResult of the SDK, by the names of the SDK headers */
enum EEosResult
{
    EOSRESULT_SUCCESS = 0,          /**< EOS_Success */
    EOSRESULT_CANCELLED,            /**< EOS_Canceled */
    EOSRESULT_NOT_FOUND,            /**< EOS_NotFound: the lobby is gone, the user is not in it, no packet waits */
    EOSRESULT_NO_PERMISSION,        /**< EOS_AccessDenied, EOS_Lobby_NotOwner, EOS_Lobby_NoPermission (to confirm) */
    EOSRESULT_INVALID_PARAMETERS,   /**< EOS_InvalidParameters, EOS_InvalidUser */
    EOSRESULT_LOBBY_FULL,           /**< EOS_Lobby_TooManyPlayers (to confirm) */
    EOSRESULT_LIMIT_EXCEEDED,       /**< EOS_LimitExceeded: too many attributes, a value that is too long, a packet that is too big (to confirm) */
    EOSRESULT_NO_CONNECTION,        /**< EOS_NoConnection, EOS_NetworkDisconnected */
    EOSRESULT_TIMEOUT,              /**< EOS_TimedOut */
    EOSRESULT_ALREADY_PENDING,      /**< EOS_AlreadyPending, EOS_TooManyRequests */
    EOSRESULT_NOT_LOGGED_IN,        /**< EOS_InvalidUser for a user that is not logged in */
    EOSRESULT_FAILED                /**< anything else */
};

/** The text of a result for logs */
GUCEF_PLUGINGLUE_EOS_EXPORT_CPP const char* EosResultToString( EEosResult result );

/*-------------------------------------------------------------------------*/

/** What EOS_Platform_Create needs; the pointers are valid for the duration of the call only */
struct SEosPlatformConfig
{
    const char* productName;
    const char* productVersion;
    const char* productId;
    const char* sandboxId;
    const char* deploymentId;
    const char* clientId;
    const char* clientSecret;
    const char* encryptionKey;      /**< 64 hex digits, may be empty */
    const char* cacheDirectory;     /**< may be empty */

    SEosPlatformConfig( void )
        : productName( GUCEF_NULL ), productVersion( GUCEF_NULL ), productId( GUCEF_NULL ), sandboxId( GUCEF_NULL ), deploymentId( GUCEF_NULL )
        , clientId( GUCEF_NULL ), clientSecret( GUCEF_NULL ), encryptionKey( GUCEF_NULL ), cacheDirectory( GUCEF_NULL )
    {}
};
typedef struct SEosPlatformConfig TEosPlatformConfig;

/**
 *  The platform: EOS_Initialize, EOS_Platform_Create, EOS_Platform_Tick, EOS_Platform_Release. Owned by the glue
 *  (CEosGlue), which creates it once for everyone who needs it.
 */
struct SEosPlatformShim
{
    void* context;

    /** Whether the SDK is there to use: compiled in and loadable */
    bool ( *isSdkAvailable )( void* context );

    /** EOS_Initialize + EOS_Platform_Create. Repeated calls are fine. errorText says why it failed */
    bool ( *createPlatform )( void* context, const SEosPlatformConfig* config, char* errorText, UInt32 errorTextSize );

    /** EOS_Platform_Release + EOS_Shutdown, for the platform that createPlatform made */
    void ( *releasePlatform )( void* context );

    bool ( *isPlatformReady )( void* context );

    /** EOS_Platform_Tick: the callbacks of the SDK run inside it and are queued as events */
    void ( *tick )( void* context );

    /** The EOS_HPlatform behind the table, for the bindings of the other interfaces; NULL for a fake */
    void* ( *getPlatformHandle )( void* context );

    SEosPlatformShim( void )
        : context( GUCEF_NULL ), isSdkAvailable( GUCEF_NULL ), createPlatform( GUCEF_NULL ), releasePlatform( GUCEF_NULL )
        , isPlatformReady( GUCEF_NULL ), tick( GUCEF_NULL ), getPlatformHandle( GUCEF_NULL )
    {}

    /** Whether the table can make a platform */
    bool CanCreatePlatform( void ) const { return GUCEF_NULL != createPlatform; }

    /** Whether two tables are for the same platform */
    bool IsSamePlatform( const SEosPlatformShim& other ) const { return context == other.context && createPlatform == other.createPlatform; }
};
typedef struct SEosPlatformShim TEosPlatformShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      LOBBY INTERFACE                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The limits of the lobby interface as assumed by default (EOS_LOBBY_MAX_LOBBY_MEMBERS, EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES, EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH: all to confirm) */
static const UInt32 EosDefaultMaxLobbyMembers = 64;
static const UInt32 EosDefaultMaxLobbyAttributes = 64;
static const UInt32 EosDefaultMaxAttributeLength = 1000;
static const UInt32 EosDefaultMaxAttributeKeyLength = 64;
static const UInt32 EosDefaultMaxSearchResults = 200;

/** EOS_ELobbyPermissionLevel */
enum EEosLobbyPermission
{
    EOSLOBBYPERM_PUBLIC_ADVERTISED = 0,     /**< EOS_LPL_PUBLICADVERTISED: found by search, joined by id */
    EOSLOBBYPERM_JOIN_VIA_PRESENCE,         /**< EOS_LPL_JOINVIAPRESENCE: found by search of members of a friend's presence, joined through presence */
    EOSLOBBYPERM_INVITE_ONLY                /**< EOS_LPL_INVITEONLY: only with an invite */
};

/** EOS_ELobbyMemberStatus */
enum EEosMemberStatus
{
    EOSMEMBERSTATUS_JOINED = 0,
    EOSMEMBERSTATUS_LEFT,
    EOSMEMBERSTATUS_DISCONNECTED,
    EOSMEMBERSTATUS_KICKED,
    EOSMEMBERSTATUS_PROMOTED,               /**< the user became the owner of the lobby */
    EOSMEMBERSTATUS_CLOSED                  /**< the lobby was destroyed */
};

/** EOS_ESessionAttributeType (the lobby attributes use the session attribute type; to confirm) */
enum EEosAttributeType
{
    EOSATTRIBUTE_STRING = 0,
    EOSATTRIBUTE_INT64
};

/** EOS_ELobbyAttributeVisibility */
enum EEosAttributeVisibility
{
    EOSVISIBILITY_PUBLIC = 0,               /**< EOS_LAT_PUBLIC: seen by search and by members */
    EOSVISIBILITY_PRIVATE                   /**< EOS_LAT_PRIVATE: seen by members only */
};

/** EOS_EComparisonOp */
enum EEosComparison
{
    EOSCOMPARE_EQUAL = 0,                   /**< EOS_CO_EQUAL */
    EOSCOMPARE_NOT_EQUAL,
    EOSCOMPARE_GREATER_THAN,
    EOSCOMPARE_GREATER_THAN_OR_EQUAL,
    EOSCOMPARE_LESS_THAN,
    EOSCOMPARE_LESS_THAN_OR_EQUAL
};

/** An attribute of a lobby or of a member of it */
struct SEosAttribute
{
    GUCEF::CORE::CString key;
    EEosAttributeType type;
    GUCEF::CORE::CString stringValue;
    Int64 intValue;
    EEosAttributeVisibility visibility;

    SEosAttribute( void ) : key(), type( EOSATTRIBUTE_STRING ), stringValue(), intValue( 0 ), visibility( EOSVISIBILITY_PUBLIC ) {}
};
typedef GUCEF::vector< SEosAttribute > TEosAttributeVector;

/** A member of a lobby: the Product User Id and its member attributes */
struct SEosLobbyMember
{
    GUCEF::CORE::CString puid;
    TEosAttributeVector attributes;
};
typedef GUCEF::vector< SEosLobbyMember > TEosLobbyMemberVector;

/** A lobby as the SDK shows it: EOS_LobbyDetails_CopyInfo, the attributes and the members */
struct SEosLobbyDetails
{
    GUCEF::CORE::CString lobbyId;
    GUCEF::CORE::CString ownerPuid;
    GUCEF::CORE::CString bucketId;
    UInt32 maxMembers;
    UInt32 availableSlots;
    EEosLobbyPermission permission;
    bool allowInvites;
    TEosAttributeVector attributes;
    TEosLobbyMemberVector members;

    SEosLobbyDetails( void ) : lobbyId(), ownerPuid(), bucketId(), maxMembers( 0 ), availableSlots( 0 ), permission( EOSLOBBYPERM_PUBLIC_ADVERTISED ), allowInvites( true ), attributes(), members() {}
};
typedef GUCEF::vector< SEosLobbyDetails > TEosLobbyDetailsVector;

/** EOS_Lobby_CreateLobbyOptions: a lobby is made without attributes, they follow with an update */
struct SEosLobbyCreateOptions
{
    GUCEF::CORE::CString localUser;
    UInt32 maxMembers;
    EEosLobbyPermission permission;
    bool presenceEnabled;                   /**< bPresenceEnabled */
    bool allowInvites;                      /**< bAllowInvites */
    GUCEF::CORE::CString bucketId;
    bool disableHostMigration;              /**< bDisableHostMigration */
    bool enableJoinById;                    /**< bEnableJoinById (to confirm) */
    bool rejoinAfterKickRequiresInvite;     /**< bRejoinAfterKickRequiresInvite (to confirm) */

    SEosLobbyCreateOptions( void )
        : localUser(), maxMembers( 4 ), permission( EOSLOBBYPERM_PUBLIC_ADVERTISED ), presenceEnabled( false ), allowInvites( true )
        , bucketId(), disableHostMigration( false ), enableJoinById( true ), rejoinAfterKickRequiresInvite( true )
    {}
};

/**
 *  The changes of one EOS_Lobby_UpdateLobby: the lobby modification handle (EOS_Lobby_UpdateLobbyModification) with
 *  EOS_LobbyModification_SetPermissionLevel, SetMaxMembers, SetInvitesAllowed, AddAttribute, RemoveAttribute,
 *  AddMemberAttribute and RemoveMemberAttribute, applied by one update, which the SDK applies at once.
 */
struct SEosLobbyUpdateOptions
{
    GUCEF::CORE::CString lobbyId;
    GUCEF::CORE::CString localUser;
    bool setPermission;
    EEosLobbyPermission permission;
    bool setMaxMembers;
    UInt32 maxMembers;
    bool setAllowInvites;
    bool allowInvites;
    TEosAttributeVector setAttributes;
    GUCEF::CORE::CString::StringVector removeAttributes;
    TEosAttributeVector setMemberAttributes;
    GUCEF::CORE::CString::StringVector removeMemberAttributes;

    SEosLobbyUpdateOptions( void )
        : lobbyId(), localUser(), setPermission( false ), permission( EOSLOBBYPERM_PUBLIC_ADVERTISED ), setMaxMembers( false ), maxMembers( 0 )
        , setAllowInvites( false ), allowInvites( true ), setAttributes(), removeAttributes(), setMemberAttributes(), removeMemberAttributes()
    {}
};

/** A condition of a lobby search: EOS_LobbySearch_SetParameter */
struct SEosLobbySearchParameter
{
    GUCEF::CORE::CString key;
    EEosComparison comparison;
    EEosAttributeType type;
    GUCEF::CORE::CString stringValue;
    Int64 intValue;

    SEosLobbySearchParameter( void ) : key(), comparison( EOSCOMPARE_EQUAL ), type( EOSATTRIBUTE_STRING ), stringValue(), intValue( 0 ) {}
};
typedef GUCEF::vector< SEosLobbySearchParameter > TEosSearchParameterVector;

/** A lobby search: EOS_Lobby_CreateLobbySearch with EOS_LobbySearch_SetMaxResults, SetLobbyId, SetTargetUserId and SetParameter */
struct SEosLobbySearchOptions
{
    GUCEF::CORE::CString localUser;
    UInt32 maxResults;
    GUCEF::CORE::CString lobbyId;           /**< EOS_LobbySearch_SetLobbyId: a search for one lobby */
    GUCEF::CORE::CString targetUser;        /**< EOS_LobbySearch_SetTargetUserId: the lobbies a user is in */
    GUCEF::CORE::CString bucketId;          /**< the search parameter EOS_LOBBY_SEARCH_BUCKET_ID (to confirm) */
    Int32 minSlotsAvailable;                /**< the search parameter EOS_LOBBY_SEARCH_MINSLOTSAVAILABLE (to confirm), -1 for none */
    TEosSearchParameterVector parameters;

    SEosLobbySearchOptions( void ) : localUser(), maxResults( 50 ), lobbyId(), targetUser(), bucketId(), minSlotsAvailable( -1 ), parameters() {}
};

enum EEosLobbyEventKind
{
    EOSLOBBYEVENT_CREATE = 0,               /**< the end of EOS_Lobby_CreateLobby: lobbyId */
    EOSLOBBYEVENT_UPDATE,                   /**< the end of EOS_Lobby_UpdateLobby */
    EOSLOBBYEVENT_JOIN,                     /**< the end of EOS_Lobby_JoinLobbyById: lobbyId */
    EOSLOBBYEVENT_LEAVE,                    /**< the end of EOS_Lobby_LeaveLobby */
    EOSLOBBYEVENT_DESTROY,                  /**< the end of EOS_Lobby_DestroyLobby */
    EOSLOBBYEVENT_KICK,                     /**< the end of EOS_Lobby_KickMember */
    EOSLOBBYEVENT_PROMOTE,                  /**< the end of EOS_Lobby_PromoteMember */
    EOSLOBBYEVENT_SEND_INVITE,              /**< the end of EOS_Lobby_SendInvite */
    EOSLOBBYEVENT_SEARCH,                   /**< the end of EOS_LobbySearch_Find: found */
    EOSLOBBYEVENT_LOBBY_UPDATED,            /**< EOS_Lobby_AddNotifyLobbyUpdateReceived: lobbyId, the attributes or the lobby settings changed */
    EOSLOBBYEVENT_MEMBER_UPDATED,           /**< EOS_Lobby_AddNotifyLobbyMemberUpdateReceived: lobbyId, userId */
    EOSLOBBYEVENT_MEMBER_STATUS,            /**< EOS_Lobby_AddNotifyLobbyMemberStatusReceived: lobbyId, userId, status */
    EOSLOBBYEVENT_INVITE_RECEIVED           /**< EOS_Lobby_AddNotifyLobbyInviteReceived: lobbyId, senderId */
};

/** An event that the SDK delivered while it was ticked */
struct SEosLobbyEvent
{
    UInt32 requestId;                       /**< of the request that ended, 0 for notifications */
    EEosLobbyEventKind kind;
    EEosResult result;
    Int32 sdkResultCode;                    /**< EOS_EResult as it came, for the log */
    GUCEF::CORE::CString lobbyId;
    GUCEF::CORE::CString userId;
    GUCEF::CORE::CString senderId;
    EEosMemberStatus status;
    TEosLobbyDetailsVector found;           /**< EOSLOBBYEVENT_SEARCH */

    SEosLobbyEvent( void ) : requestId( 0 ), kind( EOSLOBBYEVENT_CREATE ), result( EOSRESULT_FAILED ), sdkResultCode( 0 ), lobbyId(), userId(), senderId(), status( EOSMEMBERSTATUS_JOINED ), found() {}
};

/**
 *  The lobby interface of the SDK (EOS_HLobby of EOS_Platform_GetLobbyInterface). The Product User Ids are given as the
 *  text of EOS_ProductUserId_ToString.
 */
struct SEosLobbyShim
{
    SEosPlatformShim platform;
    void* context;

    bool ( *createLobby )( void* context, UInt32 requestId, const SEosLobbyCreateOptions* options );
    bool ( *updateLobby )( void* context, UInt32 requestId, const SEosLobbyUpdateOptions* options );
    bool ( *joinLobbyById )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, bool presenceEnabled );
    bool ( *leaveLobby )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser );
    bool ( *destroyLobby )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser );
    bool ( *kickMember )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    bool ( *promoteMember )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    bool ( *sendInvite )( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser );
    bool ( *searchLobbies )( void* context, UInt32 requestId, const SEosLobbySearchOptions* options );

    /** EOS_Lobby_CopyLobbyDetailsHandle + the EOS_LobbyDetails_* functions: the lobby the local user is in, as it is now */
    bool ( *copyLobbyDetails )( void* context, const char* lobbyId, const char* localUser, SEosLobbyDetails* details );

    /** Takes the next event the SDK delivered. False when there is none */
    bool ( *pollEvent )( void* context, SEosLobbyEvent* event );

    SEosLobbyShim( void )
        : platform(), context( GUCEF_NULL ), createLobby( GUCEF_NULL ), updateLobby( GUCEF_NULL ), joinLobbyById( GUCEF_NULL )
        , leaveLobby( GUCEF_NULL ), destroyLobby( GUCEF_NULL ), kickMember( GUCEF_NULL ), promoteMember( GUCEF_NULL ), sendInvite( GUCEF_NULL )
        , searchLobbies( GUCEF_NULL ), copyLobbyDetails( GUCEF_NULL ), pollEvent( GUCEF_NULL )
    {}

    /** Whether the table has what a lobby service needs */
    bool IsUsable( void ) const
    {
        return GUCEF_NULL != createLobby && GUCEF_NULL != updateLobby && GUCEF_NULL != joinLobbyById && GUCEF_NULL != leaveLobby &&
               GUCEF_NULL != destroyLobby && GUCEF_NULL != kickMember && GUCEF_NULL != promoteMember && GUCEF_NULL != searchLobbies &&
               GUCEF_NULL != copyLobbyDetails && GUCEF_NULL != pollEvent;
    }
};
typedef struct SEosLobbyShim TEosLobbyShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      P2P INTERFACE                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/** EOS_P2P_MAX_PACKET_SIZE (to confirm) */
static const UInt32 EosP2PMaxPacketSize = 1170;
/** EOS_P2P_SOCKET_NAME_MAX_LENGTH (to confirm); the text buffers hold the terminator as well */
static const UInt32 EosP2PMaxSocketNameLength = 32;
static const UInt32 EosP2PUserIdBufferSize = 64;
static const UInt32 EosP2PSocketBufferSize = 40;

/** EOS_EPacketReliability */
enum EEosPacketReliability
{
    EOSRELIABILITY_UNRELIABLE_UNORDERED = 0,
    EOSRELIABILITY_RELIABLE_UNORDERED,
    EOSRELIABILITY_RELIABLE_ORDERED
};

/** EOS_ERelayControl */
enum EEosRelayControl
{
    EOSRELAY_NO_RELAYS = 0,
    EOSRELAY_ALLOW_RELAYS,
    EOSRELAY_FORCE_RELAYS
};

/** EOS_ENATType */
enum EEosNatType
{
    EOSNAT_UNKNOWN = 0,
    EOSNAT_OPEN,
    EOSNAT_MODERATE,
    EOSNAT_STRICT
};

/** EOS_EConnectionClosedReason */
enum EEosConnectionCloseReason
{
    EOSCLOSE_UNKNOWN = 0,
    EOSCLOSE_CLOSED_BY_LOCAL_USER,
    EOSCLOSE_CLOSED_BY_PEER,
    EOSCLOSE_TIMED_OUT,
    EOSCLOSE_TOO_MANY_CONNECTIONS,
    EOSCLOSE_INVALID_MESSAGE,
    EOSCLOSE_INVALID_DATA,
    EOSCLOSE_CONNECTION_FAILED,
    EOSCLOSE_CONNECTION_CLOSED,
    EOSCLOSE_NEGOTIATION_FAILED,
    EOSCLOSE_UNEXPECTED_ERROR
};

/** EOS_ENetworkConnectionType */
enum EEosNetworkType
{
    EOSNETWORK_NONE = 0,
    EOSNETWORK_DIRECT,
    EOSNETWORK_RELAYED
};

enum EEosP2PEventKind
{
    EOSP2PEVENT_CONNECTION_REQUEST = 0,     /**< EOS_P2P_AddNotifyPeerConnectionRequest: a peer wants to connect, AcceptConnection lets it */
    EOSP2PEVENT_CONNECTION_ESTABLISHED,     /**< EOS_P2P_AddNotifyPeerConnectionEstablished */
    EOSP2PEVENT_CONNECTION_INTERRUPTED,     /**< EOS_P2P_AddNotifyPeerConnectionInterrupted */
    EOSP2PEVENT_CONNECTION_CLOSED           /**< EOS_P2P_AddNotifyPeerConnectionClosed */
};

/** A connection event of the P2P interface; text in fixed buffers so that polling does not allocate */
struct SEosP2PEvent
{
    EEosP2PEventKind kind;
    char localUser[ EosP2PUserIdBufferSize ];
    char remoteUser[ EosP2PUserIdBufferSize ];
    char socketName[ EosP2PSocketBufferSize ];
    EEosNetworkType networkType;
    EEosConnectionCloseReason closeReason;

    SEosP2PEvent( void ) : kind( EOSP2PEVENT_CONNECTION_REQUEST ), networkType( EOSNETWORK_NONE ), closeReason( EOSCLOSE_UNKNOWN )
    {
        localUser[ 0 ] = 0; remoteUser[ 0 ] = 0; socketName[ 0 ] = 0;
    }
};

/**
 *  The P2P interface of the SDK (EOS_HP2P of EOS_Platform_GetP2PInterface). Nothing in the packet functions allocates.
 */
struct SEosP2PShim
{
    SEosPlatformShim platform;
    void* context;

    /** EOS_P2P_SendPacket. A connection to the peer is opened by the first packet; the SDK holds the packet until it is (bAllowDelayedDelivery) */
    EEosResult ( *sendPacket )( void* context, const char* localUser, const char* remoteUser, const char* socketName, UInt8 channel,
                                const void* data, UInt32 size, EEosPacketReliability reliability, bool allowDelayedDelivery );

    /** EOS_P2P_GetNextReceivedPacketSize + EOS_P2P_ReceivePacket: false when no packet waits for the local user */
    bool ( *receivePacket )( void* context, const char* localUser, UInt32 bufferSize, void* buffer, UInt32* size,
                             char* remoteUser, char* socketName, UInt8* channel );

    EEosResult ( *acceptConnection )( void* context, const char* localUser, const char* remoteUser, const char* socketName );
    EEosResult ( *closeConnection )( void* context, const char* localUser, const char* remoteUser, const char* socketName );
    EEosResult ( *closeConnections )( void* context, const char* localUser, const char* socketName );

    /** Takes the next connection event. False when there is none */
    bool ( *pollEvent )( void* context, SEosP2PEvent* event );

    /** EOS_P2P_SetRelayControl */
    EEosResult ( *setRelayControl )( void* context, EEosRelayControl control );

    /** EOS_P2P_GetNATType (after EOS_P2P_QueryNATType) */
    EEosNatType ( *getNatType )( void* context );

    /** EOS_P2P_SetPacketQueueSize */
    EEosResult ( *setPacketQueueSize )( void* context, UInt64 incomingMaxBytes, UInt64 outgoingMaxBytes );

    /**
     *  The Product User Id that packets are sent and received as changed (empty: nobody): the notifications of the SDK
     *  are added for that user (EOS_P2P_AddNotifyPeerConnection*), those of the user before are removed. Optional
     */
    void ( *setLocalUser )( void* context, const char* localUser );

    SEosP2PShim( void )
        : platform(), context( GUCEF_NULL ), sendPacket( GUCEF_NULL ), receivePacket( GUCEF_NULL ), acceptConnection( GUCEF_NULL )
        , closeConnection( GUCEF_NULL ), closeConnections( GUCEF_NULL ), pollEvent( GUCEF_NULL ), setRelayControl( GUCEF_NULL )
        , getNatType( GUCEF_NULL ), setPacketQueueSize( GUCEF_NULL ), setLocalUser( GUCEF_NULL )
    {}

    bool IsUsable( void ) const
    {
        return GUCEF_NULL != sendPacket && GUCEF_NULL != receivePacket && GUCEF_NULL != acceptConnection &&
               GUCEF_NULL != closeConnection && GUCEF_NULL != pollEvent;
    }
};
typedef struct SEosP2PShim TEosP2PShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */
