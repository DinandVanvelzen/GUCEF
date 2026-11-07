/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#ifndef GUCEF_COM_CMAILTCPTRANSPORT_H
#include "gucefCOM_CMailTcpTransport.h"
#define GUCEF_COM_CMAILTCPTRANSPORT_H
#endif /* GUCEF_COM_CMAILTCPTRANSPORT_H ? */

#include "gucefCOM_CMailClientBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMailClientBase::ReadyEvent = "GUCEF::COM::CMailClientBase::ReadyEvent";
const CORE::CEvent CMailClientBase::ErrorEvent = "GUCEF::COM::CMailClientBase::ErrorEvent";
const CORE::CEvent CMailClientBase::DisconnectedEvent = "GUCEF::COM::CMailClientBase::DisconnectedEvent";

static const UInt32 TimerIntervalInMs = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailErrorEventData::CMailErrorEventData( void )
    : CORE::CICloneable()
    , errorClass( MAILERR_NONE )
    , description()
    , serverCode( 0 )
    , enhancedCode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailErrorEventData::CMailErrorEventData( EMailErrorClass errorClassOfError, const CORE::CString& descriptionOfError )
    : CORE::CICloneable()
    , errorClass( errorClassOfError )
    , description( descriptionOfError )
    , serverCode( 0 )
    , enhancedCode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailErrorEventData::CMailErrorEventData( const CMailErrorEventData& src )
    : CORE::CICloneable( src )
    , errorClass( src.errorClass )
    , description( src.description )
    , serverCode( src.serverCode )
    , enhancedCode( src.enhancedCode )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailErrorEventData::~CMailErrorEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailErrorEventData&
CMailErrorEventData::operator=( const CMailErrorEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        errorClass = src.errorClass;
        description = src.description;
        serverCode = src.serverCode;
        enhancedCode = src.enhancedCode;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailErrorEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailErrorEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailErrorEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailErrorEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMailErrorEventData";
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::RegisterEvents( void )
{GUCEF_TRACE;

    ReadyEvent.Initialize();
    ErrorEvent.Initialize();
    DisconnectedEvent.Initialize();
    CIMailTransport::RegisterEvents();
    CIMailCredentialProvider::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMailClientBase::CMailClientBase( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_transport( GUCEF_NULL )
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_settings( GUCEF_NULL )
    , m_log()
    , m_externalProvider( GUCEF_NULL )
    , m_builtInProvider( GUCEF_NULL )
    , m_state( CLIENTSTATE_DISCONNECTED )
    , m_lastError()
    , m_nextRequestId( 0 )
    , m_deadlineAt( 0 )
    , m_deadlineWindow( 0 )
    , m_deadlineWhat()
    , m_closing( false )
    , m_failureReported( false )
    , m_sasl( GUCEF_NULL )
    , m_saslServerMechanisms()
    , m_saslAllowInitialResponse( false )
    , m_saslAwaitingCredential( false )
    , m_saslFailedLocally( false )
    , m_saslRetried( false )
    , m_saslDeferredInitial()
    , m_saslHasDeferredInitial( false )
    , m_credentialRequest()
{GUCEF_TRACE;

    RegisterEvents();

    TEventCallback timerCallback( this, &CMailClientBase::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CMailClientBase::~CMailClientBase()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    ResetSasl();

    if ( GUCEF_NULL != m_transport )
    {
        m_transport->Disconnect();
        m_transport->ScheduleForDestruction();
        m_transport = GUCEF_NULL;
    }
    GUCEF_DELETE m_builtInProvider;
    m_builtInProvider = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CMailClientBase::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

UInt64
CMailClientBase::GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CMailClientBase::CreateLocalPulseGenerator( void )
{GUCEF_TRACE;

    return CORE::CPulseGenerator::CreateSharedObj();
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::RunPulseUntil( CORE::PulseGeneratorPtr pulseGenerator, const bool& flag, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const UInt64 start = GetTimeInMs();
    while ( !flag && GetTimeInMs() - start < timeoutInMs )
    {
        pulseGenerator->ForceDirectPulse();
        if ( !flag )
            pulseGenerator->WaitTillNextPulseWindow( 1, 10 );
    }
    return flag;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SetTransport( CIMailTransport* transport )
{GUCEF_TRACE;

    if ( transport == m_transport )
        return;
    if ( GUCEF_NULL != m_transport )
    {
        UnsubscribeFrom( m_transport );
        m_transport->Disconnect();
        m_transport->ScheduleForDestruction();
    }
    m_transport = transport;
    if ( GUCEF_NULL != m_transport )
        SubscribeToTransport();
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SubscribeToTransport( void )
{GUCEF_TRACE;

    TEventCallback connected( this, &CMailClientBase::OnTransportConnected );
    SubscribeTo( m_transport, CIMailTransport::ConnectedEvent, connected );
    TEventCallback tls( this, &CMailClientBase::OnTransportTlsEstablished );
    SubscribeTo( m_transport, CIMailTransport::TlsEstablishedEvent, tls );
    TEventCallback line( this, &CMailClientBase::OnTransportLine );
    SubscribeTo( m_transport, CIMailTransport::LineReceivedEvent, line );
    TEventCallback bytes( this, &CMailClientBase::OnTransportBytes );
    SubscribeTo( m_transport, CIMailTransport::BytesReceivedEvent, bytes );
    TEventCallback disconnected( this, &CMailClientBase::OnTransportDisconnected );
    SubscribeTo( m_transport, CIMailTransport::DisconnectedEvent, disconnected );
    TEventCallback error( this, &CMailClientBase::OnTransportError );
    SubscribeTo( m_transport, CIMailTransport::TransportErrorEvent, error );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SetCredentialProvider( CIMailCredentialProvider* provider )
{GUCEF_TRACE;

    if ( provider == m_externalProvider )
        return;
    if ( GUCEF_NULL != m_externalProvider )
        UnsubscribeFrom( m_externalProvider );
    m_externalProvider = provider;
    if ( GUCEF_NULL != m_externalProvider )
    {
        TEventCallback ready( this, &CMailClientBase::OnCredentialReady );
        SubscribeTo( m_externalProvider, CIMailCredentialProvider::CredentialReadyEvent, ready );
    }
}

/*-------------------------------------------------------------------------*/

CMailClientBase::EClientState
CMailClientBase::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::IsReady( void ) const
{GUCEF_TRACE;

    return CLIENTSTATE_READY == m_state;
}

/*-------------------------------------------------------------------------*/

const CMailErrorEventData&
CMailClientBase::GetLastError( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

UInt32
CMailClientBase::NextRequestId( void )
{GUCEF_TRACE;

    if ( 0 == ++m_nextRequestId )
        ++m_nextRequestId;
    return m_nextRequestId;
}

/*-------------------------------------------------------------------------*/

CIMailTransport&
CMailClientBase::GetTransport( void )
{GUCEF_TRACE;

    return *m_transport;
}

/*-------------------------------------------------------------------------*/

const CMailProtocolLog&
CMailClientBase::GetLog( void ) const
{GUCEF_TRACE;

    return m_log;
}

/*-------------------------------------------------------------------------*/

const CMailClientSettings&
CMailClientBase::GetBaseSettings( void ) const
{GUCEF_TRACE;

    return *m_settings;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SetState( EClientState state )
{GUCEF_TRACE;

    m_state = state;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SetReady( void )
{GUCEF_TRACE;

    ClearDeadline();
    SetState( CLIENTSTATE_READY );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << m_settings->host << ": ready" );
    NotifyObservers( ReadyEvent );
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::BeginConnect( const CMailClientSettings& settings )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED != m_state )
        return false;
    if ( settings.host.IsNULLOrEmpty() )
    {
        m_lastError = CMailErrorEventData( MAILERR_LOCAL, "No host to connect to" );
        return false;
    }

    m_settings = &settings;
    m_log.SetContext( GetProtocolName(), settings.host );
    m_closing = false;
    m_failureReported = false;
    m_lastError = CMailErrorEventData();
    ResetSasl();
    m_saslRetried = false;

    if ( GUCEF_NULL == m_transport )
        SetTransport( GUCEF_NEW CMailTcpTransport( m_pulseGenerator ) );
    m_transport->SetLimits( settings.maxLineLength, settings.maxLiteralSize );

    // the user name and password of the settings are the credential unless a provider was given
    if ( GUCEF_NULL == m_externalProvider )
    {
        if ( GUCEF_NULL == m_builtInProvider )
            m_builtInProvider = GUCEF_NEW CStaticMailCredentialProvider();
        m_builtInProvider->SetCredential( settings.passwordIsToken ? MAILCRED_BEARER_TOKEN : MAILCRED_PASSWORD, settings.userName, settings.password );
    }

    if ( MAILTLS_NONE == settings.tlsMode && !settings.allowInsecureAuth && WantsLogin() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << settings.host << ": no TLS and no permission to send credentials in the clear: logging in will fail" );
    }

    SetState( CLIENTSTATE_CONNECTING );
    SetDeadline( settings.connectTimeoutInMs, "the server to be ready" );
    m_timer.SetEnabled( true );

    const bool started = MAILTLS_IMPLICIT == settings.tlsMode ?
        m_transport->Connect( settings.host, settings.GetEffectivePort(), &settings.tls ) :
        m_transport->Connect( settings.host, settings.GetEffectivePort(), GUCEF_NULL );
    if ( !started )
    {
        Fail( MAILERR_CONNECTION, "The connection could not be started" );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << settings.host << ": connecting to port " << settings.GetEffectivePort() << " with TLS mode " << CMailClientSettings::TlsModeToString( settings.tlsMode ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::Disconnect( void )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state || GUCEF_NULL == m_transport )
        return;

    m_closing = true;
    if ( CLIENTSTATE_READY == m_state && m_transport->IsConnected() )
        SendQuitCommand();
    ClearDeadline();
    m_transport->Disconnect();

    // a transport that is not connected any more raises no event
    if ( CLIENTSTATE_DISCONNECTED != m_state && !m_transport->IsConnected() )
    {
        SetState( CLIENTSTATE_DISCONNECTED );
        m_timer.SetEnabled( false );
        OnClientClosed();
        NotifyObservers( DisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::SendCommandLine( const CORE::CString& line, bool secret )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_transport )
        return false;
    m_log.LogClientLine( line, secret );
    return m_transport->SendLine( line );
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::SendDataBlock( const void* data, UInt32 size, const char* what )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_transport )
        return false;
    m_log.LogData( true, what, size );
    return m_transport->SendData( data, size );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::SetDeadline( UInt32 timeoutInMs, const char* what )
{GUCEF_TRACE;

    if ( 0 == timeoutInMs )
    {
        ClearDeadline();
        return;
    }
    m_deadlineWindow = timeoutInMs;
    m_deadlineAt = GetTimeInMs() + timeoutInMs;
    m_deadlineWhat = what;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::RearmDeadline( void )
{GUCEF_TRACE;

    // a server that is sending is not silent: the wait starts again with what arrives
    if ( 0 != m_deadlineAt )
        m_deadlineAt = GetTimeInMs() + m_deadlineWindow;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::IsServerLineLoggable( const CORE::CString& line ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::ClearDeadline( void )
{GUCEF_TRACE;

    m_deadlineAt = 0;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTimer( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state )
        return;
    const UInt64 now = GetTimeInMs();
    if ( 0 != m_deadlineAt && now >= m_deadlineAt )
    {
        const CORE::CString waitedFor = m_deadlineWhat;
        m_deadlineAt = 0;
        Fail( MAILERR_TIMEOUT, "Timed out waiting for " + waitedFor );
        return;
    }
    OnTimerTick( now );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTimerTick( UInt64 nowInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnServerBytes( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnConnectionEstablished( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnClientClosed( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::Fail( EMailErrorClass errorClass, const CORE::CString& description, UInt32 serverCode, const CORE::CString& enhancedCode )
{GUCEF_TRACE;

    if ( m_failureReported || CLIENTSTATE_DISCONNECTED == m_state )
        return;
    m_failureReported = true;

    m_lastError = CMailErrorEventData( errorClass, description );
    m_lastError.serverCode = serverCode;
    m_lastError.enhancedCode = enhancedCode;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << ( GUCEF_NULL != m_settings ? m_settings->host : CORE::CString::Empty ) << ": " << description );

    ClearDeadline();
    m_timer.SetEnabled( false );
    ResetSasl();
    SetState( CLIENTSTATE_DISCONNECTED );
    OnClientFailed( m_lastError );

    m_closing = true;
    if ( GUCEF_NULL != m_transport )
        m_transport->Disconnect();

    CMailErrorEventData eData( m_lastError );
    if ( !NotifyObservers( ErrorEvent, &eData ) )
        return;
    NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportConnected( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_CONNECTING == m_state )
        OnConnectionEstablished();
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportTlsEstablished( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state )
        return;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << m_settings->host << ": TLS established" );
    OnTlsEstablished();
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportLine( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state || GUCEF_NULL == eventData )
        return;
    const CORE::CString& line = static_cast< CIMailTransport::TLineReceivedEventData* >( eventData )->GetData();
    if ( IsServerLineLoggable( line ) )
        m_log.LogServerLine( line, IsSaslActive() );
    RearmDeadline();
    OnServerLine( line );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportBytes( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state || GUCEF_NULL == eventData )
        return;
    RearmDeadline();
    OnServerBytes( static_cast< CIMailTransport::TBytesReceivedEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportDisconnected( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state )
        return;
    if ( m_closing )
    {
        ClearDeadline();
        m_timer.SetEnabled( false );
        ResetSasl();
        SetState( CLIENTSTATE_DISCONNECTED );
        OnClientClosed();
        NotifyObservers( DisconnectedEvent );
        return;
    }
    Fail( MAILERR_CONNECTION, "The server closed the connection" );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnTransportError( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == m_state || GUCEF_NULL == eventData )
        return;
    const CMailTransportErrorData* error = CORE::CICloneable::CloneableAs< CMailTransportErrorData >( eventData );
    if ( GUCEF_NULL != error )
        Fail( error->errorClass, error->description );
    else
        Fail( MAILERR_CONNECTION, "The connection failed" );
}

/*-------------------------------------------------------------------------*/

CMailClientBase::ETlsAction
CMailClientBase::DecideTls( bool serverOffersStartTls )
{GUCEF_TRACE;

    if ( m_transport->IsSecure() )
        return TLSACTION_NONE;
    switch ( m_settings->tlsMode )
    {
        case MAILTLS_STARTTLS:
        {
            if ( serverOffersStartTls )
                return TLSACTION_STARTTLS;
            Fail( MAILERR_TLS, "The server does not offer STARTTLS, which is required" );
            return TLSACTION_FAILED;
        }
        case MAILTLS_STARTTLS_IF_OFFERED:
        {
            return serverOffersStartTls ? TLSACTION_STARTTLS : TLSACTION_NONE;
        }
        default:
        {
            return TLSACTION_NONE;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::BeginTlsUpgrade( void )
{GUCEF_TRACE;

    // the transport reports the reason and closes when it cannot
    SetDeadline( m_settings->connectTimeoutInMs, "the TLS handshake" );
    return m_transport->StartTls( m_settings->tls, m_settings->host );
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::IsCredentialExposureAllowed( void ) const
{GUCEF_TRACE;

    return m_transport->IsSecure() || m_settings->allowInsecureAuth;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::WantsLogin( void ) const
{GUCEF_TRACE;

    return !m_settings->userName.IsNULLOrEmpty() || GUCEF_NULL != m_externalProvider;
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::MakeCredentialRequest( CMailCredentialRequest& request, bool previousAttemptFailed ) const
{GUCEF_TRACE;

    request.protocol = MAILPROTOCOL_SMTP;
    if ( GetProtocolName() == "POP3" ) request.protocol = MAILPROTOCOL_POP3;
    else if ( GetProtocolName() == "IMAP" ) request.protocol = MAILPROTOCOL_IMAP;
    request.host = m_settings->host;
    request.userName = m_settings->userName;
    request.previousAttemptFailed = previousAttemptFailed;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::GetCredentialNow( CMailCredential& credential, bool previousAttemptFailed )
{GUCEF_TRACE;

    CIMailCredentialProvider* provider = GUCEF_NULL != m_externalProvider ? m_externalProvider : m_builtInProvider;
    if ( GUCEF_NULL == provider )
        return false;
    CMailCredentialRequest request;
    MakeCredentialRequest( request, previousAttemptFailed );
    request.requestId = NextRequestId();
    return CIMailCredentialProvider::CRED_READY == provider->GetCredential( request, credential );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::ResetSasl( void )
{GUCEF_TRACE;

    GUCEF_DELETE m_sasl;
    m_sasl = GUCEF_NULL;
    m_saslAwaitingCredential = false;
    m_saslFailedLocally = false;
    m_saslHasDeferredInitial = false;
    m_saslDeferredInitial.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::IsSaslActive( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_sasl || m_saslAwaitingCredential;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::BeginSasl( const CORE::CString::StringVector& serverMechanisms, bool allowInitialResponse )
{GUCEF_TRACE;

    ResetSasl();
    SetState( CLIENTSTATE_AUTHENTICATING );
    m_saslServerMechanisms = serverMechanisms;
    for ( size_t i=0; i<m_saslServerMechanisms.size(); ++i )
        m_saslServerMechanisms[ i ] = m_saslServerMechanisms[ i ].Uppercase();
    m_saslAllowInitialResponse = allowInitialResponse;
    return StartSaslWithCredential();
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::StartSaslWithCredential( void )
{GUCEF_TRACE;

    CIMailCredentialProvider* provider = GUCEF_NULL != m_externalProvider ? m_externalProvider : m_builtInProvider;
    CMailCredentialRequest request;
    MakeCredentialRequest( request, m_saslRetried );
    request.requestId = NextRequestId();
    m_credentialRequest = request;

    CMailCredential credential;
    const CIMailCredentialProvider::EResult result = GUCEF_NULL != provider ? provider->GetCredential( request, credential ) : CIMailCredentialProvider::CRED_FAILED;
    if ( CIMailCredentialProvider::CRED_PENDING == result )
    {
        // the exchange starts when the provider says the credential is there
        m_saslAwaitingCredential = true;
        return true;
    }
    m_saslAwaitingCredential = false;
    if ( CIMailCredentialProvider::CRED_READY != result )
    {
        Fail( MAILERR_AUTH, "There is no credential to log in with" );
        return false;
    }

    // the mechanisms to use, for a password or for a token
    CORE::CString::StringVector preferred;
    m_settings->GetPreferredMechanisms( preferred );
    if ( MAILCRED_BEARER_TOKEN == credential.type )
    {
        preferred.clear();
        preferred.push_back( "XOAUTH2" );
    }
    else
    {
        // a token mechanism makes no sense for a password
        for ( size_t i=0; i<preferred.size(); )
        {
            if ( preferred[ i ].Equals( "XOAUTH2", false ) )
                preferred.erase( preferred.begin() + i );
            else
                ++i;
        }
    }

    CIMailSaslMechanism* chosen = GUCEF_NULL;
    bool skippedForTls = false;
    bool offered = false;
    for ( size_t p=0; p<preferred.size() && GUCEF_NULL == chosen; ++p )
    {
        const CORE::CString upper = preferred[ p ].Uppercase();
        bool serverHasIt = false;
        for ( size_t s=0; s<m_saslServerMechanisms.size(); ++s )
            serverHasIt = serverHasIt || m_saslServerMechanisms[ s ] == upper;
        if ( !serverHasIt )
            continue;
        offered = true;

        CIMailSaslMechanism* candidate = CMailSaslFactory::Create( upper, credential.userName, credential.secret );
        if ( GUCEF_NULL == candidate )
            continue;
        if ( candidate->RevealsSecret() && !IsCredentialExposureAllowed() )
        {
            skippedForTls = true;
            GUCEF_DELETE candidate;
            continue;
        }
        chosen = candidate;
    }

    if ( GUCEF_NULL == chosen )
    {
        if ( skippedForTls )
            Fail( MAILERR_TLS, "The credentials are not sent without TLS" );
        else
            Fail( MAILERR_AUTH, offered ? "No usable authentication mechanism" : "The server offers no authentication mechanism that is configured" );
        return false;
    }

    m_sasl = chosen;
    CORE::CDynamicBuffer initial;
    if ( !m_sasl->Start( initial ) )
    {
        const CORE::CString reason = m_sasl->GetFailureReason();
        ResetSasl();
        Fail( MAILERR_AUTH, "Authentication cannot start: " + reason );
        return false;
    }

    const bool hasInitial = m_sasl->HasInitialResponse();
    if ( hasInitial && !m_saslAllowInitialResponse )
    {
        // the first response waits for the empty challenge of the server
        m_saslDeferredInitial = initial;
        m_saslHasDeferredInitial = true;
        return SendSaslStart( m_sasl->GetName(), false, CORE::CString::Empty );
    }
    CORE::CString base64;
    if ( hasInitial && initial.GetDataSize() > 0 )
        base64 = CORE::Base64Encode( initial.GetConstBufferPtr(), initial.GetDataSize() );
    return SendSaslStart( m_sasl->GetName(), hasInitial, hasInitial && base64.IsNULLOrEmpty() ? CORE::CString( "=" ) : base64 );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnCredentialReady( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_saslAwaitingCredential || CLIENTSTATE_AUTHENTICATING != m_state )
        return;
    StartSaslWithCredential();
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnSaslChallenge( const CORE::CString& base64Challenge )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sasl )
    {
        Fail( MAILERR_PROTOCOL, "The server sent an authentication challenge that was not asked for" );
        return;
    }

    if ( m_saslHasDeferredInitial )
    {
        m_saslHasDeferredInitial = false;
        const CORE::CString response = m_saslDeferredInitial.GetDataSize() > 0 ? CORE::Base64Encode( m_saslDeferredInitial.GetConstBufferPtr(), m_saslDeferredInitial.GetDataSize() ) : CORE::CString();
        m_saslDeferredInitial.Clear();
        SendSaslResponse( response );
        return;
    }

    CORE::CDynamicBuffer challenge;
    if ( !base64Challenge.IsNULLOrEmpty() )
    {
        bool hadErrors = false;
        CMimeCodec::DecodeBase64Lenient( base64Challenge.C_String(), base64Challenge.ByteSize() - 1, challenge, 65536, &hadErrors );
    }

    CORE::CDynamicBuffer response;
    const EMailSaslState state = m_sasl->Step( challenge, response );
    const CORE::CString base64Response = response.GetDataSize() > 0 ? CORE::Base64Encode( response.GetConstBufferPtr(), response.GetDataSize() ) : CORE::CString();

    if ( MAILSASL_FAILED == state )
    {
        if ( m_sasl->GetName() == "XOAUTH2" )
        {
            // the protocol wants an empty answer to the error of the server, after which the server refuses
            m_saslFailedLocally = true;
            SendSaslResponse( CORE::CString::Empty );
            return;
        }
        const CORE::CString reason = m_sasl->GetFailureReason();
        SendSaslCancel();
        Fail( MAILERR_AUTH, "Authentication failed: " + reason );
        return;
    }
    SendSaslResponse( base64Response );
}

/*-------------------------------------------------------------------------*/

void
CMailClientBase::OnSaslSucceeded( void )
{GUCEF_TRACE;

    ResetSasl();
    m_saslRetried = false;
}

/*-------------------------------------------------------------------------*/

bool
CMailClientBase::OnSaslRejected( const CORE::CString& serverText, UInt32 serverCode )
{GUCEF_TRACE;

    const bool wasToken = GUCEF_NULL != m_sasl && m_sasl->GetName() == "XOAUTH2";
    ResetSasl();

    CIMailCredentialProvider* provider = GUCEF_NULL != m_externalProvider ? m_externalProvider : m_builtInProvider;
    if ( wasToken && !m_saslRetried && GUCEF_NULL != m_externalProvider && GUCEF_NULL != provider )
    {
        // the access token may have expired: the provider renews it and the exchange is tried once more
        m_saslRetried = true;
        provider->ReportRejected( m_credentialRequest );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, GetProtocolName() << " " << m_settings->host << ": the access token was refused, asking for a new one" );
        return StartSaslWithCredential();
    }
    if ( GUCEF_NULL != provider )
        provider->ReportRejected( m_credentialRequest );
    Fail( MAILERR_AUTH, "Authentication was refused: " + CMailProtocolLog::RedactServerLine( serverText, false ), serverCode );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
