/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetNameHints.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

struct SExtensionInfo
{
    const char* extension;
    const char* format;
    const char* kind;
};

const SExtensionInfo g_extensions[] =
{
    { "fbx", "fbx", "static-model" },        { "gltf", "gltf", "static-model" },      { "glb", "gltf", "static-model" },
    { "obj", "obj", "static-model" },        { "dae", "collada", "static-model" },    { "3ds", "3ds", "static-model" },
    { "blend", "blender", "static-model" },  { "usd", "usd", "static-model" },        { "usda", "usd", "static-model" },
    { "usdc", "usd", "static-model" },       { "usdz", "usd", "static-model" },       { "ply", "ply", "static-model" },
    { "stl", "stl", "static-model" },        { "max", "3dsmax", "static-model" },     { "ma", "maya", "static-model" },
    { "mb", "maya", "static-model" },        { "c4d", "cinema4d", "static-model" },   { "x", "directx", "static-model" },
    { "mesh", "ogre", "static-model" },      { "abc", "alembic", "static-model" },
    { "png", "png", "texture" },             { "jpg", "jpeg", "texture" },            { "jpeg", "jpeg", "texture" },
    { "tga", "tga", "texture" },             { "bmp", "bmp", "texture" },             { "dds", "dds", "texture" },
    { "ktx", "ktx", "texture" },             { "ktx2", "ktx", "texture" },            { "tif", "tiff", "texture" },
    { "tiff", "tiff", "texture" },           { "exr", "exr", "hdri" },                { "hdr", "hdr", "hdri" },
    { "psd", "psd", "texture" },             { "webp", "webp", "texture" },           { "gif", "gif", "texture" },
    { "mat", "material", "material" },       { "material", "material", "material" },  { "mtl", "mtl", "material" },
    { "sbsar", "substance", "material" },    { "sbs", "substance", "material" },      { "mdl", "mdl", "material" },
    { "mtlx", "materialx", "material" },
    { "wav", "wav", "sound-effect" },        { "ogg", "ogg", "sound-effect" },        { "mp3", "mp3", "sound-effect" },
    { "flac", "flac", "sound-effect" },      { "aif", "aiff", "sound-effect" },       { "aiff", "aiff", "sound-effect" },
    { "opus", "opus", "sound-effect" },
    { "ttf", "ttf", "font" },                { "otf", "otf", "font" },                { "woff", "woff", "font" },
    { "fnt", "fnt", "font" },
    { "anim", "anim", "animation" },         { "bvh", "bvh", "animation" },
    { "prefab", "unity", "prefab" },         { "unity", "unity", "scene" },           { "umap", "unreal", "scene" },
    { "uasset", "unreal", "other" },         { "uproject", "unreal", "template" },    { "unitypackage", "unity", "other" },
    { "scene", "scene", "scene" },
    { "pdf", "pdf", "document" },            { "txt", "text", "document" },           { "md", "markdown", "document" },
    { "html", "html", "document" },          { "htm", "html", "document" },           { "rtf", "rtf", "document" },
    { "cs", "csharp", "code" },              { "cpp", "cpp", "code" },                { "h", "cpp", "code" },
    { "hpp", "cpp", "code" },                { "py", "python", "code" },              { "js", "javascript", "code" },
    { "lua", "lua", "code" },                { "hlsl", "hlsl", "code" },              { "glsl", "glsl", "code" },
    { "shader", "shader", "code" },          { "usf", "unreal", "code" },
    { "zip", "zip", "other" },               { "7z", "7z", "other" },                 { "rar", "rar", "other" }
};

bool
StartsWithFolded( const CORE::CString& lowerName, const char* prefix )
{
    return 0 == strncmp( lowerName.C_String(), prefix, strlen( prefix ) );
}

bool
EndsWithFolded( const CORE::CString& lowerName, const char* suffix )
{
    const UInt32 length = (UInt32) strlen( suffix );
    const UInt32 nameLength = ByteLengthOf( lowerName );
    return nameLength >= length && 0 == strcmp( lowerName.C_String() + nameLength - length, suffix );
}

