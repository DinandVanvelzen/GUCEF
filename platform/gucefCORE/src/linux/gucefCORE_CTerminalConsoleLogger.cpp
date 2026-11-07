/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefCORE_CTerminalConsoleLogger.h"

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

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

/**
 *  Blocks SIGPIPE for the calling thread while it exists, when asked to, and takes away the signal that a write to a
 *  closed pipe left pending, so such a write fails with EPIPE instead of ending the application
 */
class CThreadSigPipeBlockScope
{
    public:

    explicit CThreadSigPipeBlockScope( bool block )
        : m_blocked( block )
        , m_wasPending( false )
    {
        if ( !m_blocked )
            return;
        sigemptyset( &m_pipeSet );
        sigaddset( &m_pipeSet, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &m_pipeSet, &m_oldMask );
        sigset_t pending;
        sigemptyset( &pending );
        sigpending( &pending );
        m_wasPending = 0 != sigismember( &pending, SIGPIPE );
    }

    ~CThreadSigPipeBlockScope()
    {
        if ( !m_blocked )
            return;
        if ( !m_wasPending )
        {
            sigset_t pending;
            sigemptyset( &pending );
            sigpending( &pending );
            if ( 0 != sigismember( &pending, SIGPIPE ) )
            {
                struct timespec noWait;
                noWait.tv_sec = 0;
                noWait.tv_nsec = 0;
                sigtimedwait( &m_pipeSet, NULL, &noWait );
            }
        }
        pthread_sigmask( SIG_SETMASK, &m_oldMask, NULL );
    }

    private:

    CThreadSigPipeBlockScope( const CThreadSigPipeBlockScope& src );
    CThreadSigPipeBlockScope& operator=( const CThreadSigPipeBlockScope& src );

    bool m_blocked;
    bool m_wasPending;
    sigset_t m_pipeSet;
    sigset_t m_oldMask;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CTerminalConsoleLogger::IsRunningInTerminal( void )
{GUCEF_TRACE;

    return -1 != GetTerminalFileDescriptor();
}

/*-------------------------------------------------------------------------*/

int
CTerminalConsoleLogger::GetTerminalFileDescriptor( void )
{GUCEF_TRACE;

    // stdout is never the terminal while it is claimed as a protocol channel
    if ( 0 != isatty( STDOUT_FILENO ) && !CStandardOutputWriter::IsClaimed() )
        return STDOUT_FILENO;
    if ( 0 != isatty( STDERR_FILENO ) )
        return STDERR_FILENO;
    return -1;
}

/*-------------------------------------------------------------------------*/

CTerminalConsoleLogger::CTerminalConsoleLogger( int fileDescriptor )
    : CTextLineConsoleLogger()
    , m_fileDescriptor( fileDescriptor )
    , m_droppedLines( 0 )
    , m_notATerminal( false )
    , m_lock()
{GUCEF_TRACE;

    if ( -1 == m_fileDescriptor )
        m_fileDescriptor = GetTerminalFileDescriptor();
    m_notATerminal = -1 != m_fileDescriptor && 0 == isatty( m_fileDescriptor );
}

/*-------------------------------------------------------------------------*/

CTerminalConsoleLogger::~CTerminalConsoleLogger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CTerminalConsoleLogger::GetDroppedLineCount( void ) const
{GUCEF_TRACE;

    return m_droppedLines;
}

/*-------------------------------------------------------------------------*/

void
CTerminalConsoleLogger::WriteLine( const CString& line )
{GUCEF_TRACE;

    if ( -1 == m_fileDescriptor )
        return;

    // A logger made before stdout was claimed as a protocol channel goes to stderr from then on
    int fileDescriptor = m_fileDescriptor;
    bool notATerminal = m_notATerminal;
    if ( STDOUT_FILENO == fileDescriptor && CStandardOutputWriter::IsClaimed() )
    {
        fileDescriptor = STDERR_FILENO;
        notATerminal = 0 == isatty( STDERR_FILENO );
    }

    CString text = line + "\n";
    const char* data = text.C_String();
    size_t remaining = text.ByteSize() - 1;

    // Lines of different threads must not end up in the middle of each other
    MT::CScopeMutex lock( m_lock );

    // A pipe whose reader is gone must give an error, not a SIGPIPE that ends the application
    CThreadSigPipeBlockScope sigPipeBlock( notATerminal );

    while ( remaining > 0 )
    {
        // Only write what the terminal accepts at once, if it does not accept anything it is stalled
        struct pollfd pollInfo;
        pollInfo.fd = fileDescriptor;
        pollInfo.events = POLLOUT;
        pollInfo.revents = 0;
        int pollResult = poll( &pollInfo, 1, 0 );
        if ( 1 != pollResult || 0 == ( pollInfo.revents & POLLOUT ) )
        {
            ++m_droppedLines;
            return;
        }

        ssize_t written = write( fileDescriptor, data, remaining );
        if ( written < 0 )
        {
            if ( EINTR == errno )
                continue;
            ++m_droppedLines;
            return;
        }
        data += written;
        remaining -= (size_t) written;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
