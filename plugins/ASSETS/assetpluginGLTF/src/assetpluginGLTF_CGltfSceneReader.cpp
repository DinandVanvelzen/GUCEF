/*
 *  assetpluginGLTF: reads a glTF or GLB file into a scene of the converter model
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

#include <string.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H
#include "assetpluginGLTF_CGltfUri.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H ? */

#include "assetpluginGLTF_CGltfSceneReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const SourceName = "gltf";

/** The element of an array member of the JSON tree; NULL when it is not there */
const CORE::CDataNode*
GetElement( const CORE::CDataNode* parent, const char* name, size_t index )
{
    if ( GUCEF_NULL == parent )
        return GUCEF_NULL;
    const CORE::CDataNode* array = parent->FindChild( name );
    if ( GUCEF_NULL == array )
        return GUCEF_NULL;
    return array->GetChildAtIndex( (UInt32) index );
}

const CORE::CDataNode*
GetMember( const CORE::CDataNode* parent, const char* name )
{
    return GUCEF_NULL == parent ? GUCEF_NULL : parent->FindChild( name );
}

CORE::CVariant
GetValue( const CORE::CDataNode* parent, const char* name )
{
    if ( GUCEF_NULL == parent )
        return CORE::CVariant();
    return parent->GetAttributeValueOrChildValueByName( name );
}

/** The numbers of an array member of the JSON tree */
void
GetNumbers( const CORE::CDataNode* parent, const char* name, GUCEF::vector< Float32 >& numbers )
{
    numbers.clear();
    const CORE::CDataNode* array = GetMember( parent, name );
    if ( GUCEF_NULL == array )
        return;
    CORE::CDataNode::const_iterator i = array->ConstBegin();
    while ( i != array->ConstEnd() )
    {
        numbers.push_back( (*i)->GetValue().AsFloat32() );
        ++i;
    }
}

CORE::CString
GetExtensionOfMime( const CORE::CString& mime, const UInt8* bytes, size_t size )
{
    if ( mime == "image/png" ) return "png";
    if ( mime == "image/jpeg" || mime == "image/jpg" ) return "jpg";
    if ( mime == "image/webp" ) return "webp";
    if ( mime == "image/ktx2" ) return "ktx2";
    if ( mime == "image/vnd-ms.dds" ) return "dds";
    if ( mime == "image/gif" ) return "gif";
    if ( size >= 4 && 0x89 == bytes[ 0 ] && 'P' == bytes[ 1 ] && 'N' == bytes[ 2 ] && 'G' == bytes[ 3 ] ) return "png";
    if ( size >= 3 && 0xFF == bytes[ 0 ] && 0xD8 == bytes[ 1 ] ) return "jpg";
    return "bin";
}

IMAGE::TWrapMode
GetWrapMode( cgltf_int mode )
{
    switch ( mode )
    {
        case cgltf_wrap_mode_clamp_to_edge : return IMAGE::WRAPMODE_CLAMP;
        case cgltf_wrap_mode_mirrored_repeat : return IMAGE::WRAPMODE_MIRROR;
        default : return IMAGE::WRAPMODE_REPEAT;
    }
}

bool
IsSupportedRequiredExtension( const char* name )
{
    static const char* const supported[] =
    {
        "KHR_texture_transform", "KHR_materials_emissive_strength", "KHR_materials_unlit", "KHR_lights_punctual", "KHR_mesh_quantization",
        "KHR_materials_clearcoat", "KHR_materials_specular", "KHR_materials_ior", "KHR_materials_sheen", "KHR_materials_transmission",
        "KHR_materials_volume", "KHR_materials_pbrSpecularGlossiness", "MSFT_lod", "OMI_physics_shape", "OMI_physics_body"
    };
    for ( size_t i=0; i<sizeof( supported ) / sizeof( supported[ 0 ] ); ++i )
        if ( 0 == strcmp( supported[ i ], name ) )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfSceneReader::CGltfSceneReader( const CORE::CString& vfsPath                ,
                                    ASSETCONVERTER::CAssetPackContext& context  ,
                                    ASSETCONVERTER::CAssetScene& scene          ,
                                    ASSETCONVERTER::CConversionReport& report   )
    : m_vfsPath( vfsPath )
    , m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_document()
    , m_json()
    , m_hasJson( false )
    , m_textureUsageFixed()
    , m_embedded()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfSceneReader::~CGltfSceneReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneReader::ReadFloats( const cgltf_accessor* accessor, UInt32 components, TFloatVector& values ) const
{GUCEF_TRACE;

    values.clear();
    if ( GUCEF_NULL == accessor || cgltf_num_components( accessor->type ) != components )
        return false;
    const size_t total = accessor->count * components;
    values.resize( total );
    if ( 0 == total )
        return true;
    return cgltf_accessor_unpack_floats( accessor, &values[ 0 ], total ) == total;
}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneReader::Read( void )
{GUCEF_TRACE;

    CORE::CString problem;
    if ( !m_document.Load( m_vfsPath, problem ) || !m_document.LoadBuffers( problem ) )
    {
        m_report.AddError( SourceName, m_vfsPath, problem );
        return false;
    }
    if ( !m_document.Validate( problem ) )
    {
        m_report.AddError( SourceName, m_vfsPath, problem );
        return false;
    }
    m_hasJson = ASSETCONVERTER::CCanonicalJson::Parse( m_document.GetJson(), m_json );
    if ( !m_hasJson )
        m_report.AddWarning( SourceName, m_vfsPath, "the JSON of the file cannot be read as a data tree: the data of extensions and the extras of nodes are not kept" );

    ReadAsset();
    ReadTextures();
    ReadMaterials();
    ReadMeshes();
    ReadLights();
    ReadNodes();
    ReadSkins();
    ReadAnimations();
    ReadLodGroups();
    ReadColliders();
    ReadDependencies();

    m_report.AddInfo( SourceName, m_vfsPath, "read " + CORE::UInt32ToString( (UInt32) m_scene.nodes.size() ) + " nodes, " + CORE::UInt32ToString( (UInt32) m_scene.meshes.size() ) +
                      " meshes, " + CORE::UInt32ToString( (UInt32) m_scene.materials.size() ) + " materials, " + CORE::UInt32ToString( (UInt32) m_scene.animations.size() ) + " animations" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadAsset( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    if ( m_scene.provenance.sourceFormat.IsNULLOrEmpty() )
        m_scene.provenance.sourceFormat = cgltf_file_type_glb == data->file_type ? "glb" : "gltf";
    if ( GUCEF_NULL != data->asset.version )
        m_scene.provenance.sourceFormatVersion = data->asset.version;
    if ( GUCEF_NULL != data->asset.generator )
        m_scene.provenance.sourceEngine = data->asset.generator;
    m_scene.coordinateSystem = MATH::CCoordinateSystem::GucefDefault();
    m_scene.normalMapConvention = ASSETCONVERTER::NORMALMAPCONVENTION_OPENGL;

    // the facts of the source that the model has no place for go to the extras, and with them to the sidecar
    CORE::CDataNode* facts = m_scene.extras.AddChild( "gltf", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != data->asset.version )
        facts->SetAttribute( "version", CORE::CString( data->asset.version ) );
    if ( GUCEF_NULL != data->asset.generator )
        facts->SetAttribute( "generator", CORE::CString( data->asset.generator ) );
    if ( GUCEF_NULL != data->asset.copyright )
        facts->SetAttribute( "copyright", CORE::CString( data->asset.copyright ) );
    CORE::CDataNode::TVariantVector used;
    for ( cgltf_size i=0; i<data->extensions_used_count; ++i )
        used.push_back( CORE::CVariant( CORE::CString( data->extensions_used[ i ] ) ) );
    if ( !used.empty() )
        facts->SetValuesOfChildByName( "extensionsUsed", used );
    CORE::CDataNode::TVariantVector required;
    for ( cgltf_size i=0; i<data->extensions_required_count; ++i )
    {
        required.push_back( CORE::CVariant( CORE::CString( data->extensions_required[ i ] ) ) );
        if ( !IsSupportedRequiredExtension( data->extensions_required[ i ] ) )
            m_report.AddWarning( SourceName, m_vfsPath, CORE::CString( "the file requires the extension " ) + data->extensions_required[ i ] + ", which is not supported: what depends on it is skipped" );
    }
    if ( !required.empty() )
        facts->SetValuesOfChildByName( "extensionsRequired", required );
    if ( data->cameras_count > 0 )
        m_report.AddSkipped( SourceName, m_vfsPath, CORE::UInt32ToString( (UInt32) data->cameras_count ) + " camera(s) are not part of the model" );
    if ( data->scenes_count > 1 )
        m_report.AddApproximation( SourceName, m_vfsPath, "the file has " + CORE::UInt32ToString( (UInt32) data->scenes_count ) + " scenes: their root nodes are all roots of the scene" );

    // the extras of the scene of the file are the extras of the scene
    const CORE::CDataNode* scene = GetElement( &m_json, "scenes", 0 );
    const CORE::CDataNode* sceneExtras = GetMember( scene, "extras" );
    if ( m_hasJson && GUCEF_NULL != sceneExtras )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = sceneExtras->AttributeBegin();
        while ( a != sceneExtras->AttributeEnd() )
        {
            m_scene.extras.SetAttribute( (*a).first, (*a).second );
            ++a;
        }
        CORE::CDataNode::const_iterator c = sceneExtras->ConstBegin();
        while ( c != sceneExtras->ConstEnd() )
        {
            m_scene.extras.DelChild( (*c)->GetName() );
            m_scene.extras.AddChild( *(*c) );
            ++c;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfSceneReader::ExtractEmbeddedImage( const cgltf_image& image, UInt32 index )
{GUCEF_TRACE;

    const UInt8* bytes = GUCEF_NULL;
    size_t size = 0;
    GUCEF::vector< UInt8 > decoded;
    CORE::CString mime = GUCEF_NULL != image.mime_type ? CORE::CString( image.mime_type ) : CORE::CString::Empty;

    if ( GUCEF_NULL != image.buffer_view )
    {
        bytes = reinterpret_cast< const UInt8* >( cgltf_buffer_view_data( image.buffer_view ) );
        size = image.buffer_view->size;
    }
    else
    if ( GUCEF_NULL != image.uri )
    {
        const char* text = image.uri;
        const char* comma = strchr( text, ',' );
        if ( GUCEF_NULL == comma || 0 != strncmp( text, "data:", 5 ) )
            return CORE::CString::Empty;
        const CORE::CString header( text, (UInt32) ( comma - text ) );
        if ( mime.IsNULLOrEmpty() )
        {
            const char* semicolon = strchr( text, ';' );
            const char* end = GUCEF_NULL != semicolon && semicolon < comma ? semicolon : comma;
            mime = CORE::CString( text + 5, (UInt32) ( end - ( text + 5 ) ) );
        }
        if ( header.HasSubstr( ";base64" ) < 0 )
            return CORE::CString::Empty;
        const char* base64 = comma + 1;
        const size_t length = strlen( base64 );
        size_t padding = 0;
        while ( padding < length && padding < 2 && '=' == base64[ length - 1 - padding ] )
            ++padding;
        const size_t decodedSize = ( length / 4 ) * 3 - padding;
        cgltf_options options;
        memset( &options, 0, sizeof( options ) );
        void* out = GUCEF_NULL;
        if ( cgltf_result_success != cgltf_load_buffer_base64( &options, decodedSize, base64, &out ) || GUCEF_NULL == out )
            return CORE::CString::Empty;
        decoded.assign( static_cast< UInt8* >( out ), static_cast< UInt8* >( out ) + decodedSize );
        free( out );
        bytes = decoded.empty() ? GUCEF_NULL : &decoded[ 0 ];
        size = decoded.size();
    }
    if ( GUCEF_NULL == bytes || 0 == size )
        return CORE::CString::Empty;

    const CORE::CString intermediate = m_context.GetManifest().GetIntermediatePath();
    if ( intermediate.IsNULLOrEmpty() )
    {
        m_report.AddWarning( SourceName, m_vfsPath, "image " + CORE::UInt32ToString( index ) + " is inside the file and the manifest has no intermediate folder to extract it to" );
        return CORE::CString::Empty;
    }
    CORE::CString assetPath;
    if ( !m_context.ToAssetPath( m_vfsPath, assetPath ) )
        assetPath = ASSETCONVERTER::CVfsPaths::NameOf( m_vfsPath );
    const CORE::CString extension = GetExtensionOfMime( mime, bytes, size );
    const CORE::CString baseName = ( GUCEF_NULL != image.name && '\0' != image.name[ 0 ] ) ? ASSETCONVERTER::CVfsPaths::NameOf( CORE::CString( image.name ) ) : "image" + CORE::UInt32ToString( index );
    CORE::CString path = ASSETCONVERTER::CVfsPaths::Join( intermediate, "embedded/" + ASSETCONVERTER::CVfsPaths::WithoutExtension( assetPath ) );
    path = ASSETCONVERTER::CVfsPaths::Join( path, CORE::UInt32ToString( index ) + "_" + baseName + "." + extension );

    CORE::CDynamicBuffer content( (const char*) bytes, (UInt32) size, false );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.CreateDir( ASSETCONVERTER::CVfsPaths::FolderOf( path ) );
    if ( !vfs.StoreAsFile( path, content, 0, true ) )
    {
        m_report.AddWarning( SourceName, m_vfsPath, "image " + CORE::UInt32ToString( index ) + " cannot be extracted to " + path );
        return CORE::CString::Empty;
    }
    m_report.AddOutputFile( path, CORE::CString::StringVector() );
    m_embedded.insert( path );
    return path;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadTextures( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    const CORE::CString folder = m_document.GetFolder();
    for ( cgltf_size t=0; t<data->textures_count; ++t )
    {
        const cgltf_texture& source = data->textures[ t ];
        ASSETCONVERTER::CAssetTexture texture;
        const cgltf_image* image = source.image;
        if ( GUCEF_NULL == image && source.has_basisu )
            image = source.basisu_image;
        if ( GUCEF_NULL == image && source.has_webp )
            image = source.webp_image;

        if ( GUCEF_NULL != source.name )
            texture.name = source.name;
        else
        if ( GUCEF_NULL != image && GUCEF_NULL != image->name )
            texture.name = image->name;
        else
            texture.name = "texture" + CORE::UInt32ToString( (UInt32) t );

        if ( GUCEF_NULL == image )
        {
            texture.missing = true;
            m_report.AddSkipped( SourceName, texture.name, "the texture has no image this converter reads" );
        }
        else
        if ( GUCEF_NULL != image->uri && !CGltfUri::IsDataUri( image->uri ) )
        {
            texture.sources.push_back( CGltfUri::Resolve( folder, image->uri ) );
        }
        else
        {
            const CORE::CString path = ExtractEmbeddedImage( *image, (UInt32) cgltf_image_index( data, image ) );
            if ( path.IsNULLOrEmpty() )
            {
                texture.missing = true;
                m_report.AddSkipped( SourceName, texture.name, "the image inside the file cannot be used" );
            }
            else
            {
                texture.sources.push_back( path );
            }
        }
        if ( GUCEF_NULL != source.sampler )
        {
            texture.wrapS = GetWrapMode( source.sampler->wrap_s );
            texture.wrapT = GetWrapMode( source.sampler->wrap_t );
        }

        // the usage the writer put in the extras; otherwise it follows from the slots that use the texture
        bool fixed = false;
        const CORE::CDataNode* extras = GetMember( GetElement( &m_json, "textures", t ), "extras" );
        if ( GUCEF_NULL != extras )
        {
            ASSETCONVERTER::TTextureUsage usage = ASSETCONVERTER::TEXTUREUSAGE_COLOUR;
            if ( ASSETCONVERTER::FromString( GetValue( extras, "usage" ).AsString(), usage ) )
            {
                texture.usage = usage;
                fixed = true;
            }
        }
        m_textureUsageFixed.push_back( fixed );
        m_scene.textures.push_back( texture );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadTextureReference( const cgltf_texture_view& view, ASSETCONVERTER::CAssetTextureReference& reference, ASSETCONVERTER::TTextureUsage usage )
{GUCEF_TRACE;

    if ( GUCEF_NULL == view.texture )
        return;
    const Int32 index = (Int32) cgltf_texture_index( m_document.GetData(), view.texture );
    reference.textureIndex = index;
    reference.texCoord = (UInt32) view.texcoord;
    reference.scale = view.scale;
    if ( view.has_transform )
    {
        reference.hasTransform = true;
        reference.offset[ 0 ] = view.transform.offset[ 0 ];
        reference.offset[ 1 ] = view.transform.offset[ 1 ];
        reference.tiling[ 0 ] = view.transform.scale[ 0 ];
        reference.tiling[ 1 ] = view.transform.scale[ 1 ];
        reference.rotation = view.transform.rotation;
        if ( view.transform.has_texcoord )
            reference.texCoord = (UInt32) view.transform.texcoord;
    }
    if ( (size_t) index < m_scene.textures.size() && !m_textureUsageFixed[ index ] )
    {
        m_scene.textures[ index ].usage = usage;
        m_textureUsageFixed[ index ] = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadMaterials( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    for ( cgltf_size m=0; m<data->materials_count; ++m )
    {
        const cgltf_material& source = data->materials[ m ];
        ASSETCONVERTER::CAssetMaterial material;
        material.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : CORE::CString::Empty;

        if ( source.has_pbr_metallic_roughness )
        {
            const cgltf_pbr_metallic_roughness& pbr = source.pbr_metallic_roughness;
            for ( UInt32 c=0; c<4; ++c )
                material.baseColorFactor[ c ] = pbr.base_color_factor[ c ];
            material.metallicFactor = pbr.metallic_factor;
            material.roughnessFactor = pbr.roughness_factor;
            ReadTextureReference( pbr.base_color_texture, material.baseColorTexture, ASSETCONVERTER::TEXTUREUSAGE_COLOUR );
            ReadTextureReference( pbr.metallic_roughness_texture, material.metallicRoughnessTexture, ASSETCONVERTER::TEXTUREUSAGE_MASK );
        }
        else
        if ( source.has_pbr_specular_glossiness )
        {
            const cgltf_pbr_specular_glossiness& pbr = source.pbr_specular_glossiness;
            for ( UInt32 c=0; c<4; ++c )
                material.baseColorFactor[ c ] = pbr.diffuse_factor[ c ];
            material.metallicFactor = 0.0f;
            material.roughnessFactor = 1.0f - pbr.glossiness_factor;
            ReadTextureReference( pbr.diffuse_texture, material.baseColorTexture, ASSETCONVERTER::TEXTUREUSAGE_COLOUR );
            m_report.AddApproximation( SourceName, material.name, "specular-glossiness material: the diffuse colour is the base colour, the roughness is one minus the glossiness factor and the specular values and textures are not used" );
        }
        ReadTextureReference( source.normal_texture, material.normalTexture, ASSETCONVERTER::TEXTUREUSAGE_NORMAL );
        ReadTextureReference( source.occlusion_texture, material.occlusionTexture, ASSETCONVERTER::TEXTUREUSAGE_MASK );
        ReadTextureReference( source.emissive_texture, material.emissiveTexture, ASSETCONVERTER::TEXTUREUSAGE_COLOUR );
        for ( UInt32 c=0; c<3; ++c )
            material.emissiveFactor[ c ] = source.emissive_factor[ c ];
        if ( source.has_emissive_strength )
            material.emissiveStrength = source.emissive_strength.emissive_strength;
        switch ( source.alpha_mode )
        {
            case cgltf_alpha_mode_mask : material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK; break;
            case cgltf_alpha_mode_blend : material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND; break;
            default : material.alphaMode = ASSETCONVERTER::ALPHAMODE_OPAQUE; break;
        }
        material.alphaCutoff = source.alpha_cutoff;
        material.doubleSided = 0 != source.double_sided;
        material.unlit = 0 != source.unlit;

        // the data of the other extensions, as the file has it
        const CORE::CDataNode* extensions = GetMember( GetElement( &m_json, "materials", m ), "extensions" );
        if ( GUCEF_NULL != extensions )
        {
            CORE::CDataNode::const_iterator e = extensions->ConstBegin();
            while ( e != extensions->ConstEnd() )
            {
                const CORE::CString& name = (*e)->GetName();
                if ( name != "KHR_materials_emissive_strength" && name != "KHR_materials_unlit" && name != "KHR_texture_transform" )
                {
                    CORE::CDataNode* copy = material.extensions.AddChild( *(*e) );
                    copy->SetName( name );
                }
                ++e;
            }
        }

        // the source material: what the file said
        material.source.shaderName = source.has_pbr_specular_glossiness && !source.has_pbr_metallic_roughness ? "glTF/specularGlossiness" : "glTF/metallicRoughness";
        CORE::CDataNode& properties = material.source.properties;
        properties.SetAttribute( "metallicFactor", material.metallicFactor );
        properties.SetAttribute( "roughnessFactor", material.roughnessFactor );
        properties.SetAttribute( "alphaMode", ASSETCONVERTER::ToString( material.alphaMode ) );
        properties.SetAttribute( "alphaCutoff", material.alphaCutoff );
        properties.SetAttribute( "doubleSided", material.doubleSided );
        properties.SetAttribute( "unlit", material.unlit );
        CORE::CDataNode::TVariantVector colour;
        for ( UInt32 c=0; c<4; ++c )
            colour.push_back( CORE::CVariant( material.baseColorFactor[ c ] ) );
        properties.SetValuesOfChildByName( "baseColorFactor", colour );
        CORE::CDataNode::TVariantVector emissive;
        for ( UInt32 c=0; c<3; ++c )
            emissive.push_back( CORE::CVariant( material.emissiveFactor[ c ] ) );
        properties.SetValuesOfChildByName( "emissiveFactor", emissive );
        struct SSlot { const char* name; const ASSETCONVERTER::CAssetTextureReference* reference; };
        const SSlot slots[] =
        {
            { "baseColorTexture", &material.baseColorTexture }, { "metallicRoughnessTexture", &material.metallicRoughnessTexture },
            { "normalTexture", &material.normalTexture }, { "occlusionTexture", &material.occlusionTexture }, { "emissiveTexture", &material.emissiveTexture }
        };
        for ( size_t s=0; s<sizeof( slots ) / sizeof( slots[ 0 ] ); ++s )
        {
            if ( !slots[ s ].reference->IsUsed() )
                continue;
            CORE::CDataNode* slot = material.source.textures.AddChild( slots[ s ].name, GUCEF_DATATYPE_OBJECT );
            slot->SetAttribute( "texture", slots[ s ].reference->textureIndex );
            slot->SetAttribute( "texCoord", slots[ s ].reference->texCoord );
        }
        m_scene.materials.push_back( material );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadMorphTargets( const cgltf_mesh& mesh, const cgltf_primitive& primitive, ASSETCONVERTER::CAssetSubMesh& subMesh )
{GUCEF_TRACE;

    const cgltf_accessor* positions = cgltf_find_accessor( &primitive, cgltf_attribute_type_position, 0 );
    const size_t vertexCount = GUCEF_NULL != positions ? positions->count : 0;
    for ( cgltf_size t=0; t<primitive.targets_count; ++t )
    {
        ASSETCONVERTER::CAssetMorphTarget target;
        if ( t < mesh.target_names_count && GUCEF_NULL != mesh.target_names[ t ] )
            target.name = mesh.target_names[ t ];
        if ( t < mesh.weights_count )
            target.defaultWeight = mesh.weights[ t ];

        const cgltf_morph_target& source = primitive.targets[ t ];
        for ( cgltf_size a=0; a<source.attributes_count; ++a )
        {
            const cgltf_attribute& attribute = source.attributes[ a ];
            TFloatVector values;
            if ( !ReadFloats( attribute.data, 3, values ) || values.size() != vertexCount * 3 )
                continue;
            if ( cgltf_attribute_type_position == attribute.type )
                target.positionDeltas.swap( values );
            else
            if ( cgltf_attribute_type_normal == attribute.type )
                target.normalDeltas.swap( values );
            else
            if ( cgltf_attribute_type_tangent == attribute.type )
                target.tangentDeltas.swap( values );
        }
        subMesh.morphTargets.push_back( target );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneReader::ReadPrimitive( const cgltf_mesh& mesh, UInt32 meshIndex, UInt32 primitiveIndex, ASSETCONVERTER::CAssetSubMesh& subMesh )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    const cgltf_primitive& primitive = mesh.primitives[ primitiveIndex ];
    const CORE::CString subject = ( GUCEF_NULL != mesh.name ? CORE::CString( mesh.name ) : "mesh" + CORE::UInt32ToString( meshIndex ) ) + "/" + CORE::UInt32ToString( primitiveIndex );

    if ( primitive.has_draco_mesh_compression )
    {
        m_report.AddSkipped( SourceName, subject, "the primitive is Draco compressed, which is not supported" );
        return false;
    }
    if ( cgltf_primitive_type_triangles != primitive.type && cgltf_primitive_type_triangle_strip != primitive.type && cgltf_primitive_type_triangle_fan != primitive.type )
    {
        m_report.AddSkipped( SourceName, subject, "the primitive is made of points or lines, not triangles" );
        return false;
    }

    const cgltf_accessor* positions = cgltf_find_accessor( &primitive, cgltf_attribute_type_position, 0 );
    if ( GUCEF_NULL == positions || GUCEF_NULL == positions->buffer_view || positions->buffer_view->has_meshopt_compression )
    {
        m_report.AddSkipped( SourceName, subject, "the primitive has no readable positions" );
        return false;
    }
    if ( !ReadFloats( positions, 3, subMesh.positions ) )
    {
        m_report.AddSkipped( SourceName, subject, "the positions cannot be read" );
        return false;
    }
    const size_t vertexCount = positions->count;

    TFloatVector values;
    if ( ReadFloats( cgltf_find_accessor( &primitive, cgltf_attribute_type_normal, 0 ), 3, values ) && values.size() == vertexCount * 3 )
        subMesh.normals.swap( values );
    if ( ReadFloats( cgltf_find_accessor( &primitive, cgltf_attribute_type_tangent, 0 ), 4, values ) && values.size() == vertexCount * 4 )
        subMesh.tangents.swap( values );
    for ( UInt32 uv=0; uv<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++uv )
        if ( ReadFloats( cgltf_find_accessor( &primitive, cgltf_attribute_type_texcoord, (cgltf_int) uv ), 2, values ) && values.size() == vertexCount * 2 )
            subMesh.uvSets[ uv ].swap( values );
    if ( GUCEF_NULL != cgltf_find_accessor( &primitive, cgltf_attribute_type_texcoord, (cgltf_int) ASSETCONVERTER::CAssetSubMesh::MaxUvSets ) )
        m_report.AddApproximation( SourceName, subject, "more than 4 UV sets: the first 4 are kept" );

    const cgltf_accessor* colours = cgltf_find_accessor( &primitive, cgltf_attribute_type_color, 0 );
    if ( GUCEF_NULL != colours )
    {
        const UInt32 components = (UInt32) cgltf_num_components( colours->type );
        if ( ReadFloats( colours, components, values ) && values.size() == vertexCount * components && ( 3 == components || 4 == components ) )
        {
            if ( 4 == components )
            {
                subMesh.colours.swap( values );
            }
            else
            {
                subMesh.colours.reserve( vertexCount * 4 );
                for ( size_t v=0; v<vertexCount; ++v )
                {
                    subMesh.colours.push_back( values[ v * 3 ] );
                    subMesh.colours.push_back( values[ v * 3 + 1 ] );
                    subMesh.colours.push_back( values[ v * 3 + 2 ] );
                    subMesh.colours.push_back( 1.0f );
                }
            }
        }
    }
    if ( GUCEF_NULL != cgltf_find_accessor( &primitive, cgltf_attribute_type_color, 1 ) )
        m_report.AddApproximation( SourceName, subject, "more than one colour set: the first is kept" );

    // the joints and weights: every set of four, then trimmed to the influences that are used
    UInt32 sets = 0;
    while ( GUCEF_NULL != cgltf_find_accessor( &primitive, cgltf_attribute_type_joints, (cgltf_int) sets ) &&
            GUCEF_NULL != cgltf_find_accessor( &primitive, cgltf_attribute_type_weights, (cgltf_int) sets ) )
        ++sets;
    if ( sets > 0 )
    {
        const UInt32 slots = sets * 4;
        TUInt32Vector joints;
        TFloatVector weights;
        joints.resize( vertexCount * slots, 0 );
        weights.resize( vertexCount * slots, 0.0f );
        bool ok = true;
        for ( UInt32 s=0; s<sets && ok; ++s )
        {
            const cgltf_accessor* jointAccessor = cgltf_find_accessor( &primitive, cgltf_attribute_type_joints, (cgltf_int) s );
            TFloatVector setWeights;
            ok = jointAccessor->count == vertexCount && ReadFloats( cgltf_find_accessor( &primitive, cgltf_attribute_type_weights, (cgltf_int) s ), 4, setWeights ) && setWeights.size() == vertexCount * 4;
            for ( size_t v=0; v<vertexCount && ok; ++v )
            {
                cgltf_uint indices[ 4 ] = { 0, 0, 0, 0 };
                ok = 0 != cgltf_accessor_read_uint( jointAccessor, v, indices, 4 );
                for ( UInt32 j=0; j<4 && ok; ++j )
                {
                    joints[ v * slots + s * 4 + j ] = indices[ j ];
                    weights[ v * slots + s * 4 + j ] = setWeights[ v * 4 + j ];
                }
            }
        }
        if ( ok )
        {
            UInt32 used = 1;
            for ( size_t v=0; v<vertexCount; ++v )
                for ( UInt32 slot=used; slot<slots; ++slot )
                    if ( weights[ v * slots + slot ] > 0.0f )
                        used = slot + 1;
            const CORE::CDataNode* extras = GetMember( GetElement( GetElement( &m_json, "meshes", meshIndex ), "primitives", primitiveIndex ), "extras" );
            const Int32 written = GetValue( extras, "gucefInfluences" ).AsInt32( 0 );
            if ( written > 0 && (UInt32) written <= slots && (UInt32) written >= used )
                used = (UInt32) written;
            subMesh.influencesPerVertex = used;
            subMesh.joints.reserve( vertexCount * used );
            subMesh.weights.reserve( vertexCount * used );
            for ( size_t v=0; v<vertexCount; ++v )
            {
                for ( UInt32 slot=0; slot<used; ++slot )
                {
                    subMesh.joints.push_back( joints[ v * slots + slot ] );
                    subMesh.weights.push_back( weights[ v * slots + slot ] );
                }
            }
        }
        else
        {
            m_report.AddWarning( SourceName, subject, "the joints and weights cannot be read: the primitive is not skinned" );
        }
    }

    // the triangles
    TUInt32Vector raw;
    if ( GUCEF_NULL != primitive.indices )
    {
        raw.resize( primitive.indices->count );
        if ( !raw.empty() && cgltf_accessor_unpack_indices( primitive.indices, &raw[ 0 ], sizeof( UInt32 ), raw.size() ) != raw.size() )
        {
            m_report.AddSkipped( SourceName, subject, "the indices cannot be read" );
            return false;
        }
    }
    else
    {
        raw.resize( vertexCount );
        for ( size_t v=0; v<vertexCount; ++v )
            raw[ v ] = (UInt32) v;
    }
    if ( cgltf_primitive_type_triangles == primitive.type )
    {
        subMesh.indices.swap( raw );
    }
    else
    if ( cgltf_primitive_type_triangle_strip == primitive.type )
    {
        for ( size_t i=2; i<raw.size(); ++i )
        {
            const bool even = 0 == ( i % 2 );
            subMesh.indices.push_back( even ? raw[ i-2 ] : raw[ i-1 ] );
            subMesh.indices.push_back( even ? raw[ i-1 ] : raw[ i-2 ] );
            subMesh.indices.push_back( raw[ i ] );
        }
    }
    else
    {
        for ( size_t i=2; i<raw.size(); ++i )
        {
            subMesh.indices.push_back( raw[ 0 ] );
            subMesh.indices.push_back( raw[ i-1 ] );
            subMesh.indices.push_back( raw[ i ] );
        }
    }
    subMesh.materialIndex = GUCEF_NULL != primitive.material ? (Int32) cgltf_material_index( data, primitive.material ) : -1;
    ReadMorphTargets( mesh, primitive, subMesh );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadMeshes( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    for ( cgltf_size m=0; m<data->meshes_count; ++m )
    {
        const cgltf_mesh& source = data->meshes[ m ];
        ASSETCONVERTER::CAssetMesh mesh;
        mesh.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : CORE::CString::Empty;
        for ( cgltf_size p=0; p<source.primitives_count; ++p )
        {
            ASSETCONVERTER::CAssetSubMesh subMesh;
            if ( ReadPrimitive( source, (UInt32) m, (UInt32) p, subMesh ) )
                mesh.subMeshes.push_back( subMesh );
        }
        m_scene.meshes.push_back( mesh );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadLights( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    for ( cgltf_size l=0; l<data->lights_count; ++l )
    {
        const cgltf_light& source = data->lights[ l ];
        ASSETCONVERTER::CAssetLight light;
        light.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : CORE::CString::Empty;
        switch ( source.type )
        {
            case cgltf_light_type_directional : light.type = ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL; break;
            case cgltf_light_type_spot : light.type = ASSETCONVERTER::LIGHTTYPE_SPOT; break;
            default : light.type = ASSETCONVERTER::LIGHTTYPE_POINT; break;
        }
        for ( UInt32 c=0; c<3; ++c )
            light.colour[ c ] = source.color[ c ];
        light.intensity = source.intensity;
        light.range = source.range;
        light.innerConeAngle = source.spot_inner_cone_angle;
        light.outerConeAngle = source.spot_outer_cone_angle;
        m_scene.lights.push_back( light );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadNodes( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    m_scene.nodes.assign( data->nodes_count, ASSETCONVERTER::CAssetNode() );
    for ( cgltf_size n=0; n<data->nodes_count; ++n )
    {
        const cgltf_node& source = data->nodes[ n ];
        ASSETCONVERTER::CAssetNode& node = m_scene.nodes[ n ];
        node.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : CORE::CString::Empty;
        node.parent = GUCEF_NULL != source.parent ? (Int32) cgltf_node_index( data, source.parent ) : -1;
        for ( cgltf_size c=0; c<source.children_count; ++c )
            node.children.push_back( (UInt32) cgltf_node_index( data, source.children[ c ] ) );
        node.mesh = GUCEF_NULL != source.mesh ? (Int32) cgltf_mesh_index( data, source.mesh ) : -1;
        node.skin = GUCEF_NULL != source.skin ? (Int32) cgltf_skin_index( data, source.skin ) : -1;
        node.light = GUCEF_NULL != source.light ? (Int32) cgltf_light_index( data, source.light ) : -1;

        if ( source.has_matrix )
        {
            const MATH::TFloat32Matrix4D matrix = MATH::TFloat32Matrix4D::FromColumnMajor( source.matrix );
            MATH::TFloat32Transform3D transform;
            if ( MATH::TFloat32Transform3D::FromMatrix( matrix, transform ) )
                node.transform = transform;
            else
                m_report.AddApproximation( SourceName, node.name, "the matrix of the node has shear or no scale: the node gets no transform" );
        }
        else
        {
            if ( source.has_translation )
                node.transform.position = MATH::TFloat32Transform3D::TPositionVector( source.translation[ 0 ], source.translation[ 1 ], source.translation[ 2 ] );
            if ( source.has_rotation )
            {
                node.transform.rotation.x = source.rotation[ 0 ];
                node.transform.rotation.y = source.rotation[ 1 ];
                node.transform.rotation.z = source.rotation[ 2 ];
                node.transform.rotation.w = source.rotation[ 3 ];
            }
            if ( source.has_scale )
            {
                node.transform.scale.x = source.scale[ 0 ];
                node.transform.scale.y = source.scale[ 1 ];
                node.transform.scale.z = source.scale[ 2 ];
            }
        }
        if ( source.weights_count > 0 )
            m_report.AddApproximation( SourceName, node.name, "the node sets weights of its morph targets: the default weights of the mesh are used" );

        // the extras of the node, without what this converter wrote for its own structure
        const CORE::CDataNode* extras = GetMember( GetElement( &m_json, "nodes", n ), "extras" );
        if ( GUCEF_NULL != extras )
        {
            CORE::CDataNode copy( *extras );
            node.prefabInstance = GetValue( &copy, "prefabInstance" ).AsString();
            node.particleSystem = GetValue( &copy, "gucefParticleSystem" ).AsInt32( -1 );
            copy.DelAttribute( "prefabInstance" );
            copy.DelAttribute( "gucefParticleSystem" );
            copy.DelAttribute( "gucefParent" );
            copy.DelAttribute( "gucefCollider" );
            copy.DelChild( "gucefLodGroup" );
            copy.DelChild( "MSFT_screencoverage" );
            node.extras = copy;
            node.extras.SetName( "extras" );
        }
    }

    // the roots: those of every scene, the default scene first
    GUCEF::set< UInt32 > seen;
    GUCEF::vector< const cgltf_scene* > scenes;
    if ( GUCEF_NULL != data->scene )
        scenes.push_back( data->scene );
    for ( cgltf_size s=0; s<data->scenes_count; ++s )
        if ( &data->scenes[ s ] != data->scene )
            scenes.push_back( &data->scenes[ s ] );
    for ( size_t s=0; s<scenes.size(); ++s )
    {
        for ( cgltf_size r=0; r<scenes[ s ]->nodes_count; ++r )
        {
            const UInt32 index = (UInt32) cgltf_node_index( data, scenes[ s ]->nodes[ r ] );
            if ( seen.insert( index ).second )
                m_scene.rootNodes.push_back( index );
        }
    }
    if ( scenes.empty() )
    {
        // no scene: the nodes without a parent
        for ( cgltf_size n=0; n<data->nodes_count; ++n )
            if ( m_scene.nodes[ n ].parent < 0 )
                m_scene.rootNodes.push_back( (UInt32) n );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadSkins( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    for ( cgltf_size s=0; s<data->skins_count; ++s )
    {
        const cgltf_skin& source = data->skins[ s ];
        ASSETCONVERTER::CAssetSkin skin;
        skin.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : CORE::CString::Empty;
        for ( cgltf_size j=0; j<source.joints_count; ++j )
            skin.joints.push_back( (UInt32) cgltf_node_index( data, source.joints[ j ] ) );
        skin.skeletonRoot = GUCEF_NULL != source.skeleton ? (Int32) cgltf_node_index( data, source.skeleton ) : -1;

        TFloatVector matrices;
        if ( GUCEF_NULL != source.inverse_bind_matrices && ReadFloats( source.inverse_bind_matrices, 16, matrices ) && matrices.size() == source.joints_count * 16 )
        {
            for ( size_t j=0; j<source.joints_count; ++j )
                skin.inverseBindMatrices.push_back( MATH::TFloat32Matrix4D::FromColumnMajor( &matrices[ j * 16 ] ) );
        }
        else
        {
            // the identity for every joint, as glTF says
            for ( size_t j=0; j<source.joints_count; ++j )
                skin.inverseBindMatrices.push_back( MATH::TFloat32Matrix4D() );
        }
        m_scene.skins.push_back( skin );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadAnimations( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    for ( cgltf_size a=0; a<data->animations_count; ++a )
    {
        const cgltf_animation& source = data->animations[ a ];
        ASSETCONVERTER::CAssetAnimation animation;
        animation.name = GUCEF_NULL != source.name ? CORE::CString( source.name ) : "animation" + CORE::UInt32ToString( (UInt32) a );

        for ( cgltf_size c=0; c<source.channels_count; ++c )
        {
            const cgltf_animation_channel& channel = source.channels[ c ];
            if ( GUCEF_NULL == channel.target_node || GUCEF_NULL == channel.sampler )
            {
                m_report.AddSkipped( SourceName, animation.name, "a channel without a node is skipped (it targets a property by pointer)" );
                continue;
            }
            ASSETCONVERTER::CAssetAnimationChannel result;
            result.targetNode = (UInt32) cgltf_node_index( data, channel.target_node );
            switch ( channel.target_path )
            {
                case cgltf_animation_path_type_translation : result.path = ASSETCONVERTER::ANIMATIONPATH_TRANSLATION; break;
                case cgltf_animation_path_type_rotation : result.path = ASSETCONVERTER::ANIMATIONPATH_ROTATION; break;
                case cgltf_animation_path_type_scale : result.path = ASSETCONVERTER::ANIMATIONPATH_SCALE; break;
                case cgltf_animation_path_type_weights : result.path = ASSETCONVERTER::ANIMATIONPATH_WEIGHTS; break;
                default :
                    m_report.AddSkipped( SourceName, animation.name, "a channel of an unknown property is skipped" );
                    continue;
            }
            switch ( channel.sampler->interpolation )
            {
                case cgltf_interpolation_type_step : result.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_STEP; break;
                case cgltf_interpolation_type_cubic_spline : result.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE; break;
                default : result.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR; break;
            }
            const cgltf_accessor* output = channel.sampler->output;
            if ( !ReadFloats( channel.sampler->input, 1, result.times ) || !ReadFloats( output, (UInt32) cgltf_num_components( output->type ), result.values ) )
            {
                m_report.AddSkipped( SourceName, animation.name, "a channel whose keys cannot be read is skipped" );
                continue;
            }
            animation.channels.push_back( result );
        }

        // the facts the writer kept of the clip
        const CORE::CDataNode* extras = GetMember( GetElement( &m_json, "animations", a ), "extras" );
        animation.loop = GetValue( extras, "loop" ).AsBool( false );
        animation.sampled = GetValue( extras, "sampled" ).AsBool( false );
        animation.sourceTake = GetValue( extras, "sourceTake" ).AsString();
        animation.frameRate = GetValue( extras, "frameRate" ).AsFloat32( 30.0f );
        animation.firstFrame = GetValue( extras, "firstFrame" ).AsFloat32( 0.0f );
        animation.lastFrame = GUCEF_NULL != extras && !GetValue( extras, "lastFrame" ).IsNULLOrEmpty() ? GetValue( extras, "lastFrame" ).AsFloat32() : animation.GetDuration() * animation.frameRate;
        m_scene.animations.push_back( animation );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadLodGroups( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    GUCEF::set< UInt32 > lowerLevels;
    for ( cgltf_size n=0; n<data->nodes_count; ++n )
    {
        const CORE::CDataNode* node = GetElement( &m_json, "nodes", n );
        const CORE::CDataNode* lod = GetMember( GetMember( node, "extensions" ), "MSFT_lod" );
        const CORE::CDataNode* ids = GetMember( lod, "ids" );
        if ( GUCEF_NULL == ids || !ids->HasChildren() )
            continue;

        // the group: the node and the lower levels
        ASSETCONVERTER::CAssetLodGroup group;
        GUCEF::vector< Float32 > coverage;
        GetNumbers( GetMember( node, "extras" ), "MSFT_screencoverage", coverage );
        group.AddLevel( (UInt32) n, coverage.size() > 0 ? coverage[ 0 ] : -1.0f );
        CORE::CDataNode::const_iterator i = ids->ConstBegin();
        UInt32 level = 1;
        while ( i != ids->ConstEnd() )
        {
            const Int32 id = (*i)->GetValue().AsInt32( -1 );
            if ( id >= 0 && (size_t) id < m_scene.nodes.size() )
            {
                group.AddLevel( (UInt32) id, level < coverage.size() ? coverage[ level ] : -1.0f );
                lowerLevels.insert( (UInt32) id );
            }
            ++level;
            ++i;
        }
        group.name = m_scene.nodes[ n ].name;
        m_scene.lodGroups.push_back( group );

        // the holder of the group is the node that says so, else the first level
        UInt32 holder = (UInt32) n;
        for ( size_t h=0; h<m_scene.nodes.size(); ++h )
        {
            const CORE::CDataNode* holderGroup = GetMember( GetMember( GetElement( &m_json, "nodes", h ), "extras" ), "gucefLodGroup" );
            if ( GUCEF_NULL != holderGroup && GetValue( holderGroup, "first" ).AsInt32( -1 ) == (Int32) n )
            {
                holder = (UInt32) h;
                m_scene.lodGroups.back().name = GetValue( holderGroup, "name" ).AsString();
                break;
            }
        }
        m_scene.nodes[ holder ].lodGroup = (Int32) m_scene.lodGroups.size() - 1;
    }

    // the parents of the lower levels: those the writer kept, else the parent of the first level
    GUCEF::set< UInt32 >::const_iterator l = lowerLevels.begin();
    while ( l != lowerLevels.end() )
    {
        const UInt32 index = *l;
        ASSETCONVERTER::CAssetNode& node = m_scene.nodes[ index ];
        ++l;
        if ( node.parent >= 0 )
            continue;
        const CORE::CDataNode* extras = GetMember( GetElement( &m_json, "nodes", index ), "extras" );
        Int32 parent = -1;
        if ( GUCEF_NULL != extras && !GetValue( extras, "gucefParent" ).IsNULLOrEmpty() )
        {
            parent = GetValue( extras, "gucefParent" ).AsInt32( -1 );
        }
        else
        {
            for ( size_t g=0; g<m_scene.lodGroups.size(); ++g )
                for ( size_t lv=1; lv<m_scene.lodGroups[ g ].levels.size(); ++lv )
                    if ( m_scene.lodGroups[ g ].levels[ lv ].node == index )
                        parent = m_scene.nodes[ m_scene.lodGroups[ g ].levels[ 0 ].node ].parent;
        }
        if ( parent >= (Int32) m_scene.nodes.size() )
            parent = -1;
        node.parent = parent;
        ASSETCONVERTER::CAssetScene::TUInt32Vector& siblings = parent >= 0 ? m_scene.nodes[ parent ].children : m_scene.rootNodes;
        size_t position = 0;
        while ( position < siblings.size() && siblings[ position ] < index )
            ++position;
        siblings.insert( siblings.begin() + position, index );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadColliders( void )
{GUCEF_TRACE;

    const CORE::CDataNode* shapes = GetMember( GetMember( GetMember( &m_json, "extensions" ), "OMI_physics_shape" ), "shapes" );
    if ( !m_hasJson || GUCEF_NULL == shapes )
        return;

    ASSETCONVERTER::CAssetScene::TUInt32Set carriers;
    const size_t nodeCount = m_scene.nodes.size();
    for ( size_t n=0; n<nodeCount; ++n )
    {
        const CORE::CDataNode* json = GetElement( &m_json, "nodes", n );
        const CORE::CDataNode* body = GetMember( GetMember( json, "extensions" ), "OMI_physics_body" );
        if ( GUCEF_NULL == body )
            continue;

        for ( UInt32 kind=0; kind<2; ++kind )
        {
            const CORE::CDataNode* reference = GetMember( body, 0 == kind ? "collider" : "trigger" );
            if ( GUCEF_NULL == reference )
                continue;
            const Int32 shapeIndex = GetValue( reference, "shape" ).AsInt32( -1 );
            const CORE::CDataNode* shape = shapeIndex >= 0 ? shapes->GetChildAtIndex( (UInt32) shapeIndex ) : GUCEF_NULL;
            if ( GUCEF_NULL == shape )
                continue;

            ASSETCONVERTER::CAssetCollider collider;
            collider.trigger = 1 == kind;
            collider.name = m_scene.nodes[ n ].name;
            const CORE::CString type = GetValue( shape, "type" ).AsString();
            const CORE::CDataNode* parameters = GetMember( shape, type.C_String() );
            if ( !ASSETCONVERTER::FromString( type, collider.shape ) )
            {
                m_report.AddSkipped( SourceName, collider.name, "the collider shape " + type + " is not known" );
                continue;
            }
            switch ( collider.shape )
            {
                case ASSETCONVERTER::COLLIDERSHAPE_BOX :
                {
                    GUCEF::vector< Float32 > size;
                    GetNumbers( parameters, "size", size );
                    if ( 3 == size.size() )
                        for ( UInt32 c=0; c<3; ++c )
                            collider.halfExtents[ c ] = size[ c ] * 0.5f;
                    break;
                }
                case ASSETCONVERTER::COLLIDERSHAPE_SPHERE :
                {
                    collider.radius = GetValue( parameters, "radius" ).AsFloat32( 0.5f );
                    break;
                }
                case ASSETCONVERTER::COLLIDERSHAPE_CAPSULE :
                case ASSETCONVERTER::COLLIDERSHAPE_CYLINDER :
                {
                    collider.height = GetValue( parameters, "height" ).AsFloat32( 1.0f );
                    collider.radius = GetValue( parameters, "radius" ).IsNULLOrEmpty() ? GetValue( parameters, "radiusTop" ).AsFloat32( 0.5f ) : GetValue( parameters, "radius" ).AsFloat32( 0.5f );
                    break;
                }
                default :
                {
                    collider.meshIndex = GetValue( parameters, "mesh" ).AsInt32( -1 );
                    if ( collider.meshIndex < 0 || (size_t) collider.meshIndex >= m_scene.meshes.size() )
                    {
                        collider.meshIndex = -1;
                        m_report.AddWarning( SourceName, collider.name, "the collider shape has no mesh" );
                    }
                    break;
                }
            }

            // a carrier node of this converter folds into the node it hangs from; a node of another file keeps the collider itself
            const bool carrier = GetValue( GetMember( json, "extras" ), "gucefCollider" ).AsBool( false );
            if ( carrier && m_scene.nodes[ n ].parent >= 0 && m_scene.nodes[ n ].mesh < 0 )
            {
                collider.transform = m_scene.nodes[ n ].transform;
                collider.transform.scale = MATH::TFloat32Transform3D::TVector::One();
                m_scene.nodes[ m_scene.nodes[ n ].parent ].colliders.push_back( collider );
                carriers.insert( (UInt32) n );
            }
            else
            {
                m_scene.nodes[ n ].colliders.push_back( collider );
            }
        }
    }
    if ( !carriers.empty() )
        m_scene.RemoveNodes( carriers );
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneReader::ReadDependencies( void )
{GUCEF_TRACE;

    const cgltf_data* data = m_document.GetData();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = m_document.GetFolder();
    for ( cgltf_size b=0; b<data->buffers_count; ++b )
    {
        if ( GUCEF_NULL != data->buffers[ b ].uri && !CGltfUri::IsDataUri( data->buffers[ b ].uri ) )
        {
            const CORE::CString path = CGltfUri::Resolve( folder, data->buffers[ b ].uri );
            if ( vfs.FileExists( path ) )
                m_scene.AddDependency( path );
        }
    }
    for ( size_t t=0; t<m_scene.textures.size(); ++t )
        for ( size_t s=0; s<m_scene.textures[ t ].sources.size(); ++s )
            if ( m_embedded.find( m_scene.textures[ t ].sources[ s ] ) == m_embedded.end() && vfs.FileExists( m_scene.textures[ t ].sources[ s ] ) )
                m_scene.AddDependency( m_scene.textures[ t ].sources[ s ] );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
