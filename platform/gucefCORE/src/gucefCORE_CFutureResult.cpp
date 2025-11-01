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

#include "gucefCORE_CFutureResult.h"

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

CFutureResult::CFutureResult( CTaskPtr task )
    : m_task( task )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFutureResult::CFutureResult( TTaskStatus taskStatus )
    : m_task()
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( !TaskStatusIsAnError( taskStatus ) )
    {
        taskStatus = TTaskStatus::TASKSTATUS_INVALID_STATUS;
    }

    m_task = CTask::CreateSharedObjWithParam( taskStatus );
}

/*-------------------------------------------------------------------------*/

CFutureResult::CFutureResult( const CFutureResult& src )
    : m_task( src.m_task )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

const CFutureResult&
CFutureResult::Await( Int32 timeoutInMs ) const
{GUCEF_TRACE;

    if ( !m_task.IsNULL() )               
    {
        MT::TLockStatus waitStatus = m_task->WaitForTaskToFinish( timeoutInMs );
        if ( MT::TLockStatus::LOCKSTATUS_OPERATION_FAILED == waitStatus ||
             MT::TLockStatus::LOCKSTATUS_WAIT_TIMEOUT == waitStatus      )
        {
            throw timeout_exception();
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFutureResult::IsReady( void ) const
{GUCEF_TRACE;

    if ( !m_task.IsNULL() )               
    {
        return m_task->IsTaskInEndState();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFutureResult::HasNoFuture( void ) const
{GUCEF_TRACE;

    if ( !m_task.IsNULL() )               
    {
        return m_task->IsTaskInErrorState();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFutureResult::HasAFuture( void ) const
{GUCEF_TRACE;

    return !HasNoFuture();
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CFutureResult::GetResult( Int32 timeoutInMs ) const
{GUCEF_TRACE;

    CFutureResult waitResult = Await( timeoutInMs );
    if ( waitResult.IsReady() )
        return m_task;
    return CTaskPtr();
}

/*-------------------------------------------------------------------------*/

CTask::TIntegerTypeUsedForTaskId
CFutureResult::GetTaskId( void ) const
{GUCEF_TRACE;

    CTaskPtr task = m_task;
    if ( !task.IsNULL() )
        return task->GetTaskId();
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
