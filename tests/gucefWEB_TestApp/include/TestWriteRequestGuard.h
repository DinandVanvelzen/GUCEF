/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_WEB_TESTAPP_TESTWRITEREQUESTGUARD_H
#define GUCEF_WEB_TESTAPP_TESTWRITEREQUESTGUARD_H

/**
 *  Tests the write request guard: rule parsing and strict path reading, the decision for methods, schemes, hosts,
 *  paths, path normalisation tricks and queries, the settings round trip, and the guard enforced by a navigation guard
 *  against the fake DevTools browser: every request intercepted, refused writes failed and reported, allowed writes and
 *  reads continued, arming and disarming on a started guard, the probe of VerifyEnforcing() and the end of enforcement
 *  when the connection ends.
 */
void
PerformWriteRequestGuardTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTWRITEREQUESTGUARD_H ? */
