/*
 *  gucefCOM: GUCEF module providing communication 
 *  implementations for standardized protocols.
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COM_H
#include "gucefCOM.h"
#define GUCEF_COM_H
#endif /* GUCEF_COM_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#include "TestDiscovery.h"
#include "TestRttProbes.h"
#include "TestNatTraversal.h"
#include "TestIrc.h"
#include "TestJsonRpc.h"
#include "TestMcpRegistry.h"
#include "TestMcpCore.h"
#include "TestMcpPolicy.h"
#include "TestMcpStdio.h"
#include "TestMcpTcp.h"
#include "TestMcpLegacy.h"
#include "TestStatsDClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

/**
 *  Suite filter, to repeat a part of the application quickly (for example under load or a sanitizer): when the
 *  environment variable GUCEF_TEST_ONLY_SUITES holds a comma separated list, only the Perform...Tests() functions
 *  whose name contains one of the entries (case insensitive) are run. Without the variable, or empty, everything runs.
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

    using namespace GUCEF;

    try
    {
        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefCOM_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger consoleOut;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

        CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        // The implementations register themselves when the module starts
        COM::CComGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefCOM_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        PerformUdpDiscoveryTests();
        PerformMdnsDiscoveryTests();
        PerformSsdpDiscoveryTests();
        PerformRttProbeTests();
        PerformNatPmpTests();
        PerformStunTests();
        PerformUpnpPluginTests();
        PerformIrcMessageTests();
        PerformIrcClientTests();
        PerformStatsDClientTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefCOM_TestApp_Results.xml" );
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
