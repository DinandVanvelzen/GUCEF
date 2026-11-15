/*
 *  AssetConverterLib: the interface of the importers of source formats
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

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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

class CAssetPackContext;
class CAssetScene;
class CConversionReport;

/*-------------------------------------------------------------------------*/

/**
 *  An importer: reads a source format of a pack into the engine-neutral model. Importers come from plugins (and from
 *  code, as the tests do) and register in CAssetPluginRegistry by GetName().
 *
 *  - GetVersion() is raised whenever the output of the importer for the same input changes; it is part of the cache
 *    key of every asset it imports.
 *  - IndexPack() is called once per pack before any import, on the calling thread: the place to resolve the cross
 *    references of the pack into the index of the context (Unity GUIDs, Godot uid:// and res:// paths, Unreal object
 *    paths). During the imports the context is read only.
 *  - CanImport() and Import() are called from the tasks of the pipeline, several at the same time: they must not keep
 *    state between calls other than what is safe to share.
 *  - Import() states the conventions of the source in the scene (coordinate system, normal map convention), adds the
 *    files it read besides the source as dependencies, and reports what it approximated or skipped. The effective
 *    settings of the asset are in CAssetScene::settings when Import() is called.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetImporter
{
    public:

    CIAssetImporter( void );

    virtual ~CIAssetImporter();

    virtual CORE::CString GetName( void ) const = 0;

    virtual UInt32 GetVersion( void ) const = 0;

    /** Whether the file at the VFS path is a source this importer reads as an asset */
    virtual bool CanImport( const CORE::CString& vfsPath, CAssetPackContext& context ) const = 0;

    virtual bool Import( const CORE::CString& vfsPath, CAssetPackContext& context, CAssetScene& out, CConversionReport& report ) = 0;

    /** Fills the index of the pack; the default has no index */
    virtual bool IndexPack( CAssetPackContext& context );

    /** The defaults of the settings of an asset, the first layer of its effective settings; the default has none */
    virtual void GetDefaultSettings( CORE::CDataNode& settings ) const;

    /**
     *  The post-processors that run when the manifest names none for the source format, in order. The default: the
     *  built-in NameHints, GenerateTangents, PruneEmptyNodes and MergeIdenticalMaterials.
     */
    virtual void GetDefaultPostProcessors( CORE::CString::StringVector& names ) const;

    private:

    CIAssetImporter( const CIAssetImporter& src );            /**< not implemented */
    CIAssetImporter& operator=( const CIAssetImporter& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIAssetImporter, MT::CMutex > TAssetImporterPtr;
typedef CORE::CTGlobalONRegistry< CIAssetImporter, MT::CMutex > TAssetImporterRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */
