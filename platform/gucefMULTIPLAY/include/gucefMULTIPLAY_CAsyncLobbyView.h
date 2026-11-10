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

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H
#define GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H

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

#ifndef GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H
#include "gucefMULTIPLAY_CLobbyPresentation.h"
#define GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H
#endif /* GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H ? */

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

/** One lobby as the local player sees it now, as a backend gives it */
class GUCEF_MULTIPLAY_EXPORT_CPP CAsyncLobbyView
{
    public:

    CLobbyState state;                  /**< the members, the slots, the phase and the settings; the same for every member */
    CLobbySettingsSchema schema;
    CORE::CString localMemberId;        /**< the member of the local player; empty when the local player is not in the lobby (yet) */
    CLobbyPresentation presentation;    /**< what the screen shows and offers the local player */
    bool closed;                        /**< the lobby is over: cancelled, begun or dismissed */
    CORE::CString closedReason;         /**< why, in words, for the player; empty when it is not closed */
    bool lastVotePassed;                /**< how the vote that ended last came out, for the event that tells it ended */

    CAsyncLobbyView( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H ? */
