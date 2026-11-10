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


#ifndef GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#define GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#include "inputdriverpluginstructs.h"
#define GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#endif /* GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#include "pluginglueWAYLAND_CWaylandScale.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H
#include "inputdriverWAYLAND_CAxisAccumulator.h"
#define GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H
#endif /* GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CKEYREPEAT_H
#include "inputdriverWAYLAND_CKeyRepeat.h"
#define GUCEF_INPUT_WAYLAND_CKEYREPEAT_H
#endif /* GUCEF_INPUT_WAYLAND_CKEYREPEAT_H ? */

#ifndef GUCEF_INPUT_WAYLAND_XKB_H
#include "inputdriverWAYLAND_Xkb.h"
#define GUCEF_INPUT_WAYLAND_XKB_H
#endif /* GUCEF_INPUT_WAYLAND_XKB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The input of one window: the pointer and the keyboard of the seat, for the surface of a window context of
 *  guidriverWAYLAND, as the callbacks of a gucefINPUT input driver. Device 0 is the mouse and the keyboard.
 *
 *  Pointer: positions are in pixels of the drawing buffer of the window, the surface local position of the compositor times
 *  the scale of the window (so a window of a monitor with a scale of 2 gets coordinates twice as large, which are those of
 *  GetWidth() / GetHeight() of the window context). Buttons: 0 left, 1 right, 2 middle, 3 and 4 the side buttons. The wheel
 *  is in notches, variable 0 the vertical one (up is positive) and 1 the horizontal one (right is positive).
 *
 *  Focus: entering the surface moves the pointer there; leaving it, or the keyboard focus going, lets go of the buttons and
 *  keys that are held (as releases, so the receivers never see a key stuck down). A pointer that keeps the focus because a
 *  button is held (an implicit grab) sends no leave while the button is down, so a drag that goes outside the window goes on.
 *  Nothing here ends a press in the GUI by itself: that is for the window context, which reports a real change of the
 *  activation of the window.
 *
 *  Keys: the key code is that of the X11 driver for the keysym of the key without modifiers; text is the UTF-8 the key types
 *  after the layout and compose sequences. A held key repeats as the compositor says, from the pulses of the application.
 */
class GUCEF_INPUT_PLUGIN_PUBLIC_CPP CWaylandInputContext : public CORE::CObservingNotifier ,
                                                           public PLUGINGLUE::WAYLAND::IWaylandSeatListener
{
    public:

    /**
     *  @param xkb the functions of libxkbcommon, NULL to load them; without the library the pointer works and the keyboard does not
     *  @param pulseDriven repeats keys on the pulses of the application (a test calls Update() itself)
     */
    CWaylandInputContext( PLUGINGLUE::WAYLAND::CWaylandConnection& connection ,
                          UInt32 surfaceId                                    ,
                          const TInputCallbacks& callbacks                    ,
                          const SXkbApi* xkb = GUCEF_NULL                     ,
                          bool pulseDriven = true                             );

    virtual ~CWaylandInputContext();

    /** The connection was acquired for this context: give it back when the context goes */
    void SetReleaseConnectionOnDestroy( bool release );

    /** The repeats of a held key that are due at 'nowInMs' (a tick count in milliseconds) */
    void Update( UInt32 nowInMs );

    /** Where the time of key repeats comes from: the tick count of the program unless a test says otherwise */
    typedef UInt32 ( *TClock )( void );

    void SetClock( TClock clock );

    /** The last position of the pointer in pixels; false while the pointer was never in the window */
    bool GetMousePosition( Int32& x, Int32& y ) const;

    bool IsButtonDown( UInt32 buttonIndex ) const;

    bool IsKeyDown( KeyCode key ) const;

    /** One byte per key code, non-zero for a key that is down */
    UInt8* GetKeyStates( void );

    bool HasPointer( void ) const;

    bool HasKeyboard( void ) const;

    bool IsPointerInside( void ) const;

    bool IsKeyboardFocused( void ) const;

    Int32 GetWheel( void ) const;

    Int32 GetHorizontalWheel( void ) const;

    CAxisAccumulator& GetAxisAccumulator( void );

    // --- IWaylandSeatListener

    virtual void OnSeatCapabilities( UInt32 capabilities ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerEnter( UInt32 serial, struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerLeave( UInt32 serial, struct wl_surface* surface ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerMotion( UInt32 timeInMs, wl_fixed_t x, wl_fixed_t y ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerButton( UInt32 serial, UInt32 timeInMs, UInt32 button, UInt32 state ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxis( UInt32 timeInMs, UInt32 axis, wl_fixed_t value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxisSource( UInt32 axisSource ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxisStop( UInt32 timeInMs, UInt32 axis ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxisDiscrete( UInt32 axis, Int32 discrete ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxisValue120( UInt32 axis, Int32 value120 ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerAxisRelativeDirection( UInt32 axis, UInt32 direction ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerFrame( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardKeymap( UInt32 format, const char* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardEnter( UInt32 serial, struct wl_surface* surface, const UInt32* pressedKeys, UInt32 pressedKeyCount ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardLeave( UInt32 serial, struct wl_surface* surface ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardKey( UInt32 serial, UInt32 timeInMs, UInt32 key, UInt32 state ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardModifiers( UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKeyboardRepeatInfo( Int32 rate, Int32 delayInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventId        ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWaylandInputContext( const CWaylandInputContext& src );
    CWaylandInputContext& operator=( const CWaylandInputContext& src );

    struct SHeldKey
    {
        UInt32 evdev;
        KeyCode code;
    };

    bool IsOwnSurface( const struct wl_surface* surface ) const;
    void MoveTo( Int32 x, Int32 y );
    void ReleaseButtons( void );
    void ReleaseKeys( void );
    void DeliverKeyDown( UInt32 evdev, KeyCode code );
    void DeliverText( UInt32 evdev );
    Int32 FindHeldKey( UInt32 evdev ) const;
    UInt32 GetScale120( void ) const;

    private:

    static const UInt32 MaxHeldKeys = 32;
    static const UInt32 MaxButtons = 32;

    PLUGINGLUE::WAYLAND::CWaylandConnection& m_connection;
    UInt32 m_surfaceId;
    TInputCallbacks m_callbacks;
    bool m_releaseConnection;
    bool m_pulsing;
    TClock m_clock;

    bool m_hasPointer;
    bool m_hasKeyboard;
    bool m_pointerInside;
    bool m_hasPosition;
    Int32 m_mouseX;
    Int32 m_mouseY;
    UInt32 m_buttonsDown;
    Int32 m_wheel;
    Int32 m_horizontalWheel;
    bool m_framesExpected;
    CAxisAccumulator m_axis;

    bool m_keyboardFocused;
    SXkbApi m_xkbApi;
    bool m_hasXkbApi;
    CXkbKeyboard* m_xkb;
    CKeyRepeat m_repeat;
    UInt32 m_modifierMask;
    SHeldKey m_held[ 32 ];
    UInt32 m_heldCount;
    UInt8 m_keyStates[ KEYCODE_MAX ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H ? */
