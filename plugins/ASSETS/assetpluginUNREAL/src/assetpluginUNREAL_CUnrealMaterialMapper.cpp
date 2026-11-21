/*
 *  assetpluginUNREAL: Maps the resolved parameters of a material to a metal-roughness material of the scene by a table of rules
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#include "assetpluginUNREAL_CUnrealMaterialMapper.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealMaterialMapper::Normalise( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString result;
    const CORE::CString lower = name.Lowercase();
    for ( UInt32 i=0; i<lower.Length(); ++i )
        if ( ' ' != lower[ i ] && '_' != lower[ i ] && '-' != lower[ i ] )
            result += lower[ i ];
    return result;
}

/*-------------------------------------------------------------------------*/

/** The parameters that rules have used, by the name as authored */
class CConsumed
{
    public:

    CORE::CString::StringSet names;

    void Add( const CORE::CString& name ) { names.insert( name ); }
    bool Has( const CORE::CString& name ) const { return names.find( name ) != names.end(); }
};

/*-------------------------------------------------------------------------*/

/** The first parameter of the map whose normalised name is in the list of normalised names, by the order of the list */
template< typename TMap >
static typename TMap::const_iterator
FindParameter( const TMap& parameters, const char* const* patterns )
{
    for ( UInt32 p=0; GUCEF_NULL != patterns[ p ]; ++p )
    {
        typename TMap::const_iterator i = parameters.begin();
        while ( i != parameters.end() )
        {
            if ( CUnrealMaterialMapper::Normalise( (*i).first ) == patterns[ p ] )
                return i;
            ++i;
        }
    }
    return parameters.end();
}

/*-------------------------------------------------------------------------*/

static const char* const BaseColourVectors[] = { "basecolortint", "tint", "basecolor", "albedo", "diffuse", "color", GUCEF_NULL };
static const char* const BaseColourTextures[] = { "basecolortex", "basecolor", "albedo", "albedotex", "diffuse", "diffusetex", "color", "colortex", GUCEF_NULL };
static const char* const BrightnessScalars[] = { "basecolorbrightness", GUCEF_NULL };
static const char* const SaturationScalars[] = { "basecolorsaturation", GUCEF_NULL };
static const char* const RoughnessScalars[] = { "roughness", GUCEF_NULL };
static const char* const RoughnessMinScalars[] = { "roughnessmin", GUCEF_NULL };
static const char* const RoughnessMaxScalars[] = { "roughnessmax", GUCEF_NULL };
static const char* const MetallicScalars[] = { "metallic", GUCEF_NULL };
static const char* const MetallicSwitches[] = { "useormmetallic", "usemetallic", GUCEF_NULL };
static const char* const NormalStrengths[] = { "normalstrength", "normalintensity", GUCEF_NULL };
static const char* const OcclusionStrengths[] = { "aostrength", "occlusionstrength", GUCEF_NULL };
static const char* const SpecularScalars[] = { "specular", GUCEF_NULL };
static const char* const EmissiveVectors[] = { "emissivecolor", "emissive", GUCEF_NULL };
static const char* const EmissiveScalars[] = { "emissiveintensity", "emissivestrength", "emissivemultiplier", GUCEF_NULL };
static const char* const OpacityScalars[] = { "opacity", GUCEF_NULL };

/*-------------------------------------------------------------------------*/

