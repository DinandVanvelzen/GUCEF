/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/stat.h>
  #include <unistd.h>
  #include <dirent.h>
  #include <string.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "TestAllocationCounter.h"
#include "TestFileUtils.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
/** The number of file descriptors the process has open, counted without the iterator under test */
static UInt32
CountOpenFileDescriptors( void )
{
    UInt32 count = 0;
    DIR* fdDir = ::opendir( "/proc/self/fd" );
    if ( GUCEF_NULL == fdDir )
        return 0;
    while ( GUCEF_NULL != ::readdir( fdDir ) )
        ++count;
    ::closedir( fdDir );
    return count;
}
#endif

void
TestRelativePathFunction( void )
{
    try
    {
        // test that the $CURWORKDIR$ macro gets converted
        CORE::CString relPath = CORE::RelativePath( "$CURWORKDIR$/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( !relPath.StartsWith( "$CURWORKDIR$" ) );
        ASSERT_TRUE( relPath.EndsWith( "test.txt" ) );
        CORE::CString::StringVector elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() > 1 );

        // test that the $MODULEDIR$ macro gets converted
        relPath = CORE::RelativePath( "$MODULEDIR$/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( !relPath.StartsWith( "$MODULEDIR$" ) );
        ASSERT_TRUE( relPath.EndsWith( "test.txt" ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() > 1 );

        // test that the function converts paths to use the O/S native dir separator
        relPath = CORE::RelativePath( "root/dir1/dir2/dir3/test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( relPath == CORE::CString( "root/dir1/dir2/dir3/test.txt" ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() == 5 );

        // test that the function simplifies paths to reduce 'up dir' segments aka ../ if feasible
        relPath = CORE::RelativePath( "root/dir1/../dir3/../test.txt", true );
        ASSERT_TRUE( !relPath.IsNULLOrEmpty() );
        ASSERT_TRUE( relPath == CORE::CString( "root/test.txt" ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
        elements = relPath.ParseElements( GUCEF_DIRSEPCHAR );
        ASSERT_TRUE( elements.size() == 2 );

        // an absolute path stays absolute when its up dir segments are simplified
        relPath = CORE::RelativePath( "/root/dir1/../test.txt", true );
        ASSERT_TRUE( relPath == CORE::CString( "/root/test.txt" ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
        relPath = CORE::RelativePath( "$MODULEDIR$/../test.txt", true );
        ASSERT_TRUE( relPath.StartsWith( CORE::RelativePath( "$MODULEDIR$", true ).SubstrToIndex( 1, true ) ) );
    }
    catch (...)
    {
        ERRORHERE;
    }
}
/*-------------------------------------------------------------------------*/

void
PerformFileUtilsTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING File Utils TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "FileUtils" );

    GUCEF_TESTFW_TESTCASE( "Test: CStorageVolumeInfoOSData gives back the platform data it made" )
    if ( !CORETEST::IsLiveHeapMeasurementAvailable() )
        GUCEF_TESTFW_SKIP( "the live heap cannot be measured in this build" );
    try
    {
        // every object makes its platform data on the heap; 20000 objects that kept it would hold several hundred KiB
        const UInt32 objects = 20000;
        const UInt64 before = CORETEST::GetLiveHeapBytes();
        for ( UInt32 i=0; i<objects; ++i )
        {
            CORE::CStorageVolumeInfoOSData* info = GUCEF_NEW CORE::CStorageVolumeInfoOSData();
            ASSERT_TRUE( GUCEF_NULL != info->GetOsData() );
            GUCEF_DELETE info;
        }
        const UInt64 after = CORETEST::GetLiveHeapBytes();
        ASSERT_TRUE( after < before + 64 * 1024 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: RelativePath function" )
        TestRelativePathFunction();
    GUCEF_TESTFW_TESTCASE_END

    // Regression: StripFilename() returned the input as-is when there was no directory part
    // which caused CFileAccess to create a directory named after the file
    // the per user directories for data and caches follow the XDG base directories on Linux
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    GUCEF_TESTFW_TESTCASE( "Test: The cache and app data dirs follow XDG with its defaults" )
        try
        {
            const char* savedCache = getenv( "XDG_CACHE_HOME" );
            const char* savedData = getenv( "XDG_DATA_HOME" );
            const CORE::CString cacheBackup = GUCEF_NULL != savedCache ? CORE::CString( savedCache ) : CORE::CString();
            const CORE::CString dataBackup = GUCEF_NULL != savedData ? CORE::CString( savedData ) : CORE::CString();
            const CORE::CString home = getenv( "HOME" );

            unsetenv( "XDG_CACHE_HOME" );
            unsetenv( "XDG_DATA_HOME" );
            CORE::CString dir;
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_CACHE_DIR, dir ) && CORE::CombinePath( home, ".cache" ) == dir );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_INTERNET_CACHE_DIR, dir ) && CORE::CombinePath( home, ".cache" ) == dir );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_APPDATA_DIR, dir ) && CORE::CombinePath( home, ".local/share" ) == dir );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_LOCAL_APPDATA_DIR, dir ) && CORE::CombinePath( home, ".local/share" ) == dir );
            ASSERT_TRUE( CORE::ResolveVars( "$CACHEDIR$/game" ) == CORE::CombinePath( home, ".cache" ) + "/game" );
            ASSERT_TRUE( CORE::ResolveVars( "$APPDATADIR$/game" ) == CORE::CombinePath( home, ".local/share" ) + "/game" );

            setenv( "XDG_CACHE_HOME", "/tmp/xdgcache", 1 );
            setenv( "XDG_DATA_HOME", "/tmp/xdgdata", 1 );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_CACHE_DIR, dir ) && "/tmp/xdgcache" == dir );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_APPDATA_DIR, dir ) && "/tmp/xdgdata" == dir );
            ASSERT_TRUE( CORE::ResolveVars( "$CACHEDIR$" ) == "/tmp/xdgcache" );
            ASSERT_TRUE( CORE::ResolveVars( "$APPDATADIR$" ) == "/tmp/xdgdata" );

            if ( GUCEF_NULL != savedCache ) setenv( "XDG_CACHE_HOME", cacheBackup.C_String(), 1 ); else unsetenv( "XDG_CACHE_HOME" );
            if ( GUCEF_NULL != savedData ) setenv( "XDG_DATA_HOME", dataBackup.C_String(), 1 ); else unsetenv( "XDG_DATA_HOME" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: StripFilename edge cases" )
        try
        {
            ASSERT_TRUE( CORE::StripFilename( "file.txt" ).IsNULLOrEmpty() );
            CORE::CString dirSep( GUCEF_DIRSEPCHAR );
            ASSERT_TRUE( CORE::StripFilename( dirSep + "file.txt" ) == dirSep );
            ASSERT_TRUE( CORE::StripFilename( "dir" + dirSep + "file.txt" ) == "dir" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux CreateDirs() failed for absolute paths and looped forever on relative paths with multiple segments
    GUCEF_TESTFW_TESTCASE( "Test: CreateDirs with absolute and multi segment paths" )
        try
        {
            TESTFW::CScopedTestDir scoped_CreateDirs( "gucefCORE_TestApp_CreateDirs" );
            CORE::CString nestedDir = CORE::CombinePath( scoped_CreateDirs.GetPath(), "a/b/c" );
            ASSERT_TRUE( CORE::CreateDirs( nestedDir ) );
            ASSERT_TRUE( CORE::DirExists( nestedDir ) );
            // creating it again must also succeed
            ASSERT_TRUE( CORE::CreateDirs( nestedDir ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: TempDir() had no implementation outside of Windows
    GUCEF_TESTFW_TESTCASE( "Test: TempDir returns an existing directory" )
        try
        {
            CORE::CString tempDir = CORE::TempDir();
            ASSERT_TRUE( !tempDir.IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::DirExists( tempDir ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux CFileSystemIterator changed the process wide working dir and resolved entries
    // relative to it, so concurrent iterators (other threads or interleaved use) misclassified entries
    GUCEF_TESTFW_TESTCASE( "Test: CFileSystemIterator does not depend on the working dir" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_FsIterator( "gucefCORE_TestApp_FsIterator" );
            CORE::CString rootDir = scoped_gucefCORE_TestApp_FsIterator.GetPath();
            CORE::CString dirA = CORE::CombinePath( rootDir, "A" );
            CORE::CString dirB = CORE::CombinePath( rootDir, "B" );
            ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( dirA, "subDirA" ) ) );
            ASSERT_TRUE( CORE::CreateDirs( dirB ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dirA, "fileA1.txt" ), "A1" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dirA, "fileA2.txt" ), "A2" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dirB, "fileB1.txt" ), "B1" ) );

            CORE::CString cwdBefore = CORE::CurrentWorkingDir();

            // Interleave two iterators on different dirs, entries of A must still be classified correctly
            CORE::CFileSystemIterator itA;
            CORE::CFileSystemIterator itB;
            ASSERT_TRUE( itA.FindFirst( dirA ) );
            ASSERT_TRUE( itB.FindFirst( dirB ) );

            UInt32 filesInA = 0;
            UInt32 dirsInA = 0;
            do
            {
                CORE::CString name = itA.GetResourceName();
                ASSERT_TRUE( itA.GetResourcePath() == CORE::CombinePath( dirA, name ) );
                if ( itA.IsAFile() )
                {
                    ASSERT_TRUE( name.StartsWith( "fileA" ) );
                    ++filesInA;
                }
                else
                if ( itA.IsADirectory() )
                {
                    ASSERT_TRUE( name == "subDirA" );
                    ++dirsInA;
                }
                itB.FindNext();
            }
            while ( itA.FindNext() );
            itA.FindClose();
            itB.FindClose();

            ASSERT_TRUE( 2 == filesInA );
            ASSERT_TRUE( 1 == dirsInA );
            ASSERT_TRUE( cwdBefore == CORE::CurrentWorkingDir() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CFileAccess::Sync flushes to storage" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_Durable( "gucefCORE_TestApp_Durable" );
            CORE::CString dir = scoped_gucefCORE_TestApp_Durable.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString path = CORE::CombinePath( dir, "synced.txt" );

            CORE::CFileAccess unopened;
            ASSERT_FALSE( unopened.Sync() );

            {
                CORE::CFileAccess file( path, "wb" );
                ASSERT_TRUE( file.IsValid() );
                ASSERT_TRUE( 5 == file.Write( "hello", 1, 5 ) );
                ASSERT_TRUE( file.Sync() );

                // the data must be readable by others while the file is still open
                CORE::CString content;
                ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) );
                ASSERT_TRUE( content == "hello" );
            }
            CORE::DeleteFile( path );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: WriteFileAtomically creates and replaces files without leftovers" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_Atomic( "gucefCORE_TestApp_Atomic" );
            CORE::CString dir = scoped_gucefCORE_TestApp_Atomic.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString path = CORE::CombinePath( dir, "target.txt" );
            if ( CORE::FileExists( path ) )
                CORE::DeleteFile( path );

            // new file
            ASSERT_TRUE( CORE::WriteFileAtomically( path, "first version", 13 ) );
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) );
            ASSERT_TRUE( content == "first version" );

            // replace with a different size
            CORE::CDynamicBuffer buffer;
            buffer.CopyFrom( 6, "second" );
            ASSERT_TRUE( CORE::WriteFileAtomically( path, buffer ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) );
            ASSERT_TRUE( content == "second" );

            // empty content
            ASSERT_TRUE( CORE::WriteFileAtomically( path, GUCEF_NULL, 0 ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) );
            ASSERT_TRUE( content.IsNULLOrEmpty() );

            // only the target may remain in the directory
            UInt32 fileCount = 0;
            CORE::CFileSystemIterator it;
            if ( it.FindFirst( dir ) )
            {
                do
                {
                    ++fileCount;
                    ASSERT_TRUE( it.GetResourceName() == "target.txt" );
                }
                while ( it.FindNext() );
                it.FindClose();
            }
            ASSERT_TRUE( 1 == fileCount );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: WriteFileAtomically leaves the original intact when the replace fails" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_AtomicFail( "gucefCORE_TestApp_AtomicFail" );
            CORE::CString dir = scoped_gucefCORE_TestApp_AtomicFail.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );

            // Case 1: the target is a directory so moving the temp file over it fails after the temp file was written
            CORE::CString dirTarget = CORE::CombinePath( dir, "isadir" );
            ASSERT_TRUE( CORE::CreateDirs( dirTarget ) );
            ASSERT_FALSE( CORE::WriteFileAtomically( dirTarget, "data", 4 ) );
            ASSERT_TRUE( CORE::DirExists( dirTarget ) );

            #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

            // Case 2: the temp file cannot be written at all. Not applicable when running as root
            if ( 0 != geteuid() )
            {
                CORE::CString lockedDir = CORE::CombinePath( dir, "locked" );
                ASSERT_TRUE( CORE::CreateDirs( lockedDir ) );
                CORE::CString lockedFile = CORE::CombinePath( lockedDir, "original.txt" );
                ASSERT_TRUE( CORE::WriteStringAsTextFile( lockedFile, "original" ) );
                ::chmod( lockedDir.C_String(), 0555 );

                bool replaced = CORE::WriteFileAtomically( lockedFile, "replacement", 11 );
                ::chmod( lockedDir.C_String(), 0755 );

                ASSERT_FALSE( replaced );
                CORE::CString content;
                ASSERT_TRUE( CORE::LoadTextFileAsString( lockedFile, content, false ) );
                ASSERT_TRUE( content == "original" );
            }

            #endif

            // no temp files may be left behind in the dir
            UInt32 entryCount = 0;
            CORE::CFileSystemIterator it;
            if ( it.FindFirst( dir ) )
            {
                do
                {
                    ++entryCount;
                    ASSERT_FALSE( it.GetResourceName().HasSubstr( ".tmp" ) >= 0 );
                }
                while ( it.FindNext() );
                it.FindClose();
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SetOwnerOnlyAccess restricts a file and a directory to the user" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_OwnerOnly( "gucefCORE_TestApp_OwnerOnly" );
            CORE::CString dir = scoped_gucefCORE_TestApp_OwnerOnly.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString path = CORE::CombinePath( dir, "secret.bin" );
            ASSERT_TRUE( CORE::WriteFileAtomically( path, "key material", 12 ) );

            ASSERT_TRUE( CORE::SetOwnerOnlyAccess( path ) );
            ASSERT_TRUE( CORE::SetOwnerOnlyAccess( dir ) );
            // nothing changes about being able to use them
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) && content == "key material" );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            struct stat info;
            ASSERT_TRUE( 0 == ::stat( dir.C_String(), &info ) && 0700 == ( info.st_mode & 0777 ) );
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            // a file made by replacing it is a new file: it is restricted again
            ASSERT_TRUE( CORE::WriteFileAtomically( path, "replaced", 8 ) );
            ASSERT_TRUE( CORE::SetOwnerOnlyAccess( path ) );
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            // a file that is open to others is closed to them
            ::chmod( path.C_String(), 0666 );
            ASSERT_TRUE( CORE::SetOwnerOnlyAccess( path ) );
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            #endif

            // what is not there and what is not a path
            ASSERT_FALSE( CORE::SetOwnerOnlyAccess( CORE::CombinePath( dir, "missing.bin" ) ) );
            ASSERT_FALSE( CORE::SetOwnerOnlyAccess( CORE::CString::Empty ) );
            CORE::Remove_Directory( dir.C_String(), 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux MoveFile ignored the overwrite flag and always replaced an existing target
    GUCEF_TESTFW_TESTCASE( "Test: MoveFile honors the overwrite flag" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_MoveFile( "gucefCORE_TestApp_MoveFile" );
            CORE::CString dir = scoped_gucefCORE_TestApp_MoveFile.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString source = CORE::CombinePath( dir, "source.txt" );
            CORE::CString target = CORE::CombinePath( dir, "target.txt" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "source" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( target, "target" ) );

            ASSERT_FALSE( CORE::MoveFile( source, target, false ) );
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) );
            ASSERT_TRUE( content == "target" );
            ASSERT_TRUE( CORE::FileExists( source ) );

            ASSERT_TRUE( CORE::MoveFile( source, target, true ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) );
            ASSERT_TRUE( content == "source" );
            ASSERT_FALSE( CORE::FileExists( source ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Copy-on-write table partitions rely on this: a file is replaced as a whole, never seen half written
    GUCEF_TESTFW_TESTCASE( "Test: MoveFile replaces a file atomically: a reader of the old file keeps it, the path gives the new one" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_MoveFileAtomic( "gucefCORE_TestApp_MoveFileAtomic" );
            CORE::CString dir = scoped_gucefCORE_TestApp_MoveFileAtomic.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString target = CORE::CombinePath( dir, "partition.bin" );
            CORE::CString replacement = CORE::CombinePath( dir, "partition.bin.tmp-1-1" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( target, "old contents of the partition" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( replacement, "new contents" ) );

            CORE::CFileAccess reader( target, "rb" );
            ASSERT_TRUE( reader.IsValid() );
            ASSERT_TRUE( CORE::MoveFile( replacement, target, true ) );
            ASSERT_FALSE( CORE::FileExists( replacement ) );

            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) );
            ASSERT_TRUE( content == "new contents" );

            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            // POSIX: the open file is the old one, whole; Windows refuses to replace a file that is open (sharing violation)
            char oldBytes[ 64 ];
            ASSERT_TRUE( 0 == reader.Setpos( 0 ) );
            UInt32 read = reader.Read( oldBytes, 1, sizeof( oldBytes ) );
            ASSERT_TRUE( 29 == read && 0 == memcmp( oldBytes, "old contents of the partition", 29 ) );
            #endif
            reader.Close();

            // the directories of a target are made, a missing source fails without touching the target
            CORE::CString nested = CORE::CombinePath( dir, "a/b/partition.bin" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( replacement, "nested" ) );
            ASSERT_TRUE( CORE::MoveFile( replacement, nested, true ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( nested, content, false ) && content == "nested" );
            ASSERT_FALSE( CORE::MoveFile( replacement, target, true ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) && content == "new contents" );

            CORE::DeleteFile( nested );
            CORE::DeleteFile( target );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_TESTCASE( "Test: MoveFile never widens permissions: the moved file keeps its mode, a refused move leaves the source as it was" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_MoveFileMode( "gucefCORE_TestApp_MoveFileMode" );
            CORE::CString dir = scoped_gucefCORE_TestApp_MoveFileMode.GetPath();
            CORE::DeleteDir( dir, true );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString source = CORE::CombinePath( dir, "secret.bin" );
            CORE::CString target = CORE::CombinePath( dir, "moved.bin" );
            struct stat info;

            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "secret" ) && 0 == ::chmod( source.C_String(), 0600 ) );
            ASSERT_TRUE( CORE::MoveFile( source, target, true ) );
            ASSERT_TRUE( 0 == ::stat( target.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );

            // the rename is refused: a directory with a file in it is where the file would go
            CORE::CString blocker = CORE::CombinePath( dir, "blocker" );
            ASSERT_TRUE( CORE::CreateDirs( blocker ) && CORE::WriteStringAsTextFile( CORE::CombinePath( blocker, "inside.txt" ), "x" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "secret" ) && 0 == ::chmod( source.C_String(), 0600 ) );
            ASSERT_FALSE( CORE::MoveFile( source, blocker, true ) );
            ASSERT_TRUE( 0 == ::stat( source.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );

            CORE::DeleteDir( dir, true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MoveFileAtomically renames or refuses: files only, the overwrite flag, the mode both files have in common" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_MoveFileAtomically( "gucefCORE_TestApp_MoveFileAtomically" );
            CORE::CString dir = scoped_gucefCORE_TestApp_MoveFileAtomically.GetPath();
            CORE::DeleteDir( dir, true );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString source = CORE::CombinePath( dir, "new.bin" );
            CORE::CString target = CORE::CombinePath( dir, "part.bin" );
            CORE::CString content;
            struct stat info;

            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "new" ) && CORE::WriteStringAsTextFile( target, "old" ) );
            ASSERT_TRUE( 0 == ::chmod( source.C_String(), 0664 ) && 0 == ::chmod( target.C_String(), 0640 ) );
            ASSERT_FALSE( CORE::MoveFileAtomically( source, target, false ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) && content == "old" );
            ASSERT_TRUE( CORE::MoveFileAtomically( source, target, true ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) && content == "new" && !CORE::FileExists( source ) );
            ASSERT_TRUE( 0 == ::stat( target.C_String(), &info ) && 0640 == ( info.st_mode & 07777 ) );

            // a new target keeps the mode of the file moved; a missing source, a directory as source or target are refused
            ASSERT_TRUE( CORE::MoveFileAtomically( target, CORE::CombinePath( dir, "a/b/moved.bin" ), false ) );
            ASSERT_TRUE( 0 == ::stat( CORE::CombinePath( dir, "a/b/moved.bin" ).C_String(), &info ) && 0640 == ( info.st_mode & 07777 ) );
            ASSERT_FALSE( CORE::MoveFileAtomically( target, CORE::CombinePath( dir, "other.bin" ), true ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "x" ) );
            ASSERT_FALSE( CORE::MoveFileAtomically( source, CORE::CombinePath( dir, "a" ), true ) );
            ASSERT_FALSE( CORE::MoveFileAtomically( CORE::CombinePath( dir, "a" ), CORE::CombinePath( dir, "c" ), true ) );
            ASSERT_TRUE( CORE::FileExists( source ) && CORE::DirExists( CORE::CombinePath( dir, "a" ) ) );

            CORE::DeleteDir( dir, true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: WriteFileAtomically keeps the permissions of the file it replaces, also with umask 0, and a new file follows the umask" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_WriteAtomicallyMode( "gucefCORE_TestApp_WriteAtomicallyMode" );
            CORE::CString dir = scoped_gucefCORE_TestApp_WriteAtomicallyMode.GetPath();
            CORE::DeleteDir( dir, true );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString path = CORE::CombinePath( dir, "owner.bin" );
            struct stat info;

            ASSERT_TRUE( CORE::WriteStringAsTextFile( path, "first" ) && 0 == ::chmod( path.C_String(), 0600 ) );
            mode_t previousMask = ::umask( 0 );
            const bool replaced = CORE::WriteFileAtomically( path, "second", 6 );
            ::umask( previousMask );
            ASSERT_TRUE( replaced );
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( path, content, false ) && content == "second" );
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );

            ASSERT_TRUE( CORE::DeleteFile( path ) );
            previousMask = ::umask( 022 );
            const bool created = CORE::WriteFileAtomically( path, "third", 5 );
            ::umask( previousMask );
            ASSERT_TRUE( created );
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &info ) && 0644 == ( info.st_mode & 07777 ) );

            CORE::DeleteDir( dir, true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    // Regression: the C style Move_File( dst, src ) moved in the wrong direction
    GUCEF_TESTFW_TESTCASE( "Test: Move_File moves the source to the destination" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_Move_File( "gucefCORE_TestApp_Move_File" );
            CORE::CString dir = scoped_gucefCORE_TestApp_Move_File.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString source = CORE::CombinePath( dir, "source.txt" );
            CORE::CString destination = CORE::CombinePath( dir, "destination.txt" );
            CORE::DeleteFile( destination );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( source, "moved content" ) );

            ASSERT_TRUE( 0 != CORE::Move_File( destination.C_String(), source.C_String(), 1 ) );
            ASSERT_FALSE( CORE::FileExists( source ) );
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( destination, content, false ) );
            ASSERT_TRUE( content == "moved content" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: on Linux FileExists also returned true for directories, unlike on Windows
    GUCEF_TESTFW_TESTCASE( "Test: FileExists is only true for files and DirExists only for directories" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_FileExists( "gucefCORE_TestApp_FileExists" );
            CORE::CString dir = scoped_gucefCORE_TestApp_FileExists.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString file = CORE::CombinePath( dir, "file.txt" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "content" ) );

            ASSERT_TRUE( CORE::FileExists( file ) );
            ASSERT_FALSE( CORE::DirExists( file ) );
            ASSERT_FALSE( CORE::FileExists( dir ) );
            ASSERT_TRUE( CORE::DirExists( dir ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "nothing.txt" ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Remove_Directory removes a directory with its files and sub directories" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_RemoveDir( "gucefCORE_TestApp_RemoveDir" );
            CORE::CString dir = scoped_gucefCORE_TestApp_RemoveDir.GetPath();
            CORE::CString sub = CORE::CombinePath( dir, "sub" );
            ASSERT_TRUE( CORE::CreateDirs( sub ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "a.txt" ), "a" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( sub, "b.txt" ), "b" ) );

            ASSERT_TRUE( 0 != CORE::Remove_Directory( dir.C_String(), 1 ) );
            ASSERT_FALSE( CORE::DirExists( dir ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The recursive deletes below run only when the listing is known to be free of . and ..
    bool iteratorIsSafe = false;

    GUCEF_TESTFW_TESTCASE( "Test: CFileSystemIterator never lists . or .., wherever the file system puts them in the listing" )
        try
        {
            // In a sandbox of the working dir (a disk file system, which lists in hash order), never in a real dir
            CORE::CString sandbox = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "gucefCORE_TestApp_IteratorDots" );
            CORE::CString dir = CORE::CombinePath( sandbox, "listed" );
            ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( dir, "sub" ) ) );
            for ( UInt32 i=0; i<64; ++i )
                ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "file" + CORE::ToString( i ) + ".txt" ), "x" ) );

            UInt32 entries = 0;
            UInt32 dots = 0;
            CORE::CFileSystemIterator it;
            if ( it.FindFirst( dir ) )
            {
                do
                {
                    ++entries;
                    CORE::CString name = it.GetResourceName();
                    if ( "." == name || ".." == name )
                        ++dots;
                }
                while ( it.FindNext() );
                it.FindClose();
            }
            ASSERT_TRUE( 0 == dots );
            ASSERT_TRUE( 65 == entries );
            iteratorIsSafe = 0 == dots;

            // Removed one by one: a recursive delete is only safe once the listing is
            for ( UInt32 i=0; i<64; ++i )
                CORE::DeleteFile( CORE::CombinePath( dir, "file" + CORE::ToString( i ) + ".txt" ) );
            CORE::DeleteDir( CORE::CombinePath( dir, "sub" ), false );
            CORE::DeleteDir( dir, false );
            CORE::DeleteDir( sandbox, false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CFileSystemIterator with a glob pattern lists the matching entries only, wherever they are in the listing" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "gucefCORE_TestApp_IteratorGlob" );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            // Names that are part of the pattern itself, which a match the wrong way around takes for matches
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            static const char* names[] = { "a.txt", "b.txt", "t", "x", "tx", "xt", ".t", "txt", "c.dat", "d.dat" };
            #else
            // where a name may have a '*', a name taken for the pattern matches wrongly
            static const char* names[] = { "a.txt", "b.txt", "t", "x", "tx", "xt", ".t", "txt", "c.dat", "d.dat", "t*", "x*", ".t*", "*t", "z.*" };
            #endif
            const UInt32 nameCount = sizeof( names ) / sizeof( names[ 0 ] );
            for ( UInt32 i=0; i<nameCount; ++i )
                ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, names[ i ] ), "x" ) );

            CORE::CString::StringSet found;
            CORE::CFileSystemIterator it;
            if ( it.FindFirst( CORE::CombinePath( dir, "*.txt" ) ) )
            {
                do
                {
                    found.insert( it.GetResourceName() );
                }
                while ( it.FindNext() );
                it.FindClose();
            }
            ASSERT_TRUE( 2 == found.size() );
            ASSERT_TRUE( found.find( "a.txt" ) != found.end() && found.find( "b.txt" ) != found.end() );

            for ( UInt32 i=0; i<nameCount; ++i )
                CORE::DeleteFile( CORE::CombinePath( dir, names[ i ] ) );
            CORE::DeleteDir( dir, false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: DeleteDir removes what is in the directory only: not its parent, not what a symlink in it points to" )
        GUCEF_TESTFW_SKIP_IF( !iteratorIsSafe, "The directory listing is not safe for a recursive delete" );
        try
        {
            // The sandbox has sentinels next to the directory that is deleted and in a directory a symlink points to
            CORE::CString sandbox = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "gucefCORE_TestApp_DeleteDirSandbox" );
            CORE::CString victim = CORE::CombinePath( sandbox, "victim" );
            CORE::CString outside = CORE::CombinePath( sandbox, "outside" );
            CORE::CString parentSentinel = CORE::CombinePath( sandbox, "sentinel.txt" );
            CORE::CString outsideSentinel = CORE::CombinePath( outside, "sentinel.txt" );
            ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( victim, "sub" ) ) );
            ASSERT_TRUE( CORE::CreateDirs( outside ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( parentSentinel, "keep" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( outsideSentinel, "keep" ) );
            for ( UInt32 i=0; i<64; ++i )
                ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( victim, "file" + CORE::ToString( i ) + ".txt" ), "x" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( CORE::CombinePath( victim, "sub" ), "deep.txt" ), "x" ) );
            #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
            ASSERT_TRUE( 0 == ::symlink( outside.C_String(), CORE::CombinePath( victim, "linkToOutside" ).C_String() ) );
            #endif

            // . and .. are no directories to delete
            ASSERT_FALSE( CORE::DeleteDir( CORE::CombinePath( victim, "." ), true ) );
            ASSERT_FALSE( CORE::DeleteDir( CORE::CombinePath( victim, ".." ), true ) );
            ASSERT_TRUE( CORE::FileExists( parentSentinel ) && CORE::DirExists( victim ) );

            ASSERT_TRUE( CORE::DeleteDir( victim, true ) );
            ASSERT_FALSE( CORE::DirExists( victim ) );
            ASSERT_TRUE( CORE::FileExists( parentSentinel ) );
            ASSERT_TRUE( CORE::FileExists( outsideSentinel ) );

            CORE::DeleteFile( outsideSentinel );
            CORE::DeleteFile( parentSentinel );
            CORE::DeleteDir( outside, false );
            CORE::DeleteDir( sandbox, false );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_TESTCASE( "Test: CFileSystemIterator releases the directory when a listing ends" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_IteratorRelease( "gucefCORE_TestApp_IteratorRelease" );
            CORE::CString dir = scoped_gucefCORE_TestApp_IteratorRelease.GetPath();
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "a.txt" ), "a" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "b.txt" ), "b" ) );

            const UInt32 before = CountOpenFileDescriptors();
            for ( UInt32 i=0; i<20; ++i )
            {
                // listed to the end
                CORE::CFileSystemIterator it;
                if ( it.FindFirst( dir ) )
                {
                    while ( it.FindNext() ) {}
                }

                // nothing matches the pattern
                CORE::CFileSystemIterator none;
                ASSERT_FALSE( none.FindFirst( CORE::CombinePath( dir, "*.nothing" ) ) );
            }
            ASSERT_TRUE( before == CountOpenFileDescriptors() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CFileSystemIterator resolves a symlink whose target path is longer than the first read buffer" )
        try
        {
            // the target path is over 256 bytes, so the link is read again into a grown buffer
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_LongLink( "gucefCORE_TestApp_LongLink" );
            CORE::CString base = scoped_gucefCORE_TestApp_LongLink.GetPath();
            CORE::Remove_Directory( base.C_String(), 1 );
            CORE::CString targetDir = base;
            for ( UInt32 i=0; i<5; ++i )
                targetDir = CORE::CombinePath( targetDir, "a_rather_long_directory_name_to_make_the_path_long_" + CORE::ToString( i ) );
            ASSERT_TRUE( CORE::CreateDirs( targetDir ) );
            const CORE::CString target = CORE::CombinePath( targetDir, "target.txt" );
            ASSERT_TRUE( target.Length() > 256 );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( target, "0123456789" ) );
            ASSERT_TRUE( 0 == symlink( target.C_String(), CORE::CombinePath( base, "link" ).C_String() ) );

            bool foundLink = false;
            CORE::CFileSystemIterator it;
            if ( it.FindFirst( base ) )
            {
                do
                {
                    if ( "link" == it.GetResourceName() )
                    {
                        foundLink = true;
                        ASSERT_TRUE( it.IsSymlink() );
                        ASSERT_TRUE( target == it.GetSymlinkedResourcePath() );      // exactly the target, nothing left over from the first read
                    }
                }
                while ( it.FindNext() );
            }
            it.FindClose();
            ASSERT_TRUE( foundLink );
            CORE::Remove_Directory( base.C_String(), 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CreateOwnerOnlyFile makes a new file for the owner only in one step, also with umask 0, and never replaces what is there" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_OwnerOnlyFile( "gucefCORE_TestApp_OwnerOnlyFile" );
            CORE::CString dir = scoped_gucefCORE_TestApp_OwnerOnlyFile.GetPath();
            CORE::Remove_Directory( dir.C_String(), 1 );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            const CORE::CString secretFile = CORE::CombinePath( dir, "secret.token" );
            const char* secret = "s3cr3t-token";
            CORE::CString problem;

            // umask 0 would make a plain create 0666: the mode is given to open() itself, so it is 0600 from the first moment
            const mode_t oldMask = ::umask( 0 );
            const bool created = CORE::CreateOwnerOnlyFile( secretFile, secret, (UInt32) strlen( secret ), problem );
            struct stat info;
            const bool statted = 0 == ::lstat( secretFile.C_String(), &info );
            ::umask( oldMask );
            ASSERT_TRUE( created );
            ASSERT_TRUE( statted && S_ISREG( info.st_mode ) && 0600 == ( info.st_mode & 07777 ) && info.st_uid == ::geteuid() );

            CORE::CDynamicBuffer content;
            bool missing = true;
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( secretFile, content, problem, &missing ) && !missing );
            ASSERT_TRUE( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) == secret );

            // a file that is there already, made by someone before us (the race), is never written to or trusted by a create
            ASSERT_FALSE( CORE::CreateOwnerOnlyFile( secretFile, "other", 5, problem ) );
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( secretFile, content, problem ) );
            ASSERT_TRUE( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) == secret );

            // a symbolic link at the path, to a file or to nothing, is not followed
            const CORE::CString target = CORE::CombinePath( dir, "target.txt" );
            const CORE::CString linkToFile = CORE::CombinePath( dir, "link-to-file" );
            const CORE::CString linkToNothing = CORE::CombinePath( dir, "link-to-nothing" );
            const CORE::CString nothing = CORE::CombinePath( dir, "planted-later" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( target, "not a secret" ) );
            ASSERT_TRUE( 0 == ::symlink( target.C_String(), linkToFile.C_String() ) );
            ASSERT_TRUE( 0 == ::symlink( nothing.C_String(), linkToNothing.C_String() ) );
            ASSERT_FALSE( CORE::CreateOwnerOnlyFile( linkToFile, "x", 1, problem ) );
            ASSERT_FALSE( CORE::CreateOwnerOnlyFile( linkToNothing, "x", 1, problem ) );
            ASSERT_FALSE( CORE::FileExists( nothing ) );
            CORE::CString targetText;
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, targetText ) && targetText == "not a secret" );

            CORE::Remove_Directory( dir.C_String(), 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ReadOwnerOnlyFile refuses a link, a file others can read, another kind of file, and tells a missing file apart" )
        try
        {
            TESTFW::CScopedTestDir scoped_gucefCORE_TestApp_OwnerOnlyFileRead( "gucefCORE_TestApp_OwnerOnlyFileRead" );
            CORE::CString dir = scoped_gucefCORE_TestApp_OwnerOnlyFileRead.GetPath();
            CORE::Remove_Directory( dir.C_String(), 1 );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            CORE::CString problem;
            CORE::CDynamicBuffer content;
            bool missing = false;

            const CORE::CString good = CORE::CombinePath( dir, "good.token" );
            ASSERT_TRUE( CORE::CreateOwnerOnlyFile( good, "abc", 3, problem ) );
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( good, content, problem, &missing ) && !missing && 3 == content.GetDataSize() );

            // readable by the group or by others: refused, whatever it holds
            const CORE::CString open = CORE::CombinePath( dir, "open.token" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( open, "abc" ) );
            ASSERT_TRUE( 0 == ::chmod( open.C_String(), 0644 ) );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( open, content, problem, &missing ) );
            ASSERT_FALSE( missing );
            ASSERT_TRUE( 0 == ::chmod( open.C_String(), 0620 ) );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( open, content, problem ) );
            ASSERT_TRUE( 0 == ::chmod( open.C_String(), 0600 ) );
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( open, content, problem ) );

            // a link to a good file is not followed
            const CORE::CString link = CORE::CombinePath( dir, "link.token" );
            ASSERT_TRUE( 0 == ::symlink( good.C_String(), link.C_String() ) );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( link, content, problem, &missing ) );
            ASSERT_FALSE( missing );

            // a directory and a FIFO are no files with a secret; the FIFO does not block the read
            const CORE::CString subDir = CORE::CombinePath( dir, "sub.token" );
            ASSERT_TRUE( CORE::CreateDirs( subDir ) );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( subDir, content, problem ) );
            const CORE::CString fifo = CORE::CombinePath( dir, "fifo.token" );
            ASSERT_TRUE( 0 == ::mkfifo( fifo.C_String(), 0600 ) );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( fifo, content, problem ) );

            const CORE::CString absent = CORE::CombinePath( dir, "absent.token" );
            ASSERT_FALSE( CORE::ReadOwnerOnlyFile( absent, content, problem, &missing ) );
            ASSERT_TRUE( missing );

            CORE::Remove_Directory( dir.C_String(), 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL File Utils TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
