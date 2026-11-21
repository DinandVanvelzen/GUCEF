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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#include "assetpluginGODOT_CGodotPaths.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H ? */

#include "assetpluginGODOT_CGodotMaterialReader.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "godot";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static Float32
Clamp01( Float64 value )
{
    return value < 0.0 ? 0.0f : ( value > 1.0 ? 1.0f : (Float32) value );
}

/*-------------------------------------------------------------------------*/

static const char*
ChannelLetter( Int32 channel )
{
    static const char* const letters[ 4 ] = { "R", "G", "B", "A" };
    return channel >= 0 && channel < 4 ? letters[ channel ] : "R";
}

/*-------------------------------------------------------------------------*/

static UInt32
ChannelIndex( Int32 channel )
{
    return channel >= 0 && channel < 4 ? (UInt32) channel : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotMaterialReader::CGodotMaterialReader( ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_materials()
    , m_textures()
    , m_document( GUCEF_NULL )
    , m_section( GUCEF_NULL )
    , m_subject()
    , m_wrap( IMAGE::WRAPMODE_REPEAT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotMaterialReader::~CGodotMaterialReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGodotMaterialReader::IsMaterialType( const CORE::CString& type )
{GUCEF_TRACE;

    return "StandardMaterial3D" == type || "ORMMaterial3D" == type || "BaseMaterial3D" == type || "ShaderMaterial" == type;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotMaterialReader::Prop( const CORE::CString& key ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_section ? CGodotVariant::FindProperty( *m_section, key ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Float64
CGodotMaterialReader::Number( const CORE::CString& key, Float64 fallback ) const
{GUCEF_TRACE;

    return CGodotVariant::GetNumber( Prop( key ), fallback );
}

/*-------------------------------------------------------------------------*/

bool
CGodotMaterialReader::Flag( const CORE::CString& key, bool fallback ) const
{GUCEF_TRACE;

    return CGodotVariant::GetBool( Prop( key ), fallback );
}

/*-------------------------------------------------------------------------*/

Int32
CGodotMaterialReader::Choice( const CORE::CString& key, Int32 fallback ) const
{GUCEF_TRACE;

    return (Int32) CGodotVariant::GetInteger( Prop( key ), fallback );
}

/*-------------------------------------------------------------------------*/

bool
CGodotMaterialReader::ResolveFile( const CGodotDocument& document, const CORE::CDataNode* value, CORE::CString& assetPath, CORE::CString* via )
{GUCEF_TRACE;

    CORE::CString kind;
    CORE::CString id;
    if ( !CGodotVariant::GetReference( value, kind, id ) || "ExtResource" != kind )
        return false;
    CORE::CString type;
    CORE::CString path;
    CORE::CString uid;
    if ( !document.GetExtResourceInfo( id, type, path, uid ) )
        return false;
    if ( GUCEF_NULL != via )
        *via = path.IsNULLOrEmpty() ? uid : path;
    return CGodotPaths::Resolve( m_context, document.GetAssetPath(), path, uid, assetPath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotMaterialReader::TextureFile( const CORE::CString& key )
{GUCEF_TRACE;

    const CORE::CDataNode* value = Prop( key );
    if ( GUCEF_NULL == value || CGodotVariant::IsType( value, "null" ) )
        return CORE::CString::Empty;
    CORE::CString assetPath;
    CORE::CString via;
    if ( ResolveFile( *m_document, value, assetPath, &via ) )
    {
        const CORE::CString vfsPath = m_context.ToVfsPath( assetPath );
        m_scene.AddReference( ASSETCONVERTER::CAssetReference::KindUsesTexture, vfsPath, via );
        return vfsPath;
    }
    CORE::CString kind;
    CORE::CString id;
    if ( CGodotVariant::GetReference( value, kind, id ) )
    {
        m_report.AddWarning( SourceName, m_subject, "the texture of " + key + " is not a file of the pack: the slot stays empty" );
        m_scene.AddUnresolvedReference( ASSETCONVERTER::CAssetReference::KindUsesTexture, via );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotMaterialReader::AddTexture( const CORE::CString& vfsPath, ASSETCONVERTER::TTextureUsage usage )
{GUCEF_TRACE;

    const CORE::CString key = vfsPath + "|" + ASSETCONVERTER::ToString( usage ) + "|" + IMAGE::ToString( m_wrap );
    TIndexMap::const_iterator known = m_textures.find( key );
    if ( known != m_textures.end() )
        return (*known).second;

    ASSETCONVERTER::CAssetTexture texture;
    texture.name = ASSETCONVERTER::CVfsPaths::NameOf( vfsPath );
    texture.sources.push_back( vfsPath );
    texture.usage = usage;
    texture.wrapS = m_wrap;
    texture.wrapT = m_wrap;
    m_scene.textures.push_back( texture );
    m_textures[ key ] = (Int32) m_scene.textures.size() - 1;
    m_scene.AddDependency( vfsPath );
    return (Int32) m_scene.textures.size() - 1;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotMaterialReader::AddDerivedTexture( const CORE::CString::StringVector& sources, const GUCEF::vector< ASSETCONVERTER::CTextureOperation >& operations, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString key = "derived|" + CORE::CString( IMAGE::ToString( m_wrap ) );
    for ( size_t s=0; s<sources.size(); ++s )
        key += "|" + sources[ s ];
    for ( size_t o=0; o<operations.size(); ++o )
        key += "|" + operations[ o ].ToKeyText();
    TIndexMap::const_iterator known = m_textures.find( key );
    if ( known != m_textures.end() )
        return (*known).second;

    ASSETCONVERTER::CAssetTexture texture;
    texture.name = name;
    texture.sources = sources;
    texture.operations = operations;
    texture.usage = ASSETCONVERTER::TEXTUREUSAGE_MASK;
    texture.wrapS = m_wrap;
    texture.wrapT = m_wrap;
    m_scene.textures.push_back( texture );
    m_textures[ key ] = (Int32) m_scene.textures.size() - 1;
    for ( size_t s=0; s<sources.size(); ++s )
        m_scene.AddDependency( sources[ s ] );
    return (Int32) m_scene.textures.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::ApplyUvTransform( ASSETCONVERTER::CAssetTextureReference& reference ) const
{GUCEF_TRACE;

    if ( !reference.IsUsed() || 0 != reference.texCoord )
        return;
    Float32 scale[ 3 ] = { 1.0f, 1.0f, 1.0f };
    Float32 offset[ 3 ] = { 0.0f, 0.0f, 0.0f };
    CGodotVariant::GetVector3( Prop( "uv1_scale" ), scale );
    CGodotVariant::GetVector3( Prop( "uv1_offset" ), offset );
    if ( 1.0f == scale[ 0 ] && 1.0f == scale[ 1 ] && 0.0f == offset[ 0 ] && 0.0f == offset[ 1 ] )
        return;
    reference.hasTransform = true;
    reference.tiling[ 0 ] = scale[ 0 ];
    reference.tiling[ 1 ] = scale[ 1 ];
    reference.offset[ 0 ] = offset[ 0 ];
    reference.offset[ 1 ] = offset[ 1 ];
    reference.rotation = 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::RecordSource( ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    const CORE::CDataNode* properties = m_section->FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        return;
    for ( const CORE::CDataNode* property = properties->GetFirstChild(); GUCEF_NULL != property; property = property->GetNext() )
    {
        CORE::CString assetPath;
        if ( ResolveFile( *m_document, property, assetPath ) )
            material.source.textures.SetAttribute( property->GetName(), m_context.ToVfsPath( assetPath ) );
        else
            CGodotVariant::AddPlain( material.source.properties, property->GetName(), *property );
    }
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::ReadMetallicRoughness( ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    material.metallicFactor = Clamp01( Number( "metallic", 0.0 ) );
    material.roughnessFactor = Clamp01( Number( "roughness", 1.0 ) );

    // R occlusion, G roughness, B metallic: the layout of glTF, used as it is
    const CORE::CString ormFile = TextureFile( "orm_texture" );
    if ( !ormFile.IsNULLOrEmpty() )
    {
        const Int32 index = AddTexture( ormFile, ASSETCONVERTER::TEXTUREUSAGE_MASK );
        material.metallicRoughnessTexture = ASSETCONVERTER::CAssetTextureReference( index );
        material.occlusionTexture = ASSETCONVERTER::CAssetTextureReference( index );
        return;
    }

    const CORE::CString metalFile = TextureFile( "metallic_texture" );
    const CORE::CString roughFile = TextureFile( "roughness_texture" );
    if ( metalFile.IsNULLOrEmpty() && roughFile.IsNULLOrEmpty() )
        return;
    const Int32 metalChannel = Choice( "metallic_texture_channel", 0 );
    const Int32 roughChannel = Choice( "roughness_texture_channel", 0 );
    if ( ( !metalFile.IsNULLOrEmpty() && 4 == metalChannel ) || ( !roughFile.IsNULLOrEmpty() && 4 == roughChannel ) )
        m_report.AddApproximation( SourceName, m_subject, "a grayscale channel of the metallic or roughness texture is read as its red channel" );

    Int32 index = -1;
    if ( !metalFile.IsNULLOrEmpty() && !roughFile.IsNULLOrEmpty() )
    {
        if ( metalFile == roughFile && 1 == roughChannel && 2 == metalChannel )
        {
            index = AddTexture( roughFile, ASSETCONVERTER::TEXTUREUSAGE_MASK );
        }
        else
        {
            CORE::CString::StringVector sources;
            sources.push_back( roughFile );
            Int32 metalSource = 0;
            if ( metalFile != roughFile )
            {
                sources.push_back( metalFile );
                metalSource = 1;
            }
            const Int32 packSources[ 4 ] = { -1, 0, metalSource, -1 };
            const UInt32 packChannels[ 4 ] = { 0, ChannelIndex( roughChannel ), ChannelIndex( metalChannel ), 0 };
            GUCEF::vector< ASSETCONVERTER::CTextureOperation > operations;
            operations.push_back( ASSETCONVERTER::CTextureOperation::Pack( packSources, packChannels ) );
            index = AddDerivedTexture( sources, operations, ASSETCONVERTER::CVfsPaths::StemOf( roughFile ) + "_metallicRoughness" );
        }
    }
    else
    {
        const bool roughOnly = !roughFile.IsNULLOrEmpty();
        const CORE::CString& file = roughOnly ? roughFile : metalFile;
        const CORE::CString letter = ChannelLetter( roughOnly ? roughChannel : metalChannel );
        CORE::CString::StringVector sources;
        sources.push_back( file );
        GUCEF::vector< ASSETCONVERTER::CTextureOperation > operations;
        operations.push_back( ASSETCONVERTER::CTextureOperation::Swizzle( roughOnly ? "1" + letter + "11" : "11" + letter + "1" ) );
        index = AddDerivedTexture( sources, operations, ASSETCONVERTER::CVfsPaths::StemOf( file ) + "_metallicRoughness" );
    }
    if ( index >= 0 )
        material.metallicRoughnessTexture = ASSETCONVERTER::CAssetTextureReference( index );
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::ReadEmission( ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    if ( !Flag( "emission_enabled", false ) )
        return;

    Float32 colour[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    CGodotVariant::GetColour( Prop( "emission" ), colour );
    Float32 factor[ 3 ];
    for ( UInt32 c=0; c<3; ++c )
        factor[ c ] = CGodotVariant::SrgbToLinear( colour[ c ] );
    Float64 strength = Number( "emission_energy_multiplier", 1.0 );

    // a colour above 1 (HDR) is a strength
    const Float32 largest = factor[ 0 ] > factor[ 1 ] ? ( factor[ 0 ] > factor[ 2 ] ? factor[ 0 ] : factor[ 2 ] ) : ( factor[ 1 ] > factor[ 2 ] ? factor[ 1 ] : factor[ 2 ] );
    if ( largest > 1.0f )
    {
        for ( UInt32 c=0; c<3; ++c )
            factor[ c ] /= largest;
        strength *= largest;
    }

    const CORE::CString file = TextureFile( "emission_texture" );
    if ( !file.IsNULLOrEmpty() )
    {
        material.emissiveTexture = ASSETCONVERTER::CAssetTextureReference( AddTexture( file, ASSETCONVERTER::TEXTUREUSAGE_COLOUR ) );
        ApplyUvTransform( material.emissiveTexture );
        const bool black = 0.0f == factor[ 0 ] && 0.0f == factor[ 1 ] && 0.0f == factor[ 2 ];
        if ( 0 == Choice( "emission_operator", 0 ) )
        {
            // the texture is added to the colour in Godot and multiplied with it in glTF
            if ( black )
            {
                factor[ 0 ] = factor[ 1 ] = factor[ 2 ] = 1.0f;
            }
            else
                m_report.AddApproximation( SourceName, m_subject, "the emission colour is added to the emission texture in Godot: it multiplies the texture" );
        }
    }
    for ( UInt32 c=0; c<3; ++c )
        material.emissiveFactor[ c ] = factor[ c ];
    material.emissiveStrength = (Float32) strength;
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::ReadBase( ASSETCONVERTER::CAssetMaterial& material, bool isOrm )
{GUCEF_TRACE;

    // the base colour
    Float32 colour[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CGodotVariant::GetColour( Prop( "albedo_color" ), colour );
    for ( UInt32 c=0; c<3; ++c )
        material.baseColorFactor[ c ] = CGodotVariant::SrgbToLinear( colour[ c ] );
    material.baseColorFactor[ 3 ] = Clamp01( colour[ 3 ] );
    const CORE::CString albedoFile = TextureFile( "albedo_texture" );
    if ( !albedoFile.IsNULLOrEmpty() )
        material.baseColorTexture = ASSETCONVERTER::CAssetTextureReference( AddTexture( albedoFile, ASSETCONVERTER::TEXTUREUSAGE_COLOUR ) );

    // transparency
    const Int32 transparency = Choice( "transparency", 0 );
    switch ( transparency )
    {
        case 1:
        case 4:
            material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
            break;
        case 2:
            material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
            material.alphaCutoff = Clamp01( Number( "alpha_scissor_threshold", 0.5 ) );
            break;
        case 3:
            material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
            material.alphaCutoff = 0.5f;
            m_report.AddApproximation( SourceName, m_subject, "alpha hash transparency is a dither of the alpha in Godot: it is a mask with the cutoff 0.5" );
            break;
        default:
            material.alphaMode = ASSETCONVERTER::ALPHAMODE_OPAQUE;
            break;
    }

    // faces and shading
    const Int32 cull = Choice( "cull_mode", 0 );
    material.doubleSided = 2 == cull;
    if ( 1 == cull )
        m_report.AddApproximation( SourceName, m_subject, "the material culls front faces, which glTF cannot express: back faces are culled" );
    material.unlit = 0 == Choice( "shading_mode", 1 );

    ReadMetallicRoughness( material );

    // the normal map
    if ( Flag( "normal_enabled", false ) )
    {
        const CORE::CString normalFile = TextureFile( "normal_texture" );
        if ( !normalFile.IsNULLOrEmpty() )
        {
            material.normalTexture = ASSETCONVERTER::CAssetTextureReference( AddTexture( normalFile, ASSETCONVERTER::TEXTUREUSAGE_NORMAL ) );
            material.normalTexture.scale = (Float32) Number( "normal_scale", 1.0 );
        }
    }

    // the occlusion
    bool occlusionOnSecondSet = false;
    if ( !isOrm && !material.occlusionTexture.IsUsed() && Flag( "ao_enabled", false ) )
    {
        const CORE::CString aoFile = TextureFile( "ao_texture" );
        if ( !aoFile.IsNULLOrEmpty() )
        {
            const Int32 channel = Choice( "ao_texture_channel", 0 );
            Int32 index = -1;
            if ( 0 == channel || 4 == channel )
                index = AddTexture( aoFile, ASSETCONVERTER::TEXTUREUSAGE_MASK );
            else
            {
                CORE::CString::StringVector sources;
                sources.push_back( aoFile );
                GUCEF::vector< ASSETCONVERTER::CTextureOperation > operations;
                operations.push_back( ASSETCONVERTER::CTextureOperation::Swizzle( CORE::CString( ChannelLetter( channel ) ) + "111" ) );
                index = AddDerivedTexture( sources, operations, ASSETCONVERTER::CVfsPaths::StemOf( aoFile ) + "_occlusion" );
            }
            material.occlusionTexture = ASSETCONVERTER::CAssetTextureReference( index );
            if ( Flag( "ao_on_uv2", false ) )
            {
                material.occlusionTexture.texCoord = 1;
                occlusionOnSecondSet = true;
            }
        }
    }

    ReadEmission( material );

    // the tiling and offset of the first UV set
    ApplyUvTransform( material.baseColorTexture );
    ApplyUvTransform( material.metallicRoughnessTexture );
    ApplyUvTransform( material.normalTexture );
    if ( !occlusionOnSecondSet )
        ApplyUvTransform( material.occlusionTexture );
    if ( Flag( "uv1_triplanar", false ) )
        m_report.AddApproximation( SourceName, m_subject, "triplanar mapping is not carried: the textures use the UV coordinates of the mesh" );

    // clear coat
    if ( Flag( "clearcoat_enabled", false ) )
    {
        CORE::CDataNode* clearcoat = material.extensions.AddChild( "KHR_materials_clearcoat", GUCEF_DATATYPE_OBJECT );
        clearcoat->SetAttribute( "clearcoatFactor", (Float32) Number( "clearcoat", 1.0 ) );
        clearcoat->SetAttribute( "clearcoatRoughnessFactor", (Float32) Number( "clearcoat_roughness", 0.5 ) );
    }

    // what only the sidecar carries
    static const char* const sidecarOnly[] = { "heightmap_enabled", "detail_enabled", "rim_enabled", "anisotropy_enabled", "subsurf_scatter_enabled", "backlight_enabled", "refraction_enabled" };
    for ( UInt32 f=0; f<sizeof( sidecarOnly ) / sizeof( sidecarOnly[ 0 ] ); ++f )
        if ( Flag( sidecarOnly[ f ], false ) )
            m_report.AddInfo( SourceName, m_subject, CORE::CString( sidecarOnly[ f ] ) + " is kept in the sidecar only" );
}

/*-------------------------------------------------------------------------*/

void
CGodotMaterialReader::ReadShader( ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    // the parameters that name a base colour, a texture, a metallic or a roughness
    static const char* const colourNames[] = { "albedo", "albedo_color", "base_color", "basecolor", "color", "tint", "diffuse" };
    static const char* const textureNames[] = { "albedo_texture", "texture_albedo", "albedo_tex", "albedo", "base_color_texture", "base_color", "basecolor", "diffuse_texture", "main_texture", "texture_diffuse" };
    static const char* const normalNames[] = { "normal_texture", "texture_normal", "normal_map", "normalmap", "normal" };

    const CORE::CString prefix = "shader_parameter/";
    bool found = false;
    for ( UInt32 n=0; n<sizeof( colourNames ) / sizeof( colourNames[ 0 ] ); ++n )
    {
        Float32 colour[ 4 ];
        if ( CGodotVariant::GetColour( Prop( prefix + colourNames[ n ] ), colour ) && CGodotVariant::IsType( Prop( prefix + colourNames[ n ] ), "Color" ) )
        {
            for ( UInt32 c=0; c<3; ++c )
                material.baseColorFactor[ c ] = CGodotVariant::SrgbToLinear( colour[ c ] );
            material.baseColorFactor[ 3 ] = Clamp01( colour[ 3 ] );
            found = true;
            break;
        }
    }
    for ( UInt32 n=0; n<sizeof( textureNames ) / sizeof( textureNames[ 0 ] ); ++n )
    {
        const CORE::CString file = TextureFile( prefix + textureNames[ n ] );
        if ( !file.IsNULLOrEmpty() )
        {
            material.baseColorTexture = ASSETCONVERTER::CAssetTextureReference( AddTexture( file, ASSETCONVERTER::TEXTUREUSAGE_COLOUR ) );
            found = true;
            break;
        }
    }
    for ( UInt32 n=0; n<sizeof( normalNames ) / sizeof( normalNames[ 0 ] ); ++n )
    {
        const CORE::CString file = TextureFile( prefix + normalNames[ n ] );
        if ( !file.IsNULLOrEmpty() )
        {
            material.normalTexture = ASSETCONVERTER::CAssetTextureReference( AddTexture( file, ASSETCONVERTER::TEXTUREUSAGE_NORMAL ) );
            break;
        }
    }
    material.metallicFactor = Clamp01( Number( prefix + "metallic", 0.0 ) );
    material.roughnessFactor = Clamp01( Number( prefix + "roughness", 1.0 ) );
    material.unlit = false;

    if ( found )
        m_report.AddApproximation( SourceName, m_subject, "a custom shader: only a base colour and textures found by the names of its parameters are carried, its parameters are in the sidecar" );
    else
        m_report.AddApproximation( SourceName, m_subject, "a custom shader whose parameters name no base colour: the material has the default look, its parameters are in the sidecar" );
}

/*-------------------------------------------------------------------------*/

Int32
CGodotMaterialReader::ReadMaterial( const CGodotDocument& document, const CORE::CDataNode& section )
{GUCEF_TRACE;

    CORE::CString type = section.GetAttributeValue( "type" ).AsString();
    if ( type.IsNULLOrEmpty() )
        type = document.GetResourceType();
    if ( !IsMaterialType( type ) )
        return -1;

    CORE::CString id = section.GetAttributeValue( "id" ).AsString();
    if ( id.IsNULLOrEmpty() )
        id = "resource";
    const CORE::CString key = document.GetAssetPath() + "#" + id;
    TIndexMap::const_iterator known = m_materials.find( key );
    if ( known != m_materials.end() )
        return (*known).second;

    m_document = &document;
    m_section = &section;
    ASSETCONVERTER::CAssetMaterial material;
    material.name = CGodotVariant::GetString( Prop( "resource_name" ) );
    if ( material.name.IsNULLOrEmpty() )
        material.name = "resource" == id ? ASSETCONVERTER::CVfsPaths::StemOf( document.GetAssetPath() ) : id;
    m_subject = material.name;
    m_wrap = Flag( "texture_repeat", true ) ? IMAGE::WRAPMODE_REPEAT : IMAGE::WRAPMODE_CLAMP;
    material.source.shaderName = type;

    if ( "ShaderMaterial" == type )
        ReadShader( material );
    else
        ReadBase( material, "ORMMaterial3D" == type );
    RecordSource( material );

    m_scene.materials.push_back( material );
    const Int32 index = (Int32) m_scene.materials.size() - 1;
    m_materials[ key ] = index;
    m_document = GUCEF_NULL;
    m_section = GUCEF_NULL;
    return index;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
