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

#ifndef GUCEF_COM_CMCPSERVER_H
#define GUCEF_COM_CMCPSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_COM_CMCPSERVERSETTINGS_H
#include "gucefCOM_CMcpServerSettings.h"
#define GUCEF_COM_CMCPSERVERSETTINGS_H
#endif /* GUCEF_COM_CMCPSERVERSETTINGS_H ? */

#ifndef GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#include "gucefCOM_CMcpImplementationInfo.h"
#define GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#endif /* GUCEF_COM_CMCPIMPLEMENTATIONINFO_H ? */

#ifndef GUCEF_COM_CMCPTOOLREGISTRY_H
#include "gucefCOM_CMcpToolRegistry.h"
#define GUCEF_COM_CMCPTOOLREGISTRY_H
#endif /* GUCEF_COM_CMCPTOOLREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPRESOURCEREGISTRY_H
#include "gucefCOM_CMcpResourceRegistry.h"
#define GUCEF_COM_CMCPRESOURCEREGISTRY_H
#endif /* GUCEF_COM_CMCPRESOURCEREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPPROMPTREGISTRY_H
#include "gucefCOM_CMcpPromptRegistry.h"
#define GUCEF_COM_CMCPPROMPTREGISTRY_H
#endif /* GUCEF_COM_CMCPPROMPTREGISTRY_H ? */

#ifndef GUCEF_COM_CIMCPACCESSPOLICY_H
#include "gucefCOM_CIMcpAccessPolicy.h"
#define GUCEF_COM_CIMCPACCESSPOLICY_H
#endif /* GUCEF_COM_CIMCPACCESSPOLICY_H ? */

#ifndef GUCEF_COM_CMCPAUDITRECORD_H
#include "gucefCOM_CMcpAuditRecord.h"
#define GUCEF_COM_CMCPAUDITRECORD_H
#endif /* GUCEF_COM_CMCPAUDITRECORD_H ? */

#ifndef GUCEF_COM_CMCPREQUESTSTATESEALER_H
#include "gucefCOM_CMcpRequestStateSealer.h"
#define GUCEF_COM_CMCPREQUESTSTATESEALER_H
#endif /* GUCEF_COM_CMCPREQUESTSTATESEALER_H ? */

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPSUBSCRIPTION_H
#include "gucefCOM_CMcpSubscription.h"
#define GUCEF_COM_CMCPSUBSCRIPTION_H
#endif /* GUCEF_COM_CMCPSUBSCRIPTION_H ? */

#ifndef GUCEF_COM_CMCPOPERATIONTASK_H
#include "gucefCOM_CMcpOperationTask.h"
#define GUCEF_COM_CMCPOPERATIONTASK_H
#endif /* GUCEF_COM_CMCPOPERATIONTASK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of ChannelOpenedEvent and ChannelClosedEvent */
struct GUCEF_COM_PUBLIC_CPP SMcpChannelEventInfo
{
    UInt64 channelId;
    CORE::CString transport;
    CORE::CString subject;

    SMcpChannelEventInfo( void );
};
typedef CORE::CTCloneableObj< SMcpChannelEventInfo > TMcpChannelEventData;

/** The data of RequestStartedEvent and RequestFinishedEvent */
struct GUCEF_COM_PUBLIC_CPP SMcpRequestEventInfo
{
    CORE::CString method;
    CORE::CString target;               /**< tool or prompt name, resource URI */
    CORE::CString requestId;            /**< as text; empty for in-process calls */
    CORE::CString transport;
    CORE::CString subject;
    UInt64 channelId;                   /**< 0 for in-process calls */
    EMcpProtocolEra era;
    CORE::CString protocolVersion;
    EMcpRequestOutcome outcome;         /**< RequestFinishedEvent only */
    Int32 errorCode;                    /**< for MCP_OUTCOME_PROTOCOL_ERROR */
    UInt64 durationInMs;                /**< RequestFinishedEvent only */

    SMcpRequestEventInfo( void );
};
typedef CORE::CTCloneableObj< SMcpRequestEventInfo > TMcpRequestEventData;

/** The data of AuditEvent */
typedef CORE::CTCloneableObj< CMcpAuditRecord > TMcpAuditEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Model Context Protocol server: protocol revision 2026-07-28, stateless and versioned per request, plus the
 *  2025-11-25 era for clients that open with initialize. Applications register tools, resources, resource templates
 *  and prompts in its registries; the server does the protocol, validation, access policy, the sealing of MRTR state,
 *  cancellation, progress and subscriptions. Transports open channels on it.
 *
 *  Threading. The server has one protocol thread: the thread that drives its pulse generator. Channels, dispatch,
 *  the legacy sessions, subscriptions and every send to a transport live there, so the request path takes no locks
 *  except the shared locks of the registries. Pool tasks and deferred handlers hand in their outcome through the
 *  request context, whose news the server takes in on its next pulse. The in-process calls (CallTool() and the like)
 *  may be made from any thread, as may NotifyResourceUpdated() and the registrations; the configuration is to be
 *  set before the server is used from other threads.
 *
 *  Where handlers run (EMcpExecutionMode) for requests from a transport: MCP_EXECUTION_CALLING_THREAD on the protocol
 *  thread inside the dispatch, MCP_EXECUTION_THREAD_POOL as a CMcpOperationTask on the thread pool. A handler of
 *  either mode may answer MCP_EXECUTE_DEFERRED and complete the request context later; its event driven work runs
 *  on CMcpRequestContext::GetPulseGenerator(): the pulse generator of the server for the calling thread, that of the
 *  task delegator in a pool task, and a local one of the calling thread in a blocking in-process call.
 *
 *  The thread pool: the one given to the constructor, else the pool named by the setting threadPoolName through
 *  CTaskManager::GetOrCreateThreadPool(), else the default pool.
 */
