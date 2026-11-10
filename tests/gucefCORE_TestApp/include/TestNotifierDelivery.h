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

#ifndef GUCEF_CORE_TESTAPP_TESTNOTIFIERDELIVERY_H
#define GUCEF_CORE_TESTAPP_TESTNOTIFIERDELIVERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the delivery of an event to many observers while the observers subscribe others and unsubscribe others
 *  during it: every observer that is subscribed when its turn comes gets the event once, one that was unsubscribed
 *  before its turn does not get it, observers subscribed during the delivery get it once and stay subscribed, and
 *  the time the delivery takes grows linearly with the number of observers. Covers subscriptions to one event, to
 *  all events and the broadcast of the modify event.
 */
void
PerformNotifierDeliveryTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTNOTIFIERDELIVERY_H ? */
