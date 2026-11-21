/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#include <algorithm>


#include "assetcookerOGRE_CBoneSubsetSplitter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The most bones that one triangle can use: three vertices of four influences */
static const UInt32 MinimumBoneLimit = 12;

struct STriangle
{
    UInt32 first;           /**< the index of its first corner in the indices */
    UInt32 sortKey;         /**< the joint its first vertex is weighted to the most */
};

bool
IsBefore( const STriangle& a, const STriangle& b )
{
    if ( a.sortKey != b.sortKey )
        return a.sortKey < b.sortKey;
    return a.first < b.first;
}

typedef GUCEF::set< UInt32 > TJointSet;

void
CollectJoints( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 vertex, TJointSet& joints )
{
    const UInt32 n = subMesh.influencesPerVertex;
    for ( UInt32 i=0; i<n; ++i )
    {
        const size_t at = (size_t) vertex * n + i;
        if ( at < subMesh.joints.size() && at < subMesh.weights.size() && subMesh.weights[ at ] > 0.0f )
            joints.insert( subMesh.joints[ at ] );
    }
}

UInt32
GetStrongestJoint( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 vertex )
{
    const UInt32 n = subMesh.influencesPerVertex;
    UInt32 joint = 0;
    Float32 weight = 0.0f;
    for ( UInt32 i=0; i<n; ++i )
    {
        const size_t at = (size_t) vertex * n + i;
        if ( at < subMesh.joints.size() && at < subMesh.weights.size() && ( subMesh.weights[ at ] > weight || ( subMesh.weights[ at ] == weight && weight > 0.0f && subMesh.joints[ at ] < joint ) ) )
        {
            weight = subMesh.weights[ at ];
            joint = subMesh.joints[ at ];
        }
    }
    return joint;
}

typedef GUCEF::vector< Float32 > TFloatVector;

void
CopyValues( const TFloatVector& from, UInt32 vertex, UInt32 stride, TFloatVector& to )
{
    if ( from.size() >= (size_t) ( vertex + 1 ) * stride )
        to.insert( to.end(), from.begin() + (size_t) vertex * stride, from.begin() + (size_t) ( vertex + 1 ) * stride );
}

