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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "tablecodecpluginPARQUET_ParquetMetaData.h"

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
//      STRUCTURES                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

SParquetBytes::SParquetBytes( void )
    : data( GUCEF_NULL )
    , size( 0 )
{
}

SParquetBytes::SParquetBytes( const void* bytes, UInt32 length )
    : data( static_cast< const UInt8* >( bytes ) )
    , size( GUCEF_NULL != bytes ? length : 0 )
{
}

bool
SParquetBytes::Equals( const char* text ) const
{
    const size_t length = GUCEF_NULL != text ? strlen( text ) : 0;
    return length == size && ( 0 == size || 0 == memcmp( data, text, size ) );
}

CORE::CString
SParquetBytes::ToString( void ) const
{
    if ( 0 == size )
        return CORE::CString::Empty;
    return CORE::CString( reinterpret_cast< const char* >( data ), size );
}

SParquetStatistics::SParquetStatistics( void )
    : hasMax( false ), hasMin( false ), hasNullCount( false ), hasDistinctCount( false ), hasMaxValue( false ), hasMinValue( false )
    , hasIsMaxValueExact( false ), hasIsMinValueExact( false ), hasNanCount( false )
    , max(), min(), nullCount( 0 ), distinctCount( 0 ), maxValue(), minValue(), isMaxValueExact( false ), isMinValueExact( false ), nanCount( 0 )
{
}

SParquetLogicalType::SParquetLogicalType( void )
    : kind( PARQUET_LOGICAL_NONE ), decimalScale( 0 ), decimalPrecision( 0 ), isAdjustedToUtc( false ), timeUnit( PARQUET_TIMEUNIT_NONE )
    , bitWidth( 0 ), isSigned( false )
{
}

SParquetSchemaElement::SParquetSchemaElement( void )
    : hasType( false ), hasTypeLength( false ), hasRepetitionType( false ), hasNumChildren( false ), hasConvertedType( false )
    , hasScale( false ), hasPrecision( false ), hasFieldId( false ), hasLogicalType( false )
    , type( 0 ), typeLength( 0 ), repetitionType( 0 ), name(), numChildren( 0 ), convertedType( 0 ), scale( 0 ), precision( 0 ), fieldId( 0 )
    , logicalType()
{
}

SParquetKeyValue::SParquetKeyValue( void )
    : key(), hasValue( false ), value()
{
}

SParquetSortingColumn::SParquetSortingColumn( void )
    : columnIdx( 0 ), descending( false ), nullsFirst( false )
{
}

SParquetPageEncodingStats::SParquetPageEncodingStats( void )
    : pageType( 0 ), encoding( 0 ), count( 0 )
{
}

SParquetColumnMetaData::SParquetColumnMetaData( void )
    : type( 0 ), encodings(), pathInSchema(), codec( 0 ), numValues( 0 ), totalUncompressedSize( 0 ), totalCompressedSize( 0 )
    , keyValueMetaData(), dataPageOffset( 0 ), hasIndexPageOffset( false ), indexPageOffset( 0 ), hasDictionaryPageOffset( false )
    , dictionaryPageOffset( 0 ), hasStatistics( false ), statistics(), hasEncodingStats( false ), encodingStats()
{
}

SParquetColumnChunk::SParquetColumnChunk( void )
    : hasFilePath( false ), filePath(), fileOffset( 0 ), hasMetaData( false ), metaData(), isEncrypted( false )
{
}

SParquetRowGroup::SParquetRowGroup( void )
    : columns(), totalByteSize( 0 ), numRows( 0 ), hasSortingColumns( false ), sortingColumns(), hasFileOffset( false ), fileOffset( 0 )
    , hasTotalCompressedSize( false ), totalCompressedSize( 0 ), hasOrdinal( false ), ordinal( 0 )
{
}

SParquetFileMetaData::SParquetFileMetaData( void )
    : version( 0 ), schema(), numRows( 0 ), rowGroups(), keyValueMetaData(), hasCreatedBy( false ), createdBy(), hasColumnOrders( false )
    , columnOrders(), hasEncryptionAlgorithm( false )
{
}

SParquetDataPageHeader::SParquetDataPageHeader( void )
    : numValues( 0 ), encoding( 0 ), definitionLevelEncoding( 0 ), repetitionLevelEncoding( 0 ), hasStatistics( false ), statistics()
{
}

SParquetDictionaryPageHeader::SParquetDictionaryPageHeader( void )
    : numValues( 0 ), encoding( 0 ), hasIsSorted( false ), isSorted( false )
{
}

SParquetDataPageHeaderV2::SParquetDataPageHeaderV2( void )
    : numValues( 0 ), numNulls( 0 ), numRows( 0 ), encoding( 0 ), definitionLevelsByteLength( 0 ), repetitionLevelsByteLength( 0 )
    , isCompressed( true ), hasStatistics( false ), statistics()
{
}

SParquetPageHeader::SParquetPageHeader( void )
    : type( 0 ), uncompressedPageSize( 0 ), compressedPageSize( 0 ), hasCrc( false ), crc( 0 ), hasDataPageHeader( false ), dataPageHeader()
    , hasDictionaryPageHeader( false ), dictionaryPageHeader(), hasDataPageHeaderV2( false ), dataPageHeaderV2()
{
}

SParquetParseBudget::SParquetParseBudget( UInt64 maxBytes )
    : maxObjectBytes( maxBytes )
    , usedObjectBytes( 0 )
{
}

