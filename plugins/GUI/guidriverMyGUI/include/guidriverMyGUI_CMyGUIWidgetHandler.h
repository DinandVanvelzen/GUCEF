/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H
#define GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGETHANDLER_H
#include "gucefGUI_CWidgetHandler.h"
#define GUCEF_GUI_CWIDGETHANDLER_H
#endif /* GUCEF_GUI_CWIDGETHANDLER_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMyGUIContext;

/**
 *  The handler of one gucefGUI widget type: it makes the MyGUI widget behind a gucefGUI widget and keeps
 *  it in step. The MyGUI widget is the one that draws, with the skins of MyGUI; gucefGUI keeps the input,
 *  the layout and the values (MyGUI gets no input of its own), so every widget behaves as it does on the
 *  other drivers and looks as MyGUI makes it look.
 *
 *  The kinds: Widget (panels, forms, containers), Label, Button, Checkbox, Editbox, Listbox, Combobox,
 *  ProgressBar, Slider, ImageFrame and Window. Types without a handler of their own (Spinner, grid views,
 *  charts ...) get the one of the closest registered base type, which is Widget at the least.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIWidgetHandler : public GUI::CWidgetHandler
{
    public:

    enum EKind
    {
        KIND_WIDGET = 0 ,
        KIND_LABEL      ,
        KIND_BUTTON     ,
        KIND_CHECKBOX   ,
        KIND_EDITBOX    ,
        KIND_LISTBOX    ,
        KIND_COMBOBOX   ,
        KIND_PROGRESSBAR,
        KIND_SLIDER     ,
        KIND_IMAGEFRAME ,
        KIND_WINDOW
    };

    /** The kind that handles a registered widget type name; false when the type has no handler of its own */
    static bool GetKindOfType( const CString& widgetType, EKind& kind );

    CMyGUIWidgetHandler( EKind kind );

    virtual ~CMyGUIWidgetHandler();

    EKind GetKind( void ) const;

    virtual bool CreateNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyChildOrder( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyState( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetNativeHandle( const GUI::CWidget& widget ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMyGUIWidgetHandler( const CMyGUIWidgetHandler& src );
    CMyGUIWidgetHandler& operator=( const CMyGUIWidgetHandler& src );

    private:

    EKind m_kind;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H ? */
