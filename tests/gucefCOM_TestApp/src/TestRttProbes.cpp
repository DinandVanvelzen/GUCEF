/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COM_CICMPRTTPROBE_H
#include "gucefCOM_CIcmpRttProbe.h"
#define GUCEF_COM_CICMPRTTPROBE_H
#endif /* GUCEF_COM_CICMPRTTPROBE_H ? */

#ifndef GUCEF_COM_CTCPCONNECTRTTPROBE_H
#include "gucefCOM_CTcpConnectRttProbe.h"
#define GUCEF_COM_CTCPCONNECTRTTPROBE_H
#endif /* GUCEF_COM_CTCPCONNECTRTTPROBE_H ? */

#ifndef GUCEF_COM_CUDPECHORTTPROBE_H
#include "gucefCOM_CUdpEchoRttProbe.h"
#define GUCEF_COM_CUDPECHORTTPROBE_H
#endif /* GUCEF_COM_CUDPECHORTTPROBE_H ? */

#ifndef GUCEF_COM_CUDPECHORESPONDER_H
#include "gucefCOM_CUdpEchoResponder.h"
#define GUCEF_COM_CUDPECHORESPONDER_H
#endif /* GUCEF_COM_CUDPECHORESPONDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestRttProbes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt16 EchoResponderPort = 47831;
const UInt16 SilentPort = 47832;
const UInt16 TcpListenPort = 47833;
const UInt16 TcpClosedPort = 47834;

/**
 *  Counts the probe events and keeps the round trip times
 */
class CProbeRecorder : public CORE::CObserver
{
    public:

    UInt32 results;
    UInt32 timeouts;
    UInt32 failures;
    bool finished;
    GUCEF::vector< UInt64 > rtts;
    GUCEF::vector< UInt32 > sequences;

    CProbeRecorder( void )
        : CORE::CObserver()
        , results( 0 )
        , timeouts( 0 )
        , failures( 0 )
        , finished( false )
        , rtts()
        , sequences()
    {GUCEF_TRACE;

    }

