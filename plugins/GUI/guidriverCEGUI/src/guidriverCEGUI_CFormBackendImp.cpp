/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_WIDGETS_H
#include "guidriverCEGUI_widgets.h"
#define GUCEF_GUIDRIVERCEGUI_WIDGETS_H
#endif /* GUCEF_GUIDRIVERCEGUI_WIDGETS_H ? */

#include "guidriverCEGUI_CFormBackendImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFormBackendImp::CFormBackendImp( void )
    : CFormBackend()        ,
      m_widgetMap()         ,
      m_rootWindow( GUCEF_NULL )  ,
      m_widgetNamePrefix()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CFormBackendImp::~CFormBackendImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CFormBackendImp::CreateAndHookWrapperForWindow( CEGUI::Window* window )
{GUCEF_TRACE;

    GUCEF::GUI::CWidget* widgetWrapper = GUCEF_NULL;
    CString typeName( CString( window->getType().c_str() ).SubstrToChar( '/', false ) );
    
    //if ( "Button" == typeName )
    //{
    //    widgetWrapper = new CButtonImp();
    //    static_cast< CButtonImp* >( widgetWrapper )->Hook( static_cast< CEGUI::PushButton* >( window ) );
    //}
    //else
    //if ( "Editbox" == typeName )
    //{
    //    widgetWrapper = new CEditboxImp();
    //    static_cast< CEditboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Editbox* >( window ) );        
    //}
    //else
    //if ( "Listbox" == typeName )
    //{
    //    widgetWrapper = new CListboxImp();
    //    static_cast< CListboxImp* >( widgetWrapper )->Hook( static_cast< CEGUI::Listbox* >( window ) );        
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
    //else        
    if ( "FrameWindow" == typeName )
    {
        widgetWrapper = new CWindowImp();
        static_cast< CWindowImp* >( widgetWrapper )->Hook( static_cast< CEGUI::FrameWindow* >( window ) );        
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
    
    GUCEF_DEBUG_LOG( 0, "Wrapped and hooked GUI widget for CEGUI widget of type " + typeName );
    return widgetWrapper;
}

/*-------------------------------------------------------------------------*/

void
CFormBackendImp::WrapAndHookChildWindows( CEGUI::Window* window )
{GUCEF_TRACE;

    GUCEF::GUI::CWidget* widget = GUCEF_NULL;
    CEGUI::Window* childWindow = GUCEF_NULL;
    size_t i=0;
    size_t childCount = window->getChildCount();
    while ( i < childCount )
    {
        childWindow = window->getChildAtIdx( i );
        widget = CreateAndHookWrapperForWindow( childWindow );
        if ( GUCEF_NULL != widget )
        {
            CString localWidgetName = widget->GetName().SubstrToChar( '/', false );
            m_widgetMap[ localWidgetName ] = widget;
        }
        
        WrapAndHookChildWindows( childWindow );
        
        ++i;                       
    }
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::LoadLayout( const GUCEF::CORE::CString& layoutStoragePath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( layoutStoragePath );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() )
        {
            return LoadLayout( *fileAccess );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::LoadLayout( GUCEF::CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;
    
    CEGUI::Window* rootWindow = GUCEF_NULL;
    CEGUI::WindowManager* wmgr = CEGUI::WindowManager::getSingletonPtr();
    
    GUCEF_DEBUG_LOG( 0, "Starting layout load for a GUI Form" );
    
    try
    {
        CORE::CDynamicBuffer memBuffer( layoutStorage );
        CEGUI::RawDataContainer container;

        container.setData( (CEGUI::uint8*) memBuffer.GetBufferPtr() );
        container.setSize( (size_t) memBuffer.GetDataSize() );

        rootWindow = wmgr->loadLayoutFromContainer( container );

        container.setData( (CEGUI::uint8*) GUCEF_NULL );
        container.setSize( (size_t) 0 );
    }
    catch ( CEGUI::Exception& e )
    {
        GUCEF_ERROR_LOG( 0, CString( "CEGUI Exception while attempting to load form layout: " ) + e.what() );
        return false;
    }
    
    // Now that we completed loading lets see what we got from CEGUI
    if ( GUCEF_NULL != rootWindow )
    {
        // Begin by providing a wrapper for the root window
        m_rootWindow = CreateAndHookWrapperForWindow( rootWindow );
        if ( GUCEF_NULL != m_rootWindow )
        {
            CString localWidgetName = m_rootWindow->GetName().SubstrToChar( '/', false );
            m_widgetMap[ localWidgetName ] = m_rootWindow;
            WrapAndHookChildWindows( rootWindow );
            
            // We will directly add the form as a child of the root for now
            // Note: This assumes that you have a GUISheet already set, otherwise this will result in a segfault!
            CEGUI::Window* globalRootWindow = CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();
            if ( GUCEF_NULL != globalRootWindow )
            {
                globalRootWindow->addChild( rootWindow );
                
                GUCEF_DEBUG_LOG( 0, "Successfully loaded a GUI Form layout" );
                return true;                
            }
            else
            {
                GUCEF_ERROR_LOG( 0, "Failed to add form as a child to the global \"root\" window" );    
            }
        }
        rootWindow->hide();
    }

    GUCEF_DEBUG_LOG( 0, "Failed to load a GUI Form layout" );
    return false; 
}

/*-------------------------------------------------------------------------*/

bool
CFormBackendImp::SaveLayout( const GUCEF::CORE::CString& layoutStoragePath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( layoutStoragePath, "wb", true );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( fileAccess.IsNULL() )
        {
            return SaveLayout( *fileAccess );
        }
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

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
