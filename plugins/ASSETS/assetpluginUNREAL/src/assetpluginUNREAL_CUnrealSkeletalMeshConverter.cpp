/*
 *  assetpluginUNREAL: Puts a skeletal mesh into the intermediate scene: bone nodes, skin, one mesh per LOD
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";

/*-------------------------------------------------------------------------*/

MATH::CCoordinateSystem
CUnrealSkeletalMeshConverter::GetCoordinateSystem( void )
{GUCEF_TRACE;

    // The axes of the engine are X forward, Y right, Z up (left handed, centimetres), but a skeletal mesh is modelled facing +Y:
    // the toes of the bind pose point to +Y and the left side of the character is +X. So: -X to the right, Z up, +Y forward.
    return MATH::CCoordinateSystem( MATH::SIGNEDAXIS_NEGATIVE_X, MATH::SIGNEDAXIS_POSITIVE_Z, MATH::SIGNEDAXIS_POSITIVE_Y, 100.0 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Builds the submesh of a section. Returns whether the vertices of the section have a colour other than white: the colour of a
 *  vertex of a skeletal mesh is data of the material of the engine (the masks of the face of a character, for one), which only
 *  uses it where the graph of the material has a vertex colour node. The source material does not say so here, so the colours are
 *  kept as the masks of the vertices (CAssetSubMesh::masks, RGBA divided by 255), not as their colours: a glTF consumer multiplies
 *  the base colour with the colours, which tints the model
 */
static bool
BuildSubMesh( const CUnrealSkeletalMesh::SLod& lod, const CUnrealSkeletalMesh::SSection& section, Int32 materialIndex, UInt32 uvSets, const GUCEF::vector< UInt32 >* jointMap, ASSETCONVERTER::CAssetSubMesh& out )
{GUCEF_TRACE;

    const size_t vertexCount = section.vertices.size();
    out.materialIndex = materialIndex;
    out.positions.resize( vertexCount * 3 );
    out.normals.resize( vertexCount * 3 );
    out.tangents.resize( vertexCount * 4 );
    for ( UInt32 u=0; u<uvSets && u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
        out.uvSets[ u ].resize( vertexCount * 2 );

    bool hasColour = false;
    for ( size_t v=0; v<vertexCount; ++v )
    {
        const CUnrealSkeletalMesh::SVertex& vertex = section.vertices[ v ];
        for ( UInt32 c=0; c<3; ++c )
        {
            out.positions[ v * 3 + c ] = vertex.position[ c ];
            out.normals[ v * 3 + c ] = vertex.tangentZ[ c ];
            out.tangents[ v * 4 + c ] = vertex.tangentX[ c ];
        }
        out.tangents[ v * 4 + 3 ] = vertex.tangentZ[ 3 ] < 0.0f ? -1.0f : 1.0f;
        for ( UInt32 u=0; u<uvSets && u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
        {
            out.uvSets[ u ][ v * 2 ] = vertex.uv[ u ][ 0 ];
            out.uvSets[ u ][ v * 2 + 1 ] = vertex.uv[ u ][ 1 ];
        }
        if ( 255 != vertex.colour[ 0 ] || 255 != vertex.colour[ 1 ] || 255 != vertex.colour[ 2 ] || 255 != vertex.colour[ 3 ] )
            hasColour = true;
    }
    if ( hasColour )
    {
        out.masks.resize( vertexCount * 4 );
        for ( size_t v=0; v<vertexCount; ++v )
            for ( UInt32 c=0; c<4; ++c )
                out.masks[ v * 4 + c ] = (Float32) section.vertices[ v ].colour[ c ] / 255.0f;
    }
    // the influences: all of the section are kept, the weights are normalised to one
    const UInt32 influences = section.maxInfluences > 0 ? section.maxInfluences : 1;
    out.influencesPerVertex = influences;
    out.joints.assign( vertexCount * influences, 0 );
    out.weights.assign( vertexCount * influences, 0.0f );
    for ( size_t v=0; v<vertexCount; ++v )
    {
        const CUnrealSkeletalMesh::SVertex& vertex = section.vertices[ v ];
        UInt32 sum = 0;
        for ( UInt32 i=0; i<influences; ++i )
            sum += vertex.weights[ i ];
        for ( UInt32 i=0; i<influences; ++i )
        {
            if ( 0 == vertex.weights[ i ] || vertex.bones[ i ] >= section.boneMap.size() )
                continue;
            const UInt32 bone = section.boneMap[ vertex.bones[ i ] ];
            out.joints[ v * influences + i ] = ( GUCEF_NULL != jointMap && bone < jointMap->size() ) ? (*jointMap)[ bone ] : bone;
            out.weights[ v * influences + i ] = (Float32) vertex.weights[ i ] / (Float32) sum;
        }
    }

    // the triangles, turned from clockwise to counter-clockwise
    out.indices.resize( (size_t) section.triangleCount * 3 );
    for ( UInt32 t=0; t<section.triangleCount; ++t )
    {
        const UInt32* source = &lod.indices[ section.baseIndex + (size_t) t * 3 ];
        out.indices[ (size_t) t * 3 ] = source[ 0 ] - section.baseVertexIndex;
        out.indices[ (size_t) t * 3 + 1 ] = source[ 2 ] - section.baseVertexIndex;
        out.indices[ (size_t) t * 3 + 2 ] = source[ 1 ] - section.baseVertexIndex;
    }
    return hasColour;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshConverter::AddSkeleton( const CUnrealReferenceSkeleton& skeleton    ,
                                           const CORE::CString& skinName               ,
                                           Int32 parentNode                            ,
                                           ASSETCONVERTER::CAssetScene& scene          ,
                                           UInt32& skinIndex                           ,
                                           GUCEF::vector< UInt32 >& boneNodes          )
{GUCEF_TRACE;

    const CUnrealReferenceSkeleton::TBoneVector& bones = skeleton.GetBones();
    if ( bones.empty() )
        return false;

    // the bones: nodes with their local bind pose, the root bone below the parent node
    boneNodes.assign( bones.size(), 0 );
    for ( size_t b=0; b<bones.size(); ++b )
    {
        const CUnrealReferenceSkeleton::SBone& bone = bones[ b ];
        const UInt32 node = scene.AddNode( bone.name, bone.parent < 0 ? parentNode : (Int32) boneNodes[ (size_t) bone.parent ] );
        boneNodes[ b ] = node;
        SetBindPose( scene.nodes[ node ], bone );
    }

    // the skin: every bone is a joint, the inverse bind matrix is the inverse of the world matrix of the bone in the bind pose
    ASSETCONVERTER::CAssetSkin skin;
    skin.name = skinName;
    skin.skeletonRoot = -1;
    for ( size_t b=0; b<bones.size(); ++b )
    {
        skin.joints.push_back( boneNodes[ b ] );
        skin.inverseBindMatrices.push_back( scene.GetWorldMatrix( boneNodes[ b ] ).InverseAffine() );
        if ( bones[ b ].parent < 0 && skin.skeletonRoot < 0 )
            skin.skeletonRoot = (Int32) boneNodes[ b ];
    }
    scene.skins.push_back( skin );
    skinIndex = (UInt32) scene.skins.size() - 1;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnrealSkeletalMeshConverter::SetBindPose( ASSETCONVERTER::CAssetNode& node, const CUnrealReferenceSkeleton::SBone& bone )
{GUCEF_TRACE;

    MATH::TFloat32Transform3D& transform = node.transform;
    transform.position = MATH::TFloat32Transform3D::TPositionVector( (Float32) bone.translation[ 0 ], (Float32) bone.translation[ 1 ], (Float32) bone.translation[ 2 ] );
    transform.rotation.x = (Float32) bone.rotation[ 0 ];
    transform.rotation.y = (Float32) bone.rotation[ 1 ];
    transform.rotation.z = (Float32) bone.rotation[ 2 ];
    transform.rotation.w = (Float32) bone.rotation[ 3 ];
    transform.scale.x = (Float32) bone.scale[ 0 ];
    transform.scale.y = (Float32) bone.scale[ 1 ];
    transform.scale.z = (Float32) bone.scale[ 2 ];
}

/*-------------------------------------------------------------------------*/

void
CUnrealSkeletalMeshConverter::AddMeshNodes( const CUnrealSkeletalMesh& mesh                  ,
                                            const GUCEF::vector< Int32 >& slotMaterials      ,
                                            const GUCEF::vector< UInt32 >* jointMap          ,
                                            Int32 skinIndex                                  ,
                                            UInt32 holderNode                                ,
                                            ASSETCONVERTER::CAssetScene& scene               ,
                                            ASSETCONVERTER::CConversionReport& report        )
{GUCEF_TRACE;

    // one mesh per LOD
    ASSETCONVERTER::CAssetLodGroup group;
    group.name = mesh.name;
    bool anyScreenSize = false;
    UInt32 sectionsWithMasks = 0;
    for ( size_t l=0; l<mesh.lods.size(); ++l )
    {
        const CUnrealSkeletalMesh::SLod& lod = mesh.lods[ l ];
        ASSETCONVERTER::CAssetMesh assetMesh;
        assetMesh.name = mesh.lods.size() > 1 ? mesh.name + "_LOD" + CORE::UInt32ToString( (UInt32) l ) : mesh.name;
        for ( size_t s=0; s<lod.sections.size(); ++s )
        {
            const CUnrealSkeletalMesh::SSection& section = lod.sections[ s ];
            if ( section.disabled || 0 == section.triangleCount )
                continue;
            ASSETCONVERTER::CAssetSubMesh subMesh;
            const Int32 material = section.materialIndex < slotMaterials.size() ? slotMaterials[ section.materialIndex ] : -1;
            if ( BuildSubMesh( lod, section, material, lod.uvSetCount, jointMap, subMesh ) && 0 == l )
                ++sectionsWithMasks;
            assetMesh.subMeshes.push_back( subMesh );
        }
        scene.meshes.push_back( assetMesh );

        const UInt32 meshNode = scene.AddNode( assetMesh.name, (Int32) holderNode );
        scene.nodes[ meshNode ].mesh = (Int32) scene.meshes.size() - 1;
        scene.nodes[ meshNode ].skin = skinIndex;
        group.AddLevel( meshNode, lod.screenSize > 0.0f ? lod.screenSize : -1.0f );
        anyScreenSize = anyScreenSize || lod.screenSize > 0.0f;
    }
    if ( mesh.lods.size() > 1 )
    {
        scene.lodGroups.push_back( group );
        scene.nodes[ holderNode ].lodGroup = (Int32) scene.lodGroups.size() - 1;
        if ( !anyScreenSize )
            report.AddApproximation( SourceName, mesh.name, "the mesh states no screen sizes for its LODs: the levels are in the order of the LODs" );
    }
    if ( sectionsWithMasks > 0 )
        report.AddInfo( SourceName, mesh.name, CORE::UInt32ToString( sectionsWithMasks ) + " section(s) have vertex colours: they are masks for the material of the engine, not colours: kept as the masks of the vertices (glTF attribute _MASK_0), not as their colours" );
    report.AddInfo( SourceName, mesh.name, "Skeletal mesh with " + CORE::UInt64ToString( mesh.skeleton.GetBones().size() ) + " bones, " + CORE::UInt64ToString( mesh.lods.size() ) + " LODs, LOD 0 with " +
        CORE::UInt32ToString( mesh.GetVertexCount( 0 ) ) + " vertices and " + CORE::UInt32ToString( mesh.GetTriangleCount( 0 ) ) + " triangles" );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshConverter::AddToScene( const CUnrealSkeletalMesh& mesh                  ,
                                          const GUCEF::vector< Int32 >& slotMaterials      ,
                                          ASSETCONVERTER::CAssetScene& scene               ,
                                          ASSETCONVERTER::CConversionReport& report        ,
                                          UInt32& rootNode                                 )
{GUCEF_TRACE;

    if ( mesh.lods.empty() || mesh.skeleton.GetBones().empty() )
    {
        report.AddError( SourceName, mesh.name, "The skeletal mesh has no LOD models or no bones" );
        return false;
    }

    // the node of the mesh comes first, the bones are below it
    const UInt32 meshRoot = scene.AddNode( mesh.name );
    UInt32 skinIndex = 0;
    GUCEF::vector< UInt32 > boneNodes;
    AddSkeleton( mesh.skeleton, mesh.name, (Int32) meshRoot, scene, skinIndex, boneNodes );
    rootNode = meshRoot;

    AddMeshNodes( mesh, slotMaterials, GUCEF_NULL, (Int32) skinIndex, meshRoot, scene, report );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