    virtual ~CProbeRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CProbeRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const COMCORE::CRttProbeEventData* data = static_cast< const COMCORE::CRttProbeEventData* >( eventData );
        if ( COMCORE::CIRttProbe::ProbeResultEvent == eventId )
        {
            ++results;
            if ( GUCEF_NULL != data )
            {
                rtts.push_back( data->GetRttInMicroSecs() );
                sequences.push_back( data->GetSequence() );
            }
        }
        else if ( COMCORE::CIRttProbe::ProbeTimeoutEvent == eventId )
            ++timeouts;
        else if ( COMCORE::CIRttProbe::ProbeFailedEvent == eventId )
            ++failures;
        else if ( COMCORE::CIRttProbe::ProbeFinishedEvent == eventId )
            finished = true;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRttProbeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "RttProbes" );

    GUCEF_TESTFW_TESTCASE( "Test: The probe methods are created by name" )
        try
        {
            COMCORE::CRttProbeFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetRttProbeFactory();
            const char* methods[] = { "icmp", "tcp-connect", "udp-echo" };
            for ( int i=0; i<3; ++i )
            {
                COMCORE::CIRttProbePtr probe = factory.Create( methods[ i ] );
                ASSERT_FALSE( probe.IsNULL() );
                if ( !probe.IsNULL() )
                    ASSERT_TRUE( probe->GetMethodName() == methods[ i ] );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The echo datagram reads back the same" )
        try
        {
            UInt8 packet[ 16 ];
            const UInt64 time = ( (UInt64) 0x11223344 << 32 ) | (UInt64) 0x55667788;
            COM::CUdpEchoRttProbe::WriteEchoPacket( packet, 0x01020304, time );
            ASSERT_TRUE( COM::CUdpEchoResponder::IsEchoPacket( packet, 16 ) );
            ASSERT_FALSE( COM::CUdpEchoResponder::IsEchoPacket( packet, 15 ) );
            UInt32 sequence = 0;
            UInt64 sendTime = 0;
            ASSERT_TRUE( COM::CUdpEchoRttProbe::ReadEchoPacket( packet, 16, sequence, sendTime ) );
            ASSERT_TRUE( 0x01020304 == sequence );
            ASSERT_TRUE( time == sendTime );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: udp-echo measures round trips against an echo responder" )
        try
        {
            COM::CUdpEchoResponder responder;
            ASSERT_TRUE( responder.Start( EchoResponderPort ) );

            COM::CUdpEchoRttProbe probe;
            ASSERT_TRUE( probe.IsAvailable() );
            CProbeRecorder recorder;
            recorder.SubscribeTo( &probe );

            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", EchoResponderPort ), 3, 20, 1000 ) );
            ASSERT_TRUE( probe.IsActive() );
            ASSERT_TRUE( PumpUntilTrue( recorder.finished, 5000 ) );
            ASSERT_TRUE( 3 == recorder.results );
            ASSERT_TRUE( 0 == recorder.timeouts );
            ASSERT_TRUE( 3 == responder.GetEchoCount() );
            for ( size_t i=0; i<recorder.rtts.size(); ++i )
                ASSERT_TRUE( recorder.rtts[ i ] < 1000000 );
            ASSERT_TRUE( 3 == recorder.sequences.size() && 0 == recorder.sequences[ 0 ] && 2 == recorder.sequences[ 2 ] );
            ASSERT_FALSE( probe.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The echo responder rides along on a socket of the application" )
        try
        {
            COMCORE::CUDPSocket gameSocket( false );
            ASSERT_TRUE( gameSocket.Open( EchoResponderPort ) );
            COM::CUdpEchoResponder responder;
            ASSERT_TRUE( responder.Attach( gameSocket ) );
            ASSERT_TRUE( EchoResponderPort == responder.GetPort() );

            COM::CUdpEchoRttProbe probe;
            CProbeRecorder recorder;
            recorder.SubscribeTo( &probe );
            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", EchoResponderPort ), 2, 10, 1000 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.finished, 5000 ) );
            ASSERT_TRUE( 2 == recorder.results );
            responder.Stop();
            gameSocket.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: udp-echo reports timeouts when nothing answers" )
        try
        {
            COMCORE::CUDPSocket silent( false );
            ASSERT_TRUE( silent.Open( SilentPort ) );

            COM::CUdpEchoRttProbe probe;
            CProbeRecorder recorder;
            recorder.SubscribeTo( &probe );
            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", SilentPort ), 2, 10, 100 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.finished, 5000 ) );
            ASSERT_TRUE( 0 == recorder.results );
            ASSERT_TRUE( 2 == recorder.timeouts );
            silent.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: tcp-connect measures connects to a listening port and refusals of a closed one" )
        try
        {
            COMCORE::CTCPServerSocket server( false );
            ASSERT_TRUE( server.ListenOnPort( TcpListenPort ) );

            COM::CTcpConnectRttProbe probe;
            ASSERT_TRUE( probe.IsAvailable() );
            CProbeRecorder recorder;
            recorder.SubscribeTo( &probe );
            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", TcpListenPort ), 3, 20, 2000 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.finished, 5000 ) );
            ASSERT_TRUE( 3 == recorder.results );
            ASSERT_TRUE( 0 == recorder.timeouts );
            ASSERT_TRUE( 0 == recorder.failures );

            // A refusal is an answer of the host as well
            CProbeRecorder refusedRecorder;
            refusedRecorder.SubscribeTo( &probe );
            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", TcpClosedPort ), 2, 20, 2000 ) );
            ASSERT_TRUE( PumpUntilTrue( refusedRecorder.finished, 5000 ) );
            ASSERT_TRUE( 2 == refusedRecorder.results );

            // Without a port there is nothing to connect to
            ASSERT_FALSE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", 0 ), 1, 20, 2000 ) );
            server.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: icmp pings the loopback address where ICMP is available" )
        GUCEF_TESTFW_SKIP_IF( !COM::CIcmpRttProbe::IsIcmpAvailable(), "ICMP echo is not available to this process" );
        try
        {
            COM::CIcmpRttProbe probe;
            ASSERT_TRUE( probe.IsAvailable() );
            CProbeRecorder recorder;
            recorder.SubscribeTo( &probe );
            ASSERT_TRUE( probe.Start( COMCORE::CHostAddress( "127.0.0.1", 0 ), 2, 50, 1000 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.finished, 10000 ) );
            ASSERT_TRUE( 2 == recorder.results );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
