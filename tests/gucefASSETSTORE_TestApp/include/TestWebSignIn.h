/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTWEBSIGNIN_H
#define TESTWEBSIGNIN_H

/**
 *  Web session sign in (section 12.1 of the design): login scripts (parsing, validation, malformed and hostile
 *  scripts), the fixed page queries, sessions and credentials in the credential store, and CWebSessionSignIn against a
 *  fake login site through a test double of the browser: automatic sign in, session reuse, re-sign in after expiry,
 *  hand over for a captcha and an e-mail code, manual sign in, a refused password, cancel, time outs, a closed window,
 *  the provider base class, and that nothing secret reaches a log or a file.
 *
 *  With GUCEF_TEST_BROWSER=1 and a Chrome, Chromium, Edge or Brave on this computer the same site is also signed in to
 *  with the real browser, headless (--headless=new) and kept off every network but the loopback; skipped otherwise.
 */
void PerformWebSignInTests( void );

/*-------------------------------------------------------------------------*/

#endif /* TESTWEBSIGNIN_H ? */
