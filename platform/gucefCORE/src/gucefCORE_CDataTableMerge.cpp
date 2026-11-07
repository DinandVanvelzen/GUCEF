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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEFILTER_H
#include "gucefCORE_CDataTableFilter.h"
#define GUCEF_CORE_CDATATABLEFILTER_H
#endif /* GUCEF_CORE_CDATATABLEFILTER_H ? */

#include "gucefCORE_CDataTableMerge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A row of one of a list of full batches */
struct SRowRef
{
    UInt32 batch;
    UInt32 row;
};
typedef GUCEF::vector< SRowRef > TRowRefVector;

/** Orders rows by their position: batch, then row */
struct SPositionLess
{
    bool operator()( const SRowRef& a, const SRowRef& b ) const { return a.batch < b.batch || ( a.batch == b.batch && a.row < b.row ); }
};

template< typename T >
inline Int32
CompareScalars( T a, T b )
{
    return a < b ? -1 : ( b < a ? 1 : 0 );
}

template< typename T >
inline Int32
CompareFloats( T a, T b )
{
    const bool aNaN = a != a;
    const bool bNaN = b != b;
    if ( aNaN || bNaN )
        return aNaN == bNaN ? 0 : ( aNaN ? 1 : -1 );
    return a < b ? -1 : ( b < a ? 1 : 0 );
}

/** Orders rows of full batches on the given schema columns */
class CRowLess
{
    public:

    CRowLess( const CDataTableMerge::TBatchVector& batches, const CDataTableSchema::TColumnIndexVector& columns )
        : m_batches( &batches )
        , m_columns( &columns )
    {
    }

    Int32 Compare( const SRowRef& a, const SRowRef& b ) const
    {
        const CDataTableBatch& left = *(*m_batches)[ a.batch ];
        const CDataTableBatch& right = *(*m_batches)[ b.batch ];
        for ( UInt32 i=0; i<(UInt32) m_columns->size(); ++i )
        {
            const UInt32 column = (*m_columns)[ i ];
            const Int32 order = CDataTableMerge::CompareCells( left.GetColumn( column ), a.row, right.GetColumn( column ), b.row );
            if ( 0 != order )
                return order;
        }
        return 0;
    }

    bool operator()( const SRowRef& a, const SRowRef& b ) const
    {
        return Compare( a, b ) < 0;
    }

    private:

    const CDataTableMerge::TBatchVector* m_batches;
    const CDataTableSchema::TColumnIndexVector* m_columns;
};

void
AppendAllRows( const CDataTableMerge::TBatchVector& batches, UInt32 batchOffset, TRowRefVector& rows )
{
    for ( UInt32 b=0; b<(UInt32) batches.size(); ++b )
    {
        if ( batches[ b ].IsNULL() )
            continue;
        const UInt32 rowCount = batches[ b ]->GetRowCount();
        for ( UInt32 r=0; r<rowCount; ++r )
        {
            SRowRef ref;
            ref.batch = batchOffset + b;
            ref.row = r;
            rows.push_back( ref );
        }
    }
}

/** Builds full batches of the rows, in the order given, of at most rowGroupRowCount rows each */
TDataTableStatus
Materialize( const CDataTableSchemaPtr& schema, const CDataTableMerge::TBatchVector& batches, const TRowRefVector& rows,
             UInt32 rowGroupRowCount, CDataTableBufferPool& pool, CDataTableMerge::TBatchVector& result, CString& error )
{
    if ( 0 == rowGroupRowCount )
        rowGroupRowCount = 1;
    const UInt32 columnCount = schema->GetColumnCount();
    CDataTableBatchBuilder builder( schema, pool );
    GUCEF::vector< CVariant > cells( columnCount );
    for ( UInt32 i=0; i<(UInt32) rows.size(); ++i )
    {
        const CDataTableBatch& source = *batches[ rows[ i ].batch ];
        for ( UInt32 c=0; c<columnCount; ++c )
            source.GetColumn( c ).GetValue( rows[ i ].row, cells[ c ], true );
        if ( !builder.AppendRow( columnCount > 0 ? &cells[ 0 ] : GUCEF_NULL, columnCount ) )
        {
            error = "A row could not be copied: " + CString( DataTableStatusToString( builder.GetLastStatus() ) );
            return builder.GetLastStatus();
        }
        if ( builder.GetRowCount() >= rowGroupRowCount || i + 1 == (UInt32) rows.size() )
        {
            CDataTableBatchPtr batch;
            TDataTableStatus status = builder.Build( batch, error );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            result.push_back( batch );
        }
    }
    return DATATABLE_STATUS_OK;
}

/** Keeps per run of equal keys the last row in the given order: the rows are stably sorted by key first */
void
KeepLastPerKey( const CRowLess& keyLess, TRowRefVector& rows, UInt64& removed )
{
    // stable sort by key keeps the order within a key; the last of each run wins
    std::stable_sort( rows.begin(), rows.end(), keyLess );
    UInt32 kept = 0;
    for ( UInt32 i=0; i<(UInt32) rows.size(); ++i )
    {
        if ( i + 1 < (UInt32) rows.size() && 0 == keyLess.Compare( rows[ i ], rows[ i + 1 ] ) )
        {
            ++removed;
            continue;
        }
        rows[ kept++ ] = rows[ i ];
    }
    rows.resize( kept );
}

template< typename T >
void
FixedMinMax( const T* values, const CDataTableColumnBatch& column, bool& found, T& minValue, T& maxValue )
{
    const UInt32 rowCount = column.GetRowCount();
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        if ( !column.IsValid( r ) )
            continue;
        const T value = values[ r ];
        if ( value != value )
            continue;
        if ( !found )
        {
            minValue = maxValue = value;
            found = true;
            continue;
        }
        if ( value < minValue )
            minValue = value;
        if ( maxValue < value )
            maxValue = value;
    }
}


/** The NaN cells of a float column, nulls not counted */
template< typename T >
UInt64
CountNaN( const T* values, const CDataTableColumnBatch& column )
{
    UInt64 count = 0;
    const UInt32 rowCount = column.GetRowCount();
    for ( UInt32 r=0; r<rowCount; ++r )
        if ( column.IsValid( r ) && values[ r ] != values[ r ] )
            ++count;
    return count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Int32
CDataTableMerge::CompareCells( const CDataTableColumnBatch& left, UInt32 leftRow, const CDataTableColumnBatch& right, UInt32 rightRow )
{GUCEF_TRACE;

    const bool leftValid = left.IsValid( leftRow );
    const bool rightValid = right.IsValid( rightRow );
    if ( !leftValid || !rightValid )
        return leftValid == rightValid ? 0 : ( leftValid ? -1 : 1 );

    switch ( left.GetStorage() )
    {
        case DATATABLE_STORAGE_BITS:
        {
            const bool a = 0 != ( left.GetBoolBits()[ leftRow >> 3 ] & ( 1u << ( leftRow & 7u ) ) );
            const bool b = 0 != ( right.GetBoolBits()[ rightRow >> 3 ] & ( 1u << ( rightRow & 7u ) ) );
            return a == b ? 0 : ( a ? 1 : -1 );
        }
        case DATATABLE_STORAGE_VARIABLE:
        case DATATABLE_STORAGE_DICTIONARY:
        {
            UInt32 aLength = 0;
            UInt32 bLength = 0;
            const char* a = left.GetVariableValue( leftRow, aLength );
            const char* b = right.GetVariableValue( rightRow, bLength );
            const UInt32 common = aLength < bLength ? aLength : bLength;
            const int order = common > 0 ? memcmp( a, b, common ) : 0;
            if ( 0 != order )
                return order < 0 ? -1 : 1;
            return CompareScalars( aLength, bLength );
        }
        case DATATABLE_STORAGE_FIXED:
        {
            switch ( left.GetLogicalType() )
            {
                case DATATABLE_TYPE_INT8:       { return CompareScalars( left.GetInt8Data()[ leftRow ], right.GetInt8Data()[ rightRow ] ); }
                case DATATABLE_TYPE_INT16:      { return CompareScalars( left.GetInt16Data()[ leftRow ], right.GetInt16Data()[ rightRow ] ); }
                case DATATABLE_TYPE_INT32:
                case DATATABLE_TYPE_DATE:       { return CompareScalars( left.GetInt32Data()[ leftRow ], right.GetInt32Data()[ rightRow ] ); }
                case DATATABLE_TYPE_INT64:
                case DATATABLE_TYPE_DECIMAL:
                case DATATABLE_TYPE_TIMESTAMP:  { return CompareScalars( left.GetInt64Data()[ leftRow ], right.GetInt64Data()[ rightRow ] ); }
                case DATATABLE_TYPE_UINT8:      { return CompareScalars( left.GetUInt8Data()[ leftRow ], right.GetUInt8Data()[ rightRow ] ); }
                case DATATABLE_TYPE_UINT16:     { return CompareScalars( left.GetUInt16Data()[ leftRow ], right.GetUInt16Data()[ rightRow ] ); }
                case DATATABLE_TYPE_UINT32:     { return CompareScalars( left.GetUInt32Data()[ leftRow ], right.GetUInt32Data()[ rightRow ] ); }
                case DATATABLE_TYPE_UINT64:     { return CompareScalars( left.GetUInt64Data()[ leftRow ], right.GetUInt64Data()[ rightRow ] ); }
                case DATATABLE_TYPE_FLOAT32:    { return CompareFloats( left.GetFloat32Data()[ leftRow ], right.GetFloat32Data()[ rightRow ] ); }
                case DATATABLE_TYPE_FLOAT64:    { return CompareFloats( left.GetFloat64Data()[ leftRow ], right.GetFloat64Data()[ rightRow ] ); }
                default:                        { return 0; }
            }
        }
        default:
        {
            return 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableMerge::Normalize( const CDataTableSchemaPtr& schema    ,
                            const CDataTableBatch& rows          ,
                            CDataTableBufferPool& pool           ,
                            CDataTableBatchPtr& normalized       ,
                            CString& error                       )
{GUCEF_TRACE;

    normalized.Reset();
    if ( schema.IsNULL() )
    {
        error = "No schema to bring the rows to";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    // per schema column the column of the batch, matched by name when the batch has a schema
    const CDataTableSchema& target = *schema;
    const UInt32 columnCount = target.GetColumnCount();
    const CDataTableSchemaPtr& rowSchema = rows.GetSchema();
    GUCEF::vector< UInt32 > sourceColumn( columnCount, GUCEF_UINT32MAX );
    bool sameLayout = GUCEF_NULL == rows.GetSelection() && rows.GetColumnCount() == columnCount;
    for ( UInt32 c=0; c<rows.GetColumnCount(); ++c )
    {
        UInt32 schemaColumn = GUCEF_UINT32MAX;
        const UInt32 rowSchemaColumn = rows.GetSchemaColumnIndex( c );
        if ( !rowSchema.IsNULL() && rowSchemaColumn < rowSchema->GetColumnCount() )
        {
            const Int32 found = target.FindColumn( rowSchema->GetColumn( rowSchemaColumn ).name );
            if ( found < 0 )
            {
                error = "Column \"" + rowSchema->GetColumn( rowSchemaColumn ).name + "\" of the rows is not a column of the table";
                return DATATABLE_STATUS_TYPE_MISMATCH;
            }
            schemaColumn = (UInt32) found;
        }
        else
        {
            schemaColumn = rowSchemaColumn;
        }
        if ( schemaColumn >= columnCount )
        {
            error = "A column of the rows has no column of the table";
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        const CDataTableColumnDef& def = target.GetColumn( schemaColumn );
        const CDataTableColumnBatch& column = rows.GetColumn( c );
        if ( column.GetLogicalType() != def.type ||
             ( DATATABLE_TYPE_DECIMAL == def.type && ( column.GetDecimalScale() != def.decimalScale || column.GetDecimalPrecision() > def.decimalPrecision ) ) ||
             ( DATATABLE_TYPE_TIMESTAMP == def.type && column.GetTimeUnit() != def.timeUnit ) )
        {
            error = "Column \"" + def.name + "\" of the rows is not of the table type " + def.GetTypeText();
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        if ( !def.nullable && column.HasNulls() )
        {
            error = "Column \"" + def.name + "\" holds nulls but is not nullable";
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        if ( GUCEF_UINT32MAX != sourceColumn[ schemaColumn ] )
        {
            error = "Column \"" + def.name + "\" is given twice";
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        sourceColumn[ schemaColumn ] = c;
        if ( schemaColumn != c || column.GetRowCount() != rows.GetRowCount() )
            sameLayout = false;
    }
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        if ( GUCEF_UINT32MAX == sourceColumn[ c ] && !target.GetColumn( c ).nullable )
        {
            error = "Column \"" + target.GetColumn( c ).name + "\" is not nullable and the rows lack it";
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
    }

    if ( sameLayout )
    {
        // the columns are shared, nothing is copied
        TDataTableStatus status = pool.AcquireBatch( normalized );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        CDataTableBatch* writable = normalized.GetWritable();
        if ( !writable->SetColumnCount( columnCount ) )
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        writable->SetSchema( schema );
        writable->SetRowCount( rows.GetRowCount() );
        writable->SetOrigin( rows.GetOrigin() );
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            writable->GetMutableColumn( c ) = rows.GetColumn( c );
            writable->SetSchemaColumnIndex( c, c );
        }
        return DATATABLE_STATUS_OK;
    }

    CDataTableBatchBuilder builder( schema, pool );
    GUCEF::vector< CVariant > cells( columnCount );
    const UInt32* selection = rows.GetSelection();
    const UInt32 selected = rows.GetSelectedRowCount();
    for ( UInt32 i=0; i<selected; ++i )
    {
        const UInt32 row = GUCEF_NULL == selection ? i : selection[ i ];
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            if ( GUCEF_UINT32MAX == sourceColumn[ c ] )
                cells[ c ].Clear();
            else
                rows.GetColumn( sourceColumn[ c ] ).GetValue( row, cells[ c ], true );
        }
        if ( !builder.AppendRow( columnCount > 0 ? &cells[ 0 ] : GUCEF_NULL, columnCount ) )
        {
            error = "Row " + ToString( row ) + " does not fit the table: " + DataTableStatusToString( builder.GetLastStatus() );
            return builder.GetLastStatus();
        }
    }
    return builder.Build( normalized, error );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableMerge::Merge( const CDataTableSchemaPtr& schema    ,
                        const TBatchVector& existing         ,
                        const TBatchVector& incoming         ,
                        TDataTableWriteMode mode             ,
                        UInt32 rowGroupRowCount              ,
                        CDataTableBufferPool& pool           ,
                        TBatchVector& result                 ,
                        UInt64& rowsReplaced                 ,
                        CString& error                       )
{GUCEF_TRACE;

    result.clear();
    rowsReplaced = 0;
    if ( schema.IsNULL() )
    {
        error = "No schema to merge the rows with";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    const CDataTableSchema::TColumnIndexVector& primaryKey = schema->GetPrimaryKey();
    if ( DATATABLE_WRITE_UPSERT == mode && primaryKey.empty() )
    {
        error = "An upsert needs a primary key, the table has none";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    // all batches in one list: the existing ones first, then the incoming ones
    TBatchVector all( existing );
    all.insert( all.end(), incoming.begin(), incoming.end() );
    const UInt32 incomingOffset = (UInt32) existing.size();
    const CRowLess keyLess( all, primaryKey );

    TRowRefVector rows;
    TRowRefVector incomingRows;
    AppendAllRows( incoming, incomingOffset, incomingRows );

    switch ( mode )
    {
        case DATATABLE_WRITE_REPLACE_PARTITIONS:
        {
            rows = incomingRows;
            break;
        }
        case DATATABLE_WRITE_INSERT:
        {
            TRowRefVector existingRows;
            AppendAllRows( existing, 0, existingRows );
            if ( !primaryKey.empty() )
            {
                // a key given twice, or one already present, is a conflict
                TRowRefVector sorted( incomingRows );
                std::stable_sort( sorted.begin(), sorted.end(), keyLess );
                for ( UInt32 i=1; i<(UInt32) sorted.size(); ++i )
                {
                    if ( 0 == keyLess.Compare( sorted[ i - 1 ], sorted[ i ] ) )
                    {
                        error = "The rows hold a primary key twice";
                        return DATATABLE_STATUS_CONFLICT;
                    }
                }
                for ( UInt32 i=0; i<(UInt32) existingRows.size(); ++i )
                {
                    if ( std::binary_search( sorted.begin(), sorted.end(), existingRows[ i ], keyLess ) )
                    {
                        error = "A primary key of the rows is already in the partition";
                        return DATATABLE_STATUS_CONFLICT;
                    }
                }
            }
            rows = existingRows;
            rows.insert( rows.end(), incomingRows.begin(), incomingRows.end() );
            break;
        }
        case DATATABLE_WRITE_UPSERT:
        {
            UInt64 duplicates = 0;
            KeepLastPerKey( keyLess, incomingRows, duplicates );
            TRowRefVector existingRows;
            AppendAllRows( existing, 0, existingRows );
            for ( UInt32 i=0; i<(UInt32) existingRows.size(); ++i )
            {
                if ( std::binary_search( incomingRows.begin(), incomingRows.end(), existingRows[ i ], keyLess ) )
                {
                    ++rowsReplaced;
                    continue;
                }
                rows.push_back( existingRows[ i ] );
            }
            rows.insert( rows.end(), incomingRows.begin(), incomingRows.end() );
            break;
        }
        default:
        {
            error = "Unknown write mode";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
    }

    const CDataTableSchema::TColumnIndexVector& sortKey = schema->GetSortKey();
    if ( !sortKey.empty() )
        std::stable_sort( rows.begin(), rows.end(), CRowLess( all, sortKey ) );
    return Materialize( schema, all, rows, rowGroupRowCount, pool, result, error );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableMerge::Canonicalize( const CDataTableSchemaPtr& schema ,
                               const TBatchVector& batches       ,
                               UInt32 rowGroupRowCount           ,
                               CDataTableBufferPool& pool        ,
                               TBatchVector& result              ,
                               UInt64& rowsRemoved               ,
                               CString& error                    )
{GUCEF_TRACE;

    result.clear();
    rowsRemoved = 0;
    if ( schema.IsNULL() )
    {
        error = "No schema to compact the rows with";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    TRowRefVector rows;
    AppendAllRows( batches, 0, rows );
    const CDataTableSchema::TColumnIndexVector& primaryKey = schema->GetPrimaryKey();
    if ( !primaryKey.empty() )
    {
        const CRowLess keyLess( batches, primaryKey );
        KeepLastPerKey( keyLess, rows, rowsRemoved );
        // back to the order the rows had, for a stable sort on the sort key
        std::sort( rows.begin(), rows.end(), SPositionLess() );
    }
    const CDataTableSchema::TColumnIndexVector& sortKey = schema->GetSortKey();
    if ( !sortKey.empty() )
        std::stable_sort( rows.begin(), rows.end(), CRowLess( batches, sortKey ) );
    return Materialize( schema, batches, rows, rowGroupRowCount, pool, result, error );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableMerge::RemoveMatchingRows( const CDataTableResolvedQuery& query ,
                                     const TBatchVector& batches          ,
                                     CDataTableBufferPool& pool           ,
                                     TBatchVector& result                 ,
                                     UInt64& rowsRemoved                  ,
                                     CString& error                       )
{GUCEF_TRACE;

    result.clear();
    rowsRemoved = 0;
    if ( query.schema.IsNULL() )
    {
        error = "The query is not resolved";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    const UInt32 columnCount = query.schema->GetColumnCount();
    CDataTableSchema::TColumnIndexVector allColumns;
    for ( UInt32 c=0; c<columnCount; ++c )
        allColumns.push_back( c );
    CDataTableFilter filter;
    TDataTableStatus status = filter.Prepare( query, allColumns, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;

    for ( UInt32 b=0; b<(UInt32) batches.size(); ++b )
    {
        const CDataTableBatch& stored = *batches[ b ];
        const UInt32 rowCount = stored.GetRowCount();

        // the filter selects on a view: the stored batch is shared and stays as it is
        CDataTableBatchPtr view;
        status = pool.AcquireBatch( view );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        CDataTableBatch* writable = view.GetWritable();
        if ( !writable->SetColumnCount( columnCount ) )
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        writable->SetSchema( stored.GetSchema() );
        writable->SetRowCount( rowCount );
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            writable->GetMutableColumn( c ) = stored.GetColumn( c );
            writable->SetSchemaColumnIndex( c, c );
        }
        UInt32 matching = 0;
        status = filter.Apply( *writable, pool, matching );
        if ( DATATABLE_STATUS_OK != status )
        {
            error = "Filtering the rows failed: " + CString( DataTableStatusToString( status ) );
            return status;
        }
        if ( 0 == matching )
        {
            result.push_back( batches[ b ] );
            continue;
        }
        rowsRemoved += matching;
        if ( matching == rowCount )
            continue;

        // the rows that stay: those not in the ascending selection
        TBatchVector single;
        single.push_back( batches[ b ] );
        TRowRefVector kept;
        const UInt32* selection = writable->GetSelection();
        UInt32 next = 0;
        for ( UInt32 r=0; r<rowCount; ++r )
        {
            if ( next < matching && GUCEF_NULL != selection && selection[ next ] == r )
            {
                ++next;
                continue;
            }
            SRowRef ref;
            ref.batch = 0;
            ref.row = r;
            kept.push_back( ref );
        }
        status = Materialize( stored.GetSchema(), single, kept, rowCount, pool, result, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDataTableMerge::ComputeStats( const CDataTableBatch& batch, CDataTableRowGroupInfo& info )
{GUCEF_TRACE;

    info.Clear();
    const UInt32 rowCount = batch.GetRowCount();
    const UInt32 columnCount = batch.GetColumnCount();
    info.rowCount = rowCount;
    info.columnStats.resize( columnCount );
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        const CDataTableColumnBatch& column = batch.GetColumn( c );
        CDataTableColumnStats& stats = info.columnStats[ c ];
        stats.Clear();
        stats.hasNullCount = true;
        stats.nullCount = column.GetNullCount();
        stats.valueCount = rowCount;
        stats.minExact = true;
        stats.maxExact = true;
        bool found = false;

        switch ( column.GetStorage() )
        {
            case DATATABLE_STORAGE_BITS:
            {
                bool seenFalse = false;
                bool seenTrue = false;
                const UInt8* bits = column.GetBoolBits();
                for ( UInt32 r=0; r<rowCount; ++r )
                {
                    if ( !column.IsValid( r ) )
                        continue;
                    if ( 0 != ( bits[ r >> 3 ] & ( 1u << ( r & 7u ) ) ) )
                        seenTrue = true;
                    else
                        seenFalse = true;
                }
                found = seenFalse || seenTrue;
                if ( found )
                {
                    stats.minValue = !seenFalse;
                    stats.maxValue = seenTrue;
                }
                break;
            }
            case DATATABLE_STORAGE_FIXED:
            {
                switch ( column.GetLogicalType() )
                {
                    case DATATABLE_TYPE_INT8:   { Int8 lo = 0, hi = 0; FixedMinMax( column.GetInt8Data(), column, found, lo, hi ); stats.minValue = (Int64) lo; stats.maxValue = (Int64) hi; break; }
                    case DATATABLE_TYPE_INT16:  { Int16 lo = 0, hi = 0; FixedMinMax( column.GetInt16Data(), column, found, lo, hi ); stats.minValue = (Int64) lo; stats.maxValue = (Int64) hi; break; }
                    case DATATABLE_TYPE_INT32:  { Int32 lo = 0, hi = 0; FixedMinMax( column.GetInt32Data(), column, found, lo, hi ); stats.minValue = (Int64) lo; stats.maxValue = (Int64) hi; break; }
                    case DATATABLE_TYPE_DATE:   { Int32 lo = 0, hi = 0; FixedMinMax( column.GetInt32Data(), column, found, lo, hi ); stats.minValue = CDataTableValues::Date( lo ); stats.maxValue = CDataTableValues::Date( hi ); break; }
                    case DATATABLE_TYPE_INT64:
                    case DATATABLE_TYPE_DECIMAL:
                    case DATATABLE_TYPE_TIMESTAMP: { Int64 lo = 0, hi = 0; FixedMinMax( column.GetInt64Data(), column, found, lo, hi ); stats.minValue = lo; stats.maxValue = hi; break; }
                    case DATATABLE_TYPE_UINT8:  { UInt8 lo = 0, hi = 0; FixedMinMax( column.GetUInt8Data(), column, found, lo, hi ); stats.minValue = (UInt64) lo; stats.maxValue = (UInt64) hi; break; }
                    case DATATABLE_TYPE_UINT16: { UInt16 lo = 0, hi = 0; FixedMinMax( column.GetUInt16Data(), column, found, lo, hi ); stats.minValue = (UInt64) lo; stats.maxValue = (UInt64) hi; break; }
                    case DATATABLE_TYPE_UINT32: { UInt32 lo = 0, hi = 0; FixedMinMax( column.GetUInt32Data(), column, found, lo, hi ); stats.minValue = (UInt64) lo; stats.maxValue = (UInt64) hi; break; }
                    case DATATABLE_TYPE_UINT64: { UInt64 lo = 0, hi = 0; FixedMinMax( column.GetUInt64Data(), column, found, lo, hi ); stats.minValue = lo; stats.maxValue = hi; break; }
                    case DATATABLE_TYPE_FLOAT32:
                    {
                        Float32 lo = 0, hi = 0;
                        FixedMinMax( column.GetFloat32Data(), column, found, lo, hi );
                        stats.minValue = (Float64) lo;
                        stats.maxValue = (Float64) hi;
                        stats.hasNanCount = true;
                        stats.nanCount = CountNaN( column.GetFloat32Data(), column );
                        break;
                    }
                    case DATATABLE_TYPE_FLOAT64:
                    {
                        Float64 lo = 0, hi = 0;
                        FixedMinMax( column.GetFloat64Data(), column, found, lo, hi );
                        stats.minValue = lo;
                        stats.maxValue = hi;
                        stats.hasNanCount = true;
                        stats.nanCount = CountNaN( column.GetFloat64Data(), column );
                        break;
                    }
                    default:                    { break; }
                }
                break;
            }
            case DATATABLE_STORAGE_VARIABLE:
            case DATATABLE_STORAGE_DICTIONARY:
            {
                const char* minBytes = GUCEF_NULL;
                const char* maxBytes = GUCEF_NULL;
                UInt32 minLength = 0;
                UInt32 maxLength = 0;
                for ( UInt32 r=0; r<rowCount; ++r )
                {
                    if ( !column.IsValid( r ) )
                        continue;
                    UInt32 length = 0;
                    const char* bytes = column.GetVariableValue( r, length );
                    if ( !found )
                    {
                        minBytes = maxBytes = bytes;
                        minLength = maxLength = length;
                        found = true;
                        continue;
                    }
                    const UInt32 minCommon = length < minLength ? length : minLength;
                    const int toMin = minCommon > 0 ? memcmp( bytes, minBytes, minCommon ) : 0;
                    if ( toMin < 0 || ( 0 == toMin && length < minLength ) )
                    {
                        minBytes = bytes;
                        minLength = length;
                    }
                    const UInt32 maxCommon = length < maxLength ? length : maxLength;
                    const int toMax = maxCommon > 0 ? memcmp( bytes, maxBytes, maxCommon ) : 0;
                    if ( toMax > 0 || ( 0 == toMax && length > maxLength ) )
                    {
                        maxBytes = bytes;
                        maxLength = length;
                    }
                }
                if ( found )
                {
                    // owned copies: statistics outlive nothing but are kept apart from the buffers
                    if ( DATATABLE_TYPE_UTF8 == column.GetLogicalType() )
                    {
                        stats.minValue.Set( minBytes, minLength + 1, GUCEF_DATATYPE_UTF8_STRING, false );
                        stats.maxValue.Set( maxBytes, maxLength + 1, GUCEF_DATATYPE_UTF8_STRING, false );
                    }
                    else
                    {
                        stats.minValue.Set( minBytes, minLength, GUCEF_DATATYPE_BINARY_BLOB, false );
                        stats.maxValue.Set( maxBytes, maxLength, GUCEF_DATATYPE_BINARY_BLOB, false );
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
        stats.hasMinMax = found;
        if ( !found )
        {
            stats.minValue.Clear();
            stats.maxValue.Clear();
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
