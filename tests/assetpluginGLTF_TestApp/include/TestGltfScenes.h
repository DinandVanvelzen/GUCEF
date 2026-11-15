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

#ifndef GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSCENES_H
#define GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSCENES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestGltfSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A scene built in code with everything the intermediate holds: a hierarchy with transforms and extras, a skinned
 *  mesh with 6 influences per vertex and bind matrices, a mesh with two morph targets, vertex streams of every kind,
 *  materials with textures, texture transforms, emissive strength, an extension that goes through and alpha modes,
 *  animations of every path and interpolation, lights, a LOD group of three levels and colliders of every shape.
 *  The textures are named by VFS paths below "work/<name>/tex".
 */
void BuildFullScene( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& name );

/** One triangle with a material that has a texture */
void BuildTriangleScene( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& name );

/** Whether the second scene holds what the first holds, as far as a glTF file can carry it; 'problem' names the first difference */
bool CompareScenes( const GUCEF::ASSETCONVERTER::CAssetScene& a, const GUCEF::ASSETCONVERTER::CAssetScene& b, GUCEF::CORE::CString& problem );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_TESTAPP_TESTSCENES_H ? */
