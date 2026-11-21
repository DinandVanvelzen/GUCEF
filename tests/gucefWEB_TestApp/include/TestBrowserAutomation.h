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

#ifndef GUCEF_WEB_TESTAPP_TESTBROWSERAUTOMATION_H
#define GUCEF_WEB_TESTAPP_TESTBROWSERAUTOMATION_H

/**
 *  Tests the browser automation primitives against the fake DevTools browser: the click listener set-up per page and
 *  frame session, navigation rules and the navigation guard, the download capture, the response tap and the page driver.
 */
void
PerformBrowserAutomationTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTBROWSERAUTOMATION_H ? */
