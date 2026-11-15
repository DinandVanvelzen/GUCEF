/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#define GUCEF_WEB_CWEBVIEWCAPABILITIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What an engine can do, as flags. Callers ask for what they need: an automatic sign-in needs Cookies, ScriptResult,
 *  IsolatedWorlds and InjectText (or SecureFieldValue); an interactive view in a form adds OffscreenFrames,
 *  InjectPointer and InjectKeyboard, or uses NativeWindow. Engines report honestly what they have: a capability that
 *  is not reported is not used.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewCapabilities
{
    public:

    enum ECapability
    {
        CAP_NONE                    = 0         ,
        CAP_OFFSCREEN_FRAMES        = 1 << 0    ,   /**< frames into the frame exchange (CPU pixel maps) */
        CAP_GPU_FRAMES              = 1 << 1    ,   /**< frames as GPU textures */
        CAP_NATIVE_WINDOW           = 1 << 2    ,   /**< the page in a window of the engine's own */
        CAP_NATIVE_CHILD_WINDOW     = 1 << 3    ,   /**< the page in a child window of the application's window */
        CAP_INJECT_POINTER          = 1 << 4    ,   /**< pointer and wheel input, trusted by the page */
        CAP_INJECT_KEYBOARD         = 1 << 5    ,   /**< key input, trusted by the page */
        CAP_INJECT_TEXT             = 1 << 6    ,   /**< text input (an input method commit) */
        CAP_IME_COMPOSITION         = 1 << 7    ,
        CAP_COOKIES                 = 1 << 8    ,   /**< the cookies of the profile, HttpOnly ones included */
        CAP_SCRIPT_RESULT           = 1 << 9    ,   /**< scripts evaluated with a result */
        CAP_ISOLATED_WORLDS         = 1 << 10   ,   /**< scripts in a world the page cannot reach */
        CAP_USER_AGENT_OVERRIDE     = 1 << 11   ,
        CAP_PROFILE_DIRECTORY       = 1 << 12   ,   /**< a folder per profile */
        CAP_EPHEMERAL_PROFILE       = 1 << 13   ,   /**< a profile that leaves nothing behind */
        CAP_PROXY_SETTINGS          = 1 << 14   ,
        CAP_NAVIGATION_POLICY       = 1 << 15   ,   /**< navigations are decided before they happen */
        CAP_DEFERRED_DECISIONS      = 1 << 16   ,   /**< decisions can be asked of the owner and answered later */
        CAP_NATIVE_CLIPBOARD        = 1 << 17   ,   /**< the engine copies and pastes with the system's clipboard itself */
        CAP_HEADLESS                = 1 << 18   ,   /**< runs without a display server */
        CAP_TRANSPARENT_BACKGROUND  = 1 << 19   ,
        CAP_ZOOM                    = 1 << 20   ,
        CAP_SECURE_FIELD_VALUE      = 1 << 21   ,   /**< a value is passed as data to a fixed helper of an isolated world, never as code */
        CAP_EDIT_COMMANDS           = 1 << 22       /**< cut, copy, paste, select all, undo, redo as commands */
    };

    static const UInt32 CapabilityCount = 23;

    CWebViewCapabilities( void );

    explicit CWebViewCapabilities( UInt32 flags );

    /** True when every flag of 'flags' is there */
    bool Has( UInt32 flags ) const;

    void Set( UInt32 flags, bool present = true );

    UInt32 GetFlags( void ) const;

    /** The flags of 'required' that are not there */
    UInt32 GetMissing( UInt32 required ) const;

    /** The names of the flags, comma separated, for reports */
    CString ToString( void ) const;

    static CString FlagsToString( UInt32 flags );

    /** Flags from comma separated names; names that are not known are left out and counted in 'unknownNames' */
    static UInt32 FlagsFromString( const CString& names, UInt32* unknownNames = GUCEF_NULL );

    /** The name of one flag, as written in settings: "OffscreenFrames", "Cookies" ...; empty for a value that is not one flag */
    static const char* GetFlagName( UInt32 singleFlag );

    bool operator==( const CWebViewCapabilities& other ) const;

    private:

    UInt32 m_flags;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWCAPABILITIES_H ? */
