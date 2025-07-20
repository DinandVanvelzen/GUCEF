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

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */


#include "gucefCORE_CGenericCallbackTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CGenericCallbackTaskConsumer::TaskType = "GenericCallbackTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGenericCallbackTaskConsumer::CGenericCallbackTaskConsumer( void )
    : CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGenericCallbackTaskConsumer::~CGenericCallbackTaskConsumer()
{GUCEF_TRACE;

    try
    {
        SignalUpcomingDestruction();    
    }
    // we should not get exceptions here, the below is mainly for defensive coding as a lesser evil
    catch ( const timeout_exception& )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "GenericCallbackTaskConsumer:Destructor: encountered timeout exception" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "GenericCallbackTaskConsumer:Destructor: encountered std exception " + CString( e.what() ) );
    }
}

/*-------------------------------------------------------------------------*/

CString
CGenericCallbackTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CGenericCallbackTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::CORE::CGenericCallbackTaskConsumer";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CGenericCallbackTaskConsumer::OnTaskStart( CTaskPtr task )
{GUCEF_TRACE;

    return CTaskConsumer::OnTaskStart( task );
}

/*-------------------------------------------------------------------------*/

 bool
 CGenericCallbackTaskConsumer::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        CICloneable* opaqueTaskData = task->GetTaskData();
        CDeferredTask* deferredTask = static_cast< CDeferredTask* >( opaqueTaskData );

        if ( GUCEF_NULL != opaqueTaskData )
            deferredTask->Invoke();
    }

    // This type of task is by definition a single invocation of the callback per 'task'
    // As such we should only ever execute 1 cycle (unless say a timeout exception occured which the generic machinery will handle)
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGenericCallbackTaskConsumer::OnTaskEnded( CTaskPtr task  ,
                                           bool wasForced )
{GUCEF_TRACE;

    return CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
