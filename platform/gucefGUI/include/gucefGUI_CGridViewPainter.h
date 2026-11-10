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

#ifndef GUCEF_GUI_CGRIDVIEWPAINTER_H
#define GUCEF_GUI_CGRIDVIEWPAINTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

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

class CGridView;
class CGUIContext;

/**
 *  How a grid looks, as drawing commands: the header, the visible rows only, the row header, the footer, the
 *  selection and the cursor, the lines, the frozen columns and rows, the scroll indicators. Used by CWidgetPainter
 *  for the widgets of type GridView. A grid that uses none of the optional features is drawn exactly as it always was.
 *
 *  The cost of a frame is in proportion to the cells in view: the painter reads the rows in view from the grid and
 *  nothing else, whatever the number of rows.
 */
class GUCEF_GUI_PUBLIC_CPP CGridViewPainter
{
    public:

    typedef CWidgetPainter::TCommandVector TCommandVector;

    /** Adds the commands for the grid, drawn in 'area' (the grid at the origin) */
    static void Paint( const CGridView& grid, CGUIContext& context, const CRect& area, TCommandVector& commands );

    private:

    CGridViewPainter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDVIEWPAINTER_H ? */
