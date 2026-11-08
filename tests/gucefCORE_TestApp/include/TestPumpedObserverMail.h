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

#ifndef GUCEF_CORE_TESTAPP_TESTPUMPEDOBSERVERMAIL_H
#define GUCEF_CORE_TESTAPP_TESTPUMPEDOBSERVERMAIL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the mailbox of a pumped observer, whose lock is a leaf that is not held while mail is delivered: discarding
 *  the mail of one notifier, a thread that destroys a notifier waits for the delivery of its mail that is under way,
 *  clearing the mailbox leaves the mail being delivered in place, mail is delivered in the order it arrived while
 *  other threads add mail, and mail that arrives during a pulse is delivered at the next one.
 */
void
PerformPumpedObserverMailTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTPUMPEDOBSERVERMAIL_H ? */
