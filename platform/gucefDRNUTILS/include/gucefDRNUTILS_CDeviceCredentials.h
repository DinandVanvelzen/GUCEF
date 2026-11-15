/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H

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

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a paired client keeps of its pairing with one service: its device id and key, the device token, the service's
 *  static key (pinned) and token issuer key, the security policy and the session endpoints. The secrets (the device
 *  key seed, the token) are CSecretString and are never logged; ToDataNode() leaves them out unless asked.
 *  Kept in a gucefAUTH credential store: one slot "<slotPrefix>" of provider "gucef.drnutils.devices" holding the JSON
 *  form with the secrets (the store encrypts it); needs the JSON codec (dstorepluginJSONPARSER).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceCredentials
{
    public:

    static const char* const StoreProvider;     /**< "gucef.drnutils.devices" */

    CORE::CString serviceName;              /**< the DRN service name of the service ("gucef.assetstore") */
    CORE::CString serviceId;                /**< the service's id (its hostInstance-independent identity, from pairing) */
    CORE::CString drnSessionName;           /**< the DRN session of the session listener */
    CORE::CString deviceId;
    CORE::CString deviceKind;
    CORE::CString::StringSet roles;         /**< as granted at pairing or last renewal; informational (the service decides) */
    AUTH::CSecretString deviceSeedHex;      /**< the Ed25519 seed, 64 hexadecimal characters */
    CORE::CString devicePublicKeyHex;
    AUTH::CSecretString token;              /**< the signed device token, base64 */
    UInt64 tokenExpiresUtcMs;
    CORE::CString serverStaticKeyHex;       /**< pinned */
    CORE::CString issuerPublicKeyHex;
    CORE::CString::StringVector sessionEndpoints;   /**< "tcp://host:port", in order of preference */
    CORE::CDataNode securityPolicy;         /**< the DRNSecurityPolicy document of the service */
    CORE::CDataNode nextSecurityPolicy;     /**< a policy the service announced for later; empty for none */

    CDeviceCredentials( void );

    /** Every member needed to connect is there */
    bool IsComplete( void ) const;

    /** The session endpoints with the pinned key as parameter "key" (DRN pinning); 'hostOverride' "host:port" first when not empty */
    bool GetEndpointList( DRN::CDRNEndpointList& endpoints, const CORE::CString& hostOverride = CORE::CString::Empty ) const;

    /** Replaces the endpoints when the list differs; true when it changed */
    bool UpdateSessionEndpoints( const CORE::CString::StringVector& endpoints );

    void ToDataNode( CORE::CDataNode& node, bool includeSecrets ) const;
    bool FromDataNode( const CORE::CDataNode& node, CORE::CString& error );

    bool SaveToStore( AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error ) const;
    bool LoadFromStore( AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error );
    static bool RemoveFromStore( AUTH::ICredentialStore& store, const CORE::CString& slot );

    /** Forgets the secrets in memory */
    void Wipe( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */
