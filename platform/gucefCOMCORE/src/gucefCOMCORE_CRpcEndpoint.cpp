/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#include "gucefCOMCORE_CRpcBuiltInContracts.h"
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */

#ifndef GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H
#include "gucefCOMCORE_CRpcReflectionService.h"
#define GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H
#endif /* GUCEF_COMCORE_CRPCREFLECTIONSERVICE_H ? */

#ifndef GUCEF_COMCORE_CRPCCLIENTSTREAM_H
#include "gucefCOMCORE_CRpcClientStream.h"
#define GUCEF_COMCORE_CRPCCLIENTSTREAM_H
#endif /* GUCEF_COMCORE_CRPCCLIENTSTREAM_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLTASK_H
#include "gucefCOMCORE_CRpcCallTask.h"
#define GUCEF_COMCORE_CRPCCALLTASK_H
#endif /* GUCEF_COMCORE_CRPCCALLTASK_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#include "gucefCOMCORE_CRpcEndpoint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Routes the session's callbacks to the endpoint */
class CRpcEndpoint::CSessionSink : public CRpcSession::CISink
{
    public:

    explicit CSessionSink( CRpcEndpoint& endpoint ) : m_endpoint( endpoint ) {}

    virtual bool OnSessionTransmit( const UInt8* frame, UInt32 size, TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE
    { return m_endpoint.SessionTransmit( frame, size, protection ); }

    virtual void OnSessionDeliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith ) GUCEF_VIRTUAL_OVERRIDE
    { m_endpoint.SessionDeliver( frame, size, header, arrivedWith ); }

    virtual void OnSessionProtocolViolation( const CRpcStatus& status ) GUCEF_VIRTUAL_OVERRIDE
    { m_endpoint.SessionProtocolViolation( status ); }

    virtual void OnSessionGap( UInt64 from, UInt64 to ) GUCEF_VIRTUAL_OVERRIDE
    { m_endpoint.SessionGap( from, to ); }

    virtual void OnSessionWritable( void ) GUCEF_VIRTUAL_OVERRIDE
    { m_endpoint.SessionWritable(); }

    private:

    CRpcEndpoint& m_endpoint;
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SClientCall
{
    UInt64 callRef;
    CRpcMethodRef ref;
    CRpcCallOptions options;
    CORE::CFutureResultSource future;
    bool hasFuture;
    CIRpcStreamPtr stream;
    CRpcClientStream* streamImpl;
    CORE::CDynamicBuffer requestBytes;
    bool hasRequest;
    UInt64 deadlineMs;
    UInt64 deadlineUnixMs;
    bool accepted;
    CRpcRequestId requestId;
    bool durable;
    UInt64 resultDeadlineMs;
    UInt64 sendCreditLimit;
    UInt64 sentMessages;
    UInt64 receivedMessages;
    UInt64 consumedMessages;
    UInt64 grantedLimit;
    UInt32 initialCredit;
    bool halfClosed;

    SClientCall( void )
        : callRef( 0 ), ref(), options(), future(), hasFuture( false ), stream(), streamImpl( GUCEF_NULL ), requestBytes(),
          hasRequest( false ), deadlineMs( 0 ), deadlineUnixMs( 0 ), accepted( false ), requestId(), durable( false ),
          resultDeadlineMs( 0 ), sendCreditLimit( 0 ), sentMessages( 0 ), receivedMessages( 0 ), consumedMessages( 0 ),
          grantedLimit( 0 ), initialCredit( 0 ), halfClosed( false )
    {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SOutBinding
{
    enum EState { STATE_FETCHING, STATE_SENT, STATE_BOUND, STATE_FAILED };

    UInt32 bindIndex;
    CORE::CString name;
    UInt32 major;
    UInt32 minMinor;
    CRpcContractPtr contract;
    UInt32 providerMinor;
    CRpcContractHash providerHash;
    CRpcContractHash fetchHash;
    EState state;
    CRpcStatus failStatus;

    SOutBinding( void ) : bindIndex( 0 ), name(), major( 0 ), minMinor( 0 ), contract(), providerMinor( 0 ), providerHash(),
                          fetchHash(), state( STATE_FETCHING ), failStatus() {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SInBinding
{
    UInt32 bindIndex;
    CRpcContractPtr contract;
    bool ok;
    CRpcStatus failStatus;

    SInBinding( void ) : bindIndex( 0 ), contract(), ok( false ), failStatus() {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SRequestInfo
{
    UInt64 callRef;
    UInt8 flags;
    UInt32 bindIndex;
    UInt32 serviceNumber;
    UInt32 methodNumber;
    UInt64 deadlineUnixMs;
    UInt32 callFlags;
    UInt64 resumeFromIndex;
    CRpcMetadata metadata;

    SRequestInfo( void ) : callRef( 0 ), flags( 0 ), bindIndex( 0 ), serviceNumber( 0 ), methodNumber( 0 ), deadlineUnixMs( 0 ),
                           callFlags( 0 ), resumeFromIndex( 0 ), metadata() {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SAssembly
{
    UInt64 startSeq;
    CORE::CDynamicBuffer frame;         /**< the first frame up to its payload (MORE cleared), then the joined payload */
    UInt32 prefixSize;
    UInt64 total;
    UInt64 received;
    TRpcProtectionClass protection;
    bool isRequest;
    SRequestInfo request;               /**< REQUEST fields are parsed once: metadata interning must not run twice */

    SAssembly( void ) : startSeq( 0 ), frame(), prefixSize( 0 ), total( 0 ), received( 0 ), protection( RPC_PROTECTION_CONFIDENTIAL ),
                        isRequest( false ), request() {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SFragmenter
{
    /** A later frame of the same call: it may not overtake the fragments (6.10) */
    struct SFollower
    {
        UInt8 type;
        UInt8 flags;
        CORE::CDynamicBuffer fields;
        CORE::CDynamicBuffer payload;
        TRpcProtectionClass protection;
    };

    UInt64 startSeq;
    UInt64 orderKey;                        /**< the call and direction, 0 = not a call's frame */
    CORE::CDynamicBuffer payload;
    UInt32 offset;
    TRpcProtectionClass protection;
    GUCEF::vector< SFollower > followers;

    SFragmenter( void ) : startSeq( 0 ), orderKey( 0 ), payload(), offset( 0 ), protection( RPC_PROTECTION_CONFIDENTIAL ), followers() {}
};

/*-------------------------------------------------------------------------*/

struct CRpcEndpoint::SPendingQuery
{
    CORE::CFutureResultSource future;
    CRpcRequestId id;
    bool resend;
    bool isCancel;

    SPendingQuery( void ) : future(), id(), resend( false ), isCancel( false ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An event raised under the endpoint lock, raised once the lock is released */
struct CRpcEndpoint::SDeferredEvent
{
    enum EKind
    {
        KIND_ENDPOINT_EVENT,
        KIND_STREAM_EVENT,
        KIND_CALL_EVENT,
        KIND_DISPATCH,
        KIND_PULSES_START,          /**< periodic pulses of the protocol pulse generator for the endpoint's timers */
        KIND_PULSES_STOP,
        KIND_DOWNGRADE_REFUSED      /**< CIRpcTransport::OnProtectionDowngradeRefused() */
    };

    EKind kind;
    CORE::CEvent eventId;
    CORE::CICloneable* eventData;     /**< owned */
    CIRpcStreamPtr stream;
    CRpcServerCallPtr call;
    TRpcExecutionMode mode;
    CORE::PulseGeneratorPtr pulseGenerator;
    UInt32 intervalMs;
    CIRpcTransportPtr transport;
    TRpcProtectionClass required;
    TRpcProtectionClass arrivedWith;

    explicit SDeferredEvent( EKind k )
        : kind( k ), eventId(), eventData( GUCEF_NULL ), stream(), call(), mode( RPC_EXECUTION_CALLING_THREAD )
        , pulseGenerator(), intervalMs( 0 ), transport(), required( RPC_PROTECTION_CONFIDENTIAL ), arrivedWith( RPC_PROTECTION_CONFIDENTIAL )
    {}

    ~SDeferredEvent()
    {
        GUCEF_DELETE eventData;
        eventData = GUCEF_NULL;
    }

    private:
    SDeferredEvent( const SDeferredEvent& src );
    SDeferredEvent& operator=( const SDeferredEvent& src );
};

/*-------------------------------------------------------------------------*/

/** A handler of the calling thread mode: run by the thread that dispatched it, once that thread released the lock */
struct CRpcEndpoint::SPendingHandler
{
    CRpcServerCallPtr call;
    TThreadId threadId;

    SPendingHandler( void ) : call(), threadId( 0 ) {}
    SPendingHandler( const CRpcServerCallPtr& c, TThreadId t ) : call( c ), threadId( t ) {}
};

/*-------------------------------------------------------------------------*/

/** Holds the endpoint lock for its scope; the events raised meanwhile go out when the outermost scope ends */
class CRpcEndpoint::CLockScope
{
    public:

    explicit CLockScope( const CRpcEndpoint& endpoint )
        : m_endpoint( endpoint ), m_locked( false )
    {
        m_endpoint.LockState();
        m_locked = true;
    }

    ~CLockScope()
    {
        if ( m_locked )
            m_endpoint.UnlockState();
    }

    void EarlyUnlock( void )
    {
        if ( m_locked )
        {
            m_locked = false;
            m_endpoint.UnlockState();
        }
    }

    private:
    CLockScope( const CLockScope& src );
    CLockScope& operator=( const CLockScope& src );

    const CRpcEndpoint& m_endpoint;
    bool m_locked;
};

/*-------------------------------------------------------------------------*/

const CORE::CEvent CRpcEndpoint::SessionEstablishedEvent = "GUCEF::COMCORE::CRpcEndpoint::SessionEstablishedEvent";
const CORE::CEvent CRpcEndpoint::SessionSuspendedEvent = "GUCEF::COMCORE::CRpcEndpoint::SessionSuspendedEvent";
const CORE::CEvent CRpcEndpoint::SessionResumedEvent = "GUCEF::COMCORE::CRpcEndpoint::SessionResumedEvent";
const CORE::CEvent CRpcEndpoint::SessionGapEvent = "GUCEF::COMCORE::CRpcEndpoint::SessionGapEvent";
const CORE::CEvent CRpcEndpoint::SessionClosedEvent = "GUCEF::COMCORE::CRpcEndpoint::SessionClosedEvent";
const CORE::CEvent CRpcEndpoint::ContractBoundEvent = "GUCEF::COMCORE::CRpcEndpoint::ContractBoundEvent";
const CORE::CEvent CRpcEndpoint::AcceptedEvent = "GUCEF::COMCORE::CRpcEndpoint::AcceptedEvent";
const CORE::CEvent CRpcEndpoint::GapDetectedEvent = "GUCEF::COMCORE::CRpcEndpoint::GapDetectedEvent";
const CORE::CEvent CRpcEndpoint::DispatchToNotificationThreadEvent = "GUCEF::COMCORE::CRpcEndpoint::DispatchToNotificationThreadEvent";
const CORE::CEvent CRpcEndpoint::ReleaseOnNotificationThreadEvent = "GUCEF::COMCORE::CRpcEndpoint::ReleaseOnNotificationThreadEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Frame header room a session frame needs at most: type, flags, seq and ack varints */
const UInt32 g_frameHeaderReserve = 2 + 10 + 10;

bool
IsZeroBytes( const UInt8* bytes, UInt32 size )
{
    for ( UInt32 i=0; i<size; ++i )
        if ( 0 != bytes[ i ] )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CRpcEndpoint::RegisterEvents( void )
{GUCEF_TRACE;

    SessionEstablishedEvent.Initialize();
    SessionSuspendedEvent.Initialize();
    SessionResumedEvent.Initialize();
    SessionGapEvent.Initialize();
    SessionClosedEvent.Initialize();
    ContractBoundEvent.Initialize();
    AcceptedEvent.Initialize();
    GapDetectedEvent.Initialize();
    DispatchToNotificationThreadEvent.Initialize();
    ReleaseOnNotificationThreadEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcEndpoint::NowMs( void )
{GUCEF_TRACE;

    return CRpcDurableRequestTable::NowMs();
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcEndpoint::WallClockMs( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

CRpcEndpointPtr
CRpcEndpoint::Create( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcEndpointSettings& settings )
{GUCEF_TRACE;

    // the global state of gucefCOMCORE registers what an endpoint uses (the task type of the thread pool mode, the
    // transport factories). Loading the module does not create it on every platform, and an application may use an
    // endpoint before anything else of gucefCOMCORE
    CComCoreGlobal::Instance();

    CRpcEndpoint* endpoint = GUCEF_NEW CRpcEndpoint( notificationPulseGenerator, settings );
    return endpoint->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CRpcEndpoint::CRpcEndpoint( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcEndpointSettings& settings )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , CORE::CTSharedObjCreator< CRpcEndpoint, MT::CMutex >( this )
    , m_lock()
    , m_lockDepth( 0 )
    , m_deferredEvents()
    , m_raisingDeferredEvents( false )
    , m_pendingHandlers()
    , m_settings( settings )
    , m_link()
    , m_sink( GUCEF_NEW CSessionSink( *this ) )
    , m_session( *m_sink )
    , m_transport()
    , m_protocolPulseGenerator()
    , m_transportObserver()
    , m_staleTransports()
    , m_selfObserver()
    , m_host( GUCEF_NULL )
    , m_remoteUri()
    , m_initiator( false )
    , m_started( false )
    , m_state( RPC_SESSIONSTATE_IDLE )
    , m_sessionGeneration( 1 )
    , m_bindGeneration( 1 )
    , m_hasIdentity( false )
    , m_lineageOwner()
    , m_resumeCount( 0 )
    , m_resumePending( false )
    , m_peerLimits()
    , m_peerNodeName()
    , m_peerOffers()
    , m_peerWants()
    , m_wants()
    , m_principal()
    , m_principalPinned( false )
    , m_services( CRpcServiceSet::Create() )
    , m_accessPolicy()
    , m_durable()
    , m_rpcContract( CRpcBuiltInContracts::GetRpcContract() )
    , m_outBindings()
    , m_inBindings()
    , m_nextBindIndex( 1 )
    , m_getContractCount( 0 )
    , m_outKeys()
    , m_inKeys()
    , m_clientCalls()
    , m_serverCalls()
    , m_retiredServerCalls()
    , m_pumpingFragments( false )
    , m_acceptedIds()
    , m_pendingQueries()
    , m_orphanResults()
    , m_assemblies()
    , m_assemblyBytes( 0 )
    , m_fragmenters()
    , m_fragmenterCursor( 0 )
    , m_waitingCalls()
    , m_lastReceiveMs( 0 )
    , m_lastSendMs( 0 )
    , m_pingSentMs( 0 )
    , m_pingId( 0 )
    , m_suspendedSinceMs( 0 )
    , m_nextReconnectMs( 0 )
    , m_reconnectDue( false )
    , m_reconnectDelayMs( settings.reconnectInitialDelayMs )
    , m_helloSentMs( 0 )
    , m_goodbyeSeq( 0 )
    , m_closeRequestedMs( 0 )
    , m_closeGraceful( false )
    , m_blockingDepth( 0 )
    , m_handlerDepth( 0 )
    , m_closeStatus()
    , m_lastHelloAck()
    , m_nextCreditResendMs( 0 )
    , m_nextCallScanMs( 0 )
    , m_queueBlockedStreams()
    , m_hasUnknownResumeId( false )
    , m_scratch()
    , m_controlBody()
    , m_counters()
    , m_reflection( GUCEF_NEW CRpcReflectionService() )
{GUCEF_TRACE;

    RegisterEvents();
    memset( m_sessionId, 0, sizeof( m_sessionId ) );
    memset( m_unknownResumeId, 0, sizeof( m_unknownResumeId ) );
    memset( m_resumeToken, 0, sizeof( m_resumeToken ) );
    memset( m_pendingNonce, 0, sizeof( m_pendingNonce ) );
    m_link = CRpcEndpointLink::Create( this );
    m_durable = CRpcDurableRequestTable::Create( settings.durableResultRetentionMs, settings.maxDurableRequestsPerSubject );

    // handlers in the notification thread mode run when this internal event reaches the notification pulse generator
    typedef CORE::CTEventHandlerFunctor< CRpcEndpoint > TEventCallback;
    TEventCallback dispatchCallback( this, &CRpcEndpoint::OnDispatchToNotificationThread );
    m_selfObserver.SubscribeTo( this, DispatchToNotificationThreadEvent, dispatchCallback );

    // gucef.rpc is bound at index 0 in every session, in both directions
    SInBinding* rpc = GUCEF_NEW SInBinding();
    rpc->bindIndex = GUCEF_RPC_BINDINDEX_RPC;
    rpc->contract = m_rpcContract;
    rpc->ok = true;
    m_inBindings[ GUCEF_RPC_BINDINDEX_RPC ] = rpc;
    SOutBinding* rpcOut = GUCEF_NEW SOutBinding();
    rpcOut->bindIndex = GUCEF_RPC_BINDINDEX_RPC;
    rpcOut->name = CRpcBuiltInContracts::RpcContractName;
    rpcOut->major = 1;
    rpcOut->contract = m_rpcContract;
    rpcOut->providerMinor = 0;
    rpcOut->state = SOutBinding::STATE_BOUND;
    if ( !m_rpcContract.IsNULL() )
        rpcOut->providerHash = m_rpcContract->GetHash();
    m_outBindings[ GUCEF_RPC_BINDINDEX_RPC ] = rpcOut;
}

/*-------------------------------------------------------------------------*/

CRpcEndpoint::~CRpcEndpoint()
{GUCEF_TRACE;

    m_link->Detach();
    m_selfObserver.UnsubscribeAllFromObserver();
    m_transportObserver.UnsubscribeAllFromObserver();

    CLockScope lock( *this );
    DetachTransport( true );
    CompleteAllClientCalls( CRpcStatus( RPC_STATUS_CANCELLED, "the endpoint was destroyed" ) );
    CancelAllServerCalls( CRpcStatus( RPC_STATUS_CANCELLED, "the endpoint was destroyed" ) );
    m_serverCalls.clear();
    ResetBindings();
    TOutBindingMap::iterator o = m_outBindings.begin();
    for ( ; o != m_outBindings.end(); ++o )
        GUCEF_DELETE (*o).second;
    m_outBindings.clear();
    TInBindingMap::iterator in = m_inBindings.begin();
    for ( ; in != m_inBindings.end(); ++in )
        GUCEF_DELETE (*in).second;
    m_inBindings.clear();
    for ( UInt32 i=0; i<m_fragmenters.size(); ++i )
        GUCEF_DELETE m_fragmenters[ i ];
    m_fragmenters.clear();
    TAssemblyMap::iterator a = m_assemblies.begin();
    for ( ; a != m_assemblies.end(); ++a )
        GUCEF_DELETE (*a).second;
    m_assemblies.clear();
    m_assemblyBytes = 0;
    TPendingQueryMap::iterator q = m_pendingQueries.begin();
    for ( ; q != m_pendingQueries.end(); ++q )
    {
        (*q).second->future.Cancel();
        GUCEF_DELETE (*q).second;
    }
    m_pendingQueries.clear();
    m_orphanResults.clear();
    m_session.Reset();
    m_pendingHandlers.clear();     // the calls were cancelled above, their handlers do not run any more
    lock.EarlyUnlock();
    for ( UInt32 i=0; i<m_deferredEvents.size(); ++i )
    {
        // events nobody can get any more; the stop of the periodic pulses still has to reach the pulse generator
        if ( SDeferredEvent::KIND_PULSES_STOP == m_deferredEvents[ i ]->kind )
            RunPulseRequest( *m_deferredEvents[ i ] );
        GUCEF_DELETE m_deferredEvents[ i ];
    }
    m_deferredEvents.clear();
    if ( !m_protocolTask.IsNULL() )
        m_protocolTask->Stop();
    GUCEF_DELETE m_reflection;
    m_reflection = GUCEF_NULL;
    GUCEF_DELETE m_sink;
    m_sink = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcEndpoint::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcEndpoint";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RaiseEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // mailed to the notification pulse generator once the endpoint lock is released
    SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_ENDPOINT_EVENT );
    deferred->eventId = eventId;
    deferred->eventData = GUCEF_NULL != eventData ? eventData->Clone() : GUCEF_NULL;
    DeferEvent( deferred );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::LockState( void ) const
{GUCEF_TRACE;

    if ( MT::LOCKSTATUS_WAIT_TIMEOUT == m_lock.Lock() )
        throw timeout_exception();
    ++m_lockDepth;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::UnlockState( void ) const
{GUCEF_TRACE;

    // the outermost hold runs the handlers this thread dispatched, then raises what was deferred, unless another
    // thread is busy doing so: that one takes these too
    if ( 1 == m_lockDepth )
    {
        CRpcEndpoint* self = const_cast< CRpcEndpoint* >( this );
        if ( !m_pendingHandlers.empty() )
            self->RunPendingHandlers();
        if ( !m_raisingDeferredEvents && !m_deferredEvents.empty() )
            self->RaiseDeferredEvents();
    }
    --m_lockDepth;
    m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RunPendingHandlers( void )
{GUCEF_TRACE;

    // called holding the lock once: the handlers run without it, on the thread that dispatched them (the protocol
    // thread), in order. What they raise is deferred and goes out after them
    TThreadId self = MT::GetCurrentThreadID();
    while ( true )
    {
        GUCEF::vector< CRpcServerCallPtr > mine;
        for ( UInt32 i=0; i<m_pendingHandlers.size(); )
        {
            if ( m_pendingHandlers[ i ].threadId == self )
            {
                mine.push_back( m_pendingHandlers[ i ].call );
                m_pendingHandlers.erase( m_pendingHandlers.begin() + i );
            }
            else
                ++i;
        }
        if ( mine.empty() )
            return;

        ++m_handlerDepth;
        --m_lockDepth;
        m_lock.Unlock();
        for ( UInt32 i=0; i<mine.size(); ++i )
        {
            try
            {
                mine[ i ]->RunHandler();
            }
            catch ( const timeout_exception& )
            {
                GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: a lock wait timed out in a handler" );
            }
        }
        mine.clear();
        if ( MT::LOCKSTATUS_WAIT_TIMEOUT == m_lock.Lock() )
            throw timeout_exception();
        ++m_lockDepth;
        --m_handlerDepth;
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RaiseDeferredEvents( void )
{GUCEF_TRACE;

    // called holding the lock once: it is released while the events go out, in the order they were raised
    m_raisingDeferredEvents = true;
    while ( !m_deferredEvents.empty() )
    {
        TDeferredEventVector batch;
        batch.swap( m_deferredEvents );
        --m_lockDepth;
        m_lock.Unlock();
        for ( UInt32 i=0; i<batch.size(); ++i )
        {
            SDeferredEvent* deferred = batch[ i ];
            try
            {
                switch ( deferred->kind )
                {
                    case SDeferredEvent::KIND_ENDPOINT_EVENT:
                        NotifyObserversFromThread( deferred->eventId, deferred->eventData );
                        break;
                    case SDeferredEvent::KIND_STREAM_EVENT:
                        static_cast< CRpcClientStream* >( deferred->stream.GetPointerAlways() )->RaiseStreamEvent( deferred->eventId, deferred->eventData );
                        break;
                    case SDeferredEvent::KIND_CALL_EVENT:
                        deferred->call->RaiseCallEvent( deferred->eventId, deferred->eventData );
                        break;
                    case SDeferredEvent::KIND_DISPATCH:
                        RunDispatch( deferred->call, deferred->mode );
                        break;
                    case SDeferredEvent::KIND_PULSES_START:
                    case SDeferredEvent::KIND_PULSES_STOP:
                        RunPulseRequest( *deferred );
                        break;
                    case SDeferredEvent::KIND_DOWNGRADE_REFUSED:
                        deferred->transport->OnProtectionDowngradeRefused( deferred->required, deferred->arrivedWith );
                        break;
                }
            }
            catch ( const timeout_exception& )
            {
                GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: a lock wait timed out while an event was raised" );
            }
            GUCEF_DELETE deferred;
        }
        if ( MT::LOCKSTATUS_WAIT_TIMEOUT == m_lock.Lock() )
        {
            m_raisingDeferredEvents = false;
            throw timeout_exception();
        }
        ++m_lockDepth;
    }
    m_raisingDeferredEvents = false;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RunPulseRequest( const SDeferredEvent& request )
{GUCEF_TRACE;

    // outside the endpoint lock: a pulse generator may pulse inside a request (an immediate pulse of a driver that is
    // not looping), and the pulse takes the endpoint lock
    CORE::PulseGeneratorPtr pulseGenerator( request.pulseGenerator );
    if ( pulseGenerator.IsNULL() )
        return;
    if ( SDeferredEvent::KIND_PULSES_START == request.kind )
    {
        pulseGenerator->RequestPulseInterval( request.intervalMs );
        pulseGenerator->RequestPeriodicPulses( this );
    }
    else
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::DeferEvent( SDeferredEvent* deferred )
{GUCEF_TRACE;

    CLockScope lock( *this );
    m_deferredEvents.push_back( deferred );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RaiseCallEvent( const CRpcServerCallPtr& call, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( call.IsNULL() )
        return;
    SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_CALL_EVENT );
    deferred->call = call;
    deferred->eventId = eventId;
    deferred->eventData = GUCEF_NULL != eventData ? eventData->Clone() : GUCEF_NULL;
    DeferEvent( deferred );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StreamMessage( SClientCall& call, const CRpcMessage& message, UInt64 streamIndex )
{GUCEF_TRACE;

    // the message is queued in the stream now (ReadNext() sees it), MessageEvent follows
    if ( GUCEF_NULL == call.streamImpl )
        return;
    call.streamImpl->RecordMessage( message );
    SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_STREAM_EVENT );
    deferred->stream = call.stream;
    deferred->eventId = CRpcClientStream::InternalMessageEvent;
    deferred->eventData = GUCEF_NEW CRpcStreamMessageEventData( message, streamIndex );
    DeferEvent( deferred );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StreamEnd( const CIRpcStreamPtr& stream, const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    CRpcClientStream* impl = static_cast< CRpcClientStream* >( CIRpcStreamPtr( stream ).GetPointerAlways() );
    if ( GUCEF_NULL == impl || !impl->RecordEnd( status, response ) )
        return;
    SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_STREAM_EVENT );
    deferred->stream = stream;
    deferred->eventId = CIRpcStream::StatusEvent;
    deferred->eventData = GUCEF_NEW CRpcStatus( status );
    DeferEvent( deferred );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StreamWritable( const CIRpcStreamPtr& stream )
{GUCEF_TRACE;

    if ( stream.IsNULL() )
        return;
    SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_STREAM_EVENT );
    deferred->stream = stream;
    deferred->eventId = CIRpcStream::WritableEvent;
    DeferEvent( deferred );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                               CIRpcServiceImpl* impl, TRpcExecutionMode mode )
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_services->RegisterService( contract, serviceName, impl, mode );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SetServiceSet( const CRpcServiceSetPtr& services )
{GUCEF_TRACE;

    CLockScope lock( *this );
    m_services = services.IsNULL() ? CRpcServiceSet::Create() : services;
}

/*-------------------------------------------------------------------------*/

CRpcServiceSetPtr
CRpcEndpoint::GetServiceSet( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_services;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SetAccessPolicy( const CIRpcAccessPolicyPtr& policy )
{GUCEF_TRACE;

    CLockScope lock( *this );
    m_accessPolicy = policy;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SetDurableRequestTable( const CRpcDurableRequestTablePtr& table )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( !table.IsNULL() )
        m_durable = table;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SetHost( CRpcHost* host )
{GUCEF_TRACE;

    CLockScope lock( *this );
    m_host = host;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::AddWantedContract( const CORE::CString& name, UInt32 major, UInt32 minMinor )
{GUCEF_TRACE;

    CLockScope lock( *this );
    for ( UInt32 i=0; i<m_wants.size(); ++i )
        if ( m_wants[ i ].name == name && m_wants[ i ].major == major )
            return;
    SRpcContractWant want;
    want.name = name;
    want.major = major;
    want.minMinor = minMinor;
    m_wants.push_back( want );

    // wanted after the handshake: bound at once
    if ( RPC_SESSIONSTATE_ESTABLISHED == m_state )
        BindWant( (UInt32) m_wants.size() - 1 );
}

/*-------------------------------------------------------------------------*/

CRpcContractRegistry&
CRpcEndpoint::GetRegistry( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_settings.contractRegistry )
        return *m_settings.contractRegistry;
    return CComCoreGlobal::Instance()->GetRpcContractRegistry();
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::IsProtocolThread( void ) const
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator = m_protocolPulseGenerator;
    return !pulseGenerator.IsNULL() && pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::AttachTransport( const CIRpcTransportPtr& transportRef )
{GUCEF_TRACE;

    // the subscriptions were made by SubscribeTransport(), outside the endpoint lock: subscribing takes the
    // notifier's lock, which the protocol thread holds while it delivers an event that takes the endpoint lock
    CIRpcTransportPtr transport( transportRef );
    DetachTransport( false );
    m_transport = transport;
    m_protocolPulseGenerator = transport->GetPulseGenerator();
    if ( !m_protocolPulseGenerator.IsNULL() )
    {
        // asked for once the endpoint lock is released, see RunPulseRequest()
        SDeferredEvent* request = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_PULSES_START );
        request->pulseGenerator = m_protocolPulseGenerator;
        request->intervalMs = m_settings.tickIntervalMs;
        DeferEvent( request );
    }

    // the session follows the transport's nature and timing
    const CRpcTransportProperties& properties = transport->GetProperties();
    CRpcSession::SSettings sessionSettings;
    sessionSettings.reliableOrdered = properties.isReliable && properties.isOrdered;
    sessionSettings.receiveWindowBytes = m_settings.limits.receiveWindowBytes;
    sessionSettings.retentionMaxFrames = m_settings.retentionMaxFrames;
    sessionSettings.retentionMaxBytes = m_settings.retentionMaxBytes;
    sessionSettings.sendQueueMaxBytes = m_settings.sendQueueMaxBytes;
    sessionSettings.suspendedOverflow = m_settings.suspendedOverflow;
    if ( m_settings.useTransportProfile )
        sessionSettings.ApplyProfile( CRpcTransportProfile::GetProfile( properties.profileName ) );
    else
        sessionSettings.ApplyProfile( m_settings.profile );
    m_session.Configure( sessionSettings );
    m_lastReceiveMs = NowMs();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::DetachTransport( bool close )
{GUCEF_TRACE;

    // no unsubscribing here (the endpoint lock is held): events of a detached transport are ignored and the
    // subscriptions go at the next pulse (OnProtocolPulse) or with the endpoint
    if ( !m_protocolPulseGenerator.IsNULL() )
    {
        SDeferredEvent* request = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_PULSES_STOP );
        request->pulseGenerator = m_protocolPulseGenerator;
        DeferEvent( request );
    }
    m_session.SetTransportUp( false, NowMs() );
    if ( !m_transport.IsNULL() )
    {
        if ( close )
            m_transport->Close();
        m_staleTransports.push_back( m_transport );
        m_transport.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SubscribeTransport( const CIRpcTransportPtr& transportRef )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    if ( transport.IsNULL() )
        return;
    typedef CORE::CTEventHandlerFunctor< CRpcEndpoint > TEventCallback;
    TEventCallback connected( this, &CRpcEndpoint::OnTransportConnected );
    TEventCallback disconnected( this, &CRpcEndpoint::OnTransportDisconnected );
    TEventCallback frame( this, &CRpcEndpoint::OnTransportFrame );
    TEventCallback writable( this, &CRpcEndpoint::OnTransportWritable );
    CIRpcTransport* notifier = transport.GetPointerAlways();
    m_transportObserver.SubscribeTo( notifier, CIRpcTransport::ConnectedEvent, connected );
    m_transportObserver.SubscribeTo( notifier, CIRpcTransport::DisconnectedEvent, disconnected );
    m_transportObserver.SubscribeTo( notifier, CIRpcTransport::FrameReceivedEvent, frame );
    m_transportObserver.SubscribeTo( notifier, CIRpcTransport::WritableEvent, writable );
    CORE::PulseGeneratorPtr pulseGenerator = transport->GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
    {
        TEventCallback pulse( this, &CRpcEndpoint::OnProtocolPulse );
        m_transportObserver.SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::IsCurrentTransport( CORE::CNotifier* notifier ) const
{GUCEF_TRACE;

    return !m_transport.IsNULL() && static_cast< const CORE::CNotifier* >( m_transport.GetPointerAlways() ) == notifier;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::RefusesInsecureTransport( const CIRpcTransportPtr& transport ) const
{GUCEF_TRACE;

    // 14.1: every transport the session runs over, the first and every one it resumes over
    if ( !m_settings.requireSecureTransport || transport->GetProperties().isSecure )
        return false;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: refused a " << transport->GetTransportType() << " transport: the settings require a secure transport" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::Start( const CIRpcTransportPtr& transportRef, bool initiator )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    SubscribeTransport( transport );
    CLockScope lock( *this );
    if ( transport.IsNULL() || m_started )
        return false;
    if ( RefusesInsecureTransport( transport ) )
    {
        transport->Close();
        return false;
    }
    m_started = true;
    m_initiator = initiator;
    m_state = RPC_SESSIONSTATE_CONNECTING;
    AttachTransport( transport );
    if ( initiator && transport->IsConnected() )
    {
        m_session.SetTransportUp( true, NowMs() );
        SendHello();
    }
    else if ( !initiator )
    {
        // the acceptor takes nothing but HELLO before HELLO_ACK
        m_principal = transport->GetPeerPrincipal();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::Connect( const CORE::CUri& remote )
{GUCEF_TRACE;

    CIRpcTransportPtr transport = CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( remote.GetScheme() );
    if ( transport.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: no RPC transport of type \"" << remote.GetScheme() << "\"" );
        return false;
    }
    // a transport made here has no thread yet: the protocol runs on a dedicated task of the task manager (10.9).
    // A transport whose thread is given by its own environment refuses the task's pulse generator (drn: the node's
    // thread, which it learns in Connect()); it brings its own, checked once it connected
    if ( transport->GetPulseGenerator().IsNULL() )
    {
        CRpcProtocolTaskPtr task;
        bool startedHere = false;
        {
            CLockScope lock( *this );
            task = m_protocolTask;
        }
        if ( task.IsNULL() )
        {
            // started without the endpoint lock: the start may make the task's thread pool, which subscribes to a pulse
            // generator, and nothing subscribes under the endpoint lock (10.9)
            CRpcProtocolTaskPtr started = CRpcProtocolTask::Start( "gucef.rpc.protocol" );
            CRpcProtocolTaskPtr unused;
            {
                CLockScope lock( *this );
                if ( m_protocolTask.IsNULL() )
                {
                    m_protocolTask = started;
                    startedHere = true;
                }
                else
                    unused = started;       // another thread connected this endpoint meanwhile
                task = m_protocolTask;
            }
            if ( !unused.IsNULL() )
                unused->Stop();
        }
        if ( task.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: no protocol thread for the \"" << remote.GetScheme() << "\" transport" );
            return false;
        }
        if ( !transport->SetPulseGenerator( task->GetPulseGenerator() ) && startedHere )
        {
            // the task is not needed: the transport runs on a thread of its own environment
            CLockScope lock( *this );
            if ( m_protocolTask == task )
                m_protocolTask.Unlink();
            task->Stop();
        }
    }
    {
        CLockScope lock( *this );
        m_remoteUri = remote;
    }

    // the transport connects before the session starts on it: some learn their thread and peer only in Connect()
    // (drn). Its events are raised on its own thread later, never inside Connect(); Start() subscribes and also
    // looks at IsConnected(), so a ConnectedEvent that came before the subscription is not missed
    if ( !transport->Connect( remote ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: cannot connect to " << remote.ToUriInStringForm() );
        return false;
    }
    if ( transport->GetPulseGenerator().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: the \"" << remote.GetScheme() << "\" transport refused a protocol thread and has none of its own" );
        transport->Close();
        return false;
    }
    if ( !Start( transport, true ) )
    {
        transport->Close();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::Rebind( const CIRpcTransportPtr& transportRef )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    SubscribeTransport( transport );
    CLockScope lock( *this );
    if ( transport.IsNULL() || RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state )
        return false;
    if ( RefusesInsecureTransport( transport ) )
    {
        transport->Close();
        return false;
    }
    if ( !m_initiator )
    {
        // the acceptor waits for the resume HELLO on the new transport
        AttachTransport( transport );
        return true;
    }
    if ( RPC_SESSIONSTATE_ESTABLISHED == m_state )
        Suspend( CRpcStatus( RPC_STATUS_UNAVAILABLE, "rebound to a new transport" ) );
    AttachTransport( transport );
    if ( transport->IsConnected() )
    {
        m_session.SetTransportUp( false, NowMs() );
        SendHello();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnTransportConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( !m_initiator || !IsCurrentTransport( notifier ) )
        return;
    // only when the HELLO did not go out yet over this transport (Start() and Rebind() send it on a connected one)
    if ( ( RPC_SESSIONSTATE_CONNECTING == m_state && 0 == m_helloSentMs ) || ( RPC_SESSIONSTATE_SUSPENDED == m_state && !m_resumePending ) )
        SendHello();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnTransportDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( !IsCurrentTransport( notifier ) )
        return;
    CRpcStatus why( RPC_STATUS_UNAVAILABLE, "the transport closed" );
    if ( GUCEF_NULL != eventData )
        why = *static_cast< CRpcStatus* >( eventData );

    switch ( m_state )
    {
        case RPC_SESSIONSTATE_ESTABLISHED:
            Suspend( why );
            break;
        case RPC_SESSIONSTATE_CONNECTING:
            if ( m_hasIdentity )
                Suspend( why );     // a resume attempt that did not get through
            else
                EndSession( why, false );
            break;
        case RPC_SESSIONSTATE_CLOSING:
            EndSession( m_closeStatus.IsOk() ? CRpcStatus( RPC_STATUS_OK, "closed" ) : m_closeStatus, false );
            break;
        case RPC_SESSIONSTATE_SUSPENDED:
            m_session.SetTransportUp( false, NowMs() );
            break;
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnTransportWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( !IsCurrentTransport( notifier ) )
        return;
    m_session.OnTick( NowMs() );
    PumpFragments();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnTransportFrame( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CRpcFrameEventData* frame = static_cast< CRpcFrameEventData* >( eventData );
    CLockScope lock( *this );
    if ( RPC_SESSIONSTATE_CLOSED == m_state || !IsCurrentTransport( notifier ) )
        return;
    m_lastReceiveMs = NowMs();
    m_counters[ "framesReceived" ] += 1;

    // 14.3: only the payload frames of a call may arrive PUBLIC (their method's class is checked when they are
    // handled); any other frame that does is a downgrade, refused before the session layer applies its ack
    if ( RPC_PROTECTION_PUBLIC == frame->GetProtection() && frame->GetSize() > 0 )
    {
        UInt8 type = frame->GetFrame()[ 0 ];
        if ( GUCEF_RPC_FRAME_REQUEST != type && GUCEF_RPC_FRAME_RESPONSE != type && GUCEF_RPC_FRAME_RESULT != type &&
             GUCEF_RPC_FRAME_STREAM_MSG != type && GUCEF_RPC_FRAME_CONTINUATION != type && !CheckProtection( GUCEF_NULL, false, RPC_PROTECTION_PUBLIC ) )
            return;
    }
    m_session.OnFrameReceived( frame->GetFrame(), frame->GetSize(), frame->GetProtection(), m_lastReceiveMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnProtocolPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF::vector< CRpcServerCallPtr > retired;
    GUCEF::vector< CIRpcTransportPtr > stale;
    bool reconnect = false;
    CORE::CUri reconnectUri;
    {
        CLockScope lock( *this );
        Tick( NowMs() );
        retired.swap( m_retiredServerCalls );
        stale.swap( m_staleTransports );
        reconnect = m_reconnectDue;
        m_reconnectDue = false;
        if ( reconnect )
            reconnectUri = m_remoteUri;

        // a call whose events go to another thread may be in the delivery of one of them there right now: its last
        // reference is handed to that thread, which delivers the mail holding it after it is done with the call
        for ( UInt32 i=0; i<retired.size(); )
        {
            CORE::PulseGeneratorPtr eventPulseGenerator = retired[ i ]->GetEventPulseGenerator();
            if ( !eventPulseGenerator.IsNULL() && eventPulseGenerator != m_protocolPulseGenerator )
            {
                CRpcCallTaskData data( retired[ i ] );
                RaiseEvent( ReleaseOnNotificationThreadEvent, &data );
                retired.erase( retired.begin() + i );
            }
            else
                ++i;
        }
    }
    // outside the endpoint lock: the last references of ended server calls (not inside their own events) and the
    // subscriptions to detached transports (unless one of them is current again)
    retired.clear();
    for ( UInt32 i=0; i<stale.size(); ++i )
    {
        bool current = false;
        {
            CLockScope lock( *this );
            current = IsCurrentTransport( stale[ i ].GetPointerAlways() );
        }
        if ( !current )
            m_transportObserver.UnsubscribeFrom( *stale[ i ].GetPointerAlways() );
    }
    stale.clear();
    if ( reconnect )
        Reconnect( reconnectUri );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::SessionTransmit( const UInt8* frame, UInt32 size, TRpcProtectionClass protection )
{GUCEF_TRACE;

    if ( m_transport.IsNULL() )
        return false;
    if ( !m_transport->SendFrame( frame, size, protection ) )
        return false;
    m_lastSendMs = NowMs();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SessionDeliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    HandleFrame( frame, size, header, arrivedWith );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SessionProtocolViolation( const CRpcStatus& status )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: protocol violation by the peer: " << status.ToString() );
    EndSession( status, true );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SessionGap( UInt64 from, UInt64 to )
{GUCEF_TRACE;

    // on a reliable transport a gap is a transport fault: closing it suspends the session, the resume repairs it
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: frames " << from << " to " << to << " missing on a reliable transport, closing it so the resume repairs the session" );
    m_counters[ "gapsDetected" ] += 1;
    CRpcGapEventData gapData( from, to );
    RaiseEvent( GapDetectedEvent, &gapData );
    if ( !m_transport.IsNULL() )
        m_transport->Close();
    if ( RPC_SESSIONSTATE_ESTABLISHED == m_state )
        Suspend( CRpcStatus( RPC_STATUS_DATA_LOSS, "sequence gap on a reliable transport" ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SessionWritable( void )
{GUCEF_TRACE;

    PumpFragments();

    // writers that were refused for want of room in the send queue get WritableEvent (only those: this runs whenever
    // the queue drains, which is after nearly every frame)
    if ( m_queueBlockedStreams.empty() )
        return;
    GUCEF::set< UInt64 > blocked;
    blocked.swap( m_queueBlockedStreams );
    GUCEF::set< UInt64 >::iterator b = blocked.begin();
    for ( ; b != blocked.end(); ++b )
    {
        SClientCall* call = FindClientCall( *b );
        if ( GUCEF_NULL != call && GUCEF_NULL != call->streamImpl )
            StreamWritable( call->stream );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::FillLimits( SRpcLimits& limits ) const
{GUCEF_TRACE;

    limits = m_settings.limits;
    if ( !m_transport.IsNULL() )
    {
        UInt32 transportMax = m_transport->GetProperties().maxFrameSize;
        if ( 0 != transportMax && ( 0 == limits.maxFrameSize || transportMax < limits.maxFrameSize ) )
            limits.maxFrameSize = transportMax;
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::ApplyPeerLimits( const SRpcLimits& limits )
{GUCEF_TRACE;

    m_peerLimits = limits;
    m_session.SetPeerReceiveWindow( limits.receiveWindowBytes );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::GetOffersFor( const CRpcPrincipal& principal, const TRpcContractWantVector* filter, TRpcContractOfferVector& offers ) const
{GUCEF_TRACE;

    offers.clear();
    TRpcContractPtrVector exported;
    m_services->GetExportedContracts( exported );
    for ( UInt32 i=0; i<exported.size(); ++i )
    {
        const CRpcContract& contract = *exported[ i ];
        if ( GUCEF_NULL != filter )
        {
            bool wanted = false;
            for ( UInt32 w=0; w<filter->size() && !wanted; ++w )
                wanted = (*filter)[ w ].name == contract.GetName();
            if ( !wanted )
                continue;
        }
        if ( !m_accessPolicy.IsNULL() && !m_accessPolicy->MayBindContract( principal, contract ) )
            continue;
        SRpcContractOffer offer;
        offer.name = contract.GetName();
        offer.major = contract.GetMajor();
        offer.minor = contract.GetMinor();
        offer.hash = contract.GetHash();
        offers.push_back( offer );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::CreateSessionIdentity( UInt8* sessionId, UInt8* resumeToken )
{GUCEF_TRACE;

    // no randomness, no session: fail closed
    if ( !CORE::CRandom::GetRandomBytes( sessionId, GUCEF_RPC_SESSIONID_SIZE ) ||
         !CORE::CRandom::GetRandomBytes( resumeToken, GUCEF_RPC_RESUMETOKEN_SIZE ) )
        return false;
    return !IsZeroBytes( sessionId, GUCEF_RPC_SESSIONID_SIZE );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::EstablishNewSession( const UInt8* sessionId, const UInt8* resumeToken )
{GUCEF_TRACE;

    memcpy( m_sessionId, sessionId, GUCEF_RPC_SESSIONID_SIZE );
    memcpy( m_resumeToken, resumeToken, GUCEF_RPC_RESUMETOKEN_SIZE );
    m_hasIdentity = true;
    m_resumeCount = 0;

    // the first session of the endpoint names its lineage: the sessions that follow it after RESUME_GAP keep it
    if ( m_lineageOwner.IsNULLOrEmpty() )
        m_lineageOwner = "session:" + CORE::Base16Encode( sessionId, GUCEF_RPC_SESSIONID_SIZE );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcEndpoint::GetDurableOwner( void ) const
{GUCEF_TRACE;

    // an authenticated subject is the same party in every session; an unauthenticated one ("tcp:<address>:<port>")
    // names a connection, and another connection may get the same address and port later: its DURABLE ids belong to
    // the session lineage of this endpoint instead (10.7)
    if ( m_principal.isAuthenticated || m_lineageOwner.IsNULLOrEmpty() )
        return m_principal.subject;
    return m_lineageOwner;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::MayQueryDurable( const CORE::CString& ownerSubject, const CRpcRequestId& id ) const
{GUCEF_TRACE;

    // the owner may; another subject only when the access policy says so (10.7)
    if ( ownerSubject == GetDurableOwner() )
        return true;
    return !m_accessPolicy.IsNULL() && m_principal.isAuthenticated && m_accessPolicy->MayQueryRequest( m_principal, ownerSubject, id );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SendHello( void )
{GUCEF_TRACE;

    if ( m_transport.IsNULL() )
        return;

    SRpcHello hello;
    hello.nodeName = m_settings.nodeName;
    FillLimits( hello.limits );
    if ( m_hasIdentity )
    {
        // resume (8.2): the token never travels, an HMAC proves it
        hello.mode = GUCEF_RPC_HELLO_RESUME;
        memcpy( hello.sessionId, m_sessionId, GUCEF_RPC_SESSIONID_SIZE );
        hello.resumeCount = ++m_resumeCount;
        if ( !CORE::CRandom::GetRandomBytes( m_pendingNonce, GUCEF_RPC_NONCE_SIZE ) )
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "no randomness for the resume nonce" ), false );
            return;
        }
        memcpy( hello.nonce, m_pendingNonce, GUCEF_RPC_NONCE_SIZE );
        hello.lastReceivedSeq = m_session.GetDeliveredSeq();
        hello.oldestRetainedSeq = m_session.GetOldestRetainedSeq();
        hello.nextSeq = m_session.GetNextSeq();
        CRpcFrameCodec::ComputeResumeProof( m_resumeToken, CRpcFrameCodec::ResumeLabel, hello.sessionId, hello.resumeCount,
                                            hello.nonce, hello.lastReceivedSeq, hello.oldestRetainedSeq, hello.nextSeq, hello.proof );
        m_resumePending = true;
    }
    else
    {
        hello.mode = GUCEF_RPC_HELLO_NEW;
        GetOffersFor( m_transport->GetPeerPrincipal(), GUCEF_NULL, hello.offers );
        hello.wants = m_wants;
    }

    CORE::CDynamicBuffer frame;
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO, 0, 0, false, 0 );
    CRpcFrameCodec::WriteHello( writer, hello );

    // the state is set before the HELLO leaves: a transport that delivers at once brings HELLO_ACK back inside
    // SessionTransmit()
    m_helloSentMs = NowMs();
    if ( RPC_SESSIONSTATE_SUSPENDED != m_state )
        m_state = RPC_SESSIONSTATE_CONNECTING;
    SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::StartAccepted( const CIRpcTransportPtr& transportRef, const UInt8* helloFrame, UInt32 size, bool sessionUnknown )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    if ( !Start( transport, false ) )
        return false;
    CLockScope lock( *this );
    SRpcFrameHeader header;
    if ( !CRpcFrameCodec::ParseHeader( helloFrame, size, header ) || GUCEF_RPC_FRAME_HELLO != header.type )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "the first frame is not a HELLO" ), true );
        return false;
    }
    if ( !sessionUnknown )
    {
        HandleHello( helloFrame + header.bodyOffset, size - header.bodyOffset );
        return RPC_SESSIONSTATE_ESTABLISHED == m_state;
    }

    // a resume of a session this side no longer knows: a new session in the same round trip (8.4)
    CORE::CBinaryWireReader reader( helloFrame + header.bodyOffset, size - header.bodyOffset );
    SRpcHello hello;
    if ( !CRpcFrameCodec::ReadHello( reader, hello ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed HELLO" ), true );
        return false;
    }
    m_principal = transport->GetPeerPrincipal();
    m_principalPinned = true;
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt8 token[ GUCEF_RPC_RESUMETOKEN_SIZE ];
    SRpcHelloAck ack;
    ack.nodeName = m_settings.nodeName;
    if ( !CreateSessionIdentity( sessionId, token ) )
    {
        ack.result = GUCEF_RPC_HELLOACK_REFUSED;
        ack.refusal.Set( RPC_STATUS_UNAVAILABLE, "no randomness for a session identity" );
    }
    else
    {
        EstablishNewSession( sessionId, token );
        memcpy( m_unknownResumeId, hello.sessionId, GUCEF_RPC_SESSIONID_SIZE );
        m_hasUnknownResumeId = true;
        ack.result = GUCEF_RPC_HELLOACK_SESSION_UNKNOWN;
        memcpy( ack.sessionId, sessionId, GUCEF_RPC_SESSIONID_SIZE );
        memcpy( ack.resumeToken, token, GUCEF_RPC_RESUMETOKEN_SIZE );
        FillLimits( ack.limits );
        GetOffersFor( m_principal, GUCEF_NULL, ack.offers );
        ack.wants = m_wants;
        ApplyPeerLimits( hello.limits );
    }
    CORE::CDynamicBuffer frame;
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO_ACK, 0, 0, false, 0 );
    CRpcFrameCodec::WriteHelloAck( writer, ack );
    SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
    if ( GUCEF_RPC_HELLOACK_REFUSED == ack.result )
    {
        EndSession( ack.refusal, false );
        return false;
    }
    m_lastHelloAck = frame;
    m_session.SetTransportUp( true, NowMs() );
    m_state = RPC_SESSIONSTATE_ESTABLISHED;
    RaiseEvent( SessionEstablishedEvent, GUCEF_NULL );
    BindWants();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleHello( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    if ( m_initiator )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "HELLO sent to the initiator of the session" ), true );
        return;
    }
    CORE::CBinaryWireReader reader( body, size );
    SRpcHello hello;
    if ( !CRpcFrameCodec::ReadHello( reader, hello ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed HELLO" ), true );
        return;
    }

    if ( GUCEF_RPC_HELLO_RESUME == hello.mode && m_hasUnknownResumeId && m_lastHelloAck.GetDataSize() > 0 &&
         0 == memcmp( hello.sessionId, m_unknownResumeId, GUCEF_RPC_SESSIONID_SIZE ) )
    {
        // the resume this endpoint answered with SESSION_UNKNOWN, seen again (a retry, or the same frame reaching
        // this endpoint after the host handed it the transport): the same answer
        SessionTransmit( static_cast< const UInt8* >( m_lastHelloAck.GetConstBufferPtr() ), m_lastHelloAck.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
        return;
    }
    if ( GUCEF_RPC_HELLO_RESUME == hello.mode && m_hasIdentity && hello.resumeCount == m_resumeCount &&
         0 == memcmp( hello.sessionId, m_sessionId, GUCEF_RPC_SESSIONID_SIZE ) && RPC_SESSIONSTATE_ESTABLISHED == m_state )
    {
        // the resume this endpoint accepted, seen again (the host handed over the transport inside the frame's own
        // event): already answered; a lost answer makes the initiator try again with the next resume count
        return;
    }
    if ( GUCEF_RPC_HELLO_RESUME == hello.mode )
    {
        // a resume HELLO on a transport this endpoint already has (no host in between, or a half-open connection)
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO, 0, 0, false, 0 );
        writer.WriteBytes( body, size );
        AcceptResume( m_transport, static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize() );
        return;
    }

    if ( RPC_SESSIONSTATE_ESTABLISHED == m_state && m_lastHelloAck.GetDataSize() > 0 )
    {
        // the HELLO_ACK got lost on an unreliable transport: the same answer again
        SessionTransmit( static_cast< const UInt8* >( m_lastHelloAck.GetConstBufferPtr() ), m_lastHelloAck.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
        return;
    }
    if ( RPC_SESSIONSTATE_CONNECTING != m_state )
        return;

    SRpcHelloAck ack;
    ack.nodeName = m_settings.nodeName;
    m_principal = m_transport->GetPeerPrincipal();
    m_principalPinned = true;
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt8 token[ GUCEF_RPC_RESUMETOKEN_SIZE ];
    if ( GUCEF_RPC_PROTOCOL_MAJOR != hello.protocolMajor )
    {
        ack.result = GUCEF_RPC_HELLOACK_REFUSED;
        ack.refusal.Set( RPC_STATUS_FAILED_PRECONDITION, "protocol major " + CORE::ToString( (UInt32) hello.protocolMajor ) + " is not supported" );
    }
    else if ( !CreateSessionIdentity( sessionId, token ) )
    {
        ack.result = GUCEF_RPC_HELLOACK_REFUSED;
        ack.refusal.Set( RPC_STATUS_UNAVAILABLE, "no randomness for a session identity" );
    }
    else
    {
        EstablishNewSession( sessionId, token );
        ack.result = GUCEF_RPC_HELLOACK_NEW;
        memcpy( ack.sessionId, sessionId, GUCEF_RPC_SESSIONID_SIZE );
        memcpy( ack.resumeToken, token, GUCEF_RPC_RESUMETOKEN_SIZE );
        FillLimits( ack.limits );
        GetOffersFor( m_principal, &hello.wants, ack.offers );
        ack.wants = m_wants;
        m_peerNodeName = hello.nodeName;
        m_peerOffers = hello.offers;
        m_peerWants = hello.wants;
        ApplyPeerLimits( hello.limits );
    }

    CORE::CDynamicBuffer frame;
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO_ACK, 0, 0, false, 0 );
    CRpcFrameCodec::WriteHelloAck( writer, ack );
    SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
    if ( GUCEF_RPC_HELLOACK_REFUSED == ack.result )
    {
        EndSession( ack.refusal, false );
        return;
    }
    m_lastHelloAck = frame;
    m_session.SetTransportUp( true, NowMs() );
    m_state = RPC_SESSIONSTATE_ESTABLISHED;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session established with \"" << m_peerNodeName << "\" (" << m_principal.subject << ")" );
    RaiseEvent( SessionEstablishedEvent, GUCEF_NULL );
    BindWants();
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::AcceptResume( const CIRpcTransportPtr& transportRef, const UInt8* helloFrame, UInt32 size )
{GUCEF_TRACE;

    CIRpcTransportPtr transport( transportRef );
    CLockScope lock( *this );
    SRpcFrameHeader header;
    SRpcHello hello;
    bool parsed = CRpcFrameCodec::ParseHeader( helloFrame, size, header ) && GUCEF_RPC_FRAME_HELLO == header.type;
    if ( parsed )
    {
        CORE::CBinaryWireReader reader( helloFrame + header.bodyOffset, size - header.bodyOffset );
        parsed = CRpcFrameCodec::ReadHello( reader, hello ) && GUCEF_RPC_HELLO_RESUME == hello.mode;
    }

    SRpcHelloAck ack;
    ack.nodeName = m_settings.nodeName;
    ack.result = GUCEF_RPC_HELLOACK_REFUSED;

    // every check of 8.2; a refusal keeps the suspended session
    UInt8 expected[ GUCEF_RPC_PROOF_SIZE ];
    if ( !parsed )
        ack.refusal.Set( RPC_STATUS_INTERNAL, "malformed resume HELLO" );
    else if ( m_initiator || !m_hasIdentity || RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_CLOSING == m_state ||
              0 != memcmp( hello.sessionId, m_sessionId, GUCEF_RPC_SESSIONID_SIZE ) )
        ack.refusal.Set( RPC_STATUS_PERMISSION_DENIED, "unknown session" );
    else
    {
        CRpcFrameCodec::ComputeResumeProof( m_resumeToken, CRpcFrameCodec::ResumeLabel, hello.sessionId, hello.resumeCount,
                                            hello.nonce, hello.lastReceivedSeq, hello.oldestRetainedSeq, hello.nextSeq, expected );
        if ( !CORE::CSha256::ConstantTimeEquals( expected, hello.proof, GUCEF_RPC_PROOF_SIZE ) )
            ack.refusal.Set( RPC_STATUS_PERMISSION_DENIED, "the resume proof is not valid" );
        else if ( hello.resumeCount <= m_resumeCount )
            ack.refusal.Set( RPC_STATUS_PERMISSION_DENIED, "the resume count was used before" );
        else if ( !m_principal.MayResumeAs( transport->GetPeerPrincipal() ) )
            ack.refusal.Set( RPC_STATUS_PERMISSION_DENIED, "the principal does not match the session" );
        else if ( RefusesInsecureTransport( transport ) )
            ack.refusal.Set( RPC_STATUS_FAILED_PRECONDITION, "the settings require a secure transport" );
        else if ( hello.lastReceivedSeq >= m_session.GetNextSeq() )
            ack.refusal.Set( RPC_STATUS_INTERNAL, "the peer claims frames this side never sent" );
        else
            ack.result = GUCEF_RPC_HELLOACK_RESUMED;
    }

    if ( GUCEF_RPC_HELLOACK_REFUSED == ack.result )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: refused a resume: " << ack.refusal.ToString() );
        m_counters[ "refusedResumes" ] += 1;
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO_ACK, 0, 0, false, 0 );
        CRpcFrameCodec::WriteHelloAck( writer, ack );
        transport->SendFrame( frame.GetConstBufferPtr(), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
        if ( transport != m_transport )
            transport->Close();
        return false;
    }
    m_resumeCount = hello.resumeCount;

    // the new transport replaces the old one (also a half-open one that is still up)
    if ( transport != m_transport )
    {
        if ( !m_transport.IsNULL() )
            m_transport->Close();
        SubscribeTransport( transport );     // the host's protocol thread, inside the transport's own event
        AttachTransport( transport );
    }
    ApplyPeerLimits( hello.limits );

    bool canServeInitiator = m_session.GetOldestRetainedSeq() <= hello.lastReceivedSeq + 1;
    bool initiatorCanServe = hello.oldestRetainedSeq <= m_session.GetDeliveredSeq() + 1;
    CORE::CDynamicBuffer frame;
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO_ACK, 0, 0, false, 0 );
    if ( canServeInitiator && initiatorCanServe )
    {
        m_session.PrepareResume( hello.lastReceivedSeq, NowMs() );
        ack.result = GUCEF_RPC_HELLOACK_RESUMED;
        ack.lastReceivedSeq = m_session.GetDeliveredSeq();
        ack.oldestRetainedSeq = m_session.GetOldestRetainedSeq();
        ack.nextSeq = m_session.GetNextSeq();
        CRpcFrameCodec::ComputeResumeProof( m_resumeToken, CRpcFrameCodec::ResumedLabel, m_sessionId, hello.resumeCount, hello.nonce,
                                            ack.lastReceivedSeq, ack.oldestRetainedSeq, ack.nextSeq, ack.proof );
        FillLimits( ack.limits );
        CRpcFrameCodec::WriteHelloAck( writer, ack );
        SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
        m_session.SetTransportUp( true, NowMs() );
        m_state = RPC_SESSIONSTATE_ESTABLISHED;
        m_counters[ "resumes" ] += 1;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session resumed (acceptor)" );
        AnnounceAfterResume();
        RaiseEvent( SessionResumedEvent, GUCEF_NULL );
        return true;
    }

    // what was lost cannot be repaired: a new session in the same round trip (8.4)
    CRpcSessionGapEventData gap;
    if ( !canServeInitiator )
    {
        gap.gapToInitiatorFrom = hello.lastReceivedSeq + 1;
        gap.gapToInitiatorTo = m_session.GetOldestRetainedSeq() - 1;
    }
    if ( !initiatorCanServe )
    {
        gap.gapToAcceptorFrom = m_session.GetDeliveredSeq() + 1;
        gap.gapToAcceptorTo = hello.oldestRetainedSeq - 1;
    }
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt8 token[ GUCEF_RPC_RESUMETOKEN_SIZE ];
    if ( !CreateSessionIdentity( sessionId, token ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_UNAVAILABLE, "no randomness for a session identity" ), true );
        return false;
    }
    StartNewSessionAfterGap( gap, RPC_STATUS_RESUME_GAP );
    EstablishNewSession( sessionId, token );
    ack.result = GUCEF_RPC_HELLOACK_RESUME_GAP;
    memcpy( ack.sessionId, sessionId, GUCEF_RPC_SESSIONID_SIZE );
    memcpy( ack.resumeToken, token, GUCEF_RPC_RESUMETOKEN_SIZE );
    FillLimits( ack.limits );
    GetOffersFor( m_principal, GUCEF_NULL, ack.offers );
    ack.wants = m_wants;
    ack.gapToInitiatorFrom = gap.gapToInitiatorFrom;
    ack.gapToInitiatorTo = gap.gapToInitiatorTo;
    ack.gapToAcceptorFrom = gap.gapToAcceptorFrom;
    ack.gapToAcceptorTo = gap.gapToAcceptorTo;
    CRpcFrameCodec::WriteHelloAck( writer, ack );
    SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
    m_lastHelloAck.Clear( true );
    m_session.SetTransportUp( true, NowMs() );
    m_state = RPC_SESSIONSTATE_ESTABLISHED;
    RaiseEvent( SessionGapEvent, &gap );
    BindWants();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleHelloAck( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    if ( !m_initiator )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "HELLO_ACK sent to the acceptor of the session" ), true );
        return;
    }
    CORE::CBinaryWireReader reader( body, size );
    SRpcHelloAck ack;
    if ( !CRpcFrameCodec::ReadHelloAck( reader, ack ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed HELLO_ACK" ), true );
        return;
    }

    switch ( ack.result )
    {
        case GUCEF_RPC_HELLOACK_REFUSED:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: the peer refused the session: " << ack.refusal.ToString() );
            m_resumePending = false;
            EndSession( ack.refusal, false );
            return;
        }
        case GUCEF_RPC_HELLOACK_NEW:
        {
            if ( m_hasIdentity )
                return;     // a duplicate on an unreliable transport
            EstablishNewSession( ack.sessionId, ack.resumeToken );
            m_peerNodeName = ack.nodeName;
            m_peerOffers = ack.offers;
            m_peerWants = ack.wants;
            ApplyPeerLimits( ack.limits );
            m_principal = m_transport->GetPeerPrincipal();
            m_principalPinned = true;
            m_session.SetTransportUp( true, NowMs() );
            m_state = RPC_SESSIONSTATE_ESTABLISHED;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session established with \"" << m_peerNodeName << "\"" );
            RaiseEvent( SessionEstablishedEvent, GUCEF_NULL );
            BindWants();
            return;
        }
        case GUCEF_RPC_HELLOACK_RESUMED:
        {
            if ( !m_resumePending )
                return;
            UInt8 expected[ GUCEF_RPC_PROOF_SIZE ];
            CRpcFrameCodec::ComputeResumeProof( m_resumeToken, CRpcFrameCodec::ResumedLabel, m_sessionId, m_resumeCount, m_pendingNonce,
                                                ack.lastReceivedSeq, ack.oldestRetainedSeq, ack.nextSeq, expected );
            if ( !CORE::CSha256::ConstantTimeEquals( expected, ack.proof, GUCEF_RPC_PROOF_SIZE ) )
            {
                EndSession( CRpcStatus( RPC_STATUS_PERMISSION_DENIED, "the acceptor's resume proof is not valid" ), true );
                return;
            }
            if ( !m_session.PrepareResume( ack.lastReceivedSeq, NowMs() ) )
            {
                EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "the acceptor claims frames this side never sent" ), true );
                return;
            }
            m_resumePending = false;
            ApplyPeerLimits( ack.limits );
            m_session.SetTransportUp( true, NowMs() );
            m_state = RPC_SESSIONSTATE_ESTABLISHED;
            m_reconnectDelayMs = m_settings.reconnectInitialDelayMs;
            m_counters[ "resumes" ] += 1;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session resumed (initiator)" );

            AnnounceAfterResume();
            RaiseEvent( SessionResumedEvent, GUCEF_NULL );

            // calls made while suspended go out now
            GUCEF::vector< SClientCall* > waiting;
            waiting.swap( m_waitingCalls );
            for ( UInt32 w=0; w<waiting.size(); ++w )
            {
                CRpcStatus refusal;
                if ( !IssueCall( *waiting[ w ], GUCEF_NULL, refusal ) && 0 != waiting[ w ]->callRef )
                    CompleteClientCall( waiting[ w ]->callRef, refusal, GUCEF_NULL, 0, false );
            }
            return;
        }
        case GUCEF_RPC_HELLOACK_RESUME_GAP:
        case GUCEF_RPC_HELLOACK_SESSION_UNKNOWN:
        {
            if ( !m_resumePending )
                return;
            m_resumePending = false;
            CRpcSessionGapEventData gap;
            gap.gapToInitiatorFrom = ack.gapToInitiatorFrom;
            gap.gapToInitiatorTo = ack.gapToInitiatorTo;
            gap.gapToAcceptorFrom = ack.gapToAcceptorFrom;
            gap.gapToAcceptorTo = ack.gapToAcceptorTo;
            gap.sessionUnknown = GUCEF_RPC_HELLOACK_SESSION_UNKNOWN == ack.result;
            StartNewSessionAfterGap( gap, gap.sessionUnknown ? RPC_STATUS_UNAVAILABLE : RPC_STATUS_RESUME_GAP );
            EstablishNewSession( ack.sessionId, ack.resumeToken );
            m_peerNodeName = ack.nodeName;
            m_peerOffers = ack.offers;
            m_peerWants = ack.wants;
            ApplyPeerLimits( ack.limits );
            m_principal = m_transport->GetPeerPrincipal();
            m_session.SetTransportUp( true, NowMs() );
            m_state = RPC_SESSIONSTATE_ESTABLISHED;
            m_reconnectDelayMs = m_settings.reconnectInitialDelayMs;
            RaiseEvent( SessionGapEvent, &gap );
            BindWants();
            return;
        }
        default:
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "unknown HELLO_ACK result" ), true );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::ResetBindings( void )
{GUCEF_TRACE;

    TOutBindingMap::iterator o = m_outBindings.begin();
    while ( o != m_outBindings.end() )
    {
        if ( GUCEF_RPC_BINDINDEX_RPC != (*o).first )
        {
            GUCEF_DELETE (*o).second;
            m_outBindings.erase( o++ );
        }
        else
            ++o;
    }
    TInBindingMap::iterator in = m_inBindings.begin();
    while ( in != m_inBindings.end() )
    {
        if ( GUCEF_RPC_BINDINDEX_RPC != (*in).first )
        {
            GUCEF_DELETE (*in).second;
            m_inBindings.erase( in++ );
        }
        else
            ++in;
    }
    m_nextBindIndex = 1;
    m_getContractCount = 0;
    m_outKeys.Clear();
    m_inKeys.Clear();
    ++m_bindGeneration;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::BindWants( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_wants.size(); ++i )
        BindWant( i );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::BindWant( UInt32 wantIndex )
{GUCEF_TRACE;

    const SRpcContractWant& want = m_wants[ wantIndex ];
    TOutBindingMap::iterator i = m_outBindings.begin();
    for ( ; i != m_outBindings.end(); ++i )
        if ( (*i).second->name == want.name && (*i).second->major == want.major && SOutBinding::STATE_FAILED != (*i).second->state )
            return;

    if ( 0 != m_peerLimits.maxBindings && m_outBindings.size() > m_peerLimits.maxBindings )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: the peer's binding limit keeps " << want.name << " unbound" );
        return;
    }

    SOutBinding* binding = GUCEF_NEW SOutBinding();
    binding->bindIndex = m_nextBindIndex++;
    binding->name = want.name;
    binding->major = want.major;
    binding->minMinor = want.minMinor;
    m_outBindings[ binding->bindIndex ] = binding;

    // the offer of the peer: highest minor of the name and major
    const SRpcContractOffer* offer = GUCEF_NULL;
    for ( UInt32 o=0; o<m_peerOffers.size(); ++o )
    {
        const SRpcContractOffer& candidate = m_peerOffers[ o ];
        if ( candidate.name == want.name && candidate.major == want.major && ( GUCEF_NULL == offer || candidate.minor > offer->minor ) )
            offer = &candidate;
    }

    // the contract this side encodes with (6.4.3)
    CRpcContractRegistry& registry = GetRegistry();
    CRpcContractPtr contract;
    if ( m_settings.preferPeerContracts && GUCEF_NULL != offer )
        contract = registry.FindByHash( offer->hash );
    else
    {
        contract = registry.FindAtLeast( want.name, want.major, want.minMinor );
        if ( contract.IsNULL() )
            contract = m_services->FindExported( want.name, want.major );
        if ( contract.IsNULL() && GUCEF_NULL != offer )
            contract = registry.FindByHash( offer->hash );
    }

    // both sides know every offer of the peer that it may bind (HELLO carries all, HELLO_ACK those wanted): what is
    // not offered is not provided, refused here without a BIND on the wire (3.8)
    if ( GUCEF_NULL == offer )
    {
        FailBinding( *binding, CRpcStatus( RPC_STATUS_NOT_FOUND, "the peer does not offer " + want.name + " " + CORE::ToString( want.major ) ) );
        return;
    }
    binding->providerMinor = offer->minor;
    binding->providerHash = offer->hash;

    if ( !contract.IsNULL() )
    {
        binding->contract = contract;
        SendBind( *binding );
        return;
    }

    // neither compiled in nor cached: fetched from the peer once, then cached
    if ( 0 != m_settings.maxGetContractPerSession && m_getContractCount >= m_settings.maxGetContractPerSession )
    {
        FailBinding( *binding, CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "too many contract fetches in this session" ) );
        return;
    }
    ++m_getContractCount;
    binding->state = SOutBinding::STATE_FETCHING;
    binding->fetchHash = offer->hash;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    CRpcFrameCodec::WriteHash( writer, offer->hash );
    writer.WriteUInt8( 1 );     // with documentation
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( !SendSessionFrame( GUCEF_RPC_FRAME_GET_CONTRACT, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal ) )
        FailBinding( *binding, refusal );
    else
        m_counters[ "contractFetches" ] += 1;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SendBind( SOutBinding& binding )
{GUCEF_TRACE;

    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt32( binding.bindIndex );
    writer.WriteString( binding.contract->GetName() );
    writer.WriteVarUInt32( binding.contract->GetMajor() );
    writer.WriteVarUInt32( binding.contract->GetMinor() );
    CRpcFrameCodec::WriteHash( writer, binding.contract->GetHash() );
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( !SendSessionFrame( GUCEF_RPC_FRAME_BIND, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal ) )
    {
        FailBinding( binding, refusal );
        return;
    }
    binding.state = SOutBinding::STATE_SENT;

    // calls that waited for this binding follow the BIND in seq order
    GUCEF::vector< SClientCall* > waiting;
    for ( UInt32 i=0; i<m_waitingCalls.size(); )
    {
        if ( m_waitingCalls[ i ]->ref.bindIndex == binding.bindIndex )
        {
            waiting.push_back( m_waitingCalls[ i ] );
            m_waitingCalls.erase( m_waitingCalls.begin() + i );
        }
        else
            ++i;
    }
    for ( UInt32 w=0; w<waiting.size(); ++w )
    {
        CRpcStatus callRefusal;
        if ( !IssueCall( *waiting[ w ], GUCEF_NULL, callRefusal ) && 0 != waiting[ w ]->callRef )
            CompleteClientCall( waiting[ w ]->callRef, callRefusal, GUCEF_NULL, 0, false );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::FailBinding( SOutBinding& binding, const CRpcStatus& status )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: binding " << binding.name << " " << binding.major << " failed: " << status.ToString() );
    binding.state = SOutBinding::STATE_FAILED;
    binding.failStatus = status;
    CRpcBindingEventData data;
    data.contractName = binding.name;
    data.major = binding.major;
    data.localMinor = binding.contract.IsNULL() ? 0 : binding.contract->GetMinor();
    data.providerMinor = binding.providerMinor;
    data.bindIndex = binding.bindIndex;
    data.providerHash = binding.providerHash;
    data.status = status;
    RaiseEvent( ContractBoundEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleGetContract( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    CRpcContractHash hash;
    UInt8 flags = 0;
    if ( !CRpcFrameCodec::ReadHash( reader, hash ) || !reader.ReadUInt8( flags ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed GET_CONTRACT" ), true );
        return;
    }
    CRpcContractPtr contract = m_services->FindExported( hash );
    if ( contract.IsNULL() && !m_rpcContract.IsNULL() && m_rpcContract->GetHash() == hash )
        contract = m_rpcContract;
    CRpcStatus status;
    if ( contract.IsNULL() )
        status.Set( RPC_STATUS_NOT_FOUND, "no exported contract with hash " + hash.ToHexString() );
    else if ( !m_accessPolicy.IsNULL() && !m_accessPolicy->MayBindContract( m_principal, *contract ) )
    {
        status.Set( RPC_STATUS_PERMISSION_DENIED, "not allowed" );
        contract.Unlink();
    }
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    CRpcFrameCodec::WriteHash( writer, hash );
    CRpcFrameCodec::WriteStatus( writer, status );
    if ( !contract.IsNULL() )
    {
        writer.WriteSizedBytes( contract->GetSchema() );
        if ( 0 != ( flags & 1 ) && contract->HasDocs() )
            writer.WriteSizedBytes( contract->GetDocs() );
        else
            writer.WriteVarUInt32( 0 );
    }
    else
    {
        writer.WriteVarUInt32( 0 );
        writer.WriteVarUInt32( 0 );
    }
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_CONTRACT, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleContract( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    CRpcContractHash hash;
    CRpcStatus status;
    const UInt8* schema = GUCEF_NULL;
    UInt32 schemaSize = 0;
    const UInt8* docs = GUCEF_NULL;
    UInt32 docsSize = 0;
    if ( !CRpcFrameCodec::ReadHash( reader, hash ) || !CRpcFrameCodec::ReadStatus( reader, status ) ||
         !reader.ReadSizedBytes( schema, schemaSize ) || !reader.ReadSizedBytes( docs, docsSize ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed CONTRACT" ), true );
        return;
    }

    // only a contract this side asked for with GET_CONTRACT is taken: anything else is neither registered nor cached
    GUCEF::vector< SOutBinding* > fetching;
    TOutBindingMap::iterator i = m_outBindings.begin();
    for ( ; i != m_outBindings.end(); ++i )
        if ( SOutBinding::STATE_FETCHING == (*i).second->state && (*i).second->fetchHash == hash )
            fetching.push_back( (*i).second );
    if ( fetching.empty() )
    {
        m_counters[ "unsolicitedContracts" ] += 1;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: dropped a CONTRACT nothing asked for (hash " << hash.ToHexString() << ")" );
        return;
    }

    CRpcContractPtr contract;
    if ( status.IsOk() )
    {
        // never trust a schema that is not what was asked for
        if ( CRpcContractHash::Compute( schema, schemaSize ) != hash )
            status.Set( RPC_STATUS_DATA_LOSS, "the received schema does not match its hash" );
        else
        {
            CORE::CString error;
            if ( !GetRegistry().RegisterFromSchema( schema, schemaSize, CRpcContractRegistry::CONTRACTSOURCE_PEER, &contract, error,
                                                    docsSize > 0 ? docs : GUCEF_NULL, docsSize ) )
                status.Set( RPC_STATUS_INVALID_ARGUMENT, "the received contract is not valid: " + error );
        }
    }

    for ( UInt32 b=0; b<fetching.size(); ++b )
    {
        if ( !status.IsOk() || contract.IsNULL() )
            FailBinding( *fetching[ b ], status.IsOk() ? CRpcStatus( RPC_STATUS_INTERNAL, "no contract" ) : status );
        else
        {
            fetching[ b ]->contract = contract;
            SendBind( *fetching[ b ] );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleBind( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt32 bindIndex = 0;
    CORE::CString name;
    UInt32 major = 0;
    UInt32 minor = 0;
    CRpcContractHash hash;
    if ( !reader.ReadVarUInt32( bindIndex ) || !reader.ReadString( name ) || !reader.ReadVarUInt32( major ) ||
         !reader.ReadVarUInt32( minor ) || !CRpcFrameCodec::ReadHash( reader, hash ) || GUCEF_RPC_BINDINDEX_RPC == bindIndex )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed BIND" ), true );
        return;
    }

    // the provider binds with its own contract: by hash first, then by name and major
    CRpcStatus status;
    CRpcContractPtr contract = m_services->FindExported( hash );
    if ( contract.IsNULL() )
        contract = m_services->FindExported( name, major );
    if ( contract.IsNULL() )
    {
        if ( m_services->ExportsName( name ) )
            status.Set( RPC_STATUS_FAILED_PRECONDITION, name + " " + CORE::ToString( major ) + " is not exported: another major is" );
        else
            status.Set( RPC_STATUS_NOT_FOUND, name + " is not exported" );
    }
    else if ( !m_accessPolicy.IsNULL() && !m_accessPolicy->MayBindContract( m_principal, *contract ) )
        status.Set( RPC_STATUS_PERMISSION_DENIED, "binding " + name + " is not allowed" );

    // every index the peer assigned takes an entry, a refused one too (its REQUESTs are answered with its reason):
    // a peer beyond the maxBindings it was told is a protocol violation (14.4)
    TInBindingMap::iterator existing = m_inBindings.find( bindIndex );
    if ( existing == m_inBindings.end() && 0 != m_settings.limits.maxBindings && GetPeerBindingCount() >= m_settings.limits.maxBindings )
    {
        EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "the peer bound more than the " + CORE::ToString( m_settings.limits.maxBindings ) +
                                " contracts allowed" ), true );
        return;
    }

    SInBinding* binding = GUCEF_NEW SInBinding();
    binding->bindIndex = bindIndex;
    binding->ok = status.IsOk();
    binding->failStatus = status;
    if ( status.IsOk() )
        binding->contract = contract;
    if ( existing != m_inBindings.end() )
    {
        GUCEF_DELETE (*existing).second;
        m_inBindings.erase( existing );
    }
    m_inBindings[ bindIndex ] = binding;

    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt32( bindIndex );
    CRpcFrameCodec::WriteStatus( writer, status );
    writer.WriteVarUInt32( contract.IsNULL() ? 0 : contract->GetMinor() );
    CRpcFrameCodec::WriteHash( writer, contract.IsNULL() ? CRpcContractHash() : contract->GetHash() );
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_BIND_ACK, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleBindAck( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt32 bindIndex = 0;
    CRpcStatus status;
    UInt32 providerMinor = 0;
    CRpcContractHash providerHash;
    if ( !reader.ReadVarUInt32( bindIndex ) || !CRpcFrameCodec::ReadStatus( reader, status ) ||
         !reader.ReadVarUInt32( providerMinor ) || !CRpcFrameCodec::ReadHash( reader, providerHash ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed BIND_ACK" ), true );
        return;
    }
    TOutBindingMap::iterator i = m_outBindings.find( bindIndex );
    if ( i == m_outBindings.end() )
        return;
    SOutBinding& binding = *(*i).second;
    if ( !status.IsOk() )
    {
        FailBinding( binding, status );
        return;
    }
    binding.state = SOutBinding::STATE_BOUND;
    binding.providerMinor = providerMinor;
    binding.providerHash = providerHash;
    CRpcBindingEventData data;
    data.contractName = binding.name;
    data.major = binding.major;
    data.localMinor = binding.contract.IsNULL() ? 0 : binding.contract->GetMinor();
    data.providerMinor = providerMinor;
    data.bindIndex = bindIndex;
    data.providerHash = providerHash;
    RaiseEvent( ContractBoundEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::IsContractBound( const CORE::CString& name, UInt32 major ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    TOutBindingMap::const_iterator i = m_outBindings.begin();
    for ( ; i != m_outBindings.end(); ++i )
    {
        const SOutBinding& b = *(*i).second;
        if ( b.name == name && b.major == major && ( SOutBinding::STATE_SENT == b.state || SOutBinding::STATE_BOUND == b.state ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcEndpoint::GetPeerContractMinor( const CORE::CString& name, UInt32 major ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    TOutBindingMap::const_iterator i = m_outBindings.begin();
    for ( ; i != m_outBindings.end(); ++i )
        if ( (*i).second->name == name && (*i).second->major == major )
            return (*i).second->providerMinor;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::ResolveMethod( const CORE::CString& methodPath, CRpcMethodRef& ref, CRpcStatus& status ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    Int32 slash = methodPath.HasChar( '/' );
    Int32 dot = slash > 0 ? methodPath.SubstrToIndex( (UInt32) slash, true ).HasChar( '.', false ) : -1;
    if ( slash <= 0 || dot <= 0 )
    {
        status.Set( RPC_STATUS_INVALID_ARGUMENT, "expected \"<contract>.<Service>/<Method>\", got \"" + methodPath + "\"" );
        return false;
    }
    CORE::CString serviceName = methodPath.SubstrToIndex( (UInt32) slash, true );
    CORE::CString contractName = serviceName.SubstrToIndex( (UInt32) dot, true );
    CORE::CString methodName = methodPath.SubstrToIndex( (UInt32) slash + 1, false );

    const SOutBinding* binding = GUCEF_NULL;
    const SOutBinding* failed = GUCEF_NULL;
    TOutBindingMap::const_iterator i = m_outBindings.begin();
    for ( ; i != m_outBindings.end(); ++i )
    {
        const SOutBinding& b = *(*i).second;
        if ( b.name != contractName )
            continue;
        if ( SOutBinding::STATE_SENT == b.state || SOutBinding::STATE_BOUND == b.state )
        {
            binding = &b;
            break;
        }
        if ( SOutBinding::STATE_FAILED == b.state )
            failed = &b;
    }
    if ( GUCEF_NULL == binding )
    {
        if ( GUCEF_NULL != failed )
            status.Set( RPC_STATUS_FAILED_PRECONDITION, contractName + " is not bound: " + failed->failStatus.ToString() );
        else
            status.Set( RPC_STATUS_FAILED_PRECONDITION, contractName + " is not bound (yet)" );
        return false;
    }
    const CRpcServiceDescriptor* service = binding->contract->FindService( serviceName );
    const CRpcMethodDescriptor* method = GUCEF_NULL != service ? service->FindMethodByName( methodName ) : GUCEF_NULL;
    if ( GUCEF_NULL == method )
    {
        status.Set( RPC_STATUS_NOT_FOUND, "no method " + methodPath + " in " + binding->contract->GetDisplayName() );
        return false;
    }
    ref.contract = binding->contract;
    ref.method = method;
    ref.bindIndex = binding->bindIndex;
    ref.serviceNumber = service->GetNumber();
    ref.methodNumber = method->GetNumber();
    ref.bindGeneration = m_bindGeneration;
    ref.path = method->GetPath();
    status = CRpcStatus::Ok;
    return true;
}

/*-------------------------------------------------------------------------*/

CRpcMessage
CRpcEndpoint::NewRequest( const CRpcMethodRef& ref ) const
{GUCEF_TRACE;

    if ( !ref.IsValid() )
        return CRpcMessage();
    return CRpcMessage( ref.contract, ref.method->GetRequestType() );
}

/*-------------------------------------------------------------------------*/

TRpcProtectionClass
CRpcEndpoint::ProtectionForFrame( UInt8 type, const CRpcMethodDescriptor* method, bool responder ) const
{GUCEF_TRACE;

    // 14.3: call payload frames take the class of their method, everything else is CONFIDENTIAL
    if ( GUCEF_NULL == method )
        return RPC_PROTECTION_CONFIDENTIAL;
    switch ( type )
    {
        case GUCEF_RPC_FRAME_REQUEST:
            return method->GetRequestProtection();
        case GUCEF_RPC_FRAME_STREAM_MSG:
            return responder ? method->GetProtection() : method->GetRequestProtection();
        case GUCEF_RPC_FRAME_RESPONSE:
        case GUCEF_RPC_FRAME_RESULT:
            return method->GetProtection();
        default:
            return RPC_PROTECTION_CONFIDENTIAL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::CheckProtection( const CRpcMethodDescriptor* method, bool providerFrame, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    // only a transport that honours classes per frame reports a weaker class; on it only an attacker produces one
    if ( m_transport.IsNULL() || !m_transport->GetProperties().hasFrameProtectionClasses )
        return true;
    TRpcProtectionClass required = RPC_PROTECTION_CONFIDENTIAL;
    if ( GUCEF_NULL != method )
        required = providerFrame ? method->GetProtection() : method->GetRequestProtection();
    if ( RPC_PROTECTION_CONFIDENTIAL == required && RPC_PROTECTION_PUBLIC == arrivedWith )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint: a frame that must be CONFIDENTIAL arrived as PUBLIC: downgrade, ending the session" );
        m_counters[ "protectionDowngrades" ] += 1;
        SDeferredEvent* notice = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_DOWNGRADE_REFUSED );
        notice->transport = m_transport;
        notice->required = required;
        notice->arrivedWith = arrivedWith;
        DeferEvent( notice );
        EndSession( CRpcStatus( RPC_STATUS_PERMISSION_DENIED, "protection class downgrade" ), true );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleFrame( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    const UInt8* body = frame + header.bodyOffset;
    UInt32 bodySize = size - header.bodyOffset;

    // traces carry type, seq and size only, never payloads (14.5)
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "RpcEndpoint(" << m_settings.nodeName << "): " << RpcFrameTypeToString( header.type ) <<
                     " seq " << header.seq << " flags " << (UInt32) header.flags << " size " << size );

    // before the handshake only HELLO and HELLO_ACK are taken (6.4.2)
    if ( RPC_SESSIONSTATE_CONNECTING == m_state && GUCEF_RPC_FRAME_HELLO != header.type && GUCEF_RPC_FRAME_HELLO_ACK != header.type &&
         !m_hasIdentity )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "frame before the handshake" ), true );
        return;
    }

    switch ( header.type )
    {
        case GUCEF_RPC_FRAME_HELLO: HandleHello( body, bodySize ); return;
        case GUCEF_RPC_FRAME_HELLO_ACK: HandleHelloAck( body, bodySize ); return;
        case GUCEF_RPC_FRAME_PING: HandlePing( body, bodySize ); return;
        case GUCEF_RPC_FRAME_PONG: HandlePong( body, bodySize ); return;
        case GUCEF_RPC_FRAME_STREAM_CREDIT: HandleStreamCredit( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_ABORT: HandleAbort( body, bodySize ); return;
        default: break;
    }

    // call frames check their method's class themselves; the others must be CONFIDENTIAL
    if ( GUCEF_RPC_FRAME_REQUEST != header.type && GUCEF_RPC_FRAME_RESPONSE != header.type && GUCEF_RPC_FRAME_RESULT != header.type &&
         GUCEF_RPC_FRAME_STREAM_MSG != header.type && GUCEF_RPC_FRAME_CONTINUATION != header.type &&
         !CheckProtection( GUCEF_NULL, false, arrivedWith ) )
        return;

    switch ( header.type )
    {
        case GUCEF_RPC_FRAME_GET_CONTRACT: HandleGetContract( body, bodySize ); return;
        case GUCEF_RPC_FRAME_CONTRACT: HandleContract( body, bodySize ); return;
        case GUCEF_RPC_FRAME_BIND: HandleBind( body, bodySize ); return;
        case GUCEF_RPC_FRAME_BIND_ACK: HandleBindAck( body, bodySize ); return;
        case GUCEF_RPC_FRAME_REQUEST: HandleRequest( header, body, bodySize, arrivedWith ); return;
        case GUCEF_RPC_FRAME_RESPONSE: HandleResponse( header, body, bodySize, arrivedWith ); return;
        case GUCEF_RPC_FRAME_ACCEPTED: HandleAccepted( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_RESULT: HandleResult( header, body, bodySize, arrivedWith ); return;
        case GUCEF_RPC_FRAME_STREAM_MSG: HandleStreamMsg( header, body, bodySize, arrivedWith ); return;
        case GUCEF_RPC_FRAME_HALF_CLOSE: HandleHalfClose( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_CANCEL: HandleCancel( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_STATUS_QUERY: HandleStatusQuery( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_STATUS: HandleStatus( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_CONTINUATION: HandleContinuation( header, body, bodySize, arrivedWith ); return;
        case GUCEF_RPC_FRAME_ERROR: HandleError( header, body, bodySize ); return;
        case GUCEF_RPC_FRAME_GOODBYE: HandleGoodbye( header, body, bodySize ); return;
        default:
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "unknown frame type " + CORE::ToString( (UInt32) header.type ) ), true );
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::SendControl( UInt8 type, UInt8 flags, const CORE::CDynamicBuffer& body )
{GUCEF_TRACE;

    return m_session.SendControlFrame( type, flags, body.GetConstBufferPtr(), body.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SendAbort( const CRpcStatus& status )
{GUCEF_TRACE;

    if ( m_transport.IsNULL() )
        return;
    CORE::CDynamicBuffer frame;
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_ABORT, 0, 0, false, 0 );
    CRpcFrameCodec::WriteStatus( writer, status );
    SessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SendErrorFrame( UInt64 replyToSeq, const CRpcStatus& status )
{GUCEF_TRACE;

    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( replyToSeq );
    CRpcFrameCodec::WriteStatus( writer, status );
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_ERROR, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::AnnounceCredits( void )
{GUCEF_TRACE;

    // the current absolute limit of every open stream this side receives on; the limit counts as granted whether or
    // not its last STREAM_CREDIT went out, as this repeats it
    TClientCallMap::iterator c = m_clientCalls.begin();
    for ( ; c != m_clientCalls.end(); ++c )
        if ( GUCEF_NULL != (*c).second->streamImpl && (*c).second->grantedLimit > 0 && 0 != (*c).second->callRef )
            SendStreamCredit( (*c).second->callRef, false, (*c).second->grantedLimit );
    TServerCallMap::iterator s = m_serverCalls.begin();
    for ( ; s != m_serverCalls.end(); ++s )
        if ( (*s).second->m_method->HasClientStream() && (*s).second->m_grantedLimit > 0 )
            SendStreamCredit( (*s).first, true, (*s).second->m_grantedLimit );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::AnnounceAfterResume( void )
{GUCEF_TRACE;

    // 8.3: credits are absolute and unsequenced, and a STREAM_CREDIT that went into the dying link or was not sent
    // while suspended is lost; both sides announce their limits and their receive window again once the transport is up
    AnnounceCredits();
    m_session.SendAck( NowMs() );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::SendStreamCredit( UInt64 callRef, bool responder, UInt64 limit )
{GUCEF_TRACE;

    // written into a buffer of the endpoint: the session copies it into the frame before anything is transmitted, so a
    // nested control frame may reuse it (15: no allocation per control frame)
    m_controlBody.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( m_controlBody );
    writer.WriteVarUInt64( callRef );
    writer.WriteVarUInt64( limit );
    SendControl( GUCEF_RPC_FRAME_STREAM_CREDIT, responder ? GUCEF_RPC_FRAMEFLAG_RESPONDER : 0, m_controlBody );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::SendSessionFrame( UInt8 type, UInt8 flags, const CORE::CDynamicBuffer& fields, const void* payload, UInt32 payloadSize,
                                TRpcProtectionClass protection, UInt64& seq, CRpcStatus& refusal, UInt64 orderKey )
{GUCEF_TRACE;

    if ( RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state )
    {
        refusal.Set( RPC_STATUS_UNAVAILABLE, "the session is closed" );
        return false;
    }

    // a frame of a call whose earlier message is still going out in fragments waits behind them
    if ( 0 != orderKey )
    {
        SFragmenter* pending = FindFragmenter( orderKey );
        if ( GUCEF_NULL != pending )
        {
            SFragmenter::SFollower follower;
            pending->followers.push_back( follower );
            SFragmenter::SFollower& queued = pending->followers.back();
            queued.type = type;
            queued.flags = flags;
            queued.fields = fields;
            if ( payloadSize > 0 )
                queued.payload.CopyFrom( payloadSize, payload );
            queued.protection = protection;
            seq = 0;
            return true;
        }
    }

    UInt32 maxFrame = m_peerLimits.maxFrameSize;
    if ( !m_transport.IsNULL() )
    {
        UInt32 transportMax = m_transport->GetProperties().maxFrameSize;
        if ( 0 == maxFrame || ( 0 != transportMax && transportMax < maxFrame ) )
            maxFrame = transportMax;
    }
    if ( 0 == maxFrame )
        maxFrame = 16384;

    UInt32 fieldsSize = fields.GetDataSize();
    if ( fieldsSize + payloadSize + g_frameHeaderReserve <= maxFrame )
    {
        m_scratch.SetDataSize( 0 );
        CORE::CBinaryWireWriter writer( m_scratch );
        writer.WriteBytes( fields.GetConstBufferPtr(), fieldsSize );
        writer.WriteBytes( payload, payloadSize );
        return m_session.QueueFrame( type, flags, m_scratch.GetConstBufferPtr(), m_scratch.GetDataSize(), protection, NowMs(), seq, refusal );
    }

    // fragmentation (6.10): the first frame carries the fields, the total payload size and the first chunk
    if ( 0 != m_peerLimits.maxMessageSize && payloadSize > m_peerLimits.maxMessageSize )
    {
        refusal.Set( RPC_STATUS_RESOURCE_EXHAUSTED, "the message is larger than the peer's maximum of " + CORE::ToString( m_peerLimits.maxMessageSize ) + " bytes" );
        return false;
    }
    UInt32 totalSize = CORE::CBinaryWireWriter::GetVarUInt32Size( payloadSize );
    if ( maxFrame < g_frameHeaderReserve + fieldsSize + totalSize + 16 )
    {
        refusal.Set( RPC_STATUS_INTERNAL, "the frame size limit leaves no room for a payload" );
        return false;
    }
    UInt32 firstChunk = maxFrame - g_frameHeaderReserve - fieldsSize - totalSize;
    m_scratch.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( m_scratch );
    writer.WriteBytes( fields.GetConstBufferPtr(), fieldsSize );
    writer.WriteVarUInt32( payloadSize );
    writer.WriteBytes( payload, firstChunk );
    if ( !m_session.QueueFrame( type, (UInt8) ( flags | GUCEF_RPC_FRAMEFLAG_MORE ), m_scratch.GetConstBufferPtr(), m_scratch.GetDataSize(),
                                protection, NowMs(), seq, refusal ) )
        return false;

    SFragmenter* fragmenter = GUCEF_NEW SFragmenter();
    fragmenter->startSeq = seq;
    fragmenter->orderKey = orderKey;
    fragmenter->payload.CopyFrom( payloadSize, payload );
    fragmenter->offset = firstChunk;
    fragmenter->protection = protection;
    m_fragmenters.push_back( fragmenter );
    m_counters[ "fragmentedMessages" ] += 1;
    PumpFragments();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::PumpFragments( void )
{GUCEF_TRACE;

    // queueing a chunk may report the session writable, which pumps again: one pump at a time
    if ( m_fragmenters.empty() || !m_session.IsTransportUp() || m_pumpingFragments )
        return;
    m_pumpingFragments = true;
    UInt32 maxFrame = m_peerLimits.maxFrameSize;
    if ( !m_transport.IsNULL() )
    {
        UInt32 transportMax = m_transport->GetProperties().maxFrameSize;
        if ( 0 == maxFrame || ( 0 != transportMax && transportMax < maxFrame ) )
            maxFrame = transportMax;
    }
    if ( 0 == maxFrame )
        maxFrame = 16384;
    UInt32 chunkRoom = maxFrame - g_frameHeaderReserve - 20;

    // round robin between messages, a few frames ahead of the acknowledgements, so other frames are not held up
    // behind a large message (6.10): the next chunks follow as the peer acknowledges (Tick())
    UInt32 budget = 4 * maxFrame < 65536 ? 65536 : 4 * maxFrame;
    while ( !m_fragmenters.empty() && m_session.GetQueuedBytes() + m_session.GetInFlightBytes() < budget )
    {
        if ( m_fragmenterCursor >= m_fragmenters.size() )
            m_fragmenterCursor = 0;
        SFragmenter* fragmenter = m_fragmenters[ m_fragmenterCursor ];
        UInt32 remaining = fragmenter->payload.GetDataSize() - fragmenter->offset;
        UInt32 chunk = remaining < chunkRoom ? remaining : chunkRoom;
        bool last = chunk == remaining;

        m_scratch.SetDataSize( 0 );
        CORE::CBinaryWireWriter writer( m_scratch );
        writer.WriteVarUInt64( fragmenter->startSeq );
        writer.WriteVarUInt32( fragmenter->offset );
        writer.WriteBytes( fragmenter->payload.GetConstBufferPtr( fragmenter->offset ), chunk );
        UInt64 seq = 0;
        CRpcStatus refusal;
        if ( !m_session.QueueFrame( GUCEF_RPC_FRAME_CONTINUATION, last ? 0 : GUCEF_RPC_FRAMEFLAG_MORE, m_scratch.GetConstBufferPtr(),
                                    m_scratch.GetDataSize(), fragmenter->protection, NowMs(), seq, refusal ) )
            break;
        fragmenter->offset += chunk;
        if ( last )
        {
            m_fragmenters.erase( m_fragmenters.begin() + m_fragmenterCursor );

            // the frames that waited behind the message go now, in order (a large one fragments in turn and the
            // rest wait behind it)
            for ( UInt32 f=0; f<fragmenter->followers.size(); ++f )
            {
                SFragmenter::SFollower& follower = fragmenter->followers[ f ];
                UInt64 followerSeq = 0;
                CRpcStatus followerRefusal;
                SendSessionFrame( follower.type, follower.flags, follower.fields, follower.payload.GetConstBufferPtr(),
                                  follower.payload.GetDataSize(), follower.protection, followerSeq, followerRefusal, fragmenter->orderKey );
            }
            GUCEF_DELETE fragmenter;
        }
        else
            ++m_fragmenterCursor;
    }
    m_pumpingFragments = false;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcEndpoint::CallOrderKey( UInt64 callRef, bool responder )
{GUCEF_TRACE;

    return ( callRef << 1 ) | ( responder ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

CRpcEndpoint::SFragmenter*
CRpcEndpoint::FindFragmenter( UInt64 orderKey ) const
{GUCEF_TRACE;

    // the last one of the call: frames queue behind the latest message
    for ( UInt32 i=(UInt32) m_fragmenters.size(); i>0; --i )
        if ( m_fragmenters[ i-1 ]->orderKey == orderKey )
            return m_fragmenters[ i-1 ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::StartAssembly( const SRpcFrameHeader& header, const UInt8* frame, UInt32 prefixSize, UInt64 total,
                             const UInt8* chunk, UInt32 chunkSize, TRpcProtectionClass arrivedWith, const SRequestInfo* requestInfo )
{GUCEF_TRACE;

    if ( 0 != m_settings.maxOpenAssemblies && m_assemblies.size() >= m_settings.maxOpenAssemblies )
    {
        EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "too many fragmented messages at once" ), true );
        return false;
    }
    if ( ( 0 != m_settings.limits.maxMessageSize && total > m_settings.limits.maxMessageSize ) || total > 0xFFFFFFFFULL || chunkSize > total )
    {
        EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "fragmented message larger than the maximum message size" ), true );
        return false;
    }

    // the announced sizes of the open messages together stay within the receive window, or within one message of the
    // maximum size where that is larger (6.10); 0 for both means no maximum
    UInt64 assemblyLimit = m_settings.limits.receiveWindowBytes;
    if ( 0 == m_settings.limits.maxMessageSize )
        assemblyLimit = 0;
    else if ( m_settings.limits.maxMessageSize > assemblyLimit )
        assemblyLimit = m_settings.limits.maxMessageSize;
    if ( 0 != assemblyLimit && m_assemblyBytes + total > assemblyLimit )
    {
        EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "fragmented messages beyond the receive window" ), true );
        return false;
    }

    // the buffer grows with the chunks that arrive, not with the size the first frame announces
    SAssembly* assembly = GUCEF_NEW SAssembly();
    assembly->startSeq = header.seq;
    assembly->frame.CopyFrom( prefixSize, frame );
    static_cast< UInt8* >( assembly->frame.GetBufferPtr() )[ 1 ] &= (UInt8) ~GUCEF_RPC_FRAMEFLAG_MORE;
    assembly->frame.Append( chunk, chunkSize );
    assembly->prefixSize = prefixSize;
    assembly->total = total;
    assembly->received = chunkSize;
    assembly->protection = arrivedWith;
    if ( GUCEF_NULL != requestInfo )
    {
        assembly->isRequest = true;
        assembly->request = *requestInfo;
    }
    m_assemblies[ header.seq ] = assembly;
    m_assemblyBytes += total;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleContinuation( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 startSeq = 0;
    UInt32 offset = 0;
    if ( !reader.ReadVarUInt64( startSeq ) || !reader.ReadVarUInt32( offset ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed CONTINUATION" ), true );
        return;
    }
    TAssemblyMap::iterator i = m_assemblies.find( startSeq );
    if ( i == m_assemblies.end() )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "CONTINUATION of an unknown message" ), true );
        return;
    }
    SAssembly* assembly = (*i).second;
    UInt32 chunkSize = reader.GetRemaining();
    if ( offset != assembly->received || assembly->received + chunkSize > assembly->total )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "CONTINUATION out of order or beyond the announced size" ), true );
        return;
    }
    if ( arrivedWith != assembly->protection && RPC_PROTECTION_PUBLIC == arrivedWith && !CheckProtection( GUCEF_NULL, false, arrivedWith ) )
        return;
    assembly->frame.Append( reader.GetCurrentPtr(), chunkSize );
    assembly->received += chunkSize;
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_MORE ) )
        return;
    if ( assembly->received != assembly->total )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "fragmented message shorter than announced" ), true );
        return;
    }

    m_assemblies.erase( i );
    m_assemblyBytes -= assembly->total;
    if ( assembly->isRequest )
    {
        ProcessRequest( assembly->request, static_cast< const UInt8* >( assembly->frame.GetConstBufferPtr() ) + assembly->prefixSize,
                        (UInt32) assembly->total, assembly->protection );
    }
    else
    {
        SRpcFrameHeader joinedHeader;
        const UInt8* joined = static_cast< const UInt8* >( assembly->frame.GetConstBufferPtr() );
        if ( CRpcFrameCodec::ParseHeader( joined, assembly->frame.GetDataSize(), joinedHeader ) )
            HandleFrame( joined, assembly->frame.GetDataSize(), joinedHeader, assembly->protection );
    }
    GUCEF_DELETE assembly;
}

/*-------------------------------------------------------------------------*/

CRpcEndpoint::SClientCall*
CRpcEndpoint::FindClientCall( UInt64 callRef )
{GUCEF_TRACE;

    TClientCallMap::iterator i = m_clientCalls.find( callRef );
    return i != m_clientCalls.end() ? (*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::IssueCall( SClientCall& call, const CRpcMessage* request, CRpcStatus& refusal )
{GUCEF_TRACE;

    // a ref of an earlier binding round resolves again by its path
    if ( call.ref.bindGeneration != m_bindGeneration )
    {
        CRpcMethodRef fresh;
        if ( !ResolveMethod( call.ref.path, fresh, refusal ) )
            return false;
        call.ref = fresh;
    }
    const CRpcMethodDescriptor& method = *call.ref.method;

    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt32( call.ref.bindIndex );
    writer.WriteVarUInt32( call.ref.serviceNumber );
    writer.WriteVarUInt32( call.ref.methodNumber );
    writer.WriteVarUInt64( call.deadlineUnixMs );
    writer.WriteVarUInt32( call.options.hasResumeFromIndex ? GUCEF_RPC_CALLFLAG_RESUME_FROM : 0 );
    if ( call.options.hasResumeFromIndex )
        writer.WriteVarUInt64( call.options.resumeFromIndex );

    // keys the metadata defines are interned by the peer only when it receives this REQUEST: a REQUEST that is not
    // queued takes them back out, else the next call would use an index the peer never learned (6.9)
    const UInt32 internedBefore = m_outKeys.GetCount();
    if ( !CRpcFrameCodec::WriteMetadata( writer, call.options.metadata, m_outKeys ) )
    {
        m_outKeys.Truncate( internedBefore );
        refusal.Set( RPC_STATUS_INVALID_ARGUMENT, "metadata beyond 64 entries or 8 KB" );
        return false;
    }
    UInt8 flags = call.hasRequest ? 0 : GUCEF_RPC_FRAMEFLAG_NO_MESSAGE;

    // known before the frame leaves: an answer delivered while the frame is still being sent finds its call
    UInt64 seq = m_session.GetNextSeq();
    call.callRef = seq;
    call.sendCreditLimit = 0 != method.GetInitialCredit() ? method.GetInitialCredit() : m_peerLimits.initialStreamCredit;
    call.initialCredit = 0 != method.GetInitialCredit() ? method.GetInitialCredit() : m_settings.limits.initialStreamCredit;
    call.grantedLimit = call.initialCredit;
    m_clientCalls[ seq ] = &call;
    if ( GUCEF_NULL != call.streamImpl )
    {
        MT::CScopeMutex streamLock( call.streamImpl->m_lock );
        call.streamImpl->m_callRef = seq;
        call.streamImpl->m_sessionGeneration = m_sessionGeneration;
    }
    UInt64 sentSeq = 0;
    if ( !SendSessionFrame( GUCEF_RPC_FRAME_REQUEST, flags, fields, call.requestBytes.GetConstBufferPtr(), call.requestBytes.GetDataSize(),
                            ProtectionForFrame( GUCEF_RPC_FRAME_REQUEST, &method, false ), sentSeq, refusal, CallOrderKey( seq, false ) ) )
    {
        m_outKeys.Truncate( internedBefore );
        m_clientCalls.erase( seq );
        call.callRef = 0;
        return false;
    }
    m_counters[ "callsStarted" ] += 1;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRpcEndpoint::CallAsync( const CORE::CString& methodPath, const CRpcMessage& request, const CRpcCallOptions& options )
{GUCEF_TRACE;

    CRpcMethodRef ref;
    CRpcStatus status;
    if ( !ResolveMethod( methodPath, ref, status ) )
    {
        CORE::CFutureResultSource source;
        CRpcCallResult result;
        result.status = status;
        source.Complete( result.Clone(), true );
        return source.GetFuture();
    }
    return CallAsync( ref, request, options );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRpcEndpoint::CallAsync( const CRpcMethodRef& ref, const CRpcMessage& request, const CRpcCallOptions& options )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    CRpcCallResult local;

    CLockScope lock( *this );
    if ( !ref.IsValid() )
        local.status.Set( RPC_STATUS_INVALID_ARGUMENT, "the method reference is not resolved" );
    else if ( ref.method->IsStreaming() )
        local.status.Set( RPC_STATUS_FAILED_PRECONDITION, ref.path + " is a streaming method: use OpenStream()" );
    else if ( RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state || RPC_SESSIONSTATE_CLOSING == m_state )
        local.status.Set( RPC_STATUS_UNAVAILABLE, "the session is not open" );
    else if ( ( RPC_SESSIONSTATE_SUSPENDED == m_state || ( RPC_SESSIONSTATE_CONNECTING == m_state && m_hasIdentity ) ) && !options.waitForReady )
        local.status.Set( RPC_STATUS_UNAVAILABLE, "the session is suspended" );
    if ( !local.status.IsOk() )
    {
        source.Complete( local.Clone(), true );
        return source.GetFuture();
    }

    // fail fast for what the provider cannot do (6.5.4)
    SOutBinding* binding = GUCEF_NULL;
    TOutBindingMap::iterator b = m_outBindings.find( ref.bindIndex );
    if ( ref.bindGeneration == m_bindGeneration && b != m_outBindings.end() )
        binding = (*b).second;
    if ( GUCEF_NULL != binding && SOutBinding::STATE_FAILED == binding->state )
        local.status.Set( RPC_STATUS_FAILED_PRECONDITION, binding->name + " is not bound: " + binding->failStatus.ToString() );
    else if ( GUCEF_NULL != binding && ref.method->GetSince() > binding->providerMinor )
        local.status.Set( RPC_STATUS_UNIMPLEMENTED, ref.path + " is newer (minor " + CORE::ToString( ref.method->GetSince() ) +
                                                    ") than the provider's minor " + CORE::ToString( binding->providerMinor ) );
    if ( !local.status.IsOk() )
    {
        m_counters[ "callsFailedLocally" ] += 1;
        source.Complete( local.Clone(), true );
        return source.GetFuture();
    }

    SClientCall* call = GUCEF_NEW SClientCall();
    call->ref = ref;
    call->options = options;
    call->future = source;
    call->hasFuture = true;
    call->hasRequest = true;
    if ( !request.Encode( call->requestBytes ) )
    {
        GUCEF_DELETE call;
        local.status.Set( RPC_STATUS_INVALID_ARGUMENT, "the request cannot be encoded" );
        source.Complete( local.Clone(), true );
        return source.GetFuture();
    }
    UInt32 deadline = 0 != options.deadlineMs ? options.deadlineMs : ref.method->GetDefaultDeadlineMs();
    if ( 0 != deadline )
    {
        call->deadlineMs = NowMs() + deadline;
        call->deadlineUnixMs = WallClockMs() + deadline;
    }

    if ( GUCEF_NULL == binding && ref.bindGeneration == m_bindGeneration )
    {
        // the binding is being fetched: the call follows its BIND
        m_waitingCalls.push_back( call );
        return source.GetFuture();
    }
    CRpcStatus refusal;
    if ( !IssueCall( *call, &request, refusal ) )
    {
        local.status = refusal;
        GUCEF_DELETE call;
        source.Complete( local.Clone(), true );
        return source.GetFuture();
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::TryGetCallResult( const CORE::CFutureResult& future, CRpcCallResult& result )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return false;
    const CRpcCallResult* data = static_cast< const CRpcCallResult* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        result.status.Set( RPC_STATUS_CANCELLED, "the call was cancelled" );
        return true;
    }
    result = *data;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::TryGetRequestStatus( const CORE::CFutureResult& future, CRpcRequestStatus& requestStatus )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return false;
    const CRpcRequestStatus* data = static_cast< const CRpcRequestStatus* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        requestStatus.status.Set( RPC_STATUS_CANCELLED, "cancelled" );
        return true;
    }
    requestStatus = *data;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::WaitForFuture( const CORE::CFutureResult& future, const char* what, CRpcStatus& status )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator;
    bool nested = false;
    {
        CLockScope lock( *this );
        pulseGenerator = m_protocolPulseGenerator;
        nested = m_handlerDepth > 0 || m_blockingDepth > 0;
    }
    TThreadId driver = pulseGenerator.IsNULL() ? 0 : pulseGenerator->GetPulseDriverThreadId();
    TThreadId self = MT::GetCurrentThreadID();
    if ( driver == self )
    {
        // the work runs on this thread: pump its pulse generator until the answer (10.9)
        if ( nested )
        {
            status.Set( RPC_STATUS_FAILED_PRECONDITION, CORE::CString( what ) + " refused: a nested blocking call on the protocol thread" );
            return false;
        }
        {
            CLockScope lock( *this );
            ++m_blockingDepth;
        }
        UInt32 idle = 0;
        while ( !future.IsReady() )
        {
            pulseGenerator->ForceDirectPulse();
            if ( future.IsReady() )
                break;
            if ( ++idle > 16 )
                MT::PrecisionDelay( 1 );
        }
        CLockScope lock( *this );
        --m_blockingDepth;
        return true;
    }
    if ( 0 == driver )
    {
        status.Set( RPC_STATUS_FAILED_PRECONDITION, CORE::CString( what ) + " refused: it is not known which thread drives the protocol pulse generator" );
        return false;
    }

    // another thread runs the protocol: wait, pump nothing
    while ( !future.IsReady() )
        MT::PrecisionDelay( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::Call( const CRpcMethodRef& ref, const CRpcMessage& request, CRpcMessage& response,
                    CRpcStatus& status, const CRpcCallOptions& options )
{GUCEF_TRACE;

    {
        CLockScope lock( *this );
        if ( IsProtocolThread() && m_handlerDepth > 0 )
        {
            status.Set( RPC_STATUS_FAILED_PRECONDITION, "Call() refused: a nested blocking call inside a handler on the protocol thread" );
            return false;
        }
    }
    CORE::CFutureResult future = CallAsync( ref, request, options );
    if ( !WaitForFuture( future, "Call()", status ) )
    {
        future.RequestCancellation();
        return false;
    }
    CRpcCallResult result;
    if ( !TryGetCallResult( future, result ) )
    {
        status.Set( RPC_STATUS_INTERNAL, "no call result" );
        return false;
    }
    status = result.status;
    response = result.response;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::Call( const CORE::CString& methodPath, const CRpcMessage& request, CRpcMessage& response,
                    CRpcStatus& status, const CRpcCallOptions& options )
{GUCEF_TRACE;

    CRpcMethodRef ref;
    if ( !ResolveMethod( methodPath, ref, status ) )
        return false;
    return Call( ref, request, response, status, options );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::CompleteClientCall( UInt64 callRef, const CRpcStatus& status, const UInt8* payload, UInt32 payloadSize, bool sendCancel )
{GUCEF_TRACE;

    TClientCallMap::iterator i = m_clientCalls.find( callRef );
    if ( i == m_clientCalls.end() )
        return;
    SClientCall* call = (*i).second;
    m_clientCalls.erase( i );
    if ( call->requestId.IsSet() )
        m_acceptedIds.erase( call->requestId );

    if ( sendCancel && ( RPC_SESSIONSTATE_ESTABLISHED == m_state || RPC_SESSIONSTATE_SUSPENDED == m_state ) )
    {
        CORE::CDynamicBuffer fields;
        CORE::CBinaryWireWriter writer( fields );
        writer.WriteUInt8( 0 );
        writer.WriteVarUInt64( callRef );
        writer.WriteVarUInt32( (UInt32) status.code );
        writer.WriteString( status.message );
        UInt64 seq = 0;
        CRpcStatus refusal;
        SendSessionFrame( GUCEF_RPC_FRAME_CANCEL, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal, CallOrderKey( callRef, false ) );
    }

    CRpcCallResult result;
    result.status = status;
    result.wasAccepted = call->accepted;
    result.requestId = call->requestId;
    result.isDurable = call->durable;
    if ( status.IsOk() && GUCEF_NULL != payload && call->ref.IsValid() )
    {
        const CRpcTypeDescriptor* responseType = call->ref.method->GetResponseType();
        bool decodeResponse = !call->ref.method->HasServerStream();
        if ( decodeResponse )
        {
            result.response.Bind( call->ref.contract, responseType );
            CRpcStatus decodeStatus;
            if ( !result.response.Decode( payload, payloadSize, decodeStatus ) )
                result.status = decodeStatus;
        }
    }
    m_counters[ CORE::CString( "callsEnded." ) + RpcStatusCodeToString( result.status.code ) ] += 1;

    if ( GUCEF_NULL != call->streamImpl )
        StreamEnd( call->stream, result.status, call->ref.method->HasServerStream() ? GUCEF_NULL : &result.response );
    else if ( call->hasFuture )
        call->future.Complete( result.Clone(), true );
    GUCEF_DELETE call;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::CompleteAllClientCalls( const CRpcStatus& status )
{GUCEF_TRACE;

    while ( !m_clientCalls.empty() )
        CompleteClientCall( (*m_clientCalls.begin()).first, status, GUCEF_NULL, 0, false );
    for ( UInt32 i=0; i<m_waitingCalls.size(); ++i )
    {
        SClientCall* call = m_waitingCalls[ i ];
        CRpcCallResult result;
        result.status = status;
        if ( GUCEF_NULL != call->streamImpl )
            StreamEnd( call->stream, status, GUCEF_NULL );
        else if ( call->hasFuture )
            call->future.Complete( result.Clone(), true );
        GUCEF_DELETE call;
    }
    m_waitingCalls.clear();
    m_acceptedIds.clear();
    TPendingQueryMap::iterator q = m_pendingQueries.begin();
    for ( ; q != m_pendingQueries.end(); ++q )
    {
        CRpcRequestStatus requestStatus;
        requestStatus.requestId = (*q).second->id;
        requestStatus.status = status;
        (*q).second->future.Complete( requestStatus.Clone(), true );
        GUCEF_DELETE (*q).second;
    }
    m_pendingQueries.clear();
    m_orphanResults.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleResponse( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 callRef = 0;
    CRpcStatus status;
    if ( !header.HasFlag( GUCEF_RPC_FRAMEFLAG_RESPONDER ) || !reader.ReadVarUInt64( callRef ) || !CRpcFrameCodec::ReadStatus( reader, status ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed RESPONSE" ), true );
        return;
    }
    SClientCall* call = FindClientCall( callRef );
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_MORE ) )
    {
        UInt64 total = 0;
        if ( !reader.ReadVarUInt64( total ) )
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed RESPONSE" ), true );
            return;
        }
        UInt32 prefix = header.bodyOffset + ( reader.GetPosition() - CORE::CBinaryWireWriter::GetVarUInt64Size( total ) );
        StartAssembly( header, body - header.bodyOffset, prefix, total, reader.GetCurrentPtr(), reader.GetRemaining(), arrivedWith, GUCEF_NULL );
        return;
    }
    if ( GUCEF_NULL == call )
    {
        // the call ended here already (cancelled, deadline): acknowledged and dropped
        m_counters[ "lateAnswersDropped" ] += 1;
        return;
    }
    if ( !CheckProtection( call->ref.method, true, arrivedWith ) )
        return;
    CompleteClientCall( callRef, status, reader.GetCurrentPtr(), reader.GetRemaining(), false );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleAccepted( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 callRef = 0;
    CRpcRequestId id;
    UInt8 acceptFlags = 0;
    UInt32 retryAfterMs = 0;
    UInt32 etaMs = 0;
    if ( !reader.ReadVarUInt64( callRef ) || !CRpcFrameCodec::ReadRequestId( reader, id ) || !reader.ReadUInt8( acceptFlags ) ||
         !reader.ReadVarUInt32( retryAfterMs ) || !reader.ReadVarUInt32( etaMs ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed ACCEPTED" ), true );
        return;
    }
    SClientCall* call = FindClientCall( callRef );
    if ( GUCEF_NULL == call )
        return;
    call->accepted = true;
    call->requestId = id;
    call->durable = 0 != ( acceptFlags & GUCEF_RPC_ACCEPTFLAG_DURABLE );
    call->deadlineMs = 0;   // the request deadline covered the time until ACCEPTED
    if ( 0 != call->options.resultTimeoutMs )
        call->resultDeadlineMs = NowMs() + call->options.resultTimeoutMs;
    if ( id.IsSet() )
        m_acceptedIds[ id ] = callRef;
    m_counters[ "callsAccepted" ] += 1;

    CRpcAcceptedEventData data;
    data.callRef = callRef;
    data.methodPath = call->ref.path;
    data.requestId = id;
    data.isDurable = call->durable;
    data.retryAfterMs = retryAfterMs;
    data.etaMs = etaMs;
    RaiseEvent( AcceptedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleResult( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt8 refKind = 0;
    UInt64 callRef = 0;
    CRpcRequestId id;
    CRpcStatus status;
    bool ok = reader.ReadUInt8( refKind );
    if ( ok && 0 == refKind )
        ok = reader.ReadVarUInt64( callRef );
    else if ( ok && 1 == refKind )
        ok = CRpcFrameCodec::ReadRequestId( reader, id );
    else
        ok = false;
    if ( !ok || !CRpcFrameCodec::ReadStatus( reader, status ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed RESULT" ), true );
        return;
    }
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_MORE ) )
    {
        UInt64 total = 0;
        if ( !reader.ReadVarUInt64( total ) )
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed RESULT" ), true );
            return;
        }
        UInt32 prefix = header.bodyOffset + ( reader.GetPosition() - CORE::CBinaryWireWriter::GetVarUInt64Size( total ) );
        StartAssembly( header, body - header.bodyOffset, prefix, total, reader.GetCurrentPtr(), reader.GetRemaining(), arrivedWith, GUCEF_NULL );
        return;
    }
    if ( 1 == refKind )
    {
        TRequestIdMap::iterator i = m_acceptedIds.find( id );
        if ( i != m_acceptedIds.end() )
            callRef = (*i).second;
    }
    SClientCall* call = 0 != callRef ? FindClientCall( callRef ) : GUCEF_NULL;
    if ( GUCEF_NULL == call )
    {
        // a result whose call is gone (an earlier session): kept only for a STATUS_QUERY of this id that asked for it
        // again, which this RESULT precedes (6.7); anything else is dropped, so a peer cannot fill the table
        m_counters[ "orphanResults" ] += 1;
        if ( id.IsSet() && HasPendingResendQuery( id ) )
        {
            CRpcRequestStatus orphan;
            orphan.requestId = id;
            orphan.hasResult = true;
            orphan.result.status = status;
            orphan.result.requestId = id;
            orphan.result.wasAccepted = true;
            if ( reader.GetRemaining() > 0 )
                orphan.resultPayload.CopyFrom( reader.GetRemaining(), reader.GetCurrentPtr() );
            m_orphanResults[ id ] = orphan;
        }
        return;
    }
    if ( !CheckProtection( call->ref.method, true, arrivedWith ) )
        return;
    CompleteClientCall( callRef, status, reader.GetCurrentPtr(), reader.GetRemaining(), false );
}

/*-------------------------------------------------------------------------*/

CIRpcStreamPtr
CRpcEndpoint::OpenStream( const CRpcMethodRef& ref, const CRpcMessage* firstRequest, const CRpcCallOptions& options )
{GUCEF_TRACE;

    // made before the lock is taken: the stream subscribes to itself, which takes the lock its mail is delivered under
    CRpcClientStream* stream = GUCEF_NEW CRpcClientStream( GetPulseGenerator(), m_link, ref );
    CIRpcStreamPtr streamPtr( stream );
    CLockScope lock( *this );

    CRpcStatus status;
    if ( !ref.IsValid() )
        status.Set( RPC_STATUS_INVALID_ARGUMENT, "the method reference is not resolved" );
    else if ( !ref.method->IsStreaming() )
        status.Set( RPC_STATUS_FAILED_PRECONDITION, ref.path + " is not a streaming method" );
    else if ( RPC_METHODKIND_SERVER_STREAM == ref.method->GetKind() && GUCEF_NULL == firstRequest )
        status.Set( RPC_STATUS_INVALID_ARGUMENT, "a server stream needs its request" );
    else if ( RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state || RPC_SESSIONSTATE_CLOSING == m_state )
        status.Set( RPC_STATUS_UNAVAILABLE, "the session is not open" );
    else if ( RPC_SESSIONSTATE_SUSPENDED == m_state && !options.waitForReady )
        status.Set( RPC_STATUS_UNAVAILABLE, "the session is suspended" );
    if ( status.IsOk() )
    {
        TOutBindingMap::iterator b = m_outBindings.find( ref.bindIndex );
        if ( ref.bindGeneration == m_bindGeneration && b != m_outBindings.end() && ref.method->GetSince() > (*b).second->providerMinor )
            status.Set( RPC_STATUS_UNIMPLEMENTED, ref.path + " is newer than the provider's minor" );
    }
    if ( !status.IsOk() )
    {
        StreamEnd( streamPtr, status, GUCEF_NULL );
        return streamPtr;
    }

    SClientCall* call = GUCEF_NEW SClientCall();
    call->ref = ref;
    call->options = options;
    call->stream = streamPtr;
    call->streamImpl = stream;
    call->hasRequest = GUCEF_NULL != firstRequest;
    if ( GUCEF_NULL != firstRequest && !firstRequest->Encode( call->requestBytes ) )
    {
        GUCEF_DELETE call;
        StreamEnd( streamPtr, CRpcStatus( RPC_STATUS_INVALID_ARGUMENT, "the request cannot be encoded" ), GUCEF_NULL );
        return streamPtr;
    }
    UInt32 deadline = 0 != options.deadlineMs ? options.deadlineMs : ref.method->GetDefaultDeadlineMs();
    if ( 0 != deadline )
    {
        call->deadlineMs = NowMs() + deadline;
        call->deadlineUnixMs = WallClockMs() + deadline;
    }
    CRpcStatus refusal;
    if ( !IssueCall( *call, firstRequest, refusal ) )
    {
        GUCEF_DELETE call;
        StreamEnd( streamPtr, refusal, GUCEF_NULL );
    }
    return streamPtr;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::StreamWrite( CRpcClientStream& stream, const CRpcMessage& message )
{GUCEF_TRACE;

    CLockScope lock( *this );
    SClientCall* call = FindClientCall( stream.GetCallRef() );
    if ( GUCEF_NULL == call || call->streamImpl != &stream || !call->ref.method->HasClientStream() || call->halfClosed )
        return false;
    if ( call->sentMessages >= call->sendCreditLimit )
        return false;
    CORE::CDynamicBuffer payload;
    if ( !message.Encode( payload ) )
        return false;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( call->callRef );
    if ( call->ref.method->IsResumableStream() )
        writer.WriteVarUInt64( call->sentMessages + 1 );
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( !SendSessionFrame( GUCEF_RPC_FRAME_STREAM_MSG, 0, fields, payload.GetConstBufferPtr(), payload.GetDataSize(),
                            ProtectionForFrame( GUCEF_RPC_FRAME_STREAM_MSG, call->ref.method, false ), seq, refusal,
                            CallOrderKey( call->callRef, false ) ) )
    {
        if ( RPC_STATUS_RESOURCE_EXHAUSTED == refusal.code )
            m_queueBlockedStreams.insert( call->callRef );
        return false;
    }
    ++call->sentMessages;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::StreamWritesDone( CRpcClientStream& stream )
{GUCEF_TRACE;

    CLockScope lock( *this );
    SClientCall* call = FindClientCall( stream.GetCallRef() );
    if ( GUCEF_NULL == call || call->streamImpl != &stream || call->halfClosed )
        return false;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( call->callRef );
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( !SendSessionFrame( GUCEF_RPC_FRAME_HALF_CLOSE, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal,
                            CallOrderKey( call->callRef, false ) ) )
        return false;
    call->halfClosed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StreamCancel( CRpcClientStream& stream, const CRpcStatus& status )
{GUCEF_TRACE;

    CLockScope lock( *this );
    SClientCall* call = FindClientCall( stream.GetCallRef() );
    if ( GUCEF_NULL == call || call->streamImpl != &stream )
        return;
    CompleteClientCall( call->callRef, status.IsOk() ? CRpcStatus( RPC_STATUS_CANCELLED, "cancelled" ) : status, GUCEF_NULL, 0, true );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StreamConsumed( CRpcClientStream& stream, UInt32 count, bool explicitGrant )
{GUCEF_TRACE;

    CLockScope lock( *this );
    SClientCall* call = FindClientCall( stream.GetCallRef() );
    if ( GUCEF_NULL == call || call->streamImpl != &stream )
        return;
    if ( explicitGrant )
    {
        call->grantedLimit += count;
        SendStreamCredit( call->callRef, false, call->grantedLimit );
        return;
    }
    call->consumedMessages += count;

    // credit is raised in steps of half the initial credit: one STREAM_CREDIT per batch, not per message
    UInt64 target = call->consumedMessages + call->initialCredit;
    UInt64 step = call->initialCredit / 2 > 0 ? call->initialCredit / 2 : 1;
    if ( target >= call->grantedLimit + step )
    {
        call->grantedLimit = target;
        SendStreamCredit( call->callRef, false, call->grantedLimit );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleStreamCredit( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 callRef = 0;
    UInt64 limit = 0;
    if ( !reader.ReadVarUInt64( callRef ) || !reader.ReadVarUInt64( limit ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STREAM_CREDIT" ), true );
        return;
    }
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_RESPONDER ) )
    {
        // the provider of our call allows more of our stream messages
        SClientCall* call = FindClientCall( callRef );
        if ( GUCEF_NULL != call && limit > call->sendCreditLimit )
        {
            call->sendCreditLimit = limit;
            if ( GUCEF_NULL != call->streamImpl )
                StreamWritable( call->stream );
        }
        return;
    }
    CRpcServerCallPtr serverCall = FindServerCall( callRef );
    if ( !serverCall.IsNULL() && limit > serverCall->m_sendCreditLimit )
    {
        serverCall->m_sendCreditLimit = limit;
        RaiseCallEvent( serverCall, CRpcServerCall::WritableEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleStreamMsg( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 callRef = 0;
    if ( !reader.ReadVarUInt64( callRef ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STREAM_MSG" ), true );
        return;
    }
    bool fromProvider = header.HasFlag( GUCEF_RPC_FRAMEFLAG_RESPONDER );
    const CRpcMethodDescriptor* method = GUCEF_NULL;
    SClientCall* call = GUCEF_NULL;
    CRpcServerCallPtr serverCall;
    if ( fromProvider )
    {
        call = FindClientCall( callRef );
        if ( GUCEF_NULL != call )
            method = call->ref.method;
    }
    else
    {
        serverCall = FindServerCall( callRef );
        if ( !serverCall.IsNULL() )
            method = serverCall->m_method;
    }
    UInt64 streamIndex = 0;
    if ( GUCEF_NULL != method && method->IsResumableStream() && !reader.ReadVarUInt64( streamIndex ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STREAM_MSG" ), true );
        return;
    }
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_MORE ) )
    {
        UInt64 total = 0;
        if ( !reader.ReadVarUInt64( total ) )
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STREAM_MSG" ), true );
            return;
        }
        UInt32 prefix = header.bodyOffset + ( reader.GetPosition() - CORE::CBinaryWireWriter::GetVarUInt64Size( total ) );
        StartAssembly( header, body - header.bodyOffset, prefix, total, reader.GetCurrentPtr(), reader.GetRemaining(), arrivedWith, GUCEF_NULL );
        return;
    }
    if ( GUCEF_NULL == method )
    {
        // the call ended here already: acknowledged and dropped
        m_counters[ "lateStreamMessagesDropped" ] += 1;
        return;
    }
    if ( !CheckProtection( method, fromProvider, arrivedWith ) )
        return;

    if ( fromProvider )
    {
        if ( ++call->receivedMessages > call->grantedLimit )
        {
            EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "stream message beyond the granted credit" ), true );
            return;
        }
        CRpcMessage message( call->ref.contract, method->GetResponseType() );
        CRpcStatus decodeStatus;
        if ( !message.Decode( reader.GetCurrentPtr(), reader.GetRemaining(), decodeStatus ) )
        {
            CompleteClientCall( callRef, decodeStatus, GUCEF_NULL, 0, true );
            return;
        }
        StreamMessage( *call, message, 0 != streamIndex ? streamIndex : call->receivedMessages );
        return;
    }

    if ( ++serverCall->m_receivedMessages > serverCall->m_grantedLimit )
    {
        EndSession( CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "stream message beyond the granted credit" ), true );
        return;
    }
    CRpcMessage message( serverCall->m_contract, method->GetRequestType() );
    CRpcStatus decodeStatus;
    if ( !message.Decode( reader.GetCurrentPtr(), reader.GetRemaining(), decodeStatus ) )
    {
        ServerAnswer( *serverCall, GUCEF_RPC_FRAME_RESPONSE, decodeStatus, GUCEF_NULL );
        return;
    }
    CRpcStreamMessageEventData data( message, 0 != streamIndex ? streamIndex : serverCall->m_receivedMessages );
    RaiseCallEvent( serverCall, CRpcServerCall::InternalMessageEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleHalfClose( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 callRef = 0;
    if ( !reader.ReadVarUInt64( callRef ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed HALF_CLOSE" ), true );
        return;
    }
    CRpcServerCallPtr call = FindServerCall( callRef );
    if ( call.IsNULL() )
        return;
    call->m_halfClosed = true;
    RaiseCallEvent( call, CRpcServerCall::HalfClosedEvent );
}

/*-------------------------------------------------------------------------*/

CRpcServerCallPtr
CRpcEndpoint::FindServerCall( UInt64 callRef ) const
{GUCEF_TRACE;

    TServerCallMap::const_iterator i = m_serverCalls.find( callRef );
    return i != m_serverCalls.end() ? (*i).second : CRpcServerCallPtr();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::AnswerFrameError( UInt64 replyToSeq, UInt64 callRef, TRpcStatusCode code, const CORE::CString& message )
{GUCEF_TRACE;

    // an error before execution (6.5.4): RESPONSE to the call
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( callRef );
    CRpcFrameCodec::WriteStatus( writer, CRpcStatus( code, message ) );
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_RESPONSE, GUCEF_RPC_FRAMEFLAG_RESPONDER, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
    m_counters[ CORE::CString( "callsRefused." ) + RpcStatusCodeToString( code ) ] += 1;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleRequest( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_RESPONDER ) || RPC_SESSIONSTATE_CLOSING == m_state )
    {
        if ( RPC_SESSIONSTATE_CLOSING != m_state )
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "REQUEST with the RESPONDER flag" ), true );
        return;
    }
    CORE::CBinaryWireReader reader( body, size );
    SRequestInfo info;
    info.callRef = header.seq;
    info.flags = header.flags;
    CORE::CString error;
    if ( !reader.ReadVarUInt32( info.bindIndex ) || !reader.ReadVarUInt32( info.serviceNumber ) || !reader.ReadVarUInt32( info.methodNumber ) ||
         !reader.ReadVarUInt64( info.deadlineUnixMs ) || !reader.ReadVarUInt32( info.callFlags ) ||
         ( 0 != ( info.callFlags & GUCEF_RPC_CALLFLAG_RESUME_FROM ) && !reader.ReadVarUInt64( info.resumeFromIndex ) ) ||
         !CRpcFrameCodec::ReadMetadata( reader, info.metadata, m_inKeys, error ) )
    {
        // the metadata tables of both sides would drift apart: the session cannot continue
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed REQUEST: " + error ), true );
        return;
    }
    if ( header.HasFlag( GUCEF_RPC_FRAMEFLAG_MORE ) )
    {
        UInt64 total = 0;
        if ( !reader.ReadVarUInt64( total ) )
        {
            EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed REQUEST" ), true );
            return;
        }
        StartAssembly( header, body - header.bodyOffset, header.bodyOffset + reader.GetPosition(), total,
                       reader.GetCurrentPtr(), reader.GetRemaining(), arrivedWith, &info );
        return;
    }
    ProcessRequest( info, reader.GetCurrentPtr(), reader.GetRemaining(), arrivedWith );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::ProcessRequest( const SRequestInfo& info, const UInt8* payload, UInt32 payloadSize, TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    const UInt64 callRef = info.callRef;
    TInBindingMap::iterator b = m_inBindings.find( info.bindIndex );
    if ( b == m_inBindings.end() )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_NOT_FOUND, "unknown bind index " + CORE::ToString( info.bindIndex ) );
        return;
    }
    const SInBinding& binding = *(*b).second;
    if ( !binding.ok )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_FAILED_PRECONDITION, "the contract is not bound: " + binding.failStatus.ToString() );
        return;
    }
    const CRpcServiceDescriptor* service = binding.contract->FindServiceByNumber( info.serviceNumber );
    const CRpcMethodDescriptor* method = GUCEF_NULL != service ? service->FindMethodByNumber( info.methodNumber ) : GUCEF_NULL;
    if ( GUCEF_NULL == method )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_NOT_FOUND, "unknown service " + CORE::ToString( info.serviceNumber ) +
                                                                    " method " + CORE::ToString( info.methodNumber ) );
        return;
    }
    if ( !CheckProtection( method, false, arrivedWith ) )
        return;

    // every call passes the policy before its payload is decoded (14.2)
    if ( !m_accessPolicy.IsNULL() )
    {
        CORE::CString reason;
        TRpcAccessDecision decision = m_accessPolicy->AuthorizeCall( m_principal, *binding.contract, *service, *method, info.metadata, reason );
        if ( RPC_ACCESS_ALLOW != decision )
        {
            AnswerFrameError( callRef, callRef, RPC_ACCESS_UNAUTHENTICATED == decision ? RPC_STATUS_UNAUTHENTICATED : RPC_STATUS_PERMISSION_DENIED, reason );
            return;
        }
    }
    UInt64 wallNow = WallClockMs();
    if ( 0 != info.deadlineUnixMs && wallNow > info.deadlineUnixMs + m_settings.clockSkewAllowanceMs )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_DEADLINE_EXCEEDED, "the deadline passed before the request arrived" );
        return;
    }
    if ( 0 != m_settings.limits.maxConcurrentCalls && m_serverCalls.size() >= m_settings.limits.maxConcurrentCalls )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_RESOURCE_EXHAUSTED, "too many concurrent calls" );
        return;
    }

    CIRpcServiceImpl* impl = GUCEF_NULL;
    TRpcExecutionMode mode = RPC_EXECUTION_CALLING_THREAD;
    CRpcServiceSet::SEntry entry;
    if ( m_services->Find( binding.contract->GetHash(), service->GetNumber(), entry ) )
    {
        impl = entry.impl;
        mode = entry.mode;
    }
    else if ( GUCEF_RPC_BINDINDEX_RPC == info.bindIndex && m_settings.enableReflection )
        impl = m_reflection;
    if ( GUCEF_NULL == impl || !impl->HasHandler( *method ) )
    {
        AnswerFrameError( callRef, callRef, RPC_STATUS_UNIMPLEMENTED, method->GetPath() + " is not implemented" );
        return;
    }
    TRpcExecutionMode methodMode = mode;
    if ( impl->GetMethodExecutionMode( *method, methodMode ) )
        mode = methodMode;

    CRpcServerCall* call = GUCEF_NEW CRpcServerCall( RPC_EXECUTION_CALLING_THREAD == mode ? m_protocolPulseGenerator : GetPulseGenerator() );
    CRpcServerCallPtr callPtr = call->CreateSharedPtr();
    call->m_link = m_link;
    call->m_sessionGeneration = m_sessionGeneration;
    call->m_callRef = callRef;
    call->m_contract = binding.contract;
    call->m_method = method;
    call->m_impl = impl;
    call->m_principal = m_principal;
    call->m_metadata = info.metadata;
    call->m_deadlineUnixMs = info.deadlineUnixMs;
    if ( 0 != info.deadlineUnixMs )
        call->m_deadlineMs = NowMs() + ( info.deadlineUnixMs + m_settings.clockSkewAllowanceMs > wallNow ? info.deadlineUnixMs + m_settings.clockSkewAllowanceMs - wallNow : 0 );
    call->m_hasResumeFrom = 0 != ( info.callFlags & GUCEF_RPC_CALLFLAG_RESUME_FROM );
    call->m_resumeFromIndex = info.resumeFromIndex;
    call->m_mode = mode;
    call->m_initialCredit = 0 != method->GetInitialCredit() ? method->GetInitialCredit() : m_settings.limits.initialStreamCredit;
    call->m_grantedLimit = call->m_initialCredit;
    call->m_sendCreditLimit = 0 != method->GetInitialCredit() ? method->GetInitialCredit() : m_peerLimits.initialStreamCredit;
    call->m_hasRequest = 0 == ( info.flags & GUCEF_RPC_FRAMEFLAG_NO_MESSAGE );
    call->m_request.Bind( binding.contract, method->GetRequestType() );
    if ( call->m_hasRequest )
    {
        CRpcStatus decodeStatus;
        if ( !call->m_request.Decode( payload, payloadSize, decodeStatus ) )
        {
            AnswerFrameError( callRef, callRef, RPC_STATUS_INVALID_ARGUMENT, decodeStatus.message );
            return;
        }
        if ( method->HasClientStream() )
            call->m_receivedMessages = 1;
    }
    m_serverCalls[ callRef ] = callPtr;
    m_counters[ "serverCallsStarted" ] += 1;
    if ( method->IsSensitive() )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "RpcEndpoint: call " << callRef << " of " << method->GetPath() << " (sensitive, payload not logged)" );
    DispatchServerCall( callPtr, mode );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::DispatchServerCall( const CRpcServerCallPtr& callRef, TRpcExecutionMode mode )
{GUCEF_TRACE;

    CRpcServerCallPtr call( callRef );
    if ( RPC_EXECUTION_NOTIFICATION_THREAD == mode || RPC_EXECUTION_THREAD_POOL == mode )
    {
        // handed to another thread once the endpoint lock is released (RunDispatch()), in order with the events
        SDeferredEvent* deferred = GUCEF_NEW SDeferredEvent( SDeferredEvent::KIND_DISPATCH );
        deferred->call = call;
        deferred->mode = mode;
        DeferEvent( deferred );
        return;
    }

    // the protocol thread, as part of the frame, once it released the endpoint lock (10.6, 10.9)
    m_pendingHandlers.push_back( SPendingHandler( call, MT::GetCurrentThreadID() ) );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::RunDispatch( const CRpcServerCallPtr& callRef, TRpcExecutionMode mode )
{GUCEF_TRACE;

    CRpcServerCallPtr call( callRef );
    if ( RPC_EXECUTION_NOTIFICATION_THREAD == mode )
    {
        // mailed to the notification pulse generator: the handler runs there (10.6)
        CRpcCallTaskData data( call );
        NotifyObserversFromThread( DispatchToNotificationThreadEvent, &data );
        return;
    }
    CORE::ThreadPoolPtr pool = m_settings.threadPool;
    if ( pool.IsNULL() )
        pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_settings.threadPoolName );
    CRpcCallTaskData data( call );
    if ( pool.IsNULL() || pool->StartTask( CRpcCallTaskConsumer::TaskType, &data, false ).HasNoFuture() )
        ServerAnswer( *call, GUCEF_RPC_FRAME_RESPONSE, CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "the thread pool refused the call" ), GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::OnDispatchToNotificationThread( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CRpcServerCallPtr call = static_cast< CRpcCallTaskData* >( eventData )->call;
    if ( !call.IsNULL() )
        call->RunHandler();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::EndServerCall( CRpcServerCall& call )
{GUCEF_TRACE;

    if ( call.m_sessionGeneration != m_sessionGeneration )
        return;
    TServerCallMap::iterator i = m_serverCalls.find( call.m_callRef );
    if ( i != m_serverCalls.end() && (*i).second.GetPointerAlways() == &call )
    {
        // released at the next pulse: the call may be inside the dispatch of one of its own events right now
        m_retiredServerCalls.push_back( (*i).second );
        m_serverCalls.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::SendServerCallFrame( CRpcServerCall& call, UInt8 type, const CORE::CDynamicBuffer& fields, const CORE::CDynamicBuffer* payload,
                                   TRpcProtectionClass protection, CRpcStatus& refusal )
{GUCEF_TRACE;

    if ( call.m_sessionGeneration != m_sessionGeneration || FindServerCall( call.m_callRef ).GetPointerAlways() != &call )
    {
        refusal.Set( RPC_STATUS_UNAVAILABLE, "the session of the call ended" );
        return false;
    }
    UInt64 seq = 0;
    return SendSessionFrame( type, GUCEF_RPC_FRAMEFLAG_RESPONDER, fields, GUCEF_NULL != payload ? payload->GetConstBufferPtr() : GUCEF_NULL,
                             GUCEF_NULL != payload ? payload->GetDataSize() : 0, protection, seq, refusal, CallOrderKey( call.m_callRef, true ) );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::ServerAnswer( CRpcServerCall& call, UInt8 frameType, const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_done )
        return false;
    CORE::CDynamicBuffer payload;
    CRpcStatus finalStatus = status;
    if ( finalStatus.IsOk() && GUCEF_NULL != response && !response->Encode( payload ) )
        finalStatus.Set( RPC_STATUS_INTERNAL, "the response cannot be encoded" );
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( call.m_callRef );
    CRpcFrameCodec::WriteStatus( writer, finalStatus );
    CRpcStatus refusal;
    bool sent = SendServerCallFrame( call, frameType, fields, finalStatus.IsOk() ? &payload : GUCEF_NULL,
                                     ProtectionForFrame( frameType, call.m_method, true ), refusal );
    {
        MT::CScopeMutex callLock( call.m_lock );
        call.m_done = true;
    }
    m_counters[ CORE::CString( "serverCallsEnded." ) + RpcStatusCodeToString( finalStatus.code ) ] += 1;
    EndServerCall( call );
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::ServerAccept( CRpcServerCall& call, const CRpcRequestId& id, bool durable, UInt32 etaMs, UInt32 retryAfterMs )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_done || call.m_accepted || !call.m_method->MayAnswerAsync() )
        return false;
    if ( durable && !id.IsSet() )
        return false;
    if ( durable && !m_durable->Add( id, GetDurableOwner() ) )
    {
        lock.EarlyUnlock();
        ServerAnswer( call, GUCEF_RPC_FRAME_RESPONSE, CRpcStatus( RPC_STATUS_RESOURCE_EXHAUSTED, "too many durable requests for this subject" ), GUCEF_NULL );
        return false;
    }
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( call.m_callRef );
    CRpcFrameCodec::WriteRequestId( writer, id );
    writer.WriteUInt8( durable ? GUCEF_RPC_ACCEPTFLAG_DURABLE : 0 );
    writer.WriteVarUInt32( retryAfterMs );
    writer.WriteVarUInt32( etaMs );
    CRpcStatus refusal;
    if ( !SendServerCallFrame( call, GUCEF_RPC_FRAME_ACCEPTED, fields, GUCEF_NULL, RPC_PROTECTION_CONFIDENTIAL, refusal ) )
        return false;
    MT::CScopeMutex callLock( call.m_lock );
    call.m_accepted = true;
    call.m_durable = durable;
    call.m_requestId = id;
    if ( durable )
        call.m_durableTable = m_durable;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::ServerCompleteResult( CRpcServerCall& call, const CRpcStatus& status, const CRpcMessage* response )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_done )
        return false;
    if ( !call.m_accepted )
    {
        lock.EarlyUnlock();
        return ServerAnswer( call, GUCEF_RPC_FRAME_RESPONSE, status, response );
    }
    CORE::CDynamicBuffer payload;
    CRpcStatus finalStatus = status;
    if ( finalStatus.IsOk() && GUCEF_NULL != response && !response->Encode( payload ) )
        finalStatus.Set( RPC_STATUS_INTERNAL, "the response cannot be encoded" );
    if ( call.m_durable )
        m_durable->Complete( call.m_requestId, finalStatus, &payload );

    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    if ( call.m_requestId.IsSet() )
    {
        writer.WriteUInt8( 1 );
        CRpcFrameCodec::WriteRequestId( writer, call.m_requestId );
    }
    else
    {
        writer.WriteUInt8( 0 );
        writer.WriteVarUInt64( call.m_callRef );
    }
    CRpcFrameCodec::WriteStatus( writer, finalStatus );
    CRpcStatus refusal;
    bool sent = SendServerCallFrame( call, GUCEF_RPC_FRAME_RESULT, fields, finalStatus.IsOk() ? &payload : GUCEF_NULL,
                                     ProtectionForFrame( GUCEF_RPC_FRAME_RESULT, call.m_method, true ), refusal );
    {
        MT::CScopeMutex callLock( call.m_lock );
        call.m_done = true;
    }
    m_counters[ CORE::CString( "serverCallsEnded." ) + RpcStatusCodeToString( finalStatus.code ) ] += 1;
    EndServerCall( call );
    return sent || call.m_durable;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::ServerSetProgress( CRpcServerCall& call, UInt32 permille, UInt32 etaMs, const CORE::CString& note )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_durable )
        m_durable->SetProgress( call.m_requestId, RPC_REQUESTSTATE_RUNNING, permille, etaMs, note );
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::ServerWrite( CRpcServerCall& call, const CRpcMessage& message )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_done || !call.m_method->HasServerStream() )
        return false;
    if ( call.m_sentMessages >= call.m_sendCreditLimit )
        return false;
    CORE::CDynamicBuffer payload;
    if ( !message.Encode( payload ) )
        return false;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( call.m_callRef );
    if ( call.m_method->IsResumableStream() )
        writer.WriteVarUInt64( call.m_sentMessages + 1 );
    CRpcStatus refusal;
    if ( !SendServerCallFrame( call, GUCEF_RPC_FRAME_STREAM_MSG, fields, &payload,
                               ProtectionForFrame( GUCEF_RPC_FRAME_STREAM_MSG, call.m_method, true ), refusal ) )
        return false;
    ++call.m_sentMessages;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::ServerConsumed( CRpcServerCall& call, UInt32 count, bool explicitGrant )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( call.m_done || call.m_sessionGeneration != m_sessionGeneration )
        return;
    if ( explicitGrant )
    {
        call.m_grantedLimit += count;
        SendStreamCredit( call.m_callRef, true, call.m_grantedLimit );
        return;
    }
    call.m_consumedMessages += count;
    UInt64 target = call.m_consumedMessages + call.m_initialCredit;
    UInt64 step = call.m_initialCredit / 2 > 0 ? call.m_initialCredit / 2 : 1;
    if ( target >= call.m_grantedLimit + step )
    {
        call.m_grantedLimit = target;
        SendStreamCredit( call.m_callRef, true, call.m_grantedLimit );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::CancelAllServerCalls( const CRpcStatus& status )
{GUCEF_TRACE;

    TServerCallMap calls;
    calls.swap( m_serverCalls );
    TServerCallMap::iterator i = calls.begin();
    for ( ; i != calls.end(); ++i )
    {
        CRpcServerCall& call = *(*i).second;
        {
            MT::CScopeMutex callLock( call.m_lock );
            if ( call.m_cancelled )
                continue;
            call.m_cancelled = true;
            call.m_cancelStatus = status;
        }
        RaiseCallEvent( (*i).second, CRpcServerCall::CancelledEvent, const_cast< CRpcStatus* >( &status ) );
    }
    for ( i = calls.begin(); i != calls.end(); ++i )
        m_retiredServerCalls.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleCancel( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt8 refKind = 0;
    UInt64 callRef = 0;
    CRpcRequestId id;
    UInt32 code = RPC_STATUS_CANCELLED;
    CORE::CString reason;
    bool ok = reader.ReadUInt8( refKind );
    if ( ok && 0 == refKind )
        ok = reader.ReadVarUInt64( callRef );
    else if ( ok && 1 == refKind )
        ok = CRpcFrameCodec::ReadRequestId( reader, id );
    else
        ok = false;
    if ( !ok || !reader.ReadVarUInt32( code ) || !reader.ReadString( reason ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed CANCEL" ), true );
        return;
    }
    CRpcStatus status( (TRpcStatusCode) code, reason.IsNULLOrEmpty() ? CORE::CString( "cancelled by the peer" ) : reason );

    if ( 0 == refKind && header.HasFlag( GUCEF_RPC_FRAMEFLAG_RESPONDER ) )
    {
        // the provider of our call ended its side
        CompleteClientCall( callRef, status, GUCEF_NULL, 0, false );
        return;
    }

    CRpcServerCallPtr call;
    if ( 0 == refKind )
        call = FindServerCall( callRef );
    else
    {
        TServerCallMap::iterator i = m_serverCalls.begin();
        for ( ; i != m_serverCalls.end() && call.IsNULL(); ++i )
            if ( (*i).second->m_requestId == id )
                call = (*i).second;
    }
    TRpcRequestState state = RPC_REQUESTSTATE_UNKNOWN;
    if ( !call.IsNULL() )
    {
        {
            MT::CScopeMutex callLock( call->m_lock );
            call->m_cancelled = true;
            call->m_cancelStatus = status;
        }
        RaiseCallEvent( call, CRpcServerCall::CancelledEvent, &status );
        state = RPC_REQUESTSTATE_CANCELLED;
        m_counters[ "serverCallsCancelled" ] += 1;
    }
    if ( 1 != refKind )
        return;

    // by request id: a STATUS answers, also when the CANCEL comes from a later session (6.7)
    CRpcDurableRequestTable::SEntry entry;
    if ( m_durable->Get( id, entry ) )
    {
        bool mayQuery = MayQueryDurable( entry.ownerSubject, id );
        if ( mayQuery )
        {
            m_durable->RequestCancel( id );
            state = entry.hasResult ? entry.state : RPC_REQUESTSTATE_CANCELLED;
        }
    }
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( header.seq );
    CRpcFrameCodec::WriteRequestId( writer, id );
    writer.WriteUInt8( (UInt8) state );
    writer.WriteVarUInt32( 1001 );
    writer.WriteVarUInt32( 0 );
    writer.WriteString( CORE::CString::Empty );
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_STATUS, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleStatusQuery( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    CRpcRequestId id;
    UInt8 flags = 0;
    if ( !CRpcFrameCodec::ReadRequestId( reader, id ) || !reader.ReadUInt8( flags ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STATUS_QUERY" ), true );
        return;
    }

    TRpcRequestState state = RPC_REQUESTSTATE_UNKNOWN;
    UInt32 progress = 1001;
    UInt32 eta = 0;
    CORE::CString note;
    CRpcDurableRequestTable::SEntry entry;
    bool known = m_durable->Get( id, entry );
    if ( known )
    {
        // DURABLE ids are bound to their subject: another subject learns nothing (10.7)
        bool mayQuery = MayQueryDurable( entry.ownerSubject, id );
        if ( !mayQuery )
            known = false;
    }
    if ( known )
    {
        state = entry.state;
        progress = entry.progressPermille;
        eta = entry.etaMs;
        note = entry.note;
        if ( 0 != ( flags & 1 ) && entry.hasResult )
        {
            // the result again, for a caller whose call ended with an earlier session; RESULT goes before STATUS
            CORE::CDynamicBuffer fields;
            CORE::CBinaryWireWriter writer( fields );
            writer.WriteUInt8( 1 );
            CRpcFrameCodec::WriteRequestId( writer, id );
            CRpcFrameCodec::WriteStatus( writer, entry.resultStatus );
            UInt64 seq = 0;
            CRpcStatus refusal;
            SendSessionFrame( GUCEF_RPC_FRAME_RESULT, GUCEF_RPC_FRAMEFLAG_RESPONDER, fields,
                              entry.resultStatus.IsOk() ? entry.resultPayload.GetConstBufferPtr() : GUCEF_NULL,
                              entry.resultStatus.IsOk() ? entry.resultPayload.GetDataSize() : 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
        }
    }
    else
    {
        TServerCallMap::iterator i = m_serverCalls.begin();
        for ( ; i != m_serverCalls.end(); ++i )
        {
            if ( (*i).second->m_requestId == id && (*i).second->m_principal.subject == m_principal.subject )
            {
                state = (*i).second->m_cancelled ? RPC_REQUESTSTATE_CANCELLED : RPC_REQUESTSTATE_RUNNING;
                break;
            }
        }
    }
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteVarUInt64( header.seq );
    CRpcFrameCodec::WriteRequestId( writer, id );
    writer.WriteUInt8( (UInt8) state );
    writer.WriteVarUInt32( progress );
    writer.WriteVarUInt32( eta );
    writer.WriteString( note );
    UInt64 seq = 0;
    CRpcStatus refusal;
    SendSessionFrame( GUCEF_RPC_FRAME_STATUS, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleStatus( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 replyToSeq = 0;
    CRpcRequestStatus requestStatus;
    UInt8 state = 0;
    if ( !reader.ReadVarUInt64( replyToSeq ) || !CRpcFrameCodec::ReadRequestId( reader, requestStatus.requestId ) || !reader.ReadUInt8( state ) ||
         !reader.ReadVarUInt32( requestStatus.progressPermille ) || !reader.ReadVarUInt32( requestStatus.etaMs ) ||
         !reader.ReadString( requestStatus.note ) || state > RPC_REQUESTSTATE_FAILED )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed STATUS" ), true );
        return;
    }
    requestStatus.state = (TRpcRequestState) state;
    TPendingQueryMap::iterator i = m_pendingQueries.find( replyToSeq );
    if ( i == m_pendingQueries.end() )
        return;
    SPendingQuery* query = (*i).second;
    m_pendingQueries.erase( i );
    TOrphanResultMap::iterator orphan = m_orphanResults.find( requestStatus.requestId );
    if ( orphan != m_orphanResults.end() )
    {
        requestStatus.hasResult = true;
        requestStatus.result = (*orphan).second.result;
        requestStatus.resultPayload = (*orphan).second.resultPayload;
        m_orphanResults.erase( orphan );
    }
    query->future.Complete( requestStatus.Clone(), true );
    GUCEF_DELETE query;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcEndpoint::GetPeerBindingCount( void ) const
{GUCEF_TRACE;

    // index 0 (gucef.rpc) is bound implicitly in every session, not by the peer
    UInt32 count = (UInt32) m_inBindings.size();
    if ( count > 0 && m_inBindings.find( GUCEF_RPC_BINDINDEX_RPC ) != m_inBindings.end() )
        --count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::HasPendingResendQuery( const CRpcRequestId& id ) const
{GUCEF_TRACE;

    TPendingQueryMap::const_iterator i = m_pendingQueries.begin();
    for ( ; i != m_pendingQueries.end(); ++i )
        if ( (*i).second->resend && !(*i).second->isCancel && (*i).second->id == id )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleError( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 replyToSeq = 0;
    CRpcStatus status;
    if ( !reader.ReadVarUInt64( replyToSeq ) || !CRpcFrameCodec::ReadStatus( reader, status ) )
    {
        EndSession( CRpcStatus( RPC_STATUS_INTERNAL, "malformed ERROR" ), true );
        return;
    }
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: the peer reports an error about frame " << replyToSeq << ": " << status.ToString() );
    TPendingQueryMap::iterator i = m_pendingQueries.find( replyToSeq );
    if ( i == m_pendingQueries.end() )
        return;
    CRpcRequestStatus requestStatus;
    requestStatus.requestId = (*i).second->id;
    requestStatus.status = status;
    (*i).second->future.Complete( requestStatus.Clone(), true );
    GUCEF_DELETE (*i).second;
    m_pendingQueries.erase( i );
    if ( !HasPendingResendQuery( requestStatus.requestId ) )
        m_orphanResults.erase( requestStatus.requestId );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleGoodbye( const SRpcFrameHeader& header, const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    CRpcStatus status;
    CRpcFrameCodec::ReadStatus( reader, status );

    // no resume after GOODBYE: open calls end, the GOODBYE is acknowledged, the session ends
    CompleteAllClientCalls( CRpcStatus( RPC_STATUS_UNAVAILABLE, "session closed by peer" ) );
    CancelAllServerCalls( CRpcStatus( RPC_STATUS_CANCELLED, "session closed by peer" ) );
    m_session.SendAck( NowMs() );
    EndSession( CRpcStatus( RPC_STATUS_OK, "session closed by peer" ), false );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandlePing( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 pingId = 0;
    UInt64 sendTimeUs = 0;
    if ( !reader.ReadVarUInt64( pingId ) || !reader.ReadVarUInt64( sendTimeUs ) )
        return;
    m_controlBody.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( m_controlBody );
    writer.WriteVarUInt64( pingId );
    writer.WriteVarUInt64( sendTimeUs );
    writer.WriteVarUInt64( 0 );
    SendControl( GUCEF_RPC_FRAME_PONG, 0, m_controlBody );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandlePong( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt64 pingId = 0;
    if ( reader.ReadVarUInt64( pingId ) && pingId == m_pingId )
        m_pingSentMs = 0;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::HandleAbort( const UInt8* body, UInt32 size )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    CRpcStatus status;
    if ( !CRpcFrameCodec::ReadStatus( reader, status ) )
        status.Set( RPC_STATUS_INTERNAL, "aborted by the peer" );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint: the peer aborted the session: " << status.ToString() );
    EndSession( status, false );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRpcEndpoint::QueryStatusAsync( const CRpcRequestId& id, bool resendResultWhenDone )
{GUCEF_TRACE;

    CLockScope lock( *this );
    SPendingQuery* query = GUCEF_NEW SPendingQuery();
    query->id = id;
    query->resend = resendResultWhenDone;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    CRpcFrameCodec::WriteRequestId( writer, id );
    writer.WriteUInt8( resendResultWhenDone ? 1 : 0 );
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( RPC_SESSIONSTATE_ESTABLISHED != m_state && RPC_SESSIONSTATE_SUSPENDED != m_state )
        refusal.Set( RPC_STATUS_UNAVAILABLE, "the session is not open" );
    else if ( SendSessionFrame( GUCEF_RPC_FRAME_STATUS_QUERY, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal ) )
    {
        m_pendingQueries[ seq ] = query;
        return query->future.GetFuture();
    }
    CRpcRequestStatus requestStatus;
    requestStatus.requestId = id;
    requestStatus.status = refusal;
    query->future.Complete( requestStatus.Clone(), true );
    CORE::CFutureResult future = query->future.GetFuture();
    GUCEF_DELETE query;
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRpcEndpoint::CancelRequestAsync( const CRpcRequestId& id )
{GUCEF_TRACE;

    CLockScope lock( *this );

    // the caller's own call of that id completes CANCELLED at once (gRPC behaviour)
    TRequestIdMap::iterator open = m_acceptedIds.find( id );
    if ( open != m_acceptedIds.end() )
        CompleteClientCall( (*open).second, CRpcStatus( RPC_STATUS_CANCELLED, "cancelled" ), GUCEF_NULL, 0, false );

    SPendingQuery* query = GUCEF_NEW SPendingQuery();
    query->id = id;
    query->isCancel = true;
    CORE::CDynamicBuffer fields;
    CORE::CBinaryWireWriter writer( fields );
    writer.WriteUInt8( 1 );
    CRpcFrameCodec::WriteRequestId( writer, id );
    writer.WriteVarUInt32( (UInt32) RPC_STATUS_CANCELLED );
    writer.WriteString( CORE::CString::Empty );
    UInt64 seq = 0;
    CRpcStatus refusal;
    if ( RPC_SESSIONSTATE_ESTABLISHED != m_state && RPC_SESSIONSTATE_SUSPENDED != m_state )
        refusal.Set( RPC_STATUS_UNAVAILABLE, "the session is not open" );
    else if ( SendSessionFrame( GUCEF_RPC_FRAME_CANCEL, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, seq, refusal ) )
    {
        m_pendingQueries[ seq ] = query;
        return query->future.GetFuture();
    }
    CRpcRequestStatus requestStatus;
    requestStatus.requestId = id;
    requestStatus.status = refusal;
    query->future.Complete( requestStatus.Clone(), true );
    CORE::CFutureResult future = query->future.GetFuture();
    GUCEF_DELETE query;
    return future;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::QueryStatus( const CRpcRequestId& id, bool resendResultWhenDone, CRpcRequestStatus& requestStatus, CRpcStatus& status )
{GUCEF_TRACE;

    CORE::CFutureResult future = QueryStatusAsync( id, resendResultWhenDone );
    if ( !WaitForFuture( future, "QueryStatus()", status ) )
        return false;
    if ( !TryGetRequestStatus( future, requestStatus ) )
        return false;
    status = requestStatus.status;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::CancelRequest( const CRpcRequestId& id, CRpcStatus& status )
{GUCEF_TRACE;

    CORE::CFutureResult future = CancelRequestAsync( id );
    if ( !WaitForFuture( future, "CancelRequest()", status ) )
        return false;
    CRpcRequestStatus requestStatus;
    if ( !TryGetRequestStatus( future, requestStatus ) )
        return false;
    status = requestStatus.status;
    return status.IsOk();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::Close( bool graceful )
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state )
        return;
    if ( !graceful || RPC_SESSIONSTATE_ESTABLISHED != m_state )
    {
        EndSession( CRpcStatus( RPC_STATUS_CANCELLED, "closed" ), true );
        return;
    }
    // GOODBYE follows when the open calls completed or closeTimeoutMs passed (6.12)
    m_state = RPC_SESSIONSTATE_CLOSING;
    m_closeGraceful = true;
    m_closeRequestedMs = NowMs();
    Tick( m_closeRequestedMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::Suspend( const CRpcStatus& why )
{GUCEF_TRACE;

    if ( RPC_SESSIONSTATE_CLOSED == m_state )
        return;
    bool wasSuspended = RPC_SESSIONSTATE_SUSPENDED == m_state;
    m_state = RPC_SESSIONSTATE_SUSPENDED;
    m_resumePending = false;
    m_session.SetTransportUp( false, NowMs() );
    if ( !m_transport.IsNULL() && m_transport->IsConnected() )
        m_transport->Close();
    if ( wasSuspended )
        return;
    m_suspendedSinceMs = NowMs();
    m_nextReconnectMs = m_suspendedSinceMs + m_settings.reconnectInitialDelayMs;
    m_reconnectDelayMs = m_settings.reconnectInitialDelayMs;
    m_counters[ "suspensions" ] += 1;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session suspended: " << why.ToString() );
    CRpcStatus data( why );
    RaiseEvent( SessionSuspendedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::EndSession( const CRpcStatus& status, bool sendAbort )
{GUCEF_TRACE;

    if ( RPC_SESSIONSTATE_CLOSED == m_state )
        return;
    if ( sendAbort )
        SendAbort( status );
    m_state = RPC_SESSIONSTATE_CLOSED;
    m_closeStatus = status;

    CRpcStatus callStatus = status;
    if ( status.IsOk() )
        callStatus.Set( RPC_STATUS_UNAVAILABLE, status.message.IsNULLOrEmpty() ? CORE::CString( "the session closed" ) : status.message );
    CompleteAllClientCalls( callStatus );
    CancelAllServerCalls( callStatus );
    DetachTransport( true );

    // the resume token is kept in memory only and wiped when the session ends (14.5)
    memset( m_resumeToken, 0, sizeof( m_resumeToken ) );
    m_hasIdentity = false;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: session closed: " << status.ToString() );
    CRpcStatus data( status );
    RaiseEvent( SessionClosedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::StartNewSessionAfterGap( const CRpcSessionGapEventData& gap, TRpcStatusCode callStatus )
{GUCEF_TRACE;

    CRpcStatus status( callStatus, RPC_STATUS_RESUME_GAP == callStatus ? "the resume could not cover the lost frames" : "sessionExpired" );
    status.details.SetAttribute( "gapToInitiatorFrom", gap.gapToInitiatorFrom );
    status.details.SetAttribute( "gapToInitiatorTo", gap.gapToInitiatorTo );
    status.details.SetAttribute( "gapToAcceptorFrom", gap.gapToAcceptorFrom );
    status.details.SetAttribute( "gapToAcceptorTo", gap.gapToAcceptorTo );
    if ( RPC_STATUS_UNAVAILABLE == callStatus )
        status.details.SetAttribute( "sessionExpired", true );

    // idempotent calls are issued again on the new session (8.6); every other call of the old session ends
    GUCEF::vector< SClientCall* > retry;
    TClientCallMap calls;
    calls.swap( m_clientCalls );
    TClientCallMap::iterator i = calls.begin();
    for ( ; i != calls.end(); ++i )
    {
        SClientCall* call = (*i).second;
        if ( GUCEF_NULL == call->streamImpl && call->ref.IsValid() && call->ref.method->IsIdempotent() &&
             call->options.retryIdempotentOnNewSession && call->hasFuture )
        {
            call->callRef = 0;
            call->accepted = false;
            call->requestId.Clear();
            retry.push_back( call );
            m_counters[ "idempotentRetries" ] += 1;
            continue;
        }
        m_clientCalls[ (*i).first ] = call;
    }
    while ( !m_clientCalls.empty() )
        CompleteClientCall( (*m_clientCalls.begin()).first, status, GUCEF_NULL, 0, false );
    m_acceptedIds.clear();
    TPendingQueryMap::iterator q = m_pendingQueries.begin();
    for ( ; q != m_pendingQueries.end(); ++q )
    {
        CRpcRequestStatus requestStatus;
        requestStatus.requestId = (*q).second->id;
        requestStatus.status = status;
        (*q).second->future.Complete( requestStatus.Clone(), true );
        GUCEF_DELETE (*q).second;
    }
    m_pendingQueries.clear();
    m_orphanResults.clear();
    CancelAllServerCalls( status );
    for ( UInt32 f=0; f<m_fragmenters.size(); ++f )
        GUCEF_DELETE m_fragmenters[ f ];
    m_fragmenters.clear();
    TAssemblyMap::iterator a = m_assemblies.begin();
    for ( ; a != m_assemblies.end(); ++a )
        GUCEF_DELETE (*a).second;
    m_assemblies.clear();
    m_assemblyBytes = 0;

    m_session.Reset();
    ResetBindings();
    ++m_sessionGeneration;
    m_lastHelloAck.Clear( true );
    for ( UInt32 r=0; r<retry.size(); ++r )
        m_waitingCalls.push_back( retry[ r ] );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::TryReconnect( UInt64 nowMs )
{GUCEF_TRACE;

    if ( !m_initiator || !m_settings.autoReconnect || m_remoteUri.GetScheme().IsNULLOrEmpty() || nowMs < m_nextReconnectMs )
        return;
    if ( !m_transport.IsNULL() && m_transport->IsConnected() )
        return;

    // the transport is made when the pulse ends, without the endpoint lock (Reconnect()): a factory runs code of others
    UInt32 factor = m_settings.reconnectFactor > 1 ? m_settings.reconnectFactor : 2;
    m_reconnectDelayMs = m_reconnectDelayMs * factor;
    if ( 0 != m_settings.reconnectMaxDelayMs && m_reconnectDelayMs > m_settings.reconnectMaxDelayMs )
        m_reconnectDelayMs = m_settings.reconnectMaxDelayMs;
    m_nextReconnectMs = nowMs + m_reconnectDelayMs;
    m_reconnectDue = true;
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::Reconnect( const CORE::CUri& remote )
{GUCEF_TRACE;

    // made, set up and subscribed to without the endpoint lock (10.9: nothing is called out under it, and the endpoint
    // does not subscribe while it holds it); nobody else knows the transport yet
    CIRpcTransportPtr transport = CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( remote.GetScheme() );
    if ( transport.IsNULL() )
        return;
    {
        CLockScope lock( *this );
        if ( RefusesInsecureTransport( transport ) )
            return;
    }
    transport->SetPulseGenerator( m_protocolPulseGenerator );
    SubscribeTransport( transport );

    CLockScope lock( *this );
    if ( RPC_SESSIONSTATE_SUSPENDED != m_state || m_resumePending || ( !m_transport.IsNULL() && m_transport->IsConnected() ) )
    {
        // resumed or ended meanwhile: the transport is not needed; its subscription goes at the next pulse
        m_staleTransports.push_back( transport );
        return;
    }
    AttachTransport( transport );
    if ( !transport->Connect( remote ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcEndpoint: reconnect to " << remote.ToUriInStringForm() << " failed, next attempt in " << m_reconnectDelayMs << " ms" );
        return;
    }
    if ( transport->IsConnected() )
        SendHello();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::Tick( UInt64 nowMs )
{GUCEF_TRACE;

    if ( RPC_SESSIONSTATE_CLOSED == m_state || RPC_SESSIONSTATE_IDLE == m_state )
        return;

    m_session.OnTick( nowMs );
    PumpFragments();

    // STREAM_CREDIT is a control frame: on a transport that may lose frames it is sent again now and then (it is
    // absolute, so a repeat is harmless), else a lost one would stall its stream
    if ( RPC_SESSIONSTATE_ESTABLISHED == m_state && !m_session.GetSettings().reliableOrdered && nowMs >= m_nextCreditResendMs )
    {
        UInt64 interval = (UInt64) ( 4.0 * m_session.GetSmoothedRttMs() );
        if ( interval < 200 )
            interval = 200;
        m_nextCreditResendMs = nowMs + interval;
        AnnounceCredits();
    }

    // HELLO again on unreliable transports while it is not answered
    if ( m_initiator && ( RPC_SESSIONSTATE_CONNECTING == m_state || ( RPC_SESSIONSTATE_SUSPENDED == m_state && m_resumePending ) ) &&
         !m_transport.IsNULL() && m_transport->IsConnected() && !m_transport->GetProperties().isReliable && 0 != m_helloSentMs )
    {
        CRpcTransportProfile profile = m_settings.useTransportProfile ? CRpcTransportProfile::GetProfile( m_transport->GetProperties().profileName ) : m_settings.profile;
        if ( nowMs - m_helloSentMs >= profile.helloRetryMs )
            SendHello();
    }

    // deadlines, result timeouts and cancellation through the futures: a look at every open call, so once per tick
    // interval (the protocol thread may pulse far more often, a busy one with thousands of calls open)
    if ( nowMs >= m_nextCallScanMs )
    {
        m_nextCallScanMs = nowMs + ( 0 != m_settings.tickIntervalMs ? m_settings.tickIntervalMs : 1 );
        GUCEF::vector< UInt64 > expired;
        GUCEF::vector< UInt64 > resultTimeouts;
        GUCEF::vector< UInt64 > cancelled;
        TClientCallMap::iterator i = m_clientCalls.begin();
        for ( ; i != m_clientCalls.end(); ++i )
        {
            SClientCall* call = (*i).second;
            if ( 0 != call->deadlineMs && nowMs >= call->deadlineMs && !call->accepted )
                expired.push_back( (*i).first );
            else if ( call->accepted && 0 != call->resultDeadlineMs && nowMs >= call->resultDeadlineMs )
                resultTimeouts.push_back( (*i).first );
            else if ( call->hasFuture && call->future.IsCancellationRequested() )
                cancelled.push_back( (*i).first );
        }
        for ( UInt32 e=0; e<expired.size(); ++e )
            CompleteClientCall( expired[ e ], CRpcStatus( RPC_STATUS_DEADLINE_EXCEEDED, "the deadline passed" ), GUCEF_NULL, 0, true );
        for ( UInt32 r=0; r<resultTimeouts.size(); ++r )
        {
            SClientCall* call = FindClientCall( resultTimeouts[ r ] );
            bool sendCancel = GUCEF_NULL != call && !call->options.keepRunningAfterResultTimeout;
            CompleteClientCall( resultTimeouts[ r ], CRpcStatus( RPC_STATUS_DEADLINE_EXCEEDED, "no result in time" ), GUCEF_NULL, 0, sendCancel );
        }
        for ( UInt32 c=0; c<cancelled.size(); ++c )
            CompleteClientCall( cancelled[ c ], CRpcStatus( RPC_STATUS_CANCELLED, "cancelled by the caller" ), GUCEF_NULL, 0, true );
        for ( UInt32 w=0; w<m_waitingCalls.size(); )
        {
            SClientCall* call = m_waitingCalls[ w ];
            bool expire = 0 != call->deadlineMs && nowMs >= call->deadlineMs;
            bool cancel = call->hasFuture && call->future.IsCancellationRequested();
            if ( expire || cancel )
            {
                CRpcCallResult result;
                result.status.Set( expire ? RPC_STATUS_DEADLINE_EXCEEDED : RPC_STATUS_CANCELLED, expire ? "the deadline passed" : "cancelled by the caller" );
                if ( call->hasFuture )
                    call->future.Complete( result.Clone(), true );
                else if ( GUCEF_NULL != call->streamImpl )
                    StreamEnd( call->stream, result.status, GUCEF_NULL );
                GUCEF_DELETE call;
                m_waitingCalls.erase( m_waitingCalls.begin() + w );
            }
            else
                ++w;
        }

        // running calls past their deadline are cancelled (6.8)
        TServerCallMap::iterator s = m_serverCalls.begin();
        for ( ; s != m_serverCalls.end(); ++s )
        {
            CRpcServerCall& call = *(*s).second;
            if ( 0 != call.m_deadlineMs && nowMs >= call.m_deadlineMs && !call.m_cancelled && !call.m_done && !call.m_accepted )
            {
                CRpcStatus status( RPC_STATUS_DEADLINE_EXCEEDED, "the deadline passed" );
                {
                    MT::CScopeMutex callLock( call.m_lock );
                    call.m_cancelled = true;
                    call.m_cancelStatus = status;
                }
                RaiseCallEvent( (*s).second, CRpcServerCall::CancelledEvent, &status );
            }
        }
    }

    switch ( m_state )
    {
        case RPC_SESSIONSTATE_ESTABLISHED:
        {
            // keepalive: PING when idle, no PONG in time closes the transport, which suspends the session
            CRpcTransportProfile profile = m_settings.useTransportProfile && !m_transport.IsNULL() ?
                                           CRpcTransportProfile::GetProfile( m_transport->GetProperties().profileName ) : m_settings.profile;
            if ( 0 != profile.keepaliveIntervalMs )
            {
                UInt64 lastActivity = m_lastReceiveMs > m_lastSendMs ? m_lastReceiveMs : m_lastSendMs;
                if ( 0 == m_pingSentMs && nowMs - lastActivity >= profile.keepaliveIntervalMs )
                {
                    m_controlBody.SetDataSize( 0 );
                    CORE::CBinaryWireWriter writer( m_controlBody );
                    writer.WriteVarUInt64( ++m_pingId );
                    writer.WriteVarUInt64( nowMs * 1000 );
                    if ( SendControl( GUCEF_RPC_FRAME_PING, 0, m_controlBody ) )
                        m_pingSentMs = nowMs;
                }
                else if ( 0 != m_pingSentMs && 0 != profile.keepaliveTimeoutMs && nowMs - m_pingSentMs >= profile.keepaliveTimeoutMs )
                {
                    m_pingSentMs = 0;
                    Suspend( CRpcStatus( RPC_STATUS_UNAVAILABLE, "keepalive timeout" ) );
                }
            }
            break;
        }
        case RPC_SESSIONSTATE_SUSPENDED:
        {
            if ( 0 != m_settings.limits.resumeGraceMs && nowMs - m_suspendedSinceMs >= m_settings.limits.resumeGraceMs )
            {
                CRpcStatus expiredStatus( RPC_STATUS_UNAVAILABLE, "the resume grace period expired" );
                expiredStatus.details.SetAttribute( "sessionExpired", true );
                EndSession( expiredStatus, false );
                return;
            }
            if ( !m_resumePending )
                TryReconnect( nowMs );
            break;
        }
        case RPC_SESSIONSTATE_CLOSING:
        {
            if ( 0 == m_goodbyeSeq )
            {
                bool idle = m_clientCalls.empty() && m_waitingCalls.empty() && m_serverCalls.empty();
                if ( idle || nowMs - m_closeRequestedMs >= m_settings.closeTimeoutMs )
                {
                    CORE::CDynamicBuffer fields;
                    CORE::CBinaryWireWriter writer( fields );
                    CRpcFrameCodec::WriteStatus( writer, CRpcStatus::Ok );
                    CRpcStatus refusal;
                    if ( !SendSessionFrame( GUCEF_RPC_FRAME_GOODBYE, 0, fields, GUCEF_NULL, 0, RPC_PROTECTION_CONFIDENTIAL, m_goodbyeSeq, refusal ) )
                        EndSession( CRpcStatus( RPC_STATUS_OK, "closed" ), false );
                }
            }
            else if ( m_session.GetPeerAckedSeq() >= m_goodbyeSeq )
                EndSession( CRpcStatus( RPC_STATUS_OK, "closed" ), false );
            else if ( nowMs - m_closeRequestedMs >= 2 * m_settings.closeTimeoutMs + 1000 )
                EndSession( CRpcStatus( RPC_STATUS_OK, "closed without acknowledgement of GOODBYE" ), false );
            break;
        }
        default:
            break;
    }

    PumpFragments();
}

/*-------------------------------------------------------------------------*/

TRpcSessionState
CRpcEndpoint::GetState( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::IsInitiator( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_initiator;
}

/*-------------------------------------------------------------------------*/

const CRpcPrincipal&
CRpcEndpoint::GetPeerPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

CIRpcTransportPtr
CRpcEndpoint::GetTransport( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_transport;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CRpcEndpoint::GetProtocolPulseGenerator( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_protocolPulseGenerator;
}

/*-------------------------------------------------------------------------*/

const CRpcEndpointSettings&
CRpcEndpoint::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpoint::GetSessionId( UInt8* sessionId ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    if ( !m_hasIdentity )
        return false;
    memcpy( sessionId, m_sessionId, GUCEF_RPC_SESSIONID_SIZE );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcEndpoint::GetNextSendSeq( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_session.GetNextSeq();
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcEndpoint::GetDeliveredSeq( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return m_session.GetDeliveredSeq();
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcEndpoint::GetOpenCallCount( void ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    return (UInt32) ( m_clientCalls.size() + m_waitingCalls.size() );
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpoint::GetTelemetry( CORE::CDataNode& telemetry ) const
{GUCEF_TRACE;

    CLockScope lock( *this );
    const CRpcSession::SCounters& counters = m_session.GetCounters();
    telemetry.SetAttribute( "state", RpcSessionStateToString( m_state ) );
    telemetry.SetAttribute( "framesSent", counters.framesSent );
    telemetry.SetAttribute( "framesReceived", counters.framesReceived );
    telemetry.SetAttribute( "bytesSent", counters.bytesSent );
    telemetry.SetAttribute( "bytesReceived", counters.bytesReceived );
    telemetry.SetAttribute( "retransmissions", counters.retransmissions );
    telemetry.SetAttribute( "resumeRetransmissions", counters.resumeRetransmissions );
    telemetry.SetAttribute( "duplicateFrames", counters.duplicateFrames );
    telemetry.SetAttribute( "gaps", counters.gaps );
    telemetry.SetAttribute( "standaloneAcks", counters.standaloneAcks );
    telemetry.SetAttribute( "droppedFrames", counters.droppedFrames );
    telemetry.SetAttribute( "retainedFrames", m_session.GetRetainedFrameCount() );
    telemetry.SetAttribute( "nextSeq", m_session.GetNextSeq() );
    telemetry.SetAttribute( "deliveredSeq", m_session.GetDeliveredSeq() );
    telemetry.SetAttribute( "smoothedRttMs", CORE::CVariant( m_session.GetSmoothedRttMs() ) );
    telemetry.SetAttribute( "inBindings", (UInt64) GetPeerBindingCount() );
    telemetry.SetAttribute( "openAssemblies", (UInt64) m_assemblies.size() );
    telemetry.SetAttribute( "openAssemblyBytes", m_assemblyBytes );
    telemetry.SetAttribute( "orphanResultsHeld", (UInt64) m_orphanResults.size() );
    GUCEF::map< CORE::CString, UInt64 >::const_iterator i = m_counters.begin();
    for ( ; i != m_counters.end(); ++i )
        telemetry.SetAttribute( (*i).first, (*i).second );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
