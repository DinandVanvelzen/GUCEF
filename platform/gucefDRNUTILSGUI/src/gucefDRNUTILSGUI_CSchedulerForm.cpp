/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#include "gucefGUI_CTabContentPane.h"
#define GUCEF_GUI_CTABCONTENTPANE_H
#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILSGUI_CSchedulerForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CSchedulerForm::ClassTypeName = "GUCEF::DRNUTILSGUI::CSchedulerForm";
const CORE::CEvent CSchedulerForm::ClosedEvent = "GUCEF::DRNUTILSGUI::CSchedulerForm::ClosedEvent";

namespace {

const SGridColumnSpec g_jobColumns[] =
{
    { "jobId", "Job", 150 }, { "title", "Title", 200 }, { "lane", "Lane", 90 }, { "state", "State", 100 }, { "trigger", "Trigger", 160 },
    { "nextDueUtc", "Next due", 190 }, { "waitingFor", "Waiting for", 110 }, { "lastRunState", "Last run", 100 },
    { "lastRunEndedUtc", "Last run ended", 190 }, { "runCount", "Runs", 60 }, { "failureCount", "Failed", 60 }, { "activeRuns", "Active", 60 },
    { "executionTarget", "Target", 110 }, { "threadPool", "Pool", 110 }, { "taskType", "Task type", 160 }, { "origin", "Origin", 80 }
};

const SGridColumnSpec g_laneColumns[] =
{
    { "lane", "Lane", 130 }, { "mode", "Mode", 90 }, { "state", "State", 90 }, { "inFlight", "In flight", 70 }, { "maxConcurrent", "Max", 60 },
    { "queuedP0", "Queued P0", 80 }, { "queuedP1", "Queued P1", 80 }, { "queuedP2", "Queued P2", 80 }, { "queuedP3", "Queued P3", 80 },
    { "waitingFor", "Waiting for", 120 }, { "waitingUntilUtc", "Waiting until", 190 }, { "heldBy", "Held by", 120 }
};

const SGridColumnSpec g_runColumns[] =
{
    { "runId", "Run", 80 }, { "jobId", "Job", 140 }, { "state", "State", 100 }, { "cause", "Cause", 100 }, { "scheduledUtc", "Scheduled", 190 },
    { "startedUtc", "Started", 190 }, { "endedUtc", "Ended", 190 }, { "attempt", "Attempt", 60 }, { "progressDone", "Done", 60 },
    { "progressTotal", "Total", 60 }, { "executionTarget", "Target", 110 }, { "threadPool", "Pool", 110 }, { "taskType", "Task type", 140 },
    { "message", "Message", 240 }, { "requestedBy", "By", 100 }
};

template< class T, UInt32 N >
UInt32
CountOf( const T ( & )[ N ] )
{
    return N;
}

bool
IsJobStateRunnable( const CORE::CString& state )
{
    return "DISABLED" != state && "UNAVAILABLE" != state && "ENDED" != state;
}

bool
IsRunStateActive( const CORE::CString& state )
{
    return "QUEUED" == state || "WAITING" == state || "RUNNING" == state;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSchedulerForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSchedulerForm::CSchedulerForm( void )
    : GUI::CFormEx()
    , m_tables()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_jobsBinding()
    , m_lanesBinding()
    , m_runsBinding()
    , m_confirmForm( GUCEF_NULL )
    , m_historyForm( GUCEF_NULL )
    , m_pendingAction( CORE::JOBCONTROLACTION_UNSPECIFIED )
    , m_pendingTargetKind( CORE::JOBCONTROLTARGET_UNSPECIFIED )
    , m_pendingTargetId()
    , m_pendingText()
    , m_sentCommandId()
    , m_lastCommandText()
    , m_detailJobId()
    , m_detailRequestId()
    , m_haveDetail( false )
    , m_detail()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSchedulerForm::CSchedulerForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_tables()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_jobsBinding()
    , m_lanesBinding()
    , m_runsBinding()
    , m_confirmForm( GUCEF_NULL )
    , m_historyForm( GUCEF_NULL )
    , m_pendingAction( CORE::JOBCONTROLACTION_UNSPECIFIED )
    , m_pendingTargetKind( CORE::JOBCONTROLTARGET_UNSPECIFIED )
    , m_pendingTargetId()
    , m_pendingText()
    , m_sentCommandId()
    , m_lastCommandText()
    , m_detailJobId()
    , m_detailRequestId()
    , m_haveDetail( false )
    , m_detail()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSchedulerForm::~CSchedulerForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_jobsBinding.Unbind();
    m_lanesBinding.Unbind();
    m_runsBinding.Unbind();
    GUCEF_DELETE m_confirmForm;
    m_confirmForm = GUCEF_NULL;
    GUCEF_DELETE m_historyForm;
    m_historyForm = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "SchedulerForm", "0.04,0 0.04,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Scheduler" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,16 0,8", "1,-32 0,26" );

    CORE::CDataNode* tabs = GUI::CFormLayout::AddWidgetNode( *window, "TabControl", "Tabs", "0,16 0,40", "1,-32 0.5,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *tabs, GUI::EWidgetProperty_Tabs, "Jobs\nLanes\nRuns" );
    CORE::CDataNode* jobsPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "JobsPane" );
    GUI::CFormLayout::AddWidgetNode( *jobsPane, "GridView", "JobsGrid", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* lanesPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "LanesPane" );
    GUI::CFormLayout::AddWidgetNode( *lanesPane, "GridView", "LanesGrid", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* runsPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "RunsPane" );
    GUI::CFormLayout::AddWidgetNode( *runsPane, "GridView", "RunsGrid", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* runNow = GUI::CFormLayout::AddWidgetNode( *window, "Button", "RunNow", "0,16 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *runNow, GUI::EWidgetProperty_Text, "Run now" );
    CORE::CDataNode* cancelRun = GUI::CFormLayout::AddWidgetNode( *window, "Button", "CancelRun", "0,134 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancelRun, GUI::EWidgetProperty_Text, "Cancel run" );
    CORE::CDataNode* enable = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Enable", "0,252 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *enable, GUI::EWidgetProperty_Text, "Enable" );
    CORE::CDataNode* disable = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Disable", "0,370 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *disable, GUI::EWidgetProperty_Text, "Disable" );
    CORE::CDataNode* history = GUI::CFormLayout::AddWidgetNode( *window, "Button", "History", "0,488 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *history, GUI::EWidgetProperty_Text, "History" );
    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Close", "1,-126 0.5,52", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );

    CORE::CDataNode* details = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Details", "0,16 0.5,96", "1,-32 0.5,-132" );
    GUI::CFormLayout::SetWidgetNodeProperty( *details, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* result = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Result", "0,16 1,-34", "1,-32 0,26" );
    GUI::CFormLayout::SetWidgetNodeProperty( *result, GUI::EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::Build( void )
{GUCEF_TRACE;

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
CSchedulerForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    struct SButton { const char* name; void ( CSchedulerForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "RunNow", &CSchedulerForm::OnRunNowActivated }, { "CancelRun", &CSchedulerForm::OnCancelRunActivated },
        { "Enable", &CSchedulerForm::OnEnableActivated }, { "Disable", &CSchedulerForm::OnDisableActivated },
        { "History", &CSchedulerForm::OnHistoryActivated }, { "Close", &CSchedulerForm::OnCloseActivated }
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

    GUI::CWidget* tabs = GetWidget( "Tabs" );
    if ( GUCEF_NULL != tabs )
    {
        TEventCallback callback( this, &CSchedulerForm::OnActiveTabChanged );
        SubscribeTo( tabs, GUI::CTabControl::ActiveTabChangedEvent, callback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CSchedulerForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }

    CTableGridBinding* bindings[] = { &m_jobsBinding, &m_lanesBinding, &m_runsBinding };
    for ( UInt32 i=0; i<3; ++i )
    {
        TEventCallback changed( this, &CSchedulerForm::OnSelectionOrModelChanged );
        SubscribeTo( bindings[ i ], CTableGridBinding::ModelChangedEvent, changed );
        SubscribeTo( bindings[ i ], CTableGridBinding::SelectionChangedEvent, changed );
    }

    BindTables();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::BindTables( void )
{GUCEF_TRACE;

    m_jobsBinding.Bind( GetWidgetAs< GUI::CGridView >( "JobsGrid" ), m_tables.GetJobs(), "jobId", g_jobColumns, CountOf( g_jobColumns ), m_timeDisplay );
    m_lanesBinding.Bind( GetWidgetAs< GUI::CGridView >( "LanesGrid" ), m_tables.GetLanes(), "lane", g_laneColumns, CountOf( g_laneColumns ), m_timeDisplay );
    m_runsBinding.Bind( GetWidgetAs< GUI::CGridView >( "RunsGrid" ), m_tables.GetRuns(), "runId", g_runColumns, CountOf( g_runColumns ), m_timeDisplay );

    // the newest run first
    m_runsBinding.SortBy( "runId", false );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    if ( GUCEF_NULL != m_client )
        m_client->RequestInfo();
    RequestSelectedJobDetail( true );
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::Close( void )
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
CSchedulerForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SetTables( const DRNUTILS::CSchedulerTableSet& tables )
{GUCEF_TRACE;

    m_tables = tables;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        BindTables();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

const DRNUTILS::CSchedulerTableSet&
CSchedulerForm::GetTables( void ) const
{GUCEF_TRACE;

    return m_tables;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SetClient( DRNUTILS::CSchedulerClient* client )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        UnsubscribeFrom( m_client, DRNUTILS::CSchedulerClient::InfoReceivedEvent );
        UnsubscribeFrom( m_client, DRNUTILS::CSchedulerClient::CommandCompletedEvent );
        UnsubscribeFrom( m_client, DRNUTILS::CSchedulerClient::JobDetailReceivedEvent );
    }
    m_client = client;
    m_hostInfo = DRNUTILS::SSchedulerHostInfo();
    m_detailJobId.Clear();
    m_detailRequestId.Clear();
    m_haveDetail = false;
    if ( GUCEF_NULL != m_client )
    {
        TEventCallback infoCallback( this, &CSchedulerForm::OnInfoReceived );
        SubscribeTo( m_client, DRNUTILS::CSchedulerClient::InfoReceivedEvent, infoCallback );
        TEventCallback commandCallback( this, &CSchedulerForm::OnCommandCompleted );
        SubscribeTo( m_client, DRNUTILS::CSchedulerClient::CommandCompletedEvent, commandCallback );
        TEventCallback detailCallback( this, &CSchedulerForm::OnJobDetailReceived );
        SubscribeTo( m_client, DRNUTILS::CSchedulerClient::JobDetailReceivedEvent, detailCallback );
        m_hostInfo = m_client->GetLastInfo();
    }
    if ( m_historyForm != GUCEF_NULL )
    {
        m_historyForm->SetClient( m_client );
        m_historyForm->SetHostInfo( m_hostInfo );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CSchedulerClient*
CSchedulerForm::GetClient( void ) const
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SetTimeDisplay( const CTimeDisplaySettings& settings )
{GUCEF_TRACE;

    m_timeDisplay = settings;
    m_jobsBinding.SetTimeDisplay( settings );
    m_lanesBinding.SetTimeDisplay( settings );
    m_runsBinding.SetTimeDisplay( settings );
    if ( GUCEF_NULL != m_historyForm )
        m_historyForm->SetTimeDisplay( settings );
    Refresh();
}

/*-------------------------------------------------------------------------*/

const CTimeDisplaySettings&
CSchedulerForm::GetTimeDisplay( void ) const
{GUCEF_TRACE;

    return m_timeDisplay;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::ShowTab( TTab tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GetWidgetAs< GUI::CTabControl >( "Tabs" );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

CSchedulerForm::TTab
CSchedulerForm::GetShownTab( void ) const
{GUCEF_TRACE;

    const GUI::CTabControl* tabs = const_cast< CSchedulerForm* >( this )->GetWidgetAs< GUI::CTabControl >( "Tabs" );
    if ( GUCEF_NULL == tabs )
        return TAB_JOBS;
    const Int32 active = tabs->GetActiveTab();
    return active >= 0 && active <= (Int32) TAB_RUNS ? (TTab) active : TAB_JOBS;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SelectJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    if ( !m_jobsBinding.SelectKeyText( jobId ) )
        m_jobsBinding.ClearSelection();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::GetSelectedJobId( void ) const
{GUCEF_TRACE;

    return m_jobsBinding.GetSelectedKeyText();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SelectRun( UInt64 runId )
{GUCEF_TRACE;

    if ( !m_runsBinding.SelectKeyText( CORE::UInt64ToString( runId ) ) )
        m_runsBinding.ClearSelection();
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedulerForm::GetSelectedRunId( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_runsBinding.GetSelectedCellValue( "runId", value ) )
        return 0;
    return value.AsUInt64( 0, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::GetSelectedJobState( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_jobsBinding.GetSelectedCellValue( "state", value ) )
        return CORE::CString::Empty;
    return value.AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::GetSelectedRunState( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_runsBinding.GetSelectedCellValue( "state", value ) )
        return CORE::CString::Empty;
    return value.AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::IsActionAllowedByHost( CORE::TJobControlAction action ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_hostInfo.allowedActions.size(); ++i )
        if ( m_hostInfo.allowedActions[ i ].action == action )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::IsActionEnabled( CORE::TJobControlAction action ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !IsActionAllowedByHost( action ) )
        return false;

    switch ( action )
    {
        case CORE::JOBCONTROLACTION_RUN_NOW:
        {
            const CORE::CString state = GetSelectedJobState();
            return !GetSelectedJobId().IsNULLOrEmpty() && !state.IsNULLOrEmpty() && IsJobStateRunnable( state );
        }
        case CORE::JOBCONTROLACTION_CANCEL_RUN:
        {
            return 0 != GetSelectedRunId() && IsRunStateActive( GetSelectedRunState() );
        }
        case CORE::JOBCONTROLACTION_ENABLE:
        {
            return !GetSelectedJobId().IsNULLOrEmpty() && "DISABLED" == GetSelectedJobState();
        }
        case CORE::JOBCONTROLACTION_DISABLE:
        {
            const CORE::CString state = GetSelectedJobState();
            return !GetSelectedJobId().IsNULLOrEmpty() && !state.IsNULLOrEmpty() && "DISABLED" != state && "ENDED" != state;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::DescribeAction( CORE::TJobControlAction action ) const
{GUCEF_TRACE;

    switch ( action )
    {
        case CORE::JOBCONTROLACTION_RUN_NOW: return "Run now";
        case CORE::JOBCONTROLACTION_CANCEL_RUN: return "Cancel run";
        case CORE::JOBCONTROLACTION_ENABLE: return "Enable";
        case CORE::JOBCONTROLACTION_DISABLE: return "Disable";
        default: return CORE::JobControlActionToString( action );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::SendCommand( CORE::TJobControlAction action           ,
                             CORE::TJobControlTargetKind targetKind   ,
                             const CORE::CString& targetId            ,
                             const CORE::CString& reason              )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;

    CORE::SJobControlCommand command;
    command.action = action;
    command.targetKind = targetKind;
    command.targetId = targetId;
    command.reason = reason;

    m_pendingText = DescribeAction( action ) + " of " + ( CORE::JOBCONTROLTARGET_RUN == targetKind ? "run " : "job " ) + targetId;

    // asynchronous: the answer arrives as CommandCompletedEvent on the thread of the form
    const CORE::CString commandId = m_client->Control( command, false );
    if ( commandId.IsNULLOrEmpty() )
    {
        m_sentCommandId.Clear();
        m_lastCommandText = m_pendingText + " could not be sent";
        Refresh();
        return false;
    }
    m_sentCommandId = commandId;
    m_lastCommandText = m_pendingText + " sent";
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::RunSelectedJobNow( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsActionEnabled( CORE::JOBCONTROLACTION_RUN_NOW ) )
        return false;
    return SendCommand( CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, GetSelectedJobId(), reason );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::CancelSelectedRun( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsActionEnabled( CORE::JOBCONTROLACTION_CANCEL_RUN ) )
        return false;
    return SendCommand( CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLTARGET_RUN, CORE::UInt64ToString( GetSelectedRunId() ), reason );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::SetSelectedJobEnabled( bool enabled, const CORE::CString& reason )
{GUCEF_TRACE;

    const CORE::TJobControlAction action = enabled ? CORE::JOBCONTROLACTION_ENABLE : CORE::JOBCONTROLACTION_DISABLE;
    if ( !IsActionEnabled( action ) )
        return false;
    return SendCommand( action, CORE::JOBCONTROLTARGET_JOB, GetSelectedJobId(), reason );
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm*
CSchedulerForm::GetConfirmForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_confirmForm && GUCEF_NULL != GetContext() )
    {
        m_confirmForm = GUCEF_NEW CControlConfirmForm( GetContext() );
        TEventCallback callback( this, &CSchedulerForm::OnConfirmed );
        SubscribeTo( m_confirmForm, CControlConfirmForm::ConfirmedEvent, callback );
    }
    return m_confirmForm;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::AskToConfirm( CORE::TJobControlAction action )
{GUCEF_TRACE;

    if ( !IsActionEnabled( action ) )
        return false;
    CControlConfirmForm* confirm = GetConfirmForm();
    if ( GUCEF_NULL == confirm )
        return false;

    const bool onRun = CORE::JOBCONTROLACTION_CANCEL_RUN == action;
    m_pendingAction = action;
    m_pendingTargetKind = onRun ? CORE::JOBCONTROLTARGET_RUN : CORE::JOBCONTROLTARGET_JOB;
    m_pendingTargetId = onRun ? CORE::UInt64ToString( GetSelectedRunId() ) : GetSelectedJobId();

    CORE::CString targetText = onRun ? "Run " + m_pendingTargetId + " of job " : "Job ";
    CORE::CVariant value;
    if ( onRun )
    {
        if ( m_runsBinding.GetSelectedCellValue( "jobId", value ) )
            targetText += value.AsString( CORE::CString::Empty, false );
    }
    else
    {
        targetText += m_pendingTargetId;
        if ( m_jobsBinding.GetSelectedCellValue( "title", value ) && !value.AsString( CORE::CString::Empty, false ).IsNULLOrEmpty() )
            targetText += " (" + value.AsString( CORE::CString::Empty, false ) + ")";
    }
    return confirm->Ask( DescribeAction( action ), DescribeAction( action ) + " on the node " + m_hostInfo.hostName, targetText );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnConfirmed( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CControlConfirmForm::TConfirmedEventData* reason = static_cast< const CControlConfirmForm::TConfirmedEventData* >( eventData );
    if ( CORE::JOBCONTROLACTION_UNSPECIFIED == m_pendingAction || GUCEF_NULL == reason )
        return;

    const CORE::TJobControlAction action = m_pendingAction;
    m_pendingAction = CORE::JOBCONTROLACTION_UNSPECIFIED;
    if ( !SendCommand( action, m_pendingTargetKind, m_pendingTargetId, reason->GetData() ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CSchedulerForm: The command " << DescribeAction( action ) << " on " << m_pendingTargetId << " could not be sent" );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnCommandCompleted( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CSchedulerClient::TCommandCompletedEventData* data = static_cast< const DRNUTILS::CSchedulerClient::TCommandCompletedEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const DRNUTILS::SDrnUtilsCommandOutcome& outcome = data->GetData();

    // answers of commands this form did not send (another form on the same client) are not its business
    if ( outcome.commandId.IsNULLOrEmpty() || outcome.commandId != m_sentCommandId )
        return;

    if ( DRNUTILS::DRNUTILS_STATUS_OK == outcome.status )
        m_lastCommandText = m_pendingText + ": done";
    else
    {
        m_lastCommandText = m_pendingText + " was refused: ";
        m_lastCommandText += outcome.statusMessage.IsNULLOrEmpty() ? CORE::CString( DRNUTILS::DrnUtilsCommandStatusToString( outcome.status ) ) : outcome.statusMessage;
        m_lastCommandText += " (";
        m_lastCommandText += DRNUTILS::DrnUtilsCommandStatusToString( outcome.status );
        m_lastCommandText += ")";
    }
    m_sentCommandId.Clear();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnInfoReceived( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CSchedulerClient::TInfoReceivedEventData* data = static_cast< const DRNUTILS::CSchedulerClient::TInfoReceivedEventData* >( eventData );
    if ( GUCEF_NULL != data )
    {
        m_hostInfo = data->GetData();
        if ( GUCEF_NULL != m_historyForm )
            m_historyForm->SetHostInfo( m_hostInfo );
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnSelectionOrModelChanged( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestSelectedJobDetail( false );
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::RequestSelectedJobDetail( bool force )
{GUCEF_TRACE;

    const CORE::CString jobId = GetSelectedJobId();
    if ( !force && jobId == m_detailJobId )
        return;
    m_detailJobId = jobId;
    m_detailRequestId.Clear();
    m_haveDetail = false;
    m_detail = DRNUTILS::SSchedulerJobDetailAnswer();
    if ( jobId.IsNULLOrEmpty() || GUCEF_NULL == m_client || !m_open )
        return;
    // asynchronous: the answer comes as JobDetailReceivedEvent, matched by its request id
    m_detailRequestId = m_client->RequestJobDetail( jobId, 5, true );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnJobDetailReceived( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CSchedulerClient::TJobDetailReceivedEventData* data = static_cast< const DRNUTILS::CSchedulerClient::TJobDetailReceivedEventData* >( eventData );
    if ( GUCEF_NULL == data || m_detailRequestId.IsNULLOrEmpty() || data->GetData().requestId != m_detailRequestId )
        return;
    m_detail = data->GetData();
    m_haveDetail = true;
    Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::GetSelectedJobDetail( DRNUTILS::SSchedulerJobDetailAnswer& detail ) const
{GUCEF_TRACE;

    if ( !m_haveDetail || m_detail.jobId != GetSelectedJobId() )
        return false;
    detail = m_detail;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnActiveTabChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::UpdateButtons( void )
{GUCEF_TRACE;

    struct SButton { const char* name; CORE::TJobControlAction action; };
    const SButton buttons[] =
    {
        { "RunNow", CORE::JOBCONTROLACTION_RUN_NOW }, { "CancelRun", CORE::JOBCONTROLACTION_CANCEL_RUN },
        { "Enable", CORE::JOBCONTROLACTION_ENABLE }, { "Disable", CORE::JOBCONTROLACTION_DISABLE }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
            button->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( buttons[ i ].action ) );
    }
    GUI::CWidget* history = GetWidget( "History" );
    if ( GUCEF_NULL != history )
        history->SetBoolProperty( GUI::EWidgetProperty_Enabled, !GetSelectedJobId().IsNULLOrEmpty() );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    UpdateButtons();
    SetLabelText( "Status", GetStatusText() );
    SetLabelText( "Details", GetJobDetailsText() );
    SetLabelText( "Result", m_lastCommandText );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::GetStatusText( void ) const
{GUCEF_TRACE;

    CORE::CString text = "Host ";
    text += m_hostInfo.hostName.IsNULLOrEmpty() ? CORE::CString( "(unknown)" ) : m_hostInfo.hostName;
    if ( !m_hostInfo.state.IsNULLOrEmpty() )
        text += ", " + m_hostInfo.state;
    text += ", revision " + CORE::UInt64ToString( m_hostInfo.revision );
    text += GUCEF_NULL != m_client && m_client->IsConnected() ? ", connected" : ", not connected";

    // the zone is described at the time of the host when it told one, else now
    const CTimeDisplayFormatter formatter( m_timeDisplay );
    const Int64 instant = 0 != m_hostInfo.serverTimeUtcMs ? m_hostInfo.serverTimeUtcMs : (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    text += ", times in " + formatter.GetZoneDescription( instant );
    return text;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerForm::GetLastCommandText( void ) const
{GUCEF_TRACE;

    return m_lastCommandText;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerForm::GetJobDetailsText( void ) const
{GUCEF_TRACE;

    const CTimeDisplayTableModel* model = m_jobsBinding.GetModel();
    const Int32 row = m_jobsBinding.GetSelectedRow();
    if ( GUCEF_NULL == model || row < 0 )
        return CORE::CString::Empty;

    CORE::CString text;
    const UInt32 columns = model->GetColumnCount();
    for ( UInt32 i=0; i<columns; ++i )
    {
        const CORE::CString cell = model->GetCellText( (UInt32) row, i );
        if ( cell.IsNULLOrEmpty() )
            continue;
        if ( !text.IsNULLOrEmpty() )
            text += '\n';
        text += model->GetColumn( i ).GetDisplayTitle() + ": " + cell;
        const CORE::CString utc = model->GetCellUtcText( (UInt32) row, i );
        if ( !utc.IsNULLOrEmpty() )
            text += " (" + utc + ")";
    }

    // what Scheduler/GetJob told about the selected job
    DRNUTILS::SSchedulerJobDetailAnswer detail;
    if ( !GetSelectedJobDetail( detail ) )
        return text;
    if ( DRNUTILS::DRNUTILS_STATUS_OK != detail.status )
    {
        text += "\nDetail: " + CORE::CString( DRNUTILS::DrnUtilsCommandStatusToString( detail.status ) );
        if ( !detail.statusMessage.IsNULLOrEmpty() )
            text += " (" + detail.statusMessage + ")";
        return text;
    }
    if ( !detail.detail.job.description.IsNULLOrEmpty() )
        text += "\nDescription: " + detail.detail.job.description;
    CORE::CString actions;
    for ( UInt32 a=0; a<detail.allowedActions.size(); ++a )
    {
        if ( !actions.IsNULLOrEmpty() )
            actions += ", ";
        const DRNUTILS::SSchedulerActionInfo& action = detail.allowedActions[ a ];
        if ( !action.title.IsNULLOrEmpty() )
            actions += action.title;
        else
        if ( CORE::JOBCONTROLACTION_CUSTOM == action.action )
            actions += action.customName;
        else
            actions += CORE::JobControlActionToString( action.action );
    }
    text += "\nAllowed actions: " + ( actions.IsNULLOrEmpty() ? CORE::CString( "none" ) : actions );
    const CORE::TJobRunRecordVector& runs = detail.detail.recentRuns;
    text += "\nRecent runs: " + CORE::UInt32ToString( (UInt32) runs.size() );
    if ( !runs.empty() )
        text += " (last " + CORE::UInt64ToString( runs[ 0 ].runId ) + ": " + CORE::JobRunStateToString( runs[ 0 ].state ) + ")";
    const CORE::CDataNode& source = detail.detail.sourceStatus;
    if ( source.GetAttCount() > 0 )
    {
        CORE::CString status;
        CORE::CDataNode::TAttributeMap::const_iterator att = source.AttributeBegin();
        while ( att != source.AttributeEnd() )
        {
            if ( !status.IsNULLOrEmpty() )
                status += ", ";
            status += (*att).first + "=" + (*att).second.AsString();
            ++att;
        }
        text += "\nWork source: " + status;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CSchedulerForm::GetJobsBinding( void )
{GUCEF_TRACE;

    return m_jobsBinding;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CSchedulerForm::GetLanesBinding( void )
{GUCEF_TRACE;

    return m_lanesBinding;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CSchedulerForm::GetRunsBinding( void )
{GUCEF_TRACE;

    return m_runsBinding;
}

/*-------------------------------------------------------------------------*/

const DRNUTILS::SSchedulerHostInfo&
CSchedulerForm::GetHostInfo( void ) const
{GUCEF_TRACE;

    return m_hostInfo;
}

/*-------------------------------------------------------------------------*/

CJobRunHistoryForm*
CSchedulerForm::GetRunHistoryForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_historyForm && GUCEF_NULL != GetContext() )
    {
        m_historyForm = GUCEF_NEW CJobRunHistoryForm( GetContext() );
        m_historyForm->SetClient( m_client );
        m_historyForm->SetHostInfo( m_hostInfo );
        m_historyForm->SetTimeDisplay( m_timeDisplay );
    }
    return m_historyForm;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerForm::OpenRunHistory( void )
{GUCEF_TRACE;

    const CORE::CString jobId = GetSelectedJobId();
    CJobRunHistoryForm* history = GetRunHistoryForm();
    if ( jobId.IsNULLOrEmpty() || GUCEF_NULL == history )
        return false;
    history->SetRunsTable( m_tables.GetRuns() );
    history->SetJobId( jobId );
    return history->Open();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnRunNowActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( CORE::JOBCONTROLACTION_RUN_NOW );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnCancelRunActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( CORE::JOBCONTROLACTION_CANCEL_RUN );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnEnableActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( CORE::JOBCONTROLACTION_ENABLE );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnDisableActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( CORE::JOBCONTROLACTION_DISABLE );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnHistoryActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OpenRunHistory();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnCloseActivated( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerForm::OnNavigation( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Close();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
