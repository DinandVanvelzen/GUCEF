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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef REDISINFO_REDISCLUSTERNODECMDS_H
#include "redisinfo_RedisClusterNodeCmds.h"
#define REDISINFO_REDISCLUSTERNODECMDS_H
#endif /* REDISINFO_REDISCLUSTERNODECMDS_H ? */

#ifndef REDISINFO_CREDISCLUSTERKEYCACHE_H
#include "redisinfo_CRedisClusterKeyCache.h"
#define REDISINFO_CREDISCLUSTERKEYCACHE_H
#endif /* REDISINFO_CREDISCLUSTERKEYCACHE_H ? */

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

const CORE::CString CRedisClusterKeyPrunerTask::TaskType = "RedisClusterKeyPrunerTask";
const CORE::CString CRedisClusterKeyPrunerTaskData::ClassTypeName = "GUCEF::REDISINFO::CRedisClusterKeyPrunerTaskData";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRedisClusterKeyPrunerTask::CRedisClusterKeyPrunerTask( void )
    : CORE::CTaskConsumer()
    , CORE::CTSharedPtrCreator< CRedisClusterKeyPrunerTask, MT::CMutex >( this )
    , m_currentPage( 0 )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyPrunerTask::~CRedisClusterKeyPrunerTask()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool 
CRedisClusterKeyPrunerTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CRedisClusterKeyPrunerTask::GetKeyIdleTime( RedisClusterPtr redisCluster, const CORE::CString& key, CORE::UInt32 defaultIdleTime )
{GUCEF_TRACE;

    if ( redisCluster.IsNULL() )
        return defaultIdleTime;

    static const CORE::CString objectCmd( "OBJECT" );
    static const CORE::CString idletimeParam( "IDLETIME" );

    try
    {
        sw::redis::StringView objectCmdSV( objectCmd.C_String(), objectCmd.Length() );
        sw::redis::StringView idletimeParamSV( idletimeParam.C_String(), idletimeParam.Length() );
        sw::redis::StringView keyParamSV( key.C_String(), key.Length() );

        sw::redis::ReplyUPtr cmdReply = redisCluster->command( objectCmdSV, idletimeParamSV, keyParamSV );
        if ( cmdReply )
        {
            if ( REDIS_REPLY_INTEGER == cmdReply->type )
            {
                long long keyIdleTime = cmdReply->integer;
                return (CORE::UInt32) keyIdleTime;
            }
        }
            
        // we should not get here
        return defaultIdleTime;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, CORE::CString( "RedisClusterKeyPrunerTask:GetKeyIdleTime: Redis++ exception: " ) + e.what() );
        return defaultIdleTime;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, CORE::CString( "RedisClusterKeyPrunerTask:GetKeyIdleTime: STL exception: " ) + e.what() );
        return defaultIdleTime;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyPrunerTask::DeleteKey( RedisClusterPtr redisCluster, const CORE::CString& key )
{GUCEF_TRACE;

    if ( redisCluster.IsNULL() )
        return false;

    try
    {
        sw::redis::StringView keyParamSV( key.C_String(), key.Length() );
        long long itemsdeleted = redisCluster->del( keyParamSV );
        return 1 == itemsdeleted;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, CORE::CString( "RedisClusterKeyPrunerTask:DeleteKey: Redis++ exception: " ) + e.what() );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, CORE::CString( "RedisClusterKeyPrunerTask:DeleteKey: STL exception: " ) + e.what() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyPrunerTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    CORE::CICloneable* opaqueTaskData = task->GetTaskData();
    CRedisClusterKeyPrunerTaskData* prunerTaskData = static_cast< CRedisClusterKeyPrunerTaskData* >( opaqueTaskData );
    if ( prunerTaskData->redisCluster.IsNULL() )
    {
        // no cluster access, we are done
        return false;
    }
    
    CORE::CString::StringSet keyPage;
    if ( CRedisClusterKeyCache::Instance()->GetRedisKeys( prunerTaskData->redisCluster      , 
                                                          keyPage                           , 
                                                          prunerTaskData->keyType           , 
                                                          prunerTaskData->keyFilterPatterns , 
                                                          prunerTaskData->pruneBatchSize    , 
                                                          m_currentPage                     ) )
    {
        CORE::CString::StringSet::iterator i = keyPage.begin();
        while ( i != keyPage.end() && !IsDeactivationRequested() )
        {            
            CORE::UInt64 keyIdleTimeInSecs = GetKeyIdleTime( prunerTaskData->redisCluster, (*i), 0 );
            if ( keyIdleTimeInSecs > prunerTaskData->maxIdleTimeForPruningInSecs )
            {
                if ( DeleteKey( prunerTaskData->redisCluster, (*i) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterKeyPrunerTask: Pruned key \"" + (*i) + "\" which was idle for " + CORE::ToString( keyIdleTimeInSecs ) + " seconds" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterKeyPrunerTask: Failed to delete key \"" + (*i) + "\" which has been idle for " + CORE::ToString( keyIdleTimeInSecs ) + " seconds" );
                }
            }
            else
            {
                GUCEF_DEBUG_LOG_EVERYTHING( "RedisClusterKeyPrunerTask: key \"" + (*i) + "\" has been idle for " + CORE::ToString( keyIdleTimeInSecs ) + " seconds, no need to prune it" );
            }
            ++i;
        }        
        ++m_currentPage;
    }

    // We are done when we run out of keys to process
    return keyPage.empty();
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyPrunerTask::OnTaskEnding( CORE::CTaskPtr task ,
                                          bool willBeForced   )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnding( task, willBeForced );
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterKeyPrunerTask::OnTaskEnded( CORE::CTaskPtr task ,
                                         bool wasForced      )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString 
CRedisClusterKeyPrunerTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyPrunerTaskData::CRedisClusterKeyPrunerTaskData( void )
    : CORE::CStdDataNodeSerializableTaskData()
    , redisCluster()
    , maxIdleTimeForPruningInSecs( 604800 ) // 1 week
    , keyType( "stream" )
    , keyFilterPatterns()
    , pruneBatchSize( 5000 )
{GUCEF_TRACE;

    m_taskTypeName = CRedisClusterKeyPrunerTask::TaskType;
}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyPrunerTaskData::CRedisClusterKeyPrunerTaskData( const CRedisClusterKeyPrunerTaskData& src )
    : CORE::CStdDataNodeSerializableTaskData( src )
    , redisCluster( src.redisCluster )
    , maxIdleTimeForPruningInSecs( src.maxIdleTimeForPruningInSecs )
    , keyType( src.keyType )
    , keyFilterPatterns( src.keyFilterPatterns )
    , pruneBatchSize( src.pruneBatchSize )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedisClusterKeyPrunerTaskData::~CRedisClusterKeyPrunerTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CRedisClusterKeyPrunerTaskData::Clone( void ) const
{GUCEF_TRACE;

    return new CRedisClusterKeyPrunerTaskData( *this );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyPrunerTaskData::SerializeTaskData( CORE::CDataNode& domRootNode                        ,
                                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = domRootNode.SetAttribute( "maxIdleTimeForPruningInSecs", maxIdleTimeForPruningInSecs ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "keyType", keyType ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "keyFilterPatterns", CORE::StringSetToString( keyFilterPatterns ) ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "pruneBatchSize", pruneBatchSize ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterKeyPrunerTaskData::DeserializeTaskData( const CORE::CDataNode& domRootNode                  ,
                                                     const CORE::CDataNodeSerializableSettings& settings ) 
{GUCEF_TRACE;

    if ( CRedisClusterKeyPrunerTask::TaskType == m_taskTypeName )
    {
        maxIdleTimeForPruningInSecs = domRootNode.GetAttributeValueOrChildValueByName( "maxIdleTimeForPruningInSecs" ).AsUInt32( maxIdleTimeForPruningInSecs, true );
        keyType = domRootNode.GetAttributeValueOrChildValueByName( "keyType" ).AsString( keyType, true );
        keyFilterPatterns = CORE::StringToStringSet( domRootNode.GetAttributeValueOrChildValueByName( "keyFilterPatterns" ).AsString( CORE::StringSetToString( keyFilterPatterns ), true ) );
        pruneBatchSize = domRootNode.GetAttributeValueOrChildValueByName( "pruneBatchSize" ).AsUInt32( pruneBatchSize, true );
        return true;
    }    
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CRedisClusterKeyPrunerTaskData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISINFO */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
