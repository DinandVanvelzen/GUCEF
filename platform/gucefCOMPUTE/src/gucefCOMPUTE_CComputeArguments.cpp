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

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

#include "gucefCOMPUTE_CComputeArguments.h"

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

CComputeArguments::CComputeArguments( void )
    : m_boundMask( 0 )
    , m_constantsSize( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<GUCEF_COMPUTE_MAX_BINDINGS; ++i )
    {
        m_bindings[ i ].offset = 0;
        m_bindings[ i ].size = 0;
    }
    memset( m_constants, 0, sizeof( m_constants ) );
}

/*-------------------------------------------------------------------------*/

CComputeArguments::CComputeArguments( const CComputeArguments& src )
    : m_boundMask( src.m_boundMask )
    , m_constantsSize( src.m_constantsSize )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<GUCEF_COMPUTE_MAX_BINDINGS; ++i )
        m_bindings[ i ] = src.m_bindings[ i ];
    memcpy( m_constants, src.m_constants, sizeof( m_constants ) );
}

/*-------------------------------------------------------------------------*/

CComputeArguments::~CComputeArguments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeArguments&
CComputeArguments::operator=( const CComputeArguments& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        for ( UInt32 i=0; i<GUCEF_COMPUTE_MAX_BINDINGS; ++i )
            m_bindings[ i ] = src.m_bindings[ i ];
        m_boundMask = src.m_boundMask;
        memcpy( m_constants, src.m_constants, sizeof( m_constants ) );
        m_constantsSize = src.m_constantsSize;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CComputeArguments::SetBuffer( UInt32 slot, const TComputeBufferPtr& buffer, UInt64 offset, UInt64 size )
{GUCEF_TRACE;

    if ( slot >= GUCEF_COMPUTE_MAX_BINDINGS || buffer.IsNULL() )
        return false;
    m_bindings[ slot ].buffer = buffer;
    m_bindings[ slot ].offset = offset;
    m_bindings[ slot ].size = size;
    m_boundMask |= ( 1u << slot );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeArguments::ClearBuffer( UInt32 slot )
{GUCEF_TRACE;

    if ( slot >= GUCEF_COMPUTE_MAX_BINDINGS )
        return;
    m_bindings[ slot ].buffer.Unlink();
    m_bindings[ slot ].offset = 0;
    m_bindings[ slot ].size = 0;
    m_boundMask &= ~( 1u << slot );
}

/*-------------------------------------------------------------------------*/

const CComputeArguments::SBufferBinding&
CComputeArguments::GetBinding( UInt32 slot ) const
{GUCEF_TRACE;

    return m_bindings[ slot < GUCEF_COMPUTE_MAX_BINDINGS ? slot : 0 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeArguments::GetBoundSlotMask( void ) const
{GUCEF_TRACE;

    return m_boundMask;
}

/*-------------------------------------------------------------------------*/

bool
CComputeArguments::SetConstants( const void* data, UInt32 sizeInBytes )
{GUCEF_TRACE;

    if ( sizeInBytes > GUCEF_COMPUTE_MAX_CONSTANTS_SIZE )
        return false;
    if ( sizeInBytes > 0 && GUCEF_NULL == data )
        return false;
    if ( sizeInBytes > 0 )
        memcpy( m_constants, data, sizeInBytes );
    m_constantsSize = sizeInBytes;
    return true;
}

/*-------------------------------------------------------------------------*/

const void*
CComputeArguments::GetConstants( void ) const
{GUCEF_TRACE;

    return m_constants;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeArguments::GetConstantsSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_constantsSize;
}

/*-------------------------------------------------------------------------*/

void
CComputeArguments::Clear( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<GUCEF_COMPUTE_MAX_BINDINGS; ++i )
        ClearBuffer( i );
    m_constantsSize = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
