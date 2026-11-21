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

#include <string.h>

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#include "gucefDRN_CDRNResourceHasher.h"
#define GUCEF_DRN_CDRNRESOURCEHASHER_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */

#include "gucefDRN_CDRNResourcePartialState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char StateMagic[ 8 ] = { 'G', 'U', 'D', 'R', 'N', 'R', 'S', '1' };

/** The bitmap fits a CDynamicBuffer: up to 2^35 chunks, 2 PiB at 64 KiB */
static const UInt64 MaxStateChunks = (UInt64) 0xFFFFFFF0 * 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourcePartialState::CDRNResourcePartialState( void )
    : m_provider()
    , m_name()
    , m_info()
    , m_bitmap()
    , m_chunkCount( 0 )
    , m_heldCount( 0 )
    , m_valid( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNResourcePartialState::Clear( void )
{GUCEF_TRACE;

    m_provider.Clear();
    m_name.Clear();
    m_info.Clear();
    m_bitmap.Clear();
    m_chunkCount = 0;
    m_heldCount = 0;
    m_valid = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourcePartialState::Reset( const CORE::CString& provider, const CORE::CString& name, const CDRNResourceInfo& info )
{GUCEF_TRACE;

    Clear();
    UInt64 chunkCount = info.GetChunkCount();
    if ( chunkCount > MaxStateChunks )
        return false;
    UInt32 bitmapBytes = (UInt32) ( ( chunkCount + 7 ) / 8 );
    if ( bitmapBytes > 0 && !m_bitmap.SetBufferSize( bitmapBytes, false, true ) )
        return false;
    m_bitmap.SetDataSize( bitmapBytes );
    if ( bitmapBytes > 0 )
        memset( m_bitmap.GetBufferPtr(), 0, bitmapBytes );
    m_provider = provider;
    m_name = name;
    m_info = info;
    m_chunkCount = chunkCount;
    m_valid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourcePartialState::IsOf( const CORE::CString& provider, const CORE::CString& name, const CDRNResourceInfo& info ) const
{GUCEF_TRACE;

    return m_valid && m_provider == provider && m_name == name && m_info.IsSameRevisionAs( info );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourcePartialState::IsValid( void ) const
{GUCEF_TRACE;

    return m_valid;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourcePartialState::GetProvider( void ) const
{GUCEF_TRACE;

    return m_provider;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourcePartialState::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CDRNResourceInfo&
CDRNResourcePartialState::GetInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourcePartialState::GetChunkCount( void ) const
{GUCEF_TRACE;

    return m_chunkCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourcePartialState::GetHeldCount( void ) const
{GUCEF_TRACE;

    return m_heldCount;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourcePartialState::IsHeld( UInt64 chunkIndex ) const
{GUCEF_TRACE;

    if ( chunkIndex >= m_chunkCount )
        return false;
    const UInt8* bits = static_cast< const UInt8* >( m_bitmap.GetConstBufferPtr() );
    return 0 != ( bits[ chunkIndex >> 3 ] & ( 1 << ( chunkIndex & 7 ) ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourcePartialState::SetHeld( UInt64 chunkIndex, bool held )
{GUCEF_TRACE;

    if ( chunkIndex >= m_chunkCount || IsHeld( chunkIndex ) == held )
        return;
    UInt8* bits = static_cast< UInt8* >( m_bitmap.GetBufferPtr() );
    if ( held )
    {
        bits[ chunkIndex >> 3 ] |= (UInt8) ( 1 << ( chunkIndex & 7 ) );
        ++m_heldCount;
    }
    else
    {
        bits[ chunkIndex >> 3 ] &= (UInt8) ~( 1 << ( chunkIndex & 7 ) );
        --m_heldCount;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourcePartialState::GetHeldBytes( void ) const
{GUCEF_TRACE;

    if ( 0 == m_heldCount )
        return 0;
    UInt64 bytes = m_heldCount * DRN_RESOURCE_CHUNK_SIZE;
    UInt64 last = m_chunkCount - 1;
    if ( IsHeld( last ) )
        bytes -= DRN_RESOURCE_CHUNK_SIZE - m_info.GetChunkBytes( last );
    return bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourcePartialState::FindFirstMissing( UInt64 from ) const
{GUCEF_TRACE;

    const UInt8* bits = static_cast< const UInt8* >( m_bitmap.GetConstBufferPtr() );
    UInt64 i = from;
    while ( i < m_chunkCount )
    {
        // whole bytes of held chunks are skipped at once
        if ( 0 == ( i & 7 ) && 0xFF == bits[ i >> 3 ] && i + 8 <= m_chunkCount )
        {
            i += 8;
            continue;
        }
        if ( !IsHeld( i ) )
            return i;
        ++i;
    }
    return m_chunkCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourcePartialState::FindFirstHeld( UInt64 from ) const
{GUCEF_TRACE;

    const UInt8* bits = static_cast< const UInt8* >( m_bitmap.GetConstBufferPtr() );
    UInt64 i = from;
    while ( i < m_chunkCount )
    {
        if ( 0 == ( i & 7 ) && 0 == bits[ i >> 3 ] )
        {
            i += 8;
            continue;
        }
        if ( IsHeld( i ) )
            return i;
        ++i;
    }
    return m_chunkCount;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourcePartialState::Serialize( CORE::CDynamicBuffer& output ) const
{GUCEF_TRACE;

    output.SetDataSize( 0 );
    CDRNWireWriter writer( output );
    writer.WriteBytes( StateMagic, sizeof( StateMagic ) );
    writer.WriteUInt8( FormatVersion );
    writer.WriteString( m_provider );
    writer.WriteString( m_name );
    writer.WriteVarUInt64( m_info.size );
    writer.WriteVarUInt64( m_info.modifiedUtcMs );
    writer.WriteString( m_info.revision );
    writer.WriteVarUInt32( DRN_RESOURCE_CHUNK_SIZE );
    UInt8 flags = ( m_info.hasSha256 ? DRN_RES_DIGEST_SHA256 : 0 ) | ( m_info.hasTreeRoot ? DRN_RES_DIGEST_TREE : 0 );
    writer.WriteUInt8( flags );
    if ( m_info.hasSha256 )
        writer.WriteBytes( m_info.sha256, CDRNResourceInfo::DigestSize );
    if ( m_info.hasTreeRoot )
        writer.WriteBytes( m_info.treeRoot, CDRNResourceInfo::DigestSize );
    writer.WriteVarUInt64( m_chunkCount );
    writer.WriteSizedBytes( m_bitmap.GetConstBufferPtr(), m_bitmap.GetDataSize() );

    UInt8 checksum[ CDRNResourceHasher::DigestSize ];
    CDRNResourceHasher hasher;
    hasher.Hash( output.GetConstBufferPtr(), writer.GetOffset(), checksum );
    writer.WriteBytes( checksum, sizeof( checksum ) );
    output.SetDataSize( writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourcePartialState::Deserialize( const CORE::CDynamicBuffer& input )
{GUCEF_TRACE;

    Clear();
    UInt32 total = input.GetDataSize();
    if ( total < sizeof( StateMagic ) + CDRNResourceHasher::DigestSize )
        return false;

    UInt32 contentSize = total - CDRNResourceHasher::DigestSize;
    UInt8 checksum[ CDRNResourceHasher::DigestSize ];
    CDRNResourceHasher hasher;
    hasher.Hash( input.GetConstBufferPtr(), contentSize, checksum );
    if ( !CORE::CSha256::ConstantTimeEquals( checksum, static_cast< const UInt8* >( input.GetConstBufferPtr() ) + contentSize, sizeof( checksum ) ) )
        return false;

    CDRNWireReader reader( input.GetConstBufferPtr(), contentSize );
    char magic[ sizeof( StateMagic ) ];
    UInt8 version = 0;
    CORE::CString provider;
    CORE::CString name;
    CDRNResourceInfo info;
    UInt32 chunkSize = 0;
    UInt8 flags = 0;
    UInt64 chunkCount = 0;
    if ( !reader.ReadBytes( magic, sizeof( magic ) ) || 0 != memcmp( magic, StateMagic, sizeof( magic ) ) ||
         !reader.ReadUInt8( version ) || FormatVersion != version ||
         !reader.ReadString( provider ) || !reader.ReadString( name ) ||
         !reader.ReadVarUInt64( info.size ) || !reader.ReadVarUInt64( info.modifiedUtcMs ) || !reader.ReadString( info.revision ) ||
         !reader.ReadVarUInt32( chunkSize ) || DRN_RESOURCE_CHUNK_SIZE != chunkSize || !reader.ReadUInt8( flags ) )
        return false;
    info.name = name;
    info.hasSha256 = 0 != ( flags & DRN_RES_DIGEST_SHA256 );
    info.hasTreeRoot = 0 != ( flags & DRN_RES_DIGEST_TREE );
    if ( ( info.hasSha256 && !reader.ReadBytes( info.sha256, CDRNResourceInfo::DigestSize ) ) ||
         ( info.hasTreeRoot && !reader.ReadBytes( info.treeRoot, CDRNResourceInfo::DigestSize ) ) )
        return false;
    const UInt8* bitmap = GUCEF_NULL;
    UInt32 bitmapSize = 0;
    if ( !reader.ReadVarUInt64( chunkCount ) || chunkCount != info.GetChunkCount() || !reader.ReadSizedBytes( bitmap, bitmapSize ) ||
         (UInt64) bitmapSize != ( chunkCount + 7 ) / 8 || 0 != reader.GetRemaining() )
        return false;

    if ( !Reset( provider, name, info ) )
        return false;
    if ( bitmapSize > 0 )
        memcpy( m_bitmap.GetBufferPtr(), bitmap, bitmapSize );
    m_heldCount = 0;
    for ( UInt64 i=0; i<m_chunkCount; ++i )
        if ( IsHeld( i ) )
            ++m_heldCount;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
