/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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


#ifndef GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDLIVE_H
#define GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDLIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The tests against a real compositor, run only when the test application is started with --live (tools/test-wayland.sh does
 *  that on a compositor of its own, private socket and runtime directory): a window is made, configured, drawn into with
 *  EGL, resized and closed. Without --live nothing here runs and nothing connects anywhere.
 */
void
PerformWaylandLiveTests( bool live );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDLIVE_H ? */
