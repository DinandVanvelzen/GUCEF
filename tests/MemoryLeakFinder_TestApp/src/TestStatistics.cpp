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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestStatistics.h"

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
PerformStatisticsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Statistics TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Statistics" );

    /*
     * Note: The internal statistics API (CMemoryTracker::GetTotalMemoryAllocations etc.)
     * is on a GUCEF_HIDDEN class and therefore not exported from the DLL.
     * These tests verify statistical behaviour via the observable C API:
     * allocations that succeed return non-null, deallocations do not crash,
     * and the module handles multiple concurrent live allocations correctly.
     */

    GUCEF::MLF::MEMMAN_Initialize();

    // Test 1: Single allocation returns non-null (tracked)
    GUCEF_TESTFW_TESTCASE( "Test 1: Single allocation is tracked" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Single allocation is tracked" );
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 16, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Multiple simultaneous live allocations succeed
    GUCEF_TESTFW_TESTCASE( "Test 2: Multiple simultaneous live allocations" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Multiple simultaneous live allocations" );
            void* ptr1 = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 16, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            void* ptr2 = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            void* ptr3 = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr1 != GUCEF_NULL );
            ASSERT_TRUE( ptr2 != GUCEF_NULL );
            ASSERT_TRUE( ptr3 != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr1, MM_FREE, GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr2, MM_FREE, GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr3, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Repeated alloc/dealloc cycles do not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: Repeated alloc/dealloc cycles" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Repeated alloc/dealloc cycles" );
            for ( int i = 0; i < 10; ++i )
            {
                void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, static_cast< size_t >( ( i + 1 ) * 8 ), MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
                ASSERT_TRUE( ptr != GUCEF_NULL );
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Mixed allocation types each return distinct non-null pointers
    GUCEF_TESTFW_TESTCASE( "Test 4: Mixed allocation types return distinct pointers" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Mixed allocation types return distinct pointers" );
            void* pMalloc  = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 24, MM_MALLOC,    GUCEF_NULL, GUCEF_NULL );
            void* pNew     = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 24, MM_NEW,       GUCEF_NULL, "T" );
            void* pNewArr  = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 24, MM_NEW_ARRAY, GUCEF_NULL, "T[]" );
            ASSERT_TRUE( pMalloc  != GUCEF_NULL );
            ASSERT_TRUE( pNew     != GUCEF_NULL );
            ASSERT_TRUE( pNewArr  != GUCEF_NULL );
            ASSERT_TRUE( pMalloc  != pNew );
            ASSERT_TRUE( pMalloc  != pNewArr );
            ASSERT_TRUE( pNew     != pNewArr );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( pMalloc,  MM_FREE,         GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( pNew,     MM_DELETE,       "T" );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( pNewArr,  MM_DELETE_ARRAY, "T[]" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Large allocation and deallocation does not crash
    GUCEF_TESTFW_TESTCASE( "Test 5: Large allocation and deallocation" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Large allocation and deallocation" );
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 1024 * 1024, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::MLF::MEMMAN_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Statistics TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
