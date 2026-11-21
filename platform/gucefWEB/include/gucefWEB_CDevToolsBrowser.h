/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CDEVTOOLSBROWSER_H
#define GUCEF_WEB_CDEVTOOLSBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <utility>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#include "gucefWEB_CSiteDiscoverySettings.h"
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */

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
 *  A Chrome (or Chromium, Edge) browser to talk to over the DevTools protocol, found in one of two ways:
 *
 *  - attached: a browser the user already runs, started with --remote-debugging-port=<port> (it answers on that port)
 *    or with remote debugging allowed in chrome://inspect/#remote-debugging, in which case its DevToolsActivePort
 *    file (in the profile folder) names the port and the path of the browser's web socket, and the browser asks its
 *    user to allow the connection. Such a browser is never started or ended by this class.
 *  - launched: started with the debugging port and a profile folder of its own (owner-only), which keeps the user's
 *    login between runs. The user drives it by hand; it outlives this object unless Shutdown() is called.
 *
 *  The probes of the debugging port use a client on the pulse generator given with SetPulseGenerator() (by default the
 *  application's): call this class on the thread that drives that pulse generator.
 */
class GUCEF_WEB_PUBLIC_CPP CDevToolsBrowser
{
    public:

    typedef std::pair< CORE::CString, CORE::CString >   TNameValue;
    typedef GUCEF::vector< TNameValue >                 TEnvironment;
    typedef GUCEF::map< CORE::CString, CORE::CString >  TStringMap;

    CDevToolsBrowser( void );

    ~CDevToolsBrowser();

    /** Takes the browser* settings: port, executable, profile folder, headless, the profile folders searched for DevToolsActivePort */
    void Configure( const CSiteDiscoverySettings& settings );

    void SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    void SetExecutablePath( const CORE::CString& path );
    void SetDebugPort( UInt16 port );
    UInt16 GetDebugPort( void ) const;
    void SetProfileDir( const CORE::CString& dir );
    void SetHeadless( bool headless );
    void AddArgument( const CORE::CString& argument );

    /** Look for a running browser that allows remote debugging in these profile folders, or the default ones if there are none */
    void SetAttachToRunning( bool attach );
    void SetUserDataDirs( const CORE::CString::StringVector& dirs );

    /** The configured browser, else the usual names on the search path, else the usual install locations. Empty if none */
    CORE::CString FindExecutable( void ) const;

    /** The command line arguments to start the browser with */
    CORE::CString::StringVector BuildArguments( const CORE::CString& startUrl ) const;

    /**
     *  A terminal started from inside a snap package (VS Code for example) hands every program its private library
     *  and settings locations, which makes Chrome crash right at startup. Works out which variables have to go, and
     *  which have to get the value they had before the snap changed them. Nothing when the environment is not a snap's.
     */
    static void BuildCleanEnvironment( const TEnvironment& environment, CORE::CString::StringVector& unsetNames, TStringMap& overrides );

    /** DevToolsActivePort: a port on the first line and the path of the browser's web socket on the second */
    static bool ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath );

    /** The folders the browsers keep their profile in by default */
    static CORE::CString::StringVector GetDefaultUserDataDirs( void );

    /** Does anything that speaks http listen on the port, a browser or not */
    bool IsListening( UInt16 port ) const;

    /** Does a browser answer on the debugging port? */
    bool IsAnswering( UInt32 timeoutInMs = 2000 ) const;

    /** Finds a running browser: on the debugging port, or (with SetAttachToRunning) through a DevToolsActivePort file. Never starts one */
    bool Attach( void );

    /** Starts a browser with the debugging port and the profile folder, and waits until it answers */
    bool Launch( const CORE::CString& startUrl, UInt32 timeoutInMs = 30000 );

    /** Attach(), else Launch() */
    bool EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs = 30000 );

    /** Is it a browser that was already running and which we did not start, found through DevToolsActivePort? */
    bool IsAttached( void ) const;

    /** Was the browser started by us? */
    bool WasLaunched( void ) const;

    /** Ends a browser which we started. One that was already running is left alone */
    bool Shutdown( void );

    /** Connects the client to the browser itself, which is what is needed to see all of its tabs */
    bool ConnectToBrowser( CChromeDevToolsClient& client, UInt32 timeoutInMs = 10000 ) const;

    private:

    CDevToolsBrowser( const CDevToolsBrowser& src );                /**< not implemented */
    CDevToolsBrowser& operator=( const CDevToolsBrowser& src );     /**< not implemented */

    bool Probe( UInt16 port, UInt32 timeoutInMs, CORE::CString* failureReason ) const;

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CString m_executablePath;
    UInt16 m_debugPort;
    CORE::CString m_profileDir;
    bool m_headless;
    CORE::CString::StringVector m_extraArguments;
    CORE::CProcess m_process;
    bool m_launched;
    bool m_attachToRunning;
    CORE::CString::StringVector m_userDataDirs;
    CORE::CString m_browserWebSocketUrl;      /**< set when we work with a browser found through DevToolsActivePort */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CDEVTOOLSBROWSER_H ? */
