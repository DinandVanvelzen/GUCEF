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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CProcessPipe.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#else
  #include <unistd.h>
  #include <poll.h>
  #include <signal.h>
  #include <pthread.h>
  #include <time.h>
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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

namespace {

/**
 *  Blocks SIGPIPE for the calling thread while it exists, so that a write to a pipe of which the reader is gone fails
 *  with EPIPE instead of ending the process, and takes away a SIGPIPE such a write left pending
 */
class CPipeWriteSigPipeGuard
{
    public:

    CPipeWriteSigPipeGuard( void )
    {
        sigemptyset( &m_pipeSet );
        sigaddset( &m_pipeSet, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &m_pipeSet, &m_oldMask );
    }

    ~CPipeWriteSigPipeGuard()
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

    CPipeWriteSigPipeGuard( const CPipeWriteSigPipeGuard& src );
    CPipeWriteSigPipeGuard& operator=( const CPipeWriteSigPipeGuard& src );

    sigset_t m_pipeSet;
    sigset_t m_oldMask;
};

}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CProcessPipe::TNativeHandle
CProcessPipe::GetInvalidHandle( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return GUCEF_NULL;
    #else
    return -1;
    #endif
}

/*-------------------------------------------------------------------------*/

CProcessPipe::CProcessPipe( TNativeHandle handle, EDirection direction )
    : CIOAccess()
    , m_handle( handle )
    , m_direction( direction )
    , m_atEnd( false )
    , m_bytesTransferred( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProcessPipe::~CProcessPipe()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

CProcessPipe::EDirection
CProcessPipe::GetDirection( void ) const
{GUCEF_TRACE;

    return m_direction;
}

/*-------------------------------------------------------------------------*/

CProcessPipe::TNativeHandle
CProcessPipe::GetNativeHandle( void ) const
{GUCEF_TRACE;

    return m_handle;
}

/*-------------------------------------------------------------------------*/

void
CProcessPipe::Open( void )
{GUCEF_TRACE;

    // a pipe end is open from the moment it exists, and cannot be opened again
}

/*-------------------------------------------------------------------------*/

void
CProcessPipe::Close( void )
{GUCEF_TRACE;

    if ( GetInvalidHandle() != m_handle )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        ::CloseHandle( (HANDLE) m_handle );
        #else
        ::close( m_handle );
        #endif
        m_handle = GetInvalidHandle();
    }
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::Opened( void ) const
{GUCEF_TRACE;

    return GetInvalidHandle() != m_handle;
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::WaitForData( UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( GetInvalidHandle() == m_handle || PIPE_READ_FROM_CHILD != m_direction )
        return false;
    if ( m_atEnd )
        return true;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // an anonymous pipe cannot be waited on: look at it until the deadline. A broken pipe counts as readable, since a read then ends at once
    const UInt32 stepInMs = 2;
    UInt32 waitedInMs = 0;
    for ( ; ; )
    {
        DWORD available = 0;
        if ( !::PeekNamedPipe( (HANDLE) m_handle, GUCEF_NULL, 0, GUCEF_NULL, &available, GUCEF_NULL ) )
            return true;
        if ( available > 0 )
            return true;
        if ( waitedInMs >= timeoutInMs )
            return false;
        MT::ThreadDelay( stepInMs );
        waitedInMs += stepInMs;
    }

    #else

    struct pollfd descriptor;
    descriptor.fd = m_handle;
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    UInt64 remainingInMs = timeoutInMs;
    for ( ; ; )
    {
        int result = ::poll( &descriptor, 1, (int) ( remainingInMs > 100 ? 100 : remainingInMs ) );
        if ( result > 0 )
            return true;                // data, or POLLHUP / POLLERR: a read will not block
        if ( result < 0 && EINTR != errno )
            return true;                // an error: a read will tell
        if ( remainingInMs <= 100 )
            return false;
        remainingInMs -= 100;
    }

    #endif
}

/*-------------------------------------------------------------------------*/

UInt32
CProcessPipe::ReadSome( void* dest, UInt32 maxBytes )
{GUCEF_TRACE;

    if ( GetInvalidHandle() == m_handle || PIPE_READ_FROM_CHILD != m_direction || 0 == maxBytes || m_atEnd )
        return 0;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    DWORD bytesRead = 0;
    if ( !::ReadFile( (HANDLE) m_handle, dest, (DWORD) maxBytes, &bytesRead, GUCEF_NULL ) || 0 == bytesRead )
    {
        m_atEnd = true;                 // ERROR_BROKEN_PIPE is the normal end of a pipe
        return 0;
    }
    m_bytesTransferred += bytesRead;
    return (UInt32) bytesRead;

    #else

    for ( ; ; )
    {
        ssize_t result = ::read( m_handle, dest, maxBytes );
        if ( result > 0 )
        {
            m_bytesTransferred += (UInt64) result;
            return (UInt32) result;
        }
        if ( result < 0 && EINTR == errno )
            continue;
        m_atEnd = true;
        return 0;
    }

    #endif
}

/*-------------------------------------------------------------------------*/

UInt32
CProcessPipe::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( 0 == esize || 0 == elements )
        return 0;
    const UInt64 total = (UInt64) esize * elements;
    if ( total > GUCEF_UINT32MAX )
        return 0;

    char* out = static_cast< char* >( dest );
    UInt32 done = 0;
    while ( done < (UInt32) total )
    {
        UInt32 got = ReadSome( out + done, (UInt32) total - done );
        if ( 0 == got )
            break;
        done += got;
    }
    return done / esize;
}

/*-------------------------------------------------------------------------*/

UInt32
CProcessPipe::Write( const void* srcdata, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( GetInvalidHandle() == m_handle || PIPE_WRITE_TO_CHILD != m_direction || 0 == esize || 0 == elements )
        return 0;
    const UInt64 total = (UInt64) esize * elements;
    if ( total > GUCEF_UINT32MAX )
        return 0;

    const char* data = static_cast< const char* >( srcdata );
    UInt32 done = 0;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    CPipeWriteSigPipeGuard sigPipeGuard;
    #endif

    while ( done < (UInt32) total )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        DWORD written = 0;
        if ( !::WriteFile( (HANDLE) m_handle, data + done, (DWORD) ( (UInt32) total - done ), &written, GUCEF_NULL ) || 0 == written )
            break;
        done += (UInt32) written;

        #else

        ssize_t result = ::write( m_handle, data + done, (UInt32) total - done );
        if ( result < 0 && EINTR == errno )
            continue;
        if ( result <= 0 )
            break;                      // EPIPE: the child closed its end
        done += (UInt32) result;

        #endif
    }
    m_bytesTransferred += done;
    return done / esize;
}

/*-------------------------------------------------------------------------*/

UInt64
CProcessPipe::Tell( void ) const
{GUCEF_TRACE;

    return m_bytesTransferred;
}

/*-------------------------------------------------------------------------*/

Int32
CProcessPipe::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    return -1;
}

/*-------------------------------------------------------------------------*/

char
CProcessPipe::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    if ( 1 == Read( &c, 1, 1 ) )
        return c;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::Eof( void ) const
{GUCEF_TRACE;

    return m_atEnd;
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::IsReadable( void ) const
{GUCEF_TRACE;

    return Opened() && PIPE_READ_FROM_CHILD == m_direction;
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::IsWriteable( void ) const
{GUCEF_TRACE;

    return Opened() && PIPE_WRITE_TO_CHILD == m_direction;
}

/*-------------------------------------------------------------------------*/

bool
CProcessPipe::IsValid( void )
{GUCEF_TRACE;

    return Opened();
}

/*-------------------------------------------------------------------------*/

void
CProcessPipe::Flush( void )
{GUCEF_TRACE;

    // writes go straight to the pipe
}

/*-------------------------------------------------------------------------*/

CICloneable*
CProcessPipe::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
