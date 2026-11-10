/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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


#ifndef GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H
#define GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

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
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The states of a toplevel window as bits: what xdg_toplevel.configure and libdecor say, in one form */
enum EToplevelState
{
    TOPLEVELSTATE_MAXIMIZED    = 1 << 0 ,
    TOPLEVELSTATE_FULLSCREEN   = 1 << 1 ,
    TOPLEVELSTATE_RESIZING     = 1 << 2 ,
    TOPLEVELSTATE_ACTIVATED    = 1 << 3 ,
    TOPLEVELSTATE_TILED_LEFT   = 1 << 4 ,
    TOPLEVELSTATE_TILED_RIGHT  = 1 << 5 ,
    TOPLEVELSTATE_TILED_TOP    = 1 << 6 ,
    TOPLEVELSTATE_TILED_BOTTOM = 1 << 7 ,
    TOPLEVELSTATE_SUSPENDED    = 1 << 8
};

/** What the application has to do after a configure: the size to draw at (surface units) and the states */
struct SConfigureResult
{
    UInt32 serial;              /**< to acknowledge */
    Int32 width;
    Int32 height;
    bool sizeChanged;           /**< the size is not the one before (always for the first configure) */
    UInt32 states;
    UInt32 changedStates;       /**< bits that differ from the states before */
    bool firstConfigure;

    SConfigureResult( void ) : serial( 0 ), width( 0 ), height( 0 ), sizeChanged( false ), states( 0 ), changedStates( 0 ), firstConfigure( false ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The negotiation of the size and state of a toplevel window with the compositor. The compositor sends the wanted size and
 *  the states (xdg_toplevel.configure), possibly more than once, and ends with xdg_surface.configure: that is the moment
 *  to apply what came last and to acknowledge it. A size of 0 x 0 leaves it to the application: the size the window had
 *  as a floating window, else the size that was asked for. A size is in surface units.
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CXdgConfigureState
{
    public:

    CXdgConfigureState( void );

    void Reset( void );

    /** The size the application wants, used while the compositor leaves it open and no floating size is known */
    void SetRequestedSize( Int32 width, Int32 height );

    /** The application resized its floating window itself (by drawing a buffer of another size): true when that changes the size */
    bool ApplyClientSize( Int32 width, Int32 height );

    void OnToplevelConfigure( Int32 width, Int32 height, UInt32 stateBits );

    void OnToplevelBounds( Int32 width, Int32 height );

    void OnCapabilities( const UInt32* capabilities, UInt32 count );

    /** The end of a series of configure events: what to apply. False when no toplevel configure came with it (nothing to apply, but the serial is still to acknowledge) */
    bool OnSurfaceConfigure( UInt32 serial, SConfigureResult& result );

    bool IsConfigured( void ) const;

    Int32 GetWidth( void ) const;

    Int32 GetHeight( void ) const;

    UInt32 GetStates( void ) const;

    /** Not maximised, full screen or tiled: the application sets the size */
    bool IsFloating( void ) const;

    bool IsActivated( void ) const;

    Int32 GetBoundsWidth( void ) const;

    Int32 GetBoundsHeight( void ) const;

    /** Capabilities of the window manager (xdg_toplevel.wm_capabilities); everything is assumed possible until it says otherwise */
    bool CanFullscreen( void ) const;

    bool CanMaximize( void ) const;

    /** The xdg_toplevel state numbers as bits */
    static UInt32 StatesFromProtocol( const UInt32* states, UInt32 count );

    /** The libdecor window state bits as bits of this */
    static UInt32 StatesFromLibdecor( int libdecorState );

    private:

    static bool IsFloatingStates( UInt32 states );

    private:

    Int32 m_requestedWidth;
    Int32 m_requestedHeight;
    Int32 m_floatingWidth;
    Int32 m_floatingHeight;
    Int32 m_width;
    Int32 m_height;
    Int32 m_boundsWidth;
    Int32 m_boundsHeight;
    UInt32 m_states;
    Int32 m_pendingWidth;
    Int32 m_pendingHeight;
    UInt32 m_pendingStates;
    bool m_hasPending;
    bool m_configured;
    bool m_canFullscreen;
    bool m_canMaximize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CXDGCONFIGURESTATE_H ? */
