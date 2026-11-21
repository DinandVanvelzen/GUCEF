/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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

/* the test framework implementation is compiled as part of this file */
#define GUCEF_TESTFW_IMPL

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

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "TestDRNTestUtils.h"
#include "TestDrnBridgeConformance.h"
#include "TestDrnBridgeBehaviour.h"
#include "TestDrnBridgeLimits.h"
#include "TestDrnBridgeMounting.h"
#include "TestDrnBridgeRemote.h"
#include "TestDrnBridgeRemoteCases.h"
#include "TestDrnBridgeBaseline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Logs the events of the test framework and flushes the log, so that the log shows how far a run got */
static void
DrnBridgeTestEventCallback( GUCEF::TESTFW::ETestEvent eventType ,
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

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "vfspluginDRN_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the modules, as every application using them has to do
        VFS::CVfsGlobal::Instance();
        DRN::CDRNGlobal::Instance();
        if ( !DRNTEST::EnsureCryptoCodecPluginLoaded() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "vfspluginDRN_TestApp: codecspluginMONOCYPHER is not available, encrypted links cannot be made" );

        GUCEF_TESTFW_INIT( "vfspluginDRN_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( DrnBridgeTestEventCallback, GUCEF_NULL );

        // the suites to run can be named on the command line: conformance, conformance-cache, conformance-nocache, conformance-pumped, behaviour, limits, mounting, remote, remote-loopback, remote-proxied, remotecases (default all), baseline (measurements, only when named)
        bool runAll = argc < 2;
        bool runConformanceCache = runAll, runConformanceNoCache = runAll, runConformancePumped = runAll, runBehaviour = runAll, runLimits = runAll, runMounting = runAll;
        bool runRemoteLoopback = runAll, runRemoteProxied = runAll, runRemoteCases = runAll;
        bool runBaseline = false;
        for ( int i=1; i<argc; ++i )
        {
            CORE::CString arg = argv[ i ];
            runConformanceCache = runConformanceCache || arg == "conformance" || arg == "conformance-cache";
            runConformanceNoCache = runConformanceNoCache || arg == "conformance" || arg == "conformance-nocache";
            runConformancePumped = runConformancePumped || arg == "conformance" || arg == "conformance-pumped";
            runBehaviour = runBehaviour || arg == "behaviour";
            runLimits = runLimits || arg == "limits";
            runMounting = runMounting || arg == "mounting";
            runRemoteLoopback = runRemoteLoopback || arg == "remote" || arg == "remote-loopback";
            runRemoteProxied = runRemoteProxied || arg == "remote" || arg == "remote-proxied";
            runRemoteCases = runRemoteCases || arg == "remotecases";
            runBaseline = runBaseline || arg == "baseline";
        }

        if ( runConformanceCache || runConformanceNoCache || runConformancePumped )
            PerformDrnBridgeConformanceTests( runConformanceCache, runConformanceNoCache, runConformancePumped );
        if ( runBehaviour )
            PerformDrnBridgeBehaviourTests();
        if ( runLimits )
            PerformDrnBridgeLimitTests();
        if ( runMounting )
            PerformDrnBridgeMountingTests();
        if ( runRemoteLoopback || runRemoteProxied )
            PerformDrnBridgeRemoteTests( runRemoteLoopback, runRemoteProxied );
        if ( runRemoteCases )
            PerformDrnBridgeRemoteCases();
        if ( runBaseline )
            PerformDrnBridgeBaseline();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "vfspluginDRN_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );

        CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
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
