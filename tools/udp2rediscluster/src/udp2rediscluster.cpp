/*
 *  Udp2RedisCluster: service which pushes UDP packets into clustered redis streams
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

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#include "udp2rediscluster.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <winsock2.h>
#endif

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#include "crc16.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_UDP_RECEIVE_BUFFERS                           100
#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000
#define GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE              (1024 * 1024 * 10)
#define GUCEF_DEFAULT_UDP_MAX_SOCKET_CYCLES_PER_PULSE               25
#define GUCEF_DEFAULT_MAX_DEDICATED_REDIS_WRITER_MAIL_BULK_READ     100
#define GUCEF_DEFAULT_REDIS_RECONNECT_DELAY_IN_MS                   100
#define GUCEF_DEFAULT_REDIS_KEEPALIVE                               true
#define GUCEF_DEFAULT_REDIS_CONNECT_TIMEOUT_IN_MS                   1000
#define GUCEF_DEFAULT_REDIS_SOCKET_TIMEOUT_IN_MS                    1000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ChannelSettings::ChannelSettings( void )
    : channelId( -1 )
    , redisAddress()
    , channelStreamName()
    , udpInterface()
    , udpMulticastToJoin()
    , collectMetrics( false )
    , wantsTestPackage( false )
    , ticketRefillOnBusyCycle( GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE )
    , nrOfUdpReceiveBuffersPerSocket( GUCEF_DEFAULT_UDP_RECEIVE_BUFFERS )
    , udpSocketOsReceiveBufferSize( GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE )
    , udpSocketUpdateCyclesPerPulse( GUCEF_DEFAULT_UDP_MAX_SOCKET_CYCLES_PER_PULSE )
    , performRedisWritesInDedicatedThread( true )
    , maxSizeOfDedicatedRedisWriterBulkMailRead( GUCEF_DEFAULT_MAX_DEDICATED_REDIS_WRITER_MAIL_BULK_READ )
    , applyThreadCpuAffinity( false )
    , cpuAffinityForDedicatedRedisWriterThread( 0 )
    , cpuAffinityForMainChannelThread( 0 )
    , redisXAddMaxLen( -1 )
    , redisXAddMaxLenIsApproximate( true )
    , redisReconnectDelayInMs( GUCEF_DEFAULT_REDIS_RECONNECT_DELAY_IN_MS )
    , redisConnectionOptionKeepAlive( GUCEF_DEFAULT_REDIS_KEEPALIVE )
    , redisConnectionOptionConnectTimeoutInMs( GUCEF_DEFAULT_REDIS_CONNECT_TIMEOUT_IN_MS )
    , redisConnectionOptionSocketTimeoutInMs( GUCEF_DEFAULT_REDIS_SOCKET_TIMEOUT_IN_MS )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ChannelSettings::ChannelSettings( const ChannelSettings& src )
    : channelId( src.channelId )
    , redisAddress( src.redisAddress )
    , channelStreamName( src.channelStreamName )
    , udpInterface( src.udpInterface )
    , udpMulticastToJoin( src.udpMulticastToJoin )
    , collectMetrics( src.collectMetrics )
    , wantsTestPackage( src.wantsTestPackage )
    , ticketRefillOnBusyCycle( src.ticketRefillOnBusyCycle )
    , nrOfUdpReceiveBuffersPerSocket( src.nrOfUdpReceiveBuffersPerSocket )
    , udpSocketOsReceiveBufferSize( src.udpSocketOsReceiveBufferSize )
    , udpSocketUpdateCyclesPerPulse( src.udpSocketUpdateCyclesPerPulse )
    , performRedisWritesInDedicatedThread( src.performRedisWritesInDedicatedThread )
    , maxSizeOfDedicatedRedisWriterBulkMailRead( src.maxSizeOfDedicatedRedisWriterBulkMailRead )
    , applyThreadCpuAffinity( src.applyThreadCpuAffinity )
    , cpuAffinityForDedicatedRedisWriterThread( src.cpuAffinityForDedicatedRedisWriterThread )
    , cpuAffinityForMainChannelThread( src.cpuAffinityForMainChannelThread )
    , redisXAddMaxLen( src.redisXAddMaxLen )
    , redisXAddMaxLenIsApproximate( src.redisXAddMaxLenIsApproximate )
    , redisReconnectDelayInMs( src.redisReconnectDelayInMs )
    , redisConnectionOptionKeepAlive( src.redisConnectionOptionKeepAlive )
    , redisConnectionOptionConnectTimeoutInMs( src.redisConnectionOptionConnectTimeoutInMs )
    , redisConnectionOptionSocketTimeoutInMs( src.redisConnectionOptionSocketTimeoutInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ChannelSettings&
ChannelSettings::operator=( const ChannelSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        channelId = src.channelId;
        redisAddress = src.redisAddress;
        channelStreamName = src.channelStreamName;
        udpInterface = src.udpInterface;
        udpMulticastToJoin = src.udpMulticastToJoin;
        collectMetrics = src.collectMetrics;
        wantsTestPackage = src.wantsTestPackage;
        ticketRefillOnBusyCycle = src.ticketRefillOnBusyCycle;
        nrOfUdpReceiveBuffersPerSocket = src.nrOfUdpReceiveBuffersPerSocket;
        udpSocketOsReceiveBufferSize = src.udpSocketOsReceiveBufferSize;
        udpSocketUpdateCyclesPerPulse = src.udpSocketUpdateCyclesPerPulse;
        performRedisWritesInDedicatedThread = src.performRedisWritesInDedicatedThread;
        maxSizeOfDedicatedRedisWriterBulkMailRead = src.maxSizeOfDedicatedRedisWriterBulkMailRead;
        applyThreadCpuAffinity = src.applyThreadCpuAffinity;
        cpuAffinityForDedicatedRedisWriterThread = src.cpuAffinityForDedicatedRedisWriterThread;
        cpuAffinityForMainChannelThread = src.cpuAffinityForMainChannelThread;
        redisXAddMaxLen = src.redisXAddMaxLen;
        redisXAddMaxLenIsApproximate = src.redisXAddMaxLenIsApproximate;
        redisReconnectDelayInMs = src.redisReconnectDelayInMs;
        redisConnectionOptionKeepAlive = src.redisConnectionOptionKeepAlive;
        redisConnectionOptionConnectTimeoutInMs = src.redisConnectionOptionConnectTimeoutInMs;
        redisConnectionOptionSocketTimeoutInMs = src.redisConnectionOptionSocketTimeoutInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
ChannelSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "channelId", channelId );
    cfg.SetAttribute( "redisAddress", redisAddress.HostnameAndPortAsString() );
    cfg.SetAttribute( "channelStreamName", channelStreamName );
    cfg.SetAttribute( "udpInterface", udpInterface.HostnameAndPortAsString() );
    cfg.SetAttribute( "collectMetrics", collectMetrics );
    cfg.SetAttribute( "wantsTestPackage", wantsTestPackage );
    cfg.SetAttribute( "ticketRefillOnBusyCycle", ticketRefillOnBusyCycle );
    cfg.SetAttribute( "nrOfUdpReceiveBuffersPerSocket", nrOfUdpReceiveBuffersPerSocket );
    cfg.SetAttribute( "udpSocketOsReceiveBufferSize", udpSocketOsReceiveBufferSize );
    cfg.SetAttribute( "udpSocketUpdateCyclesPerPulse", udpSocketUpdateCyclesPerPulse );
    cfg.SetAttribute( "performRedisWritesInDedicatedThread", performRedisWritesInDedicatedThread );
    cfg.SetAttribute( "maxSizeOfDedicatedRedisWriterBulkMailRead", maxSizeOfDedicatedRedisWriterBulkMailRead );
    cfg.SetAttribute( "applyThreadCpuAffinity", applyThreadCpuAffinity );
    cfg.SetAttribute( "cpuAffinityForDedicatedRedisWriterThread", cpuAffinityForDedicatedRedisWriterThread );
    cfg.SetAttribute( "cpuAffinityForMainChannelThread", cpuAffinityForMainChannelThread );
    cfg.SetAttribute( "redisXAddMaxLen", redisXAddMaxLen );
    cfg.SetAttribute( "redisXAddMaxLenIsApproximate", redisXAddMaxLenIsApproximate );
    cfg.SetAttribute( "redisReconnectDelayInMs", redisReconnectDelayInMs );
    cfg.SetAttribute( "redisConnectionOptionKeepAlive", redisConnectionOptionKeepAlive );
    cfg.SetAttribute( "redisConnectionOptionConnectTimeoutInMs", redisConnectionOptionConnectTimeoutInMs );
    cfg.SetAttribute( "redisConnectionOptionSocketTimeoutInMs", redisConnectionOptionSocketTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ChannelSettings::LoadConfig( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    channelId = CORE::StringToInt32( tree.GetAttributeValueOrChildValueByName( "channelId", CORE::Int32ToString( channelId ) ) );
    redisAddress.SetHostnameAndPort( tree.GetAttributeValueOrChildValueByName( "redisAddress", redisAddress.HostnameAndPortAsString() ) );
    channelStreamName = tree.GetAttributeValueOrChildValueByName( "channelStreamName", channelStreamName );
    udpInterface.SetHostname( tree.GetAttributeValueOrChildValueByName( "udpInterface", udpInterface.HostnameAndPortAsString() ) );
    collectMetrics = CORE::StringToBool( tree.GetAttributeValueOrChildValueByName( "collectMetrics" ), collectMetrics );
    wantsTestPackage = CORE::StringToBool( tree.GetAttributeValueOrChildValueByName( "wantsTestPackage" ), wantsTestPackage );
    ticketRefillOnBusyCycle = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "ticketRefillOnBusyCycle", CORE::UInt32ToString( ticketRefillOnBusyCycle ) ) );
    nrOfUdpReceiveBuffersPerSocket = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "nrOfUdpReceiveBuffersPerSocket", CORE::UInt32ToString( nrOfUdpReceiveBuffersPerSocket ) ) );
    udpSocketOsReceiveBufferSize = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "udpSocketOsReceiveBufferSize", CORE::UInt32ToString( udpSocketOsReceiveBufferSize ) ) );
    udpSocketUpdateCyclesPerPulse = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "udpSocketUpdateCyclesPerPulse", CORE::UInt32ToString( udpSocketUpdateCyclesPerPulse ) ) );
    performRedisWritesInDedicatedThread = CORE::StringToBool( tree.GetAttributeValueOrChildValueByName( "performRedisWritesInDedicatedThread" ), performRedisWritesInDedicatedThread );
    maxSizeOfDedicatedRedisWriterBulkMailRead = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "maxSizeOfDedicatedRedisWriterBulkMailRead", CORE::UInt32ToString( maxSizeOfDedicatedRedisWriterBulkMailRead ) ) );
    applyThreadCpuAffinity = CORE::StringToBool( tree.GetAttributeValueOrChildValueByName( "applyThreadCpuAffinity" ), applyThreadCpuAffinity );
    cpuAffinityForDedicatedRedisWriterThread = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "cpuAffinityForDedicatedRedisWriterThread", CORE::UInt32ToString( cpuAffinityForDedicatedRedisWriterThread ) ) );
    cpuAffinityForMainChannelThread = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "cpuAffinityForMainChannelThread", CORE::UInt32ToString( cpuAffinityForMainChannelThread ) ) );
    redisXAddMaxLen = CORE::StringToInt32( tree.GetAttributeValueOrChildValueByName( "redisXAddMaxLen" ), redisXAddMaxLen );
    redisXAddMaxLenIsApproximate = CORE::StringToBool( tree.GetAttributeValueOrChildValueByName( "redisXAddMaxLenIsApproximate" ), redisXAddMaxLenIsApproximate );
    redisReconnectDelayInMs = CORE::StringToUInt32( tree.GetAttributeValueOrChildValueByName( "redisReconnectDelayInMs", CORE::UInt32ToString( redisReconnectDelayInMs ) ) );
    redisConnectionOptionKeepAlive = tree.GetAttributeValueOrChildValueByName( "redisConnectionOptionKeepAlive" ).AsBool( redisConnectionOptionKeepAlive, true );
    redisConnectionOptionConnectTimeoutInMs = tree.GetAttributeValueOrChildValueByName( "redisConnectionOptionConnectTimeoutInMs" ).AsUInt64( redisConnectionOptionConnectTimeoutInMs, true );
    redisConnectionOptionSocketTimeoutInMs = tree.GetAttributeValueOrChildValueByName( "redisConnectionOptionSocketTimeoutInMs" ).AsUInt64( redisConnectionOptionSocketTimeoutInMs, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
ChannelSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "udp2rediscluster::ChannelSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

ClusterChannelRedisWriter::ClusterChannelRedisWriter()
    : CORE::CTaskConsumer()
    , m_redisContext( GUCEF_NULL )
    , m_redisPipeline( GUCEF_NULL )
    , m_redisMsgQueueOverflowQueue()
    , m_redisErrorReplies( 0 )
    , m_redisTransmitQueueSize( 0 )
    , m_redisMsgsTransmitted( 0 )
    , m_redisPacketsInMsgsTransmitted( 0 )
    , m_redisPacketsInMsgsRatio( 0 )
    , m_redisHashSlot( 0 )
    , m_redisShardHost()
    , m_redisShardNodeId()
    , m_channelSettings()
    , m_mailbox()
    , m_bulkMail()
    , m_bulkPackets()
    , m_bulkPacketCounts()
    , m_redisPacketArgs()
    , m_metricsTimer( GUCEF_NULL )
    , m_redisReconnectTimer( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ClusterChannelRedisWriter::~ClusterChannelRedisWriter()
{GUCEF_TRACE;

    RedisDisconnect();
}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &ClusterChannelRedisWriter::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );

    TEventCallback callback2( this, &ClusterChannelRedisWriter::OnRedisReconnectTimerCycle );
    SubscribeTo( m_redisReconnectTimer          ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::CalculateRedisHashSlot( const CORE::CString& keyStr ) const
{GUCEF_TRACE;

    // The following code is copied from: https://redis.io/topics/cluster-spec

    // Minor changes were made to accept a string object and use platform types
    CORE::UInt32 keylen = keyStr.Length();
    const char* key = keyStr.C_String();

    int s, e; /* start-end indexes of { and } */

    /* Search the first occurrence of '{'. */
    for (s = 0; s < keylen; s++)
        if (key[s] == '{') break;

    /* No '{' ? Hash the whole key. This is the base case. */
    if ( s == keylen)
        return sw::redis::crc16(key,keylen) & 16383;

    /* '{' found? Check if we have the corresponding '}'. */
    for (e = s+1; e < keylen; e++)
        if (key[e] == '}') break;

    /* No '}' or nothing between {} ? Hash the whole key. */
    if (e == keylen || e == s+1)
        return sw::redis::crc16(key,keylen) & 16383;

    /* If we are here there is both a { and a } on its right. Hash
     * what is in the middle between { and }. */
    return sw::redis::crc16(key+s+1,e-s-1) & 16383;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetCurrentRedisHashSlot( void ) const
{GUCEF_TRACE;

    return m_redisHashSlot;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ClusterChannelRedisWriter::GetType( void ) const
{GUCEF_TRACE;

    return "ClusterChannelRedisWriter";
}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_channelSettings.performRedisWritesInDedicatedThread )
    {
        // There can be lock contention on AmountOfMail but thats why we only do it at the interval of the metrics timer
        m_redisTransmitQueueSize = (CORE::UInt32) m_redisMsgQueueOverflowQueue.size() + m_mailbox.AmountOfMail();
    }
    else
    {
        m_redisTransmitQueueSize = (CORE::UInt32) m_redisMsgQueueOverflowQueue.size();
    }
}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( RedisConnect() )
    {
        m_redisReconnectTimer->SetEnabled( false );
    }
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );
    m_metricsTimer->SetEnabled( m_channelSettings.collectMetrics );

    GUCEF_DELETE m_redisReconnectTimer;
    m_redisReconnectTimer = new CORE::CTimer( GetPulseGenerator(), m_channelSettings.redisReconnectDelayInMs );

    if ( m_channelSettings.performRedisWritesInDedicatedThread )
    {
        // Set the minimum number of cycles we will go full speed if a single cycle was not enough to handle
        // all the processing. This will cause a bypass of CPU yielding if/when the situation arises.
        // In such a case the thread will run at max speed for a least the below set nr of cycles.
        GetPulseGenerator()->RequestPulsesPerImmediatePulseRequest( m_channelSettings.ticketRefillOnBusyCycle );

        // Default smallest pulse delta at 10ms
        GetPulseGenerator()->RequestPeriodicPulses( this, 10 );

        if ( m_channelSettings.applyThreadCpuAffinity )
        {
            if ( SetCpuAffinityByCpuId( m_channelSettings.cpuAffinityForDedicatedRedisWriterThread ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) +
                    "):OnTaskStart: Successfully set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForDedicatedRedisWriterThread ) );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) +
                    "):OnTaskStart: Failed to set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForDedicatedRedisWriterThread ) +
                    ". Proceeding without affinity");
            }
        }
    }

    RegisterEventHandlers();

    // Setup connection to Redis
    // Note that if there is an error here we will just keep on trying automatically
    RedisConnect();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_redisContext || GUCEF_NULL == m_redisPipeline )
    {
        m_redisReconnectTimer->SetEnabled( true );

        // We are never 'done' so return false
        return false;
    }

    if ( m_channelSettings.performRedisWritesInDedicatedThread )
    {
        if ( m_redisMsgQueueOverflowQueue.empty() )
        {
            m_bulkMail.clear();
            if ( m_mailbox.GetMailList( m_bulkMail, m_channelSettings.maxSizeOfDedicatedRedisWriterBulkMailRead ) )
            {
                m_bulkPackets.clear();
                m_bulkPacketCounts.clear();

                TBufferMailbox::TMailVector::iterator i = m_bulkMail.begin();
                while ( !IsDeactivationRequested() && i != m_bulkMail.end() )
                {
                    CORE::UInt32 packetCount = (*i).eventid;
                    TCloneablePacketEntryVector* udpPackets = static_cast< TCloneablePacketEntryVector* >( (*i).data );
                    if ( GUCEF_NULL != udpPackets )
                    {
                        m_bulkPackets.push_back( udpPackets );
                        m_bulkPacketCounts.push_back( packetCount );
                    }
                    ++i;
                }

                if ( !IsDeactivationRequested() )
                    RedisSendSync( m_bulkPackets, m_bulkPacketCounts );

                i = m_bulkMail.begin();
                while ( i != m_bulkMail.end() )
                {
                    delete static_cast< TCloneablePacketEntryVector* >( (*i).data );
                    ++i;
                }
            }
        }
        else
        {
            SendQueuedPackagesIfAny();
        }

        // If we have the mailbox filling up we should burst for more speed
        if ( m_channelSettings.maxSizeOfDedicatedRedisWriterBulkMailRead > 0                        &&
             m_mailbox.AmountOfMail() > m_channelSettings.maxSizeOfDedicatedRedisWriterBulkMailRead  )
        {
            GetPulseGenerator()->RequestImmediatePulse();
        }
    }

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::OnTaskEnding( CORE::CTaskPtr task ,
                                         bool willBeForced   )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::OnTaskEnded( CORE::CTaskPtr task ,
                                        bool wasForced      )
{GUCEF_TRACE;

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    delete m_redisReconnectTimer;
    m_redisReconnectTimer = GUCEF_NULL;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetRedisTransmitQueueSize( void ) const
{GUCEF_TRACE;

    return m_redisTransmitQueueSize;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetRedisErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisErrorReplies = m_redisErrorReplies;
        m_redisErrorReplies = 0;
        return redisErrorReplies;
    }
    else
        return m_redisErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetRedisMsgsTransmittedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisMsgsTransmitted = m_redisMsgsTransmitted;
        m_redisMsgsTransmitted = 0;
        return redisMsgsTransmitted;
    }
    else
        return m_redisMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetRedisPacketsInMsgsTransmittedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisPacketsInMsgsTransmitted = m_redisPacketsInMsgsTransmitted;
        m_redisPacketsInMsgsTransmitted = 0;
        return redisPacketsInMsgsTransmitted;
    }
    else
        return m_redisPacketsInMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ClusterChannelRedisWriter::GetRedisPacketsInMsgsRatio( void ) const
{GUCEF_TRACE;

    return m_redisPacketsInMsgsRatio;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisSendSync( const TPacketEntryVector& udpPackets ,
                                          CORE::UInt32 packetCount             )
{GUCEF_TRACE;

    TPacketEntryVectorPtrVector vecPts;
    vecPts.push_back( &udpPackets );

    TUInt32Vector vecCounts;
    vecCounts.push_back( packetCount );

    return RedisSendSync( vecPts, vecCounts );
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisSendSync( const TPacketEntryVectorPtrVector& udpPackets ,
                                          const TUInt32Vector& packetCounts             )
{GUCEF_TRACE;

    if ( !IsDeactivationRequested() && SendQueuedPackagesIfAny() )
    {
        if ( RedisSendSyncImpl( udpPackets, packetCounts ) )
        {
            return true;
        }
        else
        {
            return AddToOverflowQueue( udpPackets, packetCounts );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisSendSyncImpl( const TPacketEntryVectorPtrVector& udpPackets ,
                                              const TUInt32Vector& packetCounts             )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_redisContext )
        return false;

    try
    {
        static const CORE::CString fieldName = "UDP";
        static const CORE::CString msgId = "*";
        static const CORE::CString maxLen = "MAXLEN";

        sw::redis::StringView cnSV( m_channelSettings.channelStreamName.C_String(), m_channelSettings.channelStreamName.Length() );
        sw::redis::StringView idSV( msgId.C_String(), msgId.Length() );
        sw::redis::StringView fnSV( fieldName.C_String(), fieldName.Length() );

        CORE::UInt32 totalPacketCount = 0;
        for ( CORE::UInt32 n=0; n<packetCounts.size(); ++n )
            totalPacketCount += packetCounts[ n ];

        m_redisPacketArgs.clear();
        m_redisPacketArgs.reserve( totalPacketCount + 1 ); // +1 for potential maxlen arg

        for ( CORE::UInt32 n=0; n<packetCounts.size(); ++n )
        {
            CORE::UInt32 packetCount = packetCounts[ n ];
            const TPacketEntryVector& udpPacketSet = (*udpPackets[ n ]);

            if ( packetCount > udpPacketSet.size() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Invalid packet count of " +
                    CORE::UInt32ToString( packetCount ) + " which exceeds storage size of " + CORE::UInt64ToString( udpPacketSet.size() ) +
                    ". This should never happen or be possible. Will clamp to smaller nr." );
                packetCount = (CORE::UInt32) udpPacketSet.size();
            }

            for ( CORE::UInt32 i=0; i<packetCount; ++i )
            {
                const CORE::CDynamicBuffer& packetBuffer = udpPacketSet[ i ].dataBuffer.GetData();
                sw::redis::StringView fvSV( (const char*) packetBuffer.GetConstBufferPtr(), packetBuffer.GetDataSize() );
                m_redisPacketArgs.push_back( std::pair< sw::redis::StringView, sw::redis::StringView >( fnSV, fvSV ) );
            }
        }

        if ( m_channelSettings.redisXAddMaxLen >= 0 )
            m_redisPipeline->xadd( cnSV, idSV, m_redisPacketArgs.begin(), m_redisPacketArgs.end(), m_channelSettings.redisXAddMaxLen, m_channelSettings.redisXAddMaxLenIsApproximate );
        else
            m_redisPipeline->xadd( cnSV, idSV, m_redisPacketArgs.begin(), m_redisPacketArgs.end() );

        sw::redis::QueuedReplies redisReplies = m_redisPipeline->exec();

        bool totalSuccess = true;
        size_t replyCount = redisReplies.size();
        for ( size_t r=0; r<replyCount; ++r )
        {
            redisReply& reply = redisReplies.get( r );

            switch ( reply.integer )
            {
                case REDIS_OK:
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Successfully sent " +
                        CORE::UInt32ToString( totalPacketCount ) + " UDP messages, combining " + CORE::UInt32ToString( (CORE::UInt32)udpPackets.size() ) +
                        " sets of packages. MsgID=" + CORE::ToString( reply.str ) );

                    ++m_redisMsgsTransmitted;
                    m_redisPacketsInMsgsTransmitted += totalPacketCount;
                    m_redisPacketsInMsgsRatio = totalPacketCount;
                    break;
                }
                default:
                case REDIS_ERR:
                {
                    totalSuccess = false;
                    ++m_redisErrorReplies;

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Error sending " +
                        CORE::UInt32ToString( totalPacketCount ) + " UDP messages, combining " + CORE::UInt32ToString( (CORE::UInt32)udpPackets.size() ) +
                        " sets of packages. Error=" + CORE::ToString( reply.str ) );
                    break;
                }
            }
        }

        return totalSuccess;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::MovedError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ MovedError (Redirect failed?) . Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );
        RedisReconnect();
        return false;
    }
    catch ( const sw::redis::RedirectionError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ RedirectionError (rebalance? node failure?). Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );

        RedisReconnect();
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ exception: " + e.what() );
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: exception: " + e.what() );
        RedisReconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisSendASync( const TPacketEntryVector& udpPackets ,
                                           CORE::UInt32 packetCount             )
{GUCEF_TRACE;

    // Since the Redis library is blocking we cannot do async unless we have a thread
    if ( !m_channelSettings.performRedisWritesInDedicatedThread )
        return false;

    // We dont want to copy empty buffers needlessly so we size the mail to match the actual nr of packets
    // We need to perform a memory copy regardless because the socket needs ownership of its buffers back

    TCloneablePacketEntryVector mail;
    mail.resize( packetCount );

    // Link the external buffers into the cut-to-size mail envelope
    // The mailbox will do the cloning, we just link here
    for ( CORE::UInt32 i=0; i<packetCount; ++i )
    {
        mail[ i ].dataBuffer.LinkTo( &udpPackets[ i ].dataBuffer.GetData() );
        mail[ i ].sourceAddress = udpPackets[ i ].sourceAddress;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisSendASync: Adding group of "
            + CORE::UInt32ToString( packetCount ) + " packets to the mailbox as a single piece of mail" );

    m_mailbox.AddMail( packetCount, &mail );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::SendQueuedPackagesIfAny( void )
{GUCEF_TRACE;

    if ( !m_redisMsgQueueOverflowQueue.empty() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):SendQueuedPackagesIfAny: There are "
                + CORE::UInt32ToString( (CORE::UInt32) m_redisMsgQueueOverflowQueue.size() ) + " sets of packets to the overflow queue, Attempting to send..." );

        if ( RedisSendSyncImpl( m_redisMsgQueueOverflowQueue, m_redisMsgQueueOverflowQueueCounts ) )
        {
            for ( CORE::UInt32 n=0; n<m_redisMsgQueueOverflowQueue.size(); ++n )
            {
                delete m_redisMsgQueueOverflowQueue[ n ];
            }
            m_redisMsgQueueOverflowQueue.clear();
            m_redisMsgQueueOverflowQueueCounts.clear();
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):SendQueuedPackagesIfAny: Unable to send overflow queue packets" );
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::AddToOverflowQueue( const TPacketEntryVectorPtrVector& udpPackets ,
                                               const TUInt32Vector& packetCounts             )
{GUCEF_TRACE;

    for ( CORE::UInt32 n=0; n<packetCounts.size(); ++n )
    {
        CORE::UInt32 packetCount = packetCounts[ n ];
        const TPacketEntryVector* udpPacketSet = udpPackets[ n ];

        if ( packetCount > udpPacketSet->size() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):AddToOverflowQueue: Invalid packet count of " +
                CORE::UInt32ToString( packetCount ) + " which exceeds storage size of " + CORE::UInt64ToString( udpPacketSet->size() ) +
                ". This should never happen or be possible. Will clamp to smaller nr." );
            packetCount = (CORE::UInt32) udpPacketSet->size();
        }

        TPacketEntryVector* udpPacketSetCopy = new TPacketEntryVector( *udpPacketSet );

        m_redisMsgQueueOverflowQueue.push_back( udpPacketSetCopy );
        m_redisMsgQueueOverflowQueueCounts.push_back( packetCount );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):AddToOverflowQueue: Adding group of "
                + CORE::UInt32ToString( packetCount ) + " packets to the overflow queue" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

RedisNode::RedisNode( void )
    : host()
    , nodeId()
    , startSlot( 0 )
    , endSlot( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::GetRedisClusterNodeMap( RedisNodeMap& nodeMap )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_redisContext )
        return false;

    try
    {
        CORE::CString clusterCmd( "CLUSTER" );
        CORE::CString slotsParam( "SLOTS" );

        sw::redis::StringView clusterCmdSV( clusterCmd.C_String(), clusterCmd.Length() );
        sw::redis::StringView slotsParamSV( slotsParam.C_String(), slotsParam.Length() );

        auto reply = m_redisContext->command( clusterCmdSV, slotsParamSV );
        if ( reply )
        {
            int type = reply->type;
            if ( REDIS_REPLY_ARRAY == type )
            {
                size_t max = reply->elements;
                for ( size_t i=0; i<max; ++i )
                {
                    const struct redisReply* e = reply->element[ i ];
                    int eType = e->type;
                    if ( REDIS_REPLY_ARRAY == eType )
                    {
                        size_t eMax = e->elements;

                        long long startSlot = -1;
                        if ( eMax > 0 && REDIS_REPLY_INTEGER == e->element[ 0 ]->type )
                            startSlot = e->element[ 0 ]->integer;
                        else
                            return false;

                        long long endSlot = -1;
                        if ( eMax > 1 && REDIS_REPLY_INTEGER == e->element[ 1 ]->type )
                            endSlot = e->element[ 1 ]->integer;
                        else
                            return false;

                        for ( size_t m=2; m<eMax; ++m )
                        {
                            const struct redisReply* slotSegment = e->element[ m ];
                            int sType = slotSegment->type;
                            if ( REDIS_REPLY_ARRAY == sType )
                            {
                                size_t slotSegAttCount = slotSegment->elements;

                                CORE::CString ip;
                                if ( slotSegAttCount > 0 && REDIS_REPLY_STRING == slotSegment->element[ 0 ]->type )
                                    ip = slotSegment->element[ 0 ]->str;
                                else
                                    return false;

                                long long port = -1;
                                if ( slotSegAttCount > 1 && REDIS_REPLY_INTEGER == slotSegment->element[ 1 ]->type )
                                    port = slotSegment->element[ 1 ]->integer;
                                else
                                    return false;

                                // Read the optional Node ID if present. This depends on the Redis version
                                CORE::CString nodeId;
                                if ( slotSegAttCount > 2 && REDIS_REPLY_STRING == slotSegment->element[ 2 ]->type )
                                    nodeId = slotSegment->element[ 2 ]->str;

                                RedisNode& node = nodeMap[ (CORE::UInt32) startSlot ];
                                node.startSlot = (CORE::UInt32) startSlot;
                                node.endSlot = (CORE::UInt32) endSlot;
                                node.nodeId = nodeId;
                                node.host.SetHostnameAndPort( ip, (UInt16) port );
                            }
                        }
                    }
                }
            }
            else
            {
                if ( REDIS_REPLY_ERROR == type )
                {
                    ++m_redisErrorReplies;
                    return false;
                }
            }
        }

    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: Redis++ exception: " + e.what() );
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: exception: " + e.what() );
        RedisReconnect();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisDisconnect( void )
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL != m_redisContext )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisDisconnect: Beginning cleanup" );

            delete m_redisPipeline;
            m_redisPipeline = GUCEF_NULL;

            delete m_redisContext;
            m_redisContext = GUCEF_NULL;

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisDisconnect: Finished cleanup" );
        }
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisDisconnect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisDisconnect: Redis++ exception: " + e.what() );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisDisconnect: exception: " + e.what() );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::RedisConnect( void )
{GUCEF_TRACE;

    RedisDisconnect();

    try
    {
        m_channelSettings.redisAddress.Refresh();
        
        sw::redis::ConnectionOptions rppConnectionOptions;
        rppConnectionOptions.host = m_channelSettings.redisAddress.GetHostname();  // Required.
        rppConnectionOptions.port = m_channelSettings.redisAddress.GetPortInHostByteOrder(); // Optional. The default port is 6379.
        //rppConnectionOptions.password = "auth";   // Optional. No password by default.
        //rppConnectionOptions.db = 1;  // Optional. Use the 0th database by default.

        // Optional. Timeout before we successfully send request to or receive response from redis.
        // By default, the timeout is 0ms, i.e. never timeout and block until we send or receive successfuly.
        // NOTE: if any command is timed out, we throw a TimeoutError exception.
        rppConnectionOptions.socket_timeout = std::chrono::milliseconds( m_channelSettings.redisConnectionOptionSocketTimeoutInMs );
        rppConnectionOptions.connect_timeout = std::chrono::milliseconds( m_channelSettings.redisConnectionOptionConnectTimeoutInMs );

        rppConnectionOptions.keep_alive = m_channelSettings.redisConnectionOptionKeepAlive;

        // Connect to the Redis cluster
        delete m_redisContext;
        m_redisContext = new sw::redis::RedisCluster( rppConnectionOptions );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Successfully created a Redis context" );

        // The following is not a must-have for connectivity
        RedisNodeMap nodeMap;
        if ( GetRedisClusterNodeMap( nodeMap ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Successfully obtained Redis cluster nodes" );

            RedisNodeMap::iterator i = nodeMap.begin();
            while ( i != nodeMap.end() )
            {
                if ( (*i).first > m_redisHashSlot )
                    break;
                if ( m_redisHashSlot >= (*i).first && m_redisHashSlot <= (*i).second.endSlot )
                {
                    m_redisShardHost = (*i).second.host;
                    m_redisShardNodeId = (*i).second.nodeId;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Stream \"" + m_channelSettings.channelStreamName +
                        "\" hashes to hash slot " + CORE::ToString( m_redisHashSlot ) + " which lives at " + (*i).second.host.HostnameAndPortAsString() + " with node id " + (*i).second.nodeId );
                    break;
                }
                ++i;
            }
        }

        // Connect to the specific shard used for this channel's stream with a single dedicated connection
        sw::redis::StringView cnSV( m_channelSettings.channelStreamName.C_String(), m_channelSettings.channelStreamName.Length() );
        delete m_redisPipeline;
        m_redisPipeline = new sw::redis::Pipeline( m_redisContext->pipeline( cnSV ) );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Successfully created a Redis pipeline. Hash Slot " + CORE::ToString( m_redisHashSlot ) );
        return true;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: Redis++ exception: " + e.what() );
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::PointerToString( this ) + "):RedisConnect: exception: " + e.what() );
        RedisReconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
ClusterChannelRedisWriter::RedisReconnect( void )
{GUCEF_TRACE;

    RedisDisconnect();    
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ClusterChannelRedisWriter(" + CORE::ToString( this ) + "):RedisReconnect: starting reconnect timer" );
    m_redisReconnectTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
ClusterChannelRedisWriter::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    m_redisHashSlot = CalculateRedisHashSlot( m_channelSettings.channelStreamName );
    return true;
}

/*-------------------------------------------------------------------------*/

const ChannelSettings&
ClusterChannelRedisWriter::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

Udp2RedisClusterChannel::Udp2RedisClusterChannel()
    : CORE::CTaskConsumer()
    , m_channelSettings()
    , m_udpSocket( GUCEF_NULL )
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_redisWriter( new ClusterChannelRedisWriter() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2RedisClusterChannel::Udp2RedisClusterChannel( const Udp2RedisClusterChannel& src )
    : CORE::CTaskConsumer()
    , m_channelSettings( src.m_channelSettings )
    , m_udpSocket( GUCEF_NULL )
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_redisWriter( new ClusterChannelRedisWriter() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2RedisClusterChannel::~Udp2RedisClusterChannel()
{GUCEF_TRACE;

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    delete m_udpSocket;
    m_udpSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Register UDP socket event handlers
    TEventCallback callback( this, &Udp2RedisClusterChannel::OnUDPSocketError );
    SubscribeTo( m_udpSocket                              ,
                 COMCORE::CUDPSocket::UDPSocketErrorEvent ,
                 callback                                 );
    TEventCallback callback2( this, &Udp2RedisClusterChannel::OnUDPSocketClosed );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketClosedEvent ,
                 callback2                                 );
    TEventCallback callback3( this, &Udp2RedisClusterChannel::OnUDPSocketClosing );
    SubscribeTo( m_udpSocket                                ,
                 COMCORE::CUDPSocket::UDPSocketClosingEvent ,
                 callback3                                  );
    TEventCallback callback4( this, &Udp2RedisClusterChannel::OnUDPSocketOpened );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketOpenedEvent ,
                 callback4                                 );
    TEventCallback callback5( this, &Udp2RedisClusterChannel::OnUDPPacketsRecieved );
    SubscribeTo( m_udpSocket                                  ,
                 COMCORE::CUDPSocket::UDPPacketsRecievedEvent ,
                 callback5                                    );

    TEventCallback callback6( this, &Udp2RedisClusterChannel::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisClusterChannel::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    return m_redisWriter->LoadConfig( channelSettings );
}

/*-------------------------------------------------------------------------*/

const ChannelSettings&
Udp2RedisClusterChannel::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Udp2RedisClusterChannel::GetType( void ) const
{GUCEF_TRACE;

    return "Udp2RedisClusterChannel";
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisClusterChannel::WaitForTaskToFinish( CORE::Int32 timeoutInMs )
{GUCEF_TRACE;

    // Overriding the base class implementation because this consumer can start its own
    // consumer based on settings transparent to the caller.
    if ( CTaskConsumer::WaitForTaskToFinish( timeoutInMs ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:WaitForTaskToFinish: Successfully waited for channel " + CORE::Int32ToString( m_channelSettings.channelId ) + "'s task to stop" );
        if ( m_channelSettings.performRedisWritesInDedicatedThread )
        {
            if ( m_redisWriter->WaitForTaskToFinish( timeoutInMs ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:WaitForTaskToFinish: Successfully waited for channel " + CORE::Int32ToString( m_channelSettings.channelId ) + "'s dedicated redis writer task to stop" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:WaitForTaskToFinish: Failed waiting for dedicated redis writer task to stop for channel " + CORE::Int32ToString( m_channelSettings.channelId ) );
                return false;
            }
        }
        return true;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:WaitForTaskToFinish: Failed waiting for task to stop for channel " + CORE::Int32ToString( m_channelSettings.channelId ) );
    return false;
}

/*-------------------------------------------------------------------------*/

Udp2RedisClusterChannel::ChannelMetrics::ChannelMetrics( void )
    : udpBytesReceived( 0 )
    , udpPacketsReceived( 0 )
    , redisMessagesTransmitted( 0 )
    , redisPacketsInMsgsTransmitted( 0 )
    , redisPacketsInMsgsRatio( 0 )
    , redisTransmitQueueSize( 0 )
    , redisErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_metrics.udpBytesReceived = m_udpSocket->GetBytesReceived( true );
    m_metrics.udpPacketsReceived = m_udpSocket->GetNrOfDataReceivedEvents( true );
    m_metrics.redisTransmitQueueSize = m_redisWriter->GetRedisTransmitQueueSize();
    m_metrics.redisMessagesTransmitted = m_redisWriter->GetRedisMsgsTransmittedCounter( true );
    m_metrics.redisPacketsInMsgsTransmitted = m_redisWriter->GetRedisPacketsInMsgsTransmittedCounter( true );
    m_metrics.redisPacketsInMsgsRatio = m_redisWriter->GetRedisPacketsInMsgsRatio();
    m_metrics.redisErrorReplies = m_redisWriter->GetRedisErrorRepliesCounter( true );
}

/*-------------------------------------------------------------------------*/

const Udp2RedisClusterChannel::ChannelMetrics&
Udp2RedisClusterChannel::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnUDPSocketError( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventID  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CUDPSocket::TSocketErrorEventData* eData = static_cast< COMCORE::CUDPSocket::TSocketErrorEventData* >( eventData );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel: UDP Socket experienced error " + CORE::Int32ToString( eData->GetData() ) );

    m_channelSettings.udpInterface.Refresh();
    ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
    while ( m != m_channelSettings.udpMulticastToJoin.end() )
    {
        COMCORE::CHostAddress& multicastAddr = (*m);
        multicastAddr.Refresh();
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                                            const CORE::CEvent& eventID ,
                                            CORE::CICloneable* evenData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel: UDP Socket has been closed" );
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnUDPSocketClosing( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventID  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel: UDP Socket is going to close" );

    // Gracefully leave the multicast groups we joined
    ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
    while ( m != m_channelSettings.udpMulticastToJoin.end() )
    {
        const COMCORE::CHostAddress& multicastAddr = (*m);
        if ( m_udpSocket->Leave( multicastAddr.GetFirstIPv4Address() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel:OnUDPSocketClosing: Successfully to left multicast " + multicastAddr.GetFirstAddressAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnUDPSocketClosing: Failed to leave multicast " + multicastAddr.GetFirstAddressAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
        }
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                                            const CORE::CEvent& eventID ,
                                            CORE::CICloneable* evenData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel: UDP Socket has been opened" );

    ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
    while ( m != m_channelSettings.udpMulticastToJoin.end() )
    {
        const COMCORE::CHostAddress& multicastAddr = (*m);
        if ( m_udpSocket->Join( multicastAddr.GetFirstIPv4Address() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel:OnUDPSocketOpened: Successfully to joined multicast " + multicastAddr.GetFirstAddressAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnUDPSocketOpened: Failed to join multicast " + multicastAddr.GetFirstAddressAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
        }
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                                               const CORE::CEvent& eventID ,
                                               CORE::CICloneable* evenData )
{GUCEF_TRACE;

    COMCORE::CUDPSocket::UDPPacketsRecievedEventData* udpPacketData = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( evenData );
    if ( GUCEF_NULL != udpPacketData )
    {
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = udpPacketData->GetData();

        CORE::UInt32 packetsReceived = data.packetsReceived;
        if ( packetsReceived > data.packets.size() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnUDPPacketsRecieved: Invalid packet count of " +
                CORE::UInt32ToString( packetsReceived ) + " which exceeds storage size of " + CORE::UInt64ToString( data.packets.size() ) +
                ". This should never happen or be possible. Will clamp to smaller nr." );
            packetsReceived = (CORE::UInt32) data.packets.size();
        }

        if ( m_channelSettings.performRedisWritesInDedicatedThread )
        {
            if ( !m_redisWriter->RedisSendASync( data.packets, packetsReceived ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnUDPPacketsRecieved: Failed to submit " +
                        CORE::UInt32ToString( packetsReceived ) + " packets for async Redis transmission. Data loss will occur!!!" );
            }
        }
        else
        {
            if ( !m_redisWriter->RedisSendSync( data.packets, packetsReceived ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnUDPPacketsRecieved: Failed to request sync blocking transmission of " +
                        CORE::UInt32ToString( packetsReceived ) + " packets to Redis. Data loss will occur!!!" );
            }
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel: UDP Socket has a data received event but no data was provided" );
    }
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisClusterChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( m_channelSettings.performRedisWritesInDedicatedThread )
    {
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
        if ( !threadPool->StartTask( m_redisWriter ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisClusterChannel:OnTaskStart: Failed to start dedicated task (dedicated thread) for Redis writes. Falling back to a single thread" );
            m_channelSettings.performRedisWritesInDedicatedThread = false;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel:OnTaskStart: Successfully requested the launch of a dedicated task (dedicated thread) for Redis writes" );
        }
    }
    else
    {
        if ( !m_redisWriter->OnTaskStart( task ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisClusterChannel:OnTaskStart: Failed startup of Redis writer logic" );
            return false;
        }
    }

    GUCEF_DELETE m_udpSocket;
    m_udpSocket = new COMCORE::CUDPSocket( GetPulseGenerator(), false );
    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );
    m_metricsTimer->SetEnabled( m_channelSettings.collectMetrics );

    // Set the minimum number of cycles we will go full speed if a single cycle was not enough to handle
    // all the processing. This will cause a bypass of CPU yielding if/when the situation arises.
    // In such a case the thread will run at max speed for a least the below set nr of cycles.
    GetPulseGenerator()->RequestPulsesPerImmediatePulseRequest( m_channelSettings.ticketRefillOnBusyCycle );

    // Default smallest pulse delta at 10ms
    GetPulseGenerator()->RequestPeriodicPulses( this, 10 );

    if ( m_channelSettings.applyThreadCpuAffinity )
    {
        if ( SetCpuAffinityByCpuId( m_channelSettings.cpuAffinityForMainChannelThread ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel(" + CORE::PointerToString( this ) +
                "):OnTaskStart: Successfully set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForMainChannelThread ) );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel(" + CORE::PointerToString( this ) +
                "):OnTaskStart: Failed to set a CPU affinity for logical CPU " + CORE::UInt32ToString( m_channelSettings.cpuAffinityForMainChannelThread ) +
                ". Proceeding without affinity");
        }
    }

    RegisterEventHandlers();

    // Configure and open the UDP port.
    // Note that if there is an error here we will just keep on trying automatically
    m_udpSocket->SetMaxUpdatesPerCycle( m_channelSettings.udpSocketUpdateCyclesPerPulse );
    m_udpSocket->SetOsLevelSocketReceiveBufferSize( m_channelSettings.udpSocketOsReceiveBufferSize );
    m_udpSocket->SetNrOfReceiveBuffers( m_channelSettings.nrOfUdpReceiveBuffersPerSocket );
    m_udpSocket->SetAutoReOpenOnError( true );
    if ( m_udpSocket->Open( m_channelSettings.udpInterface.GetFirstIPv4Address() ) )
    {
		GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel:OnTaskStart: Successfully opened UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisClusterChannel:OnTaskStart: Failed to open UDP socket on " + m_channelSettings.udpInterface.GetFirstAddressAndPortAsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisClusterChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( !m_channelSettings.performRedisWritesInDedicatedThread )
    {
        m_redisWriter->OnTaskCycle( task );
    }

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnTaskEnding( CORE::CTaskPtr task ,
                                       bool willBeForced   )
{GUCEF_TRACE;

    if ( !m_channelSettings.performRedisWritesInDedicatedThread )
    {
        m_redisWriter->OnTaskEnding( task, willBeForced );
    }
    else
    {
        // Since we are the ones that launched the dedicated Redis write thread we should also ask
        // to have it cleaned up when we are shutting down this thread
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
        if ( !threadPool->RequestTaskToStop( m_redisWriter.StaticCast< CORE::CTaskConsumer >(), false ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisClusterChannel:OnTaskEnding: Failed to request the dedicated task (dedicated thread) for Redis writes to stop" );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisClusterChannel:OnTaskEnding: Successfully requested the dedicated task (dedicated thread) for Redis writes to stop" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisClusterChannel::OnTaskEnded( CORE::CTaskPtr task ,
                                      bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DELETE m_udpSocket;
    m_udpSocket = GUCEF_NULL;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    if ( !m_channelSettings.performRedisWritesInDedicatedThread )
    {
        m_redisWriter->OnTaskEnded( task, wasForced );
    }

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisInfoResource::RestApiUdp2RedisInfoResource( Udp2RedisCluster* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisInfoResource::~RestApiUdp2RedisInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2RedisInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                         CORE::CDataNode& output             ,
                                         const CORE::CString& representation ,
                                         const CORE::CString& params         )
{GUCEF_TRACE;

    static const CORE::CDateTime compileDt = CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ );

    output.SetName( "info" );
    output.SetAttribute( "application", "udp2rediscluster" );
    output.SetAttribute( "appBuildDateTime", compileDt.ToIso8601DateTimeString( true, true ) );
    output.SetAttribute( "platformBuildDateTime", CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( false ) );
    #else
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( true ) );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::RestApiUdp2RedisConfigResource( Udp2RedisCluster* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::~RestApiUdp2RedisConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2RedisConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                           CORE::CDataNode& output             ,
                                           const CORE::CString& representation ,
                                           const CORE::CString& params         )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const CORE::CDataNode* appConfigData = m_app->GetAppConfig();
        if ( GUCEF_NULL != appConfigData )
        {
            output.Copy( *appConfigData );
            return true;
        }
        return false;
    }
    else
    {
        const CORE::CDataNode& globalConfigData = m_app->GetGlobalConfig();
        output.Copy( globalConfigData );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::TDeserializeState
RestApiUdp2RedisConfigResource::UpdateGlobalConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    // First put the app in standby mode before we mess with the settings
    if ( !m_app->SetStandbyMode( true ) )
        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;

    // Grab a copy of the current global config for rollback if needed
    CORE::CDataNode oldGlobalConfig = m_app->GetGlobalConfig();
    const CORE::CDataNode* oldAppConfigData = m_app->GetAppConfig( oldGlobalConfig );

    if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( cfg ) )
    {
        const CORE::CDataNode* appConfigData = m_app->GetAppConfig();
        if ( GUCEF_NULL == appConfigData )
        {
            // We should not lose access to the app config section, this indicates a serious issue
            // Attempt to roll back so we are not entirely broken
            if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( oldGlobalConfig ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiUdp2RedisConfigResource: Failed to obtain access to app config after applying new consolidated global config. Successfully rolled back to previous global config" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RestApiUdp2RedisConfigResource: Failed to obtain access to app config after applying new consolidated global config and subsequently also failed to roll back to previous global config" );
            }

            return TDeserializeState::DESERIALIZESTATE_CORRUPTEDINPUT;
        }

        if ( GUCEF_NULL != oldAppConfigData )
        {
            bool allowRestApiAppGlobalConfigToPersist = appConfigData->GetAttributeValueOrChildValueByName( "allowPeristanceOfNewGlobalConfigViaRestApi" ).AsBool( false, true );
            if ( allowRestApiAppGlobalConfigToPersist )
            {
                if ( CORE::CCoreGlobal::Instance()->GetConfigStore().SaveConsolidatedConfig( cfg ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiUdp2RedisConfigResource: Successfully saved new consolidated global config" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiUdp2RedisConfigResource: Failed to save new consolidated global config" );
                }
            }
        }

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
        // We ran into an issue applying the config
        // Attempt to roll back so we are not entirely broken
        if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( oldGlobalConfig ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiUdp2RedisConfigResource: Failed to apply new global config. Successfully rolled back to previous global config" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RestApiUdp2RedisConfigResource: Failed to apply new global config and subsequently also failed to roll back to previous global config" );
        }
        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
    }
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::TDeserializeState
RestApiUdp2RedisConfigResource::Deserialize( const CORE::CString& resourcePath   ,
                                             const CORE::CDataNode& input        ,
                                             const CORE::CString& representation ,
                                             bool isDeltaUpdateOnly              )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        // Make a copy of the entire global cfg
        CORE::CDataNode globalCfg = m_app->GetGlobalConfig();
        CORE::CDataNode* appConfigData = m_app->GetAppConfig( globalCfg );

        if ( isDeltaUpdateOnly )
        {
            // Not supported yet. @TODO
            // requires proper tree merging support

            return TDeserializeState::DESERIALIZESTATE_NOTSUPPORTED;
        }
        else
        {
            // Provided input content replaces the entire app config segment
            // We also protect against dropping the node name if only the values are provided in this subset
            CORE::CString nodeName = appConfigData->GetName();
            *appConfigData = input;
            if ( appConfigData->GetName().IsNULLOrEmpty() )
                appConfigData->SetName( nodeName );

        }
        return UpdateGlobalConfig( globalCfg );
    }
    else
    {
        if ( isDeltaUpdateOnly )
        {
            // Not supported yet. @TODO
            // requires proper tree merging support

            return TDeserializeState::DESERIALIZESTATE_NOTSUPPORTED;
        }
        else
        {
            return UpdateGlobalConfig( input );
        }
    }
}

/*-------------------------------------------------------------------------*/

Udp2RedisCluster::Udp2RedisCluster( void )
    : CORE::CObserver()
    , CORE::CGloballyConfigurable()
    , m_isInStandby( false )
    , m_globalStandbyEnabled( false )
    , m_udpStartPort( 20000 )
    , m_channelCount( 0 )
    , m_redisStreamStartChannelID( 1 )
    , m_redisStreamName( "udp-ingress-ch{channelID}" )
    , m_redisHost( "127.0.0.1" )
    , m_redisPort( 6379 )
    , m_channels()
    , m_channelSettings()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_globalConfig()
    , m_metricsTimer()
    , m_transmitMetrics( true )
    , m_testUdpSocket( false )
    , m_testPacketTransmitTimer()
    , m_transmitTestPackets( false )
{GUCEF_TRACE;

    TEventCallback callback1( this, &Udp2RedisCluster::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );

    TEventCallback callback2( this, &Udp2RedisCluster::OnTransmitTestPacketTimerCycle );
    SubscribeTo( &m_testPacketTransmitTimer     ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );
}

/*-------------------------------------------------------------------------*/

Udp2RedisCluster::~Udp2RedisCluster()
{GUCEF_TRACE;

    m_httpServer.Close();
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisCluster::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_globalStandbyEnabled;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisCluster::Start( void )
{GUCEF_TRACE;

    m_isInStandby = true;
    bool success = SetStandbyMode( m_globalStandbyEnabled );

    if ( success )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster: Opening REST API" );
        success =  m_httpServer.Listen();

        if ( success )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster: REST API now available on port " + CORE::ToString( m_httpServer.GetPort() ) );
            return true;
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster: Failed to listen on port " + CORE::ToString( m_httpServer.GetPort() ) + " for REST API" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisCluster::SetStandbyMode( bool putInStandbyMode )
{GUCEF_TRACE;

    // Check if we need to do anything
    if ( m_isInStandby == putInStandbyMode )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( " + CORE::BoolToString( putInStandbyMode ) + " ): Already in the desired mode (" + CORE::BoolToString( m_isInStandby ) + "), nothing to do" );
        return true;
    }
    
    // Before we do anything drastic make sure our logs are flushed to a known point to help diagnostics
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();

    if ( putInStandbyMode )
    {
        bool totalSuccess = true;
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        m_testPacketTransmitTimer.SetEnabled( false );

        // Signal all channel threads to stop gracefully
        // Since this standby operation is global not per channel we signal all to stop before
        // we starting any waiting operation
        Udp2RedisClusterChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            Udp2RedisClusterChannelPtr channel = (*i).second;
            if ( !threadPool->RequestTaskToStop( channel.StaticCast< CORE::CTaskConsumer >(), false ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( true ): Requested channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        // Now actually wait for the threads to be finished
        i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            Udp2RedisClusterChannelPtr channel = (*i).second;
            if ( !channel->WaitForTaskToFinish( 30000 ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( true ): Successfully waited for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        m_metricsTimer.SetEnabled( false );

        m_isInStandby = totalSuccess;

        // After anything drastic make sure our logs are flushed to a known point to help diagnostics
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();

        return totalSuccess;
    }
    else
    {
        bool totalSuccess = true;

        // Channel config could have changed such that we need to remove channels that should no longer exist
        Udp2RedisClusterChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CORE::Int32 channelId = (*i).first;
            ChannelSettingsMap::iterator n = m_channelSettings.find( channelId );
            if ( n == m_channelSettings.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( false ): Found channel which no longer has corresponding channel settings, deleting channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels.erase( i );
                i = m_channels.begin();
                continue;
            }
            ++i;
        }

        // Alternatively channel config could have changed such that we have new channels
        ChannelSettingsMap::iterator n = m_channelSettings.begin();
        while ( n != m_channelSettings.end() )
        {
            CORE::Int32 channelId = (*n).first;
            Udp2RedisClusterChannelMap::iterator i = m_channels.find( channelId );
            if ( i == m_channels.end() )
            {
                // This is a brand new channel. Lets add the channel object for it
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster:SetStandbyMode( false ): Found channel settings whith no corresponding channel object, creating new channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels[ channelId ] = Udp2RedisClusterChannelPtr( new Udp2RedisClusterChannel() );
            }
            ++n;
        }

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        n = m_channelSettings.begin();
        while ( n != m_channelSettings.end() )
        {
            CORE::Int32 channelId = (*n).first;
            Udp2RedisClusterChannelMap::iterator i = m_channels.find( channelId );
            if ( i != m_channels.end() )
            {
                const ChannelSettings& channelSettings = (*n).second;
                Udp2RedisClusterChannelPtr channel = (*i).second;

                if ( !channel->LoadConfig( channelSettings ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::SetStandbyMode( false ): Failed to set channel settings on channel " + CORE::Int32ToString( channelId ) );
                    totalSuccess = false;
                    break;
                }

                if ( !threadPool->StartTask( channel ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::SetStandbyMode( false ): Failed to start task (dedicated thread) for channel " + CORE::Int32ToString( channelId ) );
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
        m_testPacketTransmitTimer.SetEnabled( m_transmitTestPackets );

        m_isInStandby = !totalSuccess;

        // After anything drastic make sure our logs are flushed to a known point to help diagnostics
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();

        return totalSuccess;
    }
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisCluster::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    const CORE::CDataNode* appConfig = GetAppConfig( globalConfig );
    if ( GUCEF_NULL == appConfig )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: Unable to locate app config in global config" );
        return false;
    }

    m_transmitMetrics = appConfig->GetAttributeValueOrChildValueByName( "transmitMetrics" ).AsBool( m_transmitMetrics, true );
    m_globalStandbyEnabled = appConfig->GetAttributeValueOrChildValueByName( "globalStandbyEnabled" ).AsBool( m_globalStandbyEnabled, true );

    m_redisStreamStartChannelID = appConfig->GetAttributeValueOrChildValueByName( "redisStreamStartChannelID" ).AsInt32( m_redisStreamStartChannelID, true );

    CORE::CString::StringSet channelIDStrs = appConfig->GetAttributeValueOrChildValueByName( "channelIDs" ).AsString( CORE::CString::Empty, true ).ParseUniqueElements( ',', false );
    m_channelCount = appConfig->GetAttributeValueOrChildValueByName( "channelCount" ).AsUInt16( (UInt16) channelIDStrs.empty() ? 1 : (UInt16) channelIDStrs.size(), true );

    m_udpStartPort = appConfig->GetAttributeValueOrChildValueByName( "udpStartPort" ).AsUInt16( m_udpStartPort, true );
    CORE::Int32 udpPortChannelIdOffset = appConfig->GetAttributeValueOrChildValueByName( "udpPortChannelIdOffset" ).AsInt32( 20000, true );
    bool useUdpPortChannelIdOffset = appConfig->GetAttributeValueOrChildValueByName( "useUdpPortChannelIdOffset" ).AsBool( false, true );
    if ( useUdpPortChannelIdOffset )
        m_udpStartPort = (CORE::UInt16) ( m_redisStreamStartChannelID + udpPortChannelIdOffset );

    m_redisStreamName = appConfig->GetAttributeValueOrChildValueByName( "redisStreamName" ).AsString( m_redisStreamName, true );
    m_redisHost = appConfig->GetAttributeValueOrChildValueByName( "redisHost" ).AsString( m_redisHost, true );
    m_redisPort = appConfig->GetAttributeValueOrChildValueByName( "redisPort" ).AsUInt16( m_redisPort, true );

    CORE::UInt32 ticketRefillOnBusyCycle = appConfig->GetAttributeValueOrChildValueByName( "ticketRefillOnBusyCycle" ).AsUInt32( GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE, true );
    CORE::UInt32 nrOfUdpReceiveBuffersPerSocket = appConfig->GetAttributeValueOrChildValueByName( "nrOfUdpReceiveBuffersPerSocket" ).AsUInt32( GUCEF_DEFAULT_UDP_RECEIVE_BUFFERS, true );
    CORE::UInt32 udpSocketOsReceiveBufferSize = appConfig->GetAttributeValueOrChildValueByName( "udpSocketOsReceiveBufferSize" ).AsUInt32( GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE, true );
    CORE::UInt32 udpSocketUpdateCyclesPerPulse = appConfig->GetAttributeValueOrChildValueByName( "maxUdpSocketUpdateCyclesPerPulse" ).AsUInt32( GUCEF_DEFAULT_UDP_MAX_SOCKET_CYCLES_PER_PULSE, true );
    bool performRedisWritesInDedicatedThread = appConfig->GetAttributeValueOrChildValueByName( "performRedisWritesInDedicatedThread" ).AsBool( true, true );
    CORE::Int32 maxSizeOfDedicatedRedisWriterBulkMailRead = appConfig->GetAttributeValueOrChildValueByName( "maxSizeOfDedicatedRedisWriterBulkMailRead" ).AsInt32( GUCEF_DEFAULT_MAX_DEDICATED_REDIS_WRITER_MAIL_BULK_READ, true );
    bool applyCpuThreadAffinityByDefault = appConfig->GetAttributeValueOrChildValueByName( "applyCpuThreadAffinityByDefault" ).AsBool( false, true );
    CORE::Int32 redisXAddMaxLenDefault = appConfig->GetAttributeValueOrChildValueByName( "redisXAddMaxLen" ).AsInt32( -1, true );
    bool redisXAddMaxLenIsApproxDefault = appConfig->GetAttributeValueOrChildValueByName( "redisXAddMaxLenIsApproximate" ).AsBool( true, true );

    CORE::UInt32 logicalCpuCount = CORE::GetLogicalCPUCount();

    // We will assume we are always given a full not a partial config so we clear the existing channel settings
    m_channelSettings.clear();

    // Validate the channel IDs.
    // Depending on the use case these could be vital identifiers not just an index so some validation is in order
    Int32Set channelIDs;
    CORE::CString::StringSet::iterator n = channelIDStrs.begin();
    while ( n != channelIDStrs.end() )
    {
        const CORE::CString& str = (*n);
        if ( str.HasChar( '-' ) >= 0 )
        {
            CORE::Int32 startId = GUCEF_MT_INT32MIN;
            CORE::Int32 endId = GUCEF_MT_INT32MIN;

            CORE::CString::StringVector rangeStrs = str.ParseElements( '-', false );
            if ( rangeStrs.size() > 0 )
                startId = CORE::StringToInt32( rangeStrs[ 0 ], GUCEF_MT_INT32MIN );
            if ( rangeStrs.size() > 1 )
                endId = CORE::StringToInt32( rangeStrs[ 1 ], GUCEF_MT_INT32MIN );

            if ( startId != GUCEF_MT_INT32MIN && endId != GUCEF_MT_INT32MIN )
            {
                if ( endId < startId )
                {
                    CORE::Int32 tmp = startId;
                    startId = endId;
                    endId = tmp;
                }

                for ( CORE::Int32 i=startId; i<=endId; i++ )
                    channelIDs.insert( i );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: Invalid channel ID range provided. Fix the config. Str: " + str );
                return false;
            }
        }
        else
        {
            CORE::Int32 id = CORE::StringToInt32( str, GUCEF_MT_INT32MIN );
            if ( id != GUCEF_MT_INT32MIN )
                channelIDs.insert( id );
        }
        ++n;
    }
    if ( channelIDs.size() < channelIDStrs.size() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: Only " + CORE::ToString( channelIDs.size() ) + " numerical channel IDs were obtained from the channel list which contained more strings. Fix the config" );
        return false;
    }

    // Make sure we have enough channel IDs specified to cover the channel count
    if ( channelIDs.size() < m_channelCount )
    {
        // auto-generate additional channel IDs
        // This allows a configuration style where you don't have to specify all channel IDs
        CORE::Int32 lastAutoGenChannelId = m_redisStreamStartChannelID;
        CORE::Int32 missingChannels = m_channelCount - (CORE::Int32) channelIDs.size();
        for ( CORE::Int32 i=0; i<missingChannels; ++i )
        {
            while ( channelIDs.find( lastAutoGenChannelId ) != channelIDs.end() )
                ++lastAutoGenChannelId;
            channelIDs.insert( lastAutoGenChannelId );
            ++lastAutoGenChannelId;
        }
    }
    else
    if ( m_channelCount < channelIDs.size() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: " + CORE::ToString( channelIDs.size() ) + " numerical channel IDs were provided but a total channel count of " +
            CORE::ToString( m_channelCount ) + " was configured. Channel count will be increased to match the nr of IDs" );
        m_channelCount = (CORE::UInt16) channelIDs.size();
    }

    CORE::UInt32 currentCpu = 0;
    CORE::UInt16 udpPort = m_udpStartPort;
    Int32Set::iterator idListIttr = channelIDs.begin();
    for ( CORE::Int32 i=0; i<m_channelCount; ++i )
    {
        CORE::Int32 channelId = (*idListIttr);
        ChannelSettings& channelSettings = m_channelSettings[ channelId ];

        if ( useUdpPortChannelIdOffset )
            udpPort = (CORE::UInt16) ( channelId + udpPortChannelIdOffset );

        channelSettings.channelId = channelId;
        channelSettings.collectMetrics = m_transmitMetrics;
        channelSettings.redisAddress.SetHostname( m_redisHost );
        channelSettings.redisAddress.SetPortInHostByteOrder( m_redisPort );
        channelSettings.ticketRefillOnBusyCycle = ticketRefillOnBusyCycle;
        channelSettings.nrOfUdpReceiveBuffersPerSocket = nrOfUdpReceiveBuffersPerSocket;
        channelSettings.udpSocketOsReceiveBufferSize = udpSocketOsReceiveBufferSize;
        channelSettings.udpSocketUpdateCyclesPerPulse = udpSocketUpdateCyclesPerPulse;
        channelSettings.performRedisWritesInDedicatedThread = performRedisWritesInDedicatedThread;
        channelSettings.maxSizeOfDedicatedRedisWriterBulkMailRead = maxSizeOfDedicatedRedisWriterBulkMailRead;
        channelSettings.redisXAddMaxLen = redisXAddMaxLenDefault;
        channelSettings.redisXAddMaxLenIsApproximate = redisXAddMaxLenIsApproxDefault;

        CORE::CString settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".redisStreamName";
        CORE::CString settingValue = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsString( CORE::CString::Empty, true );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.channelStreamName = CORE::ResolveVars( settingValue.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }
        else
        {
            // Use the auto naming and numbering scheme based on a single template name instead
            channelSettings.channelStreamName = CORE::ResolveVars( m_redisStreamName.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".applyCpuThreadAffinity";
        settingValue = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsString( CORE::CString::Empty, true );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.applyThreadCpuAffinity = CORE::StringToBool( CORE::ResolveVars( settingValue ), applyCpuThreadAffinityByDefault );
        }
        else
        {
            channelSettings.applyThreadCpuAffinity = applyCpuThreadAffinityByDefault;
        }

        if ( channelSettings.applyThreadCpuAffinity )
        {
            settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".cpuAffinityForMainChannelThread";
            settingValue = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsString( CORE::CString::Empty, true );
            if ( !settingValue.IsNULLOrEmpty() )
            {
                channelSettings.cpuAffinityForMainChannelThread = CORE::StringToUInt32( CORE::ResolveVars( settingValue ), currentCpu );
            }
            else
            {
                channelSettings.cpuAffinityForMainChannelThread = currentCpu;
            }

            ++currentCpu;
            if ( currentCpu >= logicalCpuCount ) // Wrap around if we run out of CPUs
                currentCpu = 0;

            if ( channelSettings.performRedisWritesInDedicatedThread )
            {
                settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".cpuAffinityForDedicatedRedisWriterThread";
                settingValue = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsString( CORE::CString::Empty, true );
                if ( !settingValue.IsNULLOrEmpty() )
                {
                    channelSettings.cpuAffinityForDedicatedRedisWriterThread = CORE::StringToUInt32( CORE::ResolveVars( settingValue ), currentCpu );
                }
                else
                {
                    channelSettings.cpuAffinityForDedicatedRedisWriterThread = currentCpu;
                }

                ++currentCpu;
                if ( currentCpu >= logicalCpuCount ) // Wrap around if we run out of CPUs
                    currentCpu = 0;
            }
        }

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".udpInterface";
        settingValue = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsString( CORE::CString::Empty, true );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.udpInterface.SetHostnameAndPort( settingValue );
        }
        else
        {
            // Use the auto numbering scheme instead
            // This is either a offset of the channel ID or its an auto-incremented nr from a base port nr
            // take care to use the ANY interface since we dont have an explicit NIC interface specified
            channelSettings.udpInterface.SetHostnameAndPort( CORE::CString::Empty, udpPort );
        }

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".wantsTestPackage";
        channelSettings.wantsTestPackage = appConfig->GetAttributeValueOrChildValueByName( settingName ).AsBool( false, true );

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".multicast.Join";
        CORE::CString::StringVector settingValues = CORE::ToStringVector( appConfig->GetAttributeValueOrChildValuesByName( settingName ) );
        CORE::CString::StringVector::iterator n = settingValues.begin();
        while ( n != settingValues.end() )
        {
            const CORE::CString& settingValue = (*n);
            COMCORE::CHostAddress multicastAddress( settingValue );
            channelSettings.udpMulticastToJoin.push_back( multicastAddress );
            ++n;
        }

        ++udpPort;
        ++idListIttr;
    }

    m_globalConfig.Copy( globalConfig );

    if ( m_httpServer.SetPort( appConfig->GetAttributeValueOrChildValueByName( "restApiPort" ).AsUInt16( 10000, true ) ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: http server port has been set to " + CORE::ToString( m_httpServer.GetPort() ) );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: Failed to set http server port. Current value: " + CORE::ToString( m_httpServer.GetPort() ) );
    }

    m_httpRouter.SetResourceMapping( "/info", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisInfoResource( this ) )  );
    m_httpRouter.SetResourceMapping( "/config/appargs", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisConfigResource( this, true ) ) );
    m_httpRouter.SetResourceMapping( "/config", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisConfigResource( this, false ) )  );
    m_httpRouter.SetResourceMapping( appConfig->GetAttributeValueOrChildValueByName( "RestBasicHealthUri" ).AsString( "/health/basic", true ), RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() ) );

    if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisCluster::LoadConfig: Failed to set up task management API" );
    }

    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );

    m_testPacketTransmitTimer.SetInterval( appConfig->GetAttributeValueOrChildValueByName( "testPacketTransmissionIntervalInMs" ).AsUInt32( m_testPacketTransmitTimer.GetInterval(), true ) );
    m_transmitTestPackets = appConfig->GetAttributeValueOrChildValueByName( "transmitTestPackets" ).AsBool( m_transmitTestPackets, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisCluster::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::Int32 channelId = m_redisStreamStartChannelID;
    Udp2RedisClusterChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const Udp2RedisClusterChannel::ChannelMetrics& metrics = (*i).second->GetMetrics();
        CORE::CString metricPrefix = "udp2redis.ch" + CORE::Int32ToString( channelId ) + ".";

        GUCEF_METRIC_TIMING( metricPrefix + "redisErrorReplies", metrics.redisErrorReplies, 1.0f );
        GUCEF_METRIC_TIMING( metricPrefix + "redisMessagesTransmitted", metrics.redisMessagesTransmitted, 1.0f );
        GUCEF_METRIC_TIMING( metricPrefix + "redisPacketsInMessagesTransmitted", metrics.redisPacketsInMsgsTransmitted, 1.0f );
        GUCEF_METRIC_GAUGE( metricPrefix + "redisPacketsInMessagesRatio", metrics.redisPacketsInMsgsRatio, 1.0f );
        GUCEF_METRIC_GAUGE( metricPrefix + "redisTransmitQueueSize", metrics.redisTransmitQueueSize, 1.0f );
        GUCEF_METRIC_TIMING( metricPrefix + "udpBytesReceived", metrics.udpBytesReceived, 1.0f );
        GUCEF_METRIC_TIMING( metricPrefix + "udpPacketsReceived", metrics.udpPacketsReceived, 1.0f );
        ++i; ++channelId;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisCluster::OnTransmitTestPacketTimerCycle( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_testUdpSocket.IsActive() )
        if ( !m_testUdpSocket.Open() )
            return;

    Udp2RedisClusterChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const ChannelSettings& settings = (*i).second->GetChannelSettings();
        if ( settings.wantsTestPackage )
        {
            m_testUdpSocket.SendPacketTo( settings.udpInterface.GetFirstIPv4Address(), "TEST", 4 );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
Udp2RedisCluster::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "Udp2RedisCluster";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
Udp2RedisCluster::GetAppConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    const CORE::CDataNode* appConfig = globalConfig.Search( "Main/AppArgs", '/', true, true );
    return appConfig;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode*
Udp2RedisCluster::GetAppConfig( CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    CORE::CDataNode* appConfig = globalConfig.Structure( "Main/AppArgs", '/' );
    return appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
Udp2RedisCluster::GetAppConfig( void ) const
{GUCEF_TRACE;

    return GetAppConfig( m_globalConfig );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
Udp2RedisCluster::GetGlobalConfig( void ) const
{GUCEF_TRACE;

    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/
