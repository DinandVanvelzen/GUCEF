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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#define GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

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
 *  A buffer of the CPU backend: host memory aligned to GUCEF_COMPUTE_CPU_BUFFER_ALIGNMENT (64) bytes, zeroed when
 *  created. Every memory location maps to it and it is always mappable.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeBuffer : public CComputeBuffer
{
    public:

    /** Creates the buffer; null when the memory cannot be had */
    static CCpuComputeBuffer* Create( CComputeDevice* device, const CComputeBufferDesc& desc );

    virtual ~CCpuComputeBuffer() GUCEF_VIRTUAL_OVERRIDE;

    /** The memory of the buffer, for the CPU queue and CPU kernels */
    UInt8* GetHostMemory( void ) const;

    protected:

    virtual TComputeResult OnMap( TComputeMapAccess access, void*& data ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUnmap( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCpuComputeBuffer( CComputeDevice* device, const CComputeBufferDesc& desc, UInt8* memory );

    CCpuComputeBuffer( const CCpuComputeBuffer& src );            /**< not implemented */
    CCpuComputeBuffer& operator=( const CCpuComputeBuffer& src ); /**< not implemented */

    private:

    UInt8* m_memory;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H ? */
