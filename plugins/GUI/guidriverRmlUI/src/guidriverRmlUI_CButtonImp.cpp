///*
// *  guidriverRmlUI: GUI backend using RmlUI
// *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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
//#ifndef GUCEF_CORE_CTRACER_H
//#include "CTracer.h"
//#define GUCEF_CORE_CTRACER_H
//#endif /* GUCEF_CORE_CTRACER_H ? */
//
//#include "guceMyGUIOgre_CButtonImp.h"
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
//CButtonImp::CButtonImp( void )
//    : CWidgetImp< GUCEF::GUI::CButton >() ,
//      m_button( NULL )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CButtonImp::~CButtonImp()
//{GUCE_TRACE;
//
//    m_button = NULL;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CButtonImp::SetButtonText( const CString& newText )
//{GUCE_TRACE;
//
//    if ( NULL != m_button )
//    {
//        m_button->setCaption( newText.C_String() );
//    }
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CButtonImp::GetButtonText( CString& text ) const
//{GUCE_TRACE;
//
//    if ( NULL != m_button )
//    {
//       text = m_button->getCaption().asUTF8();
//    }
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CButtonImp::Hook( MyGUI::Button* button )
//{GUCE_TRACE;
//
//    m_button = button;
//    CWidgetImp< GUCEF::GUI::CButton >::Hook( button );
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CButtonImp::OnMouseClick( MyGUI::Widget* sender )
//{GUCE_TRACE;
//
//    GUCEF_USER_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "MyGUIOgre: User clicked button \"" + GetName() + "\"" );
//    
//    CWidgetImp< GUCEF::GUI::CButton >::OnMouseClick( sender );    
//    NotifyObservers( ButtonClickedEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CButtonImp::OnMouseButtonPressed( MyGUI::Widget* sender     , 
//                                  int left                  ,
//                                  int top                   ,
//                                  MyGUI::MouseButton button )
//{GUCE_TRACE;
//
//    GUCEF_USER_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "MyGUIOgre: User pressed button \"" + GetName() + "\"" );
//    
//    CWidgetImp< GUCEF::GUI::CButton >::OnMouseButtonPressed( sender ,
//                                                             left   ,
//                                                             top    ,
//                                                             button );
//    NotifyObservers( ButtonDownEvent );
//}
//
///*-------------------------------------------------------------------------*/
//                                       
//void
//CButtonImp::OnMouseButtonReleased( MyGUI::Widget* sender     , 
//                                   int left                  ,
//                                   int top                   ,
//                                   MyGUI::MouseButton button )
//{GUCE_TRACE;
//
//    GUCEF_USER_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "MyGUIOgre: User released button \"" + GetName() + "\"" );
//    
//    CWidgetImp< GUCEF::GUI::CButton >::OnMouseButtonReleased( sender ,
//                                                              left   ,
//                                                              top    ,
//                                                              button );
//    NotifyObservers( ButtonUpEvent );
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

