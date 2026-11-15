/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDrnUtilsGuiFixtures.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The scheduler form on the null driver with the tables of a scheduler, a client that records and a host that allows the control; times shown in UTC */
struct SSchedulerFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    STestSchedulerTables tables;
    CFakeSchedulerClient client;
    DRNUTILSGUI::CSchedulerForm form;

    SSchedulerFixture( void )
        : contextPtr( CreateFormContext() ), context( ContextOf( contextPtr ) ), tables(), client(), form( context )
    {
    }

    bool Init( bool allowControl )
    {
        if ( GUCEF_NULL == context || !tables.Fill() )
            return false;
        DRNUTILSGUI::CTimeDisplaySettings display;
        display.SetDisplayTimeZone( "utc" );
        form.SetTimeDisplay( display );
        form.SetTables( tables.GetSet() );
        form.SetClient( &client );
        if ( !form.Open() )
            return false;
        client.AnnounceInfo( MakeSchedulerInfo( "node-1", allowControl ) );
        Settle();
        return true;
    }

    void Settle( void )
    {
        PumpFor( 20 );
        context->FlushChanges();
    }

    bool Enabled( const char* name )
    {
        return IsWidgetEnabled( form, name );
    }
};

/** The text of a cell of a row of a binding, found by the key of the row */
CORE::CString
CellOfKey( DRNUTILSGUI::CTableGridBinding& binding, const CORE::CString& keyText, const CORE::CString& column )
{
    for ( UInt32 row=0; row<binding.GetRowCount(); ++row )
    {
        CORE::CString key;
        if ( binding.GetCellText( row, binding.GetKeyColumn(), key ) && key == keyText )
        {
            CORE::CString text;
            binding.GetCellText( row, column, text );
            return text;
        }
    }
    return "(no such row)";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSchedulerFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.SchedulerForm" );

    GUCEF_TESTFW_TESTCASE( "UI-T1: the form binds to the tables, shows the columns and their titles, and follows RowsChanged" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CSchedulerForm& form = fixture.form;
            ASSERT_TRUE( form.IsOpen() );
            ASSERT_TRUE( form.GetJobsBinding().IsBound() && form.GetLanesBinding().IsBound() && form.GetRunsBinding().IsBound() );
            ASSERT_TRUE( 3 == form.GetJobsBinding().GetRowCount() && 1 == form.GetLanesBinding().GetRowCount() && 4 == form.GetRunsBinding().GetRowCount() );

            // the grids of the form show the models
            GUI::CGridView* jobsGrid = form.GetWidgetAs< GUI::CGridView >( "JobsGrid" );
            ASSERT_TRUE( GUCEF_NULL != jobsGrid && jobsGrid->HasModel() && 3 == jobsGrid->GetRowCount() );

            // columns and titles: the ones of the form, none of the columns it does not show
            const DRNUTILSGUI::CTimeDisplayTableModel* model = form.GetJobsBinding().GetModel();
            ASSERT_TRUE( GUCEF_NULL != model );
            if ( GUCEF_NULL != model )
            {
                ASSERT_TRUE( "Job" == model->GetColumn( 0 ).title && "jobId" == model->GetColumn( 0 ).name );
                const UInt32 next = model->FindShownColumn( "nextDueUtc" );
                ASSERT_TRUE( GUCEF_UINT32MAX != next && "Next due" == model->GetColumn( next ).title );
                ASSERT_TRUE( GUCEF_UINT32MAX != model->FindShownColumn( "executionTarget" ) && GUCEF_UINT32MAX != model->FindShownColumn( "taskType" ) );
                ASSERT_TRUE( GUCEF_UINT32MAX == model->FindShownColumn( "submittedBy" ) );
            }
            ASSERT_TRUE( "Fetch quotes" == CellOfKey( form.GetJobsBinding(), "job-1", "title" ) );
            ASSERT_TRUE( "MAIN_THREAD" == CellOfKey( form.GetJobsBinding(), "job-2", "executionTarget" ) );
            ASSERT_TRUE( "2026-03-08 06:59:59 UTC" == CellOfKey( form.GetJobsBinding(), "job-1", "nextDueUtc" ) );

            // rows that change behind the back of the table raise RowsChanged: the grid has the new rows, the form is told
            CEventCount modelChanged( &form.GetJobsBinding(), DRNUTILSGUI::CTableGridBinding::ModelChangedEvent );
            TTestRowVector rows;
            rows.push_back( CTestRow().Text( "jobId", "job-2" ).Text( "title", "Rebuild index" ).Text( "lane", "batch" ).Text( "state", "QUEUED" ) );
            rows.push_back( CTestRow().Text( "jobId", "job-4" ).Text( "title", "New job" ).Text( "state", "SCHEDULED" ) );
            ASSERT_TRUE( UpsertTestRows( *fixture.tables.jobs, rows ) );
            fixture.tables.jobs->RaiseRowsChanged();
            ASSERT_TRUE( modelChanged.count >= 1 );
            ASSERT_TRUE( 4 == form.GetJobsBinding().GetRowCount() && 4 == jobsGrid->GetRowCount() );
            ASSERT_TRUE( "QUEUED" == CellOfKey( form.GetJobsBinding(), "job-2", "state" ) );
            ASSERT_TRUE( "New job" == CellOfKey( form.GetJobsBinding(), "job-4", "title" ) );

            ASSERT_TRUE( DeleteTestRows( *fixture.tables.jobs, "jobId", "job-3" ) );
            fixture.tables.jobs->RaiseRowsChanged();
            ASSERT_TRUE( 3 == form.GetJobsBinding().GetRowCount() );
            ASSERT_TRUE( "(no such row)" == CellOfKey( form.GetJobsBinding(), "job-3", "title" ) );

            // the runs and the lanes follow their tables as well
            rows.clear();
            rows.push_back( CTestRow().Num( "runId", 103 ).Text( "jobId", "job-2" ).Text( "state", "QUEUED" ) );
            ASSERT_TRUE( UpsertTestRows( *fixture.tables.runs, rows ) );
            fixture.tables.runs->RaiseRowsChanged();
            ASSERT_TRUE( 5 == form.GetRunsBinding().GetRowCount() );
            // the newest run first
            CORE::CString first;
            ASSERT_TRUE( form.GetRunsBinding().GetCellText( 0, "runId", first ) && "200" == first );

            // the tabs
            ASSERT_TRUE( DRNUTILSGUI::CSchedulerForm::TAB_JOBS == form.GetShownTab() );
            form.ShowTab( DRNUTILSGUI::CSchedulerForm::TAB_RUNS );
            ASSERT_TRUE( DRNUTILSGUI::CSchedulerForm::TAB_RUNS == form.GetShownTab() );
            form.ShowTab( DRNUTILSGUI::CSchedulerForm::TAB_JOBS );

            // the status line: host, revision, connection and the time zone of the display
            const CORE::CString status = form.GetStatusText();
            ASSERT_TRUE( status.HasSubstr( "node-1" ) >= 0 && status.HasSubstr( "revision 42" ) >= 0 && status.HasSubstr( "times in UTC" ) >= 0 );
            ASSERT_TRUE( TextOfWidget( form, "Status" ) == status );
            ASSERT_TRUE( fixture.client.infoRequests >= 1 );

            // for people to look at (GUCEF_TEST_SHOT_DIR), not part of what the test checks
            form.SelectJob( "job-1" );
            fixture.Settle();
            WriteShot( CaptureFrame( fixture.context ), "SchedulerForm_Jobs" );
            form.ShowTab( DRNUTILSGUI::CSchedulerForm::TAB_RUNS );
            fixture.Settle();
            WriteShot( CaptureFrame( fixture.context ), "SchedulerForm_Runs" );
            form.ShowTab( DRNUTILSGUI::CSchedulerForm::TAB_JOBS );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T1: the hints of the table win: a hidden column is not shown, a title of the table replaces the title of the form" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( GUCEF_NULL != fixture.context && fixture.tables.Fill() );
        if ( GUCEF_NULL != fixture.context )
        {
            fixture.tables.jobs->SetHint( "lane", "hidden", CORE::CVariant( true ) );
            fixture.tables.jobs->SetHint( "title", "title", CORE::CVariant( CORE::CString( "Name of the job" ) ) );
            fixture.tables.jobs->SetHint( "jobId", "hidden", CORE::CVariant( true ) );        // the key stays
            fixture.tables.jobs->SetHint( "runCount", "width", CORE::CVariant( (Int32) 321 ) );
            fixture.form.SetTables( fixture.tables.GetSet() );
            ASSERT_TRUE( fixture.form.Open() );

            const DRNUTILSGUI::CTimeDisplayTableModel* model = fixture.form.GetJobsBinding().GetModel();
            ASSERT_TRUE( GUCEF_NULL != model );
            if ( GUCEF_NULL != model )
            {
                ASSERT_TRUE( GUCEF_UINT32MAX == model->FindShownColumn( "lane" ) );
                ASSERT_TRUE( GUCEF_UINT32MAX != model->FindShownColumn( "jobId" ) );
                const UInt32 title = model->FindShownColumn( "title" );
                ASSERT_TRUE( GUCEF_UINT32MAX != title && "Name of the job" == model->GetColumn( title ).title );
                const UInt32 runs = model->FindShownColumn( "runCount" );
                ASSERT_TRUE( GUCEF_UINT32MAX != runs && 321 == model->GetColumn( runs ).width );
                ASSERT_TRUE( 150 == model->GetColumn( 0 ).width );      // the width of the form where the table has no hint
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T2: the buttons follow the allowed actions of the host and the state of the selected row" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CSchedulerForm& form = fixture.form;

            // nothing selected: nothing to act on
            ASSERT_TRUE( !fixture.Enabled( "RunNow" ) && !fixture.Enabled( "CancelRun" ) && !fixture.Enabled( "Enable" ) && !fixture.Enabled( "Disable" ) );

            form.SelectJob( "job-1" );
            ASSERT_TRUE( "job-1" == form.GetSelectedJobId() );
            ASSERT_TRUE( fixture.Enabled( "RunNow" ) && fixture.Enabled( "Disable" ) && !fixture.Enabled( "Enable" ) && !fixture.Enabled( "CancelRun" ) );
            ASSERT_TRUE( form.IsActionEnabled( CORE::JOBCONTROLACTION_RUN_NOW ) && !form.IsActionEnabled( CORE::JOBCONTROLACTION_ENABLE ) );
            ASSERT_TRUE( fixture.Enabled( "History" ) );

            // a disabled job can be enabled, not run
            form.SelectJob( "job-3" );
            ASSERT_TRUE( !fixture.Enabled( "RunNow" ) && !fixture.Enabled( "Disable" ) && fixture.Enabled( "Enable" ) );

            // a run that is over cannot be cancelled, one that runs can
            form.SelectRun( 100 );
            ASSERT_TRUE( 100 == form.GetSelectedRunId() && !fixture.Enabled( "CancelRun" ) );
            form.SelectRun( 102 );
            ASSERT_TRUE( 102 == form.GetSelectedRunId() && fixture.Enabled( "CancelRun" ) );

            // the host takes the right to run now away from the viewer: the button follows, the others stay
            DRNUTILS::SSchedulerHostInfo info = MakeSchedulerInfo( "node-1", true );
            info.allowedActions.erase( info.allowedActions.begin() );
            fixture.client.AnnounceInfo( info );
            fixture.Settle();
            form.SelectJob( "job-1" );
            ASSERT_TRUE( !fixture.Enabled( "RunNow" ) && fixture.Enabled( "Disable" ) && fixture.Enabled( "CancelRun" ) );

            // a viewer without any control right gets nothing enabled
            fixture.client.AnnounceInfo( MakeSchedulerInfo( "node-1", false ) );
            fixture.Settle();
            ASSERT_TRUE( !fixture.Enabled( "RunNow" ) && !fixture.Enabled( "CancelRun" ) && !fixture.Enabled( "Enable" ) && !fixture.Enabled( "Disable" ) );
            ASSERT_TRUE( !form.RunSelectedJobNow( "no" ) && fixture.client.commands.empty() );

            // the state of the row changes through the table: the buttons follow
            fixture.client.AnnounceInfo( MakeSchedulerInfo( "node-1", true ) );
            fixture.Settle();
            TTestRowVector rows;
            rows.push_back( CTestRow().Text( "jobId", "job-1" ).Text( "state", "DISABLED" ) );
            ASSERT_TRUE( UpsertTestRows( *fixture.tables.jobs, rows ) );
            fixture.tables.jobs->RaiseRowsChanged();
            fixture.Settle();
            form.SelectJob( "job-1" );
            ASSERT_TRUE( !fixture.Enabled( "RunNow" ) && fixture.Enabled( "Enable" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T2 and UI-T3: a control action asks first, then sends one command with the reason; a refusal is shown" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CSchedulerForm& form = fixture.form;
            DRNUTILSGUI::CControlConfirmForm* confirm = form.GetConfirmForm();
            ASSERT_TRUE( GUCEF_NULL != confirm );
            if ( GUCEF_NULL != confirm )
            {
                // pressing Run now asks, and calls nothing yet
                form.SelectJob( "job-1" );
                ClickWidget( fixture.context, form.GetWidget( "RunNow" ) );
                ASSERT_TRUE( confirm->IsOpen() && fixture.client.commands.empty() );
                ASSERT_TRUE( confirm->GetTargetText().HasSubstr( "job-1" ) >= 0 && confirm->GetTargetText().HasSubstr( "Fetch quotes" ) >= 0 );

                // No: nothing is sent
                ClickWidget( fixture.context, confirm->GetWidget( "Cancel" ) );
                ASSERT_TRUE( !confirm->IsOpen() && fixture.client.commands.empty() );

                // Yes with a reason: one call, the reason in it, the answer does not block the form
                ClickWidget( fixture.context, form.GetWidget( "RunNow" ) );
                ASSERT_TRUE( confirm->IsOpen() );
                confirm->GetWidget( "Reason" )->SetProperty( "Text", "operator check" );
                ClickWidget( fixture.context, confirm->GetWidget( "Confirm" ) );
                ASSERT_TRUE( !confirm->IsOpen() && 1 == fixture.client.commands.size() );
                if ( 1 == fixture.client.commands.size() )
                {
                    const CORE::SJobControlCommand& command = fixture.client.commands[ 0 ];
                    ASSERT_TRUE( CORE::JOBCONTROLACTION_RUN_NOW == command.action && CORE::JOBCONTROLTARGET_JOB == command.targetKind );
                    ASSERT_TRUE( "job-1" == command.targetId && "operator check" == command.reason );
                }
                const CORE::CString firstCommandId = fixture.client.lastCommandId;
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "Run now of job job-1" ) >= 0 && form.GetLastCommandText().HasSubstr( "sent" ) >= 0 );

                // the host refuses: the message of the host is shown
                fixture.client.AnnounceOutcome( firstCommandId, DRNUTILS::DRNUTILS_STATUS_PERMISSION_DENIED, "the role scheduler.control is required" );
                fixture.Settle();
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "refused" ) >= 0 && form.GetLastCommandText().HasSubstr( "the role scheduler.control is required" ) >= 0 );
                ASSERT_TRUE( TextOfWidget( form, "Result" ) == form.GetLastCommandText() );

                // answers to commands of somebody else are not the business of the form
                const CORE::CString shown = form.GetLastCommandText();
                fixture.client.AnnounceOutcome( "somebody-else", DRNUTILS::DRNUTILS_STATUS_OK, "" );
                fixture.Settle();
                ASSERT_TRUE( shown == form.GetLastCommandText() );

                // Disable of the job, a second command with its own id, accepted this time
                ClickWidget( fixture.context, form.GetWidget( "Disable" ) );
                ASSERT_TRUE( confirm->IsOpen() );
                confirm->Confirm( "maintenance" );
                ASSERT_TRUE( 2 == fixture.client.commands.size() );
                if ( 2 == fixture.client.commands.size() )
                    ASSERT_TRUE( CORE::JOBCONTROLACTION_DISABLE == fixture.client.commands[ 1 ].action && "job-1" == fixture.client.commands[ 1 ].targetId && "maintenance" == fixture.client.commands[ 1 ].reason );
                ASSERT_TRUE( fixture.client.lastCommandId != firstCommandId );
                fixture.client.AnnounceOutcome( fixture.client.lastCommandId, DRNUTILS::DRNUTILS_STATUS_OK, "" );
                fixture.Settle();
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "Disable of job job-1: done" ) >= 0 );

                // Cancel run on a selected run, targeted at the run
                form.SelectRun( 102 );
                ClickWidget( fixture.context, form.GetWidget( "CancelRun" ) );
                ASSERT_TRUE( confirm->IsOpen() && confirm->GetTargetText().HasSubstr( "102" ) >= 0 );
                confirm->Confirm( "stuck" );
                ASSERT_TRUE( 3 == fixture.client.commands.size() );
                if ( 3 == fixture.client.commands.size() )
                    ASSERT_TRUE( CORE::JOBCONTROLACTION_CANCEL_RUN == fixture.client.commands[ 2 ].action && CORE::JOBCONTROLTARGET_RUN == fixture.client.commands[ 2 ].targetKind && "102" == fixture.client.commands[ 2 ].targetId );

                // a command the client cannot send is said so
                fixture.client.refuseToSend = true;
                ASSERT_TRUE( !form.SetSelectedJobEnabled( false, "no node" ) );
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "could not be sent" ) >= 0 );
                ASSERT_TRUE( 3 == fixture.client.commands.size() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "C-2: the details panel asks the client for the detail of the selected job (Scheduler/GetJob) and shows it; answers to other requests are ignored, a refusal is shown" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CSchedulerForm& form = fixture.form;
            form.SelectJob( "job-3" );
            fixture.Settle();
            ASSERT_TRUE( !fixture.client.detailJobs.empty() && "job-3" == fixture.client.detailJobs.back() );
            fixture.client.detailJobs.clear();
            form.SelectJob( "job-1" );
            fixture.Settle();
            ASSERT_TRUE( 1 == fixture.client.detailJobs.size() && "job-1" == fixture.client.detailJobs.back() );
            const CORE::CString requestId = fixture.client.lastDetailRequestId;
            DRNUTILS::SSchedulerJobDetailAnswer probe;
            ASSERT_FALSE( form.GetSelectedJobDetail( probe ) );
            // the same selection again asks nothing new
            form.SelectJob( "job-1" );
            fixture.Settle();
            ASSERT_TRUE( 1 == fixture.client.detailJobs.size() );

            // an answer to another request goes nowhere
            DRNUTILS::SSchedulerJobDetailAnswer other;
            other.requestId = "another-request";
            other.jobId = "job-1";
            other.detail.job.description = "not this one";
            fixture.client.AnnounceJobDetail( other );
            fixture.Settle();
            ASSERT_FALSE( form.GetSelectedJobDetail( probe ) );
            ASSERT_TRUE( 0 > form.GetJobDetailsText().HasSubstr( "not this one" ) );

            DRNUTILS::SSchedulerJobDetailAnswer answer;
            answer.requestId = requestId;
            answer.jobId = "job-1";
            answer.detail.job.jobId = "job-1";
            answer.detail.job.description = "Fetches the quotes of the watch list";
            DRNUTILS::SSchedulerActionInfo runNow;
            runNow.action = CORE::JOBCONTROLACTION_RUN_NOW;
            runNow.targetKind = CORE::JOBCONTROLTARGET_JOB;
            answer.allowedActions.push_back( runNow );
            CORE::SJobRunRecord run;
            run.runId = 100;
            run.jobId = "job-1";
            run.state = CORE::JOBRUNSTATE_SUCCEEDED;
            answer.detail.recentRuns.push_back( run );
            answer.detail.sourceStatus.SetAttribute( "pendingSymbols", "12" );
            fixture.client.AnnounceJobDetail( answer );
            fixture.Settle();
            ASSERT_TRUE( form.GetSelectedJobDetail( probe ) && "job-1" == probe.jobId );
            const CORE::CString details = form.GetJobDetailsText();
            ASSERT_TRUE( details.HasSubstr( "Fetch quotes" ) >= 0 );
            ASSERT_TRUE( details.HasSubstr( "Description: Fetches the quotes of the watch list" ) >= 0 );
            ASSERT_TRUE( details.HasSubstr( "Allowed actions: RUN_NOW" ) >= 0 );
            ASSERT_TRUE( details.HasSubstr( "Recent runs: 1 (last 100: SUCCEEDED)" ) >= 0 );
            ASSERT_TRUE( details.HasSubstr( "Work source: pendingSymbols=12" ) >= 0 );
            ASSERT_TRUE( TextOfWidget( form, "Details" ) == details );

            // another job: a new request, the old answer is not shown for it; a refusal says why
            form.SelectJob( "job-2" );
            fixture.Settle();
            ASSERT_TRUE( 2 == fixture.client.detailJobs.size() && "job-2" == fixture.client.detailJobs.back() );
            ASSERT_TRUE( 0 > form.GetJobDetailsText().HasSubstr( "Fetches the quotes" ) );
            DRNUTILS::SSchedulerJobDetailAnswer refused;
            refused.requestId = fixture.client.lastDetailRequestId;
            refused.jobId = "job-2";
            refused.status = DRNUTILS::DRNUTILS_STATUS_PERMISSION_DENIED;
            refused.statusMessage = "no scheduler.view";
            fixture.client.AnnounceJobDetail( refused );
            fixture.Settle();
            ASSERT_TRUE( form.GetJobDetailsText().HasSubstr( "Detail: PERMISSION_DENIED (no scheduler.view)" ) >= 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The form without client or tables stays empty and refuses to act" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            DRNUTILSGUI::CSchedulerForm form( context );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( 0 == form.GetJobsBinding().GetRowCount() && !form.GetJobsBinding().IsBound() );
            ASSERT_TRUE( !form.IsActionEnabled( CORE::JOBCONTROLACTION_RUN_NOW ) && !form.RunSelectedJobNow( "x" ) && !form.OpenRunHistory() );
            ASSERT_TRUE( !IsWidgetEnabled( form, "RunNow" ) );
            form.Close();
            ASSERT_TRUE( !form.IsOpen() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformJobRunHistoryFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.JobRunHistoryForm" );

    GUCEF_TESTFW_TESTCASE( "The runs of one job, newest first, with the progress of the selected run; Cancel run asks first" )
    try
    {
        SSchedulerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CSchedulerForm& form = fixture.form;
            form.SelectJob( "job-1" );
            ASSERT_TRUE( form.OpenRunHistory() );
            DRNUTILSGUI::CJobRunHistoryForm* history = form.GetRunHistoryForm();
            ASSERT_TRUE( GUCEF_NULL != history );
            if ( GUCEF_NULL != history )
            {
                ASSERT_TRUE( history->IsOpen() && "job-1" == history->GetJobId() );
                DRNUTILSGUI::CTableGridBinding& binding = history->GetBinding();

                // only the runs of the job, the newest first
                ASSERT_TRUE( 3 == binding.GetRowCount() );
                CORE::CString text;
                ASSERT_TRUE( binding.GetCellText( 0, "runId", text ) && "102" == text );
                ASSERT_TRUE( binding.GetCellText( 2, "runId", text ) && "100" == text );
                ASSERT_TRUE( "Runs of job job-1" == TextOfWidget( *history, "Header" ) );
                ASSERT_TRUE( !IsWidgetEnabled( *history, "CancelRun" ) );

                // a run with a total shows its progress, a bar and the text
                history->SelectRun( 102 );
                ASSERT_TRUE( 102 == history->GetSelectedRunId() );
                ASSERT_TRUE( "12 of 40" == history->GetProgressText() && "12 of 40" == TextOfWidget( *history, "ProgressText" ) );
                ASSERT_TRUE( history->GetProgressPercentage() > 29.9f && history->GetProgressPercentage() < 30.1f );
                ASSERT_TRUE( IsWidgetEnabled( *history, "CancelRun" ) );
                history->SelectRun( 101 );
                ASSERT_TRUE( history->GetProgressText().IsNULLOrEmpty() && !IsWidgetEnabled( *history, "CancelRun" ) );

                // the form follows the table: a new run of the job, one of another job
                TTestRowVector rows;
                rows.push_back( CTestRow().Num( "runId", 103 ).Text( "jobId", "job-1" ).Text( "state", "QUEUED" ) );
                rows.push_back( CTestRow().Num( "runId", 201 ).Text( "jobId", "job-2" ).Text( "state", "QUEUED" ) );
                ASSERT_TRUE( UpsertTestRows( *fixture.tables.runs, rows ) );
                fixture.tables.runs->RaiseRowsChanged();
                ASSERT_TRUE( 4 == binding.GetRowCount() );
                ASSERT_TRUE( binding.GetCellText( 0, "runId", text ) && "103" == text );

                // Cancel run: asks, then one command at the run, the reason with it
                history->SelectRun( 102 );
                DRNUTILSGUI::CControlConfirmForm* confirm = history->GetConfirmForm();
                ASSERT_TRUE( GUCEF_NULL != confirm );
                if ( GUCEF_NULL != confirm )
                {
                    ClickWidget( fixture.context, history->GetWidget( "CancelRun" ) );
                    ASSERT_TRUE( confirm->IsOpen() && fixture.client.commands.empty() );
                    confirm->Confirm( "hanging" );
                    ASSERT_TRUE( 1 == fixture.client.commands.size() );
                    if ( 1 == fixture.client.commands.size() )
                    {
                        ASSERT_TRUE( CORE::JOBCONTROLACTION_CANCEL_RUN == fixture.client.commands[ 0 ].action && CORE::JOBCONTROLTARGET_RUN == fixture.client.commands[ 0 ].targetKind );
                        ASSERT_TRUE( "102" == fixture.client.commands[ 0 ].targetId && "hanging" == fixture.client.commands[ 0 ].reason );
                    }
                    fixture.client.AnnounceOutcome( fixture.client.lastCommandId, DRNUTILS::DRNUTILS_STATUS_FAILED_PRECONDITION, "the run is over" );
                    fixture.Settle();
                    ASSERT_TRUE( history->GetLastCommandText().HasSubstr( "was refused: the run is over" ) >= 0 );
                }

                // another job
                history->SetJobId( "job-2" );
                ASSERT_TRUE( 2 == binding.GetRowCount() );

                // closing tells
                CEventCount closed( history, DRNUTILSGUI::CJobRunHistoryForm::ClosedEvent );
                ClickWidget( fixture.context, history->GetWidget( "Close" ) );
                ASSERT_TRUE( !history->IsOpen() && 1 == closed.count );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
