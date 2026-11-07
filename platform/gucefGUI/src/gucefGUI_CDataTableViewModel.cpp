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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#include "gucefCORE_LoggingMacros.h"
#define GUCEF_CORE_LOGGINGMACROS_H
#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */

#include "gucefGUI_CDataTableViewModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The most rows a grid can address */
const UInt64 MaxViewRows = 0x7fffffff;

/** Orders row numbers by the values of one column; rows without a value (null) come last in both directions, equal values keep their order */
class CKeyLess
{
    public:

    CKeyLess( const GUCEF::vector< CORE::CVariant >& keys, bool ascending ) : m_keys( keys ), m_ascending( ascending ) {}

    bool operator()( UInt32 a, UInt32 b ) const
    {
        const CORE::CVariant& left = m_keys[ a ];
        const CORE::CVariant& right = m_keys[ b ];
        const bool leftEmpty = left.IsNULL() || left.IsNIL();
        const bool rightEmpty = right.IsNULL() || right.IsNIL();
        if ( leftEmpty || rightEmpty )
            return !leftEmpty && rightEmpty;
        return m_ascending ? left < right : right < left;
    }

    private:

    const GUCEF::vector< CORE::CVariant >& m_keys;
    bool m_ascending;
};

const CTableViewColumn&
EmptyColumn( void )
{
    static const CTableViewColumn column;
    return column;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableViewModel::CDataTableViewModel( const CORE::CIDataTablePtr& table )
    : CITableViewModel()
    , m_table( table )
    , m_schema()
    , m_columns()
    , m_projection()
    , m_predicates()
    , m_batchRowCount( 4096 )
    , m_maxCachedBatches( 64 )
    , m_maxSortRows( 1000000 )
    , m_counted( false )
    , m_totalRows( 0 )
    , m_entries()
    , m_cachedBatches( 0 )
    , m_useCounter( 0 )
    , m_scan()
    , m_scanNextEntry( 0 )
    , m_order()
    , m_inverseOrder()
    , m_sorted( false )
    , m_sortColumn( GUCEF_UINT32MAX )
    , m_sortAscending( true )
    , m_scansStarted( 0 )
    , m_batchesDecoded( 0 )
{GUCEF_TRACE;

    if ( m_table.IsNULL() )
        return;
    m_schema = m_table->GetSchema();
    if ( m_schema.IsNULL() )
        return;
    for ( UInt32 i=0; i<m_schema->GetColumnCount(); ++i )
        if ( CORE::DATATABLE_TYPE_UNSUPPORTED != m_schema->GetColumn( i ).type )
            m_projection.push_back( i );
    BuildColumns();

    TEventCallback writtenCallback( this, &CDataTableViewModel::OnPartitionWritten );
    SubscribeTo( m_table.GetPointerAlways(), CORE::CIDataTable::PartitionWrittenEvent, writtenCallback );
    TEventCallback deletedCallback( this, &CDataTableViewModel::OnPartitionDeleted );
    SubscribeTo( m_table.GetPointerAlways(), CORE::CIDataTable::PartitionDeletedEvent, deletedCallback );
    TEventCallback changedCallback( this, &CDataTableViewModel::OnRowsChanged );
    SubscribeTo( m_table.GetPointerAlways(), CORE::CIDataTable::RowsChangedEvent, changedCallback );
}

/*-------------------------------------------------------------------------*/

CDataTableViewModel::~CDataTableViewModel()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::IsValid( void ) const
{GUCEF_TRACE;

    return !m_table.IsNULL() && !m_schema.IsNULL() && !m_projection.empty();
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CDataTableViewModel::GetTable( void ) const
{GUCEF_TRACE;

    return m_table;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::SetBatchRowCount( UInt32 rows )
{GUCEF_TRACE;

    m_batchRowCount = rows;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::SetMaxCachedBatches( UInt32 batches )
{GUCEF_TRACE;

    m_maxCachedBatches = batches;
    if ( batches > 0 )
        TrimCacheTo( batches );
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::TrimCacheTo( UInt32 keep ) const
{GUCEF_TRACE;

    // the least recently used batches go until no more than 'keep' are left
    while ( m_cachedBatches > keep )
    {
        size_t oldest = m_entries.size();
        for ( size_t i=0; i<m_entries.size(); ++i )
            if ( !m_entries[ i ].batch.IsNULL() && ( oldest == m_entries.size() || m_entries[ i ].lastUse < m_entries[ oldest ].lastUse ) )
                oldest = i;
        if ( oldest == m_entries.size() )
            break;
        m_entries[ oldest ].batch = CORE::CDataTableBatchPtr();
        --m_cachedBatches;
    }
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::SetMaxSortRows( UInt32 rows )
{GUCEF_TRACE;

    m_maxSortRows = rows;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::BuildColumns( void )
{GUCEF_TRACE;

    m_columns.clear();
    for ( size_t i=0; i<m_projection.size(); ++i )
    {
        const CORE::CDataTableColumnDef& definition = m_schema->GetColumn( m_projection[ i ] );
        CTableViewColumn column( definition.name, definition.type, 0, true );
        if ( CORE::DATATABLE_TYPE_DECIMAL == definition.type )
            column.decimals = definition.decimalScale;
        m_columns.push_back( column );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableQuery
CDataTableViewModel::BuildQuery( void ) const
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    for ( size_t i=0; i<m_projection.size(); ++i )
        query.Select( m_projection[ i ] );
    for ( size_t i=0; i<m_predicates.size(); ++i )
        query.Where( m_predicates[ i ] );
    query.SetBatchRowCount( m_batchRowCount );
    return query;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::StartScan( void ) const
{GUCEF_TRACE;

    ++m_scansStarted;
    m_scanNextEntry = 0;
    m_scan = CORE::CDataTableScanPtr();
    const CORE::TDataTableStatus status = m_table->Scan( BuildQuery(), m_scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CDataTableViewModel: The scan of table \"" << m_table->GetName() << "\" did not start: "
                         << CORE::DataTableStatusToString( status ) << ", " << m_table->GetLastErrorMessage() );
        m_scan = CORE::CDataTableScanPtr();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::DropScan( void ) const
{GUCEF_TRACE;

    m_scan = CORE::CDataTableScanPtr();
    m_scanNextEntry = 0;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::DropCache( void ) const
{GUCEF_TRACE;

    DropScan();
    m_entries.clear();
    m_cachedBatches = 0;
    m_counted = false;
    m_totalRows = 0;
    m_sorted = false;
    m_order.clear();
    m_inverseOrder.clear();
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::StoreBatch( SEntry& entry, const CORE::CDataTableBatchPtr& batch ) const
{GUCEF_TRACE;

    if ( entry.batch.IsNULL() )
        ++m_cachedBatches;
    entry.batch = batch;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::EnsureCounted( void ) const
{GUCEF_TRACE;

    if ( m_counted )
        return true;
    if ( !IsValid() )
        return false;

    m_entries.clear();
    m_cachedBatches = 0;
    m_totalRows = 0;
    m_sorted = false;
    m_order.clear();
    m_inverseOrder.clear();
    m_counted = true;

    if ( !StartScan() )
        return false;
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    for (;;)
    {
        CORE::CDataTableBatchPtr batch;
        status = m_scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_OK != status || batch.IsNULL() )
            break;
        ++m_batchesDecoded;
        const UInt32 rows = batch->GetSelectedRowCount();
        if ( 0 == rows )
            continue;
        SEntry entry;
        entry.firstRow = m_totalRows;
        entry.rowCount = rows;
        entry.lastUse = ++m_useCounter;
        if ( 0 == m_maxCachedBatches || m_cachedBatches < m_maxCachedBatches )
            StoreBatch( entry, batch );
        m_entries.push_back( entry );
        m_totalRows += rows;
    }
    if ( CORE::DATATABLE_STATUS_END != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CDataTableViewModel: The scan of table \"" << m_table->GetName() << "\" ended with "
                         << CORE::DataTableStatusToString( status ) << " after " << m_totalRows << " rows" );
    }
    m_scanNextEntry = (UInt32) m_entries.size();

    // a sort the user asked for stays: the order is made again for the rows there are now
    if ( m_sortColumn != GUCEF_UINT32MAX )
        ApplySort();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::FindEntry( UInt64 sourceRow, UInt32& entryIndex ) const
{GUCEF_TRACE;

    size_t low = 0, high = m_entries.size();
    while ( low < high )
    {
        const size_t middle = low + ( high - low ) / 2;
        if ( m_entries[ middle ].firstRow + m_entries[ middle ].rowCount <= sourceRow )
            low = middle + 1;
        else
            high = middle;
    }
    if ( low >= m_entries.size() || sourceRow < m_entries[ low ].firstRow )
        return false;
    entryIndex = (UInt32) low;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::EnsureEntry( UInt32 entryIndex ) const
{GUCEF_TRACE;

    if ( entryIndex >= m_entries.size() )
        return false;
    m_entries[ entryIndex ].lastUse = ++m_useCounter;
    if ( !m_entries[ entryIndex ].batch.IsNULL() )
        return true;

    // the least recently used batch makes room
    if ( m_maxCachedBatches > 0 && m_cachedBatches >= m_maxCachedBatches )
        TrimCacheTo( m_maxCachedBatches - 1 );

    // the scan only goes forward
    if ( m_scan.IsNULL() || m_scanNextEntry > entryIndex )
    {
        if ( !StartScan() )
            return false;
    }
    while ( m_scanNextEntry <= entryIndex )
    {
        CORE::CDataTableBatchPtr batch;
        const CORE::TDataTableStatus status = m_scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_OK != status || batch.IsNULL() )
        {
            // fewer rows than counted: the table changed since, the next read counts again
            DropCache();
            NotifyObservers( ModelResetEvent );
            return false;
        }
        ++m_batchesDecoded;
        if ( 0 == batch->GetSelectedRowCount() )
            continue;
        const UInt32 current = m_scanNextEntry++;
        if ( current >= m_entries.size() || batch->GetSelectedRowCount() != m_entries[ current ].rowCount )
        {
            DropCache();
            NotifyObservers( ModelResetEvent );
            return false;
        }
        if ( current == entryIndex )
            StoreBatch( m_entries[ current ], batch );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableViewModel::ToSourceRow( UInt32 row ) const
{GUCEF_TRACE;

    return m_sorted && row < m_order.size() ? (UInt64) m_order[ row ] : (UInt64) row;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::ReadSource( UInt64 sourceRow, UInt32 projectedColumn, CORE::CVariant& value ) const
{GUCEF_TRACE;

    UInt32 entryIndex = 0;
    if ( !FindEntry( sourceRow, entryIndex ) || !EnsureEntry( entryIndex ) )
        return false;
    const SEntry& entry = m_entries[ entryIndex ];
    const UInt32 local = (UInt32) ( sourceRow - entry.firstRow );
    const UInt32* selection = entry.batch->GetSelection();
    return entry.batch->GetValue( GUCEF_NULL != selection ? selection[ local ] : local, projectedColumn, value, false );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableViewModel::GetRowCount( void ) const
{GUCEF_TRACE;

    if ( !EnsureCounted() )
        return 0;
    return (UInt32) ( m_totalRows > MaxViewRows ? MaxViewRows : m_totalRows );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableViewModel::GetColumnCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_columns.size();
}

/*-------------------------------------------------------------------------*/

const CTableViewColumn&
CDataTableViewModel::GetColumn( UInt32 column ) const
{GUCEF_TRACE;

    return column < m_columns.size() ? m_columns[ column ] : EmptyColumn();
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const
{GUCEF_TRACE;

    if ( column >= m_projection.size() || row >= GetRowCount() )
        return false;
    return ReadSource( ToSourceRow( row ), column, value );
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::PrepareRange( const CTableViewRange& visible )
{GUCEF_TRACE;

    const UInt32 count = GetRowCount();
    UInt32 last = GUCEF_UINT32MAX;
    for ( UInt32 row = visible.firstRow; row < count && row - visible.firstRow < visible.rowCount; ++row )
    {
        UInt32 entryIndex = 0;
        if ( FindEntry( ToSourceRow( row ), entryIndex ) && entryIndex != last )
        {
            EnsureEntry( entryIndex );
            last = entryIndex;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::GetColumnSpan( UInt32 column, UInt32 firstRow, UInt32 rowCount, CORE::CVariant::VariantVector& values ) const
{GUCEF_TRACE;

    values.clear();
    if ( column >= m_projection.size() )
        return false;
    const UInt32 total = GetRowCount();
    if ( firstRow >= total )
        return true;
    const UInt32 count = rowCount < total - firstRow ? rowCount : total - firstRow;
    values.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariant value;
        ReadSource( ToSourceRow( firstRow + i ), column, value );
        values.push_back( value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::GetRowKey( UInt32 row, CORE::CVariant& key ) const
{GUCEF_TRACE;

    if ( row >= GetRowCount() )
        return false;
    key = (UInt64) ToSourceRow( row );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::FindRowByKey( const CORE::CVariant& key, UInt32& row ) const
{GUCEF_TRACE;

    if ( !key.IsNumber() || !EnsureCounted() )
        return false;
    const UInt64 sourceRow = key.AsUInt64();
    if ( sourceRow >= m_totalRows || sourceRow > MaxViewRows )
        return false;
    if ( !m_sorted )
    {
        row = (UInt32) sourceRow;
        return true;
    }
    if ( m_inverseOrder.size() != m_order.size() )
    {
        m_inverseOrder.assign( m_order.size(), 0 );
        for ( size_t i=0; i<m_order.size(); ++i )
            m_inverseOrder[ m_order[ i ] ] = (UInt32) i;
    }
    row = m_inverseOrder[ (size_t) sourceRow ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::IsSortSupported( void ) const
{GUCEF_TRACE;

    if ( !EnsureCounted() || m_columns.empty() )
        return false;
    return 0 == m_maxSortRows || m_totalRows <= m_maxSortRows;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::IsCellReadOnly( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::ApplySort( void ) const
{GUCEF_TRACE;

    m_sorted = false;
    m_order.clear();
    m_inverseOrder.clear();
    if ( m_sortColumn >= m_projection.size() )
        return false;
    if ( m_maxSortRows > 0 && m_totalRows > m_maxSortRows )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CDataTableViewModel: Table \"" << m_table->GetName() << "\" has " << m_totalRows << " rows, more than the "
                           << m_maxSortRows << " rows the model sorts: the sort is not supported, the rows stay in the order of the table" );
        m_sortColumn = GUCEF_UINT32MAX;
        return false;
    }

    // the column is read once, in the order of the scan
    GUCEF::vector< CORE::CVariant > keys;
    keys.resize( (size_t) m_totalRows );
    for ( UInt64 i=0; i<m_totalRows; ++i )
        ReadSource( i, m_sortColumn, keys[ (size_t) i ] );
    m_order.resize( (size_t) m_totalRows );
    for ( size_t i=0; i<m_order.size(); ++i )
        m_order[ i ] = (UInt32) i;
    std::stable_sort( m_order.begin(), m_order.end(), CKeyLess( keys, m_sortAscending ) );
    m_sorted = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::SortByColumn( UInt32 column, bool ascending )
{GUCEF_TRACE;

    if ( !IsSortSupported() || column >= m_projection.size() )
        return false;
    m_sortColumn = column;
    m_sortAscending = ascending;
    if ( !ApplySort() )
        return false;
    NotifyObservers( ModelResetEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::ClearSort( void )
{GUCEF_TRACE;

    m_sortColumn = GUCEF_UINT32MAX;
    m_sorted = false;
    m_order.clear();
    m_inverseOrder.clear();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::SetColumns( const CString::StringVector& columnNames )
{GUCEF_TRACE;

    if ( m_schema.IsNULL() || columnNames.empty() )
        return false;
    GUCEF::vector< UInt32 > projection;
    for ( size_t i=0; i<columnNames.size(); ++i )
    {
        const Int32 index = m_schema->FindColumn( columnNames[ i ] );
        if ( index < 0 || CORE::DATATABLE_TYPE_UNSUPPORTED == m_schema->GetColumn( (UInt32) index ).type )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CDataTableViewModel: Table \"" << m_table->GetName() << "\" has no column \"" << columnNames[ i ] << "\"" );
            return false;
        }
        projection.push_back( (UInt32) index );
    }
    m_projection = projection;
    m_sortColumn = GUCEF_UINT32MAX;
    BuildColumns();
    DropCache();
    NotifyObservers( ColumnsChangedEvent );
    NotifyObservers( ModelResetEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableViewModel::SetFilter( const CORE::CDataTableQuery::TPredicateVector& predicates )
{GUCEF_TRACE;

    if ( m_schema.IsNULL() )
        return false;
    CORE::CDataTableQuery query;
    for ( size_t i=0; i<predicates.size(); ++i )
        query.Where( predicates[ i ] );
    CORE::CDataTableResolvedQuery resolved;
    CString error;
    if ( !query.Resolve( *m_schema, resolved, error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CDataTableViewModel: The filter does not fit table \"" << m_table->GetName() << "\": " << error );
        return false;
    }
    m_predicates = predicates;
    DropCache();
    NotifyObservers( ModelResetEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::ClearFilter( void )
{GUCEF_TRACE;

    m_predicates.clear();
    DropCache();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::Invalidate( void )
{GUCEF_TRACE;

    if ( !m_table.IsNULL() )
        m_table->Refresh();
    DropCache();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableViewModel::GetCachedBatchCount( void ) const
{GUCEF_TRACE;

    return m_cachedBatches;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableViewModel::GetScansStarted( void ) const
{GUCEF_TRACE;

    return m_scansStarted;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableViewModel::GetBatchesDecoded( void ) const
{GUCEF_TRACE;

    return m_batchesDecoded;
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::OnPartitionWritten( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    DropCache();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::OnPartitionDeleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    DropCache();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

void
CDataTableViewModel::OnRowsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    DropCache();
    NotifyObservers( ModelResetEvent );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableViewModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CDataTableViewModel";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
