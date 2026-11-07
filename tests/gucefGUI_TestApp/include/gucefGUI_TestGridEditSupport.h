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

#ifndef GUCEF_GUI_TESTGRIDEDITSUPPORT_H
#define GUCEF_GUI_TESTGRIDEDITSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefGUI_TestGridSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GridTest {

/**
 *  The test model of the grid tests with cells that can be changed: every column but the id is editable, the model
 *  validates (no negative price, no name that starts with "!", no integer above a million), keeps the changed values
 *  next to the generated ones, and has the undo and redo of CTableEditHistory.
 */
class CEditableTestModel : public CTestModel
{
    public:

    GUCEF::set< UInt32 > readOnlyIds;       /**< the ids of the rows whose cells are read only */
    bool refuseAll;                         /**< the model refuses every value */
    UInt32 setCalls;
    mutable UInt32 validateCalls;
    UInt32 groupsBegun;
    UInt32 groupsEnded;

    explicit CEditableTestModel( UInt32 rows )
        : CTestModel( rows ), readOnlyIds(), refuseAll( false ), setCalls( 0 ), validateCalls( 0 ), groupsBegun( 0 ), groupsEnded( 0 ), m_overrides(), m_history()
    {
        for ( UInt32 c=0; c<COLUMNS; ++c )
            EditColumn( c ).readOnly = ( COL_ID == c );
    }

    CORE::CVariant Current( UInt32 row, UInt32 column ) const
    {
        CORE::CVariant value;
        GetCellValue( row, column, value );
        return value;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !CTestModel::GetCellValue( row, column, value ) )
            return false;
        GUCEF::map< UInt64, CORE::CVariant >::const_iterator found = m_overrides.find( MakeKey( IdOf( row ), column ) );
        if ( found != m_overrides.end() )
            value = found->second;
        return true;
    }

    virtual bool IsCellReadOnly( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return CTestModel::IsCellReadOnly( row, column ) || readOnlyIds.find( IdOf( row ) ) != readOnlyIds.end();
    }

    virtual GUI::CTableEditResult ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++validateCalls;
        if ( IsCellReadOnly( row, column ) )
            return GUI::CTableEditResult::ReadOnly();
        if ( refuseAll )
            return GUI::CTableEditResult::Refused( "The model refuses every value" );
        if ( COL_PRICE == column && value.IsNumber() && value.AsFloat64() < 0.0 )
            return GUI::CTableEditResult::Refused( "The price must not be negative" );
        if ( COL_NAME == column && value.IsString() && value.AsString().StartsWith( "!" ) )
            return GUI::CTableEditResult::Refused( "A name must not start with an exclamation mark" );
        if ( COL_MAYBE == column && value.IsNumber() && value.AsFloat64() > 1000000.0 )
            return GUI::CTableEditResult::Refused( "The number is too large" );
        return GUI::CTableEditResult::Accepted();
    }

    virtual GUI::CTableEditResult SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value ) GUCEF_VIRTUAL_OVERRIDE
    {
        const GUI::CTableEditResult result = ValidateCellEdit( row, column, value );
        if ( !result.IsAccepted() )
            return result;
        ++setCalls;
        const CORE::CVariant old = Current( row, column );
        const UInt32 id = IdOf( row );
        m_overrides[ MakeKey( id, column ) ] = value;
        m_history.Record( CORE::CVariant( id ), column, old, value );
        RaiseCells( row, 1, column, 1 );
        return GUI::CTableEditResult::Accepted();
    }

    virtual void BeginEditGroup( void ) GUCEF_VIRTUAL_OVERRIDE { ++groupsBegun; m_history.BeginGroup(); }

    virtual void EndEditGroup( void ) GUCEF_VIRTUAL_OVERRIDE { ++groupsEnded; m_history.EndGroup(); }

    virtual bool CanUndo( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_history.CanUndo(); }

    virtual bool CanRedo( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_history.CanRedo(); }

    virtual bool Undo( void ) GUCEF_VIRTUAL_OVERRIDE { return Apply( true ); }

    virtual bool Redo( void ) GUCEF_VIRTUAL_OVERRIDE { return Apply( false ); }

    const GUI::CTableEditHistory& GetHistory( void ) const { return m_history; }

    private:

    static UInt64 MakeKey( UInt32 id, UInt32 column ) { return (UInt64) id * 100 + column; }

    bool Apply( bool undo )
    {
        GUI::CTableEditHistory::TEditVector edits;
        if ( undo ? !m_history.TakeUndo( edits ) : !m_history.TakeRedo( edits ) )
            return false;
        for ( size_t i=0; i<edits.size(); ++i )
        {
            UInt32 row = 0;
            if ( !FindRowByKey( edits[ i ].rowKey, row ) )
                continue;
            m_overrides[ MakeKey( IdOf( row ), edits[ i ].column ) ] = undo ? edits[ i ].oldValue : edits[ i ].newValue;
            RaiseCells( row, 1, edits[ i ].column, 1 );
        }
        return true;
    }

    GUCEF::map< UInt64, CORE::CVariant > m_overrides;
    GUI::CTableEditHistory m_history;
};

/** The clipboard of the tests: text in memory, in place of the clipboard of the system, behind the hook of CSecretInput */
class CFakeClipboard
{
    public:

    static CORE::CString& Text( void ) { static CORE::CString text; return text; }

    static bool& Available( void ) { static bool available = true; return available; }

    static UInt32& Writes( void ) { static UInt32 writes = 0; return writes; }

    static bool Read( char* dest, UInt32 size, UInt32* written )
    {
        *written = 0;
        if ( !Available() || Text().IsNULLOrEmpty() )
            return false;
        UInt32 length = (UInt32) strlen( Text().C_String() );
        if ( length > size - 1 )
            length = size - 1;
        memcpy( dest, Text().C_String(), length );
        dest[ length ] = '\0';
        *written = length;
        return true;
    }

    static bool Write( const char* text )
    {
        if ( !Available() )
            return false;
        Text() = text;
        ++Writes();
        return true;
    }

    /** Takes the clipboard of the tests in use and empties it; the destructor gives the clipboard of the system back */
    CFakeClipboard( void )
    {
        Text().Clear();
        Available() = true;
        Writes() = 0;
        GUI::CSecretInput::SetClipboardReader( &CFakeClipboard::Read );
        GUI::CSecretInput::SetClipboardWriter( &CFakeClipboard::Write );
    }

    ~CFakeClipboard()
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
    }
};

/** Records the reports of the edit events of a grid, in the order they came */
class CEditEvents : public CORE::CObserver
{
    public:

    struct SEvent
    {
        CORE::CEvent eventId;
        GUI::CGridEditReport report;
    };
    GUCEF::vector< SEvent > events;

    explicit CEditEvents( GUI::CGridView* grid )
        : CORE::CObserver(), events()
    {
        SubscribeTo( grid, GUI::CGridView::CellEditStartedEvent );
        SubscribeTo( grid, GUI::CGridView::CellEditCommittedEvent );
        SubscribeTo( grid, GUI::CGridView::CellEditCancelledEvent );
        SubscribeTo( grid, GUI::CGridView::CellEditRefusedEvent );
        SubscribeTo( grid, GUI::CGridView::RangeEditResultEvent );
    }

    virtual ~CEditEvents() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUI::CGridView::TGridEditReportEventData* data = static_cast< GUI::CGridView::TGridEditReportEventData* >( eventData );
        if ( GUCEF_NULL == data )
            return;
        SEvent event;
        event.eventId = eventId;
        event.report = data->GetData();
        events.push_back( event );
    }

    UInt32 Count( const CORE::CEvent& eventId ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<events.size(); ++i )
            if ( events[ i ].eventId == eventId )
                ++count;
        return count;
    }

    /** The last event of the kind, NULL when there is none */
    const SEvent* Last( const CORE::CEvent& eventId ) const
    {
        for ( size_t i=events.size(); i>0; --i )
            if ( events[ i - 1 ].eventId == eventId )
                return &events[ i - 1 ];
        return GUCEF_NULL;
    }
};

/** A grid in cell mode on an editable model, 800 x 300 at 10, 10, with editing and the clipboard on or off */
inline GUI::CGridView*
MakeEditGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model, bool editing, bool clipboard )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetModel( model );
    grid->SetProperty( "SelectionMode", "Cell" );
    grid->SetBoolProperty( "CellEditing", editing );
    grid->SetBoolProperty( "CellClipboard", clipboard );
    context->FlushChanges();
    context->SetFocus( grid );
    return grid;
}

/** What the cell at a row holds, as the model says it, formatted */
inline CORE::CString
Shown( CEditableTestModel* model, UInt32 row, UInt32 column )
{
    return model->GetCellText( row, column );
}

/** Types text into the grid (the focus is on it) */
inline void
Type( GUI::CGUIContext* context, const CORE::CString& text )
{
    context->FlushChanges();
    context->InjectText( text );
}

/** A double click in the middle of a cell */
inline bool
DoubleClickCell( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column )
{
    Int32 x = 0, y = 0;
    if ( !CellPoint( context, grid, row, column, x, y ) )
        return false;
    ClickAt( context, x, y, 0, false );
    ClickAt( context, x, y );
    return true;
}

/** A grid on an editable model of 100 rows with editing on, the cursor on a cell */
struct SEditScene
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    CEditableTestModel* model;
    GUI::TTableViewModelPtr modelPtr;
    GUI::CGridView* grid;

    SEditScene( bool editing = true, bool clipboard = false, UInt32 rows = 100 )
        : contextPtr( MakeContext( 1280, 960 ) ), context( ContextOf( contextPtr ) ), model( GUCEF_NULL ), modelPtr(), grid( GUCEF_NULL )
    {
        if ( GUCEF_NULL == context )
            return;
        model = GUCEF_NEW CEditableTestModel( rows );
        modelPtr = GUI::TTableViewModelPtr( model );
        grid = MakeEditGrid( context, modelPtr, editing, clipboard );
    }

    bool IsReady( void ) const { return GUCEF_NULL != grid; }

    /** Clicks the cell, scrolling it into view first */
    bool Select( Int32 row, Int32 column )
    {
        context->FlushChanges();
        grid->ScrollToRow( row );
        return ClickCell( context, grid, row, column );
    }
};

}

#endif /* GUCEF_GUI_TESTGRIDEDITSUPPORT_H ? */
