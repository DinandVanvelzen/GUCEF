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

#ifndef GUCEF_GUI_CPANEL_H
#define GUCEF_GUI_CPANEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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

/**
 *  A rectangle that holds other widgets, with an optional background colour, picture and border.
 *  Registered as "Panel" and "Form" (the usual root of a layout), and as the containers "HBox",
 *  "VBox", "Grid" and "Margin" whose children the context places (Padding, Spacing, Columns).
 */
class GUCEF_GUI_PUBLIC_CPP CPanel : public CWidget
{
    public:

    CPanel( void );

    CPanel( const CString& widgetTypeName );

    virtual ~CPanel();

    /** The size of the background picture; for the HBox and VBox containers the size of their children */
    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    protected:

    /** The natural size of the children side by side or below each other, with the padding and spacing */
    bool GetStackNaturalSize( bool horizontal, Int32& width, Int32& height ) const;

    private:

    CPanel( const CPanel& src );
    CPanel& operator=( const CPanel& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPANEL_H ? */
