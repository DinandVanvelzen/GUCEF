/*
 *  gucef common header: lightweight header-only test framework with JUnit XML output
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*
 *  STB-style single-header test framework.
 *
 *  Usage:
 *    - In every test TU:   #include "gucef_test_framework.h"
 *    - In exactly ONE TU:  #define GUCEF_TESTFW_IMPL
 *                          #include "gucef_test_framework.h"
 *    (put #define GUCEF_TESTFW_IMPL before the include in the main.cpp of each test app)
 *
 *  Quick-start pattern in a test function:
 *
 *    void PerformMyTests( void )
 *    {
 *        GUCEF_TESTFW_SUITE_SCOPE( "MySuite" );
 *
 *        GUCEF_TESTFW_TESTCASE( "Test 1: something basic" )
 *            GUCEF_TESTFW_ASSERT_TRUE( 1 + 1 == 2 );
 *        GUCEF_TESTFW_TESTCASE_END
 *    }
 *
 *  In main() / GUCEF_OSMAIN_BEGIN:
 *
 *    GUCEF_TESTFW_INIT( "MyApp" );
 *    PerformMyTests();
 *    GUCEF_TESTFW_WRITE_RESULTS( "MyApp_Results.xml" );
 *    return GUCEF_TESTFW_EXITCODE;
 *
 *  Optional: Hook logging via callback:
 *
 *    void MyLogCallback( ETestEvent event, ... ) { ... }
 *    GUCEF_TESTFW_SET_CALLBACK( MyLogCallback, NULL );
 */

#ifndef GUCEF_TEST_FRAMEWORK_H
#define GUCEF_TEST_FRAMEWORK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      C++ GUARD                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#include <string>
#include <vector>
#include <ctime>    /* clock(), clock_t, CLOCKS_PER_SEC */
#include <cstdlib>  /* getenv() */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Event types for the test framework callback mechanism.
 *  Allows external code (like logging systems) to hook into test events.
 */
enum ETestEvent
{
    GUCEF_TESTFW_EVENT_INIT = 0,        /**< Test framework initialized */
    GUCEF_TESTFW_EVENT_SUITE_BEGIN,     /**< Test suite started */
    GUCEF_TESTFW_EVENT_SUITE_END,       /**< Test suite ended */
    GUCEF_TESTFW_EVENT_CASE_BEGIN,      /**< Test case started */
    GUCEF_TESTFW_EVENT_CASE_END,        /**< Test case ended (check failureCount) */
    GUCEF_TESTFW_EVENT_FAILURE,         /**< Assertion failure recorded */
    GUCEF_TESTFW_EVENT_EXCEPTION,       /**< Exception caught in test case */
    GUCEF_TESTFW_EVENT_RESULTS_WRITTEN, /**< Results XML file written */
    GUCEF_TESTFW_EVENT_SKIPPED          /**< Test case skipped, message holds the reason */
};

/*-------------------------------------------------------------------------*/

/**
 *  Callback function signature for test event notifications.
 *  
 *  @param eventType    The type of event that occurred
 *  @param suiteName    Current suite name (may be NULL)
 *  @param caseName     Current test case name (may be NULL)
 *  @param message      Additional info: failure expression, exception message, filepath, etc.
 *  @param file         Source file where event occurred (for failures/exceptions)
 *  @param line         Line number in source file (for failures/exceptions)
 *  @param failureCount Number of failures in current case (for CASE_END)
 *  @param userData     User-provided context pointer
 */
typedef void (*TTestEventCallback)( 
    ETestEvent  eventType,
    const char* suiteName,
    const char* caseName,
    const char* message,
    const char* file,
    Int32       line,
    UInt32      failureCount,
    void*       userData
);

/*-------------------------------------------------------------------------*/

/**
 *  Represents a single assertion or exception failure within a test case.
 */
struct SFailure
{
    std::string expr;   /**< Stringified expression or exception message */
    std::string file;   /**< Source file where the failure was recorded   */
    Int32       line;   /**< Line number in the source file               */
};

/*-------------------------------------------------------------------------*/

/**
 *  Aggregates all failures for one named test case.
 */
struct STestCase
{
    std::string           name;
    std::vector<SFailure> failures;
    double                durationSec;
    bool                  skipped;      /**< true when the case opted out via GUCEF_TESTFW_SKIP */
    std::string           skipReason;   /**< why the case was skipped */

    STestCase() : durationSec( 0.0 ), skipped( false ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Thrown by GUCEF_TESTFW_SKIP to abandon the body of a test case without
 *  counting it as a failure. Caught by GUCEF_TESTFW_TESTCASE_END.
 */
struct STestSkipRequest
{
    std::string reason;

    explicit STestSkipRequest( const char* why ) : reason( why != GUCEF_NULL ? why : "" ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Read an environment variable, for tests that are gated on external data
 *  (for example an installation or disc image the repository cannot ship).
 *  @return the value, or an empty string when the variable is not set.
 */
std::string GetEnvVar( const char* name );

/*-------------------------------------------------------------------------*/

/**
 *  Aggregates all test cases for one named test suite.
 */
struct STestSuite
{
    std::string            name;
    std::vector<STestCase> cases;
    double                 durationSec;
    bool                   nested;      /**< opened while another suite was open: its time is part of that one's */

    STestSuite() : durationSec( 0.0 ), nested( false ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Global singleton that accumulates all test results for a process run.
 *  Thread-unsafe by design - test registration is expected to be sequential.
 *
 *  Suites and test cases nest: a suite opened while another one is open (a Perform...Tests() function that calls
 *  another one) is recorded as a suite of its own, and when it ends the suite and the test case that were open before
 *  are current again, so the cases and failures that follow it are recorded where they belong. A test case outside
 *  every suite is recorded in a suite named after the application. The results stay flat: every suite is a
 *  testsuite element of its own in the JUnit XML.
 */
class CTestContext
{
public:

    /** The name of the test case that holds the failures recorded outside every test case of a suite */
    static const char* const ImplicitCaseName;

    static CTestContext& Instance();

    /** Call once at process start to name the overall test run. */
    void Init( const char* appName );

    /** 
     *  Set a callback to receive test event notifications.
     *  @param callback  Function to call on events (NULL to disable)
     *  @param userData  Opaque pointer passed to callback
     */
    void SetEventCallback( TTestEventCallback callback, void* userData );

    /** Open a new test suite, inside the one that is open, if any.  Matched by EndSuite(). */
    void BeginSuite( const char* name );

    /** Close the current test suite and record its wall-clock duration; the suite it was opened in is current again. */
    void EndSuite();

    /** Open a new test case within the current suite (the suite of the application outside every suite). */
    void BeginTestCase( const char* name );

    /** Close the current test case and record its duration; the case it was opened in is current again. */
    void EndTestCase();

    /**
     *  Record an assertion failure for the current test case.
     *  Outside every test case (in a suite, in a suite nested in a case, or outside every suite) the failure is
     *  recorded in a test case of the current suite named ImplicitCaseName, made when the first such failure
     *  happens there, so it shows in the results and in the exit code.
     */
    void RecordFailure( const char* expr, const char* file, Int32 line );

    /**
     *  Record an exception for the current test case, outside every test case as RecordFailure() does.
     *  Similar to RecordFailure but fires EXCEPTION event instead of FAILURE.
     */
    void RecordException( const char* message, const char* file, Int32 line );

    /**
     *  Mark the current test case as skipped. A skipped case is not a failure
     *  and does not influence the exit code, it is reported as <skipped> in the XML.
     */
    void RecordSkip( const char* reason, const char* file, Int32 line );

    /** @return the number of test cases that were skipped so far. */
    UInt32 GetSkippedCount() const;

    /** @return the number of test cases recorded so far in the suites of that name. */
    UInt32 GetCaseCount( const char* suiteName ) const;

    /** @return the number of suites recorded so far (a suite opened twice counts twice). */
    UInt32 GetSuiteCount() const;

    /** @return the name of the suite that is open where this is called, NULL outside every suite. */
    const char* GetCurrentSuiteName() const;

    /** @return the name of the test case that is open where this is called, NULL outside every test case. */
    const char* GetCurrentCaseName() const;

    /** @return the name given to Init(). */
    const char* GetAppName() const;

    /**
     *  Write a JUnit-compatible XML results file.
     *  @return true  on success, false if the file could not be opened.
     */
    bool WriteJUnitXml( const char* filepath ) const;

    /**
     *  @return 0 when every recorded test case passed; 1 if any failed.
     *          Suitable for direct use as a process exit code.
     */
    Int32 GetExitCode() const;

private:

    /** A suite that is open: its place in m_suites (indices stay valid when m_suites grows, pointers do not) */
    struct SOpenSuite
    {
        size_t  suiteIndex;
        clock_t start;
    };

    /** A test case that is open */
    struct SOpenCase
    {
        size_t  suiteIndex;
        size_t  caseIndex;
        clock_t start;
    };

    CTestContext();

    void FireEvent( ETestEvent eventType, const char* message, const char* file, Int32 line, UInt32 failureCount ) const;

    STestSuite* CurrentSuite();
    const STestSuite* CurrentSuite() const;
    STestCase* CurrentCase();
    const STestCase* CurrentCase() const;

    /** The case for a failure where no case is open: the implicit case of the current suite, made when needed */
    STestCase* CaseForFailure();

    static const size_t NoSuite = static_cast<size_t>( -1 );

    std::string             m_appName;
    std::vector<STestSuite> m_suites;
    std::vector<SOpenSuite> m_openSuites;       /**< innermost last */
    std::vector<SOpenCase>  m_openCases;        /**< innermost last */
    size_t                  m_appSuite;         /**< the suite of the cases outside every suite, NoSuite until one ran */
    TTestEventCallback      m_callback;
    void*                   m_callbackUserData;
};

/*-------------------------------------------------------------------------*/

/**
 *  RAII guard that opens a suite on construction and closes it on destruction.
 *  Declare one at the top of a test function via GUCEF_TESTFW_SUITE_SCOPE.
 */
class CSuiteGuard
{
public:

    explicit CSuiteGuard( const char* name )
    {
        CTestContext::Instance().BeginSuite( name );
    }

    ~CSuiteGuard()
    {
        CTestContext::Instance().EndSuite();
    }

private:

    CSuiteGuard( const CSuiteGuard& );
    CSuiteGuard& operator=( const CSuiteGuard& );
};

/*-------------------------------------------------------------------------*/

/**
 *  RAII guard that opens a test case on construction and closes it on
 *  destruction.  Used internally by GUCEF_TESTFW_TESTCASE.
 */
class CTestCaseGuard
{
public:

    explicit CTestCaseGuard( const char* name )
    {
        CTestContext::Instance().BeginTestCase( name );
    }

    ~CTestCaseGuard()
    {
        CTestContext::Instance().EndTestCase();
    }

private:

    CTestCaseGuard( const CTestCaseGuard& );
    CTestCaseGuard& operator=( const CTestCaseGuard& );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      END OF NAMESPACE - macros follow outside the namespace             //
//                                                                         //
//-------------------------------------------------------------------------*/

}} /* namespace GUCEF::TESTFW */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Token-paste helpers for generating unique variable names via __LINE__ */
#define GUCEF_TESTFW_PASTE2(a, b)  a##b
#define GUCEF_TESTFW_PASTE(a, b)   GUCEF_TESTFW_PASTE2(a, b)

/*
 *  GUCEF_TESTFW_SUITE_SCOPE( name )
 *    Declare at the top of a test function (or any scope) to register a
 *    named suite.  The suite is closed automatically when the scope exits.
 *    Suites nest: a test function may call others that open suites of their
 *    own, the cases that follow such a call belong to the suite of the caller.
 */
#define GUCEF_TESTFW_SUITE_SCOPE( name ) \
    ::GUCEF::TESTFW::CSuiteGuard \
        GUCEF_TESTFW_PASTE( gucef_suite_, __LINE__ )( name )

/*
 *  GUCEF_TESTFW_TESTCASE( name ) ... GUCEF_TESTFW_TESTCASE_END
 *    Wrap a named test block.  Any std::exception or unknown exception thrown
 *    inside is caught and recorded as a failure rather than propagating.
 */
#define GUCEF_TESTFW_TESTCASE( name ) \
    { \
    ::GUCEF::TESTFW::CTestCaseGuard \
        GUCEF_TESTFW_PASTE( gucef_tc_, __LINE__ )( name ); \
    try {

#define GUCEF_TESTFW_TESTCASE_END \
    } catch ( const ::GUCEF::TESTFW::STestSkipRequest& gucef_skip_ ) { \
        ::GUCEF::TESTFW::CTestContext::Instance().RecordSkip( \
            gucef_skip_.reason.c_str(), __FILE__, __LINE__ ); \
    } catch ( const std::exception& gucef_ex_ ) { \
        ::GUCEF::TESTFW::CTestContext::Instance().RecordException( \
            gucef_ex_.what(), __FILE__, __LINE__ ); \
    } catch ( ... ) { \
        ::GUCEF::TESTFW::CTestContext::Instance().RecordException( \
            "Unknown exception", __FILE__, __LINE__ ); \
    } }

/*
 *  GUCEF_TESTFW_ASSERT_TRUE( expr )
 *    Records a failure if expr evaluates to false.  Does NOT halt execution.
 */
#define GUCEF_TESTFW_ASSERT_TRUE( expr ) \
    do { \
        if ( !(expr) ) \
            ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( \
                #expr, __FILE__, __LINE__ ); \
    } while (0)

/*
 *  GUCEF_TESTFW_ASSERT_FALSE( expr )
 *    Records a failure if expr evaluates to true.  Does NOT halt execution.
 */
#define GUCEF_TESTFW_ASSERT_FALSE( expr ) \
    do { \
        if ( (expr) ) \
            ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( \
                "!(" #expr ")", __FILE__, __LINE__ ); \
    } while (0)

/*
 *  GUCEF_TESTFW_SKIP( reason )
 *    Abandons the rest of the enclosing GUCEF_TESTFW_TESTCASE and reports it
 *    as skipped (not failed). Use for tests that need data or hardware that
 *    is not present, e.g. a disc image pointed to by an environment variable.
 *    Only valid inside GUCEF_TESTFW_TESTCASE ... GUCEF_TESTFW_TESTCASE_END.
 *    Skipping works by throwing, so put it before any try { } catch ( ... ) { } of your own in the test case,
 *    otherwise that catch takes the skip request for an error.
 */
#define GUCEF_TESTFW_SKIP( reason ) \
    throw ::GUCEF::TESTFW::STestSkipRequest( reason )

/*
 *  GUCEF_TESTFW_SKIP_IF( cond, reason )
 *    Skips the enclosing test case when cond is true.
 */
#define GUCEF_TESTFW_SKIP_IF( cond, reason ) \
    do { if ( (cond) ) GUCEF_TESTFW_SKIP( reason ); } while (0)

/*
 *  GUCEF_TESTFW_GETENV( name )
 *    Evaluates to the value of the environment variable as std::string,
 *    empty when it is not set.
 */
#define GUCEF_TESTFW_GETENV( name ) \
    ::GUCEF::TESTFW::GetEnvVar( name )

/*
 *  GUCEF_TESTFW_REQUIRE_ENV( var, name )
 *    Declares std::string var holding the environment variable and skips the
 *    enclosing test case when it is not set or empty.
 */
#define GUCEF_TESTFW_REQUIRE_ENV( var, name ) \
    const ::std::string var = ::GUCEF::TESTFW::GetEnvVar( name ); \
    GUCEF_TESTFW_SKIP_IF( var.empty(), "environment variable " name " is not set" )

/*
 *  GUCEF_TESTFW_ERRORHERE
 *    Unconditionally record a failure at the current source location.
 */
#define GUCEF_TESTFW_ERRORHERE \
    ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( \
        "ERRORHERE", __FILE__, __LINE__ )

/*
 *  GUCEF_TESTFW_INIT( appName )
 *    Call once in main() before running any test functions.
 */
#define GUCEF_TESTFW_INIT( appName ) \
    ::GUCEF::TESTFW::CTestContext::Instance().Init( appName )

/*
 *  GUCEF_TESTFW_SET_CALLBACK( callback, userData )
 *    Set a callback to receive test event notifications for logging, etc.
 */
#define GUCEF_TESTFW_SET_CALLBACK( callback, userData ) \
    ::GUCEF::TESTFW::CTestContext::Instance().SetEventCallback( callback, userData )

/*
 *  GUCEF_TESTFW_WRITE_RESULTS( filepath )
 *    Write the JUnit XML file after all tests have run.
 */
#define GUCEF_TESTFW_WRITE_RESULTS( filepath ) \
    ::GUCEF::TESTFW::CTestContext::Instance().WriteJUnitXml( filepath )

/*
 *  GUCEF_TESTFW_EXITCODE
 *    Expression that evaluates to 0 (all pass) or 1 (any failure).
 */
#define GUCEF_TESTFW_EXITCODE \
    ::GUCEF::TESTFW::CTestContext::Instance().GetExitCode()

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION  (compiled in exactly one translation unit)         //
//      Define GUCEF_TESTFW_IMPL before including this header in main.cpp  //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_TESTFW_IMPL

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

static std::string XmlEscape( const std::string& s )
{
    std::string out;
    out.reserve( s.size() );
    for ( std::string::size_type i = 0; i < s.size(); ++i )
    {
        const char c = s[i];
        switch ( c )
        {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default:   out += c;        break;
        }
    }
    return out;
}

/*-------------------------------------------------------------------------*/

static std::string DoubleToStr( double v )
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/*-------------------------------------------------------------------------*/

static std::string Int32ToStr( Int32 v )
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/*-------------------------------------------------------------------------*/

static std::string UInt32ToStr( UInt32 v )
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

/*-------------------------------------------------------------------------*/

std::string GetEnvVar( const char* name )
{
    if ( name == GUCEF_NULL )
        return std::string();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    char* buffer = GUCEF_NULL;
    size_t length = 0;
    if ( 0 != _dupenv_s( &buffer, &length, name ) || buffer == GUCEF_NULL )
        return std::string();
    std::string value( buffer );
    free( buffer );
    return value;

    #else

    const char* value = getenv( name );
    return value != GUCEF_NULL ? std::string( value ) : std::string();

    #endif
}

/*-------------------------------------------------------------------------*/

const char* const CTestContext::ImplicitCaseName = "(assertions outside a test case)";

/*-------------------------------------------------------------------------*/

CTestContext::CTestContext()
    : m_appName()
    , m_suites()
    , m_openSuites()
    , m_openCases()
    , m_appSuite( NoSuite )
    , m_callback( GUCEF_NULL )
    , m_callbackUserData( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

CTestContext& CTestContext::Instance()
{
    static CTestContext instance;
    return instance;
}

/*-------------------------------------------------------------------------*/

void CTestContext::SetEventCallback( TTestEventCallback callback, void* userData )
{
    m_callback = callback;
    m_callbackUserData = userData;
}

/*-------------------------------------------------------------------------*/

void CTestContext::FireEvent( ETestEvent eventType, const char* message, const char* file, Int32 line, UInt32 failureCount ) const
{
    if ( m_callback != GUCEF_NULL )
    {
        m_callback( eventType, GetCurrentSuiteName(), GetCurrentCaseName(), message, file, line, failureCount, m_callbackUserData );
    }
}

/*-------------------------------------------------------------------------*/

STestSuite* CTestContext::CurrentSuite()
{
    return const_cast<STestSuite*>( static_cast<const CTestContext*>( this )->CurrentSuite() );
}

/*-------------------------------------------------------------------------*/

const STestSuite* CTestContext::CurrentSuite() const
{
    // suites are appended when they open, so of the innermost open suite and the suite of the innermost open case the
    // one opened last has the higher index. A case outside every suite is in the suite of the application
    if ( !m_openSuites.empty() && ( m_openCases.empty() || m_openSuites.back().suiteIndex >= m_openCases.back().suiteIndex ) )
        return &m_suites[ m_openSuites.back().suiteIndex ];
    if ( !m_openCases.empty() )
        return &m_suites[ m_openCases.back().suiteIndex ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

STestCase* CTestContext::CurrentCase()
{
    return const_cast<STestCase*>( static_cast<const CTestContext*>( this )->CurrentCase() );
}

/*-------------------------------------------------------------------------*/

const STestCase* CTestContext::CurrentCase() const
{
    // a case in which the innermost open suite was opened is not current while that suite is open
    if ( m_openCases.empty() )
        return GUCEF_NULL;
    const SOpenCase& open = m_openCases.back();
    if ( !m_openSuites.empty() && m_openSuites.back().suiteIndex > open.suiteIndex )
        return GUCEF_NULL;
    return &m_suites[ open.suiteIndex ].cases[ open.caseIndex ];
}

/*-------------------------------------------------------------------------*/

STestCase* CTestContext::CaseForFailure()
{
    STestCase* currentCase = CurrentCase();
    if ( currentCase != GUCEF_NULL )
        return currentCase;

    // the innermost open suite, or outside every suite the suite of the application, made when needed
    size_t suiteIndex = NoSuite;
    if ( !m_openSuites.empty() )
    {
        suiteIndex = m_openSuites.back().suiteIndex;
    }
    else
    {
        if ( m_appSuite == NoSuite )
        {
            STestSuite suite;
            suite.name = m_appName;
            m_suites.push_back( suite );
            m_appSuite = m_suites.size() - 1;
        }
        suiteIndex = m_appSuite;
    }

    std::vector<STestCase>& cases = m_suites[ suiteIndex ].cases;
    for ( std::vector<STestCase>::iterator ci = cases.begin(); ci != cases.end(); ++ci )
    {
        if ( ci->name == ImplicitCaseName )
            return &(*ci);
    }
    STestCase implicitCase;
    implicitCase.name = ImplicitCaseName;
    cases.push_back( implicitCase );
    return &cases.back();
}

/*-------------------------------------------------------------------------*/

void CTestContext::Init( const char* appName )
{
    m_appName  = ( appName != GUCEF_NULL ) ? appName : "";
    m_suites.clear();
    m_openSuites.clear();
    m_openCases.clear();
    m_appSuite = NoSuite;

    FireEvent( GUCEF_TESTFW_EVENT_INIT, m_appName.c_str(), GUCEF_NULL, 0, 0 );
}

/*-------------------------------------------------------------------------*/

void CTestContext::BeginSuite( const char* name )
{
    STestSuite suite;
    suite.name   = ( name != GUCEF_NULL ) ? name : "";
    suite.nested = !m_openSuites.empty();
    m_suites.push_back( suite );

    SOpenSuite open;
    open.suiteIndex = m_suites.size() - 1;
    open.start      = clock();
    m_openSuites.push_back( open );

    FireEvent( GUCEF_TESTFW_EVENT_SUITE_BEGIN, GUCEF_NULL, GUCEF_NULL, 0, 0 );
}

/*-------------------------------------------------------------------------*/

void CTestContext::EndSuite()
{
    if ( m_openSuites.empty() )
        return;

    // cases of this suite that are still open end with it (the guards normally ended them already)
    const SOpenSuite open = m_openSuites.back();
    while ( !m_openCases.empty() && m_openCases.back().suiteIndex >= open.suiteIndex )
        EndTestCase();

    const clock_t now = clock();
    m_suites[ open.suiteIndex ].durationSec =
        static_cast<double>( now - open.start ) /
        static_cast<double>( CLOCKS_PER_SEC );

    FireEvent( GUCEF_TESTFW_EVENT_SUITE_END, GUCEF_NULL, GUCEF_NULL, 0, 0 );

    // the suite and the case that were open before this suite opened are current again
    m_openSuites.pop_back();
}

/*-------------------------------------------------------------------------*/

void CTestContext::BeginTestCase( const char* name )
{
    size_t suiteIndex = NoSuite;
    if ( !m_openSuites.empty() )
    {
        suiteIndex = m_openSuites.back().suiteIndex;
    }
    else
    {
        // outside every suite: the suite of the application, made when the first such case runs
        if ( m_appSuite == NoSuite )
        {
            STestSuite suite;
            suite.name = m_appName;
            m_suites.push_back( suite );
            m_appSuite = m_suites.size() - 1;
        }
        suiteIndex = m_appSuite;
    }

    STestCase tc;
    tc.name = ( name != GUCEF_NULL ) ? name : "";
    m_suites[ suiteIndex ].cases.push_back( tc );

    SOpenCase open;
    open.suiteIndex = suiteIndex;
    open.caseIndex  = m_suites[ suiteIndex ].cases.size() - 1;
    open.start      = clock();
    m_openCases.push_back( open );

    FireEvent( GUCEF_TESTFW_EVENT_CASE_BEGIN, GUCEF_NULL, GUCEF_NULL, 0, 0 );
}

/*-------------------------------------------------------------------------*/

void CTestContext::EndTestCase()
{
    STestCase* currentCase = CurrentCase();
    if ( currentCase != GUCEF_NULL )
    {
        const clock_t now = clock();
        currentCase->durationSec =
            static_cast<double>( now - m_openCases.back().start ) /
            static_cast<double>( CLOCKS_PER_SEC );

        UInt32 failureCount = static_cast<UInt32>( currentCase->failures.size() );
        FireEvent( GUCEF_TESTFW_EVENT_CASE_END, GUCEF_NULL, GUCEF_NULL, 0, failureCount );

        // the case this one was opened in is current again
        m_openCases.pop_back();
    }
}

/*-------------------------------------------------------------------------*/

void CTestContext::RecordFailure( const char* expr, const char* file, Int32 line )
{
    STestCase* currentCase = CaseForFailure();

    SFailure f;
    f.expr = ( expr != GUCEF_NULL ) ? expr : "";
    f.file = ( file != GUCEF_NULL ) ? file : "";
    f.line = line;
    currentCase->failures.push_back( f );

    FireEvent( GUCEF_TESTFW_EVENT_FAILURE, expr, file, line, static_cast<UInt32>( currentCase->failures.size() ) );
}

/*-------------------------------------------------------------------------*/

void CTestContext::RecordException( const char* message, const char* file, Int32 line )
{
    STestCase* currentCase = CaseForFailure();

    SFailure f;
    f.expr = ( message != GUCEF_NULL ) ? message : "";
    f.file = ( file != GUCEF_NULL ) ? file : "";
    f.line = line;
    currentCase->failures.push_back( f );

    FireEvent( GUCEF_TESTFW_EVENT_EXCEPTION, message, file, line, static_cast<UInt32>( currentCase->failures.size() ) );
}

/*-------------------------------------------------------------------------*/

void CTestContext::RecordSkip( const char* reason, const char* file, Int32 line )
{
    STestCase* currentCase = CurrentCase();
    if ( currentCase == GUCEF_NULL )
        return;

    currentCase->skipped = true;
    currentCase->skipReason = ( reason != GUCEF_NULL ) ? reason : "";

    FireEvent( GUCEF_TESTFW_EVENT_SKIPPED, reason, file, line, 0 );
}

/*-------------------------------------------------------------------------*/

UInt32 CTestContext::GetSkippedCount() const
{
    UInt32 count = 0;
    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( ci->skipped )
                ++count;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32 CTestContext::GetCaseCount( const char* suiteName ) const
{
    UInt32 count = 0;
    if ( suiteName == GUCEF_NULL )
        return count;
    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        if ( si->name == suiteName )
            count += static_cast<UInt32>( si->cases.size() );
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32 CTestContext::GetSuiteCount() const
{
    return static_cast<UInt32>( m_suites.size() );
}

/*-------------------------------------------------------------------------*/

const char* CTestContext::GetCurrentSuiteName() const
{
    const STestSuite* currentSuite = CurrentSuite();
    return ( currentSuite != GUCEF_NULL ) ? currentSuite->name.c_str() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const char* CTestContext::GetCurrentCaseName() const
{
    const STestCase* currentCase = CurrentCase();
    return ( currentCase != GUCEF_NULL ) ? currentCase->name.c_str() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const char* CTestContext::GetAppName() const
{
    return m_appName.c_str();
}

/*-------------------------------------------------------------------------*/

bool CTestContext::WriteJUnitXml( const char* filepath ) const
{
    if ( filepath == GUCEF_NULL )
        return false;

    std::ofstream out( filepath );
    if ( !out.is_open() )
        return false;

    /* Compute overall totals across all suites */
    UInt32 totalTests    = 0;
    UInt32 totalFailures = 0;
    UInt32 totalSkipped  = 0;
    double totalTime     = 0.0;

    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        totalTests += static_cast<UInt32>( si->cases.size() );
        if ( !si->nested )
            totalTime += si->durationSec;     // a nested suite's time is part of the time of the suite it ran in
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( !ci->failures.empty() )
                ++totalFailures;
            if ( ci->skipped )
                ++totalSkipped;
        }
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<testsuites"
        << " name=\""     << XmlEscape( m_appName )        << "\""
        << " tests=\""    << UInt32ToStr( totalTests )      << "\""
        << " failures=\"" << UInt32ToStr( totalFailures )   << "\""
        << " skipped=\""  << UInt32ToStr( totalSkipped )    << "\""
        << " errors=\"0\""
        << " time=\""     << DoubleToStr( totalTime )       << "\""
        << ">\n";

    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        UInt32 suiteFailures = 0;
        UInt32 suiteSkipped = 0;
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( !ci->failures.empty() )
                ++suiteFailures;
            if ( ci->skipped )
                ++suiteSkipped;
        }

        out << "  <testsuite"
            << " name=\""     << XmlEscape( si->name ) << "\""
            << " tests=\""    << UInt32ToStr( static_cast<UInt32>( si->cases.size() ) ) << "\""
            << " failures=\"" << UInt32ToStr( suiteFailures ) << "\""
            << " skipped=\""  << UInt32ToStr( suiteSkipped ) << "\""
            << " errors=\"0\""
            << " time=\""     << DoubleToStr( si->durationSec ) << "\""
            << ">\n";

        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            out << "    <testcase"
                << " name=\""      << XmlEscape( ci->name )          << "\""
                << " classname=\"" << XmlEscape( si->name )          << "\""
                << " time=\""      << DoubleToStr( ci->durationSec ) << "\"";

            if ( ci->skipped && ci->failures.empty() )
            {
                out << ">\n";
                out << "      <skipped message=\"" << XmlEscape( ci->skipReason ) << "\"/>\n";
                out << "    </testcase>\n";
            }
            else if ( ci->failures.empty() )
            {
                out << "/>\n";
            }
            else
            {
                out << ">\n";
                for ( std::vector<SFailure>::const_iterator fi = ci->failures.begin();
                      fi != ci->failures.end(); ++fi )
                {
                    std::string loc = fi->file + "(" + Int32ToStr( fi->line ) + "): " + fi->expr;
                    out << "      <failure"
                        << " message=\"" << XmlEscape( fi->expr ) << "\""
                        << " type=\"ASSERTION_FAILURE\""
                        << ">"
                        << XmlEscape( loc )
                        << "</failure>\n";
                }
                out << "    </testcase>\n";
            }
        }

        out << "  </testsuite>\n";
    }

    out << "</testsuites>\n";
    
    bool success = out.good();
    
    // Fire event after writing
    const_cast<CTestContext*>(this)->FireEvent( GUCEF_TESTFW_EVENT_RESULTS_WRITTEN, filepath, GUCEF_NULL, 0, totalFailures );
    
    return success;
}

/*-------------------------------------------------------------------------*/

Int32 CTestContext::GetExitCode() const
{
    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( !ci->failures.empty() )
                return 1;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

}} /* namespace GUCEF::TESTFW */

#endif /* GUCEF_TESTFW_IMPL ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      END OF C++ GUARD                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_TEST_FRAMEWORK_H ? */
