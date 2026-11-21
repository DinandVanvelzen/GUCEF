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

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CICONTENTTYPERESOLVER_H
#include "gucefGUI_CIContentTypeResolver.h"
#define GUCEF_GUI_CICONTENTTYPERESOLVER_H
#endif /* GUCEF_GUI_CICONTENTTYPERESOLVER_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The kinds of files: the vocabulary of the kind registry of the converter library (CAssetKindRegistry), which holds the table of
 *  extensions that the resolver consults. The workbench has a view for model, image, texture, material (.material), scene (.scene),
 *  skeleton and particle files; the other kinds are listed and typed only.
 */
using ASSETCONVERTER::EAssetKind;
using ASSETCONVERTER::ASSETKIND_OTHER;
using ASSETCONVERTER::ASSETKIND_MODEL;
using ASSETCONVERTER::ASSETKIND_TEXTURE;
using ASSETCONVERTER::ASSETKIND_MATERIAL;
using ASSETCONVERTER::ASSETKIND_SCENE;
using ASSETCONVERTER::ASSETKIND_SKELETON;
using ASSETCONVERTER::ASSETKIND_FOLDER;
using ASSETCONVERTER::ASSETKIND_PARTICLE;
using ASSETCONVERTER::ASSETKIND_IMAGE;
using ASSETCONVERTER::ASSETKIND_ANIMATION;
using ASSETCONVERTER::ASSETKIND_SHADER;
using ASSETCONVERTER::ASSETKIND_PREFAB;
using ASSETCONVERTER::ASSETKIND_AUDIO;
using ASSETCONVERTER::ASSETKIND_FONT;
using ASSETCONVERTER::ASSETKIND_DOCUMENT;
using ASSETCONVERTER::ASSETKIND_DATA;
using ASSETCONVERTER::ASSETKIND_PACKAGE;
using ASSETCONVERTER::ASSETKIND_METADATA;
using ASSETCONVERTER::ASSETKIND_COUNT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Says what kind of asset a path is from its extension, without reading the content: the resolver of the content browser
 *  of the workbench, and the choice of the view for an asset. The table is the kind registry of the converter library: the
 *  built-in extensions and what the importer and exporter plugins declare. The type names are those of the kinds (Image, Texture,
 *  Model, Material, Scene, Skeleton, Audio, Package and so on, Other for what is not known, Folder for a directory).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetTypeResolver : public GUI::CIContentTypeResolver
{
    public:

    CAssetTypeResolver( void );

    virtual ~CAssetTypeResolver();

    virtual CORE::CString GetTypeName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetIconName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSidecar( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    /** The kind of a path by its extension (case does not matter) */
    static EAssetKind GetKind( const CORE::CString& path );

    /** The name of a kind, as the type column shows it */
    static CORE::CString GetKindName( EAssetKind kind );

    /** The extensions (lower case, without the dot) of the files of a kind, in the order of the table of the resolver */
    static void GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions );

    /** The extension of a path without the dot, in lower case; empty without one */
    static CORE::CString GetExtension( const CORE::CString& path );

    /** Whether the workbench has a view for the file: a model, image, texture or skeleton or particle file, a .material script, a .scene */
    static bool HasView( const CORE::CString& path );

    /** Whether the workbench has a view for the files of an extension of the kind */
    static bool HasViewForExtension( EAssetKind kind, const CORE::CString& extension );

    /** Whether the file is a metadata file of an asset (Unity .meta, Godot .import, Unreal .uexp): it is hidden in the content browser until asked for */
    static bool IsMetadata( const CORE::CString& path );

    private:

    CAssetTypeResolver( const CAssetTypeResolver& src );
    CAssetTypeResolver& operator=( const CAssetTypeResolver& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */
