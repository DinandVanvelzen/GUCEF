/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#include "gucefCORE_CJobSchedulerTask.h"

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

const CString CJobSchedulerTask::TaskType = "GUCEF::CORE::CJobSchedulerTask";

/*-------------------------------------------------------------------------*/

CJobSchedulerTask::CJobSchedulerTask( CJobScheduler* scheduler )
    : CTaskConsumer()
    , m_scheduler( scheduler )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobSchedulerTask::~CJobSchedulerTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerTask::OnTaskStart( CTaskPtr task )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_scheduler )
        return false;

    // the dispatcher sleeps until its next step: no periodic pulses on this thread meanwhile
    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->ForceStopOfPeriodicPulses();
    m_scheduler->OnDispatcherStarted( pulseGenerator );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerTask::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_scheduler )
        return true;

    Int64 delayMs = m_scheduler->DispatchCycle();
    if ( delayMs < 0 )
        return true;
    if ( delayMs < 1 )
        delayMs = 1;
    if ( delayMs > 0x7FFFFFFF )
        delayMs = 0x7FFFFFFF;
    RequestTaskCycleDelayInMs( (UInt32) delayMs );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerTask::OnTaskEnding( CTaskPtr task, bool willBeForced )
{GUCEF_TRACE;

    // the pulse generator is the delegator's, which may carry out other tasks after this one
    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->AllowPeriodicPulses();
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerTask::OnTaskEnded( CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_scheduler )
        m_scheduler->OnDispatcherEnded();
    CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

CString
CJobSchedulerTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
