/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_GUI_TESTGRIDSUPPORT_H
#define GUCEF_GUI_TESTGRIDSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#include "gucefCORE.h"
#include "gucefCORE_CDataTableValues.h"
#include "gucefCORE_CDecimal.h"
#include "gucefINPUT.h"
#include "gucefGUI.h"
#include "DVCPPOSWRAP.h"
#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GridTest {

using namespace GUCEF;

/** A context of the null driver at the given resolution with the bitmap font "Test" (8 x 10) and the dark theme; NULL when the driver is missing */
inline GUI::TGuiContextPtr
MakeContext( Int32 width, Int32 height )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( width, height );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

inline GUI::CGUIContext*
ContextOf( GUI::TGuiContextPtr& contextPtr )
{
    return contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
}

inline GUI::CWidget*
MakeScreen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

/** A grid at a pixel position and size with the test font and a row height of 20 */
inline GUI::CGridView*
MakeGrid( GUI::CGUIContext* context, GUI::CWidget* parent, Int32 left, Int32 top, Int32 width, Int32 height )
{
    GUI::CGridView* grid = GUI::CWidget::As< GUI::CGridView >( context->CreateWidget( "GridView", "Grid", parent ) );
    if ( GUCEF_NULL == grid )
        return GUCEF_NULL;
    grid->SetPosition( GUI::CUVector( GUI::CUDim( 0.0f, left ), GUI::CUDim( 0.0f, top ) ) );
    grid->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, width ), GUI::CUDim( 0.0f, height ) ) );
    grid->SetProperty( "RowHeight", "20" );
    grid->SetProperty( "Font", "Test" );
    return grid;
}

/** The commands the painter makes for a widget */
inline GUI::CDrawSurface::TCommandVector
PaintedCommands( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    GUI::CDrawSurface::TCommandVector commands;
    context->FlushChanges();
    GUI::CWidgetPainter::Paint( *widget, *context, commands );
    return commands;
}

/** The first text command with the text, NULL when none */
inline const GUI::CDrawSurface::TCommand*
FindText( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& text )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && text == commands[ i ].text )
            return &commands[ i ];
    return GUCEF_NULL;
}

inline UInt32
CountText( const GUI::CDrawSurface::TCommandVector& commands )
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type )
            ++count;
    return count;
}

/** Whether a fill of exactly the rectangle is among the commands */
inline bool
HasFill( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect, GUI::CColour* colour = GUCEF_NULL )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_FILLRECT == command.type && command.rect.left == rect.left && command.rect.top == rect.top &&
             command.rect.width == rect.width && command.rect.height == rect.height )
        {
            if ( GUCEF_NULL != colour )
                *colour = command.colour;
            return true;
        }
    }
    return false;
}

inline Int32
TextWidth( GUI::CGUIContext* context, const CORE::CString& text )
{
    Int32 width = 0, height = 0;
    context->MeasureText( "Test", text, 0, width, height );
    return width;
}

/** The 64 bit FNV-1a hash of a byte run, continuing 'hash' */
inline UInt64
Fnv( UInt64 hash, const void* data, size_t size )
{
    const UInt8* bytes = static_cast< const UInt8* >( data );
    for ( size_t i=0; i<size; ++i )
    {
        hash ^= bytes[ i ];
        hash *= (UInt64) 0x100000001b3ULL;
    }
    return hash;
}

inline UInt64
FnvInt( UInt64 hash, Int64 value )
{
    return Fnv( hash, &value, sizeof( value ) );
}

inline UInt64
FnvText( UInt64 hash, const CORE::CString& text )
{
    return Fnv( hash, text.C_String(), text.ByteSize() );
}

/** A hash of every field of every command: two command lists with the same hash draw the same */
inline UInt64
HashCommands( const GUI::CDrawSurface::TCommandVector& commands )
{
    UInt64 hash = (UInt64) 0xcbf29ce484222325ULL;
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& c = commands[ i ];
        hash = FnvInt( hash, (Int64) c.type );
        hash = FnvInt( hash, c.rect.left ); hash = FnvInt( hash, c.rect.top ); hash = FnvInt( hash, c.rect.width ); hash = FnvInt( hash, c.rect.height );
        hash = FnvInt( hash, c.source.left ); hash = FnvInt( hash, c.source.top ); hash = FnvInt( hash, c.source.width ); hash = FnvInt( hash, c.source.height );
        hash = FnvInt( hash, c.x0 ); hash = FnvInt( hash, c.y0 ); hash = FnvInt( hash, c.x1 ); hash = FnvInt( hash, c.y1 );
        hash = FnvInt( hash, c.thickness );
        hash = FnvInt( hash, c.colour.r ); hash = FnvInt( hash, c.colour.g ); hash = FnvInt( hash, c.colour.b ); hash = FnvInt( hash, c.colour.a );
        hash = FnvText( hash, c.image ); hash = FnvText( hash, c.font ); hash = FnvText( hash, c.text );
        for ( size_t p=0; p<c.points.size(); ++p )
        {
            hash = FnvInt( hash, c.points[ p ].x );
            hash = FnvInt( hash, c.points[ p ].y );
        }
    }
    return hash;
}

/** Microseconds taken by 'ticks' of the precision timer */
inline double
Micros( UInt64 ticks )
{
    return (double) ticks * 1000000.0 / (double) MT::PrecisionTimerResolution();
}


/*
 *  Input
 */

inline void
PressKey( GUI::CGUIContext* context, INPUT::KeyCode key, UInt32 modifiers = 0 )
{
    context->FlushChanges();
    context->InjectKey( key, true, modifiers );
    context->InjectKey( key, false, modifiers );
}

/** Holds the modifier keys (INPUT::KEYMOD_*) while an action runs: press with PressModifiers, release with ReleaseModifiers */
inline void
PressModifiers( GUI::CGUIContext* context, UInt32 modifiers )
{
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
        context->InjectKey( INPUT::KEYCODE_LCONTROL, true, modifiers );
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) )
        context->InjectKey( INPUT::KEYCODE_LSHIFT, true, modifiers );
}

inline void
ReleaseModifiers( GUI::CGUIContext* context, UInt32 modifiers )
{
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
        context->InjectKey( INPUT::KEYCODE_LCONTROL, false, 0 );
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) )
        context->InjectKey( INPUT::KEYCODE_LSHIFT, false, 0 );
}

/** A key with the modifiers held */
inline void
Chord( GUI::CGUIContext* context, INPUT::KeyCode key, UInt32 modifiers )
{
    PressModifiers( context, modifiers );
    PressKey( context, key, modifiers );
    ReleaseModifiers( context, modifiers );
}

/** A click at a point of the context, with the modifiers held; the time moves on after it, so that the next click is a click of its own */
inline void
ClickAt( GUI::CGUIContext* context, Int32 x, Int32 y, UInt32 modifiers = 0, bool separate = true )
{
    context->FlushChanges();
    PressModifiers( context, modifiers );
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    ReleaseModifiers( context, modifiers );
    if ( separate )
        context->RenderFrame( 500 );
}

/** Press at one point, move to the other in steps, release */
inline void
DragFrom( GUI::CGUIContext* context, Int32 x0, Int32 y0, Int32 x1, Int32 y1, UInt32 modifiers = 0 )
{
    context->FlushChanges();
    PressModifiers( context, modifiers );
    context->InjectMouseMove( x0, y0 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    const Int32 steps = 8;
    for ( Int32 i=1; i<=steps; ++i )
        context->InjectMouseMove( x0 + ( x1 - x0 ) * i / steps, y0 + ( y1 - y0 ) * i / steps );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    ReleaseModifiers( context, modifiers );
    context->RenderFrame( 500 );
}

/** The middle of a cell in the coordinates of the context; false when the cell is not in view */
inline bool
CellPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column, Int32& x, Int32& y )
{
    context->FlushChanges();
    GUI::CRect cell;
    if ( !grid->GetCellRect( row, column, cell ) )
        return false;
    x = grid->GetRect().left + cell.left + cell.width / 2;
    y = grid->GetRect().top + cell.top + cell.height / 2;
    return true;
}

inline bool
ClickCell( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column, UInt32 modifiers = 0 )
{
    Int32 x = 0, y = 0;
    if ( !CellPoint( context, grid, row, column, x, y ) )
        return false;
    ClickAt( context, x, y, modifiers );
    return true;
}

/** The point in the middle of the row header cell of a row */
inline bool
RowHeaderPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32& x, Int32& y )
{
    context->FlushChanges();
    const Int32 top = grid->GetRowTop( row );
    if ( top < 0 || grid->GetRowHeaderWidth() <= 0 )
        return false;
    x = grid->GetRect().left + grid->GetRowHeaderWidth() / 2;
    y = grid->GetRect().top + top + grid->GetRowHeight() / 2;
    return true;
}

/** The point in the middle of the title of a column (of the data) */
inline bool
TitlePoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 column, Int32& x, Int32& y )
{
    context->FlushChanges();
    const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
    const Int32 position = layout.FindPosition( column );
    if ( position < 0 )
        return false;
    x = grid->GetRect().left + layout.GetScreenLeft( position ) + layout.GetWidth( position ) / 2;
    y = grid->GetRect().top + grid->GetRowHeight() / 2;
    return true;
}

/** The point on the right border of the title of a column */
inline bool
BorderPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 column, Int32& x, Int32& y )
{
    context->FlushChanges();
    const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
    const Int32 position = layout.FindPosition( column );
    if ( position < 0 )
        return false;
    x = grid->GetRect().left + layout.GetScreenLeft( position ) + layout.GetWidth( position ) - 1;
    y = grid->GetRect().top + grid->GetRowHeight() / 2;
    return true;
}

/** Counts the events of one notifier */
class CEventCount : public CORE::CObserver
{
    public:

    UInt32 count;

    CEventCount( CORE::CNotifier* notifier, const CORE::CEvent& eventId )
        : CORE::CObserver(), count( 0 ), m_eventId( eventId )
    {
        SubscribeTo( notifier, eventId );
    }

    virtual ~CEventCount() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == m_eventId )
            ++count;
    }

    private:

    CORE::CEvent m_eventId;
};

/*
 *  A model of generated rows for tests: the value of a cell follows from the id of its row, so it can be told from
 *  anywhere. It counts what the grid asks, and changes (insert, remove, re-order, change cells) on demand.
 */
class CTestModel : public GUI::CITableViewModel
{
    public:

    enum EColumn
    {
        COL_ID = 0  ,   /**< Int64, grouped digits */
        COL_PRICE   ,   /**< Float64, 2 decimals, "$" and grouped digits */
        COL_NAME    ,   /**< UTF8 "name <id>" */
        COL_FLAG    ,   /**< Bool: id is odd */
        COL_DATE    ,   /**< Date: 2019-04-14 plus id % 1000 days */
        COL_STAMP   ,   /**< Timestamp in milliseconds: a minute for every id */
        COL_MAYBE   ,   /**< Int32, null for every third id */
        COL_AMOUNT  ,   /**< Decimal(12,2): id.25 */
        COLUMNS
    };

    // what the grid asked for: counted by reads, which are const
    mutable UInt32 valueCalls;
    mutable UInt32 textCalls;
    mutable UInt32 contentCalls;
    UInt32 prepareCalls;
    mutable UInt32 spanCalls;
    GUI::CTableViewRange lastPrepared;
    mutable GUCEF::set< UInt32 > rowsFetched;
    bool footer;
    bool titles;
    GUCEF::set< UInt32 > emphasizedIds;             /**< the ids of the rows the model says stand out */

    explicit CTestModel( UInt32 rows, UInt32 columns = COLUMNS )
        : GUI::CITableViewModel(), valueCalls( 0 ), textCalls( 0 ), contentCalls( 0 ), prepareCalls( 0 ), spanCalls( 0 ), lastPrepared(), rowsFetched()
        , footer( false ), titles( false ), emphasizedIds(), m_ids(), m_columns(), m_nextId( 0 ), m_sortable( false )
    {
        for ( UInt32 r=0; r<rows; ++r )
            m_ids.push_back( m_nextId++ );
        for ( UInt32 c=0; c<columns && c<COLUMNS; ++c )
            m_columns.push_back( MakeColumn( c ) );
    }

    static GUI::CTableViewColumn MakeColumn( UInt32 column )
    {
        static const char* names[ COLUMNS ] = { "Id", "Price", "Name", "Flag", "Date", "Stamp", "Maybe", "Amount" };
        static const CORE::TDataTableLogicalType types[ COLUMNS ] = { CORE::DATATABLE_TYPE_INT64, CORE::DATATABLE_TYPE_FLOAT64, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_BOOL,
                                                                      CORE::DATATABLE_TYPE_DATE, CORE::DATATABLE_TYPE_TIMESTAMP, CORE::DATATABLE_TYPE_INT32, CORE::DATATABLE_TYPE_DECIMAL };
        GUI::CTableViewColumn definition( names[ column ], types[ column ], 0, true );
        if ( COL_ID == column )
            definition.thousandsSeparator = true;
        if ( COL_PRICE == column )
        {
            definition.thousandsSeparator = true;
            definition.decimals = 2;
            definition.prefix = "$";
        }
        if ( COL_MAYBE == column )
            definition.nullText = "";
        return definition;
    }

    static CORE::CVariant ValueOf( UInt32 id, UInt32 column )
    {
        switch ( column )
        {
            case COL_ID:      return CORE::CVariant( (Int64) id );
            case COL_PRICE:   return CORE::CVariant( (Float64) id * 0.25 );
            case COL_NAME:    return CORE::CVariant( CORE::CString( "name " ) + CORE::UInt32ToString( id ) );
            case COL_FLAG:    return CORE::CVariant( 1 == id % 2 );
            case COL_DATE:    return CORE::CDataTableValues::Date( (Int32) ( 18000 + id % 1000 ) );
            case COL_STAMP:   return CORE::CDataTableValues::Timestamp( (Int64) 1700000000000LL + (Int64) id * 60000, CORE::DATATABLE_TIMEUNIT_MILLISECONDS );
            case COL_MAYBE:
            {
                if ( 0 == id % 3 )
                {
                    CORE::CVariant empty;
                    empty.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL, false );
                    return empty;
                }
                return CORE::CVariant( (Int32) id );
            }
            case COL_AMOUNT:
            {
                CORE::CDecimal amount;
                CORE::CDecimal::FromUnscaled( (Int64) id * 100 + 25, 2, amount );
                return CORE::CDataTableValues::Decimal( amount );
            }
            default:          return CORE::CVariant();
        }
    }

    void ResetCounts( void )
    {
        valueCalls = 0;
        textCalls = 0;
        contentCalls = 0;
        prepareCalls = 0;
        spanCalls = 0;
        rowsFetched.clear();
    }

    void SetSortable( bool sortable ) { m_sortable = sortable; }

    UInt32 IdOf( UInt32 row ) const { return row < m_ids.size() ? m_ids[ row ] : GUCEF_UINT32MAX; }

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) m_ids.size(); }

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) m_columns.size(); }

    virtual const GUI::CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUI::CTableViewColumn none;
        return column < m_columns.size() ? m_columns[ column ] : none;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++valueCalls;
        if ( row >= m_ids.size() || column >= m_columns.size() )
            return false;
        rowsFetched.insert( row );
        value = ValueOf( m_ids[ row ], column );
        if ( COL_NAME == column )
        {
            GUCEF::map< UInt32, CORE::CString >::const_iterator named = m_names.find( m_ids[ row ] );
            if ( named != m_names.end() )
                value = CORE::CVariant( named->second );
        }
        return true;
    }

    virtual bool GetCellContent( UInt32 row, UInt32 column, GUI::SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++contentCalls;
        return GUI::CITableViewModel::GetCellContent( row, column, content );
    }

    virtual CORE::CString GetCellText( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++textCalls;
        return GUI::CITableViewModel::GetCellText( row, column );
    }

    virtual void PrepareRange( const GUI::CTableViewRange& visible ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++prepareCalls;
        lastPrepared = visible;
    }

    virtual bool GetColumnSpan( UInt32 column, UInt32 firstRow, UInt32 rowCount, CORE::CVariant::VariantVector& values ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++spanCalls;
        values.clear();
        if ( column >= m_columns.size() )
            return false;
        for ( UInt32 r=firstRow; r<firstRow+rowCount && r<m_ids.size(); ++r )
            values.push_back( ValueOf( m_ids[ r ], column ) );
        return true;
    }

    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= m_ids.size() )
            return false;
        key = (UInt32) m_ids[ row ];
        return true;
    }

    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 id = key.AsUInt32( GUCEF_UINT32MAX );
        for ( UInt32 r=0; r<m_ids.size(); ++r )
        {
            if ( m_ids[ r ] == id )
            {
                row = r;
                return true;
            }
        }
        return false;
    }

    virtual bool GetRowTitle( UInt32 row, CORE::CString& title ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !titles || row >= m_ids.size() )
            return false;
        title = CORE::CString( "R" ) + CORE::UInt32ToString( m_ids[ row ] );
        return true;
    }

    virtual bool IsRowEmphasized( UInt32 row ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return row < m_ids.size() && emphasizedIds.find( m_ids[ row ] ) != emphasizedIds.end();
    }

    virtual bool HasFooter( void ) const GUCEF_VIRTUAL_OVERRIDE { return footer; }

    virtual bool GetFooterValue( UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COL_ID == column )
        {
            value = (Int64) m_ids.size();
            return true;
        }
        if ( COL_NAME == column )
        {
            value = CORE::CString( "Total" );
            return true;
        }
        return false;
    }

    virtual bool IsSortSupported( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_sortable; }

    /** Orders the rows by the value of the column, ids as the tie breaker, and tells the grid */
    virtual bool SortByColumn( UInt32 column, bool ascending ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_sortable || column >= m_columns.size() )
            return false;
        for ( size_t i=1; i<m_ids.size(); ++i )
        {
            const UInt32 current = m_ids[ i ];
            size_t j = i;
            while ( j > 0 && Before( current, m_ids[ j - 1 ], column, ascending ) )
            {
                m_ids[ j ] = m_ids[ j - 1 ];
                --j;
            }
            m_ids[ j ] = current;
        }
        NotifyObservers( ModelResetEvent );
        return true;
    }

    /** Puts the rows in the reverse order and tells the grid */
    void Reverse( void )
    {
        std::reverse( m_ids.begin(), m_ids.end() );
        NotifyObservers( ModelResetEvent );
    }

    void InsertRows( UInt32 first, UInt32 count )
    {
        GUCEF::vector< UInt32 > added;
        for ( UInt32 i=0; i<count; ++i )
            added.push_back( m_nextId++ );
        m_ids.insert( m_ids.begin() + first, added.begin(), added.end() );
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( first, count, 0, (UInt32) m_columns.size() ) );
        NotifyObservers( RowsInsertedEvent, &range );
    }

    void RemoveRows( UInt32 first, UInt32 count )
    {
        m_ids.erase( m_ids.begin() + first, m_ids.begin() + first + count );
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( first, count, 0, (UInt32) m_columns.size() ) );
        NotifyObservers( RowsRemovedEvent, &range );
    }

    /** Changes the value the model gives for an id (the name column shows it), and tells the grid which cells changed */
    void ChangeName( UInt32 row, const CORE::CString& name )
    {
        m_names[ m_ids[ row ] ] = name;
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( row, 1, COL_NAME, 1 ) );
        NotifyObservers( CellsChangedEvent, &range );
    }

    /** Replaces every row by the rows with these ids and tells the grid: all rows removed, the new ones inserted; with 'batch' between BeginBatch() and EndBatch() */
    void Refill( const GUCEF::vector< UInt32 >& ids, bool batch )
    {
        if ( batch )
            BeginBatch();
        const UInt32 old = (UInt32) m_ids.size();
        if ( old > 0 )
        {
            m_ids.clear();
            GUI::TTableViewRangeEventData removed( GUI::CTableViewRange( 0, old, 0, (UInt32) m_columns.size() ) );
            NotifyObservers( RowsRemovedEvent, &removed );
        }
        m_ids = ids;
        for ( size_t i=0; i<ids.size(); ++i )
            if ( ids[ i ] >= m_nextId )
                m_nextId = ids[ i ] + 1;
        if ( !ids.empty() )
        {
            GUI::TTableViewRangeEventData inserted( GUI::CTableViewRange( 0, (UInt32) ids.size(), 0, (UInt32) m_columns.size() ) );
            NotifyObservers( RowsInsertedEvent, &inserted );
        }
        if ( batch )
            EndBatch();
    }

    /** The ids of the rows in the order they are shown */
    GUCEF::vector< UInt32 > GetIds( void ) const { return m_ids; }

    /** The model raises the event that these cells changed */
    void RaiseCells( UInt32 firstRow, UInt32 rowCount, UInt32 firstColumn, UInt32 columnCount )
    {
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( firstRow, rowCount, firstColumn, columnCount ) );
        NotifyObservers( CellsChangedEvent, &range );
    }

    /** The model raises the footer event for these columns */
    void RaiseFooterChanged( UInt32 firstColumn, UInt32 columnCount )
    {
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( 0, 0, firstColumn, columnCount ) );
        NotifyObservers( FooterChangedEvent, &range );
    }

    /** The definition of a column, to be changed by a test */
    GUI::CTableViewColumn& EditColumn( UInt32 column ) { return m_columns[ column ]; }

    void SetColumnTitle( UInt32 column, const CORE::CString& title )
    {
        m_columns[ column ].title = title;
        NotifyObservers( ColumnsChangedEvent );
    }

    void AddColumn( void )
    {
        if ( m_columns.size() < COLUMNS )
        {
            m_columns.push_back( MakeColumn( (UInt32) m_columns.size() ) );
            NotifyObservers( ColumnsChangedEvent );
        }
    }

    private:

    bool Before( UInt32 a, UInt32 b, UInt32 column, bool ascending ) const
    {
        const CORE::CVariant va = ValueOf( a, column );
        const CORE::CVariant vb = ValueOf( b, column );
        const bool aNull = va.IsNULL(), bNull = vb.IsNULL();
        if ( aNull || bNull )
            return !aNull && bNull;
        return ascending ? va < vb : vb < va;
    }

    GUCEF::vector< UInt32 > m_ids;
    GUCEF::vector< GUI::CTableViewColumn > m_columns;
    GUCEF::map< UInt32, CORE::CString > m_names;
    UInt32 m_nextId;
    bool m_sortable;
};

/** Notes what an observer of SelectionChangedEvent can read from the grid at the time of every notification */
class CSelectionProbe : public CORE::CObserver
{
    public:

    struct SSnapshot
    {
        Int32 selectedRow;
        GUCEF::vector< Int32 > selectedRows;
        Int32 selectedColumn;
        CORE::CString ranges;
    };

    GUCEF::vector< SSnapshot > snapshots;

    CSelectionProbe( GUI::CGridView* grid )
        : CORE::CObserver(), snapshots(), m_grid( grid )
    {
        SubscribeTo( grid, GUI::CGridView::SelectionChangedEvent );
    }

    virtual ~CSelectionProbe() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId != GUI::CGridView::SelectionChangedEvent )
            return;
        SSnapshot snapshot;
        snapshot.selectedRow = m_grid->GetIntProperty( "SelectedRow" );
        snapshot.selectedRows = m_grid->GetIntListProperty( "SelectedRows" );
        snapshot.selectedColumn = m_grid->GetIntProperty( "SelectedColumn" );
        snapshot.ranges = m_grid->GetProperty( "SelectedRanges" );
        snapshots.push_back( snapshot );
    }

    private:

    GUI::CGridView* m_grid;
};

/*
 *  A picture of a capture for a person to look at, written when GUCEF_TEST_SHOT_DIR names a directory:
 *  RGBA scan lines in stored deflate blocks
 */

inline UInt32
Crc32( const UInt8* data, size_t size, UInt32 crc )
{
    static UInt32 table[ 256 ];
    static bool tableMade = false;
    if ( !tableMade )
    {
        for ( UInt32 n=0; n<256; ++n )
        {
            UInt32 c = n;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? 0xedb88320u ^ ( c >> 1 ) : c >> 1;
            table[ n ] = c;
        }
        tableMade = true;
    }
    crc = ~crc;
    for ( size_t i=0; i<size; ++i )
        crc = table[ ( crc ^ data[ i ] ) & 0xff ] ^ ( crc >> 8 );
    return ~crc;
}

inline void
Put32( GUCEF::vector< UInt8 >& out, UInt32 value )
{
    out.push_back( (UInt8) ( value >> 24 ) );
    out.push_back( (UInt8) ( value >> 16 ) );
    out.push_back( (UInt8) ( value >> 8 ) );
    out.push_back( (UInt8) value );
}

inline void
PutChunk( GUCEF::vector< UInt8 >& out, const char* type, const GUCEF::vector< UInt8 >& data )
{
    Put32( out, (UInt32) data.size() );
    GUCEF::vector< UInt8 > body( type, type + 4 );
    body.insert( body.end(), data.begin(), data.end() );
    out.insert( out.end(), body.begin(), body.end() );
    Put32( out, Crc32( &body[ 0 ], body.size(), 0 ) );
}

/** Renders a frame and captures it; false when the driver cannot */
inline bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels )
{
    context->RenderFrame( 16 );
    return context->CaptureFrame( pixels ) && !pixels.IsNULL();
}

inline bool
WriteShot( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& name )
{
    CORE::CString directory = CORE::GetEnv( "GUCEF_TEST_SHOT_DIR" );
    if ( directory.IsNULLOrEmpty() || pixels.IsNULL() )
        return false;
    UInt32 width = pixels->GetWidthInPixels();
    UInt32 height = pixels->GetHeightInPixels();
    const UInt8* rgba = static_cast< const UInt8* >( pixels->GetDataPtr() );

    GUCEF::vector< UInt8 > raw;
    raw.reserve( (size_t) height * ( width * 4 + 1 ) );
    for ( UInt32 y=0; y<height; ++y )
    {
        raw.push_back( 0 );
        raw.insert( raw.end(), rgba + (size_t) y * width * 4, rgba + (size_t) ( y + 1 ) * width * 4 );
    }
    GUCEF::vector< UInt8 > zlib;
    zlib.push_back( 0x78 );
    zlib.push_back( 0x01 );
    size_t position = 0;
    UInt32 a = 1, b = 0;
    for ( size_t i=0; i<raw.size(); ++i )
    {
        a = ( a + raw[ i ] ) % 65521;
        b = ( b + a ) % 65521;
    }
    do
    {
        size_t count = raw.size() - position;
        if ( count > 65535 )
            count = 65535;
        bool last = position + count >= raw.size();
        zlib.push_back( last ? 1 : 0 );
        zlib.push_back( (UInt8) ( count & 0xff ) );
        zlib.push_back( (UInt8) ( count >> 8 ) );
        zlib.push_back( (UInt8) ( ~count & 0xff ) );
        zlib.push_back( (UInt8) ( ( ~count >> 8 ) & 0xff ) );
        zlib.insert( zlib.end(), raw.begin() + position, raw.begin() + position + count );
        position += count;
    }
    while ( position < raw.size() );
    Put32( zlib, ( b << 16 ) | a );

    GUCEF::vector< UInt8 > header;
    Put32( header, width );
    Put32( header, height );
    header.push_back( 8 );
    header.push_back( 6 );
    header.push_back( 0 );
    header.push_back( 0 );
    header.push_back( 0 );

    GUCEF::vector< UInt8 > png;
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
    png.insert( png.end(), signature, signature + 8 );
    PutChunk( png, "IHDR", header );
    PutChunk( png, "IDAT", zlib );
    PutChunk( png, "IEND", GUCEF::vector< UInt8 >() );

    CORE::CString path = directory;
    CORE::AppendToPath( path, name + ".png" );
    CORE::CFileAccess file( path, "wb" );
    return file.IsValid() && png.size() == file.Write( &png[ 0 ], 1, (UInt32) png.size() );
}

inline GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

}; /* namespace GridTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TESTGRIDSUPPORT_H ? */
