/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H
#define GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

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
 *  What a screen of a lobby shows and offers the local member right now: the texts of the controls, which of them are
 *  there and which can be used. The lobby gives it (CILobby::GetPresentation()), because the lobby knows the rules; the
 *  screen does not have to know which kind of lobby it shows. A lobby on a live connection gives what a player of a
 *  normal lobby sees (the defaults of this class and of CILobby::GetPresentation()); a lobby whose members answer slowly,
 *  such as a game by mail, hides the latency and the choice of faction and colour and says in its own words what the
 *  buttons do.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyPresentation
{
    public:

    /* ---- what is shown ---- */
    bool showLatency;                   /**< the latency column of the slots and of the spectators */
    bool showFactionAndColour;          /**< the faction and colour choices of the local player */
    bool showReady;                     /**< the check box of the answer of the local player: a host who does not answer has none */
    bool showStart;
    bool showForceStart;
    bool showVoteStart;
    bool showSpectate;
    bool showInvite;                    /**< the row to invite another player by name and contact */
    bool settingsEditable;              /**< false: the settings of the game are fixed, the panel shows them and offers no change */
    bool showBack;                      /**< a button that takes the player back to the list without answering or leaving: a lobby that can be looked at and answered later */
    bool showCancel;                    /**< a button that closes the lobby for everybody, or proposes to: a lobby whose members decide by vote cannot be closed by leaving it */

    /* ---- the words ---- */
    CORE::CString readyText;            /**< the check box of the answer of the local player */
    CORE::CString readyColumnTitle;     /**< the column of the slots with the answers */
    CORE::CString spectateText;
    CORE::CString backText;             /**< the button that goes back to the list */
    CORE::CString leaveText;
    CORE::CString startText;
    CORE::CString forceStartText;
    CORE::CString voteStartText;
    CORE::CString kickText;             /**< the entry of the menu of a player that removes the player */
    CORE::CString cancelText;           /**< the button that closes the lobby or proposes to */
    CORE::CString startHint;            /**< the tooltips of the controls, to say what the control does or why it is off: plain words, empty for none */
    CORE::CString forceStartHint;
    CORE::CString leaveHint;
    CORE::CString cancelHint;
    CORE::CString note;                 /**< plain words about how the lobby works that the player has to see, empty for none */
    GUCEF::vector< CORE::CString > voteLines;   /**< the decisions that were proposed and how they stand, one line each: who proposed it, who voted yes and no, who has not voted, the tally against the votes needed, and what became of it. The form shows them under the note as they come in */
    CORE::CString voteWarning;          /**< something the player has to be warned about in a decision (the host has not carried out a decision that passed), empty for none */

    /* ---- what the local member can do now ---- */
    bool canReady;
    bool canSpectate;
    bool canStart;
    bool canForceStart;
    bool canVoteStart;
    bool canLeave;
    bool canCancel;
    bool canInvite;
    bool canKick;
    bool canBan;
    bool canTransferBoss;
    bool canManageSlots;                /**< open, close or give a slot to an AI */
    bool canChooseSlot;                 /**< take a free slot */
    bool canChooseFaction;
    bool canChooseColour;

    CLobbyPresentation( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H ? */
