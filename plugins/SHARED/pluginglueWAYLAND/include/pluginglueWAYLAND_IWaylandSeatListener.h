/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H
#define GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Takes the events of the pointer and the keyboard of the seat as the connection receives them, on the thread that
 *  pumps the connection. Every call has an empty body so a listener overrides what it uses. A listener that is added
 *  while the seat already has capabilities, a keymap, a repeat rate or a focus is told of those at once.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP IWaylandSeatListener
{
    public:

    virtual ~IWaylandSeatListener() {}

    /** The pointer and keyboard the seat has now, WL_SEAT_CAPABILITY_POINTER and WL_SEAT_CAPABILITY_KEYBOARD bits; they come and go (hot plug) */
    virtual void OnSeatCapabilities( UInt32 capabilities ) {}

    virtual void OnPointerEnter( UInt32 serial, struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y ) {}

    virtual void OnPointerLeave( UInt32 serial, struct wl_surface* surface ) {}

    virtual void OnPointerMotion( UInt32 timeInMs, wl_fixed_t x, wl_fixed_t y ) {}

    virtual void OnPointerButton( UInt32 serial, UInt32 timeInMs, UInt32 button, UInt32 state ) {}

    virtual void OnPointerAxis( UInt32 timeInMs, UInt32 axis, wl_fixed_t value ) {}

    virtual void OnPointerAxisSource( UInt32 axisSource ) {}

    virtual void OnPointerAxisStop( UInt32 timeInMs, UInt32 axis ) {}

    virtual void OnPointerAxisDiscrete( UInt32 axis, Int32 discrete ) {}

    virtual void OnPointerAxisValue120( UInt32 axis, Int32 value120 ) {}

    /** Whether the compositor inverted the axis values of the group relative to the movement of the device (natural scrolling): GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL or _INVERTED (wl_pointer version 9) */
    virtual void OnPointerAxisRelativeDirection( UInt32 axis, UInt32 direction ) {}

    /** Ends a group of axis events that belong together (wl_pointer version 5 and newer); older compositors send none */
    virtual void OnPointerFrame( void ) {}

    /** The keymap as text (the data is only valid during the call; 'size' includes any terminating zero) */
    virtual void OnKeyboardKeymap( UInt32 format, const char* data, UInt32 size ) {}

    virtual void OnKeyboardEnter( UInt32 serial, struct wl_surface* surface, const UInt32* pressedKeys, UInt32 pressedKeyCount ) {}

    virtual void OnKeyboardLeave( UInt32 serial, struct wl_surface* surface ) {}

    /** 'key' is the evdev code of the key, 'state' WL_KEYBOARD_KEY_STATE_PRESSED or released */
    virtual void OnKeyboardKey( UInt32 serial, UInt32 timeInMs, UInt32 key, UInt32 state ) {}

    virtual void OnKeyboardModifiers( UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group ) {}

    /** Characters a second the compositor wants repeated for a held key (0 for none) and the delay before the first repeat, both as the compositor says; Wayland repeats nothing itself */
    virtual void OnKeyboardRepeatInfo( Int32 rate, Int32 delayInMs ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_WAYLAND_IWAYLANDSEATLISTENER_H ? */
