/*
 *  AssetWorkbenchLib: a post-processor for the preview of a presentation
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

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#include "AssetWorkbenchLib_CPackPreviewPostProcessor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Fewer nodes with a mesh than this are not merged: a model of some parts keeps them */
static const UInt32 MinimumMeshNodesToMerge = 200;

const CORE::CString CPackPreviewPostProcessor::ProcessorName = "workbench-preview";
const CORE::CString CPackPreviewPostProcessor::MergedProcessorName = "workbench-preview-merged";

/*-------------------------------------------------------------------------*/

CPackPreviewPostProcessor::CPackPreviewPostProcessor( bool merge )
    : ASSETCONVERTER::CIAssetPostProcessor()
    , m_merge( merge )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackPreviewPostProcessor::~CPackPreviewPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString CPackPreviewPostProcessor::GetName( void ) const { return m_merge ? MergedProcessorName : ProcessorName; }
UInt32 CPackPreviewPostProcessor::GetVersion( void ) const { return 2; }

/*-------------------------------------------------------------------------*/

bool
CPackPreviewPostProcessor::Process( ASSETCONVERTER::CAssetScene& scene, const CORE::CDataNode& settings, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    // only the first level of every group of levels keeps its mesh
    UInt32 dropped = 0;
    for ( size_t g=0; g<scene.lodGroups.size(); ++g )
    {
        const ASSETCONVERTER::CAssetLodGroup::TLevelVector& levels = scene.lodGroups[ g ].levels;
        for ( size_t l=1; l<levels.size(); ++l )
        {
            if ( levels[ l ].node < scene.nodes.size() && scene.nodes[ levels[ l ].node ].mesh >= 0 )
            {
                scene.nodes[ levels[ l ].node ].mesh = -1;
                ++dropped;
            }
        }
    }
    // the rig stays: the skins, the weights and the animations are what the character is moved by; the viewer plays them
    report.AddInfo( GetName(), scene.name, "The preview keeps the rig: " + CORE::UInt32ToString( dropped ) + " levels of detail are left out" );
    if ( m_merge )
    {
        UInt32 before = 0, after = 0;
        if ( MergeStaticMeshes( scene, MinimumMeshNodesToMerge, before, after ) && after < before )
            report.AddInfo( GetName(), scene.name, "The preview merges the static geometry by material: " + CORE::UInt32ToString( before ) + " submeshes became " + CORE::UInt32ToString( after ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

/** A submesh group that is merged: the same material and the same streams */
struct SMergeKey
{
    Int32 material;
    UInt32 uvMask;
    bool normals, tangents, colours;
    bool operator<( const SMergeKey& o ) const
    {
        if ( material != o.material ) return material < o.material;
        if ( uvMask != o.uvMask ) return uvMask < o.uvMask;
        if ( normals != o.normals ) return normals < o.normals;
        if ( tangents != o.tangents ) return tangents < o.tangents;
        return colours < o.colours;
    }
};

SMergeKey
KeyOf( const ASSETCONVERTER::CAssetSubMesh& sub )
{
    SMergeKey key;
    key.material = sub.materialIndex;
    key.uvMask = 0;
    const UInt32 vertices = sub.GetVertexCount();
    for ( UInt32 u=0; u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
        if ( sub.uvSets[ u ].size() == (size_t) vertices * 2 )
            key.uvMask |= 1 << u;
    key.normals = sub.normals.size() == (size_t) vertices * 3;
    key.tangents = sub.tangents.size() == (size_t) vertices * 4;
    key.colours = sub.colours.size() == (size_t) vertices * 4;
    return key;
}

}

bool
CPackPreviewPostProcessor::MergeStaticMeshes( ASSETCONVERTER::CAssetScene& scene, UInt32 minimumMeshNodes, UInt32& submeshesBefore, UInt32& submeshesAfter )
{GUCEF_TRACE;

    submeshesBefore = 0;
    submeshesAfter = 0;

    // the nodes that animations move are not static
    GUCEF::set< UInt32 > moving;
    for ( size_t a=0; a<scene.animations.size(); ++a )
        for ( size_t c=0; c<scene.animations[ a ].channels.size(); ++c )
            moving.insert( scene.animations[ a ].channels[ c ].targetNode );

    GUCEF::vector< UInt32 > candidates;
    for ( UInt32 n=0; n<(UInt32) scene.nodes.size(); ++n )
    {
        const ASSETCONVERTER::CAssetNode& node = scene.nodes[ n ];
        if ( node.mesh < 0 || (size_t) node.mesh >= scene.meshes.size() || node.skin >= 0 || moving.find( n ) != moving.end() )
            continue;
        bool plain = true;
        const ASSETCONVERTER::CAssetMesh& mesh = scene.meshes[ (size_t) node.mesh ];
        for ( size_t s=0; s<mesh.subMeshes.size() && plain; ++s )
            plain = mesh.subMeshes[ s ].morphTargets.empty() && !mesh.subMeshes[ s ].IsSkinned();
        if ( plain )
            candidates.push_back( n );
    }
    if ( candidates.size() < minimumMeshNodes )
        return false;

    typedef GUCEF::map< SMergeKey, ASSETCONVERTER::CAssetSubMesh > TMergedMap;
    TMergedMap merged;
    for ( size_t i=0; i<candidates.size(); ++i )
    {
        const UInt32 n = candidates[ i ];
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( n );
        // a mirror turns the triangles over
        const MATH::TFloat32Matrix4D::TVector3 ax = world.TransformVector( MATH::TFloat32Matrix4D::TVector3( 1.0f, 0.0f, 0.0f ) );
        const MATH::TFloat32Matrix4D::TVector3 ay = world.TransformVector( MATH::TFloat32Matrix4D::TVector3( 0.0f, 1.0f, 0.0f ) );
        const MATH::TFloat32Matrix4D::TVector3 az = world.TransformVector( MATH::TFloat32Matrix4D::TVector3( 0.0f, 0.0f, 1.0f ) );
        const Float32 determinant = ax.x * ( ay.y * az.z - ay.z * az.y ) - ax.y * ( ay.x * az.z - ay.z * az.x ) + ax.z * ( ay.x * az.y - ay.y * az.x );
        const bool mirrored = determinant < 0.0f;
        const MATH::TFloat32Matrix4D normalMatrix = world.InverseAffine().Transposed();

        const ASSETCONVERTER::CAssetMesh& mesh = scene.meshes[ (size_t) scene.nodes[ n ].mesh ];
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            const ASSETCONVERTER::CAssetSubMesh& sub = mesh.subMeshes[ s ];
            const UInt32 vertices = sub.GetVertexCount();
            if ( 0 == vertices || sub.indices.empty() )
                continue;
            ++submeshesBefore;
            ASSETCONVERTER::CAssetSubMesh& target = merged[ KeyOf( sub ) ];
            target.materialIndex = sub.materialIndex;
            const UInt32 offset = target.GetVertexCount();
            for ( UInt32 v=0; v<vertices; ++v )
            {
                const MATH::TFloat32Point3D p = world.TransformPoint( MATH::TFloat32Point3D( sub.positions[ v*3 ], sub.positions[ v*3+1 ], sub.positions[ v*3+2 ] ) );
                target.positions.push_back( p.x ); target.positions.push_back( p.y ); target.positions.push_back( p.z );
                if ( sub.normals.size() == (size_t) vertices * 3 )
                {
                    const MATH::TFloat32Matrix4D::TVector3 d = normalMatrix.TransformVector( MATH::TFloat32Matrix4D::TVector3( sub.normals[ v*3 ], sub.normals[ v*3+1 ], sub.normals[ v*3+2 ] ) );
                    const Float32 length = sqrtf( d.x*d.x + d.y*d.y + d.z*d.z );
                    const Float32 scale = length > 1e-12f ? 1.0f / length : 1.0f;
                    target.normals.push_back( d.x * scale ); target.normals.push_back( d.y * scale ); target.normals.push_back( d.z * scale );
                }
                if ( sub.tangents.size() == (size_t) vertices * 4 )
                {
                    const MATH::TFloat32Matrix4D::TVector3 d = world.TransformVector( MATH::TFloat32Matrix4D::TVector3( sub.tangents[ v*4 ], sub.tangents[ v*4+1 ], sub.tangents[ v*4+2 ] ) );
                    const Float32 length = sqrtf( d.x*d.x + d.y*d.y + d.z*d.z );
                    const Float32 scale = length > 1e-12f ? 1.0f / length : 1.0f;
                    target.tangents.push_back( d.x * scale ); target.tangents.push_back( d.y * scale ); target.tangents.push_back( d.z * scale );
                    target.tangents.push_back( mirrored ? -sub.tangents[ v*4+3 ] : sub.tangents[ v*4+3 ] );
                }
                if ( sub.colours.size() == (size_t) vertices * 4 )
                    target.colours.insert( target.colours.end(), sub.colours.begin() + (size_t) v * 4, sub.colours.begin() + (size_t) v * 4 + 4 );
            }
            for ( UInt32 u=0; u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
                if ( sub.uvSets[ u ].size() == (size_t) vertices * 2 )
                    target.uvSets[ u ].insert( target.uvSets[ u ].end(), sub.uvSets[ u ].begin(), sub.uvSets[ u ].end() );
            for ( size_t t=0; t+2<sub.indices.size(); t+=3 )
            {
                target.indices.push_back( sub.indices[ t ] + offset );
                target.indices.push_back( sub.indices[ mirrored ? t+2 : t+1 ] + offset );
                target.indices.push_back( sub.indices[ mirrored ? t+1 : t+2 ] + offset );
            }
        }
    }
    if ( merged.empty() )
        return false;

    // one mesh of the merged submeshes on a node of its own at the root; the nodes that were merged have no mesh any more
    ASSETCONVERTER::CAssetMesh all;
    all.name = "PreviewMerged";
    for ( TMergedMap::iterator m = merged.begin(); m != merged.end(); ++m )
        all.subMeshes.push_back( (*m).second );
    submeshesAfter = (UInt32) all.subMeshes.size();
    for ( size_t i=0; i<candidates.size(); ++i )
        scene.nodes[ candidates[ i ] ].mesh = -1;
    scene.meshes.push_back( all );
    const UInt32 node = scene.AddNode( "PreviewMerged", -1 );
    scene.nodes[ node ].mesh = (Int32) scene.meshes.size() - 1;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
