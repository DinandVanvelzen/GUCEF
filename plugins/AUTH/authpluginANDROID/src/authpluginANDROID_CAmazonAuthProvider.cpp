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

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H
#include "authpluginANDROID_CAndroidAuthUtils.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H ? */

#include "authpluginANDROID_CAmazonAuthProvider.h"

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

const CORE::CString CAmazonAuthProvider::ProviderName = "amazon";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAmazonAuthProvider::CAmazonAuthProvider( void )
    : CAndroidAuthProviderBase()
    , m_settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAmazonAuthProvider::~CAmazonAuthProvider()
{GUCEF_TRACE;

    // No answer reaches this provider after this, and none is in the middle of being handed over
    Shutdown();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAmazonAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

UInt32
CAmazonAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE;
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    return GUCEF_NULL != shim.amazonSignIn && GUCEF_NULL != shim.isAmazonAvailable && shim.isAmazonAvailable( shim.context );
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::CanStart( bool interactive, CORE::CString& reason ) const
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.amazonSignIn )
    {
        reason = "This is not an Android device with the Java side of the plugin";
        return false;
    }
    if ( GUCEF_NULL != shim.isAmazonAvailable && !shim.isAmazonAvailable( shim.context ) )
    {
        reason = "Login with Amazon is not available on this device";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    UInt32 timeout = 0;
    {
        MT::CScopeMutex lock( m_lock );
        timeout = m_settings.silentTimeoutInMs;
    }
    return StartRequest( false, timeout, CORE::CString::Empty, future );
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return StartRequest( true, options.timeoutInMs > 0 ? options.timeoutInMs : AUTH::CAuthOptions::DefaultTimeoutInMs, CORE::CString::Empty, future );
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::StartNative( UInt32 requestId, bool interactive, const CORE::CString& nonce )
{GUCEF_TRACE;

    CORE::CString scopes;
    {
        MT::CScopeMutex lock( m_lock );
        scopes = m_settings.amazonScopes;
    }

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.amazonSignIn )
        return false;
    return shim.amazonSignIn( shim.context, requestId, interactive ? ANDROIDSIGNIN_INTERACTIVE : ANDROIDSIGNIN_SILENT, scopes.C_String() );
}

/*-------------------------------------------------------------------------*/

void
CAmazonAuthProvider::HandleResult( SPending& pending, const SAndroidAuthResult& result )
{GUCEF_TRACE;

    if ( ANDROIDAUTH_OK != result.status )
    {
        FailLogin( pending.source, ToFailureReason( result.status ), result.message.IsNULLOrEmpty() ? CORE::CString( "Login with Amazon did not finish" ) : result.message );
        return;
    }

    AUTH::CAccountRef account( AUTH::CAuthProviderIds::Amazon, AUTH::CAccountRef::EncodeSubject( result.accountId ) );
    if ( result.accessToken.IsEmpty() || result.accountId.IsNULLOrEmpty() || !account.IsValid() )
    {
        FailLogin( pending.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "Login with Amazon gave no access token and user id" );
        return;
    }

    AUTH::CAuthResult login;
    login.accountRef = account;
    login.displayName = result.displayName;
    login.avatarUrl = result.avatarUrl;
    login.accessToken = result.accessToken;
    login.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, result.accessToken.Reveal() );
    CompleteLogin( pending.source, login );
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken )
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL != shim.amazonSignOut )
        shim.amazonSignOut( shim.context );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    if ( proof.kind != AUTH::CAuthProof::KindPlatformTicket || proof.data.IsEmpty() )
        return false;

    CAndroidAuthSettings settings;
    {
        MT::CScopeMutex lock( m_lock );
        settings = m_settings;
    }

    // A token that was issued to another app of Amazon would otherwise be accepted: the audience is checked
    if ( settings.clientId.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: An access token cannot be verified without the setting clientId (the audience)" );
        return false;
    }
    if ( !CAndroidAuthUtils::IsAcceptableUrl( settings.tokenInfoEndpoint, settings.allowInsecureHttp ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The tokenInfoEndpoint is not an https url, an access token is not verified over it" );
        return false;
    }

    CORE::CString encoded;
    CORE::CUrlEncoder::Encode( proof.data.Reveal(), encoded );
    CORE::CString url = settings.tokenInfoEndpoint;
    url += url.HasChar( '?' ) >= 0 ? "&access_token=" : "?access_token=";
    url += encoded;

    Int32 status = 0;
    CORE::CString answer;
    if ( !CAndroidAuthUtils::Execute( "GET", url, CORE::CString::Empty, CORE::CString::Empty, settings.httpTimeoutInMs, status, answer ) )
        return false;
    if ( 200 != status )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The token info endpoint refused the access token, status " << status );
        return false;
    }

    CORE::CDataNode root;
    if ( !CAndroidAuthUtils::ParseJson( answer, root ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The token info endpoint gave an answer that is not json" );
        return false;
    }

    CORE::CString audience = CAndroidAuthUtils::GetString( root, "aud" );
    if ( audience != settings.clientId )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The access token was not issued to this app" );
        return false;
    }

    // exp is the number of seconds that the token is still valid, according to the documentation. To confirm
    const CORE::CDataNode::TKeyValuePair* expiry = root.GetAttribute( "exp" );
    if ( GUCEF_NULL != expiry && !expiry->second.IsNULL() && expiry->second.AsInt64() <= 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The access token has run out" );
        return false;
    }

    CORE::CString userId = CAndroidAuthUtils::GetString( root, "user_id" );
    AUTH::CAccountRef account( AUTH::CAuthProviderIds::Amazon, AUTH::CAccountRef::EncodeSubject( userId ) );
    if ( userId.IsNULLOrEmpty() || !account.IsValid() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: The token info has no usable user id" );
        return false;
    }

    result = AUTH::CAuthResult();
    result.providerName = ProviderName;
    result.accountRef = account;
    result.proof = proof;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AmazonAuthProvider: Access token verified for " << account.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

CAndroidAuthSettings
CAmazonAuthProvider::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CAmazonAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAmazonAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAmazonAuthProvider";
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
