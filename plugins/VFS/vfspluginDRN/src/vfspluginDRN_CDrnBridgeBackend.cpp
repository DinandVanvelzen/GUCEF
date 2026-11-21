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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H
#include "vfspluginDRN_CDrnBridgeBlockSource.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H ? */

#include "vfspluginDRN_CDrnBridgeBackend.h"

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

CDrnBridgeBackend::CDrnBridgeBackend( const CDrnBridgeConnectionPtr& connection   ,
                                      const CORE::CString& share                  ,
                                      const CDrnBridgeSettings& settings          ,
                                      const CDrnBridgeLastResultsPtr& lastResults )
    : VFS::CIVfsRemoteBackend()
    , CORE::CObserver()
    , m_connection( connection )
    , m_share( share )
    , m_settings( settings )
    , m_metadata( GUCEF_NEW CDrnBridgeMetadataCache( settings ) )
    , m_pool()
    , m_lastResults( lastResults )
    , m_watchLock()
    , m_watched( GUCEF_NULL )
    , m_listener( GUCEF_NULL )
    , m_shutdown( false )
    , m_wasUp( true )
    , m_protocolProblem( 0 )
    , m_fetchCalls( 0 )
    , m_bytesFetched( 0 )
    , m_statBaseline( connection->GetStatCallCount() )
    , m_listBaseline( connection->GetListCallCount() )
    , m_openBaseline( connection->GetOpenCallCount() )
    , m_busyBaseline( connection->GetBusyRetryCount() )
{GUCEF_TRACE;

    m_pool = CDrnBridgeHandlePoolPtr( GUCEF_NEW CDrnBridgeHandlePool( m_connection, m_share, m_settings, m_metadata, m_lastResults ) );
    m_pool->SetChangeListener( this );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeBackend::~CDrnBridgeBackend()
{GUCEF_TRACE;

    Shutdown();
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::SetLinkListener( VFS::CVfsRemoteArchiveCore* core )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_watchLock );
        m_listener = core;
    }
    EnsureWatching();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::Shutdown( void )
{GUCEF_TRACE;

    m_shutdown.Store( true );
    DRN::CDRNSession* watched = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_watchLock );
        m_listener = GUCEF_NULL;
        watched = m_watched;
        m_watched = GUCEF_NULL;
    }
    if ( GUCEF_NULL != watched )
    {
        UnsubscribeFrom( *watched );
        UnsubscribeFrom( watched->GetNode() );
    }
    if ( !m_pool.IsNULL() )
    {
        m_pool->SetChangeListener( GUCEF_NULL );
        m_pool->Shutdown();
    }
    if ( !m_metadata.IsNULL() )
        m_metadata->InvalidateAll();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::EnsureWatching( void ) const
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;

    DRN::CDRNSession* session = m_connection->GetSession();
    MT::CScopeMutex lock( m_watchLock );
    if ( session == m_watched || m_shutdown.Load() )
        return;

    CDrnBridgeBackend* self = const_cast< CDrnBridgeBackend* >( this );
    if ( GUCEF_NULL != m_watched )
    {
        self->UnsubscribeFrom( *m_watched );
        self->UnsubscribeFrom( m_watched->GetNode() );
    }
    m_watched = session;
    if ( GUCEF_NULL != session )
    {
        // the link to the member comes and goes with these: the node tells of its links, the session of its members
        self->SubscribeTo( &session->GetNode(), DRN::CDRNNode::LinkOperationalEvent );
        self->SubscribeTo( &session->GetNode(), DRN::CDRNNode::LinkClosedEvent );
        self->SubscribeTo( session, DRN::CDRNSession::JoinedEvent );
        self->SubscribeTo( session, DRN::CDRNSession::MemberJoinedEvent );
        self->SubscribeTo( session, DRN::CDRNSession::MemberLeftEvent );
        self->SubscribeTo( session, DRN::CDRNSession::SessionClosedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::PassOnLinkState( void ) const
{GUCEF_TRACE;

    if ( m_shutdown.Load() )
        return;
    // the state is read before the lock is taken (the node's lock is held by the thread that tells us of the link; this lock is only ever taken after it)
    const bool up = m_connection->IsConnected();

    TellLayer( up );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::TellLayer( bool up ) const
{GUCEF_TRACE;

    // the lock is held while the layer is told: Shutdown() takes it, so that nothing is told after it returned
    MT::CScopeMutex lock( m_watchLock );
    if ( GUCEF_NULL == m_listener )
        return;
    const bool wasUp = m_wasUp.Exchange( up );
    m_listener->NotifyLinkState( up );
    if ( wasUp && !up && !m_settings.serveStaleWhenOffline )
        m_listener->InvalidateMetadata();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::OnNotify( CORE::CNotifier* notifier   ,
                             const CORE::CEvent& eventid ,
                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid )
    {
        MT::CScopeMutex lock( m_watchLock );
        if ( GUCEF_NULL != m_watched && ( static_cast< CORE::CNotifier* >( m_watched ) == notifier || static_cast< CORE::CNotifier* >( &m_watched->GetNode() ) == notifier ) )
            m_watched = GUCEF_NULL;
        return;
    }
    if ( CORE::CNotifier::SubscribeEvent == eventid || CORE::CNotifier::UnsubscribeEvent == eventid || CORE::CNotifier::ModifyEvent == eventid )
        return;

    PassOnLinkState();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::IsLinkUp( void ) const
{GUCEF_TRACE;

    // a session that is bound by name may be another one now: its events are followed from here on
    EnsureWatching();
    const bool up = m_connection->IsConnected();

    // the state is told when it is asked for as well: the events of the link are not the only way a change is found out, and a call that finds the link down is not to be the first to know
    if ( !m_shutdown.Load() )
        TellLayer( up );
    return up;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeBackend::GetPlaceDescription( void ) const
{GUCEF_TRACE;

    return "the share \"" + m_share + "\" of member " + m_connection->GetMemberDescription() + " of session \"" + m_connection->GetSessionName() + "\"";
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeConnectionPtr&
CDrnBridgeBackend::GetConnection( void ) const
{GUCEF_TRACE;

    return m_connection;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeMetadataCachePtr&
CDrnBridgeBackend::GetMetadataCache( void ) const
{GUCEF_TRACE;

    return m_metadata;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeHandlePoolPtr&
CDrnBridgeBackend::GetPool( void ) const
{GUCEF_TRACE;

    return m_pool;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeBackend::GetShare( void ) const
{GUCEF_TRACE;

    return m_share;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeBackend::GetFetchCallCount( void ) const
{GUCEF_TRACE;

    return m_fetchCalls.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeBackend::GetBytesFetched( void ) const
{GUCEF_TRACE;

    return m_bytesFetched.Load();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::HasProtocolProblem( void ) const
{GUCEF_TRACE;

    return 0 != m_protocolProblem.Load();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::FindShareSha256( const CORE::CString& normalizedPath, bool mayAsk, UInt8* digest ) const
{GUCEF_TRACE;

    DRN::CDRNNamespaceEntry entry;
    bool found = false;
    if ( mayAsk )
    {
        CDrnBridgeResult result;
        found = LookupEntry( normalizedPath, entry, result );
        m_lastResults->Set( result );
    }
    else
        found = PeekEntry( normalizedPath, entry );

    if ( !found || !entry.IsFile() || !entry.hasSha256 )
        return false;
    memcpy( digest, entry.sha256, 32 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::OnFileChanged( const CORE::CString& path, const CORE::CString& revision )
{GUCEF_TRACE;

    // the lock is held while the layer is told, as for the state of the link
    MT::CScopeMutex lock( m_watchLock );
    if ( GUCEF_NULL != m_listener )
        m_listener->OnSourceChanged( path, revision );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::InvalidateMetadata( void )
{GUCEF_TRACE;

    m_metadata->InvalidateAll();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::InvalidateMetadata( const CORE::CString& path )
{GUCEF_TRACE;

    m_metadata->InvalidatePath( path );
    m_metadata->InvalidateDirectory( path );
    m_metadata->InvalidateDirectory( CDrnBridgeMetadataCache::ParentOf( path ) );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::PeekEntry( const CORE::CString& path, DRN::CDRNNamespaceEntry& entry ) const
{GUCEF_TRACE;

    bool exists = false;
    if ( CDrnBridgeMetadataCache::STATE_MISS == m_metadata->FindEntry( path, entry, exists ) )
        return false;
    return exists;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::NoteOutcome( const CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    // what says the share cannot be used as a share at all, as opposed to a path that is not there or a link that is down
    if ( DRN::DRN_NSRESULT_INTEGRITY == result.code || DRN::DRN_NSRESULT_UNSUPPORTED == result.code || DRN::DRN_NSRESULT_NO_SUCH_SHARE == result.code )
        m_protocolProblem.Store( 1 );
    else
    if ( result.IsOk() )
        m_protocolProblem.Store( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::LookupEntry( const CORE::CString& normalizedPath, DRN::CDRNNamespaceEntry& entry, CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    result = CDrnBridgeResult();

    bool exists = false;
    const CDrnBridgeMetadataCache::TState state = m_metadata->FindEntry( normalizedPath, entry, exists );
    if ( CDrnBridgeMetadataCache::STATE_FRESH == state )
    {
        if ( !exists )
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, "\"" + normalizedPath + "\" is not there" );
        return exists;
    }

    // a stat that carries the revision the cache has of the path: unchanged is all the answer costs
    CORE::CString::StringVector paths;
    paths.push_back( normalizedPath );
    CORE::CString::StringVector known;
    if ( CDrnBridgeMetadataCache::STATE_STALE == state && exists )
        known.push_back( entry.revision );
    DRN::CDRNNamespaceClient::TEntryVector answers;
    CDrnBridgeResult call = m_connection->Stat( m_share, paths, known, answers, m_settings.callTimeoutMs, m_settings.busyRetries, m_settings.busyBackoffMs );
    if ( !call.IsOk() || answers.size() != 1 )
    {
        if ( call.IsOk() )
            call = CDrnBridgeResult( DRN::DRN_NSRESULT_INTEGRITY, DRN::DRN_RESSTATUS_PROTOCOL_ERROR, "the answer to a stat does not answer the one path asked for" );
        NoteOutcome( call );

        // the answer that the cache could give when the share cannot be reached is the generic layer's to give, from its own record of the path
        result = call;
        entry.Clear();
        return false;
    }

    NoteOutcome( call );
    const DRN::CDRNNamespaceEntry& answer = answers[ 0 ];
    switch ( answer.status )
    {
        case DRN::DRN_RESSTATUS_OK:
        {
            entry = answer;
            m_metadata->PutEntry( normalizedPath, entry );
            return true;
        }
        case DRN::DRN_RESSTATUS_UNCHANGED:
        {
            // the entry is what the cache had
            m_metadata->TouchEntry( normalizedPath );
            return exists;
        }
        case DRN::DRN_RESSTATUS_NOT_FOUND:
        {
            m_metadata->PutAbsent( normalizedPath );
            entry.Clear();
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, "\"" + normalizedPath + "\" is not there" );
            return false;
        }
        default:
        {
            entry.Clear();
            result = CDrnBridgeResult( DRN::CDRNNamespaceResult::CodeFromStatus( answer.status ), answer.status,
                "the stat of \"" + normalizedPath + "\" was answered with " + DRN::CDRNResourceTransferResult::StatusToString( answer.status ) );
            NoteOutcome( result );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeBackend::FetchListing( const CORE::CString& normalizedDir, DRN::CDRNNamespaceClient::TEntryVector& entries, CORE::CString& dirRevision, bool& complete, CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    complete = true;
    bool changedDuringListing = false;
    result = m_connection->ListAll( m_share, normalizedDir, entries, dirRevision, changedDuringListing, m_settings.maxEntriesPerDirectory, m_settings.listPageEntries, m_settings.callTimeoutMs, m_settings.busyRetries, m_settings.busyBackoffMs );
    NoteOutcome( result );
    if ( DRN::DRN_NSRESULT_TOO_LARGE == result.code )
    {
        // what there is, up to the bound
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeBackend: The directory \"" << normalizedDir << "\" of share \"" << m_share << "\" has more than " << m_settings.maxEntriesPerDirectory << " entries, the list is cut there" );
        complete = false;
        return true;
    }
    // a directory that changed while it was listed is a list that was true for no moment: it is used but not remembered
    if ( result.IsOk() && changedDuringListing )
        complete = false;
    return result.IsOk();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::TEntryVectorPtr
CDrnBridgeBackend::GetListing( const CORE::CString& normalizedDir, CDrnBridgeResult& result, bool& fromTheShare ) const
{GUCEF_TRACE;

    result = CDrnBridgeResult();
    fromTheShare = false;

    // something that is known not to be a directory has no listing
    DRN::CDRNNamespaceEntry known;
    bool knownExists = false;
    const CDrnBridgeMetadataCache::TState entryState = m_metadata->FindEntry( normalizedDir, known, knownExists );
    if ( CDrnBridgeMetadataCache::STATE_FRESH == entryState )
    {
        if ( !knownExists )
        {
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, "\"" + normalizedDir + "\" is not there" );
            return CDrnBridgeMetadataCache::TEntryVectorPtr();
        }
        if ( !known.IsDirectory() )
        {
            result = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_A_DIRECTORY, DRN::DRN_RESSTATUS_NOT_A_DIRECTORY, "\"" + normalizedDir + "\" is not a directory" );
            return CDrnBridgeMetadataCache::TEntryVectorPtr();
        }
    }

    CDrnBridgeMetadataCache::TEntryVectorPtr cached;
    CORE::CString cachedRevision;
    const CDrnBridgeMetadataCache::TState state = m_metadata->FindListing( normalizedDir, cached, cachedRevision );
    if ( CDrnBridgeMetadataCache::STATE_FRESH == state )
        return cached;

    DRN::CDRNNamespaceClient::TEntryVector entries;
    CORE::CString dirRevision;
    bool complete = true;

    // an old listing is checked with the revision it has: one small call, and nothing more when the directory is as it was
    if ( CDrnBridgeMetadataCache::STATE_STALE == state && !cachedRevision.IsNULLOrEmpty() )
    {
        DRN::CDRNNamespaceListPage page;
        CDrnBridgeResult call = m_connection->List( m_share, normalizedDir, CORE::CString::Empty, m_settings.listPageEntries, cachedRevision, page, m_settings.callTimeoutMs, m_settings.busyRetries, m_settings.busyBackoffMs );
        NoteOutcome( call );
        if ( call.IsOk() && page.unchanged )
        {
            m_metadata->TouchListing( normalizedDir );
            return cached;
        }
        if ( call.IsOk() )
        {
            // the directory moved on: its entries, as far as this page goes and the pages that follow
            entries = page.entries;
            dirRevision = page.dirRevision;
            CORE::CString cursor = page.nextCursor;
            bool failed = false;
            while ( !cursor.IsNULLOrEmpty() && !failed )
            {
                call = m_connection->List( m_share, normalizedDir, cursor, m_settings.listPageEntries, CORE::CString::Empty, page, m_settings.callTimeoutMs, m_settings.busyRetries, m_settings.busyBackoffMs );
                NoteOutcome( call );
                if ( !call.IsOk() )
                {
                    failed = true;
                    break;
                }
                entries.insert( entries.end(), page.entries.begin(), page.entries.end() );
                cursor = page.nextCursor;
                if ( entries.size() > m_settings.maxEntriesPerDirectory )
                {
                    complete = false;
                    break;
                }
            }
            if ( !failed )
            {
                fromTheShare = true;
                CDrnBridgeMetadataCache::TEntryVectorPtr listing = m_metadata->PutListing( normalizedDir, entries, dirRevision );
                if ( !complete )
                    m_metadata->InvalidateDirectory( normalizedDir );
                return listing;
            }

            // a cursor that expired, or a failure: start over with the whole listing
            entries.clear();
            dirRevision.Clear();
        }
        else
        if ( DRN::DRN_NSRESULT_NOT_FOUND == call.code || DRN::DRN_NSRESULT_NOT_A_DIRECTORY == call.code )
        {
            m_metadata->InvalidateDirectory( normalizedDir );
            if ( DRN::DRN_NSRESULT_NOT_FOUND == call.code )
                m_metadata->PutAbsent( normalizedDir );
            result = call;
            return CDrnBridgeMetadataCache::TEntryVectorPtr();
        }
    }

    CDrnBridgeResult fetch;
    if ( !FetchListing( normalizedDir, entries, dirRevision, complete, fetch ) )
    {
        if ( DRN::DRN_NSRESULT_NOT_FOUND == fetch.code )
            m_metadata->PutAbsent( normalizedDir );
        m_metadata->InvalidateDirectory( normalizedDir );
        result = fetch;
        return CDrnBridgeMetadataCache::TEntryVectorPtr();
    }

    fromTheShare = true;
    CDrnBridgeMetadataCache::TEntryVectorPtr listing = m_metadata->PutListing( normalizedDir, entries, dirRevision );
    if ( !complete )
        m_metadata->InvalidateDirectory( normalizedDir );
    return listing;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::ToRemoteEntry( const CORE::CString& name, const DRN::CDRNNamespaceEntry& source, bool attributesKnown, VFS::CVfsRemoteEntry& entry )
{GUCEF_TRACE;

    entry = VFS::CVfsRemoteEntry();
    entry.attributesKnown = attributesKnown;
    entry.name = name;
    entry.exists = true;
    entry.isDirectory = source.IsDirectory();
    entry.size = entry.isDirectory ? 0 : source.size;
    entry.revision = source.revision;
    entry.modifiedUtcMs = source.modifiedUtcMs;
    if ( !entry.isDirectory && source.hasSha256 )
        entry.digest = VFS::CVfsDigest::FromBytes( VFS::CVfsDigest::ALGORITHM_SHA256, source.sha256, 32 );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CDrnBridgeBackend::Stat( const CORE::CString& path, VFS::CVfsRemoteEntry& entry )
{GUCEF_TRACE;

    entry = VFS::CVfsRemoteEntry();
    entry.exists = false;

    DRN::CDRNNamespaceEntry found;
    CDrnBridgeResult result;
    const bool exists = LookupEntry( path, found, result );
    m_lastResults->Set( result );
    if ( !exists )
    {
        // the share has nothing at the path: a result of the call that went well. Anything else says that the share could not tell
        if ( result.IsOk() || DRN::DRN_NSRESULT_NOT_FOUND == result.code )
            return VFS::CVfsCallResult();
        return result.ToCallResult();
    }

    ToRemoteEntry( CDrnBridgeMetadataCache::LeafOf( path ), found, true, entry );
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CDrnBridgeBackend::List( const CORE::CString& dirPath, VFS::CVfsRemoteEntry::TVector& entries, bool& dirExists )
{GUCEF_TRACE;

    entries.clear();
    dirExists = false;

    CDrnBridgeResult result;
    bool fromTheShare = false;
    CDrnBridgeMetadataCache::TEntryVectorPtr listing = GetListing( dirPath, result, fromTheShare );
    m_lastResults->Set( result );
    if ( listing.IsNULL() )
    {
        // a directory that is not there, or a file where a directory was meant, is an answer of the share
        if ( result.IsOk() || DRN::DRN_NSRESULT_NOT_FOUND == result.code || DRN::DRN_NSRESULT_NOT_A_DIRECTORY == result.code )
            return VFS::CVfsCallResult();
        return result.ToCallResult();
    }

    // what the cache holds of the entries is as old as its last full look: a listing that was renewed by a check of the directory alone vouches for the names
    const bool attributesKnown = fromTheShare || m_metadata->AreAttributesCurrent( dirPath );
    dirExists = true;
    entries.reserve( listing->size() );
    for ( CDrnBridgeMetadataCache::TEntryVector::const_iterator i = listing->begin(); i != listing->end(); ++i )
    {
        VFS::CVfsRemoteEntry entry;
        ToRemoteEntry( (*i).name, (*i), attributesKnown, entry );
        entries.push_back( entry );
    }
    return VFS::CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

VFS::CIVfsBlockSourcePtr
CDrnBridgeBackend::OpenBlockSource( const CORE::CString& path, const CORE::CString& revision, VFS::CVfsCallResult& result )
{GUCEF_TRACE;

    CDrnBridgeResult opened;
    VFS::CIVfsBlockSourcePtr source = CDrnBridgeBlockSource::Open( m_pool, path, revision, result, &opened, &m_fetchCalls, &m_bytesFetched );
    NoteOutcome( opened );
    m_lastResults->Set( opened );
    return source;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeBackend::GetBackendStatistics( VFS::CVfsRemoteStatistics& statistics ) const
{GUCEF_TRACE;

    // the calls the connection made for this backend: it can be shared by several mounts of a member, so the counts are those since this backend was made
    statistics.statCalls = m_connection->GetStatCallCount() - m_statBaseline;
    statistics.listCalls = m_connection->GetListCallCount() - m_listBaseline;
    statistics.openCalls = m_connection->GetOpenCallCount() - m_openBaseline;
    statistics.busyRetries = m_connection->GetBusyRetryCount() - m_busyBaseline;

    statistics.handlesOpen = m_pool->GetOpenHandleCount();
    statistics.handlesPeak = m_pool->GetPeakOpenHandleCount();
    statistics.handleEvictions = m_pool->GetEvictionCount();
    statistics.handleReopens = m_pool->GetReopenCount();
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
