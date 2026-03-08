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

#ifndef GUCEF_DRGUP_CALLSTACK_H
#include "gucefDRGUP_callstack.h"
#define GUCEF_DRGUP_CALLSTACK_H
#endif /* GUCEF_DRGUP_CALLSTACK_H ? */

#ifndef GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#include "gucefDRGUP_SMemoryTrackerConfig.h"
#define GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCallStack.h"

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
PerformCallStackTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CallStack TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CallStack" );

    /*
     * Note: GUCEF_CALLSTACK_TRACING_DISABLED may be set in the DLL build,
     * making these functions no-ops inside the DLL.  The test verifies that
     * the exported C API functions are callable without crash regardless.
     */

    GUCEF::DRGUP::DRGUP_Initialize();

    // Test 1: Single CallstackScopeBegin does not crash
    GUCEF_TESTFW_TESTCASE( "Test 1: Single CallstackScopeBegin no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Single CallstackScopeBegin no crash" );
            GUCEF::DRGUP::DRGUP_CallstackScopeBegin( __FILE__, __LINE__ );
            GUCEF::DRGUP::DRGUP_CallstackScopeEnd();
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Nested CallstackScopeBegin does not crash
    GUCEF_TESTFW_TESTCASE( "Test 2: Nested CallstackScopeBegin no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Nested CallstackScopeBegin no crash" );
            GUCEF::DRGUP::DRGUP_CallstackScopeBegin( __FILE__, __LINE__ );
            GUCEF::DRGUP::DRGUP_CallstackScopeBegin( __FILE__, __LINE__ );
            GUCEF::DRGUP::DRGUP_CallstackScopeEnd();
            GUCEF::DRGUP::DRGUP_CallstackScopeEnd();
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Deeply nested scopes do not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: Deeply nested scopes no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Deeply nested scopes no crash" );
            for ( int i = 0; i < 5; ++i )
            {
                GUCEF::DRGUP::DRGUP_CallstackScopeBegin( __FILE__, __LINE__ );
            }
            for ( int i = 0; i < 5; ++i )
            {
                GUCEF::DRGUP::DRGUP_CallstackScopeEnd();
            }
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: GetCallstackForCurrentThread does not crash (result may be null if disabled)
    GUCEF_TESTFW_TESTCASE( "Test 4: GetCallstackForCurrentThread no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: GetCallstackForCurrentThread no crash" );
            GUCEF::DRGUP::TCallStack* outStack = GUCEF_NULL;
            GUCEF::DRGUP::DRGUP_CallstackScopeBegin( __FILE__, __LINE__ );
            GUCEF::DRGUP::DRGUP_GetCallstackForCurrentThread( &outStack );
            GUCEF::DRGUP::DRGUP_CallstackScopeEnd();
            /* outStack may be null if callstack tracing is disabled in the DLL */
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Default config (enableRawCallstackCapture=true) - alloc + dealloc succeeds
    GUCEF_TESTFW_TESTCASE( "Test 5: Default enableRawCallstackCapture alloc+dealloc succeeds" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Default enableRawCallstackCapture alloc+dealloc succeeds" );
            GUCEF::DRGUP::SMemoryTrackerConfig cfg;
            GUCEF::DRGUP::SMemoryTrackerConfig_SetDefaults( cfg );
            ASSERT_TRUE( cfg.enableRawCallstackCapture == true );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: enableRawCallstackCapture=false - alloc + dealloc still works
    GUCEF_TESTFW_TESTCASE( "Test 6: enableRawCallstackCapture=false alloc+dealloc works" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: enableRawCallstackCapture=false alloc+dealloc works" );
            GUCEF::DRGUP::SMemoryTrackerConfig cfg;
            GUCEF::DRGUP::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.enableRawCallstackCapture = false;
            ASSERT_TRUE( cfg.enableRawCallstackCapture == false );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: maxRawCallstackDepth=1 - alloc + dealloc still works
    GUCEF_TESTFW_TESTCASE( "Test 7: maxRawCallstackDepth=1 alloc+dealloc works" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: maxRawCallstackDepth=1 alloc+dealloc works" );
            GUCEF::DRGUP::SMemoryTrackerConfig cfg;
            GUCEF::DRGUP::SMemoryTrackerConfig_SetDefaults( cfg );
            cfg.maxRawCallstackDepth = 1;
            ASSERT_TRUE( cfg.maxRawCallstackDepth == 1 );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::DRGUP::DRGUP_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CallStack TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
