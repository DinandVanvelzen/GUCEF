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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#include "gucefVFS_CVfsThreadPolicyScope.h"
#define GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#include "vfspluginDRN_CDrnBridgeFileAccess.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */

#include "vfspluginDRN_CDrnBridgeArchive.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
JoinPath( const CORE::CString& dir, const CORE::CString& name )
{GUCEF_TRACE;

    return dir.IsNULLOrEmpty() ? name : dir + "/" + name;
}

/** Puts the rule of the calling thread back when the call that needed another one is over */
class CPolicyScopeHolder
{
    public:

    CPolicyScopeHolder( void )
        : m_scope( GUCEF_NULL )
    {GUCEF_TRACE;
    }

    ~CPolicyScopeHolder()
    {GUCEF_TRACE;

        GUCEF_DELETE m_scope;
    }

    void Enter( const VFS::CVfsThreadPolicy& policy )
    {GUCEF_TRACE;

        m_scope = GUCEF_NEW VFS::CVfsThreadPolicyScope( policy );
    }

    private:

    CPolicyScopeHolder( const CPolicyScopeHolder& src );
    CPolicyScopeHolder& operator=( const CPolicyScopeHolder& src );

    VFS::CVfsThreadPolicyScope* m_scope;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeArchive::CDrnBridgeArchive( void )
    : VFS::CVfsRemoteArchive()
    , m_archiveName()
    , m_archivePath()
    , m_share()
    , m_identity()
    , m_settings()
    , m_connection()
    , m_backend()
    , m_lastResults( GUCEF_NEW CDrnBridgeLastResults() )
    , m_scheduler( GUCEF_NULL )
    , m_loaded( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeArchive::~CDrnBridgeArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::IsLoaded( void ) const
{GUCEF_TRACE;

    return 0 != m_loaded.Load();
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeResult&
CDrnBridgeArchive::Record( const CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    return m_lastResults->Set( result );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::Settle( void ) const
{GUCEF_TRACE;

    // the backend records what the share said; what the generic layer decided by itself (a miss that is not waited for, a link that is down while the answer
    // was to come from the cache) is recorded here, in the form of the bridge
    if ( !m_lastResults->Get().IsOk() )
        return;
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( core.IsNULL() )
        return;
    const VFS::CVfsCallResult last = core->GetLastResult();
    if ( !last.IsSuccess() )
        m_lastResults->Set( CDrnBridgeResult::FromCallResult( last ) );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::Refuse( const char* what ) const
{GUCEF_TRACE;

    Record( CDrnBridgeResult( DRN::DRN_NSRESULT_READ_ONLY, DRN::DRN_RESSTATUS_READ_ONLY,
        CORE::CString( "the share is read only: " ) + what + " is not possible" ) );
    return false;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeArchive::GetLastResult( void ) const
{GUCEF_TRACE;

    return m_lastResults->Get();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::NormalizePath( const CORE::CString& path, CORE::CString& normalized )
{GUCEF_TRACE;

    // the usual path is right already: one pass to see
    bool plain = true;
    const UInt32 length = path.Length();
    for ( UInt32 i=0; i<length && plain; ++i )
    {
        const char c = path[ i ];
        if ( '\\' == c )
            plain = false;
        else
        if ( '/' == c )
        {
            if ( 0 == i || i + 1 == length || '/' == path[ i+1 ] )
                plain = false;
        }
        else
        if ( '.' == c && ( 0 == i || '/' == path[ i-1 ] ) && ( i + 1 == length || '/' == path[ i+1 ] || '.' == path[ i+1 ] ) )
            plain = false;
    }
    if ( plain )
    {
        normalized = path;
        return true;
    }

    CORE::CString unified = path.ReplaceChar( '\\', '/' );
    const CORE::CString::StringVector parts = unified.ParseElements( '/', false );
    normalized.Clear();
    for ( CORE::CString::StringVector::const_iterator i = parts.begin(); i != parts.end(); ++i )
    {
        if ( (*i) == "." )
            continue;
        if ( (*i) == ".." )
            return false;
        normalized = JoinPath( normalized, (*i) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::ParseDrnAddress( const CORE::CString& source, CORE::CString& sessionName, CORE::CString& member, CORE::CString& share, CORE::CString& why )
{GUCEF_TRACE;

    // drn://<session name>/<member>/<share>; the path handling of the archive settings turns the "//" into a "/", which is as good
    CORE::CString rest = source.CutChars( 4, true, 0 );
    while ( rest.Length() > 0 && '/' == rest[ 0 ] )
        rest = rest.CutChars( 1, true, 0 );
    CORE::CUri uri;
    if ( !uri.ParseUriInStringForm( "drn://" + rest ) )
    {
        why = "\"" + source + "\" is not a URI";
        return false;
    }
    sessionName = uri.GetAuthorityHost().IsNULLOrEmpty() ? uri.GetAuthority() : uri.GetAuthorityHost();
    const CORE::CString::StringVector parts = uri.GetPath().ParseElements( '/', false );
    if ( sessionName.IsNULLOrEmpty() || 2 != parts.size() )
    {
        why = "\"" + source + "\" is not of the form drn://<session name>/<member>/<share>";
        return false;
    }
    if ( parts[ 0 ].IsNULLOrEmpty() )
    {
        why = "\"" + source + "\" names no member";
        return false;
    }
    member = parts[ 0 ];
    share = parts[ 1 ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::ResolveSource( const CORE::CString& source, CDrnBridgeConnectionPtr& connection, CORE::CString& share, CORE::CString& why ) const
{GUCEF_TRACE;

    if ( source.StartsWith( "drn:" ) )
    {
        CORE::CString sessionName;
        CORE::CString member;
        if ( !ParseDrnAddress( source, sessionName, member, share, why ) )
            return false;

        // the session is found by its name each time the connection is used: it need not exist yet, and a session of the name that
        // replaces another one is followed. Until there is one every call says that the share cannot be reached
        connection = CDrnBridgeConnectionPtr( GUCEF_NEW CDrnBridgeConnection( sessionName, member ) );
        return true;
    }

    // <connection id>|<share>
    const Int32 bar = source.HasChar( '|', true );
    if ( bar <= 0 )
    {
        why = "\"" + source + "\" is neither drn://<session name>/<member>/<share> nor <connection id>|<share>";
        return false;
    }
    const CORE::CString id = source.SubstrToIndex( (UInt32) bar, true );
    share = source.CutChars( (UInt32) bar + 1, true, 0 );
    connection = CDrnBridgeGlobal::Instance()->FindConnection( id );
    if ( connection.IsNULL() )
    {
        why = "no connection \"" + id + "\" is registered (CDrnBridgeGlobal::RegisterConnection())";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeArchive::MakeDefaultIdentity( void ) const
{GUCEF_TRACE;

    // the address of the share: the form drn://<session name>/<member>/<share> in the one spelling, or what the source said for a connection by id
    if ( m_archivePath.StartsWith( "drn:" ) )
    {
        CORE::CString sessionName;
        CORE::CString member;
        CORE::CString share;
        CORE::CString why;
        if ( ParseDrnAddress( m_archivePath, sessionName, member, share, why ) )
            return "drn://" + sessionName + "/" + member + "/" + share;
    }
    if ( !m_archivePath.IsNULLOrEmpty() )
        return m_archivePath;
    return "drn://" + m_connection->GetSessionName() + "/" + m_connection->GetMemberDescription() + "/" + m_share;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    CORE::CString source = settings.GetActualArchivePath();
    if ( source.IsNULLOrEmpty() )
        source = settings.GetArchivePath();

    CDrnBridgeConnectionPtr connection;
    CORE::CString share;
    CORE::CString why;
    if ( !ResolveSource( source, connection, share, why ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DrnBridgeArchive: Cannot mount \"" << settings.GetArchiveName() << "\": " << why );
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, why ) );
        return false;
    }

    CDrnBridgeSettings bridgeSettings;
    bridgeSettings.LoadFromArchiveSettings( settings );
    m_archivePath = source;
    return LoadArchive( connection, share, bridgeSettings, settings.GetArchiveName() );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::LoadArchive( const VFS::CString& archiveName       ,
                                VFS::TBasicVfsResourcePtr vfsResource ,
                                const bool writeableRequest           )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeArchive: An archive of the type drn is a share of another node: it cannot be loaded from a resource" );
    Record( CDrnBridgeResult( DRN::DRN_NSRESULT_UNSUPPORTED, DRN::DRN_RESSTATUS_UNSUPPORTED, "a share of another node cannot be loaded from a resource" ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::LoadArchive( const CDrnBridgeConnectionPtr& connection ,
                                const CORE::CString& share                ,
                                const CDrnBridgeSettings& settings        ,
                                const CORE::CString& archiveName          )
{GUCEF_TRACE;

    UnloadArchive();

    if ( connection.IsNULL() )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "no connection to a session was given" ) );
        return false;
    }
    if ( !DRN::CDRNResourceName::IsValidProviderName( share ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DrnBridgeArchive: \"" << share << "\" is not the name of a share" );
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + share + "\" is not the name of a share" ) );
        return false;
    }

    m_settings = settings;
    m_settings.Normalize();
    m_share = share;
    m_archiveName = archiveName.IsNULLOrEmpty() ? share : archiveName;
    if ( m_archivePath.IsNULLOrEmpty() )
        m_archivePath = "drn:" + connection->GetSessionName() + "/" + connection->GetMemberDescription() + "/" + share;
    m_connection = connection;
    m_identity = m_settings.cacheIdentity.IsNULLOrEmpty() ? MakeDefaultIdentity() : m_settings.cacheIdentity;

    // the block cache: the folder of the settings (one cache for every archive that names the folder), else the one the application made the default
    VFS::CVfsBlockCachePtr cache;
    if ( !m_settings.blockCacheFolder.IsNULLOrEmpty() )
    {
        cache = VFS::CVfsGlobal::Instance()->AcquireBlockCache( m_settings.blockCacheFolder, m_settings.blockCacheMaxBytes,
                    0 == m_settings.blockCacheBlockBytes ? VFS::CVfsBlockCache::DefaultBlockBytes : m_settings.blockCacheBlockBytes );
        if ( cache.IsNULL() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DrnBridgeArchive: The block cache folder \"" << m_settings.blockCacheFolder << "\" cannot be opened: share \"" << share << "\" is mounted without a block cache" );
    }
    else
        cache = VFS::CVfsGlobal::Instance()->GetDefaultBlockCache();

    // the share itself. With a block cache the blocks are read in runs of their own and the facility's chunk read ahead is not used for them
    CDrnBridgeSettings backendSettings( m_settings );
    if ( !cache.IsNULL() )
        backendSettings.readAheadChunks = 0;
    CDrnBridgeBackendPtr backend( GUCEF_NEW CDrnBridgeBackend( m_connection, m_share, backendSettings, m_lastResults ) );

    // the generic layer on top of it
    VFS::CVfsRemoteSettings coreSettings;
    coreSettings.cacheIdentity = m_identity;
    coreSettings.metadataTtlMs = m_settings.metadataTtlMs;
    coreSettings.maxMetadataEntries = m_settings.maxCachedEntries;
    coreSettings.readAheadAfterBlocks = m_settings.readAheadAfterBlocks;
    coreSettings.readAheadBlocks = m_settings.readAheadBlocks;
    coreSettings.maxBlocksPerFetch = m_settings.maxBlocksPerFetch;
    coreSettings.verifyCompleteFiles = m_settings.verifyCompleteFiles;
    coreSettings.refetchPinned = m_settings.refetchPinned;
    coreSettings.offlineListings = m_settings.offlineListings;

    VFS::CVfsFetchScheduler* scheduler = GUCEF_NEW VFS::CVfsFetchScheduler( m_settings.fetchThreadPool );
    scheduler->SetMaxRunning( m_settings.maxBackgroundFetches );
    scheduler->SetMaxQueued( m_settings.maxQueuedFetches );

    VFS::CVfsRemoteArchiveCorePtr core = VFS::CVfsRemoteArchiveCore::Create( backend, cache, coreSettings, scheduler, GUCEF_NULL );
    backend->SetLinkListener( core.GetPointerAlways() );
    m_lastResults->Set( CDrnBridgeResult() );

    // AttachCore() unloads what was there: the new backend and scheduler are kept after it
    AttachCore( core, m_archiveName );
    m_backend = backend;
    m_scheduler = scheduler;
    m_loaded.Store( 1 );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeArchive: Mounted share \"" << share << "\" of member " << connection->GetMemberDescription() << " of session \"" << connection->GetSessionName() << "\" as archive \"" << m_archiveName << "\"" << ( cache.IsNULL() ? "" : ", with a block cache" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::UnloadArchive( void )
{GUCEF_TRACE;

    // what is open stays valid as an object: it reads nothing any more. The state is not torn down here, the files that are open use it
    m_loaded.Store( 0 );

    // the jobs of the generic layer are dropped and waited for first: nothing of them runs, and no event is raised, after this
    VFS::CVfsRemoteArchive::UnloadArchive();
    if ( !m_backend.IsNULL() )
        m_backend->Shutdown();

    // a file that is open holds the layer, which no longer asks the scheduler: it is not needed any more
    GUCEF_DELETE m_scheduler;
    m_scheduler = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeArchive::GetType( void ) const
{GUCEF_TRACE;

    return CDrnBridgeGlobal::ArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeArchive::GetShare( void ) const
{GUCEF_TRACE;

    return m_share;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeSettings&
CDrnBridgeArchive::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeConnectionPtr&
CDrnBridgeArchive::GetConnection( void ) const
{GUCEF_TRACE;

    return m_connection;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeBackendPtr&
CDrnBridgeArchive::GetBackend( void ) const
{GUCEF_TRACE;

    return m_backend;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsBlockCachePtr
CDrnBridgeArchive::GetBlockCache( void ) const
{GUCEF_TRACE;

    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    return core.IsNULL() ? VFS::CVfsBlockCachePtr() : core->GetBlockCache();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeArchive::GetCacheIdentity( void ) const
{GUCEF_TRACE;

    return m_identity;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsFetchScheduler*
CDrnBridgeArchive::GetFetchScheduler( void ) const
{GUCEF_TRACE;

    return m_scheduler;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeArchive::GetFetchCallCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetFetchCallCount();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeArchive::GetBytesFetched( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetBytesFetched();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::IsHealthy( void ) const
{GUCEF_TRACE;

    // healthy is what can be used as it is meant to be: mounted, with a session that is still there and a share that speaks the
    // protocol. A link that is down is not unhealthy, it is IsConnected() that tells: the mount stays and serves what it holds. A mount that
    // names its session by name expects the session to come and go, so the absence of one is a down link to it, not a fault
    if ( !IsLoaded() || m_connection.IsNULL() || m_backend.IsNULL() )
        return false;
    if ( GUCEF_NULL == m_connection->GetSession() && !m_connection->IsBoundByName() )
        return false;
    return !m_backend->HasProtocolProblem();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::IsConnected( void ) const
{GUCEF_TRACE;

    if ( !IsLoaded() || m_backend.IsNULL() )
        return false;

    // the state is also told when it is asked for: the events of the link are not the only way it is found out
    const bool connected = m_backend->IsLinkUp();
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( !core.IsNULL() )
        core->NotifyLinkState( connected );
    return connected;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::InvalidateMetadata( void )
{GUCEF_TRACE;

    // the layer drops its own records and tells the backend to drop those of the plugin (VFS::CIVfsRemoteBackend::InvalidateMetadata())
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( !core.IsNULL() )
        core->InvalidateMetadata();
    else
    if ( !m_backend.IsNULL() )
        m_backend->InvalidateMetadata();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::InvalidateMetadata( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString normalized;
    if ( !NormalizePath( path, normalized ) )
        return;
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( !core.IsNULL() )
    {
        core->InvalidateMetadata( normalized );
        core->InvalidateMetadata( CDrnBridgeMetadataCache::ParentOf( normalized ) );
    }
    else
    if ( !m_backend.IsNULL() )
        m_backend->InvalidateMetadata( normalized );
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeArchive::GetOpenHandleCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetPool()->GetOpenHandleCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeArchive::GetPeakOpenHandleCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetPool()->GetPeakOpenHandleCount();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeArchive::GetHandleEvictionCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetPool()->GetEvictionCount();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeArchive::GetHandleReopenCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetPool()->GetReopenCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeArchive::GetHandleLimit( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetPool()->GetHandleLimit();
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeArchive::GetCachedEntryCount( void ) const
{GUCEF_TRACE;

    return m_backend.IsNULL() ? 0 : m_backend->GetMetadataCache()->GetEntryCount();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeArchive::EKind
CDrnBridgeArchive::Classify( const CORE::CString& normalizedPath, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( core.IsNULL() )
        return KIND_FAILED;

    Record( CDrnBridgeResult() );
    if ( core->GetFileMetaData( normalizedPath, metaData ) )
    {
        Settle();
        return metaData.hasResourceSizeInBytes ? KIND_FILE : KIND_DIRECTORY;
    }

    // a call that went well and found nothing there: the path is not there. Anything else is a failure of the call, which the result says
    if ( core->GetLastResult().IsSuccess() )
    {
        if ( m_lastResults->Get().IsOk() )
            Record( CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, "\"" + normalizedPath + "\" is not there" ) );
        return KIND_ABSENT;
    }
    Settle();
    return KIND_FAILED;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::GetFileList( TStringVector& outputList                   ,
                                const VFS::CString& mountLocation           ,
                                const VFS::CString& archiveLocation         ,
                                bool recursive                              ,
                                bool includePathInFilename                  ,
                                const CORE::CString::StringSet& nameFilters ,
                                UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, "the archive is not loaded" ) );
        return;
    }
    CORE::CString location;
    if ( !NormalizePath( archiveLocation, location ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + archiveLocation + "\" leads out of the share" ) );
        return;
    }
    Record( CDrnBridgeResult() );
    VFS::CVfsRemoteArchive::GetFileList( outputList, mountLocation, location, recursive, includePathInFilename, nameFilters, maxListEntries );
    Settle();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeArchive::GetDirList( TStringVector& outputList                   ,
                               const VFS::CString& mountLocation           ,
                               const VFS::CString& archiveLocation         ,
                               bool recursive                              ,
                               bool includeParentDirInName                 ,
                               const CORE::CString::StringSet& nameFilters ,
                               UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, "the archive is not loaded" ) );
        return;
    }
    CORE::CString location;
    if ( !NormalizePath( archiveLocation, location ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + archiveLocation + "\" leads out of the share" ) );
        return;
    }
    Record( CDrnBridgeResult() );
    VFS::CVfsRemoteArchive::GetDirList( outputList, mountLocation, location, recursive, includeParentDirInName, nameFilters, maxListEntries );
    Settle();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return false;
    CORE::CString path;
    if ( !NormalizePath( filePath, path ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + filePath + "\" leads out of the share" ) );
        return false;
    }
    if ( path.IsNULLOrEmpty() )
    {
        // the root of the share is a directory by construction: no call to the remote place
        Record( CDrnBridgeResult() );
        return false;
    }
    CORE::CResourceMetaData metaData;
    return KIND_FILE == Classify( path, metaData );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return false;
    CORE::CString path;
    if ( !NormalizePath( dirPath, path ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + dirPath + "\" leads out of the share" ) );
        return false;
    }
    if ( path.IsNULLOrEmpty() )
    {
        // the root of the share exists by construction: no call to the remote place, whatever the state of the link and of the cache
        Record( CDrnBridgeResult() );
        return true;
    }
    CORE::CResourceMetaData metaData;
    const EKind kind = Classify( path, metaData );
    if ( KIND_FAILED == kind && CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == m_lastResults->Get().fileSystemResult )
    {
        // while the share cannot be reached a directory is there when the cache holds files in it
        VFS::CVfsRemoteArchiveCorePtr core = GetCore();
        if ( !core.IsNULL() && core->DirExists( path ) )
        {
            Record( CDrnBridgeResult() );
            return true;
        }
    }
    return KIND_DIRECTORY == kind;
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CDrnBridgeArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return 0;
    CORE::CString path;
    if ( !NormalizePath( filePath, path ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + filePath + "\" leads out of the share" ) );
        return 0;
    }
    CORE::CResourceMetaData metaData;
    return KIND_FILE == Classify( path, metaData ) ? metaData.resourceSizeInBytes : 0;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::GetFileMetaData( const VFS::CString& filePath      ,
                                    CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    if ( !IsLoaded() )
        return false;
    CORE::CString path;
    if ( !NormalizePath( filePath, path ) )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + filePath + "\" leads out of the share" ) );
        return false;
    }

    const EKind kind = Classify( path, metaData );
    if ( KIND_FILE != kind && KIND_DIRECTORY != kind )
    {
        metaData.Clear();
        return false;
    }

    // what the generic layer does not carry: the flags of the share, and whether this answer was given while the share could not be reached
    DRN::CDRNNamespaceEntry entry;
    const bool hidden = m_backend->PeekEntry( path, entry ) && 0 != ( entry.flags & DRN::DRN_NS_ENTRYFLAG_HIDDEN );
    metaData.hasIsHidden = true;
    metaData.isHidden = hidden;
    metaData.hasIsOffline = true;
    metaData.isOffline = !m_backend->IsLinkUp();
    return true;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CDrnBridgeArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return VFS::CString::Empty;
    CORE::CString path;
    if ( !NormalizePath( file, path ) )
        return VFS::CString::Empty;

    // what the share knows, as its entry has it: the entry of the metadata cache of the plugin, or asked of the share by a thread that may wait while it can be reached
    Record( CDrnBridgeResult() );
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    const bool mayAsk = !core.IsNULL() && !core->GetCurrentThreadPolicy().NeverWaits() && m_backend->IsLinkUp();
    UInt8 digest[ 32 ];
    if ( !m_backend->FindShareSha256( path, mayAsk, digest ) )
        return VFS::CString::Empty;
    return CORE::Base16Encode( digest, 32 );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsDigest
CDrnBridgeArchive::GetFileDigest( const VFS::CString& file ) const
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return VFS::CVfsDigest();
    CORE::CString path;
    if ( !NormalizePath( file, path ) )
        return VFS::CVfsDigest();
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( core.IsNULL() )
        return VFS::CVfsDigest();

    Record( CDrnBridgeResult() );
    const VFS::CVfsDigest digest = core->GetFileDigest( path );
    Settle();
    return digest;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CDrnBridgeArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    CORE::CDateTime time;
    if ( !IsLoaded() )
        return time;
    CORE::CString path;
    if ( !NormalizePath( filePath, path ) )
        return time;
    CORE::CResourceMetaData metaData;
    if ( KIND_FILE == Classify( path, metaData ) && metaData.hasModifiedDateTime )
        time = metaData.modifiedDateTime;
    return time;
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CDrnBridgeArchive::GetFile( const VFS::CString& file      ,
                            const char* mode              ,
                            const VFS::UInt32 memLoadSize ,
                            const bool overwrite          )
{GUCEF_TRACE;

    // reading only
    if ( GUCEF_NULL == mode || 'r' != *mode || GUCEF_NULL != strchr( mode, '+' ) )
    {
        Refuse( "opening a file for writing" );
        return VFS::TBasicVfsResourcePtr();
    }
    if ( !IsLoaded() )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, "the archive is not loaded" ) );
        return VFS::TBasicVfsResourcePtr();
    }
    CORE::CString path;
    if ( !NormalizePath( file, path ) || path.IsNULLOrEmpty() )
    {
        Record( CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, "\"" + file + "\" is not a file of the share" ) );
        return VFS::TBasicVfsResourcePtr();
    }

    // a thread that must not wait opens only what is held completely, when the settings say so
    CPolicyScopeHolder policyScope;
    if ( m_settings.openRequiresCompleteFile )
    {
        VFS::CVfsThreadPolicy policy = VFS::CVfsGlobal::Instance()->GetThreadPolicyRegistry().GetCurrent();
        if ( policy.NeverWaits() && !policy.openRequiresCompleteFile )
        {
            policy.openRequiresCompleteFile = true;
            policyScope.Enter( policy );
        }
    }

    // without a block cache the file is read through the handles of the facility, as it always was: a handle is taken at the open, which pins the file at the
    // revision it has then. A thread that must not wait cannot do that and goes through the generic layer, which says what it can do for it
    VFS::CVfsRemoteArchiveCorePtr core = GetCore();
    if ( !core.IsNULL() && core->GetBlockCache().IsNULL() && !core->GetCurrentThreadPolicy().NeverWaits() )
    {
        CDrnBridgeResult opened;
        CDrnBridgeFileAccess* access = CDrnBridgeHandlePool::OpenFile( m_backend->GetPool(), path, opened, CORE::CString::Empty, true );
        m_backend->NoteOutcome( opened );
        Record( opened );
        core->RecordCallResult( opened.ToCallResult() );
        if ( GUCEF_NULL == access )
            return VFS::TBasicVfsResourcePtr();
        CORE::IOAccessPtr fileAccess( access );
        return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( fileAccess, file, CORE::CombinePath( m_archivePath, path ) ), this );
    }

    Record( CDrnBridgeResult() );
    VFS::TBasicVfsResourcePtr resource = VFS::CVfsRemoteArchive::GetFile( path, mode, memLoadSize, overwrite );
    Settle();
    return resource;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::StoreAsFile( const CORE::CString& filepath    ,
                                const CORE::CDynamicBuffer& data ,
                                const CORE::UInt64 offset        ,
                                const bool overwrite             )
{GUCEF_TRACE;

    return Refuse( "storing a file" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    return Refuse( "deleting a file" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::MoveFile( const VFS::CString& oldFilePath ,
                             const VFS::CString& newFilePath ,
                             const bool overwrite            )
{GUCEF_TRACE;

    return Refuse( "moving a file" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::MoveFileAtomically( const VFS::CString& oldFilePath ,
                                       const VFS::CString& newFilePath ,
                                       const bool overwrite            )
{GUCEF_TRACE;

    return Refuse( "moving a file" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::CreateDir( const VFS::CString& dirPath )
{GUCEF_TRACE;

    return Refuse( "making a directory" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::DeleteDir( const VFS::CString& dirPath, bool deleteContents )
{GUCEF_TRACE;

    return Refuse( "deleting a directory" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeArchive::MoveDir( const VFS::CString& oldDirPath, const VFS::CString& newDirPath )
{GUCEF_TRACE;

    return Refuse( "moving a directory" );
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
