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


#ifndef GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDWIRE_H
#define GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDWIRE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the connection against the real libwayland-client and libwayland-server over a socket in a private directory, no
 *  compositor and no session involved: a small server of the test (libwayland-server in a thread of its own) serves the globals
 *  and the events of a seat. This is what proves that the function tables, the generated protocol tables and the pump work
 *  with the real libraries: marshalling, file descriptors in events, fixed point numbers, prepare / read / dispatch on a real
 *  socket. Skipped when libwayland-server.so.0 is not installed.
 */
void
PerformWaylandWireTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_WAYLANDTESTAPP_TESTWAYLANDWIRE_H ? */
