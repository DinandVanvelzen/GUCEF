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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CCpuComputeBuffer.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CCpuComputeKernel.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CCpuComputeQueue.h"

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

namespace {

/** The chunks of a CPU dispatch: 'grain' groups each, the last one shorter */
class CDispatchBody : public CCpuParallelFor::CBody
{
    public:

    CDispatchBody( const CCpuComputeKernel& kernel, const CCpuDispatchContext::SDispatchData& data, UInt32 grain )
        : CCpuParallelFor::CBody()
        , m_kernel( kernel )
        , m_data( data )
        , m_grain( grain )
    {GUCEF_TRACE;
    }

    virtual void ExecuteChunk( UInt32 chunkIndex, void* scratch, UInt32 scratchSizeInBytes ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const UInt32 first = chunkIndex * m_grain;
        const UInt32 left = m_data.totalGroups - first;
        const UInt32 count = left < m_grain ? left : m_grain;
        CCpuDispatchContext context( m_data, scratch, scratchSizeInBytes );
        m_kernel.ExecuteGroups( context, first, count );
    }

    private:

    const CCpuComputeKernel& m_kernel;
    const CCpuDispatchContext::SDispatchData& m_data;
    UInt32 m_grain;
};

}

/*-------------------------------------------------------------------------*/

CCpuComputeQueue::CCpuComputeQueue( CComputeDevice* device, UInt32 queueIndex, TComputeExecutionPolicy policy, bool cooperative, const TCpuParallelForPtr& parallelFor )
    : CComputeQueue( device, queueIndex, policy, cooperative )
    , m_parallelFor( parallelFor )
    , m_timeline()
    , m_commandScratch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuComputeQueue::~CCpuComputeQueue()
{GUCEF_TRACE;

    ShutdownQueue();
}

/*-------------------------------------------------------------------------*/

UInt64
CCpuComputeQueue::GetCompletedValue( void ) const
{GUCEF_TRACE;

    return m_timeline.GetCompletedValue();
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::WaitForValue( UInt64 value, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( !IsCooperative() || m_timeline.GetCompletedValue() >= value )
        return m_timeline.Wait( value, timeoutInMs );

    // cooperative: the waiting thread runs the pending work up to the value itself, in order
    TComputePendingWorkVector finished;
    DrainCooperativeWork( value, timeoutInMs, finished );
    const TComputeResult result = m_timeline.Wait( value, timeoutInMs );
    for ( size_t i=0; i<finished.size(); ++i )
        GUCEF_DELETE finished[ i ];
    return result;
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeQueue::OnDeviceLost( void )
{GUCEF_TRACE;

    m_timeline.Fail( COMPUTERESULT_DEVICE_LOST );
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeQueue::OnQueueClosed( TComputeResult reason )
{GUCEF_TRACE;

    m_timeline.Fail( reason );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::OnSubmit( const CComputeCommandList& list, UInt64 fenceValue )
{GUCEF_TRACE;

    // nothing to do now: the list is executed in the order of the values, by the completion task (ProcessPendingWork()),
    // by a cooperative drain, or for a synchronous call by the calling thread (OnExecute())
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::OnExecute( const CComputeCommandList& list, UInt64 fenceValue, TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    // in order: the work submitted before runs first (on the completion task, other calling threads, or in cooperative
    // mode on this thread), then this list on the calling thread. The queue lock is not held here
    if ( fenceValue > 1 )
    {
        const TComputeResult earlier = WaitForValue( fenceValue - 1, GUCEF_COMPUTE_INFINITE_TIMEOUT );
        if ( COMPUTERESULT_SUCCESS != earlier )
            return earlier;
    }
    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;

    CString errorText;
    const TComputeResult result = ExecuteCommands( list, policy, errorText );
    m_timeline.Signal( fenceValue );
    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;
    return result;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::ProcessPendingWork( CComputePendingWork& work, UInt32 timeoutInMs, CString& errorText )
{GUCEF_TRACE;

    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;

    // a synchronous call that took the value before may still be running on its own thread
    const UInt64 value = work.GetFenceValue();
    if ( value > 1 )
    {
        const TComputeResult earlier = m_timeline.Wait( value - 1, timeoutInMs );
        if ( COMPUTERESULT_SUCCESS != earlier )
            return earlier;
    }

    const TComputeResult result = ExecuteCommands( work.GetCommands(), GetExecutionPolicy(), errorText );
    m_timeline.Signal( work.GetFenceValue() );
    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;
    return result;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::ExecuteDispatch( const CComputeCommandList::SCommand& command, TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    const CCpuComputeKernel& kernel = static_cast< const CCpuComputeKernel& >( *command.kernel );
    const CComputeKernelSignature& signature = kernel.GetSignature();

    CCpuDispatchContext::SDispatchData data;
    memcpy( data.constants, command.constants, sizeof( data.constants ) );
    data.constantsSize = command.constantsSize;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        data.groupCount[ axis ] = command.dispatchSize.GetGroupCount( axis, signature );
        data.workgroupSize[ axis ] = signature.GetWorkgroupSize( axis );
    }
    const UInt64 totalGroups = command.dispatchSize.GetTotalGroupCount( signature );
    data.totalGroups = static_cast< UInt32 >( totalGroups );

    // the header of the constants: the CPU backend never splits, so the base item is 0
    const UInt32 header[ 2 ] = { 0, command.dispatchSize.GetItemCount( signature ) };
    memcpy( data.constants, header, sizeof( header ) );

    for ( UInt32 i=0; i<command.bindingCount; ++i )
    {
        const CComputeCommandList::SDispatchBinding& binding = command.bindings[ i ];
        const CCpuComputeBuffer* buffer = static_cast< const CCpuComputeBuffer* >( binding.buffer );
        CCpuDispatchContext::SBinding& out = data.bindings[ binding.slot ];
        out.data = buffer->GetHostMemory() + binding.offset;
        out.sizeInBytes = binding.size;
        out.access = binding.access;
    }
    if ( 0 == data.totalGroups )
        return COMPUTERESULT_SUCCESS;

    // chunk boundaries depend only on the kernel and the size of the dispatch, never on the number of workers
    UInt32 grain = kernel.GetDecl().GetPreferredGrain();
    if ( 0 == grain )
    {
        const UInt64 bytesPerGroup = static_cast< UInt64 >( signature.GetInvocationsPerGroup() ) * signature.GetOutputBytesPerItem();
        grain = bytesPerGroup >= 65536 ? 1 : static_cast< UInt32 >( 65536 / bytesPerGroup );
    }
    const UInt32 chunkCount = static_cast< UInt32 >( ( totalGroups + grain - 1 ) / grain );

    if ( m_parallelFor.IsNULL() )
        return COMPUTERESULT_BACKEND_ERROR;
    CDispatchBody body( kernel, data, grain );
    return m_parallelFor->Run( body, chunkCount, signature.GetWorkgroupMemoryInBytes(), policy );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeQueue::ExecuteCommands( const CComputeCommandList& list, TComputeExecutionPolicy policy, CString& errorText )
{GUCEF_TRACE;

    // the scratch command is only used by the thread that executes, and execution of one queue is never concurrent
    CComputeCommandList::SCommand& command = m_commandScratch;
    UInt32 cursor = 0;
    while ( list.ReadCommand( cursor, command ) )
    {
        if ( IsLost() )
            return COMPUTERESULT_DEVICE_LOST;

        switch ( command.type )
        {
            case CComputeCommandList::COMMANDTYPE_WRITE_FROM_HOST:
            {
                UInt8* target = static_cast< CCpuComputeBuffer* >( command.buffer )->GetHostMemory() + command.offset;
                if ( command.size > 0 )
                    memcpy( target, command.hostSource, static_cast< size_t >( command.size ) );
                break;
            }
            case CComputeCommandList::COMMANDTYPE_READ_TO_HOST:
            {
                const UInt8* source = static_cast< CCpuComputeBuffer* >( command.buffer )->GetHostMemory() + command.offset;
                if ( command.size > 0 )
                    memcpy( command.hostDestination, source, static_cast< size_t >( command.size ) );
                break;
            }
            case CComputeCommandList::COMMANDTYPE_COPY:
            {
                const UInt8* source = static_cast< CCpuComputeBuffer* >( command.sourceBuffer )->GetHostMemory() + command.sourceOffset;
                UInt8* target = static_cast< CCpuComputeBuffer* >( command.buffer )->GetHostMemory() + command.offset;
                if ( command.size > 0 )
                    memmove( target, source, static_cast< size_t >( command.size ) );
                break;
            }
            case CComputeCommandList::COMMANDTYPE_FILL:
            {
                UInt8* target = static_cast< CCpuComputeBuffer* >( command.buffer )->GetHostMemory() + command.offset;
                const UInt64 words = command.size / 4;
                for ( UInt64 w=0; w<words; ++w )
                    memcpy( target + w * 4, &command.fillPattern, 4 );
                break;
            }
            case CComputeCommandList::COMMANDTYPE_DISPATCH:
            {
                const TComputeResult result = ExecuteDispatch( command, policy );
                if ( COMPUTERESULT_SUCCESS != result )
                {
                    errorText = "a chunk of kernel \"" + command.kernel->GetKernelId() + "\" failed";
                    return result;
                }
                break;
            }
            case CComputeCommandList::COMMANDTYPE_WAIT_FOR:
            {
                if ( command.waitQueue != this )
                {
                    const TComputeResult result = command.waitQueue->WaitForValue( command.waitValue, GUCEF_COMPUTE_INFINITE_TIMEOUT );
                    if ( COMPUTERESULT_SUCCESS != result )
                        return result;
                }
                break;
            }
            default:
            {
                errorText = "the command list holds an unknown command";
                return COMPUTERESULT_BACKEND_ERROR;
            }
        }
    }
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
