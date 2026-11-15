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

#include "gucefCOMPUTE_CComputeDeviceRequirements.h"

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

CComputeDeviceRequirements::CComputeDeviceRequirements( void )
    : m_requiredFeatures( 0 )
    , m_minInvocationsPerGroup( 0 )
    , m_minWorkgroupMemory( 0 )
    , m_minStorageBuffers( 0 )
    , m_minConstantsSize( 0 )
    , m_minBufferSize( 0 )
    , m_allowedBackends()
    , m_allowedDeviceTypes( 0 )
    , m_cpuDeviceAcceptable( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TComputeFeatures
CComputeDeviceRequirements::GetRequiredFeatures( void ) const
{GUCEF_TRACE;

    return m_requiredFeatures;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetRequiredFeatures( TComputeFeatures features )
{GUCEF_TRACE;

    m_requiredFeatures = features;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceRequirements::GetMinInvocationsPerGroup( void ) const
{GUCEF_TRACE;

    return m_minInvocationsPerGroup;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetMinInvocationsPerGroup( UInt32 invocations )
{GUCEF_TRACE;

    m_minInvocationsPerGroup = invocations;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceRequirements::GetMinWorkgroupMemoryInBytes( void ) const
{GUCEF_TRACE;

    return m_minWorkgroupMemory;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetMinWorkgroupMemoryInBytes( UInt32 bytes )
{GUCEF_TRACE;

    m_minWorkgroupMemory = bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceRequirements::GetMinStorageBuffersPerKernel( void ) const
{GUCEF_TRACE;

    return m_minStorageBuffers;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetMinStorageBuffersPerKernel( UInt32 count )
{GUCEF_TRACE;

    m_minStorageBuffers = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceRequirements::GetMinConstantsSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_minConstantsSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetMinConstantsSizeInBytes( UInt32 bytes )
{GUCEF_TRACE;

    m_minConstantsSize = bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeDeviceRequirements::GetMinBufferSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_minBufferSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetMinBufferSizeInBytes( UInt64 bytes )
{GUCEF_TRACE;

    m_minBufferSize = bytes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet&
CComputeDeviceRequirements::GetAllowedBackends( void ) const
{GUCEF_TRACE;

    return m_allowedBackends;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetAllowedBackends( const CString::StringSet& backendNames )
{GUCEF_TRACE;

    m_allowedBackends.clear();
    CString::StringSet::const_iterator i = backendNames.begin();
    while ( i != backendNames.end() )
    {
        m_allowedBackends.insert( (*i).Lowercase() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::AllowBackend( const CString& backendName )
{GUCEF_TRACE;

    m_allowedBackends.insert( backendName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceRequirements::GetAllowedDeviceTypes( void ) const
{GUCEF_TRACE;

    return m_allowedDeviceTypes;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetAllowedDeviceTypes( UInt32 deviceTypeMask )
{GUCEF_TRACE;

    m_allowedDeviceTypes = deviceTypeMask;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::AllowDeviceType( TComputeDeviceType deviceType )
{GUCEF_TRACE;

    m_allowedDeviceTypes |= ( 1u << static_cast< UInt32 >( deviceType ) );
}

/*-------------------------------------------------------------------------*/

bool
CComputeDeviceRequirements::IsCpuDeviceAcceptable( void ) const
{GUCEF_TRACE;

    return m_cpuDeviceAcceptable;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceRequirements::SetCpuDeviceAcceptable( bool acceptable )
{GUCEF_TRACE;

    m_cpuDeviceAcceptable = acceptable;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDeviceRequirements::IsMetBy( const CComputeDeviceInfo& device ) const
{GUCEF_TRACE;

    const CComputeDeviceCapabilities& caps = device.GetCapabilities();

    if ( !m_cpuDeviceAcceptable && DEVICETYPE_CPU == device.GetDeviceType() )
        return false;
    if ( 0 != m_allowedDeviceTypes && 0 == ( m_allowedDeviceTypes & ( 1u << static_cast< UInt32 >( device.GetDeviceType() ) ) ) )
        return false;
    if ( !m_allowedBackends.empty() && m_allowedBackends.end() == m_allowedBackends.find( device.GetBackendName().Lowercase() ) )
        return false;
    if ( !caps.HasFeatures( m_requiredFeatures ) )
        return false;
    if ( caps.GetMaxInvocationsPerGroup() < m_minInvocationsPerGroup )
        return false;
    if ( caps.GetMaxWorkgroupMemoryInBytes() < m_minWorkgroupMemory )
        return false;
    if ( caps.GetMaxStorageBuffersPerKernel() < m_minStorageBuffers )
        return false;
    if ( caps.GetMaxConstantsSizeInBytes() < m_minConstantsSize )
        return false;
    if ( caps.GetMaxBufferSizeInBytes() < m_minBufferSize )
        return false;
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
