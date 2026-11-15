/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COM_CMCPJOBBRIDGE_H
#include "gucefCOM_CMcpJobBridge.h"
#define GUCEF_COM_CMCPJOBBRIDGE_H
#endif /* GUCEF_COM_CMCPJOBBRIDGE_H ? */

#include "gucefCOM_CMcpJobWaiter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpJobWaiter::CMcpJobWaiter( CMcpJobBridge* bridge                        ,
                              const CMcpRequestContextPtr& context          ,
                              const CORE::CString& jobId                    ,
                              UInt64 deadlineMonotonicMs                    ,
                              const CIMcpJobResultBuilderPtr& builder       ,
                              const CORE::PulseGeneratorPtr& pulseGenerator ,
                              UInt32 pollIntervalMs                         )
    : CORE::CObserver()
    , lastProgressDone( 0 )
    , m_lock()
    , m_bridge( bridge )
    , m_context( context )
    , m_jobId( jobId )
    , m_deadlineMonotonicMs( deadlineMonotonicMs )
    , m_builder( builder )
    , m_timer( pulseGenerator, pollIntervalMs )
    , m_done( 0 )
{GUCEF_TRACE;

    TEventCallback callback( this, &CMcpJobWaiter::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, callback );
}

/*-------------------------------------------------------------------------*/

CMcpJobWaiter::~CMcpJobWaiter()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobWaiter::Start( void )
{GUCEF_TRACE;

    return m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobWaiter::IsDone( void ) const
{GUCEF_TRACE;

    return 0 != m_done.Load();
}

/*-------------------------------------------------------------------------*/

void
CMcpJobWaiter::Abandon( const CMcpToolResult* result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_bridge = GUCEF_NULL;
    m_timer.SetEnabled( false );
    if ( 0 == m_done.Load() )
    {
        m_done.Store( 1 );
        if ( GUCEF_NULL != result && !m_context.IsNULL() && !m_context->IsCompleted() )
            m_context->CompleteWithToolResult( *result );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpJobWaiter::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL == m_bridge || 0 != m_done.Load() )
        return;
    if ( m_bridge->OnWaiterTick( *this ) )
    {
        m_done.Store( 1 );
        m_timer.SetEnabled( false );
    }
}

/*-------------------------------------------------------------------------*/

const CMcpRequestContextPtr& CMcpJobWaiter::GetContext( void ) const { return m_context; }
const CORE::CString& CMcpJobWaiter::GetJobId( void ) const { return m_jobId; }
UInt64 CMcpJobWaiter::GetDeadlineMonotonicMs( void ) const { return m_deadlineMonotonicMs; }
const CIMcpJobResultBuilderPtr& CMcpJobWaiter::GetBuilder( void ) const { return m_builder; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpJobWaiter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpJobWaiter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
