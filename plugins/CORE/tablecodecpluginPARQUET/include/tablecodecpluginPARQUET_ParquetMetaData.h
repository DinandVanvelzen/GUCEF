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

#ifndef TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#define TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_STL_H
#include "gucef_stl.h"
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H
#include "tablecodecpluginPARQUET_CThriftCompactReader.h"
#define TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H
#endif /* TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H
#include "tablecodecpluginPARQUET_CThriftCompactWriter.h"
#define TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H
#endif /* TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H ? */

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

/*
 *  The enumerations and Thrift field ids of the Apache Parquet format (parquet.thrift of parquet-format).
 *  Values are file format values: never renumbered.
 */

enum EParquetType
{
    PARQUET_TYPE_BOOLEAN                = 0 ,
    PARQUET_TYPE_INT32                  = 1 ,
    PARQUET_TYPE_INT64                  = 2 ,
    PARQUET_TYPE_INT96                  = 3 ,
    PARQUET_TYPE_FLOAT                  = 4 ,
    PARQUET_TYPE_DOUBLE                 = 5 ,
    PARQUET_TYPE_BYTE_ARRAY             = 6 ,
    PARQUET_TYPE_FIXED_LEN_BYTE_ARRAY   = 7
};

enum EParquetConvertedType
{
    PARQUET_CONVERTED_UTF8              = 0 ,
    PARQUET_CONVERTED_MAP               = 1 ,
    PARQUET_CONVERTED_MAP_KEY_VALUE     = 2 ,
    PARQUET_CONVERTED_LIST              = 3 ,
    PARQUET_CONVERTED_ENUM              = 4 ,
    PARQUET_CONVERTED_DECIMAL           = 5 ,
    PARQUET_CONVERTED_DATE              = 6 ,
    PARQUET_CONVERTED_TIME_MILLIS       = 7 ,
    PARQUET_CONVERTED_TIME_MICROS       = 8 ,
    PARQUET_CONVERTED_TIMESTAMP_MILLIS  = 9 ,
    PARQUET_CONVERTED_TIMESTAMP_MICROS  = 10 ,
    PARQUET_CONVERTED_UINT_8            = 11 ,
    PARQUET_CONVERTED_UINT_16           = 12 ,
    PARQUET_CONVERTED_UINT_32           = 13 ,
    PARQUET_CONVERTED_UINT_64           = 14 ,
    PARQUET_CONVERTED_INT_8             = 15 ,
    PARQUET_CONVERTED_INT_16            = 16 ,
    PARQUET_CONVERTED_INT_32            = 17 ,
    PARQUET_CONVERTED_INT_64            = 18 ,
    PARQUET_CONVERTED_JSON              = 19 ,
    PARQUET_CONVERTED_BSON              = 20 ,
    PARQUET_CONVERTED_INTERVAL          = 21
};

enum EParquetRepetition
{
    PARQUET_REPETITION_REQUIRED = 0 ,
    PARQUET_REPETITION_OPTIONAL = 1 ,
    PARQUET_REPETITION_REPEATED = 2
};

enum EParquetEncoding
{
    PARQUET_ENCODING_PLAIN                      = 0 ,
    PARQUET_ENCODING_PLAIN_DICTIONARY           = 2 ,
    PARQUET_ENCODING_RLE                        = 3 ,
    PARQUET_ENCODING_BIT_PACKED                 = 4 ,
    PARQUET_ENCODING_DELTA_BINARY_PACKED        = 5 ,
    PARQUET_ENCODING_DELTA_LENGTH_BYTE_ARRAY    = 6 ,
    PARQUET_ENCODING_DELTA_BYTE_ARRAY           = 7 ,
    PARQUET_ENCODING_RLE_DICTIONARY             = 8 ,
    PARQUET_ENCODING_BYTE_STREAM_SPLIT          = 9 ,
    PARQUET_ENCODING_ALP                        = 10
};

enum EParquetCompression
{
    PARQUET_COMPRESSION_UNCOMPRESSED    = 0 ,
    PARQUET_COMPRESSION_SNAPPY          = 1 ,
    PARQUET_COMPRESSION_GZIP            = 2 ,
    PARQUET_COMPRESSION_LZO             = 3 ,
    PARQUET_COMPRESSION_BROTLI          = 4 ,
    PARQUET_COMPRESSION_LZ4             = 5 ,
    PARQUET_COMPRESSION_ZSTD            = 6 ,
    PARQUET_COMPRESSION_LZ4_RAW         = 7
};

enum EParquetPageType
{
    PARQUET_PAGE_DATA_PAGE          = 0 ,
    PARQUET_PAGE_INDEX_PAGE         = 1 ,
    PARQUET_PAGE_DICTIONARY_PAGE    = 2 ,
    PARQUET_PAGE_DATA_PAGE_V2       = 3
};

