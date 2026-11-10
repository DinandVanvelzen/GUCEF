/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#include "tablecodecpluginPARQUET_ParquetMetaData.h"
#define TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H ? */

#include "tablecodecpluginPARQUET_CParquetCompression.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  Output into a fixed block of memory that refuses every write past its end: the bound of a decompression.
 *  A write that does not fit writes nothing and returns 0 elements, which codecs treat as an output that stopped.
 */
class CBoundedMemoryOutput : public CORE::CIOAccess
{
    public:

    CBoundedMemoryOutput( UInt8* memory, UInt32 capacity )
        : CORE::CIOAccess()
        , m_memory( memory )
        , m_capacity( capacity )
        , m_position( 0 )
        , m_overflowed( false )
    {
    }

    virtual ~CBoundedMemoryOutput() GUCEF_VIRTUAL_OVERRIDE {}

    using CORE::CIOAccess::Write;
    using CORE::CIOAccess::Read;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE { m_position = 0; }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt32 Read( void*, UInt32, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return 0; }

    virtual UInt32 Write( const void* source, UInt32 elementSize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 bytes = (UInt64) elementSize * elements;
        if ( 0 == bytes )
            return 0;
        if ( bytes > (UInt64) ( m_capacity - m_position ) )
        {
            m_overflowed = true;
            return 0;
        }
        memcpy( m_memory + m_position, source, (size_t) bytes );
        m_position += (UInt32) bytes;
        return elements;
    }

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }
    virtual Int32 Seek( Int64, Int32 ) GUCEF_VIRTUAL_OVERRIDE { return -1; }
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_capacity; }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    inline UInt32 GetWritten( void ) const { return m_position; }
    inline bool HasOverflowed( void ) const { return m_overflowed; }

    private:

    UInt8* m_memory;
    UInt32 m_capacity;
    UInt32 m_position;
    bool m_overflowed;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParquetCompression::CParquetCompression( void )
    : m_parquetCodec( PARQUET_COMPRESSION_UNCOMPRESSED )
    , m_typeName()
    , m_codec()
    , m_params()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CParquetCompression::GetCodecTypeName( Int32 parquetCodec, CORE::CString& typeName )
{GUCEF_TRACE;

    switch ( parquetCodec )
    {
        case PARQUET_COMPRESSION_SNAPPY: { typeName = "snappy"; return true; }
        case PARQUET_COMPRESSION_GZIP: { typeName = "gzip"; return true; }
        case PARQUET_COMPRESSION_LZO: { typeName = "lzo"; return true; }
        case PARQUET_COMPRESSION_BROTLI: { typeName = "brotli"; return true; }
        case PARQUET_COMPRESSION_ZSTD: { typeName = "zstd"; return true; }
        case PARQUET_COMPRESSION_LZ4_RAW: { typeName = "lz4_raw"; return true; }
        default: { typeName.Clear(); return false; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CParquetCompression::ParquetCodecFromName( const CORE::CString& name, Int32& parquetCodec )
{GUCEF_TRACE;

    const CORE::CString lower = name.Lowercase();
    if ( lower.IsNULLOrEmpty() || lower == "none" || lower == "uncompressed" ) { parquetCodec = PARQUET_COMPRESSION_UNCOMPRESSED; return true; }
    if ( lower == "gzip" ) { parquetCodec = PARQUET_COMPRESSION_GZIP; return true; }
    if ( lower == "snappy" ) { parquetCodec = PARQUET_COMPRESSION_SNAPPY; return true; }
    if ( lower == "zstd" ) { parquetCodec = PARQUET_COMPRESSION_ZSTD; return true; }
    if ( lower == "lz4_raw" ) { parquetCodec = PARQUET_COMPRESSION_LZ4_RAW; return true; }
    if ( lower == "brotli" ) { parquetCodec = PARQUET_COMPRESSION_BROTLI; return true; }
    if ( lower == "lzo" ) { parquetCodec = PARQUET_COMPRESSION_LZO; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetCompression::Init( Int32 parquetCodec, Int32 level, CORE::CString& error )
{GUCEF_TRACE;

    m_parquetCodec = parquetCodec;
    m_codec.Unlink();
    m_params.Clear();
    m_typeName.Clear();
    if ( PARQUET_COMPRESSION_UNCOMPRESSED == parquetCodec )
        return CORE::DATATABLE_STATUS_OK;

    if ( !GetCodecTypeName( parquetCodec, m_typeName ) )
    {
        error = "Compression " + CParquetMetaDataSerializer::CompressionToString( parquetCodec ) + " is not supported";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    m_codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, m_typeName );
    if ( m_codec.IsNULL() )
    {
        error = "Compression " + CParquetMetaDataSerializer::CompressionToString( parquetCodec ) + " needs a \"" + m_typeName +
                "\" codec of the " + CORE::CoreCodecTypes::CompressionCodec + " family, and none is registered";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    if ( CORE::CDataTableWriterOptions::DefaultCompressionLevel != level )
        m_params.Set( "level", CORE::CVariant( level ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetCompression::Decompress( const UInt8* input, UInt32 inputSize, UInt8* output, UInt32 outputSize, CORE::CString& error )
{GUCEF_TRACE;

    if ( m_codec.IsNULL() )
    {
        error = "No decompression codec was prepared";
        return CORE::DATATABLE_STATUS_FAILED;
    }
    if ( PARQUET_COMPRESSION_GZIP == m_parquetCodec && (UInt64) outputSize > (UInt64) inputSize * GzipMaxExpansion + 1024 )
    {
        error = "A GZIP page of " + CORE::ToString( inputSize ) + " bytes announces " + CORE::ToString( outputSize ) +
                " uncompressed bytes, more than deflate can produce";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    CORE::CMFileAccess source( input, inputSize );
    CBoundedMemoryOutput destination( output, outputSize );
    const bool decoded = m_codec->Decode( source, destination, m_params );
    if ( destination.HasOverflowed() )
    {
        error = m_typeName + " data decompresses to more than the " + CORE::ToString( outputSize ) + " bytes its page header announces";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( !decoded )
    {
        error = m_typeName + " data of a page is not valid: " + m_codec->GetLastErrorMessage();
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( destination.GetWritten() != outputSize )
    {
        error = m_typeName + " data decompresses to " + CORE::ToString( destination.GetWritten() ) + " bytes where its page header announces " +
                CORE::ToString( outputSize );
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetCompression::Compress( const UInt8* input, UInt32 inputSize, CORE::CDynamicBuffer& output, CORE::CString& error )
{GUCEF_TRACE;

    output.Clear( true );
    if ( m_codec.IsNULL() )
    {
        error = "No compression codec was prepared";
        return CORE::DATATABLE_STATUS_FAILED;
    }
    CORE::CMFileAccess source( input, inputSize );
    CORE::CDynamicBufferAccess destination( output, CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    if ( !m_codec->Encode( source, destination, m_params ) )
    {
        error = m_typeName + " compression failed: " + m_codec->GetLastErrorMessage();
        return CORE::DATATABLE_STATUS_FAILED;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
