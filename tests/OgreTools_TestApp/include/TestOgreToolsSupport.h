/*
 *  OgreTools_TestApp: Test application for the Ogre asset tools and plugins (OgreAssimpConverter, OgreXMLConverter,
 *  OgreMeshUpgrader, OgreCodec_Assimp, OgrePlugin_DotScene)
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

#ifndef GUCEF_OGRETOOLS_TESTAPP_TESTSUPPORT_H
#define GUCEF_OGRETOOLS_TESTAPP_TESTSUPPORT_H

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

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

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

/** Size of a file of the file system in bytes, 0 when there is none */
GUCEF::UInt64
GetTestFileSize( const GUCEF::CORE::CString& path );

/** Removes the work directory with everything in it */
void
RemoveTestWorkDir( void );

/** Tells the support where the test application was started from (argv[0]), to find the tools built next to it */
void
SetTestAppPath( const char* argv0 );

/** The path of a tool (an executable module like OgreXMLConverter) built next to the test application, empty when not found */
GUCEF::CORE::CString
FindToolExecutable( const GUCEF::CORE::CString& toolName );

/**
 *  Runs a tool to its end with its output collected (standard output and error together), waiting at most
 *  'timeoutInMs'. Returns true when the tool was started and ended in time; 'exitCode' is its exit code.
 */
bool
RunTool( const GUCEF::CORE::CString& toolName                       ,
         const GUCEF::CORE::CProcess::TStringVector& args           ,
         const GUCEF::CORE::CString& workingDir                     ,
         GUCEF::Int32& exitCode                                     ,
         GUCEF::CORE::CString& output                               ,
         GUCEF::UInt32 timeoutInMs = 120000                         );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRETOOLS_TESTAPP_TESTSUPPORT_H ? */
