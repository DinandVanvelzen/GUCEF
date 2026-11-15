/*
 *  guidriverRmlUIOgre_TestApp: Test application for the GUI driver RmlUiOgre (guidriverRmlUIOgre)
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

#ifndef GUIDRIVERRMLUIOGRE_TESTAPP_TESTRMLUIOGRECONTEXT_H
#define GUIDRIVERRMLUIOGRE_TESTAPP_TESTRMLUIOGRECONTEXT_H

/** Loads the plugins and makes the main window; false when there is nothing to test */
bool PerformRmlUiOgreSetUpTests( void );

/** The driver and its contexts: against the software reference, layers over a 3D viewport, textures, release */
void PerformRmlUiOgreContextTests( void );

/** The contexts and the window go, then the plugins and the Root */
void PerformRmlUiOgreTearDownTests( void );

#endif /* GUIDRIVERRMLUIOGRE_TESTAPP_TESTRMLUIOGRECONTEXT_H ? */
