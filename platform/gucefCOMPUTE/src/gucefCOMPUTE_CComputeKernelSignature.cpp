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

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeKernelSignature.h"

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

CComputeKernelSignature::CComputeKernelSignature( void )
    : m_bindingCount( 0 )
    , m_constantsSize( GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE )
    , m_workgroupMemory( 0 )
    , m_requiredFeatures( 0 )
{GUCEF_TRACE;

    m_workgroupSize[ 0 ] = 64;
    m_workgroupSize[ 1 ] = 1;
    m_workgroupSize[ 2 ] = 1;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelSignature::AddBinding( UInt32 slot, TComputeAccess access, TComputeElementType elementType )
{GUCEF_TRACE;

    if ( slot >= GUCEF_COMPUTE_MAX_BINDINGS || m_bindingCount >= GUCEF_COMPUTE_MAX_BINDINGS )
        return false;
    if ( 0 != ( GetBoundSlotMask() & ( 1u << slot ) ) )
        return false;

    SBinding& binding = m_bindings[ m_bindingCount ];
    binding.slot = slot;
    binding.access = access;
    binding.elementType = elementType;
    ++m_bindingCount;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetBindingCount( void ) const
{GUCEF_TRACE;

    return m_bindingCount;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelSignature::GetBinding( UInt32 index, SBinding& binding ) const
{GUCEF_TRACE;

    if ( index >= m_bindingCount )
        return false;
    binding = m_bindings[ index ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelSignature::FindBinding( UInt32 slot, SBinding& binding ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_bindingCount; ++i )
    {
        if ( m_bindings[ i ].slot == slot )
        {
            binding = m_bindings[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetBoundSlotMask( void ) const
{GUCEF_TRACE;

    UInt32 mask = 0;
    for ( UInt32 i=0; i<m_bindingCount; ++i )
        mask |= ( 1u << m_bindings[ i ].slot );
    return mask;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelSignature::SetConstantsSizeInBytes( UInt32 bytes )
{GUCEF_TRACE;

    if ( bytes < GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE || bytes > GUCEF_COMPUTE_MAX_CONSTANTS_SIZE )
        return false;
    m_constantsSize = bytes;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetConstantsSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_constantsSize;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelSignature::SetWorkgroupSize( UInt32 x, UInt32 y, UInt32 z )
{GUCEF_TRACE;

    if ( 0 == x || 0 == y || 0 == z )
        return false;
    m_workgroupSize[ 0 ] = x;
    m_workgroupSize[ 1 ] = y;
    m_workgroupSize[ 2 ] = z;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetWorkgroupSize( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < 3 ? m_workgroupSize[ axis ] : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetInvocationsPerGroup( void ) const
{GUCEF_TRACE;

    return m_workgroupSize[ 0 ] * m_workgroupSize[ 1 ] * m_workgroupSize[ 2 ];
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelSignature::SetWorkgroupMemoryInBytes( UInt32 bytes )
{GUCEF_TRACE;

    m_workgroupMemory = bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetWorkgroupMemoryInBytes( void ) const
{GUCEF_TRACE;

    return m_workgroupMemory;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelSignature::SetRequiredFeatures( TComputeFeatures features )
{GUCEF_TRACE;

    m_requiredFeatures = features;
}

/*-------------------------------------------------------------------------*/

TComputeFeatures
CComputeKernelSignature::GetRequiredFeatures( void ) const
{GUCEF_TRACE;

    return m_requiredFeatures;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelSignature::GetOutputBytesPerItem( void ) const
{GUCEF_TRACE;

    UInt32 largest = 0;
    for ( UInt32 i=0; i<m_bindingCount; ++i )
    {
        if ( COMPUTEACCESS_READ != m_bindings[ i ].access )
        {
            const UInt32 size = ComputeElementTypeSize( m_bindings[ i ].elementType );
            if ( size > largest )
                largest = size;
        }
    }
    return largest > 0 ? largest : 4;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
