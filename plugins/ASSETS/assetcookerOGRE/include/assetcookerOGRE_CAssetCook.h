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

#ifndef GUCEF_ASSETCOOKER_OGRE_CASSETCOOK_H
#define GUCEF_ASSETCOOKER_OGRE_CASSETCOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#include "AssetConverterLib_CCookSettings.h"
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H
#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H
#include "assetcookerOGRE_COutputNames.h"
#define GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H
#endif /* GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H
#include "assetcookerOGRE_CTextureCooker.h"
#define GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H
#endif /* GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H ? */

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

class CSkeletonCooker;

/*-------------------------------------------------------------------------*/

/**
 *  Cooks one asset of the intermediate, read back into the model, into the files of one cook profile (names as
 *  COutputNames gives them), and reports every file with the files it references:
 *
 *    - one material script with every material of the asset (and a default material for submeshes without one);
 *    - a skeleton per skin, with the clips that animate its joints;
 *    - a mesh per mesh and skin that a node shows, with poses and pose animations, linked to its skeleton and material
 *      script; the other levels of a LOD group become meshes of their own and manual levels of the first, or (policy
 *      "generate") MeshLodGenerator makes the levels of meshes without source levels; with policy "none" only the
 *      first level is cooked;
 *    - the DotScene of the asset (every root node), and one for every prefab of the sidecar (a prefab that is the whole
 *      asset only gives its overrides to the scene of the asset);
 *    - <asset>.collision.json with the colliders, unless the collider policy is "none".
 *
 *  Triangles of a body that hidden face sets hide (the outfits of a character, CAssetHiddenFaceSet) stay in the mesh, in
 *  submeshes named by CMeshCooker::GetHiddenFaceSubMeshName() with a material variant per material and sets. The variant draws
 *  nothing while one of its sets is worn (active in the intermediate, or named by the pack setting "hiddenFaceSets") and
 *  is a copy of the material otherwise; the entity of the mesh in the DotScene names the sets in its user data
 *  ("gucefHiddenFaceSets", "gucefActiveHiddenFaceSets"). Stock Ogre shows the body under the worn outfit hidden; an
 *  application that switches outfits uses SubEntity::setVisible() and the base material.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CAssetCook
{
    public:

    CAssetCook( const ASSETCONVERTER::CCookSettings& settings       ,
                const ASSETCONVERTER::CCookSettings::SAsset& asset  ,
                const CORE::CString& profileDir                     ,
                CTextureCooker& textures                            ,
                ASSETCONVERTER::CConversionReport& report           );

    ~CAssetCook();

    /** Cooks the scene of the asset; the sidecar gives the prefabs. False when a file could not be made */
    bool Cook( const ASSETCONVERTER::CAssetScene& scene, const CORE::CDataNode& sidecar );

    private:

    struct SCookedMesh
    {
        UInt32 mesh;
        Int32 skin;
        CORE::CString name;
    };
    typedef GUCEF::vector< SCookedMesh > TCookedMeshVector;
    typedef std::pair< Int32, UInt32 > TMaterialVariant;              /**< a material (-1: the default) and the hidden face sets of its triangles */
    typedef GUCEF::vector< TMaterialVariant > TMaterialVariantVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    CORE::CString ToVfsPath( const CORE::CString& name ) const;

    void AddFile( const CORE::CString& name, const CORE::CString::StringVector& referenceNames );

    bool WriteText( const CORE::CString& name, const CORE::CString& text );

    /** The cooked mesh of a mesh and skin, cooked when it is not cooked yet; empty when it fails */
    CORE::CString CookMesh( UInt32 mesh, Int32 skin, const TUInt32Vector& manualLevelNodes, const GUCEF::vector< Float32 >& manualLevelHeights );

    void ReadPrefabs( const CORE::CDataNode& sidecar, GUCEF::vector< ASSETCONVERTER::CAssetPrefab >& prefabs ) const;

    CAssetCook( const CAssetCook& src );              /**< not implemented */
    CAssetCook& operator=( const CAssetCook& src );   /**< not implemented */

    private:

    const ASSETCONVERTER::CCookSettings& m_settings;
    const ASSETCONVERTER::CCookSettings::SAsset& m_asset;
    CORE::CString m_profileDir;
    CTextureCooker& m_textures;
    ASSETCONVERTER::CConversionReport& m_report;
    COutputNames m_names;
    const ASSETCONVERTER::CAssetScene* m_scene;
    CORE::CString::StringVector m_materialNames;
    CORE::CString m_defaultMaterialName;
    CORE::CString m_materialScript;
    GUCEF::vector< CSkeletonCooker* > m_skeletons;
    CORE::CString::StringVector m_skeletonNames;
    TCookedMeshVector m_meshes;
    bool m_ok;
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CASSETCOOK_H ? */
