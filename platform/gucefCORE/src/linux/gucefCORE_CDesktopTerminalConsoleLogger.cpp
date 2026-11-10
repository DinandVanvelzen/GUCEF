/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CDesktopTerminalConsoleLogger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** how a terminal emulator wants to be told what to run */
enum ETerminalCommandStyle
{
    TERMINALSTYLE_DIRECT = 0    ,   /**< terminal [options] command args */
    TERMINALSTYLE_DASH_E        ,   /**< terminal [options] -e command args */
    TERMINALSTYLE_DASH_X        ,   /**< terminal [options] -x command args */
    TERMINALSTYLE_DASHDASH          /**< terminal [options] -- command args */
};

/** how a terminal emulator wants to be told the window title, if it can */
enum ETerminalTitleStyle
{
    TERMINALTITLE_NONE = 0      ,
    TERMINALTITLE_DASH_T        ,   /**< -T title */
    TERMINALTITLE_LONG_EQ       ,   /**< --title=title */
    TERMINALTITLE_LONG_SPACE        /**< --title title */
};

struct SKnownTerminal
{
    const char* name;
    ETerminalCommandStyle commandStyle;
    ETerminalTitleStyle titleStyle;
    const char* extraOption;        /**< option to always pass, or NULL */
};

/** In order of preference after the explicitly configured ones */
static const SKnownTerminal g_knownTerminals[] =
{
    { "xdg-terminal-exec"   , TERMINALSTYLE_DIRECT   , TERMINALTITLE_NONE       , NULL               },
    { "x-terminal-emulator" , TERMINALSTYLE_DASH_E   , TERMINALTITLE_NONE       , NULL               },
    { "ptyxis"              , TERMINALSTYLE_DASHDASH , TERMINALTITLE_LONG_EQ    , "--new-window"     },
    { "gnome-terminal"      , TERMINALSTYLE_DASHDASH , TERMINALTITLE_LONG_EQ    , NULL               },
    { "konsole"             , TERMINALSTYLE_DASH_E   , TERMINALTITLE_NONE       , "--separate"       },
    { "xfce4-terminal"      , TERMINALSTYLE_DASH_X   , TERMINALTITLE_DASH_T     , "--disable-server" },
    { "kgx"                 , TERMINALSTYLE_DASH_E   , TERMINALTITLE_NONE       , NULL               },
    { "kitty"               , TERMINALSTYLE_DIRECT   , TERMINALTITLE_LONG_SPACE , NULL               },
    { "alacritty"           , TERMINALSTYLE_DASH_E   , TERMINALTITLE_LONG_SPACE , NULL               },
    { "foot"                , TERMINALSTYLE_DIRECT   , TERMINALTITLE_LONG_SPACE , NULL               },
    { "xterm"               , TERMINALSTYLE_DASH_E   , TERMINALTITLE_DASH_T     , NULL               }
};

static const UInt32 g_knownTerminalCount = sizeof( g_knownTerminals ) / sizeof( g_knownTerminals[ 0 ] );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Blocks SIGPIPE for the calling thread for as long as the object exists, and takes away the signal that
 *  a write may have caused, so a closed pipe is reported as an error code instead of killing the application
 */
class CSigPipeGuard
{
    public:

    CSigPipeGuard( void )
        : m_wasPending( false )
    {
        sigemptyset( &m_pipeSet );
        sigaddset( &m_pipeSet, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &m_pipeSet, &m_oldMask );

        sigset_t pending;
        sigemptyset( &pending );
        sigpending( &pending );
        m_wasPending = 0 != sigismember( &pending, SIGPIPE );
    }

    ~CSigPipeGuard()
    {
        if ( !m_wasPending )
        {
            sigset_t pending;
            sigemptyset( &pending );
            sigpending( &pending );
            if ( 0 != sigismember( &pending, SIGPIPE ) )
            {
                struct timespec noWait;
                noWait.tv_sec = 0;
                noWait.tv_nsec = 0;
                sigtimedwait( &m_pipeSet, NULL, &noWait );
            }
        }
        pthread_sigmask( SIG_SETMASK, &m_oldMask, NULL );
    }

    private:

    sigset_t m_pipeSet;
    sigset_t m_oldMask;
    bool m_wasPending;
};

/*-------------------------------------------------------------------------*/

