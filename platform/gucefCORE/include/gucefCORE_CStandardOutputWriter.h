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

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
 *  Writes lines to the standard output of the process, or to another stream such as a pipe, on a thread of its own.
 *
 *  WriteLine() never blocks: the line goes into a queue of at most maxQueuedBytes bytes, and the writer thread
 *  writes each line whole, its text followed by '\n', retrying partial writes. A reader that stops reading fills the
 *  pipe and then the queue, after which WriteLine() refuses lines.
 *
 *  When the reading end is gone (EPIPE), or another write error happens, BrokenPipeEvent is raised on the writer thread
 *  and every later WriteLine() fails. The writer thread blocks SIGPIPE for itself, so a closed pipe never ends the
 *  process through the signal; the process-wide disposition is left alone unless SetSigPipeIgnoredProcessWide() is used.
 *
 *  When stdout carries a protocol, ClaimExclusive() tells the rest of the process, such as the console loggers,
 *  to stay off it.
 */
class GUCEF_CORE_PUBLIC_CPP CStandardOutputWriter : public CNotifier
{
    public:

    static const CEvent BrokenPipeEvent;        /**< TCloneableString, what went wrong; the writer stops writing */

    typedef TCloneableString TBrokenPipeEventData;

    static void RegisterEvents( void );

    static const CString ClassTypeName;

    static const UInt32 DefaultMaxQueuedBytes = 64 * 1024 * 1024;

    static const UInt32 DefaultFlushTimeoutInMs = 2000;

    /** The native handle of a stream: a file descriptor, or on MS Windows a HANDLE */
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    typedef void* TStreamHandle;
    #else
    typedef int TStreamHandle;
    #endif

    /**
     *  Process wide: stdout is a protocol channel from now on. Loggers stay off it, see CPlatformNativeConsoleLogger.
     *  @return false when it was claimed already
     */
    static bool ClaimExclusive( void );

    static bool IsClaimed( void );

    /** Ends a claim made with ClaimExclusive() */
    static void ReleaseClaim( void );

    /**
     *  The explicit process-wide setting for SIGPIPE, for applications that write to pipes and sockets elsewhere too:
     *  true sets the disposition to ignored, false back to the default. The writer itself does not need it.
     *  @return false where the platform has no SIGPIPE or the disposition cannot be changed
     */
    static bool SetSigPipeIgnoredProcessWide( bool ignore );

    /** Writes to the standard output of the process */
    explicit CStandardOutputWriter( UInt32 maxQueuedBytes = DefaultMaxQueuedBytes );

    /**
     *  Writes to the given stream instead, for example the write end of a pipe. The stream is not owned: it is not
     *  closed, and it has to stay open while the writer runs
     */
    CStandardOutputWriter( UInt32 maxQueuedBytes, TStreamHandle stream );

    /** Stops the writer thread without flushing */
    virtual ~CStandardOutputWriter() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the writer thread. False when it runs already or the thread cannot be started */
    bool Start( void );

    /**
     *  Stops the writer thread.
     *  @param flush             write what is queued first, for at most flushTimeoutInMs
     *  @param flushTimeoutInMs  what is still queued after this is dropped, so a reader that stopped reading cannot hold up the stop
     */
    void Stop( bool flush, UInt32 flushTimeoutInMs = DefaultFlushTimeoutInMs );

    /**
     *  Queues one line, without its line end. Thread safe, never blocks.
     *  @return false when the writer is not started, the line holds a '\n', the queue is full or the stream broke
     */
    bool WriteLine( const CString& line );

    /** True once the stream broke, see BrokenPipeEvent */
    bool IsBroken( void ) const;

    /** The bytes queued and not written yet */
    UInt32 GetQueuedBytes( void ) const;

    UInt32 GetMaxQueuedBytes( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CWriterThread;
    friend class CWriterThread;

    /** Runs on the writer thread until Stop() or a broken stream */
    void WriteLoop( void );

    /** Writes all of the given bytes. @return false when the stream broke or the stop does not wait for them */
    bool WriteAll( const char* data, UInt32 size );

    /** Marks the stream as broken and raises BrokenPipeEvent */
    void OnBroken( const CString& reason );

    /** True when the thread should give up on what is left: a stop without flush, or after the flush timeout */
    bool ShouldAbandonWriting( void ) const;

    void WakeWriterThread( void );

    CStandardOutputWriter( const CStandardOutputWriter& src );            /**< not implemented */
    CStandardOutputWriter& operator=( const CStandardOutputWriter& src ); /**< not implemented */

    private:

    UInt32 m_maxQueuedBytes;
    TStreamHandle m_stream;
    bool m_isStandardOutput;
    CWriterThread* m_thread;
    bool m_started;
    CDynamicBuffer m_queue;                 /**< lines waiting for the writer thread, each with its '\n' */
    UInt32 m_queuedBytes;                   /**< the queue plus what the writer thread is still busy with */
    MT::CTAtomic< bool > m_broken;
    MT::CTAtomic< bool > m_stopRequested;
    bool m_flushOnStop;
    UInt64 m_flushDeadlineInMs;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    void* m_wakeEvent;                      /**< signalled when there is something to write or the writer has to stop */
    void* m_threadHandle;                   /**< the writer thread, for cancelling a blocking write */
    #else
    int m_wakeRead;                         /**< a byte written to the wake pipe ends a wait of the writer thread */
    int m_wakeWrite;
    #endif
    MT::CMutex m_queueLock;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */
