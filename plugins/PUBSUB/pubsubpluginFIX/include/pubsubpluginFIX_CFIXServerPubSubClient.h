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

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H
#include "pubsubpluginFIX_CFIXClientMessage.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#include "pubsubpluginFIX_CFIXClientSessionFields.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H
#include "pubsubpluginFIX_CFIXServerPubSubClientConfig.h"
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H
#include "pubsubpluginFIX_CFIXServerPubSubClientTopic.h"
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPIC_H ? */

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
 *  PubSub client that acts as a FIX protocol TCP server.
 *  Listens for inbound connections, identifies each logical FIX session
 *  by the connecting party's SenderCompID (from its Logon message), and
 *  maps each session to a dedicated pubsub topic.
 *
 *  Session bumping: if a new connection arrives for the same SenderCompID
 *  the old connection is cleanly replaced.
 *
 *  TypeName = "FIXServer"
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXServerPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName;

    CFIXServerPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CFIXServerPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

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

    const CFIXServerPubSubClientConfig& GetConfig( void ) const;

    private:

    /**
     *  State kept for each newly accepted connection while we wait for its Logon.
     *  Once the Logon arrives the connection is promoted to a topic.
     */
    struct SPendingConnectionState
    {
        COMCORE::CTCPServerConnection* connection;
        CORE::CDynamicBuffer           buffer;

        SPendingConnectionState( void )
            : connection( GUCEF_NULL )
            , buffer()
        {}
    };

    typedef GUCEF::map< CORE::CString, CFIXServerPubSubClientTopicPtr >    TTopicMap;
    typedef GUCEF::map< CORE::UInt32, SPendingConnectionState >            TPendingMap;
    typedef CORE::CTEventHandlerFunctor< CFIXServerPubSubClient >          TEventCallback;

    void RegisterEventHandlers( void );

    void
    OnClientConnected( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void
    OnClientDisconnected( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnServerSocketOpened( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnServerSocketClosed( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnServerSocketError( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnPendingConnectionData( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    void SubscribeToPendingConnection( COMCORE::CTCPServerConnection* connection, CORE::UInt32 connIdx );
    void UnsubscribeFromPendingConnection( COMCORE::CTCPServerConnection* connection );

    /**
     *  Try to scan for a complete Logon in a pending connection's buffer.
     *  Returns true and populates fields if a Logon was found.
     */
    bool TryScanLogon( const CORE::CDynamicBuffer& buffer     ,
                       CFIXClientSessionFields& outFields     ,
                       CORE::UInt32& outLogonLen              ) const;

    /**
     *  Once a Logon has been identified on a pending connection, route it to a topic.
     */
    void PromotePendingToTopic( CORE::UInt32 connIdx                   ,
                                COMCORE::CTCPServerConnection* conn     ,
                                const CFIXClientSessionFields& fields   ,
                                CORE::UInt32 logonLen                   );

    CFIXServerPubSubClient( void ); // not implemented

    private:

    CFIXServerPubSubClientConfig    m_fixConfig;
    COMCORE::CTCPServerSocket       m_tcpServerSocket;
    TTopicMap                       m_topicMap;
    TPendingMap                     m_pendingMap;
    MT::CMutex                      m_lock;
    bool                            m_initialized;
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

#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H ? */
