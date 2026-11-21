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

#ifndef GUCEF_WEB_TESTAPP_TESTWEBLOGINSCRIPT_H
#define GUCEF_WEB_TESTAPP_TESTWEBLOGINSCRIPT_H

/**
 *  The login scripts of a web session sign in: parsing, validation, malformed and hostile scripts, the fixed page
 *  queries a sign in sends to the page, the patterns, hosts and cookie domains of a script, the arguments a sign in
 *  browser may get, and the default check of an ended session.
 */
void
PerformWebLoginScriptTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTWEBLOGINSCRIPT_H ? */
