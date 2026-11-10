/*
 *  gucefPATCHERAPP_TestApp: Application for testing gucefPATCHERAPP
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_PATCHER_H
#include "gucefPATCHER.h"
#define GUCEF_PATCHER_H
#endif /* GUCEF_PATCHER_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
#ifndef GUCEF_LOADER_H
#include "gucefLOADER.h"
#define GUCEF_LOADER_H
#endif /* GUCEF_LOADER_H ? */
#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

#ifndef GUCEF_PATCHERAPP_TESTAPP_TESTPATCHENGINE_H
#include "TestPatchEngine.h"
#define GUCEF_PATCHERAPP_TESTAPP_TESTPATCHENGINE_H
#endif /* GUCEF_PATCHERAPP_TESTAPP_TESTPATCHENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/**
 *  The original scenario: load and run the patcher application with test data. Not part of the automated
 *  test run, start the application with -runapp to use it.
 */
static int
RunPatcherApp( void )
{GUCEF_TRACE;

    // the test dir path is based on a CMake win32 build
    LOADER::Int32 appReturnCode = LOADER::LoadAndRunGucefPlatformApp( "gucefPATCHERAPP", "$MODULEDIR$/../../../TestData/gucefPATCHERAPP_TestApp/mswin", 0, 0, 0, 0, 0, 0 );
    return appReturnCode;
}

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        for ( int i=1; i<argc; ++i )
        {
            if ( 0 == strcmp( argv[ i ], "-runapp" ) )
                return RunPatcherApp();
        }
        #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefPATCHERAPP_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefPATCHERAPP_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        PerformPatchEngineTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefPATCHERAPP_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 10-12-2006
        - Dinand: Added this comment section
- 2026
        - Ported to the reporting test framework, with patch engine tests

---------------------------------------------------------------------------*/
