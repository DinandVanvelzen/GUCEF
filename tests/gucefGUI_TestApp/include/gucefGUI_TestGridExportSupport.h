/*
 *  gucefGUI_TestApp: test application for the gucefGUI module
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

#ifndef GUCEF_GUI_TESTGRIDEXPORTSUPPORT_H
#define GUCEF_GUI_TESTGRIDEXPORTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE.h"
#include "CDynamicBufferAccess.h"
#include "gucefGUI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Support of the tests of the column chooser, the grid preferences and the CSV export: a model of rows of values
 *  that filters and sorts, a model of generated rows that counts how it is read, outputs in memory, a preference store
 *  in memory and export targets that fail on purpose.
 */
namespace GridExportTest {

using namespace GUCEF;

/** A variant that is null */
inline CORE::CVariant
NullValue( void )
{
    CORE::CVariant value;
    value.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL, false );
    return value;
}

/*-------------------------------------------------------------------------*/

/** The text of a cell as the model of a grid gives it for copying */
inline CORE::CString
CellTextOf( GUI::CGridView* grid, UInt32 row, UInt32 column )
{
    if ( GUCEF_NULL == grid || grid->GetModel().IsNULL() )
        return "<no model>";
    return grid->GetModel()->GetCellText( row, column );
}

/*-------------------------------------------------------------------------*/

/** Rows of values with columns given by the test; filters on a column having a value and sorts by one column, raising ModelResetEvent */
class CTextTableModel : public GUI::CITableViewModel
{
    public:

    bool sortable;
    Int32 sortedColumn;
    bool sortedAscending;
    GUCEF::vector< GUI::CTableViewColumn > columns;
    GUCEF::vector< GUCEF::vector< CORE::CVariant > > data;

    CTextTableModel( void )
        : GUI::CITableViewModel(), sortable( false ), sortedColumn( -1 ), sortedAscending( true ), columns(), data(), m_view(), m_filterColumn( -1 )
    {
    }

    GUI::CTableViewColumn& AddColumn( const CORE::CString& title, CORE::TDataTableLogicalType type, const CORE::CString& name )
    {
        columns.push_back( GUI::CTableViewColumn( title, type, 0, true ) );
        columns.back().name = name;
        NotifyObservers( ColumnsChangedEvent );
        return columns.back();
    }

    void AddRow( const GUCEF::vector< CORE::CVariant >& values )
    {
        data.push_back( values );
        Rebuild();
    }

    void AddTextRow( const char* first, const char* second, const CORE::CVariant& third )
    {
        GUCEF::vector< CORE::CVariant > values;
        values.push_back( CORE::CVariant( CORE::CString( first ) ) );
        values.push_back( CORE::CVariant( CORE::CString( second ) ) );
        values.push_back( third );
        AddRow( values );
    }

    /** Only the rows that have a value in the column; -1 for every row */
    void SetFilter( Int32 column, bool notNull )
    {
        m_filterColumn = notNull ? column : -1;
        Rebuild();
        NotifyObservers( ModelResetEvent );
    }

