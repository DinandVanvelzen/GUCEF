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

#ifndef ASSETCONVERTERLIB_CASSETSUBMESH_H
#define ASSETCONVERTERLIB_CASSETSUBMESH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETMORPHTARGET_H
#include "AssetConverterLib_CAssetMorphTarget.h"
#define ASSETCONVERTERLIB_CASSETMORPHTARGET_H
#endif /* ASSETCONVERTERLIB_CASSETMORPHTARGET_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A submesh: one material, triangles, and the vertex streams of glTF. Streams are flat arrays of Float32 per vertex:
 *  positions, normals (3), tangents (4, w the handedness of the bitangent), up to four UV sets (2) and colours (4,
 *  linear RGBA). Skinning keeps every influence of the source: 'influencesPerVertex' joint indices (into the joints
 *  of the skin) and weights per vertex, also more than four. Indices are triangles, counter-clockwise front faces.
 *  An empty stream is not there.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetSubMesh
{
    public:

    static const UInt32 MaxUvSets = 4;

    typedef GUCEF::vector< Float32 > TFloatVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::vector< CAssetMorphTarget > TMorphTargetVector;

    TFloatVector positions;
    TFloatVector normals;
    TFloatVector tangents;
    TFloatVector uvSets[ MaxUvSets ];
    TFloatVector colours;
    TUInt32Vector joints;
    TFloatVector weights;
    UInt32 influencesPerVertex;
    TUInt32Vector indices;
    Int32 materialIndex;            /**< into the materials of the scene, -1 for none */
    TMorphTargetVector morphTargets;

    CAssetSubMesh( void );

    UInt32 GetVertexCount( void ) const;
    UInt32 GetTriangleCount( void ) const;
    UInt32 GetUvSetCount( void ) const;
    bool HasNormals( void ) const;
    bool HasTangents( void ) const;
    bool IsSkinned( void ) const;

    /** The number of different joints with a weight above 0 */
    UInt32 GetUsedJointCount( void ) const;

    /** Turns every triangle over (front faces become back faces): needed when a conversion mirrors */
    void ReverseWinding( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETSUBMESH_H ? */
