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

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDate;
class CDateTime;
class CDecimal;

/**
 *  Appends the values of one column into pooled buffers. Part of a CDataTableBatchBuilder.
 *
 *  Buffers grow by size class (doubling), so appending is amortized constant time, and since built batches hand
 *  their buffers back to the pool when they are released, a reused builder stops allocating once the pool holds
 *  buffers of the working size.
 *
 *  Every append checks the value against the column: a value of another type, out of range, needing rounding or a
 *  null in a column that is not nullable fails with DATATABLE_STATUS_TYPE_MISMATCH (GetLastStatus()) and appends
 *  nothing. Text is never parsed into a numeric column; parsing is the job of the codecs.
 *  One thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableColumnBuilder
{
    public:

    bool AppendNull( void );
    bool AppendBool( bool value );
    bool AppendInt64( Int64 value );                              /**< any integer column, range checked */
    bool AppendUInt64( UInt64 value );                            /**< any integer column, range checked */
    bool AppendFloat64( Float64 value );                          /**< FLOAT32 columns: exact conversion only (NaN and infinities convert) */
    bool AppendDecimal( const CDecimal& value );                  /**< rescaled exactly to the column scale; a value that needs rounding or exceeds the precision is TYPE_MISMATCH */
    bool AppendDecimalMantissa( Int64 mantissaAtColumnScale );    /**< within the column precision */
    bool AppendDate( Int32 daysSinceEpoch );
    bool AppendDate( const CDate& date );
    bool AppendTimestamp( Int64 ticksInColumnUnit );
    bool AppendDateTime( const CDateTime& dateTime );             /**< converted to UTC and the column unit, exactly */
    bool AppendUtf8( const char* bytes, UInt32 byteLength );      /**< UTF8 columns; the bytes are taken as they are */
    bool AppendString( const CString& value );                    /**< UTF8 columns; converted to UTF-8 when CString is not */
    bool AppendBinary( const void* bytes, UInt32 byteLength );    /**< BINARY columns */

    /**
     *  Converts the variant to the column type; refuses lossy conversions with TYPE_MISMATCH. A GUCEF_DATATYPE_NULL
     *  variant and an uninitialized one are null. Accepted: booleans for BOOL; integers (and integral decimals and
     *  floats) in range for integer columns; floats and integers that convert exactly for float columns; decimals and
     *  integers that rescale exactly for decimal columns; dates for DATE; timestamps of any unit and dates (midnight
     *  UTC) that convert exactly for TIMESTAMP; strings for UTF8; blobs, BSOBs and strings for BINARY.
     */
    bool Append( const CVariant& value );

    /** Bulk appends; validity is an optional LSB first bitmap of the values, bit set = present. All or nothing */
    bool AppendInt64s( const Int64* values, UInt32 count, const UInt8* validity = GUCEF_NULL );
    bool AppendInt32s( const Int32* values, UInt32 count, const UInt8* validity = GUCEF_NULL );
    bool AppendFloat64s( const Float64* values, UInt32 count, const UInt8* validity = GUCEF_NULL );

    UInt32 GetRowCount( void ) const;
    UInt32 GetNullCount( void ) const;
    TDataTableStatus GetLastStatus( void ) const;
    const CDataTableColumnDef& GetColumnDef( void ) const;

    /** Drops the appended rows; the buffers are kept for the next rows */
    void Reset( void );

    private:
    friend class CDataTableBatchBuilder;

    CDataTableColumnBuilder( void );
    CDataTableColumnBuilder( const CDataTableColumnDef& column, CDataTableBufferPool* pool );
    ~CDataTableColumnBuilder();

    CDataTableColumnBuilder( const CDataTableColumnBuilder& src );              /**< not implemented */
    CDataTableColumnBuilder& operator=( const CDataTableColumnBuilder& src );   /**< not implemented */

    bool Fail( TDataTableStatus status );
    bool PrepareRow( void );
    bool MarkPresent( void );
    bool CreateValidity( void );
    bool AppendFixed( const void* value );
    bool AppendVariable( const char* bytes, UInt32 byteLength );
    bool AppendIntegerChecked( Int64 signedValue, UInt64 unsignedValue, bool isUnsigned );
    void Truncate( UInt32 rowCount );
    TDataTableStatus BuildInto( CDataTableColumnBatch& column );

    CDataTableColumnDef m_column;
    CDataTableBufferPool* m_pool;
    TDataTableStorage m_storage;
    UInt32 m_width;
    UInt32 m_rowCount;
    UInt32 m_nullCount;
    UInt32 m_byteCount;
    TDataTableStatus m_lastStatus;
    CDataTableBufferPtr m_values;           /**< fixed values, bits or offsets */
    CDataTableBufferPtr m_bytes;            /**< bytes of variable values */
    CDataTableBufferPtr m_validity;
};

/*-------------------------------------------------------------------------*/

/**
 *  Builds batches of a schema row by row or column by column.
 *
 *  Append through the column builders (GetColumn) or a row at a time (AppendRow), then Build hands out a batch over
 *  the appended rows, taking over the buffers, and resets the builder. One thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBatchBuilder
{
    public:

    CDataTableBatchBuilder( const CDataTableSchemaPtr& schema, CDataTableBufferPool& pool );

    ~CDataTableBatchBuilder();

    /** The builder of the schema column; for an index out of range a builder whose appends fail with INVALID_ARGUMENT */
    CDataTableColumnBuilder& GetColumn( UInt32 schemaColumnIndex );

    UInt32 GetColumnCount( void ) const;

    /** One value per schema column. On failure no column keeps a value of the row */
    bool AppendRow( const CVariant* values, UInt32 valueCount );

    /** The rows of the columns when they agree, else the lowest */
    UInt32 GetRowCount( void ) const;

    /** Status of the last failed AppendRow or Build, OK otherwise */
    TDataTableStatus GetLastStatus( void ) const;

    /** Hands out a batch over the appended rows and resets the builder; every column must have the same row count */
    TDataTableStatus Build( CDataTableBatchPtr& batch, CString& error );

    /** Drops the appended rows */
    void Reset( void );

    const CDataTableSchemaPtr& GetSchema( void ) const;

    private:

    typedef GUCEF::vector< CDataTableColumnBuilder* > TColumnBuilderVector;

    CDataTableBatchBuilder( const CDataTableBatchBuilder& src );                /**< not implemented */
    CDataTableBatchBuilder& operator=( const CDataTableBatchBuilder& src );     /**< not implemented */

    CDataTableSchemaPtr m_schema;
    CDataTableBufferPool* m_pool;
    TColumnBuilderVector m_columns;
    CDataTableColumnBuilder* m_invalidColumn;
    TDataTableStatus m_lastStatus;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */
