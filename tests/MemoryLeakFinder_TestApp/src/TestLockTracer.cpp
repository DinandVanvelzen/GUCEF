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

#ifndef GUCEF_MLF_MEMORYMANAGER_H
#include "gucefMLF_MemoryManager.h"
#define GUCEF_MLF_MEMORYMANAGER_H
#endif /* GUCEF_MLF_MEMORYMANAGER_H ? */

#ifndef GUCEF_LOCKTRACE_H
#include "gucefMLF_locktrace.h"
#define GUCEF_LOCKTRACE_H
#endif /* GUCEF_LOCKTRACE_H ? */

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

    GUCEF::MLF::MEMMAN_Initialize();

    // Test 1: Create/obtain/release/destroy lifecycle does not crash
    GUCEF_TESTFW_TESTCASE( "Test 1: Lock create/obtain/release/destroy no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Lock create/obtain/release/destroy no crash" );
            void* lockId = reinterpret_cast< void* >( static_cast< uintptr_t >( 0x1234ABCD ) );
            GUCEF::MLF::MEMMAN_ExclusiveLockCreated( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockObtained( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockReleased( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockDestroy( lockId );
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
            GUCEF::MLF::MEMMAN_ExclusiveLockCreated( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockObtained( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockObtained( lockId );  /* re-entrant same thread */
            GUCEF::MLF::MEMMAN_ExclusiveLockReleased( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockReleased( lockId );
            GUCEF::MLF::MEMMAN_ExclusiveLockDestroy( lockId );
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
            GUCEF::MLF::MEMMAN_ExclusiveLockCreated( lockA );
            GUCEF::MLF::MEMMAN_ExclusiveLockCreated( lockB );
            GUCEF::MLF::MEMMAN_ExclusiveLockObtained( lockA );
            GUCEF::MLF::MEMMAN_ExclusiveLockObtained( lockB );
            GUCEF::MLF::MEMMAN_ExclusiveLockReleased( lockA );
            GUCEF::MLF::MEMMAN_ExclusiveLockReleased( lockB );
            GUCEF::MLF::MEMMAN_ExclusiveLockDestroy( lockA );
            GUCEF::MLF::MEMMAN_ExclusiveLockDestroy( lockB );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::MLF::MEMMAN_Shutdown();

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after MEMMAN_Shutdown\n" ); fclose( dbg ); } }

    CORE::CLogStreamScope::FlushLogs();

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after FlushLogs\n" ); fclose( dbg ); } }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL LockTracer TESTS COMPLETED" );

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "LOCKTRACERTEST: after GUCEF_LOG\n" ); fclose( dbg ); } }
}

/*-------------------------------------------------------------------------*/
