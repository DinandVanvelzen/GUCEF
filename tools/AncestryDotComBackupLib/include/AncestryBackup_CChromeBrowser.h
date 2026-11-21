/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CCHROMEBROWSER_H
#define ANCESTRYBACKUP_CCHROMEBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <map>
#include <utility>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CDEVTOOLSBROWSER_H
#include "gucefWEB_CDevToolsBrowser.h"
#define GUCEF_WEB_CDEVTOOLSBROWSER_H
#endif /* GUCEF_WEB_CDEVTOOLSBROWSER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Chrome (or Chromium) browser the tool drives: finds it, starts it with remote debugging and a profile of our own,
 *  or uses one the user runs with remote debugging allowed, and makes a DevTools connection to it. The work is done by
 *  WEB::CDevToolsBrowser; this keeps the tool's interface.
 *
 *  The profile directory is what keeps the user logged in between runs. A browser that is already running on the
 *  debugging port is used as it is, so the window the user logged in with stays the one we work with.
 *  A browser we start is not stopped when we end, the login lives in it.
 */
class CChromeBrowser
{
    public:

    CChromeBrowser( void );

    void SetExecutablePath( const CORE::CString& path );

    void SetDebugPort( UInt16 port );

    void SetProfileDir( const CORE::CString& dir );

    void SetHeadless( bool headless );

    void AddArgument( const CORE::CString& argument );

    UInt16 GetDebugPort( void ) const;

    /** The configured browser, the usual names on the search path, then the usual install locations */
    CORE::CString FindExecutable( void ) const;

    /** The command line arguments to start the browser with */
    std::vector< CORE::CString > BuildArguments( const CORE::CString& startUrl ) const;

    /** See WEB::CDevToolsBrowser::BuildCleanEnvironment() */
    static void BuildCleanEnvironment( const std::vector< std::pair< std::string, std::string > >& environment ,
                                       std::vector< std::string >& unsetNames                                  ,
                                       std::map< std::string, std::string >& overrides                         );

    /** See WEB::CDevToolsBrowser::ParseDevToolsActivePort() */
    static bool ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath );

    /** The directories the browsers keep their profile in by default */
    static std::vector< CORE::CString > GetDefaultUserDataDirs( void );

    /** Look for a running browser that allows remote debugging in these profile directories, or the default ones if there are none */
    void SetAttachToRunning( bool attach );

    void SetUserDataDirs( const std::vector< CORE::CString >& dirs );

    /** Are we working with a browser that was already running and which we did not start? */
    bool IsAttached( void ) const;

    bool IsListening( UInt16 port ) const;

    bool IsAnswering( UInt32 timeoutInMs = 2000 ) const;

    /** Uses the browser on the debugging port or a running one that allows remote debugging, else starts one */
    bool EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs = 30000 );

    bool WasLaunched( void ) const;

    /** Ends a browser which we started. One that was already running is left alone. */
    bool Shutdown( void );

    /** Connects the client to the browser itself, which is what is needed to see all of its tabs */
    bool ConnectToBrowser( WEB::CChromeDevToolsClient& client, UInt32 timeoutInMs = 10000 ) const;

    private:

    WEB::CDevToolsBrowser m_browser;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CCHROMEBROWSER_H ? */
