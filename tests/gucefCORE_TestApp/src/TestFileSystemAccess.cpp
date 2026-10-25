/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <stdlib.h>
#include <string.h>

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

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

/* in TestFileSystemAccessResultC.c, compiled as C */
extern "C" int FileSystemAccessResultHeaderWorksInC( void );

namespace {

/** A fresh, empty directory for a test */
CORE::CString
MakeTestDir( const char* name )
{
    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::TempDir(), "gucefCORE_TestApp_FileSystemAccess" ), name );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, const CORE::CString& content )
{
    return CORE::WriteStringAsTextFile( path, content );
}

/*-------------------------------------------------------------------------*/

/** The names of the entries an iterator gives, with a '/' after directories */
CORE::CString::StringSet
ListNames( CORE::IFileSystemIterator& iterator, const CORE::CString& dir )
{
    CORE::CString::StringSet names;
    if ( iterator.FindFirst( dir ) )
    {
        do
        {
            names.insert( iterator.GetResourceName() + ( iterator.IsADirectory() ? "/" : "" ) );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

/** Waits for an async operation of a file system access to end and gives its result */
CORE::TFileSystemAccessResult
AwaitResult( const CORE::CFutureResult& future )
{
    future.Await( 10000 );
    CORE::TFileSystemAccessResult result = CORE::FILESYSTEMACCESS_TIMEOUT;
    CORE::IFileSystemAccess::TryGetAsyncResult( future, result );
    return result;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** Sets or clears an environment variable for a scope */
class CScopedEnv
{
    public:

    CScopedEnv( const char* name, const char* value )
        : m_name( name )
        , m_had( GUCEF_NULL != getenv( name ) )
        , m_old( GUCEF_NULL != getenv( name ) ? getenv( name ) : "" )
    {
        if ( GUCEF_NULL != value )
            setenv( name, value, 1 );
        else
            unsetenv( name );
    }

    ~CScopedEnv()
    {
        if ( m_had )
            setenv( m_name.C_String(), m_old.C_String(), 1 );
        else
            unsetenv( m_name.C_String() );
    }

    private:

    CORE::CString m_name;
    bool m_had;
    CORE::CString m_old;
};

#endif

/*-------------------------------------------------------------------------*/

void
PerformResultCodeTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: Every result code has its name, unknown values have one too, and C code can read the header" )
        try
        {
            static const CORE::TFileSystemAccessResult all[] = {
                CORE::FILESYSTEMACCESS_SUCCESS, CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS, CORE::FILESYSTEMACCESS_NOT_FOUND,
                CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND, CORE::FILESYSTEMACCESS_ALREADY_EXISTS, CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY,
                CORE::FILESYSTEMACCESS_NOT_A_FILE, CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY, CORE::FILESYSTEMACCESS_ACCESS_DENIED,
                CORE::FILESYSTEMACCESS_READ_ONLY, CORE::FILESYSTEMACCESS_INVALID_PATH, CORE::FILESYSTEMACCESS_INVALID_NAME,
                CORE::FILESYSTEMACCESS_SAME_PATH, CORE::FILESYSTEMACCESS_CROSS_DEVICE, CORE::FILESYSTEMACCESS_IN_USE,
                CORE::FILESYSTEMACCESS_NO_SPACE, CORE::FILESYSTEMACCESS_TOO_LONG, CORE::FILESYSTEMACCESS_NOT_SUPPORTED,
                CORE::FILESYSTEMACCESS_QUEUEING_FAILED, CORE::FILESYSTEMACCESS_CANCELLED, CORE::FILESYSTEMACCESS_TIMEOUT,
                CORE::FILESYSTEMACCESS_FAILED };
            CORE::CString::StringSet names;
            for ( size_t i=0; i<sizeof( all ) / sizeof( all[ 0 ] ); ++i )
            {
                const CORE::CString name = CORE::FileSystemAccessResultToString( all[ i ] );
                ASSERT_TRUE( name.StartsWith( "FILESYSTEMACCESS_" ) && "FILESYSTEMACCESS_UNKNOWN_RESULT" != name );
                names.insert( name );
            }
            ASSERT_TRUE( names.size() == sizeof( all ) / sizeof( all[ 0 ] ) );
            ASSERT_TRUE( CORE::CString( "FILESYSTEMACCESS_UNKNOWN_RESULT" ) == CORE::FileSystemAccessResultToString( (CORE::TFileSystemAccessResult) 77 ) );
            ASSERT_TRUE( CORE::FileSystemAccessSucceeded( CORE::FILESYSTEMACCESS_SUCCESS ) && !CORE::FileSystemAccessSucceeded( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS ) );
            ASSERT_TRUE( 1 == FileSystemAccessResultHeaderWorksInC() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSpecialDirTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: The special directories of the platform are listed, each listed one resolves, those of another platform are not listed" )
        try
        {
            CORE::TSpecialDirSet dirs;
            ASSERT_TRUE( CORE::ListAvailableSpecialDirs( dirs ) );
            ASSERT_FALSE( dirs.empty() );
            CORE::TSpecialDirSet::const_iterator i = dirs.begin();
            while ( i != dirs.end() )
            {
                CORE::CString path;
                const bool resolved = CORE::TryResolveSpecialDir( (*i), path );
                if ( !resolved )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemAccess test: listed special dir " << CORE::SpecialDirToString( (*i) ) << " does not resolve" );
                ASSERT_TRUE( resolved && !path.IsNULLOrEmpty() );
                ++i;
            }
            ASSERT_TRUE( dirs.find( CORE::SPECIALDIR_TEMP_DIR ) != dirs.end() && dirs.find( CORE::SPECIALDIR_MODULE_DIR ) != dirs.end() );
            ASSERT_TRUE( CORE::CString( "SPECIALDIR_MY_DOWNLOADS_DIR" ) == CORE::SpecialDirToString( CORE::SPECIALDIR_MY_DOWNLOADS_DIR ) );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( dirs.find( CORE::SPECIALDIR_HOME_DIR ) != dirs.end() && dirs.find( CORE::SPECIALDIR_MY_DOWNLOADS_DIR ) != dirs.end() );
            ASSERT_TRUE( dirs.find( CORE::SPECIALDIR_WINDOWS_DIR ) == dirs.end() && dirs.find( CORE::SPECIALDIR_SYSTEM32_DIR ) == dirs.end() );
            ASSERT_TRUE( dirs.find( CORE::SPECIALDIR_X86_PROGRAMS_DIR ) == dirs.end() );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: On Linux the user directories come from the environment, else from user-dirs.dirs as desktops write it, else the home directory" )
        try
        {
            const CORE::CString configDir = MakeTestDir( "xdgconfig" );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( configDir, "user-dirs.dirs" ),
                                    "# written by xdg-user-dirs-update\nXDG_DESKTOP_DIR=\"$HOME/Bureau\"\nXDG_DOWNLOAD_DIR=\"/data/My Downloads\"\n" ) );
            CScopedEnv home( "HOME", "/home/tester" );
            CScopedEnv config( "XDG_CONFIG_HOME", configDir.C_String() );
            CScopedEnv download( "XDG_DOWNLOAD_DIR", GUCEF_NULL );
            CScopedEnv desktop( "XDG_DESKTOP_DIR", GUCEF_NULL );
            CScopedEnv music( "XDG_MUSIC_DIR", GUCEF_NULL );

            CORE::CString path;
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_DESKTOP_DIR, path ) && "/home/tester/Bureau" == path );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_MY_DOWNLOADS_DIR, path ) && "/data/My Downloads" == path );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_MY_MUSIC_DIR, path ) && "/home/tester" == path );      // not in the file

            CScopedEnv fromEnv( "XDG_DOWNLOAD_DIR", "/elsewhere" );
            ASSERT_TRUE( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_MY_DOWNLOADS_DIR, path ) && "/elsewhere" == path );
            CORE::DeleteDir( configDir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformGlobPatternTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: Glob patterns match names and paths with *, **, ?, classes, alternatives and escapes" )
        try
        {
            using CORE::CGlobPattern;
            ASSERT_TRUE( CGlobPattern::IsMatch( "*.cue", "ITVGOLD.CUE" ) );                       // case ignored by default
            ASSERT_FALSE( CGlobPattern::IsMatch( "*.cue", "ITVGOLD.CUE", true ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "*", "" ) && CGlobPattern::IsMatch( "", "" ) && !CGlobPattern::IsMatch( "", "a" ) );
            ASSERT_FALSE( CGlobPattern::IsMatch( "*.cue", "disc/ITV.cue" ) );                       // * stops at a directory
            ASSERT_TRUE( CGlobPattern::IsMatch( "*/*.cue", "disc/ITV.cue" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "**/*.cue", "a/b/c/ITV.cue" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "**/*.cue", "ITV.cue" ) );                          // **/ also matches no directory
            ASSERT_TRUE( CGlobPattern::IsMatch( "a/**", "a/b/c" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "disc?.iso", "disc1.iso" ) && !CGlobPattern::IsMatch( "disc?.iso", "disc12.iso" ) );
            ASSERT_FALSE( CGlobPattern::IsMatch( "a?b", "a/b" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "track[0-9][0-9].wav", "track07.wav" ) && !CGlobPattern::IsMatch( "track[0-9][0-9].wav", "track7a.wav" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "[!.]*", "save.gam" ) && !CGlobPattern::IsMatch( "[!.]*", ".hidden" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "[^x]", "y" ) && !CGlobPattern::IsMatch( "[^x]", "x" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "[]a]", "]" ) );                                    // ] first is a character
            ASSERT_TRUE( CGlobPattern::IsMatch( "*.{cue,iso}", "game.iso" ) && CGlobPattern::IsMatch( "*.{cue,iso}", "game.cue" ) );
            ASSERT_FALSE( CGlobPattern::IsMatch( "*.{cue,iso}", "game.bin" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "{a,b{c,d}}x", "bdx" ) && !CGlobPattern::IsMatch( "{a,b{c,d}}x", "bx" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "a\\*b", "a*b" ) && !CGlobPattern::IsMatch( "a\\*b", "axb" ) );
            ASSERT_TRUE( CGlobPattern::IsMatch( "m?sique", "m\xC3\xBBsique" ) );                     // ? is one code point (u with a circumflex)
            ASSERT_TRUE( CGlobPattern::IsMatch( "*\xC3\xA9*", "caf\xC3\xA9 menu" ) );

            CGlobPattern open( "[abc" );
            ASSERT_FALSE( open.IsValid() || open.IsMatch( "a" ) );
            CGlobPattern brace( "{a,b" );
            ASSERT_FALSE( brace.IsValid() );
            CGlobPattern valid( "*.GAM", true );
            ASSERT_TRUE( valid.IsValid() && valid.IsCaseSensitive() && "*.GAM" == valid.GetPattern() && valid.IsMatch( "GAME1.GAM" ) );

            ASSERT_TRUE( CGlobPattern::HasGlobCharacters( "*.cue" ) && CGlobPattern::HasGlobCharacters( "disc{1,2}" ) );

            // the suffix checks the archive type lookup of the VFS relies on
            ASSERT_TRUE( CORE::CString( "data.zip" ).EndsWith( ".zip" ) );
            ASSERT_TRUE( CORE::CString( "a.zip" ).EndsWith( ".zip" ) );
            ASSERT_TRUE( CORE::CString( "ab.zip" ).EndsWith( ".zip" ) );
            ASSERT_TRUE( CORE::CString( ".zip" ).EndsWith( ".zip" ) );
            ASSERT_FALSE( CORE::CString( "a.zap" ).EndsWith( ".zip" ) );

            // also strings made from others, as folder names of a listing are: cut, then a separator appended
            CORE::CString folder = CORE::CString( "empty/" ).CutChars( 1, false );
            ASSERT_TRUE( "empty" == folder && 5 == folder.Length() );
            CORE::CString listed = folder.CutChars( 0, true ) + '/';
            ASSERT_TRUE( "empty/" == listed && 6 == listed.Length() );
            ASSERT_TRUE( listed.EndsWith( "empty/" ) );
            ASSERT_TRUE( listed.EndsWith( "/" ) && listed.EndsWith( "y/" ) );
            ASSERT_TRUE( CORE::CString( "empty/" ).EndsWith( "empty/" ) );
            ASSERT_TRUE( CORE::CString( "data/" ).EndsWith( "data/" ) );

            // a suffix longer than the string is never its end (it compared "not found" with the negative difference of the lengths)
            ASSERT_FALSE( CORE::CString( "data/" ).EndsWith( "empty/" ) );
            ASSERT_FALSE( CORE::CString( "a" ).EndsWith( "ab" ) );
            ASSERT_FALSE( CORE::CString( "zip" ).EndsWith( ".zip" ) );
            ASSERT_TRUE( 1 == CORE::CString( "zip" ).ParseUniqueElements( ';', false ).size() );
            ASSERT_TRUE( 2 == CORE::CString( "zip;fsx" ).ParseUniqueElements( ';', false ).size() );
            ASSERT_TRUE( 2 == CORE::CString( "zip;fsx" ).ParseElements( ';', false ).size() );
            ASSERT_TRUE( 1 == CORE::CString( "zip" ).ParseElements( ';', false ).size() );

            // the elements are the text between the separators, also the last one
            CORE::CString::StringVector elements = CORE::CString( "zip;fsx" ).ParseElements( ';', false );
            ASSERT_TRUE( 2 == elements.size() && "zip" == elements[ 0 ] && "fsx" == elements[ 1 ] && 3 == elements[ 1 ].Length() );
            elements = CORE::CString( "7z" ).ParseElements( ';', false );
            ASSERT_TRUE( 1 == elements.size() && "7z" == elements[ 0 ] && 2 == elements[ 0 ].Length() && 3 == elements[ 0 ].ByteSize() );
            CORE::CString::StringSet unique = CORE::CString( "7z;7z" ).ParseUniqueElements( ';', false );
            ASSERT_TRUE( 1 == unique.size() && "7z" == *unique.begin() && 2 == unique.begin()->Length() );
            unique = CORE::CString( "cab" ).ParseUniqueElements( ';', false );
            ASSERT_TRUE( 1 == unique.size() && "cab" == *unique.begin() && 3 == unique.begin()->Length() );
            ASSERT_TRUE( CORE::CString( "a.cab" ).EndsWith( "." + *unique.begin() ) );
            ASSERT_FALSE( CGlobPattern::HasGlobCharacters( "ITVGOLD.CUE" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformNativeFileSystemAccessTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: CORE::MoveFile moves a directory with its contents (the rename the native file system access relies on)" )
        try
        {
            const CORE::CString dir = MakeTestDir( "movedir" );
            const CORE::CString from = CORE::CombinePath( dir, "from" );
            ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( from, "sub" ) ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( from, "sub/a.txt" ), "a" ) );
            const CORE::CString to = CORE::CombinePath( dir, "to" );
            ASSERT_TRUE( CORE::MoveFile( from, to, false ) );
            ASSERT_TRUE( CORE::DirExists( to ) && !CORE::DirExists( from ) && CORE::FileExists( CORE::CombinePath( to, "sub/a.txt" ) ) );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: The native file system makes, moves, renames and deletes files and directories and tells why it cannot" )
        try
        {
            CORE::TFileSystemAccessPtr access = ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
            ASSERT_TRUE( !access.IsNULL() && CORE::CNativeFileSystemAccess::DefaultDisplayName == access->GetDisplayName() );
            const CORE::CString dir = MakeTestDir( "native" );

            // directories
            const CORE::CString a = CORE::CombinePath( dir, "a" );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectory( a, false ) && CORE::DirExists( a ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ALREADY_EXISTS == access->TryCreateDirectory( a, false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND == access->TryCreateDirectory( CORE::CombinePath( dir, "x/y" ), false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectory( CORE::CombinePath( dir, "x/y" ), true ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_PATH == access->TryCreateDirectory( "", true ) );

            // files
            const CORE::CString file = CORE::CombinePath( a, "one.txt" );
            ASSERT_TRUE( WriteFile( file, "hello" ) );
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( file, type ) && CORE::FILESYSTEMENTRYTYPE_FILE == type );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( a, type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( CORE::CombinePath( a, "none" ), type ) && CORE::FILESYSTEMENTRYTYPE_NONE == type );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetMetaData( file, metaData ) && metaData.hasResourceSizeInBytes && 5 == metaData.resourceSizeInBytes );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetMetaData( CORE::CombinePath( a, "none" ), metaData ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY == access->TryDeleteDirectory( a, false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY == access->TryDeleteDirectory( file, true ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_A_FILE == access->TryDeleteFile( a ) );

            // rename and move
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_NAME == access->TryRenameFile( file, "sub/two.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_NAME == access->TryRenameFile( file, ".." ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryRenameFile( file, "two.txt" ) );
            const CORE::CString two = CORE::CombinePath( a, "two.txt" );
            ASSERT_TRUE( CORE::FileExists( two ) && !CORE::FileExists( file ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryRenameFile( file, "three.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SAME_PATH == access->TryMoveFile( two, two, true ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( a, "other.txt" ), "x" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ALREADY_EXISTS == access->TryMoveFile( two, CORE::CombinePath( a, "other.txt" ), false ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveFile( two, CORE::CombinePath( a, "other.txt" ), true ) );
            ASSERT_TRUE( 5 == CORE::FileSize( CORE::CombinePath( a, "other.txt" ) ) );

            // a directory moves with its contents, not into itself
            const CORE::CString b = CORE::CombinePath( dir, "b" );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_INVALID_PATH == access->TryMoveDirectory( a, CORE::CombinePath( a, "inside" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SAME_PATH == access->TryMoveDirectory( a, a ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveDirectory( a, b ) );
            ASSERT_TRUE( CORE::DirExists( b ) && !CORE::DirExists( a ) && CORE::FileExists( CORE::CombinePath( b, "other.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryMoveDirectory( a, b ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ALREADY_EXISTS == access->TryMoveDirectory( CORE::CombinePath( dir, "x" ), b ) );

            // several files at once: some can, some cannot
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "m1.txt" ), "1" ) && WriteFile( CORE::CombinePath( dir, "m2.txt" ), "2" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( b, "m2.txt" ), "old" ) );
            CORE::CString::StringVector files;
            files.push_back( CORE::CombinePath( dir, "m1.txt" ) );
            files.push_back( CORE::CombinePath( dir, "m2.txt" ) );
            files.push_back( CORE::CombinePath( dir, "missing.txt" ) );
            CORE::IFileSystemAccess::TFileSystemAccessResultVector perFile;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS == access->TryMoveFiles( files, b, false, &perFile ) );
            ASSERT_TRUE( 3 == perFile.size() && CORE::FILESYSTEMACCESS_SUCCESS == perFile[ 0 ] && CORE::FILESYSTEMACCESS_ALREADY_EXISTS == perFile[ 1 ] && CORE::FILESYSTEMACCESS_NOT_FOUND == perFile[ 2 ] );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryMoveFiles( files, CORE::CombinePath( dir, "nowhere" ), false, &perFile ) );
            files.pop_back();
            files.erase( files.begin() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveFiles( files, b, true, GUCEF_NULL ) );

            // the iterator, at the interface level of CFileSystemIterator
            CORE::TFileSystemIteratorPtr iterator;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( b, iterator ) && !iterator.IsNULL() );
        #ifdef GUCEF_RTTI_IS_SUPPORTED
            ASSERT_TRUE( GUCEF_NULL != dynamic_cast< CORE::CFileSystemIterator* >( iterator.GetPointerAlways() ) );
        #endif
            CORE::CString::StringSet names = ListNames( *iterator, b );
            ASSERT_TRUE( 3 == names.size() && names.count( "other.txt" ) && names.count( "m1.txt" ) && names.count( "m2.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY == access->TryGetFileSystemIterator( CORE::CombinePath( b, "m1.txt" ), iterator ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryGetFileSystemIterator( CORE::CombinePath( dir, "nowhere" ), iterator ) );

            // deleting
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteFile( CORE::CombinePath( b, "m1.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryDeleteFile( CORE::CombinePath( b, "m1.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteDirectory( b, true ) && !CORE::DirExists( b ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == access->TryDeleteDirectory( b, true ) );

            // variables in paths are resolved
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "$TEMPDIR$", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
            ASSERT_TRUE( access->IsWriteable( dir ) );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( "/" == access->GetRootPath() );
            if ( 0 != geteuid() )
            {
                // a directory the user cannot write to: the OS refuses, the metadata of Linux does not tell why
                const CORE::CString locked = CORE::CombinePath( dir, "locked" );
                ASSERT_TRUE( CORE::CreateDirs( locked ) && WriteFile( CORE::CombinePath( locked, "f.txt" ), "f" ) );
                chmod( locked.C_String(), 0500 );
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_FAILED == access->TryDeleteFile( CORE::CombinePath( locked, "f.txt" ) ) );
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_FAILED == access->TryCreateDirectory( CORE::CombinePath( locked, "sub" ), false ) );
                chmod( locked.C_String(), 0700 );
            }
            #endif

            // special directories through the access
            CORE::TSpecialDirSet specialDirs;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->ListAvailableSpecialDirs( specialDirs ) && !specialDirs.empty() );
            CORE::CString tempDir;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryResolveSpecialDir( CORE::SPECIALDIR_TEMP_DIR, tempDir ) && !tempDir.IsNULLOrEmpty() );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_SUPPORTED == access->TryResolveSpecialDir( CORE::SPECIALDIR_WINDOWS_DIR, tempDir ) );
            #endif

            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: The async variants run the operation on a pool thread and give its result, a listing is read whole" )
        try
        {
            CORE::TFileSystemAccessPtr access = ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
            const CORE::CString dir = MakeTestDir( "async" );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectoryAsync( CORE::CombinePath( dir, "d" ), false, future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && CORE::DirExists( CORE::CombinePath( dir, "d" ) ) );

            // a future can be assigned: both refer to the same task
            CORE::CFutureResult copy( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            copy = future;
            ASSERT_TRUE( copy.GetTaskId() == future.GetTaskId() && CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( copy ) );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectoryAsync( CORE::CombinePath( dir, "d" ), false, future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ALREADY_EXISTS == AwaitResult( future ) );

            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "f.txt" ), "12345678" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetMetaDataAsync( CORE::CombinePath( dir, "f.txt" ), future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncMetaData( future, metaData ) && 8 == metaData.resourceSizeInBytes );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryTypeAsync( CORE::CombinePath( dir, "d" ), future ) );
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && CORE::IFileSystemAccess::TryGetAsyncEntryType( future, type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryRenameFileAsync( CORE::CombinePath( dir, "f.txt" ), "g.txt", future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && CORE::FileExists( CORE::CombinePath( dir, "g.txt" ) ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveFileAsync( CORE::CombinePath( dir, "g.txt" ), CORE::CombinePath( dir, "d/g.txt" ), false, future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) );

            CORE::CString::StringVector files;
            files.push_back( CORE::CombinePath( dir, "d/g.txt" ) );
            files.push_back( CORE::CombinePath( dir, "nothing.txt" ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveFilesAsync( files, dir, false, future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS == AwaitResult( future ) );
            CORE::IFileSystemAccess::TFileSystemAccessResultVector perFile;
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncPerFileResults( future, perFile ) && 2 == perFile.size() && CORE::FILESYSTEMACCESS_NOT_FOUND == perFile[ 1 ] );

            // the listing comes back read: an iterator over entries in memory
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIteratorAsync( dir, future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) );
            CORE::TFileSystemIteratorPtr iterator;
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncIterator( future, iterator ) && !iterator.IsNULL() );
        #ifdef GUCEF_RTTI_IS_SUPPORTED
            ASSERT_TRUE( GUCEF_NULL != dynamic_cast< CORE::CFileSystemEntryListIterator* >( iterator.GetPointerAlways() ) );
        #endif
            CORE::CString::StringSet names = ListNames( *iterator, dir );
            ASSERT_TRUE( 2 == names.size() && names.count( "d/" ) && names.count( "g.txt" ) );

            // it is an iterator over entries in memory: what is made on disk after the listing does not show up in it
            const CORE::CString lateFile = CORE::CombinePath( dir, "late.txt" );
            ASSERT_TRUE( WriteFile( lateFile, "late" ) );
            CORE::CString::StringSet snapshotNames = ListNames( *iterator, dir );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteFile( lateFile ) );
            ASSERT_TRUE( 2 == snapshotNames.size() && 0 == snapshotNames.count( "late.txt" ) );

            // a future that carries other data than a file system access result is not one
            CORE::CFutureResultSource foreign;
            CORE::CFutureResult foreignFuture = foreign.GetFuture();
            ASSERT_TRUE( foreign.Complete( GUCEF_NEW CORE::TCloneableString( "something else" ), true ) );
            CORE::CResourceMetaData foreignMeta;
            CORE::TFileSystemEntryType foreignType = CORE::FILESYSTEMENTRYTYPE_NONE;
            CORE::TFileSystemIteratorPtr foreignIterator;
            CORE::IFileSystemAccess::TFileSystemAccessResultVector foreignResults;
            CORE::TFileSystemAccessResult foreignResult = CORE::FILESYSTEMACCESS_SUCCESS;
            ASSERT_FALSE( CORE::IFileSystemAccess::TryGetAsyncMetaData( foreignFuture, foreignMeta ) );
            ASSERT_FALSE( CORE::IFileSystemAccess::TryGetAsyncEntryType( foreignFuture, foreignType ) );
            ASSERT_FALSE( CORE::IFileSystemAccess::TryGetAsyncIterator( foreignFuture, foreignIterator ) );
            ASSERT_FALSE( CORE::IFileSystemAccess::TryGetAsyncPerFileResults( foreignFuture, foreignResults ) );
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncResult( foreignFuture, foreignResult ) && CORE::FILESYSTEMACCESS_FAILED == foreignResult );

            ASSERT_TRUE( iterator->FindFirst( dir ) );
            while ( "g.txt" != iterator->GetResourceName() && iterator->FindNext() ) {}
            ASSERT_TRUE( iterator->TryReadMetaData( metaData ) && 8 == metaData.resourceSizeInBytes );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIteratorAsync( CORE::CombinePath( dir, "nowhere" ), future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == AwaitResult( future ) && !CORE::IFileSystemAccess::TryGetAsyncIterator( future, iterator ) );

            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteFileAsync( CORE::CombinePath( dir, "g.txt" ), future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryMoveDirectoryAsync( CORE::CombinePath( dir, "d" ), CORE::CombinePath( dir, "e" ), future ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && CORE::DirExists( CORE::CombinePath( dir, "e" ) ) );

            // the provider lives as long as a queued operation needs it
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteDirectoryAsync( dir, true, future ) );
            access.Unlink();
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && !CORE::DirExists( dir ) );

            // the future of an operation that could not be queued
            CORE::TFileSystemAccessResult neverQueued = CORE::FILESYSTEMACCESS_SUCCESS;
            ASSERT_TRUE( CORE::IFileSystemAccess::TryGetAsyncResult( CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED ), neverQueued ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_QUEUEING_FAILED == neverQueued );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: On a pool without worker threads (a build without threads) an async operation runs when the caller pitches in" )
        try
        {
            const CORE::CString poolName = "gucefCORE_TestApp.NoWorkers";
            CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
            CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( poolName );
            ASSERT_TRUE( !pool.IsNULL() );
            pool->SetDesiredMaxTotalNrOfThreads( 0 );
            ASSERT_TRUE( 0 == pool->GetActiveNrOfThreads() );

            const CORE::CString dir = MakeTestDir( "noworkers" );
            CORE::TFileSystemAccessPtr access = ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryCreateDirectoryAsync( CORE::CombinePath( dir, "d" ), false, future, poolName ) );
            ASSERT_TRUE( !future.IsReady() && 1 == pool->GetNrOfQueuedTasks() );

            // the calling thread does the task
            ASSERT_TRUE( 1 == pool->CarryOutQueuedTasksIfAny( 1 ) );
            ASSERT_TRUE( future.IsReady() && CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && CORE::DirExists( CORE::CombinePath( dir, "d" ) ) );
            ASSERT_TRUE( 0 == pool->CarryOutQueuedTasksIfAny( 1 ) );

            // and again, the pool keeps working that way
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryDeleteDirectoryAsync( dir, true, future, poolName ) );
            ASSERT_TRUE( 1 == pool->CarryOutQueuedTasksIfAny( 5 ) );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == AwaitResult( future ) && !CORE::DirExists( dir ) );
            taskManager.UnregisterThreadPool( poolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileSystemAccess: The entry list iterator gives the entries it was given, and reads another iterator whole" )
        try
        {
            CORE::CFileSystemEntryListIterator::TEntryVector entries;
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = "x";
            entry.path = "/v/x";
            entry.isFile = true;
            entries.push_back( entry );
            entry.name = "y";
            entry.path = "/v/y";
            entry.isFile = false;
            entry.isDirectory = true;
            entries.push_back( entry );
            CORE::CFileSystemEntryListIterator list( "/v", entries );
            ASSERT_TRUE( "/v" == list.GetDirPath() && 2 == list.GetEntries().size() );
            ASSERT_FALSE( list.IsAFile() );                                                         // not positioned yet
            ASSERT_TRUE( list.FindFirst( "/v" ) && "x" == list.GetResourceName() && "/v/x" == list.GetResourcePath() && list.IsAFile() );
            CORE::CResourceMetaData metaData;
            ASSERT_FALSE( list.TryReadMetaData( metaData ) );
            ASSERT_TRUE( list.FindNext() && "y" == list.GetResourceName() && list.IsADirectory() && !list.IsAFile() );
            ASSERT_FALSE( list.FindNext() );
            ASSERT_TRUE( list.FindClose() && list.GetResourceName().IsNULLOrEmpty() );
            CORE::CFileSystemEntryListIterator empty;
            ASSERT_FALSE( empty.FindFirst( "/" ) );

            const CORE::CString dir = MakeTestDir( "readall" );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "f" ), "abc" ) && CORE::CreateDirs( CORE::CombinePath( dir, "s" ) ) );
            CORE::CFileSystemIterator source;
            ASSERT_TRUE( CORE::CFileSystemEntryListIterator::TryReadAll( source, dir, entries ) && 2 == entries.size() );
            for ( size_t i=0; i<entries.size(); ++i )
            {
                if ( "f" == entries[ i ].name )
                    ASSERT_TRUE( entries[ i ].isFile && entries[ i ].hasMetaData && 3 == entries[ i ].metaData.resourceSizeInBytes );
                else
                    ASSERT_TRUE( "s" == entries[ i ].name && entries[ i ].isDirectory );
            }
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

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING FileSystemAccess TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "FileSystemAccess" );

    PerformResultCodeTests();
    PerformSpecialDirTests();
    PerformGlobPatternTests();
    PerformNativeFileSystemAccessTests();
}

/*-------------------------------------------------------------------------*/
