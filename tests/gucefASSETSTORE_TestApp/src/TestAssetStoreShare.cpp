/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#include "gucefDRNUTILS_CRemoteShareMount.h"
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#include "vfspluginDRN_CDrnBridgeArchive.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#include "gucefASSETSTORE_CAssetShareDecision.h"
#define GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#endif /* GUCEF_ASSETSTORE_CASSETSHAREDECISION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#include "gucefASSETSTORE_CAssetThumbnail.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestAssetStoreShare.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 BigSize = 5 * 262144 + 1000;            // more than one block of the share, the last one short

/*-------------------------------------------------------------------------*/

/**
 *  The service of the server side whose answers about the restrictions of a path are slow when they have to be made (the catalogue is far away):
 *  every such answer is counted, with the thread it was asked on
 */
class CSlowRestrictionsService : public CLocalAssetStoreService
{
    public:

    mutable MT::CTAtomic< UInt32 > delayMs;
    mutable MT::CTAtomic< UInt32 > slowCalls;
    mutable MT::CTAtomic< UInt32 > slowCallsOnNodeThread;
    mutable MT::CTAtomic< UInt32 > digestCalls;
    DRN::CDRNNode* volatile node;

    explicit CSlowRestrictionsService( const CAssetStoreServiceConfig& config )
        : CLocalAssetStoreService( config ), delayMs( 0 ), slowCalls( 0 ), slowCallsOnNodeThread( 0 ), digestCalls( 0 ), node( GUCEF_NULL )
    {
    }

    virtual bool GetRestrictionsOfPath( const CString& vfsPath, CString::StringSet& flags ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CString::StringSet kept;
        bool inPack = false;
        if ( !TryGetRestrictionsOfPathKept( vfsPath, kept, inPack ) )
        {
            // the answer is not kept: this is the query of the catalogue
            slowCalls.FetchAdd( 1 );
            DRN::CDRNNode* current = node;
            if ( GUCEF_NULL != current && current->IsWorkingOnThisThread() )
                slowCallsOnNodeThread.FetchAdd( 1 );
            const UInt32 delay = delayMs.Load();
            if ( delay > 0 )
                MT::PrecisionDelay( delay );
        }
        return CLocalAssetStoreService::GetRestrictionsOfPath( vfsPath, flags );
    }

    virtual bool GetCatalogueDigestOfPath( const CString& vfsPath, UInt64& sizeBytes, CString& sha256Hex ) const GUCEF_VIRTUAL_OVERRIDE
    {
        digestCalls.FetchAdd( 1 );
        return CLocalAssetStoreService::GetCatalogueDigestOfPath( vfsPath, sizeBytes, sha256Hex );
    }
};

/*-------------------------------------------------------------------------*/

CString
RelativeTo( const CString& root, const CString& path )
{
    return 0 == path.HasSubstr( root + "/", true ) ? path.CutChars( root.Length() + 1, true ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
SameBytes( const CDynamicBuffer& a, const CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteServiceConnection*
ConnectionOfClient( CIAssetStoreService& service )
{
    CRemoteAssetStoreService* remote = CAssetStoreServiceConfig::RemoteImplementationId == service.GetType() ? static_cast< CRemoteAssetStoreService* >( &service ) : GUCEF_NULL;
    return GUCEF_NULL == remote ? GUCEF_NULL : remote->GetConnection();
}

/*-------------------------------------------------------------------------*/

/** What the VFS plugin for DRN made of a call, asked on a thread of its own while the test thread pumps (the service is on the pulse generator of the test) */
class CArchiveProbe : public MT::CActiveObject
{
    public:

    enum EOperation { OPERATION_READ = 0, OPERATION_SIZE, OPERATION_LIST, OPERATION_EXISTS, OPERATION_HASH };

    VFS::CVFS::TArchivePtr archive;
    EOperation operation;
    CString path;
    bool ok;
    UInt64 size;
    CDynamicBuffer content;
    VFS::CArchive::TStringVector names;
    CString hash;
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
    MT::CTAtomic< UInt32 > finished;
    UInt64 elapsedMs;

    CArchiveProbe( const VFS::CVFS::TArchivePtr& a, EOperation op, const CString& p )
        : MT::CActiveObject(), archive( a ), operation( op ), path( p ), ok( false ), size( 0 ), content(), names(), hash(), result(), finished( 0 ), elapsedMs( 0 ) {}

    virtual ~CArchiveProbe() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    /** Runs the call and pumps until it is done */
    bool Run( UInt32 timeoutMs = 60000 )
    {
        Activate( GUCEF_NULL );
        for ( UInt32 waited=0; waited<timeoutMs && 0 == finished.Load(); waited+=2 )
            PumpFor( 2 );
        return 0 != finished.Load();
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 start = MT::PrecisionTickCount();
        try
        {
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() );
            if ( OPERATION_EXISTS == operation )
                ok = archive->FileExists( path );
            else
            if ( OPERATION_SIZE == operation )
            {
                size = archive->GetFileSize( path );
                ok = true;
            }
            else
            if ( OPERATION_HASH == operation )
            {
                hash = archive->GetFileHash( path );
                ok = !hash.IsNULLOrEmpty();
            }
            else
            if ( OPERATION_LIST == operation )
            {
                archive->GetFileList( names, "", path, false, false );
                VFS::CArchive::TStringVector dirs;
                archive->GetDirList( dirs, "", path, false, false );
                for ( UInt32 i=0; i<dirs.size(); ++i )
                    names.push_back( dirs[ i ] );
                ok = true;
            }
            else
            {
                VFS::TBasicVfsResourcePtr file = archive->GetFile( path, "rb", 0, false );
                if ( !file.IsNULL() && !file->GetAccess().IsNULL() )
                {
                    CDynamicBuffer piece( 65536, true );
                    ok = true;
                    while ( true )
                    {
                        const UInt32 read = file->GetAccess()->Read( piece.GetBufferPtr(), 1, 65536 );
                        if ( 0 == read )
                            break;
                        content.Append( piece.GetConstBufferPtr(), read );
                    }
                }
            }
            result = bridge->GetLastResult();
        }
        catch ( ... )
        {
        }
        elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        finished.Store( 1 );
        return true;
    }
};

bool
HasName( const VFS::CArchive::TStringVector& names, const CString& name )
{
    for ( UInt32 i=0; i<names.size(); ++i )
        if ( names[ i ] == name || names[ i ] == name + "/" )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** A server side with the files of a primary store, its host with the share, and the owner device */
class CShareFixture
{
    public:

    CString name;
    CString base;
    CString mainRoot;
    CRemoteAssetStoreWorld world;
    CSlowRestrictionsService* slow;
    TAssetStoreServicePtr owner;
    CDynamicBuffer big;
    bool started;

    CString plainPack, plainAsset, plainPath;
    CString restrictedPack, restrictedAsset, restrictedPath;
    CString learningPack, learningAsset, learningPath;

    /** locations: 0 one location "main", 1 two locations below one folder, 2 two locations whose roots have nothing in common */
    explicit CShareFixture( const CString& fixtureName, UInt32 locations = 0, const CString& shareName = "assets" )
        : name( fixtureName ), base( "astest/sh_" + fixtureName ), mainRoot(), world( "sh_" + fixtureName ), slow( GUCEF_NULL ), owner(), big(), started( false )
    {
        world.bindingSettings.roles.insert( "viewer" );
        world.bindingSettings.roles.insert( "mcp" );
        world.bindingSettings.shareName = shareName;
        mainRoot = 1 == locations ? base + "/a/main" : base + "/main";
        CString secondRoot = 1 == locations ? base + "/a/extra" : "astest_other_" + fixtureName + "/extra";
        MountFreshDirectory( "sh_" + name + "_main", mainRoot );
        MountFreshDirectory( "sh_" + name + "_source", base + "/source" );
        MountFreshDirectory( "sh_" + name + "_catalogue", base + "/catalogue" );
        if ( 0 != locations )
            MountFreshDirectory( "sh_" + name + "_extra", secondRoot );

        CAssetStoreServiceConfig config;
        config.primaryStore.locations.clear();
        CAssetStorageLocation main;
        main.locationId = "main";
        main.name = "Main";
        main.vfsRoot = mainRoot;
        config.primaryStore.locations.push_back( main );
        if ( 0 != locations )
        {
            CAssetStorageLocation extra;
            extra.locationId = "extra";
            extra.name = "Extra";
            extra.vfsRoot = secondRoot;
            config.primaryStore.locations.push_back( extra );
        }
        config.primaryStore.defaultLocationId = "main";
        config.primaryStore.database.backendName = "vfstables";
        config.primaryStore.database.database = base + "/catalogue/db";
        config.primaryStore.database.backendOptions.SetAllowMultipleValues( false );
        config.primaryStore.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
        config.loadDownloadQueue = false;
        config.startDownloads = false;
        config.implementationId = CAssetStoreServiceConfig::LocalImplementationId;
        world.serverConfig = config;
        slow = GUCEF_NEW CSlowRestrictionsService( config );
        world.serverService = TAssetStoreServicePtr( CORE::CTSharedPtr< CIAssetStoreService, MT::CMutex >( slow ) );
        CString error;
        if ( ASSETSTORE_STATUS_OK != world.serverService->Open() )
            return;
        if ( 0 == locations )
        {
            MakePattern( BigSize, 7, big );
            WriteVfsFile( mainRoot + "/bigpack/original/big.bin", big );
            WriteVfsFile( mainRoot + "/staging/job1/staged.txt", CString( "work in progress of an ingest" ) );
            WriteVfsFile( mainRoot + "/notes/readme.txt", CString( "a file of the location that is in no pack" ) );
            WriteVfsFile( mainRoot + "/intermediate/ZetaPack/converted.dds", CString( "an intermediate version of a pack" ) );
            WriteVfsFile( mainRoot + "/generations/ZetaPack/previous/old.dds", CString( "the previous generation of a pack" ) );
        }
        if ( !world.StartHost( error ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ShareFixture: the host did not start: " << error );
            return;
        }
        slow->node = world.host->GetNode();
        owner = world.MakeClient( "owner", error );
        if ( owner.IsNULL() || ASSETSTORE_STATUS_OK != owner->Open() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ShareFixture: no owner client: " << error );
            return;
        }
        started = true;
    }

    ~CShareFixture()
    {
        owner.Unlink();
        world.StopServer();
        UnmountDirectory( "sh_" + name + "_main" );
        UnmountDirectory( "sh_" + name + "_source" );
        UnmountDirectory( "sh_" + name + "_catalogue" );
        UnmountDirectory( "sh_" + name + "_extra" );
    }

    /** A pack of one texture with its licence restrictions, ingested through the owner client */
    bool IngestPack( const CString& packName, const CString& restrictions, UInt32 variant, CString& packId, CString& assetId, CString& mainPath )
    {
        const CString root = base + "/source/" + packName;
        CDynamicBuffer png;
        const UInt32 nonce = (UInt32) ( MT::PrecisionTickCount() % 3721 );
        MakePng( 16 + variant * 8 + nonce % 61, 16 + variant * 8 + nonce / 61, false, png );
        WriteVfsFile( root + "/pack.json", "{ \"title\": \"" + packName + "\", \"author\": \"Test Works\", \"licence\": { \"name\": \"Test\", \"restrictions\": [ " + restrictions + " ] } }" );
        WriteVfsFile( root + "/Textures/T_" + packName + "_D.png", png );
        CAssetIngestRequest request;
        request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
        request.contentPath = root;
        request.keep = "original";
        CAssetIngestResult result;
        if ( ASSETSTORE_STATUS_OK != owner->Ingest( "sh-ingest-" + packName, request, result ) || result.packId.IsNULLOrEmpty() )
            return false;
        packId = result.packId;
        CAssetRecordPage assets;
        if ( ASSETSTORE_STATUS_OK != owner->GetAssets( packId, CString::Empty, 0, assets ) || assets.entries.empty() )
            return false;
        assetId = assets.entries[ 0 ].assetId;
        CAssetFileSet files;
        if ( ASSETSTORE_STATUS_OK != owner->GetAssetFileSet( assetId, files ) )
            return false;
        mainPath = files.mainVfsPath;
        return !mainPath.IsNULLOrEmpty();
    }

    bool IngestPacks( void )
    {
        return IngestPack( "Fields", "\"attributionRequired\"", 3, plainPack, plainAsset, plainPath ) &&
               IngestPack( "Vault", "\"noRedistribution\"", 2, restrictedPack, restrictedAsset, restrictedPath ) &&
               IngestPack( "Corpus", "\"noMachineLearning\"", 4, learningPack, learningAsset, learningPath );
    }

    CString Rel( const CString& servicePath ) const { return RelativeTo( mainRoot, servicePath ); }
};

/*-------------------------------------------------------------------------*/

/** One device with its client of the service and its mount of the share; closed by the destructor */
class CShareSession
{
    public:

    CShareFixture& fixture;
    CString slot;
    TAssetStoreServicePtr client;
    DRNUTILS::CRemoteShareMount* mount;
    VFS::CVFS::TArchivePtr archive;
    DRNUTILS::CDeviceCredentials credentials;
    CString mountPoint;
    CString lastError;
    CDataNode bridgeSettings;                   /**< the archive specific settings of the mount, set before Open() */

    explicit CShareSession( CShareFixture& f ) : fixture( f ), slot(), client(), mount( GUCEF_NULL ), archive(), credentials(), mountPoint(), lastError(), bridgeSettings() {}

    ~CShareSession()
    {
        Close();
    }

    bool PumpUntilOnline( UInt32 timeoutMs )
    {
        for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
        {
            if ( GUCEF_NULL != mount && DRNUTILS::REMOTESHARE_ONLINE == mount->GetState() )
                return true;
            PumpFor( 5 );
        }
        return GUCEF_NULL != mount && DRNUTILS::REMOTESHARE_ONLINE == mount->GetState();
    }

    bool PumpUntilState( DRNUTILS::TRemoteShareState state, UInt32 timeoutMs )
    {
        for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
        {
            if ( GUCEF_NULL != mount && state == mount->GetState() )
                return true;
            PumpFor( 5 );
        }
        return GUCEF_NULL != mount && state == mount->GetState();
    }

    /** The device of the slot (paired when it was not) with the roles, its client opened and the share mounted and online */
    bool Open( const CString& slotName, const CString::StringSet* extraRoles, bool grantOwner, const CString& share = "assets", const CString& mountSuffix = CString::Empty )
    {
        slot = slotName;
        client = fixture.world.MakeClient( slot, lastError, extraRoles, grantOwner );
        if ( client.IsNULL() || ASSETSTORE_STATUS_OK != client->Open() )
            return false;
        DRNUTILS::CRemoteServiceConnection* connection = ConnectionOfClient( *client );
        if ( GUCEF_NULL == connection || !credentials.LoadFromStore( fixture.world.clientStore, slot, lastError ) )
            return false;
        DRNUTILS::CRemoteShareMountSettings settings;
        settings.share = share;
        settings.channel = 0;
        settings.bridgeSettings = bridgeSettings;
        mountPoint = "astest/shm_" + fixture.name + "_" + slot + mountSuffix;
        settings.mountPoint = mountPoint;
        mount = GUCEF_NEW DRNUTILS::CRemoteShareMount( *connection, settings );
        if ( !mount->Start( credentials.drnSessionName, lastError ) )
            return false;
        if ( !PumpUntilOnline( 30000 ) )
        {
            lastError = mount->GetLastError();
            return false;
        }
        archive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( mount->GetArchiveName() );
        return !archive.IsNULL();
    }

    void Close( void )
    {
        archive.Unlink();
        if ( GUCEF_NULL != mount )
        {
            mount->Stop();
            GUCEF_DELETE mount;
            mount = GUCEF_NULL;
        }
        if ( !client.IsNULL() )
        {
            client->Close();
            client.Unlink();
        }
    }

    bool Read( const CString& name, CDynamicBuffer& content, VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult* result = GUCEF_NULL, UInt64* elapsedMs = GUCEF_NULL )
    {
        CArchiveProbe probe( archive, CArchiveProbe::OPERATION_READ, name );
        probe.Run();
        content = probe.content;
        if ( GUCEF_NULL != result )
            *result = probe.result;
        if ( GUCEF_NULL != elapsedMs )
            *elapsedMs = probe.elapsedMs;
        return probe.ok;
    }

    /** A read after a STAT of the name, as a client that lists and stats before it reads: the restrictions of the pack are kept by then */
    bool ReadAfterStat( const CString& name, CDynamicBuffer& content, VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult* result = GUCEF_NULL )
    {
        UInt64 size = 0;
        Size( name, size );
        return Read( name, content, result );
    }

    bool Exists( const CString& name )
    {
        CArchiveProbe probe( archive, CArchiveProbe::OPERATION_EXISTS, name );
        probe.Run();
        return probe.ok;
    }

    bool Size( const CString& name, UInt64& size )
    {
        CArchiveProbe probe( archive, CArchiveProbe::OPERATION_SIZE, name );
        probe.Run();
        size = probe.size;
        return probe.ok && probe.result.IsOk();
    }

    CString Hash( const CString& name )
    {
        CArchiveProbe probe( archive, CArchiveProbe::OPERATION_HASH, name );
        probe.Run();
        return probe.hash;
    }

    bool List( const CString& dir, VFS::CArchive::TStringVector& names )
    {
        CArchiveProbe probe( archive, CArchiveProbe::OPERATION_LIST, dir );
        probe.Run();
        names = probe.names;
        return probe.ok;
    }
};

/*-------------------------------------------------------------------------*/

CString::StringSet
Roles( const char* a, const char* b = GUCEF_NULL )
{
    CString::StringSet roles;
    roles.insert( a );
    if ( GUCEF_NULL != b )
        roles.insert( b );
    return roles;
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStoreShareTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreShare" );

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();

    GUCEF_TESTFW_TESTCASE( "Share: the root of the primary store is shared under the name of the settings by the host; no name, no share; locations below one folder share that folder, locations with nothing in common one share each under the id" )
        try
        {
            {
                CShareFixture f( "names" );
                ASSERT_TRUE( f.started );
                CString::StringVector names;
                f.world.binding->GetShareNames( names );
                ASSERT_TRUE( 1 == names.size() && "assets" == names[ 0 ] );
                ASSERT_TRUE( f.world.binding->GetShareRoot( "assets" ) == f.mainRoot );
                ASSERT_TRUE( f.world.host->IsShareServed( "assets" ) );
                ASSERT_TRUE( NULL != f.world.binding->GetShareDecision() );
            }
            {
                CShareFixture f( "noname", 0, CString::Empty );
                ASSERT_TRUE( f.started );
                CString::StringVector names;
                f.world.binding->GetShareNames( names );
                ASSERT_TRUE( names.empty() && !f.world.host->HasShare( "assets" ) );
            }
            {
                CShareFixture f( "common", 1 );
                ASSERT_TRUE( f.started );
                PumpFor( 100 );
                CString::StringVector names;
                f.world.binding->GetShareNames( names );
                ASSERT_TRUE( 1 == names.size() && "assets" == names[ 0 ] );
                ASSERT_TRUE( f.world.binding->GetShareRoot( "assets" ) == f.base + "/a" );
            }
            {
                CShareFixture f( "apart", 2 );
                ASSERT_TRUE( f.started );
                PumpFor( 100 );
                CString::StringVector names;
                f.world.binding->GetShareNames( names );
                ASSERT_TRUE( 2 == names.size() );
                ASSERT_TRUE( f.world.binding->GetShareRoot( "main" ) == f.mainRoot );
                ASSERT_TRUE( f.world.binding->GetShareRoot( "extra" ) == "astest_other_apart/extra" );
                ASSERT_TRUE( f.world.host->IsShareServed( "main" ) && f.world.host->IsShareServed( "extra" ) && !f.world.host->HasShare( "assets" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // one world for the cases of the devices
    CShareFixture f( "main" );
    bool ready = f.started && f.IngestPacks();
    const CString::StringSet viewerRoles = Roles( "viewer" );
    const CString::StringSet mcpRoles = Roles( "mcp" );
    const CString::StringSet learnerRoles = Roles( "mlexport" );

    GUCEF_TESTFW_TESTCASE( "Share, owner: lists, stats and reads every file of the root (the packs, the intermediate area, staging, files outside packs, a file of several blocks) equal to what the service has; the digest is the catalogue's" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );

            VFS::CArchive::TStringVector names;
            ASSERT_TRUE( s.List( "", names ) );
            ASSERT_TRUE( HasName( names, "bigpack" ) && HasName( names, "notes" ) && HasName( names, "staging" ) && HasName( names, "original" ) && HasName( names, "intermediate" ) && HasName( names, "generations" ) );
            ASSERT_TRUE( s.List( "staging/job1", names ) && HasName( names, "staged.txt" ) );

            // the bytes of the file of a pack, the staging file and a file outside the packs
            const CString paths[ 5 ] = { f.Rel( f.plainPath ), "staging/job1/staged.txt", "notes/readme.txt", "intermediate/ZetaPack/converted.dds", "generations/ZetaPack/previous/old.dds" };
            for ( UInt32 i=0; i<5; ++i )
            {
                CDynamicBuffer fromShare, fromService;
                ASSERT_TRUE( s.ReadAfterStat( paths[ i ], fromShare ) );
                ASSERT_TRUE( ReadVfsFile( f.mainRoot + "/" + paths[ i ], fromService ) );
                ASSERT_TRUE( fromShare.GetDataSize() > 0 && SameBytes( fromShare, fromService ) );
                UInt64 size = 0;
                ASSERT_TRUE( s.Size( paths[ i ], size ) && size == fromService.GetDataSize() );
            }
            CDynamicBuffer bigFromShare;
            ASSERT_TRUE( s.ReadAfterStat( "bigpack/original/big.bin", bigFromShare ) && SameBytes( bigFromShare, f.big ) );

            // the digest: the catalogue's SHA-256 of the file of the pack; none for a file that the catalogue does not hold
            CAssetRecord asset;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->GetAsset( f.plainAsset, asset ) );
            CString catalogueDigest;
            for ( UInt32 i=0; i<asset.files.size(); ++i )
                if ( "main" == asset.files[ i ].role )
                    catalogueDigest = asset.files[ i ].digest;
            ASSERT_TRUE( 64 == catalogueDigest.Length() );
            CDynamicBuffer plainBytes;
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.plainPath ), plainBytes ) );
            ASSERT_TRUE( CAssetDigest::AreEqual( catalogueDigest, Sha256Of( plainBytes ) ) );
            const UInt32 digestCallsBefore = f.slow->digestCalls.Load();
            ASSERT_TRUE( CAssetDigest::AreEqual( catalogueDigest, s.Hash( f.Rel( f.plainPath ) ) ) );
            ASSERT_TRUE( f.slow->digestCalls.Load() >= digestCallsBefore );
            ASSERT_TRUE( s.Hash( "staging/job1/staged.txt" ).IsNULLOrEmpty() );
            ASSERT_TRUE( s.Hash( "bigpack/original/big.bin" ).IsNULLOrEmpty() );

            // a file of the pack in the catalogue: the digest that is known is the one of a file of that size only
            ASSERT_TRUE( f.slow->digestCalls.Load() > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, machine role (mcp): lists and stats everything, is refused the bytes with the words of the gate" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "mcp", &mcpRoles, false ) );
            VFS::CArchive::TStringVector names;
            ASSERT_TRUE( s.List( "", names ) && HasName( names, "original" ) && HasName( names, "staging" ) );
            UInt64 size = 0;
            ASSERT_TRUE( s.Size( f.Rel( f.plainPath ), size ) && size > 0 );
            ASSERT_TRUE( s.Size( f.Rel( f.restrictedPath ), size ) && size > 0 );
            ASSERT_TRUE( s.Exists( "notes/readme.txt" ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_FALSE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) );
            ASSERT_TRUE( content.GetDataSize() == 0 );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "withheld from the machine role" ) >= 0 );
            ASSERT_FALSE( s.ReadAfterStat( "notes/readme.txt", content, &result ) );
            ASSERT_FALSE( s.ReadAfterStat( "staging/job1/staged.txt", content, &result ) );
            ASSERT_TRUE( s.Hash( f.Rel( f.plainPath ) ).Length() > 0 );             // digests are metadata

            // the metadata of every pack stays visible to the machine role, the pictures and the bytes do not
            CAssetFileSet machineFiles;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->GetAssetFileSet( f.plainAsset, machineFiles ) && ASSETSTORE_STATUS_OK == s.client->GetAssetFileSet( f.restrictedAsset, machineFiles ) && !machineFiles.mainVfsPath.IsNULLOrEmpty() );
            CAssetPackRecord machinePack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->GetPack( f.restrictedPack, machinePack ) && ASSETSTORE_STATUS_OK == s.client->GetPack( f.learningPack, machinePack ) );
            CString::StringVector machineIds;
            machineIds.push_back( f.plainAsset );
            CAssetThumbnailPage machineThumbnails;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == s.client->GetThumbnails( machineIds, 64, machineThumbnails ) );
            CFutureResult machineAsync = s.client->GetThumbnailsAsync( machineIds, 64 );
            ASSERT_TRUE( WaitForFuture( machineAsync, 10000 ) && ASSETSTORE_STATUS_REFUSED == CIAssetStoreService::GetStatusOfFuture( machineAsync ) );
            ASSERT_TRUE( CIAssetStoreService::GetMessageOfFuture( machineAsync ).HasSubstr( "withheld from the machine role" ) >= 0 );
            CAssetSearchQuery machineQuery;
            CAssetSearchResultPage machineResults;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->Search( machineQuery, machineResults ) && machineResults.hits.size() >= 3 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, viewer: reads the packs it may have; the files of a noRedistribution pack are refused in the words of the gate, still listed and sized; a pack with noMachineLearning is no problem for a viewer" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) && content.GetDataSize() > 0 );
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.learningPath ), content, &result ) && content.GetDataSize() > 0 );
            ASSERT_TRUE( s.ReadAfterStat( "notes/readme.txt", content, &result ) );
            ASSERT_FALSE( s.ReadAfterStat( f.Rel( f.restrictedPath ), content, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "does not allow its files to leave the devices of the owner" ) >= 0 );
            UInt64 size = 0;
            ASSERT_TRUE( s.Size( f.Rel( f.restrictedPath ), size ) && size > 0 );
            VFS::CArchive::TStringVector names;
            ASSERT_TRUE( s.List( "", names ) && HasName( names, "original" ) );
            // the service knows the flags of the pack a path is in, and that a picture of the thumbnail folder is in no pack
            CString::StringSet flags;
            ASSERT_TRUE( f.slow->GetRestrictionsOfPath( f.restrictedPath, flags ) && 1 == flags.count( "noRedistribution" ) );
            ASSERT_TRUE( f.slow->GetRestrictionsOfPath( f.plainPath, flags ) && 0 == flags.count( "noRedistribution" ) && 1 == flags.count( "attributionRequired" ) );
            ASSERT_TRUE( !f.slow->GetRestrictionsOfPath( f.mainRoot + "/thumbnails/ab/x.png", flags ) );
            // the metadata of the pack stays visible
            CAssetFileSet viewerFiles;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->GetAssetFileSet( f.restrictedAsset, viewerFiles ) && !viewerFiles.mainVfsPath.IsNULLOrEmpty() );
            // the owner reads the same file
            CShareSession o( f );
            s.Close();
            ASSERT_TRUE( o.Open( "owner", GUCEF_NULL, true ) );
            ASSERT_TRUE( o.ReadAfterStat( f.Rel( f.restrictedPath ), content, &result ) && content.GetDataSize() > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, learning role: the files of a noMachineLearning pack are refused as if they were not there, the folder and the names stay listed; the other packs read" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "learner", &learnerRoles, true ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) && content.GetDataSize() > 0 );
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.restrictedPath ), content, &result ) );             // owner role, no restriction on redistribution for the owner
            ASSERT_FALSE( s.ReadAfterStat( f.Rel( f.learningPath ), content, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "There is no such file" ) >= 0 );
            UInt64 size = 0;
            ASSERT_TRUE( s.Size( f.Rel( f.learningPath ), size ) && size > 0 );              // no list hiding
            const CString folder = f.Rel( f.learningPath ).SubstrFromRange( 0, (UInt32) f.Rel( f.learningPath ).HasChar( '/', false ) );
            VFS::CArchive::TStringVector names;
            ASSERT_TRUE( s.List( folder, names ) && !names.empty() );
            ASSERT_TRUE( s.Exists( f.Rel( f.learningPath ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, pictures: a device reads the picture it was given by the service (GetThumbnails) and is refused the others in the thumbnail folder; the owner reads them all" )
        try
        {
            ASSERT_TRUE( ready );
            CDynamicBuffer png;
            ASSERT_TRUE( MakePng( 32, 32, false, png ) );
            const CString assets[ 2 ] = { f.restrictedAsset, f.plainAsset };
            for ( UInt32 i=0; i<2; ++i )
            {
                CAssetThumbnail thumbnail;
                thumbnail.assetId = assets[ i ];
                thumbnail.kind = "thumbnail";
                thumbnail.size = 32;
                thumbnail.key.size = 32;
                thumbnail.key.rendererId = "test";
                thumbnail.encoded = png;
                CAssetServiceResult put;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->PutThumbnail( "share-put-" + ToString( i ), thumbnail, put ) );
            }
            CString::StringVector ids;
            ids.push_back( f.restrictedAsset );
            ids.push_back( f.plainAsset );
            CAssetThumbnailPage mine;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->GetThumbnails( ids, 32, mine ) && 2 == mine.entries.size() );
            const CString restrictedPicture = f.Rel( mine.entries[ 0 ].vfsPath );
            const CString plainPicture = f.Rel( mine.entries[ 1 ].vfsPath );
            ASSERT_TRUE( !restrictedPicture.IsNULLOrEmpty() && !plainPicture.IsNULLOrEmpty() && restrictedPicture != plainPicture );
            ASSERT_TRUE( 0 == restrictedPicture.HasSubstr( "thumbnails/", true ) );

            CShareSession s( f );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;

            // not given yet: no picture of the folder is for this device
            ASSERT_FALSE( s.ReadAfterStat( plainPicture, content, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "This picture was not given to this device" ) >= 0 );

            // given: GetThumbnails() of the device itself for the asset of the pack it may have
            CAssetThumbnailPage page;
            CString::StringVector plainOnly;
            plainOnly.push_back( f.plainAsset );
            plainOnly.push_back( f.restrictedAsset );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->GetThumbnails( plainOnly, 32, page ) && 2 == page.entries.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == page.entries[ 0 ].status && ASSETSTORE_STATUS_REFUSED == page.entries[ 1 ].status );
            ASSERT_TRUE( page.entries[ 1 ].vfsPath.IsNULLOrEmpty() && page.entries[ 1 ].message.HasSubstr( "owner" ) >= 0 );
            ASSERT_TRUE( f.Rel( page.entries[ 0 ].vfsPath ) == plainPicture );
            ASSERT_TRUE( s.ReadAfterStat( plainPicture, content, &result ) && content.GetDataSize() > 0 );
            ASSERT_FALSE( s.ReadAfterStat( restrictedPicture, content, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            s.Close();

            // the owner reads both
            CShareSession o( f );
            ASSERT_TRUE( o.Open( "owner", GUCEF_NULL, true ) );
            ASSERT_TRUE( o.ReadAfterStat( restrictedPicture, content, &result ) && o.ReadAfterStat( plainPicture, content, &result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, registry: the role taken away in the registry takes effect at the next call and the role given back works again; a revoked device cannot continue" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "revoked", &viewerRoles, false ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) );
            CString error;
            ASSERT_TRUE( f.world.host->SetDeviceRoles( s.credentials.deviceId, Roles( "curator" ), "test", error ) );
            ASSERT_FALSE( s.ReadAfterStat( "notes/readme.txt", content, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( f.world.host->SetDeviceRoles( s.credentials.deviceId, Roles( "viewer" ), "test", error ) );
            ASSERT_TRUE( s.ReadAfterStat( "staging/job1/staged.txt", content, &result ) );
            // a role that gets to read more is a role of the next call too: the machine role refuses the bytes at once
            ASSERT_TRUE( f.world.host->SetDeviceRoles( s.credentials.deviceId, Roles( "mcp" ), "test", error ) );
            ASSERT_FALSE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) );
            ASSERT_TRUE( result.message.HasSubstr( "withheld from the machine role" ) >= 0 );
            ASSERT_TRUE( f.world.host->SetDeviceRoles( s.credentials.deviceId, Roles( "viewer" ), "test", error ) );
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( f.plainPath ), content, &result ) );

            ASSERT_TRUE( f.world.host->RevokeDevice( s.credentials.deviceId, "lost", "test", error ) );
            PumpFor( 1500 );
            ASSERT_FALSE( s.ReadAfterStat( "notes/readme.txt", content, &result ) );
            ASSERT_FALSE( DRNUTILS::REMOTESHARE_ONLINE == s.mount->GetState() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, names: .., absolute, drive, backslash and odd names are refused (by the archive and by the decision of the host), what lies outside the root is not reachable" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
            CDynamicBuffer canonical;
            ASSERT_TRUE( s.ReadAfterStat( "notes/readme.txt", canonical ) && canonical.GetDataSize() > 0 );
            CDynamicBuffer outside;
            ASSERT_TRUE( ReadVfsFile( f.base + "/source/Fields/pack.json", outside ) && outside.GetDataSize() > 0 );          // above the root of the share, in the VFS of the service

            // names that leave the root are never read; the plugin of the VFS makes some of the others canonical before they are sent (a leading or
            // doubled separator, a "." part, a backslash), so what they read is the file they stand for and nothing else
            const char* leaving[] = { "../source/Fields/pack.json", "notes/../../source/Fields/pack.json", "../main/notes/readme.txt", "notes/../../main/notes/readme.txt", "C:/notes/readme.txt", "notes/readme.txt/..", "..", "" };
            for ( UInt32 i=0; i<sizeof( leaving ) / sizeof( leaving[ 0 ] ); ++i )
            {
                CDynamicBuffer content;
                const bool read = s.ReadAfterStat( leaving[ i ], content );
                if ( read && content.GetDataSize() > 0 )
                    GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ShareNames: the name \"" << leaving[ i ] << "\" was read" );
                ASSERT_FALSE( read && content.GetDataSize() > 0 );
            }
            const char* odd[] = { "/notes/readme.txt", "notes\\readme.txt", "notes//readme.txt", "./notes/readme.txt", "notes/./readme.txt" };
            for ( UInt32 i=0; i<sizeof( odd ) / sizeof( odd[ 0 ] ); ++i )
            {
                CDynamicBuffer content;
                if ( s.ReadAfterStat( odd[ i ], content ) )
                    ASSERT_TRUE( SameBytes( content, canonical ) );
            }
            ASSERT_FALSE( s.Exists( "../source/Fields/pack.json" ) );
            ASSERT_FALSE( s.Exists( "catalogue/db" ) );
            ASSERT_FALSE( s.Exists( "source/Fields/pack.json" ) );

            // the decision of the host on its own, with the roles of a device record
            CAssetFileServingGate gate( f.slow );
            CAssetShareDecision decision( f.world.serverService, gate, CString::StringSet() );
            decision.SetShareRoot( "assets", f.mainRoot );
            DRNUTILS::CDeviceRecord device;
            device.deviceId = "d-TEST";
            device.status = DRNUTILS::DEVICESTATUS_ACTIVE;
            device.roles.insert( "owner" );
            CString refusal;
            ASSERT_TRUE( decision.MayRead( device, "assets", "notes/readme.txt", refusal ) );
            const char* refusedByDecision[] = { "../source/Fields/pack.json", "notes/../../source/Fields/pack.json", "/notes/readme.txt", "notes\\readme.txt", "C:/notes/readme.txt", "notes//readme.txt", "./notes/readme.txt", "notes/./readme.txt", "notes/readme.txt/..", "..", "" };
            for ( UInt32 i=0; i<sizeof( refusedByDecision ) / sizeof( refusedByDecision[ 0 ] ); ++i )
                ASSERT_FALSE( decision.MayRead( device, "assets", refusedByDecision[ i ], refusal ) );
            ASSERT_FALSE( decision.MayRead( device, "elsewhere", "notes/readme.txt", refusal ) );
            decision.Shutdown( 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, devices and files: each device reads what the gate lets it have and is refused the rest (the owner everything, a viewer all but the noRedistribution pack, the machine role nothing, a learning role all but the noMachineLearning pack), the bytes it gets are the bytes of the file of the service, and a refusal is ACCESS_DENIED" )
        try
        {
            ASSERT_TRUE( ready );
            const CString servicePaths[ 5 ] = { f.plainPath, f.restrictedPath, f.learningPath, f.mainRoot + "/staging/job1/staged.txt", f.mainRoot + "/bigpack/original/big.bin" };
            struct SDevice { const char* slot; const CString::StringSet* roles; bool grantOwner; bool mayRead[ 5 ]; };
            const SDevice devices[ 4 ] =
            {
                { "owner", GUCEF_NULL, true, { true, true, true, true, true } },
                { "viewer", &viewerRoles, false, { true, false, true, true, true } },
                { "mcp", &mcpRoles, false, { false, false, false, false, false } },
                { "learner", &learnerRoles, true, { true, true, false, true, true } }
            };
            for ( UInt32 d=0; d<4; ++d )
            {
                CShareSession s( f );
                ASSERT_TRUE( s.Open( devices[ d ].slot, devices[ d ].roles, devices[ d ].grantOwner ) );
                for ( UInt32 p=0; p<5; ++p )
                {
                    CDynamicBuffer viaShare, onDisk;
                    VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
                    const bool shareRead = s.ReadAfterStat( f.Rel( servicePaths[ p ] ), viaShare, &result );
                    if ( devices[ d ].mayRead[ p ] != shareRead )
                        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ShareMatrix: " << devices[ d ].slot << " " << servicePaths[ p ] << ": expected " << devices[ d ].mayRead[ p ] << ", read " << shareRead );
                    ASSERT_TRUE( devices[ d ].mayRead[ p ] == shareRead );
                    if ( shareRead )
                    {
                        ASSERT_TRUE( ReadVfsFile( servicePaths[ p ], onDisk ) && viaShare.GetDataSize() > 0 && SameBytes( viaShare, onDisk ) );
                    }
                    else
                    {
                        ASSERT_TRUE( 0 == viaShare.GetDataSize() && FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
                        // the names and the sizes of what is refused stay visible
                        UInt64 size = 0;
                        ASSERT_TRUE( s.Size( f.Rel( servicePaths[ p ] ), size ) && size > 0 );
                    }
                }
                s.Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, thumbnail upload permission: the owner device puts and replaces a thumbnail, a viewer is refused (with and without a command id) and keeps its read access to what the owner put" )
        try
        {
            ASSERT_TRUE( ready );
            CDynamicBuffer png;
            ASSERT_TRUE( MakePng( 48, 48, false, png ) );
            CAssetThumbnail thumbnail;
            thumbnail.assetId = f.plainAsset;
            thumbnail.kind = "thumbnail";
            thumbnail.size = 48;
            thumbnail.key.size = 48;
            thumbnail.key.rendererId = "test";
            thumbnail.encoded = png;
            // the owner puts it, and replaces it
            CAssetServiceResult put;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->PutThumbnail( "perm-put-owner-1", thumbnail, put ) );
            thumbnail.key.rendererId = "test-replaced";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->PutThumbnail( "perm-put-owner-2", thumbnail, put ) );

            CShareSession s( f );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            // the viewer is refused, with or without a command id
            CAssetServiceResult refused;
            thumbnail.key.rendererId = "test-viewer";
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == s.client->PutThumbnail( "perm-put-viewer-1", thumbnail, refused ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == s.client->PutThumbnail( CString::Empty, thumbnail, refused ) );
            // the viewer reads what the owner put: the picture it is given through the share
            CString::StringVector ids;
            ids.push_back( f.plainAsset );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == s.client->GetThumbnails( ids, 48, page ) && 1 == page.entries.size() && ASSETSTORE_STATUS_OK == page.entries[ 0 ].status );
            CDynamicBuffer content;
            ASSERT_TRUE( s.ReadAfterStat( f.Rel( page.entries[ 0 ].vfsPath ), content ) && Sha256Of( content ) == Sha256Of( png ) );
            // the picture is still the one of the owner
            CAssetThumbnailPage ownerPage;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == f.owner->GetThumbnails( ids, 48, ownerPage ) && 1 == ownerPage.entries.size() && ASSETSTORE_STATUS_OK == ownerPage.entries[ 0 ].status );
            ASSERT_TRUE( ownerPage.entries[ 0 ].key.rendererId == "test-replaced" );
            s.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, the node's thread: the first read of a pack whose restrictions are not kept yet is not refused: the node answers busy at once (no query of the catalogue on its thread, however slow it is), the pool makes the answer, the client asks again by itself and the read succeeds; the owner and the machine role are not looked up and not delayed, and the next read is decided at once" )
        try
        {
            ASSERT_TRUE( ready );
            CString extraPack, extraAsset, extraPath;
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            UInt64 elapsed = 0;
            CDynamicBuffer plainOfViewer;
            {
                CShareSession s( f );
                // a change of the catalogue (a pack is added): nothing is kept for the new revision
                ASSERT_TRUE( f.IngestPack( "Extra", "\"attributionRequired\"", 5, extraPack, extraAsset, extraPath ) );
                ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
                f.slow->delayMs.Store( 700 );
                const UInt32 slowBefore = f.slow->slowCalls.Load();

                // no refusal reaches the client: the read waits for the answer of the pool and succeeds
                ASSERT_TRUE( s.Read( f.Rel( f.plainPath ), plainOfViewer, &result, &elapsed ) );
                ASSERT_TRUE( plainOfViewer.GetDataSize() > 0 );
                ASSERT_TRUE( elapsed >= 500 && elapsed < 10000 );

                // the answer was made on the pool, never on the thread of the node; the client waited by retrying the busy answer
                ASSERT_TRUE( f.slow->slowCalls.Load() > slowBefore );
                ASSERT_TRUE( 0 == f.slow->slowCallsOnNodeThread.Load() );
                VFS::CVfsRemoteStatistics statistics;
                VFS::CIRemoteArchive* remote = s.archive->GetRemoteArchiveInterface();
                ASSERT_TRUE( GUCEF_NULL != remote );
                if ( GUCEF_NULL != remote )
                    remote->GetRemoteStatistics( statistics );
                ASSERT_TRUE( statistics.busyRetries >= 1 );
                ASSERT_TRUE( f.world.binding->GetShareDecision()->GetNotKeptCount() >= 1 && f.world.binding->GetShareDecision()->GetWarmCount() >= 1 );

                // the next read is decided at once, by the gate
                ASSERT_TRUE( s.Read( f.Rel( f.plainPath ), content, &result, &elapsed ) && elapsed < 400 && SameBytes( content, plainOfViewer ) );
                ASSERT_TRUE( 0 == f.slow->slowCallsOnNodeThread.Load() );
                s.Close();
            }

            // the catalogue changes again and answers slowly: the owner and the machine role are not looked up, so nothing waits for the catalogue
            ASSERT_TRUE( f.IngestPack( "ExtraAgain", "\"attributionRequired\"", 8, extraPack, extraAsset, extraPath ) );
            const UInt64 notKeptBefore = f.world.binding->GetShareDecision()->GetNotKeptCount();
            {
                CShareSession o( f );
                ASSERT_TRUE( o.Open( "owner", GUCEF_NULL, true ) );
                CDynamicBuffer ownerContent;
                ASSERT_TRUE( o.Read( f.Rel( f.plainPath ), ownerContent, &result, &elapsed ) && SameBytes( ownerContent, plainOfViewer ) );
                VFS::CVfsRemoteStatistics ownerStatistics;
                o.archive->GetRemoteArchiveInterface()->GetRemoteStatistics( ownerStatistics );
                ASSERT_TRUE( 0 == ownerStatistics.busyRetries );
                o.Close();
            }
            {
                CShareSession m( f );
                ASSERT_TRUE( m.Open( "mcp", &mcpRoles, false ) );
                ASSERT_FALSE( m.Read( f.Rel( f.plainPath ), content, &result, &elapsed ) );
                ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult && result.message.HasSubstr( "withheld from the machine role" ) >= 0 );
                VFS::CVfsRemoteStatistics machineStatistics;
                m.archive->GetRemoteArchiveInterface()->GetRemoteStatistics( machineStatistics );
                ASSERT_TRUE( 0 == machineStatistics.busyRetries );
                m.Close();
            }
            ASSERT_TRUE( notKeptBefore == f.world.binding->GetShareDecision()->GetNotKeptCount() );
            ASSERT_TRUE( 0 == f.slow->slowCallsOnNodeThread.Load() );
            f.slow->delayMs.Store( 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, a real refusal after the wait: the first read of a pack that the gate refuses waits for the restrictions like any other and then ends with ACCESS_DENIED in the words of the gate (never the words of the lookup); once they are kept the same refusal is immediate" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            CString extraPack, extraAsset, extraPath;
            ASSERT_TRUE( f.IngestPack( "ExtraRefused", "\"attributionRequired\"", 6, extraPack, extraAsset, extraPath ) );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            f.slow->delayMs.Store( 300 );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            UInt64 elapsed = 0;
            ASSERT_FALSE( s.Read( f.Rel( f.restrictedPath ), content, &result, &elapsed ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "does not allow its files to leave the devices of the owner" ) >= 0 );
            ASSERT_TRUE( result.message.HasSubstr( "being looked up" ) < 0 );
            ASSERT_TRUE( 0 == content.GetDataSize() );
            ASSERT_TRUE( elapsed >= 200 );                                   // it waited for the restrictions first
            ASSERT_FALSE( s.Read( f.Rel( f.restrictedPath ), content, &result, &elapsed ) );
            ASSERT_TRUE( FILESYSTEMACCESS_ACCESS_DENIED == result.fileSystemResult && elapsed < 250 );
            ASSERT_TRUE( 0 == f.slow->slowCallsOnNodeThread.Load() );
            f.slow->delayMs.Store( 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, retries that run out: when the lookup takes longer than the retries of the client wait, the read ends with the retry-later result and the words of the lookup, nothing is left stuck, and the next call succeeds without anyone doing anything" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            CString extraPack, extraAsset, extraPath;
            ASSERT_TRUE( f.IngestPack( "ExtraSlow", "\"attributionRequired\"", 7, extraPack, extraAsset, extraPath ) );
            s.bridgeSettings.SetAttribute( "busyRetries", CString( "2" ) );
            s.bridgeSettings.SetAttribute( "busyBackoffMs", CString( "20" ) );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            f.slow->delayMs.Store( 1500 );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            UInt64 elapsed = 0;
            ASSERT_FALSE( s.Read( f.Rel( f.plainPath ), content, &result, &elapsed ) );
            ASSERT_TRUE( FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == result.fileSystemResult );
            ASSERT_TRUE( result.message.HasSubstr( "being looked up" ) >= 0 );
            ASSERT_TRUE( elapsed < 1000 );                                   // the retries were used up, not waited past
            ASSERT_TRUE( 0 == f.slow->slowCallsOnNodeThread.Load() );

            // the pool finishes the lookup by itself; the next call is served
            PumpFor( 2200 );
            f.slow->delayMs.Store( 0 );
            ASSERT_TRUE( s.Read( f.Rel( f.plainPath ), content, &result, &elapsed ) && content.GetDataSize() > 0 );
            ASSERT_TRUE( elapsed < 400 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, digests: the digest of the catalogue is given only for a file that is the size the catalogue has of it; a file that was changed on the disk has none, and its bytes are what is on the disk" )
        try
        {
            CShareFixture g( "digest" );
            ASSERT_TRUE( g.started );
            CString pack, asset, path;
            ASSERT_TRUE( g.IngestPack( "Fields", "\"attributionRequired\"", 3, pack, asset, path ) );
            CAssetRecord record;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.owner->GetAsset( asset, record ) );
            CString catalogueDigest;
            for ( UInt32 i=0; i<record.files.size(); ++i )
                if ( "main" == record.files[ i ].role )
                    catalogueDigest = record.files[ i ].digest;
            ASSERT_TRUE( 64 == catalogueDigest.Length() );
            {
                CShareSession s( g );
                ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
                ASSERT_TRUE( CAssetDigest::AreEqual( catalogueDigest, s.Hash( g.Rel( path ) ) ) );
            }

            // the catalogue changes (a digest of its table, then a change that moves its revision): the files kept of the pack are made again
            {
                CPrimaryAssetStore* store = g.slow->GetPrimaryStore();
                ASSERT_TRUE( GUCEF_NULL != store );
                CDataTableQuery query;
                query.WhereEquals( "asset_id", CVariant( asset ) );
                query.WhereEquals( "role", CVariant( CString( "main" ) ) );
                CAssetCatalogue::TRowVector rows;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetCatalogue().Select( "asset_files", query, rows ) && 1 == rows.size() );
                rows[ 0 ][ "digest" ] = CVariant( CString( "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff" ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetCatalogue().Write( "asset_files", rows ) );
                CString otherPack, otherAsset, otherPath;
                ASSERT_TRUE( g.IngestPack( "Vault", "\"attributionRequired\"", 2, otherPack, otherAsset, otherPath ) );
                CShareSession s( g );
                ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
                ASSERT_TRUE( CAssetDigest::AreEqual( "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff", s.Hash( g.Rel( path ) ) ) );
            }
            CDynamicBuffer changed;
            MakePattern( 3333, 99, changed );
            ASSERT_TRUE( WriteVfsFile( path, changed ) );
            CShareSession s( g );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
            ASSERT_TRUE( s.Hash( g.Rel( path ) ).IsNULLOrEmpty() );
            CDynamicBuffer fromShare;
            ASSERT_TRUE( s.Read( g.Rel( path ), fromShare ) && SameBytes( fromShare, changed ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, restrictions kept per directory: a pack of the earlier layout (its folder in the location) keeps its flags for the files in its folder and not for the files next to it" )
        try
        {
            CShareFixture g( "legacy" );
            ASSERT_TRUE( g.started );
            CString vaultPack, vaultAsset, vaultPath;
            ASSERT_TRUE( g.IngestPack( "Vault", "\"noRedistribution\"", 2, vaultPack, vaultAsset, vaultPath ) );

            // the pack takes the earlier layout: its folder, which holds the files, is a folder of the location
            CPrimaryAssetStore* store = g.slow->GetPrimaryStore();
            ASSERT_TRUE( GUCEF_NULL != store );
            CDataTableQuery query;
            query.WhereEquals( "pack_id", CVariant( vaultPack ) );
            CAssetCatalogue::TRowVector rows;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetCatalogue().Select( "packs", query, rows ) && 1 == rows.size() );
            rows[ 0 ][ "root_path" ] = CVariant( CString( "packs/Legacy/Vault" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetCatalogue().Write( "packs", rows ) );
            ASSERT_TRUE( WriteVfsFile( g.mainRoot + "/packs/Legacy/Vault/original/art.bin", CString( "the files of a pack that is not for the devices of others" ) ) );
            ASSERT_TRUE( WriteVfsFile( g.mainRoot + "/packs/Legacy/Vault/original/deeper/art2.bin", CString( "and the files below them" ) ) );
            ASSERT_TRUE( WriteVfsFile( g.mainRoot + "/packs/Other/open.bin", CString( "a file next to the folder of the pack" ) ) );
            ASSERT_TRUE( WriteVfsFile( g.mainRoot + "/packs/open2.bin", CString( "a file in the folder above it" ) ) );
            CString fieldsPack, fieldsAsset, fieldsPath;
            ASSERT_TRUE( g.IngestPack( "Fields", "\"attributionRequired\"", 3, fieldsPack, fieldsAsset, fieldsPath ) );        // a change of the catalogue: nothing kept before is current

            CShareSession s( g );
            ASSERT_TRUE( s.Open( "viewer", &viewerRoles, false ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            const char* open[] = { "packs/Other/open.bin", "packs/open2.bin", "packs/Other/open.bin" };
            const char* shut[] = { "packs/Legacy/Vault/original/art.bin", "packs/Legacy/Vault/original/deeper/art2.bin" };
            for ( UInt32 round=0; round<2; ++round )
            {
                for ( UInt32 i=0; i<sizeof( shut ) / sizeof( shut[ 0 ] ); ++i )
                {
                    ASSERT_FALSE( s.ReadAfterStat( shut[ i ], content, &result ) );
                    ASSERT_TRUE( result.message.HasSubstr( "does not allow its files to leave the devices of the owner" ) >= 0 );
                }
                for ( UInt32 i=0; i<sizeof( open ) / sizeof( open[ 0 ] ); ++i )
                    ASSERT_TRUE( s.ReadAfterStat( open[ i ], content, &result ) && content.GetDataSize() > 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, locations: a location that goes offline takes the share away and one that comes back brings it back, and the mount of a device follows" )
        try
        {
            CShareFixture g( "offline" );
            ASSERT_TRUE( g.started );
            CString pack, asset, path;
            ASSERT_TRUE( g.IngestPack( "Fields", "\"attributionRequired\"", 3, pack, asset, path ) );
            CShareSession s( g );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_TRUE( s.ReadAfterStat( g.Rel( path ), content, &result ) && content.GetDataSize() > 0 );
            ASSERT_TRUE( g.world.host->IsShareServed( "assets" ) );

            // the root of the location is gone from the VFS: the service says so and the host stops sharing it
            UnmountDirectory( "sh_offline_main" );
            CAssetLocationPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.slow->RefreshLocationStates( page ) );
            for ( UInt32 waited=0; waited<5000 && g.world.host->HasShare( "assets" ); waited+=10 )
                PumpFor( 10 );
            ASSERT_FALSE( g.world.host->HasShare( "assets" ) );
            ASSERT_FALSE( g.world.host->IsShareServed( "assets" ) );
            CString::StringVector names;
            g.world.binding->GetShareNames( names );
            ASSERT_TRUE( names.empty() );
            ASSERT_FALSE( s.ReadAfterStat( g.Rel( path ), content, &result ) );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "ShareOffline: a read of a share that is gone: result " << (Int32) result.fileSystemResult << ", " << result.message );
            ASSERT_TRUE( FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.fileSystemResult || FILESYSTEMACCESS_NOT_FOUND == result.fileSystemResult );

            // back: the share is served again and the same mount reads
            VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( TestDirectory( "sh_offline_main" ), "AssetStoreTest_sh_offline_main", true, false, g.mainRoot );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.slow->RefreshLocationStates( page ) );
            for ( UInt32 waited=0; waited<5000 && !g.world.host->IsShareServed( "assets" ); waited+=10 )
                PumpFor( 10 );
            ASSERT_TRUE( g.world.host->IsShareServed( "assets" ) );
            bool back = false;
            for ( UInt32 attempt=0; attempt<40 && !back; ++attempt )
            {
                back = s.ReadAfterStat( g.Rel( path ), content, &result ) && content.GetDataSize() > 0;
                if ( !back )
                    PumpFor( 250 );
            }
            ASSERT_TRUE( back );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, restart: the host stopped and started again serves the share again on its new session, and the mount of the device comes back by itself" )
        try
        {
            CShareFixture g( "restart" );
            ASSERT_TRUE( g.started );
            CString pack, asset, path;
            ASSERT_TRUE( g.IngestPack( "Fields", "\"attributionRequired\"", 3, pack, asset, path ) );
            CShareSession s( g );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
            CDynamicBuffer content;
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
            ASSERT_TRUE( s.ReadAfterStat( g.Rel( path ), content, &result ) );

            g.world.StopHost();
            ASSERT_TRUE( s.PumpUntilState( DRNUTILS::REMOTESHARE_OFFLINE, 20000 ) );
            CDynamicBuffer unreachable;
            ASSERT_FALSE( s.Read( "notes/readme.txt", unreachable, &result ) );
            ASSERT_TRUE( FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result.fileSystemResult );
            CString error;
            ASSERT_TRUE( g.world.StartHost( error ) );
            ASSERT_TRUE( g.world.host->IsShareServed( "assets" ) );
            ASSERT_TRUE( s.PumpUntilOnline( 60000 ) );
            CDynamicBuffer again;
            ASSERT_TRUE( s.ReadAfterStat( g.Rel( path ), again, &result ) && SameBytes( again, content ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Share, two mounts of one device on one connection: stopping the first leaves the second online and reading" )
        try
        {
            ASSERT_TRUE( ready );
            CShareSession s( f );
            ASSERT_TRUE( s.Open( "owner", GUCEF_NULL, true ) );
            DRNUTILS::CRemoteServiceConnection* connection = ConnectionOfClient( *s.client );
            ASSERT_TRUE( GUCEF_NULL != connection );
            DRNUTILS::CRemoteShareMountSettings settings;
            settings.share = "assets";
            settings.channel = 0;
            settings.mountPoint = "astest/shm_main_second";
            DRNUTILS::CRemoteShareMount second( *connection, settings );
            CString error;
            ASSERT_TRUE( second.Start( s.credentials.drnSessionName, error ) );
            for ( UInt32 waited=0; waited<20000 && DRNUTILS::REMOTESHARE_ONLINE != second.GetState(); waited+=5 )
                PumpFor( 5 );
            ASSERT_TRUE( DRNUTILS::REMOTESHARE_ONLINE == second.GetState() );
            DRN::CDRNSession* session = connection->GetDrnSession( 0 );
            ASSERT_TRUE( GUCEF_NULL != session && 2 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );

            s.mount->Stop();
            ASSERT_TRUE( 1 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
            ASSERT_TRUE( DRNUTILS::REMOTESHARE_ONLINE == second.GetState() );
            VFS::CVFS::TArchivePtr secondArchive = VFS::CVfsGlobal::Instance()->GetVfs().GetMountedArchive( second.GetArchiveName() );
            ASSERT_TRUE( !secondArchive.IsNULL() );
            CArchiveProbe probe( secondArchive, CArchiveProbe::OPERATION_READ, "notes/readme.txt" );
            ASSERT_TRUE( probe.Run() && probe.ok && probe.content.GetDataSize() > 0 );
            second.Stop();
            ASSERT_TRUE( 0 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
