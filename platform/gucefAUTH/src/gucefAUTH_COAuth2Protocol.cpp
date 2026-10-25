/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include <string.h>

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CAUTHJSON_H
#include "gucefAUTH_CAuthJson.h"
#define GUCEF_AUTH_CAUTHJSON_H
#endif /* GUCEF_AUTH_CAUTHJSON_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_COPENID2_H
#include "gucefAUTH_COpenId2.h"
#define GUCEF_AUTH_COPENID2_H
#endif /* GUCEF_AUTH_COPENID2_H ? */

#ifndef GUCEF_AUTH_CJWT_H
#include "gucefAUTH_CJwt.h"
#define GUCEF_AUTH_CJWT_H
#endif /* GUCEF_AUTH_CJWT_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_COAuth2Protocol.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 COAuth2Protocol::CodeVerifierBytes;
const UInt32 COAuth2Protocol::RandomTokenBytes;

const CORE::CString COAuth2Protocol::ConsumersTenantId = "9188040d-6c67-4c5b-b112-36a304b66dad";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COidcMetadata::COidcMetadata( void )
    : issuer()
    , authorizationEndpoint()
    , tokenEndpoint()
    , deviceAuthorizationEndpoint()
    , userInfoEndpoint()
    , jwksUri()
    , revocationEndpoint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