bool
ContainsFolded( const CORE::CString& lowerText, const char* part )
{
    return GUCEF_NULL != strstr( lowerText.C_String(), part );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::GetExtension( const CORE::CString& fileName )
{GUCEF_TRACE;

    const CORE::CString name = fileName.ReplaceChar( '\\', '/' );
    const Int32 slash = name.HasChar( '/', false );
    const CORE::CString last = slash >= 0 ? name.CutChars( (UInt32) slash + 1, true ) : name;
    const Int32 dot = last.HasChar( '.', false );
    if ( dot <= 0 )
        return CORE::CString::Empty;
    return last.CutChars( (UInt32) dot + 1, true ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::GetBaseName( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    const Int32 slash = normalized.HasChar( '/', false );
    const CORE::CString last = slash >= 0 ? normalized.CutChars( (UInt32) slash + 1, true ) : normalized;
    const Int32 dot = last.HasChar( '.', false );
    return dot > 0 ? last.SubstrToIndex( (UInt32) dot, true ) : last;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::FormatFromExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    const CORE::CString lower = extension.Lowercase();
    for ( UInt32 i=0; i<sizeof( g_extensions ) / sizeof( g_extensions[ 0 ] ); ++i )
        if ( lower == g_extensions[ i ].extension )
            return g_extensions[ i ].format;
    return lower;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::KindFromExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    const CORE::CString lower = extension.Lowercase();
    for ( UInt32 i=0; i<sizeof( g_extensions ) / sizeof( g_extensions[ 0 ] ); ++i )
        if ( lower == g_extensions[ i ].extension )
            return g_extensions[ i ].kind;
    return "other";
}

/*-------------------------------------------------------------------------*/

bool
CAssetNameHints::IsModelKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return kind == "static-model" || kind == "skeletal-model";
}

/*-------------------------------------------------------------------------*/

bool
CAssetNameHints::IsArchiveExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    const CORE::CString lower = extension.Lowercase();
    return lower == "zip" || lower == "7z" || lower == "rar" || lower == "unitypackage";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::GuessKind( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString extension = GetExtension( path );
    const CORE::CString kind = KindFromExtension( extension );
    const CORE::CString name = GetBaseName( path ).Lowercase();
    const CORE::CString lowerPath = path.ReplaceChar( '\\', '/' ).Lowercase();

    if ( IsModelKind( kind ) )
    {
        if ( StartsWithFolded( name, "sk_" ) || StartsWithFolded( name, "skm_" ) )
            return "skeletal-model";
        if ( StartsWithFolded( name, "a_" ) || StartsWithFolded( name, "anim_" ) || StartsWithFolded( name, "as_" ) || ContainsFolded( lowerPath, "/animations/" ) )
            return "animation";
        return "static-model";
    }
    if ( kind == "texture" )
    {
        if ( ContainsFolded( name, "hdri" ) || ContainsFolded( name, "skybox" ) )
            return "hdri";
        if ( ContainsFolded( name, "decal" ) || StartsWithFolded( name, "d_" ) )
            return "decal";
        if ( ContainsFolded( name, "icon" ) || ContainsFolded( lowerPath, "/icons/" ) )
            return "icon";
        if ( ContainsFolded( name, "sprite" ) || ContainsFolded( name, "flipbook" ) || ContainsFolded( lowerPath, "/sprites/" ) )
            return "sprite";
        return "texture";
    }
    if ( kind == "sound-effect" )
    {
        if ( ContainsFolded( lowerPath, "music" ) || StartsWithFolded( name, "mus_" ) )
            return "music";
        if ( ContainsFolded( lowerPath, "ambien" ) || StartsWithFolded( name, "amb_" ) )
            return "ambient-sound";
        if ( StartsWithFolded( name, "vo_" ) || ContainsFolded( lowerPath, "voice" ) )
            return "voice";
        return "sound-effect";
    }
    if ( kind == "other" && ( StartsWithFolded( name, "fx_" ) || StartsWithFolded( name, "ps_" ) || StartsWithFolded( name, "ns_" ) ) )
        return "vfx";
    return kind;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetNameHints::GuessTextureRole( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lower = GetBaseName( name ).Lowercase();
    struct SRole { const char* suffix; const char* role; };
    static const SRole roles[] =
    {
        { "_basecolor", "albedo" }, { "_base_color", "albedo" }, { "_albedo", "albedo" }, { "_diffuse", "albedo" }, { "_color", "albedo" }, { "_col", "albedo" }, { "_d", "albedo" }, { "_bc", "albedo" },
        { "_normal", "normal" }, { "_nrm", "normal" }, { "_nor", "normal" }, { "_n", "normal" },
        { "_roughness", "roughness" }, { "_rough", "roughness" }, { "_r", "roughness" },
        { "_metallic", "metallic" }, { "_metalness", "metallic" }, { "_metal", "metallic" }, { "_m", "metallic" },
        { "_ambientocclusion", "ao" }, { "_occlusion", "ao" }, { "_ao", "ao" },
        { "_emissive", "emissive" }, { "_emission", "emissive" }, { "_e", "emissive" },
        { "_height", "height" }, { "_displacement", "height" }, { "_disp", "height" }, { "_h", "height" },
        { "_orm", "orm" }, { "_rma", "orm" }, { "_mra", "orm" }, { "_arm", "orm" },
        { "_mask", "mask" }, { "_opacity", "opacity" }, { "_alpha", "opacity" }
    };
    for ( UInt32 i=0; i<sizeof( roles ) / sizeof( roles[ 0 ] ); ++i )
        if ( EndsWithFolded( lower, roles[ i ].suffix ) )
            return roles[ i ].role;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetNameHints::GetLodIndex( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lower = GetBaseName( name ).Lowercase();
    const Int32 at = lower.HasSubstr( "_lod", false );
    if ( at < 0 )
        return -1;
    const CORE::CString digits = lower.CutChars( (UInt32) at + 4, true );
    if ( digits.IsNULLOrEmpty() )
        return -1;
    for ( UInt32 i=0; i<ByteLengthOf( digits ); ++i )
        if ( digits.C_String()[ i ] < '0' || digits.C_String()[ i ] > '9' )
            return -1;
    return CORE::StringToInt32( digits );
}

/*-------------------------------------------------------------------------*/

bool
CAssetNameHints::IsCollisionName( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lower = GetBaseName( name ).Lowercase();
    return StartsWithFolded( lower, "ucx_" ) || StartsWithFolded( lower, "ubx_" ) || StartsWithFolded( lower, "usp_" ) ||
           StartsWithFolded( lower, "ucp_" ) || EndsWithFolded( lower, "-col" ) || EndsWithFolded( lower, "_col_mesh" ) ||
           EndsWithFolded( lower, "_collision" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
