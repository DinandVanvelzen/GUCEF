/*
 *  assetpluginUNREAL: Assembles the body and the garments of a character into one skeleton and several skinned meshes
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H ? */

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
 *  A character is a body skeletal mesh and garment skeletal meshes (clothes, shoes) that each carry their own copy of the
 *  skeleton of the body. The assembly makes one skeleton from the body: its bones are nodes below one root node named after
 *  the character, and one skin lists them all. Every garment is skinned onto that skin by joint name: the bone of a garment
 *  gets the joint of the body bone with the same name. A garment bone that the body does not have becomes an extra joint
 *  below the joint of its parent, with the bind pose of the garment. What does not match is reported: a bone of the garment whose
 *  parent has another name than in the body, a bind pose that differs from the one of the body (the body wins: the garment
 *  is skinned relative to the bind pose of the shared skeleton).
 *
 *  Below the root node are the bones and one node per mesh (the body and each garment) that holds the levels of detail of the
 *  mesh, as the converter of a single skeletal mesh makes them.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealCharacterAssembler
{
    public:

    /** Translation (centimetres), rotation (quaternion dot product shortfall) and scale differences up to which two bind poses are the same */
    static const Float64 BindTranslationTolerance;
    static const Float64 BindRotationTolerance;
    static const Float64 BindScaleTolerance;

    /** A mesh to assemble and the scene materials of its material slots */
    struct SPart
    {
        const CUnrealSkeletalMesh* mesh;
        const GUCEF::vector< Int32 >* slotMaterials;

        SPart( void );
        SPart( const CUnrealSkeletalMesh* meshOfPart, const GUCEF::vector< Int32 >* materialsOfPart );
    };
    typedef GUCEF::vector< SPart > TPartVector;

    /** What the assembly made */
    struct SResult
    {
        UInt32 rootNode;                            /**< the node named after the character, the parent of everything else */
        UInt32 skinIndex;                           /**< the one skin of the character */
        GUCEF::vector< UInt32 > jointNodes;         /**< the node of each joint of the skin, in the order of the skin */
        UInt32 bodyHolderNode;                      /**< the node that holds the levels of the body */
        UInt32 bodyMeshFirst;                       /**< index in the scene of the first LOD mesh of the body */
        GUCEF::vector< UInt32 > garmentHolderNodes;
        GUCEF::vector< UInt32 > garmentMeshFirst;   /**< index in the scene of the first LOD mesh of each garment */
        UInt32 extraJoints;                         /**< joints the garments added to the skeleton of the body */
        UInt32 mismatches;                          /**< bones of garments whose parent or bind pose differs from the body */
        CORE::CString::StringVector mismatchTexts;  /**< the first of them as text, the report has them too */

        SResult( void );
    };

    /**
     *  Moves the vertices of a garment submesh (joints already those of the shared skin) from the bind pose of the garment's own
     *  skeleton into the shared bind pose of the body: every vertex gets the blend, with its weights, of SharedWorld * GarmentWorld^-1
     *  of its joints; normals and tangents turn with it. At the bind pose of the body the result is where the engine draws the
     *  garment when it follows the pose of the body with its own reference pose.
     *  @return the number of vertices that moved; 'largestMove' grows to the largest distance (centimetres of the source)
     */
    static UInt32 MoveIntoSharedBindPose( const CUnrealReferenceSkeleton& garmentSkeleton   ,
                                          const GUCEF::vector< UInt32 >& jointMap          ,
                                          const SResult& result                            ,
                                          const ASSETCONVERTER::CAssetScene& scene         ,
                                          ASSETCONVERTER::CAssetSubMesh& subMesh           ,
                                          Float32& largestMove                             );

    /**
     *  @param garments the parts that are skinned onto the skeleton of the body
     *  @return false when the body has no bones or no LODs; the scene is untouched then
     */
    static bool Assemble( const CORE::CString& characterName       ,
                          const SPart& body                         ,
                          const TPartVector& garments               ,
                          ASSETCONVERTER::CAssetScene& scene        ,
                          ASSETCONVERTER::CConversionReport& report ,
                          SResult& result                           );

    /**
     *  Compares the skeleton of a garment with the shared one by bone name.
     *  @param missing receives the number of bones of 'other' that 'shared' does not have
     *  @param texts receives a text per bone whose parent name or bind pose differs
     *  @return the number of bones that differ
     */
    static UInt32 CompareSkeletons( const CUnrealReferenceSkeleton& shared, const CUnrealReferenceSkeleton& other, UInt32& missing, CORE::CString::StringVector& texts );

    /** Whether a skeleton can carry the garment: the same root bone name and at least half of its bones by name in the shared skeleton */
    static bool IsCompatible( const CUnrealReferenceSkeleton& shared, const CUnrealReferenceSkeleton& other );

    private:

    CUnrealCharacterAssembler( void );    /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H ? */
