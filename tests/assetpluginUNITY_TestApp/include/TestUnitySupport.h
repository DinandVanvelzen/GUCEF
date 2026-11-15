/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSUPPORT_H
#define GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSUPPORT_H

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

/** Loads the plugins (JSON, zlib, FreeImage, libarchive, Unity packages, glTF, FBX and Unity), mounts the work folder as "work" */
bool SetUpUnityTests( void );

void TearDownUnityTests( void );

/** A new, empty folder of the test case on the disk, below the work folder; its disk path. Its VFS path is "work/<name>" */
GUCEF::CORE::CString MakeWorkFolder( const GUCEF::CORE::CString& name );

/** The disk path of a path below the work folder */
GUCEF::CORE::CString WorkDiskPath( const GUCEF::CORE::CString& relativePath );

/** Writes a file below the work folder (folders made as needed) */
bool WriteWorkFile( const GUCEF::CORE::CString& relativePath, const void* bytes, GUCEF::UInt32 size );
bool WriteWorkText( const GUCEF::CORE::CString& relativePath, const GUCEF::CORE::CString& text );

/** Reads a whole file of the disk */
bool ReadDiskFile( const GUCEF::CORE::CString& diskPath, GUCEF::CORE::CDynamicBuffer& bytes );

/** The bytes of a PNG of one colour, made with the PNG writer of the converter */
bool MakeTestPng( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a, GUCEF::CORE::CDynamicBuffer& bytes );

/** A licence record below the work folder */
bool WriteLicence( const GUCEF::CORE::CString& relativeFolder, const GUCEF::CORE::CString& id );

/** Validates a glTF file of the disk with cgltf */
bool ValidateWithCgltf( const GUCEF::CORE::CString& diskPath, GUCEF::CORE::CString& problem );

bool Near( GUCEF::Float32 a, GUCEF::Float32 b, GUCEF::Float32 tolerance = 1e-4f );

/** The index of the first node with the name, -1 when there is none */
GUCEF::Int32 FindNode( const GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& name );

/** The bounds of a mesh of the scene (all submeshes), in its own frame */
void GetMeshBounds( const GUCEF::ASSETCONVERTER::CAssetMesh& mesh, GUCEF::Float32 min[ 3 ], GUCEF::Float32 max[ 3 ] );

/** The bounds of the meshes of the scene in the frame of the scene (node transforms applied) */
bool GetSceneBounds( const GUCEF::ASSETCONVERTER::CAssetScene& scene, GUCEF::Float32 min[ 3 ], GUCEF::Float32 max[ 3 ] );

/** The number of report items of the kind whose message holds the text */
GUCEF::UInt32 CountItems( const GUCEF::ASSETCONVERTER::CConversionReport& report, GUCEF::ASSETCONVERTER::TReportItemKind kind, const GUCEF::CORE::CString& text );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSUPPORT_H ? */
