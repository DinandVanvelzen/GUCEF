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

#ifndef GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H
#define GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H

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

class CWidget;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where an application keeps the tooltips of its widgets in one place, apart from the code that makes them: a table of texts by the
 *  name of the widget, which people can review as a whole. CTooltipService asks it for the text of a widget that has no tooltip of its
 *  own, so also the widgets that forms make later (a dialog that opens for the first time) have their text without any call. The
 *  tooltip that a widget has itself (its Tooltip property) always goes first.
 */
class GUCEF_GUI_PUBLIC_CPP CITooltipTextProvider
{
    public:

    CITooltipTextProvider( void );

    virtual ~CITooltipTextProvider();

    /** The text for the point of the widget (relative to its top left), empty for none */
    virtual CString GetTooltipFor( const CWidget& widget, Int32 localX, Int32 localY ) const = 0;

    private:

    CITooltipTextProvider( const CITooltipTextProvider& src );              /**< not implemented */
    CITooltipTextProvider& operator=( const CITooltipTextProvider& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H ? */
