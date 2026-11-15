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

#include "gucefCOMPUTE_CComputeBufferDesc.h"

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

CComputeBufferDesc::CComputeBufferDesc( void )
    : m_sizeInBytes( 0 )
    , m_usage( BUFFERUSAGE_STORAGE )
    , m_location( MEMORYLOCATION_DEVICE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeBufferDesc::CComputeBufferDesc( UInt64 sizeInBytes, TComputeBufferUsage usage, TComputeMemoryLocation location )
    : m_sizeInBytes( sizeInBytes )
    , m_usage( usage )
    , m_location( location )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CComputeBufferDesc::GetSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_sizeInBytes;
}

/*-------------------------------------------------------------------------*/

void
CComputeBufferDesc::SetSizeInBytes( UInt64 sizeInBytes )
{GUCEF_TRACE;

    m_sizeInBytes = sizeInBytes;
}

/*-------------------------------------------------------------------------*/

TComputeBufferUsage
CComputeBufferDesc::GetUsage( void ) const
{GUCEF_TRACE;

    return m_usage;
}

/*-------------------------------------------------------------------------*/

void
CComputeBufferDesc::SetUsage( TComputeBufferUsage usage )
{GUCEF_TRACE;

    m_usage = usage;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBufferDesc::HasUsage( TComputeBufferUsage usage ) const
{GUCEF_TRACE;

    return usage == ( m_usage & usage );
}

/*-------------------------------------------------------------------------*/

TComputeMemoryLocation
CComputeBufferDesc::GetMemoryLocation( void ) const
{GUCEF_TRACE;

    return m_location;
}

/*-------------------------------------------------------------------------*/

void
CComputeBufferDesc::SetMemoryLocation( TComputeMemoryLocation location )
{GUCEF_TRACE;

    m_location = location;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBufferDesc::IsHostVisible( void ) const
{GUCEF_TRACE;

    return MEMORYLOCATION_DEVICE != m_location;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
