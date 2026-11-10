/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_ISOIMAGEWRITER_H
#include "gucef_test_isoimagewriter.h"
#define GUCEF_TEST_ISOIMAGEWRITER_H
#endif /* GUCEF_TEST_ISOIMAGEWRITER_H ? */

#include "TestArchiveUtils.h"
#include "TestIso9660Archive.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace TestArchiveUtils;

/*-------------------------------------------------------------------------//
//                                                                         //
//      A SMALL ISO 9660 IMAGE WRITER                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

using namespace GUCEF::TESTFW::ISO;

/** Content for the test files: recognisable and different per file */
CORE::CDynamicBuffer
MakeContent( UInt32 size, UInt32 seed )
{
    CORE::CDynamicBuffer content( size, true );
    UInt32 x = 2463534242U + seed;
    for ( UInt32 i = 0; i < size; ++i )
    {
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        const UInt8 byte = (UInt8) ( x & 0xFF );
        content.Append( &byte, 1 );
    }
    return content;
}

/** The image writer is test framework support, outside the libraries, and takes its content as std::string */
std::string
AsFileContent( const CORE::CDynamicBuffer& content )
{
    return std::string( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
}

CORE::CDynamicBuffer
TextContent( const char* text )
{
    CORE::CDynamicBuffer content;
    content.Append( text, (UInt32) strlen( text ) );
    return content;
}

CORE::IOAccessPtr
MakeAccess( const TBytes& bytes )
{
    CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer();
    buffer->CopyFrom( (UInt32) bytes.size(), bytes.empty() ? static_cast< const void* >( "" ) : static_cast< const void* >( &bytes[ 0 ] ) );
    return CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
}

bool
HasName( const VFS::CVFS::TStringVector& list, const char* name )
{
    for ( size_t i = 0; i < list.size(); ++i )
        if ( list[ i ] == name )
            return true;
    return false;
}

bool
SameContent( VFS::CVFS& vfs, const CORE::CString& vfsPath, const CORE::CDynamicBuffer& expected )
{
    CORE::CDynamicBuffer buffer;
    if ( !vfs.LoadFile( buffer, vfsPath, "rb" ) )
        return false;
    return buffer.GetDataSize() == expected.GetDataSize() &&
           ( 0 == expected.GetDataSize() || 0 == memcmp( buffer.GetConstBufferPtr(), expected.GetConstBufferPtr(), expected.GetDataSize() ) );
}

bool
SameContent( VFS::CVFS& vfs, const CORE::CString& vfsPath, const char* expectedText )
{
    return SameContent( vfs, vfsPath, TextContent( expectedText ) );
}

/** A tree that exercises: nesting, a file with a larger content than the VFS loads into memory, an empty file, similar names */
SNode
MakeTestTree( const CORE::CDynamicBuffer& bigContent )
{
    SNode root = Dir( "", "" );
    root.children.push_back( File( "ReadMe.txt", "README.TXT;1", "hello from the root" ) );
    root.children.push_back( File( "NOEXT", "NOEXT.;1", "a file without extension" ) );
    root.children.push_back( File( "empty.dat", "EMPTY.DAT;1", "" ) );
    root.children.push_back( File( "big.bin", "BIG.BIN;1", AsFileContent( bigContent ) ) );
    SNode data = Dir( "Data", "DATA" );
    data.children.push_back( File( "Level1.map", "LEVEL1.MAP;1", AsFileContent( MakeContent( 700, 1 ) ) ) );
    data.children.push_back( File( "Level2.map", "LEVEL2.MAP;1", AsFileContent( MakeContent( 5000, 2 ) ) ) );
    SNode deep = Dir( "Deep", "DEEP" );
    deep.children.push_back( File( "Deeper.txt", "DEEPER.TXT;1", "deep content" ) );
    data.children.push_back( deep );
    root.children.push_back( data );
    return root;
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
CheckTestTree( VFS::CVFS& vfs, const char* mountPoint, const CORE::CDynamicBuffer& bigContent, bool expectLongNames )
{
    const CORE::CString root( mountPoint );

    ASSERT_TRUE( vfs.IsMountedArchive( root ) );
    ASSERT_TRUE( vfs.FileExists( root + "/README.TXT" ) || vfs.FileExists( root + "/ReadMe.txt" ) );

    // lookups ignore case
    ASSERT_TRUE( vfs.FileExists( root + "/readme.txt" ) );
    ASSERT_TRUE( vfs.FileExists( root + "/README.TXT" ) );
    ASSERT_TRUE( vfs.FileExists( root + "/data/level1.map" ) );
    ASSERT_TRUE( vfs.FileExists( root + "/DATA/DEEP/DEEPER.TXT" ) );
    ASSERT_FALSE( vfs.FileExists( root + "/data" ) );
    ASSERT_FALSE( vfs.FileExists( root + "/data/nothing.map" ) );

    // contents, small ones are loaded into memory by the VFS and larger ones are windows on the image
    ASSERT_TRUE( SameContent( vfs, root + "/readme.txt", "hello from the root" ) );
    ASSERT_TRUE( SameContent( vfs, root + "/noext", "a file without extension" ) );
    ASSERT_TRUE( SameContent( vfs, root + "/empty.dat", "" ) );
    ASSERT_TRUE( SameContent( vfs, root + "/big.bin", bigContent ) );
    ASSERT_TRUE( SameContent( vfs, root + "/data/level1.map", MakeContent( 700, 1 ) ) );
    ASSERT_TRUE( SameContent( vfs, root + "/data/level2.map", MakeContent( 5000, 2 ) ) );
    ASSERT_TRUE( SameContent( vfs, root + "/data/deep/deeper.txt", "deep content" ) );

    // sizes
    ASSERT_TRUE( vfs.GetFileSize( root + "/big.bin" ) == bigContent.GetDataSize() );
    ASSERT_TRUE( vfs.GetFileSize( root + "/data/level2.map" ) == 5000 );
    ASSERT_TRUE( vfs.GetFileSize( root + "/empty.dat" ) == 0 );

    // listings
    VFS::CVFS::TStringVector files;
    vfs.GetFileList( files, root, false, false, CORE::CString::EmptyStringSet );
    ASSERT_TRUE( files.size() == 4 );
    ASSERT_TRUE( HasName( files, "big.bin" ) || HasName( files, "BIG.BIN" ) );

    files.clear();
    vfs.GetFileList( files, root + "/data", false, false, CORE::CString::EmptyStringSet );
    ASSERT_TRUE( files.size() == 2 );

    files.clear();
    vfs.GetFileList( files, root, true, true, CORE::CString::EmptyStringSet );
    ASSERT_TRUE( files.size() == 7 );
    bool foundDeepWithPath = false;
    for ( size_t i = 0; i < files.size(); ++i )
        if ( files[ i ].Lowercase() == ( root + "/data/deep/deeper.txt" ).Lowercase() )
            foundDeepWithPath = true;
    ASSERT_TRUE( foundDeepWithPath );

    VFS::CVFS::TStringVector dirs;
    vfs.GetDirList( dirs, root, false, false, CORE::CString::EmptyStringSet );
    ASSERT_TRUE( dirs.size() == 1 );

    if ( expectLongNames )
    {
        // the names as the image stores them are kept in listings
        files.clear();
        vfs.GetFileList( files, root, false, false, CORE::CString::EmptyStringSet );
        ASSERT_TRUE( HasName( files, "ReadMe.txt" ) );
        ASSERT_TRUE( HasName( files, "NOEXT" ) );
    }
    else
    {
        files.clear();
        vfs.GetFileList( files, root, false, false, CORE::CString::EmptyStringSet );
        ASSERT_TRUE( HasName( files, "README.TXT" ) );
        ASSERT_TRUE( HasName( files, "NOEXT" ) );
    }
}

void
PerformIso9660ArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ISO 9660 plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "Iso9660Archive" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool pluginLoaded = LoadVfsPlugin( "vfspluginISO9660" );
    bool itvPluginLoaded = LoadVfsPlugin( "vfspluginITV" );

    const CORE::CDynamicBuffer bigContent = MakeContent( 70000, 7 );

    GUCEF_TESTFW_TESTCASE( "Test: The plugin provides the iso9660 archive type" )
        try
        {
            ASSERT_TRUE( pluginLoaded );
            VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
            vfs.GetListOfSupportedArchiveTypes( types );
            ASSERT_TRUE( types.find( "iso9660" ) != types.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A plain ISO 9660 image with upper case names mounts, lists and reads" )
        try
        {
            SOptions options;
            CImageWriter writer( MakeTestTree( bigContent ), options, "PLAIN_DISC" );
            TBytes image = writer.Write();

            ASSERT_TRUE( vfs.MountArchive( "isoPlain", MakeAccess( image ), "iso9660", "/isoplain", false ) );
            CheckTestTree( vfs, "/isoplain", bigContent, false );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoPlain" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A Joliet image uses its long mixed case and Unicode names" )
        try
        {
            SNode tree = MakeTestTree( bigContent );
            tree.children.push_back( File( "\xC3\x9C" "n\xC3\xAF" "code \xE6\x97\xA5\xE6\x9C\xAC.txt", "UNICODE.TXT;1", "unicode name" ) );
            tree.children.push_back( File( "A name that is a lot longer than eight dot three allows.text", "LONGNAME.TXT;1", "long name" ) );

            SOptions options;
            options.joliet = true;
            CImageWriter writer( tree, options, "JOLIET_DISC" );
            TBytes image = writer.Write();

            ASSERT_TRUE( vfs.MountArchive( "isoJoliet", MakeAccess( image ), "iso9660", "/isojoliet", false ) );

            // the 'NOEXT.' trick of the plain names does not exist in Joliet, the long name is used as is
            ASSERT_TRUE( vfs.FileExists( "/isojoliet/readme.txt" ) );
            ASSERT_TRUE( SameContent( vfs, "/isojoliet/ReadMe.txt", "hello from the root" ) );
            ASSERT_TRUE( SameContent( vfs, "/isojoliet/\xC3\x9C" "n\xC3\xAF" "code \xE6\x97\xA5\xE6\x9C\xAC.txt", "unicode name" ) );
            ASSERT_TRUE( SameContent( vfs, "/isojoliet/A name that is a lot longer than eight dot three allows.text", "long name" ) );
            ASSERT_TRUE( SameContent( vfs, "/isojoliet/data/level2.map", MakeContent( 5000, 2 ) ) );
            ASSERT_TRUE( SameContent( vfs, "/isojoliet/big.bin", bigContent ) );

            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "/isojoliet", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( HasName( files, "ReadMe.txt" ) );
            ASSERT_TRUE( HasName( files, "A name that is a lot longer than eight dot three allows.text" ) );
            ASSERT_FALSE( HasName( files, "README.TXT" ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoJoliet" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A Rock Ridge image uses its alternate names, including names that continue in another block" )
        try
        {
            CORE::CString veryLongName;
            for ( UInt32 i = 0; i < 150; ++i )
                veryLongName += 'x';
            veryLongName += ".bin";
            SNode tree = MakeTestTree( bigContent );
            tree.children.push_back( File( veryLongName.C_String(), "VERYLONG.BIN;1", "a very long name" ) );

            SOptions options;
            options.rockRidge = true;
            CImageWriter writer( tree, options, "RR_DISC" );
            TBytes image = writer.Write();

            ASSERT_TRUE( vfs.MountArchive( "isoRockRidge", MakeAccess( image ), "iso9660", "/isorr", false ) );
            ASSERT_TRUE( SameContent( vfs, "/isorr/ReadMe.txt", "hello from the root" ) );
            ASSERT_TRUE( SameContent( vfs, "/isorr/data/Level1.map", MakeContent( 700, 1 ) ) );
            ASSERT_TRUE( SameContent( vfs, "/isorr/" + veryLongName, "a very long name" ) );
            ASSERT_TRUE( SameContent( vfs, "/isorr/big.bin", bigContent ) );

            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "/isorr", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( HasName( files, "ReadMe.txt" ) );
            ASSERT_TRUE( HasName( files, veryLongName.C_String() ) );
            ASSERT_FALSE( HasName( files, "README.TXT" ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoRockRidge" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A file stored in several contiguous extents is read as one" )
        try
        {
            SNode tree = Dir( "", "" );
            SNode split = File( "split.bin", "SPLIT.BIN;1", AsFileContent( MakeContent( 3 * 2048 + 100, 11 ) ) );
            split.splitAt = 2048;
            tree.children.push_back( split );

            SOptions options;
            CImageWriter writer( tree, options, "SPLIT_DISC" );
            TBytes image = writer.Write();

            ASSERT_TRUE( vfs.MountArchive( "isoSplit", MakeAccess( image ), "iso9660", "/isosplit", false ) );
            ASSERT_TRUE( vfs.GetFileSize( "/isosplit/split.bin" ) == 3 * 2048 + 100 );
            ASSERT_TRUE( SameContent( vfs, "/isosplit/split.bin", MakeContent( 3 * 2048 + 100, 11 ) ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoSplit" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Images that are not usable are refused" )
        try
        {
            // not an ISO at all
            TBytes junk( 40 * 2048, 0x5A );
            ASSERT_FALSE( vfs.MountArchive( "isoJunk", MakeAccess( junk ), "iso9660", "/isojunk", false ) );

            // too small to hold the volume descriptors
            TBytes tiny( 100, 0 );
            ASSERT_FALSE( vfs.MountArchive( "isoTiny", MakeAccess( tiny ), "iso9660", "/isotiny", false ) );

            // a raw 2352 byte sector image starts with a sync pattern
            TBytes raw( 40 * 2352, 0 );
            const UInt8 sync[ 12 ] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
            memcpy( &raw[ 0 ], sync, 12 );
            ASSERT_FALSE( vfs.MountArchive( "isoRaw", MakeAccess( raw ), "iso9660", "/isoraw", false ) );

            // a writeable mount makes no sense
            SOptions options;
            CImageWriter writer( MakeTestTree( bigContent ), options, "RO_DISC" );
            TBytes image = writer.Write();
            ASSERT_FALSE( vfs.MountArchive( "isoWriteable", MakeAccess( image ), "iso9660", "/isowriteable", true ) );

            // the directory data of the root points at a place outside of the image
            TBytes cut( image.begin(), image.begin() + 18 * 2048 );
            ASSERT_FALSE( vfs.MountArchive( "isoCut", MakeAccess( cut ), "iso9660", "/isocut", false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A directory that points back at an earlier directory does not make the reader loop" )
        try
        {
            SNode tree = Dir( "", "" );
            tree.children.push_back( Dir( "Loop", "LOOP" ) );
            tree.children.push_back( File( "a.txt", "A.TXT;1", "a" ) );
            SOptions options;
            CImageWriter writer( tree, options, "LOOP_DISC" );
            TBytes image = writer.Write();

            // the extent of the sub directory record in the root is overwritten with the extent of the root itself
            const UInt32 rootExtent = (UInt32) image[ 16 * 2048 + 156 + 2 ] | ( (UInt32) image[ 16 * 2048 + 156 + 3 ] << 8 );
            UInt8* rootData = &image[ (size_t) rootExtent * 2048 ];
            size_t pos = rootData[ 0 ] + rootData[ rootData[ 0 ] ]; // after "." and ".."
            // the first real record is "LOOP"
            rootData[ pos + 2 ] = (UInt8) ( rootExtent & 0xFF );
            rootData[ pos + 3 ] = (UInt8) ( rootExtent >> 8 );

            ASSERT_TRUE( vfs.MountArchive( "isoLoop", MakeAccess( image ), "iso9660", "/isoloop", false ) );
            ASSERT_TRUE( vfs.FileExists( "/isoloop/a.txt" ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoLoop" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Not run by default: needs an image made by an external tool (xorriso, genisoimage, mkisofs, a disc burning program)
    // GUCEF_TEST_ISO is the image, GUCEF_TEST_ISO_SOURCE_DIR the directory it was made from
    GUCEF_TESTFW_TESTCASE( "Test: An image made by an external tool has the same files as the directory it was made from" )
        // the skips go before the try: the skip request is an exception and the catch below would take it for an error
        GUCEF_TESTFW_REQUIRE_ENV( isoPath, "GUCEF_TEST_ISO" );
        GUCEF_TESTFW_REQUIRE_ENV( sourceDir, "GUCEF_TEST_ISO_SOURCE_DIR" );
        try
        {
            ASSERT_TRUE( vfs.MountArchive( "extIso", CORE::CString( isoPath.c_str() ), "iso9660", "/extiso", false, false, CORE::CString( isoPath.c_str() ) ) );
            ASSERT_TRUE( vfs.AddRoot( CORE::CString( sourceDir.c_str() ), "ExtIsoSource", false, false, "extisosource" ) );

            VFS::CVFS::TStringVector fromIso;
            VFS::CVFS::TStringVector fromDir;
            vfs.GetFileList( fromIso, "/extiso", true, true, CORE::CString::EmptyStringSet );
            vfs.GetFileList( fromDir, "/extisosource", true, true, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( fromIso.size() > 0 );
            ASSERT_TRUE( fromIso.size() == fromDir.size() );

            // every file of the directory is on the image with the same size and content, names compared ignoring case
            UInt32 compared = 0;
            for ( size_t i = 0; i < fromDir.size(); ++i )
            {
                CORE::CString relative = fromDir[ i ].CutChars( CORE::CString( "/extisosource/" ).Length(), true, 0 );
                CORE::CString isoFile = "/extiso/" + relative;
                ASSERT_TRUE( vfs.FileExists( isoFile ) );
                ASSERT_TRUE( vfs.GetFileSize( isoFile ) == vfs.GetFileSize( fromDir[ i ] ) );

                CORE::CDynamicBuffer a;
                CORE::CDynamicBuffer b;
                ASSERT_TRUE( vfs.LoadFile( a, isoFile, "rb" ) );
                ASSERT_TRUE( vfs.LoadFile( b, fromDir[ i ], "rb" ) );
                ASSERT_TRUE( a.GetDataSize() == b.GetDataSize() );
                ASSERT_TRUE( a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
                ++compared;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Compared " << compared << " files of the external image with its source directory" );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "extIso" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An image written to disk mounts by path and an archive inside the image mounts from the opened file" )
        try
        {
            ASSERT_TRUE( itvPluginLoaded );

            // a small ITV style archive pair: index with the record type and (id, offset) pairs, and the data
            TBytes idx;
            idx.push_back( 7 ); idx.push_back( 0 );
            const char* chunk1 = "first resource";
            const char* chunk2 = "second resource, a bit longer";
            const UInt32 offsets[ 2 ] = { 0, (UInt32) strlen( chunk1 ) };
            const UInt32 ids[ 2 ] = { 10, 20 };
            for ( int i = 0; i < 2; ++i )
            {
                for ( int b = 0; b < 4; ++b ) idx.push_back( (UInt8) ( ids[ i ] >> ( 8 * b ) ) );
                for ( int b = 0; b < 4; ++b ) idx.push_back( (UInt8) ( offsets[ i ] >> ( 8 * b ) ) );
            }
            CORE::CDynamicBuffer resData = TextContent( chunk1 );
            resData.Append( chunk2, (UInt32) strlen( chunk2 ) );

            SNode tree = Dir( "", "" );
            SNode res = Dir( "RES", "RES" );
            res.children.push_back( File( "TEST.IDX", "TEST.IDX;1", std::string( (const char*) &idx[ 0 ], idx.size() ) ) );
            res.children.push_back( File( "TEST.RES", "TEST.RES;1", AsFileContent( resData ) ) );
            tree.children.push_back( res );

            SOptions options;
            CImageWriter writer( tree, options, "NESTED_DISC" );
            TBytes image = writer.Write();

            // on disk
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_Iso9660" );
            CORE::CreateDirs( dir );
            CORE::CString isoPath = CORE::CombinePath( dir, "nested.iso" );
            {
                CORE::CFileAccess out( isoPath, "wb" );
                ASSERT_TRUE( out.Opened() );
                ASSERT_TRUE( out.Write( &image[ 0 ], 1, (UInt32) image.size() ) == image.size() );
            }
            ASSERT_TRUE( vfs.MountArchive( "isoOnDisk", isoPath, "iso9660", "/isodisk", false, false, isoPath ) );
            ASSERT_TRUE( vfs.FileExists( "/isodisk/res/test.res" ) );

            // the ITV archive that lives inside the disc image: the plugin finds its index next to the data file through the VFS
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "/isodisk/RES/TEST.RES", "rb" );
            ASSERT_FALSE( resource.IsNULL() );
            if ( !resource.IsNULL() )
            {
                ASSERT_TRUE( vfs.MountArchive( "itvInIso", resource, "res", "/itviniso", false ) );
                ASSERT_TRUE( SameContent( vfs, "/itviniso/10.itv7", "first resource" ) );
                ASSERT_TRUE( SameContent( vfs, "/itviniso/20.itv7", "second resource, a bit longer" ) );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "itvInIso" ) );

                // a resource must not outlive the archive it came from: let go of it before unmounting that archive
                resource.Unlink();
            }
            ASSERT_TRUE( vfs.UnmountArchiveByName( "isoOnDisk" ) );
            CORE::DeleteFile( isoPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
