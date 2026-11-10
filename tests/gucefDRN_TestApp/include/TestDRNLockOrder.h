/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNLOCKORDER_H
#define GUCEF_DRN_TESTAPP_TESTDRNLOCKORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the lock order of the node (gucefDRN design 9): nodes on a pulse generator of the application that a thread
 *  of its own pulses, called from other threads while their links carry traffic, without and with a thread of their
 *  own. Every thread has to keep going: one that makes no progress for seconds is a deadlock, which the test reports
 *  instead of hanging (a lock wait ends with an exception after the lock timeout of gucefMT).
 */
void
PerformDRNLockOrderTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNLOCKORDER_H ? */
