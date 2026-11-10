/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CNativeFileSystemAccess.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #undef MoveFile
  #undef DeleteFile
  #undef CopyFile
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CNativeFileSystemAccess::DefaultDisplayName = "Native file system";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The path as the operating system takes it: variables resolved, no separator at the end (except for a root) */
CString
ResolvePath( const CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return path;
    CString resolved = RelativePath( path ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR );
    while ( resolved.Length() > 1 && GUCEF_DIRSEPCHAR == resolved[ resolved.Length()-1 ] )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        if ( 3 == resolved.Length() && ':' == resolved[ 1 ] )
            break;    // "C:\" is a root
        #endif
        resolved = resolved.CutChars( 1, false );
    }
    return resolved;
}

/*-------------------------------------------------------------------------*/

/** The directory a resolved path is in */
CString
ParentOf( const CString& resolvedPath )
{GUCEF_TRACE;

    return StripFilename( resolvedPath );
}

/*-------------------------------------------------------------------------*/

bool
IsSamePath( const CString& resolvedA, const CString& resolvedB )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return resolvedA.Equals( resolvedB, false );
    #else
    return resolvedA == resolvedB;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Whether a resolved path is inside a resolved directory (not the directory itself) */
bool
IsInside( const CString& resolvedDir, const CString& resolvedPath )
{GUCEF_TRACE;

    CString prefix = resolvedDir;
    if ( prefix.IsNULLOrEmpty() || GUCEF_DIRSEPCHAR != prefix[ prefix.Length()-1 ] )
        prefix += GUCEF_DIRSEPCHAR;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return resolvedPath.Lowercase().StartsWith( prefix.Lowercase() );
    #else
    return resolvedPath.StartsWith( prefix );
    #endif
}

/*-------------------------------------------------------------------------*/

bool
HasEntries( const CString& resolvedDir )
{GUCEF_TRACE;

    CFileSystemIterator iterator;
    const bool hasEntries = iterator.FindFirst( resolvedDir );
    iterator.FindClose();
    return hasEntries;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNativeFileSystemAccess::CNativeFileSystemAccess( const CString& displayName )
    : IFileSystemAccess()
    , m_displayName( displayName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNativeFileSystemAccess::~CNativeFileSystemAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CNativeFileSystemAccess::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CString
CNativeFileSystemAccess::GetRootPath( void ) const
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return CString::Empty;      // the drives
    #else
    return "/";
    #endif
}

/*-------------------------------------------------------------------------*/

bool
CNativeFileSystemAccess::IsValidEntryName( const CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || "." == name || ".." == name )
        return false;
    return name.HasChar( '/' ) < 0 && name.HasChar( '\\' ) < 0;
}

/*-------------------------------------------------------------------------*/

bool
CNativeFileSystemAccess::IsReadOnlyDir( const CString& dirPath )
{GUCEF_TRACE;

    CResourceMetaData metaData;
    return GetFileMetaData( dirPath, metaData ) && metaData.hasIsReadOnly && metaData.isReadOnly;
}

/*-------------------------------------------------------------------------*/

