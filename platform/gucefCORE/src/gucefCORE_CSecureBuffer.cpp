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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#include "gucefCORE_CSecureBuffer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CSecureBuffer::TReleaseAuditor g_releaseAuditor = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSecureBuffer::SetReleaseAuditor( TReleaseAuditor auditor )
{GUCEF_TRACE;

    g_releaseAuditor = auditor;
}

/*-------------------------------------------------------------------------*/

CSecureBuffer::CSecureBuffer( void )
    : m_storage( GUCEF_NULL )
    , m_capacity( 0 )
    , m_size( 0 )
    , m_ownsStorage( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecureBuffer::CSecureBuffer( UInt32 capacity )
    : m_storage( GUCEF_NULL )
    , m_capacity( 0 )
    , m_size( 0 )
    , m_ownsStorage( true )
{GUCEF_TRACE;

    SetCapacity( capacity );
}

/*-------------------------------------------------------------------------*/

CSecureBuffer::CSecureBuffer( void* storage, UInt32 capacity )
    : m_storage( static_cast< UInt8* >( storage ) )
    , m_capacity( capacity )
    , m_size( 0 )
    , m_ownsStorage( false )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_storage )
        m_capacity = 0;
    else
        SecureWipe( m_storage, m_capacity + 1 );
}

/*-------------------------------------------------------------------------*/

CSecureBuffer::CSecureBuffer( const CSecureBuffer& src )
    : m_storage( GUCEF_NULL )
    , m_capacity( 0 )
    , m_size( 0 )
    , m_ownsStorage( true )
{GUCEF_TRACE;

    if ( SetCapacity( src.m_size ) )
    {
        if ( src.m_size > 0 )
            ::memcpy( m_storage, src.m_storage, src.m_size );
        m_size = src.m_size;
    }
}

/*-------------------------------------------------------------------------*/

CSecureBuffer::~CSecureBuffer()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CSecureBuffer::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_storage )
    {
        SecureWipe( m_storage, m_capacity + 1 );
        if ( m_ownsStorage )
        {
            if ( GUCEF_NULL != g_releaseAuditor )
                g_releaseAuditor( m_storage, m_capacity + 1 );
            GUCEF_DELETE [] m_storage;
        }
    }
    m_storage = GUCEF_NULL;
    m_capacity = 0;
    m_size = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecureBuffer::GetCapacity( void ) const
{GUCEF_TRACE;

    return m_capacity;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecureBuffer::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::IsEmpty( void ) const
{GUCEF_TRACE;

    return 0 == m_size;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::IsOwningStorage( void ) const
{GUCEF_TRACE;

    return m_ownsStorage;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CSecureBuffer::GetData( void ) const
{GUCEF_TRACE;

    static const UInt8 empty[ 1 ] = { 0 };
    return GUCEF_NULL != m_storage ? m_storage : empty;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::Assign( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( size > 0 && GUCEF_NULL == data )
        return false;

    // the new content may be inside the old: it is copied before the old is wiped
    if ( size > m_capacity )
    {
        Clear();
        return false;
    }
    if ( size > 0 && GUCEF_NULL != m_storage )
        ::memmove( m_storage, data, size );
    if ( m_size > size )
        SecureWipe( m_storage + size, m_size - size );
    if ( GUCEF_NULL != m_storage )
        m_storage[ size ] = 0;
    m_size = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::Append( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return true;
    if ( GUCEF_NULL == data || size > m_capacity - m_size )
        return false;
    ::memcpy( m_storage + m_size, data, size );
    m_size += size;
    m_storage[ m_size ] = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::CopyFrom( const CSecureBuffer& src )
{GUCEF_TRACE;

    if ( &src == this )
        return true;
    return Assign( src.m_storage, src.m_size );
}

/*-------------------------------------------------------------------------*/

void
CSecureBuffer::Truncate( UInt32 size )
{GUCEF_TRACE;

    if ( size >= m_size )
        return;
    SecureWipe( m_storage + size, m_size - size );
    m_size = size;
}

/*-------------------------------------------------------------------------*/

void
CSecureBuffer::Clear( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_storage )
        SecureWipe( m_storage, m_capacity + 1 );
    m_size = 0;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::SetCapacity( UInt32 capacity )
{GUCEF_TRACE;

    if ( !m_ownsStorage )
        return false;
    if ( capacity == m_capacity && ( GUCEF_NULL != m_storage || 0 == capacity ) )
        return true;
    if ( 0 == capacity )
    {
        Release();
        return true;
    }

    UInt8* storage = GUCEF_NEW UInt8[ capacity + 1 ];
    if ( GUCEF_NULL == storage )
        return false;
    ::memset( storage, 0, capacity + 1 );
    UInt32 kept = m_size < capacity ? m_size : capacity;
    if ( kept > 0 )
        ::memcpy( storage, m_storage, kept );

    Release();
    m_storage = storage;
    m_capacity = capacity;
    m_size = kept;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::Equals( const void* data, UInt32 size ) const
{GUCEF_TRACE;

    if ( size != m_size )
        return false;
    if ( 0 == size )
        return true;
    if ( GUCEF_NULL == data )
        return false;
    const UInt8* other = static_cast< const UInt8* >( data );
    UInt8 difference = 0;
    for ( UInt32 i=0; i<size; ++i )
        difference |= (UInt8) ( m_storage[ i ] ^ other[ i ] );
    return 0 == difference;
}

/*-------------------------------------------------------------------------*/

UInt8*
CSecureBuffer::GetWritableData( void )
{GUCEF_TRACE;

    Clear();
    return m_storage;
}

/*-------------------------------------------------------------------------*/

bool
CSecureBuffer::SetSize( UInt32 size )
{GUCEF_TRACE;

    if ( size > m_capacity )
        return false;
    if ( GUCEF_NULL != m_storage )
    {
        // whatever was written beyond the new size is not content any more
        if ( size < m_capacity )
            SecureWipe( m_storage + size, m_capacity - size );
        m_storage[ size ] = 0;
    }
    m_size = size;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSecureBuffer::CopyToString( CString& destination ) const
{GUCEF_TRACE;

    SecureWipeString( destination );
    if ( m_size > 0 )
        destination.Set( reinterpret_cast< const char* >( m_storage ), m_size );
}

/*-------------------------------------------------------------------------*/

const UInt8*
CSecureBuffer::GetRawStorage( void ) const
{GUCEF_TRACE;

    return m_storage;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecureBuffer::GetRawStorageSize( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_storage ? m_capacity + 1 : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
