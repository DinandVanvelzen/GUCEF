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

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#include "gucefDRN_CDRNResourceHasher.h"
#define GUCEF_DRN_CDRNRESOURCEHASHER_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#include "TestDrnBridgeFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNBRIDGETEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
NativeFileSystem( void )
{GUCEF_TRACE;

    return ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CHashingProvider::CHashingProvider( const CORE::CString& root )
    : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) )
    , m_lock()
    , m_known()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHashingProvider::GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const
{GUCEF_TRACE;

    // the large file of the reference tree is a hole: nobody keeps a digest of that
    if ( !entry.IsFile() || entry.size > 8 * 1024 * 1024 )
        return false;

    {
        MT::CScopeMutex lock( m_lock );
        TDigestMap::const_iterator i = m_known.find( name );
        if ( i != m_known.end() && (*i).second.revision == entry.revision )
        {
            memcpy( digest, (*i).second.digest, 32 );
            return true;
        }
    }

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
    CKnownDigest known;
    known.revision = entry.revision;
    hasher.Final( known.digest );
    memcpy( digest, known.digest, 32 );

    MT::CScopeMutex lock( m_lock );
    m_known[ name ] = known;
    return true;
}

/*-------------------------------------------------------------------------*/

const char* const CLoopbackShare::ShareName = "files";

/*-------------------------------------------------------------------------*/

CLoopbackShare::CLoopbackShare( const CORE::CString& sessionName )
    : m_group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, sessionName )
    , m_sessionName( sessionName )
    , m_serverPeerId( 0 )
    , m_clientOwnThread( true )
    , m_started( false )
    , m_registered( false )
{GUCEF_TRACE;

    // sessions that carry mounts run their node on a thread of their own
    m_group.nodeSettings.ownThread = true;
}

/*-------------------------------------------------------------------------*/

CLoopbackShare::~CLoopbackShare()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

void
CLoopbackShare::SetClientOwnThread( bool ownThread )
{GUCEF_TRACE;

    m_clientOwnThread = ownThread;
}

/*-------------------------------------------------------------------------*/

bool
CLoopbackShare::Start( const CORE::CString& root, const DRN::CIDRNResourceProviderPtr& provider )
{GUCEF_TRACE;

    if ( m_clientOwnThread )
    {
        if ( !m_group.Start( 2 ) )
            return false;
    }
    else
    {
        // the serving node has a thread of its own, the client node has none: this thread pumps it, as the thread of an application's main loop does
        if ( !m_group.Start( 1 ) )
            return false;
        m_started = true;
        m_group.nodeSettings.ownThread = false;
        if ( !m_group.AddMember() || !m_group.nodes[ 1 ]->IsPumpThread() )
            return false;
    }
    m_started = true;
    {
        MT::CScopeMutex lock( Server().GetNode().GetLock() );
        m_serverPeerId = Server().GetLocalPeerId();
    }

    DRN::CIDRNResourceProviderPtr served = provider;
    if ( served.IsNULL() )
        served = DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) ) );
    return Server().GetResources().RegisterProvider( ShareName, served, DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole ) );
}

/*-------------------------------------------------------------------------*/

