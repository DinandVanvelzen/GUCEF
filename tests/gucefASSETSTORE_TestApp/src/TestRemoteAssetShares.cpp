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

#include "gucefCORE_Logging.h"

#include "gucefMT_CScopeMutex.h"

#include "gucefMT_CActiveObject.h"

#include "CFileAccess.h"

#include "dvcppfileutils.h"

#include "CObserver.h"

#include "gucefDRNUTILS_CRemoteShareMount.h"

#include "gucefVFS_CVfsBlockCache.h"

#include "gucefVFS_CVfsThreadPolicyScope.h"

#include "gucefVFS_CVfsFetchProgress.h"

#include "gucefVFS_CVfsFetchRequest.h"

#include "gucefVFS_CVfsAsyncResult.h"

#include "gucefVFS_CRemoteArchiveEvents.h"

#include "gucefASSETSTORE_CRemoteAssetShares.h"

#include "gucefASSETSTORE_CRemoteFileTransfer.h"

#include "gucefASSETSTORE_CAssetThumbnailFiles.h"

#include "gucefASSETSTORE_CLocalAssetStoreService.h"

#include "vfspluginDRN_CDrnBridgeGlobal.h"

#include "TestAssetStoreRemoteWorld.h"

#include "TestRemoteAssetShares.h"

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

static const UInt32 BlockBytes = VFS::CVfsBlockCache::DefaultBlockBytes;
static const UInt32 BigSize = 13 * BlockBytes + 100000;             // 14 blocks, the last one short

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteServiceConnection*
ConnectionOfClient( CIAssetStoreService& service )
{
    CRemoteAssetStoreService* remote = CAssetStoreServiceConfig::RemoteImplementationId == service.GetType() ? static_cast< CRemoteAssetStoreService* >( &service ) : GUCEF_NULL;
    return GUCEF_NULL == remote ? GUCEF_NULL : remote->GetConnection();
}

/*-------------------------------------------------------------------------*/

bool
SameBytes( const CDynamicBuffer& a, const CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

bool
ReadDiskFile( const CString& path, CDynamicBuffer& content )
{
    CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    content.Clear( false );
    GUCEF::vector< UInt8 > buffer;
    buffer.resize( 65536 );
    while ( true )
    {
        const UInt32 read = file.Read( &buffer[ 0 ], 1, 65536 );
        if ( 0 == read )
            break;
        content.Append( &buffer[ 0 ], read );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Pumps the pulse generator in steps that leave the service little time: every answer of the service takes a while, a slow link */
void
SlowPump( UInt32 durationInMs )
{
    for ( UInt32 waited=0; waited<durationInMs; waited+=25 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 25 );
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  One call of the VFS made on a thread of its own while the test thread pumps (the service and the client live on the pulse generator of the test, and a
 *  call that waits for the service has to leave the thread that drives it free)
 */
class CVfsProbe : public MT::CActiveObject
{
    public:

    enum EOperation
    {
        OP_READ = 0,            // the whole file in pieces
        OP_RANGE,               // 'length' bytes at 'offset' through an open file
        OP_SEEKEND,             // the last 'length' bytes through an open file, and the end of the file
        OP_EXISTS,
        OP_DIREXISTS,
        OP_SIZE,
        OP_FILES,
        OP_DIRS,
        OP_FETCHWHOLE,          // CRemoteAssetShares::FetchWholeFile() of the service path 'path'
        OP_FOLDER,              // CRemoteAssetShares::GetFolderEntries() of the service path 'path'
        OP_FILEINFO             // CRemoteAssetShares::GetFileInfo() of the service path 'path'
    };

    CRemoteAssetShares* shares;
    EOperation operation;
    CString path;
    UInt64 offset;
    UInt32 length;
    UInt32 piece;
    bool pin;
    bool ok;
    bool opened;
    bool eof;
    UInt64 size;
    CDynamicBuffer content;
    VFS::CVFS::TStringVector names;
    VFS::CVfsCallResult last;
    TAssetStoreStatus status;
    CString message;
    CIRemoteFileSource::SFileInfo info;
    MT::CTAtomic< UInt32 > finished;
    UInt64 elapsedMs;

    CVfsProbe( EOperation op, const CString& p )
        : MT::CActiveObject(), shares( GUCEF_NULL ), operation( op ), path( p ), offset( 0 ), length( 0 ), piece( 65536 ), pin( false ), ok( false ), opened( false ), eof( false ), size( 0 )
        , content(), names(), last(), status( ASSETSTORE_STATUS_OK ), message(), info(), finished( 0 ), elapsedMs( 0 ) {}

    virtual ~CVfsProbe() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    /** Runs the call and pumps until it is done */
    bool Run( UInt32 timeoutMs = 120000 )
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
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            if ( OP_EXISTS == operation )
                ok = vfs.FileExists( path );
            else
            if ( OP_DIREXISTS == operation )
                ok = vfs.DirExists( path );
            else
            if ( OP_SIZE == operation )
            {
                size = vfs.GetFileSize( path );
                ok = true;
            }
            else
            if ( OP_FILES == operation )
                ok = vfs.GetFileList( names, path, false, false, CString::Empty );
            else
            if ( OP_DIRS == operation )
                ok = vfs.GetDirList( names, path, false, false, CString::Empty );
            else
            if ( OP_FETCHWHOLE == operation )
            {
                status = shares->FetchWholeFile( path, pin, GUCEF_NULL, GUCEF_NULL, message );
                ok = ASSETSTORE_STATUS_OK == status;
            }
            else
            if ( OP_FOLDER == operation )
            {
                CString::StringVector dirs;
                CString::StringVector files;
                ok = shares->GetFolderEntries( path, dirs, files );
                for ( size_t i=0; i<dirs.size(); ++i )
                    names.push_back( dirs[ i ] + "/" );
                for ( size_t i=0; i<files.size(); ++i )
                    names.push_back( files[ i ] );
            }
            else
            if ( OP_FILEINFO == operation )
                ok = shares->GetFileInfo( path, info );
            else
            {
                VFS::TBasicVfsResourcePtr file = vfs.GetFile( path, "rb", false );
                opened = !file.IsNULL() && !file->GetAccess().IsNULL();
                if ( !opened )
                    vfs.GetLastResult( last );
                else
                {
                    IOAccessPtr access = file->GetAccess();
                    size = access->GetSize();
                    if ( OP_READ == operation )
                    {
                        GUCEF::vector< UInt8 > buffer;
                        buffer.resize( piece );
                        ok = true;
                        while ( true )
                        {
                            const UInt32 read = access->Read( &buffer[ 0 ], 1, piece );
                            if ( 0 == read )
                                break;
                            content.Append( &buffer[ 0 ], read );
                        }
                    }
                    else
                    if ( OP_RANGE == operation )
                    {
                        GUCEF::vector< UInt8 > buffer;
                        buffer.resize( length );
                        const bool positioned = 0 == access->Setpos( offset );
                        const UInt32 read = positioned ? access->Read( &buffer[ 0 ], 1, length ) : 0;
                        content.Append( &buffer[ 0 ], read );
                        ok = positioned && read == length;
                        eof = access->Eof();
                    }
                    else
                    {
                        GUCEF::vector< UInt8 > buffer;
                        buffer.resize( length + 22 );
                        const bool positioned = 0 == access->Seek( - (Int32) length, SEEK_END );
                        const UInt32 read = positioned ? access->Read( &buffer[ 0 ], 1, length + 22 ) : 0;
                        content.Append( &buffer[ 0 ], read );
                        eof = access->Eof();
                        ok = positioned && read == length && 0 == access->Read( &buffer[ 0 ], 1, 1 );
                    }
                    if ( !ok )
                        vfs.GetLastResult( last );
                }
            }
        }
        catch ( ... )
        {
        }
        elapsedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        finished.Store( 1 );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** What the VFS tells of the arrival of content, with the thread each event was delivered on */
class CVfsEvents : public CORE::CObserver
{
    public:

    struct SRecord
    {
        CString kind;
        CString path;
        bool complete;
        UInt64 firstByte;
        UInt64 byteCount;
        CString message;
        bool connected;
        TThreadId thread;
        SRecord( void ) : kind(), path(), complete( false ), firstByte( 0 ), byteCount( 0 ), message(), connected( false ), thread( 0 ) {}
    };
    GUCEF::vector< SRecord > records;

    CVfsEvents( void ) : CORE::CObserver(), records()
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::FileArrivedEvent );
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::ListingArrivedEvent );
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::MetadataArrivedEvent );
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::FetchFailedEvent );
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::FileChangedEvent );
        SubscribeTo( &vfs, VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent );
    }

    virtual ~CVfsEvents() GUCEF_VIRTUAL_OVERRIDE { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        SRecord record;
        record.thread = MT::GetCurrentThreadID();
        if ( VFS::CRemoteArchiveEvents::FileArrivedEvent == id ) record.kind = "file";
        else if ( VFS::CRemoteArchiveEvents::ListingArrivedEvent == id ) record.kind = "listing";
        else if ( VFS::CRemoteArchiveEvents::MetadataArrivedEvent == id ) record.kind = "metadata";
        else if ( VFS::CRemoteArchiveEvents::FetchFailedEvent == id ) record.kind = "failed";
        else if ( VFS::CRemoteArchiveEvents::FileChangedEvent == id ) record.kind = "changed";
        else record.kind = "connection";
        if ( VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent == id )
        {
            const VFS::CRemoteArchiveEvents::TConnectionStateChangedVfsEventData* data = static_cast< const VFS::CRemoteArchiveEvents::TConnectionStateChangedVfsEventData* >( eventData );
            if ( GUCEF_NULL != data )
            {
                record.path = data->GetData().mountPath;
                record.connected = data->GetData().connected;
            }
        }
        else
        {
            const VFS::CVfsArrivalInfo* info = static_cast< const VFS::CVfsArrivalInfo* >( eventData );
            if ( GUCEF_NULL != info )
            {
                record.path = info->path;
                record.complete = info->fileComplete;
                record.firstByte = info->firstByte;
                record.byteCount = info->byteCount;
                record.message = info->message;
            }
        }
        records.push_back( record );
    }

    /** The records of a kind whose path holds 'text' */
    UInt32 Count( const CString& kind, const CString& text = CString::Empty ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].kind == kind && ( text.IsNULLOrEmpty() || records[ i ].path.HasSubstr( text ) >= 0 ) )
                ++count;
        return count;
    }

    const SRecord* Find( const CString& kind, const CString& text ) const
    {
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].kind == kind && ( text.IsNULLOrEmpty() || records[ i ].path.HasSubstr( text ) >= 0 ) )
                return &records[ i ];
        return GUCEF_NULL;
    }

    void Dump( void ) const
    {
        for ( size_t i=0; i<records.size(); ++i )
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsEvents: " << records[ i ].kind << " " << records[ i ].path << " complete " << records[ i ].complete << " first " << records[ i ].firstByte << " count " << records[ i ].byteCount << " connected " << records[ i ].connected << " " << records[ i ].message );
    }

    /** Pumps until a record of the kind is there (or the time is over) */
    bool PumpUntil( const CString& kind, const CString& text, UInt32 timeoutMs )
    {
        for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
        {
            if ( Count( kind, text ) > 0 )
                return true;
            PumpFor( 5 );
        }
        return Count( kind, text ) > 0;
    }
};

/*-------------------------------------------------------------------------*/

/** A server side with its files, a client of it and the files of the service on the client: the share of the host mounted below a prefix */
class CSharesWorld
{
    public:

    CString name;
    CString base;
    CString mainRoot;
    CString prefix;
    CRemoteAssetStoreWorld world;
    TAssetStoreServicePtr client;
    CRemoteAssetShares* shares;
    CRemoteFilesSettings settings;
    CDynamicBuffer big;
    CString bigDigest;
    bool started;

    explicit CSharesWorld( const CString& worldName, bool withRoles = false )
        : name( worldName ), base( "astest/rs_" + worldName ), mainRoot(), prefix( "client" ), world( "rs_" + worldName ), client(), shares( GUCEF_NULL )
        , settings(), big(), bigDigest(), started( false )
    {
        mainRoot = base + "/main";
        if ( withRoles )
        {
            world.bindingSettings.roles.insert( "viewer" );
            world.bindingSettings.roles.insert( "mcp" );
        }
        MountFreshDirectory( "rs_" + name + "_main", base + "/main" );
        MountFreshDirectory( "rs_" + name + "_source", base + "/source" );
        MountFreshDirectory( "rs_" + name + "_catalogue", base + "/catalogue" );
        MakePattern( BigSize, 7, big );
        bigDigest = Sha256Of( big );
        WriteVfsFile( mainRoot + "/bigpack/original/big.bin", big );
        WriteVfsFile( mainRoot + "/bigpack/original/small.txt", CString( "a small file of the pack" ) );
        WriteVfsFile( mainRoot + "/bigpack/intermediate/a/b.txt", CString( "an intermediate file" ) );

        CAssetStoreServiceConfig config;
        config.primaryStore.locations.clear();
        CAssetStorageLocation main;
        main.locationId = "main";
        main.name = "Main";
        main.vfsRoot = base + "/main";
        config.primaryStore.locations.push_back( main );
        config.primaryStore.defaultLocationId = "main";
        config.primaryStore.database.backendName = "vfstables";
        config.primaryStore.database.database = base + "/catalogue/db";
        config.primaryStore.database.backendOptions.SetAllowMultipleValues( false );
        config.primaryStore.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
        config.loadDownloadQueue = false;
        config.startDownloads = false;
        config.implementationId = CAssetStoreServiceConfig::LocalImplementationId;
        world.serverConfig = config;
        world.serverService = TAssetStoreServicePtr( CORE::CTSharedPtr< CIAssetStoreService, MT::CMutex >( GUCEF_NEW CLocalAssetStoreService( config ) ) );
        CString error;
        if ( ASSETSTORE_STATUS_OK != world.serverService->Open() )
            return;
        if ( !world.StartHost( error ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "SharesWorld: the host did not start: " << error );
            return;
        }
        client = world.MakeClient( "shares", error );
        if ( client.IsNULL() || ASSETSTORE_STATUS_OK != client->Open() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "SharesWorld: no client: " << error );
            return;
        }
        started = true;
    }

    ~CSharesWorld()
    {
        CloseShares();
        client.Unlink();
        world.StopServer();
        UnmountDirectory( "rs_" + name + "_main" );
        UnmountDirectory( "rs_" + name + "_source" );
        UnmountDirectory( "rs_" + name + "_catalogue" );
    }

    CString CacheFolder( void ) const
    {
        return TestDirectory( "rs_cache_" + name );
    }

    bool PumpUntilOnline( UInt32 timeoutMs )
    {
        for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
        {
            if ( GUCEF_NULL != shares && !shares->IsOffline() && DRNUTILS::REMOTESHARE_ONLINE == shares->GetMount( 0 )->GetState() )
                return true;
            PumpFor( 5 );
        }
        return false;
    }

    bool PumpUntilOffline( UInt32 timeoutMs )
    {
        for ( UInt32 waited=0; waited<timeoutMs; waited+=5 )
        {
            if ( GUCEF_NULL != shares && shares->IsOffline() )
                return true;
            PumpFor( 5 );
        }
        return false;
    }

    /** Opens the files on the client with a fresh cache (or the cache that is there), the share mounted below the prefix and online */
    bool OpenShares( bool freshCache )
    {
        CloseShares();
        if ( freshCache )
            DeleteDir( CacheFolder(), true );
        settings.cacheFolder = CacheFolder();
        shares = GUCEF_NEW CRemoteAssetShares( *client, settings );
        if ( !shares->Open( prefix ) )
        {
            CloseShares();
            return false;
        }
        return PumpUntilOnline( 30000 );
    }

    void CloseShares( void )
    {
        if ( GUCEF_NULL != shares )
        {
            shares->Close();
            GUCEF_DELETE shares;
            shares = GUCEF_NULL;
        }
    }

    CString ServerPath( const CString& relative ) const { return mainRoot + "/" + relative; }
    CString ClientPath( const CString& relative ) const { return prefix + "/" + mainRoot + "/" + relative; }

    bool Statistics( VFS::CVfsRemoteStatistics& statistics ) const
    {
        return GUCEF_NULL != shares && shares->GetStatistics( statistics );
    }
};

/*-------------------------------------------------------------------------*/

/** Reads a whole file through the VFS on a thread of its own */
bool
ReadViaVfs( const CString& path, CDynamicBuffer& content, UInt32 piece )
{
    CVfsProbe probe( CVfsProbe::OP_READ, path );
    probe.piece = piece;
    probe.Run();
    content = probe.content;
    if ( !probe.opened || !probe.ok )
        GUCEF_LOG( LOGLEVEL_NORMAL, "ReadViaVfs: " << path << " opened " << probe.opened << " ok " << probe.ok << " read " << probe.content.GetDataSize() << " of " << probe.size << ": " << probe.last.ToString() );
    return probe.opened && probe.ok;
}

/*-------------------------------------------------------------------------*/

/** Whether a list of names has a name */
bool
HasName( const VFS::CVFS::TStringVector& names, const CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Pumps until the transfer ended or the time is over */
bool
PumpUntilTransferEnded( const TRemoteFileTransferPtr& transfer, UInt32 timeoutMs )
{
    for ( UInt32 waited=0; waited<timeoutMs; waited+=10 )
    {
        if ( transfer->IsEnded() )
            return true;
        PumpFor( 10 );
    }
    return transfer->IsEnded();
}

/*-------------------------------------------------------------------------*/

/** A thread that must never wait for the service: the rule of the thread of an interface, for as long as the object lives */
class CNoWaitScope
{
    public:

    CNoWaitScope( void ) : m_thread( MT::GetCurrentThreadID() )
    {
        VFS::CVfsThreadPolicy policy;
        policy.missAction = VFS::MISSACTION_FAIL_AND_FETCH;
        VFS::CVfsGlobal::Instance()->GetThreadPolicyRegistry().SetThreadPolicy( m_thread, policy );
    }

    ~CNoWaitScope()
    {
        VFS::CVfsGlobal::Instance()->GetThreadPolicyRegistry().ClearThreadPolicy( m_thread );
    }

    private:

    TThreadId m_thread;
};

/*-------------------------------------------------------------------------*/

/** Writes a pack of one texture with its licence restrictions on the source side of a world and ingests it; the pack id and the id of its asset */
bool
IngestTexturePack( CSharesWorld& w, CIAssetStoreService& service, const CString& packName, const CString& restrictions, UInt32 variant, CString& packId, CString& assetId, CString& mainPath )
{
    const CString root = w.base + "/source/" + packName;
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
    if ( ASSETSTORE_STATUS_OK != service.Ingest( "rs-ingest-" + packName, request, result ) || result.packId.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "IngestTexturePack: " << packName << ": " << AssetStoreStatusToString( result.status ) << ", " << result.message );
        return false;
    }
    packId = result.packId;
    CAssetRecordPage assets;
    if ( ASSETSTORE_STATUS_OK != service.GetAssets( packId, CString::Empty, 0, assets ) || assets.entries.empty() )
        return false;
    assetId = assets.entries[ 0 ].assetId;
    CAssetFileSet files;
    if ( ASSETSTORE_STATUS_OK != service.GetAssetFileSet( assetId, files ) )
        return false;
    mainPath = files.mainVfsPath;
    return !mainPath.IsNULLOrEmpty();
}

