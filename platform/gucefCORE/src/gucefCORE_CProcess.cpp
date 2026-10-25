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

#include <string>

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #include <windows.h>

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

  // posix_spawn() is part of Android's C library from API level 28, below that a child process is made with
  // fork() and execve(). Defining GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN on any platform selects fork() and execve()
  #if ( !defined( GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN ) && ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) && ( !defined( __ANDROID_API__ ) || ( __ANDROID_API__ < 28 ) ) )
    #define GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN
  #endif

  #ifndef GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN
    #include <spawn.h>
  #endif
  #include <signal.h>
  #include <errno.h>
  #include <string.h>
  #include <stdlib.h>
  #include <paths.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <dirent.h>
  #include <sys/types.h>
  #include <sys/wait.h>

  #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )
    #include <crt_externs.h>
    #define environ ( *_NSGetEnviron() )
  #else
    extern char** environ;
  #endif

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CProcess::CProcess( void )
    : m_processId( 0 )
    , m_osHandle( GUCEF_NULL )
    , m_isDetached( false )
    , m_isolate( false )
    , m_hasExited( false )
    , m_exitCode( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProcess::~CProcess()
{GUCEF_TRACE;

    // A detached process is meant to outlive us, anything else must not be left behind
    if ( 0 != m_processId && !m_isDetached && !m_hasExited )
        Terminate( false );

    ReleaseHandle();
}

/*-------------------------------------------------------------------------*/

void
CProcess::ReleaseHandle( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( GUCEF_NULL != m_osHandle )
    {
        ::CloseHandle( (HANDLE) m_osHandle );
        m_osHandle = GUCEF_NULL;
    }

    #endif
}

/*-------------------------------------------------------------------------*/

void
CProcess::SetIsolateFromParent( bool isolate )
{GUCEF_TRACE;

    m_isolate = isolate;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::GetIsolateFromParent( void ) const
{GUCEF_TRACE;

    return m_isolate;
}

/*-------------------------------------------------------------------------*/

UInt32
CProcess::GetProcessId( void ) const
{GUCEF_TRACE;

    return m_processId;
}

/*-------------------------------------------------------------------------*/

CProcess::TStringVector
CProcess::SplitCommandLine( const CString& commandLine )
{GUCEF_TRACE;

    TStringVector args;
    std::string current;
    bool inToken = false;
    char quoteChar = 0;

    const std::string& line = commandLine.STL_String();
    for ( size_t i=0; i<line.size(); ++i )
    {
        char c = line[ i ];
        if ( 0 != quoteChar )
        {
            if ( c == quoteChar )
            {
                quoteChar = 0;
            }
            else
            if ( '\\' == c && '"' == quoteChar && i+1 < line.size() && ( '"' == line[ i+1 ] || '\\' == line[ i+1 ] ) )
            {
                current += line[ ++i ];
            }
            else
            {
                current += c;
            }
        }
        else
        if ( '"' == c || '\'' == c )
        {
            quoteChar = c;
            inToken = true;
        }
        else
        if ( ' ' == c || '\t' == c || '\r' == c || '\n' == c )
        {
            if ( inToken )
            {
                args.push_back( current );
                current.clear();
                inToken = false;
            }
        }
        else
        {
            current += c;
            inToken = true;
        }
    }
    if ( inToken )
        args.push_back( current );
    return args;
}

/*-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

/**
 *  The file descriptors this process has open apart from the standard three: what an isolated child must not get. The directory
 *  that lists them is itself a descriptor while it is read, which is closed again before the list is used, so it is left out.
 */
static void
ListInheritableDescriptors( GUCEF::vector< int >& descriptors )
{GUCEF_TRACE;

    descriptors.clear();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS )
    DIR* directory = opendir( "/dev/fd" );
    #else
    DIR* directory = opendir( "/proc/self/fd" );
    #endif
    if ( GUCEF_NULL != directory )
    {
        const int directoryDescriptor = dirfd( directory );
        struct dirent* entry = GUCEF_NULL;
        while ( GUCEF_NULL != ( entry = readdir( directory ) ) )
        {
            char* end = GUCEF_NULL;
            const long value = strtol( entry->d_name, &end, 10 );
            if ( end != entry->d_name && '\0' == *end && value > 2 && value != directoryDescriptor )
                descriptors.push_back( (int) value );
        }
        closedir( directory );
        return;
    }

    // no listing: every number up to the limit
    long limit = sysconf( _SC_OPEN_MAX );
    if ( limit < 0 || limit > 65536 )
        limit = 65536;
    for ( long fd=3; fd<limit; ++fd )
        descriptors.push_back( (int) fd );
}

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN

/**
 *  Starts the child process with posix_spawn()
 *  @return 0 if the process was started, otherwise the error code of why not
 */
static int
SpawnChildProcess( const CString& exePath       ,
                   char* const* argv            ,
                   char* const* envp            ,
                   const CString& workingDir    ,
                   bool detach                  ,
                   bool isolate                 ,
                   const GUCEF::vector< int >& descriptorsToClose ,
                   pid_t& pid                   )
{GUCEF_TRACE;

    posix_spawn_file_actions_t fileActions;
    posix_spawnattr_t spawnAttr;
    posix_spawn_file_actions_init( &fileActions );
    posix_spawnattr_init( &spawnAttr );

    bool success = true;
    short spawnFlags = 0;

    if ( isolate )
    {
        // nothing of the terminal in, nothing of ours out: the three standard streams are /dev/null
        success = 0 == posix_spawn_file_actions_addopen( &fileActions, 0, "/dev/null", O_RDONLY, 0 ) &&
                  0 == posix_spawn_file_actions_addopen( &fileActions, 1, "/dev/null", O_WRONLY, 0 ) &&
                  0 == posix_spawn_file_actions_addopen( &fileActions, 2, "/dev/null", O_WRONLY, 0 );
        #ifdef POSIX_SPAWN_CLOEXEC_DEFAULT
        spawnFlags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
        #else
        for ( size_t i=0; i<descriptorsToClose.size() && success; ++i )
            success = 0 == posix_spawn_file_actions_addclose( &fileActions, descriptorsToClose[ i ] );
        #endif
    }

    if ( !workingDir.IsNULLOrEmpty() )
    {
        #if ( ( defined( __GLIBC__ ) && ( __GLIBC__ > 2 || ( __GLIBC__ == 2 && __GLIBC_MINOR__ >= 29 ) ) ) || ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) && defined( __ANDROID_API__ ) && ( __ANDROID_API__ >= 34 ) ) )
        success = 0 == posix_spawn_file_actions_addchdir_np( &fileActions, workingDir.C_String() );
        #else
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Setting the working dir of a child process is not supported by this C library" );
        success = false;
        #endif
    }

    if ( detach )
    {
        #ifdef POSIX_SPAWN_SETSID
        spawnFlags |= POSIX_SPAWN_SETSID;
        #else
        spawnFlags |= POSIX_SPAWN_SETPGROUP;
        #endif
    }
    if ( 0 != spawnFlags )
        success = success && 0 == posix_spawnattr_setflags( &spawnAttr, spawnFlags );

    int spawnResult = EINVAL;
    if ( success )
        spawnResult = posix_spawnp( &pid, exePath.C_String(), &fileActions, &spawnAttr, argv, envp );

    posix_spawn_file_actions_destroy( &fileActions );
    posix_spawnattr_destroy( &spawnAttr );
    return spawnResult;
}

#else /* GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN ? */

/**
 *  Starts the child process with fork() and execve(), for C libraries without posix_spawn().
 *  Everything the child needs is prepared before the fork, after it only functions which are safe to
 *  call between fork() and execve() in a multi threaded process are used: setsid(), chdir(), execve(),
 *  write() and _exit(). Whether the exe could be executed is passed from the child to us through a pipe
 *  which is closed by a successful execve().
 *  @return 0 if the process was started, otherwise the error code of why not
 */
static int
SpawnChildProcess( const CString& exePath       ,
                   char* const* argv            ,
                   char* const* envp            ,
                   const CString& workingDir    ,
                   bool detach                  ,
                   bool isolate                 ,
                   const GUCEF::vector< int >& descriptorsToClose ,
                   pid_t& pid                   )
{GUCEF_TRACE;

    // The files to try executing: as given if it holds a directory, otherwise in every directory of the search path
    GUCEF::vector< CString > candidates;
    if ( GUCEF_NULL != strchr( exePath.C_String(), '/' ) )
    {
        candidates.push_back( exePath );
    }
    else
    {
        const char* searchPath = getenv( "PATH" );
        if ( GUCEF_NULL == searchPath )
            searchPath = _PATH_DEFPATH;
        const char* dirStart = searchPath;
        for ( ; ; )
        {
            const char* dirEnd = strchr( dirStart, ':' );
            size_t dirLength = GUCEF_NULL == dirEnd ? strlen( dirStart ) : (size_t) ( dirEnd - dirStart );
            CString candidate;
            if ( dirLength > 0 )
            {
                candidate.Append( dirStart, (UInt32) dirLength );
                candidate += "/";
            }
            candidate += exePath;
            candidates.push_back( candidate );
            if ( GUCEF_NULL == dirEnd )
                break;
            dirStart = dirEnd + 1;
        }
    }
    GUCEF::vector< const char* > candidatePaths;
    for ( size_t i=0; i<candidates.size(); ++i )
        candidatePaths.push_back( candidates[ i ].C_String() );
    const char* const* candidatePathArray = &candidatePaths[ 0 ];
    const size_t candidateCount = candidatePaths.size();
    const char* workingDirPath = workingDir.IsNULLOrEmpty() ? GUCEF_NULL : workingDir.C_String();

    const int* closeList = descriptorsToClose.empty() ? GUCEF_NULL : &descriptorsToClose[ 0 ];
    const size_t closeCount = descriptorsToClose.size();

    int errorPipe[ 2 ] = { -1, -1 };
    if ( 0 != pipe2( errorPipe, O_CLOEXEC ) )
        return errno;

    pid = fork();
    if ( pid < 0 )
    {
        int forkError = errno;
        close( errorPipe[ 0 ] );
        close( errorPipe[ 1 ] );
        return forkError;
    }

    if ( 0 == pid )
    {
        // The child, nothing but the functions that are safe here
        int childError = 0;
        if ( detach && setsid() < 0 )
            childError = errno;
        if ( 0 == childError && isolate )
        {
            // open() and dup2() and close() are safe here; the descriptors were listed before the fork
            int devNull = open( "/dev/null", O_RDWR );
            if ( devNull < 0 )
                childError = errno;
            else
            {
                for ( int standard=0; standard<3; ++standard )
                    if ( dup2( devNull, standard ) < 0 )
                        childError = errno;
                if ( devNull > 2 )
                    close( devNull );
            }
            for ( size_t i=0; i<closeCount; ++i )
                close( closeList[ i ] );
        }
        if ( 0 == childError && GUCEF_NULL != workingDirPath && 0 != chdir( workingDirPath ) )
            childError = errno;
        if ( 0 == childError )
        {
            bool accessDenied = false;
            for ( size_t i=0; i<candidateCount; ++i )
            {
                execve( candidatePathArray[ i ], argv, envp );
                childError = errno;
                if ( EACCES == childError )
                    accessDenied = true;
            }
            if ( accessDenied )
                childError = EACCES;
        }
        ssize_t written = write( errorPipe[ 1 ], &childError, sizeof( childError ) );
        (void) written;
        _exit( 127 );
    }

    // The pipe is closed when the child executes the exe, or holds why that failed
    close( errorPipe[ 1 ] );
    int childError = 0;
    size_t bytesRead = 0;
    while ( bytesRead < sizeof( childError ) )
    {
        ssize_t result = read( errorPipe[ 0 ], reinterpret_cast< char* >( &childError ) + bytesRead, sizeof( childError ) - bytesRead );
        if ( result < 0 && EINTR == errno )
            continue;
        if ( result <= 0 )
            break;
        bytesRead += (size_t) result;
    }
    close( errorPipe[ 0 ] );

    if ( sizeof( childError ) == bytesRead )
    {
        // The child never became the exe, it is on its way out
        int status = 0;
        while ( waitpid( pid, &status, 0 ) < 0 && EINTR == errno ) {}
        pid = 0;
        return 0 != childError ? childError : ENOEXEC;
    }
    return 0;
}

#endif /* GUCEF_CORE_CPROCESS_NO_POSIX_SPAWN ? */

/*-------------------------------------------------------------------------*/

bool
CProcess::Launch( const CString& exePath          ,
                  const TStringVector& args       ,
                  const CString& workingDir       ,
                  const TStringMap& envOverrides  ,
                  bool detach                     )
{GUCEF_TRACE;

    if ( exePath.IsNULLOrEmpty() )
        return false;
    if ( 0 != m_processId && !m_hasExited && IsRunning() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Launch called while a process is still running" );
        return false;
    }

    // Everything the child needs is prepared before spawning
    std::vector< std::string > argStrings;
    argStrings.push_back( exePath.STL_String() );
    for ( size_t i=0; i<args.size(); ++i )
        argStrings.push_back( args[ i ].STL_String() );
    std::vector< char* > argv;
    for ( size_t i=0; i<argStrings.size(); ++i )
        argv.push_back( const_cast< char* >( argStrings[ i ].c_str() ) );
    argv.push_back( GUCEF_NULL );

    std::map< std::string, std::string > env;
    for ( char** e = environ; GUCEF_NULL != e && GUCEF_NULL != *e; ++e )
    {
        const char* eq = strchr( *e, '=' );
        if ( GUCEF_NULL != eq )
            env[ std::string( *e, eq - *e ) ] = eq + 1;
    }
    for ( TStringMap::const_iterator i = envOverrides.begin(); i != envOverrides.end(); ++i )
        env[ (*i).first.STL_String() ] = (*i).second.STL_String();
    std::vector< std::string > envStrings;
    for ( std::map< std::string, std::string >::const_iterator i = env.begin(); i != env.end(); ++i )
        envStrings.push_back( (*i).first + "=" + (*i).second );
    std::vector< char* > envp;
    for ( size_t i=0; i<envStrings.size(); ++i )
        envp.push_back( const_cast< char* >( envStrings[ i ].c_str() ) );
    envp.push_back( GUCEF_NULL );

    GUCEF::vector< int > descriptorsToClose;
    if ( m_isolate )
        ListInheritableDescriptors( descriptorsToClose );

    pid_t pid = 0;
    int spawnResult = SpawnChildProcess( exePath, &argv[ 0 ], &envp[ 0 ], workingDir, detach, m_isolate, descriptorsToClose, pid );
    bool success = 0 == spawnResult;

    if ( !success )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Failed to launch \"" + exePath + "\": " + CString( strerror( spawnResult ) ) );
        return false;
    }

    m_processId = (UInt32) pid;
    m_isDetached = detach;
    m_hasExited = false;
    m_exitCode = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Reap( void )
{GUCEF_TRACE;

    if ( 0 == m_processId )
        return false;
    if ( m_hasExited )
        return true;

    int status = 0;
    pid_t result = ::waitpid( (pid_t) m_processId, &status, WNOHANG );
    if ( result == (pid_t) m_processId )
    {
        m_hasExited = true;
        if ( WIFEXITED( status ) )
            m_exitCode = WEXITSTATUS( status );
        else
        if ( WIFSIGNALED( status ) )
            m_exitCode = 128 + WTERMSIG( status );
        return true;
    }
    if ( -1 == result && ECHILD == errno )
    {
        // Somebody else reaped it (or SIGCHLD is ignored): it is gone but we cannot know the exit code
        m_hasExited = true;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::IsRunning( void )
{GUCEF_TRACE;

    if ( 0 == m_processId )
        return false;
    return !Reap();
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Terminate( bool graceful, UInt32 gracePeriodInMs )
{GUCEF_TRACE;

    if ( 0 == m_processId )
        return true;
    if ( !IsRunning() )
        return true;

    Int32 exitCode = 0;
    if ( graceful )
    {
        ::kill( (pid_t) m_processId, SIGTERM );
        if ( WaitForExit( gracePeriodInMs, exitCode ) )
            return true;
    }

    ::kill( (pid_t) m_processId, SIGKILL );
    return WaitForExit( 5000, exitCode );
}

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/**
 *  Quotes an argument so that CommandLineToArgvW and the MSVC runtime parse it back to the original
 */
static std::wstring
QuoteWindowsArg( const std::wstring& arg )
{
    if ( !arg.empty() && std::wstring::npos == arg.find_first_of( L" \t\n\v\"" ) )
        return arg;

    std::wstring quoted = L"\"";
    for ( size_t i=0; ; ++i )
    {
        size_t backslashes = 0;
        while ( i < arg.size() && L'\\' == arg[ i ] )
        {
            ++i;
            ++backslashes;
        }
        if ( i == arg.size() )
        {
            quoted.append( backslashes * 2, L'\\' );
            break;
        }
        else
        if ( L'"' == arg[ i ] )
        {
            quoted.append( backslashes * 2 + 1, L'\\' );
            quoted += L'"';
        }
        else
        {
            quoted.append( backslashes, L'\\' );
            quoted += arg[ i ];
        }
    }
    quoted += L'"';
    return quoted;
}

/*-------------------------------------------------------------------------*/

static std::wstring
UpperCase( std::wstring str )
{
    for ( size_t i=0; i<str.size(); ++i )
        str[ i ] = (wchar_t) ::towupper( str[ i ] );
    return str;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Launch( const CString& exePath          ,
                  const TStringVector& args       ,
                  const CString& workingDir       ,
                  const TStringMap& envOverrides  ,
                  bool detach                     )
{GUCEF_TRACE;

    if ( exePath.IsNULLOrEmpty() )
        return false;
    if ( 0 != m_processId && !m_hasExited && IsRunning() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Launch called while a process is still running" );
        return false;
    }

    std::wstring commandLine = QuoteWindowsArg( ToWString( exePath ) );
    for ( size_t i=0; i<args.size(); ++i )
        commandLine += L" " + QuoteWindowsArg( ToWString( args[ i ] ) );

    // Environment block: our environment plus overrides, sorted by upper cased name as Windows expects
    std::map< std::wstring, std::wstring > env;     // upper cased name -> full "Name=Value"
    LPWCH currentEnv = ::GetEnvironmentStringsW();
    if ( GUCEF_NULL != currentEnv )
    {
        for ( LPWCH e = currentEnv; 0 != *e; e += wcslen( e ) + 1 )
        {
            std::wstring entry( e );
            size_t eq = entry.find( L'=', 1 );  // names such as "=C:" start with =
            if ( std::wstring::npos != eq )
                env[ UpperCase( entry.substr( 0, eq ) ) ] = entry;
        }
        ::FreeEnvironmentStringsW( currentEnv );
    }
    for ( TStringMap::const_iterator i = envOverrides.begin(); i != envOverrides.end(); ++i )
    {
        std::wstring name = ToWString( (*i).first );
        env[ UpperCase( name ) ] = name + L"=" + ToWString( (*i).second );
    }
    std::wstring envBlock;
    for ( std::map< std::wstring, std::wstring >::const_iterator i = env.begin(); i != env.end(); ++i )
    {
        envBlock += (*i).second;
        envBlock += L'\0';
    }
    envBlock += L'\0';

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset( &si, 0, sizeof( si ) );
    memset( &pi, 0, sizeof( pi ) );
    si.cb = sizeof( si );

    DWORD flags = CREATE_UNICODE_ENVIRONMENT;
    if ( detach )
        flags |= DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP;

    std::wstring wWorkingDir = ToWString( workingDir );
    std::wstring wExePath = ToWString( exePath );
    std::vector< wchar_t > mutableCommandLine( commandLine.begin(), commandLine.end() );
    mutableCommandLine.push_back( L'\0' );

    BOOL result = ::CreateProcessW( GUCEF_NULL, &mutableCommandLine[ 0 ], GUCEF_NULL, GUCEF_NULL, FALSE, flags,
                                    (LPVOID) envBlock.c_str(),
                                    wWorkingDir.empty() ? GUCEF_NULL : wWorkingDir.c_str(),
                                    &si, &pi );
    if ( !result )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Failed to launch \"" + exePath + "\": error code " + ToString( (UInt32) ::GetLastError() ) );
        return false;
    }

    ::CloseHandle( pi.hThread );
    ReleaseHandle();
    m_osHandle = pi.hProcess;
    m_processId = (UInt32) pi.dwProcessId;
    m_isDetached = detach;
    m_hasExited = false;
    m_exitCode = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Reap( void )
{GUCEF_TRACE;

    if ( 0 == m_processId || GUCEF_NULL == m_osHandle )
        return false;
    if ( m_hasExited )
        return true;

    DWORD exitCode = 0;
    if ( ::GetExitCodeProcess( (HANDLE) m_osHandle, &exitCode ) && STILL_ACTIVE != exitCode )
    {
        m_hasExited = true;
        m_exitCode = (Int32) exitCode;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::IsRunning( void )
{GUCEF_TRACE;

    if ( 0 == m_processId )
        return false;
    return !Reap();
}

/*-------------------------------------------------------------------------*/

static BOOL CALLBACK
PostCloseToProcessWindow( HWND window, LPARAM processId )
{
    DWORD windowProcessId = 0;
    ::GetWindowThreadProcessId( window, &windowProcessId );
    if ( windowProcessId == (DWORD) processId )
        ::PostMessage( window, WM_CLOSE, 0, 0 );
    return TRUE;
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Terminate( bool graceful, UInt32 gracePeriodInMs )
{GUCEF_TRACE;

    if ( 0 == m_processId || !IsRunning() )
        return true;

    Int32 exitCode = 0;
    if ( graceful )
    {
        ::EnumWindows( PostCloseToProcessWindow, (LPARAM) m_processId );
        if ( WaitForExit( gracePeriodInMs, exitCode ) )
            return true;
    }

    ::TerminateProcess( (HANDLE) m_osHandle, 1 );
    return WaitForExit( 5000, exitCode );
}

#else

bool
CProcess::Launch( const CString&, const TStringVector&, const CString&, const TStringMap&, bool )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CProcess::Launch: Platform has no supported implementation" );
    return false;
}

bool CProcess::Reap( void ) {GUCEF_TRACE; return false; }
bool CProcess::IsRunning( void ) {GUCEF_TRACE; return false; }
bool CProcess::Terminate( bool, UInt32 ) {GUCEF_TRACE; return false; }

#endif

/*-------------------------------------------------------------------------*/

bool
CProcess::WaitForExit( UInt32 timeoutInMs, Int32& exitCode )
{GUCEF_TRACE;

    if ( 0 == m_processId )
        return false;

    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    do
    {
        if ( Reap() )
        {
            exitCode = m_exitCode;
            return true;
        }
        MT::ThreadDelay( 5 );
    }
    while ( GUCEF_UINT32MAX == timeoutInMs || ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs < (Float64) timeoutInMs );

    // one last check so that a timeout of 0 still sees an already ended process
    if ( Reap() )
    {
        exitCode = m_exitCode;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
