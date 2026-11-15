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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#include "gucefGUI_CTransferQueueForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CTransferQueueForm::PauseRequestedEvent = "GUCEF::GUI::CTransferQueueForm::PauseRequestedEvent";
const CORE::CEvent CTransferQueueForm::ResumeRequestedEvent = "GUCEF::GUI::CTransferQueueForm::ResumeRequestedEvent";
const CORE::CEvent CTransferQueueForm::CancelRequestedEvent = "GUCEF::GUI::CTransferQueueForm::CancelRequestedEvent";
const CORE::CEvent CTransferQueueForm::RetryRequestedEvent = "GUCEF::GUI::CTransferQueueForm::RetryRequestedEvent";
const CORE::CEvent CTransferQueueForm::RemoveRequestedEvent = "GUCEF::GUI::CTransferQueueForm::RemoveRequestedEvent";
const CORE::CEvent CTransferQueueForm::OpenFolderRequestedEvent = "GUCEF::GUI::CTransferQueueForm::OpenFolderRequestedEvent";
const CORE::CEvent CTransferQueueForm::SelectionChangedEvent = "GUCEF::GUI::CTransferQueueForm::SelectionChangedEvent";

namespace {

/** The names of the buttons of a row, by action */
const char* const ButtonNames[] = { "Pause", "Resume", "Cancel", "Retry", "Remove", "OpenFolder" };

/** What the rows are drawn in, by state of the entry */
const CColour NeutralRowColour( 46, 50, 62, 255 );
const CColour FailedRowColour( 96, 44, 44, 255 );
const CColour DoneRowColour( 38, 70, 48, 255 );
const CColour CancelledRowColour( 60, 60, 64, 255 );
const CColour RowBorderColour( 30, 32, 38, 255 );
const CColour SelectedBorderColour( 150, 180, 240, 255 );
const CColour NameColour( 235, 238, 245, 255 );
const CColour InfoColour( 200, 205, 215, 255 );
const CColour DetailColour( 140, 146, 162, 255 );

/** The metrics of a row in pixels */
const Int32 RowMargin = 8;
const Int32 LineHeight = 18;
const Int32 ButtonHeight = 20;
const Int32 ButtonGap = 4;
const Int32 ButtonMinWidth = 44;
const Int32 ButtonPadding = 16;
const Int32 FirstLineTop = 4;
const Int32 SecondLineTop = 24;
const Int32 ThirdLineTop = 44;
const Int32 ThirdLineHeight = 11;
const Int32 ProgressHeight = 14;

CColour
Lighten( const CColour& colour, UInt32 amount )
{GUCEF_TRACE;

    const UInt32 r = colour.r + amount;
    const UInt32 g = colour.g + amount;
    const UInt32 b = colour.b + amount;
    return CColour( (UInt8) ( r > 255 ? 255 : r ), (UInt8) ( g > 255 ? 255 : g ), (UInt8) ( b > 255 ? 255 : b ), colour.a );
}

/*-------------------------------------------------------------------------*/

CColour
RowColourOf( CITransferQueueModel::ETransferState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case CITransferQueueModel::TRANSFERSTATE_FAILED:
            return FailedRowColour;
        case CITransferQueueModel::TRANSFERSTATE_DONE:
            return DoneRowColour;
        case CITransferQueueModel::TRANSFERSTATE_CANCELLED:
            return CancelledRowColour;
        default:
            return NeutralRowColour;
    }
}

/*-------------------------------------------------------------------------*/

CColour
BarColourOf( CITransferQueueModel::ETransferState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case CITransferQueueModel::TRANSFERSTATE_PAUSED:
            return CColour( 200, 160, 50, 255 );
        case CITransferQueueModel::TRANSFERSTATE_FINISHING:
            return CColour( 80, 150, 200, 255 );
        case CITransferQueueModel::TRANSFERSTATE_FAILED:
            return CColour( 200, 70, 70, 255 );
        case CITransferQueueModel::TRANSFERSTATE_CANCELLED:
            return CColour( 120, 120, 128, 255 );
        default:
            return CColour( 60, 140, 60, 255 );
    }
}

/*-------------------------------------------------------------------------*/

/** A text on one line */
CString
OneLine( const CString& text )
{GUCEF_TRACE;

    return text.ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' ).ReplaceChar( '\t', ' ' );
}

/*-------------------------------------------------------------------------*/

CString
TwoDigits( UInt64 value )
{GUCEF_TRACE;

    return value < 10 ? "0" + CORE::UInt64ToString( value ) : CORE::UInt64ToString( value );
}

/*-------------------------------------------------------------------------*/

