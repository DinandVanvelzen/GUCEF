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

#ifndef GUCEF_LABLINK_REPORTER_H
#define GUCEF_LABLINK_REPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#include "gucefLabLink_Machine.h"
#include "gucefLabLink_Probes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace DRN { class CDRNNode; class CDRNSession; }; };

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The machine side of the node report: keeps a link to the lab endpoint of the cluster it has linked to, checks the
 *  cluster once more over that link (the proof of the lab secret) and sends a report every interval. Nothing here
 *  waits: opening, the calls and their answers are driven by Update() from the main loop, so a hub that does not
 *  answer costs no time. A failure drops the link and tries again later, with a growing pause; a cluster that does
 *  not know the call is left alone for an hour.
 */
class CNodeReporter
{
    public:

    enum EState
    {
        STATE_IDLE = 0,         /**< no link */
        STATE_OPENING,          /**< the link and the RPC session come up */
        STATE_CHECKING,         /**< Hello is asked, the cluster has to prove itself */
        STATE_REPORTING,        /**< reports go out every interval */
        STATE_UNSUPPORTED       /**< the cluster does not know Report */
    };

    CNodeReporter( const SSettings& settings, const SIdentity& identity );

    ~CNodeReporter();

    /** Whether it is time to open a link: reporting is enabled, there is none and a pause after a failure is over */
    bool WantsToConnect( GUCEF::UInt64 monoNowInMs ) const;

    /** Starts to open the link; the cluster must be one the machine has linked to, the token one for the lab endpoint */
    void Connect( const SClusterSighting& cluster, const GUCEF::CORE::CString& secret, const GUCEF::CORE::CString& token );

    /** From the main loop: pumps the link and does what the state asks for */
    void Update( void );

    /** Closes the link. A later Connect() starts anew */
    void Disconnect( void );

    EState GetState( void ) const;

    /** What happened last, for the log and tests */
    const GUCEF::CORE::CString& GetLastOutcome( void ) const;

    /** The reports the hub has accepted since this object exists */
    GUCEF::UInt32 GetReportsAccepted( void ) const;

    /** Seconds the clock of the hub is ahead of ours, as of the last accepted report */
    GUCEF::Int64 GetHubClockOffsetInSecs( void ) const;

    private:

    CNodeReporter( const CNodeReporter& src );
    CNodeReporter& operator=( const CNodeReporter& src );

    void Fail( const GUCEF::CORE::CString& problem );
    bool IsSessionActive( void );
    void StartRpc( void );
    void PumpNode( void );
    void SendHello( void );
    void SendReport( void );
    void OnHelloAnswer( void );
    void OnReportAnswer( void );
    void CollectReport( SNodeReport& report );
    bool CloseLink( void );
    void ReleaseFuture( void );

    SSettings m_settings;
    const SIdentity& m_identity;
    EState m_state;

    SClusterSighting m_cluster;
    GUCEF::CORE::CString m_secret;
    GUCEF::DRN::CDRNNode* m_node;
    GUCEF::DRN::CDRNSession* m_session;
    GUCEF::COMCORE::CRpcContractRegistry* m_registry;
    GUCEF::COMCORE::CRpcContractPtr m_contract;
    GUCEF::COMCORE::CRpcEndpointPtr m_endpoint;
    GUCEF::CORE::CFutureResult* m_future;
    bool m_futurePending;

    GUCEF::UInt64 m_deadlineInMs;               /**< for the opening, the check and a pending call */
    GUCEF::UInt64 m_nextAttemptInMs;            /**< no link before this moment */
    GUCEF::UInt64 m_nextReportInMs;
    GUCEF::UInt32 m_failureCount;
    GUCEF::UInt32 m_reportIntervalInSecs;
    GUCEF::UInt32 m_reportsAccepted;
    GUCEF::Int64 m_hubClockOffsetInSecs;
    GUCEF::CORE::CString m_lastOutcome;

    GUCEF::CORE::CString m_instanceId;
    GUCEF::UInt64 m_startedInUnixSecs;
    GUCEF::CORE::CString m_bootId;
    CResourceProbe m_resources;
    GUCEF::vector< SGpuInfo > m_gpus;
    GUCEF::UInt64 m_nextGpuProbeInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_REPORTER_H ? */
