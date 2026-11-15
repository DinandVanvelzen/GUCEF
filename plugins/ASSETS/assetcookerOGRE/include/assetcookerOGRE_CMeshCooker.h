/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_CMESHCOOKER_H
#define GUCEF_ASSETCOOKER_OGRE_CMESHCOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreMesh.h"
#include "OgreSkeleton.h"
#include "OgreMaterial.h"

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Builds an Ogre v1 mesh from a mesh of the intermediate and writes it as .mesh (MeshSerializer, the newest version
 *  of 14.6.3, little endian).
 *
 *  - Submeshes per material: submeshes of the intermediate with the same material and the same vertex streams are
 *    merged into one; each submesh has vertex data of its own, laid out by Ogre's auto-organised declaration for its
 *    animation (skeletal, poses with or without normals), and 16 bit indices up to 65535 vertices, else 32 bit.
 *  - Streams: position, normal, tangent (four components, the handedness in w), vertex colour (8 bits per channel) and
 *    up to four UV sets. Where the material of a submesh uses a normal map and the submesh has no tangents, tangents are
 *    generated per triangle from the normal map's UV set (reported: not MikkTSpace).
 *  - Skinning: at most four weights per vertex. A vertex with more influences keeps the four strongest, renormalised;
 *    the number of such vertices and the most influences seen are reported as an approximation. The mesh links to the
 *    skeleton given; the bone of a weight is the index of its joint in the skin.
 *  - Morph targets become poses (one per target and submesh, offsets and, where the target has them, normal offsets)
 *    and every clip that animates the weights of a node that uses the mesh becomes a pose animation of the length of
 *    the clip, a key per key of the weights. Default weights other than 0 are reported: Ogre has no base weights.
 *  - Bounds: the box and sphere of the positions, with the morph offsets added.
 *
 *  Submeshes get their material by name: a material of that name that does not exist yet is made empty in the
 *  resource group of the cooker for the time of the cooking, as Ogre needs one to name it.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CMeshCooker
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    CMeshCooker( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    ~CMeshCooker();

    /**
     *  Builds the mesh under the resource name in the resource group of the cooker.
     *
     *  @param materialNames the Ogre material name of every material of the scene, by index
     *  @param defaultMaterialName the material of submeshes without one
     *  @param skinIndex the skin of the mesh, -1 for none
     *  @param skeleton the skeleton of that skin, linked to the mesh
     *  @param animatedNodes the nodes that use the mesh, whose weight channels become its pose animations
     */
    bool Build( UInt32 meshIndex                                ,
                const CORE::CString& resourceName               ,
                const CORE::CString::StringVector& materialNames,
                const CORE::CString& defaultMaterialName        ,
                Int32 skinIndex                                 ,
                const Ogre::SkeletonPtr& skeleton               ,
                const TUInt32Vector& animatedNodes              );

    Ogre::MeshPtr& GetMesh( void );

    bool Save( const CORE::CString& vfsPath ) const;

    /** Takes the mesh and the materials made for it out of their managers */
    void Destroy( void );

    /** Merges the submeshes of the mesh that have the same material and the same streams; the number merged away */
    static UInt32 MergeSubMeshesByMaterial( const ASSETCONVERTER::CAssetMesh& mesh, ASSETCONVERTER::CAssetMesh& merged );

    /** Tangents (x, y, z, handedness) per vertex from positions, normals and a UV set; false without them */
    static bool GenerateTangents( ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 uvSet );

    private:

    bool BuildSubMesh( const ASSETCONVERTER::CAssetSubMesh& source, UInt32 index, const CORE::CString& materialName, bool skinned, Ogre::AxisAlignedBox& bounds );

    void BuildPoses( const ASSETCONVERTER::CAssetMesh& mesh, const TUInt32Vector& animatedNodes );

    void UseMaterial( Ogre::SubMesh* subMesh, const CORE::CString& materialName );

    CMeshCooker( const CMeshCooker& src );              /**< not implemented */
    CMeshCooker& operator=( const CMeshCooker& src );   /**< not implemented */

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    Ogre::MeshPtr m_mesh;
    GUCEF::vector< Ogre::MaterialPtr > m_madeMaterials;
    CORE::CString m_name;
    UInt32 m_reducedVertices;
    UInt32 m_mostInfluences;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CMESHCOOKER_H ? */
