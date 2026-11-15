/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#ifndef GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTWORKQUEUEBENCHMARK_H
#define GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTWORKQUEUEBENCHMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Report only measurements of the work queue on the GUCEF thread pool (COgreTaskWorkQueue) against Ogre's own
 *  DefaultWorkQueue with the same number of worker threads and the same workloads: throughput and overhead per task,
 *  the time from addTask() to the start of the task, processMainThreadTasks() per call, addTask() from four threads,
 *  the processor time of idle workers and the memory allocations per task. Opt in with the environment variable
 *  GUCEF_TEST_OGRE_BENCH=1, skipped otherwise; meant for a release build. The numbers go to the log; the tests fail
 *  only when something could not be set up, never on a time.
 */
void
PerformWorkQueueBenchmarkTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTWORKQUEUEBENCHMARK_H ? */
