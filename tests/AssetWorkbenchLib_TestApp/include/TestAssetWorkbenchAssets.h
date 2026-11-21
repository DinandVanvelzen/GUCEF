/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#ifndef ASSETWORKBENCHLIB_TESTAPP_TESTASSETS_H
#define ASSETWORKBENCHLIB_TESTAPP_TESTASSETS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The test model, a glTF 2.0 file (.gltf with the buffer embedded as a base64 data URI):
 *    - a textured cube: 24 vertices (4 per face) with positions, normals and UVs, 12 triangles, one material ("CubeMaterial") with a
 *      base colour texture that refers to the file "checker.png";
 *    - a skinned strip: 6 vertices in 3 rows, 4 triangles, two bones ("Bone0" and its child "Bone1") and one animation clip ("Wave")
 *      of 2 seconds that turns Bone1. The cube is bound to Bone0 with a weight of 1.
 */
GUCEF::CORE::CString BuildTestGltf( void );

static const GUCEF::UInt32 TestGltfVertexCount = 24 + 6;
static const GUCEF::UInt32 TestGltfTriangleCount = 12 + 4;
static const GUCEF::UInt32 TestGltfSubMeshCount = 2;
/** The skeleton Ogre makes of it has the two bones and the bone "ROOT" that the loader adds above them */
static const GUCEF::UInt32 TestGltfSkeletonBoneCount = 3;
static const GUCEF::UInt32 TestGltfMaterialCount = 2;
static const GUCEF::UInt32 TestGltfAnimationLengthInMs = 2000;

/** A PNG file of width x height pixels: the left half red, the right half blue, with an alpha of 255; false when it cannot be written */
bool WriteTestPng( const GUCEF::CORE::CString& path, GUCEF::UInt32 width, GUCEF::UInt32 height );

/** A DDS file of width x height pixels in the format DXT1 (BC1): every block is the colour given, as the two end points of the block */
bool WriteTestDds( const GUCEF::CORE::CString& path, GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt16 colour565 );

/** A DotScene file with the nodes Parent (with Child and Sibling below it) and Lamp, an entity of the glTF mesh on Child and a light on Lamp */
GUCEF::CORE::CString BuildTestScene( const GUCEF::CORE::CString& meshFile );

/** A material script with the materials "TestRed" and "TestTextured" (the second names the picture "checker.png") */
GUCEF::CORE::CString BuildTestMaterialScript( void );

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_TESTAPP_TESTASSETS_H ? */
