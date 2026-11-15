/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#include "TestComputeSupport.h"
#include "TestComputeModule.h"
#include "TestComputeTypes.h"
#include "TestCpuBackend.h"
#include "TestCpuParallelFor.h"
#include "TestKernelRegistry.h"
#include "TestBackendAvailability.h"
#include "TestDeviceLoss.h"
#include "TestQueueContract.h"
#include "TestMathBridge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  GUCEF_TEST_COMPUTE_SUITES names the suites to run, separated by commas (for example MathBridge), empty for all: the
 *  repeated runs of the sanitizer builds run one suite at a time
 */
static bool
IsSuiteWanted( const ::std::string& filter, const char* name )
{
    return filter.empty() || ::std::string::npos != ( ::std::string( "," ) + filter + "," ).find( ::std::string( "," ) + name + "," );
}

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefCOMPUTE_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefCOMPUTE_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefCOMPUTE_TestApp_Results.xml" );

        const ::std::string suites = GUCEF_TESTFW_GETENV( "GUCEF_TEST_COMPUTE_SUITES" );
        if ( IsSuiteWanted( suites, "Module" ) )
        {
            PerformComputeModuleTests();
            PerformComputeTypesTests();
        }

        // the kernels of the built-in test set and the gate kernel, for every suite that dispatches
        if ( !GUCEF::COMPUTE::TEST::RegisterComputeTestKernels() )
        {
            GUCEF_TESTFW_TESTCASE( "ComputeTestApp: the test kernels register" )
            GUCEF_TESTFW_ERRORHERE;
            GUCEF_TESTFW_TESTCASE_END
        }
        if ( IsSuiteWanted( suites, "Cpu" ) )
        {
            PerformCpuBackendTests();
            PerformCpuParallelForTests();
        }
        if ( IsSuiteWanted( suites, "Registry" ) )
            PerformKernelRegistryTests();
        if ( IsSuiteWanted( suites, "Availability" ) )
            PerformBackendAvailabilityTests();
        if ( IsSuiteWanted( suites, "DeviceLoss" ) )
            PerformDeviceLossTests();
        if ( IsSuiteWanted( suites, "Queue" ) )
            PerformQueueContractTests();
        if ( IsSuiteWanted( suites, "MathBridge" ) )
        {
            PerformMathBridgeTests();
            PerformMathBridgeBenchmarks();
        }
        if ( IsSuiteWanted( suites, "Unload" ) )
            PerformComputeModuleUnloadTests();
        GUCEF::COMPUTE::TEST::UnregisterComputeTestKernels();

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
