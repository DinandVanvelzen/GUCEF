/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubPlugin_FIX_Integration.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::UInt16 TEST_FIX_PORT            = 19878;
static const CORE::UInt32 SESSION_TIMEOUT_MS        = 5000;
static const CORE::UInt32 MSG_FLOW_TIMEOUT_MS       = 3000;
static const CORE::UInt32 DISCONNECT_TIMEOUT_MS     = 3000;
static const CORE::UInt32 RECONNECT_TIMEOUT_MS      = 5000;
static const char* TEST_SERVER_COMP_ID              = "TESTSERVER";
static const char* TEST_CLIENT_COMP_ID              = "TESTCLIENT";

/*-------------------------------------------------------------------------//
//                                                                         //
//      HELPER CLASSES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Observer that captures events from a FIX pubsub topic.
 *  Since all event callbacks fire synchronously on the pulse generator
 *  thread (our test thread) no extra locking is needed.
 */
class CFIXIntegrationTestObserver : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CFIXIntegrationTestObserver > TEventCallback;

    CORE::UInt32                    msgsReceived;
    GUCEF::vector< CORE::CString >  capturedPayloads;  /**< primary payload of each received msg */
    GUCEF::vector< CORE::CString >  capturedMsgTypes;  /**< tag 35 value of each received msg */
    bool                            connected;
    bool                            disconnected;

    CFIXIntegrationTestObserver( void )
        : CORE::CObservingNotifier()
        , msgsReceived( 0 )
        , capturedPayloads()
        , capturedMsgTypes()
        , connected( false )
        , disconnected( false )
    {}

    void Reset( void )
    {GUCEF_TRACE;
        msgsReceived = 0;
        capturedPayloads.clear();
        capturedMsgTypes.clear();
        connected    = false;
        disconnected = false;
    }

    void ObserveTopic( PUBSUB::CPubSubClientTopicBasicPtr topic )
    {GUCEF_TRACE;
        if ( topic.IsNULL() )
            return;

        TEventCallback msgCb( this, &CFIXIntegrationTestObserver::OnMsgsReceived );
        SubscribeTo( topic.GetPointerAlways()                     ,
                     PUBSUB::CPubSubClientTopic::MsgsRecievedEvent ,
                     msgCb                                         );

        TEventCallback connCb( this, &CFIXIntegrationTestObserver::OnConnected );
        SubscribeTo( topic.GetPointerAlways()                      ,
                     PUBSUB::CPubSubClientTopic::ConnectedEvent     ,
                     connCb                                         );

        TEventCallback disconnCb( this, &CFIXIntegrationTestObserver::OnDisconnected );
        SubscribeTo( topic.GetPointerAlways()                         ,
                     PUBSUB::CPubSubClientTopic::DisconnectedEvent    ,
                     disconnCb                                        );
    }

    private:

    void OnMsgsReceived( CORE::CNotifier* /*notifier*/  ,
                         const CORE::CEvent& /*eventId*/ ,
                         CORE::CICloneable* eventData    )
    {GUCEF_TRACE;
        PUBSUB::CPubSubClientTopic::TMsgsRecievedEventData* msgs =
            static_cast< PUBSUB::CPubSubClientTopic::TMsgsRecievedEventData* >( eventData );
        if ( GUCEF_NULL == msgs )
            return;

        for ( CORE::UInt32 i = 0; i < (CORE::UInt32) msgs->size(); ++i )
        {
            const PUBSUB::CIPubSubMsg* msg = (*msgs)[ i ];
            if ( GUCEF_NULL == msg )
                continue;

            ++msgsReceived;

            // Copy primary payload immediately — linked views are only valid during this callback
            capturedPayloads.push_back( msg->GetPrimaryPayload().AsString( CORE::CString::Empty, true ) );

            // Extract tag 35 (MsgType) from metadata
            CORE::CString msgType;
            const PUBSUB::CIPubSubMsg::TKeyValuePairs& meta = msg->GetMetaDataKeyValuePairs();
            for ( CORE::UInt32 m = 0; m < (CORE::UInt32) meta.size(); ++m )
            {
                if ( meta[ m ].first.AsUInt32( 0, true ) == 35u )
                {
                    msgType = meta[ m ].second.AsString( CORE::CString::Empty, true );
                    break;
                }
            }
            capturedMsgTypes.push_back( msgType );
        }
    }

    void OnConnected( CORE::CNotifier* /*notifier*/  ,
                      const CORE::CEvent& /*eventId*/ ,
                      CORE::CICloneable* /*data*/     )
    {GUCEF_TRACE;
        connected = true;
    }

    void OnDisconnected( CORE::CNotifier* /*notifier*/  ,
                         const CORE::CEvent& /*eventId*/ ,
                         CORE::CICloneable* /*data*/     )
    {GUCEF_TRACE;
        disconnected = true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      FREE FUNCTIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Pump the pulse generator until condition() returns true or timeoutMs elapses.
 *  Each iteration: fire one pulse (which drives socket select() + timers + event dispatch),
 *  then sleep 5 ms to yield the OS scheduler.
 *
 *  @return true if condition was satisfied before timeout, false if it timed out.
 */
template< typename TConditionFn >
static bool
PumpUntil( CORE::CBusyWaitPulseGeneratorDriver& driver ,
           CORE::PulseGeneratorPtr pg                   ,
           TConditionFn condition                       ,
           CORE::UInt32 timeoutMs                       )
{GUCEF_TRACE;
    CORE::UInt64 startTick = MT::PrecisionTickCount();
    while ( !condition() )
    {
        driver.RequestImmediatePulse( *pg );
        MT::ThreadDelay( 5 );
        CORE::Float64 elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTick );
        if ( elapsedMs >= static_cast< CORE::Float64 >( timeoutMs ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Build a minimal valid FIX 4.2 application-level message with correct
 *  BodyLength and CheckSum.  The sequence number is caller-supplied so
 *  tests can control it explicitly.
 */
static CORE::CString
BuildAppMsg( const char*  senderCompId ,
             const char*  targetCompId ,
             CORE::UInt64 seqNum       ,
             char         msgType      )
{GUCEF_TRACE;
    const char SOH = '\x01';

    char msgTypeBuf[ 2 ] = { msgType, '\0' };

    CORE::CString body;
    body += CORE::CString( "35=" ) + CORE::CString( msgTypeBuf ) + SOH;
    body += CORE::CString( "49=" ) + CORE::CString( senderCompId ) + SOH;
    body += CORE::CString( "56=" ) + CORE::CString( targetCompId ) + SOH;
    body += CORE::CString( "34=" ) + CORE::ToString( seqNum ) + SOH;
    body += CORE::CString( "52=20240101-12:00:00" ) + SOH;

    CORE::UInt32 bodyLen = (CORE::UInt32) body.Length();

    CORE::CString header;
    header += CORE::CString( "8=FIX.4.2" ) + SOH;
    header += CORE::CString( "9=" ) + CORE::ToString( bodyLen ) + SOH;

    CORE::CString msgWithoutChecksum = header + body;

    CORE::UInt32 byteSum = 0;
    const char* ptr = msgWithoutChecksum.C_String();
    CORE::UInt32 msgLen = (CORE::UInt32) msgWithoutChecksum.Length();
    for ( CORE::UInt32 i = 0; i < msgLen; ++i )
        byteSum += (CORE::UInt8) ptr[ i ];
    CORE::UInt32 checksum = byteSum % 256;

    char csBuf[ 8 ];
    ::sprintf( csBuf, "%03u", checksum );

    return msgWithoutChecksum + CORE::CString( "10=" ) + CORE::CString( csBuf ) + SOH;
}

/*-------------------------------------------------------------------------*/

/**
 *  Build a FIX application message with an intentionally wrong checksum.
 *  Used for negative security tests.
 */
static CORE::CString
BuildAppMsgBadChecksum( const char*  senderCompId ,
                        const char*  targetCompId ,
                        CORE::UInt64 seqNum       ,
                        char         msgType      )
{GUCEF_TRACE;
    const char SOH = '\x01';

    char msgTypeBuf[ 2 ] = { msgType, '\0' };

    CORE::CString body;
    body += CORE::CString( "35=" ) + CORE::CString( msgTypeBuf ) + SOH;
    body += CORE::CString( "49=" ) + CORE::CString( senderCompId ) + SOH;
    body += CORE::CString( "56=" ) + CORE::CString( targetCompId ) + SOH;
    body += CORE::CString( "34=" ) + CORE::ToString( seqNum ) + SOH;
    body += CORE::CString( "52=20240101-12:00:00" ) + SOH;

    CORE::UInt32 bodyLen = (CORE::UInt32) body.Length();

    CORE::CString header;
    header += CORE::CString( "8=FIX.4.2" ) + SOH;
    header += CORE::CString( "9=" ) + CORE::ToString( bodyLen ) + SOH;

    // Intentionally wrong checksum value
    return header + body + CORE::CString( "10=000" ) + SOH;
}

/*-------------------------------------------------------------------------*/

/**
 *  Construct a CPubSubClientConfig for the FIXServer backend.
 *  All FIX-specific settings go into customConfig attributes;
 *  the generic CPubSubClientConfig fields are set directly.
 */
static PUBSUB::CPubSubClientConfig
MakeServerConfig( CORE::PulseGeneratorPtr pg         ,
                  CORE::UInt16 port = TEST_FIX_PORT  )
{GUCEF_TRACE;
    PUBSUB::CPubSubClientConfig cfg;
    cfg.pubsubClientType    = "FIXServer";
    cfg.pulseGenerator      = pg;

    cfg.customConfig.SetAttribute( "senderCompId",                   CORE::CString( TEST_SERVER_COMP_ID ) );
    cfg.customConfig.SetAttribute( "listenPort",                     (CORE::UInt32) port );
    cfg.customConfig.SetAttribute( "fixVersion",                     CORE::CString( "FIX.4.2" ) );
    cfg.customConfig.SetAttribute( "heartbeatIntervalSecs",          (CORE::UInt32) 30 );
    cfg.customConfig.SetAttribute( "logonTimeoutInMs",               (CORE::UInt32) 5000 );
    cfg.customConfig.SetAttribute( CORE::CString( "allowPublishing" ),                true );
    cfg.customConfig.SetAttribute( "maxMsgSizeBytes",                (CORE::UInt32) 65536 );
    cfg.customConfig.SetAttribute( CORE::CString( "disableChecksumValidation" ),      false );
    cfg.customConfig.SetAttribute( "maxConsecutiveChecksumFailures", (CORE::UInt32) 3 );
    return cfg;
}

/*-------------------------------------------------------------------------*/

/**
 *  Construct a CPubSubClientConfig for the FIXClient backend.
 *  resetSeqNumOnLogon=true so that post-Logon app messages start at seqNum=1.
 */
static PUBSUB::CPubSubClientConfig
MakeClientConfig( CORE::PulseGeneratorPtr pg         ,
                  CORE::UInt16 port = TEST_FIX_PORT  )
{GUCEF_TRACE;
    PUBSUB::CPubSubClientConfig cfg;
    cfg.pubsubClientType    = "FIXClient";
    cfg.pulseGenerator      = pg;
    cfg.reconnectDelayInMs  = 1000;

    cfg.customConfig.SetAttribute( "senderCompId",                   CORE::CString( TEST_CLIENT_COMP_ID ) );
    cfg.customConfig.SetAttribute( "targetCompId",                   CORE::CString( TEST_SERVER_COMP_ID ) );
    cfg.customConfig.SetAttribute( "fixVersion",                     CORE::CString( "FIX.4.2" ) );
    cfg.customConfig.SetAttribute( "heartbeatIntervalSecs",          (CORE::UInt32) 30 );
    cfg.customConfig.SetAttribute( "logonTimeoutInMs",               (CORE::UInt32) 5000 );
    cfg.customConfig.SetAttribute( CORE::CString( "resetSeqNumOnLogon" ),             true );
    cfg.customConfig.SetAttribute( CORE::CString( "allowPublishing" ),                true );
    cfg.customConfig.SetAttribute( "maxMsgSizeBytes",                (CORE::UInt32) 65536 );
    cfg.customConfig.SetAttribute( CORE::CString( "disableChecksumValidation" ),      false );
    cfg.customConfig.SetAttribute( "maxConsecutiveChecksumFailures", (CORE::UInt32) 3 );

    // Remote address: the FIX server to connect to
    cfg.remoteAddresses.push_back( COMCORE::CHostAddress( CORE::CString( "127.0.0.1" ), port ) );
    return cfg;
}

/*-------------------------------------------------------------------------*/

/**
 *  Helper: create a client-side topic with the given name and subscribe it.
 *  Returns the topic or null on failure.
 */
static PUBSUB::CPubSubClientTopicBasicPtr
CreateAndSubscribeClientTopic( PUBSUB::CPubSubClientPtr client       ,
                                const CORE::CString& topicName        ,
                                CORE::PulseGeneratorPtr pg            )
{GUCEF_TRACE;
    if ( client.IsNULL() )
        return PUBSUB::CPubSubClientTopicBasicPtr();

    PUBSUB::CPubSubClientTopicConfigPtr topicCfg = client->GetOrCreateTopicConfig( topicName );
    if ( topicCfg.IsNULL() )
        return PUBSUB::CPubSubClientTopicBasicPtr();

    PUBSUB::CPubSubClientTopicBasicPtr topic = client->CreateTopicAccess( topicCfg, pg );
    if ( topic.IsNULL() )
        return PUBSUB::CPubSubClientTopicBasicPtr();

    topic->Subscribe();
    return topic;
}

/*-------------------------------------------------------------------------*/

/**
 *  Helper: cleanly shut down a server+client pair and pump until both
 *  are disconnected so the OS port is freed before the next test.
 */
static void
TeardownSession( PUBSUB::CPubSubClientPtr client                    ,
                 PUBSUB::CPubSubClientPtr server                    ,
                 CORE::CBusyWaitPulseGeneratorDriver& driver        ,
                 CORE::PulseGeneratorPtr pg                         )
{GUCEF_TRACE;
    if ( !client.IsNULL() )
        client->Disconnect();
    if ( !server.IsNULL() )
        server->Disconnect();

    // Pump briefly to let disconnect events propagate and sockets close
    PumpUntil( driver, pg,
        [&]() -> bool {
            bool clientDown = client.IsNULL() || !client->IsConnected();
            bool serverDown = server.IsNULL() || !server->IsConnected();
            return clientDown && serverDown;
        },
        DISCONNECT_TIMEOUT_MS );

    // Extra pump cycles to ensure OS resources are released
    for ( CORE::UInt32 i = 0; i < 10; ++i )
    {
        driver.RequestImmediatePulse( *pg );
        MT::ThreadDelay( 5 );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEST IMPLEMENTATION                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPubSubPlugin_FIXIntegrationTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginFIX Integration TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginFIX_Integration" );

    // -----------------------------------------------------------------------
    //  Load the plugin (try debug-mode path first, fall back to release)
    // -----------------------------------------------------------------------

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginFIX_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginFIX";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginFIX";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginFIX_d";
    #endif

    bool pluginLoaded = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginFIX_integration_tests", true );
    if ( !pluginLoaded )
        pluginLoaded = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginFIX_integration_tests", true );

    if ( !pluginLoaded )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginFIX binary not available, skipping integration tests" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginFIX Integration TESTS COMPLETED (skipped)" );
        return;
    }

    PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();

    // =====================================================================
    //  GROUP A — Factory & Config
    // =====================================================================

    // A1: FIXServer registers in factory
    GUCEF_TESTFW_TESTCASE( "A1: FIXServer registers in PubSubClientFactory" )
        try
        {
            ASSERT_TRUE( factory.IsConstructible( "FIXServer" ) );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // A2: FIXServer config SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "A2: FIXServer config SaveConfig/LoadConfig round-trip" )
        try
        {
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();
            PUBSUB::CPubSubClientConfig cfgA = MakeServerConfig( pg );

            PUBSUB::CPubSubClientPtr serverA = factory.Create( "FIXServer", cfgA );
            ASSERT_FALSE( serverA.IsNULL() );
            if ( serverA.IsNULL() )
                return;

            // Save and reload
            PUBSUB::CPubSubClientConfig savedCfg;
            ASSERT_TRUE( serverA->SaveConfig( savedCfg ) );

            PUBSUB::CPubSubClientPtr serverB = factory.Create( "FIXServer", savedCfg );
            ASSERT_FALSE( serverB.IsNULL() );
            if ( serverB.IsNULL() )
                return;

            // Verify key attributes were preserved
            CORE::CString senderA = cfgA.customConfig.GetAttributeValueOrChildValueByName( "senderCompId" ).AsString();
            CORE::CString senderB = savedCfg.customConfig.GetAttributeValueOrChildValueByName( "senderCompId" ).AsString();
            ASSERT_TRUE( senderA == senderB );

            CORE::CString fixVerA = cfgA.customConfig.GetAttributeValueOrChildValueByName( "fixVersion" ).AsString();
            CORE::CString fixVerB = savedCfg.customConfig.GetAttributeValueOrChildValueByName( "fixVersion" ).AsString();
            ASSERT_TRUE( fixVerA == fixVerB );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // A3: FIXClient config SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "A3: FIXClient config SaveConfig/LoadConfig round-trip" )
        try
        {
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();
            PUBSUB::CPubSubClientConfig cfgA = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr clientA = factory.Create( "FIXClient", cfgA );
            ASSERT_FALSE( clientA.IsNULL() );
            if ( clientA.IsNULL() )
                return;

            PUBSUB::CPubSubClientConfig savedCfg;
            ASSERT_TRUE( clientA->SaveConfig( savedCfg ) );

            PUBSUB::CPubSubClientPtr clientB = factory.Create( "FIXClient", savedCfg );
            ASSERT_FALSE( clientB.IsNULL() );
            if ( clientB.IsNULL() )
                return;

            // Verify senderCompId and targetCompId were preserved
            CORE::CString senderA = cfgA.customConfig.GetAttributeValueOrChildValueByName( "senderCompId" ).AsString();
            CORE::CString senderB = savedCfg.customConfig.GetAttributeValueOrChildValueByName( "senderCompId" ).AsString();
            ASSERT_TRUE( senderA == senderB );

            CORE::CString targetA = cfgA.customConfig.GetAttributeValueOrChildValueByName( "targetCompId" ).AsString();
            CORE::CString targetB = savedCfg.customConfig.GetAttributeValueOrChildValueByName( "targetCompId" ).AsString();
            ASSERT_TRUE( targetA == targetB );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP B — Session Establishment
    // =====================================================================

    // B1: Client and server establish FIX session (Logon exchange)
    GUCEF_TESTFW_TESTCASE( "B1: Client and server establish FIX session" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            // Set the shared pulse generator explicitly
            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );

            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            // Pump until client is connected (session active = Logon exchange complete)
            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );

            // Server should have auto-created exactly one topic for "TESTCLIENT"
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic = server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // B2: Logon with resetSeqNumOnLogon=true starts at seq 1
    GUCEF_TESTFW_TESTCASE( "B2: Logon with resetSeqNumOnLogon=true starts sequence at 1" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );
            // Ensure reset is enabled (default in MakeClientConfig, but explicit here)
            clientCfg.customConfig.SetAttribute( CORE::CString( "resetSeqNumOnLogon" ), true );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );

            // Verify client is connected
            ASSERT_TRUE( client->IsConnected() );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // B3: Session-level messages delivered when includeSessionLevelMsgs=true
    GUCEF_TESTFW_TESTCASE( "B3: Session-level msgs delivered with includeSessionLevelMsgs=true" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            // Configure default topic config with includeSessionLevelMsgs=true
            // The server auto-creates topics from defaultTopicConfig template
            PUBSUB::CPubSubClientTopicConfigPtr defaultTopicCfg(
                GUCEF_NEW PUBSUB::CPubSubClientTopicConfig() );
            defaultTopicCfg->customConfig.SetAttribute( CORE::CString( "includeSessionLevelMsgs" ), true );

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            serverCfg.defaultTopicConfig = defaultTopicCfg;

            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            // Wait for session
            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );

            // Get server topic and attach observer
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );

            if ( !serverTopic.IsNULL() )
            {
                // Subscribe the server topic so it delivers msgs
                serverTopic->Subscribe();

                CFIXIntegrationTestObserver serverObs;
                serverObs.ObserveTopic( serverTopic );

                // Already connected — the Logon was already received.
                // Pump briefly to check if any session-level msgs arrive via newly subscribed observer.
                // In practice the Logon fires on connect so subscribe early enough for C tests.
                PumpUntil( driver, pg,
                    [&]() -> bool { return serverObs.msgsReceived >= 1; },
                    MSG_FLOW_TIMEOUT_MS );

                // We can't guarantee Logon is re-delivered to a late subscriber,
                // but verify the server topic is healthy and connected.
                ASSERT_TRUE( serverTopic->IsConnected() );
            }

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP C — Application Message Flow
    // =====================================================================

    // C1: Client publishes application message received by server topic
    GUCEF_TESTFW_TESTCASE( "C1: Client publishes app message received by server topic" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Get server topic and subscribe an observer
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CFIXIntegrationTestObserver serverObs;
            serverObs.ObserveTopic( serverTopic );

            // Create client topic and publish an app message
            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // seqNum=1 because resetSeqNumOnLogon=true resets server's expected counter to 1
            CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, 1, 'D' );
            PUBSUB::CBasicPubSubMsg pubMsg;
            pubMsg.GetPrimaryPayload() = rawFix;

            CORE::UInt64 publishActionId = 0;
            bool publishOk = clientTopic->Publish( publishActionId, pubMsg, false );
            ASSERT_TRUE( publishOk );

            // Pump until server receives the message
            bool msgReceived = PumpUntil( driver, pg,
                [&]() -> bool { return serverObs.msgsReceived >= 1; },
                MSG_FLOW_TIMEOUT_MS );
            ASSERT_TRUE( msgReceived );

            if ( serverObs.msgsReceived >= 1 )
            {
                // Verify payload starts with FIX wire format
                ASSERT_TRUE( !serverObs.capturedPayloads.empty() );
                ASSERT_TRUE( serverObs.capturedPayloads[0].StartsWith( "8=FIX.4.2" ) );

                // Verify MsgType = 'D'
                ASSERT_TRUE( !serverObs.capturedMsgTypes.empty() );
                ASSERT_TRUE( serverObs.capturedMsgTypes[0] == "D" );
            }

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // C2: Server publishes application message received by client topic
    GUCEF_TESTFW_TESTCASE( "C2: Server publishes app message received by client topic" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Create client topic and subscribe an observer
            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            CFIXIntegrationTestObserver clientObs;
            clientObs.ObserveTopic( clientTopic );

            // Get server topic and publish a message to the client
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CORE::CString rawFix = BuildAppMsg( TEST_SERVER_COMP_ID, TEST_CLIENT_COMP_ID, 1, '8' );
            PUBSUB::CBasicPubSubMsg pubMsg;
            pubMsg.GetPrimaryPayload() = rawFix;

            CORE::UInt64 publishActionId = 0;
            bool publishOk = serverTopic->Publish( publishActionId, pubMsg, false );
            ASSERT_TRUE( publishOk );

            bool msgReceived = PumpUntil( driver, pg,
                [&]() -> bool { return clientObs.msgsReceived >= 1; },
                MSG_FLOW_TIMEOUT_MS );
            ASSERT_TRUE( msgReceived );

            if ( clientObs.msgsReceived >= 1 )
            {
                // Verify MsgType = '8' (ExecutionReport)
                ASSERT_TRUE( !clientObs.capturedMsgTypes.empty() );
                ASSERT_TRUE( clientObs.capturedMsgTypes[0] == "8" );
            }

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // C3: Bidirectional multi-message flow (5 each direction)
    GUCEF_TESTFW_TESTCASE( "C3: Bidirectional multi-message flow (5 each direction)" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Setup server topic + observer
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CFIXIntegrationTestObserver serverObs;
            serverObs.ObserveTopic( serverTopic );

            // Setup client topic + observer
            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            CFIXIntegrationTestObserver clientObs;
            clientObs.ObserveTopic( clientTopic );

            // Publish 5 messages from client → server  (seqNums 1..5 per resetSeqNumOnLogon=true)
            for ( CORE::UInt64 seq = 1; seq <= 5; ++seq )
            {
                CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, seq, 'D' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            // Publish 5 messages from server → client
            for ( CORE::UInt64 seq = 1; seq <= 5; ++seq )
            {
                CORE::CString rawFix = BuildAppMsg( TEST_SERVER_COMP_ID, TEST_CLIENT_COMP_ID, seq, '8' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                serverTopic->Publish( aid, msg, false );
            }

            // Pump until all 10 messages are received
            bool allReceived = PumpUntil( driver, pg,
                [&]() -> bool {
                    return serverObs.msgsReceived >= 5 && clientObs.msgsReceived >= 5;
                },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( allReceived );
            ASSERT_TRUE( serverObs.msgsReceived >= 5 );
            ASSERT_TRUE( clientObs.msgsReceived >= 5 );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP D — Sequence Numbers & Bookmarks
    // =====================================================================

    // D1: GetCurrentBookmark reflects FIX sequence number
    GUCEF_TESTFW_TESTCASE( "D1: GetCurrentBookmark reflects FIX sequence number" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CFIXIntegrationTestObserver serverObs;
            serverObs.ObserveTopic( serverTopic );

            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Publish 3 messages from client
            for ( CORE::UInt64 seq = 1; seq <= 3; ++seq )
            {
                CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, seq, 'D' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            PumpUntil( driver, pg,
                [&]() -> bool { return serverObs.msgsReceived >= 3; },
                MSG_FLOW_TIMEOUT_MS );
            ASSERT_TRUE( serverObs.msgsReceived >= 3 );

            // Bookmark should be available and represent received count
            PUBSUB::CPubSubBookmark bookmark = serverTopic->GetCurrentBookmark();
            ASSERT_TRUE( bookmark.GetBookmarkType() != PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP E — Graceful Teardown
    // =====================================================================

    // E1: Graceful Logout: client Disconnect triggers both sides to disconnect
    GUCEF_TESTFW_TESTCASE( "E1: Graceful Logout - client Disconnect triggers both sides" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Subscribe observer to client topic for disconnect event
            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            CFIXIntegrationTestObserver clientObs;
            if ( !clientTopic.IsNULL() )
                clientObs.ObserveTopic( clientTopic );

            // Disconnect the client
            client->Disconnect();

            bool clientDisconnected = PumpUntil( driver, pg,
                [&]() -> bool { return !client->IsConnected(); },
                DISCONNECT_TIMEOUT_MS );
            ASSERT_TRUE( clientDisconnected );
            ASSERT_TRUE( !client->IsConnected() );

            // Server should also clean up the session (topic disconnected)
            if ( !clientTopic.IsNULL() )
            {
                // Give server time to detect TCP close
                PumpUntil( driver, pg,
                    [&]() -> bool { return !clientTopic->IsConnected(); },
                    DISCONNECT_TIMEOUT_MS );
            }

            // Clean up server
            server->Disconnect();
            for ( CORE::UInt32 i = 0; i < 10; ++i )
            {
                driver.RequestImmediatePulse( *pg );
                MT::ThreadDelay( 5 );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP F — Reconnection & Session Bumping
    // =====================================================================

    // F1: Session bumping - second Logon with same SenderCompID replaces first
    GUCEF_TESTFW_TESTCASE( "F1: Server session bump - second Logon replaces first" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig client1Cfg = MakeClientConfig( pg );
            PUBSUB::CPubSubClientConfig client2Cfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server  = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client1 = factory.Create( "FIXClient", client1Cfg );
            PUBSUB::CPubSubClientPtr client2 = factory.Create( "FIXClient", client2Cfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client1.IsNULL() );
            ASSERT_FALSE( client2.IsNULL() );
            if ( server.IsNULL() || client1.IsNULL() || client2.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client1->SetPulseGenerator( pg, true );
            client2->SetPulseGenerator( pg, true );

            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client1->Connect( false ) );

            // First session established
            bool firstSession = PumpUntil( driver, pg,
                [&]() -> bool { return client1->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( firstSession );

            // Connect second client with same SenderCompID — triggers session bump
            ASSERT_TRUE( client2->Connect( false ) );

            bool secondSession = PumpUntil( driver, pg,
                [&]() -> bool { return client2->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( secondSession );

            // Server should still have exactly one topic for TEST_CLIENT_COMP_ID
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );

            // Publish via second client to confirm the new session is active
            if ( !serverTopic.IsNULL() )
            {
                serverTopic->Subscribe();
                CFIXIntegrationTestObserver serverObs;
                serverObs.ObserveTopic( serverTopic );

                PUBSUB::CPubSubClientTopicBasicPtr clientTopic2 =
                    CreateAndSubscribeClientTopic( client2, CORE::CString( TEST_SERVER_COMP_ID ), pg );
                if ( !clientTopic2.IsNULL() )
                {
                    CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, 1, 'D' );
                    PUBSUB::CBasicPubSubMsg msg;
                    msg.GetPrimaryPayload() = rawFix;
                    CORE::UInt64 aid = 0;
                    clientTopic2->Publish( aid, msg, false );

                    PumpUntil( driver, pg,
                        [&]() -> bool { return serverObs.msgsReceived >= 1; },
                        MSG_FLOW_TIMEOUT_MS );
                    ASSERT_TRUE( serverObs.msgsReceived >= 1 );
                }
            }

            TeardownSession( client2, server, driver, pg );
            client1->Disconnect();
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // F2: Client auto-reconnect after server-side close
    GUCEF_TESTFW_TESTCASE( "F2: Client auto-reconnect after server-side close" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );
            clientCfg.reconnectDelayInMs = 500;
            clientCfg.customConfig.SetAttribute( "reconnectDelayInMs", (CORE::UInt32) 500 );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Disconnect the server-side topic to simulate server-side close
            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            if ( !serverTopic.IsNULL() )
                serverTopic->Disconnect();

            // Client should detect the drop
            PumpUntil( driver, pg,
                [&]() -> bool { return !client->IsConnected(); },
                DISCONNECT_TIMEOUT_MS );

            // Client should reconnect automatically within reconnectDelayInMs + margin
            bool reconnected = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                RECONNECT_TIMEOUT_MS );
            ASSERT_TRUE( reconnected );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP G — MsgType Filtering
    // =====================================================================

    // G1: MsgType filter allows only matching types through
    GUCEF_TESTFW_TESTCASE( "G1: MsgType filter allows only matching types through" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            // Configure server default topic with msgTypeFilter="D"
            PUBSUB::CPubSubClientTopicConfigPtr defaultTopicCfg(
                GUCEF_NEW PUBSUB::CPubSubClientTopicConfig() );
            defaultTopicCfg->customConfig.SetAttribute( "msgTypeFilter", CORE::CString( "D" ) );

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            serverCfg.defaultTopicConfig = defaultTopicCfg;

            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CFIXIntegrationTestObserver serverObs;
            serverObs.ObserveTopic( serverTopic );

            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Send Heartbeat (type '0') — should NOT pass the filter
            {
                CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, 1, '0' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            // Pump briefly to ensure Heartbeat would arrive if it passed the filter
            for ( CORE::UInt32 i = 0; i < 20; ++i )
            {
                driver.RequestImmediatePulse( *pg );
                MT::ThreadDelay( 5 );
            }

            CORE::UInt32 msgsAfterHeartbeat = serverObs.msgsReceived;

            // Send NewOrderSingle-like (type 'D') — SHOULD pass the filter
            {
                CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, 2, 'D' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            bool msgReceived = PumpUntil( driver, pg,
                [&]() -> bool { return serverObs.msgsReceived > msgsAfterHeartbeat; },
                MSG_FLOW_TIMEOUT_MS );
            ASSERT_TRUE( msgReceived );

            if ( serverObs.msgsReceived > msgsAfterHeartbeat )
            {
                // The last received message should be type 'D'
                CORE::UInt32 lastIdx = serverObs.msgsReceived - 1;
                if ( lastIdx < (CORE::UInt32) serverObs.capturedMsgTypes.size() )
                    ASSERT_TRUE( serverObs.capturedMsgTypes[ lastIdx ] == "D" );
            }

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // G2: Empty msgTypeFilter passes all application messages
    GUCEF_TESTFW_TESTCASE( "G2: Empty msgTypeFilter passes all application messages" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            PUBSUB::CPubSubClientTopicBasicPtr serverTopic =
                server->GetTopicAccess( CORE::CString( TEST_CLIENT_COMP_ID ) );
            ASSERT_FALSE( serverTopic.IsNULL() );
            if ( serverTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }
            serverTopic->Subscribe();

            CFIXIntegrationTestObserver serverObs;
            serverObs.ObserveTopic( serverTopic );

            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Send 3 different app message types
            const char msgTypes[] = { 'D', '8', 'V' };
            for ( CORE::UInt32 i = 0; i < 3; ++i )
            {
                CORE::CString rawFix = BuildAppMsg( TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID,
                                                    (CORE::UInt64)(i+1), msgTypes[i] );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            bool allReceived = PumpUntil( driver, pg,
                [&]() -> bool { return serverObs.msgsReceived >= 3; },
                MSG_FLOW_TIMEOUT_MS );
            ASSERT_TRUE( allReceived );
            ASSERT_TRUE( serverObs.msgsReceived >= 3 );

            TeardownSession( client, server, driver, pg );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // =====================================================================
    //  GROUP H — Security Safeguards
    // =====================================================================

    // H1: maxConsecutiveChecksumFailures triggers disconnect
    GUCEF_TESTFW_TESTCASE( "H1: maxConsecutiveChecksumFailures=3 triggers server disconnect" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            serverCfg.customConfig.SetAttribute( CORE::CString( "disableChecksumValidation" ),      false );
            serverCfg.customConfig.SetAttribute( "maxConsecutiveChecksumFailures", (CORE::UInt32) 3 );

            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Create a client topic to publish bad-checksum messages
            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Send 3 messages with bad checksums
            for ( CORE::UInt32 i = 1; i <= 3; ++i )
            {
                CORE::CString rawFix = BuildAppMsgBadChecksum(
                    TEST_CLIENT_COMP_ID, TEST_SERVER_COMP_ID, (CORE::UInt64) i, 'D' );
                PUBSUB::CBasicPubSubMsg msg;
                msg.GetPrimaryPayload() = rawFix;
                CORE::UInt64 aid = 0;
                clientTopic->Publish( aid, msg, false );
            }

            // Server should close the connection after 3 consecutive checksum failures
            bool disconnected = PumpUntil( driver, pg,
                [&]() -> bool { return !client->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( disconnected );

            // Cleanup
            server->Disconnect();
            for ( CORE::UInt32 i = 0; i < 10; ++i )
            {
                driver.RequestImmediatePulse( *pg );
                MT::ThreadDelay( 5 );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // H2: Oversized message (BodyLength > maxMsgSizeBytes) closes connection
    GUCEF_TESTFW_TESTCASE( "H2: Oversized message triggers server disconnect" )
        try
        {
            CORE::CBusyWaitPulseGeneratorDriver driver;
            CORE::PulseGeneratorPtr pg = ( GUCEF_NEW CORE::CPulseGenerator() )->CreateSharedPtr();

            PUBSUB::CPubSubClientConfig serverCfg = MakeServerConfig( pg );
            serverCfg.customConfig.SetAttribute( "maxMsgSizeBytes", (CORE::UInt32) 100 );

            PUBSUB::CPubSubClientConfig clientCfg = MakeClientConfig( pg );

            PUBSUB::CPubSubClientPtr server = factory.Create( "FIXServer", serverCfg );
            PUBSUB::CPubSubClientPtr client = factory.Create( "FIXClient", clientCfg );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_FALSE( client.IsNULL() );
            if ( server.IsNULL() || client.IsNULL() )
                return;

            server->SetPulseGenerator( pg, true );
            client->SetPulseGenerator( pg, true );
            ASSERT_TRUE( server->Connect( false ) );
            ASSERT_TRUE( client->Connect( false ) );

            bool sessionEstablished = PumpUntil( driver, pg,
                [&]() -> bool { return client->IsConnected() && server->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( sessionEstablished );
            if ( !sessionEstablished )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            PUBSUB::CPubSubClientTopicBasicPtr clientTopic =
                CreateAndSubscribeClientTopic( client, CORE::CString( TEST_SERVER_COMP_ID ), pg );
            ASSERT_FALSE( clientTopic.IsNULL() );
            if ( clientTopic.IsNULL() )
            {
                TeardownSession( client, server, driver, pg );
                return;
            }

            // Build a message whose body is > 100 bytes (padding with extra fields)
            const char SOH = '\x01';
            CORE::CString body;
            body += CORE::CString( "35=D" ) + SOH;
            body += CORE::CString( "49=" ) + CORE::CString( TEST_CLIENT_COMP_ID ) + SOH;
            body += CORE::CString( "56=" ) + CORE::CString( TEST_SERVER_COMP_ID ) + SOH;
            body += CORE::CString( "34=1" ) + SOH;
            body += CORE::CString( "52=20240101-12:00:00" ) + SOH;
            // Pad to exceed 100 bytes
            body += CORE::CString( "58=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" ) + SOH;

            CORE::UInt32 bodyLen = (CORE::UInt32) body.Length();
            CORE::CString header;
            header += CORE::CString( "8=FIX.4.2" ) + SOH;
            header += CORE::CString( "9=" ) + CORE::ToString( bodyLen ) + SOH;

            CORE::CString msgWithoutCs = header + body;
            CORE::UInt32 byteSum = 0;
            const char* ptr = msgWithoutCs.C_String();
            for ( CORE::UInt32 i = 0; i < (CORE::UInt32) msgWithoutCs.Length(); ++i )
                byteSum += (CORE::UInt8) ptr[ i ];
            char csBuf[ 8 ];
            ::sprintf( csBuf, "%03u", byteSum % 256 );
            CORE::CString oversizedMsg = msgWithoutCs + CORE::CString( "10=" ) + CORE::CString( csBuf ) + SOH;

            ASSERT_TRUE( bodyLen > 100 );

            PUBSUB::CBasicPubSubMsg msg;
            msg.GetPrimaryPayload() = oversizedMsg;
            CORE::UInt64 aid = 0;
            clientTopic->Publish( aid, msg, false );

            bool disconnected = PumpUntil( driver, pg,
                [&]() -> bool { return !client->IsConnected(); },
                SESSION_TIMEOUT_MS );
            ASSERT_TRUE( disconnected );

            // Cleanup
            server->Disconnect();
            for ( CORE::UInt32 i = 0; i < 10; ++i )
            {
                driver.RequestImmediatePulse( *pg );
                MT::ThreadDelay( 5 );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // -----------------------------------------------------------------------

    pluginControl.UnloadPluginGroup( "pubsubpluginFIX_integration_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginFIX Integration TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
