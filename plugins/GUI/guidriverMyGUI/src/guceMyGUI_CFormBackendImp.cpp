/*
 *  guidriverMyGUI: glue module for the MyGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

//#ifndef __Exception_H_
//#include "OgreException.h"
//#define __Exception_H_
//#endif /* __Exception_H_ ? */
//
//#ifndef __ROOT__
//#include "OgreRoot.h"
//#define __ROOT__
//#endif /* __ROOT__ ? */

#ifndef __MYGUI_XML_DOCUMENT_H__
#include "MyGUI_XmlDocument.h"
#define __MYGUI_XML_DOCUMENT_H__
#endif /* __MYGUI_XML_DOCUMENT_H__ */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_MYGUI_CIOACCESSTOMYGUIDATASTREAMADAPTER_H
#include "guceMyGUI_CIOAccessToMyGuiDataStreamAdapter.h"
#define GUCEF_MYGUI_CIOACCESSTOMYGUIDATASTREAMADAPTER_H
#endif /* GUCEF_MYGUI_CIOACCESSTOMYGUIDATASTREAMADAPTER_H ? */

#ifndef GUIDRIVER_MYGUI_WIDGETS_H
#include "guidriverMyGUI_widgets.h"
#define GUIDRIVER_MYGUI_WIDGETS_H
#endif /* GUIDRIVER_MYGUI_WIDGETS_H ? */

#include "guceMyGUI_CFormBackendImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFormBackendImp::CFormBackendImp( void )
    : CFormBackend()        ,
      m_widgetMap()         ,
      m_rootWindow( GUCEF_NULL )  ,
      m_widgetNamePrefix()  ,
      m_resourceGroupName() //,
      //m_dummyArchive( NULL )
{GUCEF_TRACE;

    //m_resourceGroupName = "GUCE::MYGUIOGRE::CFormBackendImp::" + GUCEF::CORE::PointerToString( this );
    m_widgetNamePrefix = m_resourceGroupName + '/';

    //// provide hacky access to data we get in the CFormBackend API
    //Ogre::Root::getSingletonPtr()->addResourceLocation( m_resourceGroupName      ,
    //                                                    "GUCEF::CORE::CIOAccess" ,
    //                                                    m_resourceGroupName      );
    //m_dummyArchive = (*CORE::CIOAccessArchiveFactory::Instance()->GetArchiveMap().find( m_resourceGroupName )).second;
}

/*-------------------------------------------------------------------------*/

CFormBackendImp::~CFormBackendImp()
{GUCEF_TRACE;

    //Ogre::Root::getSingletonPtr()->removeResourceLocation( m_resourceGroupName, m_resourceGroupName );
    //m_dummyArchive = NULL;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CFormBackendImp::CreateAndHookWrapperForWindow( MyGUI::Widget* widget )
{GUCEF_TRACE;

    GUCEF::GUI::CWidget* widgetWrapper = GUCEF_NULL;
    CString typeName( CString( widget->getTypeName().c_str() ).SubstrToChar( '/', false ) );

    if ( "Widget" == typeName )
    {
        widgetWrapper = new TBasicWidgetImp();
        static_cast< TBasicWidgetImp* >( widgetWrapper )->Hook( widget );
    }
    else
    if ( "Button" == typeName )
    {
        widgetWrapper = new CButtonImp();
        static_cast< CButtonImp* >( widgetWrapper )->Hook( static_cast< MyGUI::Button* >( widget ) );
    }
    //else
    //if ( "Editbox" == typeName )
    //{
    //    widgetWrapper = new CEditboxImp();
    //    static_cast< CEditboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Editbox* >( widget ) );
    //}
    //else
    //if ( "Listbox" == typeName )
    //{
    //    widgetWrapper = new CListboxImp();
    //    static_cast< CListboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Listbox* >( widget ) );
    //}
    //else
    //if ( "StaticImage" == typeName )
    //{
    //    CString widgetName( CString( window->getName().c_str() ).SubstrToChar( '/', false ) );
    //
    //    if ( widgetName == "RenderContext" )
    //    {
    //        widgetWrapper = new CRenderContextImp();
    //        static_cast< CRenderContextImp* >( widgetWrapper )->Hook( window );
    //    }
    //    else
    //    {
    //        widgetWrapper = new CImageFrameImp();
    //        static_cast< CImageFrameImp* >( widgetWrapper )->Hook( window );
    //    }
    //}
    //else
    //if ( "StaticText" == typeName )
    //{
    //    widgetWrapper = new CLabelImp();
    //    static_cast< CLabelImp* >( widgetWrapper )->Hook( window );
    //}
    //else
    //if ( "Checkbox" == typeName )
    //{
    //    widgetWrapper = new CCheckboxImp();
    //    static_cast< CCheckboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Checkbox* >( window ) );
    //}
    //else
    //if ( "Combobox" == typeName )
    //{
    //    widgetWrapper = new CComboboxImp();
    //    static_cast< CComboboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Combobox* >( window ) );
    //}
    //if ( "TabControl" == typeName )
    //{
    //    widgetWrapper = new CTabControlImp();
    //    static_cast< CTabControlImp* >( widgetWrapper )->Hook( static_cast< CEGUI::TabControl* >( window ) );
    //}
    //else
    //if ( "TabContentPane" == typeName )
    //{
    //    widgetWrapper = new CTabContentPaneImp();
    //    static_cast< CTabContentPaneImp* >( widgetWrapper )->Hook( window );
    //}
    else
    if ( "Window" == typeName )
    {
        widgetWrapper = new CWindowImp();
        static_cast< CWindowImp* >( widgetWrapper )->Hook( static_cast< MyGUI::Window* >( widget ) );
    }
    //else
    //if ( "MultiColumnList" == typeName )
    //{
    //    widgetWrapper = new CGridViewImp();
    //    static_cast< CGridViewImp* >( widgetWrapper )->Hook( static_cast< CEGUI::MultiColumnList* >( window ) );
    //}
    //else
    //if ( "Spinner" == typeName )
    //{
    //    widgetWrapper = new CSpinnerImp();
    //    static_cast< CSpinnerImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Spinner* >( window ) );
    //}
    //else
    //if ( "DefaultWindow" == typeName )
    //{
    //    widgetWrapper = new TBasicWidgetImp();
    //    static_cast< TBasicWidgetImp* >( widgetWrapper )->Hook( window );
    //}
    else
    {
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Unable match widget type to wrapper for widget type: " + typeName );
    }

    if ( GUCEF_NULL != widgetWrapper )
    {
        GUCEF_DEBUG_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Wrapped and hooked GUI widget of type " + widgetWrapper->GetType() + " for MyGUI widget of type " + typeName );
    }
    return widgetWrapper;
}

/*-------------------------------------------------------------------------*/

void
CFormBackendImp::WrapAndHookChildWindows( MyGUI::Widget* widget )
{GUCEF_TRACE;

    GUCEF::GUI::CWidget* wrappedWidget = GUCEF_NULL;
    MyGUI::Widget* childWidget = GUCEF_NULL;

    MyGUI::EnumeratorWidgetPtr childWidgets = widget->getEnumerator();
    while ( childWidgets.next() )
    {
        childWidget = childWidgets.current();
        wrappedWidget = CreateAndHookWrapperForWindow( childWidget );
        if ( GUCEF_NULL != wrappedWidget )
        {
            CString localWidgetName = wrappedWidget->GetName().SubstrToChar( '/', false );
            m_widgetMap[ localWidgetName ] = wrappedWidget;
        }

        WrapAndHookChildWindows( childWidget );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::LoadLayout( const GUCEF::CORE::CString& layoutStoragePath )
{GUCEF_TRACE;

    MyGUI::VectorWidgetPtr rootWidgets;
    MyGUI::LayoutManager* lmgr = MyGUI::LayoutManager::getInstancePtr();

    GUCEF_DEBUG_LOG( 0, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Starting layout load for a GUI Form" );

    try
    {
        rootWidgets = lmgr->loadLayout( layoutStoragePath.STL_String()  ,
                                        m_widgetNamePrefix.STL_String() ,
                                        nullptr                         );
    }
    catch ( std::exception& e )
    {
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, CString( "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): exception while attempting to load form layout: " ) + e.what() );
        return false;
    }

    // Now that we completed loading lets see what we got from MyGUI
    m_rootWindow = GUCEF_NULL;
    MyGUI::VectorWidgetPtr::iterator i = rootWidgets.begin();
    while ( i != rootWidgets.end() )
    {
        // Begin by providing a wrapper for the root window
        MyGUI::Widget* rootWidget = (*i);
        GUCEF::GUI::CWidget* wrappedRootWidget = CreateAndHookWrapperForWindow( rootWidget );
        if ( GUCEF_NULL != wrappedRootWidget )
        {
            CString localWidgetName = wrappedRootWidget->GetName().SubstrToChar( '/', false );
            m_widgetMap[ localWidgetName ] = wrappedRootWidget;
            if ( GUCEF_NULL == m_rootWindow )
            {
                m_rootWindow = wrappedRootWidget;
            }
            WrapAndHookChildWindows( rootWidget );
        }
        rootWidget->setVisible( false );
        ++i;
    }

    GUCEF_DEBUG_LOG( 0, "CFormBackendImp(" + GUCEF::CORE::PointerToString( this ) + "): Successfully loaded a GUI Form layout" );
    return true;
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

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::SaveLayout( GUCEF::CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
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

    return GUCEF_NULL;
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

}; /* namespace MYGUIOGRE */
}; /* namespace GUCE */

/*-------------------------------------------------------------------------*/
