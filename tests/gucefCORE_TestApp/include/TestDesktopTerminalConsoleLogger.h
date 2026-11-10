/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_CORE_TESTAPP_TESTDESKTOPTERMINALCONSOLELOGGER_H
#define GUCEF_CORE_TESTAPP_TESTDESKTOPTERMINALCONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the Linux console loggers: the terminal logger, the desktop terminal window logger with fake terminal
 *  emulators and the selection of a console logger for the environment. Does nothing on other platforms.
 */
void
PerformDesktopTerminalConsoleLoggerTests( void );

/*-------------------------------------------------------------------------*/

/** The argument that starts the test app as the child process of a test that the console log window must not crash the exit of */
#define GUCEF_CORE_TESTAPP_EXIT_WITH_CONSOLE_LOG_WINDOW_ARG    "--exit-with-console-log-window"

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_EXIT_WITH_CONSOLE_LOG_WINDOW_ARG: switches the console
 *  log window on, the way an application does from its settings, and returns the exit code of main.
 *  0 when it is on; the exit itself is what the test looks at
 */
int
RunExitWithConsoleLogWindow( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDESKTOPTERMINALCONSOLELOGGER_H ? */
