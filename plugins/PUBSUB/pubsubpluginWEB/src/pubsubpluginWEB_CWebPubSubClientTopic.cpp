/*
 *  pubsubpluginWEB: Generic GUCEF COMCORE plugin for providing pubsub approximation via the WEB
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

#include <string>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H */

#ifndef PUBSUBPLUGIN_UDP_CWEBPUBSUBCLIENT_H
#include "pubsubpluginWEB_CWebPubSubClient.h"
#define PUBSUBPLUGIN_UDP_CWEBPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_UDP_CWEBPUBSUBCLIENT_H ? */

#include "pubsubpluginWEB_CWebPubSubClientTopic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString CWebPubSubClientTopic::UnknownClientType = "unknown";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

RestApiPublishedMessagesResource::RestApiPublishedMessagesResource( TBasicWebPubSubClientTopicPtr& topic )
    : GUCEF::WEB::CCodecBasedHTTPServerResource()
    , m_topic( topic )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPublishedMessagesResource::~RestApiPublishedMessagesResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPublishedMessagesResource::Serialize( const CORE::CString& resourcePath   ,
                                             CORE::CDataNode& output             ,
                                             const CORE::CString& representation ,
                                             const CORE::CString& params         )
{GUCEF_TRACE;

    //if ( m_appConfig )
    //{
    //    const CORE::CDataNode* appConfigData = m_app->GetAppConfig();
    //    if ( GUCEF_NULL != appConfigData )
    //    {
    //        output.Copy( *appConfigData );
    //        return true;
    //    }
    //    return false;
    //}
    //else
    //{
    //    const CORE::CDataNode& globalConfigData = m_app->GetGlobalConfig();
    //    output.Copy( globalConfigData );
    //    return true;
    //}

    return false;
}

/*-------------------------------------------------------------------------*/

RestApiPublishedMessagesResource::TDeserializeState
RestApiPublishedMessagesResource::Deserialize( const CORE::CString& resourcePath   ,
                                                 const CORE::CDataNode& input        ,
                                                 const CORE::CString& representation ,
                                                 bool isDeltaUpdateOnly              )
{GUCEF_TRACE;


            return CIHTTPServerResource::DESERIALIZESTATE_NOTSUPPORTED;

}

/*-------------------------------------------------------------------------*/

