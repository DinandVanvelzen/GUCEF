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

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_MULTIPLAY_CTOKENBUCKET_H
#include "gucefMULTIPLAY_CTokenBucket.h"
#define GUCEF_MULTIPLAY_CTOKENBUCKET_H
#endif /* GUCEF_MULTIPLAY_CTOKENBUCKET_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#include "pluginglueEOS_CEosP2PRouter.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H ? */

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

class CEosLobbyService;

/**
 *  A lobby of the service "eos", as a member sees it. The member id is the Product User Id; the boss is the owner of the
 *  EOS lobby (the two are always the same: Promote in EOS is TransferBoss here, and when the owner leaves EOS picks the new
 *  owner, who becomes the boss).
 *
 *  The member that is the owner also runs the lobby: it applies the rules to the requests of the members and publishes
 *  the state as lobby attributes (only the owner can change those). Members send requests as attributes of themselves;
 *  the owner answers with an acknowledgement or a refusal in the state, which shows as RequestRefusedEvent. Nothing a
 *  member writes counts for more than a request: data of other members is validated, sizes bounded, and the data of the
 *  lobby counts only when the owner of the lobby wrote it.
 *
 *  Limits of what EOS gives (see CEosLobbyService): there is no password in EOS, no approval of a join, no text chat, and the
 *  attributes of a lobby are limited in number and size. A lobby with a password is kept by the owner: a member that joins
 *  has to show the proof of the password in its attributes, the owner removes (kicks) whoever does not within the grace
 *  time; the password is a courtesy, not a lock, as anyone who can read the lobby can join it. Use an invite only lobby for
 *  privacy. The host migration of EOS is off for lobbies with a password (the successor would not know the password), the
 *  lobby closes with its owner.
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobby : public MULTIPLAY::CILobby
{
    public:

    /** A lobby of the service; attach false makes one that is not known to the service, to try out whether data fits */
    CEosLobby( CEosLobbyService& service, const CORE::CString& lobbyId, const CORE::CString& localUser, bool attach = true );

    virtual ~CEosLobby();

    virtual const MULTIPLAY::CLobbyState& GetState( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const MULTIPLAY::CLobbySettingsSchema& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetLocalMemberId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSetting( const CORE::CString& key, const CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ProposeSetting( const CORE::CString& key, const CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Vote( const CORE::CString& voteId, bool yes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetReady( bool ready ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSpectator( bool spectate ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestSlot( UInt32 slotIndex ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetFaction( const CORE::CString& faction ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetColour( const CORE::CString& colour ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Kick( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;

    /** The member is kicked and cannot join this lobby again for as long as it exists. The bans are not kept. Boss only */
    virtual bool Ban( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: the Reports interface of EOS is not used; RequestRefusedEvent says so */
    virtual bool ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TransferBoss( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSlotKind( UInt32 slotIndex, MULTIPLAY::TLobbySlotKind kind, const CORE::CString& aiType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ForceStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Leave( void ) GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetLobbyId( void ) const;

    /** Whether the local user is the owner of the lobby in EOS, which makes it the boss and the one that runs the rules */
    bool IsOwner( void ) const;

    /** Whether the local user is in the lobby still */
    bool IsMember( void ) const;

    /** Whether the lobby has been made or accepted, and can be used */
    bool IsReady( void ) const;

    /** Owner only: shows the lobby to searches (public permission level) or hides it (invite only) */
    bool SetVisibility( bool visible );

    /** Sends an EOS invitation to the user; the owner or any member when the lobby allows invites */
    bool InvitePlayer( const CORE::CString& userId );

    /** The launch description of the game, once the lobby started it and the token of this member arrived */
    bool HasLaunchDescription( void ) const;

    const MULTIPLAY::CLaunchDescription& GetLaunchDescription( void ) const;

    /** The P2P socket the game runs on (the members of the lobby are the peers it lets in), NULL before the start */
    PLUGINGLUE::EOS::CEosP2PSocket* GetGameSocket( void ) const;

    /** How often the owner called UpdateLobby; with the rate limit it stays far below the number of changes */
    UInt32 GetUpdateCount( void ) const;

    /** The revision of the state that is published, counts with every publication of the owner */
    UInt32 GetRevision( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CEosLobbyService;

    enum EStage
    {
        STAGE_NEW = 0,
        STAGE_STARTING,         /**< the owner: the first update is under way. A member: waiting for the owner to accept it */
        STAGE_ACTIVE,
        STAGE_DETACHED
    };

    struct SPendingJoin
    {
        CORE::CString userId;
        UInt64 deadlineInMs;
    };
    typedef GUCEF::vector< SPendingJoin > TPendingJoinVector;

    /** A member that is to be kicked once the publication that says why has been made */
    struct SKick
    {
        CORE::CString userId;
        UInt32 minRevision;
        UInt64 notBeforeInMs;
    };
    typedef GUCEF::vector< SKick > TKickVector;

    /** A request of a member that waits for the acknowledgement of the owner */
    struct SRequest
    {
        UInt32 sequence;
        CORE::CString what;         /**< for the refusal event */
        bool isReady;
        bool readyValue;
        CORE::CString verb;
        CORE::CString arg1;
        CORE::CString arg2;
        UInt64 deadlineInMs;

        SRequest( void ) : sequence( 0 ), what(), isReady( false ), readyValue( false ), verb(), arg1(), arg2(), deadlineInMs( 0 ) {}
    };
    typedef GUCEF::vector< SRequest > TRequestVector;
    typedef GUCEF::map< CORE::CString, UInt32 > TSequenceMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TTextMap;
    typedef GUCEF::map< CORE::CString, MULTIPLAY::CTokenBucket > TBucketMap;

    CEosLobby( const CEosLobby& src );
    CEosLobby& operator=( const CEosLobby& src );

    // --- the service
    bool StartAsOwner( const MULTIPLAY::CLobbyCreateParams& params, const CORE::CString& password, CORE::CString& reason );
    bool StartAsMember( const PLUGINGLUE::EOS::SEosLobbyDetails& details, const CORE::CString& password, bool asSpectator, CORE::CString& reason );
    bool PrepareOwner( const MULTIPLAY::CLobbyCreateParams& params, const CORE::CString& password, CORE::CString& reason );
    static bool Validate( const CEosLobbyService& service, const MULTIPLAY::CLobbyCreateParams& params, CORE::CString& reason );
    static UInt32 GetMaxMembers( const CEosLobbyServiceSettings& settings, const MULTIPLAY::CLobbyCreateParams& params );
    static PLUGINGLUE::EOS::EEosLobbyPermission GetPermission( const CEosLobbyServiceSettings& settings, bool isPublic, bool visible );
    void OnRequestEnded( UInt32 requestId, PLUGINGLUE::EOS::EEosLobbyEventKind kind, PLUGINGLUE::EOS::EEosResult result );
    void OnLobbyEvent( const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnMemberStatus( const PLUGINGLUE::EOS::SEosLobbyEvent& event );
    void OnRemoved( const CORE::CString& reason );
    void OnTick( UInt64 nowInMs );
    void OnServiceGone( void );

    /** The service disconnects while the lobby is there: the local user leaves, or the lobby is destroyed when it ends with its owner, and the application is told it closed */
    void OnServiceDisconnected( const CORE::CString& reason );
    void Detach( const CORE::CString& closeReason, bool raiseClosed );
    void ReleaseSockets( void );

    // --- reading the lobby from EOS (every member)
    void Refresh( void );
    void RefreshAsMember( const PLUGINGLUE::EOS::SEosLobbyDetails& details );
    bool LoadDefinition( const CORE::CDataNode& definition, CORE::CString& reason );
    void AdoptState( const MULTIPLAY::CLobbyState& newState );
    void ReadExtras( const CORE::CDataNode& extras );
    void RaiseVoteEvents( const MULTIPLAY::CLobbyState& oldState, const MULTIPLAY::CLobbyState& newState );
    void FilterMembers( const PLUGINGLUE::EOS::SEosLobbyDetails& details, MULTIPLAY::CLobbyState& state ) const;
    static bool IsInEos( const PLUGINGLUE::EOS::SEosLobbyDetails& details, const CORE::CString& userId );
    void StateChanged( const CORE::CString& path );
    void Refuse( const CORE::CString& what, const CORE::CString& reason );

    // --- requests of a member
    bool SendRequest( const CORE::CString& what, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 );
    void PumpRequests( UInt64 nowInMs );
    void OnAck( UInt32 ackedSequence );
    void OnRefusal( const CORE::CString& text );
    bool WriteMemberAttributes( const PLUGINGLUE::EOS::TEosAttributeVector& attributes );

    // --- the launch (every member)
    void CheckLaunch( const PLUGINGLUE::EOS::SEosLobbyDetails& details, const CEosLobbySummary& summary );
    void RaiseLaunch( void );
    void RequestToken( UInt64 nowInMs );
    void OnControlPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet );
    bool OpenControlSocket( void );
    bool OpenGameSocket( void );
    void UpdateSockets( const PLUGINGLUE::EOS::SEosLobbyDetails& details );

    // --- the owner
    void Takeover( const PLUGINGLUE::EOS::SEosLobbyDetails& details );
    void HostTick( UInt64 nowInMs );
    void HostRequestLocal( const CORE::CString& what, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 );
    void HostHandle( const CORE::CString& memberId, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2, UInt32 sequence );
    void HostRefuse( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& reason, UInt32 sequence );
    void HostSetReady( const CORE::CString& memberId, bool ready, UInt32 sequence );
    void HostMemberJoined( const CORE::CString& userId );
    void HostEvaluateJoin( const CORE::CString& userId, bool deadlinePassed );
    void HostMemberGone( const CORE::CString& userId );
    void HostProcessMember( const CORE::CString& userId );
    void HostOnMemberUpdated( const CORE::CString& userId );
    void HostStartVote( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& key, const CORE::CString& value );
    void HostEvaluateVote( void );
    void HostEndVote( bool passed );
    void HostStartCountdown( void );
    void HostCancelCountdown( void );
    void HostLaunch( void );
    bool HostPrepareLaunch( CORE::CString& reason );
    void HostSecond( void );
    void HostApplyLeaveOutcome( const CORE::CString& userId, const MULTIPLAY::CLobbyVote& voteBefore, const MULTIPLAY::CLobbyRules::TLeaveOutcome& outcome );
    void HostKickMember( const CORE::CString& userId, const CORE::CString& reason, bool ban );
    void HostSendTokenTo( const CORE::CString& userId );
    CORE::CString HostIssueToken( const CORE::CString& userId, const CORE::CString& role ) const;
    void HostMakeLaunch( MULTIPLAY::CLaunchDescription& launch ) const;
    void HostFixBoss( void );
    bool HostCommit( const CORE::CString& path, const CORE::CString& requester, const CORE::CString& what );
    bool HostBuildAttributes( PLUGINGLUE::EOS::TEosAttributeVector& attributes, CORE::CString& reason, UInt32 revision );
    bool HostDefinitionAttributes( PLUGINGLUE::EOS::TEosAttributeVector& attributes, CORE::CString& reason );
    bool HostPublish( void );
    void HostOnUpdateEnded( PLUGINGLUE::EOS::EEosResult result );
    void HostProcessQueues( UInt64 nowInMs );
    void HostClose( const CORE::CString& reason );
    void HostFinishClose( void );

    private:

    CEosLobbyService* m_service;
    CORE::CString m_lobbyId;
    CORE::CString m_localUser;
    EStage m_stage;
    bool m_isOwner;
    CORE::CString m_ownerUserId;
    MULTIPLAY::CLobbyState m_state;
    MULTIPLAY::CLobbyState m_ownerState;
    bool m_haveOwnerState;
    MULTIPLAY::CLobbyState m_lastGoodState;
    MULTIPLAY::CLobbySettingsSchema m_schema;
    MULTIPLAY::CLobbyCreateParams m_params;
    MULTIPLAY::CLobbyRules m_rules;
    CEosLobbySummary m_summary;
    CORE::CString m_password;
    CORE::CString m_joinPassword;
    bool m_joinAsSpectator;
    bool m_hasDefinition;
    bool m_closing;
    UInt64 m_closeDestroyAtInMs;

    // what is published and how
    PLUGINGLUE::EOS::TEosAttributeVector m_wireAttributes;
    PLUGINGLUE::EOS::TEosAttributeVector m_definitionAttributes;
    bool m_forceFull;
    bool m_dirty;
    bool m_updateInFlight;
    UInt32 m_updateRequestId;
    UInt32 m_memberWriteRequestId;
    UInt32 m_promoteRequestId;
    UInt32 m_updateFailures;
    UInt64 m_nextPublishInMs;
    UInt32 m_updateCount;
    UInt32 m_revision;
    UInt32 m_publishedRevision;
    UInt32 m_confirmedRevision;
    UInt32 m_closeRevision;
    UInt32 m_lastRevisionSeen;
    MULTIPLAY::CTokenBucket m_updateBucket;
    bool m_visible;
    bool m_visibilityChange;

    // the owner
    TSequenceMap m_processedSequences;
    TSequenceMap m_acks;
    TTextMap m_refusals;
    TTextMap m_rejections;
    CORE::CString::StringSet m_bans;
    TPendingJoinVector m_pendingJoins;
    TBucketMap m_memberBuckets;
    TKickVector m_kickQueue;
    Int32 m_countdownLeft;
    Int32 m_voteSecondsLeft;
    UInt32 m_voteCount;
    UInt64 m_lastSecondInMs;
    MULTIPLAY::CLobbyVote m_lastEndedVote;
    bool m_lastEndedPassed;
    CORE::CString m_closeReason;
    bool m_launchPublished;
    bool m_haveLaunchKey;
    UInt8 m_launchKey[ 32 ];
    CORE::CString m_launchPublicKey;
    MULTIPLAY::CLaunchDescription m_launch;
    bool m_haveLaunch;
    GUCEF::map< CORE::CString, UInt64 > m_lastTokenSent;
    CORE::CString m_promoteTarget;

    // a member
    UInt32 m_nextSequence;
    TRequestVector m_requests;
    bool m_requestInFlight;
    SRequest m_inFlight;
    UInt32 m_lastAck;
    UInt32 m_lastRefusalSequence;
    UInt32 m_pendingAck;
    CORE::CString m_pendingRefusal;
    CORE::CString m_rejection;
    Int32 m_lastCountdownSeen;
    Int32 m_countdownFromOwner;
    CORE::CString m_lastVoteStarted;
    CORE::CString m_lastVoteEndedSeen;
    bool m_launchRaised;
    bool m_launchWanted;
    MULTIPLAY::CLaunchDescription m_launchTemplate;
    CORE::CString m_token;
    UInt64 m_nextTokenRequestInMs;
    UInt64 m_tokenDeadlineInMs;
    UInt64 m_joinDeadlineInMs;

    // the sockets of the game
    PLUGINGLUE::EOS::CEosP2PRouter* m_router;
    PLUGINGLUE::EOS::CEosP2PSocket* m_gameSocket;
    PLUGINGLUE::EOS::CEosP2PSocket* m_controlSocket;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H ? */
