/*
 *  AssetWorkbenchLib: the kinds of assets the workbench shows and the names of those kinds in the content browser
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

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#include "AssetWorkbenchLib_CAssetTypeResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The kind registry of the converter library, NULL when the library is gone (the application shuts down) */
const ASSETCONVERTER::CAssetKindRegistry*
Kinds( void )
{
    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::Instance();
    return GUCEF_NULL == global ? GUCEF_NULL : &global->GetPluginRegistry().GetKinds();
}

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

    return ASSETCONVERTER::CAssetKindRegistry::GetExtensionOfPath( path );
}

/*-------------------------------------------------------------------------*/

EAssetKind
CAssetTypeResolver::GetKind( const CORE::CString& path )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetKindRegistry* kinds = Kinds();
    return GUCEF_NULL == kinds ? ASSETKIND_OTHER : kinds->GetKindOfPath( path );
}

/*-------------------------------------------------------------------------*/

void
CAssetTypeResolver::GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetKindRegistry* kinds = Kinds();
    if ( GUCEF_NULL != kinds )
        kinds->GetExtensions( kind, extensions );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetKindName( EAssetKind kind )
{GUCEF_TRACE;

    return ASSETCONVERTER::CAssetKindRegistry::GetKindName( kind );
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

bool
CAssetTypeResolver::HasViewForExtension( EAssetKind kind, const CORE::CString& extension )
{GUCEF_TRACE;

    switch ( kind )
    {
        case ASSETKIND_MODEL:
        case ASSETKIND_IMAGE:
        case ASSETKIND_TEXTURE:
        case ASSETKIND_SKELETON:
        case ASSETKIND_PARTICLE:    return true;
        // the views read the scripts of Ogre: the .material of a material kind, the .scene of a scene kind
        case ASSETKIND_MATERIAL:    return "material" == extension.Lowercase();
        case ASSETKIND_SCENE:       return "scene" == extension.Lowercase();
        default:                    return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetTypeResolver::HasView( const CORE::CString& path )
{GUCEF_TRACE;

    return HasViewForExtension( GetKind( path ), GetExtension( path ) );
}

/*-------------------------------------------------------------------------*/

bool
CAssetTypeResolver::IsMetadata( const CORE::CString& path )
{GUCEF_TRACE;

    return ASSETKIND_METADATA == GetKind( path );
}

/*-------------------------------------------------------------------------*/

bool
CAssetTypeResolver::IsSidecar( const CORE::CString& path, bool isDirectory )
{GUCEF_TRACE;

    return !isDirectory && IsMetadata( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTypeResolver::GetIconName( const CORE::CString& path, bool isDirectory )
{GUCEF_TRACE;

    // the tiles show the type name until a thumbnail arrives: the workbench registers no icons
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
