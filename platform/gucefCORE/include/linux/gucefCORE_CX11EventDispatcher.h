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

#ifndef GUCEF_CORE_CX11EVENTDISPATCHER_H
#define GUCEF_CORE_CX11EVENTDISPATCHER_H

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

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

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
 *  Class which wraps the X11 event polling and turns it into a notification
 *  system
 */
class GUCEF_CORE_PUBLIC_CPP CX11EventDispatcher : public CObservingNotifier
{
    public:

    static const CEvent X11Event;

    static void RegisterEvents( void );

    typedef CTCloneableObj< ::XEvent >  TX11EventData;

    public:

    static CX11EventDispatcher* Instance( void );

    /**
     *  In addition to the standard subscription options this gives you the option of
     *  subscribing to events to be sent only if the event is destined for the given window
     */
    void SubscribeOnBehalfOfWindow( CObserver& observer ,
                                    ::Window window     );

    void UnsubscribeOnBehalfOfWindow( CObserver& observer ,
                                      ::Window window     );

    ::Display* GetDisplay( void ) const;

    /**
     *  The time the X server gave the last key press or button press that was dispatched (the 'time' of the event, in
     *  milliseconds of the server), 0 when there was none yet. What the startup notification id of a program that is
     *  launched on behalf of the user carries, so that the window manager lets its window to the front.
     */
    UInt32 GetLastUserInputTime( void ) const;

    /** The server time of an event that is an action of the user, a key press or a button press; 0 for every other event (a release or a motion is no reason to let a window come to the front) */
    static UInt32 GetUserInputTimeOfEvent( const ::XEvent& event );

    /**
     *  Whether 'next' replaces 'event': both are motions of the pointer in the same window (and child), with the same
     *  buttons and keys held. A mouse that reports a thousand times a second makes a thousand of them, only the last
     *  one tells where the pointer is: the dispatcher does not hand on the ones that are replaced, as the work for
     *  each of them would keep the application from ever getting to the clicks and the frames behind them
     */
    static bool IsSupersededMotion( const ::XEvent& event, const ::XEvent& next );

    protected:

    virtual void OnObserverDestruction( CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void LockData( void ) const;

    virtual void UnlockData( void ) const;

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CModule;

    static void Deinstance( void );

    private:

    CX11EventDispatcher();

    virtual ~CX11EventDispatcher();

    void ProcessEvent( ::XEvent& event );

    void DispatchEventToWindow( ::XEvent& event, ::Window window );

    private:

    typedef std::set< CObserver* > TObserverSet;
    typedef std::map< ::Window, TObserverSet >  TWindowObserverMap;

    static CX11EventDispatcher* g_instance;
    static MT::CMutex g_mutex;
    TWindowObserverMap m_windowObserverMap;
    ::Display* m_display;
    UInt32 m_lastUserInputTime;
};

/*-------------------------------------------------------------------------*/

/**
 *  Gives a program that is launched from an X11 window the startup notification id of the freedesktop.org startup
 *  notification specification, which ends with the server time of the last key or button press of the user
 *  (CX11EventDispatcher::GetLastUserInputTime()):
 *
 *      <name>/<pid>/<n>-<hostname>_TIME<time>
 *
 *  The launcher puts it in DESKTOP_STARTUP_ID, the window manager takes the time of it as the time of the user action
 *  and lets the window of the launched program come to the front instead of behind the window of this one. Without any
 *  input yet there is no id. A session of XWayland gets the behaviour of Wayland from the compositor; nothing more is done for it.
 */
class GUCEF_CORE_PUBLIC_CPP CX11ActivationTokenProvider : public CIActivationTokenProvider
{
    public:

    /** @param appName  the name of the launching program in the id, made of letters, digits, '.', '_' and '-' (anything else becomes '_'); "program" when empty */
    explicit CX11ActivationTokenProvider( const CString& appName = CString() );

    virtual ~CX11ActivationTokenProvider();

    virtual bool GetActivationToken( CString& token, TActivationTokenKind& kind, UInt32 maxWaitInMs ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes an id from its parts; the name and the host name are cleaned up as described above, an empty host name is "localhost" */
    static CString MakeStartupId( const CString& appName, UInt32 processId, UInt32 sequence, const CString& hostName, UInt32 userInputTime );

    protected:

    /** The time of the last user input: the one of the dispatcher, a test says its own */
    virtual UInt32 GetUserInputTime( void ) const;

    private:

    CX11ActivationTokenProvider( const CX11ActivationTokenProvider& src );
    CX11ActivationTokenProvider& operator=( const CX11ActivationTokenProvider& src );

    private:

    CString m_appName;
    UInt32 m_sequence;
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

#endif /* GUCEF_CORE_CX11EVENTDISPATCHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation

---------------------------------------------------------------------------*/
