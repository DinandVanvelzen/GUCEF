/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestDataTableSupport.h"
#include "TestDataTableColumnBatch.h"

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

/*-------------------------------------------------------------------------*/

/** A pooled buffer holding a copy of the bytes */
static CDataTableBufferPtr
BufferWith( CDataTableBufferPool& pool, const void* bytes, UInt32 byteCount )
{
    CDataTableBufferPtr buffer;
    pool.Acquire( byteCount, buffer );
    if ( byteCount > 0 )
        memcpy( buffer->GetData(), bytes, byteCount );
    buffer->SetDataSize( byteCount );
    return buffer;
}

/*-------------------------------------------------------------------------*/

/** A fixed width column batch over a copy of the values */
static void
MakeFixed( CDataTableBufferPool& pool, CDataTableColumnBatch& column, TDataTableLogicalType type, const void* values, UInt32 rowCount,
           UInt8 precision = 18, UInt8 scale = 0, TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS )
{
    column.SetType( type, precision, scale, unit );
    UInt32 width = DataTableLogicalTypeByteWidth( type );
    UInt32 byteCount = 0 == width ? ( rowCount + 7 ) / 8 : rowCount * width;
    CDataTableBufferPtr buffer = BufferWith( pool, values, byteCount );
    column.SetFixedValues( rowCount, buffer->GetData(), buffer );
}

/*-------------------------------------------------------------------------*/

/** A variable column batch of the strings, each followed by its terminator; a null pointer is an empty value */
static void
MakeVariable( CDataTableBufferPool& pool, CDataTableColumnBatch& column, TDataTableLogicalType type, const char* const* values, UInt32 rowCount )
{
    column.SetType( type );
    CDataTableBufferPtr offsets, bytes;
    pool.Acquire( ( rowCount + 1 ) * 4, offsets );
    pool.Acquire( 4096, bytes );
    UInt32* offsetData = static_cast< UInt32* >( offsets->GetData() );
    char* byteData = static_cast< char* >( bytes->GetData() );
    UInt32 used = 0;
    offsetData[ 0 ] = 0;
    for ( UInt32 i=0; i<rowCount; ++i )
    {
        const char* value = GUCEF_NULL != values[ i ] ? values[ i ] : "";
        UInt32 length = (UInt32) strlen( value );
        memcpy( byteData + used, value, length + 1 );
        used += length + 1;
        offsetData[ i + 1 ] = used;
    }
    offsets->SetDataSize( ( rowCount + 1 ) * 4 );
    bytes->SetDataSize( used );
    column.SetVariableValues( rowCount, offsetData, offsets, byteData, bytes );
}

/*-------------------------------------------------------------------------*/

/** Counts the typed span getters that answer */
static UInt32
TypedSpanCount( const CDataTableColumnBatch& column )
{
    UInt32 count = 0;
    count += GUCEF_NULL != column.GetInt8Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetInt16Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetInt32Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetInt64Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetUInt8Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetUInt16Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetUInt32Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetUInt64Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetFloat32Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetFloat64Data() ? 1 : 0;
    count += GUCEF_NULL != column.GetBoolBits() ? 1 : 0;
    count += GUCEF_NULL != column.GetOffsets() ? 1 : 0;
    count += GUCEF_NULL != column.GetDictionaryIndices() ? 1 : 0;
    return count;
}

/*-------------------------------------------------------------------------*/

void
PerformDataTableColumnBatchTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING data table column batch TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DataTableColumnBatch" );

    GUCEF_TESTFW_TESTCASE( "Test: fixed width storage of every type: one typed span each, Tier 1 under the native type ids" )
        try
        {
            CDataTableBufferPool pool;
            CVariant out;

            const Int8 i8[] = { -128, 0, 127 };
            const Int16 i16[] = { -32768, 1, 32767 };
            const Int32 i32[] = { GUCEF_INT32MIN, 2, GUCEF_INT32MAX };
            const Int64 i64[] = { GUCEF_INT64MIN, 3, GUCEF_INT64MAX };
            const UInt8 u8[] = { 0, 4, 255 };
            const UInt16 u16[] = { 0, 5, 65535 };
            const UInt32 u32[] = { 0, 6, GUCEF_UINT32MAX };
            const UInt64 u64[] = { 0, 7, GUCEF_UINT64MAX };
            const Float32 f32[] = { -1.5f, 0.0f, 3.25f };
            const Float64 f64[] = { -1e300, 0.1, 2.5 };

            CDataTableColumnBatch column;
            MakeFixed( pool, column, DATATABLE_TYPE_INT8, i8, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && GUCEF_NULL != column.GetInt8Data() && -128 == column.GetInt8Data()[ 0 ] );
            ASSERT_TRUE( DATATABLE_STORAGE_FIXED == column.GetStorage() && 3 == column.GetRowCount() && column.GetFixedData() == column.GetInt8Data() );
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_INT8 == out.GetTypeId() && 127 == out.AsInt8() );
            MakeFixed( pool, column, DATATABLE_TYPE_INT16, i16, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 32767 == column.GetInt16Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_INT16 == out.GetTypeId() && -32768 == out.AsInt16() );
            MakeFixed( pool, column, DATATABLE_TYPE_INT32, i32, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && GUCEF_INT32MIN == column.GetInt32Data()[ 0 ] );
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_INT32 == out.GetTypeId() && GUCEF_INT32MAX == out.AsInt32() );
            MakeFixed( pool, column, DATATABLE_TYPE_INT64, i64, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && GUCEF_INT64MAX == column.GetInt64Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_INT64 == out.GetTypeId() && GUCEF_INT64MIN == out.AsInt64() );
            MakeFixed( pool, column, DATATABLE_TYPE_UINT8, u8, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 255 == column.GetUInt8Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 1, out ) && GUCEF_DATATYPE_UINT8 == out.GetTypeId() && 4 == out.AsUInt8() );
            MakeFixed( pool, column, DATATABLE_TYPE_UINT16, u16, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 65535 == column.GetUInt16Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_UINT16 == out.GetTypeId() && 65535 == out.AsUInt16() );
            MakeFixed( pool, column, DATATABLE_TYPE_UINT32, u32, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && GUCEF_UINT32MAX == column.GetUInt32Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_UINT32 == out.GetTypeId() && GUCEF_UINT32MAX == out.AsUInt32() );
            MakeFixed( pool, column, DATATABLE_TYPE_UINT64, u64, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && GUCEF_UINT64MAX == column.GetUInt64Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_UINT64 == out.GetTypeId() && GUCEF_UINT64MAX == out.AsUInt64() );
            MakeFixed( pool, column, DATATABLE_TYPE_FLOAT32, f32, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 3.25f == column.GetFloat32Data()[ 2 ] );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_FLOAT32 == out.GetTypeId() && -1.5f == out.AsFloat32() );
            MakeFixed( pool, column, DATATABLE_TYPE_FLOAT64, f64, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 0.1 == column.GetFloat64Data()[ 1 ] );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_FLOAT64 == out.GetTypeId() && -1e300 == out.AsFloat64() );

            // dates and timestamps share the integer spans, Tier 1 gives their own ids
            const Int32 days[] = { -1, 0, 20395 };
            MakeFixed( pool, column, DATATABLE_TYPE_DATE, days, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && -1 == column.GetInt32Data()[ 0 ] && DATATABLE_TYPE_DATE == column.GetLogicalType() );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == out.GetTypeId() && out.AsString() == "1969-12-31" );
            const Int64 ticks[] = { -1, 0, 1762180200123456LL };
            const TDataTableTimeUnit units[] = { DATATABLE_TIMEUNIT_MILLISECONDS, DATATABLE_TIMEUNIT_MICROSECONDS, DATATABLE_TIMEUNIT_NANOSECONDS };
            const UInt8 ids[] = { GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH, GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH };
            for ( UInt32 u=0; u<3; ++u )
            {
                MakeFixed( pool, column, DATATABLE_TYPE_TIMESTAMP, ticks, 3, 18, 0, units[ u ] );
                ASSERT_TRUE( 1 == TypedSpanCount( column ) && units[ u ] == column.GetTimeUnit() && column.IsUtcAdjusted() );
                ASSERT_TRUE( column.GetValue( 0, out ) && ids[ u ] == out.GetTypeId() && -1 == out.AsInt64() );
                ASSERT_TRUE( column.GetValue( 2, out ) && 1762180200123456LL == out.AsInt64() );
            }
            ASSERT_TRUE( column.GetValue( 2, out ) && GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == out.GetTypeId() );

            // decimals: mantissas at the column scale, Tier 1 as CDecimal with that scale
            const Int64 mantissas[] = { -1250, 0, 999999999999999999LL };
            MakeFixed( pool, column, DATATABLE_TYPE_DECIMAL, mantissas, 3, 18, 3 );
            ASSERT_TRUE( 1 == TypedSpanCount( column ) && 3 == column.GetDecimalScale() && 18 == column.GetDecimalPrecision() );
            ASSERT_TRUE( column.GetValue( 0, out ) && out.IsDecimal() && out.AsDecimal().IsIdentical( CDecimal::Parse( "-1.250" ) ) );
            ASSERT_TRUE( column.GetValue( 2, out ) && out.AsString() == "999999999999999.999" );
            const Int64 corrupt[] = { GUCEF_INT64MIN };
            MakeFixed( pool, column, DATATABLE_TYPE_DECIMAL, corrupt, 1, 18, 3 );
            ASSERT_FALSE( column.GetValue( 0, out ) );
            ASSERT_FALSE( out.IsInitialized() );

            // rows out of range
            MakeFixed( pool, column, DATATABLE_TYPE_INT32, i32, 3 );
            out = 5;
            ASSERT_FALSE( column.GetValue( 3, out ) );
            ASSERT_FALSE( out.IsInitialized() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: booleans as bits, validity bitmaps and null cells" )
        try
        {
            CDataTableBufferPool pool;
            CVariant out;
            CDataTableColumnBatch column;

            const UInt8 bits[] = { 0x05, 0x01 };                // rows 0, 2 and 8 true of 9
            MakeFixed( pool, column, DATATABLE_TYPE_BOOL, bits, 9 );
            ASSERT_TRUE( DATATABLE_STORAGE_BITS == column.GetStorage() && 1 == TypedSpanCount( column ) && GUCEF_NULL != column.GetBoolBits() );
            ASSERT_TRUE( GUCEF_NULL == column.GetFixedData() );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_BOOLEAN_INT32 == out.GetTypeId() && out.AsBool() );
            ASSERT_TRUE( column.GetValue( 1, out ) && !out.AsBool() );
            ASSERT_TRUE( column.GetValue( 8, out ) && out.AsBool() );
            ASSERT_FALSE( column.GetValue( 9, out ) );

            // the fast path: no nulls, no bitmap
            ASSERT_FALSE( column.HasNulls() );
            ASSERT_TRUE( GUCEF_NULL == column.GetValidity() && 0 == column.GetNullCount() && column.IsValid( 3 ) );

            const Int64 values[] = { 10, 0, 30, 0, 50, 60, 70, 80, 0 };
            const UInt8 validity[] = { 0xF5, 0x00 };            // rows 1, 3 and 8 null
            MakeFixed( pool, column, DATATABLE_TYPE_INT64, values, 9 );
            CDataTableBufferPtr validityBuffer = BufferWith( pool, validity, 2 );
            column.SetValidity( static_cast< const UInt8* >( validityBuffer->GetData() ), 3, validityBuffer );
            ASSERT_TRUE( column.HasNulls() && 3 == column.GetNullCount() && GUCEF_NULL != column.GetValidity() );
            ASSERT_TRUE( column.IsValid( 0 ) && !column.IsValid( 1 ) && !column.IsValid( 3 ) && column.IsValid( 7 ) && !column.IsValid( 8 ) );
            ASSERT_TRUE( column.GetValue( 1, out ) && out.IsNULL() && GUCEF_DATATYPE_NULL == out.GetTypeId() );
            ASSERT_TRUE( column.GetValue( 2, out ) && !out.IsNULL() && 30 == out.AsInt64() );
            CString error;
            ASSERT_TRUE( column.Validate( error ) );
            ASSERT_TRUE( 2 == validityBuffer->GetReferenceCount() );

            // a null count of zero or no bitmap drops the bitmap
            column.SetValidity( static_cast< const UInt8* >( validityBuffer->GetData() ), 0, validityBuffer );
            ASSERT_TRUE( GUCEF_NULL == column.GetValidity() && !column.HasNulls() && 1 == validityBuffer->GetReferenceCount() );
            column.SetValidity( GUCEF_NULL, 3, validityBuffer );
            ASSERT_TRUE( GUCEF_NULL == column.GetValidity() && 0 == column.GetNullCount() );

            // null slots must be zero and the count must match the bitmap
            column.SetValidity( static_cast< const UInt8* >( validityBuffer->GetData() ), 2, validityBuffer );
            ASSERT_FALSE( column.Validate( error ) );
            const Int64 dirty[] = { 10, 99, 30, 0, 50, 60, 70, 80, 0 };
            MakeFixed( pool, column, DATATABLE_TYPE_INT64, dirty, 9 );
            column.SetValidity( static_cast< const UInt8* >( validityBuffer->GetData() ), 3, validityBuffer );
            ASSERT_FALSE( column.Validate( error ) );
            ASSERT_TRUE( error.HasSubstr( "row 1" ) >= 0 );

            // an all-null column
            const Int32 zeros[] = { 0, 0, 0, 0 };
            const UInt8 noneValid[] = { 0x00 };
            MakeFixed( pool, column, DATATABLE_TYPE_INT32, zeros, 4 );
            CDataTableBufferPtr noneBuffer = BufferWith( pool, noneValid, 1 );
            column.SetValidity( static_cast< const UInt8* >( noneBuffer->GetData() ), 4, noneBuffer );
            ASSERT_TRUE( column.Validate( error ) && 4 == column.GetNullCount() );
            for ( UInt32 i=0; i<4; ++i )
                ASSERT_TRUE( column.GetValue( i, out ) && out.IsNULL() );

            // a null bool bit must be clear
            const UInt8 setBit[] = { 0x02 };
            const UInt8 secondNull[] = { 0x01 };
            MakeFixed( pool, column, DATATABLE_TYPE_BOOL, setBit, 2 );
            CDataTableBufferPtr secondNullBuffer = BufferWith( pool, secondNull, 1 );
            column.SetValidity( static_cast< const UInt8* >( secondNullBuffer->GetData() ), 1, secondNullBuffer );
            ASSERT_FALSE( column.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: variable values carry their terminator and Tier 1 links into the buffer" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableColumnBatch column;
            const char* values[] = { "", "a", "h\xC3\xA9llo", "XNYS", GUCEF_NULL };
            MakeVariable( pool, column, DATATABLE_TYPE_UTF8, values, 5 );
            ASSERT_TRUE( DATATABLE_STORAGE_VARIABLE == column.GetStorage() && 1 == TypedSpanCount( column ) );
            ASSERT_TRUE( GUCEF_NULL != column.GetOffsets() && GUCEF_NULL != column.GetBytes() && GUCEF_NULL == column.GetFixedData() );
            ASSERT_TRUE( GUCEF_NULL == column.GetDictionary() );
            const UInt32* offsets = column.GetOffsets();
            ASSERT_TRUE( 0 == offsets[ 0 ] && 1 == offsets[ 1 ] && 3 == offsets[ 2 ] );
            ASSERT_TRUE( '\0' == column.GetBytes()[ offsets[ 3 ] - 1 ] );

            UInt32 length = 99;
            const char* bytes = column.GetVariableValue( 2, length );
            ASSERT_TRUE( 6 == length && 0 == memcmp( bytes, "h\xC3\xA9llo", 6 ) && '\0' == bytes[ length ] );
            ASSERT_TRUE( GUCEF_NULL != column.GetVariableValue( 0, length ) && 0 == length );
            CString error;
            ASSERT_TRUE( column.Validate( error ) );

            // linked: the variant points into the batch buffer, the terminator included, no copy
            CVariant out;
            ASSERT_TRUE( column.GetValue( 3, out ) );
            ASSERT_TRUE( GUCEF_DATATYPE_UTF8_STRING == out.GetTypeId() && out.IsDynamicMemoryLinked() );
            ASSERT_TRUE( out.AsCharPtr() == column.GetBytes() + offsets[ 3 ] );
            ASSERT_TRUE( 5 == out.ByteSize() && 4 == strlen( out.AsCharPtr() ) && out.AsString() == "XNYS" );
            ASSERT_TRUE( out == CVariant( "XNYS" ) );

            // empty and null stay distinct
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_UTF8_STRING == out.GetTypeId() && out.AsString().IsNULLOrEmpty() && !out.IsNULL() );

            // a copy detaches, LinkTo passes the link on, no link when not asked for
            ASSERT_TRUE( column.GetValue( 2, out ) && out.IsDynamicMemoryLinked() );
            CVariant copy( out );
            ASSERT_FALSE( copy.IsDynamicMemoryLinked() );
            ASSERT_TRUE( copy.AsCharPtr() != out.AsCharPtr() && copy == out );
            CVariant assigned;
            assigned = out;
            ASSERT_FALSE( assigned.IsDynamicMemoryLinked() );
            CVariant linkedCopy;
            linkedCopy.LinkTo( out );
            ASSERT_TRUE( linkedCopy.IsDynamicMemoryLinked() && linkedCopy.AsCharPtr() == out.AsCharPtr() );
            CVariant owned;
            ASSERT_TRUE( column.GetValue( 2, owned, false ) && !owned.IsDynamicMemoryLinked() && owned.AsCharPtr() != out.AsCharPtr() );
            ASSERT_TRUE( owned.AsString() == "h\xC3\xA9llo" );

            // a detached copy outlives the batch
            CDataTableBatchPtr batch;
            pool.AcquireBatch( batch );
            batch.GetWritable()->SetColumnCount( 1 );
            batch.GetWritable()->GetMutableColumn( 0 ) = column;
            batch.GetWritable()->SetRowCount( 5 );
            column.Reset();
            CVariant linked, detached;
            ASSERT_TRUE( batch->GetValue( 3, 0, linked ) && linked.IsDynamicMemoryLinked() );
            detached = linked;
            CDataTableBatchPtr held( batch );
            batch.Reset();
            ASSERT_TRUE( linked.AsString() == "XNYS" );             // valid while a handle is held
            linked.Clear();
            held.Reset();
            ASSERT_TRUE( detached.AsString() == "XNYS" );

            // binary values link without the terminator
            const char* blobs[] = { "\x01\x02", "", "abc" };
            MakeVariable( pool, column, DATATABLE_TYPE_BINARY, blobs, 3 );
            ASSERT_TRUE( column.GetValue( 0, out ) && GUCEF_DATATYPE_BINARY_BLOB == out.GetTypeId() && 2 == out.ByteSize() && out.IsDynamicMemoryLinked() );
            ASSERT_TRUE( 0 == memcmp( out.AsVoidPtr(), "\x01\x02", 2 ) );
            ASSERT_TRUE( column.GetValue( 1, out ) && GUCEF_DATATYPE_BINARY_BLOB == out.GetTypeId() && 0 == out.ByteSize() );
            ASSERT_TRUE( column.GetValue( 2, out, false ) && 3 == out.ByteSize() && !out.IsDynamicMemoryLinked() );

            // broken offsets and terminators are found
            MakeVariable( pool, column, DATATABLE_TYPE_UTF8, values, 4 );
            UInt32* writableOffsets = const_cast< UInt32* >( column.GetOffsets() );
            char* writableBytes = const_cast< char* >( column.GetBytes() );
            writableBytes[ 2 ] = 'x';
            ASSERT_FALSE( column.Validate( error ) );
            writableBytes[ 2 ] = '\0';
            ASSERT_TRUE( column.Validate( error ) );
            writableOffsets[ 2 ] = writableOffsets[ 1 ];
            ASSERT_FALSE( column.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: dictionary text: shared dictionaries, indices, Tier 1 linked into the dictionary" )
        try
        {
            CDataTableBufferPool pool;

            // the dictionary of a row group is a batch of one variable column
            CDataTableBatchPtr dictionary;
            pool.AcquireBatch( dictionary );
            const char* entries[] = { "", "XNYS", "XNAS" };
            dictionary.GetWritable()->SetColumnCount( 1 );
            MakeVariable( pool, dictionary.GetWritable()->GetMutableColumn( 0 ), DATATABLE_TYPE_UTF8, entries, 3 );
            dictionary.GetWritable()->SetRowCount( 3 );

            const UInt32 indices[] = { 1, 2, 0, 1, 2 };
            const UInt8 validity[] = { 0x1B };                  // row 2 null, its index 0
            CDataTableBufferPtr indexBuffer = BufferWith( pool, indices, sizeof( indices ) );
            CDataTableBufferPtr validityBuffer = BufferWith( pool, validity, 1 );
            CDataTableColumnBatch first, second;
            first.SetType( DATATABLE_TYPE_UTF8 );
            first.SetDictionaryValues( 5, static_cast< const UInt32* >( indexBuffer->GetData() ), indexBuffer, dictionary );
            first.SetValidity( static_cast< const UInt8* >( validityBuffer->GetData() ), 1, validityBuffer );
            second.SetType( DATATABLE_TYPE_UTF8 );
            second.SetDictionaryValues( 2, static_cast< const UInt32* >( indexBuffer->GetData() ) + 3, indexBuffer, dictionary );
            ASSERT_TRUE( 3 == dictionary->GetReferenceCount() );
            const CDataTableBatch* dictionaryBatch = dictionary.GetPointer();
            dictionary.Reset();                                  // the column batches keep it alive

            ASSERT_TRUE( DATATABLE_STORAGE_DICTIONARY == first.GetStorage() && 1 == TypedSpanCount( first ) );
            ASSERT_TRUE( GUCEF_NULL == first.GetOffsets() && GUCEF_NULL == first.GetBytes() );
            ASSERT_TRUE( first.GetDictionary() == &dictionaryBatch->GetColumn( 0 ) && first.GetDictionary() == second.GetDictionary() );
            ASSERT_TRUE( 2 == first.GetDictionaryIndices()[ 1 ] && 1 == second.GetDictionaryIndices()[ 0 ] );
            CString error;
            ASSERT_TRUE( first.Validate( error ) && second.Validate( error ) );

            UInt32 length = 0;
            ASSERT_TRUE( 0 == memcmp( first.GetVariableValue( 1, length ), "XNAS", 4 ) && 4 == length );
            CVariant out;
            ASSERT_TRUE( first.GetValue( 0, out ) && out.AsString() == "XNYS" && out.IsDynamicMemoryLinked() );
            ASSERT_TRUE( out.AsCharPtr() == first.GetDictionary()->GetBytes() + first.GetDictionary()->GetOffsets()[ 1 ] );
            ASSERT_TRUE( first.GetValue( 2, out ) && out.IsNULL() );
            ASSERT_TRUE( second.GetValue( 1, out ) && out.AsString() == "XNAS" );

            // an index beyond the dictionary and a dictionary of another type are found
            UInt32* writableIndices = const_cast< UInt32* >( first.GetDictionaryIndices() );
            writableIndices[ 4 ] = 3;
            ASSERT_FALSE( first.Validate( error ) );
            writableIndices[ 4 ] = 2;
            CDataTableColumnBatch wrongType( first );
            wrongType.SetType( DATATABLE_TYPE_BINARY );
            CDataTableBatchPtr keep;
            pool.AcquireBatch( keep );
            keep.GetWritable()->SetColumnCount( 1 );
            keep.GetWritable()->GetMutableColumn( 0 ) = second;
            wrongType.SetDictionaryValues( 2, static_cast< const UInt32* >( indexBuffer->GetData() ), indexBuffer, keep );
            ASSERT_FALSE( wrongType.Validate( error ) );                // the dictionary is itself a dictionary column

            // a column batch copy shares the buffers and the dictionary
            CDataTableColumnBatch copy( first );
            ASSERT_TRUE( copy.GetDictionary() == first.GetDictionary() && copy.GetDictionaryIndices() == first.GetDictionaryIndices() );
            first.Reset();
            ASSERT_TRUE( DATATABLE_TYPE_UTF8 == first.GetLogicalType() && DATATABLE_STORAGE_NONE == first.GetStorage() && 0 == first.GetRowCount() );
            ASSERT_TRUE( GUCEF_NULL == first.GetDictionary() && copy.GetValue( 1, out ) && out.AsString() == "XNAS" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: empty column batches and column batches without storage" )
        try
        {
            CDataTableColumnBatch empty;
            CVariant out;
            ASSERT_TRUE( DATATABLE_TYPE_UNSUPPORTED == empty.GetLogicalType() && DATATABLE_STORAGE_NONE == empty.GetStorage() );
            ASSERT_TRUE( 0 == empty.GetRowCount() && 0 == TypedSpanCount( empty ) && GUCEF_NULL == empty.GetFixedData() );
            ASSERT_FALSE( empty.GetValue( 0, out ) );
            UInt32 length = 7;
            ASSERT_TRUE( GUCEF_NULL == empty.GetVariableValue( 0, length ) && 0 == length );
            CString error;
            ASSERT_TRUE( empty.Validate( error ) );

            CDataTableColumnBatch noRows;
            noRows.SetType( DATATABLE_TYPE_INT64 );
            noRows.SetFixedValues( 0, GUCEF_NULL, CDataTableBufferPtr() );
            ASSERT_TRUE( DATATABLE_STORAGE_FIXED == noRows.GetStorage() && 0 == noRows.GetRowCount() && noRows.Validate( error ) );
            ASSERT_FALSE( noRows.GetValue( 0, out ) );

            // an unsupported column delivers nothing
            CDataTableColumnBatch unsupported;
            unsupported.SetType( DATATABLE_TYPE_UNSUPPORTED );
            const Int64 value = 1;
            unsupported.SetFixedValues( 1, &value, CDataTableBufferPtr() );
            ASSERT_TRUE( DATATABLE_STORAGE_NONE == unsupported.GetStorage() );
            ASSERT_FALSE( unsupported.GetValue( 0, out ) );
            ASSERT_FALSE( unsupported.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Tier 1 over every cell into one reused variant makes no heap allocation" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            CDataTableBufferPool pool;
            CDataTableColumnBatch numbers, decimals, text, dates, flags;
            const Int64 values[] = { 1, -2, 3, 4 };
            MakeFixed( pool, numbers, DATATABLE_TYPE_INT64, values, 4 );
            MakeFixed( pool, decimals, DATATABLE_TYPE_DECIMAL, values, 4, 10, 2 );
            const Int32 days[] = { 1, -2, 3, 4 };
            MakeFixed( pool, dates, DATATABLE_TYPE_DATE, days, 4 );
            const UInt8 bits[] = { 0x05 };
            MakeFixed( pool, flags, DATATABLE_TYPE_BOOL, bits, 4 );
            const char* strings[] = { "alpha", "", "gamma", "delta" };
            MakeVariable( pool, text, DATATABLE_TYPE_UTF8, strings, 4 );

            CVariant out;
            Int64 sum = 0;
            UInt64 textBytes = 0;
            CORETEST::StartAllocationCounting();
            for ( UInt32 round=0; round<1000; ++round )
            {
                for ( UInt32 row=0; row<4; ++row )
                {
                    numbers.GetValue( row, out );
                    sum += out.AsInt64();
                    decimals.GetValue( row, out );
                    dates.GetValue( row, out );
                    sum += out.AsInt32();
                    flags.GetValue( row, out );
                    text.GetValue( row, out );
                    textBytes += out.ByteSize();
                }
            }
            UInt32 allocations = CORETEST::StopAllocationCounting();
            if ( allocations > 0 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Tier 1 access allocated " << allocations << " times:\n" << CORETEST::GetCountedAllocationStacks() );
            ASSERT_TRUE( 0 == allocations );
            ASSERT_TRUE( 1000 * ( 6 + 6 ) == sum && 1000ULL * ( 6 + 1 + 6 + 6 ) == textBytes );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
