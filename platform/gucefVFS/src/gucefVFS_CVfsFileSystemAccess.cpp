/*
 *  gucefVFS: GUCEF module providing a Virtual File System
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefVFS_CVfsFileSystemAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsFileSystemAccess::DefaultDisplayName = "Virtual file system";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A conformed path as a directory path of the VFS, with '/' at the end */
CORE::CString
AsDirPath( const CORE::CString& conformedPath )
{GUCEF_TRACE;

    return "/" == conformedPath ? conformedPath : conformedPath + "/";
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinPath( const CORE::CString& conformedDir, const CORE::CString& name )
{GUCEF_TRACE;

    return AsDirPath( conformedDir ) + name;
}

/*-------------------------------------------------------------------------*/

/** A name of a listing without the '/' the file system archive puts after directories */
CORE::CString
StripTrailingSlash( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString result = name;
    while ( result.Length() > 0 && '/' == result[ result.Length()-1 ] )
        result = result.CutChars( 1, false );
    return result;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsFileSystemAccess::CVfsFileSystemAccess( const CORE::CString& displayName )
    : CORE::IFileSystemAccess()
    , m_vfs( &CVfsGlobal::Instance()->GetVfs() )
    , m_displayName( displayName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFileSystemAccess::CVfsFileSystemAccess( CVFS& vfs, const CORE::CString& displayName )
    : CORE::IFileSystemAccess()
    , m_vfs( &vfs )
    , m_displayName( displayName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFileSystemAccess::~CVfsFileSystemAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFileSystemAccess::ConformPath( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString conformed = path.ReplaceChar( '\\', '/' ).CompactRepeatingChar( '/' );
    if ( conformed.IsNULLOrEmpty() || '/' != conformed[ 0 ] )
        conformed = "/" + conformed;
    while ( conformed.Length() > 1 && '/' == conformed[ conformed.Length()-1 ] )
        conformed = conformed.CutChars( 1, false );
    return conformed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFileSystemAccess::ParentOf( const CORE::CString& conformedPath )
{GUCEF_TRACE;

    const Int32 lastSlash = conformedPath.HasChar( '/', false );
    if ( lastSlash <= 0 )
        return "/";
    return conformedPath.SubstrToIndex( (UInt32) lastSlash, true );
}

/*-------------------------------------------------------------------------*/

bool
CVfsFileSystemAccess::IsDirectory( const CORE::CString& conformedPath ) const
{GUCEF_TRACE;

    if ( "/" == conformedPath || m_vfs->DirExists( AsDirPath( conformedPath ) ) )
        return true;

    // a directory that holds mount points
    CVFS::TStringSet mountPaths;
    m_vfs->GetMountedArchiveList( AsDirPath( conformedPath ), CORE::CString::Empty, true, mountPaths );
    return !mountPaths.empty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFileSystemAccess::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFileSystemAccess::GetRootPath( void ) const
{GUCEF_TRACE;

    return "/";
}

/*-------------------------------------------------------------------------*/

bool
CVfsFileSystemAccess::IsWriteable( const CORE::CString& path ) const
{GUCEF_TRACE;

    return m_vfs->IsWriteable( ConformPath( path ) );
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryCreateDirectory( const CORE::CString& path, bool createParents )
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( path );
    if ( "/" == conformed )
        return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( IsDirectory( conformed ) || m_vfs->FileExists( conformed ) )
        return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
    const CORE::CString parent = ParentOf( conformed );
    if ( !createParents && !IsDirectory( parent ) )
        return m_vfs->FileExists( parent ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND;
    if ( !m_vfs->IsWriteable( conformed ) )
        return CORE::FILESYSTEMACCESS_READ_ONLY;
    return m_vfs->CreateDir( conformed ) && IsDirectory( conformed ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryDeleteDirectory( const CORE::CString& path, bool deleteContents )
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( path );
    if ( !IsDirectory( conformed ) )
        return m_vfs->FileExists( conformed ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;
    if ( "/" == conformed || !m_vfs->IsWriteable( conformed ) )
        return CORE::FILESYSTEMACCESS_READ_ONLY;
    if ( !deleteContents )
    {
        CORE::TFileSystemIteratorPtr listing;
        if ( CORE::FILESYSTEMACCESS_SUCCESS == TryGetFileSystemIterator( conformed, listing ) && listing->FindFirst( conformed ) )
            return CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY;
    }
    return m_vfs->DeleteDir( conformed, deleteContents ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath )
{GUCEF_TRACE;

    const CORE::CString from = ConformPath( oldPath );
    const CORE::CString to = ConformPath( newPath );
    if ( !IsDirectory( from ) )
        return m_vfs->FileExists( from ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;
    if ( from == to )
        return CORE::FILESYSTEMACCESS_SAME_PATH;
    if ( to.StartsWith( AsDirPath( from ) ) )
        return CORE::FILESYSTEMACCESS_INVALID_PATH;      // into itself
    if ( IsDirectory( to ) || m_vfs->FileExists( to ) )
        return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( "/" == from || !m_vfs->IsWriteable( from ) || !m_vfs->IsWriteable( to ) )
        return CORE::FILESYSTEMACCESS_READ_ONLY;
    return m_vfs->MoveDir( from, to ) && IsDirectory( to ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite )
{GUCEF_TRACE;

    const CORE::CString from = ConformPath( oldPath );
    const CORE::CString to = ConformPath( newPath );
    if ( !m_vfs->FileExists( from ) )
        return IsDirectory( from ) ? CORE::FILESYSTEMACCESS_NOT_A_FILE : CORE::FILESYSTEMACCESS_NOT_FOUND;
    if ( from == to )
        return CORE::FILESYSTEMACCESS_SAME_PATH;
    if ( IsDirectory( to ) )
        return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( m_vfs->FileExists( to ) && !overwrite )
        return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( !m_vfs->IsWriteable( to ) || !m_vfs->IsWriteable( from ) )
        return CORE::FILESYSTEMACCESS_READ_ONLY;
    return m_vfs->MoveFile( from, to, overwrite ) && m_vfs->FileExists( to ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryMoveFiles( const CORE::CString::StringVector& filePaths ,
                                    const CORE::CString& destinationDir         ,
                                    bool overwrite                              ,
                                    TFileSystemAccessResultVector* perFileResults )
{GUCEF_TRACE;

    if ( GUCEF_NULL != perFileResults )
        perFileResults->clear();

    const CORE::CString destination = ConformPath( destinationDir );
    CORE::TFileSystemAccessResult destinationResult = CORE::FILESYSTEMACCESS_SUCCESS;
    if ( !IsDirectory( destination ) )
        destinationResult = m_vfs->FileExists( destination ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;

    UInt32 moved = 0;
    CORE::TFileSystemAccessResult firstFailure = CORE::FILESYSTEMACCESS_SUCCESS;
    CORE::CString::StringVector::const_iterator i = filePaths.begin();
    while ( i != filePaths.end() )
    {
        CORE::TFileSystemAccessResult result = destinationResult;
        if ( CORE::FILESYSTEMACCESS_SUCCESS == result )
        {
            const CORE::CString from = ConformPath( (*i) );
            result = TryMoveFile( from, JoinPath( destination, CORE::ExtractFilename( from ) ), overwrite );
        }
        if ( CORE::FILESYSTEMACCESS_SUCCESS == result )
            ++moved;
        else
        if ( CORE::FILESYSTEMACCESS_SUCCESS == firstFailure )
            firstFailure = result;
        if ( GUCEF_NULL != perFileResults )
            perFileResults->push_back( result );
        ++i;
    }

    if ( filePaths.empty() )
        return destinationResult;
    if ( moved == filePaths.size() )
        return CORE::FILESYSTEMACCESS_SUCCESS;
    return 0 == moved ? firstFailure : CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryRenameFile( const CORE::CString& path, const CORE::CString& newName )
{GUCEF_TRACE;

    if ( !CORE::CNativeFileSystemAccess::IsValidEntryName( newName ) )
        return CORE::FILESYSTEMACCESS_INVALID_NAME;
    const CORE::CString conformed = ConformPath( path );
    return TryMoveFile( conformed, JoinPath( ParentOf( conformed ), newName ), false );
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryDeleteFile( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( path );
    if ( !m_vfs->FileExists( conformed ) )
        return IsDirectory( conformed ) ? CORE::FILESYSTEMACCESS_NOT_A_FILE : CORE::FILESYSTEMACCESS_NOT_FOUND;
    if ( !m_vfs->IsWriteable( conformed ) )
        return CORE::FILESYSTEMACCESS_READ_ONLY;
    return m_vfs->DeleteFile( conformed, false ) && !m_vfs->FileExists( conformed ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( path );
    if ( m_vfs->FileExists( conformed ) )
        return m_vfs->GetFileMetaData( conformed, metaData ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
    if ( IsDirectory( conformed ) )
    {
        // the VFS keeps no metadata of directories
        metaData.resourceExists = true;
        metaData.name = CORE::ExtractFilename( conformed );
        metaData.hasName = true;
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }
    return CORE::FILESYSTEMACCESS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( path );
    entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    if ( IsDirectory( conformed ) )
        entryType = CORE::FILESYSTEMENTRYTYPE_DIRECTORY;
    else
    if ( m_vfs->FileExists( conformed ) )
        entryType = CORE::FILESYSTEMENTRYTYPE_FILE;
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const
{GUCEF_TRACE;

    const CORE::CString conformed = ConformPath( dirPath );
    if ( !IsDirectory( conformed ) )
        return m_vfs->FileExists( conformed ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;
    const CORE::CString location = AsDirPath( conformed );

    // the directories and files of every mount that covers the directory, and the mount points just below it, once each
    CORE::CFileSystemEntryListIterator::TEntryVector entries;
    CORE::CString::StringSet seen;

    CVFS::TStringVector dirs;
    m_vfs->GetDirList( dirs, location, false, false, CORE::CString::EmptyStringSet );
    CVFS::TStringVector::const_iterator d = dirs.begin();
    while ( d != dirs.end() )
    {
        const CORE::CString name = CORE::ExtractFilename( StripTrailingSlash( (*d) ) );
        if ( !name.IsNULLOrEmpty() && seen.insert( name ).second )
        {
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = name;
            entry.path = JoinPath( conformed, name );
            entry.isDirectory = true;
            entries.push_back( entry );
        }
        ++d;
    }

    CVFS::TStringSet mountPaths;
    m_vfs->GetMountedArchiveList( location, CORE::CString::Empty, true, mountPaths );
    CVFS::TStringSet::const_iterator m = mountPaths.begin();
    while ( m != mountPaths.end() )
    {
        // the directory right below the location on the way to the mount point
        const CORE::CString below = ConformPath( (*m) );
        if ( below.Length() > location.Length() && below.StartsWith( location ) )
        {
            CORE::CString name = below.CutChars( location.Length(), true );
            const Int32 slash = name.HasChar( '/' );
            if ( slash > 0 )
                name = name.SubstrToIndex( (UInt32) slash, true );
            if ( !name.IsNULLOrEmpty() && seen.insert( name ).second )
            {
                CORE::CFileSystemEntryListIterator::CEntry entry;
                entry.name = name;
                entry.path = JoinPath( conformed, name );
                entry.isDirectory = true;
                entries.push_back( entry );
            }
        }
        ++m;
    }

    CVFS::TStringVector files;
    m_vfs->GetFileList( files, location, false, false, CORE::CString::EmptyStringSet );
    CVFS::TStringVector::const_iterator f = files.begin();
    while ( f != files.end() )
    {
        const CORE::CString name = CORE::ExtractFilename( (*f) );
        if ( !name.IsNULLOrEmpty() && seen.insert( name ).second )
        {
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = name;
            entry.path = JoinPath( conformed, name );
            entry.isFile = true;
            entry.hasMetaData = m_vfs->GetFileMetaData( entry.path, entry.metaData );
            entries.push_back( entry );
        }
        ++f;
    }

    iterator = CORE::TFileSystemIteratorPtr( GUCEF_NEW CORE::CFileSystemEntryListIterator( conformed, entries ) );
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const
{GUCEF_TRACE;

    return m_vfs->ListAvailableSpecialDirs( specialDirs ) ? CORE::FILESYSTEMACCESS_SUCCESS : CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CVfsFileSystemAccess::TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const
{GUCEF_TRACE;

    CORE::CString vfsPath;
    if ( m_vfs->TryResolveSpecialDirToVfsPath( specialDir, vfsPath ) )
    {
        path = ConformPath( vfsPath );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }
    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
