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

#ifndef GUCEF_CORE_CSTANDARDINPUTREADER_H
#define GUCEF_CORE_CSTANDARDINPUTREADER_H

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
 *  Reads lines from the standard input of the process, or from another stream such as a pipe, on a thread of its own.
 *
 *  Lines end with '\n'; a '\r' in front of it is dropped. A line longer than maxLineBytes is skipped up to its '\n'
 *  and reported with LineTooLongEvent instead. The end of the input, and a last line without a line end before it,
 *  are reported when the writer closes its end.
 *
 *  The events are raised on the reader thread, in the order of the input. Subscribe with a CPumpedObserver to get them
 *  on the thread of a pulse generator. An observer must not destroy the reader from within an event handler.
 */
class GUCEF_CORE_PUBLIC_CPP CStandardInputReader : public CNotifier
{
    public:

    static const CEvent LineReceivedEvent;      /**< TCloneableString, one line without its line end */
    static const CEvent LineTooLongEvent;       /**< TCloneableUInt32, the size that was skipped */
    static const CEvent EndOfInputEvent;        /**< the writer closed its end of the stream */
    static const CEvent ReadErrorEvent;         /**< TCloneableString, what went wrong; the reader stops */

    typedef TCloneableString TLineReceivedEventData;
    typedef TCloneableUInt32 TLineTooLongEventData;
    typedef TCloneableString TReadErrorEventData;

    static void RegisterEvents( void );

    static const CString ClassTypeName;

    /** The native handle of a stream: a file descriptor, or on MS Windows a HANDLE */
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    typedef void* TStreamHandle;
    #else
    typedef int TStreamHandle;
    #endif

    /** Reads the standard input of the process */
    explicit CStandardInputReader( UInt32 maxLineBytes );

    /**
     *  Reads the given stream instead, for example the read end of a pipe. The stream is not owned: it is not closed,
     *  and it has to stay open while the reader runs
     */
    CStandardInputReader( UInt32 maxLineBytes, TStreamHandle stream );

    /** Stops the reader thread */
    virtual ~CStandardInputReader() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the reader thread. False when it runs already or the thread cannot be started */
    bool Start( void );

    /** Wakes and joins the reader thread. Lines that were not read yet stay in the stream. Not for use in an event handler */
    void Stop( void );

    /** True from Start() until Stop(), also after the end of the input or a read error ended the thread */
    bool IsStarted( void ) const;

    UInt32 GetMaxLineBytes( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CReaderThread;
    friend class CReaderThread;

    /** Runs on the reader thread until the end of the input, an error, or Stop() */
    void ReadLoop( void );

    /**
     *  Splits what was read into lines and raises the events for them.
     *  @return false when the reader was destroyed by an event handler
     */
    bool ProcessBytes( const char* data, UInt32 size );

    /** Raises the events for the end of the input. @return false when the reader was destroyed by an event handler */
    bool ProcessEndOfInput( void );

    bool DeliverLine( const char* data, UInt32 size );

    CStandardInputReader( const CStandardInputReader& src );            /**< not implemented */
    CStandardInputReader& operator=( const CStandardInputReader& src ); /**< not implemented */

    private:

    UInt32 m_maxLineBytes;
    TStreamHandle m_stream;
    bool m_isStandardInput;
    CReaderThread* m_thread;
    bool m_started;
    MT::CTAtomic< bool > m_stopRequested;
    CDynamicBuffer m_partialLine;           /**< the start of a line that continues in the next read */
    bool m_skippingLine;
    UInt64 m_skippedBytes;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    void* m_threadHandle;                   /**< the reader thread, for cancelling its blocking read */
    #else
    int m_wakeRead;                         /**< a byte written to the wake pipe ends the wait for input */
    int m_wakeWrite;
    #endif
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

#endif /* GUCEF_CORE_CSTANDARDINPUTREADER_H ? */
