/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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
#ifndef ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H
#define ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#include "gucefASSETSTORE_CAssetStoreDownloadSettings.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H
#include "gucefASSETSTORE_CAssetStoreFormat.h"
#define GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#include "AssetStoreConverterHook_CConverterHookSettings.h"
#define ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#include "gucefASSETSTORE_CRemoteFilesSettings.h"
#define GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  The settings of the asset sources of the workbench, a section "AssetSources" of its .json settings:
 *
 *    - "primaryStore": the settings of the primary store (location, catalogue database, ingest policies), as
 *      ASSETSTORE::CPrimaryAssetStoreSettings has them;
 *    - "downloads": the settings of the download manager (concurrency, retries, throttle, queue file);
 *    - "downloadRoot": the VFS folder that downloads that are not for the primary store go into; empty for the folder
 *      "downloads" of the default location of the primary store;
 *    - "formatPreference": the formats in the order that is chosen from when a store offers several, a text of ids
 *      separated by commas ("gltf,glb,fbx,obj,usd"); formats that are not in the list come after it and an engine
 *      export (Unreal Engine) comes last;
 *    - "mountOnCompletion": a finished download is mounted in the workbench so that it can be browsed and opened;
 *    - "conversionEnabled": the primary store gets the converter hook (conversion during ingest of packs that keep converted
 *      content, and the "Convert" action of the page); "conversion": its settings, as
 *      ASSETSTORECONVERTER::CConverterHookSettings has them (the cook profile of the store, the cook target).
 *    - "keyringIdleLockInSeconds": the saved passwords of the sign in locks again when they were not used for this long (900;
 *      0 never); "credentialFolder": where the sessions and the saved passwords are kept, empty for the folder "Auth" of the
 *      settings folder of the workbench. Only the folder is a setting: no password, code, cookie or token is ever in the settings.
 *    - "service": how the asset store is reached (ASSETSTORE::CIAssetStoreService): "mode" is "local" (the default: the store,
 *      the downloads and the online stores in the process of the workbench, made from the settings above) or the id of another
 *      implementation ("remote": a service on a server, available only in a build that registers such an implementation; the
 *      page says so otherwise). The rest of the object goes to that implementation as its settings.
 *    - "remoteFiles": the files of a remote service on this computer (ASSETSTORE::CRemoteFilesSettings): "cacheFolder" (empty: the folder
 *      "remote" next to the thumbnail cache of the user), "maxCacheBytes" (20 GiB by default: the entries that were used longest ago go
 *      first, the packs that are kept offline stay), "readAheadBlocks", "maxBlocksPerRequest", "maxListEntries" and the others of that class.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetSourcesSettings
{
    public:

    ASSETSTORE::CPrimaryAssetStoreSettings primaryStore;
    ASSETSTORE::CAssetStoreDownloadSettings downloads;
    CORE::CString downloadRoot;
    CORE::CString::StringVector formatPreference;
    bool mountOnCompletion;
    bool conversionEnabled;
    ASSETSTORECONVERTER::CConverterHookSettings conversion;
    UInt32 keyringIdleLockInSeconds;
    CORE::CString credentialFolder;
    CORE::CString serviceMode;                              /**< "local" (the default), or the id of another implementation of the service */
    CORE::CDataNode serviceSettings;                        /**< the object "service": what an implementation other than the local one needs */
    ASSETSTORE::CRemoteFilesSettings remoteFiles;           /**< the object "remoteFiles": the files of a remote store service: the block cache (folder, limit, 20 GB by default), read ahead and the bounds of the lists */

    static const CORE::CString LocalServiceMode;            /**< "local" */

    CAssetSourcesSettings( void );

    /** The formats of the default preference: glTF, GLB, FBX, OBJ, USD */
    static CORE::CString::StringVector GetDefaultFormatPreference( void );

    /** The VFS folder of the downloads that are not for the primary store */
    CORE::CString GetEffectiveDownloadRoot( void ) const;

    /**
     *  The index in 'formats' of the format to download: the first of the preference list that the store offers (the id or the
     *  name of the format, any letter case), else the first that is not an engine export, else the first; -1 without formats
     */
    Int32 ChooseFormat( const ASSETSTORE::TAssetStoreFormatVector& formats ) const;

    /** The formats ordered as the preference list says, the best first and engine exports last; the indices into 'formats' */
    void RankFormats( const ASSETSTORE::TAssetStoreFormatVector& formats, GUCEF::vector< UInt32 >& order ) const;

    /** Writes the section "AssetSources" into the root of the settings */
    bool Save( CORE::CDataNode& globalConfig ) const;

    /** Reads the section "AssetSources" of the root of the settings; what it does not hold keeps its default. False for a section that cannot be read */
    bool Load( const CORE::CDataNode& globalConfig );

    static const CORE::CString SectionName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETSOURCESSETTINGS_H ? */
