/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUIDRIVEROGRE_COGRESURFACESTATE_H
#define GUIDRIVEROGRE_COGRESURFACESTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVEROGRE_MACROS_H
#include "guidriverOgre_macros.h"
#define GUIDRIVEROGRE_MACROS_H
#endif /* GUIDRIVEROGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Whether an Ogre render window has a native surface to draw on, and what is to be done when the platform window
 *  loses its native surface and gets one again (an Android app that goes to the background and returns).
 *
 *  The render window, the Root and the resources stay when the surface goes: Ogre is told that the surface is gone
 *  (RenderWindow::_notifySurfaceDestroyed()) while the native window still exists, no frames are drawn, and Ogre gets the
 *  new native window (RenderWindow::_notifySurfaceCreated()) when it comes. The EGL context is kept over the gap
 *  (preserveContext), so nothing has to be loaded again. A loss without a render window, a second loss and a restore
 *  without a loss before are ignored.
 */
class GUIDRIVEROGRE_PUBLIC_CPP COgreSurfaceState
{
    public:

    enum EState
    {
        SURFACESTATE_NO_WINDOW = 0,     /**< no render window */
        SURFACESTATE_READY,             /**< the render window has its surface: frames are drawn */
        SURFACESTATE_LOST               /**< the native surface is gone: nothing is drawn until it comes back */
    };

    enum EAction
    {
        SURFACEACTION_NONE = 0,
        SURFACEACTION_RELEASE_SURFACE,  /**< tell Ogre the surface goes (RenderWindow::_notifySurfaceDestroyed()) */
        SURFACEACTION_ADOPT_SURFACE     /**< give Ogre the new native window (RenderWindow::_notifySurfaceCreated()) */
    };

    COgreSurfaceState( void );

    void OnRenderWindowCreated( void );

    void OnRenderWindowDestroyed( void );

    /** The platform window loses its native surface */
    EAction OnSurfaceLost( void );

    /** The platform window has a native surface again; 'hasNativeWindow' says whether its native window property names one */
    EAction OnSurfaceRestored( bool hasNativeWindow );

    /** Whether a frame can be drawn now */
    bool CanDraw( void ) const;

    EState GetState( void ) const;

    /** How often the surface was lost and how often a new one was adopted */
    UInt32 GetLossCount( void ) const;

    UInt32 GetRestoreCount( void ) const;

    static const char* ToString( EState state );

    private:

    EState m_state;
    UInt32 m_lossCount;
    UInt32 m_restoreCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_COGRESURFACESTATE_H ? */
