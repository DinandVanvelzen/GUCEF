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

#include <string.h>
#include <errno.h>

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CEXTERNALTHREADSCOPE_H
#include "gucefCORE_CExternalThreadScope.h"
#define GUCEF_CORE_CEXTERNALTHREADSCOPE_H
#endif /* GUCEF_CORE_CEXTERNALTHREADSCOPE_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_STDOUTCLAIM_H
#include "gucefCORE_StdoutClaim.h"
#define GUCEF_CORE_STDOUTCLAIM_H
#endif /* GUCEF_CORE_STDOUTCLAIM_H ? */

#include "gucefCORE_CStandardOutputWriter.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <stdio.h>
#else
  #include <unistd.h>
  #include <poll.h>
  #include <fcntl.h>
  #include <signal.h>
  #include <pthread.h>
  #include <limits.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CStandardOutputWriter::BrokenPipeEvent = "GUCEF::CORE::CStandardOutputWriter::BrokenPipeEvent";

const CString CStandardOutputWriter::ClassTypeName = "GUCEF::CORE::CStandardOutputWriter";


#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

/**
 *  The most bytes written at once after poll() reported the stream writable. A pipe that reports POLLOUT takes at
 *  least PIPE_BUF bytes, so a write of that size does not block, and a stop never waits for a stalled reader
 */
#ifdef PIPE_BUF
static const UInt32 WriteChunkSize = PIPE_BUF;
#else
static const UInt32 WriteChunkSize = 512;
#endif

/** How long the writer thread waits for a stalled stream before it looks at a stop again */
static const int WritePollTimeoutInMs = 100;

#else

static const UInt32 WriteChunkSize = 64 * 1024;

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The writer thread. The whole write loop runs in one cycle, like the one of CStandardInputReader */
class CStandardOutputWriter::CWriterThread : public MT::CActiveObject
{
    public:

    explicit CWriterThread( CStandardOutputWriter& writer )
        : MT::CActiveObject()
        , m_writer( writer )
    {
    }

    virtual ~CWriterThread() GUCEF_VIRTUAL_OVERRIDE
    {
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        CExternalThreadScope threadScope( "gucef.stdout-writer" );
        m_writer.WriteLoop();
        return true;    // done, the thread ends
    }

    private:

    CWriterThread( const CWriterThread& src );
    CWriterThread& operator=( const CWriterThread& src );

    CStandardOutputWriter& m_writer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
NowInMs( void )
{GUCEF_TRACE;

    return CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

/** Reads what is in the (non-blocking) wake pipe, so the next wait waits again */
static void
DrainWakePipe( int wakeRead )
{GUCEF_TRACE;

    char buffer[ 64 ];
    while ( read( wakeRead, buffer, sizeof( buffer ) ) > 0 ) {}
}

/*-------------------------------------------------------------------------*/

/**
 *  Blocks SIGPIPE for the calling thread for as long as the object exists, so a write to a closed pipe fails with
 *  EPIPE instead of ending the process, and takes away a SIGPIPE such a write left pending
 */
class CThreadSigPipeBlock
{
    public:

    CThreadSigPipeBlock( void )
    {
        sigemptyset( &m_pipeSet );
        sigaddset( &m_pipeSet, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &m_pipeSet, &m_oldMask );
    }

    ~CThreadSigPipeBlock()
    {
        sigset_t pending;
        sigemptyset( &pending );
        if ( 0 == sigpending( &pending ) && 0 != sigismember( &pending, SIGPIPE ) )
        {
            #if defined( __APPLE__ )
            // macOS has no sigtimedwait(); the signal is pending for this thread, so sigwait() returns at once
            int taken = 0;
            sigwait( &m_pipeSet, &taken );
            #else
            struct timespec noWait;
            noWait.tv_sec = 0;
            noWait.tv_nsec = 0;
            sigtimedwait( &m_pipeSet, GUCEF_NULL, &noWait );
            #endif
        }
        pthread_sigmask( SIG_SETMASK, &m_oldMask, GUCEF_NULL );
    }

    private:

    CThreadSigPipeBlock( const CThreadSigPipeBlock& src );
    CThreadSigPipeBlock& operator=( const CThreadSigPipeBlock& src );

    sigset_t m_pipeSet;
    sigset_t m_oldMask;
};

#endif

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/**
 *  CancelSynchronousIo() of kernel32 (Windows Vista and later), looked up at run time so the code builds whatever
 *  _WIN32_WINNT the build targets. Cancels a blocking ReadFile() or WriteFile() of the given thread
 */
static void
CancelBlockingIoOfThread( void* threadHandle )
{GUCEF_TRACE;

    typedef BOOL ( WINAPI *TCancelSynchronousIo )( HANDLE thread );
    static TCancelSynchronousIo cancelSynchronousIo =
        (TCancelSynchronousIo) ::GetProcAddress( ::GetModuleHandleA( "kernel32.dll" ), "CancelSynchronousIo" );
    if ( GUCEF_NULL != cancelSynchronousIo && GUCEF_NULL != threadHandle )
        cancelSynchronousIo( (HANDLE) threadHandle );
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CStandardOutputWriter::RegisterEvents( void )
{GUCEF_TRACE;

    BrokenPipeEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::ClaimExclusive( void )
{GUCEF_TRACE;

    return TryClaimStdout();
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::IsClaimed( void )
{GUCEF_TRACE;

    return IsStdoutClaimed();
}

/*-------------------------------------------------------------------------*/

void
CStandardOutputWriter::ReleaseClaim( void )
{GUCEF_TRACE;

    ReleaseStdoutClaim();
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::SetSigPipeIgnoredProcessWide( bool ignore )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return false;

    #else

    struct sigaction action;
    memset( &action, 0, sizeof( action ) );
    action.sa_handler = ignore ? SIG_IGN : SIG_DFL;
    sigemptyset( &action.sa_mask );
    if ( 0 != sigaction( SIGPIPE, &action, GUCEF_NULL ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: cannot change the process wide disposition of SIGPIPE: " << strerror( errno ) );
        return false;
    }
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: SIGPIPE is " << ( ignore ? "ignored" : "back to its default disposition" ) << " process wide, as asked" );
    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

static CStandardOutputWriter::TStreamHandle
StandardOutputHandle( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (CStandardOutputWriter::TStreamHandle) ::GetStdHandle( STD_OUTPUT_HANDLE );
    #else
    return STDOUT_FILENO;
    #endif
}

/*-------------------------------------------------------------------------*/

CStandardOutputWriter::CStandardOutputWriter( UInt32 maxQueuedBytes )
    : CNotifier()
    , m_maxQueuedBytes( maxQueuedBytes )
    , m_stream( StandardOutputHandle() )
    , m_isStandardOutput( true )
    , m_thread( GUCEF_NULL )
    , m_started( false )
    , m_queue()
    , m_queuedBytes( 0 )
    , m_broken( false )
    , m_stopRequested( false )
    , m_flushOnStop( false )
    , m_flushDeadlineInMs( 0 )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    , m_wakeEvent( GUCEF_NULL )
    , m_threadHandle( GUCEF_NULL )
    #else
    , m_wakeRead( -1 )
    , m_wakeWrite( -1 )
    #endif
    , m_queueLock()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
    m_queue.SetGrowthStrategy( CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    m_thread = GUCEF_NEW CWriterThread( *this );
}

/*-------------------------------------------------------------------------*/

CStandardOutputWriter::CStandardOutputWriter( UInt32 maxQueuedBytes, TStreamHandle stream )
    : CNotifier()
    , m_maxQueuedBytes( maxQueuedBytes )
    , m_stream( stream )
    , m_isStandardOutput( false )
    , m_thread( GUCEF_NULL )
    , m_started( false )
    , m_queue()
    , m_queuedBytes( 0 )
    , m_broken( false )
    , m_stopRequested( false )
    , m_flushOnStop( false )
    , m_flushDeadlineInMs( 0 )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    , m_wakeEvent( GUCEF_NULL )
    , m_threadHandle( GUCEF_NULL )
    #else
    , m_wakeRead( -1 )
    , m_wakeWrite( -1 )
    #endif
    , m_queueLock()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
    m_queue.SetGrowthStrategy( CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    m_thread = GUCEF_NEW CWriterThread( *this );
}

/*-------------------------------------------------------------------------*/

CStandardOutputWriter::~CStandardOutputWriter()
{GUCEF_TRACE;

    Stop( false );
    GUCEF_DELETE m_thread;
    m_thread = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CString&
CStandardOutputWriter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStandardOutputWriter::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStandardOutputWriter::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::IsBroken( void ) const
{GUCEF_TRACE;

    return m_broken.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CStandardOutputWriter::GetQueuedBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    return m_queuedBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CStandardOutputWriter::GetMaxQueuedBytes( void ) const
{GUCEF_TRACE;

    return m_maxQueuedBytes;
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::Start( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    if ( m_started )
        return false;

    m_broken = false;
    m_stopRequested = false;
    m_flushOnStop = false;
    m_queue.Clear();
    m_queuedBytes = 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( m_isStandardOutput )
    {
        // the bytes as they are: no text mode translation of line ends
        fflush( stdout );
        _setmode( _fileno( stdout ), _O_BINARY );
        m_stream = (TStreamHandle) ::GetStdHandle( STD_OUTPUT_HANDLE );
    }
    if ( GUCEF_NULL == m_stream || INVALID_HANDLE_VALUE == (HANDLE) m_stream )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: there is no output stream to write to" );
        return false;
    }
    m_wakeEvent = (void*) ::CreateEventA( NULL, FALSE, FALSE, NULL );
    if ( GUCEF_NULL == m_wakeEvent )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: cannot create the wake event" );
        return false;
    }

    #else

    int wakePipe[ 2 ] = { -1, -1 };
    if ( 0 != pipe( wakePipe ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: cannot create the wake pipe: " << strerror( errno ) );
        return false;
    }
    for ( int i=0; i<2; ++i )
    {
        fcntl( wakePipe[ i ], F_SETFD, FD_CLOEXEC );
        fcntl( wakePipe[ i ], F_SETFL, fcntl( wakePipe[ i ], F_GETFL ) | O_NONBLOCK );
    }
    m_wakeRead = wakePipe[ 0 ];
    m_wakeWrite = wakePipe[ 1 ];

    #endif

    if ( !m_thread->Activate( GUCEF_NULL, 0, 0 ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: cannot start the writer thread" );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        ::CloseHandle( (HANDLE) m_wakeEvent );
        m_wakeEvent = GUCEF_NULL;
        #else
        close( m_wakeRead );
        close( m_wakeWrite );
        m_wakeRead = m_wakeWrite = -1;
        #endif
        return false;
    }
    m_started = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStandardOutputWriter::WakeWriterThread( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( GUCEF_NULL != m_wakeEvent )
        ::SetEvent( (HANDLE) m_wakeEvent );
    #else
    char wake = 1;
    if ( write( m_wakeWrite, &wake, 1 ) < 0 && EAGAIN != errno )
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: cannot wake the writer thread: " << strerror( errno ) );
    #endif
}

/*-------------------------------------------------------------------------*/

void
CStandardOutputWriter::Stop( bool flush, UInt32 flushTimeoutInMs )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_queueLock );
        if ( !m_started )
            return;
        m_flushOnStop = flush;
        m_flushDeadlineInMs = NowInMs() + flushTimeoutInMs;
    }
    m_stopRequested = true;
    WakeWriterThread();

    // The thread gives up on what is left at the deadline; the wait for it covers that, also beyond the
    // 15 seconds Deactivate() waits at most
    UInt64 waitUntil = NowInMs() + ( flush ? flushTimeoutInMs : 0 ) + 2000;
    while ( !m_thread->HasThreadEnded() && NowInMs() < waitUntil )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        // A WriteFile() blocked on a reader that does not read is woken by cancelling it
        if ( ShouldAbandonWriting() )
        {
            MT::CScopeMutex lock( m_queueLock );
            CancelBlockingIoOfThread( m_threadHandle );
        }
        #endif
        MT::ThreadDelay( 5 );
    }
    m_thread->Deactivate( false, true );

    MT::CScopeMutex lock( m_queueLock );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    ::CloseHandle( (HANDLE) m_wakeEvent );
    m_wakeEvent = GUCEF_NULL;
    #else
    close( m_wakeRead );
    close( m_wakeWrite );
    m_wakeRead = m_wakeWrite = -1;
    #endif
    m_queue.Clear();
    m_queuedBytes = 0;
    m_started = false;
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::WriteLine( const CString& line )
{GUCEF_TRACE;

    const char* data = line.C_String();
    UInt32 size = line.ByteSize() > 0 ? line.ByteSize() - 1 : 0;
    if ( GUCEF_NULL != data && GUCEF_NULL != memchr( data, '\n', size ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: refused a line with a line end inside it, it would break the framing" );
        return false;
    }

    bool wasEmpty = false;
    {
        MT::CScopeMutex lock( m_queueLock );
        if ( !m_started || m_stopRequested || m_broken )
            return false;
        if ( (UInt64) m_queuedBytes + size + 1 > m_maxQueuedBytes )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: the queue is full (" << m_queuedBytes << " bytes): the reader does not keep up, a line of " << size+1 << " bytes is refused" );
            return false;
        }
        wasEmpty = 0 == m_queue.GetDataSize();
        if ( size > 0 )
            m_queue.Append( data, size );
        m_queue.Append( "\n", 1 );
        m_queuedBytes += size + 1;
    }
    if ( wasEmpty )
        WakeWriterThread();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::ShouldAbandonWriting( void ) const
{GUCEF_TRACE;

    if ( !m_stopRequested )
        return false;
    MT::CScopeMutex lock( m_queueLock );
    return !m_flushOnStop || NowInMs() >= m_flushDeadlineInMs;
}

/*-------------------------------------------------------------------------*/

void
CStandardOutputWriter::OnBroken( const CString& reason )
{GUCEF_TRACE;

    m_broken = true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardOutputWriter: the output stream broke: " << reason );
    TBrokenPipeEventData eventData( reason );
    NotifyObservers( BrokenPipeEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CStandardOutputWriter::WriteAll( const char* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 offset = 0;
    while ( offset < size )
    {
        if ( ShouldAbandonWriting() )
            return false;

        UInt32 chunk = size - offset;
        if ( chunk > WriteChunkSize )
            chunk = WriteChunkSize;

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        DWORD written = 0;
        if ( !::WriteFile( (HANDLE) m_stream, data + offset, (DWORD) chunk, &written, NULL ) )
        {
            DWORD errorCode = ::GetLastError();
            if ( ERROR_OPERATION_ABORTED == errorCode )
                continue;       // Stop() cancelled the write, the loop looks at the stop
            if ( ERROR_BROKEN_PIPE == errorCode || ERROR_NO_DATA == errorCode )
                OnBroken( "the reading end of the stream is closed" );
            else
                OnBroken( "WriteFile() failed with error code " + UInt32ToString( (UInt32) errorCode ) );
            return false;
        }

        #else

        struct pollfd waitFor[ 2 ];
        waitFor[ 0 ].fd = m_stream;
        waitFor[ 0 ].events = POLLOUT;
        waitFor[ 0 ].revents = 0;
        waitFor[ 1 ].fd = m_wakeRead;
        waitFor[ 1 ].events = POLLIN;
        waitFor[ 1 ].revents = 0;
        int ready = poll( waitFor, 2, WritePollTimeoutInMs );
        if ( ready < 0 )
        {
            if ( EINTR == errno )
                continue;
            OnBroken( CString( "poll() failed: " ) + strerror( errno ) );
            return false;
        }
        if ( 0 != waitFor[ 1 ].revents )
            DrainWakePipe( m_wakeRead );     // more to write or a stop: the loop looks at both
        if ( 0 != ( waitFor[ 0 ].revents & POLLNVAL ) )
        {
            OnBroken( "the output stream is not open" );
            return false;
        }
        if ( 0 == ( waitFor[ 0 ].revents & ( POLLOUT | POLLERR | POLLHUP ) ) )
            continue;

        ssize_t result = write( m_stream, data + offset, chunk );
        if ( result < 0 )
        {
            if ( EINTR == errno || EAGAIN == errno )
                continue;
            if ( EPIPE == errno )
                OnBroken( "the reading end of the stream is closed" );
            else
                OnBroken( CString( "write() failed: " ) + strerror( errno ) );
            return false;
        }
        UInt32 written = (UInt32) result;

        #endif

        offset += (UInt32) written;
        MT::CScopeMutex lock( m_queueLock );
        m_queuedBytes -= (UInt32) written;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStandardOutputWriter::WriteLoop( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    HANDLE threadHandle = NULL;
    ::DuplicateHandle( ::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), &threadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS );
    {
        MT::CScopeMutex lock( m_queueLock );
        m_threadHandle = threadHandle;
    }
    #elif ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    CThreadSigPipeBlock sigPipeBlock;
    #endif

    CDynamicBuffer sending;
    sending.SetGrowthStrategy( CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_queueLock );
            if ( m_queue.GetDataSize() > 0 )
                m_queue.Swap( sending );    // the queue takes the empty buffer, which keeps its capacity
        }
        if ( sending.GetDataSize() > 0 )
        {
            bool written = WriteAll( static_cast< const char* >( sending.GetConstBufferPtr() ), sending.GetDataSize() );
            sending.SetDataSize( 0 );
            if ( !written )
                break;
            continue;
        }
        if ( m_stopRequested )
            break;

        // Nothing to write: wait for a line or a stop
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        ::WaitForSingleObject( (HANDLE) m_wakeEvent, INFINITE );
        #else
        struct pollfd waitFor;
        waitFor.fd = m_wakeRead;
        waitFor.events = POLLIN;
        waitFor.revents = 0;
        if ( poll( &waitFor, 1, -1 ) > 0 )
            DrainWakePipe( m_wakeRead );
        #endif
    }

    // What could not be written any more is dropped
    {
        MT::CScopeMutex lock( m_queueLock );
        m_queue.Clear();
        m_queuedBytes = 0;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        m_threadHandle = GUCEF_NULL;
        #endif
    }
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( NULL != threadHandle )
        ::CloseHandle( threadHandle );
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
