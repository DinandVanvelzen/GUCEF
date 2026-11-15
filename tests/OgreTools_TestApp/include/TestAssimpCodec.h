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

#ifndef GUCEF_OGRETOOLS_TESTAPP_TESTASSIMPCODEC_H
#define GUCEF_OGRETOOLS_TESTAPP_TESTASSIMPCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Loads the glTF of the test asset directly through the codec of OgreCodec_Assimp: MeshManager::load of the .gltf,
 *  from a directory that is mounted in the VFS and reached through the archive type GucefVFS of OgrePlugin_GucefVFS.
 *  Needs the environment of StartOgreToolsEnvironment() and the glTF files of PerformToolConversionTests().
 */
void
PerformAssimpCodecTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRETOOLS_TESTAPP_TESTASSIMPCODEC_H ? */
