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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A static mesh read from an editor package: its material slots, bounds and levels of detail, each a list of sections (one per
 *  polygon group, with the material slot of the group) with the vertex streams in the source system (centimetres, Z up, left
 *  handed) and the triangles in the order the source stores them. A vertex is a vertex instance of the mesh description: position,
 *  normal, tangent and colour, up to four UV sets.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealStaticMesh
{
    public:

    struct SSection
    {
        CORE::CString groupName;                    /**< the imported material slot name of the polygon group */
        Int32 slot;                                 /**< index in the material slots, -1 when the group names none */
        GUCEF::vector< Float32 > positions;         /**< 3 per vertex */
        GUCEF::vector< Float32 > normals;           /**< 3 per vertex */
        GUCEF::vector< Float32 > tangents;          /**< 4 per vertex, w the sign of the bitangent */
        GUCEF::vector< Float32 > colours;           /**< 4 per vertex, empty when all are white */
        GUCEF::vector< Float32 > uvSets[ 4 ];       /**< 2 per vertex */
        GUCEF::vector< UInt32 > indices;            /**< triangles, in the order of the source */

        SSection( void );
    };

    struct SLod
    {
        GUCEF::vector< SSection > sections;
        Float32 screenSize;                         /**< 0 when the mesh does not state it */
        UInt32 uvSetCount;
        UInt32 sourceUvSetCount;                    /**< the number of UV sets the source has, more than uvSetCount when sets were left out */
        UInt32 vertexCount;
        UInt32 triangleCount;
        Float32 windingAgreement;                   /**< the share of triangles whose geometric normal ( b - a ) x ( c - a ) points the way of the vertex normals */

        SLod( void );
    };

    CORE::CString name;
    CUnrealSkeletalMesh::TMaterialSlotVector materialSlots;
    GUCEF::vector< SLod > lods;
    Float64 boundsOrigin[ 3 ];
    Float64 boundsExtent[ 3 ];

    CUnrealStaticMesh( void );

    UInt32 GetVertexCount( UInt32 lod ) const;
    UInt32 GetTriangleCount( UInt32 lod ) const;

    /** The bounds of the positions of a level of detail: minimum then maximum */
    bool GetPositionBounds( UInt32 lod, Float32* minimum, Float32* maximum ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESH_H ? */
