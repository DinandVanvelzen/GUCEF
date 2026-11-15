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

#include "TestOgreToolsSupport.h"
#include "TestOgreEnv.h"
#include "TestToolConversion.h"

#include "TestDotScene.h"

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

static const Ogre::String SceneGroup = "OgreToolsScene";

static const char* TestSceneText =
    "<scene formatVersion=\"1.1\">\n"
    "    <nodes>\n"
    "        <node name=\"Parent\">\n"
    "            <position x=\"1\" y=\"2\" z=\"3\" />\n"
    "            <scale x=\"2\" y=\"2\" z=\"2\" />\n"
    "            <node name=\"Child\">\n"
    "                <position x=\"0\" y=\"5\" z=\"0\" />\n"
    "                <rotation qx=\"0\" qy=\"0.7071068\" qz=\"0\" qw=\"0.7071068\" />\n"
    "                <entity name=\"TheCube\" meshFile=\"cubeonly.mesh\" />\n"
    "            </node>\n"
    "            <node name=\"Sibling\">\n"
    "                <position x=\"-1\" y=\"0\" z=\"0\" />\n"
    "            </node>\n"
    "        </node>\n"
    "        <node name=\"Lamp\">\n"
    "            <position x=\"-4\" y=\"8\" z=\"1\" />\n"
    "            <light name=\"LampLight\" type=\"point\">\n"
    "                <colourDiffuse r=\"0.4\" g=\"0.4\" b=\"0.5\" />\n"
    "            </light>\n"
    "        </node>\n"
    "    </nodes>\n"
    "</scene>\n";

/*-------------------------------------------------------------------------*/

static bool
Near( float a, float b )
{
    return a > b - 0.001f && a < b + 0.001f;
}

/*-------------------------------------------------------------------------*/

static void
TestSceneLoad( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OgrePlugin_DotScene: the codec of the .scene format fills a scene manager with the node tree, entity and light of the file" )
        try
        {
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "scene" ) );

            const CORE::CString sceneDir = MakeCleanTestDir( "scene" );
            ASSERT_TRUE( WriteTestFile( sceneDir + "/test.scene", TestSceneText ) );
            ASSERT_TRUE( AddVfsResourceLocation( SceneGroup, sceneDir, "/ogretools_scene" ) );
            // the entity of the scene is found in the world resource group
            ASSERT_TRUE( AddVfsResourceLocation( Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName(), GetConvertedDir(), "/ogretools_world" ) );

            Ogre::Root& root = Ogre::Root::getSingleton();
            Ogre::SceneManager* sceneManager = root.createSceneManager();
            ASSERT_TRUE( GUCEF_NULL != sceneManager );
            if ( GUCEF_NULL != sceneManager )
            {
                Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( "test.scene", SceneGroup );
                ASSERT_TRUE( (bool) stream );
                Ogre::Codec::getCodec( "scene" )->decode( stream, Ogre::Any( sceneManager->getRootSceneNode() ) );

                Ogre::SceneNode* top = sceneManager->getRootSceneNode();
                ASSERT_TRUE( 2 == top->numChildren() );
                ASSERT_TRUE( sceneManager->hasSceneNode( "Parent" ) );
                ASSERT_TRUE( sceneManager->hasSceneNode( "Child" ) );
                ASSERT_TRUE( sceneManager->hasSceneNode( "Sibling" ) );
                ASSERT_TRUE( sceneManager->hasSceneNode( "Lamp" ) );

                Ogre::SceneNode* parent = sceneManager->getSceneNode( "Parent" );
                ASSERT_TRUE( top == parent->getParent() );
                ASSERT_TRUE( 2 == parent->numChildren() );
                ASSERT_TRUE( Near( 1, parent->getPosition().x ) && Near( 2, parent->getPosition().y ) && Near( 3, parent->getPosition().z ) );
                ASSERT_TRUE( Near( 2, parent->getScale().x ) && Near( 2, parent->getScale().y ) && Near( 2, parent->getScale().z ) );
                ASSERT_TRUE( 0 == parent->numAttachedObjects() );

                Ogre::SceneNode* child = sceneManager->getSceneNode( "Child" );
                ASSERT_TRUE( parent == child->getParent() );
                ASSERT_TRUE( 0 == child->numChildren() );
                ASSERT_TRUE( Near( 5, child->getPosition().y ) );
                ASSERT_TRUE( Near( 0.7071068f, child->getOrientation().y ) && Near( 0.7071068f, child->getOrientation().w ) );
                ASSERT_TRUE( 1 == child->numAttachedObjects() );
                // the position in the world is the one of the parent, with the scale of the parent on the offset of the child
                ASSERT_TRUE( Near( 1, child->_getDerivedPosition().x ) && Near( 12, child->_getDerivedPosition().y ) && Near( 3, child->_getDerivedPosition().z ) );

                ASSERT_TRUE( sceneManager->hasEntity( "TheCube" ) );
                if ( sceneManager->hasEntity( "TheCube" ) )
                {
                    Ogre::Entity* entity = sceneManager->getEntity( "TheCube" );
                    ASSERT_TRUE( entity == child->getAttachedObject( 0 ) );
                    ASSERT_TRUE( "cubeonly.mesh" == entity->getMesh()->getName() );
                    ASSERT_TRUE( 1 == entity->getNumSubEntities() );
                }

                Ogre::SceneNode* sibling = sceneManager->getSceneNode( "Sibling" );
                ASSERT_TRUE( parent == sibling->getParent() );
                ASSERT_TRUE( Near( -1, sibling->getPosition().x ) );
                ASSERT_TRUE( 0 == sibling->numAttachedObjects() );

                Ogre::SceneNode* lamp = sceneManager->getSceneNode( "Lamp" );
                ASSERT_TRUE( top == lamp->getParent() );
                ASSERT_TRUE( Near( -4, lamp->getPosition().x ) && Near( 8, lamp->getPosition().y ) );
                ASSERT_TRUE( sceneManager->hasLight( "LampLight" ) );
                if ( sceneManager->hasLight( "LampLight" ) )
                {
                    Ogre::Light* light = sceneManager->getLight( "LampLight" );
                    ASSERT_TRUE( 1 == lamp->numAttachedObjects() );
                    ASSERT_TRUE( Ogre::Light::LT_POINT == light->getType() );
                    ASSERT_TRUE( Near( 0.4f, light->getDiffuseColour().r ) && Near( 0.5f, light->getDiffuseColour().b ) );
                }

                root.destroySceneManager( sceneManager );
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
PerformDotSceneTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgreTools.DotScene" );

    TestSceneLoad();
}

/*-------------------------------------------------------------------------*/
