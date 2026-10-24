/*
 *  gucefCOM: GUCEF module providing communication 
 *  implementations for standardized protocols.
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COM_H
#include "gucefCOM.h"
#define GUCEF_COM_H
#endif /* GUCEF_COM_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef TESTCODE_CHTTPCLIENT_H
#include "TestCode_CHTTPClient.h"
#define TESTCODE_CHTTPCLIENT_H
#endif /* TESTCODE_CHTTPCLIENT_H ? */

#ifndef TESTCODE_CHTTPSERVER_H
#include "TestCode_CHTTPServer.h"
#define TESTCODE_CHTTPSERVER_H
#endif /* TESTCODE_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H
#include "TestWebSocketCodec.h"
#define GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H
#endif /* GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTWEBSOCKET_H
#include "TestWebSocket.h"
#define GUCEF_WEB_TESTAPP_TESTWEBSOCKET_H
#endif /* GUCEF_WEB_TESTAPP_TESTWEBSOCKET_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTJSONRPC_H
#include "TestJsonRpc.h"
#define GUCEF_WEB_TESTAPP_TESTJSONRPC_H
#endif /* GUCEF_WEB_TESTAPP_TESTJSONRPC_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTCHROMEDEVTOOLS_H
#include "TestChromeDevTools.h"
#define GUCEF_WEB_TESTAPP_TESTCHROMEDEVTOOLS_H
#endif /* GUCEF_WEB_TESTAPP_TESTCHROMEDEVTOOLS_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H
#include "TestHttpClient.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_TESTIRCWEBSOCKET_H
#include "TestIrcWebSocket.h"
#define GUCEF_WEB_TESTIRCWEBSOCKET_H
#endif /* GUCEF_WEB_TESTIRCWEBSOCKET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The original interactive scenarios: a HTTP client GET against a public site and a HTTP server which
 *  keeps running on a port for use with browsers and other tools. Not part of the automated test run,
 *  start the application with -manual to use them.
 */
static int
RunManualTests( int argc, char** argv )
{GUCEF_TRACE;

    using namespace GUCEF;

    SetupHTTPClientTest();
    RunHTTPServerTest( argc, argv );
    return 1;
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        for ( int i=1; i<argc; ++i )
        {
            if ( 0 == strcmp( argv[ i ], "-manual" ) )
                return RunManualTests( argc, argv );
        }

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefWEB_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the module, as every application using gucefWEB has to do
        WEB::CWebGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefWEB_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( CORE::TestFrameworkLogCallback, GUCEF_NULL );

        PerformWebSocketCodecTests();
        PerformWebSocketTests();
        PerformJsonRpcTests();
        PerformChromeDevToolsTests();
        PerformHttpClientTests();
        PerformIrcWebSocketTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefWEB_TestApp_Results.xml" );
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
