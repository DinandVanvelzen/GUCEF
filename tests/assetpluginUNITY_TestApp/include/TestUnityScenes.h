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

#ifndef GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSCENES_H
#define GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSCENES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Scenes of many nested prefab instances: a synthetic package with a scene of thousands of instances converts with
 *  every file read once and the meshes shared, nothing the intermediate cannot express is dropped silently, and, when
 *  the environment variable GUCEF_TEST_ASSET_CORPUS is 1, the scenes of the package that GUCEF_TEST_UNITY_SCENES_PACKAGE
 *  names convert to the counts of the golden files in the folder GUCEF_TEST_UNITY_SCENES_GOLDEN (made when absent)
 */
void PerformUnitySceneTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYSCENES_H ? */
