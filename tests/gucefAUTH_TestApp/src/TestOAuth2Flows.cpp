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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

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

#include "TestText.h"
#include "TestLogCapture.h"
#include "gucefCORE_BrowserLauncher.h"
#include "TestOAuth2Fixture.h"
#include "TestOpenId2Backend.h"
#include "TestOAuth2Flows.h"

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

static CORE::CString
ChallengeOf( const CORE::CString& verifier )
{GUCEF_TRACE;

    UInt8 digest[ AUTH::CAuthCrypto::DigestSize ];
    AUTH::CAuthCrypto::Sha256( verifier.C_String(), AUTH::GetByteLength( verifier ), digest );
    CORE::CString challenge;
    CORE::Base64UrlEncode( digest, AUTH::CAuthCrypto::DigestSize, challenge );
    return challenge;
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{GUCEF_TRACE;

    return !part.IsNULLOrEmpty() && text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

/** Waits for the address of a login that wants the browser, and returns it */
static bool
PumpUntilAction( COAuth2Fixture& f, UInt32 count, UInt32 timeoutInMs = 5000 )
{GUCEF_TRACE;

    return PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_ACTION, count, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

static bool
ListenerIsGone( const CORE::CString& redirectUri )
{GUCEF_TRACE;

    WEB::CHttpRequestData request;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    request.SetUrl( redirectUri.SubstrToChar( '?', true ) );
    return !PumpedExecute( request, response, info, CORE::CString::Empty, 0, 2000 );
}

/*-------------------------------------------------------------------------*/

/** Stops the test case when the browser kept no redirect (the case cannot go on without it) */
#define REQUIRE_REDIRECT( f ) do { if ( (f).browser.GetRedirects().empty() ) { GUCEF_TESTFW_ASSERT_TRUE( !(f).browser.GetRedirects().empty() ); throw CORE::CString( "required: a redirect" ); } } while ( 0 )

/** Whether the last authorization request that the fake got had a login_hint */
static bool
HasLoginHintParameter( COAuth2Fixture& f )
{GUCEF_TRACE;

    CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
    return asked.find( "login_hint" ) != asked.end();
}

/*-------------------------------------------------------------------------*/

static void
PerformCodeFlowTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A login with the Google descriptor through the manager: PKCE, state, nonce, loopback redirect, tokens, link" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings google = f.LocalSettings();
            ASSERT_TRUE( f.ConfigureOne( "google", google ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "google" ) );

            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "google:sub-1000" == result.accountRef.ToString() && "google" == result.providerName && "Fake Player" == result.displayName );
            ASSERT_TRUE( "https://pictures.invalid/p.png" == result.avatarUrl );
            ASSERT_TRUE( !result.accessToken.IsEmpty() && !result.idToken.IsEmpty() && !result.refreshToken.IsEmpty() );
            ASSERT_TRUE( result.expiresAtMs > AUTH::GetEpochTimeInMs() + 3000000 && result.expiresAtMs < AUTH::GetEpochTimeInMs() + 4000000 );
            ASSERT_TRUE( AUTH::ContainsString( result.scopes, "openid" ) );
            ASSERT_TRUE( AUTH::CAuthProof::KindOidcIdToken == result.proof.kind && result.proof.data.Reveal() == result.idToken.Reveal() );

            // the browser got the address that the event carried, once
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_ACTION ) );
            const CAuthRecorder::SRecord* action = f.recorder.Last( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( GUCEF_NULL != action && AUTH::AUTHACTION_OPEN_URL == action->action.kind && "google" == action->action.providerName );
            ASSERT_TRUE( 1 == f.browser.GetOpenCount() && action->action.url == f.browser.GetLastUrl() && action->action.expiresAtMs > AUTH::GetEpochTimeInMs() );

            // what the authorization request asked
            CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
            ASSERT_TRUE( "code" == asked[ "response_type" ] && "fake-client-id" == asked[ "client_id" ] && "S256" == asked[ "code_challenge_method" ] );
            ASSERT_TRUE( asked[ "state" ].Length() >= 22 && asked[ "nonce" ].Length() >= 22 && 43 == asked[ "code_challenge" ].Length() );
            ASSERT_TRUE( Contains( asked[ "redirect_uri" ], "http://127.0.0.1:" ) && asked[ "redirect_uri" ].EndsWith( "/callback" ) );
            ASSERT_TRUE( Contains( asked[ "scope" ], "openid" ) && "offline" == asked[ "access_type" ] );

            // what the token request carried: the verifier is a verifier and belongs to the challenge, the code was used once
            CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "authorization_code" == sent[ "grant_type" ] && "fake-client-id" == sent[ "client_id" ] && asked[ "redirect_uri" ] == sent[ "redirect_uri" ] );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsValidCodeVerifier( sent[ "code_verifier" ] ) && ChallengeOf( sent[ "code_verifier" ] ) == asked[ "code_challenge" ] );
            ASSERT_TRUE( sent.end() == sent.find( "client_secret" ) && "application/x-www-form-urlencoded" == f.fake.GetLastTokenContentType() );
            CFakeStats stats = f.fake.GetStats();
            ASSERT_TRUE( 1 == stats.token && 0 == stats.pkceFailures && 0 == stats.codeReuse && 1 == stats.jwks && 1 == stats.authorize );
            ASSERT_TRUE( 0 == stats.userinfo );

            // the redirect address is not served any more, no login is under way
            CORE::CString redirect = f.browser.GetRedirects().empty() ? CORE::CString::Empty : f.browser.GetRedirects()[ 0 ];
            ASSERT_TRUE( !redirect.IsNULLOrEmpty() && ListenerIsGone( asked[ "redirect_uri" ] ) );
            PumpFor( 250 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );
            ASSERT_TRUE( 1 == f.browser.GetStatuses().size() && 200 == f.browser.GetStatuses()[ 0 ] );

            // the login is linked and the refresh token is stored
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            AUTH::CSecretString stored;
            ASSERT_TRUE( f.credentials.Load( "google", "google:sub-1000", stored ) && stored.Reveal() == result.refreshToken.Reveal() );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );

            // state, nonce and verifier are new every time
            CORE::CString::StringSet states, nonces, verifiers;
            states.insert( asked[ "state" ] );
            nonces.insert( asked[ "nonce" ] );
            verifiers.insert( sent[ "code_verifier" ] );
            for ( UInt32 i=0; i<3; ++i )
            {
                f.manager.Unlink( result.accountRef );
                ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
                CORE::CString::StringMap again = f.fake.GetLastAuthorizeParameters();
                states.insert( again[ "state" ] );
                nonces.insert( again[ "nonce" ] );
                verifiers.insert( f.fake.GetLastTokenParameters()[ "code_verifier" ] );
            }
            ASSERT_TRUE( 4 == states.size() && 4 == nonces.size() && 4 == verifiers.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The e-mail address of a login is remembered from the claims and prefilled at the next one, as login_hint through the real encoding; an explicit hint wins, the setting off stops it, the hint is not logged" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CLogCapture capture;
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthSettings both;
            both.SetProviderSettings( "google", f.LocalSettings() );
            AUTH::CAuthProviderSettings facebook = f.LocalSettings();
            facebook.userInfoEndpoint = f.fake.GetBaseUrl() + "/me";
            both.SetProviderSettings( "facebook", facebook );
            ASSERT_TRUE( f.Configure( both ) );

            // the id token carries the address: a plus, an at sign and a letter beyond ASCII, all three have to survive the url
            const CORE::CString address = "j\xC3\xBCrgen+test@example.invalid";
            CFakeTokenSpec spec;
            spec.extraPayloadMembers = "\"email\":\"j\\u00fcrgen+test@example.invalid\"";
            f.fake.SetTokenSpec( spec );

            // the first login has nothing to prefill; the address of the claims is what the next one gets
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( !HasLoginHintParameter( f ) );
            ASSERT_TRUE( address == result.loginHint );
            CORE::CString remembered;
            ASSERT_TRUE( f.manager.GetSettings().GetLoginHint( "google", remembered ) && address == remembered && f.manager.HasLoginHints() );
            ASSERT_TRUE( f.recorder.Count( CAuthRecorder::KIND_SETTINGS ) >= 1 );

            // the second login sends it; the fake server decoded the request it got from the browser address
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( address == f.fake.GetLastAuthorizeParameters()[ "login_hint" ] );
            // in the address of the browser the characters are percent encoded and the address is still one the launcher accepts
            ASSERT_TRUE( Contains( f.browser.GetLastUrl(), "login_hint=j%C3%BCrgen%2Btest%40example.invalid" ) );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( f.browser.GetLastUrl() ) );

            // an explicit hint of the caller beats the remembered one
            AUTH::CAuthOptions explicitOptions;
            explicitOptions.loginHint = "someone.else@example.invalid";
            ASSERT_TRUE( f.Login( "google", explicitOptions, result ) );
            ASSERT_TRUE( "someone.else@example.invalid" == f.fake.GetLastAuthorizeParameters()[ "login_hint" ] );

            // a provider that has no such parameter is sent nothing, whatever the caller gives
            CFakeBehavior behavior;
            behavior.issueIdToken = false;
            behavior.requirePkce = false;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Login( "facebook", explicitOptions, result ) );
            ASSERT_TRUE( !HasLoginHintParameter( f ) );
            ASSERT_FALSE( Contains( f.browser.GetLastUrl(), "login_hint" ) );
            CORE::CString none;
            ASSERT_FALSE( f.manager.GetSettings().GetLoginHint( "facebook", none ) );
            behavior = CFakeBehavior();
            f.fake.SetBehavior( behavior );

            // forgetting: the next login starts with nothing
            ASSERT_TRUE( f.manager.ClearLoginHints() && !f.manager.HasLoginHints() );
            ASSERT_FALSE( f.manager.ClearLoginHints() );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( !HasLoginHintParameter( f ) );
            // and that login remembered it again
            ASSERT_TRUE( f.manager.HasLoginHints() );

            // the setting off: nothing is used, nothing is kept
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            settings.rememberLoginHints = false;
            f.manager.SetSettings( settings );
            ASSERT_FALSE( f.manager.HasLoginHints() );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( !HasLoginHintParameter( f ) );
            ASSERT_TRUE( f.manager.GetSettings().loginHints.empty() );

            // the address is personal data: the log says only whether there was a hint
            ASSERT_FALSE( capture.Contains( "test@example.invalid" ) );
            ASSERT_FALSE( capture.Contains( "someone.else" ) );
            ASSERT_TRUE( capture.Contains( "login hint used: yes" ) && capture.Contains( "login hint used: no" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A name beyond ASCII is kept as it is, control characters are taken out of it" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            CFakeTokenSpec spec;
            spec.name = "Zo\xC3\xAB \xE2\x98\x83\x01x\x7f\xF0\x9F\x8E\xAE";
            f.fake.SetTokenSpec( spec );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( CORE::CString( "Zo\xC3\xAB \xE2\x98\x83x\xF0\x9F\x8E\xAE" ) == result.displayName );

            // a name that is too long is not taken
            f.manager.Unlink( result.accountRef );
            spec.name = RepeatedText( 'n', 400 );
            f.fake.SetTokenSpec( spec );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && result.displayName.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The addresses can come from the discovery document, which is cached" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.DiscoverySettings() ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
            CFakeStats stats = f.fake.GetStats();
            ASSERT_TRUE( 1 == stats.discovery && 1 == stats.jwks && 1 == stats.token );
            // the document is cached
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().discovery );

            // a document of another issuer is not accepted
            COAuth2Fixture other;
            ASSERT_TRUE( other.ready );
            AUTH::CAuthProviderSettings wrong = other.DiscoverySettings();
            wrong.issuer = "https://not.the.issuer.example";
            ASSERT_TRUE( other.ConfigureOne( "google", wrong ) );
            ASSERT_TRUE( other.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( 0 == other.browser.GetOpenCount() && 0 == other.fake.GetStats().authorize );

            // a discovery address that is not served
            COAuth2Fixture missing;
            ASSERT_TRUE( missing.ready );
            AUTH::CAuthProviderSettings nowhere = missing.DiscoverySettings();
            nowhere.discoveryUrl = missing.fake.GetBaseUrl() + "/nowhere/.well-known/other";
            ASSERT_TRUE( missing.ConfigureOne( "google", nowhere ) );
            ASSERT_TRUE( missing.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Microsoft: the issuer names the tenant, the account is tenant:object, a tenant outside the setting is refused" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            f.fake.SetIssuerPath( "/{tid}/v2.0" );
            CFakeTokenSpec spec;
            spec.tenantId = "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001";
            spec.objectId = "0b3f2a9c-7d11-4e2e-9a77-000000000042";
            f.fake.SetTokenSpec( spec );

            AUTH::CAuthProviderSettings microsoft = f.LocalSettings();
            microsoft.issuer = f.fake.GetBaseUrl() + "/{tid}/v2.0";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "microsoft", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "microsoft:5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001:0b3f2a9c-7d11-4e2e-9a77-000000000042" == result.accountRef.ToString() );
            ASSERT_TRUE( AUTH::ContainsString( result.scopes, "offline_access" ) || AUTH::ContainsString( result.scopes, "openid" ) );
            ASSERT_TRUE( Contains( f.browser.GetLastUrl(), "offline_access" ) );

            // a tenant of its own in the setting: this one is allowed, another is not
            f.manager.Unlink( result.accountRef );
            microsoft.parameters[ "tenant" ] = "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_TRUE( f.Login( "microsoft", AUTH::CAuthOptions(), result ) );
            f.manager.Unlink( result.accountRef );
            microsoft.parameters[ "tenant" ] = "11111111-2222-3333-4444-555555555555";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_TRUE( f.Fails( "microsoft", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            // organizations excludes the tenant of personal accounts
            microsoft.parameters[ "tenant" ] = "organizations";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_TRUE( f.Login( "microsoft", AUTH::CAuthOptions(), result ) );
            f.manager.Unlink( result.accountRef );
            spec.tenantId = AUTH::COAuth2Protocol::ConsumersTenantId;
            f.fake.SetTokenSpec( spec );
            ASSERT_TRUE( f.Fails( "microsoft", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            microsoft.parameters[ "tenant" ] = "consumers";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_TRUE( f.Login( "microsoft", AUTH::CAuthOptions(), result ) && Contains( result.accountRef.ToString(), AUTH::COAuth2Protocol::ConsumersTenantId ) );

            // the tid claim of a token has to be the tenant of its issuer
            f.manager.Unlink( result.accountRef );
            microsoft.parameters[ "tenant" ] = "common";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            spec.tenantId = "tenant-claim";
            spec.issuer = f.fake.GetBaseUrl() + "/tenant-issuer/v2.0";
            f.fake.SetTokenSpec( spec );
            ASSERT_TRUE( f.Fails( "microsoft", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            // an issuer of the shape but of another host is not the issuer
            spec.issuer = "https://evil.example/tenant-claim/v2.0";
            f.fake.SetTokenSpec( spec );
            ASSERT_TRUE( f.Fails( "microsoft", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Amazon Cognito: the account names the pool, the addresses come from the parameters of the pool" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            f.fake.SetIssuerPath( "/eu-west-1_TestPool" );
            AUTH::CAuthProviderSettings cognito = f.LocalSettings();
            cognito.parameters[ "region" ] = "eu-west-1";
            cognito.parameters[ "poolId" ] = "eu-west-1_TestPool";
            cognito.parameters[ "poolDomain" ] = "game.auth.eu-west-1.amazoncognito.com";
            ASSERT_TRUE( f.ConfigureOne( "aws-cognito", cognito ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "aws-cognito", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "aws:eu-west-1_TestPool:sub-1000" == result.accountRef.ToString() );

            // without the parameters the provider is not offered
            COAuth2Fixture bare;
            ASSERT_TRUE( bare.ready );
            AUTH::CAuthProviderSettings incomplete;
            incomplete.clientId = "fake-client-id";
            ASSERT_TRUE( bare.ConfigureOne( "aws-cognito", incomplete ) );
            ASSERT_FALSE( bare.manager.IsProviderOffered( "aws-cognito" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every kind of wrong id token is refused, nothing is stored or linked, a good one after them works" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            UInt32 index = 0;
            CFakeTokenSpec spec;
            CORE::CString name;
            UInt32 refused = 0;
            for ( ; ; ++index )
            {
                spec = CFakeTokenSpec();
                if ( !BadTokenSpec( index, f.fake.GetIssuer(), spec, name ) )
                    break;
                f.fake.SetTokenSpec( spec );
                bool failed = f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE );
                if ( !failed )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The wrong id token was not refused: " << name );
                ASSERT_TRUE( failed );
                if ( failed )
                    ++refused;
            }
            ASSERT_TRUE( 22 == index && 22 == refused );
            AUTH::CSecretString none;
            ASSERT_FALSE( f.credentials.Load( "google", "google:sub-1000", none ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() && 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            PumpFor( 250 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );

            // a token that is only a little off in time is within the skew; the same far off is not
            spec = CFakeTokenSpec();
            spec.expiresInSeconds = -60;
            spec.setNotBefore = true;
            spec.notBeforeInSeconds = 60;
            spec.issuedAtInSeconds = 60;
            f.fake.SetTokenSpec( spec );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformKeyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A key id the cache does not know gives one refresh and then a failure, not a loop; a rotated key is found by that refresh" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
            settings.jwksMinRefreshIntervalInMs = 1000;
            f.engine->SetSettings( settings );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );

            // right after the fetch an unknown key id is just refused: no fetch
            CFakeTokenSpec spec;
            spec.keyId = "nobody";
            f.fake.SetTokenSpec( spec );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );

            // after the interval: one refresh, then it fails
            PumpFor( 1100 );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().jwks );
            // and the next one, right after, does not fetch
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().jwks );

            // the provider rotates its key: the first token with the new id finds it with one refresh
            f.fake.PublishKey( "key-b", &TestKeyB() );
            f.fake.SetSigningKey( "key-b", &TestKeyB() );
            f.fake.UnpublishKey( "key-a" );
            f.fake.SetTokenSpec( CFakeTokenSpec() );
            PumpFor( 1100 );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
            ASSERT_TRUE( 3 == f.fake.GetStats().jwks );
            // the new key is cached now
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 3 == f.fake.GetStats().jwks );

            // a token that names the old key id, signed by the old key, is now refused: it is not in the set
            CFakeTokenSpec old;
            old.keyId = "key-a";
            old.signWith = "key-a";
            f.fake.SetTokenSpec( old );
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The key set is kept as long as the provider says; a set that is stale is fetched again" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
            settings.jwksMinTtlInSeconds = 1;
            settings.jwksMaxTtlInSeconds = 100;
            settings.jwksDefaultTtlInSeconds = 50;
            f.engine->SetSettings( settings );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );

            CFakeBehavior behavior;
            behavior.jwksCacheControl = "public, max-age=1";
            f.fake.SetBehavior( behavior );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );
            PumpFor( 1200 );
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().jwks );

            // no header: the default of the settings; a header that forbids caching: the minimum
            behavior.jwksCacheControl = CORE::CString::Empty;
            f.fake.SetBehavior( behavior );
            f.engine->GetJwksCache().Clear();
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            PumpFor( 1200 );
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 3 == f.fake.GetStats().jwks );
            behavior.jwksCacheControl = "no-store";
            f.fake.SetBehavior( behavior );
            f.engine->GetJwksCache().Clear();
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            PumpFor( 1200 );
            f.manager.Unlink( result.accountRef );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( 5 == f.fake.GetStats().jwks );

            // a key set that is not acceptable fails the login and is not kept
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthProviderSettings broken = g.LocalSettings();
            broken.jwksUri = g.fake.GetBaseUrl() + "/.well-known/openid-configuration";
            ASSERT_TRUE( g.ConfigureOne( "google", broken ) );
            ASSERT_TRUE( g.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            AUTH::CAuthProviderSettings gone = g.LocalSettings();
            gone.jwksUri = g.fake.GetBaseUrl() + "/nokeys";
            ASSERT_TRUE( g.ConfigureOne( "google", gone ) );
            ASSERT_TRUE( g.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformListenerFlowTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A callback with another host, method, path or state does not end the login; the right one does, the next is turned away" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            f.browser.SetMode( CTestBrowser::MODE_SEQUENCE );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
            for ( UInt32 waited=0; waited<3000 && !f.browser.IsIdle(); ++waited )
                PumpFor( 1 );
            GUCEF::vector< int > statuses = f.browser.GetStatuses();
            ASSERT_TRUE( 6 == statuses.size() );
            if ( 6 == statuses.size() )
            {
                ASSERT_TRUE( 400 == statuses[ 0 ] );                 // another host
                ASSERT_TRUE( 405 == statuses[ 1 ] );                 // POST
                ASSERT_TRUE( 404 == statuses[ 2 ] );                 // another path
                ASSERT_TRUE( 400 == statuses[ 3 ] );                 // another state
                ASSERT_TRUE( 200 == statuses[ 4 ] );                 // the callback
                ASSERT_TRUE( 404 == statuses[ 5 ] || 0 == statuses[ 5 ] );  // once
            }
            ASSERT_TRUE( 1 == f.fake.GetStats().token );

            // only a wrong state: the login waits, and can be cancelled
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            ASSERT_TRUE( g.ConfigureOne( "google", g.LocalSettings() ) );
            g.browser.SetMode( CTestBrowser::MODE_WRONG_STATE_ONLY );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( g.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            for ( UInt32 waited=0; waited<3000 && g.browser.GetStatuses().empty(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( 1 == g.browser.GetStatuses().size() && 400 == g.browser.GetStatuses()[ 0 ] );
            PumpFor( 300 );
            ASSERT_TRUE( !future.IsReady() && g.manager.IsLoginPending( "google" ) && 0 == g.fake.GetStats().token );
            ASSERT_TRUE( g.manager.CancelLogin( "google" ) );
            for ( UInt32 waited=0; waited<3000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            PumpUntilRecorded( g.recorder, CAuthRecorder::KIND_FAILED, 1, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == g.LastFailure() );
            ASSERT_TRUE( ListenerIsGone( g.fake.GetLastAuthorizeParameters()[ "redirect_uri" ] ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A slow login that is finished after nine minutes works, one that is not finished in ten ends as a timeout; the setting changes the time; the listener goes with the login" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            ASSERT_TRUE( 600000 == AUTH::CAuthOptions::DefaultTimeoutInMs && 600 == AUTH::CAuthSettings::DefaultLoginTimeoutInSeconds );

            // the browser is told to wait: the redirect to the listener is kept and gone to later, minutes later by the clock of the module
            struct SGuard { ~SGuard() { AUTH::SetMonotonicClockOffsetInMs( 0 ); } } guard;
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            f.browser.SetMode( CTestBrowser::MODE_CAPTURE );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            for ( UInt32 waited=0; waited<5000 && f.browser.GetRedirects().empty(); ++waited )
                PumpFor( 1 );
            REQUIRE_REDIRECT( f );
            const CORE::CString redirect = f.browser.GetRedirects()[ 0 ];

            // what the person is told: the address and the time it is good for, ten minutes
            const CAuthRecorder::SRecord* action = f.recorder.Last( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( GUCEF_NULL != action && action->action.expiresAtMs > AUTH::GetEpochTimeInMs() + 590000 && action->action.expiresAtMs <= AUTH::GetEpochTimeInMs() + 600000 );

            // nine minutes and the login is still waiting, and then it works
            AUTH::SetMonotonicClockOffsetInMs( 540000 );
            PumpFor( 100 );
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( 1 == f.engine->GetActiveLoginCount() );
            WEB::CHttpRequestData request;
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            request.SetUrl( redirect );
            ASSERT_TRUE( PumpedExecute( request, response, info, CORE::CString::Empty, 0, 5000 ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            AUTH::CAuthResult result;
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && "google:sub-1000" == result.accountRef.ToString() );
            AUTH::SetMonotonicClockOffsetInMs( 0 );

            // not finished at all: after ten minutes and a bit the flow ends it, with the reason timeout, and the listener is gone
            f.manager.Unlink( result.accountRef );
            f.browser.ClearHistory();
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            for ( UInt32 waited=0; waited<5000 && f.browser.GetRedirects().empty(); ++waited )
                PumpFor( 1 );
            REQUIRE_REDIRECT( f );
            const CORE::CString lateRedirect = f.browser.GetRedirects()[ 0 ];
            AUTH::SetMonotonicClockOffsetInMs( 590000 );
            PumpFor( 100 );
            ASSERT_FALSE( future.IsReady() );
            AUTH::SetMonotonicClockOffsetInMs( 601000 );
            for ( UInt32 waited=0; waited<5000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_FAILED, 1, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == f.LastFailure() );
            AUTH::SetMonotonicClockOffsetInMs( 0 );
            ASSERT_TRUE( ListenerIsGone( lateRedirect ) );
            PumpFor( 200 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );

            // the setting: a minute and a half, applied because the options say the default; an explicit time of the caller is left alone
            AUTH::CAuthSettings settings;
            settings.SetProviderSettings( "google", f.LocalSettings() );
            settings.loginTimeoutInSeconds = 90;
            ASSERT_TRUE( f.Configure( settings ) );
            f.browser.ClearHistory();
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            for ( UInt32 waited=0; waited<5000 && f.browser.GetRedirects().empty(); ++waited )
                PumpFor( 1 );
            REQUIRE_REDIRECT( f );
            action = f.recorder.Last( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( GUCEF_NULL != action && action->action.expiresAtMs <= AUTH::GetEpochTimeInMs() + 90000 && action->action.expiresAtMs > AUTH::GetEpochTimeInMs() + 80000 );
            AUTH::SetMonotonicClockOffsetInMs( 100000 );
            for ( UInt32 waited=0; waited<5000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_FAILED, 2, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == f.LastFailure() );
        }
        catch ( ... )
        {
            AUTH::SetMonotonicClockOffsetInMs( 0 );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login that nobody finishes ends at its timeout, one that is cancelled ends cancelled, both close the listener" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            f.browser.SetMode( CTestBrowser::MODE_IGNORE );

            AUTH::CAuthOptions options;
            options.timeoutInMs = 400;
            UInt64 started = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_TIMEOUT ) );
            UInt64 took = AUTH::GetMonotonicTimeInMs() - started;
            ASSERT_TRUE( took >= 380 && took < 4000 );
            ASSERT_TRUE( 1 == f.browser.GetOpenCount() );
            CORE::CString redirectUri = f.LastUrlParameters()[ "redirect_uri" ];
            ASSERT_TRUE( !redirectUri.IsNULLOrEmpty() && ListenerIsGone( redirectUri ) );

            // cancelled when the action was announced
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( PumpUntilAction( f, 2 ) );
            redirectUri = f.LastUrlParameters()[ "redirect_uri" ];
            UInt64 cancelAt = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( f.manager.CancelLogin( "google" ) );
            for ( UInt32 waited=0; waited<3000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && AUTH::GetMonotonicTimeInMs() - cancelAt < 2000 );
            PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_FAILED, 2, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == f.LastFailure() && ListenerIsGone( redirectUri ) );
            PumpFor( 200 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );
            ASSERT_TRUE( 0 == f.fake.GetStats().token );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A refusal at the provider, and errors of the token endpoint, end the login with the right reason" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );

            CFakeBehavior behavior;
            behavior.denyAuthorization = true;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_DENIED ) );
            ASSERT_TRUE( 0 == f.fake.GetStats().token );

            behavior = CFakeBehavior();
            behavior.tokenError = "invalid_grant";
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_GRANT ) );
            behavior.tokenError = "invalid_client";
            behavior.tokenErrorStatus = 401;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            behavior.tokenError = "temporarily_unavailable";
            behavior.tokenErrorStatus = 503;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
            behavior.tokenError = "access_denied";
            behavior.tokenErrorStatus = 400;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_DENIED ) );

            // a token endpoint that redirects is not followed
            behavior = CFakeBehavior();
            f.fake.SetBehavior( behavior );
            AUTH::CAuthProviderSettings redirecting = f.LocalSettings();
            redirecting.tokenEndpoint = f.fake.GetBaseUrl() + "/redirect";
            ASSERT_TRUE( f.ConfigureOne( "google", redirecting ) );
            UInt32 tokensBefore = f.fake.GetStats().token;
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( tokensBefore == f.fake.GetStats().token );

            // a token endpoint that is not there
            AUTH::CAuthProviderSettings down = f.LocalSettings();
            down.tokenEndpoint = "http://127.0.0.1:1/token";
            ASSERT_TRUE( f.ConfigureOne( "google", down ) );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );

            // the id token is the only place the account comes from: the profile has to be of the same subject
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            CFakeTokenSpec spec;
            spec.name = CORE::CString::Empty;
            f.fake.SetTokenSpec( spec );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "Info Player" == result.displayName && 1 == f.fake.GetStats().userinfo );
            f.manager.Unlink( result.accountRef );
            behavior.userinfoSubjectOverride = "someone-else";
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            // a profile that cannot be had does not stop a login that is proven by its id token
            behavior.userinfoSubjectOverride = CORE::CString::Empty;
            behavior.userinfoFails = true;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() && result.displayName.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBrowserTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The address is announced always; the browser is opened unless the options or the engine say not; a launch that fails leaves the address in the event" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CAuthResult result;

            // the launch fails: the event is there, a person who copies the address finishes the login
            f.browser.SetLaunchResult( false );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( PumpUntilAction( f, 1 ) );
            CORE::CString url = f.recorder.Last( CAuthRecorder::KIND_ACTION )->action.url;
            ASSERT_TRUE( !url.IsNULLOrEmpty() && 1 == f.browser.GetOpenCount() && url == f.browser.GetLastUrl() );
            PumpFor( 200 );
            ASSERT_TRUE( !future.IsReady() );
            f.browser.Browse( url );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && "google:sub-1000" == result.accountRef.ToString() );
            f.manager.Unlink( result.accountRef );
            f.browser.SetLaunchResult( true );
            f.browser.ClearHistory();

            // the options say that the application opens the address itself
            AUTH::CAuthOptions own;
            own.preferBrowser = false;
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", own, future ) );
            ASSERT_TRUE( PumpUntilAction( f, 2 ) );
            url = f.recorder.Last( CAuthRecorder::KIND_ACTION )->action.url;
            PumpFor( 100 );
            ASSERT_TRUE( 0 == f.browser.GetOpenCount() && !future.IsReady() );
            f.browser.Browse( url );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            f.manager.Unlink( result.accountRef );
            f.browser.ClearHistory();

            // the engine is set not to open the browser
            AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
            settings.launchBrowser = false;
            f.engine->SetSettings( settings );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( PumpUntilAction( f, 3 ) );
            url = f.recorder.Last( CAuthRecorder::KIND_ACTION )->action.url;
            PumpFor( 100 );
            ASSERT_TRUE( 0 == f.browser.GetOpenCount() );
            f.manager.CancelLogin( "google" );
            for ( UInt32 waited=0; waited<3000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A redirect that the application brings back (custom scheme, manual) is taken when it is the one of the login with its state" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            const char* modes[] = { "custom-scheme", "manual" };
            for ( size_t m=0; m<2; ++m )
            {
                COAuth2Fixture f;
                ASSERT_TRUE( f.ready );
                AUTH::CAuthProviderSettings google = f.LocalSettings();
                google.redirectMode = modes[ m ];
                google.redirectUri = "com.example.game:/oauth2redirect";
                ASSERT_TRUE( f.ConfigureOne( "google", google ) );
                f.browser.SetMode( CTestBrowser::MODE_CAPTURE );
                AUTH::CDescriptorAuthProvider* provider = static_cast< AUTH::CDescriptorAuthProvider* >( f.manager.GetProvider( "google" ) );
                ASSERT_TRUE( GUCEF_NULL != provider );
                if ( GUCEF_NULL == provider )
                    return;

                // nothing waits for it
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/oauth2redirect?code=x&state=y" ) );

                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), future ) );
                for ( UInt32 waited=0; waited<5000 && f.browser.GetRedirects().empty(); ++waited )
                    PumpFor( 1 );
                GUCEF::CORE::CString::StringVector redirects = f.browser.GetRedirects();
                ASSERT_TRUE( 1 == redirects.size() );
                if ( 1 != redirects.size() )
                    return;
                const CORE::CString redirect = redirects[ 0 ];
                ASSERT_TRUE( redirect.StartsWith( "com.example.game:/oauth2redirect?" ) );
                // no listener: the engine opened no port
                ASSERT_TRUE( f.fake.GetLastAuthorizeParameters()[ "redirect_uri" ] == "com.example.game:/oauth2redirect" );

                // not for this login: another address, no state, another state, no code
                CORE::CString query = redirect.SubstrToChar( '?', false, true );
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/other?" + query ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/oauth2redirect" ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/oauth2redirect?code=x" ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/oauth2redirect?code=x&state=forged" ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( "com.example.game:/oauth2redirect?state=" + f.fake.GetLastAuthorizeParameters()[ "state" ] ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( redirect + "&state=twice" ) );
                ASSERT_FALSE( provider->SubmitRedirectUrl( CORE::CString::Empty ) );
                ASSERT_FALSE( future.IsReady() );
                ASSERT_TRUE( 0 == f.fake.GetStats().token );

                ASSERT_TRUE( provider->SubmitRedirectUrl( redirect ) );
                for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                    PumpFor( 1 );
                AUTH::CAuthResult result;
                ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && "google:sub-1000" == result.accountRef.ToString() );
                // once
                ASSERT_FALSE( provider->SubmitRedirectUrl( redirect ) );
                ASSERT_TRUE( f.fake.GetLastTokenParameters()[ "redirect_uri" ] == "com.example.game:/oauth2redirect" );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDeviceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The device flow shows the code, polls at the interval of the provider, backs off on slow_down and ends with the account" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            CFakeBehavior behavior;
            behavior.devicePendingPolls = 2;
            behavior.deviceSlowDownAtPoll = 2;
            behavior.deviceInterval = 1;
            f.fake.SetBehavior( behavior );

            AUTH::CAuthOptions options;
            options.preferDeviceCode = true;
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", options, result ) && "google:sub-1000" == result.accountRef.ToString() );
            ASSERT_TRUE( !result.refreshToken.IsEmpty() && !result.idToken.IsEmpty() );

            // the code is shown, the browser is not opened
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_ACTION ) );
            const CAuthRecorder::SRecord* action = f.recorder.Last( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( GUCEF_NULL != action && AUTH::AUTHACTION_SHOW_DEVICE_CODE == action->action.kind );
            ASSERT_TRUE( "WDJB-MJHT" == action->action.userCode && f.fake.GetBaseUrl() + "/device/activate" == action->action.url );
            ASSERT_TRUE( f.fake.GetBaseUrl() + "/device/activate?user_code=WDJB-MJHT" == action->action.urlWithCode && action->action.expiresAtMs > AUTH::GetEpochTimeInMs() );
            ASSERT_TRUE( 0 == f.browser.GetOpenCount() && 0 == f.fake.GetStats().authorize );

            // the requests
            ASSERT_TRUE( 1 == f.fake.GetStats().device && 3 == f.fake.GetStats().devicePolls );
            CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "urn:ietf:params:oauth:grant-type:device_code" == sent[ "grant_type" ] && sent[ "device_code" ].Length() > 10 && "fake-client-id" == sent[ "client_id" ] );

            // the spacing: the interval, then the interval and five more after slow_down
            GUCEF::vector< UInt64 > times = f.fake.GetDevicePollTimes();
            ASSERT_TRUE( 3 == times.size() );
            if ( 3 == times.size() )
            {
                ASSERT_TRUE( times[ 1 ] - times[ 0 ] >= 20 && times[ 1 ] - times[ 0 ] < 150 );
                ASSERT_TRUE( times[ 2 ] - times[ 1 ] >= 170 );
            }
            PumpFor( 250 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );

            // the interval is the one the provider gives
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            ASSERT_TRUE( g.ConfigureOne( "google", g.LocalSettings() ) );
            behavior = CFakeBehavior();
            behavior.devicePendingPolls = 1;
            behavior.deviceInterval = 4;
            g.fake.SetBehavior( behavior );
            ASSERT_TRUE( g.Login( "google", options, result ) );
            times = g.fake.GetDevicePollTimes();
            ASSERT_TRUE( 2 == times.size() && times[ 1 ] - times[ 0 ] >= 100 );

            // and 5 when it gives none
            behavior.deviceOmitInterval = true;
            g.fake.SetBehavior( behavior );
            g.manager.Unlink( result.accountRef );
            ASSERT_TRUE( g.Login( "google", options, result ) );
            times = g.fake.GetDevicePollTimes();
            ASSERT_TRUE( 4 == times.size() && times[ 3 ] - times[ 2 ] >= 130 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The device flow ends as denied, as expired, as timed out and as cancelled, and then stops polling" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CAuthOptions options;
            options.preferDeviceCode = true;

            CFakeBehavior behavior;
            behavior.devicePendingPolls = 1;
            behavior.deviceOutcome = "denied";
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_DENIED ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().devicePolls );

            behavior.deviceOutcome = "expired";
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_TIMEOUT ) );

            // the code runs out before anyone enters it: five intervals here
            behavior = CFakeBehavior();
            behavior.devicePendingPolls = 100000;
            behavior.deviceExpiresIn = 1;
            f.fake.SetBehavior( behavior );
            UInt64 started = AUTH::GetMonotonicTimeInMs();
            UInt32 pollsBefore = f.fake.GetStats().devicePolls;
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_TIMEOUT ) );
            ASSERT_TRUE( AUTH::GetMonotonicTimeInMs() - started < 4000 && f.fake.GetStats().devicePolls >= pollsBefore + 2 );

            // the overall timeout of the options
            behavior.deviceExpiresIn = 600;
            f.fake.SetBehavior( behavior );
            AUTH::CAuthOptions shortOptions = options;
            shortOptions.timeoutInMs = 500;
            started = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( f.Fails( "google", shortOptions, AUTH::AUTHFAIL_TIMEOUT ) );
            UInt64 took = AUTH::GetMonotonicTimeInMs() - started;
            ASSERT_TRUE( took >= 450 && took < 4000 );

            // cancelled by the requester, polling stops
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            UInt32 failures = f.FailureCount();
            UInt32 actions = f.recorder.Count( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( f.manager.Login( "google", options, future ) );
            ASSERT_TRUE( PumpUntilAction( f, actions + 1 ) );
            PumpFor( 200 );
            ASSERT_TRUE( f.manager.CancelLogin( "google" ) );
            for ( UInt32 waited=0; waited<3000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_FAILED, failures + 1, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == f.LastFailure() );
            PumpFor( 150 );
            UInt32 polls = f.fake.GetStats().devicePolls;
            PumpFor( 400 );
            ASSERT_TRUE( polls == f.fake.GetStats().devicePolls );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );

            // the endpoint refuses the client: no polling at all
            behavior = CFakeBehavior();
            f.fake.SetBehavior( behavior );
            AUTH::CAuthProviderSettings stranger = f.LocalSettings();
            stranger.clientId = "not-our-client";
            ASSERT_TRUE( f.ConfigureOne( "google", stranger ) );
            polls = f.fake.GetStats().devicePolls;
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( polls == f.fake.GetStats().devicePolls );

            // a poll that fails to get through is tried again, a few times
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            behavior.tokenError = "temporarily_unavailable";
            behavior.tokenErrorStatus = 503;
            f.fake.SetBehavior( behavior );
            polls = f.fake.GetStats().devicePolls;
            UInt32 tokens = f.fake.GetStats().token;
            ASSERT_TRUE( f.Fails( "google", options, AUTH::AUTHFAIL_NETWORK ) );
            ASSERT_TRUE( f.fake.GetStats().token >= tokens + 4 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRefreshTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A refresh token gives a new login, the new refresh token replaces it, the old one is not accepted again" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CIAuthProvider* provider = f.manager.GetProvider( "google" );
            ASSERT_TRUE( GUCEF_NULL != provider && 0 != ( provider->GetCapabilities() & AUTH::AUTHCAP_REFRESHABLE ) );
            if ( GUCEF_NULL == provider )
                return;
            CAuthRecorder providerEvents;
            providerEvents.SubscribeTo( provider );

            const AUTH::CAccountRef account( "google", "sub-1000" );
            CORE::CString oldToken = f.fake.IssueRefreshToken( "sub-1000" );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( oldToken ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            AUTH::CAuthResult result;
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( account == result.accountRef && !result.accessToken.IsEmpty() && result.expiresAtMs > AUTH::GetEpochTimeInMs() );
            ASSERT_TRUE( !result.refreshToken.IsEmpty() && result.refreshToken.Reveal() != oldToken );
            ASSERT_TRUE( !f.fake.IsRefreshTokenValid( oldToken ) && f.fake.IsRefreshTokenValid( result.refreshToken.Reveal() ) );
            CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "refresh_token" == sent[ "grant_type" ] && oldToken == sent[ "refresh_token" ] && "fake-client-id" == sent[ "client_id" ] );
            ASSERT_TRUE( 0 == f.browser.GetOpenCount() );

            // the old token again: the provider does not accept it, the reason is that the grant is no good
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( oldToken ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && !AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            PumpUntilRecorded( providerEvents, CAuthRecorder::KIND_FAILED, 1, 2000 );
            ASSERT_TRUE( 1 == providerEvents.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_INVALID_GRANT == providerEvents.Last( CAuthRecorder::KIND_FAILED )->failure.reason );

            // a provider that does not rotate keeps the token the application has
            CFakeBehavior behavior;
            behavior.rotateRefreshTokens = false;
            f.fake.SetBehavior( behavior );
            CORE::CString steady = f.fake.IssueRefreshToken( "sub-1000" );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( steady ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && steady == result.refreshToken.Reveal() && f.fake.IsRefreshTokenValid( steady ) );

            // a renewed id token is validated and has to be of the account that is renewed
            behavior.refreshIssuesIdToken = true;
            f.fake.SetBehavior( behavior );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( steady ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && account == result.accountRef );
            ASSERT_TRUE( AUTH::CAuthProof::KindOidcIdToken == result.proof.kind && !result.proof.data.IsEmpty() && "Fake Player" == result.displayName );
            CORE::CString other = f.fake.IssueRefreshToken( "sub-2000" );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( other ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && !AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            PumpUntilRecorded( providerEvents, CAuthRecorder::KIND_FAILED, 2, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == providerEvents.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            // and one that is wrong in itself
            CFakeTokenSpec spec;
            spec.expiresInSeconds = -3600;
            f.fake.SetTokenSpec( spec );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider->Refresh( account, AUTH::CSecretString( steady ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && !AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            PumpUntilRecorded( providerEvents, CAuthRecorder::KIND_FAILED, 3, 2000 );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == providerEvents.Last( CAuthRecorder::KIND_FAILED )->failure.reason );

            // not possible without a token
            ASSERT_FALSE( provider->Refresh( account, AUTH::CSecretString(), future ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The startup policy renews stored logins; a refused token is dropped, one lost to the network is kept" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthSettings settings;
            settings.SetProviderSettings( "google", f.LocalSettings() );
            settings.lastUsedProviders.push_back( "google" );
            ASSERT_TRUE( f.Configure( settings ) );

            // a stored token that works
            CORE::CString token = f.fake.IssueRefreshToken( "sub-1000" );
            ASSERT_TRUE( f.credentials.Store( "google", "google:sub-1000", AUTH::CSecretString( token ) ) );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.AutoLogin( future ) );
            ASSERT_TRUE( PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_AUTOFINISHED, 1, 15000 ) );
            AUTH::CSecretString stored;
            ASSERT_TRUE( f.credentials.Load( "google", "google:sub-1000", stored ) && stored.Reveal() != token && f.fake.IsRefreshTokenValid( stored.Reveal() ) );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 0 == f.browser.GetOpenCount() );
            AUTH::CAuthResult session;
            ASSERT_TRUE( f.manager.GetSession( AUTH::CAccountRef( "google", "sub-1000" ), session ) && !session.accessToken.IsEmpty() );

            // a token that the provider does not know any more is removed
            CAuthRecorder second;
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthSettings gs;
            gs.SetProviderSettings( "google", g.LocalSettings() );
            gs.lastUsedProviders.push_back( "google" );
            ASSERT_TRUE( g.Configure( gs ) );
            ASSERT_TRUE( g.credentials.Store( "google", "google:sub-1000", AUTH::CSecretString( "revoked-token-value" ) ) );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( g.manager.AutoLogin( future ) );
            ASSERT_TRUE( PumpUntilRecorded( g.recorder, CAuthRecorder::KIND_AUTOFINISHED, 1, 15000 ) );
            ASSERT_FALSE( g.credentials.Load( "google", "google:sub-1000", stored ) );
            ASSERT_TRUE( 0 == g.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );

            // a provider that cannot be reached: the token stays
            COAuth2Fixture h;
            ASSERT_TRUE( h.ready );
            AUTH::CAuthProviderSettings unreachable = h.LocalSettings();
            unreachable.tokenEndpoint = "http://127.0.0.1:1/token";
            AUTH::CAuthSettings hs;
            hs.SetProviderSettings( "google", unreachable );
            hs.lastUsedProviders.push_back( "google" );
            ASSERT_TRUE( h.Configure( hs ) );
            ASSERT_TRUE( h.credentials.Store( "google", "google:sub-1000", AUTH::CSecretString( "kept-token-value" ) ) );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( h.manager.AutoLogin( future ) );
            ASSERT_TRUE( PumpUntilRecorded( h.recorder, CAuthRecorder::KIND_AUTOFINISHED, 1, 15000 ) );
            ASSERT_TRUE( h.credentials.Load( "google", "google:sub-1000", stored ) && "kept-token-value" == stored.Reveal() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logging out revokes the refresh token at the provider, from the endpoint of the discovery document too" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            CORE::CString token = result.refreshToken.Reveal();
            ASSERT_TRUE( f.fake.IsRefreshTokenValid( token ) );
            ASSERT_TRUE( f.manager.Logout( result.accountRef ) );
            ASSERT_TRUE( PumpUntilStat( f.fake, &CFakeStats::revoke, 1, 5000 ) );
            GUCEF::CORE::CString::StringVector revoked = f.fake.GetRevokedTokens();
            ASSERT_TRUE( 1 == revoked.size() && token == revoked[ 0 ] && !f.fake.IsRefreshTokenValid( token ) );
            AUTH::CSecretString none;
            ASSERT_FALSE( f.credentials.Load( "google", result.accountRef.ToString(), none ) );

            // the address from the discovery document
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthProviderSettings discovered = g.DiscoverySettings();
            ASSERT_TRUE( g.ConfigureOne( "google", discovered ) );
            ASSERT_TRUE( g.Login( "google", AUTH::CAuthOptions(), result ) );
            token = result.refreshToken.Reveal();
            ASSERT_TRUE( g.manager.Unlink( result.accountRef ) );
            ASSERT_TRUE( PumpUntilStat( g.fake, &CFakeStats::revoke, 1, 5000 ) );
            ASSERT_TRUE( !g.fake.IsRefreshTokenValid( token ) );
            ASSERT_TRUE( 1 == g.fake.GetStats().discovery );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, AUTH::CDescriptorAuthProvider, MT::CMutex > TDescriptorProviderFactory;

/** A provider of a descriptor of the test, added to the manager of the fixture; the settings of the manager reach it */
static AUTH::CDescriptorAuthProvider*
AddProvider( COAuth2Fixture& f, TDescriptorProviderFactory& factory, const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings )
{GUCEF_TRACE;

    TDescriptorProviderFactory::TProductPtr product = factory.Create();
    AUTH::CDescriptorAuthProvider* provider = static_cast< AUTH::CDescriptorAuthProvider* >( product.GetPointerAlways() );
    provider->SetDescriptor( descriptor );
    if ( !f.manager.AddProvider( product ) )
        return GUCEF_NULL;
    AUTH::CAuthSettings all = f.manager.GetSettings();
    all.SetProviderSettings( descriptor.name, settings );
    f.manager.SetSettings( all );
    return provider;
}

/*-------------------------------------------------------------------------*/

static AUTH::CAuthProviderDescriptor
CopyOf( const CORE::CString& builtIn, const CORE::CString& name )
{GUCEF_TRACE;

    AUTH::CAuthProviderDescriptor descriptor;
    AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( builtIn, descriptor );
    descriptor.name = name;
    return descriptor;
}

/*-------------------------------------------------------------------------*/

static void
PerformBodyStyleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A client secret that is configured for Google (the Auth section of the configuration) is sent at the token endpoint, for the code and for a refresh, and is not logged" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );

            // the section as the game's configuration has it: the secret of a Desktop app client sits next to the client id
            AUTH::CAuthProviderSettings local = f.LocalSettings();
            CORE::CDataNode auth( "Auth" );
            CORE::CDataNode* providers = auth.AddChild( "Providers", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* googleNode = providers->AddChild( "google", GUCEF_DATATYPE_OBJECT );
            local.SaveConfig( *googleNode );
            googleNode->SetAttribute( "clientSecret", "GOCSPX-desktop-secret-0042" );
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( settings.LoadConfig( auth ) );
            ASSERT_TRUE( f.Configure( settings ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "google" ) );

            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
            CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "authorization_code" == sent[ "grant_type" ] && "fake-client-id" == sent[ "client_id" ] );
            ASSERT_TRUE( "GOCSPX-desktop-secret-0042" == sent[ "client_secret" ] );
            ASSERT_TRUE( f.fake.GetLastTokenAuthorization().IsNULLOrEmpty() );

            // the refresh carries it as well
            AUTH::CIAuthProvider* provider = f.manager.GetProvider( "google" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CString token = f.fake.IssueRefreshToken( "sub-1000" );
            ASSERT_TRUE( GUCEF_NULL != provider && provider->Refresh( AUTH::CAccountRef( "google", "sub-1000" ), AUTH::CSecretString( token ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "refresh_token" == sent[ "grant_type" ] && "GOCSPX-desktop-secret-0042" == sent[ "client_secret" ] );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Token requests are forms or JSON with camel case names as the descriptor says; the client secret goes in the body or in a Basic header" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            TDescriptorProviderFactory factory;
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthResult result;

            // JSON bodies
            AUTH::CAuthProviderDescriptor jsonDescriptor = CopyOf( "google", "jsonprov" );
            jsonDescriptor.jsonRequestBodies = true;
            AUTH::CAuthProviderSettings settings = f.LocalSettings();
            ASSERT_TRUE( GUCEF_NULL != AddProvider( f, factory, jsonDescriptor, settings ) );
            ASSERT_TRUE( f.Login( "jsonprov", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
            ASSERT_TRUE( "application/json" == f.fake.GetLastTokenContentType() );
            CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "authorization_code" == sent[ "grant_type" ] && !sent[ "code_verifier" ].IsNULLOrEmpty() && "fake-client-id" == sent[ "client_id" ] );

            // the secret in the body (the way of Google for installed applications)
            AUTH::CAuthProviderDescriptor postDescriptor = CopyOf( "google", "postprov" );
            postDescriptor.clientAuthMethod = "post";
            AUTH::CAuthProviderSettings withSecret = f.LocalSettings();
            withSecret.clientSecret.Set( "the-client-secret" );
            ASSERT_TRUE( GUCEF_NULL != AddProvider( f, factory, postDescriptor, withSecret ) );
            ASSERT_TRUE( f.Login( "postprov", AUTH::CAuthOptions(), result ) );
            sent = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( "the-client-secret" == sent[ "client_secret" ] && f.fake.GetLastTokenAuthorization().IsNULLOrEmpty() );
            ASSERT_TRUE( "application/x-www-form-urlencoded" == f.fake.GetLastTokenContentType() );

            // the secret in a Basic header (the way of Epic)
            AUTH::CAuthProviderDescriptor basicDescriptor = CopyOf( "google", "basicprov" );
            basicDescriptor.clientAuthMethod = "basic";
            ASSERT_TRUE( GUCEF_NULL != AddProvider( f, factory, basicDescriptor, withSecret ) );
            ASSERT_TRUE( f.Login( "basicprov", AUTH::CAuthOptions(), result ) );
            sent = f.fake.GetLastTokenParameters();
            CORE::CString expected = "Basic " + CORE::Base64Encode( "fake-client-id:the-client-secret", 32 );
            ASSERT_TRUE( sent.end() == sent.find( "client_secret" ) && expected == f.fake.GetLastTokenAuthorization() );

            // the refresh uses the same style
            AUTH::CIAuthProvider* provider = f.manager.GetProvider( "basicprov" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CString token = f.fake.IssueRefreshToken( "sub-1000" );
            ASSERT_TRUE( GUCEF_NULL != provider && provider->Refresh( AUTH::CAccountRef( "google", "sub-1000" ), AUTH::CSecretString( token ), future ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( future.IsReady() && expected == f.fake.GetLastTokenAuthorization() && "refresh_token" == f.fake.GetLastTokenParameters()[ "grant_type" ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformOAuth2OnlyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A provider without an id token (Facebook, Amazon): the profile gives the account, the access token is the proof" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            CFakeBehavior behavior;
            behavior.issueIdToken = false;
            behavior.requirePkce = false;
            f.fake.SetBehavior( behavior );

            AUTH::CAuthProviderSettings facebook = f.LocalSettings();
            facebook.userInfoEndpoint = f.fake.GetBaseUrl() + "/me";
            ASSERT_TRUE( f.ConfigureOne( "facebook", facebook ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "facebook", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "facebook:sub-1000" == result.accountRef.ToString() && "Fb Player" == result.displayName && "https://pictures.invalid/fb.png" == result.avatarUrl );
            ASSERT_TRUE( result.idToken.IsEmpty() && !result.accessToken.IsEmpty() && AUTH::CAuthProof::KindOAuth2AccessToken == result.proof.kind && result.proof.data.Reveal() == result.accessToken.Reveal() );
            // no PKCE and no nonce for this one
            CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
            ASSERT_TRUE( asked.end() == asked.find( "code_challenge" ) && asked.end() == asked.find( "nonce" ) && "public_profile" == asked[ "scope" ] );
            CORE::CString::StringMap tokenRequest = f.fake.GetLastTokenParameters();
            ASSERT_TRUE( tokenRequest.end() == tokenRequest.find( "code_verifier" ) && "authorization_code" == tokenRequest[ "grant_type" ] );
            ASSERT_TRUE( 1 == f.fake.GetStats().userinfo && 0 == f.fake.GetStats().jwks );

            // without a profile there is no account
            f.manager.Unlink( result.accountRef );
            behavior.userinfoFails = true;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Fails( "facebook", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );

            // Amazon
            behavior.userinfoFails = false;
            behavior.requirePkce = true;
            f.fake.SetBehavior( behavior );
            AUTH::CAuthProviderSettings amazon = f.LocalSettings();
            amazon.userInfoEndpoint = f.fake.GetBaseUrl() + "/profile";
            ASSERT_TRUE( f.ConfigureOne( "amazon", amazon ) );
            ASSERT_TRUE( f.Login( "amazon", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "amazon:sub-1000" == result.accountRef.ToString() && "Amazon Player" == result.displayName && AUTH::CAuthProof::KindOAuth2AccessToken == result.proof.kind );
            ASSERT_TRUE( !f.fake.GetLastAuthorizeParameters()[ "code_challenge" ].IsNULLOrEmpty() );

            // the token type has to be bearer
            ASSERT_TRUE( 2 == f.fake.GetStats().userinfo || f.fake.GetStats().userinfo >= 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static AUTH::CAuthProviderSettings
RelayOpenId2Settings( COAuth2Fixture& f, const CORE::CString& relayUrl );

/*-------------------------------------------------------------------------*/

static void
PerformOpenId2Tests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0: a person who takes nine minutes is let in: the age of the response nonce is the time since the provider made the assertion, not since the login began" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            struct SGuard { ~SGuard() { AUTH::SetMonotonicClockOffsetInMs( 0 ); } } guard;
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );
            ASSERT_TRUE( 300 == f.engine->GetSettings().openId2MaxAgeInSeconds );
            f.browser.SetMode( CTestBrowser::MODE_CAPTURE );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "testopenid", AUTH::CAuthOptions(), future ) );
            for ( UInt32 waited=0; waited<5000 && f.browser.GetRedirects().empty(); ++waited )
                PumpFor( 1 );
            REQUIRE_REDIRECT( f );
            const CORE::CString redirect = f.browser.GetRedirects()[ 0 ];

            // the person has been at the login page of the provider for nine minutes; the assertion is made when the person is done (the fake made it a moment ago)
            AUTH::SetMonotonicClockOffsetInMs( 540000 );
            PumpFor( 100 );
            ASSERT_FALSE( future.IsReady() );
            WEB::CHttpRequestData request;
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            request.SetUrl( redirect );
            ASSERT_TRUE( PumpedExecute( request, response, info, CORE::CString::Empty, 0, 5000 ) );
            for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                PumpFor( 1 );
            AUTH::CAuthResult result;
            ASSERT_TRUE( future.IsReady() && AUTH::CAuthResult::TryGetFromFuture( future, result ) && "testopenid:4200000042" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            AUTH::SetMonotonicClockOffsetInMs( 0 );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 over https through the curl backend: the engine setting caBundlePath names the authority that is trusted, another authority ends the login with a message about the certificate" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
        GUCEF_TESTFW_SKIP_IF( !WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ), "the plugin for https is not loaded" );
        try
        {
            TESTFW::CTlsTestCerts certs;
            ASSERT_TRUE( certs.IsReady() );

            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            TESTFW::CTlsTestTerminator terminator;
            GUCEF_TESTFW_SKIP_IF( !terminator.Start( certs.GetPath( "good.crt" ), certs.GetPath( "good.key" ), f.fake.GetPort() ), "no TLS server library" );
            const CORE::CString secureBase = "https://localhost:" + CORE::ToString( terminator.GetPort() );

            // the browser of the test (a client of its own) trusts the authority of the test through the environment; the engine is told by its settings,
            // which come before the environment: it trusts the authority of its setting and no other
            const CORE::CString previousTrust = CORE::GetEnv( "SSL_CERT_FILE" );
            CORE::SetEnv( "SSL_CERT_FILE", certs.GetPath( "ca.crt" ) );
            const AUTH::COAuth2EngineSettings saved = f.engine->GetSettings();
            AUTH::COAuth2EngineSettings settings = saved;
            settings.caBundlePath = certs.GetPath( "ca.crt" );
            f.engine->SetSettings( settings );

            // a POST of a form to the provider as the check of an assertion is: the curl backend carries it, over the connection the terminator made
            {
                AUTH::COAuth2Http http( f.engine->GetSettings() );
                AUTH::CAuthHttpRequest request;
                ASSERT_TRUE( request.Post( secureBase + "/openid/login", "application/x-www-form-urlencoded", "openid.mode=check_authentication&openid.assoc_handle=1234&openid.sig=ab%2Bc%2FdE%3D", f.engine->GetSettings() ) );
                AUTH::CAuthHttpResponse response;
                ASSERT_TRUE( http.Execute( request, response ) );
                ASSERT_TRUE( response.transportOk && 200 == response.statusCode && "curl" == response.backendName );
                ASSERT_TRUE( "application/x-www-form-urlencoded" == f.fake.GetLastOpenIdCheckContentType() );
                CORE::CString::StringMap form = f.fake.GetLastOpenIdCheck();
                ASSERT_TRUE( "check_authentication" == form[ "openid.mode" ] && "ab+c/dE=" == form[ "openid.sig" ] );
                http.ReleaseBackends();
            }

            // the login end to end, the provider is at a https address
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

            // another authority: the certificate of the provider is not trusted, the check does not happen and the login ends with a sentence that says so
            f.manager.Unlink( result.accountRef );
            settings.caBundlePath = certs.GetPath( "other_ca.crt" );
            f.engine->SetSettings( settings );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
            const CAuthRecorder::SRecord* failed = f.recorder.Last( CAuthRecorder::KIND_FAILED );
            ASSERT_TRUE( GUCEF_NULL != failed );
            if ( GUCEF_NULL != failed )
                ASSERT_TRUE( Contains( failed->failure.message, "certificate" ) && Contains( failed->failure.message, "localhost" ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );

            // a bundle that does not exist is no reason to skip the check
            f.manager.Unlink( result.accountRef );
            settings.caBundlePath = certs.GetPath( "does-not-exist.crt" );
            f.engine->SetSettings( settings );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );

            f.engine->SetSettings( saved );
            CORE::SetEnv( "SSL_CERT_FILE", previousTrust );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0: a check at the provider that is answered with an error status ends the login as a network failure that names the status" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );

            CFakeBehavior unavailable;
            unavailable.openIdCheckStatus = 503;
            f.fake.SetBehavior( unavailable );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_NETWORK ) );
            const CAuthRecorder::SRecord* failed = f.recorder.Last( CAuthRecorder::KIND_FAILED );
            ASSERT_TRUE( GUCEF_NULL != failed );
            ASSERT_TRUE( Contains( failed->failure.message, "HTTP status 503" ) && Contains( failed->failure.message, "127.0.0.1" ) );
            // nothing but the host: no path, no query
            ASSERT_FALSE( Contains( failed->failure.message, "openid" ) || Contains( failed->failure.message, "state" ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdCheck );

            // a 4xx is not the provider being down, the assertion is not accepted
            unavailable.openIdCheckStatus = 400;
            f.fake.SetBehavior( unavailable );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );

            // and once the provider is well again the same login works
            CFakeBehavior well;
            f.fake.SetBehavior( well );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) && "testopenid:4200000042" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0: OpenID 2.0 in the browser with the loopback return address as realm, checked at the provider, giving testopenid:<id>" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "testopenid" ) );

            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "testopenid:4200000042" == result.accountRef.ToString() && "testopenid" == result.providerName );
            ASSERT_TRUE( AUTH::CAuthProof::KindOpenId2Assertion == result.proof.kind && !result.proof.data.IsEmpty() && result.refreshToken.IsEmpty() && result.accessToken.IsEmpty() );

            // what the browser was sent to
            CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
            ASSERT_TRUE( "checkid_setup" == asked[ "openid.mode" ] && AUTH::COpenId2Assertion::IdentifierSelect == asked[ "openid.identity" ] && AUTH::COpenId2Assertion::IdentifierSelect == asked[ "openid.claimed_id" ] );
            CORE::CString returnTo = asked[ "openid.return_to" ];
            ASSERT_TRUE( returnTo.StartsWith( "http://127.0.0.1:" ) && Contains( returnTo, "/callback?state=" ) && asked[ "openid.realm" ].Length() > 10 && returnTo.StartsWith( asked[ "openid.realm" ] ) );
            ASSERT_TRUE( asked[ "openid.realm" ] == AUTH::CAuthUrl::GetOrigin( returnTo ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdLogin && 1 == f.fake.GetStats().openIdCheck && 0 == f.fake.GetStats().token );
            // the proof is the assertion as it came
            AUTH::COpenId2Assertion assertion;
            ASSERT_TRUE( assertion.LoadFromQueryString( result.proof.data.Reveal() ) && assertion.Get( "openid.return_to" ) == returnTo && "id_res" == assertion.Get( "openid.mode" ) );

            // what the provider was asked to check (OpenID 2.0, 11.4.2.1): a form with every field of the assertion back as it was, except the mode
            CORE::CString::StringMap checked = f.fake.GetLastOpenIdCheck();
            ASSERT_TRUE( "application/x-www-form-urlencoded" == f.fake.GetLastOpenIdCheckContentType() );
            ASSERT_TRUE( "check_authentication" == checked[ "openid.mode" ] && assertion.fields.size() == checked.size() );
            for ( CORE::CString::StringMap::const_iterator field = assertion.fields.begin(); field != assertion.fields.end(); ++field )
                if ( (*field).first != "openid.mode" )
                    ASSERT_TRUE( checked.find( (*field).first ) != checked.end() && checked[ (*field).first ] == (*field).second );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            ASSERT_TRUE( ListenerIsGone( returnTo ) );

            // an assertion that is wrong in any way is refused
            struct SCase { const char* name; };
            for ( UInt32 i=0; i<7; ++i )
            {
                f.manager.Unlink( result.accountRef );
                CFakeBehavior behavior;
                const char* name = "";
                switch ( i )
                {
                    case 0 : name = "the provider says it is not valid"; behavior.openIdAlwaysInvalid = true; break;
                    case 1 : name = "a claimed id with upper case letters"; behavior.openIdClaimedIdOverride = f.fake.GetBaseUrl() + "/openid/id/ABC"; break;
                    case 2 : name = "a claimed id of another place"; behavior.openIdClaimedIdOverride = "https://openid.example.org/openid/id/4200000042"; break;
                    case 3 : name = "a claimed id with a path after it"; behavior.openIdClaimedIdOverride = f.fake.GetBaseUrl() + "/openid/id/12345/evil"; break;
                    case 4 : name = "another return address"; behavior.openIdReturnToOverride = "http://127.0.0.1:1/callback?state=other"; break;
                    case 5 : name = "an old nonce"; behavior.openIdNonceAgeInSeconds = 3600; break;
                    case 6 : name = "a return address that is not signed"; behavior.openIdOmitSignedReturnTo = true; break;
                }
                f.fake.SetBehavior( behavior );
                bool failed = f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE );
                if ( !failed )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The assertion was not refused: " << name );
                ASSERT_TRUE( failed );
            }
            // the first four reached the provider, the ones that fail on their own did not need to
            ASSERT_TRUE( f.fake.GetStats().openIdCheck >= 2 );

            // cancelled at the provider: the answer is openid.mode=cancel
            CFakeBehavior plain;
            f.fake.SetBehavior( plain );
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) && "testopenid:4200000042" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0: the profile of the provider decides the subject of the account and the name that is in the result, and what an assertion carries besides is passed on to the check" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );

            // the profile reads the name from an extension of the assertion
            CFakeBehavior behavior;
            behavior.openIdName = "Test Person";
            behavior.openIdSubject = "abc123";
            f.fake.SetBehavior( behavior );
            TestOpenId2Profile().readsName = true;
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "testopenid:abc123" == result.accountRef.ToString() && "Test Person" == result.displayName );
            // the extension went back to the provider with the rest of the assertion
            ASSERT_TRUE( "Test Person" == f.fake.GetLastOpenIdCheck()[ "openid.test_name" ] );

            // a profile that does not read a name: the result has none, the login is the same
            TestOpenId2Profile().readsName = false;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) && "testopenid:abc123" == result.accountRef.ToString() && result.displayName.IsNULLOrEmpty() );
            TestOpenId2Profile().Reset();
        }
        catch ( ... )
        {
            TestOpenId2Profile().Reset();
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0: a provider whose profile does not take a relay is not offered with one, and one without a profile is not offered and its assertions are refused" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::COpenId2ProfileRegistry& profiles = AUTH::CAuthGlobal::Instance()->GetOpenId2Profiles();

            // the realm policy of the profile
            ASSERT_TRUE( f.ConfigureOne( "testopenid", RelayOpenId2Settings( f, "https://login.relay-test.example/openid-return" ) ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "testopenid" ) );
            TestOpenId2Profile().relayAllowed = false;
            ASSERT_FALSE( f.manager.IsProviderOffered( "testopenid" ) );
            CORE::CFutureResult none( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.manager.Login( "testopenid", AUTH::CAuthOptions(), none ) );
            // without a relay the same profile is fine
            AUTH::CAuthProviderSettings plain;
            plain.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", plain ) && f.manager.IsProviderOffered( "testopenid" ) );
            TestOpenId2Profile().relayAllowed = true;

            // a login, and then the backend goes: the proof of it is refused like the proof of a provider that is not known
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
            AUTH::CIAuthProvider* provider = f.manager.GetProvider( "testopenid" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            AUTH::CAuthResult verified;
            const UInt32 checksBefore = f.fake.GetStats().openIdCheck;
            ASSERT_TRUE( profiles.Unregister( "testopenid" ) );
            ASSERT_TRUE( GUCEF_NULL == profiles.Find( "testopenid" ) );
            ASSERT_FALSE( provider->IsAvailable() );
            ASSERT_FALSE( f.manager.IsProviderOffered( "testopenid" ) );
            ASSERT_FALSE( f.manager.Login( "testopenid", AUTH::CAuthOptions(), none ) );
            CFakeBehavior fresh;
            f.fake.SetBehavior( fresh );
            CORE::CString assertionText = result.proof.data.Reveal();
            ASSERT_FALSE( provider->Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, assertionText ), verified ) );
            ASSERT_TRUE( checksBefore == f.fake.GetStats().openIdCheck );     // nobody asked the provider
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetOAuth2Engine()->FindOpenId2Profile( MakeTestOpenId2Descriptor() ) == GUCEF_NULL );

            // the backend is back
            ASSERT_TRUE( profiles.Register( &TestOpenId2Profile() ) );
            ASSERT_FALSE( profiles.Register( &TestOpenId2Profile() ) );     // once
            ASSERT_TRUE( provider->IsAvailable() && f.manager.IsProviderOffered( "testopenid" ) );
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) && "testopenid:4200000042" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            TestOpenId2Profile().Reset();
            if ( GUCEF_NULL == AUTH::CAuthGlobal::Instance()->GetOpenId2Profiles().Find( "testopenid" ) )
                AUTH::CAuthGlobal::Instance()->GetOpenId2Profiles().Register( &TestOpenId2Profile() );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The settings of the OpenID 2.0 provider for the fake, with the relay page on a domain that exists in no network: the test browser plays the page */
static AUTH::CAuthProviderSettings
RelayOpenId2Settings( COAuth2Fixture& f, const CORE::CString& relayUrl )
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings openId2;
    openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
    openId2.parameters[ "relayUrl" ] = relayUrl;
    return openId2;
}

/*-------------------------------------------------------------------------*/

static void
PerformOpenId2RelayTests( void )
{GUCEF_TRACE;

    const CORE::CString relayUrl = "https://login.relay-test.example/openid-return";

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 with a relay: realm and return address are on the relay host with the port and state in the query, the relay page takes the browser to the listener, the assertion is checked at the provider" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CLogCapture capture;
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "testopenid", RelayOpenId2Settings( f, relayUrl ) ) );
            ASSERT_TRUE( f.manager.IsProviderOffered( "testopenid" ) );
            f.browser.SetMode( CTestBrowser::MODE_RELAY );

            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
            ASSERT_TRUE( "testopenid:4200000042" == result.accountRef.ToString() && "testopenid" == result.providerName );
            ASSERT_TRUE( AUTH::CAuthProof::KindOpenId2Assertion == result.proof.kind && !result.proof.data.IsEmpty() && result.refreshToken.IsEmpty() && result.accessToken.IsEmpty() );

            // what the browser was sent to: the player sees the host of the realm
            CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
            ASSERT_TRUE( "checkid_setup" == asked[ "openid.mode" ] && AUTH::COpenId2Assertion::IdentifierSelect == asked[ "openid.identity" ] && AUTH::COpenId2Assertion::IdentifierSelect == asked[ "openid.claimed_id" ] );
            ASSERT_TRUE( "https://login.relay-test.example/" == asked[ "openid.realm" ] );
            CORE::CString returnTo = asked[ "openid.return_to" ];
            ASSERT_TRUE( returnTo.StartsWith( "https://login.relay-test.example/openid-return?port=" ) && returnTo.StartsWith( asked[ "openid.realm" ] ) );
            ASSERT_FALSE( Contains( returnTo, "127.0.0.1" ) || Contains( returnTo, "localhost" ) || Contains( returnTo, "/callback" ) || Contains( asked[ "openid.realm" ], "127.0.0.1" ) );

            // port of the listener and state in the query of the return address, nothing else
            CORE::CString scheme, host, path, query;
            UInt16 port = 0;
            ASSERT_TRUE( AUTH::CAuthUrl::Split( returnTo, scheme, host, port, path, query ) );
            CORE::CString::StringMap inReturnTo;
            bool duplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( query, inReturnTo, duplicate ) && !duplicate && 2 == inReturnTo.size() );
            UInt32 listenerPort = CORE::StringToUInt32( inReturnTo[ "port" ] );
            ASSERT_TRUE( listenerPort >= 1024 && listenerPort <= 65535 && CORE::ToString( listenerPort ) == inReturnTo[ "port" ] );
            ASSERT_TRUE( AUTH::COpenId2Relay::IsValidState( inReturnTo[ "s" ] ) && 43 == inReturnTo[ "s" ].Length() );
            ASSERT_TRUE( AUTH::COpenId2Relay::BuildReturnTo( "https://login.relay-test.example/openid-return", (UInt16) listenerPort, inReturnTo[ "s" ] ) == returnTo );

            // the relay page took the answer of the provider to the listener of this login, with every parameter
            CORE::CString::StringVector relayed = f.browser.GetRelayed();
            ASSERT_TRUE( 1 == relayed.size() && 1 == f.browser.GetStatuses().size() && 200 == f.browser.GetStatuses()[ 0 ] );
            ASSERT_TRUE( relayed[ 0 ].StartsWith( "http://127.0.0.1:" + CORE::ToString( listenerPort ) + "/callback?port=" + CORE::ToString( listenerPort ) + "&s=" + inReturnTo[ "s" ] + "&openid." ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdLogin && 1 == f.fake.GetStats().openIdCheck && 0 == f.fake.GetStats().token );

            // the proof is the assertion as it came, with the return address the login sent
            AUTH::COpenId2Assertion assertion;
            ASSERT_TRUE( assertion.LoadFromQueryString( result.proof.data.Reveal() ) && assertion.Get( "openid.return_to" ) == returnTo && "id_res" == assertion.Get( "openid.mode" ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            ASSERT_TRUE( ListenerIsGone( "http://127.0.0.1:" + CORE::ToString( listenerPort ) + "/callback" ) );
            PumpFor( 250 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );

            // the state and the port are new every time
            CORE::CString::StringSet states;
            states.insert( inReturnTo[ "s" ] );
            for ( UInt32 i=0; i<3; ++i )
            {
                f.manager.Unlink( result.accountRef );
                ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
                CORE::CString again = f.fake.GetLastAuthorizeParameters()[ "openid.return_to" ];
                AUTH::CAuthUrl::Split( again, scheme, host, port, path, query );
                inReturnTo.clear();
                AUTH::CAuthUrl::ParseQuery( query, inReturnTo, duplicate );
                states.insert( inReturnTo[ "s" ] );
            }
            ASSERT_TRUE( 4 == states.size() && 4 == f.fake.GetStats().openIdCheck );

            // nothing of the answer, the state or the nonce ends up in a log
            const CORE::CString logText = capture.Text();
            for ( CORE::CString::StringSet::const_iterator s = states.begin(); s != states.end(); ++s )
                ASSERT_FALSE( Contains( logText, *s ) );
            ASSERT_FALSE( Contains( logText, assertion.Get( "openid.response_nonce" ) ) || Contains( logText, assertion.Get( "openid.sig" ) ) || Contains( logText, assertion.Get( "openid.assoc_handle" ) + "&" ) );
            ASSERT_FALSE( Contains( logText, "openid.sig=" + assertion.Get( "openid.sig" ) ) || Contains( logText, EncodeUrlText( assertion.Get( "openid.sig" ) ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 with a relay: an assertion with another port, state, host, path, scheme, extra query or the loopback form as return address is refused, before the provider is asked" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "testopenid", RelayOpenId2Settings( f, relayUrl ) ) );
            f.browser.SetMode( CTestBrowser::MODE_RELAY );

            const char* edits[] = { "port", "state", "host", "path", "scheme", "extra", "loopback" };
            for ( UInt32 i=0; i<sizeof( edits ) / sizeof( edits[ 0 ] ); ++i )
            {
                CFakeBehavior behavior;
                behavior.openIdReturnToEdit = edits[ i ];
                f.fake.SetBehavior( behavior );
                UInt32 checksBefore = f.fake.GetStats().openIdCheck;
                bool failed = f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE );
                if ( !failed )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The assertion was not refused: " << edits[ i ] );
                ASSERT_TRUE( failed );
                // the relay page did its work and the listener took the callback, the application refused the answer
                ASSERT_TRUE( checksBefore == f.fake.GetStats().openIdCheck );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED != f.manager.GetLoginState() );
            }
            ASSERT_TRUE( sizeof( edits ) / sizeof( edits[ 0 ] ) == f.browser.GetRelayed().size() );
            ASSERT_TRUE( 0 == f.fake.GetStats().openIdCheck );

            // the provider is still asked, and has the last word, for an assertion with the right return address
            CFakeBehavior invalid;
            invalid.openIdAlwaysInvalid = true;
            f.fake.SetBehavior( invalid );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdCheck );

            // and without the trouble the same login works
            f.fake.SetBehavior( CFakeBehavior() );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) && "testopenid:4200000042" == result.accountRef.ToString() );
            ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 with a relay: a relay page that changes the state or its name does not get the listener to take the answer; cancel at the provider ends the login as denied" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "testopenid", RelayOpenId2Settings( f, relayUrl ) ) );

            const CTestBrowser::EMode modes[] = { CTestBrowser::MODE_RELAY_WRONG_STATE, CTestBrowser::MODE_RELAY_STATE_RENAMED };
            for ( UInt32 i=0; i<2; ++i )
            {
                f.browser.ClearHistory();
                f.browser.SetMode( modes[ i ] );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.Login( "testopenid", AUTH::CAuthOptions(), future ) );
                for ( UInt32 waited=0; waited<3000 && f.browser.GetStatuses().empty(); ++waited )
                    PumpFor( 1 );
                ASSERT_TRUE( 1 == f.browser.GetStatuses().size() && 400 == f.browser.GetStatuses()[ 0 ] );
                PumpFor( 300 );
                ASSERT_TRUE( !future.IsReady() && f.manager.IsLoginPending( "testopenid" ) && 0 == f.fake.GetStats().openIdCheck );
                CORE::CString::StringMap inReturnTo;
                CORE::CString scheme, host, path, query;
                UInt16 port = 0;
                bool duplicate = false;
                AUTH::CAuthUrl::Split( f.fake.GetLastAuthorizeParameters()[ "openid.return_to" ], scheme, host, port, path, query );
                AUTH::CAuthUrl::ParseQuery( query, inReturnTo, duplicate );
                ASSERT_TRUE( f.manager.CancelLogin( "testopenid" ) );
                for ( UInt32 waited=0; waited<3000 && !future.IsReady(); ++waited )
                    PumpFor( 1 );
                ASSERT_TRUE( future.IsReady() );
                PumpUntilRecorded( f.recorder, CAuthRecorder::KIND_FAILED, i + 1, 2000 );
                ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == f.LastFailure() );
                ASSERT_TRUE( ListenerIsGone( "http://127.0.0.1:" + inReturnTo[ "port" ] + "/callback" ) );
            }

            // the player cancels at the provider: the answer is openid.mode=cancel and goes through the relay page like any other
            f.browser.ClearHistory();
            f.browser.SetMode( CTestBrowser::MODE_RELAY );
            CFakeBehavior cancel;
            cancel.openIdCancel = true;
            f.fake.SetBehavior( cancel );
            ASSERT_TRUE( f.Fails( "testopenid", AUTH::CAuthOptions(), AUTH::AUTHFAIL_DENIED ) );
            ASSERT_TRUE( 1 == f.browser.GetRelayed().size() && 0 == f.fake.GetStats().openIdCheck );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: OpenID 2.0 without a relay keeps the loopback address as realm and return address, with no relay setting and with an empty one; a relay address that is not acceptable does not make a relay" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            const CORE::CString relayValues[] = { "no setting", "", "http://login.relay-test.example/openid-return", "https://user@login.relay-test.example/openid-return", "https://login.relay-test.example:8443/openid-return", "https://login.relay-test.example/../openid-return" };
            UInt32 checks = 0;
            for ( size_t i=0; i<sizeof( relayValues ) / sizeof( relayValues[ 0 ] ); ++i )
            {
                AUTH::CAuthProviderSettings openId2;
                openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
                if ( 0 != i )
                    openId2.parameters[ "relayUrl" ] = relayValues[ i ];
                ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );

                // an address that is not acceptable refuses the settings of the provider as a whole: the provider keeps what it had
                AUTH::CAuthProviderSettings configured;
                ASSERT_TRUE( i < 2 ? f.manager.GetSettings().GetProviderSettings( "testopenid", configured ) : !f.manager.GetSettings().GetProviderSettings( "testopenid", configured ) );

                f.browser.SetMode( CTestBrowser::MODE_FOLLOW );
                f.browser.ClearHistory();
                AUTH::CAuthResult result;
                ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), result ) );
                CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
                CORE::CString returnTo = asked[ "openid.return_to" ];
                ASSERT_TRUE( returnTo.StartsWith( "http://127.0.0.1:" ) && Contains( returnTo, "/callback?state=" ) && asked[ "openid.realm" ] == AUTH::CAuthUrl::GetOrigin( returnTo ) );
                ASSERT_TRUE( 0 == f.browser.GetRelayed().size() && ++checks == f.fake.GetStats().openIdCheck );
                f.manager.Unlink( result.accountRef );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPolicyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: An http address is acceptable only when the engine setting allows it and the host is a loopback address" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            AUTH::COAuth2Engine* engine = AUTH::CAuthGlobal::Instance()->GetOAuth2Engine();
            AUTH::COAuth2EngineSettings saved = engine->GetSettings();
            AUTH::CAuthProviderDescriptor google;
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( "google", google );

            AUTH::CAuthProviderSettings https;
            https.clientId = "client";
            ASSERT_TRUE( engine->IsSupported( google, https ) );
            AUTH::CAuthProviderSettings noClient;
            ASSERT_TRUE( engine->IsSupported( google, noClient ) == false );

            AUTH::CAuthProviderSettings http = https;
            http.tokenEndpoint = "http://127.0.0.1:4000/token";
            http.authorizationEndpoint = "http://127.0.0.1:4000/authorize";
            http.jwksUri = "http://127.0.0.1:4000/jwks";
            AUTH::COAuth2EngineSettings settings = saved;
            ASSERT_FALSE( saved.allowInsecureHttp );
            settings.allowInsecureHttp = false;
            engine->SetSettings( settings );
            ASSERT_FALSE( engine->IsSupported( google, http ) );
            settings.allowInsecureHttp = true;
            engine->SetSettings( settings );
            ASSERT_TRUE( engine->IsSupported( google, http ) );

            // other hosts stay refused, for every address of the provider
            AUTH::CAuthProviderSettings remote = http;
            remote.tokenEndpoint = "http://accounts.example/token";
            ASSERT_FALSE( engine->IsSupported( google, remote ) );
            remote = http;
            remote.jwksUri = "http://192.168.1.10/jwks";
            ASSERT_FALSE( engine->IsSupported( google, remote ) );
            remote = http;
            remote.issuer = "http://accounts.example";
            ASSERT_FALSE( engine->IsSupported( google, remote ) );
            remote = http;
            remote.authorizationEndpoint = "http://localhost.evil.example/authorize";
            ASSERT_FALSE( engine->IsSupported( google, remote ) );

            // the manager does not offer what the engine cannot run
            AUTH::CAuthManager manager;
            AUTH::CMemoryCredentialStore credentials;
            manager.SetCredentialStore( &credentials );
            AUTH::CAuthSettings all;
            all.SetProviderSettings( "google", http );
            CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
            all.SaveConfig( config );
            ASSERT_TRUE( manager.LoadConfig( config ) && manager.IsProviderOffered( "google" ) );
            settings.allowInsecureHttp = false;
            engine->SetSettings( settings );
            ASSERT_FALSE( manager.IsProviderOffered( "google" ) );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( manager.Login( "google", AUTH::CAuthOptions(), future ) );

            // the default is that it is not allowed, and the engine has a provider without the plugins it needs unavailable
            AUTH::COAuth2EngineSettings defaults;
            ASSERT_FALSE( defaults.allowInsecureHttp );
            engine->SetSettings( saved );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The engine settings are those of the application: defaults, a round trip, the section of the manager, bounds" )
        try
        {
            AUTH::COAuth2Engine* engine = AUTH::CAuthGlobal::Instance()->GetOAuth2Engine();
            AUTH::COAuth2EngineSettings saved = engine->GetSettings();
            AUTH::COAuth2EngineSettings defaults;
            ASSERT_TRUE( !defaults.allowInsecureHttp && defaults.launchBrowser && 120 == defaults.clockSkewInSeconds && 1000 == defaults.deviceIntervalUnitInMs );
            ASSERT_TRUE( 5 == defaults.deviceDefaultIntervalInSeconds && 5 == defaults.deviceSlowDownIncrementInSeconds && 300 == defaults.openId2MaxAgeInSeconds && 16 == defaults.maxActiveLogins );
            ASSERT_TRUE( 60 == defaults.jwksMinTtlInSeconds && 86400 == defaults.jwksMaxTtlInSeconds && 3600 == defaults.jwksDefaultTtlInSeconds && 10000 == defaults.jwksMinRefreshIntervalInMs );

            AUTH::COAuth2EngineSettings changed;
            changed.allowInsecureHttp = true;
            changed.launchBrowser = false;
            changed.clockSkewInSeconds = 30;
            changed.maxResponseBytes = 4096;
            changed.maxActiveLogins = 3;
            changed.openId2MaxAgeInSeconds = 90;
            ASSERT_TRUE( defaults.caBundlePath.IsNULLOrEmpty() );
            changed.caBundlePath = "/etc/ssl/certs/test-bundle.crt";
            CORE::CDataNode node( "Engine", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( changed.SaveConfig( node ) );
            AUTH::COAuth2EngineSettings back;
            ASSERT_TRUE( back.LoadConfig( node ) );
            ASSERT_TRUE( back.allowInsecureHttp && !back.launchBrowser && 30 == back.clockSkewInSeconds && 4096 == back.maxResponseBytes && 3 == back.maxActiveLogins && 90 == back.openId2MaxAgeInSeconds );
            ASSERT_TRUE( "/etc/ssl/certs/test-bundle.crt" == back.caBundlePath );

            // every request of the engine checks the certificate of the server, against the bundle of the settings when there is one
            AUTH::CAuthHttpRequest bundleRequest;
            ASSERT_TRUE( bundleRequest.Get( "https://provider.example/token", back ) && bundleRequest.options.verifyPeer && "/etc/ssl/certs/test-bundle.crt" == bundleRequest.options.caBundlePath );
            ASSERT_TRUE( bundleRequest.Post( "https://provider.example/token", "application/x-www-form-urlencoded", "a=b", defaults ) && bundleRequest.options.verifyPeer && bundleRequest.options.caBundlePath.IsNULLOrEmpty() );

            // the engine settings are a child of the section of the manager and are found there, also when the node holds the section
            CORE::CDataNode auth( "Auth", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* engineNode = auth.AddChild( "Engine", GUCEF_DATATYPE_OBJECT );
            engineNode->SetAttribute( "allowInsecureHttp", true );
            engineNode->SetAttribute( "clockSkewInSeconds", (UInt32) 45 );
            AUTH::COAuth2EngineSettings fromSection;
            ASSERT_TRUE( fromSection.LoadConfig( auth ) && fromSection.allowInsecureHttp && 45 == fromSection.clockSkewInSeconds );
            CORE::CDataNode root( "Root", GUCEF_DATATYPE_OBJECT );
            root.AddChild( auth );
            AUTH::COAuth2EngineSettings fromRoot;
            ASSERT_TRUE( fromRoot.LoadConfig( root ) && fromRoot.allowInsecureHttp && 45 == fromRoot.clockSkewInSeconds );

            // a skew over the cap is capped, by the settings and by the engine
            engineNode->SetAttribute( "clockSkewInSeconds", (UInt32) 100000 );
            ASSERT_TRUE( fromSection.LoadConfig( auth ) && 600 == fromSection.clockSkewInSeconds );
            AUTH::COAuth2EngineSettings wide = saved;
            wide.clockSkewInSeconds = 100000;
            engine->SetSettings( wide );
            ASSERT_TRUE( 600 == engine->GetSettings().clockSkewInSeconds );
            engine->SetSettings( saved );

            // the manager of the process passes the section on to the engine
            AUTH::CAuthManager manager;
            AUTH::CMemoryCredentialStore credentials;
            manager.SetCredentialStore( &credentials );
            engineNode->SetAttribute( "clockSkewInSeconds", (UInt32) 77 );
            engineNode->SetAttribute( "launchBrowser", false );
            ASSERT_TRUE( manager.LoadConfig( auth ) );
            ASSERT_TRUE( 77 == engine->GetSettings().clockSkewInSeconds && !engine->GetSettings().launchBrowser && engine->GetSettings().allowInsecureHttp );
            // a section without the child leaves them alone
            CORE::CDataNode plain( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( manager.LoadConfig( plain ) && 77 == engine->GetSettings().clockSkewInSeconds );
            engine->SetSettings( saved );

            // the engine of the module is the flow engine
            AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
            ASSERT_TRUE( GUCEF_NULL != global->GetOAuth2Engine() && global->GetFlowEngine() == global->GetOAuth2Engine() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Logins of two providers run at the same time, and a provider that goes away takes its login with it" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            f.fake.SetIssuerPath( "/eu-west-1_TestPool" );
            AUTH::CAuthProviderSettings cognito = f.LocalSettings();
            cognito.parameters[ "region" ] = "eu-west-1";
            cognito.parameters[ "poolId" ] = "eu-west-1_TestPool";
            cognito.parameters[ "poolDomain" ] = "game.auth.eu-west-1.amazoncognito.com";
            AUTH::CAuthProviderSettings google = f.LocalSettings();
            google.issuer = f.fake.GetIssuer();
            AUTH::CAuthSettings settings;
            settings.SetProviderSettings( "google", google );
            settings.SetProviderSettings( "aws-cognito", cognito );
            ASSERT_TRUE( f.Configure( settings ) );

            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), first ) && f.manager.Login( "aws-cognito", AUTH::CAuthOptions(), second ) );
            for ( UInt32 waited=0; waited<20000 && !( first.IsReady() && second.IsReady() ); ++waited )
                PumpFor( 1 );
            AUTH::CAuthResult a, b;
            ASSERT_TRUE( first.IsReady() && second.IsReady() && AUTH::CAuthResult::TryGetFromFuture( first, a ) && AUTH::CAuthResult::TryGetFromFuture( second, b ) );
            ASSERT_TRUE( "google:sub-1000" == a.accountRef.ToString() && "aws:eu-west-1_TestPool:sub-1000" == b.accountRef.ToString() );
            ASSERT_TRUE( 2 == f.browser.GetOpenCount() && 2 == f.fake.GetStats().token );
            AUTH::TLinkRecordVector links;
            ASSERT_TRUE( f.manager.GetLinkedAccounts( links ) && 2 == links.size() );

            // the provider is removed while the user is still at the provider
            f.browser.SetMode( CTestBrowser::MODE_IGNORE );
            f.manager.Unlink( a.accountRef );
            CORE::CFutureResult pending( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), pending ) );
            ASSERT_TRUE( PumpUntilAction( f, 3 ) );
            ASSERT_TRUE( 1 == f.engine->GetActiveLoginCount() );
            CORE::CString redirectUri = f.LastUrlParameters()[ "redirect_uri" ];
            ASSERT_TRUE( f.manager.RemoveProvider( "google" ) );
            PumpFor( 300 );
            ASSERT_TRUE( pending.IsReady() && 0 == f.engine->GetActiveLoginCount() );
            ASSERT_TRUE( ListenerIsGone( redirectUri ) );

            // the engine is told to cancel everything
            CORE::CFutureResult again( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "aws-cognito", AUTH::CAuthOptions(), again ) );
            ASSERT_TRUE( PumpUntilAction( f, 4 ) );
            ASSERT_TRUE( 1 == f.engine->GetActiveLoginCount() );
            f.engine->CancelAll();
            for ( UInt32 waited=0; waited<3000 && !again.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( again.IsReady() );
            PumpFor( 250 );
            ASSERT_TRUE( 0 == f.engine->GetActiveLoginCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only so many logins run at the same time, a provider that is not configured cannot start" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
            settings.maxActiveLogins = 1;
            f.engine->SetSettings( settings );
            AUTH::CAuthSettings all;
            all.SetProviderSettings( "google", f.LocalSettings() );
            all.SetProviderSettings( "microsoft", f.LocalSettings() );
            ASSERT_TRUE( f.Configure( all ) );
            f.browser.SetMode( CTestBrowser::MODE_IGNORE );
            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "google", AUTH::CAuthOptions(), first ) );
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.manager.Login( "microsoft", AUTH::CAuthOptions(), second ) );
            f.manager.CancelLogin( "google" );
            for ( UInt32 waited=0; waited<3000 && !first.IsReady(); ++waited )
                PumpFor( 1 );
            PumpFor( 300 );
            ASSERT_TRUE( f.manager.Login( "microsoft", AUTH::CAuthOptions(), second ) );
            f.manager.CancelLogin( "microsoft" );
            for ( UInt32 waited=0; waited<3000 && !second.IsReady(); ++waited )
                PumpFor( 1 );

            // no client id, no login
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthProviderSettings anonymous = g.LocalSettings();
            anonymous.clientId = CORE::CString::Empty;
            ASSERT_TRUE( g.ConfigureOne( "google", anonymous ) );
            CORE::CFutureResult none( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( g.manager.Login( "google", AUTH::CAuthOptions(), none ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  The log of the HTTP server of gucefWEB shows the responses it sends, with the redirect to the application and its state and code in it.
 *  That server is the one of the provider that lives in the test, so those records are left out: the rest is what the module logs
 */
static CORE::CString
WithoutServerEchoes( const CORE::CString& log )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = log.ParseElements( '\n', true );
    CORE::CString kept;
    bool skipping = false;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString& line = lines[ i ];
        // a record starts with a time stamp in the form YYYY-MM-DDThh:mm:ss.mmmZ|
        bool startsRecord = line.Length() > 24 && '-' == line[ 4 ] && '-' == line[ 7 ] && 'T' == line[ 10 ] && '|' == line[ 24 ];
        if ( startsRecord )
            skipping = line.HasSubstr( "Finished building response header" ) >= 0;
        if ( !skipping )
        {
            kept += line;
            kept += '\n';
        }
    }
    return kept;
}

/*-------------------------------------------------------------------------*/

static void
PerformLogTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: No token, code, verifier, state, nonce, device code or secret appears in the log, in logins that work and in logins that fail" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CLogCapture capture;
            CORE::CString::StringVector secrets;
            {
                COAuth2Fixture f;
                ASSERT_TRUE( f.ready );
                AUTH::CAuthProviderSettings google = f.LocalSettings();
                google.clientSecret.Set( "client-secret-VALUE-0042" );
                ASSERT_TRUE( f.ConfigureOne( "google", google ) );
                secrets.push_back( "client-secret-VALUE-0042" );

                // a login that works
                AUTH::CAuthResult result;
                ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
                secrets.push_back( result.accessToken.Reveal() );
                secrets.push_back( result.idToken.Reveal() );
                secrets.push_back( result.refreshToken.Reveal() );
                CORE::CString::StringMap asked = f.fake.GetLastAuthorizeParameters();
                CORE::CString::StringMap sent = f.fake.GetLastTokenParameters();
                secrets.push_back( asked[ "state" ] );
                secrets.push_back( asked[ "nonce" ] );
                secrets.push_back( asked[ "code_challenge" ] );
                secrets.push_back( sent[ "code" ] );
                secrets.push_back( sent[ "code_verifier" ] );

                // a refresh
                AUTH::CIAuthProvider* provider = f.manager.GetProvider( "google" );
                CORE::CString oldRefresh = f.fake.IssueRefreshToken( "sub-1000" );
                secrets.push_back( oldRefresh );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( provider->Refresh( result.accountRef, AUTH::CSecretString( oldRefresh ), future ) );
                for ( UInt32 waited=0; waited<10000 && !future.IsReady(); ++waited )
                    PumpFor( 1 );
                AUTH::CAuthResult refreshed;
                ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, refreshed ) );
                secrets.push_back( refreshed.accessToken.Reveal() );
                secrets.push_back( refreshed.refreshToken.Reveal() );

                // a logout
                f.manager.Unlink( result.accountRef );
                PumpUntilStat( f.fake, &CFakeStats::revoke, 1, 5000 );

                // a device login
                CFakeBehavior behavior;
                behavior.devicePendingPolls = 1;
                f.fake.SetBehavior( behavior );
                AUTH::CAuthOptions device;
                device.preferDeviceCode = true;
                ASSERT_TRUE( f.Login( "google", device, result ) );
                secrets.push_back( result.accessToken.Reveal() );
                secrets.push_back( result.refreshToken.Reveal() );
                secrets.push_back( result.idToken.Reveal() );
                secrets.push_back( f.fake.GetLastTokenParameters()[ "device_code" ] );

                // logins that fail: a token that is no good, an error of the provider, a wrong callback
                f.manager.Unlink( result.accountRef );
                CFakeTokenSpec spec;
                spec.tamperSignature = true;
                f.fake.SetTokenSpec( spec );
                f.fake.SetBehavior( CFakeBehavior() );
                ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
                secrets.push_back( f.fake.GetLastAuthorizeParameters()[ "state" ] );
                secrets.push_back( f.fake.GetLastTokenParameters()[ "code" ] );
                secrets.push_back( f.fake.GetLastTokenParameters()[ "code_verifier" ] );
                f.fake.SetTokenSpec( CFakeTokenSpec() );
                behavior = CFakeBehavior();
                behavior.tokenError = "invalid_grant";
                f.fake.SetBehavior( behavior );
                ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_GRANT ) );
                secrets.push_back( f.fake.GetLastAuthorizeParameters()[ "state" ] );
                secrets.push_back( f.fake.GetLastTokenParameters()[ "code" ] );
                secrets.push_back( f.fake.GetLastTokenParameters()[ "code_verifier" ] );
                f.fake.SetBehavior( CFakeBehavior() );
                f.browser.SetMode( CTestBrowser::MODE_SEQUENCE );
                ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) );
                secrets.push_back( f.fake.GetLastAuthorizeParameters()[ "state" ] );
                for ( UInt32 waited=0; waited<3000 && !f.browser.IsIdle(); ++waited )
                    PumpFor( 1 );
            }

            CORE::CString all = WithoutServerEchoes( capture.Text() );
            ASSERT_TRUE( all.Length() > 200 );
            UInt32 checked = 0;
            for ( size_t i=0; i<secrets.size(); ++i )
            {
                if ( secrets[ i ].Length() < 8 )
                    continue;
                ++checked;
                bool leaked = Contains( all, secrets[ i ] );
                if ( leaked )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: A secret of kind number " << (UInt32) i << " is in the log" );
                ASSERT_FALSE( leaked );
            }
            ASSERT_TRUE( checked >= 20 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformOAuth2FlowTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OAuth2 flow TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "OAuth2Flows" );

    PerformCodeFlowTests();
    PerformKeyTests();
    PerformListenerFlowTests();
    PerformBrowserTests();
    PerformDeviceTests();
    PerformRefreshTests();
    PerformBodyStyleTests();
    PerformOAuth2OnlyTests();
    PerformOpenId2Tests();
    PerformOpenId2RelayTests();
    PerformPolicyTests();
    PerformLifecycleTests();
    PerformLogTests();
}

/*-------------------------------------------------------------------------*/
