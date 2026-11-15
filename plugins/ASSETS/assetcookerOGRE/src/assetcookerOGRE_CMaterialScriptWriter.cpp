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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "assetcookerOGRE_CMaterialScriptWriter.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";

/** The extensions the mapping carries itself (as values of the material or the texture references) */
static const char* const HandledExtensions[] = { "KHR_materials_emissive_strength", "KHR_texture_transform", "KHR_materials_unlit", GUCEF_NULL };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsOne( Float32 value )
{
    return fabs( value - 1.0f ) < 1e-6f;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
AddressMode( IMAGE::TWrapMode mode )
{
    switch ( mode )
    {
        case IMAGE::WRAPMODE_CLAMP : return "clamp";
        case IMAGE::WRAPMODE_MIRROR : return "mirror";
        default : return "wrap";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMaterialScriptWriter::CMaterialScriptWriter( const ASSETCONVERTER::CAssetScene& scene      ,
                                              CTextureCooker& textures                       ,
                                              const ASSETCONVERTER::CCookProfile& profile    ,
                                              ASSETCONVERTER::CConversionReport& report      )
    : m_scene( scene )
    , m_textures( textures )
    , m_profile( profile )
    , m_report( report )
    , m_text()
    , m_references()
    , m_materialCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32 CMaterialScriptWriter::GetMaterialCount( void ) const { return m_materialCount; }
const CORE::CString& CMaterialScriptWriter::GetText( void ) const { return m_text; }
const CORE::CString::StringVector& CMaterialScriptWriter::GetReferences( void ) const { return m_references; }

/*-------------------------------------------------------------------------*/

CORE::CString
CMaterialScriptWriter::Quote( const CORE::CString& text )
{GUCEF_TRACE;

    return "\"" + text.ReplaceChar( '"', '_' ) + "\"";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMaterialScriptWriter::FormatNumber( Float32 value )
{GUCEF_TRACE;

    return ASSETCONVERTER::CCanonicalJson::FormatFloat32( value );
}

/*-------------------------------------------------------------------------*/

void
CMaterialScriptWriter::Line( UInt32 depth, const CORE::CString& text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<depth; ++i )
        m_text += "    ";
    m_text += text;
    m_text += "\n";
}

/*-------------------------------------------------------------------------*/

void
CMaterialScriptWriter::AddReference( const CORE::CString& textureName )
{GUCEF_TRACE;

    const CORE::CString path = m_textures.ToVfsPath( textureName );
    for ( size_t i=0; i<m_references.size(); ++i )
        if ( m_references[ i ] == path )
            return;
    m_references.push_back( path );
}

/*-------------------------------------------------------------------------*/

bool
CMaterialScriptWriter::CookTexture( const ASSETCONVERTER::CAssetTextureReference& reference, CORE::CString& name )
{GUCEF_TRACE;

    if ( !reference.IsUsed() || reference.textureIndex < 0 || (size_t) reference.textureIndex >= m_scene.textures.size() )
        return false;
    if ( !m_textures.Cook( m_scene.textures[ reference.textureIndex ], name, m_report ) )
        return false;
    AddReference( name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMaterialScriptWriter::CookDerived( const ASSETCONVERTER::CAssetTexture& derived, CORE::CString& name )
{GUCEF_TRACE;

    if ( !m_textures.CookDerived( derived, name, m_report ) )
        return false;
    AddReference( name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMaterialScriptWriter::WriteTextureUnit( const CORE::CString& unitName                              ,
                                         const CORE::CString& textureName                           ,
                                         const ASSETCONVERTER::CAssetTextureReference& reference    ,
                                         bool sRgb                                                  ,
                                         const CORE::CString& extraLines                            ,
                                         const CORE::CString& materialName                          )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetTexture& texture = m_scene.textures[ reference.textureIndex ];
    Line( 3, "texture_unit " + unitName );
    Line( 3, "{" );
    CORE::CString textureLine = "texture " + Quote( textureName );
    if ( !m_profile.generateMips )
        textureLine += " 0";
    if ( sRgb )
        textureLine += " gamma";
    Line( 4, textureLine );
    if ( 0 != reference.texCoord )
        Line( 4, "tex_coord_set " + CORE::UInt32ToString( reference.texCoord ) );
    if ( IMAGE::WRAPMODE_REPEAT != texture.wrapS || IMAGE::WRAPMODE_REPEAT != texture.wrapT )
        Line( 4, "tex_address_mode " + AddressMode( texture.wrapS ) + " " + AddressMode( texture.wrapT ) );
    if ( reference.hasTransform )
    {
        // glTF: uv * tiling + offset. Ogre: uv / scale + ( 0.5 - 0.5 / scale ) + scroll, so scale = 1 / tiling and
        // scroll = offset - 0.5 + 0.5 * tiling
        const Float32 tilingU = 0.0f != reference.tiling[ 0 ] ? reference.tiling[ 0 ] : 1.0f;
        const Float32 tilingV = 0.0f != reference.tiling[ 1 ] ? reference.tiling[ 1 ] : 1.0f;
        if ( !IsOne( tilingU ) || !IsOne( tilingV ) )
            Line( 4, "scale " + FormatNumber( 1.0f / tilingU ) + " " + FormatNumber( 1.0f / tilingV ) );
        const Float32 scrollU = reference.offset[ 0 ] - 0.5f + 0.5f * tilingU;
        const Float32 scrollV = reference.offset[ 1 ] - 0.5f + 0.5f * tilingV;
        if ( 0.0f != scrollU || 0.0f != scrollV )
            Line( 4, "scroll " + FormatNumber( scrollU ) + " " + FormatNumber( scrollV ) );
        if ( 0.0f != reference.rotation )
            m_report.AddApproximation( SourceName, materialName, "the rotation of the texture transform of " + unitName + " is left out" );
    }
    if ( !extraLines.IsNULLOrEmpty() )
    {
        const CORE::CString::StringVector lines = extraLines.ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
            Line( 4, lines[ i ] );
    }
    Line( 3, "}" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMaterialScriptWriter::GetMetalRoughness( const ASSETCONVERTER::CAssetMaterial& material, bool& occlusion )
{GUCEF_TRACE;

    occlusion = false;
    const ASSETCONVERTER::CAssetTextureReference& mr = material.metallicRoughnessTexture;
    const ASSETCONVERTER::CAssetTextureReference& ao = material.occlusionTexture;
    const bool hasMr = mr.IsUsed() && mr.textureIndex >= 0 && (size_t) mr.textureIndex < m_scene.textures.size() && !m_scene.textures[ mr.textureIndex ].missing;
    const bool hasAo = ao.IsUsed() && ao.textureIndex >= 0 && (size_t) ao.textureIndex < m_scene.textures.size() && !m_scene.textures[ ao.textureIndex ].missing;
    const bool aoInMr = hasMr && hasAo && ao.textureIndex == mr.textureIndex;
    const bool factors = !IsOne( material.metallicFactor ) || !IsOne( material.roughnessFactor );

    if ( hasAo && !IsOne( ao.scale ) )
        m_report.AddApproximation( SourceName, material.name, "the occlusion strength " + FormatNumber( ao.scale ) + " is not applied: the occlusion is used at full strength" );
    if ( ( hasMr && 0 != mr.texCoord ) || ( hasAo && 0 != ao.texCoord ) )
        m_report.AddApproximation( SourceName, material.name, "the metal-roughness stage reads its texture with the first UV set, not set " + CORE::UInt32ToString( hasMr ? mr.texCoord : ao.texCoord ) );
    if ( ( hasMr && mr.hasTransform ) || ( hasAo && ao.hasTransform ) )
        m_report.AddApproximation( SourceName, material.name, "the metal-roughness stage does not apply the texture transform" );

    if ( !hasMr && !hasAo )
        return CORE::CString::Empty;

    CORE::CString name;
    if ( hasMr && !factors && ( !hasAo || aoInMr ) )
    {
        // the texture of the intermediate as it is: G roughness, B metallic and, packed in already, R occlusion
        if ( !CookTexture( mr, name ) )
            return CORE::CString::Empty;
        occlusion = aoInMr;
        return name;
    }

    // a derived texture: R occlusion, G roughness, B metallic, with the factors folded in
    const ASSETCONVERTER::CAssetTexture& first = m_scene.textures[ hasMr ? mr.textureIndex : ao.textureIndex ];
    ASSETCONVERTER::CAssetTexture derived;
    derived.name = material.name + " ORM";
    derived.usage = ASSETCONVERTER::TEXTUREUSAGE_MASK;
    derived.wrapS = first.wrapS;
    derived.wrapT = first.wrapT;
    Int32 sources[ 4 ] = { -1, -1, -1, -1 };
    UInt32 channels[ 4 ] = { 0, 1, 2, 3 };
    if ( hasMr )
    {
        derived.sources.push_back( m_scene.textures[ mr.textureIndex ].sources.empty() ? CORE::CString::Empty : m_scene.textures[ mr.textureIndex ].sources[ 0 ] );
        sources[ 1 ] = 0;
        sources[ 2 ] = 0;
    }
    if ( hasAo )
    {
        if ( aoInMr )
        {
            sources[ 0 ] = 0;
        }
        else
        {
            derived.sources.push_back( m_scene.textures[ ao.textureIndex ].sources.empty() ? CORE::CString::Empty : m_scene.textures[ ao.textureIndex ].sources[ 0 ] );
            sources[ 0 ] = (Int32) derived.sources.size() - 1;
        }
        channels[ 0 ] = 0;
    }
    derived.operations.push_back( ASSETCONVERTER::CTextureOperation::Pack( sources, channels ) );
    if ( hasMr )
    {
        if ( !IsOne( material.roughnessFactor ) )
            derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ScaleBias( 0x2, material.roughnessFactor, 0.0f ) );
        if ( !IsOne( material.metallicFactor ) )
            derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ScaleBias( 0x4, material.metallicFactor, 0.0f ) );
    }
    else
    {
        // without a metal-roughness texture the channels hold the factors; the packed constants are 0 and the
        // texels have 8 bits
        derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ScaleBias( 0x2, 1.0f, material.roughnessFactor * 255.0f ) );
        derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ScaleBias( 0x4, 1.0f, material.metallicFactor * 255.0f ) );
    }
    derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_LINEAR ) );
    if ( !CookDerived( derived, name ) )
        return CORE::CString::Empty;
    occlusion = hasAo;
    m_report.AddInfo( SourceName, material.name, "the metal-roughness texture of the RTSS is derived: occlusion in R, roughness in G, metalness in B, factors folded in" );
    return name;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMaterialScriptWriter::GetEmissive( const ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetTextureReference& reference = material.emissiveTexture;
    if ( !reference.IsUsed() || reference.textureIndex < 0 || (size_t) reference.textureIndex >= m_scene.textures.size() || m_scene.textures[ reference.textureIndex ].missing )
        return CORE::CString::Empty;
    Float32 factor[ 3 ];
    bool neutral = true;
    bool black = true;
    bool clamped = false;
    for ( UInt32 c=0; c<3; ++c )
    {
        factor[ c ] = material.emissiveFactor[ c ] * material.emissiveStrength;
        neutral = neutral && IsOne( factor[ c ] );
        black = black && 0.0f == factor[ c ];
        clamped = clamped || factor[ c ] > 1.0f;
    }
    if ( black )
        return CORE::CString::Empty;
    CORE::CString name;
    if ( neutral )
        return CookTexture( reference, name ) ? name : CORE::CString::Empty;

    const ASSETCONVERTER::CAssetTexture& source = m_scene.textures[ reference.textureIndex ];
    ASSETCONVERTER::CAssetTexture derived;
    derived.name = material.name + " emissive";
    derived.usage = ASSETCONVERTER::TEXTUREUSAGE_COLOUR;
    derived.wrapS = source.wrapS;
    derived.wrapT = source.wrapT;
    if ( !source.sources.empty() )
        derived.sources.push_back( source.sources[ 0 ] );
    for ( UInt32 c=0; c<3; ++c )
        if ( !IsOne( factor[ c ] ) )
            derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ScaleBias( 1u << c, (Float32) pow( (Float64) factor[ c ], 1.0 / 2.2 ), 0.0f ) );
    derived.operations.push_back( ASSETCONVERTER::CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_SRGB ) );
    if ( !CookDerived( derived, name ) )
        return CORE::CString::Empty;
    if ( clamped )
        m_report.AddApproximation( SourceName, material.name, "the emission above 1 (factor times strength) is clamped in the derived emissive texture" );
    else
        m_report.AddInfo( SourceName, material.name, "the emissive factor is folded into a derived emissive texture" );
    return name;
}

/*-------------------------------------------------------------------------*/

void
CMaterialScriptWriter::AddMaterial( const CORE::CString& materialName, const ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    // the extensions the RTSS has nothing for
    CORE::CDataNode::const_iterator e = material.extensions.ConstBegin();
    while ( e != material.extensions.ConstEnd() )
    {
        const CORE::CString extension = (*e)->GetName();
        bool handled = false;
        for ( UInt32 i=0; GUCEF_NULL != HandledExtensions[ i ] && !handled; ++i )
            handled = extension == HandledExtensions[ i ];
        if ( !handled && !extension.IsNULLOrEmpty() )
            m_report.AddApproximation( SourceName, material.name, "the extension " + extension + " has no counterpart in the RTSS material script and is left out" );
        ++e;
    }

    if ( m_materialCount > 0 )
        m_text += "\n";
    ++m_materialCount;

    Line( 0, "material " + Quote( materialName ) );
    Line( 0, "{" );
    Line( 1, "technique" );
    Line( 1, "{" );
    Line( 2, "pass" );
    Line( 2, "{" );

    const Float32* base = material.baseColorFactor;
    Line( 3, "diffuse " + FormatNumber( base[ 0 ] ) + " " + FormatNumber( base[ 1 ] ) + " " + FormatNumber( base[ 2 ] ) + " " + FormatNumber( base[ 3 ] ) );
    if ( !material.unlit )
        Line( 3, "specular " + FormatNumber( material.roughnessFactor ) + " " + FormatNumber( material.metallicFactor ) + " 0 1 0" );

    const CORE::CString emissiveTexture = material.unlit ? CORE::CString::Empty : GetEmissive( material );
    if ( emissiveTexture.IsNULLOrEmpty() && !material.unlit )
    {
        const Float32 r = material.emissiveFactor[ 0 ] * material.emissiveStrength;
        const Float32 g = material.emissiveFactor[ 1 ] * material.emissiveStrength;
        const Float32 b = material.emissiveFactor[ 2 ] * material.emissiveStrength;
        if ( 0.0f != r || 0.0f != g || 0.0f != b )
            Line( 3, "emissive " + FormatNumber( r ) + " " + FormatNumber( g ) + " " + FormatNumber( b ) );
    }

    switch ( material.alphaMode )
    {
        case ASSETCONVERTER::ALPHAMODE_BLEND :
        {
            Line( 3, "scene_blend alpha_blend" );
            Line( 3, "depth_write off" );
            break;
        }
        case ASSETCONVERTER::ALPHAMODE_MASK :
        {
            Float32 cutoff = material.alphaCutoff * 255.0f + 0.5f;
            if ( cutoff < 0.0f ) cutoff = 0.0f;
            if ( cutoff > 255.0f ) cutoff = 255.0f;
            Line( 3, "alpha_rejection greater_equal " + CORE::UInt32ToString( (UInt32) cutoff ) );
            break;
        }
        default :
            break;
    }
    if ( material.doubleSided )
        Line( 3, "cull_hardware none" );
    if ( material.unlit )
    {
        Line( 3, "lighting off" );
        if ( !IsOne( base[ 0 ] ) || !IsOne( base[ 1 ] ) || !IsOne( base[ 2 ] ) )
            m_report.AddApproximation( SourceName, material.name, "unlit: the base colour factor is not applied without lighting" );
    }

    CORE::CString name;
    if ( CookTexture( material.baseColorTexture, name ) )
        WriteTextureUnit( "BaseColour", name, material.baseColorTexture, true, CORE::CString::Empty, material.name );
    if ( !material.unlit && CookTexture( material.normalTexture, name ) )
    {
        WriteTextureUnit( "NormalMap", name, material.normalTexture, false, "rtshader_system\n{\n    normal_map tangent_space\n}", material.name );
        if ( !IsOne( material.normalTexture.scale ) )
            m_report.AddApproximation( SourceName, material.name, "the normal scale " + FormatNumber( material.normalTexture.scale ) + " is not applied by the RTSS normal map" );
    }
    if ( !emissiveTexture.IsNULLOrEmpty() )
        WriteTextureUnit( "Emissive", emissiveTexture, material.emissiveTexture, true, "colour_op add", material.name );

    if ( !material.unlit )
    {
        bool occlusion = false;
        const CORE::CString metalRoughness = GetMetalRoughness( material, occlusion );
        Line( 3, "rtshader_system" );
        Line( 3, "{" );
        if ( metalRoughness.IsNULLOrEmpty() )
            Line( 4, "lighting_stage metal_roughness" );
        else
            Line( 4, "lighting_stage metal_roughness texture " + Quote( metalRoughness ) + ( occlusion ? " occlusion" : "" ) );
        if ( !emissiveTexture.IsNULLOrEmpty() )
            Line( 4, "texturing_stage late_add_blend" );
        Line( 3, "}" );
    }

    Line( 2, "}" );
    Line( 1, "}" );
    Line( 0, "}" );
}

/*-------------------------------------------------------------------------*/

void
CMaterialScriptWriter::AddParticleMaterial( const CORE::CString& materialName, const ASSETCONVERTER::CAssetParticleSystem& system )
{GUCEF_TRACE;

    ++m_materialCount;
    if ( !m_text.IsNULLOrEmpty() )
        Line( 0, "" );
    Line( 0, "material " + Quote( materialName ) );
    Line( 0, "{" );
    Line( 1, "technique" );
    Line( 1, "{" );
    Line( 2, "pass" );
    Line( 2, "{" );
    Line( 3, "lighting off" );
    if ( system.renderer.useParticleColour )
        Line( 3, "diffuse vertexcolour" );
    switch ( system.renderer.blendMode )
    {
        case ASSETCONVERTER::PARTICLEBLEND_ADDITIVE: Line( 3, "scene_blend src_alpha one" ); break;
        case ASSETCONVERTER::PARTICLEBLEND_PREMULTIPLIED: Line( 3, "scene_blend one one_minus_src_alpha" ); break;
        case ASSETCONVERTER::PARTICLEBLEND_MULTIPLY: Line( 3, "scene_blend modulate" ); break;
        case ASSETCONVERTER::PARTICLEBLEND_SUBTRACT:
            Line( 3, "scene_blend src_alpha one" );
            Line( 3, "scene_blend_op reverse_subtract" );
            break;
        case ASSETCONVERTER::PARTICLEBLEND_OPAQUE: break;
        default: Line( 3, "scene_blend alpha_blend" ); break;
    }
    if ( ASSETCONVERTER::PARTICLEBLEND_OPAQUE != system.renderer.blendMode )
        Line( 3, "depth_write off" );
    if ( system.renderer.alphaCutoff > 0.0f )
    {
        Float32 cutoff = system.renderer.alphaCutoff * 255.0f + 0.5f;
        cutoff = cutoff > 255.0f ? 255.0f : cutoff;
        Line( 3, "alpha_rejection greater_equal " + CORE::UInt32ToString( (UInt32) cutoff ) );
    }
    Line( 3, "cull_hardware none" );

    const Int32 index = system.renderer.material;
    CORE::CString name;
    if ( index >= 0 && (size_t) index < m_scene.materials.size() && CookTexture( m_scene.materials[ index ].baseColorTexture, name ) )
    {
        const ASSETCONVERTER::CAssetTextureReference& reference = m_scene.materials[ index ].baseColorTexture;
        const bool sheet = system.textureSheet.enabled && system.textureSheet.tilesX * system.textureSheet.tilesY > 1;
        WriteTextureUnit( "BaseColour", name, reference, true, sheet ? "tex_address_mode clamp" : CORE::CString::Empty, m_scene.materials[ index ].name );
    }
    Line( 2, "}" );
    Line( 1, "}" );
    Line( 0, "}" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
