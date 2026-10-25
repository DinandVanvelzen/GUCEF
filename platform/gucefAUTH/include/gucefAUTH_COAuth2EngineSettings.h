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

#ifndef GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#define GUCEF_AUTH_COAUTH2ENGINESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the built in flow engine behaves. These are settings of the game or application, never of a player: they come
 *  from the application configuration (the child "Engine" of the section "Auth") and are not saved with a profile.
 *
 *  Config: allowInsecureHttp, launchBrowser, clockSkewInSeconds, httpConnectTimeoutInMs, httpTimeoutInMs,
 *  maxResponseBytes, jwksMinTtlInSeconds, jwksMaxTtlInSeconds, jwksDefaultTtlInSeconds, jwksMinRefreshIntervalInMs,
 *  discoveryTtlInSeconds, deviceIntervalUnitInMs, deviceDefaultIntervalInSeconds, deviceSlowDownIncrementInSeconds,
 *  openId2MaxAgeInSeconds, maxActiveLogins, caBundlePath
 */
class GUCEF_AUTH_EXPORT_CPP COAuth2EngineSettings : public CORE::CIConfigurable
{
    public:

    /**
     *  Endpoints, issuers and redirects have to be https. With this set an http endpoint is accepted when its host is a
     *  loopback address (127.x.x.x, localhost, ::1), for a provider that runs on the machine itself. Any other host
     *  stays refused. Default false.
     */
    bool allowInsecureHttp;

    /** A login with a browser opens the system browser (CORE::OpenUrlInSystemBrowser). The event with the url is raised either way. Default true */
    bool launchBrowser;

    UInt32 clockSkewInSeconds;                  /**< on exp, nbf and iat of tokens, default 120, at most 600 */
    UInt32 httpConnectTimeoutInMs;              /**< default 10000 */
    UInt32 httpTimeoutInMs;                     /**< per request, default 20000 */
    UInt32 maxResponseBytes;                    /**< of any answer of a provider, default 1 MiB */
    UInt32 jwksMinTtlInSeconds;                 /**< default 60 */
    UInt32 jwksMaxTtlInSeconds;                 /**< default 86400 */
    UInt32 jwksDefaultTtlInSeconds;             /**< when the provider gives no max-age, default 3600 */
    UInt32 jwksMinRefreshIntervalInMs;          /**< the time between two refreshes of a key set for a key id that is not in it, default 10000 */
    UInt32 discoveryTtlInSeconds;               /**< default 3600 */
    UInt32 deviceIntervalUnitInMs;              /**< what one second of a device flow interval is, default 1000: only a test makes it smaller */
    UInt32 deviceDefaultIntervalInSeconds;      /**< when the provider gives none, default 5 */
    UInt32 deviceSlowDownIncrementInSeconds;    /**< what slow_down adds to the interval, default 5 */
    UInt32 openId2MaxAgeInSeconds;              /**< how old the response_nonce of an OpenID 2.0 assertion may be, default 300 */
    UInt32 maxActiveLogins;                     /**< logins that run at the same time, default 16 */

    /**
     *  The file with the certificates of the authorities that vouch for the servers of the providers (PEM). Empty, the default: the
     *  environment variable SSL_CERT_FILE, else the bundle of the system (/etc/ssl/certs/ca-certificates.crt, /etc/pki/tls/certs/ca-bundle.crt,
     *  /etc/ssl/cert.pem). The certificate of a server is always checked, a file that cannot be read makes every https request fail.
     */
    CORE::CString caBundlePath;

    COAuth2EngineSettings( void );
    COAuth2EngineSettings( const COAuth2EngineSettings& src );
    virtual ~COAuth2EngineSettings();
    COAuth2EngineSettings& operator=( const COAuth2EngineSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the settings themselves, or a node that has the child "Engine", or the section "Auth" with that child */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COAUTH2ENGINESETTINGS_H ? */
