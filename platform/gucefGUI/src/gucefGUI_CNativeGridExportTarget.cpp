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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#include "gucefGUI_CNativeGridExportTarget.h"

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

const CString CNativeGridExportTarget::DefaultDisplayName = "This computer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNativeGridExportTarget::CNativeGridExportTarget( const CString& displayName, const CString& initialDirectory )
    : CIGridExportTarget()
    , m_displayName( displayName )
    , m_initialDirectory( initialDirectory )
    , m_access( ( GUCEF_NEW CORE::CNativeFileSystemAccess( displayName ) )->CreateSharedPtr() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNativeGridExportTarget::~CNativeGridExportTarget()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CNativeGridExportTarget::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
CNativeGridExportTarget::GetFileSystemAccess( void ) const
{GUCEF_TRACE;

    return m_access;
}

/*-------------------------------------------------------------------------*/

CString
CNativeGridExportTarget::GetInitialDirectory( void ) const
{GUCEF_TRACE;

    if ( !m_initialDirectory.IsNULLOrEmpty() )
        return m_initialDirectory;
    CString home;
    CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home );
    return home;
}

/*-------------------------------------------------------------------------*/

bool
CNativeGridExportTarget::OpenForWriting( const CString& path, CORE::IOAccessPtr& stream, CString& error )
{GUCEF_TRACE;

    // the paths of the dialog are paths of the native file system access, which resolves them the same way
    const CString temporary = GetTemporaryPath( CORE::RelativePath( path ) );
    CORE::CFileAccess* file = GUCEF_NEW CORE::CFileAccess( temporary, "wb" );
    stream = CORE::IOAccessPtr( file );
    if ( !file->IsValid() || !file->IsWriteable() )
    {
        stream.Unlink();
        CORE::DeleteFile( temporary );
        error = "\"" + temporary + "\" cannot be made";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNativeGridExportTarget::Commit( const CString& path, CORE::IOAccessPtr& stream, CString& error )
{GUCEF_TRACE;

    const CString destination = CORE::RelativePath( path );
    const CString temporary = GetTemporaryPath( destination );
    bool synced = true;
    if ( !stream.IsNULL() )
    {
        CORE::CFileAccess* file = static_cast< CORE::CFileAccess* >( stream.GetPointerAlways() );
        synced = file->Sync();
        file->Close();
        stream.Unlink();
    }
    if ( !synced )
    {
        CORE::DeleteFile( temporary );
        error = "\"" + temporary + "\" could not be written to storage";
        return false;
    }
    if ( !CORE::MoveFileAtomically( temporary, destination, true ) )
    {
        CORE::DeleteFile( temporary );
        error = "\"" + destination + "\" could not be replaced";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNativeGridExportTarget::Abandon( const CString& path, CORE::IOAccessPtr& stream )
{GUCEF_TRACE;

    if ( !stream.IsNULL() )
    {
        stream->Close();
        stream.Unlink();
    }
    CORE::DeleteFile( GetTemporaryPath( CORE::RelativePath( path ) ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
