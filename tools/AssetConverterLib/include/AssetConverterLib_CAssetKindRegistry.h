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

#ifndef ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#define ASSETCONVERTERLIB_CASSETKINDREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETCONVERTERLIB_CASSETFILEKIND_H
#include "AssetConverterLib_CAssetFileKind.h"
#define ASSETCONVERTERLIB_CASSETFILEKIND_H
#endif /* ASSETCONVERTERLIB_CASSETFILEKIND_H ? */

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
 *  Which kind of asset a file is, from its extension: the table of the asset tools. It starts with a built-in list (the common
 *  model, image, texture, audio, font, document, data and package formats, and the metadata and package files of the engines
 *  that the plugins read) and takes the declarations of the importer and exporter plugins (CIAssetImporter::GetFileKinds()),
 *  which CAssetPluginRegistry hands over when a plugin registers and takes back when it unregisters. A later declaration of
 *  an extension wins over an earlier one; the earlier one is back when the later one goes.
 *
 *  Extensions are looked up without regard to letter case. Thread-safe. The kinds are the values of EAssetKind; GetKindName() is
 *  what a person reads for one and GetKindId() its stable name for settings and filters.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetKindRegistry
{
    public:

    static const CORE::CString BuiltInSource;

    CAssetKindRegistry( void );

    ~CAssetKindRegistry();

    /** The name of a kind as a person reads it: "Image", "Texture", "Particles" */
    static CORE::CString GetKindName( EAssetKind kind );

    /** The stable lower case name of a kind: "image", "texture", "particle" */
    static CORE::CString GetKindId( EAssetKind kind );

    static bool TryParseKindId( const CORE::CString& id, EAssetKind& kind );

    /** The extension of a path without the dot, in lower case; empty without one */
    static CORE::CString GetExtensionOfPath( const CORE::CString& path );

    /** Adds declarations of a source (a plugin name); returns how many were taken (those with an extension) */
    UInt32 RegisterFileKinds( const TAssetFileKindVector& kinds, const CORE::CString& source );

    /** Takes the declarations of a source back; returns how many went */
    UInt32 UnregisterSource( const CORE::CString& source );

    EAssetKind GetKindOfExtension( const CORE::CString& extension ) const;

    /** The kind of a path by its extension; ASSETKIND_OTHER when there is none or it is not known */
    EAssetKind GetKindOfPath( const CORE::CString& path ) const;

    /** The extensions of the files of a kind in the order they were declared */
    void GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions ) const;

    /** Whether the file is a metadata file of an asset (Unity .meta, Godot .import, Unreal .uexp) */
    bool IsMetadataPath( const CORE::CString& path ) const;

    /** The paths of the assets a metadata file can belong to; false when the path is no metadata file */
    bool GetOwnerPaths( const CORE::CString& metadataPath, CORE::CString::StringVector& owners ) const;

    /** The paths of the metadata files an asset can have */
    void GetMetadataPaths( const CORE::CString& assetPath, CORE::CString::StringVector& metadataPaths ) const;

    UInt32 GetDeclarationCount( void ) const;

    private:

    typedef GUCEF::map< CORE::CString, UInt32 > TIndexMap;

    void RegisterBuiltIns( void );
    void RebuildIndex( void );
    const CAssetFileKind* Find( const CORE::CString& extension ) const;

    CAssetKindRegistry( const CAssetKindRegistry& src );            /**< not implemented */
    CAssetKindRegistry& operator=( const CAssetKindRegistry& src ); /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    TAssetFileKindVector m_entries;
    TIndexMap m_index;              /**< extension to the last entry that declares it */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETKINDREGISTRY_H ? */
