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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#include "gucefPUBSUB_CPubSubFlowRouter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubFlowRouter::CPubSubFlowRouter( void )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, false )
    , m_config()
    , m_normalizedConfig()
    , m_routeMap()
    , m_usedInRouteMap()
    , m_spilloverInfoMap()
    , m_spilloverInfoForTargetsMap()
    , m_busyDeterminingActiveRoutes( false )
    , m_lock( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::~CPubSubFlowRouter()
{GUCEF_TRACE;

    ClearRoutes();
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CRouteTopicLinks::CRouteTopicLinks( void )
    : fromTopic( GUCEF_NULL )
    , activeTopic( GUCEF_NULL )
    , activeTopicType( RouteType::Disabled )
    , toTopic( GUCEF_NULL )
    , failoverTopic( GUCEF_NULL )
    , spilloverTopic( GUCEF_NULL )
    , deadletterTopic( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CRouteTopicLinks::CRouteTopicLinks( const CRouteTopicLinks& src )
    : fromTopic( src.fromTopic )
    , activeTopic( src.activeTopic )
    , activeTopicType( src.activeTopicType )
    , toTopic( src.toTopic )
    , failoverTopic( src.failoverTopic )
    , spilloverTopic( src.spilloverTopic )
    , deadletterTopic( src.deadletterTopic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CRouteInfo::CRouteInfo( void )
    : fromSide( GUCEF_NULL )
    , activeSide( GUCEF_NULL )
    , activeSideType( RouteType::Disabled )
    , toSide( GUCEF_NULL )
    , toSideIsHealthy( true )
    , toSideLastHealthStatusChange( CORE::CDateTime::PastMax )
    , failoverSide( GUCEF_NULL )
    , failoverSideIsHealthy( true )
    , failoverSideLastHealthStatusChange( CORE::CDateTime::PastMax )
    , spilloverBufferSide( GUCEF_NULL )
    , spilloverBufferSideIsHealthy( true )
    , spilloverBufferSideLastHealthStatusChange( CORE::CDateTime::PastMax )
    , flowingIntoSpillover( true )
    , isSpilloverEgressRoute( false )
    , deadLetterSide( GUCEF_NULL )
    , deadLetterSideIsHealthy( true )
    , deadLetterSideLastHealthStatusChange( CORE::CDateTime::PastMax )
    , routeSwitchingTimer( CORE::PulseGeneratorPtr(), 1000 )
    , spilloverInfo( GUCEF_NULL )
    , routeConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CRouteInfo::CRouteInfo( const CRouteInfo& src )
    : fromSide( src.fromSide )
    , activeSide( src.activeSide )
    , activeSideType( src.activeSideType )
    , toSide( src.toSide )
    , toSideIsHealthy( src.toSideIsHealthy )
    , toSideLastHealthStatusChange( src.toSideLastHealthStatusChange )
    , failoverSide( src.failoverSide )
    , failoverSideIsHealthy( src.failoverSideIsHealthy )
    , failoverSideLastHealthStatusChange( src.failoverSideLastHealthStatusChange )
    , spilloverBufferSide( src.spilloverBufferSide )
    , spilloverBufferSideIsHealthy( src.spilloverBufferSideIsHealthy )
    , spilloverBufferSideLastHealthStatusChange( src.spilloverBufferSideLastHealthStatusChange )
    , flowingIntoSpillover( src.flowingIntoSpillover )
    , isSpilloverEgressRoute( src.isSpilloverEgressRoute )
    , deadLetterSide( src.deadLetterSide )
    , deadLetterSideIsHealthy( src.deadLetterSideIsHealthy )
    , deadLetterSideLastHealthStatusChange( src.deadLetterSideLastHealthStatusChange )
    , routeSwitchingTimer( src.routeSwitchingTimer )
    , spilloverInfo( src.spilloverInfo )
    , routeConfig( src.routeConfig )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::IsSpilloverEgressActive( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != spilloverInfo )
        return spilloverInfo->IsEgressActive();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::IsSpilloverEgressOngoing( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != spilloverInfo )
        return spilloverInfo->IsEgressOngoing();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::IsSpilloverIngressOngoing( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != spilloverInfo )
        return spilloverInfo->IsIngressOngoing();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::IsSpilloverInActiveUse( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != spilloverInfo )
        return spilloverInfo->IsInActiveUse();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::DidMsgsFlowIntoSpillover( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != spilloverInfo )
        return spilloverInfo->DidMsgsFlowIntoSpillover();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::CRouteInfo::SwitchTopicLinksActiveTopic( RouteType activeSide, CRouteTopicLinks& topicLinks )
{GUCEF_TRACE;

    switch ( activeSide )
    {
        case RouteType::Primary: { topicLinks.activeTopic = topicLinks.toTopic; break; }
        case RouteType::Failover: { topicLinks.activeTopic = topicLinks.failoverTopic; break; }
        case RouteType::SpilloverBuffer: { topicLinks.activeTopic = topicLinks.spilloverTopic; break; }
        case RouteType::DeadLetter: { topicLinks.activeTopic = topicLinks.deadletterTopic; break; }

        case RouteType::Disabled:
        default:
        {
            topicLinks.activeTopic = GUCEF_NULL;
        }
        case RouteType::Active: { break; }
    }

    topicLinks.activeTopicType = activeSide;
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::CRouteInfo::SwitchAllTopicLinksActiveTopic( RouteType activeSide )
{GUCEF_TRACE;

    TStringToRouteTopicLinksMap::iterator i = fromSideTopicNameLinks.begin();
    while ( i != fromSideTopicNameLinks.end() )
    {
        CRouteTopicLinks& topicLinks = (*i).second;
        SwitchTopicLinksActiveTopic( activeSide, topicLinks );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CRouteTopicLinks*
CPubSubFlowRouter::GetTargetTopicLinks( CPubSubClientTopic* fromTopic ,
                                        CRouteInfo& routeInfo         ,
                                        bool& errorOccured            )
{GUCEF_TRACE;

    errorOccured = false;

    TTopicRawPtrToRouteTopicLinksRawPtrMap::iterator t = routeInfo.fromSideTopicLinks.find( fromTopic );
    if ( t != routeInfo.fromSideTopicLinks.end() )
    {
        return (*t).second;
    }
    else
    {
        // Check if we need to worry about auto matching topics
        if ( fromTopic != GUCEF_NULL &&
             !routeInfo.routeConfig.IsNULL() &&
             routeInfo.routeConfig->IsAnyAutoTopicMatchingNeeded() )
        {
            // We should always have a target topic considering we are auto matching them and not relying on an explicit map
            // Due to topic discovery and auto generation this scenario can happen.
            // We will 'lazy init' the mapping now.

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RouteInfo:GetTargetTopicLinks: topic links not yet found, will lazy init" );

            MT::CScopeWriterLock lock( m_lock );

            CORE::PulseGeneratorPtr nullPulseGenerator;
            if ( routeInfo.MatchTopicRouteConfig( fromTopic, nullPulseGenerator ) )
            {
                // Now lets try again after lazy matching the topic route configs

                t = routeInfo.fromSideTopicLinks.find( fromTopic );
                if ( t != routeInfo.fromSideTopicLinks.end() )
                {
                    return (*t).second;
                }
            }

            errorOccured = true;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RouteInfo:GetTargetTopicLinks: Failed to lazy init topic links for topic " + fromTopic->GetTopicName() );
        }
    }

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::MatchTopicRouteConfig( CPubSubClientTopic* topicAccess            ,
                                                      CORE::PulseGeneratorPtr destPulseGenerator )
{GUCEF_TRACE;

    if ( topicAccess != GUCEF_NULL &&
         topicAccess->GetClient() != GUCEF_NULL )
    {
        CPubSubClientPtr fromSideClient = fromSide->GetCurrentUnderlyingPubSubClient();
        if ( !fromSideClient.IsNULL() && topicAccess->GetClient() == fromSideClient.GetPointerAlways() )
        {
            // Seems we want it, lets see if we can support it and if it applies
            CPubSubClientFeatures supportedFeatures;
            fromSideClient->GetSupportedFeatures( supportedFeatures );

            const CORE::CString& topicName = topicAccess->GetTopicName();
            CPubSubFlowRouteTopicConfigPtr topicRouteConfig = routeConfig->FindOrCreateTopicAssociation( topicName, supportedFeatures.supportsGlobPatternTopicNames );

            return MatchTopicRouteConfig( topicRouteConfig, topicAccess->GetPulseGenerator(), destPulseGenerator );
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RouteInfo:MatchTopicRouteConfig: invalid topic access provided from which to derive route config" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::MatchTopicRouteConfig( const CPubSubFlowRouteTopicConfigPtr topicRouteConfig ,
                                                      CORE::PulseGeneratorPtr fromPulseGenerator            ,
                                                      CORE::PulseGeneratorPtr destPulseGenerator            )
{GUCEF_TRACE;

    if ( topicRouteConfig.IsNULL() )
        return false;
    if ( topicRouteConfig->fromSideTopicName.IsNULLOrEmpty() )
        return true;

    bool totalSuccess = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Applying config for topic association for 'from' topic " +
            topicRouteConfig->fromSideTopicName );

    // refresh destination topic pointers to match config
    // this will also auto create topics as needed to satisfy the routing needs

    if ( GUCEF_NULL != fromSide )
    {
        CPubSubClientPtr fromSideClient = fromSide->GetCurrentUnderlyingPubSubClient();
        if ( !fromSideClient.IsNULL() )
        {
            PUBSUB::CPubSubClientFeatures supportedFeatures;
            fromSideClient->GetSupportedFeatures( supportedFeatures );

            // Check if we need to worry about glob pattern topic names
            if ( routeConfig->IsAnyAutoTopicMatchingNeeded()             &&
                 supportedFeatures.supportsGlobPatternTopicNames         &&
                 topicRouteConfig->fromSideTopicName.HasChar( '*' ) >= 0  )
            {
                // Glob pattern topic names are supported by the backend and we actually have a wildcard topic name
                // As such we need to map N:N not 1:1 for each association

                CPubSubClient::PubSubClientTopicSet topicAccessSet;
                if ( fromSideClient->GetOrCreateMultiTopicAccess( topicRouteConfig->fromSideTopicName ,
                                                                  topicAccessSet                     ,
                                                                  fromPulseGenerator                 ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Obtained access to " + CORE::ToString( topicAccessSet.size() ) +
                        " topics association for pattern matched 'from' topic " + topicRouteConfig->fromSideTopicName +
                        ( fromPulseGenerator.IsNULL() ?  "" : ". Requested using pulse generator for thread " + CORE::ToString( fromPulseGenerator->GetPulseDriverThreadId() ) ) );

                    CPubSubClient::PubSubClientTopicSet::iterator i = topicAccessSet.begin();
                    while ( i != topicAccessSet.end() )
                    {
                        CPubSubClientTopicBasicPtr topicAccess = (*i);
                        if ( !topicAccess.IsNULL() )
                        {
                            const CORE::CString& topicName = topicAccess->GetTopicName();

                            if ( topicName.HasChar( '*' ) < 0 )
                            {
                                if ( routeConfig->preferFromTopicThreadForDestination && fromPulseGenerator.IsNULL()  )
                                {
                                    fromPulseGenerator = topicAccess->GetPulseGenerator();
                                    if ( fromPulseGenerator.IsNULL() )
                                        fromPulseGenerator = fromSideClient->GetDefaultTopicPulseGenerator();

                                    if ( !fromPulseGenerator.IsNULL() )
                                    {
                                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Preference given to use 'from' thread for driving destinations as well, will be using pulse generator for thread " +
                                            CORE::ToString( fromPulseGenerator->GetPulseDriverThreadId() ) );
                                    }
                                    else
                                    {
                                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Preference given to use 'from' thread for driving destinations as well, however no suitable pulse generator is available" );
                                    }
                                }

                                CPubSubFlowRouteTopicConfigPtr autoTopicRouteConfig = CPubSubFlowRouteTopicConfig::CreateSharedObj();
                                autoTopicRouteConfig->fromSideTopicName = topicName;

                                if ( routeConfig->toSideTopicsAutoMatchFromSide )
                                    autoTopicRouteConfig->toSideTopicName = topicName;
                                else
                                    autoTopicRouteConfig->toSideTopicName = topicRouteConfig->toSideTopicName;

                                if ( routeConfig->failoverSideTopicsAutoMatchFromSide )
                                    autoTopicRouteConfig->failoverSideTopicName = topicName;
                                else
                                    autoTopicRouteConfig->failoverSideTopicName = topicRouteConfig->failoverSideTopicName;

                                if ( routeConfig->spilloverSideTopicsAutoMatchFromSide )
                                    autoTopicRouteConfig->spilloverSideTopicName = topicName;
                                else
                                    autoTopicRouteConfig->spilloverSideTopicName = topicRouteConfig->spilloverSideTopicName;

                                if ( routeConfig->deadLetterSideTopicsAutoMatchFromSide )
                                    autoTopicRouteConfig->deadLetterSideTopicName = topicName;
                                else
                                    autoTopicRouteConfig->deadLetterSideTopicName = topicRouteConfig->deadLetterSideTopicName;

                                totalSuccess = MatchTopicRouteConfig( autoTopicRouteConfig ,
                                                                      fromPulseGenerator   ,
                                                                      destPulseGenerator   ) && totalSuccess;
                            }
                            else
                            {
                                // Something went wrong. We should not get back a topic with a topic mame which still
                                // has wildcards in it. That will confuse this code. The backend should resolve to non wildcard containing names
                                totalSuccess = false;
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: glob pattern topic \""  +
                                        topicRouteConfig->fromSideTopicName + "\" for side  \"" + fromSide->GetSideId() + "\" was matched by the backend to topic \"" +
                                        topicName + "\" which still contains a wildcard. This is not supported." );
                            }
                        }
                        ++i;
                    }
                }
                else
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Found no relevant topic access for glob pattern topic \""  +
                            topicRouteConfig->fromSideTopicName + "\" for 'from' side \"" + fromSide->GetSideId() + "\" at this time" );
                }
            }
            else
            {
                // Glob pattern topic names are not supported by the backend or the topic does not contain any wildcards (anymore? since the above in the end calls this)
                // either way, we can do a more straightforward explicit mapping

                CRouteTopicLinks& topicLinks = fromSideTopicNameLinks[ topicRouteConfig->fromSideTopicName ];
                CPubSubClientTopic* oldFromTopic = topicLinks.fromTopic;
                topicLinks.fromTopic = fromSideClient->GetOrCreateTopicAccess( topicRouteConfig->fromSideTopicName, fromPulseGenerator ).GetPointerAlways();
                if ( GUCEF_NULL != topicLinks.fromTopic )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Obtained access to topic for 'from' topic " + topicRouteConfig->fromSideTopicName +
                            ( fromPulseGenerator.IsNULL() ?  "" : ". Requested using pulse generator for thread " + CORE::ToString( fromPulseGenerator->GetPulseDriverThreadId() ) ) );
                }
                else
                {
                    totalSuccess = false;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Failed to obtain topic access to topic \""  +
                            topicRouteConfig->fromSideTopicName + "\" for 'from' side \"" + fromSide->GetSideId() + "\"" );
                }

                if ( fromPulseGenerator.IsNULL() && routeConfig->preferFromTopicThreadForDestination )
                {
                    if ( GUCEF_NULL != topicLinks.fromTopic )
                        fromPulseGenerator = topicLinks.fromTopic->GetPulseGenerator();
                    if ( fromPulseGenerator.IsNULL() )
                        fromPulseGenerator = fromSideClient->GetDefaultTopicPulseGenerator();

                    if ( !fromPulseGenerator.IsNULL() )
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Preference given to use 'from' thread for driving destinations as well, will be using pulse generator for thread " +
                            CORE::ToString( fromPulseGenerator->GetPulseDriverThreadId() ) );
                    }
                    else
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Preference given to use 'from' thread for driving destinations as well, however no suitable pulse generator is available" );
                    }
                }
                if ( destPulseGenerator.IsNULL() && routeConfig->preferFromTopicThreadForDestination )
                {
                    destPulseGenerator = fromPulseGenerator;
                }

                if ( topicLinks.fromTopic != oldFromTopic )
                {
                    if ( GUCEF_NULL != oldFromTopic )
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Erasing map entry for old topic implementation: " + CORE::ToString( oldFromTopic ) );
                        fromSideTopicLinks.erase( oldFromTopic );
                    }
                    if ( GUCEF_NULL != topicLinks.fromTopic )
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Adding map entry for new topic implementation: " + CORE::ToString( topicLinks.fromTopic ) );
                        fromSideTopicLinks[ topicLinks.fromTopic ] = &topicLinks;
                    }
                }

                if ( GUCEF_NULL != toSide && !topicRouteConfig->toSideTopicName.IsNULLOrEmpty() )
                {
                    CPubSubClientPtr toSideClient = toSide->GetCurrentUnderlyingPubSubClient();
                    if ( !toSideClient.IsNULL() )
                    {
                        topicLinks.toTopic = toSideClient->GetOrCreateTopicAccess( topicRouteConfig->toSideTopicName, destPulseGenerator ).GetPointerAlways();
                        if ( GUCEF_NULL == topicLinks.toTopic )
                        {
                            totalSuccess = false;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Failed to obtain topic access to topic \""  +
                                    topicRouteConfig->toSideTopicName + "\" for 'to' side \"" + toSide->GetSideId() + "\"" );
                        }
                    }
                }
                if ( GUCEF_NULL != failoverSide && !topicRouteConfig->failoverSideTopicName.IsNULLOrEmpty() )
                {
                    CPubSubClientPtr failoverSideClient = failoverSide->GetCurrentUnderlyingPubSubClient();
                    if ( !failoverSideClient.IsNULL() )
                    {
                        topicLinks.failoverTopic = failoverSideClient->GetOrCreateTopicAccess( topicRouteConfig->failoverSideTopicName, destPulseGenerator ).GetPointerAlways();
                        if ( GUCEF_NULL == topicLinks.failoverTopic )
                        {
                            totalSuccess = false;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Failed to obtain topic access to topic \""  +
                                    topicRouteConfig->failoverSideTopicName + "\" for 'failover' side \"" + failoverSide->GetSideId() + "\"" );
                        }
                    }
                }
                if ( GUCEF_NULL != spilloverBufferSide && !topicRouteConfig->spilloverSideTopicName.IsNULLOrEmpty() )
                {
                    CPubSubClientPtr spilloverSideClient = spilloverBufferSide->GetCurrentUnderlyingPubSubClient();
                    if ( !spilloverSideClient.IsNULL() )
                    {
                        topicLinks.spilloverTopic = spilloverSideClient->GetOrCreateTopicAccess( topicRouteConfig->spilloverSideTopicName, destPulseGenerator ).GetPointerAlways();
                        if ( GUCEF_NULL == topicLinks.spilloverTopic )
                        {
                            totalSuccess = false;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Failed to obtain topic access to topic \""  +
                                    topicRouteConfig->spilloverSideTopicName + "\" for 'spillover' side \"" + spilloverBufferSide->GetSideId() + "\"" );
                        }
                    }
                }
                if ( GUCEF_NULL != deadLetterSide && !topicRouteConfig->deadLetterSideTopicName.IsNULLOrEmpty() )
                {
                    CPubSubClientPtr deadLetterSideClient = deadLetterSide->GetCurrentUnderlyingPubSubClient();
                    if ( !deadLetterSideClient.IsNULL() )
                    {
                        topicLinks.deadletterTopic = deadLetterSideClient->GetOrCreateTopicAccess( topicRouteConfig->deadLetterSideTopicName, destPulseGenerator ).GetPointerAlways();
                        if ( GUCEF_NULL == topicLinks.deadletterTopic )
                        {
                            totalSuccess = false;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchTopicRouteConfig: Failed to obtain topic access to topic \""  +
                                    topicRouteConfig->deadLetterSideTopicName + "\" for 'dead letter' side \"" + deadLetterSide->GetSideId() + "\"" );
                        }
                    }
                }
            }
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::MatchTopicRouteConfig( const CPubSubFlowRouteTopicConfigPtr topicRouteConfig )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr nullPulseGenerator;
    return MatchTopicRouteConfig( topicRouteConfig, nullPulseGenerator, nullPulseGenerator );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CRouteInfo::MatchAllTopicRouteConfigs( void )
{GUCEF_TRACE;

    if ( routeConfig.IsNULL() )
        return false;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:RouteInfo:MatchAllTopicRouteConfigs: Applying config for " +
            CORE::ToString( routeConfig->topicAssociations.size() ) + " topic associations" );

    bool totalSuccess = true;
    CPubSubFlowRouteConfig::PubSubFlowRouteTopicConfigVector::const_iterator t = routeConfig->topicAssociations.begin();
    while ( t != routeConfig->topicAssociations.end() )
    {
        totalSuccess = MatchTopicRouteConfig( (*t) ) && totalSuccess;
        ++t;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouter::CSpilloverInfo::CSpilloverInfo( void )
    : spilloverIngressMsgCount( 0 )
    , spilloverEgressMsgCount( 0 )
    , endOfDataEventOccured( false )
    , endOfDataEventSupported( false )
    , supportsDiscoveryOfAvailableTopics( false )
    , msgsFlowedIntoSpillover( false )
    , route( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CSpilloverInfo::IsEgressActive( void ) const
{GUCEF_TRACE;

    // Is the spillover active and in egress mode?
    if ( GUCEF_NULL != route && route->activeSide == route->spilloverBufferSide && !route->flowingIntoSpillover )
    {
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CSpilloverInfo::IsEgressOngoing( void ) const
{GUCEF_TRACE;

    // Is the spillover active and in egress mode?
    if ( GUCEF_NULL != route && route->activeSide == route->spilloverBufferSide && !route->flowingIntoSpillover )
    {
        // did any messages flow into the spillover to begin with?
        if ( msgsFlowedIntoSpillover || spilloverIngressMsgCount > 0 )
        {
            // Favor the end-of-data mechanism if supported
            if ( endOfDataEventSupported )
            {
                // Checked to see if we async heard about end-of-data
                if ( !endOfDataEventOccured )
                {
                    // Nothing yet, lets ask directly right now
                    CPubSubClientPtr client = route->spilloverBufferSide->GetCurrentUnderlyingPubSubClient();
                    if ( !client.IsNULL() )
                    {
                        endOfDataEventOccured = client->AreAllSubscriptionsAtEndOfData();
                    }
                }
                return !endOfDataEventOccured;
            }

            // Alternatively rely on the ingress vs egress message count
            return spilloverEgressMsgCount < spilloverIngressMsgCount;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CSpilloverInfo::IsIngressOngoing( void ) const
{GUCEF_TRACE;

    // Is the spillover active and in ingress mode?
    return GUCEF_NULL != route && route->activeSide == route->spilloverBufferSide && route->flowingIntoSpillover;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CSpilloverInfo::DidMsgsFlowIntoSpillover( void ) const
{GUCEF_TRACE;

    // Is the spillover active and in ingress mode?
    return GUCEF_NULL != route && route->activeSide == route->spilloverBufferSide && msgsFlowedIntoSpillover;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::CSpilloverInfo::IsInActiveUse( void ) const
{GUCEF_TRACE;

    return IsIngressOngoing() || IsEgressOngoing();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubFlowRouter::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.WriterStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubFlowRouter::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.WriterStop() );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubFlowRouter::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.ReaderStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubFlowRouter::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.ReaderStop() );
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::ClearRoutes( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );

    // Unsubscribe the event handlers for each side we have routes for
    TSidePtrToRouteInfoVectorMap::iterator r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        UnsubscribeFrom( (*r).first );
        ++r;
    }

    m_spilloverInfoForTargetsMap.clear();
    m_spilloverInfoMap.clear();
    m_usedInRouteMap.clear();
    m_routeMap.clear();
}

/*-------------------------------------------------------------------------*/

CPubSubClientSidePtr
CPubSubFlowRouter::GetSideWithId( const TPubSubClientSidePtrVector& sides ,
                                  const CORE::CString& sideId             )
{GUCEF_TRACE;

    TPubSubClientSidePtrVector::const_iterator i = sides.begin();
    while ( i != sides.end() )
    {
        const CPubSubClientSidePtr& side = (*i);
        if ( side->GetSideId() == sideId )
        {
            return side;
        }
        ++i;
    }
    return CPubSubClientSidePtr();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::NormalizeConfig( const CPubSubFlowRouterConfig& originalConfig ,
                                    TPubSubClientSidePtrVector& sides             ,
                                    CPubSubFlowRouterConfig& normalizedConfig     )
{GUCEF_TRACE;

    normalizedConfig.ackStyle = originalConfig.ackStyle;

    // First we gather all the 'known' sides
    // If the config says we map to or from anything '*' it still needs to map to an actual side

    CORE::CString::StringSet knownSides;

    TPubSubClientSidePtrVector::const_iterator i = sides.begin();
    while ( i != sides.end() )
    {
        knownSides.insert( (*i)->GetSideId() );
        ++i;
    }

    CPubSubFlowRouterConfig::PubSubFlowRouteConfigVector::const_iterator n = originalConfig.routes.begin();
    while ( n != originalConfig.routes.end() )
    {
        if ( "*" != (*n)->toSideId )
            knownSides.insert( (*n)->toSideId );
        if ( "*" != (*n)->fromSideId )
            knownSides.insert( (*n)->fromSideId );
        ++n;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: There are " + CORE::ToString( knownSides.size() ) + " known sides" );

    // Now create a map of explicit routes
    // These override any wildcard routes and as such we wont auto generate routes for those
    CORE::CValueList explicitRoutes;
    explicitRoutes.SetAllowMultipleValues( true );
    explicitRoutes.SetAllowDuplicates( false );
    n = originalConfig.routes.begin();
    while ( n != originalConfig.routes.end() )
    {
        if ( "*" != (*n)->fromSideId &&  "*" != (*n)->toSideId )
        {
            explicitRoutes.Set( (*n)->fromSideId, (*n)->toSideId );

            // Nothing to normalize, just copy to the normalized output
            normalizedConfig.routes.push_back( (*n) );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: We have an explicit route from \"" +
                    (*n)->fromSideId + "\" to \"" + (*n)->toSideId + "\" with failover side \"" + (*n)->failoverSideId + "\" and spillover side \"" +
                    (*n)->spilloverBufferSideId + "\" and deadletter side \"" + (*n)->deadLetterSideId + "\"" );
        }
        ++n;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: There are " + CORE::ToString( explicitRoutes.GetValueCount() ) + " explicit routes configured" );

    // Now that we know about the IDs of all known sides we can populate the '*' as needed for the wildcard side
    // Note that we intentionally dont handle the * -> * case yet since that is the catch-all applied at the end
    n = originalConfig.routes.begin();
    while ( n != originalConfig.routes.end() )
    {
        if ( "*" == (*n)->toSideId && "*" != (*n)->fromSideId )
        {
             // Try to generate routes using all known sides
             CORE::CString::StringSet::iterator m = knownSides.begin();
             while ( m != knownSides.end() )
             {
                // Don't generate a route where the 'from' and 'to' are the same side
                if ( (*n)->fromSideId != (*m) )
                {
                    // Don't generate a route if an explicit route was specified
                    if ( !explicitRoutes.HasKeyAndValue( (*n)->fromSideId, (*m) ) )
                    {
                        CPubSubFlowRouteConfigPtr route( CPubSubFlowRouteConfig::CreateSharedObjWithParam( *(*n) ) );
                        route->toSideId = (*m);
                        normalizedConfig.routes.push_back( route );

                        explicitRoutes.Set( route->fromSideId, route->toSideId );

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: Generated route from \"" +
                                route->fromSideId + "\" to \"" + route->toSideId + "\" with failover side \"" + route->failoverSideId + "\" and spillover side \"" +
                                route->spilloverBufferSideId + "\" and deadletter side \"" + route->deadLetterSideId + "\"" );
                    }
                }
                ++m;
             }
        }
        else
        if ( "*" == (*n)->fromSideId && "*" != (*n)->toSideId )
        {
             // Try to generate routes using all known sides
             CORE::CString::StringSet::iterator m = knownSides.begin();
             while ( m != knownSides.end() )
             {
                // Don't generate a route where the 'from' and 'to' are the same side
                if ( (*n)->toSideId != (*m) )
                {
                    // Don't generate a route if an explicit route was specified
                    if ( !explicitRoutes.HasKeyAndValue( (*n)->toSideId, (*m) ) )
                    {
                        CPubSubFlowRouteConfigPtr route( CPubSubFlowRouteConfig::CreateSharedObjWithParam( *(*n) ) );
                        route->fromSideId = (*m);
                        normalizedConfig.routes.push_back( route );

                        explicitRoutes.Set( route->fromSideId, route->toSideId );

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: Generated route from \"" +
                                route->fromSideId + "\" to \"" + route->toSideId + "\" with failover side \"" + route->failoverSideId + "\" and spillover side \"" +
                                route->spilloverBufferSideId + "\" and deadletter side \"" + route->deadLetterSideId + "\"" );
                    }
                }
                ++m;
             }
        }

        ++n;
    }

    // Now handle the catch-all *->* route use-case
    bool foundCatchAll = false;
    n = originalConfig.routes.begin();
    while ( n != originalConfig.routes.end() )
    {
        if ( "*" == (*n)->fromSideId &&  "*" == (*n)->toSideId )
        {
            if ( !foundCatchAll )
            {
                foundCatchAll = true;

                // Generate a combo for anything that is not yet defined as an explicit route
                CORE::CString::StringSet::iterator m = knownSides.begin();
                while ( m != knownSides.end() )
                {
                    CORE::CString::StringSet::iterator k = knownSides.begin();
                    while ( k != knownSides.end() )
                    {
                        // Don't generate a route where the 'from' and 'to' are the same side
                        if ( (*m) != (*k) )
                        {
                            // Don't generate a route if an explicit route was specified
                            if ( !explicitRoutes.HasKeyAndValue( (*m), (*k) ) )
                            {
                                CPubSubFlowRouteConfigPtr route( CPubSubFlowRouteConfig::CreateSharedObjWithParam( *(*n) ) );
                                route->fromSideId = (*m);
                                route->toSideId = (*k);
                                normalizedConfig.routes.push_back( route );

                                explicitRoutes.Set( route->fromSideId, route->toSideId );

                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: Generated catch-all route from \"" +
                                        route->fromSideId + "\" to \"" + route->toSideId + "\" with failover side \"" + route->failoverSideId + "\" and spillover side \"" +
                                        route->spilloverBufferSideId + "\" and deadletter side \"" + route->deadLetterSideId + "\"" );
                            }
                        }
                        ++k;
                    }
                    ++m;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:NormalizeConfig: Found another catch-all *->* route in the configuration. This is not allowed. The first found catch-all will apply and the rest is ignored" );
            }
        }
        ++n;
    }

    // if the router config dictates that we should match topics mappings across sides we will do so in bulk now
    // we do also handle events for new topics which will perform the same process as needed per topic

    CPubSubFlowRouterConfig::PubSubFlowRouteConfigVector::iterator r = normalizedConfig.routes.begin();
    while ( r != normalizedConfig.routes.end() )
    {
        CPubSubFlowRouteConfigPtr routeConfig = (*r);
        CPubSubClientSidePtr fromSide = GetSideWithId( sides, routeConfig->fromSideId );
        if ( !fromSide.IsNULL() )
        {
            if ( routeConfig->IsAnyAutoTopicMatchingNeeded() )
            {
                CORE::CString::StringSet fromSideTopicNames;
                if ( fromSide->GetCurrentTopicNames( fromSideTopicNames ) )
                {
                    CORE::CString::StringSet::iterator t = fromSideTopicNames.begin();
                    while ( t != fromSideTopicNames.end() )
                    {
                        const CORE::CString& fromSideTopicName = (*t);

                        // Find of create (config permitting) the topic association config
                        CPubSubFlowRouteTopicConfigPtr topicRouteConfig = routeConfig->FindOrCreateTopicAssociation( fromSideTopicName, false );
                        if ( topicRouteConfig.IsNULL() )
                        {
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: No topic association entry found for side \"" + routeConfig->fromSideId +
                                "\" and from-topic \"" + fromSideTopicName + "\"" );
                        }
                        ++t;
                    }
                }
            }
        }
        ++r;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:NormalizeConfig: There are now " + CORE::ToString( normalizedConfig.routes.size() ) + " explicit routes configured" );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::BuildRoutes( const CPubSubFlowRouterConfig& config ,
                                TPubSubClientSidePtrVector& sides     ,
                                bool attemptConnect                   )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );

    // We might get called multiple times so wipe whatever we had first
    ClearRoutes();

    // Since we support wildcard based config to reduce config burden we need to normalize the config first
    CPubSubFlowRouterConfig normalizedConfig;
    if ( !NormalizeConfig( config, sides, normalizedConfig ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: Failed to normalize config" );
        return false;
    }
    m_normalizedConfig = normalizedConfig;

    // Now we generate the pointer map based on the normalized config
    UInt32 totalPossibleRoutes = 0;
    CPubSubFlowRouterConfig::PubSubFlowRouteConfigVector::const_iterator i = m_normalizedConfig.routes.begin();
    while ( i != m_normalizedConfig.routes.end() )
    {
        const CPubSubFlowRouteConfigPtr routeConfig = (*i);

        // Try to find actual side objects to match against the IDs in the config
        // Note that based on how sloppy the config is we may not find any matches
        UInt32 possibleRoutes = 0;
        CPubSubClientSidePtr fromSide;
        CPubSubClientSidePtr toSide;
        CPubSubClientSidePtr failoverSide;
        CPubSubClientSidePtr spilloverSide;
        CPubSubClientSidePtr deadletterSide;
        TPubSubClientSidePtrVector::iterator n = sides.begin();
        while ( n != sides.end() )
        {
            CORE::CString sideId = (*n)->GetSideId();
            if ( !sideId.IsNULLOrEmpty() )
            {
                if ( routeConfig->fromSideId == sideId )
                    fromSide = (*n);
                else
                if ( routeConfig->toSideId == sideId )
                {
                    toSide = (*n);
                    ++possibleRoutes;
                }
                else
                if ( routeConfig->failoverSideId == sideId )
                {
                    failoverSide = (*n);
                    ++possibleRoutes;
                }
                else
                if ( routeConfig->spilloverBufferSideId == sideId )
                {
                    spilloverSide = (*n);
                    ++possibleRoutes;
                }
                else
                if ( routeConfig->deadLetterSideId == sideId )
                {
                    deadletterSide = (*n);
                    ++possibleRoutes;
                }
            }
            ++n;
        }

        // The minimum you need is from and to sides, the rest is optional
        if ( !fromSide.IsNULL() && !toSide.IsNULL() )
        {
            // We found the side combo for which we have a route config
            TRouteInfoVector& multiRouteInfo = m_routeMap[ fromSide.GetPointerAlways() ];
            multiRouteInfo.push_back( CRouteInfo() );
            CRouteInfo& routeInfo = multiRouteInfo.back();
            routeInfo.fromSide = fromSide.GetPointerAlways();
            routeInfo.toSide = toSide.GetPointerAlways();
            routeInfo.failoverSide = failoverSide.GetPointerAlways();
            routeInfo.spilloverBufferSide = spilloverSide.GetPointerAlways();
            routeInfo.deadLetterSide = deadletterSide.GetPointerAlways();
            routeInfo.routeConfig = routeConfig;

            totalPossibleRoutes += possibleRoutes;
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: Unable to find sides referenced in route " +
                routeConfig->fromSideId + " -> " + routeConfig->toSideId + ". Route has no effect" );
        }

        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: Routes are now defined for " +
        CORE::ToString( m_routeMap.size() ) + " source sides, with a total of " + CORE::ToString( totalPossibleRoutes ) + " possible routes" );

    // Validate side capabilities relative to the assigned roles
    CORE::UInt32 sideClientCapabilityErrors = 0;
    TSidePtrToRouteInfoVectorMap::iterator r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        CPubSubClientSide* fromSide = (*r).first;
        TRouteInfoVector& multiRouteInfo = (*r).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);

            if ( GUCEF_NULL != routeInfo.toSide )
            {
                CPubSubClientFeatures features;
                if ( routeInfo.toSide->GetPubSubClientSupportedFeatures( features ) )
                {
                    // the 'to' side must be capable of:
                    //      - receiving messages from the 'from' side and thus for the client that means capable of publishing

                    bool isValid = true;
                    if ( !features.supportsPublishing )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: 'to' side's client does not support publishing. This is invalid, disabling route: \"" +
                            fromSide->GetSideId() + "\" -> \"" + routeInfo.toSide->GetSideId() );
                        isValid = false;
                        ++sideClientCapabilityErrors;
                    }

                    if ( !isValid )
                        routeInfo.toSide = GUCEF_NULL;
                }
            }
            if ( GUCEF_NULL != routeInfo.deadLetterSide )
            {
                CPubSubClientFeatures features;
                if ( routeInfo.deadLetterSide->GetPubSubClientSupportedFeatures( features ) )
                {
                    // the 'dead letter' side must be capable of:
                    //      - receiving messages from the 'from' side and thus for the client that means capable of publishing

                    bool isValid = true;
                    if ( !features.supportsPublishing )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: 'dead letter' side's client does not support publishing. This is invalid, disabling route: \"" +
                            fromSide->GetSideId() + "\" -> \"" + routeInfo.deadLetterSide->GetSideId() );
                        isValid = false;
                        ++sideClientCapabilityErrors;
                    }

                    if ( !isValid )
                        routeInfo.deadLetterSide = GUCEF_NULL;
                }
            }
            if ( GUCEF_NULL != routeInfo.failoverSide )
            {
                CPubSubClientFeatures features;
                if ( routeInfo.failoverSide->GetPubSubClientSupportedFeatures( features ) )
                {
                    // the 'failover' side must be capable of:
                    //      - receiving messages from the 'from' side and thus for the client that means capable of publishing

                    bool isValid = true;
                    if ( !features.supportsPublishing )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: 'failover' side's client does not support publishing. This is invalid, disabling route: \"" +
                            fromSide->GetSideId() + "\" -> \"" + routeInfo.failoverSide->GetSideId() );
                        isValid = false;
                        ++sideClientCapabilityErrors;
                    }

                    if ( !isValid )
                        routeInfo.failoverSide = GUCEF_NULL;
                }
            }
            if ( GUCEF_NULL != routeInfo.spilloverBufferSide )
            {
                bool isValid = true;

                // The spillover only works if you also have a surviving 'to' or 'failover' side
                // since its not a final destination in of itself
                CPubSubClientFeatures features;
                if ( GUCEF_NULL != routeInfo.toSide || GUCEF_NULL != routeInfo.failoverSide )
                {
                    if ( routeInfo.spilloverBufferSide->GetPubSubClientSupportedFeatures( features ) )
                    {
                        // the 'spill over' side must be capable of:
                        //      - receiving messages from the 'from' side and thus for the client that means capable of publishing
                        //      - read back messages from the 'spillover' side and thus for the client that means capable of subscribing

                        if ( !features.supportsPublishing )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: 'spillover' side's client does not support publishing. This is invalid, disabling route: \"" +
                                fromSide->GetSideId() + "\" -> \"" + routeInfo.spilloverBufferSide->GetSideId() );
                            isValid = false;
                            ++sideClientCapabilityErrors;
                        }
                        if ( !features.supportsSubscribing )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: 'spillover' side's client does not support subscribing. This is invalid, disabling route: \"" +
                                fromSide->GetSideId() + "\" -> \"" + routeInfo.spilloverBufferSide->GetSideId() );
                            isValid = false;
                            ++sideClientCapabilityErrors;
                        }
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: having a 'spillover' side requires also having a surviving 'to' or 'failover' side. Having neither is invalid, disabling route: \"" +
                        fromSide->GetSideId() + "\" -> \"" + routeInfo.spilloverBufferSide->GetSideId() );
                    isValid = false;
                }

                // We now also check if its unique, which is a requirement, and add it to a lookup map for faster ack management
                TSidePtrToSpilloverInfoMap::iterator s = m_spilloverInfoMap.find( routeInfo.spilloverBufferSide );
                if ( s == m_spilloverInfoMap.end() )
                {
                    // Add this one to the lookup map as part of the validation
                    // this is both initialization as it is uniqueness validation
                    CSpilloverInfo& newSpilloverInfo = m_spilloverInfoMap[ routeInfo.spilloverBufferSide ];
                    newSpilloverInfo.route = &routeInfo;
                    newSpilloverInfo.endOfDataEventSupported = features.supportsSubscriptionEndOfDataEvent;
                    newSpilloverInfo.supportsDiscoveryOfAvailableTopics = features.supportsDiscoveryOfAvailableTopics;
                    routeInfo.spilloverInfo = &newSpilloverInfo;

                    // We also add the non-spillover sides for the return flow
                    // this will be used to link back acks from the spillover's egress
                    if ( GUCEF_NULL != routeInfo.toSide )
                        m_spilloverInfoForTargetsMap[ routeInfo.toSide ] = &newSpilloverInfo;
                    if ( GUCEF_NULL != routeInfo.failoverSide )
                        m_spilloverInfoForTargetsMap[ routeInfo.failoverSide ] = &newSpilloverInfo;

                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: Spillover buffers must be unique to a given route, they cannot be shared. The configuration is invalid for spillover buffer \"" + routeInfo.spilloverBufferSide->GetSideId() +
                        "\" with 'from' side \"" + fromSide->GetSideId() + "\". Removing spillover buffer feature from this route as a best-effort fallback" );
                    isValid = false;
                }

                if ( !isValid )
                    routeInfo.spilloverBufferSide = GUCEF_NULL;
            }

            // Check if the route is still partially viable even with errors
            // This requires at least 1 surviving pathway
            if ( GUCEF_NULL == routeInfo.toSide              &&
                 GUCEF_NULL == routeInfo.failoverSide        &&
                 GUCEF_NULL == routeInfo.spilloverBufferSide )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: No surviving viable pathways for side " + fromSide->GetSideId() + " post client capability verification" );
                return false;
            }

            ++n;
        }
        ++r;
    }

    if ( 0 == sideClientCapabilityErrors )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter:BuildRoutes: No issues were detected with pubsub client capabilities for the assigned roles on the configured routes" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter:BuildRoutes: A total of " + CORE::ToString( sideClientCapabilityErrors ) + " issues were detected with pubsub client capabilities for the assigned roles on the configured routes" );
    }

    // For any valid spillovers we will need to auto generate, if it was not explicitly specified, a route from that spillover to the intended 'to' destination
    // since the originally intended traffic was 'from' -> 'to' but it will really at times flow 'from' -> 'spillover' -> 'to'
    // hence we need the 'spillover' -> 'to' route defined for proper flow to occur
    r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        CPubSubClientSide* fromSide = (*r).first;
        TRouteInfoVector& multiRouteInfo = (*r).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);

            if ( GUCEF_NULL != routeInfo.spilloverBufferSide )
            {
                CORE::CString spilloverSideId = routeInfo.spilloverBufferSide->GetSideId();

                // We found a validated spillover side
                // now lets make sure we have an egress route for it
                TSidePtrToRouteInfoVectorMap::iterator r2 = m_routeMap.find( routeInfo.spilloverBufferSide );
                if ( r2 != m_routeMap.end() )
                {
                    CPubSubClientSide* toSide = GUCEF_NULL != routeInfo.toSide ? routeInfo.toSide : routeInfo.failoverSide;
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:BuildRoutes: Egress route for spillover buffer \"" + spilloverSideId +
                        "\" to side \"" + toSide->GetSideId() + "\" exists as an explicitly provided configured route. No such route will be auto-generated" );

                    // Ensure the egress routes are marked as such
                    TRouteInfoVector& spilloverMultiRouteInfo = (*r2).second;
                    TRouteInfoVector::iterator n2 = spilloverMultiRouteInfo.begin();
                    while ( n2 != spilloverMultiRouteInfo.end() )
                    {
                        CRouteInfo& spilloverRouteInfo = (*n2);
                        spilloverRouteInfo.isSpilloverEgressRoute = true;
                        ++n2;
                    }
                }
                else
                {
                    // We are lacking an egress route
                    // Since this is a spillover its pretty clear what the intent is and as such we can auto-generate this route
                    // Doing so cuts down on the complexity of the config that needs to be provided while retaining the option to explicitly configure should it be needed
                    TRouteInfoVector& spilloverMultiRouteInfo = m_routeMap[ routeInfo.spilloverBufferSide ];

                    // The same destinations still apply coming out of the spillover
                    // we do need to get rid of the spillover itself to make sure we dont create an endless loop
                    CRouteInfo newSpilloverBypassRoute( routeInfo );
                    newSpilloverBypassRoute.fromSide = routeInfo.spilloverBufferSide;
                    newSpilloverBypassRoute.spilloverBufferSide = GUCEF_NULL;
                    newSpilloverBypassRoute.spilloverInfo = GUCEF_NULL;
                    newSpilloverBypassRoute.isSpilloverEgressRoute = true;

                    spilloverMultiRouteInfo.push_back( newSpilloverBypassRoute );
                    CRouteInfo& spilloverBypassRoute = spilloverMultiRouteInfo.back();

                    bool foundSpilloverEgressRouteConfig = false;
                    CPubSubFlowRouterConfig::PubSubFlowRouteConfigVector::iterator c = m_normalizedConfig.routes.begin();
                    while ( c != m_normalizedConfig.routes.end() )
                    {
                        CPubSubFlowRouteConfigPtr routeConfig = (*c);
                        if ( routeConfig->fromSideId == spilloverSideId )
                        {
                            foundSpilloverEgressRouteConfig = true;
                        }
                        ++c;
                    }

                    if ( !foundSpilloverEgressRouteConfig )
                    {
                        CPubSubClientSide* toSide = GUCEF_NULL != routeInfo.toSide ? routeInfo.toSide : routeInfo.failoverSide;
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:BuildRoutes: No route config found for egress route from spillover buffer \"" + spilloverSideId +
                            "\" to side \"" + toSide->GetSideId() + "\". Auto generating egress route config" );

                        // We need to add a route config for this auto-generated route

                        CORE::CString fromSideId = routeInfo.fromSide->GetSideId();

                        if ( !routeInfo.routeConfig.IsNULL() )
                        {
                            CPubSubFlowRouteConfigPtr egressRouteConfig( CPubSubFlowRouteConfig::CreateSharedObjWithParam( *routeInfo.routeConfig ) );
                            egressRouteConfig->fromSideId = spilloverSideId;
                            egressRouteConfig->spilloverBufferSideId.Clear();
                            spilloverBypassRoute.routeConfig = egressRouteConfig;
                            m_normalizedConfig.routes.push_back( egressRouteConfig );
                        }
                        else
                        {
                            CPubSubClientSide* toSide = GUCEF_NULL != routeInfo.toSide ? routeInfo.toSide : routeInfo.failoverSide;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:BuildRoutes: No route config found which uses spillover \"" + spilloverSideId +
                                "\" and 'from' side \"" + fromSideId + "\". This should not happen since we have instantiations" );
                        }
                    }

                    CPubSubClientSide* toSide = GUCEF_NULL != routeInfo.toSide ? routeInfo.toSide : routeInfo.failoverSide;
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter:BuildRoutes: Auto-generated egress route for spillover buffer \"" + spilloverSideId +
                        "\" to side \"" + toSide->GetSideId() + "\"" );
                }

                // The spillover is a special case and we reconfigure it as needed anyway
                // as such there is no point to connect it only to then later reconnect it
                routeInfo.spilloverBufferSide->SetPerformConnectOnTaskStart( false );
            }

            ++n;
        }
        ++r;
    }

    // Now set up the reverse lookup map for efficient updates to routes
    TSidePtrToRouteInfoPtrSetMap sideUsedInMap;
    r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        TRouteInfoVector& multiRouteInfo = (*r).second;

        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);

            if ( GUCEF_NULL != routeInfo.toSide )
                sideUsedInMap[ routeInfo.toSide ].insert( &routeInfo );
            if ( GUCEF_NULL != routeInfo.failoverSide )
                sideUsedInMap[ routeInfo.failoverSide ].insert( &routeInfo );
            if ( GUCEF_NULL != routeInfo.spilloverBufferSide )
                sideUsedInMap[ routeInfo.spilloverBufferSide ].insert( &routeInfo );
            if ( GUCEF_NULL != routeInfo.deadLetterSide )
                sideUsedInMap[ routeInfo.deadLetterSide ].insert( &routeInfo );

            CPubSubClientSide* fromSide = (*r).first;
            if ( GUCEF_NULL != fromSide )
                sideUsedInMap[ fromSide ].insert( &routeInfo );

            ++n;
        }

        ++r;
    }
    // Sets are slower than vectors so we convert to a vector for runtime efficiency
    // The set already got rid of duplicate links and this will remain static going forward
    TSidePtrToRouteInfoPtrSetMap::iterator s = sideUsedInMap.begin();
    while ( s != sideUsedInMap.end() )
    {
        TRouteInfoPtrSet& ptrSet = (*s).second;
        TRouteInfoPtrSet::iterator s2 = ptrSet.begin();
        while ( s2 != ptrSet.end() )
        {
            m_usedInRouteMap[ (*s).first ].push_back( (*s2) );
            ++s2;
        }
        ++s;
    }

    // Now that all route info is build at the 'side' level we also layer in the topic level mappings
    // these are to topic objects, leveraging the sides we just setup to gain access to those
    r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        TRouteInfoVector& multiRouteInfo = (*r).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);
            routeInfo.MatchAllTopicRouteConfigs();

            ++n;
        }

        ++r;
    }

    // Now set up all the event handlers
    r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        TRouteInfoVector& multiRouteInfo = (*r).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);
            routeInfo.routeSwitchingTimer.SetInterval( normalizedConfig.routeSwitchingTimerIntervalInMs );
            RegisterRouteEventHandlers( routeInfo );
            ++n;
        }

        ++r;
    }

    UpdatePulseGeneratorUsage();

    // Now connect the routes if desired
    if ( attemptConnect )
    {
        r = m_routeMap.begin();
        while ( r != m_routeMap.end() )
        {
            TRouteInfoVector& multiRouteInfo = (*r).second;
            TRouteInfoVector::iterator n = multiRouteInfo.begin();
            while ( n != multiRouteInfo.end() )
            {
                CRouteInfo& routeInfo = (*n);
                DetermineActiveRoute( routeInfo, true );
                ++n;
            }

            ++r;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::RegisterRouteEventHandlers( CRouteInfo& routeInfo )
{GUCEF_TRACE;

    TEventCallback callback( this, &CPubSubFlowRouter::OnRouteSwitchTimerCycle );
    SubscribeTo( &routeInfo.routeSwitchingTimer ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );

    // For efficiency to avoid an additional lookup we use the user data facility
    // to link back to the parent route info for the timer handler
    routeInfo.routeSwitchingTimer.SetOpaqueUserData( &routeInfo );

    if ( GUCEF_NULL != routeInfo.fromSide )
        RegisterSideEventHandlers( routeInfo.fromSide );
    if ( GUCEF_NULL != routeInfo.toSide )
        RegisterSideEventHandlers( routeInfo.toSide );
    if ( GUCEF_NULL != routeInfo.failoverSide )
        RegisterSideEventHandlers( routeInfo.failoverSide );
    if ( GUCEF_NULL != routeInfo.spilloverBufferSide )
        RegisterSideEventHandlers( routeInfo.spilloverBufferSide );
    if ( GUCEF_NULL != routeInfo.deadLetterSide )
        RegisterSideEventHandlers( routeInfo.deadLetterSide );
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::UpdatePulseGeneratorUsage( void )
{GUCEF_TRACE;

    TSidePtrToRouteInfoVectorMap::iterator r = m_routeMap.begin();
    while ( r != m_routeMap.end() )
    {
        TRouteInfoVector& multiRouteInfo = (*r).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo& routeInfo = (*n);
            routeInfo.routeSwitchingTimer.SetPulseGenerator( GetPulseGenerator() );
            ++n;
        }
        ++r;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );
    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    UpdatePulseGeneratorUsage();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::SaveConfig( CPubSubFlowRouterConfig& cfg ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_lock );
    cfg = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::LoadConfig( const CPubSubFlowRouterConfig& cfg )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );
    m_config = cfg;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::AcknowledgeReceiptForSide( CPubSubClientSide* msgReceiverSide ,
                                              CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    {
        // since spillover buffers are unique for a given route, garanteed by route building validation, logically only 1 thread would be
        // updating the spillover info, we just need the mapping to remain in existance.
        // as such a read lock will suffice
        MT::CScopeReaderLock lock( m_lock );

        // Check if msgReceiverSide is a spillover target
        TSidePtrToSpilloverInfoPtrMap::iterator i = m_spilloverInfoForTargetsMap.find( msgReceiverSide );
        if ( i != m_spilloverInfoForTargetsMap.end() )
        {
            // Check if this is normal flow or spillover egress
            CSpilloverInfo* spilloverInfo = (*i).second;
            if ( spilloverInfo->IsEgressOngoing() )
            {
                --spilloverInfo->spilloverIngressMsgCount;

                // If the backend does not support EOD events the message counters will be the only thing to go on
                if ( !spilloverInfo->endOfDataEventSupported && 0 <= spilloverInfo->spilloverIngressMsgCount )
                {
                    // reset the flag to indicate whether any msgs flowed into the spillover
                    // this flag is used to reduce redundant spillover reconfiguration which takes time
                    spilloverInfo->msgsFlowedIntoSpillover = false;
                }
            }
        }
        else
        {
            // Check if msgReceiverSide is a spillover
            TSidePtrToSpilloverInfoMap::iterator n = m_spilloverInfoMap.find( msgReceiverSide );
            if ( i != m_spilloverInfoForTargetsMap.end() )
            {
                CSpilloverInfo& spilloverInfo = (*n).second;
                ++spilloverInfo.spilloverIngressMsgCount;
            }
        }
    }

    // Right now we dont use the route info but instead use the source location
    // we can revisit this later but it is fragile due to reliance on GetOpaqueUserData()
    // Due to this we right now dont need a lock here for the ack portion

    // Note that acks are always ONLY to the side that originated the message since that side is the only
    // one that knows about that specific message and whatever conventions its abstracting wrt the underlying message format

    CPubSubClientTopicBasicPtr originTopic = msg->GetOriginClientTopic();
    if ( !originTopic.IsNULL() )
    {
        CPubSubClient* originClient = originTopic->GetClient();
        if ( GUCEF_NULL != originClient )
        {
            CPubSubClientSide* originSide = static_cast< CPubSubClientSide* >( originClient->GetOpaqueUserData() );
            if ( GUCEF_NULL != originSide )
            {
                return originSide->AcknowledgeReceipt( msg );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):AcknowledgeReceiptForSide: Unable to ack receipt of message to origin topic since parent client owning the origin topic provided on the message does not have a link back to the owner pubsub side. This should never happen." );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                "):AcknowledgeReceiptForSide: Unable to ack receipt of message to origin topic since no parent client owns the origin topic provided on the message. This should never happen." );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
            "):AcknowledgeReceiptForSide: Unable to ack receipt of message to origin topic since no origin topic was provided on the message. Check your config and backend feature compatibility" );
        return false;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::PublishMsgs( CPubSubClientSide* fromSide                          ,
                                const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                                RouteType routeType                                  )
{GUCEF_TRACE;

    // Note that messages in a given batch, as given to the router, are assumed to all come
    // from the same source topic. Without this assumption we would have to sort all messages which is a big
    // performance hit
    CPubSubClientTopic* fromTopic = GUCEF_NULL;
    if ( !msgs.empty() )
    {
        fromTopic = msgs.front()->GetOriginClientTopic().GetPointerAlways();
    }

    MT::CScopeReaderLock lock( m_lock );

    bool publishIstotalSuccess = true;
    TSidePtrToRouteInfoVectorMap::iterator i = m_routeMap.find( fromSide );
    if ( i != m_routeMap.end() )
    {
        TRouteInfoVector& multiRouteInfo = (*i).second;
        TRouteInfoVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            bool errorOccured = false;
            CPubSubClientSide* targetSide = GUCEF_NULL;
            CPubSubClientTopic* targetTopic = GUCEF_NULL;

            try
            {

                CRouteInfo& routeInfo = (*n);

                // Make sure we are not in spillover egress mode
                // if spillover egress is active the 'from' side of the route cannot published to the primary 'to' side
                // Doing so would mess up sequencing and intermix new and older messages. We should just wait for the spillover egress to finish
                if ( !routeInfo.IsSpilloverEgressActive() )
                {
                    CRouteTopicLinks* topicLinks = GetTargetTopicLinks( fromTopic, routeInfo, errorOccured );

                    switch ( routeType )
                    {
                        case RouteType::Active:
                        {
                            targetSide = routeInfo.activeSide;
                            if ( GUCEF_NULL != topicLinks )
                            {
                                targetTopic = topicLinks->activeTopic;
                                if ( GUCEF_NULL == targetTopic )
                                {
                                    // The active destination topic had potentially not yet been determined, set it now lazy init style
                                    routeInfo.SwitchTopicLinksActiveTopic( routeInfo.activeSideType, *topicLinks );
                                    targetTopic = topicLinks->activeTopic;
                                }
                            }
                            break;
                        }

                        case RouteType::Primary:
                        {
                            targetSide = routeInfo.toSide;
                            if ( GUCEF_NULL != topicLinks )
                                targetTopic = topicLinks->toTopic;
                            break;
                        }
                        case RouteType::Failover:
                        {
                            targetSide = routeInfo.failoverSide;
                            if ( GUCEF_NULL != topicLinks )
                                targetTopic = topicLinks->failoverTopic;
                            break;
                        }
                        case RouteType::SpilloverBuffer:
                        {
                            targetSide = routeInfo.spilloverBufferSide;
                            if ( GUCEF_NULL != topicLinks )
                                targetTopic = topicLinks->spilloverTopic;
                            break;
                        }
                        case RouteType::DeadLetter:
                        {
                            targetSide = routeInfo.deadLetterSide;
                            if ( GUCEF_NULL != topicLinks )
                                targetTopic = topicLinks->deadletterTopic;
                            break;
                        }
                        default: { break; }
                    }
                }

                if ( GUCEF_NULL != targetSide && !errorOccured )
                {
                    if ( targetSide->PublishMsgs( msgs, targetTopic ) )
                    {
                        if ( routeInfo.spilloverBufferSide == targetSide && GUCEF_NULL != routeInfo.spilloverInfo )
                            routeInfo.spilloverInfo->msgsFlowedIntoSpillover = true;
                    }
                    else
                    {
                        publishIstotalSuccess = false;
                    }
                }
                else
                {
                    publishIstotalSuccess = false;
                }
            }
            catch ( const timeout_exception& e )
            {
                CORE::CString sideId;
                if ( GUCEF_NULL != targetSide )
                    sideId = targetSide->GetSideId();

                CORE::CString topicName;
                if ( GUCEF_NULL != targetTopic )
                    topicName = targetTopic->GetTopicName();

                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                    "):PublishMsgs: Timeout exception trying to publish to side \"" + sideId + "\" and topic \"" + topicName + "\"" );

                publishIstotalSuccess = false;
            }
            catch ( const std::exception& e )
            {
                CORE::CString sideId;
                if ( GUCEF_NULL != targetSide )
                    sideId = targetSide->GetSideId();

                CORE::CString topicName;
                if ( GUCEF_NULL != targetTopic )
                    topicName = targetTopic->GetTopicName();

                GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                    "):PublishMsgs: Exception trying to publish to side \"" + sideId + "\" and topic \"" + topicName + "\": " +
                    CORE::CString( e.what() ) );

                publishIstotalSuccess = false;
            }

            ++n;
        }
    }
    else
    {
        // no route from this side
        publishIstotalSuccess = false;
    }

    return publishIstotalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::IsTrackingInFlightPublishedMsgsForAcksNeeded( CPubSubClientSide* sideWeAskFor ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == sideWeAskFor )
        return false;
    
    MT::CScopeReaderLock lock( m_lock );
    
    // Find routes this side is used in
    TSidePtrToRouteInfoPtrVectorMap::const_iterator i = m_usedInRouteMap.find( sideWeAskFor );
    if ( i != m_usedInRouteMap.end() )
    {
        // For each route this side is used in check if tracking is needed
        // If just one side needs tracking we need to track for the entire route
        const TRouteInfoPtrVector& multiRouteInfo = (*i).second;
        TRouteInfoPtrVector::const_iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            const CRouteInfo* routeInfo = (*n);

            // We need tracking as soon as 1 side anywhere needs a subscriber ack
            bool trackingNeeded = IsTrackingInFlightPublishedMsgsForAcksNeededForSide( routeInfo->fromSide ) ||
                                  IsTrackingInFlightPublishedMsgsForAcksNeededForSide( routeInfo->toSide ) ||
                                  IsTrackingInFlightPublishedMsgsForAcksNeededForSide( routeInfo->failoverSide ) ||
                                  IsTrackingInFlightPublishedMsgsForAcksNeededForSide( routeInfo->deadLetterSide ) ||
                                  IsTrackingInFlightPublishedMsgsForAcksNeededForSide( routeInfo->spilloverBufferSide );

            if ( trackingNeeded )
                return trackingNeeded;
            ++n;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::IsTrackingInFlightPublishedMsgsForAcksNeededForSide( const CPubSubClientSide* side ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != side )
    {
        if ( side->HasSubscribersNeedingAcks() )
            return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::ShouldSideBeConnected( CPubSubClientSide* side ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != side )
    {
        MT::CScopeReaderLock lock( m_lock );

        TSidePtrToRouteInfoPtrVectorMap::const_iterator i = m_usedInRouteMap.find( side );
        if ( i != m_usedInRouteMap.end() )
        {
            const TRouteInfoPtrVector& multiRouteInfo = (*i).second;
            TRouteInfoPtrVector::const_iterator n = multiRouteInfo.begin();
            while ( n != multiRouteInfo.end() )
            {
                const CRouteInfo* routeInfo = (*n);

                if ( GUCEF_NULL == routeInfo->activeSide )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):ShouldSideBeConnected: Determined that side \"" + side->GetSideId() + "\" should not be connected at this time due to lacking an active destination for the route" );
                    return false;
                }

                if ( routeInfo->fromSide == side         &&
                     routeInfo->IsSpilloverInActiveUse() )
                {
                    // For destinations we always want to be connected so they are ready for use
                    // for sources this is usually also true except for when a spillover is used
                    // a spillover alters the 'from' of the route dynamically
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):ShouldSideBeConnected: Determined that side \"" + side->GetSideId() + "\" should not be connected at this time due to spillover activity" );
                    return false;
                }
                ++n;
            }
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::RegisterSideEventHandlers( CPubSubClientSide* side )
{GUCEF_TRACE;

    if ( GUCEF_NULL != side )
    {
        TEventCallback callback( this, &CPubSubFlowRouter::OnSideHealthStatusChange );
        SubscribeTo( side                                       ,
                     CPubSubClientSide::HealthStatusChangeEvent ,
                     callback                                   );

        TEventCallback callback2( this, &CPubSubFlowRouter::OnSidePubSubClientInstantiation );
        SubscribeTo( side                                              ,
                     CPubSubClientSide::PubSubClientInstantiationEvent ,
                     callback2                                         );

        CPubSubClientPtr currentClient = side->GetCurrentUnderlyingPubSubClient();
        if ( !currentClient.IsNULL() )
            RegisterSidePubSubClientEventHandlers( currentClient );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::RegisterSidePubSubClientEventHandlers( CPubSubClientPtr& client )
{GUCEF_TRACE;

    if ( !client.IsNULL() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
            "):RegisterSidePubSubClientEventHandlers: Hooking up event handlers for client with implementation type: " + client->GetClassTypeName()  );

        CPubSubClientFeatures clientFeatures;
        client->GetSupportedFeatures( clientFeatures );

        // client and topic level subscriptions are currently only used for EOD
        // if we have another use-case later remove this check
        if ( clientFeatures.supportsSubscriptionEndOfDataEvent )
        {
            TEventCallback callback( this, &CPubSubFlowRouter::OnSidePubSubClientTopicCreation );
            SubscribeTo( client.GetPointerAlways()              ,
                         CPubSubClient::TopicAccessCreatedEvent ,
                         callback                               );

            TEventCallback callback2( this, &CPubSubFlowRouter::OnSidePubSubClientTopicsAutoCreation );
            SubscribeTo( client.GetPointerAlways()                   ,
                         CPubSubClient::TopicsAccessAutoCreatedEvent ,
                         callback2                                   );

            // register event handlers for all already existing topics (if any)
            RegisterSidePubSubClientTopicEventHandlers( client );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::RegisterSidePubSubClientTopicEventHandlers( CPubSubClientTopicBasicPtr topicAccess )
{GUCEF_TRACE;

    if ( !topicAccess.IsNULL() )
    {
        TEventCallback callback( this, &CPubSubFlowRouter::OnSidePubSubClientTopicEndOfData );
        SubscribeTo( topicAccess.GetPointerAlways()                 ,
                     CPubSubClientTopic::SubscriptionEndOfDataEvent ,
                     callback                                       );

        // Check init state in case we missed it before we set up the event handler
        if ( topicAccess->IsSubscriptionAtEndOfData() )
            OnSidePubSubClientTopicEndOfData( topicAccess.GetPointerAlways() );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::RegisterSidePubSubClientTopicEventHandlers( CPubSubClientPtr& client )
{GUCEF_TRACE;

    if ( !client.IsNULL() )
    {
        CPubSubClient::PubSubClientTopicSet topics;
        client->GetAllCreatedTopicAccess( topics );

        CPubSubClient::PubSubClientTopicSet::iterator i = topics.begin();
        while ( i != topics.end() )
        {
            RegisterSidePubSubClientTopicEventHandlers( (*i) );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::ConfigureSpillover( CPubSubClientSide* spilloverSide, bool flowIntoSpillover, bool attemptConnect )
{GUCEF_TRACE;

    // pull a copy of the config
    CPubSubSideChannelSettingsPtr originalSideSettings = spilloverSide->GetSideSettings();
    CPubSubSideChannelSettingsPtr sideSettings = CPubSubSideChannelSettings::CreateSharedObjWithParam( *originalSideSettings );

    // (Re)Configure for the intended flow direction
    if ( flowIntoSpillover )
    {
        // Configure for ingress aka publishing
        sideSettings->pubsubClientConfig.desiredFeatures.supportsPublishing = true;
        sideSettings->pubsubClientConfig.desiredFeatures.supportsSubscribing = false;

        CPubSubClientConfig::TPubSubClientTopicConfigPtrVector& topicConfigs = sideSettings->pubsubClientConfig.topics;
        CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator t = topicConfigs.begin();
        while ( t != topicConfigs.end() )
        {
            CPubSubClientTopicConfigPtr topicConfig = (*t);

            topicConfig->needPublishSupport = true;
            topicConfig->needSubscribeSupport = false;

            ++t;
        }
    }
    else
    {
        // Configure for egress aka subscribing
        sideSettings->pubsubClientConfig.desiredFeatures.supportsPublishing = false;
        sideSettings->pubsubClientConfig.desiredFeatures.supportsSubscribing = true;

        CPubSubClientConfig::TPubSubClientTopicConfigPtrVector& topicConfigs = sideSettings->pubsubClientConfig.topics;
        CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator t = topicConfigs.begin();
        while ( t != topicConfigs.end() )
        {
            CPubSubClientTopicConfigPtr topicConfig = (*t);

            topicConfig->needPublishSupport = false;
            topicConfig->needSubscribeSupport = true;

            ++t;
        }
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
        "):ConfigureSpillover: Side \"" + spilloverSide->GetSideId() + "\" is being reconfigured for " + ( flowIntoSpillover ? CORE::CString( "ingress" ) : CORE::CString( "egress" ) ) );

    if ( spilloverSide->DisconnectPubSubClient() )
    {
        if ( spilloverSide->LoadConfig( sideSettings ) )
        {
            if ( attemptConnect )
                return spilloverSide->ConnectPubSubClient( true );
            else
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::ConnectSide( CPubSubClientSide* sideToConnect )
{GUCEF_TRACE;

    if ( GUCEF_NULL != sideToConnect )
    {
        if ( sideToConnect->ConnectPubSubClient( false ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                "):ConnectSide: side successfully connected: " + sideToConnect->GetSideId() );
            return true;
        }
        else
        {
            if ( sideToConnect->IsPubSubClientInfraReadyToConnect() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):ConnectSide: side did not connect successfully: " + sideToConnect->GetSideId() );
                return false;
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):ConnectSide: side is not ready to connect, need to defer for side: " + sideToConnect->GetSideId() );
                return false;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouter::ConnectRoutesForFromSide( CPubSubClientSide* fromSide )
{GUCEF_TRACE;

    if ( GUCEF_NULL == fromSide )
        return false;

    bool allTargetsAreConnected = true;
    TSidePtrToRouteInfoVectorMap::iterator n = m_routeMap.find( fromSide );
    if ( n != m_routeMap.end() )
    {
        TRouteInfoVector& routes = (*n).second;
        TRouteInfoVector::iterator i = routes.begin();
        while ( i != routes.end() )
        {
            CRouteInfo& specificRoute = (*i);

            if ( GUCEF_NULL == specificRoute.activeSide )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):ConnectRoutesForFromSide: A route does not have an active side yet, we will have to determine it first" );

                DetermineActiveRouteDestination( specificRoute, true );
            }

            allTargetsAreConnected = ConnectSide( specificRoute.activeSide ) && allTargetsAreConnected;
            ++i;
        }
    }

    if ( allTargetsAreConnected )
    {
        // Only connect the 'from' side if we have viable destinations where we can send the resulting data
        return ConnectSide( fromSide );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::DetermineFirstActiveRoute( CRouteInfo& routeInfo, bool attemptConnect )
{GUCEF_TRACE;

    // For a first run we cover the scenario of having a spillover which was not fully drained for whatever
    // reason when last we ran (crash?). As such data would remain stuck in the spillover until we happened to need it again
    // and there is no telling when that will be. As such we need to make sure the spillover gets drained on startup.
    if ( GUCEF_NULL != routeInfo.spilloverBufferSide )
    {
        if ( !routeInfo.spilloverBufferSide->IsPubSubClientInfraReadyToConnect() )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                "):DetermineFirstActiveRoute: The route has a spillover configured but its not ready yet, deferring." );
            routeInfo.routeSwitchingTimer.SetEnabled( true );
            return;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
            "):DetermineFirstActiveRoute: The route has a spillover configured and this is the first active route determination. Commencing spillover egress to ensure there is no data stuck there" );

        routeInfo.spilloverInfo->msgsFlowedIntoSpillover = true;    // init state assuming msgs previously flowed into the spillover
        routeInfo.flowingIntoSpillover = false;                     // egress mode
        if ( !ConfigureSpillover( routeInfo.spilloverBufferSide, false, attemptConnect ) )
        {
            // Initial connect did not succeed, rely on timer to try again
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                "):DetermineFirstActiveRoute: Initial spillover configfure and connect failed. Will retry" );
            routeInfo.routeSwitchingTimer.SetEnabled( true );
            return;
        }

        // Make sure we discover any and all spillover topic automatically if so configured
        // This is to ensure that if the topics available in the spillover are not known to the router between runs we still are
        // able to egress data from the spillover from such a previous run
        if ( routeInfo.spilloverInfo->supportsDiscoveryOfAvailableTopics      &&
             routeInfo.routeConfig->egressAllDiscoveredSpilloverTopicsOnStart )
        {
            CPubSubClientPtr spilloverClient = routeInfo.spilloverBufferSide->GetCurrentUnderlyingPubSubClient();
            if ( !spilloverClient.IsNULL() )
            {
                if ( spilloverClient->BeginTopicDiscovery() )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):DetermineFirstActiveRoute: Commencing spillover topic discovery" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):DetermineFirstActiveRoute: Failed to commence spillover topic discovery. Data might be stuck in the spillover" );
                }
            }
        }
        else
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                "):DetermineFirstActiveRoute: Spillover topic discovery is not supported or not configured to be used. Skipping spillover topic discovery" );
        }

        // grab the spillover egress route belonging to this spillover
        TSidePtrToRouteInfoVectorMap::iterator i = m_routeMap.find( routeInfo.spilloverBufferSide );
        if ( i != m_routeMap.end() )
        {
            // lets make sure the egress routes are connected
            bool allSpilloverEgressPathsConnected = true;
            TRouteInfoVector& spilloverDestinations = (*i).second;
            TRouteInfoVector::iterator n = spilloverDestinations.begin();
            while ( n != spilloverDestinations.end() )
            {
                CRouteInfo& spilloverRoute = (*n);
                if ( GUCEF_NULL == spilloverRoute.activeSide )
                    DetermineActiveRouteDestination( spilloverRoute, attemptConnect );

                if ( attemptConnect )
                {
                    if ( GUCEF_NULL != spilloverRoute.activeSide )
                    {
                        if ( ConnectSide( spilloverRoute.activeSide ) )
                        {
                            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                                "):DetermineFirstActiveRoute: The spillover destination is now connected for route \"" + routeInfo.spilloverBufferSide->GetSideId() +
                                    "\" -> \"" + spilloverRoute.activeSide->GetSideId() + "\" for route with regular 'from' side \"" + routeInfo.fromSide->GetSideId() + "\"" );
                        }
                        else
                        {
                            allSpilloverEgressPathsConnected = false;
                            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                                "):DetermineFirstActiveRoute: Failed to connect spillover egress route. Will retry" );
                        }
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                            "):DetermineFirstActiveRoute: Failed to determine active route for spillover egress route" );
                    }
                }
                ++n;
            }

            if ( attemptConnect && allSpilloverEgressPathsConnected )
            {
                if ( ConnectSide( routeInfo.spilloverBufferSide ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):DetermineFirstActiveRoute: The route is now connected and spillover egress thus commencing for spillover \"" + routeInfo.spilloverBufferSide->GetSideId() +
                            "\" for route with regular 'from' side \"" + routeInfo.fromSide->GetSideId() + "\"" );

                    routeInfo.activeSide = routeInfo.spilloverBufferSide;
                    routeInfo.activeSideType = RouteType::SpilloverBuffer;
                }
            }
        }
        else
        {
            // This should not happen, mappings are incorrect
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                "):DetermineFirstActiveRoute: Egress route for spillover not found" );
        }

        routeInfo.routeSwitchingTimer.SetEnabled( true );
        return;
    }
    else
    {
        // Even if this route does not have a spillover we have to make sure this route is not actually the
        // derived spillover route itself in which case we need to make sure we do not partake in any initialization ourselves
        // independently of the parent route
        if ( !routeInfo.isSpilloverEgressRoute )
        {
            // This is a regular route without any spillover functionality

            if ( attemptConnect )
            {
                // As such we can straightforward connect without special spillover bootstrapping

                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                    "):DetermineFirstActiveRoute: Commencing first route connectivity attempt for routes from side: " + routeInfo.fromSide->GetSideId() );

                if ( ConnectRoutesForFromSide( routeInfo.fromSide ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):DetermineFirstActiveRoute: The route is now connected. \"" + routeInfo.fromSide->GetSideId() +
                            "\" -> \"" + routeInfo.activeSide->GetSideId() + "\"" );
                }
                else
                {
                    // Initial connect did not succeed, rely on timer to try again
                    routeInfo.routeSwitchingTimer.SetEnabled( true );
                }
            }
        }
        else
        {
            //  This is a spillover derived route where the 'from' side is actually a spillover side
            //  in which case do nothing connectivity wise and just determine the active route destination
            if ( GUCEF_NULL != routeInfo.activeSide )
                DetermineActiveRouteDestination( routeInfo, attemptConnect );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::DetermineActiveRouteDestination( CRouteInfo& routeInfo, bool attemptConnect )
{GUCEF_TRACE;

    CORE::UInt64 primarySideHealthDurationInMs = (CORE::UInt64) routeInfo.toSideLastHealthStatusChange.GetTimeDifferenceInMillisecondsToNow();

    // Try our primary 'to' route first
    // This is the normal 99% (hopefully) case
    if ( GUCEF_NULL != routeInfo.toSide &&
         routeInfo.toSideIsHealthy &&
         primarySideHealthDurationInMs > m_config.minPrimarySideGoodHealthDurationBeforeActivationInMs )
    {
        if ( routeInfo.activeSide != routeInfo.toSide )
        {
            // Before (re)activating the primary route check to make sure we didnt use the spillover
            // if so need we need to egress those messages first before restoring the normal route
            if ( !routeInfo.DidMsgsFlowIntoSpillover() )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):DetermineActiveRoute: Switching active route to the Primary side. \"" + routeInfo.fromSide->GetSideId() +
                    "\" -> \"" + routeInfo.toSide->GetSideId() + "\"" );

                routeInfo.activeSide = routeInfo.toSide;
                routeInfo.activeSideType = RouteType::Primary;
                routeInfo.routeSwitchingTimer.SetEnabled( false );
                routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
            }
            else
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):DetermineActiveRoute: Primary side health is restored but spillover absorbed messages. Commencing spillover egress" );

                // Switch from ingress to egress
                if ( ConfigureSpillover( routeInfo.spilloverBufferSide, false, attemptConnect ) )
                {
                    routeInfo.flowingIntoSpillover = false;
                }
                else
                {
                    // rely on timer cycle to try again, nothing we can do
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                        "):DetermineActiveRoute: Primary side health is restored and spillover absorbed messages. Failed to reconfigure spillover for egress" );
                    routeInfo.routeSwitchingTimer.SetEnabled( true );
                }
            }
        }
        else
        {
            routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
        }
    }
    else
    {
        // Primary 'to' route is not available
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
            "):DetermineActiveRoute: The Primary route is not available. Its health status last changed " + CORE::ToString( primarySideHealthDurationInMs ) +
            "ms ago" );

        if ( primarySideHealthDurationInMs > m_config.minBadHealthDurationBeforeSpilloverInMs )
        {
            CORE::UInt64 failoverSideHealthDurationInMs = (CORE::UInt64) routeInfo.failoverSideLastHealthStatusChange.GetTimeDifferenceInMillisecondsToNow();

            // Try the failover route if one is available
            if ( GUCEF_NULL != routeInfo.failoverSide &&
                 routeInfo.failoverSideIsHealthy &&
                 failoverSideHealthDurationInMs > m_config.minFailoverSideGoodHealthDurationBeforeActivationInMs )
            {
                if ( routeInfo.activeSide != routeInfo.failoverSide )
                {
                    // Before (re)activating the failover route check to make sure we didnt use the spillover
                    // if so need we need to egress those messages first before restoring the normal route
                    if ( !routeInfo.DidMsgsFlowIntoSpillover() )
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                            "):DetermineActiveRoute: Switching active route to the Failover side" );

                        routeInfo.activeSide = routeInfo.failoverSide;
                        routeInfo.activeSideType = RouteType::Failover;
                        routeInfo.routeSwitchingTimer.SetEnabled( false );
                        routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
                    }
                    else
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                            "):DetermineActiveRoute: Failover side health is restored and spillover absorbed messages. Commencing spillover egress" );

                        // Switch from ingress to egress
                        if ( ConfigureSpillover( routeInfo.spilloverBufferSide, false, attemptConnect ) )
                        {
                            routeInfo.flowingIntoSpillover = false;
                        }
                        else
                        {
                            // rely on timer cycle to try again, nothing we can do
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                                "):DetermineActiveRoute: Failover side health is restored and spillover absorbed messages. Failed to reconfigure spillover for egress" );
                            routeInfo.routeSwitchingTimer.SetEnabled( true );
                        }
                    }
                }
                else
                {
                    routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
                }
            }
            else
            {
                // It seems the primary and failover routes have failed us
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                    "):DetermineActiveRoute: The Primary and Failover sides for the route are not available. Failover health status last changed " + CORE::ToString( failoverSideHealthDurationInMs ) +
                    "ms ago" );

                // If there is a spillover available we can try using that route as a different type of failover
                if ( GUCEF_NULL != routeInfo.spilloverBufferSide &&
                     routeInfo.spilloverBufferSideIsHealthy &&
                     routeInfo.spilloverBufferSideLastHealthStatusChange.GetTimeDifferenceInMillisecondsToNow() > m_config.minSpilloverSideGoodHealthDurationBeforeActivationInMs )
                {
                    // At this point the spillover is healthy and the primary and failover are not
                    if ( primarySideHealthDurationInMs > m_config.minBadHealthDurationBeforeSpilloverInMs ||
                         failoverSideHealthDurationInMs > m_config.minBadHealthDurationBeforeSpilloverInMs )
                    {
                        // At this point we also know we waited the configured time before failing over to the spillover
                        // this is important because for a quick flickering connection its not worth the overhead to fail over to the spillover buffer which likely involves setup cost
                        // plus the ongoing headache of having to read back from the spillover once the primary or failover routes are restored.
                        // Thus if a simple retry saves the day between 'from' and 'to' you should favor that instead of the spillover mechanism
                        if ( routeInfo.activeSide != routeInfo.spilloverBufferSide )
                        {
                            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                                "):DetermineActiveRoute: Switching active route to the Spillover buffer side" );

                            routeInfo.activeSide = routeInfo.spilloverBufferSide;
                            routeInfo.activeSideType = RouteType::SpilloverBuffer;
                            routeInfo.routeSwitchingTimer.SetEnabled( false );
                            routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );

                            // If the spillover is configured for egress instead of ingress we need to either wait of egress to finish
                            // or if its finished we need to reconfigure for ingress
                            if ( !routeInfo.flowingIntoSpillover )
                            {
                                CSpilloverInfo& spilloverInfo = m_spilloverInfoMap[ routeInfo.spilloverBufferSide ];

                                // If the spillover is performing egress we need to let it complete first
                                if ( !spilloverInfo.IsEgressOngoing()  )
                                {
                                    // reconfigure the spillover for data ingress
                                    // this is an expensive operation
                                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                                        "):DetermineActiveRoute: Reconfiguring spillover side " + routeInfo.spilloverBufferSide->GetSideId() + " for data ingress" );

                                    if ( ConfigureSpillover( routeInfo.spilloverBufferSide, true, attemptConnect ) )
                                    {
                                        routeInfo.flowingIntoSpillover = true;
                                    }
                                    else
                                    {
                                        // Something went wrong, abort and try again next cycle
                                        routeInfo.activeSide = GUCEF_NULL;
                                        routeInfo.activeSideType = RouteType::Disabled;
                                        routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
                                        routeInfo.routeSwitchingTimer.SetEnabled( true );
                                    }
                                }
                            }
                        }
                        else
                        {
                            routeInfo.SwitchAllTopicLinksActiveTopic( routeInfo.activeSideType );
                        }
                    }
                }
            }
        }
    }
}
/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::DetermineActiveRoutes( bool attemptConnect )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );

    // check to prevent causing a recursive loop via an eventing chain
    if ( !m_busyDeterminingActiveRoutes )
    {
        m_busyDeterminingActiveRoutes = true;

        TSidePtrToRouteInfoVectorMap::iterator r = m_routeMap.begin();
        while ( r != m_routeMap.end() )
        {
            TRouteInfoVector& multiRouteInfo = (*r).second;
            TRouteInfoVector::iterator n = multiRouteInfo.begin();
            while ( n != multiRouteInfo.end() )
            {
                CRouteInfo& routeInfo = (*n);
                DetermineActiveRoute( routeInfo, attemptConnect );
                ++n;
            }

            ++r;
        }

        m_busyDeterminingActiveRoutes = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::DetermineActiveRoute( CRouteInfo& routeInfo, bool attemptConnect )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );

    // Check if this our first run at determining the active route
    if ( GUCEF_NULL == routeInfo.activeSide )
    {
        DetermineFirstActiveRoute( routeInfo, attemptConnect );
        return;
    }

    if ( routeInfo.IsSpilloverEgressOngoing() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
            "):DetermineActiveRoute: Not changing routes right now and deferring because spillover egress is ongoing" );
        routeInfo.routeSwitchingTimer.SetEnabled( true );
        return;
    }

    DetermineActiveRouteDestination( routeInfo, attemptConnect );

    if ( attemptConnect && GUCEF_NULL != routeInfo.activeSide )
    {
        if ( ConnectRoutesForFromSide( routeInfo.fromSide ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                "):DetermineActiveRoute: Successfully connected route" );

            routeInfo.routeSwitchingTimer.SetEnabled( false );
        }
        else
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                "):DetermineActiveRoute: failed to connect route, will retry" );

            routeInfo.routeSwitchingTimer.SetEnabled( true );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnRouteSwitchTimerCycle( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CTimer* routeSwitchTimer = static_cast< CORE::CTimer* >( notifier );
    if ( GUCEF_NULL != routeSwitchTimer )
    {
        CRouteInfo* routeInfo = static_cast< CRouteInfo* >( routeSwitchTimer->GetOpaqueUserData() );
        if ( GUCEF_NULL != routeInfo )
        {
            DetermineActiveRoute( *routeInfo, true );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::UpdateRoutesBasedOnSideHealthStatus( CPubSubClientSide* side ,
                                                        bool isHealthy          )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );

    TSidePtrToRouteInfoPtrVectorMap::iterator i = m_usedInRouteMap.find( side );
    if ( i != m_usedInRouteMap.end() )
    {
        TRouteInfoPtrVector& multiRouteInfo = (*i).second;

        CORE::UInt32 flagsUpdated = 0;
        TRouteInfoPtrVector::iterator n = multiRouteInfo.begin();
        while ( n != multiRouteInfo.end() )
        {
            CRouteInfo* routeInfo = (*n);

            if ( side == routeInfo->toSide )
            {
                if ( routeInfo->toSideIsHealthy != isHealthy )
                {
                    routeInfo->toSideIsHealthy = isHealthy;
                    routeInfo->toSideLastHealthStatusChange = CORE::CDateTime::NowUTCDateTime();
                    ++flagsUpdated;
                    routeInfo->routeSwitchingTimer.SetEnabled( true );
                }
            }
            if ( side == routeInfo->failoverSide )
            {
                if ( routeInfo->failoverSideIsHealthy != isHealthy )
                {
                    routeInfo->failoverSideIsHealthy = isHealthy;
                    routeInfo->failoverSideLastHealthStatusChange = CORE::CDateTime::NowUTCDateTime();
                    ++flagsUpdated;
                    routeInfo->routeSwitchingTimer.SetEnabled( true );
                }
            }
            if ( side == routeInfo->deadLetterSide )
            {
                if ( routeInfo->deadLetterSideIsHealthy != isHealthy )
                {
                    routeInfo->deadLetterSideIsHealthy = isHealthy;
                    routeInfo->deadLetterSideLastHealthStatusChange = CORE::CDateTime::NowUTCDateTime();
                    ++flagsUpdated;
                    routeInfo->routeSwitchingTimer.SetEnabled( true );
                }
            }
            if ( side == routeInfo->spilloverBufferSide )
            {
                if ( routeInfo->spilloverBufferSideIsHealthy != isHealthy )
                {
                    routeInfo->spilloverBufferSideIsHealthy = isHealthy;
                    routeInfo->spilloverBufferSideLastHealthStatusChange = CORE::CDateTime::NowUTCDateTime();
                    ++flagsUpdated;
                    routeInfo->routeSwitchingTimer.SetEnabled( true );
                }
            }

            DetermineActiveRoute( *routeInfo, true );

            ++n;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
            "):UpdateRoutesBasedOnSideHealthStatus: Updated " + CORE::ToString( multiRouteInfo.size() ) + " routes that have side \"" +
            side->GetSideId() + "\" for a total of " + CORE::ToString( flagsUpdated ) + " health status updates. Healthy=" + CORE::ToString( isHealthy ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSideHealthStatusChange( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClientSide* side = static_cast< CPubSubClientSide* >( notifier );
    CPubSubClientSide::THealthStatusChangeEventData* healthStatus = static_cast< CPubSubClientSide::THealthStatusChangeEventData* >( eventData );

    if ( GUCEF_NULL == side || GUCEF_NULL == healthStatus )
        return;

    UpdateRoutesBasedOnSideHealthStatus( side, *healthStatus );
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSidePubSubClientTopicEndOfData( CPubSubClientTopic* topic )
{GUCEF_TRACE;

    if ( GUCEF_NULL == topic )
        return;

    CPubSubClient* client = topic->GetClient();
    if ( GUCEF_NULL == client )
        return;

    CPubSubClientSide* side = static_cast< CPubSubClientSide* >( client->GetOpaqueUserData() );
    if ( GUCEF_NULL == side )
        return;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
        "):OnSidePubSubClientTopicEndOfData: Side \"" + side->GetSideId() + "\" has client of type \"" + client->GetType() +
        "\" which notified that topic \"" + topic->GetTopicName() + "\" has observed an 'end of data' event" );

    MT::CScopeReaderLock readLock( m_lock );

    // Check if this side is a spillover and in doing so get the spillover info
    TSidePtrToSpilloverInfoMap::iterator i = m_spilloverInfoMap.find( side );
    if ( i != m_spilloverInfoMap.end() )
    {
        // The end-of-data side is a spillover side

        // Now we will perform the next check since it might be worth it...
        // We really care about the aggregate status in the router, not per topic
        if ( client->AreAllSubscriptionsAtEndOfData() )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
                "):OnSidePubSubClientTopicEndOfData: Spillover Side \"" + side->GetSideId() + "\" has client of type \"" + client->GetType() +
                "\" is determined to have reached an 'end of data' event for all subscriptions" );

            CSpilloverInfo& spilloverInfo = (*i).second;
            spilloverInfo.endOfDataEventOccured = true;

            // reset the flag to indicate whether any msgs flowed into the spillover
            // this flag is used to reduce redundant spillover reconfiguration which takes time
            spilloverInfo.msgsFlowedIntoSpillover = false;

            // Flip the spillover to ingress
            spilloverInfo.route->flowingIntoSpillover = true;

            // transition our read lock to a write lock
            // we want to make sure the data references we obtained above do not become invalid
            MT::CScopeWriterLock writeLock( readLock );

            // if an entire client and thus a side (since its a 1:1 relationship) has reached EOD
            // we check to see if we need to update the active route
            DetermineActiveRoute( *spilloverInfo.route, true );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSidePubSubClientTopicEndOfData( CORE::CNotifier* notifier    ,
                                                     const CORE::CEvent& eventId  ,
                                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClientTopic* topic = static_cast< CPubSubClientTopic* >( notifier );
    if ( GUCEF_NULL == topic )
        return;
    OnSidePubSubClientTopicEndOfData( topic );
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSidePubSubClientInstantiation( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventId  ,
                                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClientPtr* pubsubClientPtr = static_cast< CPubSubClientPtr* >( eventData );
    if ( GUCEF_NULL == pubsubClientPtr )
        return;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
        "):OnSidePubSubClientInstantiation: Adding event handlers for new pubsub client of type " + (*pubsubClientPtr)->GetType() );

    RegisterSidePubSubClientEventHandlers( *pubsubClientPtr );
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::CreateNewFromTopicAssociationAsNeeded( CPubSubClientTopicBasicPtr fromTopicPtr, bool attemptConnect )
{GUCEF_TRACE;

    if ( fromTopicPtr.IsNULL() )
        return;
    const CORE::CString& fromTopicName = fromTopicPtr->GetTopicName();

    CPubSubClient* fromClient = fromTopicPtr->GetClient();
    if ( GUCEF_NULL == fromClient )
        return;

    CPubSubClientSide* fromSide = static_cast< CPubSubClientSide* >( fromClient->GetOpaqueUserData() );
    if ( GUCEF_NULL == fromSide )
        return;

    MT::CScopeWriterLock writeLock( m_lock );

    // look up the routes for which this side is the 'from' side
    TSidePtrToRouteInfoVectorMap::iterator r = m_routeMap.find( fromSide );
    if ( r == m_routeMap.end() )
        return;

    // If we are auto matching topic associations do so for new runtime discovered topics as well
    CPubSubFlowRouterConfig::PubSubFlowRouteConfigVector::iterator c = m_config.routes.begin();
    while ( c != m_config.routes.end() )
    {
        CPubSubFlowRouteConfigPtr routeConfig = (*c);
        CPubSubFlowRouteTopicConfigPtr topicRouteConfig = routeConfig->FindOrCreateTopicAssociation( fromTopicName, false );
        if ( !topicRouteConfig.IsNULL() )
        {
            // We either had an explicit topic mapping or we are auto-matching
            // either way make sure our raw pointers are up to date

            CORE::PulseGeneratorPtr fromPulseGenerator;
            if ( routeConfig->preferFromTopicThreadForDestination )
            {
                fromPulseGenerator = fromTopicPtr->GetPulseGenerator();
                if ( fromPulseGenerator.IsNULL() )
                    fromPulseGenerator = fromClient->GetDefaultTopicPulseGenerator();
            }
            CORE::PulseGeneratorPtr destPulseGenerator;
            if ( !fromPulseGenerator.IsNULL() && routeConfig->preferFromTopicThreadForDestination )
            {
                destPulseGenerator = fromPulseGenerator;
            }

            TRouteInfoVector& routes = (*r).second;

            // first match the all configs in a batch
            TRouteInfoVector::iterator i = routes.begin();
            while ( i != routes.end() )
            {
                CRouteInfo& routeInfo = (*i);
                if ( !routeInfo.MatchTopicRouteConfig( topicRouteConfig, fromPulseGenerator, destPulseGenerator ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubFlowRouter(" + CORE::ToString( this ) +
                        "):CreateNewFromTopicAssociationAsNeeded: Failed to match topic route config to runtime topics for new topic " + fromTopicName );
                }
                ++i;
            }

            // next determine the new active route if any after processing the change batch above
            if ( attemptConnect )
            {
                i = routes.begin();
                while ( i != routes.end() )
                {
                    CRouteInfo& routeInfo = (*i);
                    DetermineActiveRoute( routeInfo, attemptConnect );
                    ++i;
                }
            }
        }
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSidePubSubClientTopicCreation( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventId  ,
                                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClient* pubsubClient = static_cast< CPubSubClient* >( notifier );
    if ( GUCEF_NULL == pubsubClient || GUCEF_NULL == eventData )
        return;

    CORE::CString topicName = *static_cast< CPubSubClient::TopicAccessCreatedEventData* >( eventData );
    CPubSubClientTopicBasicPtr topicAccess = pubsubClient->GetTopicAccess( topicName );
    if ( !topicAccess.IsNULL() )
    {
        // create the topic association but dont connect yet
        CreateNewFromTopicAssociationAsNeeded( topicAccess, false );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::PointerToString( this ) +
            "):OnSidePubSubClientTopicCreation: Adding event handlers for new topic " + topicAccess->GetTopicName() );

        // hook up the event handlers that will listen to the new topic
        RegisterSidePubSubClientTopicEventHandlers( topicAccess );

        // now that everything is set up confirm the active route and connect as needed
        DetermineActiveRoutes( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubFlowRouter::OnSidePubSubClientTopicsAutoCreation( CORE::CNotifier* notifier    ,
                                                         const CORE::CEvent& eventId  ,
                                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClient* pubsubClient = static_cast< CPubSubClient* >( notifier );
    if ( GUCEF_NULL == pubsubClient || GUCEF_NULL == eventData )
        return;

    CPubSubClient::PubSubClientTopicSet& topics = *static_cast< CPubSubClient::TopicsAccessAutoCreatedEventData* >( eventData );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
        "):OnSidePubSubClientTopicsAutoCreation: Received notice of " + CORE::ToString( topics.size() ) +
        " new auto created topics");

    CPubSubClient::PubSubClientTopicSet::iterator i = topics.begin();
    while ( i != topics.end() )
    {
        // create the topic association but dont connect yet
        CreateNewFromTopicAssociationAsNeeded( (*i), false );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubFlowRouter(" + CORE::ToString( this ) +
            "):OnSidePubSubClientTopicsAutoCreation: Adding event handlers for new auto created topic " + (*i)->GetTopicName() );

        // hook up the event handlers that will listen to the new topic
        RegisterSidePubSubClientTopicEventHandlers( (*i) );

        ++i;
    }

    // now that everything is set up confirm the active route and connect as needed
    DetermineActiveRoutes( true );
}


/*-------------------------------------------------------------------------*/

const CString&
CPubSubFlowRouter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubFlowRouter";
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
