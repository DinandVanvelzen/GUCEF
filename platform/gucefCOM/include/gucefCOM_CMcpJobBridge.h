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

#ifndef GUCEF_COM_CMCPJOBBRIDGE_H
#define GUCEF_COM_CMCPJOBBRIDGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#include "gucefCORE_CCommandIdempotencyCache.h"
#define GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#ifndef GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#include "gucefCOM_CIMcpJobResultBuilder.h"
#define GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#endif /* GUCEF_COM_CIMCPJOBRESULTBUILDER_H ? */

#ifndef GUCEF_COM_CMCPJOBBRIDGESETTINGS_H
#include "gucefCOM_CMcpJobBridgeSettings.h"
#define GUCEF_COM_CMCPJOBBRIDGESETTINGS_H
#endif /* GUCEF_COM_CMCPJOBBRIDGESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpJobWaiter;

/**
 *  Long work of MCP tools as runs of a CORE::CJobScheduler, behind opaque job handles that are bound to the principal
 *  that started them. The protocol is stateless: a client keeps the handle and asks with it; the server never shows
 *  the scheduler's own ids.
 *
 *  - StartJob() defines the work as a submitted job of the scheduler (CJobScheduler::SubmitJob(); its task type must be
 *    registered as submittable by the owner of the tools) and runs it now. AdoptRun() takes a run started otherwise.
 *    An optional request key makes a start safe to repeat: the same key with the same arguments digest gives the job
 *    of the first start (the command idempotency cache of gucefCORE), the same key with other arguments is refused.
 *  - A handle resolves for the principal (subject) that started it only, or for one with the operator scope of the
 *    settings; anyone else gets JOB_UNKNOWN, as for a handle that never existed.
 *  - Wait() is what a job tool returns from Execute(): the result at once when the job is final or waitMs is 0,
 *    otherwise MCP_EXECUTE_DEFERRED and the call completes when the run ends or when waitMs passed, with the job
 *    object then. While it waits, the run's progress goes to the client as notifications/progress (when the request
 *    has a progress token). The wait polls the run on a timer of the request's own pulse generator
 *    (CMcpRequestContext::GetPulseGenerator()), so it works for calls on the protocol thread, on the pool and in
 *    process; no thread blocks. A cancelled call (client cancel, stream closed) ends the wait only: the job goes on.
 *    Only CancelJob() stops a job.
 *  - Handles of finished jobs stay valid for resultTtlMs, then JOB_EXPIRED. SaveState() and LoadState() keep the
 *    handles across a restart of the server; a job whose run did not end in the earlier process is interrupted.
 *
 *  Thread safe. The bridge lock is held while the scheduler is asked (lock order: bridge, then scheduler); it is never
 *  held while a request context or a result builder is called.
 */
class GUCEF_COM_PUBLIC_CPP CMcpJobBridge
{
    public:

    enum ELookup
    {
        JOB_FOUND   ,
        JOB_UNKNOWN ,   /**< no such handle, or not one of this principal */
        JOB_EXPIRED     /**< the handle of a job of this principal that ended longer than resultTtlMs ago */
    };

    enum EStartResult
    {
        JOBSTART_STARTED        ,   /**< a new job */
        JOBSTART_REPEATED       ,   /**< the request key was seen with the same arguments: the job of the first start */
        JOBSTART_KEY_CONFLICT   ,   /**< the request key was seen with other arguments */
        JOBSTART_BUSY           ,   /**< the principal has maxActivePerPrincipal jobs that are not final */
        JOBSTART_REFUSED            /**< the scheduler refused the job or the run; 'error' says why */
    };

    explicit CMcpJobBridge( const CORE::JobSchedulerPtr& scheduler );

    /** Completes the calls that still wait (with the job object), forgets the handles */
    ~CMcpJobBridge();

    const CORE::JobSchedulerPtr& GetScheduler( void ) const;

    /** False when the settings are not usable; nothing changes then */
    bool SetSettings( const CMcpJobBridgeSettings& settings );
    CMcpJobBridgeSettings GetSettings( void ) const;

    /** Tests: clocks of their own (not owned; GUCEF_NULL restores the system clocks) */
    void SetClocks( const CORE::CIUtcClock* utcClock, const CORE::CIMonotonicClock* monotonicClock );

    /**
     *  Starts a job: 'definition' gets a job id of the bridge and the trigger Manual and is submitted; then a run now
     *  with the principal's subject as requestedBy.
     *  @param kind             what the job does, shown in the job object
     *  @param requestKey       optional, 1 to 64 characters: a repeat with the same key and digest gives the same job
     *  @param argumentsDigest  of the arguments the key goes with; empty: not compared
     */
    EStartResult StartJob( const CMcpPrincipal& principal          ,
                           const CORE::CString& kind               ,
                           const CORE::CJobDefinition& definition  ,
                           const CORE::CString& requestKey         ,
                           const CORE::CString& argumentsDigest    ,
                           CMcpJobInfo& job                        ,
                           CORE::CString& error                    );

    /** A handle for a run that was started otherwise (a run now of a job defined in code); false when the run is not known */
    bool AdoptRun( const CMcpPrincipal& principal, const CORE::CString& kind, UInt64 runId, CMcpJobInfo& job );

    ELookup GetJob( const CMcpPrincipal& principal, const CORE::CString& jobId, CMcpJobInfo& job );

    /** Asks the run to cancel (its task polls for it); a final job is left as it is. JOB_FOUND with the job as it then is */
    ELookup CancelJob( const CMcpPrincipal& principal, const CORE::CString& jobId, CMcpJobInfo& job, CORE::CString& error );

    /** The jobs the principal may see (all with the operator scope), newest first; filters are ignored when empty */
    void ListJobs( const CMcpPrincipal& principal, TMcpJobInfoVector& jobs,
                   const CORE::CString& kindFilter = CORE::CString::Empty, const CORE::CString& stateFilter = CORE::CString::Empty );

    /**
     *  For Execute() of a job tool, see the class description. waitMs above maxWaitMs is cut to it.
     *  @param builder  makes the result (empty: the job object as structured content and as text)
     *  @return MCP_EXECUTE_COMPLETE with 'result' filled in, or MCP_EXECUTE_DEFERRED
     */
    EMcpExecuteResult Wait( CMcpRequestContext& context                                       ,
                            const CORE::CString& jobId                                        ,
                            UInt32 waitMs                                                     ,
                            CMcpToolResult& result                                            ,
                            const CIMcpJobResultBuilderPtr& builder = CIMcpJobResultBuilderPtr() );

    /** The number of calls that wait */
    UInt32 GetWaitingCount( void ) const;

    UInt32 GetJobCount( void ) const;

    /** The default result: the job object as structured content (and as its JSON text) */
    static void BuildDefaultResult( const CMcpJobInfo& job, CMcpToolResult& result );

    /**
     *  A tool execution error with structured content { "error": { "code", "message", "retryable" } }, for the error
     *  answers of job tools ("job-unknown", "job-expired", "busy", ...)
     */
    static void BuildErrorResult( const CORE::CString& code, const CORE::CString& message, bool retryable, CMcpToolResult& result );

    /** The handles as data: kind, subject, the run they stand for and its last state */
    bool SaveState( CORE::CDataNode& state ) const;

    /**
     *  Takes the handles of SaveState(), adding to those held. A handle whose run belonged to another instance of the
     *  scheduler (an earlier process) and was not final becomes interrupted
     */
    bool LoadState( const CORE::CDataNode& state );

    /** SaveState() into a file only the user of the process can read (made so, or replaced atomically); JSON */
    bool SaveStateToFile( const CORE::CString& path, CORE::CString& error ) const;

    /** LoadState() from a file of SaveStateToFile(); a file that is not there is no error (nothing to load) */
    bool LoadStateFromFile( const CORE::CString& path, CORE::CString& error );

    /** Ends expired handles and request keys; done by every call as well */
    void Expire( void );

    private:

    friend class CMcpJobWaiter;

    struct SJobEntry
    {
        CMcpJobInfo info;
        UInt64 hostInstanceId;
        UInt64 runId;
        CORE::CString schedulerJobId;       /**< the submitted job of StartJob(), removed when the run ended; empty for adopted runs */
        UInt64 sequence;                    /**< order of creation */
    };
    typedef GUCEF::map< CORE::CString, SJobEntry > TJobMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;
    typedef GUCEF::vector< CMcpJobWaiter* > TWaiterVector;

    CMcpJobBridge( const CMcpJobBridge& src );              /**< not implemented */
    CMcpJobBridge& operator=( const CMcpJobBridge& src );   /**< not implemented */

    /** A new handle and the scheduler job id made of the same bytes */
    static bool MakeHandle( CORE::CString& handle, CORE::CString& schedulerJobId );

    bool MaySee( const CMcpPrincipal& principal, const CORE::CString& subject ) const;
    void RefreshLocked( SJobEntry& entry, Int64 nowUtcMs, CORE::CString::StringVector& jobsToRemove );
    void ExpireLocked( Int64 nowUtcMs, CORE::CString::StringVector& jobsToRemove );
    void RemoveSchedulerJobs( const CORE::CString::StringVector& jobsToRemove );
    UInt32 CountActiveLocked( const CORE::CString& subject ) const;
    void PruneWaiters( void );

    /** From the waiter's timer: looks at the run, forwards progress, completes the call when it is time; true when done */
    bool OnWaiterTick( CMcpJobWaiter& waiter );

    Int64 NowUtcMs( void ) const;
    UInt64 NowMonotonicMs( void ) const;

    CORE::JobSchedulerPtr m_scheduler;
    mutable MT::CMutex m_lock;
    CMcpJobBridgeSettings m_settings;
    CORE::CSystemUtcClock m_systemUtcClock;
    const CORE::CIUtcClock* m_utcClock;
    const CORE::CIMonotonicClock* m_monotonicClock;
    TJobMap m_jobs;
    TStringMap m_expired;                   /**< handle to subject of handles that expired */
    UInt64 m_nextSequence;
    CORE::CCommandIdempotencyCache m_requestKeys;
    TWaiterVector m_waiters;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPJOBBRIDGE_H ? */
