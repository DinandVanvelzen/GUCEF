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

#include "guidriverWAYLAND_CWaylandFramePacer.h"

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

CWaylandFramePacer::CWaylandFramePacer( void )
    : m_vsync( true )
    , m_periodInMs( 16 )
    , m_timeoutInMs( 1000 )
    , m_configured( false )
    , m_forced( false )
    , m_callbackPending( false )
    , m_callbackRequestedAt( 0 )
    , m_lastDrawn( 0 )
    , m_hasDrawn( false )
    , m_framesDrawn( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::SetVerticalSync( bool vsync )
{GUCEF_TRACE;

    m_vsync = vsync;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandFramePacer::GetVerticalSync( void ) const
{GUCEF_TRACE;

    return m_vsync;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::SetFrequency( UInt32 framesPerSecond )
{GUCEF_TRACE;

    m_periodInMs = framesPerSecond > 0 ? 1000 / framesPerSecond : 16;
    if ( 0 == m_periodInMs )
        m_periodInMs = 1;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::SetCallbackTimeout( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_timeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::SetConfigured( bool configured )
{GUCEF_TRACE;

    m_configured = configured;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::RequestRedraw( void )
{GUCEF_TRACE;

    m_forced = true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::OnFrameCallbackRequested( UInt32 now )
{GUCEF_TRACE;

    m_callbackPending = true;
    m_callbackRequestedAt = now;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::OnFrameCallbackDone( void )
{GUCEF_TRACE;

    m_callbackPending = false;
}

/*-------------------------------------------------------------------------*/

void
CWaylandFramePacer::OnFrameDrawn( UInt32 now )
{GUCEF_TRACE;

    m_forced = false;
    // a frame drawn on the timeout restarts the wait for the callback that is still out
    if ( m_callbackPending )
        m_callbackRequestedAt = now;
    m_hasDrawn = true;
    m_lastDrawn = now;
    ++m_framesDrawn;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandFramePacer::ShouldDraw( UInt32 now ) const
{GUCEF_TRACE;

    if ( !m_configured )
        return false;
    if ( m_forced || !m_hasDrawn )
        return true;

    if ( m_vsync )
    {
        if ( !m_callbackPending )
            return true;
        // the unsigned difference is right also when the tick count wrapped
        return (UInt32) ( now - m_callbackRequestedAt ) >= m_timeoutInMs;
    }
    return (UInt32) ( now - m_lastDrawn ) >= m_periodInMs;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandFramePacer::IsCallbackPending( void ) const
{GUCEF_TRACE;

    return m_callbackPending;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandFramePacer::WantsFrameCallback( void ) const
{GUCEF_TRACE;

    return m_vsync && !m_callbackPending;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandFramePacer::GetFramesDrawn( void ) const
{GUCEF_TRACE;

    return m_framesDrawn;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
