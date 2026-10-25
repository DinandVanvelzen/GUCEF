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

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#define GUCEF_AUTH_CAUTHSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

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
 *  What the application configures for one login provider. A provider without a client id is not offered when its
 *  capabilities say it needs one. The endpoints are overrides: empty means the value of the provider's descriptor.
 *
 *  Config: clientId, clientSecret (only for the provider kinds whose "installed application" secret is not confidential),
 *  audiences (array, or one value that is a comma list: further client ids a server accepts, see below), scopes (array), discoveryUrl, issuer, authorizationEndpoint, tokenEndpoint, deviceAuthorizationEndpoint, userInfoEndpoint, revocationEndpoint, tokenVerifyEndpoint,
 *  jwksUri, redirectMode ("loopback" or "custom-scheme"), loopbackPort (0 is an ephemeral port), loopbackPath, redirectUri
 *  (for the custom scheme), idScope ("global", "organisation", "app" or empty for the descriptor's), and the child
 *  Parameters with the values of the placeholders the descriptor names, for example tenant, region, poolId, poolDomain.
 *
 *  OpenID 2.0: the parameter relayUrl, empty by default, is the address of a relay page on a domain of the developer
 *  (https, port 443, a plain path, see COpenId2Relay::ParseRelayUrl). With it the realm and the return address of the login
 *  are on that domain, which is what the provider shows the player, and the page sends the browser on to the loopback
 *  address of the application. Without it the loopback address is the realm. An address that is not acceptable refuses the
 *  settings of the provider when they are loaded. A relay needs the loopback redirect mode, the path /callback and a
 *  loopbackPort of 0 or from 1024 on.
 *
 *  Client ids. A login of this build uses clientId, the client id of the client that is registered for this build, and
 *  nothing else. A server that verifies proofs accepts a token that was issued to any client id it knows: clientId and the
 *  list audiences together. A game that has a client per platform (desktop, Android, web) and one server lists all of them
 *  at the server, in audiences, and may leave clientId empty there. Client ids are public identifiers, not secrets. With
 *  no client id at all a server accepts nothing.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthProviderSettings : public CORE::CIConfigurable
{
    public:

    typedef CORE::CString::StringMap TParameterMap;

    CORE::CString clientId;                 /**< the client id of this build's own client: the only one a login uses */
    CSecretString clientSecret;
    CORE::CString::StringVector audiences;  /**< further client ids a server accepts besides clientId (the clients of the same game on other platforms or projects); never used by a login */
    CORE::CString::StringVector scopes;
    CORE::CString discoveryUrl;
    CORE::CString issuer;
    CORE::CString authorizationEndpoint;
    CORE::CString tokenEndpoint;
    CORE::CString deviceAuthorizationEndpoint;
    CORE::CString userInfoEndpoint;
    CORE::CString revocationEndpoint;
    CORE::CString tokenVerifyEndpoint;      /**< where a server checks an access token of a provider that has no id token (Facebook, Amazon) */
    CORE::CString jwksUri;
    CORE::CString redirectMode;             /**< "loopback" (default) or "custom-scheme" */
    UInt16 loopbackPort;                    /**< 0: any free port, the provider's registration must then accept any port on the loopback address */
    CORE::CString loopbackPath;
    CORE::CString redirectUri;              /**< custom scheme redirect, as registered with the provider */
    bool hasIdScope;
    TAuthIdScope idScope;                   /**< only when hasIdScope: replaces the scope of the descriptor */
    TParameterMap parameters;               /**< placeholder values */

    CAuthProviderSettings( void );

    CAuthProviderSettings( const CAuthProviderSettings& src );

    virtual ~CAuthProviderSettings();

    CAuthProviderSettings& operator=( const CAuthProviderSettings& src );

    /** Whether this build has a client id of its own to log in with. The list of audiences does not count */
    bool HasClientId( void ) const;

    /** Whether a server has a client id to check tokens against: clientId or an entry of audiences that is not empty */
    bool HasAcceptedClientIds( void ) const;

    /**
     *  The client ids a server accepts: clientId first, then the entries of audiences, without blanks around them, without
     *  empty entries and without duplicates. Empty when there is none, which accepts nothing
     */
    void GetAcceptedClientIds( CORE::CString::StringVector& clientIds ) const;

    /** The value of a placeholder parameter, empty when there is none */
    CORE::CString GetParameter( const CORE::CString& name ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The settings of the login module, the config section "Auth" of the application. Security related values belong to the
 *  game or application, not to a player.
 *
 *  Config: autoLogin, showUnavailableProviders (default true), appName, displayName (written only when there is one), credentialDirectory, linkStorePath, autoLoginStepTimeoutInMs, loginTimeoutInSeconds (60 to 1800, default 600), loginDeadlineSlackInMs,
 *  enabledProviders (array; empty means every registered provider), lastUsedProviders (array, most recent first),
 *  rememberLoginHints (default true), the child LoginHints with the name of the account of the last login at each provider
 *  as one attribute per provider name (written only while rememberLoginHints is on) and the child Providers with one child
 *  per provider name, see CAuthProviderSettings.
 *
 *  Login hints. After a login that worked the manager remembers the name of the account at the provider (the e-mail address
 *  or user name of the verified claims) and prefills it at the next login at the same provider, for the providers that take such
 *  a parameter. It is personal data kept on this machine only, in the configuration of the application next to
 *  lastUsedProviders, never in a log, never sent anywhere but to that provider's own login page; ClearLoginHints() forgets
 *  all of it and rememberLoginHints off stops remembering and using them (what is in the file is dropped when it is loaded).
 */
class GUCEF_AUTH_EXPORT_CPP CAuthSettings : public CORE::CIConfigurable
{
    public:

    typedef GUCEF::map< CORE::CString, CAuthProviderSettings > TProviderSettingsMap;

    static const UInt32 MaxLastUsedProviders = 16;
    static const UInt32 MaxLoginHints = 64;
    static const UInt32 DefaultLoginTimeoutInSeconds = 600;
    static const UInt32 MinLoginTimeoutInSeconds = 60;
    static const UInt32 MaxLoginTimeoutInSeconds = 1800;
    static const UInt32 DefaultAutoLoginStepTimeoutInMs = 15000;
    static const UInt32 DefaultLoginDeadlineSlackInMs = 5000;

    bool autoLogin;                                 /**< CAuthManager::AutoLogin() does something at all */
    bool showUnavailableProviders;                  /**< CAuthManager::GetUnavailableProviders() lists the providers that are not offered, with the reason, for a screen to show them disabled; on while a product is being set up, off for a shipped build */
    CORE::CString appName;                          /**< the application directory name below $APPDATADIR$ */
    CORE::CString displayName;                      /**< the name of the person for a tool that has no profile of its own (CAuthSettingsProfile), empty for none */
    CORE::CString credentialDirectory;              /**< empty: $APPDATADIR$/<appName>/Auth */
    CORE::CString linkStorePath;                    /**< empty: links.json in the credential directory */
    UInt32 autoLoginStepTimeoutInMs;                /**< how long AutoLogin() waits for one provider before it goes on with the next */
    UInt32 loginTimeoutInSeconds;                   /**< how long a person gets to finish an interactive login in the browser, 60 to 1800, default 600 (see CAuthOptions::DefaultTimeoutInMs for why); applied when the options of a login say the default. A device login ends when the provider says its code expires */
    UInt32 loginDeadlineSlackInMs;                  /**< the time the manager gives a provider on top of the timeout of a login before it ends the login itself */
    CORE::CString::StringVector enabledProviders;
    CORE::CString::StringVector lastUsedProviders;
    bool rememberLoginHints;                        /**< the name of the account of the last login at a provider is kept and prefilled at the next one (default true) */
    CORE::CString::StringMap loginHints;            /**< per provider name: the name of the account of the last login; written by the manager, empty while rememberLoginHints is off */
    TProviderSettingsMap providers;

    CAuthSettings( void );

    CAuthSettings( const CAuthSettings& src );

    virtual ~CAuthSettings();

    CAuthSettings& operator=( const CAuthSettings& src );

    /** loginTimeoutInSeconds brought into the range that is accepted */
    static UInt32 ClampLoginTimeoutInSeconds( UInt32 seconds );

    /** The directory of the credential store with its path variables resolved */
    CORE::CString GetCredentialDirectory( void ) const;

    /** The file of the local link store with its path variables resolved */
    CORE::CString GetLinkStorePath( void ) const;

    bool GetProviderSettings( const CORE::CString& providerName, CAuthProviderSettings& providerSettings ) const;

    void SetProviderSettings( const CORE::CString& providerName, const CAuthProviderSettings& providerSettings );

    /** Whether the provider is in enabledProviders, or the list is empty */
    bool IsProviderEnabled( const CORE::CString& providerName ) const;

    /**
     *  Enabled, and when the provider needs a client id one is configured. This is the settings half of what the manager
     *  offers; the provider's own IsAvailable() is the other half.
     */
    bool IsProviderOffered( const CORE::CString& providerName, bool providerRequiresClientId ) const;

    /**
     *  Makes the settings that are kept under the name 'from' (an earlier name of a provider) the settings of 'to': the settings
     *  of the provider, its place in enabledProviders and lastUsedProviders and its remembered login hint. Where 'to' has a
     *  setting of its own that one stays; the parameters of 'from' that 'to' does not have are added to its parameters. True
     *  when anything was kept under 'from'.
     */
    bool RenameProvider( const CORE::CString& from, const CORE::CString& to );

    /** Puts the provider first in lastUsedProviders; the list keeps MaxLastUsedProviders at most. True when the list changed */
    bool TouchLastUsed( const CORE::CString& providerName );

    /** The remembered name of the account at the provider; false when there is none or rememberLoginHints is off */
    bool GetLoginHint( const CORE::CString& providerName, CORE::CString& loginHint ) const;

    /** Remembers the name of the account; True when that changed what is kept. Nothing is kept while rememberLoginHints is off, for an empty provider name or for a name that is not acceptable as a hint */
    bool SetLoginHint( const CORE::CString& providerName, const CORE::CString& loginHint );

    /** Forgets every remembered name of an account; True when there was one */
    bool ClearLoginHints( void );

    bool HasLoginHints( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the section itself or a node that has the section "Auth" as a child */
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

#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */
