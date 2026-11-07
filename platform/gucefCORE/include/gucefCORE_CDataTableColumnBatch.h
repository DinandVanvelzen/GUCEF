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

#ifndef GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#define GUCEF_CORE_CDATATABLECOLUMNBATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFER_H
#include "gucefCORE_CDataTableBuffer.h"
#define GUCEF_CORE_CDATATABLEBUFFER_H
#endif /* GUCEF_CORE_CDATATABLEBUFFER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHPTR_H
#include "gucefCORE_CDataTableBatchPtr.h"
#define GUCEF_CORE_CDATATABLEBATCHPTR_H
#endif /* GUCEF_CORE_CDATATABLEBATCHPTR_H ? */

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

class CDataTableColumnDef;
class CDataTableBufferPool;

/**
 *  One column for a contiguous range of rows: the value model of section 3 of the data tables design.
 *
 *  Storage is spaced: every row has a slot, nulls included, and null slots are zero (0, 0.0, a cleared bit,
 *  dictionary index 0, an empty value). The optional validity bitmap is LSB first, bit set = value present;
 *  GetValidity() is GUCEF_NULL when the column batch has no nulls. Variable values are followed by one zero byte that
 *  is counted in the offsets, so value i is bytes [ offsets[ i ], offsets[ i + 1 ] - 1 ) and its terminator is at
 *  offsets[ i + 1 ] - 1.
 *
 *  Three access tiers: Tier 1 GetValue() into a CVariant (fixed size values inline, text and binary linked into the
 *  buffers), Tier 2 typed spans for hot loops (inline, GUCEF_NULL for the wrong type, one compare), and scans.
 *
 *  The column batch pins the buffers its spans point into. Every pointer it hands out, and every variant linked by
 *  GetValue(), is valid while the batch holding it is held. Read only after delivery and readable from several
 *  threads at once; the Set... members are for producers (builders, readers) before the batch is shared.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableColumnBatch
{
    public:

    CDataTableColumnBatch( void );

    CDataTableColumnBatch( const CDataTableColumnBatch& src );

    ~CDataTableColumnBatch();

    CDataTableColumnBatch& operator=( const CDataTableColumnBatch& src );

    inline TDataTableLogicalType GetLogicalType( void ) const { return m_type; }
    inline TDataTableStorage GetStorage( void ) const { return m_storage; }
    inline UInt32 GetRowCount( void ) const { return m_rowCount; }
    inline UInt8 GetDecimalPrecision( void ) const { return m_decimalPrecision; }
    inline UInt8 GetDecimalScale( void ) const { return m_decimalScale; }
    inline TDataTableTimeUnit GetTimeUnit( void ) const { return m_timeUnit; }
    inline bool IsUtcAdjusted( void ) const { return m_utcAdjusted; }

    inline bool HasNulls( void ) const { return m_nullCount > 0; }
    inline UInt32 GetNullCount( void ) const { return m_nullCount; }
    inline const UInt8* GetValidity( void ) const { return m_validity; }           /**< GUCEF_NULL when the batch has no nulls */
    inline bool IsValid( UInt32 row ) const { return GUCEF_NULL == m_validity || 0 != ( m_validity[ row >> 3 ] & ( 1u << ( row & 7u ) ) ); }

    inline const Int8* GetInt8Data( void ) const { return PHYSICAL_INT8 == m_physical ? static_cast< const Int8* >( m_values ) : GUCEF_NULL; }        /**< GUCEF_NULL unless the storage is that type */
    inline const Int16* GetInt16Data( void ) const { return PHYSICAL_INT16 == m_physical ? static_cast< const Int16* >( m_values ) : GUCEF_NULL; }
    inline const Int32* GetInt32Data( void ) const { return PHYSICAL_INT32 == m_physical ? static_cast< const Int32* >( m_values ) : GUCEF_NULL; }     /**< INT32 and DATE (days) */
    inline const Int64* GetInt64Data( void ) const { return PHYSICAL_INT64 == m_physical ? static_cast< const Int64* >( m_values ) : GUCEF_NULL; }     /**< INT64, DECIMAL (mantissas), TIMESTAMP (ticks) */
    inline const UInt8* GetUInt8Data( void ) const { return PHYSICAL_UINT8 == m_physical ? static_cast< const UInt8* >( m_values ) : GUCEF_NULL; }
    inline const UInt16* GetUInt16Data( void ) const { return PHYSICAL_UINT16 == m_physical ? static_cast< const UInt16* >( m_values ) : GUCEF_NULL; }
    inline const UInt32* GetUInt32Data( void ) const { return PHYSICAL_UINT32 == m_physical ? static_cast< const UInt32* >( m_values ) : GUCEF_NULL; }
    inline const UInt64* GetUInt64Data( void ) const { return PHYSICAL_UINT64 == m_physical ? static_cast< const UInt64* >( m_values ) : GUCEF_NULL; }
    inline const Float32* GetFloat32Data( void ) const { return PHYSICAL_FLOAT32 == m_physical ? static_cast< const Float32* >( m_values ) : GUCEF_NULL; }
    inline const Float64* GetFloat64Data( void ) const { return PHYSICAL_FLOAT64 == m_physical ? static_cast< const Float64* >( m_values ) : GUCEF_NULL; }
    inline const UInt8* GetBoolBits( void ) const { return PHYSICAL_BITS == m_physical ? static_cast< const UInt8* >( m_values ) : GUCEF_NULL; }

    /** The fixed width value storage of any type, GUCEF_NULL for other storage */
    inline const void* GetFixedData( void ) const { return DATATABLE_STORAGE_FIXED == m_storage ? m_values : GUCEF_NULL; }

    inline const UInt32* GetOffsets( void ) const { return DATATABLE_STORAGE_VARIABLE == m_storage ? m_offsets : GUCEF_NULL; }     /**< STORAGE_VARIABLE: rowCount + 1 entries */
    inline const char* GetBytes( void ) const { return DATATABLE_STORAGE_VARIABLE == m_storage ? m_bytes : GUCEF_NULL; }

    /**
     *  The value of a variable or dictionary column: the pointer and the length without the terminator.
     *  The row must be below GetRowCount(); GUCEF_NULL for other storage.
     */
    inline const char* GetVariableValue( UInt32 row, UInt32& byteLength ) const
    {
        if ( DATATABLE_STORAGE_VARIABLE == m_storage )
        {
            const UInt32 begin = m_offsets[ row ];
            byteLength = m_offsets[ row + 1 ] - begin - 1;
            return m_bytes + begin;
        }
        if ( DATATABLE_STORAGE_DICTIONARY == m_storage )
            return m_dictionary->GetVariableValue( m_indices[ row ], byteLength );
        byteLength = 0;
        return GUCEF_NULL;
    }

    inline const UInt32* GetDictionaryIndices( void ) const { return DATATABLE_STORAGE_DICTIONARY == m_storage ? m_indices : GUCEF_NULL; }    /**< STORAGE_DICTIONARY */
    inline const CDataTableColumnBatch* GetDictionary( void ) const { return DATATABLE_STORAGE_DICTIONARY == m_storage ? m_dictionary : GUCEF_NULL; }

    /**
     *  The batch holding the dictionary of a DATATABLE_STORAGE_DICTIONARY column, a NULL handle otherwise. For consumers
     *  that keep work per dictionary (a filter's dictionary bitmap) and hold the dictionary while they do.
     */
    inline const CDataTableBatchPtr& GetDictionaryBatch( void ) const { return m_dictionaryOwner; }

    /**
     *  Tier 1. A null cell sets out to GUCEF_DATATYPE_NULL and returns true. Fixed size values are copied inline:
     *  bool as GUCEF_DATATYPE_BOOLEAN_INT32, integers and floats under their native type id, decimals as CDecimal,
     *  dates under GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH, timestamps under the id of their unit. UTF8 and
     *  BINARY values are linked into the buffer when linkIfPossible is set (UTF8 including its terminator), copied
     *  otherwise. False for a row out of range, an UNSUPPORTED column, and a decimal mantissa outside 18 digits; out is
     *  cleared then.
     */
    bool GetValue( UInt32 row, CVariant& out, bool linkIfPossible = true ) const;

    /** Releases all buffers; type and type parameters stay, storage becomes NONE and the row count 0 */
    void Reset( void );

    /** Sets the logical type and its parameters; releases the values */
    void SetType( TDataTableLogicalType type                                        ,
                  UInt8 decimalPrecision = DataTableMaxDecimalPrecision            ,
                  UInt8 decimalScale = 0                                            ,
                  TDataTableTimeUnit timeUnit = DATATABLE_TIMEUNIT_MICROSECONDS     ,
                  bool utcAdjusted = true                                           );

    /** Sets the logical type and its parameters from a column definition; releases the values */
    void SetType( const CDataTableColumnDef& column );

    /**
     *  Fixed width values (DATATABLE_STORAGE_FIXED) or bits (BOOL, DATATABLE_STORAGE_BITS) of the type, rowCount of them,
     *  in native byte order, pinned by the owner (which may hold them at an offset). Clears the validity.
     */
    void SetFixedValues( UInt32 rowCount, const void* values, const CDataTableBufferPtr& owner );

    /** rowCount + 1 offsets into the bytes, each value followed by its terminator; for UTF8 and BINARY. Clears the validity */
    void SetVariableValues( UInt32 rowCount                         ,
                            const UInt32* offsets                   ,
                            const CDataTableBufferPtr& offsetsOwner ,
                            const char* bytes                       ,
                            const CDataTableBufferPtr& bytesOwner   );

    /**
     *  rowCount indices into the dictionary: a column of the dictionary batch of DATATABLE_STORAGE_VARIABLE storage and
     *  the same type, shared by the batches of one row group. Clears the validity.
     */
    void SetDictionaryValues( UInt32 rowCount                           ,
                              const UInt32* indices                     ,
                              const CDataTableBufferPtr& indicesOwner   ,
                              const CDataTableBatchPtr& dictionary      ,
                              UInt32 dictionaryColumn = 0               );

    /** The validity bitmap with the number of cleared bits among the rows, or GUCEF_NULL (and 0) for no nulls */
    void SetValidity( const UInt8* validity, UInt32 nullCount, const CDataTableBufferPtr& owner );

    /**
     *  Makes this column a view of rows [ firstRow, firstRow + rowCount ) of the source, sharing its buffers: fixed width
     *  values, offsets and dictionary indices are used in place. Bits (BOOL values and the validity) that do not start
     *  on a byte boundary are copied, shifted, into a buffer of the pool. The source may be this column.
     *  INVALID_ARGUMENT for a range outside the source; OUT_OF_MEMORY or LIMIT_EXCEEDED from the pool.
     */
    TDataTableStatus SetSlice( const CDataTableColumnBatch& source, UInt32 firstRow, UInt32 rowCount, CDataTableBufferPool& pool );

    /** Keeps the first rowCount rows (no copy) and recounts the nulls among them; a count at or above the row count changes nothing */
    void Truncate( UInt32 rowCount );

    /**
     *  Checks the column batch is consistent: spans present for the storage, offsets ascending within the bytes with a
     *  zero terminator for each value, dictionary indices within the dictionary, the null count matching the validity
     *  and null slots zero. For values from untrusted sources and for tests; costs a pass over the column.
     */
    bool Validate( CString& error ) const;

    private:

    enum EPhysical
    {
        PHYSICAL_NONE = 0   ,
        PHYSICAL_BITS       ,
        PHYSICAL_INT8       ,
        PHYSICAL_INT16      ,
        PHYSICAL_INT32      ,
        PHYSICAL_INT64      ,
        PHYSICAL_UINT8      ,
        PHYSICAL_UINT16     ,
        PHYSICAL_UINT32     ,
        PHYSICAL_UINT64     ,
        PHYSICAL_FLOAT32    ,
        PHYSICAL_FLOAT64    ,
        PHYSICAL_VARIABLE   ,
        PHYSICAL_DICTIONARY
    };

    void UpdatePhysical( void );
    bool GetVariableCell( const char* bytes, UInt32 byteLength, CVariant& out, bool linkIfPossible ) const;

    TDataTableLogicalType m_type;
    TDataTableStorage m_storage;
    EPhysical m_physical;
    UInt32 m_rowCount;
    UInt32 m_nullCount;
    UInt8 m_decimalPrecision;
    UInt8 m_decimalScale;
    TDataTableTimeUnit m_timeUnit;
    bool m_utcAdjusted;

    const void* m_values;
    const UInt32* m_offsets;
    const char* m_bytes;
    const UInt32* m_indices;
    const UInt8* m_validity;
    const CDataTableColumnBatch* m_dictionary;

    CDataTableBufferPtr m_valuesOwner;          /**< values, offsets or indices */
    CDataTableBufferPtr m_bytesOwner;           /**< bytes of variable values */
    CDataTableBufferPtr m_validityOwner;
    CDataTableBatchPtr m_dictionaryOwner;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLECOLUMNBATCH_H ? */
