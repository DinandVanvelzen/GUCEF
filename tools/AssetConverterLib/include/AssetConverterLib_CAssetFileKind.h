/*
 *  AssetConverterLib: a file extension that a plugin or the built-in table says is of a kind of asset
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

#ifndef ASSETCONVERTERLIB_CASSETFILEKIND_H
#define ASSETCONVERTERLIB_CASSETFILEKIND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */


#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One declaration of the kind registry: files with this extension are of this kind. Importer and exporter plugins hand their
 *  declarations over in GetFileKinds(); the kind registry holds them next to its built-in table.
 *
 *  A declaration of the kind ASSETKIND_METADATA says which asset the file belongs to:
 *  - ownerExtensions empty: the asset is the path without the extension of the metadata file (Unity "a.png.meta" belongs to
 *    "a.png", Godot "a.png.import" to "a.png"),
 *  - ownerExtensions given: the asset is the same path with one of these extensions (Unreal "a.uexp" belongs to "a.uasset" or "a.umap").
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetFileKind
{
    public:

    CAssetFileKind( void );

    CAssetFileKind( const CORE::CString& fileExtension, EAssetKind fileKind );

    CAssetFileKind( const CORE::CString& fileExtension, EAssetKind fileKind, const CORE::CString& owners );

    /** Lower case, without the dot */
    CORE::CString extension;
    EAssetKind kind;
    CORE::CString::StringVector ownerExtensions;
    /** Who declared it: "builtin", or the plugin; set by the kind registry */
    CORE::CString source;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CAssetFileKind > TAssetFileKindVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETFILEKIND_H ? */
