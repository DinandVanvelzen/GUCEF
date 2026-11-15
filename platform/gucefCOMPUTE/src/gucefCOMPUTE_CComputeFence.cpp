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

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#include "gucefCOMPUTE_CComputeFence.h"

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

CComputeFence::CComputeFence( void )
    : m_queue()
    , m_value( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeFence::CComputeFence( const TComputeQueuePtr& queue, UInt64 value )
    : m_queue( queue )
    , m_value( value )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeFence::CComputeFence( const CComputeFence& src )
    : m_queue( src.m_queue )
    , m_value( src.m_value )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeFence::~CComputeFence()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeFence&
CComputeFence::operator=( const CComputeFence& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_queue = src.m_queue;
        m_value = src.m_value;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CComputeFence::HasQueue( void ) const
{GUCEF_TRACE;

    return !m_queue.IsNULL();
}

/*-------------------------------------------------------------------------*/

const TComputeQueuePtr&
CComputeFence::GetQueue( void ) const
{GUCEF_TRACE;

    return m_queue;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeFence::GetValue( void ) const
{GUCEF_TRACE;

    return m_value;
}

/*-------------------------------------------------------------------------*/

bool
CComputeFence::IsSignalled( void ) const
{GUCEF_TRACE;

    if ( m_queue.IsNULL() || 0 == m_value )
        return true;
    return m_queue->GetCompletedValue() >= m_value;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeFence::Wait( UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    if ( m_queue.IsNULL() || 0 == m_value )
        return COMPUTERESULT_SUCCESS;
    TComputeQueuePtr queue( m_queue );
    return queue->WaitForValue( m_value, timeoutInMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
