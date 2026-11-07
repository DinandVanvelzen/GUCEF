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

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedEncoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#include "tablecodecpluginPARQUET_CParquetSchemaMapping.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */

#include "tablecodecpluginPARQUET_CParquetTableWriter.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CParquetTableWriter::CreatedBy = "gucef tablecodecpluginPARQUET version 1.0.0";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the values of a column compare and are written */
enum EValueFamily
{
    FAMILY_SIGNED = 0   ,   /**< INT8 .. INT64, DECIMAL mantissas, DATE days, TIMESTAMP ticks */
    FAMILY_UNSIGNED     ,
    FAMILY_FLOAT        ,
    FAMILY_BOOL         ,
    FAMILY_BYTES            /**< UTF8, BINARY */
};

/**
 *  The rows of one column of the row group being built, spaced like a column batch: fixed values, bits, or offsets
 *  and bytes (without terminators); a validity bitmap for nullable columns. Buffers keep their capacity between row
 *  groups.
 */
class CParquetColumnStage
{
    public:

    CORE::CDataTableColumnDef def;
    CORE::CUtf8String name;
    Int32 physical;
    UInt32 physicalWidth;           /**< bytes of a PLAIN value, 0 for BOOLEAN and BYTE_ARRAY */
    UInt32 width;                   /**< bytes of a staged fixed value, 0 for bits and bytes */
    EValueFamily family;
    bool useDictionary;
    CORE::CDataTableBufferPool& pool;

    CORE::CDataTableBufferPtr values;       /**< fixed values, bits, or rows + 1 offsets */
    CORE::CDataTableBufferPtr bytes;
    CORE::CDataTableBufferPtr validity;
    UInt32 rows;
    UInt32 nullCount;
    UInt32 byteCount;

    bool hasFileStats;
    CORE::CDataTableColumnStats fileStats;

    explicit CParquetColumnStage( CORE::CDataTableBufferPool& bufferPool )
        : def(), name(), physical( 0 ), physicalWidth( 0 ), width( 0 ), family( FAMILY_SIGNED ), useDictionary( false ), pool( bufferPool )
        , values(), bytes(), validity(), rows( 0 ), nullCount( 0 ), byteCount( 0 ), hasFileStats( false ), fileStats()
    {
    }

    void Reset( void )
    {
        rows = 0;
        nullCount = 0;
        byteCount = 0;
    }

    inline bool IsValid( UInt32 r ) const
    {
        return validity.IsNULL() || 0 != ( static_cast< const UInt8* >( validity->GetData() )[ r >> 3 ] & ( 1u << ( r & 7 ) ) );
    }

    inline const UInt8* FixedValue( UInt32 r ) const { return static_cast< const UInt8* >( values->GetData() ) + (size_t) r * width; }

    inline bool GetBit( UInt32 r ) const { return 0 != ( static_cast< const UInt8* >( values->GetData() )[ r >> 3 ] & ( 1u << ( r & 7 ) ) ); }

    inline const UInt8* GetBytes( UInt32 r, UInt32& length ) const
    {
        const UInt32* offsets = static_cast< const UInt32* >( values->GetData() );
        length = offsets[ r + 1 ] - offsets[ r ];
        return static_cast< const UInt8* >( bytes->GetData() ) + offsets[ r ];
    }

    inline Int64 GetSigned( UInt32 r ) const
    {
        const UInt8* p = FixedValue( r );
        switch ( width )
        {
            case 1: return *reinterpret_cast< const Int8* >( p );
            case 2: { Int16 v; memcpy( &v, p, 2 ); return v; }
            case 4: { Int32 v; memcpy( &v, p, 4 ); return v; }
            default: { Int64 v; memcpy( &v, p, 8 ); return v; }
        }
    }

    inline UInt64 GetUnsigned( UInt32 r ) const
    {
        const UInt8* p = FixedValue( r );
        switch ( width )
        {
            case 1: return *p;
            case 2: { UInt16 v; memcpy( &v, p, 2 ); return v; }
            case 4: { UInt32 v; memcpy( &v, p, 4 ); return v; }
            default: { UInt64 v; memcpy( &v, p, 8 ); return v; }
        }
    }

    inline Float64 GetFloat( UInt32 r ) const
    {
        const UInt8* p = FixedValue( r );
        if ( 4 == width ) { Float32 v; memcpy( &v, p, 4 ); return v; }
        Float64 v; memcpy( &v, p, 8 ); return v;
    }

    /** Makes room for count more rows and extraBytes more variable bytes */
    CORE::TDataTableStatus Reserve( UInt32 count, UInt64 extraBytes )
    {
        const UInt64 needRows = (UInt64) rows + count;
        if ( needRows > 0x7FFFFFFF )
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        UInt64 valueBytes = 0;
        if ( FAMILY_BYTES == family ) valueBytes = ( needRows + 1 ) * 4;
        else if ( FAMILY_BOOL == family ) valueBytes = ( needRows + 7 ) / 8;
        else valueBytes = needRows * width;
        CORE::TDataTableStatus status = Grow( values, valueBytes, FAMILY_BYTES == family ? ( (UInt64) rows + 1 ) * 4 : ( FAMILY_BOOL == family ? ( (UInt64) rows + 7 ) / 8 : (UInt64) rows * width ) );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        if ( FAMILY_BYTES == family )
        {
            if ( 0 == rows )
                static_cast< UInt32* >( values->GetData() )[ 0 ] = 0;
            const UInt64 needBytes = (UInt64) byteCount + extraBytes;
            if ( needBytes > 0xFFFFFFFFULL - 4096 )
                return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
            status = Grow( bytes, needBytes > 0 ? needBytes : 1, byteCount );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }
        if ( def.nullable )
        {
            status = Grow( validity, ( needRows + 7 ) / 8, ( (UInt64) rows + 7 ) / 8 );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }
        return CORE::DATATABLE_STATUS_OK;
    }

    private:

    CORE::TDataTableStatus Grow( CORE::CDataTableBufferPtr& buffer, UInt64 needed, UInt64 used )
    {
        if ( needed > 0x7FFFFFFFULL )
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        if ( buffer.IsNULL() )
            return pool.Acquire( needed < 4096 ? 4096 : (UInt32) needed, buffer );
        if ( buffer->GetCapacity() >= needed )
            return CORE::DATATABLE_STATUS_OK;
        UInt64 grown = (UInt64) buffer->GetCapacity() * 2;
        if ( grown < needed )
            grown = needed;
        if ( grown > 0x7FFFFFFFULL )
            grown = needed;
        buffer->SetDataSize( (UInt32) used );
        return pool.Reserve( buffer, (UInt32) grown );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Bytes appended to a pooled buffer that grows by doubling */
class CByteSink
{
    public:

    explicit CByteSink( CORE::CDataTableBufferPool& pool ) : m_pool( pool ), m_buffer(), m_size( 0 ), m_status( CORE::DATATABLE_STATUS_OK ) {}

    void Clear( void ) { m_size = 0; m_status = CORE::DATATABLE_STATUS_OK; }

    /** Room for more bytes; returns the place to write them, GUCEF_NULL when no memory could be had */
    UInt8* Extend( UInt32 count )
    {
        if ( CORE::DATATABLE_STATUS_OK != m_status )
            return GUCEF_NULL;
        const UInt64 needed = (UInt64) m_size + count;
        if ( needed > 0x7FFFFFFFULL )
        {
            m_status = CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
            return GUCEF_NULL;
        }
        if ( m_buffer.IsNULL() )
            m_status = m_pool.Acquire( needed < 65536 ? 65536 : (UInt32) needed, m_buffer );
        else
        if ( m_buffer->GetCapacity() < needed )
        {
            UInt64 grown = (UInt64) m_buffer->GetCapacity() * 2;
            if ( grown < needed ) grown = needed;
            if ( grown > 0x7FFFFFFFULL ) grown = needed;
            m_buffer->SetDataSize( m_size );
            m_status = m_pool.Reserve( m_buffer, (UInt32) grown );
        }
        if ( CORE::DATATABLE_STATUS_OK != m_status )
            return GUCEF_NULL;
        UInt8* place = static_cast< UInt8* >( m_buffer->GetData() ) + m_size;
        m_size += count;
        return place;
    }

    void Append( const void* data, UInt32 count )
    {
        UInt8* place = Extend( count );
        if ( GUCEF_NULL != place && count > 0 )
            memcpy( place, data, count );
    }

    inline const UInt8* GetData( void ) const { return m_buffer.IsNULL() ? GUCEF_NULL : static_cast< const UInt8* >( m_buffer->GetData() ); }
    inline UInt32 GetSize( void ) const { return m_size; }
    inline CORE::TDataTableStatus GetStatus( void ) const { return m_status; }

    private:

    CORE::CDataTableBufferPool& m_pool;
    CORE::CDataTableBufferPtr m_buffer;
    UInt32 m_size;
    CORE::TDataTableStatus m_status;
};

/** Minimum, maximum, null and NaN counts over a page or a chunk, in the order of the value family */
struct SStatsAccumulator
{
    bool hasValue;
    Int64 minSigned, maxSigned;
    UInt64 minUnsigned, maxUnsigned;
    Float64 minFloat, maxFloat;
    const UInt8* minBytes;
    UInt32 minLength;
    const UInt8* maxBytes;
    UInt32 maxLength;
    UInt64 nullCount;
    UInt64 nanCount;

    SStatsAccumulator( void ) { Clear(); }

    void Clear( void )
    {
        hasValue = false;
        minSigned = maxSigned = 0;
        minUnsigned = maxUnsigned = 0;
        minFloat = maxFloat = 0.0;
        minBytes = maxBytes = GUCEF_NULL;
        minLength = maxLength = 0;
        nullCount = nanCount = 0;
    }

    static Int32 CompareBytes( const UInt8* a, UInt32 aLength, const UInt8* b, UInt32 bLength )
    {
        const UInt32 common = aLength < bLength ? aLength : bLength;
        const int c = common > 0 ? memcmp( a, b, common ) : 0;
        if ( 0 != c )
            return c < 0 ? -1 : 1;
        return aLength == bLength ? 0 : ( aLength < bLength ? -1 : 1 );
    }

    inline void AddSigned( Int64 v )
    {
        if ( !hasValue ) { minSigned = maxSigned = v; hasValue = true; return; }
        if ( v < minSigned ) minSigned = v;
        if ( v > maxSigned ) maxSigned = v;
    }

    inline void AddUnsigned( UInt64 v )
    {
        if ( !hasValue ) { minUnsigned = maxUnsigned = v; hasValue = true; return; }
        if ( v < minUnsigned ) minUnsigned = v;
        if ( v > maxUnsigned ) maxUnsigned = v;
    }

    inline void AddFloat( Float64 v )
    {
        if ( v != v ) { ++nanCount; return; }
        if ( !hasValue ) { minFloat = maxFloat = v; hasValue = true; return; }
        if ( v < minFloat ) minFloat = v;
        if ( v > maxFloat ) maxFloat = v;
    }

    inline void AddBytes( const UInt8* v, UInt32 length )
    {
        if ( !hasValue ) { minBytes = maxBytes = v; minLength = maxLength = length; hasValue = true; return; }
        if ( CompareBytes( v, length, minBytes, minLength ) < 0 ) { minBytes = v; minLength = length; }
        if ( CompareBytes( v, length, maxBytes, maxLength ) > 0 ) { maxBytes = v; maxLength = length; }
    }

    void Merge( const SStatsAccumulator& other, EValueFamily family )
    {
        nullCount += other.nullCount;
        nanCount += other.nanCount;
        if ( !other.hasValue )
            return;
        switch ( family )
        {
            case FAMILY_SIGNED:
            case FAMILY_BOOL: { AddSigned( other.minSigned ); AddSigned( other.maxSigned ); break; }
            case FAMILY_UNSIGNED: { AddUnsigned( other.minUnsigned ); AddUnsigned( other.maxUnsigned ); break; }
            case FAMILY_FLOAT: { const UInt64 nans = nanCount; AddFloat( other.minFloat ); AddFloat( other.maxFloat ); nanCount = nans; break; }
            case FAMILY_BYTES: { AddBytes( other.minBytes, other.minLength ); AddBytes( other.maxBytes, other.maxLength ); break; }
        }
    }

    /** The PLAIN bytes of a statistics value; store holds 8 bytes for fixed values */
    static void EncodeValue( const CParquetColumnStage& stage, const SStatsAccumulator& acc, bool minimum, UInt8* store, SParquetBytes& bytes )
    {
        switch ( stage.family )
        {
            case FAMILY_SIGNED:
            {
                const Int64 v = minimum ? acc.minSigned : acc.maxSigned;
                if ( 4 == stage.physicalWidth ) { StoreLE32( store, (UInt32) (Int32) v ); bytes = SParquetBytes( store, 4 ); }
                else { StoreLE64( store, (UInt64) v ); bytes = SParquetBytes( store, 8 ); }
                break;
            }
            case FAMILY_UNSIGNED:
            {
                const UInt64 v = minimum ? acc.minUnsigned : acc.maxUnsigned;
                if ( 4 == stage.physicalWidth ) { StoreLE32( store, (UInt32) v ); bytes = SParquetBytes( store, 4 ); }
                else { StoreLE64( store, v ); bytes = SParquetBytes( store, 8 ); }
                break;
            }
            case FAMILY_FLOAT:
            {
                Float64 v = minimum ? acc.minFloat : acc.maxFloat;
                // the specification asks for -0.0 as a zero minimum and +0.0 as a zero maximum
                if ( 0.0 == v )
                    v = minimum ? -0.0 : 0.0;
                if ( 4 == stage.physicalWidth ) { StoreLEFloat32( store, (Float32) v ); bytes = SParquetBytes( store, 4 ); }
                else { StoreLEFloat64( store, v ); bytes = SParquetBytes( store, 8 ); }
                break;
            }
            case FAMILY_BOOL:
            {
                store[ 0 ] = (UInt8) ( 0 != ( minimum ? acc.minSigned : acc.maxSigned ) ? 1 : 0 );
                bytes = SParquetBytes( store, 1 );
                break;
            }
            case FAMILY_BYTES:
            {
                bytes = minimum ? SParquetBytes( acc.minBytes, acc.minLength ) : SParquetBytes( acc.maxBytes, acc.maxLength );
                break;
            }
        }
    }

    /** The Statistics structure; min and max are left out for text above maxValueBytes */
    void Fill( const CParquetColumnStage& stage, UInt32 maxValueBytes, UInt8* minStore, UInt8* maxStore, SParquetStatistics& s ) const
    {
        s = SParquetStatistics();
        s.hasNullCount = true;
        s.nullCount = (Int64) nullCount;
        if ( FAMILY_FLOAT == stage.family )
        {
            s.hasNanCount = true;
            s.nanCount = (Int64) nanCount;
        }
        if ( !hasValue )
            return;
        if ( FAMILY_BYTES == stage.family && ( minLength > maxValueBytes || maxLength > maxValueBytes ) )
            return;
        EncodeValue( stage, *this, true, minStore, s.minValue );
        EncodeValue( stage, *this, false, maxStore, s.maxValue );
        s.hasMinValue = s.hasMaxValue = true;
        s.hasIsMinValueExact = s.hasIsMaxValueExact = true;
        s.isMinValueExact = s.isMaxValueExact = true;
    }

    /** The statistics in the native form the table layer uses */
    void ToColumnStats( const CParquetColumnStage& stage, UInt64 rows, CORE::CDataTableColumnStats& stats ) const
    {
        stats.Clear();
        stats.hasNullCount = true;
        stats.nullCount = nullCount;
        stats.valueCount = rows;
        if ( FAMILY_FLOAT == stage.family )
        {
            stats.hasNanCount = true;
            stats.nanCount = nanCount;
        }
        if ( !hasValue )
            return;
        stats.hasMinMax = true;
        stats.minExact = stats.maxExact = true;
        switch ( stage.family )
        {
            case FAMILY_SIGNED:
            {
                if ( CORE::DATATABLE_TYPE_DATE == stage.def.type )
                {
                    stats.minValue = CORE::CDataTableValues::Date( (Int32) minSigned );
                    stats.maxValue = CORE::CDataTableValues::Date( (Int32) maxSigned );
                }
                else
                {
                    stats.minValue = CORE::CVariant( minSigned );
                    stats.maxValue = CORE::CVariant( maxSigned );
                }
                break;
            }
            case FAMILY_UNSIGNED: { stats.minValue = CORE::CVariant( minUnsigned ); stats.maxValue = CORE::CVariant( maxUnsigned ); break; }
            case FAMILY_FLOAT: { stats.minValue = CORE::CVariant( minFloat ); stats.maxValue = CORE::CVariant( maxFloat ); break; }
            case FAMILY_BOOL: { stats.minValue = CORE::CVariant( 0 != minSigned ); stats.maxValue = CORE::CVariant( 0 != maxSigned ); break; }
            case FAMILY_BYTES:
            {
                if ( CORE::DATATABLE_TYPE_UTF8 == stage.def.type )
                {
                    GUCEF::vector< char > text( (size_t) minLength + 1, '\0' );
                    if ( minLength > 0 ) memcpy( &text[ 0 ], minBytes, minLength );
                    stats.minValue.Set( &text[ 0 ], minLength + 1, GUCEF_DATATYPE_UTF8_STRING, false );
                    text.assign( (size_t) maxLength + 1, '\0' );
                    if ( maxLength > 0 ) memcpy( &text[ 0 ], maxBytes, maxLength );
                    stats.maxValue.Set( &text[ 0 ], maxLength + 1, GUCEF_DATATYPE_UTF8_STRING, false );
                }
                else
                {
                    stats.minValue.Set( minBytes, minLength, GUCEF_DATATYPE_BINARY_BLOB, false );
                    stats.maxValue.Set( maxBytes, maxLength, GUCEF_DATATYPE_BINARY_BLOB, false );
                }
                break;
            }
        }
    }
};

/** A dictionary under construction: distinct values in order of first use, found through an open addressing hash */
class CDictionaryBuilder
{
    public:

    CDictionaryBuilder( void ) : m_offsets(), m_bytes(), m_slots(), m_mask( 0 ), m_plainBytes( 0 ) { m_offsets.push_back( 0 ); }

    inline UInt32 GetCount( void ) const { return (UInt32) m_offsets.size() - 1; }
    inline UInt64 GetPlainBytes( void ) const { return m_plainBytes; }
    inline const UInt8* GetValue( UInt32 index, UInt32& length ) const
    {
        length = m_offsets[ index + 1 ] - m_offsets[ index ];
        return m_bytes.empty() ? GUCEF_NULL : &m_bytes[ 0 ] + m_offsets[ index ];
    }

    /** The index of the value, added when it is new; plainWidth is the PLAIN size of a fixed value, 0 for byte arrays */
    UInt32 Insert( const UInt8* data, UInt32 length, UInt32 plainWidth )
    {
        if ( ( GetCount() + 1 ) * 2 > m_slots.size() )
            Rehash( m_slots.empty() ? 1024 : (UInt32) m_slots.size() * 2 );
        const UInt64 hash = Hash( data, length );
        UInt32 slot = (UInt32) hash & m_mask;
        for ( ;; )
        {
            const UInt32 entry = m_slots[ slot ];
            if ( 0 == entry )
                break;
            UInt32 entryLength = 0;
            const UInt8* entryData = GetValue( entry - 1, entryLength );
            if ( entryLength == length && ( 0 == length || 0 == memcmp( entryData, data, length ) ) )
                return entry - 1;
            slot = ( slot + 1 ) & m_mask;
        }
        const UInt32 index = GetCount();
        if ( length > 0 )
            m_bytes.insert( m_bytes.end(), data, data + length );
        m_offsets.push_back( (UInt32) m_bytes.size() );
        m_slots[ slot ] = index + 1;
        m_plainBytes += 0 != plainWidth ? plainWidth : 4 + length;
        return index;
    }

    /** Takes out every value from index count on */
    void Truncate( UInt32 count, UInt32 plainWidth )
    {
        if ( count >= GetCount() )
            return;
        m_bytes.resize( m_offsets[ count ] );
        m_offsets.resize( count + 1 );
        m_plainBytes = 0 != plainWidth ? (UInt64) count * plainWidth : m_bytes.size() + 4ULL * count;
        Rehash( (UInt32) m_slots.size() );
    }

    private:

    static UInt64 Hash( const UInt8* data, UInt32 length )
    {
        UInt64 hash = 1469598103934665603ULL;
        for ( UInt32 i=0; i<length; ++i )
        {
            hash ^= data[ i ];
            hash *= 1099511628211ULL;
        }
        return hash ^ ( hash >> 29 );
    }

    void Rehash( UInt32 slotCount )
    {
        m_slots.assign( slotCount, 0 );
        m_mask = slotCount - 1;
        for ( UInt32 i=0; i<GetCount(); ++i )
        {
            UInt32 length = 0;
            const UInt8* data = GetValue( i, length );
            UInt32 slot = (UInt32) Hash( data, length ) & m_mask;
            while ( 0 != m_slots[ slot ] )
                slot = ( slot + 1 ) & m_mask;
            m_slots[ slot ] = i + 1;
        }
    }

    GUCEF::vector< UInt32 > m_offsets;
    GUCEF::vector< UInt8 > m_bytes;
    GUCEF::vector< UInt32 > m_slots;
    UInt32 m_mask;
    UInt64 m_plainBytes;
};

/** Writes the PLAIN form of a staged fixed value of the physical type */
inline void
PutPlainFixed( const CParquetColumnStage& stage, UInt32 r, UInt8* out )
{
    switch ( stage.family )
    {
        case FAMILY_SIGNED: { if ( 4 == stage.physicalWidth ) StoreLE32( out, (UInt32) (Int32) stage.GetSigned( r ) ); else StoreLE64( out, (UInt64) stage.GetSigned( r ) ); break; }
        case FAMILY_UNSIGNED: { if ( 4 == stage.physicalWidth ) StoreLE32( out, (UInt32) stage.GetUnsigned( r ) ); else StoreLE64( out, stage.GetUnsigned( r ) ); break; }
        case FAMILY_FLOAT:
        {
            if ( 4 == stage.physicalWidth ) { Float32 v; memcpy( &v, stage.FixedValue( r ), 4 ); StoreLEFloat32( out, v ); }
            else { Float64 v; memcpy( &v, stage.FixedValue( r ), 8 ); StoreLEFloat64( out, v ); }
            break;
        }
        default: { break; }
    }
}

/** Adds the value of a staged row to statistics */
inline void
AddStat( const CParquetColumnStage& stage, UInt32 r, SStatsAccumulator& acc )
{
    switch ( stage.family )
    {
        case FAMILY_SIGNED: { acc.AddSigned( stage.GetSigned( r ) ); break; }
        case FAMILY_UNSIGNED: { acc.AddUnsigned( stage.GetUnsigned( r ) ); break; }
        case FAMILY_FLOAT: { acc.AddFloat( stage.GetFloat( r ) ); break; }
        case FAMILY_BOOL: { acc.AddSigned( stage.GetBit( r ) ? 1 : 0 ); break; }
        case FAMILY_BYTES: { UInt32 length = 0; const UInt8* value = stage.GetBytes( r, length ); acc.AddBytes( value, length ); break; }
    }
}

/** The order of two staged rows on one column: values ascending, NaN after numbers, nulls last */
Int32
CompareStagedRows( const CParquetColumnStage& stage, UInt32 a, UInt32 b )
{
    const bool validA = stage.IsValid( a );
    const bool validB = stage.IsValid( b );
    if ( !validA || !validB )
        return validA == validB ? 0 : ( validA ? -1 : 1 );
    switch ( stage.family )
    {
        case FAMILY_SIGNED: { const Int64 x = stage.GetSigned( a ), y = stage.GetSigned( b ); return x < y ? -1 : ( x > y ? 1 : 0 ); }
        case FAMILY_UNSIGNED: { const UInt64 x = stage.GetUnsigned( a ), y = stage.GetUnsigned( b ); return x < y ? -1 : ( x > y ? 1 : 0 ); }
        case FAMILY_FLOAT:
        {
            const Float64 x = stage.GetFloat( a ), y = stage.GetFloat( b );
            const bool nanX = x != x, nanY = y != y;
            if ( nanX || nanY )
                return nanX == nanY ? 0 : ( nanX ? 1 : -1 );
            return x < y ? -1 : ( x > y ? 1 : 0 );
        }
        case FAMILY_BOOL: { const int x = stage.GetBit( a ) ? 1 : 0, y = stage.GetBit( b ) ? 1 : 0; return x - y; }
        case FAMILY_BYTES:
        {
            UInt32 la = 0, lb = 0;
            const UInt8* va = stage.GetBytes( a, la );
            const UInt8* vb = stage.GetBytes( b, lb );
            return SStatsAccumulator::CompareBytes( va, la, vb, lb );
        }
    }
    return 0;
}

/** Whether the batch column carries the logical type of the schema column */
bool
IsSameColumnType( const CORE::CDataTableColumnBatch& column, const CORE::CDataTableColumnDef& def )
{
    if ( column.GetLogicalType() != def.type )
        return false;
    if ( CORE::DATATABLE_TYPE_DECIMAL == def.type && column.GetDecimalScale() != def.decimalScale )
        return false;
    if ( CORE::DATATABLE_TYPE_TIMESTAMP == def.type && ( column.GetTimeUnit() != def.timeUnit || column.IsUtcAdjusted() != def.utcAdjusted ) )
        return false;
    return true;
}

/**
 *  Encodes the staged rows of one column as a column chunk: the dictionary page (when one is used) and the data pages,
 *  their statistics and the metadata of the chunk
 */
class CChunkEncoder
{
    public:

    CChunkEncoder( const CParquetColumnStage& stage, const CORE::CDataTableWriterOptions& options, CParquetCompression& compression,
                   CORE::CDataTableBufferPool& pool )
        : m_stage( stage ), m_options( options ), m_compression( compression ), m_pages( pool ), m_dictionaryPage( pool ), m_payload( pool )
        , m_dictionary(), m_indices(), m_chunkStats(), m_uncompressedBytes( 0 ), m_compressedBytes( 0 ), m_firstValuesOffset( 0 ), m_hasFirstValues( false )
        , m_dictionaryPages( 0 ), m_plainPages( 0 ), m_dictionaryDataPages( 0 ), m_alignedPages( true ), m_error()
    {
    }

    CORE::TDataTableStatus Encode( void )
    {
        const UInt32 rows = m_stage.rows;
        bool dictionaryActive = m_stage.useDictionary;
        const UInt32 plainWidth = m_stage.physicalWidth;

        UInt32 rowsPerPage = m_options.pageRowCount > 0 ? m_options.pageRowCount : 16384;
        if ( 0 != m_stage.width && FAMILY_BYTES != m_stage.family )
        {
            const UInt32 byWidth = m_options.pageByteLimit / ( 0 != plainWidth ? plainWidth : 1 );
            if ( byWidth > 0 && byWidth < rowsPerPage )
                rowsPerPage = byWidth;
        }
        UInt32 start = 0;
        while ( start < rows )
        {
            UInt32 end = rows - start > rowsPerPage ? start + rowsPerPage : rows;
            if ( FAMILY_BYTES == m_stage.family )
            {
                // a text page is cut early at the byte limit, with at least one row
                const UInt32* offsets = static_cast< const UInt32* >( m_stage.values->GetData() );
                UInt32 cut = start + 1;
                while ( cut < end && (UInt64) offsets[ cut ] - offsets[ start ] + 4ULL * ( cut - start ) < m_options.pageByteLimit )
                    ++cut;
                end = cut;
            }

            bool pageUsesDictionary = false;
            if ( dictionaryActive )
            {
                const UInt32 countBefore = m_dictionary.GetCount();
                m_indices.clear();
                for ( UInt32 r=start; r<end; ++r )
                {
                    if ( !m_stage.IsValid( r ) )
                        continue;
                    UInt32 length = 0;
                    const UInt8* value = GUCEF_NULL;
                    if ( FAMILY_BYTES == m_stage.family )
                        value = m_stage.GetBytes( r, length );
                    else
                    {
                        value = m_stage.FixedValue( r );
                        length = m_stage.width;
                    }
                    m_indices.push_back( m_dictionary.Insert( value, length, plainWidth ) );
                }
                if ( m_dictionary.GetPlainBytes() > m_options.dictionaryByteLimit )
                {
                    // the dictionary would pass its limit: this page and the rest of the chunk are PLAIN
                    m_dictionary.Truncate( countBefore, plainWidth );
                    dictionaryActive = false;
                }
                else
                {
                    pageUsesDictionary = true;
                }
            }
            CORE::TDataTableStatus status = EncodePage( start, end, pageUsesDictionary );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            start = end;
        }
        if ( m_dictionaryDataPages > 0 )
            return EncodeDictionaryPage();
        return CORE::DATATABLE_STATUS_OK;
    }

    inline const CByteSink& GetDictionaryPage( void ) const { return m_dictionaryPage; }
    inline const CByteSink& GetDataPages( void ) const { return m_pages; }
    inline const SStatsAccumulator& GetChunkStats( void ) const { return m_chunkStats; }
    inline UInt64 GetUncompressedBytes( void ) const { return m_uncompressedBytes; }
    inline UInt64 GetCompressedBytes( void ) const { return m_compressedBytes; }
    inline bool UsesDictionary( void ) const { return m_dictionaryDataPages > 0; }
    inline UInt32 GetPlainPages( void ) const { return m_plainPages; }
    inline UInt32 GetDictionaryDataPages( void ) const { return m_dictionaryDataPages; }
    inline const CORE::CString& GetError( void ) const { return m_error; }

    /** Offset of the values of the first data page within the data pages, when they are PLAIN and stored uncompressed */
    inline bool GetFirstValuesOffset( UInt32& offset ) const { offset = m_firstValuesOffset; return m_hasFirstValues; }

    /** Whether every page of the chunk has its values at a multiple of 8 from the start of the chunk */
    inline bool HasAlignedPages( void ) const { return m_hasFirstValues && m_alignedPages && 0 == m_dictionaryPages; }

    private:

    CORE::TDataTableStatus Store( CByteSink& sink, const SParquetPageHeader& headerTemplate, UInt32 levelBytes, bool recordValues )
    {
        const UInt8* payload = m_payload.GetData();
        const UInt32 payloadSize = m_payload.GetSize();
        if ( CORE::DATATABLE_STATUS_OK != m_payload.GetStatus() )
            return m_payload.GetStatus();
        const UInt8* stored = payload;
        UInt32 storedSize = payloadSize;
        if ( m_compression.IsCompressing() )
        {
            CORE::TDataTableStatus status = m_compression.Compress( payload, payloadSize, m_compressed, m_error );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            stored = static_cast< const UInt8* >( m_compressed.GetConstBufferPtr() );
            storedSize = m_compressed.GetDataSize();
        }
        SParquetPageHeader header = headerTemplate;
        header.uncompressedPageSize = (Int32) payloadSize;
        header.compressedPageSize = (Int32) storedSize;
        m_header.Clear( true );
        {
            CThriftCompactWriter thrift( m_header );
            CParquetMetaDataSerializer::WritePageHeader( thrift, header );
        }
        const bool alignValues = recordValues && !m_compression.IsCompressing() && 0 == ( sink.GetSize() & 7 );
        if ( alignValues && 0 != ( ( m_header.GetDataSize() + levelBytes ) & 7 ) )
        {
            // values in place need them at a multiple of 8: the header grows by long form field headers, a byte each
            const UInt32 extra = 8 - ( ( m_header.GetDataSize() + levelBytes ) & 7 );
            m_header.Clear( true );
            CThriftCompactWriter thrift( m_header );
            thrift.SetLongFormFieldCount( extra );
            CParquetMetaDataSerializer::WritePageHeader( thrift, header );
        }
        const UInt32 headerSize = m_header.GetDataSize();
        if ( alignValues && 0 == ( ( headerSize + levelBytes ) & 7 ) && 0 == ( payloadSize & 7 ) )
            m_alignedPages = true;
        else
        if ( recordValues )
            m_alignedPages = false;

        if ( recordValues && !m_hasFirstValues && !m_compression.IsCompressing() )
        {
            m_firstValuesOffset = sink.GetSize() + headerSize + levelBytes;
            m_hasFirstValues = true;
        }
        if ( !recordValues )
            m_alignedPages = false;
        sink.Append( m_header.GetConstBufferPtr(), headerSize );
        sink.Append( stored, storedSize );
        if ( CORE::DATATABLE_STATUS_OK != sink.GetStatus() )
            return sink.GetStatus();
        m_uncompressedBytes += (UInt64) headerSize + payloadSize;
        m_compressedBytes += (UInt64) headerSize + storedSize;
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::TDataTableStatus EncodePage( UInt32 start, UInt32 end, bool useDictionary )
    {
        const UInt32 rowCount = end - start;
        m_payload.Clear();
        SStatsAccumulator pageStats;

        // definition levels of an OPTIONAL column: the validity bits as RLE / bit-packed runs, 4 byte length first
        UInt32 levelBytes = 0;
        if ( m_stage.def.nullable )
        {
            UInt32 nulls = 0;
            for ( UInt32 r=start; r<end; ++r )
                nulls += m_stage.IsValid( r ) ? 0 : 1;
            pageStats.nullCount = nulls;
            m_levels.Clear( true );
            if ( 0 == nulls )
            {
                CRleBitPackedEncoder::EncodeRepeatedBit( true, rowCount, m_levels );
            }
            else
            {
                const UInt8* validity = static_cast< const UInt8* >( m_stage.validity->GetData() );
                if ( 0 == ( start & 7 ) )
                {
                    CRleBitPackedEncoder::EncodeBits( validity + start / 8, rowCount, m_levels );
                }
                else
                {
                    m_shifted.assign( ( rowCount + 7 ) / 8, 0 );
                    for ( UInt32 i=0; i<rowCount; ++i )
                    {
                        if ( m_stage.IsValid( start + i ) )
                            m_shifted[ i >> 3 ] |= (UInt8) ( 1u << ( i & 7 ) );
                    }
                    CRleBitPackedEncoder::EncodeBits( &m_shifted[ 0 ], rowCount, m_levels );
                }
            }
            UInt8 length[ 4 ];
            StoreLE32( length, m_levels.GetDataSize() );
            m_payload.Append( length, 4 );
            m_payload.Append( m_levels.GetConstBufferPtr(), m_levels.GetDataSize() );
            levelBytes = 4 + m_levels.GetDataSize();
        }

        Int32 encoding = PARQUET_ENCODING_PLAIN;
        if ( useDictionary )
        {
            encoding = PARQUET_ENCODING_RLE_DICTIONARY;
            for ( UInt32 r=start; r<end; ++r )
            {
                if ( m_stage.IsValid( r ) )
                    AddStat( m_stage, r, pageStats );
            }
            const UInt32 count = m_dictionary.GetCount();
            UInt32 bitWidth = CRleBitPackedEncoder::BitWidthFor( count > 0 ? count - 1 : 0 );
            if ( 0 == bitWidth )
                bitWidth = 1;
            const UInt8 widthByte = (UInt8) bitWidth;
            m_payload.Append( &widthByte, 1 );
            m_levels.Clear( true );
            if ( !m_indices.empty() )
                CRleBitPackedEncoder::EncodeValues( &m_indices[ 0 ], (UInt32) m_indices.size(), bitWidth, m_levels );
            m_payload.Append( m_levels.GetConstBufferPtr(), m_levels.GetDataSize() );
            ++m_dictionaryDataPages;
        }
        else
        {
            switch ( m_stage.family )
            {
                case FAMILY_BOOL:
                {
                    const UInt32 dense = rowCount - (UInt32) pageStats.nullCount;
                    UInt8* bits = m_payload.Extend( ( dense + 7 ) / 8 );
                    if ( GUCEF_NULL == bits )
                        return m_payload.GetStatus();
                    memset( bits, 0, ( dense + 7 ) / 8 );
                    UInt32 k = 0;
                    for ( UInt32 r=start; r<end; ++r )
                    {
                        if ( !m_stage.IsValid( r ) )
                            continue;
                        if ( m_stage.GetBit( r ) )
                            bits[ k >> 3 ] |= (UInt8) ( 1u << ( k & 7 ) );
                        ++k;
                        AddStat( m_stage, r, pageStats );
                    }
                    break;
                }
                case FAMILY_BYTES:
                {
                    for ( UInt32 r=start; r<end; ++r )
                    {
                        if ( !m_stage.IsValid( r ) )
                            continue;
                        UInt32 length = 0;
                        const UInt8* value = m_stage.GetBytes( r, length );
                        UInt8* place = m_payload.Extend( 4 + length );
                        if ( GUCEF_NULL == place )
                            return m_payload.GetStatus();
                        StoreLE32( place, length );
                        if ( length > 0 )
                            memcpy( place + 4, value, length );
                        pageStats.AddBytes( value, length );
                    }
                    break;
                }
                default:
                {
                    const UInt32 width = m_stage.physicalWidth;
                    const UInt32 dense = rowCount - (UInt32) pageStats.nullCount;
                    UInt8* out = m_payload.Extend( dense * width );
                    if ( GUCEF_NULL == out && dense > 0 )
                        return m_payload.GetStatus();
                    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
                    if ( 0 == pageStats.nullCount && m_stage.width == width )
                    {
                        // the staged span is the PLAIN form already
                        memcpy( out, m_stage.FixedValue( start ), (size_t) rowCount * width );
                        for ( UInt32 r=start; r<end; ++r )
                            AddStat( m_stage, r, pageStats );
                        break;
                    }
                    #endif
                    UInt32 k = 0;
                    for ( UInt32 r=start; r<end; ++r )
                    {
                        if ( !m_stage.IsValid( r ) )
                            continue;
                        PutPlainFixed( m_stage, r, out + (size_t) ( k++ ) * width );
                        AddStat( m_stage, r, pageStats );
                    }
                    break;
                }
            }
            ++m_plainPages;
        }

        SParquetPageHeader header;
        header.type = PARQUET_PAGE_DATA_PAGE;
        header.hasDataPageHeader = true;
        header.dataPageHeader.numValues = (Int32) rowCount;
        header.dataPageHeader.encoding = encoding;
        header.dataPageHeader.definitionLevelEncoding = PARQUET_ENCODING_RLE;
        header.dataPageHeader.repetitionLevelEncoding = PARQUET_ENCODING_RLE;
        UInt8 minStore[ 8 ];
        UInt8 maxStore[ 8 ];
        if ( m_options.writePageStatistics )
        {
            pageStats.Fill( m_stage, m_options.statisticsMaxValueBytes, minStore, maxStore, header.dataPageHeader.statistics );
            header.dataPageHeader.hasStatistics = true;
        }
        m_chunkStats.Merge( pageStats, m_stage.family );
        return Store( m_pages, header, levelBytes, !useDictionary && FAMILY_BYTES != m_stage.family && FAMILY_BOOL != m_stage.family );
    }

    CORE::TDataTableStatus EncodeDictionaryPage( void )
    {
        m_payload.Clear();
        const UInt32 count = m_dictionary.GetCount();
        for ( UInt32 i=0; i<count; ++i )
        {
            UInt32 length = 0;
            const UInt8* value = m_dictionary.GetValue( i, length );
            if ( FAMILY_BYTES == m_stage.family )
            {
                UInt8* place = m_payload.Extend( 4 + length );
                if ( GUCEF_NULL == place )
                    return m_payload.GetStatus();
                StoreLE32( place, length );
                if ( length > 0 )
                    memcpy( place + 4, value, length );
            }
            else
            {
                // a staged fixed value in its PLAIN form, through a one row view of the value
                UInt8* place = m_payload.Extend( m_stage.physicalWidth );
                if ( GUCEF_NULL == place )
                    return m_payload.GetStatus();
                UInt8 raw[ 8 ] = { 0 };
                memcpy( raw, value, length );
                switch ( m_stage.family )
                {
                    case FAMILY_SIGNED:
                    {
                        Int64 v = 0;
                        switch ( m_stage.width ) { case 1: v = (Int8) raw[ 0 ]; break; case 2: { Int16 x; memcpy( &x, raw, 2 ); v = x; break; } case 4: { Int32 x; memcpy( &x, raw, 4 ); v = x; break; } default: memcpy( &v, raw, 8 ); break; }
                        if ( 4 == m_stage.physicalWidth ) StoreLE32( place, (UInt32) (Int32) v ); else StoreLE64( place, (UInt64) v );
                        break;
                    }
                    case FAMILY_UNSIGNED:
                    {
                        UInt64 v = 0;
                        switch ( m_stage.width ) { case 1: v = raw[ 0 ]; break; case 2: { UInt16 x; memcpy( &x, raw, 2 ); v = x; break; } case 4: { UInt32 x; memcpy( &x, raw, 4 ); v = x; break; } default: memcpy( &v, raw, 8 ); break; }
                        if ( 4 == m_stage.physicalWidth ) StoreLE32( place, (UInt32) v ); else StoreLE64( place, v );
                        break;
                    }
                    case FAMILY_FLOAT:
                    {
                        if ( 4 == m_stage.physicalWidth ) { Float32 v; memcpy( &v, raw, 4 ); StoreLEFloat32( place, v ); }
                        else { Float64 v; memcpy( &v, raw, 8 ); StoreLEFloat64( place, v ); }
                        break;
                    }
                    default: { break; }
                }
            }
        }
        SParquetPageHeader header;
        header.type = PARQUET_PAGE_DICTIONARY_PAGE;
        header.hasDictionaryPageHeader = true;
        header.dictionaryPageHeader.numValues = (Int32) count;
        header.dictionaryPageHeader.encoding = PARQUET_ENCODING_PLAIN;
        m_dictionaryPages = 1;
        return Store( m_dictionaryPage, header, 0, false );
    }

    const CParquetColumnStage& m_stage;
    const CORE::CDataTableWriterOptions& m_options;
    CParquetCompression& m_compression;
    CByteSink m_pages;
    CByteSink m_dictionaryPage;
    CByteSink m_payload;
    CDictionaryBuilder m_dictionary;
    GUCEF::vector< UInt32 > m_indices;
    GUCEF::vector< UInt8 > m_shifted;
    CORE::CDynamicBuffer m_levels;
    CORE::CDynamicBuffer m_header;
    CORE::CDynamicBuffer m_compressed;
    SStatsAccumulator m_chunkStats;
    UInt64 m_uncompressedBytes;
    UInt64 m_compressedBytes;
    UInt32 m_firstValuesOffset;
    bool m_hasFirstValues;
    UInt32 m_dictionaryPages;
    UInt32 m_plainPages;
    UInt32 m_dictionaryDataPages;
    bool m_alignedPages;
    CORE::CString m_error;
};

/** Combines the statistics of a chunk into the statistics of the file */
void
MergeColumnStats( CORE::TDataTableLogicalType type, const CORE::CDataTableColumnStats& chunk, CORE::CDataTableColumnStats& file, bool first )
{
    if ( first )
    {
        file = chunk;
        return;
    }
    file.valueCount += chunk.valueCount;
    file.nullCount += chunk.nullCount;
    file.hasNanCount = file.hasNanCount && chunk.hasNanCount;
    file.nanCount += chunk.nanCount;
    if ( !chunk.hasMinMax )
        return;
    if ( !file.hasMinMax )
    {
        file.hasMinMax = true;
        file.minValue = chunk.minValue;
        file.maxValue = chunk.maxValue;
        file.minExact = chunk.minExact;
        file.maxExact = chunk.maxExact;
        return;
    }
    if ( CORE::CDataTablePruner::CompareNative( type, chunk.minValue, file.minValue ) < 0 )
        file.minValue = chunk.minValue;
    if ( CORE::CDataTablePruner::CompareNative( type, chunk.maxValue, file.maxValue ) > 0 )
        file.maxValue = chunk.maxValue;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParquetTableWriter::CParquetTableWriter( const CORE::IOAccessPtr& destination            ,
                                          const CORE::CDataTableSchemaPtr& schema         ,
                                          const CORE::CDataTableWriterOptions& options    )
    : CORE::CITableWriter()
    , m_destination( destination )
    , m_schema( schema )
    , m_options( options )
    , m_compression()
    , m_stages()
    , m_rowGroupBytes()
    , m_rowGroupCount( 0 )
    , m_rowCount( 0 )
    , m_position( 0 )
    , m_started( false )
    , m_finished( false )
    , m_aborted( false )
    , m_failed( false )
    , m_failedStatus( CORE::DATATABLE_STATUS_OK )
    , m_lastError()
{GUCEF_TRACE;

    m_rowGroupBytes.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

CParquetTableWriter::~CParquetTableWriter()
{GUCEF_TRACE;

    ReleaseStages();
}

/*-------------------------------------------------------------------------*/

void
CParquetTableWriter::ReleaseStages( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_stages.size(); ++i )
        GUCEF_DELETE m_stages[ i ];
    m_stages.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetTableWriter::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_failed = true;
    m_failedStatus = CORE::DataTableStatusIsError( status ) ? status : CORE::DATATABLE_STATUS_FAILED;
    m_lastError = message;
    return m_failedStatus;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::Init( CORE::CString& error )
{GUCEF_TRACE;

    if ( m_destination.IsNULL() || m_schema.IsNULL() )
    {
        error = "A destination and a schema are needed";
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    }
    if ( !m_options.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    Int32 codec = 0;
    if ( !CParquetCompression::ParquetCodecFromName( m_options.compression, codec ) )
    {
        error = "Compression \"" + m_options.compression + "\" has no parquet codec";
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, error );
    }
    CORE::TDataTableStatus status = m_compression.Init( codec, m_options.compressionLevel, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    CORE::CDataTableBufferPool& pool = CORE::CCoreGlobal::Instance()->GetDataTableBufferPool();
    ReleaseStages();
    for ( UInt32 c=0; c<m_schema->GetColumnCount(); ++c )
    {
        const CORE::CDataTableColumnDef& def = m_schema->GetColumn( c );
        SParquetSchemaElement element;
        if ( !CParquetSchemaMapping::MapColumnForWrite( def, "", 0, element, error ) )
            return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, error );

        CParquetColumnStage* stage = GUCEF_NEW CParquetColumnStage( pool );
        m_stages.push_back( stage );
        stage->def = def;
        stage->name = CORE::ToUtf8String( def.name );
        stage->physical = element.type;
        stage->width = CORE::DataTableLogicalTypeByteWidth( def.type );
        switch ( element.type )
        {
            case PARQUET_TYPE_INT32:
            case PARQUET_TYPE_FLOAT: { stage->physicalWidth = 4; break; }
            case PARQUET_TYPE_INT64:
            case PARQUET_TYPE_DOUBLE: { stage->physicalWidth = 8; break; }
            default: { stage->physicalWidth = 0; break; }
        }
        if ( CORE::DATATABLE_TYPE_BOOL == def.type ) stage->family = FAMILY_BOOL;
        else if ( CORE::DATATABLE_TYPE_UTF8 == def.type || CORE::DATATABLE_TYPE_BINARY == def.type ) stage->family = FAMILY_BYTES;
        else if ( CORE::DATATABLE_TYPE_FLOAT32 == def.type || CORE::DATATABLE_TYPE_FLOAT64 == def.type ) stage->family = FAMILY_FLOAT;
        else if ( CORE::DataTableLogicalTypeIsUnsigned( def.type ) ) stage->family = FAMILY_UNSIGNED;
        else stage->family = FAMILY_SIGNED;

        // the dictionary mode of the options decides, the encoding hint of the column decides when the mode is auto
        switch ( m_options.dictionary )
        {
            case CORE::DATATABLE_DICTIONARY_NEVER: { stage->useDictionary = false; break; }
            case CORE::DATATABLE_DICTIONARY_ALWAYS: { stage->useDictionary = FAMILY_BOOL != stage->family; break; }
            default:
            {
                if ( CORE::DATATABLE_ENCODING_DICTIONARY == def.encodingHint )
                    stage->useDictionary = FAMILY_BOOL != stage->family;
                else
                if ( CORE::DATATABLE_ENCODING_PLAIN == def.encodingHint )
                    stage->useDictionary = false;
                else
                    stage->useDictionary = FAMILY_BYTES == stage->family;
                break;
            }
        }
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::Output( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return CORE::DATATABLE_STATUS_OK;
    if ( size != m_destination->Write( data, 1, size ) )
        return Fail( CORE::DATATABLE_STATUS_IO_ERROR, "The output did not take " + CORE::ToString( size ) + " bytes at byte " + CORE::ToString( m_position ) );
    m_position += size;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::StartFile( void )
{GUCEF_TRACE;

    if ( m_started )
        return CORE::DATATABLE_STATUS_OK;
    m_started = true;
    return Output( "PAR1", 4 );
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetTableWriter::GetStagedBytes( void ) const
{
    UInt64 bytes = 0;
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        const CParquetColumnStage& stage = *m_stages[ c ];
        if ( FAMILY_BYTES == stage.family )
            bytes += stage.byteCount + 4ULL * stage.rows;
        else
        if ( FAMILY_BOOL == stage.family )
            bytes += ( stage.rows + 7 ) / 8;
        else
            bytes += (UInt64) stage.rows * stage.physicalWidth;
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::AppendRows( const CORE::CDataTableBatch& batch, const UInt32* selection, UInt32 first, UInt32 count )
{GUCEF_TRACE;

    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        CParquetColumnStage& stage = *m_stages[ c ];
        const CORE::CDataTableColumnBatch& column = batch.GetColumn( (UInt32) c );
        if ( column.HasNulls() && !stage.def.nullable )
        {
            // only a null among the rows written is refused
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( !column.IsValid( GUCEF_NULL != selection ? selection[ i ] : first + i ) )
                    return Fail( CORE::DATATABLE_STATUS_TYPE_MISMATCH, "Column \"" + stage.def.name + "\" does not take null" );
            }
        }

        UInt64 extraBytes = 0;
        if ( FAMILY_BYTES == stage.family )
        {
            for ( UInt32 i=0; i<count; ++i )
            {
                const UInt32 row = GUCEF_NULL != selection ? selection[ i ] : first + i;
                UInt32 length = 0;
                if ( column.IsValid( row ) )
                    column.GetVariableValue( row, length );
                extraBytes += length;
            }
        }
        CORE::TDataTableStatus status = stage.Reserve( count, extraBytes );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, "No room to stage the rows of column \"" + stage.def.name + "\"" );

        UInt8* validity = stage.validity.IsNULL() ? GUCEF_NULL : static_cast< UInt8* >( stage.validity->GetData() );
        for ( UInt32 i=0; i<count; ++i )
        {
            const UInt32 row = GUCEF_NULL != selection ? selection[ i ] : first + i;
            const UInt32 target = stage.rows + i;
            const bool valid = column.IsValid( row );
            if ( GUCEF_NULL != validity )
            {
                if ( 0 == ( target & 7 ) )
                    validity[ target >> 3 ] = 0;
                if ( valid )
                    validity[ target >> 3 ] |= (UInt8) ( 1u << ( target & 7 ) );
            }
            if ( !valid )
                ++stage.nullCount;
        }

        switch ( stage.family )
        {
            case FAMILY_BOOL:
            {
                const UInt8* source = column.GetBoolBits();
                UInt8* bits = static_cast< UInt8* >( stage.values->GetData() );
                for ( UInt32 i=0; i<count; ++i )
                {
                    const UInt32 row = GUCEF_NULL != selection ? selection[ i ] : first + i;
                    const UInt32 target = stage.rows + i;
                    if ( 0 == ( target & 7 ) )
                        bits[ target >> 3 ] = 0;
                    if ( column.IsValid( row ) && 0 != ( source[ row >> 3 ] & ( 1u << ( row & 7 ) ) ) )
                        bits[ target >> 3 ] |= (UInt8) ( 1u << ( target & 7 ) );
                }
                break;
            }
            case FAMILY_BYTES:
            {
                UInt32* offsets = static_cast< UInt32* >( stage.values->GetData() );
                UInt8* bytes = static_cast< UInt8* >( stage.bytes->GetData() );
                for ( UInt32 i=0; i<count; ++i )
                {
                    const UInt32 row = GUCEF_NULL != selection ? selection[ i ] : first + i;
                    UInt32 length = 0;
                    const char* value = column.IsValid( row ) ? column.GetVariableValue( row, length ) : GUCEF_NULL;
                    if ( GUCEF_NULL != value && length > 0 )
                        memcpy( bytes + stage.byteCount, value, length );
                    stage.byteCount += GUCEF_NULL != value ? length : 0;
                    offsets[ stage.rows + i + 1 ] = stage.byteCount;
                }
                break;
            }
            default:
            {
                const UInt8* source = static_cast< const UInt8* >( column.GetFixedData() );
                UInt8* target = static_cast< UInt8* >( stage.values->GetData() ) + (size_t) stage.rows * stage.width;
                if ( GUCEF_NULL == selection )
                {
                    memcpy( target, source + (size_t) first * stage.width, (size_t) count * stage.width );
                }
                else
                {
                    for ( UInt32 i=0; i<count; ++i )
                        memcpy( target + (size_t) i * stage.width, source + (size_t) selection[ i ] * stage.width, stage.width );
                }
                break;
            }
        }
        stage.rows += count;
    }
    m_rowCount += count;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::WriteBatch( const CORE::CDataTableBatch& batch )
{GUCEF_TRACE;

    if ( m_failed )
        return m_failedStatus;
    if ( m_finished || m_aborted )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The writer has finished" );
    if ( m_stages.empty() && 0 != m_schema->GetColumnCount() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The writer was not initialized" );
    if ( batch.GetColumnCount() != m_stages.size() )
        return Fail( CORE::DATATABLE_STATUS_SCHEMA_MISMATCH, "A batch of " + CORE::ToString( batch.GetColumnCount() ) + " columns for a schema of " +
                     CORE::ToString( (UInt32) m_stages.size() ) + " columns" );
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        const CORE::CDataTableColumnBatch& column = batch.GetColumn( (UInt32) c );
        if ( !IsSameColumnType( column, m_stages[ c ]->def ) || column.GetRowCount() != batch.GetRowCount() ||
             ( CORE::DATATABLE_STORAGE_NONE == column.GetStorage() && batch.GetRowCount() > 0 ) )
            return Fail( CORE::DATATABLE_STATUS_SCHEMA_MISMATCH, "Column " + CORE::ToString( (UInt32) c ) + " of the batch does not have the type of column \"" +
                         m_stages[ c ]->def.name + "\" (" + m_stages[ c ]->def.GetTypeText() + ")" );
    }
    CORE::TDataTableStatus status = StartFile();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    const UInt32 selected = batch.GetSelectedRowCount();
    const UInt32* selection = batch.GetSelection();
    UInt32 done = 0;
    while ( done < selected )
    {
        const UInt32 staged = m_stages.empty() ? 0 : m_stages[ 0 ]->rows;
        if ( staged >= m_options.rowGroupRowCount )
        {
            status = FlushRowGroup();
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
            continue;
        }
        const UInt32 room = m_options.rowGroupRowCount - staged;
        UInt32 take = selected - done < room ? selected - done : room;

        // rows up to the byte limit of the row group, estimated from the bytes per row of the batch
        const UInt64 stagedBytes = GetStagedBytes();
        const UInt64 perRow = EstimateRowBytes( batch );
        if ( perRow > 0 && stagedBytes < m_options.rowGroupByteLimit )
        {
            const UInt64 fit = ( m_options.rowGroupByteLimit - stagedBytes + perRow - 1 ) / perRow;
            if ( fit < take )
                take = fit > 0 ? (UInt32) fit : 1;
        }
        status = AppendRows( batch, GUCEF_NULL != selection ? selection + done : GUCEF_NULL, done, take );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        done += take;
        if ( GetStagedBytes() >= m_options.rowGroupByteLimit )
        {
            status = FlushRowGroup();
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetTableWriter::EstimateRowBytes( const CORE::CDataTableBatch& batch ) const
{
    // fixed widths as staged, variable columns by the average of the batch
    const UInt32 rows = batch.GetRowCount();
    if ( 0 == rows )
        return 0;
    UInt64 bytes = 0;
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        const CParquetColumnStage& stage = *m_stages[ c ];
        if ( FAMILY_BYTES == stage.family )
        {
            const CORE::CDataTableColumnBatch& column = batch.GetColumn( (UInt32) c );
            UInt64 total = 0;
            if ( CORE::DATATABLE_STORAGE_VARIABLE == column.GetStorage() )
                total = column.GetOffsets()[ rows ] - column.GetOffsets()[ 0 ];
            else
            if ( CORE::DATATABLE_STORAGE_DICTIONARY == column.GetStorage() && GUCEF_NULL != column.GetDictionary() && column.GetDictionary()->GetRowCount() > 0 )
            {
                const CORE::CDataTableColumnBatch& dictionary = *column.GetDictionary();
                total = (UInt64) ( dictionary.GetOffsets()[ dictionary.GetRowCount() ] - dictionary.GetOffsets()[ 0 ] ) * rows / dictionary.GetRowCount();
            }
            bytes += 4 + total / rows;
        }
        else
        if ( FAMILY_BOOL == stage.family )
            bytes += 1;
        else
            bytes += stage.physicalWidth;
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

bool
CParquetTableWriter::IsStagedSortedOnKey( void ) const
{GUCEF_TRACE;

    const CORE::CDataTableSchema::TColumnIndexVector& key = m_schema->GetSortKey();
    if ( key.empty() || m_stages.empty() )
        return false;
    const UInt32 rows = m_stages[ 0 ]->rows;
    for ( UInt32 r=1; r<rows; ++r )
    {
        for ( size_t k=0; k<key.size(); ++k )
        {
            const Int32 order = CompareStagedRows( *m_stages[ key[ k ] ], r - 1, r );
            if ( order < 0 )
                break;
            if ( order > 0 )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::FlushRowGroup( void )
{GUCEF_TRACE;

    if ( m_failed )
        return m_failedStatus;
    if ( m_finished || m_aborted )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The writer has finished" );
    const UInt32 rows = m_stages.empty() ? 0 : m_stages[ 0 ]->rows;
    if ( 0 == rows )
        return CORE::DATATABLE_STATUS_OK;
    CORE::TDataTableStatus status = StartFile();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CDataTableBufferPool& pool = CORE::CCoreGlobal::Instance()->GetDataTableBufferPool();
    SParquetRowGroup rowGroup;
    rowGroup.numRows = rows;
    rowGroup.columns.resize( m_stages.size() );
    UInt64 totalUncompressed = 0;
    UInt64 totalCompressed = 0;
    UInt64 rowGroupStart = 0;
    GUCEF::vector< UInt8 > statsStore( m_stages.size() * 16, 0 );

    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        const CParquetColumnStage& stage = *m_stages[ c ];
        CChunkEncoder encoder( stage, m_options, m_compression, pool );
        status = encoder.Encode();
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, "Column \"" + stage.def.name + "\" cannot be encoded: " + encoder.GetError() );

        // padding before a PLAIN fixed width chunk puts its page values at multiples of 8 in the file
        UInt32 valuesOffset = 0;
        if ( encoder.GetFirstValuesOffset( valuesOffset ) )
        {
            const UInt64 at = encoder.HasAlignedPages() ? m_position : m_position + encoder.GetDictionaryPage().GetSize() + valuesOffset;
            const UInt32 padding = (UInt32) ( ( 8 - ( at & 7 ) ) & 7 );
            static const UInt8 zeros[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            status = Output( zeros, padding );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return status;
        }

        const UInt64 chunkStart = m_position;
        if ( 0 == c )
            rowGroupStart = chunkStart;
        status = Output( encoder.GetDictionaryPage().GetData(), encoder.GetDictionaryPage().GetSize() );
        if ( CORE::DATATABLE_STATUS_OK == status )
            status = Output( encoder.GetDataPages().GetData(), encoder.GetDataPages().GetSize() );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;

        SParquetColumnChunk& chunk = rowGroup.columns[ c ];
        chunk.fileOffset = (Int64) chunkStart;
        chunk.hasMetaData = true;
        SParquetColumnMetaData& meta = chunk.metaData;
        meta.type = stage.physical;
        if ( encoder.UsesDictionary() )
            meta.encodings.push_back( PARQUET_ENCODING_PLAIN );
        else
        if ( encoder.GetPlainPages() > 0 )
            meta.encodings.push_back( PARQUET_ENCODING_PLAIN );
        if ( stage.def.nullable )
            meta.encodings.push_back( PARQUET_ENCODING_RLE );
        if ( encoder.UsesDictionary() )
            meta.encodings.push_back( PARQUET_ENCODING_RLE_DICTIONARY );
        meta.pathInSchema.push_back( SParquetBytes( stage.name.C_String(), stage.name.ByteSize() > 0 ? stage.name.ByteSize() - 1 : 0 ) );
        meta.codec = m_compression.GetParquetCodec();
        meta.numValues = rows;
        meta.totalUncompressedSize = (Int64) encoder.GetUncompressedBytes();
        meta.totalCompressedSize = (Int64) encoder.GetCompressedBytes();
        meta.dataPageOffset = (Int64) ( chunkStart + encoder.GetDictionaryPage().GetSize() );
        if ( encoder.UsesDictionary() )
        {
            meta.hasDictionaryPageOffset = true;
            meta.dictionaryPageOffset = (Int64) chunkStart;
        }
        if ( m_options.writeStatistics )
        {
            encoder.GetChunkStats().Fill( stage, m_options.statisticsMaxValueBytes, &statsStore[ c * 16 ], &statsStore[ c * 16 + 8 ], meta.statistics );
            meta.hasStatistics = true;
        }
        meta.hasEncodingStats = true;
        if ( encoder.UsesDictionary() )
        {
            SParquetPageEncodingStats stats;
            stats.pageType = PARQUET_PAGE_DICTIONARY_PAGE;
            stats.encoding = PARQUET_ENCODING_PLAIN;
            stats.count = 1;
            meta.encodingStats.push_back( stats );
            stats.pageType = PARQUET_PAGE_DATA_PAGE;
            stats.encoding = PARQUET_ENCODING_RLE_DICTIONARY;
            stats.count = (Int32) encoder.GetDictionaryDataPages();
            meta.encodingStats.push_back( stats );
        }
        if ( encoder.GetPlainPages() > 0 )
        {
            SParquetPageEncodingStats stats;
            stats.pageType = PARQUET_PAGE_DATA_PAGE;
            stats.encoding = PARQUET_ENCODING_PLAIN;
            stats.count = (Int32) encoder.GetPlainPages();
            meta.encodingStats.push_back( stats );
        }
        totalUncompressed += encoder.GetUncompressedBytes();
        totalCompressed += encoder.GetCompressedBytes();

        // the statistics of the file, in the native form
        CORE::CDataTableColumnStats chunkStats;
        encoder.GetChunkStats().ToColumnStats( stage, rows, chunkStats );
        CParquetColumnStage& writable = *m_stages[ c ];
        MergeColumnStats( stage.def.type, chunkStats, writable.fileStats, !writable.hasFileStats );
        writable.hasFileStats = true;
    }

    rowGroup.totalByteSize = (Int64) totalUncompressed;
    rowGroup.hasFileOffset = true;
    rowGroup.fileOffset = (Int64) rowGroupStart;
    rowGroup.hasTotalCompressedSize = true;
    rowGroup.totalCompressedSize = (Int64) totalCompressed;
    rowGroup.hasOrdinal = m_rowGroupCount <= 32767;
    rowGroup.ordinal = (Int16) m_rowGroupCount;
    if ( IsStagedSortedOnKey() )
    {
        const CORE::CDataTableSchema::TColumnIndexVector& key = m_schema->GetSortKey();
        rowGroup.hasSortingColumns = true;
        for ( size_t k=0; k<key.size(); ++k )
        {
            SParquetSortingColumn sorting;
            sorting.columnIdx = (Int32) key[ k ];
            sorting.descending = false;
            sorting.nullsFirst = false;
            rowGroup.sortingColumns.push_back( sorting );
        }
    }

    // encoded now, while the statistics bytes it points at are alive
    CThriftCompactWriter thrift( m_rowGroupBytes );
    CParquetMetaDataSerializer::WriteRowGroup( thrift, rowGroup );
    ++m_rowGroupCount;
    for ( size_t c=0; c<m_stages.size(); ++c )
        m_stages[ c ]->Reset();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableWriter::Finish( CORE::CDataTableFileSummary& summary )
{GUCEF_TRACE;

    summary.Clear();
    if ( m_failed )
        return m_failedStatus;
    if ( m_finished || m_aborted )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The writer has finished" );
    CORE::TDataTableStatus status = FlushRowGroup();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = StartFile();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // the schema: the root group and one leaf per column
    GUCEF::vector< SParquetSchemaElement > elements( m_stages.size() + 1 );
    static const char rootName[] = "schema";
    elements[ 0 ].name = SParquetBytes( rootName, 6 );
    elements[ 0 ].hasNumChildren = true;
    elements[ 0 ].numChildren = (Int32) m_stages.size();
    CORE::CString error;
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        const CParquetColumnStage& stage = *m_stages[ c ];
        if ( !CParquetSchemaMapping::MapColumnForWrite( stage.def, stage.name.C_String(), stage.name.ByteSize() > 0 ? stage.name.ByteSize() - 1 : 0, elements[ c + 1 ], error ) )
            return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, error );
    }

    // key values: those of the options, and the table schema as JSON when a JSON codec is there
    GUCEF::vector< CORE::CUtf8String > texts;
    GUCEF::vector< SParquetKeyValue > keyValues;
    bool hasSchemaKey = false;
    for ( UInt32 i=0; i<m_options.keyValueMetaData.GetKeyCount(); ++i )
    {
        texts.push_back( CORE::ToUtf8String( m_options.keyValueMetaData.GetKey( i ).AsString() ) );
        texts.push_back( CORE::ToUtf8String( m_options.keyValueMetaData.GetValue( i ).AsString() ) );
        hasSchemaKey = hasSchemaKey || texts[ texts.size() - 2 ] == CParquetFileMetaData::TableSchemaKey;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    CORE::CDynamicBuffer schemaJson;
    if ( !hasSchemaKey && CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) && !json.IsNULL() )
    {
        CORE::CDataNode root;
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDynamicBufferAccess access( schemaJson );
        if ( m_schema->Serialize( root, settings ) && json->StoreDataTree( &root, &access ) && schemaJson.GetDataSize() > 0 )
        {
            texts.push_back( CORE::CUtf8String( CParquetFileMetaData::TableSchemaKey ) );
            texts.push_back( CORE::CUtf8String( static_cast< const char* >( schemaJson.GetConstBufferPtr() ), schemaJson.GetDataSize() ) );
        }
    }
    for ( size_t i=0; i+1<texts.size(); i+=2 )
    {
        SParquetKeyValue kv;
        kv.key = SParquetBytes( texts[ i ].C_String(), texts[ i ].ByteSize() > 0 ? texts[ i ].ByteSize() - 1 : 0 );
        kv.hasValue = true;
        kv.value = SParquetBytes( texts[ i + 1 ].C_String(), texts[ i + 1 ].ByteSize() > 0 ? texts[ i + 1 ].ByteSize() - 1 : 0 );
        keyValues.push_back( kv );
    }

    // FileMetaData, with the row groups encoded at their flush
    CORE::CDynamicBuffer footer;
    CThriftCompactWriter thrift( footer );
    thrift.BeginStruct();
    thrift.WriteI32Field( 1, 2 );
    thrift.BeginListField( 2, THRIFT_COMPACT_STRUCT, (UInt32) elements.size() );
    for ( size_t i=0; i<elements.size(); ++i )
        CParquetMetaDataSerializer::WriteSchemaElement( thrift, elements[ i ] );
    thrift.WriteI64Field( 3, (Int64) m_rowCount );
    thrift.BeginListField( 4, THRIFT_COMPACT_STRUCT, m_rowGroupCount );
    thrift.WriteRaw( m_rowGroupBytes.GetConstBufferPtr(), m_rowGroupBytes.GetDataSize() );
    if ( !keyValues.empty() )
    {
        thrift.BeginListField( 5, THRIFT_COMPACT_STRUCT, (UInt32) keyValues.size() );
        for ( size_t i=0; i<keyValues.size(); ++i )
            CParquetMetaDataSerializer::WriteKeyValue( thrift, keyValues[ i ] );
    }
    thrift.WriteBinaryField( 6, CreatedBy, (UInt32) strlen( CreatedBy ) );
    thrift.BeginListField( 7, THRIFT_COMPACT_STRUCT, (UInt32) m_stages.size() );
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        thrift.BeginStruct();
        thrift.BeginStructField( PARQUET_COLUMNORDER_TYPE_DEFINED );
        thrift.EndStruct();
        thrift.EndStruct();
    }
    thrift.EndStruct();
    if ( !thrift.IsValid() )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The footer could not be encoded" );

    UInt8 tail[ 8 ];
    StoreLE32( tail, footer.GetDataSize() );
    memcpy( tail + 4, "PAR1", 4 );
    status = Output( footer.GetConstBufferPtr(), footer.GetDataSize() );
    if ( CORE::DATATABLE_STATUS_OK == status )
        status = Output( tail, 8 );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    m_destination->Flush();
    m_finished = true;

    summary.rowCount = m_rowCount;
    summary.rowGroupCount = m_rowGroupCount;
    summary.byteCount = m_position;
    summary.columnStats.resize( m_stages.size() );
    for ( size_t c=0; c<m_stages.size(); ++c )
    {
        if ( m_stages[ c ]->hasFileStats )
        {
            summary.columnStats[ c ] = m_stages[ c ]->fileStats;
        }
        else
        {
            summary.columnStats[ c ].Clear();
            summary.columnStats[ c ].hasNullCount = true;
            summary.columnStats[ c ].nullCount = 0;
            summary.columnStats[ c ].valueCount = 0;
        }
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CParquetTableWriter::Abort( void )
{GUCEF_TRACE;

    m_aborted = true;
    for ( size_t c=0; c<m_stages.size(); ++c )
        m_stages[ c ]->Reset();
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
