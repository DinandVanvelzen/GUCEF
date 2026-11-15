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

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVEROGRE_MACROS_H
#include "guidriverOgre_macros.h"
#define GUIDRIVEROGRE_MACROS_H
#endif /* GUIDRIVEROGRE_MACROS_H ? */

namespace Ogre { class RenderSystem; class SceneManager; class Viewport; }

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
 *  Something that draws over the viewports of an Ogre window: a GUI context of a GUI driver on Ogre.
 *
 *  An overlay registers with its window (COgreWindowContext::RegisterOverlay()). In every frame of the window the
 *  overlays draw after the viewports of the application, from the lowest order up (equal orders in the order they
 *  were registered), before the picture is presented. The window context also renders one overlay alone into a
 *  render texture for captures (COgreWindowContext::CaptureOverlay()).
 *
 *  Drawing happens on the render thread, with the render system's viewport set to the viewport given and inside
 *  RenderSystem::_beginFrame() / _endFrame(). The overlay sets every render state it depends on.
 */
class GUIDRIVEROGRE_PUBLIC_CPP CIOgreWindowOverlay
{
    public:

    CIOgreWindowOverlay( void );

    virtual ~CIOgreWindowOverlay();

    /** Where the overlay is drawn among the overlays of its window: lower first. GUI contexts give their layer */
    virtual Int32 GetOverlayOrder( void ) const = 0;

    /**
     *  Draws the overlay.
     *
     *  @param renderSystem the render system of the Root, with 'viewport' set
     *  @param viewport the whole render target that is drawn: the window, or the render texture of a capture
     *  @param sceneManager the scene manager of the camera of the last viewport of the window, NULL when the window
     *         has no viewport with a camera or when drawing a capture
     *  @param isCapture true when the overlay is drawn alone into a render texture of the size the capture asked
     *         for, at the origin and without the scaling of the window
     */
    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem ,
                                Ogre::Viewport& viewport         ,
                                Ogre::SceneManager* sceneManager ,
                                bool isCapture                   ) = 0;

    private:

    CIOgreWindowOverlay( const CIOgreWindowOverlay& src );              /**< not implemented */
    CIOgreWindowOverlay& operator=( const CIOgreWindowOverlay& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */
