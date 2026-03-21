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

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginFIX_CFIXClientPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPICCONFIG_H ? */

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

class CFIXClientPubSubClient;

/**
 *  Represents the single pubsub topic exposed per FIX TCP session.
 *  Received application-level FIX messages are delivered here as pubsub messages.
 *  Outgoing publishes are serialized as raw FIX and sent via the client socket.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXClientPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                        public CORE::CTSharedObjCreator< CFIXClientPubSubClientTopic, MT::CMutex >
{
    public:

    CFIXClientPubSubClientTopic( CFIXClientPubSubClient* client );

    virtual ~CFIXClientPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

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

    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Called by CFIXClientPubSubClient when a new FIX message arrives (application or session level).
     *  Delivers raw FIX wire bytes as ASCII_STRING primary payload (linked, zero-copy).
     *  Delivers essential session fields as numeric UInt32 tag-keyed metadata (linked, zero-copy).
     *  Fires MsgsRecievedEvent synchronously — linked views are valid until this call returns.
     *
     *  @param msgStart  Pointer into receive buffer (valid until ProcessReceiveBuffer compacts)
     *  @param msgLen    Total byte length of the complete FIX message including tag 10
     *  @param fields    Pre-scanned session fields (raw pointers into receive buffer)
     */
    void OnApplicationMessage( const char* msgStart                  ,
                               CORE::UInt32 msgLen                   ,
                               const CFIXClientSessionFields& fields );

    const CFIXClientPubSubClientTopicConfig& GetTopicConfig( void ) const;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CFIXClientPubSubClientTopic > TEventCallback;
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;

    bool IsMsgTypePassedByFilter( const char* msgType, CORE::UInt32 msgTypeLen ) const;

    private:

    CFIXClientPubSubClient*            m_client;
    CFIXClientPubSubClientTopicConfig  m_config;
    bool                               m_isSubscribed;
    CORE::UInt64                       m_lastReceivedSeqNum;
    CORE::UInt64                       m_currentPublishActionId;
    CORE::UInt64                       m_currentReceiveActionId;
    MT::CMutex                         m_lock;
    TPubSubMsgsVector                  m_pubsubMsgs;
    TMsgsRecievedEventData             m_pubsubMsgsRefs;
    TPublishActionIdVector             m_publishSuccessActionIds;
    TMsgsPublishedEventData            m_publishSuccessActionEventData;
    TPublishActionIdVector             m_publishFailureActionIds;
    TMsgsPublishFailureEventData       m_publishFailureActionEventData;
};

/*--------------------------------------------------------------------------*/

typedef CFIXClientPubSubClientTopic::TSharedPtrType   CFIXClientPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTTOPIC_H ? */
