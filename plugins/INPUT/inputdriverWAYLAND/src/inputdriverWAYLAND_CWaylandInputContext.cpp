/*
 *  inputdriverWAYLAND: GUCEF input driver for Wayland: the pointer and keyboard of the seat for the windows of guidriverWAYLAND
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

#include <string.h>
#include <exception>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CKEYMAPPING_H
#include "inputdriverWAYLAND_CKeyMapping.h"
#define GUCEF_INPUT_WAYLAND_CKEYMAPPING_H
#endif /* GUCEF_INPUT_WAYLAND_CKEYMAPPING_H ? */

#include "inputdriverWAYLAND_CWaylandInputContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace WAYLAND {

using namespace PLUGINGLUE::WAYLAND;

namespace {

/** A sum that wraps around instead of overflowing, for the running totals of the wheel */
Int32
WrapAdd( Int32 total, Int32 delta )
{
    return (Int32) ( (UInt32) total + (UInt32) delta );
}

/**
 *  Tells the receiver of the wheel and the log of what the user does. The receiver runs the application's code from inside the
 *  events of the compositor: what it throws stops here, and the callbacks are those of a copy, so a receiver that takes the
 *  window and with it the input context away leaves nothing to touch afterwards.
 */
void
DeliverWheel( const TInputCallbacks& callbacks, UInt32 variable, Int32 total, Int32 delta, Int32 x, Int32 y )
{
    const char* notch = 0 == variable ? "wheel notch " : "horizontal wheel notch ";
    const char* sign = delta > 0 ? "+" : "";
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Wayland input: " << notch << sign << delta << " at " << x << "," << y );
    if ( GUCEF_NULL == callbacks.onMouseVarChanged )
        return;
    try
    {
        callbacks.onMouseVarChanged( callbacks.userData, 0, variable, total, delta );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandInputContext: The receiver of the wheel threw an exception: " << e.what() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandInputContext: The receiver of the wheel threw an exception" );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWaylandInputContext::CWaylandInputContext( CWaylandConnection& connection ,
                                            UInt32 surfaceId               ,
                                            const TInputCallbacks& callbacks ,
                                            const SXkbApi* xkb             ,
                                            bool pulseDriven               )
    : CORE::CObservingNotifier()
    , m_connection( connection )
    , m_surfaceId( surfaceId )
    , m_callbacks( callbacks )
    , m_releaseConnection( false )
    , m_pulsing( false )
    , m_clock( &CORE::GUCEFGetTickCount )
    , m_hasPointer( false )
    , m_hasKeyboard( false )
    , m_pointerInside( false )
    , m_hasPosition( false )
    , m_mouseX( 0 )
    , m_mouseY( 0 )
    , m_buttonsDown( 0 )
    , m_wheel( 0 )
    , m_horizontalWheel( 0 )
    , m_framesExpected( connection.GetSeatVersion() >= GUCEF_WL_WL_POINTER_FRAME_EVENT_SINCE_VERSION )
    , m_axis()
    , m_keyboardFocused( false )
    , m_hasXkbApi( false )
    , m_xkb( GUCEF_NULL )
    , m_repeat()
    , m_modifierMask( 0 )
    , m_heldCount( 0 )
{GUCEF_TRACE;

    memset( &m_xkbApi, 0, sizeof( m_xkbApi ) );
    memset( m_held, 0, sizeof( m_held ) );
    memset( m_keyStates, 0, sizeof( m_keyStates ) );

    if ( GUCEF_NULL != xkb )
    {
        m_xkbApi = *xkb;
        m_hasXkbApi = true;
    }
    else
        m_hasXkbApi = LoadXkbApi( m_xkbApi );
    if ( m_hasXkbApi )
        m_xkb = new CXkbKeyboard( m_xkbApi );
    else
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandInputContext: libxkbcommon.so.0 is not available: the keyboard cannot be used" );

    if ( pulseDriven )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        pulseGenerator->RequestPeriodicPulses( this, CWaylandConnection::PumpIntervalInMs );
        m_pulsing = true;
    }

    // the seat tells what it has at once: the devices, the keymap, the repeat rate and where the focus is
    m_connection.AddSeatListener( this );
}

/*-------------------------------------------------------------------------*/

CWaylandInputContext::~CWaylandInputContext()
{GUCEF_TRACE;

    m_connection.RemoveSeatListener( this );
    if ( m_pulsing )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
    }
    delete m_xkb;
    m_xkb = GUCEF_NULL;
    if ( m_releaseConnection )
        CWaylandConnection::Release( &m_connection );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::SetClock( TClock clock )
{GUCEF_TRACE;

    m_clock = GUCEF_NULL != clock ? clock : &CORE::GUCEFGetTickCount;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::SetReleaseConnectionOnDestroy( bool release )
{GUCEF_TRACE;

    m_releaseConnection = release;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandInputContext::IsOwnSurface( const struct wl_surface* surface ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == surface )
        return false;
    PLUGINGLUE::WAYLAND::SWaylandSurfaceInfo* info = m_connection.FindSurface( m_surfaceId );
    return GUCEF_NULL != info && info->surface == surface;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandInputContext::GetScale120( void ) const
{GUCEF_TRACE;

    PLUGINGLUE::WAYLAND::SWaylandSurfaceInfo* info = m_connection.FindSurface( m_surfaceId );
    return GUCEF_NULL != info ? info->scale120 : CWaylandScale::Denominator;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnSeatCapabilities( UInt32 capabilities )
{GUCEF_TRACE;

    const bool pointer = 0 != ( capabilities & GUCEF_WL_WL_SEAT_CAPABILITY_POINTER );
    const bool keyboard = 0 != ( capabilities & GUCEF_WL_WL_SEAT_CAPABILITY_KEYBOARD );

    if ( pointer && !m_hasPointer )
    {
        m_hasPointer = true;
        if ( GUCEF_NULL != m_callbacks.onMouseAttached )
            m_callbacks.onMouseAttached( m_callbacks.userData, 0 );
    }
    else
    if ( !pointer && m_hasPointer )
    {
        ReleaseButtons();
        m_pointerInside = false;
        m_axis.Reset();
        m_hasPointer = false;
        if ( GUCEF_NULL != m_callbacks.onMouseDetached )
            m_callbacks.onMouseDetached( m_callbacks.userData, 0 );
    }

    if ( keyboard && !m_hasKeyboard )
    {
        m_hasKeyboard = true;
        if ( GUCEF_NULL != m_callbacks.onKeyboardAttached )
            m_callbacks.onKeyboardAttached( m_callbacks.userData, 0 );
    }
    else
    if ( !keyboard && m_hasKeyboard )
    {
        ReleaseKeys();
        m_keyboardFocused = false;
        m_hasKeyboard = false;
        if ( GUCEF_NULL != m_callbacks.onKeyboardDetached )
            m_callbacks.onKeyboardDetached( m_callbacks.userData, 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::MoveTo( Int32 x, Int32 y )
{GUCEF_TRACE;

    const Int32 deltaX = m_hasPosition ? x - m_mouseX : 0;
    const Int32 deltaY = m_hasPosition ? y - m_mouseY : 0;
    m_mouseX = x;
    m_mouseY = y;
    m_hasPosition = true;
    if ( GUCEF_NULL != m_callbacks.onMouseMove )
        m_callbacks.onMouseMove( m_callbacks.userData, 0, x, y, deltaX, deltaY );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerEnter( UInt32 serial, struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y )
{GUCEF_TRACE;

    if ( !IsOwnSurface( surface ) )
        return;
    m_pointerInside = true;
    const UInt32 scale = GetScale120();
    MoveTo( CWaylandScale::PointerToBuffer( x, scale ), CWaylandScale::PointerToBuffer( y, scale ) );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerLeave( UInt32 serial, struct wl_surface* surface )
{GUCEF_TRACE;

    if ( !m_pointerInside || !IsOwnSurface( surface ) )
        return;
    m_pointerInside = false;

    // a leave while buttons are held does not come from a grab (that keeps the focus): the buttons are let go so nothing stays pressed
    ReleaseButtons();
    m_axis.Reset();
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerMotion( UInt32 timeInMs, wl_fixed_t x, wl_fixed_t y )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    const UInt32 scale = GetScale120();
    MoveTo( CWaylandScale::PointerToBuffer( x, scale ), CWaylandScale::PointerToBuffer( y, scale ) );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerButton( UInt32 serial, UInt32 timeInMs, UInt32 button, UInt32 state )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    const Int32 index = CKeyMapping::MapButton( button );
    if ( index < 0 || (UInt32) index >= MaxButtons )
        return;
    const UInt32 bit = 1u << index;
    const bool pressed = GUCEF_WL_WL_POINTER_BUTTON_STATE_PRESSED == state;
    if ( pressed )
    {
        if ( 0 != ( m_buttonsDown & bit ) )
            return;
        m_buttonsDown |= bit;
        if ( GUCEF_NULL != m_callbacks.onMouseButtonDown )
            m_callbacks.onMouseButtonDown( m_callbacks.userData, 0, (UInt32) index );
    }
    else
    {
        if ( 0 == ( m_buttonsDown & bit ) )
            return;
        m_buttonsDown &= ~bit;
        if ( GUCEF_NULL != m_callbacks.onMouseButtonUp )
            m_callbacks.onMouseButtonUp( m_callbacks.userData, 0, (UInt32) index );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::ReleaseButtons( void )
{GUCEF_TRACE;

    for ( UInt32 index=0; index<MaxButtons && 0 != m_buttonsDown; ++index )
    {
        const UInt32 bit = 1u << index;
        if ( 0 != ( m_buttonsDown & bit ) )
        {
            m_buttonsDown &= ~bit;
            if ( GUCEF_NULL != m_callbacks.onMouseButtonUp )
                m_callbacks.onMouseButtonUp( m_callbacks.userData, 0, index );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxis( UInt32 timeInMs, UInt32 axis, wl_fixed_t value )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    m_axis.OnAxis( axis, gucef_wl_fixed_to_double( value ) );
    if ( !m_framesExpected )
        OnPointerFrame();
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxisSource( UInt32 axisSource )
{GUCEF_TRACE;

    m_axis.OnAxisSource( axisSource );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxisStop( UInt32 timeInMs, UInt32 axis )
{GUCEF_TRACE;

    m_axis.OnAxisStop( axis );
    if ( !m_framesExpected )
        OnPointerFrame();
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxisDiscrete( UInt32 axis, Int32 discrete )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    m_axis.OnAxisDiscrete( axis, discrete );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxisValue120( UInt32 axis, Int32 value120 )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    m_axis.OnAxisValue120( axis, value120 );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerAxisRelativeDirection( UInt32 axis, UInt32 direction )
{GUCEF_TRACE;

    if ( !m_pointerInside )
        return;
    m_axis.OnAxisRelativeDirection( axis, direction );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnPointerFrame( void )
{GUCEF_TRACE;

    Int32 vertical = 0, horizontal = 0;
    if ( !m_axis.Flush( vertical, horizontal ) )
        return;

    // everything the delivery needs is taken before the first receiver is called: the receiver may take this context away
    const TInputCallbacks callbacks = m_callbacks;
    const Int32 x = m_mouseX;
    const Int32 y = m_mouseY;
    if ( 0 != vertical )
        m_wheel = WrapAdd( m_wheel, vertical );
    if ( 0 != horizontal )
        m_horizontalWheel = WrapAdd( m_horizontalWheel, horizontal );
    const Int32 verticalTotal = m_wheel;
    const Int32 horizontalTotal = m_horizontalWheel;

    if ( 0 != vertical )
        DeliverWheel( callbacks, 0, verticalTotal, vertical, x, y );
    if ( 0 != horizontal )
        DeliverWheel( callbacks, 1, horizontalTotal, horizontal, x, y );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardKeymap( UInt32 format, const char* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_xkb || GUCEF_WL_WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 != format )
        return;
    if ( !m_xkb->LoadKeymap( data, size ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandInputContext: The keymap of the compositor is not usable: keys have no codes and no text" );
    m_modifierMask = 0;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardEnter( UInt32 serial, struct wl_surface* surface, const UInt32* pressedKeys, UInt32 pressedKeyCount )
{GUCEF_TRACE;

    if ( !IsOwnSurface( surface ) )
        return;
    // keys that were held before the focus came are not ours: their releases are ignored
    m_keyboardFocused = true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardLeave( UInt32 serial, struct wl_surface* surface )
{GUCEF_TRACE;

    if ( !m_keyboardFocused || !IsOwnSurface( surface ) )
        return;
    m_keyboardFocused = false;
    ReleaseKeys();
    if ( GUCEF_NULL != m_xkb )
        m_xkb->ResetCompose();
}

/*-------------------------------------------------------------------------*/

Int32
CWaylandInputContext::FindHeldKey( UInt32 evdev ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_heldCount; ++i )
        if ( m_held[ i ].evdev == evdev )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::DeliverText( UInt32 evdev )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_callbacks.onKeyboardText || GUCEF_NULL == m_xkb )
        return;
    char text[ 64 ];
    if ( m_xkb->GetText( evdev, text, sizeof( text ) ) )
        m_callbacks.onKeyboardText( m_callbacks.userData, 0, text );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::DeliverKeyDown( UInt32 evdev, KeyCode code )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_callbacks.onKeyboardKeyDown )
        m_callbacks.onKeyboardKeyDown( m_callbacks.userData, 0, code, m_modifierMask );
    DeliverText( evdev );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardKey( UInt32 serial, UInt32 timeInMs, UInt32 key, UInt32 state )
{GUCEF_TRACE;

    if ( !m_keyboardFocused || GUCEF_NULL == m_xkb || !m_xkb->HasKeymap() )
        return;

    if ( GUCEF_WL_WL_KEYBOARD_KEY_STATE_RELEASED == state )
    {
        const Int32 held = FindHeldKey( key );
        if ( held < 0 )
            return;
        const KeyCode code = m_held[ held ].code;
        m_held[ held ] = m_held[ m_heldCount - 1 ];
        --m_heldCount;
        m_repeat.Stop( key );
        m_keyStates[ code ] = 0;
        if ( GUCEF_NULL != m_callbacks.onKeyboardKeyUp )
            m_callbacks.onKeyboardKeyUp( m_callbacks.userData, 0, code, m_modifierMask );
        return;
    }

    // pressed, or repeated by the compositor itself
    Int32 held = FindHeldKey( key );
    if ( held < 0 )
    {
        UInt32 keysyms[ 8 ];
        const UInt32 count = m_xkb->GetBaseKeysyms( key, keysyms, 8 );
        KeyCode code = KEYCODE_UNASSIGNED;
        for ( UInt32 i=0; i<count && KEYCODE_UNASSIGNED == code; ++i )
            code = CKeyMapping::MapKeysym( keysyms[ i ] );
        if ( m_heldCount >= MaxHeldKeys )
            return;
        m_held[ m_heldCount ].evdev = key;
        m_held[ m_heldCount ].code = code;
        held = (Int32) m_heldCount;
        ++m_heldCount;
        m_keyStates[ code ] = 1;

        if ( m_xkb->KeyRepeats( key ) && !CKeyMapping::IsModifierKey( code ) )
            m_repeat.Start( key, m_clock() );
        else
            m_repeat.StopAll();
    }
    DeliverKeyDown( key, m_held[ held ].code );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::ReleaseKeys( void )
{GUCEF_TRACE;

    m_repeat.StopAll();
    while ( m_heldCount > 0 )
    {
        --m_heldCount;
        const KeyCode code = m_held[ m_heldCount ].code;
        m_keyStates[ code ] = 0;
        if ( GUCEF_NULL != m_callbacks.onKeyboardKeyUp )
            m_callbacks.onKeyboardKeyUp( m_callbacks.userData, 0, code, m_modifierMask );
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardModifiers( UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_xkb )
        return;
    m_xkb->UpdateMask( depressed, latched, locked, group );
    m_modifierMask = m_xkb->GetModifierMask();
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnKeyboardRepeatInfo( Int32 rate, Int32 delayInMs )
{GUCEF_TRACE;

    m_repeat.SetRate( rate, delayInMs );
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::Update( UInt32 nowInMs )
{GUCEF_TRACE;

    if ( !m_repeat.IsRepeating() )
        return;
    UInt32 key = 0;
    const UInt32 due = m_repeat.Poll( nowInMs, key );
    if ( 0 == due )
        return;
    const Int32 held = FindHeldKey( key );
    if ( held < 0 )
    {
        m_repeat.StopAll();
        return;
    }
    for ( UInt32 i=0; i<due; ++i )
        DeliverKeyDown( key, m_held[ held ].code );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandInputContext::GetMousePosition( Int32& x, Int32& y ) const
{GUCEF_TRACE;

    if ( !m_hasPosition )
        return false;
    x = m_mouseX;
    y = m_mouseY;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandInputContext::IsButtonDown( UInt32 buttonIndex ) const
{GUCEF_TRACE;

    return buttonIndex < MaxButtons && 0 != ( m_buttonsDown & ( 1u << buttonIndex ) );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandInputContext::IsKeyDown( KeyCode key ) const
{GUCEF_TRACE;

    return (UInt32) key < KEYCODE_MAX && 0 != m_keyStates[ key ];
}

/*-------------------------------------------------------------------------*/

UInt8*
CWaylandInputContext::GetKeyStates( void )
{GUCEF_TRACE;

    return m_keyStates;
}

/*-------------------------------------------------------------------------*/

bool CWaylandInputContext::HasPointer( void ) const { return m_hasPointer; }
bool CWaylandInputContext::HasKeyboard( void ) const { return m_hasKeyboard; }
bool CWaylandInputContext::IsPointerInside( void ) const { return m_pointerInside; }
bool CWaylandInputContext::IsKeyboardFocused( void ) const { return m_keyboardFocused; }
Int32 CWaylandInputContext::GetWheel( void ) const { return m_wheel; }
Int32 CWaylandInputContext::GetHorizontalWheel( void ) const { return m_horizontalWheel; }
CAxisAccumulator& CWaylandInputContext::GetAxisAccumulator( void ) { return m_axis; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWaylandInputContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::INPUT::WAYLAND::CWaylandInputContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWaylandInputContext::OnNotify( CORE::CNotifier* notifier   ,
                                const CORE::CEvent& eventId ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventId )
        Update( m_clock() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
