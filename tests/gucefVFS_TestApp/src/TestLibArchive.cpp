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
#include <vector>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestArchiveUtils.h"
#include "TestLibArchive.h"

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

using namespace GUCEF;
using namespace TestArchiveUtils;

/*-------------------------------------------------------------------------*/

static const char* TarCheckScript =
    "import sys, tarfile, zipfile, os\n"
    "path, mode = sys.argv[1], sys.argv[2]\n"
    "if mode == 'create':\n"
    "    # archives as made by other tools, with long and UTF-8 names\n"
    "    longname = 'deep/' + 'x' * 120 + '/file.txt'\n"
    "    for fmt, name in ((tarfile.GNU_FORMAT, 'gnu.tar'), (tarfile.PAX_FORMAT, 'pax.tar')):\n"
    "        with tarfile.open(os.path.join(path, name), 'w', format=fmt) as t:\n"
    "            for entry in (longname, 'na\\u00efve/\\u65e5\\u672c.txt', 'plain.txt'):\n"
    "                data = ('content of ' + entry).encode('utf-8')\n"
    "                info = tarfile.TarInfo(entry)\n"
    "                info.size = len(data)\n"
    "                info.mtime = 1623760496\n"
    "                import io\n"
    "                t.addfile(info, io.BytesIO(data))\n"
    "    sys.exit(0)\n"
    "if mode in ('tar', 'tgz'):\n"
    "    t = tarfile.open(path)\n"
    "    names = set(m.name.rstrip('/') for m in t.getmembers())\n"
    "    expected = set(n.rstrip('/') for n in sys.argv[5:])\n"
    "    if not expected <= names: sys.exit('missing entries %r' % sorted(expected - names))\n"
    "    m = t.getmember(sys.argv[3])\n"
    "    if int(m.mtime) != int(sys.argv[4]): sys.exit('wrong mtime %r' % m.mtime)\n"
    "    for m in t.getmembers():\n"
    "        if m.isfile() and len(t.extractfile(m).read()) != m.size: sys.exit('wrong size of ' + m.name)\n"
    "elif mode == 'zip':\n"
    "    z = zipfile.ZipFile(path)\n"
    "    bad = z.testzip()\n"
    "    if bad is not None: sys.exit('corrupt entry ' + bad)\n"
    "    names = set(n.rstrip('/') for n in z.namelist())\n"
    "    expected = set(n.rstrip('/') for n in sys.argv[5:])\n"
    "    if not expected <= names: sys.exit('missing entries %r' % sorted(expected - names))\n"
    "elif mode == 'large':\n"
    "    t = tarfile.open(path)\n"
    "    if t.getmember(sys.argv[4]).size != int(sys.argv[3]): sys.exit('wrong size')\n"
    "print('archive check OK')\n";

/*-------------------------------------------------------------------------*/

