/*
 *  AssetWorkbenchLib: what a thread of a pool is asked to do for the connection to a remote asset store service, and what came of it
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

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

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
 *  What CWorkbenchStoreConnection hands to a task of a pool: the pairing of this computer with a service (blocking for up to the
 *  connect timeout, and while the owner of the service has to confirm), or the opening of a remote asset store service (it waits for
 *  the connection and asks the service to describe itself). The fields marked "filled" are the outcome; the data stays after the task.
 *  The pairing code is wiped by the task as soon as it was used.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CStoreConnectionTaskData : public CORE::CICloneable
{
    public:

    enum EKind
    {
        KIND_PAIR = 0 ,
        KIND_CONNECT
    };
    typedef enum EKind TKind;

    TKind kind;

    /** @name Of a pairing */
    ///@{
    CORE::CString pairingEndpoint;      /**< "tcp://host:port" of the pairing listener; empty: the pairing looks on the local network (lanDiscovery) */
    CORE::CString serviceType;          /**< the type of the announcement to look for when there is no endpoint */
    CORE::CString serviceName;          /**< the DRN service name of the service */
    CORE::CString sessionName;          /**< the DRN session of its session listener */
    CORE::CString deviceName;           /**< what the service shows for this computer */
    CORE::CString clientKind;
    CORE::CString credentialsSlot;
    AUTH::CSecretString code;           /**< wiped by the task when it was used */
    AUTH::ICredentialStore* store;      /**< where the credentials of this device are kept: not owned */
    UInt32 timeoutInMs;                 /**< the wait for the connection; for a pairing also the base of the wait for a confirmation */
    bool lanDiscovery;
    ///@}

    /** @name Of the opening of a service */
    ///@{
    ASSETSTORE::TAssetStoreServicePtr service;
    ///@}

    /** @name Filled by the task */
    ///@{
    bool done;                          /**< the task ran to its end */
    bool succeeded;
    CORE::CString problem;              /**< in words, when it did not succeed; never a secret */
    Int32 failure;                      /**< a pairing: DRNUTILS::CDevicePairingClient::TPairingFailure */
    CORE::CString fingerprint;          /**< a pairing: the fingerprint of the key of this device */
    bool transient;                     /**< the opening failed for a reason that a later try may remove (the service could not be reached) */
    CORE::CString serverName;           /**< the opening: the name of the service as it said it in its hello */
    CORE::CString serverVersion;
    ASSETSTORE::CAssetServiceInfo info; /**< the opening: what the service says it is and offers */
    ///@}

    CStoreConnectionTaskData( void );
    CStoreConnectionTaskData( const CStoreConnectionTaskData& src );
    virtual ~CStoreConnectionTaskData() GUCEF_VIRTUAL_OVERRIDE;
    CStoreConnectionTaskData& operator=( const CStoreConnectionTaskData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H ? */
