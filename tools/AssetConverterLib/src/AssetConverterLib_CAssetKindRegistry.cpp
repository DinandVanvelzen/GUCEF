/*
 *  AssetConverterLib: the table of file kinds: the built-in extensions and what the importer and exporter plugins declare
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "AssetConverterLib_CAssetKindRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetKindRegistry::BuiltInSource = "builtin";

namespace {

/** Adds the extensions (a null terminated list) as declarations of one kind */
void
AddList( TAssetFileKindVector& kinds, EAssetKind kind, const char* const* extensions )
{
    for ( ; GUCEF_NULL != *extensions; ++extensions )
        kinds.push_back( CAssetFileKind( *extensions, kind ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetKindRegistry::CAssetKindRegistry( void )
    : m_lock()
    , m_entries()
    , m_index()
{GUCEF_TRACE;

    RegisterBuiltIns();
}

/*-------------------------------------------------------------------------*/

CAssetKindRegistry::~CAssetKindRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetKindRegistry::RegisterBuiltIns( void )
{GUCEF_TRACE;

    static const char* const model[] = { "mesh", "fbx", "obj", "gltf", "glb", "dae", "3ds", "stl", "ply", "x", "blend", "ms3d", "md2", "md3", "md5mesh", "lwo", "off", "ase", "dxf", "3mf", GUCEF_NULL };
    static const char* const image[] = { "png", "jpg", "jpeg", "tga", "bmp", "gif", "tif", "tiff", "psd", "webp", "hdr", "exr", "pcx", GUCEF_NULL };
    static const char* const texture[] = { "dds", "ktx", "ktx2", "astc", "pvr", "pkm", "basis", GUCEF_NULL };
    static const char* const material[] = { "material", "mtl", GUCEF_NULL };
    static const char* const scene[] = { "scene", "umap", GUCEF_NULL };
    static const char* const skeleton[] = { "skeleton", GUCEF_NULL };
    static const char* const particle[] = { "particle", GUCEF_NULL };
    static const char* const animation[] = { "bvh", "anim", GUCEF_NULL };
    static const char* const shader[] = { "shader", "glsl", "hlsl", "vert", "frag", "wgsl", "spv", "cginc", "gdshader", GUCEF_NULL };
    static const char* const prefab[] = { "prefab", "tscn", GUCEF_NULL };
    static const char* const audio[] = { "wav", "ogg", "opus", "flac", "mp3", GUCEF_NULL };
    static const char* const font[] = { "ttf", "otf", "woff", "woff2", GUCEF_NULL };
    static const char* const document[] = { "txt", "md", "pdf", "rtf", GUCEF_NULL };
    static const char* const data[] = { "json", "xml", "yaml", "yml", "csv", "ini", "cfg", "toml", "uproject", "godot", GUCEF_NULL };
    static const char* const package[] = { "zip", "7z", "rar", "tar", "gz", "tgz", "unitypackage", "pck", "mhpkg", "uasset", GUCEF_NULL };

    TAssetFileKindVector kinds;
    AddList( kinds, ASSETKIND_MODEL, model );
    AddList( kinds, ASSETKIND_IMAGE, image );
    AddList( kinds, ASSETKIND_TEXTURE, texture );
    AddList( kinds, ASSETKIND_MATERIAL, material );
    AddList( kinds, ASSETKIND_SCENE, scene );
    AddList( kinds, ASSETKIND_SKELETON, skeleton );
    AddList( kinds, ASSETKIND_PARTICLE, particle );
    AddList( kinds, ASSETKIND_ANIMATION, animation );
    AddList( kinds, ASSETKIND_SHADER, shader );
    AddList( kinds, ASSETKIND_PREFAB, prefab );
    AddList( kinds, ASSETKIND_AUDIO, audio );
    AddList( kinds, ASSETKIND_FONT, font );
    AddList( kinds, ASSETKIND_DOCUMENT, document );
    AddList( kinds, ASSETKIND_DATA, data );
    AddList( kinds, ASSETKIND_PACKAGE, package );
    // the files that belong to an asset: "a.png.meta" and "a.png.import" to "a.png", "a.uexp" to "a.uasset" or "a.umap"
    kinds.push_back( CAssetFileKind( "meta", ASSETKIND_METADATA ) );
    kinds.push_back( CAssetFileKind( "import", ASSETKIND_METADATA ) );
    kinds.push_back( CAssetFileKind( "uexp", ASSETKIND_METADATA, "uasset umap" ) );
    kinds.push_back( CAssetFileKind( "ubulk", ASSETKIND_METADATA, "uasset umap" ) );
    kinds.push_back( CAssetFileKind( "uptnl", ASSETKIND_METADATA, "uasset umap" ) );
    RegisterFileKinds( kinds, BuiltInSource );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetKindRegistry::RegisterFileKinds( const TAssetFileKindVector& kinds, const CORE::CString& source )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 taken = 0;
    for ( size_t i=0; i<kinds.size(); ++i )
    {
        CAssetFileKind entry = kinds[ i ];
        entry.extension = entry.extension.Lowercase();
        if ( entry.extension.Length() > 0 && '.' == entry.extension[ 0 ] )
            entry.extension = entry.extension.SubstrFromRange( 1, entry.extension.Length() );
        if ( entry.extension.IsNULLOrEmpty() )
            continue;
        entry.source = source;
        m_entries.push_back( entry );
        ++taken;
    }
    RebuildIndex();
    return taken;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetKindRegistry::UnregisterSource( const CORE::CString& source )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TAssetFileKindVector kept;
    UInt32 removed = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].source == source )
            ++removed;
        else
            kept.push_back( m_entries[ i ] );
    }
    if ( removed > 0 )
    {
        m_entries = kept;
        RebuildIndex();
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

void
CAssetKindRegistry::RebuildIndex( void )
{GUCEF_TRACE;

    m_index.clear();
    for ( size_t i=0; i<m_entries.size(); ++i )
        m_index[ m_entries[ i ].extension ] = (UInt32) i;
}

/*-------------------------------------------------------------------------*/

const CAssetFileKind*
CAssetKindRegistry::Find( const CORE::CString& extension ) const
{GUCEF_TRACE;

    TIndexMap::const_iterator i = m_index.find( extension );
    return i == m_index.end() ? GUCEF_NULL : &m_entries[ i->second ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKindRegistry::GetExtensionOfPath( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    const CORE::CString name = slash < 0 ? path : path.SubstrFromRange( (UInt32) slash+1, path.Length() );
    const Int32 dot = name.HasChar( '.', false );
    if ( dot < 0 || dot+1 >= (Int32) name.Length() )
        return CORE::CString::Empty;
    return name.SubstrFromRange( (UInt32) dot+1, name.Length() ).Lowercase();
}

/*-------------------------------------------------------------------------*/

EAssetKind
CAssetKindRegistry::GetKindOfExtension( const CORE::CString& extension ) const
{GUCEF_TRACE;

    if ( extension.IsNULLOrEmpty() )
        return ASSETKIND_OTHER;
    const CORE::CString lower = extension.Lowercase();
    MT::CScopeMutex lock( m_lock );
    const CAssetFileKind* entry = Find( lower );
    return GUCEF_NULL == entry ? ASSETKIND_OTHER : entry->kind;
}

/*-------------------------------------------------------------------------*/

EAssetKind
CAssetKindRegistry::GetKindOfPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    return GetKindOfExtension( GetExtensionOfPath( path ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetKindRegistry::GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        if ( kind != m_entries[ i ].kind )
            continue;
        // an extension that a later declaration gave another kind is not of this kind, and one that is declared twice is listed once
        const CAssetFileKind* effective = Find( m_entries[ i ].extension );
        if ( GUCEF_NULL == effective || kind != effective->kind )
            continue;
        bool listed = false;
        for ( size_t e=0; e<extensions.size() && !listed; ++e )
            listed = extensions[ e ] == m_entries[ i ].extension;
        if ( !listed )
            extensions.push_back( m_entries[ i ].extension );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetKindRegistry::IsMetadataPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    return ASSETKIND_METADATA == GetKindOfPath( path );
}

/*-------------------------------------------------------------------------*/

bool
CAssetKindRegistry::GetOwnerPaths( const CORE::CString& metadataPath, CORE::CString::StringVector& owners ) const
{GUCEF_TRACE;

    const CORE::CString extension = GetExtensionOfPath( metadataPath );
    if ( extension.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    const CAssetFileKind* entry = Find( extension );
    if ( GUCEF_NULL == entry || ASSETKIND_METADATA != entry->kind )
        return false;
    const CORE::CString base = metadataPath.SubstrFromRange( 0, metadataPath.Length() - extension.Length() - 1 );
    if ( entry->ownerExtensions.empty() )
        owners.push_back( base );
    else
        for ( size_t i=0; i<entry->ownerExtensions.size(); ++i )
            owners.push_back( base + "." + entry->ownerExtensions[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetKindRegistry::GetMetadataPaths( const CORE::CString& assetPath, CORE::CString::StringVector& metadataPaths ) const
{GUCEF_TRACE;

    const CORE::CString assetExtension = GetExtensionOfPath( assetPath );
    MT::CScopeMutex lock( m_lock );
    for ( TIndexMap::const_iterator i = m_index.begin(); i != m_index.end(); ++i )
    {
        const CAssetFileKind& entry = m_entries[ i->second ];
        if ( ASSETKIND_METADATA != entry.kind )
            continue;
        if ( entry.ownerExtensions.empty() )
        {
            metadataPaths.push_back( assetPath + "." + entry.extension );
            continue;
        }
        if ( assetExtension.IsNULLOrEmpty() )
            continue;
        for ( size_t o=0; o<entry.ownerExtensions.size(); ++o )
        {
            if ( entry.ownerExtensions[ o ] == assetExtension )
            {
                metadataPaths.push_back( assetPath.SubstrFromRange( 0, assetPath.Length() - assetExtension.Length() ) + entry.extension );
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetKindRegistry::GetDeclarationCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

namespace {

struct SKindNames
{
    const char* id;
    const char* name;
};

/** In the order of EAssetKind */
const SKindNames KindNames[ ASSETKIND_COUNT ] =
{
    { "other", "Other" }, { "model", "Model" }, { "texture", "Texture" }, { "material", "Material" }, { "scene", "Scene" },
    { "skeleton", "Skeleton" }, { "folder", "Folder" }, { "particle", "Particles" }, { "image", "Image" }, { "animation", "Animation" },
    { "shader", "Shader" }, { "prefab", "Prefab" }, { "audio", "Audio" }, { "font", "Font" }, { "document", "Document" },
    { "data", "Data" }, { "package", "Package" }, { "metadata", "Metadata" }
};

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKindRegistry::GetKindName( EAssetKind kind )
{GUCEF_TRACE;

    return (UInt32) kind < (UInt32) ASSETKIND_COUNT ? KindNames[ kind ].name : KindNames[ ASSETKIND_OTHER ].name;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetKindRegistry::GetKindId( EAssetKind kind )
{GUCEF_TRACE;

    return (UInt32) kind < (UInt32) ASSETKIND_COUNT ? KindNames[ kind ].id : KindNames[ ASSETKIND_OTHER ].id;
}

/*-------------------------------------------------------------------------*/

bool
CAssetKindRegistry::TryParseKindId( const CORE::CString& id, EAssetKind& kind )
{GUCEF_TRACE;

    const CORE::CString lower = id.Lowercase();
    for ( UInt32 i=0; i<(UInt32) ASSETKIND_COUNT; ++i )
    {
        if ( lower == KindNames[ i ].id )
        {
            kind = static_cast< EAssetKind >( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
