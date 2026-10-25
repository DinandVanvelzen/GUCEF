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

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H

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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H
#include "pluginglueEOS_CEosPlatformSettings.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_MACROS_H
#include "authpluginEOS_macros.h"
#define GUCEF_AUTHPLUGIN_EOS_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_EOS_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the "epic" provider of this plugin. CAuthManager hands a provider the node that CAuthProviderSettings
 *  writes for it (section Auth, child Providers/epic): clientId and clientSecret as attributes and the attributes of
 *  its child Parameters. The keys of this plugin that are not in CAuthProviderSettings therefore go in Parameters; all
 *  keys are also read directly from the node, for an application that loads the provider itself.
 *
 *  Keys (the product ids, the client id and the client secret are public client credentials from the Epic developer portal):
 *    clientId                       client id of the product's client  (CAuthProviderSettings key)
 *    clientSecret                   client secret of the client, never logged  (CAuthProviderSettings key)
 *    productId, sandboxId, deploymentId    the ids of the product in the developer portal
 *    encryptionKey                  64 hex digits, optional, never logged
 *    productName, productVersion    EOS_Initialize, default "GUCEF" and "1.0"
 *    cacheDirectory                 optional cache directory of the SDK (path variables are resolved)
 *    useConnect                     also log in at EOS Connect to get the product user id (true)
 *    createConnectUser              make the product user when Connect says the account has none (true)
 *    deletePersistentAuthOnLogout   Logout() removes the stored token, so the next silent login finds none (true)
 *    tickIntervalInMs               period of the timer that ticks the SDK while a login is under way (50)
 *    silentTimeoutInMs              how long a silent login may take (15000)
 *
 *  Every key of CAuthProviderSettings that the plugin does not use (issuer, jwksUri, ...) is kept for the OIDC
 *  engine, to which the verification of id tokens is delegated.
 */
class GUCEF_AUTHPLUGIN_EOS_EXPORT_CPP CEosAuthSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultTickIntervalInMs = 50;
    static const UInt32 DefaultSilentTimeoutInMs = 15000;

    CORE::CString clientId;
    AUTH::CSecretString clientSecret;
    CORE::CString productId;
    CORE::CString sandboxId;
    CORE::CString deploymentId;
    AUTH::CSecretString encryptionKey;
    CORE::CString productName;
    CORE::CString productVersion;
    CORE::CString cacheDirectory;
    bool useConnect;
    bool createConnectUser;
    bool deletePersistentAuthOnLogout;
    UInt32 tickIntervalInMs;
    UInt32 silentTimeoutInMs;

    CEosAuthSettings( void );

    CEosAuthSettings( const CEosAuthSettings& src );

    virtual ~CEosAuthSettings();

    CEosAuthSettings& operator=( const CEosAuthSettings& src );

    bool operator==( const CEosAuthSettings& other ) const;

    /** All of the ids and the client credentials that EOS_Platform_Create needs are there */
    bool IsConfigured( void ) const;

    /** The values that have a form are in it: the encryption key is 64 hex digits when it is set */
    bool IsWellFormed( void ) const;

    /** The part of the settings that makes the platform: what the glue (pluginglueEOS) shares with the lobby and P2P plugins */
    PLUGINGLUE::EOS::CEosPlatformSettings GetPlatformSettings( void ) const;

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

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H ? */
