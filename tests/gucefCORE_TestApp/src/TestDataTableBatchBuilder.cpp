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

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEROWCURSOR_H
#include "gucefCORE_CDataTableRowCursor.h"
#define GUCEF_CORE_CDATATABLEROWCURSOR_H
#endif /* GUCEF_CORE_CDATATABLEROWCURSOR_H ? */

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
#include "TestDataTableBatchBuilder.h"

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

enum EAllTypesColumn
{
    COL_B = 0, COL_I8, COL_I16, COL_I32, COL_I64, COL_U8, COL_U16, COL_U32, COL_U64, COL_F32, COL_F64,
    COL_DEC, COL_D, COL_TS, COL_TMS, COL_TNS, COL_S, COL_BIN, COL_COUNT
};

/*-------------------------------------------------------------------------*/

static CDecimal
D( const char* text )
{
    return CDecimal::Parse( text );
}

/*-------------------------------------------------------------------------*/

static Int32
DaysOf( Int32 year, UInt32 month, UInt32 day )
{
    Int32 days = 0;
    CDataTableValues::DaysFromCivil( year, month, day, days );
    return days;
}

/*-------------------------------------------------------------------------*/

/** A schema of one column */
static CDataTableSchemaPtr
OneColumn( const CDataTableColumnDef& column )
{
    CDataTableSchema schema;
    schema.AddColumn( column );
    return CreateDataTableSchemaPtr( schema );
}

/*-------------------------------------------------------------------------*/

/** Appends the variant to a fresh one-column builder and tells whether it was taken; the value in out when it was */
static bool
AppendsAs( const CDataTableColumnDef& column, const CVariant& value, CVariant& out, TDataTableStatus* status = GUCEF_NULL )
{
    CDataTableBufferPool& pool = CCoreGlobal::Instance()->GetDataTableBufferPool();
    CDataTableBatchBuilder builder( OneColumn( column ), pool );
    bool ok = builder.GetColumn( 0 ).Append( value );
    if ( GUCEF_NULL != status )
        *status = builder.GetColumn( 0 ).GetLastStatus();
    if ( !ok )
        return false;
    CDataTableBatchPtr batch;
    CString error;
    if ( DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        return false;
    return batch->GetValue( 0, 0, out, false );
}

/*-------------------------------------------------------------------------*/

/** Batches handed out by a test source: a list of batches, then END, or a failure */
class CTestBatchSource : public CIDataTableBatchSource
{
    public:

    GUCEF::vector< CDataTableBatchPtr > batches;
    UInt32 next;
    TDataTableStatus endStatus;

    CTestBatchSource( void ) : CIDataTableBatchSource(), batches(), next( 0 ), endStatus( DATATABLE_STATUS_END ) {}

    virtual TDataTableStatus NextBatch( CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( next < (UInt32) batches.size() )
        {
            batch = batches[ next++ ];
            return DATATABLE_STATUS_OK;
        }
        return endStatus;
    }

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return DATATABLE_STATUS_END == endStatus ? CString::Empty : CString( "the test source failed" );
    }
};

/*-------------------------------------------------------------------------*/

/** A batch of one INT32 column with the values first .. first + count - 1 */
static CDataTableBatchPtr
MakeIntBatch( CDataTableBufferPool& pool, Int32 first, UInt32 count )
{
    CDataTableBatchBuilder builder( OneColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_INT32 ) ), pool );
    for ( UInt32 i=0; i<count; ++i )
        builder.GetColumn( 0 ).AppendInt64( first + (Int32) i );
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/*-------------------------------------------------------------------------*/