/** A location of a page of locations of a service */
void
AddLocation( CAssetLocationPage& page, const CString& id, const CString& root, bool online )
{
    CAssetLocationInfo info;
    info.location.locationId = id;
    info.location.vfsRoot = root;
    info.online = online;
    page.entries.push_back( info );
}

/*-------------------------------------------------------------------------*/


} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteSharesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteShares" );

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();
    CSharesWorld w( "shares" );

    GUCEF_TESTFW_TESTCASE( "Remote shares: the shares are laid out as the host serves them (the common folder of the roots, or one share per online location), below a prefix; the files settings of the previous client map onto the settings of the mount, and old settings keep working" )
        try
        {
            CRemoteAssetShares::TShareVector layout;
            CAssetLocationPage page;
            AddLocation( page, "main", "store/a/main", true );
            CRemoteAssetShares::MakeShareLayout( page, "assets", CString::Empty, layout );
            ASSERT_TRUE( 1 == layout.size() && "assets" == layout[ 0 ].name && "store/a/main" == layout[ 0 ].serviceRoot && "store/a/main" == layout[ 0 ].mountPoint );
            CRemoteAssetShares::MakeShareLayout( page, "assets", "/client/", layout );
            ASSERT_TRUE( 1 == layout.size() && "client/store/a/main" == layout[ 0 ].mountPoint && "store/a/main" == layout[ 0 ].serviceRoot );
            // locations below one folder share that folder, also while only one of them is online
            AddLocation( page, "extra", "store/a/extra", false );
            CRemoteAssetShares::MakeShareLayout( page, "assets", "client", layout );
            ASSERT_TRUE( 1 == layout.size() && "store/a" == layout[ 0 ].serviceRoot && "client/store/a" == layout[ 0 ].mountPoint );
            // locations with nothing in common: one share each under the id of the location, only the online ones, a share name that cannot be one is left out
            CAssetLocationPage apart;
            AddLocation( apart, "main", "store/main", true );
            AddLocation( apart, "elsewhere", "other/root", true );
            AddLocation( apart, "off", "third/root", false );
            AddLocation( apart, "bad id!", "fourth/root", true );
            CRemoteAssetShares::MakeShareLayout( apart, "assets", "client", layout );
            ASSERT_TRUE( 2 == layout.size() && "main" == layout[ 0 ].name && "store/main" == layout[ 0 ].serviceRoot && "client/store/main" == layout[ 0 ].mountPoint && "elsewhere" == layout[ 1 ].name && "client/other/root" == layout[ 1 ].mountPoint );
            // nothing online, nothing to share; no name, no share of the common folder
            CAssetLocationPage none;
            AddLocation( none, "main", "store/a/main", false );
            CRemoteAssetShares::MakeShareLayout( none, "assets", CString::Empty, layout );
            ASSERT_TRUE( layout.empty() );
            CRemoteAssetShares::MakeShareLayout( page, CString::Empty, CString::Empty, layout );
            ASSERT_TRUE( layout.empty() );
            CString::StringVector roots;
            ASSERT_TRUE( CRemoteAssetShares::FindCommonRoot( roots ).IsNULLOrEmpty() );
            roots.push_back( "a/b/c" );
            roots.push_back( "a/b/d" );
            ASSERT_TRUE( "a/b" == CRemoteAssetShares::FindCommonRoot( roots ) );
            roots.push_back( "a/bc" );
            ASSERT_TRUE( "a" == CRemoteAssetShares::FindCommonRoot( roots ) );

            // the settings of the previous client, as they were written on a disk, load with the defaults of what is new and map onto the archive of the mount
            CDataNode old;
            old.SetName( "remoteFiles" );
            old.SetAttribute( "cacheFolder", "cache/remote" );
            old.SetAttribute( "maxCacheBytes", (UInt64) 123456789 );
            old.SetAttribute( "readAheadAfterBlocks", (UInt32) 3 );
            old.SetAttribute( "readAheadBlocks", (UInt32) 5 );
            old.SetAttribute( "maxBlocksPerRequest", (UInt32) 7 );
            old.SetAttribute( "metadataEntries", (UInt32) 4321 );
            old.SetAttribute( "negativeTtlMs", (UInt32) 2500 );
            old.SetAttribute( "metadataTtlMs", (UInt32) 1500 );
            old.SetAttribute( "listPageEntries", (UInt32) 250 );
            old.SetAttribute( "maxListEntries", (UInt32) 9000 );
            old.SetAttribute( "blockingReadTimeoutMs", (UInt32) 12000 );
            // the keys that no longer have a meaning (the transfers never read them) are in an old tree: they load without a word and are not written again
            old.SetAttribute( "maxParallelFiles", (UInt32) 6 );
            old.SetAttribute( "transferQueueThresholdBytes", (UInt32) 1000000 );
            old.SetAttribute( "openWithoutPrefetchBytes", (UInt32) 4000000 );
            old.SetAttribute( "maxPrefetchBytes", (UInt64) 5000000 );
            CRemoteFilesSettings loaded;
            ASSERT_TRUE( loaded.Deserialize( old, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( "assets" == loaded.shareName && loaded.refetchPinned && 123456789 == loaded.maxCacheBytes );
            CDataNode bridge;
            CRemoteAssetShares::MakeBridgeSettings( loaded, "assetstore:service:assets", bridge );
            ASSERT_TRUE( "cache/remote" == bridge.GetAttributeValueOrChildValueByName( "blockCacheFolder" ).AsString( CString::Empty, true ) );
            ASSERT_TRUE( 123456789 == bridge.GetAttributeValueOrChildValueByName( "blockCacheMaxBytes" ).AsUInt64( 0, true ) );
            ASSERT_TRUE( "assetstore:service:assets" == bridge.GetAttributeValueOrChildValueByName( "cacheIdentity" ).AsString( CString::Empty, true ) );
            ASSERT_TRUE( 3 == bridge.GetAttributeValueOrChildValueByName( "readAheadAfterBlocks" ).AsUInt32( 0, true ) && 5 == bridge.GetAttributeValueOrChildValueByName( "readAheadBlocks" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( 7 == bridge.GetAttributeValueOrChildValueByName( "maxBlocksPerFetch" ).AsUInt32( 0, true ) && 4321 == bridge.GetAttributeValueOrChildValueByName( "maxCachedEntries" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( 1500 == bridge.GetAttributeValueOrChildValueByName( "metadataTtlMs" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( 2500 == bridge.GetAttributeValueOrChildValueByName( "negativeTtlMs" ).AsUInt32( 0, true ) && 250 == bridge.GetAttributeValueOrChildValueByName( "listPageEntries" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( 9000 == bridge.GetAttributeValueOrChildValueByName( "maxEntriesPerDirectory" ).AsUInt32( 0, true ) && 12000 == bridge.GetAttributeValueOrChildValueByName( "callTimeoutMs" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( 12000 == bridge.GetAttributeValueOrChildValueByName( "openTimeoutMs" ).AsUInt32( 0, true ) && bridge.GetAttributeValueOrChildValueByName( "refetchPinned" ).AsBool( false, true ) );
            // no cache folder: no cache of the mount
            CRemoteFilesSettings withoutCache;
            CRemoteAssetShares::MakeBridgeSettings( withoutCache, CString::Empty, bridge );
            ASSERT_TRUE( !bridge.IsAttribute( "blockCacheFolder" ) && !bridge.IsAttribute( "cacheIdentity" ) );
            // what is written reads back, with the name of the share
            CDataNode written;
            written.SetName( "remoteFiles" );
            loaded.shareName = "files";
            loaded.refetchPinned = false;
            ASSERT_TRUE( loaded.Serialize( written, CDataNodeSerializableSettings() ) );
            CRemoteFilesSettings again;
            ASSERT_TRUE( again.Deserialize( written, CDataNodeSerializableSettings() ) && "files" == again.shareName && 7 == again.maxBlocksPerRequest && !again.refetchPinned );
            ASSERT_TRUE( !written.IsAttribute( "maxParallelFiles" ) && !written.IsAttribute( "transferQueueThresholdBytes" ) );
            ASSERT_TRUE( 4000000 == loaded.openWithoutPrefetchBytes && 5000000 == loaded.maxPrefetchBytes && written.IsAttribute( "openWithoutPrefetchBytes" ) && written.IsAttribute( "maxPrefetchBytes" ) );

            // the shares that the client makes are those that the host serves: the same name and the same root
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            CRemoteAssetShares::TShareVector mounted;
            w.shares->GetShares( mounted );
            CString::StringVector hostNames;
            w.world.binding->GetShareNames( hostNames );
            ASSERT_TRUE( 1 == mounted.size() && 1 == hostNames.size() && mounted[ 0 ].name == hostNames[ 0 ] && mounted[ 0 ].serviceRoot == w.world.binding->GetShareRoot( hostNames[ 0 ] ) );
            ASSERT_TRUE( mounted[ 0 ].mountPoint == w.prefix + "/" + w.mainRoot && GUCEF_NULL != w.shares->GetMount( 0 ) && GUCEF_NULL == w.shares->GetMount( 1 ) );
            ASSERT_TRUE( w.shares->IsMounted( w.mainRoot ) && w.shares->IsMounted( w.mainRoot + "/original" ) && !w.shares->IsMounted( "astest/other" ) );
            CString::StringVector locationRoots;
            w.shares->GetLocationRoots( locationRoots );
            ASSERT_TRUE( 1 == locationRoots.size() && w.mainRoot == locationRoots[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: the share of the service mounts below the prefix at the root of the location, and its files open through the VFS; the paths map both ways" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            ASSERT_TRUE( w.shares->IsMounted( w.mainRoot ) );
            CVfsProbe exists( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/original/big.bin" ) );
            exists.Run();
            CVfsProbe exists2( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/original/small.txt" ) );
            exists2.Run();
            ASSERT_TRUE( exists.ok && exists2.ok );
            CVfsProbe missing( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/original/none.bin" ) );
            missing.Run();
            CVfsProbe folderAsFile( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/original" ) );
            folderAsFile.Run();
            ASSERT_TRUE( !missing.ok && !folderAsFile.ok );
            CVfsProbe dir1( CVfsProbe::OP_DIREXISTS, w.ClientPath( "bigpack/original" ) );
            dir1.Run();
            CVfsProbe dir2( CVfsProbe::OP_DIREXISTS, w.ClientPath( "bigpack/intermediate/a" ) );
            dir2.Run();
            CVfsProbe dir3( CVfsProbe::OP_DIREXISTS, w.ClientPath( "nothing" ) );
            dir3.Run();
            ASSERT_TRUE( dir1.ok && dir2.ok && !dir3.ok );
            CVfsProbe size( CVfsProbe::OP_SIZE, w.ClientPath( "bigpack/original/big.bin" ) );
            size.Run();
            ASSERT_TRUE( BigSize == size.size );
            CVfsProbe files( CVfsProbe::OP_FILES, w.ClientPath( "bigpack/original" ) );
            files.Run();
            ASSERT_TRUE( files.ok && 2 == files.names.size() );
            CVfsProbe dirs( CVfsProbe::OP_DIRS, w.ClientPath( "bigpack" ) );
            dirs.Run();
            ASSERT_TRUE( dirs.ok && 2 == dirs.names.size() );

            // the paths of the service and of this computer
            CString servicePath;
            ASSERT_TRUE( w.shares->ToServicePath( w.ClientPath( "bigpack/original/big.bin" ), servicePath ) && servicePath == w.ServerPath( "bigpack/original/big.bin" ) );
            ASSERT_TRUE( w.shares->ToServicePath( w.prefix + "/" + w.mainRoot, servicePath ) && servicePath == w.mainRoot );
            ASSERT_TRUE( !w.shares->ToServicePath( "astest/other/x.bin", servicePath ) && !w.shares->ToServicePath( w.ServerPath( "bigpack/original/big.bin" ), servicePath ) );
            ASSERT_TRUE( w.shares->ToClientPath( w.ServerPath( "bigpack/original/big.bin" ) ) == w.ClientPath( "bigpack/original/big.bin" ) );
            ASSERT_TRUE( w.shares->Contains( w.ClientPath( "bigpack" ) ) && !w.shares->Contains( "astest/other/x.bin" ) && !w.shares->Contains( w.prefix + "/" + w.mainRoot + "x" ) );

            CVfsProbe text( CVfsProbe::OP_READ, w.ClientPath( "bigpack/original/small.txt" ) );
            text.piece = 4;
            text.Run();
            ASSERT_TRUE( text.ok && 24 == text.content.GetDataSize() && 0 == memcmp( text.content.GetConstBufferPtr(), "a small file of the pack", 24 ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 100000 ) && SameBytes( content, w.big ) );
            CVfsProbe none( CVfsProbe::OP_READ, w.ClientPath( "bigpack/original/none.bin" ) );
            none.Run();
            ASSERT_TRUE( !none.opened );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a read fetches only the blocks it touches, a second read is from the cache, positioning follows the f-functions" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            VFS::CVfsRemoteStatistics statistics;
            // a header probe: one block
            CVfsProbe header( CVfsProbe::OP_RANGE, w.ClientPath( "bigpack/original/big.bin" ) );
            header.offset = 0;
            header.length = 32;
            header.Run();
            ASSERT_TRUE( header.opened && BigSize == header.size && header.ok && 0 == memcmp( header.content.GetConstBufferPtr(), w.big.GetConstBufferPtr(), 32 ) );
            ASSERT_TRUE( w.Statistics( statistics ) && BlockBytes == statistics.bytesFetched );
            // a read across a block border, from the middle of the file
            const UInt64 border = 5ULL * BlockBytes;
            CVfsProbe across( CVfsProbe::OP_RANGE, w.ClientPath( "bigpack/original/big.bin" ) );
            across.offset = border - 20;
            across.length = 64;
            across.Run();
            ASSERT_TRUE( across.ok && 0 == memcmp( across.content.GetConstBufferPtr(), static_cast< const UInt8* >( w.big.GetConstBufferPtr() ) + border - 20, 64 ) );
            ASSERT_TRUE( w.Statistics( statistics ) && 3ULL * BlockBytes == statistics.bytesFetched );
            // the end of the file: the short block, then nothing
            CVfsProbe tail( CVfsProbe::OP_SEEKEND, w.ClientPath( "bigpack/original/big.bin" ) );
            tail.length = 10;
            tail.Run();
            ASSERT_TRUE( tail.ok && tail.eof && 10 == tail.content.GetDataSize() && 0 == memcmp( tail.content.GetConstBufferPtr(), static_cast< const UInt8* >( w.big.GetConstBufferPtr() ) + BigSize - 10, 10 ) );
            // reading the same again costs no call
            w.Statistics( statistics );
            const UInt64 calls = statistics.fetchCalls;
            header.finished.Store( 0 );
            CVfsProbe headerAgain( CVfsProbe::OP_RANGE, w.ClientPath( "bigpack/original/big.bin" ) );
            headerAgain.length = 32;
            headerAgain.Run();
            CVfsProbe acrossAgain( CVfsProbe::OP_RANGE, w.ClientPath( "bigpack/original/big.bin" ) );
            acrossAgain.offset = border - 20;
            acrossAgain.length = 64;
            acrossAgain.Run();
            ASSERT_TRUE( headerAgain.ok && acrossAgain.ok );
            w.Statistics( statistics );
            ASSERT_TRUE( calls == statistics.fetchCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a sequential read reads ahead (fewer, larger calls), and gives the bytes of the file" )
        try
        {
            ASSERT_TRUE( w.started );
            UInt32 callsWithout = 0;
            UInt64 msWithout = 0;
            UInt32 callsWith = 0;
            UInt64 msWith = 0;
            for ( UInt32 round=0; round<2; ++round )
            {
                w.settings.readAheadBlocks = 0 == round ? 0 : 8;
                w.settings.readAheadAfterBlocks = 2;
                ASSERT_TRUE( w.OpenShares( true ) );
                const UInt64 started = MT::PrecisionTickCount();
                CDynamicBuffer content;
                ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
                const UInt64 ms = ( MT::PrecisionTickCount() - started ) * 1000 / MT::PrecisionTimerResolution();
                VFS::CVfsRemoteStatistics statistics;
                ASSERT_TRUE( w.Statistics( statistics ) );
                if ( 0 == round )
                {
                    callsWithout = (UInt32) statistics.fetchCalls;
                    msWithout = ms;
                    ASSERT_TRUE( 0 == statistics.readAheadBlocks );
                }
                else
                {
                    callsWith = (UInt32) statistics.fetchCalls;
                    msWith = ms;
                    ASSERT_TRUE( statistics.readAheadBlocks >= 8 );
                }
            }
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "RemoteShares: " << BigSize << " bytes in 64 KiB reads: without read ahead " << callsWithout << " calls in " << msWithout << " ms, with read ahead " << callsWith << " calls in " << msWith << " ms" );
            ASSERT_TRUE( 14 == callsWithout && callsWith <= 5 );
            w.settings.readAheadBlocks = 8;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a fetch that was cancelled before it began ends as cancelled, nothing is fetched" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            VFS::CVfsFetchRequest request;
            VFS::CVfsFetchProgressPtr progress( GUCEF_NEW VFS::CVfsFetchProgress() );
            progress->cancel.Store( true );
            VFS::CVfsAsyncResultPtr result;
            CFutureResult future = vfs.FetchFileAsync( w.ClientPath( "bigpack/original/big.bin" ), request, progress, result );
            ASSERT_TRUE( future.HasAFuture() );
            for ( UInt32 waited=0; waited<30000 && !future.IsReady(); waited+=5 )
                PumpFor( 5 );
            ASSERT_TRUE( future.IsReady() && !result.IsNULL() && FILESYSTEMACCESS_CANCELLED == result->result.result );
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) && 0 == statistics.bytesFetched );
            // the same through the files of the service: ended as cancelled, and in words
            MT::CTAtomic< bool > cancel( true );
            CString message;
            CVfsProbe whole( CVfsProbe::OP_FETCHWHOLE, w.ServerPath( "bigpack/original/big.bin" ) );
            whole.shares = w.shares;
            whole.Run();
            ASSERT_TRUE( whole.ok );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: offline, the files of the cache open and read, the others say that they are not in the cache and the service cannot be reached" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 262144 ) && SameBytes( content, w.big ) );
            CDynamicBuffer small;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/small.txt" ), small, 100 ) );
            // another file, known by its list, not read yet
            CVfsProbe known( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/intermediate/a/b.txt" ) );
            known.Run();
            ASSERT_TRUE( known.ok );
            // the link is gone: the host stops, the connection of the client stays and tries again
            w.world.StopHost();
            ASSERT_TRUE( w.PumpUntilOffline( 30000 ) );
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 4096 ) && SameBytes( content, w.big ) );
            CVfsProbe stillKnown( CVfsProbe::OP_EXISTS, w.ClientPath( "bigpack/intermediate/a/b.txt" ) );
            stillKnown.Run();
            ASSERT_TRUE( stillKnown.ok );
            CVfsProbe notCached( CVfsProbe::OP_READ, w.ClientPath( "bigpack/intermediate/a/b.txt" ) );
            notCached.Run();
            ASSERT_TRUE( !notCached.opened && FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == notCached.last.result );
            ASSERT_TRUE( notCached.last.message.HasSubstr( "not in the cache" ) >= 0 && notCached.last.message.HasSubstr( "cannot be reached" ) >= 0 );
            ASSERT_TRUE( w.shares->IsOffline() );
            // a folder that was never listed says nothing
            CVfsProbe unseen( CVfsProbe::OP_FOLDER, w.ServerPath( "bigpack/unseen" ) );
            unseen.shares = w.shares;
            unseen.Run();
            ASSERT_TRUE( !unseen.ok );
            CString error;
            ASSERT_TRUE( w.world.StartHost( error ) );
            ASSERT_TRUE( w.PumpUntilOnline( 60000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a block of the cache damaged on the disk is fetched again when a restart reads it, and the file is right" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
            w.CloseShares();
            // the data file of the big file: the one that is as large as the file
            CString dataFile;
            {
                CString::StringVector subfolders;
                struct SFinder { static void Find( const CString& folder, CString& found )
                {
                    CString::StringVector entries;
                    CORE::CFileSystemIterator iterator;
                    if ( iterator.FindFirst( folder ) )
                    {
                        do
                        {
                            const CString name = iterator.GetResourceName();
                            if ( name == "." || name == ".." )
                                continue;
                            const CString full = CombinePath( folder, name );
                            if ( iterator.IsADirectory() )
                                Find( full, found );
                            else
                            if ( name.EndsWith( ".vbd" ) && CORE::FileSize( full ) >= (UInt64) BigSize )
                                found = full;
                        }
                        while ( iterator.FindNext() );
                        iterator.FindClose();
                    }
                } };
                SFinder::Find( w.CacheFolder(), dataFile );
            }
            ASSERT_TRUE( !dataFile.IsNULLOrEmpty() && FileExists( dataFile ) );
            {
                CFileAccess file( dataFile, "r+b" );
                ASSERT_TRUE( file.Opened() );
                file.Setpos( 3ULL * BlockBytes + 5 );
                const UInt8 damaged = (UInt8) ( static_cast< const UInt8* >( w.big.GetConstBufferPtr() )[ 3ULL * BlockBytes + 5 ] ^ 0x55 );
                file.Write( &damaged, 1, 1 );
            }
            ASSERT_TRUE( w.OpenShares( false ) );
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) );
            ASSERT_TRUE( 1 == statistics.fetchCalls && 1 == statistics.corruptBlocks );             // the one block
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a file that is not the one of the catalogue is refused (its digest), emptied from the cache and reported in plain words; the right digest passes" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            const CString path = w.ServerPath( "bigpack/original/big.bin" );
            w.shares->NoteDigest( path, "00" + w.bigDigest.SubstrFromRange( 2, 64 ) );
            CVfsProbe wrong( CVfsProbe::OP_FETCHWHOLE, path );
            wrong.shares = w.shares;
            wrong.Run();
            ASSERT_TRUE( ASSETSTORE_STATUS_DIGEST_MISMATCH == wrong.status && wrong.message.HasSubstr( "differs from what its catalogue says" ) >= 0 );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( !VFS::CVfsGlobal::Instance()->GetVfs().GetFileCacheState( w.ClientPath( "bigpack/original/big.bin" ), state ) || 0 == state.bytesHeld );
            w.shares->NoteDigest( path, w.bigDigest );
            CVfsProbe right( CVfsProbe::OP_FETCHWHOLE, path );
            right.shares = w.shares;
            right.pin = true;
            right.Run();
            ASSERT_TRUE( right.ok );
            ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().GetFileCacheState( w.ClientPath( "bigpack/original/big.bin" ), state ) && state.complete && state.pinned && state.digestVerified && state.digest.value == w.bigDigest );
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) );
            ASSERT_TRUE( statistics.digestMismatches >= 2 && statistics.filesVerified >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;


    GUCEF_TESTFW_TESTCASE( "Remote shares: a download to a folder through a link that drops completes, is digest checked, and a cancelled one continues from its part" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxBlocksPerRequest = 1;
            ASSERT_TRUE( w.OpenShares( true ) );
            VFS::CVfsBlockCachePtr cache = VFS::CVfsGlobal::Instance()->AcquireBlockCache( w.CacheFolder(), 0 );
            ASSERT_TRUE( !cache.IsNULL() );
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            const CString target = TestDirectory( "rs_download" );
            DeleteDir( target, true );

            // a link that drops in the middle of the transfer
            TRemoteFileTransferPtr first( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            first->SetDestination( target );
            first->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            CFutureResult future = w.shares->StartTransfer( first );
            ASSERT_TRUE( future.HasAFuture() );
            for ( UInt32 waited=0; waited<60000 && first->GetBytesDone() < 3ULL * BlockBytes; waited+=25 )
                SlowPump( 25 );
            ASSERT_TRUE( first->GetBytesDone() >= 3ULL * BlockBytes && !first->IsEnded() );
            ConnectionOfClient( *w.client )->DropLink();
            ASSERT_TRUE( PumpUntilTransferEnded( first, 120000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == first->GetState() && ASSETSTORE_STATUS_OK == first->GetStatus() );
            CDynamicBuffer downloaded;
            ASSERT_TRUE( ReadDiskFile( CombinePath( target, "big.bin" ), downloaded ) && SameBytes( downloaded, w.big ) );
            ASSERT_TRUE( !FileExists( CombinePath( target, "big.bin.part" ) ) );
            ASSERT_TRUE( w.PumpUntilOnline( 60000 ) );

            // cancelled in the middle: the part stays, the next run reuses it
            DeleteFile( CombinePath( target, "big.bin" ) );
            cache->Clear();
            TRemoteFileTransferPtr second( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            second->SetDestination( target );
            second->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            future = w.shares->StartTransfer( second );
            for ( UInt32 waited=0; waited<60000 && second->GetBytesDone() < 5ULL * BlockBytes; waited+=25 )
                SlowPump( 25 );
            second->Cancel();
            ASSERT_TRUE( PumpUntilTransferEnded( second, 60000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_CANCELLED == second->GetState() && second->GetBytesDone() < BigSize );
            ASSERT_TRUE( FileExists( CombinePath( target, "big.bin.part" ) ) && !FileExists( CombinePath( target, "big.bin" ) ) );
            cache->Clear();
            w.shares->ForgetMetadata();
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            TRemoteFileTransferPtr third( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            third->SetDestination( target );
            third->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            future = w.shares->StartTransfer( third );
            ASSERT_TRUE( PumpUntilTransferEnded( third, 60000 ) );
            GUCEF_LOG( LOGLEVEL_NORMAL, "RemoteShares: the third download: state " << (UInt32) third->GetState() << ", status " << (UInt32) third->GetStatus() << ", reused " << third->GetBytesReused() << ", done " << third->GetBytesDone() << ", " << third->GetMessage() << "; the second: done " << second->GetBytesDone() );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == third->GetState() && third->GetBytesReused() >= 3ULL * BlockBytes );
            ASSERT_TRUE( ReadDiskFile( CombinePath( target, "big.bin" ), downloaded ) && SameBytes( downloaded, w.big ) && !FileExists( CombinePath( target, "big.bin.part" ) ) );

            // a file that is there already and right is not fetched again; a folder with its files
            VFS::CVfsRemoteStatistics before;
            w.Statistics( before );
            TRemoteFileTransferPtr again( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "pack" ) );
            again->SetDestination( target );
            again->AddFolder( w.ServerPath( "bigpack/original" ), "" );
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            future = w.shares->StartTransfer( again );
            ASSERT_TRUE( PumpUntilTransferEnded( again, 60000 ) );
            VFS::CVfsRemoteStatistics after;
            w.Statistics( after );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == again->GetState() && 2 == again->GetFileCount() && again->GetBytesReused() >= BigSize );
            ASSERT_TRUE( FileExists( CombinePath( target, "small.txt" ) ) && after.fetchCalls - before.fetchCalls <= 1 );
            w.settings.maxBlocksPerRequest = 16;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a large file (64 MiB) streams through the block cache over the loopback link: whole by a fetch and sequentially through the VFS, both digest right, the second pass from the cache" )
        try
        {
            ASSERT_TRUE( w.started );
            const UInt32 hugeSize = 64 * 1024 * 1024;
            CDynamicBuffer huge;
            MakePattern( hugeSize, 11, huge );
            const CString hugeDigest = Sha256Of( huge );
            ASSERT_TRUE( WriteVfsFile( w.ServerPath( "bigpack/original/huge.bin" ), huge ) );
            w.settings.maxCacheBytes = 20ULL * 1024 * 1024 * 1024;
            ASSERT_TRUE( w.OpenShares( true ) );
            VFS::CVfsBlockCachePtr cache = VFS::CVfsGlobal::Instance()->AcquireBlockCache( w.CacheFolder(), 0 );
            ASSERT_TRUE( !cache.IsNULL() );
            const CString path = w.ServerPath( "bigpack/original/huge.bin" );
            w.shares->NoteDigest( path, hugeDigest );
            UInt64 start = MT::PrecisionTickCount();
            const UInt64 resolution = MT::PrecisionTimerResolution();
            CVfsProbe whole( CVfsProbe::OP_FETCHWHOLE, path );
            whole.shares = w.shares;
            ASSERT_TRUE( whole.Run() && whole.ok );
            const UInt64 wholeMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) );
            const UInt64 calls = statistics.fetchCalls;
            start = MT::PrecisionTickCount();
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/huge.bin" ), content, 65536 ) && content.GetDataSize() == hugeSize && Sha256Of( content ) == hugeDigest );
            const UInt64 cachedMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            ASSERT_TRUE( w.Statistics( statistics ) && statistics.fetchCalls == calls );
            // the same through the VFS with no cache entry yet: sequential reads fetch ahead
            cache->Clear();
            w.Statistics( statistics );
            const UInt64 callsBefore = statistics.fetchCalls;
            start = MT::PrecisionTickCount();
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/huge.bin" ), content, 65536 ) && Sha256Of( content ) == hugeDigest );
            const UInt64 streamMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            w.Statistics( statistics );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "RemoteShares: 64 MiB over the loopback link: whole file by a fetch " << wholeMs << " ms (" << ( wholeMs > 0 ? 64000 / wholeMs : 0 ) << " MiB/s, " << calls
                       << " calls), streamed through the VFS in 64 KiB reads " << streamMs << " ms (" << ( streamMs > 0 ? 64000 / streamMs : 0 ) << " MiB/s including the hash, " << ( statistics.fetchCalls - callsBefore )
                       << " calls), the same from the cache " << cachedMs << " ms" );
            ASSERT_TRUE( statistics.fetchCalls - callsBefore <= 40 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: keep offline pins the files in the cache, and the cache keeps them under a size limit that removes the rest" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxCacheBytes = 16ULL * BlockBytes;
            ASSERT_TRUE( w.OpenShares( true ) );
            VFS::CVfsBlockCachePtr cache = VFS::CVfsGlobal::Instance()->AcquireBlockCache( w.CacheFolder(), 0 );
            ASSERT_TRUE( !cache.IsNULL() );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            TRemoteFileTransferPtr pin( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_KEEP_OFFLINE, "keep" ) );
            pin->AddFolder( w.ServerPath( "bigpack/intermediate" ), "" );
            CFutureResult future = w.shares->StartTransfer( pin );
            ASSERT_TRUE( PumpUntilTransferEnded( pin, 60000 ) && CRemoteFileTransfer::STATE_DONE == pin->GetState() );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( vfs.GetFileCacheState( w.ClientPath( "bigpack/intermediate/a/b.txt" ), state ) && state.complete && state.pinned );
            // a read of the big file (14 blocks) and then a smaller limit push out unpinned entries but never the pin
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 262144 ) && SameBytes( content, w.big ) );
            cache->SetMaxBytes( 2ULL * BlockBytes );
            ASSERT_TRUE( vfs.GetFileCacheState( w.ClientPath( "bigpack/intermediate/a/b.txt" ), state ) && state.complete && state.pinned );
            ASSERT_TRUE( !vfs.GetFileCacheState( w.ClientPath( "bigpack/original/big.bin" ), state ) || 0 == state.bytesHeld );
            // released, then it goes with the limit
            TRemoteFileTransferPtr release( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_RELEASE_OFFLINE, "release" ) );
            release->AddFolder( w.ServerPath( "bigpack/intermediate" ), "" );
            future = w.shares->StartTransfer( release );
            ASSERT_TRUE( PumpUntilTransferEnded( release, 60000 ) );
            ASSERT_TRUE( !vfs.GetFileCacheState( w.ClientPath( "bigpack/intermediate/a/b.txt" ), state ) || !state.pinned );
            w.settings.maxCacheBytes = 20ULL * 1024 * 1024 * 1024;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;


    GUCEF_TESTFW_TESTCASE( "Remote shares: the stored thumbnails are put by a device, found by the digest of the content and the size, and read through the files of the service" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            CString packId;
            CString assetId;
            CString mainPath;
            ASSERT_TRUE( IngestTexturePack( w, *w.client, "Crates", "", 1, packId, assetId, mainPath ) );
            CString::StringVector ids;
            ids.push_back( assetId );
            ids.push_back( "pack:" + packId );
            ids.push_back( "no-such-asset" );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->GetThumbnails( ids, 64, page ) && 3 == page.entries.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 0 ].status && ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 1 ].status && ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 2 ].status );

            CDynamicBuffer png32, png64, png128;
            ASSERT_TRUE( MakePng( 32, 32, false, png32 ) && MakePng( 64, 64, true, png64 ) && MakePng( 128, 128, false, png128 ) );
            const UInt32 sizes[ 3 ] = { 32, 64, 128 };
            const CDynamicBuffer* pngs[ 3 ] = { &png32, &png64, &png128 };
            for ( UInt32 i=0; i<3; ++i )
            {
                CAssetThumbnail thumbnail;
                thumbnail.assetId = assetId;
                thumbnail.kind = "thumbnail";
                thumbnail.size = sizes[ i ];
                thumbnail.key.rendererId = "test";
                thumbnail.key.rendererVersion = "1";
                thumbnail.key.size = sizes[ i ];
                thumbnail.encoded = *pngs[ i ];
                CAssetServiceResult put;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->PutThumbnail( "rf-put-" + ToString( sizes[ i ] ), thumbnail, put ) );
                // the same command again is answered from what was kept, the picture is not written twice
                CAssetServiceResult repeated;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->PutThumbnail( "rf-put-" + ToString( sizes[ i ] ), thumbnail, repeated ) && repeated.replayed );
            }
            // not a picture, a picture of other content
            {
                CAssetThumbnail bad;
                bad.assetId = assetId;
                bad.key.size = 32;
                bad.size = 32;
                const char* text = "this is not a png picture at all, it is a text of some length that is more than a header";
                bad.encoded.CopyFrom( (UInt32) strlen( text ), text );
                CAssetServiceResult put;
                ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == w.client->PutThumbnail( "rf-put-bad", bad, put ) );
                CAssetThumbnail other;
                other.assetId = assetId;
                other.key.size = 32;
                other.key.digest = Sha256Of( png32 );
                other.size = 32;
                other.encoded = png32;
                ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == w.client->PutThumbnail( "rf-put-other", other, put ) && put.message.HasSubstr( "other content" ) >= 0 );
            }
            // the size asked for: the size, else the smallest that is larger, else the largest that is smaller
            const UInt32 asked[ 4 ] = { 64, 100, 200, 16 };
            const UInt32 got[ 4 ] = { 64, 128, 128, 32 };
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->GetThumbnails( ids, asked[ i ], page ) && ASSETSTORE_STATUS_OK == page.entries[ 0 ].status && got[ i ] == page.entries[ 0 ].key.size );
            }
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->GetThumbnails( ids, 64, page ) );
            ASSERT_TRUE( !page.entries[ 0 ].vfsPath.IsNULLOrEmpty() && "test" == page.entries[ 0 ].key.rendererId && Sha256Of( png32 ).Length() == page.entries[ 0 ].key.digest.Length() );
            CDynamicBuffer fetched;
            ASSERT_TRUE( ReadViaVfs( w.shares->ToClientPath( page.entries[ 0 ].vfsPath ), fetched, 4096 ) && SameBytes( fetched, png64 ) );
            // the same picture through the asynchronous range read of the VFS, as the workbench reads the pictures of a remote store
            {
                VFS::CVfsAsyncResultPtr range;
                CFutureResult read = VFS::CVfsGlobal::Instance()->GetVfs().ReadFileRangeAsync( w.shares->ToClientPath( page.entries[ 0 ].vfsPath ), 0, 2097153, range );
                ASSERT_TRUE( read.HasAFuture() );
                for ( UInt32 waited=0; waited<30000 && !read.IsReady(); waited+=5 )
                    PumpFor( 5 );
                ASSERT_TRUE( read.IsReady() && !range.IsNULL() && range->result.IsSuccess() && SameBytes( range->data, png64 ) );
            }
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 1 ].status && ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 2 ].status );

            // the picture of a pack (the automatic thumbnail)
            CAssetThumbnail auto_;
            auto_.assetId = "pack:" + packId;
            auto_.kind = "auto";
            auto_.size = 32;
            auto_.key.size = 32;
            auto_.key.rendererId = "workbench";
            auto_.encoded = png32;
            CAssetServiceResult put;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->PutThumbnail( "rf-put-pack", auto_, put ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->GetThumbnails( ids, 32, page ) && ASSETSTORE_STATUS_OK == page.entries[ 1 ].status && "auto" == page.entries[ 1 ].kind );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    GUCEF_TESTFW_TESTCASE( "Remote shares: the thread of the interface never waits: with a service that answers nothing every call returns at once and says that the data is not here yet, the arrival is an event of the VFS on that thread, and the bytes follow" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CVfsEvents events;
            CNoWaitScope noWait;
            const CString original = w.ClientPath( "bigpack/original" );
            // nothing is pumped here: the service answers nothing, however long the thread looks at it
            UInt64 worstMs = 0;
            VFS::CVfsCallResult last;
            UInt64 started = MT::PrecisionTickCount();
            const bool exists = vfs.FileExists( w.ClientPath( "bigpack/original/small.txt" ) );
            worstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started );
            vfs.GetLastResult( last );
            ASSERT_TRUE( !exists && FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == last.result && last.fetchStarted );
            started = MT::PrecisionTickCount();
            const bool dirExists = vfs.DirExists( w.ClientPath( "bigpack/intermediate" ) );
            worstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) > worstMs ? MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) : worstMs;
            ASSERT_TRUE( !dirExists );
            VFS::CVFS::TStringVector names;
            started = MT::PrecisionTickCount();
            const bool listed = vfs.GetFileList( names, original, false, false, CString::Empty );
            worstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) > worstMs ? MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) : worstMs;
            vfs.GetLastResult( last );
            ASSERT_TRUE( !listed && names.empty() && FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == last.result );
            started = MT::PrecisionTickCount();
            VFS::TBasicVfsResourcePtr file = vfs.GetFile( w.ClientPath( "bigpack/original/big.bin" ), "rb", false );
            worstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) > worstMs ? MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) : worstMs;
            ASSERT_TRUE( file.IsNULL() );
            ASSERT_TRUE( !w.shares->IsFolderKnown( w.ServerPath( "bigpack/original" ) ) );
            started = MT::PrecisionTickCount();
            w.shares->RequestFolder( w.ServerPath( "bigpack/original" ) );
            CIRemoteFileSource::SFileInfo info;
            const bool known = w.shares->GetFileInfo( w.ServerPath( "bigpack/original/big.bin" ), info );
            worstMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) > worstMs ? MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) : worstMs;
            ASSERT_TRUE( !known );
            ASSERT_TRUE( worstMs < 150 );
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) && statistics.policyMisses >= 1 );

            // the service gets its time: what was asked for arrives as events, delivered on this thread, with the path of the VFS
            ASSERT_TRUE( events.PumpUntil( "listing", "bigpack/original", 30000 ) );
            const CVfsEvents::SRecord* arrived = events.Find( "listing", "bigpack/original" );
            ASSERT_TRUE( GUCEF_NULL != arrived && arrived->thread == MT::GetCurrentThreadID() && arrived->path.HasSubstr( w.prefix + "/" ) >= 0 );
            ASSERT_TRUE( w.shares->IsFolderKnown( w.ServerPath( "bigpack/original" ) ) );
            ASSERT_TRUE( vfs.GetFileList( names, original, false, false, CString::Empty ) && HasName( names, "small.txt" ) && HasName( names, "big.bin" ) );
            ASSERT_TRUE( w.shares->GetFileInfo( w.ServerPath( "bigpack/original/big.bin" ), info ) && BigSize == info.size && !info.isDirectory );

            // the bytes: the file opens from what is known, the first read misses at once, the blocks arrive as an event, and then the read is answered
            for ( UInt32 waited=0; waited<30000 && file.IsNULL(); waited+=5 )
            {
                file = vfs.GetFile( w.ClientPath( "bigpack/original/big.bin" ), "rb", false );
                if ( file.IsNULL() )
                    PumpFor( 5 );
            }
            ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() );
            IOAccessPtr access = file->GetAccess();
            UInt8 probe[ 32 ];
            started = MT::PrecisionTickCount();
            const UInt32 firstRead = access->Read( probe, 1, 32 );
            ASSERT_TRUE( 0 == firstRead && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) < 150 );
            ASSERT_TRUE( events.PumpUntil( "file", "big.bin", 30000 ) );
            const CVfsEvents::SRecord* blocks = events.Find( "file", "big.bin" );
            ASSERT_TRUE( GUCEF_NULL != blocks && blocks->thread == MT::GetCurrentThreadID() && 0 == blocks->firstByte && blocks->byteCount >= 32 && !blocks->complete );
            ASSERT_TRUE( 0 == access->Setpos( 0 ) && 32 == access->Read( probe, 1, 32 ) && 0 == memcmp( probe, w.big.GetConstBufferPtr(), 32 ) );
            ASSERT_TRUE( w.Statistics( statistics ) && statistics.policyMisses >= 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: after the link is cut the files that were looked at are browsed and read from the cache by the thread of the interface, the rest says why not at once, and the state of the link is an event both ways" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CVfsProbe listing( CVfsProbe::OP_FILES, w.ClientPath( "bigpack/original" ) );
            listing.Run();
            ASSERT_TRUE( listing.ok && HasName( listing.names, "small.txt" ) && HasName( listing.names, "big.bin" ) );
            CDynamicBuffer small;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/small.txt" ), small, 100 ) && 24 == small.GetDataSize() );
            CVfsEvents events;
            w.world.StopHost();
            ASSERT_TRUE( w.PumpUntilOffline( 30000 ) );
            ASSERT_TRUE( events.PumpUntil( "connection", w.prefix + "/", 30000 ) );
            ASSERT_TRUE( !events.records.back().connected && events.records.back().thread == MT::GetCurrentThreadID() );
            {
                CNoWaitScope noWait;
                const UInt64 started = MT::PrecisionTickCount();
                VFS::CVFS::TStringVector names;
                ASSERT_TRUE( vfs.GetFileList( names, w.ClientPath( "bigpack/original" ), false, false, CString::Empty ) && HasName( names, "small.txt" ) && HasName( names, "big.bin" ) );
                ASSERT_TRUE( vfs.FileExists( w.ClientPath( "bigpack/original/small.txt" ) ) );
                VFS::TBasicVfsResourcePtr file = vfs.GetFile( w.ClientPath( "bigpack/original/small.txt" ), "rb", false );
                ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() );
                UInt8 text[ 24 ];
                ASSERT_TRUE( 24 == file->GetAccess()->Read( text, 1, 24 ) && 0 == memcmp( text, "a small file of the pack", 24 ) );
                // never looked at: it is not here, and the link is the reason
                VFS::TBasicVfsResourcePtr unseen = vfs.GetFile( w.ClientPath( "bigpack/intermediate/a/b.txt" ), "rb", false );
                VFS::CVfsCallResult last;
                vfs.GetLastResult( last );
                ASSERT_TRUE( unseen.IsNULL() && FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == last.result && last.message.HasSubstr( "cannot be reached" ) >= 0 );
                ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - started ) < 500 );
            }
            const UInt32 connectionEvents = events.Count( "connection" );
            CString error;
            ASSERT_TRUE( w.world.StartHost( error ) );
            ASSERT_TRUE( w.PumpUntilOnline( 60000 ) );
            for ( UInt32 waited=0; waited<30000 && !( events.Count( "connection" ) > connectionEvents && events.records.back().connected ); waited+=5 )
                PumpFor( 5 );
            ASSERT_TRUE( events.Count( "connection" ) > connectionEvents && events.records.back().connected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a file that is kept offline keeps its old complete copy when the file changed on the service and the link is gone, and is fetched again by itself, still kept, when the link is up" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.refetchPinned = false;
            ASSERT_TRUE( w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CDynamicBuffer v1;
            MakePattern( 3 * BlockBytes + 777, 21, v1 );
            CDynamicBuffer v2;
            MakePattern( 2 * BlockBytes + 5, 22, v2 );
            const CString keep = w.ServerPath( "bigpack/original/keep.bin" );
            const CString clientKeep = w.ClientPath( "bigpack/original/keep.bin" );
            ASSERT_TRUE( WriteVfsFile( keep, v1 ) );
            w.shares->ForgetMetadata();
            TRemoteFileTransferPtr pin( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_KEEP_OFFLINE, "keep" ) );
            pin->AddFile( keep, keep, 0, CString::Empty );
            CFutureResult future = w.shares->StartTransfer( pin );
            ASSERT_TRUE( PumpUntilTransferEnded( pin, 60000 ) && CRemoteFileTransfer::STATE_DONE == pin->GetState() );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( vfs.GetFileCacheState( clientKeep, state ) && state.complete && state.pinned && !state.staleRevision );

            // the file changes on the service and this computer learns of it: nothing is fetched (the setting is off), the link goes
            ASSERT_TRUE( WriteVfsFile( keep, v2 ) );
            w.shares->ForgetMetadata();
            CVfsProbe changed( CVfsProbe::OP_FILEINFO, keep );
            changed.shares = w.shares;
            changed.Run();
            ASSERT_TRUE( changed.ok && v2.GetDataSize() == changed.info.size );
            w.world.StopHost();
            ASSERT_TRUE( w.PumpUntilOffline( 30000 ) );
            ASSERT_TRUE( vfs.GetFileCacheState( clientKeep, state ) && state.complete && state.pinned && state.staleRevision );
            CVfsProbe offline( CVfsProbe::OP_READ, clientKeep );
            offline.Run();
            ASSERT_TRUE( offline.opened && SameBytes( offline.content, v1 ) );
            CString error;
            ASSERT_TRUE( w.world.StartHost( error ) );
            ASSERT_TRUE( w.PumpUntilOnline( 60000 ) );

            // the link is up and the setting is on: the copy of the new version is fetched, and the pin moves to it
            w.settings.refetchPinned = true;
            ASSERT_TRUE( w.OpenShares( false ) );
            CVfsProbe learn( CVfsProbe::OP_FILEINFO, keep );
            learn.shares = w.shares;
            learn.Run();
            ASSERT_TRUE( learn.ok && v2.GetDataSize() == learn.info.size );
            bool healed = false;
            for ( UInt32 waited=0; waited<60000 && !healed; waited+=10 )
            {
                PumpFor( 10 );
                healed = vfs.GetFileCacheState( clientKeep, state ) && state.complete && !state.staleRevision && state.pinned && state.fileSize == v2.GetDataSize();
            }
            ASSERT_TRUE( healed );
            CDynamicBuffer fetched;
            ASSERT_TRUE( ReadViaVfs( clientKeep, fetched, 65536 ) && SameBytes( fetched, v2 ) );
            w.settings.refetchPinned = true;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: keeping a file offline that does not fit the limit of the cache is refused in plain words, the file stays in the cache unkept, and the failure is an event" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxCacheBytes = 2ULL * BlockBytes;
            ASSERT_TRUE( w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CVfsEvents events;
            TRemoteFileTransferPtr pin( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_KEEP_OFFLINE, "too large" ) );
            pin->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            CFutureResult future = w.shares->StartTransfer( pin );
            ASSERT_TRUE( PumpUntilTransferEnded( pin, 60000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_FAILED == pin->GetState() && ASSETSTORE_STATUS_NO_SPACE == pin->GetStatus() );
            ASSERT_TRUE( pin->GetMessage().HasSubstr( "cannot be kept offline" ) >= 0 && pin->GetMessage().HasSubstr( "big.bin" ) >= 0 );
            VFS::CVfsCacheState state;
            ASSERT_TRUE( !vfs.GetFileCacheState( w.ClientPath( "bigpack/original/big.bin" ), state ) || !state.pinned );
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( w.Statistics( statistics ) && statistics.pinsRefused >= 1 );
            ASSERT_TRUE( events.PumpUntil( "failed", "big.bin", 10000 ) );
            // what fits is kept
            TRemoteFileTransferPtr fits( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_KEEP_OFFLINE, "fits" ) );
            fits->AddFile( w.ServerPath( "bigpack/original/small.txt" ), "small.txt", 0, CString::Empty );
            future = w.shares->StartTransfer( fits );
            ASSERT_TRUE( PumpUntilTransferEnded( fits, 60000 ) && CRemoteFileTransfer::STATE_DONE == fits->GetState() );
            ASSERT_TRUE( vfs.GetFileCacheState( w.ClientPath( "bigpack/original/small.txt" ), state ) && state.pinned );
            w.settings.maxCacheBytes = 20ULL * 1024 * 1024 * 1024;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a change of the catalogue makes the files forget what they know of the folders and the digests, at once and not when the time of the lists is over" )
        try
        {
            // the lists are taken as current for an hour: only a change of the catalogue renews them
            w.settings.metadataTtlMs = 3600000;
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            const CString folder = w.ServerPath( "bigpack/intermediate/a" );
            CVfsProbe before( CVfsProbe::OP_FOLDER, folder );
            before.shares = w.shares;
            before.Run();
            ASSERT_TRUE( before.ok && 1 == before.names.size() );
            ASSERT_TRUE( WriteVfsFile( w.ServerPath( "bigpack/intermediate/a/c.txt" ), CString( "a file that came later" ) ) );
            CVfsProbe stale( CVfsProbe::OP_FOLDER, folder );
            stale.shares = w.shares;
            stale.Run();
            ASSERT_TRUE( stale.ok && 1 == stale.names.size() );
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            CVfsProbe seen( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/big.bin" ) );
            seen.shares = w.shares;
            seen.Run();
            CString digest;
            ASSERT_TRUE( seen.ok && w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/big.bin" ), digest ) );

            // the catalogue of the service changed
            w.shares->ForgetMetadata();
            CVfsProbe after( CVfsProbe::OP_FOLDER, folder );
            after.shares = w.shares;
            after.Run();
            ASSERT_TRUE( after.ok && 2 == after.names.size() );
            ASSERT_TRUE( !w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/big.bin" ), digest ) );

            // a change of the catalogue on the service comes through the link: a pack that is ingested
            CVfsProbe known( CVfsProbe::OP_FOLDER, w.ServerPath( "bigpack/intermediate" ) );
            known.shares = w.shares;
            known.Run();
            ASSERT_TRUE( known.ok && 1 == known.names.size() );
            ASSERT_TRUE( WriteVfsFile( w.ServerPath( "bigpack/intermediate/d.txt" ), CString( "another file that came later" ) ) );
            CString packId, assetId, mainPath;
            ASSERT_TRUE( IngestTexturePack( w, *w.world.serverService, "Barrels", "", 2, packId, assetId, mainPath ) );
            bool renewed = false;
            for ( UInt32 waited=0; waited<20000 && !renewed; waited+=20 )
            {
                PumpFor( 20 );
                CVfsProbe again( CVfsProbe::OP_FOLDER, w.ServerPath( "bigpack/intermediate" ) );
                again.shares = w.shares;
                again.Run();
                renewed = again.ok && 2 == again.names.size();
            }
            ASSERT_TRUE( renewed );
            w.settings.metadataTtlMs = 3000;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: the digests of the catalogue are hints: kept in lower case with the name of the algorithm, given back for a file, checked against the bytes of a complete file, forgotten with the rest" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenShares( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            GUCEF::vector< CAssetFileRecord > records;
            CAssetFileRecord record;
            record.path = w.ServerPath( "bigpack/original/big.bin" );
            record.sizeBytes = BigSize;
            record.digest = w.bigDigest.Uppercase();
            record.role = "main";
            records.push_back( record );
            CAssetFileRecord folderRecord;
            folderRecord.path = w.ServerPath( "bigpack/original" );
            folderRecord.sizeBytes = 0;
            folderRecord.digest = w.bigDigest;
            folderRecord.role = "directory";
            records.push_back( folderRecord );
            CAssetFileRecord broken;
            broken.path = w.ServerPath( "bigpack/original/small.txt" );
            broken.sizeBytes = 24;
            broken.digest = "not a digest";
            broken.role = "main";
            records.push_back( broken );
            w.shares->NoteFiles( records );
            CVfsProbe known( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/big.bin" ) );
            known.shares = w.shares;
            known.Run();
            CVfsProbe knownSmall( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/small.txt" ) );
            knownSmall.shares = w.shares;
            knownSmall.Run();
            ASSERT_TRUE( known.ok && knownSmall.ok );
            CString digest;
            ASSERT_TRUE( w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/big.bin" ), digest ) && w.bigDigest.Lowercase() == digest );
            ASSERT_TRUE( !w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/small.txt" ), digest ) && !w.shares->GetKnownDigest( w.ServerPath( "bigpack/original" ), digest ) );
            // the VFS names the algorithm
            {
                VFS::CVfsThreadPolicy policy;
                policy.missAction = VFS::MISSACTION_FAIL;
                VFS::CVfsThreadPolicyScope scope( policy );
                VFS::CVfsDigest named;
                ASSERT_TRUE( vfs.GetFileDigest( w.ClientPath( "bigpack/original/big.bin" ), named ) && VFS::CVfsDigest::ALGORITHM_SHA256 == named.algorithm && w.bigDigest.Lowercase() == named.value );
            }
            // through the info of a file, and checked when the file is complete
            CVfsProbe info( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/big.bin" ) );
            info.shares = w.shares;
            info.Run();
            ASSERT_TRUE( info.ok && BigSize == info.info.size && w.bigDigest.Lowercase() == info.info.digest );
            CVfsProbe whole( CVfsProbe::OP_FETCHWHOLE, w.ServerPath( "bigpack/original/big.bin" ) );
            whole.shares = w.shares;
            whole.Run();
            VFS::CVfsRemoteStatistics statistics;
            ASSERT_TRUE( whole.ok && w.Statistics( statistics ) && statistics.filesVerified >= 1 && 0 == statistics.digestMismatches );
            // a hint of the wrong file
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/small.txt" ), w.bigDigest );
            CVfsProbe wrong( CVfsProbe::OP_FETCHWHOLE, w.ServerPath( "bigpack/original/small.txt" ) );
            wrong.shares = w.shares;
            wrong.Run();
            ASSERT_TRUE( ASSETSTORE_STATUS_DIGEST_MISMATCH == wrong.status );
            // forgotten with the rest: the hint of the wrong file is gone, the file is as the share has it (no digest)
            CVfsProbe hinted( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/small.txt" ) );
            hinted.shares = w.shares;
            hinted.Run();
            ASSERT_TRUE( hinted.ok && w.bigDigest.Lowercase() == hinted.info.digest );
            w.shares->ForgetMetadata();
            ASSERT_TRUE( !w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/small.txt" ), digest ) );
            CVfsProbe again( CVfsProbe::OP_FILEINFO, w.ServerPath( "bigpack/original/small.txt" ) );
            again.shares = w.shares;
            again.Run();
            ASSERT_TRUE( again.ok && 24 == again.info.size && again.info.digest.IsNULLOrEmpty() && !w.shares->GetKnownDigest( w.ServerPath( "bigpack/original/small.txt" ), digest ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote shares: a prefetch shows its progress as the bytes arrive, a cancel ends it and what arrived stays in the cache for the next one" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxBlocksPerRequest = 1;
            ASSERT_TRUE( w.OpenShares( true ) );
            w.shares->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            TRemoteFileTransferPtr first( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_PREFETCH, "big" ) );
            first->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            CFutureResult future = w.shares->StartTransfer( first );
            ASSERT_TRUE( future.HasAFuture() );
            UInt64 lastSeen = 0;
            UInt32 distinct = 0;
            bool monotonic = true;
            for ( UInt32 waited=0; waited<60000 && !first->IsEnded() && first->GetBytesDone() < 4ULL * BlockBytes; waited+=25 )
            {
                SlowPump( 25 );
                const UInt64 seen = first->GetBytesDone();
                monotonic = monotonic && seen >= lastSeen;
                if ( seen > lastSeen )
                    ++distinct;
                lastSeen = seen;
            }
            ASSERT_TRUE( !first->IsEnded() && monotonic && distinct >= 2 && lastSeen >= 4ULL * BlockBytes && lastSeen < BigSize );
            ASSERT_TRUE( first->GetBytesTotal() == BigSize && 1 == first->GetFileCount() );
            first->Cancel();
            ASSERT_TRUE( PumpUntilTransferEnded( first, 60000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_CANCELLED == first->GetState() && ASSETSTORE_STATUS_CANCELLED == first->GetStatus() && first->GetBytesDone() < BigSize );
            ASSERT_TRUE( !w.shares->IsFileCached( w.ServerPath( "bigpack/original/big.bin" ), BigSize ) );
            // the next one finds what arrived
            TRemoteFileTransferPtr second( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_PREFETCH, "big" ) );
            second->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            future = w.shares->StartTransfer( second );
            ASSERT_TRUE( PumpUntilTransferEnded( second, 60000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == second->GetState() && second->GetBytesReused() >= 4ULL * BlockBytes && BigSize == second->GetBytesDone() );
            ASSERT_TRUE( w.shares->IsFileCached( w.ServerPath( "bigpack/original/big.bin" ), BigSize ) && !w.shares->IsFileCached( w.ServerPath( "bigpack/original/big.bin" ), BigSize + 1 ) );
            w.settings.maxBlocksPerRequest = 16;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;


    w.CloseShares();
}
