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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#define GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H
#include "authpluginANDROID_CAndroidAuthProviderBase.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H
#include "authpluginANDROID_CAndroidAuthSettings.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Logs in with the Google account of an Android device: provider "google", account references "google:<sub>", the
 *  same account that the OAuth based "google" descriptor gives (the sub claim of the Google id token).
 *  Capabilities: AUTHCAP_SILENT, AUTHCAP_SERVER_VERIFIABLE and AUTHCAP_REQUIRES_CLIENT_ID (the web client id of the
 *  game's server).
 *
 *  Client. The Java side (GucefAuthBridge) does the sign in: Play Games Services v2 GamesSignInClient (isAuthenticated for a
 *  silent login, signIn when the person is asked) and requestServerSideAccess( clientId ) for a one time server auth code,
 *  and, with the setting useCredentialManager, Credential Manager for the Google id token ("Sign in with Google").
 *  Play Games gives the server auth code but not who the person is: the account id (sub) is read from the id token of
 *  Credential Manager. Without an id token the login fails, with a message that says so. The id token is checked for the
 *  nonce this side generated, the audience (clientId), the issuer and the expiry; those checks only guard against a
 *  broken Java side, nothing the client says is believed by a server.
 *  The result has the account, the persona, the id token and as the proof the server auth code (kind platform-ticket) or,
 *  by the setting proofKind, the id token (kind oidc-id-token).
 *
 *  Server. Verify() takes either proof. A server auth code is exchanged at the token endpoint of Google with the web
 *  client id and secret of the settings; the id token it gives, or the id token of the proof itself, is verified by the
 *  OIDC engine of gucefAUTH (CAuthGlobal::GetFlowEngine()) with the built in "google" descriptor. Without an engine
 *  Verify() reports that it cannot.
 *
 *  UNVERIFIED AGAINST THE REAL SERVICES: the Java side could not be compiled here and nothing was tried against Google.
 *  The tests run on a fake table of functions and a fake token endpoint.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CGoogleAuthProvider : public CAndroidAuthProviderBase
{
    public:

    static const CORE::CString ProviderName;        /**< "google" */

    CGoogleAuthProvider( void );

    virtual ~CGoogleAuthProvider();

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A copy of the settings in use */
    CAndroidAuthSettings GetSettings( void ) const;

    /** Whether Verify() can verify id tokens: there is an OIDC engine that supports the "google" descriptor with these settings */
    bool IsVerifySupported( void ) const;

    protected:

    virtual bool CanStart( bool interactive, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartNative( UInt32 requestId, bool interactive, const CORE::CString& nonce ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void HandleResult( SPending& pending, const SAndroidAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGoogleAuthProvider( const CGoogleAuthProvider& src );
    CGoogleAuthProvider& operator=( const CGoogleAuthProvider& src );

    bool ExchangeCode( const CAndroidAuthSettings& settings, const AUTH::CSecretString& code, AUTH::CSecretString& idToken ) const;

    private:

    CAndroidAuthSettings m_settings;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CGoogleAuthProvider, MT::CMutex > TGoogleAuthProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H ? */
