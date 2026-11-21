/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_CIHUB_H
#define GUCEF_LABLINK_CIHUB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#include "gucefLabLink_Settings.h"
#include "gucefLabLink_Store.h"
#include "gucefLabLink_CiRegistryFile.h"
#include "gucefLabLink_CiGatewayTokens.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What CiConfig answers a machine */
struct SCiConfigAnswer
{
    GUCEF::CORE::CString registry;          /**< ci.json, with the object "gateway" of the hub */
    GUCEF::CORE::CString gatewayToken;      /**< the token of the machine */
    GUCEF::CORE::CString caCertificate;     /**< the authority of the gateway's certificate (tls/ca.crt of the MCP directory) */
    GUCEF::CORE::CString gatewayUrl;
};

/**
 *  The CI side of the hub: the registry (CCiRegistryFile) and the gateway tokens (CCiGatewayTokens). Answers
 *  CiConfig for tokens of an approval only, with the machine id the token is bound to as the identity; a token of
 *  the lab secret is made by the machine itself, with a machine id of its own choosing, and gets nothing. Takes the
 *  tokens of machines that are no longer approved away, at the start and once a minute. Thread safe: CiConfig runs
 *  on the threads of the RPC pool.
 */
class CCiHub
{
    public:

    /** What the answer says when the hub has no CI registry: the machine removes what it had */
    static const char* NoCiConfigured;

    CCiHub( const SSettings& settings, CStore& store );

    /** Logs what the hub hands out and makes the hash file follow the approvals */
    void Start( void );

    /** From the main loop: the tokens of machines that are no longer approved are taken away, once a minute */
    void Update( GUCEF::UInt64 nowInMs );

    /**
     *  The answer to CiConfig of a caller of the lab endpoint.
     *  @param subject      the subject of its principal, "drn:<machine id>"
     *  @param roles        the roles of its token: RoleApproved, or RoleLab for a token of the lab secret
     *  @param peerAddress  for the log
     *  @return OK; PERMISSION_DENIED for a token that is not of a current approval; NOT_FOUND (NoCiConfigured) when
     *          the hub has no registry; FAILED_PRECONDITION when the registry is not valid or cannot be checked;
     *          RESOURCE_EXHAUSTED when too many machines have a token; UNAVAILABLE when the tokens cannot be stored
     */
    GUCEF::COMCORE::CRpcStatus Answer( const GUCEF::CORE::CString& subject              ,
                                       const GUCEF::CORE::CString::StringSet& roles     ,
                                       const GUCEF::CORE::CString& peerAddress          ,
                                       SCiConfigAnswer& answer                          );

    CCiGatewayTokens& GetTokens( void );

    CCiRegistryFile& GetRegistry( void );

    /** The gateway URL of the settings, else https://<cluster name>:8792 */
    static GUCEF::CORE::CString GatewayUrlOf( const SSettings& settings );

    /** The registry file of the settings, else ci.json in the state directory */
    static GUCEF::CORE::CString RegistryFileOf( const SSettings& settings );

    private:

    CCiHub( const CCiHub& src );
    CCiHub& operator=( const CCiHub& src );

    void Reconcile( void );

    SSettings m_settings;
    CStore& m_store;
    CCiRegistryFile m_registry;
    CCiGatewayTokens m_tokens;
    GUCEF::UInt64 m_nextReconcileInMs;
    GUCEF::CORE::CString m_lastReconcileProblem;   /**< said once until it changes */
    GUCEF::CORE::CString m_lastTokenProblem;       /**< said once until it changes */
    GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CIHUB_H ? */
