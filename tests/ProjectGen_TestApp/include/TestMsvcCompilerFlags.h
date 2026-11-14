/*
 *  ProjectGen_TestApp: GUCEF test application for the ProjectGen module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 */

#ifndef GUCEF_PROJECTGEN_TESTAPP_TESTMSVCCOMPILERFLAGS_H
#define GUCEF_PROJECTGEN_TESTAPP_TESTMSVCCOMPILERFLAGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests that compiler flags for the MSVC compiler are parsed from the module info and
 *  that the CMake generator emits them as MSVC only target options next to the GNU/Clang ones
 */
void
PerformMsvcCompilerFlagsTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_TESTAPP_TESTMSVCCOMPILERFLAGS_H ? */
