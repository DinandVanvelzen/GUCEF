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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#include "guidriverWAYLAND_CXdgConfigureState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CXdgConfigureState::CXdgConfigureState( void )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CXdgConfigureState::Reset( void )
{GUCEF_TRACE;

    m_requestedWidth = 0;
    m_requestedHeight = 0;
    m_floatingWidth = 0;
    m_floatingHeight = 0;
    m_width = 0;
    m_height = 0;
    m_boundsWidth = 0;
    m_boundsHeight = 0;
    m_states = 0;
    m_pendingWidth = 0;
    m_pendingHeight = 0;
    m_pendingStates = 0;
    m_hasPending = false;
    m_configured = false;
    m_canFullscreen = true;
    m_canMaximize = true;
}

/*-------------------------------------------------------------------------*/

void
CXdgConfigureState::SetRequestedSize( Int32 width, Int32 height )
{GUCEF_TRACE;

    m_requestedWidth = width;
    m_requestedHeight = height;
    if ( !m_configured )
    {
        m_width = width;
        m_height = height;
    }
}

/*-------------------------------------------------------------------------*/

bool
CXdgConfigureState::ApplyClientSize( Int32 width, Int32 height )
{GUCEF_TRACE;

    if ( width <= 0 || height <= 0 )
        return false;
    m_requestedWidth = width;
    m_requestedHeight = height;
    if ( !IsFloatingStates( m_states ) )
        return false;       // the compositor decides the size of a window it maximised, tiled or made full screen
    m_floatingWidth = width;
    m_floatingHeight = height;
    const bool changed = width != m_width || height != m_height;
    m_width = width;
    m_height = height;
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CXdgConfigureState::IsFloatingStates( UInt32 states )
{GUCEF_TRACE;

    return 0 == ( states & ( TOPLEVELSTATE_MAXIMIZED | TOPLEVELSTATE_FULLSCREEN | TOPLEVELSTATE_TILED_LEFT | TOPLEVELSTATE_TILED_RIGHT | TOPLEVELSTATE_TILED_TOP | TOPLEVELSTATE_TILED_BOTTOM ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CXdgConfigureState::StatesFromProtocol( const UInt32* states, UInt32 count )
{GUCEF_TRACE;

    UInt32 bits = 0;
    for ( UInt32 i=0; GUCEF_NULL != states && i<count; ++i )
    {
        switch ( states[ i ] )
        {
            case GUCEF_WL_XDG_TOPLEVEL_STATE_MAXIMIZED :    bits |= TOPLEVELSTATE_MAXIMIZED; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_FULLSCREEN :   bits |= TOPLEVELSTATE_FULLSCREEN; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_RESIZING :     bits |= TOPLEVELSTATE_RESIZING; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_ACTIVATED :    bits |= TOPLEVELSTATE_ACTIVATED; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_LEFT :   bits |= TOPLEVELSTATE_TILED_LEFT; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_RIGHT :  bits |= TOPLEVELSTATE_TILED_RIGHT; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_TOP :    bits |= TOPLEVELSTATE_TILED_TOP; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_TILED_BOTTOM : bits |= TOPLEVELSTATE_TILED_BOTTOM; break;
            case GUCEF_WL_XDG_TOPLEVEL_STATE_SUSPENDED :    bits |= TOPLEVELSTATE_SUSPENDED; break;
            default : break;
        }
    }
    return bits;
}

/*-------------------------------------------------------------------------*/

UInt32
CXdgConfigureState::StatesFromLibdecor( int libdecorState )
{GUCEF_TRACE;

    UInt32 bits = 0;
    if ( 0 != ( libdecorState & ( 1 << 0 ) ) ) bits |= TOPLEVELSTATE_ACTIVATED;
    if ( 0 != ( libdecorState & ( 1 << 1 ) ) ) bits |= TOPLEVELSTATE_MAXIMIZED;
    if ( 0 != ( libdecorState & ( 1 << 2 ) ) ) bits |= TOPLEVELSTATE_FULLSCREEN;
    if ( 0 != ( libdecorState & ( 1 << 3 ) ) ) bits |= TOPLEVELSTATE_TILED_LEFT;
    if ( 0 != ( libdecorState & ( 1 << 4 ) ) ) bits |= TOPLEVELSTATE_TILED_RIGHT;
    if ( 0 != ( libdecorState & ( 1 << 5 ) ) ) bits |= TOPLEVELSTATE_TILED_TOP;
    if ( 0 != ( libdecorState & ( 1 << 6 ) ) ) bits |= TOPLEVELSTATE_TILED_BOTTOM;
    if ( 0 != ( libdecorState & ( 1 << 7 ) ) ) bits |= TOPLEVELSTATE_SUSPENDED;
    return bits;
}

/*-------------------------------------------------------------------------*/

void
CXdgConfigureState::OnToplevelConfigure( Int32 width, Int32 height, UInt32 stateBits )
{GUCEF_TRACE;

    m_pendingWidth = width;
    m_pendingHeight = height;
    m_pendingStates = stateBits;
    m_hasPending = true;
}

/*-------------------------------------------------------------------------*/

void
CXdgConfigureState::OnToplevelBounds( Int32 width, Int32 height )
{GUCEF_TRACE;

    m_boundsWidth = width;
    m_boundsHeight = height;
}

/*-------------------------------------------------------------------------*/

void
CXdgConfigureState::OnCapabilities( const UInt32* capabilities, UInt32 count )
{GUCEF_TRACE;

    m_canFullscreen = false;
    m_canMaximize = false;
    for ( UInt32 i=0; GUCEF_NULL != capabilities && i<count; ++i )
    {
        if ( GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN == capabilities[ i ] ) m_canFullscreen = true;
        if ( GUCEF_WL_XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE == capabilities[ i ] ) m_canMaximize = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CXdgConfigureState::OnSurfaceConfigure( UInt32 serial, SConfigureResult& result )
{GUCEF_TRACE;

    const bool hadPending = m_hasPending;
    const bool first = !m_configured;
    const UInt32 oldStates = m_states;
    const Int32 oldWidth = m_width;
    const Int32 oldHeight = m_height;

    Int32 width = m_width;
    Int32 height = m_height;
    UInt32 states = m_states;
    if ( hadPending )
    {
        states = m_pendingStates;
        if ( m_pendingWidth > 0 && m_pendingHeight > 0 )
        {
            width = m_pendingWidth;
            height = m_pendingHeight;
        }
        else
        {
            // left to the application: a floating window goes back to the size it had, anything else keeps what it has
            if ( IsFloatingStates( states ) && m_floatingWidth > 0 && m_floatingHeight > 0 )
            {
                width = m_floatingWidth;
                height = m_floatingHeight;
            }
            else
            if ( width <= 0 || height <= 0 )
            {
                width = m_requestedWidth;
                height = m_requestedHeight;
            }
        }
    }
    else
    if ( width <= 0 || height <= 0 )
    {
        width = m_requestedWidth;
        height = m_requestedHeight;
    }
    m_hasPending = false;
    m_pendingWidth = 0;
    m_pendingHeight = 0;

    // the size a floating window has is the one to go back to
    if ( IsFloatingStates( states ) && width > 0 && height > 0 )
    {
        m_floatingWidth = width;
        m_floatingHeight = height;
    }

    m_width = width;
    m_height = height;
    m_states = states;
    m_configured = true;

    result.serial = serial;
    result.width = width;
    result.height = height;
    result.sizeChanged = first || width != oldWidth || height != oldHeight;
    result.states = states;
    result.changedStates = states ^ oldStates;
    result.firstConfigure = first;
    return hadPending;
}

/*-------------------------------------------------------------------------*/

bool CXdgConfigureState::IsConfigured( void ) const { return m_configured; }
Int32 CXdgConfigureState::GetWidth( void ) const { return m_width; }
Int32 CXdgConfigureState::GetHeight( void ) const { return m_height; }
UInt32 CXdgConfigureState::GetStates( void ) const { return m_states; }
bool CXdgConfigureState::IsFloating( void ) const { return IsFloatingStates( m_states ); }
bool CXdgConfigureState::IsActivated( void ) const { return 0 != ( m_states & TOPLEVELSTATE_ACTIVATED ); }
Int32 CXdgConfigureState::GetBoundsWidth( void ) const { return m_boundsWidth; }
Int32 CXdgConfigureState::GetBoundsHeight( void ) const { return m_boundsHeight; }
bool CXdgConfigureState::CanFullscreen( void ) const { return m_canFullscreen; }
bool CXdgConfigureState::CanMaximize( void ) const { return m_canMaximize; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
