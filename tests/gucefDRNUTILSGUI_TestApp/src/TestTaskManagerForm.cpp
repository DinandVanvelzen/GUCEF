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

/** The task manager form on the null driver with its tables, a client that records and a host that allows the control; times shown in UTC */
struct STaskManagerFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    STestTaskManagerTables tables;
    CFakeTaskManagerClient client;
    DRNUTILSGUI::CTaskManagerForm form;

    STaskManagerFixture( void )
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
        client.AnnounceInfo( MakeTaskManagerInfo( "node-1", allowControl ) );
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
PerformTaskManagerFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.TaskManagerForm" );

    GUCEF_TESTFW_TESTCASE( "UI-T1: the form binds to the pools, threads, tasks and scheduled jobs and follows RowsChanged" )
    try
    {
        STaskManagerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CTaskManagerForm& form = fixture.form;
            ASSERT_TRUE( form.GetPoolsBinding().IsBound() && form.GetThreadsBinding().IsBound() && form.GetTasksBinding().IsBound() && form.GetScheduledJobsBinding().IsBound() );
            ASSERT_TRUE( 2 == form.GetPoolsBinding().GetRowCount() && 1 == form.GetThreadsBinding().GetRowCount() );
            ASSERT_TRUE( 2 == form.GetTasksBinding().GetRowCount() && 2 == form.GetScheduledJobsBinding().GetRowCount() );

            const DRNUTILSGUI::CTimeDisplayTableModel* pools = form.GetPoolsBinding().GetModel();
            ASSERT_TRUE( GUCEF_NULL != pools );
            if ( GUCEF_NULL != pools )
            {
                ASSERT_TRUE( "Pool" == pools->GetColumn( 0 ).title && "name" == pools->GetColumn( 0 ).name );
                const UInt32 workers = pools->FindShownColumn( "activeWorkers" );
                ASSERT_TRUE( GUCEF_UINT32MAX != workers && "Workers" == pools->GetColumn( workers ).title );
            }
            ASSERT_TRUE( "4" == CellOfKey( form.GetPoolsBinding(), "workers", "activeWorkers" ) );

            // the scheduled jobs tab shows which job runs on the main thread and which on a pool
            ASSERT_TRUE( "MAIN_THREAD" == CellOfKey( form.GetScheduledJobsBinding(), "main/job-2", "executionTarget" ) );
            ASSERT_TRUE( "POOL" == CellOfKey( form.GetScheduledJobsBinding(), "main/job-1", "executionTarget" ) && "workers" == CellOfKey( form.GetScheduledJobsBinding(), "main/job-1", "pool" ) );

            // the times of a task, in the display zone
            ASSERT_TRUE( "2026-03-08 06:59:00 UTC" == CellOfKey( form.GetTasksBinding(), "55", "startedUtc" ) );

            CEventCount changed( &form.GetPoolsBinding(), DRNUTILSGUI::CTableGridBinding::ModelChangedEvent );
            TTestRowVector rows;
            rows.push_back( CTestRow().Text( "name", "workers" ).Num( "activeWorkers", 6 ).Num( "desiredMinWorkers", 2 ).Num( "desiredMaxThreads", 8 ).Num( "queued", 0 ).Num( "running", 6 ) );
            ASSERT_TRUE( UpsertTestRows( *fixture.tables.pools, rows ) );
            fixture.tables.pools->RaiseRowsChanged();
            ASSERT_TRUE( changed.count >= 1 && "6" == CellOfKey( form.GetPoolsBinding(), "workers", "activeWorkers" ) );

            rows.clear();
            rows.push_back( CTestRow().Num( "taskId", 57 ).Text( "pool", "workers" ).Text( "taskType", "FetchTask" ).Text( "status", "QUEUED" ) );
            ASSERT_TRUE( UpsertTestRows( *fixture.tables.tasks, rows ) );
            fixture.tables.tasks->RaiseRowsChanged();
            ASSERT_TRUE( "QUEUED" == CellOfKey( form.GetTasksBinding(), "57", "status" ) );

            // for people to look at (GUCEF_TEST_SHOT_DIR), not part of what the test checks
            form.SelectPool( "workers" );
            fixture.Settle();
            WriteShot( CaptureFrame( fixture.context ), "TaskManagerForm_Pools" );

            // the tabs
            ASSERT_TRUE( DRNUTILSGUI::CTaskManagerForm::TAB_POOLS == form.GetShownTab() );
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_SCHEDULED_JOBS );
            ASSERT_TRUE( DRNUTILSGUI::CTaskManagerForm::TAB_SCHEDULED_JOBS == form.GetShownTab() );
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_POOLS );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Viewing mode: the form views while it is open, only watches what its shown tab needs, and stops when it closes" )
    try
    {
        STaskManagerFixture fixture;
        ASSERT_TRUE( GUCEF_NULL != fixture.context && fixture.tables.Fill() );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CTaskManagerForm& form = fixture.form;
            form.SetTables( fixture.tables.GetSet() );
            form.SetClient( &fixture.client );

            // not open: nothing is watched
            ASSERT_TRUE( !form.IsViewing() && fixture.client.viewings.empty() );

            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( form.IsViewing() && 1 == fixture.client.viewings.size() && fixture.client.infoRequests >= 1 );
            if ( 1 == fixture.client.viewings.size() )
                ASSERT_TRUE( 2000 == fixture.client.viewings[ 0 ].sampleIntervalMs && !fixture.client.viewings[ 0 ].includeThreads && !fixture.client.viewings[ 0 ].includeTasks );

            // the tasks are only sampled while their tab is shown
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_TASKS );
            ASSERT_TRUE( 2 == fixture.client.viewings.size() && 1 == fixture.client.stopViewingCalls );
            if ( 2 == fixture.client.viewings.size() )
                ASSERT_TRUE( !fixture.client.viewings[ 1 ].includeThreads && fixture.client.viewings[ 1 ].includeTasks );
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_THREADS );
            ASSERT_TRUE( 3 == fixture.client.viewings.size() );
            if ( 3 == fixture.client.viewings.size() )
                ASSERT_TRUE( fixture.client.viewings[ 2 ].includeThreads && !fixture.client.viewings[ 2 ].includeTasks );
            // the scheduled jobs come with the overview of the pools: no extra sampling
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_SCHEDULED_JOBS );
            ASSERT_TRUE( 4 == fixture.client.viewings.size() );
            if ( 4 == fixture.client.viewings.size() )
                ASSERT_TRUE( !fixture.client.viewings[ 3 ].includeThreads && !fixture.client.viewings[ 3 ].includeTasks );
            form.ShowTab( DRNUTILSGUI::CTaskManagerForm::TAB_SCHEDULED_JOBS );
            ASSERT_TRUE( 4 == fixture.client.viewings.size() );

            // the sample interval: at least a second, the selector shows it
            form.SetSampleIntervalMs( 10000 );
            ASSERT_TRUE( 10000 == form.GetSampleIntervalMs() && 5 == fixture.client.viewings.size() );
            if ( 5 == fixture.client.viewings.size() )
                ASSERT_TRUE( 10000 == fixture.client.viewings[ 4 ].sampleIntervalMs );
            GUI::CCombobox* interval = form.GetWidgetAs< GUI::CCombobox >( "SampleInterval" );
            ASSERT_TRUE( GUCEF_NULL != interval && 3 == interval->GetSelectedIndex() );
            form.SetSampleIntervalMs( 100 );
            ASSERT_TRUE( 1000 == form.GetSampleIntervalMs() && 0 == interval->GetSelectedIndex() );
            ASSERT_TRUE( form.GetStatusText().HasSubstr( "viewing every 1000 ms" ) >= 0 );

            // closing ends the viewing, opening starts it again
            const UInt32 stopsBefore = fixture.client.stopViewingCalls;
            CEventCount closed( &form, DRNUTILSGUI::CTaskManagerForm::ClosedEvent );
            ClickWidget( fixture.context, form.GetWidget( "Close" ) );
            ASSERT_TRUE( !form.IsOpen() && !form.IsViewing() && 1 == closed.count && fixture.client.stopViewingCalls == stopsBefore + 1 );
            const UInt32 viewingsBefore = (UInt32) fixture.client.viewings.size();
            ASSERT_TRUE( form.Open() && form.IsViewing() && fixture.client.viewings.size() == viewingsBefore + 1 );

            // taking the client away ends the viewing on it
            const UInt32 stops = fixture.client.stopViewingCalls;
            form.SetClient( GUCEF_NULL );
            ASSERT_TRUE( !form.IsViewing() && fixture.client.stopViewingCalls == stops + 1 );
            form.SetClient( &fixture.client );
            ASSERT_TRUE( form.IsViewing() );
        }
        // destroying an open form ends its viewing
        const UInt32 stopsOfClient = fixture.client.stopViewingCalls;
        {
            DRNUTILSGUI::CTaskManagerForm shortLived( fixture.context );
            shortLived.SetClient( &fixture.client );
            ASSERT_TRUE( shortLived.Open() );
        }
        ASSERT_TRUE( fixture.client.stopViewingCalls == stopsOfClient + 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T2: the buttons follow the capabilities of the host, the pools it lets us control and the selection" )
    try
    {
        STaskManagerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CTaskManagerForm& form = fixture.form;
            ASSERT_TRUE( !fixture.Enabled( "ResizePool" ) && !fixture.Enabled( "CancelTask" ) && !fixture.Enabled( "StopTask" ) );

            form.SelectPool( "workers" );
            ASSERT_TRUE( "workers" == form.GetSelectedPool() && fixture.Enabled( "ResizePool" ) );
            // the pool without worker threads of the main thread jobs is not one the host lets us resize
            form.SelectPool( "gucef.scheduler.main" );
            ASSERT_TRUE( !fixture.Enabled( "ResizePool" ) && !form.IsActionEnabled( DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL ) );

            form.SelectTask( 55 );
            ASSERT_TRUE( 55 == form.GetSelectedTaskId() && fixture.Enabled( "CancelTask" ) && fixture.Enabled( "StopTask" ) );

            // a host that does not let the viewer stop tasks
            DRNUTILS::STaskManagerHostInfo info = MakeTaskManagerInfo( "node-1", true );
            info.capabilities.erase( "stopTask" );
            fixture.client.AnnounceInfo( info );
            fixture.Settle();
            ASSERT_TRUE( fixture.Enabled( "CancelTask" ) && !fixture.Enabled( "StopTask" ) );

            // a viewer that may only look
            fixture.client.AnnounceInfo( MakeTaskManagerInfo( "node-1", false ) );
            fixture.Settle();
            form.SelectPool( "workers" );
            ASSERT_TRUE( !fixture.Enabled( "ResizePool" ) && !fixture.Enabled( "CancelTask" ) && !fixture.Enabled( "StopTask" ) );
            ASSERT_TRUE( !form.CancelSelectedTask( "no" ) && !form.StopSelectedTask( "no" ) && !form.ResizeSelectedPool( 1, 2, "no" ) && fixture.client.commands.empty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "UI-T2 and UI-T3: resize, cancel and stop ask first, send one command with the reason, and show a refusal" )
    try
    {
        STaskManagerFixture fixture;
        ASSERT_TRUE( fixture.Init( true ) );
        if ( GUCEF_NULL != fixture.context )
        {
            DRNUTILSGUI::CTaskManagerForm& form = fixture.form;
            DRNUTILSGUI::CControlConfirmForm* confirm = form.GetConfirmForm();
            ASSERT_TRUE( GUCEF_NULL != confirm );
            if ( GUCEF_NULL != confirm )
            {
                // the fields of the resize start with what the pool has
                form.SelectPool( "workers" );
                ASSERT_TRUE( "2" == TextOfWidget( form, "MinWorkers" ) && "8" == TextOfWidget( form, "MaxThreads" ) );

                // wrong input is said at once, nothing is asked
                form.GetWidget( "MinWorkers" )->SetProperty( "Text", "many" );
                ClickWidget( fixture.context, form.GetWidget( "ResizePool" ) );
                ASSERT_TRUE( !confirm->IsOpen() && fixture.client.commands.empty() && form.GetLastCommandText().HasSubstr( "not sent" ) >= 0 );
                form.GetWidget( "MinWorkers" )->SetProperty( "Text", "9" );
                form.GetWidget( "MaxThreads" )->SetProperty( "Text", "4" );
                ClickWidget( fixture.context, form.GetWidget( "ResizePool" ) );
                ASSERT_TRUE( !confirm->IsOpen() && form.GetLastCommandText().HasSubstr( "cannot be below" ) >= 0 );

                // good input asks, then sends the numbers and the reason
                form.GetWidget( "MinWorkers" )->SetProperty( "Text", "3" );
                form.GetWidget( "MaxThreads" )->SetProperty( "Text", "-1" );
                ClickWidget( fixture.context, form.GetWidget( "ResizePool" ) );
                ASSERT_TRUE( confirm->IsOpen() && fixture.client.commands.empty() );
                ASSERT_TRUE( confirm->GetTargetText().HasSubstr( "workers" ) >= 0 && confirm->GetTargetText().HasSubstr( "no maximum" ) >= 0 );
                confirm->GetWidget( "Reason" )->SetProperty( "Text", "more load" );
                ClickWidget( fixture.context, confirm->GetWidget( "Confirm" ) );
                ASSERT_TRUE( 1 == fixture.client.commands.size() );
                if ( 1 == fixture.client.commands.size() )
                {
                    const DRNUTILS::STaskManagerControlCommand& command = fixture.client.commands[ 0 ];
                    ASSERT_TRUE( DRNUTILS::DRNUTILS_TASKACTION_RESIZE_POOL == command.action && "workers" == command.poolName );
                    ASSERT_TRUE( 3 == command.minWorkers && -1 == command.maxThreads && "more load" == command.reason );
                }
                fixture.client.AnnounceOutcome( fixture.client.lastCommandId, DRNUTILS::DRNUTILS_STATUS_RESOURCE_EXHAUSTED, "the node has no more threads" );
                fixture.Settle();
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "Resize of pool workers was refused: the node has no more threads" ) >= 0 );
                ASSERT_TRUE( TextOfWidget( form, "Result" ) == form.GetLastCommandText() );

                // cancel a task: asks, No sends nothing, Yes sends the command with the task and its pool
                form.SelectTask( 55 );
                ClickWidget( fixture.context, form.GetWidget( "CancelTask" ) );
                ASSERT_TRUE( confirm->IsOpen() );
                ClickWidget( fixture.context, confirm->GetWidget( "Cancel" ) );
                ASSERT_TRUE( 1 == fixture.client.commands.size() );
                ClickWidget( fixture.context, form.GetWidget( "CancelTask" ) );
                confirm->Confirm( "stuck" );
                ASSERT_TRUE( 2 == fixture.client.commands.size() );
                if ( 2 == fixture.client.commands.size() )
                {
                    const DRNUTILS::STaskManagerControlCommand& command = fixture.client.commands[ 1 ];
                    ASSERT_TRUE( DRNUTILS::DRNUTILS_TASKACTION_CANCEL_TASK == command.action && 55 == command.taskId && "workers" == command.poolName && "stuck" == command.reason );
                }
                fixture.client.AnnounceOutcome( fixture.client.lastCommandId, DRNUTILS::DRNUTILS_STATUS_OK, "" );
                fixture.Settle();
                ASSERT_TRUE( form.GetLastCommandText().HasSubstr( "Cancel of task 55: done" ) >= 0 );

                // stop a task
                ClickWidget( fixture.context, form.GetWidget( "StopTask" ) );
                ASSERT_TRUE( confirm->IsOpen() );
                confirm->Confirm( "shutdown of the service" );
                ASSERT_TRUE( 3 == fixture.client.commands.size() );
                if ( 3 == fixture.client.commands.size() )
                    ASSERT_TRUE( DRNUTILS::DRNUTILS_TASKACTION_STOP_TASK == fixture.client.commands[ 2 ].action && 55 == fixture.client.commands[ 2 ].taskId );
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
