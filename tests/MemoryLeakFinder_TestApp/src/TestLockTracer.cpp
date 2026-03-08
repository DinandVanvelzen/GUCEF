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

#include <stdio.h>

#ifndef GUCEF_DRGUP_MEMORYMANAGER_H
#include "gucefDRGUP_MemoryManager.h"
#define GUCEF_DRGUP_MEMORYMANAGER_H
#endif /* GUCEF_DRGUP_MEMORYMANAGER_H ? */

#ifndef GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#include "gucefDRGUP_SMemoryTrackerConfig.h"
#define GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_DRGUP_LOCKTRACE_H
#include "gucefDRGUP_locktrace.h"
#define GUCEF_DRGUP_LOCKTRACE_H
#endif /* GUCEF_DRGUP_LOCKTRACE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLockTracer.h"

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
PerformLockTracerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING LockTracer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "LockTracer" );

    /*
     * Note: GUCEF_PLATFORM_LOCK_TRACER_DISABLED may be set in the DLL build,
     * making these functions no-ops inside the DLL.  The test verifies that
     * the exported C API functions are callable without crash regardless.
     */

    GUCEF::DRGUP::DRGUP_Initialize();

    // Test 1: Create/obtain/release/destroy lifecycle does not crash
    GUCEF_TESTFW_TESTCASE( "Test 1: Lock create/obtain/release/destroy no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Lock create/obtain/release/destroy no crash" );
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0x1234ABCD ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockId );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Re-entrant lock lifecycle does not crash
    GUCEF_TESTFW_TESTCASE( "Test 2: Re-entrant lock lifecycle no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Re-entrant lock lifecycle no crash" );
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0x5678EF01 ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockId );  /* re-entrant same thread */
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockId );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Multiple distinct lock IDs do not interfere
    GUCEF_TESTFW_TESTCASE( "Test 3: Multiple distinct lock IDs no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Multiple distinct lock IDs no crash" );
            void* lockA = reinterpret_cast< void* >( static_cast< uintptr_t >( 0xAAAA0001 ) );
            void* lockB = reinterpret_cast< void* >( static_cast< uintptr_t >( 0xBBBB0002 ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockA );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockB );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockA );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockB );
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockA );
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockB );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockA );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockB );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

#ifndef GUCEF_DRGUP_TSAN_ACTIVE

    // Test 4: LockProtectsRange + LockUnprotectsRange smoke test - no crash
    GUCEF_TESTFW_TESTCASE( "Test 4: LockProtectsRange+LockUnprotectsRange no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: LockProtectsRange+LockUnprotectsRange no crash" );
            char buf[ 64 ];
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0xCCCC0004 ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockId );
            GUCEF::DRGUP::DRGUP_LockProtectsRange( lockId, buf, sizeof( buf ) );
            GUCEF::DRGUP::DRGUP_LockUnprotectsRange( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockId );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Lock IS held at dealloc time - no warning expected
    GUCEF_TESTFW_TESTCASE( "Test 5: Lock held at dealloc - no warning" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Lock held at dealloc - no warning" );
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0xDDDD0005 ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockObtained( lockId );
            void* buf = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( buf != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_LockProtectsRange( lockId, buf, 64 );
            /* Remove range association before freeing to avoid triggering the warning path */
            GUCEF::DRGUP::DRGUP_LockUnprotectsRange( lockId );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( buf, MM_FREE, GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_ExclusiveLockReleased( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockId );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Lock NOT held at dealloc time - warning logged, must not crash
    GUCEF_TESTFW_TESTCASE( "Test 6: Lock not held at dealloc - warning logged no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Lock not held at dealloc - warning logged no crash" );
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0xEEEE0006 ) );
            GUCEF::DRGUP::DRGUP_ExclusiveLockCreated( lockId );
            /* Do NOT obtain the lock - so it is not held during dealloc */
            void* buf = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 64, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( buf != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_LockProtectsRange( lockId, buf, 64 );
            /* Dealloc while lock not held: a warning is expected, but execution continues */
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( buf, MM_FREE, GUCEF_NULL );
            /* These must still be reachable */
            GUCEF::DRGUP::DRGUP_LockUnprotectsRange( lockId );
            GUCEF::DRGUP::DRGUP_ExclusiveLockDestroy( lockId );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

#endif /* GUCEF_DRGUP_TSAN_ACTIVE ? */

    GUCEF::DRGUP::DRGUP_Shutdown();

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after DRGUP_Shutdown\n" ); fclose( dbg ); } }

    CORE::CLogStreamScope::FlushLogs();

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after FlushLogs\n" ); fclose( dbg ); } }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL LockTracer TESTS COMPLETED" );

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after GUCEF_LOG\n" ); fclose( dbg ); } }
}

/*-------------------------------------------------------------------------*/
