/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CCpuWorkerTaskConsumer;
class CComputeGlobal;

/**
 *  The job system of the CPU backend: fork-join over a fixed number of chunks. There is one worker set per process,
 *  owned by CComputeGlobal (GetCpuParallelFor()); its workers are dedicated tasks (CCpuWorkerTaskConsumer) on the
 *  thread pool "ComputeCpu", started on first use. Bridge plugins use it directly for multi-core providers over host
 *  memory (slicing a batch into chunks), and the CPU backend runs dispatches through it.
 *
 *  Run() is the synchronous form of data-parallel work: the calling thread executes chunks itself and the workers
 *  execute the others; it returns when every chunk is done. With EXECUTIONPOLICY_CALLING_THREAD_ONLY, on a single
 *  chunk, and on targets without threads every chunk runs on the calling thread.
 *   - Chunks are claimed with an atomic counter: per chunk there is no allocation, no task manager call and no lock.
 *     The bookkeeping of a run (scratch and the wake-up of the caller) comes from a pool that grows to the number of
 *     concurrent callers and is then reused, so steady use does not allocate at all.
 *   - No deadlock under nesting: a thread that runs a dispatch takes part in it, so progress never depends on a free
 *     worker; a worker that starts a nested run executes its chunks and idle workers help.
 *   - Determinism is up to the body: chunk boundaries depend only on the chunk count, never on the number of workers.
 *   - Every participant gets a scratch area of the requested size (64 byte aligned) for the duration of a chunk:
 *     the workgroup memory of CPU kernels.
 *  Thread-safe; several threads may run at once. Shared (TCpuParallelForPtr): CPU queues hold it, so dispatches in
 *  flight outlive the module global.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuParallelFor : public CORE::CTSharedObjCreator< CCpuParallelFor, MT::CMutex >
{
    public:

    static const CString ThreadPoolName;    /**< "ComputeCpu" */

    /** The work of one run, called concurrently from several threads with distinct chunk indices */
    class GUCEF_COMPUTE_PUBLIC_CPP CBody
    {
        public:

        CBody( void );
        virtual ~CBody();

        /** Executes chunk 'chunkIndex'. 'scratch' belongs to the calling thread until the call returns. Must not throw */
        virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) = 0;

        private:

        CBody( const CBody& src );              /**< not implemented */
        CBody& operator=( const CBody& src );   /**< not implemented */
    };

    /**
     *  Executes chunks 0 to chunkCount - 1 of the body and returns when all are done. COMPUTERESULT_SUCCESS, or
     *  COMPUTERESULT_BACKEND_ERROR when a chunk threw (the other chunks still ran), COMPUTERESULT_OUT_OF_HOST_MEMORY
     *  when the scratch could not be had.
     */
    TComputeResult Run( CBody& body, UInt32 chunkCount, UInt32 scratchSizeInBytes = 0, TComputeExecutionPolicy policy = EXECUTIONPOLICY_DEFAULT );

    /**
     *  The number of workers; 0 for the logical CPUs minus one. Running workers are stopped and the new number starts on
     *  the next run. Runs in progress complete.
     */
    void SetWorkerCount( UInt32 workerCount );

    /** The number of workers that runs use (resolved: never "0 for the default" unless the machine has one CPU) */
    UInt32 GetWorkerCount( void ) const;

    /** The workers whose threads run right now */
    UInt32 GetRunningWorkerCount( void ) const;

    /**
     *  Stops the workers; a later run starts them again. Waits for them without holding a lock, so chunks that start
     *  runs of their own meanwhile do not block; called on a worker it does not wait for that worker
     */
    void StopWorkers( void );

    /** Stops the workers for good: later runs execute on their calling thread only (the module unloads) */
    void Shutdown( void );

    virtual ~CCpuParallelFor() GUCEF_VIRTUAL_OVERRIDE;

    /** The logical CPUs minus one, at least 0 */
    static UInt32 GetDefaultWorkerCount( void );

    private:

    friend class CComputeGlobal;
    friend class CCpuWorkerTaskConsumer;

    /** The bookkeeping of one caller of Run(): reused between runs */
    struct SParticipant
    {
        MT::CSemaphore done;
        void* scratch;
        UInt32 scratchSize;
        SParticipant* nextFree;

        SParticipant( void );
        ~SParticipant();
        bool EnsureScratch( UInt32 size );
    };

    struct SJob
    {
        CBody* body;
        UInt32 chunkCount;
        UInt32 scratchSize;
        MT::CTAtomic< UInt32 > nextChunk;
        MT::CTAtomic< UInt32 > failed;
        UInt32 helpers;                 /**< guarded by m_jobLock */
        bool closed;                    /**< guarded by m_jobLock */
        SParticipant* owner;
        SJob* next;                     /**< guarded by m_jobLock */

        SJob( void );
    };

    typedef CORE::CTSharedPtr< CCpuWorkerTaskConsumer, MT::CMutex > TWorkerPtr;

    CCpuParallelFor( void );

    bool EnsureWorkers( void );
    void StopWorkerSet( GUCEF::vector< TWorkerPtr >& workers );
    SParticipant* AcquireParticipant( void );
    void ReleaseParticipant( SParticipant* participant );
    static void RunChunks( SJob& job, void* scratch, UInt32 scratchSize );
    void WorkerLoop( CCpuWorkerTaskConsumer& worker );

    CCpuParallelFor( const CCpuParallelFor& src );            /**< not implemented */
    CCpuParallelFor& operator=( const CCpuParallelFor& src ); /**< not implemented */

    private:

    MT::CMutex m_jobLock;
    SJob* m_jobs;                           /**< open jobs, guarded by m_jobLock */
    SParticipant* m_freeParticipants;       /**< guarded by m_jobLock */
    GUCEF::vector< SParticipant* > m_allParticipants;
    MT::CSemaphore m_wake;
    MT::CMutex m_workerLock;
    GUCEF::vector< TWorkerPtr > m_workers;  /**< guarded by m_workerLock */
    UInt32 m_configuredWorkerCount;         /**< guarded by m_workerLock */
    MT::CTAtomic< UInt32 > m_workerCount;
    MT::CTAtomic< UInt32 > m_shutDown;
    MT::CTAtomic< UInt32 > m_runningWorkers;
    bool m_workerStartFailed;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CCpuParallelFor, MT::CMutex > TCpuParallelForPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */
