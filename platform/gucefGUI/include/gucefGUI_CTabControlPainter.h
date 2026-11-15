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

#ifndef GUCEF_GUI_CTABCONTROLPAINTER_H
#define GUCEF_GUI_CTABCONTROLPAINTER_H

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

class CTabControl;
class CGUIContext;

/**
 *  How a tab control looks, as drawing commands: the background, a header per tab that is shown (fill, frame,
 *  the title centred in the room left of its badge), the badges (a dot or a pill in the accent colour), and the
 *  overflow button of TabSizing Content. A tab control with TabSizing Equal and no badges is drawn with the commands
 *  it always had. Used by CWidgetPainter for the widgets of type TabControl.
 */
class GUCEF_GUI_PUBLIC_CPP CTabControlPainter
{
    public:

    typedef CWidgetPainter::TCommandVector TCommandVector;

    /** The colour of the badges and the colour of the text of a pill */
    static const CColour BadgeColour;
    static const CColour BadgeTextColour;

    /** Adds the commands for the tab control, drawn in 'area' (the control at the origin) */
    static void Paint( const CTabControl& tabs, CGUIContext& context, const CRect& area, TCommandVector& commands );

    private:

    CTabControlPainter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABCONTROLPAINTER_H ? */
