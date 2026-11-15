/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#include "gucefGUI_CVfsGridExportTarget.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CVfsGridExportTarget::DefaultDisplayName = "Data";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsGridExportTarget::CVfsGridExportTarget( const CString& displayName, const CString& initialDirectory )
    : CIGridExportTarget()
    , m_displayName( displayName )
    , m_initialDirectory( initialDirectory )
    , m_access( ( GUCEF_NEW VFS::CVfsFileSystemAccess( displayName ) )->CreateSharedPtr() )
    , m_openFiles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsGridExportTarget::~CVfsGridExportTarget()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CVfsGridExportTarget::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
CVfsGridExportTarget::GetFileSystemAccess( void ) const
{GUCEF_TRACE;

    return m_access;
}

/*-------------------------------------------------------------------------*/

CString
CVfsGridExportTarget::GetInitialDirectory( void ) const
{GUCEF_TRACE;

    return m_initialDirectory.IsNULLOrEmpty() ? CString( "/" ) : m_initialDirectory;
}

/*-------------------------------------------------------------------------*/

void
CVfsGridExportTarget::Close( const CString& temporary, CORE::IOAccessPtr& stream )
{GUCEF_TRACE;

    if ( !stream.IsNULL() )
    {
        stream->Close();
        stream.Unlink();
    }
    m_openFiles.erase( temporary );
}

/*-------------------------------------------------------------------------*/

bool
CVfsGridExportTarget::OpenForWriting( const CString& path, CORE::IOAccessPtr& stream, CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString temporary = GetTemporaryPath( path );
    if ( !vfs.IsWriteable( temporary ) )
    {
        error = "\"" + path + "\" is in a part of the data that cannot be written";
        return false;
    }
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( temporary, "wb", true );
    if ( file.IsNULL() || file->GetAccess().IsNULL() || !file->GetAccess()->IsValid() )
    {
        error = "\"" + temporary + "\" cannot be made";
        return false;
    }
    m_openFiles[ temporary ] = file;
    stream = file->GetAccess();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsGridExportTarget::Commit( const CString& path, CORE::IOAccessPtr& stream, CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString temporary = GetTemporaryPath( path );
    Close( temporary, stream );
    if ( !vfs.MoveFile( temporary, path, true ) )
    {
        vfs.DeleteFile( temporary, true );
        error = "\"" + path + "\" could not be replaced";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsGridExportTarget::Abandon( const CString& path, CORE::IOAccessPtr& stream )
{GUCEF_TRACE;

    const CString temporary = GetTemporaryPath( path );
    Close( temporary, stream );
    VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( temporary, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
