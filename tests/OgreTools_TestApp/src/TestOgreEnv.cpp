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
#include "OgreDefaultHardwareBufferManager.h"

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H
#include "pluginglueOGRE_pluginAPI.h"
#define GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H
#endif /* GUCEF_PLUGINGLUE_OGRE_PLUGINAPI_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestGltfAsset.h"
#include "TestOgreEnv.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString OgreToolsPluginGroup = "ogretoolsplugins";

static Ogre::DefaultHardwareBufferManager* g_bufferManager = GUCEF_NULL;
static Ogre::DefaultTextureManager* g_textureManager = GUCEF_NULL;
static CORE::CString::StringVector g_mountedArchiveNames;

/*-------------------------------------------------------------------------*/

SMeshFacts::SMeshFacts( void )
    : subMeshCount( 0 )
    , totalVertexCount( 0 )
    , hasSkeleton( false )
    , boneCount( 0 )
    , hasBone0AndBone1( false )
    , animationCount( 0 )
    , firstAnimationName()
    , firstAnimationLength( 0 )
    , lodLevelCount( 0 )
    , allSubMeshesHaveTangents( false )
    , allSubMeshesHaveUVs( false )
{GUCEF_TRACE;

    for ( int i=0; i<4; ++i )
    {
        vertexCounts[ i ] = 0;
        triangleCounts[ i ] = 0;
    }
}

/*-------------------------------------------------------------------------*/

/** The entry of a plugin in the bootstrap configuration, as the .ini of an application makes it */
static void
AddBootstrapEntry( CORE::CDataNode& group, const CORE::CString& pluginName )
{GUCEF_TRACE;

    CORE::CDataNode* entry = group.AddChild( "PluginMetaData" );
    entry->SetAttribute( "Description", pluginName );
    entry->SetAttribute( "Path", "$MODULEDIR$" );
    entry->SetAttribute( "Filename", pluginName );
    entry->SetAttribute( "AltFilename", pluginName + "_d" );
    entry->SetAttribute( "Type", "OgrePlugin" );
    entry->SetAttribute( "LoadFailAllowed", false );
}

/*-------------------------------------------------------------------------*/

bool
StartOgreToolsEnvironment( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();

    // the library makes the log adapter, the Root and the plugin manager when it loads
    if ( 1 != PLUGINGLUE::OGRE::GUCEFPlugin_Load( 0, GUCEF_NULL ) || GUCEF_NULL == Ogre::Root::getSingletonPtr() )
        return false;

    CORE::CDataNode config;
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", OgreToolsPluginGroup );
    group->SetAttribute( "LoadImmediately", true );
    AddBootstrapEntry( *group, "OgrePlugin_GucefVFS" );
    AddBootstrapEntry( *group, "OgreCodec_Assimp" );
    AddBootstrapEntry( *group, "OgrePlugin_DotScene" );
    if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config ) )
        return false;

    // there is no render system: the buffers are in memory and the textures are not made
    g_bufferManager = OGRE_NEW Ogre::DefaultHardwareBufferManager();
    g_textureManager = OGRE_NEW Ogre::DefaultTextureManager();
    Ogre::MaterialManager::getSingleton().initialise();
    return true;
}

/*-------------------------------------------------------------------------*/

void
StopOgreToolsEnvironment( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
        Ogre::ResourceGroupManager::getSingleton().shutdownAll();

    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        for ( size_t i=0; i<g_mountedArchiveNames.size(); ++i )
            vfs.UnmountArchiveByName( g_mountedArchiveNames[ i ] );
        g_mountedArchiveNames.clear();
    }

    OGRE_DELETE g_textureManager;
    g_textureManager = GUCEF_NULL;
    OGRE_DELETE g_bufferManager;
    g_bufferManager = GUCEF_NULL;

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( OgreToolsPluginGroup );
    PLUGINGLUE::OGRE::GUCEFPlugin_Unload();
}

/*-------------------------------------------------------------------------*/

bool
AddTestResourceLocation( const Ogre::String& group, const CORE::CString& path, const Ogre::String& archiveType )
{GUCEF_TRACE;

    try
    {
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( !groups.resourceGroupExists( group ) )
            groups.createResourceGroup( group );
        groups.addResourceLocation( path.STL_String(), archiveType, group );
        return true;
    }
    catch ( const Ogre::Exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

static bool
HasElement( Ogre::VertexData* data, Ogre::VertexElementSemantic semantic )
{GUCEF_TRACE;

    return GUCEF_NULL != data && GUCEF_NULL != data->vertexDeclaration->findElementBySemantic( semantic );
}

/*-------------------------------------------------------------------------*/

bool
LoadMeshFacts( const Ogre::String& meshName, const Ogre::String& group, SMeshFacts& facts )
{GUCEF_TRACE;

    facts = SMeshFacts();
    try
    {
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( meshName, group );
        if ( !mesh )
            return false;

        facts.subMeshCount = mesh->getNumSubMeshes();
        facts.lodLevelCount = mesh->getNumLodLevels();
        facts.allSubMeshesHaveTangents = facts.subMeshCount > 0;
        facts.allSubMeshesHaveUVs = facts.subMeshCount > 0;
        for ( size_t i=0; i<facts.subMeshCount; ++i )
        {
            Ogre::SubMesh* subMesh = mesh->getSubMesh( (unsigned short) i );
            Ogre::VertexData* vertices = subMesh->useSharedVertices ? mesh->sharedVertexData : subMesh->vertexData;
            const UInt32 vertexCount = GUCEF_NULL == vertices ? 0 : (UInt32) vertices->vertexCount;
            facts.totalVertexCount += vertexCount;
            if ( i < 4 )
            {
                facts.vertexCounts[ i ] = vertexCount;
                facts.triangleCounts[ i ] = (UInt32) ( subMesh->indexData->indexCount / 3 );
            }
            facts.allSubMeshesHaveTangents = facts.allSubMeshesHaveTangents && HasElement( vertices, Ogre::VES_TANGENT );
            facts.allSubMeshesHaveUVs = facts.allSubMeshesHaveUVs && HasElement( vertices, Ogre::VES_TEXTURE_COORDINATES );
        }

        Ogre::SkeletonPtr skeleton = mesh->getSkeleton();
        facts.hasSkeleton = (bool) skeleton;
        if ( skeleton )
        {
            facts.boneCount = skeleton->getNumBones();
            facts.hasBone0AndBone1 = skeleton->hasBone( "Bone0" ) && skeleton->hasBone( "Bone1" );
            facts.animationCount = skeleton->getNumAnimations();
            if ( facts.animationCount > 0 )
            {
                Ogre::Animation* animation = skeleton->getAnimation( (unsigned short) 0 );
                facts.firstAnimationName = animation->getName();
                facts.firstAnimationLength = animation->getLength();
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LoadMeshFacts: " << meshName.c_str() << ": " << (UInt32) facts.subMeshCount << " sub meshes, vertices " << facts.vertexCounts[ 0 ] << "/" << facts.vertexCounts[ 1 ]
            << ", triangles " << facts.triangleCounts[ 0 ] << "/" << facts.triangleCounts[ 1 ] << ", " << (UInt32) facts.lodLevelCount << " lod levels, skeleton " << facts.hasSkeleton
            << " with " << (UInt32) facts.boneCount << " bones and " << (UInt32) facts.animationCount << " animations, first \"" << facts.firstAnimationName.c_str()
            << "\" of " << facts.firstAnimationLength << "s, tangents " << facts.allSubMeshesHaveTangents << ", UVs " << facts.allSubMeshesHaveUVs );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LoadMeshFacts: mesh " << meshName.c_str() << " did not load: " << e.getFullDescription().c_str() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CheckTestAssetFacts( const SMeshFacts& facts )
{GUCEF_TRACE;

    GUCEF_TESTFW_ASSERT_TRUE( 2 == facts.subMeshCount );
    // the cube and the strip, in the order of the glTF nodes
    GUCEF_TESTFW_ASSERT_TRUE( TestGltfCubeVertexCount == facts.vertexCounts[ 0 ] );
    GUCEF_TESTFW_ASSERT_TRUE( TestGltfCubeTriangleCount == facts.triangleCounts[ 0 ] );
    GUCEF_TESTFW_ASSERT_TRUE( TestGltfStripVertexCount == facts.vertexCounts[ 1 ] );
    GUCEF_TESTFW_ASSERT_TRUE( TestGltfStripTriangleCount == facts.triangleCounts[ 1 ] );
    GUCEF_TESTFW_ASSERT_TRUE( facts.allSubMeshesHaveUVs );
    GUCEF_TESTFW_ASSERT_TRUE( facts.hasSkeleton );
    GUCEF_TESTFW_ASSERT_TRUE( TestGltfSkeletonBoneCount == facts.boneCount );
    GUCEF_TESTFW_ASSERT_TRUE( facts.hasBone0AndBone1 );
    GUCEF_TESTFW_ASSERT_TRUE( 1 == facts.animationCount );
    GUCEF_TESTFW_ASSERT_TRUE( "Wave" == facts.firstAnimationName );
    GUCEF_TESTFW_ASSERT_TRUE( facts.firstAnimationLength > TestGltfAnimationLength - 0.01f && facts.firstAnimationLength < TestGltfAnimationLength + 0.01f );
}

/*-------------------------------------------------------------------------*/

bool
AddVfsResourceLocation( const Ogre::String& group, const CORE::CString& dir, const CORE::CString& mountPath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString archiveName = "ogretools" + mountPath;
    if ( !vfs.AddRoot( dir, archiveName, true, false, mountPath ) )
        return false;
    g_mountedArchiveNames.push_back( archiveName );
    return AddTestResourceLocation( group, mountPath, "GucefVFS" );
}

/*-------------------------------------------------------------------------*/

bool
RefreshVfsResourceLocation( const Ogre::String& group, const CORE::CString& mountPath )
{GUCEF_TRACE;

    // Ogre lists the files of a location when it is added: files that came after need the location to be added again
    try
    {
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        groups.removeResourceLocation( mountPath.STL_String(), group );
        groups.addResourceLocation( mountPath.STL_String(), "GucefVFS", group );
        return true;
    }
    catch ( const Ogre::Exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/
