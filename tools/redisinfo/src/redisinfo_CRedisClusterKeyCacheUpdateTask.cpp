/*
 *  pubsubpluginREDISCLUSTER: Generic GUCEF COMCORE plugin for providing pubsub via Redis
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef REDISINFO_REDISCLUSTERNODECMDS_H
#include "redisinfo_RedisClusterNodeCmds.h"
#define REDISINFO_REDISCLUSTERNODECMDS_H
#endif /* REDISINFO_REDISCLUSTERNODECMDS_H ? */

#include "redisinfo_CRedisClusterKeyCacheUpdateTask.h"

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

#define GUCEF_DEFAULT_MAX_UPDATE_DELTA_SIZE             256
#define GUCEF_DEFAULT_SCAN_COUNT_SIZE                   1000     

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRedisClusterKeyCacheUpdateTask::CRedisClusterKeyCacheUpdateTask( void )
    : CORE::CTaskConsumer()
    , CORE::CTSharedPtrCreator< CRedisClusterKeyCacheUpdateTask, MT::CMutex >( this )
    , m_indexingTimer( GUCEF_NULL )
    , m_scanCountSize( GUCEF_DEFAULT_SCAN_COUNT_SIZE )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyCacheUpdateTask::~CRedisClusterKeyCacheUpdateTask()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCacheUpdateTask::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CRedisClusterKeyCacheUpdateTask::OnIndexingTimerCycle );
    SubscribeTo( m_indexingTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback                       );
}   

/*-------------------------------------------------------------------------*/

bool 
CRedisClusterKeyCacheUpdateTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( pulseGenerator.IsNULL() )
        return false;

    GUCEF_DELETE m_indexingTimer;
    m_indexingTimer = GUCEF_NEW CORE::CTimer( pulseGenerator, CRedisClusterKeyCache::Instance()->GetIndexingIntervalInMs() );

    RegisterEventHandlers();

    m_indexingTimer->SetEnabled( true );
    m_indexingTimer->TriggerNow();
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCacheUpdateTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    m_scanCountSize = CRedisClusterKeyCache::Instance()->GetRedisScanInterationCountSize();
    m_indexingTimer->SetInterval( CRedisClusterKeyCache::Instance()->GetIndexingIntervalInMs() );

    // We are never done.
    // Perpetual task running on a timer    
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCacheUpdateTask::OnIndexingTimerCycle( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRedisClusterKeyCache::RedisClusterPtrSet clusterAccessSet;
    if ( CRedisClusterKeyCache::Instance()->GetRedisClusterAccess( clusterAccessSet ) )
    {
        CRedisClusterKeyCache::RedisClusterPtrSet::iterator i = clusterAccessSet.begin();
        while ( i != clusterAccessSet.end() && !IsDeactivationRequested() )
        {
            CRedisClusterKeyCache::CacheUpdateInfo updateInfo;
            updateInfo.redisCluster = (*i);
            updateInfo.keyType = "stream";

            // Get the existing cached keys
            // Note we do so via pointer access to improve performance since this task and the cache itself are tightly coupled regardless
            MT::CScopeReaderLock readerLock( CRedisClusterKeyCache::Instance()->g_dataLock );
            CORE::CString::StringSet* allOldKeys = GUCEF_NULL;
            if ( CRedisClusterKeyCache::Instance()->GetRedisKeyCache( updateInfo.redisCluster, updateInfo.keyType, &allOldKeys ) && !IsDeactivationRequested() )
            {
                // Get all keys from the cluster and determine the key delta on the fly
                if ( GetRedisKeys( updateInfo.redisCluster, updateInfo.keyType, *allOldKeys, updateInfo.newKeys, updateInfo.deletedKeys, -1 ) && !IsDeactivationRequested() )
                {
                    // Now that we have determined the key delta we can release the reader lock
                    readerLock.EarlyUnlock();

                    // Delta change lists can become rather large, especially on initial cache population runs
                    // As such we check against a max delta update size so we can batch the updates into reasonable chunks
                    size_t totalDeltaChangeSize = updateInfo.newKeys.size() + updateInfo.deletedKeys.size();
                    if ( totalDeltaChangeSize <= GUCEF_DEFAULT_MAX_UPDATE_DELTA_SIZE )
                    {
                        // Now we perform the cache write action of applying the delta
                        CRedisClusterKeyCache::Instance()->ApplyKeyDelta( updateInfo );
                    }
                    else
                    {
                        // Too much: We will need to cut the massive delta update into smaller chunks
                        CRedisClusterKeyCache::CacheUpdateInfo updateInfoBatch;
                        updateInfoBatch.redisCluster = updateInfo.redisCluster;
                        updateInfoBatch.keyType = updateInfo.keyType;

                        // Lets try draining deleted keys first so we can free up resources, if applicable
                        size_t totalDeltaBatchChangeSize = 0;
                        while ( !updateInfo.deletedKeys.empty() )
                        {
                            CORE::CString::StringSet::iterator dk = updateInfo.deletedKeys.begin();
                            updateInfoBatch.deletedKeys.insert( (*dk) );
                            updateInfo.deletedKeys.erase( dk );
                            ++totalDeltaBatchChangeSize;

                            if ( ( totalDeltaBatchChangeSize >= GUCEF_DEFAULT_MAX_UPDATE_DELTA_SIZE ) || 
                                 ( updateInfo.newKeys.empty() && totalDeltaBatchChangeSize > 0 )       )
                            {
                                // Now we perform the cache write action of applying the delta batch
                                if ( CRedisClusterKeyCache::Instance()->ApplyKeyDelta( updateInfoBatch ) )
                                {
                                    updateInfoBatch.deletedKeys.clear();
                                    totalDeltaBatchChangeSize = 0;                                
                                }
                            }
                        }
                        // Now drain the new keys in reasonable batches
                        while ( !updateInfo.newKeys.empty() )
                        {
                            CORE::CString::StringSet::iterator nk = updateInfo.newKeys.begin();
                            updateInfoBatch.newKeys.insert( (*nk) );
                            updateInfo.newKeys.erase( nk );
                            ++totalDeltaBatchChangeSize;

                            if ( ( totalDeltaBatchChangeSize >= GUCEF_DEFAULT_MAX_UPDATE_DELTA_SIZE ) || 
                                 ( updateInfo.newKeys.empty() && totalDeltaBatchChangeSize > 0 )       )
                            {
                                // Now we perform the cache write action of applying the delta batch
                                if ( CRedisClusterKeyCache::Instance()->ApplyKeyDelta( updateInfoBatch ) )
                                {
                                    updateInfoBatch.deletedKeys.clear();
                                    updateInfoBatch.newKeys.clear();
                                    totalDeltaBatchChangeSize = 0;                                
                                }
                            }
                        }
                    }
                }
            }

            CRedisClusterKeyCache::Instance()->OnKeyRefreshCycleCompleted( updateInfo.redisCluster, updateInfo.keyType );

            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCacheUpdateTask::OnTaskEnding( CORE::CTaskPtr task ,
                                               bool willBeForced   )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnding( task, willBeForced );
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCacheUpdateTask::OnTaskEnded( CORE::CTaskPtr task ,
                                              bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DELETE m_indexingTimer;
    m_indexingTimer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString 
CRedisClusterKeyCacheUpdateTask::GetType( void ) const
{GUCEF_TRACE;

    static const CORE::CString taskTypeName = "RedisClusterKeyCacheUpdateTask";
    return taskTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCacheUpdateTask::GetRedisKeys( RedisClusterPtr& clusterAccess              ,
                                               const CORE::CString& keyType                ,
                                               const CORE::CString::StringSet& currentKeys ,
                                               CORE::CString::StringSet& newKeys           ,
                                               CORE::CString::StringSet& deletedKeys       ,
                                               CORE::Int64 maxNewKeys                      )
{GUCEF_TRACE;

    if ( clusterAccess.IsNULL() )
        return false;

    CORE::CString::StringSet stillExistsKeys;
    CORE::UInt32 shardKeysFound = 0;
    CORE::UInt32 keysFound = 0;
    try
    {               
        RedisClusterNodeCmds cmds( clusterAccess );
        
        CORE::UInt32 iterationCounter = 0;
        CORE::UInt32 scanIterattorId = 0;
        sw::redis::Shards redisNodeMap = clusterAccess->_pool.shards();
        sw::redis::Shards::iterator i = redisNodeMap.begin();
        while ( i != redisNodeMap.end() && ( maxNewKeys < 0 || keysFound < maxNewKeys ) )
        {
            sw::redis::Node& redisNode = (*i).second;
            sw::redis::ConnectionPoolSPtr nodeConnectionPool = clusterAccess->_pool.fetch( redisNode );
            sw::redis::SafeConnection safeConnection( *nodeConnectionPool );
            sw::redis::Connection& nodeConnection = safeConnection.connection();

            do
            {
                try
                {            
                    sw::redis::ReplyUPtr cmdReply = cmds.scan( nodeConnection, keyType, scanIterattorId, m_scanCountSize );
                    if ( cmdReply )
                    {
                        if ( REDIS_REPLY_ARRAY == cmdReply->type )
                        {
                            size_t ittCount = cmdReply->elements;

                            if ( ittCount > 0 && REDIS_REPLY_STRING == cmdReply->element[ 0 ]->type )
                            {
                                scanIterattorId = CORE::StringToUInt32( cmdReply->element[ 0 ]->str, scanIterattorId );
                            }
                            else
                                return false;

                            if ( ittCount > 1 && REDIS_REPLY_ARRAY == cmdReply->element[ 1 ]->type )
                            {
                                size_t keyCount = cmdReply->element[ 1 ]->elements;
                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::ToString( this ) + "):GetRedisKeys: Received " + CORE::ToString( keyCount ) + " keys in SCAN iteration nr " + CORE::ToString( iterationCounter ) );

                                struct redisReply** keyList = cmdReply->element[ 1 ]->element;
                                for ( size_t n=0; n<keyCount; ++n )
                                {
                                    if ( REDIS_REPLY_STRING == keyList[ n ]->type )
                                    {
                                        CORE::CString keyName( keyList[ n ]->str );
                                        if ( currentKeys.find( keyName ) == currentKeys.end() )
                                        {
                                            if ( !keyName.IsNULLOrEmpty() )
                                            {
                                                newKeys.insert( keyName );
                                                GUCEF_DEBUG_LOG_EVERYTHING( "RedisClusterKeyCacheUpdateTask(" + CORE::PointerToString( this ) + "):GetRedisKeys: New key found: " + keyName );
                                            }
                                        }
                                        else
                                        {
                                            stillExistsKeys.insert( keyName );
                                        }
                                
                                        ++keysFound; ++shardKeysFound;

                                        if ( maxNewKeys > 0 && keysFound >= maxNewKeys )
                                        {
                                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::ToString( this ) + "):GetRedisKeys: Max nr of new keys found, taking a break" );
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                                return false;
                        }
                    }
                    ++iterationCounter;
                }
                catch ( const sw::redis::TimeoutError& )
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::PointerToString( this ) + "):GetRedisKeys: Timeout trying get a batch of keys, taking a nap" );
                    MT::ThreadDelay( 100 );
                }
            }
            while ( scanIterattorId != 0 && !IsDeactivationRequested() && ( maxNewKeys < 0 || keysFound < maxNewKeys ) );

            ++i;
            
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::ToString( this ) + "):GetRedisKeys: Found " + CORE::ToString( shardKeysFound ) + " keys in Redis shard " + redisNode.host + ':' + CORE::ToString( redisNode.port ) );
            shardKeysFound = 0;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::ToString( this ) + "):GetRedisKeys: Found a total of " + CORE::ToString( keysFound ) + " keys in Redis cluster of type " + keyType );
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterKeyCacheUpdateTask(" + CORE::PointerToString( this ) + "):GetRedisKeys: Redis++ exception: " + e.what() );
        //++m_redisErrorReplies;
        //Disconnect();
        //if ( GUCEF_NULL != m_redisReconnectTimer )
        //    m_redisReconnectTimer->SetEnabled( true );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClient(" + CORE::PointerToString( this ) + "):GetRedisKeys: exception: " + e.what() );
        return false;
    }

    if ( !IsDeactivationRequested() && ( maxNewKeys < 0 || keysFound < maxNewKeys ) )
    {
        // Now we determine the deleted keys
        CORE::CString::StringSet::iterator o = currentKeys.begin();
        while ( o != currentKeys.end() )
        {
            if ( stillExistsKeys.find( (*o) ) == stillExistsKeys.end() )                
            {
                deletedKeys.insert( (*o) );
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisClusterKeyCacheUpdateTask(" + CORE::PointerToString( this ) + "):GetRedisKeys: Deleted key found: " + (*o) );
            }
            ++o;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