static CString
BaseName( const CString& path )
{
    Int32 lastSlash = path.HasChar( '/', false );
    if ( lastSlash >= 0 )
        return path.SubstrToIndex( (UInt32) lastSlash+1, false );
    return path;
}

/*-------------------------------------------------------------------------*/

static const SKnownTerminal*
FindKnownTerminal( const CString& nameOrPath )
{
    CString name = BaseName( nameOrPath );
    for ( UInt32 i=0; i<g_knownTerminalCount; ++i )
    {
        if ( name == g_knownTerminals[ i ].name )
            return &g_knownTerminals[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static void
BuildTerminalArgs( ETerminalCommandStyle commandStyle         ,
                   ETerminalTitleStyle titleStyle             ,
                   const char* extraOption                    ,
                   const CString& windowTitle                 ,
                   const CProcess::TStringVector& command     ,
                   CProcess::TStringVector& args              )
{
    args.clear();
    if ( GUCEF_NULL != extraOption )
        args.push_back( extraOption );

    if ( !windowTitle.IsNULLOrEmpty() )
    {
        switch ( titleStyle )
        {
            case TERMINALTITLE_DASH_T :         { args.push_back( "-T" ); args.push_back( windowTitle ); break; }
            case TERMINALTITLE_LONG_EQ :        { args.push_back( "--title=" + windowTitle ); break; }
            case TERMINALTITLE_LONG_SPACE :     { args.push_back( "--title" ); args.push_back( windowTitle ); break; }
            default : break;
        }
    }

    switch ( commandStyle )
    {
        case TERMINALSTYLE_DASH_E :     { args.push_back( "-e" ); break; }
        case TERMINALSTYLE_DASH_X :     { args.push_back( "-x" ); break; }
        case TERMINALSTYLE_DASHDASH :   { args.push_back( "--" ); break; }
        default : break;
    }
    args.insert( args.end(), command.begin(), command.end() );
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::IsDesktopSessionAvailable( void )
{GUCEF_TRACE;

    const char* wayland = getenv( "WAYLAND_DISPLAY" );
    if ( GUCEF_NULL != wayland && '\0' != wayland[ 0 ] )
        return true;
    const char* display = getenv( "DISPLAY" );
    if ( GUCEF_NULL != display && '\0' != display[ 0 ] )
        return true;
    const char* sessionType = getenv( "XDG_SESSION_TYPE" );
    if ( GUCEF_NULL != sessionType && ( 0 == strcmp( sessionType, "x11" ) || 0 == strcmp( sessionType, "wayland" ) ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
IsExecutableFile( const CString& path )
{
    struct stat info;
    return 0 == stat( path.C_String(), &info ) && S_ISREG( info.st_mode ) && 0 == access( path.C_String(), X_OK );
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::FindExecutableInPath( const CString& name ,
                                                     CString& fullPath   )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;

    if ( name.HasChar( '/', true ) >= 0 )
    {
        if ( IsExecutableFile( name ) )
        {
            fullPath = name;
            return true;
        }
        return false;
    }

    const char* pathEnv = getenv( "PATH" );
    if ( GUCEF_NULL == pathEnv )
        return false;

    CString::StringVector dirs = CString( pathEnv ).ParseElements( ':', false );
    for ( CString::StringVector::iterator i = dirs.begin(); i != dirs.end(); ++i )
    {
        if ( (*i).IsNULLOrEmpty() )
            continue;
        CString candidate = (*i) + "/" + name;
        if ( IsExecutableFile( candidate ) )
        {
            fullPath = candidate;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::DiscoverTerminal( const CString& preferredTerminal         ,
                                                 const CString& windowTitle               ,
                                                 const CProcess::TStringVector& command   ,
                                                 CString& exePath                         ,
                                                 CProcess::TStringVector& args            ,
                                                 CString& terminalName                    )
{GUCEF_TRACE;

    // The terminals which were explicitly asked for, using the generic -e convention if we do not know them
    CString::StringVector explicitTerminals;
    if ( !preferredTerminal.IsNULLOrEmpty() )
        explicitTerminals.push_back( preferredTerminal );
    const char* terminalEnv = getenv( "TERMINAL" );
    if ( GUCEF_NULL != terminalEnv && '\0' != terminalEnv[ 0 ] )
        explicitTerminals.push_back( terminalEnv );

    for ( CString::StringVector::iterator i = explicitTerminals.begin(); i != explicitTerminals.end(); ++i )
    {
        CString fullPath;
        if ( FindExecutableInPath( (*i), fullPath ) )
        {
            const SKnownTerminal* known = FindKnownTerminal( (*i) );
            if ( GUCEF_NULL != known )
                BuildTerminalArgs( known->commandStyle, known->titleStyle, known->extraOption, windowTitle, command, args );
            else
                BuildTerminalArgs( TERMINALSTYLE_DASH_E, TERMINALTITLE_NONE, GUCEF_NULL, windowTitle, command, args );
            exePath = fullPath;
            terminalName = BaseName( fullPath );
            return true;
        }
    }

    for ( UInt32 i=0; i<g_knownTerminalCount; ++i )
    {
        CString fullPath;
        if ( FindExecutableInPath( g_knownTerminals[ i ].name, fullPath ) )
        {
            BuildTerminalArgs( g_knownTerminals[ i ].commandStyle, g_knownTerminals[ i ].titleStyle, g_knownTerminals[ i ].extraOption, windowTitle, command, args );
            exePath = fullPath;
            terminalName = g_knownTerminals[ i ].name;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDesktopTerminalConsoleLogger::CDesktopTerminalConsoleLogger( bool initialize )
    : CTextLineConsoleLogger()
    , CIConfigurable()
    , m_lock()
    , m_state( STATE_IDLE )
    , m_preferredTerminal()
    , m_windowTitle( "Log" )
    , m_terminalName()
    , m_keepWindowOpen( false )
    , m_enabled( true )
    , m_connectTimeoutInMs( 10000 )
    , m_maxBufferedBytes( 256 * 1024 )
    , m_fifoPath()
    , m_fifoFd( -1 )
    , m_terminalProcess()
    , m_startTicks( 0 )
    , m_pending()
    , m_pendingBytes( 0 )
    , m_droppedLines( 0 )
    , m_droppedSinceLastMarker( 0 )
    , m_partialOffset( 0 )
{GUCEF_TRACE;

    // Applications that do not have their own configuration can still control the logger via the environment
    const char* envValue = getenv( "GUCEF_CONSOLE_TERMINAL" );
    if ( GUCEF_NULL != envValue && '\0' != envValue[ 0 ] )
        m_preferredTerminal = envValue;
    envValue = getenv( "GUCEF_CONSOLE_KEEP_OPEN" );
    if ( GUCEF_NULL != envValue && '\0' != envValue[ 0 ] )
        m_keepWindowOpen = StringToBool( CString( envValue ), false );
    envValue = getenv( "GUCEF_CONSOLE_ENABLED" );
    if ( GUCEF_NULL != envValue && '\0' != envValue[ 0 ] )
        m_enabled = StringToBool( CString( envValue ), true );

    if ( initialize )
        Initialize();
}

/*-------------------------------------------------------------------------*/

CDesktopTerminalConsoleLogger::~CDesktopTerminalConsoleLogger()
{GUCEF_TRACE;

    // A window which is still open reads the end of the log and then closes (unless it was told to stay).
    // A window which never opened the log is not wanted anymore.
    MT::CScopeMutex lock( m_lock );
    if ( STATE_CONNECTING == m_state && m_terminalProcess.IsRunning() )
        m_terminalProcess.Terminate( false );
    Cleanup();
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::Cleanup( void )
{GUCEF_TRACE;

    if ( -1 != m_fifoFd )
    {
        close( m_fifoFd );
        m_fifoFd = -1;
    }
    if ( !m_fifoPath.IsNULLOrEmpty() )
    {
        unlink( m_fifoPath.C_String() );
        m_fifoPath.Clear();
    }
    m_pending.clear();
    m_pendingBytes = 0;
    m_partialOffset = 0;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::Initialize( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return InitializeImp();
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::InitializeImp( void )
{GUCEF_TRACE;

    if ( STATE_IDLE != m_state )
        return STATE_DISABLED != m_state;

    if ( !m_enabled )
    {
        m_state = STATE_DISABLED;
        return false;
    }

    // The FIFO is private to this user
    CString fifoDir;
    const char* runtimeDir = getenv( "XDG_RUNTIME_DIR" );
    if ( GUCEF_NULL != runtimeDir && '\0' != runtimeDir[ 0 ] && 0 == access( runtimeDir, W_OK ) )
        fifoDir = runtimeDir;
    else
        fifoDir = "/tmp";

    static UInt32 s_instanceCounter = 0;
    CString fifoPath = fifoDir + "/gucef-console-" + UInt32ToString( (UInt32) getpid() ) + "-" + UInt32ToString( ++s_instanceCounter ) + ".fifo";
    unlink( fifoPath.C_String() );
    if ( 0 != mkfifo( fifoPath.C_String(), 0600 ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DesktopTerminalConsoleLogger: Failed to create the FIFO at \"" + fifoPath + "\", console logger is disabled" );
        m_state = STATE_DISABLED;
        return false;
    }

    // What the window runs: set the window title and then show whatever comes through the FIFO
    CProcess::TStringVector command;
    command.push_back( "sh" );
    command.push_back( "-c" );
    command.push_back( "printf '\\033]0;%s\\007' \"$1\"; cat \"$2\"; if [ \"$3\" = 1 ]; then printf '\\n[The log has ended, press Enter to close this window]\\n'; read dummy; fi" );
    command.push_back( "gucefconsole" );
    command.push_back( m_windowTitle );
    command.push_back( fifoPath );
    command.push_back( m_keepWindowOpen ? "1" : "0" );

    CString exePath;
    CProcess::TStringVector args;
    if ( !DiscoverTerminal( m_preferredTerminal, m_windowTitle, command, exePath, args, m_terminalName ) )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DesktopTerminalConsoleLogger: No terminal emulator found, console logger is disabled" );
        unlink( fifoPath.C_String() );
        m_state = STATE_DISABLED;
        return false;
    }

    if ( !m_terminalProcess.Launch( exePath, args, CString(), CProcess::TStringMap(), true ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DesktopTerminalConsoleLogger: Failed to start the terminal emulator \"" + exePath + "\", console logger is disabled" );
        unlink( fifoPath.C_String() );
        m_state = STATE_DISABLED;
        return false;
    }

    m_fifoPath = fifoPath;
    m_startTicks = MT::PrecisionTickCount();
    m_state = STATE_CONNECTING;
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DesktopTerminalConsoleLogger: Started terminal emulator \"" + exePath + "\" to show the log" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::Disable( const CString& reason )
{GUCEF_TRACE;

    m_state = STATE_DISABLED;
    Cleanup();
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DesktopTerminalConsoleLogger: " + reason + ", console logger is disabled" );
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::QueueLine( const CString& line )
{GUCEF_TRACE;

    CString text = line + "\n";
    m_pendingBytes += text.ByteSize() - 1;
    m_pending.push_back( text );

    // The window cannot keep up or is not there yet: keep the newest, the oldest are the first to go
    while ( m_pendingBytes > m_maxBufferedBytes && m_pending.size() > 1 )
    {
        std::deque< CString >::iterator victim = m_pending.begin();
        if ( m_partialOffset > 0 )
            ++victim;       // the line that is partially written is not dropped
        if ( victim == m_pending.end() || ( victim + 1 ) == m_pending.end() )
            break;          // never drop the line that was just added
        m_pendingBytes -= (*victim).ByteSize() - 1;
        m_pending.erase( victim );
        ++m_droppedLines;
        ++m_droppedSinceLastMarker;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::FlushPending( void )
{GUCEF_TRACE;

    // Mark that lines are missing, which are older than everything that is still buffered
    if ( m_droppedSinceLastMarker > 0 && 0 == m_partialOffset )
    {
        CString marker = "[... " + UInt32ToString( m_droppedSinceLastMarker ) + " log lines were dropped because the window could not keep up ...]\n";
        m_pendingBytes += marker.ByteSize() - 1;
        m_pending.push_front( marker );
        m_droppedSinceLastMarker = 0;
    }

    CSigPipeGuard sigPipeGuard;

    while ( !m_pending.empty() )
    {
        const CString& front = m_pending.front();
        UInt32 frontSize = front.ByteSize() - 1;
        ssize_t written = write( m_fifoFd, front.C_String() + m_partialOffset, frontSize - m_partialOffset );
        if ( written < 0 )
        {
            if ( EINTR == errno )
                continue;
            if ( EAGAIN == errno || EWOULDBLOCK == errno )
                return true;        // the window is busy, we try again later
            if ( EPIPE == errno )
                Disable( "The terminal window was closed" );
            else
                Disable( "Failed to write to the terminal window" );
            return false;
        }

        m_partialOffset += (UInt32) written;
        if ( m_partialOffset >= frontSize )
        {
            m_pendingBytes -= frontSize;
            m_pending.pop_front();
            m_partialOffset = 0;
        }
        else
        {
            return true;            // partial write: the window is busy
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::Pump( void )
{GUCEF_TRACE;

    if ( STATE_CONNECTING == m_state )
    {
        // Succeeds as soon as the window has started reading the log
        int fd = open( m_fifoPath.C_String(), O_WRONLY | O_NONBLOCK );
        if ( -1 != fd )
        {
            m_fifoFd = fd;
            m_state = STATE_CONNECTED;

            // Nothing else needs to find the FIFO anymore
            unlink( m_fifoPath.C_String() );
            m_fifoPath.Clear();
        }
        else
        if ( ENXIO == errno )
        {
            Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
            Float64 waitedInMs = ( MT::PrecisionTickCount() - m_startTicks ) / ticksPerMs;
            if ( waitedInMs > m_connectTimeoutInMs )
                Disable( "The terminal window did not open the log in time" );
            return;
        }
        else
        {
            Disable( "Failed to open the log for the terminal window" );
            return;
        }
    }

    if ( STATE_CONNECTED == m_state )
        FlushPending();
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::WriteLine( const CString& line )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( STATE_IDLE == m_state )
        InitializeImp();
    if ( STATE_DISABLED == m_state )
        return;

    QueueLine( line );
    Pump();
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::OnFlushLog( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( STATE_CONNECTING == m_state || STATE_CONNECTED == m_state )
        Pump();
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::IsActive( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return STATE_CONNECTING == m_state || STATE_CONNECTED == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return STATE_CONNECTED == m_state;
}

/*-------------------------------------------------------------------------*/

UInt32
CDesktopTerminalConsoleLogger::GetDroppedLineCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_droppedLines;
}

/*-------------------------------------------------------------------------*/

CString
CDesktopTerminalConsoleLogger::GetTerminalName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_terminalName;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetPreferredTerminal( const CString& terminal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_preferredTerminal = terminal;
}

/*-------------------------------------------------------------------------*/

CString
CDesktopTerminalConsoleLogger::GetPreferredTerminal( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_preferredTerminal;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetKeepWindowOpenOnExit( bool keepOpen )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_keepWindowOpen = keepOpen;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::GetKeepWindowOpenOnExit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_keepWindowOpen;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetEnabled( bool enabled )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_enabled = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::GetEnabled( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetWindowTitle( const CString& title )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_windowTitle = title;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetConnectTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_connectTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CDesktopTerminalConsoleLogger::SetMaxBufferedBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxBufferedBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::SaveConfig( CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CDataNode* node = config.Structure( "GUCEF%CORE%CDesktopTerminalConsoleLogger", '%' );
    if ( GUCEF_NULL == node )
        return false;
    node->SetAttribute( "Enabled", BoolToString( m_enabled ) );
    node->SetAttribute( "PreferredTerminal", m_preferredTerminal );
    node->SetAttribute( "KeepWindowOpenOnExit", BoolToString( m_keepWindowOpen ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopTerminalConsoleLogger::LoadConfig( const CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const CDataNode* node = config.Search( "GUCEF%CORE%CDesktopTerminalConsoleLogger", '%', false );
    if ( GUCEF_NULL == node )
        return false;

    const CDataNode::TKeyValuePair* att = node->GetAttribute( "Enabled" );
    if ( GUCEF_NULL != att )
        m_enabled = att->second.AsBool( true );
    att = node->GetAttribute( "PreferredTerminal" );
    if ( GUCEF_NULL != att )
        m_preferredTerminal = att->second;
    att = node->GetAttribute( "KeepWindowOpenOnExit" );
    if ( GUCEF_NULL != att )
        m_keepWindowOpen = att->second.AsBool( false );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CDesktopTerminalConsoleLogger::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDesktopTerminalConsoleLogger";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