/** The part of the material the texture of a parameter is meant for, by the names that master materials use */
static const char*
TextureSlotOf( const CORE::CString& parameter )
{
    const CORE::CString name = CUnrealMaterialMapper::Normalise( parameter );
    if ( name.HasSubstr( "normal" ) >= 0 ) return "normalTexture";
    if ( name.HasSubstr( "orm" ) == 0 || name.HasSubstr( "arm" ) == 0 || name.HasSubstr( "occlusionroughnessmetallic" ) >= 0 ) return "occlusionTexture and metallicRoughnessTexture";
    if ( name.HasSubstr( "emissive" ) >= 0 ) return "emissiveTexture";
    if ( name.HasSubstr( "basecolor" ) >= 0 || name.HasSubstr( "albedo" ) >= 0 || name.HasSubstr( "diffuse" ) >= 0 || "basetexture" == name || "maintexture" == name || name.HasSubstr( "colortexture" ) >= 0 || name.HasSubstr( "colourtexture" ) >= 0 ) return "baseColorTexture";
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CUnrealMaterialMapper::Map( const CUnrealResolvedMaterial& resolved, const CORE::CString& name, ASSETCONVERTER::CAssetMaterial& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    out = ASSETCONVERTER::CAssetMaterial();
    out.name = name;
    out.metallicFactor = 0.0f;
    out.roughnessFactor = 0.5f;
    CConsumed consumed;
    CORE::CDataNode& properties = out.source.properties;
    out.source.shaderName = resolved.masterPath.IsNULLOrEmpty() ? CORE::CString( "Unreal/unknown" ) : "Unreal" + ( resolved.masterPath[ 0 ] == '/' ? resolved.masterPath : "/" + resolved.masterPath );
    properties.SetAttribute( "objectPath", resolved.chain.empty() ? name : resolved.chain[ 0 ] );
    if ( !resolved.blendMode.IsNULLOrEmpty() )
        properties.SetAttribute( "blendMode", resolved.blendMode );
    if ( !resolved.shadingModel.IsNULLOrEmpty() )
        properties.SetAttribute( "shadingModel", resolved.shadingModel );

    // the base colour: the tint is the factor; brightness and saturation are baked into it when no texture carries the colour
    bool baseColourTexture = false;
    {
        CUnrealResolvedMaterial::TTextureMap::const_iterator t = FindParameter( resolved.textures, BaseColourTextures );
        baseColourTexture = t != resolved.textures.end() && !(*t).second.IsNULLOrEmpty();
    }
    Float64 colour[ 3 ] = { 1.0, 1.0, 1.0 };
    {
        CUnrealResolvedMaterial::TColourMap::const_iterator c = FindParameter( resolved.colours, BaseColourVectors );
        if ( c != resolved.colours.end() && baseColourTexture && 0.0 == (*c).second.r && 0.0 == (*c).second.g && 0.0 == (*c).second.b )
        {
            // a black tint next to a base colour texture is the default of the parameter (nothing stores it): a factor of black would
            // hide the texture, which no master material that offers the texture does; how the master uses the tint is not read
            report.AddApproximation( SourceName, name, "the tint " + (*c).first + " is black (the default of the parameter) next to the base colour texture: not used as the base colour factor, kept in the source material" );
        }
        else
        if ( c != resolved.colours.end() )
        {
            colour[ 0 ] = (*c).second.r; colour[ 1 ] = (*c).second.g; colour[ 2 ] = (*c).second.b;
            consumed.Add( (*c).first );
        }
        CUnrealResolvedMaterial::TScalarMap::const_iterator brightness = FindParameter( resolved.scalars, BrightnessScalars );
        CUnrealResolvedMaterial::TScalarMap::const_iterator saturation = FindParameter( resolved.scalars, SaturationScalars );
        if ( !baseColourTexture )
        {
            if ( brightness != resolved.scalars.end() )
            {
                for ( UInt32 i=0; i<3; ++i ) colour[ i ] *= (*brightness).second;
                consumed.Add( (*brightness).first );
            }
            if ( saturation != resolved.scalars.end() )
            {
                const Float64 grey = 0.2126 * colour[ 0 ] + 0.7152 * colour[ 1 ] + 0.0722 * colour[ 2 ];
                for ( UInt32 i=0; i<3; ++i ) colour[ i ] = grey + ( colour[ i ] - grey ) * (*saturation).second;
                consumed.Add( (*saturation).first );
            }
        }
        else
        if ( brightness != resolved.scalars.end() || saturation != resolved.scalars.end() )
            report.AddApproximation( SourceName, name, "the brightness and saturation of the base colour apply to the base colour texture: kept in the source material, not baked" );
        for ( UInt32 i=0; i<3; ++i )
            out.baseColorFactor[ i ] = (Float32) ( colour[ i ] < 0.0 ? 0.0 : colour[ i ] );
        out.baseColorFactor[ 3 ] = 1.0f;
    }

    // roughness and metallic
    {
        CUnrealResolvedMaterial::TScalarMap::const_iterator roughness = FindParameter( resolved.scalars, RoughnessScalars );
        if ( roughness != resolved.scalars.end() )
        {
            out.roughnessFactor = (Float32) (*roughness).second;
            consumed.Add( (*roughness).first );
        }
        else
        {
            CUnrealResolvedMaterial::TScalarMap::const_iterator low = FindParameter( resolved.scalars, RoughnessMinScalars );
            CUnrealResolvedMaterial::TScalarMap::const_iterator high = FindParameter( resolved.scalars, RoughnessMaxScalars );
            if ( low != resolved.scalars.end() && high != resolved.scalars.end() )
            {
                // the roughness of the texture is remapped between the two: the middle is the best single value
                out.roughnessFactor = (Float32) ( ( (*low).second + (*high).second ) * 0.5 );
                consumed.Add( (*low).first );
                consumed.Add( (*high).first );
                report.AddApproximation( SourceName, name, "the roughness is remapped between a minimum and a maximum in the material: the mean is used as factor" );
            }
        }
        CUnrealResolvedMaterial::TScalarMap::const_iterator metallic = FindParameter( resolved.scalars, MetallicScalars );
        if ( metallic != resolved.scalars.end() )
        {
            out.metallicFactor = (Float32) (*metallic).second;
            consumed.Add( (*metallic).first );
        }
        // a static switch that turns the metallic channel of the packed texture off
        CUnrealResolvedMaterial::TSwitchMap::const_iterator metalSwitch = FindParameter( resolved.switches, MetallicSwitches );
        if ( metalSwitch != resolved.switches.end() )
        {
            if ( !(*metalSwitch).second )
                out.metallicFactor = 0.0f;
            consumed.Add( (*metalSwitch).first );
        }
        out.roughnessFactor = out.roughnessFactor < 0.0f ? 0.0f : ( out.roughnessFactor > 1.0f ? 1.0f : out.roughnessFactor );
        out.metallicFactor = out.metallicFactor < 0.0f ? 0.0f : ( out.metallicFactor > 1.0f ? 1.0f : out.metallicFactor );
    }

    // strengths of the normal map and the occlusion, and the specular
    {
        CUnrealResolvedMaterial::TScalarMap::const_iterator normal = FindParameter( resolved.scalars, NormalStrengths );
        if ( normal != resolved.scalars.end() )
        {
            out.normalTexture.scale = (Float32) (*normal).second;
            consumed.Add( (*normal).first );
        }
        CUnrealResolvedMaterial::TScalarMap::const_iterator occlusion = FindParameter( resolved.scalars, OcclusionStrengths );
        if ( occlusion != resolved.scalars.end() )
        {
            out.occlusionTexture.scale = (Float32) (*occlusion).second;
            consumed.Add( (*occlusion).first );
        }
        CUnrealResolvedMaterial::TScalarMap::const_iterator specular = FindParameter( resolved.scalars, SpecularScalars );
        if ( specular != resolved.scalars.end() )
        {
            CORE::CDataNode* extension = out.extensions.AddChild( "KHR_materials_specular", GUCEF_DATATYPE_OBJECT );
            extension->SetAttribute( "specularFactor", (Float32) (*specular).second );
            consumed.Add( (*specular).first );
        }
    }

    // emission
    {
        CUnrealResolvedMaterial::TColourMap::const_iterator emissive = FindParameter( resolved.colours, EmissiveVectors );
        if ( emissive != resolved.colours.end() )
        {
            out.emissiveFactor[ 0 ] = (Float32) (*emissive).second.r;
            out.emissiveFactor[ 1 ] = (Float32) (*emissive).second.g;
            out.emissiveFactor[ 2 ] = (Float32) (*emissive).second.b;
            consumed.Add( (*emissive).first );
        }
        CUnrealResolvedMaterial::TScalarMap::const_iterator intensity = FindParameter( resolved.scalars, EmissiveScalars );
        if ( intensity != resolved.scalars.end() )
        {
            out.emissiveStrength = (Float32) (*intensity).second;
            consumed.Add( (*intensity).first );
        }
    }

    // the base properties: blend mode, mask clip value and two sidedness
    out.doubleSided = resolved.twoSided;
    if ( "BLEND_Masked" == resolved.blendMode )
    {
        out.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
        out.alphaCutoff = (Float32) resolved.opacityMaskClipValue;
    }
    else
    if ( "BLEND_Translucent" == resolved.blendMode || "BLEND_AlphaComposite" == resolved.blendMode || "BLEND_AlphaHoldout" == resolved.blendMode )
        out.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    else
    if ( "BLEND_Additive" == resolved.blendMode || "BLEND_Modulate" == resolved.blendMode )
    {
        out.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
        report.AddApproximation( SourceName, name, "the blend mode " + resolved.blendMode + " has no counterpart: the material is blended" );
    }
    else
        out.alphaMode = ASSETCONVERTER::ALPHAMODE_OPAQUE;
    if ( ASSETCONVERTER::ALPHAMODE_BLEND == out.alphaMode )
    {
        CUnrealResolvedMaterial::TScalarMap::const_iterator opacity = FindParameter( resolved.scalars, OpacityScalars );
        if ( opacity != resolved.scalars.end() )
        {
            out.baseColorFactor[ 3 ] = (Float32) (*opacity).second;
            consumed.Add( (*opacity).first );
        }
    }

    // the shading model: metal-roughness stands in for the others
    if ( !resolved.shadingModel.IsNULLOrEmpty() && "MSM_DefaultLit" != resolved.shadingModel )
    {
        if ( "MSM_Cloth" == resolved.shadingModel )
        {
            UInt32 fuzz = 0;
            for ( CUnrealResolvedMaterial::TScalarMap::const_iterator i = resolved.scalars.begin(); i != resolved.scalars.end(); ++i )
                if ( Normalise( (*i).first ).HasSubstr( "fuzz" ) == 0 ) { consumed.Add( (*i).first ); ++fuzz; }
            for ( CUnrealResolvedMaterial::TColourMap::const_iterator i = resolved.colours.begin(); i != resolved.colours.end(); ++i )
                if ( Normalise( (*i).first ).HasSubstr( "fuzz" ) == 0 ) { consumed.Add( (*i).first ); ++fuzz; }
            report.AddApproximation( SourceName, name, "the shading model is cloth: metal-roughness is used, its " + CORE::UInt32ToString( fuzz ) + " fuzz parameters are kept in the source material" );
        }
        else
            report.AddApproximation( SourceName, name, "the shading model " + resolved.shadingModel + " has no counterpart: metal-roughness is used" );
    }

    // the textures: recorded with the slot they are meant for; CUnrealTextureConverter::AttachToMaterials() fills the slots
    UInt32 textureCount = 0;
    for ( CUnrealResolvedMaterial::TTextureMap::const_iterator t = resolved.textures.begin(); t != resolved.textures.end(); ++t )
    {
        if ( (*t).second.IsNULLOrEmpty() )
            continue;
        CORE::CDataNode* slot = out.source.textures.AddChild( (*t).first, GUCEF_DATATYPE_OBJECT );
        slot->SetAttribute( "object", (*t).second );
        const char* target = TextureSlotOf( (*t).first );
        if ( GUCEF_NULL != target )
            slot->SetAttribute( "slot", CORE::CString( target ) );
        consumed.Add( (*t).first );
        ++textureCount;
    }
    if ( textureCount > 0 )
        report.AddApproximation( SourceName, name, CORE::UInt32ToString( textureCount ) + " texture parameters are recorded with the slot they are meant for; the importer puts the textures it can read in those slots (engine textures and textures that are not in the pack stay empty)" );

    // everything no rule used goes to the source material, and one report entry names it
    CORE::CString unmapped;
    UInt32 unmappedCount = 0;
    for ( CUnrealResolvedMaterial::TScalarMap::const_iterator i = resolved.scalars.begin(); i != resolved.scalars.end(); ++i )
    {
        if ( consumed.Has( (*i).first ) )
            continue;
        properties.SetAttribute( (*i).first, (Float32) (*i).second );
        if ( unmappedCount++ < 40 ) unmapped += ( unmapped.IsNULLOrEmpty() ? "" : ", " ) + (*i).first;
    }
    for ( CUnrealResolvedMaterial::TColourMap::const_iterator i = resolved.colours.begin(); i != resolved.colours.end(); ++i )
    {
        if ( consumed.Has( (*i).first ) )
            continue;
        CORE::CDataNode::TVariantVector values;
        values.push_back( CORE::CVariant( (Float32) (*i).second.r ) );
        values.push_back( CORE::CVariant( (Float32) (*i).second.g ) );
        values.push_back( CORE::CVariant( (Float32) (*i).second.b ) );
        values.push_back( CORE::CVariant( (Float32) (*i).second.a ) );
        properties.SetValuesOfChildByName( (*i).first, values );
        if ( unmappedCount++ < 40 ) unmapped += ( unmapped.IsNULLOrEmpty() ? "" : ", " ) + (*i).first;
    }
    for ( CUnrealResolvedMaterial::TSwitchMap::const_iterator i = resolved.switches.begin(); i != resolved.switches.end(); ++i )
    {
        if ( consumed.Has( (*i).first ) )
            continue;
        properties.SetAttribute( (*i).first, (*i).second );
        if ( unmappedCount++ < 40 ) unmapped += ( unmapped.IsNULLOrEmpty() ? "" : ", " ) + (*i).first;
    }
    if ( unmappedCount > 0 )
        report.AddApproximation( SourceName, name, CORE::UInt32ToString( unmappedCount ) + " parameters have no counterpart in the metal-roughness material and are kept in the source material: " + unmapped + ( unmappedCount > 40 ? ", ..." : "" ) );
    for ( size_t p=0; p<resolved.problems.size(); ++p )
        report.AddWarning( SourceName, name, resolved.problems[ p ] );
}

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
