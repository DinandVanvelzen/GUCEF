/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_VFS_CVFSFETCHTASKDATA_H
#include "gucefVFS_CVfsFetchTaskData.h"
#define GUCEF_VFS_CVFSFETCHTASKDATA_H
#endif /* GUCEF_VFS_CVFSFETCHTASKDATA_H ? */

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#include "gucefVFS_CVfsFetchScheduler.h"
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H
#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */

#include "gucefVFS_CVfsFetchTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsFetchTaskConsumer::TaskType = "GUCEF::VFS::CVfsFetchTaskConsumer";

/*-------------------------------------------------------------------------*/

CVfsFetchTaskConsumer::CVfsFetchTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchTaskConsumer::~CVfsFetchTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFetchTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    CVfsFetchTaskData* data = static_cast< CVfsFetchTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && GUCEF_NULL != data->scheduler && !data->job.IsNULL() )
        data->scheduler->RunJob( data->key, data->job );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool /* wasForced */ )
{GUCEF_TRACE;

    // the job goes with the task data; the scheduler holds its own reference as long as it needs one
    if ( task.IsNULL() )
        return;
    CVfsFetchTaskData* data = static_cast< CVfsFetchTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data )
        data->job.Unlink();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
