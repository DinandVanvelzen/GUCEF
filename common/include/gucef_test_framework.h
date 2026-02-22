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
 */

#ifndef GUCEF_TEST_FRAMEWORK_H
#define GUCEF_TEST_FRAMEWORK_H

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

    STestCase() : durationSec( 0.0 ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Aggregates all test cases for one named test suite.
 */
struct STestSuite
{
    std::string            name;
    std::vector<STestCase> cases;
    double                 durationSec;

    STestSuite() : durationSec( 0.0 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Global singleton that accumulates all test results for a process run.
 *  Thread-unsafe by design — test registration is expected to be sequential.
 */
class CTestContext
{
public:

    static CTestContext& Instance();

    /** Call once at process start to name the overall test run. */
    void Init( const char* appName );

    /** Open a new test suite.  Matched by EndSuite(). */
    void BeginSuite( const char* name );

    /** Close the current test suite and record its wall-clock duration. */
    void EndSuite();

    /** Open a new test case within the current suite. */
    void BeginTestCase( const char* name );

    /** Close the current test case and record its duration. */
    void EndTestCase();

    /**
     *  Record an assertion failure or exception for the current test case.
     *  No-op when no test case is currently open.
     */
    void RecordFailure( const char* expr, const char* file, Int32 line );

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

    CTestContext();

    std::string             m_appName;
    std::vector<STestSuite> m_suites;
    STestSuite*             m_currentSuite;
    STestCase*              m_currentCase;
    clock_t                 m_suiteStart;
    clock_t                 m_caseStart;
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
//      END OF NAMESPACE — macros follow outside the namespace            //
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
    } catch ( const std::exception& gucef_ex_ ) { \
        ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( \
            gucef_ex_.what(), __FILE__, __LINE__ ); \
    } catch ( ... ) { \
        ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( \
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
//      IMPLEMENTATION  (compiled in exactly one translation unit)        //
//      Define GUCEF_TESTFW_IMPL before including this header in main.cpp //
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

CTestContext::CTestContext()
    : m_currentSuite( GUCEF_NULL )
    , m_currentCase( GUCEF_NULL )
    , m_suiteStart( 0 )
    , m_caseStart( 0 )
{
}

/*-------------------------------------------------------------------------*/

CTestContext& CTestContext::Instance()
{
    static CTestContext instance;
    return instance;
}

/*-------------------------------------------------------------------------*/

void CTestContext::Init( const char* appName )
{
    m_appName      = ( appName != GUCEF_NULL ) ? appName : "";
    m_currentSuite = GUCEF_NULL;
    m_currentCase  = GUCEF_NULL;
    m_suites.clear();
}

/*-------------------------------------------------------------------------*/

void CTestContext::BeginSuite( const char* name )
{
    STestSuite suite;
    suite.name = ( name != GUCEF_NULL ) ? name : "";
    m_suites.push_back( suite );
    m_currentSuite = &m_suites.back();
    m_currentCase  = GUCEF_NULL;
    m_suiteStart   = clock();
}

/*-------------------------------------------------------------------------*/

void CTestContext::EndSuite()
{
    if ( m_currentSuite != GUCEF_NULL )
    {
        const clock_t now = clock();
        m_currentSuite->durationSec =
            static_cast<double>( now - m_suiteStart ) /
            static_cast<double>( CLOCKS_PER_SEC );
        m_currentSuite = GUCEF_NULL;
    }
    m_currentCase = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void CTestContext::BeginTestCase( const char* name )
{
    if ( m_currentSuite == GUCEF_NULL )
        return;

    STestCase tc;
    tc.name = ( name != GUCEF_NULL ) ? name : "";
    m_currentSuite->cases.push_back( tc );
    m_currentCase = &m_currentSuite->cases.back();
    m_caseStart   = clock();
}

/*-------------------------------------------------------------------------*/

void CTestContext::EndTestCase()
{
    if ( m_currentCase != GUCEF_NULL )
    {
        const clock_t now = clock();
        m_currentCase->durationSec =
            static_cast<double>( now - m_caseStart ) /
            static_cast<double>( CLOCKS_PER_SEC );
        m_currentCase = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void CTestContext::RecordFailure( const char* expr, const char* file, Int32 line )
{
    if ( m_currentCase == GUCEF_NULL )
        return;

    SFailure f;
    f.expr = ( expr != GUCEF_NULL ) ? expr : "";
    f.file = ( file != GUCEF_NULL ) ? file : "";
    f.line = line;
    m_currentCase->failures.push_back( f );
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
    double totalTime     = 0.0;

    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        totalTests += static_cast<UInt32>( si->cases.size() );
        totalTime  += si->durationSec;
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( !ci->failures.empty() )
                ++totalFailures;
        }
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<testsuites"
        << " name=\""     << XmlEscape( m_appName )        << "\""
        << " tests=\""    << UInt32ToStr( totalTests )      << "\""
        << " failures=\"" << UInt32ToStr( totalFailures )   << "\""
        << " errors=\"0\""
        << " time=\""     << DoubleToStr( totalTime )       << "\""
        << ">\n";

    for ( std::vector<STestSuite>::const_iterator si = m_suites.begin();
          si != m_suites.end(); ++si )
    {
        UInt32 suiteFailures = 0;
        for ( std::vector<STestCase>::const_iterator ci = si->cases.begin();
              ci != si->cases.end(); ++ci )
        {
            if ( !ci->failures.empty() )
                ++suiteFailures;
        }

        out << "  <testsuite"
            << " name=\""     << XmlEscape( si->name ) << "\""
            << " tests=\""    << UInt32ToStr( static_cast<UInt32>( si->cases.size() ) ) << "\""
            << " failures=\"" << UInt32ToStr( suiteFailures ) << "\""
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

            if ( ci->failures.empty() )
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
    return out.good();
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

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_TEST_FRAMEWORK_H ? */
