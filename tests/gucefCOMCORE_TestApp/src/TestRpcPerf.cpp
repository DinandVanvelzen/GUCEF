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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#include "gucefCOMCORE_CRpcLoopbackTransport.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_CRPCSESSION_H
#include "gucefCOMCORE_CRpcSession.h"
#define GUCEF_COMCORE_CRPCSESSION_H
#endif /* GUCEF_COMCORE_CRPCSESSION_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#include "TestRpcConformance.h"

#define GUCEF_RPCTEST_ALLOCATION_COUNTER_IMPLEMENTATION
#include "TestRpcAllocationCounter.h"

#include "TestRpcPerf.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace {

/** Loopback pairs on the test thread's pulse generator, or each side on a task of its own */
class CPerfLoopbackTransports : public CIRpcConformanceTransports
{
    public:

    explicit CPerfLoopbackTransports( bool ownThreads ) : CIRpcConformanceTransports(), m_ownThreads( ownThreads ) {}

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_ownThreads ? "loopback, protocol on tasks of its own" : "loopback, protocol on the test thread";
    }

    virtual bool MakePair( COMCORE::CIRpcTransportPtr& initiator, COMCORE::CIRpcTransportPtr& acceptor ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::PulseGeneratorPtr pulseGenerator;
        if ( !m_ownThreads )
            pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        return COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, initiator, acceptor );
    }

    private:

    bool m_ownThreads;
};

/** The transport side of a session in a test: every frame is taken, nothing is delivered anywhere */
class CTestSessionSink : public COMCORE::CRpcSession::CISink
{
    public:

    UInt32 transmitted;

    CTestSessionSink( void ) : COMCORE::CRpcSession::CISink(), transmitted( 0 ) {}

    virtual bool OnSessionTransmit( const UInt8*, UInt32, COMCORE::TRpcProtectionClass ) GUCEF_VIRTUAL_OVERRIDE { ++transmitted; return true; }
    virtual void OnSessionDeliver( const UInt8*, UInt32, const COMCORE::SRpcFrameHeader&, COMCORE::TRpcProtectionClass ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnSessionProtocolViolation( const COMCORE::CRpcStatus& ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnSessionGap( UInt64, UInt64 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnSessionWritable( void ) GUCEF_VIRTUAL_OVERRIDE {}
};

/** A session with no limits that left the transport up */
void
StartTestSession( COMCORE::CRpcSession& session, bool reliableOrdered )
{
    COMCORE::CRpcSession::SSettings settings;
    settings.reliableOrdered = reliableOrdered;
    settings.retentionMaxFrames = 0;
    settings.retentionMaxBytes = 0;
    settings.sendQueueMaxBytes = 0;
    settings.receiveWindowBytes = 64 * 1024 * 1024;
    settings.ackDelayMs = 1000000;
    settings.ackEveryFrames = 0;
    session.Configure( settings );
    session.SetPeerReceiveWindow( 0x7FFFFFFF );
    session.SetTransportUp( true, 1 );
}

/**
 *  The time the session takes for ackCount ACK frames with 64 SACK ranges among the first 128 frames of a retention
 *  of retained frames, in milliseconds
 */
double
SackProcessingMs( UInt32 retained, bool reliableOrdered, UInt32 ackCount )
{
    CTestSessionSink sink;
    COMCORE::CRpcSession session( sink );
    StartTestSession( session, reliableOrdered );
    UInt8 body[ 16 ];
    memset( body, 1, sizeof( body ) );
    for ( UInt32 i=0; i<retained; ++i )
    {
        UInt64 seq = 0;
        COMCORE::CRpcStatus refusal;
        session.QueueFrame( GUCEF_RPC_FRAME_REQUEST, 0, body, sizeof( body ), COMCORE::RPC_PROTECTION_CONFIDENTIAL, 1, seq, refusal );
    }
    CORE::CDynamicBuffer ack;
    CORE::CBinaryWireWriter writer( ack );
    COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_ACK, 0, 0, true, 0 );
    writer.WriteVarUInt32( 1024 * 1024 );
    writer.WriteVarUInt32( 64 );
    for ( UInt32 r=0; r<64; ++r )
    {
        writer.WriteVarUInt64( 1 );     // every second frame from seq 2 on
        writer.WriteVarUInt64( 1 );
    }
    const UInt8* frame = static_cast< const UInt8* >( ack.GetConstBufferPtr() );
    UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<ackCount; ++i )
        session.OnFrameReceived( frame, ack.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL, 2 );
    double ms = (double) ( MT::PrecisionTickCount() - start ) * 1000.0 / (double) MT::PrecisionTimerResolution();
    if ( session.GetRetainedFrameCount() != retained )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcPerf: the session holds " << session.GetRetainedFrameCount() << " frames instead of " << retained );
    return ms;
}

}

/*-------------------------------------------------------------------------*/

void
PerformRpcPerfTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_perf" );

    GUCEF_TESTFW_TESTCASE( "rpc_perf: unary latency and throughput over loopback on the test thread" )
    try
    {
        CPerfLoopbackTransports transports( false );
        SRpcUnaryFigures figures;
        GUCEF_TESTFW_ASSERT_TRUE( RpcPerfUnary( transports, RpcPerfCalls( 2000 ), RpcPerfCalls( 10000 ), figures ) );
        GUCEF_TESTFW_ASSERT_TRUE( figures.medianUs > 0.0 && figures.callsPerSecond > 0.0 );
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_perf: unary latency and throughput over loopback, protocol on tasks of its own" )
    try
    {
        CPerfLoopbackTransports transports( true );
        SRpcUnaryFigures figures;
        GUCEF_TESTFW_ASSERT_TRUE( RpcPerfUnary( transports, RpcPerfCalls( 2000 ), RpcPerfCalls( 10000 ), figures ) );
        GUCEF_TESTFW_ASSERT_TRUE( figures.medianUs > 0.0 && figures.callsPerSecond > 0.0 );

        #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
        // a frame wakes the protocol task of the other side: no wait for the next 1 ms cycle per hop
        GUCEF_TESTFW_ASSERT_TRUE( figures.medianUs < 500.0 );
        #endif
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_perf: SACK ranges cost the same whatever the retention holds; reliable transports ignore them" )
    try
    {
        // each SACK range searched the whole retention: 64 ranges over 16000 retained frames took 16 times as long as
        // over 1000. The ranges index the retention now, and a reliable ordered transport does not look at them
        const UInt32 ackCount = 200;
        for ( UInt32 mode=0; mode<2; ++mode )
        {
            bool reliable = 1 == mode;
            double small = SackProcessingMs( 1000, reliable, ackCount );
            double large = SackProcessingMs( 16000, reliable, ackCount );
            double ratio = large / ( small > 0.05 ? small : 0.05 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcPerf: " << ackCount << " ACKs with 64 SACK ranges, " << ( reliable ? "reliable" : "unreliable" ) <<
                       " transport: 1000 retained " << small << " ms, 16000 retained " << large << " ms (x" << ratio << ")" );
            #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
            GUCEF_TESTFW_ASSERT_TRUE( ratio < 4.0 );
            #endif
        }
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_perf: the session layer allocates nothing per frame once warm (design 15)" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RPCTEST::IsAllocationCountingAvailable(), RPCTEST::GetAllocationCountingUnavailableReason() );

        // a session frame out and acknowledged, a standalone ACK, a STREAM_CREDIT: each allocated its frame buffer
        CTestSessionSink sink;
        COMCORE::CRpcSession session( sink );
        StartTestSession( session, true );
        UInt8 body[ 32 ];
        memset( body, 2, sizeof( body ) );
        CORE::CDynamicBuffer ack( 64, true );
        UInt32 allocations = 0;
        for ( UInt32 round=0; round<2; ++round )
        {
            if ( 1 == round )
                RPCTEST::StartAllocationCounting();
            for ( UInt32 i=0; i<1000; ++i )
            {
                UInt64 seq = 0;
                COMCORE::CRpcStatus refusal;
                session.QueueFrame( GUCEF_RPC_FRAME_REQUEST, 0, body, sizeof( body ), COMCORE::RPC_PROTECTION_CONFIDENTIAL, 2, seq, refusal );
                ack.SetDataSize( 0 );
                CORE::CBinaryWireWriter writer( ack );
                COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_ACK, 0, 0, true, seq );
                writer.WriteVarUInt32( 1024 * 1024 );
                writer.WriteVarUInt32( 0 );
                session.OnFrameReceived( static_cast< const UInt8* >( ack.GetConstBufferPtr() ), ack.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL, 2 );
                session.SendAck( 2 );
                session.SendControlFrame( GUCEF_RPC_FRAME_STREAM_CREDIT, 0, body, 4 );
            }
            if ( 1 == round )
                allocations = RPCTEST::StopAllocationCounting();
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcPerf: session layer allocations for 1000 frames, ACKs and STREAM_CREDITs: " << allocations );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == session.GetRetainedFrameCount() );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == allocations );
        if ( 0 != allocations )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcPerf: the first allocations:\n" << RPCTEST::GetCountedAllocationStacks() );
    }
    catch ( const GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;      // the skip goes to the test case
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
