
/*
 *  redisinfo: Service for obtaining redis info from a cluster
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

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

#include "redisinfo.h"

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#include "crc16.h"

#include "redisinfo_CRedisClusterKeyPrunerTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace REDISINFO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000
#define GUCEF_DEFAULT_REDIS_STREAM_INDEXING_INTERVAL                (1000 * 60 * 10) // 10 mins

const CORE::CString RedisInfoService::HashSlotFileCodec = "json";
static TRedisClusterKeyPrunerTaskDataFactory g_redisClusterKeyPrunerTaskDataFactory;
static TRedisClusterKeyPrunerTaskConsumerFactory g_redisClusterKeyPrunerTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Settings::Settings( void )
    : clusterName()
    , redisAddress()
    , collectMetrics( true )
    , metricPrefix()
    , gatherInfoReplication( true )
    , gatherInfoPersistence( true )
    , gatherInfoStats( true )
    , gatherInfoCommandStats( true )
    , gatherInfoMemory( true )
    , gatherStreamInfo( true )
    , streamsToGatherInfoFrom()
    , streamIndexingInterval( GUCEF_DEFAULT_REDIS_STREAM_INDEXING_INTERVAL )
    , redisScanCountSize( 5000 )
    , persistRedisKeyCacheSnapshot( false )
    , redisKeyCacheSnapshotPath( "InstallPath/redis/clusters/{clusterName}/RedisKeyCacheSnapshot.v1.json" )
    , redisKeyCacheSnapshotCodec( "json" )
    , gatherInfoClients( true )
    , gatherInfoCpu( true )
    , gatherInfoKeyspace( true )
    , gatherClusterInfo( true )
    , gatherErrorReplyCount( true )
    , redisConnectionOptionKeepAlive( true )
    , redisConnectionOptionConnectTimeoutInMs( 1000 )
    , redisConnectionOptionSocketTimeoutInMs( 1000 )
    , retainRedisInfo( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Settings::Settings( const Settings& src )
    : clusterName( src.clusterName )
    , redisAddress( src.redisAddress )
    , collectMetrics( src.collectMetrics )
    , metricPrefix()
    , gatherInfoReplication( src.gatherInfoReplication )
    , gatherInfoPersistence( src.gatherInfoPersistence )
    , gatherInfoStats( src.gatherInfoStats )
    , gatherInfoCommandStats( src.gatherInfoCommandStats )
    , gatherInfoMemory( src.gatherInfoMemory )
    , gatherStreamInfo( src.gatherStreamInfo )
    , streamsToGatherInfoFrom( src.streamsToGatherInfoFrom )
    , streamIndexingInterval( src.streamIndexingInterval )
    , redisScanCountSize( src.redisScanCountSize )
    , persistRedisKeyCacheSnapshot( src.persistRedisKeyCacheSnapshot )
    , redisKeyCacheSnapshotPath( src.redisKeyCacheSnapshotPath )
    , redisKeyCacheSnapshotCodec( src.redisKeyCacheSnapshotCodec )
    , gatherInfoClients( src.gatherInfoClients )
    , gatherInfoCpu( src.gatherInfoCpu )
    , gatherInfoKeyspace( src.gatherInfoKeyspace )
    , gatherClusterInfo( src.gatherClusterInfo )
    , gatherErrorReplyCount( src.gatherErrorReplyCount )
    , redisConnectionOptionKeepAlive( src.redisConnectionOptionKeepAlive )
    , redisConnectionOptionConnectTimeoutInMs( src.redisConnectionOptionConnectTimeoutInMs )
    , redisConnectionOptionSocketTimeoutInMs( src.redisConnectionOptionSocketTimeoutInMs )
    , retainRedisInfo( src.retainRedisInfo )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Settings&
Settings::operator=( const Settings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        clusterName = src.clusterName;
        redisAddress = src.redisAddress;
        collectMetrics = src.collectMetrics;
        metricPrefix = src.metricPrefix;
        gatherInfoReplication = src.gatherInfoReplication;
        gatherInfoPersistence = src.gatherInfoPersistence;
        gatherInfoStats = src.gatherInfoStats;
        gatherInfoCommandStats = src.gatherInfoCommandStats;
        gatherInfoMemory = src.gatherInfoMemory;
        gatherStreamInfo = src.gatherStreamInfo;
        streamsToGatherInfoFrom = src.streamsToGatherInfoFrom;
        streamIndexingInterval = src.streamIndexingInterval;
        redisScanCountSize = src.redisScanCountSize;
        persistRedisKeyCacheSnapshot = src.persistRedisKeyCacheSnapshot;
        redisKeyCacheSnapshotPath = src.redisKeyCacheSnapshotPath;
        redisKeyCacheSnapshotCodec = src.redisKeyCacheSnapshotCodec;
        gatherInfoClients = src.gatherInfoClients;
        gatherInfoCpu = src.gatherInfoCpu;
        gatherInfoKeyspace = src.gatherInfoKeyspace;
        gatherClusterInfo = src.gatherClusterInfo;
        gatherErrorReplyCount = src.gatherErrorReplyCount;
        redisConnectionOptionKeepAlive = src.redisConnectionOptionKeepAlive;
        redisConnectionOptionConnectTimeoutInMs = src.redisConnectionOptionConnectTimeoutInMs;
        redisConnectionOptionSocketTimeoutInMs = src.redisConnectionOptionSocketTimeoutInMs;
        retainRedisInfo = src.retainRedisInfo;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
Settings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "clusterName", clusterName );
    cfg.SetAttribute( "redisAddress", redisAddress.HostnameAndPortAsString() );
    cfg.SetAttribute( "collectMetrics", collectMetrics );
    cfg.SetAttribute( "metricPrefix", metricPrefix );
    cfg.SetAttribute( "gatherInfoReplication", gatherInfoReplication );
    cfg.SetAttribute( "gatherInfoPersistence", gatherInfoPersistence );
    cfg.SetAttribute( "gatherInfoStats", gatherInfoStats );
    cfg.SetAttribute( "gatherInfoCommandStats", gatherInfoCommandStats );
    cfg.SetAttribute( "gatherInfoMemory", gatherInfoMemory );
    cfg.SetAttribute( "gatherStreamInfo", gatherStreamInfo );
    cfg.SetAttribute( "streamsToGatherInfoFrom", CORE::ToString( streamsToGatherInfoFrom ) );
    cfg.SetAttribute( "streamIndexingInterval", streamIndexingInterval );
    cfg.SetAttribute( "redisScanCountSize", redisScanCountSize );
    cfg.SetAttribute( "persistRedisKeyCacheSnapshot", persistRedisKeyCacheSnapshot );
    cfg.SetAttribute( "redisKeyCacheSnapshotPath", redisKeyCacheSnapshotPath );
    cfg.SetAttribute( "redisKeyCacheSnapshotCodec", redisKeyCacheSnapshotCodec );
    cfg.SetAttribute( "gatherInfoClients", gatherInfoClients );
    cfg.SetAttribute( "gatherInfoCpu", gatherInfoCpu );
    cfg.SetAttribute( "gatherInfoKeyspace", gatherInfoKeyspace );
    cfg.SetAttribute( "gatherClusterInfo", gatherClusterInfo );
    cfg.SetAttribute( "gatherErrorReplyCount", gatherErrorReplyCount );
    cfg.SetAttribute( "redisConnectionOptionKeepAlive", redisConnectionOptionKeepAlive );
    cfg.SetAttribute( "redisConnectionOptionConnectTimeoutInMs", redisConnectionOptionConnectTimeoutInMs );
    cfg.SetAttribute( "redisConnectionOptionSocketTimeoutInMs", redisConnectionOptionSocketTimeoutInMs );
    cfg.SetAttribute( "retainRedisInfo", retainRedisInfo );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Settings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    clusterName = cfg.GetAttributeValueOrChildValueByName( "clusterName" ).AsString( clusterName, true );
    redisAddress.SetHostnameAndPort( cfg.GetAttributeValueOrChildValueByName( "redisAddress" ).AsString( redisAddress.HostnameAndPortAsString(), true ) );
    collectMetrics = cfg.GetAttributeValueOrChildValueByName( "collectMetrics" ).AsBool( collectMetrics, true );
    metricPrefix = cfg.GetAttributeValueOrChildValueByName( "metricPrefix" ).AsString( metricPrefix, true );
    gatherInfoReplication = cfg.GetAttributeValueOrChildValueByName( "gatherInfoReplication" ).AsBool( gatherInfoReplication, true );
    gatherInfoPersistence = cfg.GetAttributeValueOrChildValueByName( "gatherInfoPersistence" ).AsBool( gatherInfoPersistence, true );
    gatherInfoStats = cfg.GetAttributeValueOrChildValueByName( "gatherInfoStats" ).AsBool( gatherInfoStats, true );
    gatherInfoCommandStats = cfg.GetAttributeValueOrChildValueByName( "gatherInfoCommandStats" ).AsBool( gatherInfoCommandStats, true );
    gatherInfoMemory = cfg.GetAttributeValueOrChildValueByName( "gatherInfoMemory" ).AsBool( gatherInfoMemory, true );
    gatherStreamInfo = cfg.GetAttributeValueOrChildValueByName( "gatherStreamInfo" ).AsBool( gatherStreamInfo, true );
    streamsToGatherInfoFrom = CORE::StringToStringSet( cfg.GetAttributeValueOrChildValueByName( "streamsToGatherInfoFrom" ).AsString( CORE::StringSetToString( streamsToGatherInfoFrom ), true ) );
    streamIndexingInterval = cfg.GetAttributeValueOrChildValueByName( "streamIndexingInterval" ).AsInt32( streamIndexingInterval, true );
    redisScanCountSize = cfg.GetAttributeValueOrChildValueByName( "redisScanCountSize" ).AsUInt32( redisScanCountSize, true );
    persistRedisKeyCacheSnapshot = cfg.GetAttributeValueOrChildValueByName( "persistRedisKeyCacheSnapshot" ).AsBool( persistRedisKeyCacheSnapshot, true );
    redisKeyCacheSnapshotPath = cfg.GetAttributeValueOrChildValueByName( "redisKeyCacheSnapshotPath" ).AsString( redisKeyCacheSnapshotPath, true );
    redisKeyCacheSnapshotCodec = cfg.GetAttributeValueOrChildValueByName( "redisKeyCacheSnapshotCodec" ).AsString( redisKeyCacheSnapshotCodec, true );
    gatherInfoClients = cfg.GetAttributeValueOrChildValueByName( "gatherInfoClients" ).AsBool( gatherInfoClients, true );
    gatherInfoCpu = cfg.GetAttributeValueOrChildValueByName( "gatherInfoCpu" ).AsBool( gatherInfoCpu, true );
    gatherInfoKeyspace = cfg.GetAttributeValueOrChildValueByName( "gatherInfoKeyspace" ).AsBool( gatherInfoKeyspace, true );
    gatherClusterInfo = cfg.GetAttributeValueOrChildValueByName( "gatherClusterInfo" ).AsBool( gatherClusterInfo, true );
    gatherErrorReplyCount = cfg.GetAttributeValueOrChildValueByName( "gatherErrorReplyCount" ).AsBool( gatherErrorReplyCount, true );
    redisConnectionOptionKeepAlive = cfg.GetAttributeValueOrChildValueByName( "redisConnectionOptionKeepAlive" ).AsBool( redisConnectionOptionKeepAlive, true );
    redisConnectionOptionConnectTimeoutInMs = cfg.GetAttributeValueOrChildValueByName( "redisConnectionOptionConnectTimeoutInMs" ).AsUInt64( redisConnectionOptionConnectTimeoutInMs, true );
    redisConnectionOptionSocketTimeoutInMs = cfg.GetAttributeValueOrChildValueByName( "redisConnectionOptionSocketTimeoutInMs" ).AsUInt64( redisConnectionOptionSocketTimeoutInMs, true );
    retainRedisInfo = cfg.GetAttributeValueOrChildValueByName( "retainRedisInfo" ).AsBool( retainRedisInfo, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
Settings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "redisinfo::Settings";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

RedisInfoService::RedisInfoService()
    : CORE::CTaskConsumer()
    , m_redisContext()
    , m_redisPacketArgs()
    , m_metricsTimer( GUCEF_NULL )
    , m_redisReconnectTimer( GUCEF_NULL )
    , m_filteredStreamNames()
    , m_redisNodesMap()
    , m_hashSlotOriginStrMap()
    , m_redisClusterErrorReplies( 0 )
    , m_redisClusterTimeoutReplies( 0 )
    , m_cmdClusterInfo()
    , m_cmdInfoCommandstats()
    , m_cmdInfoMemory()
    , m_cmdInfoReplication()
    , m_cmdInfoPersistence()
    , m_cmdInfoStats()
    , m_cmdInfoClients()
    , m_cmdInfoCpu()
    , m_cmdInfoKeyspace()
    , m_cmdXinfoStreamMap()
    , m_status()
    , m_redisInfoValueTypes()
    , m_vfsIsInitialized( false )
    , m_lock( true )
{GUCEF_TRACE;

    m_status[ "connected" ] = false;
    m_status[ "reconnecting" ] = false;
    m_status[ "redisClusterErrorReplies" ] = 0;
    m_status[ "redisClusterTimeoutReplies" ] = 0;

    PopulateDefaultRedisInfoValueTypes();
}

/*-------------------------------------------------------------------------*/

RedisInfoService::~RedisInfoService()
{GUCEF_TRACE;

    RedisDisconnect();

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback1( this, &RedisInfoService::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );

    TEventCallback callback2( this, &RedisInfoService::OnRedisReconnectTimerCycle );
    SubscribeTo( m_redisReconnectTimer          ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );

    TEventCallback callback4( this, &RedisInfoService::OnVfsInitCompleted );
    SubscribeTo( &VFS::CVfsGlobal::Instance()->GetVfs()     ,
                 VFS::CVFS::VfsInitializationCompletedEvent ,
                 callback4                                  );
    m_vfsIsInitialized = VFS::CVfsGlobal::Instance()->GetVfs().IsInitialized();

    TEventCallback callback5( this, &RedisInfoService::OnRedisKeyCacheUpdate );
    SubscribeTo( CRedisClusterKeyCache::Instance()       ,
                 CRedisClusterKeyCache::CacheUpdateEvent ,
                 callback5                               );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::LoadConfig( const Settings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    return true;
}

/*-------------------------------------------------------------------------*/

const Settings&
RedisInfoService::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::ConnectHttpRouting( WEB::CIHTTPServerRouter& router )
{GUCEF_TRACE;

    if ( !m_settings.retainRedisInfo )
        return true;

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdClusterInfoRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdClusterInfo, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/cluster/info", cmdClusterInfoRsc );
    m_httpResources.push_back( cmdClusterInfoRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoCommandStatsRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoCommandstats, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/commandstats", cmdInfoCommandStatsRsc );
    m_httpResources.push_back( cmdInfoCommandStatsRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoMemoryRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoMemory, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/memory", cmdInfoMemoryRsc );
    m_httpResources.push_back( cmdInfoMemoryRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoReplicationRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoReplication, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/replication", cmdInfoReplicationRsc );
    m_httpResources.push_back( cmdInfoReplicationRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoPersistenceRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoPersistence, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/persistence", cmdInfoPersistenceRsc );
    m_httpResources.push_back( cmdInfoPersistenceRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoStatsRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoStats, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/stats", cmdInfoStatsRsc );
    m_httpResources.push_back( cmdInfoStatsRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoClientsRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoClients, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/clients", cmdInfoClientsRsc );
    m_httpResources.push_back( cmdInfoClientsRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoCpuRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoCpu, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/cpu", cmdInfoCpuRsc );
    m_httpResources.push_back( cmdInfoCpuRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdInfoKeyspaceRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_cmdInfoKeyspace, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/info/keyspace", cmdInfoKeyspaceRsc );
    m_httpResources.push_back( cmdInfoKeyspaceRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr cmdXinfoStreamIndexRsc( ( GUCEF_NEW TStringToValueListMapHttpResource( "Streams", "streamName", GUCEF_NULL, &m_cmdXinfoStreamMap, &m_lock, false ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/api/command/xinfo/stream", cmdXinfoStreamIndexRsc );
    m_httpResources.push_back( cmdXinfoStreamIndexRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr statusRsc( ( GUCEF_NEW GUCEF::WEB::CConfigurableHttpServerResource( &m_status, true, false, &m_lock ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/status", statusRsc );
    m_httpResources.push_back( statusRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr redisClusterNodesIndexRsc( ( GUCEF_NEW TUInt32ToRedisNodeMapHttpResource( "RedisClusterNodes", "startSlot", GUCEF_NULL, &m_redisNodesMap, &m_lock, false ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/nodes", redisClusterNodesIndexRsc );
    m_httpResources.push_back( redisClusterNodesIndexRsc );

    GUCEF::WEB::CIHTTPServerRouter::THTTPServerResourcePtr hashSlotHashStringIndexRsc( ( GUCEF_NEW TUInt32ToStringSetMapHttpResource( "HashSlotHashStrings", "hashSlot", GUCEF_NULL, &m_hashSlotOriginStrMap, &m_lock, false ) )->CreateSharedPtr() );
    router.SetResourceMapping( "/v1/clusters/" + m_settings.clusterName + "/redis/hashslothashstrings", hashSlotHashStringIndexRsc );
    m_httpResources.push_back( hashSlotHashStringIndexRsc );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::DisconnectHttpRouting( WEB::CIHTTPServerRouter& router )
{GUCEF_TRACE;

    bool totalSuccess = true;
    THttpResourceVector::iterator i = m_httpResources.begin();
    while ( i != m_httpResources.end() )
    {
        totalSuccess = router.RemoveResourceMapping( (*i) ) && totalSuccess;
        ++i;
    }
    m_httpResources.clear();
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
RedisInfoService::CalculateRedisHashSlot( const CORE::CString& keyStr ) const
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

bool
RedisInfoService::CalculateKeysForAllHashSlots( TUInt32ToStringSetMap& hashMap ,
                                                CORE::UInt32 minKeysPerSlot    ,
                                                CORE::UInt32 maxKeysPerSlot    ) const
{GUCEF_TRACE;

    // Since we need to generate a key per slot a minimum lower than 1 makes no sense
    if ( minKeysPerSlot < 1 )
        minKeysPerSlot = 1;

    CORE::UInt32 lowestKeysInSlot = 0;

    CORE::UInt64 i=0;
    size_t lastSize=0;
    while ( hashMap.size() < 16383 || lowestKeysInSlot < minKeysPerSlot )
    {
        CORE::CString hashOriginString = CORE::ToString( i );
        CORE::UInt32 hashSlot = CalculateRedisHashSlot( hashOriginString );

        TStringSet& hashOriginStringSet = hashMap[ hashSlot ];
        if ( hashOriginStringSet.size() < maxKeysPerSlot || 0 == maxKeysPerSlot )
        {
            hashOriginStringSet.insert( hashOriginString );

            if ( hashMap.size() >= 16383 )
            {
                // Check for lowest keys per slow threshold
                lowestKeysInSlot = minKeysPerSlot;
                TUInt32ToStringSetMap::iterator n = hashMap.begin();
                while ( n != hashMap.end() )
                {
                    if ( (*n).second.size() < lowestKeysInSlot )
                        lowestKeysInSlot = (CORE::UInt32)(*n).second.size();
                    ++n;
                }
            }
        }

        if ( hashMap.size() > lastSize )
        {
            CORE::Float32 percDone = hashMap.size() / 163.83f;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService:CalculateKeysForAllHashSlots: Now " + CORE::ToString( percDone ) + "% done generating hashes for all slots" );
            lastSize = hashMap.size();
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::SerializeKeysForHashSlots( const TUInt32ToStringSetMap& hashMap, CORE::CDataNode& doc ) const
{GUCEF_TRACE;

    doc.Clear();
    doc.SetName( "RedisHashMap" );
    doc.SetNodeType( GUCEF_DATATYPE_ARRAY );
    TUInt32ToStringSetMap::const_iterator i = hashMap.begin();
    while ( i != hashMap.end() )
    {
        CORE::CDataNode* slotEntry = doc.AddChild( "slot", GUCEF_DATATYPE_OBJECT );
        slotEntry->SetAttribute( "id", CORE::ToString( (*i).first ), GUCEF_DATATYPE_UINT32 );

        CORE::CDataNode* originStringArray = slotEntry->AddChild( "hashOriginStrings", GUCEF_DATATYPE_ARRAY );
        const TStringSet& hashOriginStrings = (*i).second;
        TStringSet::const_iterator n = hashOriginStrings.begin();
        while ( n != hashOriginStrings.end() )
        {
            originStringArray->AddChild( (*n), GUCEF_DATATYPE_STRING );
            ++n;
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::DeserializeKeysForHashSlots( TUInt32ToStringSetMap& hashMap, const CORE::CDataNode& doc ) const
{GUCEF_TRACE;

    CORE::CDataNode::const_iterator i = doc.ConstBegin();
    while ( i != doc.ConstEnd() )
    {
        CORE::CString hashSlot = (*i)->GetAttributeValueOrChildValueByName( "id" );
        CORE::CDataNode* hashOriginStringsNode = (*i)->FindChild( "hashOriginStrings" );
        if ( GUCEF_NULL != hashOriginStringsNode )
        {
            CORE::CDataNode::TVariantVector hashOriginStrings = hashOriginStringsNode->GetChildrenValues();

            if ( !hashSlot.IsNULLOrEmpty() && !hashOriginStrings.empty() )
            {
                TStringSet& hashEntries = hashMap[ CORE::StringToUInt16( hashSlot ) ];
                CORE::CDataNode::TVariantVector::iterator n = hashOriginStrings.begin();
                while ( n != hashOriginStrings.end() )
                {
                    hashEntries.insert( (*n) );
                    ++n;
                }
            }
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::SaveDocTo( const CORE::CDataNode& doc     ,
                             const CORE::CString& codecName ,
                             const CORE::CString& vfsPath   ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.IsInitialized() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):SaveDocTo: Deferring until the VFS is initialized" );
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false );
    if ( !codec )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):SaveDocTo: Could not obtain codec for codecName : " + codecName );
        return false;
    }

    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "wb", true );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):SaveDocTo: VFS could not provide access to file at path: " + vfsPath );
        return false;
    }

    return codec->StoreDataTree( &doc, file->GetAccess().GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::LoadDocFrom( CORE::CDataNode& doc           ,
                               const CORE::CString& codecName ,
                               const CORE::CString& vfsPath   ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.IsInitialized() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):LoadDocFrom: Deferring until the VFS is initialized" );
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false );
    if ( !codec )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):LoadDocFrom: Could not obtain codec for codecName : " + codecName );
        return false;
    }

    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "rb", false );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):LoadDocFrom: VFS could not provide access to file at path: " + vfsPath );
        return false;
    }

    return codec->BuildDataTree( &doc, file->GetAccess().GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::LoadDocFrom( CORE::CDataNode& doc         ,
                               const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString fileType = CORE::ExtractFileExtention( vfsPath );
    return LoadDocFrom( doc, fileType, vfsPath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
RedisInfoService::GetType( void ) const
{GUCEF_TRACE;

    return "RedisInfoService";
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::ProvideHashSlotMapDoc( void )
{GUCEF_TRACE;

    const CORE::CString hashSlotFile = "InstallPath/redis/clusters/" + m_settings.clusterName + "/RedisHashMap.v1.json";

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( hashSlotFile ) )
    {
        if ( CalculateKeysForAllHashSlots( m_hashSlotOriginStrMap, 1, 1 ) )
        {
            CORE::CDataNode hashSlotsDoc;
            if ( SerializeKeysForHashSlots( m_hashSlotOriginStrMap, hashSlotsDoc ) )
            {
                return SaveDocTo( hashSlotsDoc, HashSlotFileCodec, hashSlotFile );
            }
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::LoadHashSlotMap( void )
{GUCEF_TRACE;

    const CORE::CString hashSlotFile = "InstallPath/redis/clusters/" + m_settings.clusterName + "/RedisHashMap.v1.json";
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( hashSlotFile ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: hash slot map did not exist, attempting to generate a new one" );
        if ( !ProvideHashSlotMapDoc() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: hash slot map did not exist and failed to provide a new one" );
            return false;
        }
        return true;
    }
    else
    {
        CORE::CDataNode hashSlotsDoc;
        if ( LoadDocFrom( hashSlotsDoc, HashSlotFileCodec, hashSlotFile ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: Loaded hash slot map document, deserializing..." );
            if ( DeserializeKeysForHashSlots( m_hashSlotOriginStrMap, hashSlotsDoc ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: Successfully loaded hash slot map" );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: Failed to parse hash slot map document" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):LoadHashSlotMap: Failed to load hash slot map document" );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::RefreshRedisNodePipes( void )
{GUCEF_TRACE;

    RedisNodeMap redisNodes;
    if ( GetRedisClusterSlots( redisNodes ) )
    {
        // Due to the way the Redis library API works we need the hash slot mapping to be able to create
        // a pipeline to each node since there is no API call to get a connection to a specific node
        // It is abstracted away from us. The hash map allows us to work around that.
        if ( m_hashSlotOriginStrMap.empty() )
        {
            if ( !LoadHashSlotMap() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):RefreshRedisNodePipes: Unable to lazy load the hash map, as such cannot generate strings to establish dedicated pipes to the nodes" );
                return false;
            }
        }

        RedisNodeMap::iterator i = redisNodes.begin();
        while ( i != redisNodes.end() )
        {
            RedisNodeWithPipe& node = m_redisNodesMap[ (*i).first ];
            node = (*i).second;

            TStringSet hashStrings = m_hashSlotOriginStrMap[ node.startSlot ];
            if ( !hashStrings.empty() )
            {
                try
                {
                    CORE::CString hashString = *hashStrings.begin();
                    sw::redis::StringView hashStringSV( hashString.C_String(), hashString.Length() );

                    delete node.redisPipe;
                    node.redisPipe = GUCEF_NULL;
                    node.redisPipe = new sw::redis::Pipeline( m_redisContext->pipeline( hashStringSV ) );
                }
                catch ( const sw::redis::Error& e )
                {
                    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RefreshRedisNodePipes: Redis++ exception: " + e.what() );
                    ++m_redisClusterErrorReplies;
                    RedisReconnect();
                    return false;
                }
                catch ( const std::exception& e )
                {
                    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RefreshRedisNodePipes: exception: " + e.what() );
                    RedisReconnect();
                    return false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):RefreshRedisNodePipes: Unable to find a hash slot entry for node " +
                    node.nodeId + " with start slot " + CORE::ToString( node.startSlot ) + ", as such cannot generate string to establish a dedicated pipe to the node" );
            }
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::ProvideRedisNodesDoc( void )
{GUCEF_TRACE;

    static const CORE::CString redisNodesCodec = "json";
    const CORE::CString redisNodesFile = "InstallPath/redis/clusters/" + m_settings.clusterName + "/RedisNodes.v1.json";

    RedisNodeMap redisNodes;
    if ( GetRedisClusterSlots( redisNodes ) )
    {
        CORE::CDataNode clusterNodesDoc;
        if ( SerializeRedisClusterNodeMap( redisNodes, clusterNodesDoc ) )
        {
            return SaveDocTo( clusterNodesDoc, redisNodesCodec, redisNodesFile );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    delete m_redisReconnectTimer;
    m_redisReconnectTimer = new CORE::CTimer( GetPulseGenerator(), 100 );

    delete m_metricsTimer;
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );
    m_metricsTimer->SetEnabled( m_settings.collectMetrics );

    if ( IsStreamIndexingNeeded() )
    {
        CRedisClusterKeyCache* cache = CRedisClusterKeyCache::Instance();

        if ( m_settings.streamIndexingInterval > 0 )
            cache->SetIndexingIntervalInMs( (CORE::UInt32) m_settings.streamIndexingInterval );
        cache->SetRedisScanInterationCountSize( m_settings.redisScanCountSize );
        cache->SetPersistKeyCacheSnapshot( m_settings.persistRedisKeyCacheSnapshot );
        cache->SetPersistKeyCacheSnapshotPath( m_settings.redisKeyCacheSnapshotPath );
        cache->SetPersistKeyCacheSnapshotCodec( m_settings.redisKeyCacheSnapshotCodec );

    }

    RegisterEventHandlers();

    //LoadHashSlotMap();

    // Setup connection to Redis
    // Note that if there is an error here we will just keep on trying automatically
    // same thing for VFS init which will defer the connection attempt
    if ( m_vfsIsInitialized )
        RedisConnect( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
    {
        m_redisReconnectTimer->SetEnabled( true );
    }

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnTaskEnding( CORE::CTaskPtr task ,
                                bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnTaskEnded( CORE::CTaskPtr task ,
                               bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    GUCEF_DELETE m_redisReconnectTimer;
    m_redisReconnectTimer = GUCEF_NULL;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString RedisNode::ClassTypeName = "GUCEF::REDISINFO::RedisNode";

/*-------------------------------------------------------------------------*/

RedisNode::RedisNode( void )
    : CORE::CIDataNodeSerializable()
    , host()
    , nodeId()
    , startSlot( 0 )
    , endSlot( 0 )
    , replicas()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

RedisNode::RedisNode( const RedisNode& src )
    : CORE::CIDataNodeSerializable( src )
    , host( src.host )
    , nodeId( src.nodeId )
    , startSlot( src.startSlot )
    , endSlot( src.endSlot )
    , replicas( src.replicas )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

RedisNode&
RedisNode::operator=( const RedisNode& other )
{GUCEF_TRACE;

    if ( this != &other )
    {
        host = other.host;
        nodeId = other.nodeId;
        startSlot = other.startSlot;
        endSlot = other.endSlot;
        replicas = other.replicas;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
RedisNode::Clone( void ) const
{GUCEF_TRACE;

    return new RedisNode( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
RedisNode::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
RedisNode::Serialize( CORE::CDataNode& domRootNode                        ,
                      const CORE::CDataNodeSerializableSettings& settings ,
                      CORE::CDataNode** targetNode                        ) const
{GUCEF_TRACE;

    CORE::CDataNode* redisNodeEntry = domRootNode.AddChild( "node", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != redisNodeEntry )
    {
        if ( GUCEF_NULL != targetNode )
            *targetNode = redisNodeEntry;

        bool totalSuccess = true;
        totalSuccess = redisNodeEntry->SetAttribute( "id", nodeId ) && totalSuccess;
        totalSuccess = redisNodeEntry->SetAttribute( "startSlot", startSlot ) && totalSuccess;
        totalSuccess = redisNodeEntry->SetAttribute( "endSlot", endSlot ) && totalSuccess;
        totalSuccess = redisNodeEntry->SetAttribute( "host", host.GetHostname() ) && totalSuccess;
        totalSuccess = redisNodeEntry->SetAttribute( "port", host.GetPortInHostByteOrder() ) && totalSuccess;
        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
RedisNode::Serialize( CORE::CDataNode& domRootNode                        ,
                      const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return Serialize( domRootNode, settings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
RedisNode::Deserialize( const CORE::CDataNode& domRootNode                  ,
                        const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* redisNodeEntry = domRootNode.Find( "node" );
    if ( GUCEF_NULL != redisNodeEntry )
    {
        nodeId = redisNodeEntry->GetAttributeValueOrChildValueByName( "id" ).AsString();
        startSlot = redisNodeEntry->GetAttributeValueOrChildValueByName( "startSlot" ).AsInt32();
        endSlot = redisNodeEntry->GetAttributeValueOrChildValueByName( "endSlot" ).AsInt32();
        CORE::CString hostname = redisNodeEntry->GetAttributeValueOrChildValueByName( "host" ).AsString();
        CORE::UInt16 port = redisNodeEntry->GetAttributeValueOrChildValueByName( "port" ).AsUInt16();
        if ( host.SetHostnameAndPort( hostname, port ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

RedisNode*
RedisNode::FindReplica( const CORE::CString& replicaNodeId ,
                        const CORE::CString& parentNodeId  ,
                        bool createIfNotFound              )
{GUCEF_TRACE;

    if ( nodeId == parentNodeId )
    {
        RedisNodeVector::iterator i = replicas.begin();
        while ( i != replicas.end() )
        {
            if ( (*i).nodeId == replicaNodeId )
                return &(*i);
            ++i;
        }
        if ( createIfNotFound )
        {
            RedisNode newReplica;
            newReplica.nodeId = replicaNodeId;
            newReplica.startSlot = startSlot;
            newReplica.endSlot = endSlot;
            replicas.push_back( newReplica );
            return &replicas.back();
        }
        return GUCEF_NULL;
    }
    else
    {
        RedisNodeVector::iterator i = replicas.begin();
        while ( i != replicas.end() )
        {
            RedisNode* replica = (*i).FindReplica( replicaNodeId, parentNodeId, createIfNotFound );
            if ( GUCEF_NULL != replica )
                return replica;
            ++i;
        }
        return GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

RedisNodeWithPipe::RedisNodeWithPipe( void )
    : RedisNode()
    , redisPipe( GUCEF_NULL )
    , redisErrorReplies( 0 )
    , redisTimeoutReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

RedisNodeWithPipe&
RedisNodeWithPipe::operator=( const RedisNode& other )
{GUCEF_TRACE;

    if ( this != &other )
    {
        RedisNode::operator=( other );
        //redisPipe
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
RedisNodeWithPipe::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    CORE::CDataNode* targetNode = GUCEF_NULL;
    if ( RedisNode::Serialize( domRootNode, settings, &targetNode ) && GUCEF_NULL != targetNode )
    {
        if ( CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MaximumDetails == settings.levelOfDetail )
        {
            targetNode->SetAttribute( "redisErrorReplies", redisErrorReplies );
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
RedisNodeWithPipe::GetRedisTimeoutRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 nodeRedisTimeoutReplies = redisTimeoutReplies;
        redisTimeoutReplies = 0;
        return nodeRedisTimeoutReplies;
    }
    else
        return redisTimeoutReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
RedisNodeWithPipe::GetRedisErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 nodeRedisErrorReplies = redisErrorReplies;
        redisErrorReplies = 0;
        return nodeRedisErrorReplies;
    }
    else
        return redisErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
RedisInfoService::GetRedisClusterErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisErrorReplies = m_redisClusterErrorReplies;
        m_redisClusterErrorReplies = 0;
        return redisErrorReplies;
    }
    else
        return m_redisClusterErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
RedisInfoService::GetRedisClusterTimeoutRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisClusterTimeoutReplies = m_redisClusterTimeoutReplies;
        m_redisClusterTimeoutReplies = 0;
        return redisClusterTimeoutReplies;
    }
    else
        return m_redisClusterTimeoutReplies;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoKeyspace( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "keyspace" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoPersistence( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "persistence" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoReplication( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "replication" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoStats( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "stats" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoCommandStats( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "commandstats" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoClients( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "clients" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoMemory( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "memory" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfoCpu( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "info" );
    static CORE::CString typeParam( "cpu" );

    bool totalSuccess = true;
    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        totalSuccess = GetRedisInfo( cmdParam, typeParam, kv, &(*i).second ) && totalSuccess;

        // Abort additional info retrievals if we lose the context
        if ( m_redisContext.IsNULL() || m_redisNodesMap.empty() )
            return totalSuccess;

        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisClusterInfo( CORE::CValueList& kv )
{GUCEF_TRACE;

    static CORE::CString cmdParam( "cluster" );
    static CORE::CString typeParam( "info" );

    return GetRedisInfo( cmdParam, typeParam, kv );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfo( struct redisReply* reply       ,
                                const CORE::CString& type      ,
                                CORE::CValueList& kv           ,
                                const CORE::CString& keyPrefix )
{GUCEF_TRACE;

    if ( GUCEF_NULL == reply )
        return false;

    if ( REDIS_REPLY_STRING == reply->type )
    {
        CORE::CString replyStr = reply->str;
        replyStr = replyStr.ReplaceSubstr( "\r\n", "\n" );

        CORE::CValueList kvCombined;
        kvCombined.SetMultiple( replyStr, '\n', ':' );

        CORE::CValueList::TValueMap::const_iterator i = kvCombined.GetDataBeginIterator();
        while ( i != kvCombined.GetDataEndIterator() )
        {
            if ( !(*i).second.empty() )
            {
                const CORE::CString& key = (*i).first;
                const CORE::CString& comboValue = (*i).second.front();
                CORE::CString::StringVector values = comboValue.ParseElements( ',', false );
                if ( values.size() == 1 )
                {
                    // Filter anything labeled as for human viewing since its redundant plus
                    // we are all about automation in this code
                    if ( -1 == key.HasSubstr( "_human", false ) && -1 == key.HasChar( '#', true ) )
                    {
                        CORE::CVariant varValue( GetTypeOfRedisInfoValue( key ), comboValue );
                        kv.Set( keyPrefix + key, varValue );
                    }
                }
                else
                if ( values.size() > 1 )
                {
                    CORE::CString extraKeyPrefix = keyPrefix + key + '_';
                    CORE::CString::StringVector::iterator n = values.begin();
                    while ( n != values.end() )
                    {
                        // Filter anything labeled as for human viewing since its redundant plus
                        // we are all about automation in this code
                        if ( -1 == (*n).HasSubstr( "_human", false ) && -1 == (*n).HasChar( '#', true ) )
                        {
                            const CORE::CString& keyAndValue = (*n);
                            CORE::CString key( keyAndValue.SubstrToChar( '=', true ) );
                            CORE::CVariant value( GetTypeOfRedisInfoValue( key ), keyAndValue.SubstrToChar( '=', false ) );
                            kv.Set( extraKeyPrefix + key, value );
                        }
                        ++n;
                    }
                }
            }
            ++i;
        }
        return true;
    }

    if ( REDIS_REPLY_ERROR == reply->type )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::PopulateDefaultRedisInfoValueTypes( void )
{GUCEF_TRACE;

    // Various
    m_redisInfoValueTypes[ "calls" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "count" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "avg_ttl" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "keys" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "expires" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "usec" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "usec_per_call" ] = GUCEF_DATATYPE_UINT64;

    // # Server
    m_redisInfoValueTypes[ "redis_version" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "redis_git_dirty" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "redis_build_id" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "redis_mode" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "multiplexing_api" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "arch_bits" ] = GUCEF_DATATYPE_UINT8;
    m_redisInfoValueTypes[ "multiplexing_api" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "atomicvar_api" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "gcc_version" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "process_id" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "process_supervised" ] = GUCEF_DATATYPE_BOOLEAN_UTF8_STRING;
    m_redisInfoValueTypes[ "run_id" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "tcp_port" ] = GUCEF_DATATYPE_UINT16;
    m_redisInfoValueTypes[ "server_time_usec" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "uptime_in_seconds" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "uptime_in_days" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "hz" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "configured_hz" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "lru_clock" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "executable" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "config_file" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "io_threads_active" ] = GUCEF_DATATYPE_UINT64;

    // # Clients
    m_redisInfoValueTypes[ "connected_clients" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_connections" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "maxclients" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "client_recent_max_input_buffer" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "client_recent_max_output_buffer" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "blocked_clients" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "tracking_clients" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "clients_in_timeout_table" ] = GUCEF_DATATYPE_UINT64;

    // # Memory
    m_redisInfoValueTypes[ "used_memory" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "used_memory_rss" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_peak" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_peak_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "used_memory_peak_perc" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_memory_overhead" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_startup" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_dataset" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_dataset_perc" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "allocator_allocated" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "allocator_active" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "allocator_resident" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_system_memory" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_system_memory_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "used_memory_lua" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_vm_eval" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_lua_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "used_memory_scripts_eval" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "number_of_cached_scripts" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "number_of_functions" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "number_of_libraries" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_vm_functions" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_vm_total" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_vm_total_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "used_memory_functions" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_scripts" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "used_memory_scripts_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "maxmemory" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "maxmemory_human" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "maxmemory_policy" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "allocator_frag_ratio" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "allocator_frag_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "allocator_rss_ratio" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "allocator_rss_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rss_overhead_ratio" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "rss_overhead_bytes" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "mem_fragmentation_ratio" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "mem_fragmentation_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_not_counted_for_evict" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_replication_backlog" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_total_replication_buffers" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_clients_slaves" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_clients_normal" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_cluster_links" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_aof_buffer" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "mem_allocator" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "active_defrag_running" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "lazyfree_pending_objects" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "lazyfreed_objects" ] = GUCEF_DATATYPE_UINT64;

    // # Persistence
    m_redisInfoValueTypes[ "loading" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "async_loading" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "current_cow_size" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "current_cow_size_age" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "current_fork_perc" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "current_save_keys_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "current_save_keys_total" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_changes_since_last_save" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_bgsave_in_progress" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_last_save_time" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_last_bgsave_status" ] = GUCEF_DATATYPE_BOOLEAN_UTF8_STRING;
    m_redisInfoValueTypes[ "rdb_last_bgsave_time_sec" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "rdb_current_bgsave_time_sec" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "rdb_current_bgsave_time_sec" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "rdb_saves" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_last_cow_size" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_last_load_keys_expired" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "rdb_last_load_keys_loaded" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "aof_enabled" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "aof_rewrite_in_progress" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "aof_rewrite_scheduled" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "aof_last_rewrite_time_sec" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "aof_current_rewrite_time_sec" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "aof_last_write_status" ] = GUCEF_DATATYPE_BOOLEAN_UTF8_STRING;
    m_redisInfoValueTypes[ "aof_last_cow_size" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "module_fork_in_progress" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "module_fork_last_cow_size" ] = GUCEF_DATATYPE_UINT64;

    // # Stats
    m_redisInfoValueTypes[ "total_connections_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_commands_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "instantaneous_ops_per_sec" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_net_input_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_net_output_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "instantaneous_input_kbps" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "instantaneous_output_kbps" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "rejected_connections" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "sync_full" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "sync_partial_ok" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "sync_partial_err" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "expired_keys" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "expired_stale_perc" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "expired_time_cap_reached_count" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "expire_cycle_cpu_milliseconds" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "evicted_keys" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "evicted_clients" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_eviction_exceeded_time" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "current_eviction_exceeded_time" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "keyspace_hits" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "keyspace_misses" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "pubsub_channels" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "pubsub_patterns" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "latest_fork_usec" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_forks" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "migrate_cached_sockets" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "slave_expires_tracked_keys" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "active_defrag_hits" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "active_defrag_misses" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "active_defrag_key_hits" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "active_defrag_key_misses" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_active_defrag_time" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "current_active_defrag_time" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "tracking_total_keys" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "tracking_total_items" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "tracking_total_prefixes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "unexpected_error_replies" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_error_replies" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "dump_payload_sanitizations" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_reads_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_writes_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "io_threaded_reads_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "io_threaded_writes_processed" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "reply_buffer_shrinks" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "reply_buffer_expands" ] = GUCEF_DATATYPE_UINT64;

    // # Replication
    m_redisInfoValueTypes[ "role" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "connected_slaves" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "master_failover_state" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "master_replid" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "master_replid2" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "master_repl_offset" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "second_repl_offset" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "repl_backlog_active" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "repl_backlog_size" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "repl_backlog_first_byte_offset" ] = GUCEF_DATATYPE_UINT64;
    // If the instance is a replica, these additional fields are provided:
    m_redisInfoValueTypes[ "master_host" ] = GUCEF_DATATYPE_UTF8_STRING;
    m_redisInfoValueTypes[ "master_port" ] = GUCEF_DATATYPE_UINT16;
    m_redisInfoValueTypes[ "master_link_status" ] = GUCEF_DATATYPE_BOOLEAN_UTF8_STRING;
    m_redisInfoValueTypes[ "master_last_io_seconds_ago" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "master_sync_in_progress" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "slave_read_repl_offset" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "slave_repl_offset" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "slave_priority" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "slave_read_only" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "replica_announced" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    // If a SYNC operation is on-going, these additional fields are provided:
    m_redisInfoValueTypes[ "master_sync_total_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "master_sync_read_bytes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "master_sync_left_bytes" ] = GUCEF_DATATYPE_INT64;
    m_redisInfoValueTypes[ "repl_backlog_histlen" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "master_sync_perc" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "master_sync_last_io_seconds_ago" ] = GUCEF_DATATYPE_UINT64;
    // If the link between master and replica is down, an additional field is provided:
    m_redisInfoValueTypes[ "master_link_down_since_seconds" ] = GUCEF_DATATYPE_UINT64;
    // If the server is configured with the min-slaves-to-write (or starting with Redis 5 with the min-replicas-to-write) directive, an additional field is provided:
    m_redisInfoValueTypes[ "min_slaves_good_slaves" ] = GUCEF_DATATYPE_UINT64;

    // # CPU
    m_redisInfoValueTypes[ "used_cpu_sys" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_cpu_user" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_cpu_sys_children" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_cpu_user_children" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_cpu_sys_main_thread" ] = GUCEF_DATATYPE_FLOAT32;
    m_redisInfoValueTypes[ "used_cpu_user_main_thread" ] = GUCEF_DATATYPE_FLOAT32;

    // # Cluster
    m_redisInfoValueTypes[ "cluster_enabled" ] = GUCEF_DATATYPE_BOOLEAN_INT32;
    m_redisInfoValueTypes[ "cluster_slots_assigned" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_slots_ok" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_slots_pfail" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_slots_fail" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_known_nodes" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_size" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_current_epoch" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_my_epoch" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "total_cluster_links_buffer_limit_exceeded" ] = GUCEF_DATATYPE_UINT64;
    // the following are only sent by Redis is the value is non-zero per the docs:
    m_redisInfoValueTypes[ "cluster_stats_messages_ping_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_ping_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_pong_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_pong_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_meet_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_meet_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_fail_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_fail_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_publish_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_publish_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_auth-req_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_auth-req_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_auth-ack_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_auth-ack_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_update_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_update_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_mfstart_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_mfstart_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_module_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_module_received" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_publishshard_sent" ] = GUCEF_DATATYPE_UINT64;
    m_redisInfoValueTypes[ "cluster_stats_messages_publishshard_received" ] = GUCEF_DATATYPE_UINT64;

}

/*-------------------------------------------------------------------------*/

CORE::UInt8
RedisInfoService::GetTypeOfRedisInfoValue( const CORE::CString& valueName ) const
{GUCEF_TRACE;

    TStringToUInt8Map::const_iterator i = m_redisInfoValueTypes.find( valueName );
    if ( i != m_redisInfoValueTypes.end() )
    {
        // By default we get all values as strings so if we dont know we just keep it as a string
        if ( GUCEF_DATATYPE_UNKNOWN == (*i).second )
            return GUCEF_DATATYPE_UTF8_STRING;

        return (*i).second;
    }

    // By default we get all values as strings so if we dont know we just keep it as a string
    return GUCEF_DATATYPE_UTF8_STRING;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisInfo( const CORE::CString& cmd  ,
                                const CORE::CString& type ,
                                CORE::CValueList& kv      ,
                                RedisNodeWithPipe* node   )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
        return false;

    try
    {
        sw::redis::StringView infoCmdSV( cmd.C_String(), cmd.Length() );
        sw::redis::StringView typeParamSV( type.C_String(), type.Length() );

        if ( GUCEF_NULL != node && GUCEF_NULL != node->redisPipe )
        {
            node->redisPipe->command( infoCmdSV, typeParamSV );
            sw::redis::QueuedReplies redisReplies = node->redisPipe->exec();
            size_t replyCount = redisReplies.size();
            if ( replyCount > 0 )
            {
                redisReply& reply = redisReplies.get( 0 );
                return GetRedisInfo( &reply, type, kv, node->nodeId + '.' );
            }
        }
        else
        {
            auto reply = m_redisContext->command( infoCmdSV, typeParamSV );
            return GetRedisInfo( reply.get(), type, kv, CORE::CString::Empty );
        }
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        if ( GUCEF_NULL != node )
        {
            ++node->redisTimeoutReplies;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisInfo: Redis Node " + node->nodeId + " : Redis++ timeout exception: " + e.what() );
        }
        else
        {
            ++m_redisClusterTimeoutReplies;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisInfo: Redis++ TimeoutError exception: " + e.what() );
        }
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisInfo: Redis++ exception: " + e.what() );
        if ( GUCEF_NULL != node )
            ++node->redisErrorReplies;
        ++m_redisClusterErrorReplies;
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisInfo: exception: " + e.what() );
        RedisReconnect();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisStreamInfo( struct redisReply* replyNode           ,
                                      CORE::CValueList& info                 ,
                                      const CORE::CString& optionalKeyPrefix ,
                                      bool statLikeValuesOnly                ,
                                      bool redisIdsAsFloat                   )
{GUCEF_TRACE;

    if ( GUCEF_NULL != replyNode )
    {
        int type = replyNode->type;
        if ( REDIS_REPLY_ARRAY == type )
        {
            size_t i=0;
            size_t infoCount = replyNode->elements;
            while ( i < infoCount )
            {
                CORE::CString infoElementName;
                CORE::CString infoElementShortName;
                if ( REDIS_REPLY_STRING == replyNode->element[ i ]->type )
                {
                    infoElementShortName = replyNode->element[ i ]->str;
                    infoElementName = optionalKeyPrefix + infoElementShortName;
                    ++i;

                    switch( replyNode->element[ i ]->type )
                    {
                        case REDIS_REPLY_VERB:
                        case REDIS_REPLY_STATUS:
                        case REDIS_REPLY_STRING:
                        {
                            bool isRedisId = false;
                            if ( redisIdsAsFloat )
                            {
                                if ( "last-generated-id" == infoElementShortName )
                                {
                                    isRedisId = true;
                                    CORE::CString idValue = replyNode->element[ i ]->str;
                                    idValue = idValue.ReplaceChar( '-', '.' );
                                    info.Set( infoElementName, CORE::StringToDouble( idValue ) );
                                }
                            }

                            if ( !statLikeValuesOnly && !isRedisId )
                                info.Set( infoElementName, replyNode->element[ i ]->str );
                            break;
                        }
                        case REDIS_REPLY_ERROR:
                        case REDIS_REPLY_INTEGER:
                        {
                            info.Set( infoElementName, replyNode->element[ i ]->integer );
                            break;
                        }
                        case REDIS_REPLY_DOUBLE:
                        {
                            info.Set( infoElementName, replyNode->element[ i ]->dval );
                            break;
                        }
                        case REDIS_REPLY_BOOL:
                        {
                            if ( !statLikeValuesOnly )
                                info.Set( infoElementName, CORE::StringToBool( replyNode->element[ i ]->str ) );
                            break;
                        }
                        case REDIS_REPLY_ARRAY:
                        {
                            return GetRedisStreamInfo( replyNode->element[ i ], info, optionalKeyPrefix, statLikeValuesOnly, redisIdsAsFloat );
                        }
                        default:
                            break;
                    }
                }
                else
                    break;
                ++i;
            }
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

RedisNodeWithPipe*
RedisInfoService::FindNodeInfo( CORE::UInt32 hashSlot )
{GUCEF_TRACE;

    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        RedisNodeWithPipe& node = (*i).second;
        if ( node.startSlot <= hashSlot && node.endSlot >= hashSlot )
            return &node;
        if ( node.endSlot > hashSlot )
            break;
        ++i;
    }

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

RedisNodeWithPipe*
RedisInfoService::FindNodeInfo( const CORE::CString& hashable )
{GUCEF_TRACE;

    CORE::UInt32 hashSlot = CalculateRedisHashSlot( hashable );
    return FindNodeInfo( hashSlot );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisStreamInfo( const CORE::CString& streamName        ,
                                      CORE::CValueList& info                 ,
                                      const CORE::CString& optionalKeyPrefix ,
                                      bool statLikeValuesOnly                ,
                                      bool redisIdsAsFloat                   )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
        return false;

    RedisNodeWithPipe* node = FindNodeInfo( streamName );
    if ( GUCEF_NULL == node )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: No known node could house the hash slot for stream \"" + streamName + "\"" );
        return false;
    }

    try
    {
        CORE::CString xinfoCmd( "XINFO" );
        CORE::CString streamParam( "STREAM" );

        sw::redis::StringView xinfoCmdSV( xinfoCmd.C_String(), xinfoCmd.Length() );
        sw::redis::StringView streamParamSV( streamParam.C_String(), streamParam.Length() );
        sw::redis::StringView streamNameParamSV( streamName.C_String(), streamName.Length() );

        node->redisPipe->command( xinfoCmdSV, streamParamSV, streamNameParamSV );
        sw::redis::QueuedReplies redisReplies = node->redisPipe->exec();
        size_t replyCount = redisReplies.size();
        if ( replyCount > 0 )
        {
            redisReply& reply = redisReplies.get( 0 );
            return GetRedisStreamInfo( &reply, info, optionalKeyPrefix, statLikeValuesOnly, redisIdsAsFloat );
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Obtained " + CORE::ToString( info.GetKeyCount() ) + " pieces of information" );
    }
    catch ( const sw::redis::MovedError& e )
    {
        if ( GUCEF_NULL != node )
        {
            ++node->redisErrorReplies;
            GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Redis Node " + node->nodeId + " : Redis++ Moved exception: " + e.what() );
        }
        else
        {
            ++m_redisClusterErrorReplies;
            GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Redis++ Moved exception: " + e.what() );
        }
        RedisReconnect();
        return false;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        if ( GUCEF_NULL != node )
        {
            ++node->redisTimeoutReplies;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Redis Node " + node->nodeId + " : Redis++ timeout exception: " + e.what() );
        }
        else
        {
            ++m_redisClusterTimeoutReplies;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Redis++ timeout exception: " + e.what() );
        }
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: Redis++ exception: " + e.what() );
        if ( GUCEF_NULL != node )
            ++node->redisErrorReplies;
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisStreamInfo: exception: " + e.what() );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisStreamInfoForAllStreams( CORE::CValueList& info  ,
                                                   bool statLikeValuesOnly ,
                                                   bool redisIdsAsFloat    )
{GUCEF_TRACE;

    bool totalSuccess = true;
    CORE::CString::StringSet::iterator i = m_filteredStreamNames.begin();
    while ( i != m_filteredStreamNames.end() )
    {
        CORE::CString keyPrefix = (*i).ReplaceChar( '.', '_' ).ReplaceChar( '{', '_' ).ReplaceChar( '}', '_' ) + '.';

        if ( m_settings.retainRedisInfo )
        {
            CORE::CValueList& streamInfo = m_cmdXinfoStreamMap[ (*i) ];
            streamInfo.Clear();
            if ( GetRedisStreamInfo( (*i), streamInfo, CORE::CString::Empty, statLikeValuesOnly, redisIdsAsFloat ) )
            {
                info.SetMultiple( streamInfo, keyPrefix );
                streamInfo[ "streamName" ] = (*i);
            }
            else
                totalSuccess = false;
        }
        else
        {
            totalSuccess = GetRedisStreamInfo( (*i), info, keyPrefix, statLikeValuesOnly, redisIdsAsFloat ) && totalSuccess;
        }
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisKeysForNode( RedisNodeWithPipe& node           ,
                                       CORE::CString::StringVector& keys ,
                                       const CORE::CString& keyType      )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() || GUCEF_NULL == node.redisPipe )
        return false;

    CORE::UInt32 keysFound = 0;
    try
    {
        static const CORE::CString scanCmd( "SCAN" );
        CORE::CString itteratorParam( "0" );
        static const CORE::CString typeParam( "TYPE" );
        static const CORE::CString countParam( "COUNT" );
        CORE::CString countValueParam( CORE::ToString( m_settings.redisScanCountSize ) );

        CORE::UInt32 iterationCounter = 0;
        sw::redis::StringView scanCmdSV( scanCmd.C_String(), scanCmd.Length() );
        do
        {
            sw::redis::StringView itteratorParamSV( itteratorParam.C_String(), itteratorParam.Length() );
            sw::redis::StringView typeParamSV( typeParam.C_String(), typeParam.Length() );
            sw::redis::StringView typeValueParamSV( keyType.C_String(), keyType.Length() );
            sw::redis::StringView countParamSV( countParam.C_String(), countParam.Length() );
            sw::redis::StringView countValueParamSV( countValueParam.C_String(), countValueParam.Length() );

            node.redisPipe->command( scanCmdSV, itteratorParamSV, typeParamSV, typeValueParamSV, countParamSV, countValueParamSV );
            sw::redis::QueuedReplies redisReplies = node.redisPipe->exec();
            size_t replyCount = redisReplies.size();

            for ( size_t r=0; r<replyCount; ++r )
            {
                redisReply& reply = redisReplies.get( r );
                if ( REDIS_REPLY_ARRAY == reply.type )
                {
                    size_t ittCount = reply.elements;
                    if ( ittCount > 0 && REDIS_REPLY_STRING == reply.element[ 0 ]->type )
                        itteratorParam = reply.element[ 0 ]->str;
                    else
                        return false;

                    if ( ittCount > 1 && REDIS_REPLY_ARRAY == reply.element[ 1 ]->type )
                    {
                        size_t keyCount = reply.element[ 1 ]->elements;
                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisKeysForNode: Received " + CORE::ToString( keyCount ) + " keys in SCAN iteration nr " + CORE::ToString( iterationCounter ) );

                        struct redisReply** keyList = reply.element[ 1 ]->element;
                        for ( size_t n=0; n<keyCount; ++n )
                        {
                            if ( REDIS_REPLY_STRING == keyList[ n ]->type )
                            {
                                keys.push_back( keyList[ n ]->str );
                                ++keysFound;
                            }
                        }
                    }
                    else
                        return false;
                }
            }
            ++iterationCounter;
        }
        while ( itteratorParam != "0" );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisKeys: Found " + CORE::ToString( keysFound ) + " keys for Redis node " + node.nodeId );
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisKeys: Redis Node " + node.nodeId + " : Redis++ timeout exception: " + e.what() );
        ++node.redisTimeoutReplies;
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisKeys: Redis Node " + node.nodeId + " : Redis++ exception: " + e.what() );
        ++node.redisErrorReplies;
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisKeys: Redis Node " + node.nodeId + " : exception: " + e.what() );
        RedisReconnect();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisClusterNodeMap( RedisNodeMap& nodeMap )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
        return false;

    try
    {
        CORE::CString clusterCmd( "CLUSTER" );
        CORE::CString nodesParam( "NODES" );

        sw::redis::StringView clusterCmdSV( clusterCmd.C_String(), clusterCmd.Length() );
        sw::redis::StringView nodesParamSV( nodesParam.C_String(), nodesParam.Length() );

        auto reply = m_redisContext->command( clusterCmdSV, nodesParamSV );
        if ( reply )
        {
            if ( REDIS_REPLY_STRING == reply->type )
            {
                CORE::CString allText = reply->str;
                CORE::CString::StringVector lines = allText.ParseElements( '\n', false );

                // First add all the primary nodes
                CORE::CString::StringVector::iterator i = lines.begin();
                while ( i != lines.end() )
                {
                    CORE::CString::StringVector properties = (*i).ParseElements( ' ', true );
                    if ( properties.size() >= 8 )
                    {
                        bool isReplica = properties[ 2 ].HasSubstr( "slave" ) > -1;
                        if ( !isReplica )
                        {
                            CORE::CString& nodeId = properties[ 0 ];
                            CORE::CString ipAndPort = properties[ 1 ].SubstrToChar( '@' );
                            //CORE::CString& parentNodeId = properties[ 3 ];
                            //bool isConnected = properties[ 7 ] == "connected";

                            CORE::Int32 startSlot = -1;
                            CORE::Int32 endSlot = -1;
                            if ( properties.size() >= 9 )
                            {
                                startSlot = CORE::StringToInt32( properties[ 8 ].SubstrToChar( '-', true ), -1 );
                                endSlot = CORE::StringToInt32( properties[ 8 ].SubstrToChar( '-', false ), -1 );
                            }

                            RedisNode& node = nodeMap[ startSlot ];
                            node.nodeId = nodeId;
                            node.host.SetHostnameAndPort( ipAndPort );
                            node.startSlot = startSlot;
                            node.endSlot = endSlot;
                        }
                    }
                    ++i;
                }

                // Now add the replicas
                i = lines.begin();
                while ( i != lines.end() )
                {
                    CORE::CString::StringVector properties = (*i).ParseElements( ' ', false );
                    if ( properties.size() >= 8 )
                    {
                        bool isReplica = properties[ 2 ].HasSubstr( "slave" ) > -1;
                        if ( isReplica )
                        {
                            CORE::CString& nodeId = properties[ 0 ];
                            CORE::CString ipAndPort = properties[ 1 ].SubstrToChar( '@' );
                            CORE::CString& parentNodeId = properties[ 3 ];
                            //bool isConnected = properties[ 7 ] == "connected";

                            RedisNodeMap::iterator n = nodeMap.begin();
                            while ( n != nodeMap.end() )
                            {
                                RedisNode* replicaNode = (*n).second.FindReplica( nodeId, parentNodeId, true );
                                if ( GUCEF_NULL != replicaNode )
                                {
                                    replicaNode->host.SetHostnameAndPort( ipAndPort );
                                    break;
                                }
                                ++n;
                            }
                        }
                    }
                    ++i;
                }
            }
        }
        return true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: Redis++ TimeoutError exception: " + e.what() );
        ++m_redisClusterTimeoutReplies;
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: Redis++ exception: " + e.what() );
        ++m_redisClusterErrorReplies;
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: exception: " + e.what() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::GetRedisClusterSlots( RedisNodeMap& nodeMap )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
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
                                node.host.SetHostnameAndPort( ip, (CORE::UInt16)port );
                            }
                        }
                    }
                }
            }
            else
            {
                if ( REDIS_REPLY_ERROR == type )
                {
                    ++m_redisClusterErrorReplies;
                    return false;
                }
            }
        }

    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterSlots: Redis++ TimeoutError exception: " + e.what() );
        ++m_redisClusterTimeoutReplies;
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterSlots: Redis++ exception: " + e.what() );
        ++m_redisClusterErrorReplies;
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):GetRedisClusterSlots: exception: " + e.what() );
        RedisReconnect();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::SerializeRedisClusterNodeMap( const RedisNodeMap& nodeMap, CORE::CDataNode& doc ) const
{GUCEF_TRACE;

    CORE::CDataNodeSerializableSettings settings;
    settings.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_AverageDetails;

    doc.Clear();
    doc.SetName( "RedisClusterNodes" );
    doc.SetNodeType( GUCEF_DATATYPE_ARRAY );
    RedisNodeMap::const_iterator i = nodeMap.begin();
    bool totalSuccess = true;
    while ( i != nodeMap.end() )
    {
        totalSuccess = (*i).second.Serialize( doc, settings ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::RedisDisconnect( void )
{GUCEF_TRACE;

    try
    {
        if ( !m_redisContext.IsNULL() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: Beginning cleanup" );

            RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
            while ( i != m_redisNodesMap.end() )
            {
                sw::redis::Pipeline* redisPipe = (*i).second.redisPipe;
                delete redisPipe;
                redisPipe = GUCEF_NULL;
                ++i;
            }
            m_redisNodesMap.clear();

            CRedisClusterKeyCache::Instance()->StopUpdatesForCluster( m_redisContext );
            m_redisContext.Unlink();

            {
                MT::CScopeWriterLock lock( m_lock );
                m_status[ "connected" ] = false;
            }

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: Finished cleanup" );
        }
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisClusterErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: Redis++ TimeoutError exception: " + e.what() );
        ++m_redisClusterTimeoutReplies;
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisClusterErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: Redis++ exception: " + e.what() );
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisDisconnect: exception: " + e.what() );
        RedisReconnect();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::RedisConnect( bool reset )
{GUCEF_TRACE;

    if ( reset )
        RedisDisconnect();

    try
    {
        if ( reset || m_redisContext.IsNULL() )
        {
            m_settings.redisAddress.Refresh();

            sw::redis::ConnectionOptions rppConnectionOptions;
            rppConnectionOptions.host = m_settings.redisAddress.GetHostname();  // Required.
            rppConnectionOptions.port = m_settings.redisAddress.GetPortInHostByteOrder(); // Optional. The default port is 6379.
            //rppConnectionOptions.password = "auth";   // Optional. No password by default.
            //rppConnectionOptions.db = 1;  // Optional. Use the 0th database by default.

            // Optional. Timeout before we successfully send request to or receive response from redis.
            // By default, the timeout is 0ms, i.e. never timeout and block until we send or receive successfuly.
            // NOTE: if any command is timed out, we throw a TimeoutError exception.
            rppConnectionOptions.socket_timeout = std::chrono::milliseconds( m_settings.redisConnectionOptionSocketTimeoutInMs );
            rppConnectionOptions.connect_timeout = std::chrono::milliseconds( m_settings.redisConnectionOptionConnectTimeoutInMs );

            rppConnectionOptions.keep_alive = m_settings.redisConnectionOptionKeepAlive;

            // Connect to the Redis cluster
            m_redisContext.Unlink();
            m_redisContext = RedisClusterPtr( new sw::redis::RedisCluster( rppConnectionOptions ) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisConnect: Successfully created a Redis context" );

        }

        ProvideRedisNodesDoc();

        if ( !RefreshRedisNodePipes() )
        {
            m_redisReconnectTimer->SetEnabled( true );
            return false;
        }

        if ( m_settings.gatherStreamInfo )
        {
            if ( IsStreamIndexingNeeded() )
            {
                // kick off the key scans and delta updates
                CRedisClusterKeyCache::Instance()->GetRedisKeys( m_redisContext, m_filteredStreamNames, "stream", m_settings.streamsToGatherInfoFrom );
            }
            else
            {
                m_filteredStreamNames = m_settings.streamsToGatherInfoFrom;
            }
        }

        {
            MT::CScopeWriterLock lock( m_lock );
            m_status[ "connected" ] = true;
        }

        return true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisConnect: Redis++ TimeoutError exception: " + e.what() );
        ++m_redisClusterTimeoutReplies;
        RedisReconnect();
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisConnect: Redis++ exception: " + e.what() );
        ++m_redisClusterErrorReplies;
        RedisReconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisConnect: exception: " + e.what() );
        RedisReconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::RedisReconnect( void )
{GUCEF_TRACE;

    RedisDisconnect();
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfoService(" + CORE::PointerToString( this ) + "):RedisReconnect: starting reconnect timer" );
    m_redisReconnectTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::SendKeyValueStats( const CORE::CValueList& kv        ,
                                     const CORE::CString& metricPrefix )
{GUCEF_TRACE;

    CORE::CValueList::TValueMap::const_iterator i = kv.GetDataBeginIterator();
    while ( i != kv.GetDataEndIterator() )
    {
        if ( !(*i).second.empty() )
        {
            const CORE::CString& key = (*i).first;
            const CORE::CString& value = (*i).second.front();

            //if ( 0 != CORE::IsANumber( value.C_String() ) )
            {
                if ( -1 != value.HasChar( '.' ) )
                {
                    GUCEF_METRIC_GAUGE( metricPrefix + key, CORE::StringToDouble( value ), 1.0f );
                }
                else
                {
                    GUCEF_METRIC_GAUGE( metricPrefix + key, CORE::StringToInt64( value ), 1.0f );
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
RedisInfoService::IsStreamIndexingNeeded( void ) const
{GUCEF_TRACE;

    CORE::CString::StringSet::const_iterator i = m_settings.streamsToGatherInfoFrom.begin();
    while ( i != m_settings.streamsToGatherInfoFrom.end() )
    {
        if ( (*i).HasChar( '*' ) > -1 )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::PointerToString( this ) + "):IsStreamIndexingNeeded: Determined that at least one stream has wildcards and thus indexing is needed: " + (*i) );
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnRedisKeyCacheUpdate( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRedisClusterKeyCache::CacheUpdateEventData* cacheUpdate = static_cast< CRedisClusterKeyCache::CacheUpdateEventData* >( eventData );
    if ( GUCEF_NULL != cacheUpdate )
    {
        if ( cacheUpdate->redisCluster == m_redisContext )
        {
            CORE::CString::StringSet::iterator i = m_settings.streamsToGatherInfoFrom.begin();
            while ( i != m_settings.streamsToGatherInfoFrom.end() )
            {
                if ( (*i).HasChar( '*' ) > -1 )
                {
                    CORE::CString::StringSet::iterator n = cacheUpdate->newKeys.begin();
                    while ( n != cacheUpdate->newKeys.end() )
                    {
                        if ( (*n).WildcardEquals( (*i), '*', true ) )
                        {
                            m_filteredStreamNames.insert( (*n) );
                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnRedisKeyCacheUpdate: Discovered new key matching our search pattern: " + (*n) );
                        }
                        ++n;
                    }

                    n = cacheUpdate->deletedKeys.begin();
                    while ( n != cacheUpdate->deletedKeys.end() )
                    {
                        if ( (*n).WildcardEquals( (*i), '*', true ) )
                        {
                            m_filteredStreamNames.erase( (*n) );
                            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnRedisKeyCacheUpdate: Discovered that key matching our search pattern has been deleted: " + (*n) );
                        }
                        ++n;
                    }
                }
                ++i;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_redisReconnectTimer->SetEnabled( false );

    if ( !RedisConnect( true ) )
    {
        {
            MT::CScopeWriterLock lock( m_lock );
            m_status[ "reconnecting" ] = true;
        }
        m_redisReconnectTimer->SetEnabled( true );
    }
    {
        MT::CScopeWriterLock lock( m_lock );
        m_status[ "reconnecting" ] = false;
    }
}



/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnVfsInitCompleted( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnVfsInitCompleted" );
    m_vfsIsInitialized = true;
    RedisConnect( false );
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::SendErrorReplyStats( const CORE::CString& metricPrefix )
{GUCEF_TRACE;

    CORE::UInt32 clusterErrorReplyCount = GetRedisClusterErrorRepliesCounter( true );
    CORE::UInt32 clusterTimeoutReplyCount = GetRedisClusterTimeoutRepliesCounter( true );

    CORE::CString clusterMetricPrefix = metricPrefix + "ClusterErrorReplies";
    GUCEF_METRIC_COUNT( clusterMetricPrefix, clusterErrorReplyCount, 1.0f );
    clusterMetricPrefix = metricPrefix + "ClusterTimeoutReplies";
    GUCEF_METRIC_COUNT( clusterMetricPrefix, clusterTimeoutReplyCount, 1.0f );

    CORE::CValueList localErrorStats;
    localErrorStats[ "redisClusterErrorReplies" ] = clusterErrorReplyCount;
    localErrorStats[ "redisClusterTimeoutReplies" ] = clusterTimeoutReplyCount;

    RedisNodeWithPipeMap::iterator i = m_redisNodesMap.begin();
    while ( i != m_redisNodesMap.end() )
    {
        RedisNodeWithPipe& node = (*i).second;

        CORE::UInt32 nodeErrorReplyCount = node.GetRedisErrorRepliesCounter( true );
        CORE::UInt32 nodeTimeoutReplyCount = node.GetRedisTimeoutRepliesCounter( true );

        CORE::CString nodeMetricPrefix = metricPrefix + node.nodeId + ".ErrorReplies";
        GUCEF_METRIC_COUNT( nodeMetricPrefix, nodeErrorReplyCount, 1.0f );
        nodeMetricPrefix = metricPrefix + node.nodeId + ".TimeoutReplies";
        GUCEF_METRIC_COUNT( nodeMetricPrefix, nodeTimeoutReplyCount, 1.0f );

        localErrorStats[ "redisNodeErrorReplies." + node.nodeId ] = nodeErrorReplyCount;
        localErrorStats[ "redisNodeTimeoutReplies." + node.nodeId ] = nodeTimeoutReplyCount;

        ++i;
    }

    {
        MT::CScopeWriterLock lock( m_lock );
        m_status.SetMultiple( localErrorStats );
    }
}

/*-------------------------------------------------------------------------*/

void
RedisInfoService::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_settings.gatherClusterInfo )
    {
        CORE::CValueList kv;
        if ( GetRedisClusterInfo( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained cluster-info" );

            // Cluster info is all numeric except for the cluster state
            // We turn the state into a number here so we can sent it as a stat
            static const CORE::CString clusterStateStatName = "cluster_state";
            CORE::CString clusterStateStr = kv.GetValueAlways( clusterStateStatName );
            if ( !clusterStateStr.IsNULLOrEmpty() )
            {
                if ( "ok" == clusterStateStr )
                {
                    static const CORE::CString clusterStateStatOk = "1";
                    kv.Set( clusterStateStatName, clusterStateStatOk );
                }
                else
                {
                    static const CORE::CString clusterStateStatFail = "0";
                    kv.Set( clusterStateStatName, clusterStateStatFail );
                }
            }
            CORE::CString metricPrefix = m_settings.metricPrefix + "ClusterInfo.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdClusterInfo = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain cluster-info" );
        }
    }
    if ( m_settings.gatherInfoCommandStats )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoCommandStats( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-command-stats" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "CommandStats.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoCommandstats = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-command-stats" );
        }
    }
    if ( m_settings.gatherInfoMemory )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoMemory( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-memory" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Memory.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoMemory = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-memory" );
        }
    }
    if ( m_settings.gatherInfoReplication )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoReplication( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-replication" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Replication.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoReplication = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-replication" );
        }
    }
    if ( m_settings.gatherInfoPersistence )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoPersistence( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-persistance" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Persistence.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoPersistence = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-persistance" );
        }
    }
    if ( m_settings.gatherInfoStats )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoStats( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-stats" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Stats.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoStats = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-stats" );
        }
    }
    if ( m_settings.gatherInfoClients )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoClients( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-clients" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Clients.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoClients = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-clients" );
        }
    }
    if ( m_settings.gatherInfoCpu )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoCpu( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-cpu" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Cpu.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoCpu = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-cpu" );
        }
    }
    if ( m_settings.gatherInfoKeyspace )
    {
        CORE::CValueList kv;
        if ( GetRedisInfoKeyspace( kv ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained info-keyspace" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "Keyspace.";
            SendKeyValueStats( kv, metricPrefix );

            if ( m_settings.retainRedisInfo )
                m_cmdInfoKeyspace = kv;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain info-keyspace" );
        }
    }
    if ( m_settings.gatherStreamInfo )
    {
        CORE::CValueList kv;
        if ( GetRedisStreamInfoForAllStreams( kv, true, true ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Obtained stream info" );

            CORE::CString metricPrefix = m_settings.metricPrefix + "StreamInfo.";
            SendKeyValueStats( kv, metricPrefix );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisInfoService(" + CORE::ToString( this ) + "):OnMetricsTimerCycle: Failed to obtain stream info" );
        }
    }
    if ( m_settings.gatherErrorReplyCount )
    {
        SendErrorReplyStats( m_settings.metricPrefix );
    }
}

/*-------------------------------------------------------------------------*/

RestApiRedisInfoInfoResource::RestApiRedisInfoInfoResource( RedisInfo* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiRedisInfoInfoResource::~RestApiRedisInfoInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiRedisInfoInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                         CORE::CDataNode& output             ,
                                         const CORE::CString& representation ,
                                         const CORE::CString& params         )
{GUCEF_TRACE;

    output.Clear();
    output.SetName( "info" );
    output.SetNodeType( GUCEF_DATATYPE_OBJECT );
    output.SetAttribute( "application", "redisinfo", GUCEF_DATATYPE_ASCII_STRING );
    output.SetAttribute( "appBuildDateTime", CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ ).ToIso8601DateTimeString( true, true ) );
    output.SetAttribute( "platformBuildDateTime", CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( false ) );
    #else
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( true ) );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiRedisInfoConfigResource::RestApiRedisInfoConfigResource( RedisInfo* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiRedisInfoConfigResource::~RestApiRedisInfoConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiRedisInfoConfigResource::Serialize( const CORE::CString& resourcePath   ,
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

RestApiRedisInfoConfigResource::TDeserializeState
RestApiRedisInfoConfigResource::Deserialize( const CORE::CString& resourcePath   ,
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
            if ( m_app->LoadConfig( loadedAppConfig, globalConfig ) )
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
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiRedisInfoConfigResource: IsGlobalStandbyEnabled is true. We will leave the app in standby mode" );
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
            if ( m_app->LoadConfig( loadedAppConfig, input ) )
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

RedisInfo::RedisInfo( void )
    : CORE::CObserver()
    , m_isInStandby( false )
    , m_globalStandbyEnabled( false )
    , m_redisHost()
    , m_redisPort()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_appConfig()
    , m_globalConfig()
    , m_infoServices()
    , m_settings()
    , m_appLock( true )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskDataFactory( CRedisClusterKeyPrunerTask::TaskType, &g_redisClusterKeyPrunerTaskDataFactory );
    taskManager.RegisterTaskConsumerFactory( CRedisClusterKeyPrunerTask::TaskType, &g_redisClusterKeyPrunerTaskConsumerFactory );
}

/*-------------------------------------------------------------------------*/

RedisInfo::~RedisInfo()
{GUCEF_TRACE;

    m_httpServer.Close();

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.UnregisterTaskDataFactory( CRedisClusterKeyPrunerTask::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CRedisClusterKeyPrunerTask::TaskType );
}

/*-------------------------------------------------------------------------*/

bool
RedisInfo::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_globalStandbyEnabled;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfo::Start( void )
{GUCEF_TRACE;

    m_isInStandby = true;
    bool success = SetStandbyMode( m_globalStandbyEnabled );

    if ( success )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo: Opening REST API" );
        if ( m_httpServer.Listen() )
        {
            success = true;
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo: REST API port opened successfully" );
        }
        else
        {
            success = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RedisInfo: Unable to open REST API port (port in use?)" );
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
RedisInfo::SetStandbyMode( bool putInStandbyMode )
{GUCEF_TRACE;

    // Check if we need to do anything
    if ( m_isInStandby == putInStandbyMode )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo:SetStandbyMode( " + CORE::BoolToString( putInStandbyMode ) + " ): Already in the desired mode (" + CORE::BoolToString( m_isInStandby ) + "), nothing to do" );
        return true;
    }

    if ( putInStandbyMode )
    {
        bool totalSuccess = true;
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        // Signal all threads to stop gracefully
        // Since this standby operation is global not per cluster we signal all to stop before
        // we start any waiting operation
        TStringToInfoServiceMap::iterator i = m_infoServices.begin();
        while ( i != m_infoServices.end() )
        {
            RedisInfoServicePtr clusterSvc = (*i).second;
            if ( !threadPool->RequestTaskToStop( clusterSvc.StaticCast< CORE::CTaskConsumer >(), false ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo:SetStandbyMode( true ): Failed to signal task for redis info target \"" + (*i).first + "\" to stop" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo:SetStandbyMode( true ): Requested task for redis info target \"" + (*i).first + "\" to stop" );
            }
            ++i;
        }

        // Now actually wait for the threads to be finished
        i = m_infoServices.begin();
        while ( i != m_infoServices.end() )
        {
            RedisInfoServicePtr clusterSvc = (*i).second;
            if ( !clusterSvc->WaitForTaskToFinish( 30000 ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo:SetStandbyMode( true ): Failed to wait for task to finish for target \"" + (*i).first );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo:SetStandbyMode( true ): Finished wait for task to finish for target \"" + (*i).first );
            }
            ++i;
        }

        m_isInStandby = totalSuccess;
        return totalSuccess;
    }
    else
    {
        bool totalSuccess = true;

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        TStringToSettingsMap::iterator i = m_settings.begin();
        while ( i != m_settings.end() )
        {
            RedisInfoServicePtr redisInfoTarget( new RedisInfoService() );
            if ( redisInfoTarget->LoadConfig( (*i).second ) )
            {
                if ( threadPool->StartTask( redisInfoTarget ) )
                {
                    // Check if we have a pre-existing cluster service for a cluster with the same name
                    // If so clean up its links first
                    TStringToInfoServiceMap::iterator s = m_infoServices.find( (*i).first );
                    if ( s != m_infoServices.end() )
                    {
                        RedisInfoServicePtr oldRedisInfoTarget = (*s).second;
                        if ( !oldRedisInfoTarget.IsNULL() )
                        {
                            oldRedisInfoTarget->DisconnectHttpRouting( m_httpRouter );
                        }
                    }

                    // Hook up the new cluster info service
                    m_infoServices[ (*i).first ] = redisInfoTarget;
                    redisInfoTarget->ConnectHttpRouting( m_httpRouter );

                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo::SetStandbyMode( false ): Successfully started task for redis info target \"" + (*i).second.clusterName + "\"" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo::SetStandbyMode( false ): Failed to start task for redis info target \"" + (*i).second.clusterName + "\"" );
                    totalSuccess = false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo::SetStandbyMode( false ): Failed to set settings for task for redis info target \"" + (*i).second.clusterName + "\"" );
                totalSuccess = false;
            }
            ++i;
        }

        m_isInStandby = !totalSuccess;
        return totalSuccess;
    }
}

/*-------------------------------------------------------------------------*/

bool
RedisInfo::LoadConfig( const CORE::CValueList& appConfig   ,
                       const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_globalStandbyEnabled = CORE::StringToBool( appConfig.GetValueAlways( "globalStandbyEnabled" ), false );
    CORE::CDataNode::TDataNodeVector redisInfoTargets = globalConfig.SearchForAll( "RedisInfoTarget", '/', true, true );
    CORE::CDataNode::TDataNodeVector::iterator i = redisInfoTargets.begin();
    while ( i != redisInfoTargets.end() )
    {
        Settings redisTargetSettings;
        if ( redisTargetSettings.LoadConfig( *(*i) ) )
        {
            m_settings[ redisTargetSettings.clusterName ] = redisTargetSettings;
        }
        ++i;
    }

    m_appConfig = appConfig;
    m_globalConfig.Copy( globalConfig );

    m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "restApiPort" ) ), 10000 ) );

    m_httpRouter.SetResourceMapping( "/info", ( new RestApiRedisInfoInfoResource( this ) )->CreateSharedPtr()  );
    m_httpRouter.SetResourceMapping( "/config/appargs", ( new RestApiRedisInfoConfigResource( this, true ) )->CreateSharedPtr() );
    m_httpRouter.SetResourceMapping( "/config", ( new RestApiRedisInfoConfigResource( this, false ) )->CreateSharedPtr()  );
    m_httpRouter.SetResourceMapping( "/v1/clusters", ( new TStringToInfoServiceMapWebResource( "RedisClusters", "RedisCluster", "clusterName", &m_infoServices, &m_appLock ) )->CreateSharedPtr() );
    m_httpRouter.SetResourceMapping( CORE::ResolveVars( appConfig.GetValueAlways( "RestBasicHealthUri", "/health/basic" ).AsString() ), RestApiRedisInfoInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

    if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisInfo::LoadConfig: Failed to set up task management API" );
    }

    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
RedisInfo::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
RedisInfo::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
