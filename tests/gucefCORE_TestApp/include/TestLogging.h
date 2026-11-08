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

#ifndef GUCEF_CORE_TESTAPP_TESTLOGGING_H
#define GUCEF_CORE_TESTAPP_TESTLOGGING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the logging backend including the streaming rework.
 *  Covers CStdLogger, CLogManager, CLogStreamScope, CMultiLogger dispatch,
 *  per-thread buffering, FlushThreadStreamBuffers, and the GUCEF_LOG macros.
 *  Sets a user break when a test fails.
 */
void
PerformLoggingTests( void );

/*-------------------------------------------------------------------------*/

/** The argument which starts the test app as the process of the log buffer swap race, see RunLogBufferSwapRace() */
#define GUCEF_CORE_TESTAPP_LOG_BUFFER_SWAP_RACE_ARG    "--log-buffer-swap-race"

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_LOG_BUFFER_SWAP_RACE_ARG: one thread logs while the
 *  main thread flushes the log buffers for two seconds. Returns the exit code, 0 when both got to the end
 */
int
RunLogBufferSwapRace( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTLOGGING_H ? */
