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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#include "tablecodecpluginPARQUET_ParquetMetaData.h"
#define TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H ? */

#include "TestParquetSupport.h"
#include "TestParquetThrift.h"

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

namespace {

/** Whether the buffer holds exactly the bytes */
bool
HasBytes( const CDynamicBuffer& buffer, const UInt8* bytes, UInt32 count )
{
    return buffer.GetDataSize() == count && ( 0 == count || 0 == memcmp( buffer.GetConstBufferPtr(), bytes, count ) );
}

const UInt8*
Bytes( const CDynamicBuffer& buffer )
{
    return static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
}

/** A struct nested depth times inside field 1 of each other */
void
WriteNested( CDynamicBuffer& output, UInt32 depth )
{
    // written by hand: the writer itself stops at its supported depth
    const UInt8 field = 0x1C;   // delta 1, STRUCT
    for ( UInt32 i=0; i<depth; ++i )
        output.Append( &field, 1 );
    const UInt8 stop = 0;
    for ( UInt32 i=0; i<=depth; ++i )
        output.Append( &stop, 1 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetThriftTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Thrift" );

    GUCEF_TESTFW_TESTCASE( "Thrift: varints and zigzag integers have the bytes of the compact protocol specification" )
    try
    {
        // the example of the specification: 50399 is DF 89 03
        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            writer.WriteVarint( 50399 );
        }
        const UInt8 golden50399[] = { 0xDF, 0x89, 0x03 };
        ASSERT_TRUE( HasBytes( out, golden50399, 3 ) );

        // zigzag: 0 -> 0, -1 -> 1, 1 -> 2, -2 -> 3, then the extremes
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            writer.WriteI32( 0 ); writer.WriteI32( -1 ); writer.WriteI32( 1 ); writer.WriteI32( -2 );
            writer.WriteI32( 2147483647 ); writer.WriteI32( (Int32) 0x80000000 );
            writer.WriteI64( (Int64) 0x8000000000000000ULL ); writer.WriteI16( -32768 ); writer.WriteI8( -1 );
        }
        const UInt8 goldenZigzag[] = { 0x00, 0x01, 0x02, 0x03,
                                       0xFE, 0xFF, 0xFF, 0xFF, 0x0F,                                    // 2^31 - 1 -> 2^32 - 2
                                       0xFF, 0xFF, 0xFF, 0xFF, 0x0F,                                    // -2^31 -> 2^32 - 1
                                       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,     // -2^63 -> 2^64 - 1: 10 bytes
                                       0xFF, 0xFF, 0x03,                                                // -32768 -> 65535
                                       0xFF };                                                          // i8 is one byte
        ASSERT_TRUE( HasBytes( out, goldenZigzag, sizeof( goldenZigzag ) ) );

        CThriftCompactReader reader( Bytes( out ), out.GetDataSize() );
        Int32 i32 = 99; Int64 i64 = 0; Int16 i16 = 0; Int8 i8 = 0;
        ASSERT_TRUE( reader.ReadI32( i32 ) && 0 == i32 );
        ASSERT_TRUE( reader.ReadI32( i32 ) && -1 == i32 );
        ASSERT_TRUE( reader.ReadI32( i32 ) && 1 == i32 );
        ASSERT_TRUE( reader.ReadI32( i32 ) && -2 == i32 );
        ASSERT_TRUE( reader.ReadI32( i32 ) && 2147483647 == i32 );
        ASSERT_TRUE( reader.ReadI32( i32 ) && (Int32) 0x80000000 == i32 );
        ASSERT_TRUE( reader.ReadI64( i64 ) && (Int64) 0x8000000000000000ULL == i64 );
        ASSERT_TRUE( reader.ReadI16( i16 ) && -32768 == i16 );
        ASSERT_TRUE( reader.ReadI8( i8 ) && -1 == i8 );
        ASSERT_TRUE( 0 == reader.GetRemaining() && !reader.HasFailed() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thrift: field headers, bools, doubles, binary, lists and maps have the bytes of the specification" )
    try
    {
        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 1, 7 );                   // delta 1, I32: 0x15, then 7 zigzagged: 0x0E
            writer.WriteBoolField( 2, true );               // delta 1, BOOLEAN_TRUE: 0x11
            writer.WriteBoolField( 3, false );              // delta 1, BOOLEAN_FALSE: 0x12
            writer.WriteDoubleField( 4, 1.0 );              // 0x17, 8 bytes little endian
            writer.WriteBinaryField( 19, "abc", 3 );        // delta 15: 0xF8, length 3, the bytes
            writer.WriteI64Field( 40, -1 );                 // delta 21: long form 0x06, zigzag id 80 = 0x50, value 0x01
            writer.WriteI16Field( 2, 5 );                   // backwards: long form 0x04, zigzag id 4, value 0x0A
            writer.BeginListField( 3, THRIFT_COMPACT_I32, 3 );  // delta 1: 0x19, list header 0x35
            writer.WriteI32( 1 ); writer.WriteI32( 2 ); writer.WriteI32( 3 );
            writer.BeginListField( 4, THRIFT_COMPACT_BOOL_TRUE, 15 );   // 0x19, long list header 0xF1 0x0F
            for ( UInt32 i=0; i<15; ++i )
                writer.WriteBoolElement( 0 == i % 2 );      // elements: 1 true, 2 false
            writer.EndStruct();                             // stop: 0x00
        }
        const UInt8 golden[] =
        {
            0x15, 0x0E,
            0x11,
            0x12,
            0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F,
            0xF8, 0x03, 'a', 'b', 'c',
            0x06, 0x50, 0x01,
            0x04, 0x04, 0x0A,
            0x19, 0x35, 0x02, 0x04, 0x06,
            0x19, 0xF1, 0x0F, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1,
            0x00
        };
        ASSERT_TRUE( HasBytes( out, golden, sizeof( golden ) ) );

        CThriftCompactReader reader( Bytes( out ), out.GetDataSize() );
        UInt8 type = 0;
        Int16 id = 0;
        ASSERT_TRUE( reader.BeginStruct() );
        Int32 i32 = 0; bool flag = false; Float64 dbl = 0; const UInt8* bytes = GUCEF_NULL; UInt32 length = 0; Int64 i64 = 0; Int16 i16 = 0;
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && THRIFT_COMPACT_I32 == type && 1 == id && reader.ReadI32( i32 ) && 7 == i32 );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 2 == id && reader.ReadBoolField( type, flag ) && flag );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 3 == id && reader.ReadBoolField( type, flag ) && !flag );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 4 == id && reader.ReadDouble( dbl ) && 1.0 == dbl );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 19 == id && reader.ReadBinary( bytes, length ) && 3 == length && 0 == memcmp( bytes, "abc", 3 ) );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 40 == id && THRIFT_COMPACT_I64 == type && reader.ReadI64( i64 ) && -1 == i64 );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 2 == id && THRIFT_COMPACT_I16 == type && reader.ReadI16( i16 ) && 5 == i16 );
        UInt8 elementType = 0;
        UInt32 size = 0;
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 3 == id && reader.ReadListHeader( elementType, size ) && THRIFT_COMPACT_I32 == elementType && 3 == size );
        for ( UInt32 i=1; i<=3; ++i )
            ASSERT_TRUE( reader.ReadI32( i32 ) && (Int32) i == i32 );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && 4 == id && reader.ReadListHeader( elementType, size ) && 15 == size );
        for ( UInt32 i=0; i<15; ++i )
            ASSERT_TRUE( reader.ReadBoolElement( flag ) && flag == ( 0 == i % 2 ) );
        ASSERT_TRUE( reader.ReadFieldHeader( type, id ) && THRIFT_COMPACT_STOP == type );
        ASSERT_TRUE( reader.EndStruct() && 0 == reader.GetRemaining() );

        // maps: empty is one zero byte; otherwise the size, then the key and value types in one byte
        const UInt8 emptyMap[] = { 0x00 };
        CThriftCompactReader mapReader( emptyMap, 1 );
        UInt8 keyType = 9, valueType = 9;
        ASSERT_TRUE( mapReader.ReadMapHeader( keyType, valueType, size ) && 0 == size && 0 == keyType );
        const UInt8 oneMap[] = { 0x01, 0x58, 0x02, 0x01, 'x' };    // one entry, I32 -> BINARY: 1 -> "x"
        CThriftCompactReader mapReader2( oneMap, sizeof( oneMap ) );
        ASSERT_TRUE( mapReader2.ReadMapHeader( keyType, valueType, size ) && 1 == size && THRIFT_COMPACT_I32 == keyType && THRIFT_COMPACT_BINARY == valueType );
        ASSERT_TRUE( mapReader2.ReadI32( i32 ) && 1 == i32 && mapReader2.ReadBinary( bytes, length ) && 1 == length && 'x' == bytes[ 0 ] );

        // the writer can make the long form on request: a byte more per field, read the same
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            writer.SetLongFormFieldCount( 2 );
            writer.BeginStruct();
            writer.WriteI32Field( 1, 7 );
            writer.WriteI32Field( 2, 8 );
            writer.WriteI32Field( 3, 9 );
            writer.EndStruct();
        }
        const UInt8 longForm[] = { 0x05, 0x02, 0x0E, 0x05, 0x04, 0x10, 0x15, 0x12, 0x00 };
        ASSERT_TRUE( HasBytes( out, longForm, sizeof( longForm ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thrift: unknown fields of every type are skipped, nested values included, and the next known field is read" )
    try
    {
        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 100, 1 );
            writer.WriteBoolField( 101, true );
            writer.WriteI8Field( 102, 5 );
            writer.WriteI16Field( 103, 6 );
            writer.WriteI64Field( 104, 1LL << 40 );
            writer.WriteDoubleField( 105, 2.5 );
            writer.WriteBinaryField( 106, "skip me", 7 );
            writer.BeginListField( 107, THRIFT_COMPACT_STRUCT, 2 );
            writer.BeginStruct(); writer.WriteI32Field( 1, 1 ); writer.BeginListField( 2, THRIFT_COMPACT_BINARY, 1 ); writer.WriteBinary( "z", 1 ); writer.EndStruct();
            writer.BeginStruct(); writer.EndStruct();
            writer.WriteFieldHeader( THRIFT_COMPACT_SET, 108 );
            writer.WriteListHeader( THRIFT_COMPACT_I64, 2 ); writer.WriteI64( 1 ); writer.WriteI64( 2 );
            writer.WriteFieldHeader( THRIFT_COMPACT_MAP, 109 );
            writer.WriteVarint( 2 ); const UInt8 types = 0x8C; writer.WriteRaw( &types, 1 );    // BINARY -> STRUCT
            writer.WriteBinary( "a", 1 ); writer.BeginStruct(); writer.WriteBoolField( 1, false ); writer.EndStruct();
            writer.WriteBinary( "b", 1 ); writer.BeginStruct(); writer.EndStruct();
            writer.WriteFieldHeader( THRIFT_COMPACT_UUID, 110 );
            const UInt8 uuid[ 16 ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
            writer.WriteRaw( uuid, 16 );
            writer.BeginStructField( 111 ); writer.BeginStructField( 1 ); writer.WriteI32Field( 1, 3 ); writer.EndStruct(); writer.EndStruct();
            writer.WriteI32Field( 200, 4242 );
            writer.EndStruct();
        }
        CThriftCompactReader reader( Bytes( out ), out.GetDataSize() );
        ASSERT_TRUE( reader.BeginStruct() );
        Int32 known = 0;
        for ( ;; )
        {
            UInt8 type = 0;
            Int16 id = 0;
            if ( !reader.ReadFieldHeader( type, id ) || THRIFT_COMPACT_STOP == type )
                break;
            if ( 200 == id )
                ASSERT_TRUE( reader.ReadI32( known ) );
            else
                ASSERT_TRUE( reader.SkipField( type ) );
        }
        ASSERT_TRUE( !reader.HasFailed() && reader.EndStruct() && 4242 == known && 0 == reader.GetRemaining() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thrift: hostile input fails with a status: deep nesting, overlong varints, values beyond their type, sizes beyond the data, unknown types, truncation" )
    try
    {
        // nesting: 31 levels inside the root are fine, 32 are not at the default depth of 32
        CDynamicBuffer nested;
        WriteNested( nested, 31 );
        CThriftCompactReader fine( Bytes( nested ), nested.GetDataSize() );
        ASSERT_TRUE( fine.Skip( THRIFT_COMPACT_STRUCT ) && 0 == fine.GetRemaining() );
        nested.Clear( true );
        WriteNested( nested, 40 );
        CThriftCompactReader deep( Bytes( nested ), nested.GetDataSize() );
        ASSERT_FALSE( deep.Skip( THRIFT_COMPACT_STRUCT ) );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == deep.GetStatus() && !deep.GetErrorMessage().IsNULLOrEmpty() );
        nested.Clear( true );
        WriteNested( nested, 100000 );
        CThriftCompactReader veryDeep( Bytes( nested ), nested.GetDataSize() );
        ASSERT_FALSE( veryDeep.Skip( THRIFT_COMPACT_STRUCT ) );

        // lists of lists nest as well
        GUCEF::vector< UInt8 > lists( 200, 0x19 );     // one element of type LIST, 200 times
        lists.push_back( 0x00 );
        CThriftCompactReader listDeep( &lists[ 0 ], (UInt32) lists.size() );
        ASSERT_FALSE( listDeep.Skip( THRIFT_COMPACT_LIST ) );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == listDeep.GetStatus() );

        const UInt8 elevenBytes[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01 };
        UInt64 u64 = 0;
        CThriftCompactReader overlong( elevenBytes, sizeof( elevenBytes ) );
        ASSERT_FALSE( overlong.ReadVarint( u64 ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == overlong.GetStatus() );
        const UInt8 tenthTooBig[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02 };
        CThriftCompactReader beyond64( tenthTooBig, sizeof( tenthTooBig ) );
        ASSERT_FALSE( beyond64.ReadVarint( u64 ) );

        const UInt8 bigI32[] = { 0x80, 0x80, 0x80, 0x80, 0x10 };      // 2^32: beyond i32
        Int32 i32 = 0;
        CThriftCompactReader overI32( bigI32, sizeof( bigI32 ) );
        ASSERT_FALSE( overI32.ReadI32( i32 ) );
        const UInt8 bigI16[] = { 0x80, 0x80, 0x04 };                  // 65536: beyond i16
        Int16 i16 = 0;
        CThriftCompactReader overI16( bigI16, sizeof( bigI16 ) );
        ASSERT_FALSE( overI16.ReadI16( i16 ) );

        const UInt8 longBinary[] = { 0x10, 'a', 'b' };                 // 16 bytes announced, 2 present
        const UInt8* bytes = GUCEF_NULL;
        UInt32 length = 0;
        CThriftCompactReader shortBinary( longBinary, sizeof( longBinary ) );
        ASSERT_FALSE( shortBinary.ReadBinary( bytes, length ) );
        ASSERT_TRUE( GUCEF_NULL == bytes && 0 == length );

        const UInt8 hugeList[] = { 0xF5, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x01 };   // 2^32 - 1 i32 elements in one byte
        UInt8 elementType = 0;
        UInt32 size = 0;
        CThriftCompactReader listReader( hugeList, sizeof( hugeList ) );
        ASSERT_FALSE( listReader.ReadListHeader( elementType, size ) );
        ASSERT_TRUE( 0 == size );
        const UInt8 doubleList[] = { 0x27, 0, 0, 0, 0, 0, 0, 0, 0 };   // 2 doubles announced, 8 bytes present
        CThriftCompactReader doubles( doubleList, sizeof( doubleList ) );
        ASSERT_FALSE( doubles.ReadListHeader( elementType, size ) );
        const UInt8 hugeMap[] = { 0xFF, 0xFF, 0xFF, 0x7F, 0x55, 0x00 };
        UInt8 keyType = 0, valueType = 0;
        CThriftCompactReader mapReader( hugeMap, sizeof( hugeMap ) );
        ASSERT_FALSE( mapReader.ReadMapHeader( keyType, valueType, size ) );

        const UInt8 unknownType[] = { 0x1E, 0x00 };                    // type 14 does not exist
        UInt8 type = 0;
        Int16 id = 0;
        CThriftCompactReader unknown( unknownType, sizeof( unknownType ) );
        ASSERT_TRUE( unknown.BeginStruct() );
        ASSERT_FALSE( unknown.ReadFieldHeader( type, id ) );
        const UInt8 badBool[] = { 0x03 };
        bool flag = false;
        CThriftCompactReader boolReader( badBool, 1 );
        ASSERT_FALSE( boolReader.ReadBoolElement( flag ) );
        const UInt8 stopWithId[] = { 0x10 };
        CThriftCompactReader stopReader( stopWithId, 1 );
        ASSERT_TRUE( stopReader.BeginStruct() );
        ASSERT_FALSE( stopReader.ReadFieldHeader( type, id ) );

        // the first failure sticks
        ASSERT_FALSE( stopReader.ReadI32( i32 ) );
        ASSERT_TRUE( stopReader.HasFailed() );

        // every truncation of a valid structure fails cleanly or skips what is there; none reads past the end
        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteBinaryField( 1, "0123456789", 10 );
            writer.BeginListField( 2, THRIFT_COMPACT_STRUCT, 3 );
            for ( UInt32 i=0; i<3; ++i ) { writer.BeginStruct(); writer.WriteI64Field( 1, -1000000 ); writer.WriteDoubleField( 2, 3.0 ); writer.EndStruct(); }
            writer.WriteI32Field( 3, 77 );
            writer.EndStruct();
        }
        for ( UInt32 cut=0; cut<out.GetDataSize(); ++cut )
        {
            GUCEF::vector< UInt8 > truncated( Bytes( out ), Bytes( out ) + cut );
            CThriftCompactReader partial( truncated.empty() ? GUCEF_NULL : &truncated[ 0 ], cut );
            ASSERT_FALSE( partial.Skip( THRIFT_COMPACT_STRUCT ) );
        }
        CThriftCompactReader whole( Bytes( out ), out.GetDataSize() );
        ASSERT_TRUE( whole.Skip( THRIFT_COMPACT_STRUCT ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thrift: the parquet metadata structures round trip with the field ids of the specification" )
    try
    {
        const char* names[] = { "schema", "id", "price", "symbol" };
        SParquetFileMetaData meta;
        meta.version = 2;
        meta.numRows = 1000;
        SParquetSchemaElement root;
        root.name = SParquetBytes( names[ 0 ], 6 );
        root.hasNumChildren = true;
        root.numChildren = 3;
        meta.schema.push_back( root );
        SParquetSchemaElement id;
        id.name = SParquetBytes( names[ 1 ], 2 );
        id.hasType = true; id.type = PARQUET_TYPE_INT64;
        id.hasRepetitionType = true; id.repetitionType = PARQUET_REPETITION_REQUIRED;
        id.hasLogicalType = true; id.logicalType.kind = PARQUET_LOGICAL_INTEGER; id.logicalType.bitWidth = 64; id.logicalType.isSigned = true;
        id.hasConvertedType = true; id.convertedType = PARQUET_CONVERTED_INT_64;
        meta.schema.push_back( id );
        SParquetSchemaElement price;
        price.name = SParquetBytes( names[ 2 ], 5 );
        price.hasType = true; price.type = PARQUET_TYPE_INT64;
        price.hasRepetitionType = true; price.repetitionType = PARQUET_REPETITION_OPTIONAL;
        price.hasLogicalType = true; price.logicalType.kind = PARQUET_LOGICAL_DECIMAL; price.logicalType.decimalPrecision = 18; price.logicalType.decimalScale = 6;
        price.hasScale = true; price.scale = 6; price.hasPrecision = true; price.precision = 18;
        meta.schema.push_back( price );
        SParquetSchemaElement symbol;
        symbol.name = SParquetBytes( names[ 3 ], 6 );
        symbol.hasType = true; symbol.type = PARQUET_TYPE_BYTE_ARRAY;
        symbol.hasRepetitionType = true; symbol.repetitionType = PARQUET_REPETITION_OPTIONAL;
        symbol.hasLogicalType = true; symbol.logicalType.kind = PARQUET_LOGICAL_TIMESTAMP; symbol.logicalType.isAdjustedToUtc = true; symbol.logicalType.timeUnit = PARQUET_TIMEUNIT_NANOS;
        meta.schema.push_back( symbol );
        SParquetRowGroup rowGroup;
        rowGroup.numRows = 1000;
        rowGroup.totalByteSize = 12345;
        rowGroup.hasSortingColumns = true;
        SParquetSortingColumn sorting; sorting.columnIdx = 0;
        rowGroup.sortingColumns.push_back( sorting );
        rowGroup.hasOrdinal = true; rowGroup.ordinal = 0;
        SParquetColumnChunk chunk;
        chunk.fileOffset = 4;
        chunk.hasMetaData = true;
        chunk.metaData.type = PARQUET_TYPE_INT64;
        chunk.metaData.encodings.push_back( PARQUET_ENCODING_PLAIN );
        chunk.metaData.encodings.push_back( PARQUET_ENCODING_RLE );
        chunk.metaData.pathInSchema.push_back( SParquetBytes( names[ 1 ], 2 ) );
        chunk.metaData.codec = PARQUET_COMPRESSION_GZIP;
        chunk.metaData.numValues = 1000;
        chunk.metaData.totalUncompressedSize = 8100;
        chunk.metaData.totalCompressedSize = 3000;
        chunk.metaData.dataPageOffset = 4;
        chunk.metaData.hasStatistics = true;
        const UInt8 lo[ 8 ] = { 1, 0, 0, 0, 0, 0, 0, 0 };
        const UInt8 hi[ 8 ] = { 0xE8, 3, 0, 0, 0, 0, 0, 0 };
        chunk.metaData.statistics.hasMinValue = true; chunk.metaData.statistics.minValue = SParquetBytes( lo, 8 );
        chunk.metaData.statistics.hasMaxValue = true; chunk.metaData.statistics.maxValue = SParquetBytes( hi, 8 );
        chunk.metaData.statistics.hasNullCount = true; chunk.metaData.statistics.nullCount = 0;
        chunk.metaData.statistics.hasIsMinValueExact = true; chunk.metaData.statistics.isMinValueExact = true;
        chunk.metaData.hasEncodingStats = true;
        SParquetPageEncodingStats es; es.pageType = PARQUET_PAGE_DATA_PAGE; es.encoding = PARQUET_ENCODING_PLAIN; es.count = 1;
        chunk.metaData.encodingStats.push_back( es );
        for ( UInt32 c=0; c<3; ++c )
            rowGroup.columns.push_back( chunk );
        meta.rowGroups.push_back( rowGroup );
        SParquetKeyValue kv;
        kv.key = SParquetBytes( "k", 1 ); kv.hasValue = true; kv.value = SParquetBytes( "v", 1 );
        meta.keyValueMetaData.push_back( kv );
        meta.hasCreatedBy = true; meta.createdBy = SParquetBytes( "test", 4 );
        meta.hasColumnOrders = true;
        meta.columnOrders.push_back( PARQUET_COLUMNORDER_TYPE_DEFINED );
        meta.columnOrders.push_back( PARQUET_COLUMNORDER_IEEE754_TOTAL );
        meta.columnOrders.push_back( PARQUET_COLUMNORDER_TYPE_DEFINED );

        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            CParquetMetaDataSerializer::WriteFileMetaData( writer, meta );
            ASSERT_TRUE( writer.IsValid() );
        }
        CThriftCompactReader reader( Bytes( out ), out.GetDataSize() );
        SParquetFileMetaData back;
        SParquetParseBudget budget;
        ASSERT_TRUE( CParquetMetaDataSerializer::ReadFileMetaData( reader, back, budget ) );
        ASSERT_TRUE( 0 == reader.GetRemaining() );
        ASSERT_TRUE( 2 == back.version && 1000 == back.numRows && 4 == back.schema.size() && 1 == back.rowGroups.size() );
        ASSERT_TRUE( back.schema[ 0 ].name.Equals( "schema" ) && back.schema[ 0 ].hasNumChildren && 3 == back.schema[ 0 ].numChildren && !back.schema[ 0 ].hasType );
        ASSERT_TRUE( PARQUET_LOGICAL_INTEGER == back.schema[ 1 ].logicalType.kind && 64 == back.schema[ 1 ].logicalType.bitWidth && back.schema[ 1 ].logicalType.isSigned );
        ASSERT_TRUE( PARQUET_LOGICAL_DECIMAL == back.schema[ 2 ].logicalType.kind && 18 == back.schema[ 2 ].logicalType.decimalPrecision && 6 == back.schema[ 2 ].logicalType.decimalScale );
        ASSERT_TRUE( PARQUET_LOGICAL_TIMESTAMP == back.schema[ 3 ].logicalType.kind && back.schema[ 3 ].logicalType.isAdjustedToUtc && PARQUET_TIMEUNIT_NANOS == back.schema[ 3 ].logicalType.timeUnit );
        const SParquetColumnChunk& c = back.rowGroups[ 0 ].columns[ 2 ];
        ASSERT_TRUE( c.hasMetaData && PARQUET_COMPRESSION_GZIP == c.metaData.codec && 2 == c.metaData.encodings.size() && 3000 == c.metaData.totalCompressedSize );
        ASSERT_TRUE( c.metaData.hasStatistics && c.metaData.statistics.hasMinValue && 8 == c.metaData.statistics.minValue.size && 0xE8 == c.metaData.statistics.maxValue.data[ 0 ] );
        ASSERT_TRUE( c.metaData.statistics.hasIsMinValueExact && !c.metaData.statistics.hasIsMaxValueExact && !c.metaData.statistics.hasMin );
        ASSERT_TRUE( c.metaData.hasEncodingStats && 1 == c.metaData.encodingStats.size() && 1 == c.metaData.encodingStats[ 0 ].count );
        ASSERT_TRUE( back.rowGroups[ 0 ].hasSortingColumns && 1 == back.rowGroups[ 0 ].sortingColumns.size() && back.rowGroups[ 0 ].hasOrdinal );
        ASSERT_TRUE( 1 == back.keyValueMetaData.size() && back.keyValueMetaData[ 0 ].key.Equals( "k" ) && back.keyValueMetaData[ 0 ].value.Equals( "v" ) );
        ASSERT_TRUE( back.hasCreatedBy && back.createdBy.Equals( "test" ) );
        ASSERT_TRUE( back.hasColumnOrders && 3 == back.columnOrders.size() && PARQUET_COLUMNORDER_IEEE754_TOTAL == back.columnOrders[ 1 ] );

        // page headers of all three kinds
        SParquetPageHeader page;
        page.type = PARQUET_PAGE_DATA_PAGE_V2;
        page.uncompressedPageSize = 100; page.compressedPageSize = 50;
        page.hasDataPageHeaderV2 = true;
        page.dataPageHeaderV2.numValues = 10; page.dataPageHeaderV2.numNulls = 2; page.dataPageHeaderV2.numRows = 10;
        page.dataPageHeaderV2.encoding = PARQUET_ENCODING_RLE_DICTIONARY; page.dataPageHeaderV2.definitionLevelsByteLength = 3;
        page.dataPageHeaderV2.isCompressed = false;
        page.hasCrc = true; page.crc = -5;
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            CParquetMetaDataSerializer::WritePageHeader( writer, page );
        }
        CThriftCompactReader pageReader( Bytes( out ), out.GetDataSize() );
        SParquetPageHeader pageBack;
        ASSERT_TRUE( CParquetMetaDataSerializer::ReadPageHeader( pageReader, pageBack ) );
        ASSERT_TRUE( PARQUET_PAGE_DATA_PAGE_V2 == pageBack.type && pageBack.hasDataPageHeaderV2 && 2 == pageBack.dataPageHeaderV2.numNulls &&
                     !pageBack.dataPageHeaderV2.isCompressed && 3 == pageBack.dataPageHeaderV2.definitionLevelsByteLength && pageBack.hasCrc && -5 == pageBack.crc );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Thrift: metadata missing a required field, with a field of the wrong type or with negative sizes is CORRUPT_DATA, and the object budget bounds the memory of a parse" )
    try
    {
        // a page header without its compressed size
        CDynamicBuffer out;
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 1, PARQUET_PAGE_DATA_PAGE );
            writer.WriteI32Field( 2, 10 );
            writer.EndStruct();
        }
        SParquetPageHeader page;
        CThriftCompactReader missing( Bytes( out ), out.GetDataSize() );
        ASSERT_FALSE( CParquetMetaDataSerializer::ReadPageHeader( missing, page ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == missing.GetStatus() );

        // the compressed size as a string
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 1, PARQUET_PAGE_DATA_PAGE );
            writer.WriteI32Field( 2, 10 );
            writer.WriteBinaryField( 3, "10", 2 );
            writer.EndStruct();
        }
        CThriftCompactReader wrongType( Bytes( out ), out.GetDataSize() );
        ASSERT_FALSE( CParquetMetaDataSerializer::ReadPageHeader( wrongType, page ) );

        // negative sizes
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 1, PARQUET_PAGE_DATA_PAGE );
            writer.WriteI32Field( 2, -1 );
            writer.WriteI32Field( 3, 10 );
            writer.EndStruct();
        }
        CThriftCompactReader negative( Bytes( out ), out.GetDataSize() );
        ASSERT_FALSE( CParquetMetaDataSerializer::ReadPageHeader( negative, page ) );

        // a footer that declares many empty schema elements: each element costs memory and the budget stops it early
        out.Clear( true );
        const UInt32 count = 200000;
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.WriteI32Field( 1, 1 );
            writer.BeginListField( 2, THRIFT_COMPACT_STRUCT, count );
            const UInt8 element[] = { 0x48, 0x01, 'x', 0x00 };     // a struct holding name "x"
            for ( UInt32 i=0; i<count; ++i )
                writer.WriteRaw( element, sizeof( element ) );
            writer.EndStruct();
        }
        SParquetFileMetaData meta;
        CThriftCompactReader manyReader( Bytes( out ), out.GetDataSize() );
        SParquetParseBudget small( 1024 * 1024 );
        ASSERT_FALSE( CParquetMetaDataSerializer::ReadFileMetaData( manyReader, meta, small ) );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == manyReader.GetStatus() );
        ASSERT_TRUE( small.usedObjectBytes <= small.maxObjectBytes );

        // and a logical type with two members is not a union
        out.Clear( true );
        {
            CThriftCompactWriter writer( out );
            writer.BeginStruct();
            writer.BeginStructField( PARQUET_LOGICAL_STRING ); writer.EndStruct();
            writer.BeginStructField( PARQUET_LOGICAL_DATE ); writer.EndStruct();
            writer.EndStruct();
        }
        SParquetLogicalType logical;
        CThriftCompactReader twoMembers( Bytes( out ), out.GetDataSize() );
        ASSERT_FALSE( CParquetMetaDataSerializer::ReadLogicalType( twoMembers, logical ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
