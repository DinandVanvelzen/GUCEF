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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#define GUCEF_TESTFW_IMPL
#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTCALLSTACK_H
#include "TestCallStack.h"
#define GUCEF_MLF_TESTAPP_TESTCALLSTACK_H
#endif /* GUCEF_MLF_TESTAPP_TESTCALLSTACK_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTCONFIG_H
#include "TestConfig.h"
#define GUCEF_MLF_TESTAPP_TESTCONFIG_H
#endif /* GUCEF_MLF_TESTAPP_TESTCONFIG_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTLOCKTRACER_H
#include "TestLockTracer.h"
#define GUCEF_MLF_TESTAPP_TESTLOCKTRACER_H
#endif /* GUCEF_MLF_TESTAPP_TESTLOCKTRACER_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTMEMORYTRACKING_H
#include "TestMemoryTracking.h"
#define GUCEF_MLF_TESTAPP_TESTMEMORYTRACKING_H
#endif /* GUCEF_MLF_TESTAPP_TESTMEMORYTRACKING_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTOLEAPI_H
#include "TestOleApi.h"
#define GUCEF_MLF_TESTAPP_TESTOLEAPI_H
#endif /* GUCEF_MLF_TESTAPP_TESTOLEAPI_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTREPORTER_H
#include "TestReporter.h"
#define GUCEF_MLF_TESTAPP_TESTREPORTER_H
#endif /* GUCEF_MLF_TESTAPP_TESTREPORTER_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTSENTINELS_H
#include "TestSentinels.h"
#define GUCEF_MLF_TESTAPP_TESTSENTINELS_H
#endif /* GUCEF_MLF_TESTAPP_TESTSENTINELS_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTSTATISTICS_H
#include "TestStatistics.h"
#define GUCEF_MLF_TESTAPP_TESTSTATISTICS_H
#endif /* GUCEF_MLF_TESTAPP_TESTSTATISTICS_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTTRACKERLIFECYCLE_H
#include "TestTrackerLifecycle.h"
#define GUCEF_MLF_TESTAPP_TESTTRACKERLIFECYCLE_H
#endif /* GUCEF_MLF_TESTAPP_TESTTRACKERLIFECYCLE_H ? */

#ifndef GUCEF_MLF_TESTAPP_TESTVALIDATION_H
#include "TestValidation.h"
#define GUCEF_MLF_TESTAPP_TESTVALIDATION_H
#endif /* GUCEF_MLF_TESTAPP_TESTVALIDATION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "MemoryLeakFinder_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );

        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        GUCEF_TESTFW_INIT( "MemoryLeakFinder_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::CORE::TestFrameworkLogCallback, GUCEF_NULL );

        PerformTrackerLifecycleTests();
        PerformConfigTests();
        PerformMemoryTrackingTests();
        PerformSentinelTests();
        PerformStatisticsTests();
        PerformValidationTests();
        PerformReporterTests();
        PerformCallStackTests();
        PerformLockTracerTests();
        { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "MAIN: after PerformLockTracerTests\n" ); fclose( dbg ); } }
        PerformOleApiTests();
        { FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" ); if ( dbg ) { fprintf( dbg, "MAIN: after PerformOleApiTests\n" ); fclose( dbg ); } }

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "MemoryLeakFinder_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
