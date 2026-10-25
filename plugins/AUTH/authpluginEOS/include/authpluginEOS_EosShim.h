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

#ifndef GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#define GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_EOS_MACROS_H
#include "authpluginEOS_macros.h"
#define GUCEF_AUTHPLUGIN_EOS_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_EOS_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The ways of EOS_Auth_Login that the provider uses */
enum EEosLoginType
{
    EOSLOGIN_PERSISTENT = 0,        /**< EOS_LCT_PersistentAuth: the token that the SDK stored at an earlier login, no user interaction */
    EOSLOGIN_ACCOUNT_PORTAL,        /**< EOS_LCT_AccountPortal: the SDK sends the user to the Epic account portal in a browser */
    EOSLOGIN_DEVICE_CODE            /**< a login with a code the user enters on another device; the SDK reports the pin grant info */
};

/** What an event that the SDK delivered is about */
enum EEosEventKind
{
    EOSEVENT_AUTH_LOGIN = 0,        /**< the end of EOS_Auth_Login */
    EOSEVENT_PIN_GRANT,             /**< the device code of EOS_Auth_Login: userCode and the uris; the login goes on */
    EOSEVENT_CONNECT_LOGIN,         /**< the end of EOS_Connect_Login */
    EOSEVENT_CONNECT_CREATE_USER,   /**< the end of EOS_Connect_CreateUser */
    EOSEVENT_CONNECT_LINK_ACCOUNT,  /**< the end of EOS_Connect_LinkAccount */
    EOSEVENT_LOGOUT                 /**< the end of EOS_Auth_Logout */
};

/**
 *  How an operation ended. The real binding translates the EOS_EResult of the SDK, by the names of the SDK headers, so
 *  that the provider does not depend on the numbers of the SDK.
 */
enum EEosOutcome
{
    EOSOUTCOME_SUCCESS = 0,
    EOSOUTCOME_CANCELLED,           /**< EOS_Canceled */
    EOSOUTCOME_DENIED,              /**< the user or the account refused: EOS_AccessDenied, the account is locked or restricted */
    EOSOUTCOME_INVALID_CREDENTIALS, /**< there is no stored token or it no longer works: EOS_InvalidCredentials, EOS_InvalidAuth */
    EOSOUTCOME_NO_CONNECTION,       /**< EOS_NoConnection, EOS_NetworkDisconnected */
    EOSOUTCOME_TIMEOUT,             /**< EOS_TimedOut, the pin grant expired */
    EOSOUTCOME_NEEDS_USER_CREATION, /**< EOS_Connect_Login: the external account has no product user yet (EOS_InvalidUser with a continuance token) */
    EOSOUTCOME_FAILED               /**< anything else */
};

/** The longest ids and tokens that an event can carry (an EOS id is 32 hex digits) */
static const UInt32 EosMaxIdLength = 64;
static const UInt32 EosMaxUriLength = 512;

/** An event that the SDK delivered while it was ticked */
struct SEosEvent
{
    UInt32 requestId;
    EEosEventKind kind;
    EEosOutcome outcome;
    Int32 sdkResultCode;                        /**< EOS_EResult as it came, for the log */
    char epicAccountId[ EosMaxIdLength ];       /**< EOSEVENT_AUTH_LOGIN: EOS_EpicAccountId_ToString */
    char productUserId[ EosMaxIdLength ];       /**< EOSEVENT_CONNECT_LOGIN, EOSEVENT_CONNECT_CREATE_USER and EOSEVENT_CONNECT_LINK_ACCOUNT: EOS_ProductUserId_ToString */
    char userCode[ 64 ];                        /**< EOSEVENT_PIN_GRANT */
    char verificationUri[ EosMaxUriLength ];
    char verificationUriComplete[ EosMaxUriLength ];
    UInt32 expiresInSeconds;