/** The members of the LogicalType union: the field id of the member that is set */
enum EParquetLogicalTypeKind
{
    PARQUET_LOGICAL_NONE        = 0 ,
    PARQUET_LOGICAL_STRING      = 1 ,
    PARQUET_LOGICAL_MAP         = 2 ,
    PARQUET_LOGICAL_LIST        = 3 ,
    PARQUET_LOGICAL_ENUM        = 4 ,
    PARQUET_LOGICAL_DECIMAL     = 5 ,
    PARQUET_LOGICAL_DATE        = 6 ,
    PARQUET_LOGICAL_TIME        = 7 ,
    PARQUET_LOGICAL_TIMESTAMP   = 8 ,
    PARQUET_LOGICAL_INTEGER     = 10 ,
    PARQUET_LOGICAL_UNKNOWN     = 11 ,     /**< NullType: only nulls */
    PARQUET_LOGICAL_JSON        = 12 ,
    PARQUET_LOGICAL_BSON        = 13 ,
    PARQUET_LOGICAL_UUID        = 14 ,
    PARQUET_LOGICAL_FLOAT16     = 15
};

/** The members of the TimeUnit union */
enum EParquetTimeUnit
{
    PARQUET_TIMEUNIT_NONE   = 0 ,
    PARQUET_TIMEUNIT_MILLIS = 1 ,
    PARQUET_TIMEUNIT_MICROS = 2 ,
    PARQUET_TIMEUNIT_NANOS  = 3
};

/** The members of the ColumnOrder union */
enum EParquetColumnOrder
{
    PARQUET_COLUMNORDER_NONE                = 0 ,
    PARQUET_COLUMNORDER_TYPE_DEFINED        = 1 ,
    PARQUET_COLUMNORDER_IEEE754_TOTAL       = 2 ,
    PARQUET_COLUMNORDER_INT96_TIMESTAMP     = 3
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Bytes of a binary or string field: a pointer into the parsed data (reading) or into memory the writer keeps (writing) */
struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetBytes
{
    const UInt8* data;
    UInt32 size;

    SParquetBytes( void );
    SParquetBytes( const void* bytes, UInt32 length );

    /** Whether the bytes equal the zero terminated text */
    bool Equals( const char* text ) const;
    CORE::CString ToString( void ) const;
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetStatistics
{
    bool hasMax;
    bool hasMin;
    bool hasNullCount;
    bool hasDistinctCount;
    bool hasMaxValue;
    bool hasMinValue;
    bool hasIsMaxValueExact;
    bool hasIsMinValueExact;
    bool hasNanCount;
    SParquetBytes max;                  /**< 1, deprecated: signed order */
    SParquetBytes min;                  /**< 2, deprecated: signed order */
    Int64 nullCount;                    /**< 3 */
    Int64 distinctCount;                /**< 4 */
    SParquetBytes maxValue;             /**< 5, in the column order */
    SParquetBytes minValue;             /**< 6, in the column order */
    bool isMaxValueExact;               /**< 7 */
    bool isMinValueExact;               /**< 8 */
    Int64 nanCount;                     /**< 9 */

    SParquetStatistics( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetLogicalType
{
    Int16 kind;                         /**< EParquetLogicalTypeKind, or the field id of a member this reader does not know */
    Int32 decimalScale;                 /**< DECIMAL 1 */
    Int32 decimalPrecision;             /**< DECIMAL 2 */
    bool isAdjustedToUtc;               /**< TIME and TIMESTAMP 1 */
    Int16 timeUnit;                     /**< TIME and TIMESTAMP 2: EParquetTimeUnit */
    Int8 bitWidth;                      /**< INTEGER 1 */
    bool isSigned;                      /**< INTEGER 2 */

    SParquetLogicalType( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetSchemaElement
{
    bool hasType;
    bool hasTypeLength;
    bool hasRepetitionType;
    bool hasNumChildren;
    bool hasConvertedType;
    bool hasScale;
    bool hasPrecision;
    bool hasFieldId;
    bool hasLogicalType;
    Int32 type;                         /**< 1: EParquetType */
    Int32 typeLength;                   /**< 2 */
    Int32 repetitionType;               /**< 3: EParquetRepetition */
    SParquetBytes name;                 /**< 4 */
    Int32 numChildren;                  /**< 5 */
    Int32 convertedType;                /**< 6: EParquetConvertedType */
    Int32 scale;                        /**< 7 */
    Int32 precision;                    /**< 8 */
    Int32 fieldId;                      /**< 9 */
    SParquetLogicalType logicalType;    /**< 10 */

    SParquetSchemaElement( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetKeyValue
{
    SParquetBytes key;                  /**< 1 */
    bool hasValue;
    SParquetBytes value;                /**< 2 */

    SParquetKeyValue( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetSortingColumn
{
    Int32 columnIdx;                    /**< 1 */
    bool descending;                    /**< 2 */
    bool nullsFirst;                    /**< 3 */

    SParquetSortingColumn( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetPageEncodingStats
{
    Int32 pageType;                     /**< 1: EParquetPageType */
    Int32 encoding;                     /**< 2: EParquetEncoding */
    Int32 count;                        /**< 3 */

    SParquetPageEncodingStats( void );
};

typedef GUCEF::vector< Int32 > TInt32Vector;
typedef GUCEF::vector< SParquetBytes > TParquetBytesVector;
typedef GUCEF::vector< SParquetKeyValue > TParquetKeyValueVector;
typedef GUCEF::vector< SParquetSortingColumn > TParquetSortingColumnVector;
typedef GUCEF::vector< SParquetPageEncodingStats > TParquetPageEncodingStatsVector;

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetColumnMetaData
{
    Int32 type;                                     /**< 1 */
    TInt32Vector encodings;                         /**< 2 */
    TParquetBytesVector pathInSchema;               /**< 3 */
    Int32 codec;                                    /**< 4: EParquetCompression */
    Int64 numValues;                                /**< 5 */
    Int64 totalUncompressedSize;                    /**< 6 */
    Int64 totalCompressedSize;                      /**< 7 */
    TParquetKeyValueVector keyValueMetaData;        /**< 8 */
    Int64 dataPageOffset;                           /**< 9 */
    bool hasIndexPageOffset;
    Int64 indexPageOffset;                          /**< 10 */
    bool hasDictionaryPageOffset;
    Int64 dictionaryPageOffset;                     /**< 11 */
    bool hasStatistics;
    SParquetStatistics statistics;                  /**< 12 */
    bool hasEncodingStats;
    TParquetPageEncodingStatsVector encodingStats;  /**< 13 */

    SParquetColumnMetaData( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetColumnChunk
{
    bool hasFilePath;
    SParquetBytes filePath;             /**< 1: the chunk is in another file */
    Int64 fileOffset;                   /**< 2 */
    bool hasMetaData;
    SParquetColumnMetaData metaData;    /**< 3 */
    bool isEncrypted;                   /**< 8 crypto_metadata or 9 encrypted_column_metadata present */

    SParquetColumnChunk( void );
};

typedef GUCEF::vector< SParquetColumnChunk > TParquetColumnChunkVector;

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetRowGroup
{
    TParquetColumnChunkVector columns;              /**< 1 */
    Int64 totalByteSize;                            /**< 2 */
    Int64 numRows;                                  /**< 3 */
    bool hasSortingColumns;
    TParquetSortingColumnVector sortingColumns;     /**< 4 */
    bool hasFileOffset;
    Int64 fileOffset;                               /**< 5 */
    bool hasTotalCompressedSize;
    Int64 totalCompressedSize;                      /**< 6 */
    bool hasOrdinal;
    Int16 ordinal;                                  /**< 7 */

    SParquetRowGroup( void );
};

typedef GUCEF::vector< SParquetSchemaElement > TParquetSchemaElementVector;
typedef GUCEF::vector< SParquetRowGroup > TParquetRowGroupVector;
typedef GUCEF::vector< Int16 > TParquetColumnOrderVector;      /**< EParquetColumnOrder, or the field id of an unknown member */

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetFileMetaData
{
    Int32 version;                                  /**< 1 */
    TParquetSchemaElementVector schema;             /**< 2 */
    Int64 numRows;                                  /**< 3 */
    TParquetRowGroupVector rowGroups;               /**< 4 */
    TParquetKeyValueVector keyValueMetaData;        /**< 5 */
    bool hasCreatedBy;
    SParquetBytes createdBy;                        /**< 6 */
    bool hasColumnOrders;
    TParquetColumnOrderVector columnOrders;         /**< 7 */
    bool hasEncryptionAlgorithm;                    /**< 8 */

    SParquetFileMetaData( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetDataPageHeader
{
    Int32 numValues;                    /**< 1 */
    Int32 encoding;                     /**< 2 */
    Int32 definitionLevelEncoding;      /**< 3 */
    Int32 repetitionLevelEncoding;      /**< 4 */
    bool hasStatistics;
    SParquetStatistics statistics;      /**< 5 */

    SParquetDataPageHeader( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetDictionaryPageHeader
{
    Int32 numValues;                    /**< 1 */
    Int32 encoding;                     /**< 2 */
    bool hasIsSorted;
    bool isSorted;                      /**< 3 */

    SParquetDictionaryPageHeader( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetDataPageHeaderV2
{
    Int32 numValues;                    /**< 1 */
    Int32 numNulls;                     /**< 2 */
    Int32 numRows;                      /**< 3 */
    Int32 encoding;                     /**< 4 */
    Int32 definitionLevelsByteLength;   /**< 5 */
    Int32 repetitionLevelsByteLength;   /**< 6 */
    bool isCompressed;                  /**< 7, true when absent */
    bool hasStatistics;
    SParquetStatistics statistics;      /**< 8 */

    SParquetDataPageHeaderV2( void );
};

struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetPageHeader
{
    Int32 type;                                     /**< 1: EParquetPageType */
    Int32 uncompressedPageSize;                     /**< 2 */
    Int32 compressedPageSize;                       /**< 3 */
    bool hasCrc;
    Int32 crc;                                      /**< 4 */
    bool hasDataPageHeader;
    SParquetDataPageHeader dataPageHeader;          /**< 5 */
    bool hasDictionaryPageHeader;
    SParquetDictionaryPageHeader dictionaryPageHeader;  /**< 7 */
    bool hasDataPageHeaderV2;
    SParquetDataPageHeaderV2 dataPageHeaderV2;      /**< 8 */

    SParquetPageHeader( void );
};

/**
 *  Bounds of a metadata parse. Every element of a list in the metadata is charged with the bytes its structure takes in
 *  memory; when the total passes maxObjectBytes the parse fails with LIMIT_EXCEEDED, so a small hostile footer that
 *  declares millions of empty structs cannot make the reader allocate gigabytes.
 */
struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetParseBudget
{
    static const UInt64 DefaultMaxObjectBytes = 256ULL * 1024 * 1024;

    UInt64 maxObjectBytes;
    UInt64 usedObjectBytes;

    SParquetParseBudget( UInt64 maxBytes = DefaultMaxObjectBytes );

    /** False (and the reader failed with LIMIT_EXCEEDED) when the bytes do not fit */
    bool Charge( CThriftCompactReader& reader, UInt64 bytes );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads and writes the metadata structures of the format with the Thrift compact protocol.
 *  Unknown fields are skipped; a missing required field, a field of the wrong type or a negative count makes the reader
 *  fail with CORRUPT_DATA. Read structures point into the bytes the reader reads from.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetMetaDataSerializer
{
    public:

    static bool ReadFileMetaData( CThriftCompactReader& reader, SParquetFileMetaData& metaData, SParquetParseBudget& budget );
    static bool ReadPageHeader( CThriftCompactReader& reader, SParquetPageHeader& header );
    static bool ReadStatistics( CThriftCompactReader& reader, SParquetStatistics& statistics );
    static bool ReadSchemaElement( CThriftCompactReader& reader, SParquetSchemaElement& element );
    static bool ReadLogicalType( CThriftCompactReader& reader, SParquetLogicalType& logicalType );
    static bool ReadRowGroup( CThriftCompactReader& reader, SParquetRowGroup& rowGroup, SParquetParseBudget& budget );
    static bool ReadColumnChunk( CThriftCompactReader& reader, SParquetColumnChunk& columnChunk, SParquetParseBudget& budget );
    static bool ReadColumnMetaData( CThriftCompactReader& reader, SParquetColumnMetaData& columnMetaData, SParquetParseBudget& budget );
    static bool ReadKeyValue( CThriftCompactReader& reader, SParquetKeyValue& keyValue );

    static void WriteFileMetaData( CThriftCompactWriter& writer, const SParquetFileMetaData& metaData );
    static void WritePageHeader( CThriftCompactWriter& writer, const SParquetPageHeader& header );
    static void WriteStatistics( CThriftCompactWriter& writer, const SParquetStatistics& statistics );
    static void WriteSchemaElement( CThriftCompactWriter& writer, const SParquetSchemaElement& element );
    static void WriteLogicalType( CThriftCompactWriter& writer, const SParquetLogicalType& logicalType );
    static void WriteRowGroup( CThriftCompactWriter& writer, const SParquetRowGroup& rowGroup );
    static void WriteColumnChunk( CThriftCompactWriter& writer, const SParquetColumnChunk& columnChunk );
    static void WriteColumnMetaData( CThriftCompactWriter& writer, const SParquetColumnMetaData& columnMetaData );
    static void WriteKeyValue( CThriftCompactWriter& writer, const SParquetKeyValue& keyValue );

    /** "PLAIN", "RLE_DICTIONARY", ... or the number */
    static CORE::CString EncodingToString( Int32 encoding );

    /** "UNCOMPRESSED", "SNAPPY", "GZIP", ... or the number */
    static CORE::CString CompressionToString( Int32 codec );

    private:

    CParquetMetaDataSerializer( void );     /**< not implemented: static members only */
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

#endif /* TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H ? */
