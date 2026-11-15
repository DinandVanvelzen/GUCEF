/*
 *  gucefCORE_TestApp: GUCEF test application
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

#ifndef GUCEF_CORE_TESTAPP_TESTSERVICEPROCESSKIT_H
#define GUCEF_CORE_TESTAPP_TESTSERVICEPROCESSKIT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The service process kit of gucefCORE: CMonthlyJsonLinesFile (whole lines, months by a fake clock, torn-line repair,
 *  retention by names, refused appends), CServiceLifecycle (the transition table, conditions, drain, failure, events),
 *  CServiceStopSignals (a noted signal raises one event at the next pulse; installed handlers take a real SIGTERM
 *  without stopping the application) and CServiceResourceWatch (process values, source counters, the soft limit)
 */
void
PerformServiceProcessKitTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTSERVICEPROCESSKIT_H ? */
