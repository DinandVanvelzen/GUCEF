/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H

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

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#include "pluginglueEOS_CEosGlue.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H
#include "lobbyservicepluginEOS_CEosLobbyServiceSettings.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CEosLobby;

/** The data of CEosLobbyService::InviteReceivedEvent */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosInviteEventData : public CORE::CICloneable
{
    public:

    CORE::CString lobbyId;
    CORE::CString senderUserId;

    CEosInviteEventData( void );

    CEosInviteEventData( const CORE::CString& lobbyId, const CORE::CString& senderUserId );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The lobby service "eos": lobbies on the lobby interface of Epic Online Services (EOS_Lobby_*), found with
 *  EOS_LobbySearch, hosted by the player that created them (the owner of the EOS lobby is the boss of ours), with the
 *  game started over EOS P2P. EOS has no server of ours: the owner applies the rules (CLobbyRules) to the requests of the
 *  members, which travel as member attributes, and publishes the state as lobby attributes; members read the state from
 *  the lobby. See CEosLobbyCodec for what is on the wire.
 *
 *  Needs: the SDK (compiled in, or a table of functions given with SetShim()), a platform that is logged in at EOS
 *  Connect (the auth plugin does the login: the Product User Id is the member id in the lobbies). Without any of that
 *  Connect() fails with the reason. Everything is asynchronous: the calls return at once, the futures end when the SDK
 *  has answered, on the thread that pumps the pulse generator; every future ends exactly once, in time (the request
 *  timeout) or when it is cancelled.
 *
 *  Capabilities: voting (the owner runs the votes), relay (the P2P of EOS relays), in-progress spectating; no
 *  matchmaking and no chat: EOS lobbies have no text chat, so the "lobby" chat service does not work over this one.
 *
 *  UNVERIFIED AGAINST THE REAL SDK: see CEosLobbySdk. Only tested with a fake SDK.
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbyService : public MULTIPLAY::CILobbyService
{
    public:

    static const CORE::CString ServiceName;         /**< "eos" */

    static const CORE::CEvent InviteReceivedEvent;  /**< CEosInviteEventData: another user invited the local user to a lobby */

    static void RegisterEvents( void );

    CEosLobbyService( void );

    virtual ~CEosLobbyService();

    virtual bool Connect( const MULTIPLAY::CPlayerIdentity& identity, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Browse( const MULTIPLAY::CGameListingFilter& filter, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CreateLobby( const MULTIPLAY::CLobbyCreateParams& params, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinLobby( const CORE::CString& lobbyId    ,
                            const CORE::CString& password   ,
                            bool asSpectator                ,
                            CORE::CFutureResult& future     ) GUCEF_VIRTUAL_OVERRIDE;
    virtual MULTIPLAY::CLobbyServiceCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Makes a lobby of the local user that is hosted here visible: the permission level public. False for a listing of anything else */
    virtual bool PublishListing( const MULTIPLAY::CGameListingEntry& listing ) GUCEF_VIRTUAL_OVERRIDE;

    /** Hides a lobby of the local user: the permission level becomes invite only. The lobby stays, members stay */
    virtual bool WithdrawListing( const CORE::CString& listingId ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Uses these tables of functions instead of the SDK that is compiled in: for tests and for applications that bind the
     *  SDK themselves. The P2P table is optional; without it a game cannot be started. Before Connect()
     */
    void SetShim( const PLUGINGLUE::EOS::SEosLobbyShim& lobbyShim, const PLUGINGLUE::EOS::SEosP2PShim* p2pShim );

    const CEosLobbyServiceSettings& GetSettings( void ) const;

    /** Changes the settings; the ones the service uses to connect apply from the next Connect() */
    bool SetSettings( const CEosLobbyServiceSettings& settings );

    /** Browse() only shows lobbies whose mirrored setting (CEosLobbyServiceSettings::mirrorSettings) has the value */
    void SetBrowseSettingFilter( const CORE::CString& key, const CORE::CString& value );

    void ClearBrowseSettingFilters( void );

    /** Browse() looks in this bucket instead of the one of the settings, empty for the setting */
    void SetBrowseBucket( const CORE::CString& bucketId );

    /** The platform of the glue the service holds while connected */
    PLUGINGLUE::EOS::CEosPlatform* GetPlatform( void ) const;

    /** The Product User Id the service acts as, empty while not connected */
    const CORE::CString& GetLocalUser( void ) const;

    /** The requests to the SDK that have not ended yet, the lobbies that are being made or joined included */
    UInt32 GetPendingRequestCount( void ) const;

    /** The lobbies the service has */
    UInt32 GetLobbyCount( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CEosLobbyService > TEventCallback;

    friend class CEosLobby;

    enum EOpKind
    {
        OP_CREATE = 0,      /**< a lobby being made: create, then the first update, then the future ends */
        OP_JOIN,            /**< a lobby being joined: join, the member attributes, the acceptance of the owner */
        OP_BROWSE,
        OP_LOBBY,           /**< a request of a lobby: update, leave, destroy, kick, promote, invite */
        OP_CLEANUP          /**< a request without anyone waiting: the leave or destroy of a lobby that nobody wants any more */
    };

    struct SOp
    {
        UInt32 requestId;
        EOpKind kind;
        PLUGINGLUE::EOS::EEosLobbyEventKind expected;
        CORE::CFutureResultSource source;
        UInt64 deadlineInMs;
        CORE::CString lobbyId;
        MULTIPLAY::CILobbyPtr lobby;
        MULTIPLAY::CLobbyCreateParams params;
        CORE::CString password;
        bool asSpectator;
        MULTIPLAY::CGameListingFilter filter;
        bool abandoned;             /**< the future has ended, the SDK has not answered yet */
        bool sdkAnswered;           /**< OP_CREATE and OP_JOIN: the first request of the SDK ended */

        SOp( void ) : requestId( 0 ), kind( OP_LOBBY ), expected( PLUGINGLUE::EOS::EOSLOBBYEVENT_UPDATE ), source(), deadlineInMs( 0 ), lobbyId(), lobby()
                    , params(), password(), asSpectator( false ), filter(), abandoned( false ), sdkAnswered( false ) {}
    };
    typedef GUCEF::map< UInt32, SOp > TOpMap;
    typedef GUCEF::map< CORE::CString, CEosLobby* > TLobbyMap;

    CEosLobbyService( const CEosLobbyService& src );
    CEosLobbyService& operator=( const CEosLobbyService& src );

    bool FailConnect( CORE::CFutureResultSource& source, const CORE::CString& reason );
    bool ResolveShim( CORE::CString& reason );
    void ReleasePlatform( void );
    void UpdateTickDemand( void );
    void OnTick( void );

    void OnPlatformTick( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void OnPlatformLoginStateChanged( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData );
    void PollEvents( void );
    void CheckOps( UInt64 nowInMs );
    void HandleEvent( const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnRequestEnd( const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnBrowseEnd( SOp& op, const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnCreateEnd( SOp& op, const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnJoinEnd( SOp& op, const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void EndOp( UInt32 requestId, bool succeeded, const CORE::CString& reason );
    void AbandonOp( UInt32 requestId, bool cancelled, const CORE::CString& reason );
    void Cleanup( const CORE::CString& lobbyId, bool owner );
    CEosLobby* FindLobby( const CORE::CString& lobbyId ) const;
    void OnLoginStateChanged( void );
    UInt32 NextRequestId( void );
    UInt32 AddOp( EOpKind kind, PLUGINGLUE::EOS::EEosLobbyEventKind expected, const CORE::CString& lobbyId, UInt32 requestId );
    static CORE::CString DescribeResult( PLUGINGLUE::EOS::EEosResult result );

    // For the lobbies
    bool CopyDetails( const CORE::CString& lobbyId, PLUGINGLUE::EOS::SEosLobbyDetails& details ) const;
    UInt32 RequestUpdate( const CORE::CString& lobbyId, const PLUGINGLUE::EOS::SEosLobbyUpdateOptions& options );
    UInt32 RequestLeave( const CORE::CString& lobbyId );
    UInt32 RequestDestroy( const CORE::CString& lobbyId );
    UInt32 RequestKick( const CORE::CString& lobbyId, const CORE::CString& target );
    UInt32 RequestPromote( const CORE::CString& lobbyId, const CORE::CString& target );
    UInt32 RequestInvite( const CORE::CString& lobbyId, const CORE::CString& target );
    void OnLobbyStarted( CEosLobby* lobby, bool succeeded, const CORE::CString& reason );
    void Register( CEosLobby* lobby );
    void Unregister( CEosLobby* lobby );
    PLUGINGLUE::EOS::CEosP2PRouter* GetRouter( void ) const;
    const MULTIPLAY::CPlayerIdentity& GetIdentity( void ) const;

    private:

    CEosLobbyServiceSettings m_settings;
    PLUGINGLUE::EOS::SEosLobbyShim m_lobbyShim;
    PLUGINGLUE::EOS::SEosP2PShim m_p2pShim;
    bool m_haveShim;
    bool m_haveP2PShim;
    PLUGINGLUE::EOS::CEosPlatform* m_platform;
    UInt32 m_tickDemand;
    MULTIPLAY::CPlayerIdentity m_identity;
    CORE::CString m_localUser;
    bool m_connected;
    UInt32 m_nextRequestId;
    TOpMap m_ops;
    TLobbyMap m_lobbies;
    CORE::CString::StringMap m_browseSettingFilters;
    CORE::CString m_browseBucket;
    GUCEF::vector< MULTIPLAY::CILobbyPtr > m_retiredLobbies;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< MULTIPLAY::CILobbyService, CEosLobbyService, MT::CMutex > TEosLobbyServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H ? */
