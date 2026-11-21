/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

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

#ifndef GUCEF_VFS_CVFSREMOTEPATH_H
#include "gucefVFS_CVfsRemotePath.h"
#define GUCEF_VFS_CVFSREMOTEPATH_H
#endif /* GUCEF_VFS_CVFSREMOTEPATH_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#include "gucefVFS_CVfsBlockRunFetcher.h"
#define GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#endif /* GUCEF_VFS_CVFSBLOCKRUNFETCHER_H ? */

#ifndef GUCEF_VFS_CVFSCACHEDIOACCESS_H
#include "gucefVFS_CVfsCachedIOAccess.h"
#define GUCEF_VFS_CVFSCACHEDIOACCESS_H
#endif /* GUCEF_VFS_CVFSCACHEDIOACCESS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEFETCHJOB_H
#include "gucefVFS_CVfsRemoteFetchJob.h"
#define GUCEF_VFS_CVFSREMOTEFETCHJOB_H
#endif /* GUCEF_VFS_CVFSREMOTEFETCHJOB_H ? */

#include "gucefVFS_CVfsRemoteArchiveCore.h"

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

/** A source without revisions passes "-": the cache needs a revision in its key */
static void
NormalizeRemoteEntry( CVfsRemoteEntry& entry )
{GUCEF_TRACE;

    if ( entry.attributesKnown && entry.exists && !entry.isDirectory && entry.revision.IsNULLOrEmpty() )
        entry.revision = "-";
}

/*-------------------------------------------------------------------------*/

static void
NormalizeRemoteEntries( CVfsRemoteEntry::TVector& entries )
{GUCEF_TRACE;

    for ( size_t i=0; i<entries.size(); ++i )
        NormalizeRemoteEntry( entries[ i ] );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsRemoteArchiveCorePtr
CVfsRemoteArchiveCore::Create( const CIVfsRemoteBackendPtr& backend  ,
                               const CVfsBlockCachePtr& cache        ,
                               const CVfsRemoteSettings& settings    ,
                               CVfsFetchScheduler* scheduler         ,
                               CVfsThreadPolicyRegistry* policies    )
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr core( GUCEF_NEW CVfsRemoteArchiveCore( backend, cache, settings, scheduler, policies ) );
    core->m_self = core;
    return core;
}

/*-------------------------------------------------------------------------*/

CVfsRemoteArchiveCore::CVfsRemoteArchiveCore( const CIVfsRemoteBackendPtr& backend  ,
                                              const CVfsBlockCachePtr& cache        ,
                                              const CVfsRemoteSettings& settings    ,
                                              CVfsFetchScheduler* scheduler         ,
                                              CVfsThreadPolicyRegistry* policies    )
    : CORE::CNotifier()
    , CIRemoteArchive()
    , m_backend( backend )
    , m_cache( cache )
    , m_settings( settings )
    , m_identity( settings.cacheIdentity.IsNULLOrEmpty() ? CORE::CString( "default" ) : settings.cacheIdentity )
    , m_scheduler( scheduler )
    , m_policies( policies )
    , m_metadata( settings.metadataTtlMs, settings.maxMetadataEntries )
    , m_counters()
    , m_lastResults()
    , m_hintLock()
    , m_hints()
    , m_hintSequence( 0 )
    , m_shutdown( false )
    , m_linkWasUp( false )
    , m_self()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_scheduler )
        m_scheduler = &CVfsGlobal::Instance()->GetFetchScheduler();
    if ( GUCEF_NULL == m_policies )
        m_policies = &CVfsGlobal::Instance()->GetThreadPolicyRegistry();
    m_linkWasUp.Store( !m_backend.IsNULL() && m_backend->IsLinkUp() );
}

/*-------------------------------------------------------------------------*/

