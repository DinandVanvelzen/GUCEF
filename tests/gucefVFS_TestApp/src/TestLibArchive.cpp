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
#include <stdlib.h>

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

/*-------------------------------------------------------------------------*/

/** One entry for the stored zip builder: the data is a repeatable pattern so that large entries need no memory */
struct SStoredZipEntry
{
    CORE::CString name;
    UInt64 size;
    UInt32 seed;
    UInt64 localOffset;
    UInt32 crc;
};

static UInt8
PatternByte( UInt32 seed, UInt64 position )
{
    // cheap and position dependent, a shift by one position or a wrong entry is noticed
    return (UInt8) ( ( position * 2654435761ULL ) >> 13 ) ^ (UInt8) ( seed + ( position >> 16 ) );
}

static void
FillPattern( UInt32 seed, UInt64 position, UInt8* buffer, UInt32 length )
{
    for ( UInt32 i=0; i<length; ++i )
        buffer[ i ] = PatternByte( seed, position + i );
}

static bool
CheckPattern( UInt32 seed, UInt64 position, const UInt8* buffer, UInt32 length )
{
    for ( UInt32 i=0; i<length; ++i )
        if ( buffer[ i ] != PatternByte( seed, position + i ) )
            return false;
    return true;
}

static UInt32
Crc32Update( UInt32 crc, const UInt8* data, UInt32 length )
{
    for ( UInt32 i=0; i<length; ++i )
    {
        crc ^= data[ i ];
        for ( UInt32 b=0; b<8; ++b )
            crc = ( crc >> 1 ) ^ ( 0xEDB88320U & (UInt32) -(Int32) ( crc & 1 ) );
    }
    return crc;
}

static void
PutLE( GUCEF::vector< UInt8 >& out, UInt64 value, UInt32 bytes )
{
    for ( UInt32 i=0; i<bytes; ++i )
        out.push_back( (UInt8) ( value >> ( 8 * i ) ) );
}

static void
PutText( GUCEF::vector< UInt8 >& out, const CORE::CString& text )
{
    for ( UInt32 i=0; i<text.Length(); ++i )
        out.push_back( (UInt8) text[ i ] );
}

/** Writes bytes at a position of a file that can be far beyond its end: the gap becomes a hole on file systems that have those */
static bool
WriteAt( CORE::CFileAccess& file, UInt64 position, const void* data, UInt32 length )
{
    return 0 == file.Seek( (Int64) position, SEEK_SET ) && length == file.Write( data, 1, length );
}

/**
 *  Writes a zip archive of stored entries the way zip64 writers do: sizes and offsets of 64 bits in the extra fields,
 *  the zip64 end of directory record and locator in front of the end of directory record. The entry data is written
 *  in blocks of 1 MiB for small entries; for entries of 1 GiB or more only the first and last megabyte are written,
 *  which leaves a hole in between (the content there reads as zero) and the checksum is then not valid.
 */
static bool
WriteStoredZip64( const CORE::CString& path, GUCEF::vector< SStoredZipEntry >& entries, bool useZip64 )
{
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    UInt64 position = 0;
    for ( size_t e=0; e<entries.size(); ++e )
    {
        SStoredZipEntry& entry = entries[ e ];
        entry.localOffset = position;
        const bool sparse = entry.size >= 1024ULL * 1024 * 1024;
        entry.crc = 0xFFFFFFFFU;
        GUCEF::vector< UInt8 > header;
        PutLE( header, 0x04034b50, 4 );
        PutLE( header, useZip64 ? 45 : 10, 2 );
        PutLE( header, 0, 2 );                       // flags
        PutLE( header, 0, 2 );                       // method: stored
        PutLE( header, 0, 4 );                       // time and date
        // the checksum is known after the data has been generated: it is patched in below
        const size_t crcAt = header.size();
        PutLE( header, 0, 4 );
        PutLE( header, useZip64 ? 0xFFFFFFFFULL : entry.size, 4 );
        PutLE( header, useZip64 ? 0xFFFFFFFFULL : entry.size, 4 );
        PutLE( header, entry.name.Length(), 2 );
        PutLE( header, useZip64 ? 20 : 0, 2 );
        PutText( header, entry.name );
        if ( useZip64 )
        {
            PutLE( header, 1, 2 );
            PutLE( header, 16, 2 );
            PutLE( header, entry.size, 8 );
            PutLE( header, entry.size, 8 );
        }
        // data
        UInt64 written = 0;
        GUCEF::vector< UInt8 > block( 1024 * 1024 );
        const UInt64 dataStart = position + header.size();
        while ( written < entry.size )
        {
            if ( sparse && written == 1024 * 1024 )
            {
                written = entry.size - 1024 * 1024;
                continue;
            }
            const UInt32 chunk = (UInt32) ( entry.size - written < block.size() ? entry.size - written : block.size() );
            FillPattern( entry.seed, written, &block[ 0 ], chunk );
            if ( !WriteAt( file, dataStart + written, &block[ 0 ], chunk ) )
                return false;
            if ( !sparse )
                entry.crc = Crc32Update( entry.crc, &block[ 0 ], chunk );
            written += chunk;
        }
        entry.crc = sparse ? 0 : ~entry.crc;
        for ( UInt32 i=0; i<4; ++i )
            header[ crcAt + i ] = (UInt8) ( entry.crc >> ( 8 * i ) );
        if ( !WriteAt( file, position, &header[ 0 ], (UInt32) header.size() ) )
            return false;
        position = dataStart + entry.size;
    }

    // the central directory
    const UInt64 directoryOffset = position;
    GUCEF::vector< UInt8 > directory;
    for ( size_t e=0; e<entries.size(); ++e )
    {
        const SStoredZipEntry& entry = entries[ e ];
        const bool bigOffset = entry.localOffset >= 0xFFFFFFFFULL;
        PutLE( directory, 0x02014b50, 4 );
        PutLE( directory, 45, 2 );
        PutLE( directory, useZip64 ? 45 : 10, 2 );
        PutLE( directory, 0, 2 );
        PutLE( directory, 0, 2 );
        PutLE( directory, 0, 4 );
        PutLE( directory, entry.crc, 4 );
        PutLE( directory, useZip64 ? 0xFFFFFFFFULL : entry.size, 4 );
        PutLE( directory, useZip64 ? 0xFFFFFFFFULL : entry.size, 4 );
        PutLE( directory, entry.name.Length(), 2 );
        PutLE( directory, useZip64 ? ( bigOffset ? 28 : 20 ) : 0, 2 );
        PutLE( directory, 0, 2 );                    // comment length
        PutLE( directory, 0, 2 );                    // disk number
        PutLE( directory, 0, 2 );                    // internal attributes
        PutLE( directory, 0, 4 );                    // external attributes
        PutLE( directory, bigOffset ? 0xFFFFFFFFULL : entry.localOffset, 4 );
        PutText( directory, entry.name );
        if ( useZip64 )
        {
            PutLE( directory, 1, 2 );
            PutLE( directory, bigOffset ? 24 : 16, 2 );
            PutLE( directory, entry.size, 8 );
            PutLE( directory, entry.size, 8 );
            if ( bigOffset )
                PutLE( directory, entry.localOffset, 8 );
        }
    }
    GUCEF::vector< UInt8 > tail;
    UInt64 recordsAt = directoryOffset + directory.size();
    if ( useZip64 )
    {
        PutLE( tail, 0x06064b50, 4 );
        PutLE( tail, 44, 8 );
        PutLE( tail, 45, 2 );
        PutLE( tail, 45, 2 );
        PutLE( tail, 0, 4 );
        PutLE( tail, 0, 4 );
        PutLE( tail, entries.size(), 8 );
        PutLE( tail, entries.size(), 8 );
        PutLE( tail, directory.size(), 8 );
        PutLE( tail, directoryOffset, 8 );
        PutLE( tail, 0x07064b50, 4 );
        PutLE( tail, 0, 4 );
        PutLE( tail, recordsAt, 8 );
        PutLE( tail, 1, 4 );
    }
    PutLE( tail, 0x06054b50, 4 );
    PutLE( tail, 0, 2 );
    PutLE( tail, 0, 2 );
    PutLE( tail, useZip64 ? 0xFFFF : entries.size(), 2 );
    PutLE( tail, useZip64 ? 0xFFFF : entries.size(), 2 );
    PutLE( tail, useZip64 ? 0xFFFFFFFFULL : directory.size(), 4 );
    PutLE( tail, useZip64 ? 0xFFFFFFFFULL : directoryOffset, 4 );
    PutLE( tail, 0, 2 );
    return WriteAt( file, directoryOffset, &directory[ 0 ], (UInt32) directory.size() ) &&
           WriteAt( file, recordsAt, &tail[ 0 ], (UInt32) tail.size() ) && ( file.Close(), true );
}

/** Reads length bytes at the position of a file of the VFS and checks them against the pattern of the entry */
static bool
CheckVfsRange( VFS::CVFS& vfs, const CORE::CString& path, UInt32 seed, UInt64 position, UInt32 length, double* ms )
{
    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( path, "rb" );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        return false;
    GUCEF::vector< UInt8 > buffer( length );
    const UInt64 start = MT::PrecisionTickCount();
    const bool ok = 0 == resource->GetAccess()->Seek( (Int64) position, SEEK_SET ) && length == resource->GetAccess()->Read( &buffer[ 0 ], 1, length );
    if ( NULL != ms )
        *ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
    return ok && CheckPattern( seed, position, &buffer[ 0 ], length );
}

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

    GUCEF_TESTFW_TESTCASE( "Test: Stored zip64 entries mount through the mhpkg type and read by range with direct seeks" )
        try
        {
            const CORE::CString zipPath = CORE::CombinePath( dir, "stored64.mhpkg" );
            GUCEF::vector< SStoredZipEntry > entries;
            const UInt64 sizes[] = { 300000, 0, 1, 2500000 };
            const char* names[] = { "a/first.bin", "empty.bin", "one.bin", "b/second.bin" };
            for ( UInt32 i=0; i<4; ++i )
            {
                SStoredZipEntry entry;
                entry.name = names[ i ];
                entry.size = sizes[ i ];
                entry.seed = 17 + i;
                entries.push_back( entry );
            }
            // zip64 and plain stored archives are read the same way
            for ( UInt32 variant=0; variant<2; ++variant )
            {
                ASSERT_TRUE( WriteStoredZip64( zipPath, entries, 0 == variant ) );
                ASSERT_TRUE( vfs.MountArchive( "libarchive_stored64", "libarchivetemp/stored64.mhpkg", "mhpkg", "libarchive_stored64", false, false ) );
                for ( UInt32 i=0; i<4; ++i )
                {
                    const CORE::CString path = "libarchive_stored64/" + entries[ i ].name;
                    ASSERT_TRUE( entries[ i ].size == vfs.GetFileSize( path ) );
                    if ( 0 == entries[ i ].size )
                        continue;
                    CORE::CDynamicBuffer all;
                    ASSERT_TRUE( ReadVfsFile( vfs, path, all ) && all.GetDataSize() == entries[ i ].size );
                    ASSERT_TRUE( CheckPattern( entries[ i ].seed, 0, static_cast< const UInt8* >( all.GetConstBufferPtr() ), (UInt32) all.GetDataSize() ) );
                }
                // ranges in the middle, at the end, backwards, across the 64 KiB steps of a stream reader
                const SStoredZipEntry& big = entries[ 3 ];
                const CORE::CString bigPath = "libarchive_stored64/" + big.name;
                ASSERT_TRUE( CheckVfsRange( vfs, bigPath, big.seed, 1234567, 65536, GUCEF_NULL ) );
                ASSERT_TRUE( CheckVfsRange( vfs, bigPath, big.seed, 7, 100, GUCEF_NULL ) );
                ASSERT_TRUE( CheckVfsRange( vfs, bigPath, big.seed, big.size - 1000, 1000, GUCEF_NULL ) );
                ASSERT_TRUE( CheckVfsRange( vfs, bigPath, big.seed, 65535, 3, GUCEF_NULL ) );

                // one handle: forward, back, from the end and from the current position, and reads at and past the end
                VFS::TBasicVfsResourcePtr handle = vfs.GetFile( bigPath, "rb" );
                ASSERT_FALSE( handle.IsNULL() );
                CORE::IOAccessPtr access = handle->GetAccess();
                ASSERT_TRUE( big.size == access->GetSize() );
                UInt8 buffer[ 512 ];
                ASSERT_TRUE( 0 == access->Seek( 2000000, SEEK_SET ) && 512 == access->Read( buffer, 1, 512 ) && CheckPattern( big.seed, 2000000, buffer, 512 ) );
                ASSERT_TRUE( 2000512 == access->Tell() );
                ASSERT_TRUE( 0 == access->Seek( -512, SEEK_CUR ) && 2000000 == access->Tell() );
                ASSERT_TRUE( 0 == access->Seek( 100, SEEK_SET ) && 512 == access->Read( buffer, 1, 512 ) && CheckPattern( big.seed, 100, buffer, 512 ) );
                ASSERT_TRUE( 0 == access->Seek( -100, SEEK_END ) && 100 == access->Read( buffer, 1, 512 ) && access->Eof() );
                ASSERT_TRUE( 0 == access->Read( buffer, 1, 1 ) );
                ASSERT_TRUE( 0 != access->Seek( (Int64) big.size + 1, SEEK_SET ) && 0 != access->Seek( -1, SEEK_SET ) );
                ASSERT_TRUE( 0 == access->Seek( 0, SEEK_SET ) && 0 == access->Tell() );

                // two handles of the same entry at different positions do not disturb each other
                VFS::TBasicVfsResourcePtr other = vfs.GetFile( bigPath, "rb" );
                ASSERT_FALSE( other.IsNULL() );
                UInt8 otherBuffer[ 64 ];
                ASSERT_TRUE( 0 == access->Seek( 1000, SEEK_SET ) && 0 == other->GetAccess()->Seek( 900000, SEEK_SET ) );
                ASSERT_TRUE( 64 == other->GetAccess()->Read( otherBuffer, 1, 64 ) && CheckPattern( big.seed, 900000, otherBuffer, 64 ) );
                ASSERT_TRUE( 64 == access->Read( otherBuffer, 1, 64 ) && CheckPattern( big.seed, 1000, otherBuffer, 64 ) );

                // a cloned access reads the same bytes from its own start
                MT::CICloneable* clone = access->Clone();
                ASSERT_TRUE( GUCEF_NULL != clone );
                CORE::CIOAccess* cloneAccess = static_cast< CORE::CIOAccess* >( clone );
                ASSERT_TRUE( cloneAccess->IsValid() && 0 == cloneAccess->Seek( 3, SEEK_SET ) && 8 == cloneAccess->Read( buffer, 1, 8 ) && CheckPattern( big.seed, 3, buffer, 8 ) );
                GUCEF_DELETE cloneAccess;

                handle.Unlink();
                other.Unlink();
                ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_stored64" ) );
            }
            CORE::DeleteFile( zipPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Truncated and damaged stored zip64 archives do not crash and are not read wrongly" )
        try
        {
            const CORE::CString zipPath = CORE::CombinePath( dir, "damaged.mhpkg" );
            GUCEF::vector< SStoredZipEntry > entries;
            SStoredZipEntry entry;
            entry.name = "x.bin";
            entry.size = 100000;
            entry.seed = 3;
            entries.push_back( entry );
            ASSERT_TRUE( WriteStoredZip64( zipPath, entries, true ) );
            CORE::CDynamicBuffer original;
            ASSERT_TRUE( original.LoadContentFromFile( zipPath ) );

            // cut at many places and flip bytes of the directory area: mounting may fail but nothing may crash or hang,
            // and when an archive still mounts, the entry reads either as an error or with the right content
            const UInt32 total = original.GetDataSize();
            UInt32 mounted = 0;
            for ( UInt32 variant=0; variant<60; ++variant )
            {
                CORE::CDynamicBuffer copy;
                copy.CopyFrom( original.GetDataSize(), original.GetConstBufferPtr() );
                UInt32 length = total;
                if ( variant < 30 )
                    length = ( total * variant ) / 30;
                else
                {
                    const UInt32 at = total - 1 - ( ( variant - 30 ) * 3 ) % 200;
                    static_cast< UInt8* >( copy.GetBufferPtr() )[ at ] ^= 0xFF;
                }
                CORE::CFileAccess output( zipPath, "wb" );
                ASSERT_TRUE( output.Opened() );
                output.Write( copy.GetConstBufferPtr(), 1, length );
                output.Close();
                if ( vfs.MountArchive( "libarchive_damaged", "libarchivetemp/damaged.mhpkg", "mhpkg", "libarchive_damaged", false, false ) )
                {
                    ++mounted;
                    {
                        VFS::TBasicVfsResourcePtr handle = vfs.GetFile( "libarchive_damaged/x.bin", "rb" );
                        if ( !handle.IsNULL() && !handle->GetAccess().IsNULL() )
                        {
                            UInt8 buffer[ 4096 ];
                            const UInt32 got = handle->GetAccess()->Read( buffer, 1, 4096 );
                            if ( 4096 == got )
                                ASSERT_TRUE( CheckPattern( 3, 0, buffer, 4096 ) );
                        }
                    }
                    ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_damaged" ) );
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Damaged stored archives: " << mounted << " of 60 variants still mounted" );
            CORE::DeleteFile( zipPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( TestArchiveUtils::LargeFileTestsEnabled() )
    {
        GUCEF_TESTFW_TESTCASE( "Test: A stored zip64 entry of 5 GiB at offsets beyond 4 GiB reads ranges without reading what is in front of them" )
            try
            {
                const CORE::CString zipPath = CORE::CombinePath( dir, "huge64.mhpkg" );
                GUCEF::vector< SStoredZipEntry > entries;
                SStoredZipEntry first;
                first.name = "huge/first.bin";
                first.size = 5ULL * 1024 * 1024 * 1024;
                first.seed = 5;
                entries.push_back( first );
                SStoredZipEntry second;
                second.name = "huge/second.bin";
                second.size = 3000000;
                second.seed = 9;
                entries.push_back( second );
                ASSERT_TRUE( WriteStoredZip64( zipPath, entries, true ) );

                ASSERT_TRUE( vfs.MountArchive( "libarchive_huge64", "libarchivetemp/huge64.mhpkg", "mhpkg", "libarchive_huge64", false, false ) );
                ASSERT_TRUE( first.size == vfs.GetFileSize( "libarchive_huge64/huge/first.bin" ) );
                ASSERT_TRUE( second.size == vfs.GetFileSize( "libarchive_huge64/huge/second.bin" ) );
                double ms = 0;
                // the first and the last megabyte have the pattern, the hole in between reads as zero
                ASSERT_TRUE( CheckVfsRange( vfs, "libarchive_huge64/huge/first.bin", first.seed, 100, 65536, &ms ) );
                ASSERT_TRUE( CheckVfsRange( vfs, "libarchive_huge64/huge/first.bin", first.seed, first.size - 70000, 65536, &ms ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "64 KiB at 5 GiB minus 70000 of a stored entry: " << ms << " ms" );
                ASSERT_TRUE( ms < 500.0 );
                // the second entry sits behind the 4 GiB mark
                ASSERT_TRUE( CheckVfsRange( vfs, "libarchive_huge64/huge/second.bin", second.seed, 1500000, 65536, &ms ) );
                ASSERT_TRUE( ms < 500.0 );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_huge64" ) );
                CORE::DeleteFile( zipPath );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "Corpus: a MetaHuman package (.mhpkg, zip64, stored) mounts and reads ranges of its entries fast" )
        const char* corpus = getenv( "GUCEF_TEST_ASSET_CORPUS" );
        const char* home = getenv( "HOME" );
        if ( GUCEF_NULL == corpus || 0 != strcmp( corpus, "1" ) || GUCEF_NULL == home )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
        const CORE::CString packagePath = CORE::CombinePath( home, "Downloads/Fab.com/Metahuman/scarletjohansson.mhpkg" );
        if ( !CORE::FileExists( packagePath ) )
            GUCEF_TESTFW_SKIP( "The corpus package scarletjohansson.mhpkg is not present" );
        try
        {
            const UInt64 mountStart = MT::PrecisionTickCount();
            ASSERT_TRUE( vfs.MountArchive( "libarchive_mhpkg", packagePath, "mhpkg", "libarchive_mhpkg", false, false, packagePath ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Mounting the package took " << MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - mountStart ) << " ms" );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, "libarchive_mhpkg", true, true, CORE::CString::EmptyStringSet, 100000 );
            ASSERT_TRUE( files.size() > 10 );

            // the largest entry and a range from its middle
            CORE::CString largest;
            UInt64 largestSize = 0;
            for ( size_t i=0; i<files.size(); ++i )
            {
                const UInt64 size = vfs.GetFileSize( files[ i ] );
                if ( size > largestSize ) { largestSize = size; largest = files[ i ]; }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Largest entry " << largest << ": " << largestSize << " bytes" );
            ASSERT_TRUE( largestSize > 100ULL * 1024 * 1024 );
            VFS::TBasicVfsResourcePtr handle = vfs.GetFile( largest, "rb" );
            ASSERT_FALSE( handle.IsNULL() );
            CORE::IOAccessPtr access = handle->GetAccess();
            UInt8 head[ 4 ];
            ASSERT_TRUE( 4 == access->Read( head, 1, 4 ) && 0xC1 == head[ 0 ] && 0x83 == head[ 1 ] && 0x2A == head[ 2 ] && 0x9E == head[ 3 ] ); // package file tag

            GUCEF::vector< UInt8 > middle( 65536 );
            GUCEF::vector< UInt8 > again( 65536 );
            const UInt64 positions[] = { largestSize / 2, largestSize - 65536, 4096 };
            for ( UInt32 p=0; p<3; ++p )
            {
                const UInt64 start = MT::PrecisionTickCount();
                ASSERT_TRUE( 0 == access->Seek( (Int64) positions[ p ], SEEK_SET ) && 65536 == access->Read( &middle[ 0 ], 1, 65536 ) );
                const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "64 KiB at " << positions[ p ] << " of the largest entry: " << ms << " ms" );
                ASSERT_TRUE( ms < 500.0 );
                // a second handle reads the same bytes, and by a read in small steps from a seek to the same place
                VFS::TBasicVfsResourcePtr other = vfs.GetFile( largest, "rb" );
                ASSERT_FALSE( other.IsNULL() );
                ASSERT_TRUE( 0 == other->GetAccess()->Seek( (Int64) positions[ p ], SEEK_SET ) );
                for ( UInt32 step=0; step<16; ++step )
                    ASSERT_TRUE( 4096 == other->GetAccess()->Read( &again[ step * 4096 ], 1, 4096 ) );
                ASSERT_TRUE( 0 == memcmp( &middle[ 0 ], &again[ 0 ], 65536 ) );
            }
            handle.Unlink();
            ASSERT_TRUE( vfs.UnmountArchiveByName( "libarchive_mhpkg" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    vfs.UnmountArchiveByName( "LibArchiveTempDir" );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL libarchive plugin TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
