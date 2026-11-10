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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_ISOIMAGEWRITER_H
#include "gucef_test_isoimagewriter.h"
#define GUCEF_TEST_ISOIMAGEWRITER_H
#endif /* GUCEF_TEST_ISOIMAGEWRITER_H ? */

#include "TestArchiveUtils.h"
#include "TestCueBinArchive.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace TestArchiveUtils;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

using namespace GUCEF::TESTFW::ISO;

const UInt32 CookedSectorSize = 2048;
const UInt32 RawSectorSize = 2352;

/** Recognisable bytes, different per seed */
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

/** A small file system for the data track */
CORE::CDynamicBuffer
MakeDataTrack( const CORE::CDynamicBuffer& gameFile )
{
    SNode root = Dir( "", "" );
    root.children.push_back( File( "README.TXT", "README.TXT;1", "on the data track" ) );
    SNode data = Dir( "DATA", "DATA" );
    data.children.push_back( File( "GAME.DAT", "GAME.DAT;1", AsFileContent( gameFile ) ) );
    root.children.push_back( data );

    SOptions options;
    CImageWriter writer( root, options, "CUETEST" );
    const TBytes image = writer.Write();
    CORE::CDynamicBuffer result;
    result.Append( &image[ 0 ], (UInt32) image.size() );
    return result;
}

/** Cooked 2048 byte sectors as raw MODE1 2352 byte sectors: sync, header with the address and mode, user data, ECC (zero) */
CORE::CDynamicBuffer
MakeRawMode1( const CORE::CDynamicBuffer& cooked )
{
    CORE::CDynamicBuffer raw;
    const UInt32 sectors = ( cooked.GetDataSize() + CookedSectorSize - 1 ) / CookedSectorSize;
    UInt8 sector[ RawSectorSize ];
    for ( UInt32 s=0; s<sectors; ++s )
    {
        memset( sector, 0, sizeof( sector ) );
        memset( sector + 1, 0xFF, 10 );
        const UInt32 lba = s + 150;
        sector[ 12 ] = (UInt8) ( ( ( lba / 4500 ) / 10 ) << 4 | ( ( lba / 4500 ) % 10 ) );
        sector[ 13 ] = (UInt8) ( ( ( ( lba / 75 ) % 60 ) / 10 ) << 4 | ( ( lba / 75 ) % 60 ) % 10 );
        sector[ 14 ] = (UInt8) ( ( ( lba % 75 ) / 10 ) << 4 | ( lba % 75 ) % 10 );
        sector[ 15 ] = 1;
        const UInt32 offset = s * CookedSectorSize;
        const UInt32 count = cooked.GetDataSize() - offset < CookedSectorSize ? cooked.GetDataSize() - offset : CookedSectorSize;
        memcpy( sector + 16, cooked.GetConstBufferPtr( offset ), count );
        raw.Append( sector, RawSectorSize );
    }
    return raw;
}

/** Whole sectors of cooked data, padded with zeros */
CORE::CDynamicBuffer
PadToSectors( const CORE::CDynamicBuffer& data, UInt32 sectorSize )
{
    CORE::CDynamicBuffer padded( data );
    const UInt32 rest = data.GetDataSize() % sectorSize;
    if ( 0 != rest )
    {
        CORE::CDynamicBuffer zeros( sectorSize - rest, true );
        zeros.SetDataSize( sectorSize - rest );
        memset( zeros.GetBufferPtr(), 0, sectorSize - rest );
        padded.Append( zeros.GetConstBufferPtr(), sectorSize - rest );
    }
    return padded;
}

CORE::CString
Msf( UInt32 sectors )
{
    char text[ 16 ];
    snprintf( text, sizeof( text ), "%02u:%02u:%02u", sectors / 4500, ( sectors / 75 ) % 60, sectors % 75 );
    return text;
}

bool
WriteFile( const CORE::CString& path, const CORE::CDynamicBuffer& data )
{
    CORE::CFileAccess out( path, "wb" );
    return out.Opened() && data.GetDataSize() == out.Write( data.GetConstBufferPtr(), 1, data.GetDataSize() );
}

bool
WriteText( const CORE::CString& path, const CORE::CString& text )
{
    CORE::CDynamicBuffer data;
    data.Append( text.C_String(), text.ByteSize() - 1 );
    return WriteFile( path, data );
}

bool
SameContent( VFS::CVFS& vfs, const CORE::CString& vfsPath, const CORE::CDynamicBuffer& expected, UInt32 skip = 0 )
{
    CORE::CDynamicBuffer buffer;
    if ( !vfs.LoadFile( buffer, vfsPath, "rb" ) || buffer.GetDataSize() != expected.GetDataSize() + skip )
        return false;
    return 0 == expected.GetDataSize() || 0 == memcmp( buffer.GetConstBufferPtr( skip ), expected.GetConstBufferPtr(), expected.GetDataSize() );
}

bool
HasName( const VFS::CVFS::TStringVector& list, const char* name )
{
    for ( size_t i = 0; i < list.size(); ++i )
        if ( list[ i ].Equals( name, false ) )
            return true;
    return false;
}

UInt32
LE32( const CORE::CDynamicBuffer& data, UInt32 offset )
{
    const UInt8* b = static_cast< const UInt8* >( data.GetConstBufferPtr( offset ) );
    return (UInt32) b[ 0 ] | ( (UInt32) b[ 1 ] << 8 ) | ( (UInt32) b[ 2 ] << 16 ) | ( (UInt32) b[ 3 ] << 24 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCueBinArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CUE/BIN plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CueBinArchive" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const bool isoPluginLoaded = LoadVfsPlugin( "vfspluginISO9660" );
    const bool pluginLoaded = LoadVfsPlugin( "vfspluginCUEBIN" );

    // the image: a raw MODE1 data track, an audio track, and an audio track with its pregap in the file
    const CORE::CDynamicBuffer gameFile = MakeContent( 9000, 3 );
    const CORE::CDynamicBuffer dataTrack = PadToSectors( MakeDataTrack( gameFile ), CookedSectorSize );
    const UInt32 dataSectors = dataTrack.GetDataSize() / CookedSectorSize;
    const CORE::CDynamicBuffer audio2 = MakeContent( 5 * RawSectorSize, 20 );
    CORE::CDynamicBuffer pregap3Bytes( 2 * RawSectorSize, true );
    pregap3Bytes.SetDataSize( 2 * RawSectorSize );
    memset( pregap3Bytes.GetBufferPtr(), 0, 2 * RawSectorSize );
    const CORE::CDynamicBuffer audio3 = MakeContent( 3 * RawSectorSize, 30 );

    const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_CueBin" );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    {
        CORE::CDynamicBuffer image = MakeRawMode1( dataTrack );
        image.Append( audio2.GetConstBufferPtr(), audio2.GetDataSize() );
        image.Append( pregap3Bytes.GetConstBufferPtr(), pregap3Bytes.GetDataSize() );
        image.Append( audio3.GetConstBufferPtr(), audio3.GetDataSize() );
        WriteFile( CORE::CombinePath( dir, "TestDisc.bin" ), image );
    }
    // the sheet names the image in other letter case, as sheets made on case insensitive systems do
    const CORE::CString sheet =
        "REM made by the test\r\n"
        "FILE \"TESTDISC.BIN\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    PREGAP 00:02:00\r\n"
        "    INDEX 01 " + Msf( dataSectors ) + "\r\n"
        "  TRACK 03 AUDIO\r\n"
        "    INDEX 00 " + Msf( dataSectors + 5 ) + "\r\n"
        "    INDEX 01 " + Msf( dataSectors + 7 ) + "\r\n";
    const CORE::CString sheetPath = CORE::CombinePath( dir, "TestDisc.cue" );
    WriteText( sheetPath, sheet );

    GUCEF_TESTFW_TESTCASE( "CueBin: The plugin provides the cue archive type" )
        try
        {
            ASSERT_TRUE( isoPluginLoaded && pluginLoaded );
            VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
            vfs.GetListOfSupportedArchiveTypes( types );
            ASSERT_TRUE( types.find( "cue" ) != types.end() );
            ASSERT_TRUE( types.find( "bin" ) == types.end() );           // .bin is used for all kinds of dumps
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( !isoPluginLoaded || !pluginLoaded )
        return;

    GUCEF_TESTFW_TESTCASE( "CueBin: The data track of raw sectors is read through the ISO 9660 file system" )
        try
        {
            ASSERT_TRUE( vfs.MountArchive( "cueDisc", sheetPath, "cue", "/cuedisc", false, false, sheetPath ) );
            ASSERT_TRUE( vfs.FileExists( "/cuedisc/README.TXT" ) );
            ASSERT_TRUE( vfs.FileExists( "/cuedisc/data/game.dat" ) );          // lookups ignore case
            ASSERT_TRUE( vfs.DirExists( "/cuedisc/DATA" ) );
            ASSERT_TRUE( SameContent( vfs, "/cuedisc/DATA/GAME.DAT", gameFile ) );

            VFS::CVFS::TStringVector dirs;
            vfs.GetDirList( dirs, CORE::CString( "/cuedisc" ), false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( HasName( dirs, "DATA/" ) || HasName( dirs, "DATA" ) );
            ASSERT_TRUE( HasName( dirs, "CDDA/" ) || HasName( dirs, "CDDA" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CueBin: Audio tracks are WAVE files of the PCM from INDEX 01 to the next track, the pregap in the file left out" )
        try
        {
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, CORE::CString( "/cuedisc/CDDA" ), false, false, CORE::CString::EmptyStringSet );
            ASSERT_TRUE( 2 == files.size() && HasName( files, "TRACK02.WAV" ) && HasName( files, "TRACK03.WAV" ) );

            // 44 byte header, then exactly the audio of the track
            ASSERT_TRUE( SameContent( vfs, "/cuedisc/CDDA/TRACK02.WAV", audio2, 44 ) );
            ASSERT_TRUE( SameContent( vfs, "/cuedisc/cdda/track03.wav", audio3, 44 ) );
            ASSERT_TRUE( 44 + audio3.GetDataSize() == vfs.GetFileSize( "/cuedisc/CDDA/TRACK03.WAV" ) );

            CORE::CDynamicBuffer wave;
            ASSERT_TRUE( vfs.LoadFile( wave, "/cuedisc/CDDA/TRACK02.WAV", "rb" ) );
            ASSERT_TRUE( 0 == memcmp( wave.GetConstBufferPtr(), "RIFF", 4 ) && 0 == memcmp( wave.GetConstBufferPtr( 8 ), "WAVEfmt ", 8 ) );
            ASSERT_TRUE( 44100 == LE32( wave, 24 ) && 44100 * 4 == LE32( wave, 28 ) );
            ASSERT_TRUE( audio2.GetDataSize() == LE32( wave, 40 ) );

            // read in pieces that cross sector boundaries, as a player streams it
            VFS::TBasicVfsResourcePtr track = vfs.GetFile( "/cuedisc/CDDA/TRACK03.WAV", "rb" );
            ASSERT_FALSE( track.IsNULL() );
            if ( !track.IsNULL() )
            {
                CORE::IOAccessPtr access = track->GetAccess();
                ASSERT_TRUE( 0 == access->Setpos( 44 + RawSectorSize - 10 ) );
                UInt8 piece[ 20 ];
                ASSERT_TRUE( 20 == access->Read( piece, 1, 20 ) );
                ASSERT_TRUE( 0 == memcmp( piece, audio3.GetConstBufferPtr( RawSectorSize - 10 ), 20 ) );
                ASSERT_TRUE( 0 == access->Seek( 0, SEEK_END ) && access->Eof() );
                ASSERT_TRUE( 0 == access->Read( piece, 1, 1 ) );
                track.Unlink();
            }
            ASSERT_TRUE( vfs.UnmountArchiveByName( "cueDisc" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CueBin: An image the sheet names that is not there is found by the name of the sheet" )
        try
        {
            const CORE::CString renamed =
                "FILE \"Original Name.img\" BINARY\r\n"
                "  TRACK 01 MODE1/2352\r\n"
                "    INDEX 01 00:00:00\r\n";
            const CORE::CString renamedSheetPath = CORE::CombinePath( dir, "TestDisc_renamed.cue" );
            WriteText( renamedSheetPath, renamed );
            CORE::CDynamicBuffer image;
            ASSERT_TRUE( image.LoadContentFromFile( CORE::CombinePath( dir, "TestDisc.bin" ) ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "TESTDISC_RENAMED.BIN" ), image ) );
            ASSERT_TRUE( vfs.MountArchive( "cueRenamed", renamedSheetPath, "cue", "/cuerenamed", false, false, renamedSheetPath ) );
            ASSERT_TRUE( SameContent( vfs, "/cuerenamed/DATA/GAME.DAT", gameFile ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "cueRenamed" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CueBin: Cooked data sectors and raw audio sectors in one file are each found at their own offset" )
        try
        {
            CORE::CDynamicBuffer image( dataTrack );
            image.Append( audio2.GetConstBufferPtr(), audio2.GetDataSize() );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "mixed.bin" ), image ) );
            const CORE::CString mixed =
                "FILE \"mixed.bin\" BINARY\n"
                "  TRACK 01 MODE1/2048\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    INDEX 01 " + Msf( dataSectors ) + "\n";
            const CORE::CString mixedSheetPath = CORE::CombinePath( dir, "mixed.cue" );
            WriteText( mixedSheetPath, mixed );
            ASSERT_TRUE( vfs.MountArchive( "cueMixed", mixedSheetPath, "cue", "/cuemixed", false, false, mixedSheetPath ) );
            ASSERT_TRUE( SameContent( vfs, "/cuemixed/DATA/GAME.DAT", gameFile ) );
            ASSERT_TRUE( SameContent( vfs, "/cuemixed/CDDA/TRACK02.WAV", audio2, 44 ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "cueMixed" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CueBin: A .bin itself, a sheet without its image and text that is no sheet are refused" )
        try
        {
            const CORE::CString binPath = CORE::CombinePath( dir, "TestDisc.bin" );
            ASSERT_FALSE( vfs.MountArchive( "cueFromBin", binPath, "cue", "/cuefrombin", false, false, binPath ) );

            const CORE::CString orphanPath = CORE::CombinePath( dir, "orphan.cue" );
            WriteText( orphanPath, "FILE \"gone.bin\" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n" );
            ASSERT_FALSE( vfs.MountArchive( "cueOrphan", orphanPath, "cue", "/cueorphan", false, false, orphanPath ) );

            const CORE::CString textPath = CORE::CombinePath( dir, "notes.cue" );
            WriteText( textPath, "these are just some notes\nnothing more\n" );
            ASSERT_FALSE( vfs.MountArchive( "cueText", textPath, "cue", "/cuetext", false, false, textPath ) );

            ASSERT_FALSE( vfs.MountArchive( "cueWrite", sheetPath, "cue", "/cuewrite", true, false, sheetPath ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // A real disc image, for those who have one: set GUCEF_TEST_CUE_IMAGE to the path of its .cue
    GUCEF_TESTFW_TESTCASE( "CueBin: A real disc image mounts with its file system and audio tracks" )
        GUCEF_TESTFW_REQUIRE_ENV( realSheet, "GUCEF_TEST_CUE_IMAGE" );
        try
        {
            const CORE::CString realSheetPath( realSheet.c_str() );
            ASSERT_TRUE( vfs.MountArchive( "cueReal", realSheetPath, "cue", "/cuereal", false, false, realSheetPath ) );
            VFS::CVFS::TStringVector files, tracks;
            vfs.GetFileList( files, CORE::CString( "/cuereal" ), true, false, CORE::CString::EmptyStringSet );
            vfs.GetFileList( tracks, CORE::CString( "/cuereal/CDDA" ), false, false, CORE::CString::EmptyStringSet );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Real disc image: " << files.size() << " files, " << tracks.size() << " audio tracks" );
            ASSERT_TRUE( files.size() > tracks.size() );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "cueReal" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::DeleteDir( dir, true );
}

/*-------------------------------------------------------------------------*/
