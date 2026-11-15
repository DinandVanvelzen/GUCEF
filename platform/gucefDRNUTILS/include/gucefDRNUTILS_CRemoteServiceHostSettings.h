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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H
#define GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H

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

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H
#include "gucefCOMCORE_CServiceAdvertiserSettings.h"
#define GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H
#endif /* GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H ? */

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
 *  Settings of a remote service host (CRemoteServiceHost), section "RemoteServiceHost": the listeners, the DRN timing and
 *  pre-authentication limits, tokens, the client session cache, the RPC limits of control and bulk sessions, pairing and
 *  drain (gucefASSETSTORE_RemoteNetworking_Design.md 4 to 7). LoadConfig() refuses a key it does not know (logged), so a
 *  mistyped setting never goes unnoticed; lists are children with values ("sessionEndpoints": [ ... ]) or a comma
 *  separated text. Every limit accepts 0 for "no maximum" where a maximum is not a safety requirement.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceHostSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString serviceName;               /**< the DRN service name of both listeners; part of the pairing key derivation and the shared-secret proof; default "gucef.service" */
    CORE::CString serviceVersion;            /**< told in Hello; default "1.0.0" */
    CORE::CString drnSessionName;            /**< the DRN session of the session listener; the pairing listener's is this plus ".pair"; default "service" */
    UInt16 sessionPort;                      /**< the session listener's port (0: a free port, tests); default 0 */
    UInt16 pairingPort;                      /**< the pairing listener's port (0: a free port, tests); it listens only while a pairing window is open or a pairing waits; default 0 */
    CORE::CString bindAddress;               /**< the address both listeners bind to; empty binds every interface; default "" */
    CORE::CString::StringVector sessionEndpoints; /**< the session endpoints clients are told ("tcp://host:port", in order); empty: tcp://127.0.0.1:<bound port> (one machine only); default empty */
    CORE::CString::StringVector pairingEndpoints; /**< the pairing endpoints an admin is told when it opens a window; empty: tcp://127.0.0.1:<bound port>; default empty */
    UInt32 linkTimeoutMs;                    /**< DRN link timeout of both listeners; default 15000 */
    UInt32 echoIntervalMs;                   /**< DRN echo interval when nothing else was sent; default 1000 */
    UInt32 handshakeTimeoutMs;               /**< per DRN handshake step; default 5000 */
    UInt32 maxPendingHandshakes;             /**< accepted links still in their handshake; default 64 */
    UInt32 maxHandshakesPerAddressPerMinute; /**< accepted channels per remote address per minute; default 60 */
    UInt32 tokenLifetimeDays;                /**< lifetime of a device token; default 90 */
    UInt32 renewBeforeDays;                  /**< RenewToken is allowed in the last days of a token; default 30 */
    UInt32 resumeGraceSeconds;               /**< how long a suspended client session (and an RPC session) is kept; default 600 */
    UInt32 maxSessions;                      /**< client sessions in total; default 64 */
    UInt32 maxSessionsPerDevice;             /**< client sessions per device; default 4 */
    UInt32 maxBulkLinksPerSession;           /**< bulk channels attached to a client session; default 2 */
    UInt32 eventRetentionEvents;             /**< events kept per client session; default 4096 */
    UInt64 eventRetentionBytes;              /**< bytes of events kept per client session; default 2097152 */
    UInt64 sessionBudgetBytes;               /**< bytes of all client sessions (rings and extensions); suspended ones are evicted beyond it; default 268435456 */
    UInt32 controlWindowBytes;               /**< RPC receive window of control sessions; default 1048576 */
    UInt32 controlMaxMessageBytes;           /**< largest RPC message of control sessions; default 4194304 */
    UInt32 controlSendQueueBytes;            /**< RPC send queue of control sessions; default 4194304 */
    UInt32 bulkWindowBytes;                  /**< RPC receive window of bulk sessions (applied by the client; the host's endpoints use the larger of both); default 8388608 */
    UInt32 bulkMaxMessageBytes;              /**< largest RPC message of bulk sessions; default 1048576 */
    UInt32 bulkSendQueueBytes;               /**< RPC send queue of bulk sessions; default 16777216 */
    UInt32 maxConcurrentCalls;               /**< calls a peer may have open at once; default 64 */
    UInt32 pairingDefaultSeconds;            /**< a window open without a time; default 300 */
    UInt32 pairingMaxSeconds;                /**< the longest window; default 900 */
    UInt32 maxFailedProofs;                  /**< wrong codes that close a window; default 5 */
    UInt32 banAfterFailedProofs;             /**< wrong codes from one address that ban it from the pairing listener; default 3 */
    UInt32 banMinutes;                       /**< how long such a ban lasts; default 15 */
    UInt32 confirmationTimeoutSeconds;       /**< how long a pairing waits for the owner's confirmation; default 300 */
    CORE::CString::StringSet requireConfirmationForRoles; /**< windows that grant one of these roles need the fingerprint confirmation; default admin */
    CORE::CString::StringSet adminRoles;     /**< roles that may use gucef.drnutils.devices.Devices; default admin */
    CORE::CString ownerUserId;               /**< the user of every paired device in version 1; default "owner" */
    UInt32 drainSeconds;                     /**< how long running calls get at a drain; default 20 */
    UInt32 expectedBackSeconds;              /**< what SERVICE_STOPPING tells clients; default 30 */
    UInt32 tickIntervalMs;                   /**< how often the host expires sessions, drains events written by other threads and ends the pairing node; default 250 */
    CORE::CString staticPrivateKeyHex;       /**< the static x25519 key (tests); empty: from the credential store; default "" */
    CORE::CString issuerSeedHex;             /**< the Ed25519 seed of the token issuer (tests); empty: from the credential store; default "" */
    bool lanDiscovery;                       /**< answer "which services are there" on the local network and announce the service (COMCORE::CServiceAdvertiser) while it runs and does not drain; a convenience next to the configured endpoints, never a source of trust; default true (set false to stay silent on the LAN) */
    COMCORE::CServiceAdvertiserSettings lanDiscoveryAdvertiser; /**< child "lanDiscoveryAdvertiser": serviceType (default "_gucef-service._tcp", a product sets its own such as "_gucef-assetstore._tcp"), displayName (default serviceName), ttlInSecs (default 30), discoveryClients (default "broadcast,multicast") or "DiscoveryClient" children */
    bool lanDiscoveryKeyFingerprint;         /**< announce the fingerprint of the static key (public: lets a paired client recognise its service, proves nothing); default true */
    bool lanDiscoveryPairingPort;            /**< announce the pairing port (property "pairingPort") when it is a fixed port; default true */
    CORE::CString lanDiscoveryProtocolVersion; /**< the protocol version announced; empty: the major version of gucef.drnutils.session; default "" */

    CRemoteServiceHostSettings( void );

    /** The DRN session of the pairing listener */
    CORE::CString GetPairingSessionName( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H ? */