void
PerformDataTableBatchBuilderTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING data table batch builder TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DataTableBatchBuilder" );

    GUCEF_TESTFW_TESTCASE( "Test: a round trip of every column type with typed appends, nulls included" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = CORETEST::MakeAllTypesSchema();
            CDataTableBatchBuilder builder( schema, pool );
            ASSERT_TRUE( COL_COUNT == builder.GetColumnCount() && builder.GetSchema().GetPointerAlways() == schema.GetPointerAlways() );

            const Int64 tsTicks = 1762180200123456LL;
            // row 0: low values, row 1: nulls, row 2: high values
            ASSERT_TRUE( builder.GetColumn( COL_B ).AppendBool( true ) );
            ASSERT_TRUE( builder.GetColumn( COL_I8 ).AppendInt64( -128 ) );
            ASSERT_TRUE( builder.GetColumn( COL_I16 ).AppendInt64( -32768 ) );
            ASSERT_TRUE( builder.GetColumn( COL_I32 ).AppendInt64( GUCEF_INT32MIN ) );
            ASSERT_TRUE( builder.GetColumn( COL_I64 ).AppendInt64( GUCEF_INT64MIN ) );
            ASSERT_TRUE( builder.GetColumn( COL_U8 ).AppendUInt64( 0 ) );
            ASSERT_TRUE( builder.GetColumn( COL_U16 ).AppendInt64( 1 ) );
            ASSERT_TRUE( builder.GetColumn( COL_U32 ).AppendUInt64( 2 ) );
            ASSERT_TRUE( builder.GetColumn( COL_U64 ).AppendUInt64( 3 ) );
            ASSERT_TRUE( builder.GetColumn( COL_F32 ).AppendFloat64( -0.5 ) );
            ASSERT_TRUE( builder.GetColumn( COL_F64 ).AppendFloat64( -1e-300 ) );
            ASSERT_TRUE( builder.GetColumn( COL_DEC ).AppendDecimal( D( "-12.5" ) ) );
            ASSERT_TRUE( builder.GetColumn( COL_D ).AppendDate( CDate( 1962, 1, 2 ) ) );
            ASSERT_TRUE( builder.GetColumn( COL_TS ).AppendTimestamp( -1 ) );
            ASSERT_TRUE( builder.GetColumn( COL_TMS ).AppendDateTime( CDateTime( CDate( 2026, 11, 3 ), CTime( 14, 30, 0, 123 ), 0 ) ) );
            ASSERT_TRUE( builder.GetColumn( COL_TNS ).AppendTimestamp( 5 ) );
            ASSERT_TRUE( builder.GetColumn( COL_S ).AppendString( "XNYS" ) );
            ASSERT_TRUE( builder.GetColumn( COL_BIN ).AppendBinary( "\x00\x01", 2 ) );
            for ( UInt32 c=0; c<COL_COUNT; ++c )
                ASSERT_TRUE( builder.GetColumn( c ).AppendNull() );
            ASSERT_TRUE( builder.GetColumn( COL_B ).AppendBool( false ) );
            ASSERT_TRUE( builder.GetColumn( COL_I8 ).AppendInt64( 127 ) );
            ASSERT_TRUE( builder.GetColumn( COL_I16 ).AppendInt64( 32767 ) );
            ASSERT_TRUE( builder.GetColumn( COL_I32 ).AppendInt64( GUCEF_INT32MAX ) );
            ASSERT_TRUE( builder.GetColumn( COL_I64 ).AppendInt64( GUCEF_INT64MAX ) );
            ASSERT_TRUE( builder.GetColumn( COL_U8 ).AppendUInt64( 255 ) );
            ASSERT_TRUE( builder.GetColumn( COL_U16 ).AppendUInt64( 65535 ) );
            ASSERT_TRUE( builder.GetColumn( COL_U32 ).AppendUInt64( GUCEF_UINT32MAX ) );
            ASSERT_TRUE( builder.GetColumn( COL_U64 ).AppendUInt64( GUCEF_UINT64MAX ) );
            ASSERT_TRUE( builder.GetColumn( COL_F32 ).AppendFloat64( 3.25 ) );
            ASSERT_TRUE( builder.GetColumn( COL_F64 ).AppendFloat64( 0.1 ) );
            ASSERT_TRUE( builder.GetColumn( COL_DEC ).AppendDecimalMantissa( 999999999999999999LL ) );
            ASSERT_TRUE( builder.GetColumn( COL_D ).AppendDate( DaysOf( 9999, 12, 31 ) ) );
            ASSERT_TRUE( builder.GetColumn( COL_TS ).AppendTimestamp( tsTicks ) );
            ASSERT_TRUE( builder.GetColumn( COL_TMS ).AppendTimestamp( -86400000LL ) );
            ASSERT_TRUE( builder.GetColumn( COL_TNS ).AppendTimestamp( GUCEF_INT64MAX ) );
            ASSERT_TRUE( builder.GetColumn( COL_S ).AppendUtf8( "h\xC3\xA9llo", 6 ) );
            ASSERT_TRUE( builder.GetColumn( COL_BIN ).AppendBinary( GUCEF_NULL, 0 ) );
            ASSERT_TRUE( 3 == builder.GetRowCount() && 1 == builder.GetColumn( COL_S ).GetNullCount() );

            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) && !batch.IsNULL() );
            ASSERT_TRUE( 0 == builder.GetRowCount() );
            ASSERT_TRUE( 3 == batch->GetRowCount() && COL_COUNT == batch->GetColumnCount() && 3 == batch->GetSelectedRowCount() );
            ASSERT_TRUE( batch->GetSchema().GetPointerAlways() == schema.GetPointerAlways() );
            for ( UInt32 c=0; c<COL_COUNT; ++c )
            {
                const CDataTableColumnBatch& column = batch->GetColumn( c );
                ASSERT_TRUE( c == batch->GetSchemaColumnIndex( c ) );
                ASSERT_TRUE( column.GetLogicalType() == schema->GetColumn( c ).type );
                ASSERT_TRUE( 3 == column.GetRowCount() && 1 == column.GetNullCount() && column.IsValid( 0 ) && !column.IsValid( 1 ) && column.IsValid( 2 ) );
                ASSERT_TRUE( column.Validate( error ) );
                CVariant cell;
                ASSERT_TRUE( batch->GetValue( 1, c, cell ) && cell.IsNULL() );
            }

            // spans
            ASSERT_TRUE( 0x01 == ( batch->GetColumn( COL_B ).GetBoolBits()[ 0 ] & 0x07 ) );
            ASSERT_TRUE( -128 == batch->GetColumn( COL_I8 ).GetInt8Data()[ 0 ] && 0 == batch->GetColumn( COL_I8 ).GetInt8Data()[ 1 ] );
            ASSERT_TRUE( 32767 == batch->GetColumn( COL_I16 ).GetInt16Data()[ 2 ] );
            ASSERT_TRUE( GUCEF_INT64MIN == batch->GetColumn( COL_I64 ).GetInt64Data()[ 0 ] && 0 == batch->GetColumn( COL_I64 ).GetInt64Data()[ 1 ] );
            ASSERT_TRUE( GUCEF_UINT64MAX == batch->GetColumn( COL_U64 ).GetUInt64Data()[ 2 ] );
            ASSERT_TRUE( 3.25f == batch->GetColumn( COL_F32 ).GetFloat32Data()[ 2 ] && 0.0f == batch->GetColumn( COL_F32 ).GetFloat32Data()[ 1 ] );
            ASSERT_TRUE( -125000 == batch->GetColumn( COL_DEC ).GetInt64Data()[ 0 ] && 4 == batch->GetColumn( COL_DEC ).GetDecimalScale() );
            ASSERT_TRUE( DaysOf( 1962, 1, 2 ) == batch->GetColumn( COL_D ).GetInt32Data()[ 0 ] );
            ASSERT_TRUE( tsTicks == batch->GetColumn( COL_TS ).GetInt64Data()[ 2 ] && DATATABLE_TIMEUNIT_MICROSECONDS == batch->GetColumn( COL_TS ).GetTimeUnit() );
            ASSERT_FALSE( batch->GetColumn( COL_TMS ).IsUtcAdjusted() );
            const Int64 msTicks = (Int64) DaysOf( 2026, 11, 3 ) * 86400000LL + ( 14LL * 3600 + 30 * 60 ) * 1000 + 123;
            ASSERT_TRUE( msTicks == batch->GetColumn( COL_TMS ).GetInt64Data()[ 0 ] );
            UInt32 length = 0;
            ASSERT_TRUE( 0 == memcmp( batch->GetColumn( COL_S ).GetVariableValue( 0, length ), "XNYS", 5 ) && 4 == length );
            ASSERT_TRUE( 0 == memcmp( batch->GetColumn( COL_S ).GetVariableValue( 2, length ), "h\xC3\xA9llo", 7 ) && 6 == length );
            ASSERT_TRUE( GUCEF_NULL != batch->GetColumn( COL_S ).GetVariableValue( 1, length ) && 0 == length );
            ASSERT_TRUE( 0 == memcmp( batch->GetColumn( COL_BIN ).GetVariableValue( 0, length ), "\x00\x01", 2 ) && 2 == length );

            // Tier 1
            CVariant cell;
            ASSERT_TRUE( batch->GetValue( 0, COL_B, cell ) && cell.AsBool() && batch->GetValue( 2, COL_B, cell ) && !cell.AsBool() );
            ASSERT_TRUE( batch->GetValue( 2, COL_U32, cell ) && GUCEF_UINT32MAX == cell.AsUInt32() );
            ASSERT_TRUE( batch->GetValue( 0, COL_DEC, cell ) && cell.AsDecimal().IsIdentical( D( "-12.5000" ) ) );
            ASSERT_TRUE( batch->GetValue( 2, COL_DEC, cell ) && cell.AsString() == "99999999999999.9999" );
            ASSERT_TRUE( batch->GetValue( 2, COL_D, cell ) && cell.AsString() == "9999-12-31" );
            ASSERT_TRUE( batch->GetValue( 0, COL_TS, cell ) && cell.AsString() == "1969-12-31T23:59:59.999999Z" );
            ASSERT_TRUE( batch->GetValue( 2, COL_TMS, cell ) && GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == cell.GetTypeId() && -86400000LL == cell.AsInt64() );
            ASSERT_TRUE( batch->GetValue( 2, COL_S, cell ) && cell.AsString() == "h\xC3\xA9llo" && cell.IsDynamicMemoryLinked() );
            ASSERT_TRUE( batch->GetValue( 2, COL_BIN, cell ) && 0 == cell.ByteSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: typed appends check the column type and range and refuse nulls in non-nullable columns" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableBatchBuilder builder( CORETEST::MakeAllTypesSchema( false ), pool );
            ASSERT_FALSE( builder.GetColumn( COL_I8 ).AppendNull() );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == builder.GetColumn( COL_I8 ).GetLastStatus() );
            ASSERT_FALSE( builder.GetColumn( COL_I8 ).AppendInt64( 128 ) || builder.GetColumn( COL_I8 ).AppendInt64( -129 ) );
            ASSERT_FALSE( builder.GetColumn( COL_I8 ).AppendUInt64( 200 ) );
            ASSERT_FALSE( builder.GetColumn( COL_U8 ).AppendInt64( -1 ) || builder.GetColumn( COL_U8 ).AppendInt64( 256 ) );
            ASSERT_FALSE( builder.GetColumn( COL_U16 ).AppendInt64( 65536 ) );
            ASSERT_FALSE( builder.GetColumn( COL_I32 ).AppendInt64( (Int64) GUCEF_INT32MAX + 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_U32 ).AppendUInt64( (UInt64) GUCEF_UINT32MAX + 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_I64 ).AppendUInt64( (UInt64) GUCEF_INT64MAX + 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_U64 ).AppendInt64( -1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_F32 ).AppendFloat64( 0.1 ) );                  // not exact in a float
            ASSERT_FALSE( builder.GetColumn( COL_F32 ).AppendFloat64( 1e40 ) );
            ASSERT_TRUE( builder.GetColumn( COL_F32 ).AppendFloat64( 1.0 / 0.0 ) );
            ASSERT_FALSE( builder.GetColumn( COL_I64 ).AppendFloat64( 1.0 ) );                  // typed appends do not convert
            ASSERT_FALSE( builder.GetColumn( COL_F64 ).AppendInt64( 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_B ).AppendInt64( 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_D ).AppendTimestamp( 1 ) || builder.GetColumn( COL_TS ).AppendDate( 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_D ).AppendDate( CDate( 2026, 2, 30 ) ) );
            ASSERT_FALSE( builder.GetColumn( COL_S ).AppendBinary( "x", 1 ) || builder.GetColumn( COL_BIN ).AppendUtf8( "x", 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_S ).AppendUtf8( GUCEF_NULL, 3 ) );
            ASSERT_TRUE( builder.GetColumn( COL_S ).AppendUtf8( GUCEF_NULL, 0 ) );
            ASSERT_FALSE( builder.GetColumn( COL_DEC ).AppendInt64( 1 ) );
            ASSERT_FALSE( builder.GetColumn( COL_TS ).AppendDateTime( CDateTime( CDate( 2026, 2, 30 ), CTime( 0, 0, 0, 0 ), 0 ) ) );

            // decimals: exact rescaling within the precision only
            CDataTableBatchBuilder decimals( OneColumn( CDataTableColumnDef::Decimal( "d", 6, 2 ) ), pool );
            CDataTableColumnBuilder& column = decimals.GetColumn( 0 );
            ASSERT_TRUE( column.AppendDecimal( D( "1.5" ) ) );
            ASSERT_TRUE( column.AppendDecimal( D( "1.2300000" ) ) );
            ASSERT_FALSE( column.AppendDecimal( D( "1.234" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == column.GetLastStatus() );
            ASSERT_TRUE( column.AppendDecimal( D( "-9999.99" ) ) );
            ASSERT_FALSE( column.AppendDecimal( D( "10000.00" ) ) );
            ASSERT_TRUE( column.AppendDecimalMantissa( 999999 ) );
            ASSERT_FALSE( column.AppendDecimalMantissa( 1000000 ) || column.AppendDecimalMantissa( -1000000 ) || column.AppendDecimalMantissa( GUCEF_INT64MIN ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == column.GetLastStatus() || DATATABLE_STATUS_TYPE_MISMATCH == column.GetLastStatus() );
            ASSERT_TRUE( 4 == column.GetRowCount() );
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == decimals.Build( batch, error ) );
            const Int64* mantissas = batch->GetColumn( 0 ).GetInt64Data();
            ASSERT_TRUE( 150 == mantissas[ 0 ] && 123 == mantissas[ 1 ] && -999999 == mantissas[ 2 ] && 999999 == mantissas[ 3 ] );

            // an index out of range gives a builder that takes nothing
            ASSERT_FALSE( builder.GetColumn( 1000 ).AppendInt64( 1 ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == builder.GetColumn( 1000 ).GetLastStatus() );
            ASSERT_TRUE( 0 == builder.GetColumn( 1000 ).GetRowCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Append( CVariant ) converts without loss and refuses everything else" )
        try
        {
            CVariant out;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            const CDataTableColumnDef b( "b", DATATABLE_TYPE_BOOL );
            const CDataTableColumnDef i8( "i8", DATATABLE_TYPE_INT8 );
            const CDataTableColumnDef i64( "i64", DATATABLE_TYPE_INT64 );
            const CDataTableColumnDef u64( "u64", DATATABLE_TYPE_UINT64 );
            const CDataTableColumnDef f32( "f32", DATATABLE_TYPE_FLOAT32 );
            const CDataTableColumnDef f64( "f64", DATATABLE_TYPE_FLOAT64 );
            const CDataTableColumnDef dec = CDataTableColumnDef::Decimal( "dec", 10, 3 );
            const CDataTableColumnDef d( "d", DATATABLE_TYPE_DATE );
            const CDataTableColumnDef ts = CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS );
            const CDataTableColumnDef s( "s", DATATABLE_TYPE_UTF8 );
            const CDataTableColumnDef bin( "bin", DATATABLE_TYPE_BINARY );

            // nulls
            ASSERT_TRUE( AppendsAs( i64, CVariant(), out ) && out.IsNULL() );
            CVariant nullValue;
            nullValue.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL );
            ASSERT_TRUE( AppendsAs( s, nullValue, out ) && out.IsNULL() );
            ASSERT_FALSE( AppendsAs( CDataTableColumnDef( "x", DATATABLE_TYPE_INT64, false ), nullValue, out, &status ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

            // booleans
            ASSERT_TRUE( AppendsAs( b, CVariant( true ), out ) && out.AsBool() );
            ASSERT_TRUE( AppendsAs( b, CVariant( (Int32) 0 ), out ) && !out.AsBool() );
            ASSERT_TRUE( AppendsAs( b, CVariant( (UInt8) 1 ), out ) && out.AsBool() );
            ASSERT_FALSE( AppendsAs( b, CVariant( (Int32) 2 ), out ) );
            ASSERT_FALSE( AppendsAs( b, CVariant( "true" ), out ) );

            // integers
            ASSERT_TRUE( AppendsAs( i8, CVariant( (Int64) -5 ), out ) && -5 == out.AsInt64() );
            ASSERT_FALSE( AppendsAs( i8, CVariant( (Int32) 300 ), out, &status ) && DATATABLE_STATUS_TYPE_MISMATCH == status );
            ASSERT_TRUE( AppendsAs( i64, CVariant( 7.0 ), out ) && 7 == out.AsInt64() );
            ASSERT_FALSE( AppendsAs( i64, CVariant( 7.5 ), out ) );
            ASSERT_FALSE( AppendsAs( i64, CVariant( 1e19 ), out ) );
            ASSERT_TRUE( AppendsAs( i64, CVariant( D( "12.000" ) ), out ) && 12 == out.AsInt64() );
            ASSERT_FALSE( AppendsAs( i64, CVariant( D( "12.5" ) ), out ) );
            ASSERT_FALSE( AppendsAs( i64, CVariant( "12" ), out ) );                          // text is never parsed
            ASSERT_FALSE( AppendsAs( i64, CDataTableValues::Date( 5 ), out ) );               // a date is no plain integer
            ASSERT_FALSE( AppendsAs( i64, CDataTableValues::Timestamp( 5, DATATABLE_TIMEUNIT_MILLISECONDS ), out ) );
            ASSERT_FALSE( AppendsAs( i64, CVariant( true ), out ) );
            ASSERT_TRUE( AppendsAs( u64, CVariant( GUCEF_UINT64MAX ), out ) && GUCEF_UINT64MAX == out.AsUInt64() );
            ASSERT_FALSE( AppendsAs( u64, CVariant( (Int64) -1 ), out ) );
            ASSERT_FALSE( AppendsAs( i64, CVariant( GUCEF_UINT64MAX ), out ) );

            // floats
            ASSERT_TRUE( AppendsAs( f64, CVariant( (Int64) 9007199254740992LL ), out ) && 9007199254740992.0 == out.AsFloat64() );
            ASSERT_FALSE( AppendsAs( f64, CVariant( (Int64) 9007199254740993LL ), out ) );     // not exact in a double
            ASSERT_TRUE( AppendsAs( f64, CVariant( (Float32) 0.1f ), out ) && (Float64) 0.1f == out.AsFloat64() );
            ASSERT_TRUE( AppendsAs( f32, CVariant( 0.5 ), out ) && 0.5f == out.AsFloat32() );
            ASSERT_FALSE( AppendsAs( f32, CVariant( 0.1 ), out ) );
            ASSERT_TRUE( AppendsAs( f32, CVariant( (Int32) 16777216 ), out ) );
            ASSERT_FALSE( AppendsAs( f32, CVariant( (Int32) 16777217 ), out ) );
            ASSERT_TRUE( AppendsAs( f64, CVariant( D( "0.5" ) ), out ) && 0.5 == out.AsFloat64() );
            ASSERT_FALSE( AppendsAs( f64, CVariant( D( "0.1" ) ), out ) );
            ASSERT_FALSE( AppendsAs( f64, CVariant( "0.5" ), out ) );

            // decimals
            ASSERT_TRUE( AppendsAs( dec, CVariant( D( "1.5" ) ), out ) && out.AsDecimal().IsIdentical( D( "1.500" ) ) );
            ASSERT_FALSE( AppendsAs( dec, CVariant( D( "1.5001" ) ), out ) );
            ASSERT_TRUE( AppendsAs( dec, CVariant( (Int32) -42 ), out ) && out.AsDecimal().IsIdentical( D( "-42.000" ) ) );
            ASSERT_FALSE( AppendsAs( dec, CVariant( (Int64) 10000000 ), out ) );                // 8 integer digits > 7
            ASSERT_FALSE( AppendsAs( dec, CVariant( 1.5 ), out ) );
            ASSERT_FALSE( AppendsAs( dec, CVariant( "1.5" ), out ) );

            // dates and timestamps
            ASSERT_TRUE( AppendsAs( d, CDataTableValues::Date( -10 ), out ) && -10 == out.AsInt32() );
            CVariant foreignDate;
            foreignDate.SetFromString( GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, "1962-01-02" );
            ASSERT_TRUE( AppendsAs( d, foreignDate, out ) && DaysOf( 1962, 1, 2 ) == out.AsInt32() );
            ASSERT_FALSE( AppendsAs( d, CVariant( (Int32) 3 ), out ) );
            ASSERT_FALSE( AppendsAs( d, CVariant( "2026-11-03" ), out ) );
            ASSERT_TRUE( AppendsAs( ts, CDataTableValues::Timestamp( -3, DATATABLE_TIMEUNIT_MILLISECONDS ), out ) && -3000 == out.AsInt64() );
            ASSERT_TRUE( AppendsAs( ts, CDataTableValues::Timestamp( 4000, DATATABLE_TIMEUNIT_NANOSECONDS ), out ) && 4 == out.AsInt64() );
            ASSERT_FALSE( AppendsAs( ts, CDataTableValues::Timestamp( 4001, DATATABLE_TIMEUNIT_NANOSECONDS ), out ) );
            ASSERT_TRUE( AppendsAs( ts, CDataTableValues::Date( 1 ), out ) && 86400000000LL == out.AsInt64() );
            CVariant seconds;
            seconds.SetFromString( GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, "-2" );
            ASSERT_TRUE( AppendsAs( ts, seconds, out ) && -2000000 == out.AsInt64() );
            ASSERT_FALSE( AppendsAs( ts, CVariant( (Int64) 5 ), out ) );

            // text and binary
            ASSERT_TRUE( AppendsAs( s, CVariant( "XNYS" ), out ) && out.AsString() == "XNYS" );
            ASSERT_TRUE( AppendsAs( s, CVariant( CUtf16String( "w\xC3\xA9" ) ), out ) && out.AsString() == "w\xC3\xA9" );
            ASSERT_FALSE( AppendsAs( s, CVariant( (Int32) 5 ), out ) );
            CVariant blob;
            blob.Set( "\x01\x02\x03", 3, GUCEF_DATATYPE_BINARY_BLOB );
            ASSERT_TRUE( AppendsAs( bin, blob, out ) && 3 == out.ByteSize() && 0 == memcmp( out.AsVoidPtr(), "\x01\x02\x03", 3 ) );
            ASSERT_TRUE( AppendsAs( bin, CVariant( "ab" ), out ) && 2 == out.ByteSize() );
            ASSERT_FALSE( AppendsAs( bin, CVariant( (Int32) 5 ), out ) );
            ASSERT_FALSE( AppendsAs( s, blob, out ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: bulk appends, with and without validity, all or nothing" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64 ) );
            schema.AddColumn( CDataTableColumnDef( "i8", DATATABLE_TYPE_INT8 ) );
            schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32 ) );
            schema.AddColumn( CDataTableColumnDef( "d", DATATABLE_TYPE_DATE ) );
            schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MILLISECONDS ) );
            schema.AddColumn( CDataTableColumnDef::Decimal( "dec", 4, 2 ) );
            CDataTableBatchBuilder builder( CreateDataTableSchemaPtr( schema ), pool );

            const Int64 values[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            const UInt8 validity[] = { 0xFD, 0x03 };              // value 1 null
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendInt64( -1 ) );
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendInt64s( values, 10 ) );
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendNull() );
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendInt64s( values, 10 ) );                  // direct copy with a bitmap in place
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendInt64s( values, 10, validity ) );
            ASSERT_TRUE( 32 == builder.GetColumn( 0 ).GetRowCount() && 2 == builder.GetColumn( 0 ).GetNullCount() );

            const Int32 small[] = { 1, 2, 300, 4 };
            ASSERT_TRUE( builder.GetColumn( 1 ).AppendInt32s( small, 2 ) );
            ASSERT_FALSE( builder.GetColumn( 1 ).AppendInt32s( small, 4 ) );                    // 300 does not fit
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == builder.GetColumn( 1 ).GetLastStatus() && 2 == builder.GetColumn( 1 ).GetRowCount() );
            const UInt8 thirdNull[] = { 0x0B };
            ASSERT_TRUE( builder.GetColumn( 1 ).AppendInt32s( small, 4, thirdNull ) && 6 == builder.GetColumn( 1 ).GetRowCount() && 1 == builder.GetColumn( 1 ).GetNullCount() );

            const Float64 floats[] = { 0.5, 0.25, 0.1 };
            ASSERT_FALSE( builder.GetColumn( 2 ).AppendFloat64s( floats, 3 ) );
            ASSERT_TRUE( 0 == builder.GetColumn( 2 ).GetRowCount() );
            const UInt8 lastNull[] = { 0x03 };
            ASSERT_TRUE( builder.GetColumn( 2 ).AppendFloat64s( floats, 3, lastNull ) && 3 == builder.GetColumn( 2 ).GetRowCount() );

            const Int32 days[] = { -1, 0, 1 };
            ASSERT_TRUE( builder.GetColumn( 3 ).AppendInt32s( days, 3 ) );
            ASSERT_TRUE( builder.GetColumn( 4 ).AppendInt64s( values, 3 ) );
            ASSERT_TRUE( builder.GetColumn( 5 ).AppendInt64s( values, 3 ) );
            const Int64 tooLarge[] = { 1, 10000 };
            ASSERT_FALSE( builder.GetColumn( 5 ).AppendInt64s( tooLarge, 2 ) );
            ASSERT_TRUE( 3 == builder.GetColumn( 5 ).GetRowCount() );

            // row counts differ: no batch
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == builder.Build( batch, error ) && batch.IsNULL() );
            ASSERT_TRUE( error.HasSubstr( "rows" ) >= 0 && DATATABLE_STATUS_INVALID_ARGUMENT == builder.GetLastStatus() );
            ASSERT_TRUE( 3 == builder.GetRowCount() );

            CDataTableBatchBuilder single( OneColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64 ) ), pool );
            single.GetColumn( 0 ).AppendInt64s( values, 10 );
            single.GetColumn( 0 ).AppendNull();
            single.GetColumn( 0 ).AppendInt64s( values, 10, validity );
            ASSERT_TRUE( DATATABLE_STATUS_OK == single.Build( batch, error ) );
            const CDataTableColumnBatch& column = batch->GetColumn( 0 );
            ASSERT_TRUE( 21 == column.GetRowCount() && 2 == column.GetNullCount() && column.Validate( error ) );
            ASSERT_TRUE( 10 == column.GetInt64Data()[ 9 ] && !column.IsValid( 10 ) && 0 == column.GetInt64Data()[ 10 ] );
            ASSERT_TRUE( !column.IsValid( 12 ) && 0 == column.GetInt64Data()[ 12 ] && column.IsValid( 13 ) && 3 == column.GetInt64Data()[ 13 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: AppendRow appends a whole row or nothing" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT32, false ) );
            schema.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8 ) );
            schema.AddColumn( CDataTableColumnDef( "ok", DATATABLE_TYPE_BOOL ) );
            CDataTableBatchBuilder builder( CreateDataTableSchemaPtr( schema ), pool );

            CVariant row[ 3 ];
            row[ 0 ] = (Int32) 1;
            row[ 1 ] = CVariant( "first" );
            row[ 2 ] = true;
            ASSERT_TRUE( builder.AppendRow( row, 3 ) );
            row[ 0 ] = (Int32) 2;
            row[ 1 ].Clear();
            row[ 2 ] = (Int32) 7;                                // not a boolean: the row fails at the last column
            ASSERT_FALSE( builder.AppendRow( row, 3 ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == builder.GetLastStatus() );
            ASSERT_TRUE( 1 == builder.GetRowCount() && 1 == builder.GetColumn( 1 ).GetRowCount() && 0 == builder.GetColumn( 1 ).GetNullCount() );
            ASSERT_FALSE( builder.AppendRow( row, 2 ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == builder.GetLastStatus() );
            row[ 2 ] = false;
            ASSERT_TRUE( builder.AppendRow( row, 3 ) && DATATABLE_STATUS_OK == builder.GetLastStatus() );
            row[ 0 ].Clear();                                    // null id: not nullable
            ASSERT_FALSE( builder.AppendRow( row, 3 ) );

            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) && 2 == batch->GetRowCount() );
            CVariant cell;
            ASSERT_TRUE( batch->GetValue( 1, 0, cell ) && 2 == cell.AsInt32() );
            ASSERT_TRUE( batch->GetValue( 1, 1, cell ) && cell.IsNULL() );
            ASSERT_TRUE( batch->GetValue( 0, 1, cell ) && cell.AsString() == "first" );
            for ( UInt32 c=0; c<3; ++c )
                ASSERT_TRUE( batch->GetColumn( c ).Validate( error ) );

            // Reset drops rows
            builder.AppendRow( row, 3 );
            row[ 0 ] = (Int32) 3;
            builder.AppendRow( row, 3 );
            ASSERT_TRUE( 1 == builder.GetRowCount() );
            builder.Reset();
            ASSERT_TRUE( 0 == builder.GetRowCount() && 0 == builder.GetColumn( 1 ).GetNullCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: edge sizes: empty batches, one row, all-null columns, the largest batch" )
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = CORETEST::MakeAllTypesSchema();
            CDataTableBatchBuilder builder( schema, pool );
            CString error;

            // empty
            CDataTableBatchPtr empty;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( empty, error ) );
            ASSERT_TRUE( 0 == empty->GetRowCount() && COL_COUNT == empty->GetColumnCount() && 0 == empty->GetSelectedRowCount() );
            for ( UInt32 c=0; c<COL_COUNT; ++c )
            {
                const CDataTableColumnBatch& column = empty->GetColumn( c );
                ASSERT_TRUE( 0 == column.GetRowCount() && column.GetLogicalType() == schema->GetColumn( c ).type && column.Validate( error ) );
                ASSERT_TRUE( DATATABLE_STORAGE_NONE != column.GetStorage() );
                CVariant cell;
                ASSERT_FALSE( column.GetValue( 0, cell ) );
            }
            ASSERT_TRUE( GUCEF_NULL != empty->GetColumn( COL_S ).GetOffsets() && 0 == empty->GetColumn( COL_S ).GetOffsets()[ 0 ] );

            // one row and all nulls
            for ( UInt32 c=0; c<COL_COUNT; ++c )
                builder.GetColumn( c ).AppendNull();
            CDataTableBatchPtr oneRow;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( oneRow, error ) && 1 == oneRow->GetRowCount() );
            for ( UInt32 c=0; c<COL_COUNT; ++c )
            {
                const CDataTableColumnBatch& column = oneRow->GetColumn( c );
                ASSERT_TRUE( 1 == column.GetNullCount() && GUCEF_NULL != column.GetValidity() && column.Validate( error ) );
                CVariant cell;
                ASSERT_TRUE( column.GetValue( 0, cell ) && cell.IsNULL() );
            }
            ASSERT_TRUE( 0 == oneRow->GetColumn( COL_I64 ).GetInt64Data()[ 0 ] && 0 == ( oneRow->GetColumn( COL_B ).GetBoolBits()[ 0 ] & 1 ) );
            ASSERT_TRUE( 1 == oneRow->GetColumn( COL_S ).GetOffsets()[ 1 ] );

            // an all-null column next to a column with values over several bitmap bytes
            CDataTableSchema twoColumns;
            twoColumns.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_FLOAT64 ) );
            twoColumns.AddColumn( CDataTableColumnDef( "n", DATATABLE_TYPE_UTF8 ) );
            CDataTableBatchBuilder nulls( CreateDataTableSchemaPtr( twoColumns ), pool );
            for ( UInt32 i=0; i<100; ++i )
            {
                nulls.GetColumn( 0 ).AppendFloat64( i );
                nulls.GetColumn( 1 ).AppendNull();
            }
            CDataTableBatchPtr allNull;
            ASSERT_TRUE( DATATABLE_STATUS_OK == nulls.Build( allNull, error ) );
            ASSERT_TRUE( 100 == allNull->GetColumn( 1 ).GetNullCount() && allNull->GetColumn( 1 ).Validate( error ) );
            ASSERT_TRUE( GUCEF_NULL == allNull->GetColumn( 0 ).GetValidity() && 99.0 == allNull->GetColumn( 0 ).GetFloat64Data()[ 99 ] );
            const UInt8* bits = allNull->GetColumn( 1 ).GetValidity();
            bool allClear = true;
            for ( UInt32 i=0; i<13; ++i )
                allClear = allClear && 0 == bits[ i ];
            ASSERT_TRUE( allClear );

            // the largest batch of a scan: 1048576 rows
            CDataTableSchema large;
            large.AddColumn( CDataTableColumnDef( "i", DATATABLE_TYPE_INT64 ) );
            large.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_BOOL ) );
            large.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
            CDataTableBatchBuilder largeBuilder( CreateDataTableSchemaPtr( large ), pool );
            for ( UInt32 i=0; i<DataTableMaxBatchRowCount; ++i )
            {
                largeBuilder.GetColumn( 0 ).AppendInt64( (Int64) i * 3 );
                largeBuilder.GetColumn( 1 ).AppendBool( 0 == ( i % 3 ) );
                if ( 0 == ( i % 1000 ) )
                    largeBuilder.GetColumn( 2 ).AppendNull();
                else
                    largeBuilder.GetColumn( 2 ).AppendUtf8( "abc", 1 + ( i % 3 ) );
            }
            CDataTableBatchPtr largest;
            ASSERT_TRUE( DATATABLE_STATUS_OK == largeBuilder.Build( largest, error ) );
            ASSERT_TRUE( DataTableMaxBatchRowCount == largest->GetRowCount() );
            const Int64* ints = largest->GetColumn( 0 ).GetInt64Data();
            ASSERT_TRUE( 0 == ints[ 0 ] && (Int64) ( DataTableMaxBatchRowCount - 1 ) * 3 == ints[ DataTableMaxBatchRowCount - 1 ] );
            const UInt8* boolBits = largest->GetColumn( 1 ).GetBoolBits();
            ASSERT_TRUE( 0 != ( boolBits[ 0 ] & 1 ) && 0 == ( boolBits[ 0 ] & 2 ) && 0 != ( boolBits[ 0 ] & 8 ) );
            ASSERT_TRUE( 1049 == largest->GetColumn( 2 ).GetNullCount() );
            for ( UInt32 c=0; c<3; ++c )
                ASSERT_TRUE( largest->GetColumn( c ).Validate( error ) );
            UInt32 length = 0;
            ASSERT_TRUE( 0 == memcmp( largest->GetColumn( 2 ).GetVariableValue( DataTableMaxBatchRowCount - 1, length ), "abc", 1 ) && 1 == length );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a reused builder over a warm pool builds batches without a heap allocation" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            CDataTableBufferPool pool;
            CDataTableSchemaPtr schema = CORETEST::MakeAllTypesSchema();
            CDataTableBatchBuilder builder( schema, pool );
            const CString text = "XNYS";
            CVariant cell;
            UInt32 rounds = 0;
            UInt32 allocations = 0;
            CDataTableBufferPoolStats before, after;
            for ( UInt32 round=0; round<5; ++round )
            {
                if ( 2 == round )
                {
                    pool.GetStats( before );
                    CORETEST::StartAllocationCounting();
                }
                for ( UInt32 i=0; i<2000; ++i )
                {
                    builder.GetColumn( COL_B ).AppendBool( 0 == ( i & 1 ) );
                    builder.GetColumn( COL_I8 ).AppendInt64( (Int64) ( i % 100 ) );
                    builder.GetColumn( COL_I16 ).AppendInt64( i );
                    builder.GetColumn( COL_I32 ).AppendInt64( i );
                    builder.GetColumn( COL_I64 ).AppendInt64( i );
                    builder.GetColumn( COL_U8 ).AppendNull();
                    builder.GetColumn( COL_U16 ).AppendUInt64( i );
                    builder.GetColumn( COL_U32 ).AppendUInt64( i );
                    builder.GetColumn( COL_U64 ).AppendUInt64( i );
                    builder.GetColumn( COL_F32 ).AppendFloat64( 0.5 );
                    builder.GetColumn( COL_F64 ).AppendFloat64( i * 0.25 );
                    builder.GetColumn( COL_DEC ).AppendDecimalMantissa( i );
                    builder.GetColumn( COL_D ).AppendDate( (Int32) i );
                    builder.GetColumn( COL_TS ).AppendTimestamp( i );
                    builder.GetColumn( COL_TMS ).AppendTimestamp( i );
                    builder.GetColumn( COL_TNS ).AppendTimestamp( i );
                    builder.GetColumn( COL_S ).AppendString( text );
                    builder.GetColumn( COL_BIN ).AppendBinary( "abc", 3 );
                }
                CDataTableBatchPtr batch;
                CString error;
                builder.Build( batch, error );
                for ( UInt32 row=0; row<batch->GetRowCount(); row+=97 )
                {
                    for ( UInt32 c=0; c<COL_COUNT; ++c )
                        batch->GetValue( row, c, cell );
                }
                cell.Clear();
                ++rounds;
            }
            allocations = CORETEST::StopAllocationCounting();
            pool.GetStats( after );
            if ( allocations > 0 )
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Builder reuse allocated " << allocations << " times:\n" << CORETEST::GetCountedAllocationStacks() );
            ASSERT_TRUE( 5 == rounds && 0 == allocations );
            ASSERT_TRUE( before.misses == after.misses && before.batchMisses == after.batchMisses );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: the row cursor visits the selected rows of one batch or of a batch source" )
        try
        {
            CDataTableBufferPool pool;

            // one batch, with a selection
            CDataTableBatchPtr batch = MakeIntBatch( pool, 10, 5 );
            CDataTableBufferPtr selection;
            pool.Acquire( 8, selection );
            static_cast< UInt32* >( selection->GetData() )[ 0 ] = 1;
            static_cast< UInt32* >( selection->GetData() )[ 1 ] = 4;
            batch.GetWritable()->SetSelection( static_cast< const UInt32* >( selection->GetData() ), 2, selection );
            CDataTableRowCursor single( batch );
            CVariant cell;
            ASSERT_FALSE( single.GetValue( 0, cell ) );
            ASSERT_TRUE( single.GetCurrentBatch().IsNULL() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == single.Next() && 1 == single.GetCurrentRow() && single.GetValue( 0, cell ) && 11 == cell.AsInt32() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == single.Next() && 4 == single.GetCurrentRow() && single.GetValue( 0, cell ) && 14 == cell.AsInt32() );
            ASSERT_TRUE( DATATABLE_STATUS_END == single.Next() && DATATABLE_STATUS_END == single.Next() );
            ASSERT_FALSE( single.GetValue( 0, cell ) );
            ASSERT_TRUE( 2 == single.GetRowsVisited() && single.GetLastErrorMessage().IsNULLOrEmpty() );

            // a source of several batches, one of them empty
            CTestBatchSource* source = GUCEF_NEW CTestBatchSource();
            source->batches.push_back( MakeIntBatch( pool, 0, 3 ) );
            source->batches.push_back( MakeIntBatch( pool, 100, 0 ) );
            source->batches.push_back( MakeIntBatch( pool, 3, 2 ) );
            CDataTableBatchSourcePtr sourcePtr( source );
            CDataTableRowCursor cursor( sourcePtr );
            Int32 expected = 0;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            while ( DATATABLE_STATUS_OK == ( status = cursor.Next() ) )
            {
                ASSERT_TRUE( cursor.GetValue( 0, cell ) && expected == cell.AsInt32() );
                ASSERT_FALSE( cursor.GetValue( 1, cell ) );
                ++expected;
            }
            ASSERT_TRUE( DATATABLE_STATUS_END == status && 5 == expected && 5 == cursor.GetRowsVisited() );

            // a failing source ends the cursor with its status and message
            CTestBatchSource* failing = GUCEF_NEW CTestBatchSource();
            failing->batches.push_back( MakeIntBatch( pool, 0, 1 ) );
            failing->endStatus = DATATABLE_STATUS_IO_ERROR;
            CDataTableRowCursor failingCursor( ( CDataTableBatchSourcePtr( failing ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == failingCursor.Next() );
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == failingCursor.Next() );
            ASSERT_TRUE( failingCursor.GetLastErrorMessage() == "the test source failed" );
            ASSERT_TRUE( DATATABLE_STATUS_END == failingCursor.Next() );

            // the cursor holds its batch: a linked value stays valid until the next move
            CDataTableBatchBuilder textBuilder( OneColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) ), pool );
            textBuilder.GetColumn( 0 ).AppendString( "kept" );
            CDataTableBatchPtr textBatch;
            CString error;
            textBuilder.Build( textBatch, error );
            CDataTableRowCursor textCursor( textBatch );
            textBatch.Reset();
            ASSERT_TRUE( DATATABLE_STATUS_OK == textCursor.Next() && textCursor.GetValue( 0, cell ) && cell.IsDynamicMemoryLinked() );
            ASSERT_TRUE( cell.AsString() == "kept" && 1 == textCursor.GetCurrentBatch()->GetReferenceCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
