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

#ifndef GUCEF_COMPUTE_ALIGNEDMEMORY_H
#include "gucefCOMPUTE_AlignedMemory.h"
#define GUCEF_COMPUTE_ALIGNEDMEMORY_H
#endif /* GUCEF_COMPUTE_ALIGNEDMEMORY_H ? */

#include "gucefCOMPUTE_CCpuComputeBuffer.h"

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

CCpuComputeBuffer*
CCpuComputeBuffer::Create( CComputeDevice* device, const CComputeBufferDesc& desc )
{GUCEF_TRACE;

    const UInt64 size = desc.GetSizeInBytes();
    if ( size > static_cast< UInt64 >( static_cast< size_t >( -1 ) ) )
        return GUCEF_NULL;
    UInt8* memory = static_cast< UInt8* >( AlignedAllocate( size, GUCEF_COMPUTE_CPU_BUFFER_ALIGNMENT ) );
    if ( GUCEF_NULL == memory )
        return GUCEF_NULL;
    memset( memory, 0, static_cast< size_t >( size ) );
    return GUCEF_NEW CCpuComputeBuffer( device, desc, memory );
}

/*-------------------------------------------------------------------------*/

CCpuComputeBuffer::CCpuComputeBuffer( CComputeDevice* device, const CComputeBufferDesc& desc, UInt8* memory )
    : CComputeBuffer( device, desc )
    , m_memory( memory )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuComputeBuffer::~CCpuComputeBuffer()
{GUCEF_TRACE;

    AlignedFree( m_memory );
    m_memory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt8*
CCpuComputeBuffer::GetHostMemory( void ) const
{GUCEF_TRACE;

    return m_memory;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeBuffer::OnMap( TComputeMapAccess access, void*& data )
{GUCEF_TRACE;

    data = m_memory;
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeBuffer::OnUnmap( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