/** The percentage of the progress, rounded down */
UInt32
PercentOf( const CITransferQueueModel::TEntry& entry )
{GUCEF_TRACE;

    if ( CITransferQueueModel::TRANSFERSTATE_DONE == entry.state )
        return 100;
    if ( entry.fraction < 0.0f && entry.totalBytes > 0 )
    {
        if ( entry.doneBytes >= entry.totalBytes )
            return 100;
        return (UInt32) ( ( (Float64) entry.doneBytes * 100.0 ) / (Float64) entry.totalBytes );
    }
    const Float64 percent = (Float64) entry.GetProgress() * 100.0 + 0.0001;
    return percent >= 100.0 ? 100 : (UInt32) percent;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTransferQueueForm::SRow::SRow( void )
    : panel( GUCEF_NULL )
    , name( GUCEF_NULL )
    , info( GUCEF_NULL )
    , detail( GUCEF_NULL )
    , progress( GUCEF_NULL )
    , entryIndex( -1 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) CITransferQueueModel::TRANSFERACTION_COUNT; ++i )
        buttons[ i ] = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::RegisterEvents( void )
{GUCEF_TRACE;

    PauseRequestedEvent.Initialize();
    ResumeRequestedEvent.Initialize();
    CancelRequestedEvent.Initialize();
    RetryRequestedEvent.Initialize();
    RemoveRequestedEvent.Initialize();
    OpenFolderRequestedEvent.Initialize();
    SelectionChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTransferQueueForm::CTransferQueueForm( void )
    : CFormEx()
    , m_model( GUCEF_NULL )
    , m_entries()
    , m_rows()
    , m_selectedId()
    , m_summary()
    , m_performActions( true )
    , m_dirty( false )
    , m_rowsDirty( true )
    , m_refreshed( false )
    , m_waitedInMs( 0 )
    , m_refreshCount( 0 )
    , m_lastScroll( -1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_frameSubscribed( false )
    , m_fontLineHeight( 0 )
    , m_rowHeight( (Int32) RowHeight )
    , m_lineHeight( LineHeight )
    , m_buttonHeight( ButtonHeight )
    , m_secondLineTop( SecondLineTop )
    , m_thirdLineTop( ThirdLineTop )
    , m_thirdLineHeight( ThirdLineHeight )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTransferQueueForm::CTransferQueueForm( CIGUIContext* context )
    : CFormEx( context )
    , m_model( GUCEF_NULL )
    , m_entries()
    , m_rows()
    , m_selectedId()
    , m_summary()
    , m_performActions( true )
    , m_dirty( false )
    , m_rowsDirty( true )
    , m_refreshed( false )
    , m_waitedInMs( 0 )
    , m_refreshCount( 0 )
    , m_lastScroll( -1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_frameSubscribed( false )
    , m_fontLineHeight( 0 )
    , m_rowHeight( (Int32) RowHeight )
    , m_lineHeight( LineHeight )
    , m_buttonHeight( ButtonHeight )
    , m_secondLineTop( SecondLineTop )
    , m_thirdLineTop( ThirdLineTop )
    , m_thirdLineHeight( ThirdLineHeight )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTransferQueueForm::~CTransferQueueForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_model = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "TransferQueueForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "TransferQueueForm", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* rows = CFormLayout::AddWidgetNode( *content, "ScrollPanel", "Rows", "0,0 0,0", "1,0 1,-24" );
    CFormLayout::SetWidgetNodeProperty( *rows, EWidgetProperty_BackgroundColour, "30,32,38,255" );
    CFormLayout::SetWidgetNodeProperty( *rows, EWidgetProperty_Focusable, "true" );
    CFormLayout::SetWidgetNodeProperty( *rows, EWidgetProperty_ScrollStep, CORE::Int32ToString( m_rowHeight ) );

    CORE::CDataNode* empty = CFormLayout::AddWidgetNode( *content, "Label", "Empty", "0,0 0.5,-22", "1,0 0,20" );
    CFormLayout::SetWidgetNodeProperty( *empty, EWidgetProperty_Text, "No transfers" );
    CFormLayout::SetWidgetNodeProperty( *empty, EWidgetProperty_HTextAlign, "Center" );
    CFormLayout::SetWidgetNodeProperty( *empty, EWidgetProperty_VTextAlign, "Center" );
    CFormLayout::SetWidgetNodeProperty( *empty, EWidgetProperty_InputTransparent, "true" );

    CORE::CDataNode* summary = CFormLayout::AddWidgetNode( *content, "Label", "Summary", "0,8 1,-22", "1,-16 0,20" );
    CFormLayout::SetWidgetNodeProperty( *summary, EWidgetProperty_VTextAlign, "Center" );
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    // the rows are made as they are needed: rows that came with a saved layout, or from an earlier layout, are not used
    m_rows.clear();
    CGUIContext* context = GetGUIContext();
    CWidget* rows = GetWidget( "Rows" );
    if ( GUCEF_NULL != rows && GUCEF_NULL != context )
    {
        CWidget::TWidgetSet children;
        rows->GetChildWidgetSet( children );
        for ( CWidget::TWidgetSet::iterator i = children.begin(); i != children.end(); ++i )
            context->DestroyWidget( (*i) );
        TEventCallback clickCallback( this, &CTransferQueueForm::OnRowsMouseClick );
        SubscribeTo( rows, CWidget::MouseClickEvent, clickCallback );
        TEventCallback keyCallback( this, &CTransferQueueForm::OnRowsKeyDown );
        SubscribeTo( rows, CWidget::KeyDownEvent, keyCallback );
    }
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CTransferQueueForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }
    m_lastScroll = -1;
    m_rowsDirty = true;
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::SetModel( CITransferQueueModel* model )
{GUCEF_TRACE;

    if ( model != m_model )
    {
        if ( GUCEF_NULL != m_model )
            UnsubscribeFrom( m_model );
        m_model = model;
        if ( GUCEF_NULL != m_model )
        {
            TEventCallback changedCallback( this, &CTransferQueueForm::OnModelChanged );
            SubscribeTo( m_model, CITransferQueueModel::EntriesChangedEvent, changedCallback );
        }
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

CITransferQueueModel*
CTransferQueueForm::GetModel( void ) const
{GUCEF_TRACE;

    return m_model;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::SetPerformActions( bool perform )
{GUCEF_TRACE;

    m_performActions = perform;
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::GetPerformActions( void ) const
{GUCEF_TRACE;

    return m_performActions;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::Refresh( void )
{GUCEF_TRACE;

    ++m_refreshCount;
    m_dirty = false;
    m_waitedInMs = 0;
    m_refreshed = true;

    TEntryVector entries;
    if ( GUCEF_NULL != m_model )
        m_model->GetEntries( entries );
    m_entries = entries;

    const bool selectionVanished = !m_selectedId.IsNULLOrEmpty() && FindEntryIndex( m_selectedId ) < 0;
    if ( selectionVanished )
        m_selectedId.Clear();

    ShowEmptyState();
    ShowSummary();
    m_rowsDirty = true;
    RealizeRows();

    if ( selectionVanished )
    {
        CORE::TCloneableString data( CString::Empty );
        NotifyObservers( SelectionChangedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( m_dirty )
    {
        m_waitedInMs += deltaInMs;
        if ( m_waitedInMs >= (UInt32) RefreshIntervalInMs )
            Refresh();
    }

    // the rows follow the font (its line height), the scrolling and the size of the list
    if ( UpdateRowMetrics() )
        m_rowsDirty = true;
    CScrollPanel* rows = CWidget::ToScrollPanel( GetWidget( "Rows" ) );
    if ( GUCEF_NULL != rows )
    {
        const CRect viewport = rows->GetViewportRect();
        if ( m_rowsDirty || rows->GetScrollPosition() != m_lastScroll || viewport.width != m_lastViewportWidth || viewport.height != m_lastViewportHeight )
            RealizeRows();
    }
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::UpdateRowMetrics( void )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    CWidget* summary = GetWidget( "Summary" );
    if ( GUCEF_NULL == context || GUCEF_NULL == summary )
        return false;
    const Int32 line = context->GetLineHeight( summary->GetPropertyRef( EWidgetProperty_Font ) );
    if ( line == m_fontLineHeight )
        return false;
    m_fontLineHeight = line;

    // a small font keeps the metrics the row was drawn for; a taller line makes every line and the buttons as high as it needs
    m_lineHeight = line > LineHeight ? line : LineHeight;
    m_buttonHeight = line + 4 > ButtonHeight ? line + 4 : ButtonHeight;
    m_secondLineTop = FirstLineTop + m_lineHeight + 2;
    m_thirdLineTop = m_secondLineTop + m_buttonHeight;
    m_thirdLineHeight = line > ThirdLineHeight ? line : ThirdLineHeight;
    m_rowHeight = line <= ThirdLineHeight ? (Int32) RowHeight : m_thirdLineTop + m_thirdLineHeight + 4;

    // the list scrolls a row at a time; the summary is one line high at the bottom, the list ends above it
    const Int32 summaryHeight = line > 20 ? line : 20;
    CWidget* rows = GetWidget( "Rows" );
    if ( GUCEF_NULL != rows )
    {
        rows->SetProperty( EWidgetProperty_ScrollStep, CORE::Int32ToString( m_rowHeight ) );
        rows->SetProperty( EWidgetProperty_Size, "1,0 1,-" + CORE::Int32ToString( summaryHeight + 4 ) );
    }
    summary->SetProperty( EWidgetProperty_Position, "0,8 1,-" + CORE::Int32ToString( summaryHeight + 2 ) );
    summary->SetProperty( EWidgetProperty_Size, "1,-16 0," + CORE::Int32ToString( summaryHeight ) );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CTransferQueueForm::GetRowHeight( void ) const
{GUCEF_TRACE;

    return m_rowHeight;
}

/*-------------------------------------------------------------------------*/

UInt32
CTransferQueueForm::GetRefreshCount( void ) const
{GUCEF_TRACE;

    return m_refreshCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTransferQueueForm::GetEntryCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

const CTransferQueueForm::TEntry&
CTransferQueueForm::GetEntry( UInt32 index ) const
{GUCEF_TRACE;

    static const TEntry none;
    return index < (UInt32) m_entries.size() ? m_entries[ index ] : none;
}

/*-------------------------------------------------------------------------*/

Int32
CTransferQueueForm::FindEntryIndex( const CString& id ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_entries.size(); ++i )
        if ( m_entries[ i ].id == id )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

const CString&
CTransferQueueForm::GetSelectedEntryId( void ) const
{GUCEF_TRACE;

    return m_selectedId;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::ChangeSelection( const CString& id )
{GUCEF_TRACE;

    if ( id == m_selectedId )
        return;
    m_selectedId = id;
    m_rowsDirty = true;
    RealizeRows();
    CORE::TCloneableString data( id );
    NotifyObservers( SelectionChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::SelectEntry( const CString& id )
{GUCEF_TRACE;

    if ( id.IsNULLOrEmpty() )
    {
        ChangeSelection( CString::Empty );
        return true;
    }
    const Int32 index = FindEntryIndex( id );
    if ( index < 0 )
        return false;

    CScrollPanel* rows = CWidget::ToScrollPanel( GetWidget( "Rows" ) );
    if ( GUCEF_NULL != rows )
    {
        const Int32 top = index * m_rowHeight;
        const Int32 scroll = rows->GetScrollPosition();
        const Int32 height = rows->GetViewportRect().height;
        if ( top < scroll )
            rows->SetScrollPosition( top );
        else
        if ( height > 0 && top + m_rowHeight > scroll + height )
            rows->SetScrollPosition( top + m_rowHeight - height );
    }
    ChangeSelection( id );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CEvent&
CTransferQueueForm::GetActionEvent( ETransferAction action )
{GUCEF_TRACE;

    switch ( action )
    {
        case CITransferQueueModel::TRANSFERACTION_PAUSE:        return PauseRequestedEvent;
        case CITransferQueueModel::TRANSFERACTION_RESUME:       return ResumeRequestedEvent;
        case CITransferQueueModel::TRANSFERACTION_CANCEL:       return CancelRequestedEvent;
        case CITransferQueueModel::TRANSFERACTION_RETRY:        return RetryRequestedEvent;
        case CITransferQueueModel::TRANSFERACTION_REMOVE:       return RemoveRequestedEvent;
        default:                                                return OpenFolderRequestedEvent;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::RequestAction( const CString& id, ETransferAction action )
{GUCEF_TRACE;

    if ( (UInt32) action >= (UInt32) CITransferQueueModel::TRANSFERACTION_COUNT )
        return false;
    const Int32 index = FindEntryIndex( id );
    if ( index < 0 )
        return false;
    if ( GUCEF_NULL != m_model && !m_model->IsActionAvailable( m_entries[ index ], action ) )
        return false;

    // a copy: what the observers do may change the entries
    const CString entryId = m_entries[ index ].id;
    CORE::TCloneableString data( entryId );
    NotifyObservers( GetActionEvent( action ), &data );
    if ( m_performActions && GUCEF_NULL != m_model )
    {
        m_model->PerformAction( entryId, action );
        m_dirty = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::IsEmptyStateShown( void ) const
{GUCEF_TRACE;

    CWidget* empty = const_cast< CTransferQueueForm* >( this )->GetWidget( "Empty" );
    return GUCEF_NULL != empty && empty->GetBoolProperty( EWidgetProperty_Visible );
}

/*-------------------------------------------------------------------------*/

UInt32
CTransferQueueForm::GetRealizedRowCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32) m_rows.size(); ++i )
        if ( m_rows[ i ].entryIndex >= 0 )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::IsActionShown( const CString& id, ETransferAction action ) const
{GUCEF_TRACE;

    const Int32 index = FindEntryIndex( id );
    if ( index < 0 || (UInt32) action >= (UInt32) CITransferQueueModel::TRANSFERACTION_COUNT )
        return false;
    for ( UInt32 i=0; i<(UInt32) m_rows.size(); ++i )
    {
        if ( m_rows[ i ].entryIndex == index )
            return GUCEF_NULL != m_rows[ i ].buttons[ action ] && m_rows[ i ].buttons[ action ]->GetBoolProperty( EWidgetProperty_Visible );
    }
    return GUCEF_NULL != m_model && m_model->IsActionAvailable( m_entries[ index ], action );
}

/*-------------------------------------------------------------------------*/

CWidget*
CTransferQueueForm::GetRowWidget( const CString& id ) const
{GUCEF_TRACE;

    const Int32 index = FindEntryIndex( id );
    if ( index < 0 )
        return GUCEF_NULL;
    for ( UInt32 i=0; i<(UInt32) m_rows.size(); ++i )
        if ( m_rows[ i ].entryIndex == index )
            return m_rows[ i ].panel;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::ShowEmptyState( void )
{GUCEF_TRACE;

    CWidget* empty = GetWidget( "Empty" );
    if ( GUCEF_NULL != empty )
        empty->SetBoolProperty( EWidgetProperty_Visible, GUCEF_NULL == m_model || m_entries.empty() );
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::ShowSummary( void )
{GUCEF_TRACE;

    m_summary.Clear();
    if ( m_refreshed && GUCEF_NULL != m_model )
    {
        UInt32 running = 0;
        UInt64 done = 0, total = 0, rate = 0;
        for ( UInt32 i=0; i<(UInt32) m_entries.size(); ++i )
        {
            const TEntry& entry = m_entries[ i ];
            if ( CITransferQueueModel::TRANSFERSTATE_RUNNING == entry.state )
            {
                ++running;
                rate += entry.bytesPerSecond;
            }
            if ( entry.totalBytes > 0 )
            {
                total += entry.totalBytes;
                done += ( entry.doneBytes > entry.totalBytes || CITransferQueueModel::TRANSFERSTATE_DONE == entry.state ) ? entry.totalBytes : entry.doneBytes;
            }
        }
        const UInt32 count = (UInt32) m_entries.size();
        m_summary = CORE::UInt32ToString( count ) + ( 1 == count ? " transfer, " : " transfers, " ) + CORE::UInt32ToString( running ) + " running";
        if ( total > 0 )
            m_summary += ", " + FormatSize( done ) + " of " + FormatSize( total );
        if ( rate > 0 )
            m_summary += ", " + FormatRate( rate );
    }
    CWidget* summary = GetWidget( "Summary" );
    if ( GUCEF_NULL != summary )
        summary->SetProperty( EWidgetProperty_Text, m_summary );
}

/*-------------------------------------------------------------------------*/

Int32
CTransferQueueForm::TextWidth( CWidget* widget, const CString& text ) const
{GUCEF_TRACE;

    CGUIContext* context = const_cast< CTransferQueueForm* >( this )->GetGUIContext();
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != context && GUCEF_NULL != widget && context->MeasureText( widget->GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height ) )
        return width;
    return (Int32) text.Length() * 7;
}

/*-------------------------------------------------------------------------*/

bool
CTransferQueueForm::MakeRow( UInt32 slot )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    CWidget* rows = GetWidget( "Rows" );
    if ( GUCEF_NULL == context || GUCEF_NULL == rows )
        return false;

    SRow row;
    row.panel = context->CreateWidget( "Panel", "Row_" + CORE::UInt32ToString( slot ), rows );
    if ( GUCEF_NULL == row.panel )
        return false;
    row.panel->SetBoolProperty( EWidgetProperty_Clip, true );
    row.panel->SetBoolProperty( EWidgetProperty_Visible, false );

    row.name = context->CreateWidget( "Label", "Name", row.panel );
    row.name->SetProperty( EWidgetProperty_VTextAlign, "Center" );
    row.name->SetColourProperty( EWidgetProperty_TextColour, NameColour );
    row.name->SetBoolProperty( EWidgetProperty_InputTransparent, true );

    row.info = context->CreateWidget( "Label", "Info", row.panel );
    row.info->SetProperty( EWidgetProperty_HTextAlign, "Right" );
    row.info->SetProperty( EWidgetProperty_VTextAlign, "Center" );
    row.info->SetColourProperty( EWidgetProperty_TextColour, InfoColour );
    row.info->SetBoolProperty( EWidgetProperty_InputTransparent, true );

    row.progress = context->CreateWidget( "ProgressBar", "Progress", row.panel );
    row.progress->SetBoolProperty( EWidgetProperty_InputTransparent, true );

    row.detail = context->CreateWidget( "Label", "Detail", row.panel );
    row.detail->SetProperty( EWidgetProperty_VTextAlign, "Center" );
    row.detail->SetColourProperty( EWidgetProperty_TextColour, DetailColour );
    row.detail->SetBoolProperty( EWidgetProperty_InputTransparent, true );

    for ( UInt32 a=0; a<(UInt32) CITransferQueueModel::TRANSFERACTION_COUNT; ++a )
    {
        CWidget* button = context->CreateWidget( "Button", ButtonNames[ a ], row.panel );
        button->SetProperty( EWidgetProperty_Text, CITransferQueueModel::ActionToString( (ETransferAction) a ) );
        button->SetBoolProperty( EWidgetProperty_Visible, false );
        TEventCallback callback( this, &CTransferQueueForm::OnButtonActivated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
        row.buttons[ a ] = button;
    }
    m_rows.push_back( row );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::BindRow( UInt32 slot, Int32 entryIndex, Int32 rowWidth )
{GUCEF_TRACE;

    SRow& row = m_rows[ slot ];
    const TEntry& entry = m_entries[ entryIndex ];
    row.entryIndex = entryIndex;
    CGUIContext* context = GetGUIContext();
    const bool selected = entry.id == m_selectedId;

    CColour background = RowColourOf( entry.state );
    if ( selected )
        background = Lighten( background, 30 );
    row.panel->SetProperty( EWidgetProperty_Position, "0,0 0," + CORE::Int32ToString( entryIndex * m_rowHeight ) );
    row.panel->SetProperty( EWidgetProperty_Size, "1,0 0," + CORE::Int32ToString( m_rowHeight - 2 ) );
    row.panel->SetColourProperty( EWidgetProperty_BackgroundColour, background );
    row.panel->SetColourProperty( EWidgetProperty_BorderColour, selected ? SelectedBorderColour : RowBorderColour );
    row.panel->SetBoolProperty( EWidgetProperty_Visible, true );

    // the first line: the name takes what the state text leaves
    const CString info = FormatEntryInfo( entry );
    const Int32 available = rowWidth - 3 * (Int32) RowMargin;
    Int32 infoRoom = TextWidth( row.info, info ) + 2;
    const Int32 infoMax = available - available / 3;
    if ( infoRoom > infoMax )
        infoRoom = infoMax;
    if ( infoRoom < 0 )
        infoRoom = 0;
    const Int32 nameRoom = available - infoRoom > 0 ? available - infoRoom : 0;
    bool cut = false;
    const CString& font = row.name->GetPropertyRef( EWidgetProperty_Font );
    row.name->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( (Int32) RowMargin ) + " 0," + CORE::Int32ToString( (Int32) FirstLineTop ) );
    row.name->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( nameRoom ) + " 0," + CORE::Int32ToString( m_lineHeight ) );
    row.name->SetProperty( EWidgetProperty_Text, GUCEF_NULL != context ? CWidgetPainter::FitText( *context, font, OneLine( entry.name ), nameRoom, cut ) : OneLine( entry.name ) );
    row.info->SetProperty( EWidgetProperty_Position, "1,-" + CORE::Int32ToString( (Int32) RowMargin + infoRoom ) + " 0," + CORE::Int32ToString( (Int32) FirstLineTop ) );
    row.info->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( infoRoom ) + " 0," + CORE::Int32ToString( m_lineHeight ) );
    row.info->SetProperty( EWidgetProperty_Text, GUCEF_NULL != context ? CWidgetPainter::FitText( *context, font, info, infoRoom, cut ) : info );

    // the second line: the buttons from the right edge, the progress bar in the room that is left
    Int32 taken = (Int32) RowMargin;
    for ( Int32 a = (Int32) CITransferQueueModel::TRANSFERACTION_COUNT - 1; a >= 0; --a )
    {
        CWidget* button = row.buttons[ a ];
        const bool shown = GUCEF_NULL == m_model || m_model->IsActionAvailable( entry, (ETransferAction) a );
        button->SetBoolProperty( EWidgetProperty_Visible, shown );
        if ( !shown )
            continue;
        Int32 width = TextWidth( button, CITransferQueueModel::ActionToString( (ETransferAction) a ) ) + (Int32) ButtonPadding;
        if ( width < (Int32) ButtonMinWidth )
            width = (Int32) ButtonMinWidth;
        taken += width;
        button->SetProperty( EWidgetProperty_Position, "1,-" + CORE::Int32ToString( taken ) + " 0," + CORE::Int32ToString( m_secondLineTop ) );
        button->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( width ) + " 0," + CORE::Int32ToString( m_buttonHeight ) );
        taken += (Int32) ButtonGap;
    }
    row.progress->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( (Int32) RowMargin ) + " 0," + CORE::Int32ToString( m_secondLineTop + ( m_buttonHeight - (Int32) ProgressHeight ) / 2 ) );
    const Int32 progressWidth = rowWidth - (Int32) RowMargin - taken;
    row.progress->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( progressWidth > 0 ? progressWidth : 0 ) + " 0," + CORE::Int32ToString( (Int32) ProgressHeight ) );
    row.progress->SetFloatProperty( EWidgetProperty_Value, entry.GetProgress() * 100.0f );
    row.progress->SetColourProperty( EWidgetProperty_BarColour, BarColourOf( entry.state ) );

    // the third line: what kind of transfer it is and what it is for
    CString detail = entry.kind;
    if ( !entry.detail.IsNULLOrEmpty() )
        detail += ( detail.IsNULLOrEmpty() ? "" : ": " ) + OneLine( entry.detail );
    row.detail->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( (Int32) RowMargin ) + " 0," + CORE::Int32ToString( m_thirdLineTop ) );
    const Int32 detailRoom = rowWidth - 2 * (Int32) RowMargin;
    row.detail->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( detailRoom > 0 ? detailRoom : 0 ) + " 0," + CORE::Int32ToString( m_thirdLineHeight ) );
    row.detail->SetProperty( EWidgetProperty_Text, GUCEF_NULL != context ? CWidgetPainter::FitText( *context, font, detail, detailRoom, cut ) : detail );

    CString tooltip = entry.name + "\n" + info;
    if ( !detail.IsNULLOrEmpty() )
        tooltip += "\n" + detail;
    if ( !entry.message.IsNULLOrEmpty() )
        tooltip += "\n" + entry.message;
    row.panel->SetProperty( EWidgetProperty_Tooltip, tooltip );
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::RealizeRows( void )
{GUCEF_TRACE;

    CScrollPanel* rows = CWidget::ToScrollPanel( GetWidget( "Rows" ) );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == rows || GUCEF_NULL == context )
        return;
    const CRect viewport = rows->GetViewportRect();
    if ( viewport.width <= 0 || viewport.height <= 0 )
    {
        m_rowsDirty = true;     // not laid out yet: the next frame
        return;
    }
    m_rowsDirty = false;

    // the room of all the rows: a spacer at the bottom makes the scroll panel as high as the rows need
    const Int32 count = (Int32) m_entries.size();
    CWidget* spacer = rows->GetChildWidget( "Spacer" );
    if ( GUCEF_NULL == spacer )
    {
        spacer = context->CreateWidget( "Panel", "Spacer", rows );
        if ( GUCEF_NULL != spacer )
        {
            spacer->SetColourProperty( EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
            spacer->SetColourProperty( EWidgetProperty_BorderColour, CColour( 0, 0, 0, 0 ) );
            spacer->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        }
    }
    if ( GUCEF_NULL != spacer )
    {
        spacer->SetProperty( EWidgetProperty_Position, "0,0 0," + CORE::Int32ToString( count > 0 ? count * m_rowHeight - 1 : 0 ) );
        spacer->SetProperty( EWidgetProperty_Size, "0,1 0,1" );
    }

    // the rows in view and a few more
    Int32 scroll = rows->GetScrollPosition();
    const Int32 maxScroll = count * m_rowHeight - viewport.height;
    if ( scroll > ( maxScroll > 0 ? maxScroll : 0 ) )
        scroll = maxScroll > 0 ? maxScroll : 0;
    if ( scroll < 0 )
        scroll = 0;
    const Int32 firstEntry = scroll / m_rowHeight;
    Int32 needed = viewport.height / m_rowHeight + 3;
    if ( needed > count - firstEntry )
        needed = count - firstEntry;
    if ( needed < 0 )
        needed = 0;
    while ( (Int32) m_rows.size() < needed )
    {
        if ( !MakeRow( (UInt32) m_rows.size() ) )
            break;
    }
    for ( UInt32 s=0; s<(UInt32) m_rows.size(); ++s )
    {
        if ( (Int32) s < needed )
            BindRow( s, firstEntry + (Int32) s, viewport.width );
        else
        {
            m_rows[ s ].entryIndex = -1;
            m_rows[ s ].panel->SetBoolProperty( EWidgetProperty_Visible, false );
        }
    }
    m_lastScroll = rows->GetScrollPosition();
    m_lastViewportWidth = viewport.width;
    m_lastViewportHeight = viewport.height;
}

/*-------------------------------------------------------------------------*/

CString
CTransferQueueForm::FormatEntryInfo( const TEntry& entry )
{GUCEF_TRACE;

    CString info = entry.stateText.IsNULLOrEmpty() ? CITransferQueueModel::StateToString( entry.state ) : entry.stateText;
    if ( CITransferQueueModel::TRANSFERSTATE_FAILED == entry.state && !entry.message.IsNULLOrEmpty() )
        return info + ": " + OneLine( entry.message );

    const bool progressKnown = entry.fraction >= 0.0f || entry.totalBytes > 0;
    const bool active = CITransferQueueModel::TRANSFERSTATE_QUEUED != entry.state && !entry.IsEnded();
    if ( progressKnown && active )
        info += ", " + CORE::UInt32ToString( PercentOf( entry ) ) + "%";
    if ( entry.totalBytes > 0 )
    {
        if ( CITransferQueueModel::TRANSFERSTATE_DONE == entry.state || ( CITransferQueueModel::TRANSFERSTATE_QUEUED == entry.state && 0 == entry.doneBytes ) )
            info += ", " + FormatSize( entry.totalBytes );
        else
            info += ", " + FormatSize( entry.doneBytes > entry.totalBytes ? entry.totalBytes : entry.doneBytes ) + " of " + FormatSize( entry.totalBytes );
    }
    else
    if ( entry.doneBytes > 0 )
        info += ", " + FormatSize( entry.doneBytes );
    if ( CITransferQueueModel::TRANSFERSTATE_RUNNING == entry.state )
    {
        if ( entry.bytesPerSecond > 0 )
            info += ", " + FormatRate( entry.bytesPerSecond );
        if ( entry.secondsLeft > 0 )
            info += ", " + FormatDuration( entry.secondsLeft ) + " left";
    }
    return info;
}

/*-------------------------------------------------------------------------*/

CString
CTransferQueueForm::FormatSize( UInt64 bytes )
{GUCEF_TRACE;

    static const char* const units[] = { "B", "KB", "MB", "GB", "TB" };
    if ( bytes < 1024 )
        return CORE::UInt64ToString( bytes ) + " B";

    Float64 value = (Float64) bytes;
    UInt32 unit = 0;
    while ( value >= 1024.0 && unit < 4 )
    {
        value /= 1024.0;
        ++unit;
    }
    for ( ;; )
    {
        // one decimal up to megabytes, two above; a value that rounds to 1024 is shown in the next unit
        const UInt64 scale = unit >= 3 ? 100 : 10;
        const UInt64 scaled = (UInt64) ( value * (Float64) scale + 0.5 );
        if ( scaled >= 1024 * scale && unit < 4 )
        {
            value /= 1024.0;
            ++unit;
            continue;
        }
        CString fraction = CORE::UInt64ToString( scaled % scale );
        if ( 100 == scale && ( scaled % scale ) < 10 )
            fraction = "0" + fraction;
        return CORE::UInt64ToString( scaled / scale ) + "." + fraction + " " + CString( units[ unit ] );
    }
}

/*-------------------------------------------------------------------------*/

CString
CTransferQueueForm::FormatRate( UInt64 bytesPerSecond )
{GUCEF_TRACE;

    return FormatSize( bytesPerSecond ) + "/s";
}

/*-------------------------------------------------------------------------*/

CString
CTransferQueueForm::FormatDuration( UInt64 seconds )
{GUCEF_TRACE;

    if ( seconds < 60 )
        return CORE::UInt64ToString( seconds ) + " s";
    if ( seconds < 3600 )
        return CORE::UInt64ToString( seconds / 60 ) + " min " + TwoDigits( seconds % 60 ) + " s";
    return CORE::UInt64ToString( seconds / 3600 ) + " h " + TwoDigits( ( seconds % 3600 ) / 60 ) + " min";
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnModelChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnNotify( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventId && GUCEF_NULL != notifier && notifier == m_model )
    {
        m_model = GUCEF_NULL;
        m_dirty = true;
        m_waitedInMs = (UInt32) RefreshIntervalInMs;
        return;
    }
    CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnButtonActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CRoutedEventData* routed = CRoutedEventData::FromCloneable( eventData );
    if ( GUCEF_NULL != routed && EVENTPHASE_TUNNEL == routed->phase )
        return;
    for ( UInt32 r=0; r<(UInt32) m_rows.size(); ++r )
    {
        if ( m_rows[ r ].entryIndex < 0 )
            continue;
        for ( UInt32 a=0; a<(UInt32) CITransferQueueModel::TRANSFERACTION_COUNT; ++a )
        {
            if ( notifier == m_rows[ r ].buttons[ a ] )
            {
                const CString id = m_entries[ m_rows[ r ].entryIndex ].id;
                RequestAction( id, (ETransferAction) a );
                return;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnRowsMouseClick( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CPointerEventData* pointer = static_cast< const CPointerEventData* >( CRoutedEventData::FromCloneable( eventData ) );
    if ( GUCEF_NULL == pointer || MOUSEBUTTON_LEFT != pointer->button || EVENTPHASE_TUNNEL == pointer->phase )
        return;
    CWidget* rows = GetWidget( "Rows" );
    CWidget* hit = pointer->source;
    if ( GUCEF_NULL != hit && hit->IsRegisteredAs( "Button" ) )
        return;     // a button has its own event

    // the row that was hit: the widget below the scroll panel that the source is, or is in
    while ( GUCEF_NULL != hit && hit->GetParentWidget() != rows )
        hit = hit->GetParentWidget();
    if ( GUCEF_NULL == hit )
        return;
    for ( UInt32 r=0; r<(UInt32) m_rows.size(); ++r )
    {
        if ( m_rows[ r ].panel == hit && m_rows[ r ].entryIndex >= 0 )
        {
            const CString id = m_entries[ m_rows[ r ].entryIndex ].id;
            rows->SetFocus();
            ChangeSelection( id );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnRowsKeyDown( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( CRoutedEventData::FromCloneable( eventData ) );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_BUBBLE == key->phase )
        return;
    if ( INPUT::KEYCODE_DELETE == key->keyCode && !m_selectedId.IsNULLOrEmpty() )
    {
        const CString id = m_selectedId;
        if ( RequestAction( id, CITransferQueueModel::TRANSFERACTION_REMOVE ) )
            key->handled = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTransferQueueForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

const CString&
CTransferQueueForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CTransferQueueForm";
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
