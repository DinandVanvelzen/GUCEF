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

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"       /* wrappers for threading related O/S functionality */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H
#include "gucefPUBSUB_CPubSubFlowRouter.h"
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H
#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H ? */

#include "gucefPUBSUB_CPubSubClientSide.h"

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

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

const CORE::CEvent CPubSubClientSide::HealthStatusChangeEvent = "GUCEF::PUBSUB::CPubSubClientSide::HealthStatusChangeEvent";
const CORE::CEvent CPubSubClientSide::PubSubClientInstantiationEvent = "GUCEF::PUBSUB::CPubSubClientSide::PubSubClientInstantiationEvent";

#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000
#define GUCEF_DEFAULT_PUBSUB_RECONNECT_DELAY_IN_MS                  100
#define GUCEF_DEFAULT_PUBSUB_MAX_PUBLISHED_MSG_INFLIGHT_TIME_IN_MS  ( 30 * 1000 )
#define GUCEF_DEFAULT_PUBSUB_SIDE_MAX_IN_FLIGHT                     1000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPubSubClientSide::RegisterEvents( void )
{GUCEF_TRACE;

    HealthStatusChangeEvent.Initialize();
    PubSubClientInstantiationEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::CPubSubClientSide( const CORE::CString& sideId   ,
                                      CPubSubFlowRouter* flowRouter )
    : CORE::CTaskConsumer()
    , m_pubsubClient()
    , m_clientFeatures()
    , m_pubsubBookmarkPersistence()
    , m_bookmarkNamespace()
    , m_topicPtrs()
    , m_topics()
    , m_sideSettings()
    , m_broadcastMailbox()
    , m_metricsTimer( CORE::PulseGeneratorPtr(), 1000 )
    , m_pubsubClientReconnectTimer( CORE::PulseGeneratorPtr(), 1000 )
    , m_sideId( sideId )
    , m_threadIdOfSide( 0 )
    , m_flowRouter( flowRouter )
    , m_isHealthy( true )
    , m_connectOnTaskStart( true )
    , m_rwdataLock( true )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::~CPubSubClientSide()
{GUCEF_TRACE;

    DisconnectPubSubClient( true );
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

CPubSubClientPtr
CPubSubClientSide::GetCurrentUnderlyingPubSubClient( void )
{GUCEF_TRACE;

    return m_pubsubClient;
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CPubSubClientSide::GetCurrentUnderlyingPubSubClientTopicByName( const CORE::CString& topicName ) const
{GUCEF_TRACE;

    CPubSubClientPtr client = m_pubsubClient;
    if ( !client.IsNULL() )
    {
        return client->GetTopicAccess( topicName );
    }
    return CPubSubClientTopicBasicPtr();
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::TopicLink( void )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, false )
    , CORE::CTSharedObjCreator< TopicLink, MT::CMutex >( this )
    , topic()
    , currentPublishActionIds()
    , inFlightMsgs()
    , publishFailedMsgs()
    , publishAckdMsgsMailbox()
    , maxASyncPublishMailboxSize( 1000 )
    , maxASyncPublishMailboxSizeDuringAFR( 1000 )
    , metricFriendlyTopicName()
    , metricsPrefix()
    , metrics( CPubSubClientSideMetrics::CreateSharedObj() )
    , lastBookmarkPersistSuccess( CORE::CDateTime::PastMax )
    , msgsSinceLastBookmarkPersist( 0 )
    , bookmarksOnMsgReceived()
    , msgMailbox()
    , flowRouter( GUCEF_NULL )
    , side( GUCEF_NULL )
    , clientFeatures()
    , needToTrackInFlightPublishedMsgsForAck( true ) // safer default even through its less performant
    , retryFailedPublishAttempts( true )             // safer default even through its less performant
    , pubsubBookmarkPersistence()
    , awaitingFailureReport( false )
    , totalMsgsInFlight( 0 )
    , bookmarkNamespace()
    , threadIdOfTopicLink( 0 )
    , timedOutInFlightMessagesCheckTimer( CORE::PulseGeneratorPtr(), 5000 )
    , metricsTimer( CORE::PulseGeneratorPtr(), 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::TopicLink( CPubSubClientTopicBasicPtr t )
    : CORE::CTSGNotifier( CORE::PulseGeneratorPtr(), true, false )
    , CORE::CTSharedObjCreator< TopicLink, MT::CMutex >( this )
    , topic( t )
    , currentPublishActionIds()
    , inFlightMsgs()
    , publishFailedMsgs()
    , publishAckdMsgsMailbox()
    , maxASyncPublishMailboxSize( 1000 )
    , maxASyncPublishMailboxSizeDuringAFR( 1000 )
    , metricFriendlyTopicName()
    , metricsPrefix()
    , metrics( CPubSubClientSideMetrics::CreateSharedObj() )
    , lastBookmarkPersistSuccess( CORE::CDateTime::PastMax )
    , msgsSinceLastBookmarkPersist( 0 )
    , bookmarksOnMsgReceived()
    , msgMailbox()
    , flowRouter( GUCEF_NULL )
    , side( GUCEF_NULL )
    , clientFeatures()
    , needToTrackInFlightPublishedMsgsForAck( true ) // safer default even through its less performant
    , retryFailedPublishAttempts( true )             // safer default even through its less performant
    , pubsubBookmarkPersistence()
    , awaitingFailureReport( false )
    , totalMsgsInFlight( 0 )
    , bookmarkNamespace()
    , threadIdOfTopicLink( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::~TopicLink()
{GUCEF_TRACE;

    Clear();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

const CString&
CPubSubClientSide::TopicLink::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::PUBSUB::CPubSubClientSide::TopicLink";
    return typeName;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetNeedToTrackInFlightPublishedMsgsForAck( bool isNeeded )
{GUCEF_TRACE;

    needToTrackInFlightPublishedMsgsForAck = isNeeded;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::AddInFlightMsg( CORE::UInt64 publishActionId       ,
                                              CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    inFlightMsgs[ publishActionId ] = MsgTrackingEntry( publishActionId, msg );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::AddInFlightMsgs( const CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                                               const CPubSubClientTopic::TIPubSubMsgSPtrVector& msgs              ,
                                               bool inFlightDefaultState                                          )
{GUCEF_TRACE;

    // this variation gets called during async flow
    size_t max = SMALLEST( publishActionIds.size(), msgs.size() );
    if ( publishActionIds.size() != msgs.size() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubClientSide(" + CORE::ToString( this ) +
            "):TopicLink:AddInFlightMsgs: Nr of publishActionIds (" + CORE::ToString( publishActionIds.size() ) +
            ") does not match Nr of msgs (" + CORE::ToString( msgs.size() ) + "). Will proceed best effort but this will likely cause issues" );
    }

    for ( size_t i=0; i<max; ++i )
    {
        CIPubSubMsg::TNoLockSharedPtr msgPtr( msgs[ i ] );
        inFlightMsgs[ publishActionIds[ i ] ] = MsgTrackingEntry( publishActionIds[ i ], msgPtr, inFlightDefaultState );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::AddInFlightMsgs( const CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                                               const CPubSubClientTopic::TPubSubMsgsRefVector& msgs               ,
                                               bool inFlightDefaultState                                          )
{GUCEF_TRACE;

    // this variation gets called during sync flow
    size_t max = SMALLEST( publishActionIds.size(), msgs.size() );
    if ( publishActionIds.size() != msgs.size() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubClientSide(" + CORE::ToString( this ) +
            "):TopicLink:AddInFlightMsgs: Nr of publishActionIds (" + CORE::ToString( publishActionIds.size() ) +
            ") does not match Nr of msgs (" + CORE::ToString( msgs.size() ) + "). Will proceed best effort but this will likely cause issues" );
    }

    for ( size_t i=0; i<max; ++i )
    {
        // in the sync/blocking flow we have not yet made a copy, lifecycle is for the call chain which doesnt work
        // for keeping longer term references. As such we create a copy of the message here
        CIPubSubMsg::TNoLockSharedPtr msgPtr( static_cast< CIPubSubMsg* >( msgs[ i ]->Clone() ) );
        inFlightMsgs[ publishActionIds[ i ] ] = MsgTrackingEntry( publishActionIds[ i ], msgPtr, inFlightDefaultState );
    }
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::MsgTrackingEntry::MsgTrackingEntry( void )
    : retryCount( 0 )
    , firstPublishAttempt( CORE::CDateTime::Empty )
    , lastPublishAttempt( CORE::CDateTime::Empty )
    , publishActionId( 0 )
    , msg()
    , isInFlight( false )
    , waitingForInFlightConfirmation( false )
    , readyToAckPublishSuccessButAckFailed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::MsgTrackingEntry::MsgTrackingEntry( CORE::UInt64 publishActionID       ,
                                                                  CIPubSubMsg::TNoLockSharedPtr& msg ,
                                                                  bool isInFlightState               )
    : retryCount( 0 )
    , firstPublishAttempt( CORE::CDateTime::NowUTCDateTime() )
    , lastPublishAttempt( CORE::CDateTime::Empty )
    , publishActionId( publishActionID )
    , msg( msg )
    , isInFlight( isInFlightState )
    , waitingForInFlightConfirmation( true )
    , readyToAckPublishSuccessButAckFailed( false )
{GUCEF_TRACE;

    lastPublishAttempt = firstPublishAttempt;
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::MsgTrackingEntry::MsgTrackingEntry( const MsgTrackingEntry& src )
    : retryCount( src.retryCount )
    , firstPublishAttempt( src.firstPublishAttempt )
    , lastPublishAttempt( src.lastPublishAttempt )
    , publishActionId( src.publishActionId )
    , msg( src.msg )
    , isInFlight( src.isInFlight )
    , waitingForInFlightConfirmation( src.waitingForInFlightConfirmation )
    , readyToAckPublishSuccessButAckFailed( src.readyToAckPublishSuccessButAckFailed )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::TopicLink::MsgTrackingEntry&
CPubSubClientSide::TopicLink::MsgTrackingEntry::operator=( const MsgTrackingEntry& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        retryCount = src.retryCount;
        firstPublishAttempt = src.firstPublishAttempt;
        lastPublishAttempt = src.lastPublishAttempt;
        publishActionId = src.publishActionId;
        msg = src.msg;
        isInFlight = src.isInFlight;
        waitingForInFlightConfirmation = src.waitingForInFlightConfirmation;
        readyToAckPublishSuccessButAckFailed = src.readyToAckPublishSuccessButAckFailed;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::RegisterPubSubClientEventHandlers( CPubSubClientPtr& pubsubClient )
{GUCEF_TRACE;

    TEventCallback callback( this, &CPubSubClientSide::OnTopicsAccessAutoCreated );
    SubscribeTo( pubsubClient.GetPointerAlways()             ,
                 CPubSubClient::TopicsAccessAutoCreatedEvent ,
                 callback                                    );

    TEventCallback callback2( this, &CPubSubClientSide::OnTopicsAccessAutoDestroyed );
    SubscribeTo( pubsubClient.GetPointerAlways()               ,
                 CPubSubClient::TopicsAccessAutoDestroyedEvent ,
                 callback2                                     );

    TEventCallback callback3( this, &CPubSubClientSide::OnClientHealthStatusChanged );
    SubscribeTo( pubsubClient.GetPointerAlways()        ,
                 CPubSubClient::HealthStatusChangeEvent ,
                 callback3                              );

    TEventCallback callback4( this, &CPubSubClientSide::OnTopicAccessCreated );
    SubscribeTo( pubsubClient.GetPointerAlways()        ,
                 CPubSubClient::TopicAccessCreatedEvent ,
                 callback4                              );

    TEventCallback callback5( this, &CPubSubClientSide::OnTopicAccessDestroyed );
    SubscribeTo( pubsubClient.GetPointerAlways()          ,
                 CPubSubClient::TopicAccessDestroyedEvent ,
                 callback5                                );

    // register event handlers for any topics the client may already have
    RegisterTopicEventHandlers( pubsubClient );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CPubSubClientSide::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );

    TEventCallback callback3( this, &CPubSubClientSide::OnPubSubClientReconnectTimerCycle );
    SubscribeTo( &m_pubsubClientReconnectTimer  ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback3                      );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::RegisterTopicEventHandlers( CPubSubClientPtr& pubsubClient )
{GUCEF_TRACE;

    CPubSubClient::PubSubClientTopicSet allTopicAccess;
    pubsubClient->GetAllCreatedTopicAccess( allTopicAccess );

    CPubSubClient::PubSubClientTopicSet::iterator i = allTopicAccess.begin();
    while ( i != allTopicAccess.end() )
    {
        CPubSubClientTopicBasicPtr topic = (*i);
        if ( !topic.IsNULL() )
        {
            // Configuring the topic link takes care of the event handlers for the topic
            // this gets a bit complicated since the threading model and lookup tables etc all potentially need to be updated as well
            ConfigureTopicLink( m_sideSettings, topic, false );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientSide::GetType( void ) const
{GUCEF_TRACE;

    return "PubSubClientSide";
}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::CPubSubClientSideMetrics::CPubSubClientSideMetrics( void )
    : CORE::CTSharedObjCreator< CPubSubClientSideMetrics, MT::CMutex >( this )
    , publishedMsgsInFlight( 0 )
    , publishOrAckFailedMsgs( 0 )
    , lastPublishBatchSize( 0 )
    , queuedReceiveSuccessAcks( 0 )
    , hasSupportForPublishing( false )
    , hasSupportForSubscribing( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::CPubSubClientSideMetrics::CPubSubClientSideMetrics( const CPubSubClientSideMetrics& src )
    : CORE::CTSharedObjCreator< CPubSubClientSideMetrics, MT::CMutex >( this )
    , publishedMsgsInFlight( src.publishedMsgsInFlight )
    , publishOrAckFailedMsgs( src.publishOrAckFailedMsgs )
    , lastPublishBatchSize( src.lastPublishBatchSize )
    , queuedReceiveSuccessAcks( src.queuedReceiveSuccessAcks )
    , hasSupportForPublishing( src.hasSupportForPublishing )
    , hasSupportForSubscribing( src.hasSupportForSubscribing )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientSide::CPubSubClientSideMetrics::~CPubSubClientSideMetrics()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );


}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::GetPubSubClientSupportedFeatures( CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    try
    {
        if ( !m_pubsubClient.IsNULL() && m_pubsubClient->GetSupportedFeatures( features ) )
            return true;
    }
    catch ( const std::exception& )
    {

    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::IsHealthy( void ) const
{GUCEF_TRACE;

    // @TODO: have some read-only version
    MT::CScopeWriterLock lock( m_rwdataLock );

    // Aggregate the health status across all concerns
    bool fullyHealthy = true;

    try
    {
        if ( !m_pubsubClient.IsNULL() && !m_pubsubClient->IsHealthy() )
            fullyHealthy = false;
    }
    catch ( const std::exception& )
    {
        fullyHealthy = false;
    }

    try
    {
        if ( !m_pubsubBookmarkPersistence.IsNULL() && !m_pubsubBookmarkPersistence->IsHealthy() )
            fullyHealthy = false;
    }
    catch ( const std::exception& )
    {
        fullyHealthy = false;
    }

    // Notify if there was a change in status
    if ( fullyHealthy != m_isHealthy )
    {
        m_isHealthy = fullyHealthy;

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide:IsHealthy: overall health is now Ok for side with id " + m_sideId );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide:IsHealthy: overall health status is now unhealthy for side with id " + m_sideId );
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( fullyHealthy );
        NotifyObservers( HealthStatusChangeEvent, &eData );
    }

    return fullyHealthy;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnClientHealthStatusChanged( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the overall health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventId  ,
                                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateTopicMetrics();

    // We update topic metric numbers both on a timer cycle AND at various points
    // in the processing pipeline. The reasoning is that processing under certain load profiles
    // can cause the totality of thread cycle operations to exceed the timer cycle time
    // this would unintentionally reduce the number of metric updates obtained

    if ( metrics->hasSupportForPublishing )
    {
        GUCEF_METRIC_GAUGE( metricFriendlyTopicName + "publishedMsgsInFlight", metrics->publishedMsgsInFlight, 1.0f );
        GUCEF_METRIC_GAUGE( metricFriendlyTopicName + "publishOrAckFailedMsgs", metrics->publishOrAckFailedMsgs, 1.0f );
        GUCEF_METRIC_GAUGE( metricFriendlyTopicName + "lastPublishBatchSize", metrics->lastPublishBatchSize, 1.0f );
    }
    if ( metrics->hasSupportForSubscribing )
    {
        GUCEF_METRIC_GAUGE( metricFriendlyTopicName + "queuedReceiveSuccessAcks", metrics->queuedReceiveSuccessAcks, 1.0f );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnPulseCycle( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ProcessAcknowledgeReceiptsMailbox();

    bool retrySuccess = RetryPublishFailedMsgs();
    if ( retrySuccess || side->GetSideSettings()->allowOutOfOrderPublishRetry )
    {
        PublishMailboxMsgs();
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTopicAccessCreated( CORE::CNotifier* notifier    ,
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
        MT::CScopeWriterLock lock( m_rwdataLock );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):OnTopicAccessCreated: Configuring topic link for new topic " + topicAccess->GetTopicName() );

        // Configure the link to the topic if needed
        // leave pre-existing topics alone
        if ( ConfigureTopicLink( m_sideSettings, topicAccess, false ) )
        {
            // Connect the newly configured topic if this is part of an active flow/route
            // Similarly if this is part of an inactive route we should defer connecting
            if ( GUCEF_NULL == m_flowRouter || m_flowRouter->ShouldSideBeConnected( this ) )
            {
                ConnectPubSubClientTopic( *topicAccess, m_clientFeatures, m_sideSettings, false );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnTopicAccessCreated: Skipping connecting topic at this time: " + topicAccess->GetTopicName() );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):OnTopicAccessCreated: Failed configuring topic link for new topic " + topicAccess->GetTopicName() );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTopicAccessDestroyed( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClient* pubsubClient = static_cast< CPubSubClient* >( notifier );
    if ( GUCEF_NULL == pubsubClient || GUCEF_NULL == eventData )
        return;

    if ( pubsubClient != m_pubsubClient )
    {
        // the pointer is suspect, this should not happen
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):OnTopicAccessDestroyed: client pointer doesnt match. Event=" + CORE::ToString( pubsubClient ) + " Ours=" + CORE::ToString( m_pubsubClient.GetPointerAlways() ) );
        return;
    }

    CORE::CString topicName = *static_cast< CPubSubClient::TopicAccessCreatedEventData* >( eventData );
    CPubSubClientTopicBasicPtr topicAccess = pubsubClient->GetTopicAccess( topicName );
    if ( !topicAccess.IsNULL() )
    {
        MT::CScopeWriterLock lock( m_rwdataLock );

        TopicPtrMap::iterator i = m_topicPtrs.find( topicAccess.GetPointerAlways() );
        if ( i != m_topicPtrs.end() )
        {
            TopicLinkPtr topicLink = (*i).second;

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):OnTopicAccessDestroyed: Removing topic link info for destroyed topic " + topicAccess->GetTopicName() +
                ". There are " + CORE::ToString( topicLink->GetTotalMsgsInFlight() ) + " messages left in-flight" );

            // @TODO: What to do about in-flight messages, mailbox, etc? Any special action?
            //      send them to dead letter ?

        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):OnTopicAccessDestroyed: Cannot find topic link info for destroyed topic " + topicAccess->GetTopicName() );
        }

        m_topicPtrs.erase( topicAccess.GetPointerAlways() );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTopicsAccessAutoCreated( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClient::TopicsAccessAutoCreatedEventData* eData = static_cast< CPubSubClient::TopicsAccessAutoCreatedEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        CPubSubClient::PubSubClientTopicSet& topicsAccess = *eData;
        CPubSubClient::PubSubClientTopicSet::iterator i = topicsAccess.begin();
        while ( i != topicsAccess.end() )
        {
            CPubSubClientTopicBasicPtr topicAccess = (*i);
            if ( !topicAccess.IsNULL() )
            {
                MT::CScopeWriterLock lock( m_rwdataLock );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnTopicsAccessAutoCreated: Configuring topic link for new auto topic " + topicAccess->GetTopicName() );

                // Configure the link to the topic if needed
                // leave pre-existing topics alone
                if ( ConfigureTopicLink( m_sideSettings, topicAccess, false ) )
                {
                    // Connect the newly configured topic if this is part of an active flow/route
                    // Similarly if this is part of an inactive route we should defer connecting
                    if ( GUCEF_NULL == m_flowRouter || m_flowRouter->ShouldSideBeConnected( this ) )
                    {
                        ConnectPubSubClientTopic( *topicAccess, m_clientFeatures, m_sideSettings, false );
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                            "):OnTopicsAccessAutoCreated: Skipping connecting topic at this time: " + topicAccess->GetTopicName() );
                    }
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):OnTopicsAccessAutoCreated: Failed configuring topic link for new auto topic " + topicAccess->GetTopicName() );
                }
            }
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTopicsAccessAutoDestroyed( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClient::TopicsAccessAutoDestroyedEventData* eData = static_cast< CPubSubClient::TopicsAccessAutoDestroyedEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        CPubSubClient::PubSubClientTopicSet& topicsAccessSet = *eData;
        CPubSubClient::PubSubClientTopicSet::iterator i = topicsAccessSet.begin();
        while ( i != topicsAccessSet.end() )
        {
            CPubSubClientTopicBasicPtr topicAccess = (*i);
            if ( !topicAccess.IsNULL() )
            {
                MT::CScopeWriterLock lock( m_rwdataLock );

                TopicPtrMap::iterator i = m_topicPtrs.find( topicAccess.GetPointerAlways() );
                if ( i != m_topicPtrs.end() )
                {
                    TopicLinkPtr topicLink = (*i).second;

                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):OnTopicsAccessAutoDestroyed: Removing topic link info for destroyed topic " + topicAccess->GetTopicName() +
                        ". There are " + CORE::ToString( topicLink->GetTotalMsgsInFlight() ) + " messages left in-flight" );

                    // @TODO: What to do about in-flight messages, mailbox, etc? Any special action?
                    //      send them to dead letter ?

                    m_topicPtrs.erase( topicAccess.GetPointerAlways() );

                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):OnTopicAccessDestroyed: Cannot find topic link info for destroyed topic " + topicAccess->GetTopicName() );
                }
            }
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnPubSubClientReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventId  ,
                                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // stop the timer, reconnect time itself should not count towards the reconnect interval
    m_pubsubClientReconnectTimer.SetEnabled( false );

    if ( IsPubSubClientInfraReadyToConnect() )
    {
        if ( m_clientFeatures.supportsAutoReconnect )
        {
            if ( ConnectPubSubClient( true ) )
                return; // no need to resume the timer
        }
        else
        {
            // Since the client does not support reconnects we will destructively reconnnect
            // Meaning we wipe out any previous client as we cannot rely on the client implementation
            // properly handling the state transition
            if ( DisconnectPubSubClient( true ) )
            {
                if ( ConnectPubSubClient( true ) )
                    return; // no need to resume the timer
            }
        }
    }

    // no joy, start the timer again
    m_pubsubClientReconnectTimer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::HasSubscribersNeedingAcks( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    // better safe then sorry...
    if ( m_pubsubClient.IsNULL() )
        return true;

    const CPubSubClientConfig& pubSubConfig = m_sideSettings->pubsubClientConfig;

    // Whether we need to track successfull message handoff (garanteed handling) depends both on whether we want that extra reliability per the config
    // (optional since nothing is free and this likely degrades performance a bit) but also whether the backend even supports it.
    // If the backend doesnt support it all we will be able to do between the sides is fire-and-forget

    bool doWeWantIt = ( pubSubConfig.desiredFeatures.supportsSubscribing &&                         // <- does it apply in this context ?
                        ( pubSubConfig.desiredFeatures.supportsSubscriberMsgReceivedAck ||          // <- do we want it?
                          pubSubConfig.desiredFeatures.supportsSubscribingUsingBookmark  )
                      );

    bool isItSupported = m_clientFeatures.supportsSubscriberMsgReceivedAck ||
                         ( m_clientFeatures.supportsBookmarkingConcept && m_clientFeatures.supportsSubscribingUsingBookmark );

    bool canWeNotWantIt = m_clientFeatures.supportsAbsentMsgReceivedAck &&          // <- Is it even an option to not do it regardless of desired features
                          ( !m_clientFeatures.supportsBookmarkingConcept ||         // <- if we need to perform client-side bookmarking then its not really an option to forgo acks if you want a reliable handoff and thus bookmark progression
                             m_clientFeatures.supportsBookmarkingConcept && m_clientFeatures.supportsSubscribingUsingBookmark && m_clientFeatures.supportsServerSideBookmarkPersistance );

    bool acksNeeded =  ( doWeWantIt && isItSupported ) ||
                       ( !canWeNotWantIt && isItSupported );

    return acksNeeded;
}

/*-------------------------------------------------------------------------*/

template < typename TMsgCollection >
bool
CPubSubClientSide::BroadcastPublishMsgsSync( const TMsgCollection& msgs )
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwdataLock );

    CORE::UInt32 topicsToPublishOn = 0;
    CORE::UInt32 topicsPublishedOn = 0;
    bool totalSuccess = true;
    TopicPtrMap::iterator i = m_topicPtrs.begin();
    while ( i != m_topicPtrs.end() )
    {
        TopicLinkPtr topicLink = (*i).second;
        if ( !topicLink.IsNULL() )
        {
            CPubSubClientTopicBasicPtr topic = topicLink->GetTopic();
            if ( !topic.IsNULL() )
            {
                if ( topic->IsPublishingSupported() )
                {
                    ++topicsToPublishOn;

                    bool publishSuccess = topicLink->PublishMsgs( msgs );
                    totalSuccess = publishSuccess && totalSuccess;

                    if ( publishSuccess )
                        ++topicsPublishedOn;
                }
            }
        }
        ++i;
    }

    if ( !totalSuccess )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):BroadcastPublishMsgsSync: Failures encountered broadcasting messages to " + CORE::ToString( topicsToPublishOn ) +
            " successfully broadcast to " + CORE::ToString( topicsPublishedOn ) + " topics" );
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):BroadcastPublishMsgsSync: Successfully broadcasted messages to " + CORE::ToString( topicsPublishedOn ) + " topics, out of " +
            CORE::ToString( topicsToPublishOn ) + " topics available for publishing" );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

template < typename TMsgCollection >
bool
CPubSubClientSide::PublishMsgsSync( const TMsgCollection& msgs              ,
                                    CPubSubClientTopic* specificTargetTopic )
{GUCEF_TRACE;

    try
    {
        MT::CScopeReaderLock readerLock( m_rwdataLock );

        if ( GUCEF_NULL == specificTargetTopic )
        {
            if ( m_sideSettings->treatPublishWithoutTargetTopicAsBroadcast )
            {
                // No target topic is specific, treat as broadcast to all topics
                return BroadcastPublishMsgsSync( msgs );
            }
            return false;
        }

        TopicPtrMap::iterator i = m_topicPtrs.find( specificTargetTopic );
        if ( i != m_topicPtrs.end() )
        {
            TopicLinkPtr topicLink = (*i).second;
            return topicLink->PublishMsgsSync( msgs );
        }
        else
        {
            // it is possible the underlying implementation was swapped out
            CPubSubClientTopicBasicPtr newTopicObj = GetCurrentUnderlyingPubSubClientTopicByName( specificTargetTopic->GetTopicName() );
            if ( !newTopicObj.IsNULL() )
            {
                if ( newTopicObj == specificTargetTopic )
                {
                    // This is a publish on a topic for which we have not received the topic creation event notification yet
                    // in multi-threaded setups the order in which the events arrive vs other event listeners on the same can vary depending
                    // depending on thread timings
                    ConfigureTopicLink( m_sideSettings, newTopicObj, true );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PublishMsgsASync: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                        " was not registered yet, will do so now to instance " + CORE::ToString( newTopicObj.GetPointerAlways() ) +
                        " for topic " + newTopicObj->GetTopicName() );
                }
                else
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PublishMsgsSync: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                        " seems to have been replaced by a new instance " + CORE::ToString( newTopicObj.GetPointerAlways() ) +
                        " for topic " + newTopicObj->GetTopicName() );
                }
                return PublishMsgsSync( msgs, newTopicObj.GetPointerAlways() );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):PublishMsgsSync: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                    " is not a known topic nor does a topic by that name exist: " + specificTargetTopic->GetTopicName() );
                return false;
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):PublishMsgsSync: exception occured: " + CORE::ToString( e.what() ) );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

template < typename TMsgCollection >
bool
CPubSubClientSide::TopicLink::PublishMsgsSync( const TMsgCollection& msgs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    CORE::CObserverNotificationHold hold( this );

    if ( GUCEF_NULL == side )
        return false;
    const CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();

    if ( topic->IsPublishingSupported() )
    {
        currentPublishActionIds.clear();
        if ( topic->Publish( currentPublishActionIds, msgs, needToTrackInFlightPublishedMsgsForAck ) )
        {
            totalMsgsInFlight += msgs.size();

            if ( needToTrackInFlightPublishedMsgsForAck )
                AddInFlightMsgs( currentPublishActionIds, msgs, true );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):PublishMsgsSync: Successfully published " + CORE::ToString( msgs.size() ) + " messages to " + topic->GetTopicName() );

            UpdateTopicMetrics();
            return true;
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):PublishMsgsSync: Failed to publish messages to topic " + topic->GetTopicName() );

            if ( sideSettings->retryFailedPublishAttempts )
            {
                AddInFlightMsgs( currentPublishActionIds, msgs, false );
                awaitingFailureReport = true;
            }
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
            "):PublishMsgsSync: topic does not support publishing" );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::ApplySettings( const CPubSubSideChannelSettingsPtr sideSettings )
{GUCEF_TRACE;

    bool totalSuccess = true;
    MT::CScopeMutex lock( m_dataLock );

    metricsPrefix = sideSettings->metricsPrefix;
    maxASyncPublishMailboxSize = sideSettings->maxASyncPublishMailboxSize;
    maxASyncPublishMailboxSizeDuringAFR = sideSettings->maxASyncPublishMailboxSizeDuringAFR;
    needToTrackInFlightPublishedMsgsForAck = sideSettings->needToTrackInFlightPublishedMsgsForAck;
    retryFailedPublishAttempts = sideSettings->retryFailedPublishAttempts;

    if ( sideSettings->maxPublishedMsgInFlightTimeInMs > 0 )
    {
        timedOutInFlightMessagesCheckTimer.SetInterval( (CORE::UInt32) sideSettings->maxPublishedMsgInFlightTimeInMs );
        totalSuccess = timedOutInFlightMessagesCheckTimer.SetEnabled( sideSettings->maxPublishedMsgInFlightTimeInMs > 0 ) && totalSuccess;
    }

    if ( sideSettings->collectMetrics )
    {
        metricsTimer.SetInterval( sideSettings->metricsIntervalInMs );
        totalSuccess = metricsTimer.SetEnabled( sideSettings->collectMetrics ) && totalSuccess;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetTopic( CPubSubClientTopicBasicPtr newTopic )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    UnsubscribeFrom( topic.GetPointerAlways() );
    topic = newTopic;

    if ( !topic.IsNULL() )
    {
        RegisterTopicEventHandlers( topic );

        metrics->hasSupportForPublishing = topic->IsPublishingSupported();
        metrics->hasSupportForSubscribing = topic->IsSubscribingSupported();

        metricFriendlyTopicName = metricsPrefix + "topic." + CPubSubClientSide::GenerateMetricsFriendlyTopicName( topic->GetTopicName() ) + ".";
    }
}

/*-------------------------------------------------------------------------*/

CPubSubClientTopicBasicPtr
CPubSubClientSide::TopicLink::GetTopic( void ) const
{GUCEF_TRACE;

    return topic;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetParentSide( CPubSubClientSide* parentSide )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    side = parentSide;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetFlowRouter( CPubSubFlowRouter* router )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    flowRouter = router;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    UnsubscribeFrom( GetPulseGenerator().GetPointerAlways() );

    metricsTimer.SetPulseGenerator( newPulseGenerator );
    timedOutInFlightMessagesCheckTimer.SetPulseGenerator( newPulseGenerator );
    threadIdOfTopicLink = newPulseGenerator->GetPulseDriverThreadId();

    TEventCallback callback( this, &CPubSubClientSide::TopicLink::OnPulseCycle );
    SubscribeTo( newPulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent    ,
                 callback                             );

    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );

    if ( !topic.IsNULL() )
        topic->SetPulseGenerator( newPulseGenerator );
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CPubSubClientSide::TopicLink::GetTotalMsgsInFlight( void ) const
{GUCEF_TRACE;

    return totalMsgsInFlight;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientSide::TopicLink::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return metricFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetPubsubBookmarkPersistence( TIPubSubBookmarkPersistenceBasicPtr persistance )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock ); // block other use so we dont leave thing half-finished
    pubsubBookmarkPersistence = persistance;
}

/*-------------------------------------------------------------------------*/

TIPubSubBookmarkPersistenceBasicPtr
CPubSubClientSide::TopicLink::GetPubsubBookmarkPersistence( void ) const
{GUCEF_TRACE;

    return pubsubBookmarkPersistence;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetPubsubBookmarkNamespace( const CString& newNamespace )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    bookmarkNamespace = newNamespace;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetClientFeatures( const CPubSubClientFeatures& newFeatures )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    clientFeatures = newFeatures;
}

/*-------------------------------------------------------------------------*/

template < typename TMsgCollection >
bool
CPubSubClientSide::TopicLink::PublishMsgsASync( const TMsgCollection& msgs )
{GUCEF_TRACE;

    if ( awaitingFailureReport )
    {
        if ( maxASyncPublishMailboxSizeDuringAFR < 0                                  ||
             msgMailbox.AmountOfMail() < (UInt32) maxASyncPublishMailboxSizeDuringAFR )
        {
            return msgMailbox.AddBulkMail< const TMsgCollection >( msgs );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):PublishMsgsASync: max mailbox items reached while waiting for failure report. wont add more" );
            return false;
        }
    }
    else
    {
        if ( maxASyncPublishMailboxSize < 0                                  ||
             msgMailbox.AmountOfMail() < (UInt32) maxASyncPublishMailboxSize )
        {
            return msgMailbox.AddBulkMail< const TMsgCollection >( msgs );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):PublishMsgsASync: max mailbox items reached. wont add more" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::PublishMsgs( const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                                CPubSubClientTopic* specificTargetTopic              )
{GUCEF_TRACE;

    if ( GUCEF_NULL == specificTargetTopic )
    {
        // Without a specific target topic this can only be valid if its a broadcast attempt
        // the mailbox has its own lock, no need for extra locking
        if ( m_sideSettings->treatPublishWithoutTargetTopicAsBroadcast )
        {
            if ( m_sideSettings->maxASyncPublishMailboxSize < 0                                          ||
                 m_broadcastMailbox.AmountOfMail() < (UInt32) m_sideSettings->maxASyncPublishMailboxSize )
            {
                return m_broadcastMailbox.AddBulkMail< const CPubSubClientTopic::TPubSubMsgsRefVector >( msgs );
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):PublishMsgs: max broadcast mailbox items reached. wont add more" );
                return false;
            }
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):PublishMsgs: No specificTargetTopic passed and treat-as-broadcast is not enabled" );
            return false;
        }
    }

    TopicLinkPtr topicLink;
    {
        MT::CScopeReaderLock readerLock( m_rwdataLock );

        TopicPtrMap::iterator i = m_topicPtrs.find( specificTargetTopic );
        if ( i != m_topicPtrs.end() )
        {
            topicLink = (*i).second;
        }
        else
        {
            // it is possible the underlying implementation was swapped out
            CPubSubClientTopicBasicPtr newTopicObj = GetCurrentUnderlyingPubSubClientTopicByName( specificTargetTopic->GetTopicName() );
            if ( !newTopicObj.IsNULL() )
            {
                if ( newTopicObj == specificTargetTopic )
                {
                    MT::CScopeWriterLock writeLock( readerLock );

                    // This is a publish on a topic for which we have not received the topic creation event notification yet
                    // in multi-threaded setups the order in which the events arrive vs other event listeners on the same can vary depending
                    // depending on thread timings
                    ConfigureTopicLink( m_sideSettings, newTopicObj, true );

                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PublishMsgs: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                        " was not registered yet, will do so now to instance " + CORE::ToString( newTopicObj.GetPointerAlways() ) +
                        " for topic " + newTopicObj->GetTopicName() );

                    i = m_topicPtrs.find( specificTargetTopic );
                    if ( i != m_topicPtrs.end() )
                    {
                        topicLink = (*i).second;
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                            "):PublishMsgs: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                            " was not registered yet, failed to register instance " + CORE::ToString( newTopicObj.GetPointerAlways() ) +
                            " for topic " + newTopicObj->GetTopicName() );

                        return false;
                    }
                }
                else
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PublishMsgs: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                        " seems to have been replaced by a new instance " + CORE::ToString( newTopicObj.GetPointerAlways() ) +
                        " for topic " + newTopicObj->GetTopicName() );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):PublishMsgs: specificTargetTopic passed " + CORE::ToString( specificTargetTopic ) +
                    " is not a known topic nor does a topic by that name exist: " + specificTargetTopic->GetTopicName() );
                return false;
            }
        }
    }

    if ( !topicLink.IsNULL() )
    {
        return topicLink->PublishMsgs( msgs );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename TMsgCollection >
bool
CPubSubClientSide::TopicLink::PublishMsgs( const TMsgCollection& msgs )
{GUCEF_TRACE;

    // We need to determine if the caller is running in the same thread
    // We only want to take the hit of using the mailbox (requires dynamic mem usage) if we are multi-threaded as its a performance trade-off
    if ( threadIdOfTopicLink == MT::GetCurrentTaskID() )
    {
        if ( !awaitingFailureReport )
        {
            return PublishMsgsSync< const TMsgCollection >( msgs );
        }
        else
        {
            // The pipe is jammed up due to a failure
            // use the mailbox as a temp queue while we wait to resolve the issue
            //
            // this is a performance tradeoff where pulling messages in-proc as a prefetch is expected
            // to provide faster recovery
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):PublishMsgs: awaiting failure report. Deferring transmission as async send for " + CORE::ToString( msgs.size() ) +
                " msgs for topic " + ( !topic.IsNULL() ? topic->GetTopicName() : "\"\"" )  );

            return PublishMsgsASync< const TMsgCollection >( msgs );
        }
    }
    else
    {
        // caller is in a different thread from this side
        // always use the mailbox
        return PublishMsgsASync< const TMsgCollection >( msgs );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientSide::GetMsgAttributesForLog( const CIPubSubMsg& msg )
{GUCEF_TRACE;

    return "MsgId=\"" + msg.GetMsgId().AsString() + "\", MsgIndex=\"" + msg.GetMsgIndex().AsString() + "\", MsgDateTime=\"" + CORE::ToString( msg.GetMsgDateTime() ) +
            "\", MsgPrimaryPayloadSize=\"" + CORE::ToString( msg.GetPrimaryPayload().ByteSize( false ) ) + "\", MsgNrOfKeyValuePairs=\"" + CORE::ToString( msg.GetKeyValuePairs().size() ) + "\", MsgNrOfMetaDataKeyValuePairs=\"" + CORE::ToString( msg.GetMetaDataKeyValuePairs().size() ) +
            "\", ReceiveActionId=\"" + CORE::ToString( msg.GetReceiveActionId() ) + "\", OriginClientTopic=\"" + ( !msg.GetOriginClientTopic().IsNULL() ?  msg.GetOriginClientTopic()->GetTopicName() : CORE::CString::Empty ) + "\"";
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnCheckForTimedOutInFlightMessagesTimerCycle( CORE::CNotifier* notifier    ,
                                                                            const CORE::CEvent& eventId  ,
                                                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    CORE::CObserverNotificationHold hold( this );

    CORE::UInt64 totalMsgsInFlightCount = 0;
    CORE::CDateTime cycleNowDt = CORE::CDateTime::NowUTCDateTime();

    totalMsgsInFlightCount += inFlightMsgs.size();

    if ( !topic.IsNULL() && topic->IsPublishingSupported() && GUCEF_NULL != side )
    {
        const CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
        "):OnCheckForTimedOutInFlightMessagesTimerCycle: Topic " + topic->GetTopicName() + " has " + CORE::ToString( inFlightMsgs.size() ) + " messages in flight" );

        TopicLink::TUInt64Set inFlightDiscardList;
        TopicLink::TUInt64ToMsgTrackingEntryMap::iterator m = inFlightMsgs.begin();
        while ( m != inFlightMsgs.end() )
        {
            TopicLink::MsgTrackingEntry& inFlightEntry = (*m).second;
            if ( !inFlightEntry.msg.IsNULL() )
            {
                if ( inFlightEntry.isInFlight && inFlightEntry.waitingForInFlightConfirmation )
                {
                    CORE::Int64 timeInFlightInMs = inFlightEntry.lastPublishAttempt.GetTimeDifferenceInMillisecondsTowards( cycleNowDt );
                    if ( timeInFlightInMs > sideSettings->maxPublishedMsgInFlightTimeInMs )
                    {
                        // Since the max time elapsed we no longer consider the message in-flight
                        inFlightEntry.isInFlight = false;

                        if ( sideSettings->allowTimedOutPublishedInFlightMsgsRetryOutOfOrder )
                        {
                            // We will act as if we received a notice that publishing failed and will perform the regular
                            // retry routine

                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                "):OnCheckForTimedOutInFlightMessagesTimerCycle: A message on topic " + topic->GetTopicName() + " has exceeded max in-flight time in ms (" + CORE::ToString( timeInFlightInMs ) +
                                "). Will consider publishing as failed and will retry. First publish attempt was at " + CORE::ToString( inFlightEntry.firstPublishAttempt ) +
                                " and the last attempt occured at " + CORE::ToString( inFlightEntry.lastPublishAttempt ) +
                                ". publishActionId=" + CORE::ToString( inFlightEntry.publishActionId ) + ". " +
                                CPubSubClientSide::GetMsgAttributesForLog( *inFlightEntry.msg ) );

                            // If the message has not been retried yet we will bump up the last published time
                            // this prevents the message from being discarded due to exceeding max retry time
                            if ( 0 == inFlightEntry.retryCount )
                                inFlightEntry.firstPublishAttempt = cycleNowDt;

                            publishFailedMsgs.insert( inFlightEntry.publishActionId );
                        }
                        else
                        {
                            inFlightDiscardList.insert( inFlightEntry.publishActionId );

                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                "):OnCheckForTimedOutInFlightMessagesTimerCycle: A message on topic " + topic->GetTopicName() + " has exceeded max in-flight time in ms (" + CORE::ToString( timeInFlightInMs ) +
                                ") Since we do not allow out-of-order retry for these the mesage will be lost! First publish attempt was at " + CORE::ToString( inFlightEntry.firstPublishAttempt ) +
                                " and the last attempt occured at " + CORE::ToString( inFlightEntry.lastPublishAttempt ) +
                                ". publishActionId=" + CORE::ToString( inFlightEntry.publishActionId ) + ". " +
                                CPubSubClientSide::GetMsgAttributesForLog( *inFlightEntry.msg ) );
                        }
                    }
                }
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):OnCheckForTimedOutInFlightMessagesTimerCycle: A message tracking entry on topic " + topic->GetTopicName() + " has no message associated. We will discard the entry. publishActionId=" + CORE::ToString( inFlightEntry.publishActionId ) );

                inFlightDiscardList.insert( inFlightEntry.publishActionId );
            }

            ++m;
        }

        size_t nrOfDiscardedMsgs = inFlightDiscardList.size();
        if ( nrOfDiscardedMsgs > 0 )
        {
            // Before we delete the message in-memory lets send it to the dead letter queue if one is configured for this flow
            if ( GUCEF_NULL != flowRouter )
            {
                // We create a list with all messages instead of trying to send to the dead letter destination on a per message basis
                // reason being that this is a localized cost whereas publishing across has a cross-locking blocking cost when running multi-threaded
                // hence a batch operation reduces the calling /locking overhead
                CPubSubClientTopic::TPubSubMsgsRefVector discardedMsgs;
                discardedMsgs.reserve( nrOfDiscardedMsgs );

                TopicLink::TUInt64Set::iterator n = inFlightDiscardList.begin();
                while ( n != inFlightDiscardList.end() )
                {
                    TopicLink::MsgTrackingEntry& trackingEntry = inFlightMsgs[ (*n) ];
                    CPubSubClientTopic::TPubSubMsgRef msgRef = trackingEntry.msg.GetPointerAlways();
                    discardedMsgs.push_back( msgRef );
                    ++n;
                }

                if ( flowRouter->PublishMsgs( side, discardedMsgs, RouteType::DeadLetter ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                        "):OnCheckForTimedOutInFlightMessagesTimerCycle: For topic " + topic->GetTopicName() + " we successfully sent a total of " + CORE::ToString( nrOfDiscardedMsgs ) +
                        " messages to a dead letter destination" );
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                        "):OnCheckForTimedOutInFlightMessagesTimerCycle: For topic " + topic->GetTopicName() + " we were unable to send a total of " + CORE::ToString( nrOfDiscardedMsgs ) +
                        " messages to any dead letter destination" );
                }
            }

            // end of the road as far as we are concerned here
            // just delete the relevant tracking entries
            TopicLink::TUInt64Set::iterator n = inFlightDiscardList.begin();
            while ( n != inFlightDiscardList.end() )
            {
                inFlightMsgs.erase( (*n) );
                ++n;
            }

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                "):OnCheckForTimedOutInFlightMessagesTimerCycle: For topic " + topic->GetTopicName() + " we discarded a total of " + CORE::ToString( nrOfDiscardedMsgs ) +
                " messages due to exceeding the max in-flight time allowed which is configured as " + CORE::ToString( sideSettings->maxPublishedMsgInFlightTimeInMs ) );
        }
    }

    UpdateTopicMetrics();
    totalMsgsInFlight = totalMsgsInFlightCount;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::ProcessMailbox( void )
{GUCEF_TRACE;

    // Are we supporting blanket cross-topic broadcasts?
    if ( m_sideSettings->treatPublishWithoutTargetTopicAsBroadcast )
    {
        MT::CScopeReaderLock readerLock( m_rwdataLock );
        bool totalSuccess = true;

        if ( m_sideSettings->treatPublishWithoutTargetTopicAsBroadcast )
        {
            UInt64 approxTotalMsgsInFlight = 0;

            TopicPtrMap::iterator i = m_topicPtrs.begin();
            while ( i != m_topicPtrs.end() )
            {
                TopicLinkPtr topicLink = (*i).second;
                if ( !topicLink.IsNULL() )
                {
                    approxTotalMsgsInFlight += topicLink->GetTotalMsgsInFlight();
                }
                ++i;
            }

            CORE::Int32 maxMailItemsToGrab = -1;
            if ( m_sideSettings->pubsubClientConfig.maxTotalMsgsInFlight > 0 )
            {
                CORE::Int64 remainingForFlight = m_sideSettings->pubsubClientConfig.maxTotalMsgsInFlight - approxTotalMsgsInFlight;
                if ( remainingForFlight > 0 && remainingForFlight < GUCEF_MT_INT32MAX )
                    maxMailItemsToGrab = (CORE::Int32) remainingForFlight;
            }

            CPubSubClientTopic::TIPubSubMsgSPtrVector msgs;
            try
            {
                if ( m_broadcastMailbox.GetSPtrBulkMail( msgs, maxMailItemsToGrab ) )
                {
                    totalSuccess = BroadcastPublishMsgsSync< CPubSubClientTopic::TIPubSubMsgSPtrVector >( msgs ) && totalSuccess;
                }
            }
            catch ( const timeout_exception& )
            {
                if ( !m_broadcastMailbox.ReInsertSPtrBulkMail( msgs ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):RetryPublishFailedMsgsAndProcessMailbox: Failed to reinsert " + CORE::ToString( msgs.size() ) +
                        " messages into the mailbox after an exception occured" );
                }
            }
        }
        return totalSuccess;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::RetryPublishFailedMsgs( void )
{GUCEF_TRACE;

    // Check if we have a max for messages in-flight, if so dont try to process any more messages right now
    if ( GUCEF_NULL != side )
    {
        const CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();
        if ( sideSettings->pubsubClientConfig.maxTotalMsgsInFlight > 0 && totalMsgsInFlight > (CORE::UInt64) sideSettings->pubsubClientConfig.maxTotalMsgsInFlight )
        {
            // We already have the max nr of messages in-flight, dont try to add more right now
            return true;
        }
    }
    if ( awaitingFailureReport )
    {
        // We are awaiting a failure report, lets not complicate things with more messages in flight
        return true;
    }

    MT::CScopeMutex lock( m_dataLock );
    CORE::CObserverNotificationHold hold( this );

    CORE::UInt64 totalMsgsInFlightCount = 0;
    bool publishRetrySuccess = true;

    totalMsgsInFlightCount += inFlightMsgs.size();

    if ( !topic.IsNULL() && GUCEF_NULL != side )
    {
        if ( !publishFailedMsgs.empty() )
        {
            if ( !topic->IsPublishingSupported() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "TopicLink(" + CORE::ToString( this ) +
                    "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has " + CORE::ToString( publishFailedMsgs.size() ) +
                    " publish failed messages and yet the topic is set as being incapable of publishing. This should never happen! Clearing the failed message as resolving the situation is impossible" );

                publishFailedMsgs.clear();
                return false;
            }

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has " + CORE::ToString( publishFailedMsgs.size() ) + " messages queued for retry" );

            const CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();

            TopicLink::TUInt64Set inFlightDiscardList;
            TopicLink::TUInt64Set::iterator n = publishFailedMsgs.begin();
            while ( n != publishFailedMsgs.end() )
            {
                CORE::UInt64 publishActionId = (*n);
                TopicLink::TUInt64ToMsgTrackingEntryMap::iterator m = inFlightMsgs.find( publishActionId );
                if ( m != inFlightMsgs.end() )
                {
                    TopicLink::MsgTrackingEntry& retryEntry = (*m).second;
                    if ( !retryEntry.msg.IsNULL() )
                    {
                        if ( !retryEntry.isInFlight )
                        {
                            // Is this a publish retry or a ack retry?
                            if ( retryEntry.readyToAckPublishSuccessButAckFailed )
                            {
                                // We are retrying a publish success ack for this message

                                // Check if we are applying a max nr of retries and if we are in violation if so
                                if ( sideSettings->maxMsgPublishAckRetryAttempts >= 0                                    &&
                                        retryEntry.retryCount >= (CORE::UInt32) sideSettings->maxMsgPublishAckRetryAttempts )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                        "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has reached max nr of ack retries (" + CORE::ToString( retryEntry.retryCount ) +
                                        ") for acknowledging successfull publishing of a message. First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                        " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                        ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input publishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                        CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                    inFlightDiscardList.insert( retryEntry.publishActionId );
                                    n = publishFailedMsgs.erase( n );
                                    continue;
                                }
                                else
                                if ( sideSettings->maxMsgPublishRetryTotalTimeInMs >= 0 ) // Are we checking for max retry time?
                                {
                                    CORE::Int64 timeDiffInMs = retryEntry.firstPublishAttempt.GetTimeDifferenceInMillisecondsTowards( retryEntry.lastPublishAttempt );
                                    if ( timeDiffInMs >= sideSettings->maxMsgPublishRetryTotalTimeInMs )
                                    {
                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                            "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has reached amount of ack retry time in ms (" + CORE::ToString( timeDiffInMs ) +
                                            ") for acknowledging successfull publishing of a message. First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                            " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                            ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input publishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                            CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                        inFlightDiscardList.insert( retryEntry.publishActionId );
                                        n = publishFailedMsgs.erase( n );
                                        continue;
                                    }
                                }

                                // If we make it here we are allowed to try again

                                ++retryEntry.retryCount;
                                retryEntry.lastPublishAttempt = CORE::CDateTime::NowUTCDateTime();

                                if ( flowRouter != GUCEF_NULL )
                                {
                                    if ( flowRouter->AcknowledgeReceiptForSide( side, retryEntry.msg ) )
                                    {
                                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                                            "):RetryPublishFailedMsgs: Signaled receipt of message acknowledgement. publishActionId=" + CORE::ToString( retryEntry.publishActionId ) +
                                            ". receiveActionId=" + CORE::ToString( retryEntry.msg->GetReceiveActionId() ) + ". Success after " + CORE::ToString( retryEntry.retryCount ) + " ack retry attempts" );

                                        inFlightMsgs.erase( m );
                                    }
                                    else
                                    {
                                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                                            "):RetryPublishFailedMsgs: Still failed to signal receipt of message acknowledgement. publishActionId=" + CORE::ToString( retryEntry.publishActionId ) +
                                            ". receiveActionId=" + CORE::ToString( retryEntry.msg->GetReceiveActionId() ) + ". Currently at " + CORE::ToString( retryEntry.retryCount ) + " ack retry attempts" );
                                    }
                                }
                            }
                            else
                            {
                                // We are retrying a backend publish of the message

                                // Check if we are applying a max nr of retries and if we are in violation if so
                                if ( sideSettings->maxMsgPublishRetryAttempts >= 0                                    &&
                                     retryEntry.retryCount >= (CORE::UInt32) sideSettings->maxMsgPublishRetryAttempts )
                                {
                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                        "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has reached max nr of publish retries (" + CORE::ToString( retryEntry.retryCount ) +
                                        ") for publishing a message. First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                        " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                        ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input publishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                        CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                    inFlightDiscardList.insert( retryEntry.publishActionId );
                                    n = publishFailedMsgs.erase( n );
                                    continue;
                                }
                                else
                                if ( sideSettings->maxMsgPublishRetryTotalTimeInMs >= 0 ) // Are we checking for max retry time?
                                {
                                    CORE::Int64 timeDiffInMs = retryEntry.firstPublishAttempt.GetTimeDifferenceInMillisecondsTowards( retryEntry.lastPublishAttempt );
                                    if ( timeDiffInMs >= sideSettings->maxMsgPublishRetryTotalTimeInMs )
                                    {
                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                            "):RetryPublishFailedMsgs: Topic " + topic->GetTopicName() + " has reached amount of publish retry time in ms (" + CORE::ToString( timeDiffInMs ) +
                                            ") for publishing a message. First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                            " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                            ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input publishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                            CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                        inFlightDiscardList.insert( retryEntry.publishActionId );
                                        n = publishFailedMsgs.erase( n );
                                        continue;
                                    }
                                }

                                // If we make it here we are allowed to try again

                                ++retryEntry.retryCount;
                                retryEntry.lastPublishAttempt = CORE::CDateTime::NowUTCDateTime();

                                CORE::UInt64 publishActionId = retryEntry.publishActionId;
                                retryEntry.isInFlight = true;
                                retryEntry.waitingForInFlightConfirmation = true;
                                if ( topic->Publish( publishActionId, *retryEntry.msg.GetPointerAlways(), sideSettings->needToTrackInFlightPublishedMsgsForAck ) )
                                {
                                    // It worked this time!

                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                        "):RetryPublishFailedMsgs: Successfully retried publish of message on topic " + topic->GetTopicName() + " after " + CORE::ToString( retryEntry.retryCount ) +
                                        " retries. First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                        " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                        ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input publishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                        CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                    n = publishFailedMsgs.erase( n );
                                }
                                else
                                {
                                    // Still didnt work :(
                                    publishRetrySuccess = false;

                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                                        "):RetryPublishFailedMsgs: Retry failed of publish of message on topic " + topic->GetTopicName() + ". Retry attempt " + CORE::ToString( retryEntry.retryCount ) +
                                        ". First retry attempt was at " + CORE::ToString( retryEntry.firstPublishAttempt ) +
                                        " and this last attempt was started at " + CORE::ToString( retryEntry.lastPublishAttempt ) +
                                        ". output publishActionId=" + CORE::ToString( publishActionId ) + ". input PublishActionId=" + CORE::ToString( retryEntry.publishActionId ) + ". " +
                                        CPubSubClientSide::GetMsgAttributesForLog( *retryEntry.msg ) );

                                    if ( !sideSettings->allowOutOfOrderPublishRetry )
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                            "):RetryPublishFailedMsgs: A message tracking entry on topic " + topic->GetTopicName() + " has no message associated. We will discard the entry. publishActionId=" + CORE::ToString( retryEntry.publishActionId ) );

                        inFlightDiscardList.insert( retryEntry.publishActionId );
                    }
                }

                if ( n != publishFailedMsgs.end() )
                    ++n;
            }

            size_t discardedMsgs = inFlightDiscardList.size();
            TopicLink::TUInt64Set::iterator d = inFlightDiscardList.begin();
            while ( d != inFlightDiscardList.end() )
            {
                inFlightMsgs.erase( (*d) );
                ++d;
            }
            if ( discardedMsgs > 0 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):RetryPublishFailedMsgs: For topic " + topic->GetTopicName() + " we discarded a total of " + CORE::ToString( discardedMsgs ) +
                    " messages due to exceeding the max retry attempts and/or sanity checks" );
            }
        }
    }
    UpdateTopicMetrics();

    totalMsgsInFlight = totalMsgsInFlightCount;
    return publishRetrySuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::PublishMailboxMsgs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    bool totalSuccess = true;
    if ( !topic.IsNULL() && GUCEF_NULL != side )
    {
        CORE::Int32 maxMailItemsToGrab = -1;
        CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();

        if ( sideSettings->pubsubClientConfig.maxTotalMsgsInFlight > 0 )
        {
            CORE::Int64 remainingForFlight = sideSettings->pubsubClientConfig.maxTotalMsgsInFlight - inFlightMsgs.size();
            if ( remainingForFlight > 0 && remainingForFlight < GUCEF_MT_INT32MAX )
                maxMailItemsToGrab = (CORE::Int32) remainingForFlight;
        }

        CPubSubClientTopic::TIPubSubMsgSPtrVector msgs;
        try
        {
            if ( msgMailbox.GetSPtrBulkMail( msgs, maxMailItemsToGrab ) )
            {
                bool publishSuccess = false;
                bool timeoutOccured = false;
                try
                {

                    CPubSubClientTopic::TIPubSubMsgSPtrVector::iterator i = msgs.begin();
                    publishSuccess = PublishMsgsSync< CPubSubClientTopic::TIPubSubMsgSPtrVector >( msgs );
                    totalSuccess = publishSuccess && totalSuccess;
                }
                catch ( const timeout_exception& )
                {
                    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) + " caught timeout_exception while attempting to PublishMsgsSync" );

                    publishSuccess = false;
                    timeoutOccured = true;
                    totalSuccess = false;
                }
                if ( !publishSuccess )
                {
                    if ( !msgMailbox.ReInsertSPtrBulkMail( msgs ) )
                    {
                        if ( timeoutOccured )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "TopicLink(" + CORE::ToString( this ) +
                                "):PublishMailboxMsgs: Failed to pop back " + CORE::ToString( msgs.size() ) +
                                " messages into the mailbox after a timeout exception occured" );
                        }
                        else
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "TopicLink(" + CORE::ToString( this ) +
                                "):PublishMailboxMsgs: Failed to pop back " + CORE::ToString( msgs.size() ) +
                                " messages into the mailbox after a failure to publish occured" );
                        }
                    }
                }
            }
        }
        catch ( const timeout_exception& )
        {
            totalSuccess = false;
            if ( !msgMailbox.ReInsertSPtrBulkMail( msgs ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "TopicLink(" + CORE::ToString( this ) +
                    "):PublishMailboxMsgs: Failed to pop back " + CORE::ToString( msgs.size() ) +
                    " messages into the mailbox after a timeout exception occured" );
            }
        }
        catch ( const std::exception& e )
        {
            totalSuccess = false;
            if ( !msgMailbox.ReInsertSPtrBulkMail( msgs ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "TopicLink(" + CORE::ToString( this ) +
                    "):PublishMailboxMsgs: Failed to pop back " + CORE::ToString( msgs.size() ) +
                    " messages into the mailbox after an exception occured" );
            }
            throw e;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::RegisterTopicEventHandlers( CPubSubClientTopicBasicPtr topic )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "TopicLink(" + CORE::ToString( this ) +
        "):RegisterTopicEventHandlers: Registering event handlers for topic with name: " + topic->GetTopicName() );

    TEventCallback callback( this, &CPubSubClientSide::TopicLink::OnPubSubTopicMsgsReceived );
    SubscribeTo( topic.GetPointerAlways()              ,
                 CPubSubClientTopic::MsgsRecievedEvent ,
                 callback                              );

    TEventCallback callback2( this, &CPubSubClientSide::TopicLink::OnPubSubTopicMsgsPublished );
    SubscribeTo( topic.GetPointerAlways()               ,
                 CPubSubClientTopic::MsgsPublishedEvent ,
                 callback2                              );

    TEventCallback callback3( this, &CPubSubClientSide::TopicLink::OnPubSubTopicMsgsPublishFailure );
    SubscribeTo( topic.GetPointerAlways()                    ,
                 CPubSubClientTopic::MsgsPublishFailureEvent ,
                 callback3                                   );

    TEventCallback callback4( this, &CPubSubClientSide::TopicLink::OnPubSubTopicLocalPublishQueueFull );
    SubscribeTo( topic.GetPointerAlways()                       ,
                 CPubSubClientTopic::LocalPublishQueueFullEvent ,
                 callback4                                      );

    TEventCallback callback5( this, &CPubSubClientSide::TopicLink::OnCheckForTimedOutInFlightMessagesTimerCycle );
    SubscribeTo( &timedOutInFlightMessagesCheckTimer ,
                 CORE::CTimer::TimerUpdateEvent        ,
                 callback5                             );

    TEventCallback callback6( this, &CPubSubClientSide::TopicLink::OnMetricsTimerCycle );
    SubscribeTo( &metricsTimer                  ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::DetachFromTopic( void )
{GUCEF_TRACE;

    CPubSubClientTopicBasicPtr topicCopy = topic;
    if ( topicCopy.IsNULL() ) // this check can have a race condition but its merely here to save on some lock penalties
        return;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
        "):DetachFromTopic: Detaching from topic implementation " + CORE::ToString( topicCopy.GetPointerAlways() ) +
        " for topic \"" + topicCopy->GetTopicName() + "\"" );

    // Stop the influx of new event messages into our logic before anything else
    // we dont lock this here because we dont want to co-mingle the notification level locks
    UnsubscribeFrom( topicCopy.GetPointerAlways() );

    // lock to ensure no other operation is in some state of partial completion
    MT::CScopeMutex lock( m_dataLock );
    if ( topicCopy != topic )
        return;

    topic.Unlink();


}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::UpdateTopicMetrics( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    if ( !metrics.IsNULL() )
    {
        if ( metrics->hasSupportForPublishing )
        {
            metrics->publishOrAckFailedMsgs = publishFailedMsgs.size();
            metrics->lastPublishBatchSize = currentPublishActionIds.size();
            metrics->publishedMsgsInFlight = inFlightMsgs.size();
        }
        if ( metrics->hasSupportForSubscribing )
        {
            metrics->queuedReceiveSuccessAcks = publishAckdMsgsMailbox.AmountOfMail();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnPubSubTopicLocalPublishQueueFull( CORE::CNotifier* notifier    ,
                                                                  const CORE::CEvent& eventId  ,
                                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClientTopic* topic = static_cast< CPubSubClientTopic* >( notifier );

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
        "):OnPubSubTopicLocalPublishQueueFull: Topic=" + topic->GetTopicName() );


    // @TODO: Anything extra to do here?
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnPubSubTopicMsgsReceived( CORE::CNotifier* notifier    ,
                                                         const CORE::CEvent& eventId  ,
                                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    try
    {
        const CPubSubClientTopic::TPubSubMsgsRefVector& msgs = *static_cast< CPubSubClientTopic::TPubSubMsgsRefVector* >( eventData );
        if ( !msgs.empty() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):OnPubSubTopicMsgsReceived: Received " + CORE::ToString( msgs.size() ) + " message(s) from pubsub client on side: " + side->GetSideId() );

            MT::CScopeMutex lock( m_dataLock );

            // We now broadcast the received messages to all other sides which is the purpose of this class
            if ( GUCEF_NULL != flowRouter )
            {
                bool totalSuccess = flowRouter->PublishMsgs( side, msgs, RouteType::Active );
                if ( totalSuccess )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Successfully relayed message(s) via flow router from side " + side->GetSideId() );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Failed to relay (some?) message(s) via flow router from side " + side->GetSideId() );
                }

                if ( totalSuccess && clientFeatures.supportsBookmarkingConcept && !clientFeatures.supportsDerivingBookmarkFromMsg )
                {
                    const CPubSubClientTopic::TPubSubMsgRef& lastMsgRef = msgs[ msgs.size()-1 ];
                    CPubSubBookmark currentBookmark;
                    if ( clientFeatures.supportsDerivingBookmarkFromMsg )   // do we need this???
                    {
                        if ( !lastMsgRef->GetOriginClientTopic()->DeriveBookmarkFromMsg( *lastMsgRef, currentBookmark ) )
                        {
                            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                                "):OnPubSubTopicMsgsReceived: Unable to derive bookmark from message even through its supported. Falling back to getting bookmark for the current message batch" );

                            currentBookmark = lastMsgRef->GetOriginClientTopic()->GetCurrentBookmark();
                        }
                    }
                    else
                    {
                        currentBookmark = lastMsgRef->GetOriginClientTopic()->GetCurrentBookmark();
                    }
                    CPubSubBookmark::TBookmarkType currentBookmarkType = currentBookmark.GetBookmarkType();

                    switch ( currentBookmarkType )
                    {
                        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE:
                        {
                            // Better luck next batch, we will potentially not be able to use bookmarking to cover the current batch in isolation
                            // if derivation of a bookmark from a message is not supported
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                                "):OnPubSubTopicMsgsReceived: Current bookmark is temp not available for the current message batch" );
                            break;
                        }
                        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_APPLICABLE:
                        {
                            // We should not get here, if we do there is some logic error in feature checking here or in
                            // bookmark management in the backend code
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                                "):OnPubSubTopicMsgsReceived: Current bookmark obtained from backend came back as not applicable. This should not happen based on the feature set!" );
                            break;
                        }
                        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED:
                        {
                            // We should not get here, if we do there is some logic error in feature checking here or in
                            // bookmark management in the backend code
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                                "):OnPubSubTopicMsgsReceived: Current bookmark obtained from backend came back as not initialized. This should not happen!" );
                            break;
                        }
                        default:
                        {
                            // Per the spec the 'current' bookmark requested during this event and its callback handler will be
                            // bookmark position of the beginning of this message batch 'msgs'
                            // As such even if the backend does not support deriving a bookmark from a message we can still use bookmarks
                            // we just need perform some extra administration to keep track of them. Its the extra admin needs that make
                            // the derivation of a bookmark directly from a message the preferred route as its likely to be more efficient
                            if ( side->GetSideSettings()->needToTrackInFlightPublishedMsgsForAck )
                            {
                                if ( !clientFeatures.supportsDerivingBookmarkFromMsg )
                                {
                                    // note this this operation is in opposition to the one below where we dont need to keep track because we dont care
                                    // about delayed acks. In that case you have the luxory of skipping this overhead
                                    //
                                    // Note that entries added here are removed at a later time via calls to CleanupMsgBatchBookmarksUpTo()
                                    bookmarksOnMsgReceived[ lastMsgRef->GetReceiveActionId() ] = currentBookmark;

                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                                        "):OnPubSubTopicMsgsReceived: Retaining bookmark at recieveActionId " + CORE::ToString( lastMsgRef->GetReceiveActionId() ) );
                                }
                            }
                            else
                            {
                                // If we successfully published but we dont need to wait on any acks then we can update bookmarks right now if applicable
                                // instead of waiting for the ack to come back before doing the same thing
                                const CPubSubClientTopic::TPubSubMsgRef& lastMsgRef = msgs[ msgs.size()-1 ];
                                msgsSinceLastBookmarkPersist += (Int32) msgs.size();
                                UpdateReceivedMessagesBookmarkAsNeeded( *lastMsgRef, currentBookmark );
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) + "):OnPubSubTopicMsgsReceived: exception: " + CORE::CString( e.what() ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::UpdateReceivedMessagesBookmarkAsNeeded( const CIPubSubMsg& msg                  ,
                                                                      const CPubSubBookmark& msgBatchBookmark )
{GUCEF_TRACE;

    if ( GUCEF_NULL == side )
        return false;

    if ( !clientFeatures.supportsBookmarkingConcept )
        return true; // not supported, treat as fyi no-op

    MT::CScopeMutex lock( m_dataLock );

    // Check criterea for our generic bookmark persistance
    const CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();
    if ( !pubsubBookmarkPersistence.IsNULL() &&
            (
                ( sideSettings->pubsubBookmarkPersistenceConfig.autoPersistMsgInterval > 0 && msgsSinceLastBookmarkPersist >= sideSettings->pubsubBookmarkPersistenceConfig.autoPersistMsgInterval ) ||
                ( sideSettings->pubsubBookmarkPersistenceConfig.autoPersistIntervalInMs > 0 && lastBookmarkPersistSuccess.GetTimeDifferenceInMillisecondsToNow() >= sideSettings->pubsubBookmarkPersistenceConfig.autoPersistIntervalInMs )
            ) )
    {
        CPubSubClientPtr client = side->GetCurrentUnderlyingPubSubClient();
        if ( pubsubBookmarkPersistence->StoreBookmark( bookmarkNamespace          ,
                                                       *client.GetPointerAlways() ,
                                                       *topic                     ,
                                                       msgBatchBookmark           ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                "):UpdateReceivedMessagesBookmarkAsNeeded: Stored bookmark. receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );

            lastBookmarkPersistSuccess = CORE::CDateTime::NowUTCDateTime();
            msgsSinceLastBookmarkPersist = 0;
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                "):UpdateReceivedMessagesBookmarkAsNeeded: Failed to store bookmark. msgsSinceLastBookmarkPersist=" + CORE::ToString( msgsSinceLastBookmarkPersist ) +
                ". lastBookmarkPersistSuccess=" + CORE::ToString( lastBookmarkPersistSuccess ) + ". " +
                CPubSubClientSide::GetMsgAttributesForLog( msg ) );
            return false;
        }
    }

    // not supported due to config or does not apply due to storage contraints
    // either way... treat as fyi no-op
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::UpdateReceivedMessagesBookmarkAsNeeded( const CIPubSubMsg& msg )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    if ( clientFeatures.supportsBookmarkingConcept )
    {
        // Deriving a bookmark from a message is preferred if supported due to the reduced administrative overhead
        // versus the assumed-to-be-implemented-more-efficiently 'DeriveBookmarkFromMsg' operation
        CPubSubBookmark bookmark;
        if ( clientFeatures.supportsDerivingBookmarkFromMsg )
        {
            if ( topic->DeriveBookmarkFromMsg( msg, bookmark ) )
            {
                // this message will represent the 'bookmarked' batch which may differ from the batch size in which it was originally received
                // functionality this does not matter as the effect achieved is the same
                return UpdateReceivedMessagesBookmarkAsNeeded( msg, bookmark );
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):UpdateReceivedMessagesBookmarkAsNeeded: Failed to derive bookmark from message. msgsSinceLastBookmarkPersist=" + CORE::ToString( msgsSinceLastBookmarkPersist ) +
                    ". lastBookmarkPersistSuccess=" + CORE::ToString( lastBookmarkPersistSuccess ) + ". " +
                    CPubSubClientSide::GetMsgAttributesForLog( msg ) );
                return false;
            }
        }
        else
        {
            // find the bookmark associated with the message batch
            // this gets a tad tricky since we do not store bookmarks for every message in a batch plus the ones we do store at the batch beginning
            // may not line up since this message is likely not the exact beginning of a batch
            // As such we rely on the receiveActionId to help us out since we know it is required to always increment
            // We can use this in a sorted map to find the closest bookmark <= the position of this message
            UInt64 msgBatchBookmarkReceiveId = 0;
            if ( FindClosestMsgBatchBookmarkToMsg( msg, msgBatchBookmarkReceiveId, bookmark ) )
            {
                // the message passed will represent the closest 'bookmarked' batch which may differ from the batch size in which it was originally received
                // functionality this does not matter as the effect achieved is the same
                if ( UpdateReceivedMessagesBookmarkAsNeeded( msg, bookmark ) )
                {
                    CleanupMsgBatchBookmarksUpTo( msgBatchBookmarkReceiveId );
                    return true;
                }
                return false;
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):UpdateReceivedMessagesBookmarkAsNeeded: Failed to locate nearest msg batch bookmark for message. msgsSinceLastBookmarkPersist=" + CORE::ToString( msgsSinceLastBookmarkPersist ) +
                    ". lastBookmarkPersistSuccess=" + CORE::ToString( lastBookmarkPersistSuccess ) + ". " +
                    CPubSubClientSide::GetMsgAttributesForLog( msg ) );
                return false;
            }
        }
    }

    // not supported, treat as fyi no-op
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::FindClosestMsgBatchBookmarkToMsg( const CIPubSubMsg& msg            ,
                                                                UInt64& msgBatchBookmarkReceiveId ,
                                                                CPubSubBookmark& msgBatchBookmark )
{GUCEF_TRACE;

    CORE::UInt64 msgReceiveActionId = msg.GetReceiveActionId();
    CORE::UInt64 lastBestBmReceiveActionId = 0;
    const CPubSubBookmark* lastBestBm = GUCEF_NULL;

    // the map will store the IDs in ascending order, we can count on that
    TopicLink::TUInt64ToBookmarkMap::const_iterator i = bookmarksOnMsgReceived.begin();
    while ( i != bookmarksOnMsgReceived.end() )
    {
        CORE::UInt64 bmReceiveActionId = (*i).first;
        if ( bmReceiveActionId <= msgReceiveActionId )
        {
            lastBestBmReceiveActionId = bmReceiveActionId;
            lastBestBm = &(*i).second;
        }
        else
            break;
        ++i;
    }

    if ( 0 != lastBestBmReceiveActionId && GUCEF_NULL != lastBestBm )
    {
        // We found closest bookmark we can use which matches or pre-dates the bookmark
        // that would have existed for the message given
        msgBatchBookmarkReceiveId = lastBestBmReceiveActionId;
        msgBatchBookmark.LinkTo( *lastBestBm );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::CleanupMsgBatchBookmarksUpTo( UInt64 msgBatchBookmarkReceiveId )
{GUCEF_TRACE;

    // the map will store the IDs in ascending order, we can count on that, hence we can delete the entire range
    TopicLink::TUInt64ToBookmarkMap::iterator i = bookmarksOnMsgReceived.find( msgBatchBookmarkReceiveId );
    if ( i != bookmarksOnMsgReceived.end() )
    {
        bookmarksOnMsgReceived.erase( bookmarksOnMsgReceived.begin(), i );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::ProcessAcknowledgeReceiptsMailbox( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( publishAckdMsgsMailbox.GetLock() );

    CIPubSubMsg::TNoLockSharedPtr msg;
    while ( publishAckdMsgsMailbox.PeekMail( lock, msg, GUCEF_NULL ) )
    {
        try
        {
            if ( AcknowledgeReceiptSync( msg ) )
            {
                publishAckdMsgsMailbox.PopMail( lock );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):ProcessAcknowledgeReceiptsMailbox: Failed to sync ack receipt of message. " + CPubSubClientSide::GetMsgAttributesForLog( *msg ) );
            }
        }
        catch ( const timeout_exception& )
        {
            GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                        "):ProcessAcknowledgeReceiptsMailbox: caught timeout_exception while attempting to ack msg receipt. " +
                        CPubSubClientSide::GetMsgAttributesForLog( *msg ) );
            break;
        }
    }

    UpdateTopicMetrics();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::AcknowledgeReceipt( CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    TopicPtrMap::iterator i = m_topicPtrs.find( msg->GetOriginClientTopic().GetPointerAlways() );
    if ( i != m_topicPtrs.end() )
    {
        TopicLinkPtr topicLink = (*i).second;

        lock.EarlyUnlock();

        if ( !topicLink.IsNULL() )
        {
            return topicLink->AcknowledgeReceipt( msg );
        }
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):AcknowledgeReceiptSync: Message origin topic is not familiar when trying to ack msg receipt. " +
                    CPubSubClientSide::GetMsgAttributesForLog( *msg ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::AcknowledgeReceipt( CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    if ( threadIdOfTopicLink == MT::GetCurrentTaskID() )
        return AcknowledgeReceiptSync( msg );
    else
        return AcknowledgeReceiptASync( msg );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::AcknowledgeReceiptSync( CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    bool success = true;
    if ( clientFeatures.supportsSubscriberMsgReceivedAck )
        success = msg->GetOriginClientTopic()->AcknowledgeReceipt( *msg );

    if ( success )
    {
        // the ack success determines the result, the bookmark persistance is best-effort
        ++msgsSinceLastBookmarkPersist;
        UpdateReceivedMessagesBookmarkAsNeeded( *msg.GetPointerAlways() );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::TopicLink::AcknowledgeReceiptASync( CIPubSubMsg::TNoLockSharedPtr& msg )
{GUCEF_TRACE;

    return publishAckdMsgsMailbox.AddMail( msg );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnPubSubTopicMsgsPublished( CORE::CNotifier* notifier    ,
                                                          const CORE::CEvent& eventId  ,
                                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || GUCEF_NULL == notifier || GUCEF_NULL == side )
        return;

    if ( !needToTrackInFlightPublishedMsgsForAck )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublished: Ignoring msg acks from client because we are not tracking msgs for acks" );
        return;
    }

    const CPubSubClientTopic::TMsgsPublishedEventData& eData = *static_cast< CPubSubClientTopic::TMsgsPublishedEventData* >( eventData );
    const CPubSubClientTopic::TPublishActionIdVector* publishActionIds = eData;
    CPubSubClientTopic* topicNotifier = static_cast< CPubSubClientTopic* >( notifier );

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                        "):OnPubSubTopicMsgsPublished: Received " + CORE::ToString( publishActionIds->size() ) +
                        " msg acks from client" );

    MT::CScopeMutex lock( m_dataLock );
    CORE::CObserverNotificationHold hold( this );

    // Here we translate the publish action IDs back into the original messages
    // Subsequently we use said original messages to ack that to the message origin that we received the message
    // This is what allows us to provide a garanteed handling garantee since its an explicit handoff all the way through

    if ( topicNotifier == topic.GetPointerAlways() )
    {
        CPubSubClientTopic::TPublishActionIdVector::const_iterator n = publishActionIds->begin();
        while ( n != publishActionIds->end() )
        {
            TopicLink::TUInt64ToMsgTrackingEntryMap::iterator m = inFlightMsgs.find( (*n) );
            if ( m != inFlightMsgs.end() )
            {
                TopicLink::MsgTrackingEntry& msgTrackingEntry = (*m).second;
                msgTrackingEntry.isInFlight = false;
                msgTrackingEntry.waitingForInFlightConfirmation = false;
                CIPubSubMsg::TNoLockSharedPtr msg = msgTrackingEntry.msg;

                if ( GUCEF_NULL != flowRouter )
                {
                    if ( flowRouter->AcknowledgeReceiptForSide( side, msg ) )
                    {
                        GUCEF_DEBUG_LOG_EVERYTHING( "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublished: Signaled receipt of message acknowledgement. publishActionId=" + CORE::ToString( msgTrackingEntry.publishActionId ) +
                            ". receiveActionId=" + CORE::ToString( msg->GetReceiveActionId() ) );

                        UInt64 msgActionId = msg->GetReceiveActionId();

                        inFlightMsgs.erase( m );
                        if ( totalMsgsInFlight > 0 )
                            --totalMsgsInFlight;

                        if ( !journal.IsNULL() )
                            journal->AddMessageDeletedFromCacheJournalEntry( msgActionId );
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublished: Failed to signal receipt of message acknowledgement of message on topic " + topic->GetTopicName() +
                            ". Will reset entry for PublishAck retries. Publish details: retryCount=" + CORE::ToString( msgTrackingEntry.retryCount ) +
                            ". First publish attempt was at " + CORE::ToString( msgTrackingEntry.firstPublishAttempt ) +
                            " and this last attempt was successfull at " + CORE::ToString( msgTrackingEntry.lastPublishAttempt ) +
                            ". publishActionId=" + CORE::ToString( msgTrackingEntry.publishActionId ) + ". " +
                            CPubSubClientSide::GetMsgAttributesForLog( *msgTrackingEntry.msg ) );

                        // Configure the entry for a different type of retry: Ack retry
                        // This is a different retry and as such we reset the retry count and time, the log message above will provide the original values for tracing
                        msgTrackingEntry.readyToAckPublishSuccessButAckFailed = true;
                        msgTrackingEntry.retryCount = 0;
                        msgTrackingEntry.firstPublishAttempt = CORE::CDateTime::NowUTCDateTime();
                        msgTrackingEntry.lastPublishAttempt = msgTrackingEntry.firstPublishAttempt;

                        publishFailedMsgs.insert( msgTrackingEntry.publishActionId );
                    }
                }
                else
                {
                    // @TODO
                }
            }
            else
            {
                // This should not happen
                // Only understandable reasons are bad eventing from the backend or if the in-flight time-out is configured too aggressively
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                    "):OnPubSubTopicMsgsPublished: Failed to locate original in-flight message related to publishActionId " + CORE::ToString( (*n) ) );
            }
            ++n;
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublished: Processed " + CORE::ToString( publishActionIds->size() ) +
                            " msg acks from client" );

        UpdateTopicMetrics();
    }
    else
    {
        // This should not happen
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublished: Could not process " + CORE::ToString( publishActionIds->size() ) +
                            " msg acks from client because the topic obj is unknown to us" );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::OnPubSubTopicMsgsPublishFailure( CORE::CNotifier* notifier    ,
                                                               const CORE::CEvent& eventId  ,
                                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || GUCEF_NULL == notifier || GUCEF_NULL == side || !side->GetSideSettings()->retryFailedPublishAttempts )
        return;

    const CPubSubClientTopic::TMsgsPublishedEventData& eData = *static_cast< CPubSubClientTopic::TMsgsPublishedEventData* >( eventData );
    const CPubSubClientTopic::TPublishActionIdVector* publishActionIds = eData;
    CPubSubClientTopic* topicNotifier = static_cast< CPubSubClientTopic* >( notifier );

    // Here we translate the publish action IDs back into the original messages
    // Subsequently we use said original messages to set up the retry mechanism

    MT::CScopeMutex lock( m_dataLock );

    if ( topicNotifier == topic.GetPointerAlways() )
    {
        CPubSubClientTopic::TPublishActionIdVector::const_iterator n = publishActionIds->begin();
        while ( n != publishActionIds->end() )
        {
            CORE::UInt64 publishActionId = (*n);
            TopicLink::TUInt64ToMsgTrackingEntryMap::iterator m = inFlightMsgs.find( publishActionId );
            if ( m != inFlightMsgs.end() )
            {
                TopicLink::MsgTrackingEntry& msgTrackingEntry = (*m).second;
                msgTrackingEntry.isInFlight = false;
                msgTrackingEntry.waitingForInFlightConfirmation = false;
                msgTrackingEntry.readyToAckPublishSuccessButAckFailed = false;
                publishFailedMsgs.insert( publishActionId );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnPubSubTopicMsgsPublishFailure: Message with publishActionId of " + CORE::ToString( publishActionId ) + " queued for retry, publish failed" +
                    ". receiveActionId=" + CORE::ToString( msgTrackingEntry.msg->GetReceiveActionId() ) );
            }
            else
            {
                if ( !journal.IsNULL() )
                    journal->AddMessageCacheMissJournalEntry( publishActionId );

                // This should not happen
                // Only understandable reasons are bad eventing from the backend or if the in-flight time-out is configured too aggressively
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnPubSubTopicMsgsPublishFailure: Failed to locate original in-flight message related to publishActionId " + CORE::ToString( (*n) ) );
            }
            ++n;
        }

        UpdateTopicMetrics();
    }
    else
    {
        // This should not happen
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TopicLink(" + CORE::ToString( this ) +
                            "):OnPubSubTopicMsgsPublishFailure: Could not process " + CORE::ToString( publishActionIds->size() ) +
                            " msg failure notifications from client because the topic obj is unknown to us" );
    }

    awaitingFailureReport = false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    msgMailbox.Clear();
    publishAckdMsgsMailbox.Clear();

    pubsubBookmarkPersistence.Unlink();
    topic.Unlink();

    awaitingFailureReport = false;
    threadIdOfTopicLink = 0;
    msgsSinceLastBookmarkPersist = 0;
    totalMsgsInFlight = 0;

    currentPublishActionIds.clear();
    inFlightMsgs.clear();
    publishFailedMsgs.clear();
    metricFriendlyTopicName.Clear();
    lastBookmarkPersistSuccess.Clear();
    bookmarksOnMsgReceived.clear();
    bookmarkNamespace.Clear();

    metrics.Unlink();
    flowRouter = GUCEF_NULL;
    side = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::TopicLink::SetJournal( CIPubSubJournalBasicPtr newJournal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    journal = newJournal;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientSide::GenerateMetricsFriendlyTopicName( const CORE::CString& topicName )
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
CPubSubClientSide::DisconnectPubSubClient( bool destroyClient )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CORE::CObserverNotificationHold hold( this );

    if ( m_pubsubClient.IsNULL() )
        return true;

    if ( !m_pubsubClient->Disconnect() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConnectPubSubClient: Failed to disconnect the pub-sub client" );
        return false;
    }

    if ( destroyClient || !m_clientFeatures.supportsAutoReconnect )
    {
        m_pubsubClient.Unlink();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::ConfigureTopicLink( CPubSubSideChannelSettingsPtr pubSubSideSettings ,
                                       CPubSubClientTopicBasicPtr topic                 ,
                                       bool reset                                       )
{GUCEF_TRACE;

    if ( pubSubSideSettings.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConfigureTopicLink: NULL pubSubSideSettings passed. SideId=" + m_sideId );
        return false;
    }
    if ( topic.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConfigureTopicLink: NULL topic passed. SideId=" + m_sideId );
        return false;
    }

    const CORE::CString& topicName = topic->GetTopicName();

    CPubSubClientTopicConfigPtr topicConfig = pubSubSideSettings->GetTopicConfig( topicName );
    if ( topicConfig.IsNULL() )
    {
        // Considering the backend was able to make a topic but we don't have a config for it
        // it seems this topic was auto created by the backend. As such we will make a copy of the config for our own record keeping
        // this is relevant in case we need to perform a reset which would wipe out the topic access and would want to
        // recreate it based on config
        topicConfig = CPubSubClientTopicConfigPtr( GUCEF_NEW CPubSubClientTopicConfig() );
        if ( topic->SaveConfig( *topicConfig ) )
        {
            pubSubSideSettings->pubsubClientConfig.topics.push_back( topicConfig );

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):ConfigureTopicLink: Obtained a copy of the topic config from the topic itself for topic which has no predefined config. topicName=" + topicName + " SideId=" + m_sideId  );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):ConfigureTopicLink: Unable to get a copy of the topic config from the topic itself for topic which has no predefined config. Knowledge about the Topic will be lost across resets. topicName=" + topicName + " SideId=" + m_sideId );
        }
    }

    CORE::PulseGeneratorPtr pulseGenerator;
    if ( pubSubSideSettings->useBackendTopicThreadForTopicIfAvailable )
        pulseGenerator = topic->GetPulseGenerator();
    if ( pulseGenerator.IsNULL() )
        pulseGenerator = GetPulseGenerator();

    TopicPtrMap::iterator i = m_topicPtrs.find( topic.GetPointerAlways() );
    if ( reset || i == m_topicPtrs.end() )
    {
        TopicLinkPtr topicLink = m_topicPtrs[ topic.GetPointerAlways() ];
        if ( topicLink.IsNULL() )
        {
            TopicNameMap::iterator t = m_topics.find( topic->GetTopicName() );
            if ( t == m_topics.end() )
            {
                topicLink = TopicLink::CreateSharedObj();

                if ( !topicLink.IsNULL() )
                {
                    m_topicPtrs[ topic.GetPointerAlways() ] = topicLink;
                    m_topics[ topic->GetTopicName() ] = topicLink;

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):ConfigureTopicLink: Created topic link for topic with name: " + topicName + " for side with id " + m_sideId );

                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):ConfigureTopicLink: Failed to create topic link for topic with name " + topicName + " for side with id " + m_sideId  );
                    return false;
                }
            }
            else
            {
                topicLink = (*t).second;
                CPubSubClientTopicBasicPtr originalTopic = topicLink->GetTopic();

                if ( topic != originalTopic )
                {
                    // If we get here we had a pre-existing topic link for a topic of this name.
                    // However the implementation, the topic object, is now different.
                    // This can occur when we perform a swap-out/reset of the underlying implementation
                    // such as to solve issues like bad state in some backend implementation

                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):ConfigureTopicLink: Found pre-existing topic link for topic with name \"" + topicName +
                        "\", relinking to the new topic implementation. " + CORE::ToString( originalTopic.GetPointerAlways() ) + " -> " + CORE::ToString( topic.GetPointerAlways() ) +
                        " for side with id " + m_sideId );

                    // Note most of the 're-linking' is actually done in the code a bit further down
                    topicLink->DetachFromTopic();
                    m_topicPtrs[ topic.GetPointerAlways() ] = topicLink;
                }
            }
        }
        else
        {
            // make sure the topic name based index references the latest understanding of the correct topic link
            m_topics[ topic->GetTopicName() ] = topicLink;
        }

        // Create and configure the pub-sub bookmark persistence
        // we create a private copy per topic link to minimize potential contention across threads
        CPubSubBookmarkPersistenceConfig& pubsubBookmarkPersistenceConfig = pubSubSideSettings->pubsubBookmarkPersistenceConfig;
        TIPubSubBookmarkPersistenceBasicPtr pubsubBookmarkPersistence = CPubSubGlobal::Instance()->GetPubSubBookmarkPersistenceFactory().Create( pubsubBookmarkPersistenceConfig.bookmarkPersistenceType, pubsubBookmarkPersistenceConfig );
        if ( pubsubBookmarkPersistence.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):ConfigureTopicLink: Failed to create bookmark persistance access of type \"" + pubsubBookmarkPersistenceConfig.bookmarkPersistenceType + "\". Cannot proceed" );
            return false;
        }

        MT::CObjectScopeLock topicLinkLock( topicLink->AsLockable() );

        topicLink->SetFlowRouter( m_flowRouter );
        topicLink->SetParentSide( this );
        topicLink->SetTopic( topic );
        topicLink->ApplySettings( pubSubSideSettings );
        topicLink->SetClientFeatures( m_clientFeatures );
        topicLink->SetPubsubBookmarkPersistence( pubsubBookmarkPersistence );
        topicLink->SetPubsubBookmarkNamespace( m_bookmarkNamespace );
        topicLink->SetJournal( topic->GetJournal() );

        // Set the pulse generator last because we dont want to still be configuring the topic link
        // while the associated thread already starts running updates on the same
        topicLink->SetPulseGenerator( pulseGenerator );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::GetLatestBookmark( const CPubSubClientTopic& topic ,
                                      CPubSubBookmark& bookmark       )
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( m_pubsubBookmarkPersistence.IsNULL() || m_pubsubClient.IsNULL() )
        return false;

    return m_pubsubBookmarkPersistence->GetLatestBookmark( m_bookmarkNamespace, *m_pubsubClient.GetPointerAlways(), topic, bookmark );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::ConnectPubSubClientTopic( CPubSubClientTopic& topic                              ,
                                             const CPubSubClientFeatures& clientFeatures            ,
                                             const CPubSubSideChannelSettingsPtr pubSubSideSettings ,
                                             bool reset                                             )
{GUCEF_TRACE;

    if ( topic.InitializeConnectivity( reset ) )
    {
        CPubSubClientTopicConfigPtr topicConfig = m_pubsubClient->GetTopicConfig( topic.GetTopicName() );
        if ( topicConfig.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):ConnectPubSubClientTopic: config for topic is not available" );
            return false;
        }

        // do we even need to do anything?
        if ( !reset && ( topic.IsConnected() && topic.IsHealthy() ) )
        {
            if ( !topicConfig->needSubscribeSupport )
            {
                // No subscription needed so merely having a connection is good enough
                return true;
            }
            else
            {
                // We need to also be actively subscribed if we need a subscription
                if ( topic.IsSubscribed() )
                    return true;
            }
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConnectPubSubClientTopic: Successfully requested connectivity initialization for topic \"" + topic.GetTopicName() + "\". Proceeding" );

        // We use the 'desired' feature to also drive whether we actually subscribe at this point
        // saves us an extra setting

        if ( topicConfig->needSubscribeSupport )
        {
            // The method of subscription depends on the supported feature set
            bool subscribeSuccess = false;
            if ( !clientFeatures.supportsBookmarkingConcept ) // We have no control bookmark wise with this backend, just subscribe and hope for the best
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):ConnectPubSubClientTopic: Bookmarking concept is not supported by the backend, we will attempt to subscribe as-is" );

                subscribeSuccess = topic.Subscribe();
            }
            else
            if ( clientFeatures.supportsServerSideBookmarkPersistance ) // first preference is always backend managed bookmarking if available
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):ConnectPubSubClientTopic: Bookmarking concept is natively supported and managed by the backend independently and we will attempt to subscribe as such" );

                subscribeSuccess = topic.Subscribe();
            }
            else
            {
                // bookmarks are supported but they rely on client-side persistance
                // we will need to obtain said bookmark
                CPubSubBookmark bookmark;
                if ( GetLatestBookmark( topic, bookmark ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):ConnectPubSubClientTopic: Bookmarking concept is supported by the backend via a client-side bookmark. Bookmark type=" + CORE::ToString( bookmark.GetBookmarkType() ) + ". Bookmark=" + bookmark.GetBookmarkData().AsString() );

                    subscribeSuccess = topic.SubscribeStartingAtBookmark( bookmark );
                }
                else
                {
                    // GUCEF_ERROR_LOG
                    // This is not fully supported yet, make it a non-error log statement for now
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):ConnectPubSubClientTopic: Bookmarking concept is supported by the backend via a client-side message index marker but we failed at obtaining the last used message index" );

                    if ( pubSubSideSettings->subscribeWithoutBookmarkIfNoneIsPersisted )
                    {
                        subscribeSuccess = topic.Subscribe();
                        if ( !subscribeSuccess )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                                "):ConnectPubSubClientTopic: Also unable to subscribe using the default bookmark as a fallback" );
                            return false;
                        }
                    }
                    else
                        return false;
                }
            }

            if ( !subscribeSuccess )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):ConnectPubSubClient: Failed to subscribe to topic: " + topic.GetTopicName() );
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::DetachFromClient( CPubSubClientPtr pubsubClient )
{GUCEF_TRACE;

    if ( pubsubClient.IsNULL() )
        return;
    if ( m_pubsubClient != pubsubClient )
        return;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
        "):DetachFromClient: Detaching from underlying pubsub client implementation of type \"" + pubsubClient->GetType() + "\" for side with id " +
        GetSideId() );

    // If we already had in-flight messages and the like but due to some reason (bad state?) we have to swap out the
    // underlying implementation, we need to take care to put things in a defined state, not soley dependent on the timing of the swap

    // guard against the list changing while we are going through it due to auto cleanup actions
    TopicPtrMap topicListCopy = m_topicPtrs;

    TopicPtrMap::iterator t = topicListCopy.begin();
    while ( t != topicListCopy.end() )
    {
        TopicLinkPtr& topicLink = (*t).second;
        topicLink->DetachFromTopic();
        ++t;
    }

    // get rid of out client link
    m_pubsubClient.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::PerformPubSubClientSetup( bool hardReset )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CORE::CObserverNotificationHold hold( this );

    if ( hardReset )
    {
        if ( !DisconnectPubSubClient( hardReset ) )
            return false;
    }

    CPubSubClientConfig& pubSubConfig = m_sideSettings->pubsubClientConfig;
    CPubSubBookmarkPersistenceConfig& pubsubBookmarkPersistenceConfig = m_sideSettings->pubsubBookmarkPersistenceConfig;

    // Set the bookmark namespace:
    // Depending on the context in which pubsub is used you'd want a different namespace
    // In this case we need to take care to account for the channel and side relationship to provide a unique storage area or rely on config
    if ( !m_sideSettings->pubsubBookmarkPersistenceConfig.bookmarkNamespace.IsNULLOrEmpty() )
        m_bookmarkNamespace = m_sideSettings->pubsubBookmarkPersistenceConfig.bookmarkNamespace;
    else
        m_bookmarkNamespace = m_sideSettings->pubsubIdPrefix + '.' + m_sideId;

    // Set the pubsub ID prefix automatically if so desired:
    // Depending on the context in which pubsub is used you'd want a different namespace
    // In this case we need to take care to account for the channel and side relationship to provide a unique storage area / namespace / prefix or rely on config
    if ( pubSubConfig.pubsubIdPrefix.IsNULLOrEmpty() || pubSubConfig.pubsubIdPrefix == "{auto}"  )
        pubSubConfig.pubsubIdPrefix = m_sideSettings->pubsubIdPrefix + '.' + m_sideId;

    bool clientSetupWasNeeded = false;
    if ( hardReset || m_pubsubClient.IsNULL() )
    {
        // First decouple from the current client if any, including at the topic level
        if ( !m_pubsubClient.IsNULL() )
        {
            DetachFromClient( m_pubsubClient );
        }

        // Create and configure the pub-sub client
        pubSubConfig.pulseGenerator = GetPulseGenerator();
        pubSubConfig.metricsPrefix = m_sideSettings->metricsPrefix;

        if ( pubSubConfig.journalConfig.useJournal )
        {
            if ( hardReset || pubSubConfig.journal.IsNULL() )
            {
                pubSubConfig.journal = CPubSubGlobal::Instance()->GetPubSubJournalFactory().Create( pubSubConfig.journalConfig.journalType, pubSubConfig.journalConfig );
                if ( pubSubConfig.journal.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_VERY_IMPORTANT, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PerformPubSubClientSetup: Failed to create a pub-sub journal of type \"" + pubSubConfig.journalConfig.journalType + "\". No journaling capability" );
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                        "):PerformPubSubClientSetup: Created a pub-sub journal of type \"" + pubSubConfig.journalConfig.journalType + "\" for side with id " +
                        GetSideId() );
                }
            }
        }

        m_pubsubClient = CPubSubGlobal::Instance()->GetPubSubClientFactory().Create( pubSubConfig.pubsubClientType, pubSubConfig );

        if ( m_pubsubClient.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):PerformPubSubClientSetup: Failed to create a pub-sub client of type \"" + pubSubConfig.pubsubClientType + "\". Cannot proceed" );
            return false;
        }

        // Link the client back to this object
        // This allows getting all the way from:
        //      a message -> a topic -> a client -> a pubsub side
        m_pubsubClient->SetOpaqueUserData( this );
        m_pubsubClient->SetPulseGenerator( GetPulseGenerator() );

        clientSetupWasNeeded = true;
    }

    // Refresh our client features cache
    if ( !m_pubsubClient->GetSupportedFeatures( m_clientFeatures ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):PerformPubSubClientSetup: Failed to obtain supported a pub-sub client supported features" );
        return false;
    }

    if ( !m_clientFeatures.supportsAutoReconnect )
    {
        m_pubsubClientReconnectTimer.SetInterval( pubSubConfig.reconnectDelayInMs );
    }

    if ( clientSetupWasNeeded )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):PerformPubSubClientSetup: Setup completed for pub-sub client of type \"" + pubSubConfig.pubsubClientType + "\" for side with id " +
            GetSideId() );

        // First tell folks there is a new underlying client
        NotifyObservers( PubSubClientInstantiationEvent, &m_pubsubClient );

        // Now hook up ourselves to event messages from said underlying client
        // this may cause us to right away start responding to event messages by sending out our own
        // hence the sequencing with the above
        RegisterPubSubClientEventHandlers( m_pubsubClient );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::ConnectPubSubClient( bool reset )
{GUCEF_TRACE;

    if ( !reset && IsConnectedAndSubscribedAsNeeded() )
        return true;

    MT::CObjectScopeLock lock( this );
    CORE::CObserverNotificationHold hold( this );

    // Make sure setup was completed before connecting
    if ( !PerformPubSubClientSetup( reset ) )
        return false;

    if ( !IsPubSubClientInfraReadyToConnect() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConnectPubSubClient: Deferring pubsub client connect awaiting prereqs" );

        m_pubsubClientReconnectTimer.SetEnabled( true );
        return false;
    }

    CPubSubClientConfig& pubSubConfig = m_sideSettings->pubsubClientConfig;
    CPubSubBookmarkPersistenceConfig& pubsubBookmarkPersistenceConfig = m_sideSettings->pubsubBookmarkPersistenceConfig;

    // Whether we need to track successfull message handoff (garanteed handling) depends on various factors outside the scope of any one side
    // as such we need to ask the overarching infra to come up with a conclusion on this need
    // We will cache the outcome as a side local setting to negate locking needs
    m_sideSettings->needToTrackInFlightPublishedMsgsForAck = m_sideSettings->needToTrackInFlightPublishedMsgsForAckDefault;
    if ( GUCEF_NULL != m_flowRouter )
        m_sideSettings->needToTrackInFlightPublishedMsgsForAck = m_flowRouter->IsTrackingInFlightPublishedMsgsForAcksNeeded( this );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
        "):ConnectPubSubClient: needToTrackInFlightPublishedMsgsForAck=" + CORE::ToString( m_sideSettings->needToTrackInFlightPublishedMsgsForAck ) );

    if ( m_pubsubBookmarkPersistence.IsNULL() || reset )
    {
        // Create and configure the pub-sub bookmark persistence
        m_pubsubBookmarkPersistence = CPubSubGlobal::Instance()->GetPubSubBookmarkPersistenceFactory().Create( pubsubBookmarkPersistenceConfig.bookmarkPersistenceType, pubsubBookmarkPersistenceConfig );

        if ( m_pubsubBookmarkPersistence.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):ConnectPubSubClient: Failed to create bookmark persistance access of type \"" + pubsubBookmarkPersistenceConfig.bookmarkPersistenceType + "\". Cannot proceed" );
            return false;
        }
    }

    if ( !m_pubsubClient->Connect( reset ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):ConnectPubSubClient: Failed to connect the pub-sub client" );
        return false;
    }

    // Now create and configure the pub-sub client's topics

    if ( reset )
        m_topicPtrs.clear();

    // create a copy of the topic config vector to avoid issues with the vector changing while we are iterating through it
    CPubSubClientConfig::TPubSubClientTopicConfigPtrVector topics( pubSubConfig.topics );
    CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = topics.begin();
    while ( i != topics.end() )
    {
        CPubSubClient::PubSubClientTopicSet topicAccess;
        if ( m_pubsubClient->GetOrCreateMultiTopicAccess( (*i), topicAccess ) )
        {
            CPubSubClient::PubSubClientTopicSet::iterator a = topicAccess.begin();
            while ( a != topicAccess.end() )
            {
                CPubSubClientTopicBasicPtr topic = (*a);
                if ( topic.IsNULL() )
                {
                    if ( !(*i)->isOptional )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                            "):ConnectPubSubClient: Failed to create a pub-sub client topic access for topic \"" + (*i)->topicName + "\". Cannot proceed" );
                        DisconnectPubSubClient( true );
                        return false;
                    }
                    else
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                            "):ConnectPubSubClient: Unable to create a pub-sub client topic access for optional topic \"" + (*i)->topicName + "\". Proceeding" );
                    }
                }
                else
                {
                    ConfigureTopicLink( m_sideSettings, topic, reset );
                }
                ++a;
            }
        }
        else
        {
            if ( !(*i)->isOptional )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):ConnectPubSubClient: Failed to create a pub-sub client topic access for topic \"" + (*i)->topicName + "\". Cannot proceed" );
                DisconnectPubSubClient( true );
                return false;
            }
            else
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):ConnectPubSubClient: Unable to create a pub-sub client topic access for optional topic \"" + (*i)->topicName + "\". Proceeding" );
            }
        }

        ++i;
    }

    bool totalTopicConnectSuccess = true;
    TopicPtrMap::iterator t = m_topicPtrs.begin();
    while ( t != m_topicPtrs.end() )
    {
        TopicLinkPtr& topicLink = (*t).second;
        CPubSubClientTopicBasicPtr topic = topicLink->GetTopic();

        totalTopicConnectSuccess = ConnectPubSubClientTopic( *topic            ,
                                                             m_clientFeatures  ,
                                                             m_sideSettings    ,
                                                             reset             ) && totalTopicConnectSuccess;

        ++t;
    }
    return totalTopicConnectSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::IsPubSubClientInfraReadyToConnect( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( !m_pubsubBookmarkPersistence.IsNULL() )
    {
        // if we are using bookmark peristence it should be initialized before
        // we use the client because we will need to source the bookmarks for the client
        if ( !m_pubsubBookmarkPersistence->IsInitialized() )
            return false;
    }

    if ( !m_pubsubClient.IsNULL() )
    {
        if ( !m_pubsubClient->IsInitialized() )
            return false;
    }
    else
    {
        // we dont even have a client to connect
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::IsConnectedAndSubscribedAsNeeded( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( !m_pubsubBookmarkPersistence.IsNULL() )
    {
        // if we are using bookmark peristence it's connected state counts as part of the aggregate connected state
        // because we will need to source the bookmarks for the client
        if ( !m_pubsubBookmarkPersistence->IsConnected() )
            return false;
    }
    if ( !m_pubsubClient.IsNULL() )
    {
        if ( m_pubsubClient->IsConnectedAndSubscribedAsNeeded() )
            return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( !m_pubsubBookmarkPersistence.IsNULL() )
    {
        // if we are using bookmark peristence it's connected state counts as part of the aggregate connected state
        // because we will need to source the bookmarks for the client
        if ( !m_pubsubBookmarkPersistence->IsConnected() )
            return false;
    }
    if ( !m_pubsubClient.IsNULL() )
    {
        if ( m_pubsubClient->IsConnected() )
            return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // IMPORTANT: For thread safety we need to capture the ID of the thread running this side (consumer)
    //            We allow for running sides in their own dedicated thread or in the channel thread depending on config
    //            to tune the performance trade-offs based on the needs. This does mean we need to keep in mind a varried threading model
    m_threadIdOfSide = MT::GetCurrentTaskID();


    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    CPubSubClientConfig& pubSubConfig = m_sideSettings->pubsubClientConfig;

    m_pubsubClientReconnectTimer.SetPulseGenerator( pulseGenerator );

    m_metricsTimer.SetInterval( m_sideSettings->metricsIntervalInMs );
    m_metricsTimer.SetPulseGenerator( pulseGenerator );
    m_metricsTimer.SetEnabled( m_sideSettings->collectMetrics );

    if ( m_sideSettings->performPubSubInDedicatedThread )
    {
        // Set the minimum number of cycles we will go full speed if a single cycle was not enough to handle
        // all the processing. This will cause a bypass of CPU yielding if/when the situation arises.
        // In such a case the thread will run at max speed for a least the below set nr of cycles.
        GetPulseGenerator()->RequestPulsesPerImmediatePulseRequest( m_sideSettings->ticketRefillOnBusyCycle );

        // Default smallest pulse delta at 25ms
        GetPulseGenerator()->RequestPeriodicPulses( this, 25 );

        if ( m_sideSettings->applyThreadCpuAffinity )
        {
            if ( SetCpuAffinityByCpuId( m_sideSettings->cpuAffinityForPubSubThread ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnTaskStart: Successfully set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_sideSettings->cpuAffinityForPubSubThread ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnTaskStart: Failed to set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_sideSettings->cpuAffinityForPubSubThread ) +
                    ". Proceeding without affinity");
            }
        }
    }

    if ( m_connectOnTaskStart )
    {
        if ( IsPubSubClientInfraReadyToConnect() )
        {
            if ( !ConnectPubSubClient( false ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                    "):OnTaskStart: Failed initial connection attempt on task start, will rely on auto-reconnect" );
                m_pubsubClientReconnectTimer.SetEnabled( true );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
                "):OnTaskStart: Deferring pubsub client connect awaiting prereqs" );

            m_pubsubClientReconnectTimer.SetEnabled( true );
        }
    }

    RegisterEventHandlers();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::IsRunningInDedicatedThread( void ) const
{GUCEF_TRACE;

    return m_sideSettings->performPubSubInDedicatedThread;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::SetPerformConnectOnTaskStart( bool performConnectOnStart )
{GUCEF_TRACE;

    m_connectOnTaskStart = performConnectOnStart;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    ProcessMailbox();

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTaskEnding( CORE::CTaskPtr task ,
                                 bool willBeForced   )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnding( task, willBeForced );

    if ( willBeForced )
    {
        // reduce memory leaks
        // Mailboxes of messages will have the largest expected memory footprints but also their own locks
        // as such its safe to wipe those here

        m_broadcastMailbox.Clear();

        try
        {
            // Its possible the iterator becomes invalid here, its best effort
            TopicPtrMap::iterator i = m_topicPtrs.begin();
            while ( i != m_topicPtrs.end() )
            {
                (*i).second->Clear();
                ++i;
            }
        }
        catch ( const std::exception& )
        {
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientSide::OnTaskEnded( CORE::CTaskPtr task ,
                                bool wasForced      )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::LoadConfig( const CPubSubSideChannelSettingsPtr sideSettings )
{GUCEF_TRACE;

    if ( sideSettings.IsNULL() )
        return false;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
        "):LoadConfig: Loading new side settings for side: " + m_sideId );

    // (re)loading the side settings could have many repercussions for the pubsub client it uses
    // as such we get rid of the client if we need to (re)load the config
    MT::CObjectScopeLock lock( this );
    if ( DisconnectPubSubClient( true ) )
    {
        m_sideSettings = CPubSubSideChannelSettings::CreateSharedObjWithParam( *sideSettings.GetPointerAlways() );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientSide(" + CORE::ToString( this ) +
            "):LoadConfig: New side settings for side \"" + m_sideId + "\": " + CORE::ToString( m_sideSettings.GetPointerAlways() ) );

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CPubSubSideChannelSettingsPtr
CPubSubClientSide::GetSideSettings( void ) const
{GUCEF_TRACE;

    return m_sideSettings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientSide::GetSideId( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );
    return m_sideId;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientSide::GetCurrentTopicNames( CORE::CString::StringSet& topicNames ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwdataLock );

    if ( !m_sideSettings.IsNULL() )
    {
        CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::const_iterator t = m_sideSettings->pubsubClientConfig.topics.begin();
        while ( t != m_sideSettings->pubsubClientConfig.topics.end() )
        {
            CPubSubClientTopicConfigPtr topicConfig( (*t) );
            topicNames.insert( topicConfig->topicName );
            ++t;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CPubSubClientSide::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubClientSide";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubClientSide::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.WriterStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubClientSide::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.WriterStop() );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubClientSide::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.ReaderStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CPubSubClientSide::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwdataLock.ReaderStop() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


