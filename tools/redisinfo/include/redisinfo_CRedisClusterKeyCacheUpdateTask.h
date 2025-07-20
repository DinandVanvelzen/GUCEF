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

#ifndef REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H
#define REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef REDISINFO_CREDISCLUSTERKEYCACHE_H
#include "redisinfo_CRedisClusterKeyCache.h"
#define REDISINFO_CREDISCLUSTERKEYCACHE_H
#endif /* REDISINFO_CREDISCLUSTERKEYCACHE_H ? */

#include "hiredis.h"
#include "async.h"
#include "redis++.h"

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

/**
 *  Provides a blocking thread to act as a dedicated reader from a Redis stream
 *  This allows simulation of push mechanics
 */
class CRedisClusterKeyCacheUpdateTask : public CORE::CTaskConsumer ,
                                        public CORE::CTSharedPtrCreator< CRedisClusterKeyCacheUpdateTask, MT::CMutex >
{
    public:

    CRedisClusterKeyCacheUpdateTask( void );
    virtual ~CRedisClusterKeyCacheUpdateTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRedisClusterKeyCacheUpdateTask > TEventCallback;

    void RegisterEventHandlers( void );

    void
    OnIndexingTimerCycle( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    bool GetRedisKeys( RedisClusterPtr& clusterAccess              ,
                       const CORE::CString& keyType                ,
                       const CORE::CString::StringSet& currentKeys ,
                       CORE::CString::StringSet& newKeys           ,
                       CORE::CString::StringSet& deletedKeys       ,
                       CORE::Int64 maxNewKeys                      );

    CRedisClusterKeyCacheUpdateTask( const CRedisClusterKeyCacheUpdateTask& src ); // not implemented

    private:
    
    CORE::CTimer* m_indexingTimer;
    CORE::UInt32 m_scanCountSize;
};

typedef CORE::CTSharedPtr< CRedisClusterKeyCacheUpdateTask, MT::CMutex >   CRedisClusterKeyCacheUpdateTaskPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* REDISINFO_CREDISCLUSTERKEYCACHEUPDATETASK_H ? */
