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

#ifndef GUCEF_CORE_CX11CLIPBOARD_H
#define GUCEF_CORE_CX11CLIPBOARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* module config macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <X11/Xlib.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  The system clipboard of an X11 session: the CLIPBOARD selection.
 *
 *  Setting a text makes this process the owner of CLIPBOARD; every other X client that pastes asks this object for the
 *  text, as long as the process runs or until another client takes the clipboard. The object has its own connection to
 *  the X server (the one of $DISPLAY) with an unmapped input only window, and its own thread that answers the requests
 *  of other clients, so the application does not have to pump any X11 events and needs no X11 window of its own.
 *  Every Xlib call on that connection is made with the one mutex of the object held, so XInitThreads() is not needed.
 *
 *  Served targets: TARGETS, TIMESTAMP, MULTIPLE, UTF8_STRING, text/plain;charset=utf-8, TEXT, and STRING (Latin-1)
 *  and text/plain (ASCII) when the text is representable in them. Texts larger than one X request are sent and
 *  received with the INCR protocol. Only CLIPBOARD is owned, never PRIMARY: PRIMARY follows the highlighted text.
 *
 *  Without a display ($DISPLAY not set, or the server cannot be reached) every operation answers false; each use tries
 *  again to connect. This is the Linux implementation of StringToClipboard() and StringFromClipboard() of DVOSWRAP.h.
 */
class GUCEF_CORE_PUBLIC_CPP CX11Clipboard
{
    public:

    /** The largest text that is put on, or taken from, the clipboard: 16 MiB */
    static const UInt32 MaxTextSizeInBytes = 16 * 1024 * 1024;

    /** How long a read waits for the owner of the clipboard to answer (or to send the next part of a large text) */
    static const UInt32 DefaultReceiveTimeoutInMs = 1000;

    /** The object, created on the first call; it connects to the X server on the first Set or Get */
    static CX11Clipboard* Instance( void );

    /**
     *  Puts the text on the clipboard and makes this process the owner of it
     *  @return whether the X server confirmed the ownership; false without a display or for a text above MaxTextSizeInBytes
     *          (the previous content then stays)
     */
    bool SetText( const CString& text );

    /** As SetText(), for byteCount bytes of UTF-8 text (no terminator needed) */
    bool SetUtf8Text( const char* utf8Text, UInt32 byteCount );

    /**
     *  Gets the text of the clipboard: from memory when this process owns it, otherwise from the owner, waiting on the
     *  calling thread at most timeoutInMs for an answer (or for the next part of a large text)
     *  @return false when the clipboard is empty, the owner has no text or does not answer in time, or there is no display
     */
    bool GetText( CString& text, UInt32 timeoutInMs = DefaultReceiveTimeoutInMs );

    /** As GetText(), the text as UTF-8 bytes without terminator */
    bool GetUtf8Text( CDynamicBuffer& utf8Text, UInt32 timeoutInMs = DefaultReceiveTimeoutInMs );

    /** Whether this process owns the clipboard, as far as the events processed so far tell */
    bool IsClipboardOwner( void ) const;

    /** Whether there is a connection to the X server, making one if there is none yet */
    bool HasDisplay( void );

    private:
    friend class CCoreGlobal;

    static void Deinstance( void );

    private:

    class CServeThread;
    friend class CServeThread;

    /** An outgoing transfer of a large text with the INCR protocol */
    struct SIncrTransfer
    {
        ::Window requestor;
        ::Atom property;
        ::Atom type;
        CDynamicBuffer data;
        UInt32 offset;
        UInt64 lastActivityInMs;
    };
    typedef GUCEF::vector< SIncrTransfer* > TIncrTransferVector;

    enum EReceiveState
    {
        RECEIVESTATE_IDLE = 0,
        RECEIVESTATE_WAITING_FOR_NOTIFY,
        RECEIVESTATE_INCR,
        RECEIVESTATE_DONE,
        RECEIVESTATE_REFUSED,
        RECEIVESTATE_FAILED
    };

    CX11Clipboard( void );

    ~CX11Clipboard();

    bool Connect( void );

    void Disconnect( void );

    void ServeLoop( void );

    void WakeServeThread( void );

    void ProcessPendingEvents( void );

    void ProcessEvent( ::XEvent& event );

    bool PumpEventsUntil( const bool& condition, UInt32 timeoutInMs, const UInt32* progressCounter );

    ::Time AcquireServerTime( void );

    void OnSelectionRequest( const ::XSelectionRequestEvent& request );

    void OnSelectionClear( const ::XSelectionClearEvent& clear );

    void OnSelectionNotify( const ::XSelectionEvent& notify );

    void OnPropertyNotify( const ::XPropertyEvent& propertyEvent );

    void OnDestroyNotify( const ::XDestroyWindowEvent& destroyEvent );

    bool ConvertTarget( ::Window requestor, ::Atom target, ::Atom property, bool allowIncr );

    bool ConvertMultiple( ::Window requestor, ::Atom property );

    bool WriteData( ::Window requestor, ::Atom property, ::Atom type, const char* data, UInt32 byteCount, bool allowIncr );

    void ContinueIncrTransfer( SIncrTransfer& transfer );

    void EndIncrTransfer( TIncrTransferVector::iterator i, bool resetEventMask );

    void ExpireStalledIncrTransfers( void );

    bool RequestConversion( ::Atom target, UInt32 timeoutInMs );

    bool ReadReceivedProperty( bool& isIncr, UInt32& byteCount );

    static UInt64 MonotonicTimeInMs( void );

    private:

    CX11Clipboard( const CX11Clipboard& src );
    CX11Clipboard& operator=( const CX11Clipboard& src );

    private:

    static CX11Clipboard* g_instance;
    static MT::CMutex g_instanceLock;

    MT::CMutex m_lock;
    ::Display* m_display;
    ::Window m_window;
    int m_wakePipe[ 2 ];
    CServeThread* m_thread;
    bool m_stopRequested;
    bool m_hasLoggedConnectFailure;

    ::Atom m_atomClipboard;
    ::Atom m_atomTargets;
    ::Atom m_atomMultiple;
    ::Atom m_atomTimestamp;
    ::Atom m_atomIncr;
    ::Atom m_atomAtomPair;
    ::Atom m_atomUtf8String;
    ::Atom m_atomText;
    ::Atom m_atomTextPlainUtf8;
    ::Atom m_atomTextPlain;
    ::Atom m_atomDataProperty;
    ::Atom m_atomTimeProperty;
    UInt32 m_maxChunkSize;

    bool m_isOwner;
    ::Time m_ownershipTime;
    CDynamicBuffer m_text;
    bool m_textIsAscii;
    bool m_textIsLatin1;

    bool m_timeReceived;
    ::Time m_receivedTime;

    TIncrTransferVector m_incrTransfers;

    EReceiveState m_receiveState;
    bool m_receiveEnded;
    ::Atom m_receiveTarget;
    CDynamicBuffer m_received;
    UInt32 m_receiveProgress;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LINUX_BUILD ? */

#endif /* GUCEF_CORE_CX11CLIPBOARD_H ? */