/** The submesh of the triangles in the list: their vertices with all the streams of the source, in the order they are first used */
void
BuildPart( const ASSETCONVERTER::CAssetSubMesh& source, const GUCEF::vector< UInt32 >& triangleStarts, ASSETCONVERTER::CAssetSubMesh& part )
{
    const UInt32 vertexCount = source.GetVertexCount();
    GUCEF::vector< Int32 > remap( vertexCount, -1 );
    GUCEF::vector< UInt32 > used;
    part = ASSETCONVERTER::CAssetSubMesh();
    part.materialIndex = source.materialIndex;
    part.hiddenBy = source.hiddenBy;
    part.influencesPerVertex = source.influencesPerVertex;
    for ( size_t t=0; t<triangleStarts.size(); ++t )
    {
        for ( UInt32 c=0; c<3; ++c )
        {
            const UInt32 vertex = source.indices[ triangleStarts[ t ] + c ];
            if ( remap[ vertex ] < 0 )
            {
                remap[ vertex ] = (Int32) used.size();
                used.push_back( vertex );
            }
            part.indices.push_back( (UInt32) remap[ vertex ] );
        }
    }

    const UInt32 n = source.influencesPerVertex;
    part.morphTargets.resize( source.morphTargets.size() );
    for ( size_t m=0; m<source.morphTargets.size(); ++m )
    {
        part.morphTargets[ m ].name = source.morphTargets[ m ].name;
        part.morphTargets[ m ].defaultWeight = source.morphTargets[ m ].defaultWeight;
    }
    const UInt32 colourComponents = vertexCount > 0 ? (UInt32) ( source.colours.size() / vertexCount ) : 0;
    for ( size_t u=0; u<used.size(); ++u )
    {
        const UInt32 v = used[ u ];
        CopyValues( source.positions, v, 3, part.positions );
        CopyValues( source.normals, v, 3, part.normals );
        CopyValues( source.tangents, v, 4, part.tangents );
        for ( UInt32 s=0; s<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++s )
            CopyValues( source.uvSets[ s ], v, 2, part.uvSets[ s ] );
        CopyValues( source.colours, v, colourComponents, part.colours );
        if ( source.masks.size() == (size_t) vertexCount * 4 )
            CopyValues( source.masks, v, 4, part.masks );
        if ( n > 0 )
        {
            part.joints.insert( part.joints.end(), source.joints.begin() + (size_t) v * n, source.joints.begin() + (size_t) ( v + 1 ) * n );
            part.weights.insert( part.weights.end(), source.weights.begin() + (size_t) v * n, source.weights.begin() + (size_t) ( v + 1 ) * n );
        }
        for ( size_t m=0; m<source.morphTargets.size(); ++m )
        {
            CopyValues( source.morphTargets[ m ].positionDeltas, v, 3, part.morphTargets[ m ].positionDeltas );
            CopyValues( source.morphTargets[ m ].normalDeltas, v, 3, part.morphTargets[ m ].normalDeltas );
            CopyValues( source.morphTargets[ m ].tangentDeltas, v, 3, part.morphTargets[ m ].tangentDeltas );
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CBoneSubsetSplitter::SplitSubMesh( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 maxBones, ASSETCONVERTER::CAssetMesh::TSubMeshVector& parts )
{GUCEF_TRACE;

    if ( maxBones < MinimumBoneLimit )
        maxBones = MinimumBoneLimit;
    const UInt32 vertexCount = subMesh.GetVertexCount();
    if ( !subMesh.IsSkinned() || subMesh.GetUsedJointCount() <= maxBones || 0 == vertexCount || 0 != subMesh.indices.size() % 3 )
    {
        parts.push_back( subMesh );
        return;
    }
    for ( size_t i=0; i<subMesh.indices.size(); ++i )
    {
        if ( subMesh.indices[ i ] >= vertexCount )
        {
            parts.push_back( subMesh );
            return;
        }
    }

    // the triangles in the order of the bone their first vertex is mostly weighted to: the triangles of a limb are next to each other
    GUCEF::vector< STriangle > triangles;
    triangles.reserve( subMesh.indices.size() / 3 );
    for ( size_t i=0; i<subMesh.indices.size(); i+=3 )
    {
        STriangle triangle;
        triangle.first = (UInt32) i;
        triangle.sortKey = GetStrongestJoint( subMesh, subMesh.indices[ i ] );
        triangles.push_back( triangle );
    }
    std::stable_sort( triangles.begin(), triangles.end(), IsBefore );

    // the parts: a part takes triangles until the next one would bring it over the limit
    TJointSet inPart;
    GUCEF::vector< UInt32 > partTriangles;
    for ( size_t t=0; t<triangles.size(); ++t )
    {
        TJointSet needed;
        for ( UInt32 c=0; c<3; ++c )
            CollectJoints( subMesh, subMesh.indices[ triangles[ t ].first + c ], needed );
        UInt32 additional = 0;
        for ( TJointSet::const_iterator it = needed.begin(); it != needed.end(); ++it )
            if ( inPart.find( *it ) == inPart.end() )
                ++additional;
        if ( !partTriangles.empty() && inPart.size() + additional > maxBones )
        {
            parts.push_back( ASSETCONVERTER::CAssetSubMesh() );
            BuildPart( subMesh, partTriangles, parts.back() );
            partTriangles.clear();
            inPart.clear();
        }
        inPart.insert( needed.begin(), needed.end() );
        partTriangles.push_back( triangles[ t ].first );
    }
    if ( !partTriangles.empty() )
    {
        parts.push_back( ASSETCONVERTER::CAssetSubMesh() );
        BuildPart( subMesh, partTriangles, parts.back() );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CBoneSubsetSplitter::Split( const ASSETCONVERTER::CAssetMesh& mesh, UInt32 maxBones, ASSETCONVERTER::CAssetMesh& split, UInt32& extraSubMeshes )
{GUCEF_TRACE;

    split = mesh;
    split.subMeshes.clear();
    extraSubMeshes = 0;
    UInt32 splitCount = 0;
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
    {
        const size_t before = split.subMeshes.size();
        SplitSubMesh( mesh.subMeshes[ s ], maxBones, split.subMeshes );
        const UInt32 partCount = (UInt32) ( split.subMeshes.size() - before );
        if ( partCount > 1 )
        {
            ++splitCount;
            extraSubMeshes += partCount - 1;
        }
    }
    return splitCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
