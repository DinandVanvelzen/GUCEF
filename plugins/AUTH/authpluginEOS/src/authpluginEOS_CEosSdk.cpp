/*
 *  authpluginEOS: gucefAUTH plugin for logging in with Epic Online Services
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifdef GUCEF_AUTH_HAS_EOS

// The headers of the Epic Online Services SDK, found through the include path the build was given
#include "eos_sdk.h"
#include "eos_auth.h"
#include "eos_connect.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#endif /* GUCEF_AUTH_HAS_EOS ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#include "pluginglueEOS_CEosPlatformSdk.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H ? */

#include "authpluginEOS_CEosSdk.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

#ifndef GUCEF_AUTH_HAS_EOS

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITHOUT THE SDK                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CEosSdk::SImpl
{
    int unused;
};

/*-------------------------------------------------------------------------*/

CEosSdk::CEosSdk( void )
    : m_impl( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosSdk::~CEosSdk()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::FillShim( TEosShim& shim )
{GUCEF_TRACE;

    shim = TEosShim();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::AdoptPlatform( void* platformHandle )
{GUCEF_TRACE;

    return false;
}

#else /* GUCEF_AUTH_HAS_EOS */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD WITH THE SDK. NEVER COMPILED OR RUN WHERE THIS WAS WRITTEN   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data that travels with a request through the SDK to its callback */
struct SRequestData
{
    CEosSdk::SImpl* sdk;
    UInt32 requestId;
};

/*-------------------------------------------------------------------------*/

struct CEosSdk::SImpl
{
    MT::CMutex lock;
    EOS_ContinuanceToken continuanceToken;
    GUCEF::vector< TEosEvent > events;

    SImpl( void ) : lock(), continuanceToken( GUCEF_NULL ), events() {}

    void Push( const TEosEvent& event )
    {
        MT::CScopeMutex scopeLock( lock );
        if ( events.size() < 64 )
            events.push_back( event );
    }
};

/*-------------------------------------------------------------------------*/

static void
CopyText( const char* text, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == buffer || 0 == bufferSize )
        return;
    size_t length = GUCEF_NULL != text ? strlen( text ) : 0;
    if ( length >= bufferSize )
        length = bufferSize - 1;
    if ( length > 0 )
        memcpy( buffer, text, length );
    buffer[ length ] = '\0';
}

/*-------------------------------------------------------------------------*/

static EEosOutcome
TranslateResult( EOS_EResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case EOS_Success :                  return EOSOUTCOME_SUCCESS;
        case EOS_Canceled :                 return EOSOUTCOME_CANCELLED;
        case EOS_AccessDenied :             return EOSOUTCOME_DENIED;
        case EOS_InvalidCredentials :
        case EOS_InvalidAuth :              return EOSOUTCOME_INVALID_CREDENTIALS;
        case EOS_NoConnection :
        case EOS_NetworkDisconnected :      return EOSOUTCOME_NO_CONNECTION;
        case EOS_TimedOut :                 return EOSOUTCOME_TIMEOUT;
        default :                           return EOSOUTCOME_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

static void
EpicAccountIdToText( EOS_EpicAccountId id, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    buffer[ 0 ] = '\0';
    if ( GUCEF_NULL == id || !EOS_EpicAccountId_IsValid( id ) )
        return;
    int32_t length = (int32_t) bufferSize;
    if ( EOS_Success != EOS_EpicAccountId_ToString( id, buffer, &length ) )
        buffer[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

static void
ProductUserIdToText( EOS_ProductUserId id, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    buffer[ 0 ] = '\0';
    if ( GUCEF_NULL == id || !EOS_ProductUserId_IsValid( id ) )
        return;
    int32_t length = (int32_t) bufferSize;
    if ( EOS_Success != EOS_ProductUserId_ToString( id, buffer, &length ) )
        buffer[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnAuthLogin( const EOS_Auth_LoginCallbackInfo* data )
{GUCEF_TRACE;

    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    TEosEvent event;
    event.requestId = request->requestId;
    event.sdkResultCode = (Int32) data->ResultCode;

    // The pin grant is an intermediate answer: the request goes on and ends with another callback
    if ( EOS_Auth_PinGrantCode == data->ResultCode && GUCEF_NULL != data->PinGrantInfo )
    {
        event.kind = EOSEVENT_PIN_GRANT;
        event.outcome = EOSOUTCOME_SUCCESS;
        CopyText( data->PinGrantInfo->UserCode, event.userCode, sizeof( event.userCode ) );
        CopyText( data->PinGrantInfo->VerificationURI, event.verificationUri, sizeof( event.verificationUri ) );
        CopyText( data->PinGrantInfo->VerificationURIComplete, event.verificationUriComplete, sizeof( event.verificationUriComplete ) );
        event.expiresInSeconds = data->PinGrantInfo->ExpiresIn > 0 ? (UInt32) data->PinGrantInfo->ExpiresIn : 0;
        request->sdk->Push( event );
        return;
    }

    event.kind = EOSEVENT_AUTH_LOGIN;
    event.outcome = TranslateResult( data->ResultCode );
    if ( EOS_Success == data->ResultCode )
        EpicAccountIdToText( data->LocalUserId, event.epicAccountId, sizeof( event.epicAccountId ) );
    request->sdk->Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectLogin( const EOS_Connect_LoginCallbackInfo* data )
{GUCEF_TRACE;

    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    TEosEvent event;
    event.requestId = request->requestId;
    event.kind = EOSEVENT_CONNECT_LOGIN;
    event.sdkResultCode = (Int32) data->ResultCode;
    event.outcome = TranslateResult( data->ResultCode );

    // EOS_InvalidUser with a continuance token: the external account has no product user yet
    if ( EOS_InvalidUser == data->ResultCode && GUCEF_NULL != data->ContinuanceToken )
    {
        request->sdk->continuanceToken = data->ContinuanceToken;
        event.outcome = EOSOUTCOME_NEEDS_USER_CREATION;
    }
    if ( EOS_Success == data->ResultCode )
        ProductUserIdToText( data->LocalUserId, event.productUserId, sizeof( event.productUserId ) );
    request->sdk->Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectCreateUser( const EOS_Connect_CreateUserCallbackInfo* data )
{GUCEF_TRACE;

    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    TEosEvent event;
    event.requestId = request->requestId;
    event.kind = EOSEVENT_CONNECT_CREATE_USER;
    event.sdkResultCode = (Int32) data->ResultCode;
    event.outcome = TranslateResult( data->ResultCode );
    if ( EOS_Success == data->ResultCode )
        ProductUserIdToText( data->LocalUserId, event.productUserId, sizeof( event.productUserId ) );
    request->sdk->continuanceToken = GUCEF_NULL;
    request->sdk->Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnConnectLinkAccount( const EOS_Connect_LinkAccountCallbackInfo* data )
{GUCEF_TRACE;

    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    TEosEvent event;
    event.requestId = request->requestId;
    event.kind = EOSEVENT_CONNECT_LINK_ACCOUNT;
    event.sdkResultCode = (Int32) data->ResultCode;
    event.outcome = TranslateResult( data->ResultCode );
    if ( EOS_Success == data->ResultCode )
        ProductUserIdToText( data->LocalUserId, event.productUserId, sizeof( event.productUserId ) );
    request->sdk->continuanceToken = GUCEF_NULL;
    request->sdk->Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnAuthLogout( const EOS_Auth_LogoutCallbackInfo* data )
{GUCEF_TRACE;

    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    TEosEvent event;
    event.requestId = request->requestId;
    event.kind = EOSEVENT_LOGOUT;
    event.sdkResultCode = (Int32) data->ResultCode;
    event.outcome = TranslateResult( data->ResultCode );
    request->sdk->Push( event );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

static void EOS_CALL
OnDeletePersistentAuth( const EOS_Auth_DeletePersistentAuthCallbackInfo* data )
{GUCEF_TRACE;

    // The outcome of removing the stored token has no one waiting for it
    SRequestData* request = static_cast< SRequestData* >( data->ClientData );
    GUCEF_DELETE request;
}

/*-------------------------------------------------------------------------*/

/** The platform that the glue (pluginglueEOS) made; the interfaces of the SDK are reached through it */
static EOS_HPlatform
Platform( void )
{GUCEF_TRACE;

    return static_cast< EOS_HPlatform >( PLUGINGLUE::EOS::CEosPlatformSdk::GetPlatformHandle() );
}

/*-------------------------------------------------------------------------*/

static bool
ShimPollEvent( void* context, SEosEvent* event )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    MT::CScopeMutex scopeLock( sdk->lock );
    if ( sdk->events.empty() )
        return false;
    *event = sdk->events.front();
    sdk->events.erase( sdk->events.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

static SRequestData*
MakeRequest( CEosSdk::SImpl* sdk, UInt32 requestId )
{GUCEF_TRACE;

    SRequestData* request = GUCEF_NEW SRequestData();
    request->sdk = sdk;
    request->requestId = requestId;
    return request;
}

/*-------------------------------------------------------------------------*/

static bool
ShimAuthLogin( void* context, UInt32 requestId, EEosLoginType type )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform )
        return false;

    EOS_Auth_Credentials credentials;
    memset( &credentials, 0, sizeof( credentials ) );
    credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    switch ( type )
    {
        case EOSLOGIN_PERSISTENT :      credentials.Type = EOS_LCT_PersistentAuth; break;
        case EOSLOGIN_ACCOUNT_PORTAL :  credentials.Type = EOS_LCT_AccountPortal; break;
        case EOSLOGIN_DEVICE_CODE :     credentials.Type = EOS_LCT_DeviceCode; break;
        default :                       return false;
    }

    EOS_Auth_LoginOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
    options.Credentials = &credentials;
    options.ScopeFlags = EOS_AS_BasicProfile;

    EOS_HAuth auth = EOS_Platform_GetAuthInterface( platform );
    EOS_Auth_Login( auth, &options, MakeRequest( sdk, requestId ), &OnAuthLogin );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ShimAuthCopyIdToken( void* context, const char* epicAccountId, char* buffer, UInt32 bufferSize )
{GUCEF_TRACE;

    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform || GUCEF_NULL == epicAccountId )
        return false;

    EOS_EpicAccountId account = EOS_EpicAccountId_FromString( epicAccountId );
    if ( GUCEF_NULL == account )
        return false;

    EOS_Auth_CopyIdTokenOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
    options.AccountId = account;

    EOS_Auth_IdToken* token = GUCEF_NULL;
    EOS_HAuth auth = EOS_Platform_GetAuthInterface( platform );
    if ( EOS_Success != EOS_Auth_CopyIdToken( auth, &options, &token ) || GUCEF_NULL == token )
        return false;

    bool fits = GUCEF_NULL != token->JsonWebToken && strlen( token->JsonWebToken ) < bufferSize;
    if ( fits )
        CopyText( token->JsonWebToken, buffer, bufferSize );
    EOS_Auth_IdToken_Release( token );
    return fits;
}

/*-------------------------------------------------------------------------*/

static bool
ShimConnectLogin( void* context, UInt32 requestId, const char* idToken )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform || GUCEF_NULL == idToken )
        return false;

    EOS_Connect_Credentials credentials;
    memset( &credentials, 0, sizeof( credentials ) );
    credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
    credentials.Token = idToken;
    credentials.Type = EOS_ECT_EPIC_ID_TOKEN;

    EOS_Connect_LoginOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
    options.Credentials = &credentials;

    EOS_HConnect connect = EOS_Platform_GetConnectInterface( platform );
    EOS_Connect_Login( connect, &options, MakeRequest( sdk, requestId ), &OnConnectLogin );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ShimConnectCreateUser( void* context, UInt32 requestId )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform || GUCEF_NULL == sdk->continuanceToken )
        return false;

    EOS_Connect_CreateUserOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
    options.ContinuanceToken = sdk->continuanceToken;

    EOS_HConnect connect = EOS_Platform_GetConnectInterface( platform );
    EOS_Connect_CreateUser( connect, &options, MakeRequest( sdk, requestId ), &OnConnectCreateUser );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ShimConnectLinkAccount( void* context, UInt32 requestId, const char* productUserId )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform || GUCEF_NULL == sdk->continuanceToken || GUCEF_NULL == productUserId )
        return false;

    EOS_ProductUserId localUser = EOS_ProductUserId_FromString( productUserId );
    if ( GUCEF_NULL == localUser )
        return false;

    EOS_Connect_LinkAccountOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_CONNECT_LINKACCOUNT_API_LATEST;
    options.LocalUserId = localUser;
    options.ContinuanceToken = sdk->continuanceToken;

    EOS_HConnect connect = EOS_Platform_GetConnectInterface( platform );
    EOS_Connect_LinkAccount( connect, &options, MakeRequest( sdk, requestId ), &OnConnectLinkAccount );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ShimAuthLogout( void* context, UInt32 requestId, const char* epicAccountId, bool deletePersistentAuth )
{GUCEF_TRACE;

    CEosSdk::SImpl* sdk = static_cast< CEosSdk::SImpl* >( context );
    EOS_HPlatform platform = Platform();
    if ( GUCEF_NULL == platform || GUCEF_NULL == epicAccountId )
        return false;

    EOS_HAuth auth = EOS_Platform_GetAuthInterface( platform );
    if ( deletePersistentAuth )
    {
        EOS_Auth_DeletePersistentAuthOptions deleteOptions;
        memset( &deleteOptions, 0, sizeof( deleteOptions ) );
        deleteOptions.ApiVersion = EOS_AUTH_DELETEPERSISTENTAUTH_API_LATEST;
        EOS_Auth_DeletePersistentAuth( auth, &deleteOptions, MakeRequest( sdk, 0 ), &OnDeletePersistentAuth );
    }

    EOS_EpicAccountId account = EOS_EpicAccountId_FromString( epicAccountId );
    if ( GUCEF_NULL == account )
        return false;
    EOS_Auth_LogoutOptions options;
    memset( &options, 0, sizeof( options ) );
    options.ApiVersion = EOS_AUTH_LOGOUT_API_LATEST;
    options.LocalUserId = account;
    EOS_Auth_Logout( auth, &options, MakeRequest( sdk, requestId ), &OnAuthLogout );
    return true;
}

/*-------------------------------------------------------------------------*/

CEosSdk::CEosSdk( void )
    : m_impl( GUCEF_NEW SImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosSdk::~CEosSdk()
{GUCEF_TRACE;

    GUCEF_DELETE m_impl;
    m_impl = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::IsCompiledIn( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::FillShim( TEosShim& shim )
{GUCEF_TRACE;

    shim = TEosShim();
    shim.context = m_impl;
    shim.pollEvent = &ShimPollEvent;
    shim.authLogin = &ShimAuthLogin;
    shim.authCopyIdToken = &ShimAuthCopyIdToken;
    shim.connectLogin = &ShimConnectLogin;
    shim.connectCreateUser = &ShimConnectCreateUser;
    shim.connectLinkAccount = &ShimConnectLinkAccount;
    shim.authLogout = &ShimAuthLogout;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosSdk::AdoptPlatform( void* platformHandle )
{GUCEF_TRACE;

    // The platform belongs to the glue, which shares it with the other EOS plugins; the application keeps ticking it
    return PLUGINGLUE::EOS::CEosPlatformSdk::AdoptPlatform( platformHandle, true );
}

#endif /* GUCEF_AUTH_HAS_EOS ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
