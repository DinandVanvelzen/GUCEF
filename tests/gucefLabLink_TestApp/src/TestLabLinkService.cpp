/*
 *  gucefLabLink_TestApp: tests of gucefLabLink
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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
#endif

#include "gucefLabLink_ConfigFile.h"
#include "gucefLabLink_PlatformDefaults.h"
#include "gucefLabLink_ServiceHost.h"

#include "TestLabLinkService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEFLabLink;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CORE::CString::StringVector TArguments;

/*-------------------------------------------------------------------------*/

TArguments
Args( const char* a = GUCEF_NULL, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL,
      const char* e = GUCEF_NULL, const char* f = GUCEF_NULL )
{
    TArguments arguments;
    const char* all[] = { a, b, c, d, e, f };
    for ( int i=0; i<6 && GUCEF_NULL != all[ i ]; ++i )
        arguments.push_back( all[ i ] );
    return arguments;
}

/*-------------------------------------------------------------------------*/

bool
Parses( const CORE::CString& text, const TArguments& expected )
{
    TArguments arguments;
    CORE::CString problem;
    if ( !ParseConfigText( text, "lablink.conf", arguments, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Refused: " << problem );
        return false;
    }
    return arguments == expected;
}

/*-------------------------------------------------------------------------*/

/** Refused, with the line in the problem and no arguments */
bool
Refuses( const CORE::CString& text, const char* line )
{
    TArguments arguments;
    arguments.push_back( "left over" );
    CORE::CString problem;
    if ( ParseConfigText( text, "lablink.conf", arguments, problem ) )
        return false;
    return arguments.empty() && problem.HasSubstr( line ) >= 0;
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, const CORE::CString& text, int mode = 0600 )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    ::unlink( path.C_String() );
    const int fd = ::open( path.C_String(), O_WRONLY | O_CREAT | O_EXCL, 0600 );
    if ( fd < 0 )
        return false;
    const size_t size = ::strlen( text.C_String() );
    const bool written = (ssize_t) size == ::write( fd, text.C_String(), size );
    ::close( fd );
    return written && 0 == ::chmod( path.C_String(), (mode_t) mode );
    #else
    FILE* file = fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    fwrite( text.C_String(), 1, ::strlen( text.C_String() ), file );
    fclose( file );
    return true;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Runs gucefLabLink with the arguments; the exit code, or a negative number when it did not run or end */
Int32
RunLabLink( const CORE::CString& program, const TArguments& arguments )
{
    CORE::CProcess::TStringVector args( arguments.begin(), arguments.end() );
    CORE::CProcess::TStringMap environment;
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
    CORE::CProcess process;
    process.SetIsolateFromParent( true );
    if ( !process.Launch( program, args, CORE::CString::Empty, environment ) )
        return -1000;
    Int32 exitCode = -1001;
    if ( !process.WaitForExit( 60000, exitCode ) )
    {
        process.Terminate( false, 0 );
        return -1002;
    }
    return exitCode;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLabLinkServiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "LabLinkService" );

    GUCEF_TESTFW_TESTCASE( "Configuration file: every option of the command line, as name = value, name value, --name value and flags" )
        try
        {
            // the options of the command line that take a value
            static const char* valued[] = { "role", "state-dir", "allow", "cluster-name", "machine-name", "lab-port", "access-port",
                                            "broadcast-port", "join-command", "k3s-bin", "mcp-dir", "join-script", "node-marker",
                                            "claude-user", "claude-command", "claude-home", "local-node", "refresh", "retry", "db",
                                            "db-dir", "db-host", "db-port", "db-name", "db-user", "db-ssl-mode", "db-schema",
                                            "db-password-file", "history-days", "history-max", "report-interval" };
            for ( size_t i=0; i<sizeof( valued ) / sizeof( valued[ 0 ] ); ++i )
            {
                const CORE::CString name = valued[ i ];
                const TArguments expected = Args( ( "--" + name ).C_String(), "the value" );
                ASSERT_TRUE( Parses( name + " = the value\n", expected ) );
                ASSERT_TRUE( Parses( name + "=the value", expected ) );
                ASSERT_TRUE( Parses( name + " the value\n", expected ) );
                ASSERT_TRUE( Parses( "--" + name + " the value\n", expected ) );
                ASSERT_TRUE( Parses( "\t" + name + "\t=\t\"the value\"  \n", expected ) );
            }
            // and those without
            static const char* flags[] = { "no-mdns", "no-broadcast", "permanent", "no-report" };
            for ( size_t i=0; i<sizeof( flags ) / sizeof( flags[ 0 ] ); ++i )
            {
                const CORE::CString name = flags[ i ];
                ASSERT_TRUE( Parses( name + "\n", Args( ( "--" + name ).C_String() ) ) );
                ASSERT_TRUE( Parses( "  --" + name + "  \r\n", Args( ( "--" + name ).C_String() ) ) );
            }
            // a flag with a value gives the value as an argument of its own, which the option parser refuses
            ASSERT_TRUE( Parses( "no-mdns = false\n", Args( "--no-mdns", "false" ) ) );

            // a whole file: comments, empty lines, CR line ends, a byte order mark, repeated options in order
            const CORE::CString file = "\xEF\xBB\xBF# gucefLabLink\r\n"
                                       "; the role\r\n"
                                       "role = machine\r\n"
                                       "\r\n"
                                       "allow = 192.168.1.0/24\r\n"
                                       "allow 10.0.0.0/8\r\n"
                                       "   # indented comment\r\n"
                                       "state-dir = \"C:\\ProgramData\\gucef lablink\\\"\r\n"
                                       "machine-name = a=b#c\r\n"
                                       "no-mdns";
            TArguments expected;
            expected.push_back( "--role" ); expected.push_back( "machine" );
            expected.push_back( "--allow" ); expected.push_back( "192.168.1.0/24" );
            expected.push_back( "--allow" ); expected.push_back( "10.0.0.0/8" );
            expected.push_back( "--state-dir" ); expected.push_back( "C:\\ProgramData\\gucef lablink\\" );
            expected.push_back( "--machine-name" ); expected.push_back( "a=b#c" );
            expected.push_back( "--no-mdns" );
            ASSERT_TRUE( Parses( file, expected ) );

            // an empty file, or one of comments only, holds no options
            ASSERT_TRUE( Parses( "", TArguments() ) );
            ASSERT_TRUE( Parses( "# nothing\n\n; else\n", TArguments() ) );
            // a quoted empty value is a value
            ASSERT_TRUE( Parses( "claude-user = \"\"\n", Args( "--claude-user", "" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Configuration file: what is refused, with the line" )
        try
        {
            ASSERT_TRUE( Refuses( "role = machine\nstate-dir =\n", "line 2" ) );
            ASSERT_TRUE( Refuses( "role =   \t\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "# fine\nstate-dir = \"C:\\x\n", "line 2" ) );
            ASSERT_TRUE( Refuses( "state-dir = \"\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "ro!e = machine\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "role: machine\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "= machine\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "--\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "---role machine\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "\n\nconfig = other.conf\n", "line 3" ) );
            ASSERT_TRUE( Refuses( "console\n", "line 1" ) );
            ASSERT_TRUE( Refuses( "--console\n", "line 1" ) );

            CORE::CString longLine = "machine-name = ";
            while ( longLine.Length() <= 4096 )
                longLine += "x";
            ASSERT_TRUE( Refuses( "role = machine\n" + longLine + "\n", "line 2" ) );

            CORE::CString large;
            while ( large.Length() <= MaxConfigFileSize )
                large += "# a comment line that makes the file large\n";
            ASSERT_TRUE( Refuses( large, "larger" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Arguments: --config is taken from the command line, the file comes first so the command line wins" )
        try
        {
            TArguments options;
            CORE::CString configFile;
            CORE::CString problem;
            ASSERT_TRUE( CServiceHost::TakeConfigOption( Args( "--role", "machine", "--config", "/etc/a.conf", "--no-mdns" ), options, configFile, problem ) );
            ASSERT_TRUE( configFile == "/etc/a.conf" );
            ASSERT_TRUE( options == Args( "--role", "machine", "--no-mdns" ) );

            ASSERT_TRUE( CServiceHost::TakeConfigOption( Args( "--config=/etc/a.conf", "--config", "C:\\b c.conf" ), options, configFile, problem ) );
            ASSERT_TRUE( configFile == "C:\\b c.conf" );
            ASSERT_TRUE( options.empty() );

            ASSERT_TRUE( CServiceHost::TakeConfigOption( Args( "--console", "--list" ), options, configFile, problem ) );
            ASSERT_TRUE( configFile.IsNULLOrEmpty() );
            ASSERT_TRUE( options == Args( "--console", "--list" ) );

            ASSERT_FALSE( CServiceHost::TakeConfigOption( Args( "--role", "machine", "--config" ), options, configFile, problem ) );
            ASSERT_FALSE( CServiceHost::TakeConfigOption( Args( "--config=" ), options, configFile, problem ) );
            ASSERT_FALSE( CServiceHost::TakeConfigOption( Args( "--config", "" ), options, configFile, problem ) );

            // argv[ 0 ], the file, the command line: a later value of an option replaces an earlier one in the parser
            const TArguments merged = CServiceHost::MergeArguments( "gucefLabLink", Args( "--role", "cluster", "--allow", "10.0.0.0/8" ), Args( "--role", "machine" ) );
            TArguments expected = Args( "gucefLabLink", "--role", "cluster", "--allow", "10.0.0.0/8" );
            expected.push_back( "--role" ); expected.push_back( "machine" );
            ASSERT_TRUE( merged == expected );
            ASSERT_TRUE( CServiceHost::MergeArguments( "p", TArguments(), TArguments() ) == Args( "p" ) );

            // without --config and without a configuration file of the platform, the arguments are those of main()
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            {
                char a0[] = "gucefLabLink"; char a1[] = "--role"; char a2[] = "machine"; char a3[] = "--console";
                char* argv[] = { a0, a1, a2, a3, GUCEF_NULL };
                CServiceHost host( false );
                ASSERT_TRUE( host.Prepare( 4, argv ) );
                ASSERT_TRUE( 4 == host.GetArgc() );
                for ( int i=0; i<4; ++i )
                    ASSERT_TRUE( 0 == ::strcmp( argv[ i ], host.GetArgv()[ i ] ) );
                ASSERT_TRUE( GUCEF_NULL == host.GetArgv()[ 4 ] );
                ASSERT_TRUE( 1 == host.GetFileArgc() );
                ASSERT_TRUE( host.GetConfigFile().IsNULLOrEmpty() );
                ASSERT_TRUE( host.UsesConsoleLog() );
            }
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Defaults: the paths, the role and the architecture of each platform" )
        try
        {
            const SSettings settings;
            const SPlatformPaths linuxPaths = PlatformPathsFor( false, "C:\\ProgramData" );
            ASSERT_TRUE( linuxPaths.stateDir == settings.stateDir );
            ASSERT_TRUE( linuxPaths.stateDir == "/var/lib/gucef-lablink" );
            ASSERT_TRUE( linuxPaths.mcpDir == settings.mcpDir );
            ASSERT_TRUE( linuxPaths.configFile.IsNULLOrEmpty() );

            const SPlatformPaths windowsPaths = PlatformPathsFor( true, "C:\\ProgramData" );
            ASSERT_TRUE( windowsPaths.stateDir == "C:\\ProgramData\\gucef-lablink" );
            ASSERT_TRUE( windowsPaths.configFile == "C:\\ProgramData\\gucef-lablink\\lablink.conf" );
            ASSERT_TRUE( windowsPaths.mcpDir == "C:\\ProgramData\\gucef-mcp" );
            ASSERT_TRUE( PlatformPathsFor( true, "D:\\Data\\" ).stateDir == "D:\\Data\\gucef-lablink" );
            ASSERT_TRUE( PlatformPathsFor( true, "D:\\Data//" ).configFile == "D:\\Data\\gucef-lablink\\lablink.conf" );
            ASSERT_TRUE( PlatformPathsFor( true, "" ).stateDir == "C:\\ProgramData\\gucef-lablink" );
            ASSERT_TRUE( PlatformPathsFor( true, "E:\\" ).stateDir == "E:\\gucef-lablink" );

            // on this platform: Linux keeps the defaults of SSettings
            SSettings applied;
            ApplyPlatformDefaults( applied );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            ASSERT_TRUE( applied.stateDir == ThisPlatformPaths().stateDir );
            #else
            ASSERT_TRUE( applied.stateDir == settings.stateDir );
            ASSERT_TRUE( applied.mcpDir == settings.mcpDir );
            ASSERT_TRUE( ThisPlatformPaths().configFile.IsNULLOrEmpty() );
            #endif

            ASSERT_TRUE( DefaultRoleFor( false, true ) == "cluster" );
            ASSERT_TRUE( DefaultRoleFor( false, false ) == "machine" );
            ASSERT_TRUE( DefaultRoleFor( true, true ) == "machine" );
            ASSERT_TRUE( DefaultRoleFor( true, false ) == "machine" );

            ASSERT_TRUE( WindowsArchName( 9 ) == "x86_64" );
            ASSERT_TRUE( WindowsArchName( 12 ) == "aarch64" );
            ASSERT_TRUE( WindowsArchName( 0 ) == "x86" );
            ASSERT_TRUE( WindowsArchName( 5 ) == "arm" );
            ASSERT_TRUE( WindowsArchName( 0xFFFF ) == "unknown" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows service status: check points and wait hints only for a start the application reports running" )
        try
        {
            unsigned long checkPoint = 99;
            unsigned long waitHint = 99;

            // without the option: what GUCEF_OSSERVICEMAIN_BEGIN always reported
            const unsigned long states[] = { 1, GUCEF_OSSERVICE_STATE_START_PENDING, GUCEF_OSSERVICE_STATE_STOP_PENDING, 4 };
            for ( int i=0; i<4; ++i )
            {
                gucef_osservice_pending_status( states[ i ], 0, 7, &checkPoint, &waitHint );
                ASSERT_TRUE( 0 == checkPoint && 0 == waitHint );
            }

            // with it: a start that goes on counts up, running resets, a stop counts up again
            const unsigned long later = GUCEF_OSSERVICE_OPTION_REPORT_RUNNING_LATER;
            gucef_osservice_pending_status( GUCEF_OSSERVICE_STATE_START_PENDING, later, 0, &checkPoint, &waitHint );
            ASSERT_TRUE( 1 == checkPoint && GUCEF_OSSERVICE_PENDING_WAIT_HINT_IN_MS == waitHint );
            gucef_osservice_pending_status( GUCEF_OSSERVICE_STATE_START_PENDING, later, checkPoint, &checkPoint, &waitHint );
            ASSERT_TRUE( 2 == checkPoint && 30000 == waitHint );
            gucef_osservice_pending_status( 4 /* running */, later, checkPoint, &checkPoint, &waitHint );
            ASSERT_TRUE( 0 == checkPoint && 0 == waitHint );
            gucef_osservice_pending_status( GUCEF_OSSERVICE_STATE_STOP_PENDING, later, checkPoint, &checkPoint, &waitHint );
            ASSERT_TRUE( 1 == checkPoint && 30000 == waitHint );
            gucef_osservice_pending_status( 1 /* stopped */, later, checkPoint, &checkPoint, &waitHint );
            ASSERT_TRUE( 0 == checkPoint && 0 == waitHint );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    GUCEF_TESTFW_TESTCASE( "Configuration file: only a file its administrators alone can change is read" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_ConfigFile" );
            const CORE::CString path = CORE::CombinePath( dir.GetPath(), "lablink.conf" );
            TArguments arguments;
            bool missing = false;
            CORE::CString problem;

            ASSERT_FALSE( ReadConfigFile( path, arguments, missing, problem ) );
            ASSERT_TRUE( missing );

            ASSERT_TRUE( WriteFile( path, "role = machine\nno-mdns\n", 0600 ) );
            ASSERT_TRUE( ReadConfigFile( path, arguments, missing, problem ) );
            ASSERT_FALSE( missing );
            ASSERT_TRUE( arguments == Args( "--role", "machine", "--no-mdns" ) );
            ASSERT_TRUE( WriteFile( path, "role = machine\n", 0644 ) );
            ASSERT_TRUE( ReadConfigFile( path, arguments, missing, problem ) );

            // writable by the group or by others: refused
            ASSERT_TRUE( WriteFile( path, "role = machine\n", 0664 ) );
            ASSERT_FALSE( ReadConfigFile( path, arguments, missing, problem ) );
            ASSERT_FALSE( missing );
            ASSERT_TRUE( arguments.empty() );
            ASSERT_TRUE( WriteFile( path, "role = machine\n", 0646 ) );
            ASSERT_FALSE( ReadConfigFile( path, arguments, missing, problem ) );

            // a symbolic link, a directory, a file that is no text or too large: refused
            ASSERT_TRUE( WriteFile( path, "role = machine\n", 0600 ) );
            const CORE::CString link = CORE::CombinePath( dir.GetPath(), "link.conf" );
            ASSERT_TRUE( 0 == ::symlink( path.C_String(), link.C_String() ) );
            ASSERT_FALSE( ReadConfigFile( link, arguments, missing, problem ) );
            ASSERT_FALSE( ReadConfigFile( dir.GetPath(), arguments, missing, problem ) );
            const CORE::CString binary = CORE::CombinePath( dir.GetPath(), "binary.conf" );
            {
                const int fd = ::open( binary.C_String(), O_WRONLY | O_CREAT | O_EXCL, 0600 );
                ASSERT_TRUE( fd >= 0 );
                ASSERT_TRUE( 5 == ::write( fd, "ro\0le", 5 ) );
                ::close( fd );
            }
            ASSERT_FALSE( ReadConfigFile( binary, arguments, missing, problem ) );
            CORE::CString large;
            while ( large.Length() <= MaxConfigFileSize )
                large += "# a comment line that makes the file large\n";
            ASSERT_TRUE( WriteFile( path, large, 0600 ) );
            ASSERT_FALSE( ReadConfigFile( path, arguments, missing, problem ) );
            // a syntax problem names the file and the line
            ASSERT_TRUE( WriteFile( path, "role = machine\nstate-dir =\n", 0600 ) );
            ASSERT_FALSE( ReadConfigFile( path, arguments, missing, problem ) );
            ASSERT_TRUE( problem.HasSubstr( "lablink.conf line 2" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Program: the options of a configuration file reach the settings, the command line wins, wrong files are refused" )
        try
        {
            const CORE::CString program = CORE::RelativePath( "$MODULEDIR$/gucefLabLink" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( program ), "gucefLabLink is not next to the test application" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_ConfigProgram" );
            const CORE::CString conf = CORE::CombinePath( dir.GetPath(), "lablink.conf" );
            const CORE::CString fromFile = CORE::CombinePath( dir.GetPath(), "from-file" );
            const CORE::CString fromCommandLine = CORE::CombinePath( dir.GetPath(), "from-command-line" );

            // --init makes the lab secret in the state directory the file names
            ASSERT_TRUE( WriteFile( conf, "# test\nstate-dir = " + fromFile + "\nno-mdns\n" ) );
            Int32 exitCode = RunLabLink( program, Args( "--config", conf.C_String(), "--init" ) );
            GUCEF_TESTFW_SKIP_IF( 3 == exitCode, "codecspluginMONOCYPHER is not next to gucefLabLink" );
            ASSERT_TRUE( 0 == exitCode );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( fromFile, "secret" ) ) );

            // the command line wins over the file; --config=FILE is the same
            ASSERT_TRUE( 0 == RunLabLink( program, Args( ( "--config=" + conf ).C_String(), "--state-dir", fromCommandLine.C_String(), "--init" ) ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( fromCommandLine, "secret" ) ) );

            // refused, with exit code 2 and nothing made: an option that is not known, an option without its value,
            // a command in the file, a syntax error, a file that is not there
            const CORE::CString other = CORE::CombinePath( dir.GetPath(), "other" );
            const char* wrong[] = { "no-such-option = 1\n", "permanent\nrole\n", "list\n", "init\n", "help\n", "role =\n" };
            for ( size_t i=0; i<sizeof( wrong ) / sizeof( wrong[ 0 ] ); ++i )
            {
                ASSERT_TRUE( WriteFile( conf, "state-dir = " + other + "\n" + wrong[ i ] ) );
                ASSERT_TRUE( 2 == RunLabLink( program, Args( "--config", conf.C_String(), "--init" ) ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( other, "secret" ) ) );
            }
            ASSERT_TRUE( 2 == RunLabLink( program, Args( "--config", CORE::CombinePath( dir.GetPath(), "missing.conf" ).C_String(), "--init" ) ) );
            ASSERT_TRUE( 2 == RunLabLink( program, Args( "--state-dir", other.C_String(), "--init", "--config" ) ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( WriteFile( conf, "state-dir = " + other + "\n", 0666 ) );
            ASSERT_TRUE( 2 == RunLabLink( program, Args( "--config", conf.C_String(), "--init" ) ) );
            #endif
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( other, "secret" ) ) );

            // without --config nothing changes: the command line alone
            ASSERT_TRUE( 0 == RunLabLink( program, Args( "--state-dir", other.C_String(), "--init" ) ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( other, "secret" ) ) );
            ASSERT_TRUE( 0 == RunLabLink( program, Args( "--help" ) ) );
            ASSERT_TRUE( 2 == RunLabLink( program, Args( "--no-such-option" ) ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
