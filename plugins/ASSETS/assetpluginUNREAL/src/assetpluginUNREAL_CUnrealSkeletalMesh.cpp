/*
 *  assetpluginUNREAL: A skeletal mesh as read from a package: bounds, material slots, skeleton and the LOD models
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#include "assetpluginUNREAL_CUnrealSkeletalMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H ? */

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

CUnrealSkeletalMesh::CUnrealSkeletalMesh( void )
    : name()
    , boundsRadius( 0.0 )
    , materialSlots()
    , skeleton()
    , lods()
    , skeletonAsset()
    , morphTargetCount( 0 )
    , socketCount( 0 )
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CUnrealSkeletalMesh::Clear( void )
{GUCEF_TRACE;

    name = CORE::CString();
    for ( UInt32 i=0; i<3; ++i )
    {
        boundsOrigin[ i ] = 0.0;
        boundsExtent[ i ] = 0.0;
    }
    boundsRadius = 0.0;
    materialSlots.clear();
    skeleton.Clear();
    lods.clear();
    skeletonAsset = CUnrealPackageIndex();
    morphTargetCount = 0;
    socketCount = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealSkeletalMesh::GetVertexCount( UInt32 lod ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( lod < lods.size() )
        for ( size_t s=0; s<lods[ lod ].sections.size(); ++s )
            count += (UInt32) lods[ lod ].sections[ s ].vertices.size();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealSkeletalMesh::GetTriangleCount( UInt32 lod ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( lod < lods.size() )
        for ( size_t s=0; s<lods[ lod ].sections.size(); ++s )
            count += lods[ lod ].sections[ s ].triangleCount;
    return count;
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
