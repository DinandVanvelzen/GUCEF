/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <stdlib.h>

#ifdef GUCEF_LINUX_BUILD
#include <unistd.h>
#endif /* GUCEF_LINUX_BUILD ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGGINGGLOBAL_H
#include "gucefCORE_CLoggingGlobal.h"
#define GUCEF_CORE_CLOGGINGGLOBAL_H
#endif /* GUCEF_CORE_CLOGGINGGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestCoreGlobalTeardown.h"

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

namespace GUCEF {
namespace CORE {

/** The test seam of CLoggingGlobal: how many logging globals the process made and whether one exists */
class CLoggingGlobalTestAccess
{
    public:

    static UInt32 GetInstanceCreationCount( void )
    {
        return CLoggingGlobal::GetInstanceCreationCount();
    }

    static bool HasInstance( void )
    {
        return CLoggingGlobal::HasInstance();
    }
};

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

namespace {

/** Exit codes of the child, see ReportLoggingGlobalAfterTeardown() */
const int TeardownExitLoggingGlobalGone = 0;
const int TeardownExitPluginNotLoaded = 2;
const int TeardownExitReportBeforeTeardown = 3;
const int TeardownExitLoggingGlobalMadeAgain = 4;
const int TeardownExitLoggingGlobalNeverMade = 5;

/** Whether the child loaded its plugin: the exit code is that of ReportLoggingGlobalAfterTeardown() either way */
bool g_teardownPluginLoaded = false;

/**
 *  Registered with atexit() before the global objects of gucefCORE exist, so it runs after their teardown (atexit() is
 *  first in, last out): tells by the exit code whether the logging global was made once and is gone. _exit() so that
 *  nothing after it changes the exit code
 */
void
ReportLoggingGlobalAfterTeardown( void )
{
    const UInt32 creations = CORE::CLoggingGlobalTestAccess::GetInstanceCreationCount();
    const bool exists = CORE::CLoggingGlobalTestAccess::HasInstance();

    int exitCode = TeardownExitLoggingGlobalGone;
    if ( !g_teardownPluginLoaded )
        exitCode = TeardownExitPluginNotLoaded;
    else
    if ( 0 == creations )
        exitCode = TeardownExitLoggingGlobalNeverMade;
    else
    if ( creations > 1 )
        exitCode = TeardownExitLoggingGlobalMadeAgain;
    else
    if ( exists )
        exitCode = TeardownExitReportBeforeTeardown;
    _exit( exitCode );
}

}

/*-------------------------------------------------------------------------*/

int
RunCoreGlobalTeardown( void )
{GUCEF_TRACE;

    atexit( ReportLoggingGlobalAfterTeardown );

    // the global objects of gucefCORE and the logging global, with their teardown at exit
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    // a plugin is loaded and unloaded: the plugin control logs as it does, and again when it goes at exit
    if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER_d", "test", true ) &&
         !pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER", "test", true ) )
        return TeardownExitPluginNotLoaded;
    g_teardownPluginLoaded = true;
    pluginControl.UnloadAll();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CoreGlobalTeardown: a plugin was loaded and unloaded, exiting" );
    return TeardownExitLoggingGlobalGone;
}

/*-------------------------------------------------------------------------*/

void
PerformCoreGlobalTeardownTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING gucefCORE global teardown TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CoreGlobalTeardown" );

    #ifdef GUCEF_LINUX_BUILD

    // The teardown is that of the whole process, so it runs in a child process of its own
    GUCEF_TESTFW_TESTCASE( "An application that loads and unloads a plugin and exits makes the logging global once and has none after the teardown" )
        try
        {
            char exePath[ 4096 ];
            ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
            ASSERT_TRUE( exePathLength > 0 );
            if ( exePathLength > 0 )
            {
                exePath[ exePathLength ] = '\0';

                CORE::CProcess::TStringVector args;
                args.push_back( GUCEF_CORE_TESTAPP_CORE_GLOBAL_TEARDOWN_ARG );
                CORE::CProcess child;
                child.SetIsolateFromParent( true );
                ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );

                Int32 exitCode = -1;
                const bool ended = child.WaitForExit( 120000, exitCode );
                if ( !ended )
                    child.Terminate( false );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CoreGlobalTeardown: the child exited with " << exitCode << " (0 the logging global was made once and is gone, "
                    << TeardownExitLoggingGlobalMadeAgain << " it was made again during the teardown, " << TeardownExitReportBeforeTeardown
                    << " the report ran before the teardown, " << TeardownExitPluginNotLoaded << " the plugin was not loaded)" );
                ASSERT_TRUE( ended );
                ASSERT_TRUE( TeardownExitLoggingGlobalGone == exitCode );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/
