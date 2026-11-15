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

#include "gucefCOMPUTE_CComputeDeviceCapabilities.h"

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

CComputeDeviceCapabilities::CComputeDeviceCapabilities( void )
    : m_features( 0 )
    , m_maxInvocationsPerGroup( 256 )
    , m_maxWorkgroupMemory( 16 * 1024 )
    , m_maxStorageBuffers( GUCEF_COMPUTE_MAX_BINDINGS )
    , m_maxConstantsSize( GUCEF_COMPUTE_MAX_CONSTANTS_SIZE )
    , m_maxBufferSize( static_cast< UInt64 >( 256 ) * 1024 * 1024 )
    , m_maxBindingSize( static_cast< UInt64 >( 128 ) * 1024 * 1024 )
    , m_minSubgroupSize( 0 )
    , m_maxSubgroupSize( 0 )
    , m_queueCount( 1 )
    , m_memoryBudget( 0 )
{GUCEF_TRACE;

    m_maxGroupSize[ 0 ] = 256;
    m_maxGroupSize[ 1 ] = 256;
    m_maxGroupSize[ 2 ] = 64;
    m_maxGroupsPerDispatch[ 0 ] = 65535;
    m_maxGroupsPerDispatch[ 1 ] = 65535;
    m_maxGroupsPerDispatch[ 2 ] = 65535;
}

/*-------------------------------------------------------------------------*/

CComputeDeviceCapabilities
CComputeDeviceCapabilities::PortableLimits( void )
{GUCEF_TRACE;

    return CComputeDeviceCapabilities();
}

/*-------------------------------------------------------------------------*/

bool
CComputeDeviceCapabilities::HasFeatures( TComputeFeatures features ) const
{GUCEF_TRACE;

    return features == ( m_features & features );
}

/*-------------------------------------------------------------------------*/

TComputeFeatures
CComputeDeviceCapabilities::GetFeatures( void ) const
{GUCEF_TRACE;

    return m_features;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetFeatures( TComputeFeatures features )
{GUCEF_TRACE;

    m_features = features;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetFeature( TComputeFeatures features, bool supported )
{GUCEF_TRACE;

    if ( supported )
        m_features |= features;
    else
        m_features &= ~features;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxInvocationsPerGroup( void ) const
{GUCEF_TRACE;

    return m_maxInvocationsPerGroup;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxInvocationsPerGroup( UInt32 invocations )
{GUCEF_TRACE;

    m_maxInvocationsPerGroup = invocations;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxGroupSize( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < 3 ? m_maxGroupSize[ axis ] : 0;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxGroupSize( UInt32 x, UInt32 y, UInt32 z )
{GUCEF_TRACE;

    m_maxGroupSize[ 0 ] = x;
    m_maxGroupSize[ 1 ] = y;
    m_maxGroupSize[ 2 ] = z;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxWorkgroupMemoryInBytes( void ) const
{GUCEF_TRACE;

    return m_maxWorkgroupMemory;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxWorkgroupMemoryInBytes( UInt32 bytes )
{GUCEF_TRACE;

    m_maxWorkgroupMemory = bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxGroupsPerDispatch( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < 3 ? m_maxGroupsPerDispatch[ axis ] : 0;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxGroupsPerDispatch( UInt32 x, UInt32 y, UInt32 z )
{GUCEF_TRACE;

    m_maxGroupsPerDispatch[ 0 ] = x;
    m_maxGroupsPerDispatch[ 1 ] = y;
    m_maxGroupsPerDispatch[ 2 ] = z;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxStorageBuffersPerKernel( void ) const
{GUCEF_TRACE;

    return m_maxStorageBuffers;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxStorageBuffersPerKernel( UInt32 count )
{GUCEF_TRACE;

    m_maxStorageBuffers = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxConstantsSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_maxConstantsSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxConstantsSizeInBytes( UInt32 bytes )
{GUCEF_TRACE;

    m_maxConstantsSize = bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeDeviceCapabilities::GetMaxBufferSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_maxBufferSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxBufferSizeInBytes( UInt64 bytes )
{GUCEF_TRACE;

    m_maxBufferSize = bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeDeviceCapabilities::GetMaxBindingSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_maxBindingSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMaxBindingSizeInBytes( UInt64 bytes )
{GUCEF_TRACE;

    m_maxBindingSize = bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMinSubgroupSize( void ) const
{GUCEF_TRACE;

    return m_minSubgroupSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetMaxSubgroupSize( void ) const
{GUCEF_TRACE;

    return m_maxSubgroupSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetSubgroupSizeRange( UInt32 minSize, UInt32 maxSize )
{GUCEF_TRACE;

    m_minSubgroupSize = minSize;
    m_maxSubgroupSize = maxSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceCapabilities::GetQueueCount( void ) const
{GUCEF_TRACE;

    return m_queueCount;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetQueueCount( UInt32 count )
{GUCEF_TRACE;

    m_queueCount = count > 0 ? count : 1;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeDeviceCapabilities::GetMemoryBudgetInBytes( void ) const
{GUCEF_TRACE;

    return m_memoryBudget;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceCapabilities::SetMemoryBudgetInBytes( UInt64 bytes )
{GUCEF_TRACE;

    m_memoryBudget = bytes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
