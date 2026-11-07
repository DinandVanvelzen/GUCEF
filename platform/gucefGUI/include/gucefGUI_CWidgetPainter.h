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

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#define GUCEF_GUI_CWIDGETPAINTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  How a widget looks, as 2D drawing commands (the same commands a CDrawSurface holds), in coordinates
 *  relative to the widget. Drivers that draw widgets themselves (RmlUi, the software renderer, LVGL ...)
 *  turn these commands into their own drawing, so a widget looks the same on every backend: the same
 *  pictures for the same states, text broken and aligned by the same rules.
 *
 *  Text: one COMMAND_TEXT per line, already placed. Pictures: COMMAND_IMAGE with the image name.
 */
class GUCEF_GUI_PUBLIC_CPP CWidgetPainter
{
    public:

    typedef CDrawSurface::TCommandVector TCommandVector;

    /** Adds the commands for the widget itself (not its children) */
    static void Paint( const CWidget& widget, CGUIContext& context, TCommandVector& commands );

    /**
     *  What is drawn over everything: the tooltip of the widget under the pointer and the cursor picture,
     *  in logical coordinates of the context
     */
    static void PaintOverlay( CGUIContext& context, TCommandVector& commands );

    /** Adds the commands for one block of text: broken into lines and aligned in 'area' */
    static void PaintText( CGUIContext& context                ,
                           const CString& font                 ,
                           const CString& text                 ,
                           const CRect& area                   ,
                           const CString& hAlign               ,
                           const CString& vAlign               ,
                           bool wordWrap                       ,
                           const CColour& colour               ,
                           TCommandVector& commands            );

    /**
     *  The text as it fits in 'maxWidth' pixels of the font: the whole text when it fits, else the longest start of it
     *  followed by "..." that fits ('cut' true). A room too small even for "..." gives an empty text (also cut)
     */
    static CString FitText( CGUIContext& context, const CString& font, const CString& text, Int32 maxWidth, bool& cut );

    /** The left of a text of 'textWidth' pixels in a cell from 'left' with 'width' pixels for an alignment (Left, Center, Right) and padding */
    static Int32 AlignText( Int32 left, Int32 width, Int32 textWidth, const CString& hAlign, Int32 padLeft, Int32 padRight );

    /** Where an image of the given size goes in 'area' for a ScaleMode */
    static CRect FitImage( Int32 imageWidth, Int32 imageHeight, const CRect& area, const CString& scaleMode );

    private:

    static void PaintBox( const CWidget& widget, const CRect& area, TCommandVector& commands );
    static void PaintButton( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands );
    static void PaintEditbox( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands );
    static void PaintRows( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands );
    static void PaintImage( const CWidget& widget, CGUIContext& context, const CRect& area, TCommandVector& commands );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */
