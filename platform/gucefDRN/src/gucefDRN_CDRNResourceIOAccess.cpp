/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <stdio.h>
#include <string.h>

#include "gucefDRN_CDRNResourceIOAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceIOAccess::CDRNResourceIOAccess( const CDRNResourceHandlePtr& handle, UInt32 cacheChunks, UInt32 readAheadChunks )
    : CORE::CIOAccess()
    , m_handle( handle )
    , m_cacheChunks( cacheChunks < 1 ? 1 : cacheChunks )
    , m_readAheadChunks( readAheadChunks )
    , m_opened( false )
    , m_size( 0 )
    , m_position( 0 )
    , m_lastReadEnd( 0 )
    , m_cache()
    , m_order()
    , m_lastResult()
    , m_chunksFetched( 0 )
    , m_cacheHits( 0 )
{GUCEF_TRACE;

    if ( m_readAheadChunks >= m_cacheChunks )
        m_readAheadChunks = m_cacheChunks - 1;
}

/*-------------------------------------------------------------------------*/

CDRNResourceIOAccess::~CDRNResourceIOAccess()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const CDRNResourceHandlePtr&
CDRNResourceIOAccess::GetHandle( void ) const
{GUCEF_TRACE;

    return m_handle;
}

/*-------------------------------------------------------------------------*/

const CDRNResourceTransferResult&
CDRNResourceIOAccess::GetLastResult( void ) const
{GUCEF_TRACE;

    return m_lastResult;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceIOAccess::GetChunksFetched( void ) const
{GUCEF_TRACE;

    return m_chunksFetched;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceIOAccess::GetCacheHits( void ) const
{GUCEF_TRACE;

    return m_cacheHits;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceIOAccess::Open( void )
{GUCEF_TRACE;

    if ( m_opened )
        return;
    if ( m_handle.IsNULL() )
    {
        m_lastResult = CDRNResourceTransferResult( DRN_RESSTATUS_CLOSED, "no handle" );
        return;
    }
    if ( !m_handle->IsOpen() && !m_handle->WaitOpen( m_handle->GetOptions().openTimeoutMs ) )
    {
        m_lastResult = m_handle->GetResult();
        return;
    }
    m_size = m_handle->GetInfo().size;
    m_position = 0;
    m_lastReadEnd = 0;
    m_opened = true;
    m_lastResult = CDRNResourceTransferResult();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceIOAccess::Close( void )
{GUCEF_TRACE;

    m_opened = false;
    m_cache.clear();
    m_order.clear();
    m_handle.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_opened;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceIOAccess::Remember( UInt64 chunk, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( m_cache.find( chunk ) != m_cache.end() )
        return;
    while ( m_cache.size() >= m_cacheChunks && !m_order.empty() )
    {
        m_cache.erase( m_order.front() );
        m_order.erase( m_order.begin() );
    }
    m_cache[ chunk ] = data;
    m_order.push_back( chunk );
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer*
CDRNResourceIOAccess::GetChunk( UInt64 chunk, bool sequential )
{GUCEF_TRACE;

    TChunkCache::iterator cached = m_cache.find( chunk );
    if ( cached != m_cache.end() )
    {
        // most recently used goes last
        for ( UInt32 i=0; i<m_order.size(); ++i )
        {
            if ( m_order[ i ] == chunk )
            {
                m_order.erase( m_order.begin() + i );
                break;
            }
        }
        m_order.push_back( chunk );
        ++m_cacheHits;
        return &(*cached).second;
    }

    // the chunk and, for a read that goes on where the last ended, the ones after it that are not cached yet
    UInt64 chunkCount = CDRNResourceInfo::ChunkCountFor( m_size );
    UInt32 count = 1;
    UInt32 wanted = sequential ? 1 + m_readAheadChunks : 1;
    while ( count < wanted && chunk + count < chunkCount && m_cache.find( chunk + count ) == m_cache.end() )
        ++count;

    GUCEF::vector< CORE::CDynamicBuffer > chunks;
    m_lastResult = m_handle->ReadChunks( chunk, count, chunks );
    if ( !m_lastResult.IsOk() || chunks.size() != count )
        return GUCEF_NULL;
    m_chunksFetched += count;
    for ( UInt32 i=0; i<count; ++i )
        Remember( chunk + i, chunks[ i ] );
    cached = m_cache.find( chunk );
    return cached != m_cache.end() ? &(*cached).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( !m_opened || GUCEF_NULL == dest || 0 == esize || 0 == elements || m_position >= m_size )
        return 0;

    // whole elements only, as fread() gives them
    UInt64 available = m_size - m_position;
    UInt64 wantedElements = elements;
    if ( wantedElements * esize > available )
        wantedElements = available / esize;
    if ( 0 == wantedElements )
        return 0;
    UInt64 bytes = wantedElements * esize;

    bool sequential = m_position == m_lastReadEnd;
    UInt8* out = static_cast< UInt8* >( dest );
    UInt64 done = 0;
    while ( done < bytes )
    {
        UInt64 at = m_position + done;
        UInt64 chunk = at / DRN_RESOURCE_CHUNK_SIZE;
        const CORE::CDynamicBuffer* data = GetChunk( chunk, sequential );
        if ( GUCEF_NULL == data )
            break;
        UInt64 inChunk = at - chunk * DRN_RESOURCE_CHUNK_SIZE;
        if ( inChunk >= data->GetDataSize() )
            break;
        UInt64 n = data->GetDataSize() - inChunk;
        if ( n > bytes - done )
            n = bytes - done;
        memcpy( out + done, static_cast< const UInt8* >( data->GetConstBufferPtr() ) + inChunk, (size_t) n );
        done += n;
        sequential = true;
    }

    // a read that failed part way gives the whole elements it got
    UInt32 elementsRead = (UInt32) ( done / esize );
    m_position += (UInt64) elementsRead * esize;
    m_lastReadEnd = m_position;
    return elementsRead;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

Int32
CDRNResourceIOAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    if ( !m_opened )
        return -1;
    Int64 base = 0;
    switch ( origin )
    {
        case SEEK_SET: { base = 0; break; }
        case SEEK_CUR: { base = (Int64) m_position; break; }
        case SEEK_END: { base = (Int64) m_size; break; }
        default: return -1;
    }
    Int64 target = base + offset;
    if ( target < 0 )
        return -1;
    m_position = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    if ( !m_opened )
        return 1;
    m_position = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CDRNResourceIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    if ( 1 != Read( &c, 1, 1 ) )
        return (char) EOF;
    return c;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return !m_opened || m_position >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceIOAccess::IsValid( void )
{GUCEF_TRACE;

    return m_opened && !m_handle.IsNULL() && m_handle->IsOpen();
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDRNResourceIOAccess::Clone( void ) const
{GUCEF_TRACE;

    CDRNResourceIOAccess* clone = GUCEF_NEW CDRNResourceIOAccess( m_handle, m_cacheChunks, m_readAheadChunks );
    clone->m_opened = m_opened;
    clone->m_size = m_size;
    clone->m_position = m_position;
    clone->m_lastReadEnd = m_lastReadEnd;
    return clone;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceIOAccess::Write( const void* /* srcdata */, UInt32 /* esize */, UInt32 /* elements */ )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