/** one archive format the plugin can write and mount */
struct SFormat
{
    const char* writerType;
    const char* extension;
    bool checkWithPythonTar;
    bool checkWithPythonZip;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
LongPath( void )
{
    // over the 100 characters of the classic tar name field
    return "long/" + CORE::CString( "abcdefghij" ) + "abcdefghij" + "abcdefghij" + "abcdefghij" + "abcdefghij" +
           "abcdefghij" + "abcdefghij" + "abcdefghij" + "abcdefghij" + "abcdefghij" + "abcdefghij" + "abcdefghij" + "/file.txt";
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Utf8Name( void )
{
    return "na\xC3\xAFve/\xC3\x9Cn\xC3\xAF\x63\xC3\xB6\x64\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC.txt";
}

/*-------------------------------------------------------------------------*/

void
PerformLibArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING libarchive plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "LibArchive" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool pluginLoaded = LoadVfsPlugin( "vfspluginLIBARCHIVE" );

    CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_LibArchive" );
    CORE::CreateDirs( dir );
    CORE::CString scriptPath = CORE::CombinePath( dir, "check_archive.py" );
    CORE::WriteStringAsTextFile( scriptPath, TarCheckScript );
    vfs.AddRoot( dir, "LibArchiveTempDir", false, false, "libarchivetemp" );

    GUCEF_TESTFW_TESTCASE( "Test: The plugin provides readers and writers for its formats" )
        try
        {
            ASSERT_TRUE( pluginLoaded );
            const char* writerTypes[] = { "tar", "tgz", "tar.gz", "zip", "7z", "cpio" };
            for ( UInt32 i=0; i<sizeof( writerTypes ) / sizeof( writerTypes[ 0 ] ); ++i )
            {
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( writerTypes[ i ] );
                ASSERT_FALSE( writer.IsNULL() );
            }

            VFS::CVFS::TAbstractArchiveFactory::TKeySet readerTypes;
            vfs.GetListOfSupportedArchiveTypes( readerTypes );
            const char* someReaderTypes[] = { "tar", "tgz", "7z", "rar", "iso", "cpio", "zip" };
            for ( UInt32 i=0; i<sizeof( someReaderTypes ) / sizeof( someReaderTypes[ 0 ] ); ++i )
                ASSERT_TRUE( readerTypes.find( someReaderTypes[ i ] ) != readerTypes.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    const SFormat formats[] = { { "tar", "tar", true, false },
                                { "tgz", "tgz", true, false },
                                { "zip", "zip", false, true },
                                { "7z", "7z", false, false },
                                { "cpio", "cpio", false, false } };

    for ( UInt32 f=0; f<sizeof( formats ) / sizeof( formats[ 0 ] ); ++f )
    {
        const SFormat& format = formats[ f ];
        CORE::CString caseName = CORE::CString( "Test: A " ) + format.writerType + " archive written by the plugin mounts and reads back identically";

        GUCEF_TESTFW_TESTCASE( caseName.C_String() )
            try
            {
                CORE::CString archiveFile = CORE::CString( "basic." ) + format.extension;
                CORE::CString archivePath = CORE::CombinePath( dir, archiveFile );
                CORE::CString mountPoint = CORE::CString( "libarchive_" ) + format.extension;

                CORE::CString text = "Hello archive";
                CORE::CDynamicBuffer textContent;
                textContent.CopyFrom( text.ByteSize()-1, text.C_String() );
                CORE::CDynamicBuffer numbers;
                for ( UInt32 i=0; i<1024*1024; ++i )
                {
                    UInt8 value = (UInt8) ( i % 251 );
                    numbers.Append( &value, 1 );
                }

                VFS::CArchiveWriterEntrySettings fixedTime;
                fixedTime.modificationTime = CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 );
                VFS::CArchiveWriterEntrySettings deflated;
                deflated.compression = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE;

                {
                    CORE::CFileAccess output( archivePath, "wb" );
                    ASSERT_TRUE( output.IsValid() );
                    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( format.writerType );
                    ASSERT_FALSE( writer.IsNULL() );
                    if ( !writer.IsNULL() && output.IsValid() )
                    {
                        ASSERT_TRUE( writer->GetArchiveType() == format.writerType );
                        ASSERT_TRUE( writer->Open( output ) );
                        CORE::CDynamicBufferAccess textAccess( textContent );
                        ASSERT_TRUE( writer->AddFile( "readme.txt", textAccess, fixedTime ) );
                        CORE::CDynamicBufferAccess numbersAccess( numbers );
                        ASSERT_TRUE( writer->AddFile( "data/numbers.bin", numbersAccess, deflated ) );
                        CORE::CDynamicBufferAccess utf8Access( textContent );
                        ASSERT_TRUE( writer->AddFile( Utf8Name(), utf8Access, deflated ) );
                        CORE::CDynamicBufferAccess longAccess( textContent );
                        ASSERT_TRUE( writer->AddFile( LongPath(), longAccess, fixedTime ) );
                        CZeroSource unknownSize( 3000, false );
                        ASSERT_TRUE( writer->AddFile( "stream/unknown.bin", unknownSize, fixedTime ) );
                        ASSERT_TRUE( writer->AddDirectory( "empty", fixedTime.modificationTime ) );
                        ASSERT_TRUE( writer->Close() );
                    }
                }

                // mount and read back
                ASSERT_TRUE( vfs.MountArchive( mountPoint, "libarchivetemp/" + archiveFile, format.writerType, mountPoint, false, false ) );

                CORE::CDynamicBuffer readBack;
                ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/readme.txt", readBack ) && readBack == textContent );
                ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/data/numbers.bin", readBack ) && readBack == numbers );
                ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/" + Utf8Name(), readBack ) && readBack == textContent );
                ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/" + LongPath(), readBack ) && readBack == textContent );
                ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/stream/unknown.bin", readBack ) && 3000 == readBack.GetDataSize() );

                ASSERT_TRUE( vfs.FileExists( mountPoint + "/data/numbers.bin" ) );
                ASSERT_FALSE( vfs.FileExists( mountPoint + "/data/nothing.bin" ) );
                ASSERT_TRUE( vfs.DirExists( mountPoint + "/data" ) );
                ASSERT_TRUE( vfs.DirExists( mountPoint + "/empty" ) );
                ASSERT_TRUE( 1024*1024 == vfs.GetFileSize( mountPoint + "/data/numbers.bin" ) );

                // the modification time is kept to the second
                CORE::CDateTime modified = vfs.GetFileModificationTime( mountPoint + "/readme.txt" );
                ASSERT_TRUE( 1623760496000ULL == modified.ToUnixEpochBasedTicksInMillisecs() );

                VFS::CVFS::TStringVector files;
                vfs.GetFileList( files, mountPoint + "/data", false, false, CORE::CString::EmptyStringSet );
                ASSERT_TRUE( 1 == files.size() && files[ 0 ] == "numbers.bin" );
                files.clear();
                vfs.GetFileList( files, mountPoint, true, false, CORE::CString::EmptyStringSet );
                ASSERT_TRUE( 5 == files.size() );     // the directory is not a file

                // reading part of a file from the middle after reading further on
                VFS::TBasicVfsResourcePtr numbersFile = vfs.GetFile( mountPoint + "/data/numbers.bin", "rb", false );
                ASSERT_FALSE( numbersFile.IsNULL() );
                if ( !numbersFile.IsNULL() )
                {
                    CORE::IOAccessPtr access = numbersFile->GetAccess();
                    char chunk[ 4 ];
                    ASSERT_TRUE( 0 == access->Seek( 1000000, SEEK_SET ) && 4 == access->Read( chunk, 1, 4 ) );
                    ASSERT_TRUE( 0 == access->Seek( 251, SEEK_SET ) && 4 == access->Read( chunk, 1, 4 ) );
                    ASSERT_TRUE( 0 == chunk[ 0 ] && 1 == chunk[ 1 ] && 2 == chunk[ 2 ] && 3 == chunk[ 3 ] );
                }
                numbersFile = VFS::TBasicVfsResourcePtr();

                ASSERT_TRUE( vfs.UnmountArchiveByName( mountPoint ) );

                // other implementations agree
                CORE::CProcess::TStringVector args;
                args.push_back( archivePath );
                args.push_back( format.checkWithPythonZip ? "zip" : format.extension );
                args.push_back( "readme.txt" );
                args.push_back( "1623760496" );
                args.push_back( "readme.txt" );
                args.push_back( "data/numbers.bin" );
                args.push_back( Utf8Name() );
                args.push_back( LongPath() );
                args.push_back( "stream/unknown.bin" );
                args.push_back( "empty/" );
                if ( format.checkWithPythonTar || format.checkWithPythonZip )
                    ASSERT_TRUE( RunPythonCheck( scriptPath, args ) );
                if ( format.checkWithPythonTar && IsToolAvailable( "tar", "--version" ) )
                {
                    CORE::CProcess::TStringVector tarArgs;
                    tarArgs.push_back( "-tf" );
                    tarArgs.push_back( archivePath );
                    ASSERT_TRUE( 0 == RunExternalProgram( "tar", tarArgs ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Test: Tar archives made by other tools mount and read back" )
        try
        {
            if ( IsToolAvailable( "python3", "--version" ) )
            {
                CORE::CProcess::TStringVector args;
                args.push_back( dir );
                args.push_back( "create" );
                ASSERT_TRUE( RunPythonCheck( scriptPath, args ) );

                CORE::CString longName = "deep/" + CORE::CString( "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" ) + "/file.txt";
                CORE::CString utf8Name = "na\xC3\xAFve/\xE6\x97\xA5\xE6\x9C\xAC.txt";
                const char* archives[] = { "gnu.tar", "pax.tar" };
                for ( UInt32 a=0; a<2; ++a )
                {
                    CORE::CString mountPoint = CORE::CString( "libarchive_other_" ) + CORE::ToString( a );
                    ASSERT_TRUE( vfs.MountArchive( mountPoint, CORE::CString( "libarchivetemp/" ) + archives[ a ], "tar", mountPoint, false, false ) );

                    const CORE::CString names[] = { longName, utf8Name, "plain.txt" };
                    for ( UInt32 n=0; n<3; ++n )
                    {
                        CORE::CString expected = "content of " + names[ n ];
                        CORE::CDynamicBuffer readBack;
                        ASSERT_TRUE( ReadVfsFile( vfs, mountPoint + "/" + names[ n ], readBack ) );
                        ASSERT_TRUE( readBack.GetDataSize() == expected.ByteSize()-1 && 0 == memcmp( readBack.GetConstBufferPtr(), expected.C_String(), readBack.GetDataSize() ) );
                    }
                    ASSERT_TRUE( vfs.UnmountArchiveByName( mountPoint ) );
                }
            }

            // and one made by GNU tar from a directory
            if ( IsToolAvailable( "tar", "--version" ) )
            {
                CORE::CString sourceDir = CORE::CombinePath( dir, "gnutarsource" );
                ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( sourceDir, "sub" ) ) );
                ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( sourceDir, "sub/a.txt" ), "made by GNU tar" ) );

                CORE::CProcess::TStringVector tarArgs;
                tarArgs.push_back( "-czf" );
                tarArgs.push_back( CORE::CombinePath( dir, "gnutar.tgz" ) );
                tarArgs.push_back( "-C" );
                tarArgs.push_back( sourceDir );
                tarArgs.push_back( "sub" );
                ASSERT_TRUE( 0 == RunExternalProgram( "tar", tarArgs ) );

                ASSERT_TRUE( vfs.MountArchive( "libarchive_gnutar", "libarchivetemp/gnutar.tgz", "tgz", "libarchive_gnutar", false, false ) );
                CORE::CDynamicBuffer readBack;
                ASSERT_TRUE( ReadVfsFile( vfs, "libarchive_gnutar/sub/a.txt", readBack ) );
                ASSERT_TRUE( readBack.GetDataSize() == 15 && 0 == memcmp( readBack.GetConstBufferPtr(), "made by GNU tar", 15 ) );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_gnutar" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A zip archive with more than 65535 entries mounts with all its entries" )
        try
        {
            const UInt32 entryCount = 70000;
            CORE::CString zipPath = CORE::CombinePath( dir, "many.zip" );
            {
                CORE::CFileAccess output( zipPath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    CORE::CDynamicBuffer content;
                    content.CopyFrom( 1, "x" );
                    VFS::CArchiveWriterEntrySettings settings;
                    bool allAdded = true;
                    for ( UInt32 i=0; i<entryCount && allAdded; ++i )
                    {
                        CORE::CDynamicBufferAccess access( content );
                        allAdded = writer->AddFile( "entries/" + CORE::UInt32ToString( i ) + ".txt", access, settings );
                    }
                    ASSERT_TRUE( allAdded );
                    ASSERT_TRUE( writer->Close() );
                }
            }

            ASSERT_TRUE( vfs.MountArchive( "libarchive_many", "libarchivetemp/many.zip", "zip", "libarchive_many", false, false ) );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "libarchive_many/entries", false, false, CORE::CString::EmptyStringSet, entryCount + 10 );
            ASSERT_TRUE( entryCount == files.size() );
            CORE::CDynamicBuffer readBack;
            ASSERT_TRUE( ReadVfsFile( vfs, "libarchive_many/entries/69999.txt", readBack ) && 1 == readBack.GetDataSize() );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_many" ) );
            CORE::DeleteFile( zipPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( TestArchiveUtils::LargeFileTestsEnabled() )
    {
        GUCEF_TESTFW_TESTCASE( "Test: A tar entry of more than 8 GiB uses pax and reads back" )
            try
            {
                const UInt64 entrySize = 9ULL * 1024 * 1024 * 1024;
                CORE::CString archivePath = CORE::CombinePath( dir, "large.tgz" );
                {
                    CORE::CFileAccess output( archivePath, "wb" );
                    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "tgz" );
                    ASSERT_FALSE( writer.IsNULL() );
                    if ( !writer.IsNULL() )
                    {
                        ASSERT_TRUE( writer->Open( output ) );
                        CZeroSource zeros( entrySize, true );
                        ASSERT_TRUE( writer->AddFile( "huge.bin", zeros, VFS::CArchiveWriterEntrySettings() ) );
                        ASSERT_TRUE( writer->Close() );
                    }
                }

                CORE::CProcess::TStringVector args;
                args.push_back( archivePath );
                args.push_back( "large" );
                args.push_back( CORE::UInt64ToString( entrySize ) );
                args.push_back( "huge.bin" );
                ASSERT_TRUE( RunPythonCheck( scriptPath, args ) );

                ASSERT_TRUE( vfs.MountArchive( "libarchive_large", "libarchivetemp/large.tgz", "tgz", "libarchive_large", false, false ) );
                ASSERT_TRUE( entrySize == vfs.GetFileSize( "libarchive_large/huge.bin" ) );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_large" ) );
                CORE::DeleteFile( archivePath );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "libarchive plugin tests: skipping the test with an entry of more than 8 GiB, it takes minutes. Use -large or set GUCEF_TEST_LARGE_FILES=1 to run it" );
    }

    vfs.UnmountArchiveByName( "LibArchiveTempDir" );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL libarchive plugin TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
