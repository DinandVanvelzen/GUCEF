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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#include "assetpluginFBX_CFbxMaterialReader.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const SourceName = "fbx";

CORE::CString
ToString( const ufbx_string& text )
{
    return GUCEF_NULL != text.data && text.length > 0 ? CORE::CString( text.data, (UInt32) text.length ) : CORE::CString::Empty;
}

/** The path of a (relative) file name below a folder, with '.' and '..' parts resolved */
CORE::CString
Collapse( const CORE::CString& folder, const CORE::CString& relative )
{
    CORE::CString::StringVector parts = ASSETCONVERTER::CVfsPaths::Normalize( folder ).ParseElements( '/', false );
    const CORE::CString::StringVector more = ASSETCONVERTER::CVfsPaths::Normalize( relative ).ParseElements( '/', false );
    for ( size_t i=0; i<more.size(); ++i )
    {
        if ( "." == more[ i ] )
            continue;
        if ( ".." == more[ i ] )
        {
            if ( !parts.empty() )
                parts.pop_back();
            continue;
        }
        parts.push_back( more[ i ] );
    }
    CORE::CString path;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        if ( i > 0 )
            path += "/";
        path += parts[ i ];
    }
    return path;
}

UInt32
CommonPrefix( const CORE::CString& a, const CORE::CString& b )
{
    const char* x = a.C_String();
    const char* y = b.C_String();
    UInt32 length = 0;
    while ( '\0' != x[ length ] && x[ length ] == y[ length ] )
        ++length;
    return length;
}

Float32
Clamp01( Float32 value )
{
    return value < 0.0f ? 0.0f : ( value > 1.0f ? 1.0f : value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFbxMaterialReader::CFbxMaterialReader( const ufbx_scene& fbx                       ,
                                        const CORE::CString& fbxPath                ,
                                        ASSETCONVERTER::CAssetPackContext& context  ,
                                        ASSETCONVERTER::CAssetScene& scene          ,
                                        ASSETCONVERTER::CConversionReport& report   )
    : m_fbx( fbx )
    , m_fbxPath( fbxPath )
    , m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_textures()
    , m_packFiles()
    , m_packFilesBuilt( false )
    , m_embedded()
    , m_materialName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxMaterialReader::~CFbxMaterialReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::ReadMaterials( void )
{GUCEF_TRACE;

    for ( size_t m=0; m<m_fbx.materials.count; ++m )
    {
        const ufbx_material& source = *m_fbx.materials.data[ m ];
        ASSETCONVERTER::CAssetMaterial material;
        material.name = ToString( source.name );
        m_materialName = material.name;
        ReadMaterial( source, material );
        m_scene.materials.push_back( material );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFbxMaterialReader::ExtractContent( const ufbx_texture& texture )
{GUCEF_TRACE;

    const CORE::CString intermediate = m_context.GetManifest().GetIntermediatePath();
    if ( intermediate.IsNULLOrEmpty() )
    {
        m_report.AddWarning( SourceName, m_materialName, "the texture " + ToString( texture.name ) + " is inside the file and the manifest has no intermediate folder to extract it to" );
        return CORE::CString::Empty;
    }
    CORE::CString assetPath;
    if ( !m_context.ToAssetPath( m_fbxPath, assetPath ) )
        assetPath = ASSETCONVERTER::CVfsPaths::NameOf( m_fbxPath );

    CORE::CString name = ASSETCONVERTER::CVfsPaths::NameOf( ToString( texture.relative_filename ) );
    if ( name.IsNULLOrEmpty() )
        name = ASSETCONVERTER::CVfsPaths::NameOf( ToString( texture.filename ) );
    if ( name.IsNULLOrEmpty() || ASSETCONVERTER::CVfsPaths::ExtensionOf( name ).IsNULLOrEmpty() )
    {
        const UInt8* bytes = static_cast< const UInt8* >( texture.content.data );
        const bool png = texture.content.size >= 4 && 0x89 == bytes[ 0 ] && 'P' == bytes[ 1 ] && 'N' == bytes[ 2 ] && 'G' == bytes[ 3 ];
        const bool jpg = texture.content.size >= 3 && 0xFF == bytes[ 0 ] && 0xD8 == bytes[ 1 ];
        name = "texture" + CORE::UInt32ToString( texture.typed_id ) + ( png ? ".png" : ( jpg ? ".jpg" : ".bin" ) );
    }
    CORE::CString path = ASSETCONVERTER::CVfsPaths::Join( intermediate, "embedded/" + ASSETCONVERTER::CVfsPaths::WithoutExtension( assetPath ) );
    path = ASSETCONVERTER::CVfsPaths::Join( path, name );

    CORE::CDynamicBuffer content( static_cast< const char* >( texture.content.data ), (UInt32) texture.content.size, false );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.CreateDir( ASSETCONVERTER::CVfsPaths::FolderOf( path ) );
    if ( !vfs.StoreAsFile( path, content, 0, true ) )
    {
        m_report.AddWarning( SourceName, m_materialName, "the texture " + name + " inside the file cannot be extracted to " + path );
        return CORE::CString::Empty;
    }
    m_report.AddOutputFile( path, CORE::CString::StringVector() );
    m_embedded.insert( path );
    return path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFbxMaterialReader::ResolveFile( const ufbx_texture& texture )
{GUCEF_TRACE;

    if ( texture.content.size > 0 && GUCEF_NULL != texture.content.data )
        return ExtractContent( texture );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( m_fbxPath );
    const CORE::CString names[ 3 ] = { ToString( texture.relative_filename ), ToString( texture.filename ), ToString( texture.absolute_filename ) };
    for ( UInt32 n=0; n<3; ++n )
    {
        if ( names[ n ].IsNULLOrEmpty() )
            continue;
        const CORE::CString relative = Collapse( folder, names[ n ] );
        if ( vfs.FileExists( relative ) )
            return relative;
        const CORE::CString sibling = ASSETCONVERTER::CVfsPaths::Join( folder, ASSETCONVERTER::CVfsPaths::NameOf( names[ n ] ) );
        if ( vfs.FileExists( sibling ) )
            return sibling;
    }

    // the same file name somewhere in the pack: the nearest to the FBX
    if ( !m_packFilesBuilt )
    {
        m_packFilesBuilt = true;
        const CORE::CString::StringVector& files = m_context.GetFiles();
        for ( size_t f=0; f<files.size(); ++f )
            m_packFiles[ ASSETCONVERTER::CVfsPaths::NameOf( files[ f ] ).Lowercase() ].push_back( m_context.ToVfsPath( files[ f ] ) );
    }
    for ( UInt32 n=0; n<3; ++n )
    {
        if ( names[ n ].IsNULLOrEmpty() )
            continue;
        TFileMap::const_iterator found = m_packFiles.find( ASSETCONVERTER::CVfsPaths::NameOf( names[ n ] ).Lowercase() );
        if ( found == m_packFiles.end() )
            continue;
        CORE::CString best;
        UInt32 bestLength = 0;
        for ( size_t c=0; c<(*found).second.size(); ++c )
        {
            const UInt32 length = CommonPrefix( (*found).second[ c ], folder );
            if ( best.IsNULLOrEmpty() || length > bestLength )
            {
                best = (*found).second[ c ];
                bestLength = length;
            }
        }
        return best;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const ufbx_texture*
CFbxMaterialReader::GetFileTexture( const ufbx_texture* texture ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == texture )
        return GUCEF_NULL;
    if ( UFBX_TEXTURE_FILE == texture->type )
        return texture;
    if ( texture->file_textures.count > 0 )
        return texture->file_textures.data[ 0 ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Int32
CFbxMaterialReader::AddFileTexture( const ufbx_texture& texture, ASSETCONVERTER::TTextureUsage usage, const CORE::CString& owner )
{GUCEF_TRACE;

    const CORE::CString path = ResolveFile( texture );
    if ( path.IsNULLOrEmpty() )
    {
        m_report.AddWarning( SourceName, owner, "the texture file " + ToString( texture.filename ) + " is not in the pack: the slot stays empty" );
        return -1;
    }
    const IMAGE::TWrapMode wrapU = UFBX_WRAP_CLAMP == texture.wrap_u ? IMAGE::WRAPMODE_CLAMP : IMAGE::WRAPMODE_REPEAT;
    const IMAGE::TWrapMode wrapV = UFBX_WRAP_CLAMP == texture.wrap_v ? IMAGE::WRAPMODE_CLAMP : IMAGE::WRAPMODE_REPEAT;
    const CORE::CString key = path + "|" + ASSETCONVERTER::ToString( usage ) + "|" + IMAGE::ToString( wrapU ) + IMAGE::ToString( wrapV );
    TTextureMap::const_iterator known = m_textures.find( key );
    if ( known != m_textures.end() )
        return (*known).second;

    ASSETCONVERTER::CAssetTexture result;
    result.name = ASSETCONVERTER::CVfsPaths::NameOf( path );
    result.sources.push_back( path );
    result.usage = usage;
    result.wrapS = wrapU;
    result.wrapT = wrapV;
    m_scene.textures.push_back( result );
    m_textures[ key ] = (Int32) m_scene.textures.size() - 1;
    if ( m_embedded.find( path ) == m_embedded.end() )
        m_scene.AddDependency( path );
    return (Int32) m_scene.textures.size() - 1;
}

/*-------------------------------------------------------------------------*/

Int32
CFbxMaterialReader::AddDerivedTexture( const CORE::CString::StringVector& sources, const GUCEF::vector< ASSETCONVERTER::CTextureOperation >& operations, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString key = "derived";
    for ( size_t s=0; s<sources.size(); ++s )
        key += "|" + sources[ s ];
    for ( size_t o=0; o<operations.size(); ++o )
        key += "|" + operations[ o ].ToKeyText();
    TTextureMap::const_iterator known = m_textures.find( key );
    if ( known != m_textures.end() )
        return (*known).second;

    ASSETCONVERTER::CAssetTexture result;
    result.name = name;
    result.sources = sources;
    result.operations = operations;
    result.usage = ASSETCONVERTER::TEXTUREUSAGE_MASK;
    m_scene.textures.push_back( result );
    m_textures[ key ] = (Int32) m_scene.textures.size() - 1;
    for ( size_t s=0; s<sources.size(); ++s )
        if ( m_embedded.find( sources[ s ] ) == m_embedded.end() )
            m_scene.AddDependency( sources[ s ] );
    return (Int32) m_scene.textures.size() - 1;
}

/*-------------------------------------------------------------------------*/

bool
CFbxMaterialReader::SetReference( const ufbx_material_map& map, ASSETCONVERTER::TTextureUsage usage, ASSETCONVERTER::CAssetTextureReference& reference, const CORE::CString& owner )
{GUCEF_TRACE;

    const ufbx_texture* texture = GetFileTexture( map.texture );
    if ( GUCEF_NULL == texture || !map.texture_enabled )
        return false;
    const Int32 index = AddFileTexture( *texture, usage, owner );
    if ( index < 0 )
        return false;
    reference.textureIndex = index;
    reference.texCoord = 0;

    // the transform of the texture coordinates, in the V up space of FBX, as a transform of the V down space of glTF
    if ( map.texture->has_uv_transform )
    {
        const ufbx_matrix& m = map.texture->uv_to_texture;
        const Float32 a = (Float32) m.m00, b = (Float32) m.m01, c = (Float32) m.m03;
        const Float32 d = (Float32) m.m10, e = (Float32) m.m11, f = (Float32) m.m13;
        if ( fabs( b ) > 1e-5 || fabs( d ) > 1e-5 )
        {
            m_report.AddApproximation( SourceName, owner, "the texture " + ToString( texture->name ) + " has a rotated UV transform: it is not carried" );
        }
        else
        if ( fabs( a - 1.0f ) > 1e-6 || fabs( e - 1.0f ) > 1e-6 || fabs( c ) > 1e-6 || fabs( f ) > 1e-6 )
        {
            reference.hasTransform = true;
            reference.tiling[ 0 ] = a;
            reference.tiling[ 1 ] = e;
            reference.offset[ 0 ] = c;
            reference.offset[ 1 ] = 1.0f - e - f;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFbxMaterialReader::GetTextureFile( const ufbx_material_map& map )
{GUCEF_TRACE;

    const ufbx_texture* texture = GetFileTexture( map.texture );
    if ( GUCEF_NULL == texture || !map.texture_enabled )
        return CORE::CString::Empty;
    const CORE::CString path = ResolveFile( *texture );
    if ( path.IsNULLOrEmpty() )
        m_report.AddWarning( SourceName, m_materialName, "the texture file " + ToString( texture->filename ) + " is not in the pack: the slot stays empty" );
    return path;
}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::AddPackedMetalRoughness( const ufbx_material_map& roughness, const ufbx_material_map& metalness, bool roughnessIsGlossiness, ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    const CORE::CString roughnessFile = GetTextureFile( roughness );
    const CORE::CString metalnessFile = GetTextureFile( metalness );
    if ( roughnessFile.IsNULLOrEmpty() && metalnessFile.IsNULLOrEmpty() )
        return;

    // glTF: G roughness, B metallic. A missing channel is 1, so the factor of the material decides.
    CORE::CString::StringVector sources;
    GUCEF::vector< ASSETCONVERTER::CTextureOperation > operations;
    if ( !roughnessFile.IsNULLOrEmpty() && !metalnessFile.IsNULLOrEmpty() )
    {
        sources.push_back( roughnessFile );
        sources.push_back( metalnessFile );
        const Int32 packSources[ 4 ] = { -1, 0, 1, -1 };
        const UInt32 packChannels[ 4 ] = { 0, 0, 0, 0 };
        operations.push_back( ASSETCONVERTER::CTextureOperation::Pack( packSources, packChannels ) );
    }
    else
    if ( !roughnessFile.IsNULLOrEmpty() )
    {
        sources.push_back( roughnessFile );
        operations.push_back( ASSETCONVERTER::CTextureOperation::Swizzle( "1R11" ) );
    }
    else
    {
        sources.push_back( metalnessFile );
        operations.push_back( ASSETCONVERTER::CTextureOperation::Swizzle( "11R1" ) );
    }
    if ( roughnessIsGlossiness && !roughnessFile.IsNULLOrEmpty() )
        operations.push_back( ASSETCONVERTER::CTextureOperation::Invert( 2 ) );

    const Int32 index = AddDerivedTexture( sources, operations, ASSETCONVERTER::CVfsPaths::StemOf( sources[ 0 ] ) + "_metallicRoughness" );
    if ( index >= 0 )
        material.metallicRoughnessTexture = ASSETCONVERTER::CAssetTextureReference( index );
}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::ReadPbr( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    const ufbx_material_pbr_maps& pbr = source.pbr;

    if ( pbr.base_color.has_value )
    {
        material.baseColorFactor[ 0 ] = (Float32) pbr.base_color.value_vec3.x;
        material.baseColorFactor[ 1 ] = (Float32) pbr.base_color.value_vec3.y;
        material.baseColorFactor[ 2 ] = (Float32) pbr.base_color.value_vec3.z;
        if ( pbr.base_color.value_components >= 4 )
            material.baseColorFactor[ 3 ] = (Float32) pbr.base_color.value_vec4.w;
    }
    if ( pbr.base_factor.has_value )
        for ( UInt32 c=0; c<3; ++c )
            material.baseColorFactor[ c ] *= (Float32) pbr.base_factor.value_real;
    SetReference( pbr.base_color, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, material.baseColorTexture, material.name );

    material.metallicFactor = pbr.metalness.has_value ? (Float32) pbr.metalness.value_real : 0.0f;
    const bool glossiness = source.features.roughness_as_glossiness.enabled;
    if ( pbr.roughness.has_value )
        material.roughnessFactor = glossiness ? 1.0f - (Float32) pbr.roughness.value_real : (Float32) pbr.roughness.value_real;
    else
        material.roughnessFactor = 1.0f;
    material.metallicFactor = Clamp01( material.metallicFactor );
    material.roughnessFactor = Clamp01( material.roughnessFactor );
    AddPackedMetalRoughness( pbr.roughness, pbr.metalness, glossiness, material );

    SetReference( pbr.normal_map, ASSETCONVERTER::TEXTUREUSAGE_NORMAL, material.normalTexture, material.name );
    SetReference( pbr.ambient_occlusion, ASSETCONVERTER::TEXTUREUSAGE_MASK, material.occlusionTexture, material.name );

    if ( pbr.emission_color.has_value )
    {
        const Float32 factor = pbr.emission_factor.has_value ? (Float32) pbr.emission_factor.value_real : 1.0f;
        material.emissiveFactor[ 0 ] = Clamp01( (Float32) pbr.emission_color.value_vec3.x * factor );
        material.emissiveFactor[ 1 ] = Clamp01( (Float32) pbr.emission_color.value_vec3.y * factor );
        material.emissiveFactor[ 2 ] = Clamp01( (Float32) pbr.emission_color.value_vec3.z * factor );
    }
    SetReference( pbr.emission_color, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, material.emissiveTexture, material.name );

    if ( pbr.opacity.has_value && pbr.opacity.value_real < 1.0 )
    {
        material.baseColorFactor[ 3 ] = Clamp01( (Float32) pbr.opacity.value_real );
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    }
    if ( pbr.opacity.texture != GUCEF_NULL && pbr.opacity.texture_enabled )
        m_report.AddApproximation( SourceName, material.name, "the opacity texture is not converted: the alpha of the base colour texture decides" );
}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::ReadStandard( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    const ufbx_material_fbx_maps& fbx = source.fbx;

    if ( fbx.diffuse_color.has_value )
    {
        const Float32 factor = fbx.diffuse_factor.has_value ? (Float32) fbx.diffuse_factor.value_real : 1.0f;
        material.baseColorFactor[ 0 ] = Clamp01( (Float32) fbx.diffuse_color.value_vec3.x * factor );
        material.baseColorFactor[ 1 ] = Clamp01( (Float32) fbx.diffuse_color.value_vec3.y * factor );
        material.baseColorFactor[ 2 ] = Clamp01( (Float32) fbx.diffuse_color.value_vec3.z * factor );
    }
    SetReference( fbx.diffuse_color, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, material.baseColorTexture, material.name );

    // opacity: the opacity property when the file has one, else one minus the transparency colour (black is opaque) times its factor
    Float32 opacity = 1.0f;
    if ( source.pbr.opacity.has_value )
    {
        opacity = (Float32) source.pbr.opacity.value_real;
    }
    else
    if ( fbx.transparency_factor.has_value )
    {
        const Float32 tint = fbx.transparency_color.has_value ? (Float32) ( fbx.transparency_color.value_vec3.x + fbx.transparency_color.value_vec3.y + fbx.transparency_color.value_vec3.z ) / 3.0f : 1.0f;
        opacity = 1.0f - (Float32) fbx.transparency_factor.value_real * tint;
    }
    if ( opacity < 1.0f )
    {
        material.baseColorFactor[ 3 ] = Clamp01( opacity );
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    }

    // no metal in the FBX standard materials; the shininess gives an approximate roughness of the Blinn-Phong lobe
    material.metallicFactor = 0.0f;
    if ( fbx.specular_exponent.has_value && fbx.specular_exponent.value_real > 0.0 )
    {
        material.roughnessFactor = Clamp01( (Float32) sqrt( 2.0 / ( fbx.specular_exponent.value_real + 2.0 ) ) );
        m_report.AddApproximation( SourceName, material.name, "roughness from the specular exponent of the FBX material (sqrt( 2 / ( exponent + 2 ) ))" );
    }
    else
    {
        material.roughnessFactor = 1.0f;
    }

    if ( fbx.emission_color.has_value )
    {
        const Float32 factor = fbx.emission_factor.has_value ? (Float32) fbx.emission_factor.value_real : 1.0f;
        material.emissiveFactor[ 0 ] = Clamp01( (Float32) fbx.emission_color.value_vec3.x * factor );
        material.emissiveFactor[ 1 ] = Clamp01( (Float32) fbx.emission_color.value_vec3.y * factor );
        material.emissiveFactor[ 2 ] = Clamp01( (Float32) fbx.emission_color.value_vec3.z * factor );
    }
    SetReference( fbx.emission_color, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, material.emissiveTexture, material.name );

    if ( !SetReference( fbx.normal_map, ASSETCONVERTER::TEXTUREUSAGE_NORMAL, material.normalTexture, material.name ) &&
         GUCEF_NULL != fbx.bump.texture && fbx.bump.texture_enabled )
        m_report.AddSkipped( SourceName, material.name, "the bump map is a height map, which is not converted to a normal map" );
    if ( GUCEF_NULL != fbx.displacement.texture || GUCEF_NULL != fbx.vector_displacement.texture )
        m_report.AddSkipped( SourceName, material.name, "displacement maps are not converted" );
}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::ReadSourceMaterial( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    static const char* const shaderNames[] = { "unknown", "FBX Lambert", "FBX Phong", "OSL standard surface", "Arnold standard surface", "3ds Max physical material",
                                               "3ds Max PBR (metal/rough)", "3ds Max PBR (spec/gloss)", "glTF", "OpenPBR", "ShaderFX graph", "Blender Phong", "Wavefront MTL" };
    const UInt32 shaderType = (UInt32) source.shader_type;
    material.source.shaderName = "FBX/" + ( shaderType < sizeof( shaderNames ) / sizeof( shaderNames[ 0 ] ) ? CORE::CString( shaderNames[ shaderType ] ) : CORE::CString( "unknown" ) );
    CORE::CDataNode& properties = material.source.properties;
    const CORE::CString model = ToString( source.shading_model_name );
    if ( !model.IsNULLOrEmpty() )
        properties.SetAttribute( "shadingModel", model );
    properties.SetAttribute( "pbr", source.features.pbr.enabled );

    for ( size_t p=0; p<source.props.props.count; ++p )
    {
        const ufbx_prop& prop = source.props.props.data[ p ];
        const CORE::CString name = ToString( prop.name );
        if ( name.IsNULLOrEmpty() )
            continue;
        switch ( prop.type )
        {
            case UFBX_PROP_BOOLEAN : properties.SetAttribute( name, prop.value_int != 0 ); break;
            case UFBX_PROP_INTEGER : properties.SetAttribute( name, (Int64) prop.value_int ); break;
            case UFBX_PROP_NUMBER :
            case UFBX_PROP_DISTANCE : properties.SetAttribute( name, (Float32) prop.value_real ); break;
            case UFBX_PROP_STRING : properties.SetAttribute( name, ToString( prop.value_str ) ); break;
            case UFBX_PROP_VECTOR :
            case UFBX_PROP_COLOR :
            case UFBX_PROP_COLOR_WITH_ALPHA :
            case UFBX_PROP_TRANSLATION :
            case UFBX_PROP_ROTATION :
            case UFBX_PROP_SCALING :
            {
                CORE::CDataNode::TVariantVector values;
                const UInt32 count = UFBX_PROP_COLOR_WITH_ALPHA == prop.type ? 4 : 3;
                for ( UInt32 c=0; c<count; ++c )
                    values.push_back( CORE::CVariant( (Float32) prop.value_real_arr[ c ] ) );
                properties.SetValuesOfChildByName( name, values );
                break;
            }
            default : break;
        }
    }

    for ( size_t t=0; t<source.textures.count; ++t )
    {
        const ufbx_material_texture& link = source.textures.data[ t ];
        const ufbx_texture* file = GetFileTexture( link.texture );
        CORE::CString property = ToString( link.material_prop );
        if ( property.IsNULLOrEmpty() )
            property = ToString( link.shader_prop );
        if ( GUCEF_NULL == file || property.IsNULLOrEmpty() )
            continue;
        CORE::CDataNode* slot = material.source.textures.AddChild( property, GUCEF_DATATYPE_OBJECT );
        slot->SetAttribute( "file", ToString( file->filename ) );
        slot->SetAttribute( "relativeFile", ToString( file->relative_filename ) );
        slot->SetAttribute( "embedded", file->content.size > 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CFbxMaterialReader::ReadMaterial( const ufbx_material& source, ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    if ( source.features.pbr.enabled )
        ReadPbr( source, material );
    else
        ReadStandard( source, material );
    material.doubleSided = source.features.double_sided.enabled;
    material.unlit = source.features.unlit.enabled;
    ReadSourceMaterial( source, material );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
