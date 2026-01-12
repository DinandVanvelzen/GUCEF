/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

#ifndef GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H
#define GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_GUI_CFORMBACKEND_H
#include "gucefGUI_CFormBackend.h"
#define GUCEF_GUI_CFORMBACKEND_H
#endif /* GUCEF_GUI_CFORMBACKEND_H ? */

#ifndef GUCEF_guidriverRmlUI_CWIDGETIMP_H
#include "guidriverRmlUI_CWidgetImp.h"
#define GUCEF_guidriverRmlUI_CWIDGETIMP_H
#endif /* GUCEF_guidriverRmlUI_CWIDGETIMP_H ? */

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

class CGUIContext;

/*-------------------------------------------------------------------------*/

/**
 *  Class used by the form class to abstract from the backend.
 *  This allows you to descend from the CForm class as you wish without having
 *  to create a matching backend implementation for every form type.
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CFormBackendImp : public GUI::CFormBackend 
{
    public:
    
    typedef std::map< CString, GUI::CWidget* > TWidgetMap;

    CFormBackendImp( CGUIContext* context );
    
    virtual ~CFormBackendImp();

    virtual bool LoadLayout( const CORE::CString& layoutStoragePath );
    
    virtual bool SaveLayout( const CORE::CString& layoutStoragePath );

    virtual bool LoadLayout( CORE::CIOAccess& layoutStorage );
    
    virtual bool SaveLayout( CORE::CIOAccess& layoutStorage );
    
    virtual GUCEF::GUI::CWidget* GetRootWidget( void );
    
    virtual const GUI::CWidget* GetRootWidget( void ) const;
    
    virtual GUI::CWidget* GetWidget( const CString& widgetName );
    
    virtual void GetWidgetVector( const CString& widgetName   ,
                                  TWidgetVector& widgetVector );
                                  
    private:
    
    CFormBackendImp( void );
    CFormBackendImp( const CFormBackendImp& src );
    CFormBackendImp& operator=( const CFormBackendImp& other );
    
    //GUCEF::GUI::CWidget*
    //CreateAndHookWrapperForWindow( MyGUI::Widget* window );    
    //
    //void
    //WrapAndHookChildWindows( MyGUI::Widget* window );
    
    private:
    
    TWidgetMap m_widgetMap;
    GUCEF::GUI::CWidget* m_rootWindow;
    CString m_widgetNamePrefix;
    CString m_resourceGroupName;
    CGUIContext* m_context;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/

