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

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefVFS_CFileSystemArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemArchive::CFileSystemArchive( void )
    : m_diskCacheList()            
    , m_rootDir()         
    , m_archiveName() 
    , m_writable( false )
    , m_fsWatcher()
{GUCEF_TRACE;

    SubscribeTo( &m_fsWatcher );
}

/*-------------------------------------------------------------------------*/

CFileSystemArchive::CFileSystemArchive( const CString& archiveName ,
                                        const CString& rootDir     ,
                                        bool writeable             )
    : m_diskCacheList()            
    , m_rootDir( rootDir )         
    , m_archiveName( archiveName ) 
    , m_writable( writeable )
    , m_fsWatcher()
{GUCEF_TRACE;

    SubscribeTo( &m_fsWatcher );
}

/*-------------------------------------------------------------------------*/

CFileSystemArchive::~CFileSystemArchive()
{GUCEF_TRACE;

    m_diskCacheList.clear();
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::LoadArchive( const CArchiveSettings& settings )
{GUCEF_TRACE;

    m_archiveName = settings.GetArchiveName();
    m_rootDir = settings.GetActualArchivePath();
    m_writable = settings.GetWriteableRequested();

    if ( settings.GetAutoMountSubArchives() )
    {
        CVFS& vfs = CVfsGlobal::Instance()->GetVfs();

        // Get a list of all files from the root onward, as paths in this archive
        TStringVector files;
        GetFileList( files, CString::Empty, CString::Empty, true, true, CString::EmptyStringSet );

        // Find mountable types
        TStringVector::iterator i = files.begin();
        while ( i != files.end() )
        {
            // Mount based on archive file extention type: the type that declared the extension, or one named like it
            CString archiveType;
            if ( vfs.TryGetArchiveTypeForFile( (*i), archiveType ) )
            {
                // Found a compatible type,.. create an archive for the type: mounted at its VFS path, read from its file
                const CString vfsPath = CORE::CombinePath( settings.GetMountPath(), (*i) ).ReplaceChar( '\\', '/' );
                const CString fsPath = CORE::CombinePath( m_rootDir, (*i).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
                CArchiveSettings subArchiveSettings;
                subArchiveSettings.SetActualArchivePath( fsPath );
                subArchiveSettings.SetArchiveName( vfsPath );
                subArchiveSettings.SetArchivePath( vfsPath );
                subArchiveSettings.SetArchiveType( archiveType );
                subArchiveSettings.SetAutoMountSubArchives( settings.GetAutoMountSubArchives() && settings.GetAutoMountSubArchivesIsRecursive() );
                subArchiveSettings.SetAutoMountSubArchivesIsRecursive( settings.GetAutoMountSubArchivesIsRecursive() );
                subArchiveSettings.SetMountPath( vfsPath );
                subArchiveSettings.SetWriteableRequested( settings.GetWriteableRequested() );
                subArchiveSettings.SetParentArchiveName( settings.GetArchiveName() );
                if ( vfs.MountArchive( subArchiveSettings ) )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive: Auto mounted sub archive with name " << vfsPath );
                }
            }
            ++i;
        }

    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::LoadArchive( const VFS::CString& archiveName  ,
                                 TBasicVfsResourcePtr vfsResource ,
                                 const bool writeableRequest      )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive: Attempting to load a \"FileSystem\" archive from a file. That does not make sense. Bad config?" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::UnloadArchive( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CFileSystemArchive::GetFile( const CString& filepath  ,
                             const char* mode         ,
                             const UInt32 memLoadSize ,
                             const bool overwrite     )

{GUCEF_TRACE;

    return LoadFromDisk( filepath    ,
                         mode        ,
                         GUCEF_NULL  ,
                         memLoadSize ,
                         overwrite   );
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CFileSystemArchive::GetFileAs( const CString& filepath                 ,
                               const CORE::CResourceMetaData& metaData ,
                               const char* mode                        ,
                               const UInt32 memLoadSize                ,
                               const bool overwrite                    )

{GUCEF_TRACE;

    return LoadFromDisk( filepath    ,
                         mode        ,
                         &metaData   ,
                         memLoadSize ,
                         overwrite   );
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::DeleteFile( const CString& filePath )
{GUCEF_TRACE;

    if ( !IsWithinArchive( filePath ) )
        return false;
    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );    
    if ( CORE::FileExists( path ) )
    {
        return CORE::DeleteFile( path );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::MoveFile( const CString& oldFilePath ,
                              const CString& newFilePath ,
                              const bool overwrite       ) 
{GUCEF_TRACE;

    CString oldPath = CORE::CombinePath( m_rootDir, oldFilePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    CString newPath = CORE::CombinePath( m_rootDir, newFilePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    
    CString newPathDir = CORE::StripFilename( newPath );
    if ( !CORE::DirExists( newPathDir ) )
        if ( !CORE::CreateDirs( newPathDir ) )
            return false;

    return CORE::MoveFile( oldPath, newPath, overwrite );
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::StoreAsFile( const CORE::CString& filePath    ,
                                 const CORE::CDynamicBuffer& data ,
                                 const CORE::UInt64 offset        ,
                                 const bool overwrite             )
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    CString dir = CORE::StripFilename( path );
    CORE::CreateDirs( dir );
    return data.WriteContentToFile( path, offset, overwrite );    
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchive::GetListFromRoot( const CORE::CString& actualFsDir  ,
                                     const CString& vfsMountLocation   , 
                                     const CString& vfsArchiveLocation ,
                                     bool recursive                    ,
                                     bool includePathInFilename        ,
                                     const CString::StringSet& filters ,
                                     TStringVector& outputList         ,
                                     bool addFiles                     ,
                                     bool addDirs                      ) const
{GUCEF_TRACE;

    CORE::CString vfsPath = CORE::CombinePath( vfsMountLocation, vfsArchiveLocation );

    CORE::CFileSystemIterator fileSystemIterator;
    if ( fileSystemIterator.FindFirst( actualFsDir ) )
    {
        /*
         *      Iterate the dir content adding the file entries from
         *      the dir to the list
         */
        do
        {
            if ( fileSystemIterator.IsAFile() )
            {
                if ( addFiles )
                {
                    CORE::CString filename = fileSystemIterator.GetResourceName();

                    if ( filename != '.' && filename != ".." )
                    {
                        if ( CVFS::FilterValidation( filename ,
                                                     filters  ) )
                        {
                            if ( !includePathInFilename )
                            {
                                outputList.push_back( filename );
                            }
                            else
                            {
                                CORE::CString vfsFilePath = CORE::CombinePath( vfsPath, filename );
                                vfsFilePath = vfsFilePath.ReplaceChar( '\\', '/' );
                                outputList.push_back( vfsFilePath );
                            }
                        }
                    }
                }
            }
            else
            {
                CORE::CString dirName = fileSystemIterator.GetResourceName();
                if ( dirName != '.' && dirName != ".." )
                {
                    if ( addDirs )
                    {
                        if ( CVFS::FilterValidation( dirName ,
                                                     filters ) )
                        {
                            outputList.push_back( dirName + '/' );
                        }
                    }

                    if ( recursive )
                    {
                        CORE::CString fsSubdir = CORE::CombinePath( actualFsDir, dirName );
                        CORE::CString vfsSubdir = CORE::CombinePath( vfsArchiveLocation, dirName );

                        // Recursively process the sub-dir
                        GetListFromRoot( fsSubdir              ,
                                         vfsMountLocation      ,
                                         vfsSubdir             ,
                                         recursive             ,
                                         includePathInFilename ,
                                         filters               ,
                                         outputList            ,
                                         true                  ,
                                         false                 );
                    }
                }
            }
        }
        while ( fileSystemIterator.FindNext() );
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchive::GetFileList( TStringVector& outputList             ,
                                 const CString& mountLocation          , 
                                 const CString& archiveLocation        ,
                                 bool recursive                        ,
                                 bool includePathInFilename            ,
                                 const CString::StringSet& nameFilters ,
                                 UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    // Translate to a local filesystem path
    CString actualFsRootdir = CombinePath( m_rootDir, archiveLocation.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );

    /*
     *      Process the root
     */
    GetListFromRoot( actualFsRootdir       ,
                     mountLocation         ,
                     archiveLocation       ,
                     recursive             ,
                     includePathInFilename ,
                     nameFilters           ,
                     outputList            ,
                     true                  ,
                     false                 );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchive::GetDirList( TStringVector& outputList             ,
                                const CString& mountLocation          , 
                                const CString& archiveLocation        ,
                                bool recursive                        ,
                                bool includePathInFilename            ,
                                const CString::StringSet& nameFilters ,
                                UInt32 maxListEntries                 ) const
{GUCEF_TRACE;

    // Translate to a local filesystem path
    CString actualFsRootdir = CombinePath( m_rootDir, archiveLocation.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );

    /*
     *      Process the root
     */
    GetListFromRoot( actualFsRootdir       ,
                     mountLocation         ,
                     archiveLocation       ,
                     recursive             ,
                     includePathInFilename ,
                     nameFilters           ,
                     outputList            ,
                     false                 ,
                     true                  );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::FileExists( const CString& filePath ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return CORE::FileExists( path );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::DirExists( const CString& dirPath ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, dirPath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return CORE::DirExists( path );
}

/*-------------------------------------------------------------------------*/

UInt64
CFileSystemArchive::GetFileSize( const CString& filePath ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    if ( CORE::FileExists( path ) )
    {
        return CORE::FileSize( path );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemArchive::GetFileHash( const CString& filePath ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    if ( CORE::FileExists( path ) )
    {
        CORE::CFileAccess fileAccess( path );

        UInt8 md5Hash[ 16 ];
        if ( CORE::md5fromfile( fileAccess.CStyleAccess() ,
                                md5Hash                   ) == 1 )
        {
            return CORE::MD5ToString( md5Hash );
        }
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::SetFileMetaData( const CString& filePath                 ,
                                     const CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return CORE::SetFileMetaData( path, metaData );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::GetFileMetaData( const CString& filePath           ,
                                     CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return CORE::GetFileMetaData( path, metaData );
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CFileSystemArchive::GetFileModificationTime( const CString& filePath ) const
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    if ( CORE::FileExists( path ) )
    {
        return CORE::GetFileModificationTime( path );
    }
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return m_writable;
}

/*-------------------------------------------------------------------------*/

const CString& 
CFileSystemArchive::GetType( void ) const
{GUCEF_TRACE;

    return CVFS::FileSystemArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr
CFileSystemArchive::LoadFromDisk( const CString& filePath                 ,
                                  const char* mode                        ,
                                  const CORE::CResourceMetaData* metaData ,
                                  const UInt32 memLoadSize                ,
                                  const bool overwrite                    )
{GUCEF_TRACE;

    // Create a file path for this root
    CString path = CORE::CombinePath( m_rootDir, filePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );

    bool isAppend( ( strchr( mode, 'a' ) != GUCEF_NULL ) || ( strchr( mode, '+' ) != GUCEF_NULL ) );
    bool needwriteable( ( strchr( mode, 'a' ) != GUCEF_NULL ) || ( strchr( mode, 'w' ) != GUCEF_NULL ) || ( strchr( mode, '+' ) != GUCEF_NULL ) );

    if ( GUCEF_NULL == metaData )
    {
        // Check if we can perform read-only access which allows us
        // to share the resource
        if ( ( strcmp( mode, "rb" ) == 0 ) ||
             ( strcmp( mode, "r" ) == 0 )   )
        {
            // Check our cache for this file
            TFileMemCache::iterator n = m_diskCacheList.find( path );
            if ( n != m_diskCacheList.end() )
            {
                // We found the file in our cache, we will link to the existing buffer.
                TDynamicBufferPtr bufferPtr = (*n).second;
                CORE::DynamicBufferAccessPtr bufferAccess( GUCEF_NEW CORE::CDynamicBufferAccess( bufferPtr.GetPointer(), false ) );
                TVfsResourcePtr vfsResource( GUCEF_NEW CVFSHandle( bufferAccess ,
                                                                   path         ,
                                                                   filePath     ,
                                                                   bufferPtr    ) );
                return vfsResource;
            }
        }
    }

    // Check if we can proceed under these circumstances
    bool exists( CORE::FileExists( path ) );
    if ( !exists && needwriteable )
    {
        CORE::CString dirPath = CORE::StripFilename( path );
        if ( !CORE::CreateDirs( dirPath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive:LoadFromDisk: Unable to create directories to store new file: " + dirPath );
            return TBasicVfsResourcePtr();
        }
    }
    if ( ( exists && overwrite && needwriteable ) ||
         ( !exists && needwriteable )             ||
         ( exists && !needwriteable )             ||
         ( exists && isAppend )                    )
    {
        // Attempt to get access to the file
        CORE::FileAccessPtr fileAccess( GUCEF_NEW CORE::CFileAccess() );
        if ( fileAccess.IsNULL() )
        {
            // memory issue
            return TBasicVfsResourcePtr();
        }

        if ( GUCEF_NULL == metaData )
        {
            if ( !fileAccess->Open( path, mode ) )
            {
                // try a different root
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive:LoadFromDisk: Failed to open access to file: " + path );
                fileAccess.Unlink();
                return TBasicVfsResourcePtr();
            }
        }
        else
        {
            if ( !fileAccess->Open( path, *metaData, mode ) )
            {
                // try a different root
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive:LoadFromDisk: Failed to open access to file: " + path );
                fileAccess.Unlink();
                return TBasicVfsResourcePtr();
            }
        }

        if ( ( strcmp( mode, "rb" ) == 0 ) ||
             ( strcmp( mode, "r" ) == 0 )   )
        {
            // check if we can load the file into memory
            UInt64 fsize = CORE::FileSize( path );
            if ( fsize <= memLoadSize )
            {
                // Create the memory buffer
                TDynamicBufferPtr bufferPtr( GUCEF_NEW CORE::CDynamicBuffer() );
                CORE::DynamicBufferAccessPtr bufferAccess( GUCEF_NEW CORE::CDynamicBufferAccess( bufferPtr.GetPointer(), false ) );

                // Copy the file into the buffer
                if ( fsize == bufferAccess->Write( *(fileAccess.GetPointerAlways()) ) )
                {
                    fileAccess.Unlink();

                    // reset the carat so the user can access the file from the beginning
                    bufferAccess->Setpos( 0 );

                    // Add the file to our buffered files list
                    m_diskCacheList.insert( std::pair< CORE::CString, TDynamicBufferPtr >( path, bufferPtr ) );

                    // return the file handle
                    return TVfsResourcePtr( GUCEF_NEW CVFSHandle( bufferAccess ,
                                                                  path         ,
                                                                  filePath     ,
                                                                  bufferPtr    ) );
                }
                else
                {
                    // Something went wrong while trying to load the file into the buffer
                    // try a different root
                    fileAccess->Setpos( 0 );
                    bufferAccess.Unlink();
                }
            }
        }

        // return the file handle
        return TVfsResourcePtr( GUCEF_NEW CVFSHandle( fileAccess ,
                                                      path       ,
                                                      filePath   ) );

    }

    return TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchive::DestroyObject( CVFSHandle* vfshandle ) const
{GUCEF_TRACE;

    if ( vfshandle != GUCEF_NULL )
    {
        vfshandle->GetAccess().Unlink();

        if ( vfshandle->IsLoadedInMemory() )
        {
            // This operation is logically const as not altering the archive
            // the cache is a transparent bolt-on
            CFileSystemArchive* mutableThis = const_cast< CFileSystemArchive* >( this );
            
            TFileMemCache::iterator n = mutableThis->m_diskCacheList.find( vfshandle->GetFilePath() );
            if ( n != mutableThis->m_diskCacheList.end() )
            {
                // We found the file in our cache, we will link to the existing buffer.
                if ( (*n).second.GetReferenceCount() == 1 )
                {
                    // nobody else is using the memory buffer anymore
                    mutableThis->m_diskCacheList.erase( n );
                }
            }
        }

        GUCEF_DELETE vfshandle;
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemArchive::GetRootDir( void ) const
{GUCEF_TRACE;

    return m_rootDir;
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::AddDirToWatch( const CString& dirToWatch       ,
                                   const CDirWatchOptions& options ,
                                   bool tryToCreatePathIfNotExists )
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, dirToWatch.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return m_fsWatcher.AddDirToWatch( path, options, tryToCreatePathIfNotExists );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::RemoveDirToWatch( const CString& dirToWatch )
{GUCEF_TRACE;

    CString path = CORE::CombinePath( m_rootDir, dirToWatch.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return m_fsWatcher.RemoveDirToWatch( path );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::RemoveAllWatches( void )
{GUCEF_TRACE;

    return m_fsWatcher.RemoveAllWatches();
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::GetAllWatchedDirs( CString::StringSet& dirs ) const
{GUCEF_TRACE;

    CString::StringSet internalDirs;
    if ( m_fsWatcher.GetAllWatchedDirs( internalDirs ) )
    {
        CString::StringSet::iterator i = internalDirs.begin();
        while ( i != internalDirs.end() )
        {
            const CORE::CString& internalDir = (*i);
            CORE::CString archiveDir = internalDir.CutChars( m_rootDir.Length(), true, 0 );
            dirs.insert( archiveDir );
            
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::IsDirectoryWatchingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString 
CFileSystemArchive::GetVfsPathForFileSystemPath( const CORE::CString& fsPath ) const
{GUCEF_TRACE;

    // Check to see if the given path is a sub-path of the root dir
    // Localize the path to the archive if so and return it
    if ( 0 == fsPath.HasSubstr( m_rootDir, true ) ) 
        return fsPath.CutChars( m_rootDir.Length(), true, 0 );
    
    // No match, the archive is not responsible for this path
    // As such we kick this problem to the overall VFS for resolution
    CORE::CString vfsPath;
    if ( CVfsGlobal::Instance()->GetVfs().TryGetVfsPathForFileSystemPath( fsPath, vfsPath ) )
        return vfsPath;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchive::OnDirectoryWatcherEvent( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventid  ,
                                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // Archives are also DirectoryWatcher's so we pretend the event msg came from us
    // we do need to translate the paths first

    if ( IsDirectoryWatcherDirEvent( eventid ) )
    {
        if ( eventid == CORE::CDirectoryWatcherEvents::DirRenamedEvent )
        {
            CORE::CDirectoryWatcherEvents::TDirRenamedEventData* dirRenameInfo = static_cast< CORE::CDirectoryWatcherEvents::TDirRenamedEventData* >( eventdata );
            if ( GUCEF_NULL != dirRenameInfo )
            {
                CORE::CDirectoryWatcherEvents::TDirRenamedEventData vfsAdjusted;
                vfsAdjusted.GetData().newDirName = GetVfsPathForFileSystemPath( dirRenameInfo->GetData().newDirName );
                vfsAdjusted.GetData().oldDirName = GetVfsPathForFileSystemPath( dirRenameInfo->GetData().oldDirName );
                NotifyObservers( eventid, &vfsAdjusted );
            }              
        }
        else
        {
            CORE::TCloneableString* dirPath = static_cast< CORE::TCloneableString* >( eventdata );
            if ( GUCEF_NULL != dirPath )
            {
                CORE::TCloneableString vfsPath( GetVfsPathForFileSystemPath( *dirPath ) );
                NotifyObservers( eventid, &vfsPath );
            }
        }
    }
    else
    if ( IsDirectoryWatcherFileEvent( eventid ) )
    {
        if ( eventid == CORE::CDirectoryWatcherEvents::FileRenamedEvent )
        {
            CORE::CDirectoryWatcherEvents::TFileRenamedEventData* fileRenameInfo = static_cast< CORE::CDirectoryWatcherEvents::TFileRenamedEventData* >( eventdata );
            if ( GUCEF_NULL != fileRenameInfo )
            {
                CORE::CDirectoryWatcherEvents::TFileRenamedEventData vfsAdjusted;
                vfsAdjusted.GetData().newFilename = GetVfsPathForFileSystemPath( fileRenameInfo->GetData().newFilename );
                vfsAdjusted.GetData().oldFilename = GetVfsPathForFileSystemPath( fileRenameInfo->GetData().oldFilename );
                NotifyObservers( eventid, &vfsAdjusted );
            }
        }
        else
        {
            CORE::TCloneableString* filePath = static_cast< CORE::TCloneableString* >( eventdata );
            if ( GUCEF_NULL != filePath )
            {
                CORE::TCloneableString vfsPath( GetVfsPathForFileSystemPath( *filePath ) );
                NotifyObservers( eventid, &vfsPath );
            }                
        }
    }
    else
    {
        // adminstrative event
        NotifyObservers( eventid );
    }
}

/*-------------------------------------------------------------------------*/

void 
CFileSystemArchive::OnNotify( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    // For anything from the file system directory watcher we are merely
    // acting as a proxy to abstracting its reuse
    if ( notifier == &m_fsWatcher )
    {
        // Archives are also DirectoryWatcher's so we pretend the event msg came from us
        // we do need to translate the paths first
        OnDirectoryWatcherEvent( notifier, eventId, eventdata );
    }
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::IsHealthy( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::IsConnected( void ) const
{GUCEF_TRACE;

    // try to interact with the file system and verify the root path exists
    return CORE::DirExists( m_rootDir );
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemArchive::TryResolveSpecialDir( CORE::TSpecialDirs dir, CString& resolvedPath ) const
{GUCEF_TRACE;

    CString fsPath;
    if ( CORE::TryResolveSpecialDir( dir, fsPath ) )
    {
        resolvedPath = GetVfsPathForFileSystemPath( fsPath );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::CreateDir( const CString& dirPath )
{GUCEF_TRACE;

    if ( !m_writable )
        return false;
    CString path = CORE::CombinePath( m_rootDir, dirPath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    return CORE::CreateDirs( path ) && CORE::DirExists( path );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::IsWithinArchive( const CString& archivePath )
{GUCEF_TRACE;

    CString::StringVector segments = archivePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ).ParseElements( GUCEF_DIRSEPCHAR, false );
    for ( CString::StringVector::const_iterator i = segments.begin(); i != segments.end(); ++i )
    {
        if ( ".." == (*i) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchive: The path " << archivePath << " leaves the archive" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::DeleteDir( const CString& dirPath, bool deleteContents )
{GUCEF_TRACE;

    if ( !m_writable || !IsWithinArchive( dirPath ) )
        return false;
    CString path = CORE::CombinePath( m_rootDir, dirPath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    if ( !CORE::DirExists( path ) )
        return false;
    return CORE::DeleteDir( path, deleteContents ) && !CORE::DirExists( path );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchive::MoveDir( const CString& oldDirPath, const CString& newDirPath )
{GUCEF_TRACE;

    if ( !m_writable )
        return false;
    CString oldPath = CORE::CombinePath( m_rootDir, oldDirPath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    CString newPath = CORE::CombinePath( m_rootDir, newDirPath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR ) );
    if ( !CORE::DirExists( oldPath ) || CORE::DirExists( newPath ) || CORE::FileExists( newPath ) )
        return false;

    // the rename of MoveFile() also moves a directory
    return CORE::MoveFile( oldPath, newPath, false ) && CORE::DirExists( newPath );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
