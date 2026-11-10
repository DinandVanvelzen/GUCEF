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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestArchiveUtils.h"
#include "TestZipArchiveWriter.h"

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

/*-------------------------------------------------------------------------*/

static bool
LoadZipPlugin( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.CreateArchiveWriter( "zip" ).IsNULL() )
        TestArchiveUtils::LoadVfsPlugin( "vfspluginZIP" );
    return !vfs.CreateArchiveWriter( "zip" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

/** How many of the listed names end with the given text */
static UInt32
CountEndingWith( const CORE::CString::StringVector& listed, const CORE::CString& ending )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)listed.size(); ++i )
        if ( listed[ i ].EndsWith( ending ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

static const char* ZipCheckScript =
    "import sys, zipfile, struct\n"
    "path, mode = sys.argv[1], sys.argv[2]\n"
    "z = zipfile.ZipFile(path)\n"
    "bad = z.testzip()\n"
    "if bad is not None: sys.exit('corrupt entry ' + bad)\n"
    "def ut_mtime(info):\n"
    "    extra = info.extra\n"
    "    while len(extra) >= 4:\n"
    "        hid, size = struct.unpack('<HH', extra[:4])\n"
    "        if hid == 0x5455: return struct.unpack('<I', extra[5:9])[0]\n"
    "        extra = extra[4+size:]\n"
    "    return None\n"
    "if mode == 'basic':\n"
    "    expected = set(sys.argv[5:])\n"
    "    names = set(z.namelist())\n"
    "    if names != expected: sys.exit('names differ: %r vs %r' % (sorted(names), sorted(expected)))\n"
    "    for info in z.infolist():\n"
    "        if not info.flag_bits & 0x800: sys.exit('no UTF-8 flag on ' + info.filename)\n"
    "        if info.create_system != 3: sys.exit('not created on unix: ' + info.filename)\n"
    "        kind = (info.external_attr >> 16) & 0o170000\n"
    "        if info.is_dir() != (kind == 0o040000): sys.exit('wrong unix mode on ' + info.filename)\n"
    "        if ut_mtime(info) is None: sys.exit('no extended timestamp on ' + info.filename)\n"
    "    info = z.getinfo(sys.argv[3])\n"
    "    if ut_mtime(info) != int(sys.argv[4]): sys.exit('wrong UTC time %r' % ut_mtime(info))\n"
    "    if info.date_time != (2021, 6, 15, 12, 34, 56): sys.exit('wrong DOS time %r' % (info.date_time,))\n"
    "elif mode == 'count':\n"
    "    if len(z.infolist()) != int(sys.argv[3]): sys.exit('wrong number of entries %d' % len(z.infolist()))\n"
    "elif mode == 'large':\n"
    "    for name in sys.argv[4:]:\n"
    "        if z.getinfo(name).file_size != int(sys.argv[3]): sys.exit('wrong size of ' + name)\n"
    "print('zip check OK')\n";

/** checks the archive with Python's zipfile, skipped when Python is not available */
static bool
CheckWithPython( const CORE::CString& scriptPath, const CORE::CString& zipPath, const CORE::CString& mode, const CORE::CProcess::TStringVector& extraArgs )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( zipPath );
    args.push_back( mode );
    args.insert( args.end(), extraArgs.begin(), extraArgs.end() );
    return TestArchiveUtils::RunPythonCheck( scriptPath, args );
}

/*-------------------------------------------------------------------------*/

/** checks the archive with Info-ZIP unzip, skipped when unzip is not available */
static bool
CheckWithUnzip( const CORE::CString& zipPath )
{GUCEF_TRACE;

    if ( !TestArchiveUtils::IsToolAvailable( "unzip", "-v" ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter tests: unzip not available, skipping the unzip check" );
        return true;
    }
    CORE::CProcess::TStringVector args;
    args.push_back( "-tqq" );
    args.push_back( zipPath );
    return 0 == TestArchiveUtils::RunExternalProgram( "unzip", args );
}

/*-------------------------------------------------------------------------*/

/** Reads one file of a mounted archive over and over on a thread of its own and counts the reads that differ from what it should be */
class CZipReaderThread : public MT::CActiveObject
{
    public:

    CZipReaderThread( const CORE::CString& path, const CORE::CDynamicBuffer& expected, UInt32 reads )
        : MT::CActiveObject(), m_path( path ), m_expected( expected ), m_readsLeft( reads ), m_failures( 0 )
    {GUCEF_TRACE;
    }

    UInt32 GetFailures( void ) const { return m_failures; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( 0 == m_readsLeft )
            return true;
        --m_readsLeft;
        CORE::CDynamicBuffer content;
        if ( !TestArchiveUtils::ReadVfsFile( VFS::CVfsGlobal::Instance()->GetVfs(), m_path, content ) || !( content == m_expected ) )
            ++m_failures;
        return 0 == m_readsLeft;
    }

    private:

    CORE::CString m_path;
    CORE::CDynamicBuffer m_expected;
    UInt32 m_readsLeft;
    UInt32 m_failures;
};

/*-------------------------------------------------------------------------*/

void
PerformZipArchiveWriterTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ZIP archive writer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ZipArchiveWriter" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !LoadZipPlugin() )
    {
        GUCEF_TESTFW_TESTCASE( "Test: The zip plugin provides an archive writer" )
            ERRORHERE;
        GUCEF_TESTFW_TESTCASE_END
        return;
    }

    CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_ZipWriter" );
    CORE::CreateDirs( dir );
    CORE::CString scriptPath = CORE::CombinePath( dir, "check_zip.py" );
    CORE::WriteStringAsTextFile( scriptPath, ZipCheckScript );

    GUCEF_TESTFW_TESTCASE( "Test: Writers are created per archive type" )
        try
        {
            VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
            ASSERT_FALSE( writer.IsNULL() );
            if ( !writer.IsNULL() )
            {
                ASSERT_TRUE( writer->GetArchiveType() == "zip" );
                ASSERT_FALSE( writer->IsOpen() );

                // nothing can be added before an archive is opened
                CORE::CDynamicBuffer content;
                CORE::CDynamicBufferAccess contentAccess( content );
                ASSERT_FALSE( writer->AddFile( "a.txt", contentAccess, VFS::CArchiveWriterEntrySettings() ) );
                ASSERT_FALSE( writer->AddDirectory( "a", CORE::CDateTime::NowLocalDateTime() ) );
                ASSERT_TRUE( writer->Close() );
            }
            ASSERT_FALSE( vfs.CreateArchiveWriter( "pk3" ).IsNULL() );
            ASSERT_TRUE( vfs.CreateArchiveWriter( "nosuchformat" ).IsNULL() );

            VFS::CVFS::TAbstractArchiveWriterFactory::TKeySet types;
            vfs.GetListOfSupportedArchiveWriterTypes( types );
            ASSERT_TRUE( types.find( "zip" ) != types.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An archive written with files, directories and UTF-8 names reads back identically" )
        try
        {
            CORE::CString zipPath = CORE::CombinePath( dir, "basic.zip" );
            CORE::CString utf8Name = "na\xC3\xAFve/\xC3\x9Cn\xC3\xAF\x63\xC3\xB6\x64\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC.txt";

            // content: text, a compressible megabyte and an existing file
            CORE::CString text = "Hello archive";
            CORE::CDynamicBuffer textContent;
            textContent.CopyFrom( text.ByteSize()-1, text.C_String() );
            CORE::CDynamicBuffer numbers;
            for ( UInt32 i=0; i<1024*1024; ++i )
            {
                UInt8 value = (UInt8) ( i % 251 );
                numbers.Append( &value, 1 );
            }
            CORE::CString onDiskPath = CORE::CombinePath( dir, "ondisk.txt" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( onDiskPath, "from disk" ) );

            VFS::CArchiveWriterEntrySettings stored;
            stored.modificationTime = CORE::CDateTime( 2021, 6, 15, 12, 34, 56, 0, 0 );
            VFS::CArchiveWriterEntrySettings deflated;
            deflated.compression = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE;

            {
                CORE::CFileAccess output( zipPath, "wb" );
                ASSERT_TRUE( output.IsValid() );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() && output.IsValid() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    ASSERT_FALSE( writer->Open( output ) );     // already open
                    ASSERT_TRUE( writer->IsOpen() );

                    CORE::CDynamicBufferAccess textAccess( textContent );
                    ASSERT_TRUE( writer->AddFile( "readme.txt", textAccess, stored ) );
                    CORE::CDynamicBufferAccess numbersAccess( numbers );
                    ASSERT_TRUE( writer->AddFile( "data\\numbers.bin", numbersAccess, deflated ) );   // separator is normalized
                    CORE::CDynamicBufferAccess utf8Access( textContent );
                    ASSERT_TRUE( writer->AddFile( utf8Name, utf8Access, deflated ) );
                    ASSERT_TRUE( writer->AddFileFromPath( "disk/ondisk.txt", onDiskPath, stored ) );
                    ASSERT_TRUE( writer->AddDirectory( "empty", CORE::CDateTime::NowLocalDateTime() ) );
                    ASSERT_FALSE( writer->AddFile( "", textAccess, stored ) );
                    ASSERT_TRUE( writer->Close() );
                    ASSERT_FALSE( writer->IsOpen() );
                }
            }
            ASSERT_TRUE( CORE::FileSize( zipPath ) > 0 );

            // the deflated megabyte takes much less space than stored
            ASSERT_TRUE( CORE::FileSize( zipPath ) < 512*1024 );

            // read back via the VFS, which uses the zip archive reader of the same plugin
            ASSERT_TRUE( vfs.AddRoot( dir, "ZipWriterTempDir", false, false, "zipwritertemp" ) );
            ASSERT_TRUE( vfs.MountArchive( "ZipWriterBasic", "zipwritertemp/basic.zip", "zip", "zipwriterbasic", false, false ) );

            CORE::CDynamicBuffer readBack;
            ASSERT_TRUE( TestArchiveUtils::ReadVfsFile( vfs, "zipwriterbasic/readme.txt", readBack ) );
            ASSERT_TRUE( readBack == textContent );
            ASSERT_TRUE( TestArchiveUtils::ReadVfsFile( vfs, "zipwriterbasic/data/numbers.bin", readBack ) );
            ASSERT_TRUE( readBack == numbers );
            ASSERT_TRUE( TestArchiveUtils::ReadVfsFile( vfs, "zipwriterbasic/" + utf8Name, readBack ) );
            ASSERT_TRUE( readBack == textContent );
            ASSERT_TRUE( TestArchiveUtils::ReadVfsFile( vfs, "zipwriterbasic/disk/ondisk.txt", readBack ) );
            ASSERT_TRUE( readBack.GetDataSize() == 9 && 0 == memcmp( readBack.GetConstBufferPtr(), "from disk", 9 ) );

            // the mounted archive is listed as a folder is: its files and folders, also the folders without an entry of their own
            CORE::CString::StringVector listed;
            vfs.GetFileList( listed, "zipwriterbasic", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 1 == listed.size() && 1 == CountEndingWith( listed, "readme.txt" ) );
            listed.clear();
            vfs.GetFileList( listed, "zipwriterbasic", true, true, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 4 == listed.size() && 1 == CountEndingWith( listed, "zipwriterbasic/data/numbers.bin" ) && 1 == CountEndingWith( listed, "zipwriterbasic/" + utf8Name ) );
            listed.clear();
            vfs.GetFileList( listed, "zipwriterbasic/data", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 1 == listed.size() && "numbers.bin" == listed[ 0 ] );
            listed.clear();
            vfs.GetDirList( listed, "zipwriterbasic", false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 4 == listed.size() && 1 == CountEndingWith( listed, "data/" ) && 1 == CountEndingWith( listed, "disk/" ) && 1 == CountEndingWith( listed, "empty/" ) );
            ASSERT_TRUE( vfs.DirExists( "zipwriterbasic/data" ) );

            // reading counts elements as fread does: one element of 5 bytes, then 5 of 1 byte
            {
                VFS::TBasicVfsResourcePtr file = vfs.GetFile( "zipwriterbasic/readme.txt", "rb", false );
                ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() );
                if ( !file.IsNULL() && !file->GetAccess().IsNULL() )
                {
                    char bytes[ 8 ];
                    CORE::IOAccessPtr access = file->GetAccess();
                    ASSERT_TRUE( 1 == access->Read( bytes, 5, 1 ) && 0 == memcmp( bytes, "Hello", 5 ) );
                    ASSERT_TRUE( 0 == access->Setpos( 0 ) && 5 == access->Read( bytes, 1, 5 ) && 0 == memcmp( bytes, "Hello", 5 ) );
                    ASSERT_TRUE( 0 == access->Setpos( 10 ) && 0 == access->Read( bytes, 4, 1 ) );    // 3 bytes left: no whole element
                }
            }

            // two files of the archive open: after the one read last is closed, the other still seeks and reads
            {
                VFS::TBasicVfsResourcePtr first = vfs.GetFile( "zipwriterbasic/data/numbers.bin", "rb", false );
                VFS::TBasicVfsResourcePtr second = vfs.GetFile( "zipwriterbasic/readme.txt", "rb", false );
                ASSERT_TRUE( !first.IsNULL() && !second.IsNULL() );
                if ( !first.IsNULL() && !second.IsNULL() )
                {
                    UInt8 value = 0;
                    ASSERT_TRUE( 1 == first->GetAccess()->Read( &value, 1, 1 ) );
                    first.Unlink();
                    char bytes[ 4 ];
                    CORE::IOAccessPtr access = second->GetAccess();
                    ASSERT_TRUE( 0 == access->Setpos( 1 ) && 4 == access->Read( bytes, 1, 4 ) && 0 == memcmp( bytes, "ello", 4 ) );
                }
            }

            // a file that is not there has no metadata
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( !vfs.GetFileMetaData( "zipwriterbasic/nothing.txt", metaData ) || !metaData.resourceExists );
            ASSERT_TRUE( vfs.GetFileMetaData( "zipwriterbasic/readme.txt", metaData ) && metaData.resourceExists && metaData.resourceSizeInBytes == textContent.GetDataSize() );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "ZipWriterBasic" ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "ZipWriterTempDir" ) );

            // other implementations agree: names, UTF-8 flag, unix attributes and both timestamps
            CORE::CProcess::TStringVector extraArgs;
            extraArgs.push_back( "readme.txt" );
            extraArgs.push_back( "1623760496" );    // 2021-06-15 12:34:56 UTC
            extraArgs.push_back( "readme.txt" );
            extraArgs.push_back( "data/numbers.bin" );
            extraArgs.push_back( utf8Name );
            extraArgs.push_back( "disk/ondisk.txt" );
            extraArgs.push_back( "empty/" );
            ASSERT_TRUE( CheckWithPython( scriptPath, zipPath, "basic", extraArgs ) );
            ASSERT_TRUE( CheckWithUnzip( zipPath ) );

            // the archive is kept for inspection, it is overwritten by the next run
            CORE::DeleteFile( onDiskPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Files of one archive read on several threads at once read back identically" )
        try
        {
            // the files of an archive share its file handle, as music streamed on one thread while pictures load on another
            CORE::CString zipPath = CORE::CombinePath( dir, "threads.zip" );
            const UInt32 fileCount = 4;
            CORE::CDynamicBuffer contents[ fileCount ];
            for ( UInt32 f=0; f<fileCount; ++f )
                for ( UInt32 i=0; i<256*1024; ++i )
                {
                    UInt8 value = (UInt8) ( ( i * ( f + 3 ) + i / 997 ) % 241 );
                    contents[ f ].Append( &value, 1 );
                }
            {
                CORE::CFileAccess output( zipPath, "wb" );
                VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                ASSERT_TRUE( output.IsValid() && !writer.IsNULL() );
                if ( !writer.IsNULL() && output.IsValid() )
                {
                    ASSERT_TRUE( writer->Open( output ) );
                    VFS::CArchiveWriterEntrySettings deflated;
                    deflated.compression = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE;
                    for ( UInt32 f=0; f<fileCount; ++f )
                    {
                        CORE::CDynamicBufferAccess access( contents[ f ] );
                        ASSERT_TRUE( writer->AddFile( "file" + CORE::ToString( f ) + ".bin", access, deflated ) );
                    }
                    ASSERT_TRUE( writer->Close() );
                }
            }
            ASSERT_TRUE( vfs.AddRoot( dir, "ZipThreadsTempDir", false, false, "zipthreadstemp" ) );
            ASSERT_TRUE( vfs.MountArchive( "ZipThreads", "zipthreadstemp/threads.zip", "zip", "zipthreads", false, false ) );

            CZipReaderThread* readers[ fileCount ];
            for ( UInt32 f=0; f<fileCount; ++f )
                readers[ f ] = GUCEF_NEW CZipReaderThread( "zipthreads/file" + CORE::ToString( f ) + ".bin", contents[ f ], 25 );
            for ( UInt32 f=0; f<fileCount; ++f )
                ASSERT_TRUE( readers[ f ]->Activate( GUCEF_NULL, 0, 0 ) );
            UInt32 failures = 0;
            for ( UInt32 f=0; f<fileCount; ++f )
            {
                readers[ f ]->WaitForThreadToFinish( 120000 );
                failures += readers[ f ]->GetFailures();
                GUCEF_DELETE readers[ f ];
            }
            ASSERT_TRUE( 0 == failures );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "ZipThreads" ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "ZipThreadsTempDir" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An archive with more than 65535 entries uses ZIP64" )
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

            CORE::CProcess::TStringVector extraArgs;
            extraArgs.push_back( CORE::UInt32ToString( entryCount ) );
            ASSERT_TRUE( CheckWithPython( scriptPath, zipPath, "count", extraArgs ) );
            ASSERT_TRUE( CheckWithUnzip( zipPath ) );
            CORE::DeleteFile( zipPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( TestArchiveUtils::LargeFileTestsEnabled() )
    {
        GUCEF_TESTFW_TESTCASE( "Test: Entries of more than 4 GiB use ZIP64, with known and unknown sizes" )
            try
            {
                const UInt64 entrySize = 4500ULL * 1024 * 1024;
                CORE::CString zipPath = CORE::CombinePath( dir, "large.zip" );
                {
                    CORE::CFileAccess output( zipPath, "wb" );
                    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
                    ASSERT_FALSE( writer.IsNULL() );
                    if ( !writer.IsNULL() )
                    {
                        ASSERT_TRUE( writer->Open( output ) );
                        VFS::CArchiveWriterEntrySettings settings;
                        settings.compression = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE;
                        settings.compressionLevel = 1;

                        TestArchiveUtils::CZeroSource knownSize( entrySize, true );
                        ASSERT_TRUE( writer->AddFile( "known.bin", knownSize, settings ) );
                        TestArchiveUtils::CZeroSource unknownSize( entrySize, false );
                        ASSERT_TRUE( writer->AddFile( "unknown.bin", unknownSize, settings ) );
                        ASSERT_TRUE( writer->Close() );
                    }
                }

                CORE::CProcess::TStringVector extraArgs;
                extraArgs.push_back( CORE::UInt64ToString( entrySize ) );
                extraArgs.push_back( "known.bin" );
                extraArgs.push_back( "unknown.bin" );
                ASSERT_TRUE( CheckWithPython( scriptPath, zipPath, "large", extraArgs ) );
                ASSERT_TRUE( CheckWithUnzip( zipPath ) );
                CORE::DeleteFile( zipPath );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ZIP archive writer tests: skipping the test with entries of more than 4 GiB, it takes minutes. Use -large or set GUCEF_TEST_LARGE_FILES=1 to run it" );
    }

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL ZIP archive writer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
