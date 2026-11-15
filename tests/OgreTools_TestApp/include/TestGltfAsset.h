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

#ifndef GUCEF_OGRETOOLS_TESTAPP_TESTGLTFASSET_H
#define GUCEF_OGRETOOLS_TESTAPP_TESTGLTFASSET_H

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
 *  The test asset, a glTF 2.0 file (.gltf with the buffer embedded as a base64 data URI):
 *    - a textured cube: 24 vertices (4 per face) with positions, normals and UVs, 12 triangles, one material
 *      ("CubeMaterial") with a base colour texture that refers to the file "checker.png";
 *    - when 'withStrip' is set, also a skinned strip: 6 vertices in 3 rows, 4 triangles, two bones ("Bone0" and
 *      its child "Bone1") and one animation clip ("Wave") of 2 seconds that turns Bone1. The cube is then bound
 *      to Bone0 with a weight of 1, since the loader of Ogre leaves out the meshes that have no bone weights as
 *      soon as the scene has a skeleton.
 */
GUCEF::CORE::CString
BuildTestGltf( bool withStrip );

/** Facts of the asset above, what the tests check the converted and loaded meshes against */
static const GUCEF::UInt32 TestGltfCubeVertexCount = 24;
static const GUCEF::UInt32 TestGltfCubeTriangleCount = 12;
static const GUCEF::UInt32 TestGltfStripVertexCount = 6;
static const GUCEF::UInt32 TestGltfStripTriangleCount = 4;
static const GUCEF::UInt32 TestGltfBoneCount = 2;
/** The skeleton Ogre makes of it has these bones and the bone "ROOT" that the loader adds above them */
static const GUCEF::UInt32 TestGltfSkeletonBoneCount = 3;
static const float TestGltfAnimationLength = 2.0f;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRETOOLS_TESTAPP_TESTGLTFASSET_H ? */
