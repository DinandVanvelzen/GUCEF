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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "Ogre.h"
#include "OgreLodStrategy.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestCookerSupport.h"
#include "TestCookerScenes.h"
#include "TestCookerMeshes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const Ogre::String CharacterGroup = "CookerCharacter";
const Ogre::String LodGroup = "CookerLod";
const Float32 QuarterPi = 0.78539816f;

/** Cooks the character of BuildCharacterScene() as "Characters/Hero.fbx" into "work/character" */
bool
CookCharacter( CConversionReport& report )
{
    MakeWorkFolder( "character" );
    if ( !WriteTestPng( "work/character/src/tex/skin.png", 4, 4, 200, 150, 120, 255 ) || !WriteTestPng( "work/character/src/tex/normal.png", 4, 4, 128, 128, 255, 255 ) )
        return false;
    CAssetScene scene;
    BuildCharacterScene( scene, "work/character/src/tex" );
    CCookSettings settings;
    CCookSettings::SAsset asset;
    if ( !MakeIntermediate( scene, "character", "Characters/Hero.fbx", asset ) )
        return false;
    settings.assets.push_back( asset );
    return CookIntermediate( "character", settings, report );
}

/** Cooks the LOD scene as "Props/Crate.fbx" into "work/<folder>" with the LOD policy */
bool
CookLods( const CORE::CString& folder, const CORE::CString& policy, const CORE::CDataNode* lodSettings, CConversionReport& report )
{
    MakeWorkFolder( folder );
    CAssetScene scene;
    BuildLodScene( scene );
    CCookSettings settings;
    settings.lodPolicy = policy;
    if ( GUCEF_NULL != lodSettings )
        settings.lodSettings = *lodSettings;
    CCookSettings::SAsset asset;
    if ( !MakeIntermediate( scene, folder, "Props/Crate.fbx", asset ) )
        return false;
    settings.assets.push_back( asset );
    return CookIntermediate( folder, settings, report );
}

bool
HasElement( const Ogre::SubMesh* subMesh, Ogre::VertexElementSemantic semantic, unsigned short index = 0 )
{
    return GUCEF_NULL != subMesh->vertexData && GUCEF_NULL != subMesh->vertexData->vertexDeclaration->findElementBySemantic( semantic, index );
}

bool
NearVector( const Ogre::Vector3& v, Float32 x, Float32 y, Float32 z )
{
    return Near( (float) v.x, x ) && Near( (float) v.y, y ) && Near( (float) v.z, z );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCookerMeshesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetcookerOGRE.Meshes" );

    CConversionReport characterReport;
    bool characterCooked = false;

    GUCEF_TESTFW_TESTCASE( "Mesh: the skinned mesh loads back with a submesh per material, its vertex and index counts, normals, UVs and generated tangents" )
    try
    {
        characterCooked = CookCharacter( characterReport );
        ASSERT_TRUE( characterCooked );
        ASSERT_FALSE( characterReport.HasFailures() );
        ASSERT_TRUE( VfsFileExists( GetProfileDir( "character" ) + "/pack/Characters/Hero/BodyMesh.mesh" ) );
        ASSERT_TRUE( UseOgreGroup( CharacterGroup, GetProfileDir( "character" ) ) );

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Characters/Hero/BodyMesh.mesh", CharacterGroup );
        ASSERT_TRUE( (bool) mesh );
        if ( mesh )
        {
            // two submeshes of Skin with the same streams became one; Armour is the other
            ASSERT_TRUE( 2 == mesh->getNumSubMeshes() );
            if ( 2 == mesh->getNumSubMeshes() )
            {
                const Ogre::SubMesh* skin = mesh->getSubMesh( 0 );
                const Ogre::SubMesh* armour = mesh->getSubMesh( 1 );
                ASSERT_TRUE( "pack/Characters/Hero/Skin" == skin->getMaterialName() );
                ASSERT_TRUE( "pack/Characters/Hero/Armour" == armour->getMaterialName() );
                ASSERT_TRUE( !skin->useSharedVertices && 8 == skin->vertexData->vertexCount && 12 == skin->indexData->indexCount );
                ASSERT_TRUE( 3 == armour->vertexData->vertexCount && 3 == armour->indexData->indexCount );
                ASSERT_TRUE( Ogre::HardwareIndexBuffer::IT_16BIT == skin->indexData->indexBuffer->getType() );
                ASSERT_TRUE( HasElement( skin, Ogre::VES_POSITION ) && HasElement( skin, Ogre::VES_NORMAL ) && HasElement( skin, Ogre::VES_TEXTURE_COORDINATES ) );
                ASSERT_TRUE( HasElement( armour, Ogre::VES_NORMAL ) && HasElement( armour, Ogre::VES_TEXTURE_COORDINATES ) );
                // tangents only where a normal map needs them; the source had none, so they were generated
                ASSERT_FALSE( HasElement( skin, Ogre::VES_TANGENT ) );
                ASSERT_TRUE( HasElement( armour, Ogre::VES_TANGENT ) );
                if ( HasElement( armour, Ogre::VES_TANGENT ) )
                    ASSERT_TRUE( Ogre::VET_FLOAT4 == armour->vertexData->vertexDeclaration->findElementBySemantic( Ogre::VES_TANGENT )->getType() );
                ASSERT_TRUE( ReportHas( characterReport, REPORTITEM_APPROXIMATION, "tangents of submesh" ) );
            }
            ASSERT_TRUE( NearVector( mesh->getBounds().getMinimum(), -1.0f, -1.0f, 0.0f ) && NearVector( mesh->getBounds().getMaximum(), 1.0f, 1.0f, 0.0f ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mesh: a vertex with six influences keeps the strongest four, renormalised, and the report says so" )
    try
    {
        ASSERT_TRUE( characterCooked );
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Characters/Hero/BodyMesh.mesh", CharacterGroup );
        ASSERT_TRUE( mesh && mesh->getNumSubMeshes() > 0 );
        if ( mesh && mesh->getNumSubMeshes() > 0 )
        {
            const Ogre::SubMesh::VertexBoneAssignmentList& assignments = mesh->getSubMesh( 0 )->getBoneAssignments();
            UInt32 count = 0;
            Float32 total = 0.0f;
            Float32 weightOfBone[ 6 ] = { 0, 0, 0, 0, 0, 0 };
            Ogre::SubMesh::VertexBoneAssignmentList::const_iterator i = assignments.lower_bound( 0 );
            while ( i != assignments.upper_bound( 0 ) )
            {
                ++count;
                total += (Float32) (*i).second.weight;
                if ( (*i).second.boneIndex < 6 )
                    weightOfBone[ (*i).second.boneIndex ] = (Float32) (*i).second.weight;
                ++i;
            }
            ASSERT_TRUE( 4 == count );
            ASSERT_TRUE( Near( total, 1.0f ) );
            // 0.3, 0.25, 0.2 and the first of the two 0.1 influences, over 0.85
            ASSERT_TRUE( Near( weightOfBone[ 0 ], 0.3f / 0.85f ) && Near( weightOfBone[ 1 ], 0.25f / 0.85f ) && Near( weightOfBone[ 2 ], 0.2f / 0.85f ) && Near( weightOfBone[ 3 ], 0.1f / 0.85f ) );
            ASSERT_TRUE( 0.0f == weightOfBone[ 4 ] && 0.0f == weightOfBone[ 5 ] );
            // a vertex with four influences keeps them
            ASSERT_TRUE( 4 == assignments.count( 3 ) );
        }
        ASSERT_TRUE( ReportHas( characterReport, REPORTITEM_APPROXIMATION, "1 vertices had up to 6 influences; reduced to the strongest 4 and renormalised" ) );
        ASSERT_TRUE( 256 == GetOgreCooker()->GetMaxBonesPerSubMesh() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Skeleton: the bones with the bind pose of the inverse bind matrices, and every clip that animates a joint with its length and keys" )
    try
    {
        ASSERT_TRUE( characterCooked );
        ASSERT_TRUE( VfsFileExists( GetProfileDir( "character" ) + "/pack/Characters/Hero/Rig.skeleton" ) );
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Characters/Hero/BodyMesh.mesh", CharacterGroup );
        ASSERT_TRUE( mesh && mesh->hasSkeleton() && "pack/Characters/Hero/Rig.skeleton" == mesh->getSkeletonName() );
        Ogre::SkeletonPtr skeleton = mesh ? mesh->getSkeleton() : Ogre::SkeletonPtr();
        ASSERT_TRUE( (bool) skeleton );
        if ( skeleton )
        {
            skeleton->load();
            ASSERT_TRUE( 6 == skeleton->getNumBones() );
            const char* const names[ 6 ] = { "Hip", "Spine", "Head", "ArmL", "ArmR", "Tail" };
            for ( unsigned short b=0; b<6 && b<skeleton->getNumBones(); ++b )
                ASSERT_TRUE( names[ b ] == skeleton->getBone( b )->getName() );
            if ( 6 == skeleton->getNumBones() )
            {
                skeleton->reset( true );
                ASSERT_TRUE( GUCEF_NULL == skeleton->getBone( "Hip" )->getParent() );
                ASSERT_TRUE( skeleton->getBone( "Spine" ) == skeleton->getBone( "Head" )->getParent() );
                ASSERT_TRUE( skeleton->getBone( "Hip" ) == skeleton->getBone( "Tail" )->getParent() );
                ASSERT_TRUE( NearVector( skeleton->getBone( "Hip" )->getInitialPosition(), 0.0f, 1.0f, 0.0f ) );
                ASSERT_TRUE( NearVector( skeleton->getBone( "Spine" )->getInitialPosition(), 0.0f, 0.5f, 0.0f ) );
                ASSERT_TRUE( NearVector( skeleton->getBone( "ArmL" )->getInitialPosition(), 0.5f, 0.0f, 0.0f ) );
                ASSERT_TRUE( NearVector( skeleton->getBone( "Head" )->_getDerivedPosition(), 0.0f, 2.0f, 0.0f ) );
                ASSERT_TRUE( NearVector( skeleton->getBone( "Tail" )->_getDerivedPosition(), 0.0f, 1.0f, -0.3f ) );
            }

            // Walk and Wave animate joints; Blink (weights) and Spin (a node that is no joint) do not
            ASSERT_TRUE( 2 == skeleton->getNumAnimations() );
            ASSERT_TRUE( skeleton->hasAnimation( "Walk" ) && skeleton->hasAnimation( "Wave" ) );
            ASSERT_FALSE( skeleton->hasAnimation( "Blink" ) || skeleton->hasAnimation( "Spin" ) );
            if ( skeleton->hasAnimation( "Walk" ) )
            {
                Ogre::Animation* walk = skeleton->getAnimation( "Walk" );
                ASSERT_TRUE( Near( (float) walk->getLength(), 2.0f ) );
                ASSERT_TRUE( 3 == walk->getNumNodeTracks() );
                ASSERT_TRUE( walk->hasNodeTrack( 0 ) && walk->hasNodeTrack( 1 ) && walk->hasNodeTrack( 2 ) );
                if ( walk->hasNodeTrack( 0 ) )
                {
                    // the keys hold the offset from the binding pose: Hip rests at (0, 1, 0)
                    Ogre::NodeAnimationTrack* hip = walk->getNodeTrack( 0 );
                    ASSERT_TRUE( 3 == hip->getNumKeyFrames() );
                    if ( 3 == hip->getNumKeyFrames() )
                    {
                        ASSERT_TRUE( Near( (float) hip->getNodeKeyFrame( 1 )->getTime(), 0.5f ) );
                        ASSERT_TRUE( NearVector( hip->getNodeKeyFrame( 1 )->getTranslate(), 0.0f, 0.2f, 0.3f ) );
                        ASSERT_TRUE( NearVector( hip->getNodeKeyFrame( 2 )->getTranslate(), 0.0f, 0.0f, 0.6f ) );
                    }
                }
                if ( walk->hasNodeTrack( 1 ) && 3 == walk->getNodeTrack( 1 )->getNumKeyFrames() )
                {
                    const Ogre::Quaternion rotation = walk->getNodeTrack( 1 )->getNodeKeyFrame( 1 )->getRotation();
                    ASSERT_TRUE( Near( (float) rotation.y, 0.38268343f ) && Near( (float) rotation.w, 0.92387953f ) );
                }
                if ( walk->hasNodeTrack( 2 ) && 2 == walk->getNodeTrack( 2 )->getNumKeyFrames() )
                    ASSERT_TRUE( NearVector( walk->getNodeTrack( 2 )->getNodeKeyFrame( 1 )->getScale(), 2.0f, 2.0f, 2.0f ) );
            }
            if ( skeleton->hasAnimation( "Wave" ) )
            {
                Ogre::Animation* wave = skeleton->getAnimation( "Wave" );
                ASSERT_TRUE( Near( (float) wave->getLength(), 1.5f ) );
                ASSERT_TRUE( 1 == wave->getNumNodeTracks() && wave->hasNodeTrack( 3 ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Poses: the morph targets are poses and the clip of their weights a pose animation" )
    try
    {
        ASSERT_TRUE( characterCooked );
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Characters/Hero/PropMesh.mesh", CharacterGroup );
        ASSERT_TRUE( (bool) mesh );
        if ( mesh )
        {
            ASSERT_FALSE( mesh->hasSkeleton() );
            ASSERT_TRUE( 2 == mesh->getPoseCount() );
            if ( 2 == mesh->getPoseCount() )
            {
                ASSERT_TRUE( "Smile" == mesh->getPose( 0 )->getName() && "Blink" == mesh->getPose( 1 )->getName() );
                ASSERT_TRUE( 1 == mesh->getPose( 0 )->getTarget() );
                const Ogre::Pose::VertexOffsetMap& offsets = mesh->getPose( 0 )->getVertexOffsets();
                ASSERT_TRUE( offsets.find( 3 ) != offsets.end() );
                if ( offsets.find( 3 ) != offsets.end() )
                    ASSERT_TRUE( Near( offsets.find( 3 )->second.y, 0.3f ) );
            }
            ASSERT_TRUE( mesh->hasAnimation( "Blink" ) );
            ASSERT_FALSE( mesh->hasAnimation( "Spin" ) );
            if ( mesh->hasAnimation( "Blink" ) )
            {
                Ogre::Animation* blink = mesh->getAnimation( "Blink" );
                ASSERT_TRUE( Near( (float) blink->getLength(), 1.0f ) );
                ASSERT_TRUE( 1 == blink->getNumVertexTracks() && blink->hasVertexTrack( 1 ) );
                if ( blink->hasVertexTrack( 1 ) )
                {
                    Ogre::VertexAnimationTrack* track = blink->getVertexTrack( 1 );
                    ASSERT_TRUE( Ogre::VAT_POSE == track->getAnimationType() );
                    ASSERT_TRUE( 3 == track->getNumKeyFrames() );
                    if ( 3 == track->getNumKeyFrames() )
                    {
                        const Ogre::VertexPoseKeyFrame::PoseRefList& references = track->getVertexPoseKeyFrame( 1 )->getPoseReferences();
                        ASSERT_TRUE( 2 == references.size() );
                        if ( 2 == references.size() )
                        {
                            ASSERT_TRUE( 0 == references[ 0 ].poseIndex && Near( references[ 0 ].influence, 1.0f ) );
                            ASSERT_TRUE( 1 == references[ 1 ].poseIndex && Near( references[ 1 ].influence, 0.25f ) );
                        }
                    }
                }
            }
            // the bounds hold the morphed positions too
            ASSERT_TRUE( mesh->getBounds().getMaximum().y >= 0.8f - 1e-4f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    DropOgreGroup( CharacterGroup );

    GUCEF_TESTFW_TESTCASE( "LOD: the levels of a LOD group become manual levels with the screen ratio pixel count strategy" )
    try
    {
        CConversionReport report;
        ASSERT_TRUE( CookLods( "lodsource", "source", GUCEF_NULL, report ) );
        ASSERT_FALSE( report.HasFailures() );
        const CORE::CString folder = GetProfileDir( "lodsource" ) + "/pack/Props/Crate/";
        ASSERT_TRUE( VfsFileExists( folder + "CrateHigh.mesh" ) && VfsFileExists( folder + "CrateMid.mesh" ) && VfsFileExists( folder + "CrateLow.mesh" ) && VfsFileExists( folder + "GridMesh.mesh" ) );
        ASSERT_TRUE( UseOgreGroup( LodGroup, GetProfileDir( "lodsource" ) ) );
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Props/Crate/CrateHigh.mesh", LodGroup );
        ASSERT_TRUE( mesh && 3 == mesh->getNumLodLevels() );
        if ( mesh && 3 == mesh->getNumLodLevels() )
        {
            ASSERT_TRUE( "screen_ratio_pixel_count" == mesh->getLodStrategy()->getName() );
            ASSERT_TRUE( mesh->hasManualLodLevel() );
            const Ogre::MeshLodUsage& middle = mesh->getLodLevel( 1 );
            ASSERT_TRUE( "pack/Props/Crate/CrateMid.mesh" == middle.manualName );
            ASSERT_TRUE( Near( (float) middle.userValue, QuarterPi * 0.3f * 0.3f ) );
            ASSERT_TRUE( middle.manualMesh && 16 == middle.manualMesh->getSubMesh( 0 )->vertexData->vertexCount );
            const Ogre::MeshLodUsage& low = mesh->getLodLevel( 2 );
            ASSERT_TRUE( "pack/Props/Crate/CrateLow.mesh" == low.manualName );
            ASSERT_TRUE( Near( (float) low.userValue, QuarterPi * 0.1f * 0.1f ) );
        }
        // a mesh without a LOD group gets no levels with the policy "source"
        Ogre::MeshPtr grid = Ogre::MeshManager::getSingleton().load( "pack/Props/Crate/GridMesh.mesh", LodGroup );
        ASSERT_TRUE( grid && 1 == grid->getNumLodLevels() );
        // the other levels are not nodes of the scene
        const CORE::CString scene = ReadVfsText( folder + "Crate.scene" );
        ASSERT_TRUE( scene.HasSubstr( "Crate_LOD0" ) >= 0 && scene.HasSubstr( "Crate_LOD1" ) < 0 && scene.HasSubstr( "Crate_LOD2" ) < 0 );
        DropOgreGroup( LodGroup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LOD: the policy generate runs MeshLodGenerator with the reduction table; the policy none keeps the first level only" )
    try
    {
        CORE::CDataNode lod( "lod", GUCEF_DATATYPE_OBJECT );
        lod.SetAttribute( "policy", "generate" );
        CORE::CDataNode* levels = lod.AddChild( "levels", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* first = levels->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        first->SetAttribute( "screenRelativeHeight", 0.5f );
        first->SetAttribute( "reduction", 0.5f );
        CORE::CDataNode* second = levels->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        second->SetAttribute( "screenRelativeHeight", 0.2f );
        second->SetAttribute( "reduction", 0.75f );
        CConversionReport report;
        ASSERT_TRUE( CookLods( "lodgenerate", "generate", &lod, report ) );
        ASSERT_FALSE( report.HasFailures() );
        ASSERT_TRUE( UseOgreGroup( LodGroup, GetProfileDir( "lodgenerate" ) ) );
        Ogre::MeshPtr grid = Ogre::MeshManager::getSingleton().load( "pack/Props/Crate/GridMesh.mesh", LodGroup );
        ASSERT_TRUE( grid && 3 == grid->getNumLodLevels() );
        if ( grid && 3 == grid->getNumLodLevels() )
        {
            ASSERT_FALSE( grid->hasManualLodLevel() );
            ASSERT_TRUE( "screen_ratio_pixel_count" == grid->getLodStrategy()->getName() );
            ASSERT_TRUE( Near( (float) grid->getLodLevel( 1 ).userValue, QuarterPi * 0.25f ) );
            ASSERT_TRUE( Near( (float) grid->getLodLevel( 2 ).userValue, QuarterPi * 0.04f ) );
            // the reduced levels have fewer indices
            const Ogre::SubMesh* subMesh = grid->getSubMesh( 0 );
            ASSERT_TRUE( 2 == subMesh->mLodFaceList.size() );
            if ( 2 == subMesh->mLodFaceList.size() )
                ASSERT_TRUE( subMesh->mLodFaceList[ 0 ]->indexCount < subMesh->indexData->indexCount && subMesh->mLodFaceList[ 1 ]->indexCount < subMesh->mLodFaceList[ 0 ]->indexCount );
        }
        // the LOD group keeps its source levels
        Ogre::MeshPtr crate = Ogre::MeshManager::getSingleton().load( "pack/Props/Crate/CrateHigh.mesh", LodGroup );
        ASSERT_TRUE( crate && crate->hasManualLodLevel() && 3 == crate->getNumLodLevels() );
        DropOgreGroup( LodGroup );

        CConversionReport noneReport;
        ASSERT_TRUE( CookLods( "lodnone", "none", GUCEF_NULL, noneReport ) );
        ASSERT_TRUE( UseOgreGroup( LodGroup, GetProfileDir( "lodnone" ) ) );
        Ogre::MeshPtr only = Ogre::MeshManager::getSingleton().load( "pack/Props/Crate/CrateHigh.mesh", LodGroup );
        ASSERT_TRUE( only && 1 == only->getNumLodLevels() );
        ASSERT_FALSE( VfsFileExists( GetProfileDir( "lodnone" ) + "/pack/Props/Crate/CrateMid.mesh" ) );
        ASSERT_TRUE( ReportHas( noneReport, REPORTITEM_SKIPPED, "LOD policy none" ) );
        DropOgreGroup( LodGroup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
