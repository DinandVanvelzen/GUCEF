/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTTASKMANAGERCOUNTERS_H
#define GUCEF_CORE_TESTAPP_TESTTASKMANAGERCOUNTERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The task manager delta (design 15.2, TM-T1..T4) and the scheduler registry of the task manager (JS-T14) */
void
PerformTaskManagerCounterTests( void );

/*-------------------------------------------------------------------------*/

/** started by a test as the process whose task manager shuts down with a running job scheduler */
#define GUCEF_CORE_TESTAPP_SCHEDULER_SHUTDOWN_ARG    "--job-scheduler-shutdown"

/** The child of that test: exit code 0 when the scheduler was stopped, with its state saved, before its pools */
int
RunJobSchedulerShutdownChild( void );

/**
 *  Started by the test "a pool task that ends while the process exits": the child
 *  queues a task on a pool with a worker thread and returns from main while the task
 *  still runs, so the task ends while the process runs its exit handlers and static
 *  destructors (finding B-6 of the gucefDRNUTILS freeze). Exits 0 when that is safe.
 */
#define GUCEF_CORE_TESTAPP_POOL_TASK_ENDS_AT_EXIT_ARG    "--pool-task-ends-at-exit"

int
RunPoolTaskEndsAtExitChild( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTTASKMANAGERCOUNTERS_H ? */
