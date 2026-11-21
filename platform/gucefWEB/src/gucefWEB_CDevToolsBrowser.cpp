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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

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

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#include "gucefWEB_CDevToolsBrowser.h"

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
#include <unistd.h>
extern char** environ;
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Remembers why a discovery of the debugging port failed */
class CDevToolsProbeFailure : public CORE::CObserver
{
    public:

    explicit CDevToolsProbeFailure( CChromeDevToolsClient& client )
        : CORE::CObserver()
        , reason()
    {
        TEventCallback callback( this, &CDevToolsProbeFailure::OnDiscoveryFailed );
        SubscribeTo( &client, CChromeDevToolsClient::DiscoveryFailedEvent, callback );
    }

    CORE::CString reason;

    private:

    typedef CORE::CTEventHandlerFunctor< CDevToolsProbeFailure > TEventCallback;

    void OnDiscoveryFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
            reason = static_cast< CORE::TCloneableString* >( eventData )->GetData();
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDevToolsBrowser::CDevToolsBrowser( void )
    : m_pulseGenerator()
    , m_executablePath()
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

CDevToolsBrowser::~CDevToolsBrowser()
{GUCEF_TRACE;

    // a browser we started holds the user's login, it outlives us unless Shutdown() was called
}

/*-------------------------------------------------------------------------*/

void
CDevToolsBrowser::Configure( const CSiteDiscoverySettings& settings )
{GUCEF_TRACE;

    m_executablePath = settings.browserExecutablePath;
    m_debugPort = settings.browserDebugPort;
    m_profileDir = settings.browserProfileDirectory;
    m_headless = settings.browserHeadless;
    m_userDataDirs = settings.browserUserDataDirs;
    m_attachToRunning = CSiteDiscoverySettings::BROWSER_RUNMODE_ATTACH == settings.browserRunMode;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsBrowser::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void CDevToolsBrowser::SetExecutablePath( const CORE::CString& path ) {GUCEF_TRACE; m_executablePath = path; }
void CDevToolsBrowser::SetDebugPort( UInt16 port ) {GUCEF_TRACE; m_debugPort = port; }
UInt16 CDevToolsBrowser::GetDebugPort( void ) const {GUCEF_TRACE; return m_debugPort; }
void CDevToolsBrowser::SetProfileDir( const CORE::CString& dir ) {GUCEF_TRACE; m_profileDir = dir; }
void CDevToolsBrowser::SetHeadless( bool headless ) {GUCEF_TRACE; m_headless = headless; }
void CDevToolsBrowser::AddArgument( const CORE::CString& argument ) {GUCEF_TRACE; m_extraArguments.push_back( argument ); }
void CDevToolsBrowser::SetAttachToRunning( bool attach ) {GUCEF_TRACE; m_attachToRunning = attach; }
void CDevToolsBrowser::SetUserDataDirs( const CORE::CString::StringVector& dirs ) {GUCEF_TRACE; m_userDataDirs = dirs; }
bool CDevToolsBrowser::IsAttached( void ) const {GUCEF_TRACE; return !m_browserWebSocketUrl.IsNULLOrEmpty(); }
bool CDevToolsBrowser::WasLaunched( void ) const {GUCEF_TRACE; return m_launched; }

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath )
{GUCEF_TRACE;

    const char* t = fileContent.C_String();
    const UInt32 len = fileContent.IsNULLOrEmpty() ? 0 : fileContent.ByteSize() - 1;
    UInt32 firstEnd = 0;
    while ( firstEnd < len && t[ firstEnd ] != '\r' && t[ firstEnd ] != '\n' )
        ++firstEnd;
    if ( firstEnd == len || firstEnd == 0 )
        return false;
    const UInt32 value = CORE::StringToUInt32( CORE::CString( t, firstEnd ), 0 );
    if ( value == 0 || value > 65535 )
        return false;
    UInt32 secondStart = firstEnd;
    while ( secondStart < len && ( t[ secondStart ] == '\r' || t[ secondStart ] == '\n' ) )
        ++secondStart;
    if ( secondStart == len )
        return false;
    UInt32 secondEnd = secondStart;
    while ( secondEnd < len && t[ secondEnd ] != '\r' && t[ secondEnd ] != '\n' )
        ++secondEnd;
    if ( t[ secondStart ] != '/' )
        return false;
    port = (UInt16) value;
    webSocketPath = CORE::CString( t + secondStart, secondEnd - secondStart );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CDevToolsBrowser::GetDefaultUserDataDirs( void )
{GUCEF_TRACE;

    CORE::CString::StringVector dirs;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const CORE::CString local = CORE::GetEnv( "LOCALAPPDATA" );
    if ( !local.IsNULLOrEmpty() )
    {
        dirs.push_back( CORE::CombinePath( local, "Google\\Chrome\\User Data" ) );
        dirs.push_back( CORE::CombinePath( local, "Chromium\\User Data" ) );
        dirs.push_back( CORE::CombinePath( local, "Microsoft\\Edge\\User Data" ) );
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

CORE::CString
CDevToolsBrowser::FindExecutable( void ) const
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
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const char separator = ';';
    #else
    const char separator = ':';
    #endif
    const CORE::CString::StringVector paths = CORE::GetEnv( "PATH" ).ParseElements( separator, false );
    for ( UInt32 n=0; names[ n ] != GUCEF_NULL; ++n )
    {
        for ( size_t p=0; p<paths.size(); ++p )
        {
            const CORE::CString candidate = CORE::CombinePath( paths[ p ], names[ n ] );
            if ( CORE::FileExists( candidate ) )
                return candidate;
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

CORE::CString::StringVector
CDevToolsBrowser::BuildArguments( const CORE::CString& startUrl ) const
{GUCEF_TRACE;

    CORE::CString::StringVector args;
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
CDevToolsBrowser::BuildCleanEnvironment( const TEnvironment& environment, CORE::CString::StringVector& unsetNames, TStringMap& overrides )
{GUCEF_TRACE;

    unsetNames.clear();
    overrides.clear();

    static const char* const snapOriginalSuffix = "_VSCODE_SNAP_ORIG";
    static const char* const polluters[] = { "GSETTINGS_SCHEMA_DIR", "GTK_PATH", "GTK_EXE_PREFIX", "GTK_MODULES", "GTK_IM_MODULE_FILE",
                                             "GDK_PIXBUF_MODULE_FILE", "GDK_PIXBUF_MODULEDIR", "GIO_MODULE_DIR", "LOCPATH",
                                             "GIO_LAUNCHED_DESKTOP_FILE", "GIO_LAUNCHED_DESKTOP_FILE_PID", GUCEF_NULL };

    bool fromSnap = false;
    for ( size_t i=0; i<environment.size(); ++i )
        if ( environment[ i ].first == "SNAP" && !environment[ i ].second.IsNULLOrEmpty() )
            fromSnap = true;
    if ( !fromSnap )
        return;

    const UInt32 suffixLength = (UInt32) strlen( snapOriginalSuffix );
    for ( size_t i=0; i<environment.size(); ++i )
    {
        const CORE::CString& name = environment[ i ].first;
        const char* n = name.C_String();
        const UInt32 len = name.IsNULLOrEmpty() ? 0 : name.ByteSize() - 1;
        bool unsetIt = name == "SNAP" || ( len >= 5 && 0 == strncmp( n, "SNAP_", 5 ) );
        for ( UInt32 p=0; polluters[ p ] != GUCEF_NULL; ++p )
            if ( name == polluters[ p ] )
                unsetIt = true;
        if ( len > suffixLength && 0 == strcmp( n + len - suffixLength, snapOriginalSuffix ) )
        {
            // the snap saved what the variable was before it changed it
            overrides[ CORE::CString( n, len - suffixLength ) ] = environment[ i ].second;
            unsetIt = true;
        }
        if ( unsetIt )
            unsetNames.push_back( name );
    }

    // a variable that is restored is not also unset
    CORE::CString::StringVector remaining;
    for ( size_t i=0; i<unsetNames.size(); ++i )
        if ( overrides.find( unsetNames[ i ] ) == overrides.end() )
            remaining.push_back( unsetNames[ i ] );
    unsetNames.swap( remaining );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::Probe( UInt16 port, UInt32 timeoutInMs, CORE::CString* failureReason ) const
{GUCEF_TRACE;

    CChromeDevToolsClient* probe = m_pulseGenerator ? GUCEF_NEW CChromeDevToolsClient( m_pulseGenerator ) : GUCEF_NEW CChromeDevToolsClient();
    bool answered = false;
    {
        CDevToolsProbeFailure failure( *probe );
        SChromeDiscoveryResult result;
        answered = probe->DiscoverTargetsAndWait( "127.0.0.1", port, result, timeoutInMs );
        if ( GUCEF_NULL != failureReason )
            *failureReason = failure.reason;
    }
    GUCEF_DELETE probe;
    return answered;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::IsListening( UInt16 port ) const
{GUCEF_TRACE;

    CORE::CString reason;
    if ( Probe( port, 3000, &reason ) )
        return true;
    // any answer of a web server will do
    return reason.HasSubstr( "answered with HTTP code" ) >= 0;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::IsAnswering( UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    return Probe( m_debugPort, timeoutInMs, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::Attach( void )
{GUCEF_TRACE;

    m_browserWebSocketUrl.Clear();
    if ( IsAnswering() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: Using the browser that is running on debugging port " << m_debugPort );
        return true;
    }
    if ( !m_attachToRunning )
        return false;

    // a browser the user already has open, which was set to allow remote debugging
    const CORE::CString::StringVector dirs = m_userDataDirs.empty() ? GetDefaultUserDataDirs() : m_userDataDirs;
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
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: " << file << " names port " << port << " but nothing is listening there" );
            continue;
        }
        m_browserWebSocketUrl = "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + path;
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "DevToolsBrowser: Using the browser that is already running, which allows remote debugging (" << dirs[ i ] << ")" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::Launch( const CORE::CString& startUrl, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const CORE::CString exe = FindExecutable();
    if ( exe.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DevToolsBrowser: Unable to find Chrome or Chromium. Install it or configure its path" );
        return false;
    }
    if ( !m_profileDir.IsNULLOrEmpty() )
    {
        if ( !CORE::DirExists( m_profileDir ) && !CORE::CreateDirs( m_profileDir ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DevToolsBrowser: Unable to create the profile folder " << m_profileDir );
            return false;
        }
        CORE::SetOwnerOnlyAccess( m_profileDir );
    }

    // detached: the browser holds the login, it has to outlive us
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: Starting " << exe << ( m_headless ? " without a window" : " with a window" ) );
    const CORE::CString::StringVector arguments = BuildArguments( startUrl );
    CORE::CProcess::TStringVector args( arguments.begin(), arguments.end() );
    CORE::CString launchExe = exe;
    CORE::CProcess::TStringMap overrides;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    TEnvironment environment;
    for ( char** e = environ; GUCEF_NULL != e && GUCEF_NULL != *e; ++e )
    {
        const char* entry = *e;
        const char* eq = strchr( entry, '=' );
        if ( GUCEF_NULL != eq )
            environment.push_back( TNameValue( CORE::CString( entry, (UInt32) ( eq - entry ) ), CORE::CString( eq + 1 ) ) );
    }
    CORE::CString::StringVector unsetNames;
    TStringMap restored;
    BuildCleanEnvironment( environment, unsetNames, restored );
    if ( !unsetNames.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: This terminal comes from a snap package, starting the browser without the snap's environment" );
        // env removes the variables, CProcess can only add
        CORE::CProcess::TStringVector wrapped;
        for ( size_t i=0; i<unsetNames.size(); ++i )
        {
            wrapped.push_back( "-u" );
            wrapped.push_back( unsetNames[ i ] );
        }
        wrapped.push_back( exe );
        wrapped.insert( wrapped.end(), args.begin(), args.end() );
        args.swap( wrapped );
        launchExe = "/usr/bin/env";
        for ( TStringMap::const_iterator r = restored.begin(); r != restored.end(); ++r )
            overrides[ r->first ] = r->second;
    }
    #endif

    if ( !m_process.Launch( launchExe, args, CORE::CString::Empty, overrides, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DevToolsBrowser: Failed to start " << exe );
        return false;
    }
    m_launched = true;
    m_browserWebSocketUrl.Clear();

    const UInt64 start = MT::PrecisionTickCount();
    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    bool warned = false;
    while ( ( MT::PrecisionTickCount() - start ) / ticksPerMs < timeoutInMs )
    {
        if ( IsAnswering( 1000 ) )
            return true;
        if ( !warned && !m_process.IsRunning() )
        {
            // chrome hands over to an instance that is already running when it finds one using the same profile
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: The browser process ended right away, is a browser with this profile already running without the debugging port?" );
            warned = true;
        }
        MT::ThreadDelay( 200 );
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DevToolsBrowser: The browser did not open the debugging port in time" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( Attach() )
        return true;
    return Launch( startUrl, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::Shutdown( void )
{GUCEF_TRACE;

    if ( !m_launched )
        return true;
    m_launched = false;
    return m_process.Terminate( true, 5000 );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsBrowser::ConnectToBrowser( CChromeDevToolsClient& client, UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    if ( !m_browserWebSocketUrl.IsNULLOrEmpty() )
    {
        // the browser asks its user whether this is allowed
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "DevToolsBrowser: Connecting to the browser. If it asks whether to allow remote debugging, allow it." );
        return client.ConnectAndWait( m_browserWebSocketUrl, timeoutInMs < 90000 ? 90000 : timeoutInMs );
    }
    SChromeDiscoveryResult result;
    if ( !client.DiscoverTargetsAndWait( "127.0.0.1", m_debugPort, result, timeoutInMs ) )
        return false;
    if ( result.browser.webSocketDebuggerUrl.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsBrowser: The browser did not give a web socket url for itself" );
        return false;
    }
    return client.ConnectAndWait( result.browser.webSocketDebuggerUrl, timeoutInMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
