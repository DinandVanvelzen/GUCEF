/*
 *  AssetConverterLib: a submesh of the intermediate model: vertex streams, indices and morph targets
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

#include "AssetConverterLib_CAssetSubMesh.h"

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

CAssetSubMesh::CAssetSubMesh( void )
    : positions()
    , normals()
    , tangents()
    , colours()
    , masks()
    , joints()
    , weights()
    , influencesPerVertex( 0 )
    , indices()
    , materialIndex( -1 )
    , morphTargets()
    , hiddenBy( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSubMesh::GetVertexCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( positions.size() / 3 );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSubMesh::GetTriangleCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( indices.size() / 3 );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSubMesh::GetUvSetCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<MaxUvSets; ++i )
        if ( !uvSets[ i ].empty() )
            count = i+1;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSubMesh::HasNormals( void ) const
{GUCEF_TRACE;

    return !normals.empty() && normals.size() == positions.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSubMesh::HasTangents( void ) const
{GUCEF_TRACE;

    return !tangents.empty() && tangents.size() / 4 == positions.size() / 3;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSubMesh::IsSkinned( void ) const
{GUCEF_TRACE;

    return influencesPerVertex > 0 && !joints.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSubMesh::GetUsedJointCount( void ) const
{GUCEF_TRACE;

    GUCEF::set< UInt32 > used;
    for ( size_t i=0; i<joints.size() && i<weights.size(); ++i )
        if ( weights[ i ] > 0.0f )
            used.insert( joints[ i ] );
    return (UInt32) used.size();
}

/*-------------------------------------------------------------------------*/

void
CAssetSubMesh::ReverseWinding( void )
{GUCEF_TRACE;

    for ( size_t i=0; i+2<indices.size(); i+=3 )
    {
        const UInt32 swap = indices[ i+1 ];
        indices[ i+1 ] = indices[ i+2 ];
        indices[ i+2 ] = swap;
    }
}

/** Appends the values of one vertex of a stream with 'width' values per vertex, when the stream is there */
static void
CopyVertex( const CAssetSubMesh::TFloatVector& source, CAssetSubMesh::TFloatVector& target, UInt32 vertex, UInt32 width, UInt32 vertexCount )
{GUCEF_TRACE;

    if ( 0 == width || source.size() != (size_t) vertexCount * width )
        return;
    for ( UInt32 c=0; c<width; ++c )
        target.push_back( source[ (size_t) vertex * width + c ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetSubMesh::ExtractTriangles( const TUInt32Vector& triangles, CAssetSubMesh& out ) const
{GUCEF_TRACE;

    out = CAssetSubMesh();
    out.materialIndex = materialIndex;
    out.hiddenBy = hiddenBy;
    out.influencesPerVertex = influencesPerVertex;
    const UInt32 vertexCount = GetVertexCount();
    const UInt32 colourWidth = 0 == vertexCount ? 0 : (UInt32) ( colours.size() / vertexCount );
    const UInt32 influences = influencesPerVertex;
    for ( size_t t=0; t<morphTargets.size(); ++t )
    {
        CAssetMorphTarget target;
        target.name = morphTargets[ t ].name;
        target.defaultWeight = morphTargets[ t ].defaultWeight;
        out.morphTargets.push_back( target );
    }

    GUCEF::map< UInt32, UInt32 > newIndex;
    for ( size_t i=0; i<triangles.size(); ++i )
    {
        const size_t first = (size_t) triangles[ i ] * 3;
        if ( first + 2 >= indices.size() )
            continue;
        for ( UInt32 c=0; c<3; ++c )
        {
            const UInt32 vertex = indices[ first + c ];
            GUCEF::map< UInt32, UInt32 >::iterator found = newIndex.find( vertex );
            if ( found != newIndex.end() )
            {
                out.indices.push_back( (*found).second );
                continue;
            }
            const UInt32 index = out.GetVertexCount();
            newIndex[ vertex ] = index;
            out.indices.push_back( index );
            CopyVertex( positions, out.positions, vertex, 3, vertexCount );
            CopyVertex( normals, out.normals, vertex, 3, vertexCount );
            CopyVertex( tangents, out.tangents, vertex, 4, vertexCount );
            for ( UInt32 u=0; u<MaxUvSets; ++u )
                CopyVertex( uvSets[ u ], out.uvSets[ u ], vertex, 2, vertexCount );
            CopyVertex( colours, out.colours, vertex, colourWidth, vertexCount );
            CopyVertex( masks, out.masks, vertex, 4, vertexCount );
            if ( influences > 0 && joints.size() == (size_t) vertexCount * influences && weights.size() == (size_t) vertexCount * influences )
            {
                for ( UInt32 k=0; k<influences; ++k )
                {
                    out.joints.push_back( joints[ (size_t) vertex * influences + k ] );
                    out.weights.push_back( weights[ (size_t) vertex * influences + k ] );
                }
            }
            for ( size_t t=0; t<morphTargets.size(); ++t )
            {
                CopyVertex( morphTargets[ t ].positionDeltas, out.morphTargets[ t ].positionDeltas, vertex, 3, vertexCount );
                CopyVertex( morphTargets[ t ].normalDeltas, out.morphTargets[ t ].normalDeltas, vertex, 3, vertexCount );
                CopyVertex( morphTargets[ t ].tangentDeltas, out.morphTargets[ t ].tangentDeltas, vertex, 3, vertexCount );
            }
        }
    }
    if ( out.joints.empty() )
        out.influencesPerVertex = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
