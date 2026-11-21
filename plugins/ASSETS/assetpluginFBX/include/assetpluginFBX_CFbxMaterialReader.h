/*
 *  assetpluginFBX: the materials and textures of an FBX scene as materials and textures of the converter model
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXMATERIALREADER_H
#define GUCEF_ASSETPLUGIN_FBX_CFBXMATERIALREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

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
 *  Maps the materials of an FBX scene to the resolved PBR values and textures of the model, in the order of the
 *  file (the model index of a material is its typed id). The original properties and textures of every material go
 *  to its source material, so the sidecar has them.
 *
 *  - Materials that ufbx recognises as PBR (Stingray and 3ds Max physical and PBR materials, Arnold, glTF and other
 *    PBR shaders) give base colour, metalness, roughness (glossiness is inverted), normal map, ambient occlusion,
 *    emission and opacity; separate roughness and metalness textures are packed to the channels of glTF.
 *  - FBX standard materials (Lambert, Phong) give the diffuse colour, the emission, the transparency and the normal
 *    map; the roughness is approximated from the specular exponent. A bump map (a height map) is not converted.
 *  - A texture is a file next to the FBX or in the pack, or content inside the FBX, which is written below the
 *    intermediate folder ("embedded/<asset>/"). Where a file is not found the slot stays empty and the report says so.
 */
class GUCEF_ASSETPLUGIN_FBX_EXPORT_CPP CFbxMaterialReader
{
    public:

    CFbxMaterialReader( const ufbx_scene& fbx                       ,
                        const CORE::CString& fbxPath                ,
                        ASSETCONVERTER::CAssetPackContext& context  ,
                        ASSETCONVERTER::CAssetScene& scene          ,
                        ASSETCONVERTER::CConversionReport& report   );

    ~CFbxMaterialReader();

    /** Adds a material to the scene for every material of the file */
    void ReadMaterials( void );

    private:

    CFbxMaterialReader( const CFbxMaterialReader& src );                /**< not implemented */
    CFbxMaterialReader& operator=( const CFbxMaterialReader& src );     /**< not implemented */

    typedef GUCEF::map< CORE::CString, Int32 > TTextureMap;

    void ReadMaterial( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material );
    void ReadPbr( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material );
    void ReadStandard( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material );
    void ReadSourceMaterial( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material );
    CORE::CString ResolveFile( const ufbx_texture& texture );
    CORE::CString ExtractContent( const ufbx_texture& texture );
    const ufbx_texture* GetFileTexture( const ufbx_texture* texture ) const;

    /** The file name of a file texture as the FBX file wrote it (relative where it has one): the 'via' of its relation */
    static CORE::CString ReferenceText( const ufbx_texture& texture );
    bool SetReference( const ufbx_material_map& map, ASSETCONVERTER::TTextureUsage usage, ASSETCONVERTER::CAssetTextureReference& reference, const CORE::CString& owner );
    Int32 AddFileTexture( const ufbx_texture& texture, ASSETCONVERTER::TTextureUsage usage, const CORE::CString& owner );
    Int32 AddDerivedTexture( const CORE::CString::StringVector& sources, const GUCEF::vector< ASSETCONVERTER::CTextureOperation >& operations, const CORE::CString& name );
    void AddPackedMetalRoughness( const ufbx_material_map& roughness, const ufbx_material_map& metalness, bool roughnessIsGlossiness, ASSETCONVERTER::CAssetMaterial& material );
    CORE::CString GetTextureFile( const ufbx_material_map& map );

    private:

    const ufbx_scene& m_fbx;
    CORE::CString m_fbxPath;
    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    TTextureMap m_textures;
    CORE::CString::StringSet m_embedded;
    CORE::CString m_materialName;
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

#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXMATERIALREADER_H ? */
