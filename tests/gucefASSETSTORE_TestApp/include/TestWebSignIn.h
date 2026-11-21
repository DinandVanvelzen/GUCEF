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
 *  The web session sign in of a store (section 12.1 of the design) on the provider base class
 *  (CWebSessionAssetStoreProvider) against a fake login site through a test double of the browser: the sign in kind,
 *  the session cookies on requests, renewal after the session ended and sign out, and that nothing secret reaches a log
 *  or a file. The login scripts are tested in gucefWEB_TestApp, the sign in itself (AUTH::CWebSessionSignIn) in
 *  gucefAUTH_TestApp.
 */
void PerformWebSignInTests( void );

/*-------------------------------------------------------------------------*/

#endif /* TESTWEBSIGNIN_H ? */
