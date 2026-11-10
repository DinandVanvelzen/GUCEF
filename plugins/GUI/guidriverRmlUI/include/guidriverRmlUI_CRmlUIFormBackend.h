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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H
#define GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMBACKEND_H
#include "gucefGUI_CFormBackend.h"
#define GUCEF_GUI_CFORMBACKEND_H
#endif /* GUCEF_GUI_CFORMBACKEND_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Rml { class ElementDocument; }

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRmlUIContext;

/**
 *  Loads RmlUi's own documents (RML with RCSS) as forms: for screens made with RmlUi's tools that are
 *  not gucefGUI layouts. The form gets a gucefGUI root widget (a panel over the whole picture) that is
 *  shown, hidden and stacked like any other form; the document follows it and gets the input that
 *  reaches it. The document's elements are RmlUi's, not gucefGUI widgets: GetWidget() only knows the
 *  root, and the layout cannot be saved.
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIFormBackend : public GUI::CFormBackend
{
    public:

    CRmlUIFormBackend( CRmlUIContext& context );

    virtual ~CRmlUIFormBackend();

    virtual bool LoadLayout( const CString& layoutStoragePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveLayout( const CString& layoutStoragePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadLayout( CORE::CIOAccess& layoutStorage ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveLayout( CORE::CIOAccess& layoutStorage ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidget* GetRootWidget( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUI::CWidget* GetRootWidget( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidget* GetWidget( const CString& widgetName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetWidgetVector( const CString& widgetName   ,
                                  TWidgetVector& widgetVector ) GUCEF_VIRTUAL_OVERRIDE;

    Rml::ElementDocument* GetDocument( void ) const;

    private:

    bool Adopt( Rml::ElementDocument* document );

    void Release( void );

    CRmlUIFormBackend( const CRmlUIFormBackend& src );
    CRmlUIFormBackend& operator=( const CRmlUIFormBackend& src );

    private:

    CRmlUIContext* m_context;
    GUI::CWidget* m_root;
    Rml::ElementDocument* m_document;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H ? */
