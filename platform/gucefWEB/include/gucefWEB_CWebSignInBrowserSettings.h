/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H
#define GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

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
 *  How the browser of a web session sign in is started (CIWebSignInBrowser, CWebSessionSignIn): which executable,
 *  which profile folder, which debugging port, with or without a window, and how long to wait for it. The debugging
 *  port is always on the loopback interface; arguments that would change that, the port or the profile are refused.
 *
 *  Config (SaveConfig()/LoadConfig()): executablePath, profileDirectory, debuggingPort, headless, keepOpen,
 *  extraArguments (a list), startTimeoutMs, commandTimeoutMs.
 */
class GUCEF_WEB_PUBLIC_CPP CWebSignInBrowserSettings
{
    public:

    static const UInt32 DefaultStartTimeoutInMs;    /**< 30000 */
    static const UInt32 DefaultCommandTimeoutInMs;  /**< 15000 */

    CORE::CString executablePath;                   /**< empty: the first browser CChromeBrowserLocator finds */
    CORE::CString profileDirectory;                 /**< empty: chosen by CWebSessionSignIn, one per store */
    UInt16 debuggingPort;                           /**< 0: the browser picks a free one and says which in its profile */
    bool headless;                                  /**< no window: --headless=new; always so for an unattended sign in */
    bool keepOpen;                                  /**< leave the browser open after the sign in */
    CORE::CString::StringVector extraArguments;     /**< more command line arguments; see IsArgumentAllowed() */
    UInt32 startTimeoutInMs;
    UInt32 commandTimeoutInMs;

    CWebSignInBrowserSettings( void );

    bool SaveConfig( CORE::CDataNode& config ) const;

    bool LoadConfig( const CORE::CDataNode& config );

    /**
     *  Whether an extra argument may be given: anything but those that set the debugging address, port, pipe or allowed
     *  origins, or the profile folder (--remote-debugging-*, --remote-allow-origins, --user-data-dir)
     */
    static bool IsArgumentAllowed( const CORE::CString& argument );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H ? */
