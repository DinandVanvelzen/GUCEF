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

#ifndef GUCEF_CORE_TESTAPP_TESTTASKDELEGATORWAKEUP_H
#define GUCEF_CORE_TESTAPP_TESTTASKDELEGATORWAKEUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests how fast the delegator of a dedicated task answers a pulse request made on another thread while it waits
 *  between its cycles: an immediate pulse request and a request for periodic pulses each give a pulse well within the
 *  minimal cycle delta of the delegator, and requests leave no wake-ups behind that would make an idle delegator
 *  cycle faster than its minimal cycle delta. An immediate pulse asked for while the delegator pulses (on its thread
 *  or another one) is followed by a pulse; without a usable wake-up signal (CTaskManagerTestAccess) a delegator waits
 *  without spinning and still answers requests; once its thread ended its pulse generator names no driver thread.
 */
void
PerformTaskDelegatorWakeUpTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTTASKDELEGATORWAKEUP_H ? */
