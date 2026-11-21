/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestRemoteArchiveFixture.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteTestEnv::CRemoteTestEnv( const char* name )
    : CRemoteEnv()
    , m_name( name )
    , m_options()
    , m_folder( GUCEF_NULL )
    , m_backend()
    , m_cache()
    , m_scheduler( GUCEF_NULL )
    , m_factory()
    , m_events()
    , m_factoryRegistered( false )
    , m_setUp( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteTestEnv::~CRemoteTestEnv()
{GUCEF_TRACE;

    Teardown();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteTestEnv::OpenCache( void )
{GUCEF_TRACE;

    // the place that was registered holds the cache too: the folder is closed when the last holder lets go
    CTestRemoteArchive::UnregisterPlace( m_name );
    m_cache.Unlink();
    if ( !m_options.useCache )
        return true;

    VFS::CVfsBlockCachePtr cache( GUCEF_NEW VFS::CVfsBlockCache() );
    if ( !cache->Open( m_folder->GetPath(), m_options.cacheMaxBytes, m_options.blockBytes, 64 ) )
        return false;
    m_cache = cache;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteTestEnv::RegisterPlaceNow( void )
{GUCEF_TRACE;

    CTestRemoteArchive::SPlace place;
    place.backend = m_backend;
    place.cache = m_cache;
    place.scheduler = m_scheduler;
    place.settings.cacheIdentity = "remote-test-place";
    place.settings.metadataTtlMs = m_options.metadataTtlMs;
    place.settings.maxMetadataEntries = m_options.maxMetadataEntries;
    place.settings.readAheadAfterBlocks = m_options.readAheadAfterBlocks;
    place.settings.readAheadBlocks = m_options.readAheadBlocks;
    place.settings.maxBlocksPerFetch = m_options.maxBlocksPerFetch;
    place.settings.offlineListings = m_options.offlineListings;
    place.settings.verifyCompleteFiles = m_options.verifyCompleteFiles;
    place.settings.refetchPinned = m_options.refetchPinned;
    CTestRemoteArchive::RegisterPlace( m_name, place );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteTestEnv::MountArchive( void )
{GUCEF_TRACE;

    RegisterPlaceNow();

    VFS::CArchiveSettings settings;
    settings.SetArchiveName( ArchiveName );
    settings.SetArchiveType( CTestRemoteArchive::TypeName );
    settings.SetArchivePath( m_name );
    settings.SetActualArchivePath( m_name );
    settings.SetMountPath( MountPath );
    settings.SetWriteableRequested( false );
    if ( !Vfs().MountArchive( settings ) )
        return false;

    m_events.Listen( &Vfs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteTestEnv::Setup( const SRemoteEnvOptions& options )
{GUCEF_TRACE;

    Teardown();
    m_options = options;
    m_folder = GUCEF_NEW CBlockCacheTestFolder( m_name.C_String() );

    m_backend = CTestRemoteBackendPtr( GUCEF_NEW CTestRemoteBackend() );
    m_backend->AddFile( ReadmeFile, 1, 1000 );
    m_backend->AddFile( NotesFile, 2, 70000 );
    m_backend->AddFile( BigFile, 3, 1000000 );
    m_backend->AddFile( MidFile, 4, 300000 );
    m_backend->AddFile( EmptyFile, 5, 0 );
    m_backend->AddFile( TopFile, 6, 100 );
    m_backend->AddFile( DeepFile, 7, 5000 );

    if ( !OpenCache() )
        return false;

    m_scheduler = GUCEF_NEW VFS::CVfsFetchScheduler( "VfsRemoteOpsTest" );
    m_scheduler->SetMaxRunning( options.maxRunning );
    m_scheduler->SetMaxQueued( options.maxQueued );
    m_scheduler->SetCancelWaitMs( 10000 );

    Vfs().RegisterArchiveFactory( CTestRemoteArchive::TypeName, m_factory );
    m_factoryRegistered = true;

    if ( !MountArchive() )
        return false;
    m_events.Clear();
    m_setUp = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteTestEnv::Unmount( void )
{GUCEF_TRACE;

    return Vfs().UnmountArchiveByName( ArchiveName );
}

/*-------------------------------------------------------------------------*/

void
CRemoteTestEnv::CloseCache( void )
{GUCEF_TRACE;

    CTestRemoteArchive::UnregisterPlace( m_name );
    m_cache.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteTestEnv::Restart( bool reopenCache )
{GUCEF_TRACE;

    Unmount();
    if ( reopenCache )
    {
        // the process ends: the cache of the run is closed and a new one reads the folder
        if ( !OpenCache() )
            return false;
    }
    return MountArchive();
}

/*-------------------------------------------------------------------------*/

void
CRemoteTestEnv::Teardown( void )
{GUCEF_TRACE;

    if ( !m_scheduler && !m_factoryRegistered && !m_folder )
        return;

    if ( VFS::CVfsGlobal::Instance() )
        Vfs().UnmountArchiveByName( ArchiveName );
    if ( m_factoryRegistered )
    {
        Vfs().UnregisterArchiveFactory( CTestRemoteArchive::TypeName );
        m_factoryRegistered = false;
    }
    CTestRemoteArchive::UnregisterPlace( m_name );
    m_events.UnsubscribeAllFromObserver();
    m_events.Clear();

    // the scheduler first: nothing of a job runs when the cache and the place go
    GUCEF_DELETE m_scheduler;
    m_scheduler = GUCEF_NULL;
    m_cache.Unlink();
    m_backend.Unlink();
    GUCEF_DELETE m_folder;
    m_folder = GUCEF_NULL;
    m_setUp = false;
}

/*-------------------------------------------------------------------------*/

CTestRemoteBackend&
CRemoteTestEnv::Backend( void )
{GUCEF_TRACE;

    return *m_backend.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsRemoteArchiveCore*
CRemoteTestEnv::Core( void )
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive = Vfs().GetMountedArchive( ArchiveName );
    if ( archive.IsNULL() )
        return GUCEF_NULL;
    return static_cast< CTestRemoteArchive* >( archive.GetPointerAlways() )->GetCore().GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsBlockCachePtr
CRemoteTestEnv::Cache( void )
{GUCEF_TRACE;

    return m_cache;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsFetchScheduler&
CRemoteTestEnv::Scheduler( void )
{GUCEF_TRACE;

    return *m_scheduler;
}

/*-------------------------------------------------------------------------*/

CTestEventRecorder&
CRemoteTestEnv::Events( void )
{GUCEF_TRACE;

    return m_events;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteTestEnv::GetCacheFolder( void ) const
{GUCEF_TRACE;

    return m_folder->GetPath();
}

/*-------------------------------------------------------------------------*/

const SRemoteEnvOptions&
CRemoteTestEnv::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

CRemoteArchiveConformanceFixture::CRemoteArchiveConformanceFixture( void )
    : CArchiveConformanceFixture()
    , m_env( GUCEF_NULL )
    , m_tree( false )
    , m_capabilities()
    , m_archive( GUCEF_NULL )
    , m_mutated()
{GUCEF_TRACE;

    m_capabilities.isWritable = false;
    m_capabilities.supportsDirectoryWatching = false;
    m_capabilities.hasModificationTimes = true;
    m_capabilities.modificationTimeResolutionInMs = 1000;
    m_capabilities.revisionIsWeak = false;
    m_capabilities.canMutateTree = true;
    m_capabilities.canProvideOutsidePath = false;
    m_capabilities.supportsFileHash = false;
    m_capabilities.fileHashIsMd5 = false;
    m_capabilities.digestAlgorithm = VFS::CVfsDigest::ALGORITHM_UNKNOWN;
    m_capabilities.supportsFileMetaData = true;
    m_capabilities.honoursMaxListEntries = true;
    m_capabilities.dirListHonoursIncludePath = true;
    m_capabilities.dirListRecursesIntoSubDirs = true;
    m_capabilities.dirEntriesHaveTrailingSeparator = true;
    m_capabilities.confinesPathsToArchive = true;
    m_capabilities.readHandlesRefuseWrites = true;
    m_capabilities.supportsLargeFiles = false;
    m_capabilities.storesEmptyFiles = false;
    m_capabilities.supportsPartialWrites = false;
    m_capabilities.supportsAtomicMove = false;
    m_capabilities.supportsDirOperations = false;
    m_capabilities.supportsConcurrentCallers = true;
}

/*-------------------------------------------------------------------------*/

CRemoteArchiveConformanceFixture::~CRemoteArchiveConformanceFixture()
{GUCEF_TRACE;

    Teardown();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteArchiveConformanceFixture::GetName( void ) const
{GUCEF_TRACE;

    return "TestRemote";
}

/*-------------------------------------------------------------------------*/

bool
CRemoteArchiveConformanceFixture::Setup( void )
{GUCEF_TRACE;

    Teardown();
    m_env = GUCEF_NEW CRemoteTestEnv( "gucefvfs-remoteconf" );
    SRemoteEnvOptions options;
    options.metadataTtlMs = 0;
    if ( !m_env->Setup( options ) )
        return false;

    // the reference tree replaces the small tree of the environment
    CTestRemoteBackend& backend = m_env->Backend();
    backend.Clear();
    const CArchiveConformanceTree::TEntryVector& entries = m_tree.GetEntries();
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const CArchiveConformanceTree::SEntry& entry = entries[ i ];
        if ( entry.isDirectory )
            backend.AddDirectory( entry.path );
        else
            backend.AddFile( entry.path, entry.contentId, entry.size );
    }
    m_env->Vfs().InvalidateRemoteMetadata();
    m_archive = m_env->Vfs().GetMountedArchive( CRemoteTestEnv::ArchiveName ).GetPointerAlways();
    return GUCEF_NULL != m_archive;
}

/*-------------------------------------------------------------------------*/

void
CRemoteArchiveConformanceFixture::Teardown( void )
{GUCEF_TRACE;

    m_archive = GUCEF_NULL;
    GUCEF_DELETE m_env;
    m_env = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

VFS::CArchive*
CRemoteArchiveConformanceFixture::GetArchive( void )
{GUCEF_TRACE;

    return m_archive;
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceTree&
CRemoteArchiveConformanceFixture::GetTree( void ) const
{GUCEF_TRACE;

    return m_tree;
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceCapabilities&
CRemoteArchiveConformanceFixture::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteArchiveConformanceFixture::PutFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_env )
        return false;
    m_env->Backend().PutFile( path, content );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteArchiveConformanceFixture::RemoveFile( const CORE::CString& path )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_env )
        return false;
    m_env->Backend().RemoveFile( path );
    return true;
}

/*-------------------------------------------------------------------------*/
