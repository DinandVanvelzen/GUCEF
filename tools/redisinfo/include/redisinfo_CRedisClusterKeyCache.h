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

#ifndef REDISINFO_CREDISCLUSTERKEYCACHE_H
#define REDISINFO_CREDISCLUSTERKEYCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "hiredis.h"
#include "async.h"
#include "redis++.h"

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace REDISINFO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< sw::redis::RedisCluster, MT::CMutex >    RedisClusterPtr;

/*--------------------------------------------------------------------------*/

class CRedisClusterKeyCache : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent CacheUpdateEvent;

    typedef CORE::CTCloneableExpansion< CORE::CString::StringSet >  TopicDiscoveryEventData;                /**< list of discovered topics */

    class CacheUpdateInfo : public CORE::CICloneable
    {
        public:

        RedisClusterPtr redisCluster;
        CORE::CString keyType;
        
        TopicDiscoveryEventData newKeys;
        CORE::CString::StringSet deletedKeys;

        virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

        CacheUpdateInfo( void );
        CacheUpdateInfo( const CacheUpdateInfo& src );
        virtual ~CacheUpdateInfo();
    };
    typedef CacheUpdateInfo     CacheUpdateEventData;

    static CRedisClusterKeyCache* Instance( void );
    static void Deinstance( void );

    void SetRedisScanInterationCountSize( UInt32 countSize );

    UInt32 GetRedisScanInterationCountSize( void ) const;

    void SetIndexingIntervalInMs( UInt32 interval );

    UInt32 GetIndexingIntervalInMs( void ) const;

    void SetPersistKeyCacheSnapshot( bool persistSnapshot );

    void SetPersistKeyCacheSnapshotPath( const CORE::CString& snapshotPath );

    void SetPersistKeyCacheSnapshotCodec( const CORE::CString& snapshotCodec );

    bool GetRedisKeys( RedisClusterPtr redisCluster                                                        ,
                       CORE::CDataNode& keys                                                               ,
                       const CORE::CString& keyType                                                        ,
                       const CORE::CString::StringSet& globPatternsToMatch = CORE::CString::EmptyStringSet ,
                       CORE::UInt32 maxResults = GUCEF_UINT32MAX                                           ,
                       CORE::UInt32 page = 0                                                               );

    bool GetRedisKeys( RedisClusterPtr redisCluster                                                        ,
                       CORE::CString::StringSet& keys                                                      ,
                       const CORE::CString& keyType                                                        ,
                       const CORE::CString::StringSet& globPatternsToMatch = CORE::CString::EmptyStringSet ,
                       CORE::UInt32 maxResults = GUCEF_UINT32MAX                                           ,
                       CORE::UInt32 page = 0                                                               );

    bool GetRedisKeys( RedisClusterPtr redisCluster            ,
                       CORE::CString::StringSet& keys          ,
                       const CORE::CString& keyType            ,
                       const CORE::CString& globPatternToMatch );


    void StopUpdatesForCluster( RedisClusterPtr redisCluster );

    private:
    friend class CRedisClusterKeyCacheUpdateTask;

    typedef GUCEF::set< RedisClusterPtr > RedisClusterPtrSet;

    bool GetRedisClusterAccess( RedisClusterPtrSet& set );

    bool GetRedisKeyCache( RedisClusterPtr& redisCluster    ,
                           const CORE::CString& keyType     ,
                           CORE::CString::StringSet** keys  );

    bool ApplyKeyDelta( CacheUpdateInfo& updateInfo );

    bool SaveDocTo( const CORE::CDataNode& doc     , 
                    const CORE::CString& codecName , 
                    const CORE::CString& vfsPath   ) const;
    
    void OnKeyRefreshCycleCompleted( RedisClusterPtr& redisCluster ,
                                     const CORE::CString& keyType  );
    
    private:

    typedef GUCEF::map< CORE::CString, CORE::CString::StringSet > TStringToStringSetMap;
    typedef GUCEF::map< RedisClusterPtr, TStringToStringSetMap >  TRedisClusterPtrToTypeKeysMap;

    static void RegisterEvents( void );

    bool LazyStartCacheUpdateTask( void );

    CRedisClusterKeyCache( void );
    virtual ~CRedisClusterKeyCache();

    CORE::ThreadPoolPtr m_threadPool;
    CORE::CTaskConsumerPtr m_cacheUpdateTask;
    TRedisClusterPtrToTypeKeysMap m_cache;
    CORE::UInt32 m_scanCountSize;
    bool m_persistKeySnapshot;
    CORE::CString m_snapshotPath;
    CORE::CString m_snapshotCodec;
    CORE::UInt32 m_indexingIntervalInMs;

    static MT::CReadWriteLock g_dataLock;
    static CRedisClusterKeyCache* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* REDISINFO_CREDISCLUSTERKEYCACHE_H ? */
