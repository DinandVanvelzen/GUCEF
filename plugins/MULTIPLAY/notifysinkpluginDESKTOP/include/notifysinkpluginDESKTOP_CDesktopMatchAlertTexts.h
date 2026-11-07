/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H
#include "notifysinkpluginDESKTOP_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The wording of the notifications. A game replaces these to translate them or to use its own words.
 *  In a format string "{title}" is the title of the match, "{mover}" the name of the previous mover and
 *  "{count}" the number of matches. Anything else is taken literally. The text of a notification of an invitation
 *  never gets a title or a name, whatever its format says.
 */
class GUCEF_MULTIPLAYPLUGIN_DESKTOP_EXPORT_CPP CDesktopMatchAlertTexts
{
    public:

    CORE::CString yourTurnSummaryFallback;      /**< summary of a turn in a match without a title */
    CORE::CString yourTurnBody;                 /**< body of a turn when the previous mover is not known */
    CORE::CString yourTurnBodyAfterMover;       /**< body of a turn when the previous mover is known */
    CORE::CString coalescedSummary;             /**< several games are waiting */
    CORE::CString coalescedSummaryOne;          /**< ... when the count is 1 */
    CORE::CString digestSummary;                /**< the daily digest */
    CORE::CString digestBody;
    CORE::CString digestBodyOne;
    CORE::CString inviteSummary;                /**< an invitation */
    CORE::CString finishedSummary;              /**< a match has ended */
    CORE::CString finishedBody;

    /** The English wording */
    CDesktopMatchAlertTexts( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H ? */
