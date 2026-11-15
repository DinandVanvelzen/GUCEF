/*
 *  guidriverMyGUI: GUI backend using MyGUI
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H
#include "guidriverMyGUI_CMyGUIVfsDataStream.h"
#define GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H ? */

#include "guidriverMyGUI_CMyGUIDataManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIDataManager::CMyGUIDataManager( void )
    : MyGUI::DataManager()
    , m_searchPaths()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIDataManager::~CMyGUIDataManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMyGUIDataManager::AddSearchPath( const CORE::CString& vfsDirectory )
{GUCEF_TRACE;

    for ( TStringVector::const_iterator i=m_searchPaths.begin(); i!=m_searchPaths.end(); ++i )
        if ( (*i) == vfsDirectory )
            return;
    m_searchPaths.push_back( vfsDirectory );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDataManager::RemoveSearchPath( const CORE::CString& vfsDirectory )
{GUCEF_TRACE;

    for ( TStringVector::iterator i=m_searchPaths.begin(); i!=m_searchPaths.end(); ++i )
    {
        if ( (*i) == vfsDirectory )
        {
            m_searchPaths.erase( i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

const CMyGUIDataManager::TStringVector&
CMyGUIDataManager::GetSearchPaths( void ) const
{GUCEF_TRACE;

    return m_searchPaths;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDataManager::ResolvePath( const CORE::CString& name, CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( TStringVector::const_iterator i=m_searchPaths.begin(); i!=m_searchPaths.end(); ++i )
    {
        CORE::CString candidate = (*i);
        CORE::AppendToPath( candidate, name );
        if ( vfs.FileExists( candidate ) )
        {
            vfsPath = candidate;
            return true;
        }
    }
    if ( vfs.FileExists( name ) )
    {
        vfsPath = name;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

MyGUI::IDataStream*
CMyGUIDataManager::getData( const std::string& name ) const
{GUCEF_TRACE;

    CORE::CString vfsPath;
    if ( !ResolvePath( name.c_str(), vfsPath ) )
        return GUCEF_NULL;
    CMyGUIVfsDataStream* stream = GUCEF_NEW CMyGUIVfsDataStream( vfsPath );
    if ( !stream->IsValid() )
    {
        delete stream;
        return GUCEF_NULL;
    }
    return stream;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDataManager::freeData( MyGUI::IDataStream* data )
{GUCEF_TRACE;

    delete data;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDataManager::isDataExist( const std::string& name ) const
{GUCEF_TRACE;

    CORE::CString vfsPath;
    return ResolvePath( name.c_str(), vfsPath );
}

/*-------------------------------------------------------------------------*/

MyGUI::VectorString
CMyGUIDataManager::getDataListNames( const std::string& pattern ) const
{GUCEF_TRACE;

    MyGUI::VectorString names;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( TStringVector::const_iterator i=m_searchPaths.begin(); i!=m_searchPaths.end(); ++i )
    {
        VFS::CVFS::TStringVector files;
        if ( vfs.GetFileList( files, (*i), true, false, CORE::CString( pattern.c_str() ) ) )
        {
            for ( VFS::CVFS::TStringVector::const_iterator n=files.begin(); n!=files.end(); ++n )
                names.push_back( (*n).C_String() );
        }
    }
    return names;
}

/*-------------------------------------------------------------------------*/

std::string
CMyGUIDataManager::getDataPath( const std::string& name ) const
{GUCEF_TRACE;

    CORE::CString vfsPath;
    if ( ResolvePath( name.c_str(), vfsPath ) )
        return vfsPath.C_String();
    return std::string();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
