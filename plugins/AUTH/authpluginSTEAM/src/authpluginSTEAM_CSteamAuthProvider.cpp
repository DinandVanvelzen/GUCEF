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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H
#include "authpluginSTEAM_CSteamOpenId2Profile.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H ? */

#include "authpluginSTEAM_CSteamAuthProvider.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CSteamAuthProvider::ProviderName = "steam";

// The Steam Web API method that checks a ticket. To confirm against the documentation of the Steamworks Web API
static const char* const AuthenticateUserTicketPath = "/ISteamUserAuth/AuthenticateUserTicket/v1/";

// The longest answer of the Web API that is read, bytes
static const UInt32 MaxWebApiResponseSize = 65536;

// The longest text of a Steam error description that is passed on in a message
static const UInt32 MaxErrorDescriptionSize = 128;

// EResult values the failure reason is chosen by: k_EResultNoConnection, k_EResultTimeout, k_EResultServiceUnavailable
static const Int32 SteamResultNoConnection = 3;
static const Int32 SteamResultTimeout = 16;
static const Int32 SteamResultServiceUnavailable = 20;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whether the text is one or more decimal digits and at most 20 of them (fits an UInt64) */
static bool
IsSteamId( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 length = AUTH::GetByteLength( text );
    if ( length < 1 || length > 20 )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
        if ( chars[ i ] < '0' || chars[ i ] > '9' )
            return false;
    // 0 is no account
    return StringToUInt64( text, 0 ) != 0;
}

/*-------------------------------------------------------------------------*/

/** Whether the text is an even number of hex digits that is not longer than maxBytes bytes */
static bool
IsHexTicket( const CORE::CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    UInt32 length = AUTH::GetByteLength( text );
    if ( length < 2 || 0 != ( length % 2 ) || length > maxBytes * 2 )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = chars[ i ];
        bool digit = ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
        if ( !digit )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Keeps printable ASCII only and caps the length: text from a remote service ends up in messages */
static CORE::CString
SanitizeRemoteText( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString clean;
    UInt32 length = AUTH::GetByteLength( text );
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<length && i<MaxErrorDescriptionSize; ++i )
    {
        char c = chars[ i ];
        clean.Append( (Int32) ( ( c >= 0x20 && c < 0x7F ) ? c : '?' ) );
    }
    return clean;
}

/*-------------------------------------------------------------------------*/

static bool
StartsWithNoCase( const CORE::CString& text, const char* prefix )
{GUCEF_TRACE;

    return text.Lowercase().StartsWith( prefix );
}

/*-------------------------------------------------------------------------*/

/** The node at the path below the document node. A data store codec gives the document as the root or as its only child */
static const CORE::CDataNode*
FindObject( const CORE::CDataNode& root, const char* first, const char* second )
{GUCEF_TRACE;

    const CORE::CDataNode* start = &root;
    if ( GUCEF_NULL == start->FindChild( first ) )
    {
        CORE::CDataNode::const_iterator c = root.ConstBegin();
        while ( c != root.ConstEnd() )
        {
            if ( GUCEF_NULL != (*c)->FindChild( first ) )
            {
                start = (*c);
                break;
            }
            ++c;
        }
    }
    const CORE::CDataNode* node = start->FindChild( first );
    if ( GUCEF_NULL != node && GUCEF_NULL != second )
        node = node->FindChild( second );
    return node;
}

/*-------------------------------------------------------------------------*/

/** Parses a json text with the json codec of the process */
static bool
ParseJson( const CORE::CString& text, CORE::CDataNode& root )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: There is no json codec to read the answer of the Steam Web API with" );
        return false;
    }
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( text, false );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &root, &access );
}

/*-------------------------------------------------------------------------*/

/** Gets the url with the query for the request, the key in it is never logged */
static CORE::CString
EncodeQueryValue( const CORE::CString& value )
{GUCEF_TRACE;

    CORE::CString encoded;
    CORE::CUrlEncoder::Encode( value, encoded );
    return encoded;
}

/** The descriptor of the provider: the one in the registry of the process (the application may have changed it in its configuration), else the one of the plugin */
static AUTH::CAuthProviderDescriptor
GetSteamDescriptor( void )
{GUCEF_TRACE;

    AUTH::CAuthProviderDescriptor descriptor;
    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    if ( GUCEF_NULL == global || !global->GetDescriptorRegistry().GetDescriptor( CSteamAuthProvider::ProviderName, descriptor ) )
        descriptor = CSteamOpenId2Profile::MakeDescriptor();

    // The provider is the provider of this name, whatever the configuration made of the rest of the description
    descriptor.name = CSteamAuthProvider::ProviderName;
    return descriptor;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSteamAuthProvider::CPump::CPump( CSteamAuthProvider& owner )
    : CORE::CObserver()
    , m_owner( owner )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamAuthProvider::CPump::~CPump()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamAuthProvider::CPump::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider::CPump";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::CPump::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CTimer::TimerUpdateEvent == eventId )
        m_owner.OnTimer();
}

/*-------------------------------------------------------------------------*/

CSteamAuthProvider::CSteamAuthProvider( void )
    : AUTH::CDescriptorAuthProvider( GetSteamDescriptor() )
    , m_lock()
    , m_steamSettings()
    , m_runtime()
    , m_shim()
    , m_usesRuntime( true )
    , m_ready( false )
    , m_lastFailureInMs( 0 )
    , m_hasFailed( false )
    , m_lastFailureReason()
    , m_weInitialized( false )
    , m_pending()
    , m_issued()
    , m_timer( CSteamAuthSettings::DefaultCallbackIntervalInMs )
    , m_pump( *this )
{GUCEF_TRACE;

    m_pump.SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent );
}

/*-------------------------------------------------------------------------*/

CSteamAuthProvider::CSteamAuthProvider( const TSteamShim& shim )
    : AUTH::CDescriptorAuthProvider( GetSteamDescriptor() )
    , m_lock()
    , m_steamSettings()
    , m_runtime()
    , m_shim( shim )
    , m_usesRuntime( false )
    , m_ready( false )
    , m_lastFailureInMs( 0 )
    , m_hasFailed( false )
    , m_lastFailureReason()
    , m_weInitialized( false )
    , m_pending()
    , m_issued()
    , m_timer( CSteamAuthSettings::DefaultCallbackIntervalInMs )
    , m_pump( *this )
{GUCEF_TRACE;

    m_pump.SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent );
}

/*-------------------------------------------------------------------------*/

CSteamAuthProvider::~CSteamAuthProvider()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    m_pump.UnsubscribeAllFromObserver();

    MT::CScopeMutex lock( m_lock );

    // A ticket that is not used any more is not left valid
    for ( size_t i=0; i<m_pending.size(); ++i )
    {
        if ( GUCEF_NULL != m_shim.cancelTicket )
            m_shim.cancelTicket( m_shim.context, m_pending[ i ].ticketHandle );
        m_pending[ i ].source.Cancel();
    }
    m_pending.clear();

    for ( TIssuedMap::iterator i = m_issued.begin(); i != m_issued.end(); ++i )
        if ( GUCEF_NULL != m_shim.cancelTicket )
            m_shim.cancelTicket( m_shim.context, (*i).second );
    m_issued.clear();

    if ( m_weInitialized )
    {
        if ( m_usesRuntime )
            m_runtime.SetOwnsApi( m_steamSettings.ownsSteamApi );
        else
        if ( m_steamSettings.ownsSteamApi && GUCEF_NULL != m_shim.shutdown )
            m_shim.shutdown( m_shim.context );
    }
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::SetShim( const TSteamShim& shim )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_shim = shim;
    m_usesRuntime = false;
    m_ready = false;
    m_hasFailed = false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return AUTH::CDescriptorAuthProvider::GetCapabilities() | AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::EnsureReady( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( m_ready && ( GUCEF_NULL == m_shim.isInitialized || m_shim.isInitialized( m_shim.context ) ) )
        return true;
    m_ready = false;

    UInt64 now = AUTH::GetMonotonicTimeInMs();
    if ( m_hasFailed && ( now - m_lastFailureInMs ) < m_steamSettings.retryIntervalInMs )
    {
        reason = m_lastFailureReason;
        return false;
    }

    CORE::CString failure;
    if ( m_usesRuntime && GUCEF_NULL == m_shim.init )
    {
        if ( !m_runtime.Load( m_steamSettings.runtimeLibrary ) || !m_runtime.FillShim( m_shim ) )
            failure = "The Steam runtime library cannot be loaded";
    }

    if ( failure.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL == m_shim.init )
        {
            failure = "There is no Steam API to use";
        }
        else
        {
            char text[ 512 ];
            text[ 0 ] = '\0';
            if ( m_shim.init( m_shim.context, text, sizeof( text ) ) )
            {
                m_ready = true;
                m_hasFailed = false;
                m_weInitialized = true;
                m_lastFailureReason.Clear();
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The Steam API is initialized" );
                return true;
            }
            failure = '\0' == text[ 0 ] ? CORE::CString( "The Steam API could not be started" ) : CORE::CString( text );
        }
    }

    // Logged on the first time and when the reason changes, the check is repeated at the retry interval
    if ( !m_hasFailed || m_lastFailureReason != failure )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: Steam is not available: " << failure );
    m_hasFailed = true;
    m_lastFailureInMs = now;
    m_lastFailureReason = failure;
    reason = failure;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::PrepareTicketRequest( bool& useWebApi, CORE::CString& reason ) const
{GUCEF_TRACE;

    const bool haveWebApi = GUCEF_NULL != m_shim.requestWebApiTicket;
    const bool haveSession = GUCEF_NULL != m_shim.requestSessionTicket;

    if ( m_steamSettings.ticketMode == "webapi" )
    {
        useWebApi = true;
        if ( !haveWebApi )
        {
            reason = "The Steam runtime cannot create a ticket for the web api (GetAuthTicketForWebApi needs a newer Steamworks SDK)";
            return false;
        }
    }
    else
    if ( m_steamSettings.ticketMode == "session" )
    {
        useWebApi = false;
        if ( !haveSession )
        {
            reason = "The Steam runtime cannot create a session ticket (GetAuthSessionTicket)";
            return false;
        }
    }
    else
    {
        useWebApi = haveWebApi;
        if ( !haveWebApi && !haveSession )
        {
            reason = "The Steam runtime cannot create a ticket";
            return false;
        }
    }

    if ( useWebApi && m_steamSettings.ticketIdentity.IsNULLOrEmpty() )
    {
        reason = "The setting ticketIdentity is not set, a ticket for the web api needs the identity of the service that uses it";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::IsClientAvailable( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString reason;
    if ( !EnsureReady( reason ) )
        return false;
    bool useWebApi = false;
    return PrepareTicketRequest( useWebApi, reason );
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    // The login in the browser is the one that needs nothing of this machine, so it is asked first: the Steam runtime is only looked for when it has to be
    return AUTH::CDescriptorAuthProvider::IsAvailable() || IsClientAvailable();
}

/*-------------------------------------------------------------------------*/

/** Whether a login with the Steam client can be made now: it is there, it can make a ticket, it is connected and says who the user is */
bool
CSteamAuthProvider::CanUseClient( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString reason;
    if ( !EnsureReady( reason ) )
        return false;
    bool useWebApi = false;
    if ( !PrepareTicketRequest( useWebApi, reason ) )
        return false;
    if ( GUCEF_NULL != m_shim.isLoggedOn && !m_shim.isLoggedOn( m_shim.context ) )
        return false;
    return GUCEF_NULL == m_shim.getSteamId || 0 != m_shim.getSteamId( m_shim.context );
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    return Start( false, future );
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    // With the Steam client there is no browser step: the login is the same as the silent one. Without a client that can be
    // used the browser is the way, unless that is not available either: then the login ends with the reason the client gives
    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( CanUseClient() || !AUTH::CDescriptorAuthProvider::IsAvailable() )
        return Start( true, future );
    return AUTH::CDescriptorAuthProvider::BeginLogin( options, future );
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::Start( bool interactive, CORE::CFutureResult& future )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    CORE::CString reason;
    bool useWebApi = false;
    bool ready = EnsureReady( reason );
    if ( ready )
        ready = PrepareTicketRequest( useWebApi, reason );
    if ( ready && GUCEF_NULL != m_shim.isLoggedOn && !m_shim.isLoggedOn( m_shim.context ) )
    {
        reason = "The Steam client is not connected to Steam (offline mode)";
        ready = false;
    }
    UInt64 steamId = 0;
    if ( ready )
    {
        steamId = GUCEF_NULL != m_shim.getSteamId ? m_shim.getSteamId( m_shim.context ) : 0;
        if ( 0 == steamId )
        {
            reason = "Steam did not tell who the user is";
            ready = false;
        }
    }

    if ( !ready )
    {
        // A login in the background gives up quietly, one that the person asked for tells why it did not happen
        if ( !interactive )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SteamAuthProvider: No silent login: " << reason );
            return false;
        }
        CORE::CFutureResultSource failed;
        future = failed.GetFuture();
        FailLogin( failed, AUTH::AUTHFAIL_NOT_AVAILABLE, reason );
        return true;
    }

    AUTH::CAccountRef account( CSteamOpenId2Profile::AccountNamespace, CORE::UInt64ToString( steamId ) );
    if ( !account.IsValid() )
        return false;

    CORE::CString displayName;
    if ( GUCEF_NULL != m_shim.getPersonaName )
    {
        char persona[ 256 ];
        persona[ 0 ] = '\0';
        if ( m_shim.getPersonaName( m_shim.context, persona, sizeof( persona ) ) )
            displayName = persona;
    }

    CORE::CFutureResultSource source;
    future = source.GetFuture();

    SPending pending;
    pending.source = source;
    pending.account = account;
    pending.displayName = displayName;
    pending.deadlineInMs = AUTH::GetMonotonicTimeInMs() + m_steamSettings.ticketTimeoutInMs;

    if ( useWebApi )
    {
        pending.ticketHandle = m_shim.requestWebApiTicket( m_shim.context, m_steamSettings.ticketIdentity.C_String() );
        if ( SteamInvalidTicketHandle == pending.ticketHandle )
        {
            FailLogin( source, AUTH::AUTHFAIL_INTERNAL, "Steam did not create a ticket" );
            return true;
        }
        m_pending.push_back( pending );
        UpdateTimer();
        return true;
    }

    // The session ticket is there at once
    SSteamTicketResponse sessionTicket;
    UInt32 ticketSize = 0;
    pending.ticketHandle = m_shim.requestSessionTicket( m_shim.context, sessionTicket.ticket, SteamMaxTicketSize, &ticketSize );
    if ( SteamInvalidTicketHandle == pending.ticketHandle || 0 == ticketSize || ticketSize > SteamMaxTicketSize )
    {
        if ( SteamInvalidTicketHandle != pending.ticketHandle && GUCEF_NULL != m_shim.cancelTicket )
            m_shim.cancelTicket( m_shim.context, pending.ticketHandle );
        FailLogin( source, AUTH::AUTHFAIL_INTERNAL, "Steam did not create a session ticket" );
        return true;
    }
    EndLogin( pending, sessionTicket.ticket, ticketSize );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::RememberIssued( const AUTH::CAccountRef& account, UInt32 ticketHandle )
{GUCEF_TRACE;

    // A ticket that is replaced by a new one for the same account is not left valid
    TIssuedMap::iterator i = m_issued.find( account );
    if ( i != m_issued.end() && (*i).second != ticketHandle && GUCEF_NULL != m_shim.cancelTicket )
        m_shim.cancelTicket( m_shim.context, (*i).second );
    m_issued[ account ] = ticketHandle;
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::EndLogin( SPending& pending, const UInt8* ticket, UInt32 ticketSize )
{GUCEF_TRACE;

    RememberIssued( pending.account, pending.ticketHandle );

    AUTH::CAuthResult result;
    result.accountRef = pending.account;
    result.displayName = pending.displayName;
    result.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, AUTH::CAuthCrypto::ToHex( ticket, ticketSize ) );
    CompleteLogin( pending.source, result );
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::HandleResponse( const SSteamTicketResponse& response )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pending.size(); ++i )
    {
        if ( m_pending[ i ].ticketHandle != response.ticketHandle )
            continue;

        SPending pending = m_pending[ i ];
        m_pending.erase( m_pending.begin() + i );

        if ( SteamResultOk != response.result || 0 == response.ticketSize || response.ticketSize > SteamMaxTicketSize )
        {
            AUTH::TAuthFailureReason reason = AUTH::AUTHFAIL_INTERNAL;
            if ( SteamResultNoConnection == response.result || SteamResultTimeout == response.result || SteamResultServiceUnavailable == response.result )
                reason = AUTH::AUTHFAIL_NETWORK;
            CORE::CString message = "Steam could not create the ticket (EResult ";
            message += CORE::Int32ToString( response.result );
            message += ")";
            FailLogin( pending.source, reason, message );
            return;
        }
        EndLogin( pending, response.ticket, response.ticketSize );
        return;
    }

    // An answer for a ticket nobody waits for (it timed out, or was cancelled): not an error
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SteamAuthProvider: A ticket answer arrived for a ticket that is not waited for any more" );
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::OnTicketResponse( const SSteamTicketResponse& response )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    HandleResponse( response );
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::UpdateTimer( void )
{GUCEF_TRACE;

    bool needed = !m_pending.empty();
    if ( needed )
    {
        UInt32 interval = m_steamSettings.callbackIntervalInMs > 0 ? m_steamSettings.callbackIntervalInMs : CSteamAuthSettings::DefaultCallbackIntervalInMs;
        if ( m_timer.GetInterval() != interval )
            m_timer.SetInterval( interval );
    }
    if ( needed != m_timer.GetEnabled() )
        m_timer.SetEnabled( needed );
}

/*-------------------------------------------------------------------------*/

void
CSteamAuthProvider::OnTimer( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_pending.empty() && m_steamSettings.callbackMode == "manual" )
    {
        if ( GUCEF_NULL != m_shim.runFrame )
            m_shim.runFrame( m_shim.context );
        if ( GUCEF_NULL != m_shim.pollTicketResponse )
        {
            SSteamTicketResponse response;
            while ( m_shim.pollTicketResponse( m_shim.context, &response ) )
                HandleResponse( response );
        }
    }

    UInt64 now = AUTH::GetMonotonicTimeInMs();
    size_t i = 0;
    while ( i < m_pending.size() )
    {
        SPending& pending = m_pending[ i ];
        bool cancelled = pending.source.IsCancellationRequested();
        bool timedOut = now >= pending.deadlineInMs;
        if ( !cancelled && !timedOut )
        {
            ++i;
            continue;
        }

        SPending ended = pending;
        m_pending.erase( m_pending.begin() + i );
        if ( GUCEF_NULL != m_shim.cancelTicket )
            m_shim.cancelTicket( m_shim.context, ended.ticketHandle );
        if ( cancelled )
            FailLogin( ended.source, AUTH::AUTHFAIL_CANCELLED, "The login was cancelled" );
        else
            FailLogin( ended.source, AUTH::AUTHFAIL_TIMEOUT, "Steam did not give the ticket in time" );
    }

    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TIssuedMap::iterator i = m_issued.find( account );
    if ( i == m_issued.end() )
        return true;

    if ( GUCEF_NULL != m_shim.cancelTicket )
        m_shim.cancelTicket( m_shim.context, (*i).second );
    m_issued.erase( i );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The ticket of " << account.ToString() << " is cancelled" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::ParseAuthenticateUserTicketResponse( const CORE::CString& body, CORE::CString& steamId, CORE::CString& reason )
{GUCEF_TRACE;

    steamId.Clear();
    reason.Clear();

    CORE::CDataNode root;
    if ( !ParseJson( body, root ) )
    {
        reason = "The answer of the Steam Web API cannot be read";
        return false;
    }

    // {"response":{"params":{"result":"OK","steamid":"..."}}} or {"response":{"error":{"errorcode":..,"errordesc":".."}}}
    const CORE::CDataNode* params = FindObject( root, "response", "params" );
    if ( GUCEF_NULL == params )
    {
        const CORE::CDataNode* error = FindObject( root, "response", "error" );
        if ( GUCEF_NULL != error )
        {
            reason = "Steam refused the ticket: ";
            reason += SanitizeRemoteText( error->GetAttributeValueOrChildValueByName( "errordesc" ).AsString() );
            return false;
        }
        reason = "The answer of the Steam Web API has no parameters";
        return false;
    }

    CORE::CString result = params->GetAttributeValueOrChildValueByName( "result" ).AsString();
    if ( result != "OK" )
    {
        reason = "Steam did not accept the ticket (result ";
        reason += SanitizeRemoteText( result );
        reason += ")";
        return false;
    }

    CORE::CString id = params->GetAttributeValueOrChildValueByName( "steamid" ).AsString();
    if ( !IsSteamId( id ) )
    {
        reason = "The answer of the Steam Web API has no valid steamid";
        return false;
    }

    steamId = id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::VerifyWithWebApi( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    CSteamAuthSettings settings;
    {
        MT::CScopeMutex lock( m_lock );
        settings = m_steamSettings;
    }

    if ( settings.webApiKey.IsEmpty() || settings.appId.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: A ticket cannot be verified without the settings webApiKey and appId" );
        return false;
    }

    const CORE::CString& ticket = proof.data.Reveal();
    if ( !IsHexTicket( ticket, SteamMaxTicketSize ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The ticket of the proof is not hex of an acceptable size" );
        return false;
    }

    // The key travels in the query: only over https, unless the settings say it is a test
    const bool secure = StartsWithNoCase( settings.webApiBaseUrl, "https://" );
    if ( !secure && !( settings.allowInsecureHttp && StartsWithNoCase( settings.webApiBaseUrl, "http://" ) ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The webApiBaseUrl is not an https url, a ticket is not verified over it" );
        return false;
    }

    // The address has to be a plain one: no user information, no fragment, https (http only for the loopback address)
    if ( !AUTH::CAuthUrl::IsAcceptableEndpoint( settings.webApiBaseUrl, settings.allowInsecureHttp ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The webApiBaseUrl is not an acceptable address, a ticket is not verified over it" );
        return false;
    }

    CORE::CString url = settings.webApiBaseUrl;
    while ( url.EndsWith( "/" ) )
        url = url.SubstrFromRange( 0, url.Length() - 1 );
    url += AuthenticateUserTicketPath;
    const bool keyInHeader = settings.webApiKeyPlacement == "header";
    url += "?";
    if ( !keyInHeader )
    {
        url += "key=";
        url += EncodeQueryValue( settings.webApiKey.Reveal() );
        url += "&";
    }
    url += "appid=";
    url += EncodeQueryValue( settings.appId );
    url += "&ticket=";
    url += ticket;
    if ( !settings.ticketIdentity.IsNULLOrEmpty() )
    {
        url += "&identity=";
        url += EncodeQueryValue( settings.ticketIdentity );
    }

    WEB::CHttpRequestData request;
    request.requestType = "GET";
    if ( !request.SetUrl( url ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The webApiBaseUrl cannot be used" );
        return false;
    }

    if ( keyInHeader )
        request.otherHeaders[ "x-webapi-key" ] = settings.webApiKey.Reveal();

    WEB::CHttpClientRequestOptions options;
    options.totalTimeoutInMs = settings.webApiTimeoutInMs;
    options.connectTimeoutInMs = settings.webApiTimeoutInMs;
    options.followRedirects = false;        // the key is in the url: it is not sent anywhere the settings did not name
    options.verifyPeer = true;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transfer;
    if ( !client.Execute( request, options, response, &transfer ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The Steam Web API did not answer (" << WEB::HttpTransferErrorClassToString( transfer.errorClass ) << "): " << transfer.errorDescription );
        return false;
    }

    if ( 200 != response.statusCode )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The Steam Web API answered with status " << response.statusCode );
        return false;
    }
    if ( response.content.GetDataSize() > MaxWebApiResponseSize )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The answer of the Steam Web API is too large" );
        return false;
    }

    CORE::CString body;
    if ( response.content.GetDataSize() > 0 )
        body = CORE::CString( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    CORE::CString steamId;
    CORE::CString reason;
    if ( !ParseAuthenticateUserTicketResponse( body, steamId, reason ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: Ticket not verified: " << reason );
        return false;
    }

    AUTH::CAccountRef account( CSteamOpenId2Profile::AccountNamespace, steamId );
    if ( !account.IsValid() )
        return false;

    result = AUTH::CAuthResult();
    result.providerName = ProviderName;
    result.accountRef = account;
    result.proof = proof;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: Ticket verified for " << account.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::VerifyPeerTicket( const AUTH::CAccountRef& claimed, const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    if ( !claimed.IsValid() || claimed.GetProvider() != CSteamOpenId2Profile::AccountNamespace || !IsSteamId( claimed.GetSubject() ) )
        return false;
    if ( proof.kind != AUTH::CAuthProof::KindPlatformTicket )
        return false;

    CORE::CDynamicBuffer ticket;
    if ( !IsHexTicket( proof.data.Reveal(), SteamMaxTicketSize ) || !AUTH::CAuthCrypto::FromHex( proof.data.Reveal(), ticket ) )
        return false;

    MT::CScopeMutex lock( m_lock );
    CORE::CString reason;
    if ( !EnsureReady( reason ) || GUCEF_NULL == m_shim.beginAuthSession || GUCEF_NULL == m_shim.endAuthSession )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: A peer ticket cannot be checked here, the Steam client is not available" );
        return false;
    }

    UInt64 steamId = CORE::StringToUInt64( claimed.GetSubject(), 0 );
    Int32 begin = m_shim.beginAuthSession( m_shim.context, ticket.GetConstBufferPtr(), (Int32) ticket.GetDataSize(), steamId );
    if ( SteamBeginAuthSessionOk != begin )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SteamAuthProvider: The peer ticket was refused (EBeginAuthSessionResult " << begin << ")" );
        return false;
    }
    // The session is only opened to check the ticket
    m_shim.endAuthSession( m_shim.context, steamId );

    result = AUTH::CAuthResult();
    result.providerName = ProviderName;
    result.accountRef = claimed;
    result.proof = proof;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    // The assertion of the browser login is for the engine of the module
    if ( proof.kind == AUTH::CAuthProof::KindOpenId2Assertion )
        return AUTH::CDescriptorAuthProvider::Verify( proof, result );

    if ( proof.kind != AUTH::CAuthProof::KindPlatformTicket || proof.data.IsEmpty() )
        return false;

    CORE::CString mode;
    {
        MT::CScopeMutex lock( m_lock );
        mode = m_steamSettings.verifyMode;
    }
    if ( mode == "peer" )
    {
        // The account the peer claims is the one the caller put in the result
        AUTH::CAccountRef claimed = result.accountRef;
        return VerifyPeerTicket( claimed, proof, result );
    }
    return VerifyWithWebApi( proof, result );
}

/*-------------------------------------------------------------------------*/

CSteamAuthSettings
CSteamAuthProvider::GetSteamSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_steamSettings;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool saved = AUTH::CDescriptorAuthProvider::SaveConfig( config );

    // The keys of the Steam client and the Web API go in the Parameters that the settings of the provider have already
    CORE::CDataNode steamNode( "Steam", GUCEF_DATATYPE_OBJECT );
    {
        MT::CScopeMutex lock( m_lock );
        saved = m_steamSettings.SaveConfig( steamNode ) && saved;
    }
    const CORE::CDataNode* steamParameters = steamNode.FindChild( "Parameters" );
    CORE::CDataNode* parameters = config.FindOrAddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != steamParameters && GUCEF_NULL != parameters )
    {
        CORE::CDataNode::TAttributeMap::const_iterator i = steamParameters->AttributeBegin();
        while ( i != steamParameters->AttributeEnd() )
        {
            parameters->SetAttribute( (*i).first, (*i).second );
            ++i;
        }
    }
    return saved;
}

/*-------------------------------------------------------------------------*/

bool
CSteamAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    // The settings of the provider (the relay of the browser login among them), and the Steam keys in their Parameters
    bool loaded = AUTH::CDescriptorAuthProvider::LoadConfig( config );

    MT::CScopeMutex lock( m_lock );
    CSteamAuthSettings previous( m_steamSettings );
    if ( !m_steamSettings.LoadConfig( config ) )
        return false;

    // Another library or another identity is another attempt
    if ( previous.runtimeLibrary != m_steamSettings.runtimeLibrary )
        m_hasFailed = false;
    return loaded;
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamAuthProvider::GetIssuedTicketCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_issued.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamAuthProvider::GetPendingTicketCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
