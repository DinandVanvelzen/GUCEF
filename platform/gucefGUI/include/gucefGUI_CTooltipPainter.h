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

#ifndef GUCEF_GUI_CTOOLTIPPAINTER_H
#define GUCEF_GUI_CTOOLTIPPAINTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CTOOLTIPLAYOUT_H
#include "gucefGUI_CTooltipLayout.h"
#define GUCEF_GUI_CTOOLTIPLAYOUT_H
#endif /* GUCEF_GUI_CTOOLTIPLAYOUT_H ? */

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

class CTooltipService;

/**
 *  The draw commands of a tooltip, as CGridViewPainter makes those of a grid: the box filled, its border, and one text
 *  command per line, in the colours of the tooltip service. The commands are in logical pixels of the context; the overlay
 *  pass of CWidgetPainter adds them after everything else.
 */
class GUCEF_GUI_PUBLIC_CPP CTooltipPainter
{
    public:

    typedef CDrawSurface::TCommandVector TCommandVector;

    static void Paint( const CTooltipLayout& layout, const CString& font, const CTooltipService& settings, TCommandVector& commands );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTOOLTIPPAINTER_H ? */
