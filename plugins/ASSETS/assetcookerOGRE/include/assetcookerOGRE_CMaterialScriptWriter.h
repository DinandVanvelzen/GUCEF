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

#ifndef GUCEF_ASSETCOOKER_OGRE_CMATERIALSCRIPTWRITER_H
#define GUCEF_ASSETCOOKER_OGRE_CMATERIALSCRIPTWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

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

/**
 *  Writes the materials of an asset as one Ogre material script for the run time shader system (RTSS) of Ogre 14.6.3.
 *  The syntax follows the RTSS script translators of 14.6.3 (OgreShaderCookTorranceLighting.cpp,
 *  OgreShaderExNormalMapLighting.cpp, OgreShaderFFPTexturing.cpp) and its RTSS manual (rtss.md): every material has
 *  one technique with one pass:
 *
 *      material "<name>"
 *      {
 *          technique
 *          {
 *              pass
 *              {
 *                  diffuse <base colour factor, alpha included>
 *                  specular <roughness> <metallic> 0 1 0        roughness and metalness of metal_roughness without texture
 *                  emissive <emissive factor * strength>        when there is no emissive texture
 *                  scene_blend alpha_blend + depth_write off    alpha mode BLEND
 *                  alpha_rejection greater_equal <cutoff * 255> alpha mode MASK
 *                  cull_hardware none                           double sided
 *                  lighting off                                 KHR_materials_unlit
 *                  texture_unit BaseColour { texture "<name>" gamma ... }
 *                  texture_unit NormalMap { texture "<name>" ... rtshader_system { normal_map tangent_space } }
 *                  texture_unit Emissive { texture "<name>" gamma ... colour_op add }
 *                  rtshader_system
 *                  {
 *                      lighting_stage metal_roughness [texture "<name>" [occlusion]]
 *                      texturing_stage late_add_blend           with an emissive texture: added after the lighting
 *                  }
 *              }
 *          }
 *      }
 *
 *  A texture unit carries tex_coord_set, tex_address_mode (glTF sampler wrap modes) and, for KHR_texture_transform,
 *  scale and scroll (the scale of Ogre works around the centre of the texture, which the scroll compensates); a
 *  rotation is reported. The metal-roughness stage reads roughness from G, metalness from B and, with "occlusion",
 *  ambient occlusion from R, and ignores the factors when it has a texture: a separate occlusion texture or factors
 *  other than 1 are therefore baked into a derived texture (CTextureCooker::CookDerived) that packs occlusion, roughness
 *  and metalness with the factors folded in. An emissive texture with a factor other than 1 is derived the same way
 *  (the factor applied to the sRGB values with an exponent of 1/2.2; values above 1 clamp, which is reported).
 *  Extensions without a counterpart in the RTSS (clearcoat, specular, sheen, transmission, ...) are reported.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CMaterialScriptWriter
{
    public:

    CMaterialScriptWriter( const ASSETCONVERTER::CAssetScene& scene      ,
                           CTextureCooker& textures                       ,
                           const ASSETCONVERTER::CCookProfile& profile    ,
                           ASSETCONVERTER::CConversionReport& report      );

    /** Appends the script of a material under the name */
    void AddMaterial( const CORE::CString& materialName, const ASSETCONVERTER::CAssetMaterial& material );

    /**
     *  Adds the material a particle system draws with (design section 5.7.4): unlit, the particle colour as the vertex
     *  colour (when the system uses it) times the base colour texture, decoded as sRGB like every colour texture of the
     *  cooked materials (Ogre refuses one texture with and without the gamma flag; the particle colours are written
     *  linear to match), no depth writes for blended particles, both sides, and the blend mode of the system. Sprite
     *  sheets clamp their texture.
     */
    void AddParticleMaterial( const CORE::CString& materialName, const ASSETCONVERTER::CAssetParticleSystem& system );

    UInt32 GetMaterialCount( void ) const;

    const CORE::CString& GetText( void ) const;

    /** The VFS paths of the textures the script references */
    const CORE::CString::StringVector& GetReferences( void ) const;

    /** A quoted string of the script language */
    static CORE::CString Quote( const CORE::CString& text );

    static CORE::CString FormatNumber( Float32 value );

    private:

    bool CookTexture( const ASSETCONVERTER::CAssetTextureReference& reference, CORE::CString& name );

    bool CookDerived( const ASSETCONVERTER::CAssetTexture& derived, CORE::CString& name );

    void WriteTextureUnit( const CORE::CString& unitName                              ,
                           const CORE::CString& textureName                           ,
                           const ASSETCONVERTER::CAssetTextureReference& reference    ,
                           bool sRgb                                                  ,
                           const CORE::CString& extraLines                            ,
                           const CORE::CString& materialName                          );

    CORE::CString GetMetalRoughness( const ASSETCONVERTER::CAssetMaterial& material, bool& occlusion );

    CORE::CString GetEmissive( const ASSETCONVERTER::CAssetMaterial& material );

    void AddReference( const CORE::CString& textureName );

    void Line( UInt32 depth, const CORE::CString& text );

    CMaterialScriptWriter( const CMaterialScriptWriter& src );              /**< not implemented */
    CMaterialScriptWriter& operator=( const CMaterialScriptWriter& src );   /**< not implemented */

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    CTextureCooker& m_textures;
    ASSETCONVERTER::CCookProfile m_profile;
    ASSETCONVERTER::CConversionReport& m_report;
    CORE::CString m_text;
    CORE::CString::StringVector m_references;
    UInt32 m_materialCount;
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CMATERIALSCRIPTWRITER_H ? */
