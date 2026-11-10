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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#include <cstdlib>
#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
#include <unistd.h>
#endif

#include "AncestryBackup_CChromeBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChromeBrowser::CChromeBrowser( void )
    : m_executablePath()
    , m_debugPort( 9222 )
    , m_profileDir()
    , m_headless( false )
    , m_extraArguments()
    , m_process()
    , m_launched( false )
    , m_attachToRunning( true )
    , m_userDataDirs()
    , m_browserWebSocketUrl()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetExecutablePath( const CORE::CString& path )
{GUCEF_TRACE;

    m_executablePath = path;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetDebugPort( UInt16 port )
{GUCEF_TRACE;

    m_debugPort = port;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetProfileDir( const CORE::CString& dir )
{GUCEF_TRACE;

    m_profileDir = dir;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetHeadless( bool headless )
{GUCEF_TRACE;

    m_headless = headless;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::AddArgument( const CORE::CString& argument )
{GUCEF_TRACE;

    m_extraArguments.push_back( argument );
}

/*-------------------------------------------------------------------------*/

UInt16
CChromeBrowser::GetDebugPort( void ) const
{GUCEF_TRACE;

    return m_debugPort;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetAttachToRunning( bool attach )
{GUCEF_TRACE;

    m_attachToRunning = attach;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetUserDataDirs( const std::vector< CORE::CString >& dirs )
{GUCEF_TRACE;

    m_userDataDirs = dirs;
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::IsAttached( void ) const
{GUCEF_TRACE;

    return !m_browserWebSocketUrl.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath )
{GUCEF_TRACE;

    const std::string text = fileContent.STL_String();
    const size_t firstEnd = text.find_first_of( "\r\n" );
    if ( firstEnd == std::string::npos || firstEnd == 0 )
        return false;
    const UInt32 value = CORE::StringToUInt32( text.substr( 0, firstEnd ).c_str(), 0 );
    if ( value == 0 || value > 65535 )
        return false;

    size_t secondStart = text.find_first_not_of( "\r\n", firstEnd );
    if ( secondStart == std::string::npos )
        return false;
    size_t secondEnd = text.find_first_of( "\r\n", secondStart );
    if ( secondEnd == std::string::npos )
        secondEnd = text.size();
    const std::string path = text.substr( secondStart, secondEnd - secondStart );
    if ( path.empty() || path[ 0 ] != '/' )
        return false;

    port = (UInt16) value;
    webSocketPath = path.c_str();
    return true;
}

/*-------------------------------------------------------------------------*/

std::vector< CORE::CString >
CChromeBrowser::GetDefaultUserDataDirs( void )
{GUCEF_TRACE;

    std::vector< CORE::CString > dirs;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const CORE::CString local = CORE::GetEnv( "LOCALAPPDATA" );
    if ( !local.IsNULLOrEmpty() )
    {
        dirs.push_back( CORE::CombinePath( local, "Google\\Chrome\\User Data" ) );
        dirs.push_back( CORE::CombinePath( local, "Chromium\\User Data" ) );
    }
    #else
    const CORE::CString home = CORE::GetEnv( "HOME" );
    if ( !home.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )
        dirs.push_back( CORE::CombinePath( home, "Library/Application Support/Google/Chrome" ) );
        dirs.push_back( CORE::CombinePath( home, "Library/Application Support/Chromium" ) );
        #else
        dirs.push_back( CORE::CombinePath( home, ".config/google-chrome" ) );
        dirs.push_back( CORE::CombinePath( home, ".config/chromium" ) );
        dirs.push_back( CORE::CombinePath( home, "snap/chromium/common/chromium" ) );
        #endif
    }
    #endif
    return dirs;
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::WasLaunched( void ) const
{GUCEF_TRACE;

    return m_launched;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChromeBrowser::FindExecutable( void ) const
{GUCEF_TRACE;

    if ( !m_executablePath.IsNULLOrEmpty() )
        return m_executablePath;

    static const char* const names[] = {
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        "chrome.exe", "msedge.exe",
    #else
        "google-chrome-stable", "google-chrome", "chrome", "chromium", "chromium-browser",
    #endif
        GUCEF_NULL };

    // the search path first, the same way a shell would find it
    const CORE::CString searchPath = CORE::GetEnv( "PATH" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const char separator = ';';
    #else
    const char separator = ':';
    #endif
    const std::string paths = searchPath.STL_String().c_str();
    for ( UInt32 n=0; names[ n ] != GUCEF_NULL; ++n )
    {
        size_t start = 0;
        while ( start <= paths.size() && !paths.empty() )
        {
            size_t end = paths.find( separator, start );
            if ( end == std::string::npos )
                end = paths.size();
            const std::string dir = paths.substr( start, end - start );
            if ( !dir.empty() )
            {
                const CORE::CString candidate = CORE::CombinePath( dir.c_str(), names[ n ] );
                if ( CORE::FileExists( candidate ) )
                    return candidate;
            }
            start = end + 1;
        }
    }

    // and where the installers put it
    static const char* const locations[] = {
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
        "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )
        "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
        "/Applications/Chromium.app/Contents/MacOS/Chromium",
    #else
        "/usr/bin/google-chrome-stable", "/usr/bin/google-chrome", "/usr/bin/chromium", "/usr/bin/chromium-browser", "/snap/bin/chromium",
    #endif
        GUCEF_NULL };
    for ( UInt32 i=0; locations[ i ] != GUCEF_NULL; ++i )
        if ( CORE::FileExists( locations[ i ] ) )
            return locations[ i ];

    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

std::vector< CORE::CString >
CChromeBrowser::BuildArguments( const CORE::CString& startUrl ) const
{GUCEF_TRACE;

    std::vector< CORE::CString > args;
    args.push_back( "--remote-debugging-port=" + CORE::UInt32ToString( m_debugPort ) );
    if ( !m_profileDir.IsNULLOrEmpty() )
        args.push_back( "--user-data-dir=" + m_profileDir );
    args.push_back( "--no-first-run" );
    args.push_back( "--no-default-browser-check" );
    if ( m_headless )
    {
        args.push_back( "--headless=new" );
        args.push_back( "--disable-gpu" );
    }
    for ( size_t i=0; i<m_extraArguments.size(); ++i )
        args.push_back( m_extraArguments[ i ] );
    if ( !startUrl.IsNULLOrEmpty() )
        args.push_back( startUrl );
    return args;
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::BuildCleanEnvironment( const std::vector< std::pair< std::string, std::string > >& environment ,
                                       std::vector< std::string >& unsetNames                                  ,
                                       std::map< std::string, std::string >& overrides                         )
{GUCEF_TRACE;

    unsetNames.clear();
    overrides.clear();

    static const char* const snapOriginalSuffix = "_VSCODE_SNAP_ORIG";
    static const char* const polluters[] = { "GSETTINGS_SCHEMA_DIR", "GTK_PATH", "GTK_EXE_PREFIX", "GTK_MODULES", "GTK_IM_MODULE_FILE",
                                             "GDK_PIXBUF_MODULE_FILE", "GDK_PIXBUF_MODULEDIR", "GIO_MODULE_DIR", "LOCPATH",
                                             "GIO_LAUNCHED_DESKTOP_FILE", "GIO_LAUNCHED_DESKTOP_FILE_PID", GUCEF_NULL };

    bool fromSnap = false;
    for ( size_t i=0; i<environment.size(); ++i )
        if ( environment[ i ].first == "SNAP" && !environment[ i ].second.empty() )
            fromSnap = true;
    if ( !fromSnap )
        return;

    const size_t suffixLength = strlen( snapOriginalSuffix );
    for ( size_t i=0; i<environment.size(); ++i )
    {
        const std::string& name = environment[ i ].first;
        bool unsetIt = name == "SNAP" || name.compare( 0, 5, "SNAP_" ) == 0;
        for ( UInt32 p=0; polluters[ p ] != GUCEF_NULL; ++p )
            if ( name == polluters[ p ] )
                unsetIt = true;

        if ( name.size() > suffixLength && name.compare( name.size() - suffixLength, suffixLength, snapOriginalSuffix ) == 0 )
        {
            // the snap saved what the variable was before it changed it
            overrides[ name.substr( 0, name.size() - suffixLength ) ] = environment[ i ].second;
            unsetIt = true;
        }
        if ( unsetIt )
            unsetNames.push_back( name );
    }

    // a variable that is restored is not also unset
    std::vector< std::string > remaining;
    for ( size_t i=0; i<unsetNames.size(); ++i )
        if ( overrides.find( unsetNames[ i ] ) == overrides.end() )
            remaining.push_back( unsetNames[ i ] );
    unsetNames.swap( remaining );
}

/*-------------------------------------------------------------------------*/

/** Remembers why a discovery failed */
class CFailureRecorder : public CORE::CObserver
{
    public:
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == WEB::CChromeDevToolsClient::DiscoveryFailedEvent && GUCEF_NULL != eventdata )
            reason = static_cast< CORE::TCloneableString* >( eventdata )->GetData();
    }
    CORE::CString reason;
};

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::IsListening( UInt16 port ) const
{GUCEF_TRACE;

    WEB::CChromeDevToolsClient probe;
    CFailureRecorder recorder;
    recorder.SubscribeTo( &probe );
    WEB::SChromeDiscoveryResult result;
    if ( probe.DiscoverTargetsAndWait( "127.0.0.1", port, result, 3000 ) )
        return true;

    // any answer of a web server will do
    return std::string( recorder.reason.STL_String() ).find( "answered with HTTP code" ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::IsAnswering( UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    WEB::CChromeDevToolsClient probe;
    WEB::SChromeDiscoveryResult result;
    return probe.DiscoverTargetsAndWait( "127.0.0.1", m_debugPort, result, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( IsAnswering() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Using the browser that is already running on debugging port " + CORE::UInt32ToString( m_debugPort ) );
        return true;
    }

    // a browser the user already has open, which was set to allow remote debugging
    if ( m_attachToRunning )
    {
        const std::vector< CORE::CString > dirs = m_userDataDirs.empty() ? GetDefaultUserDataDirs() : m_userDataDirs;
        for ( size_t i=0; i<dirs.size(); ++i )
        {
            CORE::CString content;
            UInt16 port = 0;
            CORE::CString path;
            const CORE::CString file = CORE::CombinePath( dirs[ i ], "DevToolsActivePort" );
            if ( !CORE::FileExists( file ) || !CORE::LoadTextFileAsString( file, content ) || !ParseDevToolsActivePort( content, port, path ) )
                continue;

            // the file stays behind when the browser ends, is something listening? A browser which allows remote
            // debugging in this way answers the pages of the other way with a 404, nothing at all means nothing is there
            if ( !IsListening( port ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: " + file + " names port " + CORE::UInt32ToString( port ) + " but nothing is listening there" );
                continue;
            }

            m_browserWebSocketUrl = "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + path;
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AncestryBackup: Using the browser that is already running, which allows remote debugging (" + dirs[ i ] + ")" );
            return true;
        }
    }

    const CORE::CString exe = FindExecutable();
    if ( exe.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Unable to find Chrome or Chromium. Install it or set the chromePath setting" );
        return false;
    }

    // detached: the browser holds the login, it has to outlive us
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Starting " + exe );
    std::vector< CORE::CString > args = BuildArguments( startUrl );
    CORE::CString launchExe = exe;
    CORE::CProcess::TStringMap overrides;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    std::vector< std::pair< std::string, std::string > > environment;
    for ( char** e = environ; GUCEF_NULL != e && GUCEF_NULL != *e; ++e )
    {
        const std::string entry = *e;
        const size_t eq = entry.find( '=' );
        if ( eq != std::string::npos )
            environment.push_back( std::make_pair( entry.substr( 0, eq ), entry.substr( eq + 1 ) ) );
    }
    std::vector< std::string > unsetNames;
    std::map< std::string, std::string > restored;
    BuildCleanEnvironment( environment, unsetNames, restored );
    if ( !unsetNames.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: This terminal comes from a snap package, starting the browser without the snap's environment" );

        // env removes the variables, CProcess can only add
        std::vector< CORE::CString > wrapped;
        for ( size_t i=0; i<unsetNames.size(); ++i )
        {
            wrapped.push_back( "-u" );
            wrapped.push_back( unsetNames[ i ].c_str() );
        }
        wrapped.push_back( exe );
        wrapped.insert( wrapped.end(), args.begin(), args.end() );
        args.swap( wrapped );
        launchExe = "/usr/bin/env";
        for ( std::map< std::string, std::string >::const_iterator r = restored.begin(); r != restored.end(); ++r )
            overrides[ r->first.c_str() ] = r->second.c_str();
    }
    #endif

    if ( !m_process.Launch( launchExe, args, CORE::CString::Empty, overrides, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Failed to start " + exe );
        return false;
    }
    m_launched = true;

    const UInt64 start = MT::PrecisionTickCount();
    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( ( MT::PrecisionTickCount() - start ) / ticksPerMs < timeoutInMs )
    {
        if ( IsAnswering( 1000 ) )
            return true;
        if ( !m_process.IsRunning() )
        {
            // chrome hands over to an instance that is already running when it finds one using the same profile
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The browser process ended right away, is a browser with this profile already running without the debugging port?" );
        }
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 200 );
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: The browser did not open the debugging port in time" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::Shutdown( void )
{GUCEF_TRACE;

    if ( !m_launched )
        return true;
    m_launched = false;
    return m_process.Terminate( true, 5000 );
}

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::ConnectToBrowser( WEB::CChromeDevToolsClient& client, UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    if ( !m_browserWebSocketUrl.IsNULLOrEmpty() )
    {
        // the browser asks its user whether this is allowed
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AncestryBackup: Connecting to the browser. If it asks whether to allow remote debugging, say yes." );
        return client.ConnectAndWait( m_browserWebSocketUrl, timeoutInMs < 90000 ? 90000 : timeoutInMs );
    }

    WEB::SChromeDiscoveryResult result;
    if ( !client.DiscoverTargetsAndWait( "127.0.0.1", m_debugPort, result, timeoutInMs ) )
        return false;
    if ( result.browser.webSocketDebuggerUrl.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The browser did not give a web socket url for itself" );
        return false;
    }
    return client.ConnectAndWait( result.browser.webSocketDebuggerUrl, timeoutInMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
