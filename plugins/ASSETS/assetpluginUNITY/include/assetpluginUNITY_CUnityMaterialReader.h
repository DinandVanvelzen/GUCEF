/*
 *  assetpluginUNITY: the reader of Unity materials
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMATERIAL_H
#include "AssetConverterLib_CAssetMaterial.h"
#define ASSETCONVERTERLIB_CASSETMATERIAL_H
#endif /* ASSETCONVERTERLIB_CASSETMATERIAL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H
#include "assetpluginUNITY_CUnityBuiltins.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#include "assetpluginUNITY_CUnityProject.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

class CUnityModel;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads Unity materials (.mat, Material class 21) into materials of the scene of the project, with their textures:
 *
 *  - the shader is identified by m_Shader only (materials keep the properties of every shader they had): the
 *    built-in ids of CUnityBuiltins, the URP Lit shader by its GUID, a .shader or .shadergraph of the package by its
 *    GUID; a shader outside the package is "missing", and with the HDRP Lit property set (_BaseColorMap, _MaskMap) it
 *    is taken as HDRP Lit (inferred);
 *  - Standard, Standard specular, legacy diffuse, URP Lit and HDRP Lit are mapped by their tables (design section
 *    6.1): smoothness to roughness, the metallic-gloss map repacked (B = R, G = 1 - A * _GlossMapScale), occlusion
 *    from G, emission only with the _EMISSION keyword (its HDR intensity as KHR_materials_emissive_strength), the
 *    alpha modes, tiling and offset as KHR_texture_transform;
 *  - custom shaders and shader graphs (and unknown built-ins) by a heuristic on the property names, limited to the
 *    properties the shader declares and typed by its declaration; the manifest rule "shaderPropertyMaps"
 *    { "<shader name>": { "<property>": "<slot>" } } overrides it;
 *  - the colour space of a texture is that of the slot that uses it (colour for base colour and emission, linear
 *    for the rest), not the flag of its texture importer; colours are converted from sRGB to linear;
 *  - textures are linked only through the GUIDs of the material, never through the paths a model file names.
 *
 *  Every property goes into the source material; the slots used are recorded in its "mapping".
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityMaterialReader
{
    public:

    explicit CUnityMaterialReader( CUnityProject& project );

    /**
     *  The material of the scene for a renderer slot: a .mat ({fileID: 2100000, guid, type: 2}), the built-in
     *  default material, or the default for what cannot be resolved (with a warning). -1 for a null reference.
     */
    Int32 GetMaterial( const CUnityReference& reference, const CORE::CString& user );

    /** The material of a .mat file of the VFS; -1 when it cannot be read */
    Int32 ReadMaterialFile( const CORE::CString& vfsPath, Int64 fileId );

    /**
     *  The material of the scene for a material of a model file, as Unity assigns it to the model's own objects: the
     *  default material when the .meta imports none; else the .mat of externalObjects or of Unity's search (by
     *  texture or material name, in Materials folders up from the model or anywhere, per materialName and
     *  materialSearch); else the material of the file with its factors only (texture paths of model files are never
     *  followed). Made once per model material.
     */
    Int32 GetModelMaterial( const CUnityModel& model, UInt32 materialIndex );

    /** The default material of the editor, made once per scene */
    Int32 GetDefaultMaterial( void );

    /** Maps the object of a Material document; 'vfsPath' is its file */
    bool MapMaterial( const CORE::CDataNode& materialObject, const CORE::CString& vfsPath, ASSETCONVERTER::CAssetMaterial& material );

    /** sRGB to linear for one colour component (values above 1 of HDR colours follow the same curve) */
    static Float32 SrgbToLinear( Float32 value );

    private:

    struct STexEnv
    {
        CUnityReference texture;
        Float32 scale[ 2 ];
        Float32 offset[ 2 ];
    };
    typedef GUCEF::map< CORE::CString, STexEnv > TTexEnvMap;
    typedef GUCEF::map< CORE::CString, Float32 > TFloatMap;
    typedef GUCEF::map< CORE::CString, GUCEF::vector< Float32 > > TColourMap;

    /** The values of one material while it is mapped */
    struct SValues
    {
        CORE::CString name;
        CORE::CString subject;
        CORE::CString::StringSet keywords;
        TTexEnvMap textures;
        TFloatMap floats;
        TColourMap colours;
        CUnityBuiltins::TShaderKind kind;
        CORE::CString shaderName;
        const CUnityShaderInfo* shader;
        CORE::CDataNode* mapping;
    };

    void ReadValues( const CORE::CDataNode& materialObject, SValues& values );
    void RecordSource( const SValues& values, const CUnityReference& shaderReference, ASSETCONVERTER::CAssetMaterial& material );

    void MapStandard( SValues& values, ASSETCONVERTER::CAssetMaterial& material, bool specular );
    void MapLegacyDiffuse( SValues& values, ASSETCONVERTER::CAssetMaterial& material );
    void MapUrpLit( SValues& values, ASSETCONVERTER::CAssetMaterial& material );
    void MapHdrpLit( SValues& values, ASSETCONVERTER::CAssetMaterial& material );
    void MapByHeuristic( SValues& values, ASSETCONVERTER::CAssetMaterial& material );

    bool HasTexture( const SValues& values, const CORE::CString& property ) const;
    Float32 GetFloat( const SValues& values, const CORE::CString& property, Float32 defaultValue ) const;
    bool GetColour( const SValues& values, const CORE::CString& property, Float32 linear[ 4 ] ) const;

    /**
     *  A texture of the scene for the property, its reference with the tiling and offset of 'transformProperty' (the
     *  property itself when empty: Standard and URP Lit use those of the main texture for all their maps); unused
     *  when the slot has no texture
     */
    ASSETCONVERTER::CAssetTextureReference UseTexture( SValues& values, const CORE::CString& property, ASSETCONVERTER::TTextureUsage usage, const CORE::CString& slot,
                                                       const CORE::CString& transformProperty = CORE::CString::Empty );

    /** The metallic-roughness texture packed from a metallic map (R) and the smoothness of 'smoothnessProperty' (alpha) */
    ASSETCONVERTER::CAssetTextureReference UseMetallicGloss( SValues& values, const CORE::CString& metallicProperty, const CORE::CString& smoothnessProperty,
                                                             Float32 smoothnessScale, bool metallicFromRed, const CORE::CString& transformProperty = CORE::CString::Empty );

    /** Sets the tiling and offset of the texture environment of the property on the reference */
    void SetTransform( const SValues& values, const CORE::CString& property, ASSETCONVERTER::CAssetTextureReference& reference ) const;

    /** A texture with the operations; its index (textures with the same sources and chain are shared) */
    Int32 AddTexture( const CORE::CString::StringVector& sources, const ASSETCONVERTER::CAssetTexture::TOperationVector& operations,
                      ASSETCONVERTER::TTextureUsage usage, const CORE::CString& name, const CORE::CString& slot, const CORE::CString& subject );

    bool ResolveTexture( SValues& values, const CORE::CString& property, CORE::CString& vfsPath );

    void SetEmission( SValues& values, ASSETCONVERTER::CAssetMaterial& material, const CORE::CString& mapProperty, const CORE::CString& colourProperty, bool needsKeyword,
                      const CORE::CString& transformProperty = CORE::CString::Empty );

    CUnityMaterialReader( const CUnityMaterialReader& src );            /**< not implemented */
    CUnityMaterialReader& operator=( const CUnityMaterialReader& src ); /**< not implemented */

    CUnityProject& m_project;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H ? */
