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

#ifndef GUCEF_MYGUI_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H
#include "guceMyGUI_CVFSHandleToMyGuiDataStreamAdapter.h"
#define GUCEF_MYGUI_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H
#endif /* GUCEF_MYGUI_CVFSHANDLETOMYGUIDATASTREAMADAPTER_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "guceMyGUI_CDataManager.h"

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

CDataManager::CDataManager( void )
    : MyGUI::DataManager()
{
}

/*-------------------------------------------------------------------------*/

CDataManager::~CDataManager()
{
}

/*-------------------------------------------------------------------------*/

void
CDataManager::SetGuiDataRoot( const CORE::CString& guiDataRootPath )
{GUCEF_TRACE;

    m_guiDataRoot = guiDataRootPath;
}

/*-------------------------------------------------------------------------*/

const CORE::CString
CDataManager::GetGuiDataRoot( void ) const
{GUCEF_TRACE;

    return m_guiDataRoot;
}

/*-------------------------------------------------------------------------*/

void
CDataManager::SetResourceGroup( const CORE::CString& guiResourceGroup )
{GUCEF_TRACE;

    m_resourceGroup = guiResourceGroup;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDataManager::GetResourceGroup( void ) const
{GUCEF_TRACE;

    return m_resourceGroup;
}

/*-------------------------------------------------------------------------*/

MyGUI::IDataStream*
CDataManager::getData( const std::string& name )
{GUCEF_TRACE;

    CORE::CString pathToResource = m_guiDataRoot;
    CORE::AppendToPath( pathToResource, name.c_str() );

    VFS::TBasicVfsResourcePtr filePtr = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( pathToResource );
    if ( !filePtr.IsNULL() )
    {
        return new CVfsHandleToMyGUIDataStreamAdapter( filePtr );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDataManager::isDataExist( const std::string& name )
{GUCEF_TRACE;

    CORE::CString pathToResource = m_guiDataRoot;
    GUCEF::CORE::AppendToPath( pathToResource, name.c_str() );

    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( pathToResource );
}

/*-------------------------------------------------------------------------*/

const MyGUI::VectorString&
CDataManager::getDataListNames( const std::string& _pattern )
{GUCEF_TRACE;

    return getDataListNames( _pattern, false );
}

/*-------------------------------------------------------------------------*/

const MyGUI::VectorString&
CDataManager::getDataListNames( const std::string& pattern, bool fullpath )
{GUCEF_TRACE;

    static MyGUI::VectorString result;
    result.clear();

    GUCEF::VFS::CVFS::TStringVector vfsResultSet;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( vfsResultSet   ,
                                                       CString::Empty ,
                                                       true           ,
                                                       fullpath       ,
                                                       pattern        );


    GUCEF::VFS::CVFS::TStringVector::iterator i = vfsResultSet.begin();
    while ( i != vfsResultSet.end() )
    {
        result.push_back( (*i) );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

const std::string&
CDataManager::getDataPath( const std::string& name )
{GUCEF_TRACE;

    static std::string result;

    const MyGUI::VectorString& files = getDataListNames( name, true );
    result = files.size() == 1 ? files[0] : "";

    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUIOGRE */
}; /* namespace GUCE */

/*-------------------------------------------------------------------------*/
