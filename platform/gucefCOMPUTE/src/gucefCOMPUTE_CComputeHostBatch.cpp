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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H
#include "gucefCOMPUTE_CComputeBufferMapScope.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeHostBatch.h"

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

/** Buffers grow in powers of two from 256 bytes, so sizes that vary a little do not reallocate */
UInt64
GrownSize( UInt64 needed )
{
    UInt64 size = 256;
    while ( size < needed && size < ( static_cast< UInt64 >( 1 ) << 62 ) )
        size <<= 1;
    return size < needed ? needed : size;
}

CORE::CFutureResult
FailedFuture( TComputeResult result )
{
    CORE::CFutureResultSource source;
    source.Fail( CORE::TASKSTATUS_STARTUP_FAILED, ComputeResultToFutureInfo( result, CString::Empty ) );
    return source.GetFuture();
}

}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SHostBinding::SHostBinding( void )
    : slot( 0 )
    , kind( BINDINGKIND_INPUT )
    , input( GUCEF_NULL )
    , output( GUCEF_NULL )
    , sizeInBytes( 0 )
    , resident()
    , residentOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SHostBinding
CComputeHostBatch::SHostBinding::Input( UInt32 slot, const void* data, UInt64 sizeInBytes )
{GUCEF_TRACE;

    SHostBinding binding;
    binding.slot = slot;
    binding.kind = BINDINGKIND_INPUT;
    binding.input = data;
    binding.sizeInBytes = sizeInBytes;
    return binding;
}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SHostBinding
CComputeHostBatch::SHostBinding::Output( UInt32 slot, void* data, UInt64 sizeInBytes )
{GUCEF_TRACE;

    SHostBinding binding;
    binding.slot = slot;
    binding.kind = BINDINGKIND_OUTPUT;
    binding.output = data;
    binding.sizeInBytes = sizeInBytes;
    return binding;
}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SHostBinding
CComputeHostBatch::SHostBinding::InOut( UInt32 slot, void* data, UInt64 sizeInBytes )
{GUCEF_TRACE;

    SHostBinding binding;
    binding.slot = slot;
    binding.kind = BINDINGKIND_IN_OUT;
    binding.input = data;
    binding.output = data;
    binding.sizeInBytes = sizeInBytes;
    return binding;
}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SHostBinding
CComputeHostBatch::SHostBinding::Resident( UInt32 slot, const TComputeBufferPtr& buffer, UInt64 offset, UInt64 sizeInBytes )
{GUCEF_TRACE;

    SHostBinding binding;
    binding.slot = slot;
    binding.kind = BINDINGKIND_RESIDENT;
    binding.resident = buffer;
    binding.residentOffset = offset;
    binding.sizeInBytes = sizeInBytes;
    return binding;
}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::SSlot::SSlot( void )
    : commands()
    , arguments()
    , fence()
    , reserved( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::CComputeHostBatch( const TComputeDevicePtr& device, UInt32 ringSize )
    : m_device( device )
    , m_queue()
    , m_slots()
    , m_lock()
    , m_unified( false )
{GUCEF_TRACE;

    if ( !m_device.IsNULL() )
    {
        m_queue = m_device->GetDefaultQueue();
        const CComputeDeviceCapabilities& caps = m_device->GetCapabilities();
        m_unified = caps.HasFeatures( COMPUTEFEATURE_UNIFIED_MEMORY ) || caps.HasFeatures( COMPUTEFEATURE_MAPPABLE_DEVICE_LOCAL_MEMORY );
    }
    const UInt32 count = ringSize > 0 ? ringSize : 1;
    for ( UInt32 i=0; i<count; ++i )
        m_slots.push_back( GUCEF_NEW SSlot() );
}

/*-------------------------------------------------------------------------*/

CComputeHostBatch::~CComputeHostBatch()
{GUCEF_TRACE;

    WaitIdle( GUCEF_COMPUTE_INFINITE_TIMEOUT );
    for ( size_t i=0; i<m_slots.size(); ++i )
        GUCEF_DELETE m_slots[ i ];
    m_slots.clear();
}

/*-------------------------------------------------------------------------*/

const TComputeDevicePtr&
CComputeHostBatch::GetDevice( void ) const
{GUCEF_TRACE;

    return m_device;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeHostBatch::GetRingSize( void ) const
{GUCEF_TRACE;

    return static_cast< UInt32 >( m_slots.size() );
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeHostBatch::GetStagingSizeInBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    UInt64 total = 0;
    for ( size_t i=0; i<m_slots.size(); ++i )
        for ( UInt32 b=0; b<GUCEF_COMPUTE_MAX_BINDINGS; ++b )
            if ( !m_slots[ i ]->buffers[ b ].IsNULL() )
                total += m_slots[ i ]->buffers[ b ]->GetSizeInBytes();
    return total;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeHostBatch::AcquireSlot( SSlot*& slot )
{GUCEF_TRACE;

    slot = GUCEF_NULL;
    for ( ;; )
    {
        CComputeFence oldest;
        {
            MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );

            // the lowest free slot: calls that do not overlap keep using the same staging buffers
            SSlot* oldestSlot = GUCEF_NULL;
            for ( size_t i=0; i<m_slots.size(); ++i )
            {
                SSlot* candidate = m_slots[ i ];
                if ( candidate->reserved )
                    continue;
                if ( candidate->fence.IsSignalled() )
                {
                    candidate->reserved = true;
                    slot = candidate;
                    return COMPUTERESULT_SUCCESS;
                }
                if ( GUCEF_NULL == oldestSlot || candidate->fence.GetValue() < oldestSlot->fence.GetValue() )
                    oldestSlot = candidate;
            }
            if ( GUCEF_NULL != oldestSlot )
                oldest = oldestSlot->fence;
        }

        if ( oldest.HasQueue() )
        {
            // every slot is busy: wait for the oldest submission
            TComputeResult waited = oldest.Wait( GUCEF_COMPUTE_INFINITE_TIMEOUT );
            if ( COMPUTERESULT_SUCCESS != waited )
                return waited;
        }
        else
        {
            // every slot is reserved: other threads are recording into them, try again shortly
            MT::PrecisionDelay( 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CComputeHostBatch::ReleaseSlot( SSlot& slot, const CComputeFence& fence )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    slot.fence = fence;
    slot.reserved = false;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeHostBatch::EnsureBuffer( SSlot& slot, UInt32 bindingSlot, UInt64 sizeInBytes )
{GUCEF_TRACE;

    TComputeBufferPtr& buffer = slot.buffers[ bindingSlot ];
    if ( !buffer.IsNULL() && buffer->GetSizeInBytes() >= sizeInBytes )
        return COMPUTERESULT_SUCCESS;

    CComputeBufferDesc desc( GrownSize( sizeInBytes ), BUFFERUSAGE_STORAGE | BUFFERUSAGE_COPY_SRC | BUFFERUSAGE_COPY_DST,
                             m_unified ? MEMORYLOCATION_UNIFIED : MEMORYLOCATION_DEVICE );
    TComputeResult result = COMPUTERESULT_SUCCESS;
    TComputeBufferPtr created;
    if ( !m_device->CreateBuffer( desc, created, &result ) )
        return result;
    buffer = created;
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeHostBatch::Record( SSlot& slot, const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                           const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size )
{GUCEF_TRACE;

    slot.commands.Clear();
    slot.arguments.Clear();
    if ( kernel.IsNULL() || ( bindingCount > 0 && GUCEF_NULL == bindings ) || bindingCount > GUCEF_COMPUTE_MAX_BINDINGS )
        return COMPUTERESULT_INVALID_ARGUMENT;

    // inputs first
    for ( UInt32 i=0; i<bindingCount; ++i )
    {
        const SHostBinding& binding = bindings[ i ];
        if ( binding.slot >= GUCEF_COMPUTE_MAX_BINDINGS )
            return COMPUTERESULT_INVALID_ARGUMENT;

        if ( BINDINGKIND_RESIDENT == binding.kind )
        {
            if ( !slot.arguments.SetBuffer( binding.slot, binding.resident, binding.residentOffset, binding.sizeInBytes ) )
                return COMPUTERESULT_INVALID_ARGUMENT;
            continue;
        }
        if ( 0 == binding.sizeInBytes )
            return COMPUTERESULT_INVALID_ARGUMENT;

        TComputeResult result = EnsureBuffer( slot, binding.slot, binding.sizeInBytes );
        if ( COMPUTERESULT_SUCCESS != result )
            return result;
        const TComputeBufferPtr& buffer = slot.buffers[ binding.slot ];
        slot.arguments.SetBuffer( binding.slot, buffer, 0, binding.sizeInBytes );

        if ( BINDINGKIND_INPUT == binding.kind || BINDINGKIND_IN_OUT == binding.kind )
        {
            if ( GUCEF_NULL == binding.input )
                return COMPUTERESULT_INVALID_ARGUMENT;
            if ( m_unified )
            {
                CComputeBufferMapScope map( buffer, MAPACCESS_WRITE );
                if ( !map.IsMapped() )
                    return map.GetResult();
                memcpy( map.GetData(), binding.input, static_cast< size_t >( binding.sizeInBytes ) );
            }
            else
            {
                result = slot.commands.WriteFromHost( buffer, 0, binding.input, binding.sizeInBytes );
                if ( COMPUTERESULT_SUCCESS != result )
                    return result;
            }
        }
    }

    if ( !slot.arguments.SetConstants( constants, GUCEF_NULL == constants ? 0 : constantsSizeInBytes ) )
        return COMPUTERESULT_INVALID_ARGUMENT;
    TComputeResult result = slot.commands.Dispatch( kernel, slot.arguments, size );
    if ( COMPUTERESULT_SUCCESS != result )
        return result;

    // then the outputs
    for ( UInt32 i=0; i<bindingCount; ++i )
    {
        const SHostBinding& binding = bindings[ i ];
        if ( BINDINGKIND_OUTPUT != binding.kind && BINDINGKIND_IN_OUT != binding.kind )
            continue;
        if ( GUCEF_NULL == binding.output )
            return COMPUTERESULT_INVALID_ARGUMENT;
        result = slot.commands.ReadToHost( slot.buffers[ binding.slot ], 0, binding.output, binding.sizeInBytes );
        if ( COMPUTERESULT_SUCCESS != result )
            return result;
    }
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeHostBatch::Run( const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                        const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size )
{GUCEF_TRACE;

    if ( m_queue.IsNULL() )
        return COMPUTERESULT_INVALID_ARGUMENT;

    SSlot* slot = GUCEF_NULL;
    TComputeResult result = AcquireSlot( slot );
    if ( COMPUTERESULT_SUCCESS != result )
        return result;

    result = Record( *slot, kernel, bindings, bindingCount, constants, constantsSizeInBytes, size );
    if ( COMPUTERESULT_SUCCESS == result )
        result = m_queue->Execute( slot->commands );

    // a synchronous run leaves nothing in flight
    ReleaseSlot( *slot, CComputeFence() );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CComputeHostBatch::RunAsync( const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                             const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size )
{GUCEF_TRACE;

    if ( m_queue.IsNULL() )
        return FailedFuture( COMPUTERESULT_INVALID_ARGUMENT );

    SSlot* slot = GUCEF_NULL;
    TComputeResult result = AcquireSlot( slot );
    if ( COMPUTERESULT_SUCCESS != result )
        return FailedFuture( result );

    result = Record( *slot, kernel, bindings, bindingCount, constants, constantsSizeInBytes, size );
    if ( COMPUTERESULT_SUCCESS != result )
    {
        ReleaseSlot( *slot, CComputeFence() );
        return FailedFuture( result );
    }

    CORE::CFutureResult future = m_queue->ExecuteAsync( slot->commands );
    ReleaseSlot( *slot, m_queue->GetLastSubmittedFence() );
    return future;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeHostBatch::WaitIdle( UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( m_queue.IsNULL() )
        return COMPUTERESULT_SUCCESS;
    return m_queue->WaitIdle( timeoutInMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
