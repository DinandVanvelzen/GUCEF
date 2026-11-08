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

#ifndef GUCEF_MT_TESTAPP_TESTTHREADIDENTITY_H
#define GUCEF_MT_TESTAPP_TESTTHREADIDENTITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the identity of the calling thread as MT::GetCurrentThreadID() gives it, and the lock ownership
 *  bookkeeping built on it: the id is stable on one thread and differs between threads, also for two threads
 *  whose native handles differ by exactly 4 GiB (a 32 bit truncation of the handle would make them equal).
 *  With those two threads the mutex, its re-entrancy count and the read/write lock must keep the threads apart.
 *  The 4 GiB case needs 64 bit Linux (glibc, not Android) and a free 4 GiB address range; it is skipped elsewhere.
 */
void
PerformThreadIdentityTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_TESTAPP_TESTTHREADIDENTITY_H ? */
