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

#ifndef GUCEF_CORE_CTHREADPOOLCOUNTERS_H
#define GUCEF_CORE_CTHREADPOOLCOUNTERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Totals of one thread pool since it was created, read without walking tasks (CTaskManager::GetPoolCounters()).
 *  Rates are the deltas of two readings over the elapsed monotonic time.
 */
class GUCEF_CORE_PUBLIC_CPP CThreadPoolCounters
{
    public:

    CThreadPoolCounters( void );

    void SetThreadPoolName( const CString& threadPoolName );
    const CString& GetThreadPoolName( void ) const;

    void SetQueuedTotal( UInt64 queuedTotal );
    UInt64 GetQueuedTotal( void ) const;

    void SetStartedTotal( UInt64 startedTotal );
    UInt64 GetStartedTotal( void ) const;

    void SetFinishedTotal( UInt64 finishedTotal );
    UInt64 GetFinishedTotal( void ) const;

    void SetFailedTotal( UInt64 failedTotal );
    UInt64 GetFailedTotal( void ) const;

    void SetCancelledTotal( UInt64 cancelledTotal );
    UInt64 GetCancelledTotal( void ) const;

    void SetKilledTotal( UInt64 killedTotal );
    UInt64 GetKilledTotal( void ) const;

    void SetCumulativeQueueWaitMs( UInt64 cumulativeQueueWaitMs );
    UInt64 GetCumulativeQueueWaitMs( void ) const;

    void SetCumulativeRunMs( UInt64 cumulativeRunMs );
    UInt64 GetCumulativeRunMs( void ) const;

    /** Monotonic time at which the counters were read */
    void SetSampleMonotonicMs( UInt64 sampleMonotonicMs );
    UInt64 GetSampleMonotonicMs( void ) const;
    void Clear( void );

    private:

    CString m_threadPoolName;
    UInt64 m_queuedTotal;
    UInt64 m_startedTotal;
    UInt64 m_finishedTotal;
    UInt64 m_failedTotal;
    UInt64 m_cancelledTotal;
    UInt64 m_killedTotal;
    UInt64 m_cumulativeQueueWaitMs;
    UInt64 m_cumulativeRunMs;
    UInt64 m_sampleMonotonicMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTHREADPOOLCOUNTERS_H ? */
