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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#include "gucefCOMPUTE_CComputeKernelSignature.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H ? */

#include "gucefCOMPUTE_CComputeDispatchSize.h"

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

CComputeDispatchSize::CComputeDispatchSize( void )
    : m_isItems( true )
    , m_items( 0 )
{GUCEF_TRACE;

    m_groups[ 0 ] = 0;
    m_groups[ 1 ] = 0;
    m_groups[ 2 ] = 0;
}

/*-------------------------------------------------------------------------*/

CComputeDispatchSize
CComputeDispatchSize::ForItems( UInt32 itemCount )
{GUCEF_TRACE;

    CComputeDispatchSize size;
    size.m_isItems = true;
    size.m_items = itemCount;
    return size;
}

/*-------------------------------------------------------------------------*/

CComputeDispatchSize
CComputeDispatchSize::ForGroups( UInt32 x, UInt32 y, UInt32 z )
{GUCEF_TRACE;

    CComputeDispatchSize size;
    size.m_isItems = false;
    size.m_groups[ 0 ] = x;
    size.m_groups[ 1 ] = y;
    size.m_groups[ 2 ] = z;
    return size;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDispatchSize::IsItems( void ) const
{GUCEF_TRACE;

    return m_isItems;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDispatchSize::GetItemCount( const CComputeKernelSignature& signature ) const
{GUCEF_TRACE;

    if ( m_isItems )
        return m_items;
    const UInt64 items = GetTotalGroupCount( signature ) * static_cast< UInt64 >( signature.GetInvocationsPerGroup() );
    return items > 0xFFFFFFFFu ? 0xFFFFFFFFu : static_cast< UInt32 >( items );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDispatchSize::GetGroupCount( UInt32 axis, const CComputeKernelSignature& signature ) const
{GUCEF_TRACE;

    if ( axis > 2 )
        return 0;
    if ( !m_isItems )
        return m_groups[ axis ];
    if ( 0 != axis )
        return 0 == m_items ? 0 : 1;
    const UInt32 perGroup = signature.GetInvocationsPerGroup();
    if ( 0 == perGroup )
        return 0;
    return static_cast< UInt32 >( ( static_cast< UInt64 >( m_items ) + perGroup - 1 ) / perGroup );
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeDispatchSize::GetTotalGroupCount( const CComputeKernelSignature& signature ) const
{GUCEF_TRACE;

    return static_cast< UInt64 >( GetGroupCount( 0, signature ) ) *
           static_cast< UInt64 >( GetGroupCount( 1, signature ) ) *
           static_cast< UInt64 >( GetGroupCount( 2, signature ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
