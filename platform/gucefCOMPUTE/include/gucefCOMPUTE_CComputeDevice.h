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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#include "gucefCOMPUTE_CComputeDeviceInfo.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#include "gucefCOMPUTE_CComputeDeviceParams.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#include "gucefCOMPUTE_CComputeBufferDesc.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CComputeKernel.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

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
 *  A device of a backend: creates buffers and kernels and offers its queues (at least one). Created through
 *  CComputeBackendRegistry::CreateBestDevice() or CComputeBackend::CreateDevice(), released through TComputeDevicePtr.
 *  Buffers keep their device alive; the device keeps its queues; kernels are cached per kernel id.
 *  Thread-safe.
 *
 *  Device loss (a sticky CUDA error, VK_ERROR_DEVICE_LOST, a Metal command buffer error, the WebGPU lost callback,
 *  or SimulateDeviceLoss() in tests): the device marks itself lost once, every pending future fails with
 *  COMPUTERESULT_DEVICE_LOST, new work is refused with the same result, and DeviceLostEvent (event data: a
 *  CORE::TCloneableString with the reason) is posted once. It reaches observers on the thread of the pulse generator
 *  of CComputeDeviceParams::GetEventPulseGenerator() (the global one by default). Buffers of a lost device keep their
 *  handles but their contents are gone; their owners recreate them on a new device. The module never recreates
 *  devices itself.
 *
 *  Every device is known to CComputeGlobal while it exists: CComputeGlobal::Update() processes the pending work of its
 *  cooperative queues, unregistering its backend marks it lost ("the backend was unregistered") and unloading the
 *  module marks it lost too, so work of a device the application still holds ends before the module is gone.
 *  Kernels keep their device alive; the device caches its kernels weakly.
 */
class CComputeGlobal;

/*-------------------------------------------------------------------------*/

class GUCEF_COMPUTE_PUBLIC_CPP CComputeDevice : public CORE::CTSGNotifier ,
                                                public CORE::CTSharedObjCreator< CComputeDevice, MT::CMutex >
{
    public:

    static const CORE::CEvent DeviceLostEvent;

    static void RegisterEvents( void );

    virtual ~CComputeDevice() GUCEF_VIRTUAL_OVERRIDE;

    const CComputeDeviceInfo& GetInfo( void ) const;

    const CComputeDeviceCapabilities& GetCapabilities( void ) const;

    const CString& GetBackendName( void ) const;

    const CComputeDeviceParams& GetParams( void ) const;

    /**
     *  Creates a buffer. False with COMPUTERESULT_INVALID_ARGUMENT for a size of 0, a size above the device limit, no
     *  usage, or unified memory on a device without it; COMPUTERESULT_DEVICE_LOST on a lost device; out-of-memory
     *  results from the backend.
     */
    bool CreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer, TComputeResult* result = GUCEF_NULL );

    /**
     *  Gives the kernel with the id, compiled or loaded for this device from the kernel registry, cached after the first
     *  call. False with COMPUTERESULT_KERNEL_NOT_FOUND when the registry has no declaration or no artefact for the
     *  backend, COMPUTERESULT_UNSUPPORTED when the device lacks a feature the kernel needs, COMPUTERESULT_COMPILE_FAILED
     *  when the backend cannot build it.
     */
    bool CreateKernel( const CString& kernelId, TComputeKernelPtr& kernel, TComputeResult* result = GUCEF_NULL );

    /** Whether CreateKernel() can succeed, without creating anything */
    bool IsKernelAvailable( const CString& kernelId ) const;

    /** Forgets the cached kernels; kernels still referenced elsewhere live on */
    void ClearKernelCache( void );

    /** The cached kernels that still exist (the cache does not keep kernels alive) */
    UInt32 GetKernelCacheSize( void ) const;

    UInt32 GetQueueCount( void ) const;

    /** The queue with the index; null for an index past the end */
    TComputeQueuePtr GetQueue( UInt32 index ) const;

    /** Queue 0 */
    TComputeQueuePtr GetDefaultQueue( void ) const;

    bool IsLost( void ) const;

    /** Why the device was lost; empty while it is not */
    CString GetLostReason( void ) const;

    /** Test hook: marks the device lost as a backend does when the device fails. False when it was lost already */
    bool SimulateDeviceLoss( const CString& reason );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    CComputeDevice( const CComputeDeviceInfo& info, const CComputeDeviceParams& params );

    /** The backend creates the buffer; the description is valid for the device */
    virtual TComputeResult OnCreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer ) = 0;

    /** The backend builds the kernel from the declaration and its artefact; the device binds it afterwards */
    virtual TComputeResult OnCreateKernel( const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact, TComputeKernelPtr& kernel ) = 0;

    /** The backend name under which the artefacts of the device are registered; by default that of the device info */
    virtual CString GetArtefactBackendName( void ) const;

    /** Backends add their queues while creating the device */
    void AddQueue( const TComputeQueuePtr& queue );

    /** Marks the device lost (once), fails pending work, posts DeviceLostEvent. True the first time */
    bool MarkLost( const CString& reason );

    /**
     *  Derived destructors call it first: unregisters the device from CComputeGlobal, stops event delivery, waits for
     *  the work of the queues, stops their completion tasks, detaches them, and empties the kernel cache.
     */
    void ShutdownDevice( void );

    private:

    friend class CComputeGlobal;

    /** Cooperative queues: processes their pending work without waiting; finished work goes to 'finished' for the caller to delete */
    void ProcessCooperativeWork( TComputePendingWorkVector& finished );

    CComputeDevice( const CComputeDevice& src );            /**< not implemented */
    CComputeDevice& operator=( const CComputeDevice& src ); /**< not implemented */

    bool ResolveKernel( const CString& kernelId, TComputeKernelDeclPtr& decl, TComputeKernelArtefactPtr& artefact, TComputeResult& result ) const;

    private:

    typedef CORE::CTWeakPtr< CComputeKernel, MT::CMutex > TComputeKernelWeakPtr;
    typedef GUCEF::map< CString, TComputeKernelWeakPtr > TKernelCache;

    CComputeDeviceInfo m_info;
    CComputeDeviceParams m_params;
    GUCEF::vector< TComputeQueuePtr > m_queues;     /**< changed only while the device is created and shut down, under m_queueLock */
    MT::CMutex m_queueLock;
    TKernelCache m_kernels;
    MT::CMutex m_kernelLock;
    MT::CTAtomic< UInt32 > m_lost;
    CString m_lostReason;
    MT::CMutex m_lostLock;
    bool m_isShutdown;
    MT::CTAtomic< CComputeGlobal* > m_global;      /**< the global that knows the device, null once either let go */
    MT::CTAtomic< UInt32 > m_globalBusy;            /**< set while the global works with the device */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */
