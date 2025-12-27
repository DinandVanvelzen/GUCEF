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

#include <assert.h>

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefCORE_CSingleTaskDelegator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CSingleTaskDelegator::CSingleTaskDelegator( const ThreadPoolPtr& threadPool ,
                                            const CTaskPtr& task            )
    : CTaskDelegator( threadPool, task )
{GUCEF_TRACE;

    GUCEF_ASSERT( !task.IsNULL() );
}

/*-------------------------------------------------------------------------*/

CSingleTaskDelegator::~CSingleTaskDelegator()
{GUCEF_TRACE;

    if ( !NotifyObservers( DestructionEvent ) ) return;
    UnsubscribeAllFromNotifier();
}

/*-------------------------------------------------------------------------*/

bool
CSingleTaskDelegator::OnThreadCycle( void* )
{GUCEF_TRACE;

    CTaskPtr task = m_task;    
    if ( !task.IsNULL() )
    {
        bool taskResult = false;
        bool attemptTimedOut = false;
        do
        {
            attemptTimedOut = false;
            try
            {
                taskResult = CTaskDelegator::ProcessTask( task );
            }
            catch ( const timeout_exception& )
            {
                GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "SingleTaskDelegator: caught timeout_exception while attempting to process task of type " + task->GetTaskType() + " with id " + ToString( task->GetTaskId() ) );

                attemptTimedOut = true;
            }
        }
        while ( attemptTimedOut );
        
        TaskCleanup( task );
        m_task.Unlink();
        return taskResult;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSingleTaskDelegator::ExecuteTaskFromCallingThread( void )
{GUCEF_TRACE;

    return OnThreadCycle( GUCEF_NULL );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