COidcMetadata::Parse( const void* data, UInt32 size, const CORE::CString& expectedIssuer, bool insecureLoopbackAllowed )
{GUCEF_TRACE;

    *this = COidcMetadata();

    CORE::CDataNode root;
    if ( !CAuthJson::Parse( data, size, 262144, CAuthJson::DefaultMaxDepth, false, root ) )
        return false;
    const CORE::CDataNode& document = CAuthJson::GetDocument( root );

    COidcMetadata parsed;
    if ( !CAuthJson::GetString( document, "issuer", parsed.issuer ) || parsed.issuer.IsNULLOrEmpty() )
        return false;

    if ( !expectedIssuer.IsNULLOrEmpty() )
    {
        // The shared tenants of Microsoft leave {tenantid} in the document
        CORE::CString documentIssuer = parsed.issuer.ReplaceSubstr( "{tenantid}", "tenantid" );
        CORE::CString tenant;
        if ( !CJwt::MatchIssuer( expectedIssuer, documentIssuer, tenant ) )
            return false;
    }

    const char* names[] = { "authorization_endpoint", "token_endpoint", "device_authorization_endpoint", "userinfo_endpoint", "jwks_uri", "revocation_endpoint" };
    CORE::CString* targets[] = { &parsed.authorizationEndpoint, &parsed.tokenEndpoint, &parsed.deviceAuthorizationEndpoint, &parsed.userInfoEndpoint, &parsed.jwksUri, &parsed.revocationEndpoint };
    for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
    {
        if ( document.IsAttribute( names[ i ] ) )
        {
            if ( !CAuthJson::GetString( document, names[ i ], *targets[ i ] ) || !CAuthUrl::IsAcceptableEndpoint( *targets[ i ], insecureLoopbackAllowed ) )
                return false;
        }
    }
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

COAuth2Resolved::COAuth2Resolved( void )
    : providerName()
    , clientId()
    , clientSecret()
    , acceptedClientIds()
    , scopes()
    , issuer()
    , discoveryUrl()
    , authorizationEndpoint()
    , tokenEndpoint()
    , deviceAuthorizationEndpoint()
    , userInfoEndpoint()
    , jwksUri()
    , revocationEndpoint()
    , tokenVerifyEndpoint()
    , parameters()
    , redirectMode( "loopback" )
    , loopbackPort( 0 )
    , loopbackPath( "/callback" )
    , redirectUri()
    , relayBase()
    , relayRealm()
    , wantsDiscovery( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COAuth2TokenResponse::COAuth2TokenResponse( void )
    : hasAccessToken( false )
    , accessToken()
    , refreshToken()
    , idToken()
    , tokenType()
    , hasExpiresIn( false )
    , expiresInSeconds( 0 )
    , scope()
    , error()
    , errorDescription()
    , deviceCode()
    , userCode()
    , verificationUri()
    , verificationUriComplete()
    , hasInterval( false )
    , interval( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::MakeRandomToken( UInt32 byteCount, CORE::CString& token )
{GUCEF_TRACE;

    token.Clear();
    if ( 0 == byteCount || byteCount > 128 )
        return false;
    UInt8 bytes[ 128 ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, byteCount ) )
        return false;
    bool ok = CORE::Base64UrlEncode( bytes, byteCount, token );
    CAuthCrypto::Wipe( bytes, byteCount );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::MakeCodeVerifier( CORE::CString& verifier )
{GUCEF_TRACE;

    return MakeRandomToken( CodeVerifierBytes, verifier ) && IsValidCodeVerifier( verifier );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::IsValidCodeVerifier( const CORE::CString& verifier )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( verifier );
    return size >= 43 && size <= 128 && CAuthUrl::IsUnreservedText( verifier );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::MakeCodeChallenge( const CORE::CString& verifier, CORE::CString& challenge )
{GUCEF_TRACE;

    challenge.Clear();
    if ( !IsValidCodeVerifier( verifier ) )
        return false;
    UInt8 digest[ CAuthCrypto::DigestSize ];
    CAuthCrypto::Sha256( verifier.C_String(), GetByteLength( verifier ), digest );
    return CORE::Base64UrlEncode( digest, CAuthCrypto::DigestSize, challenge );
}

/*-------------------------------------------------------------------------*/

/** A value that is put into an endpoint: host name characters and the ones ids have, nothing that changes the structure of a url */
static bool
IsSafeParameterValue( const CORE::CString& value )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( value );
    if ( 0 == size || size > 253 )
        return false;
    const char* p = value.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = p[ i ];
        bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c || ':' == c;
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static const char* const TidSentinel = "\x01tid\x01";

/** Fills the placeholders of the text; {tid} stays when keepTid is set. Empty text gives empty output */
static bool
ExpandValue( const CORE::CString& text, const CORE::CString::StringMap& parameters, bool keepTid, CORE::CString& expanded )
{GUCEF_TRACE;

    expanded.Clear();
    if ( text.IsNULLOrEmpty() )
        return true;

    CORE::CString work = text;
    if ( keepTid )
        work = work.ReplaceSubstr( "{tid}", TidSentinel );

    CORE::CString::StringVector names;
    CAuthProviderDescriptor::GetTemplateNames( work, names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        CORE::CString::StringMap::const_iterator v = parameters.find( names[ i ] );
        if ( v == parameters.end() || !IsSafeParameterValue( (*v).second ) )
            return false;
    }
    if ( !CAuthProviderDescriptor::ExpandTemplate( work, parameters, GUCEF_NULL, false, expanded ) )
        return false;
    if ( keepTid )
        expanded = expanded.ReplaceSubstr( TidSentinel, "{tid}" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The order of preference between what the settings, the discovery document and the descriptor say */
static CORE::CString
PickValue( const CORE::CString& fromSettings, const CORE::CString& fromDescriptor, const CORE::CString& fromDiscovery, bool discoveryFirst )
{GUCEF_TRACE;

    if ( !fromSettings.IsNULLOrEmpty() )
        return fromSettings;
    if ( discoveryFirst && !fromDiscovery.IsNULLOrEmpty() )
        return fromDiscovery;
    if ( !fromDescriptor.IsNULLOrEmpty() )
        return fromDescriptor;
    return fromDiscovery;
}

/*-------------------------------------------------------------------------*/

/** A redirect uri of an application: https, http on the loopback address, or a scheme of the application */
static bool
IsAcceptableRedirectUri( const CORE::CString& uri )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( uri );
    if ( 0 == size || size > CAuthUrl::MaxUrlBytes )
        return false;
    const char* p = uri.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( (unsigned char) p[ i ] <= 0x20 || 0x7F == (unsigned char) p[ i ] || '\\' == p[ i ] || '#' == p[ i ] )
            return false;
    UInt32 colon = 0;
    while ( colon < size && ':' != p[ colon ] )
        ++colon;
    if ( 0 == colon || colon >= size - 1 )
        return false;
    for ( UInt32 i=0; i<colon; ++i )
    {
        char c = p[ i ];
        bool ok = ( c >= 'a' && c <= 'z' ) || ( i > 0 && ( ( c >= '0' && c <= '9' ) || '+' == c || '-' == c || '.' == c ) );
        if ( !ok )
            return false;
    }
    CORE::CString scheme = MakeString( p, colon );
    if ( scheme == "javascript" || scheme == "data" || scheme == "file" || scheme == "ftp" || scheme == "vbscript" )
        return false;
    if ( scheme == "http" )
    {
        CORE::CString s, host, path, query;
        UInt16 port = 0;
        return CAuthUrl::Split( uri, s, host, port, path, query ) && CAuthUrl::IsLoopbackHost( host );
    }
    if ( scheme == "https" )
    {
        CORE::CString s, host, path, query;
        UInt16 port = 0;
        return CAuthUrl::Split( uri, s, host, port, path, query );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::Resolve( const CAuthProviderDescriptor& descriptor  ,
                          const CAuthProviderSettings& settings      ,
                          const COidcMetadata* discovered            ,
                          bool insecureLoopbackAllowed               ,
                          COAuth2Resolved& resolved                  ,
                          CORE::CString& error                       )
{GUCEF_TRACE;

    resolved = COAuth2Resolved();
    error.Clear();

    resolved.providerName = descriptor.name;
    resolved.clientId = settings.clientId;
    resolved.clientSecret = settings.clientSecret;
    settings.GetAcceptedClientIds( resolved.acceptedClientIds );

    // The values of the placeholders: the defaults of the descriptor, the settings on top
    resolved.parameters = descriptor.defaultParameters;
    CAuthProviderSettings::TParameterMap::const_iterator p = settings.parameters.begin();
    while ( p != settings.parameters.end() )
    {
        resolved.parameters[ (*p).first ] = (*p).second;
        ++p;
    }

    bool discoveryFirst = discovered != GUCEF_NULL && !settings.discoveryUrl.IsNULLOrEmpty();
    COidcMetadata none;
    const COidcMetadata& found = GUCEF_NULL != discovered ? *discovered : none;

    struct SField
    {
        const CORE::CString* fromSettings;
        const CORE::CString* fromDescriptor;
        const CORE::CString* fromDiscovery;
        CORE::CString* target;
        bool keepTid;
        const char* name;
    };
    CORE::CString descriptorVerify = descriptor.tokenVerifyEndpoint;
    SField fields[] =
    {
        { &settings.issuer, &descriptor.issuer, &found.issuer, &resolved.issuer, true, "issuer" },
        { &settings.authorizationEndpoint, &descriptor.authorizationEndpoint, &found.authorizationEndpoint, &resolved.authorizationEndpoint, false, "authorizationEndpoint" },
        { &settings.tokenEndpoint, &descriptor.tokenEndpoint, &found.tokenEndpoint, &resolved.tokenEndpoint, false, "tokenEndpoint" },
        { &settings.deviceAuthorizationEndpoint, &descriptor.deviceAuthorizationEndpoint, &found.deviceAuthorizationEndpoint, &resolved.deviceAuthorizationEndpoint, false, "deviceAuthorizationEndpoint" },
        { &settings.userInfoEndpoint, &descriptor.userInfoEndpoint, &found.userInfoEndpoint, &resolved.userInfoEndpoint, false, "userInfoEndpoint" },
        { &settings.jwksUri, &descriptor.jwksUri, &found.jwksUri, &resolved.jwksUri, false, "jwksUri" },
        { &settings.revocationEndpoint, &descriptor.revocationEndpoint, &found.revocationEndpoint, &resolved.revocationEndpoint, false, "revocationEndpoint" },
        { &settings.tokenVerifyEndpoint, &descriptorVerify, &CORE::CString::Empty, &resolved.tokenVerifyEndpoint, false, "tokenVerifyEndpoint" }
    };

    for ( size_t i=0; i<sizeof( fields ) / sizeof( fields[ 0 ] ); ++i )
    {
        // What the settings say does not need the parameters of the descriptor
        CORE::CString a, b;
        bool expanded = ExpandValue( *fields[ i ].fromSettings, resolved.parameters, fields[ i ].keepTid, a );
        if ( expanded && a.IsNULLOrEmpty() )
            expanded = ExpandValue( *fields[ i ].fromDescriptor, resolved.parameters, fields[ i ].keepTid, b );
        if ( !expanded )
        {
            // These are things a provider can be used without
            if ( fields[ i ].target == &resolved.revocationEndpoint || fields[ i ].target == &resolved.tokenVerifyEndpoint )
                continue;
            error = CORE::CString( "The value of " ) + fields[ i ].name + " needs a parameter that is not set or is not acceptable";
            return false;
        }
        *fields[ i ].target = PickValue( a, b, *fields[ i ].fromDiscovery, discoveryFirst );
        if ( !fields[ i ].keepTid && !fields[ i ].target->IsNULLOrEmpty() && !CAuthUrl::IsAcceptableEndpoint( *fields[ i ].target, insecureLoopbackAllowed ) )
        {
            error = CORE::CString( "The value of " ) + fields[ i ].name + " is not an acceptable address: https is required";
            return false;
        }
    }
    if ( !resolved.issuer.IsNULLOrEmpty() )
    {
        // The issuer is compared as text but must still be an address of the same kind
        CORE::CString probe = resolved.issuer.ReplaceSubstr( "{tid}", "tenant" );
        if ( !CAuthUrl::IsAcceptableEndpoint( probe, insecureLoopbackAllowed ) )
        {
            error = "The issuer is not an acceptable address: https is required";
            return false;
        }
    }

    // Discovery
    // The discovery address is only needed when something is missing or the settings ask for it: a descriptor address that
    // needs parameters that are not there is then not an error for a login that has all its endpoints
    CORE::CString discoveryUrl;
    if ( !ExpandValue( settings.discoveryUrl, resolved.parameters, false, discoveryUrl ) )
    {
        error = "The discovery address needs a parameter that is not set or is not acceptable";
        return false;
    }
    if ( discoveryUrl.IsNULLOrEmpty() )
    {
        bool complete = !resolved.tokenEndpoint.IsNULLOrEmpty() &&
                        ( !resolved.authorizationEndpoint.IsNULLOrEmpty() || !resolved.deviceAuthorizationEndpoint.IsNULLOrEmpty() ) &&
                        ( AUTHPROTOCOL_OIDC != descriptor.protocol || !resolved.jwksUri.IsNULLOrEmpty() );
        if ( !ExpandValue( descriptor.discoveryUrl, resolved.parameters, false, discoveryUrl ) && !complete )
        {
            error = "The discovery address needs a parameter that is not set or is not acceptable";
            return false;
        }
    }
    resolved.discoveryUrl = discoveryUrl;
    if ( !discoveryUrl.IsNULLOrEmpty() && !CAuthUrl::IsAcceptableEndpoint( discoveryUrl, insecureLoopbackAllowed ) )
    {
        error = "The discovery address is not acceptable: https is required";
        return false;
    }
    if ( !discoveryUrl.IsNULLOrEmpty() && GUCEF_NULL == discovered )
    {
        bool settingsAsk = !settings.discoveryUrl.IsNULLOrEmpty();
        bool missing = resolved.tokenEndpoint.IsNULLOrEmpty() ||
                       ( resolved.authorizationEndpoint.IsNULLOrEmpty() && resolved.deviceAuthorizationEndpoint.IsNULLOrEmpty() ) ||
                       ( AUTHPROTOCOL_OIDC == descriptor.protocol && resolved.jwksUri.IsNULLOrEmpty() );
        resolved.wantsDiscovery = settingsAsk || missing;
    }

    // Scopes
    resolved.scopes = settings.scopes.empty() ? descriptor.defaultScopes : settings.scopes;

    // The redirect
    resolved.redirectMode = settings.redirectMode.IsNULLOrEmpty() ? CORE::CString( "loopback" ) : settings.redirectMode;
    if ( resolved.redirectMode != "loopback" && resolved.redirectMode != "custom-scheme" && resolved.redirectMode != "manual" )
    {
        error = "The redirect mode is not one of loopback, custom-scheme and manual";
        return false;
    }
    resolved.loopbackPort = settings.loopbackPort;
    resolved.loopbackPath = settings.loopbackPath.IsNULLOrEmpty() ? CORE::CString( "/callback" ) : settings.loopbackPath;
    resolved.redirectUri = settings.redirectUri;
    if ( resolved.redirectMode != "loopback" && !IsAcceptableRedirectUri( resolved.redirectUri ) )
    {
        error = "This redirect mode needs a redirect uri that is https, http on the loopback address or a scheme of the application";
        return false;
    }

    // The relay of an OpenID 2.0 login: the realm and the return address are on a page of the developer's domain
    if ( AUTHPROTOCOL_OPENID2 == descriptor.protocol )
    {
        CORE::CString relayUrl = resolved.parameters.find( "relayUrl" ) != resolved.parameters.end() ? resolved.parameters[ "relayUrl" ] : CORE::CString::Empty;
        if ( !relayUrl.IsNULLOrEmpty() )
        {
            CORE::CString reason;
            if ( !COpenId2Relay::ParseRelayUrl( relayUrl, insecureLoopbackAllowed, resolved.relayBase, resolved.relayRealm, reason ) )
            {
                error = CORE::CString( "The parameter relayUrl is not acceptable: " ) + reason;
                return false;
            }
            if ( resolved.redirectMode != "loopback" || resolved.loopbackPath != COpenId2Relay::CallbackPath ||
                 ( 0 != resolved.loopbackPort && resolved.loopbackPort < COpenId2Relay::MinPort ) )
            {
                resolved.relayBase.Clear();
                resolved.relayRealm.Clear();
                error = "A relay needs the redirect mode loopback, the path /callback and a port from 1024 on (or an ephemeral port)";
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Protocol::MergeScopes( const CORE::CString::StringVector& base, const CORE::CString::StringVector& added, CORE::CString::StringVector& scopes )
{GUCEF_TRACE;

    scopes.clear();
    for ( size_t i=0; i<base.size(); ++i )
        if ( !base[ i ].IsNULLOrEmpty() && !ContainsString( scopes, base[ i ] ) )
            scopes.push_back( base[ i ] );
    for ( size_t i=0; i<added.size(); ++i )
        if ( !added[ i ].IsNULLOrEmpty() && !ContainsString( scopes, added[ i ] ) )
            scopes.push_back( added[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::IsAcceptableLoginHint( const CORE::CString& loginHint )
{GUCEF_TRACE;

    const UInt32 size = GetByteLength( loginHint );
    if ( 0 == size || size > MaxLoginHintBytes )
        return false;
    const char* p = loginHint.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( (unsigned char) p[ i ] < 0x20 || 0x7F == (unsigned char) p[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::FindLoginHint( const CAuthProviderDescriptor& descriptor, const CORE::CString::StringMap& claims, CORE::CString& loginHint )
{GUCEF_TRACE;

    loginHint.Clear();
    if ( !descriptor.supportsLoginHint )
        return false;
    for ( size_t i=0; i<descriptor.loginHintClaims.size(); ++i )
    {
        CORE::CString::StringMap::const_iterator claim = claims.find( descriptor.loginHintClaims[ i ] );
        if ( claim != claims.end() )
        {
            CORE::CString value = (*claim).second.Trim( true ).Trim( false );
            if ( IsAcceptableLoginHint( value ) )
            {
                loginHint = value;
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COAuth2Protocol::BuildAuthorizationUrl( const CAuthProviderDescriptor& descriptor ,
                                        const COAuth2Resolved& resolved           ,
                                        const CORE::CString& redirectUri          ,
                                        const CORE::CString& state                ,
                                        const CORE::CString& nonce                ,
                                        const CORE::CString& codeChallenge        ,
                                        const CORE::CString& loginHint            )
{GUCEF_TRACE;

    CORE::CString::StringMap parameters = descriptor.authorizationParameters;
    parameters[ "response_type" ] = "code";
    parameters[ "client_id" ] = resolved.clientId;
    parameters[ "redirect_uri" ] = redirectUri;
    parameters[ "state" ] = state;

    CORE::CString scope;
    for ( size_t i=0; i<resolved.scopes.size(); ++i )
    {
        if ( i > 0 )
            scope += ' ';
        scope += resolved.scopes[ i ];
    }
    if ( !scope.IsNULLOrEmpty() )
        parameters[ "scope" ] = scope;
    if ( AUTHPROTOCOL_OIDC == descriptor.protocol && !nonce.IsNULLOrEmpty() )
        parameters[ "nonce" ] = nonce;
    if ( descriptor.usesPkce && !codeChallenge.IsNULLOrEmpty() )
    {
        parameters[ "code_challenge" ] = codeChallenge;
        parameters[ "code_challenge_method" ] = "S256";
    }
    if ( descriptor.supportsLoginHint && !descriptor.loginHintParameter.IsNULLOrEmpty() && IsAcceptableLoginHint( loginHint ) )
        parameters[ descriptor.loginHintParameter ] = loginHint;
    return CAuthUrl::AppendQuery( resolved.authorizationEndpoint, parameters );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Protocol::AddClientParameters( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, CORE::CString::StringMap& parameters )
{GUCEF_TRACE;

    parameters[ "client_id" ] = resolved.clientId;
    if ( !resolved.clientSecret.IsEmpty() && descriptor.clientAuthMethod != "basic" )
        parameters[ "client_secret" ] = resolved.clientSecret.Reveal();
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::MakeBasicAuthorization( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, CSecretString& headerValue )
{GUCEF_TRACE;

    headerValue.Clear();
    if ( descriptor.clientAuthMethod != "basic" || resolved.clientSecret.IsEmpty() )
        return false;
    // RFC 6749, 2.3.1: the client id and the secret are form encoded before they are put in the header
    CORE::CString encodedId;
    CORE::CString encodedSecret;
    CORE::CUrlEncoder::Encode( resolved.clientId, encodedId );
    CORE::CUrlEncoder::Encode( resolved.clientSecret.Reveal(), encodedSecret );
    CORE::CString credentials = encodedId + ":" + encodedSecret;
    encodedSecret.Clear();
    CORE::CString encoded = CORE::Base64Encode( credentials.C_String(), GetByteLength( credentials ) );
    headerValue.Set( "Basic " + encoded );
    credentials.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ToCamelCase( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString camel;
    bool upper = false;
    UInt32 size = GetByteLength( name );
    const char* p = name.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '_' == p[ i ] )
            upper = true;
        else
        if ( upper && p[ i ] >= 'a' && p[ i ] <= 'z' )
        {
            camel += (char) ( p[ i ] - 'a' + 'A' );
            upper = false;
        }
        else
        {
            camel += p[ i ];
            upper = false;
        }
    }
    return camel;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COAuth2Protocol::MakeBody( const CAuthProviderDescriptor& descriptor, const CORE::CString::StringMap& parameters, CORE::CString& contentType )
{GUCEF_TRACE;

    if ( !descriptor.jsonRequestBodies )
    {
        contentType = "application/x-www-form-urlencoded";
        return CAuthUrl::EncodeForm( parameters );
    }

    // The body is a data tree of the parameters (the names in camel case) that the json codec of the process writes
    contentType = "application/json";
    CORE::CDataNode root( "body", GUCEF_DATATYPE_OBJECT );
    CORE::CString::StringMap::const_iterator i = parameters.begin();
    while ( i != parameters.end() )
    {
        root.SetAttribute( ToCamelCase( (*i).first ), (*i).second );
        ++i;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
        return CORE::CString::Empty;
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    if ( !codec->StoreDataTree( &root, &access ) )
        return CORE::CString::Empty;
    return MakeString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** A text member by its name in snake case, or in camel case as the JSON style providers have it */
static bool
GetTextEither( const CORE::CDataNode& document, const char* snakeName, CORE::CString& value )
{GUCEF_TRACE;

    if ( CAuthJson::GetString( document, snakeName, value ) )
        return true;
    return CAuthJson::GetString( document, ToCamelCase( snakeName ), value );
}

/*-------------------------------------------------------------------------*/

/** A number member that may also be written as text */
static bool
GetNumberEither( const CORE::CDataNode& document, const char* snakeName, UInt64& value )
{GUCEF_TRACE;

    Int64 number = 0;
    CORE::CString camel = ToCamelCase( snakeName );
    if ( CAuthJson::GetInt64( document, snakeName, number ) || CAuthJson::GetInt64( document, camel, number ) )
    {
        if ( number < 0 )
            return false;
        value = (UInt64) number;
        return true;
    }
    CORE::CString text;
    if ( GetTextEither( document, snakeName, text ) )
    {
        UInt32 size = GetByteLength( text );
        if ( 0 == size || size > 18 )
            return false;
        UInt64 parsed = 0;
        const char* p = text.C_String();
        for ( UInt32 i=0; i<size; ++i )
        {
            if ( p[ i ] < '0' || p[ i ] > '9' )
                return false;
            parsed = parsed * 10 + (UInt64) ( p[ i ] - '0' );
        }
        value = parsed;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::ParseTokenResponse( const void* data, UInt32 size, UInt32 maxSize, COAuth2TokenResponse& response )
{GUCEF_TRACE;

    response = COAuth2TokenResponse();

    CORE::CDataNode root;
    if ( !CAuthJson::Parse( data, size, maxSize, CAuthJson::DefaultMaxDepth, false, root ) )
        return false;
    const CORE::CDataNode& document = CAuthJson::GetDocument( root );

    CORE::CString text;
    if ( GetTextEither( document, "access_token", text ) && !text.IsNULLOrEmpty() )
    {
        response.accessToken.Set( text );
        response.hasAccessToken = true;
    }
    if ( GetTextEither( document, "refresh_token", text ) && !text.IsNULLOrEmpty() )
        response.refreshToken.Set( text );
    if ( GetTextEither( document, "id_token", text ) && !text.IsNULLOrEmpty() )
        response.idToken.Set( text );
    text.Clear();
    GetTextEither( document, "token_type", response.tokenType );
    GetTextEither( document, "scope", response.scope );
    response.hasExpiresIn = GetNumberEither( document, "expires_in", response.expiresInSeconds );

    // The error: a code, or an object as some providers have it
    CORE::CString errorText;
    if ( CAuthJson::GetString( document, "error", errorText ) )
    {
        response.error = errorText;
        GetTextEither( document, "error_description", response.errorDescription );
    }
    else
    {
        const CORE::CDataNode* errorNode = document.FindChild( "error" );
        if ( GUCEF_NULL != errorNode )
        {
            response.error = "invalid_response";
            CORE::CString type;
            if ( CAuthJson::GetString( *errorNode, "type", type ) && !type.IsNULLOrEmpty() )
                response.error = type;
            CAuthJson::GetString( *errorNode, "message", response.errorDescription );
        }
    }

    // The device authorization answer
    if ( GetTextEither( document, "device_code", text ) && !text.IsNULLOrEmpty() )
        response.deviceCode.Set( text );
    GetTextEither( document, "user_code", response.userCode );
    if ( !GetTextEither( document, "verification_uri", response.verificationUri ) )
        GetTextEither( document, "verification_url", response.verificationUri );
    GetTextEither( document, "verification_uri_complete", response.verificationUriComplete );
    UInt64 interval = 0;
    if ( GetNumberEither( document, "interval", interval ) )
    {
        response.hasInterval = true;
        response.interval = interval > 3600 ? 3600 : (UInt32) interval;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

TAuthFailureReason
COAuth2Protocol::ReasonForError( const CORE::CString& errorCode )
{GUCEF_TRACE;

    if ( errorCode == "invalid_grant" )
        return AUTHFAIL_INVALID_GRANT;
    if ( errorCode == "access_denied" )
        return AUTHFAIL_DENIED;
    if ( errorCode == "expired_token" )
        return AUTHFAIL_TIMEOUT;
    if ( errorCode == "temporarily_unavailable" || errorCode == "server_error" )
        return AUTHFAIL_NETWORK;
    return AUTHFAIL_INVALID_RESPONSE;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Protocol::IsTenantAllowed( const CORE::CString& tenantSetting, const CORE::CString& tenantId )
{GUCEF_TRACE;

    if ( tenantId.IsNULLOrEmpty() )
        return false;
    CORE::CString setting = tenantSetting.Lowercase();
    CORE::CString tenant = tenantId.Lowercase();
    if ( setting.IsNULLOrEmpty() || setting == "common" )
        return true;
    if ( setting == "organizations" )
        return tenant != ConsumersTenantId;
    if ( setting == "consumers" )
        return tenant == ConsumersTenantId;
    return tenant == setting;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
