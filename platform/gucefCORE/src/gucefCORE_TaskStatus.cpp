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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_TaskStatus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

 TTaskStatus 
 TaskStatusStringToTaskStatus( const char* taskStatusStr )
{GUCEF_TRACE;

    if ( GUCEF_NULL == taskStatusStr )
        return ETaskStatus::TASKSTATUS_UNDEFINED;

    if ( 0 == strcmp( taskStatusStr, "INVALID_STATUS" ) )
        return ETaskStatus::TASKSTATUS_INVALID_STATUS;

    if ( 0 == strcmp( taskStatusStr, "TASKTYPE_INVALID" ) )
        return ETaskStatus::TASKSTATUS_TASKTYPE_INVALID;

    if ( 0 == strcmp( taskStatusStr, "TASKDATA_INVALID" ) )
        return ETaskStatus::TASKSTATUS_TASKDATA_INVALID;

    if ( 0 == strcmp( taskStatusStr, "TASK_WITH_TYPE_EXISTS" ) )
        return ETaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS;    

    if ( 0 == strcmp( taskStatusStr, "TASK_CHAINING_FAILED" ) )
        return ETaskStatus::TASKSTATUS_TASK_CHAINING_FAILED;

    if ( 0 == strcmp( taskStatusStr, "CHAIN_PREREQ_FAILED" ) )
        return ETaskStatus::TASKSTATUS_CHAIN_PREREQ_FAILED;
    
    if ( 0 == strcmp( taskStatusStr, "RESOURCE_NOT_AVAILABLE" ) )
        return ETaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE;

    if ( 0 == strcmp( taskStatusStr, "RESOURCE_LIMIT_REACHED" ) )
        return ETaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;       

    if ( 0 == strcmp( taskStatusStr, "QUEUEING_FAILED" ) )
        return ETaskStatus::TASKSTATUS_QUEUEING_FAILED;

    if ( 0 == strcmp( taskStatusStr, "SETUP" ) )
        return ETaskStatus::TASKSTATUS_SETUP;

    if ( 0 == strcmp( taskStatusStr, "SETUP_FAILED" ) )
        return ETaskStatus::TASKSTATUS_SETUP_FAILED;

    if ( 0 == strcmp( taskStatusStr, "QUEUED" ) )
        return ETaskStatus::TASKSTATUS_QUEUED;

    if ( 0 == strcmp( taskStatusStr, "CHAIN_PREREQ_QUEUED" ) )
        return ETaskStatus::TASKSTATUS_CHAIN_PREREQ_QUEUED;    

    if ( 0 == strcmp( taskStatusStr, "STARTUP" ) )
        return ETaskStatus::TASKSTATUS_STARTUP;

    if ( 0 == strcmp( taskStatusStr, "STARTUP_FAILED" ) )
        return ETaskStatus::TASKSTATUS_STARTUP_FAILED;

    if ( 0 == strcmp( taskStatusStr, "EXCEPTION_DURING_RUN" ) )
        return ETaskStatus::TASKSTATUS_EXCEPTION_DURING_RUN;

    if ( 0 == strcmp( taskStatusStr, "RUNNING" ) )
        return ETaskStatus::TASKSTATUS_RUNNING;

    if ( 0 == strcmp( taskStatusStr, "PAUSED" ) )
        return ETaskStatus::TASKSTATUS_PAUSED;

    if ( 0 == strcmp( taskStatusStr, "RESUMED" ) )
        return ETaskStatus::TASKSTATUS_RESUMED;

    if ( 0 == strcmp( taskStatusStr, "STOPPED" ) )
        return ETaskStatus::TASKSTATUS_STOPPED;        

    if ( 0 == strcmp( taskStatusStr, "KILLED" ) )
        return ETaskStatus::TASKSTATUS_KILLED; 

    if ( 0 == strcmp( taskStatusStr, "FINISHED" ) )
        return ETaskStatus::TASKSTATUS_FINISHED;        

    return ETaskStatus::TASKSTATUS_UNDEFINED;
}

