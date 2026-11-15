/*
 *  vfspluginUNITYPACKAGE: GUCEF VFS plugin presenting a Unity package as the project tree it contains
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

#include "archive.h"
#include "archive_entry.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#include "vfspluginUNITYPACKAGE_CUnityPackageReader.h"
#include "vfspluginUNITYPACKAGE_CUnityPackageEntryIOAccess.h"
#include "vfspluginUNITYPACKAGE_CUnityPackageArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace UNITYPACKAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** the roles the files in the folder of an asset can have */
enum EEntryRole
{
    ENTRYROLE_NONE = 0 ,
    ENTRYROLE_PATHNAME ,
    ENTRYROLE_ASSET    ,
    ENTRYROLE_META     ,
    ENTRYROLE_PREVIEW
};

/** the pathname file is a single line, anything beyond this is not a path */
static const UInt32 MaxPathnameFileSize = 4096;

/*-------------------------------------------------------------------------*/

static CORE::CString
GetEntryPath( struct archive_entry* entry )
{GUCEF_TRACE;

    const char* path = archive_entry_pathname( entry );
    return CORE::CString( GUCEF_NULL != path ? path : "" );
}

/*-------------------------------------------------------------------------*/

/** the last segment of a normalized path, paths always use '/' regardless of the platform */
static CORE::CString
LastPathSegment( const CORE::CString& path )
{GUCEF_TRACE;

    Int32 lastSeparator = path.HasChar( '/', false );
    if ( lastSeparator < 0 )
        return path;
    return path.CutChars( (UInt32) lastSeparator + 1, true );
}

/*-------------------------------------------------------------------------*/

static EEntryRole
GetEntryRole( const CORE::CString& name )
{GUCEF_TRACE;

    if ( name == "pathname" )
        return ENTRYROLE_PATHNAME;
    if ( name == "asset" )
        return ENTRYROLE_ASSET;
    if ( name == "asset.meta" )
        return ENTRYROLE_META;
    if ( name == "preview.png" )
        return ENTRYROLE_PREVIEW;
    return ENTRYROLE_NONE;
}

/*-------------------------------------------------------------------------*/

/**
 *  The content of a pathname file is the project path on its first line. The line can be followed by a line
 *  end and, in some versions of the format, a second line which we do not need
 */