    void ClearSort( void )
    {
        sortedColumn = -1;
        Rebuild();
        NotifyObservers( ModelResetEvent );
    }

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) m_view.size(); }

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) columns.size(); }

    virtual const GUI::CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUI::CTableViewColumn none;
        return column < columns.size() ? columns[ column ] : none;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= m_view.size() || column >= data[ m_view[ row ] ].size() )
            return false;
        value = data[ m_view[ row ] ][ column ];
        return true;
    }

    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= m_view.size() )
            return false;
        key = m_view[ row ];
        return true;
    }

    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 index = key.AsUInt32( GUCEF_UINT32MAX );
        for ( UInt32 r=0; r<(UInt32)m_view.size(); ++r )
            if ( m_view[ r ] == index )
            {
                row = r;
                return true;
            }
        return false;
    }

    virtual bool IsSortSupported( void ) const GUCEF_VIRTUAL_OVERRIDE { return sortable; }

    virtual bool SortByColumn( UInt32 column, bool ascending ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !sortable || column >= columns.size() )
            return false;
        sortedColumn = (Int32) column;
        sortedAscending = ascending;
        Rebuild();
        NotifyObservers( ModelResetEvent );
        return true;
    }

    private:

    static bool IsMissing( const CORE::CVariant& value ) { return value.IsNULL() || value.IsNIL(); }

    /** -1, 0, 1; nulls last whatever the direction */
    Int32 Compare( UInt32 a, UInt32 b ) const
    {
        const CORE::CVariant& x = data[ a ][ sortedColumn ];
        const CORE::CVariant& y = data[ b ][ sortedColumn ];
        if ( IsMissing( x ) || IsMissing( y ) )
            return IsMissing( x ) == IsMissing( y ) ? 0 : ( IsMissing( x ) ? 2 : -2 );
        Int32 order = 0;
        if ( x.IsInteger() && y.IsInteger() )
            order = x.AsInt64() < y.AsInt64() ? -1 : ( y.AsInt64() < x.AsInt64() ? 1 : 0 );
        else
            order = x.AsString() < y.AsString() ? -1 : ( y.AsString() < x.AsString() ? 1 : 0 );
        return sortedAscending ? order : -order;
    }

    void Rebuild( void )
    {
        m_view.clear();
        for ( UInt32 i=0; i<(UInt32)data.size(); ++i )
            if ( m_filterColumn < 0 || ( m_filterColumn < (Int32) data[ i ].size() && !IsMissing( data[ i ][ m_filterColumn ] ) ) )
                m_view.push_back( i );
        if ( sortedColumn < 0 )
            return;
        for ( UInt32 i=1; i<(UInt32)m_view.size(); ++i )      // stable insertion sort, the rows of a test are few
        {
            const UInt32 current = m_view[ i ];
            UInt32 j = i;
            while ( j > 0 )
            {
                const Int32 order = Compare( m_view[ j-1 ], current );
                if ( order <= 0 )
                    break;
                m_view[ j ] = m_view[ j-1 ];
                --j;
            }
            m_view[ j ] = current;
        }
    }

    GUCEF::vector< UInt32 > m_view;
    Int32 m_filterColumn;
};

/*-------------------------------------------------------------------------*/

/**
 *  Rows generated from their number, nothing stored: Id, Name, Price, Qty, Flag, Note (with a comma in every seventh row).
 *  With counting on it records how it is read: the rows prepared, whether rows were read in order, how many.
 *  A read changes nothing else, so with counting off it can be read from another thread.
 */
class CGeneratedModel : public GUI::CITableViewModel
{
    public:

    bool counting;
    UInt32 prepares;
    UInt32 largestPrepare;
    mutable UInt32 rowsRead;
    mutable bool inOrder;

    explicit CGeneratedModel( UInt32 rows )
        : GUI::CITableViewModel(), counting( true ), prepares( 0 ), largestPrepare( 0 ), rowsRead( 0 ), inOrder( true ), m_rows( rows ), m_offset( 0 ), m_lastRow( -1 )
    {
        static const char* titles[] = { "Id", "Name", "Price", "Qty", "Flag", "Note" };
        static const CORE::TDataTableLogicalType types[] = { CORE::DATATABLE_TYPE_INT64, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_FLOAT64,
                                                             CORE::DATATABLE_TYPE_INT32, CORE::DATATABLE_TYPE_BOOL, CORE::DATATABLE_TYPE_UTF8 };
        for ( UInt32 c=0; c<6; ++c )
            m_columns.push_back( GUI::CTableViewColumn( titles[ c ], types[ c ], 0, true ) );
        m_columns[ 2 ].decimals = 2;
        m_columns[ 2 ].thousandsSeparator = true;
    }

    void ResetCounters( void )
    {
        prepares = 0;
        largestPrepare = 0;
        rowsRead = 0;
        inOrder = true;
        m_lastRow = -1;
    }

    /** Rows at the front: the rows there were move down */
    void InsertRows( UInt32 first, UInt32 count )
    {
        m_rows += count;
        ++m_offset;
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( first, count, 0, (UInt32) m_columns.size() ) );
        NotifyObservers( RowsInsertedEvent, &range );
    }

    void RemoveRows( UInt32 first, UInt32 count )
    {
        m_rows -= count;
        --m_offset;
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( first, count, 0, (UInt32) m_columns.size() ) );
        NotifyObservers( RowsRemovedEvent, &range );
    }

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_rows; }

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) m_columns.size(); }

    virtual const GUI::CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUI::CTableViewColumn none;
        return column < m_columns.size() ? m_columns[ column ] : none;
    }

    virtual void PrepareRange( const GUI::CTableViewRange& range ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !counting )
            return;
        ++prepares;
        if ( range.rowCount > largestPrepare )
            largestPrepare = range.rowCount;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= m_rows || column >= m_columns.size() )
            return false;
        if ( counting && (Int32) row != m_lastRow )
        {
            if ( (Int32) row < m_lastRow )
                inOrder = false;
            ++rowsRead;
            m_lastRow = (Int32) row;
        }
        const UInt32 id = row + m_offset;
        switch ( column )
        {
            case 0: value = CORE::CVariant( (Int64) id ); break;
            case 1: value = CORE::CVariant( CORE::CString( "row " ) + CORE::UInt32ToString( id ) ); break;
            case 2: value = CORE::CVariant( (Float64) id * 1.25 ); break;
            case 3: value = CORE::CVariant( (Int32) ( id % 1000 ) ); break;
            case 4: value = CORE::CVariant( 0 == id % 2 ); break;
            default: value = CORE::CVariant( 0 == id % 7 ? CORE::CString( "seven, a multiple" ) : CORE::CString( "plain" ) ); break;
        }
        return true;
    }

    private:

    GUCEF::vector< GUI::CTableViewColumn > m_columns;
    UInt32 m_rows;
    UInt32 m_offset;
    mutable Int32 m_lastRow;
};

/*-------------------------------------------------------------------------*/

/** An output in memory and its text */
class CMemoryOutput
{
    public:

    CMemoryOutput( void )
        : m_buffer( 1, true )
        , m_access()
    {
        m_buffer.SetDataSize( 0 );
        CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( &m_buffer, false );
        m_access = access->CreateSharedPtr();
    }

    const CORE::IOAccessPtr& Access( void ) const { return m_access; }

    CORE::CString Text( void ) const
    {
        if ( 0 == m_buffer.GetDataSize() )
            return CORE::CString::Empty;
        return CORE::CString( static_cast< const char* >( m_buffer.GetConstBufferPtr() ), m_buffer.GetDataSize() );
    }

    private:

    CORE::CDynamicBuffer m_buffer;
    CORE::IOAccessPtr m_access;
};

/*-------------------------------------------------------------------------*/

/** An output that counts what is written and keeps nothing; with failing set every write is refused */
class CCountingOutput : public CORE::CIOAccess
{
    public:

    UInt64 bytes;
    UInt32 writes;
    UInt32 largestWrite;
    bool failing;

    explicit CCountingOutput( bool failWrites = false )
        : CORE::CIOAccess(), bytes( 0 ), writes( 0 ), largestWrite( 0 ), failing( failWrites ), m_open( true )
    {
    }

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE { m_open = true; }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE { m_open = false; }
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_open; }
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return bytes; }
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE { return -1; }
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return m_open; }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CCountingOutput( failing ); }

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( failing )
            return 0;
        const UInt32 size = esize * elements;
        bytes += size;
        ++writes;
        if ( size > largestWrite )
            largestWrite = size;
        return elements;
    }

    private:

    bool m_open;
};

/*-------------------------------------------------------------------------*/

/** Records by grid id in memory; counts saves; refuses saves on request */
class CMemoryPreferenceStore : public GUI::CIGridPreferenceStore
{
    public:

    GUCEF::map< CORE::CString, GUI::CGridPreferences > records;
    UInt32 saves;
    bool refuse;

    CMemoryPreferenceStore( void ) : GUI::CIGridPreferenceStore(), records(), saves( 0 ), refuse( false ) {}

    virtual bool Load( const CORE::CString& gridId, GUI::CGridPreferences& preferences ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::map< CORE::CString, GUI::CGridPreferences >::const_iterator found = records.find( gridId );
        if ( found == records.end() )
            return false;
        preferences = found->second;
        return true;
    }

    virtual bool Save( const CORE::CString& gridId, const GUI::CGridPreferences& preferences ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( refuse )
            return false;
        ++saves;
        records[ gridId ] = preferences;
        return true;
    }

    virtual bool Remove( const CORE::CString& gridId ) GUCEF_VIRTUAL_OVERRIDE
    {
        records.erase( gridId );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** A target of the file system of the computer that fails on purpose: at the open, while the file is written, or at the commit */
class CFailingTarget : public GUI::CNativeGridExportTarget
{
    public:

    enum EFailure
    {
        FAIL_OPEN = 0   ,
        FAIL_WRITE      ,
        FAIL_COMMIT
    };

    UInt32 abandoned;

    CFailingTarget( const CORE::CString& directory, EFailure failure )
        : GUI::CNativeGridExportTarget( "Failing", directory ), abandoned( 0 ), m_failure( failure )
    {
    }

    virtual bool OpenForWriting( const CORE::CString& path, CORE::IOAccessPtr& stream, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( FAIL_OPEN == m_failure )
        {
            error = "refused by the test";
            return false;
        }
        if ( FAIL_WRITE == m_failure )
        {
            stream = CORE::IOAccessPtr( GUCEF_NEW CCountingOutput( true ) );
            return true;
        }
        return GUI::CNativeGridExportTarget::OpenForWriting( path, stream, error );
    }

    virtual bool Commit( const CORE::CString& path, CORE::IOAccessPtr& stream, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( FAIL_COMMIT == m_failure )
        {
            GUI::CNativeGridExportTarget::Abandon( path, stream );
            error = "refused by the test at the commit";
            return false;
        }
        return GUI::CNativeGridExportTarget::Commit( path, stream, error );
    }

    virtual void Abandon( const CORE::CString& path, CORE::IOAccessPtr& stream ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++abandoned;
        if ( FAIL_WRITE == m_failure )
        {
            stream.Unlink();
            return;
        }
        GUI::CNativeGridExportTarget::Abandon( path, stream );
    }

    private:

    EFailure m_failure;
};

/*-------------------------------------------------------------------------*/

/** Keeps the report of the last CsvExportFinishedEvent of a grid */
class CReportProbe : public CORE::CObserver
{
    public:

    GUI::CGridCsvExportReport last;
    UInt32 count;

    explicit CReportProbe( GUI::CGridView* grid )
        : CORE::CObserver(), last(), count( 0 )
    {
        TEventCallback callback( this, &CReportProbe::OnFinished );
        SubscribeTo( grid, GUI::CGridView::CsvExportFinishedEvent, callback );
    }

    virtual ~CReportProbe() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    private:

    typedef CORE::CTEventHandlerFunctor< CReportProbe > TEventCallback;

    void OnFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++count;
        if ( GUCEF_NULL != eventData )
            last = static_cast< GUI::CGridView::TGridCsvExportReportEventData* >( eventData )->GetData();
    }
};

}; /* namespace GridExportTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TESTGRIDEXPORTSUPPORT_H ? */