/*-------------------------------------------------------------------------*/

 TTaskStatus 
 TaskStatusStringToTaskStatus( const CString& taskStatusStr )
{GUCEF_TRACE;

    return TaskStatusStringToTaskStatus( taskStatusStr.C_String() );
}

/*-------------------------------------------------------------------------*/

const char*
TaskStatusToTaskStatusString( TTaskStatus taskStatus )
{GUCEF_TRACE;

    switch ( taskStatus )
    {
        case ETaskStatus::TASKSTATUS_INVALID_STATUS: return "INVALID_STATUS";
        case ETaskStatus::TASKSTATUS_TASKTYPE_INVALID: return "TASKTYPE_INVALID";
        case ETaskStatus::TASKSTATUS_TASKDATA_INVALID: return "TASKDATA_INVALID";
        case ETaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS: return "TASK_WITH_TYPE_EXISTS";
        case ETaskStatus::TASKSTATUS_TASK_CHAINING_FAILED: return "TASK_CHAINING_FAILED";
        case ETaskStatus::TASKSTATUS_CHAIN_PREREQ_FAILED: return "CHAIN_PREREQ_FAILED";
        case ETaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE: return "RESOURCE_NOT_AVAILABLE";
        case ETaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED: return "RESOURCE_LIMIT_REACHED";
        case ETaskStatus::TASKSTATUS_QUEUEING_FAILED: return "QUEUEING_FAILED";
        case ETaskStatus::TASKSTATUS_SETUP_FAILED: return "SETUP_FAILED";
        case ETaskStatus::TASKSTATUS_SETUP: return "SETUP";
        case ETaskStatus::TASKSTATUS_QUEUED: return "QUEUED";
        case ETaskStatus::TASKSTATUS_CHAIN_PREREQ_QUEUED: return "CHAIN_PREREQ_QUEUED";
        case ETaskStatus::TASKSTATUS_STARTUP: return "STARTUP";
        case ETaskStatus::TASKSTATUS_STARTUP_FAILED: return "STARTUP_FAILED";
        case ETaskStatus::TASKSTATUS_EXCEPTION_DURING_RUN: return "EXCEPTION_DURING_RUN";
        case ETaskStatus::TASKSTATUS_RUNNING: return "RUNNING";
        case ETaskStatus::TASKSTATUS_PAUSED: return "PAUSED";
        case ETaskStatus::TASKSTATUS_RESUMED: return "RESUMED";
        case ETaskStatus::TASKSTATUS_STOPPED: return "STOPPED";
        case ETaskStatus::TASKSTATUS_KILLED: return "KILLED";
        case ETaskStatus::TASKSTATUS_FINISHED: return "FINISHED";

        default:
        case ETaskStatus::TASKSTATUS_UNDEFINED:
        {
            return "UNDEFINED";
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
TaskStatusIsAnEndState( TTaskStatus taskStatus )
{GUCEF_TRACE;

    switch ( taskStatus )
    {
        case ETaskStatus::TASKSTATUS_INVALID_STATUS:
        case ETaskStatus::TASKSTATUS_TASKTYPE_INVALID:
        case ETaskStatus::TASKSTATUS_TASKDATA_INVALID:
        case ETaskStatus::TASKSTATUS_TASK_WITH_TYPE_EXISTS:
        case ETaskStatus::TASKSTATUS_TASK_CHAINING_FAILED:
        case ETaskStatus::TASKSTATUS_CHAIN_PREREQ_FAILED:
        case ETaskStatus::TASKSTATUS_QUEUEING_FAILED:
        case ETaskStatus::TASKSTATUS_SETUP_FAILED:
        case ETaskStatus::TASKSTATUS_STARTUP_FAILED:
        case ETaskStatus::TASKSTATUS_STOPPED:
        case ETaskStatus::TASKSTATUS_KILLED:
        case ETaskStatus::TASKSTATUS_EXCEPTION_DURING_RUN:
        case ETaskStatus::TASKSTATUS_FINISHED:
            return true;

        default:
            return false;
    }
}
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
