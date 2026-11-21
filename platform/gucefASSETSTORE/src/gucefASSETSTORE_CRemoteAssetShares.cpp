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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#include "gucefDRNUTILS_CRemoteShareMount.h"
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H ? */

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

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#include "gucefVFS_CVfsFetchProgress.h"
#define GUCEF_VFS_CVFSFETCHPROGRESS_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */

#ifndef GUCEF_VFS_CVFSFETCHREQUEST_H
#include "gucefVFS_CVfsFetchRequest.h"
#define GUCEF_VFS_CVFSFETCHREQUEST_H
#endif /* GUCEF_VFS_CVFSFETCHREQUEST_H ? */

#ifndef GUCEF_VFS_CVFSASYNCRESULT_H
#include "gucefVFS_CVfsAsyncResult.h"
#define GUCEF_VFS_CVFSASYNCRESULT_H
#endif /* GUCEF_VFS_CVFSASYNCRESULT_H ? */

#ifndef GUCEF_VFS_CVFSCACHESTATE_H
#include "gucefVFS_CVfsCacheState.h"
#define GUCEF_VFS_CVFSCACHESTATE_H
#endif /* GUCEF_VFS_CVFSCACHESTATE_H ? */

#ifndef GUCEF_VFS_CIREMOTEARCHIVE_H
#include "gucefVFS_CIRemoteArchive.h"
#define GUCEF_VFS_CIREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CIREMOTEARCHIVE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#include "gucefASSETSTORE_CRemoteAssetStoreService.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H ? */

#include "gucefASSETSTORE_CRemoteFileTransfer.h"
#include "gucefASSETSTORE_CRemoteFileTransferTaskData.h"
#include "gucefASSETSTORE_CRemoteFileTransferTaskConsumer.h"

#include "gucefASSETSTORE_CRemoteAssetShares.h"

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

const CORE::CEvent CRemoteAssetShares::StateChangedEvent = "GUCEF::ASSETSTORE::CRemoteAssetShares::StateChangedEvent";
const CORE::CString CRemoteAssetShares::DefaultShareName = "assets";

namespace {

/** A VFS path with '/' separators and no separator at either end */
CORE::CString
CleanRoot( const CORE::CString& path )
{
    return CIRemoteFileSource::Normalize( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinPath( const CORE::CString& prefix, const CORE::CString& path )
{
    if ( prefix.IsNULLOrEmpty() )
        return path;
    if ( path.IsNULLOrEmpty() )
        return prefix;
    return prefix + "/" + path;
}

/*-------------------------------------------------------------------------*/

/** The connection of a remote service, NULL for another kind of service or one that is not open */
DRNUTILS::CRemoteServiceConnection*
ConnectionOf( CIAssetStoreService& service )
{
    if ( CAssetStoreServiceConfig::RemoteImplementationId != service.GetType() )
        return GUCEF_NULL;
    return static_cast< CRemoteAssetStoreService& >( service ).GetConnection();
}

/*-------------------------------------------------------------------------*/

/** A path of the VFS below a mount point: the mount point itself or '/' after it */
bool
IsBelow( const CORE::CString& path, const CORE::CString& root )
{
    if ( path == root )
        return true;
    return path.Length() > root.Length() && '/' == path[ root.Length() ] && 0 == path.HasSubstr( root );
}

/*-------------------------------------------------------------------------*/

CORE::CString
LeafName( const CORE::CString& name )
{
    CORE::CString leaf = name;
    while ( leaf.Length() > 0 && '/' == leaf[ leaf.Length() - 1 ] )
        leaf = leaf.CutChars( 1, false );
    return leaf;
}

/*-------------------------------------------------------------------------*/

/**
 *  Waits for the future of a call of the VFS on a thread of the pool of the transfers (or any thread that may wait), looking at the cancel flag of the
 *  caller and handing the bytes of a fetch that arrived to its counter
 */
void
WaitForCall( CORE::CFutureResult& future, const MT::CTAtomic< bool >* cancel, VFS::CVfsFetchProgress* progress, MT::CTAtomic< UInt64 >* done )
{
    bool cancelSent = false;
    while ( !future.IsReady() )
    {
        if ( !cancelSent && GUCEF_NULL != cancel && cancel->Load() )
        {
            cancelSent = true;
            if ( GUCEF_NULL != progress )
                progress->cancel.Store( true );
            future.RequestCancellation();
        }
        if ( GUCEF_NULL != done && GUCEF_NULL != progress )
            done->Store( progress->bytesDone.Load() );
        MT::PrecisionDelay( 5 );
    }
    if ( GUCEF_NULL != done && GUCEF_NULL != progress )
        done->Store( progress->bytesDone.Load() );
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteAssetShares::SShare::SShare( void )
    : name()
    , serviceRoot()
    , mountPoint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetShares::SMounted::SMounted( void )
    : share()
    , mount( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteAssetShares::CRemoteAssetShares( CIAssetStoreService& service, const CRemoteFilesSettings& settings )
    : CORE::CObservingNotifier()
    , CIRemoteFileSource()
    , m_service( service )
    , m_settings( settings )
    , m_open( false )
    , m_mounted()
    , m_wholeFetched()
    , m_locationRoots()
    , m_transfers()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRemoteAssetShares::~CRemoteAssetShares()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetShares::FindCommonRoot( const CORE::CString::StringVector& roots )
{GUCEF_TRACE;

    if ( roots.empty() )
        return CORE::CString::Empty;
    CORE::CString::StringVector common = roots[ 0 ].ParseElements( '/', false );
    for ( size_t r=1; r<roots.size(); ++r )
    {
        const CORE::CString::StringVector parts = roots[ r ].ParseElements( '/', false );
        size_t same = 0;
        while ( same < common.size() && same < parts.size() && common[ same ] == parts[ same ] )
            ++same;
        common.resize( same );
    }
    CORE::CString root;
    for ( size_t i=0; i<common.size(); ++i )
        root += ( i > 0 ? CORE::CString( "/" ) : CORE::CString::Empty ) + common[ i ];
    return root;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::MakeShareLayout( const CAssetLocationPage& locations, const CORE::CString& shareName, const CORE::CString& mountPrefix, TShareVector& shares )
{GUCEF_TRACE;

    shares.clear();
    const CORE::CString prefix = CleanRoot( mountPrefix );
    CORE::CString::StringVector roots;
    CORE::CString::StringVector onlineIds;
    GUCEF::map< CORE::CString, CORE::CString > rootOfId;
    bool anyOnline = false;
    for ( size_t l=0; l<locations.entries.size(); ++l )
    {
        const CORE::CString root = CleanRoot( locations.entries[ l ].location.vfsRoot );
        if ( root.IsNULLOrEmpty() )
            continue;
        roots.push_back( root );
        rootOfId[ locations.entries[ l ].location.locationId ] = root;
        if ( locations.entries[ l ].online )
        {
            anyOnline = true;
            onlineIds.push_back( locations.entries[ l ].location.locationId );
        }
    }

    const CORE::CString common = FindCommonRoot( roots );
    if ( !common.IsNULLOrEmpty() )
    {
        if ( anyOnline && !shareName.IsNULLOrEmpty() )
        {
            SShare share;
            share.name = shareName;
            share.serviceRoot = common;
            share.mountPoint = JoinPath( prefix, common );
            shares.push_back( share );
        }
        return;
    }
    for ( size_t i=0; i<onlineIds.size(); ++i )
    {
        if ( !DRN::CDRNResourceName::IsValidProviderName( onlineIds[ i ] ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetShares: The location " << onlineIds[ i ] << " cannot be shared: its id is no share name" );
            continue;
        }
        SShare share;
        share.name = onlineIds[ i ];
        share.serviceRoot = rootOfId[ onlineIds[ i ] ];
        share.mountPoint = JoinPath( prefix, share.serviceRoot );
        shares.push_back( share );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::MakeBridgeSettings( const CRemoteFilesSettings& settings, const CORE::CString& cacheIdentity, CORE::CDataNode& bridgeSettings )
{GUCEF_TRACE;

    // the settings of the files of the previous client keep their names and meaning; the archive and its block cache take them
    bridgeSettings.Clear();
    bridgeSettings.SetName( "BridgeSettings" );
    bridgeSettings.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( !settings.cacheFolder.IsNULLOrEmpty() )
    {
        bridgeSettings.SetAttribute( "blockCacheFolder", settings.cacheFolder );
        bridgeSettings.SetAttribute( "blockCacheMaxBytes", settings.maxCacheBytes );
    }
    if ( !cacheIdentity.IsNULLOrEmpty() )
        bridgeSettings.SetAttribute( "cacheIdentity", cacheIdentity );
    bridgeSettings.SetAttribute( "readAheadAfterBlocks", settings.readAheadAfterBlocks );
    bridgeSettings.SetAttribute( "readAheadBlocks", settings.readAheadBlocks );
    bridgeSettings.SetAttribute( "maxBlocksPerFetch", settings.maxBlocksPerRequest );
    bridgeSettings.SetAttribute( "maxCachedEntries", settings.metadataEntries );
    bridgeSettings.SetAttribute( "metadataTtlMs", settings.metadataTtlMs );
    bridgeSettings.SetAttribute( "negativeTtlMs", settings.negativeTtlMs );
    bridgeSettings.SetAttribute( "listPageEntries", settings.listPageEntries );
    bridgeSettings.SetAttribute( "maxEntriesPerDirectory", settings.maxListEntries );
    bridgeSettings.SetAttribute( "callTimeoutMs", settings.blockingReadTimeoutMs );
    bridgeSettings.SetAttribute( "openTimeoutMs", settings.blockingReadTimeoutMs );
    bridgeSettings.SetAttribute( "refetchPinned", settings.refetchPinned );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetShares::CacheIdentityOf( const CORE::CString& share ) const
{GUCEF_TRACE;

    // a cache is shared by what it holds of one service: the identity of the service, not the address it was reached at
    DRNUTILS::CRemoteServiceConnection* connection = ConnectionOf( m_service );
    if ( GUCEF_NULL == connection )
        return CORE::CString::Empty;
    const DRNUTILS::CDeviceCredentials credentials = connection->GetCredentials();
    const CORE::CString service = credentials.serviceId.IsNULLOrEmpty() ? credentials.serviceName : credentials.serviceId;
    return service.IsNULLOrEmpty() ? CORE::CString::Empty : "assetstore:" + service + ":" + share;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::Open( const CORE::CString& mountPrefix )
{GUCEF_TRACE;

    if ( m_open )
        return true;
    DRNUTILS::CRemoteServiceConnection* connection = ConnectionOf( m_service );
    if ( GUCEF_NULL == connection )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetShares: The files of the service cannot be mounted: the service is not a remote one or it is not open" );
        return false;
    }
    const CORE::CString sessionName = connection->GetCredentials().drnSessionName;
    if ( sessionName.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetShares: The files of the service cannot be mounted: this device has no session of the service" );
        return false;
    }
    CAssetLocationPage locations;
    if ( ASSETSTORE_STATUS_OK != m_service.GetLocations( locations ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetShares: The files of the service cannot be mounted: the locations of the service are not known" );
        return false;
    }
    TShareVector layout;
    MakeShareLayout( locations, m_settings.shareName.IsNULLOrEmpty() ? DefaultShareName : m_settings.shareName, mountPrefix, layout );
    if ( layout.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetShares: The files of the service cannot be mounted: no storage location of the service is online" );
        return false;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t i=0; i<layout.size(); ++i )
    {
        DRNUTILS::CRemoteShareMountSettings mountSettings;
        mountSettings.share = layout[ i ].name;
        mountSettings.mountPoint = layout[ i ].mountPoint;
        MakeBridgeSettings( m_settings, CacheIdentityOf( layout[ i ].name ), mountSettings.bridgeSettings );
        DRNUTILS::CRemoteShareMount* mount = GUCEF_NEW DRNUTILS::CRemoteShareMount( *connection, mountSettings, &vfs );
        TEventCallback stateCallback( this, &CRemoteAssetShares::OnShareStateChanged );
        SubscribeTo( mount, DRNUTILS::CRemoteShareMount::StateChangedEvent, stateCallback );
        CORE::CString error;
        if ( !mount->Start( sessionName, error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CRemoteAssetShares: The share " << layout[ i ].name << " of the store service cannot be mounted at " << layout[ i ].mountPoint << ": " << error );
            UnsubscribeFrom( mount );
            GUCEF_DELETE mount;
            continue;
        }
        SMounted mounted;
        mounted.share = layout[ i ];
        mounted.mount = mount;
        MT::CScopeMutex lock( m_lock );
        m_mounted.push_back( mounted );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteAssetShares: Mounted the share " << layout[ i ].name << " (" << layout[ i ].serviceRoot << ") of the store service at " << layout[ i ].mountPoint );
    }
    if ( m_mounted.empty() )
        return false;
    {
        MT::CScopeMutex lock( m_lock );
        m_locationRoots.clear();
        for ( size_t l=0; l<locations.entries.size(); ++l )
        {
            const CORE::CString root = CleanRoot( locations.entries[ l ].location.vfsRoot );
            if ( !root.IsNULLOrEmpty() && locations.entries[ l ].online )
                for ( size_t i=0; i<m_mounted.size(); ++i )
                    if ( IsBelow( root, m_mounted[ i ].share.serviceRoot ) )
                    {
                        m_locationRoots.push_back( root );
                        break;
                    }
        }
    }

    TEventCallback changedCallback( this, &CRemoteAssetShares::OnCatalogueChanged );
    SubscribeTo( &m_service, CIAssetStoreService::CatalogueChangedEvent, changedCallback );
    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::Close( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    UnsubscribeFrom( &m_service );
    {
        // transfers that run are ended first: they read through the mounts
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
    TMountedVector mounted;
    {
        MT::CScopeMutex lock( m_lock );
        mounted = m_mounted;
        m_mounted.clear();
        m_locationRoots.clear();
    }
    for ( size_t i=mounted.size(); i>0; --i )
    {
        UnsubscribeFrom( mounted[ i-1 ].mount );
        mounted[ i-1 ].mount->Stop();
        GUCEF_DELETE mounted[ i-1 ].mount;
    }
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool CRemoteAssetShares::IsOpen( void ) const { return m_open; }
const CRemoteFilesSettings& CRemoteAssetShares::GetSettings( void ) const { return m_settings; }
CIAssetStoreService& CRemoteAssetShares::GetService( void ) const { return m_service; }

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::GetShares( TShareVector& shares ) const
{GUCEF_TRACE;

    shares.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounted.size(); ++i )
        shares.push_back( m_mounted[ i ].share );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteShareMount*
CRemoteAssetShares::GetMount( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return index < m_mounted.size() ? m_mounted[ index ].mount : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::Contains( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString servicePath;
    return ToServicePath( vfsPath, servicePath );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::GetLocationRoots( CORE::CString::StringVector& serviceRoots ) const
{GUCEF_TRACE;

    // the locations whose root is a mounted share or lies below one, as the service reported them when the shares were mounted
    MT::CScopeMutex lock( m_lock );
    serviceRoots = m_locationRoots;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::IsMounted( const CORE::CString& serviceRoot ) const
{GUCEF_TRACE;

    const CORE::CString root = CleanRoot( serviceRoot );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounted.size(); ++i )
        if ( IsBelow( root, m_mounted[ i ].share.serviceRoot ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::ToServicePath( const CORE::CString& clientPath, CORE::CString& servicePath ) const
{GUCEF_TRACE;

    const CORE::CString path = CleanRoot( clientPath );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounted.size(); ++i )
    {
        const CORE::CString& mount = m_mounted[ i ].share.mountPoint;
        if ( IsBelow( path, mount ) )
        {
            servicePath = m_mounted[ i ].share.serviceRoot + path.CutChars( mount.Length(), true );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetShares::ToClientPath( const CORE::CString& servicePath ) const
{GUCEF_TRACE;

    const CORE::CString path = CleanRoot( servicePath );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_mounted.size(); ++i )
    {
        const CORE::CString& root = m_mounted[ i ].share.serviceRoot;
        if ( IsBelow( path, root ) )
            return m_mounted[ i ].share.mountPoint + path.CutChars( root.Length(), true );
    }
    return path;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::GetFileInfo( const CORE::CString& servicePath, SFileInfo& info )
{GUCEF_TRACE;

    info = SFileInfo();
    const CORE::CString client = ToClientPath( servicePath );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.FileExists( client ) )
    {
        info.exists = true;
        CORE::CResourceMetaData metaData;
        if ( vfs.GetFileMetaData( client, metaData ) && metaData.hasResourceSizeInBytes )
            info.size = metaData.resourceSizeInBytes;
        else
            info.size = vfs.GetFileSize( client );
        VFS::CVfsDigest digest;
        if ( vfs.GetFileDigest( client, digest ) && VFS::CVfsDigest::ALGORITHM_SHA256 == digest.algorithm )
            info.digest = digest.value;
        return true;
    }
    if ( vfs.DirExists( client ) )
    {
        info.exists = true;
        info.isDirectory = true;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::GetFolderEntries( const CORE::CString& serviceFolder, CORE::CString::StringVector& dirs, CORE::CString::StringVector& files, bool* truncated )
{GUCEF_TRACE;

    dirs.clear();
    files.clear();
    if ( GUCEF_NULL != truncated )
        *truncated = false;
    const CORE::CString client = ToClientPath( serviceFolder );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( client ) )
        return false;
    VFS::CVFS::TStringVector dirNames;
    VFS::CVFS::TStringVector fileNames;
    vfs.GetDirList( dirNames, client, false, false, CORE::CString::Empty );
    vfs.GetFileList( fileNames, client, false, false, CORE::CString::Empty );
    for ( size_t i=0; i<dirNames.size(); ++i )
    {
        const CORE::CString name = LeafName( dirNames[ i ] );
        if ( !name.IsNULLOrEmpty() )
            dirs.push_back( name );
    }
    for ( size_t i=0; i<fileNames.size(); ++i )
    {
        const CORE::CString name = LeafName( fileNames[ i ] );
        if ( !name.IsNULLOrEmpty() )
            files.push_back( name );
    }
    if ( GUCEF_NULL != truncated )
        *truncated = m_settings.maxListEntries > 0 && dirs.size() + files.size() >= m_settings.maxListEntries;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::RequestFolder( const CORE::CString& serviceFolder )
{GUCEF_TRACE;

    // a call that never waits: the list is asked for in the background and the arrival is announced
    VFS::CVfsThreadPolicy policy;
    policy.missAction = VFS::MISSACTION_FAIL_AND_FETCH;
    VFS::CVfsThreadPolicyScope scope( policy );
    VFS::CVFS::TStringVector names;
    VFS::CVfsGlobal::Instance()->GetVfs().GetDirList( names, ToClientPath( serviceFolder ), false, false, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::IsFolderKnown( const CORE::CString& serviceFolder ) const
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive;
    CORE::CString remainder;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().ResolveMount( ToClientPath( serviceFolder ), archive, remainder ) || archive.IsNULL() )
        return false;
    VFS::CIRemoteArchive* remote = archive->GetRemoteArchiveInterface();
    return GUCEF_NULL != remote && VFS::CIRemoteArchive::METADATA_UNKNOWN != remote->GetMetadataState( remainder );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::NoteDigest( const CORE::CString& servicePath, const CORE::CString& digest )
{GUCEF_TRACE;

    const VFS::CVfsDigest hint( VFS::CVfsDigest::ALGORITHM_SHA256, digest );
    if ( !hint.IsWellFormed() )
        return;
    VFS::CVfsGlobal::Instance()->GetVfs().NoteFileDigest( ToClientPath( servicePath ), hint );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::NoteFiles( const GUCEF::vector< CAssetFileRecord >& files )
{GUCEF_TRACE;

    for ( size_t i=0; i<files.size(); ++i )
        if ( files[ i ].sizeBytes >= 0 && "directory" != files[ i ].role && !files[ i ].digest.IsNULLOrEmpty() )
            NoteDigest( files[ i ].path, files[ i ].digest );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::GetKnownDigest( const CORE::CString& servicePath, CORE::CString& digest ) const
{GUCEF_TRACE;

    // from memory only: a thread that may wait does not ask the service either
    VFS::CVfsThreadPolicy policy;
    policy.missAction = VFS::MISSACTION_FAIL;
    VFS::CVfsThreadPolicyScope scope( policy );
    VFS::CVfsDigest known;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().GetFileDigest( ToClientPath( servicePath ), known ) || VFS::CVfsDigest::ALGORITHM_SHA256 != known.algorithm || known.IsEmpty() )
        return false;
    digest = known.value;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::ForgetMetadata( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.InvalidateRemoteMetadata();
    TShareVector shares;
    GetShares( shares );
    for ( size_t i=0; i<shares.size(); ++i )
        vfs.ForgetFileDigests( shares[ i ].mountPoint );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a pack came or went, or the session was made again: what is known of the folders may be old
    ForgetMetadata();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::OnShareStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( StateChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::IsOffline( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_mounted.empty() )
        return false;
    for ( size_t i=0; i<m_mounted.size(); ++i )
        if ( DRNUTILS::REMOTESHARE_ONLINE == m_mounted[ i ].mount->GetState() )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::IsFileCached( const CORE::CString& servicePath, UInt64 size ) const
{GUCEF_TRACE;

    VFS::CVfsCacheState state;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().GetFileCacheState( ToClientPath( servicePath ), state ) )
        return false;
    return state.complete && !state.staleRevision && ( 0 == size || state.fileSize == size );
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetShares::UnpinFile( const CORE::CString& servicePath )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance()->GetVfs().SetFilePinned( ToClientPath( servicePath ), false );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetShares::StatusOfResult( CORE::TFileSystemAccessResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case CORE::FILESYSTEMACCESS_SUCCESS:
        case CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS:
            return ASSETSTORE_STATUS_OK;
        case CORE::FILESYSTEMACCESS_NOT_FOUND:
        case CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND:
            return ASSETSTORE_STATUS_NOT_FOUND;
        case CORE::FILESYSTEMACCESS_ACCESS_DENIED:
            return ASSETSTORE_STATUS_REFUSED;
        case CORE::FILESYSTEMACCESS_NO_SPACE:
            return ASSETSTORE_STATUS_NO_SPACE;
        case CORE::FILESYSTEMACCESS_CANCELLED:
            return ASSETSTORE_STATUS_CANCELLED;
        case CORE::FILESYSTEMACCESS_TIMEOUT:
            return ASSETSTORE_STATUS_TIMED_OUT;
        case CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING:
        case CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER:
        case CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE:
            return ASSETSTORE_STATUS_UNAVAILABLE;
        case CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT:
            return ASSETSTORE_STATUS_DIGEST_MISMATCH;
        case CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR:
            return ASSETSTORE_STATUS_NETWORK_ERROR;
        case CORE::FILESYSTEMACCESS_NOT_SUPPORTED:
            return ASSETSTORE_STATUS_UNSUPPORTED;
        default:
            return ASSETSTORE_STATUS_FAILED;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetShares::DescribeFailure( const CORE::CString& servicePath, TAssetStoreStatus status, const CORE::CString& vfsMessage )
{GUCEF_TRACE;

    switch ( status )
    {
        case ASSETSTORE_STATUS_NOT_FOUND:
            return "The file " + servicePath + " is not on the store service (any more)";
        case ASSETSTORE_STATUS_REFUSED:
            return "The store service does not give the file " + servicePath + " to this device" + ( vfsMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + vfsMessage );
        case ASSETSTORE_STATUS_NO_SPACE:
            return "The file " + servicePath + " cannot be kept in the cache of this computer" + ( vfsMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + vfsMessage );
        case ASSETSTORE_STATUS_UNAVAILABLE:
        case ASSETSTORE_STATUS_NETWORK_ERROR:
        case ASSETSTORE_STATUS_TIMED_OUT:
            return "The file " + servicePath + " is not available offline: it is not in the cache of this computer and the store service cannot be reached" +
                   ( vfsMessage.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + vfsMessage + ")" );
        case ASSETSTORE_STATUS_DIGEST_MISMATCH:
            return "The file " + servicePath + " on the store service differs from what its catalogue says" + ( vfsMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + vfsMessage );
        case ASSETSTORE_STATUS_CANCELLED:
            return "The read of the file " + servicePath + " was cancelled";
        default:
            return "The file " + servicePath + " cannot be read from the store service" + ( vfsMessage.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + vfsMessage );
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetShares::FetchWholeFile( const CORE::CString& servicePath, bool pin, const MT::CTAtomic< bool >* cancel, MT::CTAtomic< UInt64 >* done, CORE::CString& message )
{GUCEF_TRACE;

    message.Clear();
    if ( GUCEF_NULL != done )
        done->Store( 0 );
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_wholeFetched.size() >= 4096 )
            m_wholeFetched.clear();
        m_wholeFetched.insert( CleanRoot( servicePath ) );
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVfsFetchRequest request;
    request.pin = pin;
    request.verifyDigest = true;
    VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
    VFS::CVfsAsyncResultPtr result;
    CORE::CFutureResult future = vfs.FetchFileAsync( ToClientPath( servicePath ), request, progress, result );
    if ( future.HasNoFuture() || result.IsNULL() )
    {
        message = "The file " + servicePath + " cannot be fetched: the fetch could not be queued";
        return ASSETSTORE_STATUS_FAILED;
    }
    WaitForCall( future, cancel, progress.GetPointerAlways(), done );
    const VFS::CVfsCallResult outcome = result->result;
    if ( outcome.IsSuccess() )
        return ASSETSTORE_STATUS_OK;
    const TAssetStoreStatus status = StatusOfResult( outcome.result );
    if ( CORE::FILESYSTEMACCESS_NO_SPACE == outcome.result )
        message = "The file " + servicePath + " is in the cache of this computer, but it cannot be kept offline: " + outcome.message;
    else
        message = DescribeFailure( servicePath, status, outcome.message );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetShares::ReadFileRange( const CORE::CString& servicePath, UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data, CORE::CString& message )
{GUCEF_TRACE;

    message.Clear();
    data.Clear( false );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVfsAsyncResultPtr result;
    CORE::CFutureResult future = vfs.ReadFileRangeAsync( ToClientPath( servicePath ), offset, length, result );
    if ( future.HasNoFuture() || result.IsNULL() )
    {
        message = "The file " + servicePath + " cannot be read: the read could not be queued";
        return ASSETSTORE_STATUS_FAILED;
    }
    WaitForCall( future, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL );
    if ( result->data.GetDataSize() > 0 )
        data.CopyFrom( result->data.GetDataSize(), result->data.GetConstBufferPtr() );
    const VFS::CVfsCallResult outcome = result->result;
    if ( outcome.IsSuccess() )
        return ASSETSTORE_STATUS_OK;
    const TAssetStoreStatus status = StatusOfResult( outcome.result );
    message = DescribeFailure( servicePath, status, outcome.message );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CRemoteAssetShares::StartTransfer( const TRemoteFileTransferPtr& transfer, const CORE::CString& threadPoolName )
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
CRemoteAssetShares::TransferEnded( const TRemoteFileTransferPtr& transfer )
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
CRemoteAssetShares::GetRunningTransferCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_transfers.size();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::TakeWholeFetched( const CORE::CString& servicePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return 0 != m_wholeFetched.erase( CleanRoot( servicePath ) );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetShares::GetStatistics( VFS::CVfsRemoteStatistics& statistics ) const
{GUCEF_TRACE;

    TShareVector shares;
    GetShares( shares );
    if ( shares.empty() )
        return false;
    return VFS::CVfsGlobal::Instance()->GetVfs().GetRemoteStatistics( JoinPath( shares[ 0 ].mountPoint, "statistics" ), statistics );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
