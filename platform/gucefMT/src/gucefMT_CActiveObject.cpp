/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <assert.h>
#include <exception>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"         /* OS wrappers for threading functionality */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#include "gucefMT_CActiveObject.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CActiveObject::CActiveObject( void )
    : MT::CILockable()
    , m_threadData( NULL )                    
    , m_delayInMilliSecs( 25 )             
    , m_isDeactivationRequested( false )   
    , _suspend( false )                    
    , _active( false )                     
    , _td( ThreadDataReserve() )                          
    , m_minimalCycleDeltaInMilliSecs( 10 )
    , m_datalock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CActiveObject::CActiveObject( const CActiveObject& src )
    : MT::CILockable()
    , m_threadData( src.m_threadData )         
    , m_delayInMilliSecs( src.m_delayInMilliSecs )       
    , m_isDeactivationRequested( false ) 
    , _suspend( false )                  
    , _active( false )                      
    , _td( ThreadDataReserve() )                           
    , m_minimalCycleDeltaInMilliSecs( 10 )
    , m_datalock()
{GUCEF_TRACE;

    if ( src.IsActive() )
    {
        Activate();
    }
}

/*-------------------------------------------------------------------------*/

CActiveObject::~CActiveObject()
{GUCEF_TRACE;

    Deactivate( true, true );
    
    ThreadDataCleanup( _td );
    _td = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::IsDeactivationRequested( void ) const
{GUCEF_TRACE;

    return m_isDeactivationRequested;
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::IsActive( void ) const
{GUCEF_TRACE;

    return _active;
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::CallOnThreadStartWithTimeout( Int32 maxRetries, void* taskData )
{GUCEF_TRACE;

    bool success = false;
    Int32 nrOfAttempts = 1;
    do
    {
        try
        {
            OnThreadStart( taskData );
            success = true;
        }
        catch ( const timeout_exception& )
        {
            success = false;
            if ( maxRetries <= nrOfAttempts )
                return false;
        }
    }
    while ( !success && ( maxRetries < 1 || maxRetries <= nrOfAttempts ) );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::CallOnThreadStartedWithTimeout( Int32 maxRetries, void* taskData )
{GUCEF_TRACE;

    bool success = false;
    Int32 nrOfAttempts = 1;
    do
    {
        try
        {
            OnThreadStarted( taskData );
            success = true;
        }
        catch ( const timeout_exception& )
        {
            success = false;
            if ( maxRetries <= nrOfAttempts )
                return false;
        }
    }
    while ( !success && ( maxRetries < 1 || maxRetries <= nrOfAttempts ) );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::CallOnThreadEndingWithTimeout( Int32 maxRetries, void* taskData, bool willBeForced )
{GUCEF_TRACE;

    bool success = false;
    Int32 nrOfAttempts = 1;
    do
    {
        try
        {
            OnThreadEnding( taskData, willBeForced );
            success = true;
        }
        catch ( const timeout_exception& )
        {
            success = false;
            if ( maxRetries <= nrOfAttempts )
                return false;
        }
    }
    while ( !success && ( maxRetries < 1 || maxRetries <= nrOfAttempts ) );
    return success;
}

/*-------------------------------------------------------------------------*/

Int32
CActiveObject::OnActivate( void* thisObject )
{GUCEF_TRACE;

    CActiveObject* tao = (CActiveObject*) thisObject;
    try
    {
        void* taskData = tao->GetThreadData();
        Int32 maxRetries = 5;

        if ( tao->CallOnThreadStartWithTimeout( maxRetries, taskData ) )
        {
            if ( tao->CallOnThreadStartedWithTimeout( maxRetries, taskData ) )
            {       
                Float64 ticksPerMs = PrecisionTimerResolution() / 1000.0;
                UInt64 tickCount = PrecisionTickCount();
                UInt64 newTickCount = tickCount;
                Float64 timeDeltaInMs = 0;

                bool taskIsFinished = false;
                while ( !taskIsFinished && !tao->m_isDeactivationRequested )
                {
                    // Check if the order has been given to suspend the thread
                    if ( tao->_suspend )
                    {
                        ThreadSuspend( tao->_td );
                    }

                    // We can do a cycle
                    try
                    {
                        taskIsFinished = tao->OnThreadCycle( taskData );
                    }
                    catch ( const timeout_exception& )
                    {
                        // if we timed out whatever async thing we were trying to do, 
                        // just try again next round, no max nr of attempts
                    }

                    // check if we are finished
                    if ( !taskIsFinished )
                    {
                        // If we are going to do another cycle then make sure we
                        // stay within the time slice range requested.
                        // Here we calculate the time that has passed in seconds
                        newTickCount = PrecisionTickCount();
                        timeDeltaInMs = ( newTickCount - tickCount ) / ticksPerMs;

                        if ( timeDeltaInMs < tao->m_minimalCycleDeltaInMilliSecs )
                        {
                            PrecisionDelay( tao->m_delayInMilliSecs );
                            tickCount = PrecisionTickCount();
                        }
                        else
                        {
                            tickCount = newTickCount;
                        }
                    }
                }
            }
        }

        tao->CallOnThreadEndingWithTimeout( maxRetries, taskData, false );    
        tao->_active = false;
        return 1;
    }
    catch ( const std::exception& )
    {
        tao->_active = false;
        GUCEF_ASSERT_ALWAYS; // we should not come here: fix your code
        return -1;
    }
    catch ( ... )
    {
        tao->_active = false;
        GUCEF_ASSERT_ALWAYS; // we should not come here: fix your code
        return GUCEF_INT32MIN;
    }
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::Activate( void* threadData /* = NULL */                 ,
                         const UInt32 cycleDelay /* = 25 */            ,
                         const UInt32 minimalCycleDeltaInMs /* = 10 */ )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );    

    if ( _active ) 
        return false;

    m_threadData = threadData;
    m_delayInMilliSecs = cycleDelay;
    m_minimalCycleDeltaInMilliSecs = minimalCycleDeltaInMs;
    m_isDeactivationRequested = false;
    _suspend = false;
    _active = true;
    
    bool createSuccesss = 0 != ThreadCreate( _td                      ,
                                             (TThreadFunc) OnActivate ,
                                             this                     );
    return createSuccesss;
}

/*-------------------------------------------------------------------------*/

void
CActiveObject::OnThreadPausedForcibly( void* taskdata )
{GUCEF_TRACE;

    // Dummy to avoid mandatory implementation in decending classes
}

/*-------------------------------------------------------------------------*/

void
CActiveObject::OnThreadResumed( void* taskdata )
{GUCEF_TRACE;

    // Dummy to avoid mandatory implementation in decending classes
}

/*-------------------------------------------------------------------------*/
    
void
CActiveObject::OnThreadEnded( void* taskdata ,
                              bool forced    )
{GUCEF_TRACE;

    // Dummy to avoid mandatory implementation in decending classes
}

/*-------------------------------------------------------------------------*/
    
void
CActiveObject::OnThreadEnding( void* taskdata    ,
                               bool willBeForced )
{GUCEF_TRACE;

    // Dummy to avoid mandatory implementation in decending classes
}

/*-------------------------------------------------------------------------*/
    
void
CActiveObject::OnThreadStarted( void* taskdata )
{GUCEF_TRACE;

    // Dummy to avoid mandatory implementation in decending classes
}

/*-------------------------------------------------------------------------*/

void
CActiveObject::Deactivate( bool force, bool callerShouldWait )
{GUCEF_TRACE;

    if ( force )
    {
        Lock();
        if ( _active )
        {
            // Set the flag to allow the thread to exit the run cycle gracefully
            m_isDeactivationRequested = true;            
            Unlock();

            // Lets give the thread up to 5 seconds to shut down
            // If it refuses we kill it
            UInt32 waitResult = ThreadWait( _td, 5000 );

            Lock();
            if ( _active )
            {
                ThreadSuspend( _td );
                OnThreadEnding( m_threadData, true );
            
                /*
                 *      Kill the thread :(
                 */
                ThreadKill( _td );
            }
            OnThreadEnded( m_threadData, true );
            Unlock();
        }
        else
        {
            Unlock();
        }
    }
    else
    {
        CObjectScopeLock lock( this );
        m_isDeactivationRequested = true;
        if ( _active && callerShouldWait )
        {
            OnThreadEnding( m_threadData, false );

            lock.EarlyUnlock();
            UInt32 waitResult = ThreadWait( _td, 15000 );
            OnThreadEnded( m_threadData, false );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CActiveObject::IsPauseRequested( void ) const
{GUCEF_TRACE;

    return _suspend;
}

/*-------------------------------------------------------------------------*/

void
CActiveObject::Pause( bool force )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );
    
    if ( force )
    {
        // Don't wait for the thread to cycle
        ThreadSuspend( _td );
        _active = false;
        OnThreadPausedForcibly( m_threadData );
    }
    else
    {
        _suspend = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CActiveObject::Resume( void )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );
    if ( GUCEF_NULL != _td && _suspend )
    {
        _active = true;
        ThreadResume( _td );
        OnThreadResumed( m_threadData );
    }
}

/*-------------------------------------------------------------------------*/

void*
CActiveObject::GetThreadData( void ) const
{GUCEF_TRACE;

    return m_threadData;
}

/*-------------------------------------------------------------------------*/

CActiveObject&
CActiveObject::operator=( const CActiveObject& src )
{GUCEF_TRACE;

    if ( this != &src )  
    {
        CObjectScopeLock lock( this );

        Deactivate( true, true );

        m_threadData = src.m_threadData;
        m_delayInMilliSecs = src.m_delayInMilliSecs;
        _suspend = false;

        if ( src.IsActive() )
        {
            Activate();
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CActiveObject::GetThreadID( void ) const
{GUCEF_TRACE;

    CObjectScopeReadOnlyLock lock( this );

    if ( GUCEF_NULL != _td )
    {
        return ThreadID( _td );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool 
CActiveObject::SetCpuAffinityMask( UInt32 affinityMaskSize ,
                                   void* affinityMask      )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );

    if ( GUCEF_NULL != _td )
    {
        return ThreadSetCpuAffinity( _td, affinityMaskSize, affinityMask ) != 0;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CActiveObject::GetCpuAffinityMask( UInt32 affinityMaskBufferSize ,
                                   void* affinityMask            ,
                                   UInt32& affinityMaskSize      ) const
{GUCEF_TRACE;

    CObjectScopeReadOnlyLock lock( this );

    if ( GUCEF_NULL != _td )
    {
        return ThreadGetCpuAffinity( _td, affinityMaskBufferSize, affinityMask, &affinityMaskSize ) != 0;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CActiveObject::SetCpuAffinityByCpuId( UInt32 cpuId )
{GUCEF_TRACE;

    CObjectScopeReadOnlyLock lock( this );
    return 0 != ThreadSetCpuAffinityByCpuId( _td, cpuId );
}

/*-------------------------------------------------------------------------*/

bool 
CActiveObject::GetCpuAffinityByCpuId( UInt32& cpuId ) const
{GUCEF_TRACE;

    CObjectScopeReadOnlyLock lock( this );
    return 0 != ThreadGetCpuAffinityByCpuId( _td, &cpuId );
}

/*-------------------------------------------------------------------------*/

TLockStatus
CActiveObject::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_datalock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

TLockStatus
CActiveObject::Unlock( void ) const
{GUCEF_TRACE;

    return m_datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

UInt32 
CActiveObject::WaitForThreadToFinish( Int32 timeoutInMs ) const
{GUCEF_TRACE;

    return ThreadWait( _td, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

TThreadStatus 
CActiveObject::GetThreadStatus( void ) const
{GUCEF_TRACE;

    return ThreadStatus( _td );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
