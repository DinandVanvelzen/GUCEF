/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_H
#include "guidriverWAYLAND.h"
#define GUCEF_GUIDRIVERWAYLAND_H
#endif /* GUCEF_GUIDRIVERWAYLAND_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#include "inputdriverWAYLAND_CWaylandInputContext.h"
#define GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#endif /* GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CKEYMAPPING_H
#include "inputdriverWAYLAND_CKeyMapping.h"
#define GUCEF_INPUT_WAYLAND_CKEYMAPPING_H
#endif /* GUCEF_INPUT_WAYLAND_CKEYMAPPING_H ? */

#include <stdexcept>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#include "pluginglueWAYLAND_CEventGuard.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

#include "inputdriverWAYLAND.h"
#include "TestWaylandCommon.h"
#include "TestWaylandInput.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::INPUT;
using namespace GUCEF::INPUT::WAYLAND;
using namespace GUCEF::TESTWAYLAND;

#define BTN_LEFT   0x110
#define BTN_RIGHT  0x111
#define BTN_MIDDLE 0x112
#define BTN_SIDE   0x113
#define BTN_EXTRA  0x114

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32 g_clock = 0;
UInt32 FakeClock( void ) { return g_clock; }

/** Everything an input context needs: a fake compositor with a seat, a connection, the surface of a window and the recorder of the callbacks */
struct SInputRig
{
    CFakeWayland fake;
    CWaylandConnection connection;
    CInputRecorder recorder;
    SFakeProxy* surface;
    UInt32 surfaceId;
    SXkbApi xkb;
    bool hasXkb;
    CWaylandInputContext* context;

    explicit SInputRig( UInt32 seatVersion = 9, bool withKeymap = true )
        : fake(), connection( fake.GetApi() ), recorder(), surface( GUCEF_NULL ), surfaceId( 0 ), xkb(), hasXkb( false ), context( GUCEF_NULL )
    {
        fake.OfferGlobal( "wl_compositor", 6 );
        fake.OfferGlobal( "xdg_wm_base", 6 );
        fake.OfferGlobal( "wl_seat", seatVersion );
        if ( withKeymap )
            fake.keymapText = GetTestKeymap();
        connection.SetPumpOnPulse( false );
        hasXkb = LoadXkbApi( xkb );
    }

    ~SInputRig()
    {
        delete context;
    }

    bool Start( void )
    {
        UseNoCursorTheme( connection );
        if ( !connection.Connect() )
            return false;
        surface = fake.FindProxy( "wl_compositor" );        // stands for the surface of the window
        surfaceId = connection.RegisterSurface( (struct wl_surface*) surface );
        g_clock = 1000;
        context = new CWaylandInputContext( connection, surfaceId, recorder.callbacks, hasXkb ? &xkb : GUCEF_NULL, false );
        context->SetClock( FakeClock );
        return true;
    }

    void Focus( void )
    {
        fake.SendPointerEnter( 1, surface, 0.0, 0.0 );
        fake.SendKeyboardEnter( 2, surface );
        recorder.Clear();
    }
};

/** A receiver of the wheel that throws */
void
ThrowingVar( void*, const Int32, const UInt32, const Int32, const Int32 )
{
    throw std::runtime_error( "the receiver of the wheel is broken" );
}

/** A receiver of the wheel that takes the window, and with it the input context, away while it is called */
SInputRig* g_rigToDestroy = GUCEF_NULL;

void
DestroyingVar( void*, const Int32, const UInt32, const Int32, const Int32 )
{
    if ( GUCEF_NULL != g_rigToDestroy )
    {
        delete g_rigToDestroy->context;
        g_rigToDestroy->context = GUCEF_NULL;
    }
}

/** Counts the frames of the pointer */
class CWireFrameCounter : public IWaylandSeatListener
{
    public:

    UInt32 frames;
    CWireFrameCounter( void ) : frames( 0 ) {}
    virtual void OnPointerFrame( void ) { ++frames; }
};

/** A listener of the seat that throws in every event, for the boundary with libwayland */
class CThrowingSeatListener : public IWaylandSeatListener
{
    public:

    UInt32 frames;
    CThrowingSeatListener( void ) : frames( 0 ) {}
    virtual void OnPointerAxisRelativeDirection( UInt32, UInt32 ) { throw std::runtime_error( "relative direction" ); }
    virtual void OnPointerAxisValue120( UInt32, Int32 ) { throw 42; }
    virtual void OnPointerFrame( void ) { ++frames; throw std::runtime_error( "frame" ); }
};

CORE::CString
KeyDown( KeyCode code, UInt32 mods = 0 )
{
    return "keydown " + CORE::UInt32ToString( (UInt32) code ) + " m" + CORE::UInt32ToString( mods );
}

CORE::CString
KeyUp( KeyCode code, UInt32 mods = 0 )
{
    return "keyup " + CORE::UInt32ToString( (UInt32) code ) + " m" + CORE::UInt32ToString( mods );
}

}

/*-------------------------------------------------------------------------*/

void
PerformWaylandInputTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland input TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandInput" );

    GUCEF_TESTFW_TESTCASE( "Test: A wheel notch of a high resolution wheel is a notch, and down on the screen is negative" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            // one notch down (Wayland counts down as positive)
            accumulator.OnAxisValue120( CAxisAccumulator::AXIS_VERTICAL, 120 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -1 == vertical && 0 == horizontal );
            // one notch up
            accumulator.OnAxisValue120( CAxisAccumulator::AXIS_VERTICAL, -120 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 1 == vertical );
            // one notch to the right is positive, as for the X11 driver
            accumulator.OnAxisValue120( CAxisAccumulator::AXIS_HORIZONTAL, 120 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 0 == vertical && 1 == horizontal );
            accumulator.OnAxisValue120( CAxisAccumulator::AXIS_HORIZONTAL, -240 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -2 == horizontal );
            // nothing happened
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 0 == vertical && 0 == horizontal );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Parts of a notch add up across frames and come out as whole notches" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            // a smooth wheel: thirds of a notch
            accumulator.OnAxisValue120( 0, -40 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxisValue120( 0, -40 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxisValue120( 0, -40 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 1 == vertical );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            // 5 and a half notches in one frame: 5 now, the half stays
            accumulator.OnAxisValue120( 0, 660 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -5 == vertical );
            accumulator.OnAxisValue120( 0, 60 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -1 == vertical );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Whole notches of a wheel without high resolution events (discrete) count as they are" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            accumulator.OnAxisSource( 0 );
            accumulator.OnAxis( 0, 10.0 );          // what the compositor sends besides it must not count twice
            accumulator.OnAxisDiscrete( 0, 1 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -1 == vertical );
            accumulator.OnAxis( 0, -30.0 );
            accumulator.OnAxisDiscrete( 0, -2 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 2 == vertical );
            // value120 beats discrete beats the axis value in one group
            accumulator.OnAxis( 0, 100.0 );
            accumulator.OnAxisDiscrete( 0, 5 );
            accumulator.OnAxisValue120( 0, 120 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -1 == vertical );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Smooth scrolling in surface units is 15 units a notch and accumulates; the end of the scroll drops the rest" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            accumulator.OnAxisSource( 1 );          // finger
            accumulator.OnAxis( 0, -7.5 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxis( 0, -7.5 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 1 == vertical );
            // two and a bit notches in one frame, then more
            accumulator.OnAxis( 0, 36.0 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -2 == vertical );
            accumulator.OnAxis( 0, 4.0 );           // 6 + 4 = 10 units: not a notch yet
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxis( 0, 6.0 );           // 16 units: a notch, 1 unit left
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -1 == vertical );

            // the fingers lift: the unit that is left is dropped
            accumulator.OnAxisStop( 0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxis( 0, 14.0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );          // 14, not 15
            // the scroll ends in the group that moves, too: what is due is given, the rest is dropped
            accumulator.OnAxis( 0, 20.0 );
            accumulator.OnAxisStop( 0 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( -2 == vertical );
            accumulator.OnAxis( 0, 14.0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A scroll that turns around leaves the part of a notch of the other direction behind" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            accumulator.OnAxis( 0, 10.0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxis( 0, -10.0 );          // the old 10 would have left 0: it is dropped and the count starts anew at -10
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.OnAxis( 0, -5.0 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 1 == vertical );

            // a different size of a notch
            accumulator.SetUnitsPerNotch( 10.0 );
            ASSERT_TRUE( 10.0 == accumulator.GetUnitsPerNotch() );
            accumulator.OnAxis( 1, 25.0 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( 2 == horizontal );
            accumulator.SetUnitsPerNotch( 0.0 );
            ASSERT_TRUE( 10.0 == accumulator.GetUnitsPerNotch() );
            accumulator.Reset();
            accumulator.OnAxis( 1, 5.0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
            accumulator.Reset();
            accumulator.OnAxis( 1, 5.0 );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );          // the reset dropped the 5 before
            accumulator.OnAxis( 9, 500.0 );                                     // an axis that does not exist
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Buttons of the evdev codes get the indices of the X11 driver" )
        try
        {
            ASSERT_TRUE( 0 == CKeyMapping::MapButton( BTN_LEFT ) );
            ASSERT_TRUE( 1 == CKeyMapping::MapButton( BTN_RIGHT ) );
            ASSERT_TRUE( 2 == CKeyMapping::MapButton( BTN_MIDDLE ) );
            ASSERT_TRUE( 3 == CKeyMapping::MapButton( BTN_SIDE ) );
            ASSERT_TRUE( 4 == CKeyMapping::MapButton( BTN_EXTRA ) );
            ASSERT_TRUE( 5 == CKeyMapping::MapButton( 0x115 ) );
            ASSERT_TRUE( 6 == CKeyMapping::MapButton( 0x116 ) );
            ASSERT_TRUE( 7 == CKeyMapping::MapButton( 0x117 ) );
            ASSERT_TRUE( -1 == CKeyMapping::MapButton( 0x110 - 1 ) );
            ASSERT_TRUE( -1 == CKeyMapping::MapButton( 0x118 ) );
            ASSERT_TRUE( -1 == CKeyMapping::MapButton( 0 ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Keysyms get the key codes of the X11 driver, letters in either case" )
        try
        {
            ASSERT_TRUE( KEYCODE_A == CKeyMapping::MapKeysym( 0x61 ) );        // a
            ASSERT_TRUE( KEYCODE_A == CKeyMapping::MapKeysym( 0x41 ) );        // A
            ASSERT_TRUE( KEYCODE_Z == CKeyMapping::MapKeysym( 0x7A ) );
            ASSERT_TRUE( KEYCODE_1 == CKeyMapping::MapKeysym( 0x31 ) );
            ASSERT_TRUE( KEYCODE_RETURN == CKeyMapping::MapKeysym( 0xFF0D ) );
            ASSERT_TRUE( KEYCODE_ESCAPE == CKeyMapping::MapKeysym( 0xFF1B ) );
            ASSERT_TRUE( KEYCODE_BACK == CKeyMapping::MapKeysym( 0xFF08 ) );
            ASSERT_TRUE( KEYCODE_TAB == CKeyMapping::MapKeysym( 0xFF09 ) );
            ASSERT_TRUE( KEYCODE_SPACE == CKeyMapping::MapKeysym( 0x20 ) );
            ASSERT_TRUE( KEYCODE_LEFT == CKeyMapping::MapKeysym( 0xFF51 ) );
            ASSERT_TRUE( KEYCODE_UP == CKeyMapping::MapKeysym( 0xFF52 ) );
            ASSERT_TRUE( KEYCODE_RIGHT == CKeyMapping::MapKeysym( 0xFF53 ) );
            ASSERT_TRUE( KEYCODE_DOWN == CKeyMapping::MapKeysym( 0xFF54 ) );
            ASSERT_TRUE( KEYCODE_F1 == CKeyMapping::MapKeysym( 0xFFBE ) );
            ASSERT_TRUE( KEYCODE_F12 == CKeyMapping::MapKeysym( 0xFFC9 ) );
            ASSERT_TRUE( KEYCODE_LSHIFT == CKeyMapping::MapKeysym( 0xFFE1 ) );
            ASSERT_TRUE( KEYCODE_RCONTROL == CKeyMapping::MapKeysym( 0xFFE4 ) );
            ASSERT_TRUE( KEYCODE_LMENU == CKeyMapping::MapKeysym( 0xFFE9 ) );
            ASSERT_TRUE( KEYCODE_LWIN == CKeyMapping::MapKeysym( 0xFFEB ) );
            ASSERT_TRUE( KEYCODE_NUMPAD7 == CKeyMapping::MapKeysym( 0xFFB7 ) );
            ASSERT_TRUE( KEYCODE_NUMPADENTER == CKeyMapping::MapKeysym( 0xFF8D ) );
            ASSERT_TRUE( KEYCODE_DELETE == CKeyMapping::MapKeysym( 0xFFFF ) );
            ASSERT_TRUE( KEYCODE_UNASSIGNED == CKeyMapping::MapKeysym( 0x6C6 ) );       // Cyrillic ef
            ASSERT_TRUE( KEYCODE_UNASSIGNED == CKeyMapping::MapKeysym( 0 ) );
            ASSERT_TRUE( KEYCODE_UNASSIGNED == CKeyMapping::MapKeysym( 0x21 ) );        // exclam has no code of its own
            ASSERT_TRUE( CKeyMapping::IsModifierKey( KEYCODE_LSHIFT ) && CKeyMapping::IsModifierKey( KEYCODE_RWIN ) && CKeyMapping::IsModifierKey( KEYCODE_CAPITAL ) );
            ASSERT_FALSE( CKeyMapping::IsModifierKey( KEYCODE_A ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A held key repeats after the delay at the rate the compositor says, from the time of the pulses" )
        try
        {
            CKeyRepeat repeat;
            repeat.SetRate( 25, 400 );              // every 40 ms after 400
            UInt32 key = 0;
            ASSERT_TRUE( 0 == repeat.Poll( 5000, key ) );
            ASSERT_FALSE( repeat.IsRepeating() );
            repeat.Start( 30, 1000 );
            ASSERT_TRUE( repeat.IsRepeating() && 30 == repeat.GetRepeatingKey() );
            ASSERT_TRUE( 0 == repeat.Poll( 1399, key ) );
            ASSERT_TRUE( 1 == repeat.Poll( 1400, key ) && 30 == key );
            ASSERT_TRUE( 0 == repeat.Poll( 1439, key ) );
            ASSERT_TRUE( 1 == repeat.Poll( 1440, key ) );
            // a pulse that came late owes the repeats it missed
            ASSERT_TRUE( 3 == repeat.Poll( 1560, key ) );       // 1480, 1520, 1560
            ASSERT_TRUE( 0 == repeat.Poll( 1599, key ) );
            // after a long stall the repeats are capped and the schedule starts again
            ASSERT_TRUE( CKeyRepeat::MaxRepeatsPerPoll == repeat.Poll( 9000, key ) );
            ASSERT_TRUE( 0 == repeat.Poll( 9039, key ) );
            ASSERT_TRUE( 1 == repeat.Poll( 9040, key ) );
            // another key is not the one that repeats stopping
            repeat.Stop( 31 );
            ASSERT_TRUE( repeat.IsRepeating() );
            repeat.Stop( 30 );
            ASSERT_FALSE( repeat.IsRepeating() );
            ASSERT_TRUE( 0 == repeat.Poll( 20000, key ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Key repeat is off with a rate of 0, follows new rates, and survives the tick count wrapping" )
        try
        {
            CKeyRepeat repeat;
            UInt32 key = 0;
            repeat.SetRate( 0, 400 );
            repeat.Start( 30, 1000 );
            ASSERT_FALSE( repeat.IsRepeating() );
            ASSERT_TRUE( 0 == repeat.Poll( 2000, key ) );
            repeat.SetRate( 10, 100 );
            repeat.Start( 30, 1000 );
            ASSERT_TRUE( 1 == repeat.Poll( 1100, key ) );
            ASSERT_TRUE( 0 == repeat.Poll( 1199, key ) );
            ASSERT_TRUE( 1 == repeat.Poll( 1200, key ) );
            repeat.SetRate( 0, 100 );
            ASSERT_FALSE( repeat.IsRepeating() );

            // the clock passes the end of the range of 32 bits
            repeat.SetRate( 20, 100 );
            repeat.Start( 7, 0xFFFFFFF0u );
            ASSERT_TRUE( 0 == repeat.Poll( 0xFFFFFFF0u + 99, key ) );
            ASSERT_TRUE( 1 == repeat.Poll( 0xFFFFFFF0u + 100, key ) );          // wraps to 84
            ASSERT_TRUE( 1 == repeat.Poll( 0xFFFFFFF0u + 150, key ) );
            ASSERT_TRUE( 0 == repeat.Poll( 0xFFFFFFF0u + 160, key ) );
            ASSERT_TRUE( 7 == key );
            repeat.StopAll();
            ASSERT_FALSE( repeat.IsRepeating() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The keyboard layout gives codes, modifiers and text, in any layout" )
        try
        {
            SXkbApi api;
            GUCEF_TESTFW_SKIP_IF( !LoadXkbApi( api ), "libxkbcommon.so.0 is not installed" );

            CXkbKeyboard keyboard( api );
            ASSERT_FALSE( keyboard.HasKeymap() );
            ASSERT_FALSE( keyboard.LoadKeymap( "xkb_keymap { this is not a keymap }", 34 ) );
            ASSERT_FALSE( keyboard.HasKeymap() );
            const char* text = GetTestKeymap();
            ASSERT_TRUE( keyboard.LoadKeymap( text, (UInt32) strlen( text ) + 1 ) );

            UInt32 keysyms[ 4 ];
            UInt32 count = keyboard.GetBaseKeysyms( TESTKEY_A, keysyms, 4 );
            ASSERT_TRUE( 2 == count );
            ASSERT_TRUE( 0x61 == keysyms[ 0 ] );               // the key without modifiers, whatever Shift does
            ASSERT_TRUE( 0x6C6 == keysyms[ 1 ] );              // Cyrillic ef of the other layout
            ASSERT_TRUE( 1 == keyboard.GetBaseKeysyms( TESTKEY_S, keysyms, 4 ) );
            ASSERT_TRUE( 0 == keyboard.GetBaseKeysyms( 200, keysyms, 4 ) );

            char buffer[ 64 ];
            ASSERT_TRUE( keyboard.GetText( TESTKEY_A, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "a" ) );
            keyboard.UpdateMask( 1, 0, 0, 0 );               // Shift
            ASSERT_TRUE( KEYMOD_SHIFT == keyboard.GetModifierMask() );
            ASSERT_TRUE( keyboard.GetText( TESTKEY_A, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "A" ) );
            ASSERT_TRUE( keyboard.GetText( TESTKEY_1, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "!" ) );
            keyboard.UpdateMask( 1 | 4, 0, 2, 0 );          // Shift and Control, Caps Lock locked
            ASSERT_TRUE( ( KEYMOD_SHIFT | KEYMOD_CTRL | KEYMOD_CAPSLOCK ) == keyboard.GetModifierMask() );
            keyboard.UpdateMask( 0, 0, 0, 0 );
            ASSERT_TRUE( 0 == keyboard.GetModifierMask() );

            // keys that type nothing
            ASSERT_FALSE( keyboard.GetText( TESTKEY_ENTER, buffer, sizeof( buffer ) ) );
            ASSERT_FALSE( keyboard.GetText( TESTKEY_LEFT, buffer, sizeof( buffer ) ) );
            ASSERT_FALSE( keyboard.GetText( TESTKEY_LSHIFT, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( keyboard.GetText( TESTKEY_SPACE, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, " " ) );
            ASSERT_TRUE( keyboard.KeyRepeats( TESTKEY_A ) );

            // the second layout
            keyboard.UpdateMask( 0, 0, 0, 1 );
            count = keyboard.GetBaseKeysyms( TESTKEY_A, keysyms, 4 );
            ASSERT_TRUE( 2 == count && 0x6C6 == keysyms[ 0 ] && 0x61 == keysyms[ 1 ] );     // the active layout first
            ASSERT_TRUE( keyboard.GetText( TESTKEY_A, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "\xD1\x84" ) );       // ф
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Dead keys and compose sequences type the composed character, when the locale has a compose table" )
        try
        {
            SXkbApi api;
            GUCEF_TESTFW_SKIP_IF( !LoadXkbApi( api ), "libxkbcommon.so.0 is not installed" );

            CXkbKeyboard keyboard( api );
            const char* keymap = GetTestKeymap();
            ASSERT_TRUE( keyboard.LoadKeymap( keymap, (UInt32) strlen( keymap ) + 1 ) );
            char buffer[ 64 ];
            // the dead key itself types nothing (the middle of a sequence) or, without a compose table, its own character
            const bool typesNothing = !keyboard.GetText( TESTKEY_DEAD, buffer, sizeof( buffer ) );
            GUCEF_TESTFW_SKIP_IF( !typesNothing, "the locale of this process has no compose table" );
            ASSERT_TRUE( keyboard.GetText( TESTKEY_A, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "\xC3\xA1" ) );       // á
            // and the next a is an a again
            ASSERT_TRUE( keyboard.GetText( TESTKEY_A, buffer, sizeof( buffer ) ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "a" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Devices are attached when the seat has them and detached when it loses them, with the keys held let go first" )
        try
        {
            SInputRig rig;
            ASSERT_TRUE( rig.Start() );
            ASSERT_TRUE( rig.context->HasPointer() && rig.context->HasKeyboard() );
            ASSERT_TRUE( rig.recorder.Has( "mouse attached" ) && rig.recorder.Has( "keyboard attached" ) );

            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.Focus();
            rig.fake.SendKeyboardKey( 3, 1, TESTKEY_A, 1 );
            rig.recorder.Clear();
            rig.fake.SendSeatCapabilities( 1 );             // the keyboard is unplugged
            ASSERT_TRUE( rig.recorder.Has( KeyUp( KEYCODE_A ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( "keyboard detached" ) );
            ASSERT_FALSE( rig.context->HasKeyboard() );
            ASSERT_TRUE( rig.context->HasPointer() );
            rig.recorder.Clear();
            rig.fake.SendSeatCapabilities( 3 );
            ASSERT_TRUE( rig.recorder.Has( "keyboard attached" ) );
            rig.recorder.Clear();
            rig.fake.SendSeatCapabilities( 0 );
            ASSERT_TRUE( rig.recorder.Has( "mouse detached" ) && rig.recorder.Has( "keyboard detached" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The pointer moves in pixels of the window and ignores other surfaces" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.recorder.Clear();

            rig.fake.SendPointerEnter( 5, rig.surface, 10.0, 20.0 );
            ASSERT_TRUE( rig.context->IsPointerInside() );
            ASSERT_TRUE( rig.recorder.Has( "move 10,20 d0,0" ) );
            rig.fake.SendPointerMotion( 1, 15.0, 25.0 );
            ASSERT_TRUE( rig.recorder.Has( "move 15,25 d5,5" ) );
            rig.fake.SendPointerMotion( 2, 15.5, 24.5 );
            ASSERT_TRUE( rig.recorder.Has( "move 16,25 d1,0" ) );                       // pixels, rounded
            Int32 x = 0, y = 0;
            ASSERT_TRUE( rig.context->GetMousePosition( x, y ) );
            ASSERT_TRUE( 16 == x && 25 == y );

            // a surface of someone else: the decorations of libdecor are not the window
            rig.recorder.Clear();
            SFakeProxy* other = rig.fake.FindProxy( "xdg_wm_base" );
            rig.fake.SendPointerLeave( 6, rig.surface );
            ASSERT_FALSE( rig.context->IsPointerInside() );
            rig.fake.SendPointerEnter( 7, other, 100.0, 100.0 );
            rig.fake.SendPointerMotion( 3, 101.0, 101.0 );
            rig.fake.SendPointerButton( 8, 4, BTN_LEFT, 1 );
            ASSERT_TRUE( rig.recorder.events.empty() );
            ASSERT_FALSE( rig.context->IsPointerInside() );

            // before the pointer was ever there there is no position
            SInputRig fresh( 9, false );
            ASSERT_TRUE( fresh.Start() );
            ASSERT_FALSE( fresh.context->GetMousePosition( x, y ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Pointer positions are scaled by the scale of the window, like the buffer is" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.connection.SetSurfaceScale( rig.surfaceId, 180 );
            rig.recorder.Clear();
            rig.fake.SendPointerEnter( 5, rig.surface, 100.0, 40.0 );
            ASSERT_TRUE( rig.recorder.Has( "move 150,60 d0,0" ) );
            rig.fake.SendPointerMotion( 1, 1279.0, 719.0 );                           // the last unit of a window of 1280 x 720 units
            ASSERT_TRUE( rig.recorder.Has( "move 1919,1079 d1769,1019" ) );
            rig.connection.SetSurfaceScale( rig.surfaceId, 240 );
            rig.fake.SendPointerMotion( 2, 10.0, 10.0 );
            ASSERT_TRUE( rig.recorder.Has( "move 20,20 d-1899,-1059" ) );
            // dragged to the left of and above the window
            rig.fake.SendPointerMotion( 3, -3.0, -0.5 );
            ASSERT_TRUE( rig.recorder.Has( "move -6,-1 d-26,-21" ) || rig.recorder.Has( "move -6,0 d-26,-20" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Buttons are reported once with the indices of the X11 driver and are let go when the pointer leaves" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            // a button before the pointer is in the window is nobody's
            rig.fake.SendPointerButton( 1, 1, BTN_LEFT, 1 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "down" ) );

            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            rig.recorder.Clear();
            rig.fake.SendPointerButton( 3, 1, BTN_LEFT, 1 );
            rig.fake.SendPointerButton( 3, 1, BTN_LEFT, 1 );             // twice: once
            rig.fake.SendPointerButton( 4, 2, BTN_RIGHT, 1 );
            rig.fake.SendPointerButton( 5, 3, BTN_MIDDLE, 1 );
            rig.fake.SendPointerButton( 6, 4, BTN_SIDE, 1 );
            rig.fake.SendPointerButton( 7, 5, BTN_EXTRA, 1 );
            rig.fake.SendPointerButton( 8, 6, 0x300, 1 );                // not a mouse button
            ASSERT_TRUE( 5 == rig.recorder.Count( "down" ) );
            ASSERT_TRUE( rig.recorder.Has( "down 0" ) && rig.recorder.Has( "down 1" ) && rig.recorder.Has( "down 2" ) && rig.recorder.Has( "down 3" ) && rig.recorder.Has( "down 4" ) );
            ASSERT_TRUE( rig.context->IsButtonDown( 0 ) && rig.context->IsButtonDown( 4 ) );
            ASSERT_FALSE( rig.context->IsButtonDown( 5 ) );

            rig.recorder.Clear();
            rig.fake.SendPointerButton( 9, 7, BTN_LEFT, 0 );
            rig.fake.SendPointerButton( 9, 7, BTN_LEFT, 0 );
            ASSERT_TRUE( 1 == rig.recorder.Count( "up" ) && rig.recorder.Has( "up 0" ) );
            ASSERT_FALSE( rig.context->IsButtonDown( 0 ) );

            // the focus goes with buttons held: they are let go, so no receiver keeps a button down
            rig.recorder.Clear();
            rig.fake.SendPointerLeave( 10, rig.surface );
            ASSERT_TRUE( 4 == rig.recorder.Count( "up" ) );
            ASSERT_TRUE( rig.recorder.Has( "up 1" ) && rig.recorder.Has( "up 2" ) && rig.recorder.Has( "up 3" ) && rig.recorder.Has( "up 4" ) );
            ASSERT_FALSE( rig.context->IsButtonDown( 4 ) );
            ASSERT_TRUE( 0 == rig.recorder.Count( "down" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A drag that leaves the window keeps its motion and nothing else happens until the button is released" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.fake.SendPointerEnter( 2, rig.surface, 100.0, 100.0 );
            rig.fake.SendPointerButton( 3, 1, BTN_LEFT, 1 );
            rig.recorder.Clear();

            // the compositor keeps the focus of the window while the button is down (an implicit grab): the pointer goes outside
            rig.fake.SendPointerMotion( 2, -40.0, 100.0 );
            rig.fake.SendPointerMotion( 3, 5000.0, 2000.0 );
            ASSERT_TRUE( rig.recorder.Has( "move -40,100 d-140,0" ) );
            ASSERT_TRUE( rig.recorder.Has( "move 5000,2000 d5040,1900" ) );
            ASSERT_TRUE( 2 == rig.recorder.events.size() );          // no button up, no capture lost, nothing but the motion
            ASSERT_TRUE( rig.context->IsButtonDown( 0 ) );

            // released outside: the release, and then the leave the grab was holding back, which has nothing left to let go
            rig.fake.SendPointerButton( 4, 4, BTN_LEFT, 0 );
            rig.fake.SendPointerLeave( 5, rig.surface );
            ASSERT_TRUE( 3 == rig.recorder.events.size() );
            ASSERT_TRUE( "up 0" == rig.recorder.Last() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The wheel is delivered when the frame ends, in notches, vertical up positive" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            rig.recorder.Clear();

            // a notch of a wheel: the compositor sends the legacy axis value, the discrete steps and the high resolution value, then the frame
            rig.fake.SendPointerAxisSource( 0 );
            rig.fake.SendPointerAxis( 1, 0, 15.0 );
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerAxisDiscrete( 0, 1 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );                // not before the frame
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 1 == rig.recorder.Count( "var" ) );
            ASSERT_TRUE( rig.recorder.Has( "var 0 -1 -1" ) );               // down is negative
            ASSERT_TRUE( -1 == rig.context->GetWheel() );

            rig.fake.SendPointerAxisValue120( 0, -240 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( rig.recorder.Has( "var 0 1 +2" ) );                // two notches up: the total is 1
            ASSERT_TRUE( 1 == rig.context->GetWheel() );

            // the tilt wheel
            rig.fake.SendPointerAxisValue120( 1, 120 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( rig.recorder.Has( "var 1 1 +1" ) );
            ASSERT_TRUE( 1 == rig.context->GetHorizontalWheel() );
            rig.fake.SendPointerAxisValue120( 1, -120 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( rig.recorder.Has( "var 1 0 -1" ) );

            // a touch pad: smooth values, a notch for 15 units, the rest waits for the next frame
            rig.recorder.Clear();
            rig.fake.SendPointerAxisSource( 1 );
            rig.fake.SendPointerAxis( 3, 0, -10.0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );
            rig.fake.SendPointerAxis( 4, 0, -10.0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( rig.recorder.Has( "var 0 2 +1" ) );
            rig.fake.SendPointerAxisStop( 5, 0 );
            rig.fake.SendPointerFrame();
            rig.fake.SendPointerAxis( 6, 0, -10.0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 1 == rig.recorder.Count( "var" ) );                // the stop dropped the 5 that were left

            // a wheel outside the window is not for it
            rig.fake.SendPointerLeave( 9, rig.surface );
            rig.recorder.Clear();
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A seat at version 9 sends every wheel event of the newest version, relative direction included, and the listener has a function for each" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            ASSERT_TRUE( 0 == rig.fake.rejectedListenerCount );         // every listener the code gave covers all events of the version of its proxy
            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            rig.recorder.Clear();

            // mutter, a notch of a wheel down: source, distance, 120ths, direction, frame
            rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
            rig.fake.SendPointerAxis( 1, 0, 15.0 );
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerAxisRelativeDirection( 0, GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 1 == rig.recorder.Count( "var" ) && rig.recorder.Has( "var 0 -1 -1" ) );
            ASSERT_TRUE( 0 == rig.context->GetAxisAccumulator().GetRelativeDirection( 0 ) );

            // natural scrolling: the direction is inverted and the notch is the one the compositor made of it
            rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
            rig.fake.SendPointerAxis( 2, 0, -10.0 );
            rig.fake.SendPointerAxisValue120( 0, -120 );
            rig.fake.SendPointerAxisRelativeDirection( 0, GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( rig.recorder.Has( "var 0 0 +1" ) );
            ASSERT_TRUE( 1 == rig.context->GetAxisAccumulator().GetRelativeDirection( 0 ) );

            // the other axis, and many in a row
            rig.recorder.Clear();
            for ( int i=0; i<50; ++i )
            {
                rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_WHEEL );
                rig.fake.SendPointerAxis( 3, 1, 10.0 );
                rig.fake.SendPointerAxisValue120( 1, 120 );
                rig.fake.SendPointerAxisRelativeDirection( 1, GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL );
                rig.fake.SendPointerFrame();
            }
            ASSERT_TRUE( 50 == rig.recorder.Count( "var 1" ) && 50 == rig.context->GetHorizontalWheel() );

            // a touch pad: fingers (continuous values, no 120ths), a frame for each movement, then the stop
            rig.recorder.Clear();
            for ( int i=0; i<10; ++i )
            {
                rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_FINGER );
                rig.fake.SendPointerAxis( 4, 0, 4.5 );
                rig.fake.SendPointerAxisRelativeDirection( 0, GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED );
                rig.fake.SendPointerFrame();
            }
            ASSERT_TRUE( 3 == rig.recorder.Count( "var 0" ) && rig.recorder.Has( "var 0 -2 -1" ) );          // 45 units: three notches down
            rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_FINGER );
            rig.fake.SendPointerAxisStop( 5, 0 );
            rig.fake.SendPointerFrame();

            // a continuous source (a trackpoint with a button held): the same arithmetic
            rig.recorder.Clear();
            rig.fake.SendPointerAxisSource( GUCEF_WL_WL_POINTER_AXIS_SOURCE_CONTINUOUS );
            rig.fake.SendPointerAxis( 6, 0, -30.0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 1 == rig.recorder.Count( "var 0" ) );
            ASSERT_TRUE( 0 == rig.fake.rejectedListenerCount );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A listener that lacks a function for an event of its version is refused instead of being handed to libwayland, which would abort the process on that event" )
        try
        {
            CFakeWayland fake;
            fake.rejectionsExpected = true;
            fake.OfferDefaultGlobals();
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            UseNoCursorTheme( connection );
            ASSERT_TRUE( connection.Connect() );
            ASSERT_TRUE( 0 == fake.rejectedListenerCount );

            struct wl_pointer* pointer = connection.GetPointer();
            ASSERT_TRUE( GUCEF_NULL != pointer );
            const SWaylandApi& api = fake.GetApi();
            ASSERT_TRUE( 9 == api.proxy_get_version( (struct wl_proxy*) pointer ) );

            // the listener the code had: ten functions of twelve, the rest NULL
            struct gucef_wl_pointer_listener partial;
            memset( &partial, 0, sizeof( partial ) );
            void ( **slots )( void ) = (void ( ** )( void )) &partial;
            for ( int i=0; i<10; ++i )
                slots[ i ] = (void ( * )( void )) &fake;
            ASSERT_TRUE( -1 == gucef_wl_pointer_add_listener( &api, pointer, &partial, GUCEF_NULL ) );
            ASSERT_TRUE( 1 == fake.rejectedListenerCount && "wl_pointer" == fake.lastRejectedListener );

            // with the function for the event of version 9 it is accepted; warp (version 11) is not owed to a pointer of version 9
            slots[ 10 ] = (void ( * )( void )) &fake;
            ASSERT_TRUE( 0 == gucef_wl_pointer_add_listener( &api, pointer, &partial, GUCEF_NULL ) );
            ASSERT_TRUE( 1 == fake.rejectedListenerCount );

            // a seat of an older version: the events of the newer ones are not owed
            SInputRig oldRig( 5, false );
            ASSERT_TRUE( oldRig.Start() );
            struct wl_pointer* oldPointer = oldRig.connection.GetPointer();
            ASSERT_TRUE( 5 == oldRig.fake.GetApi().proxy_get_version( (struct wl_proxy*) oldPointer ) );
            slots[ 9 ] = GUCEF_NULL;
            slots[ 10 ] = GUCEF_NULL;
            ASSERT_TRUE( 0 == gucef_wl_pointer_add_listener( &oldRig.fake.GetApi(), oldPointer, &partial, GUCEF_NULL ) );
            ASSERT_TRUE( 0 == oldRig.fake.rejectedListenerCount );
            connection.Disconnect();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Axes that do not exist, frames with nothing in them, and events before the pointer is in the window do nothing and break nothing" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );

            // nothing is in the window yet
            rig.fake.SendPointerAxisSource( 0 );
            rig.fake.SendPointerAxis( 1, 0, 15.0 );
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerAxisRelativeDirection( 0, 0 );
            rig.fake.SendPointerFrame();
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );

            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            rig.recorder.Clear();
            rig.fake.SendPointerFrame();
            rig.fake.SendPointerFrame();
            rig.fake.SendPointerAxisStop( 3, 0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );

            // axes a compositor may invent
            const UInt32 odd[] = { 2, 3, 7, 255, 0x80000000u, 0xFFFFFFFFu };
            for ( UInt32 i=0; i<sizeof( odd ) / sizeof( odd[ 0 ] ); ++i )
            {
                rig.fake.SendPointerAxis( 1, odd[ i ], 150.0 );
                rig.fake.SendPointerAxisValue120( odd[ i ], 1200 );
                rig.fake.SendPointerAxisDiscrete( odd[ i ], 10 );
                rig.fake.SendPointerAxisStop( 4, odd[ i ] );
                rig.fake.SendPointerAxisRelativeDirection( odd[ i ], 1 );
                rig.fake.SendPointerFrame();
            }
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );
            ASSERT_TRUE( 0 == rig.context->GetAxisAccumulator().GetRelativeDirection( 2 ) );

            // a source that does not exist
            rig.fake.SendPointerAxisSource( 99 );
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 1 == rig.recorder.Count( "var" ) );

            // after the pointer left, a late wheel is not for the window; neither is a frame that closes it
            rig.fake.SendPointerLeave( 9, rig.surface );
            rig.recorder.Clear();
            rig.fake.SendPointerAxisSource( 0 );
            rig.fake.SendPointerAxis( 5, 0, 15.0 );
            rig.fake.SendPointerAxisValue120( 0, 120 );
            rig.fake.SendPointerAxisRelativeDirection( 0, 0 );
            rig.fake.SendPointerFrame();
            ASSERT_TRUE( 0 == rig.recorder.Count( "var" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A flood of wheel events cannot overflow the counts: a group gives a bounded number of notches" )
        try
        {
            CAxisAccumulator accumulator;
            Int32 vertical = 0, horizontal = 0;
            for ( int i=0; i<8; ++i )
            {
                accumulator.OnAxisValue120( 0, 2147483647 );
                accumulator.OnAxisDiscrete( 1, 2147483647 );
            }
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( vertical == -CAxisAccumulator::MaxNotchesPerGroup && horizontal == CAxisAccumulator::MaxNotchesPerGroup );
            // what was cut off is not owed to the next group
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );

            accumulator.OnAxis( 0, 8388607.0 );
            accumulator.OnAxis( 0, 8388607.0 );
            accumulator.OnAxis( 1, -8388607.0 );
            ASSERT_TRUE( accumulator.Flush( vertical, horizontal ) );
            ASSERT_TRUE( vertical == -CAxisAccumulator::MaxNotchesPerGroup && horizontal == -CAxisAccumulator::MaxNotchesPerGroup );
            ASSERT_FALSE( accumulator.Flush( vertical, horizontal ) );

            // and the totals of a context wrap around instead of overflowing
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            for ( int i=0; i<3; ++i )
            {
                rig.fake.SendPointerAxisValue120( 0, 2147483647 );
                rig.fake.SendPointerAxisValue120( 0, 2147483647 );
                rig.fake.SendPointerFrame();
            }
            ASSERT_TRUE( 3 == rig.recorder.Count( "var" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What a receiver of the wheel does cannot reach libwayland: an exception stops at the boundary and a receiver that destroys the input context is safe" )
        try
        {
            // the receiver throws: the context goes on and counts
            {
                SInputRig rig( 9, false );
                rig.recorder.callbacks.onMouseVarChanged = ThrowingVar;
                ASSERT_TRUE( rig.Start() );
                rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerAxisValue120( 1, 120 );
                rig.fake.SendPointerFrame();
                ASSERT_TRUE( -1 == rig.context->GetWheel() && 1 == rig.context->GetHorizontalWheel() );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerFrame();
                ASSERT_TRUE( -2 == rig.context->GetWheel() );
            }

            // the receiver takes the context away from inside the event; later events find no listener and the connection still works
            {
                SInputRig rig( 9, false );
                rig.recorder.callbacks.onMouseVarChanged = DestroyingVar;
                ASSERT_TRUE( rig.Start() );
                g_rigToDestroy = &rig;
                rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerAxisValue120( 1, 120 );
                rig.fake.SendPointerFrame();
                g_rigToDestroy = GUCEF_NULL;
                ASSERT_TRUE( GUCEF_NULL == rig.context );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerFrame();
                rig.fake.SendPointerMotion( 3, 6.0, 6.0 );
                ASSERT_TRUE( rig.connection.IsHealthy() );
            }

            // a listener of the seat that throws: the connection drops the exception at the boundary and goes on with the next event
            {
                CFakeWayland fake;
                fake.OfferDefaultGlobals();
                CWaylandConnection connection( fake.GetApi() );
                connection.SetPumpOnPulse( false );
                UseNoCursorTheme( connection );
                ASSERT_TRUE( connection.Connect() );
                CThrowingSeatListener thrower;
                CWireFrameCounter counter;
                connection.AddSeatListener( &thrower );
                connection.AddSeatListener( &counter );
                CEventGuard::ResetExceptionLog();
                fake.SendPointerAxisRelativeDirection( 0, 1 );
                fake.SendPointerAxisValue120( 0, 120 );
                fake.SendPointerFrame();
                fake.SendPointerFrame();
                ASSERT_TRUE( 4 == CEventGuard::GetExceptionCount() );
                ASSERT_TRUE( 2 == thrower.frames );
                // the dispatch is over: listeners can still come and go
                connection.RemoveSeatListener( &thrower );
                fake.SendPointerFrame();
                ASSERT_TRUE( 2 == thrower.frames && 1 == counter.frames );
                connection.RemoveSeatListener( &counter );
                connection.Disconnect();
            }
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The user action log has one line for each wheel notch group the window got, with the position" )
        try
        {
            CLogCapture capture;
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &capture );
            {
                SInputRig rig( 9, false );
                ASSERT_TRUE( rig.Start() );
                rig.fake.SendPointerEnter( 2, rig.surface, 12.0, 34.0 );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerFrame();
                rig.fake.SendPointerAxisValue120( 0, -360 );
                rig.fake.SendPointerFrame();
                rig.fake.SendPointerAxisValue120( 1, 240 );
                rig.fake.SendPointerFrame();
                // a part of a notch is no notch and no line
                rig.fake.SendPointerAxisValue120( 0, 40 );
                rig.fake.SendPointerFrame();
                // an event for nobody
                rig.fake.SendPointerLeave( 9, rig.surface );
                rig.fake.SendPointerAxisValue120( 0, 120 );
                rig.fake.SendPointerFrame();
            }
            CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &capture );
            ASSERT_TRUE( 1 == capture.Count( "Wayland input: wheel notch -1 at 12,34" ) );
            ASSERT_TRUE( 1 == capture.Count( "Wayland input: wheel notch +3 at 12,34" ) );
            ASSERT_TRUE( 1 == capture.Count( "Wayland input: horizontal wheel notch +2 at 12,34" ) );
            ASSERT_TRUE( 3 == capture.Count( "Wayland input: " ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor older than version 5 sends no frames: each axis event is delivered at once" )
        try
        {
            SInputRig rig( 4, false );
            ASSERT_TRUE( rig.Start() );
            rig.fake.SendPointerEnter( 2, rig.surface, 5.0, 5.0 );
            rig.recorder.Clear();
            rig.fake.SendPointerAxis( 1, 0, -15.0 );
            ASSERT_TRUE( rig.recorder.Has( "var 0 1 +1" ) );
            rig.fake.SendPointerAxis( 2, 0, 15.0 );
            ASSERT_TRUE( rig.recorder.Has( "var 0 0 -1" ) );
            rig.fake.SendPointerAxis( 3, 1, 30.0 );
            ASSERT_TRUE( rig.recorder.Has( "var 1 2 +2" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Keys have the codes of the X11 driver, the text the layout makes, and the modifiers" )
        try
        {
            SInputRig rig;
            ASSERT_TRUE( rig.Start() );
            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.Focus();
            ASSERT_TRUE( rig.context->IsKeyboardFocused() );

            rig.fake.SendKeyboardKey( 3, 1, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_A ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( "text a" ) );
            ASSERT_TRUE( rig.context->IsKeyDown( KEYCODE_A ) );
            ASSERT_TRUE( 0 != rig.context->GetKeyStates()[ KEYCODE_A ] );
            rig.fake.SendKeyboardKey( 4, 2, TESTKEY_A, 0 );
            ASSERT_TRUE( rig.recorder.Has( KeyUp( KEYCODE_A ).C_String() ) );
            ASSERT_FALSE( rig.context->IsKeyDown( KEYCODE_A ) );

            // Shift: the modifier key, then the letter in capitals; the code is that of the key without Shift
            rig.recorder.Clear();
            rig.fake.SendKeyboardKey( 5, 3, TESTKEY_LSHIFT, 1 );
            rig.fake.SendKeyboardModifiers( 6, 1, 0, 0, 0 );
            rig.fake.SendKeyboardKey( 7, 4, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_LSHIFT ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_A, KEYMOD_SHIFT ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( "text A" ) );
            rig.fake.SendKeyboardKey( 8, 5, TESTKEY_A, 0 );
            rig.fake.SendKeyboardKey( 9, 6, TESTKEY_1, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_1, KEYMOD_SHIFT ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( "text !" ) );
            rig.fake.SendKeyboardKey( 10, 7, TESTKEY_1, 0 );
            rig.fake.SendKeyboardModifiers( 11, 0, 0, 0, 0 );
            rig.fake.SendKeyboardKey( 12, 8, TESTKEY_LSHIFT, 0 );
            ASSERT_TRUE( rig.recorder.Has( KeyUp( KEYCODE_LSHIFT ).C_String() ) );

            // keys that are keys and not text
            rig.recorder.Clear();
            rig.fake.SendKeyboardKey( 13, 9, TESTKEY_ENTER, 1 );
            rig.fake.SendKeyboardKey( 14, 10, TESTKEY_LEFT, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_RETURN ).C_String() ) && rig.recorder.Has( KeyDown( KEYCODE_LEFT ).C_String() ) );
            ASSERT_TRUE( 0 == rig.recorder.Count( "text" ) );
            rig.fake.SendKeyboardKey( 15, 11, TESTKEY_SPACE, 1 );
            ASSERT_TRUE( rig.recorder.Has( "text  " ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: On a layout without Latin letters a key still has the code of its place on the other layout" )
        try
        {
            SInputRig rig;
            ASSERT_TRUE( rig.Start() );
            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.Focus();
            rig.fake.SendKeyboardModifiers( 3, 0, 0, 0, 1 );                // the second layout
            rig.fake.SendKeyboardKey( 4, 1, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_A ).C_String() ) );
            ASSERT_TRUE( rig.recorder.Has( "text \xD1\x84" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A held key repeats at the rate of the compositor, with its text, and stops when it is released or another is pressed" )
        try
        {
            SInputRig rig;
            rig.fake.repeatRate = 25;
            rig.fake.repeatDelay = 400;
            ASSERT_TRUE( rig.Start() );
            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.Focus();

            g_clock = 1000;
            rig.fake.SendKeyboardKey( 3, 1, TESTKEY_A, 1 );
            ASSERT_TRUE( 1 == rig.recorder.Count( "keydown" ) );
            rig.context->Update( 1399 );
            ASSERT_TRUE( 1 == rig.recorder.Count( "keydown" ) );
            rig.context->Update( 1400 );
            ASSERT_TRUE( 2 == rig.recorder.Count( "keydown" ) );
            ASSERT_TRUE( 2 == rig.recorder.Count( "text a" ) );
            rig.context->Update( 1440 );
            rig.context->Update( 1481 );
            ASSERT_TRUE( 4 == rig.recorder.Count( "keydown" ) );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keyup" ) );

            // another key takes over
            rig.fake.SendKeyboardKey( 4, 2, TESTKEY_S, 1 );
            g_clock = 1500;
            rig.fake.SendKeyboardKey( 5, 3, TESTKEY_S, 1 );        // (a repeat of the compositor itself is no new press)
            rig.recorder.Clear();
            rig.context->Update( 1900 );
            ASSERT_TRUE( 0 == rig.recorder.Count( ( "keydown " + CORE::UInt32ToString( KEYCODE_A ) ).C_String() ) );

            // released: it stops
            rig.fake.SendKeyboardKey( 6, 4, TESTKEY_S, 0 );
            rig.fake.SendKeyboardKey( 7, 5, TESTKEY_A, 0 );
            rig.recorder.Clear();
            rig.context->Update( 5000 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );

            // a modifier does not repeat
            rig.fake.SendKeyboardKey( 8, 6, TESTKEY_LSHIFT, 1 );
            rig.recorder.Clear();
            rig.context->Update( 9000 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );
            rig.fake.SendKeyboardKey( 9, 7, TESTKEY_LSHIFT, 0 );

            // the compositor turns repeating off, and later on again at another speed
            rig.fake.SendKeyboardRepeatInfo( 0, 0 );
            g_clock = 10000;
            rig.fake.SendKeyboardKey( 10, 8, TESTKEY_A, 1 );
            rig.recorder.Clear();
            rig.context->Update( 20000 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );
            rig.fake.SendKeyboardKey( 11, 9, TESTKEY_A, 0 );
            rig.fake.SendKeyboardRepeatInfo( 10, 100 );
            g_clock = 30000;
            rig.fake.SendKeyboardKey( 12, 10, TESTKEY_A, 1 );
            rig.recorder.Clear();
            rig.context->Update( 30100 );
            ASSERT_TRUE( 1 == rig.recorder.Count( "keydown" ) );
            rig.context->Update( 30200 );
            ASSERT_TRUE( 2 == rig.recorder.Count( "keydown" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Losing the keyboard focus lets go of the keys that are held, stops the repeat, and ignores keys until the focus is back" )
        try
        {
            SInputRig rig;
            ASSERT_TRUE( rig.Start() );
            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.Focus();
            g_clock = 1000;
            rig.fake.SendKeyboardKey( 3, 1, TESTKEY_LSHIFT, 1 );
            rig.fake.SendKeyboardKey( 4, 2, TESTKEY_A, 1 );
            rig.recorder.Clear();

            rig.fake.SendKeyboardLeave( 5, rig.surface );
            ASSERT_FALSE( rig.context->IsKeyboardFocused() );
            ASSERT_TRUE( 2 == rig.recorder.Count( "keyup" ) );
            ASSERT_TRUE( rig.recorder.Has( KeyUp( KEYCODE_A ).C_String() ) && rig.recorder.Has( KeyUp( KEYCODE_LSHIFT ).C_String() ) );
            ASSERT_FALSE( rig.context->IsKeyDown( KEYCODE_A ) );
            rig.recorder.Clear();
            rig.context->Update( 9000 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );

            // keys of a window that is not ours, or while there is no focus
            rig.fake.SendKeyboardKey( 6, 3, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.events.empty() );
            rig.fake.SendKeyboardEnter( 7, rig.fake.FindProxy( "xdg_wm_base" ) );
            rig.fake.SendKeyboardKey( 8, 4, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.events.empty() );
            rig.fake.SendKeyboardEnter( 9, rig.surface );
            rig.fake.SendKeyboardKey( 10, 5, TESTKEY_A, 1 );
            ASSERT_TRUE( rig.recorder.Has( KeyDown( KEYCODE_A ).C_String() ) );

            // the release of a key that was held before the focus came is not a release of ours
            rig.fake.SendKeyboardKey( 11, 6, TESTKEY_S, 0 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keyup" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Keys that arrive before the keymap are ignored, and a keymap that cannot be read leaves the keyboard unusable but the pointer alone" )
        try
        {
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.Focus();
            rig.fake.SendKeyboardKey( 3, 1, TESTKEY_A, 1 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );
            GUCEF_TESTFW_SKIP_IF( !rig.hasXkb, "libxkbcommon.so.0 is not installed" );
            rig.fake.SendKeyboardKeymap( "this is not a keymap" );
            rig.fake.SendKeyboardKey( 4, 2, TESTKEY_A, 1 );
            ASSERT_TRUE( 0 == rig.recorder.Count( "keydown" ) );
            rig.fake.SendPointerEnter( 5, rig.surface, 3.0, 4.0 );
            ASSERT_TRUE( rig.recorder.Has( "move 3,4 d3,4" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The input of a window finds the surface of the window context of the Wayland window driver" )
        try
        {
            CFakeWayland fake;
            CFakeGraphics graphics;
            fake.OfferDefaultGlobals();
            fake.OfferGlobal( "wp_viewporter", 1 );
            fake.OfferGlobal( "wp_fractional_scale_manager_v1", 1 );
            fake.preferredFractionalScale = 180;
            CWaylandConnection connection( fake.GetApi() );
            connection.SetPumpOnPulse( false );
            UseNoCursorTheme( connection );
            ASSERT_TRUE( connection.Connect() );

            GUCEF::GUIDRIVERWAYLAND::CWaylandWindowContext window( connection, graphics.GetRuntime() );
            GUI::CVideoSettings settings;
            settings.SetResolution( 1280, 720, 32 );
            GUCEF::GUIDRIVERWAYLAND::SWaylandWindowParams params;
            ASSERT_TRUE( window.Initialize( "Redux", settings, params ) );

            // what Redux does: the WINDOW property of the window context is the parameter of the input context
            const UInt32 id = CORE::StringToUInt32( window.GetProperty( "WINDOW" ), 0 );
            CInputRecorder recorder;
            CWaylandInputContext input( connection, id, recorder.callbacks, GUCEF_NULL, false );
            recorder.Clear();

            SFakeProxy* surface = fake.FindProxy( "wl_surface" );
            fake.SendPointerEnter( 7, surface, 640.0, 360.0 );
            // the window is 1920 x 1080 pixels at a scale of 1.5: the centre of the window in its own pixels
            ASSERT_TRUE( 1920 == window.GetWidth() );
            ASSERT_TRUE( recorder.Has( "move 960,540 d0,0" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The plugin interface names the driver, refuses a context without a window or a session, and answers queries from a context" )
        try
        {
            ASSERT_TRUE( 0 == strcmp( "WAYLAND", INPUT::WAYLAND::INPUTDRIVERPLUG_Name( GUCEF_NULL ) ) );
            ASSERT_TRUE( 1 == INPUT::WAYLAND::INPUTDRIVERPLUG_Version( GUCEF_NULL )->major );

            // no window parameter
            TInputCallbacks callbacks;
            memset( &callbacks, 0, sizeof( callbacks ) );
            void* contextData = GUCEF_NULL;
            ASSERT_TRUE( 0 == INPUT::WAYLAND::INPUTDRIVERPLUG_CreateContext( GUCEF_NULL, &contextData, 0, GUCEF_NULL, &callbacks ) );

            // a window but no session: nothing connects (the socket named does not exist)
            const char* oldDisplay = ::getenv( "WAYLAND_DISPLAY" );
            const CORE::CString saved = GUCEF_NULL != oldDisplay ? oldDisplay : "";
            ::setenv( "WAYLAND_DISPLAY", "/nonexistent-directory/wayland-test-socket", 1 );
            const char* argv[] = { "-WINDOW", "1" };
            const UInt32 created = INPUT::WAYLAND::INPUTDRIVERPLUG_CreateContext( GUCEF_NULL, &contextData, 2, argv, &callbacks );
            if ( GUCEF_NULL != oldDisplay ) ::setenv( "WAYLAND_DISPLAY", saved.C_String(), 1 ); else ::unsetenv( "WAYLAND_DISPLAY" );
            ASSERT_TRUE( 0 == created );
            ASSERT_TRUE( GUCEF_NULL == contextData );

            // queries of a context
            SInputRig rig( 9, false );
            ASSERT_TRUE( rig.Start() );
            rig.fake.SendPointerEnter( 2, rig.surface, 30.0, 40.0 );
            rig.fake.SendPointerButton( 3, 1, BTN_MIDDLE, 1 );
            UInt32 x = 0, y = 0;
            ASSERT_TRUE( 1 == INPUT::WAYLAND::INPUTDRIVERPLUG_GetMousePos( GUCEF_NULL, rig.context, &x, &y ) );
            ASSERT_TRUE( 30 == x && 40 == y );
            ASSERT_TRUE( 1 == INPUT::WAYLAND::INPUTDRIVERPLUG_GetMouseButtonPressedState( GUCEF_NULL, rig.context, 2 ) );
            ASSERT_TRUE( 0 == INPUT::WAYLAND::INPUTDRIVERPLUG_GetMouseButtonPressedState( GUCEF_NULL, rig.context, 0 ) );
            ASSERT_TRUE( GUCEF_NULL != INPUT::WAYLAND::INPUTDRIVERPLUG_GetKeyBoardKeyStates( GUCEF_NULL, rig.context ) );
            ASSERT_TRUE( 0 == INPUT::WAYLAND::INPUTDRIVERPLUG_GetKeyboardKeyPressedState( GUCEF_NULL, rig.context, KEYCODE_A ) );
            ASSERT_TRUE( 1 == INPUT::WAYLAND::INPUTDRIVERPLUG_Update( GUCEF_NULL, rig.context ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip is not an error
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
