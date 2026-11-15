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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H
#define GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The fence primitive of a CPU queue: the timeline value of the last completed submission, which waiting threads
 *  block on until it reaches their value. The CPU queue signals it after it executed a submission, its copies to host
 *  memory included. When the device is lost, or the queue closed, every waiter wakes and gets the result it failed
 *  with (COMPUTERESULT_DEVICE_LOST, COMPUTERESULT_BACKEND_ERROR) for a value that was not reached.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeFence
{
    public:

    CCpuComputeFence( void );

    ~CCpuComputeFence();

    UInt64 GetCompletedValue( void ) const;

    /** Advances the timeline to 'value' (never back) and wakes the waiters */
    void Signal( UInt64 value );

    /** Waits until the timeline reaches 'value'. COMPUTERESULT_SUCCESS, _TIMEOUT or the result of Fail() */
    TComputeResult Wait( UInt64 value, UInt32 timeoutInMs );

    /** Wakes every waiter for good: values not reached fail with 'result' (the first failure counts) */
    void Fail( TComputeResult result );

    bool IsFailed( void ) const;

    private:

    /** A thread that waits: its own wake-up, so a wake-up meant for one waiter is never taken by another */
    struct SWaiter
    {
        MT::CSemaphore wake;
        UInt64 value;
        bool woken;
        SWaiter* next;

        SWaiter( void );
    };

    /** Wakes the waiters whose value is reached, all of them after a failure. Called with m_lock held */
    void WakeWaitersLocked( void );
    void Unlink( SWaiter* waiter );

    CCpuComputeFence( const CCpuComputeFence& src );            /**< not implemented */
    CCpuComputeFence& operator=( const CCpuComputeFence& src ); /**< not implemented */

    private:

    MT::CMutex m_lock;
    MT::CTAtomic< UInt64 > m_completed;
    MT::CTAtomic< UInt32 > m_failure;      /**< 0 or the TComputeResult + 1 that waiters get */
    SWaiter* m_waiting;                     /**< guarded by m_lock */
    SWaiter* m_free;                        /**< guarded by m_lock: waiter records are reused, waits do not allocate once warm */
    GUCEF::vector< SWaiter* > m_all;        /**< guarded by m_lock */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H ? */
