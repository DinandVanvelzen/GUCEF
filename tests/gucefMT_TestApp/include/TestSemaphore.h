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

#ifndef GUCEF_MT_TESTAPP_TESTSEMAPHORE_H
#define GUCEF_MT_TESTAPP_TESTSEMAPHORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CSemaphore: counting, timeouts, waking across threads, producers and consumers, many posts then many waits
 */
void
PerformSemaphoreTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_TESTAPP_TESTSEMAPHORE_H ? */
