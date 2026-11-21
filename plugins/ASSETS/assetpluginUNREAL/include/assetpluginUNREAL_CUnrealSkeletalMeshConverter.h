/*
 *  assetpluginUNREAL: Puts a skeletal mesh into the intermediate scene: bone nodes, skin, one mesh per LOD
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

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
 *  Converts a skeletal mesh into nodes, a skin and meshes of the scene. Values stay in the units and axes of the engine
 *  (centimetres, Z up, left handed): the scene declares that system and the normaliser converts. The bones become nodes
 *  with their local bind pose, the skin lists them all in the order of the reference skeleton, so the joint of a vertex
 *  is the bone index. Every LOD is a mesh on its own node below one node of the mesh; with more than one LOD the nodes
 *  are the levels of a LOD group. The triangles of the engine have clockwise front faces in a left handed system; they
 *  are turned to counter-clockwise, which the intermediate model requires of its source.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealSkeletalMeshConverter
{
    public:

    /** The coordinate system of a skeletal mesh of the engine as the scene declares it: centimetres, Z up, left handed, the mesh faces +Y */
    static MATH::CCoordinateSystem GetCoordinateSystem( void );

    /**
     *  @param slotMaterials the index in the scene of the material of each material slot of the mesh, -1 for none
     *  @param rootNode receives the node that holds the mesh
     *  @return false when the mesh has nothing to convert
     */
    static bool AddToScene( const CUnrealSkeletalMesh& mesh                  ,
                            const GUCEF::vector< Int32 >& slotMaterials      ,
                            ASSETCONVERTER::CAssetScene& scene               ,
                            ASSETCONVERTER::CConversionReport& report        ,
                            UInt32& rootNode                                 );

    /**
     *  Adds the bones of a skeleton as nodes below parentNode (-1 for a root node) and a skin that lists them all in the order of the
     *  skeleton. The node of bone i is boneNodes[ i ], the skin is scene.skins[ skinIndex ].
     */
    static bool AddSkeleton( const CUnrealReferenceSkeleton& skeleton    ,
                             const CORE::CString& skinName               ,
                             Int32 parentNode                            ,
                             ASSETCONVERTER::CAssetScene& scene          ,
                             UInt32& skinIndex                           ,
                             GUCEF::vector< UInt32 >& boneNodes          );

    /** The local bind pose of a bone as the transform of a node */
    static void SetBindPose( ASSETCONVERTER::CAssetNode& node, const CUnrealReferenceSkeleton::SBone& bone );

    /**
     *  Adds a mesh per LOD of 'mesh' as children of holderNode, all skinned with scene.skins[ skinIndex ]; with more than one LOD the
     *  nodes are the levels of a LOD group on holderNode.
     *  @param jointMap the joint of the skin per bone of the reference skeleton of the mesh, NULL when the bone index is the joint
     */
    static void AddMeshNodes( const CUnrealSkeletalMesh& mesh                  ,
                              const GUCEF::vector< Int32 >& slotMaterials      ,
                              const GUCEF::vector< UInt32 >* jointMap          ,
                              Int32 skinIndex                                  ,
                              UInt32 holderNode                                ,
                              ASSETCONVERTER::CAssetScene& scene               ,
                              ASSETCONVERTER::CConversionReport& report        );

    private:

    CUnrealSkeletalMeshConverter( void );   /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHCONVERTER_H ? */
