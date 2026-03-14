/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefPUBSUB_CPubSubClientConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_PUBSUB_CLIENT_MAX_IN_FLIGHT                     1000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubClientConfig::CPubSubClientConfig( void )
    : CORE::CIConfigurable()
    , pubsubClientType()
    , desiredFeatures()
    , customConfig()
    , reconnectDelayInMs( 0 )
    , maxTotalMsgsInFlight( GUCEF_DEFAULT_PUBSUB_CLIENT_MAX_IN_FLIGHT )
    , remoteAddresses()
    , topics()
    , defaultTopicConfig()
    , metricsPrefix()
    , pubsubIdPrefix()
    , journalConfig()
    , pulseGenerator()
    , topicPulseGenerator()
    , bookmarkPersistence()
    , journal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientConfig::CPubSubClientConfig( const CPubSubClientConfig& src )
    : CORE::CIConfigurable()
    , pubsubClientType( src.pubsubClientType )
    , desiredFeatures( src.desiredFeatures )
    , customConfig( src.customConfig )
    , reconnectDelayInMs( src.reconnectDelayInMs )
    , maxTotalMsgsInFlight( src.maxTotalMsgsInFlight )
    , remoteAddresses( src.remoteAddresses )
    , topics( src.topics )
    , defaultTopicConfig( src.defaultTopicConfig )
    , metricsPrefix( src.metricsPrefix )
    , pubsubIdPrefix( src.pubsubIdPrefix )
    , journalConfig( src.journalConfig )
    , pulseGenerator( src.pulseGenerator )
    , topicPulseGenerator( src.topicPulseGenerator )
    , bookmarkPersistence( src.bookmarkPersistence )
    , journal( src.journal )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientConfig::~CPubSubClientConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientConfig&
CPubSubClientConfig::operator=( const CPubSubClientConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        // copy the config elements
        pubsubClientType = src.pubsubClientType;
        desiredFeatures = src.desiredFeatures;
        customConfig = src.customConfig;
        reconnectDelayInMs = src.reconnectDelayInMs;
        maxTotalMsgsInFlight = src.maxTotalMsgsInFlight;
        remoteAddresses = src.remoteAddresses;
        topics = src.topics;
        defaultTopicConfig = src.defaultTopicConfig;
        metricsPrefix = src.metricsPrefix;
        pubsubIdPrefix = src.pubsubIdPrefix;
        journalConfig = src.journalConfig;

        // copy the runtime aspects
        pulseGenerator = src.pulseGenerator;
        topicPulseGenerator = src.topicPulseGenerator;
        bookmarkPersistence = src.bookmarkPersistence;
        journal = src.journal;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "pubsubClientType", pubsubClientType ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "reconnectDelayInMs", reconnectDelayInMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxTotalMsgsInFlight", maxTotalMsgsInFlight ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "metricsPrefix", metricsPrefix ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "pubsubIdPrefix", pubsubIdPrefix ) && totalSuccess;

    CORE::CDataNode* addressList = cfg.AddChild( "RemoteAddresses", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL != addressList )
    {
        THostAddressVector::const_iterator i = remoteAddresses.begin();
        while ( i != remoteAddresses.end() )
        {
            cfg.AddChildWithValue( "remoteAddress", (*i).HostnameAndPortAsString() );
            ++i;
        }
    }
    else
        return false;

    cfg.CopySubTree( customConfig );

    CORE::CDataNode* desiredFeaturesCfg = cfg.FindOrAddChild( "DesiredFeatures" );
    if ( GUCEF_NULL == desiredFeaturesCfg )
       return false;
    totalSuccess = desiredFeatures.SaveConfig( *desiredFeaturesCfg ) && totalSuccess;

    if ( !topics.empty() )
    {
        CORE::CDataNode* topicsCollection = cfg.AddChild( "Topics", GUCEF_DATATYPE_ARRAY );
        if ( GUCEF_NULL != topicsCollection )
        {
            TPubSubClientTopicConfigPtrVector::const_iterator i = topics.begin();
            while ( i != topics.end() )
            {
                CORE::CDataNode* topicCfgObj = topicsCollection->AddChild( "PubSubClientTopicConfig", GUCEF_DATATYPE_OBJECT );
                if ( GUCEF_NULL != topicCfgObj )
                {
                    totalSuccess = (*i)->SaveConfig( *topicCfgObj ) && totalSuccess;
                }
                ++i;
            }
        }
    }

    if ( !defaultTopicConfig.IsNULL() )
    {
        CORE::CDataNode* defaultTopicCfgNode = cfg.FindOrAddChild( "defaultTopicConfig" );
        if ( GUCEF_NULL == defaultTopicCfgNode )
            return false;
        totalSuccess = defaultTopicConfig->SaveConfig( *defaultTopicCfgNode ) && totalSuccess;
    }

    CORE::CDataNode* journalCfg = cfg.FindOrAddChild( "journalConfig" );
    if ( GUCEF_NULL == journalCfg )
       return false;
    totalSuccess = journalConfig.SaveConfig( *journalCfg ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    pubsubClientType = cfg.GetAttributeValueOrChildValueByName( "pubsubClientType", pubsubClientType ).AsString( pubsubClientType, true );
    reconnectDelayInMs = cfg.GetAttributeValueOrChildValueByName( "reconnectDelayInMs" ).AsUInt32( reconnectDelayInMs, true );
    maxTotalMsgsInFlight = cfg.GetAttributeValueOrChildValueByName( "maxTotalMsgsInFlight" ).AsInt64( maxTotalMsgsInFlight, true );
    metricsPrefix = cfg.GetAttributeValueOrChildValueByName( "metricsPrefix" ).AsString( metricsPrefix, true );
    pubsubIdPrefix = cfg.GetAttributeValueOrChildValueByName( "pubsubIdPrefix" ).AsString( pubsubIdPrefix, true );

    CORE::CDataNode* remoteAddressessCfg = cfg.FindChild( "RemoteAddresses" );
    if ( GUCEF_NULL != remoteAddressessCfg )
    {
        CORE::CDataNode::const_iterator i = remoteAddressessCfg->ConstBegin();
        while ( i != remoteAddressessCfg->ConstEnd() )
        {
            remoteAddresses.push_back( COMCORE::CHostAddress( (*i)->GetValue().AsString( CString::Empty, true ) ) );
            ++i;
        }
    }

    const CORE::CDataNode* newCustomConfig = cfg.FindChild( "CustomConfig" );
    if ( GUCEF_NULL != newCustomConfig )
    {
        customConfig.Clear();
        customConfig.Copy( *newCustomConfig );
    }

    const CORE::CDataNode* newDesiredFeatures = cfg.FindChild( "DesiredFeatures" );
    if ( GUCEF_NULL != newDesiredFeatures )
    {
        if ( !desiredFeatures.LoadConfig( *newDesiredFeatures ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientConfig:LoadConfig: failed to load DesiredFeatures section" );
            return false;
        }
    }

    const CORE::CDataNode* topicCollectionNode = cfg.FindChild( "Topics" );
    if ( GUCEF_NULL != topicCollectionNode )
    {
        topics.clear();

        CORE::CDataNode::TConstDataNodeSet psClientTopicConfigs = topicCollectionNode->FindChildrenOfType( "PubSubClientTopicConfig", true );
        if ( !psClientTopicConfigs.empty() )
        {
            CORE::UInt32 n=0;
            topics.resize( psClientTopicConfigs.size() );

            CORE::CDataNode::TConstDataNodeSet::iterator i = psClientTopicConfigs.begin();
            while ( i != psClientTopicConfigs.end() )
            {
                CPubSubClientTopicConfigPtr& topicConfig = topics[ n ];
                if ( topicConfig.IsNULL() )
                {
                    topicConfig = CPubSubClientTopicConfigPtr( GUCEF_NEW CPubSubClientTopicConfig() );
                }
                if ( topicConfig.IsNULL() || !topicConfig->LoadConfig( *(*i) ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientConfig:LoadConfig: failed to load PubSubClientTopicConfig section" );
                    return false;
                }
                ++i; ++n;
            }
        }
    }

    const CORE::CDataNode* defaultTopicConfigNode = cfg.FindChild( "defaultTopicConfig" );
    if ( GUCEF_NULL != defaultTopicConfigNode )
    {
        if ( defaultTopicConfig.IsNULL() )
        {
            defaultTopicConfig = CPubSubClientTopicConfigPtr( GUCEF_NEW CPubSubClientTopicConfig() );
        }
        if ( !defaultTopicConfig->LoadConfig( *defaultTopicConfigNode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientConfig:LoadConfig: failed to load defaultTopicConfig section" );
            return false;
        }
    }

    const CORE::CDataNode* journalConfigNode = cfg.FindChild( "journalConfig" );
    if ( GUCEF_NULL != journalConfigNode )
    {
        if ( !journalConfig.LoadConfig( *journalConfigNode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientConfig:LoadConfig: failed to load journal config section" );
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CPubSubClientConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubClientConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
