/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNODESETTINGS_H
#define GUCEF_DRN_CDRNNODESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Settings of a CDRNNode, section "DRNNodeSettings" of a config
 */
class GUCEF_DRN_EXPORT_CPP CDRNNodeSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CORE::CString nodeId;                   /**< identifies the node, a random id by default */
    CORE::CString serviceName;              /**< peers of another service are refused, default "drn" */
    CDRNEndpointList listenEndpoints;       /**< listened on by Start() */
    bool relayEnabled;                      /**< offer the relay service (CDRNRelayService) */
    CORE::CString relayToken;               /**< the relay service wants this token in BIND, and the node binds with it; empty = none */
    UInt32 relayMaxBytesPerSecondPerRealm;  /**< what the relay service forwards per realm, 0 = no limit */
    UInt32 echoIntervalMs;                  /**< default 250 */
    UInt32 latencyUpdateIntervalMs;         /**< default 1000 */
    UInt32 handshakeTimeoutMs;              /**< per handshake step, default 5000 */
    UInt32 linkTimeoutMs;                   /**< default 10000 */
    UInt32 connectTimeoutMs;                /**< per endpoint candidate of the connector, default 2000 */
    UInt32 ackDelayMs;                      /**< default 5 */
    UInt32 pulseIntervalMs;                 /**< how often the node works, default 5 */
    bool ownThread;                         /**< run the node on a thread of its own, default false */
    UInt32 firstSequence;                   /**< first packet sequence of every link, default 1 (for tests of the wrap) */
    CDRNAuthSettings auth;                  /**< the authenticator, for accepting and by default for connecting */
    CDRNSecurityPolicy securityPolicy;      /**< the game's policy for the links: limits, channel types, authenticators, anomalies */
    CORE::CString staticPrivateKey;         /**< the node's static x25519 key in hex; empty: from staticKeyFile */
    CORE::CString staticKeyFile;            /**< file with the static key in hex, made when it does not exist; empty and no key: a new key per run */

    CDRNNodeSettings( void );
    CDRNNodeSettings( const CDRNNodeSettings& src );
    virtual ~CDRNNodeSettings();
    CDRNNodeSettings& operator=( const CDRNNodeSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A random 16 character hex id, empty if the system has no randomness */
    static CORE::CString GenerateNodeId( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNODESETTINGS_H ? */
