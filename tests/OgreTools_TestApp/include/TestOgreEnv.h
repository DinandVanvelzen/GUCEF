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

#ifndef GUCEF_OGRETOOLS_TESTAPP_TESTOGREENV_H
#define GUCEF_OGRETOOLS_TESTAPP_TESTOGREENV_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreMesh.h"

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
 *  Makes the Ogre systems the tests work in, without a render system: the Root of the support library
 *  (pluginglueOGRE), the plugins OgrePlugin_GucefVFS, OgreCodec_Assimp and OgrePlugin_DotScene loaded through
 *  GUCEF plugin control as the bootstrap configuration of an application does, and the buffer and texture managers
 *  of offline processing. Returns false when something is missing.
 */
bool
StartOgreToolsEnvironment( void );

/** Takes the systems of StartOgreToolsEnvironment() down again */
void
StopOgreToolsEnvironment( void );

/** What the tests check of a mesh */
struct SMeshFacts
{
    size_t subMeshCount;
    GUCEF::UInt32 vertexCounts[ 4 ];        /**< vertices of the first 4 sub meshes, in their order */
    GUCEF::UInt32 triangleCounts[ 4 ];      /**< triangles of the first 4 sub meshes */
    GUCEF::UInt32 totalVertexCount;
    bool hasSkeleton;
    unsigned short boneCount;
    bool hasBone0AndBone1;
    unsigned short animationCount;
    Ogre::String firstAnimationName;
    float firstAnimationLength;
    unsigned short lodLevelCount;
    bool allSubMeshesHaveTangents;
    bool allSubMeshesHaveUVs;

    SMeshFacts( void );
};

/** Loads the mesh of the name from the resource group and collects its facts. False when it does not load */
bool
LoadMeshFacts( const Ogre::String& meshName, const Ogre::String& group, SMeshFacts& facts );

/** Asserts (as test assertions) the facts of the test asset with a skinned strip: what every way of getting the mesh must give */
void
CheckTestAssetFacts( const SMeshFacts& facts );

/** Makes a resource group with one location of the given archive type */
bool
AddTestResourceLocation( const Ogre::String& group, const GUCEF::CORE::CString& path, const Ogre::String& archiveType );

/**
 *  Mounts a directory of the file system in the VFS (read only, at the mount path) and makes it the location of a
 *  resource group. The archive types of OgrePlugin_GucefVFS read through the VFS, that includes the type "FileSystem".
 *  The mount is taken away by StopOgreToolsEnvironment().
 */
bool
AddVfsResourceLocation( const Ogre::String& group, const GUCEF::CORE::CString& dir, const GUCEF::CORE::CString& mountPath );

/** Lists the location of AddVfsResourceLocation() again, for files that were made after it was added */
bool
RefreshVfsResourceLocation( const Ogre::String& group, const GUCEF::CORE::CString& mountPath );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRETOOLS_TESTAPP_TESTOGREENV_H ? */
