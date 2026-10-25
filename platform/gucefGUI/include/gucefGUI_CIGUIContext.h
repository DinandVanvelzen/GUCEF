/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#define GUCEF_GUI_CIGUICONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CFORMBACKEND_H
#include "gucefGUI_CFormBackend.h"
#define GUCEF_GUI_CFORMBACKEND_H
#endif /* GUCEF_GUI_CFORMBACKEND_H ? */

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

class CGUIDriver;
class CForm;
class CGUIContext;

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CIGUIContext
{    
    public:
    
    typedef GUCEF::set< GUCEF::GUI::CWidget* >  TWidgetSet;
    typedef GUCEF::set< GUCEF::GUI::CForm* >    TFormSet;
    typedef GUCEF::set< CString >               TStringSet;

    virtual ~CIGUIContext();

    /** This context as a gucefGUI context, NULL when it is none (a context of a driver that is not built on CGUIContext); no run time type information needed */
    virtual CGUIContext* AsGUIContext( void );

    /** Makes a widget of the given registered type */
    virtual CWidget* CreateWidget( const CString& widgetType ) = 0;
    
    virtual void DestroyWidget( CWidget* widget ) = 0;
    
    virtual CForm* CreateForm( const CString& formName ) = 0;
    
    virtual void DestroyForm( CForm* form ) = 0;   

    virtual TStringSet GetAvailableFormTypes( void ) = 0;
    
    virtual TStringSet GetAvailableWidgetTypes( void ) = 0;
    
    virtual CFormBackend* CreateFormBackend( void ) = 0;
    
    virtual void DestroyFormBackend( CFormBackend* formBackend ) = 0;
    
    virtual CGUIDriver* GetDriver( void ) = 0;
    
    virtual TWidgetSet GetOwnedWidgets( void ) = 0;
    
    virtual TFormSet GetOwnedForms( void ) = 0;
    
    protected:
    
    CIGUIContext( void );
    
    CIGUIContext( const CIGUIContext& src );    
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIGUIContext, MT::CMutex > TGuiContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/
