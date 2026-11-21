/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_VFS_CFILESYSTEMARCHIVE_H
#include "gucefVFS_CFileSystemArchive.h"
#define GUCEF_VFS_CFILESYSTEMARCHIVE_H
#endif /* GUCEF_VFS_CFILESYSTEMARCHIVE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "TestArchiveConformance.h"

#include "TestFileSystemArchiveConformance.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemArchiveConformanceFixture::CFileSystemArchiveConformanceFixture( bool writable )
    : CArchiveConformanceFixture()
    , m_writable( writable )
    , m_scratchDir()
    , m_rootDir()
    , m_tree( true )
    , m_capabilities()
    , m_archive( GUCEF_NULL )
{GUCEF_TRACE;

    m_capabilities.isWritable = writable;
    m_capabilities.supportsDirectoryWatching = true;
    m_capabilities.hasModificationTimes = true;
    m_capabilities.modificationTimeResolutionInMs = 1000;
    m_capabilities.revisionIsWeak = false;
    m_capabilities.canMutateTree = true;
    m_capabilities.canProvideOutsidePath = true;
    m_capabilities.supportsFileHash = true;
    m_capabilities.fileHashIsMd5 = true;
    m_capabilities.digestAlgorithm = GUCEF::VFS::CVfsDigest::ALGORITHM_MD5;
    m_capabilities.supportsFileMetaData = true;
    m_capabilities.honoursMaxListEntries = true;
    m_capabilities.dirListHonoursIncludePath = true;
    m_capabilities.dirListRecursesIntoSubDirs = true;
    m_capabilities.dirEntriesHaveTrailingSeparator = true;
    m_capabilities.confinesPathsToArchive = false;
    m_capabilities.readHandlesRefuseWrites = true;
    m_capabilities.supportsLargeFiles = true;
    m_capabilities.storesEmptyFiles = false;
    m_capabilities.supportsPartialWrites = true;
    m_capabilities.supportsAtomicMove = true;
    m_capabilities.supportsDirOperations = true;
    m_capabilities.supportsConcurrentCallers = true;
}

/*-------------------------------------------------------------------------*/

