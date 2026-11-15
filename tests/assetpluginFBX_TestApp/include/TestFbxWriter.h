/*
 *  assetpluginFBX_TestApp: Test application for the FBX plugin of the asset converter (assetpluginFBX)
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

#ifndef GUCEF_ASSETPLUGIN_FBX_TESTAPP_TESTFBXWRITER_H
#define GUCEF_ASSETPLUGIN_FBX_TESTAPP_TESTFBXWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestFbxSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The axes and the unit of a test file: the FBX global settings. The file units are centimetres when 'unitScaleFactor'
 *  is 1 and metres when it is 100.
 */
struct SFbxAxes
{
    int upAxis;
    int upSign;
    int frontAxis;
    int frontSign;
    int coordAxis;
    int coordSign;
    double unitScaleFactor;
};

/** Y up, right handed, metres */
SFbxAxes FbxYUpMetres( void );

/** Z up, Y forward, right handed, centimetres */
SFbxAxes FbxZUpCentimetres( void );

/**
 *  An ASCII FBX 7.4 file of a cube of 'size' file units at the corner (0, 0, 0) on the node "Cube" at 'translation':
 *  eight corners, six quads, a normal per polygon vertex, UV coordinates through an index, a Phong material "Mat"
 *  with a diffuse colour, a diffuse texture and a normal map (file names given; empty for none).
 */
GUCEF::CORE::CString MakeCubeFbx( const SFbxAxes& axes, double size, const double translation[ 3 ], const GUCEF::CORE::CString& diffuseTexture, const GUCEF::CORE::CString& normalTexture );

/**
 *  A strip of two quads (six vertices, 0 to 2 units up) skinned to a chain of 'boneCount' bones with one take:
 *  the second bone turns about Z from 0 to 90 degrees in one second. The first vertex has an influence of every
 *  bone (weights 0.3, 0.25, 0.2, 0.15, 0.1 for five bones).
 */
GUCEF::CORE::CString MakeSkinnedFbx( const SFbxAxes& axes, GUCEF::UInt32 boneCount );

/** A cube whose diffuse texture is inside the file: the texture "name" with a video that holds the bytes of a picture, as base64 text */
GUCEF::CORE::CString MakeEmbeddedTextureFbx( const SFbxAxes& axes, const GUCEF::CORE::CString& fileName, const GUCEF::CORE::CString& base64 );

/** A cube with a PBR material the way 3ds Max writes it (metal/rough): base colour, roughness, metalness and textures */
GUCEF::CORE::CString MakePbrFbx( const SFbxAxes& axes, const GUCEF::CORE::CString& baseColorTexture, const GUCEF::CORE::CString& normalTexture );

/** A plane of four vertices with one blend shape that lifts them, the channel at half weight; with 'animated' one take moves the weight from 0 to 100 percent in one second */
GUCEF::CORE::CString MakeBlendShapeFbx( const SFbxAxes& axes, bool animated );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_FBX_TESTAPP_TESTFBXWRITER_H ? */