bool
SParquetParseBudget::Charge( CThriftCompactReader& reader, UInt64 bytes )
{
    if ( bytes > maxObjectBytes || usedObjectBytes > maxObjectBytes - bytes )
        return reader.Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The file metadata needs more memory than the metadata limit allows" );
    usedObjectBytes += bytes;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
WrongType( CThriftCompactReader& reader )
{
    return reader.Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A Parquet metadata field has the wrong Thrift type" );
}

bool
FieldI8( CThriftCompactReader& reader, UInt8 type, Int8& value )
{
    if ( THRIFT_COMPACT_I8 != type )
        return WrongType( reader );
    return reader.ReadI8( value );
}

bool
FieldI16( CThriftCompactReader& reader, UInt8 type, Int16& value )
{
    if ( THRIFT_COMPACT_I16 != type )
        return WrongType( reader );
    return reader.ReadI16( value );
}

bool
FieldI32( CThriftCompactReader& reader, UInt8 type, Int32& value )
{
    if ( THRIFT_COMPACT_I32 != type )
        return WrongType( reader );
    return reader.ReadI32( value );
}

bool
FieldI64( CThriftCompactReader& reader, UInt8 type, Int64& value )
{
    if ( THRIFT_COMPACT_I64 != type )
        return WrongType( reader );
    return reader.ReadI64( value );
}

bool
FieldBool( CThriftCompactReader& reader, UInt8 type, bool& value )
{
    if ( THRIFT_COMPACT_BOOL_TRUE != type && THRIFT_COMPACT_BOOL_FALSE != type )
        return WrongType( reader );
    return reader.ReadBoolField( type, value );
}

bool
FieldBinary( CThriftCompactReader& reader, UInt8 type, SParquetBytes& value )
{
    if ( THRIFT_COMPACT_BINARY != type )
        return WrongType( reader );
    const UInt8* bytes = GUCEF_NULL;
    UInt32 length = 0;
    if ( !reader.ReadBinary( bytes, length ) )
        return false;
    value = SParquetBytes( bytes, length );
    return true;
}

bool
ExpectStruct( CThriftCompactReader& reader, UInt8 type )
{
    if ( THRIFT_COMPACT_STRUCT != type )
        return WrongType( reader );
    return true;
}

/** Reads a list header of the given element type and enters the list */
bool
BeginList( CThriftCompactReader& reader, UInt8 type, UInt8 expectedElementType, UInt32& count, SParquetParseBudget& budget, UInt64 elementBytes )
{
    if ( THRIFT_COMPACT_LIST != type && THRIFT_COMPACT_SET != type )
        return WrongType( reader );
    UInt8 elementType = 0;
    if ( !reader.ReadListHeader( elementType, count ) )
        return false;
    if ( count > 0 && elementType != expectedElementType )
        return WrongType( reader );
    if ( !budget.Charge( reader, (UInt64) count * elementBytes ) )
        return false;
    return reader.BeginContainer();
}

bool
ReadI32List( CThriftCompactReader& reader, UInt8 type, TInt32Vector& values, SParquetParseBudget& budget )
{
    UInt32 count = 0;
    if ( !BeginList( reader, type, THRIFT_COMPACT_I32, count, budget, sizeof( Int32 ) ) )
        return false;
    values.clear();
    values.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        Int32 value = 0;
        if ( !reader.ReadI32( value ) )
            return false;
        values.push_back( value );
    }
    return reader.EndContainer();
}

bool
ReadBinaryList( CThriftCompactReader& reader, UInt8 type, TParquetBytesVector& values, SParquetParseBudget& budget )
{
    UInt32 count = 0;
    if ( !BeginList( reader, type, THRIFT_COMPACT_BINARY, count, budget, sizeof( SParquetBytes ) ) )
        return false;
    values.clear();
    values.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt8* bytes = GUCEF_NULL;
        UInt32 length = 0;
        if ( !reader.ReadBinary( bytes, length ) )
            return false;
        values.push_back( SParquetBytes( bytes, length ) );
    }
    return reader.EndContainer();
}

bool
ReadKeyValueList( CThriftCompactReader& reader, UInt8 type, TParquetKeyValueVector& values, SParquetParseBudget& budget )
{
    UInt32 count = 0;
    if ( !BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetKeyValue ) ) )
        return false;
    values.clear();
    values.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        values.push_back( SParquetKeyValue() );
        if ( !CParquetMetaDataSerializer::ReadKeyValue( reader, values.back() ) )
            return false;
    }
    return reader.EndContainer();
}

/** A union whose members are empty structs (TimeUnit, ColumnOrder, the members of LogicalType): the id of the member set */
bool
ReadUnionMemberId( CThriftCompactReader& reader, Int16& memberId )
{
    memberId = 0;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        if ( 0 == memberId )
            memberId = id;
        if ( !reader.SkipField( type ) )
            return false;
    }
    return reader.EndStruct();
}

bool
Missing( CThriftCompactReader& reader, const char* message )
{
    return reader.Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, message );
}

void
WriteEmptyStructField( CThriftCompactWriter& writer, Int16 fieldId )
{
    writer.BeginStructField( fieldId );
    writer.EndStruct();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadStatistics( CThriftCompactReader& reader, SParquetStatistics& s )
{GUCEF_TRACE;

    s = SParquetStatistics();
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldBinary( reader, type, s.max ); s.hasMax = ok; break; }
            case 2: { ok = FieldBinary( reader, type, s.min ); s.hasMin = ok; break; }
            case 3: { ok = FieldI64( reader, type, s.nullCount ); s.hasNullCount = ok; break; }
            case 4: { ok = FieldI64( reader, type, s.distinctCount ); s.hasDistinctCount = ok; break; }
            case 5: { ok = FieldBinary( reader, type, s.maxValue ); s.hasMaxValue = ok; break; }
            case 6: { ok = FieldBinary( reader, type, s.minValue ); s.hasMinValue = ok; break; }
            case 7: { ok = FieldBool( reader, type, s.isMaxValueExact ); s.hasIsMaxValueExact = ok; break; }
            case 8: { ok = FieldBool( reader, type, s.isMinValueExact ); s.hasIsMinValueExact = ok; break; }
            case 9: { ok = FieldI64( reader, type, s.nanCount ); s.hasNanCount = ok; break; }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadLogicalType( CThriftCompactReader& reader, SParquetLogicalType& logicalType )
{GUCEF_TRACE;

    logicalType = SParquetLogicalType();
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        if ( PARQUET_LOGICAL_NONE != logicalType.kind )
        {
            // a union holds one member; a second one is not valid
            return reader.Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A Parquet logical type has more than one member" );
        }
        if ( !ExpectStruct( reader, type ) )
            return false;
        logicalType.kind = id;
        switch ( id )
        {
            case PARQUET_LOGICAL_DECIMAL:
            {
                if ( !reader.BeginStruct() )
                    return false;
                bool hasScale = false, hasPrecision = false;
                for ( ;; )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    if ( !reader.ReadFieldHeader( memberType, memberId ) )
                        return false;
                    if ( THRIFT_COMPACT_STOP == memberType )
                        break;
                    bool ok = true;
                    if ( 1 == memberId ) { ok = FieldI32( reader, memberType, logicalType.decimalScale ); hasScale = true; }
                    else if ( 2 == memberId ) { ok = FieldI32( reader, memberType, logicalType.decimalPrecision ); hasPrecision = true; }
                    else ok = reader.SkipField( memberType );
                    if ( !ok )
                        return false;
                }
                if ( !hasScale || !hasPrecision )
                    return Missing( reader, "A Parquet DECIMAL logical type lacks its scale or precision" );
                if ( !reader.EndStruct() )
                    return false;
                break;
            }
            case PARQUET_LOGICAL_TIME:
            case PARQUET_LOGICAL_TIMESTAMP:
            {
                if ( !reader.BeginStruct() )
                    return false;
                bool hasUtc = false, hasUnit = false;
                for ( ;; )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    if ( !reader.ReadFieldHeader( memberType, memberId ) )
                        return false;
                    if ( THRIFT_COMPACT_STOP == memberType )
                        break;
                    bool ok = true;
                    if ( 1 == memberId ) { ok = FieldBool( reader, memberType, logicalType.isAdjustedToUtc ); hasUtc = true; }
                    else if ( 2 == memberId ) { ok = ExpectStruct( reader, memberType ) && ReadUnionMemberId( reader, logicalType.timeUnit ); hasUnit = true; }
                    else ok = reader.SkipField( memberType );
                    if ( !ok )
                        return false;
                }
                if ( !hasUtc || !hasUnit )
                    return Missing( reader, "A Parquet TIME or TIMESTAMP logical type lacks isAdjustedToUTC or its unit" );
                if ( !reader.EndStruct() )
                    return false;
                break;
            }
            case PARQUET_LOGICAL_INTEGER:
            {
                if ( !reader.BeginStruct() )
                    return false;
                bool hasWidth = false, hasSigned = false;
                for ( ;; )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    if ( !reader.ReadFieldHeader( memberType, memberId ) )
                        return false;
                    if ( THRIFT_COMPACT_STOP == memberType )
                        break;
                    bool ok = true;
                    if ( 1 == memberId ) { ok = FieldI8( reader, memberType, logicalType.bitWidth ); hasWidth = true; }
                    else if ( 2 == memberId ) { ok = FieldBool( reader, memberType, logicalType.isSigned ); hasSigned = true; }
                    else ok = reader.SkipField( memberType );
                    if ( !ok )
                        return false;
                }
                if ( !hasWidth || !hasSigned )
                    return Missing( reader, "A Parquet INTEGER logical type lacks its bit width or signedness" );
                if ( !reader.EndStruct() )
                    return false;
                break;
            }
            default:
            {
                // the empty structs of STRING, DATE, ... and members this reader does not know
                if ( !reader.Skip( THRIFT_COMPACT_STRUCT ) )
                    return false;
                break;
            }
        }
    }
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadSchemaElement( CThriftCompactReader& reader, SParquetSchemaElement& e )
{GUCEF_TRACE;

    e = SParquetSchemaElement();
    bool hasName = false;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldI32( reader, type, e.type ); e.hasType = true; break; }
            case 2: { ok = FieldI32( reader, type, e.typeLength ); e.hasTypeLength = true; break; }
            case 3: { ok = FieldI32( reader, type, e.repetitionType ); e.hasRepetitionType = true; break; }
            case 4: { ok = FieldBinary( reader, type, e.name ); hasName = true; break; }
            case 5: { ok = FieldI32( reader, type, e.numChildren ); e.hasNumChildren = true; break; }
            case 6: { ok = FieldI32( reader, type, e.convertedType ); e.hasConvertedType = true; break; }
            case 7: { ok = FieldI32( reader, type, e.scale ); e.hasScale = true; break; }
            case 8: { ok = FieldI32( reader, type, e.precision ); e.hasPrecision = true; break; }
            case 9: { ok = FieldI32( reader, type, e.fieldId ); e.hasFieldId = true; break; }
            case 10: { ok = ExpectStruct( reader, type ) && ReadLogicalType( reader, e.logicalType ); e.hasLogicalType = true; break; }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    if ( !hasName )
        return Missing( reader, "A Parquet schema element has no name" );
    if ( e.hasNumChildren && e.numChildren < 0 )
        return Missing( reader, "A Parquet schema element has a negative number of children" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadKeyValue( CThriftCompactReader& reader, SParquetKeyValue& kv )
{GUCEF_TRACE;

    kv = SParquetKeyValue();
    bool hasKey = false;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        if ( 1 == id ) { ok = FieldBinary( reader, type, kv.key ); hasKey = true; }
        else if ( 2 == id ) { ok = FieldBinary( reader, type, kv.value ); kv.hasValue = true; }
        else ok = reader.SkipField( type );
        if ( !ok )
            return false;
    }
    if ( !hasKey )
        return Missing( reader, "A Parquet key value has no key" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadColumnMetaData( CThriftCompactReader& reader, SParquetColumnMetaData& m, SParquetParseBudget& budget )
{GUCEF_TRACE;

    m = SParquetColumnMetaData();
    UInt32 required = 0;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldI32( reader, type, m.type ); required |= 1; break; }
            case 2: { ok = ReadI32List( reader, type, m.encodings, budget ); required |= 2; break; }
            case 3: { ok = ReadBinaryList( reader, type, m.pathInSchema, budget ); required |= 4; break; }
            case 4: { ok = FieldI32( reader, type, m.codec ); required |= 8; break; }
            case 5: { ok = FieldI64( reader, type, m.numValues ); required |= 16; break; }
            case 6: { ok = FieldI64( reader, type, m.totalUncompressedSize ); required |= 32; break; }
            case 7: { ok = FieldI64( reader, type, m.totalCompressedSize ); required |= 64; break; }
            case 8: { ok = ReadKeyValueList( reader, type, m.keyValueMetaData, budget ); break; }
            case 9: { ok = FieldI64( reader, type, m.dataPageOffset ); required |= 128; break; }
            case 10: { ok = FieldI64( reader, type, m.indexPageOffset ); m.hasIndexPageOffset = true; break; }
            case 11: { ok = FieldI64( reader, type, m.dictionaryPageOffset ); m.hasDictionaryPageOffset = true; break; }
            case 12: { ok = ExpectStruct( reader, type ) && ReadStatistics( reader, m.statistics ); m.hasStatistics = true; break; }
            case 13:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetPageEncodingStats ) );
                m.encodingStats.clear();
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    SParquetPageEncodingStats stats;
                    UInt32 statsRequired = 0;
                    ok = reader.BeginStruct();
                    while ( ok )
                    {
                        UInt8 memberType = 0;
                        Int16 memberId = 0;
                        ok = reader.ReadFieldHeader( memberType, memberId );
                        if ( !ok || THRIFT_COMPACT_STOP == memberType )
                            break;
                        if ( 1 == memberId ) { ok = FieldI32( reader, memberType, stats.pageType ); statsRequired |= 1; }
                        else if ( 2 == memberId ) { ok = FieldI32( reader, memberType, stats.encoding ); statsRequired |= 2; }
                        else if ( 3 == memberId ) { ok = FieldI32( reader, memberType, stats.count ); statsRequired |= 4; }
                        else ok = reader.SkipField( memberType );
                    }
                    if ( ok && 7 != statsRequired )
                        ok = Missing( reader, "A Parquet page encoding statistic lacks a required field" );
                    ok = ok && reader.EndStruct();
                    if ( ok )
                        m.encodingStats.push_back( stats );
                }
                ok = ok && reader.EndContainer();
                m.hasEncodingStats = ok;
                break;
            }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    if ( 255 != required )
        return Missing( reader, "Parquet column metadata lacks a required field" );
    if ( m.numValues < 0 || m.totalCompressedSize < 0 || m.totalUncompressedSize < 0 || m.dataPageOffset < 0 ||
         ( m.hasDictionaryPageOffset && m.dictionaryPageOffset < 0 ) )
        return Missing( reader, "Parquet column metadata holds a negative size, count or offset" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadColumnChunk( CThriftCompactReader& reader, SParquetColumnChunk& c, SParquetParseBudget& budget )
{GUCEF_TRACE;

    c = SParquetColumnChunk();
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldBinary( reader, type, c.filePath ); c.hasFilePath = true; break; }
            case 2: { ok = FieldI64( reader, type, c.fileOffset ); break; }
            case 3: { ok = ExpectStruct( reader, type ) && ReadColumnMetaData( reader, c.metaData, budget ); c.hasMetaData = true; break; }
            case 8:
            case 9: { c.isEncrypted = true; ok = reader.SkipField( type ); break; }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadRowGroup( CThriftCompactReader& reader, SParquetRowGroup& g, SParquetParseBudget& budget )
{GUCEF_TRACE;

    g = SParquetRowGroup();
    UInt32 required = 0;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetColumnChunk ) );
                g.columns.clear();
                if ( ok )
                    g.columns.reserve( count );
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    g.columns.push_back( SParquetColumnChunk() );
                    ok = ReadColumnChunk( reader, g.columns.back(), budget );
                }
                ok = ok && reader.EndContainer();
                required |= 1;
                break;
            }
            case 2: { ok = FieldI64( reader, type, g.totalByteSize ); required |= 2; break; }
            case 3: { ok = FieldI64( reader, type, g.numRows ); required |= 4; break; }
            case 4:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetSortingColumn ) );
                g.sortingColumns.clear();
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    SParquetSortingColumn column;
                    UInt32 columnRequired = 0;
                    ok = reader.BeginStruct();
                    while ( ok )
                    {
                        UInt8 memberType = 0;
                        Int16 memberId = 0;
                        ok = reader.ReadFieldHeader( memberType, memberId );
                        if ( !ok || THRIFT_COMPACT_STOP == memberType )
                            break;
                        if ( 1 == memberId ) { ok = FieldI32( reader, memberType, column.columnIdx ); columnRequired |= 1; }
                        else if ( 2 == memberId ) { ok = FieldBool( reader, memberType, column.descending ); columnRequired |= 2; }
                        else if ( 3 == memberId ) { ok = FieldBool( reader, memberType, column.nullsFirst ); columnRequired |= 4; }
                        else ok = reader.SkipField( memberType );
                    }
                    if ( ok && 7 != columnRequired )
                        ok = Missing( reader, "A Parquet sorting column lacks a required field" );
                    ok = ok && reader.EndStruct();
                    if ( ok )
                        g.sortingColumns.push_back( column );
                }
                ok = ok && reader.EndContainer();
                g.hasSortingColumns = ok;
                break;
            }
            case 5: { ok = FieldI64( reader, type, g.fileOffset ); g.hasFileOffset = true; break; }
            case 6: { ok = FieldI64( reader, type, g.totalCompressedSize ); g.hasTotalCompressedSize = true; break; }
            case 7: { ok = FieldI16( reader, type, g.ordinal ); g.hasOrdinal = true; break; }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    if ( 7 != required )
        return Missing( reader, "A Parquet row group lacks a required field" );
    if ( g.numRows < 0 )
        return Missing( reader, "A Parquet row group has a negative row count" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadFileMetaData( CThriftCompactReader& reader, SParquetFileMetaData& f, SParquetParseBudget& budget )
{GUCEF_TRACE;

    f = SParquetFileMetaData();
    UInt32 required = 0;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldI32( reader, type, f.version ); required |= 1; break; }
            case 2:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetSchemaElement ) );
                f.schema.clear();
                if ( ok )
                    f.schema.reserve( count );
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    f.schema.push_back( SParquetSchemaElement() );
                    ok = ReadSchemaElement( reader, f.schema.back() );
                }
                ok = ok && reader.EndContainer();
                required |= 2;
                break;
            }
            case 3: { ok = FieldI64( reader, type, f.numRows ); required |= 4; break; }
            case 4:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( SParquetRowGroup ) );
                f.rowGroups.clear();
                if ( ok )
                    f.rowGroups.reserve( count );
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    f.rowGroups.push_back( SParquetRowGroup() );
                    ok = ReadRowGroup( reader, f.rowGroups.back(), budget );
                }
                ok = ok && reader.EndContainer();
                required |= 8;
                break;
            }
            case 5: { ok = ReadKeyValueList( reader, type, f.keyValueMetaData, budget ); break; }
            case 6: { ok = FieldBinary( reader, type, f.createdBy ); f.hasCreatedBy = true; break; }
            case 7:
            {
                UInt32 count = 0;
                ok = BeginList( reader, type, THRIFT_COMPACT_STRUCT, count, budget, sizeof( Int16 ) );
                f.columnOrders.clear();
                for ( UInt32 i=0; ok && i<count; ++i )
                {
                    Int16 member = 0;
                    ok = ReadUnionMemberId( reader, member );
                    if ( ok )
                        f.columnOrders.push_back( member );
                }
                ok = ok && reader.EndContainer();
                f.hasColumnOrders = ok;
                break;
            }
            case 8: { f.hasEncryptionAlgorithm = true; ok = reader.SkipField( type ); break; }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    if ( 15 != required )
        return Missing( reader, "The Parquet file metadata lacks a required field" );
    if ( f.numRows < 0 )
        return Missing( reader, "The Parquet file metadata has a negative row count" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

bool
CParquetMetaDataSerializer::ReadPageHeader( CThriftCompactReader& reader, SParquetPageHeader& h )
{GUCEF_TRACE;

    h = SParquetPageHeader();
    UInt32 required = 0;
    if ( !reader.BeginStruct() )
        return false;
    for ( ;; )
    {
        UInt8 type = 0;
        Int16 id = 0;
        if ( !reader.ReadFieldHeader( type, id ) )
            return false;
        if ( THRIFT_COMPACT_STOP == type )
            break;
        bool ok = true;
        switch ( id )
        {
            case 1: { ok = FieldI32( reader, type, h.type ); required |= 1; break; }
            case 2: { ok = FieldI32( reader, type, h.uncompressedPageSize ); required |= 2; break; }
            case 3: { ok = FieldI32( reader, type, h.compressedPageSize ); required |= 4; break; }
            case 4: { ok = FieldI32( reader, type, h.crc ); h.hasCrc = true; break; }
            case 5:
            {
                SParquetDataPageHeader& d = h.dataPageHeader;
                UInt32 dataRequired = 0;
                ok = ExpectStruct( reader, type ) && reader.BeginStruct();
                while ( ok )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    ok = reader.ReadFieldHeader( memberType, memberId );
                    if ( !ok || THRIFT_COMPACT_STOP == memberType )
                        break;
                    switch ( memberId )
                    {
                        case 1: { ok = FieldI32( reader, memberType, d.numValues ); dataRequired |= 1; break; }
                        case 2: { ok = FieldI32( reader, memberType, d.encoding ); dataRequired |= 2; break; }
                        case 3: { ok = FieldI32( reader, memberType, d.definitionLevelEncoding ); dataRequired |= 4; break; }
                        case 4: { ok = FieldI32( reader, memberType, d.repetitionLevelEncoding ); dataRequired |= 8; break; }
                        case 5: { ok = ExpectStruct( reader, memberType ) && ReadStatistics( reader, d.statistics ); d.hasStatistics = true; break; }
                        default: { ok = reader.SkipField( memberType ); break; }
                    }
                }
                if ( ok && 15 != dataRequired )
                    ok = Missing( reader, "A Parquet data page header lacks a required field" );
                ok = ok && reader.EndStruct();
                h.hasDataPageHeader = ok;
                break;
            }
            case 7:
            {
                SParquetDictionaryPageHeader& d = h.dictionaryPageHeader;
                UInt32 dictRequired = 0;
                ok = ExpectStruct( reader, type ) && reader.BeginStruct();
                while ( ok )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    ok = reader.ReadFieldHeader( memberType, memberId );
                    if ( !ok || THRIFT_COMPACT_STOP == memberType )
                        break;
                    if ( 1 == memberId ) { ok = FieldI32( reader, memberType, d.numValues ); dictRequired |= 1; }
                    else if ( 2 == memberId ) { ok = FieldI32( reader, memberType, d.encoding ); dictRequired |= 2; }
                    else if ( 3 == memberId ) { ok = FieldBool( reader, memberType, d.isSorted ); d.hasIsSorted = true; }
                    else ok = reader.SkipField( memberType );
                }
                if ( ok && 3 != dictRequired )
                    ok = Missing( reader, "A Parquet dictionary page header lacks a required field" );
                ok = ok && reader.EndStruct();
                h.hasDictionaryPageHeader = ok;
                break;
            }
            case 8:
            {
                SParquetDataPageHeaderV2& d = h.dataPageHeaderV2;
                UInt32 v2Required = 0;
                ok = ExpectStruct( reader, type ) && reader.BeginStruct();
                while ( ok )
                {
                    UInt8 memberType = 0;
                    Int16 memberId = 0;
                    ok = reader.ReadFieldHeader( memberType, memberId );
                    if ( !ok || THRIFT_COMPACT_STOP == memberType )
                        break;
                    switch ( memberId )
                    {
                        case 1: { ok = FieldI32( reader, memberType, d.numValues ); v2Required |= 1; break; }
                        case 2: { ok = FieldI32( reader, memberType, d.numNulls ); v2Required |= 2; break; }
                        case 3: { ok = FieldI32( reader, memberType, d.numRows ); v2Required |= 4; break; }
                        case 4: { ok = FieldI32( reader, memberType, d.encoding ); v2Required |= 8; break; }
                        case 5: { ok = FieldI32( reader, memberType, d.definitionLevelsByteLength ); v2Required |= 16; break; }
                        case 6: { ok = FieldI32( reader, memberType, d.repetitionLevelsByteLength ); v2Required |= 32; break; }
                        case 7: { ok = FieldBool( reader, memberType, d.isCompressed ); break; }
                        case 8: { ok = ExpectStruct( reader, memberType ) && ReadStatistics( reader, d.statistics ); d.hasStatistics = true; break; }
                        default: { ok = reader.SkipField( memberType ); break; }
                    }
                }
                if ( ok && 63 != v2Required )
                    ok = Missing( reader, "A Parquet data page v2 header lacks a required field" );
                ok = ok && reader.EndStruct();
                h.hasDataPageHeaderV2 = ok;
                break;
            }
            default: { ok = reader.SkipField( type ); break; }
        }
        if ( !ok )
            return false;
    }
    if ( 7 != required )
        return Missing( reader, "A Parquet page header lacks a required field" );
    if ( h.uncompressedPageSize < 0 || h.compressedPageSize < 0 )
        return Missing( reader, "A Parquet page header has a negative page size" );
    return reader.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteStatistics( CThriftCompactWriter& writer, const SParquetStatistics& s )
{GUCEF_TRACE;

    writer.BeginStruct();
    if ( s.hasMax ) writer.WriteBinaryField( 1, s.max.data, s.max.size );
    if ( s.hasMin ) writer.WriteBinaryField( 2, s.min.data, s.min.size );
    if ( s.hasNullCount ) writer.WriteI64Field( 3, s.nullCount );
    if ( s.hasDistinctCount ) writer.WriteI64Field( 4, s.distinctCount );
    if ( s.hasMaxValue ) writer.WriteBinaryField( 5, s.maxValue.data, s.maxValue.size );
    if ( s.hasMinValue ) writer.WriteBinaryField( 6, s.minValue.data, s.minValue.size );
    if ( s.hasIsMaxValueExact ) writer.WriteBoolField( 7, s.isMaxValueExact );
    if ( s.hasIsMinValueExact ) writer.WriteBoolField( 8, s.isMinValueExact );
    if ( s.hasNanCount ) writer.WriteI64Field( 9, s.nanCount );
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteLogicalType( CThriftCompactWriter& writer, const SParquetLogicalType& t )
{GUCEF_TRACE;

    writer.BeginStruct();
    switch ( t.kind )
    {
        case PARQUET_LOGICAL_DECIMAL:
        {
            writer.BeginStructField( PARQUET_LOGICAL_DECIMAL );
            writer.WriteI32Field( 1, t.decimalScale );
            writer.WriteI32Field( 2, t.decimalPrecision );
            writer.EndStruct();
            break;
        }
        case PARQUET_LOGICAL_TIME:
        case PARQUET_LOGICAL_TIMESTAMP:
        {
            writer.BeginStructField( t.kind );
            writer.WriteBoolField( 1, t.isAdjustedToUtc );
            writer.BeginStructField( 2 );
            WriteEmptyStructField( writer, t.timeUnit );
            writer.EndStruct();
            writer.EndStruct();
            break;
        }
        case PARQUET_LOGICAL_INTEGER:
        {
            writer.BeginStructField( PARQUET_LOGICAL_INTEGER );
            writer.WriteI8Field( 1, t.bitWidth );
            writer.WriteBoolField( 2, t.isSigned );
            writer.EndStruct();
            break;
        }
        case PARQUET_LOGICAL_NONE:
        {
            break;
        }
        default:
        {
            WriteEmptyStructField( writer, t.kind );
            break;
        }
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteSchemaElement( CThriftCompactWriter& writer, const SParquetSchemaElement& e )
{GUCEF_TRACE;

    writer.BeginStruct();
    if ( e.hasType ) writer.WriteI32Field( 1, e.type );
    if ( e.hasTypeLength ) writer.WriteI32Field( 2, e.typeLength );
    if ( e.hasRepetitionType ) writer.WriteI32Field( 3, e.repetitionType );
    writer.WriteBinaryField( 4, e.name.data, e.name.size );
    if ( e.hasNumChildren ) writer.WriteI32Field( 5, e.numChildren );
    if ( e.hasConvertedType ) writer.WriteI32Field( 6, e.convertedType );
    if ( e.hasScale ) writer.WriteI32Field( 7, e.scale );
    if ( e.hasPrecision ) writer.WriteI32Field( 8, e.precision );
    if ( e.hasFieldId ) writer.WriteI32Field( 9, e.fieldId );
    if ( e.hasLogicalType )
    {
        writer.WriteFieldHeader( THRIFT_COMPACT_STRUCT, 10 );
        WriteLogicalType( writer, e.logicalType );
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteKeyValue( CThriftCompactWriter& writer, const SParquetKeyValue& kv )
{GUCEF_TRACE;

    writer.BeginStruct();
    writer.WriteBinaryField( 1, kv.key.data, kv.key.size );
    if ( kv.hasValue )
        writer.WriteBinaryField( 2, kv.value.data, kv.value.size );
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteColumnMetaData( CThriftCompactWriter& writer, const SParquetColumnMetaData& m )
{GUCEF_TRACE;

    writer.BeginStruct();
    writer.WriteI32Field( 1, m.type );
    writer.BeginListField( 2, THRIFT_COMPACT_I32, (UInt32) m.encodings.size() );
    for ( size_t i=0; i<m.encodings.size(); ++i )
        writer.WriteI32( m.encodings[ i ] );
    writer.BeginListField( 3, THRIFT_COMPACT_BINARY, (UInt32) m.pathInSchema.size() );
    for ( size_t i=0; i<m.pathInSchema.size(); ++i )
        writer.WriteBinary( m.pathInSchema[ i ].data, m.pathInSchema[ i ].size );
    writer.WriteI32Field( 4, m.codec );
    writer.WriteI64Field( 5, m.numValues );
    writer.WriteI64Field( 6, m.totalUncompressedSize );
    writer.WriteI64Field( 7, m.totalCompressedSize );
    if ( !m.keyValueMetaData.empty() )
    {
        writer.BeginListField( 8, THRIFT_COMPACT_STRUCT, (UInt32) m.keyValueMetaData.size() );
        for ( size_t i=0; i<m.keyValueMetaData.size(); ++i )
            WriteKeyValue( writer, m.keyValueMetaData[ i ] );
    }
    writer.WriteI64Field( 9, m.dataPageOffset );
    if ( m.hasIndexPageOffset ) writer.WriteI64Field( 10, m.indexPageOffset );
    if ( m.hasDictionaryPageOffset ) writer.WriteI64Field( 11, m.dictionaryPageOffset );
    if ( m.hasStatistics )
    {
        writer.WriteFieldHeader( THRIFT_COMPACT_STRUCT, 12 );
        WriteStatistics( writer, m.statistics );
    }
    if ( m.hasEncodingStats )
    {
        writer.BeginListField( 13, THRIFT_COMPACT_STRUCT, (UInt32) m.encodingStats.size() );
        for ( size_t i=0; i<m.encodingStats.size(); ++i )
        {
            writer.BeginStruct();
            writer.WriteI32Field( 1, m.encodingStats[ i ].pageType );
            writer.WriteI32Field( 2, m.encodingStats[ i ].encoding );
            writer.WriteI32Field( 3, m.encodingStats[ i ].count );
            writer.EndStruct();
        }
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteColumnChunk( CThriftCompactWriter& writer, const SParquetColumnChunk& c )
{GUCEF_TRACE;

    writer.BeginStruct();
    if ( c.hasFilePath ) writer.WriteBinaryField( 1, c.filePath.data, c.filePath.size );
    writer.WriteI64Field( 2, c.fileOffset );
    if ( c.hasMetaData )
    {
        writer.WriteFieldHeader( THRIFT_COMPACT_STRUCT, 3 );
        WriteColumnMetaData( writer, c.metaData );
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteRowGroup( CThriftCompactWriter& writer, const SParquetRowGroup& g )
{GUCEF_TRACE;

    writer.BeginStruct();
    writer.BeginListField( 1, THRIFT_COMPACT_STRUCT, (UInt32) g.columns.size() );
    for ( size_t i=0; i<g.columns.size(); ++i )
        WriteColumnChunk( writer, g.columns[ i ] );
    writer.WriteI64Field( 2, g.totalByteSize );
    writer.WriteI64Field( 3, g.numRows );
    if ( g.hasSortingColumns )
    {
        writer.BeginListField( 4, THRIFT_COMPACT_STRUCT, (UInt32) g.sortingColumns.size() );
        for ( size_t i=0; i<g.sortingColumns.size(); ++i )
        {
            writer.BeginStruct();
            writer.WriteI32Field( 1, g.sortingColumns[ i ].columnIdx );
            writer.WriteBoolField( 2, g.sortingColumns[ i ].descending );
            writer.WriteBoolField( 3, g.sortingColumns[ i ].nullsFirst );
            writer.EndStruct();
        }
    }
    if ( g.hasFileOffset ) writer.WriteI64Field( 5, g.fileOffset );
    if ( g.hasTotalCompressedSize ) writer.WriteI64Field( 6, g.totalCompressedSize );
    if ( g.hasOrdinal ) writer.WriteI16Field( 7, g.ordinal );
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WriteFileMetaData( CThriftCompactWriter& writer, const SParquetFileMetaData& f )
{GUCEF_TRACE;

    writer.BeginStruct();
    writer.WriteI32Field( 1, f.version );
    writer.BeginListField( 2, THRIFT_COMPACT_STRUCT, (UInt32) f.schema.size() );
    for ( size_t i=0; i<f.schema.size(); ++i )
        WriteSchemaElement( writer, f.schema[ i ] );
    writer.WriteI64Field( 3, f.numRows );
    writer.BeginListField( 4, THRIFT_COMPACT_STRUCT, (UInt32) f.rowGroups.size() );
    for ( size_t i=0; i<f.rowGroups.size(); ++i )
        WriteRowGroup( writer, f.rowGroups[ i ] );
    if ( !f.keyValueMetaData.empty() )
    {
        writer.BeginListField( 5, THRIFT_COMPACT_STRUCT, (UInt32) f.keyValueMetaData.size() );
        for ( size_t i=0; i<f.keyValueMetaData.size(); ++i )
            WriteKeyValue( writer, f.keyValueMetaData[ i ] );
    }
    if ( f.hasCreatedBy )
        writer.WriteBinaryField( 6, f.createdBy.data, f.createdBy.size );
    if ( f.hasColumnOrders )
    {
        writer.BeginListField( 7, THRIFT_COMPACT_STRUCT, (UInt32) f.columnOrders.size() );
        for ( size_t i=0; i<f.columnOrders.size(); ++i )
        {
            writer.BeginStruct();
            WriteEmptyStructField( writer, f.columnOrders[ i ] );
            writer.EndStruct();
        }
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

void
CParquetMetaDataSerializer::WritePageHeader( CThriftCompactWriter& writer, const SParquetPageHeader& h )
{GUCEF_TRACE;

    writer.BeginStruct();
    writer.WriteI32Field( 1, h.type );
    writer.WriteI32Field( 2, h.uncompressedPageSize );
    writer.WriteI32Field( 3, h.compressedPageSize );
    if ( h.hasCrc ) writer.WriteI32Field( 4, h.crc );
    if ( h.hasDataPageHeader )
    {
        const SParquetDataPageHeader& d = h.dataPageHeader;
        writer.BeginStructField( 5 );
        writer.WriteI32Field( 1, d.numValues );
        writer.WriteI32Field( 2, d.encoding );
        writer.WriteI32Field( 3, d.definitionLevelEncoding );
        writer.WriteI32Field( 4, d.repetitionLevelEncoding );
        if ( d.hasStatistics )
        {
            writer.WriteFieldHeader( THRIFT_COMPACT_STRUCT, 5 );
            WriteStatistics( writer, d.statistics );
        }
        writer.EndStruct();
    }
    if ( h.hasDictionaryPageHeader )
    {
        const SParquetDictionaryPageHeader& d = h.dictionaryPageHeader;
        writer.BeginStructField( 7 );
        writer.WriteI32Field( 1, d.numValues );
        writer.WriteI32Field( 2, d.encoding );
        if ( d.hasIsSorted ) writer.WriteBoolField( 3, d.isSorted );
        writer.EndStruct();
    }
    if ( h.hasDataPageHeaderV2 )
    {
        const SParquetDataPageHeaderV2& d = h.dataPageHeaderV2;
        writer.BeginStructField( 8 );
        writer.WriteI32Field( 1, d.numValues );
        writer.WriteI32Field( 2, d.numNulls );
        writer.WriteI32Field( 3, d.numRows );
        writer.WriteI32Field( 4, d.encoding );
        writer.WriteI32Field( 5, d.definitionLevelsByteLength );
        writer.WriteI32Field( 6, d.repetitionLevelsByteLength );
        writer.WriteBoolField( 7, d.isCompressed );
        if ( d.hasStatistics )
        {
            writer.WriteFieldHeader( THRIFT_COMPACT_STRUCT, 8 );
            WriteStatistics( writer, d.statistics );
        }
        writer.EndStruct();
    }
    writer.EndStruct();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetMetaDataSerializer::EncodingToString( Int32 encoding )
{GUCEF_TRACE;

    switch ( encoding )
    {
        case PARQUET_ENCODING_PLAIN: return "PLAIN";
        case PARQUET_ENCODING_PLAIN_DICTIONARY: return "PLAIN_DICTIONARY";
        case PARQUET_ENCODING_RLE: return "RLE";
        case PARQUET_ENCODING_BIT_PACKED: return "BIT_PACKED";
        case PARQUET_ENCODING_DELTA_BINARY_PACKED: return "DELTA_BINARY_PACKED";
        case PARQUET_ENCODING_DELTA_LENGTH_BYTE_ARRAY: return "DELTA_LENGTH_BYTE_ARRAY";
        case PARQUET_ENCODING_DELTA_BYTE_ARRAY: return "DELTA_BYTE_ARRAY";
        case PARQUET_ENCODING_RLE_DICTIONARY: return "RLE_DICTIONARY";
        case PARQUET_ENCODING_BYTE_STREAM_SPLIT: return "BYTE_STREAM_SPLIT";
        case PARQUET_ENCODING_ALP: return "ALP";
        default: return "encoding " + CORE::ToString( encoding );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetMetaDataSerializer::CompressionToString( Int32 codec )
{GUCEF_TRACE;

    switch ( codec )
    {
        case PARQUET_COMPRESSION_UNCOMPRESSED: return "UNCOMPRESSED";
        case PARQUET_COMPRESSION_SNAPPY: return "SNAPPY";
        case PARQUET_COMPRESSION_GZIP: return "GZIP";
        case PARQUET_COMPRESSION_LZO: return "LZO";
        case PARQUET_COMPRESSION_BROTLI: return "BROTLI";
        case PARQUET_COMPRESSION_LZ4: return "LZ4";
        case PARQUET_COMPRESSION_ZSTD: return "ZSTD";
        case PARQUET_COMPRESSION_LZ4_RAW: return "LZ4_RAW";
        default: return "codec " + CORE::ToString( codec );
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
