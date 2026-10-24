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

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#define GUCEF_MULTIPLAY_CILOBBY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

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

/** The data of VoteStartedEvent and VoteEndedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyVoteEventData : public CORE::CICloneable
{
    public:

    CLobbyVote vote;
    bool passed;            /**< VoteEndedEvent: whether the vote passed */

    CLobbyVoteEventData( void );

    CLobbyVoteEventData( const CLobbyVote& vote, bool passed );

    virtual ~CLobbyVoteEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The data of RequestRefusedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyRefusalEventData : public CORE::CICloneable
{
    public:

    CORE::CString what;     /**< the request: "setting", "proposal", "vote", "slot", "faction", "colour", "slotKind", "ready", "spectator", "kick", "transferBoss", "start", "forceStart" */
    CORE::CString reason;

    CLobbyRefusalEventData( void );

    CLobbyRefusalEventData( const CORE::CString& what, const CORE::CString& reason );

    virtual ~CLobbyRefusalEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A lobby the local player is a member of, as a lobby service gives it.
 *
 *  The requests return whether they could be made; whether they are granted is up to the host side, which applies
 *  the CLobbyRules. A granted request shows as a LobbyStateChangedEvent, a refused one as a RequestRefusedEvent.
 *  The state is the same for every member.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CILobby : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent LobbyStateChangedEvent;   /**< TLobbyStateChangedEventData: the changed paths, such as "phase", "boss", "members", "slots", "vote", "settings/<key>" */
    static const CORE::CEvent MemberJoinedEvent;        /**< TMemberEventData: the member id */
    static const CORE::CEvent MemberLeftEvent;          /**< TMemberEventData: the member id */
    static const CORE::CEvent VoteStartedEvent;         /**< CLobbyVoteEventData */
    static const CORE::CEvent VoteEndedEvent;           /**< CLobbyVoteEventData with the outcome */
    static const CORE::CEvent CountdownEvent;           /**< TCountdownEventData: the seconds left, -1 when it was cancelled */
    static const CORE::CEvent LaunchEvent;              /**< CLaunchDescription for the local member */
    static const CORE::CEvent LobbyClosedEvent;         /**< TLobbyClosedEventData: the reason */
    static const CORE::CEvent RequestRefusedEvent;      /**< CLobbyRefusalEventData */

    typedef CORE::TCloneableStringVector    TLobbyStateChangedEventData;
    typedef CORE::TCloneableString          TMemberEventData;
    typedef CORE::TCloneableInt32           TCountdownEventData;
    typedef CORE::TCloneableString          TLobbyClosedEventData;

    static void RegisterEvents( void );

    CILobby( void );

    virtual ~CILobby();

    virtual const CLobbyState& GetState( void ) const = 0;

    virtual const CLobbySettingsSchema& GetSchema( void ) const = 0;

    virtual const CORE::CString& GetLocalMemberId( void ) const = 0;

    virtual bool IsBoss( void ) const;

    /** Applies when the change policy of the setting lets the local member, else it becomes a proposal */
    virtual bool SetSetting( const CORE::CString& key, const CORE::CString& value ) = 0;

    virtual bool ProposeSetting( const CORE::CString& key, const CORE::CString& value ) = 0;

    virtual bool Vote( const CORE::CString& voteId, bool yes ) = 0;

    virtual bool SetReady( bool ready ) = 0;

    virtual bool SetSpectator( bool spectate ) = 0;

    virtual bool RequestSlot( UInt32 slotIndex ) = 0;

    virtual bool SetFaction( const CORE::CString& faction ) = 0;

    virtual bool SetColour( const CORE::CString& colour ) = 0;

    /** Boss only */
    virtual bool Kick( const CORE::CString& memberId ) = 0;

    /** Boss only */
    virtual bool TransferBoss( const CORE::CString& memberId ) = 0;

    /**
     *  Boss only: kicks the member and keeps it out of the lobby. The default implementation: the lobby has no bans.
     */
    virtual bool Ban( const CORE::CString& memberId );

    /**
     *  Reports a member for the people who run the service to look at, with why. The default implementation: the
     *  lobby takes no reports.
     */
    virtual bool ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason );

    /** Boss only: opens, closes or gives an AI a slot */
    virtual bool SetSlotKind( UInt32 slotIndex, TLobbySlotKind kind, const CORE::CString& aiType ) = 0;

    /** The boss starts when the rules allow it, another player starts a vote to start */
    virtual bool RequestStart( void ) = 0;

    /** Boss only, when the rules allow a force start */
    virtual bool ForceStart( void ) = 0;

    virtual bool Leave( void ) = 0;

    private:

    CILobby( const CILobby& src );
    CILobby& operator=( const CILobby& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CILobby, MT::CMutex >    CILobbyPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */
