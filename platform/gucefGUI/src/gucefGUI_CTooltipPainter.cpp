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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CTOOLTIPSERVICE_H
#include "gucefGUI_CTooltipService.h"
#define GUCEF_GUI_CTOOLTIPSERVICE_H
#endif /* GUCEF_GUI_CTOOLTIPSERVICE_H ? */

#include "gucefGUI_CTooltipPainter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTooltipPainter::Paint( const CTooltipLayout& layout, const CString& font, const CTooltipService& settings, TCommandVector& commands )
{GUCEF_TRACE;

    if ( layout.box.IsEmpty() || layout.lines.empty() )
        return;

    CDrawSurface::TCommand fill;
    fill.type = CDrawSurface::COMMAND_FILLRECT;
    fill.rect = layout.box;
    fill.colour = settings.GetBackgroundColour();
    commands.push_back( fill );

    CDrawSurface::TCommand border;
    border.type = CDrawSurface::COMMAND_STROKERECT;
    border.rect = layout.box;
    border.colour = settings.GetBorderColour();
    border.thickness = 1;
    commands.push_back( border );

    for ( UInt32 i=0; i<(UInt32)layout.lines.size(); ++i )
    {
        if ( layout.lines[ i ].IsNULLOrEmpty() )
            continue;
        CDrawSurface::TCommand text;
        text.type = CDrawSurface::COMMAND_TEXT;
        text.font = font;
        text.text = layout.lines[ i ];
        text.x0 = layout.box.left + layout.paddingX;
        text.y0 = layout.box.top + layout.paddingY + (Int32) i * layout.lineHeight;
        text.colour = settings.GetTextColour();
        commands.push_back( text );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
