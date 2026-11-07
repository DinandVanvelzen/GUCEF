/*
 *  gucefMULTIPLAY_DesktopNotifyTestApp: Application for testing the desktop match alert sink plugin of gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFY_H
#define GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the cleaning, cutting and escaping of the text of a desktop notification and the filling in of the wording
 */
void
PerformDesktopNotifyTextTests( void );

/**
 *  Tests the argument vector of the desktop match alert sink, its limits and failure handling, the search for
 *  the program of the launcher and the registration of the plugin in the factory of the match alert sinks
 */
void
PerformDesktopNotifySinkTests( void );

/**
 *  Tests the desktop match alert sink behind a match alert router with a match book and a fake match provider
 */
void
PerformDesktopNotifyRouterTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_DESKTOPNOTIFYTESTAPP_TESTDESKTOPNOTIFY_H ? */
