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

#ifndef GUCEF_MLF_CALLOCATIONRECORD_H
#include "gucefMLF_CAllocationRecord.h"
#define GUCEF_MLF_CALLOCATIONRECORD_H
#endif /* GUCEF_MLF_CALLOCATIONRECORD_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSentinels.h"

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
PerformSentinelTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Sentinels TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Sentinels" );

    GUCEF::MLF::MEMMAN_Initialize();
    /* Use 4-byte padding (1 long) for predictable sentinel layout */
    GUCEF::MLF::MEMMAN_SetPaddingSize( 4 );

    // Test 1: Pre-guard sentinel bytes have expected value
    GUCEF_TESTFW_TESTCASE( "Test 1: Pre-guard sentinel has correct value" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Pre-guard sentinel has correct value" );
            const size_t userSize = 16;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, userSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                /* Padding is sizeof(long) bytes before the user pointer */
                const long* preguard = reinterpret_cast< const long* >(
                    reinterpret_cast< const char* >( ptr ) - sizeof( long ) );
                ASSERT_TRUE( *preguard == GUCEF::MLF::ALLOC_PADDING_SENTINEL );
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Post-guard sentinel bytes have expected value
    GUCEF_TESTFW_TESTCASE( "Test 2: Post-guard sentinel has correct value" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Post-guard sentinel has correct value" );
            const size_t userSize = 16;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, userSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                const long* postguard = reinterpret_cast< const long* >(
                    reinterpret_cast< const char* >( ptr ) + userSize );
                ASSERT_TRUE( *postguard == GUCEF::MLF::ALLOC_PADDING_SENTINEL );
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: ValidateKnownAllocBlock on a clean block reports no violations
    GUCEF_TESTFW_TESTCASE( "Test 3: ValidateKnownAllocBlock on clean block" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: ValidateKnownAllocBlock on clean block" );
            const size_t userSize = 16;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, userSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                /* Should complete without crash when sentinels are intact */
                GUCEF::MLF::MEMMAN_ValidateKnownAllocBlock( ptr, static_cast< GUCEF::MLF::UInt32 >( userSize ), __FILE__, __LINE__ );
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Corrupt pre-guard and validate detects it (logs, no crash)
    GUCEF_TESTFW_TESTCASE( "Test 4: Corrupted pre-guard detected without crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Corrupted pre-guard detected without crash" );
            const size_t userSize = 16;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, userSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                /* Corrupt the pre-guard sentinel */
                long* preguard = reinterpret_cast< long* >(
                    reinterpret_cast< char* >( ptr ) - sizeof( long ) );
                *preguard = 0xDEADBEEFl;

                /* Validate should detect the violation (logged) without crashing */
                GUCEF::MLF::MEMMAN_ValidateKnownAllocBlock( ptr, static_cast< GUCEF::MLF::UInt32 >( userSize ), __FILE__, __LINE__ );

                /* Restore sentinel before freeing to avoid double-error in dealloc path */
                *preguard = GUCEF::MLF::ALLOC_PADDING_SENTINEL;
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Corrupt post-guard and validate detects it (logs, no crash)
    GUCEF_TESTFW_TESTCASE( "Test 5: Corrupted post-guard detected without crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Corrupted post-guard detected without crash" );
            const size_t userSize = 16;
            void* ptr = GUCEF::MLF::MEMMAN_AllocateMemory( __FILE__, __LINE__, userSize, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            if ( ptr != GUCEF_NULL )
            {
                /* Corrupt the post-guard sentinel */
                long* postguard = reinterpret_cast< long* >(
                    reinterpret_cast< char* >( ptr ) + userSize );
                *postguard = 0xDEADBEEFl;

                /* Validate should detect the violation (logged) without crashing */
                GUCEF::MLF::MEMMAN_ValidateKnownAllocBlock( ptr, static_cast< GUCEF::MLF::UInt32 >( userSize ), __FILE__, __LINE__ );

                /* Restore sentinel before freeing to avoid double-error in dealloc path */
                *postguard = GUCEF::MLF::ALLOC_PADDING_SENTINEL;
                GUCEF::MLF::MEMMAN_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            }
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::MLF::MEMMAN_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Sentinels TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
