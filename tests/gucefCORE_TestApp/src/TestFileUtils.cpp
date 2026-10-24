/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
            CORE::CString nestedDir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_CreateDirs/a/b/c" );
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
            CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_FsIterator" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_Durable" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_Atomic" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_AtomicFail" );
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

    // Regression: on Linux MoveFile ignored the overwrite flag and always replaced an existing target
    GUCEF_TESTFW_TESTCASE( "Test: MoveFile honors the overwrite flag" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_MoveFile" );
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

    // Regression: the C style Move_File( dst, src ) moved in the wrong direction
    GUCEF_TESTFW_TESTCASE( "Test: Move_File moves the source to the destination" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_Move_File" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_FileExists" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_RemoveDir" );
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
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_IteratorRelease" );
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
            CORE::CString base = CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_LongLink" );
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
    #endif

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL File Utils TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
