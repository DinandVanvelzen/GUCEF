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

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_VFS_CIREMOTEARCHIVE_H
#include "gucefVFS_CIRemoteArchive.h"
#define GUCEF_VFS_CIREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CIREMOTEARCHIVE_H ? */

#ifndef GUCEF_VFS_CIVFSREMOTEBACKEND_H
#include "gucefVFS_CIVfsRemoteBackend.h"
#define GUCEF_VFS_CIVFSREMOTEBACKEND_H
#endif /* GUCEF_VFS_CIVFSREMOTEBACKEND_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESETTINGS_H
#include "gucefVFS_CVfsRemoteSettings.h"
#define GUCEF_VFS_CVFSREMOTESETTINGS_H
#endif /* GUCEF_VFS_CVFSREMOTESETTINGS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTECOUNTERS_H
#include "gucefVFS_CVfsRemoteCounters.h"
#define GUCEF_VFS_CVFSREMOTECOUNTERS_H
#endif /* GUCEF_VFS_CVFSREMOTECOUNTERS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEMETADATACACHE_H
#include "gucefVFS_CVfsRemoteMetadataCache.h"
#define GUCEF_VFS_CVFSREMOTEMETADATACACHE_H
#endif /* GUCEF_VFS_CVFSREMOTEMETADATACACHE_H ? */

#ifndef GUCEF_VFS_CVFSLASTRESULTS_H
#include "gucefVFS_CVfsLastResults.h"
#define GUCEF_VFS_CVFSLASTRESULTS_H
#endif /* GUCEF_VFS_CVFSLASTRESULTS_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#include "gucefVFS_CVfsThreadPolicyRegistry.h"
#define GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#include "gucefVFS_CVfsFetchScheduler.h"
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H
#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */

