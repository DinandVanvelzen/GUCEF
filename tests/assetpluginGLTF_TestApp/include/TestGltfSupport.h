/*
 *  assetpluginGLTF_TestApp: Test application for the glTF plugin of the asset converter (assetpluginGLTF)
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSUPPORT_H
#define GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSUPPORT_H

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

/** Loads the plugins (JSON, zlib, FreeImage and the glTF plugin), mounts the work folder as "work" */
bool SetUpGltfTests( void );

void TearDownGltfTests( void );

/** A new, empty folder of the test case on the disk, below the work folder; its disk path. Its VFS path is "work/<name>" */
GUCEF::CORE::CString MakeWorkFolder( const GUCEF::CORE::CString& name );

/** The disk path of a path below the work folder */
GUCEF::CORE::CString WorkDiskPath( const GUCEF::CORE::CString& relativePath );

/** Writes a file below the work folder (folders made as needed) */
bool WriteWorkFile( const GUCEF::CORE::CString& relativePath, const void* bytes, GUCEF::UInt32 size );

bool WriteWorkText( const GUCEF::CORE::CString& relativePath, const GUCEF::CORE::CString& text );

/** Reads a file of the VFS */
bool ReadVfsBytes( const GUCEF::CORE::CString& vfsPath, GUCEF::CORE::CDynamicBuffer& bytes );

GUCEF::CORE::CString ReadVfsText( const GUCEF::CORE::CString& vfsPath );

/** Whether two files of the VFS exist and have the same bytes */
bool SameBytes( const GUCEF::CORE::CString& vfsPathA, const GUCEF::CORE::CString& vfsPathB );

/** Writes an RGBA PNG through the VFS, every texel the colour */
bool WriteTestPng( const GUCEF::CORE::CString& vfsPath, GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** Writes the licence record <folder>/<id>.licence.json below the work folder */
bool WriteLicence( const GUCEF::CORE::CString& relativeFolder, const GUCEF::CORE::CString& id );

/** Opens a pack context on a folder of the VFS, with an intermediate folder (where the importers extract what is inside a file) */
bool OpenPackContext( GUCEF::ASSETCONVERTER::CAssetPackContext& context, const GUCEF::CORE::CString& sourceFolder, const GUCEF::CORE::CString& intermediateFolder );

/** cgltf itself, on a file on the disk: parse, load the buffers, validate. 'problem' says what failed */
bool ValidateWithCgltf( const GUCEF::CORE::CString& diskPath, GUCEF::CORE::CString& problem );

bool Near( GUCEF::Float32 a, GUCEF::Float32 b, GUCEF::Float32 tolerance = 1e-5f );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSUPPORT_H ? */
