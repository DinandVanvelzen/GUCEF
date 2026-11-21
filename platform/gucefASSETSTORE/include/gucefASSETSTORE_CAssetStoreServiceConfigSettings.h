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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H
#include "gucefASSETSTORE_CAssetSettingsRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H ? */

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
 *  The settings of CAssetStoreServiceConfig that a client may read and an administrator may change while the service runs
 *  (gucefASSETSTORE_RemoteService_Design.md 18.1): their descriptors, and the values of a configuration as text by key. The keys
 *  are the dotted paths below the section of the service in the site settings: "reprocess.<member>" (CAssetReprocessSettings),
 *  "downloads.<member>" (the limits of CAssetStoreDownloadSettings), and the members of the service itself. Paths, folders, pool
 *  names, the catalogue, secrets and allowSettingsChanges are not among them: they stay in the site settings file.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreServiceConfigSettings
{
    public:

    static const CORE::CString GroupReprocessing;   /**< "Reprocessing" */
    static const CORE::CString GroupDownloads;      /**< "Downloads" */
    static const CORE::CString GroupService;        /**< "Service" */
    static const CORE::CString ReprocessPrefix;     /**< "reprocess." */
    static const CORE::CString DownloadsPrefix;     /**< "downloads." */

    /** Registers the descriptors; false with the reason when one could not be registered */
    static bool RegisterDescriptors( CAssetSettingsRegistry& registry, CORE::CString& reason );

    /** The value of every registered key of the configuration, as canonical text */
    static void ReadValues( const CAssetSettingsRegistry& registry, const CAssetStoreServiceConfig& config, CORE::CString::StringMap& values );

    /** Writes a value (checked by the registry already) into the configuration; false for a key that is not one of these */
    static bool WriteValue( CAssetStoreServiceConfig& config, const CORE::CString& key, const CORE::CString& value );

    private:

    CAssetStoreServiceConfigSettings( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H ? */
