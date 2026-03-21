/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
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

#ifndef PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H
#define PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXMESSAGE_H
#include "pubsubpluginFIX_CFIXMessage.h"
#define PUBSUBPLUGIN_FIX_CFIXMESSAGE_H
#endif /* PUBSUBPLUGIN_FIX_CFIXMESSAGE_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H
#include "pubsubpluginFIX_CFIXPubSubClientConfig.h"
#define PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTTOPIC_H
#include "pubsubpluginFIX_CFIXPubSubClientTopic.h"
#define PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTTOPIC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  PubSub client that establishes a FIX protocol session over TCP.
 *  Manages the full FIX session lifecycle (Logon → Active → Logout)
 *  and maps received application messages onto a single pubsub topic.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName;

    CFIXPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CFIXPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetOrCreateTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetDefaultTopicConfig( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig                    ,
                                                                  CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr GetTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Connect( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInitialized( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                    bool includeTopics                        ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientConfig& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    CFIXPubSubClientConfig& GetConfig( void );

    /**
     *  Send a raw FIX message string over the TCP socket.
     *  Used by the topic's Publish() path.
     */
    bool SendRawFix( const CORE::CString& rawMsg );

    /**
     *  Increment outgoing sequence number and return it.
     */
    CORE::UInt64 GetAndIncrementOutgoingSeqNum( void );

    /**
     *  Persist both outgoing and incoming sequence numbers as a bookmark.
     */
    void SaveSequenceBookmark( void );

    /**
     *  Request retransmission of messages starting at the given sequence number.
     *  Sends a ResendRequest to the counterparty.
     */
    void RequestReplayFrom( CORE::UInt64 fromSeqNum );

    bool IsSessionActive( void ) const;

    private:

    // FIX session state
    enum ESessionState
    {
        STATE_DISCONNECTED  = 0,
        STATE_CONNECTING    = 1,
        STATE_LOGGING_IN    = 2,
        STATE_ACTIVE        = 3,
        STATE_LOGGING_OUT   = 4
    };

    void RegisterEventHandlers( void );

    void LoadSequenceBookmark( void );

    void ProcessReceiveBuffer( void );

    void DispatchIncomingMessage( const CORE::CString& rawMsg );

    void HandleLogon( const CFIXMessage& msg );
    void HandleLogout( const CFIXMessage& msg );
    void HandleHeartbeat( const CFIXMessage& msg );
    void HandleTestRequest( const CFIXMessage& msg );
    void HandleResendRequest( const CFIXMessage& msg );
    void HandleSequenceReset( const CFIXMessage& msg );
    void HandleReject( const CFIXMessage& msg );

    void ScheduleReconnect( void );

    void
    OnTcpConnected( CORE::CNotifier* notifier    ,
                    const CORE::CEvent& eventId  ,
                    CORE::CICloneable* eventData );

    void
    OnTcpDisconnected( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void
    OnTcpDataReceived( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void
    OnTcpSocketError( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData );

    void
    OnHeartbeatTimerCycle( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void
    OnLogonTimeoutTimerCycle( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    void
    OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    CFIXPubSubClient( void ); // not implemented

    private:

    typedef CORE::CTEventHandlerFunctor< CFIXPubSubClient > TEventCallback;
    typedef GUCEF::map< CORE::CString, CFIXPubSubClientTopicPtr > TTopicMap;

    CFIXPubSubClientConfig     m_fixConfig;
    COMCORE::CTCPClientSocket  m_tcpSocket;
    CORE::CDynamicBuffer       m_receiveBuffer;
    TTopicMap                  m_topicMap;
    CORE::CTimer*              m_heartbeatTimer;
    CORE::CTimer*              m_logonTimeoutTimer;
    CORE::CTimer*              m_reconnectTimer;
    CORE::UInt64               m_outgoingSeqNum;
    CORE::UInt64               m_expectedIncomingSeqNum;
    ESessionState              m_sessionState;
    MT::CMutex                 m_lock;
    bool                       m_initialized;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H ? */
