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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H
#define GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#include "authpluginANDROID_macros.h"
#define GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_MACROS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whether the person may be asked something */
enum EAndroidSignInMode
{
    ANDROIDSIGNIN_SILENT = 0,       /**< only what works without the person doing anything */
    ANDROIDSIGNIN_INTERACTIVE       /**< account pickers and consent screens may show */
};

/** How a request of the Java side ended. The same numbers are used in the Java sources (GucefAuthBridge) */
enum EAndroidAuthStatus
{
    ANDROIDAUTH_OK = 0,
    ANDROIDAUTH_CANCELLED = 1,          /**< the person backed out */
    ANDROIDAUTH_NOT_SIGNED_IN = 2,      /**< silent: there is no signed in account (Play Games: isAuthenticated is false) */
    ANDROIDAUTH_NO_CREDENTIAL = 3,      /**< silent: no credential for this app was authorized before (Credential Manager) */
    ANDROIDAUTH_DENIED = 4,             /**< the account or the app was refused */
    ANDROIDAUTH_NETWORK = 5,
    ANDROIDAUTH_ERROR = 6               /**< anything else, the message says what */
};

/** The end of a request, as the Java side reports it through the JNI entry or a test through the bridge */
struct SAndroidAuthResult
{
    UInt32 requestId;
    EAndroidAuthStatus status;
    AUTH::CSecretString authCode;           /**< Google: the one time server auth code of Play Games requestServerSideAccess */
    AUTH::CSecretString idToken;            /**< Google: the id token of Credential Manager */
    AUTH::CSecretString accessToken;        /**< Amazon: the access token of Login with Amazon */
    CORE::CString accountId;                /**< Amazon: the user id; Google: unused, the account id is the sub of the id token */
    CORE::CString displayName;
    CORE::CString avatarUrl;
    CORE::CString message;                  /**< for the log, no secrets */

    SAndroidAuthResult( void ) : requestId( 0 ), status( ANDROIDAUTH_ERROR ), authCode(), idToken(), accessToken(), accountId(), displayName(), avatarUrl(), message() {}
};
typedef struct SAndroidAuthResult TAndroidAuthResult;

/**
 *  The function table the providers use to reach the Android side. The real table is filled by the JNI code of the
 *  plugin (GucefAuthBridge.java), a test fills it with fakes: nothing here needs a JVM. A function a table does not
 *  have is NULL. None of the functions blocks and none reports a result itself: the end of a request is reported
 *  with CAndroidAuthBridge::DeliverResult(), from any thread, as the JNI entry does.
 */
struct SAndroidShim
{
    void* context;

    /** Play Games Services (and with it Google sign in) can be used on this device */
    bool ( *isGooglePlayAvailable )( void* context );

    /** The Login with Amazon SDK is there (Fire OS, or the Amazon app on the device) */
    bool ( *isAmazonAvailable )( void* context );

    /**
     *  Starts a Google sign in: Play Games GamesSignInClient (isAuthenticated, signIn when interactive) and
     *  requestServerSideAccess( serverClientId ); with useCredentialManager also the id token of Credential Manager
     *  ("Sign in with Google"), with the nonce when it is not empty
     */
    bool ( *googleSignIn )( void* context, UInt32 requestId, EAndroidSignInMode mode, const char* serverClientId, bool useCredentialManager, const char* nonce );

    /** Starts a Login with Amazon authorization for the scopes (space separated); silent asks for a token that was authorized before */
    bool ( *amazonSignIn )( void* context, UInt32 requestId, EAndroidSignInMode mode, const char* scopes );

    /** Gives up on a request: its end is not delivered any more */
    void ( *cancelRequest )( void* context, UInt32 requestId );

    /** Forgets the Google credential state of the app (Credential Manager clearCredentialState) */
    void ( *googleSignOut )( void* context );

    /** Login with Amazon signOut */
    void ( *amazonSignOut )( void* context );

    /** Opens a Custom Tab with the url (Intent with ACTION_VIEW as the fallback of the Java side) */
    bool ( *openCustomTab )( void* context, const char* url );

    SAndroidShim( void )
        : context( GUCEF_NULL ), isGooglePlayAvailable( GUCEF_NULL ), isAmazonAvailable( GUCEF_NULL ), googleSignIn( GUCEF_NULL )
        , amazonSignIn( GUCEF_NULL ), cancelRequest( GUCEF_NULL ), googleSignOut( GUCEF_NULL ), amazonSignOut( GUCEF_NULL ), openCustomTab( GUCEF_NULL )
    {}
};
typedef struct SAndroidShim TAndroidShim;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_ANDROIDSHIM_H ? */
