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

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#include "gucefCOMPUTE_CComputeBufferDesc.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H ? */

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

class CComputeDevice;
class CComputeQueue;

/**
 *  A typeless byte range on a device, created by CComputeDevice::CreateBuffer() and released through
 *  TComputeBufferPtr. Buffers stay on their device until released; nothing migrates implicitly. A buffer keeps its
 *  device alive. Work in flight keeps the buffers it uses alive, through the references of its command list.
 *
 *  Host-visible buffers (upload, readback, unified) can be mapped; use CComputeBufferMapScope. A buffer that is
 *  mapped must not be used by work in flight: Map() refuses a buffer whose last submitted use on any queue has not
 *  completed, and queues refuse command lists that use a mapped buffer, both with COMPUTERESULT_INVALID_ARGUMENT.
 *  The buffer keeps the last use per queue; the check and the change of the mapping or of a use happen under the lock
 *  of the buffer, so a map and a submission on other threads cannot both succeed. Map() and Unmap() are thread-safe;
 *  the contents are the caller's responsibility across queues.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeBuffer : public CORE::CTSharedObjCreator< CComputeBuffer, MT::CMutex >
{
    public:

    virtual ~CComputeBuffer() GUCEF_VIRTUAL_OVERRIDE;

    const CComputeBufferDesc& GetDesc( void ) const;

    UInt64 GetSizeInBytes( void ) const;

    /** The device of the buffer */
    CComputeDevice* GetDevice( void ) const;

    /** True for buffers the host can map: upload, readback and unified */
    bool IsMappable( void ) const;

    /**
     *  Gives the host access to the whole buffer. COMPUTERESULT_UNSUPPORTED for a device-local buffer,
     *  COMPUTERESULT_INVALID_ARGUMENT when it is mapped already or in use by work that has not completed on any queue,
     *  COMPUTERESULT_DEVICE_LOST on a lost device.
     */
    TComputeResult Map( TComputeMapAccess access, void*& data );

    /** Ends the access of Map(). COMPUTERESULT_INVALID_ARGUMENT when the buffer was not mapped */
    TComputeResult Unmap( void );

    bool IsMapped( void ) const;

    /** True while a submission on any queue that uses the buffer has not completed */
    bool IsInUseByWorkInFlight( void ) const;

    protected:

    /** Keeps the device alive for as long as the buffer exists */
    CComputeBuffer( CComputeDevice* device, const CComputeBufferDesc& desc );

    /** The backend maps the buffer; called only for host-visible buffers that are not mapped */
    virtual TComputeResult OnMap( TComputeMapAccess access, void*& data ) = 0;

    virtual void OnUnmap( void ) = 0;

    private:

    friend class CComputeQueue;

    /**
     *  Records that the submission with 'fenceValue' of the queue uses the buffer; 'previous' receives the value it
     *  replaces. False, and nothing recorded, while the buffer is mapped
     */
    bool TryRecordUse( CComputeQueue* queue, UInt64 fenceValue, UInt64& previous );

    /** Puts back the use of the queue that TryRecordUse() replaced, for a submission that failed */
    void RestoreUse( CComputeQueue* queue, UInt64 previous );

    bool IsInUseLocked( void ) const;

    CComputeBuffer( const CComputeBuffer& src );            /**< not implemented */
    CComputeBuffer& operator=( const CComputeBuffer& src ); /**< not implemented */

    private:

    struct SQueueUse
    {
        CComputeQueue* queue;       /**< a queue of the device, which the device keeps while the buffer keeps the device */
        UInt64 fenceValue;
    };

    TComputeDevicePtr m_device;
    CComputeBufferDesc m_desc;
    MT::CMutex m_lock;
    bool m_mapped;                              /**< guarded by m_lock */
    GUCEF::vector< SQueueUse > m_uses;          /**< guarded by m_lock, one per queue that used the buffer */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */
