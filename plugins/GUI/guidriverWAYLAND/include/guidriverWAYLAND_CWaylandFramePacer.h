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


#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decides when a window draws its next frame. With vertical sync the compositor paces the window: a frame is drawn when
 *  the compositor said (wl_surface.frame) that it showed the one before, so the window draws once per refresh of the
 *  monitor and not at all while the compositor does not show it (minimised, on another workspace). Should a frame callback
 *  never come, a frame is drawn after a timeout so a window does not freeze for good. Without vertical sync a frame is
 *  drawn each period of the refresh rate of the video settings. Times are tick counts in milliseconds, which may wrap.
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CWaylandFramePacer
{
    public:

    CWaylandFramePacer( void );

    void SetVerticalSync( bool vsync );

    bool GetVerticalSync( void ) const;

    void SetFrequency( UInt32 framesPerSecond );

    void SetCallbackTimeout( UInt32 timeoutInMs );

    /** The window can be drawn in: the compositor configured it */
    void SetConfigured( bool configured );

    /** Something changed (size, scale) that must be shown whatever the pace */
    void RequestRedraw( void );

    /** A frame callback was requested with the frame drawn at 'now' */
    void OnFrameCallbackRequested( UInt32 now );

    /** The compositor showed the frame (the done event of the callback) */
    void OnFrameCallbackDone( void );

    /** A frame was drawn, with or without a callback requested */
    void OnFrameDrawn( UInt32 now );

    bool ShouldDraw( UInt32 now ) const;

    /** A frame callback is out and has not come back */
    bool IsCallbackPending( void ) const;

    /** Whether the next frame has to ask for a frame callback: with vertical sync, and none is out */
    bool WantsFrameCallback( void ) const;

    UInt32 GetFramesDrawn( void ) const;

    private:

    bool m_vsync;
    UInt32 m_periodInMs;
    UInt32 m_timeoutInMs;
    bool m_configured;
    bool m_forced;
    bool m_callbackPending;
    UInt32 m_callbackRequestedAt;
    UInt32 m_lastDrawn;
    bool m_hasDrawn;
    UInt32 m_framesDrawn;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDFRAMEPACER_H ? */
