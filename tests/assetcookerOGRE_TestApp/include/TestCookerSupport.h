/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker of the asset converter (assetcookerOGRE)
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

#ifndef GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSUPPORT_H
#define GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "Ogre.h"

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_MATH_H
#include "gucefMATH.h"
#define GUCEF_MATH_H
#endif /* GUCEF_MATH_H ? */

#ifndef ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define ASSETCONVERTERLIB_H
#endif /* ASSETCONVERTERLIB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Starts Ogre without a render system (the Root through pluginglueOGRE, the software buffer manager, the default
 *  texture manager, the material manager, the shader generator so RTSS sections of scripts parse), loads the Ogre
 *  plugins OgrePlugin_GucefVFS and OgrePlugin_DotScene and the GUCEF plugins (JSON, zlib, FreeImage, the glTF plugin and
 *  the cooker) and mounts the work folder as "work"
 */
bool SetUpCookerTests( void );
void TearDownCookerTests( void );

/** A new, empty folder of the test case below the work folder; its disk path. Its VFS path is "work/<name>" */
GUCEF::CORE::CString MakeWorkFolder( const GUCEF::CORE::CString& name );

/** The disk path of a path below the work folder */
GUCEF::CORE::CString WorkDiskPath( const GUCEF::CORE::CString& relativePath );

bool WriteWorkText( const GUCEF::CORE::CString& relativePath, const GUCEF::CORE::CString& text );

GUCEF::CORE::CString ReadVfsText( const GUCEF::CORE::CString& vfsPath );

bool VfsFileExists( const GUCEF::CORE::CString& vfsPath );

/** Writes an RGBA PNG through the VFS, every texel the colour */
bool WriteTestPng( const GUCEF::CORE::CString& vfsPath, GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** The size and the first texel of a picture of the VFS */
bool ReadTestPicture( const GUCEF::CORE::CString& vfsPath, GUCEF::UInt32& width, GUCEF::UInt32& height, GUCEF::UInt8 firstTexel[ 4 ] );

/** Writes the licence record <folder>/<id>.licence.json below the work folder */
bool WriteLicence( const GUCEF::CORE::CString& relativeFolder, const GUCEF::CORE::CString& id );

/** The cooker "ogre" of the registry */
GUCEF::ASSETCONVERTER::TAssetCookerPtr GetOgreCooker( void );

/**
 *  Exports the scene with the glTF exporter into "work/<caseFolder>/inter" with a sidecar (CConversionSidecar), as the
 *  pipeline makes the intermediate of the asset 'assetPath', and fills 'asset' for CCookSettings
 */
bool MakeIntermediate( const GUCEF::ASSETCONVERTER::CAssetScene& scene         ,
                       const GUCEF::CORE::CString& caseFolder                  ,
                       const GUCEF::CORE::CString& assetPath                   ,
                       GUCEF::ASSETCONVERTER::CCookSettings::SAsset& asset     );

/**
 *  Cooks the intermediate of "work/<caseFolder>/inter" into "work/<caseFolder>/out/<profile>" for pack "pack" with the
 *  sources below "work/<caseFolder>/src"; the report of the asset is in 'report'
 */
bool CookIntermediate( const GUCEF::CORE::CString& caseFolder                  ,
                       GUCEF::ASSETCONVERTER::CCookSettings& settings          ,
                       GUCEF::ASSETCONVERTER::CConversionReport& report        );

/** "work/<caseFolder>/out/<profile>" */
GUCEF::CORE::CString GetProfileDir( const GUCEF::CORE::CString& caseFolder, const GUCEF::CORE::CString& profile = "desktop" );

/**
 *  Makes a resource group (again) with a VFS folder as its recursive location, through the archives of
 *  OgrePlugin_GucefVFS. With 'parseScripts' the group is initialised, so its material scripts are parsed; entities
 *  cannot use those materials without a render system (compiling them needs its capabilities)
 */
bool UseOgreGroup( const Ogre::String& group, const GUCEF::CORE::CString& vfsFolder, bool parseScripts = true );

/** Takes the resource group down with what was loaded into it */
void DropOgreGroup( const Ogre::String& group );

/** Whether the report has an item of the kind whose message holds the text */
bool ReportHas( const GUCEF::ASSETCONVERTER::CConversionReport& report, GUCEF::ASSETCONVERTER::TReportItemKind kind, const GUCEF::CORE::CString& text );

/** Whether the report lists a cooked file (a VFS path) for any asset */
bool ReportHasFile( const GUCEF::ASSETCONVERTER::CConversionReport& report, const GUCEF::CORE::CString& path );

bool Near( float a, float b, float tolerance = 1e-4f );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSUPPORT_H ? */
