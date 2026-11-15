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

#ifndef GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSCENES_H
#define GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSCENES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A character: joints Hip, Spine, Head, ArmL, ArmR, Tail (a skin with inverse bind matrices at the rest pose); the
 *  skinned mesh BodyMesh with two submeshes of material Skin (one vertex with six influences) and one of material
 *  Armour (a normal map, no tangents); PropMesh with the morph targets Smile and Blink; the clips Walk (Hip, Spine,
 *  Head), Wave (ArmL), Blink (the weights of Prop) and Spin (the rotation of the node Prop). Textures in 'textureFolder'
 */
void BuildCharacterScene( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& textureFolder );

/** A board of ten quads, one per material of the mapping table (see the test of the materials) */
void BuildMaterialScene( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& textureFolder );

/** The LOD group Crate (Crate_LOD0 a 10x10 grid, Crate_LOD1 a 3x3 grid, Crate_LOD2 a quad) and a 12x12 grid Grid */
void BuildLodScene( GUCEF::ASSETCONVERTER::CAssetScene& scene );

/**
 *  A house: House (mesh HouseMesh, a box collider) with the children Door (DoorMesh, a convex trigger), Lamp (a point
 *  light), Sign (HouseMesh again, with extras) and Fan (DoorMesh, the clip Spin); the prefab Doorway at Door with a
 *  material override to Wood. Wood's texture is a source of the pack, Paint's one of the intermediate.
 */
void BuildHouseScene( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& sourceTexture, const GUCEF::CORE::CString& intermediateTexture );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_TESTAPP_TESTCOOKERSCENES_H ? */
