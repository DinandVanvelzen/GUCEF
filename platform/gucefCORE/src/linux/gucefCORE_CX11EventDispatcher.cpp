/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CX11EventDispatcher.h"

#ifdef GUCEF_LINUX_BUILD

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#include <unistd.h>

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

MT::CMutex CX11EventDispatcher::g_mutex;
CX11EventDispatcher* CX11EventDispatcher::g_instance = NULL;

const CEvent CX11EventDispatcher::X11Event = "GUCEF::CORE::CX11EventDispatcher::X11Event";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CX11EventDispatcher::RegisterEvents( void )
{GUCEF_TRACE;

    X11Event.Initialize();
}

/*-------------------------------------------------------------------------*/

CX11EventDispatcher::CX11EventDispatcher( void )
    : m_windowObserverMap(),
      m_display( NULL ),
      m_lastUserInputTime( 0 )
{GUCEF_TRACE;

    PulseGeneratorPtr pulseGenerator = CCoreGlobal::Instance()->GetPulseGenerator();
    SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent );
    pulseGenerator->RequestPeriodicPulses( this, 10 );

    // Open the connection to the X client
    m_display = ::XOpenDisplay( NULL );
    if ( NULL == m_display )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "CX11Window::WindowCreate(): Failed to open display" );
    }
}

/*-------------------------------------------------------------------------*/

CX11EventDispatcher::~CX11EventDispatcher()
{GUCEF_TRACE;

    m_windowObserverMap.clear();
}

/*-------------------------------------------------------------------------*/

