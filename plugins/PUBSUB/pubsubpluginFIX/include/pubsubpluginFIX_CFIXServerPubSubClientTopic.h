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

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H

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

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H
#include "pubsubpluginFIX_CFIXClientMessage.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#include "pubsubpluginFIX_CFIXClientSessionFields.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginFIX_CFIXServerPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H ? */

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

class CFIXServerPubSubClient;

/**
 *  One pubsub topic per logical FIX session (per remote SenderCompID).
 *
 *  Topics are auto-created when a new remote SenderCompID connects.
 *  They persist across connection drops; a new connection for the same
 *  SenderCompID triggers a "session bump" (see AttachConnection).
 *
 *  Receive path is zero-copy: raw FIX wire bytes are delivered as linked
 *  ASCII_STRING views into the per-topic receive buffer.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXServerPubSubClientTopic
    : public PUBSUB::CPubSubClientTopic ,
      public CORE::CTSharedObjCreator< CFIXServerPubSubClientTopic, MT::CMutex >
{
    public:

    CFIXServerPubSubClientTopic( CFIXServerPubSubClient* client );

    virtual ~CFIXServerPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClient* GetClient( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool InitializeConnectivity( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPublishingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Subscribe( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Publish( CORE::UInt64& publishActionId           ,
                          const PUBSUB::CIPubSubMsg& msg          ,
                          bool notify                             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    /**
     *  Called by the flow router when an out-of-band replay initiated by this topic has completed.
     *  Logs the completion of the replay for this topic.
     */
    virtual void OnReplayComplete( CORE::UInt64 replayRequestId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attach a new TCP connection for this FIX session.
     *  If a previous connection exists it is bumped (closed after unsubscribing).
     *  The logon bytes already read from the pending buffer are passed in so that
     *  ProcessReceiveBuffer can process them immediately.
     *
     *  @param connection   Accepted server connection (ownership stays with server socket)
     *  @param logonData    Pointer to the Logon bytes already read (may be NULL if none)
     *  @param logonLen     Length of logon data
     *  @param logonFields  Pre-scanned session fields from the Logon message
     */
    void AttachConnection( COMCORE::CTCPServerConnection* connection  ,
                           const void* logonData                      ,
                           CORE::UInt32 logonLen                      ,
                           const CFIXClientSessionFields& logonFields );

    const CFIXServerPubSubClientTopicConfig& GetTopicConfig( void ) const;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    // FIX session state (per topic = per remote session)
    enum ESessionState
    {
        STATE_NO_CONNECTION = 0,
        STATE_WAITING_LOGON = 1,
        STATE_ACTIVE        = 2,
        STATE_LOGGING_OUT   = 3
    };

    typedef CORE::CTEventHandlerFunctor< CFIXServerPubSubClientTopic > TEventCallback;
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;

    void RegisterConnectionEventHandlers( COMCORE::CTCPServerConnection* connection );
    void UnregisterConnectionEventHandlers( COMCORE::CTCPServerConnection* connection );

    void ProcessReceiveBuffer( void );

    void DispatchIncomingMessage( const char* msgStart                  ,
                                  CORE::UInt32 msgLen                   ,
                                  const CFIXClientSessionFields& fields );

    void HandleLogon( const char* msgStart                  ,
                      CORE::UInt32 msgLen                   ,
                      const CFIXClientSessionFields& fields );
    void HandleLogout( const char* msgStart                  ,
                       CORE::UInt32 msgLen                   ,
                       const CFIXClientSessionFields& fields );
    void HandleHeartbeat( const char* msgStart                  ,
                          CORE::UInt32 msgLen                   ,
                          const CFIXClientSessionFields& fields );
    void HandleTestRequest( const char* msgStart                  ,
                            CORE::UInt32 msgLen                   ,
                            const CFIXClientSessionFields& fields );
    void HandleResendRequest( const char* msgStart                  ,
                              CORE::UInt32 msgLen                   ,
                              const CFIXClientSessionFields& fields );
    void HandleSequenceReset( const char* msgStart                  ,
                              CORE::UInt32 msgLen                   ,
                              const CFIXClientSessionFields& fields );
    void HandleReject( const char* msgStart                  ,
                       CORE::UInt32 msgLen                   ,
                       const CFIXClientSessionFields& fields );

    bool SendRawFix( const CORE::CAsciiString& rawMsg );

    bool IsMsgTypePassedByFilter( const char* msgType, CORE::UInt32 msgTypeLen ) const;

    void DeliverToSubscribers( const char* msgStart                  ,
                               CORE::UInt32 msgLen                   ,
                               const CFIXClientSessionFields& fields );

    void
    OnConnectionDataReceived( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    void
    OnConnectionDisconnected( CORE::CNotifier* notifier    ,
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

    CFIXServerPubSubClientTopic( void ); // not implemented

    private:

    CFIXServerPubSubClient*              m_client;
    CFIXServerPubSubClientTopicConfig    m_config;
    COMCORE::CTCPServerConnection*       m_activeConnection;
    CORE::CDynamicBuffer                 m_receiveBuffer;
    CORE::CTimer*                        m_heartbeatTimer;
    CORE::CTimer*                        m_logonTimeoutTimer;
    CORE::UInt64                         m_outgoingSeqNum;
    CORE::UInt64                         m_expectedIncomingSeqNum;
    CORE::UInt64                         m_resendRequestSentForExpectedSeq;
    CORE::UInt32                         m_consecutiveChecksumFailures;
    ESessionState                        m_sessionState;
    bool                                 m_isSubscribed;
    CORE::UInt64                         m_currentReceiveActionId;
    CORE::UInt64                         m_currentPublishActionId;
    MT::CMutex                           m_lock;
    TPubSubMsgsVector                    m_pubsubMsgs;
    TMsgsRecievedEventData               m_pubsubMsgsRefs;
    TPublishActionIdVector               m_publishSuccessActionIds;
    TMsgsPublishedEventData              m_publishSuccessActionEventData;
    TPublishActionIdVector               m_publishFailureActionIds;
    TMsgsPublishFailureEventData         m_publishFailureActionEventData;
};

/*--------------------------------------------------------------------------*/

typedef CFIXServerPubSubClientTopic::TSharedPtrType   CFIXServerPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H ? */
