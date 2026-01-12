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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#include <RmlUI/Core.h>

//
//#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
//#include "dvcppstringutils.h"
//#define GUCEF_CORE_DVCPPSTRINGUTILS_H
//#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

//#ifndef GUCE_CORE_CIOACCESSARCHIVEFACTORY_H
//#include "CIOAccessArchiveFactory.h"
//#define GUCE_CORE_CIOACCESSARCHIVEFACTORY_H
//#endif /* GUCE_CORE_CIOACCESSARCHIVEFACTORY_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CIOACCESSTOMYGUIDATASTREAMADAPTER_H
//#include "guceMyGUIOgre_CIOAccessToMyGUIDataStreamAdapter.h"
//#define GUCE_MYGUIOGRE_CIOACCESSTOMYGUIDATASTREAMADAPTER_H
//#endif /* GUCE_MYGUIOGRE_CIOACCESSTOMYGUIDATASTREAMADAPTER_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CBUTTONIMP_H
//#include "guceMyGUIOgre_CButtonImp.h"
//#define GUCE_MYGUIOGRE_CBUTTONIMP_H
//#endif /* GUCE_MYGUIOGRE_CBUTTONIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CEDITBOXIMP_H
//#include "guceMyGUIOgre_CEditboxImp.h"
//#define GUCE_MYGUIOGRE_CEDITBOXIMP_H
//#endif /* GUCE_MYGUIOGRE_CEDITBOXIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CLISTBOXIMP_H
//#include "guceMyGUIOgre_CListboxImp.h"
//#define GUCE_MYGUIOGRE_CLISTBOXIMP_H
//#endif /* GUCE_MYGUIOGRE_CLISTBOXIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CWINDOWIMP_H
//#include "guceMyGUIOgre_CWindowImp.h"
//#define GUCE_MYGUIOGRE_CWINDOWIMP_H
//#endif /* GUCE_MYGUIOGRE_CWINDOWIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CIMAGEFRAMEIMP_H
//#include "guceMyGUIOgre_CImageFrameImp.h"
//#define GUCE_MYGUIOGRE_CIMAGEFRAMEIMP_H
//#endif /* GUCE_MYGUIOGRE_CIMAGEFRAMEIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CHECKBOXIMP_H
//#include "guceMyGUIOgre_CCheckboxImp.h"
//#define GUCE_MYGUIOGRE_CHECKBOXIMP_H
//#endif /* GUCE_MYGUIOGRE_CHECKBOXIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CCOMBOBOXIMP_H
//#include "guceMyGUIOgre_CComboboxImp.h"
//#define GUCE_MYGUIOGRE_CCOMBOBOXIMP_H
//#endif /* GUCE_MYGUIOGRE_CCOMBOBOXIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CTABCONTROLIMP_H
//#include "guceMyGUIOgre_CTabControlImp.h"
//#define GUCE_MYGUIOGRE_CTABCONTROLIMP_H
//#endif /* GUCE_MYGUIOGRE_CTABCONTROLIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CTABCONTENTPANEIMP_H
//#include "guceMyGUIOgre_CTabContentPaneImp.h"
//#define GUCE_MYGUIOGRE_CTABCONTENTPANEIMP_H
//#endif /* GUCE_MYGUIOGRE_CTABCONTENTPANEIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//#include "guceMyGUIOgre_CGridViewImp.h"
//#define GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//#endif /* GUCE_MYGUIOGRE_CGRIDVIEWIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CLABELIMP_H
//#include "guceMyGUIOgre_CLabelImp.h"
//#define GUCE_MYGUIOGRE_CLABELIMP_H
//#endif /* GUCE_MYGUIOGRE_CLABELIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CSPINNERIMP_H
//#include "guceMyGUIOgre_CSpinnerImp.h"
//#define GUCE_MYGUIOGRE_CSPINNERIMP_H
//#endif /* GUCE_MYGUIOGRE_CSPINNERIMP_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H
//#include "guceMyGUIOgre_CRenderContextImp.h"
//#define GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H
//#endif /* GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CGUICONTEXT_H
#include "guidriverRmlUI_CGUIContext.h"
#define GUCEF_guidriverRmlUI_CGUICONTEXT_H
#endif /* GUCEF_guidriverRmlUI_CGUICONTEXT_H ? */

#ifndef GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H
#include "guidriverRmlUI_CRmlUIStreamAdapter.h"
#define GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H
#endif /* GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H ? */

#include "guidriverRmlUI_CFormBackendImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFormBackendImp::CFormBackendImp( CGUIContext* context )
    : CFormBackend()        ,
      m_widgetMap()         ,
      m_rootWindow( NULL )  ,
      m_widgetNamePrefix()  ,
      m_resourceGroupName() ,
      m_context( context )  
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/
    
CFormBackendImp::~CFormBackendImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
//
//GUCEF::GUI::CWidget*
//CFormBackendImp::CreateAndHookWrapperForWindow( MyGUI::Widget* widget )
//{GUCE_TRACE;
//
//    GUCEF::GUI::CWidget* widgetWrapper = NULL;
//    CString typeName( CString( widget->getTypeName().c_str() ).SubstrToChar( '/', false ) );
//
//    if ( "Widget" == typeName )
//    {
//        widgetWrapper = new TBasicWidgetImp();
//        static_cast< TBasicWidgetImp* >( widgetWrapper )->Hook( widget );
//    }
//    else    
//    if ( "Button" == typeName )
//    {
//        widgetWrapper = new CButtonImp();
//        static_cast< CButtonImp* >( widgetWrapper )->Hook( static_cast< MyGUI::Button* >( widget ) );
//    }
//    //else
//    //if ( "Editbox" == typeName )
//    //{
//    //    widgetWrapper = new CEditboxImp();
//    //    static_cast< CEditboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Editbox* >( widget ) );        
//    //}
//    //else
//    //if ( "Listbox" == typeName )
//    //{
//    //    widgetWrapper = new CListboxImp();
//    //    static_cast< CListboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Listbox* >( widget ) );        
//    //}
//    //else
//    //if ( "StaticImage" == typeName )
//    //{
//    //    CString widgetName( CString( window->getName().c_str() ).SubstrToChar( '/', false ) );
//    //    
//    //    if ( widgetName == "RenderContext" )
//    //    {
//    //        widgetWrapper = new CRenderContextImp();
//    //        static_cast< CRenderContextImp* >( widgetWrapper )->Hook( window );
//    //    }
//    //    else
//    //    {
//    //        widgetWrapper = new CImageFrameImp();
//    //        static_cast< CImageFrameImp* >( widgetWrapper )->Hook( window );
//    //    }
//    //}
//    //else    
//    //if ( "StaticText" == typeName )
//    //{
//    //    widgetWrapper = new CLabelImp();
//    //    static_cast< CLabelImp* >( widgetWrapper )->Hook( window );
//    //}    
//    //else    
//    //if ( "Checkbox" == typeName )
//    //{
//    //    widgetWrapper = new CCheckboxImp();
//    //    static_cast< CCheckboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Checkbox* >( window ) );        
//    //}
//    //else    
//    //if ( "Combobox" == typeName )
//    //{
//    //    widgetWrapper = new CComboboxImp();
//    //    static_cast< CComboboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Combobox* >( window ) );        
//    //}
//    //if ( "TabControl" == typeName )
//    //{
//    //    widgetWrapper = new CTabControlImp();
//    //    static_cast< CTabControlImp* >( widgetWrapper )->Hook( static_cast< CEGUI::TabControl* >( window ) );        
//    //}        
//    //else
//    //if ( "TabContentPane" == typeName )
//    //{
//    //    widgetWrapper = new CTabContentPaneImp();
//    //    static_cast< CTabContentPaneImp* >( widgetWrapper )->Hook( window );        
//    //}        
//    else        
//    if ( "Window" == typeName )
//    {
//        widgetWrapper = new CWindowImp();
//        static_cast< CWindowImp* >( widgetWrapper )->Hook( static_cast< MyGUI::Window* >( widget ) );        
//    }
//    //else        
//    //if ( "MultiColumnList" == typeName )
//    //{
//    //    widgetWrapper = new CGridViewImp();
//    //    static_cast< CGridViewImp* >( widgetWrapper )->Hook( static_cast< CEGUI::MultiColumnList* >( window ) );        
//    //}
//    //else        
//    //if ( "Spinner" == typeName )
//    //{
//    //    widgetWrapper = new CSpinnerImp();
//    //    static_cast< CSpinnerImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Spinner* >( window ) );        
//    //}        
//    //else
//    //if ( "DefaultWindow" == typeName )
//    //{
//    //    widgetWrapper = new TBasicWidgetImp();
//    //    static_cast< TBasicWidgetImp* >( widgetWrapper )->Hook( window );
//    //}
//    else
//    {
//        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Unable match widget type to wrapper for widget type: " + typeName );
//    }    
//    
//    if ( NULL != widgetWrapper )
//    {
//        GUCEF_DEBUG_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Wrapped and hooked GUI widget of type " + widgetWrapper->GetType() + " for MyGUI widget of type " + typeName );
//    }
//    return widgetWrapper;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFormBackendImp::WrapAndHookChildWindows( MyGUI::Widget* widget )
//{GUCE_TRACE;
//
//    GUCEF::GUI::CWidget* wrappedWidget = NULL;
//    MyGUI::Widget* childWidget = NULL;
//    
//    MyGUI::EnumeratorWidgetPtr childWidgets = widget->getEnumerator();
//    while ( childWidgets.next() )
//    {
//        childWidget = childWidgets.current();
//        wrappedWidget = CreateAndHookWrapperForWindow( childWidget );
//        if ( NULL != widget )
//        {
//            CString localWidgetName = wrappedWidget->GetName().SubstrToChar( '/', false );
//            m_widgetMap[ localWidgetName ] = wrappedWidget;
//        }
//        
//        WrapAndHookChildWindows( childWidget );                     
//    }
//}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::LoadLayout( const GUCEF::CORE::CString& layoutStoragePath )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr filePtr = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( layoutStoragePath );
    if ( !filePtr.IsNULL() )
    {
        return LoadLayout( *filePtr->GetAccess() );
    }
    return false;        
}

/*-------------------------------------------------------------------------*/
    
bool
CFormBackendImp::SaveLayout( const GUCEF::CORE::CString& layoutStoragePath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::LoadLayout( GUCEF::CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;
    
    CRmlUIStreamAdapter streamAdapter( layoutStorage );

    // Load and show the document.
    RmlUI::Core::ElementDocument* document = m_context->GetRmlUIContext()->LoadDocument( &streamAdapter );
    if ( NULL != document )
    {
        document->Show();
        document->RemoveReference();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CFormBackendImp::SaveLayout( GUCEF::CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/
    
GUI::CWidget*
CFormBackendImp::GetRootWidget( void )
{GUCEF_TRACE;

    return m_rootWindow;
}

/*-------------------------------------------------------------------------*/
    
const GUCEF::GUI::CWidget*
CFormBackendImp::GetRootWidget( void ) const
{GUCEF_TRACE;

    return m_rootWindow;
}
    
/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CFormBackendImp::GetWidget( const CString& widgetName )
{GUCEF_TRACE;

    TWidgetMap::iterator i = m_widgetMap.find( widgetName );
    if ( i != m_widgetMap.end() )
    {
        return (*i).second;
    }
    
    return NULL;
}

/*-------------------------------------------------------------------------*/
    
void
CFormBackendImp::GetWidgetVector( const CString& widgetName   ,
                                  TWidgetVector& widgetVector )
{GUCEF_TRACE;

    TWidgetMap::iterator i = m_widgetMap.begin();
    while ( i != m_widgetMap.end() )
    {
        widgetVector.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

