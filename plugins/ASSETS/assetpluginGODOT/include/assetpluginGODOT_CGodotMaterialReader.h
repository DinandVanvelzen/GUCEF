/*
 *  assetpluginGODOT: the materials of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H

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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#include "assetpluginGODOT_CGodotDocument.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the materials of a Godot project into the scene: StandardMaterial3D and ORMMaterial3D (the properties of
 *  BaseMaterial3D) as PBR metal-roughness materials of glTF, and ShaderMaterial as far as its parameters name a base
 *  colour, a base colour texture, a normal texture, a metallic and a roughness. Every property of the material goes to
 *  its CSourceMaterial, with the texture files by property name.
 *
 *  The mapping, per property of BaseMaterial3D:
 *  - albedo_color (sRGB in Godot, linear in glTF) and albedo_texture: the base colour;
 *  - metallic and roughness with their textures and channels: the factors, and the textures repacked into G
 *    (roughness) and B (metallic) of the glTF texture, or one texture used as it is when it has that layout already;
 *  - orm_texture (R occlusion, G roughness, B metallic): the metallic-roughness and the occlusion texture without
 *    repacking;
 *  - normal_enabled, normal_texture, normal_scale: the normal texture (both conventions are OpenGL);
 *  - emission_enabled, emission (linear from sRGB, an excess over 1 goes to the strength),
 *    emission_energy_multiplier (the strength) and emission_texture;
 *  - ao_enabled, ao_texture, ao_texture_channel: the occlusion texture;
 *  - transparency: 0 OPAQUE, 1 and 4 BLEND, 2 MASK with alpha_scissor_threshold, 3 (alpha hash) MASK at 0.5 as an
 *    approximation that is reported;
 *  - cull_mode 2: double sided (front face culling is reported);
 *  - shading_mode 0 (unshaded): KHR_materials_unlit;
 *  - uv1_scale and uv1_offset: KHR_texture_transform on the textures that use the first UV set;
 *  - texture_repeat false: clamped textures;
 *  - clearcoat_enabled: KHR_materials_clearcoat;
 *  - detail_*, heightmap_*, rim, subsurface, refraction, anisotropy and the like: the sidecar only.
 *
 *  A reader keeps its materials and textures for the whole read of a scene: the same material resource, and the same
 *  file with the same use, is added once.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotMaterialReader
{
    public:

    CGodotMaterialReader( ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    ~CGodotMaterialReader();

    /** Whether the resource type is a material this reader handles */
    static bool IsMaterialType( const CORE::CString& type );

    /**
     *  Adds the material of a [sub_resource] or [resource] section of the document to the scene (once per section); its
     *  index, or -1 when the section is no material.
     */
    Int32 ReadMaterial( const CGodotDocument& document, const CORE::CDataNode& section );

    /** The path in the pack of the file a property value refers to (an ExtResource); false when it refers to none or the file is not in the pack */
    /** The path in the pack of an ExtResource value; 'via' (when given) receives the reference as the file wrote it (the res:// path, else the uid) */
    bool ResolveFile( const CGodotDocument& document, const CORE::CDataNode* value, CORE::CString& assetPath, CORE::CString* via = GUCEF_NULL );

    private:

    CGodotMaterialReader( const CGodotMaterialReader& src );            /**< not implemented */
    CGodotMaterialReader& operator=( const CGodotMaterialReader& src ); /**< not implemented */

    typedef GUCEF::map< CORE::CString, Int32 > TIndexMap;

    const CORE::CDataNode* Prop( const CORE::CString& key ) const;
    Float64 Number( const CORE::CString& key, Float64 fallback ) const;
    bool Flag( const CORE::CString& key, bool fallback ) const;
    Int32 Choice( const CORE::CString& key, Int32 fallback ) const;
    CORE::CString TextureFile( const CORE::CString& key );
    Int32 AddTexture( const CORE::CString& vfsPath, ASSETCONVERTER::TTextureUsage usage );
    Int32 AddDerivedTexture( const CORE::CString::StringVector& sources, const GUCEF::vector< ASSETCONVERTER::CTextureOperation >& operations, const CORE::CString& name );
    void ApplyUvTransform( ASSETCONVERTER::CAssetTextureReference& reference ) const;
    void RecordSource( ASSETCONVERTER::CAssetMaterial& material );
    void ReadBase( ASSETCONVERTER::CAssetMaterial& material, bool isOrm );
    void ReadMetallicRoughness( ASSETCONVERTER::CAssetMaterial& material );
    void ReadEmission( ASSETCONVERTER::CAssetMaterial& material );
    void ReadShader( ASSETCONVERTER::CAssetMaterial& material );

    private:

    ASSETCONVERTER::CAssetPackContext& m_context;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    TIndexMap m_materials;
    TIndexMap m_textures;

    // the material being read
    const CGodotDocument* m_document;
    const CORE::CDataNode* m_section;
    CORE::CString m_subject;
    IMAGE::TWrapMode m_wrap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTMATERIALREADER_H ? */
