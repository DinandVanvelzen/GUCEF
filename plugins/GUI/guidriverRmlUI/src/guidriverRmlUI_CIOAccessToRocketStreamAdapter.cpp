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
//#include "guceMyGUIOgre_CIOAccessToMyGUIDataStreamAdapter.h"
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
////      UTILTIES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CIOAccessToMyGUIDataStreamAdapter::CIOAccessToMyGUIDataStreamAdapter( GUCEF::CORE::CIOAccess& ioAccess )
//    : MyGUI::IDataStream(),
//      m_ioAccess( &ioAccess )
//{
//}
//
///*-------------------------------------------------------------------------*/
//    
//CIOAccessToMyGUIDataStreamAdapter::~CIOAccessToMyGUIDataStreamAdapter()
//{
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CIOAccessToMyGUIDataStreamAdapter::eof()
//{
//    return m_ioAccess->Eof();
//}
//
///*-------------------------------------------------------------------------*/
//    
//std::size_t
//CIOAccessToMyGUIDataStreamAdapter::size()
//{
//    return (std::size_t) m_ioAccess->GetSize();
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CIOAccessToMyGUIDataStreamAdapter::readline( std::string& _source, MyGUI::Char _delim )
//{
//    CString lineContent = m_ioAccess->ReadLine();
//    _source = lineContent.STL_String();
//}
//
///*-------------------------------------------------------------------------*/
//    
//std::size_t
//CIOAccessToMyGUIDataStreamAdapter::read( void* _buf, std::size_t _count )
//{
//    return (std::size_t) m_ioAccess->Read( _buf, (UInt32) _count, 1 );
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
//

