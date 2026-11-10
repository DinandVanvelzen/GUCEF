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

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "gucefCORE_CX11Clipboard.h"

#ifdef GUCEF_LINUX_BUILD

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How long the serve thread waits on the connection before it looks at the stalled transfers again */
static const int SERVE_POLL_TIMEOUT_IN_MS = 250;

/** How long a requestor may take to delete the property before an outgoing INCR transfer is given up */
static const UInt64 INCR_TRANSFER_TIMEOUT_IN_MS = 5000;

/** How long taking the clipboard waits for the server time */
static const UInt32 SERVER_TIME_TIMEOUT_IN_MS = 1000;

/** The upper limit of the size of one chunk of data, below the maximum request size of the server */
static const UInt32 MAX_CHUNK_SIZE_IN_BYTES = 256 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CX11Clipboard::MaxTextSizeInBytes;
const UInt32 CX11Clipboard::DefaultReceiveTimeoutInMs;

CX11Clipboard* CX11Clipboard::g_instance = GUCEF_NULL;
MT::CMutex CX11Clipboard::g_instanceLock;

/** The connection whose X errors the error handler absorbs, and the handler that was installed before it */
static ::Display* g_errorHandlerDisplay = GUCEF_NULL;
static ::XErrorHandler g_previousErrorHandler = GUCEF_NULL;
static bool g_errorHandlerInstalled = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The X error handler: errors of the clipboard connection (for example BadWindow when a requestor window is destroyed
 *  during a transfer) are not fatal, those of every other connection go to the handler that was there before
 */
static int
OnX11Error( ::Display* display, ::XErrorEvent* error )
{GUCEF_TRACE;

    if ( display == g_errorHandlerDisplay && GUCEF_NULL != g_errorHandlerDisplay )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: X error " << (UInt32) error->error_code << " of request " <<
            (UInt32) error->request_code << " on resource " << (UInt64) error->resourceid << " ignored" );
        return 0;
    }
    if ( GUCEF_NULL != g_previousErrorHandler )
        return g_previousErrorHandler( display, error );
    return 0;
}

/*-------------------------------------------------------------------------*/

static void
InstallErrorHandler( ::Display* display )
{GUCEF_TRACE;

    ::XErrorHandler previous = ::XSetErrorHandler( OnX11Error );
    if ( previous != OnX11Error )
        g_previousErrorHandler = previous;
    g_errorHandlerDisplay = display;
    g_errorHandlerInstalled = true;
}

/*-------------------------------------------------------------------------*/

static void
RemoveErrorHandler( void )
{GUCEF_TRACE;

    if ( !g_errorHandlerInstalled )
        return;

    g_errorHandlerDisplay = GUCEF_NULL;
    ::XErrorHandler current = ::XSetErrorHandler( g_previousErrorHandler );
    if ( current != OnX11Error )
    {
        // Another handler was installed after ours: that one stays, ours only passes errors on from now on
        ::XSetErrorHandler( current );
    }
    else
    {
        g_previousErrorHandler = GUCEF_NULL;
    }
    g_errorHandlerInstalled = false;
}

/*-------------------------------------------------------------------------*/

/** Determines whether UTF-8 text is ASCII and whether every code point of it is at most U+00FF */
static void
ClassifyUtf8Text( const unsigned char* text, UInt32 byteCount, bool& isAscii, bool& isLatin1 )
{GUCEF_TRACE;

    isAscii = true;
    isLatin1 = true;
    UInt32 i = 0;
    while ( i < byteCount )
    {
        unsigned char c = text[ i ];
        if ( c < 0x80 )
        {
            ++i;
            continue;
        }
        isAscii = false;
        if ( ( 0xC2 == c || 0xC3 == c ) && i+1 < byteCount && 0x80 == ( text[ i+1 ] & 0xC0 ) )
        {
            i += 2;
            continue;
        }
        isLatin1 = false;
        return;
    }
}

/*-------------------------------------------------------------------------*/

