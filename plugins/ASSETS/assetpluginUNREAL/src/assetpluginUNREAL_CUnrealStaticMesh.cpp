/*
 *  assetpluginUNREAL: A static mesh as the source states it
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#include "assetpluginUNREAL_CUnrealStaticMesh.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H ? */

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

CUnrealStaticMesh::SSection::SSection( void )
    : groupName()
    , slot( -1 )
    , positions()
    , normals()
    , tangents()
    , colours()
    , indices()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealStaticMesh::SLod::SLod( void )
    : sections()
    , screenSize( 0.0f )
    , uvSetCount( 0 )
    , sourceUvSetCount( 0 )
    , vertexCount( 0 )
    , triangleCount( 0 )
    , windingAgreement( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealStaticMesh::CUnrealStaticMesh( void )
    : name()
    , materialSlots()
    , lods()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        boundsOrigin[ i ] = 0.0;
        boundsExtent[ i ] = 0.0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealStaticMesh::GetVertexCount( UInt32 lod ) const
{GUCEF_TRACE;

    return lod < lods.size() ? lods[ lod ].vertexCount : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealStaticMesh::GetTriangleCount( UInt32 lod ) const
{GUCEF_TRACE;

    return lod < lods.size() ? lods[ lod ].triangleCount : 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMesh::GetPositionBounds( UInt32 lod, Float32* minimum, Float32* maximum ) const
{GUCEF_TRACE;

    if ( lod >= lods.size() )
        return false;
    bool any = false;
    for ( size_t s=0; s<lods[ lod ].sections.size(); ++s )
    {
        const GUCEF::vector< Float32 >& positions = lods[ lod ].sections[ s ].positions;
        for ( size_t v=0; v+2<positions.size(); v+=3 )
        {
            for ( UInt32 c=0; c<3; ++c )
            {
                if ( !any || positions[ v + c ] < minimum[ c ] ) minimum[ c ] = positions[ v + c ];
                if ( !any || positions[ v + c ] > maximum[ c ] ) maximum[ c ] = positions[ v + c ];
            }
            any = true;
        }
    }
    return any;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