void
CLoopbackShare::Stop( void )
{GUCEF_TRACE;

    if ( m_registered )
    {
        DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( Client() );
        m_registered = false;
    }
    if ( m_started )
    {
        m_started = false;
        m_group.Stop();
    }
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CLoopbackShare::Server( void )
{GUCEF_TRACE;

    return m_group[ 0 ];
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CLoopbackShare::Client( void )
{GUCEF_TRACE;

    return m_group[ 1 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CLoopbackShare::GetServerPeerId( void ) const
{GUCEF_TRACE;

    return m_serverPeerId;
}

/*-------------------------------------------------------------------------*/

UInt32
CLoopbackShare::GetClientPeerId( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( Client().GetNode().GetLock() );
    return Client().GetLocalPeerId();
}

/*-------------------------------------------------------------------------*/

void
CLoopbackShare::DropClient( void )
{GUCEF_TRACE;

    Client().Leave();
}

/*-------------------------------------------------------------------------*/

bool
CLoopbackShare::RejoinClient( void )
{GUCEF_TRACE;

    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = m_sessionName;
    if ( !Client().Join( DRN::CDRNEndpointList( m_group.endpoints[ 0 ] ), joinSettings ) )
        return false;

    const UInt64 start = DRN::CDRNClock::NowInMilliseconds();
    while ( DRN::CDRNClock::NowInMilliseconds() - start < 10000 )
    {
        {
            MT::CScopeMutex lock( Client().GetNode().GetLock() );
            if ( Client().IsActive() )
            {
                DRN::CDRNPeerLinkPtr link = Client().GetLinkToMember( m_serverPeerId );
                if ( !link.IsNULL() && link->IsOperational() )
                    return true;
            }
        }
        // without a thread of its own the client node is worked by this thread, which is the one that waits
        if ( Client().GetNode().IsPumpThread() )
            Client().GetNode().PumpOnce();
        else
            MT::ThreadDelay( 5 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLoopbackShare::GetSessionName( void ) const
{GUCEF_TRACE;

    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNResourceTransferStats
CLoopbackShare::GetServerStats( void )
{GUCEF_TRACE;

    return Server().GetResources().GetStats();
}

/*-------------------------------------------------------------------------*/

void
CLoopbackShare::RegisterClientSession( void )
{GUCEF_TRACE;

    if ( !m_registered )
        m_registered = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Register( Client() );
}

/*-------------------------------------------------------------------------*/

VFS::CArchiveSettings
MakeBridgeArchiveSettings( const CORE::CString& archiveName, const CORE::CString& source, const CORE::CString& mountPath, const CBridgeMountOptions& options, const CORE::CString& extraSettingKey, const CORE::CString& extraSettingValue )
{GUCEF_TRACE;

    // as an application's configuration holds it: the archive specific settings are attributes with the prefix
    CORE::CDataNode config;
    config.SetAttribute( "archiveName", archiveName );
    config.SetAttribute( "archiveType", CORE::CString( "drn" ) );
    config.SetAttribute( "actualArchivePath", source );
    config.SetAttribute( "path", source );
    config.SetAttribute( "mountPath", mountPath );
    config.SetAttribute( "writeable", false );
    config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "metadataTtlMs" ).C_String(), CORE::UInt32ToString( options.metadataTtlMs ) );
    config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "negativeTtlMs" ).C_String(), CORE::UInt32ToString( options.negativeTtlMs ) );
    config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "maxOpenHandles" ).C_String(), CORE::UInt32ToString( options.maxOpenHandles ) );
    config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "chunkCacheChunks" ).C_String(), CORE::UInt32ToString( options.chunkCacheChunks ) );
    config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "readAheadChunks" ).C_String(), CORE::UInt32ToString( options.readAheadChunks ) );
    if ( !options.verifyMode.IsNULLOrEmpty() )
        config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + "verifyMode" ).C_String(), options.verifyMode );
    if ( !extraSettingKey.IsNULLOrEmpty() )
        config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + extraSettingKey ).C_String(), extraSettingValue );

    VFS::CArchiveSettings settings;
    settings.LoadConfig( config );
    return settings;
}

/*-------------------------------------------------------------------------*/

