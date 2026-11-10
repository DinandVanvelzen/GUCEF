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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#include <string>
#include <vector>
#include <map>
#include <utility>

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
 *  The Chrome (or Chromium) browser we drive: finds it, starts it with remote debugging and a profile of our own,
 *  and makes a DevTools connection to it.
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

    /**
     *  Looks for a browser: the configured path, then the usual names on the search path, then the usual install locations.
     *  Chrome is preferred, Chromium is used when there is no Chrome. Both speak the same DevTools protocol.
     */
    CORE::CString FindExecutable( void ) const;

    /** The command line arguments to start the browser with */
    std::vector< CORE::CString > BuildArguments( const CORE::CString& startUrl ) const;

    /**
     *  A terminal started from inside a snap package (VS Code for example) hands every program its private
     *  library and settings locations, which makes Chrome crash right at startup. This works out which variables
     *  have to go, and which have to get the value they had before the snap changed them.
     *  Nothing happens when the environment is not from a snap.
     *  @param environment      name and value of every variable
     */
    static void BuildCleanEnvironment( const std::vector< std::pair< std::string, std::string > >& environment ,
                                       std::vector< std::string >& unsetNames                                  ,
                                       std::map< std::string, std::string >& overrides                         );

    /**
     *  A browser that is already running can be used when it was told to allow remote debugging
     *  (chrome://inspect/#remote-debugging). It then has no /json pages: where to connect is written in the
     *  DevToolsActivePort file of its profile, a port on the first line and the path of the browser's web socket
     *  on the second.
     *  @return false if the file is not there or not in that form
     */
    static bool ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath );

    /** The directories the browsers keep their profile in by default, the ones likely to hold a DevToolsActivePort file */
    static std::vector< CORE::CString > GetDefaultUserDataDirs( void );

    /** Look for a running browser that allows remote debugging in these profile directories, or the default ones if there are none */
    void SetAttachToRunning( bool attach );

    void SetUserDataDirs( const std::vector< CORE::CString >& dirs );

    /** Are we working with a browser that was already running and which we did not start? */
    bool IsAttached( void ) const;

    /** Does anything listen on the port that speaks http, whether it is the pages of a browser or not */
    bool IsListening( UInt16 port ) const;

    /** Does a browser answer on the debugging port? */
    bool IsAnswering( UInt32 timeoutInMs = 2000 ) const;

    /**
     *  Makes sure there is a browser to talk to: uses the one on the debugging port or starts one.
     *  @param startUrl     the page a newly started browser opens
     */
    bool EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs = 30000 );

    /** Was the browser started by us, as opposed to already being there? */
    bool WasLaunched( void ) const;

    /** Ends a browser which we started. One that was already running is left alone. */
    bool Shutdown( void );

    /**
     *  Connects the client to the browser itself, which is what is needed to see all of its tabs.
     *  A browser that was already running asks its user to allow the connection, which takes time.
     */
    bool ConnectToBrowser( WEB::CChromeDevToolsClient& client, UInt32 timeoutInMs = 10000 ) const;

    private:

    CORE::CString m_executablePath;
    UInt16 m_debugPort;
    CORE::CString m_profileDir;
    bool m_headless;
    std::vector< CORE::CString > m_extraArguments;
    CORE::CProcess m_process;
    bool m_launched;
    bool m_attachToRunning;
    std::vector< CORE::CString > m_userDataDirs;
    CORE::CString m_browserWebSocketUrl;      // set when we work with a browser that was already running
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