bool
CNativeFileSystemAccess::IsWriteable( const CString& path ) const
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return false;
    const CString dir = DirExists( resolved ) ? resolved : ParentOf( resolved );
    return DirExists( dir ) && !IsReadOnlyDir( dir );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryCreateDirectory( const CString& path, bool createParents )
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( DirExists( resolved ) || FileExists( resolved ) )
        return FILESYSTEMACCESS_ALREADY_EXISTS;
    const CString parent = ParentOf( resolved );
    if ( !createParents && !parent.IsNULLOrEmpty() && !DirExists( parent ) )
        return FileExists( parent ) ? FILESYSTEMACCESS_NOT_A_DIRECTORY : FILESYSTEMACCESS_PARENT_NOT_FOUND;
    if ( DirExists( parent ) && IsReadOnlyDir( parent ) )
        return FILESYSTEMACCESS_ACCESS_DENIED;
    return CreateDirs( resolved ) && DirExists( resolved ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryDeleteDirectory( const CString& path, bool deleteContents )
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( !DirExists( resolved ) )
        return FileExists( resolved ) ? FILESYSTEMACCESS_NOT_A_DIRECTORY : FILESYSTEMACCESS_NOT_FOUND;
    if ( !deleteContents && HasEntries( resolved ) )
        return FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY;
    if ( IsReadOnlyDir( ParentOf( resolved ) ) )
        return FILESYSTEMACCESS_ACCESS_DENIED;
    return DeleteDir( resolved, deleteContents ) && !DirExists( resolved ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryMoveDirectory( const CString& oldPath, const CString& newPath )
{GUCEF_TRACE;

    const CString from = ResolvePath( oldPath );
    const CString to = ResolvePath( newPath );
    if ( from.IsNULLOrEmpty() || to.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( !DirExists( from ) )
        return FileExists( from ) ? FILESYSTEMACCESS_NOT_A_DIRECTORY : FILESYSTEMACCESS_NOT_FOUND;
    if ( IsSamePath( from, to ) )
        return FILESYSTEMACCESS_SAME_PATH;
    if ( IsInside( from, to ) )
        return FILESYSTEMACCESS_INVALID_PATH;      // into itself
    if ( DirExists( to ) || FileExists( to ) )
        return FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( IsReadOnlyDir( ParentOf( from ) ) || IsReadOnlyDir( ParentOf( to ) ) )
        return FILESYSTEMACCESS_ACCESS_DENIED;
    return MoveFile( from, to, false ) && DirExists( to ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryMoveFile( const CString& oldPath, const CString& newPath, bool overwrite )
{GUCEF_TRACE;

    const CString from = ResolvePath( oldPath );
    const CString to = ResolvePath( newPath );
    if ( from.IsNULLOrEmpty() || to.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( !FileExists( from ) )
        return DirExists( from ) ? FILESYSTEMACCESS_NOT_A_FILE : FILESYSTEMACCESS_NOT_FOUND;
    if ( IsSamePath( from, to ) )
        return FILESYSTEMACCESS_SAME_PATH;
    if ( DirExists( to ) )
        return FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( FileExists( to ) && !overwrite )
        return FILESYSTEMACCESS_ALREADY_EXISTS;
    if ( IsReadOnlyDir( ParentOf( from ) ) || IsReadOnlyDir( ParentOf( to ) ) )
        return FILESYSTEMACCESS_ACCESS_DENIED;
    return MoveFile( from, to, overwrite ) && FileExists( to ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryMoveFiles( const CString::StringVector& filePaths     ,
                                       const CString& destinationDir             ,
                                       bool overwrite                            ,
                                       TFileSystemAccessResultVector* perFileResults )
{GUCEF_TRACE;

    if ( GUCEF_NULL != perFileResults )
        perFileResults->clear();

    const CString destination = ResolvePath( destinationDir );
    TFileSystemAccessResult destinationResult = FILESYSTEMACCESS_SUCCESS;
    if ( destination.IsNULLOrEmpty() )
        destinationResult = FILESYSTEMACCESS_INVALID_PATH;
    else
    if ( !DirExists( destination ) )
        destinationResult = FileExists( destination ) ? FILESYSTEMACCESS_NOT_A_DIRECTORY : FILESYSTEMACCESS_NOT_FOUND;

    UInt32 moved = 0;
    TFileSystemAccessResult firstFailure = FILESYSTEMACCESS_SUCCESS;
    CString::StringVector::const_iterator i = filePaths.begin();
    while ( i != filePaths.end() )
    {
        TFileSystemAccessResult result = destinationResult;
        if ( FILESYSTEMACCESS_SUCCESS == result )
            result = TryMoveFile( (*i), CombinePath( destination, ExtractFilename( ResolvePath( (*i) ) ) ), overwrite );
        if ( FILESYSTEMACCESS_SUCCESS == result )
            ++moved;
        else
        if ( FILESYSTEMACCESS_SUCCESS == firstFailure )
            firstFailure = result;
        if ( GUCEF_NULL != perFileResults )
            perFileResults->push_back( result );
        ++i;
    }

    if ( filePaths.empty() )
        return destinationResult;
    if ( moved == filePaths.size() )
        return FILESYSTEMACCESS_SUCCESS;
    return 0 == moved ? firstFailure : FILESYSTEMACCESS_PARTIAL_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryRenameFile( const CString& path, const CString& newName )
{GUCEF_TRACE;

    if ( !IsValidEntryName( newName ) )
        return FILESYSTEMACCESS_INVALID_NAME;
    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    return TryMoveFile( resolved, CombinePath( ParentOf( resolved ), newName ), false );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryDeleteFile( const CString& path )
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( !FileExists( resolved ) )
        return DirExists( resolved ) ? FILESYSTEMACCESS_NOT_A_FILE : FILESYSTEMACCESS_NOT_FOUND;
    if ( IsReadOnlyDir( ParentOf( resolved ) ) )
        return FILESYSTEMACCESS_ACCESS_DENIED;
    return DeleteFile( resolved ) && !FileExists( resolved ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryGetMetaData( const CString& path, CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    if ( resolved.IsNULLOrEmpty() )
        return FILESYSTEMACCESS_INVALID_PATH;
    if ( !FileExists( resolved ) && !DirExists( resolved ) )
        return FILESYSTEMACCESS_NOT_FOUND;
    return GetFileMetaData( resolved, metaData ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryGetEntryType( const CString& path, TFileSystemEntryType& entryType ) const
{GUCEF_TRACE;

    const CString resolved = ResolvePath( path );
    entryType = FILESYSTEMENTRYTYPE_NONE;
    if ( resolved.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        entryType = FILESYSTEMENTRYTYPE_DIRECTORY;     // the root of the drives
        return FILESYSTEMACCESS_SUCCESS;
        #else
        return FILESYSTEMACCESS_INVALID_PATH;
        #endif
    }
    if ( DirExists( resolved ) )
        entryType = FILESYSTEMENTRYTYPE_DIRECTORY;
    else
    if ( FileExists( resolved ) )
        entryType = FILESYSTEMENTRYTYPE_FILE;
    return FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryGetFileSystemIterator( const CString& dirPath, TFileSystemIteratorPtr& iterator ) const
{GUCEF_TRACE;

    const CString resolved = ResolvePath( dirPath );
    if ( resolved.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        // the root of the drives
        CFileSystemEntryListIterator::TEntryVector drives;
        const DWORD driveMask = ::GetLogicalDrives();
        for ( char letter='A'; letter<='Z'; ++letter )
        {
            if ( 0 != ( driveMask & ( 1u << ( letter - 'A' ) ) ) )
            {
                CFileSystemEntryListIterator::CEntry drive;
                drive.name = CString( letter ) + ":";
                drive.path = drive.name + "\\";
                drive.isDirectory = true;
                drives.push_back( drive );
            }
        }
        iterator = TFileSystemIteratorPtr( GUCEF_NEW CFileSystemEntryListIterator( CString::Empty, drives ) );
        return FILESYSTEMACCESS_SUCCESS;

        #else

        return FILESYSTEMACCESS_INVALID_PATH;

        #endif
    }
    if ( !DirExists( resolved ) )
        return FileExists( resolved ) ? FILESYSTEMACCESS_NOT_A_DIRECTORY : FILESYSTEMACCESS_NOT_FOUND;
    iterator = TFileSystemIteratorPtr( GUCEF_NEW CFileSystemIterator() );
    return FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::ListAvailableSpecialDirs( TSpecialDirSet& specialDirs ) const
{GUCEF_TRACE;

    return CORE::ListAvailableSpecialDirs( specialDirs ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
CNativeFileSystemAccess::TryResolveSpecialDir( TSpecialDirs specialDir, CString& path ) const
{GUCEF_TRACE;

    TSpecialDirSet available;
    CORE::ListAvailableSpecialDirs( available );
    if ( available.find( specialDir ) == available.end() )
        return FILESYSTEMACCESS_NOT_SUPPORTED;
    return CORE::TryResolveSpecialDir( specialDir, path ) ? FILESYSTEMACCESS_SUCCESS : FILESYSTEMACCESS_NOT_FOUND;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