CVfsRemoteArchiveCore::~CVfsRemoteArchiveCore()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsRemoteArchiveCore::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "GUCEF::VFS::CVfsRemoteArchiveCore";
    return typeName;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::Shutdown( void )
{GUCEF_TRACE;

    if ( m_shutdown.Exchange( true ) )
        return;

    // the jobs of this archive are dropped and waited for: nothing of them runs or raises an event after this returns
    m_scheduler->CancelAll( this );

    // the weak reference to itself goes while there are strong ones: released in the destructor, as the last reference to the shared data, it would
    // end the shared data there and the pointer that is being released would end it again. A core that is shut down needs it no more: every call that
    // used it ends with "the archive is unloaded" first
    m_self.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::IsShutdown( void ) const
{GUCEF_TRACE;

    return m_shutdown.Load();
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::IsLinkUp( void ) const
{GUCEF_TRACE;

    return !m_backend.IsNULL() && m_backend->IsLinkUp();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::NotifyLinkState( bool linkUp )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;
    if ( m_linkWasUp.Exchange( linkUp ) == linkUp )
        return;

    m_counters.eventsRaised.FetchAdd( 1 );
    CRemoteArchiveEvents::TConnectionStateChangedEventData eventData( linkUp );
    NotifyObservers( CRemoteArchiveEvents::ConnectionStateChangedEvent, &eventData );

    // the kept files that changed while the link was down are fetched again
    if ( linkUp )
        HealPinnedPaths();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsRemoteArchiveCore::GetIdentity( void ) const
{GUCEF_TRACE;

    return m_identity;
}

/*-------------------------------------------------------------------------*/

const CVfsRemoteSettings&
CVfsRemoteArchiveCore::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCachePtr
CVfsRemoteArchiveCore::GetBlockCache( void ) const
{GUCEF_TRACE;

    return m_cache;
}

/*-------------------------------------------------------------------------*/

CVfsRemoteCounters&
CVfsRemoteArchiveCore::GetCounters( void )
{GUCEF_TRACE;

    return m_counters;
}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler&
CVfsRemoteArchiveCore::GetScheduler( void )
{GUCEF_TRACE;

    return *m_scheduler;
}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicy
CVfsRemoteArchiveCore::GetCurrentThreadPolicy( void ) const
{GUCEF_TRACE;

    return m_policies->GetCurrent();
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::Record( const CVfsCallResult& result ) const
{GUCEF_TRACE;

    m_lastResults.Set( result );
    return result;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::RecordCallResult( const CVfsCallResult& result )
{GUCEF_TRACE;

    Record( result );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::GetLastResult( void ) const
{GUCEF_TRACE;

    return m_lastResults.Get();
}

/*-------------------------------------------------------------------------*/

static CVfsCallResult
DescribePinRefusal( const CORE::CString& path, CVfsBlockCache::EPinResult refusal )
{GUCEF_TRACE;

    if ( CVfsBlockCache::PIN_OVER_LIMIT == refusal )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_NO_SPACE, "\"" + path + "\" is not kept offline: the files that are kept offline would pass the size limit of the cache. Release a kept file or raise the limit" );
    return CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" is not kept offline: the cache holds no current revision of it" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemoteArchiveCore::DescribeOffline(
 const CORE::CString& path, UInt64 firstByte, UInt64 lastByte ) const
{GUCEF_TRACE;

    return "\"" + path + "\": bytes " + CORE::ToString( firstByte ) + "-" + CORE::ToString( lastByte ) +
           " are not in the cache of this computer and " + m_backend->GetPlaceDescription() + " cannot be reached";
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::RaiseArrival( const CORE::CEvent& eventId, const CVfsArrivalInfo& info )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;
    m_counters.eventsRaised.FetchAdd( 1 );
    CVfsArrivalInfo copy( info );
    NotifyObservers( eventId, &copy );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::RaiseFileChanged( const CORE::CString& path, const CORE::CString& oldRevision, const CORE::CString& newRevision )
{GUCEF_TRACE;

    CVfsArrivalInfo info( CVfsArrivalInfo::KIND_CHANGE, path );
    info.oldRevision = oldRevision;
    info.revision = newRevision;
    info.result = CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED;
    RaiseArrival( CRemoteArchiveEvents::FileChangedEvent, info );
}

/*-------------------------------------------------------------------------*/

CVfsBlockCache::TEntryHandle
CVfsRemoteArchiveCore::AcquireCacheEntry( const CORE::CString& path, const CVfsRemoteEntry& entry, bool create ) const
{GUCEF_TRACE;

    if ( m_cache.IsNULL() || entry.size == 0 )
        return 0;
    return m_cache->AcquireEntry( m_identity, path, entry.revision, entry.size, entry.digest, create );
}

/*-------------------------------------------------------------------------*/

CIVfsBlockSourcePtr
CVfsRemoteArchiveCore::OpenSource( const CORE::CString& path, const CORE::CString& revision, CVfsCallResult& result )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
    {
        result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" );
        return CIVfsBlockSourcePtr();
    }
    m_counters.openCalls.FetchAdd( 1 );
    result = CVfsCallResult();
    CIVfsBlockSourcePtr source = m_backend->OpenBlockSource( path, revision, result );
    if ( source.IsNULL() && result.IsSuccess() )
        result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the remote place gave no source for \"" + path + "\"" );
    return source;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::RefreshEntry( const CORE::CString& path, CVfsRemoteEntry& fresh, bool& changed, bool& revisionMoved, CORE::CString& oldRevision, bool healPinned )
{GUCEF_TRACE;

    changed = false;
    revisionMoved = false;
    m_counters.statCalls.FetchAdd( 1 );

    CVfsRemoteEntry answer;
    CVfsCallResult result = m_backend->Stat( path, answer );
    if ( !result.IsSuccess() )
        return result;

    NormalizeRemoteEntry( answer );
    CVfsRemoteEntry previous;
    bool hadPrevious = false;
    changed = m_metadata.StoreEntry( path, answer, &previous, &hadPrevious );
    if ( hadPrevious && previous.exists && !previous.isDirectory && answer.exists && !answer.isDirectory && previous.revision != answer.revision )
    {
        revisionMoved = true;
        oldRevision = previous.revision;
    }

    // what the remote place says is the current revision: the copies of other revisions in the cache are superseded
    if ( ( changed || !hadPrevious ) && answer.exists && !answer.isDirectory && !m_cache.IsNULL() )
        m_cache->SupersedeOtherRevisions( m_identity, path, answer.revision );
    if ( healPinned && ( changed || !hadPrevious ) && answer.exists && !answer.isDirectory )
        HealPinnedPath( path );

    fresh = answer;
    return CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::RefreshListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists, bool& changed )
{GUCEF_TRACE;

    m_counters.listCalls.FetchAdd( 1 );

    CVfsRemoteEntry::TVector answer;
    bool answerDirExists = false;
    CVfsCallResult result = m_backend->List( dirPath, answer, answerDirExists );
    if ( !result.IsSuccess() )
        return result;

    NormalizeRemoteEntries( answer );
    changed = m_metadata.StoreListing( dirPath, answer, answerDirExists );
    if ( changed && !m_cache.IsNULL() )
    {
        for ( size_t i=0; i<answer.size(); ++i )
        {
            if ( answer[ i ].attributesKnown && answer[ i ].exists && !answer[ i ].isDirectory )
            {
                const CORE::CString filePath = CVfsRemotePath::Join( dirPath, answer[ i ].name );
                m_cache->SupersedeOtherRevisions( m_identity, filePath, answer[ i ].revision );
                HealPinnedPath( filePath );
            }
        }
    }
    entries = answer;
    dirExists = answerDirExists;
    return CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::HealPinnedPath( const CORE::CString& path )
{GUCEF_TRACE;

    // a kept file whose current revision is not complete yet is fetched again without anyone asking: the pin moves to it when it is complete
    if ( !m_settings.refetchPinned || m_cache.IsNULL() || m_shutdown.Load() || !m_cache->HasStalePinnedCopy( m_identity, path ) )
        return;
    ScheduleJob( FETCHJOB_WHOLE, path, CVfsRemoteEntry(), 0, 0 );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::HealPinnedPaths( void )
{GUCEF_TRACE;

    if ( !m_settings.refetchPinned || m_cache.IsNULL() || m_shutdown.Load() )
        return;
    CORE::CString::StringVector paths;
    m_cache->GetStalePinnedPaths( m_identity, paths );
    for ( size_t i=0; i<paths.size(); ++i )
    {
        // the scheduler has bounds: the paths that did not get a job are found again at the next open or the next link
        const CVfsCallResult scheduled = ScheduleJob( FETCHJOB_WHOLE, paths[ i ], CVfsRemoteEntry(), 0, 0 );
        if ( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == scheduled.result )
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::EntryFromCacheIndex(
 const CORE::CString& path, CVfsRemoteEntry& entry, bool forOfflineServing ) const
{GUCEF_TRACE;

    if ( m_cache.IsNULL() )
        return false;

    // what is served offline may be an older, kept copy in place of a newer revision that is not held completely
    CVfsBlockCache::SEntryInfo info;
    if ( forOfflineServing ? !m_cache->FindOfflineRevision( m_identity, path, info ) : !m_cache->FindNewestRevision( m_identity, path, info ) )
        return false;

    entry = CVfsRemoteEntry();
    entry.name = CVfsRemotePath::LeafOf( path );
    entry.exists = true;
    entry.isDirectory = false;
    entry.size = info.fileSize;
    entry.revision = info.revision;
    entry.digest = info.digest;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::ListingFromCacheIndex( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists ) const
{GUCEF_TRACE;

    entries.clear();
    dirExists = false;
    if ( m_cache.IsNULL() )
        return;

    CVfsBlockCache::TEntryInfoVector infos;
    m_cache->GetEntryInfosBelow( m_identity, dirPath, true, infos );

    CORE::CString::StringSet dirNames;
    const UInt32 prefixBytes = dirPath.IsNULLOrEmpty() ? 0 : CVfsRemotePath::GetByteCount( dirPath ) + 1;
    for ( size_t i=0; i<infos.size(); ++i )
    {
        const CORE::CString& full = infos[ i ].path;
        const UInt32 fullBytes = CVfsRemotePath::GetByteCount( full );
        if ( fullBytes <= prefixBytes )
            continue;
        const CORE::CString relative( full.C_String() + prefixBytes, fullBytes - prefixBytes );
        const CORE::CString parentOfRelative = CVfsRemotePath::ParentOf( relative );
        if ( !parentOfRelative.IsNULLOrEmpty() )
        {
            // a file deeper down: its first directory is implied
            CORE::CString first = parentOfRelative;
            while ( !CVfsRemotePath::ParentOf( first ).IsNULLOrEmpty() )
                first = CVfsRemotePath::ParentOf( first );
            dirNames.insert( first );
            continue;
        }

        CVfsRemoteEntry entry;
        entry.name = relative;
        entry.exists = true;
        entry.isDirectory = false;
        entry.size = infos[ i ].fileSize;
        entry.revision = infos[ i ].revision;
        entry.digest = infos[ i ].digest;
        entries.push_back( entry );
    }
    for ( CORE::CString::StringSet::const_iterator d = dirNames.begin(); d != dirNames.end(); ++d )
    {
        CVfsRemoteEntry entry;
        entry.name = (*d);
        entry.exists = true;
        entry.isDirectory = true;
        entries.push_back( entry );
    }
    dirExists = !entries.empty();
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::ScheduleJob( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount )
{GUCEF_TRACE;

    CVfsRemoteArchiveCorePtr self = m_self.GetSharedPtr();
    if ( m_shutdown.Load() || self.IsNULL() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" );

    // no job is made for a link that is down: ConnectionStateChangedEvent tells when to try again
    if ( !IsLinkUp() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "\"" + path + "\": " + m_backend->GetPlaceDescription() + " cannot be reached" );

    CIVfsFetchJobPtr job( GUCEF_NEW CVfsRemoteFetchJob( self, kind, path, entry, firstBlock, blockCount ) );
    switch ( m_scheduler->Submit( job ) )
    {
        case CVfsFetchScheduler::SUBMIT_ACCEPTED:
        {
            m_counters.fetchJobsAccepted.FetchAdd( 1 );
            return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "\"" + path + "\" is not held locally: it is being fetched", true );
        }
        case CVfsFetchScheduler::SUBMIT_MERGED:
        {
            m_counters.fetchJobsMerged.FetchAdd( 1 );
            return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "\"" + path + "\" is not held locally: it is being fetched", true );
        }
        default:
        {
            m_counters.fetchJobsRefused.FetchAdd( 1 );
            CVfsArrivalInfo info( CVfsArrivalInfo::KIND_FAILURE, path );
            info.result = CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER;
            info.message = "too many background fetches are waiting: try again later";
            info.jobKind = kind;
            RaiseArrival( CRemoteArchiveEvents::FetchFailedEvent, info );
            return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER, info.message, false );
        }
    }
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::ScheduleBlocks( const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount )
{GUCEF_TRACE;

    return ScheduleJob( FETCHJOB_BLOCKS, path, entry, firstBlock, blockCount );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::LookupEntry( const CORE::CString& path, CVfsRemoteEntry& entry )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" );

    const CVfsThreadPolicy policy = GetCurrentThreadPolicy();
    const bool online = IsLinkUp();

    const CVfsRemoteMetadataCache::EState state = m_metadata.FindEntry( path, entry );
    if ( CVfsRemoteMetadataCache::STATE_FRESH == state )
        return CVfsCallResult();

    if ( CVfsRemoteMetadataCache::STATE_STALE == state )
    {
        // while the link is down the stale answer is the one there is
        if ( !online )
        {
            m_counters.offlineReads.FetchAdd( 1 );
            return CVfsCallResult();
        }
        // a thread that must not wait is answered at once and the answer is checked again in the background
        if ( policy.NeverWaits() && policy.serveStaleMetadata )
        {
            if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
                ScheduleJob( FETCHJOB_REVALIDATE, path, CVfsRemoteEntry(), 0, 0 );
            return CVfsCallResult();
        }
    }

    if ( !online )
    {
        if ( EntryFromCacheIndex( path, entry, true ) )
        {
            m_counters.offlineReads.FetchAdd( 1 );
            return CVfsCallResult();
        }
        m_counters.offlineMisses.FetchAdd( 1 );
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "offline: " + m_backend->GetPlaceDescription() + " cannot be reached and nothing is known of \"" + path + "\"" );
    }

    if ( policy.NeverWaits() )
    {
        m_counters.policyMisses.FetchAdd( 1 );
        if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
            return ScheduleJob( FETCHJOB_STAT, path, CVfsRemoteEntry(), 0, 0 );
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "\"" + path + "\" is not known locally and this thread does not wait for it" );
    }

    // a thread that waits asks the remote place
    CVfsRemoteEntry fresh;
    bool changed = false;
    bool moved = false;
    CORE::CString oldRevision;
    const CVfsCallResult refreshed = RefreshEntry( path, fresh, changed, moved, oldRevision );
    if ( !refreshed.IsSuccess() )
    {
        // the link went while this thread asked: the stale answer, when there is one
        if ( CVfsRemoteMetadataCache::STATE_STALE == state && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == refreshed.result )
            return CVfsCallResult();
        return refreshed;
    }
    entry = fresh;
    if ( moved )
        RaiseFileChanged( path, oldRevision, fresh.revision );
    return CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::LookupListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists )
{GUCEF_TRACE;

    entries.clear();
    dirExists = false;
    if ( m_shutdown.Load() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" );

    const CVfsThreadPolicy policy = GetCurrentThreadPolicy();
    const bool online = IsLinkUp();

    const CVfsRemoteMetadataCache::EState state = m_metadata.FindListing( dirPath, entries, dirExists );
    if ( CVfsRemoteMetadataCache::STATE_FRESH == state )
        return CVfsCallResult();

    if ( CVfsRemoteMetadataCache::STATE_STALE == state )
    {
        if ( !online )
        {
            m_counters.offlineReads.FetchAdd( 1 );
            return CVfsCallResult();
        }
        if ( policy.NeverWaits() && policy.serveStaleMetadata )
        {
            if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
                ScheduleJob( FETCHJOB_LIST, dirPath, CVfsRemoteEntry(), 0, 0 );
            return CVfsCallResult();
        }
    }

    if ( !online )
    {
        entries.clear();
        dirExists = false;
        if ( !m_settings.offlineListings )
        {
            m_counters.offlineMisses.FetchAdd( 1 );
            return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "offline: " + m_backend->GetPlaceDescription() + " cannot be reached and the directory \"" + dirPath + "\" is not known" );
        }
        // what the cache holds of the directory, and nothing else: the result says that the list is not the whole one
        ListingFromCacheIndex( dirPath, entries, dirExists );
        if ( entries.empty() )
            m_counters.offlineMisses.FetchAdd( 1 );
        else
            m_counters.offlineReads.FetchAdd( 1 );
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "offline: only files kept in the cache of this computer are listed" );
    }

    if ( policy.NeverWaits() )
    {
        entries.clear();
        dirExists = false;
        m_counters.policyMisses.FetchAdd( 1 );
        if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
            return ScheduleJob( FETCHJOB_LIST, dirPath, CVfsRemoteEntry(), 0, 0 );
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "the directory \"" + dirPath + "\" is not known locally and this thread does not wait for it" );
    }

    bool changed = false;
    CVfsRemoteEntry::TVector fresh;
    bool freshDirExists = false;
    const CVfsCallResult refreshed = RefreshListing( dirPath, fresh, freshDirExists, changed );
    if ( !refreshed.IsSuccess() )
    {
        if ( CVfsRemoteMetadataCache::STATE_STALE == state && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == refreshed.result )
            return CVfsCallResult();        // the stale listing stays in 'entries'
        entries.clear();
        dirExists = false;
        return refreshed;
    }
    entries = fresh;
    dirExists = freshDirExists;
    return CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::FileExists( const CORE::CString& filePath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    if ( path.IsNULLOrEmpty() )
    {
        // the root of the mount is a directory by construction: no call to the remote place
        Record( CVfsCallResult() );
        return false;
    }
    CVfsRemoteEntry entry;
    const CVfsCallResult result = Record( LookupEntry( path, entry ) );
    return result.IsSuccess() && entry.exists && !entry.isDirectory;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::DirExists( const CORE::CString& dirPath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( dirPath );
    if ( path.IsNULLOrEmpty() )
    {
        // the root of the mount exists by construction: no call to the remote place, whatever the state of the link and of the cache
        Record( CVfsCallResult() );
        return true;
    }
    CVfsRemoteEntry entry;
    const CVfsCallResult result = LookupEntry( path, entry );
    if ( result.IsSuccess() )
    {
        Record( result );
        return entry.exists && entry.isDirectory;
    }

    // offline, and the directory is not known: it is there when the cache holds files in it
    if ( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.result && m_settings.offlineListings )
    {
        CVfsRemoteEntry::TVector entries;
        bool exists = false;
        ListingFromCacheIndex( path, entries, exists );
        if ( exists )
        {
            Record( CVfsCallResult() );
            return true;
        }
    }
    Record( result );
    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsRemoteArchiveCore::GetFileSize( const CORE::CString& filePath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    CVfsRemoteEntry entry;
    const CVfsCallResult result = Record( LookupEntry( path, entry ) );
    return ( result.IsSuccess() && entry.exists && !entry.isDirectory ) ? entry.size : 0;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::GetFileMetaData( const CORE::CString& filePath, CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    metaData.Clear();

    CVfsRemoteEntry entry;
    const CVfsCallResult result = Record( LookupEntry( path, entry ) );
    if ( !result.IsSuccess() )
        return false;

    metaData.resourceExists = entry.exists;
    if ( !entry.exists )
        return false;           // the call went well and there is nothing to tell: as for a file system, no metadata of what is not there

    metaData.name = CVfsRemotePath::LeafOf( path );
    metaData.hasName = true;
    if ( !entry.isDirectory )
    {
        metaData.resourceSizeInBytes = entry.size;
        metaData.hasResourceSizeInBytes = true;
        metaData.version = entry.revision;
        metaData.hasVersion = true;
    }
    if ( entry.modifiedUtcMs > 0 )
    {
        metaData.modifiedDateTime.FromUnixEpochBasedTicksInMillisecs( entry.modifiedUtcMs );
        metaData.hasModifiedDateTime = true;
    }
    metaData.isReadOnly = true;
    metaData.hasIsReadOnly = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsRemoteArchiveCore::GetFileDigest( const CORE::CString& filePath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    CVfsRemoteEntry entry;
    const CVfsCallResult result = Record( LookupEntry( path, entry ) );
    if ( !result.IsSuccess() || !entry.exists || entry.isDirectory )
        return CVfsDigest();

    // the digest of the remote place, else a hint of the application
    const CVfsDigest known = KnownDigest( path, entry.revision, entry.digest );
    if ( !known.IsEmpty() )
        return known;

    // else the one the cache verified: never a digest that had to be computed for the call
    if ( !m_cache.IsNULL() )
    {
        CVfsBlockCache::SEntryInfo info;
        if ( FindCachedInfo( path, entry, info ) && info.digestVerified )
            return info.digest;
    }
    return CVfsDigest();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemoteArchiveCore::GetFileHash( const CORE::CString& filePath )
{GUCEF_TRACE;

    return GetFileDigest( filePath ).value;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CVfsRemoteArchiveCore::GetFileModificationTime( const CORE::CString& filePath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    CORE::CDateTime modificationTime;
    CVfsRemoteEntry entry;
    const CVfsCallResult result = Record( LookupEntry( path, entry ) );
    if ( result.IsSuccess() && entry.exists && entry.modifiedUtcMs > 0 )
        modificationTime.FromUnixEpochBasedTicksInMillisecs( entry.modifiedUtcMs );
    return modificationTime;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::ListInto( const CORE::CString& dirPath, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive, bool includePath,
                                 const CORE::CString::StringSet& filters, CORE::CString::StringVector& outputList, bool addFiles, bool addDirs, UInt32 maxEntries,
                                 const CORE::CString& relativeDir )
{GUCEF_TRACE;

    CVfsRemoteEntry::TVector entries;
    bool dirExists = false;
    CVfsCallResult flagged;
    const CVfsCallResult listed = LookupListing( dirPath, entries, dirExists );
    if ( !listed.IsSuccess() )
    {
        // the list that is made from the cache while the link is down is used, and flagged; any other failure ends the list
        if ( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == listed.result && !entries.empty() )
            flagged = listed;
        else
            return listed;
    }

    const CORE::CString vfsPath = CORE::CombinePath( mountLocation, archiveLocation );
    for ( size_t i=0; i<entries.size(); ++i )
    {
        if ( outputList.size() >= maxEntries )
            return flagged;

        const CVfsRemoteEntry& entry = entries[ i ];
        if ( !entry.isDirectory )
        {
            if ( addFiles && CVFS::FilterValidation( entry.name, filters ) )
            {
                if ( !includePath )
                    outputList.push_back( entry.name );
                else
                    outputList.push_back( CORE::CombinePath( vfsPath, entry.name ).ReplaceChar( '\\', '/' ) );
            }
            continue;
        }

        if ( addDirs && CVFS::FilterValidation( entry.name, filters ) )
        {
            if ( !includePath )
                outputList.push_back( relativeDir + entry.name + '/' );
            else
                outputList.push_back( CORE::CombinePath( vfsPath, entry.name ).ReplaceChar( '\\', '/' ) + '/' );
        }

        if ( recursive )
        {
            const CVfsCallResult sub = ListInto( CVfsRemotePath::Join( dirPath, entry.name ), mountLocation, CORE::CombinePath( archiveLocation, entry.name ), recursive, includePath,
                                                 filters, outputList, addFiles, addDirs, maxEntries, relativeDir + entry.name + '/' );
            if ( !sub.IsSuccess() )
            {
                // a part of the list that was made from the cache flags the whole list; any other failure ends it
                if ( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING != sub.result )
                    return sub;
                if ( flagged.IsSuccess() )
                    flagged = sub;
            }
        }
    }
    return flagged;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::RunListing( CORE::CString::StringVector& outputList, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive,
                                   bool includePath, const CORE::CString::StringSet& filters, bool addFiles, bool addDirs, UInt32 maxEntries )
{GUCEF_TRACE;

    const CVfsCallResult result = ListInto( CVfsRemotePath::Normalize( archiveLocation ), mountLocation, archiveLocation, recursive, includePath, filters, outputList, addFiles, addDirs, maxEntries, CORE::CString::Empty );
    Record( result );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::GetFileList( CORE::CString::StringVector& outputList     ,
                                    const CORE::CString& mountLocation          ,
                                    const CORE::CString& archiveLocation        ,
                                    bool recursive                              ,
                                    bool includePathInFilename                  ,
                                    const CORE::CString::StringSet& nameFilters ,
                                    UInt32 maxListEntries                       )
{GUCEF_TRACE;

    RunListing( outputList, mountLocation, archiveLocation, recursive, includePathInFilename, nameFilters, true, false, maxListEntries );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::GetDirList( CORE::CString::StringVector& outputList     ,
                                   const CORE::CString& mountLocation          ,
                                   const CORE::CString& archiveLocation        ,
                                   bool recursive                              ,
                                   bool includePathInFilename                  ,
                                   const CORE::CString::StringSet& nameFilters ,
                                   UInt32 maxListEntries                       )
{GUCEF_TRACE;

    RunListing( outputList, mountLocation, archiveLocation, recursive, includePathInFilename, nameFilters, false, true, maxListEntries );
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CVfsRemoteArchiveCore::OpenFile( const CORE::CString& filePath )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );

    CVfsRemoteEntry entry;
    const CVfsCallResult looked = LookupEntry( path, entry );
    if ( !looked.IsSuccess() )
    {
        Record( looked );
        return CORE::IOAccessPtr();
    }
    if ( !entry.exists )
    {
        Record( CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" is not there" ) );
        return CORE::IOAccessPtr();
    }
    if ( entry.isDirectory )
    {
        Record( CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_A_FILE, "\"" + path + "\" is a directory" ) );
        return CORE::IOAccessPtr();
    }

    CVfsRemoteArchiveCorePtr self = m_self.GetSharedPtr();
    if ( self.IsNULL() )
    {
        Record( CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" ) );
        return CORE::IOAccessPtr();
    }

    CVfsBlockCache::TEntryHandle cacheEntry = 0;
    bool staleRevision = false;
    if ( entry.size > 0 )
    {
        if ( !IsLinkUp() )
        {
            // only what the cache holds can be read while the link is down: a file without a copy there is not available
            cacheEntry = AcquireCacheEntry( path, entry, false );
            if ( !m_cache.IsNULL() && ( 0 == cacheEntry || !m_cache->IsComplete( cacheEntry ) ) )
            {
                // a path that is kept offline and has an older complete copy is served from that copy, flagged as old, when its newest revision is not held completely
                CVfsBlockCache::SEntryInfo keptCopy;
                if ( m_cache->FindOfflineRevision( m_identity, path, keptCopy ) && keptCopy.staleRevision )
                {
                    if ( 0 != cacheEntry )
                        m_cache->ReleaseEntry( cacheEntry );
                    entry.size = keptCopy.fileSize;
                    entry.revision = keptCopy.revision;
                    entry.digest = keptCopy.digest;
                    cacheEntry = AcquireCacheEntry( path, entry, false );
                    staleRevision = 0 != cacheEntry;
                }
            }
            if ( 0 != cacheEntry && 0 == m_cache->GetBlocksHeld( cacheEntry ) )
            {
                m_cache->ReleaseEntry( cacheEntry );
                cacheEntry = 0;
            }
            if ( 0 != cacheEntry && !staleRevision )
            {
                // a copy that a newer revision superseded is an older revision whatever way it was found
                CVfsBlockCache::SEntryInfo heldCopy;
                staleRevision = m_cache->GetEntryInfo( cacheEntry, heldCopy ) && heldCopy.superseded;
            }
            if ( staleRevision )
                m_counters.staleServes.FetchAdd( 1 );
            if ( 0 == cacheEntry )
            {
                m_counters.offlineMisses.FetchAdd( 1 );
                Record( CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, DescribeOffline( path, 0, entry.size - 1 ) ) );
                return CORE::IOAccessPtr();
            }
            m_counters.offlineReads.FetchAdd( 1 );
        }
        else
        {
            cacheEntry = AcquireCacheEntry( path, entry, true );
            HealPinnedPath( path );
            const CVfsThreadPolicy policy = GetCurrentThreadPolicy();
            if ( policy.NeverWaits() && policy.openRequiresCompleteFile && ( 0 == cacheEntry || !m_cache->IsComplete( cacheEntry ) ) )
            {
                if ( 0 != cacheEntry )
                    m_cache->ReleaseEntry( cacheEntry );
                m_counters.policyMisses.FetchAdd( 1 );
                if ( m_cache.IsNULL() )
                    Record( CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_SUPPORTED, "there is no block cache to fetch \"" + path + "\" into" ) );
                else
                if ( MISSACTION_FAIL_AND_FETCH == policy.missAction )
                    Record( ScheduleJob( FETCHJOB_WHOLE, path, entry, 0, 0 ) );
                else
                    Record( CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "\"" + path + "\" is not held completely and this thread does not wait for it" ) );
                return CORE::IOAccessPtr();
            }
        }
    }

    Record( CVfsCallResult() );
    CVfsCachedIOAccess* access = GUCEF_NEW CVfsCachedIOAccess( self, path, entry, cacheEntry, CIVfsBlockSourcePtr() );
    access->SetStaleRevision( staleRevision );
    return CORE::IOAccessPtr( access );
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsRemoteArchiveCore::KnownDigest( const CORE::CString& path, const CORE::CString& revision, const CVfsDigest& shareDigest ) const
{GUCEF_TRACE;

    CVfsDigest hint;
    {
        MT::CScopeMutex lock( m_hintLock );
        THintMap::const_iterator i = m_hints.find( path );
        if ( i != m_hints.end() && ( i->second.revision.IsNULLOrEmpty() || i->second.revision == revision ) )
            hint = i->second.digest;
    }

    if ( shareDigest.IsWellFormed() )
    {
        // the digest of the remote place wins for the identity of the content; a hint that disagrees is only logged
        if ( hint.IsWellFormed() && hint.algorithm == shareDigest.algorithm && !hint.Matches( shareDigest ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsRemoteArchiveCore: The digest given for \"" << path << "\" differs from the one of the remote place: the one of the remote place is used" );
        return shareDigest;
    }
    return hint;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::FindCachedInfo( const CORE::CString& path, const CVfsRemoteEntry& entry, CVfsBlockCache::SEntryInfo& info ) const
{GUCEF_TRACE;

    if ( m_cache.IsNULL() )
        return false;

    if ( entry.exists && !entry.isDirectory && entry.size > 0 )
    {
        const CVfsBlockCache::TEntryHandle handle = m_cache->AcquireEntry( m_identity, path, entry.revision, entry.size, CVfsDigest(), false );
        if ( 0 != handle )
        {
            const bool found = m_cache->GetEntryInfo( handle, info );
            m_cache->ReleaseEntry( handle );
            return found;
        }
        return false;
    }
    return m_cache->FindNewestRevision( m_identity, path, info );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::VerifyIfComplete( CVfsBlockCache::TEntryHandle cacheEntry, const CORE::CString& path, const CORE::CString& revision, const CVfsDigest& shareDigest )
{GUCEF_TRACE;

    if ( m_cache.IsNULL() || 0 == cacheEntry || !m_settings.verifyCompleteFiles )
        return CVfsCallResult();

    CVfsBlockCache::SEntryInfo info;
    if ( !m_cache->GetEntryInfo( cacheEntry, info ) || !info.complete || info.digestVerified )
        return CVfsCallResult();

    const CVfsDigest expected = KnownDigest( path, revision, shareDigest.IsWellFormed() ? shareDigest : info.digest );
    if ( !expected.IsWellFormed() )
        return CVfsCallResult();

    switch ( m_cache->VerifyDigest( cacheEntry, expected ) )
    {
        case CVfsBlockCache::VERIFY_MATCH:
        {
            m_counters.filesVerified.FetchAdd( 1 );
            return CVfsCallResult();
        }
        case CVfsBlockCache::VERIFY_MISMATCH:
        {
            m_counters.digestMismatches.FetchAdd( 1 );
            m_cache->ResetEntry( cacheEntry );
            return CVfsCallResult( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT, "\"" + path + "\": the content does not have the digest " + CORE::CString( CVfsDigest::GetAlgorithmName( expected.algorithm ) ) + " " + expected.value +
                                   ( shareDigest.IsWellFormed() && shareDigest.Matches( expected ) ? "" : " (given by the application)" ) );
        }
        default:
        {
            return CVfsCallResult();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::OnSourceChanged( const CORE::CString& path, const CORE::CString& revision )
{GUCEF_TRACE;

    // what is known of the path is old: the next call asks again, and every copy in the cache is superseded until the new revision is learned
    m_metadata.Invalidate( path );
    if ( !m_cache.IsNULL() )
        m_cache->SupersedeOtherRevisions( m_identity, path, CORE::CString( "\n" ) );
    RaiseFileChanged( path, revision, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CVfsRemoteArchiveCore::EMetadataState
CVfsRemoteArchiveCore::GetMetadataState( const CORE::CString& path ) const
{GUCEF_TRACE;

    switch ( m_metadata.GetState( CVfsRemotePath::Normalize( path ) ) )
    {
        case CVfsRemoteMetadataCache::STATE_FRESH: return METADATA_FRESH;
        case CVfsRemoteMetadataCache::STATE_STALE: return METADATA_STALE;
        default: return METADATA_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::InvalidateMetadata( void )
{GUCEF_TRACE;

    m_metadata.Clear();
    if ( !m_backend.IsNULL() )
        m_backend->InvalidateMetadata();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::InvalidateMetadata( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString normalized = CVfsRemotePath::Normalize( path );
    m_metadata.Invalidate( normalized );
    if ( !m_backend.IsNULL() )
        m_backend->InvalidateMetadata( normalized );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::NoteFileDigest( const CORE::CString& filePath, const CVfsDigest& digest, const CORE::CString& revision )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    {
        MT::CScopeMutex lock( m_hintLock );
        if ( m_hints.find( path ) == m_hints.end() && m_settings.maxDigestHints > 0 && m_hints.size() >= m_settings.maxDigestHints )
        {
            // the oldest hint goes
            THintMap::iterator oldest = m_hints.begin();
            for ( THintMap::iterator i = m_hints.begin(); i != m_hints.end(); ++i )
            {
                if ( i->second.sequence < oldest->second.sequence )
                    oldest = i;
            }
            m_hints.erase( oldest );
        }
        SHint& hint = m_hints[ path ];
        hint.digest = digest;
        hint.revision = revision;
        hint.sequence = ++m_hintSequence;
    }

    // a complete copy in the cache is checked against it once
    if ( m_cache.IsNULL() || !digest.IsWellFormed() )
        return;

    CVfsRemoteEntry entry;
    m_metadata.FindEntry( path, entry );
    CVfsBlockCache::SEntryInfo info;
    CVfsRemoteEntry cached;
    if ( !entry.exists || entry.isDirectory )
    {
        if ( !EntryFromCacheIndex( path, cached ) )
            return;
        entry = cached;
    }
    if ( !revision.IsNULLOrEmpty() && revision != entry.revision )
        return;

    const CVfsBlockCache::TEntryHandle handle = m_cache->AcquireEntry( m_identity, path, entry.revision, entry.size, CVfsDigest(), false );
    if ( 0 == handle )
        return;
    if ( m_cache->IsComplete( handle ) )
    {
        switch ( m_cache->VerifyDigest( handle, digest ) )
        {
            case CVfsBlockCache::VERIFY_MATCH:
            {
                m_counters.filesVerified.FetchAdd( 1 );
                break;
            }
            case CVfsBlockCache::VERIFY_MISMATCH:
            {
                m_counters.digestMismatches.FetchAdd( 1 );
                m_cache->ResetEntry( handle );
                Record( CVfsCallResult( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT, "\"" + path + "\": the copy in the cache does not have the digest given by the application" ) );
                break;
            }
            default: break;
        }
    }
    m_cache->ReleaseEntry( handle );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::ForgetFileDigests( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_hintLock );
    m_hints.clear();
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::GetFileCacheState( const CORE::CString& filePath, CVfsCacheState& state ) const
{GUCEF_TRACE;

    state = CVfsCacheState();
    if ( m_cache.IsNULL() )
        return false;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    CVfsRemoteEntry entry;
    m_metadata.FindEntry( path, entry );

    CVfsBlockCache::SEntryInfo info;
    bool found = FindCachedInfo( path, entry, info );
    bool staleRevision = false;
    if ( !found || !info.complete )
    {
        // the file is kept offline and the copy that is held completely is an older revision: that is the copy the state is about, flagged
        CVfsBlockCache::SEntryInfo keptCopy;
        if ( m_cache->FindOfflineRevision( m_identity, path, keptCopy ) && keptCopy.staleRevision )
        {
            info = keptCopy;
            staleRevision = true;
            found = true;
        }
    }
    if ( !found )
        return false;

    state.cached = info.blocksHeld > 0;
    state.complete = info.complete;
    state.pinned = m_cache->IsPinned( m_identity, path ) || info.pinned;
    state.superseded = info.superseded;
    state.staleRevision = staleRevision;
    state.digestVerified = info.digestVerified;
    state.fileSize = info.fileSize;
    state.bytesHeld = info.bytesHeld;
    state.revision = info.revision;
    state.digest = info.digest;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteArchiveCore::SetFilePinned( const CORE::CString& filePath, bool pinned )
{GUCEF_TRACE;

    if ( m_cache.IsNULL() )
        return false;
    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    if ( !pinned )
        return m_cache->SetPinned( m_identity, path, false );

    const CVfsBlockCache::EPinResult pin = m_cache->TryPin( m_identity, path );
    if ( CVfsBlockCache::PIN_OK == pin )
        return true;
    Record( DescribePinRefusal( path, pin ) );
    return false;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::FetchWhole( const CORE::CString& path, bool pin, bool verifyDigest, CVfsFetchProgress* progress, const MT::CTAtomic< bool >* cancelRequested, CVfsArrivalInfo& outcome )
{GUCEF_TRACE;

    outcome = CVfsArrivalInfo( CVfsArrivalInfo::KIND_FILE, path );
    outcome.jobKind = FETCHJOB_WHOLE;

    if ( m_shutdown.Load() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the archive is unloaded" );
    if ( !IsLinkUp() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "\"" + path + "\": " + m_backend->GetPlaceDescription() + " cannot be reached" );
    if ( m_cache.IsNULL() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_SUPPORTED, "there is no block cache to fetch \"" + path + "\" into" );

    // what the remote place says of the file now: a fresh record is taken as it is, else it is asked
    CVfsRemoteEntry entry;
    if ( CVfsRemoteMetadataCache::STATE_FRESH != m_metadata.FindEntry( path, entry ) )
    {
        bool changed = false, moved = false;
        CORE::CString oldRevision;
        const CVfsCallResult refreshed = RefreshEntry( path, entry, changed, moved, oldRevision, false );
        if ( !refreshed.IsSuccess() )
            return refreshed;
        if ( moved )
            RaiseFileChanged( path, oldRevision, entry.revision );
    }
    if ( !entry.exists )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" is not there" );
    if ( entry.isDirectory )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_A_FILE, "\"" + path + "\" is a directory" );

    outcome.revision = entry.revision;
    outcome.firstByte = 0;
    outcome.byteCount = entry.size;
    if ( GUCEF_NULL != progress )
        progress->bytesTotal.FetchAdd( entry.size );

    if ( 0 == entry.size )
    {
        outcome.fileComplete = true;
        return CVfsCallResult();
    }

    const CVfsBlockCache::TEntryHandle handle = AcquireCacheEntry( path, entry, true );
    if ( 0 == handle )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the block cache has no entry for \"" + path + "\"" );

    CVfsCallResult result;
    CVfsCallResult openResult;
    CIVfsBlockSourcePtr source;
    const UInt32 blockCount = m_cache->GetBlockCount( handle );
    const UInt32 maxBlocks = m_settings.maxBlocksPerFetch < 1 ? 1 : m_settings.maxBlocksPerFetch;

    for ( UInt32 attempt = 0; attempt < 2; ++attempt )
    {
        UInt32 block = 0;
        while ( block < blockCount && result.IsSuccess() )
        {
            if ( ( GUCEF_NULL != progress && progress->IsCancelRequested() ) || ( GUCEF_NULL != cancelRequested && cancelRequested->Load() ) )
            {
                result = CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the fetch of \"" + path + "\" was cancelled" );
                break;
            }
            if ( m_cache->HasBlock( handle, block ) )
            {
                ++block;
                continue;
            }

            const UInt32 firstOfRun = block;
            UInt32 count = 0;
            while ( block < blockCount && count < maxBlocks && !m_cache->HasBlock( handle, block ) )
            {
                ++block;
                ++count;
            }

            if ( source.IsNULL() )
            {
                source = OpenSource( path, entry.revision, openResult );
                if ( source.IsNULL() )
                {
                    result = openResult;
                    if ( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == result.result )
                        OnSourceChanged( path, entry.revision );
                    break;
                }
            }

            CVfsBlockRunFetcher::SRun run;
            result = CVfsBlockRunFetcher::FetchRun( m_cache.GetPointerAlways(), handle, m_cache->GetBlockBytes(), entry.size, *source.GetPointerAlways(), firstOfRun, count, &m_counters, run );
            if ( GUCEF_NULL != progress && run.bytesStored > 0 )
                progress->bytesDone.FetchAdd( run.bytesStored );
            if ( !result.IsSuccess() )
            {
                if ( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == result.result )
                    OnSourceChanged( path, entry.revision );
                break;
            }
            if ( run.blocksNotKept > 0 )
                result = CVfsCallResult( CORE::FILESYSTEMACCESS_NO_SPACE, "the block cache could not keep the blocks of \"" + path + "\"" );
        }

        if ( !result.IsSuccess() )
            break;

        // complete: checked against the digest when one is known, and fetched again once when it does not match
        if ( verifyDigest )
        {
            const CVfsCallResult verified = VerifyIfComplete( handle, path, entry.revision, entry.digest );
            if ( !verified.IsSuccess() )
            {
                result = verified;
                if ( 0 == attempt )
                {
                    result = CVfsCallResult();
                    continue;
                }
            }
        }
        break;
    }

    if ( result.IsSuccess() )
    {
        outcome.fileComplete = true;
        if ( pin )
        {
            // a pin that does not fit the limit is refused and says so: the file stays in the cache as any other, nothing that is kept is dropped for it
            const CVfsBlockCache::EPinResult pinned = m_cache->TryPin( m_identity, path );
            if ( CVfsBlockCache::PIN_OK != pinned )
                result = DescribePinRefusal( path, pinned );
        }
    }
    m_cache->ReleaseEntry( handle );
    return result;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::FetchFile( const CORE::CString& filePath, const CVfsFetchRequest& options, CVfsFetchProgress& progress )
{GUCEF_TRACE;

    const CORE::CString path = CVfsRemotePath::Normalize( filePath );
    CVfsArrivalInfo outcome;

    // the file is counted once, whatever comes of it: the bytes are counted as the fetch learns of them
    progress.filesTotal.FetchAdd( 1 );
    const CVfsCallResult result = Record( FetchWhole( path, options.pin, options.verifyDigest, &progress, GUCEF_NULL, outcome ) );
    if ( result.IsSuccess() )
    {
        progress.filesDone.FetchAdd( 1 );
    }
    else
    {
        progress.filesFailed.FetchAdd( 1 );
        progress.SetLastFailure( result );
    }

    // a fetch of a whole file ends with exactly one event, as a background job of the same work does
    RaiseEndEvent( FETCHJOB_WHOLE, path, result, outcome, false );
    return result;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::FetchDirectory( const CORE::CString& dirPath, bool recursive, UInt32 maxDirectories, CVfsFetchProgress& progress )
{GUCEF_TRACE;

    CORE::CString::StringVector pending;
    pending.push_back( CVfsRemotePath::Normalize( dirPath ) );
    progress.filesTotal.FetchAdd( 1 );

    UInt32 visited = 0;
    size_t next = 0;
    while ( next < pending.size() )
    {
        if ( progress.IsCancelRequested() )
            return Record( CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the fetch of the directory \"" + dirPath + "\" was cancelled" ) );
        if ( visited >= maxDirectories )
            break;

        const CORE::CString dir = pending[ next++ ];
        ++visited;
        if ( !IsLinkUp() )
        {
            const CVfsCallResult down( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, "\"" + dir + "\": " + m_backend->GetPlaceDescription() + " cannot be reached" );
            progress.filesFailed.FetchAdd( 1 );
            progress.SetLastFailure( down );
            return Record( down );
        }

        CVfsRemoteEntry::TVector entries;
        bool exists = false, changed = false;
        const CVfsCallResult listed = RefreshListing( dir, entries, exists, changed );
        if ( !listed.IsSuccess() )
        {
            progress.filesFailed.FetchAdd( 1 );
            progress.SetLastFailure( listed );
            return Record( listed );
        }
        progress.filesDone.FetchAdd( 1 );

        if ( recursive )
        {
            for ( size_t i=0; i<entries.size(); ++i )
            {
                if ( entries[ i ].isDirectory )
                {
                    pending.push_back( CVfsRemotePath::Join( dir, entries[ i ].name ) );
                    progress.filesTotal.FetchAdd( 1 );
                }
            }
        }
    }
    return Record( CVfsCallResult() );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::GetRemoteStatistics( CVfsRemoteStatistics& statistics ) const
{GUCEF_TRACE;

    statistics.Reset();
    m_counters.Fill( statistics );

    if ( !m_cache.IsNULL() )
    {
        CVfsBlockCache::SStatistics cache;
        m_cache->GetStatistics( cache );
        statistics.maxBytes = cache.maxBytes;
        statistics.usedBytes = cache.usedBytes;
        statistics.pinnedBytes = cache.pinnedBytes;
        statistics.entries = cache.entries;
        statistics.pinnedEntries = cache.pinnedEntries;
        statistics.blockHits = cache.blockHits;
        statistics.blockMisses = cache.blockMisses;
        statistics.blocksStored = cache.blocksStored;
        statistics.corruptBlocks = cache.corruptBlocks;
        statistics.evictedEntries = cache.evictedEntries;
        statistics.evictedBytes = cache.evictedBytes;
        statistics.supersededEvictions = cache.supersededEvictions;
        statistics.pinsRefused = cache.pinsRefused;
        statistics.storeFailures = cache.storeFailures;
        statistics.openFiles = cache.openFiles;
        statistics.fileReopens = cache.fileReopens;
    }
    if ( !m_backend.IsNULL() )
        m_backend->GetBackendStatistics( statistics );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteArchiveCore::RunJob( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount,
                               const MT::CTAtomic< bool >& cancelRequested, CVfsArrivalInfo& outcome, bool& silent )
{GUCEF_TRACE;

    silent = false;
    outcome = CVfsArrivalInfo( CVfsArrivalInfo::KIND_METADATA, path );
    outcome.jobKind = kind;

    if ( m_shutdown.Load() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the archive is unloaded" );
    if ( cancelRequested.Load() )
        return CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "cancelled before it ran" );

    switch ( kind )
    {
        case FETCHJOB_STAT:
        case FETCHJOB_REVALIDATE:
        {
            CVfsRemoteEntry fresh;
            bool changed = false, moved = false;
            CORE::CString oldRevision;
            const CVfsCallResult result = RefreshEntry( path, fresh, changed, moved, oldRevision );
            if ( !result.IsSuccess() )
                return result;
            outcome.revision = fresh.revision;
            outcome.oldRevision = oldRevision;
            // a check of metadata that was answered already says nothing when nothing changed
            silent = FETCHJOB_REVALIDATE == kind && !changed;
            return result;
        }
        case FETCHJOB_LIST:
        {
            CVfsRemoteEntry::TVector entries;
            bool dirExists = false, changed = false;
            const CVfsCallResult result = RefreshListing( path, entries, dirExists, changed );
            if ( !result.IsSuccess() )
                return result;
            outcome.kind = CVfsArrivalInfo::KIND_LISTING;
            silent = !changed;
            return result;
        }
        case FETCHJOB_BLOCKS:
        {
            outcome.kind = CVfsArrivalInfo::KIND_FILE;
            outcome.revision = entry.revision;
            if ( m_cache.IsNULL() )
                return CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_SUPPORTED, "there is no block cache to fetch \"" + path + "\" into" );

            const CVfsBlockCache::TEntryHandle handle = AcquireCacheEntry( path, entry, true );
            if ( 0 == handle )
                return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the block cache has no entry for \"" + path + "\"" );

            CVfsCallResult result;
            CIVfsBlockSourcePtr source = OpenSource( path, entry.revision, result );
            if ( !source.IsNULL() )
            {
                CVfsBlockRunFetcher::SRun run;
                result = CVfsBlockRunFetcher::FetchRun( m_cache.GetPointerAlways(), handle, m_cache->GetBlockBytes(), entry.size, *source.GetPointerAlways(), firstBlock, blockCount, &m_counters, run );
                outcome.firstByte = run.firstByte;
                outcome.byteCount = run.data.GetDataSize();
                if ( result.IsSuccess() && run.blocksNotKept > 0 )
                    result = CVfsCallResult( CORE::FILESYSTEMACCESS_NO_SPACE, "the block cache could not keep the blocks of \"" + path + "\"" );
                if ( result.IsSuccess() )
                {
                    outcome.fileComplete = m_cache->IsComplete( handle );
                    if ( outcome.fileComplete )
                        result = VerifyIfComplete( handle, path, entry.revision, entry.digest );
                }
            }
            if ( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == result.result )
                OnSourceChanged( path, entry.revision );
            m_cache->ReleaseEntry( handle );
            return result;
        }
        case FETCHJOB_WHOLE:
        {
            return FetchWhole( path, false, true, GUCEF_NULL, &cancelRequested, outcome );
        }
        default:
        {
            return CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "unknown job" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::OnJobEnded( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool silent )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;

    if ( !result.IsSuccess() )
    {
        if ( CORE::FILESYSTEMACCESS_CANCELLED == result.result )
            m_counters.fetchJobsCancelled.FetchAdd( 1 );
        else
            m_counters.fetchJobsFailed.FetchAdd( 1 );
    }
    RaiseEndEvent( kind, path, result, outcome, silent );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteArchiveCore::RaiseEndEvent( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool silent )
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;

    if ( !result.IsSuccess() )
    {
        CVfsArrivalInfo info( CVfsArrivalInfo::KIND_FAILURE, path );
        info.result = result.result;
        info.message = result.message;
        info.jobKind = kind;
        info.revision = outcome.revision;
        RaiseArrival( CRemoteArchiveEvents::FetchFailedEvent, info );
        return;
    }

    if ( silent )
        return;

    CVfsArrivalInfo info( outcome );
    info.path = path;
    info.jobKind = kind;
    switch ( kind )
    {
        case FETCHJOB_BLOCKS:
        case FETCHJOB_WHOLE:
        {
            info.kind = CVfsArrivalInfo::KIND_FILE;
            RaiseArrival( CRemoteArchiveEvents::FileArrivedEvent, info );
            break;
        }
        case FETCHJOB_LIST:
        {
            info.kind = CVfsArrivalInfo::KIND_LISTING;
            RaiseArrival( CRemoteArchiveEvents::ListingArrivedEvent, info );
            break;
        }
        default:
        {
            info.kind = CVfsArrivalInfo::KIND_METADATA;
            RaiseArrival( CRemoteArchiveEvents::MetadataArrivedEvent, info );
            break;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
