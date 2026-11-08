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

#ifndef GUCEF_MT_TESTAPP_TESTREALTIMESCHEDULINGSCOPE_H
#define GUCEF_MT_TESTAPP_TESTREALTIMESCHEDULINGSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CRealtimeSchedulingScope: the scheduling of the thread is restored, a refusal (no right) changes nothing and
 *  is reported, nested scopes and priority limits, a scope affects only its own thread. The tests pass whether or not
 *  the user may use real-time scheduling and log which case they saw
 */
void
PerformRealtimeSchedulingScopeTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_TESTAPP_TESTREALTIMESCHEDULINGSCOPE_H ? */