VFS::CVFS::TArchivePtr
CreateBridgeArchive( const CORE::CString& archiveName, const CORE::CString& source, const CBridgeMountOptions& options, const CORE::CString& extraSettingKey, const CORE::CString& extraSettingValue )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TArchivePtr archive = vfs.CreateArchive( "drn" );
    if ( archive.IsNULL() )
        return archive;

    const VFS::CArchiveSettings settings = MakeBridgeArchiveSettings( archiveName, source, CORE::CString::Empty, options, extraSettingKey, extraSettingValue );
    if ( !archive->LoadArchive( settings ) )
        return VFS::CVFS::TArchivePtr();
    return archive;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConformanceFixture::CDrnBridgeConformanceFixture( const CORE::CString& name, const CBridgeMountOptions& options )
    : CArchiveConformanceFixture()
    , m_name( name )
    , m_options( options )
    , m_local( false )
    , m_capabilities()
    , m_share( GUCEF_NULL )
    , m_archive()
    , m_connectionId()
{GUCEF_TRACE;

    // what the bridge does and does not do, in one place; everything not named is off (the default of the capabilities)
    m_capabilities.isWritable = false;                      // read only first slice: writes are refused, IsWriteable() is false
    m_capabilities.supportsDirectoryWatching = false;       // no server pushed watches yet (a later phase): IsDirectoryWatchingSupported() is false
    m_capabilities.hasModificationTimes = true;             // the entry's modification time, with the resolution of the serving file system
    m_capabilities.modificationTimeResolutionInMs = 20;
    m_capabilities.revisionIsWeak = false;                  // the share has modification times: a same size change moves the revision
    m_capabilities.canMutateTree = true;                    // the fixture changes the share's directory behind the archive
    m_capabilities.canProvideOutsidePath = true;            // a file beside the share's directory, reached by ".." when nothing confines
    m_capabilities.supportsFileHash = options.shareKnowsDigests;    // only a share that knows its digests gives one: nothing is hashed for a call
    m_capabilities.fileHashIsMd5 = false;                   // it is a SHA-256 when there is one
    m_capabilities.digestAlgorithm = options.shareKnowsDigests ? VFS::CVfsDigest::ALGORITHM_SHA256 : VFS::CVfsDigest::ALGORITHM_UNKNOWN;     // GetFileDigest() names it when the share has one
    m_capabilities.supportsFileMetaData = true;
    m_capabilities.honoursMaxListEntries = true;
    m_capabilities.dirListHonoursIncludePath = true;
    m_capabilities.dirListRecursesIntoSubDirs = true;
    m_capabilities.dirEntriesHaveTrailingSeparator = true;
    m_capabilities.confinesPathsToArchive = true;           // refused on the client and again by the serving node
    m_capabilities.readHandlesRefuseWrites = true;
    m_capabilities.supportsLargeFiles = true;               // 64 bit sizes and offsets throughout, the file above 4 GiB is in the tree
    m_capabilities.storesEmptyFiles = false;                // writes: none
    m_capabilities.supportsPartialWrites = false;
    m_capabilities.supportsAtomicMove = false;
    m_capabilities.supportsDirOperations = false;
    m_capabilities.supportsConcurrentCallers = options.clientOwnThread;     // the thread of a pumped client node works for the calls: a second caller thread waits for it
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConformanceFixture::~CDrnBridgeConformanceFixture()
{GUCEF_TRACE;

    Teardown();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeConformanceFixture::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConformanceFixture::Setup( void )
{GUCEF_TRACE;

    Teardown();

    // the tree, in a directory of the local file system
    if ( !m_local.Setup() )
        return false;

    static UInt32 sessionCounter = 0;
    m_share = GUCEF_NEW CLoopbackShare( "vfsdrn_conf_" + CORE::UInt32ToString( MT::GetProcessID() ) + "_" + CORE::UInt32ToString( ++sessionCounter ) );
    m_share->SetClientOwnThread( m_options.clientOwnThread );
    DRN::CIDRNResourceProviderPtr provider;
    if ( m_options.shareKnowsDigests )
        provider = DRN::CIDRNResourceProviderPtr( GUCEF_NEW CHashingProvider( m_local.GetRootDir() ) );
    if ( !m_share->Start( m_local.GetRootDir(), provider ) )
        return false;

    // the plugin's global makes the archive type known to the VFS
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();

    CORE::CString source;
    if ( MOUNTROUTE_URI == m_options.route )
    {
        m_share->RegisterClientSession();
        source = "drn://" + m_share->GetSessionName() + "/sequencer/" + CLoopbackShare::ShareName;
    }
    else
    {
        m_connectionId = m_share->GetSessionName();
        if ( VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance()->RegisterConnection( m_connectionId, m_share->Client(), m_share->GetServerPeerId() ).IsNULL() )
            return false;
        source = m_connectionId + "|" + CLoopbackShare::ShareName;
    }

    m_archive = CreateBridgeArchive( "DrnArchiveConformance", source, m_options );
    return !m_archive.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConformanceFixture::Teardown( void )
{GUCEF_TRACE;

    // the archive first: its open files close their handles over the link, which still has to be there
    m_archive.Unlink();
    if ( !m_connectionId.IsNULLOrEmpty() )
    {
        VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance()->UnregisterConnection( m_connectionId );
        m_connectionId.Clear();
    }
    if ( GUCEF_NULL != m_share )
    {
        m_share->Stop();
        GUCEF_DELETE m_share;
        m_share = GUCEF_NULL;
    }
    m_local.Teardown();
}

/*-------------------------------------------------------------------------*/

VFS::CArchive*
CDrnBridgeConformanceFixture::GetArchive( void )
{GUCEF_TRACE;

    return m_archive.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive*
CDrnBridgeConformanceFixture::GetBridge( void )
{GUCEF_TRACE;

    return static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( m_archive.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

CLoopbackShare*
CDrnBridgeConformanceFixture::GetShare( void )
{GUCEF_TRACE;

    return m_share;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeConformanceFixture::GetRootDir( void ) const
{GUCEF_TRACE;

    return m_local.GetRootDir();
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceTree&
CDrnBridgeConformanceFixture::GetTree( void ) const
{GUCEF_TRACE;

    return m_local.GetTree();
}

/*-------------------------------------------------------------------------*/

const CArchiveConformanceCapabilities&
CDrnBridgeConformanceFixture::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConformanceFixture::NoteChange( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !m_options.invalidateOnChange || m_archive.IsNULL() )
        return;

    // what a change notification of the serving node would tell: the path and every directory above it have changed
    CORE::CString current = path;
    while ( true )
    {
        GetBridge()->InvalidateMetadata( current );
        const CORE::CString parent = VFSPLUGIN::DRNBRIDGE::CDrnBridgeMetadataCache::ParentOf( current );
        if ( current.IsNULLOrEmpty() )
            break;
        current = parent;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConformanceFixture::PutFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    const bool done = m_local.PutFile( path, content );
    NoteChange( path );
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConformanceFixture::RemoveFile( const CORE::CString& path )
{GUCEF_TRACE;

    const bool done = m_local.RemoveFile( path );
    NoteChange( path );
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConformanceFixture::GetPathOutsideArchive( CORE::CString& path )
{GUCEF_TRACE;

    return m_local.GetPathOutsideArchive( path );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGETEST */

/*-------------------------------------------------------------------------*/
