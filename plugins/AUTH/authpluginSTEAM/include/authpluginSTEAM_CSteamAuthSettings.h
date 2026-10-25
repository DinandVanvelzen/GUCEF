/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the "steam" provider. CAuthManager hands a provider the node that CAuthProviderSettings writes
 *  for it (section Auth, child Providers/steam), which only keeps the keys it knows and the attributes of its child
 *  Parameters. The keys of this plugin therefore go in Parameters; the same keys directly on the node are read as
 *  well, for an application that loads the provider itself.
 *
 *  Keys:
 *    runtimeLibrary        name or path of the Steam runtime library, empty for the platform default
 *    ticketIdentity        the identity string of the service that consumes the ticket (GetAuthTicketForWebApi).
 *                          Steam recommends one identity per consuming service. Needed for ticketMode "webapi"
 *    ticketMode            "auto" (default: web api ticket when the runtime has it, else session ticket), "webapi", "session"
 *    callbackMode          "manual" (default: the plugin dispatches the Steam callbacks on a timer of the application)
 *                          or "external" (the application dispatches them and hands the ticket response to
 *                          CSteamAuthProvider::OnTicketResponse())
 *    ownsSteamApi          true (default): SteamAPI_Shutdown is called when the provider goes; false for an application that uses Steamworks itself
 *    ticketTimeoutInMs     how long to wait for a ticket (10000)
 *    callbackIntervalInMs  period of the callback timer while a ticket is pending (50)
 *    retryIntervalInMs     the earliest the plugin tries to start the Steam API again after it failed (5000)
 *    appId                 the Steam app id, for the verification at the Web API
 *    webApiKey             publisher key for ISteamUserAuth/AuthenticateUserTicket, a secret that only a server has; never logged
 *    webApiKeyPlacement    "query" (default, the form of the Web API documentation: key=... in the query) or "header" (the key in the
 *                          x-webapi-key header, to confirm with Steam). Read the note at CSteamAuthProvider::Verify() before using "query"
 *    webApiBaseUrl         default https://partner.steam-api.com
 *    webApiTimeoutInMs     total time of a verification request (10000)
 *    allowInsecureHttp     accept an http:// base url, for tests only (false)
 *    verifyMode            "webapi" (default) or "peer" (ISteamUser::BeginAuthSession on this client)
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamAuthSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultTicketTimeoutInMs = 10000;
    static const UInt32 DefaultCallbackIntervalInMs = 50;
    static const UInt32 DefaultRetryIntervalInMs = 5000;
    static const UInt32 DefaultWebApiTimeoutInMs = 10000;

    CORE::CString runtimeLibrary;
    CORE::CString ticketIdentity;
    CORE::CString ticketMode;
    CORE::CString callbackMode;
    bool ownsSteamApi;
    UInt32 ticketTimeoutInMs;
    UInt32 callbackIntervalInMs;
    UInt32 retryIntervalInMs;
    CORE::CString appId;
    AUTH::CSecretString webApiKey;
    CORE::CString webApiKeyPlacement;
    CORE::CString webApiBaseUrl;
    UInt32 webApiTimeoutInMs;
    bool allowInsecureHttp;
    CORE::CString verifyMode;

    CSteamAuthSettings( void );

    CSteamAuthSettings( const CSteamAuthSettings& src );

    virtual ~CSteamAuthSettings();

    CSteamAuthSettings& operator=( const CSteamAuthSettings& src );

    bool operator==( const CSteamAuthSettings& other ) const;

    /** Whether the modes are values this plugin knows */
    bool IsWellFormed( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H ? */
