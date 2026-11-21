/*
 *  assetpluginUNREAL: The textures of a MetaHuman character as images of the scene, and the record of the character as extras
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H

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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

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
 *  The textures that the character asset stores as plain PNG files become files below the intermediate folder of the pack
 *  ("embedded/<asset>/<character>_<group>_<type>.png", as the other importers do with embedded textures) and textures of the
 *  scene, whose usage follows the type (normal maps, masks and cavity maps are not colour). The ones that Oodle compresses
 *  are read through the oodle provider when one is given and its tool exists; otherwise they are listed as needing it, with the reason.
 *  The provider is the last parameter of AddToScene().
 *
 *  Which texture belongs on which material is only clear for the body: the base colour and normal map of the body (the layout
 *  of the body of the combined body mesh) go on the material of the slot "body_shader*". The face textures use the layout
 *  of the head mesh, which is not readable without Oodle; they stay unassigned images of the scene.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMetaHumanTextures
{
    public:

    /**
     *  @param intermediateFolder the folder of the VFS that receives the files; empty when the pack has none (nothing is extracted then)
     *  @param assetName the name below "embedded" (the package file without its extension)
     *  @param textureIndices receives per texture of the character the index in the scene, -1 for none
     *  @return the number of textures added to the scene
     */
    static UInt32 AddToScene( CUnrealPackage& package                       ,
                              const CUnrealMetaHumanCharacter& character    ,
                              const CORE::CString& intermediateFolder       ,
                              const CORE::CString& assetName                ,
                              const CORE::CString& characterName            ,
                              ASSETCONVERTER::CAssetScene& scene            ,
                              ASSETCONVERTER::CConversionReport& report     ,
                              GUCEF::vector< Int32 >& textureIndices        ,
                              CIUnrealOodleProvider* provider = GUCEF_NULL  );

    /**
     *  Puts the textures on the materials where the names make it clear (see above).
     *  @param unassigned receives the names of the textures of the scene that no material uses
     *  @return the number of material slots filled
     */
    static UInt32 AttachToMaterials( const CUnrealMetaHumanCharacter& character      ,
                                     const GUCEF::vector< Int32 >& textureIndices    ,
                                     ASSETCONVERTER::CAssetScene& scene              ,
                                     ASSETCONVERTER::CConversionReport& report       ,
                                     CORE::CString::StringVector& unassigned         );

    /**
     *  The combined body of a character lays out the head in UV tile 1001 (u from 0 to 1) and the rest of the body in tile 1002
     *  (u from 1 to 2); its one material slot takes the face textures for the first and the body textures for the second. When the
     *  submeshes of the body material use both tiles, the triangles of tile 1001 move to a new material (the body material with
     *  the given face textures, named <body material>_UDIM1001). The UVs of tile 1002 become those of its textures (u - 1), as a
     *  UDIM lookup does; the textures clamp at their edges. Triangles that cross tiles stay with the body material.
     *  @return the number of triangles that moved to the material of the head
     */
    static UInt32 SplitHeadTile( UInt32 bodyMaterial, Int32 faceBaseColour, Int32 faceNormal, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    /** The record of the character for the extras of the scene (key gucefCharacter): name, wardrobe, settings and the state of every texture */
    static void DescribeCharacter( const CUnrealMetaHumanCharacter& character      ,
                                   const CORE::CString& characterName              ,
                                   const GUCEF::vector< Int32 >& textureIndices    ,
                                   const ASSETCONVERTER::CAssetScene& scene        ,
                                   CORE::CDataNode& node                           );

    private:

    CUnrealMetaHumanTextures( void );    /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H ? */
