/*
 *  pubsub2storage: service which transfers between pubsub and storage
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

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "pubsub2storage.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <winsock2.h>
#endif

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000
#define GUCEF_DEFAULT_PUBSUB_RECONNECT_DELAY_IN_MS                  100
#define GUCEF_DEFAULT_MINIMAL_PUBSUB_BLOCK_STORAGE_SIZE_IN_BYTES    (1024*1024*50)// 50MB
#define GUCEF_DEFAULT_MAXIMAL_PUBSUB_BLOCK_STORE_GROW_DELAY_IN_MS   (1000*60*5)   // 5mins
#define GUCEF_DEFAULT_DECODE_GROWTH_RATIO_EXPECTATION               6.0f

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ChannelSettings::ChannelSettings( void )
    : CORE::CIConfigurable()
    , pubsubClientConfig()
    , pubsubBinarySerializerOptions()
    , desiredMinimalSerializedBlockSize( GUCEF_DEFAULT_MINIMAL_PUBSUB_BLOCK_STORAGE_SIZE_IN_BYTES )
    , desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs( GUCEF_DEFAULT_MAXIMAL_PUBSUB_BLOCK_STORE_GROW_DELAY_IN_MS )
    , vfsStorageRootPath()
    , vfsFileExtention()
    , encodeCodecFamily()
    , encodeCodecName()
    , decodeCodecFamily()
    , decodeCodecName()
    , channelId( -1 )
    , ticketRefillOnBusyCycle( GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE )
    , performPubSubInDedicatedThread( true )
    , applyThreadCpuAffinity( false )
    , cpuAffinityForDedicatedPubSubThread( 0 )
    , cpuAffinityForMainChannelThread( 0 )
    , collectMetrics( true )
    , mode( CHANNELMODE_PUBSUB_TO_STORAGE )
    , subscribeWithoutBookmarkIfNoneIsPersisted( true )
    , autoPushAfterStartupIfStorageToPubSub( true )
    , youngestStoragePubSubMsgFileToLoad( CORE::CDateTime::FutureMax )
    , oldestStoragePubSubMsgFileToLoad( CORE::CDateTime::PastMax )  
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ChannelSettings::ChannelSettings( const ChannelSettings& src )
    : pubsubClientConfig( src.pubsubClientConfig )
    , pubsubBinarySerializerOptions( src.pubsubBinarySerializerOptions )
    , desiredMinimalSerializedBlockSize( src.desiredMinimalSerializedBlockSize )
    , desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs( src.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs )
    , vfsStorageRootPath( src.vfsStorageRootPath )
    , vfsFileExtention( src.vfsFileExtention )
    , encodeCodecFamily( src.encodeCodecFamily )
    , encodeCodecName( src.encodeCodecName )
    , decodeCodecFamily( src.decodeCodecFamily )
    , decodeCodecName( src.decodeCodecName )
    , channelId( src.channelId )
    , ticketRefillOnBusyCycle( src.ticketRefillOnBusyCycle )
    , performPubSubInDedicatedThread( src.performPubSubInDedicatedThread )
    , applyThreadCpuAffinity( src.applyThreadCpuAffinity )
    , cpuAffinityForDedicatedPubSubThread( src.cpuAffinityForDedicatedPubSubThread )
    , cpuAffinityForMainChannelThread( src.cpuAffinityForMainChannelThread )
    , collectMetrics( src.collectMetrics )
    , mode( src.mode )
    , subscribeWithoutBookmarkIfNoneIsPersisted( src.subscribeWithoutBookmarkIfNoneIsPersisted )
    , autoPushAfterStartupIfStorageToPubSub( src.autoPushAfterStartupIfStorageToPubSub )
    , youngestStoragePubSubMsgFileToLoad( src.youngestStoragePubSubMsgFileToLoad )
    , oldestStoragePubSubMsgFileToLoad( src.oldestStoragePubSubMsgFileToLoad )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ChannelSettings&
ChannelSettings::operator=( const ChannelSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        pubsubClientConfig = src.pubsubClientConfig;
        pubsubBinarySerializerOptions = src.pubsubBinarySerializerOptions;
        desiredMinimalSerializedBlockSize = src.desiredMinimalSerializedBlockSize;
        desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs = src.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs;
        vfsStorageRootPath = src.vfsStorageRootPath;
        vfsFileExtention = src.vfsFileExtention;
        encodeCodecFamily = src.encodeCodecFamily;
        encodeCodecName = src.encodeCodecName;
        decodeCodecFamily = src.decodeCodecFamily;
        decodeCodecName = src.decodeCodecName;
        channelId = src.channelId;
        ticketRefillOnBusyCycle = src.ticketRefillOnBusyCycle;
        performPubSubInDedicatedThread = src.performPubSubInDedicatedThread;
        applyThreadCpuAffinity = src.applyThreadCpuAffinity;
        cpuAffinityForDedicatedPubSubThread = src.cpuAffinityForDedicatedPubSubThread;
        cpuAffinityForMainChannelThread = src.cpuAffinityForMainChannelThread;
        collectMetrics = src.collectMetrics;
        mode = src.mode;
        subscribeWithoutBookmarkIfNoneIsPersisted = src.subscribeWithoutBookmarkIfNoneIsPersisted;
        autoPushAfterStartupIfStorageToPubSub = src.autoPushAfterStartupIfStorageToPubSub;
        youngestStoragePubSubMsgFileToLoad = src.youngestStoragePubSubMsgFileToLoad;
        oldestStoragePubSubMsgFileToLoad = src.oldestStoragePubSubMsgFileToLoad;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
ChannelSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;
    
    tree.SetAttribute( "desiredMinimalSerializedBlockSize", desiredMinimalSerializedBlockSize );
    tree.SetAttribute( "desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs", desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs );
    tree.SetAttribute( "vfsStorageRootPath", vfsStorageRootPath );
    tree.SetAttribute( "vfsFileExtention", vfsFileExtention );
    tree.SetAttribute( "encodeCodecFamily", encodeCodecFamily );
    tree.SetAttribute( "encodeCodecName", encodeCodecName );
    tree.SetAttribute( "decodeCodecFamily", decodeCodecFamily );
    tree.SetAttribute( "decodeCodecName", decodeCodecName );
    tree.SetAttribute( "channelId", channelId );
    tree.SetAttribute( "ticketRefillOnBusyCycle", ticketRefillOnBusyCycle );
    tree.SetAttribute( "performPubSubInDedicatedThread", performPubSubInDedicatedThread );
    tree.SetAttribute( "applyThreadCpuAffinity", applyThreadCpuAffinity );
    tree.SetAttribute( "cpuAffinityForDedicatedPubSubThread", cpuAffinityForDedicatedPubSubThread );
    tree.SetAttribute( "cpuAffinityForMainChannelThread", cpuAffinityForMainChannelThread );
    tree.SetAttribute( "collectMetrics", collectMetrics );
    tree.SetAttribute( "mode", mode );
    tree.SetAttribute( "subscribeWithoutBookmarkIfNoneIsPersisted", subscribeWithoutBookmarkIfNoneIsPersisted );
    tree.SetAttribute( "autoPushAfterStartupIfStorageToPubSub", autoPushAfterStartupIfStorageToPubSub );
    tree.SetAttribute( "youngestStoragePubSubMsgFileToLoad", youngestStoragePubSubMsgFileToLoad.ToIso8601DateTimeString( true, true ) );
    tree.SetAttribute( "oldestStoragePubSubMsgFileToLoad", oldestStoragePubSubMsgFileToLoad.ToIso8601DateTimeString( true, true ) );

    CORE::CDataNode* psClientConfig = tree.Structure( "PubSubClientConfig", '/' );
    if ( !pubsubClientConfig.SaveConfig( *psClientConfig ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelSettings:SaveConfig: config is malformed, failed to save a mandatory PubSubClientConfig section" );
        return false;        
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ChannelSettings::LoadConfig( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    const CORE::CDataNode* psClientConfig = tree.Find( "PubSubClientConfig" );
    if ( GUCEF_NULL != psClientConfig )
    {
        if ( !pubsubClientConfig.LoadConfig( *psClientConfig ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelSettings:LoadConfig: config is unacceptable, failed to load mandatory PubSubClientConfig section" );
            return false;
        }

        // There is no sane default of pubsubClientType since it depends on the clients loaded into the application
        // as such this is a mandatory setting to provide
        if ( pubsubClientConfig.pubsubClientType.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelSettings:LoadConfig: config is malformed, \"pubsubClientType\" was not provided" );
            return false;
        }    

        // We are fully config driven with no programatically defined topics
        // As such the config must have yielded at least 1 topic
        if ( pubsubClientConfig.topics.empty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelSettings:LoadConfig: config is malformed, having at least one topic configured for the client section is mandatory" );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelSettings:LoadConfig: config is malformed, a PubSubClientConfig section is mandatory" );
        return false;
    }
    
    desiredMinimalSerializedBlockSize = tree.GetAttributeValueOrChildValueByName( "desiredMinimalSerializedBlockSize" ).AsUInt32( desiredMinimalSerializedBlockSize, true );
    desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs = tree.GetAttributeValueOrChildValueByName( "desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs" ).AsUInt32( desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs, true );
    vfsStorageRootPath = tree.GetAttributeValueOrChildValueByName( "vfsStorageRootPath" ).AsString( vfsStorageRootPath, true );
    vfsFileExtention = tree.GetAttributeValueOrChildValueByName( "vfsFileExtention" ).AsString( vfsFileExtention, true );
    encodeCodecFamily = tree.GetAttributeValueOrChildValueByName( "encodeCodecFamily" ).AsString( encodeCodecFamily, true );
    encodeCodecName = tree.GetAttributeValueOrChildValueByName( "encodeCodecName" ).AsString( encodeCodecName, true );
    decodeCodecFamily = tree.GetAttributeValueOrChildValueByName( "decodeCodecFamily" ).AsString( decodeCodecFamily, true );
    decodeCodecName = tree.GetAttributeValueOrChildValueByName( "decodeCodecName" ).AsString( decodeCodecName, true );
    channelId = tree.GetAttributeValueOrChildValueByName( "channelId" ).AsInt32( channelId, true );
    ticketRefillOnBusyCycle = tree.GetAttributeValueOrChildValueByName( "ticketRefillOnBusyCycle" ).AsUInt32( ticketRefillOnBusyCycle, true );
    performPubSubInDedicatedThread = tree.GetAttributeValueOrChildValueByName( "performPubSubInDedicatedThread" ).AsBool( performPubSubInDedicatedThread, true );
    applyThreadCpuAffinity = tree.GetAttributeValueOrChildValueByName( "applyThreadCpuAffinity" ).AsBool( applyThreadCpuAffinity, true );
    cpuAffinityForDedicatedPubSubThread = tree.GetAttributeValueOrChildValueByName( "cpuAffinityForDedicatedPubSubThread" ).AsUInt32( cpuAffinityForDedicatedPubSubThread, true );
    cpuAffinityForMainChannelThread = tree.GetAttributeValueOrChildValueByName( "cpuAffinityForMainChannelThread" ).AsUInt32( cpuAffinityForMainChannelThread, true );
    collectMetrics = tree.GetAttributeValueOrChildValueByName( "collectMetrics" ).AsBool( collectMetrics, true );
    mode = (TChannelMode) tree.GetAttributeValueOrChildValueByName( "mode" ).AsInt32( mode, true );
    subscribeWithoutBookmarkIfNoneIsPersisted = tree.GetAttributeValueOrChildValueByName( "subscribeWithoutBookmarkIfNoneIsPersisted" ).AsBool( subscribeWithoutBookmarkIfNoneIsPersisted, true );
    autoPushAfterStartupIfStorageToPubSub = tree.GetAttributeValueOrChildValueByName( "autoPushAfterStartupIfStorageToPubSub" ).AsBool( autoPushAfterStartupIfStorageToPubSub, true ); 
    youngestStoragePubSubMsgFileToLoad.FromIso8601DateTimeString( tree.GetAttributeValueOrChildValueByName( "youngestStoragePubSubMsgFileToLoad" ).AsString( youngestStoragePubSubMsgFileToLoad.ToIso8601DateTimeString( true, true ), true ) );
    oldestStoragePubSubMsgFileToLoad.FromIso8601DateTimeString( tree.GetAttributeValueOrChildValueByName( "oldestStoragePubSubMsgFileToLoad" ).AsString( oldestStoragePubSubMsgFileToLoad.ToIso8601DateTimeString( true, true ), true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
ChannelSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "pubsub2storage::ChannelSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr 
ChannelSettings::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    TTopicConfigVector::iterator i = pubsubClientConfig.topics.begin();
    while ( i != pubsubClientConfig.topics.end() )
    {
        if ( topicName == (*i)->topicName )
            return (*i);
    }    
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::CPubSubClientChannel( CIPubSubBookmarkPersistance* persistance )
    : CORE::CTaskConsumer()
    , m_pubsubClient()
    , m_topics()
    , m_channelSettings()
    , m_mailbox()
    , m_bulkMail()
    , m_metricsTimer( GUCEF_NULL )
    , m_pubsubClientReconnectTimer( GUCEF_NULL )
    , m_buffers( 2 )
    , m_msgReceiveBuffer( GUCEF_NULL )
    , m_lastWriteBlockCompletion()
    , m_msgOffsetIndex()
    , m_persistance( persistance )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::~CPubSubClientChannel()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::TopicLink::TopicLink( void )
    : topic()
    , currentPublishActionIds()
    , inFlightMsgs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubClientChannel::TopicLink::TopicLink( PUBSUB::CPubSubClientTopicBasicPtr t )
    : topic( t )
    , currentPublishActionIds()
    , inFlightMsgs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void 
CPubSubClientChannel::TopicLink::AddInFlightMsgs( const PUBSUB::CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                                                  const PUBSUB::CPubSubClientTopic::TIPubSubMsgSPtrVector& msgs              )
{GUCEF_TRACE;
    
    size_t max = SMALLEST( publishActionIds.size(), msgs.size() );    
    for ( size_t i=0; i<max; ++i )
        inFlightMsgs[ publishActionIds[ i ] ] = msgs[ i ];
}

/*-------------------------------------------------------------------------*/