class GUCEF_COM_PUBLIC_CPP CMcpServer : public CORE::CObservingNotifier         ,
                                        public CORE::CIConfigurable             ,
                                        private CIMcpRequestContextListener
{
    public:

    static const CORE::CEvent ChannelOpenedEvent;           /**< TMcpChannelEventData */
    static const CORE::CEvent ChannelClosedEvent;           /**< TMcpChannelEventData */
    static const CORE::CEvent RequestStartedEvent;          /**< TMcpRequestEventData */
    static const CORE::CEvent RequestFinishedEvent;         /**< TMcpRequestEventData with the outcome and the duration */
    static const CORE::CEvent AuditEvent;                   /**< TMcpAuditEventData: one record per request, raised right after RequestFinishedEvent */
    static const CORE::CEvent TransportErrorEvent;          /**< TCloneableString */
    static const CORE::CEvent ShutdownRequestedEvent;       /**< stdin closed, or a transport asked to stop the process */

    /*
     *  Every event is raised on the protocol thread: a notifier is not to notify from two threads at once. The request
     *  events of in-process calls made on other threads are queued and raised on the next pulse, in order.
     */

    /** How often the server looks at its requests in flight while it has any: timeouts, held progress */
    static const UInt32 HousekeepingIntervalInMs = 10;

    /**
     *  The most audit records of transport refusals (AuditTransportRefusal()) a second. The refusals beyond it are
     *  counted, and one record with the count follows once the second is over, so that a flood of refused requests
     *  cannot flood the audit trail
     */
    static const UInt32 MaxRefusalAuditsPerSecond = 20;

    static void RegisterEvents( void );

    /**
     *  @param pulseGenerator   the pulse generator whose thread is the protocol thread; empty: the application's
     *  @param threadPool       the pool handlers of MCP_EXECUTION_THREAD_POOL run on; empty: see the class description
     */
    explicit CMcpServer( CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ,
                         CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr()              );

    virtual ~CMcpServer() GUCEF_VIRTUAL_OVERRIDE;

    /** _meta serverInfo of every modern result, serverInfo of the legacy InitializeResult */
    void SetServerInfo( const CMcpImplementationInfo& info );

    const CMcpImplementationInfo& GetServerInfo( void ) const;

    void SetInstructions( const CORE::CString& instructions );

    /** Empty: the built-in default, which allows read-only tools, all resources, prompts and subscriptions, and hides the rest */
    void SetAccessPolicy( const CIMcpAccessPolicyPtr& policy );

    CIMcpAccessPolicyPtr GetAccessPolicy( void ) const;

    CMcpToolRegistry& GetToolRegistry( void );

    CMcpResourceRegistry& GetResourceRegistry( void );

    CMcpPromptRegistry& GetPromptRegistry( void );

    bool AddTransport( const CIMcpTransportPtr& transport );

    bool RemoveTransport( const CIMcpTransportPtr& transport );

    /** Starts every transport that was added */
    bool Start( void );

    /**
     *  Stops the server. Graceful: every subscription gets its closing result (and on stream channels the
     *  notifications/cancelled of its listen request), requests in flight are cancelled and pool tasks are given
     *  up to cancelGraceMs to end. Not graceful: subscriptions and requests are dropped without a message.
     */
    void Stop( bool graceful );

    bool IsStarted( void ) const;

    /** Thread safe. Sends notifications/resources/updated to the subscriptions of the URI, on the next pulse */
    void NotifyResourceUpdated( const CORE::CString& uri );

    /**
     *  Blocking: lookup, policy, validation and Execute() all on the calling thread. A DEFERRED tool is driven by a
     *  local pulse generator on the calling thread until it completes, or it times out.
     *  In-process calls are modern requests without client capabilities, so a tool that needs client input fails.
     *  @return true when the tool gave a result, which can be a tool execution error (result.isError);
     *          false with the reason in 'error' for an unknown or hidden tool and other protocol errors
     */
    bool CallTool( const CORE::CString& name            ,
                   const CORE::CDataNode& arguments     ,
                   const CMcpPrincipal& principal       ,
                   CMcpToolResult& result               ,
                   CORE::CString& error                 );

    /** Asynchronous CallTool(): a CMcpOperationTask on the thread pool. See TryGetToolResult() */
    CORE::CFutureResult CallToolAsync( const CORE::CString& name, const CORE::CDataNode& arguments, const CMcpPrincipal& principal );

    bool ReadResource( const CORE::CString& uri, const CMcpPrincipal& principal, CMcpResourceContents& contents, CORE::CString& error );

    CORE::CFutureResult ReadResourceAsync( const CORE::CString& uri, const CMcpPrincipal& principal );

    bool GetPrompt( const CORE::CString& name, const CORE::CValueList& arguments, const CMcpPrincipal& principal, CMcpPromptResult& result, CORE::CString& error );

    CORE::CFutureResult GetPromptAsync( const CORE::CString& name, const CORE::CValueList& arguments, const CMcpPrincipal& principal );

    /**
     *  The outcome of an asynchronous in-process call, once its future is ready
     *  @return false while it is not ready or when the future is not one of these calls; else what the blocking call returns
     */
    static bool TryGetToolResult( const CORE::CFutureResult& future, CMcpToolResult& result, CORE::CString& error );
    static bool TryGetResourceContents( const CORE::CFutureResult& future, CMcpResourceContents& contents, CORE::CString& error );
    static bool TryGetPromptResult( const CORE::CFutureResult& future, CMcpPromptResult& result, CORE::CString& error );

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    const CMcpServerSettings& GetSettings( void ) const;

    /** Applies settings; the requestState sealer is prepared again when its settings changed */
    bool SetSettings( const CMcpServerSettings& settings );

    const CMcpRequestStateSealer& GetRequestStateSealer( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the settings from the "McpServer" child of cfg, or from cfg itself when it has none */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // for transports, on the protocol thread

    /** Opens a channel a transport made: it gets its id and is attached. False when it is open already */
    bool OpenChannel( const CMcpChannelPtr& channel );

    /** The channel is gone: modern requests in flight are cancelled, its subscriptions end without a message */
    void CloseChannel( const CMcpChannelPtr& channel );

    /**
     *  For transports, on the protocol thread: the peer is gone for good (the client closed the standard input) or the
     *  transport cannot go on. Raises ShutdownRequestedEvent with the reason as TCloneableString. The application owns its main
     *  loop and ends the process on that event; the server never does, and a transport calls this only where its
     *  settings (stopOnStdinClosed) say so.
     */
    void RequestShutdown( const CORE::CString& reason );

    /**
     *  For transports, on the protocol thread: a request was refused before it could be dispatched, for its
     *  authentication or for where it came from (an HTTP 401, a Host or Origin 403). Raises one AuditEvent with the
     *  outcome MCP_OUTCOME_REFUSED, not authenticated, and the reason, which the transport writes without any text of
     *  the client in it. No request events are raised: there is no request yet. At most MaxRefusalAuditsPerSecond such
     *  records a second; the rest are summed up in one record.
     *  @param principal    who the transport took the client for: its transport and subject; it is recorded as not authenticated
     *  @param errorCode    the JSON-RPC error code of the refusal, 0 when it had none
     */
    void AuditTransportRefusal( const CMcpPrincipal& principal, const CORE::CString& reason, Int32 errorCode = 0 );

    UInt32 GetChannelCount( void ) const;

    UInt32 GetInFlightCount( void ) const;

    UInt32 GetSubscriptionCount( void ) const;

    private:

    friend class CMcpChannel;

    struct SDispatch;
    struct SInFlight;
    struct SChannelState;

    typedef GUCEF::map< CORE::CString, SInFlight* > TInFlightMap;
    typedef GUCEF::map< CORE::CString, CMcpSubscription > TSubscriptionMap;
    typedef GUCEF::map< UInt64, SChannelState* > TChannelStateMap;
    typedef GUCEF::map< CMcpRequestContext*, SInFlight* > TContextMap;
    typedef GUCEF::vector< CMcpRequestContext* > TContextPtrVector;
    typedef GUCEF::vector< CIMcpTransportPtr > TTransportVector;
    typedef GUCEF::vector< CIMcpTransport* > TTransportPtrVector;
    typedef GUCEF::vector< CORE::CFutureResult > TFutureVector;

    struct STokenBucket
    {
        Float64 tokens;
        UInt64 lastRefillInMs;
    };
    typedef GUCEF::map< CORE::CString, STokenBucket > TTokenBucketMap;

    CMcpServer( const CMcpServer& src );
    CMcpServer& operator=( const CMcpServer& src );

    // from the channels
    void OnChannelRequest( CMcpChannel& channel, const SJsonRpcRequest& request );
    void OnChannelNotification( CMcpChannel& channel, const SJsonRpcNotification& notification );

    // CIMcpRequestContextListener
    virtual void OnRequestContextNews( CMcpRequestContext& context ) GUCEF_VIRTUAL_OVERRIDE;

    // events
    void SendIdInUseError( SDispatch& dispatch );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void FlushRefusalSummary( UInt64 now, bool force );
    void OnTransportError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToolsListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResourcesListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPromptsListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    // dispatch
    void DispatchModern( SDispatch& dispatch, const SJsonRpcRequest& request, const CORE::CDataNode* params );
    void DispatchLegacy( SDispatch& dispatch, const SJsonRpcRequest& request, const CORE::CDataNode* params );
    void HandleDiscover( SDispatch& dispatch );
    void HandleList( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandleToolCall( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandleResourceRead( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandlePromptGet( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandleListen( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandleInitialize( SDispatch& dispatch, const CORE::CDataNode* params );
    void HandleLegacySubscribe( SDispatch& dispatch, const CORE::CDataNode* params, bool subscribe );

    bool PrepareCall( SDispatch& dispatch, const CORE::CDataNode* params, const CORE::CDataNode* arguments, CMcpOperationTaskData& call );
    void RunCall( SDispatch& dispatch, CMcpOperationTaskData& call, EMcpExecutionMode mode, UInt32 maxDurationMs );
    void FinishCall( SDispatch& dispatch );
    void SendCallOutcome( SDispatch& dispatch, EMcpRequestOutcome& outcome, Int32& errorCode );

    bool BuildToolResultNode( SDispatch& dispatch, const CMcpToolResult& result, CORE::CDataNode& node, CORE::CString& error );
    bool BuildInputRequiredNode( SDispatch& dispatch, const CMcpInputRequired& inputRequired, CORE::CDataNode& node,
                                 Int32& errorCode, CORE::CString& errorMessage, CORE::CDataNode& errorData );
    void GetResourceCacheHints( SDispatch& dispatch, const CMcpResourceContents& contents, UInt32& ttlMs, EMcpCacheScope& scope ) const;

    void AddResultMeta( const SDispatch& dispatch, CORE::CDataNode& result ) const;
    void AddCacheHints( const SDispatch& dispatch, CORE::CDataNode& result, UInt32 ttlMs, EMcpCacheScope scope ) const;
    void SendResult( SDispatch& dispatch, CORE::CDataNode& result, EMcpRequestOutcome outcome );
    void SendError( SDispatch& dispatch, Int32 code, const CORE::CString& message, const CORE::CDataNode* data = GUCEF_NULL );
    void RaiseRequestFinished( const SDispatch& dispatch, EMcpRequestOutcome outcome, Int32 errorCode );
    void RaiseRequestStarted( const SDispatch& dispatch );
    void RaiseRequestEvent( bool finished, const SMcpRequestEventInfo& info, const CMcpAuditRecord* audit = GUCEF_NULL );
    void BuildAuditRecord( const SDispatch& dispatch, EMcpRequestOutcome outcome, Int32 errorCode, const SMcpRequestEventInfo& info, CMcpAuditRecord& record ) const;
    void CaptureAuditArguments( SDispatch& dispatch, const CORE::CDataNode* arguments, const CMcpToolEntry* tool ) const;
    static void NoteDecision( SDispatch& dispatch, EMcpAccessDecision decision, const CORE::CString& reason );
    static void NoteNotVisible( SDispatch& dispatch, bool found );
    void RaiseQueuedRequestEvents( void );
    bool IsProtocolThread( void ) const;

    // in flight
    SChannelState* GetChannelState( UInt64 channelId );
    bool IsInFlightLimitReached( UInt64 channelId ) const;
    bool TakeToolCallToken( const CMcpPrincipal& principal );
    void RegisterInFlight( SDispatch& dispatch, UInt32 maxDurationMs );
    void EndInFlight( SInFlight* inFlight );
    void CancelInFlight( SInFlight* inFlight, EMcpRequestOutcome outcome );
    void ProcessContextNews( void );
    void ProcessHeldProgress( UInt64 now );
    void ProcessTimeouts( UInt64 now );
    void ProcessResourceUpdates( void );
    void SendProgress( SInFlight& inFlight, const CMcpRequestContext::SProgress& progress, UInt64 now );
    void UpdatePeriodicPulses( void );

    // subscriptions
    void SendListChanged( const CORE::CString& method, int which );
    void EndSubscription( SChannelState& state, CMcpSubscription& subscription, bool graceful );

    // in-process
    bool PrepareInProcessContext( const CORE::CString& method, const CORE::CString& target, const CMcpPrincipal& principal,
                                  SDispatch& dispatch );
    bool RunInProcess( SDispatch& dispatch, CMcpOperationTaskData& call, UInt32 maxDurationMs, CORE::CString& error );
    CORE::CFutureResult QueueInProcess( CMcpOperationTaskData& data );
    CORE::ThreadPoolPtr GetThreadPool( void );

    // helpers
    bool ApplySettings( const CMcpServerSettings& settings );
    const CIMcpAccessPolicy& GetPolicy( CIMcpAccessPolicyPtr& holder ) const;
    UInt64 NowUnixMs( void ) const;
    static UInt64 NowMonotonicMs( void );
    static CORE::CString IdKey( const CORE::CVariant& id );
    static bool IsValidRequestId( const CORE::CVariant& id );

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::ThreadPoolPtr m_threadPool;
    CMcpServerSettings m_settings;
    CMcpImplementationInfo m_serverInfo;
    CORE::CDataNode m_serverInfoNode;
    mutable MT::CMutex m_configLock;
    CIMcpAccessPolicyPtr m_policy;
    CIMcpAccessPolicyPtr m_defaultPolicy;
    CMcpToolRegistry m_toolRegistry;
    CMcpResourceRegistry m_resourceRegistry;
    CMcpPromptRegistry m_promptRegistry;
    CMcpRequestStateSealer m_sealer;
    TTransportVector m_transports;
    TTransportPtrVector m_configuredTransports;     /**< those made by LoadConfig(), replaced when the configuration is loaded again */
    bool m_started;
    bool m_requestingPulses;

    TChannelStateMap m_channels;
    UInt64 m_nextChannelId;
    TContextMap m_inFlightByContext;
    UInt32 m_inFlightTotal;
    UInt32 m_subscriptionsTotal;
    GUCEF::set< SInFlight* > m_heldProgress;

    MT::CMutex m_mailboxLock;
    TContextPtrVector m_mailbox;
    TContextPtrVector m_mailboxWork;

    MT::CMutex m_updatesLock;
    CORE::CString::StringSet m_pendingResourceUpdates;

    MT::CMutex m_rateLock;
    TTokenBucketMap m_tokenBuckets;

    // audits of transport refusals: protocol thread only
    UInt64 m_refusalWindowStartInMs;
    UInt32 m_refusalsInWindow;
    UInt32 m_refusalsSuppressed;
    CORE::CString m_suppressedRefusalTransport;
    CORE::CString m_suppressedRefusalReason;

    MT::CMutex m_inProcessLock;
    TFutureVector m_inProcessFutures;

    struct SQueuedRequestEvent
    {
        bool finished;
        SMcpRequestEventInfo info;
        bool hasAudit;
        CMcpAuditRecord audit;
    };
    MT::CMutex m_eventQueueLock;
    GUCEF::vector< SQueuedRequestEvent > m_queuedRequestEvents;
    MT::CTAtomic< TThreadId > m_protocolThreadId;     /**< the thread that pulses, as the last pulse found it */
    GUCEF_SYSTEM_THREAD_ID_CODE( MT::CTAtomic< TSystemThreadId > m_protocolSystemThreadId; )   /**< display only, recorded with m_protocolThreadId */

    // list snapshots: rebuilt when the generation of the registry changes; protocol thread only
    UInt64 m_toolSnapshotGeneration;
    CMcpToolRegistry::TToolEntryVector m_toolSnapshot;
    UInt64 m_resourceSnapshotGeneration;
    CMcpResourceRegistry::TResourceEntryVector m_resourceSnapshot;
    CMcpResourceRegistry::TTemplateEntryVector m_templateSnapshot;
    UInt64 m_promptSnapshotGeneration;
    CMcpPromptRegistry::TPromptEntryVector m_promptSnapshot;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSERVER_H ? */
