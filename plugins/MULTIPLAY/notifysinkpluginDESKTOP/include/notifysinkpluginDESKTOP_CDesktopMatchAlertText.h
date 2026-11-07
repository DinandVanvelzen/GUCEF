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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H

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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H
#include "notifysinkpluginDESKTOP_CDesktopMatchAlertTexts.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXTS_H ? */

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
 *  What is allowed to reach the notification program: text is cleaned and cut to length before it is used as an argument
 *  (see CDesktopMatchAlertSink), and the format strings of CDesktopMatchAlertTexts are filled in here.
 */
class GUCEF_MULTIPLAYPLUGIN_DESKTOP_EXPORT_CPP CDesktopMatchAlertText
{
    public:

    static const UInt32 MaxSummaryBytes = 64;
    static const UInt32 MaxBodyBytes = 160;

    /**
     *  Makes text fit to show: well formed UTF-8 only (every invalid byte is dropped), control characters, line breaks, the
     *  characters that reorder or hide text and any run of white space become a single space, no space at either end.
     *  The result is at most maxBytes bytes and is cut with an ellipsis, never inside a character. With escapeMarkup
     *  the characters '<', '>' and '&' of the text become entities and the limit counts the entities, which are never cut.
     */
    static CORE::CString Sanitize( const CORE::CString& text, UInt32 maxBytes, bool escapeMarkup );

    /** The summary of a notification: at most MaxSummaryBytes, plain text */
    static CORE::CString SanitizeSummary( const CORE::CString& text );

    /** The body of a notification: at most MaxBodyBytes, markup characters escaped */
    static CORE::CString SanitizeBody( const CORE::CString& text );

    /** Replaces "{title}", "{mover}" and "{count}" in the format string once, left to right: what is put in is not searched again */
    static CORE::CString Expand( const CORE::CString& format ,
                                 const CORE::CString& title  ,
                                 const CORE::CString& mover  ,
                                 UInt32 count                );

    /** Whether the name can be given to the notification program as a category: letters, digits, '.', '_' and '-' and not longer than 64 bytes */
    static bool IsValidCategory( const CORE::CString& category );

    private:

    CDesktopMatchAlertText( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CDESKTOPMATCHALERTTEXT_H ? */
