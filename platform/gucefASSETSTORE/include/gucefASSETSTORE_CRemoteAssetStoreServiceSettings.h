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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H
#include "gucefDRNUTILS_CRemoteServiceConnectionSettings.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H ? */

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

/**
 *  The settings of CRemoteAssetStoreService: the object CAssetStoreServiceConfig::implementationSettings carries (the
 *  "AssetSources/service" object of the viewer). Keys as the members; LoadConfig() refuses a key it does not know.
 *  The address of the service is plain configuration (endpoint, for example tcp://worker.lan:47400); without one the
 *  endpoints the service gave at pairing are used, and with lanDiscovery a client that has none looks on the local
 *  network for its paired service (the pinned key authenticates whatever it finds). Device credentials are kept in a
 *  gucefAUTH credential store: the one of the configuration (CAssetStoreServiceConfig::credentialStore) or a file store in
 *  credentialsDirectory. Nothing here is a secret.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetStoreServiceSettings : public CORE::CIConfigurable
{
    public:

    static const char* const DefaultServiceType;    /**< "_gucef-assetstore._tcp" */

    CORE::CString endpoint;                 /**< "tcp://host:port" or "host:port" of the session listener, tried first; default "" */
    CORE::CString credentialsDirectory;     /**< folder of a file credential store with the device credentials; empty: the store of the configuration; default "" */
    CORE::CString credentialsKeyFile;       /**< an external key file of that store (a mounted secret); empty: its own key file; default "" */
    CORE::CString credentialsSlot;          /**< the account of the credentials in the store; default "assetstore" */
    CORE::CString clientKind;               /**< told in Hello; default "viewer" */
    CORE::CString clientVersion;            /**< default "0.1.0" */
    UInt32 bulkChannels;                    /**< bulk links (files) beside the control link, 0 to 2; default 1 */
    UInt32 connectTimeoutMs;                /**< Open() waits at most this for the connection; default 20000 */
    UInt32 eventWaitMs;                     /**< an answer waits at most this for the events that came before it; default 2000 */
    UInt32 linkTimeoutMs;                   /**< default 15000 */
    UInt32 echoIntervalMs;                  /**< default 1000 */
    UInt32 reconnectBaseMs;                 /**< default 500 */
    UInt32 reconnectMaxMs;                  /**< default 30000 */
    bool autoReconnect;                     /**< default true */
    bool lanDiscovery;                      /**< look on the local network when no endpoint is known; default true */
    CORE::CString lanServiceType;           /**< default DefaultServiceType */
    UInt32 lanFindTimeoutMs;                /**< default 3000 */

    CRemoteAssetStoreServiceSettings( void );

    /** The settings of the client connection of the service */
    void GetConnectionSettings( DRNUTILS::CRemoteServiceConnectionSettings& settings ) const;

    /** The endpoint as "host:port" (without the scheme), empty for none */
    CORE::CString GetHostAndPort( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICESETTINGS_H ? */
