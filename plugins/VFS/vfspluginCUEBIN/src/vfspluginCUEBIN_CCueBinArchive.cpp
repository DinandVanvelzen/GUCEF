/*
 *  vfspluginCUEBIN: Generic GUCEF VFS plugin for CD images described by a CUE sheet (BIN/CUE)
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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#include "vfspluginCUEBIN_CCueBinArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace CUEBIN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const VFS::CString CCueBinArchive::CueArchiveTypeName = "cue";
const VFS::CString CCueBinArchive::AudioTrackDirName = "CDDA";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
PutLE32( UInt8* out, UInt32 value )
{
    out[ 0 ] = (UInt8) value;
    out[ 1 ] = (UInt8) ( value >> 8 );
    out[ 2 ] = (UInt8) ( value >> 16 );
    out[ 3 ] = (UInt8) ( value >> 24 );
}

/*-------------------------------------------------------------------------*/

void
PutLE16( UInt8* out, UInt16 value )
{
    out[ 0 ] = (UInt8) value;
    out[ 1 ] = (UInt8) ( value >> 8 );
}

/*-------------------------------------------------------------------------*/

/** The header of a WAVE file with the given number of bytes of CD audio (44.1 kHz, 16 bit, stereo, little endian) */
void
MakeCdAudioWaveHeader( UInt64 pcmBytes, CORE::CDynamicBuffer& header )
{
    const UInt32 dataSize = pcmBytes > 0xFFFFFFFFULL - 36 ? 0xFFFFFFFFUL - 36 : (UInt32) pcmBytes;
    UInt8 bytes[ 44 ];
    memcpy( bytes, "RIFF", 4 );
    PutLE32( bytes + 4, 36 + dataSize );
    memcpy( bytes + 8, "WAVEfmt ", 8 );
    PutLE32( bytes + 16, 16 );                  // size of the format chunk
    PutLE16( bytes + 20, 1 );                   // PCM
    PutLE16( bytes + 22, 2 );                   // channels
    PutLE32( bytes + 24, 44100 );               // frames per second
    PutLE32( bytes + 28, 44100 * 4 );           // bytes per second
    PutLE16( bytes + 32, 4 );                   // bytes per frame
    PutLE16( bytes + 34, 16 );                  // bits per sample
    memcpy( bytes + 36, "data", 4 );
    PutLE32( bytes + 40, dataSize );
    header.Clear();
    header.Append( bytes, 44 );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCueBinArchive::CCueBinArchive( void )
    : VFS::CArchive()
    , m_archiveName()
    , m_archivePath()
    , m_sheet()
    , m_audioTracks()
    , m_dataArchive()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCueBinArchive::~CCueBinArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CCueBinArchive::NormalizePath( const VFS::CString& path )
{GUCEF_TRACE;

    VFS::CString result = path.ReplaceChar( '\\', '/' );
    result = result.CompactRepeatingChar( '/' );
    while ( result.Length() > 0 && result[ 0 ] == '/' )
        result = result.CutChars( 1, true, 0 );
    while ( result.Length() > 0 && result[ result.Length()-1 ] == '/' )
        result = result.CutChars( 1, false, 0 );
    return result.Lowercase();
}

/*-------------------------------------------------------------------------*/

const CCueBinArchive::SAudioTrack*
CCueBinArchive::FindAudioTrack( const VFS::CString& path ) const
{GUCEF_TRACE;

    const VFS::CString normalized = NormalizePath( path );
    const VFS::CString dirPrefix = AudioTrackDirName.Lowercase() + '/';
    if ( normalized.Length() <= dirPrefix.Length() || !normalized.StartsWith( dirPrefix ) )
        return GUCEF_NULL;

    TAudioTrackMap::const_iterator i = m_audioTracks.find( normalized.CutChars( dirPrefix.Length(), true, 0 ) );
    if ( i == m_audioTracks.end() )
        return GUCEF_NULL;
    return &(*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::IsDataPath( const VFS::CString& normalizedPath ) const
{GUCEF_TRACE;

    if ( m_dataArchive.IsNULL() )
        return false;
    if ( m_audioTracks.empty() )
        return true;

    // everything outside the audio directory belongs to the data track
    const VFS::CString audioDir = AudioTrackDirName.Lowercase();
    if ( normalizedPath == audioDir )
        return false;
    return !normalizedPath.StartsWith( audioDir + '/' );
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CCueBinArchive::GetFile( const VFS::CString& file      ,
                         const char* mode              ,
                         const VFS::UInt32 memLoadSize ,
                         const bool overwrite          )
{GUCEF_TRACE;

    // read only
    if ( GUCEF_NULL == mode || *mode != 'r' )
        return VFS::TBasicVfsResourcePtr();

    const SAudioTrack* track = FindAudioTrack( file );
    if ( GUCEF_NULL != track )
    {
        CORE::IOAccessPtr access( GUCEF_NEW CCdTrackIOAccess( track->imagePath, track->layout, track->header ) );
        if ( !access->Opened() )
            return VFS::TBasicVfsResourcePtr();
        CORE::CString filePath = m_archivePath;
        CORE::AppendToPath( filePath, file );
        return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, track->name, filePath ), this );
    }

    if ( IsDataPath( NormalizePath( file ) ) )
        return m_dataArchive->GetFile( file, mode, memLoadSize, overwrite );
    return VFS::TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::StoreAsFile( const CORE::CString& filepath    ,
                             const CORE::CDynamicBuffer& data ,
                             const CORE::UInt64 offset        ,
                             const bool overwrite             )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CCueBinArchive::ListAudioTracks( TStringVector& outputList                   ,
                                 const VFS::CString& mountLocation           ,
                                 bool includePath                            ,
                                 const VFS::CString::StringSet& nameFilters  ,
                                 VFS::UInt32 maxListEntries                  ) const
{GUCEF_TRACE;

    for ( TAudioTrackMap::const_iterator i = m_audioTracks.begin(); i != m_audioTracks.end(); ++i )
    {
        if ( outputList.size() >= maxListEntries )
            return;
        const SAudioTrack& track = (*i).second;
        if ( !VFS::CVFS::FilterValidation( track.name, nameFilters ) )
            continue;
        if ( includePath )
            outputList.push_back( CORE::CombinePath( mountLocation, AudioTrackDirName + '/' + track.name ).ReplaceChar( '\\', '/' ) );
        else
            outputList.push_back( track.name );
    }
}

/*-------------------------------------------------------------------------*/

void
CCueBinArchive::GetFileList( TStringVector& outputList                   ,
                             const VFS::CString& mountLocation           ,
                             const VFS::CString& archiveLocation         ,
                             bool recursive                              ,
                             bool includePathInFilename                  ,
                             const VFS::CString::StringSet& nameFilters  ,
                             VFS::UInt32 maxListEntries                  ) const
{GUCEF_TRACE;

    const VFS::CString location = NormalizePath( archiveLocation );
    if ( IsDataPath( location ) )
        m_dataArchive->GetFileList( outputList, mountLocation, archiveLocation, recursive, includePathInFilename, nameFilters, maxListEntries );

    // the audio tracks are in their own directory: listed when that is listed, or the root recursively
    if ( location == AudioTrackDirName.Lowercase() || ( location.IsNULLOrEmpty() && recursive ) )
        ListAudioTracks( outputList, mountLocation, includePathInFilename, nameFilters, maxListEntries );
}

/*-------------------------------------------------------------------------*/

void
CCueBinArchive::GetDirList( TStringVector& outputList                   ,
                            const VFS::CString& mountLocation           ,
                            const VFS::CString& archiveLocation         ,
                            bool recursive                              ,
                            bool includeParentDirInName                 ,
                            const VFS::CString::StringSet& nameFilters  ,
                            VFS::UInt32 maxListEntries                  ) const
{GUCEF_TRACE;

    const VFS::CString location = NormalizePath( archiveLocation );
    if ( IsDataPath( location ) )
        m_dataArchive->GetDirList( outputList, mountLocation, archiveLocation, recursive, includeParentDirInName, nameFilters, maxListEntries );

    if ( location.IsNULLOrEmpty() && !m_audioTracks.empty() && outputList.size() < maxListEntries &&
         VFS::CVFS::FilterValidation( AudioTrackDirName, nameFilters ) )
    {
        VFS::CString listed = AudioTrackDirName;
        if ( includeParentDirInName )
            listed = CORE::CombinePath( mountLocation, AudioTrackDirName ).ReplaceChar( '\\', '/' );
        outputList.push_back( listed + '/' );
    }
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::MoveFile( const VFS::CString& oldFilePath ,
                          const VFS::CString& newFilePath ,
                          const bool overwrite            )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::GetFileMetaData( const VFS::CString& filePath      ,
                                 CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    const SAudioTrack* track = FindAudioTrack( filePath );
    if ( GUCEF_NULL != track )
    {
        metaData.Clear();
        metaData.resourceExists = true;
        metaData.name = track->name;
        metaData.hasName = true;
        metaData.isReadOnly = true;
        metaData.hasIsReadOnly = true;
        metaData.resourceSizeInBytes = track->size;
        metaData.hasResourceSizeInBytes = true;
        return true;
    }
    if ( IsDataPath( NormalizePath( filePath ) ) )
        return m_dataArchive->GetFileMetaData( filePath, metaData );
    metaData.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != FindAudioTrack( filePath ) )
        return true;
    return IsDataPath( NormalizePath( filePath ) ) && m_dataArchive->FileExists( filePath );
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    const VFS::CString normalized = NormalizePath( dirPath );
    if ( normalized.IsNULLOrEmpty() )
        return true;
    if ( !m_audioTracks.empty() && normalized == AudioTrackDirName.Lowercase() )
        return true;
    return IsDataPath( normalized ) && m_dataArchive->DirExists( dirPath );
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CCueBinArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const SAudioTrack* track = FindAudioTrack( filePath );
    if ( GUCEF_NULL != track )
        return track->size;
    if ( IsDataPath( NormalizePath( filePath ) ) )
        return m_dataArchive->GetFileSize( filePath );
    return 0;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CCueBinArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    const SAudioTrack* track = FindAudioTrack( file );
    if ( GUCEF_NULL != track )
    {
        CCdTrackIOAccess access( track->imagePath, track->layout, track->header );
        VFS::UInt8 digest[ 16 ];
        if ( access.Opened() && 0 != CORE::md5fromfile( access.CStyleAccess(), digest ) )
        {
            char md5Str[ 48 ];
            CORE::md5tostring( digest, md5Str );
            return VFS::CString( md5Str );
        }
        return VFS::CString::Empty;
    }
    if ( IsDataPath( NormalizePath( file ) ) )
        return m_dataArchive->GetFileHash( file );
    return VFS::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CCueBinArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == FindAudioTrack( filePath ) && IsDataPath( NormalizePath( filePath ) ) )
        return m_dataArchive->GetFileModificationTime( filePath );
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CCueBinArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CCueBinArchive::ResolveImageFile( const VFS::CString& sheetPath, const VFS::CString& fileName ) const
{GUCEF_TRACE;

    const VFS::CString sheetDir = CORE::StripFilename( sheetPath );

    // as written, absolute or next to the sheet
    VFS::CString path = fileName;
    if ( !CORE::FileExists( path ) )
        path = CORE::CombinePath( sheetDir, fileName );
    if ( CORE::FileExists( path ) )
        return path;

    // Next to the sheet ignoring case (sheets made on case insensitive systems), else a file with the name of the
    // sheet and the extension .bin (images are often renamed after the sheet was made)
    const VFS::CString wanted = CORE::ExtractFilename( fileName ).Lowercase();
    const VFS::CString sheetNameBin = ( CORE::StripFileExtention( CORE::ExtractFilename( sheetPath ) ) + ".bin" ).Lowercase();
    VFS::CString sameNameAsSheet;
    CORE::CFileSystemIterator it;
    if ( it.FindFirst( sheetDir ) )
    {
        do
        {
            if ( !it.IsAFile() )
                continue;
            const VFS::CString name = it.GetResourceName();
            if ( name.Lowercase() == wanted )
            {
                it.FindClose();
                return CORE::CombinePath( sheetDir, name );
            }
            if ( name.Lowercase() == sheetNameBin )
                sameNameAsSheet = CORE::CombinePath( sheetDir, name );
        }
        while ( it.FindNext() );
    }
    it.FindClose();

    if ( !sameNameAsSheet.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CueBinArchive: \"" << fileName << "\" named by the sheet is not there, using \"" << sameNameAsSheet << "\" which has the name of the sheet" );
        return sameNameAsSheet;
    }
    return VFS::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::Load( const VFS::CString& archiveName ,
                      const VFS::CString& archivePath ,
                      const VFS::CString& sheetText   ,
                      const VFS::CString& sheetPath   )
{GUCEF_TRACE;

    UnloadArchive();

    VFS::CString error;
    if ( !m_sheet.Parse( sheetText, error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CueBinArchive: \"" << sheetPath << "\" is not a usable CUE sheet: " << error );
        return false;
    }

    // the image files
    const CCueSheet::TFileVector& files = m_sheet.GetFiles();
    GUCEF::vector< VFS::CString > imagePaths;
    GUCEF::vector< VFS::UInt64 > imageSizes;
    for ( size_t f=0; f<files.size(); ++f )
    {
        const VFS::CString imagePath = ResolveImageFile( sheetPath, files[ f ].name );
        if ( imagePath.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CueBinArchive: The image file \"" << files[ f ].name << "\" of \"" << sheetPath << "\" is not there" );
            m_sheet = CCueSheet();
            return false;
        }
        imagePaths.push_back( imagePath );
        imageSizes.push_back( CORE::FileSize( imagePath ) );
    }

    // Where each track is in its file. Positions in the sheet count sectors; a file can hold tracks with sectors of
    // different sizes, so the byte offset of a track is the sum of what the tracks before it in the file take
    const CCueSheet::TTrackVector& tracks = m_sheet.GetTracks();
    VFS::UInt64 fileByteOffset = 0;
    for ( size_t t=0; t<tracks.size(); ++t )
    {
        const CCueSheet::STrack& track = tracks[ t ];
        const UInt32 sectorSize = CCueSheet::GetSectorSize( track.mode );
        const VFS::UInt64 regionStart = track.index0 >= 0 ? (VFS::UInt64) track.index0 : track.index1;
        if ( 0 == t || tracks[ t-1 ].fileIndex != track.fileIndex )
            fileByteOffset = 0;

        // the byte offset where this track's region starts was accumulated by the track before it in the same file
        const VFS::UInt64 trackStartByte = fileByteOffset + ( track.index1 - regionStart ) * sectorSize;

        VFS::UInt64 sectorCount = 0;
        const bool lastInFile = ( t+1 == tracks.size() ) || tracks[ t+1 ].fileIndex != track.fileIndex;
        if ( lastInFile )
        {
            const VFS::UInt64 fileSize = imageSizes[ track.fileIndex ];
            sectorCount = fileSize > trackStartByte ? ( fileSize - trackStartByte ) / sectorSize : 0;
        }
        else
        {
            const CCueSheet::STrack& next = tracks[ t+1 ];
            const VFS::UInt64 nextRegionStart = next.index0 >= 0 ? (VFS::UInt64) next.index0 : next.index1;
            sectorCount = nextRegionStart > track.index1 ? nextRegionStart - track.index1 : 0;
            fileByteOffset += ( nextRegionStart - regionStart ) * sectorSize;
        }

        CCdTrackIOAccess::SLayout layout;
        layout.fileOffset = trackStartByte;
        layout.sectorCount = sectorCount;
        layout.sectorSize = sectorSize;
        layout.payloadOffset = CCueSheet::GetPayloadOffset( track.mode );
        layout.payloadSize = CCueSheet::GetPayloadSize( track.mode );
        const VFS::CString& imagePath = imagePaths[ track.fileIndex ];

        if ( CCueSheet::IsDataTrack( track.mode ) )
        {
            // The first data track holds the file system
            if ( !m_dataArchive.IsNULL() )
                continue;

            m_dataArchive = VFS::CVfsGlobal::Instance()->GetVfs().CreateArchive( "iso9660" );
            if ( m_dataArchive.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CueBinArchive: There is no \"iso9660\" archive type to read the data track of \""
                    << sheetPath << "\" with, load the vfspluginISO9660 plugin" );
                UnloadArchive();
                return false;
            }

            CORE::IOAccessPtr dataAccess( GUCEF_NEW CCdTrackIOAccess( imagePath, layout ) );
            VFS::TBasicVfsResourcePtr dataResource( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( dataAccess, archiveName, archivePath ) ) );
            if ( !dataAccess->Opened() || !m_dataArchive->LoadArchive( archiveName + ".data", dataResource, false ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CueBinArchive: Track " << track.number << " of \"" << sheetPath << "\" does not hold an ISO 9660 file system" );
                UnloadArchive();
                return false;
            }
        }
        else
        {
            SAudioTrack audio;
            char name[ 32 ];
            snprintf( name, sizeof( name ), "TRACK%02u.WAV", (unsigned) track.number );
            audio.name = name;
            audio.imagePath = imagePath;
            audio.layout = layout;
            MakeCdAudioWaveHeader( sectorCount * layout.payloadSize, audio.header );
            audio.size = audio.header.GetDataSize() + sectorCount * layout.payloadSize;
            m_audioTracks[ audio.name.Lowercase() ] = audio;
        }
    }

    m_archiveName = archiveName;
    m_archivePath = archivePath;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CueBinArchive: Loaded \"" << sheetPath << "\": " << tracks.size() << " tracks, " <<
        ( m_dataArchive.IsNULL() ? "no data track" : "a data track" ) << ", " << m_audioTracks.size() << " audio tracks" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // This is a read only format
    if ( settings.GetWriteableRequested() )
        return false;

    const VFS::CString sheetPath = settings.GetActualArchivePath();
    CORE::CDynamicBuffer sheet;
    if ( !sheet.LoadContentFromFile( sheetPath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CueBinArchive: Failed to read the CUE sheet: " << sheetPath );
        return false;
    }
    const VFS::CString text( sheet.AsConstTypePtr< char >(), sheet.GetDataSize() );
    return Load( settings.GetArchiveName(), settings.GetArchivePath(), text, sheetPath );
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::LoadArchive( const VFS::CString& archiveName       ,
                             VFS::TBasicVfsResourcePtr vfsResource ,
                             const bool writeableRequest           )
{GUCEF_TRACE;

    if ( writeableRequest || vfsResource.IsNULL() || vfsResource->GetAccess().IsNULL() )
        return false;

    // The sheet can come from anywhere, the image files it names are looked for next to where the resource says it is
    CORE::IOAccessPtr access = vfsResource->GetAccess();
    CORE::CDynamicBuffer sheet;
    access->Setpos( 0 );
    sheet.Append( *access );
    const VFS::CString text( sheet.AsConstTypePtr< char >(), sheet.GetDataSize() );
    return Load( archiveName, vfsResource->GetFilePath(), text, vfsResource->GetFilePath() );
}

/*-------------------------------------------------------------------------*/

bool
CCueBinArchive::UnloadArchive( void )
{GUCEF_TRACE;

    if ( !m_dataArchive.IsNULL() )
    {
        m_dataArchive->UnloadArchive();
        m_dataArchive.Unlink();
    }
    m_audioTracks.clear();
    m_sheet = CCueSheet();
    return true;
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CCueBinArchive::GetType( void ) const
{GUCEF_TRACE;

    return CueArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

void
CCueBinArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != objectToBeDestroyed )
    {
        objectToBeDestroyed->GetAccess().Unlink();
        GUCEF_DELETE objectToBeDestroyed;
    }
}

/*-------------------------------------------------------------------------*/

const CCueSheet&
CCueBinArchive::GetCueSheet( void ) const
{GUCEF_TRACE;

    return m_sheet;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CUEBIN */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
