/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefCORE_CFutureResultSource.h"

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

CFutureResultSource::CFutureResultSource( void )
    : m_task( CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RUNNING ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFutureResultSource::CFutureResultSource( TTaskStatus initialStatus )
    : m_task()
{GUCEF_TRACE;

    // an end state at creation makes no sense: that is a future that was never pending
    if ( TaskStatusIsAnEndState( initialStatus ) || TTaskStatus::TASKSTATUS_UNDEFINED == initialStatus )
        initialStatus = TTaskStatus::TASKSTATUS_RUNNING;

    m_task = CTask::CreateSharedObjWithParam( initialStatus );
}

/*-------------------------------------------------------------------------*/

CFutureResultSource::CFutureResultSource( const CFutureResultSource& src )
    : m_task( src.m_task )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFutureResultSource&
CFutureResultSource::operator=( const CFutureResultSource& src )
{GUCEF_TRACE;

    if ( this != &src )
        m_task = src.m_task;
    return *this;
}

/*-------------------------------------------------------------------------*/

CFutureResultSource::~CFutureResultSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFutureResult
CFutureResultSource::GetFuture( void ) const
{GUCEF_TRACE;

    return CFutureResult( m_task );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::SetStatus( TTaskStatus status )
{GUCEF_TRACE;

    if ( m_task.IsNULL() || TaskStatusIsAnEndState( status ) || TTaskStatus::TASKSTATUS_UNDEFINED == status )
        return false;

    MT::CScopeMutex lock( m_task->m_lock );
    if ( TaskStatusIsAnEndState( m_task->m_taskStatus ) )
        return false;

    m_task->SetTaskStatus( status );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::EndWith( TTaskStatus endState                ,
                              const CString& extraInfo            ,
                              CICloneable* resultData             ,
                              bool assumeOwnershipOfResultData    )
{GUCEF_TRACE;

    if ( m_task.IsNULL() || !TaskStatusIsAnEndState( endState ) )
        return false;

    // The lock makes 'is it ended already' and 'end it' one step: when threads race to complete, exactly one wins
    MT::CScopeMutex lock( m_task->m_lock );
    if ( TaskStatusIsAnEndState( m_task->m_taskStatus ) )
        return false;

    if ( GUCEF_NULL != resultData )
        m_task->SetTaskData( resultData, assumeOwnershipOfResultData );
    if ( !extraInfo.IsNULLOrEmpty() )
        m_task->SetTaskStatusExtraInfo( extraInfo );

    // Waiters poll the status, so the data and info above must be in place before the status changes
    m_task->SetTaskStatus( endState );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::Complete( void )
{GUCEF_TRACE;

    return EndWith( TTaskStatus::TASKSTATUS_FINISHED, CString::Empty, GUCEF_NULL, false );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::Complete( CICloneable* resultData, bool assumeOwnershipOfResultData )
{GUCEF_TRACE;

    return EndWith( TTaskStatus::TASKSTATUS_FINISHED, CString::Empty, resultData, assumeOwnershipOfResultData );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::Complete( TTaskStatus endState, const CString& extraInfo )
{GUCEF_TRACE;

    return EndWith( endState, extraInfo, GUCEF_NULL, false );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::Fail( TTaskStatus errorStatus, const CString& extraInfo )
{GUCEF_TRACE;

    if ( TTaskStatus::TASKSTATUS_UNDEFINED == errorStatus || !TaskStatusIsAnError( errorStatus ) || !TaskStatusIsAnEndState( errorStatus ) )
        return false;
    return EndWith( errorStatus, extraInfo, GUCEF_NULL, false );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::Cancel( void )
{GUCEF_TRACE;

    return EndWith( TTaskStatus::TASKSTATUS_CANCELLED, CString::Empty, GUCEF_NULL, false );
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::IsCompleted( void ) const
{GUCEF_TRACE;

    return m_task.IsNULL() || m_task->IsTaskInEndState();
}

/*-------------------------------------------------------------------------*/

bool
CFutureResultSource::IsCancellationRequested( void ) const
{GUCEF_TRACE;

    return !m_task.IsNULL() && m_task->IsCancellationRequested();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
