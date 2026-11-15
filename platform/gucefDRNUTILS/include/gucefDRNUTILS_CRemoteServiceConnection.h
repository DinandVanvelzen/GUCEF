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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CIUTCCLOCK_H
#include "gucefCORE_CIUtcClock.h"
#define GUCEF_CORE_CIUTCCLOCK_H
#endif /* GUCEF_CORE_CIUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CIRPCSTREAM_H
#include "gucefCOMCORE_CIRpcStream.h"
#define GUCEF_COMCORE_CIRPCSTREAM_H
#endif /* GUCEF_COMCORE_CIRPCSTREAM_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLOPTIONS_H
#include "gucefCOMCORE_CRpcCallOptions.h"
#define GUCEF_COMCORE_CRPCCALLOPTIONS_H
#endif /* GUCEF_COMCORE_CRPCCALLOPTIONS_H ? */

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#ifndef GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#include "gucefDRNUTILS_CReconnectBackoff.h"
#define GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#endif /* GUCEF_DRNUTILS_CRECONNECTBACKOFF_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H
#include "gucefDRNUTILS_CRemoteServiceConnectionSettings.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H
#include "gucefDRNUTILS_CRemoteServiceChannel.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTCloneableObj< TRemoteConnectionState > TRemoteConnectionStateEventData;
typedef CORE::CTCloneableObj< COMCORE::CRpcMessage > TRemoteMessageEventData;

/**
 *  The connection of a paired client to its remote service (gucefASSETSTORE_RemoteNetworking_Design.md 4 and 5): a
 *  control channel (CRemoteServiceChannel: a DRN link with the pinned key and the device token, and an RPC session over
 *  it), optional bulk channels on links of their own attached to the client session, the client session itself
 *  (Session/Hello, or Session/Resume of the session it had), the event stream with its sequence numbers delivered once
 *  and in order (SessionEventEvent), and the reconnect cases of the design:
 *      A   the link dropped and the RPC session resumes over a new link: calls, streams and events go on;
 *      B   the RPC session is new (a gap, unknown at the service) while the client session lives: Session/Resume, the
 *          event stream from the last sequence, the bulk channels attached again;
 *      C   the event stream ended with RESUME_GAP: Session/Resync (ResyncEvent), then the stream from its sequence;
 *      D   the client session is unknown (grace over, the service restarted): a new Hello (SessionReplacedEvent);
 *      E   the service drains (SERVICE_STOPPING, GOODBYE): the next attempt waits for its hint;
 *      F   a permanent refusal (banned, revoked, another key or policy, a token not accepted): FAILED, no more attempts.
 *  Reconnects follow CReconnectBackoff (full jitter, the service's hints as lower bounds). A jump of the wall clock past
 *  the monotonic clock (the machine slept) drops the links and reconnects at once.
 *  Threading: the connection lives on the notification pulse generator given at construction; a timer on it drives the
 *  steps, and its events are raised there. The DRN nodes of its channels work on threads of their own. The blocking
 *  forms (Connect(), Call(), RenewToken(), Close()) run on the calling thread: when that thread drives the notification
 *  pulse generator (or nobody drives it) they pump it while they wait, otherwise they only wait for the thread that
 *  drives it. StartConnect() and CallAsync() return at once. Secrets (the token, the resume secret) are never logged.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceConnection : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent StateChangedEvent;            /**< TRemoteConnectionStateEventData: the new state */
    static const CORE::CEvent SessionEventEvent;            /**< TRemoteMessageEventData: a gucef.drnutils.session Event, in eventSeq order, once */
    static const CORE::CEvent ResyncEvent;                  /**< TRemoteMessageEventData: the ResyncResponse after a gap of the event stream */
    static const CORE::CEvent SessionReplacedEvent;         /**< the client session was unknown at the service and a new one was opened (case D) */
    static const CORE::CEvent CredentialsChangedEvent;      /**< the session endpoints or the token of the credentials changed */
    static const CORE::CEvent ClockSkewEvent;               /**< CORE::TCloneableInt64: the service's clock minus this machine's, milliseconds */

    static void RegisterEvents( void );

    CRemoteServiceConnection( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRemoteServiceConnectionSettings& settings );
    virtual ~CRemoteServiceConnection() GUCEF_VIRTUAL_OVERRIDE;

    /** A credential store to keep changed credentials in (endpoints, renewed token) under the slot; not owned, NULL for none */
    void SetCredentialStore( AUTH::ICredentialStore* store, const CORE::CString& slot );

    /** The clocks of the wake detection and the timing (tests); not owned, NULL for the system clocks */
    void SetClocks( CORE::CIUtcClock* utcClock, CORE::CIMonotonicClock* monotonicClock );

    /** Starts connecting and returns; StateChangedEvent tells how it goes. False when it cannot start */
    bool StartConnect( const CDeviceCredentials& credentials, CORE::CString& error );

    /**
     *  Blocking: StartConnect() and wait until CONNECTED or FAILED, at most connectTimeoutMs. When the credentials hold no
     *  session endpoint, there is no hostOverride and lanDiscovery is on, it first looks on the local network for the
     *  paired service (FindLanCandidates()) and connects to what it finds; the pinned key authenticates it as always
     */
    bool Connect( const CDeviceCredentials& credentials, CORE::CString& error );

    /**
     *  Blocking, at most lanDiscoveryFinder.findTimeoutMs: candidates of the service on the local network, only those
     *  with the fingerprint of the pinned key and the service id of the credentials (when the credentials have them).
     *  On the thread that drives the notification pulse generator the look runs on it, else on a pulse generator of its
     *  own (COMCORE::CServiceFinder::FindOnce()). False when lanDiscovery is off or nothing was found
     */
    bool FindLanCandidates( const CDeviceCredentials& credentials, COMCORE::CServiceFinder::TServiceCandidateVector& candidates );

    /** Adds the endpoints of the candidates after the endpoints the credentials hold, without duplicates; how many it added */
    static UInt32 AddCandidateEndpoints( CDeviceCredentials& credentials, const COMCORE::CServiceFinder::TServiceCandidateVector& candidates );

    /** Waits until CONNECTED (true), FAILED or the time is over (false); see the class description for the waiting */
    bool WaitUntilConnected( UInt32 timeoutMs );

    /** Ends the client session with Goodbye (when 'goodbye' and connected) and closes the links; blocking for at most timeoutMs */
    void Close( bool goodbye = true, UInt32 timeoutMs = 5000 );

    /** Drops the links without Goodbye, as a lost network would; the connection reconnects when autoReconnect */
    void DropLink( void );

    TRemoteConnectionState GetState( void ) const;
    CORE::CString GetLastError( void ) const;

    /** The client session: its id, the device, its roles, the answer of the last Hello */
    CORE::CString GetSessionId( void ) const;
    CORE::CString GetDeviceId( void ) const;
    CORE::CString::StringSet GetRoles( void ) const;
    CORE::CString GetHostInstanceId( void ) const;
    COMCORE::CRpcMessage GetHelloResponse( void ) const;

    /** The eventSeq of the last event delivered (0 before any) */
    UInt64 GetLastEventSeq( void ) const;

    /** The service's clock minus this machine's at the last Hello, Resume or Resync, milliseconds */
    Int64 GetClockOffsetMs( void ) const;

    /** Counters of the reconnect cases and more, for tests and telemetry */
    UInt32 GetRpcResumeCount( void ) const;             /**< case A */
    UInt32 GetSessionResumeCount( void ) const;         /**< case B */
    UInt32 GetResyncCount( void ) const;                /**< case C */
    UInt32 GetSessionReplaceCount( void ) const;        /**< case D */
    UInt32 GetReconnectAttemptCount( void ) const;
    UInt32 GetWakeDetectionCount( void ) const;
    UInt32 GetDuplicateEventCount( void ) const;        /**< events dropped because they were delivered before */
    UInt32 GetBulkAttachCount( void ) const;

    /** The delay of the last wait before a reconnect attempt, milliseconds (0 before the first) */
    UInt32 GetNextAttemptDelayMs( void ) const;

    /** The channel of an index: 0 the control channel, 1.. the bulk channels; NULL for none */
    CRemoteServiceChannel* GetChannel( UInt32 index ) const;

    /** Whether the bulk channel is attached to the current client session */
    bool IsBulkChannelReady( UInt32 index ) const;

    /** The endpoint of a channel (NULL before its first link) */
    COMCORE::CRpcEndpointPtr GetEndpoint( UInt32 channel = 0 ) const;

    /** A request message for a method of a bound contract; unbound when the method is not known */
    COMCORE::CRpcMessage NewRequest( const CORE::CString& methodPath, UInt32 channel = 0 ) const;

    /** Blocking call, see the class description; false without an endpoint or an answer */
    bool Call( const CORE::CString& methodPath, const COMCORE::CRpcMessage& request, COMCORE::CRpcMessage& response,
               COMCORE::CRpcStatus& status, const COMCORE::CRpcCallOptions& options = COMCORE::CRpcCallOptions(), UInt32 channel = 0 );

    /** Asynchronous call: the future's result data is a COMCORE::CRpcCallResult */
    CORE::CFutureResult CallAsync( const CORE::CString& methodPath, const COMCORE::CRpcMessage& request,
                                   const COMCORE::CRpcCallOptions& options = COMCORE::CRpcCallOptions(), UInt32 channel = 0 );

    /** A streaming call */
    COMCORE::CIRpcStreamPtr OpenStream( const CORE::CString& methodPath, const COMCORE::CRpcMessage* firstRequest,
                                        const COMCORE::CRpcCallOptions& options = COMCORE::CRpcCallOptions(), UInt32 channel = 0 );

    /** Session/RenewToken: the new token goes into the credentials (and the store when set); blocking */
    bool RenewToken( CORE::CString& error );

    /** The credentials the connection uses, with what the service changed */
    CDeviceCredentials GetCredentials( void ) const;

    /** State, counters and the round trip time of each channel */
    void GetTelemetry( CORE::CDataNode& telemetry ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    friend class CRemoteServiceChannel;

    /** The transport a channel's RPC session runs over, made for each member link; a derived class may decorate it (tests). Default: as it is */
    virtual COMCORE::CIRpcTransportPtr WrapTransport( UInt32 channel, const COMCORE::CIRpcTransportPtr& transport );

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteServiceConnection > TEventCallback;
    typedef GUCEF::vector< CRemoteServiceChannel* > TChannelVector;

    struct SBulkState
    {
        CORE::CString attachedTo;               /**< the client session the channel is attached to; empty: attach it */
        CORE::CFutureResult pending;
        bool hasPending;
        UInt64 nextAttemptMs;

        SBulkState( void );
    };
    typedef GUCEF::vector< SBulkState > TBulkStateVector;

    CRemoteServiceConnection( const CRemoteServiceConnection& src );                /**< not implemented */
    CRemoteServiceConnection& operator=( const CRemoteServiceConnection& src );     /**< not implemented */

    void OnTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStreamMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStreamStatus( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void SetState( TRemoteConnectionState state, const CORE::CString& reason );
    bool BeginJoin( CORE::CString& error );
    void StepJoining( void );
    void StepOpening( void );
    void StepConnected( void );
    void StepReconnecting( void );
    void StepBulkChannels( void );
    void CheckWake( void );
    void SendHelloOrResume( void );
    void HandleOpenResult( const COMCORE::CRpcCallResult& result );
    void OpenEvents( void );
    void StartReconnect( const CORE::CString& why );
    void FailPermanently( const CORE::CString& why );
    void NoteServerTime( UInt64 serverTimeUtcMs );
    void NoteServiceStopping( const COMCORE::CRpcMessage& event );
    void SaveCredentials( void );
    CRemoteServiceChannel::SSettings MakeChannelSettings( UInt32 channel ) const;
    bool IsPumpingThread( void ) const;
    void WaitStep( void );
    UInt64 NowMs( void ) const;
    Int64 NowUtcMs( void ) const;

    mutable MT::CMutex m_lock;
    CORE::PulseGeneratorPtr m_pulse;
    CRemoteServiceConnectionSettings m_settings;
    AUTH::ICredentialStore* m_store;
    CORE::CString m_storeSlot;
    CORE::CIUtcClock* m_utcClock;
    CORE::CIMonotonicClock* m_monotonicClock;
    CORE::CSystemUtcClock m_systemUtcClock;
    CORE::CSystemMonotonicClock m_systemMonotonicClock;
    CDeviceCredentials m_credentials;
    TRemoteConnectionState m_state;
    CORE::CString m_lastError;
    CORE::CTimer m_timer;
    TChannelVector m_channels;
    TBulkStateVector m_bulk;
    CReconnectBackoff m_backoff;
    CORE::CFutureResult m_pending;
    bool m_hasPending;
    CORE::CString m_pendingMethod;
    CORE::CFutureResult m_resync;
    bool m_hasResync;
    COMCORE::CIRpcStreamPtr m_events;
    GUCEF::vector< COMCORE::CIRpcStreamPtr > m_endedStreams;   /**< let go of at the next tick, never inside one of their own events */
    CORE::CString m_sessionId;
    AUTH::CSecretString m_resumeSecretHex;
    CORE::CString m_deviceId;
    CORE::CString::StringSet m_roles;
    CORE::CString m_hostInstanceId;
    COMCORE::CRpcMessage m_helloResponse;
    UInt64 m_lastEventSeq;
    Int64 m_clockOffsetMs;
    bool m_clockSkewNoticed;
    UInt64 m_stepStartedMs;
    UInt64 m_nextAttemptMs;
    UInt32 m_nextAttemptDelayMs;
    UInt32 m_stopHintMs;
    bool m_reconnecting;
    bool m_attemptInProgress;                   /**< an attempt of the reconnect is under way: when it fails the next one waits for the backoff */
    bool m_connectedOnce;
    UInt64 m_lastTickMs;
    Int64 m_lastTickUtcMs;
    UInt32 m_rpcResumes;
    UInt32 m_sessionResumes;
    UInt32 m_resyncs;
    UInt32 m_sessionReplaces;
    UInt32 m_attempts;
    UInt32 m_wakeDetections;
    UInt32 m_duplicateEvents;
    UInt32 m_bulkAttaches;
    bool m_inTick;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */
