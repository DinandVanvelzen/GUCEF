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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#include "gucefCOMPUTE_CComputeBuffer.h"

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

CComputeBuffer::CComputeBuffer( CComputeDevice* device, const CComputeBufferDesc& desc )
    : CORE::CTSharedObjCreator< CComputeBuffer, MT::CMutex >( this )
    , m_device( GUCEF_NULL != device ? device->CreateSharedPtr() : TComputeDevicePtr() )
    , m_desc( desc )
    , m_lock()
    , m_mapped( false )
    , m_uses()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeBuffer::~CComputeBuffer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CComputeBufferDesc&
CComputeBuffer::GetDesc( void ) const
{GUCEF_TRACE;

    return m_desc;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeBuffer::GetSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_desc.GetSizeInBytes();
}

/*-------------------------------------------------------------------------*/

CComputeDevice*
CComputeBuffer::GetDevice( void ) const
{GUCEF_TRACE;

    return const_cast< CComputeDevice* >( m_device.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

bool
CComputeBuffer::IsMappable( void ) const
{GUCEF_TRACE;

    return m_desc.IsHostVisible();
}

/*-------------------------------------------------------------------------*/

bool
CComputeBuffer::IsInUseLocked( void ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_uses.size(); ++i )
        if ( m_uses[ i ].queue->GetCompletedValue() < m_uses[ i ].fenceValue )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBuffer::IsInUseByWorkInFlight( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return IsInUseLocked();
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeBuffer::Map( TComputeMapAccess access, void*& data )
{GUCEF_TRACE;

    data = GUCEF_NULL;
    if ( !IsMappable() )
        return COMPUTERESULT_UNSUPPORTED;
    if ( !m_device.IsNULL() && m_device->IsLost() )
        return COMPUTERESULT_DEVICE_LOST;

    MT::CScopeMutex lock( m_lock );
    if ( m_mapped )
        return COMPUTERESULT_INVALID_ARGUMENT;
    if ( IsInUseLocked() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeBuffer: Refusing to map a buffer of " << m_desc.GetSizeInBytes() << " bytes that work in flight still uses" );
        return COMPUTERESULT_INVALID_ARGUMENT;
    }
    const TComputeResult result = OnMap( access, data );
    if ( COMPUTERESULT_SUCCESS != result )
    {
        data = GUCEF_NULL;
        return result;
    }
    m_mapped = true;
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeBuffer::Unmap( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_mapped )
        return COMPUTERESULT_INVALID_ARGUMENT;
    OnUnmap();
    m_mapped = false;
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBuffer::IsMapped( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mapped;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBuffer::TryRecordUse( CComputeQueue* queue, UInt64 fenceValue, UInt64& previous )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_mapped )
        return false;
    for ( size_t i=0; i<m_uses.size(); ++i )
    {
        if ( m_uses[ i ].queue == queue )
        {
            previous = m_uses[ i ].fenceValue;
            m_uses[ i ].fenceValue = fenceValue;
            return true;
        }
    }
    SQueueUse use;
    use.queue = queue;
    use.fenceValue = fenceValue;
    m_uses.push_back( use );
    previous = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeBuffer::RestoreUse( CComputeQueue* queue, UInt64 previous )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_uses.size(); ++i )
    {
        if ( m_uses[ i ].queue == queue )
        {
            m_uses[ i ].fenceValue = previous;
            return;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
