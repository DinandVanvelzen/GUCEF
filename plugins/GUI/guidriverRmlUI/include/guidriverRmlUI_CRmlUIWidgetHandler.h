/*
 *  guidriverRmlUI: GUI backend using RmlUi
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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGETHANDLER_H
#include "gucefGUI_CWidgetHandler.h"
#define GUCEF_GUI_CWIDGETHANDLER_H
#endif /* GUCEF_GUI_CWIDGETHANDLER_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Gives every gucefGUI widget an RmlUi element (CRmlUIWidgetElement) and keeps the element's place in
 *  the tree, its rectangle and its visibility in step with the widget. The element draws the widget
 *  from its properties and state, so those need no work here.
 *
 *  Backend hints: a property "rcss:<name>" sets the RCSS property <name> on the element (decorators,
 *  filters, transforms ...), "rml:class" sets its classes.
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIWidgetHandler : public GUI::CWidgetHandler
{
    public:

    static const char* RcssHintPrefix;

    CRmlUIWidgetHandler( const CString& widgetType );

    virtual ~CRmlUIWidgetHandler();

    virtual bool CreateNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyParent( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyChildOrder( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyState( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetNativeHandle( const GUI::CWidget& widget ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    static bool DrawnByElement( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CString& value );

    CRmlUIWidgetHandler( const CRmlUIWidgetHandler& src );
    CRmlUIWidgetHandler& operator=( const CRmlUIWidgetHandler& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H ? */
