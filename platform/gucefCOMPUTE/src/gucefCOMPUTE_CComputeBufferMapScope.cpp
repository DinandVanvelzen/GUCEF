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

#include "gucefCOMPUTE_CComputeBufferMapScope.h"

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

CComputeBufferMapScope::CComputeBufferMapScope( const TComputeBufferPtr& buffer, TComputeMapAccess access )
    : m_buffer( buffer )
    , m_data( GUCEF_NULL )
    , m_result( COMPUTERESULT_INVALID_ARGUMENT )
{GUCEF_TRACE;

    if ( !m_buffer.IsNULL() )
        m_result = m_buffer->Map( access, m_data );
    if ( COMPUTERESULT_SUCCESS != m_result )
        m_buffer.Unlink();
}

/*-------------------------------------------------------------------------*/

CComputeBufferMapScope::~CComputeBufferMapScope()
{GUCEF_TRACE;

    EarlyUnmap();
}

/*-------------------------------------------------------------------------*/

bool
CComputeBufferMapScope::IsMapped( void ) const
{GUCEF_TRACE;

    return !m_buffer.IsNULL();
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeBufferMapScope::GetResult( void ) const
{GUCEF_TRACE;

    return m_result;
}

/*-------------------------------------------------------------------------*/

void*
CComputeBufferMapScope::GetData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBufferMapScope::EarlyUnmap( void )
{GUCEF_TRACE;

    if ( m_buffer.IsNULL() )
        return false;
    m_buffer->Unmap();
    m_buffer.Unlink();
    m_data = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
