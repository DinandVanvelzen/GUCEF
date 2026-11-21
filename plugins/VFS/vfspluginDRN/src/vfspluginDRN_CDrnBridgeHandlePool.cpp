/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#include "vfspluginDRN_CDrnBridgeFileAccess.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H ? */

#include "vfspluginDRN_CDrnBridgeHandlePool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeHandlePool::CDrnBridgeHandlePool( const CDrnBridgeConnectionPtr& connection   ,
                                            const CORE::CString& share                  ,
                                            const CDrnBridgeSettings& settings          ,
                                            const CDrnBridgeMetadataCachePtr& cache     ,
                                            const CDrnBridgeLastResultsPtr& lastResults )
    : m_connection( connection )
    , m_share( share )
    , m_settings( settings )
    , m_cache( cache )
    , m_lastResults( lastResults )
    , m_lock()
    , m_accesses()
    , m_active( 0 )
    , m_peak( 0 )
    , m_useCounter( 0 )
    , m_evictions( 0 )
    , m_reopens( 0 )
    , m_limit( settings.maxOpenHandles )
    , m_registered( false )
    , m_shutdown( false )
    , m_changeListener( GUCEF_NULL )
{GUCEF_TRACE;

    if ( !m_connection.IsNULL() )
    {
        m_connection->RegisterHandleUser( this );
        m_registered = true;
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeHandlePool::~CDrnBridgeHandlePool()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::DeleteInners( TInnerVector& inners )
{GUCEF_TRACE;

    // closing a handle sends a message to the serving side: never done under the lock
    for ( TInnerVector::iterator i = inners.begin(); i != inners.end(); ++i )
        GUCEF_DELETE (*i);
    inners.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeHandlePool::GetShare( void ) const
{GUCEF_TRACE;

    return m_share;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeHandlePool::IsShutdown( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_shutdown;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeHandlePool::GetOpenHandleCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_active;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeHandlePool::GetPeakOpenHandleCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_peak;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeHandlePool::GetEvictionCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_evictions;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeHandlePool::GetReopenCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_reopens;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::Attach( CDrnBridgeFileAccess& access )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_accesses.push_back( &access );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::Detach( CDrnBridgeFileAccess& access )
{GUCEF_TRACE;

    TInnerVector inners;
    {
        MT::CScopeMutex lock( m_lock );
        for ( TAccessVector::iterator i = m_accesses.begin(); i != m_accesses.end(); ++i )
        {
            if ( (*i) == &access )
            {
                m_accesses.erase( i );
                break;
            }
        }
        if ( GUCEF_NULL != access.m_inner )
        {
            inners.push_back( access.m_inner );
            access.m_inner = GUCEF_NULL;
            --m_active;
        }
    }
    DeleteInners( inners );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::DropHandle( CDrnBridgeFileAccess& access )
{GUCEF_TRACE;

    TInnerVector inners;
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL != access.m_inner && !access.m_busy )
        {
            inners.push_back( access.m_inner );
            access.m_inner = GUCEF_NULL;
            --m_active;
        }
    }
    DeleteInners( inners );
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeHandlePool::GetHandleLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_limit;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::Shutdown( void )
{GUCEF_TRACE;

    TInnerVector inners;
    bool unregister = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_shutdown = true;
        unregister = m_registered;
        m_registered = false;
        for ( TAccessVector::iterator i = m_accesses.begin(); i != m_accesses.end(); ++i )
        {
            CDrnBridgeFileAccess* access = (*i);
            if ( GUCEF_NULL != access->m_inner && !access->m_busy )
            {
                inners.push_back( access->m_inner );
                access->m_inner = GUCEF_NULL;
                --m_active;
            }
        }
    }
    DeleteInners( inners );

    // the share this pool had goes to the other mounts of the member
    if ( unregister )
        m_connection->UnregisterHandleUser( this );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::RecordResult( const CDrnBridgeResult& result )
{GUCEF_TRACE;

    if ( !m_lastResults.IsNULL() )
        m_lastResults->Set( result );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::NoteChanged( const CORE::CString& path, const CORE::CString& revision, bool report )
{GUCEF_TRACE;

    if ( !m_cache.IsNULL() )
        m_cache->InvalidatePath( path );

    CIDrnBridgeChangeListener* listener = GUCEF_NULL;
    if ( report )
    {
        MT::CScopeMutex lock( m_lock );
        listener = m_changeListener;
    }
    if ( GUCEF_NULL != listener )
        listener->OnFileChanged( path, revision );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::SetChangeListener( CIDrnBridgeChangeListener* listener )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_changeListener = listener;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeFileAccess*
CDrnBridgeHandlePool::OpenFile( CDrnBridgeHandlePoolPtr pool, const CORE::CString& path, CDrnBridgeResult& result, const CORE::CString& expectedRevision, bool reportsChange )
{GUCEF_TRACE;

    CDrnBridgeFileAccess* access = GUCEF_NEW CDrnBridgeFileAccess( pool, path );
    access->m_expectedRevision = expectedRevision;
    access->m_reportsChange = reportsChange;
    if ( GUCEF_NULL == pool->BeginUse( *access, result ) )
    {
        GUCEF_DELETE access;
        if ( result.IsChanged() )
            pool->NoteChanged( path, expectedRevision, reportsChange );
        return GUCEF_NULL;
    }
    pool->EndUse( *access );
    return access;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNResourceIOAccess*
CDrnBridgeHandlePool::BeginUse( CDrnBridgeFileAccess& access, CDrnBridgeResult& result )
{GUCEF_TRACE;

    const UInt64 startMs = CDrnBridgeMetadataCache::NowMs();
    TInnerVector evicted;

    // 0. a file that has no handle needs room for one, within what this pool may hold: its own setting, or its share of what
    // the member allows when that is lower. Asked outside the lock (the member is asked once, the first time)
    bool hasHandle = false;
    {
        MT::CScopeMutex lock( m_lock );
        hasHandle = GUCEF_NULL != access.m_inner;
    }
    if ( !hasHandle )
    {
        const UInt32 share = m_connection->GetHandleShare( this, m_settings.openTimeoutMs );
        MT::CScopeMutex lock( m_lock );
        m_limit = 0 != share && share < m_settings.maxOpenHandles ? share : m_settings.maxOpenHandles;
    }

    // 1. a handle that is there is used; otherwise room is made for one: a free place, or the place of the file read longest ago
    bool reserved = false;
    while ( !reserved )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_shutdown )
            {
                result = CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, "the archive was unloaded" );
                return GUCEF_NULL;
            }
            access.m_lastUsed = ++m_useCounter;
            if ( GUCEF_NULL != access.m_inner )
            {
                access.m_busy = true;
                return access.m_inner;
            }
            if ( access.m_opening )
            {
                result = CDrnBridgeResult( DRN::DRN_NSRESULT_BUSY, DRN::DRN_RESSTATUS_BUSY, "the file is being opened by another thread: a file is read by one thread at a time" );
                return GUCEF_NULL;
            }

            if ( m_active < m_limit )
            {
                ++m_active;
                if ( m_active > m_peak )
                    m_peak = m_active;
                access.m_opening = true;
                reserved = true;
            }
            else
            {
                CDrnBridgeFileAccess* victim = GUCEF_NULL;
                for ( TAccessVector::iterator i = m_accesses.begin(); i != m_accesses.end(); ++i )
                {
                    CDrnBridgeFileAccess* candidate = (*i);
                    if ( candidate != &access && GUCEF_NULL != candidate->m_inner && !candidate->m_busy && !candidate->m_opening &&
                         ( GUCEF_NULL == victim || candidate->m_lastUsed < victim->m_lastUsed ) )
                    {
                        victim = candidate;
                    }
                }
                if ( GUCEF_NULL != victim )
                {
                    // its place becomes ours: the count of handles stays
                    evicted.push_back( victim->m_inner );
                    victim->m_inner = GUCEF_NULL;
                    ++m_evictions;
                    access.m_opening = true;
                    reserved = true;

                    // a share that shrank (another mount came): the idle handles beyond it go as well, so that the pool comes within it
                    while ( m_active > m_limit )
                    {
                        CDrnBridgeFileAccess* extra = GUCEF_NULL;
                        for ( TAccessVector::iterator i = m_accesses.begin(); i != m_accesses.end(); ++i )
                        {
                            CDrnBridgeFileAccess* candidate = (*i);
                            if ( candidate != &access && GUCEF_NULL != candidate->m_inner && !candidate->m_busy && !candidate->m_opening &&
                                 ( GUCEF_NULL == extra || candidate->m_lastUsed < extra->m_lastUsed ) )
                            {
                                extra = candidate;
                            }
                        }
                        if ( GUCEF_NULL == extra )
                            break;
                        evicted.push_back( extra->m_inner );
                        extra->m_inner = GUCEF_NULL;
                        ++m_evictions;
                        --m_active;
                    }
                }
            }
        }
        if ( !reserved )
        {
            // every handle is being read by another thread: wait for one to come free
            if ( CDrnBridgeMetadataCache::NowMs() - startMs > m_settings.openTimeoutMs )
            {
                result = CDrnBridgeResult( DRN::DRN_NSRESULT_BUSY, DRN::DRN_RESSTATUS_BUSY, "every one of the handles of the archive is in use by a read" );
                return GUCEF_NULL;
            }
            MT::ThreadDelay( 2 );
        }
    }

    // closed before the new one is asked for: the serving side then sees the close first and has room for the open
    DeleteInners( evicted );

    // 2. opened at the revision the file was pinned at, or, the first time, at whatever it is now
    DRN::CDRNResourceTransferOptions options;
    options.verifyMode = m_settings.verifyMode;
    const bool isReopen = access.m_pinned;
    if ( isReopen )
        options.expectedRevision = access.m_pinnedRevision;
    else
        options.expectedRevision = access.m_expectedRevision;

    DRN::CDRNResourceHandlePtr handle;
    result = m_connection->OpenHandle( m_share, access.m_path, options, m_settings.openTimeoutMs, handle, m_settings.busyRetries, m_settings.busyBackoffMs );
    bool good = result.IsOk() && !handle.IsNULL();
    DRN::CDRNResourceIOAccess* inner = GUCEF_NULL;
    if ( good )
    {
        const DRN::CDRNResourceInfo info = handle->GetInfo();
        if ( !access.m_pinned && !access.m_expectedRevision.IsNULLOrEmpty() && info.revision != access.m_expectedRevision )
        {
            // the serving side did not hold to the revision that was asked for: this is not the file that was meant
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_CHANGED, DRN::DRN_RESSTATUS_CHANGED, "the file is not the revision it was opened for" );
            good = false;
        }
        else
        if ( !access.m_pinned )
        {
            access.m_pinnedRevision = info.revision;
            access.m_pinnedSize = info.size;
            access.m_pinnedModifiedUtcMs = info.modifiedUtcMs;
            access.m_pinned = true;

            // a revision other than the one the cache has for the path: the cache is behind
            if ( !m_cache.IsNULL() )
            {
                DRN::CDRNNamespaceEntry cached;
                bool exists = false;
                if ( CDrnBridgeMetadataCache::STATE_MISS != m_cache->FindEntry( access.m_path, cached, exists ) && ( !exists || cached.revision != info.revision ) )
                    m_cache->InvalidatePath( access.m_path );
            }
        }
        else
        if ( info.size != access.m_pinnedSize || info.revision != access.m_pinnedRevision )
        {
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_CHANGED, DRN::DRN_RESSTATUS_CHANGED, "the file is not the revision it was opened at" );
            good = false;
        }
    }
    if ( good )
    {
        inner = GUCEF_NEW DRN::CDRNResourceIOAccess( handle, m_settings.chunkCacheChunks, m_settings.readAheadChunks );
        inner->Open();
        if ( !inner->Opened() )
        {
            result = CDrnBridgeResult::FromTransferResult( inner->GetLastResult() );
            GUCEF_DELETE inner;
            inner = GUCEF_NULL;
            good = false;
        }
    }
    else
    if ( !handle.IsNULL() )
    {
        handle->Close();
    }

    // 3. kept for the file, or given back
    {
        MT::CScopeMutex lock( m_lock );
        access.m_opening = false;
        if ( good && !m_shutdown )
        {
            access.m_inner = inner;
            access.m_busy = true;
            if ( isReopen )
                ++m_reopens;
            return inner;
        }
        --m_active;
    }
    if ( GUCEF_NULL != inner )
    {
        GUCEF_DELETE inner;
        result = CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, "the archive was unloaded" );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeHandlePool::EndUse( CDrnBridgeFileAccess& access )
{GUCEF_TRACE;

    TInnerVector inners;
    {
        MT::CScopeMutex lock( m_lock );
        access.m_busy = false;
        if ( m_shutdown && GUCEF_NULL != access.m_inner )
        {
            inners.push_back( access.m_inner );
            access.m_inner = GUCEF_NULL;
            --m_active;
        }
    }
    DeleteInners( inners );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
