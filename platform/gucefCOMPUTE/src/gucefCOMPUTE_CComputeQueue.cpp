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

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeQueue.h"

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

namespace {

CORE::CFutureResult
FailedFuture( TComputeResult result )
{
    CORE::CFutureResultSource source;
    source.Fail( CORE::TASKSTATUS_STARTUP_FAILED, ComputeResultToFutureInfo( result, CString::Empty ) );
    return source.GetFuture();
}

}

/*-------------------------------------------------------------------------*/

CComputeQueue::CComputeQueue( CComputeDevice* device, UInt32 queueIndex, TComputeExecutionPolicy policy, bool cooperative )
    : CORE::CTSharedObjCreator< CComputeQueue, MT::CMutex >( this )
    , m_device( device )
    , m_queueIndex( queueIndex )
    , m_submitLock()
    , m_lastSubmittedValue( 0 )
    , m_policy( static_cast< UInt32 >( policy ) )
    , m_completion()
    , m_cooperative( cooperative ? 1 : 0 )
    , m_closedResult( 0 )
    , m_cooperativeLock()
    , m_cooperativeWork()
    , m_cooperativeHead( 0 )
    , m_hostTimelineLock()
    , m_hostCompleted( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeQueue::~CComputeQueue()
{GUCEF_TRACE;

    // The derived queue and the device shut the queue down already (ShutdownQueue()). The derived part is gone here,
    // so nothing virtual is called: a task that still runs is stopped, what is left fails
    TComputeCompletionTaskConsumerPtr completion = m_completion;
    m_completion.Unlink();
    if ( !completion.IsNULL() )
        completion->Stop( COMPUTERESULT_BACKEND_ERROR );
    TComputePendingWorkVector left;
    for ( size_t i=m_cooperativeHead; i<m_cooperativeWork.size(); ++i )
    {
        m_cooperativeWork[ i ]->Finish( COMPUTERESULT_BACKEND_ERROR, "the queue was destroyed" );
        left.push_back( m_cooperativeWork[ i ] );
    }
    m_cooperativeWork.clear();
    DeleteFinished( left );
}

/*-------------------------------------------------------------------------*/

CComputeDevice*
CComputeQueue::GetDevice( void ) const
{GUCEF_TRACE;

    return m_device.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeQueue::GetQueueIndex( void ) const
{GUCEF_TRACE;

    return m_queueIndex;
}

/*-------------------------------------------------------------------------*/

bool
CComputeQueue::IsLost( void ) const
{GUCEF_TRACE;

    CComputeDevice* device = m_device.Load( MT::MEMORDER_ACQUIRE );
    return GUCEF_NULL == device || device->IsLost();
}

/*-------------------------------------------------------------------------*/

bool
CComputeQueue::IsClosed( void ) const
{GUCEF_TRACE;

    return 0 != m_closedResult.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CComputeQueue::IsCooperative( void ) const
{GUCEF_TRACE;

    return 0 != m_cooperative.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::SetExecutionPolicy( TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    m_policy.Store( static_cast< UInt32 >( policy ), MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

TComputeExecutionPolicy
CComputeQueue::GetExecutionPolicy( void ) const
{GUCEF_TRACE;

    return static_cast< TComputeExecutionPolicy >( m_policy.Load( MT::MEMORDER_ACQUIRE ) );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::Validate( const CComputeCommandList& list ) const
{GUCEF_TRACE;

    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;
    const UInt32 closed = m_closedResult.Load( MT::MEMORDER_ACQUIRE );
    if ( 0 != closed )
        return static_cast< TComputeResult >( closed - 1 );
    if ( COMPUTERESULT_SUCCESS != list.GetRecordingResult() )
        return list.GetRecordingResult();
    if ( !list.IsEmpty() && list.GetDevice() != GetDevice() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeQueue: Refusing a command list recorded for another device" );
        return COMPUTERESULT_INVALID_ARGUMENT;
    }
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::ReserveBufferUses( const CComputeCommandList& list, UInt64 fenceValue, GUCEF::vector< UInt64 >& previous )
{GUCEF_TRACE;

    // called with the submit lock held: the use of this queue of every buffer, refused for a mapped buffer
    previous.clear();
    for ( UInt32 i=0; i<list.GetBufferCount(); ++i )
    {
        UInt64 before = 0;
        if ( !list.GetBuffer( i )->TryRecordUse( this, fenceValue, before ) )
        {
            UndoBufferUses( list, previous );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeQueue: Refusing a command list that uses a mapped buffer" );
            return COMPUTERESULT_INVALID_ARGUMENT;
        }
        previous.push_back( before );
    }
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::UndoBufferUses( const CComputeCommandList& list, const GUCEF::vector< UInt64 >& previous )
{GUCEF_TRACE;

    for ( size_t i=0; i<previous.size(); ++i )
        list.GetBuffer( static_cast< UInt32 >( i ) )->RestoreUse( this, previous[ i ] );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::SubmitLocked( const CComputeCommandList& list, UInt64& fenceValue )
{GUCEF_TRACE;

    // called with the submit lock held. The value is taken only when the submission succeeded: no value is ever skipped
    TComputeResult result = Validate( list );
    if ( COMPUTERESULT_SUCCESS != result )
        return result;

    const UInt64 value = m_lastSubmittedValue + 1;
    GUCEF::vector< UInt64 > previous;
    result = ReserveBufferUses( list, value, previous );
    if ( COMPUTERESULT_SUCCESS != result )
        return result;
    result = OnSubmit( list, value );
    if ( COMPUTERESULT_SUCCESS != result )
    {
        UndoBufferUses( list, previous );
        return result;
    }
    m_lastSubmittedValue = value;
    fenceValue = value;
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

bool
CComputeQueue::EnsureCompletionTask( void )
{GUCEF_TRACE;

    // called with the submit lock held
    if ( IsCooperative() )
        return false;
    if ( !m_completion.IsNULL() )
        return true;

    TComputeCompletionTaskConsumerPtr completion( GUCEF_NEW CComputeCompletionTaskConsumer( this ) );
    if ( !completion->Start( completion ) )
    {
        // platforms without threads: the queue completes its work cooperatively
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeQueue: No completion task, pending work completes through CComputeGlobal::Update()" );
        m_cooperative.Store( 1, MT::MEMORDER_RELEASE );
        return false;
    }
    m_completion = completion;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::HandOver( CComputePendingWork* work )
{GUCEF_TRACE;

    // called with the submit lock held, so the pending work is in the order of its values
    if ( EnsureCompletionTask() )
    {
        m_completion->Enqueue( work );
        return;
    }
    MT::CScopeMutex lock( m_cooperativeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    m_cooperativeWork.push_back( work );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::Submit( const CComputeCommandList& list, CComputeFence& fence )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    UInt64 value = 0;
    const TComputeResult result = SubmitLocked( list, value );
    if ( COMPUTERESULT_SUCCESS != result )
        return result;
    fence = CComputeFence( CreateSharedPtr(), value );
    HandOver( GUCEF_NEW CComputePendingWork( value, list ) );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::Execute( const CComputeCommandList& list )
{GUCEF_TRACE;

    return Execute( list, GetExecutionPolicy() );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::Execute( const CComputeCommandList& list, TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    UInt64 value = 0;
    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        const TComputeResult result = SubmitLocked( list, value );
        if ( COMPUTERESULT_SUCCESS != result )
            return result;
    }
    // the wait (and on the CPU backend the execution) happens without the queue lock: other threads submit meanwhile
    return OnExecute( list, value, policy );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CComputeQueue::ExecuteAsync( const CComputeCommandList& list )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    UInt64 value = 0;
    const TComputeResult result = SubmitLocked( list, value );
    if ( COMPUTERESULT_SUCCESS != result )
        return FailedFuture( result );

    CORE::CFutureResultSource source;
    HandOver( GUCEF_NEW CComputePendingWork( value, list, source ) );
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

CComputeFence
CComputeQueue::GetLastSubmittedFence( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return CComputeFence( CreateSharedPtr(), m_lastSubmittedValue );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::WaitIdle( UInt32 timeoutInMs )
{GUCEF_TRACE;

    UInt64 value = 0;
    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        value = m_lastSubmittedValue;
    }
    if ( 0 == value )
        return COMPUTERESULT_SUCCESS;
    return WaitForValue( value, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeQueue::GetPendingWorkCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( !m_completion.IsNULL() )
            count += m_completion->GetPendingCount();
    }
    MT::CScopeMutex lock( m_cooperativeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    count += static_cast< UInt32 >( m_cooperativeWork.size() ) - m_cooperativeHead;
    return count;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::OnExecute( const CComputeCommandList& list, UInt64 fenceValue, TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    return WaitForValue( fenceValue, GUCEF_COMPUTE_INFINITE_TIMEOUT );
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::ProcessPendingWork( CComputePendingWork& work, UInt32 timeoutInMs, CString& errorText )
{GUCEF_TRACE;

    return WaitForValue( work.GetFenceValue(), timeoutInMs );
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::OnDeviceLost( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::OnQueueClosed( TComputeResult reason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::OnFinishHostTransfers( UInt64 fenceValue, CString& errorText )
{GUCEF_TRACE;

    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeQueue::AdvanceHostTimeline( UInt64 deviceCompletedValue )
{GUCEF_TRACE;

    if ( m_hostCompleted.Load( MT::MEMORDER_ACQUIRE ) >= deviceCompletedValue )
        return COMPUTERESULT_SUCCESS;

    MT::CScopeMutex lock( m_hostTimelineLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    TComputeResult result = COMPUTERESULT_SUCCESS;
    UInt64 current = m_hostCompleted.Load( MT::MEMORDER_ACQUIRE );
    while ( current < deviceCompletedValue )
    {
        CString errorText;
        const TComputeResult transfers = OnFinishHostTransfers( current + 1, errorText );
        if ( COMPUTERESULT_SUCCESS != transfers )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeQueue: The host transfers of submission " << ( current + 1 ) << " failed: " <<
                ComputeResultToString( transfers ) << " " << errorText );
            if ( COMPUTERESULT_SUCCESS == result )
                result = transfers;
        }
        ++current;
        m_hostCompleted.Store( current, MT::MEMORDER_RELEASE );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeQueue::GetHostTimelineValue( void ) const
{GUCEF_TRACE;

    return m_hostCompleted.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::DrainCooperativeWork( UInt64 upToValue, UInt32 timeoutInMs, TComputePendingWorkVector& finished )
{GUCEF_TRACE;

    // the lock is held while processing: drains from several threads stay in order
    MT::CScopeMutex lock( m_cooperativeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    while ( m_cooperativeHead < m_cooperativeWork.size() )
    {
        CComputePendingWork* work = m_cooperativeWork[ m_cooperativeHead ];
        if ( work->GetFenceValue() > upToValue )
            break;

        CString errorText;
        TComputeResult result = COMPUTERESULT_BACKEND_ERROR;
        try
        {
            result = ProcessPendingWork( *work, timeoutInMs, errorText );
        }
        catch ( ... )
        {
            errorText = "an exception was raised while the work ran";
        }
        if ( COMPUTERESULT_TIMEOUT == result )
            break;

        m_cooperativeWork[ m_cooperativeHead ] = GUCEF_NULL;
        ++m_cooperativeHead;
        work->Finish( result, errorText );
        finished.push_back( work );
    }
    if ( m_cooperativeHead == m_cooperativeWork.size() )
    {
        m_cooperativeWork.clear();
        m_cooperativeHead = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::DeleteFinished( TComputePendingWorkVector& finished )
{GUCEF_TRACE;

    for ( size_t i=0; i<finished.size(); ++i )
        GUCEF_DELETE finished[ i ];
    finished.clear();
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::NotifyDeviceLost( void )
{GUCEF_TRACE;

    OnDeviceLost();
    TComputeCompletionTaskConsumerPtr completion;
    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        completion = m_completion;
    }
    if ( !completion.IsNULL() )
        completion->Wake();
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::CloseFromCompletionTask( TComputeResult reason )
{GUCEF_TRACE;

    {
        // under the submit lock: what was submitted before is with the task and fails there, what comes after is refused
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        UInt32 expected = 0;
        if ( !m_closedResult.CompareExchangeStrong( expected, static_cast< UInt32 >( reason ) + 1, MT::MEMORDER_ACQ_REL, MT::MEMORDER_ACQUIRE ) )
            return;
    }
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeQueue: The completion task of queue " << m_queueIndex << " was stopped, the queue is closed" );
    OnQueueClosed( reason );
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::ShutdownQueue( void )
{GUCEF_TRACE;

    TComputeCompletionTaskConsumerPtr completion;
    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        completion = m_completion;
    }

    // On the completion task itself (finishing a submission released the last reference to the device) waiting for
    // the work would wait for this thread: what is left fails instead
    const bool onCompletionTask = !completion.IsNULL() && completion->IsCalledFromTask();
    if ( !IsLost() && !IsClosed() && !onCompletionTask && !IsCooperative() )
        WaitIdle( GUCEF_COMPUTE_INFINITE_TIMEOUT );

    {
        MT::CScopeMutex lock( m_submitLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        m_completion.Unlink();
    }
    if ( !completion.IsNULL() )
        completion->Stop( IsLost() ? COMPUTERESULT_DEVICE_LOST : COMPUTERESULT_BACKEND_ERROR );

    // cooperative work that nobody processed fails
    TComputePendingWorkVector left;
    {
        MT::CScopeMutex lock( m_cooperativeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        for ( size_t i=m_cooperativeHead; i<m_cooperativeWork.size(); ++i )
            left.push_back( m_cooperativeWork[ i ] );
        m_cooperativeWork.clear();
        m_cooperativeHead = 0;
    }
    for ( size_t i=0; i<left.size(); ++i )
        left[ i ]->Finish( IsLost() ? COMPUTERESULT_DEVICE_LOST : COMPUTERESULT_BACKEND_ERROR, "the queue shut down before the work was processed" );
    DeleteFinished( left );
}

/*-------------------------------------------------------------------------*/

void
CComputeQueue::Detach( void )
{GUCEF_TRACE;

    ShutdownQueue();
    m_device.Store( GUCEF_NULL, MT::MEMORDER_RELEASE );
    OnDeviceLost();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
