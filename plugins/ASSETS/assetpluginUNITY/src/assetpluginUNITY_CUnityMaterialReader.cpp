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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#include "assetpluginUNITY_CUnityModel.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H ? */

#include "assetpluginUNITY_CUnityMaterialReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

using ASSETCONVERTER::CAssetMaterial;
using ASSETCONVERTER::CAssetTexture;
using ASSETCONVERTER::CAssetTextureReference;
using ASSETCONVERTER::CTextureOperation;

/** A property a heuristic can map: its name and its type ("2D", "Color", "Float") */
struct SCandidate
{
    CORE::CString name;
    CORE::CString lower;
    CORE::CString type;
    bool used;
};
typedef GUCEF::vector< SCandidate > TCandidateVector;

bool Contains( const CORE::CString& text, const char* part ) { return text.HasSubstr( part ) >= 0; }

/** The matching key of a property name: lower case without underscores ("_Albedo_Map" is "albedomap") */
CORE::CString KeyOf( const CORE::CString& name ) { return name.Lowercase().RemoveChar( '_' ); }

/** Words in a texture name that show it is not the base colour */
bool IsOtherMapName( const CORE::CString& lower )
{
    static const char* const others[] = { "normal", "bump", "emiss", "overlay", "mask", "detail", "metal", "rough", "smooth", "gloss", "occlusion",
                                          "height", "parallax", "noise", "spec", "flow", "dissolve", "cube", "reflect", "light", "ambient" };
    for ( size_t i=0; i<sizeof( others ) / sizeof( others[ 0 ] ); ++i )
        if ( Contains( lower, others[ i ] ) )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityMaterialReader::CUnityMaterialReader( CUnityProject& project )
    : m_project( project )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float32
CUnityMaterialReader::SrgbToLinear( Float32 value )
{GUCEF_TRACE;

    const Float32 magnitude = value < 0.0f ? -value : value;
    const Float32 linear = magnitude <= 0.04045f ? magnitude / 12.92f : (Float32) pow( ( magnitude + 0.055 ) / 1.055, 2.4 );
    return value < 0.0f ? -linear : linear;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityMaterialReader::GetDefaultMaterial( void )
{GUCEF_TRACE;

    Int32 index = -1;
    if ( m_project.TryGetCached( "material|default", index ) )
        return index;
    CAssetMaterial material;
    CUnityBuiltins::MakeDefaultMaterial( material );
    m_project.GetScene().materials.push_back( material );
    index = (Int32) m_project.GetScene().materials.size() - 1;
    m_project.SetCached( "material|default", index );
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityMaterialReader::GetModelMaterial( const CUnityModel& model, UInt32 materialIndex )
{GUCEF_TRACE;

    const CORE::CString key = "modelMaterial|" + model.vfsPath + "|" + CORE::UInt32ToString( materialIndex );
    Int32 index = -1;
    if ( m_project.TryGetCached( key, index ) )
        return index;
    const CORE::CString subject = m_project.Describe( model.vfsPath );
    if ( materialIndex >= model.scene.materials.size() || ( model.hasMeta && !model.meta.importMaterials ) )
    {
        index = GetDefaultMaterial();
        m_project.SetCached( key, index );
        return index;
    }
    const CAssetMaterial& source = model.scene.materials[ materialIndex ];
    CUnityMeta::TReferenceMap::const_iterator remap = model.meta.externalObjects.find( source.name );
    if ( remap != model.meta.externalObjects.end() )
    {
        index = GetMaterial( (*remap).second, subject );
    }
    else
    {
        // Unity's search: by texture or material name, in Materials folders up from the model, or everywhere
        CORE::CString searchName = source.name;
        const CAssetTextureReference& base = source.baseColorTexture;
        const CORE::CString textureName = base.IsUsed() && (size_t) base.textureIndex < model.scene.textures.size() && !model.scene.textures[ base.textureIndex ].sources.empty() ?
                                          ASSETCONVERTER::CVfsPaths::StemOf( model.scene.textures[ base.textureIndex ].sources[ 0 ] ) : CORE::CString::Empty;
        if ( 0 == model.meta.materialName || 3 == model.meta.materialName )
            searchName = textureName.IsNULLOrEmpty() ? source.name : textureName;
        else
        if ( 2 == model.meta.materialName )
            searchName = ASSETCONVERTER::CVfsPaths::StemOf( model.vfsPath ) + "-" + source.name;
        CORE::CString found;
        CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( model.vfsPath );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        while ( found.IsNULLOrEmpty() )
        {
            const CORE::CString candidate = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( folder, "Materials" ), searchName + ".mat" );
            if ( vfs.FileExists( candidate ) )
                found = candidate;
            if ( 0 == model.meta.materialSearch || folder.IsNULLOrEmpty() || !folder.HasChar( '/' ) )
                break;
            folder = ASSETCONVERTER::CVfsPaths::FolderOf( folder );
        }
        if ( found.IsNULLOrEmpty() && 2 == model.meta.materialSearch )
            m_project.GetContext().TryResolve( "unity.materialName", searchName.Lowercase(), found );
        if ( !found.IsNULLOrEmpty() )
            index = ReadMaterialFile( found, 0 );
    }
    if ( index < 0 )
    {
        // the material of the file with its factors; its textures are linked only through .mat files
        CAssetMaterial copy = source;
        GUCEF::vector< CAssetTextureReference* > references;
        copy.GetTextureReferences( references );
        bool stripped = false;
        for ( size_t r=0; r<references.size(); ++r )
        {
            stripped = stripped || references[ r ]->IsUsed();
            *references[ r ] = CAssetTextureReference();
        }
        if ( stripped )
            m_project.Info( subject, "the material " + source.name + " of the model file has no .mat in the project: its factors are used, the texture paths of the model file are ignored" );
        copy.source.properties.SetAttribute( "shaderKind", "modelFile" );
        m_project.GetScene().materials.push_back( copy );
        index = (Int32) m_project.GetScene().materials.size() - 1;
    }
    m_project.SetCached( key, index );
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityMaterialReader::GetMaterial( const CUnityReference& reference, const CORE::CString& user )
{GUCEF_TRACE;

    if ( reference.IsNull() )
        return -1;
    const CORE::CString key = "material|" + reference.ToString();
    Int32 index = -1;
    if ( m_project.TryGetCached( key, index ) )
        return index;

    if ( reference.IsBuiltin() )
    {
        if ( CUnityBuiltins::DefaultMaterialId == reference.fileId )
            m_project.Warn( user, "uses the built-in default material of the editor (" + reference.ToString() + "): the default material is used" );
        else
            m_project.Warn( user, "uses the built-in material " + reference.ToString() + ", which is not in the package: the default material is used" );
        index = GetDefaultMaterial();
    }
    else
    {
        CORE::CString path;
        if ( !m_project.ResolveGuid( reference.guid, path ) )
        {
            m_project.Warn( user, "uses the material " + reference.ToString() + ", which is not in the package: the default material is used" );
            index = GetDefaultMaterial();
        }
        else
        {
            const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( path );
            if ( "mat" == extension )
            {
                index = ReadMaterialFile( path, reference.fileId );
            }
            else
            {
                m_project.Warn( user, "uses a material inside " + m_project.Describe( path ) + " (" + reference.ToString() + "), which is not read: the default material is used" );
            }
            if ( index < 0 )
                index = GetDefaultMaterial();
        }
    }
    m_project.SetCached( key, index );
    return index;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityMaterialReader::ReadMaterialFile( const CORE::CString& vfsPath, Int64 fileId )
{GUCEF_TRACE;

    const CUnityYamlFile* file = m_project.GetYaml( vfsPath );
    if ( GUCEF_NULL == file )
        return -1;
    const CUnityYamlDocument* document = GUCEF_NULL;
    if ( 0 != fileId )
        document = file->FindDocument( fileId );
    if ( GUCEF_NULL == document || 21 != document->classId )
        document = file->FindFirstOfClass( 21 );
    if ( GUCEF_NULL == document )
    {
        m_project.Warn( m_project.Describe( vfsPath ), "the file holds no Material object" );
        return -1;
    }
    CAssetMaterial material;
    if ( !MapMaterial( document->body, vfsPath, material ) )
        return -1;
    m_project.GetScene().materials.push_back( material );
    return (Int32) m_project.GetScene().materials.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::ReadValues( const CORE::CDataNode& materialObject, SValues& values )
{GUCEF_TRACE;

    // the keywords: a text of words (m_ShaderKeywords) or a list (m_ValidKeywords of newer files)
    const CORE::CString keywords = CUnityYamlReader::GetString( materialObject, "m_ShaderKeywords" );
    CORE::CString::StringVector words = keywords.ParseElements( ' ', false );
    for ( size_t w=0; w<words.size(); ++w )
        values.keywords.insert( words[ w ] );
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( materialObject.FindChild( "m_ValidKeywords" ), items );
    for ( size_t i=0; i<items.size(); ++i )
        values.keywords.insert( CUnityYamlReader::GetItemText( *items[ i ] ) );

    const CORE::CDataNode* saved = materialObject.FindChild( "m_SavedProperties" );
    if ( GUCEF_NULL == saved )
        return;
    CORE::CString::StringMap scalars;
    GUCEF::vector< std::pair< CORE::CString, const CORE::CDataNode* > > nodes;

    CUnityYamlReader::GetKeyedEntries( saved->FindChild( "m_TexEnvs" ), scalars, nodes );
    for ( size_t n=0; n<nodes.size(); ++n )
    {
        STexEnv env;
        env.texture = CUnityReference::Get( *nodes[ n ].second, "m_Texture" );
        env.scale[ 0 ] = env.scale[ 1 ] = 1.0f;
        env.offset[ 0 ] = env.offset[ 1 ] = 0.0f;
        CUnityYamlReader::GetComponents( *nodes[ n ].second, "m_Scale", "xy", env.scale );
        CUnityYamlReader::GetComponents( *nodes[ n ].second, "m_Offset", "xy", env.offset );
        values.textures[ nodes[ n ].first ] = env;
    }

    const char* const numberLists[] = { "m_Floats", "m_Ints" };
    for ( UInt32 l=0; l<2; ++l )
    {
        CUnityYamlReader::GetKeyedEntries( saved->FindChild( numberLists[ l ] ), scalars, nodes );
        for ( CORE::CString::StringMap::const_iterator s = scalars.begin(); s != scalars.end(); ++s )
        {
            Float64 value = 0.0;
            if ( CUnityYamlReader::ParseFloat( (*s).second, value ) )
                values.floats[ (*s).first ] = (Float32) value;
        }
    }

    CUnityYamlReader::GetKeyedEntries( saved->FindChild( "m_Colors" ), scalars, nodes );
    for ( size_t n=0; n<nodes.size(); ++n )
    {
        GUCEF::vector< Float32 > colour( 4, 0.0f );
        colour[ 3 ] = 1.0f;
        CUnityYamlReader::GetComponents( nodes[ n ].second, "rgba", &colour[ 0 ] );
        values.colours[ nodes[ n ].first ] = colour;
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::RecordSource( const SValues& values, const CUnityReference& shaderReference, CAssetMaterial& material )
{GUCEF_TRACE;

    ASSETCONVERTER::CSourceMaterial& source = material.source;
    source.shaderName = values.shaderName;
    source.properties = CORE::CDataNode( "properties", GUCEF_DATATYPE_OBJECT );
    source.textures = CORE::CDataNode( "textures", GUCEF_DATATYPE_OBJECT );
    source.properties.SetAttribute( "shaderKind", CORE::CString( CUnityBuiltins::GetShaderKindName( values.kind ) ) );
    source.properties.SetAttribute( "shaderReference", shaderReference.ToString() );
    CORE::CString::StringVector keywords( values.keywords.begin(), values.keywords.end() );
    source.properties.SetValuesOfChildByName( "keywords", keywords );

    CORE::CDataNode* floats = source.properties.AddChild( "floats", GUCEF_DATATYPE_OBJECT );
    for ( TFloatMap::const_iterator f = values.floats.begin(); f != values.floats.end(); ++f )
        floats->SetAttribute( (*f).first, (*f).second );
    CORE::CDataNode* colours = source.properties.AddChild( "colors", GUCEF_DATATYPE_OBJECT );
    for ( TColourMap::const_iterator c = values.colours.begin(); c != values.colours.end(); ++c )
    {
        CORE::CDataNode* colour = colours->AddChild( (*c).first, GUCEF_DATATYPE_OBJECT );
        colour->SetAttribute( "r", (*c).second[ 0 ] );
        colour->SetAttribute( "g", (*c).second[ 1 ] );
        colour->SetAttribute( "b", (*c).second[ 2 ] );
        colour->SetAttribute( "a", (*c).second[ 3 ] );
    }
    CORE::CDataNode* texEnvs = source.properties.AddChild( "texEnvs", GUCEF_DATATYPE_OBJECT );
    for ( TTexEnvMap::const_iterator t = values.textures.begin(); t != values.textures.end(); ++t )
    {
        CORE::CDataNode* env = texEnvs->AddChild( (*t).first, GUCEF_DATATYPE_OBJECT );
        env->SetAttribute( "texture", (*t).second.texture.ToString() );
        env->SetAttribute( "scaleX", (*t).second.scale[ 0 ] );
        env->SetAttribute( "scaleY", (*t).second.scale[ 1 ] );
        env->SetAttribute( "offsetX", (*t).second.offset[ 0 ] );
        env->SetAttribute( "offsetY", (*t).second.offset[ 1 ] );
        CORE::CString path;
        if ( !(*t).second.texture.IsNull() )
        {
            if ( !(*t).second.texture.IsBuiltin() && m_project.ResolveGuid( (*t).second.texture.guid, path ) )
                source.textures.SetAttribute( (*t).first, path );
            else
                source.textures.SetAttribute( (*t).first, (*t).second.texture.ToString() );
        }
    }
    if ( GUCEF_NULL != values.shader )
    {
        CORE::CString::StringVector declared;
        for ( size_t p=0; p<values.shader->properties.size(); ++p )
            declared.push_back( values.shader->properties[ p ].name + " (" + values.shader->properties[ p ].type + ")" );
        source.properties.SetValuesOfChildByName( "declaredProperties", declared );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnityMaterialReader::MapMaterial( const CORE::CDataNode& materialObject, const CORE::CString& vfsPath, CAssetMaterial& material )
{GUCEF_TRACE;

    material = CAssetMaterial();
    SValues values;
    values.name = CUnityYamlReader::GetString( materialObject, "m_Name", ASSETCONVERTER::CVfsPaths::StemOf( vfsPath ) );
    if ( values.name.IsNULLOrEmpty() )
        values.name = ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
    values.subject = m_project.Describe( vfsPath );
    values.kind = CUnityBuiltins::SHADERKIND_MISSING;
    values.shader = GUCEF_NULL;
    material.name = values.name;
    ReadValues( materialObject, values );

    // the shader, from m_Shader only
    const CUnityReference shader = CUnityReference::Get( materialObject, "m_Shader" );
    if ( shader.IsNull() )
    {
        values.shaderName = "(none)";
        m_project.Warn( values.subject, "the material has no shader: its properties are mapped by name" );
    }
    else
    if ( shader.IsBuiltin() || ( 0 == shader.type && shader.guid.IsNULLOrEmpty() ) )
    {
        values.kind = CUnityBuiltins::ClassifyBuiltinShader( shader.fileId, values.shaderName );
        if ( CUnityBuiltins::SHADERKIND_BUILTINPARTICLE == values.kind || CUnityBuiltins::SHADERKIND_BUILTINSPRITE == values.kind || CUnityBuiltins::SHADERKIND_BUILTINUNKNOWN == values.kind )
            m_project.Warn( values.subject, "the material uses the unknown built-in shader " + shader.ToString() + ": its properties are mapped by name" );
    }
    else
    if ( shader.guid == CUnityBuiltins::UrpLitShaderGuid )
    {
        values.kind = CUnityBuiltins::SHADERKIND_URPLIT;
        values.shaderName = "Universal Render Pipeline/Lit";
    }
    else
    {
        CORE::CString shaderPath;
        if ( m_project.ResolveGuid( shader.guid, shaderPath ) )
        {
            values.shader = m_project.GetShader( shaderPath );
            const bool graph = "shadergraph" == ASSETCONVERTER::CVfsPaths::ExtensionOf( shaderPath );
            values.kind = graph ? CUnityBuiltins::SHADERKIND_SHADERGRAPH : CUnityBuiltins::SHADERKIND_CUSTOM;
            values.shaderName = GUCEF_NULL != values.shader ? values.shader->name : ASSETCONVERTER::CVfsPaths::StemOf( shaderPath );
        }
        else
        {
            values.shaderName = "shader " + shader.ToString();
            const bool hdrp = values.textures.find( "_BaseColorMap" ) != values.textures.end() && values.textures.find( "_MaskMap" ) != values.textures.end();
            if ( hdrp )
            {
                values.kind = CUnityBuiltins::SHADERKIND_HDRPLIT;
                values.shaderName = "HDRP/Lit";
                m_project.Approximate( values.subject, "the shader " + shader.ToString() + " is not in the package: its property set is that of HDRP Lit, which is assumed" );
            }
            else
            {
                m_project.Warn( values.subject, "the shader " + shader.ToString() + " is not in the package: the properties of the material are mapped by name" );
            }
        }
    }

    RecordSource( values, shader, material );
    values.mapping = material.source.properties.AddChild( "mapping", GUCEF_DATATYPE_OBJECT );
    values.mapping->SetAttribute( "colours", "sRGB to linear" );
    values.mapping->SetAttribute( "colourSpaceOfTextures", "by slot" );

    // the manifest rules override the mapping of the shader
    const CORE::CDataNode* rules = m_project.GetSettings().FindChild( "shaderPropertyMaps" );
    const bool ruled = GUCEF_NULL != rules && GUCEF_NULL != rules->FindChild( values.shaderName );

    switch ( ruled ? CUnityBuiltins::SHADERKIND_COUNT : values.kind )
    {
        case CUnityBuiltins::SHADERKIND_STANDARD: MapStandard( values, material, false ); break;
        case CUnityBuiltins::SHADERKIND_STANDARDSPECULAR: MapStandard( values, material, true ); break;
        case CUnityBuiltins::SHADERKIND_LEGACYDIFFUSE: MapLegacyDiffuse( values, material ); break;
        case CUnityBuiltins::SHADERKIND_URPLIT: MapUrpLit( values, material ); break;
        case CUnityBuiltins::SHADERKIND_HDRPLIT: MapHdrpLit( values, material ); break;
        default: MapByHeuristic( values, material ); break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMaterialReader::HasTexture( const SValues& values, const CORE::CString& property ) const
{GUCEF_TRACE;

    TTexEnvMap::const_iterator i = values.textures.find( property );
    return i != values.textures.end() && !(*i).second.texture.IsNull();
}

/*-------------------------------------------------------------------------*/

Float32
CUnityMaterialReader::GetFloat( const SValues& values, const CORE::CString& property, Float32 defaultValue ) const
{GUCEF_TRACE;

    TFloatMap::const_iterator i = values.floats.find( property );
    return i == values.floats.end() ? defaultValue : (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMaterialReader::GetColour( const SValues& values, const CORE::CString& property, Float32 linear[ 4 ] ) const
{GUCEF_TRACE;

    TColourMap::const_iterator i = values.colours.find( property );
    if ( i == values.colours.end() )
        return false;
    for ( UInt32 c=0; c<3; ++c )
        linear[ c ] = SrgbToLinear( (*i).second[ c ] );
    linear[ 3 ] = (*i).second[ 3 ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMaterialReader::ResolveTexture( SValues& values, const CORE::CString& property, CORE::CString& vfsPath )
{GUCEF_TRACE;

    TTexEnvMap::const_iterator i = values.textures.find( property );
    if ( i == values.textures.end() || (*i).second.texture.IsNull() )
        return false;
    const CUnityReference& texture = (*i).second.texture;
    if ( texture.IsBuiltin() || texture.guid.IsNULLOrEmpty() )
    {
        m_project.Warn( values.subject, property + " uses the built-in texture " + texture.ToString() + ", which is not in the package: the slot is left empty" );
        return false;
    }
    if ( !m_project.ResolveGuid( texture.guid, vfsPath ) || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) )
    {
        m_project.Warn( values.subject, property + " uses the texture " + texture.ToString() + ", which is not in the package: the slot is left empty" );
        return false;
    }
    m_project.AddDependency( vfsPath );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityMaterialReader::AddTexture( const CORE::CString::StringVector& sources, const CAssetTexture::TOperationVector& operations, ASSETCONVERTER::TTextureUsage usage,
                                  const CORE::CString& name, const CORE::CString& slot, const CORE::CString& subject )
{GUCEF_TRACE;

    CAssetTexture texture;
    texture.name = name;
    texture.sources = sources;
    texture.operations = operations;
    texture.usage = usage;
    texture.wrapS = IMAGE::WRAPMODE_REPEAT;
    texture.wrapT = IMAGE::WRAPMODE_REPEAT;

    CORE::CString key = "texture|" + ASSETCONVERTER::ToString( usage );
    for ( size_t s=0; s<sources.size(); ++s )
        key += "|" + sources[ s ];
    key += "|" + texture.GetChainKeyText();
    Int32 index = -1;
    if ( m_project.TryGetCached( key, index ) )
        return index;

    // the sampler from the texture importer of the main source; its colour flag only goes into the report
    const CUnityMeta* meta = m_project.GetMeta( sources[ 0 ] );
    if ( GUCEF_NULL != meta )
    {
        const Int32 wrapU = meta->wrapU >= 0 ? meta->wrapU : meta->wrapMode;
        const Int32 wrapV = meta->wrapV >= 0 ? meta->wrapV : meta->wrapMode;
        texture.wrapS = 1 == wrapU ? IMAGE::WRAPMODE_CLAMP : ( 2 == wrapU || 3 == wrapU ? IMAGE::WRAPMODE_MIRROR : IMAGE::WRAPMODE_REPEAT );
        texture.wrapT = 1 == wrapV ? IMAGE::WRAPMODE_CLAMP : ( 2 == wrapV || 3 == wrapV ? IMAGE::WRAPMODE_MIRROR : IMAGE::WRAPMODE_REPEAT );
        const bool flaggedSrgb = 1 == meta->sRGBTexture || ( meta->sRGBTexture < 0 && 0 == meta->linearTexture );
        const bool flaggedLinear = 0 == meta->sRGBTexture || 1 == meta->linearTexture;
        const bool slotLinear = ASSETCONVERTER::TEXTUREUSAGE_COLOUR != usage;
        if ( ( slotLinear && flaggedSrgb && 1 != meta->textureType ) || ( !slotLinear && flaggedLinear ) )
            m_project.Info( subject, slot + " uses " + m_project.Describe( sources[ 0 ] ) + ", whose texture importer marks it " + ( flaggedSrgb ? "sRGB" : "linear" ) +
                            ": the slot decides, it is read as " + ( slotLinear ? "linear" : "sRGB" ) );
    }
    m_project.GetScene().textures.push_back( texture );
    index = (Int32) m_project.GetScene().textures.size() - 1;
    m_project.SetCached( key, index );
    return index;
}

/*-------------------------------------------------------------------------*/

CAssetTextureReference
CUnityMaterialReader::UseTexture( SValues& values, const CORE::CString& property, ASSETCONVERTER::TTextureUsage usage, const CORE::CString& slot,
                                  const CORE::CString& transformProperty )
{GUCEF_TRACE;

    CAssetTextureReference reference;
    CORE::CString path;
    if ( !ResolveTexture( values, property, path ) )
        return reference;
    CORE::CString::StringVector sources;
    sources.push_back( path );
    CAssetTexture::TOperationVector operations;
    CORE::CString name = ASSETCONVERTER::CVfsPaths::StemOf( path );
    if ( "occlusionTexture" == slot )
    {
        // Unity reads occlusion from G, glTF from R
        operations.push_back( CTextureOperation::ExtractChannel( 1 ) );
        name += "_occlusion";
    }
    reference.textureIndex = AddTexture( sources, operations, usage, name, slot, values.subject );

    // tiling and offset: Unity's UV origin is at the bottom, that of the model at the top
    SetTransform( values, values.textures.find( transformProperty ) != values.textures.end() ? transformProperty : property, reference );
    values.mapping->SetAttribute( slot, property );
    return reference;
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::SetTransform( const SValues& values, const CORE::CString& property, CAssetTextureReference& reference ) const
{GUCEF_TRACE;

    TTexEnvMap::const_iterator source = values.textures.find( property );
    if ( source == values.textures.end() || !reference.IsUsed() )
        return;
    const STexEnv& env = (*source).second;
    if ( 1.0f != env.scale[ 0 ] || 1.0f != env.scale[ 1 ] || 0.0f != env.offset[ 0 ] || 0.0f != env.offset[ 1 ] )
    {
        // Unity's UV origin is at the bottom, that of the model at the top
        reference.hasTransform = true;
        reference.tiling[ 0 ] = env.scale[ 0 ];
        reference.tiling[ 1 ] = env.scale[ 1 ];
        reference.offset[ 0 ] = env.offset[ 0 ];
        reference.offset[ 1 ] = 1.0f - env.scale[ 1 ] - env.offset[ 1 ];
    }
}

/*-------------------------------------------------------------------------*/

CAssetTextureReference
CUnityMaterialReader::UseMetallicGloss( SValues& values, const CORE::CString& metallicProperty, const CORE::CString& smoothnessProperty, Float32 smoothnessScale, bool metallicFromRed,
                                        const CORE::CString& transformProperty )
{GUCEF_TRACE;

    CAssetTextureReference reference;
    CORE::CString metallicPath;
    CORE::CString smoothnessPath;
    if ( !ResolveTexture( values, metallicProperty, metallicPath ) )
        return reference;
    if ( smoothnessProperty == metallicProperty )
        smoothnessPath = metallicPath;
    else
    if ( !ResolveTexture( values, smoothnessProperty, smoothnessPath ) )
        smoothnessPath = metallicPath;

    // B = metallic (R of the map), G = 1 - smoothness * scale (A of the map or of the albedo)
    CORE::CString::StringVector sources;
    sources.push_back( metallicPath );
    Int32 smoothnessSource = 0;
    if ( smoothnessPath != metallicPath )
    {
        sources.push_back( smoothnessPath );
        smoothnessSource = 1;
    }
    const Int32 packSources[ 4 ] = { -1, smoothnessSource, metallicFromRed ? 0 : -1, -1 };
    const UInt32 packChannels[ 4 ] = { 0, 3, 0, 0 };
    CAssetTexture::TOperationVector operations;
    operations.push_back( CTextureOperation::Pack( packSources, packChannels ) );
    operations.push_back( CTextureOperation::ScaleBias( 0x2, -smoothnessScale, 1.0f ) );
    reference.textureIndex = AddTexture( sources, operations, ASSETCONVERTER::TEXTUREUSAGE_MASK, ASSETCONVERTER::CVfsPaths::StemOf( metallicPath ) + "_metallicRoughness",
                                         "metallicRoughnessTexture", values.subject );
    values.mapping->SetAttribute( "metallicRoughnessTexture", metallicProperty + ( smoothnessPath != metallicPath ? " + " + smoothnessProperty + " alpha" : CORE::CString( " alpha" ) ) );
    SetTransform( values, values.textures.find( transformProperty ) != values.textures.end() ? transformProperty : metallicProperty, reference );
    return reference;
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::SetEmission( SValues& values, CAssetMaterial& material, const CORE::CString& mapProperty, const CORE::CString& colourProperty, bool needsKeyword,
                                   const CORE::CString& transformProperty )
{GUCEF_TRACE;

    Float32 colour[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const bool hasColour = GetColour( values, colourProperty, colour ) && ( colour[ 0 ] > 0.0f || colour[ 1 ] > 0.0f || colour[ 2 ] > 0.0f );
    const bool hasMap = HasTexture( values, mapProperty );
    if ( !hasColour && !hasMap )
        return;
    if ( needsKeyword && values.keywords.find( "_EMISSION" ) == values.keywords.end() )
    {
        if ( hasColour )
            m_project.Info( values.subject, "has an emission colour without the _EMISSION keyword: Unity does not show it, it is not used" );
        return;
    }
    if ( !hasColour )
        return;     // the map is multiplied by black
    Float32 strength = colour[ 0 ];
    if ( colour[ 1 ] > strength ) strength = colour[ 1 ];
    if ( colour[ 2 ] > strength ) strength = colour[ 2 ];
    if ( strength > 1.0f )
    {
        for ( UInt32 c=0; c<3; ++c )
            material.emissiveFactor[ c ] = colour[ c ] / strength;
        material.emissiveStrength = strength;
    }
    else
    {
        for ( UInt32 c=0; c<3; ++c )
            material.emissiveFactor[ c ] = colour[ c ];
    }
    values.mapping->SetAttribute( "emissiveFactor", colourProperty );
    if ( hasMap )
        material.emissiveTexture = UseTexture( values, mapProperty, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "emissiveTexture", transformProperty );
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::MapStandard( SValues& values, CAssetMaterial& material, bool specular )
{GUCEF_TRACE;

    Float32 colour[ 4 ];
    if ( GetColour( values, "_Color", colour ) )
    {
        for ( UInt32 c=0; c<4; ++c )
            material.baseColorFactor[ c ] = colour[ c ];
        values.mapping->SetAttribute( "baseColorFactor", "_Color" );
    }
    material.baseColorTexture = UseTexture( values, "_MainTex", ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "baseColorTexture" );

    // the rendering mode; 3 (Transparent) premultiplies the diffuse colour in the shader and keeps the highlights
    const Int32 mode = (Int32) GetFloat( values, "_Mode", 0.0f );
    if ( 1 == mode )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
        material.alphaCutoff = GetFloat( values, "_Cutoff", 0.5f );
    }
    else
    if ( 2 == mode || 3 == mode )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
        if ( 3 == mode )
            m_project.Approximate( values.subject, "rendering mode Transparent (premultiplied alpha, highlights kept at full strength) is written as alpha blending" );
    }

    if ( HasTexture( values, "_BumpMap" ) )
    {
        if ( values.keywords.find( "_NORMALMAP" ) != values.keywords.end() )
        {
            material.normalTexture = UseTexture( values, "_BumpMap", ASSETCONVERTER::TEXTUREUSAGE_NORMAL, "normalTexture", "_MainTex" );
            material.normalTexture.scale = GetFloat( values, "_BumpScale", 1.0f );
        }
        else
        {
            m_project.Info( values.subject, "_BumpMap is set without the _NORMALMAP keyword: Unity does not use it, it is not used" );
        }
    }

    const Float32 smoothness = GetFloat( values, "_Glossiness", 0.5f );
    const Float32 glossMapScale = GetFloat( values, "_GlossMapScale", 1.0f );
    const bool smoothnessFromAlbedo = 1 == (Int32) GetFloat( values, "_SmoothnessTextureChannel", 0.0f );
    const CORE::CString mapProperty = specular ? "_SpecGlossMap" : "_MetallicGlossMap";
    const CORE::CString mapKeyword = specular ? "_SPECGLOSSMAP" : "_METALLICGLOSSMAP";
    const bool useMap = HasTexture( values, mapProperty ) && values.keywords.find( mapKeyword ) != values.keywords.end();
    if ( HasTexture( values, mapProperty ) && !useMap )
        m_project.Info( values.subject, mapProperty + " is set without the " + mapKeyword + " keyword: Unity does not use it, it is not used" );
    if ( useMap )
        material.metallicRoughnessTexture = UseMetallicGloss( values, mapProperty, smoothnessFromAlbedo ? "_MainTex" : mapProperty, glossMapScale, !specular, "_MainTex" );
    if ( useMap && material.metallicRoughnessTexture.IsUsed() )
    {
        material.metallicFactor = specular ? 0.0f : 1.0f;
        material.roughnessFactor = 1.0f;
    }
    else
    {
        material.metallicFactor = specular ? 0.0f : GetFloat( values, "_Metallic", 0.0f );
        material.roughnessFactor = 1.0f - ( smoothnessFromAlbedo && material.baseColorTexture.IsUsed() ? glossMapScale : smoothness );
        values.mapping->SetAttribute( "metallicFactor", specular ? "0" : "_Metallic" );
        values.mapping->SetAttribute( "roughnessFactor", smoothnessFromAlbedo ? "1 - _GlossMapScale" : "1 - _Glossiness" );
        if ( smoothnessFromAlbedo && material.baseColorTexture.IsUsed() )
            m_project.Approximate( values.subject, "the smoothness comes from the alpha of the albedo, which is not packed without a metallic map: the scale is used as smoothness" );
    }
    m_project.Approximate( values.subject, "roughness is one minus Unity's smoothness" );

    if ( specular )
    {
        Float32 specColour[ 4 ] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GetColour( values, "_SpecColor", specColour );
        CORE::CDataNode* extension = material.extensions.AddChild( "KHR_materials_specular", GUCEF_DATATYPE_OBJECT );
        CORE::CString::StringVector factor;
        for ( UInt32 c=0; c<3; ++c )
            factor.push_back( CORE::FloatToString( specColour[ c ] ) );
        extension->SetAttribute( "specularFactor", 1.0f );
        CORE::CDataNode* colourNode = extension->AddChild( "specularColorFactor", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 c=0; c<3; ++c )
            colourNode->AddValueAsChild( CORE::CVariant( specColour[ c ] ) );
        values.mapping->SetAttribute( "KHR_materials_specular", "_SpecColor" );
        m_project.Approximate( values.subject, "the specular setup is written as metal-roughness (metallic 0) with KHR_materials_specular from _SpecColor; the colours of the specular map stay in the sidecar" );
    }

    if ( HasTexture( values, "_OcclusionMap" ) )
    {
        material.occlusionTexture = UseTexture( values, "_OcclusionMap", ASSETCONVERTER::TEXTUREUSAGE_MASK, "occlusionTexture", "_MainTex" );
        material.occlusionTexture.scale = GetFloat( values, "_OcclusionStrength", 1.0f );
    }
    SetEmission( values, material, "_EmissionMap", "_EmissionColor", true, "_MainTex" );

    const char* const unsupported[] = { "_ParallaxMap", "_DetailAlbedoMap", "_DetailNormalMap", "_DetailMask" };
    for ( UInt32 u=0; u<4; ++u )
        if ( HasTexture( values, unsupported[ u ] ) )
            m_project.Skip( values.subject, CORE::CString( unsupported[ u ] ) + " is not part of glTF core: it is kept in the sidecar" );
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::MapLegacyDiffuse( SValues& values, CAssetMaterial& material )
{GUCEF_TRACE;

    Float32 colour[ 4 ];
    if ( GetColour( values, "_Color", colour ) )
    {
        for ( UInt32 c=0; c<4; ++c )
            material.baseColorFactor[ c ] = colour[ c ];
        values.mapping->SetAttribute( "baseColorFactor", "_Color" );
    }
    material.baseColorTexture = UseTexture( values, "_MainTex", ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "baseColorTexture" );
    material.metallicFactor = 0.0f;
    material.roughnessFactor = 1.0f;
    m_project.Approximate( values.subject, "the legacy diffuse shader (Lambert) is written as a rough dielectric" );
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::MapUrpLit( SValues& values, CAssetMaterial& material )
{GUCEF_TRACE;

    Float32 colour[ 4 ];
    if ( GetColour( values, "_BaseColor", colour ) )
    {
        for ( UInt32 c=0; c<4; ++c )
            material.baseColorFactor[ c ] = colour[ c ];
        values.mapping->SetAttribute( "baseColorFactor", "_BaseColor" );
    }
    material.baseColorTexture = UseTexture( values, "_BaseMap", ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "baseColorTexture" );

    // surface: opaque or transparent with a blend mode; alpha clipping; culling
    if ( 1 == (Int32) GetFloat( values, "_Surface", 0.0f ) )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
        const Int32 blend = (Int32) GetFloat( values, "_Blend", 0.0f );
        if ( 0 != blend )
            m_project.Approximate( values.subject, "the URP blend mode " + CORE::Int32ToString( blend ) + " (1 premultiply, 2 additive, 3 multiply) is written as alpha blending" );
    }
    else
    if ( 1 == (Int32) GetFloat( values, "_AlphaClip", 0.0f ) )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
        material.alphaCutoff = GetFloat( values, "_Cutoff", 0.5f );
    }
    material.doubleSided = 0 == (Int32) GetFloat( values, "_Cull", 2.0f );

    if ( HasTexture( values, "_BumpMap" ) && values.keywords.find( "_NORMALMAP" ) != values.keywords.end() )
    {
        material.normalTexture = UseTexture( values, "_BumpMap", ASSETCONVERTER::TEXTUREUSAGE_NORMAL, "normalTexture", "_BaseMap" );
        material.normalTexture.scale = GetFloat( values, "_BumpScale", 1.0f );
    }
    else
    if ( HasTexture( values, "_BumpMap" ) )
    {
        m_project.Info( values.subject, "_BumpMap is set without the _NORMALMAP keyword: URP does not use it, it is not used" );
    }

    const bool metallicWorkflow = 1 == (Int32) GetFloat( values, "_WorkflowMode", 1.0f );
    const Float32 smoothness = GetFloat( values, "_Smoothness", 0.5f );
    const bool smoothnessFromAlbedo = 1 == (Int32) GetFloat( values, "_SmoothnessTextureChannel", 0.0f );
    const CORE::CString mapProperty = metallicWorkflow ? "_MetallicGlossMap" : "_SpecGlossMap";
    const bool useMap = HasTexture( values, mapProperty ) && values.keywords.find( "_METALLICSPECGLOSSMAP" ) != values.keywords.end();
    if ( HasTexture( values, mapProperty ) && !useMap )
        m_project.Info( values.subject, mapProperty + " is set without the _METALLICSPECGLOSSMAP keyword: URP does not use it, it is not used" );
    if ( useMap )
        material.metallicRoughnessTexture = UseMetallicGloss( values, mapProperty, smoothnessFromAlbedo ? "_BaseMap" : mapProperty, smoothness, metallicWorkflow, "_BaseMap" );
    if ( useMap && material.metallicRoughnessTexture.IsUsed() )
    {
        material.metallicFactor = metallicWorkflow ? 1.0f : 0.0f;
        material.roughnessFactor = 1.0f;
    }
    else
    {
        material.metallicFactor = metallicWorkflow ? GetFloat( values, "_Metallic", 0.0f ) : 0.0f;
        material.roughnessFactor = 1.0f - smoothness;
        values.mapping->SetAttribute( "metallicFactor", metallicWorkflow ? "_Metallic" : "0" );
        values.mapping->SetAttribute( "roughnessFactor", "1 - _Smoothness" );
    }
    m_project.Approximate( values.subject, "roughness is one minus Unity's smoothness" );
    if ( !metallicWorkflow )
    {
        Float32 specColour[ 4 ] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GetColour( values, "_SpecColor", specColour );
        CORE::CDataNode* extension = material.extensions.AddChild( "KHR_materials_specular", GUCEF_DATATYPE_OBJECT );
        extension->SetAttribute( "specularFactor", 1.0f );
        CORE::CDataNode* colourNode = extension->AddChild( "specularColorFactor", GUCEF_DATATYPE_ARRAY );
        for ( UInt32 c=0; c<3; ++c )
            colourNode->AddValueAsChild( CORE::CVariant( specColour[ c ] ) );
        m_project.Approximate( values.subject, "the specular workflow is written as metal-roughness (metallic 0) with KHR_materials_specular from _SpecColor" );
    }

    if ( HasTexture( values, "_OcclusionMap" ) )
    {
        material.occlusionTexture = UseTexture( values, "_OcclusionMap", ASSETCONVERTER::TEXTUREUSAGE_MASK, "occlusionTexture", "_BaseMap" );
        material.occlusionTexture.scale = GetFloat( values, "_OcclusionStrength", 1.0f );
    }
    SetEmission( values, material, "_EmissionMap", "_EmissionColor", true, "_BaseMap" );

    if ( values.keywords.find( "_CLEARCOAT" ) != values.keywords.end() || values.keywords.find( "_CLEARCOATMAP" ) != values.keywords.end() )
    {
        CORE::CDataNode* clearcoat = material.extensions.AddChild( "KHR_materials_clearcoat", GUCEF_DATATYPE_OBJECT );
        clearcoat->SetAttribute( "clearcoatFactor", GetFloat( values, "_ClearCoatMask", 0.0f ) );
        clearcoat->SetAttribute( "clearcoatRoughnessFactor", 1.0f - GetFloat( values, "_ClearCoatSmoothness", 0.0f ) );
        values.mapping->SetAttribute( "KHR_materials_clearcoat", "_ClearCoatMask, 1 - _ClearCoatSmoothness" );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::MapHdrpLit( SValues& values, CAssetMaterial& material )
{GUCEF_TRACE;

    Float32 colour[ 4 ];
    if ( GetColour( values, "_BaseColor", colour ) )
    {
        for ( UInt32 c=0; c<4; ++c )
            material.baseColorFactor[ c ] = colour[ c ];
        values.mapping->SetAttribute( "baseColorFactor", "_BaseColor" );
    }
    material.baseColorTexture = UseTexture( values, "_BaseColorMap", ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "baseColorTexture" );
    if ( 1 == (Int32) GetFloat( values, "_SurfaceType", 0.0f ) )
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    else
    if ( 1 == (Int32) GetFloat( values, "_AlphaCutoffEnable", 0.0f ) )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
        material.alphaCutoff = GetFloat( values, "_AlphaCutoff", 0.5f );
    }
    material.doubleSided = 1 == (Int32) GetFloat( values, "_DoubleSidedEnable", 0.0f );
    if ( HasTexture( values, "_NormalMap" ) )
    {
        material.normalTexture = UseTexture( values, "_NormalMap", ASSETCONVERTER::TEXTUREUSAGE_NORMAL, "normalTexture" );
        material.normalTexture.scale = GetFloat( values, "_NormalScale", 1.0f );
    }
    if ( HasTexture( values, "_MaskMap" ) )
    {
        // R metallic, G occlusion, B detail mask, A smoothness; the remaps scale the smoothness
        const Float32 smoothnessMax = GetFloat( values, "_SmoothnessRemapMax", 1.0f );
        material.metallicRoughnessTexture = UseMetallicGloss( values, "_MaskMap", "_MaskMap", smoothnessMax, true );
        material.metallicFactor = 1.0f;
        material.roughnessFactor = 1.0f;
        material.occlusionTexture = UseTexture( values, "_MaskMap", ASSETCONVERTER::TEXTUREUSAGE_MASK, "occlusionTexture" );
        m_project.Approximate( values.subject, "the HDRP mask map is split into metal-roughness and occlusion; only the maximum of the smoothness remap is applied, the detail mask stays in the sidecar" );
    }
    else
    {
        material.metallicFactor = GetFloat( values, "_Metallic", 0.0f );
        material.roughnessFactor = 1.0f - GetFloat( values, "_Smoothness", 0.5f );
    }
    SetEmission( values, material, "_EmissiveColorMap", "_EmissiveColor", false );
    if ( material.emissiveStrength != 1.0f || material.emissiveFactor[ 0 ] > 0.0f || material.emissiveFactor[ 1 ] > 0.0f || material.emissiveFactor[ 2 ] > 0.0f )
        m_project.Approximate( values.subject, "the HDRP emission is in physical units: its value is used as the emissive strength without a unit conversion" );
}

/*-------------------------------------------------------------------------*/

void
CUnityMaterialReader::MapByHeuristic( SValues& values, CAssetMaterial& material )
{GUCEF_TRACE;

    // the candidates: the properties the shader declares, typed by its declaration, or else those of the material
    TCandidateVector candidates;
    if ( GUCEF_NULL != values.shader && !values.shader->properties.empty() )
    {
        for ( size_t p=0; p<values.shader->properties.size(); ++p )
        {
            const CUnityShaderInfo::SProperty& property = values.shader->properties[ p ];
            if ( values.shader->HasAttribute( property, "HideInInspector" ) || "_texcoord" == property.name || "__dirty" == property.name )
                continue;
            SCandidate candidate;
            candidate.name = property.name;
            candidate.lower = KeyOf( property.name );
            candidate.type = CUnityShaderInfo::IsTextureType( property.type ) ? "2D" : ( CUnityShaderInfo::IsColourType( property.type ) ? "Color" : ( CUnityShaderInfo::IsNumberType( property.type ) ? "Float" : property.type ) );
            if ( "2D" == candidate.type && values.shader->HasAttribute( property, "Normal" ) && !Contains( candidate.lower, "normal" ) )
                candidate.lower += "normal";
            candidate.used = false;
            candidates.push_back( candidate );
        }
    }
    else
    {
        for ( TTexEnvMap::const_iterator t = values.textures.begin(); t != values.textures.end(); ++t )
        {
            SCandidate candidate = { (*t).first, KeyOf( (*t).first ), "2D", false };
            candidates.push_back( candidate );
        }
        for ( TColourMap::const_iterator c = values.colours.begin(); c != values.colours.end(); ++c )
        {
            SCandidate candidate = { (*c).first, KeyOf( (*c).first ), "Color", false };
            candidates.push_back( candidate );
        }
        for ( TFloatMap::const_iterator f = values.floats.begin(); f != values.floats.end(); ++f )
        {
            SCandidate candidate = { (*f).first, KeyOf( (*f).first ), "Float", false };
            candidates.push_back( candidate );
        }
    }

    // the slots: from the manifest rules, else by the names
    static const char* const slotNames[] = { "baseColorTexture", "baseColorFactor", "opacity", "normalTexture", "normalScale", "emissiveTexture", "emissiveFactor",
                                             "emissiveStrength", "metallicRoughnessTexture", "metallicFactor", "smoothnessFactor", "roughnessFactor", "occlusionTexture",
                                             "alphaCutoff", "overlay" };
    const UInt32 slotCount = sizeof( slotNames ) / sizeof( slotNames[ 0 ] );
    GUCEF::map< CORE::CString, CORE::CString > slots;
    const CORE::CDataNode* rules = m_project.GetSettings().FindChild( "shaderPropertyMaps" );
    const CORE::CDataNode* rule = GUCEF_NULL != rules ? rules->FindChild( values.shaderName ) : GUCEF_NULL;
    if ( GUCEF_NULL != rule )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = rule->AttributeBegin();
        while ( a != rule->AttributeEnd() )
        {
            const CORE::CString slot = (*a).second.AsString();
            for ( size_t c=0; c<candidates.size(); ++c )
                if ( candidates[ c ].name == (*a).first )
                    candidates[ c ].used = true;
            if ( "ignore" != slot )
                slots[ slot ] = (*a).first;
            ++a;
        }
        values.mapping->SetAttribute( "rule", "shaderPropertyMaps of the manifest" );
    }
    else
    {
        struct SPattern { const char* slot; const char* type; const char* pattern; bool exact; };
        static const SPattern patterns[] =
        {
            { "baseColorTexture", "2D", "maintex", true }, { "baseColorTexture", "2D", "basemap", true }, { "baseColorTexture", "2D", "basecolormap", true },
            { "baseColorTexture", "2D", "albedo", false }, { "baseColorTexture", "2D", "basecolor", false }, { "baseColorTexture", "2D", "diffuse", false },
            { "baseColorTexture", "2D", "maintex", false }, { "baseColorTexture", "2D", "texture", false }, { "baseColorTexture", "2D", "grid", false },
            { "normalTexture", "2D", "normal", false }, { "normalTexture", "2D", "bump", false },
            { "emissiveTexture", "2D", "emiss", false },
            { "metallicRoughnessTexture", "2D", "metal", false },
            { "occlusionTexture", "2D", "occlusion", false },
            { "overlay", "2D", "overlay", false },
            { "emissiveFactor", "Color", "emiss", false },
            { "baseColorFactor", "Color", "color", true }, { "baseColorFactor", "Color", "basecolor", true }, { "baseColorFactor", "Color", "albedo", false },
            { "baseColorFactor", "Color", "basecolor", false }, { "baseColorFactor", "Color", "maincolor", false }, { "baseColorFactor", "Color", "tint", false },
            { "baseColorFactor", "Color", "diffuse", false },
            { "opacity", "Color", "opacity", false }, { "opacity", "Float", "opacity", false },
            { "normalScale", "Float", "bumpscale", false }, { "normalScale", "Float", "normalscale", false }, { "normalScale", "Float", "normalstrength", false },
            { "normalScale", "Float", "normalamount", false }, { "normalScale", "Float", "normalintensity", false },
            { "emissiveStrength", "Float", "emissionintensity", false }, { "emissiveStrength", "Float", "emissionstrength", false }, { "emissiveStrength", "Float", "emissivepower", false },
            { "metallicFactor", "Float", "metal", false },
            { "smoothnessFactor", "Float", "smooth", false }, { "smoothnessFactor", "Float", "gloss", false },
            { "roughnessFactor", "Float", "rough", false },
            { "alphaCutoff", "Float", "cutoff", false }, { "alphaCutoff", "Float", "clipthreshold", false }
        };
        for ( size_t p=0; p<sizeof( patterns ) / sizeof( patterns[ 0 ] ); ++p )
        {
            if ( slots.find( patterns[ p ].slot ) != slots.end() )
                continue;
            for ( size_t c=0; c<candidates.size(); ++c )
            {
                SCandidate& candidate = candidates[ c ];
                if ( candidate.used || candidate.type != patterns[ p ].type )
                    continue;
                const bool match = patterns[ p ].exact ? candidate.lower == patterns[ p ].pattern : Contains( candidate.lower, patterns[ p ].pattern );
                if ( !match )
                    continue;
                if ( CORE::CString( "baseColorTexture" ) == patterns[ p ].slot && !patterns[ p ].exact && IsOtherMapName( candidate.lower ) )
                    continue;
                if ( CORE::CString( "baseColorFactor" ) == patterns[ p ].slot && Contains( candidate.lower, "emiss" ) )
                    continue;
                // a texture slot needs a texture on the material
                if ( "2D" == candidate.type && !HasTexture( values, candidate.name ) )
                    continue;
                slots[ patterns[ p ].slot ] = candidate.name;
                candidate.used = true;
                break;
            }
        }
    }

    // the material from the slots
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator s;
    material.metallicFactor = 0.0f;
    material.roughnessFactor = 0.5f;
    if ( ( s = slots.find( "baseColorFactor" ) ) != slots.end() )
    {
        Float32 colour[ 4 ];
        if ( GetColour( values, (*s).second, colour ) )
        {
            for ( UInt32 c=0; c<4; ++c )
                material.baseColorFactor[ c ] = colour[ c ];
            values.mapping->SetAttribute( "baseColorFactor", (*s).second );
        }
    }
    if ( ( s = slots.find( "baseColorTexture" ) ) != slots.end() )
        material.baseColorTexture = UseTexture( values, (*s).second, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "baseColorTexture" );
    if ( ( s = slots.find( "opacity" ) ) != slots.end() )
    {
        Float32 colour[ 4 ];
        if ( GetColour( values, (*s).second, colour ) )
            material.baseColorFactor[ 3 ] = ( colour[ 0 ] + colour[ 1 ] + colour[ 2 ] ) / 3.0f;
        else
            material.baseColorFactor[ 3 ] = GetFloat( values, (*s).second, 1.0f );
        values.mapping->SetAttribute( "opacity", (*s).second );
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    }
    if ( ( s = slots.find( "normalTexture" ) ) != slots.end() )
    {
        material.normalTexture = UseTexture( values, (*s).second, ASSETCONVERTER::TEXTUREUSAGE_NORMAL, "normalTexture" );
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator scale = slots.find( "normalScale" );
        if ( scale != slots.end() )
            material.normalTexture.scale = GetFloat( values, (*scale).second, 1.0f );
    }
    // a switch of the emission ("_Enable_Emission" 0) keeps it off
    bool emissionOff = false;
    for ( TFloatMap::const_iterator f = values.floats.begin(); f != values.floats.end(); ++f )
    {
        const CORE::CString key = KeyOf( (*f).first );
        if ( ( Contains( key, "enableemission" ) || Contains( key, "emissionenable" ) || Contains( key, "useemission" ) ) && 0.0f == (*f).second )
            emissionOff = true;
    }
    if ( emissionOff && ( slots.find( "emissiveFactor" ) != slots.end() || slots.find( "emissiveTexture" ) != slots.end() ) )
        m_project.Info( values.subject, "the emission of " + values.shaderName + " is switched off by the material: it is not used" );
    if ( !emissionOff && ( ( s = slots.find( "emissiveFactor" ) ) != slots.end() || slots.find( "emissiveTexture" ) != slots.end() ) )
    {
        const CORE::CString colourProperty = s != slots.end() ? (*s).second : CORE::CString::Empty;
        const GUCEF::map< CORE::CString, CORE::CString >::const_iterator map = slots.find( "emissiveTexture" );
        if ( colourProperty.IsNULLOrEmpty() && map != slots.end() )
        {
            // a map without a colour emits at full strength
            material.emissiveFactor[ 0 ] = material.emissiveFactor[ 1 ] = material.emissiveFactor[ 2 ] = 1.0f;
            material.emissiveTexture = UseTexture( values, (*map).second, ASSETCONVERTER::TEXTUREUSAGE_COLOUR, "emissiveTexture" );
        }
        else
        {
            SetEmission( values, material, map != slots.end() ? (*map).second : CORE::CString( "-" ), colourProperty, false );
        }
        const GUCEF::map< CORE::CString, CORE::CString >::const_iterator strength = slots.find( "emissiveStrength" );
        if ( strength != slots.end() )
            material.emissiveStrength *= GetFloat( values, (*strength).second, 1.0f );
    }
    if ( ( s = slots.find( "metallicRoughnessTexture" ) ) != slots.end() )
    {
        material.metallicRoughnessTexture = UseMetallicGloss( values, (*s).second, (*s).second, 1.0f, true );
        material.metallicFactor = 1.0f;
        material.roughnessFactor = 1.0f;
        m_project.Approximate( values.subject, (*s).second + " is read with the layout of Unity's metallic map (R metallic, A smoothness)" );
    }
    else
    {
        if ( ( s = slots.find( "metallicFactor" ) ) != slots.end() )
        {
            material.metallicFactor = GetFloat( values, (*s).second, 0.0f );
            values.mapping->SetAttribute( "metallicFactor", (*s).second );
        }
        if ( ( s = slots.find( "roughnessFactor" ) ) != slots.end() )
        {
            material.roughnessFactor = GetFloat( values, (*s).second, 0.5f );
            values.mapping->SetAttribute( "roughnessFactor", (*s).second );
        }
        else
        if ( ( s = slots.find( "smoothnessFactor" ) ) != slots.end() )
        {
            material.roughnessFactor = 1.0f - GetFloat( values, (*s).second, 0.5f );
            values.mapping->SetAttribute( "roughnessFactor", "1 - " + (*s).second );
        }
    }
    if ( ( s = slots.find( "occlusionTexture" ) ) != slots.end() )
        material.occlusionTexture = UseTexture( values, (*s).second, ASSETCONVERTER::TEXTUREUSAGE_MASK, "occlusionTexture" );
    if ( ( s = slots.find( "overlay" ) ) != slots.end() )
    {
        values.mapping->SetAttribute( "overlay", (*s).second );
        m_project.Skip( values.subject, "the overlay layer " + (*s).second + " has no glTF slot: it is kept in the sidecar" );
    }

    // the blending: the shader, the keywords, the kind of built-in
    const bool alphaTest = ( GUCEF_NULL != values.shader && values.shader->alphaTested ) || values.keywords.find( "_ALPHATEST_ON" ) != values.keywords.end();
    const bool blend = ( GUCEF_NULL != values.shader && values.shader->transparent ) || values.keywords.find( "_ALPHABLEND_ON" ) != values.keywords.end() ||
                       values.keywords.find( "_ALPHAPREMULTIPLY_ON" ) != values.keywords.end();
    // the render state of URP targets of shader graphs and of URP-like shaders
    const bool surfaceTransparent = 1 == (Int32) GetFloat( values, "_Surface", 0.0f ) && ( GUCEF_NULL == values.shader || CUnityBuiltins::SHADERKIND_SHADERGRAPH == values.kind );
    const bool alphaClip = 1 == (Int32) GetFloat( values, "_AlphaClip", 0.0f ) && ( GUCEF_NULL == values.shader || CUnityBuiltins::SHADERKIND_SHADERGRAPH == values.kind );
    if ( values.floats.find( "_Cull" ) != values.floats.end() && ( GUCEF_NULL == values.shader || CUnityBuiltins::SHADERKIND_SHADERGRAPH == values.kind ) )
        material.doubleSided = 0 == (Int32) GetFloat( values, "_Cull", 2.0f );
    if ( blend || surfaceTransparent )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
    }
    else
    if ( alphaTest || alphaClip )
    {
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_MASK;
        material.alphaCutoff = ( s = slots.find( "alphaCutoff" ) ) != slots.end() ? GetFloat( values, (*s).second, 0.5f ) : GetFloat( values, "_Cutoff", 0.5f );
    }
    if ( CUnityBuiltins::SHADERKIND_BUILTINPARTICLE == values.kind || CUnityBuiltins::SHADERKIND_BUILTINSPRITE == values.kind )
    {
        material.unlit = true;
        material.alphaMode = ASSETCONVERTER::ALPHAMODE_BLEND;
        m_project.Approximate( values.subject, "the built-in particle, sprite, UI or sky shader is written as an unlit, alpha blended material" );
    }

    // what the heuristic did, and what it left
    const CORE::CString lowerShader = values.shaderName.Lowercase();
    if ( Contains( lowerShader, "triplanar" ) )
        m_project.Approximate( values.subject, "the triplanar projection of " + values.shaderName + " is approximated with the UV coordinates of the mesh" );
    if ( Contains( lowerShader, "scroll" ) )
        m_project.Approximate( values.subject, "the UV scrolling of " + values.shaderName + " is not converted: the texture is static" );
    if ( GUCEF_NULL == rule )
        m_project.Approximate( values.subject, "the properties of " + values.shaderName + " are mapped by their names" );
    if ( GUCEF_NULL != values.shader )
    {
        CORE::CString::StringVector unmapped;
        for ( size_t c=0; c<candidates.size(); ++c )
            if ( !candidates[ c ].used )
                unmapped.push_back( candidates[ c ].name );
        if ( !unmapped.empty() )
        {
            CORE::CString list;
            for ( size_t u=0; u<unmapped.size(); ++u )
                list += ( 0 == u ? "" : ", " ) + unmapped[ u ];
            m_project.Info( values.subject, "properties of " + values.shaderName + " without a glTF slot, kept in the sidecar: " + list );
        }
    }
    for ( UInt32 i=0; i<slotCount; ++i )
    {
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator used = slots.find( slotNames[ i ] );
        if ( used != slots.end() && values.mapping->GetAttribute( slotNames[ i ] ) == GUCEF_NULL )
            values.mapping->SetAttribute( slotNames[ i ], (*used).second );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
