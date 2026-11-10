/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_DATATABLETYPES_H
#define GUCEF_CORE_DATATABLETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Capability flags of a table codec, see CITableCodec::GetCapabilities()
 */
#define GUCEF_TABLECODEC_CAPABILITY_READ                0x0001  /**< the codec reads files */
#define GUCEF_TABLECODEC_CAPABILITY_WRITE               0x0002  /**< the codec writes files */
#define GUCEF_TABLECODEC_CAPABILITY_ROW_GROUPS          0x0004  /**< files can hold more than one row group */
#define GUCEF_TABLECODEC_CAPABILITY_STATISTICS          0x0008  /**< files carry row group statistics */
#define GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS       0x0010  /**< the reader needs Setpos() on its input */
#define GUCEF_TABLECODEC_CAPABILITY_EMBEDDED_SCHEMA     0x0020  /**< the file states its column types */
#define GUCEF_TABLECODEC_CAPABILITY_IN_PLACE_PAGES      0x0040  /**< batch spans can point into the pages read from the file */
#define GUCEF_TABLECODEC_CAPABILITY_PAGE_PRUNING        0x0080  /**< pages are skipped using page statistics */
#define GUCEF_TABLECODEC_CAPABILITY_SORTED_SLICES       0x0100  /**< a range on the first sort column is delivered as a contiguous slice */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Readable, zero filled bytes every pooled data table buffer has after its data */
static const UInt32 DataTableBufferPadding = 64;

/** Largest decimal precision of a decimal column: the mantissa is an Int64 */
static const UInt8 DataTableMaxDecimalPrecision = 18;

/** Bounds and default of the rows per batch of a scan */
static const UInt32 DataTableMinBatchRowCount = 1024;
static const UInt32 DataTableMaxBatchRowCount = 1048576;
static const UInt32 DataTableDefaultBatchRowCount = 16384;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The logical type of a column. The values are stored in definitions of other processes only as text,
 *  see DataTableLogicalTypeToString(), so the numbers are not part of any file format.
 */
enum EDataTableLogicalType
{
    DATATABLE_TYPE_UNSUPPORTED  = 0 ,   /**< a column a codec found but cannot decode, never projected */
    DATATABLE_TYPE_BOOL             ,
    DATATABLE_TYPE_INT8             ,
    DATATABLE_TYPE_INT16            ,
    DATATABLE_TYPE_INT32            ,
    DATATABLE_TYPE_INT64            ,
    DATATABLE_TYPE_UINT8            ,
    DATATABLE_TYPE_UINT16           ,
    DATATABLE_TYPE_UINT32           ,
    DATATABLE_TYPE_UINT64           ,
    DATATABLE_TYPE_FLOAT32          ,
    DATATABLE_TYPE_FLOAT64          ,
    DATATABLE_TYPE_DECIMAL          ,   /**< Int64 mantissas at the column scale, precision 1..18 */
    DATATABLE_TYPE_DATE             ,   /**< Int32 days since 1970-01-01, proleptic Gregorian, signed */
    DATATABLE_TYPE_TIMESTAMP        ,   /**< Int64 ticks of the column time unit since the unix epoch, signed */
    DATATABLE_TYPE_UTF8             ,
    DATATABLE_TYPE_BINARY
};
typedef enum EDataTableLogicalType TDataTableLogicalType;

/**
 *  How the values of a column batch are stored, see CDataTableColumnBatch
 */
enum EDataTableStorage
{
    DATATABLE_STORAGE_NONE  = 0 ,   /**< no values: an empty column batch or an unsupported column */
    DATATABLE_STORAGE_FIXED     ,   /**< rowCount x width bytes, row i at values[ i ] */
    DATATABLE_STORAGE_BITS      ,   /**< ceil( rowCount / 8 ) bytes, row i is bit i % 8 (LSB first) of byte i / 8 */
    DATATABLE_STORAGE_VARIABLE  ,   /**< rowCount + 1 UInt32 offsets and the bytes, every value followed by one zero byte */
    DATATABLE_STORAGE_DICTIONARY    /**< rowCount UInt32 indices into a dictionary column batch of DATATABLE_STORAGE_VARIABLE */
};
typedef enum EDataTableStorage TDataTableStorage;

/**
 *  The tick unit of a timestamp column
 */
enum EDataTableTimeUnit
{
    DATATABLE_TIMEUNIT_MILLISECONDS = 0 ,   /**< "ms" */
    DATATABLE_TIMEUNIT_MICROSECONDS     ,   /**< "us" */
    DATATABLE_TIMEUNIT_NANOSECONDS          /**< "ns" */
};
typedef enum EDataTableTimeUnit TDataTableTimeUnit;

/**
 *  Outcome of a data table operation. Values are never renumbered; new ones are only added.
 */
enum EDataTableStatus
{
    DATATABLE_STATUS_OK                 = 0     ,   /**< done */
    DATATABLE_STATUS_END                = 1     ,   /**< no more batches, rows or units */
    DATATABLE_STATUS_PENDING            = 2     ,   /**< async: nothing ready yet */
    DATATABLE_STATUS_PRUNED             = 3     ,   /**< the unit cannot hold a matching row */

    DATATABLE_STATUS_FAILED             = 10    ,   /**< another failure */
    DATATABLE_STATUS_INVALID_ARGUMENT   = 11    ,   /**< a bad query, definition or option */
    DATATABLE_STATUS_NOT_FOUND          = 12    ,   /**< table, column, partition or codec absent */
    DATATABLE_STATUS_ALREADY_EXISTS     = 13    ,   /**< creating a table that exists */
    DATATABLE_STATUS_SCHEMA_MISMATCH    = 14    ,   /**< a file does not fit the table schema */
    DATATABLE_STATUS_TYPE_MISMATCH      = 15    ,   /**< a value or literal not representable in the column */
    DATATABLE_STATUS_CONFLICT           = 16    ,   /**< an existing primary key on an insert */
    DATATABLE_STATUS_CORRUPT_DATA       = 17    ,   /**< a file violates its format */
    DATATABLE_STATUS_UNSUPPORTED        = 18    ,   /**< a valid file feature outside the supported subset */
    DATATABLE_STATUS_IO_ERROR           = 19    ,   /**< the storage refused */
    DATATABLE_STATUS_READ_ONLY          = 20    ,   /**< a change on a read only table or database */
    DATATABLE_STATUS_CANCELLED          = 21    ,   /**< cancelled */
    DATATABLE_STATUS_OUT_OF_MEMORY      = 22    ,   /**< an allocation failed */
    DATATABLE_STATUS_LIMIT_EXCEEDED     = 23        /**< a configured bound: footer size, page size, value length, buffer size */
};
typedef enum EDataTableStatus TDataTableStatus;

/**
 *  The operator of a predicate, see CDataTablePredicate
 */
enum EDataTablePredicateOp
{
    DATATABLE_PREDICATE_EQUALS = 0          ,
    DATATABLE_PREDICATE_NOT_EQUALS          ,
    DATATABLE_PREDICATE_LESS                ,
    DATATABLE_PREDICATE_LESS_OR_EQUAL       ,
    DATATABLE_PREDICATE_GREATER             ,
    DATATABLE_PREDICATE_GREATER_OR_EQUAL    ,
    DATATABLE_PREDICATE_RANGE               ,
    DATATABLE_PREDICATE_IN                  ,
    DATATABLE_PREDICATE_IS_NULL             ,
    DATATABLE_PREDICATE_IS_NOT_NULL
};
typedef enum EDataTablePredicateOp TDataTablePredicateOp;

/**
 *  How far a scan applies the predicates of its query
 */
enum EDataTableFilterMode
{
    DATATABLE_FILTER_EXACT = 0      ,   /**< pruning and exact row filtering: only matching rows are selected */
    DATATABLE_FILTER_PRUNE_ONLY         /**< pruning only, the caller filters the rows of the delivered batches */
};
typedef enum EDataTableFilterMode TDataTableFilterMode;

/**
 *  How a write combines the rows with the rows already in the table
 */
enum EDataTableWriteMode
{
    DATATABLE_WRITE_INSERT = 0          ,   /**< adds rows; an existing primary key fails that partition with CONFLICT */
    DATATABLE_WRITE_UPSERT              ,   /**< adds rows; a row with an existing primary key replaces it */
    DATATABLE_WRITE_REPLACE_PARTITIONS      /**< every partition the rows fall into is replaced by exactly those rows */
};
typedef enum EDataTableWriteMode TDataTableWriteMode;

/**
 *  How a partition field is derived from its source column
 */
enum EDataTablePartitionTransform
{
    DATATABLE_PARTITION_TRANSFORM_IDENTITY = 0  ,   /**< the value itself: text, integer or date (yyyy-mm-dd) */
    DATATABLE_PARTITION_TRANSFORM_YEAR          ,   /**< yyyy of a date or timestamp (UTC) */
    DATATABLE_PARTITION_TRANSFORM_MONTH         ,   /**< yyyy-mm of a date or timestamp (UTC) */
    DATATABLE_PARTITION_TRANSFORM_DAY               /**< yyyy-mm-dd of a date or timestamp (UTC) */
};
typedef enum EDataTablePartitionTransform TDataTablePartitionTransform;

/**
 *  Encoding a column asks the writer for
 */
enum EDataTableEncodingHint
{
    DATATABLE_ENCODING_AUTO = 0     ,   /**< dictionary encoding for text and binary, plain for the rest */
    DATATABLE_ENCODING_PLAIN        ,
    DATATABLE_ENCODING_DICTIONARY
};
typedef enum EDataTableEncodingHint TDataTableEncodingHint;

/**
 *  When a writer uses dictionary encoding, see CDataTableWriterOptions
 */
enum EDataTableDictionaryMode
{
    DATATABLE_DICTIONARY_AUTO = 0   ,   /**< dictionary for UTF8 and BINARY columns */
    DATATABLE_DICTIONARY_ALWAYS     ,
    DATATABLE_DICTIONARY_NEVER
};
typedef enum EDataTableDictionaryMode TDataTableDictionaryMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** True for the statuses that are failures (10 and up) */
inline bool DataTableStatusIsError( TDataTableStatus status ) { return (UInt32) status >= (UInt32) DATATABLE_STATUS_FAILED; }

/** The name of the status: "OK", "TYPE_MISMATCH", ... */
GUCEF_CORE_PUBLIC_CPP const char* DataTableStatusToString( TDataTableStatus status );

/** The base type name: "bool", "int8" .. "uint64", "float32", "float64", "decimal", "date", "timestamp", "utf8", "binary", "unsupported" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableLogicalTypeToString( TDataTableLogicalType type );

/** Parses a base type name of DataTableLogicalTypeToString(), without parameters */
GUCEF_CORE_PUBLIC_CPP bool DataTableLogicalTypeFromString( const CString& text, TDataTableLogicalType& type );

/** Bytes per value of the storage of the type: 1 to 8 for DATATABLE_STORAGE_FIXED types, 0 for bits, variable and unsupported */
GUCEF_CORE_PUBLIC_CPP UInt32 DataTableLogicalTypeByteWidth( TDataTableLogicalType type );

/** The storage the type uses when it is not dictionary encoded */
GUCEF_CORE_PUBLIC_CPP TDataTableStorage DataTableLogicalTypeStorage( TDataTableLogicalType type );

/** True for the integer types INT8 .. UINT64 */
GUCEF_CORE_PUBLIC_CPP bool DataTableLogicalTypeIsInteger( TDataTableLogicalType type );

/** True for the unsigned integer types UINT8 .. UINT64 */
GUCEF_CORE_PUBLIC_CPP bool DataTableLogicalTypeIsUnsigned( TDataTableLogicalType type );

/** "ms", "us", "ns" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableTimeUnitToString( TDataTableTimeUnit unit );
GUCEF_CORE_PUBLIC_CPP bool DataTableTimeUnitFromString( const CString& text, TDataTableTimeUnit& unit );

/** "identity", "year", "month", "day" */
GUCEF_CORE_PUBLIC_CPP const char* DataTablePartitionTransformToString( TDataTablePartitionTransform transform );
GUCEF_CORE_PUBLIC_CPP bool DataTablePartitionTransformFromString( const CString& text, TDataTablePartitionTransform& transform );

/** "auto", "plain", "dictionary" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableEncodingHintToString( TDataTableEncodingHint hint );
GUCEF_CORE_PUBLIC_CPP bool DataTableEncodingHintFromString( const CString& text, TDataTableEncodingHint& hint );

/** "auto", "always", "never" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableDictionaryModeToString( TDataTableDictionaryMode mode );
GUCEF_CORE_PUBLIC_CPP bool DataTableDictionaryModeFromString( const CString& text, TDataTableDictionaryMode& mode );

/** "insert", "upsert", "replacePartitions" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableWriteModeToString( TDataTableWriteMode mode );
GUCEF_CORE_PUBLIC_CPP bool DataTableWriteModeFromString( const CString& text, TDataTableWriteMode& mode );

/** "exact", "pruneOnly" */
GUCEF_CORE_PUBLIC_CPP const char* DataTableFilterModeToString( TDataTableFilterMode mode );
GUCEF_CORE_PUBLIC_CPP bool DataTableFilterModeFromString( const CString& text, TDataTableFilterMode& mode );

/** "equals", "notEquals", "less", "lessOrEqual", "greater", "greaterOrEqual", "range", "in", "isNull", "isNotNull" */
GUCEF_CORE_PUBLIC_CPP const char* DataTablePredicateOpToString( TDataTablePredicateOp op );
GUCEF_CORE_PUBLIC_CPP bool DataTablePredicateOpFromString( const CString& text, TDataTablePredicateOp& op );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DATATABLETYPES_H ? */
