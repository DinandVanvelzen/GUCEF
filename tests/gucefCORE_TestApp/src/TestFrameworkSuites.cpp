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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestFrameworkSuites.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const OuterSuiteName = "TestFrameworkSuites";
const char* const InnerSuiteName = "TestFrameworkSuites.Inner";
const char* const CaseAfterInnerSuite = "Test: a case that follows an inner suite is recorded in the outer suite";
const char* const CaseAroundInnerSuite = "Test: a case inside which an inner suite runs goes on in the outer suite afterwards";

TESTFW::CTestContext&
Context( void )
{
    return TESTFW::CTestContext::Instance();
}

/*-------------------------------------------------------------------------*/

bool
IsCurrent( const char* suiteName, const char* caseName )
{
    const char* currentSuite = Context().GetCurrentSuiteName();
    const char* currentCase = Context().GetCurrentCaseName();
    return GUCEF_NULL != currentSuite && 0 == strcmp( currentSuite, suiteName ) &&
           GUCEF_NULL != currentCase && 0 == strcmp( currentCase, caseName );
}

/*-------------------------------------------------------------------------*/

/** A suite of its own, as a Perform...Tests() function of another file opens it */
void
RunInnerSuite( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( InnerSuiteName );

    GUCEF_TESTFW_TESTCASE( "Test: a case of an inner suite is recorded in the inner suite" )
        ASSERT_TRUE( IsCurrent( InnerSuiteName, "Test: a case of an inner suite is recorded in the inner suite" ) );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether the JUnit XML of the results is well formed in the way the framework writes it: one testsuites element,
 *  testsuite elements side by side (never one inside another), every testcase inside a testsuite
 */
bool
IsFlatJUnitXml( const CORE::CString& xml, UInt32& suites )
{
    suites = 0;
    if ( xml.HasSubstr( "<testsuites", 0, true ) < 0 || xml.HasSubstr( "</testsuites>", 0, true ) < 0 )
        return false;
    Int32 position = 0;
    bool inSuite = false;
    for ( ;; )
    {
        Int32 open = xml.HasSubstr( "<testsuite ", position, true );
        Int32 close = xml.HasSubstr( "</testsuite>", position, true );
        Int32 testCase = xml.HasSubstr( "<testcase ", position, true );
        if ( !inSuite )
        {
            if ( open < 0 )
                return close < 0 && testCase < 0;
            if ( ( close >= 0 && close < open ) || ( testCase >= 0 && testCase < open ) )
                return false;
            inSuite = true;
            ++suites;
            position = open + 1;
        }
        else
        {
            if ( close < 0 || ( open >= 0 && open < close ) )
                return false;
            inSuite = false;
            position = close + 1;
        }
    }
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** Runs this test app with GUCEF_CORE_TESTAPP_ASSERT_OUTSIDE_CASES_ARG, @return its exit code or a negative number */
Int32
RunAssertionsOutsideCasesChild( const CORE::CString& resultsXmlPath )
{
    char exePath[ 4096 ];
    ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
    if ( exePathLength <= 0 )
        return -2;
    exePath[ exePathLength ] = '\0';

    CORE::CProcess::TStringVector args;
    args.push_back( GUCEF_CORE_TESTAPP_ASSERT_OUTSIDE_CASES_ARG );
    args.push_back( resultsXmlPath );
    CORE::CProcess child;
    if ( !child.Launch( exePath, args, CORE::RelativePath( "$CURWORKDIR$" ), TESTFW::ChildEnvironmentWithOwnExitCode() ) )
        return -3;
    Int32 exitCode = -1;
    if ( !child.WaitForExit( 60000, exitCode ) )
        return -1;
    return exitCode;
}

#endif /* GUCEF_PLATFORM_LINUX ? */

/*-------------------------------------------------------------------------*/

/** @return how often the option is in a TSAN_OPTIONS style list */
UInt32
SanitizerOptionCount( const CORE::CString& options, const char* name )
{
    UInt32 count = 0;
    CORE::CString::StringVector parts = options.ParseElements( ' ', false );
    for ( size_t i=0; i<parts.size(); ++i )
        if ( parts[ i ].SubstrToChar( '=', true ) == name )
            ++count;
    return count;
}

}

/*-------------------------------------------------------------------------*/

int
RunAssertionsOutsideCases( const char* resultsXmlPath )
{GUCEF_TRACE;

    // a run of its own: the assertions below fail on purpose, each outside every test case
    Context().Init( "AssertionsOutsideCases" );

    const bool outsideEverySuite = false;
    ASSERT_TRUE( outsideEverySuite );
    {
        GUCEF_TESTFW_SUITE_SCOPE( "AssertionsOutsideCases.Outer" );

        GUCEF_TESTFW_TESTCASE( "outer case, it passes" )
            {
                GUCEF_TESTFW_SUITE_SCOPE( "AssertionsOutsideCases.Inner" );

                const bool inNestedSuiteOutsideCase = false;
                ASSERT_TRUE( inNestedSuiteOutsideCase );

                GUCEF_TESTFW_TESTCASE( "inner case, it passes" )
                    ASSERT_TRUE( true );
                GUCEF_TESTFW_TESTCASE_END
            }
            ASSERT_TRUE( true );
        GUCEF_TESTFW_TESTCASE_END

        const bool inSuiteOutsideCase = false;
        ASSERT_TRUE( inSuiteOutsideCase );
    }

    Context().WriteJUnitXml( resultsXmlPath );
    return Context().GetExitCode();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTestFrameworkSuitesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING TestFramework suite TESTS" );

    const UInt32 outerAtStart = Context().GetCaseCount( OuterSuiteName );
    const UInt32 innerAtStart = Context().GetCaseCount( InnerSuiteName );
    {
        GUCEF_TESTFW_SUITE_SCOPE( OuterSuiteName );

        GUCEF_TESTFW_TESTCASE( "Test: a case before an inner suite is recorded in the outer suite" )
            ASSERT_TRUE( IsCurrent( OuterSuiteName, "Test: a case before an inner suite is recorded in the outer suite" ) );
        GUCEF_TESTFW_TESTCASE_END

        RunInnerSuite();

        GUCEF_TESTFW_TESTCASE( CaseAfterInnerSuite )
            ASSERT_TRUE( IsCurrent( OuterSuiteName, CaseAfterInnerSuite ) );
            ASSERT_TRUE( outerAtStart + 2 == Context().GetCaseCount( OuterSuiteName ) );
        GUCEF_TESTFW_TESTCASE_END

        GUCEF_TESTFW_TESTCASE( CaseAroundInnerSuite )
            RunInnerSuite();
            ASSERT_TRUE( IsCurrent( OuterSuiteName, CaseAroundInnerSuite ) );
        GUCEF_TESTFW_TESTCASE_END
    }

    // outside every suite: the case goes to a suite named after the application
    const UInt32 looseAtStart = Context().GetCaseCount( Context().GetAppName() );
    GUCEF_TESTFW_TESTCASE( "Test: a case outside every suite is recorded in the suite of the application" )
        ASSERT_TRUE( IsCurrent( Context().GetAppName(), "Test: a case outside every suite is recorded in the suite of the application" ) );
    GUCEF_TESTFW_TESTCASE_END

    // checked in a suite of its own, after the nested ones ended: a check inside a case that is not recorded never runs
    GUCEF_TESTFW_SUITE_SCOPE( "TestFrameworkSuites.Check" );

    GUCEF_TESTFW_TESTCASE( "Test: every case of the outer and the inner suites was recorded" )
        ASSERT_TRUE( outerAtStart + 3 == Context().GetCaseCount( OuterSuiteName ) );
        ASSERT_TRUE( innerAtStart + 2 == Context().GetCaseCount( InnerSuiteName ) );
        ASSERT_TRUE( looseAtStart + 1 == Context().GetCaseCount( Context().GetAppName() ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the JUnit XML of nested suites is well formed, every suite on its own" )
        try
        {
            CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( path, "TestFrameworkSuites_Results.xml" );
            ASSERT_TRUE( Context().WriteJUnitXml( path.C_String() ) );
            CORE::CString xml;
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, xml ) );
            UInt32 suites = 0;
            ASSERT_TRUE( IsFlatJUnitXml( xml, suites ) );
            ASSERT_TRUE( suites == Context().GetSuiteCount() );
            CORE::CString outer = CORE::CString( "<testsuite name=\"" ) + OuterSuiteName + "\" tests=\"" + CORE::ToString( outerAtStart + 3 ) + "\"";
            ASSERT_TRUE( xml.HasSubstr( outer, 0, true ) >= 0 );
            CORE::DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: assertions that fail outside every test case fail the run and are in its JUnit XML" )
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        try
        {
            CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( path, "AssertionsOutsideCases_Results.xml" );
            CORE::DeleteFile( path );
            const Int32 exitCode = RunAssertionsOutsideCasesChild( path );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TestFrameworkSuites: the run with assertions outside its test cases ended with " << exitCode );
            ASSERT_TRUE( 1 == exitCode );

            CORE::CString xml;
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, xml ) );
            UInt32 suites = 0;
            ASSERT_TRUE( IsFlatJUnitXml( xml, suites ) );
            // the two passing cases plus, for each place with a failing assertion outside a case, one failing case
            ASSERT_TRUE( xml.HasSubstr( "<testsuites name=\"AssertionsOutsideCases\" tests=\"5\" failures=\"3\"", 0, true ) >= 0 );
            ASSERT_TRUE( xml.HasSubstr( "message=\"outsideEverySuite\"", 0, true ) >= 0 );
            ASSERT_TRUE( xml.HasSubstr( "message=\"inNestedSuiteOutsideCase\"", 0, true ) >= 0 );
            ASSERT_TRUE( xml.HasSubstr( "message=\"inSuiteOutsideCase\"", 0, true ) >= 0 );
            ASSERT_TRUE( xml.HasSubstr( "<testsuite name=\"AssertionsOutsideCases.Inner\" tests=\"2\" failures=\"1\"", 0, true ) >= 0 );
            ASSERT_TRUE( xml.HasSubstr( "<testsuite name=\"AssertionsOutsideCases.Outer\" tests=\"2\" failures=\"1\"", 0, true ) >= 0 );
            CORE::DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        #else
        GUCEF_TESTFW_SKIP( "starts the test app itself, done on Linux" );
        #endif
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a child keeps its own exit code under the thread sanitizer and its reports go to a log" )
        try
        {
            // without a log_path of the parent: one is set, so the reports of the child are not lost with its exit code
            CORE::CProcess::TStringMap withoutLogPath;
            withoutLogPath[ "TSAN_OPTIONS" ] = "second_deadlock_stack=1";
            TESTFW::KeepChildExitCodeUnderSanitizers( withoutLogPath );
            const CORE::CString options = withoutLogPath[ "TSAN_OPTIONS" ];
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TestFrameworkSuites: TSAN_OPTIONS of a child: " << options );
            ASSERT_TRUE( TESTFW::SanitizerOptionValue( options, "exitcode" ) == "0" );
            ASSERT_TRUE( TESTFW::SanitizerOptionValue( options, "second_deadlock_stack" ) == "1" );
            const CORE::CString logPath = TESTFW::SanitizerOptionValue( options, "log_path" );
            ASSERT_FALSE( logPath.IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::DirExists( CORE::StripFilename( logPath ) ) );

            // with the log_path of the parent: the child's reports go there as well
            CORE::CProcess::TStringMap withLogPath;
            withLogPath[ "TSAN_OPTIONS" ] = "log_path=/run/of/the/parent/tsan history_size=4";
            TESTFW::KeepChildExitCodeUnderSanitizers( withLogPath );
            ASSERT_TRUE( TESTFW::SanitizerOptionValue( withLogPath[ "TSAN_OPTIONS" ], "log_path" ) == "/run/of/the/parent/tsan" );
            ASSERT_TRUE( 1 == SanitizerOptionCount( withLogPath[ "TSAN_OPTIONS" ], "log_path" ) );
            ASSERT_TRUE( TESTFW::SanitizerOptionValue( withLogPath[ "TSAN_OPTIONS" ], "exitcode" ) == "0" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