CX11EventDispatcher*
CX11EventDispatcher::Instance( void )
{GUCEF_TRACE;

    if ( NULL == g_instance )
    {
        g_mutex.Lock();
        if ( NULL == g_instance )
        {
            g_instance = new CX11EventDispatcher();
        }
        g_mutex.Unlock();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::Deinstance( void )
{GUCEF_TRACE;

    g_mutex.Lock();
    delete g_instance;
    g_instance = NULL;
    g_mutex.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::SubscribeOnBehalfOfWindow( CObserver& observer ,
                                                ::Window window     )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Subscribing observer (" +
             CORE::PointerToString( &observer ) + ") to X11 events on behalf of window " +
             CORE::UInt32ToString( window ) );

    // First create the actual subscription
    Subscribe( &observer );

    // Now update our local map which we use to filter who to send event to
    g_mutex.Lock();
    m_windowObserverMap[ window ].insert( &observer );
    g_mutex.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::UnsubscribeOnBehalfOfWindow( CObserver& observer ,
                                                  ::Window window     )
{GUCEF_TRACE;

    g_mutex.Lock();
    TWindowObserverMap::iterator i = m_windowObserverMap.find( window );
    if ( i != m_windowObserverMap.end() )
    {
        (*i).second.erase( &observer );
        if ( (*i).second.empty() )
        {
            m_windowObserverMap.erase( i );
        }
    }
    g_mutex.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::OnObserverDestruction( CObserver* observer )
{GUCEF_TRACE;

    g_mutex.Lock();

    TWindowObserverMap::iterator i = m_windowObserverMap.begin();
    while ( i != m_windowObserverMap.end() )
    {
        TObserverSet& observerSet = (*i).second;
        TObserverSet::iterator n = observerSet.begin();
        while ( n != observerSet.end() )
        {
            if ( (*n) == observer )
            {
                observerSet.erase( n );
                if ( observerSet.empty() )
                {
                    m_windowObserverMap.erase( i );
                }
                break;
            }
            ++n;
        }
        ++i;
    }

    g_mutex.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::LockData( void ) const
{GUCEF_TRACE;

    g_mutex.Lock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::UnlockData( void ) const
{GUCEF_TRACE;

    g_mutex.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::DispatchEventToWindow( ::XEvent& event ,
                                            ::Window window )
{GUCEF_TRACE;

    TWindowObserverMap::iterator i = m_windowObserverMap.find( window );
    if ( i != m_windowObserverMap.end() )
    {
        TX11EventData eventData( event );
        TObserverSet& observerSet = (*i).second;
        TObserverSet::iterator n = observerSet.begin();
        while ( n != observerSet.end() )
        {
            NotifySpecificObserver( *(*n)      ,
                                    X11Event   ,
                                    &eventData );
            ++n;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CX11EventDispatcher::ProcessEvent( ::XEvent& event )
{GUCEF_TRACE;

    switch( event.type )
    {
        case KeyRelease :
        case KeyPress :
        {
            if ( 0 != GetUserInputTimeOfEvent( event ) )
                m_lastUserInputTime = GetUserInputTimeOfEvent( event );
            DispatchEventToWindow( event, event.xkey.window );
            break;
        }
        case ButtonRelease :
        case ButtonPress :
        {
            if ( 0 != GetUserInputTimeOfEvent( event ) )
                m_lastUserInputTime = GetUserInputTimeOfEvent( event );
            DispatchEventToWindow( event, event.xbutton.window );
            break;
        }
        case MotionNotify :
        {
            DispatchEventToWindow( event, event.xmotion.window );
            break;
        }
        case ResizeRequest :
        {
            DispatchEventToWindow( event, event.xresizerequest.window );
            break;
        }
        case EnterNotify :
        case LeaveNotify :
        {
            DispatchEventToWindow( event, event.xcrossing.window );
            break;
        }
        case FocusIn :
        case FocusOut :
        {
            DispatchEventToWindow( event, event.xfocus.window );
            break;
        }
        case Expose :
        {
            DispatchEventToWindow( event, event.xexpose.window );
            break;
        }
        case GraphicsExpose :
        {
            //DispatchEventToWindow( event, event.xgraphicsexpose.window );
            break;
        }
        case NoExpose :
        {
            //DispatchEventToWindow( event, event.xnoexpose.window );
            break;
        }
        case VisibilityNotify :
        {
            DispatchEventToWindow( event, event.xvisibility.window );
            break;
        }
        case CreateNotify :
        {
            DispatchEventToWindow( event, event.xcreatewindow.window );
            break;
        }
        case DestroyNotify :
        {
            DispatchEventToWindow( event, event.xdestroywindow.window );
            break;
        }
        case UnmapNotify :
        {
            DispatchEventToWindow( event, event.xunmap.window );
            break;
        }
        case MapNotify :
        {
            DispatchEventToWindow( event, event.xmap.window );
            break;
        }
        case MapRequest :
        {
            DispatchEventToWindow( event, event.xmaprequest.window );
            break;
        }
        case ReparentNotify :
        {
            DispatchEventToWindow( event, event.xreparent.window );
            break;
        }
        case ConfigureNotify :
        {
            DispatchEventToWindow( event, event.xconfigure.window );
            break;
        }
        case GravityNotify :
        {
            DispatchEventToWindow( event, event.xgravity.window );
            break;
        }
        case ConfigureRequest :
        {
            DispatchEventToWindow( event, event.xconfigurerequest.window );
            break;
        }
        case CirculateNotify :
        {
            DispatchEventToWindow( event, event.xcirculate.window );
            break;
        }
        case CirculateRequest :
        {
            DispatchEventToWindow( event, event.xcirculaterequest.window );
            break;
        }
        case PropertyNotify :
        {
            DispatchEventToWindow( event, event.xproperty.window );
            break;
        }
        case SelectionClear :
        {
            DispatchEventToWindow( event, event.xselectionclear.window );
            break;
        }
        case SelectionRequest :
        {
            DispatchEventToWindow( event, event.xselectionrequest.owner );
            break;
        }
        case SelectionNotify :
        {
            DispatchEventToWindow( event, event.xselection.requestor );
            break;
        }
        case ColormapNotify :
        {
            DispatchEventToWindow( event, event.xcolormap.window );
            break;
        }
        case ClientMessage :
        {
            DispatchEventToWindow( event, event.xclient.window );
            break;
        }
        case MappingNotify :
        {
            DispatchEventToWindow( event, event.xmapping.window );
            break;
        }
        //case ::ErrorNotify :
        //{
            //DispatchEventToWindow( event, event.xerror.window );
            //break;
        //}
        case KeymapNotify :
        {
            DispatchEventToWindow( event, event.xkeymap.window );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CX11EventDispatcher::IsSupersededMotion( const ::XEvent& event, const ::XEvent& next )
{GUCEF_TRACE;

    return MotionNotify == event.type && MotionNotify == next.type                 &&
           event.xmotion.window == next.xmotion.window                             &&
           event.xmotion.subwindow == next.xmotion.subwindow                       &&
           event.xmotion.state == next.xmotion.state                               &&
           event.xmotion.send_event == next.xmotion.send_event;
}

/*--------------------------------------------------------------------------*/

void
CX11EventDispatcher::OnNotify( CNotifier* notifier    ,
                               const CEvent& eventid  ,
                               CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid == CPulseGenerator::PulseEvent )
    {
        XSync( m_display, False );

        ::XEvent event;
        while ( 0 != ::XEventsQueued( m_display, QueuedAlready ) )
        {
            ::XNextEvent( m_display, &event );

            // the movements of the pointer that a later one replaces are not worth the work
            ::XEvent next;
            while ( MotionNotify == event.type && 0 != ::XEventsQueued( m_display, QueuedAlready ) )
            {
                ::XPeekEvent( m_display, &next );
                if ( !IsSupersededMotion( event, next ) )
                    break;
                ::XNextEvent( m_display, &event );
            }
            ProcessEvent( event );
        }
    }
}

/*-------------------------------------------------------------------------*/

::Display*
CX11EventDispatcher::GetDisplay( void ) const
{GUCEF_TRACE;

    return m_display;
}

/*-------------------------------------------------------------------------*/

UInt32
CX11EventDispatcher::GetLastUserInputTime( void ) const
{GUCEF_TRACE;

    return m_lastUserInputTime;
}

/*-------------------------------------------------------------------------*/

UInt32
CX11EventDispatcher::GetUserInputTimeOfEvent( const ::XEvent& event )
{GUCEF_TRACE;

    if ( KeyPress == event.type )
        return (UInt32) event.xkey.time;
    if ( ButtonPress == event.type )
        return (UInt32) event.xbutton.time;
    return 0;
}

/*-------------------------------------------------------------------------*/

CX11ActivationTokenProvider::CX11ActivationTokenProvider( const CString& appName )
    : CIActivationTokenProvider()
    , m_appName( appName )
    , m_sequence( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CX11ActivationTokenProvider::~CX11ActivationTokenProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CX11ActivationTokenProvider::GetUserInputTime( void ) const
{GUCEF_TRACE;

    return CX11EventDispatcher::Instance()->GetLastUserInputTime();
}

/*-------------------------------------------------------------------------*/

static CString
CleanStartupIdPart( const CString& text )
{GUCEF_TRACE;

    CString clean;
    const char* p = text.C_String();
    const UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = p[ i ];
        const bool plain = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c;
        clean += plain ? c : '_';
    }
    return clean;
}

/*-------------------------------------------------------------------------*/

CString
CX11ActivationTokenProvider::MakeStartupId( const CString& appName, UInt32 processId, UInt32 sequence, const CString& hostName, UInt32 userInputTime )
{GUCEF_TRACE;

    CString name = CleanStartupIdPart( appName );
    if ( name.IsNULLOrEmpty() )
        name = "program";
    CString host = CleanStartupIdPart( hostName );
    if ( host.IsNULLOrEmpty() )
        host = "localhost";

    CString id = name;
    id += '/';
    id += UInt32ToString( processId );
    id += '/';
    id += UInt32ToString( sequence );
    id += '-';
    id += host;
    id += "_TIME";
    id += UInt32ToString( userInputTime );
    return id;
}

/*-------------------------------------------------------------------------*/

bool
CX11ActivationTokenProvider::GetActivationToken( CString& token, TActivationTokenKind& kind, UInt32 maxWaitInMs )
{GUCEF_TRACE;

    token.Clear();
    kind = ACTIVATIONTOKEN_STARTUP_ID;

    // without a key or button press there is no action of the user to point the window manager at
    const UInt32 userInputTime = GetUserInputTime();
    if ( 0 == userInputTime )
        return false;

    char hostName[ 256 ];
    hostName[ 0 ] = '\0';
    if ( 0 != ::gethostname( hostName, sizeof( hostName ) - 1 ) )
        hostName[ 0 ] = '\0';
    hostName[ sizeof( hostName ) - 1 ] = '\0';

    token = MakeStartupId( m_appName, (UInt32) ::getpid(), m_sequence, hostName, userInputTime );
    ++m_sequence;
    return true;
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
