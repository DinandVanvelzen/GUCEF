/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFrameworkSkip.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformTestFrameworkSkipTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING TestFramework skip TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "TestFrameworkSkip" );

    const UInt32 skippedAtStart = TESTFW::CTestContext::Instance().GetSkippedCount();

    // This case skips on purpose to prove the mechanism: it must be reported as skipped, never as failed
    GUCEF_TESTFW_TESTCASE( "Test: GUCEF_TESTFW_SKIP_IF abandons the test case without failing it" )
        GUCEF_TESTFW_SKIP_IF( true, "intentional skip, verifies the skip mechanism" );
        ERRORHERE; // never reached
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a false SKIP_IF condition lets the test case continue" )
        bool reached = false;
        GUCEF_TESTFW_SKIP_IF( false, "must not skip" );
        reached = true;
        ASSERT_TRUE( reached );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: REQUIRE_ENV skips when the variable is not set" )
        GUCEF_TESTFW_REQUIRE_ENV( value, "GUCEF_TESTFW_SURELY_NOT_SET_VARIABLE" );
        ERRORHERE; // never reached
        ASSERT_TRUE( value.empty() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: skipped cases are counted" )
        ASSERT_TRUE( TESTFW::CTestContext::Instance().GetSkippedCount() == skippedAtStart + 2 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: environment variable helper" )
        try
        {
            ASSERT_TRUE( GUCEF_TESTFW_GETENV( "GUCEF_TESTFW_SURELY_NOT_SET_VARIABLE" ).empty() );
            ASSERT_TRUE( GUCEF_TESTFW_GETENV( GUCEF_NULL ).empty() );
            // PATH exists on every platform we run tests on
            ASSERT_FALSE( GUCEF_TESTFW_GETENV( "PATH" ).empty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
