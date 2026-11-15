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
    , joints()
    , weights()
    , influencesPerVertex( 0 )
    , indices()
    , materialIndex( -1 )
    , morphTargets()
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
