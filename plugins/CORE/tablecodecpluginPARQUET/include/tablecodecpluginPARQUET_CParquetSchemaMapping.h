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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#include "gucefCORE_CDataTableColumnStats.h"
#define GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNSTATS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#include "tablecodecpluginPARQUET_ParquetMetaData.h"
#define TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the values of a physical type become the batch storage of the column's logical type */
enum EParquetValueConversion
{
    PARQUET_CONVERT_NONE = 0            ,   /**< an unsupported column */
    PARQUET_CONVERT_BITS                ,   /**< BOOLEAN to bits */
    PARQUET_CONVERT_FIXED               ,   /**< INT32, INT64, FLOAT, DOUBLE of the batch width: copied as they are, or used in place */
    PARQUET_CONVERT_INT32_TO_INT8       ,
    PARQUET_CONVERT_INT32_TO_INT16      ,
    PARQUET_CONVERT_INT32_TO_UINT8      ,
    PARQUET_CONVERT_INT32_TO_UINT16     ,
    PARQUET_CONVERT_INT32_TO_INT64      ,   /**< INT32 decimals */
    PARQUET_CONVERT_INT96_TO_INT64      ,   /**< legacy timestamps: Julian day and nanoseconds of the day to nanoseconds */
    PARQUET_CONVERT_BYTES_VARIABLE      ,   /**< BYTE_ARRAY to text or binary */
    PARQUET_CONVERT_FIXED_BYTES_VARIABLE,   /**< FIXED_LEN_BYTE_ARRAY to binary */
    PARQUET_CONVERT_BYTES_DECIMAL       ,   /**< BYTE_ARRAY big endian two's complement to Int64 mantissas */
    PARQUET_CONVERT_FIXED_BYTES_DECIMAL     /**< FIXED_LEN_BYTE_ARRAY big endian two's complement to Int64 mantissas */
};

/** What the reader knows of one leaf column of a file */
struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetColumnInfo
{
    CORE::CString path;                 /**< the leaf name, dotted with its groups for nested leaves */
    UInt32 schemaElementIndex;
    Int32 physicalType;                 /**< EParquetType */
    Int32 typeLength;                   /**< FIXED_LEN_BYTE_ARRAY: bytes per value */
    UInt32 maxDefinitionLevel;
    UInt32 maxRepetitionLevel;
    bool supported;
    CORE::CString unsupportedReason;
    EParquetValueConversion conversion;
    UInt32 physicalWidth;               /**< bytes of a PLAIN value of the physical type, 0 for BOOLEAN and BYTE_ARRAY */
    UInt32 storageWidth;                /**< bytes of a value in the batch, 0 for bits and variable storage */
    bool unsignedOrder;                 /**< unsigned integers: statistics compare unsigned */
    Int16 columnOrder;                  /**< EParquetColumnOrder from the file, NONE when it has none */
    CORE::CDataTableColumnDef column;

    SParquetColumnInfo( void );
};

typedef GUCEF::vector< SParquetColumnInfo > TParquetColumnInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The type mapping of section 13.6 of the data tables design, both ways, and the statistics values.
 *
 *  Reading: a flat schema is a root group whose children are primitive leaves, REQUIRED or OPTIONAL. Leaves under nested
 *  groups and REPEATED fields are counted in the leaf order (the column chunks of a row group follow it) and become
 *  UNSUPPORTED columns, as do types outside the subset (decimals above 18 digits, TIME, FLOAT16, INTERVAL, logical types
 *  this reader does not know). Statistics become CDataTableColumnStats in the native form of the column type.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetSchemaMapping
{
    public:

    /** Maps the schema elements of a file to its leaf columns and the file's table schema */
    static CORE::TDataTableStatus MapFileSchema( const SParquetFileMetaData& metaData   ,
                                                 TParquetColumnInfoVector& columns      ,
                                                 CORE::CDataTableSchema& schema         ,
                                                 CORE::CString& error                   );

    /** A statistics value (PLAIN bytes, byte arrays without their length) in the native form of the column; false when it cannot be read */
    static bool DecodeStatisticValue( const SParquetColumnInfo& column, const SParquetBytes& bytes, CORE::CVariant& value );

    /**
     *  The statistics of a column chunk. min_value / max_value are used when the file states the type defined order (or
     *  IEEE 754 total order for floats) for the column; the deprecated min / max only for signed INT32, INT64, FLOAT
     *  and DOUBLE columns; never for INT96. Float bounds that are NaN, bounds of the wrong size, a minimum above the
     *  maximum and a null count outside 0..rows leave the respective statistics out. The NaN count (nan_count, float
     *  columns only, within 0..rows) is taken over when given; a chunk without it may hold NaN, which the pruner
     *  takes into account for NOT_EQUALS.
     */
    static void BuildColumnStats( const SParquetColumnInfo& column            ,
                                  const SParquetColumnChunk& chunk            ,
                                  Int64 rowCount                              ,
                                  CORE::CDataTableColumnStats& stats          );

    /** The leaf schema element that writes the column (name pointing into the column definition's name bytes) */
    static bool MapColumnForWrite( const CORE::CDataTableColumnDef& column, const char* nameBytes, UInt32 nameLength,
                                   SParquetSchemaElement& element, CORE::CString& error );

    /** The physical type the writer uses for the column type */
    static Int32 WritePhysicalType( CORE::TDataTableLogicalType type );

    /** Decodes a big endian two's complement integer of 1 or more bytes into an Int64; false when it does not fit */
    static bool DecodeBigEndianDecimal( const UInt8* bytes, UInt32 length, Int64& value );

    /** Strict UTF-8: no overlong forms, no surrogates, nothing above U+10FFFF, no truncated sequences */
    static bool IsValidUtf8( const UInt8* bytes, UInt32 length );

    /** INT96 (8 bytes nanoseconds of the day, 4 bytes Julian day, little endian) to nanoseconds since the epoch */
    static bool DecodeInt96Timestamp( const UInt8* bytes, Int64& nanoseconds );

    private:

    CParquetSchemaMapping( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H ? */
