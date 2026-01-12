///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
//#ifndef GUCEF_VFS_CVFS_H
//#include "CVFS.h"
//#define GUCEF_VFS_CVFS_H
//#endif /* GUCEF_VFS_CVFS_H ? */
//
//#include "guceMyGUIOgre_CVFSInfoProvider.h"
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
//CVFSInfoProvider::CVFSInfoProvider( void )
//    : CIFileSystemInfoProvider()            ,
//      m_vfs( GUCEF::VFS::CVFS::Instance() )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//CVFSInfoProvider::~CVFSInfoProvider()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CVFSInfoProvider::GetDirectoryList( const CString& path   ,
//                                    const CString& filter ,
//                                    TStringSet& list      )
//{GUCE_TRACE;
//
//    m_vfs->GetList( list   ,  // output
//                    path   ,  // location
//                    false  ,  // recursive
//                    false  ,  // includePathInFilename
//                    filter ,  // filter
//                    false  ,  // add files
//                    true   ); // add dirs
//                    
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CVFSInfoProvider::GetFileList( const CString& path   ,
//                               const CString& filter ,
//                               TStringSet& list      )
//{GUCE_TRACE;
//
//    m_vfs->GetList( list   ,  // output
//                    path   ,  // location
//                    false  ,  // recursive
//                    false  ,  // includePathInFilename
//                    filter ,  // filter
//                    true   ,  // add files
//                    false  ); // add dirs
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CVFSInfoProvider::GetArchiveList( const CString& path   ,
//                                  const CString& filter ,
//                                  TStringSet& list      )
//{GUCE_TRACE;
//
//    m_vfs->GetMountedArchiveList( path   ,  // location
//                                  filter ,  // filter
//                                  false  ,  // recursive
//                                  list   ); // output
//    return true;                           
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

