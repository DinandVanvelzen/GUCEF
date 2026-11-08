/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#if defined( __linux__ ) && !defined( __ANDROID__ )
  #include <pthread.h>
  #define GUCEF_MT_TESTAPP_MUTEX_LAYOUT_KNOWN
#endif

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestOwnerThreadIds.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 CommandWaitLimitInMs = 30000;
const Int32 ThreadWaitLimitInMs = 30000;

/** The locks the workers operate on, owned by the test case */
struct SOwnerLocks
{
    SOwnerLocks( void )
        : mutex( MT::MutexCreate() )
        , cmutex()
        , rwLock( true )
    {}

    ~SOwnerLocks()
    {
        if ( GUCEF_NULL != mutex )
            MT::MutexDestroy( mutex );
    }

    struct MT::SMutex* mutex;
    MT::CMutex cmutex;
    MT::CReadWriteLock rwLock;
};

/** What the main thread asks a worker thread to do, one command at a time */
enum EOwnerCommand
{
    OWNERCMD_NONE = 0,
    OWNERCMD_MUTEX_LOCK,
    OWNERCMD_MUTEX_UNLOCK,
    OWNERCMD_CMUTEX_LOCK,
    OWNERCMD_CMUTEX_UNLOCK,
    OWNERCMD_RW_READER_START,
    OWNERCMD_RW_READER_STOP,
    OWNERCMD_RW_WRITER_START,
    OWNERCMD_RW_WRITER_STOP,
    OWNERCMD_QUIT
};

/** One worker thread made with MT::ThreadCreate(): it records its own ids, then runs the commands the main thread posts */
class CWorker
{
    public:

    CWorker( SOwnerLocks& locks )
        : m_locks( locks )
        , m_td( MT::ThreadDataReserve() )
        , m_command( (Int32) OWNERCMD_NONE )
        , m_requestSeq( 0 )
        , m_doneSeq( 0 )
        , m_threadId( 0 )
        , m_systemThreadId( 0 )
        , m_lastResult( 0 )
        , m_nextSeq( 0 )
        , m_started( false )
    {
        if ( GUCEF_NULL != m_td && 0 != MT::ThreadCreate( m_td, &WorkerThreadFunc, this ) )
            m_started = WaitForStart();
    }

    ~CWorker()
    {
        if ( m_started )
        {
            Run( OWNERCMD_QUIT );
            MT::ThreadWait( m_td, ThreadWaitLimitInMs );
        }
        if ( GUCEF_NULL != m_td )
            MT::ThreadDataCleanup( m_td );
    }

    bool IsStarted( void ) const { return m_started; }

    /** Runs the command on the worker and gives its result, GUCEF_UINT32MAX if the worker did not finish it in time */
    UInt32 Run( EOwnerCommand command )
    {
        const UInt32 seq = ++m_nextSeq;
        m_command.Store( (Int32) command, MT::MEMORDER_RELAXED );
        m_requestSeq.Store( seq, MT::MEMORDER_RELEASE );
        if ( !WaitForDone( seq ) )
            return GUCEF_UINT32MAX;
        return m_lastResult;
    }

    TThreadId GetThreadId( void ) const { return m_threadId; }
    TSystemThreadId GetSystemThreadId( void ) const { return m_systemThreadId; }
    MT::SThreadData* GetThreadData( void ) const { return m_td; }

    private:

    bool WaitForStart( void )
    {
        for ( UInt32 waited=0; waited<CommandWaitLimitInMs; ++waited )
        {
            if ( 0 != m_threadId.Load( MT::MEMORDER_ACQUIRE ) )
                return true;
            MT::ThreadDelay( 1 );
        }
        return false;
    }

    bool WaitForDone( UInt32 seq )
    {
        for ( UInt32 waited=0; waited<CommandWaitLimitInMs; ++waited )
        {
            if ( seq == m_doneSeq.Load( MT::MEMORDER_ACQUIRE ) )
                return true;
            MT::ThreadDelay( 1 );
        }
        return false;
    }

