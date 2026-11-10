/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestProcess.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <signal.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static CORE::TProcessId
GetOwnProcessId( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (CORE::TProcessId) ::GetCurrentProcessId();
    #else
    return (CORE::TProcessId) ::getpid();
    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *  Describes a process that does nothing for a long time
 */
static void
GetSleeperCommand( CORE::CString& exe, CORE::CProcess::TStringVector& args )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    exe = "ping";
    args.push_back( "-n" );
    args.push_back( "60" );
    args.push_back( "127.0.0.1" );
    #else
    exe = "sleep";
    args.push_back( "60" );
    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *  Describes a process that exits right away with the given exit code
 */
static void
GetExitCodeCommand( CORE::CString& exe, CORE::CProcess::TStringVector& args, UInt32 exitCode )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    exe = "cmd.exe";
    args.push_back( "/c" );
    args.push_back( "exit " + CORE::ToString( exitCode ) );
    #else
    exe = "sh";
    args.push_back( "-c" );
    args.push_back( "exit " + CORE::ToString( exitCode ) );
    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads a pipe until the other end closes. Gives up with what was read so far when nothing comes within the timeout
 */
static CORE::CString
ReadPipeToEnd( CORE::CProcessPipe& pipe, UInt32 timeoutInMs )
{
    CORE::CString all;
    char buffer[ 4096 ];
    while ( pipe.WaitForData( timeoutInMs ) )
    {
        UInt32 got = pipe.ReadSome( buffer, sizeof( buffer ) );
        if ( 0 == got )
            break;
        all.Append( buffer, got );
    }
    return all;
}

/*-------------------------------------------------------------------------*/

void
PerformProcessTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Process TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Process" );

    // Regression: on Linux CheckOnProcessAliveStatus() never set the status output
    // and reported failure instead of 'not alive' for processes that no longer exist
    GUCEF_TESTFW_TESTCASE( "Test: CheckOnProcessAliveStatus for the own process" )
        try
        {
            bool isAlive = false;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( GetOwnProcessId(), isAlive ) );
            ASSERT_TRUE( isAlive );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    GUCEF_TESTFW_TESTCASE( "Test: CheckOnProcessAliveStatus for a process that exited" )
        try
        {
            pid_t childPid = ::fork();
            if ( 0 == childPid )
                ::_exit( 0 );
            ASSERT_TRUE( childPid > 0 );

            int childStatus = 0;
            ASSERT_TRUE( childPid == ::waitpid( childPid, &childStatus, 0 ) );

            bool isAlive = true;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( (CORE::TProcessId) childPid, isAlive ) );
            ASSERT_FALSE( isAlive );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif

    GUCEF_TESTFW_TESTCASE( "Test: CProcess::SplitCommandLine" )
        try
        {
            CORE::CProcess::TStringVector args = CORE::CProcess::SplitCommandLine( "add \"my dir/file name.txt\"  -m 'a message' plain\\x" );
            ASSERT_TRUE( 5 == args.size() );
            if ( 5 == args.size() )
            {
                ASSERT_TRUE( args[ 0 ] == "add" );
                ASSERT_TRUE( args[ 1 ] == "my dir/file name.txt" );
                ASSERT_TRUE( args[ 2 ] == "-m" );
                ASSERT_TRUE( args[ 3 ] == "a message" );
                ASSERT_TRUE( args[ 4 ] == "plain\\x" );
            }
            ASSERT_TRUE( CORE::CProcess::SplitCommandLine( "   " ).empty() );
            ASSERT_TRUE( CORE::CProcess::SplitCommandLine( "" ).empty() );

            // an empty quoted argument is still an argument
            args = CORE::CProcess::SplitCommandLine( "a \"\" b" );
            ASSERT_TRUE( 3 == args.size() );
            args = CORE::CProcess::SplitCommandLine( "say \"he said \\\"hi\\\"\"" );
            ASSERT_TRUE( 2 == args.size() );
            if ( 2 == args.size() )
                ASSERT_TRUE( args[ 1 ] == "he said \"hi\"" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess launch, running state and forced terminate" )
        try
        {
            CORE::CString exe;
            CORE::CProcess::TStringVector args;
            GetSleeperCommand( exe, args );

            CORE::CProcess process;
            ASSERT_TRUE( 0 == process.GetProcessId() );
            ASSERT_FALSE( process.IsRunning() );
            ASSERT_TRUE( process.Launch( exe, args ) );
            ASSERT_TRUE( process.GetProcessId() > 0 );
            ASSERT_TRUE( process.IsRunning() );

            bool isAlive = false;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( process.GetProcessId(), isAlive ) );
            ASSERT_TRUE( isAlive );

            // it is still running so waiting must time out
            Int32 exitCode = 0;
            ASSERT_FALSE( process.WaitForExit( 100, exitCode ) );

            // a second launch while the first one is running is refused
            ASSERT_FALSE( process.Launch( exe, args ) );

            ASSERT_TRUE( process.Terminate( false ) );
            ASSERT_FALSE( process.IsRunning() );
            ASSERT_TRUE( process.WaitForExit( 0, exitCode ) );

            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( process.GetProcessId(), isAlive ) );
            ASSERT_FALSE( isAlive );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess reports the exit code of a process that ends by itself" )
        try
        {
            CORE::CString exe;
            CORE::CProcess::TStringVector args;
            GetExitCodeCommand( exe, args, 7 );

            CORE::CProcess process;
            ASSERT_TRUE( process.Launch( exe, args ) );
            Int32 exitCode = 0;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 7 == exitCode );
            ASSERT_FALSE( process.IsRunning() );

            // the same instance can be used for another process
            args.clear();
            GetExitCodeCommand( exe, args, 0 );
            ASSERT_TRUE( process.Launch( exe, args ) );
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess launch of a missing executable fails cleanly" )
        try
        {
            CORE::CProcess process;
            ASSERT_FALSE( process.Launch( "gucef_this_executable_does_not_exist_anywhere", CORE::CProcess::TStringVector() ) );
            ASSERT_TRUE( 0 == process.GetProcessId() );
            ASSERT_FALSE( process.IsRunning() );
            ASSERT_FALSE( process.Launch( "", CORE::CProcess::TStringVector() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess destructor terminates a non-detached process" )
        try
        {
            CORE::CString exe;
            CORE::CProcess::TStringVector args;
            GetSleeperCommand( exe, args );

            UInt32 pid = 0;
            {
                CORE::CProcess process;
                ASSERT_TRUE( process.Launch( exe, args ) );
                pid = process.GetProcessId();
            }
            ASSERT_TRUE( pid > 0 );
            bool isAlive = true;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( pid, isAlive ) );
            ASSERT_FALSE( isAlive );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    GUCEF_TESTFW_TESTCASE( "Test: CProcess graceful terminate asks nicely first" )
        try
        {
            CORE::CString exe;
            CORE::CProcess::TStringVector args;
            GetSleeperCommand( exe, args );

            CORE::CProcess process;
            ASSERT_TRUE( process.Launch( exe, args ) );
            ASSERT_TRUE( process.Terminate( true, 5000 ) );

            // sleep honors SIGTERM so we expect that signal to have been the cause: 128 + 15
            Int32 exitCode = 0;
            ASSERT_TRUE( process.WaitForExit( 0, exitCode ) );
            ASSERT_TRUE( 143 == exitCode );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess graceful terminate falls back to force for a process that ignores the request" )
        try
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "trap '' TERM; while :; do sleep 1; done" );

            CORE::CProcess process;
            ASSERT_TRUE( process.Launch( "sh", args ) );
            MT::ThreadDelay( 200 );    // give the shell time to install the trap
            ASSERT_TRUE( process.Terminate( true, 300 ) );
            ASSERT_FALSE( process.IsRunning() );

            Int32 exitCode = 0;
            ASSERT_TRUE( process.WaitForExit( 0, exitCode ) );
            ASSERT_TRUE( 137 == exitCode );     // 128 + SIGKILL
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The child environment of the test helper keeps the thread sanitizer options and the exit code of the child its own" )
        try
        {
            // options that are given already stay, exitcode=0 comes after them and so wins
            CORE::CProcess::TStringMap given;
            given[ "TSAN_OPTIONS" ] = "log_path=/somewhere/tsan history_size=4";
            given[ "OTHER" ] = "kept";
            TESTFW::KeepChildExitCodeUnderSanitizers( given );
            ASSERT_TRUE( "log_path=/somewhere/tsan history_size=4 exitcode=0" == given[ "TSAN_OPTIONS" ] );
            ASSERT_TRUE( "kept" == given[ "OTHER" ] && 2 == given.size() );

            // without any, those of this process are the ones that carry over; when they have no log_path the reports
            // of the child go to a log in the temporary directory, its exit code no longer tells of them
            const CORE::CString ours = CORE::GetEnv( "TSAN_OPTIONS" );
            CORE::CString expected = ours;
            if ( TESTFW::SanitizerOptionValue( ours, "log_path" ).IsNULLOrEmpty() )
            {
                const CORE::CString logPath = CORE::CombinePath( CORE::CombinePath( CORE::TempDir(), "gucef_test_child_sanitizer_logs" ), "tsan" );
                expected = ours.IsNULLOrEmpty() ? "log_path=" + logPath : ours + " log_path=" + logPath;
            }
            expected += " exitcode=0";
            CORE::CProcess::TStringMap environment = TESTFW::ChildEnvironmentWithOwnExitCode();
            ASSERT_TRUE( expected == environment[ "TSAN_OPTIONS" ] );

            // and they reach the child
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "[ \"$TSAN_OPTIONS\" = \"" + expected + "\" ] && exit 0; exit 3" );
            CORE::CProcess process;
            ASSERT_TRUE( process.Launch( "sh", args, CORE::CString(), environment ) );
            Int32 exitCode = -1;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess working dir and environment overrides reach the child" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCORE_TestApp_ProcessDir" );
            CORE::CString dir = scopedDir.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );

            // exit 0 only if both the env var and the working dir are as expected
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "[ \"$GUCEF_TEST_ENV\" = \"hello world\" ] && [ \"$(pwd -P)\" = \"$(cd '" + dir + "' && pwd -P)\" ] && exit 0; exit 3" );

            CORE::CProcess::TStringMap env;
            env[ "GUCEF_TEST_ENV" ] = "hello world";

            CORE::CProcess process;
            ASSERT_TRUE( process.Launch( "sh", args, dir, env ) );
            Int32 exitCode = -1;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );

            // without the override the check must fail, which proves the check itself works
            ASSERT_TRUE( process.Launch( "sh", args, dir ) );
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 3 == exitCode );

            // an unusable working dir is a launch failure
            ASSERT_FALSE( process.Launch( "sh", args, CORE::CombinePath( dir, "does/not/exist" ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess detached process outlives the CProcess object" )
        try
        {
            CORE::CString exe;
            CORE::CProcess::TStringVector args;
            GetSleeperCommand( exe, args );

            UInt32 pid = 0;
            {
                CORE::CProcess process;
                ASSERT_TRUE( process.Launch( exe, args, CORE::CString(), CORE::CProcess::TStringMap(), true ) );
                pid = process.GetProcessId();
            }
            ASSERT_TRUE( pid > 0 );

            bool isAlive = false;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( pid, isAlive ) );
            ASSERT_TRUE( isAlive );

            // it got its own session so it is not part of our process group
            ASSERT_TRUE( ::getpgid( (pid_t) pid ) != ::getpgid( 0 ) );

            // clean up after ourselves
            ::kill( (pid_t) pid, SIGKILL );
            int status = 0;
            ::waitpid( (pid_t) pid, &status, 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // F3: Execute_Program was an empty stub on Linux
    GUCEF_TESTFW_TESTCASE( "Test: Execute_Program launches programs" )
        try
        {
            ASSERT_TRUE( 0 != CORE::Execute_Program( "sh", "-c 'exit 0'" ) );
            ASSERT_TRUE( 0 == CORE::Execute_Program( "gucef_this_executable_does_not_exist_anywhere", GUCEF_NULL ) );
            ASSERT_TRUE( 0 == CORE::Execute_Program( GUCEF_NULL, GUCEF_NULL ) );

            // the program really ran: it creates a file
            TESTFW::CScopedTestDir scopedDir( "gucefCORE_TestApp_ExecuteProgram" );
            CORE::CString dir = scopedDir.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString marker = CORE::CombinePath( dir, "marker.txt" );
            if ( CORE::FileExists( marker ) )
                CORE::DeleteFile( marker );
            ASSERT_TRUE( 0 != CORE::Execute_Program( "sh", ( "-c \"echo done > '" + marker + "'\"" ).C_String() ) );

            bool markerSeen = false;
            for ( UInt32 i=0; i<200 && !markerSeen; ++i )
            {
                markerSeen = CORE::FileExists( marker );
                if ( !markerSeen )
                    MT::ThreadDelay( 25 );
            }
            ASSERT_TRUE( markerSeen );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess pipes: standard output and error of the child, separately" )
        try
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "echo to-out; echo to-err 1>&2; exit 5" );

            CORE::CProcess process;
            ASSERT_TRUE( process.LaunchWithPipes( "sh", args, false, true, true ) );
            ASSERT_TRUE( GUCEF_NULL == process.GetStdinPipe() );
            ASSERT_TRUE( GUCEF_NULL != process.GetStdoutPipe() );
            ASSERT_TRUE( GUCEF_NULL != process.GetStderrPipe() );
            ASSERT_TRUE( process.GetStdoutPipe()->IsReadable() );
            ASSERT_FALSE( process.GetStdoutPipe()->IsWriteable() );

            ASSERT_TRUE( ReadPipeToEnd( *process.GetStdoutPipe(), 10000 ) == "to-out\n" );
            ASSERT_TRUE( ReadPipeToEnd( *process.GetStderrPipe(), 10000 ) == "to-err\n" );
            ASSERT_TRUE( process.GetStdoutPipe()->Eof() );

            Int32 exitCode = 0;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 5 == exitCode );

            // the same instance can launch again, without pipes this time, which drops the old ones
            args.clear();
            args.push_back( "-c" );
            args.push_back( "exit 0" );
            ASSERT_TRUE( process.Launch( "sh", args ) );
            ASSERT_TRUE( GUCEF_NULL == process.GetStdoutPipe() );
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess pipes: standard input to a child and its output back" )
        try
        {
            CORE::CProcess::TStringVector args;
            CORE::CProcess process;
            ASSERT_TRUE( process.LaunchWithPipes( "cat", args, true, true, false ) );
            ASSERT_TRUE( GUCEF_NULL != process.GetStdinPipe() );
            ASSERT_TRUE( GUCEF_NULL == process.GetStderrPipe() );
            ASSERT_TRUE( process.GetStdinPipe()->IsWriteable() );

            // several rounds of 32 KiB: more than the pipe holds when summed up, so the data has to be read back as it comes
            CORE::CString block;
            for ( UInt32 i=0; i<32768; ++i )
                block += (char) ( 'a' + ( i % 26 ) );
            for ( UInt32 round=0; round<32; ++round )
            {
                ASSERT_TRUE( 1 == process.GetStdinPipe()->Write( block.C_String(), block.Length(), 1 ) );
                CORE::CString echoed;
                char buffer[ 4096 ];
                while ( echoed.Length() < block.Length() )
                {
                    ASSERT_TRUE( process.GetStdoutPipe()->WaitForData( 10000 ) );
                    UInt32 got = process.GetStdoutPipe()->ReadSome( buffer, sizeof( buffer ) );
                    ASSERT_TRUE( got > 0 );
                    if ( 0 == got )
                        break;
                    echoed.Append( buffer, got );
                }
                ASSERT_TRUE( echoed == block );
            }

            // closing the input is the end of the input: cat ends, and its output ends
            process.CloseStdinPipe();
            ASSERT_FALSE( process.GetStdinPipe()->Opened() );
            ASSERT_TRUE( ReadPipeToEnd( *process.GetStdoutPipe(), 10000 ).IsNULLOrEmpty() );
            Int32 exitCode = -1;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess pipes: waiting for data times out, and the data wakes it" )
        try
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "read line; echo got-$line" );

            CORE::CProcess process;
            ASSERT_TRUE( process.LaunchWithPipes( "sh", args, true, true, false ) );
            ASSERT_FALSE( process.GetStdoutPipe()->WaitForData( 150 ) );

            ASSERT_TRUE( 1 == process.GetStdinPipe()->Write( "x\n", 2, 1 ) );
            ASSERT_TRUE( process.GetStdoutPipe()->WaitForData( 10000 ) );
            ASSERT_TRUE( ReadPipeToEnd( *process.GetStdoutPipe(), 10000 ) == "got-x\n" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess pipes: writing to a child that has gone fails without a signal" )
        try
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "exit 0" );

            CORE::CProcess process;
            ASSERT_TRUE( process.LaunchWithPipes( "sh", args, true, false, false ) );
            Int32 exitCode = -1;
            ASSERT_TRUE( process.WaitForExit( 10000, exitCode ) );

            // SIGPIPE would end this very process, so reaching the next line is the test
            CORE::CString block;
            for ( UInt32 i=0; i<1024; ++i )
                block += "0123456789";
            ASSERT_TRUE( 0 == process.GetStdinPipe()->Write( block.C_String(), block.Length(), 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess pipes: a failed launch leaves no pipes, and the destructor ends a child with open pipes" )
        try
        {
            CORE::CProcess::TStringVector args;
            CORE::CProcess process;
            ASSERT_FALSE( process.LaunchWithPipes( "gucef_this_executable_does_not_exist_anywhere", args, true, true, true ) );
            ASSERT_TRUE( GUCEF_NULL == process.GetStdinPipe() );
            ASSERT_TRUE( GUCEF_NULL == process.GetStdoutPipe() );
            ASSERT_TRUE( GUCEF_NULL == process.GetStderrPipe() );

            UInt32 pid = 0;
            {
                CORE::CProcess holder;
                args.push_back( "60" );
                ASSERT_TRUE( holder.LaunchWithPipes( "sleep", args, true, true, true ) );
                pid = holder.GetProcessId();
            }
            bool isAlive = true;
            ASSERT_TRUE( CORE::CheckOnProcessAliveStatus( pid, isAlive ) );
            ASSERT_FALSE( isAlive );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CProcess: while stdout is claimed by a protocol, a child whose output is not piped writes it to standard error" )
        try
        {
            // our standard output goes to a pipe of the test for the duration, so that what reaches it can be seen
            int captured[ 2 ] = { -1, -1 };
            ASSERT_TRUE( 0 == ::pipe( captured ) );
            const int savedStdout = ::dup( STDOUT_FILENO );
            ASSERT_TRUE( savedStdout >= 0 );
            ::fflush( stdout );
            ::dup2( captured[ 1 ], STDOUT_FILENO );
            const bool claimed = CORE::CStandardOutputWriter::ClaimExclusive();

            CORE::CString childError;
            Int32 exitCode = -1;
            Int32 plainExitCode = -1;
            bool launched = false;
            bool plainLaunched = false;
            try
            {
                CORE::CProcess::TStringVector args;
                args.push_back( "-c" );
                args.push_back( "echo leaked-to-stdout; echo to-err 1>&2" );
                CORE::CProcess process;
                launched = process.LaunchWithPipes( "sh", args, false, false, true );
                if ( launched )
                {
                    childError = ReadPipeToEnd( *process.GetStderrPipe(), 10000 );
                    process.WaitForExit( 10000, exitCode );
                }

                // without any pipe the same holds
                CORE::CProcess plain;
                args.clear();
                args.push_back( "-c" );
                args.push_back( "echo plain-leaked-to-stdout" );
                plainLaunched = plain.Launch( "sh", args );
                if ( plainLaunched )
                    plain.WaitForExit( 10000, plainExitCode );
            }
            catch( ... )
            {
            }

            if ( claimed )
                CORE::CStandardOutputWriter::ReleaseClaim();
            ::fflush( stdout );
            ::dup2( savedStdout, STDOUT_FILENO );
            ::close( savedStdout );
            ::close( captured[ 1 ] );

            // what reached our standard output while it was claimed
            CORE::CString reachedStdout;
            char buffer[ 512 ];
            ::fcntl( captured[ 0 ], F_SETFL, ::fcntl( captured[ 0 ], F_GETFL ) | O_NONBLOCK );
            ssize_t got = 0;
            while ( ( got = ::read( captured[ 0 ], buffer, sizeof( buffer ) ) ) > 0 )
                reachedStdout.Append( buffer, (UInt32) got );
            ::close( captured[ 0 ] );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Process test: the claimed stdout got \"" << reachedStdout << "\", the child's stderr \"" << childError << "\"" );
            ASSERT_TRUE( claimed );
            ASSERT_TRUE( launched && 0 == exitCode );
            ASSERT_TRUE( plainLaunched && 0 == plainExitCode );
            ASSERT_TRUE( reachedStdout.HasSubstr( "leaked" ) < 0 );
            ASSERT_TRUE( childError.HasSubstr( "leaked-to-stdout" ) >= 0 && childError.HasSubstr( "to-err" ) >= 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Process TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
