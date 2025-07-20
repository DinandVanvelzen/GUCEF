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

#ifndef REDISINFO_CREDISCLUSTERKEYPRUNERTASK_H
#define REDISINFO_CREDISCLUSTERKEYPRUNERTASK_H

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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CStdDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H ? */

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
 *  Provides a task which prunes old keys from the cluster
 */
class CRedisClusterKeyPrunerTask : public CORE::CTaskConsumer ,
                                   public CORE::CTSharedPtrCreator< CRedisClusterKeyPrunerTask, MT::CMutex >
{
    public:

    static const CORE::CString TaskType;

    CRedisClusterKeyPrunerTask( void );
    virtual ~CRedisClusterKeyPrunerTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CORE::UInt64 GetKeyIdleTime( RedisClusterPtr redisCluster, const CORE::CString& key, CORE::UInt32 defaultIdleTime );

    bool DeleteKey( RedisClusterPtr redisCluster, const CORE::CString& key );

    private:

    CRedisClusterKeyPrunerTask( const CRedisClusterKeyPrunerTask& src ); // not implemented

    private:
    
    CORE::UInt32 m_currentPage;
};

/*-------------------------------------------------------------------------*/

class CRedisClusterKeyPrunerTaskData : public CORE::CStdDataNodeSerializableTaskData
{
    public:

    static const CORE::CString ClassTypeName;

    RedisClusterPtr redisCluster;
    CORE::UInt32 maxIdleTimeForPruningInSecs;
    CORE::CString keyType;
    CORE::CString::StringSet keyFilterPatterns;
    CORE::UInt32 pruneBatchSize;

    CRedisClusterKeyPrunerTaskData( void );
    CRedisClusterKeyPrunerTaskData( const CRedisClusterKeyPrunerTaskData& src );
    virtual ~CRedisClusterKeyPrunerTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SerializeTaskData( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeserializeTaskData( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CRedisClusterKeyPrunerTaskData, MT::CMutex > TRedisClusterKeyPrunerTaskDataFactory;     
typedef CORE::CTFactory< CORE::CTaskConsumer, CRedisClusterKeyPrunerTask, MT::CMutex >                      TRedisClusterKeyPrunerTaskConsumerFactory;     
typedef CORE::CTSharedPtr< CRedisClusterKeyPrunerTask, MT::CMutex >                                         CRedisClusterKeyPrunerTaskPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* REDISINFO_CREDISCLUSTERKEYPRUNERTASK_H ? */
