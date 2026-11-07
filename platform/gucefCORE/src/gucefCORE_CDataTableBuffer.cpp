/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CDataTableBufferPoolCore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableBuffer::CDataTableBuffer( CDataTableBufferPoolCore* core, UInt32 sizeClass )
    : m_buffer( false )
    , m_data( GUCEF_NULL )
    , m_dataSize( 0 )
    , m_capacity( 0 )
    , m_sizeClass( sizeClass )
    , m_refCount( 0 )
    , m_core( core )
    , m_nextFree( GUCEF_NULL )
{GUCEF_TRACE;

    const UInt32 allocationSize = CDataTableBufferPoolCore::SizeClassBytes( sizeClass );
    if ( m_buffer.SetBufferSize( allocationSize, false ) )
    {
        m_data = m_buffer.GetBufferPtr();
        m_capacity = allocationSize - DataTableBufferPadding;
    }
}

/*-------------------------------------------------------------------------*/

CDataTableBuffer::~CDataTableBuffer()
{GUCEF_TRACE;

    m_data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableBuffer::SetDataSize( UInt32 dataSize )
{GUCEF_TRACE;

    if ( dataSize > m_capacity )
        return false;
    m_dataSize = dataSize;
    memset( static_cast< char* >( m_data ) + dataSize, 0, DataTableBufferPadding );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBuffer::GetAllocationSize( void ) const
{GUCEF_TRACE;

    return CDataTableBufferPoolCore::SizeClassBytes( m_sizeClass );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableBuffer::GetReferenceCount( void ) const
{GUCEF_TRACE;

    return m_refCount.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBuffer::AddReference( void )
{

    // a new reference is always made from an existing one, so no ordering is needed
    m_refCount.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CDataTableBuffer::ReleaseReference( void )
{

    // acquire-release: every use by every holder happens before the buffer is reused
    if ( 1 == m_refCount.FetchSub( 1, MT::MEMORDER_ACQ_REL ) )
        m_core->ReturnBuffer( this );
}

/*-------------------------------------------------------------------------*/

CDataTableBufferPtr::CDataTableBufferPtr( void )
    : m_buffer( GUCEF_NULL )
{

}

/*-------------------------------------------------------------------------*/

CDataTableBufferPtr::CDataTableBufferPtr( const CDataTableBufferPtr& src )
    : m_buffer( src.m_buffer )
{

    if ( GUCEF_NULL != m_buffer )
        m_buffer->AddReference();
}

/*-------------------------------------------------------------------------*/

CDataTableBufferPtr::~CDataTableBufferPtr()
{

    Reset();
}

/*-------------------------------------------------------------------------*/

CDataTableBufferPtr&
CDataTableBufferPtr::operator=( const CDataTableBufferPtr& src )
{

    if ( src.m_buffer != m_buffer )
    {
        CDataTableBuffer* previous = m_buffer;
        m_buffer = src.m_buffer;
        if ( GUCEF_NULL != m_buffer )
            m_buffer->AddReference();
        if ( GUCEF_NULL != previous )
            previous->ReleaseReference();
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPtr::Reset( void )
{

    if ( GUCEF_NULL != m_buffer )
    {
        CDataTableBuffer* previous = m_buffer;
        m_buffer = GUCEF_NULL;
        previous->ReleaseReference();
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPtr::Swap( CDataTableBufferPtr& other )
{

    CDataTableBuffer* mine = m_buffer;
    m_buffer = other.m_buffer;
    other.m_buffer = mine;
}

/*-------------------------------------------------------------------------*/

void
CDataTableBufferPtr::Adopt( CDataTableBuffer* buffer )
{

    Reset();
    m_buffer = buffer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
