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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMPUTE_ALIGNEDMEMORY_H
#include "gucefCOMPUTE_AlignedMemory.h"
#define GUCEF_COMPUTE_ALIGNEDMEMORY_H
#endif /* GUCEF_COMPUTE_ALIGNEDMEMORY_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#include "gucefCOMPUTE_CCpuWorkerTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CCpuWorkerTaskConsumer::TaskType = "GUCEF::COMPUTE::CCpuWorkerTaskConsumer";

/*-------------------------------------------------------------------------*/

CCpuWorkerTaskConsumer::CCpuWorkerTaskConsumer( CCpuParallelFor* parallelFor, UInt32 workerIndex )
    : CORE::CTaskConsumer()
    , m_parallelFor( parallelFor )
    , m_workerIndex( workerIndex )
    , m_scratch( GUCEF_NULL )
    , m_scratchSize( 0 )
    , m_loopRunning( 0 )
    , m_stopRequested( 0 )
    , m_threadId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuWorkerTaskConsumer::~CCpuWorkerTaskConsumer()
{GUCEF_TRACE;

    AlignedFree( m_scratch );
    m_scratch = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuWorkerTaskConsumer::GetWorkerIndex( void ) const
{GUCEF_TRACE;

    return m_workerIndex;
}

/*-------------------------------------------------------------------------*/

bool
CCpuWorkerTaskConsumer::IsLoopRunning( void ) const
{GUCEF_TRACE;

    return 0 != m_loopRunning.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CCpuWorkerTaskConsumer::RequestStop( void )
{GUCEF_TRACE;

    m_stopRequested.Store( 1, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

bool
CCpuWorkerTaskConsumer::IsStopRequested( void ) const
{GUCEF_TRACE;

    return 0 != m_stopRequested.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CCpuWorkerTaskConsumer::IsRunningOn( TThreadId threadId ) const
{GUCEF_TRACE;

    return IsLoopRunning() && threadId == m_threadId.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CCpuWorkerTaskConsumer::SetLoopRunning( bool running )
{GUCEF_TRACE;

    m_loopRunning.Store( running ? 1 : 0, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

CString
CCpuWorkerTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CCpuWorkerTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

void*
CCpuWorkerTaskConsumer::GetScratch( UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return GUCEF_NULL;
    if ( size <= m_scratchSize )
        return m_scratch;
    void* grown = AlignedAllocate( size, GUCEF_COMPUTE_CPU_BUFFER_ALIGNMENT );
    if ( GUCEF_NULL == grown )
        return GUCEF_NULL;
    AlignedFree( m_scratch );
    m_scratch = grown;
    m_scratchSize = size;
    return m_scratch;
}

/*-------------------------------------------------------------------------*/

bool
CCpuWorkerTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // the parallel-for is not touched after the loop returns, which is what its StopWorkers() waits for
    m_threadId.Store( MT::GetCurrentThreadID(), MT::MEMORDER_RELEASE );
    m_parallelFor->WorkerLoop( *this );
    SetLoopRunning( false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCpuWorkerTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // also when the cycle never ran
    SetLoopRunning( false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
