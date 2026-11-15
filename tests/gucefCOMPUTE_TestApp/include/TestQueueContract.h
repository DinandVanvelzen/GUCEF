/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_COMPUTE_TESTAPP_TESTQUEUECONTRACT_H
#define GUCEF_COMPUTE_TESTAPP_TESTQUEUECONTRACT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The contract of queues that every backend shares: a failed submission takes no timeline value, fences are
 *  host-visible completion (a backend whose native timeline signals early does its host copies first), synchronous
 *  execution holds no lock while it waits, cooperative completion through CComputeGlobal::Update(), and a queue whose
 *  completion task is stopped by its pool fails its pending work and closes without skipping a value or leaking.
 */
void
PerformQueueContractTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TESTAPP_TESTQUEUECONTRACT_H ? */
