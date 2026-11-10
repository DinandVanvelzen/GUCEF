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

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#define GUCEF_MULTIPLAY_CLOBBYRULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

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

/**
 *  The rules of a lobby, applied by whoever holds the lobby (the host side). Every Check function says whether a
 *  member may do something and why not; the Apply functions change the state the way the rules say, the edge
 *  cases included. Nothing here talks to the network, so a game can subclass it to change any rule.
 *
 *  Config: minPlayers, maxPlayers, allowSpectators, requireAllReady, allowForceStart, startByVote,
 *  countdownInSecs, voteDurationInSecs, closeWhenHostLeaves
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyRules : public CORE::CIConfigurable
{
    public:

    enum EVoteOutcome
    {
        VOTEOUTCOME_UNDECIDED = 0 ,
        VOTEOUTCOME_PASSED    = 1 ,
        VOTEOUTCOME_FAILED    = 2
    };
    typedef enum EVoteOutcome TVoteOutcome;

    /** What leaving did to the lobby */
    struct SLeaveOutcome
    {
        bool lobbyClosed;
        CORE::CString newBossId;        /**< set when the boss passed on */
        bool voteCancelled;
        bool countdownCancelled;
    };
    typedef struct SLeaveOutcome TLeaveOutcome;

    UInt32 minPlayers;
    UInt32 maxPlayers;                  /**< 0 for no limit */
    bool allowSpectators;
    bool requireAllReady;
    bool allowForceStart;
    bool startByVote;                   /**< players other than the boss can start a vote to start */
    UInt32 countdownInSecs;
    UInt32 voteDurationInSecs;
    bool closeWhenHostLeaves;           /**< a LAN lobby lives in the process of its host */

    CLobbyRules( void );

    CLobbyRules( const CLobbyRules& src );

    virtual ~CLobbyRules();

    CLobbyRules& operator=( const CLobbyRules& src );

    /** Whether someone may join, as a player or as a spectator */
    virtual bool CheckJoin( const CLobbyState& state, bool asSpectator, CORE::CString& reason ) const;

    /**
     *  Whether the member may set the setting to the value
     *
     *  @param needsVote set when the member may only propose it: it becomes a vote
     */
    virtual bool CheckSetting( const CLobbyState& state                 ,
                               const CLobbySettingsSchema& schema       ,
                               const CORE::CString& memberId            ,
                               const CORE::CString& key                 ,
                               const CORE::CString& value               ,
                               bool& needsVote                          ,
                               CORE::CString& reason                    ) const;

    /** Whether the member may propose the value for a vote */
    virtual bool CheckProposal( const CLobbyState& state                 ,
                                const CLobbySettingsSchema& schema       ,
                                const CORE::CString& memberId            ,
                                const CORE::CString& key                 ,
                                const CORE::CString& value               ,
                                CORE::CString& reason                    ) const;

    virtual bool CheckVote( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& voteId, CORE::CString& reason ) const;

    virtual bool CheckSlotRequest( const CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex, CORE::CString& reason ) const;

    virtual bool CheckFaction( const CLobbyState& state, const CLobbySettingsSchema& schema, const CORE::CString& memberId, const CORE::CString& faction, CORE::CString& reason ) const;

    virtual bool CheckColour( const CLobbyState& state, const CLobbySettingsSchema& schema, const CORE::CString& memberId, const CORE::CString& colour, CORE::CString& reason ) const;

    virtual bool CheckSlotKind( const CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex, TLobbySlotKind kind, CORE::CString& reason ) const;

    virtual bool CheckReady( const CLobbyState& state, const CORE::CString& memberId, CORE::CString& reason ) const;

    virtual bool CheckSpectator( const CLobbyState& state, const CORE::CString& memberId, bool spectate, CORE::CString& reason ) const;

    virtual bool CheckKick( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& targetId, CORE::CString& reason ) const;

    virtual bool CheckTransferBoss( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& targetId, CORE::CString& reason ) const;

    /** Whether the lobby is ready to start: enough players and all of them ready */
    virtual bool IsStartAllowed( const CLobbyState& state, CORE::CString& reason ) const;

    /**
     *  Whether the member may start the game
     *
     *  @param force a force start, which skips the readiness when the rules allow it
     *  @param needsVote set when the member may only start a vote to start
     */
    virtual bool CheckStart( const CLobbyState& state, const CORE::CString& memberId, bool force, bool& needsVote, CORE::CString& reason ) const;

    /** The yes votes that pass a vote: more than half of the players */
    virtual UInt32 GetVotesNeeded( const CLobbyState& state ) const;

    virtual TVoteOutcome EvaluateVote( const CLobbyState& state ) const;

    /** Adds a member: the first player in an empty lobby becomes the boss, a player gets the first free slot */
    virtual CLobbyMember& ApplyJoin( CLobbyState& state, const CORE::CString& memberId, const CORE::CString& name, bool asSpectator ) const;

    /**
     *  Takes a member out: the slot frees up, the vote of the member ends, the boss passes to the longest present
     *  player, the countdown stops, and the lobby closes when it is empty or its LAN host left
     *
     *  @param isHost whether the member hosts the lobby, which closes it when closeWhenHostLeaves is set
     */
    virtual void ApplyLeave( CLobbyState& state, const CORE::CString& memberId, bool isHost, TLeaveOutcome& outcome ) const;

    /** Sets the readiness, becoming unready during the countdown stops it */
    virtual void ApplyReady( CLobbyState& state, const CORE::CString& memberId, bool ready, bool& countdownCancelled ) const;

    /** Moves the member into the slot, freeing the slot the member had */
    virtual void ApplySlot( CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex ) const;

    /** Makes the member a spectator, or a player in the first free slot */
    virtual void ApplySpectator( CLobbyState& state, const CORE::CString& memberId, bool spectate ) const;

    virtual void ApplyTransferBoss( CLobbyState& state, const CORE::CString& targetId ) const;

    /** The first free slot, -1 when there is none */
    static Int32 FindFreeSlot( const CLobbyState& state );

    /**
     *  The definition of a setting: the key itself, or for a per member (OWNER) setting "<memberId>/<key>"
     *
     *  @param ownerId the member id of a per member setting, empty otherwise
     */
    static const CLobbySettingDef* FindSettingDef( const CLobbySettingsSchema& schema, const CORE::CString& key, CORE::CString& ownerId );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */
