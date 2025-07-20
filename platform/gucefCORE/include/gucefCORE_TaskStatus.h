/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_TASKSTATUS_H
#define GUCEF_CORE_TASKSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum ETaskStatus : UInt8
{
    TASKSTATUS_UNDEFINED        = 0,    /**< not a real task status but rather an initialization value */

    TASKSTATUS_TASKTYPE_INVALID       ,    /**< the task has an invalid task type preventing its execution. this is an end state */
    TASKSTATUS_TASKDATA_INVALID       ,    /**< the task has invalid task data for the task type thus preventing its execution. this is an end state */
    TASKSTATUS_TASK_WITH_TYPE_EXISTS  ,    /**< if the task is intended to be unique and a task of the given type is already executing this signals a refusal to launch another. this is an end state */
    TASKSTATUS_TASK_CHAINING_FAILED   ,    /**< failed to create a chain of tasks. this is an end state */
    TASKSTATUS_RESOURCE_NOT_AVAILABLE ,    /**< the task depends on a resource for the task type which is not available thus preventing its execution */
    TASKSTATUS_RESOURCE_LIMIT_REACHED ,    /**< the task depends on a resource who's finite limit has been reached thus preventing its execution */
    TASKSTATUS_QUEUEING_FAILED        ,    /**< the task could not be placed in a queue */
    TASKSTATUS_SETUP_FAILED           ,    /**< the task setup phase failed */
    TASKSTATUS_STARTUP_FAILED         ,    /**< the task was started by a thread but the startup handler for the task reported an error and no further processing occurred */

    TASKSTATUS_SETUP            = 100 ,    /**< the task is in the setup phase and being defined */    
    TASKSTATUS_QUEUED                 ,    /**< the task is sitting in a queue waiting for a worker thread to pick it up, currently no thread is assigned */
    TASKSTATUS_STARTUP                ,    /**< the task is going through its setup phase with the newly assigned thread */
    TASKSTATUS_RUNNING                ,    /**< the task has been successfully started and a thread is currently working the task */
    TASKSTATUS_PAUSED                 ,    /**< the task is currently paused, startup had completed and work had started but subsequently the work was put on hold */
    TASKSTATUS_RESUMED                ,    /**< same as 'TASKSTATUS_RUNNING' except that the task had been paused during its run cycle */
    TASKSTATUS_STOPPED                ,    /**< the task has been stopped on external request instead of finishing a finite length task. this is an end state */
    TASKSTATUS_KILLED                 ,    /**< the task has been stopped on external request and is not resumable. this is an end state */
    TASKSTATUS_FINISHED                    /**< the task has stopped because it finished a finite length task. this is an end state */
};
typedef enum ETaskStatus TTaskStatus;

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP const char* 
TaskStatusToTaskStatusString( TTaskStatus taskStatus );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP TTaskStatus 
TaskStatusStringToTaskStatus( const CString& taskStatusStr );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP TTaskStatus 
TaskStatusStringToTaskStatus( const char* taskStatusStr );

/*-------------------------------------------------------------------------*/

inline bool
TaskStatusIsAnError( TTaskStatus taskStatus ) { return taskStatus < TTaskStatus::TASKSTATUS_SETUP; }

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
TaskStatusIsAnEndState( TTaskStatus taskStatus );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TASKSTATUS_H ? */