CWebPubSubClientTopic::CWebPubSubClientTopic( CWebPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CWebPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_config()
    , m_reconnectTimer( GUCEF_NULL )
    , m_lock()
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_metrics()
    , m_metricFriendlyTopicName()
    , m_httpServer( client->GetPulseGenerator() )
    , m_httpRouter()
    , m_publishedMsgPrunerTimer( GUCEF_NULL )
    , m_publishedMsgs()
    , m_publishedClientTypes()
    , m_publishedTopicNamesPerClientType()
    , m_pubsubSerializerOptions()
{GUCEF_TRACE;
        
    // For a web context we prefer an ISO datetime string over a numerical epoch based offset
    m_pubsubSerializerOptions.msgDateTimeAsMsSinceUnixEpochInUtc = false;
    
    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    if ( m_client->GetConfig().desiredFeatures.supportsAutoReconnect )
    {
        m_reconnectTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), m_client->GetConfig().reconnectDelayInMs );
    }
    if ( 0 != m_config.maxPublishedMsgCountToRetainForRest )
    {
        m_publishedMsgPrunerTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), 1000 );
    }

    RegisterRestApiEndpoints(); 
    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CWebPubSubClientTopic::~CWebPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_client = GUCEF_NULL;

    Disconnect();
    m_httpRouter.RemoveAllResourceMappings();

    GUCEF_DELETE m_reconnectTimer;
    m_reconnectTimer = GUCEF_NULL;

    GUCEF_DELETE m_publishedMsgPrunerTimer;
    m_publishedMsgPrunerTimer = GUCEF_NULL;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::RegisterRestApiEndpoints( void )
{GUCEF_TRACE;
    
    TClientIndexMap* clientIndexMap = GUCEF_NULL;
    TDummyHttpServerResource* healthDummy = GUCEF_NULL;

    try
    {
        if ( m_config.supportHttpServerBasedRestEndpoints )
        {
            m_httpRouter.SetWildcardMatchUris( false );
            
            clientIndexMap = GUCEF_NEW TClientIndexMap( "PubSubClients", "PubSubClient", "clientType", &m_publishedClientTypes, this );
            GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr clientIndexMapPtr( clientIndexMap->CreateSharedPtr() );
            m_httpRouter.SetResourceMapping( "/clients", clientIndexMapPtr );          

            if ( m_config.exposeBasicHealthEndpoint )
            {
                healthDummy = GUCEF_NEW TDummyHttpServerResource();
                GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr healthDummyPtr( healthDummy->CreateSharedPtr() );
                m_httpRouter.SetResourceMapping( m_config.basicHealthEndpointPath, healthDummyPtr );
            }

            m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );
            return m_httpServer.ListenOnPort( m_config.httpServerPort );
        }
        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CWebPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_reconnectTimer )
    {
        TEventCallback callback( this, &CWebPubSubClientTopic::OnReconnectTimerCycle );
        SubscribeTo( m_reconnectTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    if ( GUCEF_NULL != m_publishedMsgPrunerTimer )
    {
        TEventCallback callback( this, &CWebPubSubClientTopic::OnPublishedMsgPrunerTimerCycle );
        SubscribeTo( m_publishedMsgPrunerTimer      ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    TEventCallback callback( this, &CWebPubSubClientTopic::OnPulseCycle );
    SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent                       ,
                 callback                                                );
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;
    
    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needSubscribeSupport;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CWebPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::PublishToRestApi( CORE::UInt64& publishActionId                               , 
                                         const PUBSUB::CIPubSubMsg& msg                              , 
                                         bool notify                                                 ,
                                         const PUBSUB::CPubSubClient* originClient                   ,
                                         const CORE::CString& originClientType                       ,
                                         const PUBSUB::CPubSubClientTopicBasicPtr& originClientTopic )
{GUCEF_TRACE;
    
    // For the REST API we retain the messages for some period/count to allow for retrieval/viewing
    // If the REST API is not to be used then maxPublishedMsgCountToRetainForRest should be set to 0 for better performance
    if ( 0 != m_config.maxPublishedMsgCountToRetainForRest )
    {
        CORE::CAsciiString urlEncodedOriginClientType;
        CORE::CUrlEncoder::EncodeToAscii( originClientType, urlEncodedOriginClientType );

        CORE::CAsciiString urlEncodedOriginClientTopicName;
        CORE::CUrlEncoder::EncodeToAscii( originClientTopic->GetTopicName(), urlEncodedOriginClientTopicName );

        PUBSUB::CIPubSubMsg::TNoLockSharedPtr msgClone( static_cast< PUBSUB::CIPubSubMsg* >( msg.Clone() ) );
        
        // Access client storage
        TPubSubClientTopicToUInt64ToIPubSubMsgSPtrVectorMap* clientTopics = GUCEF_NULL;
        TPubSubClientToPubSubClientTopicMsgsMap::iterator i = m_publishedMsgs.find( originClient );
        if ( i != m_publishedMsgs.end() )
        {
            clientTopics = &(*i).second;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebPubSubClientTopic(" + CORE::ToString( this ) + "):PublishToRestApi: discovered new client origin. Ptr=" + 
                CORE::ToString( originClient ) + ". typeName=\"" + originClientType + "\" urlencodedTypeName=" + urlEncodedOriginClientType );
            
            // completely new msg origin, add storage plus reference links for it
            m_publishedClientTypes[ urlEncodedOriginClientType ].push_back( originClient );            
            clientTopics = &m_publishedMsgs[ originClient ];
        }
        
        // Access client->topic storage
        TUInt64dToTIPubSubMsgSPtrMap* topicMsgMap = GUCEF_NULL;
        TPubSubClientTopicToUInt64ToIPubSubMsgSPtrVectorMap::iterator n = clientTopics->find( originClientTopic.GetPointerAlways() );
        if ( n != clientTopics->end() )
        {
            topicMsgMap = &(*n).second;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebPubSubClientTopic(" + CORE::ToString( this ) + "):PublishToRestApi: discovered new client origin topic. Ptr=" + 
                CORE::ToString( originClientTopic ) + ". topicName=\"" + originClientTopic->GetTopicName() + "\" urlencodedTopicName=" + urlEncodedOriginClientTopicName );
            
            // completely new msg origin, add storage plus reference links for it        
            topicMsgMap = &(*clientTopics)[ originClientTopic.GetPointerAlways() ];
            TStringToPubSubClientTopicPtrVectorMap& topicNameMap = m_publishedTopicNamesPerClientType[ urlEncodedOriginClientType ];
            TPubSubClientTopicPtrVector& topics = topicNameMap[ urlEncodedOriginClientTopicName ];
            topics.push_back( originClientTopic );

            TClientTopicIndexMap* topicIndexMap = GUCEF_NEW TClientTopicIndexMap( "PubSubClientTopics", "PubSubClientTopic", "topicName", &topicNameMap, this );
            GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr topicIndexMapPtr( topicIndexMap->CreateSharedPtr() );
            m_httpRouter.SetResourceMapping( "/clients/" + urlEncodedOriginClientType + "/topics", topicIndexMapPtr ); 

            // This one is a bit problematic: If we have multiple clients we can have duplicate topics name wise which in turn would actually be distinct 'topicMsgMap' collections
            // right now only the first instance would be accessable due to the overlap
            //TClientTopicMsgsIndexMap* topicMsgsIndexMap = GUCEF_NEW TClientTopicMsgsIndexMap( "PubSubMsgs", "PubSubMsg", "publishActionId", topicMsgMap, this );
            //GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr topicMsgsIndexMapPtr( topicMsgsIndexMap->CreateSharedPtr() );
            //m_httpRouter.SetResourceMapping( "/clients/" + urlEncodedOriginClientType + "/topics/" + urlEncodedOriginClientTopicName + "/messages", topicMsgsIndexMapPtr );

            TClientTopicMsgsMap* topicMsgsMap = GUCEF_NEW TClientTopicMsgsMap( "PubSubMsgs", "publishActionId", &m_pubsubSerializerOptions, topicMsgMap, this, false );
            GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr topicMsgsIndexMapPtr( topicMsgsMap->CreateSharedPtr() );
            m_httpRouter.SetResourceMapping( "/clients/" + urlEncodedOriginClientType + "/topics/" + urlEncodedOriginClientTopicName + "/messages", topicMsgsIndexMapPtr );
            
        }

        // Store message
        // If this is a retry it wipes the previous message with the same ID
        (*topicMsgMap)[ publishActionId ] = msgClone;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebPubSubClientTopic(" + CORE::ToString( this ) + "):PublishToRestApi: For origin \"" + 
            originClientType + ':' + originClientTopic->GetTopicName() + "\": Retaining message with publishActionId=" + CORE::ToString( publishActionId ) +
            " and origin receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );
    } 

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
{GUCEF_TRACE;
    
    bool success = true;
    
    const PUBSUB::CPubSubClient* originClient = GUCEF_NULL;
    PUBSUB::CPubSubClientTopicBasicPtr originClientTopic = msg.GetOriginClientTopic();
    if ( !originClientTopic.IsNULL() )
    {       
        originClient = originClientTopic->GetClient();
    }
    const CORE::CString& originClientType = GUCEF_NULL != originClient ? originClient->GetType() : UnknownClientType;            
        
    MT::CScopeMutex lock( m_lock );

    if ( 0 == publishActionId )
    {
        publishActionId = m_currentPublishActionId;
        ++m_currentPublishActionId;
    }

    success = PublishToRestApi( publishActionId   ,
                                msg               ,
                                notify            ,
                                originClient      ,
                                originClientType  ,
                                originClientTopic ) && success;

    if ( notify )
    {
        if ( success )
            m_publishSuccessActionIds.push_back( publishActionId );
        else
            m_publishFailureActionIds.push_back( publishActionId );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( LoadConfig( config ) )
    {

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPubSubClientTopic::GenerateMetricsFriendlyTopicName( const CORE::CString& topicName )
{GUCEF_TRACE;

    // Let's avoid non-ASCII stumbling blocks and force the down to ASCII
    CORE::CAsciiString asciiMetricsFriendlyTopicName = topicName.ForceToAscii( '_' );
    
    // Replace special chars
    static const char disallowedChars[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '=', '+', ',', '.', '<', '>', '/', '?', '`', '~', '\\', '|', '{', '}', '[', ']', ';', ':', '\'', '\"' };
    asciiMetricsFriendlyTopicName = asciiMetricsFriendlyTopicName.ReplaceChars( disallowedChars, sizeof(disallowedChars)/sizeof(char), '_' );

    // Back to the platform wide string convention format
    CORE::CString metricsFriendlyTopicName = CORE::ToString( asciiMetricsFriendlyTopicName );
    return metricsFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    m_config = config;

    m_metricFriendlyTopicName = GenerateMetricsFriendlyTopicName( m_config.topicName );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_httpServer.Close();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_httpServer.ListenOnPort( m_config.httpServerPort );
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) 
{GUCEF_TRACE;

    // Not supported. We are dependent on submissions we dont control
    return false;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark 
CWebPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    // Not supported
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE );    
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    PUBSUB::CPubSubBookmark bookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, msg.GetMsgIndex() );
    return AcknowledgeReceiptImpl( bookmark, msg.GetReceiveActionId() );
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;
    
    return AcknowledgeReceiptImpl( bookmark, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::AcknowledgeReceiptImpl( const PUBSUB::CPubSubBookmark& bookmark , 
                                               CORE::UInt64 receiveActionId             )
{GUCEF_TRACE;

 
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_httpServer.IsActive();           
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_httpServer.IsActive();     
}

/*-------------------------------------------------------------------------*/

bool 
CWebPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    // @TODO
    MT::CScopeMutex lock( m_lock );
    return true;           
}

/*-------------------------------------------------------------------------*/

bool
CWebPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_httpServer.ListenOnPort( m_config.httpServerPort );
}

/*-------------------------------------------------------------------------*/

const CWebPubSubClientTopicConfig& 
CWebPubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool totalSuccess = true;
    if ( m_config.needPublishSupport )
        totalSuccess = InitializeConnectivity( true ) && totalSuccess;
    if ( m_config.needSubscribeSupport )
        totalSuccess = Subscribe() && totalSuccess;

    m_reconnectTimer->SetEnabled( !totalSuccess );
}

/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::OnPublishedMsgPrunerTimerCycle( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    //// Check if we added max message count messages, or if such a limit should be applied at all
    //if ( 0 < m_config.maxPublishedMsgCountToRetainForRest && m_config.maxPublishedMsgCountToRetainForRest < (CORE::Int32) topicMsgMap.size() ) 
    //{
    //    // Lets prune
    //                    
    //}
}
/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;
                      
    if ( !m_publishSuccessActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData ) ) return;
        m_publishSuccessActionIds.clear();
    }
    if ( !m_publishFailureActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData ) ) return;
        m_publishFailureActionIds.clear();
    }
}

/*-------------------------------------------------------------------------*/

CWebPubSubClientTopic::TopicMetrics::TopicMetrics( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CWebPubSubClientTopic::TopicMetrics& 
CWebPubSubClientTopic::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CWebPubSubClientTopic::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    return m_metricFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

void
CWebPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CWebPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CWebPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CWebPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CWebPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::WEB::CWebPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
