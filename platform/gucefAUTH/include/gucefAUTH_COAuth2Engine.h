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

#ifndef GUCEF_AUTH_COAUTH2ENGINE_H
#define GUCEF_AUTH_COAUTH2ENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTH_COAUTH2FLOW_H
#include "gucefAUTH_COAuth2Flow.h"
#define GUCEF_AUTH_COAUTH2FLOW_H
#endif /* GUCEF_AUTH_COAUTH2FLOW_H ? */

#ifndef GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#include "gucefAUTH_COAuth2EngineSettings.h"
#define GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#endif /* GUCEF_AUTH_COAUTH2ENGINESETTINGS_H ? */

#ifndef GUCEF_AUTH_CJWKSCACHE_H
#include "gucefAUTH_CJwksCache.h"
#define GUCEF_AUTH_CJWKSCACHE_H
#endif /* GUCEF_AUTH_CJWKSCACHE_H ? */

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
 *  The built in flow engine: OpenID Connect and OAuth 2.0 with the authorization code and PKCE (loopback redirect, a
 *  custom scheme or a redirect the application hands back), the device authorization grant, refresh with rotation,
 *  revocation, OpenID 2.0 in the browser, and the server side checking of what these produce: ID tokens
 *  (signature by the key set of the provider, issuer, audience, expiry, ...), OpenID 2.0 assertions and the access
 *  tokens of providers that have no ID token (Facebook, Amazon).
 *
 *  CAuthGlobal makes one and registers it as the flow engine when the module is initialised (CAuthGlobal::GetOAuth2Engine()),
 *  so that the descriptor providers work without further setup; the engine settings are those of the application
 *  (COAuth2EngineSettings, the child "Engine" of the section "Auth").
 *
 *  Threads. StartInteractive(), StartRefresh(), Revoke(), SubmitRedirectUrl() and CancelAll() are called on the thread that
 *  pumps the global pulse generator; the logins run there, their HTTP requests run as tasks of the thread pool and
 *  their events come from the pulse thread. Verify() runs on the calling thread and can wait for the network on a miss
 *  of the key cache or for an OpenID 2.0 check: servers call it from a worker. The settings are set before use.
 *
 *  Needs the "json" data store codec (dstorepluginJSONPARSER) for OIDC and OAuth 2.0 and the RSA signature codecs of
 *  gucefCORE for ID tokens; an https endpoint needs the curl HTTP client plugin or WinHTTP. IsSupported() says no
 *  when a part is missing.
 */
class GUCEF_AUTH_EXPORT_CPP COAuth2Engine : public IAuthFlowEngine ,
                                            public CORE::CObserver
{
    public:

    COAuth2Engine( void );

    virtual ~COAuth2Engine() GUCEF_VIRTUAL_OVERRIDE;

    const COAuth2EngineSettings& GetSettings( void ) const;

    /** Replaces the settings; the key cache policy follows. Set them before the engine is used */
    void SetSettings( const COAuth2EngineSettings& settings );

    /** The key sets of the providers */
    CJwksCache& GetJwksCache( void );

    /** The nonces of the OpenID 2.0 assertions of logins, and of the proofs that were verified */
    COpenId2NonceCache& GetLoginNonceCache( void );

    COpenId2NonceCache& GetVerifyNonceCache( void );

    /** Forgets the key sets, the discovery documents and the OpenID 2.0 nonces */
    void ClearCaches( void );

    /**
     *  The profile of an OpenID 2.0 descriptor, from the registry of CAuthGlobal (CAuthGlobal::GetOpenId2Profiles()); GUCEF_NULL
     *  when there is none, and then the engine does not support the descriptor and refuses every assertion for it
     */
    IOpenId2ProviderProfile* FindOpenId2Profile( const CAuthProviderDescriptor& descriptor ) const;

    /** How many logins are under way */
    UInt32 GetActiveLoginCount( void ) const;

    /** Ends every login as cancelled */
    void CancelAll( void );

    /** Lets go of the HTTP client backends kept from earlier requests, see COAuth2Http::ReleaseBackends(): before the plugin that made them is unloaded, with no login under way */
    void ReleaseHttpBackends( void );

    virtual bool IsSupported( const CAuthProviderDescriptor& descriptor ,
                              const CAuthProviderSettings& settings     ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSecureTransportMissing( const CAuthProviderDescriptor& descriptor ,
                                           const CAuthProviderSettings& settings     ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartInteractive( const CAuthProviderDescriptor& descriptor ,
                                   const CAuthProviderSettings& settings     ,
                                   const CAuthOptions& options               ,
                                   IAuthFlowSink& sink                       ,
                                   CORE::CFutureResultSource& result         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartRefresh( const CAuthProviderDescriptor& descriptor ,
                               const CAuthProviderSettings& settings     ,
                               const CAccountRef& account                ,
                               const CSecretString& refreshToken         ,
                               IAuthFlowSink& sink                       ,
                               CORE::CFutureResultSource& result         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Revoke( const CAuthProviderDescriptor& descriptor ,
                         const CAuthProviderSettings& settings     ,
                         const CAccountRef& account                ,
                         const CSecretString& refreshToken         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const CAuthProviderDescriptor& descriptor ,
                         const CAuthProviderSettings& settings     ,
                         const CAuthProof& proof                   ,
                         CAuthResult& result                       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SubmitRedirectUrl( const CORE::CString& redirectUrl ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // For the flows of the engine

    COAuth2Http& GetHttp( void );

    bool TryGetDiscovery( const CORE::CString& address, COidcMetadata& metadata ) const;

    void StoreDiscovery( const CORE::CString& address, const COidcMetadata& metadata );

    /** Builds the account and the profile fields of a result from the claims of a login or of a proof */
    static bool MakeResultFromClaims( const CAuthProviderDescriptor& descriptor ,
                                      const CORE::CString::StringMap& claims    ,
                                      const CORE::CString::StringMap& parameters,
                                      CAuthResult& result                       );

    /** Validation options for the ID token of a login with these settings: the audience is the client id of the login and no other */
    void MakeValidationOptions( const COAuth2Resolved& resolved, CJwtValidationOptions& options ) const;

    /** Validation options for the ID token of a proof at a server: the audience is any of the client ids the server has */
    void MakeVerifyValidationOptions( const COAuth2Resolved& resolved, CJwtValidationOptions& options ) const;

    /** The tenant of a token against the tenant setting of the application, when the issuer names a tenant */
    static bool CheckTenant( const COAuth2Resolved& resolved, const CORE::CString& tenantId );

    /** Reads a key set document, stores it in the cache. False when it is not acceptable */
    bool StoreKeySet( const CORE::CString& address, const CAuthHttpResponse& response );

    private:

    typedef CORE::CTEventHandlerFunctor< COAuth2Engine > TEventCallback;

    void OnHousekeepingUpdate( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    struct SDiscoveryEntry
    {
        COidcMetadata metadata;
        UInt64 expiresAtMs;
    };

    typedef GUCEF::vector< COAuth2Flow* > TFlowVector;
    typedef GUCEF::vector< CORE::CFutureResult > TFutureVector;
    typedef GUCEF::map< CORE::CString, SDiscoveryEntry > TDiscoveryMap;

    COAuth2Engine( const COAuth2Engine& src );              /**< not implemented */
    COAuth2Engine& operator=( const COAuth2Engine& src );   /**< not implemented */

    void ReapFinished( void );
    void UpdateHousekeeping( void );
    bool Fetch( const CAuthHttpRequest& request, CAuthHttpResponse& response );
    bool VerifyIdToken( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result );
    bool VerifyOpenId2( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result );
    bool VerifyAccessToken( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result );
    bool ResolveSync( const CAuthProviderDescriptor& descriptor, const CAuthProviderSettings& settings, COAuth2Resolved& resolved );
    bool GetKeySync( const COAuth2Resolved& resolved, const CORE::CString& keyId, CJwk& key, TJwtStatus& status );

    private:

    COAuth2EngineSettings m_settings;
    COAuth2Http m_http;
    CJwksCache m_jwks;
    COpenId2NonceCache m_loginNonces;
    COpenId2NonceCache m_verifyNonces;
    TFlowVector m_flows;
    TFutureVector m_revokes;
    TDiscoveryMap m_discovery;
    mutable MT::CMutex m_lock;
    CORE::CTimer m_housekeeping;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COAUTH2ENGINE_H ? */
