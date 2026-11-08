/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "TestCode_PingTest.h"
#include "TestCode_ClientServer.h"
#include "TestCode_SimpleTCPClient.h"
#include "TestCode_DNS.h"
#include "TestCode_Discovery.h"
#include "TestCode_TlsLayer.h"
#include "TestCode_TlsOpenSsl.h"
#include "TestCode_TlsServer.h"
#include "TestDatabaseClient.h"
#include "TestRpcContract.h"
#include "TestRpcCodec.h"
#include "TestRpcEndpoint.h"
#include "TestRpcSession.h"
#include "TestRpcPerf.h"
#include "TestRpcLifecycle.h"
#include "TestSocketReadiness.h"
#include "TestTcpSocketThreads.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite filter, to repeat a part of the application quickly (for example in a loop under a sanitizer): when the
 *  environment variable GUCEF_TEST_ONLY_SUITES holds a comma separated list, only the Perform...Tests() functions
 *  whose name contains one of the entries (case insensitive) are run. Without the variable, or empty, everything runs.
 *      GUCEF_TEST_ONLY_SUITES=RpcLifecycle,RpcEndpoint ./gucefCOMCORE_TestApp
 */
static bool
IsSuiteEnabled( const char* suiteFunctionName )
{
    static const GUCEF::CORE::CString filter = GUCEF::CORE::GetEnv( "GUCEF_TEST_ONLY_SUITES" ).Uppercase();
    if ( filter.IsNULLOrEmpty() )
        return true;

    const GUCEF::CORE::CString name = GUCEF::CORE::CString( suiteFunctionName ).Uppercase();
    GUCEF::CORE::CString::StringVector entries = filter.ParseElements( ',', false );
    for ( size_t i=0; i<entries.size(); ++i )
        if ( !entries[ i ].IsNULLOrEmpty() && name.HasSubstr( entries[ i ] ) >= 0 )
            return true;
    return false;
}

#define RUN_SUITE( suiteFunction )  if ( IsSuiteEnabled( #suiteFunction ) ) suiteFunction()

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        // started by a test as a process of its own, see TestRpcEndpoint.h
        if ( argc > 2 && 0 == strcmp( argv[ 1 ], GUCEF_COMCORE_TESTAPP_RPC_FRESH_PROCESS_ARG ) )
            return RunRpcThreadPoolHandlerInFreshProcess( argv[ 2 ] );

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefCOMCORE_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger consoleOut;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

        CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        GUCEF_TESTFW_INIT( "gucefCOMCORE_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        RUN_SUITE( PerformDNSTests );
        RUN_SUITE( PerformClientServerTests );
        RUN_SUITE( PerformSimpleTCPClientTests );
        RUN_SUITE( PerformPingTests );
        RUN_SUITE( PerformDiscoveryTests );
        RUN_SUITE( PerformTlsLayerTests );
        RUN_SUITE( PerformTlsOpenSslTests );
        RUN_SUITE( PerformTlsServerTests );
        RUN_SUITE( PerformNetworkServiceInterfaceTests );
        RUN_SUITE( PerformDatabaseClientTests );
        RUN_SUITE( PerformRpcCodecTests );
        RUN_SUITE( PerformRpcContractTests );
        RUN_SUITE( PerformRpcEndpointTests );
        RUN_SUITE( PerformRpcSessionTests );
        RUN_SUITE( PerformRpcPerfTests );
        RUN_SUITE( PerformRpcLifecycleTests );
        RUN_SUITE( PerformSocketReadinessTests );
        RUN_SUITE( PerformTcpSocketThreadsTests );

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefCOMCORE_TestApp_Results.xml" );
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

/*-------------------------------------------------------------------------*/
