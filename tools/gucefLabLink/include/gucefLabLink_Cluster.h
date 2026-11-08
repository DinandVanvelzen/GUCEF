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

#ifndef GUCEF_LABLINK_CLUSTER_H
#define GUCEF_LABLINK_CLUSTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIDISCOVERYENTRY_H
#include "gucefCOMCORE_CIDiscoveryEntry.h"
#define GUCEF_COMCORE_CIDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CIDISCOVERYENTRY_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#include "gucefCOMCORE_CDiscoveryManager.h"
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H
#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#include "gucefLabLink_Settings.h"
#include "gucefLabLink_Store.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace DRN { class CDRNNode; class CDRNSession; }; namespace COMCORE { class CRpcHost; }; };

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CLabService;
class CAccessService;
class CJoinPolicy;

/**
 *  The cluster side, on a k3s server node: the lab endpoint (DRN, HMAC tokens of the lab secret or of an approval),
 *  the access endpoint (DRN, anonymous: approval requests only), the RPC services on them, and the discovery entry
 *  that announces both. Runs on the pulse generator of the application.
 */
class CClusterRole
{
    public:

    CClusterRole( const SSettings& settings, CStore& store );

    ~CClusterRole();

    /** @return false when it cannot run, problem says why */
    bool Start( GUCEF::CORE::CString& problem );

    void Stop( void );

    /** The public key of the lab endpoint, the form DRN pins */
    GUCEF::CORE::CString GetLabKey( void ) const;

    // for the services

    const SSettings& GetSettings( void ) const;
    CStore& GetStore( void );
    const GUCEF::CORE::CString& GetSecret( void ) const;
    const GUCEF::CORE::CString& GetTokenKey( void ) const;
    const GUCEF::CORE::CString& GetK3sVersion( void ) const;

    /** The address of a peer of the access endpoint, from its principal; empty when it is not known */
    GUCEF::CORE::CString GetAccessPeerAddress( const GUCEF::COMCORE::CRpcPrincipal& principal ) const;

    /** Whether the peer of an RPC call on the lab endpoint may use it: a token of the lab secret, or of a machine that is still approved */
    bool IsAllowed( const GUCEF::CORE::CString& subject, const GUCEF::CORE::CString::StringSet& roles ) const;

    private:

    CClusterRole( const CClusterRole& src );
    CClusterRole& operator=( const CClusterRole& src );

    bool StartEndpoint( bool lab, GUCEF::CORE::CString& problem );
    void Advertise( void );

    SSettings m_settings;
    CStore& m_store;
    GUCEF::CORE::CString m_secret;
    GUCEF::CORE::CString m_tokenKey;
    GUCEF::CORE::CString m_k3sVersion;
    GUCEF::DRN::CDRNNode* m_labNode;
    GUCEF::DRN::CDRNNode* m_accessNode;
    GUCEF::DRN::CDRNSession* m_labSession;
    GUCEF::DRN::CDRNSession* m_accessSession;
    CJoinPolicy* m_labPolicy;
    CJoinPolicy* m_accessPolicy;
    GUCEF::COMCORE::CRpcHost* m_labHost;
    GUCEF::COMCORE::CRpcHost* m_accessHost;
    CLabService* m_labService;
    CAccessService* m_accessService;
    GUCEF::COMCORE::CIDiscoveryEntryPtr m_entry;
    GUCEF::CORE::CString::StringVector m_discoveryClients;
};

/** Makes the discovery clients the settings ask for in the manager of the application: "mdns" and "broadcast" */
bool AddDiscoveryClients( const SSettings& settings, const GUCEF::CORE::CString& namePrefix, GUCEF::CORE::CString::StringVector& names );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CLUSTER_H ? */
