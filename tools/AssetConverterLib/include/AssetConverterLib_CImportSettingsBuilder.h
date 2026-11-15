/*
 *  AssetConverterLib: the effective settings of an asset: defaults, pack rules, presets and its override
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

#ifndef ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#define ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */


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
 *  Builds the effective settings of an asset in layers, each over the one before:
 *
 *  1. the defaults of the importer;
 *  2. the pack-wide settings of the manifest (its rules and "nameHints");
 *  3. the presets of the manifest whose glob matches the path of the asset (and whose importer filter, if any, is
 *     the importer), top to bottom;
 *  4. the override of the asset, <overrides folder>/<asset path>.import.json, when it is there.
 *
 *  A layer wins per key: values (attributes) of a later layer replace earlier ones, objects merge member by member,
 *  and an array replaces an earlier array as a whole. The sources of the layers that applied are listed, for the
 *  sidecar.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CImportSettingsBuilder
{
    public:

    /** Merges 'layer' over 'target' as described above */
    static void Merge( CORE::CDataNode& target, const CORE::CDataNode& layer );

    /** The VFS path of the override of an asset; empty when the manifest names no overrides folder */
    static CORE::CString GetOverridePath( const CAssetPackManifest& manifest, const CORE::CString& assetPath );

    /**
     *  The effective settings of the asset; 'appliedLayers' gets a text per layer that applied ("defaults", "pack",
     *  "preset <n>: <match>", "override <path>"). 'importerDefaults' are those of the importer.
     */
    static void Build( const CORE::CDataNode& importerDefaults ,
                       const CAssetPackManifest& manifest      ,
                       const CORE::CString& assetPath          ,
                       const CORE::CString& importerName       ,
                       CORE::CDataNode& settings               ,
                       CORE::CString::StringVector& appliedLayers );

    /** The settings of a post-processor or validator: those of its stack entry, with the child of the effective settings that has its name over them */
    static void GetPluginSettings( const CORE::CDataNode& effectiveSettings, const CORE::CDataNode& entrySettings, const CORE::CString& pluginName, CORE::CDataNode& pluginSettings );

    private:

    CImportSettingsBuilder( void );
    ~CImportSettingsBuilder();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H ? */