void 
CPubSubClientChannel::TopicLink::AddInFlightMsgs( const PUBSUB::CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                                                  const PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector& msgs               )
{GUCEF_TRACE;
    
    size_t max = SMALLEST( publishActionIds.size(), msgs.size() );    
    for ( size_t i=0; i<max; ++i )
    {
        PUBSUB::CIPubSubMsg::TNoLockSharedPtr ptr( static_cast< PUBSUB::CIPubSubMsg* >( msgs[ i ]->Clone() ) );
        inFlightMsgs[ publishActionIds[ i ] ] = ptr;
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CPubSubClientChannel::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );

    if ( GUCEF_NULL != m_pubsubClientReconnectTimer )
    {
        TEventCallback callback( this, &CPubSubClientChannel::OnPubSubClientReconnectTimerCycle );
        SubscribeTo( m_pubsubClientReconnectTimer   ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::RegisterTopicEventHandlers( PUBSUB::CPubSubClientTopic& topic )
{GUCEF_TRACE;

    if ( m_channelSettings.mode == TChannelMode::CHANNELMODE_PUBSUB_TO_STORAGE )
    {
        TEventCallback callback( this, &CPubSubClientChannel::OnPubSubTopicMsgsReceived );
        SubscribeTo( &topic                                         ,
                     PUBSUB::CPubSubClientTopic::MsgsRecievedEvent ,
                     callback                                       );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubClientChannel::GetType( void ) const
{GUCEF_TRACE;

    return "PubSubClientChannel";
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnPubSubClientReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                                         const CORE::CEvent& eventId  ,
                                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // stop the timer, reconnect time itself should not count towards the reconnect interval
    m_pubsubClientReconnectTimer->SetEnabled( false );
    
    // Since the client does not support reconnects we will destructively reconnnect
    // Meaning we wipe out any previous client as we cannot rely on the client implementation
    // properly handling the state transition
    if ( DisconnectPubSubClient( true ) )
    {
        if ( ConnectPubSubClient( true ) )
            return; // no need to resume the timer
    }

    // no joy, start the timer again
    m_pubsubClientReconnectTimer->SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBufferSwap& 
CPubSubClientChannel::GetSerializedMsgBuffers( void )
{GUCEF_TRACE;

    return m_buffers;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnPubSubTopicMsgsReceived( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    
    try
    {
        const PUBSUB::CPubSubClientTopic::TMsgsRecievedEventData& receiveAction = ( *static_cast< PUBSUB::CPubSubClientTopic::TMsgsRecievedEventData* >( eventData ) );
        if ( !receiveAction.empty() )
        {                            
            PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector::const_iterator i = receiveAction.begin();
            const CORE::CDateTime& firstMsgDt = (*i)->GetMsgDateTime();

            bool firstBlock = m_lastWriteBlockCompletion == CORE::CDateTime::Empty;
            if ( firstBlock )
            {                
                m_lastWriteBlockCompletion = CORE::CDateTime::NowUTCDateTime();
            }
            if ( GUCEF_NULL == m_msgReceiveBuffer )
            {
                m_msgReceiveBuffer = m_buffers.GetNextWriterBuffer( firstMsgDt, true, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                
                CORE::UInt32 newBytesWritten = 0;
                if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeHeader( m_channelSettings.pubsubBinarySerializerOptions, 0, *m_msgReceiveBuffer, newBytesWritten ) )
                {
                    // We carry on best effort but this is really bad
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Failed to write container header at start of new pub-sub msg container" );
                }
                m_msgOffsetIndex.clear();
            }
            CORE::UInt32 bufferOffset = m_msgReceiveBuffer->GetDataSize();

            while ( i != receiveAction.end() )    
            {
                CORE::UInt32 ticks = CORE::GUCEFGetTickCount();
                CORE::UInt32 msgBytesWritten = 0;
                if ( PUBSUB::CPubSubMsgBinarySerializer::Serialize( m_channelSettings.pubsubBinarySerializerOptions, *(*i), bufferOffset, *m_msgReceiveBuffer, msgBytesWritten ) )
                {
                    m_msgOffsetIndex.push_back( bufferOffset );
                    ticks = CORE::GUCEFGetTickCount() - ticks;
                    bufferOffset += msgBytesWritten;
                    m_msgReceiveBuffer->SetDataSize( bufferOffset );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Serialized a message with serialized size " + CORE::ToString( msgBytesWritten ) + ". This took " + CORE::ToString( ticks ) + "ms" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Failed to serialize a message" );
                }
                ++i;
            }

            // Check to see if we have gathered enough data or enough time has passed to consider the current container complete
            if ( m_msgReceiveBuffer->GetDataSize() >= m_channelSettings.desiredMinimalSerializedBlockSize ||
               ( !firstBlock && m_lastWriteBlockCompletion.GetTimeDifferenceInMillisecondsToNow() >= m_channelSettings.desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs ) )
            {
                // The current container is now considered to have enough content.
                // Let's wrap things up...
                
                CORE::UInt32 newBytesWritten = 0;
                if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeFooter( m_msgOffsetIndex, bufferOffset, *m_msgReceiveBuffer, newBytesWritten ) )
                {
                    // We carry on best effort but this is really bad
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):OnPubSubTopicMsgsReceived: Failed to write container footer at end of current pub-sub msg container" );
                }

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                    "):OnPubSubTopicMsgsReceived: Completed a serialized msg data block of size " + CORE::ToString( bufferOffset ) );                
                
                m_msgReceiveBuffer = GUCEF_NULL;
                m_lastWriteBlockCompletion = CORE::CDateTime::NowUTCDateTime();
            }            
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) + "):OnPubSubTopicMsgsReceived: exception: " + CORE::CString( e.what() ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::DisconnectPubSubClient( bool destroyClient )
{GUCEF_TRACE;

    if ( m_pubsubClient.IsNULL() )
        return true;

    if ( !m_pubsubClient->Disconnect() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
            "):ConnectPubSubClient: Failed to disconnect the pub-sub client" );
        return false;                    
    }

    PUBSUB::CPubSubClientFeatures clientFeatures;
    m_pubsubClient->GetSupportedFeatures( clientFeatures );

    if ( destroyClient || !clientFeatures.supportsAutoReconnect )
    {
        m_pubsubClient.Unlink();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::ConnectPubSubClient( bool reset )
{GUCEF_TRACE;

    if ( !DisconnectPubSubClient( reset ) )
        return false;

    if ( m_pubsubClient.IsNULL() )
    {
        // Create and configure the pub-sub client
        m_channelSettings.pubsubClientConfig.pulseGenerator = GetPulseGenerator();
        m_pubsubClient = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().Create( m_channelSettings.pubsubClientConfig.pubsubClientType, m_channelSettings.pubsubClientConfig );

        if ( m_pubsubClient.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                "):OnTaskStart: Failed to create a pub-sub client of type \"" + m_channelSettings.pubsubClientConfig.pubsubClientType + "\". Cannot proceed" );        
            return false;
        }
    }

    PUBSUB::CPubSubClientFeatures clientFeatures;
    m_pubsubClient->GetSupportedFeatures( clientFeatures );
        
    if ( !clientFeatures.supportsAutoReconnect )
    {
        if ( GUCEF_NULL != m_pubsubClientReconnectTimer )
            m_pubsubClientReconnectTimer = new CORE::CTimer( GetPulseGenerator(), m_channelSettings.pubsubClientConfig.reconnectDelayInMs );
    }

    SubscribeTo( m_pubsubClient.GetPointerAlways() );
    if ( !m_pubsubClient->Connect( true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
            "):ConnectPubSubClient: Failed to connect the pub-sub client" );
        return false;
    }

    // Create and configure the pub-sub client's topics
    m_topics.clear();
    m_topics.reserve( m_channelSettings.pubsubClientConfig.topics.size() );
    ChannelSettings::TTopicConfigVector::iterator i = m_channelSettings.pubsubClientConfig.topics.begin();
    while ( i != m_channelSettings.pubsubClientConfig.topics.end() )
    {
        PUBSUB::CPubSubClientTopicBasicPtr topic = m_pubsubClient->CreateTopicAccess( (*i) );
        if ( topic.IsNULL() )
        {
            if ( !(*i)->isOptional )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                    "):ConnectPubSubClient: Failed to create a pub-sub client topic access for topic \"" + (*i)->topicName + "\". Cannot proceed" );
                return false;            
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                    "):ConnectPubSubClient: Unable to create a pub-sub client topic access for optional topic \"" + (*i)->topicName + "\". Proceeding" );
            }
        }

        RegisterTopicEventHandlers( *topic );
        m_topics.push_back( TopicLink( topic ) );
        ++i;
    }

    TopicVector::iterator t = m_topics.begin();
    while ( t != m_topics.end() )
    {
        TopicLink& topicLink = (*t);
        PUBSUB::CPubSubClientTopicBasicPtr topic = topicLink.topic;

        if ( topic->InitializeConnectivity( reset ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                "):ConnectPubSubClient: Successfully requested connectivity initialization for topic \"" + topic->GetTopicName() + "\". Proceeding" );

            // We use the 'desired' feature to also drive whether we actually subscribe at this point
            // saves us an extra setting
            PUBSUB::CPubSubClientTopicConfigPtr topicConfig = m_channelSettings.GetTopicConfig( topic->GetTopicName() );
            if ( !topicConfig.IsNULL() && topicConfig->needSubscribeSupport )
            {            
                // The method of subscription depends on the supported feature set
                bool subscribeSuccess = false;
                if ( !clientFeatures.supportsBookmarkingConcept ) // We have no control bookmark wise with this backend, just subscribe and hope for the best
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):ConnectPubSubClient: Bookmarking concept is not supported by the backend, we will attempt to subscribe as-is" );

                    subscribeSuccess = topic->Subscribe();
                }
                else
                if ( clientFeatures.supportsServerSideBookmarkPersistance ) // first preference is always backend managed bookmarking if available
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):ConnectPubSubClient: Bookmarking concept is natively supported and managed by the backend independently and we will attempt to subscribe as such" );

                    subscribeSuccess = topic->Subscribe();
                }
                else
                {
                    // bookmarks are supported but they rely on client-side persistance
                    // we will need to obtain said bookmark
                    
                    PUBSUB::CPubSubBookmark bookmark;
                    if ( !m_persistance->GetPersistedBookmark( m_channelSettings.channelId, topic->GetTopicName(), bookmark ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                            "):ConnectPubSubClient: Bookmarking concept is supported by the backend via a client-side message index marker but we failed at obtaining the last used message index" );
                        
                        if ( m_channelSettings.subscribeWithoutBookmarkIfNoneIsPersisted )
                        {
                            subscribeSuccess = topic->Subscribe();
                            if ( !subscribeSuccess )
                            {
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CPubSubClientSide(" + CORE::PointerToString( this ) +
                                    "):ConnectPubSubClient: Also unable to subscribe using the default bookmark as a fallback" );
                                return false;
                            }
                        }
                        else
                            return false;
                    }
                    else
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CPubSubClientSide(" + CORE::PointerToString( this ) +
                            "):ConnectPubSubClient: Bookmarking concept is supported by the backend via a client-side bookmark. Bookmark type=" + CORE::ToString( bookmark.GetBookmarkType() ) + ". Bookmark=" + bookmark.GetBookmarkData().AsString() );

                        subscribeSuccess = topic->SubscribeStartingAtBookmark( bookmark );
                    }
                }

                if ( !subscribeSuccess )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):ConnectPubSubClient: Failed to subscribe to topic: " + topic->GetTopicName() );
                    return false;
                }
            }
        }
        ++t;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );
    m_metricsTimer->SetEnabled( m_channelSettings.pubsubClientConfig.desiredFeatures.supportsMetrics );

    m_buffers.SetMinimalBufferSize( m_channelSettings.desiredMinimalSerializedBlockSize );
    m_msgOffsetIndex.clear();
    m_msgOffsetIndex.reserve( 1000 );
    
    if ( m_channelSettings.performPubSubInDedicatedThread )
    {
        // Set the minimum number of cycles we will go full speed if a single cycle was not enough to handle
        // all the processing. This will cause a bypass of CPU yielding if/when the situation arises.
        // In such a case the thread will run at max speed for a least the below set nr of cycles.
        GetPulseGenerator()->RequestPulsesPerImmediatePulseRequest( m_channelSettings.ticketRefillOnBusyCycle );
        
        // Default smallest pulse delta at 25ms
        GetPulseGenerator()->RequestPeriodicPulses( this, 25 ); 

        if ( m_channelSettings.applyThreadCpuAffinity )
        {
            if ( SetCpuAffinityByCpuId( m_channelSettings.cpuAffinityForDedicatedPubSubThread ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                    "):OnTaskStart: Successfully set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForDedicatedPubSubThread ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                    "):OnTaskStart: Failed to set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForDedicatedPubSubThread ) +
                    ". Proceeding without affinity");
            }
        }
    }

    if ( !ConnectPubSubClient( false ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
            "):OnTaskStart: Failed initial connection attempt on task start, will rely on auto-reconnect" );
    }

    RegisterEventHandlers();

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnStoredPubSubMsgTransmissionFailure( const CORE::CDateTime& firstMsgDt )
{GUCEF_TRACE;

    
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::TransmitNextPubSubMsgBuffer( void )
{GUCEF_TRACE;    

    CORE::CDateTime firstMsgDt;
    m_msgReceiveBuffer = m_buffers.GetNextReaderBuffer( firstMsgDt, true, 25 );
    if ( GUCEF_NULL == m_msgReceiveBuffer )
        return true; // nothing to do
        
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
        "):TransmitNextPubSubMsgBuffer: New buffer is available of " + CORE::ToString( m_msgReceiveBuffer->GetDataSize() ) + " bytes" );         

    CORE::UInt32 bytesRead = 0;
    PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex originalOffsetIndex;
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeFooter( originalOffsetIndex, *m_msgReceiveBuffer, bytesRead, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
            "):TransmitNextPubSubMsgBuffer: Failed to read container footer" );
        OnStoredPubSubMsgTransmissionFailure( firstMsgDt );
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
        "):TransmitNextPubSubMsgBuffer: Per footer the buffer contains " + CORE::ToString( originalOffsetIndex.size() ) + " messages to publish" );

    // We now link logical message objects to the data in the buffer
    CORE::UInt32 startIndexOffset = 0;
    CORE::UInt32 endIndexOffset = 0;
    bool isCorrupted = false;                        
    PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;                        
    if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::Deserialize( msgs, true, originalOffsetIndex, *m_msgReceiveBuffer, isCorrupted, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
            "):TransmitNextPubSubMsgBuffer: Failed to deserialize messages from container. According to the footer the container had " + 
            CORE::ToString( originalOffsetIndex.size() ) + " entries. isCorrupted=" + CORE::BoolToString( isCorrupted ) );
        OnStoredPubSubMsgTransmissionFailure( firstMsgDt );
        return false;
    }

    // We now have the messages in a format that allows interpretation by the pub-sub backend
    // We can now proceed with publishing all the messages to the relevant topics
    CORE::UInt32 topicsToPublishOn = 0;
    CORE::UInt32 topicsPublishedOn = 0;
    bool publishSuccess = true;
    TopicVector::iterator i = m_topics.begin();
    while ( i != m_topics.end() )
    {
        TopicLink& topicLink = (*i);
        PUBSUB::CPubSubClientTopicBasicPtr topic = topicLink.topic;

        if ( GUCEF_NULL != topic )
        {
            if ( topic->IsPublishingSupported() )
            {
                ++topicsToPublishOn;
                if ( topic->Publish( topicLink.currentPublishActionIds, msgs, true ) )
                {
                    ++topicsPublishedOn;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
                        "):TransmitNextPubSubMsgBuffer: Failed to publish messages to topic" );
                    OnStoredPubSubMsgTransmissionFailure( firstMsgDt );
                    publishSuccess = false;
                }
            }
        }
        ++i;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubClientChannel(" + CORE::PointerToString( this ) +
        "):TransmitNextPubSubMsgBuffer: Successfully published messages to " + CORE::ToString( topicsPublishedOn ) + " topics, " + 
        CORE::ToString( topicsToPublishOn ) + " topics available for publishing" );

    return publishSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;    

    if ( m_channelSettings.mode == TChannelMode::CHANNELMODE_STORAGE_TO_PUBSUB )
    {
        TransmitNextPubSubMsgBuffer();
    }

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnTaskEnding( CORE::CTaskPtr task ,
                                    bool willBeForced   )
{GUCEF_TRACE;

    m_buffers.SignalEndOfWriting();
}

/*-------------------------------------------------------------------------*/

void
CPubSubClientChannel::OnTaskEnded( CORE::CTaskPtr task ,
                                   bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    GUCEF_DELETE m_pubsubClientReconnectTimer;
    m_pubsubClientReconnectTimer = GUCEF_NULL;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubClientChannel::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    return true;
}

/*-------------------------------------------------------------------------*/

const ChannelSettings&
CPubSubClientChannel::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

CStorageChannel::CStorageChannel()
    : CORE::CTaskConsumer()
    , m_channelSettings()
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_pubsubClient( new CPubSubClientChannel( this ) )
    , m_msgReceiveBuffer( GUCEF_NULL )
    , m_vfsFilePostfix( ".vUNKNOWN.bin" )
    , m_lastPersistedMsgId()
    , m_lastPersistedMsgDt()
    , m_encodeSizeRatio( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageChannel::CStorageChannel( const CStorageChannel& src )
    : CORE::CTaskConsumer()
    , m_channelSettings( src.m_channelSettings )
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_pubsubClient( new CPubSubClientChannel( this ) )
    , m_vfsFilePostfix( src.m_vfsFilePostfix )
    , m_lastPersistedMsgId()
    , m_lastPersistedMsgDt()
    , m_encodeSizeRatio( src.m_encodeSizeRatio )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageChannel::~CStorageChannel()
{GUCEF_TRACE;

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CStorageChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback6( this, &CStorageChannel::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

CStorageChannel::StorageToPubSubRequest::StorageToPubSubRequest( void )
    : startDt()
    , endDt()
    , vfsPubSubMsgContainersToPush()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageChannel::StorageToPubSubRequest::StorageToPubSubRequest( const CORE::CDateTime& startDt, const CORE::CDateTime& endDt )
    : startDt( startDt )
    , endDt( endDt )
    , vfsPubSubMsgContainersToPush()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStorageChannel::StorageToPubSubRequest::StorageToPubSubRequest( const StorageToPubSubRequest& src )
    : startDt( src.startDt )
    , endDt( src.endDt )
    , vfsPubSubMsgContainersToPush( src.vfsPubSubMsgContainersToPush )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CStorageChannel::StorageToPubSubRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "StorageToPubSubRequest";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool                                                
CStorageChannel::StorageToPubSubRequest::SaveConfig( CORE::CDataNode & tree ) const
{
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CStorageChannel::StorageToPubSubRequest::LoadConfig( const CORE::CDataNode & treeroot )
{
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::AddStorageToPubSubRequest( const StorageToPubSubRequest& request )
{GUCEF_TRACE;

    m_storageToPubSubRequests.push_back( request );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;

    m_channelSettings.vfsStorageRootPath = CORE::ResolveVars( m_channelSettings.vfsStorageRootPath ).ReplaceSubstr( "{channelId}", CORE::ToString( m_channelSettings.channelId ) );
    
    if ( m_channelSettings.vfsFileExtention.IsNULLOrEmpty() )
    {
        if ( m_channelSettings.encodeCodecFamily.IsNULLOrEmpty() || m_channelSettings.encodeCodecName.IsNULLOrEmpty() )
            m_channelSettings.vfsFileExtention = "bin";
        else
        {
            if ( "deflate" == m_channelSettings.encodeCodecName )
                m_channelSettings.vfsFileExtention = "bin.gz";    
            else
                m_channelSettings.vfsFileExtention = "bin.encoded";
        }
    }
    // the encoder and decoder almost always belong to the same codec family so we can make that the default
    if ( m_channelSettings.decodeCodecFamily.IsNULLOrEmpty() )
    {
        m_channelSettings.decodeCodecFamily = m_channelSettings.encodeCodecFamily;
    }
    if ( m_channelSettings.decodeCodecName.IsNULLOrEmpty() )
    {
        if ( "deflate" == m_channelSettings.encodeCodecName )
            m_channelSettings.decodeCodecName = "inflate";
    }
    m_vfsFilePostfix = ".v" + CORE::ToString( PUBSUB::CPubSubMsgContainerBinarySerializer::CurrentFormatVersion ) + '.' + m_channelSettings.vfsFileExtention;
        
    return m_pubsubClient->LoadConfig( channelSettings );
}

/*-------------------------------------------------------------------------*/

const ChannelSettings&
CStorageChannel::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStorageChannel::GetType( void ) const
{GUCEF_TRACE;

    return "StorageChannel";
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::WaitForTaskToFinish( CORE::Int32 timeoutInMs )
{GUCEF_TRACE;

    // Overriding the base class implementation because this consumer can start its own
    // consumer based on settings transparent to the caller.
    if ( CTaskConsumer::WaitForTaskToFinish( timeoutInMs ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "StorageChannel:WaitForTaskToFinish: Successfully waited for channel " + CORE::Int32ToString( m_channelSettings.channelId ) + "'s task to stop" );
        if ( m_channelSettings.performPubSubInDedicatedThread )
        {
            if ( m_pubsubClient->WaitForTaskToFinish( timeoutInMs ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "StorageChannel:WaitForTaskToFinish: Successfully waited for channel " + CORE::Int32ToString( m_channelSettings.channelId ) + "'s dedicated pub sub task to stop" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StorageChannel:WaitForTaskToFinish: Failed waiting for dedicated pub sub task to stop for channel " + CORE::Int32ToString( m_channelSettings.channelId ) );
            }
        }
        return true;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "StorageChannel:WaitForTaskToFinish: Failed waiting for task to stop for channel " + CORE::Int32ToString( m_channelSettings.channelId ) );
    return false;
}

/*-------------------------------------------------------------------------*/

CStorageChannel::ChannelMetrics::ChannelMetrics( void )
    //: udpBytesReceived( 0 )
    //, udpPacketsReceived( 0 )
    //, redisMessagesTransmitted( 0 )
    //, redisPacketsInMsgsTransmitted( 0 )
    //, redisPacketsInMsgsRatio( 0 )
    //, redisTransmitQueueSize( 0 )
    //, redisErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStorageChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    //m_metrics.udpBytesReceived = m_udpSocket->GetBytesReceived( true );
    //m_metrics.udpPacketsReceived = m_udpSocket->GetNrOfDataReceivedEvents( true );
    //m_metrics.redisTransmitQueueSize = m_redisWriter->GetRedisTransmitQueueSize();
    //m_metrics.redisMessagesTransmitted = m_redisWriter->GetRedisMsgsTransmittedCounter( true );
    //m_metrics.redisPacketsInMsgsTransmitted = m_redisWriter->GetRedisPacketsInMsgsTransmittedCounter( true );
    //m_metrics.redisPacketsInMsgsRatio = m_redisWriter->GetRedisPacketsInMsgsRatio();
    //m_metrics.redisErrorReplies = m_redisWriter->GetRedisErrorRepliesCounter( true );
}

/*-------------------------------------------------------------------------*/

const CStorageChannel::ChannelMetrics&
CStorageChannel::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( m_channelSettings.performPubSubInDedicatedThread )
    {
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
        CORE::CFutureResult result = threadPool->StartTaskWithConsumer( m_pubsubClient );
        if ( result.HasNoFuture() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:OnTaskStart: Failed to start dedicated task (dedicated thread) for pub-sub. Falling back to a single thread" );
            m_channelSettings.performPubSubInDedicatedThread = false;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:OnTaskStart: Successfully requested the launch of a dedicated task (dedicated thread) for pub-sub" );
        }
    }

    if ( !m_channelSettings.performPubSubInDedicatedThread )
    {
        if ( !m_pubsubClient->OnTaskStart( task ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:OnTaskStart: Failed startup of pub-sub client logic" );
            return false;
        }
    }

    if ( ( m_channelSettings.mode == TChannelMode::CHANNELMODE_STORAGE_TO_PUBSUB ) && ( m_channelSettings.autoPushAfterStartupIfStorageToPubSub ) )
    {
        AddStorageToPubSubRequest( StorageToPubSubRequest( m_channelSettings.oldestStoragePubSubMsgFileToLoad, m_channelSettings.youngestStoragePubSubMsgFileToLoad ) );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStorageChannel::GetPathToLastWrittenPubSubStorageFile( CORE::UInt32 lastOffset ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    
    CORE::CString fileFilter = '*' + m_vfsFilePostfix;
    VFS::CVFS::TStringVector index;
    vfs.GetFileList( index, m_channelSettings.vfsStorageRootPath, false, true, fileFilter );

    // The index is already alphabetically ordered and since we use the datetime as the part of filename we can leverage that
    // to get the last produced file
    if ( !index.empty() )
    {        
        VFS::CVFS::TStringVector::reverse_iterator f = index.rbegin();
        CORE::UInt32 n=0;
        while ( n<lastOffset && f != index.rend() )   
        {
            ++f; ++n;
        }

        if ( f != index.rend() )
        {
            const CORE::CString& lastFilename = (*f);            
            return lastFilename;
        }
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool 
CStorageChannel::GetLastPersistedMsgAttributes( CORE::Int32 channelId          , 
                                                const CORE::CString& topicName , 
                                                CORE::CVariant& msgId          , 
                                                CORE::CDateTime& msgDt         )
{GUCEF_TRACE;

    bool success = true;
    CORE::UInt32 lastFileOffset = 0;
    bool fileExistedButHasIssue = false;
    do
    {
        success = GetLastPersistedMsgAttributesWithOffset( channelId              ,
                                                           topicName              ,
                                                           msgId                  ,
                                                           msgDt                  ,
                                                           lastFileOffset         ,
                                                           fileExistedButHasIssue );
        ++lastFileOffset;
    }
    while ( !success && fileExistedButHasIssue );
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CStorageChannel::GetPersistedBookmark( CORE::Int32 channelId              , 
                                       const CORE::CString& topicName     , 
                                       PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    // @TODO: Update to use dedicated bookmark persistance

    CORE::CVariant msgId;
    CORE::CDateTime msgDt; 
    if ( GetLastPersistedMsgAttributes( channelId, topicName, msgId, msgDt ) )
    {
        if ( msgId.IsInitialized() )
        {
            bookmark.SetBookmarkData( msgId );
            bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID );
        }
        else
        {
            CORE::CVariant dtStrVar = msgDt.ToIso8601DateTimeString( true, true );
            bookmark.SetBookmarkData( dtStrVar );
            bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME );
        }
        return true;
    }

    bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::LoadStorageFile( const CORE::CString& vfsPath       ,
                                  CORE::CDynamicBuffer& targetBuffer )
{GUCEF_TRACE;


    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
    if ( !m_channelSettings.decodeCodecFamily.IsNULLOrEmpty() && !m_channelSettings.decodeCodecName.IsNULLOrEmpty() )
    {
        CORE::Float32 encodeRatio = m_encodeSizeRatio < 0 ? GUCEF_DEFAULT_DECODE_GROWTH_RATIO_EXPECTATION : m_encodeSizeRatio;
        CORE::UInt32 estimatedApproxDecodedSize = (CORE::UInt32) ( vfs.GetFileSize( vfsPath ) * encodeRatio );
        targetBuffer.SetBufferSize( estimatedApproxDecodedSize, false );

        if ( !vfs.DecodeAsFile( targetBuffer, 0, vfsPath, m_channelSettings.decodeCodecFamily, m_channelSettings.decodeCodecName ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:LoadStorageFile: Cannot decode and load persisted file. CodeFamily:" + m_channelSettings.decodeCodecFamily +
                " CodecName: " + m_channelSettings.decodeCodecName + ". VFS File: " + vfsPath );
            return false;
        }

        if ( targetBuffer.GetDataSize() > 0 )
            m_encodeSizeRatio = (CORE::Float32) ( targetBuffer.GetDataSize() / vfs.GetFileSize( vfsPath ) );
    }
    else
    {
        // Not using any encoding, load the file as-is
        if ( !vfs.LoadFile( targetBuffer, vfsPath, "rb" ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:LoadStorageFile: Cannot load last persisted file. VFS File: " + vfsPath );
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CStorageChannel::GetLastPersistedMsgAttributesWithOffset( CORE::Int32 channelId          , 
                                                          const CORE::CString& topicName , 
                                                          CORE::CVariant& msgId          , 
                                                          CORE::CDateTime& msgDt         ,
                                                          CORE::UInt32 lastFileOffset    ,
                                                          bool& fileExistedButHasIssue   )
{GUCEF_TRACE;

    // @TODO: topic name segregation
    
    if ( channelId != m_channelSettings.channelId )
    {
        fileExistedButHasIssue = false;
        return false; // this should never happen
    }

    if ( m_lastPersistedMsgId.IsNULLOrEmpty() && m_lastPersistedMsgDt == CORE::CDateTime::Empty )
    {
        CORE::CString lastWrittenFilePath = GetPathToLastWrittenPubSubStorageFile( lastFileOffset );
        if ( lastWrittenFilePath.IsNULLOrEmpty() )
        {
            fileExistedButHasIssue = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastWrittenPubSubMsgId: Cannot obtain path to last written file with offset " + CORE::ToString( lastFileOffset ) );
            return false;
        }

        CORE::CDynamicBuffer lastStorageFileContent;
        if ( !LoadStorageFile( lastWrittenFilePath, lastStorageFileContent ) )
        {
            fileExistedButHasIssue = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastWrittenPubSubMsgId: Unable to load file from storage. Loading using last offset " + CORE::ToString( lastFileOffset ) );
            return false;
        }

        if ( 0 == lastStorageFileContent.GetDataSize() )
        {
            fileExistedButHasIssue = true;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: last persisted file is empty. VFS File: " + lastWrittenFilePath );
            return false;
        }

        bool isCorrupted = false;
        PUBSUB::CBasicPubSubMsg msg;
        if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeMsgAtIndex( msg, true, lastStorageFileContent, 0, false, isCorrupted ) )
        {
            if ( isCorrupted )
            {
                // Attempt to recover what we can with an index rebuild
                // This could effectively move the "last" message received to the actually non-corrupt persisted message as the new "last"
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: Failed to deserialize the last message, will attempt an index rebuild of the corrupt container" );
               
                CORE::UInt32 bytesRead = 0;
                PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex newRecoveredIndex;
                if ( PUBSUB::CPubSubMsgContainerBinarySerializer::IndexRebuildScan( newRecoveredIndex, lastStorageFileContent, bytesRead, true ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: Successfully performed an index rebuild of the corrupt container, discovered " + CORE::ToString( newRecoveredIndex.size() ) + " messages. Will attempt to add a new footer" );
                    
                    CORE::UInt32 bytesWritten = 0;
                    if ( PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeFooter( newRecoveredIndex, lastStorageFileContent.GetDataSize()-1, lastStorageFileContent, bytesWritten ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: Successfully serialized a new footer to the previously corrupt container. Will attempt to persist the amended container" );

                        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
                        
                        if ( m_channelSettings.encodeCodecFamily.IsNULLOrEmpty() || m_channelSettings.encodeCodecName.IsNULLOrEmpty() )
                        {
                            if ( vfs.StoreAsFile( lastWrittenFilePath, lastStorageFileContent, 0, true ) )
                            {
                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: Successfully stored rebuild pub-sub message container at: " + lastWrittenFilePath );
                            }
                            else
                            {
                                fileExistedButHasIssue = true;
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:GetLastPersistedMsgAttributes: StoreAsFile() Failed for rebuild message container" );
                                return false;
                            }
                        }
                        else
                        {
                            if ( vfs.EncodeAsFile( lastStorageFileContent, 0, lastWrittenFilePath, true, m_channelSettings.encodeCodecFamily, m_channelSettings.encodeCodecName ) )
                            {
                                m_encodeSizeRatio = (CORE::Float32) ( lastStorageFileContent.GetDataSize() / vfs.GetFileSize( lastWrittenFilePath ) );
                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:GetLastPersistedMsgAttributes: Successfully encoded and stored rebuild pub-sub message container resource at: \"" + lastWrittenFilePath + "\" with a encoded size ratio of " + CORE::ToString( m_encodeSizeRatio ) );
                            }
                            else
                            {
                                fileExistedButHasIssue = true;
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:GetLastPersistedMsgAttributes: EncodeAsFile() Failed for rebuild message container" );
                                return false;
                            }
                        }
                    }
                }

                // Lets try again, hopefully its fixed now best effort...
                if ( !PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeMsgAtIndex( msg, true, lastStorageFileContent, 0, false, isCorrupted ) )
                {
                    // This should not happen, something is seriously wrong here.
                    fileExistedButHasIssue = true;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:GetLastPersistedMsgAttributes: Failed to load last message even after a successfull rebuild. isCorrupted=" + CORE::ToString( isCorrupted ) );
                    return false;
                }
            }
        }

        m_lastPersistedMsgId = msg.GetMsgId();
        m_lastPersistedMsgDt = msg.GetMsgDateTime();
    }

    msgId = m_lastPersistedMsgId;
    msgDt = m_lastPersistedMsgDt;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::GetStartAndEndFromContainerFilename( const CORE::CString& fullPath ,
                                                      CORE::CDateTime& startDt      ,
                                                      CORE::CDateTime& endDt        ) const
{GUCEF_TRACE;

    // first strip the extra stuff from the full path to get the string form timestamps
    CORE::CString segment = CORE::ExtractFilename( fullPath );
    segment = segment.CutChars( m_vfsFilePostfix.Length(), false, 0 );
    CORE::CString startDtSegment = segment.SubstrToChar( '_', true );
    CORE::CString endDtSegment = segment.SubstrToChar( '_', false );

    // Try to parse what is left as a valid ISO 8601 DateTime
    if ( startDt.FromIso8601DateTimeString( startDtSegment ) && endDt.FromIso8601DateTimeString( endDtSegment ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::GetPathsToPubSubStorageFiles( const CORE::CDateTime& startDt  ,
                                               const CORE::CDateTime& endDt    ,
                                               CORE::CString::StringSet& files ) const
{GUCEF_TRACE;
   
    
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    
    CORE::CString fileFilter = '*' + m_vfsFilePostfix;
    VFS::CVFS::TStringVector index;
    vfs.GetFileList( index, m_channelSettings.vfsStorageRootPath, false, true, fileFilter );

    VFS::CVFS::TStringVector::iterator i = index.begin();
    while ( i != index.end() )
    {        
        CORE::CDateTime containerFileFirstMsgDt;
        CORE::CDateTime containerFileLastMsgDt;
        if ( GetStartAndEndFromContainerFilename( (*i), containerFileFirstMsgDt, containerFileLastMsgDt ) )
        {
            // Check the container first messgage dt against the our time range
            // It is assumed here that the containers have messages chronologically ordered
            if ( containerFileFirstMsgDt.IsWithinRange( startDt, endDt ) || containerFileLastMsgDt.IsWithinRange( startDt, endDt ) )
            {
                files.insert( (*i) );
            }
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::StoreNextReceivedPubSubBuffer( void )
{GUCEF_TRACE;

    CORE::CDynamicBufferSwap& buffers = m_pubsubClient->GetSerializedMsgBuffers();
        
    CORE::CDateTime msgBatchDt;
    m_msgReceiveBuffer = buffers.GetNextReaderBuffer( msgBatchDt, false, 25 );
    if ( GUCEF_NULL != m_msgReceiveBuffer )
    {
        // Get the timestamp of the last message in the buffer.
        // This is not as expensive an operation as it would appear because we just link to the bytes in the buffer we dont copy them
        bool isCorrupted = false;
        PUBSUB::CBasicPubSubMsg lastMsg;
        PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeMsgAtIndex( lastMsg, true, *m_msgReceiveBuffer, 0, false, isCorrupted );

        CORE::CString vfsFilename = msgBatchDt.ToIso8601DateTimeString( false, true ) + '_' + lastMsg.GetMsgDateTime().ToIso8601DateTimeString( false, true ) + m_vfsFilePostfix;
        CORE::CString vfsStoragePath = CORE::CombinePath( m_channelSettings.vfsStorageRootPath, vfsFilename );
            
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

        if ( m_channelSettings.encodeCodecFamily.IsNULLOrEmpty() || m_channelSettings.encodeCodecName.IsNULLOrEmpty() )
        {
            if ( vfs.StoreAsFile( vfsStoragePath, *m_msgReceiveBuffer, 0, true ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:OnTaskCycle: Successfully stored pub-sub mesage block at: " + vfsStoragePath );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:OnTaskCycle: StoreAsFile() Failed" );
            }
        }
        else
        {
            if ( vfs.EncodeAsFile( *m_msgReceiveBuffer, 0, vfsStoragePath, true, m_channelSettings.encodeCodecFamily, m_channelSettings.encodeCodecName ) )
            {
                m_encodeSizeRatio = (CORE::Float32) ( m_msgReceiveBuffer->GetDataSize() / ( 1.0f * vfs.GetFileSize( vfsStoragePath ) ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:OnTaskCycle: Successfully encoded and stored pub-sub mesage block at: \"" + vfsStoragePath + "\" with a encoded size ratio of " + CORE::ToString( m_encodeSizeRatio ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:OnTaskCycle: EncodeAsFile() Failed" );
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CStorageChannel::OnUnableToFullFillStorageToPubSubRequest( const StorageToPubSubRequest& failedRequest )
{GUCEF_TRACE;

    
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::ProcessNextStorageToPubSubRequest( void )
{GUCEF_TRACE;

    StorageToPubSubRequestDeque::iterator i = m_storageToPubSubRequests.begin();
    if ( i != m_storageToPubSubRequests.end() )
    {
        StorageToPubSubRequest& queuedRequest = (*i);

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Request for messages in range " + 
            CORE::ToString( queuedRequest.startDt ) + " to " + CORE::ToString( queuedRequest.endDt ) );

        if ( queuedRequest.vfsPubSubMsgContainersToPush.empty() )
        {
            if ( !GetPathsToPubSubStorageFiles( queuedRequest.startDt                      ,
                                                queuedRequest.endDt                        ,
                                                queuedRequest.vfsPubSubMsgContainersToPush ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Did not obtain any storage paths for time range " +
                    queuedRequest.startDt.ToIso8601DateTimeString( true, true ) + " to " + queuedRequest.endDt.ToIso8601DateTimeString( true, true ) );
                
                OnUnableToFullFillStorageToPubSubRequest( queuedRequest );
                m_storageToPubSubRequests.pop_back();
                return false;    
            }
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Available data in the request range spans " + 
            CORE::ToString( queuedRequest.vfsPubSubMsgContainersToPush.size() ) + " containers" );
    
        size_t containersProcessed = 0;
        CORE::CString::StringSet::iterator n = queuedRequest.vfsPubSubMsgContainersToPush.begin();
        while ( n != queuedRequest.vfsPubSubMsgContainersToPush.end() )
        {
            bool needContainerSubsetOnly = false;
            bool containerStartIsInRange = true;
            bool containerEndIsInRange = true;

            CORE::CDateTime containerFileFirstMsgDt;
            CORE::CDateTime containerFileLastMsgDt;
            if ( GetStartAndEndFromContainerFilename( (*n), containerFileFirstMsgDt, containerFileLastMsgDt ) )
            {
                containerStartIsInRange = containerFileFirstMsgDt.IsWithinRange( queuedRequest.startDt, queuedRequest.endDt );
                containerEndIsInRange = containerFileLastMsgDt.IsWithinRange( queuedRequest.startDt, queuedRequest.endDt );                 
                needContainerSubsetOnly = !( containerStartIsInRange && containerEndIsInRange );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Parsed file path container start and end DateTimes. Start=" +
                    CORE::ToString( containerFileFirstMsgDt ) + ", End=" + CORE::ToString( containerFileLastMsgDt ) + ". containerStartIsInRange=" + CORE::ToString( containerStartIsInRange ) +
                    ", containerEndIsInRange=" + CORE::ToString( containerEndIsInRange ) + ", needContainerSubsetOnly=" + CORE::ToString( needContainerSubsetOnly ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Failed to parse start and/or end DateTime from file path: " + (*n) );
            }
            
            if ( needContainerSubsetOnly )
            {
                if ( GUCEF_NULL == m_msgReceiveBuffer )
                    m_msgReceiveBuffer = m_pubsubClient->GetSerializedMsgBuffers().GetNextWriterBuffer( containerStartIsInRange ? containerFileFirstMsgDt : queuedRequest.startDt, true, GUCEF_MT_INFINITE_LOCK_TIMEOUT );

                if ( GUCEF_NULL != m_msgReceiveBuffer )
                {
                    if ( LoadStorageFile( (*n), *m_msgReceiveBuffer ) )
                    {
                        CORE::UInt32 bytesRead = 0;
                        PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex originalOffsetIndex;
                        PUBSUB::CPubSubMsgContainerBinarySerializer::DeserializeFooter( originalOffsetIndex, *m_msgReceiveBuffer, bytesRead, true );

                        // Since we loaded the entire container we need to now efficiently make sure only the subset gets processed
                        // The way we can do that is by editing the footer in the buffer to logically eliminate entries we do not need
                        // This will make it appear as if only the needed entries are in the container to the reader when reading the footer
                        CORE::UInt32 startIndexOffset = 0;
                        CORE::UInt32 endIndexOffset = 0;
                        bool isCorrupted = false;                        
                        PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;                        
                        if ( PUBSUB::CPubSubMsgContainerBinarySerializer::Deserialize( msgs, true, originalOffsetIndex, *m_msgReceiveBuffer, isCorrupted, true ) )
                        {
                            // Check to see how many we need to trim from the start

                            if ( !containerStartIsInRange )
                            {
                                PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::iterator m = msgs.begin();
                                while ( m != msgs.end() )    
                                {
                                    if ( (*m).GetMsgDateTime() >= queuedRequest.startDt )
                                        break;                         
                                    ++m; ++startIndexOffset;
                                }
                            }
                            if ( !containerEndIsInRange )
                            {
                                PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector::reverse_iterator m = msgs.rbegin();
                                while ( m != msgs.rend() )    
                                {
                                    if ( (*m).GetMsgDateTime() <= queuedRequest.endDt )
                                        break;                         
                                    ++m; ++endIndexOffset;
                                }
                            }

                            CORE::UInt32 o2=0;
                            std::size_t newIndexSize = originalOffsetIndex.size() - ( startIndexOffset + endIndexOffset );
                            endIndexOffset = (CORE::UInt32) originalOffsetIndex.size() - endIndexOffset;
                            PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex newOffsetIndex( newIndexSize );                            
                            for ( CORE::UInt32 o=startIndexOffset; o<endIndexOffset; ++o )
                            {
                                newOffsetIndex[ o2 ] = originalOffsetIndex[ o ];
                                ++o2;
                            }

                            // Now we overwrite the footer in the in-memory container to only have the subset of messages we care about referenced
                            CORE::UInt32 bytesWritten = 0;
                            if ( PUBSUB::CPubSubMsgContainerBinarySerializer::SerializeFooter( newOffsetIndex, m_msgReceiveBuffer->GetDataSize()-1, *m_msgReceiveBuffer, bytesWritten ) )
                            {
                                // We are done with this container
                                ++containersProcessed;
                                m_msgReceiveBuffer = GUCEF_NULL;
                            }
                        }
                    }
                }
                else
                {
                    // No write buffer available, we need to wait before processing more requests
                    return false;
                }
            }
            else
            {
                if ( GUCEF_NULL == m_msgReceiveBuffer )
                    m_msgReceiveBuffer = m_pubsubClient->GetSerializedMsgBuffers().GetNextWriterBuffer( containerFileFirstMsgDt, true, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                
                if ( GUCEF_NULL != m_msgReceiveBuffer )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:ProcessNextStorageToPubSubRequest: Loading the entire container as-is to serve (part of) the request" );

                    if ( LoadStorageFile( (*n), *m_msgReceiveBuffer ) )
                    {
                        // Since we loaded the entire container and we dont need a subset we are done
                        ++containersProcessed;
                        m_msgReceiveBuffer = GUCEF_NULL;
                    }
                }
                else
                {
                    // No write buffer available, we need to wait before processing more requests
                    return false;
                }
            }
            ++n;
        }

        if ( containersProcessed != queuedRequest.vfsPubSubMsgContainersToPush.size() )
        {
            OnUnableToFullFillStorageToPubSubRequest( queuedRequest );
        }
        
        m_storageToPubSubRequests.pop_front();
        m_pubsubClient->GetSerializedMsgBuffers().SignalEndOfWriting();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStorageChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( !m_channelSettings.performPubSubInDedicatedThread )
    {
        m_pubsubClient->OnTaskCycle( task );
    }

    switch ( m_channelSettings.mode )
    {
        case TChannelMode::CHANNELMODE_PUBSUB_TO_STORAGE:
        {
            StoreNextReceivedPubSubBuffer();
            break;
        }
        case TChannelMode::CHANNELMODE_STORAGE_TO_PUBSUB:
        {
            ProcessNextStorageToPubSubRequest();
            break;
        }
    }

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
CStorageChannel::OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   )
{GUCEF_TRACE;

    if ( !m_channelSettings.performPubSubInDedicatedThread )
    {
        m_pubsubClient->OnTaskEnding( task, willBeForced );
    }
    else
    {
        // Since we are the ones that launched the dedicated Redis write thread we should also ask
        // to have it cleaned up when we are shutting down this thread
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
        if ( !threadPool->RequestTaskToStop( m_pubsubClient.StaticCast< CORE::CTaskConsumer >(), false ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "StorageChannel:OnTaskEnding: Failed to request the dedicated task (dedicated thread) for pub-sub to stop" );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StorageChannel:OnTaskEnding: Successfully requested the dedicated task (dedicated thread) for pub-sub to stop" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStorageChannel::OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    if ( !m_channelSettings.performPubSubInDedicatedThread )
    {
        m_pubsubClient->OnTaskEnded( task, wasForced );
    }

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2StorageInfoResource::RestApiPubSub2StorageInfoResource( PubSub2Storage* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2StorageInfoResource::~RestApiPubSub2StorageInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSub2StorageInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                              CORE::CDataNode& output             ,
                                              const CORE::CString& representation ,
                                              const CORE::CString& params         )
{GUCEF_TRACE;

    output.SetName( "info" );
    output.SetAttribute( "application", "pubsub2storage" );
    output.SetAttribute( "appBuildDateTime", PubSub2Storage::GetAppCompileDateTime().ToIso8601DateTimeString( true, true ) );
    output.SetAttribute( "platformBuildDateTime", CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", "false" );
    #else
    output.SetAttribute( "isReleaseBuild", "true" );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2StorageConfigResource::RestApiPubSub2StorageConfigResource( PubSub2Storage* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2StorageConfigResource::~RestApiPubSub2StorageConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSub2StorageConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                                CORE::CDataNode& output             ,
                                                const CORE::CString& representation ,
                                                const CORE::CString& params         )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const CORE::CValueList& loadedConfig = m_app->GetAppConfig();
        return loadedConfig.SaveConfig( output );
    }
    else
    {
        const CORE::CDataNode& globalConfig = m_app->GetGlobalConfig();
        output.Copy( globalConfig );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2StorageConfigResource::TDeserializeState
RestApiPubSub2StorageConfigResource::Deserialize( const CORE::CString& resourcePath   ,
                                                  const CORE::CDataNode& input        ,
                                                  const CORE::CString& representation ,
                                                  bool isDeltaUpdateOnly              )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        CORE::CValueList loadedAppConfig;

        if ( isDeltaUpdateOnly )
        {
            // Grab a copy of the current app config
            loadedAppConfig = m_app->GetAppConfig();
            loadedAppConfig.SetAllowMultipleValues( false );
            loadedAppConfig.SetAllowDuplicates( false );
        }
        else
        {
            loadedAppConfig.CopySettingsFrom( m_app->GetAppConfig() );
        }

        if ( loadedAppConfig.LoadConfig( input ) )
        {
            if ( isDeltaUpdateOnly )
            {
                loadedAppConfig.SetAllowMultipleValues( m_app->GetAppConfig().GetAllowMultipleValues() );
                loadedAppConfig.SetAllowDuplicates( m_app->GetAppConfig().GetAllowDuplicates() );
            }

            // First put the app in standby mode before we mess with the settings
            if ( !m_app->SetStandbyMode( true ) )
                return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;

            const CORE::CDataNode& globalConfig = m_app->GetGlobalConfig();
            if ( m_app->LoadConfig( loadedAppConfig ) )
            {
                if ( !m_app->IsGlobalStandbyEnabled() )
                {
                    if ( m_app->SetStandbyMode( false ) )
                        return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
                    else
                        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiUdp2RedisConfigResource: IsGlobalStandbyEnabled is true. We will leave the app in standby mode" );
                    return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
                }
            }
            else
            {
                return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
            }
        }

        return TDeserializeState::DESERIALIZESTATE_CORRUPTEDINPUT;
    }
    else
    {
        if ( isDeltaUpdateOnly )
        {
            //// Grab a copy of the current global config
            //CORE::CDataNode globalConfigCopy = m_app->GetGlobalConfig();
            //if ( globalConfigCopy.Merge( input ) )
            //{
            //    const CORE::CValueList& loadedAppConfig = m_app->GetAppConfig();
            //    if ( m_app->LoadConfig( loadedAppConfig, globalConfigCopy ) )
            //    {
            //        return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
            //    }
            //    else
            //    {
            //        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
            //    }
            //}

            return TDeserializeState::DESERIALIZESTATE_CORRUPTEDINPUT;
        }
        else
        {
            const CORE::CValueList& loadedAppConfig = m_app->GetAppConfig();
            if ( m_app->LoadConfig( input ) )
            {
                return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
            }
            else
            {
                return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

PubSub2Storage::PubSub2Storage( void )
    : CORE::CObserver()
    , CORE::CIConfigurable()
    , m_isInStandby( false )
    , m_globalStandbyEnabled( false )
    , m_udpStartPort()
    , m_channelCount()
    , m_storageStartChannelID()
    , m_redisStreamName()
    , m_redisHost()
    , m_redisPort()
    , m_channels()
    , m_channelSettings()
    , m_templateChannelSettings()
    , m_httpServer()
    , m_httpRouter()
    , m_appConfig()
    , m_globalConfig()
    , m_metricsTimer()
    , m_transmitMetrics( true )
{GUCEF_TRACE;

    TEventCallback callback1( this, &PubSub2Storage::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );
}

/*-------------------------------------------------------------------------*/

PubSub2Storage::~PubSub2Storage()
{GUCEF_TRACE;

    m_httpServer.Close();
}

/*-------------------------------------------------------------------------*/

bool
PubSub2Storage::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_globalStandbyEnabled;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2Storage::Start( void )
{GUCEF_TRACE;

    m_isInStandby = true;
    bool errorOccured = !SetStandbyMode( m_globalStandbyEnabled );

    if ( !errorOccured )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage: Opening REST API" );
        return m_httpServer.Listen();
    }
    return !errorOccured;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2Storage::SetStandbyMode( bool putInStandbyMode )
{GUCEF_TRACE;

    // Check if we need to do anything
    if ( m_isInStandby == putInStandbyMode )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( " + CORE::BoolToString( putInStandbyMode ) + " ): Already in the desired mode (" + CORE::BoolToString( m_isInStandby ) + "), nothing to do" );
        return true;
    }

    if ( putInStandbyMode )
    {
        bool totalSuccess = true;
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        // Signal all channel threads to stop gracefully
        // Since this standby operation is global not per channel we signal all to stop before
        // we starting any waiting operation
        StorageChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CStorageChannelPtr channel = (*i).second;
            if ( !threadPool->RequestTaskToStop( channel.StaticCast< CORE::CTaskConsumer >(), false ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( true ): Requested channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        // Now actually wait for the threads to be finished
        i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CStorageChannelPtr channel = (*i).second;
            if ( !channel->WaitForTaskToFinish( 30000 ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( true ): Successfully waited for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        m_metricsTimer.SetEnabled( false );

        m_isInStandby = totalSuccess;
        return totalSuccess;
    }
    else
    {
        bool totalSuccess = true;

        // Channel config could have changed such that we need to remove channels that should no longer exist
        StorageChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CORE::Int32 channelId = (*i).first;
            ChannelSettingsMap::iterator n = m_channelSettings.find( channelId );
            if ( n == m_channelSettings.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( false ): Found channel which no longer has corresponding channel settings, deleting channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels.erase( i );
                i = m_channels.begin();
                break;
            }
            ++i;
        }

        // Alternatively channel config could have changed such that we have new channels
        ChannelSettingsMap::iterator n = m_channelSettings.begin();
        while ( n != m_channelSettings.end() )
        {
            CORE::Int32 channelId = (*n).first;
            StorageChannelMap::iterator i = m_channels.find( channelId );
            if ( i == m_channels.end() )
            {
                // This is a brand new channel. Lets add the channel object for it
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage:SetStandbyMode( false ): Found channel settings whith no corresponding channel object, creating new channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels[ channelId ] = CStorageChannelPtr( new CStorageChannel() );
            }
            ++n;
        }

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        n = m_channelSettings.begin();
        while ( n != m_channelSettings.end() )
        {
            CORE::Int32 channelId = (*n).first;
            StorageChannelMap::iterator i = m_channels.find( channelId );
            if ( i != m_channels.end() )
            {
                const ChannelSettings& channelSettings = (*n).second;
                CStorageChannelPtr channel = (*i).second;

                if ( !channel->LoadConfig( channelSettings ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage::SetStandbyMode( false ): Failed to set channel settings on channel " + CORE::Int32ToString( channelId ) );
                    totalSuccess = false;
                    break;
                }

                CORE::CFutureResult result = threadPool->StartTaskWithConsumer( channel );
                if ( result.HasNoFuture() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2Storage::SetStandbyMode( false ): Failed to start task (dedicated thread) for channel " + CORE::Int32ToString( channelId ) );
                    totalSuccess = false;
                    break;
                }
            }
            ++n;
        }

        if ( totalSuccess && m_transmitMetrics )
        {
            m_metricsTimer.SetInterval( 1000 );
            m_metricsTimer.SetEnabled( true );
        }

        m_isInStandby = !totalSuccess;
        return totalSuccess;
    }
}

/*-------------------------------------------------------------------------*/

bool
PubSub2Storage::LoadConfig( const CORE::CValueList& appConfig )
{GUCEF_TRACE;

    m_globalStandbyEnabled = appConfig.GetValueAlways( "GlobalStandbyEnabled", m_globalStandbyEnabled ).AsBool( false );
    m_channelCount = appConfig.GetValueAlways( "ChannelCount", 1 ).AsUInt16( 1, true );
    m_storageStartChannelID = appConfig.GetValueAlways( "StorageStartChannelID", 1 ).AsUInt16( 1, true );

    bool applyCpuThreadAffinityByDefault = appConfig.GetValueAlways( "ApplyCpuThreadAffinityByDefault", false ).AsBool( false );
    CORE::UInt32 logicalCpuCount = CORE::GetLogicalCPUCount();

    CORE::UInt32 currentCpu = 0;
    CORE::Int32 maxChannelId = m_storageStartChannelID + m_channelCount;
    for ( CORE::Int32 channelId = m_storageStartChannelID; channelId < maxChannelId; ++channelId )
    {
        ChannelSettings* channelSettings = GUCEF_NULL;
        ChannelSettingsMap::iterator s = m_channelSettings.find( channelId );
        if ( s == m_channelSettings.end() )
        {
            channelSettings = &m_channelSettings[ channelId ];
            *channelSettings = m_templateChannelSettings;

            if ( -1 == channelSettings->channelId )
                channelSettings->channelId = channelId;
        }
        else
        {
            channelSettings = &m_channelSettings[ channelId ];
        }

        if ( channelSettings->applyThreadCpuAffinity || applyCpuThreadAffinityByDefault )
        {
            channelSettings->cpuAffinityForMainChannelThread = currentCpu;

            ++currentCpu;
            if ( currentCpu >= logicalCpuCount ) // Wrap around if we run out of CPUs
                currentCpu = 0;

            if ( channelSettings->performPubSubInDedicatedThread )
            {
                channelSettings->cpuAffinityForDedicatedPubSubThread = currentCpu;

                ++currentCpu;
                if ( currentCpu >= logicalCpuCount ) // Wrap around if we run out of CPUs
                    currentCpu = 0;
            }
        }
    }

    m_appConfig = appConfig;

    m_httpServer.SetPort( appConfig.GetValueAlways( "RestApiPort", 10000 ).AsUInt16( 10000, true ) );
    m_httpRouter.SetResourceMapping( "/info", RestApiPubSub2StorageInfoResource::THTTPServerResourcePtr( new RestApiPubSub2StorageInfoResource( this ) )  );
    m_httpRouter.SetResourceMapping( "/config/appargs", RestApiPubSub2StorageConfigResource::THTTPServerResourcePtr( new RestApiPubSub2StorageConfigResource( this, true ) )  );
    m_httpRouter.SetResourceMapping( "/config", RestApiPubSub2StorageConfigResource::THTTPServerResourcePtr( new RestApiPubSub2StorageConfigResource( this, false ) )  );
    m_httpRouter.SetResourceMapping(  CORE::ResolveVars( appConfig.GetValueAlways( "RestBasicHealthUri", "/health/basic" ) ), RestApiPubSub2StorageConfigResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );
    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
PubSub2Storage::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    // not fully supported right now

    tree.Copy( m_globalConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
PubSub2Storage::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    TChannelCfgMap channelMap;

    CORE::CDataNode::TConstDataNodeSet channelParentCfgs = cfg.FindChildrenOfType( "Channels", true );
    CORE::CDataNode::TConstDataNodeSet::iterator i = channelParentCfgs.begin();
    while ( i != channelParentCfgs.end() )
    {
        CORE::CDataNode::const_iterator n = (*i)->ConstBegin();
        while ( n != (*i)->ConstEnd() )
        {
            const CORE::CString& channelIndex = (*n)->GetName();
            channelMap[ channelIndex ] = (*n)->FindChildrenOfType( "StorageChannel" );            
            ++n;
        }                
        ++i;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2Storage:LoadConfig: Found " + CORE::ToString( channelMap.size() ) + " configuration entries for storage channels" );
    
    // load the template if any
    TChannelCfgMap::iterator m = channelMap.find( "*" );
    if ( m != channelMap.end() )
    {
        CORE::CDataNode::TConstDataNodeSet& matches = (*m).second;
        if ( !matches.empty() )
        {            
            if ( m_templateChannelSettings.LoadConfig( *(*matches.begin()) ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2Storage:LoadConfig: Successfully loaded template config for storage channels" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSub2Storage:LoadConfig: Failed to correctly load template config for storage channels" );
                return false;
            }
        }
    }

    // load the specifically configured channels if any
    m = channelMap.begin();
    while ( m != channelMap.end() )
    {
        const CORE::CString& channelIndexStr = (*m).first;
        if ( channelIndexStr != '*' )
        {
            CORE::CDataNode::TConstDataNodeSet& matches = (*m).second;
            if ( !matches.empty() )
            {            
                CORE::Int32 channelIndex = CORE::StringToInt32( channelIndexStr );
                ChannelSettings& channelSettings = m_channelSettings[ channelIndex ];

                if ( channelSettings.LoadConfig( *(*matches.begin()) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2Storage:LoadConfig: Successfully loaded explicit config for storage channels " + channelIndexStr );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "PubSub2Storage:LoadConfig: Failed to correctly load explicit config for storage channels " + channelIndexStr );
                    return false;
                }
            }
        }
        ++m;
    }

    m_globalConfig.Copy( cfg );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CDateTime&
PubSub2Storage::GetAppCompileDateTime( void )
{GUCEF_TRACE;

    static CORE::CDateTime compileDt = CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ );
    return compileDt;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
PubSub2Storage::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "PubSub2Storage";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
PubSub2Storage::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::Int32 channelId = m_storageStartChannelID;
    StorageChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const CStorageChannel::ChannelMetrics& metrics = (*i).second->GetMetrics();
        CORE::CString metricPrefix = "pubsub2storage.ch" + CORE::Int32ToString( channelId ) + ".";

        //GUCEF_METRIC_TIMING( metricPrefix + "redisErrorReplies", metrics.redisErrorReplies, 1.0f );
        //GUCEF_METRIC_TIMING( metricPrefix + "redisMessagesTransmitted", metrics.redisMessagesTransmitted, 1.0f );
        //GUCEF_METRIC_TIMING( metricPrefix + "redisPacketsInMessagesTransmitted", metrics.redisPacketsInMsgsTransmitted, 1.0f );
        //GUCEF_METRIC_GAUGE( metricPrefix + "redisPacketsInMessagesRatio", metrics.redisPacketsInMsgsRatio, 1.0f );
        //GUCEF_METRIC_GAUGE( metricPrefix + "redisTransmitQueueSize", metrics.redisTransmitQueueSize, 1.0f );
        //GUCEF_METRIC_TIMING( metricPrefix + "udpBytesReceived", metrics.udpBytesReceived, 1.0f );
        //GUCEF_METRIC_TIMING( metricPrefix + "udpPacketsReceived", metrics.udpPacketsReceived, 1.0f );

        ++i; ++channelId;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
PubSub2Storage::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
PubSub2Storage::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/

