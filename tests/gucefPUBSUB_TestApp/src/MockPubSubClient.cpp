/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
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

#include "MockPubSubClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMockPubSubClientTopic::ClassTypeName = "CMockPubSubClientTopic";
const CORE::CString CMockPubSubClient::ClassTypeName      = "CMockPubSubClient";
const CORE::CString CMockPubSubClient::MockClientTypeId   = "MOCK";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CMockPubSubClientTopic                             //
//                                                                         //
//-------------------------------------------------------------------------*/

CMockPubSubClientTopic::CMockPubSubClientTopic( CPubSubClient* client, const CORE::CString& topicName )
    : CPubSubClientTopic()
    , CORE::CTSharedObjCreator< CMockPubSubClientTopic, MT::CMutex >( this )
    , m_isConnected( false )
    , m_isSubscribed( false )
    , m_isHealthy( true )
    , m_supportsPublishing( true )
    , m_supportsSubscribing( true )
    , m_topicName( topicName )
    , m_client( client )
    , m_nextPublishActionId( 1 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CMockPubSubClientTopic::~CMockPubSubClientTopic()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CPubSubClient*
CMockPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;
    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::InitializeConnectivity( bool /*reset*/ )
{GUCEF_TRACE;
    m_isConnected = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;
    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::SubscribeStartingAtBookmark( const CPubSubBookmark& /*bookmark*/ )
{GUCEF_TRACE;
    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CPubSubBookmark
CMockPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;
    return CPubSubBookmark( CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;
    m_isConnected  = false;
    m_isSubscribed = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;
    return m_isConnected;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;
    return m_isSubscribed;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;
    return m_isHealthy;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;
    return m_supportsPublishing;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;
    return m_supportsSubscribing;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMockPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;
    return m_topicName;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const CIPubSubMsg& /*msg*/, bool /*notify*/ )
{GUCEF_TRACE;
    publishActionId = m_nextPublishActionId++;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::SaveConfig( CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;
    config.topicName = m_topicName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClientTopic::LoadConfig( const CPubSubClientTopicConfig& config )
{GUCEF_TRACE;
    m_topicName = config.topicName;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMockPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;
    return CMockPubSubClientTopic::ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CMockPubSubClient                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

CMockPubSubClient::CMockPubSubClient( const CPubSubClientConfig& /*config*/ )
    : CPubSubClient()
    , m_isConnected( false )
    , m_isHealthy( true )
    , m_isInitialized( true )
    , m_features()
    , m_topics()
{GUCEF_TRACE;

    m_features.supportsPublishing          = true;
    m_features.supportsSubscribing         = true;
    m_features.supportsPrimaryPayloadPerMsg = true;
    m_features.supportsKeyValueSetPerMsg   = true;
    m_features.supportsPerMsgIds           = true;
    m_features.supportsMsgIndex            = true;
}

/*-------------------------------------------------------------------------*/

CMockPubSubClient::~CMockPubSubClient()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMockPubSubClient::GetType( void ) const
{GUCEF_TRACE;
    return MockClientTypeId;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::GetSupportedFeatures( CPubSubClientFeatures& features ) const
{GUCEF_TRACE;
    features = m_features;
    return true;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CMockPubSubClient::CreateTopicAccess( CPubSubClientTopicConfigPtr topicConfig        ,
                                      CORE::PulseGeneratorPtr     /*pulseGenerator*/ )
{GUCEF_TRACE;

    if ( topicConfig.IsNULL() )
        return CPubSubClientTopicBasicPtr();

    const CORE::CString& topicName = topicConfig->topicName;
    CPubSubClientTopicBasicPtr topicPtr = ( GUCEF_NEW CMockPubSubClientTopic( this, topicName ) )->CreateSharedPtr();
    m_topics[ topicName ] = topicPtr;
    return topicPtr;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CMockPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    TopicMap::iterator it = m_topics.find( topicName );
    if ( it != m_topics.end() )
        return it->second;
    return CPubSubClientTopicBasicPtr();
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicConfigPtr
CMockPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;
    CPubSubClientTopicConfigPtr cfg( GUCEF_NEW CPubSubClientTopicConfig() );
    return cfg;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicConfigPtr
CMockPubSubClient::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;
    CPubSubClientTopicConfigPtr cfg( GUCEF_NEW CPubSubClientTopicConfig() );
    cfg->topicName = topicName;
    return cfg;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicConfigPtr
CMockPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;
    return GetTopicConfig( topicName );
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::SaveConfig( CORE::CDataNode& /*cfg*/ ) const
{GUCEF_TRACE;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::SaveConfig( CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;
    cfg.pubsubClientType = MockClientTypeId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::LoadConfig( const CORE::CDataNode& /*cfg*/ )
{GUCEF_TRACE;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::LoadConfig( const CPubSubClientConfig& /*cfg*/ )
{GUCEF_TRACE;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMockPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;
    topicNameList.clear();
}

/*-------------------------------------------------------------------------*/

void
CMockPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;
    topicNameList.clear();
    TopicMap::iterator it = m_topics.begin();
    while ( it != m_topics.end() )
    {
        topicNameList.insert( it->first );
        ++it;
    }
}

/*-------------------------------------------------------------------------*/

void
CMockPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;
    topicAccess.clear();
    TopicMap::iterator it = m_topics.begin();
    while ( it != m_topics.end() )
    {
        topicAccess.insert( it->second );
        ++it;
    }
}

/*-------------------------------------------------------------------------*/

void
CMockPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;
    m_topics.erase( topicName );
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::Connect( bool /*reset*/ )
{GUCEF_TRACE;
    m_isConnected = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::Disconnect( void )
{GUCEF_TRACE;
    m_isConnected = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;
    return m_isConnected;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;
    return m_isHealthy;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;
    return m_isInitialized;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMockPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;
    return CMockPubSubClient::ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::RegisterInFactory( void )
{GUCEF_TRACE;

    typedef CORE::CTFactoryWithParam< CPubSubClient, CMockPubSubClient, CPubSubClientConfig, MT::CMutex > TMockClientFactory;

    PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
    if ( !factory.IsConstructible( MockClientTypeId ) )
    {
        factory.RegisterConcreteFactory( MockClientTypeId, GUCEF_NEW TMockClientFactory() );
    }
    return factory.IsConstructible( MockClientTypeId );
}

/*-------------------------------------------------------------------------*/

bool
CMockPubSubClient::UnregisterFromFactory( void )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
    factory.UnregisterConcreteFactory( MockClientTypeId );
    return !factory.IsConstructible( MockClientTypeId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
