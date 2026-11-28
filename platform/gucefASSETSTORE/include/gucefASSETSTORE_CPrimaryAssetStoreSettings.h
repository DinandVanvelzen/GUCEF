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

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#include "gucefCOMCORE_CDatabaseClientSettings.h"
#define GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H ? */
#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORAGELOCATION_H
#include "gucefASSETSTORE_CAssetStorageLocation.h"
#define GUCEF_ASSETSTORE_CASSETSTORAGELOCATION_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORAGELOCATION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H
#include "gucefASSETSTORE_CAssetSourcePolicy.h"
#define GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H
#endif /* GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H ? */
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

/**
 *  The settings of the primary store, part of the .json settings of the application (sections 6 and 9.1).
 *
 *  The defaults are those of the settings of GUCEF (decision D2): one location "default" with its root at
 *  "local-assetstore/local" in the VFS (DefaultVfsRoot), the folder "$MODULEDIR$/../../assets/" mounted there (common/bin/assets/ for the
 *  builds in common/bin), and the catalogue in the vfstables backend under that location. Sources are manual and keep
 *  both the original and the converted content unless a policy says otherwise.
 *
 *  The database backend and its connection come from 'database'. Its password is never kept here: it comes from the
 *  gucefAUTH credential store given to the primary store (credentialProvider and credentialAccount name its slot) or
 *  from a file that holds only the password (databasePasswordFile), and is added to the backend options in memory when
 *  the catalogue opens. Serialize() leaves out a "password" backend option.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CPrimaryAssetStoreSettings : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CAssetStorageLocation > TLocationVector;
    typedef GUCEF::vector< CAssetSourcePolicy > TSourcePolicyVector;

    static const CORE::CString DefaultLocationId;           /**< "default" */
    static const CORE::CString DiskSourceId;                /**< "disk" */
    static const CORE::CString DefaultVfsRoot;              /**< "local-assetstore/local": the root of the default location, below the "local-assetstore" mount that also holds what was downloaded from remote stores */

    COMCORE::CDatabaseClientSettings database;
    CORE::CString credentialProvider;                       /**< the provider name of the slot of the password in the credential store */
    CORE::CString credentialAccount;                        /**< the account key of that slot */
    CORE::CString databasePasswordFile;                     /**< a file that holds the password on its first line */
    TLocationVector locations;
    CORE::CString defaultLocationId;
    TSourcePolicyVector sources;
    CAssetSourcePolicy defaultPolicy;                       /**< for sources without a policy of their own */
    TAssetDuplicatePolicy duplicatePolicy;
    bool copyDiskPacks;                                     /**< packs from disk are copied into the location (true) or referenced in place */
    CORE::CString scratchStagingPath;                       /**< a folder of a local disk (environment variables are resolved) that downloads for the store are staged in first when it is writable and has room for them; empty: none. Never required: without it, or when it cannot take a download, the download is staged in the staging area of its storage location */
    CORE::CString seedDataPath;                             /**< VFS folder of the seed data, default "gucefASSETSTORE" (the data of the module) */
    UInt32 analysisConcurrency;                             /**< content analysis tasks at once, default 4 */
    bool indexAfterIngest;                                  /**< an ingest that left the pack without a pack index (the conversion wrote none, or there was no conversion) has the converter hook build one, default false */
    CORE::CString threadPoolName;                           /**< of the async calls, default "AssetStorePrimary" */
    CORE::CString analysisThreadPoolName;                   /**< of the analysis tasks, default "AssetStoreAnalysis" */

    CPrimaryAssetStoreSettings( void );
    virtual ~CPrimaryAssetStoreSettings() GUCEF_VIRTUAL_OVERRIDE;

    const CAssetStorageLocation* FindLocation( const CORE::CString& locationId ) const;

    /** The policy of the source, the default policy (with the source id) when it has none */
    CAssetSourcePolicy GetPolicy( const CORE::CString& sourceId ) const;

    /** Sets the policy of its source */
    void SetPolicy( const CAssetSourcePolicy& policy );

    /**
     *  The database settings with the password added to the backend options, from the credential store (when given and
     *  the slot is named) or from the password file. False with the reason when a named source of the password cannot
     *  be read. The result holds the password: never log or save it
     */
    bool GetDatabaseSettingsWithCredentials( AUTH::ICredentialStore* credentialStore      ,
                                             COMCORE::CDatabaseClientSettings& settings   ,
                                             CORE::CString& error                         ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */
