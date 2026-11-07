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

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#define GUCEF_CORE_CDATATABLEBATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#include "gucefCORE_CDataTableColumnBatch.h"
#define GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNBATCH_H ? */

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

/**
 *  Where the rows of a batch came from
 */
struct GUCEF_CORE_PUBLIC_CPP CDataTableBatchOrigin
{
    UInt32 fileOrdinal;             /**< position of the file in the scan's delivery order */
    UInt32 rowGroup;                /**< row group within the file */
    UInt64 firstRowInRowGroup;      /**< row of the row group that is row 0 of the batch */

    CDataTableBatchOrigin( void );
    bool operator==( const CDataTableBatchOrigin& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The projected columns of a table for one contiguous range of rows, plus an optional selection.
 *
 *  Reached only through CDataTableBatchPtr. A batch pins everything its columns point into; every pointer and linked
 *  variant from it is valid while a handle is held, and only until the last handle goes. Read only after delivery and
 *  readable from several threads at once.
 *
 *  Batch objects come from the free list of a CDataTableBufferPool (CDataTableBufferPool::AcquireBatch), so their
 *  members keep their capacity. The Set... and GetMutable... members are for the producer of the batch before it is
 *  shared.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableBatch
{
    public:

    inline UInt32 GetRowCount( void ) const { return m_rowCount; }

    /** projected columns */
    inline UInt32 GetColumnCount( void ) const { return m_columnCount; }

    /** The column batch of the projected column; an empty column batch for an index out of range */
    const CDataTableColumnBatch& GetColumn( UInt32 projectedColumn ) const;

    /** The schema column of the projected column, GUCEF_UINT32MAX for an index out of range */
    UInt32 GetSchemaColumnIndex( UInt32 projectedColumn ) const;

    const CDataTableSchemaPtr& GetSchema( void ) const;

    /** GUCEF_NULL: all rows are selected; otherwise GetSelectedRowCount() ascending row indices */
    inline const UInt32* GetSelection( void ) const { return m_selection; }

    /** The selected rows: the row count when there is no selection */
    inline UInt32 GetSelectedRowCount( void ) const { return GUCEF_NULL == m_selection ? m_rowCount : m_selectedRowCount; }

    /** file ordinal, row group, first row within the row group */
    const CDataTableBatchOrigin& GetOrigin( void ) const;

    /** Tier 1 access, see CDataTableColumnBatch::GetValue(); false for a row or column out of range */
    bool GetValue( UInt32 row, UInt32 projectedColumn, CVariant& out, bool linkIfPossible = true ) const;

    /** A snapshot of the reference count, for tests and diagnostics */
    UInt32 GetReferenceCount( void ) const;

    /** The schema the columns belong to; the batch holds one reference */
    void SetSchema( const CDataTableSchemaPtr& schema );

    /** Sets the row count of the batch; the columns are expected to hold that many rows */
    void SetRowCount( UInt32 rowCount );

    /** Sets the number of projected columns; new columns are empty. False when the column objects cannot be allocated */
    bool SetColumnCount( UInt32 projectedColumnCount );

    /** The column batch to fill; the index must be below GetColumnCount() */
    CDataTableColumnBatch& GetMutableColumn( UInt32 projectedColumn );

    void SetSchemaColumnIndex( UInt32 projectedColumn, UInt32 schemaColumnIndex );

    /** Ascending row indices of the selected rows, pinned by the owner; GUCEF_NULL selects every row */
    void SetSelection( const UInt32* selection, UInt32 selectedRowCount, const CDataTableBufferPtr& owner );

    void SetOrigin( const CDataTableBatchOrigin& origin );

    /**
     *  The predicates of the scan's resolved query that hold for every row of the batch, set by the reader that
     *  guaranteed them (a sorted slice): bit i stands for predicate i, predicates from 64 on are never marked.
     *  CDataTableFilter does not test them again. 0 when the reader guaranteed nothing.
     */
    inline UInt64 GetGuaranteedPredicates( void ) const { return m_guaranteedPredicates; }

    void SetGuaranteedPredicates( UInt64 predicateMask );

    /** Keeps the first rowCount rows of every column (no copy) and the selected rows among them */
    void Truncate( UInt32 rowCount );

    /** Keeps the first selectedRowCount selected rows: the selection, or the rows when there is none, is cut to that many */
    void TruncateSelection( UInt32 selectedRowCount );

    /** Releases the columns, the selection and the schema; keeps the capacity of the members */
    void Reset( void );

    private:
    friend class CDataTableBatchPtr;
    friend class CDataTableBufferPoolCore;

    typedef GUCEF::vector< CDataTableColumnBatch > TColumnBatchVector;
    typedef GUCEF::vector< UInt32 > TColumnIndexVector;

    explicit CDataTableBatch( CDataTableBufferPoolCore* core );
    ~CDataTableBatch();

    CDataTableBatch( const CDataTableBatch& src );                  /**< not implemented */
    CDataTableBatch& operator=( const CDataTableBatch& src );       /**< not implemented */

    void AddReference( void );
    void ReleaseReference( void );

    UInt32 m_rowCount;
    UInt32 m_columnCount;
    TColumnBatchVector m_columns;                   /**< sized to the largest column count seen, m_columnCount in use */
    TColumnIndexVector m_schemaColumnIndices;
    CDataTableSchemaPtr m_schema;
    const UInt32* m_selection;
    UInt32 m_selectedRowCount;
    CDataTableBufferPtr m_selectionOwner;
    CDataTableBatchOrigin m_origin;
    UInt64 m_guaranteedPredicates;
    MT::CTAtomic< UInt32 > m_refCount;
    CDataTableBufferPoolCore* m_core;
    CDataTableBatch* m_nextFree;                    /**< free list link while pooled */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */
