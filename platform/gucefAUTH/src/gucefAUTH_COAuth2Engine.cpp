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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CAUTHJSON_H
#include "gucefAUTH_CAuthJson.h"
#define GUCEF_AUTH_CAUTHJSON_H
#endif /* GUCEF_AUTH_CAUTHJSON_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#include "gucefAUTH_COAuth2Engine.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 HousekeepingIntervalInMs = 100;
static const UInt32 MaxDiscoveryEntries = 32;
static const UInt32 MaxDisplayNameBytes = 256;

/*-------------------------------------------------------------------------*/

COAuth2Engine::COAuth2Engine( void )
    : IAuthFlowEngine()
    , CORE::CObserver()
    , m_settings()
    , m_http( m_settings )
    , m_jwks()
    , m_loginNonces()
    , m_verifyNonces()
    , m_flows()
    , m_revokes()
    , m_discovery()
    , m_lock()
    , m_housekeeping( HousekeepingIntervalInMs )
{GUCEF_TRACE;

    TEventCallback housekeepingCallback( this, &COAuth2Engine::OnHousekeepingUpdate );
    SubscribeTo( &m_housekeeping, CORE::CTimer::TimerUpdateEvent, housekeepingCallback );
    SetSettings( m_settings );
}

/*-------------------------------------------------------------------------*/

COAuth2Engine::~COAuth2Engine()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_housekeeping.SetEnabled( false );
    for ( size_t i=0; i<m_flows.size(); ++i )
        GUCEF_DELETE m_flows[ i ];
    m_flows.clear();
    for ( size_t i=0; i<m_revokes.size(); ++i )
        m_revokes[ i ].RequestCancellation();
    m_revokes.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COAuth2Engine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::COAuth2Engine";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const COAuth2EngineSettings&
COAuth2Engine::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::SetSettings( const COAuth2EngineSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    if ( m_settings.clockSkewInSeconds > CJwtValidationOptions::MaxClockSkewInSeconds )
        m_settings.clockSkewInSeconds = CJwtValidationOptions::MaxClockSkewInSeconds;
    m_jwks.SetPolicy( m_settings.jwksMinTtlInSeconds, m_settings.jwksMaxTtlInSeconds, m_settings.jwksDefaultTtlInSeconds, m_settings.jwksMinRefreshIntervalInMs );
}

/*-------------------------------------------------------------------------*/

CJwksCache&
COAuth2Engine::GetJwksCache( void )
{GUCEF_TRACE;

    return m_jwks;
}

/*-------------------------------------------------------------------------*/

IOpenId2ProviderProfile*
COAuth2Engine::FindOpenId2Profile( const CAuthProviderDescriptor& descriptor ) const
{GUCEF_TRACE;

    return CAuthGlobal::Instance()->GetOpenId2Profiles().Find( descriptor.name );
}

/*-------------------------------------------------------------------------*/

COpenId2NonceCache&
COAuth2Engine::GetLoginNonceCache( void )
{GUCEF_TRACE;

    return m_loginNonces;
}

/*-------------------------------------------------------------------------*/

COpenId2NonceCache&
COAuth2Engine::GetVerifyNonceCache( void )
{GUCEF_TRACE;

    return m_verifyNonces;
}

/*-------------------------------------------------------------------------*/

COAuth2Http&
COAuth2Engine::GetHttp( void )
{GUCEF_TRACE;

    return m_http;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::ClearCaches( void )
{GUCEF_TRACE;

    m_jwks.Clear();
    m_loginNonces.Clear();
    m_verifyNonces.Clear();
    MT::CScopeMutex lock( m_lock );
    m_discovery.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
COAuth2Engine::GetActiveLoginCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_flows.size(); ++i )
        if ( !m_flows[ i ]->IsFinished() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::CancelAll( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_flows.size(); ++i )
        if ( !m_flows[ i ]->IsFinished() )
            m_flows[ i ]->Cancel();
    UpdateHousekeeping();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::ReleaseHttpBackends( void )
{GUCEF_TRACE;

    m_http.ReleaseBackends();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::ReapFinished( void )
{GUCEF_TRACE;

    size_t i = 0;
    while ( i < m_flows.size() )
    {
        if ( m_flows[ i ]->IsFinished() )
        {
            GUCEF_DELETE m_flows[ i ];
            m_flows.erase( m_flows.begin() + i );
        }
        else
            ++i;
    }

    i = 0;
    while ( i < m_revokes.size() )
    {
        if ( m_revokes[ i ].IsReady() || m_revokes[ i ].HasNoFuture() )
            m_revokes.erase( m_revokes.begin() + i );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::UpdateHousekeeping( void )
{GUCEF_TRACE;

    bool needed = !m_flows.empty() || !m_revokes.empty();
    if ( needed != m_housekeeping.GetEnabled() )
        m_housekeeping.SetEnabled( needed );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::OnHousekeepingUpdate( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ReapFinished();
    UpdateHousekeeping();
}

/*-------------------------------------------------------------------------*/

/** Whether what every login with the provider needs goes over https: for OpenID 2.0 the check of the assertion, otherwise the discovery document when the login starts with it, otherwise the token endpoint; the profile and the other optional requests are left out */
static bool
UsesHttps( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved )
{
    if ( AUTHPROTOCOL_OPENID2 == descriptor.protocol )
        return CAuthUrl::IsHttpsUrl( resolved.authorizationEndpoint );
    return resolved.wantsDiscovery ? CAuthUrl::IsHttpsUrl( resolved.discoveryUrl ) : CAuthUrl::IsHttpsUrl( resolved.tokenEndpoint );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::IsSecureTransportMissing( const CAuthProviderDescriptor& descriptor ,
                                         const CAuthProviderSettings& settings     ) const
{GUCEF_TRACE;

    if ( !descriptor.IsWellFormed() || AUTHPROTOCOL_PLATFORM == descriptor.protocol )
        return false;
    COAuth2Resolved resolved;
    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, m_settings.allowInsecureHttp, resolved, error ) )
        return false;
    return UsesHttps( descriptor, resolved ) && !COAuth2Http::IsHttpsSupported();
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::IsSupported( const CAuthProviderDescriptor& descriptor ,
                            const CAuthProviderSettings& settings     ) const
{GUCEF_TRACE;

    if ( !descriptor.IsWellFormed() || AUTHPROTOCOL_PLATFORM == descriptor.protocol )
        return false;

    COAuth2Resolved resolved;
    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, m_settings.allowInsecureHttp, resolved, error ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << " is not supported as configured: " << error );
        return false;
    }

    if ( UsesHttps( descriptor, resolved ) && !COAuth2Http::IsHttpsSupported() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << " is not supported: " << COAuth2Http::GetHttpsUnavailableMessage() );
        return false;
    }

    if ( AUTHPROTOCOL_OPENID2 == descriptor.protocol )
    {
        // The provider specific parts of the protocol come from the profile that a plugin registered with the descriptor
        const IOpenId2ProviderProfile* profile = FindOpenId2Profile( descriptor );
        if ( GUCEF_NULL == profile )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << " is not supported: there is no OpenID 2.0 profile for it" );
            return false;
        }
        if ( !resolved.relayBase.IsNULLOrEmpty() && !profile->IsRelayAllowed() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << " is not supported as configured: the provider does not take a relay" );
            return false;
        }
        return !resolved.authorizationEndpoint.IsNULLOrEmpty() && ( resolved.redirectMode == "loopback" || resolved.redirectMode == "manual" );
    }

    if ( !CAuthJson::IsAvailable() )
        return false;
    if ( AUTHPROTOCOL_OIDC == descriptor.protocol && !CAuthCrypto::IsRsaAvailable() )
        return false;
    if ( resolved.clientId.IsNULLOrEmpty() )
        return false;
    return resolved.wantsDiscovery || ( !resolved.tokenEndpoint.IsNULLOrEmpty() && ( !resolved.authorizationEndpoint.IsNULLOrEmpty() || !resolved.deviceAuthorizationEndpoint.IsNULLOrEmpty() ) );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::StartInteractive( const CAuthProviderDescriptor& descriptor ,
                                 const CAuthProviderSettings& settings     ,
                                 const CAuthOptions& options               ,
                                 IAuthFlowSink& sink                       ,
                                 CORE::CFutureResultSource& result         )
{GUCEF_TRACE;

    ReapFinished();
    if ( m_flows.size() >= m_settings.maxActiveLogins )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: Too many logins are under way" );
        return false;
    }
    if ( !IsSupported( descriptor, settings ) )
        return false;

    COAuth2Resolved resolved;
    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, m_settings.allowInsecureHttp, resolved, error ) )
        return false;

    bool browserCapable = 0 != ( descriptor.capabilities & AUTHCAP_BROWSER ) && ( !resolved.authorizationEndpoint.IsNULLOrEmpty() || resolved.wantsDiscovery );
    bool deviceCapable = 0 != ( descriptor.capabilities & AUTHCAP_DEVICE_CODE ) && ( !resolved.deviceAuthorizationEndpoint.IsNULLOrEmpty() || resolved.wantsDiscovery );

    TOAuth2FlowKind kind = OAUTH2FLOW_AUTHORIZATION_CODE;
    if ( AUTHPROTOCOL_OPENID2 == descriptor.protocol )
    {
        if ( !browserCapable )
            return false;
        kind = OAUTH2FLOW_OPENID2;
    }
    else
    if ( options.preferDeviceCode && deviceCapable )
        kind = OAUTH2FLOW_DEVICE_CODE;
    else
    if ( browserCapable )
        kind = OAUTH2FLOW_AUTHORIZATION_CODE;
    else
    if ( deviceCapable )
        kind = OAUTH2FLOW_DEVICE_CODE;
    else
        return false;

    COAuth2Flow* flow = GUCEF_NEW COAuth2Flow( *this, kind, descriptor, settings, options, sink, result );
    if ( !flow->Start() )
    {
        GUCEF_DELETE flow;
        return false;
    }
    m_flows.push_back( flow );
    UpdateHousekeeping();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::StartRefresh( const CAuthProviderDescriptor& descriptor ,
                             const CAuthProviderSettings& settings     ,
                             const CAccountRef& account                ,
                             const CSecretString& refreshToken         ,
                             IAuthFlowSink& sink                       ,
                             CORE::CFutureResultSource& result         )
{GUCEF_TRACE;

    ReapFinished();
    if ( m_flows.size() >= m_settings.maxActiveLogins || refreshToken.IsEmpty() || !IsSupported( descriptor, settings ) )
        return false;
    if ( AUTHPROTOCOL_OPENID2 == descriptor.protocol || 0 == ( descriptor.capabilities & AUTHCAP_REFRESHABLE ) )
        return false;

    CAuthOptions options;
    COAuth2Flow* flow = GUCEF_NEW COAuth2Flow( *this, OAUTH2FLOW_REFRESH, descriptor, settings, options, sink, result );
    flow->SetRefreshInput( account, refreshToken );
    if ( !flow->Start() )
    {
        GUCEF_DELETE flow;
        return false;
    }
    m_flows.push_back( flow );
    UpdateHousekeeping();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::Revoke( const CAuthProviderDescriptor& descriptor ,
                       const CAuthProviderSettings& settings     ,
                       const CAccountRef& account                ,
                       const CSecretString& refreshToken         )
{GUCEF_TRACE;

    COAuth2Resolved resolved;
    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, m_settings.allowInsecureHttp, resolved, error ) )
        return false;
    // The discovery document of the provider may name it; it is there when a login used it, nothing is fetched here
    COidcMetadata metadata;
    if ( !resolved.discoveryUrl.IsNULLOrEmpty() && TryGetDiscovery( resolved.discoveryUrl, metadata ) )
    {
        if ( !COAuth2Protocol::Resolve( descriptor, settings, &metadata, m_settings.allowInsecureHttp, resolved, error ) )
            return false;
    }
    // A provider without a revocation endpoint has nothing to do
    if ( resolved.revocationEndpoint.IsNULLOrEmpty() )
        return true;
    if ( refreshToken.IsEmpty() )
        return true;

    CORE::CString::StringMap parameters;
    parameters[ "token" ] = refreshToken.Reveal();
    parameters[ "token_type_hint" ] = "refresh_token";
    COAuth2Protocol::AddClientParameters( descriptor, resolved, parameters );
    CORE::CString contentType;
    CORE::CString body = COAuth2Protocol::MakeBody( descriptor, parameters, contentType );
    CAuthHttpRequest request;
    if ( !request.Post( resolved.revocationEndpoint, contentType, body, m_settings ) )
        return false;
    CSecretString authorization;
    if ( COAuth2Protocol::MakeBasicAuthorization( descriptor, resolved, authorization ) )
        request.SetAuthorization( authorization );

    CORE::CFutureResult future = m_http.Start( request );
    if ( future.HasNoFuture() )
        return false;
    m_revokes.push_back( future );
    UpdateHousekeeping();
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": the revocation of a token was sent" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::SubmitRedirectUrl( const CORE::CString& redirectUrl )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_flows.size(); ++i )
        if ( !m_flows[ i ]->IsFinished() && m_flows[ i ]->SubmitRedirectUrl( redirectUrl ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::TryGetDiscovery( const CORE::CString& address, COidcMetadata& metadata ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDiscoveryMap::const_iterator i = m_discovery.find( address );
    if ( i == m_discovery.end() || GetMonotonicTimeInMs() >= (*i).second.expiresAtMs )
        return false;
    metadata = (*i).second.metadata;
    return true;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::StoreDiscovery( const CORE::CString& address, const COidcMetadata& metadata )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_discovery.size() >= MaxDiscoveryEntries && m_discovery.find( address ) == m_discovery.end() )
        m_discovery.clear();
    SDiscoveryEntry& entry = m_discovery[ address ];
    entry.metadata = metadata;
    entry.expiresAtMs = GetMonotonicTimeInMs() + (UInt64) m_settings.discoveryTtlInSeconds * 1000;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::StoreKeySet( const CORE::CString& address, const CAuthHttpResponse& response )
{GUCEF_TRACE;

    CJwks jwks;
    if ( !jwks.Parse( response.body.GetConstBufferPtr(), response.body.GetDataSize() ) )
        return false;
    m_jwks.Store( address, jwks, CJwksCache::ParseMaxAge( response.cacheControl ), GetMonotonicTimeInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::MakeValidationOptions( const COAuth2Resolved& resolved, CJwtValidationOptions& options ) const
{GUCEF_TRACE;

    options = CJwtValidationOptions();
    options.issuer = resolved.issuer;
    options.audience = resolved.clientId;
    options.clockSkewInSeconds = m_settings.clockSkewInSeconds;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Engine::MakeVerifyValidationOptions( const COAuth2Resolved& resolved, CJwtValidationOptions& options ) const
{GUCEF_TRACE;

    options = CJwtValidationOptions();
    options.issuer = resolved.issuer;
    options.audiences = resolved.acceptedClientIds;
    options.clockSkewInSeconds = m_settings.clockSkewInSeconds;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::CheckTenant( const COAuth2Resolved& resolved, const CORE::CString& tenantId )
{GUCEF_TRACE;

    if ( resolved.issuer.HasSubstr( "{tid}" ) < 0 )
        return true;
    CORE::CString::StringMap::const_iterator tenant = resolved.parameters.find( "tenant" );
    return COAuth2Protocol::IsTenantAllowed( tenant == resolved.parameters.end() ? CORE::CString::Empty : (*tenant).second, tenantId );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::MakeResultFromClaims( const CAuthProviderDescriptor& descriptor  ,
                                     const CORE::CString::StringMap& claims     ,
                                     const CORE::CString::StringMap& parameters ,
                                     CAuthResult& result                        )
{GUCEF_TRACE;

    result = CAuthResult();
    if ( !descriptor.MakeAccountRef( claims, parameters, result.accountRef ) )
        return false;
    result.providerName = descriptor.name;

    if ( !descriptor.displayNameClaim.IsNULLOrEmpty() )
    {
        CORE::CString::StringMap::const_iterator name = claims.find( descriptor.displayNameClaim );
        if ( name != claims.end() && GetByteLength( (*name).second ) <= MaxDisplayNameBytes )
        {
            // Control characters do not belong in a name that is shown; the rest passes as the bytes it is
            CORE::CDynamicBuffer clean;
            clean.SetDataSize( 0 );
            const char* p = (*name).second.C_String();
            UInt32 size = GetByteLength( (*name).second );
            for ( UInt32 i=0; i<size; ++i )
                if ( (unsigned char) p[ i ] >= 0x20 && 0x7F != (unsigned char) p[ i ] )
                    clean.Append( &p[ i ], 1 );
            result.displayName = MakeString( static_cast< const char* >( clean.GetConstBufferPtr() ), clean.GetDataSize() );
        }
    }
    if ( !descriptor.avatarClaim.IsNULLOrEmpty() )
    {
        CORE::CString::StringMap::const_iterator avatar = claims.find( descriptor.avatarClaim );
        if ( avatar != claims.end() && CAuthUrl::IsAcceptableEndpoint( (*avatar).second, false ) )
            result.avatarUrl = (*avatar).second;
    }
    COAuth2Protocol::FindLoginHint( descriptor, claims, result.loginHint );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::Fetch( const CAuthHttpRequest& request, CAuthHttpResponse& response )
{GUCEF_TRACE;

    bool ok = m_http.Execute( request, response );
    if ( !response.transportOk )
    {
        if ( AUTHHTTP_NO_HTTPS == response.failure )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << COAuth2Http::GetHttpsUnavailableMessage() );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: a request got no answer: " << response.DescribeForLog() );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::ResolveSync( const CAuthProviderDescriptor& descriptor, const CAuthProviderSettings& settings, COAuth2Resolved& resolved )
{GUCEF_TRACE;

    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( descriptor, settings, GUCEF_NULL, m_settings.allowInsecureHttp, resolved, error ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": " << error );
        return false;
    }
    if ( !resolved.wantsDiscovery )
        return true;

    COidcMetadata metadata;
    if ( !TryGetDiscovery( resolved.discoveryUrl, metadata ) )
    {
        CAuthHttpRequest request;
        CAuthHttpResponse response;
        if ( !request.Get( resolved.discoveryUrl, m_settings ) || !Fetch( request, response ) || !response.IsSuccessStatus() ||
             !metadata.Parse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), resolved.issuer, m_settings.allowInsecureHttp ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": the discovery document could not be fetched or is not acceptable" );
            return false;
        }
        StoreDiscovery( resolved.discoveryUrl, metadata );
    }
    if ( !COAuth2Protocol::Resolve( descriptor, settings, &metadata, m_settings.allowInsecureHttp, resolved, error ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": " << error );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::GetKeySync( const COAuth2Resolved& resolved, const CORE::CString& keyId, CJwk& key, TJwtStatus& status )
{GUCEF_TRACE;

    status = JWT_KEY_NOT_FOUND;
    if ( resolved.jwksUri.IsNULLOrEmpty() )
        return false;

    bool fetched = false;
    for ( ; ; )
    {
        CJwksCache::ELookup found = m_jwks.Lookup( resolved.jwksUri, keyId, GetMonotonicTimeInMs(), key );
        if ( CJwksCache::LOOKUP_FOUND == found )
        {
            status = JWT_OK;
            return true;
        }
        if ( CJwksCache::LOOKUP_UNKNOWN_KEY == found || fetched )
            return false;

        CAuthHttpRequest request;
        CAuthHttpResponse response;
        fetched = true;
        if ( !request.Get( resolved.jwksUri, m_settings ) || !Fetch( request, response ) || !response.IsSuccessStatus() || !StoreKeySet( resolved.jwksUri, response ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: The key set of a provider could not be fetched or is not acceptable" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::Verify( const CAuthProviderDescriptor& descriptor ,
                       const CAuthProviderSettings& settings     ,
                       const CAuthProof& proof                   ,
                       CAuthResult& result                       )
{GUCEF_TRACE;

    result = CAuthResult();
    if ( proof.IsEmpty() || AUTHPROTOCOL_PLATFORM == descriptor.protocol )
        return false;

    bool kindFits = ( AUTHPROTOCOL_OIDC == descriptor.protocol && proof.kind == CAuthProof::KindOidcIdToken ) ||
                    ( AUTHPROTOCOL_OPENID2 == descriptor.protocol && proof.kind == CAuthProof::KindOpenId2Assertion ) ||
                    ( AUTHPROTOCOL_OAUTH2 == descriptor.protocol && proof.kind == CAuthProof::KindOAuth2AccessToken && !descriptor.tokenVerifyStyle.IsNULLOrEmpty() );
    if ( !kindFits )
        return false;

    COAuth2Resolved resolved;
    if ( !ResolveSync( descriptor, settings, resolved ) )
        return false;

    bool verified = false;
    switch ( descriptor.protocol )
    {
        case AUTHPROTOCOL_OIDC :        verified = VerifyIdToken( descriptor, resolved, proof, result ); break;
        case AUTHPROTOCOL_OPENID2 :     verified = VerifyOpenId2( descriptor, resolved, proof, result ); break;
        case AUTHPROTOCOL_OAUTH2 :      verified = VerifyAccessToken( descriptor, resolved, proof, result ); break;
        default :                       break;
    }
    if ( !verified )
        result = CAuthResult();
    return verified;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::VerifyIdToken( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result )
{GUCEF_TRACE;

    if ( resolved.acceptedClientIds.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": a proof is refused: the settings have no client id (clientId, audiences) to check the audience against" );
        return false;
    }
    if ( !CAuthJson::IsAvailable() || !CAuthCrypto::IsRsaAvailable() )
        return false;

    CJwt jwt;
    TJwtStatus status = jwt.Parse( proof.data.Reveal() );
    CJwk key;
    if ( JWT_OK == status && !GetKeySync( resolved, jwt.GetKeyId(), key, status ) )
        status = JWT_KEY_NOT_FOUND;
    if ( JWT_OK == status )
        status = jwt.VerifySignature( key );

    CORE::CString tenant;
    if ( JWT_OK == status )
    {
        CJwtValidationOptions options;
        MakeVerifyValidationOptions( resolved, options );
        status = jwt.ValidateClaims( options, &tenant );
        if ( JWT_OK == status && !CheckTenant( resolved, tenant ) )
            status = JWT_TENANT;
    }
    if ( JWT_OK != status )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": a proof is refused: " << JwtStatusToString( status ) );
        return false;
    }

    CORE::CString::StringMap claims;
    jwt.GetClaims( claims );
    if ( !MakeResultFromClaims( descriptor, claims, resolved.parameters, result ) )
        return false;
    Int64 expires = 0;
    if ( jwt.GetClaimInt64( "exp", expires ) && expires > 0 )
        result.expiresAtMs = (UInt64) expires * 1000;
    result.proof = proof;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::VerifyOpenId2( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result )
{GUCEF_TRACE;

    COpenId2Assertion assertion;
    if ( !assertion.LoadFromQueryString( proof.data.Reveal() ) )
        return false;

    // A proof for a provider that has no profile (the plugin of the backend is not loaded) is refused like one for an unknown provider
    const IOpenId2ProviderProfile* profile = FindOpenId2Profile( descriptor );
    if ( GUCEF_NULL == profile )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": a proof is refused: there is no OpenID 2.0 profile for the provider" );
        return false;
    }

    UInt64 nowSeconds = GetEpochTimeInMs() / 1000;
    TOpenId2Status status = COpenId2::Validate( assertion, resolved.authorizationEndpoint, *profile, CORE::CString::Empty, resolved.loopbackPath,
                                                nowSeconds, m_settings.openId2MaxAgeInSeconds, m_verifyNonces, resolved.relayBase );
    if ( OPENID2_OK != status )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": a proof is refused: " << OpenId2StatusToString( status ) );
        return false;
    }

    CORE::CString nonce = assertion.Get( "openid.response_nonce" );
    CAuthHttpRequest request;
    CAuthHttpResponse response;
    if ( !request.Post( resolved.authorizationEndpoint, "application/x-www-form-urlencoded", COpenId2::BuildCheckAuthenticationBody( assertion ), m_settings ) ||
         !Fetch( request, response ) )
    {
        // The provider was not asked: the proof may be offered again
        m_verifyNonces.Release( nonce );
        return false;
    }
    if ( 200 != response.statusCode || !COpenId2::IsValidAnswer( response.body.GetConstBufferPtr(), response.body.GetDataSize() ) )
        return false;

    CORE::CString subject;
    if ( !profile->ExtractSubject( resolved.authorizationEndpoint, assertion.Get( "openid.claimed_id" ), assertion.Get( "openid.identity" ), subject ) )
        return false;
    CORE::CString::StringMap claims;
    claims[ descriptor.subjectClaim.IsNULLOrEmpty() ? CORE::CString( "sub" ) : descriptor.subjectClaim ] = subject;
    CORE::CString displayName;
    if ( !descriptor.displayNameClaim.IsNULLOrEmpty() && profile->GetDisplayName( assertion, displayName ) )
        claims[ descriptor.displayNameClaim ] = displayName;
    if ( !MakeResultFromClaims( descriptor, claims, resolved.parameters, result ) )
        return false;
    result.proof = proof;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The JSON object of an answer, or false */
static bool
ReadObject( const CAuthHttpResponse& response, UInt32 maxBytes, CORE::CDataNode& root )
{
    return response.IsSuccessStatus() && !response.tooLarge && CAuthJson::Parse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), maxBytes, CAuthJson::DefaultMaxDepth, false, root );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Engine::VerifyAccessToken( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, const CAuthProof& proof, CAuthResult& result )
{GUCEF_TRACE;

    if ( resolved.tokenVerifyEndpoint.IsNULLOrEmpty() || !CAuthJson::IsAvailable() )
        return false;
    if ( resolved.acceptedClientIds.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": a proof is refused: the settings have no client id (clientId, audiences) to check the audience against" );
        return false;
    }

    CORE::CString::StringMap claims;

    if ( descriptor.tokenVerifyStyle == "facebook-debug-token" )
    {
        // Only the app secret makes the check say which app the token was made for, and the secret belongs to the app of clientId:
        // the tokens of the other apps in the list are accepted too, but the question is asked with the app token of clientId
        if ( resolved.clientSecret.IsEmpty() || resolved.clientId.IsNULLOrEmpty() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Engine: " << descriptor.name << ": the app id (clientId) and the app secret are needed to verify an access token" );
            return false;
        }
        CORE::CString::StringMap parameters;
        parameters[ "input_token" ] = proof.data.Reveal();
        parameters[ "access_token" ] = resolved.clientId + "|" + resolved.clientSecret.Reveal();
        CAuthHttpRequest request;
        CAuthHttpResponse response;
        CORE::CDataNode root;
        if ( !request.Get( CAuthUrl::AppendQuery( resolved.tokenVerifyEndpoint, parameters ), m_settings ) || !Fetch( request, response ) ||
             !ReadObject( response, m_settings.maxResponseBytes, root ) )
            return false;
        const CORE::CDataNode* data = CAuthJson::GetDocument( root ).FindChild( "data" );
        bool valid = false;
        CORE::CString appId, userId;
        Int64 expiresAt = 0;
        if ( GUCEF_NULL == data || !CAuthJson::GetBool( *data, "is_valid", valid ) || !valid ||
             !CAuthJson::GetString( *data, "app_id", appId ) || !ContainsString( resolved.acceptedClientIds, appId ) ||
             !CAuthJson::GetString( *data, "user_id", userId ) || userId.IsNULLOrEmpty() )
            return false;
        if ( CAuthJson::GetInt64( *data, "expires_at", expiresAt ) && expiresAt > 0 && (UInt64) expiresAt <= GetEpochTimeInMs() / 1000 )
            return false;

        // The profile of the token (the Graph "me") gives the name; it has to be of the same user
        if ( !resolved.userInfoEndpoint.IsNULLOrEmpty() )
        {
            CAuthHttpRequest profileRequest;
            CAuthHttpResponse profileResponse;
            CORE::CDataNode profileRoot;
            if ( profileRequest.Get( resolved.userInfoEndpoint, m_settings ) )
            {
                profileRequest.SetBearer( proof.data );
                if ( Fetch( profileRequest, profileResponse ) && ReadObject( profileResponse, m_settings.maxResponseBytes, profileRoot ) )
                {
                    CORE::CString::StringMap profile;
                    CAuthJson::Flatten( CAuthJson::GetDocument( profileRoot ), profile );
                    CORE::CString::StringMap::const_iterator profileUser = profile.find( descriptor.subjectClaim );
                    if ( profileUser != profile.end() && (*profileUser).second != userId )
                        return false;
                    claims = profile;
                }
            }
        }
        claims[ descriptor.subjectClaim ] = userId;
    }
    else
    if ( descriptor.tokenVerifyStyle == "amazon-tokeninfo" )
    {
        // The audience of the token has to be one of our clients, otherwise it is a token for another app
        CORE::CString::StringMap parameters;
        parameters[ "access_token" ] = proof.data.Reveal();
        CAuthHttpRequest request;
        CAuthHttpResponse response;
        CORE::CDataNode root;
        if ( !request.Get( CAuthUrl::AppendQuery( resolved.tokenVerifyEndpoint, parameters ), m_settings ) || !Fetch( request, response ) ||
             !ReadObject( response, m_settings.maxResponseBytes, root ) )
            return false;
        const CORE::CDataNode& info = CAuthJson::GetDocument( root );
        CORE::CString audience, userId;
        if ( !CAuthJson::GetString( info, "aud", audience ) || !ContainsString( resolved.acceptedClientIds, audience ) )
            return false;
        CAuthJson::GetString( info, "user_id", userId );

        // Then who it is: the profile of the token
        if ( !resolved.userInfoEndpoint.IsNULLOrEmpty() )
        {
            CAuthHttpRequest profileRequest;
            CAuthHttpResponse profileResponse;
            CORE::CDataNode profileRoot;
            if ( profileRequest.Get( resolved.userInfoEndpoint, m_settings ) )
            {
                profileRequest.SetBearer( proof.data );
                if ( Fetch( profileRequest, profileResponse ) && ReadObject( profileResponse, m_settings.maxResponseBytes, profileRoot ) )
                {
                    CORE::CString::StringMap profile;
                    CAuthJson::Flatten( CAuthJson::GetDocument( profileRoot ), profile );
                    CORE::CString::StringMap::const_iterator profileUser = profile.find( descriptor.subjectClaim );
                    if ( profileUser != profile.end() )
                    {
                        // The two answers have to agree
                        if ( !userId.IsNULLOrEmpty() && userId != (*profileUser).second )
                            return false;
                        userId = (*profileUser).second;
                    }
                    claims = profile;
                }
            }
        }
        if ( userId.IsNULLOrEmpty() )
            return false;
        claims[ descriptor.subjectClaim ] = userId;
    }
    else
        return false;

    if ( !MakeResultFromClaims( descriptor, claims, resolved.parameters, result ) )
        return false;
    result.proof = proof;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
