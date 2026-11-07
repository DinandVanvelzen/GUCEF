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

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#include "tablecodecpluginPARQUET_CParquetSchemaMapping.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 MaxSchemaDepth = 32;
const Int32 MaxFixedLength = 16 * 1024 * 1024;
const Int64 JulianDayOfUnixEpoch = 2440588;
const Int64 NanosecondsPerDay = 86400LL * 1000000000LL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SGroupLevel
{
    Int32 remainingChildren;
    UInt32 definitionLevel;
    UInt32 repetitionLevel;
    CORE::CString prefix;
};

typedef GUCEF::vector< SGroupLevel > TGroupLevelVector;

bool
Unsupported( SParquetColumnInfo& info, const CORE::CString& reason )
{
    info.supported = false;
    info.unsupportedReason = reason;
    info.conversion = PARQUET_CONVERT_NONE;
    info.storageWidth = 0;
    info.column.type = CORE::DATATABLE_TYPE_UNSUPPORTED;
    return false;
}

bool
SetDecimal( SParquetColumnInfo& info, Int32 precision, Int32 scale, Int32 maxPrecision, EParquetValueConversion conversion )
{
    if ( precision < 1 || scale < 0 || scale > precision )
        return Unsupported( info, "decimal(" + CORE::ToString( precision ) + "," + CORE::ToString( scale ) + ") is not a valid decimal" );
    if ( precision > (Int32) CORE::DataTableMaxDecimalPrecision )
        return Unsupported( info, "decimal precision " + CORE::ToString( precision ) + " is above 18 digits" );
    if ( precision > maxPrecision )
        return Unsupported( info, "decimal precision " + CORE::ToString( precision ) + " does not fit the physical type" );
    info.column.type = CORE::DATATABLE_TYPE_DECIMAL;
    info.column.decimalPrecision = (UInt8) precision;
    info.column.decimalScale = (UInt8) scale;
    info.conversion = conversion;
    info.storageWidth = 8;
    return true;
}

bool
SetInteger( SParquetColumnInfo& info, Int32 physical, Int32 bitWidth, bool isSigned )
{
    if ( PARQUET_TYPE_INT32 == physical )
    {
        switch ( bitWidth )
        {
            case 8: { info.column.type = isSigned ? CORE::DATATABLE_TYPE_INT8 : CORE::DATATABLE_TYPE_UINT8; info.conversion = isSigned ? PARQUET_CONVERT_INT32_TO_INT8 : PARQUET_CONVERT_INT32_TO_UINT8; info.storageWidth = 1; break; }
            case 16: { info.column.type = isSigned ? CORE::DATATABLE_TYPE_INT16 : CORE::DATATABLE_TYPE_UINT16; info.conversion = isSigned ? PARQUET_CONVERT_INT32_TO_INT16 : PARQUET_CONVERT_INT32_TO_UINT16; info.storageWidth = 2; break; }
            case 32: { info.column.type = isSigned ? CORE::DATATABLE_TYPE_INT32 : CORE::DATATABLE_TYPE_UINT32; info.conversion = PARQUET_CONVERT_FIXED; info.storageWidth = 4; break; }
            default: return Unsupported( info, "INT(" + CORE::ToString( bitWidth ) + ") on INT32 is not valid" );
        }
    }
    else
    if ( PARQUET_TYPE_INT64 == physical )
    {
        if ( 64 != bitWidth )
            return Unsupported( info, "INT(" + CORE::ToString( bitWidth ) + ") on INT64 is not valid" );
        info.column.type = isSigned ? CORE::DATATABLE_TYPE_INT64 : CORE::DATATABLE_TYPE_UINT64;
        info.conversion = PARQUET_CONVERT_FIXED;
        info.storageWidth = 8;
    }
    else
    {
        return Unsupported( info, "an integer annotation on a physical type that is not INT32 or INT64" );
    }
    info.unsignedOrder = !isSigned;
    return true;
}

bool
SetTimestamp( SParquetColumnInfo& info, Int16 unit, bool utc )
{
    switch ( unit )
    {
        case PARQUET_TIMEUNIT_MILLIS: { info.column.timeUnit = CORE::DATATABLE_TIMEUNIT_MILLISECONDS; break; }
        case PARQUET_TIMEUNIT_MICROS: { info.column.timeUnit = CORE::DATATABLE_TIMEUNIT_MICROSECONDS; break; }
        case PARQUET_TIMEUNIT_NANOS: { info.column.timeUnit = CORE::DATATABLE_TIMEUNIT_NANOSECONDS; break; }
        default: return Unsupported( info, "a timestamp of an unknown time unit" );
    }
    info.column.type = CORE::DATATABLE_TYPE_TIMESTAMP;
    info.column.utcAdjusted = utc;
    info.conversion = PARQUET_CONVERT_FIXED;
    info.storageWidth = 8;
    return true;
}

/** The logical type of a leaf from its physical type, logical type and converted type */
void
MapLeafType( const SParquetSchemaElement& e, SParquetColumnInfo& info )
{
    const Int32 physical = e.type;
    const bool hasLogical = e.hasLogicalType && PARQUET_LOGICAL_NONE != e.logicalType.kind;
    const Int16 logical = hasLogical ? e.logicalType.kind : (Int16) PARQUET_LOGICAL_NONE;
    const Int32 converted = e.hasConvertedType ? e.convertedType : -1;

    switch ( physical )
    {
        case PARQUET_TYPE_BOOLEAN:
        {
            if ( hasLogical && PARQUET_LOGICAL_UNKNOWN != logical )
            {
                Unsupported( info, "a BOOLEAN column with logical type " + CORE::ToString( logical ) );
                return;
            }
            info.column.type = CORE::DATATABLE_TYPE_BOOL;
            info.conversion = PARQUET_CONVERT_BITS;
            return;
        }
        case PARQUET_TYPE_INT32:
        {
            info.physicalWidth = 4;
            if ( hasLogical )
            {
                switch ( logical )
                {
                    case PARQUET_LOGICAL_INTEGER: { SetInteger( info, physical, e.logicalType.bitWidth, e.logicalType.isSigned ); return; }
                    case PARQUET_LOGICAL_DATE: { info.column.type = CORE::DATATABLE_TYPE_DATE; info.conversion = PARQUET_CONVERT_FIXED; info.storageWidth = 4; return; }
                    case PARQUET_LOGICAL_DECIMAL: { SetDecimal( info, e.logicalType.decimalPrecision, e.logicalType.decimalScale, 9, PARQUET_CONVERT_INT32_TO_INT64 ); return; }
                    case PARQUET_LOGICAL_UNKNOWN: break;
                    case PARQUET_LOGICAL_TIME: { Unsupported( info, "time of day columns are not supported" ); return; }
                    default: { Unsupported( info, "an INT32 column with logical type " + CORE::ToString( logical ) ); return; }
                }
            }
            else
            {
                switch ( converted )
                {
                    case PARQUET_CONVERTED_INT_8: { SetInteger( info, physical, 8, true ); return; }
                    case PARQUET_CONVERTED_INT_16: { SetInteger( info, physical, 16, true ); return; }
                    case PARQUET_CONVERTED_INT_32: { SetInteger( info, physical, 32, true ); return; }
                    case PARQUET_CONVERTED_UINT_8: { SetInteger( info, physical, 8, false ); return; }
                    case PARQUET_CONVERTED_UINT_16: { SetInteger( info, physical, 16, false ); return; }
                    case PARQUET_CONVERTED_UINT_32: { SetInteger( info, physical, 32, false ); return; }
                    case PARQUET_CONVERTED_DATE: { info.column.type = CORE::DATATABLE_TYPE_DATE; info.conversion = PARQUET_CONVERT_FIXED; info.storageWidth = 4; return; }
                    case PARQUET_CONVERTED_DECIMAL: { SetDecimal( info, e.precision, e.hasScale ? e.scale : 0, 9, PARQUET_CONVERT_INT32_TO_INT64 ); return; }
                    case -1: break;
                    case PARQUET_CONVERTED_TIME_MILLIS: { Unsupported( info, "time of day columns are not supported" ); return; }
                    default: { Unsupported( info, "an INT32 column with converted type " + CORE::ToString( converted ) ); return; }
                }
            }
            info.column.type = CORE::DATATABLE_TYPE_INT32;
            info.conversion = PARQUET_CONVERT_FIXED;
            info.storageWidth = 4;
            return;
        }
        case PARQUET_TYPE_INT64:
        {
            info.physicalWidth = 8;
            if ( hasLogical )
            {
                switch ( logical )
                {
                    case PARQUET_LOGICAL_INTEGER: { SetInteger( info, physical, e.logicalType.bitWidth, e.logicalType.isSigned ); return; }
                    case PARQUET_LOGICAL_TIMESTAMP: { SetTimestamp( info, e.logicalType.timeUnit, e.logicalType.isAdjustedToUtc ); return; }
                    case PARQUET_LOGICAL_DECIMAL: { SetDecimal( info, e.logicalType.decimalPrecision, e.logicalType.decimalScale, 18, PARQUET_CONVERT_FIXED ); return; }
                    case PARQUET_LOGICAL_UNKNOWN: break;
                    case PARQUET_LOGICAL_TIME: { Unsupported( info, "time of day columns are not supported" ); return; }
                    default: { Unsupported( info, "an INT64 column with logical type " + CORE::ToString( logical ) ); return; }
                }
            }
            else
            {
                switch ( converted )
                {
                    case PARQUET_CONVERTED_INT_64: { SetInteger( info, physical, 64, true ); return; }
                    case PARQUET_CONVERTED_UINT_64: { SetInteger( info, physical, 64, false ); return; }
                    case PARQUET_CONVERTED_TIMESTAMP_MILLIS: { SetTimestamp( info, PARQUET_TIMEUNIT_MILLIS, true ); return; }
                    case PARQUET_CONVERTED_TIMESTAMP_MICROS: { SetTimestamp( info, PARQUET_TIMEUNIT_MICROS, true ); return; }
                    case PARQUET_CONVERTED_DECIMAL: { SetDecimal( info, e.precision, e.hasScale ? e.scale : 0, 18, PARQUET_CONVERT_FIXED ); return; }
                    case -1: break;
                    case PARQUET_CONVERTED_TIME_MICROS: { Unsupported( info, "time of day columns are not supported" ); return; }
                    default: { Unsupported( info, "an INT64 column with converted type " + CORE::ToString( converted ) ); return; }
                }
            }
            info.column.type = CORE::DATATABLE_TYPE_INT64;
            info.conversion = PARQUET_CONVERT_FIXED;
            info.storageWidth = 8;
            return;
        }
        case PARQUET_TYPE_INT96:
        {
            info.physicalWidth = 12;
            info.column.type = CORE::DATATABLE_TYPE_TIMESTAMP;
            info.column.timeUnit = CORE::DATATABLE_TIMEUNIT_NANOSECONDS;
            info.column.utcAdjusted = false;
            info.conversion = PARQUET_CONVERT_INT96_TO_INT64;
            info.storageWidth = 8;
            return;
        }
        case PARQUET_TYPE_FLOAT:
        case PARQUET_TYPE_DOUBLE:
        {
            if ( hasLogical && PARQUET_LOGICAL_UNKNOWN != logical )
            {
                Unsupported( info, "a floating point column with logical type " + CORE::ToString( logical ) );
                return;
            }
            const bool isFloat = PARQUET_TYPE_FLOAT == physical;
            info.physicalWidth = isFloat ? 4 : 8;
            info.column.type = isFloat ? CORE::DATATABLE_TYPE_FLOAT32 : CORE::DATATABLE_TYPE_FLOAT64;
            info.conversion = PARQUET_CONVERT_FIXED;
            info.storageWidth = info.physicalWidth;
            return;
        }
        case PARQUET_TYPE_BYTE_ARRAY:
        {
            if ( hasLogical )
            {
                switch ( logical )
                {
                    case PARQUET_LOGICAL_STRING:
                    case PARQUET_LOGICAL_ENUM:
                    case PARQUET_LOGICAL_JSON: { info.column.type = CORE::DATATABLE_TYPE_UTF8; info.conversion = PARQUET_CONVERT_BYTES_VARIABLE; return; }
                    case PARQUET_LOGICAL_BSON:
                    case PARQUET_LOGICAL_UNKNOWN: { info.column.type = CORE::DATATABLE_TYPE_BINARY; info.conversion = PARQUET_CONVERT_BYTES_VARIABLE; return; }
                    case PARQUET_LOGICAL_DECIMAL: { SetDecimal( info, e.logicalType.decimalPrecision, e.logicalType.decimalScale, 18, PARQUET_CONVERT_BYTES_DECIMAL ); return; }
                    default: { Unsupported( info, "a BYTE_ARRAY column with logical type " + CORE::ToString( logical ) ); return; }
                }
            }
            switch ( converted )
            {
                case PARQUET_CONVERTED_UTF8:
                case PARQUET_CONVERTED_ENUM:
                case PARQUET_CONVERTED_JSON: { info.column.type = CORE::DATATABLE_TYPE_UTF8; info.conversion = PARQUET_CONVERT_BYTES_VARIABLE; return; }
                case PARQUET_CONVERTED_DECIMAL: { SetDecimal( info, e.precision, e.hasScale ? e.scale : 0, 18, PARQUET_CONVERT_BYTES_DECIMAL ); return; }
                case PARQUET_CONVERTED_BSON:
                case -1: { info.column.type = CORE::DATATABLE_TYPE_BINARY; info.conversion = PARQUET_CONVERT_BYTES_VARIABLE; return; }
                default: { Unsupported( info, "a BYTE_ARRAY column with converted type " + CORE::ToString( converted ) ); return; }
            }
        }
        case PARQUET_TYPE_FIXED_LEN_BYTE_ARRAY:
        {
            if ( !e.hasTypeLength || e.typeLength < 1 || e.typeLength > MaxFixedLength )
            {
                Unsupported( info, "a FIXED_LEN_BYTE_ARRAY column without a valid type_length" );
                return;
            }
            info.typeLength = e.typeLength;
            info.physicalWidth = (UInt32) e.typeLength;
            if ( hasLogical )
            {
                switch ( logical )
                {
                    case PARQUET_LOGICAL_UUID:
                    case PARQUET_LOGICAL_UNKNOWN: { info.column.type = CORE::DATATABLE_TYPE_BINARY; info.conversion = PARQUET_CONVERT_FIXED_BYTES_VARIABLE; return; }
                    case PARQUET_LOGICAL_DECIMAL:
                    {
                        // a value wider than 8 bytes is decoded when its leading bytes are sign extension
                        SetDecimal( info, e.logicalType.decimalPrecision, e.logicalType.decimalScale, 18, PARQUET_CONVERT_FIXED_BYTES_DECIMAL );
                        return;
                    }
                    case PARQUET_LOGICAL_FLOAT16: { Unsupported( info, "FLOAT16 columns are not supported" ); return; }
                    default: { Unsupported( info, "a FIXED_LEN_BYTE_ARRAY column with logical type " + CORE::ToString( logical ) ); return; }
                }
            }
            switch ( converted )
            {
                case PARQUET_CONVERTED_DECIMAL: { SetDecimal( info, e.precision, e.hasScale ? e.scale : 0, 18, PARQUET_CONVERT_FIXED_BYTES_DECIMAL ); return; }
                case -1: { info.column.type = CORE::DATATABLE_TYPE_BINARY; info.conversion = PARQUET_CONVERT_FIXED_BYTES_VARIABLE; return; }
                case PARQUET_CONVERTED_INTERVAL: { Unsupported( info, "INTERVAL columns are not supported" ); return; }
                default: { Unsupported( info, "a FIXED_LEN_BYTE_ARRAY column with converted type " + CORE::ToString( converted ) ); return; }
            }
        }
        default:
        {
            Unsupported( info, "physical type " + CORE::ToString( physical ) + " is not known" );
            return;
        }
    }
}

