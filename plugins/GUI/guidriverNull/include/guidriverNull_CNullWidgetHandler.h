/*
 *  guidriverNull: GUCEF GUI driver that keeps the widgets but draws nothing, for tests and tools
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

#ifndef GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H
#define GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUIDRIVERNULL_MACROS_H
#include "guidriverNull_macros.h"
#define GUCEF_GUIDRIVERNULL_MACROS_H
#endif /* GUCEF_GUIDRIVERNULL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the null driver keeps per widget: everything the handler was given. Tests read it through
 *  CWidget::GetNativeHandle() to check what reaches a driver and how often.
 */
struct GUIDRIVERNULL_PUBLIC_CPP SNullNative
{
    typedef GUCEF::map< CORE::CString, CORE::CString > TPropertyMap;

    TPropertyMap properties;        /**< the last value applied per property */
    CORE::UInt32 propertyApplyCount;
    CORE::UInt32 layoutApplyCount;
    CORE::UInt32 stateApplyCount;
    CORE::UInt32 parentApplyCount;
    CORE::UInt32 childOrderApplyCount;
    CORE::UInt32 commandCount;
    GUI::CRect rect;
    CORE::UInt32 state;
    GUI::CWidget* parent;

    SNullNative( void );
};
typedef struct SNullNative TNullNative;

/*-------------------------------------------------------------------------*/

/** The handler for every widget type: it records, it draws nothing */
class GUIDRIVERNULL_PUBLIC_CPP CNullWidgetHandler : public GUI::CWidgetHandler
{
    public:

    CNullWidgetHandler( const CORE::CString& widgetType );

    virtual ~CNullWidgetHandler();

    virtual bool CreateNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyParent( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyChildOrder( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyState( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    /** Every property is taken, also the ones the type does not know (backend hints) */
    virtual bool ApplyProperty( GUI::CWidget& widget, const CORE::CString& name, const CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RunCommand( GUI::CWidget& widget, const CORE::CString& command, const CORE::CValueList& arguments ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetNativeHandle( const GUI::CWidget& widget ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    static bool RecordProperty( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CORE::CString& value );

    CNullWidgetHandler( const CNullWidgetHandler& src );
    CNullWidgetHandler& operator=( const CNullWidgetHandler& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H ? */
