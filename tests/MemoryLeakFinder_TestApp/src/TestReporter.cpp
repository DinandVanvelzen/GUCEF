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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestReporter.h"

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
PerformReporterTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Reporter TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Reporter" );

    GUCEF::DRGUP::DRGUP_Initialize();
    GUCEF::DRGUP::DRGUP_SetLogFile( "MemoryLeakFinder_TestApp_MLFLog.txt" );

    // Test 1: DumpMemoryAllocations with no live allocations does not crash
    GUCEF_TESTFW_TESTCASE( "Test 1: DumpMemoryAllocations with no leaks no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: DumpMemoryAllocations with no leaks no crash" );
            GUCEF::DRGUP::DRGUP_DumpMemoryAllocations();
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: DumpLogReport with no live allocations does not crash
    GUCEF_TESTFW_TESTCASE( "Test 2: DumpLogReport with no leaks no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: DumpLogReport with no leaks no crash" );
            GUCEF::DRGUP::DRGUP_DumpLogReport();
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: DumpMemoryAllocations with a live (leaked) allocation does not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: DumpMemoryAllocations with live allocation no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: DumpMemoryAllocations with live allocation no crash" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            /* Intentionally not freed yet to simulate a live allocation during dump */
            GUCEF::DRGUP::DRGUP_DumpMemoryAllocations();
            /* Clean up */
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: DumpLogReport with a live (leaked) allocation produces output, no crash
    GUCEF_TESTFW_TESTCASE( "Test 4: DumpLogReport with live allocation no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: DumpLogReport with live allocation no crash" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 48, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DumpLogReport();
            /* Clean up */
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: DRGUP_DumpTimeline writes an output file
    GUCEF_TESTFW_TESTCASE( "Test 5: DumpTimeline creates output file" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: DumpTimeline creates output file" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DumpTimeline( "MemoryLeakFinder_TestApp_Timeline.tsv" );
            FILE* f = fopen( "MemoryLeakFinder_TestApp_Timeline.tsv", "r" );
            ASSERT_TRUE( f != GUCEF_NULL );
            if ( f != GUCEF_NULL ) { fclose( f ); }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: DRGUP_DumpMassifFormat writes an output file
    GUCEF_TESTFW_TESTCASE( "Test 6: DumpMassifFormat creates output file" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: DumpMassifFormat creates output file" );
            void* ptr = GUCEF::DRGUP::DRGUP_AllocateMemory( __FILE__, __LINE__, 32, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( ptr != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DeAllocateMemory( ptr, MM_FREE, GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_DumpMassifFormat( "MemoryLeakFinder_TestApp_Massif.ms" );
            FILE* f = fopen( "MemoryLeakFinder_TestApp_Massif.ms", "r" );
            ASSERT_TRUE( f != GUCEF_NULL );
            if ( f != GUCEF_NULL ) { fclose( f ); }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::DRGUP::DRGUP_Shutdown();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Reporter TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
