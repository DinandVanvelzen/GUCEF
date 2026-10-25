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

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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

/**
 *  The measures the windows of the forms share, made for the title bar of BuildDarkTheme(): the content starts this many
 *  pixels below the top of the window, and the strip along the bottom edge that has the status or error and the buttons is this high.
 *  What is in between is the room of the columns, so the buttons never meet them.
 */
const Int32 WindowContentTop = 52;
const Int32 WindowFooterHeight = 88;

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

/** A time left as "m:ss" ("3:00", "0:05", "62:03"): milliseconds are rounded up to whole seconds, so it only shows 0:00 when the time is up; negative is 0:00 */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString FormatCountdown( Int64 timeInMs );

/** The names of the providers of the accounts of an identity ("Epic Games"), once each, in the order of the accounts (the names of AUTH::GetAccountProviderDisplayName()) */
GUCEF_GAMEGUI_EXPORT_CPP void GetAccountProviderNames( const MULTIPLAY::CPlayerIdentity& identity, CORE::CString::StringVector& names );

/** The providers of the accounts of an identity once each: "Guest" without accounts, else "Epic Games, Google" with at most 'maxNames' names and " +n" for the others */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString MakeAccountsSummary( const MULTIPLAY::CPlayerIdentity& identity, UInt32 maxNames );

/** Text with an entry per line, as the string list properties of widgets have it */
GUCEF_GAMEGUI_EXPORT_CPP CORE::CString JoinLines( const CORE::CString::StringVector& lines );

/**
 *  The widget as the class its widget type is created as, NULL when the widget is of another type or there is none.
 *  'typeName' is the name of the standard widget type the class is registered as ("Spinner" for GUI::CSpinner), which a
 *  widget keeps from its creation: this is a check of that type id followed by a static_cast, no run time type
 *  information of C++ is needed.
 */
template< class T >
T* WidgetAs( GUI::CWidget* widget, const char* typeName )
{
    return ( GUCEF_NULL != widget && widget->GetType() == typeName ) ? static_cast< T* >( widget ) : GUCEF_NULL;
}

template< class T >
const T* WidgetAs( const GUI::CWidget* widget, const char* typeName )
{
    return ( GUCEF_NULL != widget && widget->GetType() == typeName ) ? static_cast< const T* >( widget ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_UTILS_H ? */
