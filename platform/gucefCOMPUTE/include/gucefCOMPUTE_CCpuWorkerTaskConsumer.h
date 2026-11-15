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

#ifndef GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H
#define GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

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

class CCpuParallelFor;

/**
 *  A worker of CCpuParallelFor: a dedicated task on the thread pool "ComputeCpu" whose one cycle runs the worker loop
 *  until the parallel-for stops it or its thread pool asks the task to end (application shutdown). The loop waits for runs, helps with their chunks and keeps its own scratch area,
 *  which only grows. Created and owned by CCpuParallelFor.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuWorkerTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CString TaskType;      /**< "GUCEF::COMPUTE::CCpuWorkerTaskConsumer" */

    CCpuWorkerTaskConsumer( CCpuParallelFor* parallelFor, UInt32 workerIndex );

    virtual ~CCpuWorkerTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetWorkerIndex( void ) const;

    /** True from the start of the task until its loop ended */
    bool IsLoopRunning( void ) const;

    /** Asks the loop to end after the chunks it works on */
    void RequestStop( void );

    bool IsStopRequested( void ) const;

    /** True when the loop of this worker runs on the given thread */
    bool IsRunningOn( TThreadId threadId ) const;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CCpuParallelFor;

    /** The scratch of this worker, grown to at least 'size' bytes; null when it cannot grow */
    void* GetScratch( UInt32 size );

    void SetLoopRunning( bool running );

    CCpuWorkerTaskConsumer( const CCpuWorkerTaskConsumer& src );            /**< not implemented */
    CCpuWorkerTaskConsumer& operator=( const CCpuWorkerTaskConsumer& src ); /**< not implemented */

    private:

    CCpuParallelFor* m_parallelFor;
    UInt32 m_workerIndex;
    void* m_scratch;
    UInt32 m_scratchSize;
    MT::CTAtomic< UInt32 > m_loopRunning;
    MT::CTAtomic< UInt32 > m_stopRequested;
    MT::CTAtomic< TThreadId > m_threadId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H ? */
