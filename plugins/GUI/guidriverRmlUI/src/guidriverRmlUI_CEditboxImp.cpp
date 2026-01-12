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
//#ifndef GUCEF_CORE_CTRACER_H
//#include "CTracer.h"
//#define GUCEF_CORE_CTRACER_H
//#endif /* GUCEF_CORE_CTRACER_H ? */
//
//#include "guceMyGUIOgre_CEditboxImp.h"
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
//CEditboxImp::CEditboxImp( void )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CEditboxImp::~CEditboxImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CEditboxImp::SetText( const CString& text )
//{GUCE_TRACE;
//
//    if ( text.Length() > 0 )
//    {
//        m_editbox->setText( text.C_String() );
//    }
//    else
//    {
//        m_editbox->setText( "" );
//    }
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CEditboxImp::GetText( CString& text )
//{GUCE_TRACE;
//
//    text = m_editbox->getText().c_str();
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CEditboxImp::SetSelectedText( const CString& text )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CEditboxImp::GetSelectedText( CString& text )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CEditboxImp::Clear( void )
//{GUCE_TRACE;
//
//    m_editbox->setText( "" );
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CEditboxImp::Hook( CEGUI::Editbox* editbox )
//{GUCE_TRACE;
//
//    m_editbox = editbox;
//    CWidgetImp< GUCEF::GUI::CEditbox >::Hook( editbox );
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace GUI */
//}; /* namespace GUCEF */
//
///*-------------------------------------------------------------------------*/

