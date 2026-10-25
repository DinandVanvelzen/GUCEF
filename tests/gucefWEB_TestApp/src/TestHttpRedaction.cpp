/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestHttpRedaction.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using GUCEF::TESTFW::CLogTextCapture;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformWebSocketRedactionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The log of a web socket client which is given a URL it cannot use holds no secrets" )
        try
        {
            CLogTextCapture capture;
            WEB::CWebSocketClient socketClient;
            ASSERT_FALSE( socketClient.Connect( "ws://user:SECRETPASSWORD@127.0.0.1:1/socket?token=SECRETTOKEN#SECRETFRAGMENT" ) );
            ASSERT_FALSE( socketClient.Connect( "ws://SECRETUSER@127.0.0.1/socket?token=SECRETTOKEN" ) );
            ASSERT_FALSE( socketClient.Connect( "ftp://127.0.0.1/socket?token=SECRETTOKEN" ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRET" ) );
            ASSERT_TRUE( Contains( logged, "Unsupported or invalid url \"ftp://127.0.0.1/socket?token=***\"" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformHeaderRedactionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Building a response header does not log its cookies or the query of its location" )
        try
        {
            CLogTextCapture capture;

            WEB::CHttpResponseData response;
            response.statusCode = 302;
            response.location = "http://h/next?code=SECRETLOCATION";
            response.otherHeaders[ "Set-Cookie" ] = "session=SECRETCOOKIE; Path=/";
            response.otherHeaders[ "X-Api-Key" ] = "SECRETAPIKEY";
            response.otherHeaders[ "Refresh" ] = "0; url=http://h/n?t=SECRETREFRESH";
            CORE::CDynamicBuffer serialized;
            ASSERT_TRUE( response.Serialize( serialized ) );

            // the response itself is intact
            CORE::CString wire( serialized.AsConstTypePtr< char >(), serialized.GetDataSize() );
            ASSERT_TRUE( Contains( wire, "Set-Cookie: session=SECRETCOOKIE" ) && Contains( wire, "code=SECRETLOCATION" ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRETLOCATION" ) && !Contains( logged, "SECRETCOOKIE" ) );
            ASSERT_TRUE( !Contains( logged, "SECRETAPIKEY" ) && !Contains( logged, "SECRETREFRESH" ) );
            #ifdef GUCEF_DEBUG_MODE
            // the log line exists in this build, so the checks above were made on something
            ASSERT_TRUE( Contains( logged, "Finished building response header" ) && Contains( logged, "Set-Cookie: ***" ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHttpRedactionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP redaction TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpRedaction" );

    PerformHeaderRedactionTests();
    PerformWebSocketRedactionTests();
}

/*-------------------------------------------------------------------------*/

void
PerformHttpRedactionBackendTests( WEB::CHTTPClient& client         ,
                                     const CORE::CString& backendName ,
                                     UInt16 closedPort                ,
                                     UInt16 scriptedServerPort        )
{GUCEF_TRACE;

    CORE::CString closedBase = "http://user:SECRETPASSWORD@127.0.0.1:" + CORE::ToString( closedPort );
    CORE::CString secretUrl = closedBase + "/api/getdata?key=SECRETKEYVALUE&appid=730&access_token=SECRETTOKEN#SECRETFRAGMENT";

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The log of a request which cannot reach the server holds no secrets of the URL or headers" ).C_String() )
        try
        {
            CLogTextCapture capture;

            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( secretUrl ) );
            request.otherHeaders[ "Authorization" ] = "Bearer SECRETBEARER";
            request.otherHeaders[ "Cookie" ] = "sid=SECRETCOOKIE";
            request.otherHeaders[ "x-webapi-key" ] = "SECRETWEBAPIKEY";

            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( !info.success && !info.errorDescription.IsNULLOrEmpty() );

            // what is handed back to the caller is the caller's own
            ASSERT_TRUE( Contains( info.effectiveUrl, "SECRETKEYVALUE" ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRET" ) );
            ASSERT_TRUE( !Contains( info.errorDescription, "SECRET" ) );
            #ifdef GUCEF_DEBUG_MODE
            // the failure is logged in this build, so the check above was made on something: the
            // host and the path are there, so are the names of the parameters
            ASSERT_TRUE( Contains( logged, "127.0.0.1:" + CORE::ToString( closedPort ) + "/api/getdata?key=***&appid=***&access_token=***" ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The log of an asynchronous request which cannot reach the server holds no secrets" ).C_String() )
        try
        {
            CLogTextCapture capture;

            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( secretUrl ) );
            request.otherHeaders[ "Authorization" ] = "Bearer SECRETBEARER";
            CORE::CFutureResult future = client.ExecuteAsync( request );
            CORE::CTaskPtr task = future.GetResult( 10000 );
            WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
            ASSERT_TRUE( GUCEF_NULL != data );
            if ( GUCEF_NULL != data )
                ASSERT_TRUE( !data->transferInfo.success );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRET" ) );
            #ifdef GUCEF_DEBUG_MODE
            ASSERT_TRUE( Contains( logged, "/api/getdata?key=***&appid=***" ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The log of a URI resource accessor request which fails holds no secrets" ).C_String() )
        try
        {
            CORE::CUriResourceAccessorFactory& factory = CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory();
            CORE::CUriResourceAccessorPtr accessor = factory.Create( "http" );
            ASSERT_FALSE( accessor.IsNULL() );
            if ( accessor.IsNULL() )
                return;

            CLogTextCapture capture;
            CORE::CDynamicBuffer content;
            ASSERT_FALSE( accessor->GetResource( CORE::CUri( "http://user:SECRETPASSWORD@127.0.0.1:" + CORE::ToString( closedPort ) + "/api/getdata?key=SECRETKEYVALUE#SECRETFRAGMENT" ), content ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRET" ) );
            // an error log, present in every build
            ASSERT_TRUE( Contains( logged, "HttpUriResourceAccessor: GET " ) && Contains( logged, "/api/getdata?key=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): A redirect to a URL which is not valid reports and logs it without its secrets" ).C_String() )
        try
        {
            CLogTextCapture capture;

            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "http://127.0.0.1:" + CORE::ToString( scriptedServerPort ) + "/redirect/invalid" ) );
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( Contains( info.errorDescription, "invalid URL" ) && Contains( info.errorDescription, "key=***" ) );
            ASSERT_TRUE( !Contains( info.errorDescription, "SECRET" ) );
            ASSERT_TRUE( !Contains( capture.GetText(), "SECRET" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
