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

#ifndef GUCEF_OGRETOOLS_TESTAPP_TESTTOOLCONVERSION_H
#define GUCEF_OGRETOOLS_TESTAPP_TESTTOOLCONVERSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes the test asset (glTF), converts it with OgreAssimpConverter to .mesh, .skeleton and .material, takes the
 *  mesh through OgreXMLConverter (mesh to XML and back) and through OgreMeshUpgrader (levels of detail by number of
 *  levels and by percentage, tangents), and loads every result with Ogre to check it. Needs the environment of
 *  StartOgreToolsEnvironment().
 */
void
PerformToolConversionTests( void );

/** The directory the converter made its files in: holds ogretest.mesh, ogretest.skeleton, ogretest.material, cubeonly.mesh */
GUCEF::CORE::CString
GetConvertedDir( void );

/** The directory of the glTF files the tests wrote */
GUCEF::CORE::CString
GetGltfDir( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRETOOLS_TESTAPP_TESTTOOLCONVERSION_H ? */
