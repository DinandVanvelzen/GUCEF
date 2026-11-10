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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CStandardInputReader.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <stdio.h>
#else
  #include <unistd.h>
  #include <poll.h>
  #include <fcntl.h>
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

const CEvent CStandardInputReader::LineReceivedEvent = "GUCEF::CORE::CStandardInputReader::LineReceivedEvent";
const CEvent CStandardInputReader::LineTooLongEvent = "GUCEF::CORE::CStandardInputReader::LineTooLongEvent";
const CEvent CStandardInputReader::EndOfInputEvent = "GUCEF::CORE::CStandardInputReader::EndOfInputEvent";
const CEvent CStandardInputReader::ReadErrorEvent = "GUCEF::CORE::CStandardInputReader::ReadErrorEvent";

const CString CStandardInputReader::ClassTypeName = "GUCEF::CORE::CStandardInputReader";

/** The size of the one buffer every read goes into */
static const UInt32 ReadBufferSize = 64 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The reader thread. The whole read loop runs in one cycle, so that the external thread scope is made and destroyed
 *  on the reader thread itself: CActiveObject::Deactivate() also calls OnThreadEnding() from the thread that stops it
 */
class CStandardInputReader::CReaderThread : public MT::CActiveObject
{
    public:

    explicit CReaderThread( CStandardInputReader& reader )
        : MT::CActiveObject()
        , m_reader( reader )
    {
    }

