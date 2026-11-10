/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <poll.h>
#include <string.h>
#include <time.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestX11ClipboardPeer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
PeerMonotonicTimeInMs( void )
{GUCEF_TRACE;

    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return ( (UInt64) now.tv_sec * 1000 ) + ( (UInt64) now.tv_nsec / 1000000 );
}

/*-------------------------------------------------------------------------*/

/** Appends the data of a property read with XGetWindowProperty(): format 32 data are longs, as Xlib returns them */
static void
AppendPropertyData( CORE::CDynamicBuffer& data, const unsigned char* bytes, int format, unsigned long itemCount )
{GUCEF_TRACE;

    UInt32 itemSize = 1;
    if ( 16 == format )
        itemSize = sizeof( short );
    else
    if ( 32 == format )
        itemSize = sizeof( long );
    if ( itemCount > 0 && GUCEF_NULL != bytes )
        data.Append( bytes, (UInt32) ( itemCount * itemSize ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestX11ClipboardPeer::CTestX11ClipboardPeer( void )
    : MT::CActiveObject()
    , m_peerLock()
    , m_display( GUCEF_NULL )
    , m_window( 0 )
    , m_atomClipboard( None )
    , m_atomProperty( None )
    , m_ownText()
    , m_ownTextAsLatin1( false )
    , m_incrChunkSize( 0 )
    , m_lostClipboard( false )
    , m_sentIncr( false )
    , m_servedRequests( 0 )
    , m_incrRequestor( 0 )
    , m_incrProperty( None )
    , m_incrType( None )
    , m_incrOffset( 0 )
    , m_incrActive( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestX11ClipboardPeer::~CTestX11ClipboardPeer()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CTestX11ClipboardPeer::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::Open( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL != m_display )
        return true;

    m_display = ::XOpenDisplay( GUCEF_NULL );
    if ( GUCEF_NULL == m_display )
        return false;

    ::XSetWindowAttributes attributes;
    memset( &attributes, 0, sizeof( attributes ) );
    attributes.event_mask = PropertyChangeMask;
    m_window = ::XCreateWindow( m_display, DefaultRootWindow( m_display ), -10, -10, 1, 1, 0, 0, InputOnly, CopyFromParent,
                                CWEventMask, &attributes );
    m_atomClipboard = ::XInternAtom( m_display, "CLIPBOARD", False );
    m_atomProperty = ::XInternAtom( m_display, "GUCEF_TEST_CLIPBOARD_DATA", False );
    ::XFlush( m_display );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestX11ClipboardPeer::Close( void )
{GUCEF_TRACE;

    Deactivate( false, true );

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL != m_display )
    {
        ::XDestroyWindow( m_display, m_window );
        ::XCloseDisplay( m_display );
        m_display = GUCEF_NULL;
        m_window = 0;
    }
}

/*-------------------------------------------------------------------------*/

::Atom
CTestX11ClipboardPeer::GetAtom( const char* name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return None;
    return ::XInternAtom( m_display, name, False );
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::WaitForEvent( int eventType, ::Atom atom, ::XEvent& event, UInt32 timeoutInMs )
{GUCEF_TRACE;

    // The lock is held by the caller. Events for other purposes are not of interest while waiting and are dropped
    UInt64 deadline = PeerMonotonicTimeInMs() + timeoutInMs;
    while ( true )
    {
        while ( ::XPending( m_display ) > 0 )
        {
            ::XNextEvent( m_display, &event );
            if ( event.type != eventType )
                continue;
            if ( SelectionNotify == eventType && event.xselection.requestor == m_window )
                return true;
            if ( PropertyNotify == eventType && event.xproperty.window == m_window && event.xproperty.atom == atom &&
                 PropertyNewValue == event.xproperty.state )
                return true;
        }

        UInt64 now = PeerMonotonicTimeInMs();
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

CTestX11ClipboardPeer::EConvertResult
CTestX11ClipboardPeer::Convert( const char* targetName       ,
                                CORE::CDynamicBuffer& data   ,
                                ::Atom& type                 ,
                                int& format                  ,
                                bool& wasIncr                ,
                                UInt32 timeoutInMs           )
{GUCEF_TRACE;

    data.Clear( false );
    type = None;
    format = 0;
    wasIncr = false;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return CONVERTRESULT_FAILED;

    ::Atom target = ::XInternAtom( m_display, targetName, False );
    ::Atom incr = ::XInternAtom( m_display, "INCR", False );
    ::XDeleteProperty( m_display, m_window, m_atomProperty );
    ::XConvertSelection( m_display, m_atomClipboard, target, m_atomProperty, m_window, CurrentTime );
    ::XFlush( m_display );

    ::XEvent event;
    if ( !WaitForEvent( SelectionNotify, None, event, timeoutInMs ) )
        return CONVERTRESULT_TIMEOUT;
    if ( None == event.xselection.property )
        return CONVERTRESULT_REFUSED;

    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* bytes = GUCEF_NULL;
    if ( Success != ::XGetWindowProperty( m_display, m_window, event.xselection.property, 0, 0x7FFFFFF, True, AnyPropertyType,
                                          &type, &format, &itemCount, &bytesAfter, &bytes ) )
        return CONVERTRESULT_FAILED;

    if ( type != incr )
    {
        AppendPropertyData( data, bytes, format, itemCount );
        if ( GUCEF_NULL != bytes )
            ::XFree( bytes );
        return CONVERTRESULT_OK;
    }

    // INCR: the property was deleted by reading it, the parts follow, one property of zero length ends them
    if ( GUCEF_NULL != bytes )
        ::XFree( bytes );
    wasIncr = true;
    while ( true )
    {
        if ( !WaitForEvent( PropertyNotify, m_atomProperty, event, timeoutInMs ) )
            return CONVERTRESULT_TIMEOUT;

        bytes = GUCEF_NULL;
        if ( Success != ::XGetWindowProperty( m_display, m_window, m_atomProperty, 0, 0x7FFFFFF, True, AnyPropertyType,
                                              &type, &format, &itemCount, &bytesAfter, &bytes ) )
            return CONVERTRESULT_FAILED;

        bool done = ( 0 == itemCount );
        AppendPropertyData( data, bytes, format, itemCount );
        if ( GUCEF_NULL != bytes )
            ::XFree( bytes );
        if ( done )
            return CONVERTRESULT_OK;
    }
}

/*-------------------------------------------------------------------------*/

CTestX11ClipboardPeer::EConvertResult
CTestX11ClipboardPeer::BeginConvert( const char* targetName, bool& wasIncr )
{GUCEF_TRACE;

    wasIncr = false;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return CONVERTRESULT_FAILED;

    ::Atom target = ::XInternAtom( m_display, targetName, False );
    ::Atom incr = ::XInternAtom( m_display, "INCR", False );
    ::XDeleteProperty( m_display, m_window, m_atomProperty );
    ::XConvertSelection( m_display, m_atomClipboard, target, m_atomProperty, m_window, CurrentTime );
    ::XFlush( m_display );

    ::XEvent event;
    if ( !WaitForEvent( SelectionNotify, None, event, 2000 ) )
        return CONVERTRESULT_TIMEOUT;
    if ( None == event.xselection.property )
        return CONVERTRESULT_REFUSED;

    ::Atom type = None;
    int format = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* bytes = GUCEF_NULL;
    if ( Success != ::XGetWindowProperty( m_display, m_window, m_atomProperty, 0, 0x7FFFFFF, True, AnyPropertyType,
                                          &type, &format, &itemCount, &bytesAfter, &bytes ) )
        return CONVERTRESULT_FAILED;
    wasIncr = ( type == incr );
    if ( GUCEF_NULL != bytes )
        ::XFree( bytes );
    ::XSync( m_display, False );
    return CONVERTRESULT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::RequestForVanishingWindow( const char* targetName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return false;

    ::XSetWindowAttributes attributes;
    memset( &attributes, 0, sizeof( attributes ) );
    ::Window vanishing = ::XCreateWindow( m_display, DefaultRootWindow( m_display ), -10, -10, 1, 1, 0, 0, InputOnly,
                                          CopyFromParent, 0, &attributes );
    ::Atom target = ::XInternAtom( m_display, targetName, False );
    ::XConvertSelection( m_display, m_atomClipboard, target, m_atomProperty, vanishing, CurrentTime );
    ::XDestroyWindow( m_display, vanishing );
    ::XSync( m_display, False );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::ConvertMultiple( const char* firstTarget              ,
                                        const char* secondTarget             ,
                                        CORE::CDynamicBuffer& firstData      ,
                                        bool& firstConverted                 ,
                                        bool& secondConverted                )
{GUCEF_TRACE;

    firstData.Clear( false );
    firstConverted = false;
    secondConverted = false;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return false;

    ::Atom atomPair = ::XInternAtom( m_display, "ATOM_PAIR", False );
    ::Atom multiple = ::XInternAtom( m_display, "MULTIPLE", False );
    ::Atom multipleProperty = ::XInternAtom( m_display, "GUCEF_TEST_MULTIPLE", False );
    ::Atom firstProperty = ::XInternAtom( m_display, "GUCEF_TEST_FIRST", False );
    ::Atom secondProperty = ::XInternAtom( m_display, "GUCEF_TEST_SECOND", False );

    long pairs[ 4 ];
    pairs[ 0 ] = (long) ::XInternAtom( m_display, firstTarget, False );
    pairs[ 1 ] = (long) firstProperty;
    pairs[ 2 ] = (long) ::XInternAtom( m_display, secondTarget, False );
    pairs[ 3 ] = (long) secondProperty;
    ::XDeleteProperty( m_display, m_window, firstProperty );
    ::XDeleteProperty( m_display, m_window, secondProperty );
    ::XChangeProperty( m_display, m_window, multipleProperty, atomPair, 32, PropModeReplace, (unsigned char*) pairs, 4 );
    ::XConvertSelection( m_display, m_atomClipboard, multiple, multipleProperty, m_window, CurrentTime );
    ::XFlush( m_display );

    ::XEvent event;
    if ( !WaitForEvent( SelectionNotify, None, event, 2000 ) || None == event.xselection.property )
        return false;

    ::Atom type = None;
    int format = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* bytes = GUCEF_NULL;
    if ( Success != ::XGetWindowProperty( m_display, m_window, multipleProperty, 0, 16, True, atomPair, &type, &format,
                                          &itemCount, &bytesAfter, &bytes ) || type != atomPair || 4 != itemCount )
    {
        if ( GUCEF_NULL != bytes )
            ::XFree( bytes );
        return false;
    }
    const long* result = reinterpret_cast< const long* >( bytes );
    firstConverted = ( None != (::Atom) result[ 1 ] );
    secondConverted = ( None != (::Atom) result[ 3 ] );
    ::XFree( bytes );

    if ( firstConverted )
    {
        bytes = GUCEF_NULL;
        if ( Success == ::XGetWindowProperty( m_display, m_window, firstProperty, 0, 0x7FFFFFF, True, AnyPropertyType, &type,
                                              &format, &itemCount, &bytesAfter, &bytes ) )
        {
            AppendPropertyData( firstData, bytes, format, itemCount );
        }
        if ( GUCEF_NULL != bytes )
            ::XFree( bytes );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::TakeClipboard( const CORE::CDynamicBuffer& text, bool asLatin1String, UInt32 incrChunkSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    if ( GUCEF_NULL == m_display )
        return false;

    m_ownText = text;
    m_ownTextAsLatin1 = asLatin1String;
    m_incrChunkSize = incrChunkSize > 0 ? incrChunkSize : 1;
    m_lostClipboard = false;
    m_sentIncr = false;
    m_servedRequests = 0;
    m_incrActive = false;

    ::XSetSelectionOwner( m_display, m_atomClipboard, m_window, CurrentTime );
    return m_window == ::XGetSelectionOwner( m_display, m_atomClipboard );
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::HasLostClipboard( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    return m_lostClipboard;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::HasSentIncr( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    return m_sentIncr;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestX11ClipboardPeer::GetServedRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_peerLock );
    return m_servedRequests;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::OnThreadStart( void* taskdata )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestX11ClipboardPeer::OnThreadCycle( void* taskdata )
{GUCEF_TRACE;

    int connectionFd = -1;
    {
        MT::CScopeMutex lock( m_peerLock );
        if ( GUCEF_NULL == m_display )
            return true;
        while ( ::XPending( m_display ) > 0 )
        {
            ::XEvent event;
            ::XNextEvent( m_display, &event );
            ProcessOwnerEvent( event );
        }
        ::XFlush( m_display );
        connectionFd = ConnectionNumber( m_display );
    }

    // A short wait, so that a request to stop is seen soon
    struct pollfd waitFor;
    waitFor.fd = connectionFd;
    waitFor.events = POLLIN;
    waitFor.revents = 0;
    ::poll( &waitFor, 1, 20 );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTestX11ClipboardPeer::ProcessOwnerEvent( ::XEvent& event )
{GUCEF_TRACE;

    if ( SelectionRequest == event.type )
    {
        OnSelectionRequest( event.xselectionrequest );
    }
    else
    if ( SelectionClear == event.type && event.xselectionclear.selection == m_atomClipboard )
    {
        m_lostClipboard = true;
    }
    else
    if ( PropertyNotify == event.type && m_incrActive && event.xproperty.window == m_incrRequestor &&
         event.xproperty.atom == m_incrProperty && PropertyDelete == event.xproperty.state )
    {
        UInt32 remaining = m_ownText.GetDataSize() - m_incrOffset;
        if ( 0 == remaining )
        {
            unsigned char nothing = 0;
            ::XChangeProperty( m_display, m_incrRequestor, m_incrProperty, m_incrType, 8, PropModeReplace, &nothing, 0 );
            ::XSelectInput( m_display, m_incrRequestor, NoEventMask );
            m_incrActive = false;
        }
        else
        {
            UInt32 chunkSize = remaining < m_incrChunkSize ? remaining : m_incrChunkSize;
            ::XChangeProperty( m_display, m_incrRequestor, m_incrProperty, m_incrType, 8, PropModeReplace,
                               static_cast< const unsigned char* >( m_ownText.GetConstBufferPtr( m_incrOffset ) ), (int) chunkSize );
            m_incrOffset += chunkSize;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTestX11ClipboardPeer::OnSelectionRequest( const ::XSelectionRequestEvent& request )
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

    ::Atom targets = ::XInternAtom( m_display, "TARGETS", False );
    ::Atom textType = m_ownTextAsLatin1 ? XA_STRING : ::XInternAtom( m_display, "UTF8_STRING", False );
    ::Atom incr = ::XInternAtom( m_display, "INCR", False );

    ++m_servedRequests;
    if ( request.selection == m_atomClipboard && !m_lostClipboard && None != request.property )
    {
        if ( request.target == targets )
        {
            long list[ 2 ];
            list[ 0 ] = (long) targets;
            list[ 1 ] = (long) textType;
            ::XChangeProperty( m_display, request.requestor, request.property, XA_ATOM, 32, PropModeReplace, (unsigned char*) list, 2 );
            notify.property = request.property;
        }
        else
        if ( request.target == textType )
        {
            UInt32 size = m_ownText.GetDataSize();
            if ( size > m_incrChunkSize )
            {
                ::XSelectInput( m_display, request.requestor, PropertyChangeMask );
                long incrSize = (long) size;
                ::XChangeProperty( m_display, request.requestor, request.property, incr, 32, PropModeReplace, (unsigned char*) &incrSize, 1 );
                m_incrRequestor = request.requestor;
                m_incrProperty = request.property;
                m_incrType = textType;
                m_incrOffset = 0;
                m_incrActive = true;
                m_sentIncr = true;
            }
            else
            {
                unsigned char nothing = 0;
                const unsigned char* bytes = size > 0 ? static_cast< const unsigned char* >( m_ownText.GetConstBufferPtr() ) : &nothing;
                ::XChangeProperty( m_display, request.requestor, request.property, textType, 8, PropModeReplace, bytes, (int) size );
            }
            notify.property = request.property;
        }
    }

    ::XSendEvent( m_display, request.requestor, False, NoEventMask, reinterpret_cast< ::XEvent* >( &notify ) );
    ::XFlush( m_display );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LINUX_BUILD ? */
