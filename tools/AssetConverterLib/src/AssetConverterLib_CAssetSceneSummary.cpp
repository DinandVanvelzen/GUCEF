/*
 *  AssetConverterLib: the summary of an asset for the sidecar and the fidelity tests
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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#include "AssetConverterLib_CAssetSceneSummary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetSceneSummary::CAssetSceneSummary( void )
    : nodeCount( 0 )
    , meshCount( 0 )
    , subMeshCount( 0 )
    , vertexCount( 0 )
    , triangleCount( 0 )
    , materialCount( 0 )
    , textureCount( 0 )
    , skinCount( 0 )
    , boneCount( 0 )
    , morphTargetCount( 0 )
    , colliderCount( 0 )
    , lodGroupCount( 0 )
    , lodLevelCount( 0 )
    , lightCount( 0 )
    , prefabCount( 0 )
    , particleSystemCount( 0 )
    , hasBounds( false )
    , clips()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
        boundsMin[ i ] = boundsMax[ i ] = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAssetSceneSummary::Compute( const CAssetScene& scene )
{GUCEF_TRACE;

    *this = CAssetSceneSummary();
    nodeCount = (UInt32) scene.nodes.size();
    meshCount = (UInt32) scene.meshes.size();
    materialCount = (UInt32) scene.materials.size();
    textureCount = (UInt32) scene.textures.size();
    skinCount = (UInt32) scene.skins.size();
    lodGroupCount = (UInt32) scene.lodGroups.size();
    lightCount = (UInt32) scene.lights.size();
    prefabCount = (UInt32) scene.prefabs.size();
    particleSystemCount = (UInt32) scene.particleSystems.size();

    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        subMeshCount += (UInt32) scene.meshes[ m ].subMeshes.size();
        vertexCount += scene.meshes[ m ].GetVertexCount();
        triangleCount += scene.meshes[ m ].GetTriangleCount();
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
            morphTargetCount += (UInt32) scene.meshes[ m ].subMeshes[ s ].morphTargets.size();
    }

    GUCEF::set< UInt32 > bones;
    for ( size_t s=0; s<scene.skins.size(); ++s )
        for ( size_t j=0; j<scene.skins[ s ].joints.size(); ++j )
            bones.insert( scene.skins[ s ].joints[ j ] );
    boneCount = (UInt32) bones.size();

    for ( size_t g=0; g<scene.lodGroups.size(); ++g )
        lodLevelCount += (UInt32) scene.lodGroups[ g ].levels.size();

    // the bounds of the vertices where the nodes put them
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        colliderCount += (UInt32) scene.nodes[ n ].colliders.size();
        const Int32 meshIndex = scene.nodes[ n ].mesh;
        if ( meshIndex < 0 || (size_t) meshIndex >= scene.meshes.size() )
            continue;
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( (UInt32) n );
        const CAssetMesh& mesh = scene.meshes[ meshIndex ];
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            const CAssetSubMesh::TFloatVector& positions = mesh.subMeshes[ s ].positions;
            for ( size_t v=0; v+2<positions.size(); v+=3 )
            {
                const MATH::TFloat32Point3D p = world.TransformPoint( MATH::TFloat32Point3D( positions[ v ], positions[ v+1 ], positions[ v+2 ] ) );
                const Float32 c[ 3 ] = { p.x, p.y, p.z };
                for ( UInt32 i=0; i<3; ++i )
                {
                    if ( !hasBounds || c[ i ] < boundsMin[ i ] )
                        boundsMin[ i ] = c[ i ];
                    if ( !hasBounds || c[ i ] > boundsMax[ i ] )
                        boundsMax[ i ] = c[ i ];
                }
                hasBounds = true;
            }
        }
    }

    for ( size_t a=0; a<scene.animations.size(); ++a )
    {
        SClip clip;
        clip.name = scene.animations[ a ].name;
        clip.duration = scene.animations[ a ].GetDuration();
        clip.loop = scene.animations[ a ].loop;
        clips.push_back( clip );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetSceneSummary::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "nodes", nodeCount );
    node.SetAttribute( "meshes", meshCount );
    node.SetAttribute( "subMeshes", subMeshCount );
    node.SetAttribute( "vertices", vertexCount );
    node.SetAttribute( "triangles", triangleCount );
    node.SetAttribute( "materials", materialCount );
    node.SetAttribute( "textures", textureCount );
    node.SetAttribute( "skins", skinCount );
    node.SetAttribute( "bones", boneCount );
    node.SetAttribute( "morphTargets", morphTargetCount );
    node.SetAttribute( "colliders", colliderCount );
    node.SetAttribute( "lodGroups", lodGroupCount );
    node.SetAttribute( "lodLevels", lodLevelCount );
    node.SetAttribute( "lights", lightCount );
    node.SetAttribute( "prefabs", prefabCount );
    if ( particleSystemCount > 0 )
        node.SetAttribute( "particleSystems", particleSystemCount );
    if ( hasBounds )
    {
        CORE::CDataNode* bounds = node.AddChild( "bounds", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode::TVariantVector values;
        for ( UInt32 i=0; i<3; ++i )
            values.push_back( CORE::CVariant( boundsMin[ i ] ) );
        bounds->SetValuesOfChildByName( "min", values );
        values.clear();
        for ( UInt32 i=0; i<3; ++i )
            values.push_back( CORE::CVariant( boundsMax[ i ] ) );
        bounds->SetValuesOfChildByName( "max", values );
    }
    CORE::CDataNode* clipList = node.AddChild( "clips", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<clips.size(); ++i )
    {
        CORE::CDataNode* clip = clipList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        clip->SetAttribute( "name", clips[ i ].name );
        clip->SetAttribute( "duration", clips[ i ].duration );
        clip->SetAttribute( "loop", clips[ i ].loop );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
