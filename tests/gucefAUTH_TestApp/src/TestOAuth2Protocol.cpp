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

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#include <iterator>

#include "TestPump.h"
#include "TestText.h"
#include "TestHttp.h"
#include "TestLogCapture.h"
#include "TestOpenId2Backend.h"
#include "TestOAuth2Protocol.h"

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

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsRsaAvailable();
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{GUCEF_TRACE;

    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

static AUTH::CAuthProviderDescriptor
Descriptor( const CORE::CString& name )
{GUCEF_TRACE;

    AUTH::CAuthProviderDescriptor descriptor;
    AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( name, descriptor );
    return descriptor;
}

/*-------------------------------------------------------------------------*/

static void
PerformUrlTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Text is percent encoded but for the unreserved characters, decoded strictly, and a form is made and read again" )
        try
        {
            ASSERT_TRUE( "AZaz09-._~" == EncodeUrlText( "AZaz09-._~" ) );
            ASSERT_TRUE( "a%20b%26c%3Dd%2Be%2Ff%3Fg%23h%25i%3A%40j" == EncodeUrlText( "a b&c=d+e/f?g#h%i:@j" ) );
            ASSERT_TRUE( "%22%27%3C%3E%7B%7D%7C%5C%5E%60" == EncodeUrlText( "\"'<>{}|\\^`" ) );
            ASSERT_TRUE( "%C3%A9" == EncodeUrlText( "\xC3\xA9" ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsUnreservedText( "abc-._~123" ) && !AUTH::CAuthUrl::IsUnreservedText( "a b" ) && !AUTH::CAuthUrl::IsUnreservedText( "a/b" ) );

            CORE::CString decoded;
            ASSERT_TRUE( AUTH::CAuthUrl::Decode( "a%20b%2Bc+d", false, decoded ) && "a b+c+d" == decoded );
            ASSERT_TRUE( AUTH::CAuthUrl::Decode( "a%20b%2Bc+d", true, decoded ) && "a b+c d" == decoded );
            ASSERT_TRUE( AUTH::CAuthUrl::Decode( "%C3%A9", false, decoded ) && "\xC3\xA9" == decoded );
            ASSERT_TRUE( AUTH::CAuthUrl::Decode( CORE::CString::Empty, false, decoded ) && decoded.IsNULLOrEmpty() );
            ASSERT_FALSE( AUTH::CAuthUrl::Decode( "%", false, decoded ) );
            ASSERT_FALSE( AUTH::CAuthUrl::Decode( "%2", false, decoded ) );
            ASSERT_FALSE( AUTH::CAuthUrl::Decode( "abc%zz", false, decoded ) );
            ASSERT_FALSE( AUTH::CAuthUrl::Decode( "abc%0", false, decoded ) );
            ASSERT_FALSE( AUTH::CAuthUrl::Decode( "a%00b", false, decoded ) );

            CORE::CString::StringMap values;
            values[ "b" ] = "x y&z=1";
            values[ "a" ] = "";
            values[ "c d" ] = "\xC3\xA9";
            CORE::CString form = AUTH::CAuthUrl::EncodeForm( values );
            ASSERT_TRUE( "a=&b=x%20y%26z%3D1&c%20d=%C3%A9" == form );
            CORE::CString::StringMap back;
            bool duplicate = true;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( form, back, duplicate ) && !duplicate && back == values );
            ASSERT_TRUE( "https://x/p?a=1&b=2%203" == AUTH::CAuthUrl::AppendQuery( "https://x/p?a=1", CORE::CString::StringMap() ) + "&b=2%203" );
            CORE::CString::StringMap one;
            one[ "k" ] = "v w";
            ASSERT_TRUE( "https://x/p?k=v%20w" == AUTH::CAuthUrl::AppendQuery( "https://x/p", one ) && "https://x/p?a=1&k=v%20w" == AUTH::CAuthUrl::AppendQuery( "https://x/p?a=1", one ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A query is limited in size and parameters, a name that occurs twice is flagged, errors in the encoding are refused" )
        try
        {
            CORE::CString::StringMap values;
            bool duplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( CORE::CString::Empty, values, duplicate ) && values.empty() );
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( "a=1&&b=2&c", values, duplicate ) && 3 == values.size() && "1" == values[ "a" ] && "2" == values[ "b" ] && values[ "c" ].IsNULLOrEmpty() && !duplicate );
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( "a=1&a=2", values, duplicate ) && duplicate && "1" == values[ "a" ] );
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( "a=b=c", values, duplicate ) && "b=c" == values[ "a" ] );
            ASSERT_FALSE( AUTH::CAuthUrl::ParseQuery( "a=%zz", values, duplicate ) && !values.empty() );
            ASSERT_FALSE( AUTH::CAuthUrl::ParseQuery( "a=%", values, duplicate ) );

            CORE::CString many;
            for ( UInt32 i=0; i<=AUTH::CAuthUrl::MaxQueryParameters; ++i )
                many += ( i > 0 ? "&p" : "p" ) + CORE::UInt32ToString( i ) + "=1";
            ASSERT_FALSE( AUTH::CAuthUrl::ParseQuery( many, values, duplicate ) );
            CORE::CString limit;
            for ( UInt32 i=0; i<AUTH::CAuthUrl::MaxQueryParameters; ++i )
                limit += ( i > 0 ? "&p" : "p" ) + CORE::UInt32ToString( i ) + "=1";
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( limit, values, duplicate ) && AUTH::CAuthUrl::MaxQueryParameters == values.size() );
            CORE::CString big = "a=";
            for ( UInt32 i=0; i<AUTH::CAuthUrl::MaxQueryBytes; ++i )
                big += "x";
            ASSERT_FALSE( AUTH::CAuthUrl::ParseQuery( big, values, duplicate ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An endpoint is https, or http on the loopback address when that is allowed, without user information" )
        try
        {
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://accounts.example/token", false ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "HTTPS://Accounts.Example:8443/token?x=1", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://accounts.example/token", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://accounts.example/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://127.0.0.1:8080/token", false ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://127.0.0.1:8080/token", true ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://127.4.5.6/token", true ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://localhost:9/token", true ) );
            ASSERT_TRUE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://LocalHost/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://127.0.0.1.evil.example/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://localhost.evil.example/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://127.0.0.256/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://128.0.0.1/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://10.0.0.1/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "http://0.0.0.0/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://user@accounts.example/token", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://user:pw@accounts.example/token", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://accounts.example/token#fragment", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://accounts.example/to ken", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://accounts.example\\@evil.example/", false ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "ftp://accounts.example/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "javascript:alert(1)", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "file:///etc/passwd", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "//accounts.example/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "/token", true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( CORE::CString::Empty, true ) );
            ASSERT_FALSE( AUTH::CAuthUrl::IsAcceptableEndpoint( "https://accounts.example/" + RepeatedText( 'a', 2100 ), false ) );

            CORE::CString scheme, host, path, query;
            UInt16 port = 0;
            ASSERT_TRUE( AUTH::CAuthUrl::Split( "https://Accounts.Example:8443/a/b?x=1&y=2", scheme, host, port, path, query ) );
            ASSERT_TRUE( "https" == scheme && "accounts.example" == host && 8443 == port && "/a/b" == path && "x=1&y=2" == query );
            ASSERT_TRUE( AUTH::CAuthUrl::Split( "http://127.0.0.1", scheme, host, port, path, query ) && 80 == port && "/" == path );
            ASSERT_TRUE( "http://127.0.0.1:5000" == AUTH::CAuthUrl::GetOrigin( "http://127.0.0.1:5000/callback?x=1" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformProtocolTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The code verifier is 43 to 128 unreserved characters from the secure source and the challenge is the S256 of it" )
        try
        {
            // RFC 7636, appendix B
            CORE::CString challenge;
            ASSERT_TRUE( AUTH::COAuth2Protocol::MakeCodeChallenge( "dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk", challenge ) );
            ASSERT_TRUE( "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM" == challenge );

            // verifiers that are not verifiers
            ASSERT_FALSE( AUTH::COAuth2Protocol::MakeCodeChallenge( "short", challenge ) && !challenge.IsNULLOrEmpty() );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 42 ) ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 43 ) ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 128 ) ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 129 ) ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 42 ) + "+" ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 42 ) + "=" ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsValidCodeVerifier( RepeatedText( 'a', 42 ) + " " ) );

            // made ones: valid, long enough, all different, and the challenge is of the verifier
            CORE::CString::StringSet seen;
            for ( UInt32 i=0; i<50; ++i )
            {
                CORE::CString verifier;
                ASSERT_TRUE( AUTH::COAuth2Protocol::MakeCodeVerifier( verifier ) );
                ASSERT_TRUE( AUTH::COAuth2Protocol::IsValidCodeVerifier( verifier ) && verifier.Length() >= 43 && verifier.Length() <= 128 );
                ASSERT_TRUE( seen.insert( verifier ).second );
                CORE::CString first, second;
                ASSERT_TRUE( AUTH::COAuth2Protocol::MakeCodeChallenge( verifier, first ) && AUTH::COAuth2Protocol::MakeCodeChallenge( verifier, second ) && first == second && 43 == first.Length() && first != verifier );
            }
            CORE::CString token;
            ASSERT_TRUE( AUTH::COAuth2Protocol::MakeRandomToken( AUTH::COAuth2Protocol::RandomTokenBytes, token ) && 43 == token.Length() && AUTH::CAuthUrl::IsUnreservedText( token ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::MakeRandomToken( 0, token ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::MakeRandomToken( 1000, token ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The endpoints come from the settings, the discovery document the settings name, the descriptor and its discovery, in that order" )
        try
        {
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "client-1";
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;

            // the descriptor alone: no discovery needed, the endpoints are all there
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( "https://oauth2.googleapis.com/token" == resolved.tokenEndpoint && "https://accounts.google.com/o/oauth2/v2/auth" == resolved.authorizationEndpoint && !resolved.wantsDiscovery );
            ASSERT_TRUE( "https://accounts.google.com" == resolved.issuer && "openid" == resolved.scopes[ 0 ] && "loopback" == resolved.redirectMode && "/callback" == resolved.loopbackPath );

            // an endpoint in the settings wins
            settings.tokenEndpoint = "https://other.example/token";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) && "https://other.example/token" == resolved.tokenEndpoint );
            settings.tokenEndpoint = CORE::CString::Empty;

            // a discovery address in the settings is asked for and then wins over the descriptor
            settings.discoveryUrl = "https://disco.example/.well-known/openid-configuration";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) && resolved.wantsDiscovery && settings.discoveryUrl == resolved.discoveryUrl );
            AUTH::COidcMetadata metadata;
            CORE::CString document = "{\"issuer\":\"https://accounts.google.com\",\"token_endpoint\":\"https://disco.example/token\",\"authorization_endpoint\":\"https://disco.example/auth\",\"jwks_uri\":\"https://disco.example/keys\"}";
            ASSERT_TRUE( metadata.Parse( document.C_String(), AUTH::GetByteLength( document ), "https://accounts.google.com", false ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, &metadata, false, resolved, error ) );
            ASSERT_TRUE( !resolved.wantsDiscovery && "https://disco.example/token" == resolved.tokenEndpoint && "https://disco.example/keys" == resolved.jwksUri );
            // what the document does not have comes from the descriptor
            ASSERT_TRUE( "https://oauth2.googleapis.com/device/code" == resolved.deviceAuthorizationEndpoint );
            settings.tokenEndpoint = "https://settings.example/token";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, &metadata, false, resolved, error ) && "https://settings.example/token" == resolved.tokenEndpoint );

            // the descriptor's own discovery is used when something is missing
            AUTH::CAuthProviderDescriptor sparse = google;
            sparse.tokenEndpoint = CORE::CString::Empty;
            sparse.jwksUri = CORE::CString::Empty;
            AUTH::CAuthProviderSettings plain;
            plain.clientId = "client-1";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( sparse, plain, GUCEF_NULL, false, resolved, error ) && resolved.wantsDiscovery && "https://accounts.google.com/.well-known/openid-configuration" == resolved.discoveryUrl );
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( sparse, plain, &metadata, false, resolved, error ) && "https://disco.example/token" == resolved.tokenEndpoint && !resolved.wantsDiscovery );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Placeholders need parameters that are safe in an address; the tenant issuer keeps {tid}" )
        try
        {
            AUTH::CAuthProviderDescriptor microsoft = Descriptor( "microsoft" );
            AUTH::CAuthProviderDescriptor cognito = Descriptor( "aws-cognito" );
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "client-1";
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;

            // the default tenant is common
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( microsoft, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( "https://login.microsoftonline.com/common/oauth2/v2.0/token" == resolved.tokenEndpoint );
            ASSERT_TRUE( "https://login.microsoftonline.com/{tid}/v2.0" == resolved.issuer );
            settings.parameters[ "tenant" ] = "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( microsoft, settings, GUCEF_NULL, false, resolved, error ) && Contains( resolved.tokenEndpoint, "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001" ) );

            // a parameter that changes the shape of the address is refused
            const char* unsafe[] = { "a/b", "a?b", "a#b", "a b", "a@evil.example", "a\\b", "a%2Fb", "", "evil.example/", "a&b=c" };
            for ( size_t i=0; i<sizeof( unsafe ) / sizeof( unsafe[ 0 ] ); ++i )
            {
                settings.parameters[ "tenant" ] = unsafe[ i ];
                ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( microsoft, settings, GUCEF_NULL, false, resolved, error ) && !resolved.tokenEndpoint.IsNULLOrEmpty() && "" != CORE::CString( unsafe[ i ] ) );
            }
            settings.parameters[ "tenant" ] = "a/b";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( microsoft, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );

            // the parameters Cognito needs
            AUTH::CAuthProviderSettings pool;
            pool.clientId = "client-2";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( cognito, pool, GUCEF_NULL, false, resolved, error ) );
            pool.parameters[ "region" ] = "eu-west-1";
            pool.parameters[ "poolId" ] = "eu-west-1_AbCdEfGhI";
            pool.parameters[ "poolDomain" ] = "game.auth.eu-west-1.amazoncognito.com";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( cognito, pool, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( "https://game.auth.eu-west-1.amazoncognito.com/oauth2/token" == resolved.tokenEndpoint );
            ASSERT_TRUE( "https://cognito-idp.eu-west-1.amazonaws.com/eu-west-1_AbCdEfGhI" == resolved.issuer );
            ASSERT_TRUE( "https://cognito-idp.eu-west-1.amazonaws.com/eu-west-1_AbCdEfGhI/.well-known/jwks.json" == resolved.jwksUri );

            // settings that name every address do not need the parameters of the descriptor
            AUTH::CAuthProviderSettings direct;
            direct.clientId = "client-3";
            direct.issuer = "https://pool.example/id";
            direct.authorizationEndpoint = "https://pool.example/authorize";
            direct.tokenEndpoint = "https://pool.example/token";
            direct.jwksUri = "https://pool.example/jwks";
            direct.userInfoEndpoint = "https://pool.example/userinfo";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( cognito, direct, GUCEF_NULL, false, resolved, error ) && !resolved.wantsDiscovery );
            // without the domain of the pool there is nothing to revoke at
            ASSERT_TRUE( resolved.revocationEndpoint.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Addresses that are not https are refused unless http on the loopback address is allowed, redirect modes are checked" )
        try
        {
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "client-1";
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;

            settings.tokenEndpoint = "http://127.0.0.1:9999/token";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, true, resolved, error ) );
            settings.tokenEndpoint = "http://accounts.example/token";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, true, resolved, error ) );
            settings.tokenEndpoint = CORE::CString::Empty;
            settings.issuer = "http://accounts.example";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, true, resolved, error ) );
            settings.issuer = "http://127.0.0.1:9/{tid}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, true, resolved, error ) );
            settings.issuer = CORE::CString::Empty;
            settings.discoveryUrl = "http://accounts.example/.well-known/openid-configuration";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, true, resolved, error ) );
            settings.discoveryUrl = CORE::CString::Empty;

            // redirect modes
            settings.redirectMode = "carrier-pigeon";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );
            settings.redirectMode = "custom-scheme";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );      // no redirect uri
            const char* acceptable[] = { "com.example.game:/oauth2redirect", "https://game.example/app-link", "http://127.0.0.1:5000/cb", "http://localhost:5000/cb", "myapp://callback" };
            for ( size_t i=0; i<sizeof( acceptable ) / sizeof( acceptable[ 0 ] ); ++i )
            {
                settings.redirectUri = acceptable[ i ];
                ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) && settings.redirectUri == resolved.redirectUri );
            }
            const char* refused[] = { "http://game.example/cb", "javascript:alert(1)", "data:text/html,x", "file:///x", "ftp://x/y", "no scheme", "x", ":/x", "1bad:/x", "com.example.game:", "com.example.game:/a b", "com.example.game:/a#b", "" };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                settings.redirectUri = refused[ i ];
                ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );
            }
            settings.redirectMode = "manual";
            settings.redirectUri = "com.example.game:/cb";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) && "manual" == resolved.redirectMode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The tenant of a token has to fit the tenant setting: common, organizations, consumers or an id" )
        try
        {
            const CORE::CString consumers = AUTH::COAuth2Protocol::ConsumersTenantId;
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsTenantAllowed( "common", "tenant-1" ) && AUTH::COAuth2Protocol::IsTenantAllowed( CORE::CString::Empty, "tenant-1" ) && AUTH::COAuth2Protocol::IsTenantAllowed( "common", consumers ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsTenantAllowed( "organizations", "tenant-1" ) && !AUTH::COAuth2Protocol::IsTenantAllowed( "organizations", consumers ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsTenantAllowed( "consumers", consumers ) && !AUTH::COAuth2Protocol::IsTenantAllowed( "consumers", "tenant-1" ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsTenantAllowed( "tenant-1", "tenant-1" ) && AUTH::COAuth2Protocol::IsTenantAllowed( "TENANT-1", "tenant-1" ) && !AUTH::COAuth2Protocol::IsTenantAllowed( "tenant-1", "tenant-2" ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::IsTenantAllowed( "common", CORE::CString::Empty ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::IsTenantAllowed( "9188040D-6C67-4C5B-B112-36A304B66DAD", consumers ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The authorization address has the standard parameters, PKCE S256, the nonce of OpenID Connect and the extras of the provider" )
        try
        {
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "client 1&x";
            settings.scopes.push_back( "openid" );
            settings.scopes.push_back( "email" );
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, settings, GUCEF_NULL, false, resolved, error ) );
            CORE::CString url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( google, resolved, "http://127.0.0.1:5000/callback", "state-1", "nonce-1", "challenge-1", "me@example.invalid" );

            CORE::CString scheme, host, path, query;
            UInt16 port = 0;
            ASSERT_TRUE( AUTH::CAuthUrl::Split( url, scheme, host, port, path, query ) && "accounts.google.com" == host && "/o/oauth2/v2/auth" == path );
            CORE::CString::StringMap parameters;
            bool duplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate ) && !duplicate );
            ASSERT_TRUE( "code" == parameters[ "response_type" ] && "client 1&x" == parameters[ "client_id" ] && "http://127.0.0.1:5000/callback" == parameters[ "redirect_uri" ] );
            ASSERT_TRUE( "openid email" == parameters[ "scope" ] && "state-1" == parameters[ "state" ] && "nonce-1" == parameters[ "nonce" ] );
            ASSERT_TRUE( "challenge-1" == parameters[ "code_challenge" ] && "S256" == parameters[ "code_challenge_method" ] && "me@example.invalid" == parameters[ "login_hint" ] );
            ASSERT_TRUE( "offline" == parameters[ "access_type" ] );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( url ) );

            // an OAuth 2.0 provider without OpenID Connect and without PKCE has no nonce and no challenge
            AUTH::CAuthProviderDescriptor facebook = Descriptor( "facebook" );
            AUTH::CAuthProviderSettings fbSettings;
            fbSettings.clientId = "app-1";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( facebook, fbSettings, GUCEF_NULL, false, resolved, error ) );
            url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( facebook, resolved, "http://127.0.0.1:5000/callback", "state-1", "nonce-1", "challenge-1", CORE::CString::Empty );
            AUTH::CAuthUrl::Split( url, scheme, host, port, path, query );
            AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate );
            ASSERT_TRUE( parameters.end() == parameters.find( "nonce" ) && parameters.end() == parameters.find( "code_challenge" ) && parameters.end() == parameters.find( "login_hint" ) );
            ASSERT_TRUE( "state-1" == parameters[ "state" ] && "public_profile" == parameters[ "scope" ] );
            ASSERT_TRUE( "/v21.0/dialog/oauth" == path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: login_hint is in the authorization request of the providers that take it and only when there is a hint; it is encoded for a plus, an at sign and unicode" )
        try
        {
            const CORE::CString hint = "j\xC3\xBCrgen+tag@example.invalid";
            const char* names[] = { "google", "microsoft", "aws-cognito", "facebook", "amazon", "epic", "aws-identitycenter" };
            const bool takes[] =  { true    , true       , true         , false     , false   , false , false                };
            for ( size_t n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
            {
                AUTH::CAuthProviderDescriptor descriptor = Descriptor( names[ n ] );
                ASSERT_TRUE( takes[ n ] == descriptor.supportsLoginHint );
                AUTH::CAuthProviderSettings settings;
                settings.clientId = "client-1";
                settings.parameters[ "region" ] = "eu-west-1";
                settings.parameters[ "poolId" ] = "eu-west-1_AbCdEfGhI";
                settings.parameters[ "poolDomain" ] = "pool.auth.eu-west-1.amazoncognito.com";
                settings.parameters[ "instanceId" ] = "d-1234567890";
                AUTH::COAuth2Resolved resolved;
                CORE::CString error;
                ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, false, resolved, error ) );

                // with a hint
                CORE::CString url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( descriptor, resolved, "http://127.0.0.1:5000/callback", "s", "n", "c", hint );
                CORE::CString scheme, host, path, query;
                UInt16 port = 0;
                ASSERT_TRUE( AUTH::CAuthUrl::Split( url, scheme, host, port, path, query ) );
                CORE::CString::StringMap parameters;
                bool duplicate = false;
                ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate ) && !duplicate );
                if ( takes[ n ] )
                {
                    ASSERT_TRUE( hint == parameters[ descriptor.loginHintParameter ] );
                    ASSERT_TRUE( query.HasSubstr( "login_hint=j%C3%BCrgen%2Btag%40example.invalid" ) >= 0 );
                    ASSERT_TRUE( CORE::IsOpenableBrowserUrl( url ) );
                }
                else
                {
                    ASSERT_TRUE( parameters.end() == parameters.find( "login_hint" ) && query.HasSubstr( "example.invalid" ) < 0 );
                }

                // without one, or with one that cannot be a name, the parameter is not there
                CORE::CString::StringVector bad;
                bad.push_back( CORE::CString::Empty );
                bad.push_back( "two\nlines@example.invalid" );
                bad.push_back( "tab\there" );
                CORE::CString longText;
                for ( UInt32 c=0; c<300; ++c )
                    longText += 'a';
                bad.push_back( longText );
                for ( size_t b=0; b<bad.size(); ++b )
                {
                    url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( descriptor, resolved, "http://127.0.0.1:5000/callback", "s", "n", "c", bad[ b ] );
                    AUTH::CAuthUrl::Split( url, scheme, host, port, path, query );
                    AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate );
                    ASSERT_TRUE( parameters.end() == parameters.find( "login_hint" ) );
                }
            }

            // the name of the parameter is the descriptor's own
            AUTH::CAuthProviderDescriptor custom = Descriptor( "google" );
            custom.loginHintParameter = "username";
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "client-1";
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( custom, settings, GUCEF_NULL, false, resolved, error ) );
            CORE::CString url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( custom, resolved, "http://127.0.0.1:5000/callback", "s", "n", "c", "me@example.invalid" );
            ASSERT_TRUE( url.HasSubstr( "username=me%40example.invalid" ) >= 0 && url.HasSubstr( "login_hint" ) < 0 );
            custom.supportsLoginHint = false;
            url = AUTH::COAuth2Protocol::BuildAuthorizationUrl( custom, resolved, "http://127.0.0.1:5000/callback", "s", "n", "c", "me@example.invalid" );
            ASSERT_TRUE( url.HasSubstr( "username" ) < 0 && url.HasSubstr( "example.invalid" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The name of the account for the next login is taken from the claims the descriptor lists, the first one that is acceptable" )
        try
        {
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderDescriptor microsoft = Descriptor( "microsoft" );
            AUTH::CAuthProviderDescriptor facebook = Descriptor( "facebook" );
            CORE::CString::StringMap claims;
            claims[ "email" ] = " me@example.invalid ";
            claims[ "preferred_username" ] = "upn@example.invalid";
            CORE::CString hint;
            ASSERT_TRUE( AUTH::COAuth2Protocol::FindLoginHint( google, claims, hint ) && "me@example.invalid" == hint );
            // Microsoft names the account by its preferred user name
            ASSERT_TRUE( AUTH::COAuth2Protocol::FindLoginHint( microsoft, claims, hint ) && "upn@example.invalid" == hint );
            claims.erase( "preferred_username" );
            ASSERT_TRUE( AUTH::COAuth2Protocol::FindLoginHint( microsoft, claims, hint ) && "me@example.invalid" == hint );
            // a claim that is not acceptable is passed over, a provider with no such parameter has none
            claims[ "preferred_username" ] = "bad\nname";
            ASSERT_TRUE( AUTH::COAuth2Protocol::FindLoginHint( microsoft, claims, hint ) && "me@example.invalid" == hint );
            ASSERT_FALSE( AUTH::COAuth2Protocol::FindLoginHint( facebook, claims, hint ) );
            ASSERT_TRUE( hint.IsNULLOrEmpty() );
            CORE::CString::StringMap nothing;
            ASSERT_FALSE( AUTH::COAuth2Protocol::FindLoginHint( google, nothing, hint ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Bodies are forms or JSON with camel case names, client secrets go in the body or in a Basic header, per descriptor" )
        try
        {
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderDescriptor epic = Descriptor( "epic" );
            AUTH::CAuthProviderDescriptor center = Descriptor( "aws-identitycenter" );
            ASSERT_TRUE( "post" == google.clientAuthMethod && "basic" == epic.clientAuthMethod && center.jsonRequestBodies );

            AUTH::COAuth2Resolved resolved;
            resolved.clientId = "id 1";
            resolved.clientSecret.Set( "s:ecret" );
            CORE::CString::StringMap parameters;
            AUTH::COAuth2Protocol::AddClientParameters( google, resolved, parameters );
            ASSERT_TRUE( "id 1" == parameters[ "client_id" ] && "s:ecret" == parameters[ "client_secret" ] );
            parameters.clear();
            AUTH::COAuth2Protocol::AddClientParameters( epic, resolved, parameters );
            ASSERT_TRUE( "id 1" == parameters[ "client_id" ] && parameters.end() == parameters.find( "client_secret" ) );
            AUTH::CSecretString header;
            ASSERT_FALSE( AUTH::COAuth2Protocol::MakeBasicAuthorization( google, resolved, header ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::MakeBasicAuthorization( epic, resolved, header ) );
            // base64 of "id%201:s%3Aecret": the credentials are form encoded first
            ASSERT_TRUE( "Basic aWQlMjAxOnMlM0FlY3JldA==" == header.Reveal() );
            resolved.clientSecret.Clear();
            parameters.clear();
            AUTH::COAuth2Protocol::AddClientParameters( google, resolved, parameters );
            ASSERT_TRUE( parameters.end() == parameters.find( "client_secret" ) && !AUTH::COAuth2Protocol::MakeBasicAuthorization( epic, resolved, header ) );

            CORE::CString::StringMap body;
            body[ "grant_type" ] = "authorization_code";
            body[ "code_verifier" ] = "v\"1";
            body[ "redirect_uri" ] = "http://127.0.0.1:5/cb";
            CORE::CString contentType;
            CORE::CString text = AUTH::COAuth2Protocol::MakeBody( google, body, contentType );
            ASSERT_TRUE( "application/x-www-form-urlencoded" == contentType && "code_verifier=v%221&grant_type=authorization_code&redirect_uri=http%3A%2F%2F127.0.0.1%3A5%2Fcb" == text );
            text = AUTH::COAuth2Protocol::MakeBody( center, body, contentType );
            ASSERT_TRUE( "application/json" == contentType );
            // the codec of the process writes it: what it says is read back with the names in camel case and the quote in the verifier escaped
            CORE::CDataNode document;
            ASSERT_TRUE( AUTH::CAuthJson::Parse( text, 4096, AUTH::CAuthJson::DefaultMaxDepth, true, document ) );
            const CORE::CDataNode& members = AUTH::CAuthJson::GetDocument( document );
            CORE::CString codeVerifier, grantType, redirectUri;
            ASSERT_TRUE( AUTH::CAuthJson::GetString( members, "codeVerifier", codeVerifier ) && "v\"1" == codeVerifier );
            ASSERT_TRUE( AUTH::CAuthJson::GetString( members, "grantType", grantType ) && "authorization_code" == grantType );
            ASSERT_TRUE( AUTH::CAuthJson::GetString( members, "redirectUri", redirectUri ) && "http://127.0.0.1:5/cb" == redirectUri );
            ASSERT_TRUE( 3 == std::distance( members.AttributeBegin(), members.AttributeEnd() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The answers of token and device endpoints are read in the forms providers have, errors are mapped to reasons" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::COAuth2TokenResponse answer;
            CORE::CString body = "{\"access_token\":\"at-1\",\"token_type\":\"Bearer\",\"expires_in\":3599,\"refresh_token\":\"rt-1\",\"id_token\":\"a.b.c\",\"scope\":\"openid profile\"}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) );
            ASSERT_TRUE( answer.hasAccessToken && "at-1" == answer.accessToken.Reveal() && "rt-1" == answer.refreshToken.Reveal() && "a.b.c" == answer.idToken.Reveal() );
            ASSERT_TRUE( answer.hasExpiresIn && 3599 == answer.expiresInSeconds && "openid profile" == answer.scope && answer.error.IsNULLOrEmpty() );

            // the numbers as text, the names of the JSON style providers
            body = "{\"accessToken\":\"at-2\",\"tokenType\":\"bearer\",\"expiresIn\":\"900\",\"refreshToken\":\"rt-2\",\"idToken\":\"x.y.z\"}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) );
            ASSERT_TRUE( "at-2" == answer.accessToken.Reveal() && "rt-2" == answer.refreshToken.Reveal() && 900 == answer.expiresInSeconds && "bearer" == answer.tokenType );

            // errors: a code, or an object
            body = "{\"error\":\"invalid_grant\",\"error_description\":\"gone\"}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) );
            ASSERT_TRUE( "invalid_grant" == answer.error && !answer.hasAccessToken );
            body = "{\"error\":{\"message\":\"Invalid code\",\"type\":\"OAuthException\",\"code\":100}}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) );
            ASSERT_TRUE( "OAuthException" == answer.error );

            // not an object, not JSON, too large
            ASSERT_FALSE( AUTH::COAuth2Protocol::ParseTokenResponse( "<html>", 6, 65536, answer ) );
            ASSERT_FALSE( AUTH::COAuth2Protocol::ParseTokenResponse( "", 0, 65536, answer ) );
            body = "{\"access_token\":\"at-1\"}";
            ASSERT_FALSE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 5, answer ) );

            // the device answer
            body = "{\"device_code\":\"dc-1\",\"user_code\":\"WDJB-MJHT\",\"verification_uri\":\"https://d.example/activate\",\"verification_uri_complete\":\"https://d.example/activate?c=WDJB-MJHT\",\"expires_in\":900,\"interval\":7}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) );
            ASSERT_TRUE( "dc-1" == answer.deviceCode.Reveal() && "WDJB-MJHT" == answer.userCode && "https://d.example/activate" == answer.verificationUri && answer.hasInterval && 7 == answer.interval && 900 == answer.expiresInSeconds );
            body = "{\"device_code\":\"dc-1\",\"user_code\":\"U\",\"verification_url\":\"https://d.example/old\"}";
            ASSERT_TRUE( AUTH::COAuth2Protocol::ParseTokenResponse( body.C_String(), AUTH::GetByteLength( body ), 65536, answer ) && "https://d.example/old" == answer.verificationUri && !answer.hasInterval );

            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_GRANT == AUTH::COAuth2Protocol::ReasonForError( "invalid_grant" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_DENIED == AUTH::COAuth2Protocol::ReasonForError( "access_denied" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == AUTH::COAuth2Protocol::ReasonForError( "expired_token" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == AUTH::COAuth2Protocol::ReasonForError( "temporarily_unavailable" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == AUTH::COAuth2Protocol::ReasonForError( "invalid_client" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INVALID_RESPONSE == AUTH::COAuth2Protocol::ReasonForError( CORE::CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A discovery document is accepted for the issuer that was expected, with acceptable endpoints" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or rsa codec is not available" );
        try
        {
            AUTH::COidcMetadata metadata;
            CORE::CString good = "{\"issuer\":\"https://id.example\",\"authorization_endpoint\":\"https://id.example/auth\",\"token_endpoint\":\"https://id.example/token\",\"jwks_uri\":\"https://id.example/keys\",\"userinfo_endpoint\":\"https://id.example/me\",\"device_authorization_endpoint\":\"https://id.example/device\",\"revocation_endpoint\":\"https://id.example/revoke\"}";
            ASSERT_TRUE( metadata.Parse( good.C_String(), AUTH::GetByteLength( good ), "https://id.example", false ) );
            ASSERT_TRUE( "https://id.example/token" == metadata.tokenEndpoint && "https://id.example/keys" == metadata.jwksUri && "https://id.example/revoke" == metadata.revocationEndpoint );
            // another issuer, none expected, an endpoint that is not https
            ASSERT_FALSE( metadata.Parse( good.C_String(), AUTH::GetByteLength( good ), "https://other.example", false ) );
            ASSERT_TRUE( metadata.Parse( good.C_String(), AUTH::GetByteLength( good ), CORE::CString::Empty, false ) );
            CORE::CString insecure = good.ReplaceSubstr( "https://id.example/token", "http://id.example/token" );
            ASSERT_FALSE( metadata.Parse( insecure.C_String(), AUTH::GetByteLength( insecure ), "https://id.example", false ) );
            ASSERT_FALSE( metadata.Parse( insecure.C_String(), AUTH::GetByteLength( insecure ), "https://id.example", true ) );
            CORE::CString loopback = good.ReplaceSubstr( "https://id.example/token", "http://127.0.0.1:9/token" );
            ASSERT_FALSE( metadata.Parse( loopback.C_String(), AUTH::GetByteLength( loopback ), "https://id.example", false ) );
            ASSERT_TRUE( metadata.Parse( loopback.C_String(), AUTH::GetByteLength( loopback ), "https://id.example", true ) );
            CORE::CString noIssuer = good.ReplaceSubstr( "\"issuer\":\"https://id.example\",", "" );
            ASSERT_FALSE( metadata.Parse( noIssuer.C_String(), AUTH::GetByteLength( noIssuer ), CORE::CString::Empty, false ) );

            // the tenant forms
            CORE::CString tenant = "{\"issuer\":\"https://login.example/{tenantid}/v2.0\",\"token_endpoint\":\"https://login.example/common/token\"}";
            ASSERT_TRUE( metadata.Parse( tenant.C_String(), AUTH::GetByteLength( tenant ), "https://login.example/{tid}/v2.0", false ) );
            ASSERT_FALSE( metadata.Parse( tenant.C_String(), AUTH::GetByteLength( tenant ), "https://login.example/fixed/v2.0", false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static AUTH::COpenId2Assertion
MakeAssertion( const CORE::CString& nonce, const CORE::CString& returnTo = "http://127.0.0.1:5000/callback?state=s1" )
{GUCEF_TRACE;

    AUTH::COpenId2Assertion assertion;
    CORE::CString::StringMap fields;
    fields[ "openid.ns" ] = AUTH::COpenId2Assertion::Namespace;
    fields[ "openid.mode" ] = "id_res";
    fields[ "openid.op_endpoint" ] = "https://openid.example.org/openid/login";
    fields[ "openid.claimed_id" ] = "https://openid.example.org/openid/id/4200000042";
    fields[ "openid.identity" ] = "https://openid.example.org/openid/id/4200000042";
    fields[ "openid.return_to" ] = returnTo;
    fields[ "openid.response_nonce" ] = nonce;
    fields[ "openid.assoc_handle" ] = "1234567890";
    fields[ "openid.signed" ] = "signed,op_endpoint,claimed_id,identity,return_to,response_nonce,assoc_handle";
    fields[ "openid.sig" ] = "c2lnbmF0dXJl";
    assertion.Load( fields );
    return assertion;
}

/*-------------------------------------------------------------------------*/

static void
PerformOpenId2Tests( void )
{GUCEF_TRACE;

    const CORE::CString endpoint = "https://openid.example.org/openid/login";

    GUCEF_TESTFW_TESTCASE( "Test: The time of a response nonce is read, only in the form of the specification" )
        try
        {
            UInt64 seconds = 0;
            ASSERT_TRUE( AUTH::COpenId2::ParseNonceTime( "1970-01-01T00:00:00Zabc", seconds ) && 0 == seconds );
            ASSERT_TRUE( AUTH::COpenId2::ParseNonceTime( "2010-05-12T20:31:54Zunique", seconds ) && 1273696314 == seconds );
            ASSERT_TRUE( AUTH::COpenId2::ParseNonceTime( "2000-02-29T12:00:00Z0", seconds ) && 951825600 == seconds );
            ASSERT_TRUE( AUTH::COpenId2::ParseNonceTime( "2038-01-19T03:14:08Zx", seconds ) && 2147483648ULL == seconds );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-05-12T20:31:54", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-05-12 20:31:54Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-13-12T20:31:54Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-05-32T20:31:54Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-05-12T24:31:54Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "1969-12-31T23:59:59Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "20x0-05-12T20:31:54Zx", seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( CORE::CString::Empty, seconds ) );
            ASSERT_FALSE( AUTH::COpenId2::ParseNonceTime( "2010-05-12T20:31:54Z" + RepeatedText( 'a', 300 ), seconds ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The claimed id and the identity are for the profile of the provider to accept, and the subject of the account comes from it" )
        try
        {
            CORE::CString subject;
            const AUTH::IOpenId2ProviderProfile& profile = TestOpenId2Profile();
            ASSERT_TRUE( profile.ExtractSubject( endpoint, "https://openid.example.org/openid/id/4200000042", "https://openid.example.org/openid/id/4200000042", subject ) && "4200000042" == subject );
            ASSERT_TRUE( profile.ExtractSubject( endpoint, "https://openid.example.org/openid/id/abc9", "https://openid.example.org/openid/id/abc9", subject ) && "abc9" == subject );
            const char* refused[] =
            {
                "https://openid.example.org/openid/id/", "https://openid.example.org/openid/id/ABC", "https://openid.example.org/openid/id/12-ab", "https://openid.example.org/openid/id/-1",
                "https://openid.example.org/openid/id/123/evil", "https://openid.example.org/openid/id/123?x=1", "https://openid.example.org/openid/id/123 ", "http://openid.example.org/openid/id/123",
                "https://openid.example.org.evil.example/openid/id/123", "https://evil.example/openid/id/123", "https://openid.example.org/openid/ID/123",
                "https://openid.example.org/openid/id/123456789012345678901234567890123", "https://user@openid.example.org/openid/id/123", "", "4200000042"
            };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                ASSERT_FALSE( profile.ExtractSubject( endpoint, refused[ i ], refused[ i ], subject ) );
                ASSERT_TRUE( subject.IsNULLOrEmpty() );
            }
            // the identity is the claimed id, in this profile
            ASSERT_FALSE( profile.ExtractSubject( endpoint, "https://openid.example.org/openid/id/1", "https://openid.example.org/openid/id/2", subject ) );
            // the origin is the one of the endpoint that was configured
            ASSERT_TRUE( profile.ExtractSubject( "http://127.0.0.1:4000/openid/login", "http://127.0.0.1:4000/openid/id/5", "http://127.0.0.1:4000/openid/id/5", subject ) && "5" == subject );
            ASSERT_FALSE( profile.ExtractSubject( "http://127.0.0.1:4000/openid/login", "http://127.0.0.1:4001/openid/id/5", "http://127.0.0.1:4001/openid/id/5", subject ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An assertion is checked for namespace, mode, endpoint, claimed id, return address, signed fields and a fresh nonce used once" )
        try
        {
            const UInt64 now = 1273696314 + 30;
            const CORE::CString freshNonce = "2010-05-12T20:31:54Zunique-1";
            const CORE::CString returnTo = "http://127.0.0.1:5000/callback?state=s1";
            AUTH::COpenId2NonceCache nonces;

            AUTH::COpenId2Assertion good = MakeAssertion( freshNonce );
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces ) );
            ASSERT_TRUE( 1 == nonces.GetCount() );
            // the same assertion again: replayed
            ASSERT_TRUE( AUTH::OPENID2_REPLAYED_NONCE == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces ) );
            nonces.Release( freshNonce );
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces ) );
            nonces.Clear();

            // each field in turn
            struct SCase { const char* field; const char* value; AUTH::TOpenId2Status expected; };
            const SCase cases[] =
            {
                { "openid.ns", "http://specs.openid.net/auth/1.1", AUTH::OPENID2_MALFORMED },
                { "openid.mode", "cancel", AUTH::OPENID2_MALFORMED },
                { "openid.mode", "checkid_setup", AUTH::OPENID2_MALFORMED },
                { "openid.op_endpoint", "https://openid.example.org/openid/login/", AUTH::OPENID2_WRONG_ENDPOINT },
                { "openid.op_endpoint", "https://evil.example/openid/login", AUTH::OPENID2_WRONG_ENDPOINT },
                { "openid.claimed_id", "https://openid.example.org/openid/id/12345", AUTH::OPENID2_BAD_CLAIMED_ID },
                { "openid.identity", "https://openid.example.org/openid/id/12345", AUTH::OPENID2_BAD_CLAIMED_ID },
                { "openid.return_to", "http://127.0.0.1:5000/callback?state=other", AUTH::OPENID2_BAD_RETURN_TO },
                { "openid.return_to", "http://127.0.0.1:5000/callback", AUTH::OPENID2_BAD_RETURN_TO },
                { "openid.signed", "signed,op_endpoint,claimed_id,identity,response_nonce,assoc_handle", AUTH::OPENID2_UNSIGNED_FIELD },
                { "openid.signed", "signed,op_endpoint,claimed_id,return_to,response_nonce,assoc_handle", AUTH::OPENID2_UNSIGNED_FIELD },
                { "openid.signed", "signed", AUTH::OPENID2_UNSIGNED_FIELD },
                { "openid.response_nonce", "yesterday", AUTH::OPENID2_BAD_NONCE },
                { "openid.response_nonce", "2010-05-12T20:00:00Zold", AUTH::OPENID2_STALE_NONCE },
                { "openid.response_nonce", "2010-05-12T21:00:00Zfuture", AUTH::OPENID2_STALE_NONCE }
            };
            for ( size_t i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                AUTH::COpenId2Assertion changed = MakeAssertion( freshNonce );
                CORE::CString::StringMap fields = changed.fields;
                fields[ cases[ i ].field ] = cases[ i ].value;
                changed.Load( fields );
                ASSERT_TRUE( cases[ i ].expected == AUTH::COpenId2::Validate( changed, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces ) );
                // nothing that was refused is remembered
                ASSERT_TRUE( 0 == nonces.GetCount() );
            }

            // a missing field is an assertion that cannot be loaded
            CORE::CString::StringMap fields = good.fields;
            fields.erase( "openid.sig" );
            AUTH::COpenId2Assertion incomplete;
            ASSERT_FALSE( incomplete.Load( fields ) );
            fields = good.fields;
            fields[ "openid.sig" ] = RepeatedText( 'x', 3000 );
            ASSERT_FALSE( incomplete.Load( fields ) );
            // the check at the provider wants every openid.* field of the response back (OpenID 2.0, 11.4.2.1), so those of extensions are kept; what is not an openid.* field is not
            fields = good.fields;
            fields[ "openid.ext1.mode" ] = "fetch_response";
            fields[ "evil" ] = "x";
            AUTH::COpenId2Assertion extended;
            ASSERT_TRUE( extended.Load( fields ) && extended.fields.end() != extended.fields.find( "openid.ext1.mode" ) && extended.fields.end() == extended.fields.find( "evil" ) );
            ASSERT_TRUE( 11 == extended.fields.size() );
            // the fields of an extension do not make the assertion unacceptable: they are not signed for, so nothing is taken from them, and they go back to the provider with the rest
            AUTH::COpenId2NonceCache extendedNonces;
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( extended, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, extendedNonces ) );
            CORE::CString::StringMap extendedForm;
            bool extendedDuplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( AUTH::COpenId2::BuildCheckAuthenticationBody( extended ), extendedForm, extendedDuplicate ) && "fetch_response" == extendedForm[ "openid.ext1.mode" ] && "check_authentication" == extendedForm[ "openid.mode" ] && 11 == extendedForm.size() );

            // a name that is no field name, a value that is too long and too many of them make the assertion unacceptable
            fields = good.fields;
            fields[ "openid.ext 1" ] = "x";
            ASSERT_FALSE( incomplete.Load( fields ) );
            fields = good.fields;
            fields[ "openid.ext1.value" ] = RepeatedText( 'x', 3000 );
            ASSERT_FALSE( incomplete.Load( fields ) );
            fields = good.fields;
            for ( UInt32 i=0; i<AUTH::COpenId2Assertion::MaxExtraFields; ++i )
                fields[ "openid.ext." + CORE::ToString( i ) ] = "x";
            ASSERT_TRUE( incomplete.Load( fields ) && 10 + AUTH::COpenId2Assertion::MaxExtraFields == incomplete.fields.size() );
            fields[ "openid.ext.over" ] = "x";
            ASSERT_FALSE( incomplete.Load( fields ) );

            // without an expected return address it has to be on the loopback interface, http, with the path of the application
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces ) );
            nonces.Clear();
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/other", now, 300, nonces ) );
            const char* returns[] = { "https://127.0.0.1:5000/callback", "http://evil.example/callback", "http://127.0.0.1.evil.example/callback", "http://10.0.0.1/callback", "javascript:x" };
            for ( size_t i=0; i<sizeof( returns ) / sizeof( returns[ 0 ] ); ++i )
            {
                AUTH::COpenId2Assertion changed = MakeAssertion( freshNonce, returns[ i ] );
                ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( changed, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces ) );
            }
            AUTH::COpenId2Assertion localhost = MakeAssertion( freshNonce, "http://localhost:6000/callback?state=x" );
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( localhost, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Nonces are kept for as long as they matter, are limited in number and can be released" )
        try
        {
            AUTH::COpenId2NonceCache nonces;
            ASSERT_TRUE( nonces.Reserve( "a", 1000, 100 ) && !nonces.Reserve( "a", 1050, 100 ) && nonces.Reserve( "b", 1050, 100 ) );
            ASSERT_TRUE( 2 == nonces.GetCount() );
            // after its time a nonce is forgotten
            ASSERT_TRUE( nonces.Reserve( "a", 1100, 100 ) );
            ASSERT_TRUE( 2 == nonces.GetCount() );
            nonces.Release( "a" );
            ASSERT_TRUE( 1 == nonces.GetCount() && nonces.Reserve( "a", 1100, 100 ) );
            nonces.Clear();
            ASSERT_TRUE( 0 == nonces.GetCount() );

            // a full cache refuses rather than forgets a fresh nonce
            for ( UInt32 i=0; i<AUTH::COpenId2NonceCache::MaxEntries; ++i )
                ASSERT_TRUE( nonces.Reserve( "n" + CORE::UInt32ToString( i ), 5000, 1000 ) );
            ASSERT_FALSE( nonces.Reserve( "one-too-many", 5000, 1000 ) );
            ASSERT_FALSE( nonces.Reserve( "n0", 5000, 1000 ) );
            ASSERT_TRUE( nonces.Reserve( "later", 7000, 1000 ) );

            // the answers of check_authentication
            const CORE::CString yes = "ns:http://specs.openid.net/auth/2.0\nis_valid:true\n";
            const CORE::CString no = "ns:http://specs.openid.net/auth/2.0\nis_valid:false\n";
            ASSERT_TRUE( AUTH::COpenId2::IsValidAnswer( yes.C_String(), AUTH::GetByteLength( yes ) ) );
            ASSERT_TRUE( AUTH::COpenId2::IsValidAnswer( "is_valid:true", 13 ) && AUTH::COpenId2::IsValidAnswer( "ns:x\r\nis_valid:true\r\n", 21 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( no.C_String(), AUTH::GetByteLength( no ) ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "is_valid:true\nis_valid:false\n", 28 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "is_valid:false\nis_valid:true\n", 28 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "is_valid:TRUE", 13 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "xis_valid:true", 14 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "ns:only\n", 8 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( "", 0 ) );
            ASSERT_FALSE( AUTH::COpenId2::IsValidAnswer( GUCEF_NULL, 5 ) );

            // the bodies: the fields of the assertion with the mode changed
            AUTH::COpenId2Assertion assertion = MakeAssertion( "2010-05-12T20:31:54Zunique-1" );
            CORE::CString body = AUTH::COpenId2::BuildCheckAuthenticationBody( assertion );
            CORE::CString::StringMap fields;
            bool duplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::ParseQuery( body, fields, duplicate ) && "check_authentication" == fields[ "openid.mode" ] && 10 == fields.size() );
            ASSERT_TRUE( assertion.fields[ "openid.mode" ] == "id_res" );
            // the proof is the same assertion as a query string and back
            AUTH::COpenId2Assertion back;
            ASSERT_TRUE( back.LoadFromQueryString( assertion.ToQueryString() ) && back.fields == assertion.fields );
            ASSERT_FALSE( back.LoadFromQueryString( "openid.ns=x&openid.ns=y" ) );
            ASSERT_FALSE( back.LoadFromQueryString( CORE::CString::Empty ) );

            CORE::CString url = AUTH::COpenId2::BuildAuthenticationUrl( endpoint, "http://127.0.0.1:5000/callback?state=s", "http://127.0.0.1:5000" );
            CORE::CString scheme, host, path, query;
            UInt16 port = 0;
            AUTH::CAuthUrl::Split( url, scheme, host, port, path, query );
            AUTH::CAuthUrl::ParseQuery( query, fields, duplicate );
            ASSERT_TRUE( "checkid_setup" == fields[ "openid.mode" ] && AUTH::COpenId2Assertion::IdentifierSelect == fields[ "openid.identity" ] && AUTH::COpenId2Assertion::IdentifierSelect == fields[ "openid.claimed_id" ] );
            ASSERT_TRUE( "http://127.0.0.1:5000/callback?state=s" == fields[ "openid.return_to" ] && "http://127.0.0.1:5000" == fields[ "openid.realm" ] && "openid.example.org" == host );
            ASSERT_TRUE( CORE::IsOpenableBrowserUrl( url ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: the address of the relay page is a plain https address on a domain name, normalized, and the reason names nothing of it" )
        try
        {
            CORE::CString base, realm, reason;
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://login.vanvelzensoftware.com/openid-return", false, base, realm, reason ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/openid-return" == base && "https://login.vanvelzensoftware.com/" == realm && reason.IsNULLOrEmpty() );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "HTTPS://Login.VanvelzenSoftware.com/OpenId-Return/", false, base, realm, reason ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/OpenId-Return/" == base && "https://login.vanvelzensoftware.com/" == realm );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://login.vanvelzensoftware.com", false, base, realm, reason ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/" == base && "https://login.vanvelzensoftware.com/" == realm );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://login.vanvelzensoftware.com:443/a/b_c-d.e~f", false, base, realm, reason ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/a/b_c-d.e~f" == base && "https://login.vanvelzensoftware.com/" == realm );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://xn--bcher-kva.example/x", false, base, realm, reason ) && "https://xn--bcher-kva.example/x" == base );

            const char* refused[] =
            {
                "", " ", "login.example.com/x", "https://", "https:///x", "http://login.example.com/x", "ftp://login.example.com/x", "javascript:alert(1)", "//login.example.com/x",
                "https://user@login.example.com/x", "https://user:pw@login.example.com/", "https://@login.example.com/", "https://login.example.com@evil.example/",
                "https://login.example.com:8443/x", "https://login.example.com:80/", "https://login.example.com:0/", "https://login.example.com:/x", "https://login.example.com:abc/",
                "https://login.example.com:65536/", "https://login.example.com:443:443/", "https://login.example.com:0443/",
                "https://login.example.com/x?y=1", "https://login.example.com/?", "https://login.example.com/x#f", "https://login.example.com/#",
                "https://login.example.com/../x", "https://login.example.com/a/../x", "https://login.example.com/a/..", "https://login.example.com/..", "https://login.example.com/a..b",
                "https://login.example.com/a/./x", "https://login.example.com/a/.", "https://login.example.com//x", "https://login.example.com/a//b", "https://login.example.com/a:b",
                "https://login.example.com/%2e%2e/x", "https://login.example.com/x%20y", "https://login.example.com/x y", "https://login.example.com/x\\y", "https://login.example.com\\x",
                " https://login.example.com/x", "https://login.example.com/x ", "https://login.example.com/x\n", "https://login.example.com/\xC3\xA9", "https://exa\xC3\xA9mple.com/x",
                "https://localhost/x", "https://127.0.0.1/x", "https://192.168.1.1/x", "https://1.2.3.4.5/x", "https://[::1]/x", "https://login.example.123/x", "https://example/x",
                "https://-bad.example.com/", "https://bad-.example.com/", "https://a..example.com/", "https://.example.com/", "https://example.com./x", "https://exa_mple.com/", "https://exa mple.com/",
                "https://login.example.com/x\"", "https://login.example.com/<x>", "https://login.example.com/x'", "https://login.example.com/x(1)"
            };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                if ( AUTH::COpenId2Relay::ParseRelayUrl( refused[ i ], false, base, realm, reason ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The relay address was not refused: " << refused[ i ] );
                ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( refused[ i ], false, base, realm, reason ) );
                ASSERT_TRUE( base.IsNULLOrEmpty() && realm.IsNULLOrEmpty() && !reason.IsNULLOrEmpty() );
            }
            // length
            ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( "https://login.example.com/" + RepeatedText( 'a', 500 ), false, base, realm, reason ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( "https://" + RepeatedText( 'a', 64 ) + ".example.com/", false, base, realm, reason ) );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://" + RepeatedText( 'a', 63 ) + ".example.com/", false, base, realm, reason ) );

            // the reason says what is wrong and never repeats the address
            ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( "https://user-secret@login.example.com/path-secret?token-secret", false, base, realm, reason ) );
            ASSERT_TRUE( !Contains( reason, "secret" ) && !Contains( reason, "login.example.com" ) );

            // the tests of the engine may have another port and http on the loopback address; nothing else is relaxed
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "http://127.0.0.1:50000/openid-return", true, base, realm, reason ) );
            ASSERT_TRUE( "http://127.0.0.1:50000/openid-return" == base && "http://127.0.0.1:50000/" == realm );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "http://localhost:3000/x", true, base, realm, reason ) && "http://localhost:3000/x" == base );
            ASSERT_TRUE( AUTH::COpenId2Relay::ParseRelayUrl( "https://login.example.com:8443/x", true, base, realm, reason ) && "https://login.example.com:8443/x" == base && "https://login.example.com:8443/" == realm );
            ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( "http://127.0.0.1:50000/openid-return", false, base, realm, reason ) );
            const char* stillRefused[] =
            {
                "http://login.example.com/x", "http://10.0.0.1/x", "http://127.0.0.1.evil.example/x", "https://user@login.example.com/x", "http://user@127.0.0.1:5000/x",
                "https://login.example.com/../x", "https://login.example.com/x?y", "https://login.example.com/x#y", "https://login.example.com:0/", "https://login.example.com:70000/",
                "https://login.example.com/\xC3\xA9", "http://127.0.0.1:5000/%2e%2e/x", "ftp://127.0.0.1/x", "https://192.168.1.1/x"
            };
            for ( size_t i=0; i<sizeof( stillRefused ) / sizeof( stillRefused[ 0 ] ); ++i )
                ASSERT_FALSE( AUTH::COpenId2Relay::ParseRelayUrl( stillRefused[ i ], true, base, realm, reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: the return address is the relay page with exactly the port of the listener and the state, and nothing else is one" )
        try
        {
            const CORE::CString base = "https://login.example.com/openid-return";
            const CORE::CString state = "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789_-abcde";
            CORE::CString returnTo = AUTH::COpenId2Relay::BuildReturnTo( base, 50123, state );
            ASSERT_TRUE( "https://login.example.com/openid-return?port=50123&s=" + state == returnTo );
            ASSERT_TRUE( AUTH::COpenId2Relay::IsRelayReturnTo( returnTo, base ) );
            ASSERT_TRUE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 1024, state ), base ) );
            ASSERT_TRUE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 65535, state ), base ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 1023, state ), base ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 80, state ), base ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 50123, "short" ), base ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 50123, state + "." ), base ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( AUTH::COpenId2Relay::BuildReturnTo( base, 50123, RepeatedText( 'a', 129 ) ), base ) );

            const CORE::CString good = "port=50123&s=" + state;
            const CORE::CString refused[] =
            {
                base, base + "?", base + "?port=50123", base + "?s=" + state, base + "?port=50123&s=" + state + "&x=1", base + "?port=50123&port=50124&s=" + state, base + "?port=50123&s=" + state + "&s=" + state,
                base + "?port=05000&s=" + state, base + "?port=65536&s=" + state, base + "?port=99999&s=" + state, base + "?port=100000&s=" + state, base + "?port=5000a&s=" + state, base + "?port=%35000&s=" + state,
                base + "?port=&s=" + state, base + "?port=+5000&s=" + state, base + "?port=-5000&s=" + state, base + "#?" + good, base + "/?" + good, base + "x?" + good, base + "&" + good,
                "http://login.example.com/openid-return?" + good, "https://login.example.com.evil.example/openid-return?" + good, "https://evil.example/openid-return?" + good,
                "https://login.example.com:8443/openid-return?" + good, "https://login.example.com/other?" + good, "https://login.example.com/OpenId-Return?" + good, "https://user@login.example.com/openid-return?" + good,
                "http://127.0.0.1:50123/callback?state=" + state, "http://127.0.0.1:50123/callback?" + good, CORE::CString::Empty
            };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                if ( AUTH::COpenId2Relay::IsRelayReturnTo( refused[ i ], base ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The return address was not refused: " << refused[ i ] );
                ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( refused[ i ], base ) );
            }
            ASSERT_FALSE( AUTH::COpenId2Relay::IsRelayReturnTo( returnTo, CORE::CString::Empty ) );

            // the state of a login is what the relay page takes
            CORE::CString made;
            ASSERT_TRUE( AUTH::COAuth2Protocol::MakeRandomToken( AUTH::COAuth2Protocol::RandomTokenBytes, made ) && AUTH::COpenId2Relay::IsValidState( made ) );
            ASSERT_FALSE( AUTH::COpenId2Relay::IsValidState( CORE::CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: an assertion is valid for the relay only with the return address the login sent, on the relay host, with the port and the state in its query" )
        try
        {
            const UInt64 now = 1273696314 + 30;
            const CORE::CString nonce = "2010-05-12T20:31:54Zunique-1";
            const CORE::CString base = "https://login.example.com/openid-return";
            const CORE::CString state = "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789_-abcde";
            const CORE::CString returnTo = AUTH::COpenId2Relay::BuildReturnTo( base, 50123, state );
            AUTH::COpenId2NonceCache nonces;
            const CORE::CString endpoint = "https://openid.example.org/openid/login";

            // the one that was sent, with and without knowing which one it was
            AUTH::COpenId2Assertion good = MakeAssertion( nonce, returnTo );
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces, base ) );
            nonces.Clear();
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces, base ) );
            nonces.Clear();

            // another port, another state, another host, another path: not the one that was sent
            const CORE::CString others[] =
            {
                AUTH::COpenId2Relay::BuildReturnTo( base, 50124, state ),
                AUTH::COpenId2Relay::BuildReturnTo( base, 50123, state + "x" ),
                AUTH::COpenId2Relay::BuildReturnTo( "https://evil.example/openid-return", 50123, state ),
                AUTH::COpenId2Relay::BuildReturnTo( "https://login.example.com/other", 50123, state ),
                AUTH::COpenId2Relay::BuildReturnTo( "http://login.example.com/openid-return", 50123, state ),
                "http://127.0.0.1:50123/callback?state=" + state
            };
            for ( size_t i=0; i<sizeof( others ) / sizeof( others[ 0 ] ); ++i )
            {
                AUTH::COpenId2Assertion changed = MakeAssertion( nonce, others[ i ] );
                ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( changed, endpoint, TestOpenId2Profile(), returnTo, "/callback", now, 300, nonces, base ) );
                ASSERT_TRUE( 0 == nonces.GetCount() );
            }
            // the host of the relay is the configured one even when the assertion is the return address that was sent
            AUTH::COpenId2Assertion foreign = MakeAssertion( nonce, others[ 2 ] );
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( foreign, endpoint, TestOpenId2Profile(), others[ 2 ], "/callback", now, 300, nonces, base ) );
            ASSERT_TRUE( AUTH::OPENID2_OK == AUTH::COpenId2::Validate( foreign, endpoint, TestOpenId2Profile(), others[ 2 ], "/callback", now, 300, nonces ) );
            nonces.Clear();

            // a server that does not know the port and the state accepts the form of the relay and the host of the relay only
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( MakeAssertion( nonce, others[ 2 ] ), endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces, base ) );
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( MakeAssertion( nonce, others[ 3 ] ), endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces, base ) );
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( MakeAssertion( nonce, others[ 5 ] ), endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces, base ) );
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( MakeAssertion( nonce, base + "?port=80&s=" + state ), endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces, base ) );
            ASSERT_TRUE( 0 == nonces.GetCount() );

            // without a relay nothing changed: the loopback address is what is acceptable
            ASSERT_TRUE( AUTH::OPENID2_BAD_RETURN_TO == AUTH::COpenId2::Validate( good, endpoint, TestOpenId2Profile(), CORE::CString::Empty, "/callback", now, 300, nonces ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: the relay comes from the parameter relayUrl of an OpenID 2.0 provider, only with the loopback redirect and /callback, and is refused when it is not acceptable" )
        try
        {
            AUTH::CAuthProviderDescriptor openId2 = Descriptor( "testopenid" );
            AUTH::CAuthProviderSettings settings;
            AUTH::COAuth2Resolved resolved;
            CORE::CString error;

            // none by default
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( resolved.relayBase.IsNULLOrEmpty() && resolved.relayRealm.IsNULLOrEmpty() );
            settings.parameters[ "relayUrl" ] = CORE::CString::Empty;
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) && resolved.relayBase.IsNULLOrEmpty() );

            settings.parameters[ "relayUrl" ] = "https://login.vanvelzensoftware.com/openid-return";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/openid-return" == resolved.relayBase && "https://login.vanvelzensoftware.com/" == resolved.relayRealm );
            settings.loopbackPort = 50000;
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) && !resolved.relayBase.IsNULLOrEmpty() );
            settings.loopbackPort = 0;

            // not acceptable: the resolution fails, there is no quiet fall back, and the error has no part of the address
            const char* bad[] = { "http://login.vanvelzensoftware.com/openid-return", "https://user@login.vanvelzensoftware.com/openid-return", "https://login.vanvelzensoftware.com:8443/openid-return",
                                  "https://login.vanvelzensoftware.com/../openid-return", "https://login.vanvelzensoftware.com/openid-return?x=1", "https://127.0.0.1/openid-return", "not a url" };
            for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            {
                settings.parameters[ "relayUrl" ] = bad[ i ];
                ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
                ASSERT_TRUE( Contains( error, "relayUrl" ) && !Contains( error, "vanvelzensoftware" ) && !Contains( error, "127.0.0.1" ) && !Contains( error, "not a url" ) );
                ASSERT_TRUE( resolved.relayBase.IsNULLOrEmpty() );
            }

            // only the engine setting for tests relaxes the port and http on the loopback address
            settings.parameters[ "relayUrl" ] = "http://127.0.0.1:50000/openid-return";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, true, resolved, error ) && "http://127.0.0.1:50000/" == resolved.relayRealm );

            // the page of the relay takes the callback path and the ports from 1024 on, and the loopback listener
            settings.parameters[ "relayUrl" ] = "https://login.vanvelzensoftware.com/openid-return";
            settings.loopbackPath = "/other";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
            settings.loopbackPath = "/callback";
            settings.loopbackPort = 80;
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );
            settings.loopbackPort = 1024;
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) && !resolved.relayBase.IsNULLOrEmpty() );
            settings.loopbackPort = 0;
            settings.redirectMode = "manual";
            settings.redirectUri = "https://example.com/return";
            ASSERT_FALSE( AUTH::COAuth2Protocol::Resolve( openId2, settings, GUCEF_NULL, false, resolved, error ) );

            // a provider that is not OpenID 2.0 has no use for it: the loopback address is what they register
            AUTH::CAuthProviderDescriptor google = Descriptor( "google" );
            AUTH::CAuthProviderSettings googleSettings;
            googleSettings.clientId = "client-1";
            googleSettings.parameters[ "relayUrl" ] = "https://login.vanvelzensoftware.com/openid-return";
            ASSERT_TRUE( AUTH::COAuth2Protocol::Resolve( google, googleSettings, GUCEF_NULL, false, resolved, error ) && resolved.relayBase.IsNULLOrEmpty() && resolved.relayRealm.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A request to the listener with the Host of the given name: the connection goes to the listener, the name is in the header */
static int
AskListener( const CORE::CString& url, const CORE::CString& method, const CORE::CString& hostName, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    CORE::CString scheme, host, path, query;
    UInt16 port = 0;
    AUTH::CAuthUrl::Split( url, scheme, host, port, path, query );
    WEB::CHttpRequestData request;
    request.requestType = method;
    if ( "POST" == method )
        request.contentRepresentation = "text/plain";
    WEB::CHttpClientTransferInfo info;
    bool ok;
    if ( hostName.IsNULLOrEmpty() )
    {
        request.SetUrl( url );
        ok = PumpedExecute( request, response, info );
    }
    else
    {
        request.SetUrl( scheme + "://" + hostName + ":" + CORE::ToString( port ) + path + ( query.IsNULLOrEmpty() ? CORE::CString::Empty : "?" + query ) );
        ok = PumpedExecute( request, response, info, host, port );
    }
    return ok ? response.statusCode : 0;
}

/*-------------------------------------------------------------------------*/

static void
PerformListenerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The listener binds to the loopback address and an ephemeral port and refuses what it cannot serve" )
        try
        {
            AUTH::CLoopbackRedirectListener listener;
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_IDLE == listener.GetState() && 0 == listener.GetPort() );
            AUTH::CSecretString state( "the-state-value" );
            ASSERT_FALSE( listener.Start( 0, "callback", state, 1000 ) );                      // no slash
            ASSERT_FALSE( listener.Start( 0, CORE::CString::Empty, state, 1000 ) );
            ASSERT_FALSE( listener.Start( 0, "/call back", state, 1000 ) );
            ASSERT_FALSE( listener.Start( 0, "/callback?x=1", state, 1000 ) );
            ASSERT_FALSE( listener.Start( 0, "/callback", AUTH::CSecretString(), 1000 ) );     // no state
            ASSERT_FALSE( listener.Start( 0, "/callback", state, 1000, CORE::CString::Empty ) );
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_IDLE == listener.GetState() );

            ASSERT_TRUE( listener.Start( 0, "/callback", state, 10000 ) );
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_LISTENING == listener.GetState() && listener.GetPort() > 0 );
            ASSERT_TRUE( "http://127.0.0.1:" + CORE::ToString( listener.GetPort() ) + "/callback" == listener.GetRedirectUri() );
            ASSERT_TRUE( "http://127.0.0.1:" + CORE::ToString( listener.GetPort() ) == listener.GetOrigin() );
            ASSERT_FALSE( listener.Start( 0, "/callback", state, 10000 ) );                    // once
            listener.Close();
            listener.Close();
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_CLOSED == listener.GetState() );

            // a fixed port that is taken
            AUTH::CLoopbackRedirectListener first;
            ASSERT_TRUE( first.Start( 0, "/callback", state, 10000 ) );
            AUTH::CLoopbackRedirectListener second;
            ASSERT_FALSE( second.Start( first.GetPort(), "/callback", state, 10000 ) );
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_IDLE == second.GetState() );
            UInt16 port = first.GetPort();
            first.Close();
            PumpFor( 50 );
            AUTH::CLoopbackRedirectListener third;
            ASSERT_TRUE( third.Start( port, "/callback", state, 10000 ) && port == third.GetPort() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The listener takes one valid callback: GET, the right path, a loopback Host and the state; everything else is turned away" )
        try
        {
            CLogCapture capture;
            AUTH::CLoopbackRedirectListener listener;
            const CORE::CString stateText = "state-0123456789abcdefghij";
            const CORE::CString codeText = "code-SECRET-9876543210";
            ASSERT_TRUE( listener.Start( 0, "/callback", AUTH::CSecretString( stateText ), 20000 ) );
            const CORE::CString base = listener.GetRedirectUri();
            WEB::CHttpResponseData response;

            // the Host: any other name than the loopback address with the port is refused
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "GET", "evil.example.invalid", response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "GET", "127.0.0.2", response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "GET", "127.0.0.1.evil.example", response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "GET", "[::1]", response ) );
            // the method
            ASSERT_TRUE( 405 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "POST", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 405 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "HEAD", CORE::CString::Empty, response ) );
            // the path
            ASSERT_TRUE( 404 == AskListener( listener.GetOrigin() + "/other?code=" + codeText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 404 == AskListener( listener.GetOrigin() + "/?code=" + codeText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 404 == AskListener( listener.GetOrigin() + "/callback/?code=" + codeText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 404 == AskListener( listener.GetOrigin() + "/Callback?code=" + codeText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            // the state: wrong, missing, another case, twice, a prefix, with extra text
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=wrong", "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText.Uppercase(), "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=state-0123", "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=" + stateText + "x", "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?code=" + codeText + "&state=", "GET", CORE::CString::Empty, response ) );
            // no code, no error, no assertion
            ASSERT_TRUE( 400 == AskListener( base + "?state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_TRUE( 400 == AskListener( base + "?x=1&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            // a broken encoding
            ASSERT_TRUE( 400 == AskListener( base + "?code=%zz&state=" + stateText, "GET", CORE::CString::Empty, response ) );

            // none of this ended the wait, none of it showed the state or the code
            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_LISTENING == listener.Poll() );
            ASSERT_TRUE( listener.GetRejectedCount() >= 20 && listener.GetRequestCount() == listener.GetRejectedCount() );
            CORE::CString::StringMap none;
            ASSERT_FALSE( listener.TakeCallback( none ) );
            CORE::CString refusedPage = BodyOf( response );
            ASSERT_FALSE( Contains( refusedPage, stateText ) || Contains( refusedPage, codeText ) );

            // the valid one, with localhost as the name
            ASSERT_TRUE( 200 == AskListener( "http://localhost:" + CORE::ToString( listener.GetPort() ) + "/callback?code=" + codeText + "&state=" + stateText + "&extra=a%20b", "GET", CORE::CString::Empty, response ) );
            CORE::CString page = BodyOf( response );
            ASSERT_TRUE( Contains( page, "You can return to the application" ) );
            ASSERT_FALSE( Contains( page, stateText ) || Contains( page, codeText ) );
            ASSERT_TRUE( Contains( response.contentType, "text/html" ) && Contains( response.contentType.Lowercase(), "charset=utf-8" ) );
            ASSERT_TRUE( Contains( response.cacheability, "no-store" ) );
            ASSERT_TRUE( "no-referrer" == HeaderOf( response, "Referrer-Policy" ) && "nosniff" == HeaderOf( response, "X-Content-Type-Options" ) && "DENY" == HeaderOf( response, "X-Frame-Options" ) );
            CORE::CString policy = HeaderOf( response, "Content-Security-Policy" );
            ASSERT_TRUE( Contains( policy, "default-src 'none'" ) && Contains( policy, "frame-ancestors 'none'" ) );
            // nothing of elsewhere in the page
            ASSERT_FALSE( Contains( page, "http://" ) || Contains( page, "https://" ) || Contains( page.Lowercase(), "<script" ) || Contains( page.Lowercase(), "<img" ) || Contains( page.Lowercase(), "<link" ) );

            ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_RECEIVED == listener.Poll() );
            // from now on every request is a 404, the valid one again too
            ASSERT_TRUE( 404 == AskListener( base + "?code=" + codeText + "&state=" + stateText, "GET", CORE::CString::Empty, response ) );
            ASSERT_FALSE( Contains( BodyOf( response ), stateText ) || Contains( BodyOf( response ), codeText ) );
            ASSERT_TRUE( 404 == AskListener( base + "?code=other&state=" + stateText, "GET", CORE::CString::Empty, response ) );

            CORE::CString::StringMap callback;
            ASSERT_TRUE( listener.TakeCallback( callback ) );
            ASSERT_TRUE( codeText == callback[ "code" ] && stateText == callback[ "state" ] && "a b" == callback[ "extra" ] );
            ASSERT_FALSE( listener.TakeCallback( callback ) && !callback.empty() );

            // the listener closes itself shortly after
            PumpFor( AUTH::CLoopbackRedirectListener::CloseDelayAfterCallbackInMs + 100 );
            listener.Poll();
            WEB::CHttpClientTransferInfo info;
            WEB::CHttpRequestData request;
            request.SetUrl( listener.GetOrigin() + "/probe" );
            ASSERT_FALSE( PumpedExecute( request, response, info, CORE::CString::Empty, 0, 2000 ) );

            // nothing of the callback was logged
            ASSERT_FALSE( capture.Contains( stateText ) || capture.Contains( codeText ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An error of the provider and an OpenID 2.0 answer are callbacks too; the wait ends at the deadline and the listener is gone" )
        try
        {
            WEB::CHttpResponseData response;
            {
                AUTH::CLoopbackRedirectListener listener;
                ASSERT_TRUE( listener.Start( 0, "/cb", AUTH::CSecretString( "st" ), 20000 ) );
                ASSERT_TRUE( 200 == AskListener( listener.GetRedirectUri() + "?error=access_denied&error_description=no&state=st", "GET", CORE::CString::Empty, response ) );
                ASSERT_TRUE( Contains( BodyOf( response ), "You can return to the application" ) );
                CORE::CString::StringMap callback;
                ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_RECEIVED == listener.Poll() && listener.TakeCallback( callback ) && "access_denied" == callback[ "error" ] );
            }
            {
                AUTH::CLoopbackRedirectListener listener;
                ASSERT_TRUE( listener.Start( 0, "/cb", AUTH::CSecretString( "st" ), 20000 ) );
                ASSERT_TRUE( 200 == AskListener( listener.GetRedirectUri() + "?openid.mode=id_res&openid.sig=abc&state=st", "GET", CORE::CString::Empty, response ) );
                CORE::CString::StringMap callback;
                ASSERT_TRUE( listener.TakeCallback( callback ) == ( AUTH::CLoopbackRedirectListener::STATE_RECEIVED == listener.Poll() ) );
            }
            {
                // another name for the state parameter
                AUTH::CLoopbackRedirectListener listener;
                ASSERT_TRUE( listener.Start( 0, "/cb", AUTH::CSecretString( "st" ), 20000, "s" ) );
                ASSERT_TRUE( 400 == AskListener( listener.GetRedirectUri() + "?code=1&state=st", "GET", CORE::CString::Empty, response ) );
                ASSERT_TRUE( 200 == AskListener( listener.GetRedirectUri() + "?code=1&s=st", "GET", CORE::CString::Empty, response ) );
            }
            {
                AUTH::CLoopbackRedirectListener listener;
                ASSERT_TRUE( listener.Start( 0, "/cb", AUTH::CSecretString( "st" ), 150 ) );
                CORE::CString address = listener.GetRedirectUri();
                for ( UInt32 waited=0; waited<2000 && AUTH::CLoopbackRedirectListener::STATE_LISTENING == listener.Poll(); ++waited )
                    PumpFor( 1 );
                ASSERT_TRUE( AUTH::CLoopbackRedirectListener::STATE_TIMED_OUT == listener.Poll() );
                CORE::CString::StringMap none;
                ASSERT_FALSE( listener.TakeCallback( none ) );
                WEB::CHttpClientTransferInfo info;
                WEB::CHttpRequestData request;
                request.SetUrl( address + "?code=1&state=st" );
                ASSERT_FALSE( PumpedExecute( request, response, info, CORE::CString::Empty, 0, 2000 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformOAuth2ProtocolTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OAuth2 protocol TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "OAuth2Protocol" );

    PerformUrlTests();
    PerformProtocolTests();
    PerformOpenId2Tests();
    PerformListenerTests();
}

/*-------------------------------------------------------------------------*/
