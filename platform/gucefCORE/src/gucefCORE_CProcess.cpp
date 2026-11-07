/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_STDOUTCLAIM_H
#include "gucefCORE_StdoutClaim.h"
#define GUCEF_CORE_STDOUTCLAIM_H
#endif /* GUCEF_CORE_STDOUTCLAIM_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

  // pipe2() is not part of the C library of macOS: there a pipe is made with pipe() and fcntl()
  #if defined( __APPLE__ ) && !defined( GUCEF_CORE_CPROCESS_NO_PIPE2 )
    #define GUCEF_CORE_CPROCESS_NO_PIPE2
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

    m_pipes[ 0 ] = GUCEF_NULL;
    m_pipes[ 1 ] = GUCEF_NULL;
    m_pipes[ 2 ] = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CProcess::~CProcess()
{GUCEF_TRACE;

    // A detached process is meant to outlive us, anything else must not be left behind
    if ( 0 != m_processId && !m_isDetached && !m_hasExited )
        Terminate( false );

    ReleasePipes();
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
CProcess::ReleasePipes( void )
{GUCEF_TRACE;

    for ( int i=0; i<3; ++i )
    {
        GUCEF_DELETE m_pipes[ i ];
        m_pipes[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CProcessPipe*
CProcess::GetStdinPipe( void ) const
{GUCEF_TRACE;

    return m_pipes[ 0 ];
}

/*-------------------------------------------------------------------------*/

CProcessPipe*
CProcess::GetStdoutPipe( void ) const
{GUCEF_TRACE;

    return m_pipes[ 1 ];
}

/*-------------------------------------------------------------------------*/

CProcessPipe*
CProcess::GetStderrPipe( void ) const
{GUCEF_TRACE;

    return m_pipes[ 2 ];
}

/*-------------------------------------------------------------------------*/

void
CProcess::CloseStdinPipe( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_pipes[ 0 ] )
        m_pipes[ 0 ]->Close();
}

/*-------------------------------------------------------------------------*/

bool
CProcess::Launch( const CString& exePath          ,
                  const TStringVector& args       ,
                  const CString& workingDir       ,
                  const TStringMap& envOverrides  ,
                  bool detach                     )
{GUCEF_TRACE;

    return LaunchImpl( exePath, args, workingDir, envOverrides, detach, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CProcess::LaunchWithPipes( const CString& exePath          ,
                           const TStringVector& args       ,
                           bool pipeStdin                  ,
                           bool pipeStdout                 ,
                           bool pipeStderr                 ,
                           const CString& workingDir       ,
                           const TStringMap& envOverrides  )
{GUCEF_TRACE;

    UInt32 pipeMask = ( pipeStdin ? 1 : 0 ) | ( pipeStdout ? 2 : 0 ) | ( pipeStderr ? 4 : 0 );
    return LaunchImpl( exePath, args, workingDir, envOverrides, false, pipeMask );
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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

/**
 *  Held from the moment the pipes of a launch are made until the child has them as its standard streams. Where
 *  pipe2() exists the pipe ends are close-on-exec from the start and the lock only orders the launches. Without it
 *  (macOS) pipe() and fcntl() are two steps, and the lock keeps a launch of this class on another thread from
 *  inheriting the ends in between; a fork() by other code in that moment is beyond what a library can prevent.
 */
static MT::CMutex g_launchLock;

/**
 *  Makes a pipe whose two ends are close-on-exec
 *  @return 0 or the errno of why not
 */
static int
MakeCloseOnExecPipe( int ends[ 2 ] )
{GUCEF_TRACE;

    #ifdef GUCEF_CORE_CPROCESS_NO_PIPE2
    if ( 0 != pipe( ends ) )
        return errno;
    if ( fcntl( ends[ 0 ], F_SETFD, FD_CLOEXEC ) < 0 || fcntl( ends[ 1 ], F_SETFD, FD_CLOEXEC ) < 0 )
    {
        int error = errno;
        close( ends[ 0 ] );
        close( ends[ 1 ] );
        ends[ 0 ] = ends[ 1 ] = -1;
        return error;
    }
    return 0;
    #else
    return 0 == pipe2( ends, O_CLOEXEC ) ? 0 : errno;
    #endif
}

#endif

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
                   const int* childFds          ,
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
        // nothing of the terminal in, nothing of ours out: the three standard streams are /dev/null, unless they are pipes
        success = ( childFds[ 0 ] >= 0 || 0 == posix_spawn_file_actions_addopen( &fileActions, 0, "/dev/null", O_RDONLY, 0 ) ) &&
                  ( childFds[ 1 ] >= 0 || 0 == posix_spawn_file_actions_addopen( &fileActions, 1, "/dev/null", O_WRONLY, 0 ) ) &&
                  ( childFds[ 2 ] >= 0 || 0 == posix_spawn_file_actions_addopen( &fileActions, 2, "/dev/null", O_WRONLY, 0 ) );
    }

    // the pipes become the standard streams of the child; the descriptors they were made as are closed on exec
    for ( int standard=0; standard<3 && success; ++standard )
        if ( childFds[ standard ] >= 0 )
            success = 0 == posix_spawn_file_actions_adddup2( &fileActions, childFds[ standard ], standard );

    if ( isolate )
    {
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
                   const int* childFds          ,
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
    int errorPipeResult = MakeCloseOnExecPipe( errorPipe );
    if ( 0 != errorPipeResult )
        return errorPipeResult;

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
        }
        // the pipes become the standard streams of the child, before the descriptors of this process are closed
        for ( int standard=0; 0 == childError && standard<3; ++standard )
            if ( childFds[ standard ] >= 0 && dup2( childFds[ standard ], standard ) < 0 )
                childError = errno;
        if ( 0 == childError && isolate )
        {
            for ( size_t i=0; i<closeCount; ++i )
                if ( closeList[ i ] > 2 )
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
CProcess::LaunchImpl( const CString& exePath          ,
                      const TStringVector& args       ,
                      const CString& workingDir       ,
                      const TStringMap& envOverrides  ,
                      bool detach                     ,
                      UInt32 pipeMask                 )
{GUCEF_TRACE;

    if ( exePath.IsNULLOrEmpty() )
        return false;
    if ( 0 != m_processId && !m_hasExited && IsRunning() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Launch called while a process is still running" );
        return false;
    }
    ReleasePipes();

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

    MT::CScopeMutex launchLock( g_launchLock );

    // The pipes: the child gets one end, we keep the other. Both are close-on-exec, the child's copy is made
    // the standard stream by the spawn, so no other child inherits either end
    int childFds[ 3 ] = { -1, -1, -1 };
    int parentFds[ 3 ] = { -1, -1, -1 };
    int pipeError = 0;
    for ( int standard=0; standard<3 && 0 == pipeError; ++standard )
    {
        if ( 0 == ( pipeMask & ( 1u << standard ) ) )
            continue;
        int ends[ 2 ] = { -1, -1 };
        pipeError = MakeCloseOnExecPipe( ends );
        if ( 0 != pipeError )
            break;
        // ends[0] reads, ends[1] writes; the child reads its input and writes its output
        childFds[ standard ] = 0 == standard ? ends[ 0 ] : ends[ 1 ];
        parentFds[ standard ] = 0 == standard ? ends[ 1 ] : ends[ 0 ];
    }

    // While stdout carries a protocol (CStandardOutputWriter::ClaimExclusive()) a child must not write into it: an
    // output that is not piped goes where the standard error of the child goes, its pipe or our standard error
    const bool stdoutToStderr = 0 == ( pipeMask & 2u ) && IsStdoutClaimed();
    if ( stdoutToStderr )
        childFds[ 1 ] = childFds[ 2 ] >= 0 ? childFds[ 2 ] : STDERR_FILENO;

    pid_t pid = 0;
    int spawnResult = 0 != pipeError ? pipeError : SpawnChildProcess( exePath, &argv[ 0 ], &envp[ 0 ], workingDir, detach, m_isolate, descriptorsToClose, childFds, pid );
    bool success = 0 == spawnResult;

    // our own standard streams are not ours to close, and stdout may share the descriptor of stderr
    for ( int standard=0; standard<3; ++standard )
        if ( childFds[ standard ] > STDERR_FILENO && ( 1 != standard || childFds[ 1 ] != childFds[ 2 ] ) )
            close( childFds[ standard ] );

    if ( !success )
    {
        for ( int standard=0; standard<3; ++standard )
            if ( parentFds[ standard ] >= 0 )
                close( parentFds[ standard ] );
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Failed to launch \"" + exePath + "\": " + CString( strerror( spawnResult ) ) );
        return false;
    }

    for ( int standard=0; standard<3; ++standard )
        if ( parentFds[ standard ] >= 0 )
            m_pipes[ standard ] = GUCEF_NEW CProcessPipe( parentFds[ standard ], 0 == standard ? CProcessPipe::PIPE_WRITE_TO_CHILD : CProcessPipe::PIPE_READ_FROM_CHILD );

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
CProcess::LaunchImpl( const CString& exePath          ,
                      const TStringVector& args       ,
                      const CString& workingDir       ,
                      const TStringMap& envOverrides  ,
                      bool detach                     ,
                      UInt32 pipeMask                 )
{GUCEF_TRACE;

    if ( exePath.IsNULLOrEmpty() )
        return false;
    if ( 0 != m_processId && !m_hasExited && IsRunning() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Launch called while a process is still running" );
        return false;
    }
    ReleasePipes();

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

    // STARTUPINFOEXW: the handles the child inherits are named in a list (PROC_THREAD_ATTRIBUTE_HANDLE_LIST), so that
    // it gets its three standard handles and nothing else, also not the inheritable handles (sockets, files) that
    // other code of this process made
    STARTUPINFOEXW siEx;
    PROCESS_INFORMATION pi;
    memset( &siEx, 0, sizeof( siEx ) );
    memset( &pi, 0, sizeof( pi ) );
    siEx.StartupInfo.cb = sizeof( siEx );
    STARTUPINFOW& si = siEx.StartupInfo;

    DWORD flags = CREATE_UNICODE_ENVIRONMENT;
    if ( detach )
        flags |= DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP;

    std::wstring wWorkingDir = ToWString( workingDir );
    std::wstring wExePath = ToWString( exePath );
    std::vector< wchar_t > mutableCommandLine( commandLine.begin(), commandLine.end() );
    mutableCommandLine.push_back( L'\0' );

    // The pipes: the end for the child is inheritable, ours is not. The child's ends are closed again once the process exists.
    // The list of inherited handles below names the three standard handles of the child, so that it inherits nothing else
    HANDLE childHandles[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    HANDLE parentHandles[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    bool pipesCreated = true;
    if ( 0 != pipeMask )
    {
        SECURITY_ATTRIBUTES pipeSecurity;
        memset( &pipeSecurity, 0, sizeof( pipeSecurity ) );
        pipeSecurity.nLength = sizeof( pipeSecurity );
        pipeSecurity.bInheritHandle = TRUE;
        pipeSecurity.lpSecurityDescriptor = GUCEF_NULL;

        for ( int standard=0; standard<3 && pipesCreated; ++standard )
        {
            if ( 0 == ( pipeMask & ( 1u << standard ) ) )
                continue;
            HANDLE readEnd = GUCEF_NULL;
            HANDLE writeEnd = GUCEF_NULL;
            if ( !::CreatePipe( &readEnd, &writeEnd, &pipeSecurity, 0 ) )
            {
                pipesCreated = false;
                break;
            }
            // the child reads its input and writes its output; the end we keep must not be inherited
            childHandles[ standard ] = 0 == standard ? readEnd : writeEnd;
            parentHandles[ standard ] = 0 == standard ? writeEnd : readEnd;
            if ( !::SetHandleInformation( parentHandles[ standard ], HANDLE_FLAG_INHERIT, 0 ) )
                pipesCreated = false;
        }
    }

    // While stdout carries a protocol (CStandardOutputWriter::ClaimExclusive()) a child must not write into it: an
    // output that is not piped goes to our standard error instead
    const bool stdoutToStderr = 0 == ( pipeMask & 2u ) && IsStdoutClaimed();
    const bool setStdHandles = 0 != pipeMask || stdoutToStderr;

    // the standard streams that are not piped are handed on as inheritable duplicates of ours, so that the list of
    // inherited handles can name them; a handle that cannot be duplicated (no console) is handed on as it is
    HANDLE inheritedDuplicates[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    HANDLE handleList[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    DWORD handleCount = 0;
    LPPROC_THREAD_ATTRIBUTE_LIST attributeList = GUCEF_NULL;
    std::vector< char > attributeStorage;
    if ( pipesCreated && setStdHandles )
    {
        const DWORD stdIds[ 3 ] = { STD_INPUT_HANDLE, stdoutToStderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
        if ( stdoutToStderr && GUCEF_NULL != childHandles[ 2 ] )
        {
            // the output follows the standard error of the child into its pipe, as a second handle to the same pipe
            ::DuplicateHandle( ::GetCurrentProcess(), childHandles[ 2 ], ::GetCurrentProcess(), &inheritedDuplicates[ 1 ], 0, TRUE, DUPLICATE_SAME_ACCESS );
        }
        HANDLE* targets[ 3 ] = { &si.hStdInput, &si.hStdOutput, &si.hStdError };
        for ( int standard=0; standard<3; ++standard )
        {
            HANDLE handle = GUCEF_NULL != childHandles[ standard ] ? childHandles[ standard ] : inheritedDuplicates[ standard ];
            if ( GUCEF_NULL == handle )
            {
                HANDLE own = ::GetStdHandle( stdIds[ standard ] );
                if ( GUCEF_NULL != own && INVALID_HANDLE_VALUE != own &&
                     ::DuplicateHandle( ::GetCurrentProcess(), own, ::GetCurrentProcess(), &inheritedDuplicates[ standard ], 0, TRUE, DUPLICATE_SAME_ACCESS ) )
                    handle = inheritedDuplicates[ standard ];
                else
                    handle = own;
            }
            *targets[ standard ] = handle;
            if ( GUCEF_NULL != childHandles[ standard ] || GUCEF_NULL != inheritedDuplicates[ standard ] )
                handleList[ handleCount++ ] = handle;
        }
        si.dwFlags |= STARTF_USESTDHANDLES;

        if ( handleCount > 0 )
        {
            SIZE_T attributeSize = 0;
            ::InitializeProcThreadAttributeList( GUCEF_NULL, 1, 0, &attributeSize );
            attributeStorage.resize( attributeSize > 0 ? (size_t) attributeSize : 1 );
            attributeList = reinterpret_cast< LPPROC_THREAD_ATTRIBUTE_LIST >( &attributeStorage[ 0 ] );
            if ( 0 == attributeSize || !::InitializeProcThreadAttributeList( attributeList, 1, 0, &attributeSize ) )
            {
                attributeList = GUCEF_NULL;
                pipesCreated = false;
            }
            else
            if ( !::UpdateProcThreadAttribute( attributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handleList, handleCount * sizeof( HANDLE ), GUCEF_NULL, GUCEF_NULL ) )
            {
                pipesCreated = false;
            }
            siEx.lpAttributeList = attributeList;
        }
    }

    BOOL result = FALSE;
    if ( pipesCreated )
    {
        // handles are inherited only when the list names them: without a list nothing is inherited
        const BOOL inheritHandles = GUCEF_NULL != attributeList ? TRUE : FALSE;
        const DWORD createFlags = GUCEF_NULL != attributeList ? ( flags | EXTENDED_STARTUPINFO_PRESENT ) : flags;
        si.cb = GUCEF_NULL != attributeList ? (DWORD) sizeof( STARTUPINFOEXW ) : (DWORD) sizeof( STARTUPINFOW );
        result = ::CreateProcessW( GUCEF_NULL, &mutableCommandLine[ 0 ], GUCEF_NULL, GUCEF_NULL, inheritHandles, createFlags,
                                   (LPVOID) envBlock.c_str(),
                                   wWorkingDir.empty() ? GUCEF_NULL : wWorkingDir.c_str(),
                                   &si, &pi );
    }
    const DWORD launchError = pipesCreated ? ::GetLastError() : ERROR_BROKEN_PIPE;

    if ( GUCEF_NULL != attributeList )
        ::DeleteProcThreadAttributeList( attributeList );
    for ( int standard=0; standard<3; ++standard )
    {
        if ( GUCEF_NULL != childHandles[ standard ] )
            ::CloseHandle( childHandles[ standard ] );
        if ( GUCEF_NULL != inheritedDuplicates[ standard ] )
            ::CloseHandle( inheritedDuplicates[ standard ] );
    }

    if ( !result )
    {
        for ( int standard=0; standard<3; ++standard )
            if ( GUCEF_NULL != parentHandles[ standard ] )
                ::CloseHandle( parentHandles[ standard ] );
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CProcess: Failed to launch \"" + exePath + "\": error code " + ToString( (UInt32) launchError ) );
        return false;
    }

    for ( int standard=0; standard<3; ++standard )
        if ( GUCEF_NULL != parentHandles[ standard ] )
            m_pipes[ standard ] = GUCEF_NEW CProcessPipe( parentHandles[ standard ], 0 == standard ? CProcessPipe::PIPE_WRITE_TO_CHILD : CProcessPipe::PIPE_READ_FROM_CHILD );

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
CProcess::LaunchImpl( const CString&, const TStringVector&, const CString&, const TStringMap&, bool, UInt32 )
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
