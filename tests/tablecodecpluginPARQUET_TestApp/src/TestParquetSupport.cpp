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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#include "tablecodecpluginPARQUET_CParquetTableCodec.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H ? */

#include "TestParquetSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace PARQUETTEST {

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool g_pluginsLoaded = false;

bool
AddPlugin( const CString& name )
{
    #ifdef GUCEF_DEBUG_MODE
    const CString first = "$MODULEDIR$/" + name + "_d";
    const CString second = "$MODULEDIR$/" + name;
    #else
    const CString first = "$MODULEDIR$/" + name;
    const CString second = "$MODULEDIR$/" + name + "_d";
    #endif
    CPluginControl& pluginControl = CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( first, "ParquetTestPlugins", true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, "ParquetTestPlugins", true );
}

}

/*-------------------------------------------------------------------------*/

void
LoadRuntimePlugins( void )
{
    if ( g_pluginsLoaded )
        return;
    g_pluginsLoaded = true;
    AddPlugin( "codecspluginZLIB" );
    AddPlugin( "dstorepluginJSONPARSER" );
}

/*-------------------------------------------------------------------------*/

bool
IsGzipAvailable( void )
{
    return !CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CoreCodecTypes::CompressionCodec, "gzip" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
IsJsonAvailable( void )
{
    CDStoreCodecRegistry::TDStoreCodecPtr json;
    return CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) && !json.IsNULL();
}

/*-------------------------------------------------------------------------*/

CFileDataTable::TTableCodecPtr
GetParquetCodec( void )
{
    static CFileDataTable::TTableCodecPtr codec;
    if ( codec.IsNULL() )
    {
        CTSharedPtr< CITableCodec, MT::CMutex > made( GUCEF_NEW TABLECODECPLUGIN::PARQUET::CParquetTableCodec() );
        codec = made;
    }
    return codec;
}

/*-------------------------------------------------------------------------*/

IOAccessPtr
MakeReadAccess( const CDynamicBuffer& data )
{
    return IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( data ) );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
WriteParquet( const CDataTableSchemaPtr& schema, const TBatchVector& batches, const CDataTableWriterOptions& options,
              CDynamicBuffer& file, CString& error, bool flushEach, CDataTableFileSummary* summary )
{
    file.Clear( true );
    IOAccessPtr output( GUCEF_NEW CDynamicBufferAccess( file, CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC ) );
    CITableWriterPtr writer;
    TDataTableStatus status = GetParquetCodec()->CreateWriter( output, schema, options, writer, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    for ( size_t i=0; i<batches.size() && DATATABLE_STATUS_OK == status; ++i )
    {
        status = writer->WriteBatch( *batches[ i ] );
        if ( DATATABLE_STATUS_OK == status && flushEach )
            status = writer->FlushRowGroup();
    }
    CDataTableFileSummary localSummary;
    if ( DATATABLE_STATUS_OK == status )
        status = writer->Finish( GUCEF_NULL != summary ? *summary : localSummary );
    if ( DATATABLE_STATUS_OK != status )
        error = writer->GetLastErrorMessage();
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
ReadParquet( const CDynamicBuffer& file, TBatchVector& batches, CString& error, const CDataTableSchema::TColumnIndexVector* projection,
             UInt32 batchRowCount, CDataTableScanStats* stats, bool materializeDictionaries, const CValueList* readOptions )
{
    batches.clear();
    IOAccessPtr input = MakeReadAccess( file );
    CValueList options;
    if ( GUCEF_NULL != readOptions )
        options = *readOptions;
    CITableFileMetaDataPtr metaData;
    TDataTableStatus status = GetParquetCodec()->ReadMetaData( *input, options, metaData, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CITableReaderPtr reader;
    status = GetParquetCodec()->CreateReader( input, metaData, options, reader, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableReadRequest request;
    if ( GUCEF_NULL != projection )
        request.projection = *projection;
    request.batchRowCount = batchRowCount;
    request.materializeDictionaries = materializeDictionaries;
    request.stats = stats;
    for ( UInt32 g=0; g<metaData->GetRowGroupCount(); ++g )
    {
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
            batches.push_back( batch );
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CDataTableSchema
MakeAllTypesSchema( bool nullable )
{
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_BOOL, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i8", DATATABLE_TYPE_INT8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i16", DATATABLE_TYPE_INT16, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i32", DATATABLE_TYPE_INT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u8", DATATABLE_TYPE_UINT8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u16", DATATABLE_TYPE_UINT16, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u32", DATATABLE_TYPE_UINT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u64", DATATABLE_TYPE_UINT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "dec", 18, 6, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "dec4", 4, 2, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "tsms", DATATABLE_TIMEUNIT_MILLISECONDS, true, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "tsus", DATATABLE_TIMEUNIT_MICROSECONDS, false, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "tsns", DATATABLE_TIMEUNIT_NANOSECONDS, true, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "text", DATATABLE_TYPE_UTF8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "bin", DATATABLE_TYPE_BINARY, nullable ) );
    return schema;
}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr
MakeRandomBatch( const CDataTableSchemaPtr& schema, UInt32 rows, UInt32 nullPercent, CTestRandom& random, UInt32 distinctTexts )
{
    CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
    static const char* const words[] = { "", "a", "XNYS", "KO", "\xC3\xA9t\xC3\xA9", "the quick brown fox", "zz", "AAPL", "MSFT", "\xE2\x82\xAC" };
    for ( UInt32 r=0; r<rows; ++r )
    {
        for ( UInt32 c=0; c<schema->GetColumnCount(); ++c )
        {
            const CDataTableColumnDef& def = schema->GetColumn( c );
            CDataTableColumnBuilder& column = builder.GetColumn( c );
            if ( def.nullable && random.Below( 100 ) < nullPercent )
            {
                column.AppendNull();
                continue;
            }
            const UInt64 bits = random.Next();
            switch ( def.type )
            {
                case DATATABLE_TYPE_BOOL: { column.AppendBool( 0 != ( bits & 1 ) ); break; }
                case DATATABLE_TYPE_INT8: { column.AppendInt64( (Int8) bits ); break; }
                case DATATABLE_TYPE_INT16: { column.AppendInt64( (Int16) bits ); break; }
                case DATATABLE_TYPE_INT32: { column.AppendInt64( (Int32) bits ); break; }
                case DATATABLE_TYPE_INT64: { column.AppendInt64( (Int64) bits ); break; }
                case DATATABLE_TYPE_UINT8: { column.AppendUInt64( (UInt8) bits ); break; }
                case DATATABLE_TYPE_UINT16: { column.AppendUInt64( (UInt16) bits ); break; }
                case DATATABLE_TYPE_UINT32: { column.AppendUInt64( (UInt32) bits ); break; }
                case DATATABLE_TYPE_UINT64: { column.AppendUInt64( bits ); break; }
                case DATATABLE_TYPE_FLOAT32:
                {
                    // ordinary values mostly, now and then the special ones
                    const UInt32 pick = random.Below( 20 );
                    Float32 v = (Float32) ( (Int32) ( bits & 0xFFFFF ) - 500000 ) / 64.0f;
                    if ( 0 == pick ) v = 0.0f; else if ( 1 == pick ) v = -0.0f;
                    column.AppendFloat64( (Float64) v );
                    break;
                }
                case DATATABLE_TYPE_FLOAT64:
                {
                    const UInt32 pick = random.Below( 25 );
                    Float64 v = (Float64) (Int64) ( bits >> 11 ) / 1048576.0 - 1.0e9;
                    if ( 0 == pick ) { UInt64 nan = 0x7FF8000000000001ULL; memcpy( &v, &nan, 8 ); }
                    else if ( 1 == pick ) v = -0.0;
                    else if ( 2 == pick ) v = 1.0 / 0.0 * ( ( bits & 2 ) ? 1 : -1 );
                    column.AppendFloat64( v );
                    break;
                }
                case DATATABLE_TYPE_DECIMAL:
                {
                    Int64 limit = 1;
                    for ( UInt8 d=0; d<def.decimalPrecision; ++d ) limit *= 10;
                    column.AppendDecimalMantissa( (Int64) ( bits % (UInt64) ( 2 * limit - 1 ) ) - ( limit - 1 ) );
                    break;
                }
                case DATATABLE_TYPE_DATE: { column.AppendDate( (Int32) ( bits % 200000 ) - 100000 ); break; }
                case DATATABLE_TYPE_TIMESTAMP: { column.AppendTimestamp( (Int64) ( bits % 4000000000000000ULL ) - 2000000000000000LL ); break; }
                case DATATABLE_TYPE_UTF8:
                {
                    if ( 0 != distinctTexts )
                    {
                        const CString text = "v" + ToString( (UInt32) ( bits % distinctTexts ) );
                        column.AppendString( text );
                    }
                    else
                    {
                        const char* word = words[ bits % 10 ];
                        column.AppendUtf8( word, (UInt32) strlen( word ) );
                    }
                    break;
                }
                case DATATABLE_TYPE_BINARY:
                {
                    UInt8 blob[ 24 ];
                    const UInt32 length = (UInt32) ( ( bits >> 8 ) % 25 );
                    for ( UInt32 i=0; i<length; ++i )
                        blob[ i ] = (UInt8) ( random.Next() & ( 0 == distinctTexts ? 0xFF : 0x01 ) );
                    column.AppendBinary( blob, length );
                    break;
                }
                default: { column.AppendNull(); break; }
            }
        }
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/*-------------------------------------------------------------------------*/

UInt64
CountRows( const TBatchVector& batches )
{
    UInt64 rows = 0;
    for ( size_t i=0; i<batches.size(); ++i )
        rows += batches[ i ]->GetRowCount();
    return rows;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Whether two cells are the same: type, null, value; floats and doubles by their bits */
bool
SameCell( const CDataTableColumnBatch& a, UInt32 ra, const CDataTableColumnBatch& b, UInt32 rb )
{
    if ( a.GetLogicalType() != b.GetLogicalType() )
        return false;
    if ( a.IsValid( ra ) != b.IsValid( rb ) )
        return false;
    if ( !a.IsValid( ra ) )
        return true;
    if ( DATATABLE_TYPE_FLOAT32 == a.GetLogicalType() )
        return 0 == memcmp( a.GetFloat32Data() + ra, b.GetFloat32Data() + rb, 4 );
    if ( DATATABLE_TYPE_FLOAT64 == a.GetLogicalType() )
        return 0 == memcmp( a.GetFloat64Data() + ra, b.GetFloat64Data() + rb, 8 );
    if ( DATATABLE_TYPE_UTF8 == a.GetLogicalType() || DATATABLE_TYPE_BINARY == a.GetLogicalType() )
    {
        UInt32 la = 0, lb = 0;
        const char* va = a.GetVariableValue( ra, la );
        const char* vb = b.GetVariableValue( rb, lb );
        return la == lb && ( 0 == la || 0 == memcmp( va, vb, la ) );
    }
    return 0 == CDataTableMerge::CompareCells( a, ra, b, rb );
}

}

/*-------------------------------------------------------------------------*/

bool
SameRows( const TBatchVector& expected, const TBatchVector& actual, CString& difference, const CDataTableSchema::TColumnIndexVector* columnMap )
{
    const UInt64 expectedRows = CountRows( expected );
    const UInt64 actualRows = CountRows( actual );
    if ( expectedRows != actualRows )
    {
        difference = "row counts differ: expected " + ToString( expectedRows ) + ", got " + ToString( actualRows );
        return false;
    }
    size_t eb = 0, ab = 0;
    UInt32 er = 0, ar = 0;
    UInt64 row = 0;
    while ( row < expectedRows )
    {
        while ( er >= expected[ eb ]->GetRowCount() ) { ++eb; er = 0; }
        while ( ar >= actual[ ab ]->GetRowCount() ) { ++ab; ar = 0; }
        const CDataTableBatch& e = *expected[ eb ];
        const CDataTableBatch& a = *actual[ ab ];
        for ( UInt32 c=0; c<a.GetColumnCount(); ++c )
        {
            const UInt32 ec = ( GUCEF_NULL != columnMap && c < columnMap->size() ) ? (*columnMap)[ c ] : c;
            if ( !SameCell( e.GetColumn( ec ), er, a.GetColumn( c ), ar ) )
            {
                CVariant ev, av;
                e.GetValue( er, ec, ev, false );
                a.GetValue( ar, c, av, false );
                difference = "row " + ToString( row ) + " column " + ToString( c ) + ": expected \"" + ev.AsString() + "\", got \"" + av.AsString() + "\"";
                return false;
            }
        }
        ++er;
        ++ar;
        ++row;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
DumpRows( const TBatchVector& batches, const CDataTableSchema& schema )
{
    GUCEF::vector< char > text;
    char cell[ 64 ];
    for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
    {
        if ( c > 0 )
            text.push_back( '\t' );
        const CString& name = schema.GetColumn( c ).name;
        text.insert( text.end(), name.C_String(), name.C_String() + strlen( name.C_String() ) );
    }
    text.push_back( '\n' );
    for ( size_t b=0; b<batches.size(); ++b )
    {
        const CDataTableBatch& batch = *batches[ b ];
        for ( UInt32 r=0; r<batch.GetRowCount(); ++r )
        {
            for ( UInt32 c=0; c<batch.GetColumnCount(); ++c )
            {
                if ( c > 0 )
                    text.push_back( '\t' );
                const CDataTableColumnBatch& column = batch.GetColumn( c );
                cell[ 0 ] = '\0';
                if ( !column.IsValid( r ) )
                {
                    strcpy( cell, "N" );
                }
                else
                {
                    switch ( column.GetLogicalType() )
                    {
                        case DATATABLE_TYPE_BOOL: { strcpy( cell, 0 != ( column.GetBoolBits()[ r >> 3 ] & ( 1u << ( r & 7 ) ) ) ? "1" : "0" ); break; }
                        case DATATABLE_TYPE_INT8: { snprintf( cell, sizeof( cell ), "%d", (int) column.GetInt8Data()[ r ] ); break; }
                        case DATATABLE_TYPE_INT16: { snprintf( cell, sizeof( cell ), "%d", (int) column.GetInt16Data()[ r ] ); break; }
                        case DATATABLE_TYPE_INT32:
                        case DATATABLE_TYPE_DATE: { snprintf( cell, sizeof( cell ), "%d", (int) column.GetInt32Data()[ r ] ); break; }
                        case DATATABLE_TYPE_INT64:
                        case DATATABLE_TYPE_DECIMAL:
                        case DATATABLE_TYPE_TIMESTAMP: { snprintf( cell, sizeof( cell ), "%lld", (long long) column.GetInt64Data()[ r ] ); break; }
                        case DATATABLE_TYPE_UINT8: { snprintf( cell, sizeof( cell ), "%u", (unsigned) column.GetUInt8Data()[ r ] ); break; }
                        case DATATABLE_TYPE_UINT16: { snprintf( cell, sizeof( cell ), "%u", (unsigned) column.GetUInt16Data()[ r ] ); break; }
                        case DATATABLE_TYPE_UINT32: { snprintf( cell, sizeof( cell ), "%u", (unsigned) column.GetUInt32Data()[ r ] ); break; }
                        case DATATABLE_TYPE_UINT64: { snprintf( cell, sizeof( cell ), "%llu", (unsigned long long) column.GetUInt64Data()[ r ] ); break; }
                        case DATATABLE_TYPE_FLOAT32:
                        {
                            const Float32 v = column.GetFloat32Data()[ r ];
                            UInt32 bits = 0;
                            memcpy( &bits, &v, 4 );
                            if ( v != v ) strcpy( cell, "nan" ); else snprintf( cell, sizeof( cell ), "f%08x", (unsigned) bits );
                            break;
                        }
                        case DATATABLE_TYPE_FLOAT64:
                        {
                            const Float64 v = column.GetFloat64Data()[ r ];
                            UInt64 bits = 0;
                            memcpy( &bits, &v, 8 );
                            if ( v != v ) strcpy( cell, "nan" ); else snprintf( cell, sizeof( cell ), "d%016llx", (unsigned long long) bits );
                            break;
                        }
                        case DATATABLE_TYPE_UTF8:
                        case DATATABLE_TYPE_BINARY:
                        {
                            UInt32 length = 0;
                            const UInt8* value = reinterpret_cast< const UInt8* >( column.GetVariableValue( r, length ) );
                            text.push_back( 'x' );
                            static const char* const hex = "0123456789abcdef";
                            for ( UInt32 i=0; i<length; ++i )
                            {
                                text.push_back( hex[ value[ i ] >> 4 ] );
                                text.push_back( hex[ value[ i ] & 15 ] );
                            }
                            break;
                        }
                        default: { strcpy( cell, "?" ); break; }
                    }
                }
                text.insert( text.end(), cell, cell + strlen( cell ) );
            }
            text.push_back( '\n' );
        }
    }
    return text.empty() ? CString::Empty : CString( &text[ 0 ], (UInt32) text.size() );
}

/*-------------------------------------------------------------------------*/

CString
TestFilePath( const CString& name )
{
    const CString dir = CombinePath( TempDir(), "tablecodecpluginPARQUET_TestApp" );
    CreateDirs( dir );
    const CString path = CombinePath( dir, name );
    if ( FileExists( path ) )
        DeleteFile( path );
    return path;
}

/*-------------------------------------------------------------------------*/

}; /* namespace PARQUETTEST */

/*-------------------------------------------------------------------------*/
