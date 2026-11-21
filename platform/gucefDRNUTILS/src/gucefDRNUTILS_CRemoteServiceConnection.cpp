/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CRemoteServiceConnection.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CLONEABLES_H
#endif /* GUCEF_CLONEABLES_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteServiceConnection::StateChangedEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::StateChangedEvent";
const CORE::CEvent CRemoteServiceConnection::SessionEventEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::SessionEventEvent";
const CORE::CEvent CRemoteServiceConnection::ResyncEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::ResyncEvent";
const CORE::CEvent CRemoteServiceConnection::SessionReplacedEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::SessionReplacedEvent";
const CORE::CEvent CRemoteServiceConnection::CredentialsChangedEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::CredentialsChangedEvent";
const CORE::CEvent CRemoteServiceConnection::ClockSkewEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::ClockSkewEvent";
const CORE::CEvent CRemoteServiceConnection::ServiceStoppingEvent = "GUCEF::DRNUTILS::CRemoteServiceConnection::ServiceStoppingEvent";

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
    SessionEventEvent.Initialize();
    ResyncEvent.Initialize();
    SessionReplacedEvent.Initialize();
    CredentialsChangedEvent.Initialize();
    ClockSkewEvent.Initialize();
    ServiceStoppingEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteServiceConnection::SBulkState::SBulkState( void )
    : attachedTo()
    , pending( CORE::TASKSTATUS_UNDEFINED )
    , hasPending( false )
    , nextAttemptMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteServiceConnection::CRemoteServiceConnection( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRemoteServiceConnectionSettings& settings )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , m_lock()
    , m_pulse( notificationPulseGenerator )
    , m_settings( settings )
    , m_store( GUCEF_NULL )
    , m_storeSlot()
    , m_utcClock( GUCEF_NULL )
    , m_monotonicClock( GUCEF_NULL )
    , m_systemUtcClock()
    , m_systemMonotonicClock()
    , m_credentials()
    , m_state( REMOTECONNECTION_IDLE )
    , m_lastError()
    , m_timer( notificationPulseGenerator, settings.tickIntervalMs > 0 ? settings.tickIntervalMs : 10 )
    , m_channels()
    , m_bulk()
    , m_backoff()
    , m_pending( CORE::TASKSTATUS_UNDEFINED )
    , m_hasPending( false )
    , m_pendingMethod()
    , m_resync( CORE::TASKSTATUS_UNDEFINED )
    , m_hasResync( false )
    , m_events()
    , m_endedStreams()
    , m_sessionId()
    , m_resumeSecretHex()
    , m_deviceId()
    , m_roles()
    , m_hostInstanceId()
    , m_helloResponse()
    , m_lastEventSeq( 0 )
    , m_clockOffsetMs( 0 )
    , m_clockSkewNoticed( false )
    , m_stepStartedMs( 0 )
    , m_nextAttemptMs( 0 )
    , m_nextAttemptDelayMs( 0 )
    , m_stopHintMs( 0 )
    , m_stopRaised( false )
    , m_reconnecting( false )
    , m_attemptInProgress( false )
    , m_connectedOnce( false )
    , m_lastTickMs( 0 )
    , m_lastTickUtcMs( 0 )
    , m_rpcResumes( 0 )
    , m_sessionResumes( 0 )
    , m_resyncs( 0 )
    , m_sessionReplaces( 0 )
    , m_attempts( 0 )
    , m_wakeDetections( 0 )
    , m_duplicateEvents( 0 )
    , m_bulkAttaches( 0 )
    , m_inTick( false )
{GUCEF_TRACE;

    RegisterEvents();
    if ( m_settings.bulkChannels > 2 )
        m_settings.bulkChannels = 2;
    m_backoff.SetPolicy( m_settings.reconnectFirstMaxMs, m_settings.reconnectBaseMs, m_settings.reconnectMaxMs, m_settings.reconnectStableResetMs, 1000 );
    for ( UInt32 i=0; i<=m_settings.bulkChannels; ++i )
        m_channels.push_back( GUCEF_NEW CRemoteServiceChannel( m_pulse, i, this ) );
    m_bulk.resize( m_settings.bulkChannels + 1 );
    TEventCallback onTick( this, &CRemoteServiceConnection::OnTick );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTick );
}

/*-------------------------------------------------------------------------*/

CRemoteServiceConnection::~CRemoteServiceConnection()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_events.Unlink();
    m_endedStreams.clear();
    for ( UInt32 i=0; i<m_channels.size(); ++i )
    {
        m_channels[ i ]->Shutdown( false );
        GUCEF_DELETE m_channels[ i ];
    }
    m_channels.clear();
    m_resumeSecretHex.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteServiceConnection::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteServiceConnection";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::SetCredentialStore( AUTH::ICredentialStore* store, const CORE::CString& slot )
{GUCEF_TRACE;

    m_store = store;
    m_storeSlot = slot;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::SetClocks( CORE::CIUtcClock* utcClock, CORE::CIMonotonicClock* monotonicClock )
{GUCEF_TRACE;

    m_utcClock = utcClock;
    m_monotonicClock = monotonicClock;
    m_lastTickMs = 0;
    m_lastTickUtcMs = 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceConnection::NowMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_monotonicClock ? m_monotonicClock->GetMonotonicTimeInMs() : m_systemMonotonicClock.GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

Int64
CRemoteServiceConnection::NowUtcMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_utcClock ? m_utcClock->GetUtcTimeInMs() : m_systemUtcClock.GetUtcTimeInMs();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::SetState( TRemoteConnectionState state, const CORE::CString& reason )
{GUCEF_TRACE;

    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( REMOTECONNECTION_FAILED == state && !reason.IsNULLOrEmpty() )
            m_lastError = reason;
        changed = m_state != state;
        m_state = state;
    }
    m_stepStartedMs = NowMs();
    if ( !changed )
        return;
    if ( REMOTECONNECTION_CONNECTED == state )
        m_stopRaised = false;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: " << RemoteConnectionStateToString( state ) << ( reason.IsNULLOrEmpty() ? "" : ": " ) << reason );
    TRemoteConnectionStateEventData data( state );
    NotifyObservers( StateChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

TRemoteConnectionState CRemoteServiceConnection::GetState( void ) const { MT::CScopeMutex lock( m_lock ); return m_state; }
CORE::CString CRemoteServiceConnection::GetLastError( void ) const { MT::CScopeMutex lock( m_lock ); return m_lastError; }
CORE::CString CRemoteServiceConnection::GetSessionId( void ) const { MT::CScopeMutex lock( m_lock ); return m_sessionId; }
CORE::CString CRemoteServiceConnection::GetDeviceId( void ) const { MT::CScopeMutex lock( m_lock ); return m_deviceId; }
CORE::CString::StringSet CRemoteServiceConnection::GetRoles( void ) const { MT::CScopeMutex lock( m_lock ); return m_roles; }
CORE::CString CRemoteServiceConnection::GetHostInstanceId( void ) const { MT::CScopeMutex lock( m_lock ); return m_hostInstanceId; }
COMCORE::CRpcMessage CRemoteServiceConnection::GetHelloResponse( void ) const { MT::CScopeMutex lock( m_lock ); return m_helloResponse; }
UInt64 CRemoteServiceConnection::GetLastEventSeq( void ) const { MT::CScopeMutex lock( m_lock ); return m_lastEventSeq; }
Int64 CRemoteServiceConnection::GetClockOffsetMs( void ) const { MT::CScopeMutex lock( m_lock ); return m_clockOffsetMs; }
UInt32 CRemoteServiceConnection::GetRpcResumeCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_rpcResumes; }
UInt32 CRemoteServiceConnection::GetSessionResumeCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_sessionResumes; }
UInt32 CRemoteServiceConnection::GetResyncCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_resyncs; }
UInt32 CRemoteServiceConnection::GetSessionReplaceCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_sessionReplaces; }
UInt32 CRemoteServiceConnection::GetReconnectAttemptCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_attempts; }
UInt32 CRemoteServiceConnection::GetWakeDetectionCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_wakeDetections; }
UInt32 CRemoteServiceConnection::GetDuplicateEventCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_duplicateEvents; }
UInt32 CRemoteServiceConnection::GetBulkAttachCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_bulkAttaches; }
UInt32 CRemoteServiceConnection::GetNextAttemptDelayMs( void ) const { MT::CScopeMutex lock( m_lock ); return m_nextAttemptDelayMs; }
CDeviceCredentials CRemoteServiceConnection::GetCredentials( void ) const { MT::CScopeMutex lock( m_lock ); return m_credentials; }

/*-------------------------------------------------------------------------*/

CRemoteServiceChannel*
CRemoteServiceConnection::GetChannel( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_channels.size() ? m_channels[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::IsBulkChannelReady( UInt32 index ) const
{GUCEF_TRACE;

    if ( 0 == index || index >= m_channels.size() )
        return false;
    MT::CScopeMutex lock( m_lock );
    return !m_sessionId.IsNULLOrEmpty() && m_bulk[ index ].attachedTo == m_sessionId &&
           CRemoteServiceChannel::CHANNEL_READY == m_channels[ index ]->GetState();
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcEndpointPtr
CRemoteServiceConnection::GetEndpoint( UInt32 channel ) const
{GUCEF_TRACE;

    CRemoteServiceChannel* found = GetChannel( channel );
    return GUCEF_NULL != found ? found->GetEndpoint() : COMCORE::CRpcEndpointPtr();
}

/*-------------------------------------------------------------------------*/

COMCORE::CIRpcTransportPtr
CRemoteServiceConnection::WrapTransport( UInt32 channel, const COMCORE::CIRpcTransportPtr& transport )
{GUCEF_TRACE;

    return transport;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::IsPumpingThread( void ) const
{GUCEF_TRACE;

    TThreadId driver = m_pulse->GetPulseDriverThreadId();
    return 0 == driver || driver == MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::WaitStep( void )
{GUCEF_TRACE;

    if ( IsPumpingThread() )
        m_pulse->ForceDirectPulse();
    MT::PrecisionDelay( 1 );
}

/*-------------------------------------------------------------------------*/

CRemoteServiceChannel::SSettings
CRemoteServiceConnection::MakeChannelSettings( UInt32 channel ) const
{GUCEF_TRACE;

    CRemoteServiceChannel::SSettings settings;
    settings.linkTimeoutMs = m_settings.linkTimeoutMs;
    settings.echoIntervalMs = m_settings.echoIntervalMs;
    settings.resumeGraceMs = m_settings.resumeGraceMs;
    if ( 0 == channel )
    {
        settings.nodeName = m_settings.clientKind;
        settings.receiveWindowBytes = m_settings.controlWindowBytes;
        settings.maxMessageBytes = m_settings.controlMaxMessageBytes;
        settings.sendQueueBytes = m_settings.controlSendQueueBytes;
        settings.wantedContracts = m_settings.wantedContracts;
    }
    else
    {
        settings.nodeName = m_settings.clientKind + ".bulk" + CORE::ToString( channel );
        settings.receiveWindowBytes = m_settings.bulkWindowBytes;
        settings.maxMessageBytes = m_settings.bulkMaxMessageBytes;
        settings.sendQueueBytes = m_settings.bulkSendQueueBytes;
        settings.wantedContracts = m_settings.bulkContracts;
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::StartConnect( const CDeviceCredentials& credentials, CORE::CString& error )
{GUCEF_TRACE;

    if ( !credentials.IsComplete() )
    {
        error = "the credentials of the device are not complete (pair again)";
        return false;
    }
    if ( CRemoteSessionContracts::GetContract().IsNULL() )
    {
        error = "the contract gucef.drnutils.session cannot be built";
        return false;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_credentials = credentials;
        m_lastError.Clear();
        m_deviceId = credentials.deviceId;
    }
    m_reconnecting = false;
    m_backoff.Reset();
    m_lastTickMs = 0;
    m_lastTickUtcMs = 0;
    if ( !BeginJoin( error ) )
    {
        // the first attempt could not even start (no endpoint, no codec): nothing to retry
        SetState( REMOTECONNECTION_FAILED, error );
        return false;
    }
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::FindLanCandidates( const CDeviceCredentials& credentials, COMCORE::CServiceFinder::TServiceCandidateVector& candidates )
{GUCEF_TRACE;

    if ( !m_settings.lanDiscovery )
        return false;

    // the discovery clients are registered by gucefCOM
    COM::CComGlobal::Instance();

    // only the paired service: a candidate with another key would be refused at the handshake anyway
    COMCORE::CServiceFinderSettings finderSettings( m_settings.lanDiscoveryFinder );
    if ( 64 == credentials.serverStaticKeyHex.Length() )
        finderSettings.filter.keyFingerprint = CDevicePairingCrypto::FingerprintOfHex( credentials.serverStaticKeyHex );
    if ( !credentials.serviceId.IsNULLOrEmpty() )
        finderSettings.filter.instanceId = credentials.serviceId;

    if ( !IsPumpingThread() )
    {
        COMCORE::CServiceFinder::FindOnce( finderSettings, candidates );
        return !candidates.empty();
    }

    // on the thread of the notification pulse generator: the look runs on it, as the rest of the connection does
    COMCORE::CServiceFinder finder( m_pulse, finderSettings );
    if ( !finder.Start() )
        return false;
    const UInt64 startedAt = NowMs();
    const UInt32 wanted = finderSettings.findMinCandidates;
    while ( NowMs() - startedAt < finderSettings.findTimeoutMs && ( 0 == wanted || finder.GetCandidateCount() < wanted ) )
        WaitStep();
    finder.GetCandidates( candidates );
    finder.Stop();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: " << candidates.size() << " candidates of " << credentials.serviceName << " on the local network" );
    return !candidates.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteServiceConnection::AddCandidateEndpoints( CDeviceCredentials& credentials, const COMCORE::CServiceFinder::TServiceCandidateVector& candidates )
{GUCEF_TRACE;

    UInt32 added = 0;
    for ( UInt32 c=0; c<candidates.size(); ++c )
    {
        const CORE::CString::StringVector& endpoints = candidates[ c ].announcement.endpoints;
        for ( UInt32 e=0; e<endpoints.size(); ++e )
        {
            bool known = false;
            for ( UInt32 k=0; k<credentials.sessionEndpoints.size() && !known; ++k )
                known = credentials.sessionEndpoints[ k ] == endpoints[ e ];
            if ( known || !endpoints[ e ].StartsWith( "tcp://" ) )
                continue;
            credentials.sessionEndpoints.push_back( endpoints[ e ] );
            ++added;
        }
    }
    return added;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::Connect( const CDeviceCredentials& credentials, CORE::CString& error )
{GUCEF_TRACE;

    // no endpoint configured: the local network may know where the paired service is; the configured endpoints win
    if ( credentials.sessionEndpoints.empty() && m_settings.hostOverride.IsNULLOrEmpty() && m_settings.lanDiscovery )
    {
        CDeviceCredentials found( credentials );
        COMCORE::CServiceFinder::TServiceCandidateVector candidates;
        if ( FindLanCandidates( credentials, candidates ) && AddCandidateEndpoints( found, candidates ) > 0 )
            return Connect( found, error );
    }

    if ( !StartConnect( credentials, error ) )
        return false;
    if ( WaitUntilConnected( m_settings.connectTimeoutMs ) )
        return true;
    error = GetLastError();
    if ( error.IsNULLOrEmpty() )
        error = "the service did not answer within " + CORE::ToString( m_settings.connectTimeoutMs ) + " ms (state " + RemoteConnectionStateToString( GetState() ) + ")";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::WaitUntilConnected( UInt32 timeoutMs )
{GUCEF_TRACE;

    const UInt64 startedAt = NowMs();
    while ( NowMs() - startedAt < timeoutMs )
    {
        TRemoteConnectionState state = GetState();
        if ( REMOTECONNECTION_CONNECTED == state )
            return true;
        if ( REMOTECONNECTION_FAILED == state || REMOTECONNECTION_IDLE == state )
            return false;
        WaitStep();
    }
    return REMOTECONNECTION_CONNECTED == GetState();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::BeginJoin( CORE::CString& error )
{GUCEF_TRACE;

    CDeviceCredentials credentials = GetCredentials();
    {
        MT::CScopeMutex lock( m_lock );
        ++m_attempts;
    }
    if ( !m_channels[ 0 ]->StartJoin( credentials, MakeChannelSettings( 0 ), m_settings.hostOverride, error ) )
        return false;
    SetState( m_reconnecting ? REMOTECONNECTION_RECONNECTING : REMOTECONNECTION_JOINING, CORE::CString::Empty );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StartReconnect( const CORE::CString& why )
{GUCEF_TRACE;

    if ( !m_events.IsNULL() )
    {
        // a stream of an RPC session that went is opened again once the session is back
        m_endedStreams.push_back( m_events );
        UnsubscribeFrom( m_events.GetPointerAlways() );
        m_events.Unlink();
    }
    m_attemptInProgress = false;
    m_hasPending = false;
    if ( !m_settings.autoReconnect )
    {
        SetState( REMOTECONNECTION_FAILED, "the link to the service dropped: " + why );
        return;
    }
    m_reconnecting = true;
    UInt32 delay = m_backoff.NextDelayMs( m_stopHintMs );
    m_stopHintMs = 0;
    m_nextAttemptMs = NowMs() + delay;
    {
        MT::CScopeMutex lock( m_lock );
        m_nextAttemptDelayMs = delay;
    }
    SetState( REMOTECONNECTION_RECONNECTING, why + " (next attempt in " + CORE::ToString( delay ) + " ms)" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::FailPermanently( const CORE::CString& why )
{GUCEF_TRACE;

    m_hasPending = false;
    for ( UInt32 i=0; i<m_channels.size(); ++i )
        m_channels[ i ]->Shutdown( false );
    SetState( REMOTECONNECTION_FAILED, why );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::CheckWake( void )
{GUCEF_TRACE;

    const UInt64 nowMs = NowMs();
    const Int64 nowUtcMs = NowUtcMs();
    if ( 0 != m_lastTickMs && 0 != m_settings.wakeDetectMs )
    {
        const Int64 wall = nowUtcMs - m_lastTickUtcMs;
        const Int64 mono = (Int64) ( nowMs - m_lastTickMs );
        if ( wall - mono > (Int64) m_settings.wakeDetectMs )
        {
            // the machine slept: the monotonic clock stood still, the links are almost certainly dead
            {
                MT::CScopeMutex lock( m_lock );
                ++m_wakeDetections;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the wall clock moved " << wall << " ms in " << mono << " ms: the machine slept, reconnecting now" );
            TRemoteConnectionState state = GetState();
            if ( REMOTECONNECTION_IDLE != state && REMOTECONNECTION_FAILED != state )
            {
                for ( UInt32 i=0; i<m_channels.size(); ++i )
                    m_channels[ i ]->DropLink();
                m_backoff.Reset();
                m_nextAttemptMs = nowMs;
            }
        }
    }
    m_lastTickMs = nowMs;
    m_lastTickUtcMs = nowUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::OnTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_inTick )
        return;
    m_inTick = true;
    for ( UInt32 i=0; i<m_endedStreams.size(); ++i )
        UnsubscribeFrom( m_endedStreams[ i ].GetPointerAlways() );
    m_endedStreams.clear();
    CheckWake();
    for ( UInt32 i=0; i<m_channels.size(); ++i )
        m_channels[ i ]->Step();
    switch ( GetState() )
    {
        case REMOTECONNECTION_JOINING:
        case REMOTECONNECTION_BINDING: StepJoining(); break;
        case REMOTECONNECTION_OPENING: StepOpening(); break;
        case REMOTECONNECTION_CONNECTED: StepConnected(); break;
        case REMOTECONNECTION_RECONNECTING: StepReconnecting(); break;
        default: break;
    }
    m_inTick = false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StepJoining( void )
{GUCEF_TRACE;

    CRemoteServiceChannel& control = *m_channels[ 0 ];
    switch ( control.GetState() )
    {
        case CRemoteServiceChannel::CHANNEL_REFUSED:
        {
            FailPermanently( "the service refused the link: " + control.GetLastLinkText() );
            return;
        }
        case CRemoteServiceChannel::CHANNEL_DOWN:
        {
            CORE::CString why = control.GetLastLinkText();
            StartReconnect( why.IsNULLOrEmpty() ? CORE::CString( "the service did not answer the link" ) : why );
            return;
        }
        case CRemoteServiceChannel::CHANNEL_BINDING:
        {
            if ( REMOTECONNECTION_RECONNECTING != GetState() && REMOTECONNECTION_BINDING != GetState() )
                SetState( REMOTECONNECTION_BINDING, CORE::CString::Empty );
            return;
        }
        case CRemoteServiceChannel::CHANNEL_READY:
        {
            break;
        }
        default:
        {
            if ( NowMs() - m_stepStartedMs > m_settings.connectTimeoutMs )
                StartReconnect( "the link did not come up in time" );
            return;
        }
    }

    // Case A only when there is a client session to go on with: an RPC session can be resumed without one (the Hello of
    // the attempt before was refused, as during a drain), and then the client session is opened on it (Hello)
    if ( control.WasRpcResumed() && !control.HasNewRpcSession() && !GetSessionId().IsNULLOrEmpty() )
    {
        // case A: calls, streams and the event stream go on where they were
        {
            MT::CScopeMutex lock( m_lock );
            ++m_rpcResumes;
        }
        m_reconnecting = false;
        m_backoff.OnConnected( NowMs() );
        if ( m_events.IsNULL() && m_settings.openEventStream && !m_sessionId.IsNULLOrEmpty() )
            OpenEvents();
        SetState( REMOTECONNECTION_CONNECTED, "the RPC session was resumed" );
        return;
    }
    control.ClearNewRpcSession();
    SendHelloOrResume();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StepReconnecting( void )
{GUCEF_TRACE;

    CRemoteServiceChannel& control = *m_channels[ 0 ];
    CRemoteServiceChannel::TChannelState channelState = control.GetState();
    if ( CRemoteServiceChannel::CHANNEL_REFUSED == channelState )
    {
        FailPermanently( "the service refused the link: " + control.GetLastLinkText() );
        return;
    }
    if ( CRemoteServiceChannel::CHANNEL_JOINING == channelState || CRemoteServiceChannel::CHANNEL_BINDING == channelState ||
         CRemoteServiceChannel::CHANNEL_READY == channelState )
    {
        StepJoining();
        return;
    }
    if ( m_attemptInProgress )
    {
        // the attempt failed (the link did not come up): the next one waits for the backoff, never a tight loop
        m_attemptInProgress = false;
        UInt32 delay = m_backoff.NextDelayMs();
        m_nextAttemptMs = NowMs() + delay;
        {
            MT::CScopeMutex lock( m_lock );
            m_nextAttemptDelayMs = delay;
        }
        return;
    }
    if ( NowMs() < m_nextAttemptMs )
        return;
    CORE::CString error;
    m_attemptInProgress = true;
    if ( !BeginJoin( error ) )
    {
        m_attemptInProgress = false;
        UInt32 delay = m_backoff.NextDelayMs();
        m_nextAttemptMs = NowMs() + delay;
        {
            MT::CScopeMutex lock( m_lock );
            m_nextAttemptDelayMs = delay;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: a reconnect attempt failed (" << error << "), next in " << delay << " ms" );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::SendHelloOrResume( void )
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( 0 );
    if ( endpoint.IsNULL() )
        return;
    COMCORE::CRpcCallOptions options;
    options.waitForReady = false;
    options.deadlineMs = 15000;
    CORE::CString sessionId = GetSessionId();
    if ( sessionId.IsNULLOrEmpty() )
    {
        COMCORE::CRpcMessage hello = NewRequest( CRemoteSessionContracts::HelloPath );
        hello.SetString( 1, m_settings.clientKind );
        hello.SetString( 2, m_settings.clientVersion );
        for ( UInt32 i=0; i<m_settings.capabilities.size(); ++i )
            hello.Add( 3, CORE::CVariant( m_settings.capabilities[ i ] ) );
        hello.SetUInt64( 4, (UInt64) NowUtcMs() );
        m_pending = endpoint->CallAsync( CRemoteSessionContracts::HelloPath, hello, options );
        m_pendingMethod = CRemoteSessionContracts::HelloPath;
    }
    else
    {
        COMCORE::CRpcMessage resume = NewRequest( CRemoteSessionContracts::ResumePath );
        UInt8 secret[ 32 ];
        resume.SetString( 1, sessionId );
        if ( DRN::CDRNCrypto::FromHex( m_resumeSecretHex.Reveal(), secret, sizeof( secret ) ) )
            resume.SetBytes( 2, secret, sizeof( secret ) );
        memset( secret, 0, sizeof( secret ) );
        resume.SetUInt64( 3, GetLastEventSeq() );
        m_pending = endpoint->CallAsync( CRemoteSessionContracts::ResumePath, resume, options );
        m_pendingMethod = CRemoteSessionContracts::ResumePath;
    }
    m_hasPending = true;
    SetState( REMOTECONNECTION_OPENING, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StepOpening( void )
{GUCEF_TRACE;

    if ( !m_hasPending )
        return;
    CRemoteServiceChannel& control = *m_channels[ 0 ];
    if ( CRemoteServiceChannel::CHANNEL_DOWN == control.GetState() || CRemoteServiceChannel::CHANNEL_REFUSED == control.GetState() )
    {
        m_hasPending = false;
        if ( CRemoteServiceChannel::CHANNEL_REFUSED == control.GetState() )
            FailPermanently( "the service refused the link: " + control.GetLastLinkText() );
        else
            StartReconnect( "the link was lost while the session opened" );
        return;
    }
    if ( !m_pending.IsReady() && !m_pending.HasNoFuture() )
    {
        if ( NowMs() - m_stepStartedMs > m_settings.connectTimeoutMs )
        {
            m_hasPending = false;
            StartReconnect( "the service did not answer " + m_pendingMethod );
        }
        return;
    }
    m_hasPending = false;
    COMCORE::CRpcCallResult result;
    if ( !COMCORE::CRpcEndpoint::TryGetCallResult( m_pending, result ) )
    {
        StartReconnect( m_pendingMethod + " ended without an answer" );
        return;
    }
    HandleOpenResult( result );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::NoteServerTime( UInt64 serverTimeUtcMs )
{GUCEF_TRACE;

    if ( 0 == serverTimeUtcMs )
        return;
    const Int64 offset = (Int64) serverTimeUtcMs - NowUtcMs();
    {
        MT::CScopeMutex lock( m_lock );
        m_clockOffsetMs = offset;
    }
    const Int64 magnitude = offset < 0 ? -offset : offset;
    if ( 0 != m_settings.clockSkewNoticeMs && magnitude > (Int64) m_settings.clockSkewNoticeMs )
    {
        if ( !m_clockSkewNoticed )
        {
            m_clockSkewNoticed = true;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceConnection: the clock of the service differs from this machine's by " << offset
                               << " ms; calls with deadlines may be refused as expired" );
            CORE::TCloneableInt64 data( offset );
            NotifyObservers( ClockSkewEvent, &data );
        }
    }
    else
    {
        m_clockSkewNoticed = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::SaveCredentials( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_store && !m_storeSlot.IsNULLOrEmpty() )
    {
        CORE::CString error;
        if ( !GetCredentials().SaveToStore( *m_store, m_storeSlot, error ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the credentials were not saved: " << error );
    }
    NotifyObservers( CredentialsChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::HandleOpenResult( const COMCORE::CRpcCallResult& result )
{GUCEF_TRACE;

    const CORE::CString detail = CRemoteServiceDetailCodes::GetCode( result.status.details );
    if ( COMCORE::RPC_STATUS_UNAVAILABLE == result.status.code && detail == CRemoteServiceDetailCodes::Draining )
    {
        // case E: the service stops; the next attempt waits for its hint
        m_stopHintMs = result.status.details.GetAttributeValueOrChildValueByName( "retryAfterMs" ).AsUInt32( 1000, true );
        // a client whose event stream did not bring SERVICE_STOPPING (it had none, or it reconnected during the drain) is told here
        CORE::CDataNode stopping( "data", GUCEF_DATATYPE_OBJECT );
        stopping.SetAttribute( "reason", result.status.message );
        stopping.SetAttribute( "retryAfterMs", m_stopHintMs );
        stopping.SetAttribute( "expectedBackSeconds", ( m_stopHintMs + 999 ) / 1000 );
        RaiseServiceStopping( stopping );
        for ( UInt32 i=0; i<m_channels.size(); ++i )
            m_channels[ i ]->DropLink();
        StartReconnect( "the service drains" );
        return;
    }
    if ( m_pendingMethod == CRemoteSessionContracts::ResumePath )
    {
        if ( result.status.IsOk() )
        {
            {
                MT::CScopeMutex lock( m_lock );
                ++m_sessionResumes;
                m_hostInstanceId = result.response.GetString( 3 );
            }
            NoteServerTime( result.response.GetUInt64( 4 ) );
            m_reconnecting = false;
            m_backoff.OnConnected( NowMs() );
            if ( m_settings.openEventStream )
                OpenEvents();
            SetState( REMOTECONNECTION_CONNECTED, "the client session was resumed" );
            return;
        }
        if ( COMCORE::RPC_STATUS_NOT_FOUND == result.status.code || COMCORE::RPC_STATUS_PERMISSION_DENIED == result.status.code )
        {
            // case D: the session is gone; a new one, the application opens its state again
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the client session is unknown at the service (" << detail << "), a new one is opened" );
            {
                MT::CScopeMutex lock( m_lock );
                m_sessionId.Clear();
                m_resumeSecretHex.Clear();
                m_lastEventSeq = 0;
                ++m_sessionReplaces;
            }
            for ( UInt32 i=1; i<m_bulk.size(); ++i )
                m_bulk[ i ].attachedTo.Clear();
            SendHelloOrResume();
            NotifyObservers( SessionReplacedEvent );
            return;
        }
        StartReconnect( "Session/Resume failed: " + result.status.ToString() );
        return;
    }
    if ( !result.status.IsOk() )
    {
        if ( COMCORE::RPC_STATUS_PERMISSION_DENIED == result.status.code )
        {
            FailPermanently( "Session/Hello was refused: " + result.status.ToString() );
            return;
        }
        StartReconnect( "Session/Hello failed: " + result.status.ToString() );
        return;
    }
    bool endpointsChanged = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_helloResponse = result.response;
        m_sessionId = result.response.GetString( 1 );
        CORE::CDynamicBuffer secret;
        result.response.GetBytes( 2, secret );
        m_resumeSecretHex.Set( DRN::CDRNCrypto::ToHex( secret.GetConstBufferPtr(), secret.GetDataSize() ) );
        secret.Clear();
        m_deviceId = result.response.GetString( 3 );
        m_roles.clear();
        for ( UInt32 i=0; i<result.response.GetCount( 5 ); ++i )
        {
            CORE::CVariant role;
            if ( result.response.GetAt( 5, i, role ) )
                m_roles.insert( role.AsString() );
        }
        m_hostInstanceId = result.response.GetString( 8 );
        m_lastEventSeq = result.response.GetUInt64( 11 );
        CORE::CString::StringVector endpoints;
        for ( UInt32 i=0; i<result.response.GetCount( 14 ); ++i )
        {
            CORE::CVariant endpoint;
            if ( result.response.GetAt( 14, i, endpoint ) )
                endpoints.push_back( endpoint.AsString() );
        }
        if ( !endpoints.empty() )
            endpointsChanged = m_credentials.UpdateSessionEndpoints( endpoints );
        if ( result.response.GetUInt64( 15 ) > 0 )
            m_credentials.tokenExpiresUtcMs = result.response.GetUInt64( 15 );
    }
    for ( UInt32 i=1; i<m_bulk.size(); ++i )
        m_bulk[ i ].attachedTo.Clear();
    NoteServerTime( result.response.GetUInt64( 9 ) );
    if ( endpointsChanged )
        SaveCredentials();
    m_reconnecting = false;
    m_connectedOnce = true;
    m_backoff.OnConnected( NowMs() );
    if ( m_settings.openEventStream )
        OpenEvents();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the client session " << GetSessionId().SubstrToIndex( 8, true ) << "... is open as " << GetDeviceId() );
    SetState( REMOTECONNECTION_CONNECTED, "the client session is open" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::OpenEvents( void )
{GUCEF_TRACE;

    if ( !m_events.IsNULL() )
    {
        UnsubscribeFrom( m_events.GetPointerAlways() );
        m_events->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "opened again" ) );
        m_endedStreams.push_back( m_events );
        m_events.Unlink();
    }
    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( 0 );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    if ( endpoint.IsNULL() || !endpoint->ResolveMethod( CRemoteSessionContracts::EventsPath, ref, status ) )
        return;
    COMCORE::CRpcMessage request = endpoint->NewRequest( ref );
    request.SetUInt64( 1, GetLastEventSeq() + 1 );
    COMCORE::CIRpcStreamPtr stream = endpoint->OpenStream( ref, &request );
    if ( stream.IsNULL() )
        return;
    TEventCallback onMessage( this, &CRemoteServiceConnection::OnStreamMessage );
    SubscribeTo( stream.GetPointerAlways(), COMCORE::CIRpcStream::MessageEvent, onMessage );
    TEventCallback onStatus( this, &CRemoteServiceConnection::OnStreamStatus );
    SubscribeTo( stream.GetPointerAlways(), COMCORE::CIRpcStream::StatusEvent, onStatus );
    m_events = stream;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::NoteServiceStopping( const COMCORE::CRpcMessage& event )
{GUCEF_TRACE;

    CORE::CDataNode data;
    event.GetDataNode( 6, data );
    UInt32 hint = data.GetAttributeValueOrChildValueByName( "retryAfterMs" ).AsUInt32( 0, true );
    if ( 0 == hint )
        hint = data.GetAttributeValueOrChildValueByName( "expectedBackSeconds" ).AsUInt32( 0, true ) * 1000;
    m_stopHintMs = hint;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the service stops (" << data.GetAttributeValueOrChildValueByName( "reason" ).AsString()
               << "), back in about " << hint << " ms" );
    RaiseServiceStopping( data );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::RaiseServiceStopping( const CORE::CDataNode& data )
{GUCEF_TRACE;

    // once per stop: the event of the stream and the refusals of the reconnect attempts of the same drain are one stop
    if ( m_stopRaised )
        return;
    m_stopRaised = true;
    TRemoteServiceStoppingEventData eventData( data );
    NotifyObservers( ServiceStoppingEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::OnStreamMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || m_events.IsNULL() || notifier != m_events.GetPointerAlways() )
        return;
    const COMCORE::CRpcStreamMessageEventData* data = static_cast< const COMCORE::CRpcStreamMessageEventData* >( eventData );
    const UInt64 eventSeq = data->message.GetUInt64( 1 );
    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 != eventSeq && eventSeq <= m_lastEventSeq )
        {
            ++m_duplicateEvents;
            return;     // delivered already (a stream opened again from a sequence it had)
        }
        m_lastEventSeq = eventSeq;
    }
    if ( data->message.GetString( 2 ) == CRemoteSessionContracts::EventKindServiceStopping )
        NoteServiceStopping( data->message );
    TRemoteMessageEventData event( data->message );
    NotifyObservers( SessionEventEvent, &event );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::OnStreamStatus( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || m_events.IsNULL() || notifier != m_events.GetPointerAlways() )
        return;
    const COMCORE::CRpcStatus* status = static_cast< const COMCORE::CRpcStatus* >( eventData );
    // the stream raises this event: it is let go of at the next tick, not here where it still notifies
    m_endedStreams.push_back( m_events );
    m_events.Unlink();
    if ( COMCORE::RPC_STATUS_RESUME_GAP == status->code && "events" == status->details.GetAttributeValueOrChildValueByName( "scope" ).AsString() &&
         REMOTECONNECTION_CONNECTED == GetState() )
    {
        // case C: the ring of the session does not hold what this client missed; the answer is taken in the tick
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: the event stream has a gap, resyncing" );
        COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( 0 );
        if ( !endpoint.IsNULL() )
        {
            COMCORE::CRpcMessage request = NewRequest( CRemoteSessionContracts::ResyncPath );
            m_resync = endpoint->CallAsync( CRemoteSessionContracts::ResyncPath, request );
            m_hasResync = true;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StepConnected( void )
{GUCEF_TRACE;

    CRemoteServiceChannel& control = *m_channels[ 0 ];
    COMCORE::CRpcEndpointPtr endpoint = control.GetEndpoint();
    if ( endpoint.IsNULL() )
        return;
    if ( m_hasResync && ( m_resync.IsReady() || m_resync.HasNoFuture() ) )
    {
        m_hasResync = false;
        COMCORE::CRpcCallResult result;
        if ( COMCORE::CRpcEndpoint::TryGetCallResult( m_resync, result ) && result.status.IsOk() )
        {
            {
                MT::CScopeMutex lock( m_lock );
                m_lastEventSeq = result.response.GetUInt64( 1 );
                ++m_resyncs;
            }
            NoteServerTime( result.response.GetUInt64( 3 ) );
            TRemoteMessageEventData data( result.response );
            NotifyObservers( ResyncEvent, &data );
            if ( m_settings.openEventStream )
                OpenEvents();
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: Session/Resync failed: " << result.status.ToString() );
        }
    }

    CRemoteServiceChannel::TChannelState channelState = control.GetState();
    if ( CRemoteServiceChannel::CHANNEL_REFUSED == channelState )
    {
        FailPermanently( "the service refused the link: " + control.GetLastLinkText() );
        return;
    }
    if ( CRemoteServiceChannel::CHANNEL_DOWN == channelState )
    {
        StartReconnect( control.IsRpcClosed() ? "the RPC session ended" : "the link was lost" );
        return;
    }
    if ( control.HasNewRpcSession() && CRemoteServiceChannel::CHANNEL_READY == channelState )
    {
        // the endpoint made a new RPC session by itself (a gap, or unknown at a resume): the client session is resumed on it (case B)
        control.ClearNewRpcSession();
        if ( !m_events.IsNULL() )
        {
            UnsubscribeFrom( m_events.GetPointerAlways() );
            m_endedStreams.push_back( m_events );
            m_events.Unlink();
        }
        SendHelloOrResume();
        return;
    }
    m_backoff.OnStillConnected( NowMs() );
    StepBulkChannels();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::StepBulkChannels( void )
{GUCEF_TRACE;

    const CORE::CString sessionId = GetSessionId();
    if ( sessionId.IsNULLOrEmpty() )
        return;
    for ( UInt32 i=1; i<m_channels.size(); ++i )
    {
        CRemoteServiceChannel& channel = *m_channels[ i ];
        SBulkState& bulk = m_bulk[ i ];
        CRemoteServiceChannel::TChannelState state = channel.GetState();
        if ( CRemoteServiceChannel::CHANNEL_REFUSED == state )
            continue;
        if ( CRemoteServiceChannel::CHANNEL_IDLE == state || CRemoteServiceChannel::CHANNEL_DOWN == state )
        {
            if ( NowMs() < bulk.nextAttemptMs )
                continue;
            bulk.attachedTo.Clear();
            bulk.hasPending = false;
            CORE::CString error;
            if ( !channel.StartJoin( GetCredentials(), MakeChannelSettings( i ), m_settings.hostOverride, error ) )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: bulk channel " << i << " cannot join: " << error );
            bulk.nextAttemptMs = NowMs() + 1000;
            continue;
        }
        if ( CRemoteServiceChannel::CHANNEL_READY != state )
            continue;
        if ( channel.HasNewRpcSession() )
        {
            channel.ClearNewRpcSession();
            bulk.attachedTo.Clear();
            bulk.hasPending = false;
        }
        if ( bulk.attachedTo == sessionId )
            continue;
        if ( bulk.hasPending )
        {
            if ( !bulk.pending.IsReady() && !bulk.pending.HasNoFuture() )
                continue;
            bulk.hasPending = false;
            COMCORE::CRpcCallResult result;
            if ( COMCORE::CRpcEndpoint::TryGetCallResult( bulk.pending, result ) && result.status.IsOk() )
            {
                bulk.attachedTo = sessionId;
                MT::CScopeMutex lock( m_lock );
                ++m_bulkAttaches;
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceConnection: bulk channel " << i << " was not attached: " << result.status.ToString() );
                bulk.nextAttemptMs = NowMs() + 1000;
            }
            continue;
        }
        if ( NowMs() < bulk.nextAttemptMs )
            continue;
        COMCORE::CRpcEndpointPtr endpoint = channel.GetEndpoint();
        if ( endpoint.IsNULL() )
            continue;
        COMCORE::CRpcMessage request = NewRequest( CRemoteSessionContracts::AttachPath, i );
        UInt8 secret[ 32 ];
        request.SetString( 1, sessionId );
        if ( DRN::CDRNCrypto::FromHex( m_resumeSecretHex.Reveal(), secret, sizeof( secret ) ) )
            request.SetBytes( 2, secret, sizeof( secret ) );
        memset( secret, 0, sizeof( secret ) );
        request.SetString( 3, "bulk" );
        COMCORE::CRpcCallOptions options;
        options.waitForReady = false;
        options.deadlineMs = 15000;
        bulk.pending = endpoint->CallAsync( CRemoteSessionContracts::AttachPath, request, options );
        bulk.hasPending = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::DropLink( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_channels.size(); ++i )
        m_channels[ i ]->DropLink();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::Close( bool goodbye, UInt32 timeoutMs )
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( 0 );
    if ( goodbye && REMOTECONNECTION_CONNECTED == GetState() && !endpoint.IsNULL() )
    {
        COMCORE::CRpcMessage request = NewRequest( CRemoteSessionContracts::GoodbyePath );
        request.SetString( 1, "the client closes" );
        COMCORE::CRpcCallOptions options;
        options.deadlineMs = timeoutMs;
        options.waitForReady = false;
        CORE::CFutureResult future = endpoint->CallAsync( CRemoteSessionContracts::GoodbyePath, request, options );
        const UInt64 startedAt = NowMs();
        while ( !future.IsReady() && !future.HasNoFuture() && NowMs() - startedAt < timeoutMs )
            WaitStep();
    }
    m_timer.SetEnabled( false );
    if ( !m_events.IsNULL() )
    {
        UnsubscribeFrom( m_events.GetPointerAlways() );
        m_events.Unlink();
    }
    for ( UInt32 i=0; i<m_channels.size(); ++i )
    {
        COMCORE::CRpcEndpointPtr channelEndpoint = m_channels[ i ]->GetEndpoint();
        if ( !channelEndpoint.IsNULL() )
        {
            channelEndpoint->Close( true );
            const UInt64 startedAt = NowMs();
            while ( COMCORE::RPC_SESSIONSTATE_CLOSED != channelEndpoint->GetState() && NowMs() - startedAt < timeoutMs )
                WaitStep();
        }
        m_channels[ i ]->Shutdown( false );
        m_bulk[ i ].attachedTo.Clear();
        m_bulk[ i ].hasPending = false;
    }
    WaitStep();
    {
        MT::CScopeMutex lock( m_lock );
        m_sessionId.Clear();
        m_resumeSecretHex.Clear();
    }
    m_hasPending = false;
    m_hasResync = false;
    SetState( REMOTECONNECTION_IDLE, "closed" );
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcMessage
CRemoteServiceConnection::NewRequest( const CORE::CString& methodPath, UInt32 channel ) const
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( channel );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    if ( endpoint.IsNULL() || !endpoint->ResolveMethod( methodPath, ref, status ) )
        return COMCORE::CRpcMessage();
    return endpoint->NewRequest( ref );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::Call( const CORE::CString& methodPath, const COMCORE::CRpcMessage& request, COMCORE::CRpcMessage& response,
                                COMCORE::CRpcStatus& status, const COMCORE::CRpcCallOptions& options, UInt32 channel )
{GUCEF_TRACE;

    CORE::CFutureResult future = CallAsync( methodPath, request, options, channel );
    if ( future.HasNoFuture() )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "no connection to the service" );
        return false;
    }
    // the protocol runs on the nodes' threads: this thread only waits (and delivers the events of its own pulse generator)
    while ( !future.IsReady() && !future.HasNoFuture() )
        WaitStep();
    COMCORE::CRpcCallResult result;
    if ( !COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) )
    {
        status.Set( COMCORE::RPC_STATUS_UNKNOWN, "the call ended without a result" );
        return false;
    }
    status = result.status;
    response = result.response;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteServiceConnection::CallAsync( const CORE::CString& methodPath, const COMCORE::CRpcMessage& request,
                                     const COMCORE::CRpcCallOptions& options, UInt32 channel )
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( channel );
    if ( endpoint.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_INVALID_STATUS );
    return endpoint->CallAsync( methodPath, request, options );
}

/*-------------------------------------------------------------------------*/

COMCORE::CIRpcStreamPtr
CRemoteServiceConnection::OpenStream( const CORE::CString& methodPath, const COMCORE::CRpcMessage* firstRequest,
                                      const COMCORE::CRpcCallOptions& options, UInt32 channel )
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = GetEndpoint( channel );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    if ( endpoint.IsNULL() || !endpoint->ResolveMethod( methodPath, ref, status ) )
        return COMCORE::CIRpcStreamPtr();
    return endpoint->OpenStream( ref, firstRequest, options );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnection::RenewToken( CORE::CString& error )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request = NewRequest( CRemoteSessionContracts::RenewTokenPath );
    COMCORE::CRpcMessage response;
    COMCORE::CRpcStatus status;
    if ( !Call( CRemoteSessionContracts::RenewTokenPath, request, response, status ) || !status.IsOk() )
    {
        error = "Session/RenewToken failed: " + status.ToString();
        return false;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_credentials.token.Set( response.GetString( 1 ) );
        m_credentials.tokenExpiresUtcMs = response.GetUInt64( 2 );
    }
    SaveCredentials();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceConnection::GetTelemetry( CORE::CDataNode& telemetry ) const
{GUCEF_TRACE;

    telemetry.Clear();
    telemetry.SetName( "remoteConnection" );
    telemetry.SetNodeType( GUCEF_DATATYPE_OBJECT );
    {
        MT::CScopeMutex lock( m_lock );
        telemetry.SetAttribute( "state", RemoteConnectionStateToString( m_state ) );
        telemetry.SetAttribute( "rpcResumes", m_rpcResumes );
        telemetry.SetAttribute( "sessionResumes", m_sessionResumes );
        telemetry.SetAttribute( "resyncs", m_resyncs );
        telemetry.SetAttribute( "sessionReplaces", m_sessionReplaces );
        telemetry.SetAttribute( "attempts", m_attempts );
        telemetry.SetAttribute( "wakeDetections", m_wakeDetections );
        telemetry.SetAttribute( "duplicateEvents", m_duplicateEvents );
        telemetry.SetAttribute( "bulkAttaches", m_bulkAttaches );
        telemetry.SetAttribute( "lastEventSeq", m_lastEventSeq );
        telemetry.SetAttribute( "clockOffsetMs", m_clockOffsetMs );
    }
    for ( UInt32 i=0; i<m_channels.size(); ++i )
    {
        CORE::CDataNode* node = telemetry.AddChild( "channel", GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "index", i );
        node->SetAttribute( "state", CRemoteServiceChannel::ChannelStateToString( m_channels[ i ]->GetState() ) );
        node->SetAttribute( "links", m_channels[ i ]->GetLinkCount() );
        node->SetAttribute( "rpcResumes", m_channels[ i ]->GetRpcResumeCount() );
        node->SetAttribute( "newRpcSessions", m_channels[ i ]->GetNewRpcSessionCount() );
        UInt32 rtt = 0;
        if ( m_channels[ i ]->GetRoundTripTime( rtt ) )
            node->SetAttribute( "rttUs", rtt );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
