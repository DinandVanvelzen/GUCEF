/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINTFIXTURE_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINTFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#include "gucefCOMCORE_CRpcLoopbackTransport.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H ? */

#include "TestRpcSupport.h"

#include "TestRpcLossyTransport.h"

#include "TestRpcNotesService.h"

#include "TestRpcConformance.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Two RPC endpoints connected over a transport pair with capturing decorators on both sides: the fixture of the
 *  endpoint and session suites (and of the tcp suite in gucefCOM_TestApp, which supplies its own transports).
 *  Single thread by default: both sides run on the global pulse generator, which the test thread drives.
 */
class CRpcEndpointFixture
{
    public:

    GUCEF::COMCORE::CRpcContractRegistry clientRegistry;    /**< the contracts the client encodes with */
    GUCEF::COMCORE::CRpcContractRegistry serverRegistry;
    GUCEF::COMCORE::CRpcEndpointSettings clientSettings;
    GUCEF::COMCORE::CRpcEndpointSettings serverSettings;
    GUCEF::COMCORE::CRpcEndpointPtr client;
    GUCEF::COMCORE::CRpcEndpointPtr server;
    GUCEF::COMCORE::CIRpcTransportPtr clientTransport;
    GUCEF::COMCORE::CIRpcTransportPtr serverTransport;
    CRpcLossyTransport* clientWire;
    CRpcLossyTransport* serverWire;
    CRpcNotesTestService service;
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator;
    GUCEF::UInt32 previousDriver;
    CIRpcConformanceTransports* pairMaker;                  /**< the transports under test; NULL: loopback on the test thread */

    explicit CRpcEndpointFixture( CIRpcConformanceTransports* transports = GUCEF_NULL )
        : clientRegistry(), serverRegistry(), clientSettings(), serverSettings(), client(), server(), clientTransport(), serverTransport()
        , clientWire( GUCEF_NULL ), serverWire( GUCEF_NULL ), service(), pulseGenerator(), previousDriver( 0 ), pairMaker( transports )
    {
        pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( GUCEF::MT::GetCurrentTaskID() );
        clientSettings.nodeName = "client";
        serverSettings.nodeName = "server";
        clientSettings.contractRegistry = &clientRegistry;
        serverSettings.contractRegistry = &serverRegistry;
        clientRegistry.SetCacheDirectory( GUCEF::CORE::CString::Empty );
        serverRegistry.SetCacheDirectory( GUCEF::CORE::CString::Empty );
    }

    ~CRpcEndpointFixture()
    {
        if ( !client.IsNULL() )
            client->Close( false );
        if ( !server.IsNULL() )
            server->Close( false );
        RpcTestPump( 20 );
        client.Unlink();
        server.Unlink();
        clientTransport.Unlink();
        serverTransport.Unlink();
        RpcTestPump( 5 );
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }

    /** Loopback transports on the global pulse generator, wrapped by capturing decorators */
    bool MakeLoopbackTransports( void )
    {
        return MakeTransports();
    }

    /** A pair of the transports under test (loopback on the global pulse generator without), wrapped by capturing decorators */
    bool MakeTransports( void )
    {
        GUCEF::COMCORE::CIRpcTransportPtr a;
        GUCEF::COMCORE::CIRpcTransportPtr b;
        if ( GUCEF_NULL != pairMaker )
        {
            if ( !pairMaker->MakePair( a, b ) )
                return false;
        }
        else if ( !GUCEF::COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, a, b ) )
            return false;
        clientTransport = CRpcLossyTransport::Wrap( a, &clientWire );
        serverTransport = CRpcLossyTransport::Wrap( b, &serverWire );
        return true;
    }

    /**
     *  The server exports serverContract's Notes service, the client wants the contract (clientContract registered as
     *  a FILE contract in the client's registry, NULL for none); both start, the handshake and binding run
     */
    bool Connect( const GUCEF::COMCORE::CRpcContractPtr& serverContract, const GUCEF::COMCORE::CRpcContractPtr& clientContract,
                  GUCEF::COMCORE::TRpcExecutionMode mode = GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD, GUCEF::UInt32 wantMajor = 1,
                  bool waitForBinding = true )
    {
        if ( clientTransport.IsNULL() && !MakeLoopbackTransports() )
            return false;
        if ( !clientContract.IsNULL() )
            clientRegistry.Register( clientContract, GUCEF::COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        server = GUCEF::COMCORE::CRpcEndpoint::Create( pulseGenerator, serverSettings );
        client = GUCEF::COMCORE::CRpcEndpoint::Create( pulseGenerator, clientSettings );
        if ( !serverContract.IsNULL() && !server->RegisterService( serverContract, "Notes", &service, mode ) )
            return false;
        client->AddWantedContract( "gucef.sample.notes", wantMajor );
        if ( !server->Start( serverTransport, false ) || !client->Start( clientTransport, true ) )
            return false;
        if ( !waitForBinding )
            return true;
        return PumpUntilBound( "gucef.sample.notes", wantMajor, 2000 );
    }

    bool PumpUntilBound( const char* name, GUCEF::UInt32 major, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( !client->IsContractBound( name, major ) )
        {
            pulseGenerator->ForceDirectPulse();
            if ( GUCEF_NULL != pairMaker )
                pairMaker->Pump();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        RpcTestPump( 5 );
        return true;
    }

    /** Pumps until the future is ready */
    bool PumpUntilReady( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( !future.IsReady() )
        {
            pulseGenerator->ForceDirectPulse();
            if ( GUCEF_NULL != pairMaker )
                pairMaker->Pump();
            if ( future.IsReady() )
                break;
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    /** Pumps until the endpoint reaches the state */
    bool PumpUntilState( const GUCEF::COMCORE::CRpcEndpointPtr& endpoint, GUCEF::COMCORE::TRpcSessionState state, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( endpoint->GetState() != state )
        {
            pulseGenerator->ForceDirectPulse();
            if ( GUCEF_NULL != pairMaker )
                pairMaker->Pump();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    /** Unary call through the async API, pumped; the result */
    GUCEF::COMCORE::CRpcCallResult CallAndPump( const char* path, const GUCEF::COMCORE::CRpcMessage& request,
                                                const GUCEF::COMCORE::CRpcCallOptions& options = GUCEF::COMCORE::CRpcCallOptions(),
                                                GUCEF::UInt32 timeoutMs = 3000 )
    {
        GUCEF::COMCORE::CRpcCallResult result;
        GUCEF::CORE::CFutureResult future = client->CallAsync( path, request, options );
        if ( !PumpUntilReady( future, timeoutMs ) )
        {
            result.status.Set( GUCEF::COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "test timeout" );
            future.RequestCancellation();
            return result;
        }
        GUCEF::COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
        return result;
    }

    GUCEF::COMCORE::CRpcMessage NewClientRequest( const char* path )
    {
        GUCEF::COMCORE::CRpcMethodRef ref;
        GUCEF::COMCORE::CRpcStatus status;
        if ( !client->ResolveMethod( path, ref, status ) )
            return GUCEF::COMCORE::CRpcMessage();
        return client->NewRequest( ref );
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINTFIXTURE_H ? */
