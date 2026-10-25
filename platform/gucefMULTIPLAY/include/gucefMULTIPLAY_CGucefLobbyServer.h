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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNBANLIST_H
#include "gucefDRN_CDRNBanList.h"
#define GUCEF_DRN_CDRNBANLIST_H
#endif /* GUCEF_DRN_CDRNBANLIST_H ? */

#ifndef GUCEF_AUTH_IIDENTITYLINKSTORE_H
#include "gucefAUTH_IIdentityLinkStore.h"
#define GUCEF_AUTH_IIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_IIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERREPORT_H
#include "gucefMULTIPLAY_CPlayerReport.h"
#define GUCEF_MULTIPLAY_CPLAYERREPORT_H
#endif /* GUCEF_MULTIPLAY_CPLAYERREPORT_H ? */

#ifndef GUCEF_MULTIPLAY_CTOKENBUCKET_H
#include "gucefMULTIPLAY_CTokenBucket.h"
#define GUCEF_MULTIPLAY_CTOKENBUCKET_H
#endif /* GUCEF_MULTIPLAY_CTOKENBUCKET_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace DRN { class CDRNSession; class CDRNDataGrid; class CDRNDataStream; }

namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGameAdvertiser;

/**
 *  The settings of a CGucefLobbyServer, section "GucefLobbyServer" of a config:
 *  a "Node" child with the DRN node settings (listeners, relay), and as attributes
 *  listen (the endpoints to listen on, ';' separated, instead of those of the node), relayEnabled, relayListen,
 *  gameServiceName, keyFile, password, requireIdentityProof, requireLinkedProvider, banFile, reportFile, maxLobbies, maxMembersPerLobby,
 *  tokenLifetimeInSecs, secondInMs, latencyIntervalInMs, chatMessagesPerSecond, chatBurst, requestsPerSecond,
 *  requestsBurst, advertiseOnLan;
 *  a "Rules" child with the default CLobbyRules, an "Advertiser" child with the CGameAdvertiser config, a
 *  "MinimumSecurityPolicy" child (a DRN::CDRNSecurityPolicy) and an "ExpectedIntegrity" child (name -> value).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyServerSettings : public CORE::CIConfigurable
{
    public:

    DRN::CDRNNodeSettings node;             /**< listeners, relayEnabled (the relay for the game sessions); the authentication is the server's own */
    DRN::CDRNEndpointList relayListen;      /**< where the relay for the game sessions listens, by default the listen endpoints of the node with the next port */
    CORE::CString gameServiceName;          /**< the DRN service of the game sessions, which the relay offers, default "drn" */
    CORE::CString keyFile;                  /**< the server's secret in hex, made when the file does not exist; empty: a new one per run. The server's Ed25519 key (tokens, pinning) and the node's x25519 key follow from it */
    CORE::CString password;                 /**< the directory wants it when set */
    bool requireIdentityProof;              /**< directory clients prove the key of their identity, default true */
    CORE::CString requireLinkedProvider;    /**< which accounts at external identity providers a directory client has to show (verified link proofs): "none" (default, also empty), "any" (one account of any provider) or the provider namespaces of the accounts, ',' separated ("epic,google": one account at one of them). A setting of this server, not of a player */
    CORE::CString banFile;                  /**< where the bans are kept, empty: in memory only */
    CORE::CString reportFile;               /**< the report log, a data tree file of the codec its extension names; empty: in memory only */
    DRN::CDRNSecurityPolicy minimumSecurityPolicy;  /**< a lobby with a weaker policy is refused (DRN::CDRNSecurityPolicy::IsWeakerThan()); by default encryption and no limits */
    UInt32 chatMessagesPerSecond;           /**< per member, the lobby chat and the global channels, default 2; 0 = no limit */
    UInt32 chatBurst;                       /**< default 10 */
    UInt32 requestsPerSecond;               /**< per directory client, default 10; 0 = no limit */
    UInt32 requestsBurst;                   /**< default 40 */
    CORE::CValueList expectedIntegrity;     /**< name -> value which a client's CIClientIntegrity report must have, empty: any report */
    UInt32 maxLobbies;                      /**< 0 = no limit */
    UInt32 maxMembersPerLobby;              /**< 0 = no limit */
    UInt32 tokenLifetimeInSecs;             /**< default 3600 */
    UInt32 secondInMs;                      /**< the length of a lobby second (the countdown), default 1000 */
    UInt32 latencyIntervalInMs;             /**< how often the members' latency is refreshed, default 1000 */
    bool advertiseOnLan;                    /**< publish the lobbies on the LAN with a CGameAdvertiser */
    CORE::CDataNode advertiserConfig;
    CLobbyRules rules;                      /**< the rules of the lobbies */

    CGucefLobbyServerSettings( void );

    CGucefLobbyServerSettings( const CGucefLobbyServerSettings& src );

    virtual ~CGucefLobbyServerSettings();

    CGucefLobbyServerSettings& operator=( const CGucefLobbyServerSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The built-in lobby server, embeddable in any process: in a game that hosts a LAN lobby, or standalone
 *  (apps/gucefLOBBYSERVER).
 *
 *  It runs a DRN node with a directory session (the listings, the global chat channels, the requests to create and
 *  join lobbies) and one CLIENT_SERVER session per lobby, whose state lives in the grids of CGucefLobbyProtocol.
 *  The CLobbyRules of the lobby check every write as the DRN write validator; settings with the VOTE policy are
 *  voted on with DRN's PROPOSAL_VOTE. The server issues the tokens of the lobby sessions and of the game
 *  sessions it hands over, offers the relay when its node settings ask for it and advertises its lobbies on the
 *  LAN when advertiseOnLan is set.
 *  The relay is a DRN node of its own, of the service of the game sessions: their nodes bind at it with the tokens
 *  of their launch descriptions (see CGucefLobbyRelayAuthenticator), launch.relay names it.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyServer : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent LobbyCreatedEvent;    /**< CORE::TCloneableString: the lobby id */
    static const CORE::CEvent LobbyClosedEvent;     /**< CORE::TCloneableString: the lobby id */
    static const CORE::CEvent LobbyLaunchedEvent;   /**< CORE::TCloneableString: the lobby id */
    static const CORE::CEvent PlayerReportedEvent;  /**< CPlayerReport */

    static void RegisterEvents( void );

    CGucefLobbyServer( void );

    virtual ~CGucefLobbyServer();

    void SetSettings( const CGucefLobbyServerSettings& settings );

    const CGucefLobbyServerSettings& GetSettings( void ) const;

    /** Starts listening and hosting the directory */
    bool Start( void );

    void Stop( void );

    bool IsRunning( void ) const;

    /** Where clients reach the server */
    DRN::CDRNEndpointList GetEndpoints( void ) const;

    /**
     *  Where the nodes of the game sessions reach the relay, empty unless the relay is enabled. The endpoints carry
     *  the relay's key ("key") and the hash of its policy ("policy"), so games of any policy link with it
     */
    DRN::CDRNEndpointList GetRelayEndpoints( void ) const;

    UInt32 GetLobbyCount( void ) const;

    void GetLobbyIds( CORE::CString::StringVector& lobbyIds ) const;

    /** The state of a lobby as the server keeps it, false if there is no such lobby */
    bool GetLobbyState( const CORE::CString& lobbyId, CLobbyState& state ) const;

    DRN::CDRNNode& GetNode( void );

    /** The server's Ed25519 public key in hex: the tokens are signed with it, empty before Start() */
    CORE::CString GetPublicKey( void ) const;

    /** The x25519 public key of the server's node in hex, which clients pin, empty before Start() */
    CORE::CString GetNodePublicKey( void ) const;

    /** GetEndpoints() with the node's key pinned (the endpoint parameter "key"): what listings and replies name */
    DRN::CDRNEndpointList GetPinnedEndpoints( void ) const;

    /** The bans the node checks, kept in banFile */
    DRN::CDRNBanList& GetBanList( void );

    /**
     *  Bans a user: its user id, its identity key when it proved one, the accounts at external identity providers it
     *  verified (and those the link store has linked to its user id), and its address when byAddress; whoever is
     *  linked as that user is kicked from its lobbies and its links are closed.
     *
     *  @param expiryUnixSeconds when the ban ends, 0 = never
     *  @return false when there is nothing to ban
     */
    bool KickAndBan( const CORE::CString& userId, const CORE::CString& reason, bool byAddress, UInt64 expiryUnixSeconds = 0 );

    /**
     *  Bans an account at an external identity provider ("provider:subject"): whoever shows it verified, with any key, is
     *  refused. Whoever is connected with it is banned as a user (see KickAndBan()), and so is the profile the link store
     *  has the account linked to, with all the accounts of that profile.
     *
     *  @return false when the text is not an account reference
     */
    bool KickAndBanAccount( const CORE::CString& account, const CORE::CString& reason, bool byAddress, UInt64 expiryUnixSeconds = 0 );

    /** The accounts at external identity providers the directory client with this user id verified; false when the user is not connected */
    bool GetVerifiedAccounts( const CORE::CString& userId, CORE::CString::StringVector& accounts ) const;

    /**
     *  Where the links of accounts to profiles are known, not owned and it has to outlive the server or be given back with
     *  GUCEF_NULL: with it a profile one of whose accounts is banned is refused as well, and a ban of an account or a user
     *  reaches all the accounts of the profile. Without it only the accounts a client verified at its hello count.
     */
    void SetLinkStore( AUTH::IIdentityLinkStore* linkStore );

    /**
     *  Whether the accounts meet the requirement of requireLinkedProvider ("none", "any" or provider namespaces
     *  separated by ','). 'why' tells the player what is wanted when they do not
     */
    static bool CheckLinkedProviders( const CORE::CString& requirement, const CORE::CString::StringVector& accounts, CORE::CString& why );

    /** The reports of the report log */
    void GetReports( TPlayerReportVector& reports ) const;

    /** The policy the server launches a game with: the lobby's, with signed tokens and token policy hashes required */
    static DRN::CDRNSecurityPolicy MakeGamePolicy( const DRN::CDRNSecurityPolicy& lobbyPolicy );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CGucefLobbyServer > TEventCallback;

    void OnTimerUpdate( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void OnDirectoryItemAnnounced( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnDirectoryMemberLeftEvent( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData );

    void OnRequestsAvailable( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    class CHostedLobby;
    class CAccessPolicy;
    friend class CHostedLobby;
    friend class CAccessPolicy;

    /** A client of the directory */
    struct SDirectoryClient
    {
        CORE::CString userId;
        CORE::CString name;
        CORE::CString publicKey;            /**< the identity key it proved, hex */
        CORE::CString::StringVector accounts;   /**< the accounts at external identity providers it verified, "provider:subject" */
        CORE::CString address;              /**< the host it links from */
        CORE::CString::StringSet channels;
        CTokenBucket requests;
        CTokenBucket chat;
    };
    typedef struct SDirectoryClient TDirectoryClient;

    typedef GUCEF::map< CORE::CString, CHostedLobby* > TLobbyMap;
    typedef GUCEF::map< UInt32, TDirectoryClient > TClientMap;
    typedef GUCEF::map< CORE::CString, DRN::CDRNDataStream* > TChannelMap;
    typedef GUCEF::vector< CHostedLobby* > TLobbyVector;

    CGucefLobbyServer( const CGucefLobbyServer& src );
    CGucefLobbyServer& operator=( const CGucefLobbyServer& src );

    void OnRequests( DRN::CDRNDataStream& stream );
    void OnRequest( UInt32 peerId, UInt32 requestId, const CORE::CString& verb, CORE::CVariantStream& args );
    bool Reply( UInt32 peerId, UInt32 requestId, bool ok, const CORE::CString& reason, const CORE::CVariant* arg1 = GUCEF_NULL, const CORE::CVariant* arg2 = GUCEF_NULL, const CORE::CVariant* arg3 = GUCEF_NULL );
    void OnDirectoryMemberLeft( UInt32 peerId );
    DRN::CDRNDataStream* GetChannel( const CORE::CString& channel );
    void PostToChannel( const CORE::CString& channel, const CORE::CString& kind, const CORE::CString& from, const CORE::CString& text );
    CORE::CString IssueToken( const CORE::CString& sessionName, const CORE::CString& userId, const CORE::CString& role, const CORE::CString& userPublicKey ) const;
    CORE::CString IssueGameToken( const CORE::CString& sessionName, const CORE::CString& userId, const CORE::CString& role, const CORE::CString& userPublicKey, const DRN::CDRNSecurityPolicy& policy ) const;
    bool LoadKeys( void );
    void LoadReports( void );
    bool AddReport( const CPlayerReport& report );
    TDirectoryClient* FindClientOfUser( const CORE::CString& userId, UInt32* peerId = GUCEF_NULL );
    bool IsBannedByAccount( const CORE::CString& userId, const CORE::CString::StringVector& accounts, DRN::CDRNBanEntry& ban );
    void BanAccountsOfProfile( const CORE::CString& userId, const CORE::CString& reason, UInt64 expiryUnixSeconds );
    CHostedLobby* FindLobby( const CORE::CString& lobbyId ) const;
    CHostedLobby* FindLobbyBySession( const CORE::CString& sessionName ) const;
    void PublishListing( CHostedLobby& lobby );
    void WithdrawListing( const CORE::CString& listingId );
    void OnTimer( void );
    bool StartRelay( void );

    private:

    CGucefLobbyServerSettings m_settings;
    DRN::CDRNNode m_node;
    DRN::CDRNNode m_relayNode;              /**< the relay for the game sessions: a node of the game service */
    CAccessPolicy* m_accessPolicy;
    DRN::CDRNSession* m_directory;
    DRN::CDRNDataGrid* m_listings;
    CORE::CTimer m_timer;
    TLobbyMap m_lobbies;
    TLobbyVector m_closedLobbies;
    TClientMap m_clients;
    TChannelMap m_channels;
    UInt32 m_lobbyCount;
    UInt32 m_messageCount;
    bool m_running;
    bool m_haveKeys;
    UInt8 m_privateKey[ 32 ];               /**< the Ed25519 seed the tokens are signed with */
    UInt8 m_publicKey[ 32 ];
    UInt8 m_nodePrivateKey[ 32 ];           /**< the node's static x25519 key */
    UInt8 m_nodePublicKey[ 32 ];
    DRN::CDRNBanList m_banList;
    TPlayerReportVector m_reports;
    AUTH::IIdentityLinkStore* m_linkStore;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H ? */
