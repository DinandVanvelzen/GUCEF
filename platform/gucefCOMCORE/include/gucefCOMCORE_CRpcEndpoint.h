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

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#define GUCEF_COMCORE_CRPCENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#include "gucefCOMCORE_RpcProtocol.h"
#define GUCEF_COMCORE_RPCPROTOCOL_H
#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */

#ifndef GUCEF_COMCORE_CRPCSESSION_H
#include "gucefCOMCORE_CRpcSession.h"
#define GUCEF_COMCORE_CRPCSESSION_H
#endif /* GUCEF_COMCORE_CRPCSESSION_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#include "gucefCOMCORE_CRpcEndpointSettings.h"
#define GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#include "gucefCOMCORE_CRpcServiceSet.h"
#define GUCEF_COMCORE_CRPCSERVICESET_H
#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */

#ifndef GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#include "gucefCOMCORE_CIRpcAccessPolicy.h"
#define GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CIRPCACCESSPOLICY_H ? */

#ifndef GUCEF_COMCORE_CRPCMETHODREF_H
#include "gucefCOMCORE_CRpcMethodRef.h"
#define GUCEF_COMCORE_CRPCMETHODREF_H
#endif /* GUCEF_COMCORE_CRPCMETHODREF_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLOPTIONS_H
#include "gucefCOMCORE_CRpcCallOptions.h"
#define GUCEF_COMCORE_CRPCCALLOPTIONS_H
#endif /* GUCEF_COMCORE_CRPCCALLOPTIONS_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTSTATUS_H
#include "gucefCOMCORE_CRpcRequestStatus.h"
#define GUCEF_COMCORE_CRPCREQUESTSTATUS_H
#endif /* GUCEF_COMCORE_CRPCREQUESTSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CIRPCSTREAM_H
#include "gucefCOMCORE_CIRpcStream.h"
#define GUCEF_COMCORE_CIRPCSTREAM_H
#endif /* GUCEF_COMCORE_CIRPCSTREAM_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTLINK_H
#include "gucefCOMCORE_CRpcEndpointLink.h"
#define GUCEF_COMCORE_CRPCENDPOINTLINK_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTLINK_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

#ifndef GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#include "gucefCOMCORE_CRpcDurableRequestTable.h"
#define GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#endif /* GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

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

class CRpcEndpoint;
class CRpcClientStream;
class CRpcHost;
class CRpcReflectionService;
typedef CORE::CTSharedPtr< CRpcEndpoint, MT::CMutex > CRpcEndpointPtr;

/*-------------------------------------------------------------------------*/

