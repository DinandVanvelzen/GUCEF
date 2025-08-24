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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H
#include "redisinfo_CRedisClusterKeyCacheUpdateTask.h"
#define REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H
#endif /* REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H */

#include "redisinfo_CRedisClusterKeyCache.h"

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

const CORE::CEvent CRedisClusterKeyCache::CacheUpdateEvent =    "GUCEF::PUBSUBPLUGIN::REDISCLUSTER::CRedisClusterKeyCache::CacheUpdateEvent";
#define GUCEF_DEFAULT_SCAN_COUNT_SIZE                           5000
#define GUCEF_DEFAULT_CACHE_UPDATE_TIMER_INTERVAL               ( 5 * 60 * 1000 ) // 5mins

MT::CReadWriteLock CRedisClusterKeyCache::g_dataLock( true );
CRedisClusterKeyCache* CRedisClusterKeyCache::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRedisClusterKeyCache::CacheUpdateInfo::CacheUpdateInfo( void )
    : CORE::CICloneable()
    , redisCluster()
    , keyType()
    , newKeys()
    , deletedKeys()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyCache::CacheUpdateInfo::CacheUpdateInfo( const CacheUpdateInfo& src )
    : CORE::CICloneable( src )
    , redisCluster( src.redisCluster )
    , keyType( src.keyType )
    , newKeys( src.newKeys )
    , deletedKeys( src.deletedKeys )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyCache::CacheUpdateInfo::~CacheUpdateInfo()
{GUCEF_TRACE;

    redisCluster.Unlink();
    keyType.Clear();
    newKeys.clear();
    deletedKeys.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRedisClusterKeyCache::CacheUpdateInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CacheUpdateInfo( *this );
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyCache::CRedisClusterKeyCache( void )
    : CORE::CObservingNotifier()
    , m_threadPool()
    , m_cacheUpdateTask()
    , m_cache()
    , m_scanCountSize( GUCEF_DEFAULT_SCAN_COUNT_SIZE )
    , m_persistKeySnapshot( false )
    , m_snapshotPath()
    , m_snapshotCodec( "json" )
    , m_indexingIntervalInMs( GUCEF_DEFAULT_CACHE_UPDATE_TIMER_INTERVAL )
{GUCEF_TRACE;

    m_threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "PUBSUBPLUGIN::REDISCLUSTER::RedisClusterKeyCache", true );
    m_cacheUpdateTask = ( GUCEF_NEW CRedisClusterKeyCacheUpdateTask() )->CreateSharedPtr().StaticCast< CORE::CTaskConsumer >();
}

/*-------------------------------------------------------------------------*/

 CRedisClusterKeyCache::~CRedisClusterKeyCache( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( g_dataLock );

    if ( !m_threadPool.IsNULL() )
        m_threadPool->RequestAllThreadsToStop( true, false, GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );
    m_threadPool.Unlink();

    m_cacheUpdateTask.Unlink();

    m_cache.clear();
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyCache*
CRedisClusterKeyCache::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeWriterLock lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            RegisterEvents();
            g_instance = GUCEF_NEW CRedisClusterKeyCache();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::RegisterEvents( void )
{GUCEF_TRACE;

    CacheUpdateEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::SetPersistKeyCacheSnapshot( bool persistSnapshot )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( g_dataLock );
    m_persistKeySnapshot = persistSnapshot;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::SetRedisScanInterationCountSize( UInt32 countSize )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( g_dataLock );
    m_scanCountSize = countSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CRedisClusterKeyCache::GetRedisScanInterationCountSize( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( g_dataLock );
    return m_scanCountSize;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCache::LazyStartCacheUpdateTask( void )
{GUCEF_TRACE;

    {
        MT::CScopeReaderLock lock( g_dataLock );
        if ( !m_cacheUpdateTask.IsNULL() && 0 != m_cacheUpdateTask->GetDelegatorThreadId() )
            return true;
    }
    {
        CORE::ThreadPoolPtr threadPool = m_threadPool;
        CORE::CTaskConsumerPtr consumer = m_cacheUpdateTask;
        if ( !threadPool.IsNULL() )
        {
            CORE::CFutureResult result = threadPool->StartTaskWithConsumer( consumer );
            return result.HasAFuture();
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCache::GetRedisKeys( RedisClusterPtr redisCluster                        ,
                                     CORE::CString::StringSet& keys                      ,
                                     const CORE::CString& keyType                        ,
                                     const CORE::CString::StringSet& globPatternsToMatch ,
                                     CORE::UInt32 maxResults                             ,
                                     CORE::UInt32 page                                   )
{GUCEF_TRACE;

    if ( redisCluster.IsNULL() || !LazyStartCacheUpdateTask() )
        return false;

    CORE::UInt64 resultOffset = page * (CORE::UInt64) maxResults;

    MT::CScopeReaderLock lock( g_dataLock );

    TStringToStringSetMap& keyMap = m_cache[ redisCluster ];
    CORE::CString::StringSet& cachedKeys = keyMap[ keyType ];

    if ( globPatternsToMatch.empty() )
    {
        // No pattern matching, return everything

        // first sanity check
        if ( resultOffset + maxResults > cachedKeys.size() )
        {
            if ( resultOffset > cachedKeys.size() )
                return true; // empty result set since we are past the max

            // cap the values
            CORE::UInt64 remnant = cachedKeys.size() - resultOffset;
            if ( remnant < maxResults )
                maxResults = (CORE::UInt32) remnant;
        }

        // move iterator to the desired offset
        CORE::CString::StringSet::iterator i = cachedKeys.begin();
        for ( CORE::UInt32 n=0; n<resultOffset; ++n )
        {
            if ( i != cachedKeys.end() )
                ++i;
            else
                break;
        }

        // copy the specified subset
        CORE::UInt32 resultEntries = 0;
        while ( i != cachedKeys.end() && resultEntries < maxResults )
        {
            keys.insert( (*i) );
            ++i; ++resultEntries;
        }

        return true;
    }

    // Match the glob patterns given for all cached keys
    CORE::UInt64 currentOffset = 0;
    CORE::CString::StringSet::iterator i = cachedKeys.begin();
    while ( i != cachedKeys.end() )
    {
        const CORE::CString& key = (*i);
        CORE::CString::StringSet::iterator g = globPatternsToMatch.begin();
        while ( g != globPatternsToMatch.end() )
        {
            const CORE::CString& globPattern = (*g);
            if ( key.WildcardEquals( globPattern, '*', true ) )
            {
                if ( currentOffset < resultOffset )
                {
                    if ( maxResults > keys.size() )
                    {
                        keys.insert( key );
                    }
                    else
                    {
                        // result set max reached
                        return true;
                    }
                }
                break;
            }
            ++g;
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCache::GetRedisKeys( RedisClusterPtr redisCluster            ,
                                     CORE::CString::StringSet& keys          ,
                                     const CORE::CString& keyType            ,
                                     const CORE::CString& globPatternToMatch )
{GUCEF_TRACE;

    CORE::CString::StringSet globPatternsToMatch;
    globPatternsToMatch.insert( globPatternToMatch );
    return GetRedisKeys( redisCluster, keys, keyType, globPatternsToMatch );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCache::GetRedisKeys( RedisClusterPtr redisCluster                        ,
                                     CORE::CDataNode& keys                               ,
                                     const CORE::CString& keyType                        ,
                                     const CORE::CString::StringSet& globPatternsToMatch ,
                                     CORE::UInt32 maxResults                             ,
                                     CORE::UInt32 page                                   )
{GUCEF_TRACE;

    keys.Clear();
    keys.SetName( keyType );
    keys.SetNodeType( GUCEF_DATATYPE_ARRAY );

    CORE::CString::StringSet flatIndexOfKeys;
    if ( GetRedisKeys( redisCluster, flatIndexOfKeys, keyType, globPatternsToMatch ) )
    {
        CORE::CString::StringSet::const_iterator i = flatIndexOfKeys.begin();
        while ( i != flatIndexOfKeys.end() )
        {
            keys.AddValueAsChild( (*i) );
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::StopUpdatesForCluster( RedisClusterPtr redisCluster )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( g_dataLock );
    m_cache.erase( redisCluster );
}

/*-------------------------------------------------------------------------*/

// Only intended to be used by the closely coupled task class
bool
CRedisClusterKeyCache::GetRedisClusterAccess( RedisClusterPtrSet& set )
{GUCEF_TRACE;

    try
    {
        MT::CScopeReaderLock lock( g_dataLock );

        TRedisClusterPtrToTypeKeysMap::iterator i = m_cache.begin();
        while ( i != m_cache.end() )
        {
            set.insert( (*i).first );
            ++i;
        }
    }
    catch ( const std::exception& )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

// Only intended to be used by the closely coupled task class
bool
CRedisClusterKeyCache::GetRedisKeyCache( RedisClusterPtr& redisCluster    ,
                                         const CORE::CString& keyType     ,
                                         CORE::CString::StringSet** keys  )
{GUCEF_TRACE;

    if ( redisCluster.IsNULL() || keys == GUCEF_NULL )
        return false;

    TStringToStringSetMap& keyMap = m_cache[ redisCluster ];
    CORE::CString::StringSet& cachedKeys = keyMap[ keyType ];

    *keys = &cachedKeys;
    return true;
}

/*-------------------------------------------------------------------------*/

// Only intended to be used by the closely coupled task class
bool
CRedisClusterKeyCache::ApplyKeyDelta( CacheUpdateInfo& updateInfo )
{GUCEF_TRACE;

    if ( updateInfo.redisCluster.IsNULL() )
        return false;

    try
    {
        // First we apply the updates to the actual cache
        // We keep a write lock on the cache during this time
        {
            MT::CScopeWriterLock lock( g_dataLock );

            TStringToStringSetMap& keyMap = m_cache[ updateInfo.redisCluster ];
            CORE::CString::StringSet& cachedKeys = keyMap[ updateInfo.keyType ];

            // delete the deleted keys
            CORE::CString::StringSet::iterator i = updateInfo.deletedKeys.begin();
            while ( i != updateInfo.deletedKeys.end() )
            {
                cachedKeys.erase( (*i) );
                ++i;
            }

            // Add the new keys
            i = updateInfo.newKeys.begin();
            while ( i != updateInfo.newKeys.end() )
            {
                cachedKeys.insert( (*i) );
                ++i;
            }
        }

        // Now that the changes have been applied and the write lock released we
        // notify of the changes
        NotifyObservers( CacheUpdateEvent, &updateInfo );
    }
    catch ( const std::exception& )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::SetIndexingIntervalInMs( UInt32 interval )
{GUCEF_TRACE;

    m_indexingIntervalInMs = interval;
}

/*-------------------------------------------------------------------------*/

UInt32
CRedisClusterKeyCache::GetIndexingIntervalInMs( void ) const
{GUCEF_TRACE;

    return m_indexingIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::SetPersistKeyCacheSnapshotPath( const CORE::CString& snapshotPath )
{GUCEF_TRACE;

    m_snapshotPath = snapshotPath;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::SetPersistKeyCacheSnapshotCodec( const CORE::CString& snapshotCodec )
{GUCEF_TRACE;

    m_snapshotCodec = snapshotCodec;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyCache::SaveDocTo( const CORE::CDataNode& doc     ,
                                  const CORE::CString& codecName ,
                                  const CORE::CString& vfsPath   ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.IsInitialized() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCache:SaveDocTo: Deferring until the VFS is initialized" );
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false );
    if ( !codec )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCache:SaveDocTo: Could not obtain codec for codecName : " + codecName );
        return false;
    }

    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "wb", true );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyCache:SaveDocTo: VFS could not provide access to file at path: " + vfsPath );
        return false;
    }

    return codec->StoreDataTree( &doc, file->GetAccess().GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyCache::OnKeyRefreshCycleCompleted( RedisClusterPtr& redisCluster ,
                                                   const CORE::CString& keyType  )
{GUCEF_TRACE;

    if ( m_persistKeySnapshot )
    {
        CORE::CDataNode keyIndexDocument( keyType, GUCEF_DATATYPE_ARRAY );
        if ( GetRedisKeys( redisCluster     ,
                           keyIndexDocument ,
                           keyType          ) )
        {
            SaveDocTo( keyIndexDocument, m_snapshotCodec, m_snapshotPath );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
