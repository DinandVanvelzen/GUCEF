/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CSOFTWARERENDERER_H
#define GUCEF_GUI_CSOFTWARERENDERER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUIContext;

/**
 *  Draws a GUI context on the CPU into an RGBA picture: every shown root with its widgets (as the widget
 *  painter describes them), then the overlay (tooltip, cursor). It is the reference for what the drawing
 *  drivers show, and gives drivers without graphics (the null driver) a frame capture.
 *
 *  Rules every driver follows the same way: a widget's own drawing is cut off at its rectangle, its
 *  children too when Clip is set; pictures are sampled at the nearest pixel; colours are blended by their
 *  alpha, multiplied by the Opacity of the widget and its ancestors.
 */
class GUCEF_GUI_PUBLIC_CPP CSoftwareRenderer
{
    public:

    typedef CDrawSurface::TCommandVector TCommandVector;

    /** The whole context on black, at its render size (the logical resolution times the render scale); 'target' is made or resized as needed */
    static bool RenderContext( CGUIContext& context, IMAGE::TPixelMapPtr& target );

    /** Executes drawing commands moved by an offset, cut off at 'clip'; all in logical pixels, drawn at the render scale of the context */
    static void Execute( CGUIContext& context          ,
                         const TCommandVector& commands ,
                         Int32 offsetX                  ,
                         Int32 offsetY                  ,
                         const CRect& clip              ,
                         Float32 opacity                ,
                         IMAGE::CPixelMap& target       );

    private:

    static void RenderWidget( CGUIContext& context, const CWidget& widget, const CRect& clip, Float32 opacity, IMAGE::CPixelMap& target );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSOFTWARERENDERER_H ? */