/**
 *  One RPC session with one peer (gucefCOMCORE RPC design 6 - 10): handshake, contract binding, calls in both
 *  directions, streams, ACCEPTED / RESULT, status queries and cancellation, deadlines, fragmentation, flow control and
 *  resume. Symmetric: either side exports services and either side calls.
 *
 *  Threading (10.9): the session runs on the pulse generator of its transport (the protocol thread); frames are
 *  received, decoded, dispatched and sent there, and timers run there. Events are raised through this CTSGNotifier to
 *  the notification pulse generator given at construction (directly when that is the protocol thread). Calls may be
 *  made from any thread: one recursive endpoint mutex guards the session state. Lock order: a lock the transport holds
 *  while delivering -> the endpoint lock -> the transport's outbound queue lock.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcEndpoint : public CORE::CTSGNotifier ,
                                              public CORE::CTSharedObjCreator< CRpcEndpoint, MT::CMutex >
{
    public:

    static const CORE::CEvent SessionEstablishedEvent;
    static const CORE::CEvent SessionSuspendedEvent;    /**< CRpcStatus: why the transport went */
    static const CORE::CEvent SessionResumedEvent;
    static const CORE::CEvent SessionGapEvent;          /**< CRpcSessionGapEventData: the ranges of 8.4, a new session follows */
    static const CORE::CEvent SessionClosedEvent;       /**< CRpcStatus */
    static const CORE::CEvent ContractBoundEvent;       /**< CRpcBindingEventData */
    static const CORE::CEvent AcceptedEvent;            /**< CRpcAcceptedEventData: call id, request id, DURABLE, eta */
    static const CORE::CEvent GapDetectedEvent;         /**< CRpcGapEventData: telemetry, 7.4 */

    static void RegisterEvents( void );

    /** A new endpoint; events go to the notification pulse generator */
    static CRpcEndpointPtr Create( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcEndpointSettings& settings );

    virtual ~CRpcEndpoint() GUCEF_VIRTUAL_OVERRIDE;

    /** Services this side exports; a host shares one set between its endpoints */
    bool RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                          CIRpcServiceImpl* impl, TRpcExecutionMode mode = RPC_EXECUTION_CALLING_THREAD );
    void SetServiceSet( const CRpcServiceSetPtr& services );
    CRpcServiceSetPtr GetServiceSet( void ) const;
    void SetAccessPolicy( const CIRpcAccessPolicyPtr& policy );
    void SetDurableRequestTable( const CRpcDurableRequestTablePtr& table );

    /** Contracts this side wants to call: sent as "wants" in the handshake */
    void AddWantedContract( const CORE::CString& name, UInt32 major, UInt32 minMinor = 0 );

    /** Creates a transport through the factory (the URI scheme is the type) and starts as the initiator */
    bool Connect( const CORE::CUri& remote );

    /** Starts the session over a transport: the initiator sends HELLO, the acceptor waits for it */
    bool Start( const CIRpcTransportPtr& transport, bool initiator );

    /** Resume over a new transport (8.5); for the acceptor: the transport on which the resume HELLO will arrive */
    bool Rebind( const CIRpcTransportPtr& transport );

    void Close( bool graceful = true );

    /** Resolves "<service full name>/Method" once against the bound contract: the result is reused for every call */
    bool ResolveMethod( const CORE::CString& methodPath, CRpcMethodRef& ref, CRpcStatus& status ) const;
    CRpcMessage NewRequest( const CRpcMethodRef& ref ) const;

    /** True when the contract is bound (BIND sent; BIND_ACK may still be on the way) */
    bool IsContractBound( const CORE::CString& name, UInt32 major ) const;

    /**
     *  Blocking (10.9). True when an answer (which may be an error status) came back. On the protocol thread it pumps
     *  the protocol pulse generator; from another known thread it waits on the future; refused when the protocol
     *  thread is unknown or for a nested blocking call inside a handler.
     */
    bool Call( const CRpcMethodRef& ref, const CRpcMessage& request, CRpcMessage& response,
               CRpcStatus& status, const CRpcCallOptions& options = CRpcCallOptions() );
    bool Call( const CORE::CString& methodPath, const CRpcMessage& request, CRpcMessage& response,
               CRpcStatus& status, const CRpcCallOptions& options = CRpcCallOptions() );

    /** Asynchronous: the future's result data is a CRpcCallResult; ACCEPTED raises AcceptedEvent, the future stays pending */
    CORE::CFutureResult CallAsync( const CRpcMethodRef& ref, const CRpcMessage& request,
                                   const CRpcCallOptions& options = CRpcCallOptions() );
    CORE::CFutureResult CallAsync( const CORE::CString& methodPath, const CRpcMessage& request,
                                   const CRpcCallOptions& options = CRpcCallOptions() );
    static bool TryGetCallResult( const CORE::CFutureResult& future, CRpcCallResult& result );

    /** Streaming calls; firstRequest is the request (server streams) or an optional first message */
    CIRpcStreamPtr OpenStream( const CRpcMethodRef& ref, const CRpcMessage* firstRequest,
                               const CRpcCallOptions& options = CRpcCallOptions() );

    /** Accepted work by request id (6.7); the futures' result data is a CRpcRequestStatus */
    CORE::CFutureResult CancelRequestAsync( const CRpcRequestId& id );
    bool CancelRequest( const CRpcRequestId& id, CRpcStatus& status );
    CORE::CFutureResult QueryStatusAsync( const CRpcRequestId& id, bool resendResultWhenDone );
    bool QueryStatus( const CRpcRequestId& id, bool resendResultWhenDone, CRpcRequestStatus& requestStatus, CRpcStatus& status );
    static bool TryGetRequestStatus( const CORE::CFutureResult& future, CRpcRequestStatus& requestStatus );

    TRpcSessionState GetState( void ) const;
    bool IsInitiator( void ) const;
    const CRpcPrincipal& GetPeerPrincipal( void ) const;
    UInt32 GetPeerContractMinor( const CORE::CString& name, UInt32 major ) const;
    CIRpcTransportPtr GetTransport( void ) const;
    CORE::PulseGeneratorPtr GetProtocolPulseGenerator( void ) const;
    const CRpcEndpointSettings& GetSettings( void ) const;

    /** The session identity (16 bytes); false before HELLO_ACK */
    bool GetSessionId( UInt8* sessionId ) const;

    /** Counters (16.3): frames, bytes, retransmissions, duplicates, gaps, resumes, refused resumes, calls by status */
    void GetTelemetry( CORE::CDataNode& telemetry ) const;

    /** Session numbers, for diagnostics and tests */
    UInt64 GetNextSendSeq( void ) const;
    UInt64 GetDeliveredSeq( void ) const;
    UInt32 GetOpenCallCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CORE::CTSharedObjCreator< CRpcEndpoint, MT::CMutex >;
    friend class CRpcServerCall;
    friend class CRpcClientStream;
    friend class CRpcHost;
    friend class CRpcReflectionService;
    class CSessionSink;
    friend class CSessionSink;
    struct SClientCall;
    struct SOutBinding;
    struct SInBinding;
    struct SAssembly;
    struct SFragmenter;
    struct SPendingQuery;
    struct SRequestInfo;
    struct SDeferredEvent;
    struct SPendingHandler;
    class CLockScope;
    friend class CLockScope;

    CRpcEndpoint( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcEndpointSettings& settings );
    CRpcEndpoint( const CRpcEndpoint& src );                    /**< not implemented */
    CRpcEndpoint& operator=( const CRpcEndpoint& src );         /**< not implemented */

    /* used by CRpcHost: a transport whose first frame (a HELLO) the host read */
    bool StartAccepted( const CIRpcTransportPtr& transport, const UInt8* helloFrame, UInt32 size, bool sessionUnknown );
    bool AcceptResume( const CIRpcTransportPtr& transport, const UInt8* helloFrame, UInt32 size );
    void SetHost( CRpcHost* host );

    /* used by CRpcServerCall */
    bool ServerAnswer( CRpcServerCall& call, UInt8 frameType, const CRpcStatus& status, const CRpcMessage* response );
    bool ServerAccept( CRpcServerCall& call, const CRpcRequestId& id, bool durable, UInt32 etaMs, UInt32 retryAfterMs );
    bool ServerCompleteResult( CRpcServerCall& call, const CRpcStatus& status, const CRpcMessage* response );
    void ServerSetProgress( CRpcServerCall& call, UInt32 permille, UInt32 etaMs, const CORE::CString& note );
    bool ServerWrite( CRpcServerCall& call, const CRpcMessage& message );
    void ServerConsumed( CRpcServerCall& call, UInt32 count, bool explicitGrant );

    /* used by CRpcClientStream */
    bool StreamWrite( CRpcClientStream& stream, const CRpcMessage& message );
    bool StreamWritesDone( CRpcClientStream& stream );
    void StreamCancel( CRpcClientStream& stream, const CRpcStatus& status );
    void StreamConsumed( CRpcClientStream& stream, UInt32 count, bool explicitGrant );

    /* transport events, on the protocol thread */
    void OnTransportConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportFrame( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProtocolPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDispatchToNotificationThread( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /* the session sink */
    bool SessionTransmit( const UInt8* frame, UInt32 size, TRpcProtectionClass protection );
    void SessionDeliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith );
    void SessionProtocolViolation( const CRpcStatus& status );
    void SessionGap( UInt64 from, UInt64 to );
    void SessionWritable( void );

    /* frames */
    void HandleFrame( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith );
    void HandleHello( const UInt8* body, UInt32 size );
    void HandleHelloAck( const UInt8* body, UInt32 size );
    void HandlePing( const UInt8* body, UInt32 size );
    void HandlePong( const UInt8* body, UInt32 size );
    void HandleStreamCredit( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleAbort( const UInt8* body, UInt32 size );
    void HandleGetContract( const UInt8* body, UInt32 size );
    void HandleContract( const UInt8* body, UInt32 size );
    void HandleBind( const UInt8* body, UInt32 size );
    void HandleBindAck( const UInt8* body, UInt32 size );
    void HandleRequest( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith );
    void ProcessRequest( const SRequestInfo& info, const UInt8* payload, UInt32 payloadSize, TRpcProtectionClass arrivedWith );
    bool StartAssembly( const SRpcFrameHeader& header, const UInt8* frame, UInt32 prefixSize, UInt64 total,
                        const UInt8* chunk, UInt32 chunkSize, TRpcProtectionClass arrivedWith, const SRequestInfo* requestInfo );
    void HandleResponse( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith );
    void HandleAccepted( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleResult( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith );
    void HandleStreamMsg( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith );
    void HandleHalfClose( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleCancel( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleStatusQuery( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleStatus( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleContinuation( const SRpcFrameHeader& header, const UInt8* body, UInt32 size, TRpcProtectionClass arrivedWith );
    void HandleError( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );
    void HandleGoodbye( const SRpcFrameHeader& header, const UInt8* body, UInt32 size );

    /* sending */
    bool SendSessionFrame( UInt8 type, UInt8 flags, const CORE::CDynamicBuffer& fields, const void* payload, UInt32 payloadSize,
                           TRpcProtectionClass protection, UInt64& seq, CRpcStatus& refusal, UInt64 orderKey = 0 );
    static UInt64 CallOrderKey( UInt64 callRef, bool responder );
    SFragmenter* FindFragmenter( UInt64 orderKey ) const;
    bool SendControl( UInt8 type, UInt8 flags, const CORE::CDynamicBuffer& body );
    void SendHello( void );
    void SendAbort( const CRpcStatus& status );
    void SendErrorFrame( UInt64 replyToSeq, const CRpcStatus& status );
    void SendStreamCredit( UInt64 callRef, bool responder, UInt64 limit );
    void AnnounceCredits( void );
    void AnnounceAfterResume( void );
    bool HasPendingResendQuery( const CRpcRequestId& id ) const;
    UInt32 GetPeerBindingCount( void ) const;
    bool RefusesInsecureTransport( const CIRpcTransportPtr& transport ) const;
    CORE::CString GetDurableOwner( void ) const;
    bool MayQueryDurable( const CORE::CString& ownerSubject, const CRpcRequestId& id ) const;
    void PumpFragments( void );
    bool IssueCall( SClientCall& call, const CRpcMessage* request, CRpcStatus& refusal );
    bool SendServerCallFrame( CRpcServerCall& call, UInt8 type, const CORE::CDynamicBuffer& fields, const CORE::CDynamicBuffer* payload,
                              TRpcProtectionClass protection, CRpcStatus& refusal );

    /* handshake and binding */
    void FillLimits( SRpcLimits& limits ) const;
    void GetOffersFor( const CRpcPrincipal& principal, const TRpcContractWantVector* filter, TRpcContractOfferVector& offers ) const;
    void ApplyPeerLimits( const SRpcLimits& limits );
    void BindWants( void );
    void BindWant( UInt32 wantIndex );
    void SendBind( SOutBinding& binding );
    void FailBinding( SOutBinding& binding, const CRpcStatus& status );
    CRpcContractRegistry& GetRegistry( void ) const;
    void EstablishNewSession( const UInt8* sessionId, const UInt8* resumeToken );
    bool CreateSessionIdentity( UInt8* sessionId, UInt8* resumeToken );

    /* calls */
    SClientCall* FindClientCall( UInt64 callRef );
    void CompleteClientCall( UInt64 callRef, const CRpcStatus& status, const UInt8* payload, UInt32 payloadSize, bool sendCancel );
    void CompleteAllClientCalls( const CRpcStatus& status );
    void CancelAllServerCalls( const CRpcStatus& status );
    void DispatchServerCall( const CRpcServerCallPtr& call, TRpcExecutionMode mode );
    void EndServerCall( CRpcServerCall& call );
    CRpcServerCallPtr FindServerCall( UInt64 callRef ) const;
    void AnswerFrameError( UInt64 replyToSeq, UInt64 callRef, TRpcStatusCode code, const CORE::CString& message );
    void RetryIdempotentCalls( GUCEF::vector< SClientCall* >& calls );

    /* lifecycle */
    void AttachTransport( const CIRpcTransportPtr& transport );
    void SubscribeTransport( const CIRpcTransportPtr& transport );
    bool IsCurrentTransport( CORE::CNotifier* notifier ) const;
    void DetachTransport( bool close );
    void Suspend( const CRpcStatus& why );
    void EndSession( const CRpcStatus& status, bool sendAbort );
    void StartNewSessionAfterGap( const CRpcSessionGapEventData& gap, TRpcStatusCode callStatus );
    void ResetBindings( void );
    void Tick( UInt64 nowMs );
    void TryReconnect( UInt64 nowMs );
    void Reconnect( const CORE::CUri& remote );
    bool IsProtocolThread( void ) const;
    bool WaitForFuture( const CORE::CFutureResult& future, const char* what, CRpcStatus& status );
    void RaiseEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /*
     *  The endpoint lock (10.9). Events of the endpoint, its streams and its server calls are never raised while it is
     *  held: they wait in a list and go out, in order, when the outermost hold of the lock ends (by one thread at a
     *  time). The threads that deliver them take the endpoint lock in their handlers.
     */
    void LockState( void ) const;
    void UnlockState( void ) const;
    void RaiseDeferredEvents( void );
    void RunPendingHandlers( void );
    void DeferEvent( SDeferredEvent* deferred );
    void RaiseCallEvent( const CRpcServerCallPtr& call, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL );
    void StreamMessage( SClientCall& call, const CRpcMessage& message, UInt64 streamIndex );
    void StreamEnd( const CIRpcStreamPtr& stream, const CRpcStatus& status, const CRpcMessage* response );
    void StreamWritable( const CIRpcStreamPtr& stream );
    void RunDispatch( const CRpcServerCallPtr& call, TRpcExecutionMode mode );
    void RunPulseRequest( const SDeferredEvent& request );
    TRpcProtectionClass ProtectionForFrame( UInt8 type, const CRpcMethodDescriptor* method, bool responder ) const;
    bool CheckProtection( const CRpcMethodDescriptor* method, bool providerFrame, TRpcProtectionClass arrivedWith );

    static UInt64 NowMs( void );
    static UInt64 WallClockMs( void );

    static const CORE::CEvent DispatchToNotificationThreadEvent;
    static const CORE::CEvent ReleaseOnNotificationThreadEvent;    /**< its data holds a reference: the last one goes on that thread */

    typedef GUCEF::map< UInt64, SClientCall* > TClientCallMap;
    typedef GUCEF::map< UInt64, CRpcServerCallPtr > TServerCallMap;
    typedef GUCEF::map< UInt32, SOutBinding* > TOutBindingMap;
    typedef GUCEF::map< UInt32, SInBinding* > TInBindingMap;
    typedef GUCEF::map< UInt64, SAssembly* > TAssemblyMap;
    typedef GUCEF::vector< SFragmenter* > TFragmenterVector;
    typedef GUCEF::map< UInt64, SPendingQuery* > TPendingQueryMap;
    typedef GUCEF::map< CRpcRequestId, UInt64 > TRequestIdMap;
    typedef GUCEF::map< CRpcRequestId, CRpcRequestStatus > TOrphanResultMap;

    typedef GUCEF::vector< SDeferredEvent* > TDeferredEventVector;
    typedef GUCEF::vector< SPendingHandler > TPendingHandlerVector;

    mutable MT::CMutex m_lock;
    mutable UInt32 m_lockDepth;                         /**< nesting of the holds of m_lock by the thread that has it */
    mutable TDeferredEventVector m_deferredEvents;      /**< events raised under the lock, raised when it is released */
    mutable bool m_raisingDeferredEvents;
    mutable TPendingHandlerVector m_pendingHandlers;    /**< RPC_EXECUTION_CALLING_THREAD handlers, run by the thread that dispatched them once it released the lock */
    CRpcEndpointSettings m_settings;
    CRpcEndpointLinkPtr m_link;
    CSessionSink* m_sink;
    CRpcSession m_session;
    CIRpcTransportPtr m_transport;
    CORE::PulseGeneratorPtr m_protocolPulseGenerator;
    CRpcProtocolTaskPtr m_protocolTask;
    CORE::CObserver m_transportObserver;
    GUCEF::vector< CIRpcTransportPtr > m_staleTransports;      /**< detached, unsubscribed at the next pulse outside the lock */
    CORE::CObserver m_selfObserver;
    CRpcHost* m_host;
    CORE::CUri m_remoteUri;
    bool m_initiator;
    bool m_started;
    TRpcSessionState m_state;
    UInt32 m_sessionGeneration;
    UInt32 m_bindGeneration;
    bool m_hasIdentity;
    CORE::CString m_lineageOwner;               /**< "session:<first session id>", owner of DURABLE ids of unauthenticated principals */
    UInt8 m_sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt8 m_resumeToken[ GUCEF_RPC_RESUMETOKEN_SIZE ];
    UInt64 m_resumeCount;
    UInt8 m_pendingNonce[ GUCEF_RPC_NONCE_SIZE ];
    bool m_resumePending;
    SRpcLimits m_peerLimits;
    CORE::CString m_peerNodeName;
    TRpcContractOfferVector m_peerOffers;
    TRpcContractWantVector m_peerWants;
    TRpcContractWantVector m_wants;
    CRpcPrincipal m_principal;
    bool m_principalPinned;
    CRpcServiceSetPtr m_services;
    CIRpcAccessPolicyPtr m_accessPolicy;
    CRpcDurableRequestTablePtr m_durable;
    CRpcContractPtr m_rpcContract;
    TOutBindingMap m_outBindings;
    TInBindingMap m_inBindings;
    UInt32 m_nextBindIndex;
    UInt32 m_getContractCount;
    CRpcMetadataInternTable m_outKeys;
    CRpcMetadataInternTable m_inKeys;
    TClientCallMap m_clientCalls;
    TServerCallMap m_serverCalls;
    GUCEF::vector< CRpcServerCallPtr > m_retiredServerCalls;     /**< ended, released at the next pulse */
    bool m_pumpingFragments;
    TRequestIdMap m_acceptedIds;
    TPendingQueryMap m_pendingQueries;
    TOrphanResultMap m_orphanResults;
    TAssemblyMap m_assemblies;
    UInt64 m_assemblyBytes;                     /**< the announced sizes of the open assemblies together */
    TFragmenterVector m_fragmenters;
    UInt32 m_fragmenterCursor;
    GUCEF::vector< SClientCall* > m_waitingCalls;     /**< issued while suspended or before the binding was sent */
    UInt64 m_lastReceiveMs;
    UInt64 m_lastSendMs;
    UInt64 m_pingSentMs;
    UInt64 m_pingId;
    UInt64 m_suspendedSinceMs;
    UInt64 m_nextReconnectMs;
    bool m_reconnectDue;                        /**< Reconnect() at the end of the protocol pulse, without the lock */
    UInt32 m_reconnectDelayMs;
    UInt64 m_helloSentMs;
    UInt64 m_goodbyeSeq;
    UInt64 m_closeRequestedMs;
    bool m_closeGraceful;
    UInt32 m_blockingDepth;
    UInt32 m_handlerDepth;
    CRpcStatus m_closeStatus;
    CORE::CDynamicBuffer m_lastHelloAck;
    UInt64 m_nextCreditResendMs;
    UInt64 m_nextCallScanMs;
    GUCEF::set< UInt64 > m_queueBlockedStreams;   /**< client streams whose Write() found the send queue full: WritableEvent when it drains */            /**< the deadline and cancellation scan of the open calls runs once per tick interval, not per pulse */
    UInt8 m_unknownResumeId[ GUCEF_RPC_SESSIONID_SIZE ];  /**< the session a SESSION_UNKNOWN answer was for */
    bool m_hasUnknownResumeId;
    CORE::CDynamicBuffer m_scratch;
    CORE::CDynamicBuffer m_controlBody;         /**< bodies of STREAM_CREDIT, PING and PONG */
    GUCEF::map< CORE::CString, UInt64 > m_counters;
    CRpcReflectionService* m_reflection;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */
