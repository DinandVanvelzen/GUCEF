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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_COAuth2Engine.h"

#include "gucefAUTH_COAuth2Flow.h"

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

static const UInt32 FlowTickInMs = 10;
static const UInt32 RefreshTimeoutInMs = 60000;
static const UInt32 MaxPollFailures = 3;

/*-------------------------------------------------------------------------*/

COAuth2Flow::COAuth2Flow( COAuth2Engine& engine                      ,
                          TOAuth2FlowKind kind                       ,
                          const CAuthProviderDescriptor& descriptor  ,
                          const CAuthProviderSettings& settings      ,
                          const CAuthOptions& options                ,
                          IAuthFlowSink& sink                        ,
                          const CORE::CFutureResultSource& result    )
    : CORE::CObserver()
    , IAuthFlowSinkWatcher()
    , m_engine( engine )
    , m_kind( kind )
    , m_descriptor( descriptor )
    , m_settings( settings )
    , m_options( options )
    , m_sink( &sink )
    , m_result( result )
    , m_timer( FlowTickInMs )
    , m_stage( STAGE_NEW )
    , m_resolved()
    , m_metadata()
    , m_hasMetadata( false )
    , m_deadlineMs( 0 )
    , m_pending( CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED ) )
    , m_hasPending( false )
    , m_pendingStage( STAGE_NEW )
    , m_listener( GUCEF_NULL )
    , m_state()
    , m_nonce()
    , m_verifier()
    , m_redirectUri()
    , m_returnTo()
    , m_code()
    , m_assertion()
    , m_assertionNonce()
    , m_nonceReserved( false )
    , m_deviceCode()
    , m_deviceExpiresAtMs( 0 )
    , m_intervalMs( 0 )
    , m_nextPollAtMs( 0 )
    , m_lastPollAtMs( 0 )
    , m_pollFailures( 0 )
    , m_pollInFlight( false )
    , m_account()
    , m_refreshToken()
    , m_tokens()
    , m_claims()
    , m_tenantId()
    , m_hasIdentity( false )
    , m_keysFetched( false )
    , m_checkNonce( false )
    , m_idTokenExpiresAtMs( 0 )
{GUCEF_TRACE;

    TEventCallback timerCallback( this, &COAuth2Flow::OnTimerUpdate );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
    sink.AddWatcher( this );
}

/*-------------------------------------------------------------------------*/

COAuth2Flow::~COAuth2Flow()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_timer.SetEnabled( false );

    if ( GUCEF_NULL != m_sink )
    {
        m_sink->RemoveWatcher( this );
        m_sink = GUCEF_NULL;
    }
    if ( STAGE_DONE != m_stage )
        m_result.Cancel();
    Finish();
    GUCEF_DELETE m_listener;
    m_listener = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COAuth2Flow::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::COAuth2Flow";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt64
COAuth2Flow::Now( void ) const
{GUCEF_TRACE;

    return GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::SetRefreshInput( const CAccountRef& account, const CSecretString& refreshToken )
{GUCEF_TRACE;

    m_account = account;
    m_refreshToken = refreshToken;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::IsFinished( void ) const
{GUCEF_TRACE;

    return STAGE_DONE == m_stage;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnFlowSinkDestroyed( IAuthFlowSink* sink )
{GUCEF_TRACE;

    // The provider is gone, nobody is waiting for the outcome any more
    if ( sink == m_sink )
    {
        m_sink = GUCEF_NULL;
        if ( STAGE_DONE != m_stage )
        {
            m_result.Cancel();
            Finish();
        }
    }
}

/*-------------------------------------------------------------------------*/

static void
ApplyOptionScopes( const CAuthOptions& options, COAuth2Resolved& resolved )
{GUCEF_TRACE;

    CORE::CString::StringVector base = resolved.scopes;
    COAuth2Protocol::MergeScopes( base, options.scopes, resolved.scopes );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::Start( void )
{GUCEF_TRACE;

    UInt32 timeout = m_options.timeoutInMs > 0 ? m_options.timeoutInMs : CAuthOptions::DefaultTimeoutInMs;
    // a device code lives as long as the server says (its expires_in ends the login), the time of the browser is no limit for it
    if ( OAUTH2FLOW_DEVICE_CODE == m_kind && CAuthOptions::DefaultTimeoutInMs == timeout )
        timeout = CAuthOptions::MaxDeviceCodeTimeoutInMs;
    if ( OAUTH2FLOW_REFRESH == m_kind )
        timeout = RefreshTimeoutInMs;
    m_deadlineMs = Now() + timeout;

    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( m_descriptor, m_settings, GUCEF_NULL, m_engine.GetSettings().allowInsecureHttp, m_resolved, error ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << " cannot start: " << error );
        return false;
    }
    ApplyOptionScopes( m_options, m_resolved );

    if ( m_resolved.wantsDiscovery )
    {
        COidcMetadata cached;
        if ( m_engine.TryGetDiscovery( m_resolved.discoveryUrl, cached ) )
        {
            m_metadata = cached;
            m_hasMetadata = true;
            if ( !COAuth2Protocol::Resolve( m_descriptor, m_settings, &m_metadata, m_engine.GetSettings().allowInsecureHttp, m_resolved, error ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << " cannot start: " << error );
                return false;
            }
            ApplyOptionScopes( m_options, m_resolved );
        }
        else
        {
            CAuthHttpRequest request;
            if ( !request.Get( m_resolved.discoveryUrl, m_engine.GetSettings() ) )
                return false;
            if ( !StartRequest( request, STAGE_DISCOVERY ) )
                return false;
            m_timer.SetEnabled( true );
            return true;
        }
    }

    if ( !BeginResolved() )
        return false;
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::BeginResolved( void )
{GUCEF_TRACE;

    switch ( m_kind )
    {
        case OAUTH2FLOW_AUTHORIZATION_CODE :    return BeginAuthorizationCode();
        case OAUTH2FLOW_DEVICE_CODE :           return BeginDevice();
        case OAUTH2FLOW_OPENID2 :               return BeginOpenId2();
        case OAUTH2FLOW_REFRESH :               return BeginRefresh();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::StartRequest( const CAuthHttpRequest& request, EStage stage )
{GUCEF_TRACE;

    m_pending = m_engine.GetHttp().Start( request );
    if ( m_pending.HasNoFuture() )
    {
        m_hasPending = false;
        return false;
    }
    m_hasPending = true;
    m_pendingStage = stage;
    m_stage = stage;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::PrepareRedirect( CORE::CString& redirectUri, CORE::CString& origin, const CORE::CString& stateParameter )
{GUCEF_TRACE;

    if ( m_resolved.redirectMode == "loopback" )
    {
        UInt64 now = Now();
        UInt32 remaining = m_deadlineMs > now ? (UInt32) ( m_deadlineMs - now ) : 1;
        m_listener = GUCEF_NEW CLoopbackRedirectListener();
        if ( !m_listener->Start( m_resolved.loopbackPort, m_resolved.loopbackPath, CSecretString( m_state ), remaining, stateParameter ) )
        {
            GUCEF_DELETE m_listener;
            m_listener = GUCEF_NULL;
            return false;
        }
        redirectUri = m_listener->GetRedirectUri();
        origin = m_listener->GetOrigin();
        return true;
    }

    // The application brings the redirect back
    redirectUri = m_resolved.redirectUri;
    origin = CAuthUrl::GetOrigin( redirectUri );
    return !redirectUri.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::AnnounceUrl( const CORE::CString& url )
{GUCEF_TRACE;

    UInt64 now = Now();
    CAuthActionRequired action;
    action.kind = AUTHACTION_OPEN_URL;
    action.url = url;
    action.expiresAtMs = GetEpochTimeInMs() + ( m_deadlineMs > now ? m_deadlineMs - now : 0 );
    m_sink->OnActionRequired( action );

    // The event is raised either way: an application that opens the url itself, or shows it, has it
    if ( m_engine.GetSettings().launchBrowser && m_options.preferBrowser )
    {
        if ( CORE::OpenUrlInSystemBrowser( url ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the browser was opened" );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the browser could not be opened, the address is in the event" );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::BeginAuthorizationCode( void )
{GUCEF_TRACE;

    if ( m_resolved.authorizationEndpoint.IsNULLOrEmpty() || m_resolved.tokenEndpoint.IsNULLOrEmpty() || m_resolved.clientId.IsNULLOrEmpty() )
        return false;
    if ( AUTHPROTOCOL_OIDC == m_descriptor.protocol && m_resolved.jwksUri.IsNULLOrEmpty() )
        return false;

    if ( !COAuth2Protocol::MakeRandomToken( COAuth2Protocol::RandomTokenBytes, m_state ) ||
         !COAuth2Protocol::MakeRandomToken( COAuth2Protocol::RandomTokenBytes, m_nonce ) )
        return false;
    CORE::CString challenge;
    if ( m_descriptor.usesPkce )
    {
        if ( !COAuth2Protocol::MakeCodeVerifier( m_verifier ) || !COAuth2Protocol::MakeCodeChallenge( m_verifier, challenge ) )
            return false;
    }

    CORE::CString origin;
    if ( !PrepareRedirect( m_redirectUri, origin ) )
        return false;

    CORE::CString url = COAuth2Protocol::BuildAuthorizationUrl( m_descriptor, m_resolved, m_redirectUri, m_state, m_nonce, challenge, m_options.loginHint );

    // the hint is personal data: only whether there was one is said
    const bool loginHintUsed = m_descriptor.supportsLoginHint && COAuth2Protocol::IsAcceptableLoginHint( m_options.loginHint );
    const CORE::CString hintText = loginHintUsed ? "yes" : "no";
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": login hint used: " << hintText );
    m_checkNonce = AUTHPROTOCOL_OIDC == m_descriptor.protocol;
    m_stage = STAGE_AUTHORIZATION;
    AnnounceUrl( url );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::BeginOpenId2( void )
{GUCEF_TRACE;

    if ( m_resolved.authorizationEndpoint.IsNULLOrEmpty() || m_resolved.redirectMode == "custom-scheme" )
        return false;
    if ( !COAuth2Protocol::MakeRandomToken( COAuth2Protocol::RandomTokenBytes, m_state ) )
        return false;

    // With a relay the realm and the return address are a page of the developer's domain, which sends the browser on to the
    // listener of this login: the port and the state are in its query. Without one the return address is the listener itself.
    bool viaRelay = !m_resolved.relayBase.IsNULLOrEmpty();
    CORE::CString origin;
    if ( !PrepareRedirect( m_redirectUri, origin, viaRelay ? COpenId2Relay::StateParameter : CORE::CString( "state" ) ) || origin.IsNULLOrEmpty() )
        return false;

    if ( viaRelay )
    {
        if ( m_listener->GetPort() < COpenId2Relay::MinPort || !COpenId2Relay::IsValidState( m_state ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the listener has a port the relay does not take" );
            m_listener->Close();
            GUCEF_DELETE m_listener;
            m_listener = GUCEF_NULL;
            return false;
        }
        m_returnTo = COpenId2Relay::BuildReturnTo( m_resolved.relayBase, m_listener->GetPort(), m_state );
        origin = m_resolved.relayRealm;
    }
    else
    {
        // The state travels in the return address, the provider gives the query back with its answer added
        CORE::CString::StringMap stateParameter;
        stateParameter[ "state" ] = m_state;
        m_returnTo = CAuthUrl::AppendQuery( m_redirectUri, stateParameter );
    }
    CORE::CString url = COpenId2::BuildAuthenticationUrl( m_resolved.authorizationEndpoint, m_returnTo, origin );
    m_stage = STAGE_AUTHORIZATION;
    AnnounceUrl( url );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::BeginDevice( void )
{GUCEF_TRACE;

    if ( m_resolved.deviceAuthorizationEndpoint.IsNULLOrEmpty() || m_resolved.tokenEndpoint.IsNULLOrEmpty() || m_resolved.clientId.IsNULLOrEmpty() )
        return false;

    CORE::CString::StringMap parameters;
    COAuth2Protocol::AddClientParameters( m_descriptor, m_resolved, parameters );
    CORE::CString scope;
    for ( size_t i=0; i<m_resolved.scopes.size(); ++i )
    {
        if ( i > 0 )
            scope += ' ';
        scope += m_resolved.scopes[ i ];
    }
    if ( !scope.IsNULLOrEmpty() )
        parameters[ "scope" ] = scope;
    // Some providers want more than the standard asks for here, for example the start address of an instance
    CORE::CString startUrl = m_resolved.parameters.find( "startUrl" ) != m_resolved.parameters.end() ? m_resolved.parameters[ "startUrl" ] : CORE::CString::Empty;
    if ( !startUrl.IsNULLOrEmpty() && m_descriptor.jsonRequestBodies )
        parameters[ "start_url" ] = startUrl;

    CORE::CString contentType;
    CORE::CString body = COAuth2Protocol::MakeBody( m_descriptor, parameters, contentType );
    CAuthHttpRequest request;
    if ( !request.Post( m_resolved.deviceAuthorizationEndpoint, contentType, body, m_engine.GetSettings() ) )
        return false;
    CSecretString authorization;
    if ( COAuth2Protocol::MakeBasicAuthorization( m_descriptor, m_resolved, authorization ) )
        request.SetAuthorization( authorization );
    return StartRequest( request, STAGE_DEVICE_REQUEST );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::BeginRefresh( void )
{GUCEF_TRACE;

    if ( m_resolved.tokenEndpoint.IsNULLOrEmpty() || m_resolved.clientId.IsNULLOrEmpty() || m_refreshToken.IsEmpty() )
        return false;
    CORE::CString::StringMap parameters;
    parameters[ "grant_type" ] = "refresh_token";
    parameters[ "refresh_token" ] = m_refreshToken.Reveal();
    return StartTokenRequest( parameters );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::StartTokenRequest( const CORE::CString::StringMap& parameters )
{GUCEF_TRACE;

    CORE::CString::StringMap all = parameters;
    COAuth2Protocol::AddClientParameters( m_descriptor, m_resolved, all );
    CORE::CString contentType;
    CORE::CString body = COAuth2Protocol::MakeBody( m_descriptor, all, contentType );
    CAuthHttpRequest request;
    if ( !request.Post( m_resolved.tokenEndpoint, contentType, body, m_engine.GetSettings() ) )
        return false;
    CSecretString authorization;
    if ( COAuth2Protocol::MakeBasicAuthorization( m_descriptor, m_resolved, authorization ) )
        request.SetAuthorization( authorization );
    return StartRequest( request, STAGE_TOKEN );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnTimerUpdate( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnTimer();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnTimer( void )
{GUCEF_TRACE;

    if ( STAGE_DONE == m_stage )
        return;

    if ( m_result.IsCancellationRequested() )
    {
        Cancel();
        return;
    }
    UInt64 now = Now();
    if ( now >= m_deadlineMs )
    {
        Fail( AUTHFAIL_TIMEOUT, "The login was not finished in time" );
        return;
    }

    // The listener closes itself some time after the callback
    CLoopbackRedirectListener::EState listenerState = CLoopbackRedirectListener::STATE_IDLE;
    if ( GUCEF_NULL != m_listener )
        listenerState = m_listener->Poll();

    if ( m_hasPending )
    {
        CheckPending();
        return;
    }

    if ( STAGE_AUTHORIZATION == m_stage && GUCEF_NULL != m_listener )
    {
        if ( CLoopbackRedirectListener::STATE_RECEIVED == listenerState )
        {
            CORE::CString::StringMap parameters;
            if ( m_listener->TakeCallback( parameters ) )
                OnCallback( parameters );
        }
        else
        if ( CLoopbackRedirectListener::STATE_TIMED_OUT == listenerState )
            Fail( AUTHFAIL_TIMEOUT, "The login was not finished in time" );
        return;
    }

    if ( STAGE_DEVICE_WAIT == m_stage )
    {
        if ( now >= m_deviceExpiresAtMs )
        {
            Fail( AUTHFAIL_TIMEOUT, "The code expired before it was entered" );
            return;
        }
        if ( now >= m_nextPollAtMs )
            SendDevicePoll();
    }
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::CheckPending( void )
{GUCEF_TRACE;

    if ( !m_hasPending || !( m_pending.IsReady() || m_pending.HasNoFuture() ) )
        return;

    CAuthHttpResponse response;
    bool ok = m_engine.GetHttp().Collect( m_pending, response );
    m_hasPending = false;
    m_pending = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    OnHttpDone( ok, response );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnHttpDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    EStage stage = m_pendingStage;
    switch ( stage )
    {
        case STAGE_DISCOVERY :          OnDiscoveryDone( ok, response ); break;
        case STAGE_TOKEN :              OnTokenDone( ok, response ); break;
        case STAGE_KEYS :               OnKeysDone( ok, response ); break;
        case STAGE_USERINFO :           OnUserInfoDone( ok, response ); break;
        case STAGE_DEVICE_REQUEST :     OnDeviceRequestDone( ok, response ); break;
        case STAGE_OPENID_CHECK :       OnOpenIdCheckDone( ok, response ); break;
        default :                       break;
    }
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnDiscoveryDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    if ( !ok || !response.IsSuccessStatus() )
    {
        FailHttp( response, "the request for the discovery document" );
        return;
    }
    COidcMetadata metadata;
    if ( !metadata.Parse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), m_resolved.issuer, m_engine.GetSettings().allowInsecureHttp ) )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The discovery document of the provider is not acceptable" );
        return;
    }
    m_engine.StoreDiscovery( m_resolved.discoveryUrl, metadata );
    m_metadata = metadata;
    m_hasMetadata = true;

    CORE::CString error;
    if ( !COAuth2Protocol::Resolve( m_descriptor, m_settings, &m_metadata, m_engine.GetSettings().allowInsecureHttp, m_resolved, error ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": " << error );
        Fail( AUTHFAIL_NOT_SUPPORTED, "The provider cannot be used with these settings" );
        return;
    }
    ApplyOptionScopes( m_options, m_resolved );
    if ( !BeginResolved() )
        Fail( AUTHFAIL_NOT_SUPPORTED, "The provider cannot be used with these settings" );
    else
        m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Flow::SubmitRedirectUrl( const CORE::CString& redirectUrl )
{GUCEF_TRACE;

    if ( STAGE_AUTHORIZATION != m_stage || m_resolved.redirectMode == "loopback" )
        return false;
    UInt32 size = GetByteLength( redirectUrl );
    if ( 0 == size || size > CAuthUrl::MaxUrlBytes + CAuthUrl::MaxQueryBytes )
        return false;

    // The address without its query and fragment has to be the redirect uri of this login
    const char* p = redirectUrl.C_String();
    UInt32 queryAt = size, fragmentAt = size;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '?' == p[ i ] && queryAt == size )
            queryAt = i;
        if ( '#' == p[ i ] )
        {
            fragmentAt = i;
            break;
        }
    }
    UInt32 baseEnd = queryAt < fragmentAt ? queryAt : fragmentAt;
    if ( MakeString( p, baseEnd ) != m_redirectUri )
        return false;

    CORE::CString query;
    if ( queryAt < fragmentAt )
        query = MakeString( p + queryAt + 1, fragmentAt - queryAt - 1 );
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !CAuthUrl::ParseQuery( query, parameters, duplicate ) || duplicate )
        return false;

    // The state, in constant time
    CORE::CString::StringMap::const_iterator state = parameters.find( "state" );
    UInt32 expectedSize = GetByteLength( m_state );
    if ( state == parameters.end() || GetByteLength( (*state).second ) != expectedSize ||
         !CAuthCrypto::ConstantTimeEquals( (*state).second.C_String(), m_state.C_String(), expectedSize ) )
        return false;
    if ( parameters.find( "code" ) == parameters.end() && parameters.find( "error" ) == parameters.end() && parameters.find( "openid.mode" ) == parameters.end() )
        return false;

    OnCallback( parameters );
    return true;
}

/*-------------------------------------------------------------------------*/

/** An error code of a provider as text for a message: lower case letters, digits and underscores only */
static CORE::CString
SafeErrorCode( const CORE::CString& code )
{
    UInt32 size = GetByteLength( code );
    if ( 0 == size || size > 64 )
        return "unknown";
    const char* p = code.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = p[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '_' == c ) )
            return "unknown";
    }
    return code;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnCallback( const CORE::CString::StringMap& parameters )
{GUCEF_TRACE;

    if ( STAGE_AUTHORIZATION != m_stage )
        return;

    if ( OAUTH2FLOW_OPENID2 == m_kind )
    {
        CORE::CString::StringMap::const_iterator mode = parameters.find( "openid.mode" );
        if ( mode != parameters.end() && (*mode).second == "cancel" )
        {
            Fail( AUTHFAIL_DENIED, "The sign in was cancelled at the provider" );
            return;
        }
        if ( !m_assertion.Load( parameters ) )
        {
            Fail( AUTHFAIL_INVALID_RESPONSE, "The provider gave an incomplete assertion" );
            return;
        }
        const IOpenId2ProviderProfile* profile = m_engine.FindOpenId2Profile( m_descriptor );
        if ( GUCEF_NULL == profile )
        {
            Fail( AUTHFAIL_NOT_AVAILABLE, "There is no OpenID 2.0 profile for the provider" );
            return;
        }
        UInt64 nowSeconds = GetEpochTimeInMs() / 1000;
        TOpenId2Status status = COpenId2::Validate( m_assertion, m_resolved.authorizationEndpoint, *profile, m_returnTo, m_resolved.loopbackPath,
                                                    nowSeconds, m_engine.GetSettings().openId2MaxAgeInSeconds, m_engine.GetLoginNonceCache(), m_resolved.relayBase );
        if ( OPENID2_OK != status )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the assertion is refused: " << OpenId2StatusToString( status ) );
            Fail( AUTHFAIL_INVALID_RESPONSE, "The assertion of the provider was refused" );
            return;
        }
        m_nonceReserved = true;
        m_assertionNonce = m_assertion.Get( "openid.response_nonce" );

        CAuthHttpRequest request;
        if ( !request.Post( m_resolved.authorizationEndpoint, "application/x-www-form-urlencoded", COpenId2::BuildCheckAuthenticationBody( m_assertion ), m_engine.GetSettings() ) ||
             !StartRequest( request, STAGE_OPENID_CHECK ) )
            Fail( AUTHFAIL_NETWORK, "The provider could not be asked to check the assertion" );
        return;
    }

    // Authorization code: an error of the provider, or the code
    CORE::CString::StringMap::const_iterator error = parameters.find( "error" );
    if ( error != parameters.end() )
    {
        CORE::CString code = SafeErrorCode( (*error).second );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the provider ended the login with the error " << code );
        Fail( COAuth2Protocol::ReasonForError( code ), "The provider ended the login with an error" );
        return;
    }

    // RFC 9207: an answer that names its issuer has to name ours
    CORE::CString::StringMap::const_iterator iss = parameters.find( "iss" );
    if ( iss != parameters.end() && !m_resolved.issuer.IsNULLOrEmpty() && m_resolved.issuer.HasSubstr( "{tid}" ) < 0 && (*iss).second != m_resolved.issuer )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The answer is from another issuer" );
        return;
    }

    CORE::CString::StringMap::const_iterator code = parameters.find( "code" );
    if ( code == parameters.end() || code->second.IsNULLOrEmpty() )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The provider gave no code" );
        return;
    }
    m_code.Set( (*code).second );

    CORE::CString::StringMap tokenParameters;
    tokenParameters[ "grant_type" ] = "authorization_code";
    tokenParameters[ "code" ] = (*code).second;
    tokenParameters[ "redirect_uri" ] = m_redirectUri;
    if ( !m_verifier.IsNULLOrEmpty() )
        tokenParameters[ "code_verifier" ] = m_verifier;
    if ( !StartTokenRequest( tokenParameters ) )
        Fail( AUTHFAIL_NETWORK, "The token request could not be sent" );
    m_code.Clear();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnOpenIdCheckDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    if ( !ok )
    {
        // The provider was not asked: the assertion may come again
        if ( m_nonceReserved )
            m_engine.GetLoginNonceCache().Release( m_assertionNonce );
        m_nonceReserved = false;
        FailHttp( response, "the check of the assertion" );
        return;
    }
    if ( response.statusCode >= 500 )
    {
        FailHttp( response, "the check of the assertion" );
        return;
    }
    if ( 200 != response.statusCode || !COpenId2::IsValidAnswer( response.body.GetConstBufferPtr(), response.body.GetDataSize() ) )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The provider does not vouch for the assertion" );
        return;
    }

    const IOpenId2ProviderProfile* profile = m_engine.FindOpenId2Profile( m_descriptor );
    CORE::CString subject;
    if ( GUCEF_NULL == profile || !profile->ExtractSubject( m_resolved.authorizationEndpoint, m_assertion.Get( "openid.claimed_id" ), m_assertion.Get( "openid.identity" ), subject ) )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The assertion of the provider does not name an account" );
        return;
    }
    m_claims.clear();
    m_claims[ m_descriptor.subjectClaim.IsNULLOrEmpty() ? CORE::CString( "sub" ) : m_descriptor.subjectClaim ] = subject;
    CORE::CString displayName;
    if ( !m_descriptor.displayNameClaim.IsNULLOrEmpty() && profile->GetDisplayName( m_assertion, displayName ) )
        m_claims[ m_descriptor.displayNameClaim ] = displayName;
    Complete();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnDeviceRequestDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    if ( !ok )
    {
        FailHttp( response, "the device authorization request" );
        return;
    }
    COAuth2TokenResponse answer;
    if ( !COAuth2Protocol::ParseTokenResponse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), m_engine.GetSettings().maxResponseBytes, answer ) )
    {
        Fail( response.statusCode >= 500 ? AUTHFAIL_NETWORK : AUTHFAIL_INVALID_RESPONSE, "The provider gave no usable answer to the device authorization request" );
        return;
    }
    if ( !answer.error.IsNULLOrEmpty() )
    {
        CORE::CString code = SafeErrorCode( answer.error );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the device authorization request was refused: " << code );
        Fail( COAuth2Protocol::ReasonForError( code ), "The provider refused the device authorization request" );
        return;
    }
    if ( !response.IsSuccessStatus() || answer.deviceCode.IsEmpty() || answer.userCode.IsNULLOrEmpty() || answer.verificationUri.IsNULLOrEmpty() ||
         answer.userCode.Length() > 64 || answer.deviceCode.Length() > 2048 )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The device authorization answer is incomplete" );
        return;
    }
    bool insecure = m_engine.GetSettings().allowInsecureHttp;
    if ( !CAuthUrl::IsAcceptableEndpoint( answer.verificationUri, insecure ) )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The page where the code is entered is not an acceptable address" );
        return;
    }

    m_deviceCode = answer.deviceCode;
    const COAuth2EngineSettings& settings = m_engine.GetSettings();
    UInt64 now = Now();

    UInt64 lifetimeSeconds = answer.hasExpiresIn ? answer.expiresInSeconds : 900;
    if ( lifetimeSeconds < 5 )
        lifetimeSeconds = 5;
    if ( lifetimeSeconds > 3600 )
        lifetimeSeconds = 3600;
    m_deviceExpiresAtMs = now + lifetimeSeconds * settings.deviceIntervalUnitInMs;
    if ( m_deviceExpiresAtMs < m_deadlineMs )
        m_deadlineMs = m_deviceExpiresAtMs;

    UInt32 interval = answer.hasInterval ? answer.interval : settings.deviceDefaultIntervalInSeconds;
    if ( 0 == interval )
        interval = 1;
    m_intervalMs = interval * settings.deviceIntervalUnitInMs;
    m_lastPollAtMs = now;
    m_nextPollAtMs = now + m_intervalMs;

    CAuthActionRequired action;
    action.kind = AUTHACTION_SHOW_DEVICE_CODE;
    action.url = answer.verificationUri;
    if ( CAuthUrl::IsAcceptableEndpoint( answer.verificationUriComplete, insecure ) )
        action.urlWithCode = answer.verificationUriComplete;
    action.userCode = answer.userCode;
    action.expiresAtMs = GetEpochTimeInMs() + lifetimeSeconds * settings.deviceIntervalUnitInMs;
    if ( GUCEF_NULL != m_sink )
        m_sink->OnActionRequired( action );
    m_stage = STAGE_DEVICE_WAIT;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::SendDevicePoll( void )
{GUCEF_TRACE;

    CORE::CString::StringMap parameters;
    parameters[ "grant_type" ] = "urn:ietf:params:oauth:grant-type:device_code";
    parameters[ "device_code" ] = m_deviceCode.Reveal();
    m_lastPollAtMs = Now();
    m_pollInFlight = true;
    if ( !StartTokenRequest( parameters ) )
        Fail( AUTHFAIL_NETWORK, "The token request could not be sent" );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnTokenDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    bool polling = m_pollInFlight;
    m_pollInFlight = false;

    COAuth2TokenResponse answer;
    bool parsed = ok && COAuth2Protocol::ParseTokenResponse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), m_engine.GetSettings().maxResponseBytes, answer );

    if ( polling )
    {
        // A poll that did not get through is tried again, a few times
        bool transient = !ok || ( !parsed && response.statusCode >= 500 ) ||
                         ( parsed && ( answer.error == "temporarily_unavailable" || answer.error == "server_error" ) );
        if ( transient )
        {
            if ( ++m_pollFailures > MaxPollFailures )
            {
                if ( !ok )
                    FailHttp( response, "the token request" );
                else
                    Fail( AUTHFAIL_NETWORK, "The provider could not be reached" );
                return;
            }
            m_stage = STAGE_DEVICE_WAIT;
            m_nextPollAtMs = m_lastPollAtMs + m_intervalMs;
            return;
        }
        m_pollFailures = 0;
        if ( parsed && !answer.error.IsNULLOrEmpty() )
        {
            if ( answer.error == "authorization_pending" )
            {
                m_stage = STAGE_DEVICE_WAIT;
                m_nextPollAtMs = m_lastPollAtMs + m_intervalMs;
                return;
            }
            if ( answer.error == "slow_down" )
            {
                m_intervalMs += m_engine.GetSettings().deviceSlowDownIncrementInSeconds * m_engine.GetSettings().deviceIntervalUnitInMs;
                m_stage = STAGE_DEVICE_WAIT;
                m_nextPollAtMs = m_lastPollAtMs + m_intervalMs;
                return;
            }
        }
    }

    if ( !ok )
    {
        FailHttp( response, "the token request" );
        return;
    }
    if ( !parsed )
    {
        Fail( response.statusCode >= 500 ? AUTHFAIL_NETWORK : AUTHFAIL_INVALID_RESPONSE, "The provider gave no usable answer to the token request" );
        return;
    }
    if ( !answer.error.IsNULLOrEmpty() )
    {
        CORE::CString code = SafeErrorCode( answer.error );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the token request was refused: " << code );
        Fail( COAuth2Protocol::ReasonForError( code ), "The provider refused the token request" );
        return;
    }
    if ( !response.IsSuccessStatus() )
    {
        Fail( response.statusCode >= 500 ? AUTHFAIL_NETWORK : AUTHFAIL_INVALID_RESPONSE, "The provider answered the token request with an error" );
        return;
    }
    if ( !answer.hasAccessToken )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The provider gave no access token" );
        return;
    }
    if ( !answer.tokenType.IsNULLOrEmpty() && answer.tokenType.Lowercase() != "bearer" )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The provider gave a token of a type that is not supported" );
        return;
    }
    m_tokens = answer;
    ProcessTokens();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::ProcessTokens( void )
{GUCEF_TRACE;

    if ( AUTHPROTOCOL_OIDC == m_descriptor.protocol )
    {
        if ( m_tokens.idToken.IsEmpty() )
        {
            // A renewal does not have to bring an id token, a login has to
            if ( OAUTH2FLOW_REFRESH == m_kind )
            {
                m_hasIdentity = true;
                Complete();
                return;
            }
            Fail( AUTHFAIL_INVALID_RESPONSE, "The provider gave no id token" );
            return;
        }
        ValidateIdToken();
        return;
    }
    AfterIdentity();
}

/*-------------------------------------------------------------------------*/

static const char*
DescribeJwtFailure( TJwtStatus status )
{
    return JWT_OK == status ? "" : "The id token of the provider was refused";
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::ValidateIdToken( void )
{GUCEF_TRACE;

    CJwt jwt;
    TJwtStatus status = jwt.Parse( m_tokens.idToken.Reveal() );
    if ( JWT_OK != status )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the id token is refused: " << JwtStatusToString( status ) );
        Fail( AUTHFAIL_INVALID_RESPONSE, DescribeJwtFailure( status ) );
        return;
    }
    if ( m_resolved.jwksUri.IsNULLOrEmpty() )
    {
        Fail( AUTHFAIL_NOT_SUPPORTED, "The provider has no key set" );
        return;
    }

    CJwk key;
    CJwksCache::ELookup found = m_engine.GetJwksCache().Lookup( m_resolved.jwksUri, jwt.GetKeyId(), Now(), key );
    if ( CJwksCache::LOOKUP_FETCH_NEEDED == found || CJwksCache::LOOKUP_REFRESH_NEEDED == found )
    {
        if ( m_keysFetched )
            found = CJwksCache::LOOKUP_UNKNOWN_KEY;
        else
        {
            m_keysFetched = true;
            CAuthHttpRequest request;
            if ( !request.Get( m_resolved.jwksUri, m_engine.GetSettings() ) || !StartRequest( request, STAGE_KEYS ) )
                Fail( AUTHFAIL_NETWORK, "The key set of the provider could not be requested" );
            return;
        }
    }
    if ( CJwksCache::LOOKUP_FOUND != found )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the id token is refused: " << JwtStatusToString( JWT_KEY_NOT_FOUND ) );
        Fail( AUTHFAIL_INVALID_RESPONSE, DescribeJwtFailure( JWT_KEY_NOT_FOUND ) );
        return;
    }

    status = jwt.VerifySignature( key );
    if ( JWT_OK == status )
    {
        CJwtValidationOptions options;
        m_engine.MakeValidationOptions( m_resolved, options );
        options.checkNonce = m_checkNonce;
        options.nonce = m_nonce;
        options.accessToken = m_tokens.accessToken;
        CORE::CString tenant;
        status = jwt.ValidateClaims( options, &tenant );
        if ( JWT_OK == status && !COAuth2Engine::CheckTenant( m_resolved, tenant ) )
            status = JWT_TENANT;
        if ( JWT_OK == status )
        {
            m_tenantId = tenant;
            jwt.GetClaims( m_claims );
            Int64 expires = 0;
            if ( jwt.GetClaimInt64( "exp", expires ) && expires > 0 )
                m_idTokenExpiresAtMs = (UInt64) expires * 1000;
        }
    }
    if ( JWT_OK != status )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the id token is refused: " << JwtStatusToString( status ) );
        Fail( AUTHFAIL_INVALID_RESPONSE, DescribeJwtFailure( status ) );
        return;
    }
    m_hasIdentity = true;
    AfterIdentity();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnKeysDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    if ( !ok || !response.IsSuccessStatus() )
    {
        FailHttp( response, "the request for the key set" );
        return;
    }
    if ( !m_engine.StoreKeySet( m_resolved.jwksUri, response ) )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The key set of the provider is not acceptable" );
        return;
    }
    ValidateIdToken();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::AfterIdentity( void )
{GUCEF_TRACE;

    bool needUserInfo = false;
    bool required = AUTHPROTOCOL_OAUTH2 == m_descriptor.protocol;
    if ( OAUTH2FLOW_REFRESH != m_kind )
    {
        if ( required )
            needUserInfo = true;
        else
        if ( !m_resolved.userInfoEndpoint.IsNULLOrEmpty() && !m_descriptor.displayNameClaim.IsNULLOrEmpty() &&
             m_claims.find( m_descriptor.displayNameClaim ) == m_claims.end() && m_tokens.hasAccessToken )
            needUserInfo = true;
    }

    if ( needUserInfo )
    {
        if ( m_resolved.userInfoEndpoint.IsNULLOrEmpty() || !m_tokens.hasAccessToken )
        {
            if ( required )
                Fail( AUTHFAIL_NOT_SUPPORTED, "The provider has no way to ask who logged in" );
            else
                Complete();
            return;
        }
        StartUserInfo();
        return;
    }
    Complete();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::StartUserInfo( void )
{GUCEF_TRACE;

    CAuthHttpRequest request;
    if ( !request.Get( m_resolved.userInfoEndpoint, m_engine.GetSettings() ) )
    {
        Fail( AUTHFAIL_NOT_SUPPORTED, "The profile address of the provider is not usable" );
        return;
    }
    request.SetBearer( m_tokens.accessToken );
    if ( !StartRequest( request, STAGE_USERINFO ) )
        Fail( AUTHFAIL_NETWORK, "The profile request could not be sent" );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::OnUserInfoDone( bool ok, CAuthHttpResponse& response )
{GUCEF_TRACE;

    bool required = AUTHPROTOCOL_OAUTH2 == m_descriptor.protocol;
    CORE::CString::StringMap info;
    bool usable = false;
    if ( ok && response.IsSuccessStatus() )
    {
        CORE::CDataNode root;
        if ( CAuthJson::Parse( response.body.GetConstBufferPtr(), response.body.GetDataSize(), m_engine.GetSettings().maxResponseBytes, CAuthJson::DefaultMaxDepth, false, root ) )
        {
            CAuthJson::Flatten( CAuthJson::GetDocument( root ), info );
            usable = !info.empty();
        }
    }

    if ( !usable )
    {
        if ( required )
        {
            if ( ok )
                Fail( AUTHFAIL_INVALID_RESPONSE, "The profile of the account could not be read" );
            else
                FailHttp( response, "the profile request" );
            return;
        }
        // The profile is a nicety next to a verified id token
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the profile could not be read, the login goes on without it" );
        Complete();
        return;
    }

    if ( required )
        m_claims = info;
    else
    {
        // The profile has to be of the same subject; it only adds what the id token did not say
        CORE::CString::StringMap::const_iterator tokenSub = m_claims.find( "sub" );
        CORE::CString::StringMap::const_iterator infoSub = info.find( "sub" );
        if ( tokenSub != m_claims.end() && ( infoSub == info.end() || (*infoSub).second != (*tokenSub).second ) )
        {
            Fail( AUTHFAIL_INVALID_RESPONSE, "The profile is of another account than the id token" );
            return;
        }
        CORE::CString::StringMap::const_iterator i = info.begin();
        while ( i != info.end() )
        {
            if ( m_claims.find( (*i).first ) == m_claims.end() )
                m_claims[ (*i).first ] = (*i).second;
            ++i;
        }
    }
    Complete();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::Complete( void )
{GUCEF_TRACE;

    CAuthResult result;
    bool havePerson = false;
    if ( OAUTH2FLOW_REFRESH == m_kind )
    {
        result.accountRef = m_account;
        havePerson = true;
        if ( !m_claims.empty() )
        {
            // A renewed id token has to be of the account that is renewed
            CAuthResult fromClaims;
            if ( !COAuth2Engine::MakeResultFromClaims( m_descriptor, m_claims, m_resolved.parameters, fromClaims ) || fromClaims.accountRef != m_account )
            {
                Fail( AUTHFAIL_INVALID_RESPONSE, "The renewed login is of another account" );
                return;
            }
            result.displayName = fromClaims.displayName;
            result.avatarUrl = fromClaims.avatarUrl;
            result.loginHint = fromClaims.loginHint;
        }
    }
    else
        havePerson = COAuth2Engine::MakeResultFromClaims( m_descriptor, m_claims, m_resolved.parameters, result );

    if ( !havePerson || !result.accountRef.IsValid() )
    {
        Fail( AUTHFAIL_INVALID_RESPONSE, "The login does not name an account" );
        return;
    }

    if ( AUTHPROTOCOL_OPENID2 == m_descriptor.protocol )
    {
        result.proof = CAuthProof( CAuthProof::KindOpenId2Assertion, m_assertion.ToQueryString() );
    }
    else
    {
        result.accessToken = m_tokens.accessToken;
        result.idToken = m_tokens.idToken;
        if ( !m_tokens.refreshToken.IsEmpty() )
            result.refreshToken = m_tokens.refreshToken;
        else
            result.refreshToken = m_refreshToken;
        if ( m_tokens.hasExpiresIn )
            result.expiresAtMs = GetEpochTimeInMs() + m_tokens.expiresInSeconds * 1000;
        if ( !m_tokens.scope.IsNULLOrEmpty() )
            result.scopes = m_tokens.scope.ParseElements( ' ', false );
        else
        if ( OAUTH2FLOW_REFRESH != m_kind )
            result.scopes = m_resolved.scopes;

        if ( !m_tokens.idToken.IsEmpty() && m_hasIdentity )
            result.proof = CAuthProof( CAuthProof::KindOidcIdToken, m_tokens.idToken.Reveal() );
        else
        if ( !m_descriptor.tokenVerifyStyle.IsNULLOrEmpty() && m_tokens.hasAccessToken )
            result.proof = CAuthProof( CAuthProof::KindOAuth2AccessToken, m_tokens.accessToken.Reveal() );
    }

    if ( GUCEF_NULL != m_sink )
        m_sink->OnLoginCompleted( m_result, result );
    else
        m_result.Cancel();
    Finish();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::Fail( TAuthFailureReason reason, const char* message )
{GUCEF_TRACE;

    if ( STAGE_DONE == m_stage )
        return;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": the login failed (" << AuthFailureReasonToString( reason ) << "): " << message );
    if ( GUCEF_NULL != m_sink )
        m_sink->OnLoginFailed( m_result, reason, message );
    else
        m_result.Cancel();
    Finish();
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::FailHttp( const CAuthHttpResponse& response, const char* step )
{GUCEF_TRACE;

    TAuthFailureReason reason = AUTHFAIL_NETWORK;
    CORE::CString message = "The provider could not be reached";
    if ( !response.DescribeFailure( reason, message ) )
    {
        // An answer that is a success cannot be why this is called
        reason = AUTHFAIL_INVALID_RESPONSE;
        message = "The provider gave no usable answer";
    }

    if ( !response.transportOk && AUTHHTTP_NO_HTTPS == response.failure )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": " << COAuth2Http::GetHttpsUnavailableMessage() );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OAuth2Flow: " << m_descriptor.name << ": " << step << " got no usable answer: " << response.DescribeForLog() );
    Fail( reason, message.C_String() );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::Cancel( void )
{GUCEF_TRACE;

    Fail( AUTHFAIL_CANCELLED, "The login was cancelled" );
}

/*-------------------------------------------------------------------------*/

void
COAuth2Flow::Finish( void )
{GUCEF_TRACE;

    if ( m_nonceReserved && STAGE_DONE != m_stage && !m_assertionNonce.IsNULLOrEmpty() )
    {
        // An assertion that was not checked to the end can be offered again
        if ( STAGE_OPENID_CHECK == m_stage )
            m_engine.GetLoginNonceCache().Release( m_assertionNonce );
    }
    m_stage = STAGE_DONE;
    m_timer.SetEnabled( false );
    if ( m_hasPending )
    {
        m_pending.RequestCancellation();
        m_hasPending = false;
    }
    if ( GUCEF_NULL != m_listener )
        m_listener->Close();

    // Nothing of the login stays in memory longer than it has to
    m_code.Clear();
    m_deviceCode.Clear();
    m_refreshToken.Clear();
    m_tokens = COAuth2TokenResponse();
    m_verifier.Clear();
    m_claims.clear();
    m_assertion = COpenId2Assertion();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
