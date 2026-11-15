/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#ifndef GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTSUPPORT_H
#define GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The directory the tests make their files in, below the working directory of the test application */
GUCEF::CORE::CString
GetTestWorkDir( void );

/** Makes (or empties) a directory below the work directory and returns its path */
GUCEF::CORE::CString
MakeCleanTestDir( const GUCEF::CORE::CString& name );

/** Writes a file with the given content, making the directories above it. The path is a path of the file system */
bool
WriteTestFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );

/** The content of a file of the file system, empty when there is none */
GUCEF::CORE::CString
ReadTestFile( const GUCEF::CORE::CString& path );

/** Removes the work directory with everything in it */
void
RemoveTestWorkDir( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_TESTAPP_TESTSUPPORT_H ? */
