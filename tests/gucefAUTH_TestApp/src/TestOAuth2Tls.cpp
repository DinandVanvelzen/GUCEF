/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogCapture.h"
#include "TestOAuth2Fixture.h"
#include "TestOAuth2Tls.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Prerequisites( void )
{GUCEF_TRACE;

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsRsaAvailable() && AUTH::CAuthCrypto::IsSignatureAvailable();
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------*/

/** the environment variable is put back as it was */
class CEnvironmentVariableScope
{
    public:

    CEnvironmentVariableScope( const CORE::CString& name, const CORE::CString& value )
        : m_name( name ), m_previous( CORE::GetEnv( name ) )
    {
        CORE::SetEnv( name, value );
    }

    ~CEnvironmentVariableScope()
    {
        CORE::SetEnv( m_name, m_previous );
    }

    private:

    CORE::CString m_name;
    CORE::CString m_previous;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOAuth2TlsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OAuth2 over TLS TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "OAuth2Tls" );

    // What this group tests is the native backend: a plugin for https that was loaded before goes away for the time of the group,
    // the main program loads it again for the groups that follow
    AUTH::CAuthGlobal::Instance()->GetOAuth2Engine()->ReleaseHttpBackends();
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "https" );

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 over TLS: the check of the assertion is a POST of a form over https through the native backend, a certificate of an authority that is not trusted ends the login" )
        CORE::CString whyNoTls;
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
        GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNoTls ), "this system has no TLS provider" );
        GUCEF_TESTFW_SKIP_IF( WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ), "The plugin for https cannot be taken away" );
        try
        {
            TESTFW::CTlsTestCerts certs;
            ASSERT_TRUE( certs.IsReady() );

            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            TESTFW::CTlsTestTerminator terminator;
            ASSERT_TRUE( terminator.Start( certs.GetPath( "good.crt" ), certs.GetPath( "good.key" ), f.fake.GetPort() ) );
            const CORE::CString secureBase = "https://localhost:" + CORE::ToString( terminator.GetPort() );

            // the HTTP client of the login engine is the native one, nothing else is there to carry https
            ASSERT_TRUE( AUTH::COAuth2Http::IsHttpsSupported() );
            WEB::CHTTPClient probe;
            ASSERT_TRUE( probe.GetBackendNameForScheme( "https" ) == "native" );

            // the engine trusts the authority of the test and no other: the bundle is a setting of the application
            const AUTH::COAuth2EngineSettings saved = f.engine->GetSettings();
            AUTH::COAuth2EngineSettings trusting = saved;
            trusting.caBundlePath = certs.GetPath( "ca.crt" );
            f.engine->SetSettings( trusting );

            // a POST of a form to the provider, as the check of an assertion is
            {
                AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
                AUTH::COAuth2Http http( settings );
                AUTH::CAuthHttpRequest request;
                ASSERT_TRUE( request.Post( secureBase + "/openid/login", "application/x-www-form-urlencoded", "openid.mode=check_authentication&openid.assoc_handle=1234&openid.sig=ab%2Bc%2FdE%3D", settings ) );
                AUTH::CAuthHttpResponse response;
                ASSERT_TRUE( http.Execute( request, response ) );
                ASSERT_TRUE( response.transportOk && 200 == response.statusCode && "native" == response.backendName );
                ASSERT_TRUE( Contains( CORE::CString( response.body.AsConstTypePtr< char >(), response.body.GetDataSize() ), "is_valid:false" ) );     // an assertion the provider never made
                ASSERT_TRUE( 1 == f.fake.GetStats().openIdCheck );
                ASSERT_TRUE( "application/x-www-form-urlencoded" == f.fake.GetLastOpenIdCheckContentType() );
                CORE::CString::StringMap form = f.fake.GetLastOpenIdCheck();
                ASSERT_TRUE( "check_authentication" == form[ "openid.mode" ] && "ab+c/dE=" == form[ "openid.sig" ] );
            }

            // the login end to end, the address of the provider is https: the fake says it is where the client reached it. The page of
            // the provider is fetched by the test browser, which is told what to trust the way programs are: with the variable of OpenSSL
            CEnvironmentVariableScope browserTrust( "SSL_CERT_FILE", certs.GetPath( "ca.crt" ) );
            CFakeBehavior behavior;
            behavior.openIdOpEndpointOverride = secureBase + "/openid/login";
            behavior.openIdClaimedIdOverride = secureBase + "/openid/id/4200000042";
            f.fake.SetBehavior( behavior );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = secureBase + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "testopenid" ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "testopenid:4200000042" == result.accountRef.ToString() );
            ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );
            ASSERT_TRUE( terminator.GetAcceptedConnections() >= 2 );

            // the authority is not trusted any more: the request fails as a transport failure with the class of TLS and a sentence that says
            // the certificate of the host is not trusted; the provider is never asked
            f.manager.Unlink( result.accountRef );
            {
                AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
                settings.caBundlePath = certs.GetPath( "other_ca.crt" );
                AUTH::COAuth2Http http( settings );
                AUTH::CAuthHttpRequest request;
                ASSERT_TRUE( request.Post( secureBase + "/openid/login", "application/x-www-form-urlencoded", "openid.mode=check_authentication", settings ) );
                AUTH::CAuthHttpResponse response;
                ASSERT_FALSE( http.Execute( request, response ) );
                ASSERT_TRUE( !response.transportOk && AUTH::AUTHHTTP_TLS == response.failure && "native" == response.backendName );
                AUTH::TAuthFailureReason reason = AUTH::AUTHFAIL_UNKNOWN;
                CORE::CString message;
                ASSERT_TRUE( response.DescribeFailure( reason, message ) );
                ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == reason || AUTH::AUTHFAIL_INVALID_RESPONSE == reason || AUTH::AUTHFAIL_NOT_AVAILABLE == reason );
                ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );

                // a bundle that cannot be read is no reason to skip the verification either
                settings.caBundlePath = certs.GetPath( "does-not-exist.crt" );
                AUTH::COAuth2Http unreadable( settings );
                ASSERT_TRUE( request.Post( secureBase + "/openid/login", "application/x-www-form-urlencoded", "openid.mode=check_authentication", settings ) );
                ASSERT_FALSE( unreadable.Execute( request, response ) );
                ASSERT_TRUE( AUTH::AUTHHTTP_TLS == response.failure );
                ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );
            }
            f.engine->SetSettings( saved );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
