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

#ifndef GUCEF_CORE_CDATATABLEPREDICATE_H
#include "gucefCORE_CDataTablePredicate.h"
#define GUCEF_CORE_CDATATABLEPREDICATE_H
#endif /* GUCEF_CORE_CDATATABLEPREDICATE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILSGUI_CJobRunHistoryForm.h"

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

const CORE::CString CJobRunHistoryForm::ClassTypeName = "GUCEF::DRNUTILSGUI::CJobRunHistoryForm";
const CORE::CEvent CJobRunHistoryForm::ClosedEvent = "GUCEF::DRNUTILSGUI::CJobRunHistoryForm::ClosedEvent";

namespace {

const SGridColumnSpec g_columns[] =
{
    { "runId", "Run", 80 }, { "state", "State", 100 }, { "cause", "Cause", 100 }, { "scheduledUtc", "Scheduled", 190 },
    { "startedUtc", "Started", 190 }, { "endedUtc", "Ended", 190 }, { "attempt", "Attempt", 60 }, { "progressDone", "Done", 60 },
    { "progressTotal", "Total", 60 }, { "taskId", "Task", 70 }, { "executionTarget", "Target", 110 }, { "threadPool", "Pool", 110 },
    { "taskType", "Task type", 140 }, { "message", "Message", 240 }, { "requestedBy", "By", 100 }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CJobRunHistoryForm::CJobRunHistoryForm( void )
    : GUI::CFormEx()
    , m_runsTable()
    , m_jobId()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_binding()
    , m_confirmForm( GUCEF_NULL )
    , m_pendingRunId( 0 )
    , m_sentCommandId()
    , m_pendingText()
    , m_lastCommandText()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CJobRunHistoryForm::CJobRunHistoryForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_runsTable()
    , m_jobId()
    , m_client( GUCEF_NULL )
    , m_hostInfo()
    , m_timeDisplay()
    , m_binding()
    , m_confirmForm( GUCEF_NULL )
    , m_pendingRunId( 0 )
    , m_sentCommandId()
    , m_pendingText()
    , m_lastCommandText()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CJobRunHistoryForm::~CJobRunHistoryForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_binding.Unbind();
    GUCEF_DELETE m_confirmForm;
    m_confirmForm = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "JobRunHistoryForm", "0.06,0 0.1,0", "0.88,0 0.8,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Runs of a job" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Header", "0,16 0,8", "1,-32 0,26" );
    GUI::CFormLayout::AddWidgetNode( *window, "GridView", "Runs", "0,16 0,40", "1,-32 1,-170" );

    GUI::CFormLayout::AddWidgetNode( *window, "ProgressBar", "Progress", "0,16 1,-120", "1,-32 0,22" );
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "ProgressText", "0,16 1,-92", "1,-32 0,24" );

    CORE::CDataNode* cancelRun = GUI::CFormLayout::AddWidgetNode( *window, "Button", "CancelRun", "0,16 1,-62", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancelRun, GUI::EWidgetProperty_Text, "Cancel run" );
    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Close", "1,-126 1,-62", "0,110 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );
    CORE::CDataNode* result = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Result", "0,140 1,-58", "1,-280 0,26" );
    GUI::CFormLayout::SetWidgetNodeProperty( *result, GUI::EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CJobRunHistoryForm::Build( void )
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
CJobRunHistoryForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* cancelRun = GetWidget( "CancelRun" );
    if ( GUCEF_NULL != cancelRun )
    {
        TEventCallback callback( this, &CJobRunHistoryForm::OnCancelRunActivated );
        SubscribeTo( cancelRun, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* close = GetWidget( "Close" );
    if ( GUCEF_NULL != close )
    {
        TEventCallback callback( this, &CJobRunHistoryForm::OnCloseActivated );
        SubscribeTo( close, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CJobRunHistoryForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
    TEventCallback changed( this, &CJobRunHistoryForm::OnSelectionOrModelChanged );
    SubscribeTo( &m_binding, CTableGridBinding::ModelChangedEvent, changed );
    SubscribeTo( &m_binding, CTableGridBinding::SelectionChangedEvent, changed );

    BindTable();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::BindTable( void )
{GUCEF_TRACE;

    m_binding.Bind( GetWidgetAs< GUI::CGridView >( "Runs" ), m_runsTable, "runId", g_columns, sizeof( g_columns ) / sizeof( g_columns[ 0 ] ), m_timeDisplay );

    // only the runs of the job, the newest first
    CORE::CDataTableQuery::TPredicateVector predicates;
    if ( !m_jobId.IsNULLOrEmpty() )
        predicates.push_back( CORE::CDataTablePredicate( "jobId", CORE::DATATABLE_PREDICATE_EQUALS, CORE::CVariant( m_jobId ) ) );
    m_binding.SetFilter( predicates );
    m_binding.SortBy( "runId", false );
}

/*-------------------------------------------------------------------------*/

bool
CJobRunHistoryForm::Open( void )
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
CJobRunHistoryForm::Close( void )
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
CJobRunHistoryForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetRunsTable( const CORE::CIDataTablePtr& runsTable )
{GUCEF_TRACE;

    m_runsTable = runsTable;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        BindTable();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetJobId( const CORE::CString& jobId )
{GUCEF_TRACE;

    m_jobId = jobId;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        BindTable();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CJobRunHistoryForm::GetJobId( void ) const
{GUCEF_TRACE;

    return m_jobId;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetClient( DRNUTILS::CSchedulerClient* client )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        UnsubscribeFrom( m_client, DRNUTILS::CSchedulerClient::InfoReceivedEvent );
        UnsubscribeFrom( m_client, DRNUTILS::CSchedulerClient::CommandCompletedEvent );
    }
    m_client = client;
    m_hostInfo = DRNUTILS::SSchedulerHostInfo();
    if ( GUCEF_NULL != m_client )
    {
        TEventCallback infoCallback( this, &CJobRunHistoryForm::OnInfoReceived );
        SubscribeTo( m_client, DRNUTILS::CSchedulerClient::InfoReceivedEvent, infoCallback );
        TEventCallback commandCallback( this, &CJobRunHistoryForm::OnCommandCompleted );
        SubscribeTo( m_client, DRNUTILS::CSchedulerClient::CommandCompletedEvent, commandCallback );
        m_hostInfo = m_client->GetLastInfo();
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetTimeDisplay( const CTimeDisplaySettings& settings )
{GUCEF_TRACE;

    m_timeDisplay = settings;
    m_binding.SetTimeDisplay( settings );
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetHostInfo( const DRNUTILS::SSchedulerHostInfo& hostInfo )
{GUCEF_TRACE;

    m_hostInfo = hostInfo;
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SelectRun( UInt64 runId )
{GUCEF_TRACE;

    if ( !m_binding.SelectKeyText( CORE::UInt64ToString( runId ) ) )
        m_binding.ClearSelection();
}

/*-------------------------------------------------------------------------*/

UInt64
CJobRunHistoryForm::GetSelectedRunId( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_binding.GetSelectedCellValue( "runId", value ) )
        return 0;
    return value.AsUInt64( 0, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CJobRunHistoryForm::GetSelectedRunState( void ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !m_binding.GetSelectedCellValue( "state", value ) )
        return CORE::CString::Empty;
    return value.AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CJobRunHistoryForm::IsCancelEnabled( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || 0 == GetSelectedRunId() )
        return false;
    bool allowed = false;
    for ( size_t i=0; i<m_hostInfo.allowedActions.size(); ++i )
        if ( CORE::JOBCONTROLACTION_CANCEL_RUN == m_hostInfo.allowedActions[ i ].action )
            allowed = true;
    const CORE::CString state = GetSelectedRunState();
    return allowed && ( "QUEUED" == state || "WAITING" == state || "RUNNING" == state );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CJobRunHistoryForm::GetProgressText( void ) const
{GUCEF_TRACE;

    CORE::CVariant done, total;
    if ( !m_binding.GetSelectedCellValue( "progressDone", done ) || !m_binding.GetSelectedCellValue( "progressTotal", total ) )
        return CORE::CString::Empty;
    const UInt64 totalValue = total.AsUInt64( 0, false );
    if ( 0 == totalValue )
        return CORE::CString::Empty;
    return CORE::UInt64ToString( done.AsUInt64( 0, false ) ) + " of " + CORE::UInt64ToString( totalValue );
}

/*-------------------------------------------------------------------------*/

Float32
CJobRunHistoryForm::GetProgressPercentage( void ) const
{GUCEF_TRACE;

    CORE::CVariant done, total;
    if ( !m_binding.GetSelectedCellValue( "progressDone", done ) || !m_binding.GetSelectedCellValue( "progressTotal", total ) )
        return 0.0f;
    const UInt64 totalValue = total.AsUInt64( 0, false );
    if ( 0 == totalValue )
        return 0.0f;
    UInt64 doneValue = done.AsUInt64( 0, false );
    if ( doneValue > totalValue )
        doneValue = totalValue;
    return (Float32) ( 100.0 * (double) doneValue / (double) totalValue );
}

/*-------------------------------------------------------------------------*/

bool
CJobRunHistoryForm::CancelSelectedRun( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsCancelEnabled() )
        return false;

    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
    command.targetKind = CORE::JOBCONTROLTARGET_RUN;
    command.targetId = CORE::UInt64ToString( GetSelectedRunId() );
    command.reason = reason;
    const CORE::CString text = "Cancel run of run " + command.targetId;
    m_pendingText = text;

    const CORE::CString commandId = m_client->Control( command, false );
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

const CORE::CString&
CJobRunHistoryForm::GetLastCommandText( void ) const
{GUCEF_TRACE;

    return m_lastCommandText;
}

/*-------------------------------------------------------------------------*/

CTableGridBinding&
CJobRunHistoryForm::GetBinding( void )
{GUCEF_TRACE;

    return m_binding;
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm*
CJobRunHistoryForm::GetConfirmForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_confirmForm && GUCEF_NULL != GetContext() )
    {
        m_confirmForm = GUCEF_NEW CControlConfirmForm( GetContext() );
        TEventCallback callback( this, &CJobRunHistoryForm::OnConfirmed );
        SubscribeTo( m_confirmForm, CControlConfirmForm::ConfirmedEvent, callback );
    }
    return m_confirmForm;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnCancelRunActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CControlConfirmForm* confirm = GetConfirmForm();
    if ( GUCEF_NULL == confirm || !IsCancelEnabled() )
        return;
    m_pendingRunId = GetSelectedRunId();
    confirm->Ask( "Cancel run", "Cancel run on the node " + m_hostInfo.hostName, "Run " + CORE::UInt64ToString( m_pendingRunId ) + " of job " + m_jobId );
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnConfirmed( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CControlConfirmForm::TConfirmedEventData* reason = static_cast< const CControlConfirmForm::TConfirmedEventData* >( eventData );
    if ( 0 == m_pendingRunId || GUCEF_NULL == reason )
        return;
    // the modal question kept the selection where it was
    if ( GetSelectedRunId() == m_pendingRunId )
        CancelSelectedRun( reason->GetData() );
    m_pendingRunId = 0;
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnCommandCompleted( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CSchedulerClient::TCommandCompletedEventData* data = static_cast< const DRNUTILS::CSchedulerClient::TCommandCompletedEventData* >( eventData );
    if ( GUCEF_NULL == data || data->GetData().commandId.IsNULLOrEmpty() || data->GetData().commandId != m_sentCommandId )
        return;
    const DRNUTILS::SDrnUtilsCommandOutcome& outcome = data->GetData();

    const CORE::CString& text = m_pendingText;
    if ( DRNUTILS::DRNUTILS_STATUS_OK == outcome.status )
        m_lastCommandText = text + ": done";
    else
        m_lastCommandText = text + " was refused: " + ( outcome.statusMessage.IsNULLOrEmpty() ? CORE::CString( DRNUTILS::DrnUtilsCommandStatusToString( outcome.status ) ) : outcome.statusMessage );
    m_sentCommandId.Clear();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnInfoReceived( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const DRNUTILS::CSchedulerClient::TInfoReceivedEventData* data = static_cast< const DRNUTILS::CSchedulerClient::TInfoReceivedEventData* >( eventData );
    if ( GUCEF_NULL != data )
    {
        m_hostInfo = data->GetData();
        Refresh();
    }
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnSelectionOrModelChanged( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    SetLabelText( "Header", "Runs of job " + m_jobId );
    GUI::CWidget* cancelRun = GetWidget( "CancelRun" );
    if ( GUCEF_NULL != cancelRun )
        cancelRun->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsCancelEnabled() );

    // the progress of the selected run, a bar only for a run that has a total
    GUI::CWidget* progress = GetWidget( "Progress" );
    if ( GUCEF_NULL != progress )
        progress->SetProperty( GUI::EWidgetProperty_Value, CORE::FloatToString( GetProgressPercentage() ) );
    SetLabelText( "ProgressText", GetProgressText() );
    SetLabelText( "Result", m_lastCommandText );
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnCloseActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CJobRunHistoryForm::OnNavigation( CORE::CNotifier* notifier    ,
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
CJobRunHistoryForm::GetClassTypeName( void ) const
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
