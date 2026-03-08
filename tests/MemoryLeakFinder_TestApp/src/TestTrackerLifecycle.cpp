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

#ifndef GUCEF_DRGUP_MEMORYMANAGER_H
#include "gucefDRGUP_MemoryManager.h"
#define GUCEF_DRGUP_MEMORYMANAGER_H
#endif /* GUCEF_DRGUP_MEMORYMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestTrackerLifecycle.h"

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
PerformTrackerLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING TrackerLifecycle TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "TrackerLifecycle" );

    // Test 1: Initialize returns success
    GUCEF_TESTFW_TESTCASE( "Test 1: Initialize returns success" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Initialize returns success" );
            __int32 result = GUCEF::DRGUP::DRGUP_Initialize();
            ASSERT_TRUE( result != 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Double-init is handled gracefully (no crash)
    GUCEF_TESTFW_TESTCASE( "Test 2: Double-init handled gracefully" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Double-init handled gracefully" );
            __int32 result = GUCEF::DRGUP::DRGUP_Initialize();
            /* Either succeeds (0) or returns a benign non-crash value -- no crash is the requirement */
            (void) result;
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Can allocate memory after Initialize
    GUCEF_TESTFW_TESTCASE( "Test 3: Can allocate memory after Initialize" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Can allocate memory after Initialize" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Shutdown returns success
    GUCEF_TESTFW_TESTCASE( "Test 4: Shutdown returns success" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Shutdown returns success" );
            __int32 result = GUCEF::DRGUP::DRGUP_Shutdown();
            ASSERT_TRUE( result != 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Re-initialize after shutdown succeeds
    GUCEF_TESTFW_TESTCASE( "Test 5: Re-initialize after shutdown succeeds" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Re-initialize after shutdown succeeds" );
            __int32 result = GUCEF::DRGUP::DRGUP_Initialize();
            ASSERT_TRUE( result != 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Can allocate memory after re-initialize
    GUCEF_TESTFW_TESTCASE( "Test 6: Can allocate memory after re-initialize" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Can allocate memory after re-initialize" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Second shutdown after re-init succeeds
    GUCEF_TESTFW_TESTCASE( "Test 7: Second shutdown succeeds" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Second shutdown succeeds" );
            __int32 result = GUCEF::DRGUP::DRGUP_Shutdown();
            ASSERT_TRUE( result != 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL TrackerLifecycle TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
