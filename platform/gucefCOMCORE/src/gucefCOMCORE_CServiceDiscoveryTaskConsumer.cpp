/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H
#include "gucefCOMCORE_CServiceDiscoveryTaskData.h"
#define GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H
#endif /* GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H ? */

#include "gucefCOMCORE_CServiceDiscoveryTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CServiceDiscoveryTaskConsumer::TaskType = "gucefCOMCORE::ServiceDiscovery";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceDiscoveryTaskConsumer::CServiceDiscoveryTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceDiscoveryTaskConsumer::~CServiceDiscoveryTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceDiscoveryTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceDiscoveryTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceDiscoveryTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CServiceDiscoveryTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceDiscoveryTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CServiceDiscoveryTaskData* data = const_cast< CServiceDiscoveryTaskData* >( CORE::CICloneable::CloneableAs< CServiceDiscoveryTaskData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data )
        return true;
    if ( data->source.IsCompleted() )
        return true;
    if ( data->source.IsCancellationRequested() )
    {
        data->source.Cancel();
        return true;
    }

    // the same work as the blocking form, on this thread of the pool
    data->found = CServiceFinder::FindOnce( data->settings, data->candidates );
    data->source.Complete( GUCEF_NEW CServiceDiscoveryTaskData( *data ), true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CServiceDiscoveryTaskConsumer::OnTaskEnded( CORE::CTaskPtr task ,
                               bool wasForced      )
{GUCEF_TRACE;

    // a task that ended without its look (a pool that stopped) still lets its waiter go
    if ( !task.IsNULL() )
    {
        CServiceDiscoveryTaskData* data = const_cast< CServiceDiscoveryTaskData* >( CORE::CICloneable::CloneableAs< CServiceDiscoveryTaskData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && !data->source.IsCompleted() )
            data->source.Cancel();
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
