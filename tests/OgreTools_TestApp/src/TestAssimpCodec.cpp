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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "Ogre.h"
#include "OgreCodec.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#include "TestOgreToolsSupport.h"
#include "TestGltfAsset.h"
#include "TestOgreEnv.h"
#include "TestToolConversion.h"

#include "TestAssimpCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Ogre::String VfsGroup = "OgreToolsVfs";
static const CORE::CString VfsMountPath = "/ogretools_gltf";

/*-------------------------------------------------------------------------*/

static void
TestCodecRegistered( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreCodec_Assimp: the plugin registers the codecs of the formats of Assimp, and not the importers that are left out of the build" )
        try
        {
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "gltf" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "glb" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "fbx" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "obj" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "dae" ) );
            // the importers that are left out of the build
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "ifc" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "usd" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "m3d" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestLoadThroughVfs( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgreCodec_Assimp: MeshManager loads the .gltf from a GucefVFS archive, with the facts of the asset and the same facts as the converter made" )
        try
        {
            const CORE::CString gltfDir = GetGltfDir();
            ASSERT_FALSE( gltfDir.IsNULLOrEmpty() );

            ASSERT_TRUE( AddVfsResourceLocation( VfsGroup, gltfDir, VfsMountPath ) );

            SMeshFacts facts;
            ASSERT_TRUE( LoadMeshFacts( "ogretest.gltf", VfsGroup, facts ) );
            CheckTestAssetFacts( facts );

            // the converter got the same from the same file
            SMeshFacts converted;
            ASSERT_TRUE( LoadMeshFacts( "ogretest.mesh", "OgreToolsConverted", converted ) );
            ASSERT_TRUE( facts.subMeshCount == converted.subMeshCount );
            ASSERT_TRUE( facts.totalVertexCount == converted.totalVertexCount );
            ASSERT_TRUE( facts.boneCount == converted.boneCount );
            ASSERT_TRUE( facts.firstAnimationName == converted.firstAnimationName );

            // the material of the cube is there and its base colour texture is the one of the glTF
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().getByName( "ogretest.gltf", VfsGroup );
            ASSERT_TRUE( (bool) mesh );
            if ( mesh )
            {
                const Ogre::String materialName = mesh->getSubMesh( 0 )->getMaterialName();
                ASSERT_FALSE( materialName.empty() );
                ASSERT_TRUE( materialName.find( "CubeMaterial" ) != Ogre::String::npos );
                Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( materialName, mesh->getGroup() );
                ASSERT_TRUE( (bool) material );
                bool hasTexture = false;
                if ( material && material->getNumTechniques() > 0 && material->getTechnique( 0 )->getNumPasses() > 0 )
                {
                    Ogre::Pass* pass = material->getTechnique( 0 )->getPass( 0 );
                    for ( unsigned short i=0; i<pass->getNumTextureUnitStates(); ++i )
                        hasTexture = hasTexture || pass->getTextureUnitState( i )->getTextureName().find( "checker.png" ) != Ogre::String::npos;
                }
                ASSERT_TRUE( hasTexture );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssimpCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgreTools.AssimpCodec" );

    TestCodecRegistered();
    TestLoadThroughVfs();
}

/*-------------------------------------------------------------------------*/
