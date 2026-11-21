/*
 *  assetpluginUNREAL_TestApp: Support of the tests of the Unreal package reader: plugins, work folder, golden files
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALSUPPORT_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "assetpluginUNREAL_UnrealVersions.h"

#ifndef GUCEF_CORE_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_GUCEFCORE_H
#endif /* GUCEF_CORE_GUCEFCORE_H ? */

#ifndef GUCEF_CORE_GUCEFVFS_H
#include "gucefVFS.h"
#define GUCEF_CORE_GUCEFVFS_H
#endif /* GUCEF_CORE_GUCEFVFS_H ? */

#ifndef GUCEF_CORE_ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define GUCEF_CORE_ASSETCONVERTERLIB_H
#endif /* GUCEF_CORE_ASSETCONVERTERLIB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads the plugins (zip archives and the Unreal plugin) and mounts the work folder as "work" */
bool SetUpUnrealTests( void );

void TearDownUnrealTests( void );

/** The disk path of a path below the work folder (a folder in the scratch area outside the repositories) */
GUCEF::CORE::CString WorkDiskPath( const GUCEF::CORE::CString& relativePath );

/** The VFS path of a path below the work folder */
GUCEF::CORE::CString WorkVfsPath( const GUCEF::CORE::CString& relativePath );

/** Writes a file below the work folder (folders made as needed) */
bool WriteWorkFile( const GUCEF::CORE::CString& relativePath, const GUCEF::vector< GUCEF::UInt8 >& bytes );

void DeleteWorkFile( const GUCEF::CORE::CString& relativePath );

/** Whether the tests that read the licensed packages of the local corpus are enabled (GUCEF_TEST_ASSET_CORPUS=1) */
bool CorpusEnabled( void );

/** The locations of the local corpus, from the manifest in the scratch area of the user: lines of key=value */
GUCEF::CORE::CString CorpusSetting( const GUCEF::CORE::CString& key );

/** The home folder of the user running the tests */
GUCEF::CORE::CString HomeDir( void );

/** The folder with the golden summaries, outside every repository */
GUCEF::CORE::CString GoldenDir( void );

/**
 *  Compares text with the golden file of the name. A golden file that does not exist yet is created from the text (the first
 *  run); a golden file that differs gives false and the first difference in problem.
 */
bool CheckGolden( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& text, GUCEF::CORE::CString& problem );

/*-------------------------------------------------------------------------*/

/**
 *  For the tests whose golden numbers are those of a build without the oodle tool (the textures and meshes that need it are reported):
 *  while the object lives the tool is looked for at a path that does not exist. The suites of the tool use the tool.
 */
class CNoOodleToolScope
{
    public:
    CNoOodleToolScope( void );
    ~CNoOodleToolScope();
    private:
    bool m_had;
    GUCEF::CORE::CString m_old;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALSUPPORT_H ? */