    virtual ~CReaderThread() GUCEF_VIRTUAL_OVERRIDE
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
        CExternalThreadScope threadScope( "gucef.stdin-reader" );
        m_reader.ReadLoop();
        return true;    // done, the thread ends
    }

    private:

    CReaderThread( const CReaderThread& src );
    CReaderThread& operator=( const CReaderThread& src );

    CStandardInputReader& m_reader;
};

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
CStandardInputReader::RegisterEvents( void )
{GUCEF_TRACE;

    LineReceivedEvent.Initialize();
    LineTooLongEvent.Initialize();
    EndOfInputEvent.Initialize();
    ReadErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

static CStandardInputReader::TStreamHandle
StandardInputHandle( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (CStandardInputReader::TStreamHandle) ::GetStdHandle( STD_INPUT_HANDLE );
    #else
    return STDIN_FILENO;
    #endif
}

/*-------------------------------------------------------------------------*/

CStandardInputReader::CStandardInputReader( UInt32 maxLineBytes )
    : CNotifier()
    , m_maxLineBytes( maxLineBytes )
    , m_stream( StandardInputHandle() )
    , m_isStandardInput( true )
    , m_thread( GUCEF_NULL )
    , m_started( false )
    , m_stopRequested( false )
    , m_partialLine()
    , m_skippingLine( false )
    , m_skippedBytes( 0 )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    , m_threadHandle( GUCEF_NULL )
    #else
    , m_wakeRead( -1 )
    , m_wakeWrite( -1 )
    #endif
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
    m_partialLine.SetGrowthStrategy( CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    m_thread = GUCEF_NEW CReaderThread( *this );
}

/*-------------------------------------------------------------------------*/

CStandardInputReader::CStandardInputReader( UInt32 maxLineBytes, TStreamHandle stream )
    : CNotifier()
    , m_maxLineBytes( maxLineBytes )
    , m_stream( stream )
    , m_isStandardInput( false )
    , m_thread( GUCEF_NULL )
    , m_started( false )
    , m_stopRequested( false )
    , m_partialLine()
    , m_skippingLine( false )
    , m_skippedBytes( 0 )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    , m_threadHandle( GUCEF_NULL )
    #else
    , m_wakeRead( -1 )
    , m_wakeWrite( -1 )
    #endif
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
    m_partialLine.SetGrowthStrategy( CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    m_thread = GUCEF_NEW CReaderThread( *this );
}

/*-------------------------------------------------------------------------*/

CStandardInputReader::~CStandardInputReader()
{GUCEF_TRACE;

    Stop();
    GUCEF_DELETE m_thread;
    m_thread = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CString&
CStandardInputReader::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStandardInputReader::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CStandardInputReader::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

UInt32
CStandardInputReader::GetMaxLineBytes( void ) const
{GUCEF_TRACE;

    return m_maxLineBytes;
}

/*-------------------------------------------------------------------------*/

bool
CStandardInputReader::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CStandardInputReader::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return false;

    m_stopRequested = false;
    m_partialLine.Clear();
    m_skippingLine = false;
    m_skippedBytes = 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( m_isStandardInput )
    {
        // the bytes as they come: no text mode translation of line ends
        _setmode( _fileno( stdin ), _O_BINARY );
        m_stream = (TStreamHandle) ::GetStdHandle( STD_INPUT_HANDLE );
    }
    if ( GUCEF_NULL == m_stream || INVALID_HANDLE_VALUE == (HANDLE) m_stream )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardInputReader: there is no input stream to read" );
        return false;
    }

    #else

    int wakePipe[ 2 ] = { -1, -1 };
    if ( 0 != pipe( wakePipe ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardInputReader: cannot create the wake pipe: " << strerror( errno ) );
        return false;
    }
    fcntl( wakePipe[ 0 ], F_SETFD, FD_CLOEXEC );
    fcntl( wakePipe[ 1 ], F_SETFD, FD_CLOEXEC );
    fcntl( wakePipe[ 1 ], F_SETFL, fcntl( wakePipe[ 1 ], F_GETFL ) | O_NONBLOCK );
    m_wakeRead = wakePipe[ 0 ];
    m_wakeWrite = wakePipe[ 1 ];

    #endif

    // No delay between cycles: there is only one, which reads until the end
    if ( !m_thread->Activate( GUCEF_NULL, 0, 0 ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CStandardInputReader: cannot start the reader thread" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
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
CStandardInputReader::Stop( void )
{GUCEF_TRACE;

    if ( !m_started )
        return;

    m_stopRequested = true;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // A blocking ReadFile() is woken by cancelling it. The thread may not be in it yet, so this repeats until it ended
    UInt32 waitedMs = 0;
    while ( !m_thread->HasThreadEnded() && waitedMs < 5000 )
    {
        {
            MT::CScopeMutex lock( m_lock );
            CancelBlockingIoOfThread( m_threadHandle );
        }
        MT::ThreadDelay( 10 );
        waitedMs += 10;
    }
    m_thread->Deactivate( false, true );

    #else

    char wake = 1;
    if ( write( m_wakeWrite, &wake, 1 ) < 0 && EAGAIN != errno )
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CStandardInputReader: cannot wake the reader thread: " << strerror( errno ) );
    m_thread->Deactivate( false, true );
    close( m_wakeRead );
    close( m_wakeWrite );
    m_wakeRead = m_wakeWrite = -1;

    #endif

    m_started = false;
}

/*-------------------------------------------------------------------------*/

bool
CStandardInputReader::DeliverLine( const char* data, UInt32 size )
{GUCEF_TRACE;

    if ( size > 0 && '\r' == data[ size-1 ] )
        --size;
    TLineReceivedEventData eventData;
    eventData.GetData() = CString( data, size );
    return NotifyObservers( LineReceivedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CStandardInputReader::ProcessBytes( const char* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 start = 0;
    while ( start < size )
    {
        const char* lineEnd = static_cast< const char* >( memchr( data + start, '\n', size - start ) );
        UInt32 end = GUCEF_NULL != lineEnd ? (UInt32) ( lineEnd - data ) : size;
        UInt32 segmentSize = end - start;

        if ( m_skippingLine )
        {
            m_skippedBytes += segmentSize;
        }
        else
        if ( (UInt64) m_partialLine.GetDataSize() + segmentSize > m_maxLineBytes )
        {
            // Too long: the rest of the line is skipped without being kept
            m_skippingLine = true;
            m_skippedBytes = (UInt64) m_partialLine.GetDataSize() + segmentSize;
            m_partialLine.Clear();
        }
        else
        if ( GUCEF_NULL != lineEnd && 0 == m_partialLine.GetDataSize() )
        {
            // The common case: a whole line inside the buffer is handed on without being copied first
            if ( !DeliverLine( data + start, segmentSize ) )
                return false;
        }
        else
        {
            m_partialLine.Append( data + start, segmentSize );
            if ( GUCEF_NULL != lineEnd )
            {
                bool alive = DeliverLine( static_cast< const char* >( m_partialLine.GetConstBufferPtr() ), m_partialLine.GetDataSize() );
                if ( !alive )
                    return false;
                m_partialLine.Clear();
            }
        }

        if ( GUCEF_NULL == lineEnd )
            break;

        if ( m_skippingLine )
        {
            TLineTooLongEventData eventData( m_skippedBytes > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32) m_skippedBytes );
            m_skippingLine = false;
            m_skippedBytes = 0;
            if ( !NotifyObservers( LineTooLongEvent, &eventData ) )
                return false;
        }
        start = end + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStandardInputReader::ProcessEndOfInput( void )
{GUCEF_TRACE;

    // A last line without a line end still counts
    if ( m_skippingLine )
    {
        TLineTooLongEventData eventData( m_skippedBytes > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32) m_skippedBytes );
        m_skippingLine = false;
        m_skippedBytes = 0;
        if ( !NotifyObservers( LineTooLongEvent, &eventData ) )
            return false;
    }
    else
    if ( m_partialLine.GetDataSize() > 0 )
    {
        bool alive = DeliverLine( static_cast< const char* >( m_partialLine.GetConstBufferPtr() ), m_partialLine.GetDataSize() );
        if ( !alive )
            return false;
        m_partialLine.Clear();
    }
    return NotifyObservers( EndOfInputEvent );
}

/*-------------------------------------------------------------------------*/

void
CStandardInputReader::ReadLoop( void )
{GUCEF_TRACE;

    GUCEF::vector< char > buffer( ReadBufferSize );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HANDLE threadHandle = NULL;
    ::DuplicateHandle( ::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), &threadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS );
    {
        MT::CScopeMutex lock( m_lock );
        m_threadHandle = threadHandle;
    }

    while ( !m_stopRequested )
    {
        DWORD bytesRead = 0;
        BOOL success = ::ReadFile( (HANDLE) m_stream, &buffer[ 0 ], (DWORD) buffer.size(), &bytesRead, NULL );
        if ( !success )
        {
            DWORD errorCode = ::GetLastError();
            if ( ERROR_OPERATION_ABORTED == errorCode )
                continue;           // Stop() cancelled the read
            if ( ERROR_BROKEN_PIPE == errorCode || ERROR_HANDLE_EOF == errorCode )
            {
                ProcessEndOfInput();
                break;
            }
            TReadErrorEventData eventData( "ReadFile() failed with error code " + UInt32ToString( (UInt32) errorCode ) );
            NotifyObservers( ReadErrorEvent, &eventData );
            break;
        }
        if ( 0 == bytesRead )
        {
            ProcessEndOfInput();
            break;
        }
        if ( !ProcessBytes( &buffer[ 0 ], (UInt32) bytesRead ) )
            return;         // the reader is gone, its members with it
    }

    {
        MT::CScopeMutex lock( m_lock );
        m_threadHandle = GUCEF_NULL;
    }
    if ( NULL != threadHandle )
        ::CloseHandle( threadHandle );

    #else

    while ( !m_stopRequested )
    {
        struct pollfd waitFor[ 2 ];
        waitFor[ 0 ].fd = m_stream;
        waitFor[ 0 ].events = POLLIN;
        waitFor[ 0 ].revents = 0;
        waitFor[ 1 ].fd = m_wakeRead;
        waitFor[ 1 ].events = POLLIN;
        waitFor[ 1 ].revents = 0;

        int ready = poll( waitFor, 2, -1 );
        if ( ready < 0 )
        {
            if ( EINTR == errno )
                continue;
            TReadErrorEventData eventData( CString( "poll() failed: " ) + strerror( errno ) );
            NotifyObservers( ReadErrorEvent, &eventData );
            return;
        }
        if ( 0 != waitFor[ 1 ].revents )
            return;                 // Stop()
        if ( 0 != ( waitFor[ 0 ].revents & POLLNVAL ) )
        {
            TReadErrorEventData eventData( "the input stream is not open" );
            NotifyObservers( ReadErrorEvent, &eventData );
            return;
        }
        if ( 0 == waitFor[ 0 ].revents )
            continue;

        ssize_t bytesRead = read( m_stream, &buffer[ 0 ], buffer.size() );
        if ( bytesRead < 0 )
        {
            if ( EINTR == errno || EAGAIN == errno )
                continue;
            TReadErrorEventData eventData( CString( "read() failed: " ) + strerror( errno ) );
            NotifyObservers( ReadErrorEvent, &eventData );
            return;
        }
        if ( 0 == bytesRead )
        {
            ProcessEndOfInput();
            return;
        }
        if ( !ProcessBytes( &buffer[ 0 ], (UInt32) bytesRead ) )
            return;                 // the reader is gone, its members with it
    }

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
