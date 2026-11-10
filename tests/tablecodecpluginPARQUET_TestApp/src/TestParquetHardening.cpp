/*
 *  tablecodecpluginPARQUET_TestApp: tests of the parquet table codec plugin
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
#include <stdlib.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedEncoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H ? */

#include "TestParquetSupport.h"
#include "TestParquetFixtures.h"
#include "TestParquetHardening.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::TABLECODECPLUGIN::PARQUET;
using namespace PARQUETTEST;

namespace {

typedef GUCEF::vector< UInt8 > TBytes;

/**
 *  Reads all of a file: the metadata, every row group, every batch, every cell through Tier 1 and the checks of
 *  Validate(). The status of the first failure, OK when the whole file read
 */
TDataTableStatus
ReadEverything( const UInt8* data, UInt32 size, CString& error, const CValueList* options = GUCEF_NULL )
{
    CDynamicBuffer file;
    if ( size > 0 )
        file.CopyFrom( size, data );
    IOAccessPtr input = MakeReadAccess( file );
    CValueList readOptions;
    if ( GUCEF_NULL != options )
        readOptions = *options;
    CITableFileMetaDataPtr metaData;
    TDataTableStatus status = GetParquetCodec()->ReadMetaData( *input, readOptions, metaData, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CITableReaderPtr reader;
    status = GetParquetCodec()->CreateReader( input, metaData, readOptions, reader, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    for ( UInt32 g=0; g<metaData->GetRowGroupCount(); ++g )
    {
        CDataTableRowGroupInfo info;
        metaData->GetRowGroupInfo( g, info );
        CDataTableReadRequest request;
        request.batchRowCount = 1 + ( g * 977 ) % 3000;
        status = reader->OpenRowGroup( g, request );
        if ( DATATABLE_STATUS_OK != status )
        {
            error = reader->GetLastErrorMessage();
            return status;
        }
        for ( ;; )
        {
            CDataTableBatchPtr batch;
            status = reader->ReadNextBatch( batch );
            if ( DATATABLE_STATUS_END == status )
                break;
            if ( DATATABLE_STATUS_OK != status )
            {
                error = reader->GetLastErrorMessage();
                return status;
            }
            for ( UInt32 c=0; c<batch->GetColumnCount(); ++c )
            {
                CString invalid;
                if ( !batch->GetColumn( c ).Validate( invalid ) )
                {
                    error = "a delivered column batch is not valid: " + invalid;
                    return DATATABLE_STATUS_FAILED;
                }
                CVariant cell;
                for ( UInt32 r=0; r<batch->GetRowCount(); r += 1 + r / 2 )
                    batch->GetValue( r, c, cell, true );
            }
        }
    }
    return DATATABLE_STATUS_OK;
}

TDataTableStatus
ReadEverything( const TBytes& file, CString& error, const CValueList* options = GUCEF_NULL )
{
    return ReadEverything( file.empty() ? GUCEF_NULL : &file[ 0 ], (UInt32) file.size(), error, options );
}

TBytes
ToBytes( const CDynamicBuffer& buffer )
{
    const UInt8* data = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    return TBytes( data, data + buffer.GetDataSize() );
}

/** A valid file of a few columns and row groups, the base of the footer cases and of fuzzing */
TBytes
MakeValidFile( bool nullable, const char* compression, UInt32 rows, UInt32 seed )
{
    const CDataTableSchemaPtr schema = CreateDataTableSchemaPtr( MakeAllTypesSchema( nullable ) );
    CTestRandom random( seed );
    TBatchVector batches;
    batches.push_back( MakeRandomBatch( schema, rows, nullable ? 20 : 0, random, 0 == seed % 2 ? 7 : 0 ) );
    CDataTableWriterOptions options;
    options.compression = compression;
    options.rowGroupRowCount = rows / 2 + 1;
    options.pageRowCount = 64;
    CDynamicBuffer file;
    CString error;
    WriteParquet( schema, batches, options, file, error );
    return ToBytes( file );
}

/** The footer of the file is parsed, changed by the case and written back in place of the old one */
class CFooterEdit
{
    public:

    explicit CFooterEdit( const TBytes& file )
        : m_file( file ), m_valid( false )
    {
        if ( file.size() < 12 )
            return;
        const UInt32 length = LoadLE32( &file[ file.size() - 8 ] );
        m_footerStart = (UInt32) file.size() - 8 - length;
        m_footer.assign( file.begin() + m_footerStart, file.end() - 8 );
        CThriftCompactReader reader( &m_footer[ 0 ], (UInt32) m_footer.size() );
        SParquetParseBudget budget;
        m_valid = CParquetMetaDataSerializer::ReadFileMetaData( reader, meta, budget );
    }

    TBytes Build( void ) const
    {
        CDynamicBuffer footer;
        {
            CThriftCompactWriter writer( footer );
            CParquetMetaDataSerializer::WriteFileMetaData( writer, meta );
        }
        TBytes out( m_file.begin(), m_file.begin() + m_footerStart );
        const UInt8* bytes = static_cast< const UInt8* >( footer.GetConstBufferPtr() );
        out.insert( out.end(), bytes, bytes + footer.GetDataSize() );
        UInt8 tail[ 8 ];
        StoreLE32( tail, footer.GetDataSize() );
        memcpy( tail + 4, "PAR1", 4 );
        out.insert( out.end(), tail, tail + 8 );
        return out;
    }

    inline bool IsValid( void ) const { return m_valid; }

    SParquetFileMetaData meta;

    private:

    TBytes m_file;
    TBytes m_footer;
    UInt32 m_footerStart;
    bool m_valid;
};

/** A page of a crafted file: the header as given (sizes included, so cases can lie) and the bytes after it */
struct SCraftPage
{
    SParquetPageHeader header;
    TBytes payload;
};

typedef GUCEF::vector< SCraftPage > TCraftPageVector;

/**
 *  A file of one column chunk made of the given pages, for cases a valid writer never makes: the footer describes the
 *  column, the row count and the codec; the chunk spans exactly the pages.
 */
TBytes
CraftFile( Int32 physical, bool optional, Int16 logical, const TCraftPageVector& pages, Int64 rows, Int32 codec = PARQUET_COMPRESSION_UNCOMPRESSED )
{
    TBytes file;
    file.push_back( 'P' ); file.push_back( 'A' ); file.push_back( 'R' ); file.push_back( '1' );
    const UInt64 chunkStart = file.size();
    UInt64 dataPageOffset = 0;
    bool hasDictionary = false;
    for ( size_t i=0; i<pages.size(); ++i )
    {
        if ( PARQUET_PAGE_DICTIONARY_PAGE == pages[ i ].header.type && 0 == i )
            hasDictionary = true;
        if ( 0 == dataPageOffset && PARQUET_PAGE_DICTIONARY_PAGE != pages[ i ].header.type )
            dataPageOffset = file.size();
        CDynamicBuffer header;
        {
            CThriftCompactWriter writer( header );
            CParquetMetaDataSerializer::WritePageHeader( writer, pages[ i ].header );
        }
        const UInt8* bytes = static_cast< const UInt8* >( header.GetConstBufferPtr() );
        file.insert( file.end(), bytes, bytes + header.GetDataSize() );
        file.insert( file.end(), pages[ i ].payload.begin(), pages[ i ].payload.end() );
    }
    if ( 0 == dataPageOffset )
        dataPageOffset = chunkStart;
    const UInt64 chunkLength = file.size() - chunkStart;

    static const char name[] = "c";
    static const char root[] = "schema";
    SParquetFileMetaData meta;
    meta.version = 1;
    meta.numRows = rows;
    SParquetSchemaElement rootElement;
    rootElement.name = SParquetBytes( root, 6 );
    rootElement.hasNumChildren = true;
    rootElement.numChildren = 1;
    meta.schema.push_back( rootElement );
    SParquetSchemaElement leaf;
    leaf.name = SParquetBytes( name, 1 );
    leaf.hasType = true;
    leaf.type = physical;
    leaf.hasRepetitionType = true;
    leaf.repetitionType = optional ? PARQUET_REPETITION_OPTIONAL : PARQUET_REPETITION_REQUIRED;
    if ( PARQUET_LOGICAL_NONE != logical )
    {
        leaf.hasLogicalType = true;
        leaf.logicalType.kind = logical;
    }
    meta.schema.push_back( leaf );
    SParquetRowGroup rowGroup;
    rowGroup.numRows = rows;
    rowGroup.totalByteSize = (Int64) chunkLength;
    SParquetColumnChunk chunk;
    chunk.fileOffset = (Int64) chunkStart;
    chunk.hasMetaData = true;
    chunk.metaData.type = physical;
    chunk.metaData.encodings.push_back( PARQUET_ENCODING_PLAIN );
    chunk.metaData.pathInSchema.push_back( SParquetBytes( name, 1 ) );
    chunk.metaData.codec = codec;
    chunk.metaData.numValues = rows;
    chunk.metaData.totalUncompressedSize = (Int64) chunkLength;
    chunk.metaData.totalCompressedSize = (Int64) chunkLength;
    chunk.metaData.dataPageOffset = (Int64) dataPageOffset;
    if ( hasDictionary )
    {
        chunk.metaData.hasDictionaryPageOffset = true;
        chunk.metaData.dictionaryPageOffset = (Int64) chunkStart;
    }
    rowGroup.columns.push_back( chunk );
    meta.rowGroups.push_back( rowGroup );
    CDynamicBuffer footer;
    {
        CThriftCompactWriter writer( footer );
        CParquetMetaDataSerializer::WriteFileMetaData( writer, meta );
    }
    const UInt8* bytes = static_cast< const UInt8* >( footer.GetConstBufferPtr() );
    file.insert( file.end(), bytes, bytes + footer.GetDataSize() );
    UInt8 tail[ 8 ];
    StoreLE32( tail, footer.GetDataSize() );
    memcpy( tail + 4, "PAR1", 4 );
    file.insert( file.end(), tail, tail + 8 );
    return file;
}

/** A consistent data page v1 of PLAIN values; the sizes follow the payload */
SCraftPage
DataPage( const TBytes& payload, Int32 values, Int32 encoding = PARQUET_ENCODING_PLAIN )
{
    SCraftPage page;
    page.header.type = PARQUET_PAGE_DATA_PAGE;
    page.header.uncompressedPageSize = (Int32) payload.size();
    page.header.compressedPageSize = (Int32) payload.size();
    page.header.hasDataPageHeader = true;
    page.header.dataPageHeader.numValues = values;
    page.header.dataPageHeader.encoding = encoding;
    page.header.dataPageHeader.definitionLevelEncoding = PARQUET_ENCODING_RLE;
    page.header.dataPageHeader.repetitionLevelEncoding = PARQUET_ENCODING_RLE;
    page.payload = payload;
    return page;
}

SCraftPage
DictionaryPage( const TBytes& payload, Int32 values )
{
    SCraftPage page;
    page.header.type = PARQUET_PAGE_DICTIONARY_PAGE;
    page.header.uncompressedPageSize = (Int32) payload.size();
    page.header.compressedPageSize = (Int32) payload.size();
    page.header.hasDictionaryPageHeader = true;
    page.header.dictionaryPageHeader.numValues = values;
    page.header.dictionaryPageHeader.encoding = PARQUET_ENCODING_PLAIN;
    page.payload = payload;
    return page;
}

/** n PLAIN Int64 values 0 .. n - 1 */
TBytes
PlainInt64( UInt32 n )
{
    TBytes bytes( (size_t) n * 8 );
    for ( UInt32 i=0; i<n; ++i )
        StoreLE64( &bytes[ (size_t) i * 8 ], i );
    return bytes;
}

/** The hybrid encoding of values, without a length prefix */
TBytes
Hybrid( const GUCEF::vector< UInt32 >& values, UInt32 bitWidth )
{
    CDynamicBuffer out;
    CRleBitPackedEncoder::EncodeValues( &values[ 0 ], (UInt32) values.size(), bitWidth, out );
    const UInt8* bytes = static_cast< const UInt8* >( out.GetConstBufferPtr() );
    return TBytes( bytes, bytes + out.GetDataSize() );
}

void
Append( TBytes& bytes, const TBytes& more )
{
    bytes.insert( bytes.end(), more.begin(), more.end() );
}

void
AppendLE32( TBytes& bytes, UInt32 value )
{
    UInt8 raw[ 4 ];
    StoreLE32( raw, value );
    bytes.insert( bytes.end(), raw, raw + 4 );
}

/** Compresses with gzip through the registered codec */
TBytes
Gzip( const TBytes& data )
{
    CCodecRegistry::TICodecPtr codec = CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CoreCodecTypes::CompressionCodec, "gzip" );
    CDynamicBuffer out;
    CMFileAccess input( data.empty() ? GUCEF_NULL : &data[ 0 ], (UInt32) data.size() );
    CDynamicBufferAccess output( out );
    codec->Encode( input, output );
    const UInt8* bytes = static_cast< const UInt8* >( out.GetConstBufferPtr() );
    return TBytes( bytes, bytes + out.GetDataSize() );
}

/** One malformed file and the status it must give */
void
ExpectStatus( const char* name, const TBytes& file, TDataTableStatus expected, UInt32& failures, const CValueList* options = GUCEF_NULL )
{
    CString error;
    const TDataTableStatus status = ReadEverything( file, error, options );
    if ( status != expected )
    {
        ++failures;
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet hardening: case \"" << name << "\" gave " << DataTableStatusToString( status ) << " (" << error <<
                         ") where " << DataTableStatusToString( expected ) << " was expected" );
    }
    else
    if ( DATATABLE_STATUS_OK != status && error.IsNULLOrEmpty() )
    {
        ++failures;
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Parquet hardening: case \"" << name << "\" failed without a message" );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetHardeningTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Hardening" );

    GUCEF_TESTFW_TESTCASE( "Hardening: the file frame: sizes too small, magics, the encrypted footer, footer lengths outside the file and above the limit" )
    try
    {
        UInt32 failures = 0;
        const TBytes valid = MakeValidFile( true, "none", 300, 1 );
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadEverything( valid, error ) );
        for ( UInt32 size=0; size<12; ++size )
            ExpectStatus( "too small", TBytes( valid.begin(), valid.begin() + size ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        TBytes file = valid;
        file[ file.size() - 1 ] = 'X';
        ExpectStatus( "tail magic", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        file = valid;
        file[ file.size() - 1 ] = 'E';
        ExpectStatus( "encrypted footer", file, DATATABLE_STATUS_UNSUPPORTED, failures );
        file = valid;
        file[ 0 ] = 'Q';
        ExpectStatus( "head magic", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        file = valid;
        StoreLE32( &file[ file.size() - 8 ], 0 );
        ExpectStatus( "footer length 0", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        file = valid;
        StoreLE32( &file[ file.size() - 8 ], (UInt32) file.size() );
        ExpectStatus( "footer longer than the file", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        file = valid;
        StoreLE32( &file[ file.size() - 8 ], 0xFFFFFFFF );
        ExpectStatus( "footer length 2^32 - 1", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        CValueList smallFooter;
        smallFooter.Set( "maxFooterBytes", CVariant( "64" ) );
        ExpectStatus( "footer above maxFooterBytes", valid, DATATABLE_STATUS_LIMIT_EXCEEDED, failures, &smallFooter );
        file = valid;
        const UInt32 footerLength = LoadLE32( &file[ file.size() - 8 ] );
        for ( UInt32 i=0; i<footerLength; ++i )
            file[ file.size() - 8 - footerLength + i ] = 0xAB;
        ExpectStatus( "footer of junk", file, DATATABLE_STATUS_CORRUPT_DATA, failures );
        ASSERT_TRUE( 0 == failures );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hardening: footers that lie: row counts, column counts, chunk offsets and sizes outside the file or overlapping, wrong types, other files, unknown codecs, schemas" )
    try
    {
        UInt32 failures = 0;
        const TBytes valid = MakeValidFile( false, "none", 400, 2 );
        CFooterEdit base( valid );
        ASSERT_TRUE( base.IsValid() && 2 == base.meta.rowGroups.size() );
        if ( base.IsValid() && 2 == base.meta.rowGroups.size() )
        {
            ExpectStatus( "rewritten footer", base.Build(), DATATABLE_STATUS_OK, failures );
            CFooterEdit edit( valid );
            edit.meta.numRows += 1;
            ExpectStatus( "row count of the file", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 1 ].columns.pop_back();
            ExpectStatus( "a column chunk missing", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.dataPageOffset = (Int64) valid.size() + 100;
            ExpectStatus( "data page beyond the file", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.totalCompressedSize = 1LL << 40;
            ExpectStatus( "chunk of 2^40 bytes", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.dataPageOffset = 1;
            ExpectStatus( "chunk inside the magic", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 4 ].metaData.dataPageOffset = edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.dataPageOffset + 8;
            ExpectStatus( "overlapping chunks", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.totalCompressedSize = -5;
            ExpectStatus( "negative chunk size", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].numRows = -1;
            edit.meta.numRows = edit.meta.rowGroups[ 1 ].numRows - 1;
            ExpectStatus( "negative row group rows", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].numRows += 1000000000;
            edit.meta.numRows += 1000000000;
            ExpectStatus( "a billion rows the pages do not hold", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.type = PARQUET_TYPE_DOUBLE;
            ExpectStatus( "chunk of another physical type", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].hasFilePath = true;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].filePath = SParquetBytes( "other.parquet", 13 );
            ExpectStatus( "chunk in another file", edit.Build(), DATATABLE_STATUS_UNSUPPORTED, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.codec = 99;
            ExpectStatus( "unknown codec", edit.Build(), DATATABLE_STATUS_UNSUPPORTED, failures );
            edit = base;
            edit.meta.rowGroups[ 0 ].columns[ 3 ].metaData.codec = PARQUET_COMPRESSION_LZ4;
            ExpectStatus( "hadoop LZ4", edit.Build(), DATATABLE_STATUS_UNSUPPORTED, failures );
            edit = base;
            edit.meta.schema[ 0 ].numChildren = 1000;
            ExpectStatus( "root with too many children", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.schema[ 0 ].numChildren -= 1;
            ExpectStatus( "schema elements outside every group", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.schema[ 0 ].hasType = true;
            ExpectStatus( "root that is a leaf", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.schema[ 4 ].repetitionType = 7;
            ExpectStatus( "unknown repetition", edit.Build(), DATATABLE_STATUS_CORRUPT_DATA, failures );
            edit = base;
            edit.meta.schema[ 4 ].type = 42;
            {
                // the narrowest level: that column is listed as UNSUPPORTED, the others read
                const TBytes file = edit.Build();
                ExpectStatus( "unknown physical type", file, DATATABLE_STATUS_OK, failures );
                CDynamicBuffer buffer;
                buffer.CopyFrom( (UInt32) file.size(), &file[ 0 ] );
                IOAccessPtr input = MakeReadAccess( buffer );
                CITableFileMetaDataPtr metaData;
                CString error;
                ASSERT_TRUE( DATATABLE_STATUS_OK == GetParquetCodec()->ReadMetaData( *input, CValueList(), metaData, error ) );
                ASSERT_TRUE( !metaData.IsNULL() && DATATABLE_TYPE_UNSUPPORTED == metaData->GetSchema()->GetColumn( 3 ).type );
                TBatchVector read;
                CDataTableSchema::TColumnIndexVector projection;
                projection.push_back( 3 );
                ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == ReadParquet( buffer, read, error, &projection ) && error.HasSubstr( "physical type 42" ) >= 0 );
            }
            edit = base;
            edit.meta.hasEncryptionAlgorithm = true;
            {
                // the field is written by hand: the structure has no writer for it
                TBytes file = edit.Build();
                ExpectStatus( "plain footer of an encrypted file (field written as nothing)", file, DATATABLE_STATUS_OK, failures );
            }

            // groups nested deeper than the schema limit
            edit = base;
            static const char groupName[] = "g";
            SParquetSchemaElement group;
            group.name = SParquetBytes( groupName, 1 );
            group.hasNumChildren = true;
            group.numChildren = 1;
            group.hasRepetitionType = true;
            group.repetitionType = PARQUET_REPETITION_OPTIONAL;
            TParquetSchemaElementVector deep;
            deep.push_back( edit.meta.schema[ 0 ] );
            deep[ 0 ].numChildren = 1;
            for ( UInt32 i=0; i<40; ++i )
                deep.push_back( group );
            deep.push_back( edit.meta.schema[ 1 ] );
            edit.meta.schema = deep;
            ExpectStatus( "groups nested 40 deep", edit.Build(), DATATABLE_STATUS_LIMIT_EXCEEDED, failures );
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hardening: pages that lie: sizes beyond the chunk and the limits, value counts beyond the rows, short values, levels, dictionaries, byte arrays, compression bombs" )
    try
    {
        UInt32 failures = 0;
        TCraftPageVector pages;

        // the crafted base reads
        pages.push_back( DataPage( PlainInt64( 10 ), 10 ) );
        ExpectStatus( "crafted base", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_OK, failures );

        pages[ 0 ].header.compressedPageSize = 1000;
        ExpectStatus( "page beyond its chunk", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 10 );
        pages[ 0 ].header.uncompressedPageSize = 70;
        ExpectStatus( "uncompressed sizes that differ", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 11 );
        ExpectStatus( "more values than rows", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        ExpectStatus( "values shorter than the count", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 11 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 10 );
        ExpectStatus( "fewer rows in the pages than the row group", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 20 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ].header.dataPageHeader.numValues = -3;
        ExpectStatus( "negative value count", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 10, PARQUET_ENCODING_DELTA_BINARY_PACKED );
        ExpectStatus( "DELTA_BINARY_PACKED", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_UNSUPPORTED, failures );
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 10, PARQUET_ENCODING_BYTE_STREAM_SPLIT );
        ExpectStatus( "BYTE_STREAM_SPLIT", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_UNSUPPORTED, failures );
        CValueList smallPages;
        smallPages.Set( "maxPageBytes", CVariant( "1024" ) );
        pages[ 0 ] = DataPage( PlainInt64( 200 ), 200 );
        ExpectStatus( "page above maxPageBytes", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 200 ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures, &smallPages );
        CValueList smallChunks;
        smallChunks.Set( "maxChunkBytes", CVariant( "1024" ) );
        ExpectStatus( "chunk above maxChunkBytes", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 200 ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures, &smallChunks );

        // a page header nested deep inside its own unknown field
        pages[ 0 ] = DataPage( PlainInt64( 10 ), 10 );
        {
            TBytes file = CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 );
            // the header starts at byte 4; its first field becomes an unknown struct nested 100 deep
            TBytes nested( file.begin(), file.begin() + 4 );
            for ( UInt32 i=0; i<100; ++i ) nested.push_back( 0x1C );
            ExpectStatus( "deeply nested page header", nested, DATATABLE_STATUS_CORRUPT_DATA, failures );
        }

        // definition levels: their length beyond the page, the old bit packed encoding, and truncated runs
        TBytes levels;
        GUCEF::vector< UInt32 > valid( 10, 1 );
        valid[ 3 ] = 0;
        const TBytes levelRuns = Hybrid( valid, 1 );
        AppendLE32( levels, (UInt32) levelRuns.size() );
        Append( levels, levelRuns );
        TBytes optionalPage = levels;
        Append( optionalPage, PlainInt64( 9 ) );
        pages[ 0 ] = DataPage( optionalPage, 10 );
        ExpectStatus( "optional base", CraftFile( PARQUET_TYPE_INT64, true, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_OK, failures );
        TBytes badLength = optionalPage;
        StoreLE32( &badLength[ 0 ], 100000 );
        pages[ 0 ] = DataPage( badLength, 10 );
        ExpectStatus( "levels longer than the page", CraftFile( PARQUET_TYPE_INT64, true, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DataPage( optionalPage, 10 );
        pages[ 0 ].header.dataPageHeader.definitionLevelEncoding = PARQUET_ENCODING_BIT_PACKED;
        ExpectStatus( "BIT_PACKED levels", CraftFile( PARQUET_TYPE_INT64, true, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_UNSUPPORTED, failures );
        TBytes fewLevels;
        AppendLE32( fewLevels, 2 );
        fewLevels.push_back( 0x08 );        // an RLE run of 4 ones, for 10 values
        fewLevels.push_back( 0x01 );
        Append( fewLevels, PlainInt64( 10 ) );
        pages[ 0 ] = DataPage( fewLevels, 10 );
        ExpectStatus( "levels for fewer values", CraftFile( PARQUET_TYPE_INT64, true, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );

        // dictionaries
        TBytes indices;
        indices.push_back( 2 );             // bit width 2
        GUCEF::vector< UInt32 > index( 10, 1 );
        Append( indices, Hybrid( index, 2 ) );
        pages.clear();
        pages.push_back( DictionaryPage( PlainInt64( 3 ), 3 ) );
        pages.push_back( DataPage( indices, 10, PARQUET_ENCODING_RLE_DICTIONARY ) );
        ExpectStatus( "dictionary base", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_OK, failures );
        index[ 7 ] = 3;
        TBytes outOfRange;
        outOfRange.push_back( 2 );
        Append( outOfRange, Hybrid( index, 2 ) );
        pages[ 1 ] = DataPage( outOfRange, 10, PARQUET_ENCODING_RLE_DICTIONARY );
        ExpectStatus( "dictionary index out of range", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        TBytes wide = indices;
        wide[ 0 ] = 33;
        pages[ 1 ] = DataPage( wide, 10, PARQUET_ENCODING_RLE_DICTIONARY );
        ExpectStatus( "index bit width 33", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 1 ] = DataPage( indices, 10, PARQUET_ENCODING_RLE_DICTIONARY );
        pages[ 0 ].header.dictionaryPageHeader.numValues = 1000000;
        ExpectStatus( "dictionary larger than its page", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        pages[ 0 ] = DictionaryPage( PlainInt64( 3 ), 3 );
        TCraftPageVector noDictionary;
        noDictionary.push_back( DataPage( indices, 10, PARQUET_ENCODING_RLE_DICTIONARY ) );
        ExpectStatus( "indices without a dictionary", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, noDictionary, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        TCraftPageVector twoDictionaries = pages;
        twoDictionaries.insert( twoDictionaries.begin() + 1, DictionaryPage( PlainInt64( 3 ), 3 ) );
        ExpectStatus( "a second dictionary", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, twoDictionaries, 10 ), DATATABLE_STATUS_CORRUPT_DATA, failures );

        // byte arrays whose length runs past the page, or is negative as a signed number
        TBytes text;
        AppendLE32( text, 3 ); text.push_back( 'a' ); text.push_back( 'b' ); text.push_back( 'c' );
        AppendLE32( text, 1000 ); text.push_back( 'x' );
        pages.clear();
        pages.push_back( DataPage( text, 2 ) );
        ExpectStatus( "byte array past its page", CraftFile( PARQUET_TYPE_BYTE_ARRAY, false, PARQUET_LOGICAL_STRING, pages, 2 ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        StoreLE32( &text[ 7 ], 0xFFFFFFFF );
        pages[ 0 ] = DataPage( text, 2 );
        ExpectStatus( "byte array of length 2^32 - 1", CraftFile( PARQUET_TYPE_BYTE_ARRAY, false, PARQUET_LOGICAL_STRING, pages, 2 ), DATATABLE_STATUS_CORRUPT_DATA, failures );

        // RLE booleans announcing two billion values in a few bytes: the decoded page passes maxPageBytes
        TBytes booleans;
        GUCEF::vector< UInt32 > one( 8, 1 );
        CDynamicBuffer run;
        CRleBitPackedEncoder::EncodeRepeatedBit( true, 0x7FFFFFFF, run );
        AppendLE32( booleans, run.GetDataSize() );
        booleans.insert( booleans.end(), static_cast< const UInt8* >( run.GetConstBufferPtr() ), static_cast< const UInt8* >( run.GetConstBufferPtr() ) + run.GetDataSize() );
        pages[ 0 ] = DataPage( booleans, 0x7FFFFFFF, PARQUET_ENCODING_RLE );
        ExpectStatus( "two billion RLE booleans", CraftFile( PARQUET_TYPE_BOOLEAN, false, PARQUET_LOGICAL_NONE, pages, 0x7FFFFFFF ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures );
        // and as many all-null rows of an optional column in a level run of a few bytes
        TBytes nullLevels;
        CDynamicBuffer zeros;
        CRleBitPackedEncoder::EncodeRepeatedBit( false, 0x7FFFFFFF, zeros );
        AppendLE32( nullLevels, zeros.GetDataSize() );
        nullLevels.insert( nullLevels.end(), static_cast< const UInt8* >( zeros.GetConstBufferPtr() ), static_cast< const UInt8* >( zeros.GetConstBufferPtr() ) + zeros.GetDataSize() );
        pages[ 0 ] = DataPage( nullLevels, 0x7FFFFFFF );
        ExpectStatus( "two billion null levels", CraftFile( PARQUET_TYPE_INT64, true, PARQUET_LOGICAL_NONE, pages, 0x7FFFFFFF ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures );
        // and as many indices of bit width 0 into a dictionary of one value
        TBytes zeroWidth;
        zeroWidth.push_back( 0 );
        CDynamicBuffer zeroIndices;
        CRleBitPackedEncoder::WriteVarint( (UInt64) 0x7FFFFFFF << 1, zeroIndices );
        zeroWidth.insert( zeroWidth.end(), static_cast< const UInt8* >( zeroIndices.GetConstBufferPtr() ), static_cast< const UInt8* >( zeroIndices.GetConstBufferPtr() ) + zeroIndices.GetDataSize() );
        pages.clear();
        pages.push_back( DictionaryPage( PlainInt64( 1 ), 1 ) );
        pages.push_back( DataPage( zeroWidth, 0x7FFFFFFF, PARQUET_ENCODING_RLE_DICTIONARY ) );
        ExpectStatus( "two billion indices of bit width 0", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 0x7FFFFFFF ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures );

        // compression: data that inflates beyond the announced size, a size no deflate stream reaches, broken data
        if ( IsGzipAvailable() )
        {
            const TBytes plain = PlainInt64( 1000 );
            const TBytes compressed = Gzip( plain );
            pages.clear();
            pages.push_back( DataPage( compressed, 1000 ) );
            pages[ 0 ].header.uncompressedPageSize = (Int32) plain.size();
            ExpectStatus( "gzip base", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 1000, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_OK, failures );
            pages[ 0 ].header.uncompressedPageSize = 4000;
            ExpectStatus( "inflates beyond its size", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 1000, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_CORRUPT_DATA, failures );
            pages[ 0 ].header.uncompressedPageSize = 9000;
            ExpectStatus( "inflates short of its size", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 1000, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_CORRUPT_DATA, failures );
            pages[ 0 ].header.uncompressedPageSize = 200 * 1024 * 1024;
            ExpectStatus( "an expansion deflate cannot make", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 1000, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_CORRUPT_DATA, failures );
            // a real bomb: 64 MiB of zeros compress to some 64 KiB; announced honestly it is bounded by maxPageBytes
            const TBytes bombPlain( 64 * 1024 * 1024, 0 );
            const TBytes bomb = Gzip( bombPlain );
            pages[ 0 ] = DataPage( bomb, 8 * 1024 * 1024 );
            pages[ 0 ].header.uncompressedPageSize = (Int32) bombPlain.size();
            CValueList bounded;
            bounded.Set( "maxPageBytes", CVariant( "16777216" ) );
            ExpectStatus( "gzip bomb under maxPageBytes", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 8 * 1024 * 1024, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_LIMIT_EXCEEDED, failures, &bounded );
            // and announced small it is cut off at the announced size
            pages[ 0 ].header.uncompressedPageSize = 65536;
            ExpectStatus( "gzip bomb announced small", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 8192, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_CORRUPT_DATA, failures );
            TBytes broken = compressed;
            for ( size_t i=10; i<broken.size(); i += 7 ) broken[ i ] ^= 0x5A;
            pages[ 0 ] = DataPage( broken, 1000 );
            pages[ 0 ].header.uncompressedPageSize = (Int32) plain.size();
            ExpectStatus( "broken gzip data", CraftFile( PARQUET_TYPE_INT64, false, PARQUET_LOGICAL_NONE, pages, 1000, PARQUET_COMPRESSION_GZIP ), DATATABLE_STATUS_CORRUPT_DATA, failures );
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hardening: a table schema key that is not strict JSON (truncated literals, numbers, strings, nesting) is ignored and never reaches the JSON parser" )
    try
    {
        UInt32 failures = 0;
        const TBytes valid = MakeValidFile( false, "none", 400, 2 );
        static const char* const hostile[] = { "[fals", "[tru", "[nul", "{\"a\":fals", "[1,tr", "[1,]", "{\"a\":1,}", "[\"\\u00", "[1e", "[-", "[01]",
                                               "nul", "t", "", " ", "[", "{", "{\"a\"", "[\"a\\x\"]", "[1] x", "[\"tab\there\"]",
                                               "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[" };
        for ( UInt32 h=0; h<sizeof( hostile ) / sizeof( hostile[ 0 ] ); ++h )
        {
            CFooterEdit edit( valid );
            ASSERT_TRUE( edit.IsValid() );
            if ( !edit.IsValid() )
                break;
            bool replaced = false;
            for ( size_t k=0; k<edit.meta.keyValueMetaData.size(); ++k )
            {
                SParquetKeyValue& kv = edit.meta.keyValueMetaData[ k ];
                if ( kv.key.Equals( CParquetFileMetaData::TableSchemaKey ) )
                {
                    kv.hasValue = true;
                    kv.value = SParquetBytes( hostile[ h ], (UInt32) strlen( hostile[ h ] ) );
                    replaced = true;
                }
            }
            ASSERT_TRUE( replaced );
            // read from a buffer of exactly the file's bytes: under ASan a read past the key's text is reported
            ExpectStatus( hostile[ h ], edit.Build(), DATATABLE_STATUS_OK, failures );
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hardening: every truncation of small files and seeded bit flips, byte replacements and truncations of valid files give a status and never crash" )
    try
    {
        GUCEF::vector< TBytes > bases;
        bases.push_back( MakeValidFile( true, "none", 600, 3 ) );
        bases.push_back( MakeValidFile( false, "none", 600, 4 ) );
        if ( IsGzipAvailable() )
            bases.push_back( MakeValidFile( true, "gzip", 600, 5 ) );
        for ( UInt32 f=0; f<ParquetFixtureCount; ++f )
        {
            if ( 0 == strcmp( ParquetFixtures[ f ].expectedStatus, "OK" ) && ParquetFixtures[ f ].size > 120 )
                bases.push_back( TBytes( ParquetFixtures[ f ].bytes, ParquetFixtures[ f ].bytes + ParquetFixtures[ f ].size ) );
        }

        UInt32 reads = 0;
        UInt32 succeeded = 0;
        CString error;

        // every truncation of the small fixtures
        for ( size_t b=3; b<bases.size(); ++b )
        {
            for ( size_t cut=0; cut<bases[ b ].size(); ++cut )
            {
                if ( DATATABLE_STATUS_OK == ReadEverything( TBytes( bases[ b ].begin(), bases[ b ].begin() + cut ), error ) )
                    ++succeeded;
                ++reads;
            }
        }
        ASSERT_TRUE( 0 == succeeded );

        // mutations with fixed seeds
        const char* const rounds = getenv( "GUCEF_TEST_PARQUET_FUZZ_ROUNDS" );
        const UInt32 roundCount = GUCEF_NULL != rounds ? (UInt32) atoi( rounds ) : 600;
        const UInt8 interesting[] = { 0x00, 0xFF, 0x7F, 0x80, 0x01, 0xFE, 0x15, 0x19, 0x1C };
        for ( size_t b=0; b<bases.size(); ++b )
        {
            const TBytes& base = bases[ b ];
            if ( base.size() < 16 )
                continue;
            const UInt32 footerLength = LoadLE32( &base[ base.size() - 8 ] );
            const UInt32 footerStart = (UInt32) ( base.size() - 8 - ( footerLength < base.size() - 8 ? footerLength : 0 ) );
            CTestRandom random( 1000 + b );
            for ( UInt32 round=0; round<roundCount; ++round )
            {
                TBytes file = base;
                const UInt32 kind = random.Below( 4 );
                const UInt32 edits = 1 + random.Below( 8 );
                for ( UInt32 e=0; e<edits; ++e )
                {
                    // half of the edits go to the footer, where one byte changes the most
                    const UInt32 position = random.Below( 2 ) ? footerStart + random.Below( footerLength ) : random.Below( (UInt32) file.size() );
                    if ( position >= file.size() )
                        continue;
                    switch ( kind )
                    {
                        case 0: { file[ position ] ^= (UInt8) ( 1u << random.Below( 8 ) ); break; }
                        case 1: { file[ position ] = interesting[ random.Below( sizeof( interesting ) ) ]; break; }
                        case 2: { file[ position ] = (UInt8) random.Next(); break; }
                        default: { if ( position > 0 ) file.resize( position ); break; }
                    }
                }
                if ( DATATABLE_STATUS_OK == ReadEverything( file, error ) )
                    ++succeeded;
                ++reads;
            }
        }
        GUCEF_LOG( LOGLEVEL_NORMAL, "Parquet fuzzing: " << reads << " damaged files read, " << succeeded << " of them still read whole" );
        ASSERT_TRUE( reads > 2000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
