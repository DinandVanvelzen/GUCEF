/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H
#include "gucefASSETSTORE_CRemoteFileAccess.h"
#define GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H ? */

#include "gucefASSETSTORE_CRemoteAssetArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteAssetArchive::CRemoteAssetArchive( void )
    : VFS::CArchive()
    , m_archiveName()
    , m_fileSystemId()
    , m_serviceRoot()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetArchive::~CRemoteAssetArchive()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetArchive::ToServicePath( const CORE::CString& archivePath ) const
{GUCEF_TRACE;

    const CORE::CString relative = CRemoteAssetFileSystem::Normalize( archivePath );
    return relative.IsNULLOrEmpty() ? m_serviceRoot : m_serviceRoot + "/" + relative;
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CRemoteAssetArchive::GetFile( const CORE::CString& file, const char* mode, const UInt32 memLoadSize, const bool overwrite )
{GUCEF_TRACE;

    if ( GUCEF_NULL == mode || 'r' != *mode )
        return VFS::TBasicVfsResourcePtr();
    const CORE::CString servicePath = ToServicePath( file );
    CORE::IOAccessPtr access( GUCEF_NEW CRemoteFileAccess( m_fileSystemId, servicePath ) );
    if ( !access->IsValid() )
        return VFS::TBasicVfsResourcePtr();
    return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, servicePath, file ) );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::StoreAsFile( const CORE::CString& filepath, const CORE::CDynamicBuffer& data, const CORE::UInt64 offset, const bool overwrite )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetArchive::GetFileList( TStringVector& outputList, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive,
                                  bool includePathInFilename, const CORE::CString::StringSet& nameFilters, UInt32 maxListEntries ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system )
        return;
    CORE::CString::StringVector pending;
    pending.push_back( CRemoteAssetFileSystem::Normalize( archiveLocation ) );
    const UInt32 limit = recursive ? system->GetSettings().maxListEntries : maxListEntries;
    while ( !pending.empty() && outputList.size() < limit )
    {
        const CORE::CString folder = pending.back();
        pending.pop_back();
        CORE::CString::StringVector dirs;
        CORE::CString::StringVector files;
        bool truncated = false;
        if ( !system->GetFolderEntries( ToServicePath( folder ), dirs, files, &truncated ) )
            continue;
        const CORE::CString vfsFolder = CORE::CombinePath( mountLocation, folder );
        for ( size_t i=0; i<files.size() && outputList.size() < limit; ++i )
        {
            if ( !VFS::CVFS::FilterValidation( files[ i ], nameFilters ) )
                continue;
            if ( includePathInFilename )
                outputList.push_back( CORE::CombinePath( vfsFolder, files[ i ] ).ReplaceChar( '\\', '/' ) );
            else
                outputList.push_back( files[ i ] );
        }
        if ( recursive )
            for ( size_t i=0; i<dirs.size(); ++i )
                pending.push_back( folder.IsNULLOrEmpty() ? dirs[ i ] : folder + "/" + dirs[ i ] );
    }
    if ( recursive && outputList.size() >= limit )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetArchive: The list of " << m_serviceRoot << "/" << archiveLocation << " was cut at " << limit << " entries" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetArchive::GetDirList( TStringVector& outputList, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive,
                                 bool includePathInFilename, const CORE::CString::StringSet& nameFilters, UInt32 maxListEntries ) const
{GUCEF_TRACE;

    // as the file system archive: the folders directly in the location, with a '/' at the end
    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system )
        return;
    CORE::CString::StringVector dirs;
    CORE::CString::StringVector files;
    if ( !system->GetFolderEntries( ToServicePath( archiveLocation ), dirs, files ) )
        return;
    for ( size_t i=0; i<dirs.size() && outputList.size() < maxListEntries; ++i )
    {
        if ( !VFS::CVFS::FilterValidation( dirs[ i ], nameFilters ) )
            continue;
        if ( includePathInFilename )
            outputList.push_back( CORE::CombinePath( CORE::CombinePath( mountLocation, archiveLocation ), dirs[ i ] ).ReplaceChar( '\\', '/' ) + "/" );
        else
            outputList.push_back( dirs[ i ] + "/" );
    }
}

/*-------------------------------------------------------------------------*/

bool CRemoteAssetArchive::DeleteFile( const CORE::CString& filePath ) { return false; }
bool CRemoteAssetArchive::MoveFile( const CORE::CString& oldFilePath, const CORE::CString& newFilePath, const bool overwrite ) { return false; }

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::FileExists( const CORE::CString& filePath ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system || CRemoteAssetFileSystem::Normalize( filePath ).IsNULLOrEmpty() )
        return false;
    CRemoteAssetFileSystem::SFileInfo info;
    return system->GetFileInfo( ToServicePath( filePath ), info ) && info.exists && !info.isDirectory;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::DirExists( const CORE::CString& dirPath ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system )
        return false;
    if ( CRemoteAssetFileSystem::Normalize( dirPath ).IsNULLOrEmpty() )
        return true;
    CRemoteAssetFileSystem::SFileInfo info;
    return system->GetFileInfo( ToServicePath( dirPath ), info ) && info.exists && info.isDirectory;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteAssetArchive::GetFileSize( const CORE::CString& filePath ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    CRemoteAssetFileSystem::SFileInfo info;
    return GUCEF_NULL != system && system->GetFileInfo( ToServicePath( filePath ), info ) ? info.size : 0;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::GetFileMetaData( const CORE::CString& filePath, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    CRemoteAssetFileSystem::SFileInfo info;
    if ( GUCEF_NULL == system || !system->GetFileInfo( ToServicePath( filePath ), info ) )
        return false;
    metaData.hasResourceSizeInBytes = !info.isDirectory;
    metaData.resourceSizeInBytes = info.size;
    metaData.hasIsReadOnly = true;
    metaData.isReadOnly = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetArchive::GetFileHash( const CORE::CString& file ) const
{GUCEF_TRACE;

    // the SHA-256 that the catalogue gave: nothing is read or asked for it; empty when it is not known
    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    CORE::CString digest;
    if ( GUCEF_NULL != system )
        system->GetKnownDigest( ToServicePath( file ), digest );
    return digest;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CRemoteAssetArchive::GetFileModificationTime( const CORE::CString& filePath ) const
{GUCEF_TRACE;

    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetArchive::GetArchiveName( void ) const
{
    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool CRemoteAssetArchive::IsWriteable( void ) const { return false; }

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // the actual path is "<id of the file system>|<root of the location on the service>"
    const CORE::CString actual = settings.GetActualArchivePath();
    const Int32 bar = actual.HasChar( '|', true );
    if ( bar <= 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetArchive: The archive has no file system of a store service to read through" );
        return false;
    }
    m_fileSystemId = actual.SubstrFromRange( 0, (UInt32) bar );
    m_serviceRoot = CRemoteAssetFileSystem::Normalize( actual.CutChars( (UInt32) bar + 1, true ) );
    m_archiveName = settings.GetArchiveName();
    return GUCEF_NULL != CRemoteAssetFileSystem::FindById( m_fileSystemId ) && !m_serviceRoot.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::LoadArchive( const CORE::CString& archiveName, VFS::TBasicVfsResourcePtr vfsResource, const bool writeableRequest )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetArchive: An archive of the type assetstore is a location of a store service: it cannot be loaded from a resource" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::UnloadArchive( void )
{GUCEF_TRACE;

    m_fileSystemId.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetArchive::GetType( void ) const
{GUCEF_TRACE;

    return CRemoteAssetFileSystem::ArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetArchive::IsConnected( void ) const
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    return GUCEF_NULL != system && !system->IsOffline();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
