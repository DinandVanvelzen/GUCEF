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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMULTIPLAY_CLobbyPresentation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyPresentation::CLobbyPresentation( void )
    : showLatency( true )
    , showFactionAndColour( true )
    , showReady( true )
    , showStart( true )
    , showForceStart( true )
    , showVoteStart( true )
    , showSpectate( true )
    , showInvite( false )
    , settingsEditable( true )
    , showBack( false )
    , showCancel( false )
    , readyText( "Ready" )
    , readyColumnTitle( "Ready" )
    , spectateText( "Spectate" )
    , backText( "Back" )
    , leaveText( "Leave" )
    , startText( "Start" )
    , forceStartText( "Force start" )
    , voteStartText( "Vote start" )
    , kickText( "Kick" )
    , cancelText( "Cancel game" )
    , startHint()
    , forceStartHint()
    , leaveHint()
    , cancelHint()
    , note()
    , voteLines()
    , voteWarning()
    , canReady( false )
    , canSpectate( false )
    , canStart( false )
    , canForceStart( false )
    , canVoteStart( false )
    , canLeave( false )
    , canCancel( false )
    , canInvite( false )
    , canKick( false )
    , canBan( false )
    , canTransferBoss( false )
    , canManageSlots( false )
    , canChooseSlot( false )
    , canChooseFaction( false )
    , canChooseColour( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
