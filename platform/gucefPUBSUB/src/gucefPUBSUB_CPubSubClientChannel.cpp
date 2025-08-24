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

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#include "gucefPUBSUB_CPubSubClientChannel.h"

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

#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000

const CORE::CEvent CPubSubClientChannel::HealthStatusChangeEvent = "GUCEF::PUBSUB::CPubSubClientChannel::HealthStatusChangeEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void 
CPubSubClientChannel::RegisterEvents( void )
{GUCEF_TRACE;

    HealthStatusChangeEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::CPubSubClientChannel( void )
    : CORE::CTaskConsumer()
    , m_sides()
    , m_flowRouter()
    , m_isInitialized( false )
    , m_globalConfigLoadCompleted( false )
    , m_metricsTimer( CORE::PulseGeneratorPtr(), 1000 )
    , m_isHealthy( true )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::~CPubSubClientChannel()
{GUCEF_TRACE;

    Clear();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;
    
    TEventCallback callback( this, &CPubSubClientChannel::OnGlobalConfigLoadStarted );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetConfigStore()  ,
                 CORE::CConfigStore::GlobalConfigLoadStartingEvent ,
                 callback                                          );
    TEventCallback callback2( this, &CPubSubClientChannel::OnGlobalConfigLoadCompleted );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetConfigStore()   ,
                 CORE::CConfigStore::GlobalConfigLoadCompletedEvent ,
                 callback2                                          );
    m_globalConfigLoadCompleted = CORE::CCoreGlobal::Instance()->GetConfigStore().IsGlobalConfigLoaded(); 

    m_metricsTimer.SetPulseGenerator( GetPulseGenerator() );
    TEventCallback callback3( this, &CPubSubClientChannel::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback3                      );    
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::RegisterSideEventHandlers( CPubSubClientSidePtr side )
{GUCEF_TRACE;
    
    TEventCallback callback( this, &CPubSubClientChannel::OnClientSideHealthStatusChanged );
    SubscribeTo( side.GetPointerAlways()                    ,
                 CPubSubClientSide::HealthStatusChangeEvent ,
                 callback                                   );
    m_isHealthy = m_isHealthy && side->IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::Clear( void )
{GUCEF_TRACE;

    // First stop all the threads for sides that have their own
    TPubSubClientSidePtrVector::iterator i = m_sides.begin();
    while ( i != m_sides.end() )
    {
        CPubSubClientSidePtr side = (*i);
        CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();
        if ( sideSettings->performPubSubInDedicatedThread )
        {
            side->RequestTaskToStop( true );
        }
        else
        {
            // The side is linked to our task delegator
            // lets make sure we decouple it
            side->SetTaskDelegator( TTaskDelegatorBasicPtr() );
        }
        UnsubscribeFrom( side.GetPointerAlways() );
        ++i;
    }

    // the flow router has a map with shared pointers to sides so we should clear that first
    m_flowRouter.ClearRoutes();

    // Now get rid of the actual side providers
    m_sides.clear();
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    
    // Aggregate the health status across all sides
    bool fullyHealthy = true;
    try
    {
        TPubSubClientSidePtrVector::const_iterator i = m_sides.begin();
        while ( i != m_sides.end() )
        {
            const CPubSubClientSidePtr& side = (*i);
            if ( !side.IsNULL() && !side->IsHealthy() )
            {
                fullyHealthy = false;
                break;
            }
            ++i;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel:IsHealthy: std exception caught on channel " + CORE::ToString( m_channelSettings.channelId ) 
            + " while checking for side health. what=" + CORE::ToString( e.what() ) );
        fullyHealthy = false;
    }

    // Notify if there was a change in status
    if ( fullyHealthy != m_isHealthy )
    {
        m_isHealthy = fullyHealthy;        

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel:IsHealthy: overall health is now Ok for channel with id " + CORE::ToString( m_channelSettings.channelId ) );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel:IsHealthy: overall health status is now unhealthy for channel with id " + CORE::ToString( m_channelSettings.channelId ) );         
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( fullyHealthy ); 
        NotifyObservers( HealthStatusChangeEvent, &eData );         
    }
    
    return fullyHealthy;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubClientChannel::InitializeChannel( bool force )
{GUCEF_TRACE;

    if ( m_isInitialized && !force )
        return true;
    
    if ( !m_globalConfigLoadCompleted )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubClientChannel:InitializeChannel: Deferring initialization because global config loading has not yet been completed" );
        return false;
    }
    
    // In case we are retrying we might have state from a previous run
    // clear it first
    Clear();

    m_metricsTimer.SetInterval( m_channelSettings.metricsIntervalInMs );
    m_metricsTimer.SetEnabled( m_channelSettings.collectMetrics );

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "PubSubClientChannel(" + CORE::ToString( this ) + ")", GetPulseGenerator(), true );

    // Create a side for every side config entry
    CPubSubChannelSettings::TStringToPubSubSideChannelSettingsMap::const_iterator c = m_channelSettings.pubSubSideChannelSettingsMap.begin();
    while ( c != m_channelSettings.pubSubSideChannelSettingsMap.end() )
    {
        const CORE::CString& sideId = (*c).first;
        CPubSubSideChannelSettingsPtr sideSettings = (*c).second;
        if ( sideSettings->pubsubIdPrefix.IsNULLOrEmpty() )
            sideSettings->pubsubIdPrefix = CORE::ToString( m_channelSettings.channelId );

        CPubSubClientSidePtr side( GUCEF_NEW CPubSubClientSide( sideId, &m_flowRouter ) );        
        if ( !side->LoadConfig( sideSettings ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Aborting because side with id " + sideId + " failed LoadConfig" );
            return false;
        }

        // We want the flow router to control connectivity
        // start off with everyting disconnected
        side->SetPerformConnectOnTaskStart( false );
        
        // Ensure pulse generator access ahead of client setup        
        if ( side->IsRunningInDedicatedThread() )
        {            
            // Perform task setup
            // This causes a thread delegator to be associated and as such a pulse generator made accessible
            CORE::CFutureResult result = threadPool->SetupSingularTask( side );
            if ( result.HasNoFuture() )
            {
                // As a fallback we support trying run as part of the channel thread instead
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Failed to setup dedicated thread for side with id " + side->GetSideId() + ". Falling back to using main channel thread" );
                CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();
                sideSettings->performPubSubInDedicatedThread = false;
                side->SetTaskDelegator( GetTaskDelegator() );
            }
        }
        else
        {
            // run as part of the channel thread
            // we need to make the channel's delegator available to correctly link up 
            side->SetTaskDelegator( GetTaskDelegator() );
        }

        if ( !side->PerformPubSubClientSetup() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Aborting because side with id " + sideId + " failed to setup pubsub client" );
            return false;
        }

        m_sides.push_back( side );
        RegisterSideEventHandlers( side );
        ++c;
    }

    // Hook up the flow router to the channel's pulse generator thus sharing the thread
    m_flowRouter.SetPulseGenerator( GetPulseGenerator() );
    
    // Build the flow router's network based on available sides
    if ( !m_flowRouter.BuildRoutes( m_channelSettings.flowRouterConfig ,
                                    m_sides                            ,
                                    true                               ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Aborting because we failed to build the flow router's routes" );
        return false;
    }

    // Now get the sides themselves started to kick things off
    TPubSubClientSidePtrVector::iterator i = m_sides.begin();
    while ( i != m_sides.end() )
    {
        CPubSubClientSidePtr& side = (*i);
        if ( !side->IsRunningInDedicatedThread() )
        {
            if ( !side->OnTaskStart( CORE::CTaskPtr() ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Aborting because non-dedicated-thread side with id " + side->GetSideId() + " failed its own OnTaskStart" );
                return false;
            }
        }
        else
        {
            CORE::CFutureResult result = threadPool->StartTaskWithConsumer( side );
            if ( result.HasNoFuture() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Failed to start dedicated thread for other side. Falling back to a single thread" );
                
                // As a fallback we support trying run as part of the channel thread instead

                CPubSubSideChannelSettingsPtr sideSettings = side->GetSideSettings();
                sideSettings->performPubSubInDedicatedThread = false;
                side->SetTaskDelegator( GetTaskDelegator() );

                if ( !side->OnTaskStart( CORE::CTaskPtr() ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:InitializeChannel: Aborting because non-dedicated-thread (fallback mode) side with id " + side->GetSideId() + " failed its own OnTaskStart" );
                    return false;
                }
            }
            else
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel:InitializeChannel: Successfully requested the launch of a dedicated thread for side with id " + side->GetSideId() );
            }
        }

        ++i;
    }

    m_isInitialized = true;
    return true;

}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    RegisterEventHandlers();

    InitializeChannel( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    bool allDone = false;
    TPubSubClientSidePtrVector::iterator i = m_sides.begin();
    while ( i != m_sides.end() )
    {
        CPubSubClientSidePtr& side = (*i);
        if ( !side->IsRunningInDedicatedThread() )
        {
            // This side does not have its own thread
            // we will let it use the channel's thread
            allDone = side->OnTaskCycle( task ) || allDone;
        }

        ++i;
    }

    return allDone;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnTaskEnding( CORE::CTaskPtr task ,
                                    bool willBeForced   )
{GUCEF_TRACE;

    TPubSubClientSidePtrVector::iterator i = m_sides.begin();
    while ( i != m_sides.end() )
    {
        CPubSubClientSidePtr& side = (*i);
        if ( !side->IsRunningInDedicatedThread() )
        {
            // This side does not have its own thread
            // we have let it use the channel's thread so its fate was tied wrt OnTaskEnded timing
            side->OnTaskEnding( task, willBeForced );
        }
        else
        {
            // Since we are the ones that launched the dedicated thread for this sides we should also ask
            // to have it cleaned up when we are shutting down the main channel thread
            CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
            if ( !threadPool->RequestTaskToStop( side.StaticCast< CORE::CTaskConsumer >(), false ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel:OnTaskEnding: Failed to request the dedicated thread for side " + side->GetSideId() + " to stop" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel:OnTaskEnding: Successfully requested the dedicated thread for the side " + side->GetSideId() + " to stop" );
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnTaskEnded( CORE::CTaskPtr task ,
                                   bool wasForced      )
{GUCEF_TRACE;  

    TPubSubClientSidePtrVector::iterator i = m_sides.begin();
    while ( i != m_sides.end() )
    {
        CPubSubClientSidePtr& side = (*i);
        if ( !side->IsRunningInDedicatedThread() )
        {
            // This side does not have its own thread
            // we will have let it use the channel's thread so its fate was tied wrt OnTaskEnded timing
            side->OnTaskEnded( task, wasForced );
        }
        ++i;
    }

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnGlobalConfigLoadStarted( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // We should wait for all platform systems to initialize since this is middleware and 
    // we dont know what platform systems the various backends depend on so we cannot get a head start
    // without potentially causing issues
    m_globalConfigLoadCompleted = false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnGlobalConfigLoadCompleted( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventId  ,
                                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // We should wait for all platform systems to initialize since this is middleware and 
    // we dont know what platform systems the various backends depend on so we cannot get a head start
    // without potentially causing issues
    m_globalConfigLoadCompleted = true;
    InitializeChannel( false );
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnClientSideHealthStatusChanged( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPubSubClientSide* side = static_cast< CPubSubClientSide* >( notifier ); 
    CPubSubClientSide::THealthStatusChangeEventData* healthStatus = static_cast< CPubSubClientSide::THealthStatusChangeEventData* >( eventData );

    if ( GUCEF_NULL == side || GUCEF_NULL == healthStatus )
        return;

    // things could have changed since the event message was sent, we reevaluate etc
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientChannel::GetType( void ) const
{GUCEF_TRACE;

    return "PubSubClientChannel";
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::LoadConfig( const CPubSubChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    return m_flowRouter.LoadConfig( m_channelSettings.flowRouterConfig );
}

/*-------------------------------------------------------------------------*/

const CPubSubChannelSettings&
CPubSubClientChannel::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPubSubClientChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubClientChannel";
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


