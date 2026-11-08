/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestDRNTestUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

/** where the system cannot be asked for a free port: below the range it hands out for outgoing connections */
static UInt16 g_nextTestPort = 23110;

/*-------------------------------------------------------------------------*/

static UInt64
ElapsedMs( UInt64 startTicks )
{
    static const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    return (UInt64) ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs );
}

/*-------------------------------------------------------------------------*/

bool
PumpUntil( CTestCondition& condition, UInt32 timeoutInMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    while ( !condition.IsMet() )
    {
        if ( ElapsedMs( startTicks ) > timeoutInMs )
            return false;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

class CCounterCondition : public CTestCondition
{
    public:
    CCounterCondition( const UInt32& counter, UInt32 expected ) : m_counter( counter ), m_expected( expected ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_counter >= m_expected; }
    private:
    const UInt32& m_counter;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------*/

class CFlagCondition : public CTestCondition
{
    public:
    explicit CFlagCondition( const bool& flag ) : m_flag( flag ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_flag; }
    private:
    const bool& m_flag;
};

/*-------------------------------------------------------------------------*/

bool
PumpUntil( const UInt32& counter, UInt32 expected, UInt32 timeoutInMs )
{
    CCounterCondition condition( counter, expected );
    return PumpUntil( condition, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilFlag( const bool& flag, UInt32 timeoutInMs )
{
    CFlagCondition condition( flag );
    return PumpUntil( condition, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

void
PumpFor( UInt32 durationInMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    while ( ElapsedMs( startTicks ) < durationInMs )
    {
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

UInt16
NextTestPort( void )
{
    // a port the system says is free now, for TCP and UDP both (the tests use one port number for links of either),
    // never the same one twice in this process, so that runs of the test application side by side (and other test
    // applications) do not take each other's ports; GUCEF_TEST_PORT_BASE gives a run a range of its own
    // (gucef_test_unique.h). The DRN listeners take no port 0: a node that listens has to know its endpoint before it
    // opens it, to advertise it
    UInt16 port = TESTFW::FindFreeTcpAndUdpPort( 0 );
    if ( 0 != port )
        return port;
    return g_nextTestPort++;
}

/*-------------------------------------------------------------------------*/

bool
EnsureHashCodecPluginLoaded( void )
{
    if ( WEB::CWebSocketHandshake::IsAvailable() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return WEB::CWebSocketHandshake::IsAvailable();
}

/*-------------------------------------------------------------------------*/

bool
EnsureCryptoCodecPluginLoaded( void )
{
    if ( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable();
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
LoopbackEndpoint( const CORE::CString& channelType, UInt16 port, const CORE::CString& params )
{
    CORE::CString scheme = DRN::CDRNEndpoint::SchemeForChannelType( channelType );
    CORE::CString path = channelType.HasSubstr( "websocket", true ) >= 0 ? CORE::CString( "/drn" ) : CORE::CString::Empty;
    return DRN::CDRNEndpoint( scheme + "://127.0.0.1:" + CORE::UInt32ToString( port ) + path + params );
}

/*-------------------------------------------------------------------------*/

CLinkRecorder::CLinkRecorder( void )
    : CORE::CObserver()
    , operationalCount( 0 )
    , closedCount( 0 )
    , refusedCount( 0 )
    , latencyEventCount( 0 )
    , messageCount( 0 )
    , lastClosedReason( DRN::DRN_LINKREASON_NONE )
    , lastRefusedReason( DRN::DRN_LINKREASON_NONE )
    , lastRefusedText()
    , lastLatencyInfo()
    , links()
    , messages()
    , eventThreadId( 0 )
{

}

/*-------------------------------------------------------------------------*/

CLinkRecorder::~CLinkRecorder()
{
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CLinkRecorder::WatchNode( DRN::CDRNNode& node )
{
    SubscribeTo( &node );
}

/*-------------------------------------------------------------------------*/

void
CLinkRecorder::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{
    if ( DRN::CDRNPeerLink::MessageReceivedEvent == eventid )
    {
        const DRN::CDRNLinkMessage& message = static_cast< DRN::CDRNLinkMessageEventData* >( eventdata )->GetData();
        CRecordedMessage recorded;
        recorded.lane = message.lane;
        recorded.msgClass = message.msgClass;
        recorded.opcode = message.opcode;
        recorded.payload.SetDataSize( 0 );
        if ( message.payload.GetDataSize() > 0 )
            recorded.payload.CopyFrom( 0, message.payload.GetDataSize(), message.payload.GetConstBufferPtr() );
        messages.push_back( recorded );
        ++messageCount;
        return;
    }

    eventThreadId = MT::GetCurrentThreadID();
    if ( DRN::CDRNNode::LinkOperationalEvent == eventid )
    {
        const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData();
        links.push_back( info.link );
        if ( !info.link.IsNULL() )
            SubscribeTo( const_cast< DRN::CDRNPeerLink* >( info.link.GetPointerAlways() ), DRN::CDRNPeerLink::MessageReceivedEvent );
        ++operationalCount;
    }
    else
    if ( DRN::CDRNNode::LinkClosedEvent == eventid )
    {
        const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData();
        lastClosedReason = info.closedInfo.reason;
        ++closedCount;
    }
    else
    if ( DRN::CDRNNode::LinkRefusedEvent == eventid )
    {
        const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData();
        lastRefusedReason = info.closedInfo.reason;
        lastRefusedText = info.closedInfo.text;
        ++refusedCount;
    }
    else
    if ( DRN::CDRNNode::LatencyUpdateEvent == eventid )
    {
        lastLatencyInfo = static_cast< DRN::CDRNNodeLinkEventData* >( eventdata )->GetData().latencyInfo;
        ++latencyEventCount;
    }
}

/*-------------------------------------------------------------------------*/

CLinkedPair::CLinkedPair( void )
    : nodeA()
    , nodeB()
    , recorderA()
    , recorderB()
    , connector()
    , linkA()
    , linkB()
{

}

/*-------------------------------------------------------------------------*/

CLinkedPair::~CLinkedPair()
{
    linkA.Unlink();
    linkB.Unlink();
    connector.Unlink();
    recorderA.links.clear();
    recorderB.links.clear();
    nodeA.Stop();
    nodeB.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CLinkedPair::Setup( const DRN::CDRNNodeSettings& settingsA, const DRN::CDRNNodeSettings& settingsB, const DRN::CDRNEndpoint& listenEndpoint, const DRN::CDRNEndpoint& connectEndpoint, UInt32 timeoutInMs )
{
    if ( !nodeA.SetSettings( settingsA ) || !nodeB.SetSettings( settingsB ) )
        return false;
    recorderA.WatchNode( nodeA );
    recorderB.WatchNode( nodeB );
    if ( !nodeB.Listen( listenEndpoint ) )
        return false;
    connector = nodeA.Connect( DRN::CDRNEndpointList( connectEndpoint ) );
    if ( !PumpUntil( recorderA.operationalCount, 1, timeoutInMs ) || !PumpUntil( recorderB.operationalCount, 1, timeoutInMs ) )
        return false;
    linkA = recorderA.links[ 0 ];
    linkB = recorderB.links[ 0 ];
    return !linkA.IsNULL() && !linkB.IsNULL();
}

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/
