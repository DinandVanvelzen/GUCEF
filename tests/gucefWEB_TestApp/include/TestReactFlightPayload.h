/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_TESTAPP_TESTREACTFLIGHTPAYLOAD_H
#define GUCEF_WEB_TESTAPP_TESTREACTFLIGHTPAYLOAD_H

/**
 *  Tests the reader of React Server Components payloads (CReactFlightPayload): the string literals of the script
 *  elements of a page, the payload joined from them, the rows (models, module and hint rows, length prefixed text
 *  rows with multi byte characters and line ends), the references between rows, the objects of the models and the
 *  links of the element trees with their text. Synthetic pages only, no network.
 */
void
PerformReactFlightPayloadTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTREACTFLIGHTPAYLOAD_H ? */
