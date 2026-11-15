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

#ifndef GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#define GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#include "gucefCOMPUTE_CComputeCommandList.h"
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */

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

/**
 *  Runs one kernel over host memory: copies the inputs to the device, dispatches, and copies the outputs back.
 *  This is what an ACCELERATOR provider of a batch dispatch chain (computebridgeMATH) runs for each call.
 *
 *  It keeps a ring of slots per device; a slot holds a buffer per binding slot and a command list that are reused,
 *  so repeated calls do not allocate once the buffers have grown to the sizes in use. On devices with unified memory
 *  (the CPU device, Apple silicon, integrated GPUs) the inputs are written into mapped buffers directly; elsewhere they
 *  travel in the command list (the upload staging of the backend). Outputs are read back with ReadToHost commands.
 *
 *  Bindings: an input (read by the kernel), an output (written), an in-out (read and written; the same host memory
 *  for input and output, which makes in-place calls give the same result as separate buffers), or a resident device
 *  buffer of the caller. Every binding slot of the kernel signature must be given.
 *
 *  Run() is synchronous: it records, submits and waits on the calling thread. RunAsync() returns when the work is
 *  submitted; the inputs are copied by then, the outputs must stay valid until the future completes. When every slot
 *  is in use the call waits for the oldest one; the lowest free slot is taken, so calls that do not overlap keep using
 *  the buffers of the first slot. Thread-safe.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeHostBatch
{
    public:

    enum EBindingKind
    {
        BINDINGKIND_INPUT = 0  ,
        BINDINGKIND_OUTPUT     ,
        BINDINGKIND_IN_OUT     ,
        BINDINGKIND_RESIDENT
    };
    typedef enum EBindingKind TBindingKind;

    /** One binding of a host batch call; build it with Input(), Output(), InOut() or Resident() */
    struct GUCEF_COMPUTE_PUBLIC_CPP SHostBinding
    {
        UInt32 slot;
        TBindingKind kind;
        const void* input;
        void* output;
        UInt64 sizeInBytes;
        TComputeBufferPtr resident;
        UInt64 residentOffset;

        SHostBinding( void );
        static SHostBinding Input( UInt32 slot, const void* data, UInt64 sizeInBytes );
        static SHostBinding Output( UInt32 slot, void* data, UInt64 sizeInBytes );
        static SHostBinding InOut( UInt32 slot, void* data, UInt64 sizeInBytes );
        static SHostBinding Resident( UInt32 slot, const TComputeBufferPtr& buffer, UInt64 offset = 0, UInt64 sizeInBytes = 0 );
    };

    /** A ring of 'ringSize' slots (at least 1) on the device */
    CComputeHostBatch( const TComputeDevicePtr& device, UInt32 ringSize = 4 );

    /** Waits for the work in flight */
    ~CComputeHostBatch();

    const TComputeDevicePtr& GetDevice( void ) const;

    UInt32 GetRingSize( void ) const;

    /** The bytes of the buffers of all slots together */
    UInt64 GetStagingSizeInBytes( void ) const;

    /**
     *  Runs the kernel synchronously. 'constants' is the constants block of the kernel including its 8 byte header
     *  (the backend fills the header), or null for a kernel with only the header.
     */
    TComputeResult Run( const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                        const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size );

    /** As Run(), returning when the work is submitted. The future fails with the result of a call that could not be submitted */
    CORE::CFutureResult RunAsync( const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                                  const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size );

    /** Waits until no work of the batch is in flight */
    TComputeResult WaitIdle( UInt32 timeoutInMs = GUCEF_COMPUTE_INFINITE_TIMEOUT );

    private:

    struct SSlot
    {
        TComputeBufferPtr buffers[ GUCEF_COMPUTE_MAX_BINDINGS ];
        CComputeCommandList commands;
        CComputeArguments arguments;
        CComputeFence fence;
        bool reserved;

        SSlot( void );
    };

    TComputeResult AcquireSlot( SSlot*& slot );
    void ReleaseSlot( SSlot& slot, const CComputeFence& fence );
    TComputeResult Record( SSlot& slot, const TComputeKernelPtr& kernel, const SHostBinding* bindings, UInt32 bindingCount,
                           const void* constants, UInt32 constantsSizeInBytes, const CComputeDispatchSize& size );
    TComputeResult EnsureBuffer( SSlot& slot, UInt32 bindingSlot, UInt64 sizeInBytes );

    CComputeHostBatch( const CComputeHostBatch& src );            /**< not implemented */
    CComputeHostBatch& operator=( const CComputeHostBatch& src ); /**< not implemented */

    private:

    TComputeDevicePtr m_device;
    TComputeQueuePtr m_queue;
    GUCEF::vector< SSlot* > m_slots;
    MT::CMutex m_lock;
    bool m_unified;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H ? */
