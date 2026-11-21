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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreSignInContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#include "gucefASSETSTORE_CAssetStoreDownloadSettings.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CIAssetConverterHook;
class CIAssetRenderHook;

/**
 *  How an implementation of the asset store service is made (the construction parameter of the factories of
 *  CAssetStoreGlobal::GetServiceFactory()): the implementation by id ("local", or the id a remote implementation is
 *  registered under) and its settings.
 *
 *  - The in-process implementation ("local", CLocalAssetStoreService) uses the settings of the primary store and of the
 *    downloads, the folder of the disk cache of the listing pictures, and the objects of the process: the credential
 *    store of the catalogue password, the converter hook and the render hook (none owned, they must outlive the
 *    service; GUCEF_NULL for none).
 *  - Another implementation takes what it needs from implementationSettings (an object), for example the address of a
 *    server; it ignores what does not apply to it.
 *
 *  The events of the service are raised on the thread of pulseGenerator (the global pulse generator when it is null).
 *  The settings serialise as an object with "implementationId", "imageCacheFolder", "threadPoolName", "loadDownloadQueue",
 *  "startDownloads", "stateDirectory", "thumbnailsDirectory", "resumeIngestsOnStart", "ingestResumeAttempts", the children "primaryStore", "downloads" and "implementationSettings"; the objects do not.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreServiceConfig : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString LocalImplementationId;       /**< "local" */
    static const CORE::CString RemoteImplementationId;      /**< "remote" */
    static const CORE::CString StateVfsRoot;                /**< "assetstore-state": where the folder stateDirectory is mounted in the VFS */
    static const CORE::CString ThumbnailsVfsRoot;           /**< "assetstore-thumbnails/thumbnails": where the folder thumbnailsDirectory is mounted in the VFS (the name ends in "thumbnails" like the folder in a storage location) */
    static const CORE::CString QueueFileName;               /**< "downloads.json": the queue file of the downloads in the state folder, when downloads.queueFilePath is empty */
    static const CORE::CString ServiceStateFileName;        /**< "service-state.json": what the service keeps of the downloads it ingests and of its source policies */

    CORE::CString implementationId;
    CPrimaryAssetStoreSettings primaryStore;
    CAssetStoreDownloadSettings downloads;
    CORE::CString imageCacheFolder;                         /**< VFS folder of the disk cache of the listing pictures, empty for none */
    CORE::CString threadPoolName;                           /**< of the asynchronous forms, default "AssetStoreService" */
    bool loadDownloadQueue;                                 /**< read the saved queue of the downloads when the service opens, default true */
    bool startDownloads;                                    /**< start the download manager when the service opens, default true */
    CORE::CString stateDirectory;                           /**< folder of the files that carry the work of the service over a restart (the queue of the downloads, the downloads that are to be ingested, the policies of the sources set while it ran), mounted at StateVfsRoot; empty: nothing is kept (the in-process default) */
    CORE::CString thumbnailsDirectory;                      /**< folder on the disk (environment variables are resolved) that holds the stored pictures of the assets, mounted at ThumbnailsVfsRoot; empty: the folder "thumbnails" in the storage location (the default location of the store) */
    bool resumeIngestsOnStart;                              /**< when the catalogue opens, resume the ingests that were left staged (a restart, a failed step), default true */
    UInt32 ingestResumeAttempts;                            /**< how many starts resume the same staged ingest (one that ends the process each time is not tried for ever), default 3 */
    CORE::CDataNode implementationSettings;                 /**< what an implementation other than the local one needs */
    CORE::PulseGeneratorPtr pulseGenerator;                 /**< of the events; null: the global pulse generator */
    AUTH::ICredentialStore* credentialStore;                /**< of the password of the catalogue, not owned */
    CIAssetConverterHook* converterHook;                    /**< the conversion step and the pipeline operations, not owned */
    CIAssetRenderHook* renderHook;                          /**< contact sheets and renders, not owned */
    CAssetStoreSignInContext signIn;                        /**< what the stores need to sign in on the server (the in-process implementation only; none of it is owned) */
    UInt32 signInPromptSeconds;                             /**< how long a step of a sign in waits for the person before the sign in ends, default 600 */

    CAssetStoreServiceConfig( void );
    virtual ~CAssetStoreServiceConfig() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H ? */
