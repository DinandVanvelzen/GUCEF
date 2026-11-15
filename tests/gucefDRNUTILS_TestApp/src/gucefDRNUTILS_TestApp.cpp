/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for the gucefDRNUTILS module
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#define GUCEF_TESTFW_IMPL

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSCHEDULERHOST_H
#include "TestDrnUtilsSchedulerHost.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSCHEDULERHOST_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSCHEDULERHOST_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTASKMANAGERHOST_H
#include "TestDrnUtilsTaskManagerHost.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTASKMANAGERHOST_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTASKMANAGERHOST_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTABLES_H
#include "TestDrnUtilsTables.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTABLES_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSTABLES_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H
#include "TestDrnUtilsDrn.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H
#include "TestDrnUtilsSoak.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H ? */

#ifndef GUCEF_TESTDRNUTILSSKELETON_H
#include "TestDrnUtilsSkeleton.h"
#define GUCEF_TESTDRNUTILSSKELETON_H
#endif /* GUCEF_TESTDRNUTILSSKELETON_H ? */

#ifndef GUCEF_TESTDRNUTILSREMOTEKIT_H
#include "TestDrnUtilsRemoteKit.h"
#define GUCEF_TESTDRNUTILSREMOTEKIT_H
#endif /* GUCEF_TESTDRNUTILSREMOTEKIT_H ? */

#ifndef GUCEF_TESTDRNUTILSREMOTESERVICE_H
#include "TestDrnUtilsRemoteService.h"
#define GUCEF_TESTDRNUTILSREMOTESERVICE_H
#endif /* GUCEF_TESTDRNUTILSREMOTESERVICE_H ? */

#ifndef GUCEF_TESTDRNUTILSLANDISCOVERY_H
#include "TestDrnUtilsLanDiscovery.h"
#define GUCEF_TESTDRNUTILSLANDISCOVERY_H
#endif /* GUCEF_TESTDRNUTILSLANDISCOVERY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Logs the events of the test framework and flushes the log, so that the log shows how far a run got */
static void
DrnUtilsTestEventCallback( GUCEF::TESTFW::ETestEvent eventType ,
                           const char* suiteName               ,
                           const char* caseName                ,
                           const char* message                 ,
                           const char* file                    ,
                           GUCEF::Int32 line                   ,
                           GUCEF::UInt32 failureCount          ,
                           void* userData                      )
{GUCEF_TRACE;

    GUCEF::TESTFW::TestFrameworkLogCallback( eventType, suiteName, caseName, message, file, line, failureCount, userData );
    GUCEF::CORE::CLogStreamScope::FlushLogs();
}

/**
 *  Whether a suite runs: GUCEF_DRNUTILS_TEST_SUITES, a comma separated list of suite names (Skeleton, SchedulerHost,
 *  TaskManagerHost, Tables, Drn, RemoteKit, RemoteService, Soak), selects suites; unset or empty runs every suite
 */
static bool
IsSuiteSelected( const char* suiteName )
{GUCEF_TRACE;

    const char* selection = getenv( "GUCEF_DRNUTILS_TEST_SUITES" );
    if ( GUCEF_NULL == selection || '\0' == *selection )
        return true;
    GUCEF::CORE::CString::StringVector names = GUCEF::CORE::CString( selection ).ParseElements( ',', false );
    for ( GUCEF::UInt32 i=0; i<names.size(); ++i )
    {
        if ( names[ i ].Trim( true ).Trim( false ).Equals( suiteName, false ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        // the child process of RU-T12 (the test application starts itself as a service hosting a scheduler over drn)
        const char* childPort = getenv( "GUCEF_DRNUTILS_TEST_CHILD" );
        bool isChild = GUCEF_NULL != childPort && '\0' != *childPort;

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, isChild ? "gucefDRNUTILS_TestApp_Child_Log.txt" : "gucefDRNUTILS_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );
        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the module, as every application using gucefDRNUTILS has to do
        DRNUTILS::CDrnUtilsGlobal::Instance();

        if ( isChild )
        {
            int childExitCode = RunDrnUtilsChildHost( childPort );
            CORE::CLogStreamScope::FlushLogs();
            CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &logger );
            if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
                CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
            return childExitCode;
        }

        GUCEF_TESTFW_INIT( "gucefDRNUTILS_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( DrnUtilsTestEventCallback, GUCEF_NULL );

        // GUCEF_DRNUTILS_TEST_SOAK_ONLY runs the soak suite alone (long soak runs)
        const char* soakOnly = getenv( "GUCEF_DRNUTILS_TEST_SOAK_ONLY" );
        if ( GUCEF_NULL == soakOnly || '\0' == *soakOnly )
        {
            if ( IsSuiteSelected( "Skeleton" ) )
                PerformDrnUtilsSkeletonTests();
            if ( IsSuiteSelected( "SchedulerHost" ) )
                PerformDrnUtilsSchedulerHostTests();
            if ( IsSuiteSelected( "TaskManagerHost" ) )
                PerformDrnUtilsTaskManagerHostTests();
            if ( IsSuiteSelected( "Tables" ) )
                PerformDrnUtilsTablesTests();
            if ( IsSuiteSelected( "Drn" ) )
                PerformDrnUtilsDrnTests( argc > 0 && GUCEF_NULL != argv[ 0 ] && GUCEF_NULL != strchr( argv[ 0 ], '/' ) ? argv[ 0 ] : "" );
            if ( IsSuiteSelected( "RemoteKit" ) )
                PerformDrnUtilsRemoteKitTests();
            if ( IsSuiteSelected( "RemoteService" ) )
                PerformDrnUtilsRemoteServiceTests();
            if ( IsSuiteSelected( "LanDiscovery" ) )
                PerformDrnUtilsLanDiscoveryTests();
        }
        if ( IsSuiteSelected( "Soak" ) )
            PerformDrnUtilsSoakTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefDRNUTILS_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );

        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &logger );
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
