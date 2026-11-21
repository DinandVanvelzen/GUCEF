/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_VFS_CVFSDIGESTHASHER_H
#include "gucefVFS_CVfsDigestHasher.h"
#define GUCEF_VFS_CVFSDIGESTHASHER_H
#endif /* GUCEF_VFS_CVFSDIGESTHASHER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#include "gucefDRN_CDRNResourceHasher.h"
#define GUCEF_DRN_CDRNRESOURCEHASHER_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */

#include "vfspluginDRN_CDrnBridgeGlobal.h"

#include "TestArchiveConformanceTree.h"
#include "TestDrnBridgeFixture.h"
#include "TestDrnRemoteEnv.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNBRIDGETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPlaceProvider::CPlaceProvider( const CORE::CString& root, ILinkCutter* cutter, IPlaceHook* hook )
    : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) )
    , m_lock()
    , m_delayMs( 0 )
    , m_cutAfterBytes( 0 )
    , m_cutArmed( false )
    , m_cutLink( false )
    , m_cutServed( 0 )
    , m_hookAfter( 0 )
    , m_hookReads( 0 )
    , m_hookPath()
    , m_provided()
    , m_wrong()
    , m_corrupt( false )
    , m_statCount( 0 )
    , m_listCount( 0 )
    , m_readCount( 0 )
    , m_cutter( cutter )
    , m_hook( hook )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<4; ++i )
    {
        m_failCount[ i ] = 0;
        m_failHangs[ i ] = false;
    }
}

/*-------------------------------------------------------------------------*/

CPlaceProvider::~CPlaceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::SetDelayMs( UInt32 delayMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_delayMs = delayMs;
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::FailNext( CTestRemotePlace::EOperation operation, UInt32 count, bool hang )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_failCount[ operation ] = count;
    m_failHangs[ operation ] = hang;
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::CutNextReadsAfter( UInt64 bytes, bool cutLink )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cutLink = cutLink;
    m_cutAfterBytes = bytes;
    m_cutServed = 0;
    m_cutArmed = true;
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::SetChangeHook( UInt32 afterReadNumber, const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_hookAfter = afterReadNumber;
    m_hookReads = 0;
    m_hookPath = path;
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::ProvideDigest( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_provided.insert( path );
    m_wrong.erase( path );
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::ProvideWrongDigest( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_provided.insert( path );
    m_wrong.insert( path );
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::SetCorrupt( bool corrupt )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_corrupt = corrupt;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlaceProvider::RevisionOf( const CORE::CString& name )
{GUCEF_TRACE;

    DRN::CDRNResourceInfo info;
    if ( DRN::DRN_RESSTATUS_OK != DRN::CDRNFileSystemAccessResourceProvider::Resolve( name, info ) )
        return CORE::CString::Empty;
    return info.revision;
}

/*-------------------------------------------------------------------------*/

UInt32
CPlaceProvider::GetStatCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_statCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CPlaceProvider::GetListCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_listCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CPlaceProvider::GetReadCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_readCount;
}

/*-------------------------------------------------------------------------*/

void
CPlaceProvider::ResetReadCount( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_readCount = 0;
}

/*-------------------------------------------------------------------------*/

bool
CPlaceProvider::Prelude( CTestRemotePlace::EOperation operation ) const
{GUCEF_TRACE;

    UInt32 delayMs = 0;
    bool fail = false;
    bool hang = false;
    {
        MT::CScopeMutex lock( m_lock );
        delayMs = m_delayMs;
        if ( m_failCount[ operation ] > 0 )
        {
            --m_failCount[ operation ];
            fail = true;
            hang = m_failHangs[ operation ];
        }
    }
    if ( delayMs > 0 )
        MT::ThreadDelay( delayMs );
    if ( fail && hang )
    {
        // longer than a call of the client waits: the call ends at its own timeout, the answer that comes then is nobody's
        MT::ThreadDelay( 2500 );
        return false;
    }
    return fail;
}

/*-------------------------------------------------------------------------*/

DRN::TDRNResourceStatus
CPlaceProvider::Resolve( const CORE::CString& name, DRN::CDRNResourceInfo& info )
{GUCEF_TRACE;

    if ( Prelude( CTestRemotePlace::OP_OPEN ) )
        return DRN::DRN_RESSTATUS_IO_ERROR;
    return DRN::CDRNFileSystemAccessResourceProvider::Resolve( name, info );
}

/*-------------------------------------------------------------------------*/

DRN::TDRNResourceStatus
CPlaceProvider::Stat( const CORE::CString& name, DRN::CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        ++m_statCount;
    }
    if ( Prelude( CTestRemotePlace::OP_STAT ) )
        return DRN::DRN_RESSTATUS_IO_ERROR;
    return DRN::CDRNFileSystemAccessResourceProvider::Stat( name, entry );
}

/*-------------------------------------------------------------------------*/

DRN::TDRNResourceStatus
CPlaceProvider::OpenEnumerator( const CORE::CString& dirName, DRN::CIDRNNamespaceEnumeratorPtr& enumerator )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        ++m_listCount;
    }
    if ( Prelude( CTestRemotePlace::OP_LIST ) )
        return DRN::DRN_RESSTATUS_IO_ERROR;
    return DRN::CDRNFileSystemAccessResourceProvider::OpenEnumerator( dirName, enumerator );
}

/*-------------------------------------------------------------------------*/

DRN::TDRNResourceStatus
CPlaceProvider::Read( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead )
{GUCEF_TRACE;

    bool cut = false;
    bool corrupt = false;
    CORE::CString changePath;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_readCount;
        corrupt = m_corrupt;
        if ( m_cutArmed )
        {
            m_cutServed += size;
            if ( m_cutServed > m_cutAfterBytes )
            {
                m_cutArmed = false;
                cut = true;
            }
        }
        if ( m_hookAfter > 0 )
        {
            if ( ++m_hookReads >= m_hookAfter )
            {
                changePath = m_hookPath;
                m_hookAfter = 0;
            }
        }
    }

    if ( Prelude( CTestRemotePlace::OP_FETCH ) )
        return DRN::DRN_RESSTATUS_IO_ERROR;
    if ( cut )
    {
        // the transfer of this chunk fails: it does not arrive. For a cut of the link the link goes as well
        bool cutLink = false;
        {
            MT::CScopeMutex lock( m_lock );
            cutLink = m_cutLink;
        }
        if ( cutLink && GUCEF_NULL != m_cutter )
            m_cutter->CutLink();
        bytesRead = 0;
        return cutLink ? DRN::DRN_RESSTATUS_LINK_LOST : DRN::DRN_RESSTATUS_IO_ERROR;
    }

    const DRN::TDRNResourceStatus status = DRN::CDRNFileSystemAccessResourceProvider::Read( name, resolved, offset, size, destination, bytesRead );
    if ( DRN::DRN_RESSTATUS_OK == status && corrupt && bytesRead > 0 )
        static_cast< UInt8* >( destination )[ 0 ] ^= 0x5A;
    if ( !changePath.IsNULLOrEmpty() && GUCEF_NULL != m_hook )
        m_hook->OnPlaceHook( changePath );
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CPlaceProvider::GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const
{GUCEF_TRACE;

    bool wrong = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_provided.find( name ) == m_provided.end() )
            return false;
        wrong = m_wrong.find( name ) != m_wrong.end();
    }
    if ( !entry.IsFile() )
        return false;

    CORE::CFileAccess file( CORE::CombinePath( GetRootPath(), name ), "rb" );
    if ( !file.Opened() )
        return false;
    DRN::CDRNResourceHasher hasher;
    hasher.Begin();
    CORE::CDynamicBuffer chunk( 65536, true );
    while ( true )
    {
        const UInt32 got = file.Read( chunk.GetBufferPtr(), 1, 65536 );
        if ( 0 == got )
            break;
        hasher.Update( chunk.GetConstBufferPtr(), got );
    }
    hasher.Final( digest );
    if ( wrong )
        digest[ 0 ] ^= 0xFF;
    return true;
}

/*-------------------------------------------------------------------------*/

class CDrnRemoteEnv::CProxyPump : public MT::CActiveObject
{
    public:

    explicit CProxyPump( CDrnRemoteEnv& env ) : MT::CActiveObject(), m_env( env ) {}
    virtual ~CProxyPump() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex lock( m_env.m_proxyLock );
            m_env.m_proxyPulse->ForceDirectPulse();
        }
        MT::ThreadDelay( 1 );
        return false;
    }

    private:

    CDrnRemoteEnv& m_env;
};

/*-------------------------------------------------------------------------*/

CDrnRemoteEnv::CDrnRemoteEnv( const char* name, bool viaProxy )
    : CRemoteEnv()
    , CTestRemotePlace()
    , m_name( name )
    , m_viaProxy( viaProxy )
    , m_clientOwnThread( true )
    , m_extraKeys()
    , m_extraValues()
    , m_options()
    , m_cacheFolder( GUCEF_NULL )
    , m_serveFolder( GUCEF_NULL )
    , m_group( GUCEF_NULL )
    , m_serverPeerId( 0 )
    , m_sessionName()
    , m_registered( false )
    , m_provider( GUCEF_NULL )
    , m_providerPtr()
    , m_events()
    , m_files()
    , m_filesLock()
    , m_proxyPulse()
    , m_proxy( GUCEF_NULL )
    , m_pump( GUCEF_NULL )
    , m_proxyLock()
    , m_carriedFetches( 0 )
    , m_carriedBytes( 0 )
    , m_carriedOpens( 0 )
    , m_baseFetches( 0 )
    , m_baseBytes( 0 )
    , m_baseOpens( 0 )
    , m_baseStats( 0 )
    , m_baseLists( 0 )
    , m_corrupt( false )
    , m_mounted( false )
    , m_setUp( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnRemoteEnv::~CDrnRemoteEnv()
{GUCEF_TRACE;

    Teardown();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnRemoteEnv::FilePath( const CORE::CString& path ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_serveFolder->GetPath(), path );
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::WriteFile( const CORE::CString& path, const SFileState& state )
{GUCEF_TRACE;

    const CORE::CString full = FilePath( path );
    if ( !CORE::CreatePathDirectories( full ) )
        return false;
    CORE::CFileAccess file( full, "wb" );
    if ( !file.Opened() )
        return false;
    CORE::CDynamicBuffer block( 65536, true );
    UInt64 position = 0;
    while ( position < state.size )
    {
        const UInt32 count = (UInt32) ( state.size - position < 65536 ? state.size - position : 65536 );
        CArchiveConformanceTree::FillContent( state.contentId + state.version * 977, position, block.GetBufferPtr(), count );
        if ( count != file.Write( block.GetConstBufferPtr(), 1, count ) )
            return false;
        position += count;
    }
    file.Flush();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::AddFile( const CORE::CString& path, UInt32 contentId, UInt64 size )
{GUCEF_TRACE;

    SFileState state;
    state.contentId = contentId;
    state.size = size;
    state.version = 0;
    WriteFile( path, state );
    MT::CScopeMutex lock( m_filesLock );
    m_files[ path ] = state;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::StartServing( void )
{GUCEF_TRACE;

    // the name is the one the archive is mounted under: a restarted serving node takes the same one
    static UInt32 counter = 0;
    if ( m_sessionName.IsNULLOrEmpty() )
        m_sessionName = "drnremoteenv-" + m_name + "-" + CORE::ToString( ++counter );
    m_group = GUCEF_NEW DRNTEST::CGroup( DRN::DRN_TOPOLOGY_CLIENT_SERVER, m_sessionName );
    m_group->nodeSettings.ownThread = true;
    if ( !m_group->Start( 1 ) )
        return false;
    {
        MT::CScopeMutex lock( m_group->sessions[ 0 ]->GetNode().GetLock() );
        m_serverPeerId = m_group->sessions[ 0 ]->GetLocalPeerId();
    }

    m_provider = GUCEF_NEW CPlaceProvider( m_serveFolder->GetPath(), this, this );
    m_providerPtr = DRN::CIDRNResourceProviderPtr( m_provider );
    if ( !m_group->sessions[ 0 ]->GetResources().RegisterProvider( CLoopbackShare::ShareName, m_providerPtr, DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole ) ) )
        return false;

    if ( m_viaProxy )
    {
        m_proxyPulse = CORE::PulseGeneratorPtr( GUCEF_NEW CORE::CPulseGenerator() );
        m_proxy = GUCEF_NEW DRNUTILSTEST::CTestTcpFaultProxy();
        if ( !m_proxy->Start( m_proxyPulse, m_group->endpoints[ 0 ].GetPort() ) )
            return false;
        m_pump = GUCEF_NEW CProxyPump( *this );
        if ( !m_pump->Activate( GUCEF_NULL, 1, 1 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::WaitStep( void )
{GUCEF_TRACE;

    // without a thread of its own the client node is worked by the thread that waits
    if ( GUCEF_NULL != m_group && m_group->sessions.size() > 1 && !m_clientOwnThread && m_group->nodes[ 1 ]->IsPumpThread() )
        m_group->nodes[ 1 ]->PumpOnce();
    else
        MT::ThreadDelay( 2 );
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::IsClientLinkUp( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_group || m_group->sessions.size() < 2 )
        return false;
    DRN::CDRNSession& client = *m_group->sessions[ 1 ];
    MT::CScopeMutex lock( client.GetNode().GetLock() );
    if ( !client.IsActive() )
        return false;
    DRN::CDRNPeerLinkPtr link = client.GetLinkToMember( m_serverPeerId );
    return !link.IsNULL() && link->IsOperational();
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::JoinClient( void )
{GUCEF_TRACE;

    if ( m_group->sessions.size() < 2 )
    {
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        DRN::CDRNNodeSettings clientSettings = m_group->nodeSettings;
        clientSettings.ownThread = m_clientOwnThread;
        node->SetSettings( clientSettings );
        m_group->nodes.push_back( node );
        m_group->endpoints.push_back( DRNTEST::LoopbackEndpoint( m_group->channelType, DRNTEST::NextTestPort(), m_group->params ) );
        m_group->sessions.push_back( node->CreateSession() );
        if ( !node->Listen( m_group->endpoints.back() ) )
            return false;
    }

    DRN::CDRNEndpoint endpoint = m_group->endpoints[ 0 ];
    if ( m_viaProxy )
        endpoint = DRNTEST::LoopbackEndpoint( m_group->channelType, m_proxy->GetPort(), m_group->params );

    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = m_sessionName;
    if ( !m_group->sessions[ 1 ]->Join( DRN::CDRNEndpointList( endpoint ), joinSettings ) )
        return false;

    const UInt64 start = DRN::CDRNClock::NowInMilliseconds();
    while ( DRN::CDRNClock::NowInMilliseconds() - start < 10000 )
    {
        if ( IsClientLinkUp() )
            return true;
        WaitStep();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::MountArchive( void )
{GUCEF_TRACE;

    const CORE::CString source = "drn://" + m_sessionName + "/sequencer/" + CLoopbackShare::ShareName;

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeSettings bridge;
    bridge.metadataTtlMs = m_options.metadataTtlMs;
    bridge.negativeTtlMs = m_options.metadataTtlMs;
    bridge.maxCachedEntries = m_options.maxMetadataEntries;
    bridge.callTimeoutMs = 1500;
    bridge.openTimeoutMs = 3000;
    if ( m_options.useCache )
    {
        bridge.blockCacheFolder = m_cacheFolder->GetPath();
        bridge.blockCacheMaxBytes = m_options.cacheMaxBytes;
        bridge.blockCacheBlockBytes = m_options.blockBytes;
    }
    bridge.cacheIdentity = "remote-test-place";
    bridge.readAheadAfterBlocks = m_options.readAheadAfterBlocks;
    bridge.readAheadBlocks = m_options.readAheadBlocks;
    bridge.maxBlocksPerFetch = m_options.maxBlocksPerFetch;
    bridge.maxBackgroundFetches = m_options.maxRunning;
    bridge.maxQueuedFetches = m_options.maxQueued;
    bridge.fetchThreadPool = "VfsRemoteOpsTest";
    bridge.offlineListings = m_options.offlineListings;
    bridge.verifyCompleteFiles = m_options.verifyCompleteFiles;
    bridge.refetchPinned = m_options.refetchPinned;

    CORE::CDataNode config;
    config.SetAttribute( "archiveName", CORE::CString( ArchiveName ) );
    config.SetAttribute( "archiveType", CORE::CString( "drn" ) );
    config.SetAttribute( "actualArchivePath", source );
    config.SetAttribute( "path", source );
    config.SetAttribute( "mountPath", CORE::CString( MountPath ) );
    config.SetAttribute( "writeable", false );
    bridge.SaveToArchiveConfig( config );
    for ( size_t e=0; e<m_extraKeys.size(); ++e )
        config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + m_extraKeys[ e ] ).C_String(), m_extraValues[ e ] );
    VFS::CArchiveSettings settings;
    settings.LoadConfig( config );
    settings.SetActualArchivePath( source );

    if ( !Vfs().MountArchive( settings ) )
        return false;
    m_mounted = true;
    m_events.Listen( &Vfs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::Setup( const SRemoteEnvOptions& options )
{GUCEF_TRACE;

    Teardown();

    // makes the archive type "drn" known to the VFS
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();

    m_options = options;
    m_sessionName.Clear();
    m_cacheFolder = GUCEF_NEW CBlockCacheTestFolder( ( m_name + "-cache" ).C_String() );
    m_serveFolder = GUCEF_NEW CBlockCacheTestFolder( ( m_name + "-share" ).C_String() );

    AddFile( ReadmeFile, 1, 1000 );
    AddFile( NotesFile, 2, 70000 );
    AddFile( BigFile, 3, 1000000 );
    AddFile( MidFile, 4, 300000 );
    AddFile( EmptyFile, 5, 0 );
    AddFile( TopFile, 6, 100 );
    AddFile( DeepFile, 7, 5000 );

    if ( !StartServing() || !JoinClient() )
        return false;
    DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( *m_group->sessions[ 1 ] );
    m_registered = true;

    if ( !MountArchive() )
        return false;
    m_events.Clear();
    ResetCounters();
    m_setUp = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::CarryCounters( void )
{GUCEF_TRACE;

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = GetBridge();
    if ( GUCEF_NULL == bridge )
        return;
    m_carriedFetches += bridge->GetFetchCallCount();
    m_carriedBytes += bridge->GetBytesFetched();
    m_carriedOpens += ClientOpenCount();
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::Unmount( void )
{GUCEF_TRACE;

    CarryCounters();
    m_mounted = false;
    return Vfs().UnmountArchiveByName( ArchiveName );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::CloseCache( void )
{GUCEF_TRACE;

    // the cache is closed when the archive that holds it is unmounted: Restart() does that
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::Restart( bool /* reopenCache */ )
{GUCEF_TRACE;

    Unmount();

    // the process ends: the cache of the run is closed (its last holder was the archive) and a new archive opens the folder again
    return MountArchive();
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::StopServing( void )
{GUCEF_TRACE;

    if ( m_registered && GUCEF_NULL != m_group && m_group->sessions.size() > 1 )
        DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( *m_group->sessions[ 1 ] );
    m_registered = false;

    GUCEF_DELETE m_pump;
    m_pump = GUCEF_NULL;
    if ( GUCEF_NULL != m_proxy )
    {
        m_proxy->Stop();
        GUCEF_DELETE m_proxy;
        m_proxy = GUCEF_NULL;
    }
    m_proxyPulse.Unlink();

    GUCEF_DELETE m_group;
    m_group = GUCEF_NULL;
    m_providerPtr.Unlink();
    m_provider = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::Teardown( void )
{GUCEF_TRACE;

    if ( !m_group && !m_cacheFolder && !m_serveFolder )
        return;

    if ( VFS::CVfsGlobal::Instance() && m_mounted )
        Vfs().UnmountArchiveByName( ArchiveName );
    m_mounted = false;
    m_events.UnsubscribeAllFromObserver();
    m_events.Clear();

    StopServing();

    GUCEF_DELETE m_cacheFolder;
    m_cacheFolder = GUCEF_NULL;
    GUCEF_DELETE m_serveFolder;
    m_serveFolder = GUCEF_NULL;
    m_files.clear();
    m_setUp = false;
}

/*-------------------------------------------------------------------------*/

VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive*
CDrnRemoteEnv::GetBridge( void )
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive = Vfs().GetMountedArchive( ArchiveName );
    if ( archive.IsNULL() )
        return GUCEF_NULL;
    return static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

DRNUTILSTEST::CTestTcpFaultProxy*
CDrnRemoteEnv::GetProxy( void )
{GUCEF_TRACE;

    return m_proxy;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::LockProxy( void )
{GUCEF_TRACE;

    m_proxyLock.Lock();
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::UnlockProxy( void )
{GUCEF_TRACE;

    m_proxyLock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnRemoteEnv::GetServedRoot( void ) const
{GUCEF_TRACE;

    return m_serveFolder->GetPath();
}

/*-------------------------------------------------------------------------*/

CTestRemotePlace&
CDrnRemoteEnv::Backend( void )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsRemoteArchiveCore*
CDrnRemoteEnv::Core( void )
{GUCEF_TRACE;

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = GetBridge();
    return GUCEF_NULL == bridge ? GUCEF_NULL : bridge->GetCore().GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsBlockCachePtr
CDrnRemoteEnv::Cache( void )
{GUCEF_TRACE;

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = GetBridge();
    return GUCEF_NULL == bridge ? VFS::CVfsBlockCachePtr() : bridge->GetBlockCache();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsFetchScheduler&
CDrnRemoteEnv::Scheduler( void )
{GUCEF_TRACE;

    return *GetBridge()->GetFetchScheduler();
}

/*-------------------------------------------------------------------------*/

CTestEventRecorder&
CDrnRemoteEnv::Events( void )
{GUCEF_TRACE;

    return m_events;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnRemoteEnv::GetPlaceText( void ) const
{GUCEF_TRACE;

    return CORE::CString( "\"" ) + CLoopbackShare::ShareName + "\" of member sequencer";
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CDrnRemoteEnv::GetCutFetchResult( void ) const
{GUCEF_TRACE;

    // the serving side fails the transfer: the link itself stays
    return CORE::FILESYSTEMACCESS_FAILED;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::HasHandleStatistics( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnRemoteEnv::GetCacheFolder( void ) const
{GUCEF_TRACE;

    return m_cacheFolder->GetPath();
}

/*-------------------------------------------------------------------------*/

const SRemoteEnvOptions&
CDrnRemoteEnv::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::ChangeFile( const CORE::CString& path )
{GUCEF_TRACE;

    SFileState state;
    {
        MT::CScopeMutex lock( m_filesLock );
        TFileStateMap::iterator i = m_files.find( path );
        if ( i == m_files.end() )
            return;
        ++i->second.version;
        state = i->second;
    }
    ChangeFileSize( path, state.size );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::ChangeFileSize( const CORE::CString& path, UInt64 newSize )
{GUCEF_TRACE;

    SFileState state;
    {
        MT::CScopeMutex lock( m_filesLock );
        TFileStateMap::iterator i = m_files.find( path );
        if ( i == m_files.end() )
            return;
        if ( newSize != i->second.size )
            ++i->second.version;
        i->second.size = newSize;
        state = i->second;
    }

    // the revision of the share holds the modification time: it has to have moved, whatever the clock of the file system does
    const CORE::CString before = m_provider->RevisionOf( path );
    for ( UInt32 attempt=0; attempt<200; ++attempt )
    {
        MT::ThreadDelay( 12 );
        WriteFile( path, state );
        if ( m_provider->RevisionOf( path ) != before )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::ProvideDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm /* algorithm */ )
{GUCEF_TRACE;

    m_provider->ProvideDigest( path );
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = GetBridge();
    if ( GUCEF_NULL != bridge )
        bridge->InvalidateMetadata( path );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::ProvideWrongDigest( const CORE::CString& path )
{GUCEF_TRACE;

    m_provider->ProvideWrongDigest( path );
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = GetBridge();
    if ( GUCEF_NULL != bridge )
        bridge->InvalidateMetadata( path );
}

/*-------------------------------------------------------------------------*/

VFS::CVfsDigest
CDrnRemoteEnv::ComputeDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm algorithm ) const
{GUCEF_TRACE;

    SFileState state;
    {
        MT::CScopeMutex lock( m_filesLock );
        TFileStateMap::const_iterator i = m_files.find( path );
        if ( i == m_files.end() )
            return VFS::CVfsDigest();
        state = i->second;
    }
    VFS::CVfsDigestHasher hasher( algorithm );
    CORE::CDynamicBuffer chunk( 65536, true );
    UInt64 position = 0;
    while ( position < state.size )
    {
        const UInt32 count = (UInt32) ( state.size - position < 65536 ? state.size - position : 65536 );
        CArchiveConformanceTree::FillContent( state.contentId + state.version * 977, position, chunk.GetBufferPtr(), count );
        hasher.Update( chunk.GetConstBufferPtr(), count );
        position += count;
    }
    return hasher.Final();
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::FillExpected( const CORE::CString& path, UInt64 offset, void* dest, UInt32 count ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_filesLock );
    TFileStateMap::const_iterator i = m_files.find( path );
    if ( i == m_files.end() || offset + count > i->second.size )
        return false;
    CArchiveConformanceTree::FillContent( i->second.contentId + i->second.version * 977, offset, dest, count );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::CheckExpected( const CORE::CString& path, UInt64 offset, const void* data, UInt32 count ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer expected( count, true );
    if ( 0 == count )
        return true;
    if ( !FillExpected( path, offset, expected.GetBufferPtr(), count ) )
        return false;
    return 0 == memcmp( expected.GetConstBufferPtr(), data, count );
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnRemoteEnv::GetSize( const CORE::CString& path ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_filesLock );
    TFileStateMap::const_iterator i = m_files.find( path );
    return i == m_files.end() ? 0 : i->second.size;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnRemoteEnv::GetRevision( const CORE::CString& path ) const
{GUCEF_TRACE;

    return m_provider->RevisionOf( path );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::CutLink( void )
{GUCEF_TRACE;

    if ( m_viaProxy )
    {
        MT::CScopeMutex lock( m_proxyLock );
        m_proxy->SetRefuseNew( true );
        m_proxy->Cut();
    }
    else
    if ( GUCEF_NULL != m_group && m_group->sessions.size() > 1 )
        m_group->sessions[ 1 ]->Leave();
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::OnPlaceHook( const CORE::CString& path )
{GUCEF_TRACE;

    ChangeFile( path );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::SetLinkUp( bool linkUp )
{GUCEF_TRACE;

    if ( !linkUp )
    {
        CutLink();
        const UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        while ( IsClientLinkUp() && DRN::CDRNClock::NowInMilliseconds() - start < 5000 )
            WaitStep();
        return;
    }

    if ( IsClientLinkUp() )
        return;
    if ( m_viaProxy )
    {
        MT::CScopeMutex lock( m_proxyLock );
        m_proxy->SetRefuseNew( false );
    }
    if ( m_group->sessions[ 1 ]->IsActive() )
        m_group->sessions[ 1 ]->Leave();
    JoinClient();
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::SetDelayMs( UInt32 delayMs )
{GUCEF_TRACE;

    m_provider->SetDelayMs( delayMs );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::FailNext( EOperation operation, UInt32 count, CORE::TFileSystemAccessResult result )
{GUCEF_TRACE;

    m_provider->FailNext( operation, count, CORE::FILESYSTEMACCESS_TIMEOUT == result );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::CutNextFetchAfter( UInt64 bytes )
{GUCEF_TRACE;

    m_provider->CutNextReadsAfter( bytes, false );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::SetCorruptContent( bool corrupt )
{GUCEF_TRACE;

    m_corrupt = corrupt;
    m_provider->SetCorrupt( corrupt );
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::SetOnFetchHook( UInt32 afterFetchNumber, const CORE::CString& pathToChange )
{GUCEF_TRACE;

    // a run of the layer is a number of chunks of the serving side's transfer: the hook counts those
    const UInt32 chunkBytes = 65536;
    const UInt32 chunksPerRun = m_options.maxBlocksPerFetch * ( m_options.blockBytes / chunkBytes > 0 ? m_options.blockBytes / chunkBytes : 1 );
    m_provider->SetChangeHook( 0 == afterFetchNumber ? 0 : afterFetchNumber * chunksPerRun, pathToChange );
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::GetStatCount( void ) const
{GUCEF_TRACE;

    return m_provider->GetStatCount() - m_baseStats;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::GetListCount( void ) const
{GUCEF_TRACE;

    return m_provider->GetListCount() - m_baseLists;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::ClientOpenCount( void ) const
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( ArchiveName );
    if ( archive.IsNULL() )
        return 0;
    return (UInt32) static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetConnection()->GetOpenCallCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::GetOpenCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( m_carriedOpens + ClientOpenCount() - m_baseOpens );
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::GetFetchCount( void ) const
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( ArchiveName );
    const UInt64 current = archive.IsNULL() ? 0 : static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetFetchCallCount();
    return (UInt32) ( m_carriedFetches + current - m_baseFetches );
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnRemoteEnv::GetBytesServed( void ) const
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( ArchiveName );
    const UInt64 current = archive.IsNULL() ? 0 : static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetBytesFetched();
    return m_carriedBytes + current - m_baseBytes;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::ResetCounters( void )
{GUCEF_TRACE;

    m_baseStats = m_provider->GetStatCount();
    m_baseLists = m_provider->GetListCount();
    m_baseOpens = m_carriedOpens + ClientOpenCount();
    VFS::CVFS::TArchivePtr archive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( ArchiveName );
    const UInt64 fetches = archive.IsNULL() ? 0 : static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetFetchCallCount();
    const UInt64 bytes = archive.IsNULL() ? 0 : static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetBytesFetched();
    m_baseFetches = m_carriedFetches + fetches;
    m_baseBytes = m_carriedBytes + bytes;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::SetClientOwnThread( bool ownThread )
{GUCEF_TRACE;

    m_clientOwnThread = ownThread;
}

/*-------------------------------------------------------------------------*/

void
CDrnRemoteEnv::AddBridgeSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    m_extraKeys.push_back( key );
    m_extraValues.push_back( value );
}

/*-------------------------------------------------------------------------*/

CPlaceProvider*
CDrnRemoteEnv::GetProvider( void )
{GUCEF_TRACE;

    return m_provider;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CDrnRemoteEnv::Server( void )
{GUCEF_TRACE;

    return *m_group->sessions[ 0 ];
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CDrnRemoteEnv::Client( void )
{GUCEF_TRACE;

    return *m_group->sessions[ 1 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnRemoteEnv::GetServerPeerId( void ) const
{GUCEF_TRACE;

    return m_serverPeerId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnRemoteEnv::GetSessionName( void ) const
{GUCEF_TRACE;

    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::ReplaceClientSession( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_group || m_group->sessions.size() < 2 )
        return false;
    if ( m_registered )
        DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( *m_group->sessions[ 1 ] );
    m_registered = false;

    // the node of the client ends with its session; the next client node and session are made by JoinClient() and take the name
    DRN::CDRNNode* node = m_group->nodes[ 1 ];
    node->Stop();
    GUCEF_DELETE node;
    m_group->nodes.erase( m_group->nodes.begin() + 1 );
    m_group->sessions.erase( m_group->sessions.begin() + 1 );
    m_group->endpoints.erase( m_group->endpoints.begin() + 1 );

    if ( !JoinClient() )
        return false;
    m_registered = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( *m_group->sessions[ 1 ] );
    return m_registered;
}

/*-------------------------------------------------------------------------*/

bool
CDrnRemoteEnv::RestartServer( void )
{GUCEF_TRACE;

    StopServing();
    m_baseStats = 0;
    m_baseLists = 0;
    if ( !StartServing() || !JoinClient() )
        return false;
    m_registered = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( *m_group->sessions[ 1 ] );
    return m_registered;
}

/*-------------------------------------------------------------------------*/

CRemoteEnv*
MakeLoopbackRemoteEnv( const char* name )
{GUCEF_TRACE;

    return GUCEF_NEW CDrnRemoteEnv( name, false );
}

/*-------------------------------------------------------------------------*/

CRemoteEnv*
MakeProxiedRemoteEnv( const char* name )
{GUCEF_TRACE;

    return GUCEF_NEW CDrnRemoteEnv( name, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGETEST */

/*-------------------------------------------------------------------------*/
