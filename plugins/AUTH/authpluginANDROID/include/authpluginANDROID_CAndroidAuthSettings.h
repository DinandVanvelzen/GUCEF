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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the "google" and "amazon" providers of this plugin, one class for both: each provider uses the
 *  keys that are its own. CAuthManager hands a provider the node that CAuthProviderSettings writes for it (section Auth,
 *  child Providers/google or Providers/amazon): clientId and clientSecret as attributes and the attributes of its child
 *  Parameters. The keys of this plugin that are not in CAuthProviderSettings therefore go in Parameters; all keys are also
 *  read directly from the node, for an application that loads the provider itself.
 *
 *  Keys:
 *    clientId                the web client id of the game's server (google: what requestServerSideAccess and
 *                            Credential Manager are given as serverClientId, and the audience of the id tokens), or the
 *                            client id of the Login with Amazon security profile (amazon: the audience the server checks).
 *                            A key of CAuthProviderSettings. The google provider is not offered without it.
 *    clientSecret            the secret of that web client, only on the server that verifies codes, never logged  (key of CAuthProviderSettings)
 *    useCredentialManager    google: also get the Google id token with Credential Manager (true). It is where the
 *                            account id (sub) of the client comes from, Play Games gives a server auth code only
 *    proofKind               google: what the proof of a login is: "auth-code" (default, the one time server auth code,
 *                            kind platform-ticket) or "id-token" (the id token, kind oidc-id-token)
 *    tokenEndpoint           google: where the server exchanges the code, default https://oauth2.googleapis.com/token
 *    tokenInfoEndpoint       amazon: where the server checks the access token, default https://api.amazon.com/auth/o2/tokeninfo
 *    amazonScopes            amazon: scopes of the authorization, space separated, default "profile"
 *    allowInsecureHttp       accept http:// endpoints, for tests only (false)
 *    httpTimeoutInMs         total time of a request of the server role (10000)
 *    silentTimeoutInMs       how long a silent login may take (15000)
 *
 *  Every key of CAuthProviderSettings that the plugin does not use (issuer, jwksUri, ...) is kept for the OIDC engine, to
 *  which the verification of id tokens is delegated.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAndroidAuthSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultHttpTimeoutInMs = 10000;
    static const UInt32 DefaultSilentTimeoutInMs = 15000;

    CORE::CString clientId;
    AUTH::CSecretString clientSecret;
    bool useCredentialManager;
    CORE::CString proofKind;
    CORE::CString tokenEndpoint;
    CORE::CString tokenInfoEndpoint;
    CORE::CString amazonScopes;
    bool allowInsecureHttp;
    UInt32 httpTimeoutInMs;
    UInt32 silentTimeoutInMs;

    CAndroidAuthSettings( void );

    CAndroidAuthSettings( const CAndroidAuthSettings& src );

    virtual ~CAndroidAuthSettings();

    CAndroidAuthSettings& operator=( const CAndroidAuthSettings& src );

    bool operator==( const CAndroidAuthSettings& other ) const;

    /** The values that have a form are in it */
    bool IsWellFormed( void ) const;

    /** The settings in the form the OIDC engine takes them: the client credentials and what else the node had */
    AUTH::CAuthProviderSettings GetEngineSettings( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    AUTH::CAuthProviderSettings m_engineSettings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H ? */
