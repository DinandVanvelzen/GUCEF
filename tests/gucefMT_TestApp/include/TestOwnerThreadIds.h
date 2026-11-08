/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#ifndef GUCEF_MT_TESTAPP_TESTOWNERTHREADIDS_H
#define GUCEF_MT_TESTAPP_TESTOWNERTHREADIDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the two ids that objects recording an owning thread keep: the identity (MT::GetCurrentThreadID()) and the
 *  operating system's id (MT::GetCurrentThreadSystemID()), which is a separate value only where
 *  GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID is 0. The diagnostic accessors of the C mutex, its re-entrant use, CMutex, the
 *  read/write lock (writer and readers), the thread data and CActiveObject have to report both ids of the holding
 *  thread as that thread obtains them itself, and clear them or keep the last holder after release as documented.
 *  The sizes of the structures prove that no second id exists where the define is 1.
 */
void
PerformOwnerThreadIdsTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_TESTAPP_TESTOWNERTHREADIDS_H ? */
