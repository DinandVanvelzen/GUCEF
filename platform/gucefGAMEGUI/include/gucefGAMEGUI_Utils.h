/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_GAMEGUI_UTILS_H
#define GUCEF_GAMEGUI_UTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The text of a round trip time: "35 ms", empty when it is not known (negative) */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString FormatPing( Int32 pingMs );

/** "2/4", or "2" when there is no maximum */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString FormatPlayers( UInt32 players, UInt32 maxPlayers );

/** The text without the white space at both ends */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString TrimText( const CORE::CString& text );

/** The hours and minutes of a time as "14:05" */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString FormatClock( const CORE::CDateTime& time );

/** The name of a lobby member; the id itself when the member is not known */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString GetMemberName( const MULTIPLAY::CLobbyState& state, const CORE::CString& memberId );

/** The seconds from now to the deadline, rounded up; -1 when the deadline is not set */
GUCEF_GAMEGUI_EXPORT_CPP Int32 GetSecondsUntil( const CORE::CDateTime& deadline );

/** Text with an entry per line, as the string list properties of widgets have it */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString JoinLines( const CORE::CString::StringVector& lines );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_UTILS_H ? */
