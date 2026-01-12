///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU Lesser General Public
// *  License along with this library; if not, write to the Free Software
// *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
// */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//
//#include "guceMyGUIOgre_CWindowImp.h"
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCE {
//namespace MYGUIOGRE {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CWindowImp::CWindowImp( void )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//CWindowImp::~CWindowImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CWindowImp::SetTitleBarText( const CString& title )
//{GUCE_TRACE;
//
//    if ( NULL != m_window )
//    {
//        m_window->setCaption( title.STL_String() );
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CWindowImp::GetTitleBarText( CString& title ) const
//{GUCE_TRACE;
//
//    if ( NULL != m_window )
//    {
//        title = m_window->getCaption().asUTF8();
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CWindowImp::SetTitleBarVisible( const bool isVisible )
//{GUCE_TRACE;
//
//    if ( NULL != m_window )
//    {
//        MyGUI::Widget* captionWidget = m_window->getCaptionWidget();
//        if ( NULL != captionWidget )
//        {
//            captionWidget->setVisible( isVisible );
//            return true;
//        }
//    }
//    return false;
//}
//    
///*-------------------------------------------------------------------------*/
//
//bool
//CWindowImp::GetTitleBarVisible( bool& isVisible )
//{GUCE_TRACE;
//
//    if ( NULL != m_window )
//    {
//        MyGUI::Widget* captionWidget = m_window->getCaptionWidget();
//        if ( NULL != captionWidget )
//        {
//            captionWidget->isVisible();
//            return true;
//        }
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CWindowImp::ShowModal( void )
//{GUCE_TRACE;
//
//    if ( NULL != m_window )
//    {
//        //m_window->setModalState( true );
//        //m_window->moveToFront();
//        CWidgetImp< GUCEF::GUI::CWindow >::SetVisibility( true );
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CWindowImp::Hide( void )
//{GUCE_TRACE;
//
//    //m_window->setModalState( false );
//    CWidgetImp< GUCEF::GUI::CWindow >::SetVisibility( false );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CWindowImp::Hook( MyGUI::Window* window )
//{GUCE_TRACE;
//
//    m_window = window;
//    CWidgetImp< GUCEF::GUI::CWindow >::Hook( window );
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace MYGUIOGRE */
//}; /* namespace GUCE */
//
///*-------------------------------------------------------------------------*/
