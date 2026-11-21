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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#include "gucefLabLink_Reporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 LinkTimeoutInMs = 15000;
static const UInt32 CallTimeoutInMs = 30000;
static const UInt32 FirstRetryInMs = 5000;
static const UInt32 MaxRetryInMs = 300000;
static const UInt32 UnsupportedRetryInMs = 3600 * 1000;
static const UInt32 GpuProbeIntervalInMs = 60000;
static const UInt32 NoGpuProbeIntervalInMs = 600000;
static const UInt32 MinReportIntervalInSecs = 5;
static const UInt32 MaxReportIntervalInSecs = 300;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNodeReporter::CNodeReporter( const SSettings& settings, const SIdentity& identity )
    : m_settings( settings )
    , m_identity( identity )
    , m_state( STATE_IDLE )
    , m_cluster()
    , m_secret()
    , m_node( GUCEF_NULL )
    , m_session( GUCEF_NULL )
    , m_registry( GUCEF_NULL )
    , m_contract()
    , m_endpoint()
    , m_future( GUCEF_NULL )
    , m_futurePending( false )
    , m_deadlineInMs( 0 )
    , m_nextAttemptInMs( 0 )
    , m_nextReportInMs( 0 )
    , m_failureCount( 0 )
    , m_reportIntervalInSecs( settings.reportIntervalInSecs )
    , m_reportsAccepted( 0 )
    , m_hubClockOffsetInSecs( 0 )
    , m_lastOutcome()
    , m_instanceId( MakeMachineId() )
    , m_startedInUnixSecs( NowInUnixSecs() )
    , m_bootId( BootId() )
    , m_resources()
    , m_gpus()
    , m_nextGpuProbeInMs( 0 )
{GUCEF_TRACE;

    if ( m_reportIntervalInSecs < MinReportIntervalInSecs )
        m_reportIntervalInSecs = MinReportIntervalInSecs;
    if ( m_reportIntervalInSecs > MaxReportIntervalInSecs )
        m_reportIntervalInSecs = MaxReportIntervalInSecs;
}

/*-------------------------------------------------------------------------*/

CNodeReporter::~CNodeReporter()
{GUCEF_TRACE;

    Disconnect();
}

/*-------------------------------------------------------------------------*/

CNodeReporter::EState CNodeReporter::GetState( void ) const { return m_state; }
const CORE::CString& CNodeReporter::GetLastOutcome( void ) const { return m_lastOutcome; }
UInt32 CNodeReporter::GetReportsAccepted( void ) const { return m_reportsAccepted; }
Int64 CNodeReporter::GetHubClockOffsetInSecs( void ) const { return m_hubClockOffsetInSecs; }

/*-------------------------------------------------------------------------*/

bool
CNodeReporter::WantsToConnect( UInt64 monoNowInMs ) const
{GUCEF_TRACE;

    return m_settings.report && STATE_IDLE == m_state && monoNowInMs >= m_nextAttemptInMs;
}

/*-------------------------------------------------------------------------*/

bool
CNodeReporter::IsSessionActive( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_session )
        return false;
    MT::CScopeMutex lock( m_node->GetLock() );
    return m_session->IsActive();
}

/*-------------------------------------------------------------------------*/

