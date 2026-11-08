/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <string.h>
#include <signal.h>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_PROCESSARGUMENTS_H
#include "gucefCORE_ProcessArguments.h"
#define GUCEF_CORE_PROCESSARGUMENTS_H
#endif /* GUCEF_CORE_PROCESSARGUMENTS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestServiceHosting.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The ParseParams() that pubsub2pubsub and 16 other tools had, character for character */
void
LegacyToolParseParams( const int argc, char* argv[], CValueList& keyValueList, char pairSeparator )
{
    CString argString;
    if ( argc > 0 )
    {
        argString = *argv;
        for ( int i=1; i<argc; ++i )
        {
            argString += ' ' + CString( argv[ i ] );
        }
        keyValueList.SetMultiple( argString, pairSeparator );
    }
}

/*-------------------------------------------------------------------------*/

/** An argv for the tests: up to 6 arguments */
class CTestArgv
{
    public:

    CTestArgv( const char* a0, const char* a1 = GUCEF_NULL, const char* a2 = GUCEF_NULL, const char* a3 = GUCEF_NULL, const char* a4 = GUCEF_NULL )
        : argc( 0 )
    {
        const char* all[ 5 ] = { a0, a1, a2, a3, a4 };
        for ( int i=0; i<5 && GUCEF_NULL != all[ i ]; ++i )
        {
            m_texts[ i ] = all[ i ];
            ++argc;
        }
        for ( int i=0; i<argc; ++i )
            argv[ i ] = const_cast< char* >( m_texts[ i ].C_String() );
        argv[ argc ] = GUCEF_NULL;
    }

    int argc;
    char* argv[ 6 ];

    private:

    CString m_texts[ 5 ];
};

/*-------------------------------------------------------------------------*/

bool
HasValue( CValueList& keyValueList, const char* key, const CString& expected )
{
    CVariant value;
    if ( !keyValueList.TryGetValue( key, value ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ServiceHosting test: no key \"" << key << "\" in " << ToString( keyValueList ) );
        return false;
    }
    if ( value.AsString() != expected )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "ServiceHosting test: key \"" << key << "\" is \"" << value.AsString() << "\", expected \"" << expected << "\"" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
HasKey( CValueList& keyValueList, const char* key )
{
    CVariant value;
    return keyValueList.TryGetValue( key, value );
}

/*-------------------------------------------------------------------------*/

/**
 *  The minimalLogLevel lines of tools/ProcessMetrics/src/main.cpp as they were: returns the level that was set on the
 *  log manager, -1 when none was set
 */
Int32
ProcessMetricsMinLogLevelAsItWas( CValueList& keyValueList )
{
    Int32 applied = -1;
    Int32 minLogLevel = LOGLEVEL_BELOW_NORMAL;
    CVariant value;
    if ( !keyValueList.TryGetValue( "minimalLogLevel", value ) )
    {
        minLogLevel = value.AsInt32( minLogLevel );
        applied = minLogLevel;
    }
    return applied;
}

/*-------------------------------------------------------------------------*/

/** The same lines as they are now */
Int32
ProcessMetricsMinLogLevelAsItIs( CValueList& keyValueList )
{
    Int32 minLogLevel = LOGLEVEL_BELOW_NORMAL;
    CVariant value;
    if ( keyValueList.TryGetValue( "minimalLogLevel", value ) )
        minLogLevel = value.AsInt32( minLogLevel );
    return minLogLevel;
}

/*-------------------------------------------------------------------------*/

/** Raises SIGTERM in the process from the first timer cycle, so while the application loop runs */
class CSignalRaisingTimerObserver : public CObserver
{
    public:

    CSignalRaisingTimerObserver( void ) : CObserver(), raised( false ) {}

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString classTypeName = "CSignalRaisingTimerObserver";
        return classTypeName;
    }

    bool raised;

    protected:

    virtual void OnNotify( CNotifier* notifier, const CEvent& eventid, CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CTimer::TimerUpdateEvent == eventid && !raised )
        {
            raised = true;
            raise( SIGTERM );
        }
    }
};

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** Runs this test app with the stop signal arguments and gives its exit code, -1 when it did not end in time */
Int32
RunStopSignalChild( const char* when )
{
    char exePath[ 4096 ];
    ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
    if ( exePathLength <= 0 )
        return -2;
    exePath[ exePathLength ] = '\0';

    CProcess::TStringVector args;
    args.push_back( GUCEF_CORE_TESTAPP_STOP_SIGNAL_ARG );
    args.push_back( when );
    CProcess application;
    if ( !application.Launch( exePath, args, RelativePath( "$CURWORKDIR$" ), TESTFW::ChildEnvironmentWithOwnExitCode() ) )
        return -3;
    Int32 exitCode = -1;
    if ( !application.WaitForExit( 60000, exitCode ) )
        return -1;
    return exitCode;
}

#endif /* GUCEF_PLATFORM_LINUX ? */

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

int
RunStopSignalApplication( bool signalBeforeRun )
{GUCEF_TRACE;

    CGUCEFApplication& app = CCoreGlobal::Instance()->GetApplication();
    if ( signalBeforeRun )
    {
        // the signal came before the handler: it was kept and handed to the handler when that was installed
        if ( !CGUCEFApplication::IsStopRequestedFromSignal() || SIGTERM != CGUCEFApplication::GetStopRequestSignal() )
            return 3;
        // and the application does not start at all
        int result = app.main( 0, GUCEF_NULL, true );
        return 0 == result ? 0 : 4;
    }

    if ( CGUCEFApplication::IsStopRequestedFromSignal() )
        return 5;
    CSignalRaisingTimerObserver observer;
    CTimer timer( CCoreGlobal::Instance()->GetPulseGenerator(), 100 );
    observer.SubscribeTo( &timer );
    timer.SetEnabled( true );
    int result = app.main( 0, GUCEF_NULL, true );
    timer.SetEnabled( false );
    if ( !observer.raised )
        return 6;
    return 0 == result ? 0 : 7;
}

/*-------------------------------------------------------------------------*/

void
PerformServiceHostingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING SERVICE HOSTING TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceHosting" );

    GUCEF_TESTFW_TESTCASE( "Test: Recorded: the ParseParams() of the tools joins the Linux program path into the first key" )
        try
        {
            // Linux: argv[ 0 ] is the program path; "configPath" is not found, the first key carries the path
            CTestArgv linuxArgs( "/opt/gucef/pubsub2pubsub", "configPath=/x.json" );
            CValueList joined;
            LegacyToolParseParams( linuxArgs.argc, linuxArgs.argv, joined, '*' );
            ASSERT_FALSE( HasKey( joined, "configPath" ) );
            ASSERT_TRUE( HasValue( joined, "/opt/gucef/pubsub2pubsub configPath", "/x.json" ) );

            // only a leading '*' got the key through
            CTestArgv starArgs( "/opt/gucef/pubsub2pubsub", "*configPath=/x.json" );
            CValueList starred;
            LegacyToolParseParams( starArgs.argc, starArgs.argv, starred, '*' );
            ASSERT_TRUE( HasValue( starred, "configPath", "/x.json" ) );

            // a value with '=' kept what came after the last '='
            CTestArgv equalsArgs( "/opt/gucef/pubsub2pubsub", "*query=a=b" );
            CValueList equals;
            LegacyToolParseParams( equalsArgs.argc, equalsArgs.argv, equals, '*' );
            ASSERT_TRUE( HasValue( equals, "query", "b" ) );

            // Windows, where the entry point gives the argument string as argv[ 0 ]: works without --console,
            // with " --console" in front the key is "--console configPath"
            CTestArgv windowsArgs( "configPath=C:\\x.json*bootstrapConfigPath=C:\\y.ini" );
            CValueList windows;
            LegacyToolParseParams( windowsArgs.argc, windowsArgs.argv, windows, '*' );
            ASSERT_TRUE( HasValue( windows, "configPath", "C:\\x.json" ) && HasValue( windows, "bootstrapConfigPath", "C:\\y.ini" ) );
            CTestArgv consoleArgs( "--console configPath=C:\\x.json" );
            CValueList console;
            LegacyToolParseParams( consoleArgs.argc, consoleArgs.argv, console, '*' );
            ASSERT_FALSE( HasKey( console, "configPath" ) );
            ASSERT_TRUE( HasValue( console, "--console configPath", "C:\\x.json" ) );

            // a part without '=' became a key with itself as the value
            CTestArgv starConsoleArgs( "--console*configPath=C:\\x.json" );
            CValueList starConsole;
            LegacyToolParseParams( starConsoleArgs.argc, starConsoleArgs.argv, starConsole, '*' );
            ASSERT_TRUE( HasValue( starConsole, "configPath", "C:\\x.json" ) && HasValue( starConsole, "--console", "--console" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ParseApplicationArguments() skips the program path and reads key=value from every argument" )
        try
        {
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

            CTestArgv plainArgs( "/opt/gucef/pubsub2pubsub", "configPath=/x.json" );
            CValueList plain;
            ParseApplicationArguments( plainArgs.argc, plainArgs.argv, plain );
            ASSERT_TRUE( HasValue( plain, "configPath", "/x.json" ) );
            ASSERT_TRUE( 1 == plain.GetKeyCount() );

            // what worked before still works
            CTestArgv starArgs( "/opt/gucef/pubsub2pubsub", "*configPath=/x.json" );
            CValueList starred;
            ParseApplicationArguments( starArgs.argc, starArgs.argv, starred );
            ASSERT_TRUE( HasValue( starred, "configPath", "/x.json" ) && 1 == starred.GetKeyCount() );

            // several pairs in one argument, several arguments, --console as its own argument
            CTestArgv manyArgs( "/opt/gucef/tool", "--console", "bootstrapConfigPath=/b.ini*configPath=/c.json", "outputDir=/out" );
            CValueList many;
            ParseApplicationArguments( manyArgs.argc, manyArgs.argv, many );
            ASSERT_TRUE( HasValue( many, "bootstrapConfigPath", "/b.ini" ) && HasValue( many, "configPath", "/c.json" ) && HasValue( many, "outputDir", "/out" ) );
            ASSERT_FALSE( HasKey( many, "--console" ) );
            ASSERT_TRUE( 3 == many.GetKeyCount() );

            // the value is everything after the first '=', a key without '=' has an empty value, space around a part goes
            CTestArgv valueArgs( "/opt/gucef/tool", "query=a=b* flag *name = x " );
            CValueList values;
            ParseApplicationArguments( valueArgs.argc, valueArgs.argv, values );
            ASSERT_TRUE( HasValue( values, "query", "a=b" ) && HasValue( values, "flag", "" ) );
            ASSERT_TRUE( HasValue( values, "name ", " x" ) );

            // the shape of a Windows argument string: --console in front of a pair, also several options
            CTestArgv windowsArgs( "/opt/gucef/tool", "--console configPath=C:\\x.json*--verbose --console outputDir=C:\\out" );
            CValueList windows;
            ParseApplicationArguments( windowsArgs.argc, windowsArgs.argv, windows );
            ASSERT_TRUE( HasValue( windows, "configPath", "C:\\x.json" ) && HasValue( windows, "outputDir", "C:\\out" ) );
            ASSERT_TRUE( 2 == windows.GetKeyCount() );

            // the separator of the tools that use a quote
            CTestArgv quoteArgs( "/opt/gucef/GucefArchiver", "'configPath=/a.json'outputDir=/o" );
            CValueList quoted;
            ParseApplicationArguments( quoteArgs.argc, quoteArgs.argv, quoted, '\'' );
            ASSERT_TRUE( HasValue( quoted, "configPath", "/a.json" ) && HasValue( quoted, "outputDir", "/o" ) );

            // nothing at all
            CValueList none;
            ParseApplicationArguments( 0, GUCEF_NULL, none );
            CTestArgv onlyPath( "/opt/gucef/tool" );
            ParseApplicationArguments( onlyPath.argc, onlyPath.argv, none );
            ASSERT_TRUE( 0 == none.GetKeyCount() );

            #else

            GUCEF_TESTFW_SKIP( "The argument vector shapes of these cases are the POSIX ones" );

            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetProcessArguments() gives the arguments without the program path as they are" )
        try
        {
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

            CTestArgv args( "/opt/invest/InvestmentCli", "--remote", "host name", "", "--format=json" );
            CString::StringVector arguments;
            ASSERT_TRUE( GetProcessArguments( args.argc, args.argv, arguments ) );
            ASSERT_TRUE( 4 == arguments.size() );
            if ( 4 == arguments.size() )
            {
                ASSERT_TRUE( "--remote" == arguments[ 0 ] && "host name" == arguments[ 1 ] );
                ASSERT_TRUE( arguments[ 2 ].IsNULLOrEmpty() && "--format=json" == arguments[ 3 ] );
            }
            ASSERT_TRUE( GetProcessArguments( 0, GUCEF_NULL, arguments ) && arguments.empty() );
            CTestArgv onlyPath( "/opt/invest/InvestmentCli" );
            ASSERT_TRUE( GetProcessArguments( onlyPath.argc, onlyPath.argv, arguments ) && arguments.empty() );

            #else

            // To verify on Windows: quoted arguments with spaces, an empty argument, a trailing backslash before a quote
            GUCEF_TESTFW_SKIP( "The Windows split is checked by hand on Windows" );

            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Windows entry point finds --console as a whole token only" )
        try
        {
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "--console" ) );
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "configPath=C:\\x.json --console" ) );
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "a*--console*b" ) );
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "\"--console\" x" ) );
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "x\t--console\ty" ) );
            ASSERT_TRUE( 0 == gucef_osmain_has_console_arg( "--console-log" ) );
            ASSERT_TRUE( 0 == gucef_osmain_has_console_arg( "x--console" ) );
            ASSERT_TRUE( 0 == gucef_osmain_has_console_arg( "configPath=\"C:\\my--console\\x.json\"" ) );
            ASSERT_TRUE( 0 == gucef_osmain_has_console_arg( "configPath=C:\\a\\--console\\x.json" ) );
            ASSERT_TRUE( 0 == gucef_osmain_has_console_arg( "" ) );
            // a later whole token after a partial one counts
            ASSERT_TRUE( 1 == gucef_osmain_has_console_arg( "--console-log --console" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The minimalLogLevel of ProcessMetrics is applied when given and the default otherwise" )
        try
        {
            CValueList given;
            given.Set( "minimalLogLevel", CVariant( CString( "0" ) ) );
            CValueList absent;

            // as it was: the given level was never applied, the default only when the key was absent
            ASSERT_TRUE( -1 == ProcessMetricsMinLogLevelAsItWas( given ) );
            ASSERT_TRUE( LOGLEVEL_BELOW_NORMAL == ProcessMetricsMinLogLevelAsItWas( absent ) );

            // as it is
            ASSERT_TRUE( 0 == ProcessMetricsMinLogLevelAsItIs( given ) );
            ASSERT_TRUE( LOGLEVEL_BELOW_NORMAL == ProcessMetricsMinLogLevelAsItIs( absent ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A stop asked for from a signal handler is only a stored request until the application acts on it" )
        try
        {
            // in this process the application must not stop: only the request side is looked at, without a signal
            ASSERT_FALSE( CGUCEFApplication::IsStopRequestedFromSignal() );
            ASSERT_TRUE( 0 == CGUCEFApplication::GetStopRequestSignal() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_TESTFW_TESTCASE( "Test: SIGTERM before the start of a service is kept, handed to its handler and ends the process with 0" )
        try
        {
            ASSERT_TRUE( 0 == RunStopSignalChild( "before" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SIGTERM while the application loop runs ends the loop at the next pulse and the process with 0" )
        try
        {
            ASSERT_TRUE( 0 == RunStopSignalChild( "after" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An application that did not ask for deferred stop signals still ends by a SIGTERM before its handler" )
        try
        {
            // the default action: ended by the signal (128 + SIGTERM), not by its own return
            ASSERT_TRUE( 128 + SIGTERM == RunStopSignalChild( "default" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #else

    GUCEF_LOG( LOGLEVEL_NORMAL, "Stop signal TESTS skipped: they are for Linux" );

    #endif /* GUCEF_PLATFORM_LINUX ? */
}

/*-------------------------------------------------------------------------*/
