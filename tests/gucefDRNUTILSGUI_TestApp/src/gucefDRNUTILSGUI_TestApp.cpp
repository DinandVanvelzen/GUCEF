/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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

#ifndef GUCEF_DRNUTILSGUI_H
#include "gucefDRNUTILSGUI.h"
#define GUCEF_DRNUTILSGUI_H
#endif /* GUCEF_DRNUTILSGUI_H ? */

#ifndef GUCEF_TESTDRNUTILSGUISKELETON_H
#include "TestDrnUtilsGuiSkeleton.h"
#define GUCEF_TESTDRNUTILSGUISKELETON_H
#endif /* GUCEF_TESTDRNUTILSGUISKELETON_H ? */

#ifndef GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H
#include "gucefDRNUTILSGUI_TestCommon.h"
#define GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H
#endif /* GUCEF_DRNUTILSGUI_TESTAPP_TESTCOMMON_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Logs the events of the test framework and flushes the log, so that the log shows how far a run got */
static void
DrnUtilsGuiTestEventCallback( GUCEF::TESTFW::ETestEvent eventType ,
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

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefDRNUTILSGUI_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );
        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the module, as every application using gucefDRNUTILSGUI has to do
        DRNUTILSGUI::CDrnUtilsGuiGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefDRNUTILSGUI_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( DrnUtilsGuiTestEventCallback, GUCEF_NULL );

        PerformDrnUtilsGuiSkeletonTests();
        PerformControlConfirmFormTests();
        PerformTimeDisplayTests();
        PerformSchedulerFormTests();
        PerformJobRunHistoryFormTests();
        PerformTaskManagerFormTests();
        PerformEndToEndTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefDRNUTILSGUI_TestApp_Results.xml" );
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
