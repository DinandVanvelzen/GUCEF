/*
 *  AssetViewerLib: the kinds of assets the viewer shows and the names of those kinds in the content browser
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

#include "AssetViewerLib_CAssetTypeResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SExtensionKind
{
    const char* extension;
    EAssetKind kind;
};

/** Sorted by nothing in particular: the table is short and looked up once per listed file */
const SExtensionKind ExtensionKinds[] =
{
    { "mesh", ASSETKIND_MODEL }, { "fbx", ASSETKIND_MODEL }, { "obj", ASSETKIND_MODEL }, { "gltf", ASSETKIND_MODEL },
    { "glb", ASSETKIND_MODEL }, { "dae", ASSETKIND_MODEL }, { "3ds", ASSETKIND_MODEL }, { "stl", ASSETKIND_MODEL },
    { "ply", ASSETKIND_MODEL }, { "x", ASSETKIND_MODEL }, { "blend", ASSETKIND_MODEL }, { "ms3d", ASSETKIND_MODEL },
    { "md2", ASSETKIND_MODEL }, { "md3", ASSETKIND_MODEL }, { "md5mesh", ASSETKIND_MODEL }, { "lwo", ASSETKIND_MODEL },
    { "off", ASSETKIND_MODEL }, { "ase", ASSETKIND_MODEL }, { "dxf", ASSETKIND_MODEL }, { "3mf", ASSETKIND_MODEL },
    { "png", ASSETKIND_TEXTURE }, { "jpg", ASSETKIND_TEXTURE }, { "jpeg", ASSETKIND_TEXTURE }, { "tga", ASSETKIND_TEXTURE },
    { "bmp", ASSETKIND_TEXTURE }, { "gif", ASSETKIND_TEXTURE }, { "dds", ASSETKIND_TEXTURE }, { "ktx", ASSETKIND_TEXTURE },
    { "pkm", ASSETKIND_TEXTURE }, { "pvr", ASSETKIND_TEXTURE }, { "tif", ASSETKIND_TEXTURE }, { "tiff", ASSETKIND_TEXTURE },
    { "psd", ASSETKIND_TEXTURE }, { "hdr", ASSETKIND_TEXTURE }, { "exr", ASSETKIND_TEXTURE }, { "webp", ASSETKIND_TEXTURE },
    { "astc", ASSETKIND_TEXTURE }, { "pcx", ASSETKIND_TEXTURE },
    { "material", ASSETKIND_MATERIAL },
    { "scene", ASSETKIND_SCENE },
    { "skeleton", ASSETKIND_SKELETON },
    { "particle", ASSETKIND_PARTICLE }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetTypeResolver::CAssetTypeResolver( void )
    : GUI::CIContentTypeResolver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetTypeResolver::~CAssetTypeResolver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetExtension( const CORE::CString& path )
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
CAssetTypeResolver::GetKind( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString extension = GetExtension( path );
    if ( extension.IsNULLOrEmpty() )
        return ASSETKIND_OTHER;
    for ( size_t i=0; i<sizeof( ExtensionKinds ) / sizeof( ExtensionKinds[ 0 ] ); ++i )
        if ( extension == ExtensionKinds[ i ].extension )
            return ExtensionKinds[ i ].kind;
    return ASSETKIND_OTHER;
}

/*-------------------------------------------------------------------------*/

void
CAssetTypeResolver::GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions )
{GUCEF_TRACE;

    for ( size_t i=0; i<sizeof( ExtensionKinds ) / sizeof( ExtensionKinds[ 0 ] ); ++i )
        if ( kind == ExtensionKinds[ i ].kind )
            extensions.push_back( ExtensionKinds[ i ].extension );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetKindName( EAssetKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case ASSETKIND_MODEL:       return "Model";
        case ASSETKIND_TEXTURE:     return "Texture";
        case ASSETKIND_MATERIAL:    return "Material";
        case ASSETKIND_SCENE:       return "Scene";
        case ASSETKIND_SKELETON:    return "Skeleton";
        case ASSETKIND_FOLDER:      return "Folder";
        case ASSETKIND_PARTICLE:    return "Particles";
        default:                    return "Other";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetTypeName( const CORE::CString& path, bool isDirectory )
{GUCEF_TRACE;

    if ( isDirectory )
        return GetKindName( ASSETKIND_FOLDER );
    return GetKindName( GetKind( path ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetIconName( const CORE::CString& path, bool isDirectory )
{GUCEF_TRACE;

    // the tiles show the type name until a thumbnail arrives: the viewer registers no icons
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
