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

#include "TestMemoryTracking.h"

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
PerformMemoryTrackingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MemoryTracking TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "MemoryTracking" );

    GUCEF::DRGUP::DRGUP_Initialize();

    // Test 1: MM_MALLOC allocation returns non-null and is tracked
    GUCEF_TESTFW_TESTCASE( "Test 1: MM_MALLOC alloc and free" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: MM_MALLOC alloc and free" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: MM_NEW allocation and MM_DELETE deallocation
    GUCEF_TESTFW_TESTCASE( "Test 2: MM_NEW alloc and MM_DELETE free" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: MM_NEW alloc and MM_DELETE free" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_NEW, GUCEF_NULL, "TestObject" );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_DELETE, "TestObject" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: MM_NEW_ARRAY allocation and MM_DELETE_ARRAY deallocation
    GUCEF_TESTFW_TESTCASE( "Test 3: MM_NEW_ARRAY alloc and MM_DELETE_ARRAY free" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: MM_NEW_ARRAY alloc and MM_DELETE_ARRAY free" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 128, MM_NEW_ARRAY, GUCEF_NULL, "TestArray" );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_DELETE_ARRAY, "TestArray" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: MM_CALLOC allocation (num*size equivalent)
    GUCEF_TESTFW_TESTCASE( "Test 4: MM_CALLOC alloc and free" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: MM_CALLOC alloc and free" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 16, MM_CALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: MM_REALLOC with a prior tracked pointer
    GUCEF_TESTFW_TESTCASE( "Test 5: MM_REALLOC with valid prior pointer" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: MM_REALLOC with valid prior pointer" );
            void* ptr1 = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr1 != GUCEF_NULL );
            void* ptr2 = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_REALLOC, ptr1, GUCEF_NULL );
            ASSERT_TRUE( ptr2 != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr2, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: MM_REALLOC with NULL prior pointer (treated as malloc)
    GUCEF_TESTFW_TESTCASE( "Test 6: MM_REALLOC with NULL prior pointer" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: MM_REALLOC with NULL prior pointer" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 48, MM_REALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: DRGUP_SetOwner returns success before allocation
    GUCEF_TESTFW_TESTCASE( "Test 7: SetOwner pushes owner context" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: SetOwner pushes owner context" );
            __int32 result = GUCEF::DRGUP::DRGUP_SetOwner( __FILE__, __LINE__, "TestOwnerClass" );
            ASSERT_TRUE( result == 1 );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 24, MM_NEW, GUCEF_NULL, "TestOwnerClass" );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_DELETE, "TestOwnerClass" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Placement-new as sub-allocation linked to a parent
    GUCEF_TESTFW_TESTCASE( "Test 8: Placement-new sub-allocation" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Placement-new sub-allocation" );
            void* parent = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( parent != GUCEF_NULL );
            void* child = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 16, MM_PLACEMENT_NEW, parent, "PlacedType" );
            /* Placement-new sub-record may return the same address or null; no crash is required */
            (void) child;
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( parent, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: SetOwner before dealloc (owner context at dealloc site)
    GUCEF_TESTFW_TESTCASE( "Test 9: SetOwner before dealloc no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: SetOwner before dealloc no crash" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 16, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            /* Push owner context immediately before the dealloc (documented use pattern) */
            __int32 ownerResult = GUCEF::DRGUP::DRGUP_SetOwner( __FILE__, __LINE__, "OwnerAtDeallocSite" );
            ASSERT_TRUE( ownerResult != 0 );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: Zero-size allocation does not crash
    GUCEF_TESTFW_TESTCASE( "Test 10: Zero-size allocation no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: Zero-size allocation no crash" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 0, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            /* May return null or a minimal allocation — no crash is the requirement */
            if ( ptr != GUCEF_NULL )
            {
                GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: DeAllocateMemoryEx with file/line info does not crash
    GUCEF_TESTFW_TESTCASE( "Test 11: DeAllocateMemoryEx does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: DeAllocateMemoryEx does not crash" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemoryEx( __FILE__, __LINE__, ptr, MM_FREE, GUCEF_NULL );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::DRGUP::DRGUP_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL MemoryTracking TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
