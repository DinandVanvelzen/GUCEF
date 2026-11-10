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

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#define GUCEF_DRN_CDRNCONNECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

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

class CDRNNode;

/**
 *  Tries the endpoints of a list in priority order, relay endpoints last: the next one when a candidate failed or has
 *  not become operational (WELCOME) within the timeout, and keeps the first link that is operational.
 *  A refusal by the peer ends the attempt: the other endpoints lead to the same peer.
 *  Created by CDRNNode::Connect(), driven by the node's pulse; events are sent from the node's thread.
 */
class GUCEF_DRN_EXPORT_CPP CDRNConnector : public CORE::CObservingNotifier                            ,
                                           public CORE::CTSharedObjCreator< CDRNConnector, MT::CMutex >
{
    public:

    static const CORE::CEvent ConnectedEvent;              /**< CDRNPeerLinkEventData */
    static const CORE::CEvent ConnectFailedEvent;          /**< CDRNLinkClosedEventData: why the last candidate failed */

    static void RegisterEvents( void );

    typedef CORE::CTSharedPtr< CDRNConnector, MT::CMutex > CDRNConnectorPtr;

    CDRNConnector( void );

    virtual ~CDRNConnector();

    void Initialize( CDRNNode* node, const CDRNEndpointList& endpoints, const CDRNAuthSettings& auth, UInt32 timeoutPerCandidateMs );

    /**
     *  The links of this connector run under the security policy with this hash, the one the acceptor advertised,
     *  instead of the node's. For links to infrastructure, a relay service, whose traffic is protected end to end by
     *  the links over it; never for links to peers of the game. NULL: the node's policy.
     */
    void SetServicePolicyHash( const UInt8* policyHash );

    bool Start( void );

    void Tick( UInt64 nodeNow );

    /** Stops trying, a link that is still connecting is closed */
    void Cancel( void );

    bool IsDone( void ) const;

    bool HasSucceeded( void ) const;

    CDRNPeerLinkPtr GetLink( void ) const;

    const CDRNEndpoint& GetCurrentEndpoint( void ) const;

    UInt32 GetAttemptCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNConnector( const CDRNConnector& src );
    CDRNConnector& operator=( const CDRNConnector& src );

    void TryNext( void );
    void Fail( TDRNLinkReason reason, const CORE::CString& text );
    void DropCandidate( void );

    CDRNNode* m_node;
    CDRNEndpointList m_endpoints;
    CORE::CString m_authenticatorType;
    CIDRNAuthenticatorPtr m_authenticator;
    UInt32 m_timeoutMs;
    bool m_haveServicePolicy;
    UInt8 m_servicePolicyHash[ DRN_HANDSHAKE_HASH_SIZE ];
    UInt32 m_nextIndex;
    UInt32 m_attempts;
    CDRNEndpoint m_currentEndpoint;
    CDRNPeerLinkPtr m_candidate;
    UInt64 m_deadline;
    bool m_done;
    bool m_succeeded;
    bool m_trying;
    TDRNLinkReason m_lastReason;
    CORE::CString m_lastText;
};

/*-------------------------------------------------------------------------*/

typedef CDRNConnector::CDRNConnectorPtr CDRNConnectorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */
