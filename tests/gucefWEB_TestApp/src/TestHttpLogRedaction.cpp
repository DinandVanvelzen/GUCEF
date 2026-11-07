/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestHttpLogRedaction.h"

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

/*-------------------------------------------------------------------------*/

/** compares and logs the actual text when it differs, so a failure tells what was produced */
static bool
SameText( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HTTP log redaction test: expected \"" << expected << "\" but got \"" << actual << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** The state of the helper is global: a test that changes it puts it back */
class CHelperStateScope
{
    public:

    CHelperStateScope( void ) { WEB::CHttpLogRedaction::ClearSecrets(); WEB::CHttpLogRedaction::SetFullUrlsInDebugLogs( false ); }
    ~CHelperStateScope() { WEB::CHttpLogRedaction::ClearSecrets(); WEB::CHttpLogRedaction::SetFullUrlsInDebugLogs( false ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformUrlRuleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The URL of a request is logged with its identifiers, user info, query and fragment redacted and its words kept" )
        try
        {
            CHelperStateScope state;

            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "http://127.0.0.1:8080/trader/v1/accounts/ABCDEF0123456789ABCDEF0123456789/transactions?x=1" ),
                                   "http://127.0.0.1:8080/trader/v1/accounts/<redacted-id>/transactions?x=***" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://user:SECRETPW@api.example.com/v1/orders/42/items.json?page=2&access_token=SECRETTOKEN#SECRETFRAGMENT" ),
                                   "https://***@api.example.com/v1/orders/42/items.json?page=***&access_token=***#***" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/users/123e4567-e89b-12d3-a456-426614174000/orders/12345678/lines/3" ),
                                   "https://h/users/<redacted-id>/orders/<redacted-id>/lines/3" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/oauth/token/k3J9-x_Qm2LpV8sTz0aWbYc4dEfGhIjK" ),
                                   "https://h/oauth/token/<redacted-id>" ) );

            // the words of a path stay readable, whatever the level of the URL
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/v1/accounts/orders/positions/quotes/index.html" ), "https://h/v1/accounts/orders/positions/quotes/index.html" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "" ), "" ) );

            // the URL of a proxy: the user info goes
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "http://proxyuser:SECRETPROXYPASSWORD@proxy.example:3128" ), "http://***@proxy.example:3128" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "http://SECRETPROXYUSER@proxy.example:3128/" ), "http://***@proxy.example:3128/" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Text with URLs and credentials in it, such as the error of a backend, is logged without them" )
        try
        {
            CHelperStateScope state;

            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "Could not connect to https://u:SECRETPW@h/accounts/12345678/t?key=SECRETKEY: refused" ),
                                   "Could not connect to https://***@h/accounts/<redacted-id>/t?key=***: refused" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "Proxy said: Proxy-Authorization: Basic dXNlcjpTRUNSRVRQQVNT" ), "Proxy said: Proxy-Authorization: ***" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "Socket error 111" ), "Socket error 111" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformHeaderRuleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Header blocks are logged without the credentials and the secrets of their URLs, redirects included" )
        try
        {
            CHelperStateScope state;

            CORE::CString block = "GET /trader/v1/accounts/ABCDEF0123456789ABCDEF0123456789/transactions?access_token=SECRETQUERY HTTP/1.1\r\n"
                                  "Host: api.example.com\r\n"
                                  "Authorization: Bearer SECRETBEARER\r\n"
                                  "Proxy-Authorization: Basic SECRETPROXYBASIC\r\n"
                                  "Cookie: sid=SECRETCOOKIE\r\n"
                                  "Set-Cookie: sid=SECRETSETCOOKIE; Path=/\r\n"
                                  "X-Api-Key: SECRETAPIKEY\r\n"
                                  "Location: https://other.example.com/cb/12345678?code=SECRETCODE&state=abc\r\n"
                                  "Accept: application/json\r\n";
            CORE::CString expected = "GET /trader/v1/accounts/<redacted-id>/transactions?access_token=*** HTTP/1.1\r\n"
                                     "Host: api.example.com\r\n"
                                     "Authorization: ***\r\n"
                                     "Proxy-Authorization: ***\r\n"
                                     "Cookie: ***\r\n"
                                     "Set-Cookie: ***\r\n"
                                     "X-Api-Key: ***\r\n"
                                     "Location: https://other.example.com/cb/<redacted-id>?code=***&state=***\r\n"
                                     "Accept: application/json\r\n";
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaders( block ), expected ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A header value is logged without its secret when the header is a sensitive one" )
        try
        {
            CHelperStateScope state;

            static const char* const sensitive[] = { "Authorization", "Proxy-Authorization", "Cookie", "Set-Cookie", "X-Api-Key", "X-WebApi-Key", "x-auth-token" };
            for ( UInt32 i=0; i<sizeof( sensitive ) / sizeof( sensitive[ 0 ] ); ++i )
                ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaderValue( sensitive[ i ], "SECRETVALUE" ), "***" ) );

            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaderValue( "Content-Type", "text/plain" ), "text/plain" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaderValue( "Location", "https://h/a/12345678?t=SECRETT" ), "https://h/a/<redacted-id>?t=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSecretAndOptOutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A secret that an application registers is redacted by every function of the helper, also in its percent encoded form" )
        try
        {
            CHelperStateScope state;

            // a path segment which does not look like an identifier is logged until it is registered
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/acct-7/transactions" ), "https://h/accounts/acct-7/transactions" ) );

            ASSERT_FALSE( WEB::CHttpLogRedaction::AddSecret( "" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "a b+c" ) );

            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/acct-7/transactions?a=acct-7" ), "https://h/accounts/***/transactions?a=***" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/a%20b%2Bc/t" ), "https://h/accounts/***/t" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "no account acct-7 here" ), "no account *** here" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaders( "GET /accounts/acct-7 HTTP/1.1\r\nX-Account: acct-7\r\n" ), "GET /accounts/*** HTTP/1.1\r\nX-Account: ***\r\n" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactHeaderValue( "X-Account", "acct-7" ), "***" ) );

            WEB::CHttpLogRedaction::ClearSecrets();
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "no account acct-7 here" ), "no account acct-7 here" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A secret that an application takes back is logged again, one that two components registered stays redacted until both took it back" )
        try
        {
            CHelperStateScope state;

            ASSERT_FALSE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );
            ASSERT_FALSE( WEB::CHttpLogRedaction::RemoveSecret( "" ) );

            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "a b+c" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "acct-7 and a b+c" ), "*** and ***" ) );

            // one secret leaves, the other stays; the percent encoded form leaves with it
            ASSERT_TRUE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "acct-7 and a b+c" ), "acct-7 and ***" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/a%20b%2Bc/t" ), "https://h/accounts/***/t" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::RemoveSecret( "a b+c" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/a%20b%2Bc/t" ), "https://h/accounts/a%20b%2Bc/t" ) );
            ASSERT_FALSE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );

            // registered twice, as two logins with the same account would do: redacted until it was taken back twice
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "acct-7" ), "***" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "acct-7" ), "acct-7" ) );
            ASSERT_FALSE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );

            // a clear forgets the counts as well
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );
            WEB::CHttpLogRedaction::ClearSecrets();
            ASSERT_FALSE( WEB::CHttpLogRedaction::RemoveSecret( "acct-7" ) );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactText( "acct-7" ), "acct-7" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The opt-out for developers shows the full URLs in a debug build and keeps the credentials redacted" )
        try
        {
            CHelperStateScope state;

            ASSERT_FALSE( WEB::CHttpLogRedaction::GetFullUrlsInDebugLogs() );
            WEB::CHttpLogRedaction::SetFullUrlsInDebugLogs( true );
            WEB::CHttpLogRedaction::AddSecret( "REGISTEREDSECRET" );

            CORE::CString url = WEB::CHttpLogRedaction::RedactUrl( "https://u:SECRETPW@h/accounts/ABCDEF0123456789ABCDEF0123456789/t?x=1&k=REGISTEREDSECRET#frag" );
            CORE::CString headers = WEB::CHttpLogRedaction::RedactHeaders( "GET /a/12345678?q=1 HTTP/1.1\r\nAuthorization: Bearer SECRETBEARER\r\nCookie: SECRETCOOKIE\r\n" );
            ASSERT_TRUE( !Contains( url, "SECRETPW" ) && !Contains( url, "REGISTEREDSECRET" ) );
            ASSERT_TRUE( !Contains( headers, "SECRET" ) && Contains( headers, "Authorization: ***" ) );
            #ifdef GUCEF_DEBUG_MODE
            ASSERT_TRUE( WEB::CHttpLogRedaction::GetFullUrlsInDebugLogs() );
            ASSERT_TRUE( SameText( url, "https://***@h/accounts/ABCDEF0123456789ABCDEF0123456789/t?x=1&k=***#frag" ) );
            ASSERT_TRUE( Contains( headers, "GET /a/12345678?q=1 HTTP/1.1" ) );
            #else
            // a release build redacts everything, whatever was asked for
            ASSERT_FALSE( WEB::CHttpLogRedaction::GetFullUrlsInDebugLogs() );
            ASSERT_TRUE( SameText( url, "https://***@h/accounts/<redacted-id>/t?x=***&k=***#***" ) );
            #endif

            // and back to the default, which redacts
            WEB::CHttpLogRedaction::SetFullUrlsInDebugLogs( false );
            ASSERT_FALSE( WEB::CHttpLogRedaction::GetFullUrlsInDebugLogs() );
            ASSERT_TRUE( SameText( WEB::CHttpLogRedaction::RedactUrl( "https://h/accounts/ABCDEF0123456789ABCDEF0123456789/t?x=1" ), "https://h/accounts/<redacted-id>/t?x=***" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHttpLogRedactionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP log redaction TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpLogRedaction" );

    PerformUrlRuleTests();
    PerformHeaderRuleTests();
    PerformSecretAndOptOutTests();
}

/*-------------------------------------------------------------------------*/

void
PerformHttpLogRedactionBackendTests( WEB::CHTTPClient& client         ,
                                     const CORE::CString& backendName ,
                                     UInt16 closedPort                )
{GUCEF_TRACE;

    const CORE::CString hash = "ABCDEF0123456789ABCDEF0123456789";

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The log of a request which fails holds a secret that the application registered as redacted" ).C_String() )
        try
        {
            CHelperStateScope state;
            ASSERT_TRUE( WEB::CHttpLogRedaction::AddSecret( "acct-7" ) );

            CLogTextCapture capture;
            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "http://127.0.0.1:" + CORE::ToString( closedPort ) + "/trader/v1/accounts/acct-7/transactions" ) );
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "acct-7" ) );
            #ifdef GUCEF_DEBUG_MODE
            ASSERT_TRUE( Contains( logged, "/trader/v1/accounts/***/transactions" ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The opt-out for developers shows the path and query in a debug build, never the credentials" ).C_String() )
        try
        {
            CHelperStateScope state;
            WEB::CHttpLogRedaction::SetFullUrlsInDebugLogs( true );

            CLogTextCapture capture;
            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "http://user:SECRETPASSWORD@127.0.0.1:" + CORE::ToString( closedPort ) + "/trader/v1/accounts/" + hash + "/transactions?x=1" ) );
            request.otherHeaders[ "Authorization" ] = "Bearer SECRETBEARER";
            request.otherHeaders[ "Cookie" ] = "sid=SECRETCOOKIE";
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( !Contains( logged, "SECRET" ) );
            #ifdef GUCEF_DEBUG_MODE
            ASSERT_TRUE( Contains( logged, "/trader/v1/accounts/" + hash + "/transactions?x=1" ) );
            #else
            ASSERT_TRUE( !Contains( logged, hash ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
