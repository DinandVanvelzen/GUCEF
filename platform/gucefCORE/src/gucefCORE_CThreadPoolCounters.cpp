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

#include "gucefCORE_CThreadPoolCounters.h"

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

CThreadPoolCounters::CThreadPoolCounters( void )
    : m_threadPoolName()
    , m_queuedTotal( 0 )
    , m_startedTotal( 0 )
    , m_finishedTotal( 0 )
    , m_failedTotal( 0 )
    , m_cancelledTotal( 0 )
    , m_killedTotal( 0 )
    , m_cumulativeQueueWaitMs( 0 )
    , m_cumulativeRunMs( 0 )
    , m_sampleMonotonicMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::Clear( void )
{GUCEF_TRACE;

    m_threadPoolName = CString();
    m_queuedTotal = 0;
    m_startedTotal = 0;
    m_finishedTotal = 0;
    m_failedTotal = 0;
    m_cancelledTotal = 0;
    m_killedTotal = 0;
    m_cumulativeQueueWaitMs = 0;
    m_cumulativeRunMs = 0;
    m_sampleMonotonicMs = 0;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    m_threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CThreadPoolCounters::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetQueuedTotal( UInt64 queuedTotal )
{GUCEF_TRACE;

    m_queuedTotal = queuedTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetQueuedTotal( void ) const
{GUCEF_TRACE;

    return m_queuedTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetStartedTotal( UInt64 startedTotal )
{GUCEF_TRACE;

    m_startedTotal = startedTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetStartedTotal( void ) const
{GUCEF_TRACE;

    return m_startedTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetFinishedTotal( UInt64 finishedTotal )
{GUCEF_TRACE;

    m_finishedTotal = finishedTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetFinishedTotal( void ) const
{GUCEF_TRACE;

    return m_finishedTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetFailedTotal( UInt64 failedTotal )
{GUCEF_TRACE;

    m_failedTotal = failedTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetFailedTotal( void ) const
{GUCEF_TRACE;

    return m_failedTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetCancelledTotal( UInt64 cancelledTotal )
{GUCEF_TRACE;

    m_cancelledTotal = cancelledTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetCancelledTotal( void ) const
{GUCEF_TRACE;

    return m_cancelledTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetKilledTotal( UInt64 killedTotal )
{GUCEF_TRACE;

    m_killedTotal = killedTotal;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetKilledTotal( void ) const
{GUCEF_TRACE;

    return m_killedTotal;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetCumulativeQueueWaitMs( UInt64 cumulativeQueueWaitMs )
{GUCEF_TRACE;

    m_cumulativeQueueWaitMs = cumulativeQueueWaitMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetCumulativeQueueWaitMs( void ) const
{GUCEF_TRACE;

    return m_cumulativeQueueWaitMs;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetCumulativeRunMs( UInt64 cumulativeRunMs )
{GUCEF_TRACE;

    m_cumulativeRunMs = cumulativeRunMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetCumulativeRunMs( void ) const
{GUCEF_TRACE;

    return m_cumulativeRunMs;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolCounters::SetSampleMonotonicMs( UInt64 sampleMonotonicMs )
{GUCEF_TRACE;

    m_sampleMonotonicMs = sampleMonotonicMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadPoolCounters::GetSampleMonotonicMs( void ) const
{GUCEF_TRACE;

    return m_sampleMonotonicMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