    static UInt32 GUCEF_CALLSPEC_PREFIX
    WorkerThreadFunc( void* data ) GUCEF_CALLSPEC_SUFFIX
    {
        CWorker* worker = static_cast< CWorker* >( data );

        // the ids as this thread obtains them itself: what the objects have to report for it
        worker->m_systemThreadId.Store( MT::GetCurrentThreadSystemID() );
        worker->m_threadId.Store( MT::GetCurrentThreadID(), MT::MEMORDER_RELEASE );

        UInt32 handledSeq = 0;
        while ( true )
        {
            UInt32 seq = worker->m_requestSeq.Load( MT::MEMORDER_ACQUIRE );
            if ( seq != handledSeq )
            {
                EOwnerCommand command = (EOwnerCommand) worker->m_command.Load( MT::MEMORDER_RELAXED );
                worker->m_lastResult = worker->Execute( command );
                handledSeq = seq;
                worker->m_doneSeq.Store( seq, MT::MEMORDER_RELEASE );
                if ( OWNERCMD_QUIT == command )
                    return 0;
            }
            else
            {
                MT::ThreadDelay( 1 );
            }
        }
    }

    UInt32 Execute( EOwnerCommand command )
    {
        switch ( command )
        {
            case OWNERCMD_MUTEX_LOCK:       return MT::MutexLock( m_locks.mutex, GUCEF_MUTEX_INFINITE_TIMEOUT );
            case OWNERCMD_MUTEX_UNLOCK:     return MT::MutexUnlock( m_locks.mutex );
            case OWNERCMD_CMUTEX_LOCK:      return (UInt32) m_locks.cmutex.Lock();
            case OWNERCMD_CMUTEX_UNLOCK:    return (UInt32) m_locks.cmutex.Unlock();
            case OWNERCMD_RW_READER_START:  return (UInt32) m_locks.rwLock.ReaderStart();
            case OWNERCMD_RW_READER_STOP:   return (UInt32) m_locks.rwLock.ReaderStop();
            case OWNERCMD_RW_WRITER_START:  return (UInt32) m_locks.rwLock.WriterStart();
            case OWNERCMD_RW_WRITER_STOP:   return (UInt32) m_locks.rwLock.WriterStop();
            default:                        return 0;
        }
    }

    CWorker( const CWorker& src );
    CWorker& operator=( const CWorker& src );

    SOwnerLocks& m_locks;
    MT::SThreadData* m_td;
    MT::CTAtomic< Int32 > m_command;        /**< written by the main thread before m_requestSeq */
    MT::CTAtomic< UInt32 > m_requestSeq;    /**< the main thread posts a command by raising this */
    MT::CTAtomic< UInt32 > m_doneSeq;       /**< the worker reports the command done by setting this to m_requestSeq */
    MT::CTAtomic< TThreadId > m_threadId;
    MT::CTAtomic< TSystemThreadId > m_systemThreadId;
    UInt32 m_lastResult;                    /**< written by the worker before m_doneSeq, read by the main thread after it */
    UInt32 m_nextSeq;                       /**< used by the main thread only */
    MT::CTAtomic< bool > m_started;
};

/*-------------------------------------------------------------------------*/

/** Records the ids of its thread as that thread obtains them, and the ids the object reports for it meanwhile */
class CIdRecordingActiveObject : public MT::CActiveObject
{
    public:

    CIdRecordingActiveObject( void )
        : MT::CActiveObject()
        , m_threadId( 0 )
        , m_systemThreadId( 0 )
        , m_recorded( false )
        , m_end( false )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_systemThreadId.Store( MT::GetCurrentThreadSystemID() );
        m_threadId.Store( MT::GetCurrentThreadID() );
        m_recorded.Store( true, MT::MEMORDER_RELEASE );
        for ( UInt32 i=0; i<(UInt32)ThreadWaitLimitInMs && !m_end.Load(); ++i )
            MT::ThreadDelay( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilRecorded( void ) const
    {
        for ( UInt32 i=0; i<(UInt32)ThreadWaitLimitInMs && !m_recorded.Load( MT::MEMORDER_ACQUIRE ); ++i )
            MT::ThreadDelay( 1 );
        return m_recorded.Load( MT::MEMORDER_ACQUIRE );
    }

    void End( void ) { m_end.Store( true ); }

    MT::CTAtomic< TThreadId > m_threadId;
    MT::CTAtomic< TSystemThreadId > m_systemThreadId;

    private:

    MT::CTAtomic< bool > m_recorded;
    MT::CTAtomic< bool > m_end;
};

/*-------------------------------------------------------------------------*/

/** A structure declared with the helper macro: it has the second member only where the ids differ */
struct STestOwnerRecord
{
    TThreadId threadId;
    GUCEF_SYSTEM_THREAD_ID_MEMBER( systemThreadId )
};

#ifdef GUCEF_MT_TESTAPP_MUTEX_LAYOUT_KNOWN

/** The members of the C mutex in gucefMT_MUTEX.c, for each configuration, to check its size */
struct SMutexLayoutWithSystemThreadId
{
    TThreadId lockedByThread;
    TSystemThreadId lockedBySystemThread;
    Int32 reentrancyCount;
    pthread_mutex_t id;
};
struct SMutexLayoutWithoutSystemThreadId
{
    TThreadId lockedByThread;
    Int32 reentrancyCount;
    pthread_mutex_t id;
};

#endif /* GUCEF_MT_TESTAPP_MUTEX_LAYOUT_KNOWN ? */

/*-------------------------------------------------------------------------*/

/** The system id an object has to report for a thread that recorded 'systemThreadId' itself */
TSystemThreadId
ExpectedRecordedSystemThreadId( TSystemThreadId systemThreadId )
{
    #if !GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID && !GUCEF_MT_RECORD_SYSTEM_THREAD_ID
    return 0;
    #else
    return systemThreadId;
    #endif
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOwnerThreadIdsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OwnerThreadIds TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.OwnerThreadIds" );

    GUCEF_TESTFW_TESTCASE( "Both ids of the calling thread: one call gives what the two functions give" )
    try
    {
        MT::SThreadIds ids;
        MT::GetCurrentThreadIds( &ids );
        ASSERT_TRUE( 0 != ids.threadId );
        ASSERT_TRUE( MT::GetCurrentThreadID() == ids.threadId );
        ASSERT_TRUE( 0 != MT::GetCurrentThreadSystemID() );
        ASSERT_TRUE( MT::GetCurrentThreadSystemID() == MT::GetCurrentThreadSystemID() );

        #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
        // one id only: the system id is the identity, in type and in value
        ASSERT_TRUE( sizeof( TSystemThreadId ) == sizeof( TThreadId ) );
        ASSERT_TRUE( (TSystemThreadId) MT::GetCurrentThreadID() == MT::GetCurrentThreadSystemID() );
        #else
        ASSERT_TRUE( sizeof( TSystemThreadId ) == sizeof( UInt32 ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( MT::GetCurrentThreadSystemID() ) == ids.systemThreadId );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Structures declared with the system thread id macro have the second id only where the ids differ" )
    try
    {
        #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
        ASSERT_TRUE( sizeof( MT::SThreadIds ) == sizeof( TThreadId ) );
        ASSERT_TRUE( sizeof( STestOwnerRecord ) == sizeof( TThreadId ) );
        #else
        ASSERT_TRUE( sizeof( MT::SThreadIds ) >= sizeof( TThreadId ) + sizeof( TSystemThreadId ) );
        ASSERT_TRUE( sizeof( STestOwnerRecord ) == sizeof( MT::SThreadIds ) );
        #endif

        #ifdef GUCEF_MT_TESTAPP_MUTEX_LAYOUT_KNOWN
        #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
        ASSERT_TRUE( sizeof( SMutexLayoutWithoutSystemThreadId ) == MT::MutexStructSize() );
        #else
        ASSERT_TRUE( sizeof( SMutexLayoutWithSystemThreadId ) == MT::MutexStructSize() );
        #endif
        #endif

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "OwnerThreadIds: GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID=" << (Int32) GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID <<
            ", sizeof( TThreadId )=" << (UInt32) sizeof( TThreadId ) << ", sizeof( TSystemThreadId )=" << (UInt32) sizeof( TSystemThreadId ) <<
            ", sizeof( SThreadIds )=" << (UInt32) sizeof( MT::SThreadIds ) << ", C mutex data " << MT::MutexStructSize() << " bytes" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The thread data reports both ids of a thread made with ThreadCreate()" )
    SOwnerLocks locks;
    CWorker worker( locks );
    try
    {
        ASSERT_TRUE( worker.IsStarted() );
        ASSERT_TRUE( 0 != worker.GetThreadId() );
        ASSERT_TRUE( worker.GetThreadId() != MT::GetCurrentThreadID() );
        ASSERT_TRUE( worker.GetThreadId() == MT::ThreadIdentity( worker.GetThreadData() ) );
        ASSERT_TRUE( worker.GetSystemThreadId() == MT::ThreadID( worker.GetThreadData() ) );
        ASSERT_TRUE( 0 == MT::ThreadIdentity( GUCEF_NULL ) );
        ASSERT_TRUE( 0 == MT::ThreadID( GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "C mutex: the holder's ids are reported while it holds the lock, also re-entered, and cleared after" )
    SOwnerLocks locks;
    CWorker worker( locks );
    try
    {
        ASSERT_TRUE( worker.IsStarted() );
        ASSERT_TRUE( 0 == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( 0 == MT::MutexLockedBySystemThread( locks.mutex ) );

        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == worker.Run( OWNERCMD_MUTEX_LOCK ) );
        ASSERT_TRUE( worker.GetThreadId() == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == MT::MutexLockedBySystemThread( locks.mutex ) );

        // re-entered: still the same holder
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == worker.Run( OWNERCMD_MUTEX_LOCK ) );
        ASSERT_TRUE( 1 == MT::MutexReentrancy( locks.mutex ) );
        ASSERT_TRUE( worker.GetThreadId() == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == MT::MutexLockedBySystemThread( locks.mutex ) );

        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == worker.Run( OWNERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( worker.GetThreadId() == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == MT::MutexLockedBySystemThread( locks.mutex ) );

        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == worker.Run( OWNERCMD_MUTEX_UNLOCK ) );
        ASSERT_TRUE( 0 == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( 0 == MT::MutexLockedBySystemThread( locks.mutex ) );

        // the main thread as holder
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexLock( locks.mutex, GUCEF_MUTEX_INFINITE_TIMEOUT ) );
        ASSERT_TRUE( MT::GetCurrentThreadID() == MT::MutexLockedByThread( locks.mutex ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( MT::GetCurrentThreadSystemID() ) == MT::MutexLockedBySystemThread( locks.mutex ) );
        ASSERT_TRUE( GUCEF_MUTEX_OPERATION_SUCCESS == MT::MutexUnlock( locks.mutex ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CMutex: the ids of the thread last holding the lock are reported, also after it let go" )
    SOwnerLocks locks;
    CWorker worker( locks );
    try
    {
        ASSERT_TRUE( worker.IsStarted() );
        ASSERT_TRUE( 0 == locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( 0 == locks.cmutex.GetSystemThreadIdOfThreadLastHoldingTheLock() );

        ASSERT_TRUE( (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS == worker.Run( OWNERCMD_CMUTEX_LOCK ) );
        ASSERT_TRUE( worker.GetThreadId() == locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == locks.cmutex.GetSystemThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( (UInt32) MT::LOCKSTATUS_OPERATION_SUCCESS == worker.Run( OWNERCMD_CMUTEX_UNLOCK ) );

        // the last holder stays recorded
        ASSERT_TRUE( worker.GetThreadId() == locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == locks.cmutex.GetSystemThreadIdOfThreadLastHoldingTheLock() );

        // a new holder replaces both ids
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == locks.cmutex.Lock() );
        ASSERT_TRUE( MT::GetCurrentThreadID() == locks.cmutex.GetThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( MT::GetCurrentThreadSystemID() ) == locks.cmutex.GetSystemThreadIdOfThreadLastHoldingTheLock() );
        ASSERT_TRUE( MT::LOCKSTATUS_OPERATION_SUCCESS == locks.cmutex.Unlock() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Read/write lock: the writer's ids are reported, and kept as the last writer after it let go" )
    SOwnerLocks locks;
    CWorker worker( locks );
    try
    {
        ASSERT_TRUE( worker.IsStarted() );
        ASSERT_TRUE( 0 == locks.rwLock.LastWriterThreadId() );
        ASSERT_TRUE( 0 == locks.rwLock.LastWriterSystemThreadId() );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == worker.Run( OWNERCMD_RW_WRITER_START ) );
        ASSERT_TRUE( worker.GetThreadId() == locks.rwLock.LastWriterThreadId() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == locks.rwLock.LastWriterSystemThreadId() );

        // re-entered by the writer
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == worker.Run( OWNERCMD_RW_WRITER_START ) );
        ASSERT_TRUE( worker.GetThreadId() == locks.rwLock.LastWriterThreadId() );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == worker.Run( OWNERCMD_RW_WRITER_STOP ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == worker.Run( OWNERCMD_RW_WRITER_STOP ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveWriterCount() );

        ASSERT_TRUE( worker.GetThreadId() == locks.rwLock.LastWriterThreadId() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( worker.GetSystemThreadId() ) == locks.rwLock.LastWriterSystemThreadId() );

        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.WriterStart() );
        ASSERT_TRUE( MT::GetCurrentThreadID() == locks.rwLock.LastWriterThreadId() );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( MT::GetCurrentThreadSystemID() ) == locks.rwLock.LastWriterSystemThreadId() );
        ASSERT_TRUE( MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == locks.rwLock.WriterStop() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Read/write lock: each active reader's system id is reported for its thread id, and cleared when it stops" )
    SOwnerLocks locks;
    CWorker first( locks );
    CWorker second( locks );
    try
    {
        ASSERT_TRUE( first.IsStarted() && second.IsStarted() );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderSystemThreadId( first.GetThreadId() ) );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == first.Run( OWNERCMD_RW_READER_START ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == second.Run( OWNERCMD_RW_READER_START ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == second.Run( OWNERCMD_RW_READER_START ) );
        ASSERT_TRUE( 2 == locks.rwLock.ActiveReaderCount() );

        ASSERT_TRUE( ExpectedRecordedSystemThreadId( first.GetSystemThreadId() ) == locks.rwLock.ActiveReaderSystemThreadId( first.GetThreadId() ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( second.GetSystemThreadId() ) == locks.rwLock.ActiveReaderSystemThreadId( second.GetThreadId() ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderSystemThreadId( MT::GetCurrentThreadID() ) );

        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == first.Run( OWNERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderSystemThreadId( first.GetThreadId() ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( second.GetSystemThreadId() ) == locks.rwLock.ActiveReaderSystemThreadId( second.GetThreadId() ) );

        // a re-entered reader keeps its slot until its last stop
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == second.Run( OWNERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( ExpectedRecordedSystemThreadId( second.GetSystemThreadId() ) == locks.rwLock.ActiveReaderSystemThreadId( second.GetThreadId() ) );
        ASSERT_TRUE( (UInt32) MT::CReadWriteLock::RWLOCK_OPERATION_SUCCESS == second.Run( OWNERCMD_RW_READER_STOP ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderSystemThreadId( second.GetThreadId() ) );
        ASSERT_TRUE( 0 == locks.rwLock.ActiveReaderCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CActiveObject reports both ids of its thread" )
    CIdRecordingActiveObject activeObject;
    try
    {
        ASSERT_TRUE( 0 == activeObject.GetThreadIdentity() );
        ASSERT_TRUE( 0 == activeObject.GetThreadID() );
        ASSERT_TRUE( activeObject.Activate() );
        ASSERT_TRUE( activeObject.WaitUntilRecorded() );

        ASSERT_TRUE( 0 != activeObject.m_threadId.Load() );
        ASSERT_TRUE( activeObject.m_threadId.Load() == activeObject.GetThreadIdentity() );
        ASSERT_TRUE( activeObject.m_systemThreadId.Load() == activeObject.GetThreadID() );
        #if GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID
        ASSERT_TRUE( (TSystemThreadId) activeObject.GetThreadIdentity() == activeObject.GetThreadID() );
        #endif

        activeObject.End();
        activeObject.Deactivate( false, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
