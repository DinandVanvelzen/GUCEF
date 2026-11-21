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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H
#include "assetpluginUNREAL_CUnrealReferenceSkeleton.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREFERENCESKELETON_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#include "assetpluginUNREAL_CUnrealPackageIndex.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H ? */

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
 *  The data of a skeletal mesh that the importer needs, in the units and axes of the engine: centimetres, Z up, left
 *  handed. The LOD models are the editor models of the package: per section the vertices with tangent basis, up to four
 *  UV sets, a colour and up to twelve bone influences, and the triangle indices of the whole LOD.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealSkeletalMesh
{
    public:

    static const UInt32 MaxUvSets = 4;
    static const UInt32 MaxInfluences = 12;

    /** One entry of the material slots: the material is an object reference of the package */
    struct SMaterialSlot
    {
        CUnrealPackageIndex material;
        CORE::CString slotName;
        CORE::CString importedSlotName;
        Float32 uvDensities[ 4 ];
    };
    typedef GUCEF::vector< SMaterialSlot > TMaterialSlotVector;

    /** A vertex of an editor LOD model. The weights are 16 bit: 65535 is a weight of one */
    struct SVertex
    {
        Float32 position[ 3 ];
        Float32 tangentX[ 3 ];
        Float32 tangentY[ 3 ];
        Float32 tangentZ[ 4 ];      /**< the normal, and in w the handedness of the tangent basis */
        Float32 uv[ MaxUvSets ][ 2 ];
        UInt8 colour[ 4 ];          /**< R, G, B, A */
        UInt16 bones[ MaxInfluences ];      /**< indices into the bone map of the section */
        UInt16 weights[ MaxInfluences ];
    };
    typedef GUCEF::vector< SVertex > TVertexVector;
    typedef GUCEF::vector< UInt16 > TUInt16Vector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    struct SSection
    {
        UInt32 materialIndex;       /**< index of the material slot */
        UInt32 baseIndex;           /**< first index of the section in the indices of the LOD */
        UInt32 triangleCount;
        UInt32 baseVertexIndex;     /**< the indices of the LOD count vertices from the start of the first section */
        UInt32 maxInfluences;
        bool castShadow;
        bool disabled;
        TVertexVector vertices;
        TUInt16Vector boneMap;      /**< bone of the reference skeleton per local bone index */
    };
    typedef GUCEF::vector< SSection > TSectionVector;

    struct SLod
    {
        TSectionVector sections;
        TUInt32Vector indices;
        TUInt16Vector activeBones;
        TUInt16Vector requiredBones;
        UInt32 uvSetCount;
        Float32 screenSize;         /**< the screen size from which this LOD is used, 0 when the mesh does not state it */
    };
    typedef GUCEF::vector< SLod > TLodVector;

    CUnrealSkeletalMesh( void );

    void Clear( void );

    CORE::CString name;
    Float64 boundsOrigin[ 3 ];
    Float64 boundsExtent[ 3 ];
    Float64 boundsRadius;
    TMaterialSlotVector materialSlots;
    CUnrealReferenceSkeleton skeleton;
    TLodVector lods;
    CUnrealPackageIndex skeletonAsset;      /**< the Skeleton object the mesh uses, null when the package does not say */
    UInt32 morphTargetCount;                /**< morph targets the mesh declares; they are not read */
    UInt32 socketCount;

    UInt32 GetVertexCount( UInt32 lod ) const;

    UInt32 GetTriangleCount( UInt32 lod ) const;
};

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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESH_H ? */