static CORE::CString
ParsePathname( const char* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 length = 0;
    while ( length < size && '\n' != data[ length ] && '\r' != data[ length ] && '\0' != data[ length ] )
        ++length;

    UInt32 start = 0;
    while ( start < length && ( ' ' == data[ start ] || '\t' == data[ start ] ) )
        ++start;
    while ( length > start && ( ' ' == data[ length-1 ] || '\t' == data[ length-1 ] ) )
        --length;

    if ( length <= start )
        return CORE::CString();

    CORE::CString pathname;
    pathname.Set( data + start, length - start );
    return CUnityPackageArchive::NormalizePath( pathname );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityPackageArchive::SEntry::SEntry( void )
    : index( 0 )
    , size( 0 )
    , modificationTime()
    , hasModificationTime( false )
    , cacheSlot( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityPackageArchive::SAsset::SAsset( void )
    : pathname()
    , hasAsset( false )
    , hasMeta( false )
    , hasPreview( false )
    , asset()
    , meta()
    , preview()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityPackageArchive::CUnityPackageArchive( void )
    : VFS::CArchive()
    , m_files()
    , m_dirs()
    , m_cache()
    , m_archiveName()
    , m_archivePath()
    , m_actualArchivePath()
    , m_sourceAccess()
    , m_sourceResource()
    , m_type()
    , m_loaded( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityPackageArchive::~CUnityPackageArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityPackageArchive::NormalizePath( const CORE::CString& path )
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
CUnityPackageArchive::AddParentDirs( const CORE::CString& path )
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

void
CUnityPackageArchive::AddFile( const CORE::CString& path, const SEntry& entry )
{GUCEF_TRACE;

    if ( m_files.find( path ) != m_files.end() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: More than one asset has the path \"" << path << "\", keeping the first" );
        return;
    }

    m_files[ path ] = entry;
    AddParentDirs( path );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::BuildIndex( const CORE::CString& path, const CORE::IOAccessPtr& sourceAccess )
{GUCEF_TRACE;

    CUnityPackageReader reader;
    if ( !reader.Open( path, sourceAccess ) )
        return false;

    // Pass over the entire package: record where each entry is, read the pathnames and keep the small entries
    TAssetMap assets;
    UInt32 cacheBytes = 0;
    UInt32 ignoredEntries = 0;
    struct archive_entry* entry = GUCEF_NULL;
    while ( reader.NextEntry( &entry ) )
    {
        UInt32 index = reader.GetEntryCount() - 1;

        EEntryRole role = ENTRYROLE_NONE;
        CORE::CString guid;
        if ( AE_IFREG == archive_entry_filetype( entry ) )
        {
            // Entries are "<guid>/<name>", what is at the root, like the icon of a store download, is not of interest
            CORE::CString entryPath = NormalizePath( GetEntryPath( entry ) );
            Int32 separator = entryPath.HasChar( '/', true );
            if ( separator > 0 )
            {
                guid = entryPath.SubstrToIndex( (UInt32) separator, true );
                role = GetEntryRole( entryPath.CutChars( (UInt32) separator + 1, true ) );
            }
        }
        if ( ENTRYROLE_NONE == role )
        {
            ++ignoredEntries;
            continue;
        }

        SEntry fileEntry;
        fileEntry.index = index;
        fileEntry.size = archive_entry_size_is_set( entry ) ? (UInt64) archive_entry_size( entry ) : 0;
        fileEntry.hasModificationTime = 0 != archive_entry_mtime_is_set( entry );
        if ( fileEntry.hasModificationTime )
        {
            UInt64 ticks = (UInt64) archive_entry_mtime( entry ) * 1000 + (UInt64) ( archive_entry_mtime_nsec( entry ) / 1000000 );
            fileEntry.modificationTime.FromUnixEpochBasedTicksInMillisecs( ticks );
        }

        SAsset& asset = assets[ guid ];
        if ( ENTRYROLE_PATHNAME == role )
        {
            char data[ MaxPathnameFileSize ];
            UInt64 toRead = fileEntry.size < MaxPathnameFileSize ? fileEntry.size : MaxPathnameFileSize;
            UInt64 bytesRead = reader.ReadEntryData( data, toRead );
            asset.pathname = ParsePathname( data, (UInt32) bytesRead );
            continue;
        }

        // Keep the small ones, the data is passing by anyway
        if ( fileEntry.size <= CacheEntryLimit && cacheBytes + fileEntry.size <= CacheTotalLimit )
        {
            CORE::CDynamicBuffer* content = GUCEF_NEW CORE::CDynamicBuffer();
            content->SetDataSize( (UInt32) fileEntry.size );
            if ( reader.ReadEntryData( content->GetBufferPtr(), fileEntry.size ) == fileEntry.size )
            {
                fileEntry.cacheSlot = (Int32) m_cache.size();
                m_cache.push_back( content );
                cacheBytes += (UInt32) fileEntry.size;
            }
            else
            {
                GUCEF_DELETE content;
            }
        }

        switch ( role )
        {
            case ENTRYROLE_ASSET:   { asset.hasAsset = true; asset.asset = fileEntry; break; }
            case ENTRYROLE_META:    { asset.hasMeta = true; asset.meta = fileEntry; break; }
            case ENTRYROLE_PREVIEW: { asset.hasPreview = true; asset.preview = fileEntry; break; }
            default: break;
        }
    }

    if ( reader.HasFailed() )
        return false;
    if ( assets.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: \"" << path << "\" does not hold any Unity package assets" );
        return false;
    }

    // Now that all pathnames are known present the assets as the project tree
    UInt32 assetsWithoutPathname = 0;
    TAssetMap::const_iterator i = assets.begin();
    while ( i != assets.end() )
    {
        const SAsset& asset = (*i).second;
        if ( asset.pathname.IsNULLOrEmpty() )
        {
            ++assetsWithoutPathname;
            ++i;
            continue;
        }

        if ( asset.hasAsset )
        {
            AddFile( asset.pathname, asset.asset );
        }
        else
        {
            // A folder
            m_dirs.insert( asset.pathname );
            AddParentDirs( asset.pathname );
        }
        if ( asset.hasMeta )
            AddFile( asset.pathname + ".meta", asset.meta );
        if ( asset.hasPreview )
            AddFile( asset.pathname + ".preview.png", asset.preview );
        ++i;
    }

    if ( assetsWithoutPathname > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: " << assetsWithoutPathname << " asset folders in \"" << path << "\" have no usable pathname and are left out" );
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UnityPackageArchive: Indexed " << (UInt32) assets.size() << " assets, " << ignoredEntries <<
        " other entries ignored, " << cacheBytes << " bytes kept in memory" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // We only support reading
    if ( settings.GetWriteableRequested() )
        return false;

    UnloadArchive();

    // Note that a package that is not a file on disk can be mounted using the resource based mount
    // That path cannot be taken here as the VFS is locked while it is loading us
    if ( !BuildIndex( settings.GetActualArchivePath(), CORE::IOAccessPtr() ) )
    {
        UnloadArchive();
        return false;
    }

    m_archiveName = settings.GetArchiveName();
    m_archivePath = settings.GetArchivePath();
    m_actualArchivePath = settings.GetActualArchivePath();
    m_type = settings.GetArchiveType();
    m_loaded = true;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: Loaded Unity package \"" << m_actualArchivePath << "\" with " <<
        (UInt32) m_files.size() << " files and " << (UInt32) m_dirs.size() << " directories" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::LoadArchive( const VFS::CString& archiveName       ,
                                   VFS::TBasicVfsResourcePtr vfsResource ,
                                   const bool writeableRequest           )
{GUCEF_TRACE;

    if ( writeableRequest || vfsResource.IsNULL() || vfsResource->GetAccess().IsNULL() )
        return false;

    UnloadArchive();

    // The readers work on clones of the access object of the resource
    m_sourceAccess = vfsResource->GetAccess();
    if ( !BuildIndex( vfsResource->GetFilePath(), m_sourceAccess ) )
    {
        UnloadArchive();
        return false;
    }

    // The resource can own more than the access object, like the archive it is a file of, so keep it
    // for as long as we use the package
    m_sourceResource = vfsResource;
    m_archiveName = archiveName;
    m_archivePath = vfsResource->GetFilePath();
    m_actualArchivePath = vfsResource->GetFilePath();
    m_type = "unitypackage";
    m_loaded = true;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: Loaded Unity package \"" << m_archivePath << "\" from a resource with " <<
        (UInt32) m_files.size() << " files and " << (UInt32) m_dirs.size() << " directories" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::UnloadArchive( void )
{GUCEF_TRACE;

    m_files.clear();
    m_dirs.clear();
    for ( TCache::iterator i = m_cache.begin(); i != m_cache.end(); ++i )
        GUCEF_DELETE (*i);
    m_cache.clear();
    m_sourceAccess.Unlink();
    m_sourceResource.Unlink();
    m_loaded = false;
    return true;
}

/*-------------------------------------------------------------------------*/

const CUnityPackageArchive::SEntry*
CUnityPackageArchive::FindEntry( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_files.find( NormalizePath( filePath ) );
    if ( i != m_files.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CUnityPackageArchive::GetFile( const VFS::CString& file      ,
                               const char* mode              ,
                               const VFS::UInt32 memLoadSize ,
                               const bool overwrite          )
{GUCEF_TRACE;

    // We only support read only modes
    if ( GUCEF_NULL == mode || 'r' != *mode || GUCEF_NULL != strchr( mode, '+' ) )
        return VFS::TBasicVfsResourcePtr();

    const SEntry* entry = FindEntry( file );
    if ( GUCEF_NULL == entry )
        return VFS::TBasicVfsResourcePtr();

    CORE::IOAccessPtr fileAccess;
    if ( entry->cacheSlot >= 0 )
    {
        // Kept in memory since mounting, the access gets its own copy as it can outlive the archive
        CORE::CDynamicBuffer* copy = GUCEF_NEW CORE::CDynamicBuffer( *m_cache[ entry->cacheSlot ] );
        fileAccess = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( copy, true ) );
    }
    else
    if ( entry->size <= LoadOnOpenLimit )
    {
        CORE::CDynamicBuffer* content = GUCEF_NEW CORE::CDynamicBuffer();
        CUnityPackageReader reader;
        if ( !reader.ReadWholeEntry( m_actualArchivePath, m_sourceAccess, entry->index, (UInt32) entry->size, *content ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageArchive: Failed to read \"" << file << "\" from package \"" << m_actualArchivePath << "\"" );
            GUCEF_DELETE content;
            return VFS::TBasicVfsResourcePtr();
        }
        fileAccess = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( content, true ) );
    }
    else
    {
        CUnityPackageEntryIOAccess* access = GUCEF_NEW CUnityPackageEntryIOAccess( m_actualArchivePath, m_sourceAccess, entry->index, entry->size );
        if ( !access->IsValid() )
        {
            GUCEF_DELETE access;
            return VFS::TBasicVfsResourcePtr();
        }
        fileAccess = CORE::IOAccessPtr( access );
    }

    CORE::CString filePath = m_archivePath;
    CORE::AppendToPath( filePath, file );
    return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( fileAccess, file, filePath ), this );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::StoreAsFile( const CORE::CString& filepath    ,
                                   const CORE::CDynamicBuffer& data ,
                                   const CORE::UInt64 offset        ,
                                   const bool overwrite             )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageArchive::GetFileList( TStringVector& outputList                   ,
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
CUnityPackageArchive::GetDirList( TStringVector& outputList                   ,
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
CUnityPackageArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::MoveFile( const VFS::CString& oldFilePath ,
                                const VFS::CString& newFilePath ,
                                const bool overwrite            )
{GUCEF_TRACE;

    // Read only
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::GetFileMetaData( const VFS::CString& filePath      ,
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
CUnityPackageArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    return GUCEF_NULL != FindEntry( filePath );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    CORE::CString path = NormalizePath( dirPath );
    return path.IsNULLOrEmpty() || m_dirs.find( path ) != m_dirs.end();
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CUnityPackageArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const SEntry* entry = FindEntry( filePath );
    return GUCEF_NULL != entry ? entry->size : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CUnityPackageArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const SEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL != entry && entry->hasModificationTime )
        return entry->modificationTime;
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CUnityPackageArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    // GetFile() is not const but does not change the archive, it only hands out a new access object
    VFS::TBasicVfsResourcePtr resource = const_cast< CUnityPackageArchive* >( this )->GetFile( file, "rb", 0, false );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        return VFS::CString();

    UInt8 digest[ 16 ];
    if ( 0 != CORE::md5fromfile( resource->GetAccess()->CStyleAccess(), digest ) )
    {
        char md5Str[ 48 ];
        CORE::md5tostring( digest, md5Str );
        return VFS::CString( md5Str );
    }
    return VFS::CString();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CUnityPackageArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CUnityPackageArchive::GetType( void ) const
{GUCEF_TRACE;

    return m_type;
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITYPACKAGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