#ifndef GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#include "gucefVFS_CRemoteArchiveEvents.h"
#define GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#endif /* GUCEF_VFS_CREMOTEARCHIVEEVENTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The behaviour that every remote or slow archive shares, on top of what the place itself can do (CIVfsRemoteBackend): the
 *  answers of the archive interface from what is known in memory (CVfsRemoteMetadataCache) and from the persistent block cache,
 *  the rule of the calling thread for a miss (CVfsThreadPolicy), background fetches that end in an event
 *  (CVfsFetchScheduler), reading through the cache with read ahead (CVfsCachedIOAccess), serving what is cached while the link is
 *  down, keeping files offline, digest hints, whole file fetches and statistics. It implements CIRemoteArchive.
 *
 *  An archive composes it (CVfsRemoteArchive does) or calls it from its own CArchive methods. The core is a notifier: it raises
 *  the events of CRemoteArchiveEvents, and the archive subscribes to it and raises them again as its own, which the VFS
 *  passes on. It is shared by pointer so that open files keep it alive after the archive is unloaded: they then fail.
 *
 *  What a call does, by the state of the thread and the link (paths are relative to the share):
 *
 *  - fresh metadata answers. Stale metadata answers to a thread that waits only after a new look at the remote place, to a
 *    thread with FAIL_AND_FETCH at once (and a background check, unless the thread said it does not want stale answers), and
 *    always while the link is down;
 *  - unknown metadata: a thread that waits asks the remote place; FAIL_AND_FETCH fails with NOT_YET_AVAILABLE and starts a
 *    background job; FAIL fails with NOT_YET_AVAILABLE; while the link is down the block cache index answers for the files
 *    it holds, and everything else fails with CONNECTION_PENDING (no job is started);
 *  - the same for the listing of a directory, which while the link is down is made from what the block cache holds, and
 *    flagged as such by the result and message;
 *  - opening a file needs its size and revision; the access (CVfsCachedIOAccess) is pinned to that revision.
 *
 *  The result of every call is kept per calling thread (GetLastResult()). A job runs the same code a thread that waits does.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteArchiveCore : public CORE::CNotifier ,
                                                   public CIRemoteArchive
{
    public:

    /**
     *  Makes a core; it is always held by a shared pointer, which the open files of the archive share.
     *  @param backend      the remote place
     *  @param cache        the block cache; empty for none: nothing is kept, threads that must not wait cannot read bytes
     *  @param settings     the settings; cacheIdentity names the place in the cache, empty makes it "default"
     *  @param scheduler    runs the background jobs; NULL uses the one of the VFS (CVfsGlobal)
     *  @param policies     the rules of the threads; NULL uses the registry of the VFS
     */
    static CORE::CTSharedPtr< CVfsRemoteArchiveCore, MT::CMutex > Create( const CIVfsRemoteBackendPtr& backend         ,
                                                                          const CVfsBlockCachePtr& cache               ,
                                                                          const CVfsRemoteSettings& settings           ,
                                                                          CVfsFetchScheduler* scheduler = GUCEF_NULL   ,
                                                                          CVfsThreadPolicyRegistry* policies = GUCEF_NULL );

    virtual ~CVfsRemoteArchiveCore() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Ends the use of the archive: its background jobs are dropped (nothing is raised afterwards), no event is raised any more
     *  and the calls and open files fail. Done by the archive at unload; the destructor does it too.
     */
    void Shutdown( void );

    bool IsShutdown( void ) const;

    /** Whether the remote place can be reached now */
    bool IsLinkUp( void ) const;

    /** What a backend calls when its link comes or goes: ConnectionStateChangedEvent is raised when the state is not what was last told */
    void NotifyLinkState( bool linkUp );

    const CORE::CString& GetIdentity( void ) const;

    const CVfsRemoteSettings& GetSettings( void ) const;

    CVfsBlockCachePtr GetBlockCache( void ) const;

    CVfsRemoteCounters& GetCounters( void );

    CVfsFetchScheduler& GetScheduler( void );

    /** The rule of the calling thread */
    CVfsThreadPolicy GetCurrentThreadPolicy( void ) const;

    // the calls of the archive interface

    bool FileExists( const CORE::CString& path );

    bool DirExists( const CORE::CString& path );

    UInt64 GetFileSize( const CORE::CString& path );

    bool GetFileMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData );

    CORE::CString GetFileHash( const CORE::CString& path );

    /** The digest the remote place knows, else a hint of the application, else the one the cache verified; empty when there is none. Nothing is computed */
    CVfsDigest GetFileDigest( const CORE::CString& path );

    CORE::CDateTime GetFileModificationTime( const CORE::CString& path );

    /** The listing calls of CArchive, with the same output: the result of the call tells when the list is not complete */
    void GetFileList( CORE::CString::StringVector& outputList      ,
                      const CORE::CString& mountLocation           ,
                      const CORE::CString& archiveLocation         ,
                      bool recursive                               ,
                      bool includePathInFilename                   ,
                      const CORE::CString::StringSet& nameFilters  ,
                      UInt32 maxListEntries                        );

    void GetDirList( CORE::CString::StringVector& outputList       ,
                     const CORE::CString& mountLocation            ,
                     const CORE::CString& archiveLocation          ,
                     bool recursive                                ,
                     bool includePathInFilename                    ,
                     const CORE::CString::StringSet& nameFilters   ,
                     UInt32 maxListEntries                         );

    /** Opens a file for reading; an empty pointer when it cannot be opened, GetLastResult() says why */
    CORE::IOAccessPtr OpenFile( const CORE::CString& path );

    // CIRemoteArchive

    virtual CVfsCallResult GetLastResult( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual EMetadataState GetMetadataState( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void InvalidateMetadata( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void InvalidateMetadata( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void NoteFileDigest( const CORE::CString& path, const CVfsDigest& digest, const CORE::CString& revision = CORE::CString::Empty ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ForgetFileDigests( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetFileCacheState( const CORE::CString& path, CVfsCacheState& state ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetFilePinned( const CORE::CString& path, bool pinned ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CVfsCallResult FetchFile( const CORE::CString& path, const CVfsFetchRequest& options, CVfsFetchProgress& progress ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CVfsCallResult FetchDirectory( const CORE::CString& dir, bool recursive, UInt32 maxDirectories, CVfsFetchProgress& progress ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetRemoteStatistics( CVfsRemoteStatistics& statistics ) const GUCEF_VIRTUAL_OVERRIDE;

    /** For an archive that handled a call without the layer (a file it opened by itself): the result of that call becomes the one of the calling thread, as the calls of the layer do */
    void RecordCallResult( const CVfsCallResult& result );

    // for the cached access and the jobs of the core

    /** Opens a source of the file at the revision (the remote place is asked) */
    CIVfsBlockSourcePtr OpenSource( const CORE::CString& path, const CORE::CString& revision, CVfsCallResult& result );

    /** Starts the background fetch of a run of blocks of a file (or joins the one under way); the result is what the call that missed has to report */
    CVfsCallResult ScheduleBlocks( const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount );

    /** When the entry is complete and a digest is known and not verified yet: checks it. The digest is the one the remote place gave for the file ('shareDigest', empty for none), else a hint of the application, else the one the entry has. A mismatch empties the entry and gives RESOURCE_CORRUPT */
    CVfsCallResult VerifyIfComplete( CVfsBlockCache::TEntryHandle cacheEntry, const CORE::CString& path, const CORE::CString& revision, const CVfsDigest& shareDigest );

    /** An open file found that the remote place has another revision: what is known of the path is dropped and the other revisions in the cache are superseded; FileChangedEvent is raised */
    void OnSourceChanged( const CORE::CString& path, const CORE::CString& revision );

    /** "path": bytes a-b are not in the cache of this computer and the place cannot be reached */
    CORE::CString DescribeOffline( const CORE::CString& path, UInt64 firstByte, UInt64 lastByte ) const;

    /** Does the work of a background job on the calling (pool) thread. 'silent': the job found nothing to report */
    CVfsCallResult RunJob( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount, const MT::CTAtomic< bool >& cancelRequested, CVfsArrivalInfo& outcome, bool& silent );

    /** Raises the event that ends a background job: of the kind of the job when it succeeded, FetchFailedEvent when it did not */
    void OnJobEnded( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool silent );

    private:

    CVfsRemoteArchiveCore( const CIVfsRemoteBackendPtr& backend           ,
                           const CVfsBlockCachePtr& cache                 ,
                           const CVfsRemoteSettings& settings             ,
                           CVfsFetchScheduler* scheduler                  ,
                           CVfsThreadPolicyRegistry* policies             );

    CVfsRemoteArchiveCore( const CVfsRemoteArchiveCore& src );                  /**< not implemented */
    CVfsRemoteArchiveCore& operator=( const CVfsRemoteArchiveCore& src );       /**< not implemented */

    struct SHint
    {
        CVfsDigest digest;
        CORE::CString revision;
        UInt64 sequence;
        SHint( void ) : digest(), revision(), sequence( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, SHint > THintMap;

    CVfsCallResult Record( const CVfsCallResult& result ) const;
    CVfsCallResult LookupEntry( const CORE::CString& path, CVfsRemoteEntry& entry );
    CVfsCallResult LookupListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists );
    CVfsCallResult RefreshEntry( const CORE::CString& path, CVfsRemoteEntry& fresh, bool& changed, bool& revisionMoved, CORE::CString& oldRevision, bool healPinned = true );
    CVfsCallResult RefreshListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists, bool& changed );
    bool EntryFromCacheIndex( const CORE::CString& path, CVfsRemoteEntry& entry, bool forOfflineServing = false ) const;
    void HealPinnedPath( const CORE::CString& path );
    void HealPinnedPaths( void );
    void ListingFromCacheIndex( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists ) const;
    CVfsCallResult ScheduleJob( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsRemoteEntry& entry, UInt32 firstBlock, UInt32 blockCount );
    void RaiseArrival( const CORE::CEvent& eventId, const CVfsArrivalInfo& info );
    void RaiseFileChanged( const CORE::CString& path, const CORE::CString& oldRevision, const CORE::CString& newRevision );
    void RaiseEndEvent( EVfsFetchJobKind kind, const CORE::CString& path, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool silent );
    CVfsDigest KnownDigest( const CORE::CString& path, const CORE::CString& revision, const CVfsDigest& shareDigest ) const;
    CVfsBlockCache::TEntryHandle AcquireCacheEntry( const CORE::CString& path, const CVfsRemoteEntry& entry, bool create ) const;
    bool FindCachedInfo( const CORE::CString& path, const CVfsRemoteEntry& entry, CVfsBlockCache::SEntryInfo& info ) const;
    CVfsCallResult FetchWhole( const CORE::CString& path, bool pin, bool verifyDigest, CVfsFetchProgress* progress, const MT::CTAtomic< bool >* cancelRequested, CVfsArrivalInfo& outcome );
    CVfsCallResult ListInto( const CORE::CString& dirPath, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive, bool includePath,
                             const CORE::CString::StringSet& filters, CORE::CString::StringVector& outputList, bool addFiles, bool addDirs, UInt32 maxEntries,
                             const CORE::CString& relativeDir );
    void RunListing( CORE::CString::StringVector& outputList, const CORE::CString& mountLocation, const CORE::CString& archiveLocation, bool recursive,
                     bool includePath, const CORE::CString::StringSet& filters, bool addFiles, bool addDirs, UInt32 maxEntries );

    private:

    CIVfsRemoteBackendPtr m_backend;
    mutable CVfsBlockCachePtr m_cache;                  /**< the cache is shared and has its own lock: a const call of the archive may use it */
    CVfsRemoteSettings m_settings;
    CORE::CString m_identity;
    CVfsFetchScheduler* m_scheduler;
    CVfsThreadPolicyRegistry* m_policies;
    CVfsRemoteMetadataCache m_metadata;
    CVfsRemoteCounters m_counters;
    CVfsLastResults m_lastResults;
    MT::CMutex m_hintLock;
    THintMap m_hints;
    UInt64 m_hintSequence;
    MT::CTAtomic< bool > m_shutdown;
    MT::CTAtomic< bool > m_linkWasUp;
    CORE::CTWeakPtr< CVfsRemoteArchiveCore, MT::CMutex > m_self;
};

typedef CORE::CTSharedPtr< CVfsRemoteArchiveCore, MT::CMutex > CVfsRemoteArchiveCorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */
