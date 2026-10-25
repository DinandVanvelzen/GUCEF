/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_GUI_TESTCOMMON_H
#define GUCEF_GUI_TESTCOMMON_H

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

/** True in a web browser, with the canvas "#canvas" of the page; false in every other place a test can run */
bool IsInBrowser( void );

/** A context of the null driver at 640 x 480, NULL when the driver cannot be loaded */
GUCEF::GUI::TGuiContextPtr CreateNullContext( void );

/** A context of the RmlUi driver drawing with OpenGL in an offscreen EGL window of 640 x 480, NULL when either is missing */
GUCEF::GUI::TGuiContextPtr CreateRmlUiContext( void );

/** An RmlUi context with a logical resolution of 640 x 480 in a window of the given size */
GUCEF::GUI::TGuiContextPtr CreateRmlUiContextInWindow( GUCEF::UInt32 windowWidth, GUCEF::UInt32 windowHeight );

/** A pixel map of the given size filled with one colour */
GUCEF::IMAGE::TPixelMapPtr MakeSolidImage( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** A bitmap font with fixed width glyphs for ASCII 32..126: each glyph is 'width' x 'height' */
GUCEF::GUI::TBitmapFontPtr MakeTestFont( GUCEF::Int32 width, GUCEF::Int32 height );

/** Loads the FreeType font plugin; true when its font rasterizer is registered */
bool LoadFreeTypeFontPlugin( void );

/** Loads the XML and JSON DStore codecs; false when they are not there */
bool LoadLayoutCodecs( void );

void PerformGuiCoreTests( void );
void PerformGuiWidgetTests( void );
void PerformGuiLayoutTests( void );
void PerformGuiMediaTests( void );
void PerformGuiRenderTests( void );
void PerformGuiInputTests( void );
void PerformGuiWindowTests( void );
void PerformGuiScrollPanelTests( void );
void PerformGuiPropertyTests( void );

void PerformGuiBrowserTests( void );
void PerformGuiFontTests( void );
void PerformGuiSettingsTests( void );

void PerformGuiFileDialogTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TESTCOMMON_H ? */
