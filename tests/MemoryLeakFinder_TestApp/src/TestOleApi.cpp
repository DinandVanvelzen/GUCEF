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

#include "TestOleApi.h"

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
PerformOleApiTests( void )
{GUCEF_TRACE;

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: function entered\n" ); fclose( dbg ); } }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OleApi TESTS" );

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: after COMMENCING log\n" ); fclose( dbg ); } }

    GUCEF_TESTFW_SUITE_SCOPE( "OleApi" );

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: after SUITE_SCOPE\n" ); fclose( dbg ); } }

#ifdef MEMCHECK_OLEAPI

    GUCEF::DRGUP::DRGUP_Initialize();

    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: after DRGUP_Initialize\n" ); fclose( dbg ); } }

    // Test 1: SysAllocString returns non-null and SysFreeString does not crash
    { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: before Test1 TESTCASE macro\n" ); fclose( dbg ); } }
    GUCEF_TESTFW_TESTCASE( "Test 1: SysAllocString and SysFreeString" )
        { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - CTestCaseGuard ctor done, inside try\n" ); fclose( dbg ); } }
        try
        {
            { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - inside inner try\n" ); fclose( dbg ); } }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: SysAllocString and SysFreeString" );
            { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - after GUCEF_LOG, before SysAllocString\n" ); fclose( dbg ); } }
            wchar_t wideStr[] = L"TestString";
            wchar_t* bstr1 = GUCEF::DRGUP::DRGUP_SysAllocString( __FILE__, __LINE__, wideStr );
            { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - after SysAllocString, bstr1=%p\n", (void*)bstr1 ); fclose( dbg ); } }
            ASSERT_TRUE( bstr1 != GUCEF_NULL );
            { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - after ASSERT_TRUE, before SysFreeString\n" ); fclose( dbg ); } }
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, bstr1 );
            { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "OLEAPI: Test1 - after SysFreeString\n" ); fclose( dbg ); } }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: SysAllocStringByteLen returns non-null and SysFreeString does not crash
    GUCEF_TESTFW_TESTCASE( "Test 2: SysAllocStringByteLen and SysFreeString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: SysAllocStringByteLen and SysFreeString" );
            wchar_t* bstr2 = GUCEF::DRGUP::DRGUP_SysAllocStringByteLen( __FILE__, __LINE__, "hello", 10 );
            ASSERT_TRUE( bstr2 != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, bstr2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: SysAllocStringLen returns non-null and SysFreeString does not crash
    GUCEF_TESTFW_TESTCASE( "Test 3: SysAllocStringLen and SysFreeString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: SysAllocStringLen and SysFreeString" );
            wchar_t wideStr[] = L"TestString";
            wchar_t* bstr3 = GUCEF::DRGUP::DRGUP_SysAllocStringLen( __FILE__, __LINE__, wideStr, 4 );
            ASSERT_TRUE( bstr3 != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, bstr3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: SysReAllocString updates the BSTR in-place, no crash
    GUCEF_TESTFW_TESTCASE( "Test 4: SysReAllocString no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: SysReAllocString no crash" );
            wchar_t wideStr[]  = L"TestString";
            wchar_t wideStr2[] = L"Longer";
            wchar_t* bstr4 = GUCEF::DRGUP::DRGUP_SysAllocString( __FILE__, __LINE__, wideStr );
            ASSERT_TRUE( bstr4 != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_SysReAllocString( __FILE__, __LINE__, &bstr4, wideStr2 );
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, bstr4 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: SysReAllocStringLen updates the BSTR in-place, no crash
    GUCEF_TESTFW_TESTCASE( "Test 5: SysReAllocStringLen no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: SysReAllocStringLen no crash" );
            wchar_t wideStr[]  = L"TestString";
            wchar_t wideStr2[] = L"Longer";
            wchar_t* bstr5 = GUCEF::DRGUP::DRGUP_SysAllocString( __FILE__, __LINE__, wideStr );
            ASSERT_TRUE( bstr5 != GUCEF_NULL );
            GUCEF::DRGUP::DRGUP_SysReAllocStringLen( __FILE__, __LINE__, &bstr5, wideStr2, 3 );
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, bstr5 );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: SysFreeString(NULL) does not crash
    GUCEF_TESTFW_TESTCASE( "Test 6: SysFreeString NULL no crash" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: SysFreeString NULL no crash" );
            GUCEF::DRGUP::DRGUP_SysFreeString( __FILE__, __LINE__, GUCEF_NULL );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::DRGUP::DRGUP_Shutdown();

#else

    /* OLE API tracking is disabled in this build — skip all sub-tests */
    GUCEF_TESTFW_TESTCASE( "Test 1: OLE API not compiled in - skipped" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: OLE API not compiled in - skipped" );
            ASSERT_TRUE( true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

#endif /* MEMCHECK_OLEAPI ? */

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL OleApi TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
