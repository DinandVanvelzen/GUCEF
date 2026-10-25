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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

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

class CGucefLobby;
class CIClientIntegrity;

/**
 *  The client of the built-in lobby server, registered as "gucef": its own DRN node joined to the server's directory
 *  session. CreateLobby() and JoinLobby() ask the directory for a token and join the lobby session with it,
 *  as a CGucefLobby with a node of its own.
 *
 *  Also the way to the directory for others: requests and their replies, the global chat channels and direct
 *  messages (used by CLobbyChatService).
 *
 *  The identity given to Connect() proves itself with its key pair (see CPlayerIdentity): the server knows the
 *  player by the user id of that key. The server's endpoints may pin the key of its node with the parameter "key"
 *  (tcp://host:port?key=<hex>), as the endpoints of its listings do: a server with another key is refused.
 *  The link proofs of the identity (CPlayerIdentity::linkProofs) are shown to the server with that proof, which verifies
 *  them and knows the player by the accounts they verify as well (see CGucefLobbyAuthenticator).
 *  A CIClientIntegrity, when set, reports on the client in the requests to create and join lobbies.
 *
 *  Config: server (the endpoints of the server, ';' separated), password, localEndpoints (where the local player's
 *  game can be reached, they go into the launch descriptions, ';' separated)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyService : public CILobbyService
{
    public:

    static const CORE::CString ServiceName;         /**< "gucef" */

    static const CORE::CEvent ReplyEvent;           /**< CGucefDirectoryReply: the reply to a request */
    static const CORE::CEvent DirectMessageEvent;   /**< CGucefDirectoryReply: args = from, text, time (ms), id */
    static const CORE::CEvent ChannelEntryEvent;    /**< CGucefDirectoryReply: an entry of a chat channel; reason = the channel, args = kind, from, text, time, id */

    static void RegisterEvents( void );

    CGucefLobbyService( void );

    virtual ~CGucefLobbyService();

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Browse( const CGameListingFilter& filter, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CreateLobby( const CLobbyCreateParams& params, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinLobby( const CORE::CString& lobbyId    ,
                            const CORE::CString& password   ,
                            bool asSpectator                ,
                            CORE::CFutureResult& future     ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CLobbyServiceCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PublishListing( const CGameListingEntry& listing ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool WithdrawListing( const CORE::CString& listingId ) GUCEF_VIRTUAL_OVERRIDE;

    void SetServerEndpoints( const DRN::CDRNEndpointList& endpoints );

    const DRN::CDRNEndpointList& GetServerEndpoints( void ) const;

    void SetPassword( const CORE::CString& password );

    /** What reports on this client when it creates or joins a lobby, not owned, NULL for none */
    void SetClientIntegrity( CIClientIntegrity* integrity );

    /** Where the local player's game can be reached, for the launch descriptions */
    void SetLocalEndpoints( const CORE::CString::StringVector& endpoints );

    const CPlayerIdentity& GetIdentity( void ) const;

    /**
     *  Sends a request to the directory: { request id, verb, arguments }. The reply comes as ReplyEvent.
     *
     *  @return the request id, 0 when it could not be sent
     */
    UInt32 SendRequest( const CORE::CString& verb, const CORE::CVariant* arg1 = GUCEF_NULL, const CORE::CVariant* arg2 = GUCEF_NULL,
                        const CORE::CVariant* arg3 = GUCEF_NULL, const CORE::CVariant* arg4 = GUCEF_NULL,
                        const CORE::CVariant* arg5 = GUCEF_NULL );

    /** Starts or stops receiving the entries of a global chat channel of the server */
    bool SubscribeChannel( const CORE::CString& channel, bool subscribe );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetClassTypeName() says: compared with it a holder of a base class pointer can tell this class without RTTI */
    static const CORE::CString& ClassTypeName( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CGucefLobbyService > TEventCallback;

    enum ERequestKind
    {
        REQUEST_OTHER   = 0,
        REQUEST_HELLO   = 1,
        REQUEST_LOBBY   = 2
    };

    struct SPendingRequest
    {
        ERequestKind kind;
        CORE::CFutureResultSource result;
        bool asSpectator;
    };
    typedef struct SPendingRequest TPendingRequest;
    typedef GUCEF::map< UInt32, TPendingRequest > TPendingRequestMap;

    struct SPendingLobby
    {
        CILobbyPtr lobby;
        CORE::CFutureResultSource result;
    };
    typedef struct SPendingLobby TPendingLobby;
    typedef GUCEF::vector< TPendingLobby > TPendingLobbyVector;
    typedef GUCEF::vector< CORE::CFutureResultSource > TFutureSourceVector;
    typedef GUCEF::vector< CGameListingFilter > TFilterVector;

    CGucefLobbyService( const CGucefLobbyService& src );
    CGucefLobbyService& operator=( const CGucefLobbyService& src );

    void OnJoined( void );
    void OnAppMessage( const CORE::CDynamicBuffer& payload );
    void OnReply( UInt32 requestId, bool ok, const CORE::CString& reason, const CORE::CVariant* args );
    void OnLobbyGranted( TPendingRequest& request, const CORE::CVariant* args );
    void OnChannelEntries( DRN::CDRNDataStream& stream );
    void CompleteBrowses( void );
    void FailPending( const CORE::CString& reason );
    CORE::CVariant MakeIntegrityBlob( void );

    void SubscribeToDirectory( DRN::CDRNSession* directory );
    void SubscribeToListings( DRN::CDRNDataGrid* listings );
    void SubscribeToChannel( DRN::CDRNDataStream* stream );
    void SubscribeToLobby( CGucefLobby* lobby );
    void OnPendingLobbyEnded( CORE::CNotifier* notifier, bool ready, CORE::CICloneable* eventData );

    void OnDirectoryJoined( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnDirectoryJoinFailed( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void OnDirectoryItemAnnounced( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnDirectoryAppMessage( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void OnDirectoryClosed( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnListingsChanged( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnChannelEntriesAvailable( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );

    void OnLobbyReady( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void OnLobbyFailed( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    private:

    DRN::CDRNNode m_node;
    DRN::CDRNSession* m_directory;
    DRN::CDRNDataGrid* m_listings;
    DRN::CDRNDataStream* m_requests;
    DRN::CDRNEndpointList m_serverEndpoints;
    CORE::CString m_password;
    CIClientIntegrity* m_integrity;
    CORE::CString::StringVector m_localEndpoints;
    CPlayerIdentity m_identity;
    CORE::CFutureResultSource m_connectResult;
    bool m_connectPending;
    bool m_connected;
    UInt32 m_nextRequestId;
    TPendingRequestMap m_pendingRequests;
    TPendingLobbyVector m_pendingLobbies;
    GUCEF::vector< CILobbyPtr > m_retiredLobbies;   /**< lobbies that failed, let go of outside their own events */
    TFutureSourceVector m_pendingBrowses;
    TFilterVector m_pendingBrowseFilters;
};

/*-------------------------------------------------------------------------*/

/** The data of the directory events of CGucefLobbyService */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefDirectoryReply : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    bool ok;
    CORE::CString reason;
    CORE::CVariant args[ 5 ];

    CGucefDirectoryReply( void );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CILobbyService, CGucefLobbyService, MT::CMutex > TGucefLobbyServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H ? */
