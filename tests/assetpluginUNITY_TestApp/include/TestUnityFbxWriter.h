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

#ifndef GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYFBXWRITER_H
#define GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYFBXWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestUnitySupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The axes and the unit of a test file: the FBX global settings (UnitScaleFactor 1 is centimetres, 100 metres) */
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

/** Y up, right handed, centimetres */
SFbxAxes FbxYUpCentimetres( void );

/** Z up, Y forward, right handed, centimetres */
SFbxAxes FbxZUpCentimetres( void );

/** A node of a test file: its name, the index of its parent (-1 for none), its translation and the size of its cube (0 for no mesh) */
struct STestFbxNode
{
    const char* name;
    int parent;
    double translation[ 3 ];
    double size;
};

/**
 *  An ASCII FBX 7.4 file of the nodes; every node with a size holds a cube of that size at the corner (0, 0, 0) of its
 *  own geometry (normals, UVs) with the material "Mat" (its diffuse texture "albedo.psd", which is not in the test
 *  data, as model files of packs name textures of the author's disk)
 */
GUCEF::CORE::CString MakeNodesFbx( const SFbxAxes& axes, const STestFbxNode* nodes, GUCEF::UInt32 count );

/** A file with the node "Mover" (a cube of 10) and the take "Take 001" of two seconds that moves it 100 units along X */
GUCEF::CORE::CString MakeMoverFbx( const SFbxAxes& axes );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYFBXWRITER_H ? */
