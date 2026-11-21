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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H
#define GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H

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
 *  The settings of CAssetStoreServiceHost, the server side of the remote asset store service. LoadConfig() refuses a key
 *  it does not know, so a misspelt setting is noticed. Keys as the members.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreServiceHostSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString::StringSet roles;         /**< the device roles that may use the asset store contracts; default "owner" (owner devices first) */
    CORE::CString::StringSet signInRoles;   /**< the device roles that may sign in to online stores, answer their prompts, sign out, and lock and unlock the keyring; default "owner". The role "mcp" is refused here: a machine caller never drives a sign in or the keyring */
    CORE::CString::StringSet learningRoles; /**< the device roles whose callers are machine learning consumers: packs with the licence flag noMachineLearning are not there for them (the pack index operations answer NOT_FOUND, searches leave the packs out). Machine operation is not machine learning: the MCP role (control, search, information) sees every pack, so no role is a learning role by default; a role that exports raw data to a training pipeline is named here; default empty */
    CORE::CString stagingDirectory;         /**< the folder of the upload staging on the file system; empty: uploads are not taken; default "" */
    CORE::CString stagingVfsRoot;           /**< where the staging folder is mounted in the VFS for the ingest of a committed upload; default "assetstore-service/staging" */
    UInt64 maxUploadBytesPerSubject;        /**< bytes of uploads a device may have staged; default 50 GiB */
    UInt64 maxUploadBytesTotal;             /**< bytes of all staged uploads, 0 for no maximum; default 0 */
    UInt32 maxUploadsPerSubject;            /**< open uploads per device; default 2 */
    UInt32 uploadExpiryHours;               /**< an upload not committed is removed after this; default 48 */
    CORE::CString uploadLocationId;         /**< the storage location an upload is ingested into when the request names none; default "" (the store's default) */
    CORE::CString commandJournalDirectory;  /**< the monthly command journal (replies to repeated mutations after a restart); empty: in memory only; default "" */
    UInt32 commandJournalKeepDays;          /**< days of the journal read back at the start; default 7 */
    UInt32 eventBarrierMs;                  /**< how long a mutation waits for the events it caused to be in the session rings before it answers; default 1000 */
    UInt32 jobProgressIntervalMs;           /**< progress of a job goes out at most this often (state changes at once); default 500 */
    CORE::CString shareName;                /**< the name of the share of the primary store root through the DRN resource facility (what a device mounts as a VFS archive); the roles are those of "roles"; empty: no share; default "assets" */

    CAssetStoreServiceHostSettings( void );

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

#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H ? */
