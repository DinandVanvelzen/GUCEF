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

#include "TestValidation.h"

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
PerformValidationTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Validation TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Validation" );

    GUCEF::MLF::MEMMAN_Initialize();

    // Test 1: ValidateKnownAllocPtr on a tracked pointer does not crash
    GUCEF_TESTFW_TESTCASE( "Test 1: ValidateKnownAllocPtr on tracked ptr" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: ValidateKnownAllocPtr on tracked ptr" );
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidateKnownAllocPtr( ptr, __FILE__, __LINE__ );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: ValidateKnownAllocPtr on an untracked pointer logs warning, no crash
    GUCEF_TESTFW_TESTCASE( "Test 2: ValidateKnownAllocPtr on untracked ptr no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: ValidateKnownAllocPtr on untracked ptr no crash" );
            /* Stack variable — definitely not tracked */
            int stackVar = 42;
            GUCEF::MLF::MEMMAN_ValidateKnownAllocPtr( &stackVar, __FILE__, __LINE__ );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: ValidateKnownAllocBlock on a tracked block does not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: ValidateKnownAllocBlock on tracked block" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: ValidateKnownAllocBlock on tracked block" );
            const GUCEF::MLF::UInt32 blockSize = 64;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, blockSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidateKnownAllocBlock( ptr, blockSize, __FILE__, __LINE__ );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: ValidateChunk does not crash on a tracked block
    GUCEF_TESTFW_TESTCASE( "Test 4: ValidateChunk does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: ValidateChunk does not crash" );
            const GUCEF::MLF::UInt32 blockSize = 64;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, blockSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidateChunk( ptr, ptr, blockSize, __FILE__, __LINE__ );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: ValidateAccessibility does not crash on a tracked block
    GUCEF_TESTFW_TESTCASE( "Test 5: ValidateAccessibility does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: ValidateAccessibility does not crash" );
            const GUCEF::MLF::UInt32 blockSize = 64;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, blockSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidateAccessibility( ptr, blockSize, __FILE__, __LINE__ );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: ValidatePendingDestructor does not crash
    GUCEF_TESTFW_TESTCASE( "Test 6: ValidatePendingDestructor does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: ValidatePendingDestructor does not crash" );
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 32, MM_NEW, GUCEF_NULL, "TestType" );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidatePendingDestructor( __FILE__, __LINE__, ptr, 32, "TestType" );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_DELETE, "TestType" );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: ValidateFinishedDestructor does not crash
    GUCEF_TESTFW_TESTCASE( "Test 7: ValidateFinishedDestructor does not crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: ValidateFinishedDestructor does not crash" );
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, 32, MM_NEW, GUCEF_NULL, "TestType" );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::MLF::MEMMAN_ValidateFinishedDestructor( __FILE__, __LINE__, ptr, 32, "TestType" );
            GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_DELETE, "TestType" );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::MLF::MEMMAN_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Validation TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