/** Converts UTF-8 text of which ClassifyUtf8Text() said it is Latin-1 to ISO 8859-1 */
static bool
Utf8ToLatin1( const unsigned char* text, UInt32 byteCount, CDynamicBuffer& latin1 )
{GUCEF_TRACE;

    latin1.Clear( false );
    if ( 0 == byteCount )
        return true;
    if ( !latin1.SetBufferSize( byteCount, false ) )
        return false;

    unsigned char* out = static_cast< unsigned char* >( latin1.GetBufferPtr() );
    UInt32 outCount = 0;
    UInt32 i = 0;
    while ( i < byteCount )
    {
        unsigned char c = text[ i ];
        if ( c < 0x80 || i+1 >= byteCount )
        {
            out[ outCount++ ] = c;
            ++i;
        }
        else
        {
            out[ outCount++ ] = (unsigned char) ( ( ( c & 0x1F ) << 6 ) | ( text[ i+1 ] & 0x3F ) );
            i += 2;
        }
    }
    latin1.SetDataSize( outCount );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Converts ISO 8859-1 text to UTF-8 */
static bool
Latin1ToUtf8( const unsigned char* text, UInt32 byteCount, CDynamicBuffer& utf8 )
{GUCEF_TRACE;

    utf8.Clear( false );
    if ( 0 == byteCount )
        return true;
    if ( !utf8.SetBufferSize( byteCount * 2, false ) )
        return false;

    unsigned char* out = static_cast< unsigned char* >( utf8.GetBufferPtr() );
    UInt32 outCount = 0;
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        unsigned char c = text[ i ];
        if ( c < 0x80 )
        {
            out[ outCount++ ] = c;
        }
        else
        {
            out[ outCount++ ] = (unsigned char) ( 0xC0 | ( c >> 6 ) );
            out[ outCount++ ] = (unsigned char) ( 0x80 | ( c & 0x3F ) );
        }
    }
    utf8.SetDataSize( outCount );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The thread that serves the connection. The whole serve loop runs in one cycle, so that the external thread scope
 *  is made and destroyed on the thread itself
 */
class CX11Clipboard::CServeThread : public MT::CActiveObject
{
    public:

    explicit CServeThread( CX11Clipboard& clipboard )
        : MT::CActiveObject()
        , m_clipboard( clipboard )
    {GUCEF_TRACE;

    }

    virtual ~CServeThread() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return this;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CExternalThreadScope threadScope( "gucef.x11-clipboard" );
        m_clipboard.ServeLoop();
        return true;    // done, the thread ends
    }

    private:

    CServeThread( const CServeThread& src );
    CServeThread& operator=( const CServeThread& src );

    CX11Clipboard& m_clipboard;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CX11Clipboard*
CX11Clipboard::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_instanceLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CX11Clipboard();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_instanceLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CX11Clipboard::CX11Clipboard( void )
    : m_lock()
    , m_display( GUCEF_NULL )
    , m_window( 0 )
    , m_thread( GUCEF_NULL )
    , m_stopRequested( false )
    , m_hasLoggedConnectFailure( false )
    , m_atomClipboard( None )
    , m_atomTargets( None )
    , m_atomMultiple( None )
    , m_atomTimestamp( None )
    , m_atomIncr( None )
    , m_atomAtomPair( None )
    , m_atomUtf8String( None )
    , m_atomText( None )
    , m_atomTextPlainUtf8( None )
    , m_atomTextPlain( None )
    , m_atomDataProperty( None )
    , m_atomTimeProperty( None )
    , m_maxChunkSize( MAX_CHUNK_SIZE_IN_BYTES )
    , m_isOwner( false )
    , m_ownershipTime( CurrentTime )
    , m_text()
    , m_textIsAscii( true )
    , m_textIsLatin1( true )
    , m_timeReceived( false )
    , m_receivedTime( CurrentTime )
    , m_incrTransfers()
    , m_receiveState( RECEIVESTATE_IDLE )
    , m_receiveEnded( false )
    , m_receiveTarget( None )
    , m_received()
    , m_receiveProgress( 0 )
{GUCEF_TRACE;

    m_wakePipe[ 0 ] = -1;
    m_wakePipe[ 1 ] = -1;
}

/*-------------------------------------------------------------------------*/

CX11Clipboard::~CX11Clipboard()
{GUCEF_TRACE;

    Disconnect();
}

/*-------------------------------------------------------------------------*/

UInt64
CX11Clipboard::MonotonicTimeInMs( void )
{GUCEF_TRACE;

    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return ( (UInt64) now.tv_sec * 1000 ) + ( (UInt64) now.tv_nsec / 1000000 );
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::Connect( void )
{GUCEF_TRACE;

    // The caller holds m_lock
    if ( GUCEF_NULL != m_display )
        return true;

    const char* displayName = ::getenv( "DISPLAY" );
    if ( GUCEF_NULL == displayName || '\0' == *displayName )
    {
        if ( !m_hasLoggedConnectFailure )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: DISPLAY is not set, there is no X11 clipboard to use" );
            m_hasLoggedConnectFailure = true;
        }
        return false;
    }

    ::Display* display = ::XOpenDisplay( GUCEF_NULL );
    if ( GUCEF_NULL == display )
    {
        if ( !m_hasLoggedConnectFailure )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: Cannot connect to the X server of DISPLAY \"" << displayName <<
                "\", there is no X11 clipboard to use" );
            m_hasLoggedConnectFailure = true;
        }
        return false;
    }

    if ( 0 != ::pipe2( m_wakePipe, O_CLOEXEC | O_NONBLOCK ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: Cannot create the wake-up pipe: " << ::strerror( errno ) );
        m_wakePipe[ 0 ] = -1;
        m_wakePipe[ 1 ] = -1;
        ::XCloseDisplay( display );
        return false;
    }
    ::fcntl( ConnectionNumber( display ), F_SETFD, FD_CLOEXEC );

    InstallErrorHandler( display );

    ::XSetWindowAttributes attributes;
    memset( &attributes, 0, sizeof( attributes ) );
    attributes.event_mask = PropertyChangeMask;
    m_window = ::XCreateWindow( display, DefaultRootWindow( display ), -10, -10, 1, 1, 0, 0, InputOnly, CopyFromParent,
                                CWEventMask, &attributes );

    char* atomNames[] = { (char*) "CLIPBOARD", (char*) "TARGETS", (char*) "MULTIPLE", (char*) "TIMESTAMP", (char*) "INCR",
                          (char*) "ATOM_PAIR", (char*) "UTF8_STRING", (char*) "TEXT", (char*) "text/plain;charset=utf-8",
                          (char*) "text/plain", (char*) "GUCEF_CLIPBOARD_DATA", (char*) "GUCEF_CLIPBOARD_TIME" };
    ::Atom atoms[ 12 ];
    memset( atoms, 0, sizeof( atoms ) );
    ::XInternAtoms( display, atomNames, 12, False, atoms );
    m_atomClipboard = atoms[ 0 ];
    m_atomTargets = atoms[ 1 ];
    m_atomMultiple = atoms[ 2 ];
    m_atomTimestamp = atoms[ 3 ];
    m_atomIncr = atoms[ 4 ];
    m_atomAtomPair = atoms[ 5 ];
    m_atomUtf8String = atoms[ 6 ];
    m_atomText = atoms[ 7 ];
    m_atomTextPlainUtf8 = atoms[ 8 ];
    m_atomTextPlain = atoms[ 9 ];
    m_atomDataProperty = atoms[ 10 ];
    m_atomTimeProperty = atoms[ 11 ];

    // XMaxRequestSize() is in units of 4 bytes; keep room for the request header
    long maxRequestBytes = ::XMaxRequestSize( display ) * 4 - 1024;
    m_maxChunkSize = MAX_CHUNK_SIZE_IN_BYTES;
    if ( maxRequestBytes > 0 && (UInt64) maxRequestBytes < (UInt64) m_maxChunkSize )
        m_maxChunkSize = (UInt32) maxRequestBytes;

    ::XFlush( display );
    m_display = display;
    m_stopRequested = false;

    m_thread = GUCEF_NEW CServeThread( *this );
    if ( !m_thread->Activate( GUCEF_NULL, 0, 0 ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: Cannot start the thread that serves the clipboard" );
        GUCEF_DELETE m_thread;
        m_thread = GUCEF_NULL;
        ::XDestroyWindow( m_display, m_window );
        ::XCloseDisplay( m_display );
        m_display = GUCEF_NULL;
        m_window = 0;
        RemoveErrorHandler();
        ::close( m_wakePipe[ 0 ] );
        ::close( m_wakePipe[ 1 ] );
        m_wakePipe[ 0 ] = -1;
        m_wakePipe[ 1 ] = -1;
        return false;
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "CX11Clipboard: Connected to the X server of DISPLAY \"" << displayName <<
        "\", chunk size " << m_maxChunkSize << " bytes" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::Disconnect( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_stopRequested = true;
    }
    WakeServeThread();

    if ( GUCEF_NULL != m_thread )
    {
        m_thread->Deactivate( false, true );
        GUCEF_DELETE m_thread;
        m_thread = GUCEF_NULL;
    }

    MT::CScopeMutex lock( m_lock );

    for ( TIncrTransferVector::iterator i = m_incrTransfers.begin(); i != m_incrTransfers.end(); ++i )
        GUCEF_DELETE (*i);
    m_incrTransfers.clear();

    if ( GUCEF_NULL != m_display )
    {
        // Destroying the window ends the ownership of the selection in the server
        ::XDestroyWindow( m_display, m_window );
        ::XCloseDisplay( m_display );
        m_display = GUCEF_NULL;
        m_window = 0;
        RemoveErrorHandler();
    }
    if ( m_wakePipe[ 0 ] >= 0 )
        ::close( m_wakePipe[ 0 ] );
    if ( m_wakePipe[ 1 ] >= 0 )
        ::close( m_wakePipe[ 1 ] );
    m_wakePipe[ 0 ] = -1;
    m_wakePipe[ 1 ] = -1;

    m_isOwner = false;
    m_text.Clear( false );
    m_received.Clear( false );
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::WakeServeThread( void )
{GUCEF_TRACE;

    if ( m_wakePipe[ 1 ] >= 0 )
    {
        char wake = 1;
        ssize_t written = ::write( m_wakePipe[ 1 ], &wake, 1 );  // a full pipe already wakes the thread
        (void) written;
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::ServeLoop( void )
{GUCEF_TRACE;

    while ( true )
    {
        int connectionFd = -1;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_stopRequested || GUCEF_NULL == m_display )
                break;

            ProcessPendingEvents();
            ExpireStalledIncrTransfers();
            ::XFlush( m_display );
            connectionFd = ConnectionNumber( m_display );
        }

        struct pollfd waitFor[ 2 ];
        waitFor[ 0 ].fd = connectionFd;
        waitFor[ 0 ].events = POLLIN;
        waitFor[ 0 ].revents = 0;
        waitFor[ 1 ].fd = m_wakePipe[ 0 ];
        waitFor[ 1 ].events = POLLIN;
        waitFor[ 1 ].revents = 0;

        int ready = ::poll( waitFor, 2, SERVE_POLL_TIMEOUT_IN_MS );
        if ( ready < 0 && EINTR != errno )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: poll() failed: " << ::strerror( errno ) );
            struct timespec pause;
            pause.tv_sec = 0;
            pause.tv_nsec = SERVE_POLL_TIMEOUT_IN_MS * 1000000L;
            ::nanosleep( &pause, GUCEF_NULL );
        }
        if ( ready > 0 && 0 != ( waitFor[ 1 ].revents & POLLIN ) )
        {
            char drain[ 64 ];
            while ( ::read( m_wakePipe[ 0 ], drain, sizeof( drain ) ) > 0 ) {}
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::ProcessPendingEvents( void )
{GUCEF_TRACE;

    // The caller holds m_lock. XPending() also flushes the output buffer and reads what the socket has
    while ( ::XPending( m_display ) > 0 )
    {
        ::XEvent event;
        ::XNextEvent( m_display, &event );
        ProcessEvent( event );
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::ProcessEvent( ::XEvent& event )
{GUCEF_TRACE;

    switch ( event.type )
    {
        case SelectionRequest: { OnSelectionRequest( event.xselectionrequest ); break; }
        case SelectionClear: { OnSelectionClear( event.xselectionclear ); break; }
        case SelectionNotify: { OnSelectionNotify( event.xselection ); break; }
        case PropertyNotify: { OnPropertyNotify( event.xproperty ); break; }
        case DestroyNotify: { OnDestroyNotify( event.xdestroywindow ); break; }
        default: { break; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::PumpEventsUntil( const bool& condition, UInt32 timeoutInMs, const UInt32* progressCounter )
{GUCEF_TRACE;

    // The caller holds m_lock: the events of the connection are processed on the calling thread meanwhile
    UInt64 deadline = MonotonicTimeInMs() + timeoutInMs;
    UInt32 lastProgress = GUCEF_NULL != progressCounter ? *progressCounter : 0;
    while ( true )
    {
        ProcessPendingEvents();
        if ( condition )
            return true;

        UInt64 now = MonotonicTimeInMs();
        if ( GUCEF_NULL != progressCounter && *progressCounter != lastProgress )
        {
            lastProgress = *progressCounter;
            deadline = now + timeoutInMs;
        }
        if ( now >= deadline )
            return false;

        struct pollfd waitFor;
        waitFor.fd = ConnectionNumber( m_display );
        waitFor.events = POLLIN;
        waitFor.revents = 0;
        ::poll( &waitFor, 1, (int) ( deadline - now ) );
    }
}

/*-------------------------------------------------------------------------*/

::Time
CX11Clipboard::AcquireServerTime( void )
{GUCEF_TRACE;

    // Appending nothing to a property of our own window makes the server send a PropertyNotify with its current time
    unsigned char nothing = 0;
    m_timeReceived = false;
    m_receivedTime = CurrentTime;
    ::XChangeProperty( m_display, m_window, m_atomTimeProperty, XA_INTEGER, 8, PropModeAppend, &nothing, 0 );
    if ( PumpEventsUntil( m_timeReceived, SERVER_TIME_TIMEOUT_IN_MS, GUCEF_NULL ) )
        return m_receivedTime;

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: The X server did not send its time, the clipboard is taken with CurrentTime" );
    return CurrentTime;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::SetText( const CString& text )
{GUCEF_TRACE;

    CUtf8String utf8Text = ToUtf8String( text );
    UInt32 byteCount = utf8Text.ByteSize() > 0 ? utf8Text.ByteSize() - 1 : 0;
    return SetUtf8Text( utf8Text.C_String(), byteCount );
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::SetUtf8Text( const char* utf8Text, UInt32 byteCount )
{GUCEF_TRACE;

    if ( GUCEF_NULL == utf8Text && byteCount > 0 )
        return false;
    if ( byteCount > MaxTextSizeInBytes )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: A text of " << byteCount << " bytes is larger than the maximum of " <<
            MaxTextSizeInBytes << " bytes, it is not put on the clipboard" );
        return false;
    }

    bool isOwner = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !Connect() )
            return false;

        m_text.Clear( false );
        if ( byteCount > 0 )
            m_text.Append( utf8Text, byteCount );
        ClassifyUtf8Text( (const unsigned char*) utf8Text, byteCount, m_textIsAscii, m_textIsLatin1 );

        ::Time time = AcquireServerTime();
        ::XSetSelectionOwner( m_display, m_atomClipboard, m_window, time );
        isOwner = ( m_window == ::XGetSelectionOwner( m_display, m_atomClipboard ) );
        m_isOwner = isOwner;
        m_ownershipTime = time;
        if ( !isOwner )
        {
            m_text.Clear( false );
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: The X server did not give the clipboard to this process" );
        }
    }
    WakeServeThread();
    return isOwner;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::GetText( CString& text, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CDynamicBuffer utf8Text;
    if ( !GetUtf8Text( utf8Text, timeoutInMs ) )
        return false;

    CUtf8String utf8String;
    if ( utf8Text.GetDataSize() > 0 )
        utf8String.Set( static_cast< const char* >( utf8Text.GetConstBufferPtr() ), utf8Text.GetDataSize() );
    text = ToString( utf8String );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::GetUtf8Text( CDynamicBuffer& utf8Text, UInt32 timeoutInMs )
{GUCEF_TRACE;

    utf8Text.Clear( false );
    bool success = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !Connect() )
            return false;

        ProcessPendingEvents();
        ::Window owner = ::XGetSelectionOwner( m_display, m_atomClipboard );
        if ( None == owner )
        {
            // nobody has put anything on the clipboard
        }
        else
        if ( owner == m_window && m_isOwner )
        {
            if ( m_text.GetDataSize() > 0 )
                utf8Text.Append( m_text.GetConstBufferPtr(), m_text.GetDataSize() );
            success = true;
        }
        else
        if ( owner != m_window )
        {
            if ( RequestConversion( m_atomUtf8String, timeoutInMs ) )
            {
                if ( m_received.GetDataSize() > 0 )
                    utf8Text.Append( m_received.GetConstBufferPtr(), m_received.GetDataSize() );
                success = true;
            }
            else
            if ( RECEIVESTATE_REFUSED == m_receiveState && RequestConversion( XA_STRING, timeoutInMs ) )
            {
                success = Latin1ToUtf8( static_cast< const unsigned char* >( m_received.GetConstBufferPtr() ), m_received.GetDataSize(), utf8Text );
            }
            m_receiveState = RECEIVESTATE_IDLE;
            m_received.Clear( false );
        }
    }
    WakeServeThread();
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::IsClipboardOwner( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_isOwner;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::HasDisplay( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return Connect();
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::RequestConversion( ::Atom target, UInt32 timeoutInMs )
{GUCEF_TRACE;

    // The caller holds m_lock
    m_received.Clear( false );
    m_receiveState = RECEIVESTATE_WAITING_FOR_NOTIFY;
    m_receiveEnded = false;
    m_receiveTarget = target;
    m_receiveProgress = 0;

    ::XDeleteProperty( m_display, m_window, m_atomDataProperty );
    ::XConvertSelection( m_display, m_atomClipboard, target, m_atomDataProperty, m_window, CurrentTime );

    if ( !PumpEventsUntil( m_receiveEnded, timeoutInMs, &m_receiveProgress ) )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: The owner of the clipboard did not answer within " << timeoutInMs << " ms" );
        m_receiveState = RECEIVESTATE_FAILED;
        ::XDeleteProperty( m_display, m_window, m_atomDataProperty );
    }
    return RECEIVESTATE_DONE == m_receiveState;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::ReadReceivedProperty( bool& isIncr, UInt32& byteCount )
{GUCEF_TRACE;

    // Reads and deletes the data property; deleting it is also what asks an INCR sender for the next part
    isIncr = false;
    byteCount = 0;

    ::Atom type = None;
    int format = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = GUCEF_NULL;
    int result = ::XGetWindowProperty( m_display, m_window, m_atomDataProperty, 0, ( MaxTextSizeInBytes / 4 ) + 1, True,
                                       AnyPropertyType, &type, &format, &itemCount, &bytesAfter, &data );
    if ( Success != result )
        return false;

    bool success = false;
    if ( type == m_atomIncr )
    {
        isIncr = true;
        success = true;
    }
    else
    if ( None != type && 8 == format && 0 == bytesAfter && m_received.GetDataSize() + itemCount <= MaxTextSizeInBytes )
    {
        if ( itemCount > 0 )
            m_received.Append( data, (UInt32) itemCount );
        byteCount = (UInt32) itemCount;
        success = true;
    }
    else
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: The clipboard data cannot be used (format " << format <<
            ", " << (UInt64) itemCount << " items, " << (UInt64) bytesAfter << " bytes more, maximum " << MaxTextSizeInBytes << " bytes)" );
        if ( 0 != bytesAfter )
            ::XDeleteProperty( m_display, m_window, m_atomDataProperty );
    }
    if ( GUCEF_NULL != data )
        ::XFree( data );
    return success;
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::OnSelectionNotify( const ::XSelectionEvent& notify )
{GUCEF_TRACE;

    if ( notify.requestor != m_window || notify.selection != m_atomClipboard ||
         RECEIVESTATE_WAITING_FOR_NOTIFY != m_receiveState || notify.target != m_receiveTarget )
        return;

    if ( None == notify.property )
    {
        m_receiveState = RECEIVESTATE_REFUSED;
        m_receiveEnded = true;
        return;
    }

    bool isIncr = false;
    UInt32 byteCount = 0;
    if ( !ReadReceivedProperty( isIncr, byteCount ) )
    {
        m_receiveState = RECEIVESTATE_FAILED;
        m_receiveEnded = true;
    }
    else
    if ( isIncr )
    {
        // The property was deleted by reading it, the owner now sends the first part
        m_receiveState = RECEIVESTATE_INCR;
        ++m_receiveProgress;
    }
    else
    {
        m_receiveState = RECEIVESTATE_DONE;
        m_receiveEnded = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::OnPropertyNotify( const ::XPropertyEvent& propertyEvent )
{GUCEF_TRACE;

    if ( propertyEvent.window == m_window )
    {
        if ( propertyEvent.atom == m_atomTimeProperty && PropertyNewValue == propertyEvent.state )
        {
            m_receivedTime = propertyEvent.time;
            m_timeReceived = true;
        }
        else
        if ( propertyEvent.atom == m_atomDataProperty && PropertyNewValue == propertyEvent.state &&
             RECEIVESTATE_INCR == m_receiveState )
        {
            bool isIncr = false;
            UInt32 byteCount = 0;
            if ( !ReadReceivedProperty( isIncr, byteCount ) || isIncr )
            {
                m_receiveState = RECEIVESTATE_FAILED;
                m_receiveEnded = true;
            }
            else
            if ( 0 == byteCount )
            {
                // a part without data ends the transfer
                m_receiveState = RECEIVESTATE_DONE;
                m_receiveEnded = true;
            }
            else
            {
                ++m_receiveProgress;
            }
        }
        return;
    }

    if ( PropertyDelete != propertyEvent.state )
        return;

    for ( TIncrTransferVector::iterator i = m_incrTransfers.begin(); i != m_incrTransfers.end(); ++i )
    {
        SIncrTransfer& transfer = *(*i);
        if ( transfer.requestor == propertyEvent.window && transfer.property == propertyEvent.atom )
        {
            if ( transfer.offset >= transfer.data.GetDataSize() )
            {
                // Everything was sent: a property without data ends the transfer
                unsigned char nothing = 0;
                ::XChangeProperty( m_display, transfer.requestor, transfer.property, transfer.type, 8, PropModeReplace, &nothing, 0 );
                EndIncrTransfer( i, true );
            }
            else
            {
                ContinueIncrTransfer( transfer );
            }
            ::XFlush( m_display );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::OnDestroyNotify( const ::XDestroyWindowEvent& destroyEvent )
{GUCEF_TRACE;

    TIncrTransferVector::iterator i = m_incrTransfers.begin();
    while ( i != m_incrTransfers.end() )
    {
        if ( (*i)->requestor == destroyEvent.window )
        {
            EndIncrTransfer( i, false );
            i = m_incrTransfers.begin();
        }
        else
        {
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::OnSelectionClear( const ::XSelectionClearEvent& clear )
{GUCEF_TRACE;

    if ( clear.selection != m_atomClipboard || !m_isOwner )
        return;

    // A clear older than our ownership belongs to an ownership that this process has replaced since
    if ( CurrentTime != m_ownershipTime && CurrentTime != clear.time && clear.time < m_ownershipTime )
        return;

    m_isOwner = false;
    m_text.Clear( false );
    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: Another client took the clipboard" );
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::OnSelectionRequest( const ::XSelectionRequestEvent& request )
{GUCEF_TRACE;

    ::XSelectionEvent notify;
    memset( &notify, 0, sizeof( notify ) );
    notify.type = SelectionNotify;
    notify.display = request.display;
    notify.requestor = request.requestor;
    notify.selection = request.selection;
    notify.target = request.target;
    notify.time = request.time;
    notify.property = None;

    // Obsolete clients pass no property: the target is then the property, as ICCCM prescribes
    ::Atom property = ( None == request.property ) ? request.target : request.property;

    bool serve = request.selection == m_atomClipboard && m_isOwner &&
                 ( CurrentTime == request.time || CurrentTime == m_ownershipTime || request.time >= m_ownershipTime );
    if ( serve )
    {
        if ( request.target == m_atomMultiple )
        {
            if ( None != request.property && ConvertMultiple( request.requestor, request.property ) )
                notify.property = request.property;
        }
        else
        if ( ConvertTarget( request.requestor, request.target, property, true ) )
        {
            notify.property = property;
        }
    }

    ::XSendEvent( m_display, request.requestor, False, NoEventMask, reinterpret_cast< ::XEvent* >( &notify ) );
    ::XFlush( m_display );
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::ConvertTarget( ::Window requestor, ::Atom target, ::Atom property, bool allowIncr )
{GUCEF_TRACE;

    const char* text = static_cast< const char* >( m_text.GetConstBufferPtr() );
    UInt32 textSize = m_text.GetDataSize();

    if ( target == m_atomTargets )
    {
        ::Atom targets[ 8 ];
        int count = 0;
        targets[ count++ ] = m_atomTargets;
        targets[ count++ ] = m_atomTimestamp;
        targets[ count++ ] = m_atomMultiple;
        targets[ count++ ] = m_atomUtf8String;
        targets[ count++ ] = m_atomTextPlainUtf8;
        targets[ count++ ] = m_atomText;
        if ( m_textIsLatin1 )
            targets[ count++ ] = XA_STRING;
        if ( m_textIsAscii )
            targets[ count++ ] = m_atomTextPlain;
        ::XChangeProperty( m_display, requestor, property, XA_ATOM, 32, PropModeReplace, (unsigned char*) targets, count );
        return true;
    }
    if ( target == m_atomTimestamp )
    {
        long time = (long) m_ownershipTime;
        ::XChangeProperty( m_display, requestor, property, XA_INTEGER, 32, PropModeReplace, (unsigned char*) &time, 1 );
        return true;
    }
    if ( target == m_atomUtf8String || target == m_atomTextPlainUtf8 )
    {
        return WriteData( requestor, property, target, text, textSize, allowIncr );
    }
    if ( target == XA_STRING )
    {
        if ( !m_textIsLatin1 )
            return false;
        CDynamicBuffer latin1;
        if ( !Utf8ToLatin1( (const unsigned char*) text, textSize, latin1 ) )
            return false;
        return WriteData( requestor, property, XA_STRING, static_cast< const char* >( latin1.GetConstBufferPtr() ), latin1.GetDataSize(), allowIncr );
    }
    if ( target == m_atomTextPlain )
    {
        if ( !m_textIsAscii )
            return false;
        return WriteData( requestor, property, m_atomTextPlain, text, textSize, allowIncr );
    }
    if ( target == m_atomText )
    {
        return WriteData( requestor, property, m_textIsAscii ? XA_STRING : m_atomUtf8String, text, textSize, allowIncr );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::ConvertMultiple( ::Window requestor, ::Atom property )
{GUCEF_TRACE;

    ::Atom type = None;
    int format = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = GUCEF_NULL;
    int result = ::XGetWindowProperty( m_display, requestor, property, 0, 1024, False, AnyPropertyType, &type, &format,
                                       &itemCount, &bytesAfter, &data );

    // ICCCM prescribes ATOM_PAIR, some clients write the list as ATOM
    if ( Success != result || ( type != m_atomAtomPair && type != XA_ATOM ) || 32 != format || 0 != ( itemCount % 2 ) )
    {
        if ( GUCEF_NULL != data )
            ::XFree( data );
        return false;
    }

    // Format 32 data comes as longs: pairs of target and property
    ::Atom* pairs = reinterpret_cast< ::Atom* >( data );
    for ( unsigned long i=0; i+1<itemCount; i+=2 )
    {
        if ( None == pairs[ i+1 ] || pairs[ i ] == m_atomMultiple ||
             !ConvertTarget( requestor, pairs[ i ], pairs[ i+1 ], false ) )
        {
            pairs[ i+1 ] = None;
        }
    }
    ::XChangeProperty( m_display, requestor, property, type, 32, PropModeReplace, data, (int) itemCount );
    ::XFree( data );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CX11Clipboard::WriteData( ::Window requestor, ::Atom property, ::Atom type, const char* data, UInt32 byteCount, bool allowIncr )
{GUCEF_TRACE;

    if ( byteCount <= m_maxChunkSize )
    {
        unsigned char nothing = 0;
        const unsigned char* bytes = byteCount > 0 ? (const unsigned char*) data : &nothing;
        ::XChangeProperty( m_display, requestor, property, type, 8, PropModeReplace, bytes, (int) byteCount );
        return true;
    }
    if ( !allowIncr )
        return false;

    // A new request for the same property replaces a transfer that is still running
    for ( TIncrTransferVector::iterator i = m_incrTransfers.begin(); i != m_incrTransfers.end(); ++i )
    {
        if ( (*i)->requestor == requestor && (*i)->property == property )
        {
            EndIncrTransfer( i, false );
            break;
        }
    }

    SIncrTransfer* transfer = GUCEF_NEW SIncrTransfer();
    transfer->requestor = requestor;
    transfer->property = property;
    transfer->type = type;
    transfer->data.Append( data, byteCount );
    transfer->offset = 0;
    transfer->lastActivityInMs = MonotonicTimeInMs();
    m_incrTransfers.push_back( transfer );

    // The deletions of the property by the requestor tell when to send the next part
    if ( requestor != m_window )
        ::XSelectInput( m_display, requestor, PropertyChangeMask | StructureNotifyMask );
    long size = (long) byteCount;
    ::XChangeProperty( m_display, requestor, property, m_atomIncr, 32, PropModeReplace, (unsigned char*) &size, 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::ContinueIncrTransfer( SIncrTransfer& transfer )
{GUCEF_TRACE;

    UInt32 remaining = transfer.data.GetDataSize() - transfer.offset;
    UInt32 chunkSize = remaining < m_maxChunkSize ? remaining : m_maxChunkSize;
    const unsigned char* chunk = static_cast< const unsigned char* >( transfer.data.GetConstBufferPtr( transfer.offset ) );
    ::XChangeProperty( m_display, transfer.requestor, transfer.property, transfer.type, 8, PropModeReplace, chunk, (int) chunkSize );
    transfer.offset += chunkSize;
    transfer.lastActivityInMs = MonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::EndIncrTransfer( TIncrTransferVector::iterator i, bool resetEventMask )
{GUCEF_TRACE;

    ::Window requestor = (*i)->requestor;
    GUCEF_DELETE (*i);
    m_incrTransfers.erase( i );

    if ( !resetEventMask || requestor == m_window )
        return;
    for ( TIncrTransferVector::iterator n = m_incrTransfers.begin(); n != m_incrTransfers.end(); ++n )
    {
        if ( (*n)->requestor == requestor )
            return;
    }
    ::XSelectInput( m_display, requestor, NoEventMask );
}

/*-------------------------------------------------------------------------*/

void
CX11Clipboard::ExpireStalledIncrTransfers( void )
{GUCEF_TRACE;

    UInt64 now = MonotonicTimeInMs();
    TIncrTransferVector::iterator i = m_incrTransfers.begin();
    while ( i != m_incrTransfers.end() )
    {
        if ( now - (*i)->lastActivityInMs > INCR_TRANSFER_TIMEOUT_IN_MS )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CX11Clipboard: A transfer of " << (*i)->data.GetDataSize() <<
                " bytes stalled after " << (*i)->offset << " bytes, it is given up" );
            EndIncrTransfer( i, true );
            i = m_incrTransfers.begin();
        }
        else
        {
            ++i;
        }
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

#endif /* GUCEF_LINUX_BUILD ? */
