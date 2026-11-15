/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefCOMPUTE_CCpuComputeFence.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCpuComputeFence::SWaiter::SWaiter( void )
    : wake( 0 )
    , value( 0 )
    , woken( false )
    , next( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuComputeFence::CCpuComputeFence( void )
    : m_lock()
    , m_completed( 0 )
    , m_failure( 0 )
    , m_waiting( GUCEF_NULL )
    , m_free( GUCEF_NULL )
    , m_all()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuComputeFence::~CCpuComputeFence()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_all.size(); ++i )
        GUCEF_DELETE m_all[ i ];
    m_all.clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CCpuComputeFence::GetCompletedValue( void ) const
{GUCEF_TRACE;

    return m_completed.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeFence::WakeWaitersLocked( void )
{GUCEF_TRACE;

    const UInt64 completed = m_completed.Load( MT::MEMORDER_ACQUIRE );
    const bool failed = 0 != m_failure.Load( MT::MEMORDER_ACQUIRE );
    for ( SWaiter* waiter = m_waiting; GUCEF_NULL != waiter; waiter = waiter->next )
    {
        if ( !waiter->woken && ( failed || waiter->value <= completed ) )
        {
            waiter->woken = true;
            waiter->wake.Post();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeFence::Unlink( SWaiter* waiter )
{GUCEF_TRACE;

    SWaiter** link = &m_waiting;
    while ( GUCEF_NULL != *link && *link != waiter )
        link = &( (*link)->next );
    if ( GUCEF_NULL != *link )
        *link = waiter->next;

    // back to the free list without a pending wake-up
    while ( 0 == waiter->wake.TryWait() ) {}
    waiter->woken = false;
    waiter->next = m_free;
    m_free = waiter;
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeFence::Signal( UInt64 value )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( value > m_completed.Load( MT::MEMORDER_ACQUIRE ) )
        m_completed.Store( value, MT::MEMORDER_RELEASE );
    WakeWaitersLocked();
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeFence::Fail( TComputeResult result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    UInt32 expected = 0;
    m_failure.CompareExchangeStrong( expected, static_cast< UInt32 >( result ) + 1, MT::MEMORDER_ACQ_REL, MT::MEMORDER_ACQUIRE );
    WakeWaitersLocked();
}

/*-------------------------------------------------------------------------*/

bool
CCpuComputeFence::IsFailed( void ) const
{GUCEF_TRACE;

    return 0 != m_failure.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeFence::Wait( UInt64 value, UInt32 timeoutInMs )
{GUCEF_TRACE;

    SWaiter* waiter = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( m_completed.Load( MT::MEMORDER_ACQUIRE ) >= value )
            return COMPUTERESULT_SUCCESS;
        const UInt32 failure = m_failure.Load( MT::MEMORDER_ACQUIRE );
        if ( 0 != failure )
            return static_cast< TComputeResult >( failure - 1 );
        if ( 0 == timeoutInMs )
            return COMPUTERESULT_TIMEOUT;

        if ( GUCEF_NULL != m_free )
        {
            waiter = m_free;
            m_free = waiter->next;
        }
        else
        {
            waiter = GUCEF_NEW SWaiter();
            m_all.push_back( waiter );
        }
        waiter->value = value;
        waiter->woken = false;
        waiter->next = m_waiting;
        m_waiting = waiter;
    }

    const UInt64 startTicks = MT::PrecisionTickCount();
    for ( ;; )
    {
        UInt32 sliceInMs = GUCEF_COMPUTE_INFINITE_TIMEOUT;
        if ( GUCEF_COMPUTE_INFINITE_TIMEOUT != timeoutInMs )
        {
            const Float64 elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
            sliceInMs = elapsedMs >= static_cast< Float64 >( timeoutInMs ) ? 0 : static_cast< UInt32 >( static_cast< Float64 >( timeoutInMs ) - elapsedMs ) + 1;
        }
        const Int32 waited = 0 == sliceInMs ? 1 : waiter->wake.WaitOrTimeout( sliceInMs );

        MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        TComputeResult result = COMPUTERESULT_TIMEOUT;
        bool done = false;
        const UInt32 failure = m_failure.Load( MT::MEMORDER_ACQUIRE );
        if ( m_completed.Load( MT::MEMORDER_ACQUIRE ) >= value )
        {
            result = COMPUTERESULT_SUCCESS;
            done = true;
        }
        else
        if ( 0 != failure )
        {
            result = static_cast< TComputeResult >( failure - 1 );
            done = true;
        }
        else
        if ( waited < 0 )
        {
            result = COMPUTERESULT_BACKEND_ERROR;
            done = true;
        }
        else
        if ( 0 != waited && GUCEF_COMPUTE_INFINITE_TIMEOUT != timeoutInMs &&
             MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) >= static_cast< Float64 >( timeoutInMs ) )
        {
            done = true;
        }
        if ( done )
        {
            Unlink( waiter );
            return result;
        }
        // woken for nothing (cannot happen with a value that is not reached) or not yet timed out: wait on
        if ( waiter->woken )
        {
            while ( 0 == waiter->wake.TryWait() ) {}
            waiter->woken = false;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