CFileSystemArchiveConformanceFixture::~CFileSystemArchiveConformanceFixture()
{GUCEF_TRACE;

    Teardown();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFileSystemArchiveConformanceFixture::GetName( void ) const
{GUCEF_TRACE;

    return m_writable ? "FileSystem" : "FileSystem (read only)";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileSystemArchiveConformanceFixture::GetRootDir( void ) const
{GUCEF_TRACE;

    return m_rootDir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFileSystemArchiveConformanceFixture::GetFileSystemPath( const CORE::CString& archivePath ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_rootDir, archivePath );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchiveConformanceFixture::WriteTreeEntry( const CArchiveConformanceTree::SEntry& entry ) const
{GUCEF_TRACE;

    const CORE::CString path = GetFileSystemPath( entry.path );
    if ( entry.isDirectory )
        return CORE::CreateDirs( path );

    if ( !CORE::CreatePathDirectories( path ) )
        return false;

    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;

    // the file above 4 GiB is zero filled, which the file system keeps as a hole: it is made by setting its size
    if ( entry.isZeroFilled )
    {
        file.Close();
        return CORE::TruncateFile( path, entry.size ) && CORE::FileSize( path ) == entry.size;
    }

    CORE::CDynamicBuffer chunk( 65536, true );
    UInt64 position = 0;
    while ( position < entry.size )
    {
        const UInt32 count = (UInt32) ( entry.size - position < 65536 ? entry.size - position : 65536 );
        CArchiveConformanceTree::FillEntryContent( entry, position, chunk.GetBufferPtr(), count );
        if ( count != file.Write( chunk.GetConstBufferPtr(), 1, count ) )
            return false;
        position += count;
    }
    file.Flush();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchiveConformanceFixture::Setup( void )
{GUCEF_TRACE;

    Teardown();

    CORE::CString base = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
    if ( base.IsNULLOrEmpty() )
        base = CORE::TempDir();

    // the archive is a directory below the scratch directory: the sentinel file beside it is outside the archive
    m_scratchDir = CORE::CombinePath( base, "gucefvfs-archconf-" + CORE::ToString( MT::GetProcessID() ) + "-" + CORE::ToString( (UInt32) MT::PrecisionTickCount() ) );
    m_rootDir = CORE::CombinePath( m_scratchDir, "archive" );
    if ( !CORE::CreateDirs( m_rootDir ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchiveConformance: Failed to make the directory " << m_rootDir );
        return false;
    }

    const CArchiveConformanceTree::TEntryVector& entries = m_tree.GetEntries();
    for ( CArchiveConformanceTree::TEntryVector::const_iterator i = entries.begin(); i != entries.end(); ++i )
    {
        if ( !WriteTreeEntry( (*i) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemArchiveConformance: Failed to write " << (*i).path << " below " << m_rootDir );
            return false;
        }
    }

    CORE::CFileAccess sentinel( CORE::CombinePath( m_scratchDir, "outside_sentinel.txt" ), "wb" );
    if ( !sentinel.Opened() || 7 != sentinel.Write( "outside", 1, 7 ) )
        return false;
    sentinel.Close();

    VFS::CFileSystemArchive* archive = GUCEF_NEW VFS::CFileSystemArchive();
    VFS::CArchiveSettings settings;
    settings.SetArchiveName( "ArchiveConformance" );
    settings.SetActualArchivePath( m_rootDir );
    settings.SetArchivePath( m_rootDir );
    settings.SetWriteableRequested( m_writable );
    if ( !archive->LoadArchive( settings ) )
    {
        GUCEF_DELETE archive;
        return false;
    }
    m_archive = archive;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemArchiveConformanceFixture::Teardown( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archive )
    {
        GUCEF_DELETE m_archive;
        m_archive = GUCEF_NULL;
    }
    if ( !m_scratchDir.IsNULLOrEmpty() && m_scratchDir.HasSubstr( "gucefvfs-archconf-" ) >= 0 )
        CORE::DeleteDir( m_scratchDir, true );
    m_scratchDir.Clear();
    m_rootDir.Clear();
}

/*-------------------------------------------------------------------------*/

VFS::CArchive*
CFileSystemArchiveConformanceFixture::GetArchive( void )
{GUCEF_TRACE;

    return m_archive;
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceTree&
CFileSystemArchiveConformanceFixture::GetTree( void ) const
{GUCEF_TRACE;

    return m_tree;
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceCapabilities&
CFileSystemArchiveConformanceFixture::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchiveConformanceFixture::PutFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    if ( m_rootDir.IsNULLOrEmpty() )
        return false;

    const CORE::CString fsPath = GetFileSystemPath( path );
    if ( !CORE::CreatePathDirectories( fsPath ) )
        return false;

    CORE::CFileAccess file( fsPath, "wb" );
    if ( !file.Opened() )
        return false;
    if ( content.GetDataSize() > 0 && content.GetDataSize() != file.Write( content.GetConstBufferPtr(), 1, content.GetDataSize() ) )
        return false;
    file.Flush();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchiveConformanceFixture::RemoveFile( const CORE::CString& path )
{GUCEF_TRACE;

    if ( m_rootDir.IsNULLOrEmpty() )
        return false;

    const CORE::CString fsPath = GetFileSystemPath( path );
    return !CORE::FileExists( fsPath ) || CORE::DeleteFile( fsPath );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemArchiveConformanceFixture::GetPathOutsideArchive( CORE::CString& path )
{GUCEF_TRACE;

    if ( m_scratchDir.IsNULLOrEmpty() )
        return false;
    path = "../outside_sentinel.txt";
    return CORE::FileExists( CORE::CombinePath( m_scratchDir, "outside_sentinel.txt" ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFileSystemArchiveConformanceTests( void )
{GUCEF_TRACE;

    {
        CFileSystemArchiveConformanceFixture writable( true );
        PerformArchiveConformanceSuite( writable );
    }
    {
        CFileSystemArchiveConformanceFixture readOnly( false );
        PerformArchiveConformanceSuite( readOnly );
    }
}

/*-------------------------------------------------------------------------*/
