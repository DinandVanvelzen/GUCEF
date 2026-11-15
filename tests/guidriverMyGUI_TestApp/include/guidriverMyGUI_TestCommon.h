/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#ifndef GUCEF_guidriverMyGUI_TESTCOMMON_H
#define GUCEF_guidriverMyGUI_TESTCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the test app, the debug or release build of it */
bool LoadTestPlugin( const char* moduleName, const char* pluginGroup );

/** Loads the plugin with the image codecs (FreeImage); false when it is not there */
bool LoadImageCodecs( void );

/** A context of the MyGUI OpenGL driver in an offscreen EGL window of the given size, with a logical resolution of 640 x 480; NULL when either is missing */
GUCEF::GUI::TGuiContextPtr CreateMyGUIContextInWindow( GUCEF::UInt32 windowWidth, GUCEF::UInt32 windowHeight );

/** The same in a window of 640 x 480 */
GUCEF::GUI::TGuiContextPtr CreateMyGUIContext( void );

void PerformMyGUIDataManagerTests( void );
void PerformMyGUIImageCodecTests( void );
void PerformMyGUIRenderTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_TESTCOMMON_H ? */
