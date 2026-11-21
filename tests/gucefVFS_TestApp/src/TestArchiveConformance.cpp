/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include <string.h>
#include <stdlib.h>
#include <algorithm>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_MD5_H
#include "md5.h"
#define GUCEF_CORE_MD5_H
#endif /* GUCEF_CORE_MD5_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestArchiveConformance.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CArchiveConformanceTree::SEntry     TEntry;
typedef CArchiveConformanceTree::TStringVector TStringVector;

/** Do not keep any file in memory: every read goes to the storage */
const UInt32 NoMemoryLoad = 0;

/** Keep every file of the tree in memory (the largest of them is a few MiB) */
const UInt32 AllInMemory = 16 * 1048576;

/** The location under which the write cases work. It is not part of the tree and removed again */
const char* const WriteArea = "conformance_write";

/** The location the lists use as the mount location, to see where it ends up in the entries */
const char* const MountLocation = "mnt";

/** What a case needs to know about the fixture */
struct SSuiteContext
{
    CArchiveConformanceFixture* fixture;
    VFS::CArchive* archive;
    const CArchiveConformanceTree* tree;
    const CArchiveConformanceCapabilities* caps;
};

/*-------------------------------------------------------------------------*/

/** A file opened through the archive: the VFS handle that keeps the access valid, and the access */
class COpenFile
{
    public:

    VFS::TBasicVfsResourcePtr resource;
    CORE::IOAccessPtr access;

    bool Open( VFS::CArchive& archive, const CORE::CString& path, UInt32 memLoadSize = NoMemoryLoad, const char* mode = "rb", bool overwrite = false )
    {GUCEF_TRACE;

        Release();
        resource = archive.GetFile( path, mode, memLoadSize, overwrite );
        if ( resource.IsNULL() )
            return false;
        access = resource->GetAccess();
        return !access.IsNULL();
    }

    void Release( void )
    {GUCEF_TRACE;

        access.Unlink();
        resource.Unlink();
    }
};

/*-------------------------------------------------------------------------*/

CORE::CString
JoinPath( const char* dir, const CORE::CString& name )
{GUCEF_TRACE;

    return CORE::CString( dir ) + "/" + name;
}

/*-------------------------------------------------------------------------*/

/** Entries as lists give them, brought to one form to compare: '/' separators and no trailing separator, sorted */
TStringVector
Normalized( const TStringVector& list )
{GUCEF_TRACE;

    TStringVector result;
    for ( TStringVector::const_iterator i = list.begin(); i != list.end(); ++i )
    {
        CORE::CString entry = (*i).ReplaceChar( '\\', '/' );
        while ( entry.Length() > 0 && '/' == entry[ entry.Length()-1 ] )
            entry = entry.SubstrToIndex( entry.Length()-1, true );
        result.push_back( entry );
    }
    std::sort( result.begin(), result.end() );
    return result;
}

/*-------------------------------------------------------------------------*/

/** The last segment of every entry, sorted */
TStringVector
LeafNames( const TStringVector& paths )
{GUCEF_TRACE;

    TStringVector result;
    for ( TStringVector::const_iterator i = paths.begin(); i != paths.end(); ++i )
        result.push_back( CArchiveConformanceTree::LeafName( (*i) ) );
    std::sort( result.begin(), result.end() );
    return result;
}

/*-------------------------------------------------------------------------*/

/** Whether two sorted lists are equal, logs the first differences when not */
bool
SameList( const char* what, const TStringVector& expected, const TStringVector& actual )
{GUCEF_TRACE;

    if ( expected == actual )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << what << ": expected " << (UInt32) expected.size() << " entries, got " << (UInt32) actual.size() );
    UInt32 shown = 0;
    for ( TStringVector::const_iterator i = expected.begin(); i != expected.end() && shown < 5; ++i )
    {
        if ( !std::binary_search( actual.begin(), actual.end(), (*i) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << what << ": missing: \"" << (*i) << "\"" );
            ++shown;
        }
    }
    shown = 0;
    for ( TStringVector::const_iterator i = actual.begin(); i != actual.end() && shown < 5; ++i )
    {
        if ( !std::binary_search( expected.begin(), expected.end(), (*i) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << what << ": unexpected: \"" << (*i) << "\"" );
            ++shown;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const TStringVector& sortedList, const CORE::CString& entry )
{GUCEF_TRACE;

    return std::binary_search( sortedList.begin(), sortedList.end(), entry );
}

/*-------------------------------------------------------------------------*/

bool
IsPlainAscii( const CORE::CString& text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        if ( (UInt8) text[ i ] >= 0x80 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads exactly 'count' bytes from the current position of the access and checks them against the content of the entry */
bool
ReadAndCheck( CORE::CIOAccess& access, const TEntry& entry, UInt64 offset, UInt32 count, const char* what )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer( count > 0 ? count : 1, true );
    const UInt32 got = access.Read( buffer.GetBufferPtr(), 1, count );
    if ( got != count )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << what << ": " << entry.path << ": read of " << count << " bytes at " << offset << " gave " << got );
        return false;
    }
    UInt64 mismatch = 0;
    if ( !CArchiveConformanceTree::CheckEntryContent( entry, offset, buffer.GetConstBufferPtr(), count, &mismatch ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << what << ": " << entry.path << ": content differs at position " << mismatch );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads a file from start to end in chunks of an awkward size and checks size and content; the chunk size is no divisor of the sizes of the tree */
bool
ReadWholeAndCheck( VFS::CArchive& archive, const TEntry& entry, UInt32 memLoadSize, UInt32 chunkSize )
{GUCEF_TRACE;

    COpenFile file;
    if ( !file.Open( archive, entry.path, memLoadSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: ReadWhole: " << entry.path << ": GetFile gave nothing" );
        return false;
    }
    if ( !file.access->IsReadable() || file.access->GetSize() != entry.size )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: ReadWhole: " << entry.path << ": not readable or size " << file.access->GetSize() << " instead of " << entry.size );
        return false;
    }

    CORE::CDynamicBuffer chunk( chunkSize, true );
    UInt64 position = 0;
    while ( true )
    {
        const UInt32 got = file.access->Read( chunk.GetBufferPtr(), 1, chunkSize );
        if ( 0 == got )
            break;
        UInt64 mismatch = 0;
        if ( !CArchiveConformanceTree::CheckEntryContent( entry, position, chunk.GetConstBufferPtr(), got, &mismatch ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: ReadWhole: " << entry.path << ": content differs at position " << mismatch );
            return false;
        }
        position += got;
        if ( position > entry.size )
            break;
    }
    if ( position != entry.size )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: ReadWhole: " << entry.path << ": read " << position << " bytes instead of " << entry.size );
        return false;
    }
    return file.access->Tell() == entry.size;
}

/*-------------------------------------------------------------------------*/

/** Whether the content of the file in the archive is exactly the given buffer */
bool
FileHasContent( VFS::CArchive& archive, const CORE::CString& path, const CORE::CDynamicBuffer& expected, UInt32 memLoadSize )
{GUCEF_TRACE;

    COpenFile file;
    if ( !file.Open( archive, path, memLoadSize ) )
        return false;
    CORE::CDynamicBuffer actual;
    const UInt64 got = actual.Append( *file.access );
    if ( got != expected.GetDataSize() || actual.GetDataSize() != expected.GetDataSize() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << path << ": read " << got << " bytes, expected " << expected.GetDataSize() );
        return false;
    }
    return 0 == expected.GetDataSize() || 0 == memcmp( expected.GetConstBufferPtr(), actual.GetConstBufferPtr(), expected.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
MakeContent( UInt32 contentId, UInt32 size, CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    content.Clear( true );
    content.SetDataSize( size );
    if ( size > 0 )
        CArchiveConformanceTree::FillContent( contentId, 0, content.GetBufferPtr(), size );
}

/*-------------------------------------------------------------------------*/

/** The 48 character form of CORE::MD5ToString() (16 decimal numbers of 3 digits) as the 32 lower case hexadecimal characters of a digest */
CORE::CString
Md5StringAsHex( const CORE::CString& triplets )
{GUCEF_TRACE;

    CORE::CString hex;
    if ( 48 != triplets.Length() )
        return hex;
    for ( UInt32 i=0; i<16; ++i )
    {
        char group[ 4 ];
        memcpy( group, triplets.C_String() + i * 3, 3 );
        group[ 3 ] = '\0';
        const UInt8 byte = (UInt8) atoi( group );
        hex += CORE::Base16Encode( &byte, 1 ).Lowercase();
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

/** The MD5 of the content of an entry in the form of the archive hash, as the generator defines it */
CORE::CString
ExpectedHash( const TEntry& entry )
{GUCEF_TRACE;

    md5_state_t state;
    md5_init( &state );
    CORE::CDynamicBuffer chunk( 65536, true );
    UInt64 position = 0;
    while ( position < entry.size )
    {
        const UInt32 count = (UInt32) std::min< UInt64 >( 65536, entry.size - position );
        CArchiveConformanceTree::FillEntryContent( entry, position, chunk.GetBufferPtr(), count );
        md5_append( &state, static_cast< const md5_byte_t* >( chunk.GetConstBufferPtr() ), (int) count );
        position += count;
    }
    UInt8 digest[ 16 ];
    md5_finish( &state, digest );
    return CORE::MD5ToString( digest );
}

/*-------------------------------------------------------------------------*/

/** The entries of the tree that are files, without the file above 4 GiB which only the cases made for it touch */
TStringVector
AllFilePaths( const CArchiveConformanceTree& tree, bool withLargeFile )
{GUCEF_TRACE;

    TStringVector paths;
    tree.GetDescendantPaths( CORE::CString::Empty, true, true, false, paths );
    if ( !withLargeFile )
    {
        TStringVector::iterator i = paths.begin();
        while ( i != paths.end() )
        {
            if ( (*i) == CArchiveConformanceTree::LargeFile )
                i = paths.erase( i );
            else
                ++i;
        }
    }
    return paths;
}

/*-------------------------------------------------------------------------*/

/** The file lists of a location as the archive gives them, normalized */
TStringVector
ListFiles( const SSuiteContext& ctx, const CORE::CString& location, bool recursive, bool includePath, const CORE::CString::StringSet& filters = CORE::CString::EmptyStringSet, UInt32 maxEntries = GUCEF_UINT16MAX )
{GUCEF_TRACE;

    TStringVector list;
    ctx.archive->GetFileList( list, includePath ? CORE::CString( MountLocation ) : CORE::CString::Empty, location, recursive, includePath, filters, maxEntries );
    return Normalized( list );
}

/*-------------------------------------------------------------------------*/

TStringVector
ListDirs( const SSuiteContext& ctx, const CORE::CString& location, bool recursive, bool includePath, const CORE::CString::StringSet& filters = CORE::CString::EmptyStringSet, UInt32 maxEntries = GUCEF_UINT16MAX )
{GUCEF_TRACE;

    TStringVector list;
    ctx.archive->GetDirList( list, includePath ? CORE::CString( MountLocation ) : CORE::CString::Empty, location, recursive, includePath, filters, maxEntries );
    return Normalized( list );
}

/*-------------------------------------------------------------------------*/

/** What the file list of a location has to be: the names, or the paths with the mount location in front */
TStringVector
ExpectedFiles( const SSuiteContext& ctx, const CORE::CString& location, bool recursive, bool includePath )
{GUCEF_TRACE;

    TStringVector paths;
    ctx.tree->GetDescendantPaths( location, recursive, true, false, paths );
    TStringVector result;
    for ( TStringVector::const_iterator i = paths.begin(); i != paths.end(); ++i )
    {
        if ( (*i) == CArchiveConformanceTree::LargeFile && !ctx.tree->HasLargeFile() )
            continue;
        result.push_back( includePath ? CORE::CString( MountLocation ) + "/" + (*i) : CArchiveConformanceTree::LeafName( (*i) ) );
    }
    std::sort( result.begin(), result.end() );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
Filters( const char* a, const char* b = GUCEF_NULL )
{GUCEF_TRACE;

    CORE::CString::StringSet filters;
    filters.insert( a );
    if ( GUCEF_NULL != b )
        filters.insert( b );
    return filters;
}

/*-------------------------------------------------------------------------*/

/** Waits at least as long as the modification time of the archive can tell two versions of a file apart */
void
WaitForNextModificationTime( const SSuiteContext& ctx )
{GUCEF_TRACE;

    if ( ctx.caps->hasModificationTimes )
        MT::ThreadDelay( ctx.caps->modificationTimeResolutionInMs + 50 );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: THE READ SIDE                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Records a failure of an assertion that is about a path, with the path in the log */
#define CHECK_FOR( cond, what ) \
    do { const bool checkIsGood = ( cond ); if ( !checkIsGood ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: check failed: " #cond " for " << what ); } ASSERT_TRUE( checkIsGood ); } while (0)

/*-------------------------------------------------------------------------*/

void
TestExistenceAndSize( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    const CArchiveConformanceTree::TEntryVector& entries = ctx.tree->GetEntries();

    GUCEF_TESTFW_TESTCASE( "FileExists: every file of the tree exists and no directory is a file" )
    try
    {
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            if ( (*i).isDirectory )
            {
                CHECK_FOR( !archive.FileExists( (*i).path ), (*i).path );
            }
            else
            {
                CHECK_FOR( archive.FileExists( (*i).path ), (*i).path );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DirExists: every directory of the tree exists and no file is a directory" )
    try
    {
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            if ( (*i).isDirectory )
            {
                CHECK_FOR( archive.DirExists( (*i).path ), (*i).path );
            }
            else
            {
                CHECK_FOR( !archive.DirExists( (*i).path ), (*i).path );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileExists and DirExists: absent names, a path below a file, a file asked for as a directory and the other way around" )
    try
    {
        ASSERT_FALSE( archive.FileExists( "absent.bin" ) );
        ASSERT_FALSE( archive.FileExists( "nested/absent.bin" ) );
        ASSERT_FALSE( archive.FileExists( "absent_dir/absent.bin" ) );
        ASSERT_FALSE( archive.FileExists( "nested/level1/level2/leaf.bin.absent" ) );
        ASSERT_FALSE( archive.FileExists( "nested/level1/level2/leaf.bi" ) );
        ASSERT_FALSE( archive.DirExists( "absent_dir" ) );
        ASSERT_FALSE( archive.DirExists( "nested/absent_dir" ) );
        ASSERT_FALSE( archive.DirExists( "nested/lev" ) );

        // a name that is a directory is no file, a name that is a file is no directory, and nothing is below a file
        ASSERT_FALSE( archive.FileExists( "nested" ) );
        ASSERT_FALSE( archive.FileExists( CArchiveConformanceTree::EmptyDir ) );
        ASSERT_FALSE( archive.DirExists( CArchiveConformanceTree::OneByteFile ) );
        ASSERT_FALSE( archive.DirExists( CArchiveConformanceTree::EmptyFile ) );
        ASSERT_FALSE( archive.FileExists( CORE::CString( CArchiveConformanceTree::OneByteFile ) + "/child.bin" ) );
        ASSERT_FALSE( archive.DirExists( CORE::CString( CArchiveConformanceTree::OneByteFile ) + "/child" ) );
        ASSERT_FALSE( archive.FileExists( "nested/top.bin/child" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileSize: byte exact for every file, zero for what is not a file" )
    try
    {
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            if ( !(*i).isDirectory )
            {
                CHECK_FOR( archive.GetFileSize( (*i).path ) == (*i).size, (*i).path << ": " << archive.GetFileSize( (*i).path ) << " instead of " << (*i).size );
            }
            else
            {
                CHECK_FOR( 0 == archive.GetFileSize( (*i).path ), (*i).path );
            }
        }
        ASSERT_TRUE( 0 == archive.GetFileSize( "absent.bin" ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( "nested/absent.bin" ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( "absent_dir/absent.bin" ) );
        ASSERT_TRUE( 65535 == archive.GetFileSize( "size_65535.bin" ) );
        ASSERT_TRUE( 65536 == archive.GetFileSize( "size_65536.bin" ) );
        ASSERT_TRUE( 65537 == archive.GetFileSize( "size_65537.bin" ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( CArchiveConformanceTree::EmptyFile ) );
        ASSERT_TRUE( 1 == archive.GetFileSize( CArchiveConformanceTree::OneByteFile ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestNamesWithSpacesAndNonAscii( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    const CArchiveConformanceTree::TEntryVector& entries = ctx.tree->GetEntries();

    GUCEF_TESTFW_TESTCASE( "Names with spaces and non-ASCII characters (UTF-8): exist, have their size and content, and are listed with the exact name" )
    try
    {
        UInt32 checked = 0;
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            const TEntry& entry = (*i);
            if ( IsPlainAscii( entry.path ) && entry.path.HasChar( ' ' ) < 0 )
                continue;
            ++checked;

            const CORE::CString parent = CArchiveConformanceTree::ParentPath( entry.path );
            if ( entry.isDirectory )
            {
                CHECK_FOR( archive.DirExists( entry.path ), entry.path );
                TStringVector dirs = ListDirs( ctx, parent, false, false );
                CHECK_FOR( Contains( dirs, CArchiveConformanceTree::LeafName( entry.path ) ), entry.path );
            }
            else
            {
                CHECK_FOR( archive.FileExists( entry.path ), entry.path );
                CHECK_FOR( archive.GetFileSize( entry.path ) == entry.size, entry.path );
                CHECK_FOR( ReadWholeAndCheck( archive, entry, NoMemoryLoad, 4093 ), entry.path );
                CHECK_FOR( ReadWholeAndCheck( archive, entry, AllInMemory, 4093 ), entry.path );
                TStringVector files = ListFiles( ctx, parent, false, false );
                CHECK_FOR( Contains( files, CArchiveConformanceTree::LeafName( entry.path ) ), entry.path );
                TStringVector withPath = ListFiles( ctx, parent, false, true );
                CHECK_FOR( Contains( withPath, CORE::CString( MountLocation ) + "/" + entry.path ), entry.path );
            }
        }
        // spaces in a file name and in a directory name, and UTF-8 in both, at least
        ASSERT_TRUE( checked >= 5 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestFileLists( const SSuiteContext& ctx )
{GUCEF_TRACE;

    const CArchiveConformanceTree& tree = *ctx.tree;

    GUCEF_TESTFW_TESTCASE( "GetFileList: not recursive, for every directory of the tree, as names" )
    try
    {
        const CArchiveConformanceTree::TEntryVector& entries = tree.GetEntries();
        ASSERT_TRUE( SameList( "root", ExpectedFiles( ctx, CORE::CString::Empty, false, false ), ListFiles( ctx, CORE::CString::Empty, false, false ) ) );
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            if ( (*i).isDirectory )
            {
                ASSERT_TRUE( SameList( (*i).path.C_String(), ExpectedFiles( ctx, (*i).path, false, false ), ListFiles( ctx, (*i).path, false, false ) ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: recursive lists the files of every depth, as names without their directory" )
    try
    {
        ASSERT_TRUE( SameList( "root recursive", ExpectedFiles( ctx, CORE::CString::Empty, true, false ), ListFiles( ctx, CORE::CString::Empty, true, false ) ) );
        ASSERT_TRUE( SameList( "nested recursive", ExpectedFiles( ctx, "nested", true, false ), ListFiles( ctx, "nested", true, false ) ) );
        ASSERT_TRUE( SameList( "deep recursive", ExpectedFiles( ctx, "deep", true, false ), ListFiles( ctx, "deep", true, false ) ) );
        ASSERT_TRUE( SameList( "deep branch recursive", ExpectedFiles( ctx, "deep/d01/d02/d03", true, false ), ListFiles( ctx, "deep/d01/d02/d03", true, false ) ) );
        // the names say where they come from: the deep file is found by its name from the root
        TStringVector names = ListFiles( ctx, CORE::CString::Empty, true, false );
        ASSERT_TRUE( Contains( names, "deep.bin" ) );
        ASSERT_TRUE( Contains( names, "leaf.bin" ) );
        ASSERT_FALSE( Contains( names, "nested" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: with the path in the name the entries are the mount location and the path in the archive, at every depth" )
    try
    {
        // as verified in the reference archive: mountLocation + "/" + the path relative to the archive root, the
        // location asked for being a part of that path, with '/' as separator
        ASSERT_TRUE( SameList( "root path", ExpectedFiles( ctx, CORE::CString::Empty, false, true ), ListFiles( ctx, CORE::CString::Empty, false, true ) ) );
        ASSERT_TRUE( SameList( "nested path", ExpectedFiles( ctx, "nested", false, true ), ListFiles( ctx, "nested", false, true ) ) );
        ASSERT_TRUE( SameList( "nested recursive path", ExpectedFiles( ctx, "nested", true, true ), ListFiles( ctx, "nested", true, true ) ) );
        ASSERT_TRUE( SameList( "root recursive path", ExpectedFiles( ctx, CORE::CString::Empty, true, true ), ListFiles( ctx, CORE::CString::Empty, true, true ) ) );
        ASSERT_TRUE( SameList( "deep recursive path", ExpectedFiles( ctx, "deep", true, true ), ListFiles( ctx, "deep", true, true ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: a directory of several hundred files is listed whole, every name once" )
    try
    {
        TStringVector names = ListFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, false );
        ASSERT_TRUE( CArchiveConformanceTree::ManyFilesCount == names.size() );
        ASSERT_TRUE( SameList( "many", ExpectedFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, false ), names ) );
        TStringVector unique = names;
        unique.erase( std::unique( unique.begin(), unique.end() ), unique.end() );
        ASSERT_TRUE( unique.size() == names.size() );

        TStringVector withPath = ListFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, true );
        ASSERT_TRUE( CArchiveConformanceTree::ManyFilesCount == withPath.size() );

        // the largest number of entries a caller can ask for gives the same list
        TStringVector all = ListFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
        ASSERT_TRUE( SameList( "many with the largest limit", names, all ) );

        // all of the tree from the root in one list
        ASSERT_TRUE( AllFilePaths( tree, tree.HasLargeFile() ).size() == ListFiles( ctx, CORE::CString::Empty, true, false ).size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: maxListEntries limits the list" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->honoursMaxListEntries, "the archive does not limit its lists" );
    try
    {
        TStringVector ten = ListFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, false, CORE::CString::EmptyStringSet, 10 );
        ASSERT_TRUE( 10 == ten.size() );
        TStringVector one = ListFiles( ctx, CArchiveConformanceTree::ManyFilesDir, false, false, CORE::CString::EmptyStringSet, 1 );
        ASSERT_TRUE( 1 == one.size() );
        TStringVector fewer = ListFiles( ctx, "nested/level1", false, false, CORE::CString::EmptyStringSet, 10 );
        ASSERT_TRUE( 1 == fewer.size() );
        TStringVector recursive = ListFiles( ctx, CORE::CString::Empty, true, false, CORE::CString::EmptyStringSet, 25 );
        ASSERT_TRUE( 25 == recursive.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: name filters are patterns with wildcards, several filters match what any of them matches" )
    try
    {
        const CORE::CString dir = CArchiveConformanceTree::FilterDir;
        TStringVector expected;

        expected.clear();
        expected.push_back( "alpha.txt" );
        expected.push_back( "beta.txt" );
        ASSERT_TRUE( SameList( "*.txt", expected, ListFiles( ctx, dir, false, false, Filters( "*.txt" ) ) ) );

        expected.clear();
        expected.push_back( "delta.dat" );
        expected.push_back( "gamma.dat" );
        ASSERT_TRUE( SameList( "*.dat", expected, ListFiles( ctx, dir, false, false, Filters( "*.dat" ) ) ) );

        expected.clear();
        expected.push_back( "alpha.txt" );
        ASSERT_TRUE( SameList( "alpha*", expected, ListFiles( ctx, dir, false, false, Filters( "alpha*" ) ) ) );

        expected.clear();
        expected.push_back( "alpha.txt" );
        expected.push_back( "beta.txt" );
        expected.push_back( "gamma.dat" );
        ASSERT_TRUE( SameList( "*.txt and gamma*", expected, ListFiles( ctx, dir, false, false, Filters( "*.txt", "gamma*" ) ) ) );

        expected.clear();
        ASSERT_TRUE( SameList( "*.nomatch", expected, ListFiles( ctx, dir, false, false, Filters( "*.nomatch" ) ) ) );

        // no filter: all of them
        ASSERT_TRUE( SameList( "no filter", ExpectedFiles( ctx, dir, false, false ), ListFiles( ctx, dir, false, false ) ) );

        // the filter applies on every depth of a recursive list
        expected.clear();
        expected.push_back( "leaf.bin" );
        expected.push_back( "mid.bin" );
        expected.push_back( "top.bin" );
        ASSERT_TRUE( SameList( "recursive *.bin", expected, ListFiles( ctx, "nested", true, false, Filters( "*.bin" ) ) ) );

        // and on the entries with the path
        expected.clear();
        expected.push_back( CORE::CString( MountLocation ) + "/filter/alpha.txt" );
        expected.push_back( CORE::CString( MountLocation ) + "/filter/beta.txt" );
        ASSERT_TRUE( SameList( "path *.txt", expected, ListFiles( ctx, dir, false, true, Filters( "*.txt" ) ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList: an empty directory, an absent directory, a file and a path below a file list nothing" )
    try
    {
        ASSERT_TRUE( ListFiles( ctx, CArchiveConformanceTree::EmptyDir, false, false ).empty() );
        ASSERT_TRUE( ListFiles( ctx, CArchiveConformanceTree::EmptyDir, true, true ).empty() );
        ASSERT_TRUE( ListFiles( ctx, "absent_dir", false, false ).empty() );
        ASSERT_TRUE( ListFiles( ctx, "absent_dir", true, true ).empty() );
        ASSERT_TRUE( ListFiles( ctx, "nested/absent_dir", true, false ).empty() );
        ASSERT_TRUE( ListFiles( ctx, CArchiveConformanceTree::OneByteFile, false, false ).empty() );
        ASSERT_TRUE( ListFiles( ctx, CORE::CString( CArchiveConformanceTree::OneByteFile ) + "/below", true, false ).empty() );

        // the results are added to the list given, which the call does not clear
        TStringVector list;
        list.push_back( "kept" );
        ctx.archive->GetFileList( list, CORE::CString::Empty, "absent_dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        ASSERT_TRUE( 1 == list.size() );
        ctx.archive->GetFileList( list, CORE::CString::Empty, "filter", false, false, Filters( "alpha*" ), GUCEF_UINT16MAX );
        ASSERT_TRUE( 2 == list.size() );
        ASSERT_TRUE( "kept" == list[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileList and GetDirList: a location can be given with or without a leading and trailing separator" )
    try
    {
        const TStringVector rootFiles = ListFiles( ctx, CORE::CString::Empty, false, false );
        ASSERT_TRUE( SameList( "root as /", rootFiles, ListFiles( ctx, "/", false, false ) ) );

        const TStringVector nested = ListFiles( ctx, "nested", false, false );
        ASSERT_TRUE( 1 == nested.size() );
        ASSERT_TRUE( SameList( "/nested", nested, ListFiles( ctx, "/nested", false, false ) ) );
        ASSERT_TRUE( SameList( "nested/", nested, ListFiles( ctx, "nested/", false, false ) ) );
        ASSERT_TRUE( SameList( "/nested/", nested, ListFiles( ctx, "/nested/", false, false ) ) );

        const TStringVector rootDirs = ListDirs( ctx, CORE::CString::Empty, false, false );
        ASSERT_TRUE( SameList( "root dirs as /", rootDirs, ListDirs( ctx, "/", false, false ) ) );
        const TStringVector nestedDirs = ListDirs( ctx, "nested", false, false );
        ASSERT_TRUE( SameList( "/nested/ dirs", nestedDirs, ListDirs( ctx, "/nested/", false, false ) ) );

        // files and directories with the same forms
        ASSERT_TRUE( ctx.archive->FileExists( "/nested/top.bin" ) );
        ASSERT_TRUE( ctx.archive->DirExists( "/nested" ) );
        ASSERT_TRUE( ctx.archive->DirExists( "nested/" ) );
        ASSERT_TRUE( ctx.archive->DirExists( "/nested/level1/" ) );
        ASSERT_TRUE( 10 == ctx.archive->GetFileSize( "/nested/top.bin" ) );
        COpenFile file;
        ASSERT_TRUE( file.Open( *ctx.archive, "/nested/top.bin" ) );
        // the archive root is a directory
        ASSERT_TRUE( ctx.archive->DirExists( CORE::CString::Empty ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestDirLists( const SSuiteContext& ctx )
{GUCEF_TRACE;

    const CArchiveConformanceTree& tree = *ctx.tree;

    GUCEF_TESTFW_TESTCASE( "GetDirList: not recursive lists the direct subdirectories, for every directory of the tree" )
    try
    {
        const CArchiveConformanceTree::TEntryVector& entries = tree.GetEntries();
        for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
        {
            if ( !(*i).isDirectory )
                continue;
            TStringVector expected;
            tree.GetDescendantPaths( (*i).path, false, false, true, expected );
            ASSERT_TRUE( SameList( (*i).path.C_String(), LeafNames( expected ), ListDirs( ctx, (*i).path, false, false ) ) );
        }
        TStringVector rootExpected;
        tree.GetDescendantPaths( CORE::CString::Empty, false, false, true, rootExpected );
        ASSERT_TRUE( SameList( "root", LeafNames( rootExpected ), ListDirs( ctx, CORE::CString::Empty, false, false ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetDirList: the entries end with a path separator when the archive says so, and are never files" )
    try
    {
        TStringVector raw;
        ctx.archive->GetDirList( raw, CORE::CString::Empty, CORE::CString::Empty, false, false, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        ASSERT_TRUE( !raw.empty() );
        for ( TStringVector::const_iterator i = raw.begin(); i != raw.end(); ++i )
        {
            const bool endsWithSeparator = (*i).Length() > 0 && ( '/' == (*i)[ (*i).Length()-1 ] || '\\' == (*i)[ (*i).Length()-1 ] );
            CHECK_FOR( endsWithSeparator == ctx.caps->dirEntriesHaveTrailingSeparator, (*i) );
            CHECK_FOR( ctx.archive->DirExists( (*i) ), (*i) );
        }
        // for the file list the entries have no separator at the end
        TStringVector files;
        ctx.archive->GetFileList( files, CORE::CString::Empty, "nested", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        ASSERT_TRUE( 1 == files.size() );
        ASSERT_TRUE( "top.bin" == files[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetDirList: recursive lists the directories of every depth, and no files" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->dirListRecursesIntoSubDirs, "the archive lists the first level only in a recursive directory list" );
    try
    {
        TStringVector expected;
        tree.GetDescendantPaths( CORE::CString::Empty, true, false, true, expected );
        ASSERT_TRUE( SameList( "root recursive", LeafNames( expected ), LeafNames( ListDirs( ctx, CORE::CString::Empty, true, false ) ) ) );

        expected.clear();
        tree.GetDescendantPaths( "nested", true, false, true, expected );
        ASSERT_TRUE( SameList( "nested recursive", LeafNames( expected ), LeafNames( ListDirs( ctx, "nested", true, false ) ) ) );

        // below a location the entries are relative to that location (the list is normalised: no separator at the end)
        TStringVector below = ListDirs( ctx, "nested", true, false );
        ASSERT_TRUE( Contains( below, "level1" ) );
        ASSERT_TRUE( Contains( below, "level1/level2" ) );

        // no file name is among the directories
        TStringVector all = ListDirs( ctx, CORE::CString::Empty, true, false );
        ASSERT_FALSE( Contains( all, "top.bin" ) );
        ASSERT_FALSE( Contains( all, "deep.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetDirList: with the parent directory in the name the entries are the mount location and the path in the archive" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->dirListHonoursIncludePath, "the archive does not put the path in the directory entries" );
    try
    {
        TStringVector expected;
        tree.GetDescendantPaths( "nested", false, false, true, expected );
        TStringVector withMount;
        for ( TStringVector::const_iterator i = expected.begin(); i != expected.end(); ++i )
            withMount.push_back( CORE::CString( MountLocation ) + "/" + (*i) );
        ASSERT_TRUE( SameList( "nested with path", withMount, ListDirs( ctx, "nested", false, true ) ) );

        if ( ctx.caps->dirListRecursesIntoSubDirs )
        {
            expected.clear();
            withMount.clear();
            tree.GetDescendantPaths( "nested", true, false, true, expected );
            for ( TStringVector::const_iterator i = expected.begin(); i != expected.end(); ++i )
                withMount.push_back( CORE::CString( MountLocation ) + "/" + (*i) );
            ASSERT_TRUE( SameList( "nested recursive with path", withMount, ListDirs( ctx, "nested", true, true ) ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetDirList: name filters apply to the directory names, an empty directory and a file list nothing" )
    try
    {
        // no other directory name has these letters in it, so the pattern matches the same names whether it is matched on
        // the whole name or as a part of it
        TStringVector filtered = ListDirs( ctx, CORE::CString::Empty, false, false, Filters( "dir w*" ) );
        ASSERT_TRUE( 1 == filtered.size() );
        ASSERT_TRUE( Contains( filtered, CArchiveConformanceTree::SpacesDir ) );
        filtered = ListDirs( ctx, CORE::CString::Empty, false, false, Filters( "dup*" ) );
        ASSERT_TRUE( 1 == filtered.size() );
        ASSERT_TRUE( Contains( filtered, CArchiveConformanceTree::DupDir ) );
        ASSERT_TRUE( ListDirs( ctx, CORE::CString::Empty, false, false, Filters( "*.nomatch" ) ).empty() );

        ASSERT_TRUE( ListDirs( ctx, CArchiveConformanceTree::EmptyDir, false, false ).empty() );
        ASSERT_TRUE( ListDirs( ctx, "absent_dir", false, false ).empty() );
        ASSERT_TRUE( ListDirs( ctx, CArchiveConformanceTree::OneByteFile, false, false ).empty() );
        ASSERT_TRUE( ListDirs( ctx, "deep/d01/d02/d03/d04/d05/d06/d07/d08/d09/d10/d11/d12/d13/d14/d15/d16", false, false ).empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetDirList: maxListEntries limits the list" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->honoursMaxListEntries, "the archive does not limit its lists" );
    try
    {
        TStringVector two = ListDirs( ctx, CORE::CString::Empty, false, false, CORE::CString::EmptyStringSet, 2 );
        ASSERT_TRUE( 2 == two.size() );
        TStringVector one = ListDirs( ctx, CORE::CString::Empty, false, false, CORE::CString::EmptyStringSet, 1 );
        ASSERT_TRUE( 1 == one.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: READING FILES                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** One range of a file to read */
struct SRange
{
    const char* path;
    UInt64 offset;
    UInt32 length;
};

/*-------------------------------------------------------------------------*/

/** A deterministic sequence of numbers for the ranges of the random reads */
UInt32
NextRandom( UInt32& state )
{
    state = state * 1664525U + 1013904223U;
    return state >> 8;
}

/*-------------------------------------------------------------------------*/

/** Reads ranges at random places of a file through one access, in any order, and checks each */
bool
ReadRandomRanges( VFS::CArchive& archive, const TEntry& entry, UInt32 memLoadSize, UInt32 seed, UInt32 rangeCount )
{GUCEF_TRACE;

    COpenFile file;
    if ( !file.Open( archive, entry.path, memLoadSize ) )
        return false;

    UInt32 state = seed;
    bool allGood = true;
    for ( UInt32 i=0; i<rangeCount; ++i )
    {
        const UInt64 offset = NextRandom( state ) % entry.size;
        const UInt32 maxLength = (UInt32) std::min< UInt64 >( entry.size - offset, 70000 );
        const UInt32 length = 1 + NextRandom( state ) % maxLength;
        if ( 0 != file.access->Setpos( offset ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: RandomRanges: " << entry.path << ": Setpos( " << offset << " ) failed" );
            allGood = false;
            continue;
        }
        if ( !ReadAndCheck( *file.access, entry, offset, length, "RandomRanges" ) || file.access->Tell() != offset + length )
            allGood = false;
    }
    return allGood;
}

/*-------------------------------------------------------------------------*/

void
TestReadingFiles( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    const CArchiveConformanceTree& tree = *ctx.tree;
    const TStringVector files = AllFilePaths( tree, false );

    GUCEF_TESTFW_TESTCASE( "GetFile: the content of every file is read byte exact from start to end, with and without the file kept in memory" )
    try
    {
        for ( TStringVector::const_iterator i = files.begin(); i != files.end(); ++i )
        {
            const TEntry* entry = tree.FindEntry( (*i) );
            ASSERT_TRUE( GUCEF_NULL != entry );
            if ( GUCEF_NULL == entry )
                continue;
            // chunk sizes that divide none of the sizes of the tree
            CHECK_FOR( ReadWholeAndCheck( archive, *entry, NoMemoryLoad, 8191 ), (*i) );
            CHECK_FOR( ReadWholeAndCheck( archive, *entry, AllInMemory, 65521 ), (*i) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: one read of a whole file at once gives all of it" )
    try
    {
        const char* const names[] = { "one.bin", "size_65535.bin", "size_65536.bin", "size_65537.bin", "big.bin", "nested/level1/level2/leaf.bin" };
        for ( UInt32 n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
        {
            const TEntry* entry = tree.FindEntry( names[ n ] );
            ASSERT_TRUE( GUCEF_NULL != entry );
            if ( GUCEF_NULL == entry )
                continue;
            COpenFile file;
            ASSERT_TRUE( file.Open( archive, entry->path ) );
            ASSERT_TRUE( ReadAndCheck( *file.access, *entry, 0, (UInt32) entry->size, "AtOnce" ) );
            ASSERT_TRUE( entry->size == file.access->Tell() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: the handle of a file for reading is readable, valid, not writable (when the archive says so) and starts at the beginning" )
    try
    {
        const char* const names[] = { "empty.bin", "one.bin", "size_65536.bin", "big.bin" };
        for ( UInt32 mem=0; mem<2; ++mem )
        {
            for ( UInt32 n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
            {
                const TEntry* entry = tree.FindEntry( names[ n ] );
                ASSERT_TRUE( GUCEF_NULL != entry );
                if ( GUCEF_NULL == entry )
                    continue;
                COpenFile file;
                ASSERT_TRUE( file.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                ASSERT_TRUE( file.access->IsReadable() );
                ASSERT_TRUE( file.access->IsValid() );
                if ( ctx.caps->readHandlesRefuseWrites )
                {
                    ASSERT_FALSE( file.access->IsWriteable() );
                }
                ASSERT_TRUE( 0 == file.access->Tell() );
                ASSERT_TRUE( entry->size == file.access->GetSize() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: a write through a handle opened for reading changes nothing for the handles open at once or opened later (when the archive says so)" )
    try
    {
        if ( ctx.caps->readHandlesRefuseWrites )
        {
            const TEntry* entry = tree.FindEntry( "one.bin" );
            ASSERT_TRUE( GUCEF_NULL != entry );
            if ( GUCEF_NULL != entry )
            {
                for ( UInt32 mem=0; mem<2; ++mem )
                {
                    COpenFile first;
                    COpenFile second;
                    ASSERT_TRUE( first.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                    ASSERT_TRUE( second.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );

                    // the write may be refused or ignored; what is not allowed is a change in what anyone reads
                    const char poison[ 4 ] = { 'Z', 'Z', 'Z', 'Z' };
                    first.access->Write( poison, 1, 1 );

                    ASSERT_TRUE( ReadAndCheck( *second.access, *entry, 0, (UInt32) entry->size, "ReadWhileOtherWrote" ) );
                    first.Release();
                    second.Release();

                    COpenFile later;
                    ASSERT_TRUE( later.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                    ASSERT_TRUE( ReadAndCheck( *later.access, *entry, 0, (UInt32) entry->size, "ReadAfterOtherWrote" ) );
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: ranged reads at the edges of the 64 KiB blocks and at the ends of the files" )
    try
    {
        const SRange ranges[] =
        {
            { "one.bin", 0, 1 },
            { "size_65535.bin", 0, 65535 },
            { "size_65535.bin", 65534, 1 },
            { "size_65535.bin", 1, 65534 },
            { "size_65536.bin", 0, 65536 },
            { "size_65536.bin", 65535, 1 },
            { "size_65536.bin", 65535 - 100, 101 },
            { "size_65537.bin", 65535, 2 },
            { "size_65537.bin", 65536, 1 },
            { "size_65537.bin", 65537 - 1, 1 },
            { "size_65537.bin", 1, 65536 },
            { "big.bin", 0, 1 },
            { "big.bin", 65535, 2 },
            { "big.bin", 65536, 65536 },
            { "big.bin", 65536 * 7 - 5, 10 },
            { "big.bin", 1048576, 1 },
            { "big.bin", 1048576 - 1, 3 },
            { "big.bin", 3 * 1048576 + 12345 - 1, 1 },
            { "big.bin", 3 * 1048576 + 12345 - 1000, 1000 },
            { "big.bin", 2000000, 123457 }
        };
        for ( UInt32 mem=0; mem<2; ++mem )
        {
            for ( UInt32 r=0; r<sizeof( ranges ) / sizeof( ranges[ 0 ] ); ++r )
            {
                const TEntry* entry = tree.FindEntry( ranges[ r ].path );
                ASSERT_TRUE( GUCEF_NULL != entry );
                if ( GUCEF_NULL == entry )
                    continue;
                COpenFile file;
                ASSERT_TRUE( file.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                // Setpos() answers like fsetpos(): zero is success
                ASSERT_TRUE( 0 == file.access->Setpos( ranges[ r ].offset ) );
                ASSERT_TRUE( ranges[ r ].offset == file.access->Tell() );
                CHECK_FOR( ReadAndCheck( *file.access, *entry, ranges[ r ].offset, ranges[ r ].length, "Ranges" ), entry->path << " at " << ranges[ r ].offset );
                ASSERT_TRUE( ranges[ r ].offset + ranges[ r ].length == file.access->Tell() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: many reads at random places of one file, in any order, through one access" )
    try
    {
        const TEntry* big = tree.FindEntry( CArchiveConformanceTree::BigFile );
        const TEntry* mid = tree.FindEntry( "size_65537.bin" );
        ASSERT_TRUE( GUCEF_NULL != big && GUCEF_NULL != mid );
        if ( GUCEF_NULL != big && GUCEF_NULL != mid )
        {
            ASSERT_TRUE( ReadRandomRanges( archive, *big, NoMemoryLoad, 12345, 120 ) );
            ASSERT_TRUE( ReadRandomRanges( archive, *big, AllInMemory, 54321, 120 ) );
            ASSERT_TRUE( ReadRandomRanges( archive, *mid, NoMemoryLoad, 777, 60 ) );
            ASSERT_TRUE( ReadRandomRanges( archive, *mid, AllInMemory, 888, 60 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: Seek from the start, the current position and the end, Tell, GetChar and a seek before the start" )
    try
    {
        const TEntry* entry = tree.FindEntry( "size_65537.bin" );
        ASSERT_TRUE( GUCEF_NULL != entry );
        if ( GUCEF_NULL != entry )
        {
            for ( UInt32 mem=0; mem<2; ++mem )
            {
                COpenFile file;
                ASSERT_TRUE( file.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                CORE::CIOAccess& access = *file.access;

                ASSERT_TRUE( 0 == access.Seek( 100, SEEK_SET ) );
                ASSERT_TRUE( 100 == access.Tell() );
                ASSERT_TRUE( 0 == access.Seek( 50, SEEK_CUR ) );
                ASSERT_TRUE( 150 == access.Tell() );
                ASSERT_TRUE( 0 == access.Seek( -10, SEEK_CUR ) );
                ASSERT_TRUE( 140 == access.Tell() );
                ASSERT_TRUE( ReadAndCheck( access, *entry, 140, 20, "SeekCur" ) );
                ASSERT_TRUE( 160 == access.Tell() );

                ASSERT_TRUE( 0 == access.Seek( -10, SEEK_END ) );
                ASSERT_TRUE( entry->size - 10 == access.Tell() );
                ASSERT_TRUE( ReadAndCheck( access, *entry, entry->size - 10, 10, "SeekEnd" ) );
                ASSERT_TRUE( 0 == access.Seek( 0, SEEK_SET ) );
                ASSERT_TRUE( 0 == access.Tell() );

                // single characters from a position
                ASSERT_TRUE( 0 == access.Setpos( 12345 ) );
                ASSERT_TRUE( (char) CArchiveConformanceTree::ContentByte( entry->contentId, 12345 ) == access.GetChar() );
                ASSERT_TRUE( (char) CArchiveConformanceTree::ContentByte( entry->contentId, 12346 ) == access.GetChar() );
                ASSERT_TRUE( 12347 == access.Tell() );

                // before the start of the file is no place to be
                ASSERT_FALSE( 0 == access.Seek( -1, SEEK_SET ) );
                ASSERT_FALSE( 0 == access.Seek( -(Int64) entry->size - 1, SEEK_END ) );
                ASSERT_FALSE( 0 == access.Seek( -100000, SEEK_CUR ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: a read past the end gives what is there and then nothing, and Eof shows after it" )
    try
    {
        const TEntry* entry = tree.FindEntry( "nested/level1/mid.bin" );
        ASSERT_TRUE( GUCEF_NULL != entry && 300 == entry->size );
        if ( GUCEF_NULL != entry )
        {
            for ( UInt32 mem=0; mem<2; ++mem )
            {
                COpenFile file;
                ASSERT_TRUE( file.Open( archive, entry->path, 0 == mem ? NoMemoryLoad : AllInMemory ) );
                CORE::CIOAccess& access = *file.access;
                CORE::CDynamicBuffer buffer( 4000, true );

                // more asked than there is: the part that is there, then the end
                ASSERT_TRUE( 300 == access.Read( buffer.GetBufferPtr(), 1, 1000 ) );
                ASSERT_TRUE( CArchiveConformanceTree::CheckEntryContent( *entry, 0, buffer.GetConstBufferPtr(), 300 ) );
                ASSERT_TRUE( access.Eof() );
                ASSERT_TRUE( 300 == access.Tell() );
                ASSERT_TRUE( 0 == access.Read( buffer.GetBufferPtr(), 1, 1000 ) );
                ASSERT_TRUE( 0 == access.Read( buffer.GetBufferPtr(), 1, 1 ) );
                ASSERT_TRUE( access.Eof() );
                ASSERT_TRUE( 300 == access.Tell() );

                // a start in the middle that runs over the end
                ASSERT_TRUE( 0 == access.Setpos( 250 ) );
                ASSERT_TRUE( 50 == access.Read( buffer.GetBufferPtr(), 1, 100 ) );
                ASSERT_TRUE( CArchiveConformanceTree::CheckEntryContent( *entry, 250, buffer.GetConstBufferPtr(), 50 ) );
                ASSERT_TRUE( access.Eof() );

                // at the end exactly, and beyond it where the archive allows to be
                ASSERT_TRUE( 0 == access.Setpos( 300 ) );
                ASSERT_TRUE( 300 == access.Tell() );
                ASSERT_TRUE( 0 == access.Read( buffer.GetBufferPtr(), 1, 10 ) );
                if ( 0 == access.Setpos( 5000 ) )
                {
                    ASSERT_TRUE( 0 == access.Read( buffer.GetBufferPtr(), 1, 10 ) );
                }

                // positions are usable again after the end was reached
                ASSERT_TRUE( 0 == access.Setpos( 0 ) );
                ASSERT_TRUE( ReadAndCheck( access, *entry, 0, 100, "AfterEnd" ) );

                // elements of more than a byte: whole elements only
                ASSERT_TRUE( 0 == access.Setpos( 0 ) );
                ASSERT_TRUE( 75 == access.Read( buffer.GetBufferPtr(), 4, 100 ) );
                ASSERT_TRUE( 300 == access.Tell() );
                ASSERT_TRUE( 0 == access.Setpos( 0 ) );
                ASSERT_TRUE( 0 == access.Read( buffer.GetBufferPtr(), 1, 0 ) );
                ASSERT_TRUE( 0 == access.Tell() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: a file of no bytes gives no bytes and the end at once" )
    try
    {
        const TEntry* entry = tree.FindEntry( CArchiveConformanceTree::EmptyFile );
        ASSERT_TRUE( GUCEF_NULL != entry && 0 == entry->size );
        for ( UInt32 mem=0; mem<2; ++mem )
        {
            COpenFile file;
            ASSERT_TRUE( file.Open( archive, CArchiveConformanceTree::EmptyFile, 0 == mem ? NoMemoryLoad : AllInMemory ) );
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 0 == file.access->GetSize() );
            ASSERT_TRUE( 0 == file.access->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_TRUE( file.access->Eof() );
            ASSERT_TRUE( 0 == file.access->Tell() );
            ASSERT_TRUE( 0 == file.access->Setpos( 0 ) );
            ASSERT_TRUE( 0 == file.access->Read( buffer.GetBufferPtr(), 1, 1 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: two files open at once keep their own position" )
    try
    {
        const TEntry* a = tree.FindEntry( "size_65536.bin" );
        const TEntry* b = tree.FindEntry( "size_65537.bin" );
        const TEntry* c = tree.FindEntry( CArchiveConformanceTree::BigFile );
        ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b && GUCEF_NULL != c );
        if ( GUCEF_NULL != a && GUCEF_NULL != b && GUCEF_NULL != c )
        {
            for ( UInt32 mem=0; mem<2; ++mem )
            {
                const UInt32 memLoad = 0 == mem ? NoMemoryLoad : AllInMemory;
                COpenFile fa, fb, fc;
                ASSERT_TRUE( fa.Open( archive, a->path, memLoad ) );
                ASSERT_TRUE( fb.Open( archive, b->path, memLoad ) );
                ASSERT_TRUE( fc.Open( archive, c->path, memLoad ) );
                UInt64 posA = 0, posB = 0, posC = 0;
                for ( UInt32 round=0; round<20; ++round )
                {
                    ASSERT_TRUE( ReadAndCheck( *fa.access, *a, posA, 1000, "Interleaved a" ) );
                    posA += 1000;
                    ASSERT_TRUE( ReadAndCheck( *fb.access, *b, posB, 2500, "Interleaved b" ) );
                    posB += 2500;
                    ASSERT_TRUE( ReadAndCheck( *fc.access, *c, posC, 7000 + round, "Interleaved c" ) );
                    posC += 7000 + round;
                }
                ASSERT_TRUE( posA == fa.access->Tell() );
                ASSERT_TRUE( posB == fb.access->Tell() );
                ASSERT_TRUE( posC == fc.access->Tell() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: the same file opened twice gives two accesses with their own position, one outlives the other" )
    try
    {
        const TEntry* entry = tree.FindEntry( "size_65537.bin" );
        ASSERT_TRUE( GUCEF_NULL != entry );
        if ( GUCEF_NULL != entry )
        {
            for ( UInt32 mem=0; mem<2; ++mem )
            {
                const UInt32 memLoad = 0 == mem ? NoMemoryLoad : AllInMemory;
                COpenFile first, second;
                ASSERT_TRUE( first.Open( archive, entry->path, memLoad ) );
                ASSERT_TRUE( second.Open( archive, entry->path, memLoad ) );
                ASSERT_TRUE( first.access.GetPointer() != second.access.GetPointer() );

                ASSERT_TRUE( ReadAndCheck( *first.access, *entry, 0, 100, "Twice first" ) );
                ASSERT_TRUE( ReadAndCheck( *second.access, *entry, 0, 500, "Twice second" ) );
                ASSERT_TRUE( 100 == first.access->Tell() );
                ASSERT_TRUE( 500 == second.access->Tell() );
                ASSERT_TRUE( ReadAndCheck( *first.access, *entry, 100, 100, "Twice first on" ) );
                ASSERT_TRUE( 0 == second.access->Setpos( 60000 ) );
                ASSERT_TRUE( 200 == first.access->Tell() );
                ASSERT_TRUE( ReadAndCheck( *first.access, *entry, 200, 100, "Twice first after seek of second" ) );
                ASSERT_TRUE( ReadAndCheck( *second.access, *entry, 60000, 100, "Twice second after seek" ) );

                // the first goes away, the second stays good
                first.Release();
                ASSERT_TRUE( 0 == second.access->Setpos( 1 ) );
                ASSERT_TRUE( ReadAndCheck( *second.access, *entry, 1, 1000, "Twice second after release" ) );

                // and the file can be opened again meanwhile
                COpenFile third;
                ASSERT_TRUE( third.Open( archive, entry->path, memLoad ) );
                ASSERT_TRUE( ReadAndCheck( *third.access, *entry, 0, 100, "Twice third" ) );
                ASSERT_TRUE( ReadAndCheck( *second.access, *entry, 1001, 100, "Twice second after third" ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: a file that is not there gives nothing, for every mode of reading, and a directory is no file" )
    try
    {
        COpenFile file;
        ASSERT_FALSE( file.Open( archive, "absent.bin" ) );
        ASSERT_FALSE( file.Open( archive, "absent.bin", AllInMemory ) );
        ASSERT_FALSE( file.Open( archive, "absent.bin", NoMemoryLoad, "r" ) );
        ASSERT_FALSE( file.Open( archive, "nested/absent.bin" ) );
        ASSERT_FALSE( file.Open( archive, "absent_dir/absent.bin" ) );
        ASSERT_FALSE( file.Open( archive, "one.bin/child.bin" ) );
        ASSERT_FALSE( file.Open( archive, "nested" ) );
        ASSERT_FALSE( file.Open( archive, CArchiveConformanceTree::EmptyDir ) );
        VFS::TBasicVfsResourcePtr nothing = archive.GetFile( "absent.bin", "rb", 0, true );
        ASSERT_TRUE( nothing.IsNULL() );

        // the others do not answer for a file that is not there either
        ASSERT_TRUE( 0 == archive.GetFileSize( "absent.bin" ) );
        ASSERT_TRUE( archive.GetFileHash( "absent.bin" ).IsNULLOrEmpty() );
        CORE::CResourceMetaData metaData;
        ASSERT_FALSE( archive.GetFileMetaData( "absent.bin", metaData ) );
        ASSERT_FALSE( metaData.resourceExists );

        // and the archive still works afterwards
        ASSERT_TRUE( file.Open( archive, "one.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestLargeFile( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;

    GUCEF_TESTFW_TESTCASE( "A file above 4 GiB: size, listing and ranged reads on both sides of the 32 bit limit" )
    GUCEF_TESTFW_SKIP_IF( !ctx.tree->HasLargeFile(), "the fixture has no file above 4 GiB" );
    try
    {
        const TEntry* entry = ctx.tree->FindEntry( CArchiveConformanceTree::LargeFile );
        ASSERT_TRUE( GUCEF_NULL != entry );
        if ( GUCEF_NULL != entry )
        {
            const UInt64 fourGiB = (UInt64) 1 << 32;
            ASSERT_TRUE( archive.FileExists( entry->path ) );
            ASSERT_TRUE( entry->size == archive.GetFileSize( entry->path ) );
            ASSERT_TRUE( entry->size > fourGiB );
            ASSERT_TRUE( Contains( ListFiles( ctx, CORE::CString( "large" ), false, false ), "sparse.bin" ) );

            COpenFile file;
            ASSERT_TRUE( file.Open( archive, entry->path ) );
            ASSERT_TRUE( entry->size == file.access->GetSize() );

            const UInt64 offsets[] = { 0, fourGiB - 2, fourGiB - 1, fourGiB, fourGiB + 1, entry->size - 10 };
            for ( UInt32 i=0; i<sizeof( offsets ) / sizeof( offsets[ 0 ] ); ++i )
            {
                ASSERT_TRUE( 0 == file.access->Setpos( offsets[ i ] ) );
                ASSERT_TRUE( offsets[ i ] == file.access->Tell() );
                ASSERT_TRUE( ReadAndCheck( *file.access, *entry, offsets[ i ], 2, "Large" ) );
                ASSERT_TRUE( offsets[ i ] + 2 == file.access->Tell() );
            }
            ASSERT_TRUE( 0 == file.access->Seek( -4096, SEEK_END ) );
            ASSERT_TRUE( fourGiB == file.access->Tell() );
            ASSERT_TRUE( 0 == file.access->Seek( 10, SEEK_CUR ) );
            ASSERT_TRUE( fourGiB + 10 == file.access->Tell() );

            // the last bytes, then the end
            ASSERT_TRUE( 0 == file.access->Setpos( entry->size - 10 ) );
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 10 == file.access->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_TRUE( file.access->Eof() );
            ASSERT_TRUE( entry->size == file.access->Tell() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: HASH, TIME, METADATA, STATE                             //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
TestFileInformation( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    const CArchiveConformanceTree& tree = *ctx.tree;

    GUCEF_TESTFW_TESTCASE( "GetFileHash: the same content gives the same hash, other content another, a repeat the same again; nothing for a file that is not there" )
    try
    {
        const CORE::CString dup = CORE::CString( CArchiveConformanceTree::DupDir ) + "/";

        // not there: an empty hash, whatever the archive can do
        ASSERT_TRUE( archive.GetFileHash( "absent.bin" ).IsNULLOrEmpty() );
        ASSERT_TRUE( archive.GetFileHash( "nested/absent.bin" ).IsNULLOrEmpty() );

        const CORE::CString hashA = archive.GetFileHash( dup + "same_a.bin" );
        const CORE::CString hashB = archive.GetFileHash( dup + "same_b.bin" );
        const CORE::CString hashOther = archive.GetFileHash( dup + "other.bin" );
        const CORE::CString hashEmpty = archive.GetFileHash( CArchiveConformanceTree::EmptyFile );
        const CORE::CString hashOne = archive.GetFileHash( CArchiveConformanceTree::OneByteFile );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: GetFileHash gives \"" << hashA << "\" for a file of 5000 bytes, \"" << hashEmpty << "\" for an empty file" );

        if ( ctx.caps->supportsFileHash )
        {
            ASSERT_FALSE( hashA.IsNULLOrEmpty() );
            ASSERT_TRUE( hashA == hashB );
            ASSERT_TRUE( hashA != hashOther );
            ASSERT_TRUE( hashA == archive.GetFileHash( dup + "same_a.bin" ) );
            ASSERT_TRUE( hashOther == archive.GetFileHash( dup + "other.bin" ) );
            ASSERT_FALSE( hashOne.IsNULLOrEmpty() );
            ASSERT_TRUE( hashOne != hashA );
            // the empty file is a file like others: it has a hash of its own, which no other file has
            ASSERT_FALSE( hashEmpty.IsNULLOrEmpty() );
            ASSERT_TRUE( hashEmpty != hashOne );
            ASSERT_TRUE( hashEmpty != hashA );
            ASSERT_TRUE( hashEmpty == archive.GetFileHash( CArchiveConformanceTree::EmptyFile ) );
            // a file whose last byte differs in only one position would hash differently: 64 KiB - 1 and 64 KiB
            ASSERT_TRUE( archive.GetFileHash( "size_65535.bin" ) != archive.GetFileHash( "size_65536.bin" ) );
            ASSERT_TRUE( archive.GetFileHash( "size_65536.bin" ) != archive.GetFileHash( "size_65537.bin" ) );
            ASSERT_TRUE( archive.GetFileHash( CArchiveConformanceTree::BigFile ) == archive.GetFileHash( CArchiveConformanceTree::BigFile ) );
        }
        else
        {
            // no hash support: the answer is the same for every file, the empty hash
            ASSERT_TRUE( hashA.IsNULLOrEmpty() );
            ASSERT_TRUE( hashOther.IsNULLOrEmpty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileHash: the hash is the MD5 of the content" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->fileHashIsMd5, "the hash of the archive is not an MD5" );
    try
    {
        const char* const names[] = { "empty.bin", "one.bin", "size_65535.bin", "size_65536.bin", "size_65537.bin", "big.bin", "nested/top.bin", "dup/same_a.bin", "dup/other.bin" };
        for ( UInt32 n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
        {
            const TEntry* entry = tree.FindEntry( names[ n ] );
            ASSERT_TRUE( GUCEF_NULL != entry );
            if ( GUCEF_NULL != entry )
            {
                CHECK_FOR( ExpectedHash( *entry ) == archive.GetFileHash( entry->path ), entry->path );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileDigest: names the algorithm the capability says, is what GetFileHash() is for an archive that does not name one, is the hash of the content when it is an MD5, and is empty for a file that is not there" )
    try
    {
        const CORE::CString dup = CORE::CString( CArchiveConformanceTree::DupDir ) + "/";

        // not there: no digest, whatever the archive can do
        ASSERT_TRUE( archive.GetFileDigest( "absent.bin" ).IsEmpty() );
        ASSERT_TRUE( archive.GetFileDigest( "nested/absent.bin" ).IsEmpty() );

        const VFS::CVfsDigest digestA = archive.GetFileDigest( dup + "same_a.bin" );
        const VFS::CVfsDigest digestB = archive.GetFileDigest( dup + "same_b.bin" );
        const VFS::CVfsDigest digestOther = archive.GetFileDigest( dup + "other.bin" );
        if ( VFS::CVfsDigest::ALGORITHM_UNKNOWN == ctx.caps->digestAlgorithm )
        {
            // the default of the archive interface: the hash string as it is, tagged unknown
            ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_UNKNOWN == digestA.algorithm );
            ASSERT_TRUE( digestA.value == archive.GetFileHash( dup + "same_a.bin" ) );
            ASSERT_TRUE( digestOther.value == archive.GetFileHash( dup + "other.bin" ) );
        }
        else
        {
            ASSERT_TRUE( ctx.caps->digestAlgorithm == digestA.algorithm );
            ASSERT_TRUE( ctx.caps->digestAlgorithm == digestOther.algorithm );
            ASSERT_TRUE( digestA.IsWellFormed() );
            ASSERT_TRUE( digestOther.IsWellFormed() );
            ASSERT_TRUE( digestA.Matches( digestB ) );
            ASSERT_FALSE( digestA.Matches( digestOther ) );
            ASSERT_TRUE( digestA.Matches( archive.GetFileDigest( dup + "same_a.bin" ) ) );
            // an empty file is a file like others: it has a digest of its own
            const VFS::CVfsDigest digestEmpty = archive.GetFileDigest( CArchiveConformanceTree::EmptyFile );
            ASSERT_TRUE( digestEmpty.IsWellFormed() );
            ASSERT_FALSE( digestEmpty.Matches( digestA ) );
            if ( VFS::CVfsDigest::ALGORITHM_MD5 == ctx.caps->digestAlgorithm && ctx.caps->fileHashIsMd5 )
            {
                // the same content hash as GetFileHash() gives, in the form of a digest
                const char* const names[] = { "empty.bin", "one.bin", "size_65535.bin", "size_65536.bin", "size_65537.bin", "big.bin", "nested/top.bin", "dup/same_a.bin", "dup/other.bin" };
                for ( UInt32 n=0; n<sizeof( names ) / sizeof( names[ 0 ] ); ++n )
                {
                    const TEntry* entry = tree.FindEntry( names[ n ] );
                    ASSERT_TRUE( GUCEF_NULL != entry );
                    if ( GUCEF_NULL != entry )
                    {
                        CHECK_FOR( Md5StringAsHex( ExpectedHash( *entry ) ) == archive.GetFileDigest( entry->path ).value, entry->path );
                    }
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileModificationTime: a real time that is the same every time it is asked, none for a file that is not there" )
    try
    {
        const TStringVector files = AllFilePaths( tree, false );
        const CORE::CDateTime missing = archive.GetFileModificationTime( "absent.bin" );
        ASSERT_TRUE( missing == archive.GetFileModificationTime( "absent.bin" ) );
        ASSERT_TRUE( missing == archive.GetFileModificationTime( "nested/absent.bin" ) );

        const UInt64 nowMs = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        UInt32 checked = 0;
        for ( TStringVector::const_iterator i = files.begin(); i != files.end() && checked < 80; ++i, ++checked )
        {
            const CORE::CDateTime first = archive.GetFileModificationTime( (*i) );
            const CORE::CDateTime second = archive.GetFileModificationTime( (*i) );
            CHECK_FOR( first == second, (*i) );
            if ( ctx.caps->hasModificationTimes )
            {
                CHECK_FOR( first != missing, (*i) );
                const UInt64 ms = first.ToUnixEpochBasedTicksInMillisecs();
                // not before the year 2000 and not more than a day ahead of the clock
                CHECK_FOR( ms > (UInt64) 946684800 * 1000 && ms < nowMs + 86400000, (*i) << ": " << ms );
            }
        }
        // reading a file does not change its time
        const CORE::CDateTime before = archive.GetFileModificationTime( "size_65536.bin" );
        ASSERT_TRUE( ReadWholeAndCheck( archive, *tree.FindEntry( "size_65536.bin" ), NoMemoryLoad, 4096 ) );
        ASSERT_TRUE( before == archive.GetFileModificationTime( "size_65536.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileMetaData: the file exists and has its size, the time agrees with GetFileModificationTime to the second" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->supportsFileMetaData, "the archive does not offer the metadata of a file" );
    try
    {
        const TStringVector files = AllFilePaths( tree, false );
        UInt32 checked = 0;
        for ( TStringVector::const_iterator i = files.begin(); i != files.end() && checked < 60; ++i, ++checked )
        {
            const TEntry* entry = tree.FindEntry( (*i) );
            CORE::CResourceMetaData metaData;
            CHECK_FOR( archive.GetFileMetaData( (*i), metaData ), (*i) );
            CHECK_FOR( metaData.resourceExists, (*i) );
            CHECK_FOR( metaData.hasResourceSizeInBytes && metaData.resourceSizeInBytes == entry->size, (*i) );
            if ( ctx.caps->hasModificationTimes )
            {
                // the two may differ in resolution (the metadata of the reference archive has finer time than
                // GetFileModificationTime), they have to agree to the second
                CHECK_FOR( metaData.hasModifiedDateTime, (*i) );
                const UInt64 metaMs = metaData.modifiedDateTime.ToUnixEpochBasedTicksInMillisecs();
                const UInt64 timeMs = archive.GetFileModificationTime( (*i) ).ToUnixEpochBasedTicksInMillisecs();
                CHECK_FOR( ( metaMs > timeMs ? metaMs - timeMs : timeMs - metaMs ) < 1000, (*i) << ": " << metaMs << " against " << timeMs );
            }
        }
        CORE::CResourceMetaData missing;
        ASSERT_FALSE( archive.GetFileMetaData( "absent.bin", missing ) );
        ASSERT_FALSE( missing.resourceExists );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestArchiveState( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;

    GUCEF_TESTFW_TESTCASE( "The loaded archive is healthy and connected, has a name and a type, and says whether it is writable" )
    try
    {
        ASSERT_TRUE( archive.IsHealthy() );
        ASSERT_TRUE( archive.IsConnected() );
        ASSERT_FALSE( archive.GetArchiveName().IsNULLOrEmpty() );
        ASSERT_FALSE( archive.GetType().IsNULLOrEmpty() );
        ASSERT_TRUE( archive.IsWriteable() == ctx.caps->isWritable );
        // asking again changes nothing
        ASSERT_TRUE( archive.IsHealthy() );
        ASSERT_TRUE( archive.IsConnected() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Directory watching: support is announced as the fixture says, a watch can be added and removed" )
    try
    {
        ASSERT_TRUE( archive.IsDirectoryWatchingSupported() == ctx.caps->supportsDirectoryWatching );
        CORE::CIDirectoryWatcher::CDirWatchOptions options;
        CORE::CString::StringSet dirs;
        if ( ctx.caps->supportsDirectoryWatching )
        {
            ASSERT_TRUE( archive.AddDirToWatch( "nested", options, false ) );
            ASSERT_TRUE( archive.GetAllWatchedDirs( dirs ) );
            ASSERT_TRUE( 1 == dirs.size() );
            ASSERT_TRUE( archive.RemoveDirToWatch( "nested" ) );
            dirs.clear();
            archive.GetAllWatchedDirs( dirs );
            ASSERT_TRUE( dirs.empty() );

            ASSERT_TRUE( archive.AddDirToWatch( "nested", options, false ) );
            ASSERT_TRUE( archive.AddDirToWatch( "dup", options, false ) );
            ASSERT_TRUE( archive.RemoveAllWatches() );
            dirs.clear();
            archive.GetAllWatchedDirs( dirs );
            ASSERT_TRUE( dirs.empty() );
        }
        else
        {
            ASSERT_FALSE( archive.AddDirToWatch( "nested", options, false ) );
            ASSERT_FALSE( archive.GetAllWatchedDirs( dirs ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A path that leads out of the archive does not reach what is outside it" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->canProvideOutsidePath, "the fixture has no file outside the archive to reach" );
    try
    {
        CORE::CString outside;
        ASSERT_TRUE( ctx.fixture->GetPathOutsideArchive( outside ) );
        ASSERT_FALSE( outside.IsNULLOrEmpty() );

        if ( !ctx.caps->confinesPathsToArchive )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: this archive does not confine paths to its root, the case is informative only: FileExists gives " <<
                       archive.FileExists( outside ) << " for " << outside );
        }
        else
        {
            ASSERT_FALSE( archive.FileExists( outside ) );
            ASSERT_TRUE( 0 == archive.GetFileSize( outside ) );
            ASSERT_TRUE( archive.GetFileHash( outside ).IsNULLOrEmpty() );
            COpenFile file;
            ASSERT_FALSE( file.Open( archive, outside ) );
            ASSERT_FALSE( file.Open( archive, outside, AllInMemory ) );
            // also the way in and out again, from inside
            ASSERT_FALSE( archive.FileExists( CORE::CString( "nested/../" ) + outside ) );
            ASSERT_TRUE( ListFiles( ctx, "../", false, false ).empty() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: SEVERAL THREADS                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A thread that reads files and lists of the archive, as another thread of the application would */
class CParallelReader : public MT::CActiveObject
{
    public:

    CParallelReader( VFS::CArchive& archive, const CArchiveConformanceTree& tree, UInt32 threadNr, UInt32 fileCount )
        : MT::CActiveObject()
        , m_archive( archive )
        , m_tree( tree )
        , m_threadNr( threadNr )
        , m_fileCount( fileCount )
        , m_done( 0 )
        , m_failures( 0 )
        , m_operations( 0 )
    {GUCEF_TRACE;

    }

    virtual ~CParallelReader() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Deactivate( false, true );
    }

    bool IsDone( void ) const { return 0 != m_done.Load(); }
    UInt32 GetFailures( void ) const { return m_failures.Load(); }
    UInt32 GetOperations( void ) const { return m_operations.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        try
        {
            Run();
        }
        catch ( ... )
        {
            Fail( "exception" );
        }
        m_done.Store( 1 );
        return true;
    }

    private:

    void Fail( const char* what )
    {GUCEF_TRACE;

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: parallel reader " << m_threadNr << " failed: " << what );
        m_failures.FetchAdd( 1 );
    }

    void Run( void )
    {GUCEF_TRACE;

        const TStringVector files = AllFilePaths( m_tree, false );
        const UInt32 memLoad = 0 == ( m_threadNr % 2 ) ? NoMemoryLoad : AllInMemory;
        const TEntry* big = m_tree.FindEntry( CArchiveConformanceTree::BigFile );

        for ( UInt32 n=0; n<m_fileCount; ++n )
        {
            // every thread walks the list from another place, so they meet in the same files at other moments
            const TEntry* entry = m_tree.FindEntry( files[ ( m_threadNr * 97 + n * 7 ) % files.size() ] );
            if ( GUCEF_NULL == entry )
            {
                Fail( "entry" );
                continue;
            }
            if ( !m_archive.FileExists( entry->path ) )
                Fail( "FileExists" );
            if ( m_archive.GetFileSize( entry->path ) != entry->size )
                Fail( "GetFileSize" );
            if ( !ReadWholeAndCheck( m_archive, *entry, memLoad, 4001 + m_threadNr ) )
                Fail( "ReadWholeAndCheck" );
            m_operations.FetchAdd( 3 );

            if ( 0 == n % 10 )
            {
                if ( !SameListQuietly( "nested", true ) || !SameListQuietly( CArchiveConformanceTree::FilterDir, false ) )
                    Fail( "GetFileList" );
                if ( GUCEF_NULL != big && !ReadRandomRanges( m_archive, *big, memLoad, 100 + m_threadNr, 12 ) )
                    Fail( "ReadRandomRanges" );
                m_operations.FetchAdd( 3 );
            }
        }
    }

    bool SameListQuietly( const char* location, bool recursive )
    {GUCEF_TRACE;

        TStringVector list;
        m_archive.GetFileList( list, CORE::CString::Empty, location, recursive, false, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        TStringVector expected;
        m_tree.GetDescendantPaths( location, recursive, true, false, expected );
        return LeafNames( expected ) == Normalized( list );
    }

    private:

    VFS::CArchive& m_archive;
    const CArchiveConformanceTree& m_tree;
    UInt32 m_threadNr;
    UInt32 m_fileCount;
    MT::CTAtomic< UInt32 > m_done;
    MT::CTAtomic< UInt32 > m_failures;
    MT::CTAtomic< UInt32 > m_operations;
};

/*-------------------------------------------------------------------------*/

void
TestParallelReads( const SSuiteContext& ctx )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Several threads read, query and list the same archive at once, all with the right content" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->supportsConcurrentCallers, "the archive is called by one thread, which also works what the archive waits for" );
    try
    {
        const UInt32 threadCount = 4;
        CParallelReader* readers[ threadCount ];
        for ( UInt32 t=0; t<threadCount; ++t )
            readers[ t ] = GUCEF_NEW CParallelReader( *ctx.archive, *ctx.tree, t, 80 );

        for ( UInt32 t=0; t<threadCount; ++t )
            ASSERT_TRUE( readers[ t ]->Activate( GUCEF_NULL, 1, 1 ) );

        const UInt64 start = MT::PrecisionTickCount();
        bool allDone = false;
        while ( !allDone && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 120000.0 )
        {
            allDone = true;
            for ( UInt32 t=0; t<threadCount; ++t )
                allDone = allDone && readers[ t ]->IsDone();
            if ( !allDone )
                MT::ThreadDelay( 10 );
        }
        ASSERT_TRUE( allDone );

        UInt32 failures = 0;
        UInt32 operations = 0;
        for ( UInt32 t=0; t<threadCount; ++t )
        {
            failures += readers[ t ]->GetFailures();
            operations += readers[ t ]->GetOperations();
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: " << threadCount << " parallel readers did " << operations << " operations with " << failures << " failures" );
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( operations > 0 );

        for ( UInt32 t=0; t<threadCount; ++t )
            GUCEF_DELETE readers[ t ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: CHANGES BEHIND THE ARCHIVE                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TestChangeDetection( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    CArchiveConformanceFixture& fixture = *ctx.fixture;
    const CORE::CString path = JoinPath( CArchiveConformanceTree::MutateDir, "changing.bin" );

    GUCEF_TESTFW_TESTCASE( "Change detection: a file that changes size is seen with its new size, hash, time and content, also when it was read before" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->canMutateTree, "the fixture cannot change the content behind the archive" );
    try
    {
        CORE::CDynamicBuffer first, second, third;
        MakeContent( 9001, 1000, first );
        MakeContent( 9002, 70000, second );
        MakeContent( 9003, 300, third );

        // it appears
        ASSERT_FALSE( archive.FileExists( path ) );
        ASSERT_TRUE( fixture.PutFile( path, first ) );
        ASSERT_TRUE( archive.FileExists( path ) );
        ASSERT_TRUE( 1000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, NoMemoryLoad ) );
        const CORE::CString hash1 = archive.GetFileHash( path );
        const CORE::CDateTime time1 = archive.GetFileModificationTime( path );
        TStringVector listed = ListFiles( ctx, CArchiveConformanceTree::MutateDir, false, false );
        ASSERT_TRUE( 1 == listed.size() && "changing.bin" == listed[ 0 ] );

        // it grows
        WaitForNextModificationTime( ctx );
        ASSERT_TRUE( fixture.PutFile( path, second ) );
        ASSERT_TRUE( 70000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, NoMemoryLoad ) );
        const CORE::CString hash2 = archive.GetFileHash( path );
        const CORE::CDateTime time2 = archive.GetFileModificationTime( path );
        if ( ctx.caps->supportsFileHash )
            ASSERT_TRUE( hash1 != hash2 );
        if ( ctx.caps->hasModificationTimes )
            ASSERT_TRUE( time1 != time2 );

        // it shrinks, and the file that is open at the time keeps working until it is released
        COpenFile open;
        ASSERT_TRUE( open.Open( archive, path, NoMemoryLoad ) );
        WaitForNextModificationTime( ctx );
        ASSERT_TRUE( fixture.PutFile( path, third ) );
        open.Release();
        ASSERT_TRUE( 300 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, third, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, third, NoMemoryLoad ) );

        // it is made empty
        CORE::CDynamicBuffer empty;
        WaitForNextModificationTime( ctx );
        ASSERT_TRUE( fixture.PutFile( path, empty ) );
        ASSERT_TRUE( archive.FileExists( path ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, empty, AllInMemory ) );

        // it goes away
        ASSERT_TRUE( fixture.RemoveFile( path ) );
        ASSERT_FALSE( archive.FileExists( path ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( path ) );
        COpenFile gone;
        ASSERT_FALSE( gone.Open( archive, path ) );
        ASSERT_FALSE( gone.Open( archive, path, AllInMemory ) );
        ASSERT_TRUE( ListFiles( ctx, CArchiveConformanceTree::MutateDir, false, false ).empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    fixture.RemoveFile( path );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Change detection: a file that changes content and keeps its size is seen with its new hash, time and content" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->canMutateTree, "the fixture cannot change the content behind the archive" );
    GUCEF_TESTFW_SKIP_IF( ctx.caps->revisionIsWeak, "the archive has weak revisions: a change that keeps the size can go unnoticed" );
    try
    {
        CORE::CDynamicBuffer first, second;
        MakeContent( 9101, 5000, first );
        MakeContent( 9102, 5000, second );

        ASSERT_TRUE( fixture.PutFile( path, first ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, AllInMemory ) );
        const CORE::CString hash1 = archive.GetFileHash( path );
        const CORE::CDateTime time1 = archive.GetFileModificationTime( path );

        WaitForNextModificationTime( ctx );
        ASSERT_TRUE( fixture.PutFile( path, second ) );
        ASSERT_TRUE( 5000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, NoMemoryLoad ) );
        if ( ctx.caps->supportsFileHash )
            ASSERT_TRUE( hash1 != archive.GetFileHash( path ) );
        if ( ctx.caps->hasModificationTimes )
            ASSERT_TRUE( time1 != archive.GetFileModificationTime( path ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    fixture.RemoveFile( path );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Change detection: files added behind the archive show in its lists, and files removed no longer do" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->canMutateTree, "the fixture cannot change the content behind the archive" );
    try
    {
        const CORE::CString a = JoinPath( CArchiveConformanceTree::MutateDir, "added_a.bin" );
        const CORE::CString b = JoinPath( CArchiveConformanceTree::MutateDir, "added_b.bin" );
        const CORE::CString nested = JoinPath( CArchiveConformanceTree::MutateDir, "added_dir/added_c.bin" );
        CORE::CDynamicBuffer content;
        MakeContent( 9201, 100, content );

        ASSERT_TRUE( fixture.PutFile( a, content ) );
        ASSERT_TRUE( fixture.PutFile( b, content ) );
        ASSERT_TRUE( fixture.PutFile( nested, content ) );
        TStringVector names = ListFiles( ctx, CArchiveConformanceTree::MutateDir, false, false );
        ASSERT_TRUE( 2 == names.size() );
        ASSERT_TRUE( Contains( names, "added_a.bin" ) && Contains( names, "added_b.bin" ) );
        ASSERT_TRUE( 3 == ListFiles( ctx, CArchiveConformanceTree::MutateDir, true, false ).size() );
        ASSERT_TRUE( Contains( ListDirs( ctx, CArchiveConformanceTree::MutateDir, false, false ), "added_dir" ) );
        ASSERT_TRUE( archive.DirExists( JoinPath( CArchiveConformanceTree::MutateDir, "added_dir" ) ) );
        // added files have the same hash when they have the same content
        if ( ctx.caps->supportsFileHash )
            ASSERT_TRUE( archive.GetFileHash( a ) == archive.GetFileHash( b ) );

        ASSERT_TRUE( fixture.RemoveFile( a ) );
        names = ListFiles( ctx, CArchiveConformanceTree::MutateDir, false, false );
        ASSERT_TRUE( 1 == names.size() && "added_b.bin" == names[ 0 ] );
        ASSERT_FALSE( archive.FileExists( a ) );
        ASSERT_TRUE( archive.FileExists( b ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    fixture.RemoveFile( JoinPath( CArchiveConformanceTree::MutateDir, "added_a.bin" ) );
    fixture.RemoveFile( JoinPath( CArchiveConformanceTree::MutateDir, "added_b.bin" ) );
    fixture.RemoveFile( JoinPath( CArchiveConformanceTree::MutateDir, "added_dir/added_c.bin" ) );
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: THE WRITE SIDE                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Stores a file of generated content through the archive, and checks it reads back */
bool
StoreGenerated( VFS::CArchive& archive, const CORE::CString& path, UInt32 contentId, UInt32 size, bool overwrite = true )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    MakeContent( contentId, size, content );
    if ( !archive.StoreAsFile( path, content, 0, overwrite ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: StoreAsFile failed for " << path );
        return false;
    }
    return archive.FileExists( path ) && archive.GetFileSize( path ) == size && FileHasContent( archive, path, content, NoMemoryLoad );
}

/*-------------------------------------------------------------------------*/

void
TestWriting( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;
    const CORE::CString area = WriteArea;

    GUCEF_TESTFW_TESTCASE( "StoreAsFile: a new file is stored with the directories it needs, and reads back byte exact" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString path = area + "/a/b/new.bin";
        ASSERT_FALSE( archive.FileExists( path ) );
        ASSERT_TRUE( StoreGenerated( archive, path, 7001, 1000 ) );
        ASSERT_TRUE( archive.DirExists( area + "/a/b" ) );
        ASSERT_TRUE( archive.DirExists( area + "/a" ) );
        ASSERT_TRUE( archive.DirExists( area ) );
        ASSERT_FALSE( archive.FileExists( area + "/a" ) );
        ASSERT_TRUE( Contains( ListFiles( ctx, area + "/a/b", false, false ), "new.bin" ) );
        ASSERT_TRUE( Contains( ListDirs( ctx, area + "/a", false, false ), "b" ) );

        // sizes around the edges and a large one, and a name with a space and UTF-8 characters
        ASSERT_TRUE( StoreGenerated( archive, area + "/s1.bin", 7002, 1 ) );
        ASSERT_TRUE( StoreGenerated( archive, area + "/s65535.bin", 7003, 65535 ) );
        ASSERT_TRUE( StoreGenerated( archive, area + "/s65536.bin", 7004, 65536 ) );
        ASSERT_TRUE( StoreGenerated( archive, area + "/s65537.bin", 7005, 65537 ) );
        ASSERT_TRUE( StoreGenerated( archive, area + "/big.bin", 7006, 3 * 1048576 + 12345 ) );
        ASSERT_TRUE( StoreGenerated( archive, area + "/name with spaces \xC3\xBC.bin", 7007, 77 ) );
        ASSERT_TRUE( 6 == ListFiles( ctx, area, false, false ).size() );
        ASSERT_TRUE( Contains( ListFiles( ctx, area, false, false ), CORE::CString( "name with spaces \xC3\xBC.bin" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StoreAsFile: storing over a file replaces it whole, also after the file was read before" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString path = area + "/replace.bin";
        CORE::CDynamicBuffer first, second, third;
        MakeContent( 7101, 1000, first );
        MakeContent( 7102, 70000, second );
        MakeContent( 7103, 10, third );

        ASSERT_TRUE( archive.StoreAsFile( path, first, 0, true ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, AllInMemory ) );
        ASSERT_TRUE( archive.StoreAsFile( path, second, 0, true ) );
        ASSERT_TRUE( 70000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, second, NoMemoryLoad ) );
        // smaller: nothing of the longer file stays behind
        ASSERT_TRUE( archive.StoreAsFile( path, third, 0, true ) );
        ASSERT_TRUE( 10 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, third, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, path, third, NoMemoryLoad ) );
        ASSERT_TRUE( 1 == ListFiles( ctx, area, false, false, Filters( "replace*" ) ).size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StoreAsFile: a buffer of no bytes makes an empty file" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString path = area + "/stored_empty.bin";
        CORE::CDynamicBuffer empty;
        const bool stored = archive.StoreAsFile( path, empty, 0, true );
        if ( ctx.caps->storesEmptyFiles )
        {
            ASSERT_TRUE( stored );
            ASSERT_TRUE( archive.FileExists( path ) );
            ASSERT_TRUE( 0 == archive.GetFileSize( path ) );
            ASSERT_TRUE( FileHasContent( archive, path, empty, NoMemoryLoad ) );

            // over a file with content it makes that empty
            ASSERT_TRUE( StoreGenerated( archive, path, 7201, 100 ) );
            ASSERT_TRUE( archive.StoreAsFile( path, empty, 0, true ) );
            ASSERT_TRUE( 0 == archive.GetFileSize( path ) );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: this archive does not store empty files, StoreAsFile gave " << stored << " and the file exists: " << archive.FileExists( path ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StoreAsFile: without overwrite the data is written at the offset into the file that is there, the rest stays" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable || !ctx.caps->supportsPartialWrites, "the archive cannot write at an offset" );
    try
    {
        const CORE::CString path = area + "/partial.bin";
        CORE::CDynamicBuffer original, patch, expected;
        MakeContent( 7301, 1000, original );
        MakeContent( 7302, 16, patch );
        ASSERT_TRUE( archive.StoreAsFile( path, original, 0, true ) );

        expected = original;
        memcpy( expected.GetBufferPtr( 100 ), patch.GetConstBufferPtr(), 16 );

        ASSERT_TRUE( archive.StoreAsFile( path, patch, 100, false ) );
        ASSERT_TRUE( 1000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, expected, NoMemoryLoad ) );
        ASSERT_TRUE( FileHasContent( archive, path, expected, AllInMemory ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFile: a file opened for writing takes the bytes written, an existing file is only opened for writing when overwrite is asked, and append adds" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString path = area + "/handle/written.bin";
        CORE::CDynamicBuffer first, more, all;
        MakeContent( 7401, 1000, first );
        MakeContent( 7402, 500, more );

        {
            COpenFile file;
            ASSERT_TRUE( file.Open( archive, path, NoMemoryLoad, "wb", true ) );
            ASSERT_TRUE( file.access->IsWriteable() );
            ASSERT_TRUE( 1000 == file.access->Write( first.GetConstBufferPtr(), 1, 1000 ) );
            ASSERT_TRUE( 1000 == file.access->Tell() );
            file.access->Flush();
        }
        ASSERT_TRUE( archive.FileExists( path ) );
        ASSERT_TRUE( 1000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, NoMemoryLoad ) );

        // not asked to overwrite: the file that is there is left alone
        {
            COpenFile refused;
            ASSERT_FALSE( refused.Open( archive, path, NoMemoryLoad, "wb", false ) );
        }
        ASSERT_TRUE( 1000 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, first, NoMemoryLoad ) );

        if ( ctx.caps->supportsPartialWrites )
        {
            all = first;
            all.Append( more.GetConstBufferPtr(), 500 );
            {
                COpenFile appender;
                ASSERT_TRUE( appender.Open( archive, path, NoMemoryLoad, "ab", false ) );
                ASSERT_TRUE( 500 == appender.access->Write( more.GetConstBufferPtr(), 1, 500 ) );
                appender.access->Flush();
            }
            ASSERT_TRUE( 1500 == archive.GetFileSize( path ) );
            ASSERT_TRUE( FileHasContent( archive, path, all, NoMemoryLoad ) );
        }

        // asked to overwrite: it starts over
        {
            COpenFile again;
            ASSERT_TRUE( again.Open( archive, path, NoMemoryLoad, "wb", true ) );
            ASSERT_TRUE( 500 == again.access->Write( more.GetConstBufferPtr(), 1, 500 ) );
            again.access->Flush();
        }
        ASSERT_TRUE( 500 == archive.GetFileSize( path ) );
        ASSERT_TRUE( FileHasContent( archive, path, more, AllInMemory ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DeleteFile: a deleted file is gone for every call, also when it was read before" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString path = area + "/delete/victim.bin";
        const CORE::CString other = area + "/delete/survivor.bin";
        ASSERT_TRUE( StoreGenerated( archive, path, 7501, 1000 ) );
        ASSERT_TRUE( StoreGenerated( archive, other, 7502, 1000 ) );
        {
            COpenFile warm;
            ASSERT_TRUE( warm.Open( archive, path, AllInMemory ) );
        }

        ASSERT_TRUE( archive.DeleteFile( path ) );
        ASSERT_FALSE( archive.FileExists( path ) );
        ASSERT_TRUE( 0 == archive.GetFileSize( path ) );
        ASSERT_TRUE( archive.GetFileHash( path ).IsNULLOrEmpty() );
        COpenFile gone;
        ASSERT_FALSE( gone.Open( archive, path, AllInMemory ) );
        ASSERT_FALSE( gone.Open( archive, path, NoMemoryLoad ) );
        TStringVector names = ListFiles( ctx, area + "/delete", false, false );
        ASSERT_TRUE( 1 == names.size() && "survivor.bin" == names[ 0 ] );
        ASSERT_TRUE( archive.FileExists( other ) );

        // a file that is not there: the archive does not fail in a way that is a problem, and nothing appears
        const bool deleted = archive.DeleteFile( path );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: DeleteFile of a file that is not there gives " << deleted );
        ASSERT_FALSE( archive.FileExists( path ) );
        archive.DeleteFile( area + "/delete/never_was.bin" );
        ASSERT_FALSE( archive.FileExists( area + "/delete/never_was.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MoveFile: the file is at the new path and no longer at the old one, over a file only when asked to" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable, "the archive is not writable" );
    try
    {
        const CORE::CString src = area + "/move/src.bin";
        const CORE::CString dst = area + "/move/to/deeper/dst.bin";
        CORE::CDynamicBuffer srcContent, otherContent;
        MakeContent( 7601, 2000, srcContent );
        MakeContent( 7602, 3000, otherContent );

        ASSERT_TRUE( archive.StoreAsFile( src, srcContent, 0, true ) );
        {
            COpenFile warm;
            ASSERT_TRUE( warm.Open( archive, src, AllInMemory ) );
        }
        ASSERT_TRUE( archive.MoveFile( src, dst, false ) );
        ASSERT_FALSE( archive.FileExists( src ) );
        ASSERT_TRUE( archive.FileExists( dst ) );
        ASSERT_TRUE( 2000 == archive.GetFileSize( dst ) );
        ASSERT_TRUE( FileHasContent( archive, dst, srcContent, NoMemoryLoad ) );
        ASSERT_TRUE( FileHasContent( archive, dst, srcContent, AllInMemory ) );
        COpenFile gone;
        ASSERT_FALSE( gone.Open( archive, src, AllInMemory ) );

        // onto a file that is there: refused, and neither of the files is changed
        ASSERT_TRUE( archive.StoreAsFile( src, otherContent, 0, true ) );
        ASSERT_FALSE( archive.MoveFile( src, dst, false ) );
        ASSERT_TRUE( FileHasContent( archive, src, otherContent, NoMemoryLoad ) );
        ASSERT_TRUE( FileHasContent( archive, dst, srcContent, NoMemoryLoad ) );

        // asked to overwrite: the target is replaced, also for a reader of the old target that read it before
        {
            COpenFile warm;
            ASSERT_TRUE( warm.Open( archive, dst, AllInMemory ) );
        }
        ASSERT_TRUE( archive.MoveFile( src, dst, true ) );
        ASSERT_FALSE( archive.FileExists( src ) );
        ASSERT_TRUE( 3000 == archive.GetFileSize( dst ) );
        ASSERT_TRUE( FileHasContent( archive, dst, otherContent, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, dst, otherContent, NoMemoryLoad ) );

        // a file that is not there cannot be moved
        ASSERT_FALSE( archive.MoveFile( area + "/move/absent.bin", area + "/move/absent_to.bin", true ) );
        ASSERT_FALSE( archive.FileExists( area + "/move/absent_to.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MoveFileAtomically: the file arrives whole at the new path, over a file only when asked to" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable || !ctx.caps->supportsAtomicMove, "the archive does not offer an atomic move" );
    try
    {
        const CORE::CString src = area + "/atomic/src.bin";
        const CORE::CString dst = area + "/atomic/dst.bin";
        CORE::CDynamicBuffer srcContent, otherContent;
        MakeContent( 7701, 70000, srcContent );
        MakeContent( 7702, 100, otherContent );

        ASSERT_TRUE( archive.StoreAsFile( src, srcContent, 0, true ) );
        ASSERT_TRUE( archive.MoveFileAtomically( src, dst, false ) );
        ASSERT_FALSE( archive.FileExists( src ) );
        ASSERT_TRUE( FileHasContent( archive, dst, srcContent, NoMemoryLoad ) );

        ASSERT_TRUE( archive.StoreAsFile( src, otherContent, 0, true ) );
        ASSERT_FALSE( archive.MoveFileAtomically( src, dst, false ) );
        ASSERT_TRUE( FileHasContent( archive, dst, srcContent, NoMemoryLoad ) );
        ASSERT_TRUE( FileHasContent( archive, src, otherContent, NoMemoryLoad ) );
        ASSERT_TRUE( archive.MoveFileAtomically( src, dst, true ) );
        ASSERT_FALSE( archive.FileExists( src ) );
        ASSERT_TRUE( FileHasContent( archive, dst, otherContent, NoMemoryLoad ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CreateDir, MoveDir and DeleteDir: directories are made with their parents, moved with their content, and deleted empty or with their content" )
    GUCEF_TESTFW_SKIP_IF( !ctx.caps->isWritable || !ctx.caps->supportsDirOperations, "the archive does not offer directory operations" );
    try
    {
        const CORE::CString base = area + "/dirs";
        ASSERT_TRUE( archive.CreateDir( base + "/x/y/z" ) );
        ASSERT_TRUE( archive.DirExists( base + "/x/y/z" ) );
        ASSERT_TRUE( archive.DirExists( base + "/x/y" ) );
        ASSERT_TRUE( archive.DirExists( base + "/x" ) );
        ASSERT_TRUE( ListFiles( ctx, base + "/x/y/z", true, false ).empty() );
        ASSERT_TRUE( Contains( ListDirs( ctx, base + "/x/y", false, false ), "z" ) );
        // a directory that is there can be made again
        const bool again = archive.CreateDir( base + "/x/y/z" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: CreateDir of a directory that is there gives " << again );
        ASSERT_TRUE( archive.DirExists( base + "/x/y/z" ) );

        // a directory with a file: only deleted with its content
        ASSERT_TRUE( StoreGenerated( archive, base + "/x/y/z/in.bin", 7801, 500 ) );
        ASSERT_FALSE( archive.DeleteDir( base + "/x/y/z", false ) );
        ASSERT_TRUE( archive.DirExists( base + "/x/y/z" ) );
        ASSERT_TRUE( archive.FileExists( base + "/x/y/z/in.bin" ) );

        // moved with the content, the old name is free afterwards
        CORE::CDynamicBuffer content;
        MakeContent( 7801, 500, content );
        {
            COpenFile warm;
            ASSERT_TRUE( warm.Open( archive, base + "/x/y/z/in.bin", AllInMemory ) );
        }
        ASSERT_TRUE( archive.MoveDir( base + "/x", base + "/moved" ) );
        ASSERT_FALSE( archive.DirExists( base + "/x" ) );
        ASSERT_FALSE( archive.FileExists( base + "/x/y/z/in.bin" ) );
        ASSERT_TRUE( archive.DirExists( base + "/moved/y/z" ) );
        ASSERT_TRUE( FileHasContent( archive, base + "/moved/y/z/in.bin", content, AllInMemory ) );
        ASSERT_TRUE( FileHasContent( archive, base + "/moved/y/z/in.bin", content, NoMemoryLoad ) );

        // not onto a directory that is there
        ASSERT_TRUE( archive.CreateDir( base + "/occupied" ) );
        ASSERT_FALSE( archive.MoveDir( base + "/moved", base + "/occupied" ) );
        ASSERT_TRUE( archive.DirExists( base + "/moved" ) );

        // an empty one goes, one with content goes when asked to, one that is not there is no success
        ASSERT_TRUE( archive.DeleteDir( base + "/occupied", false ) );
        ASSERT_FALSE( archive.DirExists( base + "/occupied" ) );
        ASSERT_TRUE( archive.DeleteDir( base + "/moved", true ) );
        ASSERT_FALSE( archive.DirExists( base + "/moved" ) );
        ASSERT_FALSE( archive.FileExists( base + "/moved/y/z/in.bin" ) );
        ASSERT_FALSE( archive.DeleteDir( base + "/moved", true ) );
        COpenFile gone;
        ASSERT_FALSE( gone.Open( archive, base + "/moved/y/z/in.bin", AllInMemory ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // what the cases above made is removed again, so the tree is the one the next cases expect
    if ( ctx.caps->isWritable )
    {
        if ( ctx.caps->supportsDirOperations )
            archive.DeleteDir( area, true );
        else
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: the archive has no directory operations, the files of the write cases stay in " << area );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES: AFTER UNLOADING                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TestUnload( const SSuiteContext& ctx )
{GUCEF_TRACE;

    VFS::CArchive& archive = *ctx.archive;

    GUCEF_TESTFW_TESTCASE( "UnloadArchive: the calls that follow neither crash nor hang, whatever they answer, and unloading twice is harmless" )
    try
    {
        // a file that is open while the archive is unloaded
        COpenFile open;
        const bool wasOpened = open.Open( archive, "nested/level1/level2/leaf.bin", NoMemoryLoad );

        ASSERT_TRUE( archive.UnloadArchive() );

        const bool exists = archive.FileExists( "one.bin" );
        const bool dirExists = archive.DirExists( "nested" );
        const UInt64 size = archive.GetFileSize( "size_65536.bin" );
        const CORE::CString hash = archive.GetFileHash( "one.bin" );
        const CORE::CDateTime time = archive.GetFileModificationTime( "one.bin" );
        TStringVector files;
        archive.GetFileList( files, CORE::CString::Empty, CORE::CString::Empty, true, true, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        TStringVector dirs;
        archive.GetDirList( dirs, CORE::CString::Empty, CORE::CString::Empty, true, true, CORE::CString::EmptyStringSet, GUCEF_UINT16MAX );
        COpenFile after;
        const bool opened = after.Open( archive, "one.bin" );
        CORE::CResourceMetaData metaData;
        const bool gotMetaData = archive.GetFileMetaData( "one.bin", metaData );
        const bool healthy = archive.IsHealthy();
        const bool connected = archive.IsConnected();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveConformance: after UnloadArchive: FileExists=" << exists << " DirExists=" << dirExists << " size=" << size
            << " files=" << (UInt32) files.size() << " dirs=" << (UInt32) dirs.size() << " opened=" << opened << " metadata=" << gotMetaData
            << " healthy=" << healthy << " connected=" << connected << " hashLength=" << hash.Length() << " time=" << time.ToUnixEpochBasedTicksInMillisecs() );

        // what is still answered is answered right: a reference archive that has nothing to release carries on, an archive that
        // let go of its storage says so with an empty answer, never with another answer than it gave before
        if ( exists && size > 0 )
            ASSERT_TRUE( 65536 == size );
        if ( opened )
            ASSERT_TRUE( 1 == after.access->GetSize() || 0 == after.access->GetSize() );

        // the access that was open does not crash when used or released
        if ( wasOpened )
        {
            CORE::CDynamicBuffer buffer( 100, true );
            open.access->Read( buffer.GetBufferPtr(), 1, 100 );
            open.Release();
        }
        after.Release();

        ASSERT_TRUE( archive.UnloadArchive() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformArchiveConformanceSuite( CArchiveConformanceFixture& fixture )
{
    const CORE::CString suiteName = "ArchiveConformance: " + fixture.GetName();
    GUCEF_TESTFW_SUITE_SCOPE( suiteName.C_String() );

    bool setupGood = false;
    GUCEF_TESTFW_TESTCASE( "Setup: the fixture makes the archive with the reference tree" )
    try
    {
        setupGood = fixture.Setup();
        ASSERT_TRUE( setupGood );
        ASSERT_TRUE( GUCEF_NULL != fixture.GetArchive() );
        setupGood = setupGood && GUCEF_NULL != fixture.GetArchive();
    }
    catch ( ... )
    {
        setupGood = false;
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !setupGood )
    {
        fixture.Teardown();
        return;
    }

    try
    {
        SSuiteContext ctx;
        ctx.fixture = &fixture;
        ctx.archive = fixture.GetArchive();
        ctx.tree = &fixture.GetTree();
        ctx.caps = &fixture.GetCapabilities();

        TestArchiveState( ctx );
        TestExistenceAndSize( ctx );
        TestNamesWithSpacesAndNonAscii( ctx );
        TestFileLists( ctx );
        TestDirLists( ctx );
        TestReadingFiles( ctx );
        TestLargeFile( ctx );
        TestFileInformation( ctx );
        TestParallelReads( ctx );
        TestChangeDetection( ctx );
        TestWriting( ctx );
        TestUnload( ctx );
    }
    catch ( ... )
    {
        GUCEF_TESTFW_ERRORHERE;
    }

    fixture.Teardown();
}

/*-------------------------------------------------------------------------*/
