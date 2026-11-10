/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#include "archive.h"
#include "archive_entry.h"

#include "vfspluginLIBARCHIVE_CUtf8LocaleScope.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#include "vfspluginLIBARCHIVE_CLibArchiveEntryIOAccess.h"
#include "vfspluginLIBARCHIVE_CLibArchiveArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
GetEntryPath( struct archive_entry* entry )
{GUCEF_TRACE;

    const char* utf8Path = archive_entry_pathname_utf8( entry );
    if ( GUCEF_NULL != utf8Path )
        return CORE::CString( utf8Path );
    const char* path = archive_entry_pathname( entry );
    return CORE::CString( GUCEF_NULL != path ? path : "" );
}

/*-------------------------------------------------------------------------*/

/** the last segment of a normalized archive path, archive paths always use '/' regardless of the platform */
static CORE::CString
LastPathSegment( const CORE::CString& path )
{GUCEF_TRACE;

    Int32 lastSeparator = path.HasChar( '/', false );
    if ( lastSeparator < 0 )
        return path;
    return path.CutChars( (UInt32) lastSeparator + 1, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLibArchiveArchive::CLibArchiveArchive( void )
    : VFS::CArchive()
    , m_files()
    , m_dirs()
    , m_archiveName()
    , m_archivePath()
    , m_actualArchivePath()
    , m_type()
    , m_loaded( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLibArchiveArchive::~CLibArchiveArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

struct archive*
CLibArchiveArchive::OpenForReading( const CORE::CString& path )
{GUCEF_TRACE;

    struct archive* reader = archive_read_new();
    if ( GUCEF_NULL == reader )
        return GUCEF_NULL;

    archive_read_support_filter_all( reader );
    archive_read_support_format_all( reader );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    std::wstring widePath = CORE::ToWString( path );
    int result = archive_read_open_filename_w( reader, widePath.c_str(), 64 * 1024 );
    #else
    int result = archive_read_open_filename( reader, path.C_String(), 64 * 1024 );
    #endif

    if ( ARCHIVE_OK != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveArchive: Unable to open archive \"" + path + "\": " + CORE::CString( archive_error_string( reader ) ? archive_error_string( reader ) : "" ) );
        archive_read_free( reader );
        return GUCEF_NULL;
    }
    return reader;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibArchiveArchive::NormalizePath( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    while ( true )
    {
        if ( normalized.Length() >= 2 && '.' == normalized[ 0 ] && '/' == normalized[ 1 ] )
            normalized = normalized.CutChars( 2, true );
        else
        if ( normalized.Length() >= 1 && '/' == normalized[ 0 ] )
            normalized = normalized.CutChars( 1, true );
        else
            break;
    }
    while ( normalized.Length() > 0 && '/' == normalized[ normalized.Length()-1 ] )
        normalized = normalized.CutChars( 1, false );
    return normalized;
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveArchive::AddParentDirs( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString dir = path;
    while ( true )
    {
        Int32 lastSeparator = dir.HasChar( '/', false );
        if ( lastSeparator <= 0 )
            break;
        dir = dir.SubstrToIndex( (UInt32) lastSeparator, true );
        m_dirs.insert( dir );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    // We only support reading
    if ( settings.GetWriteableRequested() )
        return false;

    UnloadArchive();

    struct archive* reader = OpenForReading( settings.GetActualArchivePath() );
    if ( GUCEF_NULL == reader )
        return false;

    // Index all entries, for regular files we remember their position for reading them later
    bool success = true;
    UInt32 index = 0;
    struct archive_entry* entry = GUCEF_NULL;
    while ( true )
    {
        int result = archive_read_next_header( reader, &entry );
        if ( ARCHIVE_EOF == result )
            break;
        if ( ARCHIVE_OK != result && ARCHIVE_WARN != result )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveArchive: Error reading archive \"" + settings.GetActualArchivePath() + "\": " +
                CORE::CString( archive_error_string( reader ) ? archive_error_string( reader ) : "" ) );
            success = false;
            break;
        }

        CORE::CString path = NormalizePath( GetEntryPath( entry ) );
        if ( !path.IsNULLOrEmpty() )
        {
            if ( AE_IFDIR == archive_entry_filetype( entry ) )
            {
                m_dirs.insert( path );
            }
            else
            if ( AE_IFREG == archive_entry_filetype( entry ) )
            {
                SEntry fileEntry;
                fileEntry.index = index;
                fileEntry.size = archive_entry_size_is_set( entry ) ? (UInt64) archive_entry_size( entry ) : 0;
                fileEntry.hasModificationTime = 0 != archive_entry_mtime_is_set( entry );
                if ( fileEntry.hasModificationTime )
                {
                    UInt64 ticks = (UInt64) archive_entry_mtime( entry ) * 1000 + (UInt64) ( archive_entry_mtime_nsec( entry ) / 1000000 );
                    fileEntry.modificationTime.FromUnixEpochBasedTicksInMillisecs( ticks );
                }
                m_files[ path ] = fileEntry;
            }
            AddParentDirs( path );
        }

        archive_read_data_skip( reader );
        ++index;
    }
    archive_read_free( reader );

    if ( !success )
    {
        UnloadArchive();
        return false;
    }

    m_archiveName = settings.GetArchiveName();
    m_archivePath = settings.GetArchivePath();
    m_actualArchivePath = settings.GetActualArchivePath();
    m_type = settings.GetArchiveType();
    m_loaded = true;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveArchive: Loaded archive \"" + m_actualArchivePath + "\" with " +
        CORE::ToString( (UInt32) m_files.size() ) + " files and " + CORE::ToString( (UInt32) m_dirs.size() ) + " directories" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::LoadArchive( const VFS::CString& archiveName       ,
                                 VFS::TBasicVfsResourcePtr vfsResource ,
                                 const bool writeableRequest           )
{GUCEF_TRACE;

    // Archives inside other archives are not supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::UnloadArchive( void )
{GUCEF_TRACE;

    m_files.clear();
    m_dirs.clear();
    m_loaded = false;
    return true;
}

/*-------------------------------------------------------------------------*/

const CLibArchiveArchive::SEntry*
CLibArchiveArchive::FindEntry( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_files.find( NormalizePath( filePath ) );
    if ( i != m_files.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CLibArchiveArchive::GetFile( const VFS::CString& file      ,
                             const char* mode              ,
                             const VFS::UInt32 memLoadSize ,
                             const bool overwrite          )
{GUCEF_TRACE;

    // We only support read only modes
    if ( GUCEF_NULL == mode || 'r' != *mode )
        return VFS::TBasicVfsResourcePtr();

    const SEntry* entry = FindEntry( file );
    if ( GUCEF_NULL == entry )
        return VFS::TBasicVfsResourcePtr();

    CLibArchiveEntryIOAccess* access = GUCEF_NEW CLibArchiveEntryIOAccess( m_actualArchivePath, entry->index, entry->size );
    if ( !access->IsValid() )
    {
        GUCEF_DELETE access;
        return VFS::TBasicVfsResourcePtr();
    }

    CORE::IOAccessPtr fileAccess( access );
    CORE::CString filePath = m_archivePath;
    CORE::AppendToPath( filePath, file );
    return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( fileAccess, file, filePath ), this );
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::StoreAsFile( const CORE::CString& filepath    ,
                                 const CORE::CDynamicBuffer& data ,
                                 const CORE::UInt64 offset        ,
                                 const bool overwrite             )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveArchive::GetFileList( TStringVector& outputList                   ,
                                 const VFS::CString& mountLocation           ,
                                 const VFS::CString& archiveLocation         ,
                                 bool recursive                              ,
                                 bool includePathInFilename                  ,
                                 const CORE::CString::StringSet& nameFilters ,
                                 UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    CORE::CString location = NormalizePath( archiveLocation );
    CORE::CString prefix = location.IsNULLOrEmpty() ? CORE::CString() : location + '/';

    TEntryMap::const_iterator i = m_files.begin();
    while ( i != m_files.end() && outputList.size() < maxListEntries )
    {
        const CORE::CString& path = (*i).first;
        if ( prefix.IsNULLOrEmpty() || path.HasSubstr( prefix, true ) == 0 )
        {
            CORE::CString remainder = path.CutChars( prefix.Length(), true );
            if ( recursive || remainder.HasChar( '/' ) < 0 )
            {
                CORE::CString filename = LastPathSegment( path );
                if ( VFS::CVFS::FilterValidation( filename, nameFilters ) )
                {
                    if ( includePathInFilename )
                        outputList.push_back( CORE::CombinePath( mountLocation, path ).ReplaceChar( '\\', '/' ) );
                    else
                        outputList.push_back( filename );
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveArchive::GetDirList( TStringVector& outputList                   ,
                                const VFS::CString& mountLocation           ,
                                const VFS::CString& archiveLocation         ,
                                bool recursive                              ,
                                bool includeParentDirInName                 ,
                                const CORE::CString::StringSet& nameFilters ,
                                UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    CORE::CString location = NormalizePath( archiveLocation );
    CORE::CString prefix = location.IsNULLOrEmpty() ? CORE::CString() : location + '/';

    TDirSet::const_iterator i = m_dirs.begin();
    while ( i != m_dirs.end() && outputList.size() < maxListEntries )
    {
        const CORE::CString& path = (*i);
        if ( prefix.IsNULLOrEmpty() || path.HasSubstr( prefix, true ) == 0 )
        {
            CORE::CString remainder = path.CutChars( prefix.Length(), true );
            if ( !remainder.IsNULLOrEmpty() && ( recursive || remainder.HasChar( '/' ) < 0 ) )
            {
                CORE::CString dirName = LastPathSegment( path );
                if ( VFS::CVFS::FilterValidation( dirName, nameFilters ) )
                {
                    if ( includeParentDirInName )
                        outputList.push_back( CORE::CombinePath( mountLocation, path ).ReplaceChar( '\\', '/' ) + '/' );
                    else
                        outputList.push_back( remainder + '/' );
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::MoveFile( const VFS::CString& oldFilePath ,
                              const VFS::CString& newFilePath ,
                              const bool overwrite            )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::GetFileMetaData( const VFS::CString& filePath      ,
                                     CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    const SEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL == entry )
        return false;

    metaData.resourceExists = true;
    metaData.name = LastPathSegment( NormalizePath( filePath ) );
    metaData.hasName = true;
    metaData.resourceSizeInBytes = entry->size;
    metaData.hasResourceSizeInBytes = true;
    metaData.modifiedDateTime = entry->modificationTime;
    metaData.hasModifiedDateTime = entry->hasModificationTime;
    metaData.isReadOnly = true;
    metaData.hasIsReadOnly = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    return GUCEF_NULL != FindEntry( filePath );
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    CORE::CString path = NormalizePath( dirPath );
    return path.IsNULLOrEmpty() || m_dirs.find( path ) != m_dirs.end();
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CLibArchiveArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const SEntry* entry = FindEntry( filePath );
    return GUCEF_NULL != entry ? entry->size : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CLibArchiveArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const SEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL != entry && entry->hasModificationTime )
        return entry->modificationTime;
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CLibArchiveArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    const SEntry* entry = FindEntry( file );
    if ( GUCEF_NULL == entry )
        return VFS::CString();

    CLibArchiveEntryIOAccess access( m_actualArchivePath, entry->index, entry->size );
    if ( !access.IsValid() )
        return VFS::CString();

    UInt8 digest[ 16 ];
    if ( 0 != CORE::md5fromfile( access.CStyleAccess(), digest ) )
    {
        char md5Str[ 48 ];
        CORE::md5tostring( digest, md5Str );
        return VFS::CString( md5Str );
    }
    return VFS::CString();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CLibArchiveArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CLibArchiveArchive::GetType( void ) const
{GUCEF_TRACE;

    return m_type;
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
