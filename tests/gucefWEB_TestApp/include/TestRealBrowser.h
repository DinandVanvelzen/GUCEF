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

#ifndef GUCEF_WEB_TESTAPP_TESTREALBROWSER_H
#define GUCEF_WEB_TESTAPP_TESTREALBROWSER_H

/**
 *  Drives a real headless Chrome or Chromium, with a profile of its own in a temporary folder, against loopback test
 *  sites: click labels from a page and a cross-origin frame, the navigation guard, a download, a response tap and the
 *  page driver. Skipped with a notice when no browser is found. GUCEF_TEST_BROWSER=<path> picks the browser,
 *  GUCEF_TEST_CHROME_NO_SANDBOX=1 adds --no-sandbox.
 */
void
PerformRealBrowserTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTREALBROWSER_H ? */
