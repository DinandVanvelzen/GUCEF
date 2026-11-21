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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

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

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H
#include "AssetWorkbenchLib_CWorkbenchJobReport.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H
#include "AssetWorkbenchLib_CWorkbenchJobsForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchJobsForm::ClosedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchJobsForm::ClosedEvent";
const CORE::CEvent CWorkbenchJobsForm::OpenResultRequestedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchJobsForm::OpenResultRequestedEvent";
const CORE::CEvent CWorkbenchJobsForm::ReportShownEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchJobsForm::ReportShownEvent";

namespace {

const char* const g_columnTitles[] = { "Kind", "Scope", "State", "Progress", "Started", "Ended", "Duration", "Result" };
const char* const g_columnWidths[] = { "8", "14", "11", "9", "12", "12", "9", "18" };

}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
    OpenResultRequestedEvent.Initialize();
    ReportShownEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobsForm::CWorkbenchJobsForm( void )
    : GUI::CFormEx()
    , m_model( GUCEF_NULL )
    , m_service( GUCEF_NULL )
    , m_filter()
    , m_shown()
    , m_rows()
    , m_selectedId()
    , m_lastOutcome()
    , m_reportJobId()
    , m_reportForm( GUCEF_NULL )
    , m_reportFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_reportFutureJobId()
    , m_waitedInMs( 0 )
    , m_dirty( true )
    , m_hasActive( false )
    , m_open( false )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobsForm::CWorkbenchJobsForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_model( GUCEF_NULL )
    , m_service( GUCEF_NULL )
    , m_filter()
    , m_shown()
    , m_rows()
    , m_selectedId()
    , m_lastOutcome()
    , m_reportJobId()
    , m_reportForm( GUCEF_NULL )
    , m_reportFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_reportFutureJobId()
    , m_waitedInMs( 0 )
    , m_dirty( true )
    , m_hasActive( false )
    , m_open( false )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobsForm::~CWorkbenchJobsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_reportForm;
    m_reportForm = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::GetColumnTitle( EColumn column )
{GUCEF_TRACE;

    return (UInt32) column < (UInt32) COLUMN_COUNT ? CORE::CString( g_columnTitles[ column ] ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "JobsForm", "0.02,0 0.04,0", "0.96,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Jobs (times are UTC)" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_TitleHeight, "28" );
    CORE::CDataNode* content = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );

    // the filters
    CORE::CDataNode* showLabel = GUI::CFormLayout::AddWidgetNode( *content, "Label", "StateLabel", "0,8 0,8", "0,48 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *showLabel, GUI::EWidgetProperty_Text, "Show" );
    GUI::CFormLayout::AddWidgetNode( *content, "Combobox", "StateFilter", "0,60 0,4", "0,170 0,28" );
    CORE::CDataNode* kindLabel = GUI::CFormLayout::AddWidgetNode( *content, "Label", "KindLabel", "0,246 0,8", "0,44 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *kindLabel, GUI::EWidgetProperty_Text, "Kind" );
    GUI::CFormLayout::AddWidgetNode( *content, "Combobox", "KindFilter", "0,296 0,4", "0,170 0,28" );

    // what can be done with the selected job
    static const char* const names[] = { "Cancel", "Resume", "Retry", "Remove", "Report", "Open", "Close" };
    static const char* const texts[] = { "Cancel", "Resume", "Retry failed", "Remove", "Report", "Open result", "Close" };
    static const char* const positions[] = { "0,8 0,40", "0,102 0,40", "0,196 0,40", "0,310 0,40", "0,404 0,40", "0,498 0,40", "1,-102 0,40" };
    static const char* const sizes[] = { "0,86 0,28", "0,86 0,28", "0,106 0,28", "0,86 0,28", "0,86 0,28", "0,116 0,28", "0,94 0,28" };
    for ( UInt32 i=0; i<7; ++i )
    {
        CORE::CDataNode* button = GUI::CFormLayout::AddWidgetNode( *content, "Button", names[ i ], positions[ i ], sizes[ i ] );
        GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_Text, texts[ i ] );
    }

    // the jobs, then the line of counts and the details
    CORE::CDataNode* grid = GUI::CFormLayout::AddWidgetNode( *content, "GridView", "Jobs", "0,0 0,76", "1,0 0.66,-84" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_FitColumns, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_TextOverflow, "Ellipsis" );
    GUI::CFormLayout::AddWidgetNode( *content, "Label", "Status", "0,6 0.66,-4", "1,-12 0,24" );
    CORE::CDataNode* details = GUI::CFormLayout::AddWidgetNode( *content, "Textbox", "Details", "0,0 0.66,24", "1,0 0.34,-24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *details, GUI::EWidgetProperty_ReadOnly, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *details, GUI::EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::Build( void )
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
CWorkbenchJobsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    struct SButton { const char* name; void ( CWorkbenchJobsForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "Cancel", &CWorkbenchJobsForm::OnCancelActivated }, { "Resume", &CWorkbenchJobsForm::OnResumeActivated },
        { "Retry", &CWorkbenchJobsForm::OnRetryActivated }, { "Remove", &CWorkbenchJobsForm::OnRemoveActivated },
        { "Report", &CWorkbenchJobsForm::OnReportActivated }, { "Open", &CWorkbenchJobsForm::OnOpenActivated },
        { "Close", &CWorkbenchJobsForm::OnCloseActivated }
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
    const char* const combos[] = { "StateFilter", "KindFilter" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* combo = GetWidget( combos[ i ] );
        if ( GUCEF_NULL != combo )
        {
            TEventCallback callback( this, &CWorkbenchJobsForm::OnFilterChanged );
            SubscribeTo( combo, GUI::CWidget::ValueChangedEvent, callback );
        }
    }
    GUI::CWidget* grid = GetWidget( "Jobs" );
    if ( GUCEF_NULL != grid )
    {
        TEventCallback callback( this, &CWorkbenchJobsForm::OnSelectionChanged );
        SubscribeTo( grid, GUI::CGridView::SelectionChangedEvent, callback );
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CWorkbenchJobsForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    GUI::CGridView* gridView = GetWidgetAs< GUI::CGridView >( "Jobs" );
    if ( GUCEF_NULL != gridView )
    {
        CORE::CString titles;
        GUCEF::vector< Int32 > widths;
        for ( UInt32 c=0; c<(UInt32) COLUMN_COUNT; ++c )
        {
            titles += ( c > 0 ? "\n" : "" ) + CORE::CString( g_columnTitles[ c ] );
            widths.push_back( CORE::StringToInt32( g_columnWidths[ c ], 1 ) );
        }
        gridView->SetProperty( GUI::EWidgetProperty_Columns, titles );
        gridView->SetIntListProperty( GUI::EWidgetProperty_ColumnWidths, widths );
    }
    ShowChoices();
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::ShowChoices( void )
{GUCEF_TRACE;

    m_updatingWidgets = true;
    GUI::CCombobox* state = GetWidgetAs< GUI::CCombobox >( "StateFilter" );
    if ( GUCEF_NULL != state )
    {
        GUI::CCombobox::TStringVector items;
        for ( UInt32 i=0; i<(UInt32) CWorkbenchJobFilter::STATES_COUNT; ++i )
        {
            CORE::CString name = CWorkbenchJobFilter::GetStateGroupName( (CWorkbenchJobFilter::EStateGroup) i );
            items.push_back( name.SubstrToIndex( 1, true ).Uppercase() + name.CutChars( 1, true ) );
        }
        state->SetListItems( items );
        state->SetSelectedIndex( (Int32) m_filter.states );
    }
    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "KindFilter" );
    if ( GUCEF_NULL != kind )
    {
        GUI::CCombobox::TStringVector items;
        items.push_back( "All kinds" );
        CORE::CString::StringVector kinds;
        CWorkbenchJobFilter::GetKnownKinds( kinds );
        Int32 selected = 0;
        for ( size_t i=0; i<kinds.size(); ++i )
        {
            items.push_back( kinds[ i ] );
            if ( kinds[ i ] == m_filter.kind )
                selected = (Int32) i + 1;
        }
        kind->SetListItems( items );
        kind->SetSelectedIndex( selected );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::Close( void )
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
CWorkbenchJobsForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::SetModel( CAssetStoreTransferModel* model )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_model )
        UnsubscribeFrom( m_model, GUI::CITransferQueueModel::EntriesChangedEvent );
    m_model = model;
    if ( GUCEF_NULL != m_model )
    {
        TEventCallback callback( this, &CWorkbenchJobsForm::OnModelChanged );
        SubscribeTo( m_model, GUI::CITransferQueueModel::EntriesChangedEvent, callback );
    }
    m_dirty = true;
    if ( m_open )
        Refresh();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::SetService( ASSETSTORE::CIAssetStoreService* service )
{GUCEF_TRACE;

    m_service = service;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::SetStateGroup( CWorkbenchJobFilter::EStateGroup group )
{GUCEF_TRACE;

    m_filter.states = group;
    ShowChoices();
    m_dirty = true;
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::SetKindFilter( const CORE::CString& kind )
{GUCEF_TRACE;

    m_filter.kind = kind;
    ShowChoices();
    m_dirty = true;
    Refresh();
}

/*-------------------------------------------------------------------------*/

const CWorkbenchJobFilter&
CWorkbenchJobsForm::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::FormatDuration( UInt64 milliseconds )
{GUCEF_TRACE;

    if ( milliseconds < 1000 )
        return CORE::UInt64ToString( milliseconds ) + " ms";
    const UInt64 seconds = milliseconds / 1000;
    if ( seconds < 60 )
        return CORE::UInt64ToString( seconds ) + "." + CORE::UInt64ToString( ( milliseconds % 1000 ) / 100 ) + " s";
    const UInt64 minutes = seconds / 60;
    if ( minutes < 60 )
        return CORE::UInt64ToString( minutes ) + " min " + CORE::UInt64ToString( seconds % 60 ) + " s";
    return CORE::UInt64ToString( minutes / 60 ) + " h " + CORE::UInt64ToString( minutes % 60 ) + " min";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::FormatTime( Int64 unixMilliseconds )
{GUCEF_TRACE;

    if ( unixMilliseconds <= 0 )
        return CORE::CString::Empty;
    CORE::CDateTime time;
    time.FromUnixEpochBasedTicksInMillisecs( (UInt64) unixMilliseconds );
    // "11-21 03:39:22": the month and day and the time of day, UTC, short enough for the column
    CORE::CString text;
    const Int32 parts[] = { (Int32) time.GetMonth(), (Int32) time.GetDay(), (Int32) time.GetHours(), (Int32) time.GetMinutes(), (Int32) time.GetSeconds() };
    const char* const separators[] = { "-", " ", ":", ":", "" };
    for ( UInt32 p=0; p<5; ++p )
        text += ( parts[ p ] < 10 ? "0" : "" ) + CORE::Int32ToString( parts[ p ] ) + separators[ p ];
    return text;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::MakeRow( const ASSETSTORE::CAssetJobEntry& job, GUCEF::vector< CORE::CString >& cells ) const
{GUCEF_TRACE;

    cells.clear();
    cells.push_back( CWorkbenchJobFilter::DisplayKindOf( job ) );
    cells.push_back( job.scope.IsNULLOrEmpty() ? job.name : job.scope );
    cells.push_back( CWorkbenchJobFilter::StateTextOf( job ) );
    Float32 progress = job.fraction;
    if ( progress < 0.0f && job.totalBytes > 0 )
        progress = (Float32) job.doneBytes / (Float32) job.totalBytes;
    if ( progress < 0.0f && job.itemsTotal > 0 )
        progress = (Float32) job.itemsDone / (Float32) job.itemsTotal;
    if ( ASSETSTORE::ASSETJOB_STATE_DONE == job.state )
        progress = 1.0f;
    cells.push_back( progress < 0.0f ? CORE::CString( "-" ) : CORE::UInt32ToString( (UInt32) ( progress * 100.0f + 0.5f ) ) + " %" );
    cells.push_back( FormatTime( job.createdAtMs ) );
    cells.push_back( job.IsEnded() ? FormatTime( job.updatedAtMs ) : CORE::CString::Empty );
    const Int64 end = job.updatedAtMs > job.createdAtMs ? job.updatedAtMs : job.createdAtMs;
    cells.push_back( job.createdAtMs > 0 ? FormatDuration( (UInt64) ( end - job.createdAtMs ) ) : CORE::CString::Empty );
    CORE::CString result = job.resultText;
    if ( result.IsNULLOrEmpty() && job.IsEnded() )
        result = job.message;
    if ( result.IsNULLOrEmpty() && !job.IsEnded() )
        result = job.message;
    cells.push_back( result.ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' ).ReplaceChar( '\t', ' ' ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::Refresh( void )
{GUCEF_TRACE;

    m_dirty = false;
    m_waitedInMs = 0;
    m_shown.clear();
    m_rows.clear();
    m_hasActive = false;
    if ( GUCEF_NULL != m_model )
    {
        ASSETSTORE::TAssetJobEntryVector jobs;
        m_model->GetJobs( jobs );
        // the newest job first
        for ( size_t i=jobs.size(); i>0; --i )
        {
            if ( !jobs[ i - 1 ].IsEnded() )
                m_hasActive = true;
            if ( m_filter.Matches( jobs[ i - 1 ] ) )
                m_shown.push_back( jobs[ i - 1 ] );
        }
    }
    bool selectedStillShown = false;
    for ( size_t i=0; i<m_shown.size(); ++i )
    {
        GUCEF::vector< CORE::CString > cells;
        MakeRow( m_shown[ i ], cells );
        m_rows.push_back( cells );
        if ( m_shown[ i ].jobId == m_selectedId )
            selectedStillShown = true;
    }
    if ( !selectedStillShown )
        m_selectedId.Clear();
    ShowRows();
    ShowStatusAndButtons();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::ShowRows( void )
{GUCEF_TRACE;

    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Jobs" );
    if ( GUCEF_NULL == grid )
        return;
    GUI::CGridView::TRowVector rows;
    CORE::CString colours;
    Int32 selectedRow = -1;
    for ( size_t r=0; r<m_rows.size(); ++r )
    {
        GUI::CGridView::TStringVector cells;
        for ( size_t c=0; c<m_rows[ r ].size(); ++c )
            cells.push_back( m_rows[ r ][ c ] );
        rows.push_back( cells );
        const ASSETSTORE::TAssetJobState state = m_shown[ r ].state;
        colours += ( r > 0 ? "\n" : "" ) + CORE::CString( ASSETSTORE::ASSETJOB_STATE_FAILED == state ? "255,225,225,255" :
                                                          ( ASSETSTORE::ASSETJOB_STATE_CANCELLED == state ? "220,220,220,255" :
                                                          ( m_shown[ r ].IsEnded() ? "235,250,235,255" : "225,240,255,255" ) ) );
        if ( m_shown[ r ].jobId == m_selectedId )
            selectedRow = (Int32) r;
    }
    m_updatingWidgets = true;
    grid->SetRows( rows );
    grid->SetProperty( GUI::EWidgetProperty_ItemColours, colours );
    if ( selectedRow >= 0 )
        grid->SetSelectedRow( selectedRow );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::GetStatusText( void ) const
{GUCEF_TRACE;

    CWorkbenchJobSummary summary;
    UInt32 listed = 0;
    if ( GUCEF_NULL != m_model )
    {
        ASSETSTORE::TAssetJobEntryVector jobs;
        m_model->GetJobs( jobs );
        summary = CWorkbenchJobSummary::Count( jobs );
        listed = (UInt32) jobs.size();
    }
    CORE::CString text = CORE::UInt32ToString( (UInt32) m_shown.size() ) + " of " + CORE::UInt32ToString( listed ) + " shown, " +
                         CORE::UInt32ToString( summary.running ) + " running, " + CORE::UInt32ToString( summary.queued ) + " queued, " +
                         CORE::UInt32ToString( summary.failed ) + " failed, " + CORE::UInt32ToString( summary.done ) + " done";
    if ( !m_lastOutcome.IsNULLOrEmpty() )
        text += " | " + m_lastOutcome;
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::GetDetailsText( void ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = FindShown( m_selectedId );
    if ( GUCEF_NULL == job )
        return CORE::CString::Empty;
    CORE::CString text = CWorkbenchJobFilter::DisplayKindOf( *job ) + " " + job->name + ": " + CWorkbenchJobFilter::StateTextOf( *job ) + "\n";
    if ( !job->scope.IsNULLOrEmpty() )
        text += "Scope: " + job->scope + "\n";
    text += "Id: " + job->jobId + "\n";
    if ( !job->detail.IsNULLOrEmpty() )
        text += "Detail: " + job->detail + "\n";
    if ( job->itemsTotal > 0 )
        text += "Assets: " + CORE::UInt32ToString( job->itemsDone ) + " of " + CORE::UInt32ToString( job->itemsTotal ) + "\n";
    if ( !job->resultText.IsNULLOrEmpty() )
        text += "Result: " + job->resultText + "\n";
    if ( !job->message.IsNULLOrEmpty() )
        text += "Message: " + job->message + "\n";
    if ( !job->folder.IsNULLOrEmpty() )
        text += "Result folder: " + job->folder + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::ShowStatusAndButtons( void )
{GUCEF_TRACE;

    GUI::CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
        status->SetProperty( GUI::EWidgetProperty_Text, GetStatusText() );
    GUI::CWidget* details = GetWidget( "Details" );
    if ( GUCEF_NULL != details )
        details->SetProperty( GUI::EWidgetProperty_Text, GetDetailsText() );
    GUI::CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
        cancel->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
    GUI::CWidget* resume = GetWidget( "Resume" );
    if ( GUCEF_NULL != resume )
        resume->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
    GUI::CWidget* retry = GetWidget( "Retry" );
    if ( GUCEF_NULL != retry )
        retry->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
    GUI::CWidget* remove = GetWidget( "Remove" );
    if ( GUCEF_NULL != remove )
        remove->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
    GUI::CWidget* report = GetWidget( "Report" );
    if ( GUCEF_NULL != report )
        report->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsReportAvailable() );
    GUI::CWidget* open = GetWidget( "Open" );
    if ( GUCEF_NULL != open )
    {
        const ASSETSTORE::CAssetJobEntry* job = FindShown( m_selectedId );
        open->SetBoolProperty( GUI::EWidgetProperty_Enabled, GUCEF_NULL != job && job->IsEnded() && !job->folder.IsNULLOrEmpty() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_open )
        return false;
    if ( m_reportFuture.HasAFuture() && m_reportFuture.IsReady() )
    {
        ASSETSTORE::CAssetServiceDocument result;
        const CORE::CString jobId = m_reportFutureJobId;
        if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_reportFuture, result ) )
            TakeReport( result, jobId );
        else
            m_lastOutcome = "The report of " + jobId + " could not be read";
        m_reportFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        ShowStatusAndButtons();
    }
    m_waitedInMs += deltaInMs;
    if ( ( m_dirty && m_waitedInMs >= (UInt32) RefreshIntervalInMs ) || ( m_hasActive && m_waitedInMs >= (UInt32) ActiveRefreshIntervalInMs ) )
    {
        Refresh();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobsForm::GetShownJobCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_shown.size();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::GetShownJob( UInt32 row, ASSETSTORE::CAssetJobEntry& job ) const
{GUCEF_TRACE;

    if ( row >= (UInt32) m_shown.size() )
        return false;
    job = m_shown[ row ];
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::GetCellText( UInt32 row, EColumn column ) const
{GUCEF_TRACE;

    if ( row >= (UInt32) m_rows.size() || (UInt32) column >= (UInt32) m_rows[ row ].size() )
        return CORE::CString::Empty;
    return m_rows[ row ][ column ];
}

/*-------------------------------------------------------------------------*/

const ASSETSTORE::CAssetJobEntry*
CWorkbenchJobsForm::FindShown( const CORE::CString& jobId ) const
{GUCEF_TRACE;

    if ( jobId.IsNULLOrEmpty() )
        return GUCEF_NULL;
    for ( size_t i=0; i<m_shown.size(); ++i )
        if ( m_shown[ i ].jobId == jobId )
            return &m_shown[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::SelectJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == FindShown( jobId ) )
        return false;
    m_selectedId = jobId;
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobsForm::GetSelectedJobId( void ) const
{GUCEF_TRACE;

    return m_selectedId;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::IsActionEnabled( ASSETSTORE::TAssetJobAction action ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = FindShown( m_selectedId );
    return GUCEF_NULL != job && job->IsActionAvailable( action );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::Control( ASSETSTORE::TAssetJobAction action )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_model || !IsActionEnabled( action ) )
        return false;
    const CORE::CString jobId = m_selectedId;
    const bool done = m_model->ControlJob( jobId, action );
    m_lastOutcome = CORE::CString( ASSETSTORE::AssetJobActionToString( action ) ) + " of " + jobId + ( done ? ": sent" : ": refused" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JobsForm: " << m_lastOutcome );
    m_dirty = true;
    Refresh();
    return done;
}

/*-------------------------------------------------------------------------*/

bool CWorkbenchJobsForm::CancelSelected( void )
{GUCEF_TRACE;

    return Control( ASSETSTORE::ASSETJOB_ACTION_CANCEL );
}
bool CWorkbenchJobsForm::ResumeSelected( void )
{GUCEF_TRACE;

    return Control( ASSETSTORE::ASSETJOB_ACTION_RESUME );
}
bool CWorkbenchJobsForm::RetrySelected( void )
{GUCEF_TRACE;

    return Control( ASSETSTORE::ASSETJOB_ACTION_RETRY );
}
bool CWorkbenchJobsForm::RemoveSelected( void )
{GUCEF_TRACE;

    return Control( ASSETSTORE::ASSETJOB_ACTION_REMOVE );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::IsReportAvailable( void ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = FindShown( m_selectedId );
    return GUCEF_NULL != job && job->IsEnded() && GUCEF_NULL != m_service && !CWorkbenchJobRegistry::IsLocalId( job->jobId ) && "pipeline" == job->kind;
}

/*-------------------------------------------------------------------------*/

GUI::CReportViewForm*
CWorkbenchJobsForm::GetReportForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_reportForm && GUCEF_NULL != GetContext() )
    {
        m_reportForm = GUCEF_NEW GUI::CReportViewForm( GetContext() );
        if ( !m_reportForm->Build( "Job report" ) )
        {
            GUCEF_DELETE m_reportForm;
            m_reportForm = GUCEF_NULL;
        }
    }
    return m_reportForm;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchJobsForm::GetReportJobId( void ) const
{GUCEF_TRACE;

    return m_reportJobId;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::TakeReport( const ASSETSTORE::CAssetServiceDocument& result, const CORE::CString& jobId )
{GUCEF_TRACE;

    GUI::CReportViewForm* form = GetReportForm();
    if ( GUCEF_NULL == form )
    {
        m_lastOutcome = "The report cannot be shown";
        return;
    }
    GUI::CReportViewForm::TEntryVector entries;
    const UInt32 assets = CWorkbenchJobReport::BuildEntries( result, entries );
    form->SetEntries( entries );
    form->Refresh();
    form->Show();
    m_reportJobId = jobId;
    m_lastOutcome = "Report of " + jobId + ": " + CORE::UInt32ToString( assets ) + " assets";
    CORE::TCloneableString data( jobId );
    NotifyObservers( ReportShownEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::ShowReport( void )
{GUCEF_TRACE;

    if ( !IsReportAvailable() )
    {
        m_lastOutcome = "There is no report for this job";
        ShowStatusAndButtons();
        return false;
    }
    const CORE::CString jobId = m_selectedId;
    if ( m_service->IsInProcess() )
    {
        ASSETSTORE::CAssetServiceDocument result;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service->GetJobResult( jobId, CORE::CString::Empty, CORE::CString::Empty, 0, result ) )
        {
            m_lastOutcome = "The report of " + jobId + " cannot be read";
            ShowStatusAndButtons();
            return false;
        }
        TakeReport( result, jobId );
    }
    else
    {
        // asked: the report arrives with a later Update(), the interface does not wait for the server
        m_reportFutureJobId = jobId;
        m_reportFuture = m_service->GetJobResultAsync( jobId, CORE::CString::Empty, CORE::CString::Empty, 0 );
        if ( !m_reportFuture.HasAFuture() )
        {
            m_lastOutcome = "The report of " + jobId + " cannot be asked for";
            ShowStatusAndButtons();
            return false;
        }
        m_lastOutcome = "Reading the report of " + jobId;
    }
    ShowStatusAndButtons();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobsForm::OpenSelectedResult( void )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = FindShown( m_selectedId );
    if ( GUCEF_NULL == job || !job->IsEnded() || job->folder.IsNULLOrEmpty() )
        return false;
    CORE::TCloneableString data( job->jobId );
    NotifyObservers( OpenResultRequestedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::OnModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::OnFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CCombobox* state = GetWidgetAs< GUI::CCombobox >( "StateFilter" );
    if ( GUCEF_NULL != state && state->GetSelectedIndex() >= 0 && state->GetSelectedIndex() < (Int32) CWorkbenchJobFilter::STATES_COUNT )
        m_filter.states = (CWorkbenchJobFilter::EStateGroup) state->GetSelectedIndex();
    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "KindFilter" );
    if ( GUCEF_NULL != kind )
    {
        CORE::CString::StringVector kinds;
        CWorkbenchJobFilter::GetKnownKinds( kinds );
        const Int32 index = kind->GetSelectedIndex();
        m_filter.kind = index > 0 && index <= (Int32) kinds.size() ? kinds[ index - 1 ] : CORE::CString::Empty;
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobsForm::OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Jobs" );
    if ( GUCEF_NULL == grid )
        return;
    const Int32 row = grid->GetSelectedRow();
    m_selectedId = row >= 0 && row < (Int32) m_shown.size() ? m_shown[ row ].jobId : CORE::CString::Empty;
    ShowStatusAndButtons();
}

/*-------------------------------------------------------------------------*/

void CWorkbenchJobsForm::OnCancelActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    CancelSelected();
}
void CWorkbenchJobsForm::OnResumeActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    ResumeSelected();
}
void CWorkbenchJobsForm::OnRetryActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RetrySelected();
}
void CWorkbenchJobsForm::OnRemoveActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RemoveSelected();
}
void CWorkbenchJobsForm::OnReportActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    ShowReport();
}
void CWorkbenchJobsForm::OnOpenActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    OpenSelectedResult();
}
void CWorkbenchJobsForm::OnCloseActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchJobsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchJobsForm";
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
