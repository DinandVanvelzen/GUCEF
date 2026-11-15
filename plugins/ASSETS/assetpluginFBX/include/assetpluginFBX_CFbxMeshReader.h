/*
 *  assetpluginFBX: the meshes, skins and blend shapes of an FBX scene as meshes, skins and morph targets of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXMESHREADER_H
#define GUCEF_ASSETPLUGIN_FBX_CFBXMESHREADER_H

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

#ifndef GUCEF_ASSETPLUGIN_FBX_MACROS_H
#include "assetpluginFBX_macros.h"
#define GUCEF_ASSETPLUGIN_FBX_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_FBX_MACROS_H ? */

#include "ufbx.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Builds the model meshes of the mesh nodes of an FBX scene.
 *
 *  - One submesh per material of the mesh, triangulated, with the vertices the faces share kept shared: a vertex is
 *    a combination of position, normal, tangent, up to 4 UV sets and the first colour set. UV coordinates go from
 *    the V up convention of FBX to the V down convention of glTF.
 *  - A skin deformer becomes a skin: its clusters give the joints and their inverse bind matrices, and every
 *    influence of a vertex is kept (the influences per vertex of the submesh are the highest count of the mesh),
 *    ordered by weight, the weights summing to one.
 *  - A blend deformer becomes morph targets, one per channel, from the shape at the full weight of the channel;
 *    channels with in-between shapes are approximated by the last shape (reported).
 *  - A mesh that several nodes share is read once for the same materials.
 */
class GUCEF_ASSETPLUGIN_FBX_EXPORT_CPP CFbxMeshReader
{
    public:

    typedef GUCEF::vector< Int32 > TIndexVector;

    /** 'nodeMap' gives the index in the model of every node of the file (by typed id), -1 for the root node */
    CFbxMeshReader( const ufbx_scene& fbx                       ,
                    const TIndexVector& nodeMap                 ,
                    ASSETCONVERTER::CAssetScene& scene          ,
                    ASSETCONVERTER::CConversionReport& report   );

    ~CFbxMeshReader();

    /** The mesh and the skin of a node that has a mesh (-1 for none); false when the mesh cannot be used */
    bool ReadNodeMesh( const ufbx_node& node, Int32& meshIndex, Int32& skinIndex );

    /** The blend channels of a mesh in the order of its morph targets */
    static void GetBlendChannels( const ufbx_mesh& mesh, GUCEF::vector< const ufbx_blend_channel* >& channels );

    private:

    CFbxMeshReader( const CFbxMeshReader& src );                /**< not implemented */
    CFbxMeshReader& operator=( const CFbxMeshReader& src );     /**< not implemented */

    struct SCorner
    {
        UInt32 index[ 8 ];
        bool operator<( const SCorner& other ) const;
    };
    struct SPart
    {
        UInt32 slot;
        GUCEF::vector< UInt32 > faces;
    };
    typedef GUCEF::map< CORE::CString, Int32 > TMeshMap;
    typedef GUCEF::map< UInt32, Int32 > TSkinMap;
    typedef GUCEF::vector< GUCEF::vector< Float32 > > TDenseVector;

    Int32 ReadSkin( const ufbx_skin_deformer& deformer, GUCEF::vector< Int32 >& jointOfCluster );
    bool BuildSubMesh( const ufbx_mesh& mesh, const SPart& part, const ufbx_skin_deformer* deformer, const GUCEF::vector< Int32 >& jointOfCluster,
                       const TDenseVector& positionDeltas, const TDenseVector& normalDeltas, const GUCEF::vector< const ufbx_blend_channel* >& channels,
                       Int32 materialIndex, ASSETCONVERTER::CAssetSubMesh& subMesh );

    private:

    const ufbx_scene& m_fbx;
    const TIndexVector& m_nodeMap;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    TMeshMap m_meshes;
    TSkinMap m_skins;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXMESHREADER_H ? */
