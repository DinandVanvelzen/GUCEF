/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality 
 *                       providing a storage interface
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientTopic.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H ? */

#include "pubsubpluginSTORAGE_CStoragePubSubClientTopicVfsTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubClientTopicVfsTask::CStoragePubSubClientTopicVfsTask( CStoragePubSubClientTopic* ownerTopic )
    : CORE::CTaskConsumer()
    , m_ownerTopic( ownerTopic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoragePubSubClientTopicVfsTask::~CStoragePubSubClientTopicVfsTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool 
CStoragePubSubClientTopicVfsTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubClientTopicVfsTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    m_ownerTopic->PerformASyncVfsWork();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopicVfsTask::OnTaskEnding( CORE::CTaskPtr task ,
                                                bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStoragePubSubClientTopicVfsTask::OnTaskEnded( CORE::CTaskPtr task ,
                                               bool wasForced      )
{GUCEF_TRACE;

    m_ownerTopic->OnEndOfASyncVfsWork();
}

/*-------------------------------------------------------------------------*/

CORE::CString 
CStoragePubSubClientTopicVfsTask::GetType( void ) const
{GUCEF_TRACE;

    static const CORE::CString taskTypeName = "StoragePubSubClientTopicVfsTask";
    return taskTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
