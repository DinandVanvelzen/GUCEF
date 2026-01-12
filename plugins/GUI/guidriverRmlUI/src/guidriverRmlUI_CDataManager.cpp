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
//#ifndef GUCE_MYGUIOGRE_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H
//#include "guceMyGUIOgre_CVfsHandleToMyGUIDataStreamAdapter.h"
//#define GUCE_MYGUIOGRE_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H
//#endif /* GUCE_MYGUIOGRE_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H ? */
//
//#ifndef GUCEF_VFS_CVFS_H
//#include "gucefVFS_CVFS.h"
//#define GUCEF_VFS_CVFS_H
//#endif /* GUCEF_VFS_CVFS_H ? */
//
//#include "guceMyGUIOgre_CDataManager.h"
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
//MYGUI_INSTANCE_IMPLEMENT( CDataManager )
//
///*-------------------------------------------------------------------------*/
//
//void
//CDataManager::SetGuiDataRoot( const CORE::CString& guiDataRootPath )
//{GUCE_TRACE;
//
//    m_guiDataRoot = guiDataRootPath;
//}
//
///*-------------------------------------------------------------------------*/
//
//const CORE::CString
//CDataManager::GetGuiDataRoot( void ) const
//{GUCE_TRACE;
//
//    return m_guiDataRoot;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CDataManager::SetResourceGroup( const CORE::CString& guiResourceGroup )
//{GUCE_TRACE;
//
//    m_resourceGroup = guiResourceGroup;
//}
//
///*-------------------------------------------------------------------------*/
//
//const CORE::CString&
//CDataManager::GetResourceGroup( void ) const
//{GUCE_TRACE;
//
//    return m_resourceGroup;
//}
//
///*-------------------------------------------------------------------------*/
//
//MyGUI::IDataStream* 
//CDataManager::getData( const std::string& name )
//{GUCE_TRACE;
//
//    CORE::CString pathToResource = m_guiDataRoot;
//    GUCEF::CORE::AppendToPath( pathToResource, name.c_str() );
//        
//    GUCEF::VFS::CVFS::CVFSHandlePtr filePtr = GUCEF::VFS::CVFS::Instance()->GetFile( pathToResource );
//    if ( NULL != filePtr )
//    {
//        return new CVfsHandleToMyGUIDataStreamAdapter( filePtr );
//    }
//    return NULL;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool 
//CDataManager::isDataExist( const std::string& name )
//{GUCE_TRACE;
//
//    CORE::CString pathToResource = m_guiDataRoot;
//    GUCEF::CORE::AppendToPath( pathToResource, name.c_str() );
//
//    return GUCEF::VFS::CVFS::Instance()->FileExists( pathToResource );
//}
//
///*-------------------------------------------------------------------------*/
//
//const MyGUI::VectorString& 
//CDataManager::getDataListNames( const std::string& _pattern )
//{GUCE_TRACE;
//
//    return getDataListNames( _pattern, false );
//}
//
///*-------------------------------------------------------------------------*/
//
//const MyGUI::VectorString& 
//CDataManager::getDataListNames( const std::string& pattern, bool fullpath )
//{GUCE_TRACE;
//
//    static MyGUI::VectorString result;
//    result.clear();
//
//    GUCEF::VFS::CVFS::TStringSet vfsResultSet;
//    GUCEF::VFS::CVFS::Instance()->GetList( vfsResultSet , 
//                                           ""           , 
//                                           true         , 
//                                           fullpath     , 
//                                           pattern      , 
//                                           true         , 
//                                           false        );
//
//    
//    GUCEF::VFS::CVFS::TStringSet::iterator i = vfsResultSet.begin();
//    while ( i != vfsResultSet.end() )
//    {
//        result.push_back( (*i) );        
//        ++i;
//    }
//    return result;
//}
//
///*-------------------------------------------------------------------------*/
//
//const std::string&
//CDataManager::getDataPath( const std::string& name )
//{GUCE_TRACE;
//
//    static std::string result;
//
//    const MyGUI::VectorString& files = getDataListNames( name, true );
//    result = files.size() == 1 ? files[0] : "";
//
//    return result;
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

