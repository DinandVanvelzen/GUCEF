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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefCOMCORE_CRpcProtocolTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CRpcProtocolTask::TaskType = "GUCEF::COMCORE::CRpcProtocolTask";

/*-------------------------------------------------------------------------*/

CRpcProtocolTask::CRpcProtocolTask( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcProtocolTask::~CRpcProtocolTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcProtocolTaskPtr
CRpcProtocolTask::Start( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolName );
    if ( pool.IsNULL() )
        return CRpcProtocolTaskPtr();
    CRpcProtocolTaskPtr task( GUCEF_NEW CRpcProtocolTask() );
    CORE::CFutureResult setup = pool->SetupSingularTask( task );
    if ( setup.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcProtocolTask: pool \"" << threadPoolName << "\" refused the protocol task" );
        return CRpcProtocolTaskPtr();
    }
    CORE::CFutureResult started = pool->StartTaskWithConsumer( task );
    if ( started.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcProtocolTask: pool \"" << threadPoolName << "\" did not start the protocol task" );
        return CRpcProtocolTaskPtr();
    }
    return task;
}

/*-------------------------------------------------------------------------*/

void
CRpcProtocolTask::Stop( void )
{GUCEF_TRACE;

    // the identity of the thread that pulses, not GetDelegatorThreadId(): that one is the operating system's id of the thread
    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    bool ownThread = !pulseGenerator.IsNULL() && pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentThreadID();
    RequestTaskToStop( !ownThread );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcProtocolTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CRpcProtocolTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // a protocol thread is persistent: the work happens in the pulses of the delegator's pulse generator
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcProtocolTask::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcProtocolTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcProtocolTask";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
