/*
 *  MemoryLeakFinder_TestApp: Test application for the MemoryLeakFinder module
 *  Copyright (C) 2002 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_MLF_MEMORYMANAGER_H
#include "gucefMLF_MemoryManager.h"
#define GUCEF_MLF_MEMORYMANAGER_H
#endif /* GUCEF_MLF_MEMORYMANAGER_H ? */

#ifndef GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#include "gucefMLF_SMemoryTrackerConfig.h"
#define GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_MLF_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestConfig.h"

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

void
PerformConfigTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Config TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Config" );

    /* All config tests run inside an Initialize/Shutdown pair */
    GUCEF::MLF::MEMMAN_Initialize();

    // Test 1: SMemoryTrackerConfig default values
    GUCEF_TESTFW_TESTCASE( "Test 1: SMemoryTrackerConfig default values" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: SMemoryTrackerConfig default values" );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            ASSERT_TRUE( cfg.paddingSize == 4 );
            ASSERT_TRUE( cfg.exhaustiveTesting == false );
            ASSERT_TRUE( cfg.breakOnAllocationCount == -1 );
            ASSERT_TRUE( cfg.deallocRingCapacity == 10000000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: MEMMAN_SetPaddingSize does not crash
    GUCEF_TESTFW_TESTCASE( "Test 2: SetPaddingSize does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: SetPaddingSize does not crash" );
            GUCEF::MLF::MEMMAN_SetPaddingSize( 8 );
            /* Reset to default for subsequent tests */
            GUCEF::MLF::MEMMAN_SetPaddingSize( 4 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: MEMMAN_SetLogAlways does not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: SetLogAlways does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: SetLogAlways does not crash" );
            GUCEF::MLF::MEMMAN_SetLogAlways( 1 );
            GUCEF::MLF::MEMMAN_SetLogAlways( 0 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: MEMMAN_SetExhaustiveTesting does not crash
    GUCEF_TESTFW_TESTCASE( "Test 4: SetExhaustiveTesting does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: SetExhaustiveTesting does not crash" );
            GUCEF::MLF::MEMMAN_SetExhaustiveTesting( 1 );
            GUCEF::MLF::MEMMAN_SetExhaustiveTesting( 0 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: MEMMAN_SetLogFile does not crash
    GUCEF_TESTFW_TESTCASE( "Test 5: SetLogFile does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: SetLogFile does not crash" );
            GUCEF::MLF::MEMMAN_SetLogFile( "MemoryLeakFinder_TestApp_Log.txt" );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: MEMMAN_CleanLogFile does not crash
    GUCEF_TESTFW_TESTCASE( "Test 6: CleanLogFile does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: CleanLogFile does not crash" );
            GUCEF::MLF::MEMMAN_CleanLogFile( 1 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: MEMMAN_BreakOnAllocation(-1) disables break without crash
    GUCEF_TESTFW_TESTCASE( "Test 7: BreakOnAllocation disable does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: BreakOnAllocation disable does not crash" );
            GUCEF::MLF::MEMMAN_BreakOnAllocation( -1 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: MEMMAN_BreakOnDeallocation(NULL) does not crash
    GUCEF_TESTFW_TESTCASE( "Test 8: BreakOnDeallocation NULL does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: BreakOnDeallocation NULL does not crash" );
            GUCEF::MLF::MEMMAN_BreakOnDeallocation( GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: New config fields default values
    GUCEF_TESTFW_TESTCASE( "Test 9: New config fields default values" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: New config fields default values" );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            ASSERT_TRUE( cfg.enableRawCallstackCapture == true );
            ASSERT_TRUE( cfg.maxRawCallstackDepth == 32 );
            ASSERT_TRUE( cfg.deallocMismatchResponse == GUCEF::MLF::MISMATCH_LOG );
            ASSERT_TRUE( cfg.enableCallsiteProfiling == false );
            ASSERT_TRUE( cfg.useGuardPages == false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: enableRawCallstackCapture toggle does not crash
    GUCEF_TESTFW_TESTCASE( "Test 10: enableRawCallstackCapture toggle no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: enableRawCallstackCapture toggle no crash" );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.enableRawCallstackCapture = true;
            cfg.enableRawCallstackCapture = false;
            cfg.enableRawCallstackCapture = true;
            ASSERT_TRUE( cfg.enableRawCallstackCapture == true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: maxRawCallstackDepth assignment does not crash
    GUCEF_TESTFW_TESTCASE( "Test 11: maxRawCallstackDepth assignment no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: maxRawCallstackDepth assignment no crash" );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.maxRawCallstackDepth = 1;
            cfg.maxRawCallstackDepth = 62;
            cfg.maxRawCallstackDepth = 32;
            ASSERT_TRUE( cfg.maxRawCallstackDepth == 32 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: EMismatchResponse enum values are correct and assignment does not crash
    GUCEF_TESTFW_TESTCASE( "Test 12: EMismatchResponse enum values correct" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: EMismatchResponse enum values correct" );
            ASSERT_TRUE( (int) GUCEF::MLF::MISMATCH_LOG   == 0 );
            ASSERT_TRUE( (int) GUCEF::MLF::MISMATCH_BREAK == 1 );
            ASSERT_TRUE( (int) GUCEF::MLF::MISMATCH_ABORT == 2 );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.deallocMismatchResponse = GUCEF::MLF::MISMATCH_LOG;
            ASSERT_TRUE( cfg.deallocMismatchResponse == GUCEF::MLF::MISMATCH_LOG );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: enableCallsiteProfiling toggle does not crash
    GUCEF_TESTFW_TESTCASE( "Test 13: enableCallsiteProfiling toggle no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: enableCallsiteProfiling toggle no crash" );
            GUCEF::MLF::SMemoryTrackerConfig cfg;
            GUCEF::MLF::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.enableCallsiteProfiling = true;
            cfg.enableCallsiteProfiling = false;
            ASSERT_TRUE( cfg.enableCallsiteProfiling == false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::MLF::MEMMAN_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Config TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
