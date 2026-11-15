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

#ifndef GUCEF_GUI_CTOOLTIPLAYOUT_H
#define GUCEF_GUI_CTOOLTIPLAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
class CTooltipService;

/**
 *  Where a tooltip goes and how its text is broken into lines: the box in logical pixels of the context, the lines and
 *  the height of a line. Compute() measures with the fonts of the context (CGUIContext::MeasureText()), so the layout is
 *  that of the driver's fonts. Wrap() and Place() are the two steps on their own, for the tests and for other overlays.
 */
class GUCEF_GUI_PUBLIC_CPP CTooltipLayout
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    CRect box;              /**< the whole tooltip, border included */
    TStringVector lines;
    Int32 lineHeight;
    Int32 paddingX;         /**< from the edge of the box to the text */
    Int32 paddingY;

    CTooltipLayout( void );

    /**
     *  The layout of 'text' in 'font' for the pointer at (pointerX, pointerY): wrapped to the maximum width of the service (and
     *  the width of the picture), placed by Place() inside the logical picture of the context. False for an empty text
     */
    bool Compute( CGUIContext& context, const CString& font, const CString& text, Int32 pointerX, Int32 pointerY, const CTooltipService& settings );

    /**
     *  Breaks the text into lines no wider than 'maxTextWidth': at every '\n', and between words where the next word would not
     *  fit; a word wider than the room on its own is split between code points. The lines are measured with the context; when
     *  the font cannot be measured the paragraphs are kept whole. Blanks at the break are dropped
     */
    static void Wrap( CGUIContext& context, const CString& font, const CString& text, Int32 maxTextWidth, TStringVector& lines );

    /**
     *  The box of the given size for the pointer: at (pointerX + offsetX, pointerY + offsetY); moved left to end at the right edge
     *  of the area when it passes it (not before 0); above the pointer (pointerY - height - 4) when it passes the bottom, and
     *  when that is above the top, at areaHeight - height, at least 0
     */
    static CRect Place( Int32 pointerX, Int32 pointerY, Int32 width, Int32 height, Int32 areaWidth, Int32 areaHeight, Int32 offsetX, Int32 offsetY );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTOOLTIPLAYOUT_H ? */
