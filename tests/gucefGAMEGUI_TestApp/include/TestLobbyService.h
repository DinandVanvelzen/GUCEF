/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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
#ifndef GUCEF_GAMEGUI_TESTAPP_TESTLOBBYSERVICE_H
#define GUCEF_GAMEGUI_TESTAPP_TESTLOBBYSERVICE_H

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

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  An in-memory lobby service for tests: every CTestLobbyService in the process shares one registry of lobbies,
 *  as if they were clients of one server. The host side of a lobby (CTestLobbyHost) applies the CLobbyRules to
 *  every request synchronously and tells all members. Other test apps can copy these classes to exercise code
 *  that works with CILobbyService and CILobby without networking.
 *
 *  Time in the lobbies (the countdown, the vote deadline) runs in "seconds" of secondInMs milliseconds on a
 *  timer of the global pulse generator, so tests can run a countdown quickly.
 */

class CTestLobby;
class CTestLobbyService;

/*-------------------------------------------------------------------------*/

/** The host side of one lobby: the state, the rules and the members */
class CTestLobbyHost : public GUCEF::CORE::CObserver
{
    public:

    CTestLobbyHost( const GUCEF::CORE::CString& lobbyId            ,
                    const GUCEF::MULTIPLAY::CLobbyCreateParams& params ,
                    const GUCEF::MULTIPLAY::CLobbyRules& rules     ,
                    GUCEF::UInt32 secondInMs                       );

    virtual ~CTestLobbyHost();

    const GUCEF::MULTIPLAY::CLobbyState& GetState( void ) const;

    const GUCEF::MULTIPLAY::CLobbySettingsSchema& GetSchema( void ) const;

    const GUCEF::MULTIPLAY::CLobbyCreateParams& GetParams( void ) const;

    const GUCEF::MULTIPLAY::CLobbyRules& GetRules( void ) const;

    /** Adds the member and its lobby object, false with the reason when the rules refuse */
    bool Join( CTestLobby* lobby                                   ,
               const GUCEF::CORE::CString& memberId                ,
               const GUCEF::CORE::CString& name                    ,
               const GUCEF::CORE::CString::StringVector& endpoints ,
               const GUCEF::CORE::CString& password                ,
               bool asSpectator                                    ,
               GUCEF::CORE::CString& reason                        );

    /** The lobby object of the member goes away or leaves */
    void Detach( CTestLobby* lobby );

    void Leave( const GUCEF::CORE::CString& memberId );

    void SetSetting( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& value );
    void ProposeSetting( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& value );
    void Vote( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& voteId, bool yes );
    void SetReady( const GUCEF::CORE::CString& memberId, bool ready );
    void SetSpectator( const GUCEF::CORE::CString& memberId, bool spectate );
    void RequestSlot( const GUCEF::CORE::CString& memberId, GUCEF::UInt32 slotIndex );
    void SetFaction( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& faction );
    void SetColour( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& colour );
    void Kick( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& targetId );
    void TransferBoss( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& targetId );
    void SetSlotKind( const GUCEF::CORE::CString& memberId, GUCEF::UInt32 slotIndex, GUCEF::MULTIPLAY::TLobbySlotKind kind, const GUCEF::CORE::CString& aiType );
    void RequestStart( const GUCEF::CORE::CString& memberId, bool force );

    /** The launch description for the member, as the lobby hands it over */
    GUCEF::MULTIPLAY::CLaunchDescription MakeLaunchDescription( const GUCEF::CORE::CString& memberId ) const;

    /** The listing of the lobby, for Browse() */
    GUCEF::MULTIPLAY::CGameListingEntry MakeListing( void ) const;

    bool IsPasswordProtected( void ) const;

    /** Names the chat of the lobby, which every member hears about as a change of the state */
    void SetChat( const GUCEF::CORE::CString& backend, const GUCEF::CORE::CString& channel );

    /** Tells a member's latency to everyone, as the lobby session would */
    void SetMemberLatency( const GUCEF::CORE::CString& memberId, GUCEF::Int32 latencyMs );

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CTestLobbyHost > TEventCallback;
    typedef GUCEF::vector< CTestLobby* > TLobbyVector;
    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString::StringVector > TEndpointMap;

    CTestLobby* FindLobby( const GUCEF::CORE::CString& memberId ) const;
    bool IsAttached( CTestLobby* lobby ) const;
    void Refuse( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& what, const GUCEF::CORE::CString& reason );
    void Broadcast( const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void StateChanged( const GUCEF::CORE::CString& path );
    void StartVote( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& what, const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& value );
    void EvaluateVote( void );
    void EndVote( bool passed );
    void StartCountdown( void );
    void CancelCountdown( void );
    void Launch( void );
    void OnSecond( void );
    void OnTimerUpdate( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void ApplyLeaveOutcome( const GUCEF::CORE::CString& memberId                         ,
                            const GUCEF::MULTIPLAY::CLobbyVote& voteBefore               ,
                            const GUCEF::MULTIPLAY::CLobbyRules::TLeaveOutcome& outcome  ,
                            const GUCEF::CORE::CString& closeReason                      );

    private:

    GUCEF::MULTIPLAY::CLobbyState m_state;
    GUCEF::MULTIPLAY::CLobbyCreateParams m_params;
    GUCEF::MULTIPLAY::CLobbyRules m_rules;
    GUCEF::CORE::CString m_hostMemberId;
    TLobbyVector m_lobbies;
    TEndpointMap m_endpoints;
    GUCEF::CORE::CTimer m_timer;
    GUCEF::Int32 m_countdownLeft;
    GUCEF::Int32 m_voteSecondsLeft;
    GUCEF::UInt32 m_voteCount;
};

/*-------------------------------------------------------------------------*/

/** A member's view of a lobby that a CTestLobbyHost holds */
class CTestLobby : public GUCEF::MULTIPLAY::CILobby
{
    public:

    CTestLobby( CTestLobbyHost* host, const GUCEF::CORE::CString& localMemberId );

    virtual ~CTestLobby();

    virtual const GUCEF::MULTIPLAY::CLobbyState& GetState( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::MULTIPLAY::CLobbySettingsSchema& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetLocalMemberId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSetting( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ProposeSetting( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Vote( const GUCEF::CORE::CString& voteId, bool yes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetReady( bool ready ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSpectator( bool spectate ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestSlot( GUCEF::UInt32 slotIndex ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetFaction( const GUCEF::CORE::CString& faction ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetColour( const GUCEF::CORE::CString& colour ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Kick( const GUCEF::CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Ban( const GUCEF::CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ReportPlayer( const GUCEF::CORE::CString& memberId, const GUCEF::CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TransferBoss( const GUCEF::CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSlotKind( GUCEF::UInt32 slotIndex, GUCEF::MULTIPLAY::TLobbySlotKind kind, const GUCEF::CORE::CString& aiType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ForceStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Leave( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the member is still in the lobby */
    bool IsMember( void ) const;

    /** What the member reported and banned last */
    GUCEF::CORE::CString lastReportedId;
    GUCEF::CORE::CString lastReportReason;
    GUCEF::CORE::CString lastBannedId;

    /** The host tells the member something */
    bool Deliver( const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );

    /** The host no longer has the member */
    void OnDetached( void );

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTestLobbyHost* m_host;
    GUCEF::CORE::CString m_localMemberId;
    bool m_attached;
    GUCEF::MULTIPLAY::CLobbyState m_lastState;          /**< the state when the member left */
    GUCEF::MULTIPLAY::CLobbySettingsSchema m_lastSchema;
};

/*-------------------------------------------------------------------------*/

/** A client of the in-memory lobby "server" */
class CTestLobbyService : public GUCEF::MULTIPLAY::CILobbyService
{
    public:

    static const GUCEF::CORE::CString ServiceName;      /**< "test" */

    CTestLobbyService( void );

    virtual ~CTestLobbyService();

    virtual bool Connect( const GUCEF::MULTIPLAY::CPlayerIdentity& identity, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Browse( const GUCEF::MULTIPLAY::CGameListingFilter& filter, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CreateLobby( const GUCEF::MULTIPLAY::CLobbyCreateParams& params, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinLobby( const GUCEF::CORE::CString& lobbyId    ,
                            const GUCEF::CORE::CString& password   ,
                            bool asSpectator                       ,
                            GUCEF::CORE::CFutureResult& future     ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::MULTIPLAY::CLobbyServiceCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PublishListing( const GUCEF::MULTIPLAY::CGameListingEntry& listing ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool WithdrawListing( const GUCEF::CORE::CString& listingId ) GUCEF_VIRTUAL_OVERRIDE;

    /** The endpoints the local player can be reached on, they go into the launch descriptions */
    void SetLocalEndpoints( const GUCEF::CORE::CString::StringVector& endpoints );

    /** The rules of the lobbies this client creates */
    void SetRules( const GUCEF::MULTIPLAY::CLobbyRules& rules );

    /** The length of a lobby "second" for the lobbies this client creates */
    void SetSecondInMs( GUCEF::UInt32 secondInMs );

    /** The lobby of the registry by its id, for tests that look at the host side */
    static CTestLobbyHost* FindHost( const GUCEF::CORE::CString& lobbyId );

    /** Forgets every lobby and published listing, call it when a test is done with its lobbies */
    static void Reset( void );

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool EnterLobby( CTestLobbyHost* host, const GUCEF::CORE::CString& password, bool asSpectator, GUCEF::CORE::CFutureResult& future );

    private:

    GUCEF::MULTIPLAY::CPlayerIdentity m_identity;
    bool m_connected;
    GUCEF::CORE::CString::StringVector m_localEndpoints;
    GUCEF::MULTIPLAY::CLobbyRules m_rules;
    GUCEF::UInt32 m_secondInMs;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::CORE::CTFactory< GUCEF::MULTIPLAY::CILobbyService, CTestLobbyService, GUCEF::MT::CMutex > TTestLobbyServiceFactory;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTLOBBYSERVICE_H ? */
