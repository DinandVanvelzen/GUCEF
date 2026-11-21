/*
 *  assetpluginUNREAL: Reader of the payloads that are compressed buffers (stored or Oodle blocks)
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealCompressedBuffer::CUnrealCompressedBuffer( CUnrealPackage& package, CIUnrealOodleProvider* provider )
    : m_package( package )
    , m_provider( provider )
    , m_cancellation( GUCEF_NULL )
    , m_reference()
    , m_header()
    , m_blockOffsets()
    , m_compressed()
    , m_open( false )
    , m_providerMissing( false )
    , m_cancelled( false )
    , m_toolCode( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealCompressedBuffer::~CUnrealCompressedBuffer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealCompressedBuffer::SetCancellationSource( const ASSETCONVERTER::CConversionReport* report )
{GUCEF_TRACE;

    m_cancellation = report;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBuffer::Fail( const CORE::CString& reason )
{GUCEF_TRACE;

    m_error = reason;
    return false;
}

/*-------------------------------------------------------------------------*/

bool CUnrealCompressedBuffer::IsOpen( void ) const { return m_open; }
const CUnrealCompressedBufferHeader& CUnrealCompressedBuffer::GetHeader( void ) const { return m_header; }
UInt64 CUnrealCompressedBuffer::GetRawSize( void ) const { return m_header.rawSize; }
bool CUnrealCompressedBuffer::NeedsOodle( void ) const { return m_open && CUnrealCompressedBufferHeader::METHOD_OODLE == m_header.method; }
bool CUnrealCompressedBuffer::IsProviderMissing( void ) const { return m_providerMissing; }
bool CUnrealCompressedBuffer::IsCancelled( void ) const { return m_cancelled; }
Int32 CUnrealCompressedBuffer::GetToolCode( void ) const { return m_toolCode; }
const CORE::CString& CUnrealCompressedBuffer::GetError( void ) const { return m_error; }

/*-------------------------------------------------------------------------*/

UInt32
CUnrealCompressedBuffer::HeaderChecksum( const UInt8* bytes, UInt32 size )
{GUCEF_TRACE;

    // the CRC-32 of zip and PNG (reflected, polynomial 0x04C11DB7), found by comparing with the headers of the owned packages
    return CUnrealPngWriter::Crc32( bytes, size );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBuffer::Open( const CUnrealPayloadReference& reference )
{GUCEF_TRACE;

    m_open = false;
    m_providerMissing = false;
    m_cancelled = false;
    m_toolCode = 0;
    m_error.Clear();
    m_blockOffsets.clear();
    m_reference = reference;
    if ( !reference.IsReadable() )
        return Fail( "The payload is not available in this package: " + reference.reason );

    CUnrealArchiveReader reader;
    if ( !m_package.OpenPayload( reference, reader ) )
        return Fail( "The payload cannot be opened" );
    if ( !m_header.Read( reader, reference.storedSize ) )
        return Fail( "The header of the compressed buffer is not valid: " + reader.GetError() );
    if ( CUnrealCompressedBufferHeader::METHOD_NONE != m_header.method && CUnrealCompressedBufferHeader::METHOD_OODLE != m_header.method )
        return Fail( "The compressed buffer uses method " + CORE::UInt32ToString( m_header.method ) + ", which this reader does not decompress" );
    const CUnrealReaderLimits& limits = m_package.GetLimits();
    if ( m_header.rawSize > limits.maxPayloadRawBytes )
        return Fail( "The payload holds " + CORE::UInt64ToString( m_header.rawSize ) + " bytes, which is over the budget of " + CORE::UInt64ToString( limits.maxPayloadRawBytes ) );

    if ( !m_header.IsStored() )
    {
        const UInt64 blockSize = (UInt64) 1 << m_header.blockSizeLog2;
        if ( blockSize > limits.maxBlockBytes )
            return Fail( "The blocks of the compressed buffer are " + CORE::UInt64ToString( blockSize ) + " bytes, which is over the budget of " + CORE::UInt32ToString( limits.maxBlockBytes ) );
        if ( m_header.blockCount != ( m_header.rawSize + blockSize - 1 ) / blockSize )
            return Fail( "The compressed buffer has " + CORE::UInt32ToString( m_header.blockCount ) + " blocks, which does not fit its raw size of " + CORE::UInt64ToString( m_header.rawSize ) );
        UInt64 total = m_header.GetHeaderBytes();
        m_blockOffsets.reserve( m_header.blockCount );
        for ( UInt32 b=0; b<m_header.blockCount; ++b )
        {
            m_blockOffsets.push_back( total );
            total += m_header.blockSizes[ b ];
            if ( m_header.blockSizes[ b ] > RawBlockSize( b ) + 65536 || 0 == m_header.blockSizes[ b ] )
                return Fail( "Block " + CORE::UInt32ToString( b ) + " of the compressed buffer has a size that is not possible" );
        }
        if ( total != m_header.compressedSize )
            return Fail( "The blocks of the compressed buffer add up to " + CORE::UInt64ToString( total ) + " bytes where its header says " + CORE::UInt64ToString( m_header.compressedSize ) );
        if ( total > reference.storedSize )
            return Fail( "The compressed buffer is larger than its payload" );
    }
    else if ( 64 + m_header.rawSize > reference.storedSize )
        return Fail( "The stored buffer is larger than its payload" );

    // the checksum of the header: the bytes after the checksum field up to the end of the block table
    {
        CUnrealArchiveReader again;
        const UInt64 headerBytes = m_header.GetHeaderBytes();
        if ( !m_package.OpenPayload( reference, again ) || headerBytes > reference.storedSize )
            return Fail( "The header of the compressed buffer cannot be read again" );
        GUCEF::vector< UInt8 > bytes( (size_t) headerBytes );
        if ( !again.Read( &bytes[ 0 ], headerBytes ) )
            return Fail( "The header of the compressed buffer cannot be read again" );
        const UInt32 expected = HeaderChecksum( &bytes[ 8 ], (UInt32) headerBytes - 8 );
        if ( expected != m_header.crc32 )
            return Fail( "The checksum of the header of the compressed buffer does not match" );
    }
    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealCompressedBuffer::RawBlockSize( UInt32 block ) const
{GUCEF_TRACE;

    const UInt64 blockSize = (UInt64) 1 << m_header.blockSizeLog2;
    const UInt64 start = (UInt64) block * blockSize;
    if ( start >= m_header.rawSize )
        return 0;
    const UInt64 left = m_header.rawSize - start;
    return (UInt32) ( left < blockSize ? left : blockSize );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBuffer::ReadBlock( CUnrealArchiveReader& reader, UInt32 block, void* dest )
{GUCEF_TRACE;

    const UInt32 rawSize = RawBlockSize( block );
    const UInt32 compressedSize = m_header.blockSizes[ block ];
    if ( !reader.Seek( reader.GetStart() + m_blockOffsets[ block ] ) )
        return Fail( "Block " + CORE::UInt32ToString( block ) + " cannot be located in the payload: " + reader.GetError() );
    if ( compressedSize == rawSize )
    {
        // the encoder keeps a block that it could not make smaller as it is
        if ( !reader.Read( dest, rawSize ) )
            return Fail( "Block " + CORE::UInt32ToString( block ) + " cannot be read: " + reader.GetError() );
        return true;
    }
    if ( GUCEF_NULL == m_provider )
    {
        m_providerMissing = true;
        return Fail( CUnrealCapabilities::GetNeedsOodleText() );
    }
    if ( !m_provider->IsAvailable() )
    {
        m_providerMissing = true;
        return Fail( m_provider->GetUnavailableReason() );
    }
    if ( m_compressed.size() < compressedSize )
        m_compressed.resize( compressedSize );
    if ( !reader.Read( &m_compressed[ 0 ], compressedSize ) )
        return Fail( "Block " + CORE::UInt32ToString( block ) + " cannot be read: " + reader.GetError() );
    CORE::CString error;
    Int32 code = 0;
    if ( !m_provider->DecodeBlock( &m_compressed[ 0 ], compressedSize, rawSize, dest, error, code ) )
    {
        m_toolCode = code;
        m_cancelled = "cancelled" == error;
        return Fail( "Block " + CORE::UInt32ToString( block ) + " of " + CORE::UInt32ToString( m_header.blockCount ) + " cannot be decompressed: " + error );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBuffer::ReadRange( UInt64 offset, UInt64 size, void* dest )
{GUCEF_TRACE;

    if ( !m_open )
        return Fail( "The compressed buffer is not open" );
    if ( offset > m_header.rawSize || size > m_header.rawSize - offset )
        return Fail( "The range is outside the raw bytes of the payload" );
    if ( 0 == size )
        return true;
    m_providerMissing = false;
    m_cancelled = false;
    m_toolCode = 0;

    CUnrealArchiveReader reader;
    if ( !m_package.OpenPayload( m_reference, reader ) )
        return Fail( "The payload cannot be opened" );
    if ( m_header.IsStored() )
    {
        if ( !reader.Seek( reader.GetStart() + 64 + offset ) || !reader.Read( dest, size ) )
            return Fail( "The stored bytes cannot be read: " + reader.GetError() );
        return true;
    }

    const UInt64 blockSize = (UInt64) 1 << m_header.blockSizeLog2;
    UInt8* out = static_cast< UInt8* >( dest );
    GUCEF::vector< UInt8 > block;
    UInt64 position = offset;
    const UInt64 end = offset + size;
    while ( position < end )
    {
        if ( GUCEF_NULL != m_cancellation && m_cancellation->IsCancellationRequested() )
        {
            m_cancelled = true;
            return Fail( "cancelled" );
        }
        const UInt32 index = (UInt32) ( position / blockSize );
        const UInt64 blockStart = (UInt64) index * blockSize;
        const UInt32 rawSize = RawBlockSize( index );
        const UInt64 from = position - blockStart;
        const UInt64 count = ( end - position < rawSize - from ) ? end - position : rawSize - from;
        if ( 0 == from && count == rawSize )
        {
            if ( !ReadBlock( reader, index, out ) )
                return false;
        }
        else
        {
            if ( block.size() < rawSize )
                block.resize( rawSize );
            if ( !ReadBlock( reader, index, &block[ 0 ] ) )
                return false;
            memcpy( out, &block[ from ], (size_t) count );
        }
        out += count;
        position += count;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBuffer::ReadAll( CORE::CDynamicBuffer& out )
{GUCEF_TRACE;

    if ( !m_open )
        return Fail( "The compressed buffer is not open" );
    if ( m_header.rawSize > 0xFFFFFFF0ULL || m_header.rawSize > m_package.GetLimits().maxPayloadRawBytes )
        return Fail( "The payload is over the budget of memory" );
    out.SetBufferSize( (UInt32) m_header.rawSize, false );
    out.SetDataSize( (UInt32) m_header.rawSize );
    return ReadRange( 0, m_header.rawSize, out.GetBufferPtr() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
