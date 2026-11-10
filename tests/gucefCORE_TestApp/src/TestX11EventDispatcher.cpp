/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>
#include <unistd.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD
#ifndef GUCEF_CORE_CX11EVENTDISPATCHER_H
#include "gucefCORE_CX11EventDispatcher.h"
#define GUCEF_CORE_CX11EVENTDISPATCHER_H
#endif /* GUCEF_CORE_CX11EVENTDISPATCHER_H ? */
#endif /* GUCEF_LINUX_BUILD ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestX11EventDispatcher.h"

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

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#ifdef GUCEF_LINUX_BUILD

namespace {

::XEvent
MakeMotion( ::Window window, int x, int y, unsigned int state )
{
    ::XEvent event;
    memset( &event, 0, sizeof( event ) );
    event.type = MotionNotify;
    event.xmotion.window = window;
    event.xmotion.x = x;
    event.xmotion.y = y;
    event.xmotion.state = state;
    return event;
}

/** A provider that has the time of the last input from the test and not from the display */
class CTimedStartupIdProvider : public CORE::CX11ActivationTokenProvider
{
    public:

    explicit CTimedStartupIdProvider( const CORE::CString& appName ) : CORE::CX11ActivationTokenProvider( appName ), m_time( 0 ) {}

    UInt32 m_time;

    protected:

    virtual UInt32 GetUserInputTime( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_time; }
};

}

#endif /* GUCEF_LINUX_BUILD ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformX11EventDispatcherTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.X11EventDispatcher" );

    #ifdef GUCEF_LINUX_BUILD

    GUCEF_TESTFW_TESTCASE( "X11EventDispatcher: A motion of the pointer is replaced by the next one of the same window with the same buttons and keys held, and by nothing else" )
    try
    {
        ::XEvent first = MakeMotion( 10, 1, 1, 0 );
        ::XEvent second = MakeMotion( 10, 2, 2, 0 );
        ASSERT_TRUE( CORE::CX11EventDispatcher::IsSupersededMotion( first, second ) );

        // another window, or the pointer in a child of it: two different places the pointer is seen
        ::XEvent otherWindow = MakeMotion( 11, 2, 2, 0 );
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( first, otherWindow ) );
        ::XEvent inChild = MakeMotion( 10, 2, 2, 0 );
        inChild.xmotion.subwindow = 99;
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( first, inChild ) );

        // the buttons and keys that are down are part of what a motion tells: a drag is not the same as a move
        ::XEvent dragged = MakeMotion( 10, 2, 2, Button1Mask );
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( first, dragged ) );
        ::XEvent draggedMore = MakeMotion( 10, 3, 3, Button1Mask );
        ASSERT_TRUE( CORE::CX11EventDispatcher::IsSupersededMotion( dragged, draggedMore ) );
        ::XEvent shifted = MakeMotion( 10, 3, 3, Button1Mask | ShiftMask );
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( dragged, shifted ) );

        // a button or a key in between is not a motion, and an event that is not a motion is never replaced
        ::XEvent press = second;
        press.type = ButtonPress;
        press.xbutton.button = Button1;
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( first, press ) );
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( press, second ) );
        ::XEvent key = second;
        key.type = KeyPress;
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( first, key ) );
        ASSERT_FALSE( CORE::CX11EventDispatcher::IsSupersededMotion( key, key ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "X11EventDispatcher: Only the press of a key or a button is an action of the user with a time; releases, motions and the rest are not" )
    try
    {
        ::XEvent event;
        memset( &event, 0, sizeof( event ) );
        event.type = KeyPress;
        event.xkey.time = 123456;
        ASSERT_TRUE( 123456 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
        event.type = ButtonPress;
        event.xbutton.time = 654321;
        ASSERT_TRUE( 654321 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
        event.type = KeyRelease;
        ASSERT_TRUE( 0 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
        event.type = ButtonRelease;
        ASSERT_TRUE( 0 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
        event.type = MotionNotify;
        event.xmotion.time = 99;
        ASSERT_TRUE( 0 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
        event.type = FocusIn;
        ASSERT_TRUE( 0 == CORE::CX11EventDispatcher::GetUserInputTimeOfEvent( event ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "X11EventDispatcher: The startup id is name/pid/n-host_TIME and the server time of the last user input, cleaned up to what the specification allows" )
    try
    {
        ASSERT_TRUE( "redux/4242/0-workstation_TIME1234567" == CORE::CX11ActivationTokenProvider::MakeStartupId( "redux", 4242, 0, "workstation", 1234567 ) );
        ASSERT_TRUE( "redux/4242/17-workstation_TIME4294967295" == CORE::CX11ActivationTokenProvider::MakeStartupId( "redux", 4242, 17, "workstation", 4294967295U ) );

        // what is not a letter, digit, '.', '_' or '-' cannot be in the name or the host; nothing gives the defaults
        ASSERT_TRUE( "my_game_v2/1/2-lap_top.local_TIME5" == CORE::CX11ActivationTokenProvider::MakeStartupId( "my game/v2", 1, 2, "lap top.local", 5 ) );
        ASSERT_TRUE( "program/1/0-localhost_TIME5" == CORE::CX11ActivationTokenProvider::MakeStartupId( CORE::CString::Empty, 1, 0, CORE::CString::Empty, 5 ) );
        const CORE::CString nasty = CORE::CX11ActivationTokenProvider::MakeStartupId( "a\nb\"c;d", 1, 0, "h$(x)", 5 );
        ASSERT_TRUE( nasty.HasSubstr( "\n" ) < 0 && nasty.HasSubstr( "\"" ) < 0 && nasty.HasSubstr( ";" ) < 0 && nasty.HasSubstr( "$" ) < 0 && nasty.HasSubstr( "(" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "X11EventDispatcher: The provider makes an id with the time of the last user input, a new number each time, and none before there was any input" )
    try
    {
        CTimedStartupIdProvider provider( "redux" );
        CORE::CString token;
        CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;

        // nothing was pressed yet
        ASSERT_FALSE( provider.GetActivationToken( token, kind, CORE::ActivationTokenMaxWaitInMs ) );
        ASSERT_TRUE( token.IsNULLOrEmpty() && CORE::ACTIVATIONTOKEN_STARTUP_ID == kind );

        provider.m_time = 987654;
        ASSERT_TRUE( provider.GetActivationToken( token, kind, CORE::ActivationTokenMaxWaitInMs ) );
        ASSERT_TRUE( CORE::ACTIVATIONTOKEN_STARTUP_ID == kind );
        char hostName[ 256 ];
        hostName[ 0 ] = '\0';
        ::gethostname( hostName, sizeof( hostName ) - 1 );
        hostName[ sizeof( hostName ) - 1 ] = '\0';
        ASSERT_TRUE( CORE::CX11ActivationTokenProvider::MakeStartupId( "redux", (UInt32) ::getpid(), 0, hostName, 987654 ) == token );
        ASSERT_TRUE( token.HasSubstr( "_TIME987654" ) > 0 );

        provider.m_time = 987700;
        ASSERT_TRUE( provider.GetActivationToken( token, kind, 0 ) );
        ASSERT_TRUE( CORE::CX11ActivationTokenProvider::MakeStartupId( "redux", (UInt32) ::getpid(), 1, hostName, 987700 ) == token );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/
