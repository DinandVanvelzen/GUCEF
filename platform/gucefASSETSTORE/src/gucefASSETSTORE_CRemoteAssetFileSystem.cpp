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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefASSETSTORE_CRemoteFileTransfer.h"
#include "gucefASSETSTORE_CRemoteFileTransferTaskData.h"
#include "gucefASSETSTORE_CRemoteFileTransferTaskConsumer.h"
#include "gucefASSETSTORE_CRemoteAssetArchive.h"

#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteAssetFileSystem::FileArrivedEvent = "GUCEF::ASSETSTORE::CRemoteAssetFileSystem::FileArrivedEvent";
const CORE::CEvent CRemoteAssetFileSystem::ListingArrivedEvent = "GUCEF::ASSETSTORE::CRemoteAssetFileSystem::ListingArrivedEvent";
const CORE::CEvent CRemoteAssetFileSystem::StateChangedEvent = "GUCEF::ASSETSTORE::CRemoteAssetFileSystem::StateChangedEvent";
const CORE::CString CRemoteAssetFileSystem::ArchiveTypeName = "assetstore";

namespace {

typedef CORE::CTFactory< VFS::CArchive, CRemoteAssetArchive, MT::CMutex > TRemoteArchiveFactory;

static MT::CMutex s_registryLock;
static GUCEF::vector< CRemoteAssetFileSystem* > s_instances;
static TRemoteArchiveFactory* s_archiveFactory = GUCEF_NULL;
static UInt32 s_nextId = 0;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    return resolution > 0 ? ( MT::PrecisionTickCount() * 1000ULL ) / resolution : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
LeafOf( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? path : path.CutChars( (UInt32) slash + 1, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ParentOf( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? CORE::CString::Empty : path.SubstrFromRange( 0, (UInt32) slash );
}

/*-------------------------------------------------------------------------*/

bool
IsNetworkStatus( TAssetStoreStatus status )
{
    return ASSETSTORE_STATUS_UNAVAILABLE == status || ASSETSTORE_STATUS_NETWORK_ERROR == status || ASSETSTORE_STATUS_TIMED_OUT == status;
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SFileInfo::SFileInfo( void )
    : exists( false ), isDirectory( false ), size( 0 ), digest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SOpenFile::SOpenFile( void )
    : path(), size( 0 ), digest(), entry( 0 ), nextSequential( 0 ), sequentialBlocks( 0 )
    , windowStart( 0 ), window(), cancelled( false ), status( ASSETSTORE_STATUS_OK ), message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SStatistics::SStatistics( void )
    : bytesFetched( 0 ), fetchCalls( 0 ), readAheadBlocks( 0 ), listCalls( 0 ), missesOfNoWaitThread( 0 ), digestMismatches( 0 ), filesVerified( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SMeta::SMeta( void )
    : size( 0 ), digest(), kind( 0 ), validUntilMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SListing::SListing( void )
    : dirs(), files(), truncated( false ), loadedMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SAsyncList::SAsyncList( void )
    : folder(), future( CORE::TASKSTATUS_UNDEFINED ), listing(), entries( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::SAsyncRun::SAsyncRun( void )
    : path(), size( 0 ), digest(), entry( 0 ), future( CORE::TASKSTATUS_UNDEFINED ), firstBlock( 0 ), blockCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::RegisterEvents( void )
{GUCEF_TRACE;

    FileArrivedEvent.Initialize();
    ListingArrivedEvent.Initialize();
    StateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::CRemoteAssetFileSystem( CIAssetStoreService& service, const CRemoteFilesSettings& settings )
    : CORE::CObservingNotifier()
    , m_service( service )
    , m_settings( settings )
    , m_cache()
    , m_id()
    , m_open( false )
    , m_cacheOpen( false )
    , m_mounts()
    , m_noWaitThread( 0 )
    , m_meta()
    , m_listings()
    , m_asyncLists()
    , m_asyncRuns()
    , m_missed()
    , m_arrived()
    , m_verified()
    , m_digests()
    , m_transfers()
    , m_offline( false )
    , m_offlineChanged( false )
    , m_lock()
    , m_bytesFetched( 0 )
    , m_fetchCalls( 0 )
    , m_readAheadBlocks( 0 )
    , m_listCalls( 0 )
    , m_noWaitMisses( 0 )
    , m_digestMismatches( 0 )
    , m_filesVerified( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    MT::CScopeMutex lock( s_registryLock );
    m_id = "rfs" + CORE::ToString( ++s_nextId );
}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem::~CRemoteAssetFileSystem()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::Open( void )
{GUCEF_TRACE;

    if ( m_open )
        return true;
    if ( !m_settings.cacheFolder.IsNULLOrEmpty() )
        m_cacheOpen = m_cache.Open( m_settings.cacheFolder, m_settings.maxCacheBytes );
    else
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetFileSystem: No cache folder is set: the files of the service are read without a cache" );

    {
        MT::CScopeMutex lock( s_registryLock );
        if ( GUCEF_NULL == s_archiveFactory )
        {
            s_archiveFactory = GUCEF_NEW TRemoteArchiveFactory();
            VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveFactory( ArchiveTypeName, *s_archiveFactory );
        }
        s_instances.push_back( this );
    }
    TEventCallback changedCallback( this, &CRemoteAssetFileSystem::OnCatalogueChanged );
    SubscribeTo( &m_service, CIAssetStoreService::CatalogueChangedEvent, changedCallback );
    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::Close( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    UnsubscribeFrom( &m_service );
    {
        // transfers that run are ended first: they read through this object
        TTransferVector running;
        {
            MT::CScopeMutex lock( m_lock );
            running = m_transfers;
        }
        for ( size_t i=0; i<running.size(); ++i )
            running[ i ]->Cancel();
        for ( UInt32 waited=0; waited<15000 && GetRunningTransferCount() > 0; waited+=10 )
            MT::PrecisionDelay( 10 );
    }
    UnmountAll();
    {
        MT::CScopeMutex lock( s_registryLock );
        for ( size_t i=0; i<s_instances.size(); ++i )
        {
            if ( s_instances[ i ] == this )
            {
                s_instances.erase( s_instances.begin() + i );
                break;
            }
        }
        if ( s_instances.empty() && GUCEF_NULL != s_archiveFactory )
        {
            VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveFactory( ArchiveTypeName );
            GUCEF_DELETE s_archiveFactory;
            s_archiveFactory = GUCEF_NULL;
        }
    }
    // reads in flight end with their entries released
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_asyncRuns.size(); ++i )
            if ( 0 != m_asyncRuns[ i ].entry )
                m_cache.ReleaseEntry( m_asyncRuns[ i ].entry );
        m_asyncRuns.clear();
        m_asyncLists.clear();
    }
    if ( m_cacheOpen )
        m_cache.Close();
    m_cacheOpen = false;
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool CRemoteAssetFileSystem::IsOpen( void ) const { return m_open; }
const CORE::CString& CRemoteAssetFileSystem::GetId( void ) const { return m_id; }
const CRemoteFilesSettings& CRemoteAssetFileSystem::GetSettings( void ) const { return m_settings; }
CIAssetStoreService& CRemoteAssetFileSystem::GetService( void ) const { return m_service; }
CAssetBlockCache& CRemoteAssetFileSystem::GetCache( void ) { return m_cache; }

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetFileSystem::Normalize( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CString path = vfsPath.ReplaceChar( '\\', '/' );
    while ( path.Length() > 0 && '/' == path[ 0 ] )
        path = path.CutChars( 1, true );
    while ( path.Length() > 0 && '/' == path[ path.Length()-1 ] )
        path = path.CutChars( 1, false );
    return path;
}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem*
CRemoteAssetFileSystem::FindById( const CORE::CString& id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( s_registryLock );
    for ( size_t i=0; i<s_instances.size(); ++i )
        if ( s_instances[ i ]->m_id == id )
            return s_instances[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRemoteAssetFileSystem*
CRemoteAssetFileSystem::FindForPath( const CORE::CString& vfsPath, CORE::CString& servicePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( s_registryLock );
    for ( size_t i=0; i<s_instances.size(); ++i )
        if ( s_instances[ i ]->ToServicePath( vfsPath, servicePath ) )
            return s_instances[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::ToServicePath( const CORE::CString& clientPath, CORE::CString& servicePath ) const
{GUCEF_TRACE;

    const CORE::CString path = Normalize( clientPath );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        const CORE::CString& mount = m_mounts[ i ].mountPath;
        if ( path == mount )
        {
            servicePath = m_mounts[ i ].serviceRoot;
            return true;
        }
        if ( path.Length() > mount.Length() && '/' == path[ mount.Length() ] && 0 == path.HasSubstr( mount ) )
        {
            servicePath = m_mounts[ i ].serviceRoot + path.CutChars( mount.Length(), true );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetFileSystem::ToClientPath( const CORE::CString& servicePath ) const
{GUCEF_TRACE;

    const CORE::CString path = Normalize( servicePath );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        const CORE::CString& root = m_mounts[ i ].serviceRoot;
        if ( path == root )
            return m_mounts[ i ].mountPath;
        if ( path.Length() > root.Length() && '/' == path[ root.Length() ] && 0 == path.HasSubstr( root ) )
            return m_mounts[ i ].mountPath + path.CutChars( root.Length(), true );
    }
    return path;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::MountLocation( const CORE::CString& serviceRootText, const CORE::CString& mountPrefix )
{GUCEF_TRACE;

    const CORE::CString serviceRoot = Normalize( serviceRootText );
    if ( serviceRoot.IsNULLOrEmpty() || !m_open )
        return false;
    const CORE::CString prefix = Normalize( mountPrefix );
    const CORE::CString mountPath = prefix.IsNULLOrEmpty() ? serviceRoot : prefix + "/" + serviceRoot;
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_mounts.size(); ++i )
            if ( m_mounts[ i ].mountPath == mountPath )
                return true;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CArchiveSettings settings;
    settings.SetArchiveName( "assetstore:" + m_id + ":" + mountPath );
    settings.SetArchiveType( ArchiveTypeName );
    settings.SetArchivePath( m_id );
    settings.SetActualArchivePath( m_id + "|" + serviceRoot );
    settings.SetMountPath( mountPath );
    settings.SetWriteableRequested( false );
    if ( !vfs.MountArchive( settings ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetFileSystem: The location " << serviceRoot << " of the store service cannot be mounted at " << mountPath );
        return false;
    }
    SMount mount;
    mount.serviceRoot = serviceRoot;
    mount.mountPath = mountPath;
    MT::CScopeMutex lock( m_lock );
    m_mounts.push_back( mount );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetFileSystem: Mounted the location " << serviceRoot << " of the store service at " << mountPath );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetFileSystem::MountLocations( const CORE::CString& mountPrefix )
{GUCEF_TRACE;

    CAssetLocationPage locations;
    if ( ASSETSTORE_STATUS_OK != m_service.GetLocations( locations ) )
        return 0;
    UInt32 mounted = 0;
    for ( size_t i=0; i<locations.entries.size(); ++i )
    {
        if ( !locations.entries[ i ].online || locations.entries[ i ].location.vfsRoot.IsNULLOrEmpty() )
            continue;
        const CORE::CString root = Normalize( locations.entries[ i ].location.vfsRoot );
        if ( IsMounted( root ) )
            continue;
        if ( MountLocation( root, mountPrefix ) )
            ++mounted;
    }
    return mounted;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::UnmountAll( void )
{GUCEF_TRACE;

    TMountVector mounts;
    {
        MT::CScopeMutex lock( m_lock );
        mounts = m_mounts;
        m_mounts.clear();
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t i=mounts.size(); i>0; --i )
        vfs.UnmountArchiveByName( "assetstore:" + m_id + ":" + mounts[ i-1 ].mountPath );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::IsMounted( const CORE::CString& serviceRoot ) const
{GUCEF_TRACE;

    const CORE::CString root = Normalize( serviceRoot );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].serviceRoot == root )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::GetMounts( CORE::CString::StringVector& serviceRoots ) const
{GUCEF_TRACE;

    serviceRoots.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounts.size(); ++i )
        serviceRoots.push_back( m_mounts[ i ].serviceRoot );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::SetNoWaitThread( TThreadId threadId )
{
    m_noWaitThread = threadId;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::IsNoWaitThread( void ) const
{
    return 0 != m_noWaitThread && MT::GetCurrentThreadID() == m_noWaitThread;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a pack came or went, or the session was made again: what is known of the folders may be old
    ForgetMetadata();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::ForgetMetadata( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_meta.clear();
    m_listings.clear();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::LookupMeta( const CORE::CString& path, SMeta& meta ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TMetaMap::const_iterator i = m_meta.find( path );
    if ( i == m_meta.end() )
        return false;
    if ( 0 != i->second.validUntilMs && i->second.validUntilMs < NowMs() )
        return false;
    meta = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::PutMeta( const CORE::CString& path, const SMeta& meta )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_meta.size() >= m_settings.metadataEntries && m_meta.find( path ) == m_meta.end() )
    {
        // bounded: what is known is dropped and learned again from the lists
        m_meta.clear();
        m_listings.clear();
    }
    if ( 1 == meta.kind && meta.digest.IsNULLOrEmpty() )
    {
        // a digest that the catalogue gave before the list of the folder came
        GUCEF::map< CORE::CString, CORE::CString >::iterator d = m_digests.find( path );
        if ( d != m_digests.end() )
        {
            SMeta withDigest( meta );
            withDigest.digest = d->second;
            m_digests.erase( d );
            m_meta[ path ] = withDigest;
            return;
        }
    }
    TMetaMap::iterator i = m_meta.find( path );
    if ( i != m_meta.end() && 1 == i->second.kind && 1 == meta.kind && meta.digest.IsNULLOrEmpty() )
    {
        // a list has no digests: the one that is known stays
        SMeta merged( meta );
        merged.digest = i->second.digest;
        i->second = merged;
        return;
    }
    m_meta[ path ] = meta;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::NoteFile( const CORE::CString& vfsPath, UInt64 size, const CORE::CString& digest )
{GUCEF_TRACE;

    SMeta meta;
    meta.kind = 1;
    meta.size = size;
    meta.digest = digest.Lowercase();
    const CORE::CString path = Normalize( vfsPath );
    PutMeta( path, meta );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::NoteDigest( const CORE::CString& vfsPath, const CORE::CString& digest )
{GUCEF_TRACE;

    const CORE::CString path = Normalize( vfsPath );
    MT::CScopeMutex lock( m_lock );
    TMetaMap::iterator i = m_meta.find( path );
    if ( i != m_meta.end() )
    {
        if ( 1 == i->second.kind )
            i->second.digest = digest.Lowercase();
        return;
    }
    // not listed yet: kept apart until the list of its folder brings the file
    m_digests[ path ] = digest.Lowercase();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::NoteFiles( const GUCEF::vector< CAssetFileRecord >& files )
{GUCEF_TRACE;

    for ( size_t i=0; i<files.size(); ++i )
        if ( files[ i ].sizeBytes >= 0 && "directory" != files[ i ].role )
            NoteFile( files[ i ].path, (UInt64) files[ i ].sizeBytes, files[ i ].digest );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::GetKnownDigest( const CORE::CString& vfsPath, CORE::CString& digest ) const
{GUCEF_TRACE;

    SMeta meta;
    if ( !LookupMeta( Normalize( vfsPath ), meta ) || 1 != meta.kind || meta.digest.IsNULLOrEmpty() )
        return false;
    digest = meta.digest;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::ApplyPage( const CORE::CString& folder, const CAssetFileStatPage& page, SListing& listing, UInt32& entries )
{GUCEF_TRACE;

    for ( size_t i=0; i<page.entries.size(); ++i )
    {
        const CAssetFileRecord& entry = page.entries[ i ];
        const CORE::CString leaf = LeafOf( Normalize( entry.path ) );
        if ( leaf.IsNULLOrEmpty() )
            continue;
        SMeta meta;
        const CORE::CString path = folder.IsNULLOrEmpty() ? leaf : folder + "/" + leaf;
        if ( "directory" == entry.role )
        {
            meta.kind = 2;
            listing.dirs.push_back( leaf );
        }
        else
        {
            meta.kind = 1;
            meta.size = entry.sizeBytes > 0 ? (UInt64) entry.sizeBytes : 0;
            meta.digest = entry.digest.Lowercase();
            listing.files.push_back( leaf );
        }
        PutMeta( path, meta );
        ++entries;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::StoreListing( const CORE::CString& folder, const SListing& listing )
{GUCEF_TRACE;

    SMeta meta;
    meta.kind = 2;
    PutMeta( folder, meta );
    MT::CScopeMutex lock( m_lock );
    SListing stored( listing );
    stored.loadedMs = NowMs();
    m_listings[ folder ] = stored;
    if ( stored.truncated )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetFileSystem: The list of the folder " << folder << " of the store service was cut at " << m_settings.maxListEntries << " entries" );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::LoadListing( const CORE::CString& folder, SListing& listing, bool& exists )
{GUCEF_TRACE;

    exists = true;
    {
        MT::CScopeMutex lock( m_lock );
        TListingMap::const_iterator i = m_listings.find( folder );
        if ( i != m_listings.end() )
        {
            listing = i->second;
            return true;
        }
    }

    SListing build;
    UInt32 entries = 0;
    CORE::CString token;
    do
    {
        CAssetFileStatPage page;
        const TAssetStoreStatus status = m_service.ListDirectory( folder, token, m_settings.listPageEntries, page );
        m_listCalls.FetchAdd( 1 );
        if ( ASSETSTORE_STATUS_NOT_FOUND == status )
        {
            SetOffline( false );
            SMeta meta;
            meta.kind = 3;
            meta.validUntilMs = NowMs() + m_settings.negativeTtlMs;
            PutMeta( folder, meta );
            exists = false;
            return false;
        }
        if ( ASSETSTORE_STATUS_OK != status )
        {
            if ( IsNetworkStatus( status ) )
                SetOffline( true );
            return false;
        }
        SetOffline( false );
        ApplyPage( folder, page, build, entries );
        token = page.nextPageToken;
    }
    while ( !token.IsNULLOrEmpty() && entries < m_settings.maxListEntries );
    build.truncated = !token.IsNULLOrEmpty();
    StoreListing( folder, build );
    listing = build;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::RequestFolder( const CORE::CString& folderText )
{GUCEF_TRACE;

    const CORE::CString folder = Normalize( folderText );
    MT::CScopeMutex lock( m_lock );
    if ( m_listings.find( folder ) != m_listings.end() || m_asyncLists.size() >= 8 )
        return;
    for ( size_t i=0; i<m_asyncLists.size(); ++i )
        if ( m_asyncLists[ i ].folder == folder )
            return;
    SAsyncList list;
    list.folder = folder;
    list.future = m_service.ListDirectoryAsync( folder, CORE::CString::Empty, m_settings.listPageEntries );
    m_asyncLists.push_back( list );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::IsFolderKnown( const CORE::CString& vfsFolder ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_listings.find( Normalize( vfsFolder ) ) != m_listings.end();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::GetFolderEntries( const CORE::CString& vfsFolder, CORE::CString::StringVector& dirs, CORE::CString::StringVector& files, bool* truncated )
{GUCEF_TRACE;

    const CORE::CString folder = Normalize( vfsFolder );
    SListing listing;
    bool exists = true;
    if ( IsNoWaitThread() )
    {
        {
            MT::CScopeMutex lock( m_lock );
            TListingMap::const_iterator i = m_listings.find( folder );
            if ( i != m_listings.end() )
                listing = i->second;
            else
                exists = false;
        }
        if ( !exists )
        {
            m_noWaitMisses.FetchAdd( 1 );
            RequestFolder( folder );
            return false;
        }
    }
    else
    if ( !LoadListing( folder, listing, exists ) )
        return false;
    dirs = listing.dirs;
    files = listing.files;
    if ( GUCEF_NULL != truncated )
        *truncated = listing.truncated;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::GetFileInfo( const CORE::CString& vfsPath, SFileInfo& info )
{GUCEF_TRACE;

    const CORE::CString path = Normalize( vfsPath );
    info = SFileInfo();
    SMeta meta;
    if ( LookupMeta( path, meta ) )
    {
        info.exists = 3 != meta.kind;
        info.isDirectory = 2 == meta.kind;
        info.size = meta.size;
        info.digest = meta.digest;
        return info.exists;
    }
    const CORE::CString parent = ParentOf( path );
    SListing listing;
    bool exists = true;
    if ( IsNoWaitThread() )
    {
        bool known = false;
        {
            MT::CScopeMutex lock( m_lock );
            known = m_listings.find( parent ) != m_listings.end();
        }
        if ( !known )
        {
            m_noWaitMisses.FetchAdd( 1 );
            RequestFolder( parent );
            return false;
        }
    }
    else
    if ( !LoadListing( parent, listing, exists ) )
    {
        if ( !exists )
        {
            SMeta missing;
            missing.kind = 3;
            missing.validUntilMs = NowMs() + m_settings.negativeTtlMs;
            PutMeta( path, missing );
        }
        return false;
    }
    if ( LookupMeta( path, meta ) )
    {
        info.exists = 3 != meta.kind;
        info.isDirectory = 2 == meta.kind;
        info.size = meta.size;
        info.digest = meta.digest;
        return info.exists;
    }
    SMeta missing;
    missing.kind = 3;
    missing.validUntilMs = NowMs() + m_settings.negativeTtlMs;
    PutMeta( path, missing );
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetFileSystem::DescribeFailure( const CORE::CString& vfsPath, TAssetStoreStatus status, const CORE::CString& serviceMessage )
{GUCEF_TRACE;

    switch ( status )
    {
        case ASSETSTORE_STATUS_NOT_FOUND:
            return "The file " + vfsPath + " is not on the store service (any more)";
        case ASSETSTORE_STATUS_REFUSED:
            return "The store service does not give the file " + vfsPath + " to this device" + ( serviceMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + serviceMessage );
        case ASSETSTORE_STATUS_UNAVAILABLE:
        case ASSETSTORE_STATUS_NETWORK_ERROR:
        case ASSETSTORE_STATUS_TIMED_OUT:
            return "The file " + vfsPath + " is not available offline: it is not in the cache of this computer and the store service cannot be reached" +
                   ( serviceMessage.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + serviceMessage + ")" );
        case ASSETSTORE_STATUS_DIGEST_MISMATCH:
            return "The file " + vfsPath + " on the store service differs from what its catalogue says: " + serviceMessage;
        case ASSETSTORE_STATUS_CANCELLED:
            return "The read of the file " + vfsPath + " was cancelled";
        default:
            return "The file " + vfsPath + " cannot be read from the store service" + ( serviceMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + serviceMessage );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::SetOffline( bool offline )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( offline != m_offline )
    {
        m_offline = offline;
        m_offlineChanged = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::IsOffline( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_offline;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::OpenFile( const CORE::CString& vfsPath, SOpenFile& file, CORE::CString& message )
{GUCEF_TRACE;

    file = SOpenFile();
    const CORE::CString path = Normalize( vfsPath );
    SFileInfo info;
    if ( !GetFileInfo( path, info ) || info.isDirectory )
    {
        file.status = ASSETSTORE_STATUS_NOT_FOUND;
        if ( IsNoWaitThread() && !IsOffline() )
            message = "The file " + path + " is not known yet: its folder is being fetched from the store service";
        else
        if ( IsOffline() )
            message = DescribeFailure( path, ASSETSTORE_STATUS_UNAVAILABLE, CORE::CString::Empty );
        else
            message = info.isDirectory ? "The path " + path + " is a folder of the store service" : DescribeFailure( path, ASSETSTORE_STATUS_NOT_FOUND, CORE::CString::Empty );
        file.message = message;
        return false;
    }
    file.path = path;
    file.size = info.size;
    file.digest = info.digest;
    if ( m_cacheOpen && info.size > 0 )
        file.entry = m_cache.AcquireEntry( path, info.size, info.digest );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::CloseFile( SOpenFile& file )
{GUCEF_TRACE;

    if ( 0 != file.entry )
        m_cache.ReleaseEntry( file.entry );
    file.entry = 0;
    file.window.clear();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::FetchRun( SOpenFile& file, UInt32 firstBlock, UInt32 blockCount, GUCEF::vector< UInt8 >& data )
{GUCEF_TRACE;

    const UInt64 start = (UInt64) firstBlock * CAssetBlockCache::BlockBytes;
    if ( start >= file.size )
    {
        file.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        file.message = "The read of " + file.path + " starts behind the end of the file";
        return false;
    }
    UInt64 want = (UInt64) blockCount * CAssetBlockCache::BlockBytes;
    if ( start + want > file.size )
        want = file.size - start;
    CAssetFileBlock block;
    const TAssetStoreStatus status = m_service.ReadFileBlock( file.path, start, (UInt32) want, block );
    m_fetchCalls.FetchAdd( 1 );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        if ( IsNetworkStatus( status ) )
            SetOffline( true );
        file.status = status;
        file.message = DescribeFailure( file.path, status, CORE::CString::Empty );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetFileSystem: " << file.message );
        return false;
    }
    SetOffline( false );
    if ( block.fileSize != file.size )
    {
        // the file on the service is another one than the list said
        MT::CScopeMutex lock( m_lock );
        m_meta.erase( file.path );
        m_listings.erase( ParentOf( file.path ) );
        file.status = ASSETSTORE_STATUS_SIZE_MISMATCH;
        file.message = "The file " + file.path + " changed on the store service while it was read: open it again";
        return false;
    }
    if ( block.data.GetDataSize() != (UInt32) want )
    {
        file.status = ASSETSTORE_STATUS_IO_ERROR;
        file.message = "The store service gave " + CORE::ToString( block.data.GetDataSize() ) + " bytes of " + file.path + " where " + CORE::ToString( (UInt32) want ) + " were asked";
        return false;
    }
    m_bytesFetched.FetchAdd( want );
    data.resize( (size_t) want );
    memcpy( &data[ 0 ], block.data.GetConstBufferPtr(), (size_t) want );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::StoreRun( CAssetBlockCache::TEntryHandle entry, UInt32 firstBlock, const UInt8* data, UInt32 length )
{GUCEF_TRACE;

    if ( 0 == entry )
        return;
    UInt32 done = 0;
    UInt32 block = firstBlock;
    while ( done < length )
    {
        const UInt32 blockSize = m_cache.GetBlockSize( entry, block );
        if ( 0 == blockSize || done + blockSize > length )
            break;
        m_cache.StoreBlock( entry, block, data + done, blockSize );
        done += blockSize;
        ++block;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::CheckComplete( SOpenFile& file )
{GUCEF_TRACE;

    if ( 0 == file.entry || file.digest.IsNULLOrEmpty() || !m_cache.IsComplete( file.entry ) )
        return;
    {
        MT::CScopeMutex lock( m_lock );
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator v = m_verified.find( file.path );
        if ( v != m_verified.end() && v->second == file.digest )
            return;
    }
    if ( m_cache.VerifyDigest( file.entry, file.digest ) )
    {
        m_filesVerified.FetchAdd( 1 );
        MT::CScopeMutex lock( m_lock );
        m_verified[ file.path ] = file.digest;
        return;
    }
    // what the service gave does not make the file the catalogue knows: the entry is emptied and the reader is told
    m_digestMismatches.FetchAdd( 1 );
    m_cache.ResetEntry( file.entry );
    file.status = ASSETSTORE_STATUS_DIGEST_MISMATCH;
    file.message = DescribeFailure( file.path, ASSETSTORE_STATUS_DIGEST_MISMATCH, "the cached copy was emptied and is fetched again at the next read" );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetFileSystem: " << file.message );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::ScheduleMissedRun( SOpenFile& file, UInt32 firstBlock, UInt32 blockCount )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_missed.insert( file.path );
        if ( 0 == file.entry || m_asyncRuns.size() >= 8 )
            return;
        for ( size_t i=0; i<m_asyncRuns.size(); ++i )
            if ( m_asyncRuns[ i ].path == file.path && m_asyncRuns[ i ].firstBlock <= firstBlock && m_asyncRuns[ i ].firstBlock + m_asyncRuns[ i ].blockCount > firstBlock )
                return;
    }
    CAssetBlockCache::TEntryHandle entry = m_cache.AcquireEntry( file.path, file.size, file.digest );
    if ( 0 == entry )
        return;
    SAsyncRun run;
    run.path = file.path;
    run.size = file.size;
    run.digest = file.digest;
    run.entry = entry;
    run.firstBlock = firstBlock;
    run.blockCount = blockCount;
    const UInt64 start = (UInt64) firstBlock * CAssetBlockCache::BlockBytes;
    UInt64 want = (UInt64) blockCount * CAssetBlockCache::BlockBytes;
    if ( start + want > file.size )
        want = file.size - start;
    run.future = m_service.ReadFileBlockAsync( file.path, start, (UInt32) want );
    MT::CScopeMutex lock( m_lock );
    m_asyncRuns.push_back( run );
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetFileSystem::ReadFromCacheOrFetch( SOpenFile& file, UInt64 offset, void* dest, UInt32 length )
{GUCEF_TRACE;

    const UInt32 blockBytes = CAssetBlockCache::BlockBytes;
    const UInt32 first = (UInt32) ( offset / blockBytes );
    const UInt64 blockStart = (UInt64) first * blockBytes;
    const UInt64 left = file.size - blockStart;
    const UInt32 blockSize = left < blockBytes ? (UInt32) left : blockBytes;
    const UInt32 inBlock = (UInt32) ( offset - blockStart );
    UInt32 n = blockSize - inBlock;
    if ( n > length )
        n = length;

    if ( 0 != file.entry )
    {
        if ( CAssetBlockCache::READ_HIT == m_cache.ReadBlock( file.entry, first, inBlock, dest, n ) )
            return n;
    }
    else
    if ( !file.window.empty() && offset >= file.windowStart && offset + n <= file.windowStart + file.window.size() )
    {
        memcpy( dest, &file.window[ (size_t) ( offset - file.windowStart ) ], n );
        return n;
    }

    if ( IsNoWaitThread() )
    {
        // the interface never waits: the fetch starts and the application is told when it is there
        m_noWaitMisses.FetchAdd( 1 );
        const UInt32 lastNeeded = (UInt32) ( ( offset + length - 1 ) / blockBytes );
        UInt32 count = lastNeeded - first + 1;
        if ( count > m_settings.maxBlocksPerRequest )
            count = m_settings.maxBlocksPerRequest;
        ScheduleMissedRun( file, first, count );
        file.status = ASSETSTORE_STATUS_UNAVAILABLE;
        file.message = "The file " + file.path + " is being fetched from the store service";
        return 0;
    }

    // the blocks of this read that are not in the cache, in one call (at most maxBlocksPerRequest); a sequential read asks for the
    // blocks that follow in the same call (read ahead), so a reader of a whole file costs a few large calls and not one per block
    const UInt32 lastNeeded = (UInt32) ( ( offset + length - 1 ) / blockBytes );
    const UInt32 blockCount = (UInt32) ( ( file.size + blockBytes - 1 ) / blockBytes );
    UInt32 wanted = lastNeeded - first + 1;
    UInt32 ahead = 0;
    if ( m_settings.readAheadBlocks > 0 && m_settings.readAheadAfterBlocks > 0 && file.sequentialBlocks >= m_settings.readAheadAfterBlocks && offset == file.nextSequential )
        ahead = m_settings.readAheadBlocks;
    if ( wanted + ahead > m_settings.maxBlocksPerRequest )
        ahead = m_settings.maxBlocksPerRequest > wanted ? m_settings.maxBlocksPerRequest - wanted : 0;
    if ( first + wanted + ahead > blockCount )
        ahead = blockCount > first + wanted ? blockCount - first - wanted : 0;
    UInt32 count = 1;
    while ( count < wanted + ahead && count < m_settings.maxBlocksPerRequest && ( 0 == file.entry || !m_cache.HasBlock( file.entry, first + count ) ) )
        ++count;
    if ( count > wanted )
        m_readAheadBlocks.FetchAdd( count - wanted );
    GUCEF::vector< UInt8 > data;
    if ( !FetchRun( file, first, count, data ) )
        return 0;
    if ( 0 != file.entry )
        StoreRun( file.entry, first, &data[ 0 ], (UInt32) data.size() );
    else
    {
        file.window.swap( data );
        file.windowStart = blockStart;
        memcpy( dest, &file.window[ inBlock ], n );
        return n;
    }
    memcpy( dest, &data[ inBlock ], n );
    CheckComplete( file );
    return ASSETSTORE_STATUS_OK == file.status ? n : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetFileSystem::ReadFile( SOpenFile& file, UInt64 offset, void* dest, UInt32 length )
{GUCEF_TRACE;

    file.status = ASSETSTORE_STATUS_OK;
    file.message.Clear();
    if ( offset >= file.size || 0 == length )
        return 0;
    if ( file.cancelled )
    {
        file.status = ASSETSTORE_STATUS_CANCELLED;
        file.message = DescribeFailure( file.path, ASSETSTORE_STATUS_CANCELLED, CORE::CString::Empty );
        return 0;
    }
    if ( offset + length > file.size )
        length = (UInt32) ( file.size - offset );
    UInt32 total = 0;
    UInt8* out = static_cast< UInt8* >( dest );
    while ( total < length )
    {
        if ( file.cancelled )
        {
            file.status = ASSETSTORE_STATUS_CANCELLED;
            file.message = DescribeFailure( file.path, ASSETSTORE_STATUS_CANCELLED, CORE::CString::Empty );
            break;
        }
        const UInt32 n = ReadFromCacheOrFetch( file, offset + total, out + total, length - total );
        if ( 0 == n )
            break;
        total += n;
    }
    if ( total > 0 )
    {
        // a sequential read fetches the next blocks ahead
        const UInt32 blockBytes = CAssetBlockCache::BlockBytes;
        const UInt32 firstBlock = (UInt32) ( offset / blockBytes );
        const UInt32 lastBlock = (UInt32) ( ( offset + total - 1 ) / blockBytes );
        if ( offset == file.nextSequential )
        {
            const Int64 previousLast = 0 == offset ? -1 : (Int64) ( ( offset - 1 ) / blockBytes );
            const Int64 newFirst = (Int64) firstBlock > previousLast + 1 ? (Int64) firstBlock : previousLast + 1;
            if ( (Int64) lastBlock >= newFirst )
                file.sequentialBlocks += (UInt32) ( (Int64) lastBlock - newFirst + 1 );
        }
        else
            file.sequentialBlocks = 0;
        file.nextSequential = offset + total;
    }
    return total;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetFileSystem::IsFileCached( const CORE::CString& vfsPath, UInt64 size ) const
{GUCEF_TRACE;

    if ( 0 == size )
        return true;
    CAssetBlockCache::SEntryInfo info;
    return m_cacheOpen && m_cache.GetEntryInfo( Normalize( vfsPath ), info ) && info.complete && info.fileSize == size;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::UnpinFile( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( m_cacheOpen )
        m_cache.SetPinned( Normalize( vfsPath ), false );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetFileSystem::FetchWholeFile( const CORE::CString& vfsPath, bool pin, const MT::CTAtomic< bool >* cancel, MT::CTAtomic< UInt64 >* done, CORE::CString& message )
{GUCEF_TRACE;

    SOpenFile file;
    if ( !OpenFile( vfsPath, file, message ) )
        return ASSETSTORE_STATUS_NOT_FOUND == file.status && IsOffline() ? ASSETSTORE_STATUS_UNAVAILABLE : file.status;
    if ( 0 == file.size )
    {
        message.Clear();
        return ASSETSTORE_STATUS_OK;
    }
    if ( 0 == file.entry )
    {
        message = "The file " + file.path + " cannot be fetched whole: the cache of this computer is not available";
        return ASSETSTORE_STATUS_FAILED;
    }
    TAssetStoreStatus result = ASSETSTORE_STATUS_OK;
    const UInt32 blockCount = m_cache.GetBlockCount( file.entry );
    for ( UInt32 attempt=0; attempt<2; ++attempt )
    {
        result = ASSETSTORE_STATUS_OK;
        UInt32 block = 0;
        while ( block < blockCount )
        {
            if ( GUCEF_NULL != cancel && cancel->Load() )
            {
                result = ASSETSTORE_STATUS_CANCELLED;
                message = DescribeFailure( file.path, ASSETSTORE_STATUS_CANCELLED, CORE::CString::Empty );
                break;
            }
            if ( m_cache.HasBlock( file.entry, block ) )
            {
                ++block;
                continue;
            }
            UInt32 count = 1;
            while ( block + count < blockCount && count < m_settings.maxBlocksPerRequest && !m_cache.HasBlock( file.entry, block + count ) )
                ++count;
            GUCEF::vector< UInt8 > data;
            if ( !FetchRun( file, block, count, data ) )
            {
                result = file.status;
                message = file.message;
                break;
            }
            StoreRun( file.entry, block, &data[ 0 ], (UInt32) data.size() );
            if ( GUCEF_NULL != done )
                done->FetchAdd( data.size() );
            block += count;
        }
        if ( ASSETSTORE_STATUS_OK != result )
            break;
        if ( file.digest.IsNULLOrEmpty() )
            break;
        if ( m_cache.VerifyDigest( file.entry, file.digest ) )
        {
            m_filesVerified.FetchAdd( 1 );
            MT::CScopeMutex lock( m_lock );
            m_verified[ file.path ] = file.digest;
            break;
        }
        // the copy is not the file of the catalogue: emptied, and tried once more
        m_digestMismatches.FetchAdd( 1 );
        m_cache.ResetEntry( file.entry );
        result = ASSETSTORE_STATUS_DIGEST_MISMATCH;
        message = DescribeFailure( file.path, ASSETSTORE_STATUS_DIGEST_MISMATCH, "the copy was fetched again" + CORE::CString( attempt > 0 ? " and still differs" : "" ) );
        if ( attempt > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetFileSystem: " << message );
    }
    if ( ASSETSTORE_STATUS_OK == result && pin )
        m_cache.SetEntryPinned( file.entry, true );
    if ( ASSETSTORE_STATUS_OK == result )
        message.Clear();
    CloseFile( file );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetFileSystem::StartTransfer( const TRemoteFileTransferPtr& transfer, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    if ( transfer.IsNULL() || !m_open )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CString( "AssetStoreTransfers" ) : threadPoolName );
    if ( threadPool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    CRemoteFileTransferTaskData* data = GUCEF_NEW CRemoteFileTransferTaskData();
    data->system = this;
    data->transfer = transfer;
    CORE::CFutureResultSource source;
    data->source = source;
    {
        MT::CScopeMutex lock( m_lock );
        m_transfers.push_back( transfer );
    }
    CORE::CFutureResult queued = threadPool->QueueTask( CRemoteFileTransferTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        TransferEnded( transfer );
        GUCEF_DELETE data;
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The transfer could not be queued on the thread pool" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::TransferEnded( const TRemoteFileTransferPtr& transfer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_transfers.size(); ++i )
    {
        if ( m_transfers[ i ].GetPointerAlways() == transfer.GetPointerAlways() )
        {
            m_transfers.erase( m_transfers.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteAssetFileSystem::GetRunningTransferCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_transfers.size();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::TakeMissedPaths( CORE::CString::StringSet& paths )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    paths = m_missed;
    m_missed.clear();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::Update( void )
{GUCEF_TRACE;

    CORE::CString::StringVector filesArrived;
    CORE::CString::StringVector foldersArrived;
    TAsyncRunVector runs;
    TAsyncListVector lists;
    bool stateChanged = false;
    {
        // what is finished is taken out under the lock and handled outside of it
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_asyncRuns.size(); )
        {
            if ( m_asyncRuns[ i ].future.HasAFuture() && !m_asyncRuns[ i ].future.IsReady() )
            {
                ++i;
                continue;
            }
            runs.push_back( m_asyncRuns[ i ] );
            m_asyncRuns.erase( m_asyncRuns.begin() + i );
        }
        for ( size_t i=0; i<m_asyncLists.size(); )
        {
            if ( m_asyncLists[ i ].future.HasAFuture() && !m_asyncLists[ i ].future.IsReady() )
            {
                ++i;
                continue;
            }
            lists.push_back( m_asyncLists[ i ] );
            m_asyncLists.erase( m_asyncLists.begin() + i );
        }
        if ( m_offlineChanged )
        {
            m_offlineChanged = false;
            stateChanged = true;
        }
    }

    for ( size_t i=0; i<runs.size(); ++i )
    {
        SAsyncRun& run = runs[ i ];
        CAssetFileBlock block;
        if ( ASSETSTORE_STATUS_OK == CIAssetStoreService::GetStatusOfFuture( run.future ) && CIAssetStoreService::TryGetFutureResult( run.future, block ) &&
             block.fileSize == run.size )
        {
            StoreRun( run.entry, run.firstBlock, static_cast< const UInt8* >( block.data.GetConstBufferPtr() ), block.data.GetDataSize() );
            m_bytesFetched.FetchAdd( block.data.GetDataSize() );
            filesArrived.push_back( run.path );
        }
        m_cache.ReleaseEntry( run.entry );
    }

    for ( size_t i=0; i<lists.size(); ++i )
    {
        SAsyncList& list = lists[ i ];
        CAssetFileStatPage page;
        const TAssetStoreStatus status = CIAssetStoreService::GetStatusOfFuture( list.future );
        if ( ASSETSTORE_STATUS_OK != status || !CIAssetStoreService::TryGetFutureResult( list.future, page ) )
        {
            if ( ASSETSTORE_STATUS_NOT_FOUND == status )
            {
                SMeta meta;
                meta.kind = 3;
                meta.validUntilMs = NowMs() + m_settings.negativeTtlMs;
                PutMeta( list.folder, meta );
                foldersArrived.push_back( list.folder );
            }
            else
            if ( IsNetworkStatus( status ) )
                SetOffline( true );
            continue;
        }
        SetOffline( false );
        ApplyPage( list.folder, page, list.listing, list.entries );
        if ( !page.nextPageToken.IsNULLOrEmpty() && list.entries < m_settings.maxListEntries )
        {
            list.future = m_service.ListDirectoryAsync( list.folder, page.nextPageToken, m_settings.listPageEntries );
            MT::CScopeMutex lock( m_lock );
            m_asyncLists.push_back( list );
            continue;
        }
        list.listing.truncated = !page.nextPageToken.IsNULLOrEmpty();
        StoreListing( list.folder, list.listing );
        foldersArrived.push_back( list.folder );
    }
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_offlineChanged )
        {
            m_offlineChanged = false;
            stateChanged = true;
        }
    }

    for ( size_t i=0; i<filesArrived.size(); ++i )
    {
        CORE::TCloneableString data( filesArrived[ i ] );
        NotifyObservers( FileArrivedEvent, &data );
    }
    for ( size_t i=0; i<foldersArrived.size(); ++i )
    {
        CORE::TCloneableString data( foldersArrived[ i ] );
        NotifyObservers( ListingArrivedEvent, &data );
    }
    if ( stateChanged )
        NotifyObservers( StateChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetFileSystem::GetStatistics( SStatistics& statistics ) const
{GUCEF_TRACE;

    statistics.bytesFetched = m_bytesFetched.Load();
    statistics.fetchCalls = m_fetchCalls.Load();
    statistics.readAheadBlocks = m_readAheadBlocks.Load();
    statistics.listCalls = m_listCalls.Load();
    statistics.missesOfNoWaitThread = m_noWaitMisses.Load();
    statistics.digestMismatches = m_digestMismatches.Load();
    statistics.filesVerified = m_filesVerified.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
