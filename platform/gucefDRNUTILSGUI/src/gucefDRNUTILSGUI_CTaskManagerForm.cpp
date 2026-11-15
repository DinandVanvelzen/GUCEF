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
#include <stdlib.h>

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

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILSGUI_CTaskManagerForm.h"

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

const CORE::CString CTaskManagerForm::ClassTypeName = "GUCEF::DRNUTILSGUI::CTaskManagerForm";
const CORE::CEvent CTaskManagerForm::ClosedEvent = "GUCEF::DRNUTILSGUI::CTaskManagerForm::ClosedEvent";

namespace {

const SGridColumnSpec g_poolColumns[] =
{
    { "name", "Pool", 150 }, { "activeWorkers", "Workers", 70 }, { "desiredMinWorkers", "Min", 60 }, { "desiredMaxThreads", "Max", 60 },
    { "dedicatedThreads", "Dedicated", 80 }, { "queued", "Queued", 70 }, { "running", "Running", 70 }, { "utilisationPercent", "Util %", 70 },
    { "avgQueueWaitMs", "Wait ms", 80 }, { "avgRunMs", "Run ms", 80 }, { "startedPerSecond", "Started/s", 80 }, { "finishedPerSecond", "Finished/s", 80 },
    { "failedPerSecond", "Failed/s", 80 }, { "scheduledJobs", "Jobs", 60 }
};

const SGridColumnSpec g_threadColumns[] =
{
    { "systemThreadId", "Thread", 100 }, { "name", "Name", 160 }, { "pool", "Pool", 130 }, { "status", "Status", 100 }, { "taskId", "Task", 70 },
    { "taskType", "Task type", 160 }, { "busySinceUtc", "Busy since", 190 }, { "external", "External", 70 }
};

const SGridColumnSpec g_taskColumns[] =
{
    { "taskId", "Task", 70 }, { "pool", "Pool", 130 }, { "taskType", "Task type", 160 }, { "status", "Status", 100 }, { "extraInfo", "Info", 200 },
    { "queuedUtc", "Queued", 190 }, { "startedUtc", "Started", 190 }, { "runningForMs", "Running ms", 90 }, { "systemThreadId", "Thread", 90 },
    { "schedulerName", "Scheduler", 120 }, { "jobId", "Job", 130 }, { "runId", "Run", 70 }
};

const SGridColumnSpec g_scheduledJobColumns[] =
{
    { "jobKey", "Job key", 200 }, { "schedulerName", "Scheduler", 120 }, { "title", "Title", 180 }, { "taskType", "Task type", 150 },
    { "executionTarget", "Target", 110 }, { "pool", "Pool", 110 }, { "lane", "Lane", 90 }, { "state", "State", 100 }, { "nextDueUtc", "Next due", 190 },
    { "activeRuns", "Active", 60 }, { "lastRunState", "Last run", 100 }, { "lastRunEndedUtc", "Last run ended", 190 }, { "origin", "Origin", 80 }
};

template< class T, UInt32 N >
UInt32
CountOf( const T ( & )[ N ] )
{
    return N;
}

/** The sample intervals the selector offers, in milliseconds, with their texts */
const UInt32 g_intervalsMs[] = { 1000, 2000, 5000, 10000, 30000 };
const char* const g_intervalTexts[] = { "1 s", "2 s", "5 s", "10 s", "30 s" };

UInt32
FindIntervalIndex( UInt32 intervalMs )
{
    UInt32 best = 0;
    for ( UInt32 i=0; i<CountOf( g_intervalsMs ); ++i )
        if ( g_intervalsMs[ i ] <= intervalMs )
            best = i;
    return best;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTaskManagerForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskManagerForm::CTaskManagerForm( void )
    : GUI::CFormEx()
    , m_tables()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_poolsBinding()
    , m_threadsBinding()
    , m_tasksBinding()
    , m_scheduledJobsBinding()
    , m_confirmForm( GUCEF_NULL )
    , m_sampleIntervalMs( 2000 )
    , m_pendingCommand()
    , m_hasPendingCommand( false )
    , m_pendingText()
    , m_sentCommandId()
    , m_lastCommandText()
    , m_viewing( false )
    , m_viewingThreads( false )
    , m_viewingTasks( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTaskManagerForm::CTaskManagerForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_tables()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_poolsBinding()
    , m_threadsBinding()
    , m_tasksBinding()
    , m_scheduledJobsBinding()
    , m_confirmForm( GUCEF_NULL )
    , m_sampleIntervalMs( 2000 )
    , m_pendingCommand()
    , m_hasPendingCommand( false )
    , m_pendingText()
    , m_sentCommandId()
    , m_lastCommandText()
    , m_viewing( false )
    , m_viewingThreads( false )
    , m_viewingTasks( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTaskManagerForm::~CTaskManagerForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    StopViewing();
    m_poolsBinding.Unbind();
    m_threadsBinding.Unbind();
    m_tasksBinding.Unbind();
    m_scheduledJobsBinding.Unbind();
    GUCEF_DELETE m_confirmForm;
    m_confirmForm = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "TaskManagerForm", "0.04,0 0.04,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Task manager" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,16 0,8", "1,-32 0,26" );

    CORE::CDataNode* tabs = GUI::CFormLayout::AddWidgetNode( *window, "TabControl", "Tabs", "0,16 0,40", "1,-32 1,-170" );
    GUI::CFormLayout::SetWidgetNodeProperty( *tabs, GUI::EWidgetProperty_Tabs, "Pools\nThreads\nTasks\nScheduled jobs" );
    CORE::CDataNode* poolsPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "PoolsPane" );
    GUI::CFormLayout::AddWidgetNode( *poolsPane, "GridView", "PoolsGrid", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* threadsPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "ThreadsPane" );
    GUI::CFormLayout::AddWidgetNode( *threadsPane, "GridView", "ThreadsGrid", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* tasksPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "TasksPane" );
    GUI::CFormLayout::AddWidgetNode( *tasksPane, "GridView", "TasksGrid", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* scheduledPane = GUI::CFormLayout::AddWidgetNode( *tabs, "TabContentPane", "ScheduledJobsPane" );
    GUI::CFormLayout::AddWidgetNode( *scheduledPane, "GridView", "ScheduledJobsGrid", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* minLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "MinWorkersLabel", "0,16 1,-124", "0,150 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *minLabel, GUI::EWidgetProperty_Text, "Minimum workers" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "MinWorkers", "0,16 1,-98", "0,150 0,30" );
    CORE::CDataNode* maxLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "MaxThreadsLabel", "0,180 1,-124", "0,190 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxLabel, GUI::EWidgetProperty_Text, "Maximum threads (-1: none)" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "MaxThreads", "0,180 1,-98", "0,150 0,30" );
    CORE::CDataNode* resize = GUI::CFormLayout::AddWidgetNode( *window, "Button", "ResizePool", "0,344 1,-98", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *resize, GUI::EWidgetProperty_Text, "Resize pool" );

    CORE::CDataNode* cancelTask = GUI::CFormLayout::AddWidgetNode( *window, "Button", "CancelTask", "0,480 1,-98", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancelTask, GUI::EWidgetProperty_Text, "Cancel task" );
    CORE::CDataNode* stopTask = GUI::CFormLayout::AddWidgetNode( *window, "Button", "StopTask", "0,598 1,-98", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *stopTask, GUI::EWidgetProperty_Text, "Stop task" );

    CORE::CDataNode* intervalLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "SampleIntervalLabel", "0,730 1,-124", "0,150 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *intervalLabel, GUI::EWidgetProperty_Text, "Sample interval" );
    GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "SampleInterval", "0,730 1,-98", "0,110 0,30" );

    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Close", "1,-126 1,-98", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );

    CORE::CDataNode* result = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Result", "0,16 1,-56", "1,-32 0,26" );
    GUI::CFormLayout::SetWidgetNodeProperty( *result, GUI::EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::Build( void )
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
CTaskManagerForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    struct SButton { const char* name; void ( CTaskManagerForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "ResizePool", &CTaskManagerForm::OnResizeActivated }, { "CancelTask", &CTaskManagerForm::OnCancelTaskActivated },
        { "StopTask", &CTaskManagerForm::OnStopTaskActivated }, { "Close", &CTaskManagerForm::OnCloseActivated }
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
        TEventCallback callback( this, &CTaskManagerForm::OnActiveTabChanged );
        SubscribeTo( tabs, GUI::CTabControl::ActiveTabChangedEvent, callback );
    }

    GUI::CCombobox* interval = GetWidgetAs< GUI::CCombobox >( "SampleInterval" );
    if ( GUCEF_NULL != interval )
    {
        GUI::CCombobox::TStringVector items;
        for ( UInt32 i=0; i<CountOf( g_intervalTexts ); ++i )
            items.push_back( g_intervalTexts[ i ] );
        interval->SetListItems( items );
        interval->SetSelectedIndex( (Int32) FindIntervalIndex( m_sampleIntervalMs ) );
        TEventCallback callback( this, &CTaskManagerForm::OnIntervalChanged );
        SubscribeTo( interval, GUI::CWidget::ValueChangedEvent, callback );
    }

    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CTaskManagerForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }

    CTableGridBinding* bindings[] = { &m_poolsBinding, &m_threadsBinding, &m_tasksBinding, &m_scheduledJobsBinding };
    for ( UInt32 i=0; i<4; ++i )
    {
        TEventCallback changed( this, &CTaskManagerForm::OnSelectionOrModelChanged );
        SubscribeTo( bindings[ i ], CTableGridBinding::ModelChangedEvent, changed );
        SubscribeTo( bindings[ i ], CTableGridBinding::SelectionChangedEvent, changed );
    }

    BindTables();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::BindTables( void )
{GUCEF_TRACE;

    m_poolsBinding.Bind( GetWidgetAs< GUI::CGridView >( "PoolsGrid" ), m_tables.GetPools(), "name", g_poolColumns, CountOf( g_poolColumns ), m_timeDisplay );
    m_threadsBinding.Bind( GetWidgetAs< GUI::CGridView >( "ThreadsGrid" ), m_tables.GetThreads(), "systemThreadId", g_threadColumns, CountOf( g_threadColumns ), m_timeDisplay );
    m_tasksBinding.Bind( GetWidgetAs< GUI::CGridView >( "TasksGrid" ), m_tables.GetTasks(), "taskId", g_taskColumns, CountOf( g_taskColumns ), m_timeDisplay );
    m_scheduledJobsBinding.Bind( GetWidgetAs< GUI::CGridView >( "ScheduledJobsGrid" ), m_tables.GetScheduledJobs(), "jobKey", g_scheduledJobColumns, CountOf( g_scheduledJobColumns ), m_timeDisplay );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::StopViewing( void )
{GUCEF_TRACE;

    if ( m_viewing && GUCEF_NULL != m_client )
        m_client->StopViewing();
    m_viewing = false;
    m_viewingThreads = false;
    m_viewingTasks = false;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::UpdateViewing( bool restart )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_client )
    {
        StopViewing();
        return;
    }

    // sampling has a cost: the threads and the tasks are only watched while their tab is shown
    const bool wantThreads = TAB_THREADS == GetShownTab();
    const bool wantTasks = TAB_TASKS == GetShownTab();
    if ( m_viewing && !restart && wantThreads == m_viewingThreads && wantTasks == m_viewingTasks )
        return;

    if ( m_viewing )
        m_client->StopViewing();
    m_viewing = m_client->StartViewing( m_sampleIntervalMs, wantThreads, wantTasks );
    m_viewingThreads = m_viewing && wantThreads;
    m_viewingTasks = m_viewing && wantTasks;
    if ( !m_viewing )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTaskManagerForm: The client could not start viewing the task manager" );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::IsViewing( void ) const
{GUCEF_TRACE;

    return m_viewing;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    if ( GUCEF_NULL != m_client )
        m_client->RequestInfo();
    UpdateViewing( true );
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::Close( void )
{GUCEF_TRACE;

    const bool wasOpen = m_open;
    m_open = false;
    StopViewing();
    if ( GUCEF_NULL != GetRootWidget() )
        Hide();
    if ( wasOpen )
        NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SetTables( const DRNUTILS::CTaskManagerTableSet& tables )
{GUCEF_TRACE;

    m_tables = tables;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        BindTables();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

const DRNUTILS::CTaskManagerTableSet&
CTaskManagerForm::GetTables( void ) const
{GUCEF_TRACE;

    return m_tables;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SetClient( DRNUTILS::CTaskManagerClient* client )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        // the viewing was started on the client that is taken away
        StopViewing();
        UnsubscribeFrom( m_client, DRNUTILS::CTaskManagerClient::InfoReceivedEvent );
        UnsubscribeFrom( m_client, DRNUTILS::CTaskManagerClient::CommandCompletedEvent );
    }
    m_client = client;
    m_hostInfo = DRNUTILS::STaskManagerHostInfo();
    if ( GUCEF_NULL != m_client )
    {
        TEventCallback infoCallback( this, &CTaskManagerForm::OnInfoReceived );
        SubscribeTo( m_client, DRNUTILS::CTaskManagerClient::InfoReceivedEvent, infoCallback );
        TEventCallback commandCallback( this, &CTaskManagerForm::OnCommandCompleted );
        SubscribeTo( m_client, DRNUTILS::CTaskManagerClient::CommandCompletedEvent, commandCallback );
        m_hostInfo = m_client->GetLastInfo();
        if ( m_open )
            UpdateViewing( true );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CTaskManagerClient*
CTaskManagerForm::GetClient( void ) const
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SetTimeDisplay( const CTimeDisplaySettings& settings )
{GUCEF_TRACE;

    m_timeDisplay = settings;
    m_poolsBinding.SetTimeDisplay( settings );
    m_threadsBinding.SetTimeDisplay( settings );
    m_tasksBinding.SetTimeDisplay( settings );
    m_scheduledJobsBinding.SetTimeDisplay( settings );
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SetSampleIntervalMs( UInt32 sampleIntervalMs )
{GUCEF_TRACE;

    // the host samples at most once a second
    m_sampleIntervalMs = sampleIntervalMs < 1000 ? 1000 : sampleIntervalMs;
    GUI::CCombobox* interval = GetWidgetAs< GUI::CCombobox >( "SampleInterval" );
    if ( GUCEF_NULL != interval )
        interval->SetSelectedIndex( (Int32) FindIntervalIndex( m_sampleIntervalMs ) );
    UpdateViewing( true );
    Refresh();
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerForm::GetSampleIntervalMs( void ) const
{GUCEF_TRACE;

    return m_sampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::ShowTab( TTab tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GetWidgetAs< GUI::CTabControl >( "Tabs" );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

CTaskManagerForm::TTab
CTaskManagerForm::GetShownTab( void ) const
{GUCEF_TRACE;

    const GUI::CTabControl* tabs = const_cast< CTaskManagerForm* >( this )->GetWidgetAs< GUI::CTabControl >( "Tabs" );
    if ( GUCEF_NULL == tabs )
        return TAB_POOLS;
    const Int32 active = tabs->GetActiveTab();
    return active >= 0 && active <= (Int32) TAB_SCHEDULED_JOBS ? (TTab) active : TAB_POOLS;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SelectPool( const CORE::CString& poolName )
{GUCEF_TRACE;

    if ( !m_poolsBinding.SelectKeyText( poolName ) )
        m_poolsBinding.ClearSelection();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerForm::GetSelectedPool( void ) const
{GUCEF_TRACE;

    return m_poolsBinding.GetSelectedKeyText();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SelectTask( UInt32 taskId )
{GUCEF_TRACE;

    if ( !m_tasksBinding.SelectKeyText( CORE::UInt32ToString( taskId ) ) )
        m_tasksBinding.ClearSelection();
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerForm::GetSelectedTaskId( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_tasksBinding.GetSelectedCellValue( "taskId", value ) )
        return 0;
    return value.AsUInt32( 0, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerForm::GetSelectedTaskPool( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_tasksBinding.GetSelectedCellValue( "pool", value ) )
        return CORE::CString::Empty;
    return value.AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::IsActionEnabled( DRNUTILS::TDrnUtilsTaskControlAction action ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;

    switch ( action )
    {
        case DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL:
        {
            const CORE::CString pool = GetSelectedPool();
            return m_hostInfo.capabilities.find( "resizePool" ) != m_hostInfo.capabilities.end() && !pool.IsNULLOrEmpty() &&
                   m_hostInfo.controllablePools.find( pool ) != m_hostInfo.controllablePools.end();
        }
        case DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK:
            return m_hostInfo.capabilities.find( "cancelTask" ) != m_hostInfo.capabilities.end() && 0 != GetSelectedTaskId();
        case DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK:
            return m_hostInfo.capabilities.find( "stopTask" ) != m_hostInfo.capabilities.end() && 0 != GetSelectedTaskId();
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::SendCommand( const DRNUTILS::STaskManagerControlCommand& command, const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;
    m_pendingText = text;

    // asynchronous: the answer arrives as CommandCompletedEvent
    const CORE::CString commandId = m_client->Control( command );
    if ( commandId.IsNULLOrEmpty() )
    {
        m_sentCommandId.Clear();
        m_lastCommandText = text + " could not be sent";
        Refresh();
        return false;
    }
    m_sentCommandId = commandId;
    m_lastCommandText = text + " sent";
    Refresh();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::ResizeSelectedPool( UInt32 minWorkers, Int32 maxThreads, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsActionEnabled( DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL ) )
        return false;

    DRNUTILS::STaskManagerControlCommand command;
    command.action = DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL;
    command.poolName = GetSelectedPool();
    command.minWorkers = minWorkers;
    command.maxThreads = maxThreads;
    command.reason = reason;
    return SendCommand( command, "Resize of pool " + command.poolName );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::CancelSelectedTask( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsActionEnabled( DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK ) )
        return false;

    DRNUTILS::STaskManagerControlCommand command;
    command.action = DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK;
    command.taskId = GetSelectedTaskId();
    command.poolName = GetSelectedTaskPool();
    command.reason = reason;
    return SendCommand( command, "Cancel of task " + CORE::UInt32ToString( command.taskId ) );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::StopSelectedTask( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsActionEnabled( DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK ) )
        return false;

    DRNUTILS::STaskManagerControlCommand command;
    command.action = DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK;
    command.taskId = GetSelectedTaskId();
    command.poolName = GetSelectedTaskPool();
    command.reason = reason;
    return SendCommand( command, "Stop of task " + CORE::UInt32ToString( command.taskId ) );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::ReadResizeInputs( UInt32& minWorkers, Int32& maxThreads, CORE::CString& error ) const
{GUCEF_TRACE;

    CTaskManagerForm* self = const_cast< CTaskManagerForm* >( this );
    GUI::CWidget* minField = self->GetWidget( "MinWorkers" );
    GUI::CWidget* maxField = self->GetWidget( "MaxThreads" );
    if ( GUCEF_NULL == minField || GUCEF_NULL == maxField )
        return false;

    const CORE::CString minText = minField->GetProperty( GUI::EWidgetProperty_Text ).Trim( true ).Trim( false );
    const CORE::CString maxText = maxField->GetProperty( GUI::EWidgetProperty_Text ).Trim( true ).Trim( false );
    char* end = GUCEF_NULL;
    const long minValue = minText.IsNULLOrEmpty() ? -1 : strtol( minText.C_String(), &end, 10 );
    if ( minText.IsNULLOrEmpty() || '\0' != *end || minValue < 0 )
    {
        error = "The minimum number of workers has to be a number of zero or more";
        return false;
    }
    const long maxValue = maxText.IsNULLOrEmpty() ? -2 : strtol( maxText.C_String(), &end, 10 );
    if ( maxText.IsNULLOrEmpty() || '\0' != *end || maxValue < -1 )
    {
        error = "The maximum number of threads has to be a number of zero or more, or -1 for no maximum";
        return false;
    }
    if ( maxValue >= 0 && maxValue < minValue )
    {
        error = "The maximum number of threads cannot be below the minimum number of workers";
        return false;
    }
    minWorkers = (UInt32) minValue;
    maxThreads = (Int32) maxValue;
    return true;
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm*
CTaskManagerForm::GetConfirmForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_confirmForm && GUCEF_NULL != GetContext() )
    {
        m_confirmForm = GUCEF_NEW CControlConfirmForm( GetContext() );
        TEventCallback callback( this, &CTaskManagerForm::OnConfirmed );
        SubscribeTo( m_confirmForm, CControlConfirmForm::ConfirmedEvent, callback );
    }
    return m_confirmForm;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerForm::AskToConfirm( DRNUTILS::TDrnUtilsTaskControlAction action )
{GUCEF_TRACE;

    if ( !IsActionEnabled( action ) )
        return false;
    CControlConfirmForm* confirm = GetConfirmForm();
    if ( GUCEF_NULL == confirm )
        return false;

    DRNUTILS::STaskManagerControlCommand command;
    command.action = action;
    CORE::CString actionText, targetText;
    switch ( action )
    {
        case DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL:
        {
            UInt32 minWorkers = 0;
            Int32 maxThreads = -1;
            CORE::CString error;
            if ( !ReadResizeInputs( minWorkers, maxThreads, error ) )
            {
                m_lastCommandText = "Resize of pool " + GetSelectedPool() + " not sent: " + error;
                Refresh();
                return false;
            }
            command.poolName = GetSelectedPool();
            command.minWorkers = minWorkers;
            command.maxThreads = maxThreads;
            actionText = "Resize the thread pool";
            targetText = "Pool " + command.poolName + ": minimum " + CORE::UInt32ToString( minWorkers ) + " workers, maximum " +
                         ( maxThreads < 0 ? CORE::CString( "no maximum" ) : CORE::Int32ToString( maxThreads ) + " threads" );
            m_pendingText = "Resize of pool " + command.poolName;
            break;
        }
        case DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK:
            command.taskId = GetSelectedTaskId();
            command.poolName = GetSelectedTaskPool();
            actionText = "Ask the task to cancel";
            targetText = "Task " + CORE::UInt32ToString( command.taskId ) + " of pool " + command.poolName;
            m_pendingText = "Cancel of task " + CORE::UInt32ToString( command.taskId );
            break;
        case DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK:
            command.taskId = GetSelectedTaskId();
            command.poolName = GetSelectedTaskPool();
            actionText = "Ask the task to stop";
            targetText = "Task " + CORE::UInt32ToString( command.taskId ) + " of pool " + command.poolName;
            m_pendingText = "Stop of task " + CORE::UInt32ToString( command.taskId );
            break;
        default:
            return false;
    }
    m_pendingCommand = command;
    m_hasPendingCommand = true;
    return confirm->Ask( DRNUTILS::DrnUtilsTaskControlActionToString( action ), actionText + " on the node " + m_hostInfo.hostName, targetText );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnConfirmed( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CControlConfirmForm::TConfirmedEventData* reason = static_cast< const CControlConfirmForm::TConfirmedEventData* >( eventData );
    if ( !m_hasPendingCommand || GUCEF_NULL == reason )
        return;
    m_hasPendingCommand = false;
    DRNUTILS::STaskManagerControlCommand command = m_pendingCommand;
    command.reason = reason->GetData();
    SendCommand( command, m_pendingText );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnCommandCompleted( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CTaskManagerClient::TCommandCompletedEventData* data = static_cast< const DRNUTILS::CTaskManagerClient::TCommandCompletedEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const DRNUTILS::SDrnUtilsCommandOutcome& outcome = data->GetData();
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
CTaskManagerForm::OnInfoReceived( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CTaskManagerClient::TInfoReceivedEventData* data = static_cast< const DRNUTILS::CTaskManagerClient::TInfoReceivedEventData* >( eventData );
    if ( GUCEF_NULL != data )
    {
        m_hostInfo = data->GetData();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnSelectionOrModelChanged( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_poolsBinding )
        UpdateResizeInputs();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnActiveTabChanged( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateViewing( false );
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnIntervalChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CCombobox* interval = GetWidgetAs< GUI::CCombobox >( "SampleInterval" );
    if ( GUCEF_NULL == interval )
        return;
    const Int32 index = interval->GetSelectedIndex();
    if ( index >= 0 && (UInt32) index < CountOf( g_intervalsMs ) && g_intervalsMs[ index ] != m_sampleIntervalMs )
    {
        m_sampleIntervalMs = g_intervalsMs[ index ];
        UpdateViewing( true );
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::UpdateResizeInputs( void )
{GUCEF_TRACE;

    // the fields start with what the selected pool has now
    CORE::CVariant minValue, maxValue;
    GUI::CWidget* minField = GetWidget( "MinWorkers" );
    GUI::CWidget* maxField = GetWidget( "MaxThreads" );
    if ( GUCEF_NULL == minField || GUCEF_NULL == maxField )
        return;
    if ( m_poolsBinding.GetSelectedCellValue( "desiredMinWorkers", minValue ) && m_poolsBinding.GetSelectedCellValue( "desiredMaxThreads", maxValue ) )
    {
        minField->SetProperty( GUI::EWidgetProperty_Text, minValue.AsString( CORE::CString::Empty, false ) );
        maxField->SetProperty( GUI::EWidgetProperty_Text, maxValue.AsString( CORE::CString::Empty, false ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    struct SButton { const char* name; DRNUTILS::TDrnUtilsTaskControlAction action; };
    const SButton buttons[] =
    {
        { "ResizePool", DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL }, { "CancelTask", DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK },
        { "StopTask", DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
            button->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsActionEnabled( buttons[ i ].action ) );
    }
    SetLabelText( "Status", GetStatusText() );
    SetLabelText( "Result", m_lastCommandText );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerForm::GetStatusText( void ) const
{GUCEF_TRACE;

    CORE::CString text = "Host ";
    text += m_hostInfo.hostName.IsNULLOrEmpty() ? CORE::CString( "(unknown)" ) : m_hostInfo.hostName;
    text += GUCEF_NULL != m_client && m_client->IsConnected() ? ", connected" : ", not connected";
    text += m_viewing ? ", viewing every " + CORE::UInt32ToString( m_sampleIntervalMs ) + " ms" : CORE::CString( ", not viewing" );

    const CTimeDisplayFormatter formatter( m_timeDisplay );
    const Int64 instant = 0 != m_hostInfo.serverTimeUtcMs ? m_hostInfo.serverTimeUtcMs : (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    text += ", times in " + formatter.GetZoneDescription( instant );
    return text;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerForm::GetLastCommandText( void ) const
{GUCEF_TRACE;

    return m_lastCommandText;
}

/*-------------------------------------------------------------------------*/

const DRNUTILS::STaskManagerHostInfo&
CTaskManagerForm::GetHostInfo( void ) const
{GUCEF_TRACE;

    return m_hostInfo;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CTaskManagerForm::GetPoolsBinding( void )
{GUCEF_TRACE;

    return m_poolsBinding;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CTaskManagerForm::GetThreadsBinding( void )
{GUCEF_TRACE;

    return m_threadsBinding;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CTaskManagerForm::GetTasksBinding( void )
{GUCEF_TRACE;

    return m_tasksBinding;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CTaskManagerForm::GetScheduledJobsBinding( void )
{GUCEF_TRACE;

    return m_scheduledJobsBinding;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnResizeActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnCancelTaskActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnStopTaskActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    AskToConfirm( DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnCloseActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerForm::OnNavigation( CORE::CNotifier* notifier    ,
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
CTaskManagerForm::GetClassTypeName( void ) const
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
