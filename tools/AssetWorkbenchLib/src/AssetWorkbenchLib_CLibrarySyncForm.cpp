/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CPROGRESSBAR_H
#include "gucefGUI_CProgressBar.h"
#define GUCEF_GUI_CPROGRESSBAR_H
#endif /* GUCEF_GUI_CPROGRESSBAR_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "AssetWorkbenchLib_CLibrarySyncForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CLibrarySyncForm::ClosedEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncForm::ClosedEvent";

namespace {

const char* const g_storeColumns = "Store\nSign in\nMissing\nStale\nCurrent\nUnavailable\nDownload\nFree disk\nProgress\nLast result";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLibrarySyncForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLibrarySyncForm::CLibrarySyncForm( void )
    : GUI::CFormEx()
    , m_controller( GUCEF_NULL )
    , m_rows()
    , m_rowStoreIds()
    , m_selectedStoreId()
    , m_progressText()
    , m_progressPercent( 0.0f )
    , m_notice()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
    , m_updatingWidgets( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLibrarySyncForm::CLibrarySyncForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_controller( GUCEF_NULL )
    , m_rows()
    , m_rowStoreIds()
    , m_selectedStoreId()
    , m_progressText()
    , m_progressPercent( 0.0f )
    , m_notice()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
    , m_updatingWidgets( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLibrarySyncForm::~CLibrarySyncForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "LibrarySyncForm", "0.04,0 0.04,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Library sync" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_TitleHeight, "28" );
    CORE::CDataNode* c = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );

    // the stores, one row each
    CORE::CDataNode* grid = GUI::CFormLayout::AddWidgetNode( *c, "GridView", "Stores", "0,0 0,0", "1,0 1,-148" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_FitColumns, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_TextOverflow, "Ellipsis" );

    // what is done for the selected store
    struct SButton { const char* name; const char* text; const char* position; const char* size; };
    const SButton buttons[] =
    {
        { "Plan", "Plan", "0,8 1,-140", "0,90 0,28" }, { "SyncNow", "Sync now", "0,102 1,-140", "0,100 0,28" },
        { "Cancel", "Cancel", "0,206 1,-140", "0,90 0,28" }, { "RetryFailed", "Retry failed", "0,300 1,-140", "0,110 0,28" },
        { "Refresh", "Refresh", "0,414 1,-140", "0,90 0,28" }, { "Close", "Close", "1,-102 1,-140", "0,94 0,28" }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        CORE::CDataNode* button = GUI::CFormLayout::AddWidgetNode( *c, "Button", buttons[ i ].name, buttons[ i ].position, buttons[ i ].size );
        GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_Text, buttons[ i ].text );
    }

    // the progress of the sync of the selected store and the notice of the end of a sync
    CORE::CDataNode* bar = GUI::CFormLayout::AddWidgetNode( *c, "ProgressBar", "Progress", "0,8 1,-104", "1,-16 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *bar, GUI::EWidgetProperty_Min, "0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *bar, GUI::EWidgetProperty_Max, "100" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "ProgressText", "0,8 1,-72", "1,-16 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "Notice", "0,8 1,-38", "1,-16 0,28" );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::Build( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();
    struct SButton { const char* name; void ( CLibrarySyncForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "Plan", &CLibrarySyncForm::OnPlanActivated }, { "SyncNow", &CLibrarySyncForm::OnSyncActivated },
        { "Cancel", &CLibrarySyncForm::OnCancelActivated }, { "RetryFailed", &CLibrarySyncForm::OnRetryActivated },
        { "Refresh", &CLibrarySyncForm::OnRefreshActivated }, { "Close", &CLibrarySyncForm::OnCloseActivated }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
        {
            TEventCallback callback( this, buttons[ i ].handler );
            SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
        }
    }
    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Stores" );
    if ( GUCEF_NULL != grid )
    {
        grid->SetProperty( GUI::EWidgetProperty_Columns, g_storeColumns );
        GUCEF::vector< Int32 > widths;
        const Int32 weights[] = { 3, 2, 2, 2, 2, 3, 3, 3, 3, 3 };
        for ( UInt32 i=0; i<(UInt32) COLUMN_COUNT; ++i )
            widths.push_back( weights[ i ] );
        grid->SetIntListProperty( GUI::EWidgetProperty_ColumnWidths, widths );
        TEventCallback callback( this, &CLibrarySyncForm::OnSelectionChanged );
        SubscribeTo( grid, GUI::CGridView::SelectionChangedEvent, callback );
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CLibrarySyncForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    ShowRows();
    ShowSelected();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    RefreshNow();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::Close( void )
{GUCEF_TRACE;

    const bool wasOpen = m_open;
    m_open = false;
    if ( GUCEF_NULL != GetRootWidget() )
        Hide();
    if ( wasOpen )
        NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::SetController( CLibrarySyncController* controller )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        UnsubscribeFrom( m_controller );
    m_controller = controller;
    if ( GUCEF_NULL != m_controller )
    {
        TEventCallback changed( this, &CLibrarySyncForm::OnControllerChanged );
        SubscribeTo( m_controller, CLibrarySyncController::StoresChangedEvent, changed );
        SubscribeTo( m_controller, CLibrarySyncController::StoreChangedEvent, changed );
        SubscribeTo( m_controller, CLibrarySyncController::PlanReadyEvent, changed );
        TEventCallback notified( this, &CLibrarySyncForm::OnNotification );
        SubscribeTo( m_controller, CLibrarySyncController::NotificationEvent, notified );
    }
    m_dirty = true;
    ShowRows();
    ShowSelected();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

const CLibrarySyncController::SStoreState*
CLibrarySyncForm::GetSelectedState( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_selectedStoreId.IsNULLOrEmpty() )
        return GUCEF_NULL;
    return m_controller->FindStore( m_selectedStoreId );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::SelectStore( const CORE::CString& storeId )
{GUCEF_TRACE;

    for ( size_t r=0; r<m_rowStoreIds.size(); ++r )
    {
        if ( m_rowStoreIds[ r ] != storeId )
            continue;
        m_selectedStoreId = storeId;
        GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Stores" );
        if ( GUCEF_NULL != grid )
        {
            m_updatingWidgets = true;
            grid->SetSelectedRow( (Int32) r );
            m_updatingWidgets = false;
        }
        ShowSelected();
        ShowButtons();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncForm::GetSelectedStoreId( void ) const
{GUCEF_TRACE;

    return m_selectedStoreId;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::TakeProblem( bool asked )
{GUCEF_TRACE;

    if ( !asked && GUCEF_NULL != m_controller )
        m_notice = m_controller->GetProblem();
    ShowRows();
    ShowSelected();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::PlanNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_selectedStoreId.IsNULLOrEmpty() )
        return false;
    m_notice.Clear();
    const bool asked = m_controller->RequestPlan( m_selectedStoreId );
    TakeProblem( asked );
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::SyncNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_selectedStoreId.IsNULLOrEmpty() )
        return false;
    m_notice.Clear();
    const bool asked = m_controller->StartSync( m_selectedStoreId );
    TakeProblem( asked );
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::CancelSync( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_selectedStoreId.IsNULLOrEmpty() )
        return false;
    const bool asked = m_controller->Cancel( m_selectedStoreId );
    TakeProblem( asked );
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::RetryFailed( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_selectedStoreId.IsNULLOrEmpty() )
        return false;
    m_notice.Clear();
    const bool asked = m_controller->RetryFailed( m_selectedStoreId );
    TakeProblem( asked );
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::RefreshNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller )
        return false;
    const bool asked = m_controller->RefreshStores();
    TakeProblem( asked );
    return asked;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::ShowRows( void )
{GUCEF_TRACE;

    m_rows.clear();
    m_rowStoreIds.clear();
    if ( GUCEF_NULL != m_controller )
    {
        for ( UInt32 i=0; i<m_controller->GetStoreCount(); ++i )
        {
            const CLibrarySyncController::SStoreState& state = *m_controller->GetStore( i );
            GUCEF::vector< CORE::CString > row;
            row.push_back( state.displayName );
            row.push_back( state.signedIn ? CORE::CString( "signed in" ) : CORE::CString( "sign in required" ) );
            const bool counts = state.planned;
            row.push_back( counts ? CORE::UInt32ToString( state.missing ) : CORE::CString( state.planning ? "planning" : "" ) );
            row.push_back( counts ? CORE::UInt32ToString( state.stale ) : CORE::CString::Empty );
            row.push_back( counts ? CORE::UInt32ToString( state.current ) : CORE::CString::Empty );
            row.push_back( counts ? CORE::UInt32ToString( state.unavailable ) : CORE::CString::Empty );
            row.push_back( counts ? CLibrarySyncController::FormatBytes( state.estimatedBytes ) : CORE::CString::Empty );
            row.push_back( counts && state.freeBytes > 0 ? CLibrarySyncController::FormatBytes( state.freeBytes ) +
                           ( "does-not-fit" == state.storageVerdict ? CORE::CString( " (too little)" ) : CORE::CString::Empty ) : CORE::CString::Empty );
            CORE::CString progress;
            if ( state.running )
                progress = CORE::UInt32ToString( state.itemsDone ) + " of " + CORE::UInt32ToString( state.itemsTotal );
            row.push_back( progress );
            row.push_back( state.problem.IsNULLOrEmpty() ? state.lastResult : state.problem );
            m_rows.push_back( row );
            m_rowStoreIds.push_back( state.storeId );
        }
    }
    if ( m_selectedStoreId.IsNULLOrEmpty() && !m_rowStoreIds.empty() )
        m_selectedStoreId = m_rowStoreIds[ 0 ];
    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Stores" );
    if ( GUCEF_NULL != grid )
    {
        GUI::CGridView::TRowVector rows;
        Int32 selected = -1;
        for ( size_t r=0; r<m_rows.size(); ++r )
        {
            GUI::CGridView::TStringVector cells;
            for ( size_t c=0; c<m_rows[ r ].size(); ++c )
                cells.push_back( m_rows[ r ][ c ].ReplaceChar( '\n', ' ' ).ReplaceChar( '\t', ' ' ) );
            rows.push_back( cells );
            if ( m_rowStoreIds[ r ] == m_selectedStoreId )
                selected = (Int32) r;
        }
        m_updatingWidgets = true;
        grid->SetRows( rows );
        if ( selected >= 0 )
            grid->SetSelectedRow( selected );
        m_updatingWidgets = false;
    }
    m_dirty = false;
    m_waitedInMs = 0;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::ShowSelected( void )
{GUCEF_TRACE;

    const CLibrarySyncController::SStoreState* state = GetSelectedState();
    m_progressText.Clear();
    m_progressPercent = 0.0f;
    if ( GUCEF_NULL != state )
    {
        if ( state->running )
        {
            m_progressPercent = state->itemsTotal > 0 ? 100.0f * (Float32) state->itemsDone / (Float32) state->itemsTotal : 100.0f * state->fraction;
            m_progressText = state->displayName + ": " + CORE::UInt32ToString( state->itemsDone ) + " of " + CORE::UInt32ToString( state->itemsTotal ) +
                             " downloads done" + ( state->failed > 0 ? ", " + CORE::UInt32ToString( state->failed ) + " failed" : CORE::CString::Empty ) +
                             ( state->progressText.IsNULLOrEmpty() ? CORE::CString::Empty : "; " + state->progressText );
        }
        else
        if ( state->planning )
            m_progressText = state->displayName + ": planning" + ( state->progressText.IsNULLOrEmpty() ? CORE::CString::Empty : "; " + state->progressText );
        else
        if ( state->planned )
        {
            m_progressPercent = 0.0f;
            m_progressText = state->displayName + ": " + CORE::UInt32ToString( state->missing ) + " missing, " + CORE::UInt32ToString( state->stale ) + " stale, " +
                             CORE::UInt32ToString( state->queuedDownloads ) + " download(s) of about " + CLibrarySyncController::FormatBytes( state->estimatedBytes ) +
                             ( state->freeBytes > 0 ? ", " + CLibrarySyncController::FormatBytes( state->freeBytes ) + " free" : CORE::CString::Empty );
        }
        else
        if ( !state->lastResult.IsNULLOrEmpty() )
        {
            m_progressPercent = 100.0f;
            m_progressText = state->displayName + ": " + state->lastResult;
        }
    }
    GUI::CProgressBar* bar = GetWidgetAs< GUI::CProgressBar >( "Progress" );
    if ( GUCEF_NULL != bar )
        bar->SetValue( m_progressPercent );
    SetText( "ProgressText", m_progressText );
    SetText( "Notice", m_notice );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::IsSyncEnabled( void ) const
{GUCEF_TRACE;

    const CLibrarySyncController::SStoreState* state = GetSelectedState();
    return GUCEF_NULL != state && !state->running;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::ShowButtons( void )
{GUCEF_TRACE;

    const CLibrarySyncController::SStoreState* state = GetSelectedState();
    const bool idle = GUCEF_NULL != state && !state->running && !state->planning;
    struct SEnabled { const char* name; bool enabled; };
    const SEnabled buttons[] =
    {
        { "Plan", idle }, { "SyncNow", IsSyncEnabled() },
        { "Cancel", GUCEF_NULL != state && ( state->running || state->planning ) },
        { "RetryFailed", idle && state->retryAvailable && !state->runJobId.IsNULLOrEmpty() }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
            button->SetBoolProperty( GUI::EWidgetProperty_Enabled, buttons[ i ].enabled );
    }
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::SetText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        m_controller->Update();
    m_waitedInMs += deltaInMs;
    if ( m_open && m_dirty && m_waitedInMs >= (UInt32) RefreshIntervalInMs )
    {
        ShowRows();
        ShowSelected();
        ShowButtons();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CLibrarySyncForm::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_rows.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncForm::GetCellText( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return row < (UInt32) m_rows.size() && column < (UInt32) m_rows[ row ].size() ? m_rows[ row ][ column ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncForm::GetProgressText( void ) const
{GUCEF_TRACE;

    return m_progressText;
}

/*-------------------------------------------------------------------------*/

Float32
CLibrarySyncForm::GetProgressPercent( void ) const
{GUCEF_TRACE;

    return m_progressPercent;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncForm::GetNoticeText( void ) const
{GUCEF_TRACE;

    return m_notice;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::OnControllerChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // shown at the next refresh: the events of a sync come faster than a person reads
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::OnNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != text )
        m_notice = text->GetData();
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncForm::OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    const GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Stores" );
    const Int32 row = GUCEF_NULL != grid ? grid->GetSelectedRow() : -1;
    if ( row < 0 || row >= (Int32) m_rowStoreIds.size() )
        return;
    m_selectedStoreId = m_rowStoreIds[ row ];
    ShowSelected();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void CLibrarySyncForm::OnPlanActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    PlanNow();
}
void CLibrarySyncForm::OnSyncActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    SyncNow();
}
void CLibrarySyncForm::OnCancelActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    CancelSync();
}
void CLibrarySyncForm::OnRetryActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RetryFailed();
}
void CLibrarySyncForm::OnRefreshActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RefreshNow();
}
void CLibrarySyncForm::OnCloseActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLibrarySyncForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CLibrarySyncForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