/** One pulse of the node when it has a pulse generator of its own; the one of the application is the main loop's */
void
CNodeReporter::PumpNode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node )
        return;
    CORE::PulseGeneratorPtr global = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    CORE::PulseGeneratorPtr own = m_node->GetPulseGenerator();
    if ( !own.IsNULL() && own.GetPointerAlways() != global.GetPointerAlways() )
        own->ForceDirectPulse();
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::Connect( const SClusterSighting& cluster, const CORE::CString& secret, const CORE::CString& token )
{GUCEF_TRACE;

    if ( STATE_IDLE != m_state )
        return;

    m_cluster = cluster;
    m_secret = secret;

    CORE::CString error;
    m_contract = BuildLabContract( error );
    if ( m_contract.IsNULL() )
    {
        Fail( error );
        return;
    }

    DRN::CDRNAuthSettings credentials;
    credentials.authenticatorType = "token";
    credentials.SetSetting( "token", CORE::CVariant( token ) );
    DRN::CDRNNodeSettings nodeSettings;
    nodeSettings.serviceName = "gucef-lab";
    nodeSettings.securityPolicy.requireEncryption = true;
    nodeSettings.auth = credentials;
    m_node = GUCEF_NEW DRN::CDRNNode();
    if ( !m_node->SetSettings( nodeSettings ) || !m_node->Start() )
    {
        Fail( "the DRN node cannot start (is codecspluginMONOCYPHER there?)" );
        return;
    }

    // the link only comes up with the cluster that has the key of the discovery entry
    DRN::CDRNEndpoint endpoint( "tcp://" + cluster.address + ":" + CORE::ToString( cluster.labPort ) );
    if ( !cluster.key.IsNULLOrEmpty() )
        endpoint.GetParams().Set( "key", CORE::CVariant( cluster.key ) );
    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = LabSessionName;
    m_session = m_node->CreateSession();
    if ( GUCEF_NULL == m_session || !m_session->Join( DRN::CDRNEndpointList( endpoint ), joinSettings, &credentials ) )
    {
        Fail( "cannot start to link to " + endpoint.ToString() );
        return;
    }
    m_state = STATE_OPENING;
    m_deadlineInMs = NowInMs() + LinkTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::StartRpc( void )
{GUCEF_TRACE;

    UInt32 hostPeerId = 0;
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        hostPeerId = m_session->GetSequencerPeerId();
    }
    CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex > drnTransport( GUCEF_NEW DRN::CDRNRpcTransport( *m_session, hostPeerId ) );
    COMCORE::CIRpcTransportPtr transport;
    transport = drnTransport;
    m_registry = GUCEF_NEW COMCORE::CRpcContractRegistry();
    m_registry->Register( m_contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    COMCORE::CRpcEndpointSettings settings;
    settings.nodeName = MachineName( m_settings );
    settings.contractRegistry = m_registry;
    settings.requireSecureTransport = true;
    m_endpoint = COMCORE::CRpcEndpoint::Create( m_node->GetPulseGenerator(), settings );
    m_endpoint->AddWantedContract( m_contract->GetName(), 1 );
    if ( !transport->Connect( CORE::CUri() ) || !m_endpoint->Start( transport, true ) )
    {
        Fail( "the RPC session with the cluster cannot start" );
        return;
    }
    m_deadlineInMs = NowInMs() + LinkTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::SendHello( void )
{GUCEF_TRACE;

    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    m_endpoint->ResolveMethod( HelloPath, ref, status );
    ReleaseFuture();
    m_future = GUCEF_NEW CORE::CFutureResult( m_endpoint->CallAsync( HelloPath, m_endpoint->NewRequest( ref ) ) );
    m_futurePending = true;
    m_state = STATE_CHECKING;
    m_deadlineInMs = NowInMs() + CallTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::OnHelloAnswer( void )
{GUCEF_TRACE;

    m_futurePending = false;
    COMCORE::CRpcCallResult result;
    const bool answered = COMCORE::CRpcEndpoint::TryGetCallResult( *m_future, result );
    ReleaseFuture();
    if ( !answered )
    {
        Fail( "Hello was not answered" );
        return;
    }
    if ( !result.status.IsOk() )
    {
        Fail( CORE::CString( HelloPath ) + ": " + result.status.ToString() );
        return;
    }

    // with the lab secret the cluster proves it has it too, over the key of this link; a machine that was approved
    // only talks to the cluster that approved it
    if ( !m_secret.IsNULLOrEmpty() )
    {
        if ( !EqualInConstantTime( result.response.GetString( 3 ), MakeClusterProof( m_secret, m_cluster.key ) ) )
        {
            Fail( "the cluster at " + m_cluster.address + " does not prove it has the lab secret: no reports go to it" );
            return;
        }
    }
    else
    if ( m_identity.clusterKey != m_cluster.key )
    {
        Fail( "the cluster at " + m_cluster.address + " is not the one that approved this machine: no reports go to it" );
        return;
    }
    m_state = STATE_REPORTING;
    m_nextReportInMs = NowInMs();
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::CollectReport( SNodeReport& report )
{GUCEF_TRACE;

    report = SNodeReport();
    report.machineId = m_identity.machineId;
    report.instanceId = m_instanceId;
    report.startedInUnixSecs = m_startedInUnixSecs;
    report.bootId = m_bootId;
    report.name = MachineName( m_settings );
    report.os = OsName();
    report.arch = ArchName();
    report.agentVersion = AgentVersion;
    report.uptimeInSecs = UptimeInSecs();
    report.ephemeral = m_settings.ephemeral;
    report.nodeState = NodeStateAvailable;

    m_resources.Probe( m_settings.stateDir, report );

    // a program is run for the graphics cards: not at every report
    const UInt64 now = NowInMs();
    if ( now >= m_nextGpuProbeInMs )
    {
        const bool found = ProbeNvidiaGpus( m_gpus );
        m_nextGpuProbeInMs = now + ( found ? GpuProbeIntervalInMs : NoGpuProbeIntervalInMs );
    }
    report.gpus = m_gpus;

    SHostInfo native;
    native.name = "native";
    native.state = "ready";
    report.hosts.push_back( native );
    if ( !m_settings.nodeMarkerFile.IsNULLOrEmpty() )
    {
        SHostInfo k3s;
        k3s.name = "k3s-agent";
        k3s.state = CORE::FileExists( m_settings.nodeMarkerFile ) ? "joined" : "absent";
        report.hosts.push_back( k3s );
    }
    DeriveCapabilities( report );
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::SendReport( void )
{GUCEF_TRACE;

    SNodeReport report;
    CollectReport( report );

    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    m_endpoint->ResolveMethod( ReportPath, ref, status );
    COMCORE::CRpcMessage request = m_endpoint->NewRequest( ref );
    NodeReportToMessage( report, request );
    ReleaseFuture();
    m_future = GUCEF_NEW CORE::CFutureResult( m_endpoint->CallAsync( ReportPath, request ) );
    m_futurePending = true;
    m_deadlineInMs = NowInMs() + CallTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::OnReportAnswer( void )
{GUCEF_TRACE;

    m_futurePending = false;
    COMCORE::CRpcCallResult result;
    const bool answered = COMCORE::CRpcEndpoint::TryGetCallResult( *m_future, result );
    ReleaseFuture();
    if ( !answered )
    {
        Fail( "the report was not answered" );
        return;
    }
    if ( COMCORE::RPC_STATUS_UNIMPLEMENTED == result.status.code )
    {
        // a cluster of the version before the reports: nothing is wrong, it only does not take them
        m_lastOutcome = "the cluster at " + m_cluster.address + " does not take node reports";
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << m_lastOutcome );
        CloseLink();
        m_state = STATE_UNSUPPORTED;
        m_nextAttemptInMs = NowInMs() + UnsupportedRetryInMs;
        return;
    }
    if ( !result.status.IsOk() )
    {
        Fail( CORE::CString( ReportPath ) + ": " + result.status.ToString() );
        return;
    }

    if ( 0 == m_reportsAccepted || m_failureCount > 0 )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Reporting to cluster " << m_cluster.name << " at " << m_cluster.address <<
                          ( result.response.GetBool( 2 ) ? "" : " (as an unverified machine: it has no approval)" ) );
    ++m_reportsAccepted;
    m_failureCount = 0;
    m_lastOutcome = "reported to cluster " + m_cluster.name;

    const UInt64 hubTime = result.response.GetUInt64( 1 );
    if ( 0 != hubTime )
    {
        const Int64 offset = (Int64) hubTime - (Int64) NowInUnixSecs();
        if ( ( offset > 120 || offset < -120 ) && ( offset - m_hubClockOffsetInSecs > 60 || m_hubClockOffsetInSecs - offset > 60 ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The clock of the hub differs " << offset << " seconds from this machine's" );
        m_hubClockOffsetInSecs = offset;
    }
    const UInt32 told = result.response.GetUInt32( 3 );
    if ( 0 != told )
        m_reportIntervalInSecs = told < MinReportIntervalInSecs ? MinReportIntervalInSecs : ( told > MaxReportIntervalInSecs ? MaxReportIntervalInSecs : told );
    m_nextReportInMs = NowInMs() + (UInt64) m_reportIntervalInSecs * 1000;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::Update( void )
{GUCEF_TRACE;

    if ( STATE_UNSUPPORTED == m_state )
    {
        if ( NowInMs() >= m_nextAttemptInMs )
            m_state = STATE_IDLE;
        return;
    }
    if ( STATE_IDLE == m_state )
        return;

    PumpNode();
    const UInt64 now = NowInMs();

    // a link that was lost (the hub stopped, the network went) is not waited on until a call times out
    if ( ( STATE_CHECKING == m_state || STATE_REPORTING == m_state ) && !IsSessionActive() )
    {
        Fail( "the link to the cluster at " + m_cluster.address + " was lost" );
        return;
    }
    switch ( m_state )
    {
        case STATE_OPENING:
        {
            if ( m_endpoint.IsNULL() )
            {
                if ( IsSessionActive() )
                    StartRpc();
                else
                if ( now > m_deadlineInMs )
                    Fail( "the cluster at " + m_cluster.address + ":" + CORE::ToString( m_cluster.labPort ) + " did not take the link: not reachable, or it refused the token or the address" );
            }
            else
            {
                if ( m_endpoint->IsContractBound( m_contract->GetName(), 1 ) )
                    SendHello();
                else
                if ( now > m_deadlineInMs )
                    Fail( "the cluster does not offer " + m_contract->GetName() );
            }
            break;
        }
        case STATE_CHECKING:
        {
            if ( m_future->IsReady() )
                OnHelloAnswer();
            else
            if ( now > m_deadlineInMs )
                Fail( "Hello was not answered" );
            break;
        }
        case STATE_REPORTING:
        {
            if ( m_futurePending )
            {
                if ( m_future->IsReady() )
                    OnReportAnswer();
                else
                if ( now > m_deadlineInMs )
                    Fail( "the report was not answered" );
            }
            else
            if ( now >= m_nextReportInMs )
                SendReport();
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::Fail( const CORE::CString& problem )
{GUCEF_TRACE;

    m_lastOutcome = problem;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Reporting to the cluster stopped: " << problem );
    CloseLink();
    m_state = STATE_IDLE;

    // 5 seconds, doubling to 5 minutes
    UInt64 pause = FirstRetryInMs;
    for ( UInt32 i=0; i<m_failureCount && pause < MaxRetryInMs; ++i )
        pause *= 2;
    if ( pause > MaxRetryInMs )
        pause = MaxRetryInMs;
    ++m_failureCount;
    m_nextAttemptInMs = NowInMs() + pause;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::ReleaseFuture( void )
{GUCEF_TRACE;

    GUCEF_DELETE m_future;
    m_future = GUCEF_NULL;
    m_futurePending = false;
}

/*-------------------------------------------------------------------------*/

bool
CNodeReporter::CloseLink( void )
{GUCEF_TRACE;

    m_futurePending = false;
    ReleaseFuture();
    if ( !m_endpoint.IsNULL() )
    {
        m_endpoint->Close( false );
        for ( int i=0; i<20; ++i )
            PumpNode();
        m_endpoint.Unlink();
    }
    if ( GUCEF_NULL != m_node )
    {
        if ( GUCEF_NULL != m_session )
        {
            m_session->Leave();
            for ( int i=0; i<20; ++i )
                PumpNode();
            m_node->DestroySession( m_session );
            m_session = GUCEF_NULL;
        }
        m_node->Stop();
        GUCEF_DELETE m_node;
        m_node = GUCEF_NULL;
    }
    GUCEF_DELETE m_registry;
    m_registry = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNodeReporter::Disconnect( void )
{GUCEF_TRACE;

    if ( STATE_IDLE == m_state && GUCEF_NULL == m_node )
        return;
    CloseLink();
    m_state = STATE_IDLE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
