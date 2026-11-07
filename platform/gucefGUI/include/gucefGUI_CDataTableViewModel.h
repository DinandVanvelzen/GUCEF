/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CDATATABLEVIEWMODEL_H
#define GUCEF_GUI_CDATATABLEVIEWMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Shows a CORE::CIDataTable in a grid with no glue code: a VFS table, a parquet file, a replicated grid of the
 *  network layer, a table in memory. The columns are the columns of the schema (or the ones chosen), typed.
 *
 *  Memory: the rows are read with a scan into batches, and the model keeps at most SetMaxCachedBatches() batches of
 *  SetBatchRowCount() rows, the least recently used go first, so the memory stays bounded however large the table
 *  is. The number of rows is found by one pass over the table the first time it is asked for, which also fills the
 *  cache with the first batches; a table smaller than the cache is read once. A scan can only go forward, so a row
 *  that is not cached and lies before the position of the scan restarts the scan; reading in the order the user
 *  scrolls costs one pass in total, jumping back to a row that left the cache costs a pass up to that row.
 *
 *  Filter: the predicates are given to the table with every scan (the table filters, where it can with its
 *  statistics and indices), so the model only ever sees the rows that match.
 *
 *  Sort: the scan interface has no ordering, so no table sorts for the model. The model orders by one column itself
 *  when the table has at most SetMaxSortRows() rows (0: any number): it reads the column once and keeps the order
 *  as a list of row numbers, 4 bytes per row. For a larger table IsSortSupported() is false and SortByColumn()
 *  returns false and says why in the log: the sort is reported as unsupported, never silently wrong. A table that
 *  keeps its rows in the order of a sort key (a memory table with a sort key, a parquet file written sorted) shows
 *  them in that order without any of this.
 *
 *  Identity: the key of a row is the number of the row in the scan of the table under the current filter. It
 *  survives sorting, and survives changes of the table that do not add or remove rows before it. After a filter
 *  change the numbers refer to the new result.
 *
 *  Changes: the model drops its cache and raises ModelResetEvent when the table raises PartitionWrittenEvent,
 *  PartitionDeletedEvent or RowsChangedEvent (a table that follows a replicated source), and when Invalidate() is
 *  called, which is the way for a table that raises none of these. The cache is refilled by the next read.
 *
 *  Used from one thread, the thread the grid is used from. The table itself is thread safe, as CIDataTable says.
 */
class GUCEF_GUI_PUBLIC_CPP CDataTableViewModel : public CITableViewModel
{
    public:

    /** A model on the table. Not valid when the table is NULL or has no columns, see IsValid() */
    explicit CDataTableViewModel( const CORE::CIDataTablePtr& table );

    virtual ~CDataTableViewModel() GUCEF_VIRTUAL_OVERRIDE;

    bool IsValid( void ) const;

    const CORE::CIDataTablePtr& GetTable( void ) const;

    /** Rows per batch of the scans; default 4096. The tables deliver batches of at least 1024 rows */
    void SetBatchRowCount( UInt32 rows );

    /** The batches kept; default 64; 0 for no maximum, the model then keeps everything it read */
    void SetMaxCachedBatches( UInt32 batches );

    /** The largest table the model sorts; default 1,000,000 rows; 0 for no maximum */
    void SetMaxSortRows( UInt32 rows );

    /** Shows these columns of the schema in this order; false when a name is not a column. Resets the model */
    bool SetColumns( const CString::StringVector& columnNames );

    /** Shows only the rows the predicates (AND'ed) select; false when they do not fit the schema. Resets the model */
    bool SetFilter( const CORE::CDataTableQuery::TPredicateVector& predicates );

    void ClearFilter( void );

    /** Back to the order of the table */
    void ClearSort( void );

    /** Drops what was read and raises ModelResetEvent; the next read goes back to the table */
    void Invalidate( void );

    /** The batches in the cache, the scans started so far (a row that left the cache costs one) and the batches a scan decoded and gave away */
    UInt32 GetCachedBatchCount( void ) const;

    UInt32 GetScansStarted( void ) const;

    UInt32 GetBatchesDecoded( void ) const;

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void PrepareRange( const CTableViewRange& visible ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetColumnSpan( UInt32 column, UInt32 firstRow, UInt32 rowCount, CORE::CVariant::VariantVector& values ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSortSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Every cell is read only: the model shows a table and writes nothing back (SetCellValue() is the default, which says read only), whatever the table allows */
    virtual bool IsCellReadOnly( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SortByColumn( UInt32 column, bool ascending ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CDataTableViewModel > TEventCallback;

    /** A run of rows of the scan: where it starts, how many rows it has and the batch when it is in the cache */
    struct SEntry
    {
        UInt64 firstRow;
        UInt32 rowCount;
        CORE::CDataTableBatchPtr batch;
        UInt64 lastUse;

        SEntry( void ) : firstRow( 0 ), rowCount( 0 ), batch(), lastUse( 0 ) {}
    };
    typedef GUCEF::vector< SEntry > TEntryVector;
    typedef GUCEF::vector< UInt32 > TRowVector;

    void OnPartitionWritten( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPartitionDeleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRowsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CDataTableViewModel( const CDataTableViewModel& src );
    CDataTableViewModel& operator=( const CDataTableViewModel& src );

    void BuildColumns( void );
    CORE::CDataTableQuery BuildQuery( void ) const;
    bool StartScan( void ) const;
    void DropScan( void ) const;
    void DropCache( void ) const;
    void TrimCacheTo( UInt32 keep ) const;
    bool EnsureCounted( void ) const;
    bool ApplySort( void ) const;
    bool EnsureEntry( UInt32 entryIndex ) const;
    void StoreBatch( SEntry& entry, const CORE::CDataTableBatchPtr& batch ) const;
    bool FindEntry( UInt64 sourceRow, UInt32& entryIndex ) const;
    bool ReadSource( UInt64 sourceRow, UInt32 projectedColumn, CORE::CVariant& value ) const;
    UInt64 ToSourceRow( UInt32 row ) const;

    mutable CORE::CIDataTablePtr m_table;
    CORE::CDataTableSchemaPtr m_schema;
    GUCEF::vector< CTableViewColumn > m_columns;
    GUCEF::vector< UInt32 > m_projection;                               /**< schema index of every shown column */
    CORE::CDataTableQuery::TPredicateVector m_predicates;
    UInt32 m_batchRowCount;
    UInt32 m_maxCachedBatches;
    UInt32 m_maxSortRows;

    // the cache and the scan are filled by the reads, which are const: what a read does to them is not a change of the model
    mutable bool m_counted;
    mutable UInt64 m_totalRows;
    mutable TEntryVector m_entries;
    mutable UInt32 m_cachedBatches;
    mutable UInt64 m_useCounter;
    mutable CORE::CDataTableScanPtr m_scan;
    mutable UInt32 m_scanNextEntry;                                             /**< the entry the next batch of the scan belongs to */

    mutable TRowVector m_order;                                                 /**< the scan row of every row shown, when sorted */
    mutable TRowVector m_inverseOrder;                                          /**< the row shown of every scan row, made when a key is looked up */
    mutable bool m_sorted;
    mutable UInt32 m_sortColumn;                                                /**< the column the order is by, GUCEF_UINT32MAX for none; kept when the cache is dropped, the order is made again */
    bool m_sortAscending;

    mutable UInt32 m_scansStarted;
    mutable UInt32 m_batchesDecoded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CDATATABLEVIEWMODEL_H ? */