    SEosEvent( void ) : requestId( 0 ), kind( EOSEVENT_AUTH_LOGIN ), outcome( EOSOUTCOME_FAILED ), sdkResultCode( 0 ), expiresInSeconds( 0 )
    {
        epicAccountId[ 0 ] = 0; productUserId[ 0 ] = 0; userCode[ 0 ] = 0; verificationUri[ 0 ] = 0; verificationUriComplete[ 0 ] = 0;
    }
};
typedef struct SEosEvent TEosEvent;

/** What EOS_Platform_Create needs; the pointers are valid for the duration of the call only. Owned by the glue (pluginglueEOS) */
typedef PLUGINGLUE::EOS::SEosPlatformConfig SEosPlatformConfig;
typedef PLUGINGLUE::EOS::TEosPlatformConfig TEosPlatformConfig;

/**
 *  The function table the provider uses to reach the EOS SDK: the table of the platform of the glue
 *  (PLUGINGLUE::EOS::SEosPlatformShim: isSdkAvailable, createPlatform, releasePlatform, isPlatformReady, tick,
 *  which the glue owns and shares with the lobby and P2P plugins) and the functions of the Auth and Connect
 *  interfaces. The EOS SDK is not part of this repository and the plugin builds without it; when the build is told
 *  where the SDK is (see CEosSdk) the real table is filled from it, the tests fill it with a fake SDK. A function a
 *  table does not have is NULL; a table without the platform functions uses the platform of the glue for the SDK that
 *  is compiled in. None of the functions blocks: the SDK delivers the end of an operation as an event while the
 *  platform is ticked.
 */
struct SEosShim : public PLUGINGLUE::EOS::SEosPlatformShim
{
    /** Takes the next event the SDK delivered. False when there is none */
    bool ( *pollEvent )( void* context, SEosEvent* event );

    /** EOS_Auth_Login. The end comes as an EOSEVENT_AUTH_LOGIN event (and for the device code first an EOSEVENT_PIN_GRANT) */
    bool ( *authLogin )( void* context, UInt32 requestId, EEosLoginType type );

    /** EOS_Auth_CopyIdToken: the id token of the logged in account as the compact JWT */
    bool ( *authCopyIdToken )( void* context, const char* epicAccountId, char* buffer, UInt32 bufferSize );

    /** EOS_Connect_Login with the Epic id token (EOS_ECT_EPIC_ID_TOKEN). The end comes as EOSEVENT_CONNECT_LOGIN */
    bool ( *connectLogin )( void* context, UInt32 requestId, const char* idToken );

    /** EOS_Connect_CreateUser with the continuance token of the last EOSOUTCOME_NEEDS_USER_CREATION. The end comes as EOSEVENT_CONNECT_CREATE_USER */
    bool ( *connectCreateUser )( void* context, UInt32 requestId );

    /**
     *  EOS_Connect_LinkAccount: links the account of the last EOSOUTCOME_NEEDS_USER_CREATION (its continuance token) to the product user
     *  that is logged in already, given as the text of its Product User Id. The end comes as EOSEVENT_CONNECT_LINK_ACCOUNT
     */
    bool ( *connectLinkAccount )( void* context, UInt32 requestId, const char* productUserId );

    /** EOS_Auth_Logout, and with deletePersistentAuth EOS_Auth_DeletePersistentAuth: the next persistent login finds nothing */
    bool ( *authLogout )( void* context, UInt32 requestId, const char* epicAccountId, bool deletePersistentAuth );

    SEosShim( void )
        : PLUGINGLUE::EOS::SEosPlatformShim(), pollEvent( GUCEF_NULL ), authLogin( GUCEF_NULL ), authCopyIdToken( GUCEF_NULL ), connectLogin( GUCEF_NULL )
        , connectCreateUser( GUCEF_NULL ), connectLinkAccount( GUCEF_NULL ), authLogout( GUCEF_NULL )
    {}
};
typedef struct SEosShim TEosShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H ? */
