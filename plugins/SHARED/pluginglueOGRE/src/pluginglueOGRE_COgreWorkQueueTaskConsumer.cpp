/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H
#include "pluginglueOGRE_COgreWorkQueueTaskData.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKDATA_H ? */

#include "pluginglueOGRE_COgreWorkQueueTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreWorkQueueTaskConsumer::TaskType = "GUCEF::PLUGINGLUE::OGRE::COgreWorkQueueTaskConsumer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWorkQueueTaskConsumer::COgreWorkQueueTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWorkQueueTaskConsumer::~COgreWorkQueueTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreWorkQueueTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreWorkQueueTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::OGRE::COgreWorkQueueTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
COgreWorkQueueTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    const COgreWorkQueueTaskData* data = MT::CICloneable::CloneableAs< COgreWorkQueueTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->GetCore().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWorkQueueTaskConsumer: The worker has no Ogre work queue to work for" );
        return true;
    }

    // the worker runs until the queue shuts down or the pool asks it to stop; the task is done afterwards
    COgreWorkQueueCorePtr core( data->GetCore() );
    core->RunWorker( data->GetWorkerIndex(), *this );
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreWorkQueueTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
