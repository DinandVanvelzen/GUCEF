/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestArchiveUtils.h"
#include "TestFileSystemAccess.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A fresh, empty directory of the file system for a test */
CORE::CString
MakeTestDir( const char* name )
{
    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_FileSystemAccess" ), name );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** The names an iterator gives, with a '/' after directories */
CORE::CString::StringVector
ListNames( CORE::IFileSystemIterator& iterator, const CORE::CString& dir )
{
    CORE::CString::StringVector names;
    if ( iterator.FindFirst( dir ) )
    {
        do
        {
            names.push_back( iterator.GetResourceName() + ( iterator.IsADirectory() ? "/" : "" ) );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
PerformVfsFixesTests( void )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: MoveFileAsync and CopyFileAsync move and copy the file (their task keeps its own copy of the request)" )
        try
        {
            const CORE::CString dir = MakeTestDir( "asyncmove" );
            ASSERT_TRUE( vfs.AddRoot( dir, "FsaAsyncMove", true, false, "fsaasyncmove" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "a.txt" ), "abc" ) );
            CORE::CFutureResult future = vfs.MoveFileAsync( "fsaasyncmove/a.txt", "fsaasyncmove/b.txt", false );
            future.Await( 10000 );
            ASSERT_TRUE( future.IsReady() );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, "b.txt" ) ) && !CORE::FileExists( CORE::CombinePath( dir, "a.txt" ) ) );
            CORE::CFutureResult copied = vfs.CopyFileAsync( "fsaasyncmove/b.txt", "fsaasyncmove/c.txt", false );
            copied.Await( 10000 );
            ASSERT_TRUE( copied.IsReady() );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, "b.txt" ) ) && 3 == CORE::FileSize( CORE::CombinePath( dir, "c.txt" ) ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "FsaAsyncMove" ) );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: Listing the mounted archives of a directory without recursion gives the mounts right below it" )
        try
        {
            const CORE::CString dir = MakeTestDir( "nestedmount" );
            ASSERT_TRUE( vfs.AddRoot( dir, "FsaNestedMount", false, false, "fsanested/inner" ) );
            VFS::CVFS::TStringSet mounts;
            vfs.GetMountedArchiveList( "/fsanested/", CORE::CString::Empty, false, mounts );
            ASSERT_TRUE( mounts.find( "/fsanested/inner/" ) != mounts.end() );
            mounts.clear();
            vfs.GetMountedArchiveList( "/", CORE::CString::Empty, false, mounts );
            ASSERT_TRUE( mounts.find( "/fsanested/inner/" ) == mounts.end() );     // two levels below the root
            ASSERT_TRUE( vfs.UnmountArchiveByName( "FsaNestedMount" ) );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformVfsDirectoryTests( void )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: The VFS makes, moves and deletes directories in writeable mounts only" )
        try
        {
            const CORE::CString rwDir = MakeTestDir( "rw" );
            const CORE::CString roDir = MakeTestDir( "ro" );
            ASSERT_TRUE( vfs.AddRoot( rwDir, "FsaDirRw", true, false, "fsadirrw" ) );
            ASSERT_TRUE( vfs.AddRoot( roDir, "FsaDirRo", false, false, "fsadirro" ) );

            ASSERT_TRUE( vfs.IsWriteable( "fsadirrw/x" ) && !vfs.IsWriteable( "fsadirro/x" ) );
            ASSERT_TRUE( vfs.CreateDir( "fsadirrw/x/y" ) && CORE::DirExists( CORE::CombinePath( rwDir, "x/y" ) ) );
            ASSERT_FALSE( vfs.CreateDir( "fsadirro/x" ) || CORE::DirExists( CORE::CombinePath( roDir, "x" ) ) );
            ASSERT_TRUE( vfs.DirExists( "fsadirrw/x/y" ) );

            ASSERT_TRUE( vfs.MoveDir( "fsadirrw/x", "fsadirrw/z" ) );
            ASSERT_TRUE( CORE::DirExists( CORE::CombinePath( rwDir, "z/y" ) ) && !CORE::DirExists( CORE::CombinePath( rwDir, "x" ) ) );
            ASSERT_FALSE( vfs.MoveDir( "fsadirrw/z", "fsadirro/z" ) );          // no copy between archives

            ASSERT_FALSE( vfs.DeleteDir( "fsadirrw/z", false ) );               // not empty
            ASSERT_TRUE( vfs.DeleteDir( "fsadirrw/z", true ) && !CORE::DirExists( CORE::CombinePath( rwDir, "z" ) ) );
            ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( roDir, "keep" ) ) );
            ASSERT_FALSE( vfs.DeleteDir( "fsadirro/keep", true ) || !CORE::DirExists( CORE::CombinePath( roDir, "keep" ) ) );

            // A path that leaves the archive deletes nothing outside it
            const CORE::CString sibling = MakeTestDir( "sibling" );
            const CORE::CString siblingFile = CORE::CombinePath( sibling, "keep.txt" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( siblingFile, "keep" ) );
            ASSERT_FALSE( vfs.DeleteDir( "fsadirrw/../sibling", true ) );
            ASSERT_FALSE( vfs.DeleteDir( "fsadirrw/x/../../sibling", true ) );
            ASSERT_FALSE( vfs.DeleteDir( "fsadirrw/..", true ) );
            ASSERT_TRUE( CORE::FileExists( siblingFile ) && CORE::DirExists( rwDir ) );
            CORE::DeleteDir( sibling, true );

            // the special directory stubs
            CORE::TSpecialDirSet specialDirs;
            specialDirs.insert( CORE::SPECIALDIR_HOME_DIR );
            CORE::CString path;
            ASSERT_TRUE( vfs.ListAvailableSpecialDirs( specialDirs ) && specialDirs.empty() );
            ASSERT_FALSE( vfs.TryResolveSpecialDirToVfsPath( CORE::SPECIALDIR_HOME_DIR, path ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "FsaDirRw" ) && vfs.UnmountArchiveByName( "FsaDirRo" ) );
            CORE::DeleteDir( rwDir, true );
            CORE::DeleteDir( roDir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformVfsFileSystemAccessTests( void )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: The VFS as a file system: listing with mounts merged, operations, read-only places" )
        try
        {
            const CORE::CString rwDir = MakeTestDir( "fsarw" );
            const CORE::CString roDir = MakeTestDir( "fsaro" );
            const CORE::CString overlayDir = MakeTestDir( "fsaoverlay" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( rwDir, "same.txt" ), "first" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( overlayDir, "same.txt" ), "second" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( overlayDir, "extra.txt" ), "extra" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( roDir, "locked.txt" ), "locked" ) );
            ASSERT_TRUE( vfs.AddRoot( rwDir, "FsaRw", true, false, "fsa/rw" ) );
            ASSERT_TRUE( vfs.AddRoot( overlayDir, "FsaOverlay", false, false, "fsa/rw" ) );
            ASSERT_TRUE( vfs.AddRoot( roDir, "FsaRo", false, false, "fsa/ro" ) );

            CORE::TFileSystemAccessPtr access = ( GUCEF_NEW VFS::CVfsFileSystemAccess() )->CreateSharedPtr();
            ASSERT_TRUE( "/" == access->GetRootPath() && VFS::CVfsFileSystemAccess::DefaultDisplayName == access->GetDisplayName() );
            ASSERT_TRUE( "/a/b" == VFS::CVfsFileSystemAccess::ConformPath( "a\\b/" ) && "/" == VFS::CVfsFileSystemAccess::ConformPath( "" ) );

            // the root and "/fsa" exist because of the mounts below them
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/fsa", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
            CORE::TFileSystemIteratorPtr iterator;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/", iterator ) );
            ASSERT_FALSE( iterator.IsNULL() );
            ASSERT_TRUE( Contains( ListNames( *iterator, "/" ), "fsa/" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/fsa", iterator ) );
            CORE::CString::StringVector names = ListNames( *iterator, "/fsa" );
            ASSERT_TRUE( Contains( names, "rw/" ) && Contains( names, "ro/" ) && 2 == names.size() );

            // two archives on one mount point: merged, each name once
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/fsa/rw/", iterator ) );
            names = ListNames( *iterator, "/fsa/rw" );
            ASSERT_TRUE( 2 == names.size() && Contains( names, "same.txt" ) && Contains( names, "extra.txt" ) );
            ASSERT_TRUE( iterator->FindFirst( "/fsa/rw" ) );
            while ( "extra.txt" != iterator->GetResourceName() && iterator->FindNext() ) {}
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( "/fsa/rw/extra.txt" == iterator->GetResourcePath() && iterator->TryReadMetaData( metaData ) && 5 == metaData.resourceSizeInBytes );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetFileSystemIterator( "/fsa/none", iterator ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY == access->TryGetFileSystemIterator( "/fsa/rw/same.txt", iterator ) );

            // writing where a writeable mount is, refused where only read-only ones are
            ASSERT_TRUE( access->IsWriteable( "/fsa/rw" ) && !access->IsWriteable( "/fsa/ro" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectory( "/fsa/rw/new", false ) && CORE::DirExists( CORE::CombinePath( rwDir, "new" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ALREADY_EXISTS == access->TryCreateDirectory( "/fsa/rw/new", false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND == access->TryCreateDirectory( "/fsa/rw/p/q", false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectory( "/fsa/rw/p/q", true ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryCreateDirectory( "/fsa/ro/new", false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryDeleteFile( "/fsa/ro/locked.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == access->TryRenameFile( "/fsa/ro/locked.txt", "open.txt" ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( roDir, "locked.txt" ) ) );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_NAME == access->TryRenameFile( "/fsa/rw/same.txt", "a/b" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryRenameFile( "/fsa/rw/same.txt", "renamed.txt" ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( rwDir, "renamed.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveFile( "/fsa/rw/renamed.txt", "/fsa/rw/new/moved.txt", false ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( rwDir, "new/moved.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SAME_PATH == access->TryMoveFile( "/fsa/rw/new/moved.txt", "/fsa/rw/new/moved.txt", true ) );

            CORE::CString::StringVector files;
            files.push_back( "/fsa/rw/new/moved.txt" );
            files.push_back( "/fsa/rw/none.txt" );
            CORE::IFileSystemAccess::TFileSystemAccessResultVector perFile;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS == access->TryMoveFiles( files, "/fsa/rw/p", false, &perFile ) );
            ASSERT_TRUE( 2 == perFile.size() && CORE::FILESYSTEMACCESS_SUCCESS == perFile[ 0 ] && CORE::FILESYSTEMACCESS_NOT_FOUND == perFile[ 1 ] );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveDirectory( "/fsa/rw/p", "/fsa/rw/p2" ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( rwDir, "p2/moved.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_PATH == access->TryMoveDirectory( "/fsa/rw/p2", "/fsa/rw/p2/inside" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY == access->TryDeleteDirectory( "/fsa/rw/p2", false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteFile( "/fsa/rw/p2/moved.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteDirectory( "/fsa/rw/p2", true ) && !CORE::DirExists( CORE::CombinePath( rwDir, "p2" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryDeleteDirectory( "/fsa/rw/p2", true ) );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetMetaData( "/fsa/ro/locked.txt", metaData ) && 6 == metaData.resourceSizeInBytes );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetMetaData( "/fsa/ro/none.txt", metaData ) );

            // the VFS has no special directories of its own
            CORE::TSpecialDirSet specialDirs;
            CORE::CString path;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->ListAvailableSpecialDirs( specialDirs ) && specialDirs.empty() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == access->TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, path ) );

            // async through the default of the interface: the listing is read on a pool thread
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIteratorAsync( "/fsa/rw", future ) );
            future.Await( 10000 );
            CORE::TFileSystemAccessResult result = CORE::FILESYSTEMACCESS_TIMEOUT;
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncResult( future, result ) && CORE::FILESYSTEMACCESS_SUCCESS == result );
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncIterator( future, iterator ) && Contains( ListNames( *iterator, "/fsa/rw" ), "new/" ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "FsaRw" ) && vfs.UnmountArchiveByName( "FsaOverlay" ) && vfs.UnmountArchiveByName( "FsaRo" ) );
            CORE::DeleteDir( rwDir, true );
            CORE::DeleteDir( roDir, true );
            CORE::DeleteDir( overlayDir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformArchiveExtensionTests( void )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: Archive types declare their file extensions, only loaded types are offered, the first declaration wins" )
        try
        {
            if ( vfs.CreateArchiveWriter( "zip" ).IsNULL() )
                TestArchiveUtils::LoadVfsPlugin( "vfspluginZIP" );

            CORE::CString::StringSet extensions;
            vfs.GetArchiveFileExtensions( extensions );
            ASSERT_TRUE( extensions.find( "zip" ) != extensions.end() && extensions.find( "pk3" ) != extensions.end() );
            ASSERT_TRUE( extensions.find( "filesystem" ) == extensions.end() );        // not a file

            CORE::CString type;
            ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "/somewhere/Data.ZIP", type ) && "zip" == type );
            ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "c:\\games\\mod.pk3", type ) && "pk3" == type );
            ASSERT_FALSE( vfs.TryGetArchiveTypeForFile( "/somewhere/notes.txt", type ) );
            ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "x.filesystem", type ) && VFS::CVFS::FileSystemArchiveTypeName == type );   // named like the type

            // a type whose factory is not registered is not offered
            vfs.SetArchiveTypeFileExtensions( "fsanosuchtype", CORE::CString( ".NSX" ).ParseUniqueElements( ';', false ) );
            vfs.GetArchiveFileExtensions( extensions );
            ASSERT_TRUE( extensions.find( "nsx" ) == extensions.end() );
            ASSERT_FALSE( vfs.TryGetArchiveTypeForFile( "a.nsx", type ) );

            // when two types claim an extension the one that declared it first wins
            vfs.SetArchiveTypeFileExtensions( VFS::CVFS::FileSystemArchiveTypeName, CORE::CString( "zip;fsx" ).ParseUniqueElements( ';', false ) );
            ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "a.zip", type ) && "zip" == type );
            ASSERT_TRUE( vfs.TryGetArchiveTypeForFile( "a.fsx", type ) && VFS::CVFS::FileSystemArchiveTypeName == type );
            vfs.SetArchiveTypeFileExtensions( VFS::CVFS::FileSystemArchiveTypeName, CORE::CString::StringSet() );
            ASSERT_FALSE( vfs.TryGetArchiveTypeForFile( "a.fsx", type ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VfsFileSystemAccess: Sub archives are mounted by their extension, from their own file, and go with their archive" )
        try
        {
            if ( vfs.CreateArchiveWriter( "zip" ).IsNULL() )
                TestArchiveUtils::LoadVfsPlugin( "vfspluginZIP" );
            const CORE::CString dir = MakeTestDir( "automount" );
            const CORE::CString zipPath = CORE::CombinePath( dir, "inner.zip" );
            {
                CORE::CFileAccess output( zipPath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                ASSERT_TRUE( !writer.IsNULL() && output.IsValid() );
                if ( !writer.IsNULL() && output.IsValid() )
                {
                    CORE::CDynamicBuffer content;
                    content.CopyFrom( 5, "hello" );
                    CORE::CDynamicBufferAccess contentAccess( content );
                    ASSERT_TRUE( writer->Open( output ) && writer->AddFile( "hello.txt", contentAccess, VFS::CArchiveWriterEntrySettings() ) && writer->Close() );
                }
            }
            ASSERT_TRUE( vfs.AddRoot( dir, "FsaAutoMount", false, true, "fsaauto" ) );
            VFS::CVFS::TStringSet mounts;
            vfs.GetMountedArchiveList( "/", CORE::CString::Empty, true, mounts );
            CORE::CString innerMount;
            VFS::CVFS::TStringSet::const_iterator i = mounts.begin();
            while ( i != mounts.end() )
            {
                if ( (*i).HasSubstr( "inner.zip" ) >= 0 )
                    innerMount = (*i);
                ++i;
            }
            ASSERT_FALSE( innerMount.IsNULLOrEmpty() );
            if ( !innerMount.IsNULLOrEmpty() )
                ASSERT_TRUE( vfs.FileExists( CORE::CombinePath( innerMount, "hello.txt" ) ) );
            // unmounting the archive unmounts the sub archives it mounted
            ASSERT_TRUE( vfs.UnmountArchiveByName( "FsaAutoMount" ) );
            mounts.clear();
            vfs.GetMountedArchiveList( "/", CORE::CString::Empty, true, mounts );
            ASSERT_TRUE( mounts.find( innerMount ) == mounts.end() );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFileSystemAccessTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING VfsFileSystemAccess TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "VfsFileSystemAccess" );

    PerformVfsFixesTests();
    PerformVfsDirectoryTests();
    PerformVfsFileSystemAccessTests();
    PerformArchiveExtensionTests();
}

/*-------------------------------------------------------------------------*/
