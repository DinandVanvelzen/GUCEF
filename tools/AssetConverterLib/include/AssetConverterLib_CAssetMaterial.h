/*
 *  AssetConverterLib: a material of the intermediate model in glTF terms
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

#ifndef ASSETCONVERTERLIB_CASSETMATERIAL_H
#define ASSETCONVERTERLIB_CASSETMATERIAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H
#include "AssetConverterLib_CAssetTextureReference.h"
#define ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H
#endif /* ASSETCONVERTERLIB_CASSETTEXTUREREFERENCE_H ? */

#ifndef ASSETCONVERTERLIB_CSOURCEMATERIAL_H
#include "AssetConverterLib_CSourceMaterial.h"
#define ASSETCONVERTERLIB_CSOURCEMATERIAL_H
#endif /* ASSETCONVERTERLIB_CSOURCEMATERIAL_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A material of an asset: the resolved PBR metal-roughness values and textures in glTF terms, with the emissive
 *  strength of KHR_materials_emissive_strength, plus the source material (CSourceMaterial) with the shader name and
 *  every original property. Extensions with source data that the core has no members for (KHR_materials_specular,
 *  KHR_materials_clearcoat) are kept by their extension name in 'extensions'. A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetMaterial
{
    public:

    CORE::CString name;
    Float32 baseColorFactor[ 4 ];
    CAssetTextureReference baseColorTexture;
    Float32 metallicFactor;
    Float32 roughnessFactor;
    CAssetTextureReference metallicRoughnessTexture;   /**< G roughness, B metallic */
    CAssetTextureReference normalTexture;              /**< OpenGL convention, green up */
    CAssetTextureReference occlusionTexture;           /**< R occlusion */
    Float32 emissiveFactor[ 3 ];
    CAssetTextureReference emissiveTexture;
    Float32 emissiveStrength;                          /**< KHR_materials_emissive_strength, 1 without */
    TAlphaMode alphaMode;
    Float32 alphaCutoff;
    bool doubleSided;
    bool unlit;
    CORE::CDataNode extensions;                         /**< by extension name: an object node */
    CSourceMaterial source;

    CAssetMaterial( void );

    /** Whether the two render the same: every resolved value, texture and extension; the names and the sources are not compared */
    bool HasSameAppearance( const CAssetMaterial& other ) const;

    /** The texture references of the slots, for walks over every slot */
    void GetTextureReferences( GUCEF::vector< const CAssetTextureReference* >& references ) const;
    void GetTextureReferences( GUCEF::vector< CAssetTextureReference* >& references );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETMATERIAL_H ? */
