/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H
#include "authpluginANDROID_CAndroidAuthUtils.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H ? */

#include "authpluginANDROID_CGoogleAuthProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CGoogleAuthProvider::ProviderName = "google";

// The issuers Google puts in id tokens
static const char* const GoogleIssuer = "https://accounts.google.com";
static const char* const GoogleIssuerShort = "accounts.google.com";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGoogleAuthProvider::CGoogleAuthProvider( void )
    : CAndroidAuthProviderBase()
    , m_settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGoogleAuthProvider::~CGoogleAuthProvider()
{GUCEF_TRACE;

    // No answer reaches this provider after this, and none is in the middle of being handed over
    Shutdown();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGoogleAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

UInt32
CGoogleAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE | AUTH::AUTHCAP_REQUIRES_CLIENT_ID;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.googleSignIn || GUCEF_NULL == shim.isGooglePlayAvailable || !shim.isGooglePlayAvailable( shim.context ) )
        return false;

    MT::CScopeMutex lock( m_lock );
    return !m_settings.clientId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::CanStart( bool interactive, CORE::CString& reason ) const
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.googleSignIn )
    {
        reason = "This is not an Android device with the Java side of the plugin";
        return false;
    }
    if ( GUCEF_NULL != shim.isGooglePlayAvailable && !shim.isGooglePlayAvailable( shim.context ) )
    {
        reason = "Google Play services are not available on this device";
        return false;
    }
    if ( m_settings.clientId.IsNULLOrEmpty() )
    {
        reason = "The web client id of the server is not set (clientId)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    UInt32 timeout = 0;
    CORE::CString nonce;
    {
        MT::CScopeMutex lock( m_lock );
        timeout = m_settings.silentTimeoutInMs;
        if ( m_settings.useCredentialManager )
            nonce = CAndroidAuthUtils::MakeNonce();
    }
    return StartRequest( false, timeout, nonce, future );
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CString nonce;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_settings.useCredentialManager )
            nonce = CAndroidAuthUtils::MakeNonce();
    }
    return StartRequest( true, options.timeoutInMs > 0 ? options.timeoutInMs : AUTH::CAuthOptions::DefaultTimeoutInMs, nonce, future );
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::StartNative( UInt32 requestId, bool interactive, const CORE::CString& nonce )
{GUCEF_TRACE;

    CORE::CString clientId;
    bool useCredentialManager = false;
    {
        MT::CScopeMutex lock( m_lock );
        clientId = m_settings.clientId;
        useCredentialManager = m_settings.useCredentialManager;
    }

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.googleSignIn )
        return false;
    return shim.googleSignIn( shim.context, requestId, interactive ? ANDROIDSIGNIN_INTERACTIVE : ANDROIDSIGNIN_SILENT,
                              clientId.C_String(), useCredentialManager, nonce.C_String() );
}

/*-------------------------------------------------------------------------*/

void
CGoogleAuthProvider::HandleResult( SPending& pending, const SAndroidAuthResult& result )
{GUCEF_TRACE;

    if ( ANDROIDAUTH_OK != result.status )
    {
        FailLogin( pending.source, ToFailureReason( result.status ), result.message.IsNULLOrEmpty() ? CORE::CString( "Google sign in did not finish" ) : result.message );
        return;
    }

    CAndroidAuthSettings settings;
    {
        MT::CScopeMutex lock( m_lock );
        settings = m_settings;
    }

    // Play Games says that the person is signed in and gives a code, not who the person is: that is in the id token
    if ( result.idToken.IsEmpty() )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_NOT_AVAILABLE, "Google gave no id token: the account id comes from Credential Manager (setting useCredentialManager, and the account has to be authorized for the app)" );
        return;
    }

    CORE::CDataNode claims;
    if ( !CAndroidAuthUtils::DecodeJwtClaims( result.idToken.Reveal(), claims ) )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "The Google id token cannot be read" );
        return;
    }

    // The checks of a client on its own token: they guard against a broken Java side, a server verifies for real
    CORE::CString issuer = CAndroidAuthUtils::GetString( claims, "iss" );
    CORE::CString audience = CAndroidAuthUtils::GetString( claims, "aud" );
    CORE::CString tokenNonce = CAndroidAuthUtils::GetString( claims, "nonce" );
    CORE::CString subject = CAndroidAuthUtils::GetString( claims, "sub" );
    const CORE::CDataNode::TKeyValuePair* expiry = claims.GetAttribute( "exp" );

    if ( ( !issuer.IsNULLOrEmpty() && issuer != GoogleIssuer && issuer != GoogleIssuerShort ) ||
         ( !audience.IsNULLOrEmpty() && audience != settings.clientId ) )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "The Google id token is not for this app" );
        return;
    }
    if ( !pending.nonce.IsNULLOrEmpty() && tokenNonce != pending.nonce )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "The Google id token is not the one that was asked for (nonce)" );
        return;
    }
    if ( GUCEF_NULL != expiry && !expiry->second.IsNULL() && expiry->second.AsUInt64() * 1000 <= AUTH::GetEpochTimeInMs() )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "The Google id token has expired" );
        return;
    }

    AUTH::CAccountRef account( AUTH::CAuthProviderIds::Google, AUTH::CAccountRef::EncodeSubject( subject ) );
    if ( subject.IsNULLOrEmpty() || !account.IsValid() )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "The Google id token has no usable subject" );
        return;
    }

    AUTH::CAuthResult login;
    login.accountRef = account;
    login.displayName = result.displayName.IsNULLOrEmpty() ? CAndroidAuthUtils::GetString( claims, "name" ) : result.displayName;
    login.avatarUrl = result.avatarUrl.IsNULLOrEmpty() ? CAndroidAuthUtils::GetString( claims, "picture" ) : result.avatarUrl;
    login.idToken = result.idToken;
    login.scopes.push_back( "openid" );

    if ( settings.proofKind == "auth-code" && !result.authCode.IsEmpty() )
        login.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, result.authCode.Reveal() );
    else
        login.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, result.idToken.Reveal() );

    CompleteLogin( pending.source, login );
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken )
{GUCEF_TRACE;

    // Play Games has no sign out; the credential state of the app is cleared so that the next silent login asks again
    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL != shim.googleSignOut )
        shim.googleSignOut( shim.context );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::ExchangeCode( const CAndroidAuthSettings& settings, const AUTH::CSecretString& code, AUTH::CSecretString& idToken ) const
{GUCEF_TRACE;

    if ( settings.clientId.IsNULLOrEmpty() || settings.clientSecret.IsEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: A server auth code cannot be exchanged without the settings clientId and clientSecret" );
        return false;
    }
    if ( !CAndroidAuthUtils::IsAcceptableUrl( settings.tokenEndpoint, settings.allowInsecureHttp ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: The tokenEndpoint is not an https url, a server auth code is not exchanged over it" );
        return false;
    }

    // The code, the client id and the secret go in the body of the request, nowhere else
    CORE::CString body = "code=";
    CORE::CString encoded;
    CORE::CUrlEncoder::Encode( code.Reveal(), encoded );
    body += encoded;
    body += "&client_id=";
    CORE::CUrlEncoder::Encode( settings.clientId, encoded );
    body += encoded;
    body += "&client_secret=";
    CORE::CUrlEncoder::Encode( settings.clientSecret.Reveal(), encoded );
    body += encoded;
    body += "&grant_type=authorization_code";

    Int32 status = 0;
    CORE::CString answer;
    if ( !CAndroidAuthUtils::Execute( "POST", settings.tokenEndpoint, "application/x-www-form-urlencoded", body, settings.httpTimeoutInMs, status, answer ) )
        return false;

    CORE::CDataNode root;
    if ( !CAndroidAuthUtils::ParseJson( answer, root ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: The token endpoint gave an answer that is not json (status " << status << ")" );
        return false;
    }
    if ( 200 != status )
    {
        // The error code of the endpoint (invalid_grant, ...) is no secret
        CORE::CString error = CAndroidAuthUtils::GetString( root, "error" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: The server auth code was not accepted, status " << status << " " << error.SubstrFromRange( 0, error.Length() > 64 ? 64 : error.Length() ) );
        return false;
    }

    CORE::CString token = CAndroidAuthUtils::GetString( root, "id_token" );
    if ( token.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: The token endpoint gave no id_token: the code was requested without the openid scope" );
        return false;
    }
    idToken.Set( token );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    if ( proof.data.IsEmpty() )
        return false;

    CAndroidAuthSettings settings;
    {
        MT::CScopeMutex lock( m_lock );
        settings = m_settings;
    }

    AUTH::CAuthProof idProof;
    if ( proof.kind == AUTH::CAuthProof::KindOidcIdToken )
    {
        idProof = proof;
    }
    else
    if ( proof.kind == AUTH::CAuthProof::KindPlatformTicket )
    {
        AUTH::CSecretString idToken;
        if ( !ExchangeCode( settings, proof.data, idToken ) )
            return false;
        idProof = AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, idToken.Reveal() );
    }
    else
    {
        return false;
    }

    AUTH::CAuthResult verified;
    if ( !CAndroidAuthUtils::VerifyWithEngine( AUTH::CAuthProviderRegistry::GoogleName, settings.GetEngineSettings(), idProof, verified ) )
        return false;

    if ( !verified.accountRef.IsValid() || verified.accountRef.GetProvider() != AUTH::CAuthProviderIds::Google )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GoogleAuthProvider: The verified token is not for a Google account" );
        return false;
    }

    result = verified;
    result.providerName = ProviderName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::IsVerifySupported( void ) const
{GUCEF_TRACE;

    AUTH::IAuthFlowEngine* engine = AUTH::CAuthGlobal::Instance()->GetFlowEngine();
    if ( GUCEF_NULL == engine )
        return false;
    AUTH::CAuthProviderDescriptor descriptor;
    if ( !AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::GoogleName, descriptor ) )
        return false;
    MT::CScopeMutex lock( m_lock );
    return engine->IsSupported( descriptor, m_settings.GetEngineSettings() );
}

/*-------------------------------------------------------------------------*/

CAndroidAuthSettings
CGoogleAuthProvider::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CGoogleAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGoogleAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