bool
HasValidName( const CORE::CString& name )
{
    return CORE::CDataTableColumnDef::IsValidName( name );
}

bool
MakeUtf8Variant( const UInt8* bytes, UInt32 length, CORE::CVariant& value )
{
    // a UTF8 variant holds its terminator, as the values of batches do
    GUCEF::vector< char > terminated( (size_t) length + 1, '\0' );
    if ( length > 0 )
        memcpy( &terminated[ 0 ], bytes, length );
    return value.Set( &terminated[ 0 ], length + 1, GUCEF_DATATYPE_UTF8_STRING, false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SParquetColumnInfo::SParquetColumnInfo( void )
    : path()
    , schemaElementIndex( 0 )
    , physicalType( 0 )
    , typeLength( 0 )
    , maxDefinitionLevel( 0 )
    , maxRepetitionLevel( 0 )
    , supported( true )
    , unsupportedReason()
    , conversion( PARQUET_CONVERT_NONE )
    , physicalWidth( 0 )
    , storageWidth( 0 )
    , unsignedOrder( false )
    , columnOrder( PARQUET_COLUMNORDER_NONE )
    , column()
{
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetSchemaMapping::MapFileSchema( const SParquetFileMetaData& metaData   ,
                                      TParquetColumnInfoVector& columns      ,
                                      CORE::CDataTableSchema& schema         ,
                                      CORE::CString& error                   )
{GUCEF_TRACE;

    columns.clear();
    schema.Clear();
    const TParquetSchemaElementVector& elements = metaData.schema;
    if ( elements.empty() )
    {
        error = "The file schema has no root element";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    const SParquetSchemaElement& root = elements[ 0 ];
    if ( root.hasType || !root.hasNumChildren )
    {
        error = "The root of the file schema is not a group";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( (UInt64) root.numChildren > (UInt64) elements.size() - 1 )
    {
        error = "The root of the file schema has more children than there are schema elements";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    TGroupLevelVector stack;
    SGroupLevel rootLevel;
    rootLevel.remainingChildren = root.numChildren;
    rootLevel.definitionLevel = 0;
    rootLevel.repetitionLevel = 0;
    stack.push_back( rootLevel );

    size_t index = 1;
    while ( !stack.empty() )
    {
        if ( stack.back().remainingChildren <= 0 )
        {
            stack.pop_back();
            continue;
        }
        --stack.back().remainingChildren;
        if ( index >= elements.size() )
        {
            error = "The file schema ends before all children of its groups";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        const SParquetSchemaElement& e = elements[ index ];
        const SGroupLevel& parent = stack.back();
        const Int32 repetition = e.hasRepetitionType ? e.repetitionType : PARQUET_REPETITION_REQUIRED;
        if ( repetition < PARQUET_REPETITION_REQUIRED || repetition > PARQUET_REPETITION_REPEATED )
        {
            error = "Schema element " + CORE::ToString( (UInt32) index ) + " has an unknown repetition type";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        const UInt32 definitionLevel = parent.definitionLevel + ( PARQUET_REPETITION_REQUIRED != repetition ? 1 : 0 );
        const UInt32 repetitionLevel = parent.repetitionLevel + ( PARQUET_REPETITION_REPEATED == repetition ? 1 : 0 );
        const CORE::CString name = parent.prefix.IsNULLOrEmpty() ? e.name.ToString() : parent.prefix + "." + e.name.ToString();

        const bool isGroup = !e.hasType || ( e.hasNumChildren && e.numChildren > 0 );
        if ( isGroup )
        {
            if ( !e.hasNumChildren || e.numChildren < 0 || (UInt64) e.numChildren > (UInt64) ( elements.size() - index - 1 ) )
            {
                error = "Group \"" + name + "\" of the file schema has a child count that does not fit the schema";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
            if ( stack.size() >= MaxSchemaDepth )
            {
                error = "The file schema nests deeper than " + CORE::ToString( MaxSchemaDepth ) + " levels";
                return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
            }
            SGroupLevel level;
            level.remainingChildren = e.numChildren;
            level.definitionLevel = definitionLevel;
            level.repetitionLevel = repetitionLevel;
            level.prefix = name;
            ++index;
            stack.push_back( level );
            continue;
        }

        SParquetColumnInfo info;
        info.path = name;
        info.schemaElementIndex = (UInt32) index;
        info.physicalType = e.type;
        info.maxDefinitionLevel = definitionLevel;
        info.maxRepetitionLevel = repetitionLevel;
        info.column.name = name;
        info.column.nullable = ( PARQUET_REPETITION_OPTIONAL == repetition );
        if ( stack.size() > 1 )
            Unsupported( info, "nested column \"" + name + "\" is not supported" );
        else
        if ( PARQUET_REPETITION_REPEATED == repetition )
            Unsupported( info, "repeated column \"" + name + "\" is not supported" );
        else
            MapLeafType( e, info );

        if ( metaData.hasColumnOrders && columns.size() < metaData.columnOrders.size() )
            info.columnOrder = metaData.columnOrders[ columns.size() ];

        // names the table cannot take are replaced, duplicates are numbered, so every column is still listed
        if ( !HasValidName( info.column.name ) )
            info.column.name = "column_" + CORE::ToString( (UInt32) columns.size() + 1 );
        CORE::CString uniqueName = info.column.name;
        UInt32 suffix = 2;
        while ( schema.FindColumn( uniqueName ) >= 0 )
            uniqueName = info.column.name + "_" + CORE::ToString( suffix++ );
        info.column.name = uniqueName;
        if ( !schema.AddColumn( info.column ) )
        {
            error = "Column \"" + info.column.name + "\" of the file cannot be added to its schema";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        columns.push_back( info );
        ++index;
    }
    if ( index != elements.size() )
    {
        error = "The file schema has elements that belong to no group";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CParquetSchemaMapping::DecodeBigEndianDecimal( const UInt8* bytes, UInt32 length, Int64& value )
{GUCEF_TRACE;

    value = 0;
    if ( GUCEF_NULL == bytes || 0 == length )
        return false;
    UInt32 start = 0;
    if ( length > 8 )
    {
        // the leading bytes must be the sign extension of the 8 that hold the value
        start = length - 8;
        const UInt8 sign = ( 0 != ( bytes[ start ] & 0x80 ) ) ? 0xFF : 0x00;
        for ( UInt32 i=0; i<start; ++i )
        {
            if ( bytes[ i ] != sign )
                return false;
        }
    }
    UInt64 raw = ( 0 != ( bytes[ start ] & 0x80 ) ) ? ~(UInt64) 0 : 0;
    for ( UInt32 i=start; i<length; ++i )
        raw = ( raw << 8 ) | bytes[ i ];
    value = (Int64) raw;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CParquetSchemaMapping::IsValidUtf8( const UInt8* s, UInt32 length )
{GUCEF_TRACE;

    UInt32 i = 0;
    while ( i < length )
    {
        const UInt8 c = s[ i ];
        if ( c < 0x80 ) { ++i; continue; }
        UInt32 extra = 0;
        UInt32 codePoint = 0;
        UInt32 minimum = 0;
        if ( c >= 0xC2 && c <= 0xDF ) { extra = 1; codePoint = c & 0x1F; minimum = 0x80; }
        else if ( c >= 0xE0 && c <= 0xEF ) { extra = 2; codePoint = c & 0x0F; minimum = 0x800; }
        else if ( c >= 0xF0 && c <= 0xF4 ) { extra = 3; codePoint = c & 0x07; minimum = 0x10000; }
        else return false;
        if ( length - i - 1 < extra )
            return false;
        for ( UInt32 k=1; k<=extra; ++k )
        {
            const UInt8 next = s[ i + k ];
            if ( 0x80 != ( next & 0xC0 ) )
                return false;
            codePoint = ( codePoint << 6 ) | ( next & 0x3F );
        }
        if ( codePoint < minimum || codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += extra + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CParquetSchemaMapping::DecodeInt96Timestamp( const UInt8* bytes, Int64& nanoseconds )
{GUCEF_TRACE;

    nanoseconds = 0;
    const UInt64 nanosOfDay = LoadLE64( bytes );
    const Int64 julianDay = (Int64) (Int32) LoadLE32( bytes + 8 );
    if ( nanosOfDay >= (UInt64) NanosecondsPerDay )
        return false;
    const Int64 days = julianDay - JulianDayOfUnixEpoch;
    // Int64 nanoseconds reach about 106751 days on either side of the epoch
    if ( days > 106750 || days < -106751 )
        return false;
    nanoseconds = days * NanosecondsPerDay + (Int64) nanosOfDay;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CParquetSchemaMapping::DecodeStatisticValue( const SParquetColumnInfo& column, const SParquetBytes& bytes, CORE::CVariant& value )
{GUCEF_TRACE;

    value.Clear();
    if ( !column.supported )
        return false;
    const UInt8* p = bytes.data;
    const UInt32 n = bytes.size;
    switch ( column.column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:
        {
            if ( n < 1 )
                return false;
            value = CORE::CVariant( 0 != ( p[ 0 ] & 1 ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        {
            if ( 4 != n )
                return false;
            value = CORE::CVariant( (Int64) (Int32) LoadLE32( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        {
            if ( 4 != n )
                return false;
            value = CORE::CVariant( (UInt64) LoadLE32( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            if ( PARQUET_TYPE_INT64 != column.physicalType || 8 != n )
                return false;
            value = CORE::CVariant( (Int64) LoadLE64( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_UINT64:
        {
            if ( 8 != n )
                return false;
            value = CORE::CVariant( (UInt64) LoadLE64( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_DATE:
        {
            if ( 4 != n )
                return false;
            value = CORE::CDataTableValues::Date( (Int32) LoadLE32( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_FLOAT32:
        {
            if ( 4 != n )
                return false;
            value = CORE::CVariant( (Float64) LoadLEFloat32( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_FLOAT64:
        {
            if ( 8 != n )
                return false;
            value = CORE::CVariant( LoadLEFloat64( p ) );
            return true;
        }
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            Int64 mantissa = 0;
            switch ( column.conversion )
            {
                case PARQUET_CONVERT_INT32_TO_INT64: { if ( 4 != n ) return false; mantissa = (Int32) LoadLE32( p ); break; }
                case PARQUET_CONVERT_FIXED: { if ( 8 != n ) return false; mantissa = (Int64) LoadLE64( p ); break; }
                case PARQUET_CONVERT_BYTES_DECIMAL:
                case PARQUET_CONVERT_FIXED_BYTES_DECIMAL: { if ( !DecodeBigEndianDecimal( p, n, mantissa ) ) return false; break; }
                default: return false;
            }
            value = CORE::CVariant( mantissa );
            return true;
        }
        case CORE::DATATABLE_TYPE_UTF8:
        {
            return MakeUtf8Variant( p, n, value );
        }
        case CORE::DATATABLE_TYPE_BINARY:
        {
            return value.Set( p, n, GUCEF_DATATYPE_BINARY_BLOB, false );
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CParquetSchemaMapping::BuildColumnStats( const SParquetColumnInfo& column            ,
                                         const SParquetColumnChunk& chunk            ,
                                         Int64 rowCount                              ,
                                         CORE::CDataTableColumnStats& stats          )
{GUCEF_TRACE;

    stats.Clear();
    stats.valueCount = rowCount > 0 ? (UInt64) rowCount : 0;
    if ( !column.supported || !chunk.hasMetaData || !chunk.metaData.hasStatistics )
        return;
    const SParquetStatistics& s = chunk.metaData.statistics;

    if ( s.hasNullCount && s.nullCount >= 0 && s.nullCount <= rowCount )
    {
        stats.hasNullCount = true;
        stats.nullCount = (UInt64) s.nullCount;
    }

    // NaN cells are left out of the bounds of float columns: how many there are decides whether NOT_EQUALS may prune
    if ( s.hasNanCount && s.nanCount >= 0 && s.nanCount <= rowCount &&
         ( CORE::DATATABLE_TYPE_FLOAT32 == column.column.type || CORE::DATATABLE_TYPE_FLOAT64 == column.column.type ) )
    {
        stats.hasNanCount = true;
        stats.nanCount = (UInt64) s.nanCount;
    }

    // INT96 timestamps have no usable statistics in the type defined order
    if ( PARQUET_CONVERT_INT96_TO_INT64 == column.conversion )
        return;

    const bool isFloat = CORE::DATATABLE_TYPE_FLOAT32 == column.column.type || CORE::DATATABLE_TYPE_FLOAT64 == column.column.type;
    const bool orderUsable = PARQUET_COLUMNORDER_TYPE_DEFINED == column.columnOrder ||
                             ( isFloat && PARQUET_COLUMNORDER_IEEE754_TOTAL == column.columnOrder );
    const bool legacyUsable = !column.unsignedOrder &&
                              ( PARQUET_TYPE_INT32 == column.physicalType || PARQUET_TYPE_INT64 == column.physicalType ||
                                PARQUET_TYPE_FLOAT == column.physicalType || PARQUET_TYPE_DOUBLE == column.physicalType );

    const SParquetBytes* minBytes = GUCEF_NULL;
    const SParquetBytes* maxBytes = GUCEF_NULL;
    bool fromNewFields = false;
    if ( orderUsable && s.hasMinValue && s.hasMaxValue )
    {
        minBytes = &s.minValue;
        maxBytes = &s.maxValue;
        fromNewFields = true;
    }
    else
    if ( legacyUsable && s.hasMin && s.hasMax )
    {
        minBytes = &s.min;
        maxBytes = &s.max;
    }
    if ( GUCEF_NULL == minBytes )
        return;

    CORE::CVariant minValue;
    CORE::CVariant maxValue;
    if ( !DecodeStatisticValue( column, *minBytes, minValue ) || !DecodeStatisticValue( column, *maxBytes, maxValue ) )
        return;
    if ( isFloat )
    {
        const Float64 lo = minValue.AsFloat64();
        const Float64 hi = maxValue.AsFloat64();
        if ( lo != lo || hi != hi )
            return;
    }
    if ( CORE::CDataTablePruner::CompareNative( column.column.type, minValue, maxValue ) > 0 )
        return;

    stats.hasMinMax = true;
    stats.minValue = minValue;
    stats.maxValue = maxValue;
    stats.minExact = fromNewFields && s.hasIsMinValueExact && s.isMinValueExact;
    stats.maxExact = fromNewFields && s.hasIsMaxValueExact && s.isMaxValueExact;
}

/*-------------------------------------------------------------------------*/

Int32
CParquetSchemaMapping::WritePhysicalType( CORE::TDataTableLogicalType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case CORE::DATATABLE_TYPE_BOOL: return PARQUET_TYPE_BOOLEAN;
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        case CORE::DATATABLE_TYPE_DATE: return PARQUET_TYPE_INT32;
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_UINT64:
        case CORE::DATATABLE_TYPE_DECIMAL:
        case CORE::DATATABLE_TYPE_TIMESTAMP: return PARQUET_TYPE_INT64;
        case CORE::DATATABLE_TYPE_FLOAT32: return PARQUET_TYPE_FLOAT;
        case CORE::DATATABLE_TYPE_FLOAT64: return PARQUET_TYPE_DOUBLE;
        case CORE::DATATABLE_TYPE_UTF8:
        case CORE::DATATABLE_TYPE_BINARY: return PARQUET_TYPE_BYTE_ARRAY;
        default: return -1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CParquetSchemaMapping::MapColumnForWrite( const CORE::CDataTableColumnDef& column, const char* nameBytes, UInt32 nameLength,
                                          SParquetSchemaElement& e, CORE::CString& error )
{GUCEF_TRACE;

    e = SParquetSchemaElement();
    e.name = SParquetBytes( nameBytes, nameLength );
    e.hasRepetitionType = true;
    e.repetitionType = column.nullable ? PARQUET_REPETITION_OPTIONAL : PARQUET_REPETITION_REQUIRED;
    const Int32 physical = WritePhysicalType( column.type );
    if ( physical < 0 )
    {
        error = "Column \"" + column.name + "\" of type " + column.GetTypeText() + " cannot be written";
        return false;
    }
    e.hasType = true;
    e.type = physical;

    SParquetLogicalType& t = e.logicalType;
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        case CORE::DATATABLE_TYPE_UINT64:
        {
            const bool isSigned = !CORE::DataTableLogicalTypeIsUnsigned( column.type );
            const Int8 width = (Int8) ( CORE::DataTableLogicalTypeByteWidth( column.type ) * 8 );
            e.hasLogicalType = true;
            t.kind = PARQUET_LOGICAL_INTEGER;
            t.bitWidth = width;
            t.isSigned = isSigned;
            e.hasConvertedType = true;
            switch ( width )
            {
                case 8: { e.convertedType = isSigned ? PARQUET_CONVERTED_INT_8 : PARQUET_CONVERTED_UINT_8; break; }
                case 16: { e.convertedType = isSigned ? PARQUET_CONVERTED_INT_16 : PARQUET_CONVERTED_UINT_16; break; }
                case 32: { e.convertedType = isSigned ? PARQUET_CONVERTED_INT_32 : PARQUET_CONVERTED_UINT_32; break; }
                default: { e.convertedType = isSigned ? PARQUET_CONVERTED_INT_64 : PARQUET_CONVERTED_UINT_64; break; }
            }
            return true;
        }
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            e.hasLogicalType = true;
            t.kind = PARQUET_LOGICAL_DECIMAL;
            t.decimalPrecision = column.decimalPrecision;
            t.decimalScale = column.decimalScale;
            e.hasConvertedType = true;
            e.convertedType = PARQUET_CONVERTED_DECIMAL;
            e.hasScale = true;
            e.scale = column.decimalScale;
            e.hasPrecision = true;
            e.precision = column.decimalPrecision;
            return true;
        }
        case CORE::DATATABLE_TYPE_DATE:
        {
            e.hasLogicalType = true;
            t.kind = PARQUET_LOGICAL_DATE;
            e.hasConvertedType = true;
            e.convertedType = PARQUET_CONVERTED_DATE;
            return true;
        }
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            e.hasLogicalType = true;
            t.kind = PARQUET_LOGICAL_TIMESTAMP;
            t.isAdjustedToUtc = column.utcAdjusted;
            switch ( column.timeUnit )
            {
                case CORE::DATATABLE_TIMEUNIT_MILLISECONDS: { t.timeUnit = PARQUET_TIMEUNIT_MILLIS; e.hasConvertedType = true; e.convertedType = PARQUET_CONVERTED_TIMESTAMP_MILLIS; break; }
                case CORE::DATATABLE_TIMEUNIT_MICROSECONDS: { t.timeUnit = PARQUET_TIMEUNIT_MICROS; e.hasConvertedType = true; e.convertedType = PARQUET_CONVERTED_TIMESTAMP_MICROS; break; }
                default: { t.timeUnit = PARQUET_TIMEUNIT_NANOS; break; }
            }
            return true;
        }
        case CORE::DATATABLE_TYPE_UTF8:
        {
            e.hasLogicalType = true;
            t.kind = PARQUET_LOGICAL_STRING;
            e.hasConvertedType = true;
            e.convertedType = PARQUET_CONVERTED_UTF8;
            return true;
        }
        default:
        {
            return true;
        }
    }
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
