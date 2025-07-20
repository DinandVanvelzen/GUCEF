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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H
#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopic.h"
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H ? */

#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopicReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace REDISCLUSTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopicReader::CRedisClusterPubSubClientTopicReader( CRedisClusterPubSubClientTopic* ownerTopic )
    : CORE::CTaskConsumer()
    , CORE::CTSharedPtrCreator< CRedisClusterPubSubClientTopicReader, MT::CMutex >( this )
    , m_ownerTopic( ownerTopic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopicReader::~CRedisClusterPubSubClientTopicReader()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool 
CRedisClusterPubSubClientTopicReader::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopicReader::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL != m_ownerTopic )
            m_ownerTopic->RedisRead();
    }
    catch ( const timeout_exception& )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CRedisClusterPubSubClientTopicReader: caught timeout_exception while attempting to a read cycle, will try again" );
    }
    
    // we never self-determine to be 'done'
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopicReader::OnTaskEnding( CORE::CTaskPtr task ,
                                                    bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopicReader::OnTaskEnded( CORE::CTaskPtr task ,
                                                   bool wasForced      )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString 
CRedisClusterPubSubClientTopicReader::GetType( void ) const
{GUCEF_TRACE;

    static const CORE::CString taskTypeName = "RedisClusterPubSubClientTopicReader";
    return taskTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISCLUSTER */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
