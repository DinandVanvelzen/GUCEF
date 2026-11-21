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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H
#include "gucefASSETSTORE_CAssetBlockCache.h"
#define GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H
#endif /* GUCEF_ASSETSTORE_CASSETBLOCKCACHE_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#include "gucefASSETSTORE_CRemoteFileTransfer.h"
#define GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H
#include "gucefASSETSTORE_CRemoteFileAccess.h"
#define GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H
#include "gucefASSETSTORE_CAssetThumbnailFiles.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestRemoteAssetFiles.h"

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

static const UInt32 BlockBytes = CAssetBlockCache::BlockBytes;
static const UInt32 BigSize = 13 * 262144 + 100000;             // 14 blocks, the last one short

/*-------------------------------------------------------------------------*/

/** The service of the server side with a delay on every read of a block: a slow link, and a count of the calls */
class CSlowLocalService : public CLocalAssetStoreService
{
    public:

    MT::CTAtomic< UInt32 > delayMs;
    MT::CTAtomic< UInt32 > calls;
    MT::CTAtomic< UInt64 > bytes;

    explicit CSlowLocalService( const CAssetStoreServiceConfig& config )
        : CLocalAssetStoreService( config ), delayMs( 0 ), calls( 0 ), bytes( 0 )
    {
    }

    virtual TAssetStoreStatus ReadFileBlock( const CString& vfsPath, UInt64 offset, UInt32 length, CAssetFileBlock& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        calls.FetchAdd( 1 );
        const UInt32 delay = delayMs.Load();
        if ( delay > 0 )
            MT::PrecisionDelay( delay );
        const TAssetStoreStatus status = CLocalAssetStoreService::ReadFileBlock( vfsPath, offset, length, result );
        bytes.FetchAdd( result.data.GetDataSize() );
        return status;
    }
};

/*-------------------------------------------------------------------------*/

/** A server side with its files, a client of it and the file system of the client */
class CFilesWorld
{
    public:

    CString name;
    CString base;
    CString mainRoot;
    CString prefix;
    CRemoteAssetStoreWorld world;
    CSlowLocalService* slow;
    TAssetStoreServicePtr client;
    CRemoteAssetFileSystem* system;
    CRemoteFilesSettings settings;
    CDynamicBuffer big;
    CString bigDigest;
    bool started;

    explicit CFilesWorld( const CString& worldName, bool withRoles = false, const CString& thumbnailsDirectory = CString::Empty )
        : name( worldName ), base( "astest/rf_" + worldName ), mainRoot(), prefix( "client" ), world( "rf_" + worldName ), slow( GUCEF_NULL ), client(), system( GUCEF_NULL )
        , settings(), big(), bigDigest(), started( false )
    {
        mainRoot = base + "/main";
        if ( withRoles )
        {
            world.bindingSettings.roles.insert( "viewer" );
            world.bindingSettings.roles.insert( "mcp" );
        }
        MountFreshDirectory( "rf_" + name + "_main", base + "/main" );
        MountFreshDirectory( "rf_" + name + "_source", base + "/source" );
        MountFreshDirectory( "rf_" + name + "_catalogue", base + "/catalogue" );
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
        config.thumbnailsDirectory = thumbnailsDirectory;
        config.implementationId = CAssetStoreServiceConfig::LocalImplementationId;
        world.serverConfig = config;
        slow = GUCEF_NEW CSlowLocalService( config );
        world.serverService = TAssetStoreServicePtr( CORE::CTSharedPtr< CIAssetStoreService, MT::CMutex >( slow ) );
        CString error;
        if ( ASSETSTORE_STATUS_OK != world.serverService->Open() )
            return;
        if ( !world.StartHost( error ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "FilesWorld: the host did not start: " << error );
            return;
        }
        client = world.MakeClient( "files", error );
        if ( client.IsNULL() || ASSETSTORE_STATUS_OK != client->Open() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "FilesWorld: no client: " << error );
            return;
        }
        started = true;
    }

    ~CFilesWorld()
    {
        CloseSystem();
        client.Unlink();
        world.StopServer();
        UnmountDirectory( "rf_" + name + "_main" );
        UnmountDirectory( "rf_" + name + "_source" );
        UnmountDirectory( "rf_" + name + "_catalogue" );
    }

    CString CacheFolder( void ) const
    {
        return TestDirectory( "rf_cache_" + name );
    }

    /** Opens a file system on the client with a fresh cache (or the cache that is there) and mounts the location */
    bool OpenSystem( bool freshCache )
    {
        CloseSystem();
        if ( freshCache )
            DeleteDir( CacheFolder(), true );
        settings.cacheFolder = CacheFolder();
        system = GUCEF_NEW CRemoteAssetFileSystem( *client, settings );
        if ( !system->Open() || 0 == system->MountLocations( prefix ) )
        {
            CloseSystem();
            return false;
        }
        return true;
    }

    void CloseSystem( void )
    {
        if ( GUCEF_NULL != system )
        {
            system->Close();
            GUCEF_DELETE system;
            system = GUCEF_NULL;
        }
    }

    CString ServerPath( const CString& relative ) const { return mainRoot + "/" + relative; }
    CString ClientPath( const CString& relative ) const { return prefix + "/" + mainRoot + "/" + relative; }
};

/*-------------------------------------------------------------------------*/

/** Reads a whole file through the VFS in pieces */
bool
ReadViaVfs( const CString& path, CDynamicBuffer& content, UInt32 piece )
{
    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( path, "rb", false );
    if ( file.IsNULL() )
        return false;
    IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return false;
    content.Clear( false );
    GUCEF::vector< UInt8 > buffer;
    buffer.resize( piece );
    while ( true )
    {
        const UInt32 read = access->Read( &buffer[ 0 ], 1, piece );
        if ( 0 == read )
            break;
        content.Append( &buffer[ 0 ], read );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SameBytes( const CDynamicBuffer& a, const CDynamicBuffer& b )
{
    return a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteServiceConnection*
ConnectionOfClient( CIAssetStoreService& service )
{
    CRemoteAssetStoreService* remote = CAssetStoreServiceConfig::RemoteImplementationId == service.GetType() ? static_cast< CRemoteAssetStoreService* >( &service ) : GUCEF_NULL;
    return GUCEF_NULL == remote ? GUCEF_NULL : remote->GetConnection();
}

/*-------------------------------------------------------------------------*/

/** A file of the disk, whole */
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

/** Writes a pack of one texture with its licence restrictions on the source side of a world and ingests it; the pack id and the id of its asset */
bool
IngestTexturePack( CFilesWorld& world, CIAssetStoreService& service, const CString& packName, const CString& restrictions, UInt32 variant, CString& packId, CString& assetId, CString& mainPath )
{
    const CString root = world.base + "/source/" + packName;
    CDynamicBuffer png;
    // another picture per pack and per run: the same content is a duplicate, and a catalogue in a database outlives the run
    const UInt32 nonce = (UInt32) ( MT::PrecisionTickCount() % 3721 );
    MakePng( 16 + variant * 8 + nonce % 61, 16 + variant * 8 + nonce / 61, false, png );
    WriteVfsFile( root + "/pack.json", "{ \"title\": \"" + packName + "\", \"author\": \"Test Works\", \"licence\": { \"name\": \"Test\", \"restrictions\": [ " + restrictions + " ] } }" );
    WriteVfsFile( root + "/Textures/T_" + packName + "_D.png", png );
    CAssetIngestRequest request;
    request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
    request.contentPath = root;
    request.keep = "original";
    CAssetIngestResult result;
    if ( ASSETSTORE_STATUS_OK != service.Ingest( "rf-ingest-" + packName, request, result ) || result.packId.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "IngestTexturePack: " << packName << ": " << AssetStoreStatusToString( result.status ) << ", " << result.message );
        return false;
    }
    packId = result.packId;
    CAssetRecordPage assets;
    if ( ASSETSTORE_STATUS_OK != service.GetAssets( packId, CString::Empty, 0, assets ) || assets.entries.empty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "IngestTexturePack: " << packName << " has no assets" );
        return false;
    }
    assetId = assets.entries[ 0 ].assetId;
    CAssetFileSet files;
    if ( ASSETSTORE_STATUS_OK != service.GetAssetFileSet( assetId, files ) )
        return false;
    mainPath = files.mainVfsPath;
    return !mainPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

/** A block of a pattern for the cache tests */
void
MakeBlockData( UInt32 size, UInt8 seed, GUCEF::vector< UInt8 >& data )
{
    data.resize( size );
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( seed + i * 7 );
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

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStoreBlockCacheTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreBlockCache" );

    const CString folder = TestDirectory( "blockcache" );
    DeleteDir( folder, true );

    GUCEF_TESTFW_TESTCASE( "Block cache: blocks are kept and read back, a miss is a miss, a restart keeps what was fetched (resume), the digest of the whole file is checked" )
        try
        {
            CAssetBlockCache cache;
            ASSERT_TRUE( cache.Open( folder, 64ULL * 1024 * 1024 ) );
            // 5 blocks, the last one short
            const UInt64 size = 4ULL * BlockBytes + 1000;
            GUCEF::vector< UInt8 > whole;
            MakeBlockData( (UInt32) size, 3, whole );
            CDynamicBuffer wholeBuffer;
            wholeBuffer.CopyFrom( (UInt32) size, &whole[ 0 ] );
            const CString digest = Sha256Of( wholeBuffer );
            CAssetBlockCache::TEntryHandle entry = cache.AcquireEntry( "loc/pack/a.bin", size, digest );
            ASSERT_TRUE( 0 != entry && 5 == cache.GetBlockCount( entry ) && BlockBytes == cache.GetBlockSize( entry, 0 ) && 1000 == cache.GetBlockSize( entry, 4 ) );
            ASSERT_TRUE( !cache.HasBlock( entry, 0 ) && !cache.IsComplete( entry ) );
            UInt8 probe[ 16 ];
            ASSERT_TRUE( CAssetBlockCache::READ_MISS == cache.ReadBlock( entry, 0, 0, probe, 16 ) );
            ASSERT_TRUE( cache.StoreBlock( entry, 0, &whole[ 0 ], BlockBytes ) && cache.StoreBlock( entry, 2, &whole[ 2 * (size_t) BlockBytes ], BlockBytes ) );
            ASSERT_TRUE( !cache.StoreBlock( entry, 1, &whole[ 0 ], 100 ) );        // a block has its size
            ASSERT_TRUE( CAssetBlockCache::READ_HIT == cache.ReadBlock( entry, 2, 100, probe, 16 ) && 0 == memcmp( probe, &whole[ 2 * (size_t) BlockBytes + 100 ], 16 ) );
            ASSERT_TRUE( 2 == cache.GetBlocksHeld( entry ) );
            cache.ReleaseEntry( entry );
            cache.Close();

            // a restart: the blocks are where they were, the rest is fetched
            ASSERT_TRUE( cache.Open( folder, 64ULL * 1024 * 1024 ) );
            entry = cache.AcquireEntry( "loc/pack/a.bin", size, digest );
            ASSERT_TRUE( 0 != entry && 2 == cache.GetBlocksHeld( entry ) && cache.HasBlock( entry, 0 ) && !cache.HasBlock( entry, 1 ) && cache.HasBlock( entry, 2 ) );
            ASSERT_TRUE( CAssetBlockCache::READ_HIT == cache.ReadBlock( entry, 0, 5, probe, 16 ) && 0 == memcmp( probe, &whole[ 5 ], 16 ) );
            ASSERT_TRUE( !cache.VerifyDigest( entry, digest ) );                    // not complete
            ASSERT_TRUE( cache.StoreBlock( entry, 1, &whole[ BlockBytes ], BlockBytes ) && cache.StoreBlock( entry, 3, &whole[ 3 * (size_t) BlockBytes ], BlockBytes ) &&
                         cache.StoreBlock( entry, 4, &whole[ 4 * (size_t) BlockBytes ], 1000 ) );
            ASSERT_TRUE( cache.IsComplete( entry ) && cache.VerifyDigest( entry, digest ) && !cache.VerifyDigest( entry, "00" + digest.SubstrFromRange( 2, 64 ) ) );
            // another content under the same path empties the entry
            cache.ReleaseEntry( entry );
            entry = cache.AcquireEntry( "loc/pack/a.bin", size + 1, CString::Empty );
            ASSERT_TRUE( 0 != entry && 0 == cache.GetBlocksHeld( entry ) );
            cache.ReleaseEntry( entry );
            cache.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Block cache: a block that the disk gave back damaged is found by its checksum, dropped and stored again" )
        try
        {
            DeleteDir( folder, true );
            CAssetBlockCache cache;
            ASSERT_TRUE( cache.Open( folder, 64ULL * 1024 * 1024 ) );
            const UInt64 size = 3ULL * BlockBytes;
            GUCEF::vector< UInt8 > whole;
            MakeBlockData( (UInt32) size, 9, whole );
            CAssetBlockCache::TEntryHandle entry = cache.AcquireEntry( "loc/pack/b.bin", size, CString::Empty );
            ASSERT_TRUE( 0 != entry );
            for ( UInt32 b=0; b<3; ++b )
                ASSERT_TRUE( cache.StoreBlock( entry, b, &whole[ (size_t) b * BlockBytes ], BlockBytes ) );
            cache.ReleaseEntry( entry );
            cache.Close();

            // a byte of the second block changes on the disk
            const CString dataFile = CombinePath( folder, CAssetBlockCache::MakeKey( "loc/pack/b.bin" ) + ".dat" );
            {
                CFileAccess file( dataFile, "r+b" );
                ASSERT_TRUE( file.Opened() );
                file.Setpos( (UInt64) BlockBytes + 77 );
                const UInt8 damaged = (UInt8) ( whole[ (size_t) BlockBytes + 77 ] ^ 0xFF );
                file.Write( &damaged, 1, 1 );
            }
            ASSERT_TRUE( cache.Open( folder, 64ULL * 1024 * 1024 ) );
            entry = cache.AcquireEntry( "loc/pack/b.bin", size, CString::Empty );
            UInt8 probe[ 16 ];
            ASSERT_TRUE( CAssetBlockCache::READ_HIT == cache.ReadBlock( entry, 0, 0, probe, 16 ) );
            ASSERT_TRUE( CAssetBlockCache::READ_CORRUPT == cache.ReadBlock( entry, 1, 70, probe, 16 ) && !cache.HasBlock( entry, 1 ) && 2 == cache.GetBlocksHeld( entry ) );
            ASSERT_TRUE( CAssetBlockCache::READ_MISS == cache.ReadBlock( entry, 1, 70, probe, 16 ) );
            ASSERT_TRUE( cache.StoreBlock( entry, 1, &whole[ BlockBytes ], BlockBytes ) );
            ASSERT_TRUE( CAssetBlockCache::READ_HIT == cache.ReadBlock( entry, 1, 70, probe, 16 ) && 0 == memcmp( probe, &whole[ (size_t) BlockBytes + 70 ], 16 ) );
            CAssetBlockCache::SStatistics statistics;
            cache.GetStatistics( statistics );
            ASSERT_TRUE( 1 == statistics.corruptBlocks );
            cache.ReleaseEntry( entry );
            cache.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Block cache: over the limit the entry used longest ago goes, a pinned entry and an entry in use stay" )
        try
        {
            DeleteDir( folder, true );
            CAssetBlockCache cache;
            ASSERT_TRUE( cache.Open( folder, 6ULL * BlockBytes ) );
            GUCEF::vector< UInt8 > data;
            MakeBlockData( BlockBytes, 1, data );
            CAssetBlockCache::TEntryHandle a = cache.AcquireEntry( "e/a", 2ULL * BlockBytes, CString::Empty );
            ASSERT_TRUE( cache.StoreBlock( a, 0, &data[ 0 ], BlockBytes ) && cache.StoreBlock( a, 1, &data[ 0 ], BlockBytes ) );
            cache.ReleaseEntry( a );
            CAssetBlockCache::TEntryHandle b = cache.AcquireEntry( "e/b", 2ULL * BlockBytes, CString::Empty );
            ASSERT_TRUE( cache.StoreBlock( b, 0, &data[ 0 ], BlockBytes ) && cache.StoreBlock( b, 1, &data[ 0 ], BlockBytes ) );
            cache.SetEntryPinned( b, true );
            cache.ReleaseEntry( b );
            CAssetBlockCache::TEntryHandle c = cache.AcquireEntry( "e/c", 2ULL * BlockBytes, CString::Empty );
            ASSERT_TRUE( cache.StoreBlock( c, 0, &data[ 0 ], BlockBytes ) && cache.StoreBlock( c, 1, &data[ 0 ], BlockBytes ) );
            ASSERT_TRUE( 6ULL * BlockBytes == cache.GetUsedBytes() );
            // c is in use, b is pinned, a is the oldest: it goes
            CAssetBlockCache::TEntryHandle d = cache.AcquireEntry( "e/d", 2ULL * BlockBytes, CString::Empty );
            ASSERT_TRUE( cache.StoreBlock( d, 0, &data[ 0 ], BlockBytes ) );
            CAssetBlockCache::SEntryInfo info;
            ASSERT_TRUE( !cache.GetEntryInfo( "e/a", info ) );
            ASSERT_TRUE( cache.GetEntryInfo( "e/b", info ) && info.pinned && info.complete );
            ASSERT_TRUE( cache.GetEntryInfo( "e/c", info ) && info.complete );
            CAssetBlockCache::SStatistics statistics;
            cache.GetStatistics( statistics );
            ASSERT_TRUE( 1 == statistics.evictedEntries && statistics.usedBytes <= 6ULL * BlockBytes && 1 == statistics.pinnedEntries );
            // the limit comes down: only what is free to go goes
            cache.ReleaseEntry( c );
            cache.ReleaseEntry( d );
            cache.SetMaxBytes( 3ULL * BlockBytes );
            ASSERT_TRUE( cache.GetEntryInfo( "e/b", info ) && !cache.GetEntryInfo( "e/c", info ) );
            ASSERT_TRUE( cache.SetPinned( "e/b", false ) );
            cache.Close();
            // the pin and the blocks survive a restart
            ASSERT_TRUE( cache.Open( folder, 3ULL * BlockBytes ) );
            ASSERT_TRUE( cache.GetEntryInfo( "e/b", info ) && info.complete && !info.pinned );
            cache.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    DeleteDir( folder, true );
}

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteFilesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteFiles" );

    CFilesWorld w( "files" );
    GUCEF_TESTFW_TESTCASE( "Remote files: a location of the service mounts at the VFS root of the location, and its files open through the VFS" )
        try
        {
            ASSERT_TRUE( w.started );
            ASSERT_TRUE( w.OpenSystem( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            ASSERT_TRUE( w.system->IsMounted( w.mainRoot ) );
            ASSERT_TRUE( vfs.FileExists( w.ClientPath( "bigpack/original/big.bin" ) ) && vfs.FileExists( w.ClientPath( "bigpack/original/small.txt" ) ) );
            ASSERT_TRUE( !vfs.FileExists( w.ClientPath( "bigpack/original/none.bin" ) ) && !vfs.FileExists( w.ClientPath( "bigpack/original" ) ) );
            ASSERT_TRUE( vfs.DirExists( w.ClientPath( "bigpack/original" ) ) && vfs.DirExists( w.ClientPath( "bigpack/intermediate/a" ) ) && !vfs.DirExists( w.ClientPath( "nothing" ) ) );
            ASSERT_TRUE( BigSize == vfs.GetFileSize( w.ClientPath( "bigpack/original/big.bin" ) ) );
            VFS::CVFS::TStringVector files;
            vfs.GetFileList( files, w.ClientPath( "bigpack/original" ), false, false, CString::Empty );
            ASSERT_TRUE( 2 == files.size() );
            VFS::CVFS::TStringVector dirs;
            vfs.GetDirList( dirs, w.ClientPath( "bigpack" ), false, false, CString::Empty );
            ASSERT_TRUE( 2 == dirs.size() );
            CString servicePath;
            ASSERT_TRUE( w.system == CRemoteAssetFileSystem::FindForPath( w.ClientPath( "bigpack/original/big.bin" ), servicePath ) && servicePath == w.ServerPath( "bigpack/original/big.bin" ) );
            ASSERT_TRUE( GUCEF_NULL == CRemoteAssetFileSystem::FindForPath( "astest/other/x.bin", servicePath ) );

            CDynamicBuffer text;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/small.txt" ), text, 4 ) && 24 == text.GetDataSize() && 0 == memcmp( text.GetConstBufferPtr(), "a small file of the pack", 24 ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 100000 ) && SameBytes( content, w.big ) );
            ASSERT_TRUE( !VFS::CVfsGlobal::Instance()->GetVfs().GetFile( w.ClientPath( "bigpack/original/none.bin" ), "rb", false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a read fetches only the blocks it touches, a second read is from the cache, positioning follows the f-functions" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            const UInt32 callsBefore = w.slow->calls.Load();
            VFS::TBasicVfsResourcePtr file = vfs.GetFile( w.ClientPath( "bigpack/original/big.bin" ), "rb", false );
            ASSERT_TRUE( !file.IsNULL() );
            IOAccessPtr access = file->GetAccess();
            ASSERT_TRUE( BigSize == access->GetSize() );
            UInt8 probe[ 32 ];
            // a header probe: one block
            ASSERT_TRUE( 0 == access->Setpos( 0 ) && 32 == access->Read( probe, 1, 32 ) && 0 == memcmp( probe, w.big.GetConstBufferPtr(), 32 ) );
            CRemoteAssetFileSystem::SStatistics statistics;
            w.system->GetStatistics( statistics );
            ASSERT_TRUE( BlockBytes == statistics.bytesFetched && 1 == w.slow->calls.Load() - callsBefore );
            // a read across a block border, from the middle of the file
            const UInt64 border = 5ULL * BlockBytes;
            UInt8 across[ 64 ];
            ASSERT_TRUE( 0 == access->Setpos( border - 20 ) && 64 == access->Read( across, 1, 64 ) && 0 == memcmp( across, static_cast< const UInt8* >( w.big.GetConstBufferPtr() ) + border - 20, 64 ) );
            w.system->GetStatistics( statistics );
            ASSERT_TRUE( 3ULL * BlockBytes == statistics.bytesFetched );
            // the end of the file: the short block, then nothing
            ASSERT_TRUE( 0 == access->Seek( -10, SEEK_END ) && 10 == access->Read( probe, 1, 32 ) && 0 == memcmp( probe, static_cast< const UInt8* >( w.big.GetConstBufferPtr() ) + BigSize - 10, 10 ) );
            ASSERT_TRUE( access->Eof() && 0 == access->Read( probe, 1, 1 ) );
            // reading the same again costs no call
            const UInt32 calls = w.slow->calls.Load();
            ASSERT_TRUE( 0 == access->Setpos( 0 ) && 32 == access->Read( probe, 1, 32 ) && 0 == access->Setpos( border - 20 ) && 64 == access->Read( across, 1, 64 ) );
            ASSERT_TRUE( calls == w.slow->calls.Load() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a sequential read through a slow link reads ahead (fewer, larger calls), and gives the bytes of the file" )
        try
        {
            ASSERT_TRUE( w.started );
            w.slow->delayMs.Store( 20 );
            UInt32 callsWithout = 0;
            UInt64 msWithout = 0;
            UInt32 callsWith = 0;
            UInt64 msWith = 0;
            for ( UInt32 round=0; round<2; ++round )
            {
                w.settings.readAheadBlocks = 0 == round ? 0 : 8;
                w.settings.readAheadAfterBlocks = 2;
                ASSERT_TRUE( w.OpenSystem( true ) );
                const UInt32 callsBefore = w.slow->calls.Load();
                const UInt64 started = MT::PrecisionTickCount();
                CDynamicBuffer content;
                ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
                const UInt64 ms = ( MT::PrecisionTickCount() - started ) * 1000 / MT::PrecisionTimerResolution();
                CRemoteAssetFileSystem::SStatistics statistics;
                w.system->GetStatistics( statistics );
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
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "RemoteFiles: " << BigSize << " bytes through a link of 20 ms per call, in 64 KiB reads: without read ahead " << callsWithout << " calls in " << msWithout
                       << " ms, with read ahead " << callsWith << " calls in " << msWith << " ms (the service answers each 256 KiB block as a call of its own, in parallel)" );
            ASSERT_TRUE( 14 == callsWithout && callsWith <= 5 && msWith < msWithout );
            w.slow->delayMs.Store( 0 );
            w.settings.readAheadBlocks = 8;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a read of a file that was cancelled ends with a message, nothing is asked of the service" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
            CRemoteFileAccess access( w.system->GetId(), w.ServerPath( "bigpack/original/big.bin" ) );
            ASSERT_TRUE( access.IsValid() );
            const UInt32 callsBefore = w.slow->calls.Load();
            access.Cancel();
            UInt8 probe[ 16 ];
            ASSERT_TRUE( 0 == access.Read( probe, 1, 16 ) && ASSETSTORE_STATUS_CANCELLED == access.GetLastStatus() && access.GetLastMessage().HasSubstr( "cancelled" ) >= 0 );
            ASSERT_TRUE( callsBefore == w.slow->calls.Load() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: offline, the files of the cache open and read, the others say that they are not in the cache and the service cannot be reached" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 262144 ) && SameBytes( content, w.big ) );
            CDynamicBuffer small;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/small.txt" ), small, 100 ) );
            // another file, known by its list, not read yet
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            ASSERT_TRUE( vfs.FileExists( w.ClientPath( "bigpack/intermediate/a/b.txt" ) ) );
            // the link is gone
            w.client->Close();
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 4096 ) && SameBytes( content, w.big ) );
            ASSERT_TRUE( vfs.FileExists( w.ClientPath( "bigpack/intermediate/a/b.txt" ) ) );
            CRemoteFileAccess access( w.system->GetId(), w.ServerPath( "bigpack/intermediate/a/b.txt" ) );
            ASSERT_TRUE( access.IsValid() );
            UInt8 probe[ 8 ];
            ASSERT_TRUE( 0 == access.Read( probe, 1, 8 ) );
            ASSERT_TRUE( access.GetLastMessage().HasSubstr( "not available offline" ) >= 0 && access.GetLastMessage().HasSubstr( "cannot be reached" ) >= 0 );
            ASSERT_TRUE( w.system->IsOffline() );
            // a folder that was never listed says nothing
            CString::StringVector dirs;
            CString::StringVector names;
            ASSERT_TRUE( !w.system->GetFolderEntries( w.ServerPath( "bigpack/unseen" ), dirs, names ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.client->Open() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a block of the cache damaged on the disk is fetched again when a restart reads it, and the file is right" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
            w.CloseSystem();
            const CString dataFile = CombinePath( w.CacheFolder(), CAssetBlockCache::MakeKey( w.ServerPath( "bigpack/original/big.bin" ) ) + ".dat" );
            ASSERT_TRUE( FileExists( dataFile ) );
            {
                CFileAccess file( dataFile, "r+b" );
                ASSERT_TRUE( file.Opened() );
                file.Setpos( 3ULL * BlockBytes + 5 );
                const UInt8 damaged = (UInt8) ( static_cast< const UInt8* >( w.big.GetConstBufferPtr() )[ 3ULL * BlockBytes + 5 ] ^ 0x55 );
                file.Write( &damaged, 1, 1 );
            }
            ASSERT_TRUE( w.OpenSystem( false ) );
            const UInt32 callsBefore = w.slow->calls.Load();
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 65536 ) && SameBytes( content, w.big ) );
            ASSERT_TRUE( 1 == w.slow->calls.Load() - callsBefore );             // the one block
            CAssetBlockCache::SStatistics statistics;
            w.system->GetCache().GetStatistics( statistics );
            ASSERT_TRUE( 1 == statistics.corruptBlocks );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a file that is not the one of the catalogue is refused (its digest), emptied from the cache and reported in plain words; the right digest passes" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
            const CString path = w.ServerPath( "bigpack/original/big.bin" );
            CString message;
            w.system->NoteFile( path, BigSize, "00" + w.bigDigest.SubstrFromRange( 2, 64 ) );
            const TAssetStoreStatus wrong = w.system->FetchWholeFile( path, false, GUCEF_NULL, GUCEF_NULL, message );
            ASSERT_TRUE( ASSETSTORE_STATUS_DIGEST_MISMATCH == wrong && message.HasSubstr( "differs from what its catalogue says" ) >= 0 );
            CAssetBlockCache::SEntryInfo info;
            ASSERT_TRUE( !w.system->GetCache().GetEntryInfo( path, info ) || 0 == info.blocksHeld );
            w.system->NoteFile( path, BigSize, w.bigDigest );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.system->FetchWholeFile( path, true, GUCEF_NULL, GUCEF_NULL, message ) );
            ASSERT_TRUE( w.system->GetCache().GetEntryInfo( path, info ) && info.complete && info.pinned && info.digest == w.bigDigest );
            CRemoteAssetFileSystem::SStatistics statistics;
            w.system->GetStatistics( statistics );
            ASSERT_TRUE( statistics.digestMismatches >= 2 && statistics.filesVerified >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a download to a folder through a link that drops completes, is digest checked, and a cancelled one continues from its part" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxBlocksPerRequest = 1;
            ASSERT_TRUE( w.OpenSystem( true ) );
            w.system->NoteFile( w.ServerPath( "bigpack/original/big.bin" ), BigSize, w.bigDigest );
            const CString target = TestDirectory( "rf_download" );
            DeleteDir( target, true );
            w.slow->delayMs.Store( 40 );

            // a link that drops in the middle of the transfer
            TRemoteFileTransferPtr first( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            first->SetDestination( target );
            first->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            CFutureResult future = w.system->StartTransfer( first );
            ASSERT_TRUE( future.HasAFuture() );
            for ( UInt32 waited=0; waited<10000 && first->GetBytesDone() < 3ULL * BlockBytes; waited+=10 )
                PumpFor( 10 );
            ConnectionOfClient( *w.client )->DropLink();
            ASSERT_TRUE( PumpUntilTransferEnded( first, 60000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == first->GetState() && ASSETSTORE_STATUS_OK == first->GetStatus() );
            CDynamicBuffer downloaded;
            ASSERT_TRUE( ReadDiskFile( CombinePath( target, "big.bin" ), downloaded ) && SameBytes( downloaded, w.big ) );
            ASSERT_TRUE( !FileExists( CombinePath( target, "big.bin.part" ) ) );

            // cancelled in the middle: the part stays, the next run reuses it
            DeleteFile( CombinePath( target, "big.bin" ) );
            w.system->GetCache().Clear();
            TRemoteFileTransferPtr second( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            second->SetDestination( target );
            second->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            future = w.system->StartTransfer( second );
            for ( UInt32 waited=0; waited<10000 && second->GetBytesDone() < 5ULL * BlockBytes; waited+=10 )
                PumpFor( 10 );
            second->Cancel();
            ASSERT_TRUE( PumpUntilTransferEnded( second, 30000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_CANCELLED == second->GetState() && second->GetBytesDone() < BigSize );
            ASSERT_TRUE( FileExists( CombinePath( target, "big.bin.part" ) ) && !FileExists( CombinePath( target, "big.bin" ) ) );
            w.system->GetCache().Clear();
            w.system->ForgetMetadata();
            w.system->NoteFile( w.ServerPath( "bigpack/original/big.bin" ), BigSize, w.bigDigest );
            w.slow->delayMs.Store( 0 );
            TRemoteFileTransferPtr third( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "big" ) );
            third->SetDestination( target );
            third->AddFile( w.ServerPath( "bigpack/original/big.bin" ), "big.bin", BigSize, w.bigDigest );
            future = w.system->StartTransfer( third );
            ASSERT_TRUE( PumpUntilTransferEnded( third, 30000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == third->GetState() && third->GetBytesReused() >= 3ULL * BlockBytes );
            ASSERT_TRUE( ReadDiskFile( CombinePath( target, "big.bin" ), downloaded ) && SameBytes( downloaded, w.big ) && !FileExists( CombinePath( target, "big.bin.part" ) ) );

            // a file that is there already and right is not fetched again; a folder with its files
            const UInt32 callsBefore = w.slow->calls.Load();
            TRemoteFileTransferPtr again( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_DOWNLOAD, "pack" ) );
            again->SetDestination( target );
            again->AddFolder( w.ServerPath( "bigpack/original" ), "" );
            w.system->NoteDigest( w.ServerPath( "bigpack/original/big.bin" ), w.bigDigest );
            future = w.system->StartTransfer( again );
            ASSERT_TRUE( PumpUntilTransferEnded( again, 30000 ) );
            ASSERT_TRUE( CRemoteFileTransfer::STATE_DONE == again->GetState() && 2 == again->GetFileCount() && again->GetBytesReused() >= BigSize );
            ASSERT_TRUE( FileExists( CombinePath( target, "small.txt" ) ) && w.slow->calls.Load() - callsBefore <= 1 );
            w.settings.maxBlocksPerRequest = 16;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: a large file (64 MiB) streams through the block cache over the loopback link: whole by the transfer and sequentially through the VFS, both digest right, the second pass from the cache" )
        try
        {
            ASSERT_TRUE( w.started );
            const UInt32 hugeSize = 64 * 1024 * 1024;
            CDynamicBuffer huge;
            MakePattern( hugeSize, 11, huge );
            const CString hugeDigest = Sha256Of( huge );
            ASSERT_TRUE( WriteVfsFile( w.ServerPath( "bigpack/original/huge.bin" ), huge ) );
            w.settings.maxCacheBytes = 20ULL * 1024 * 1024 * 1024;
            ASSERT_TRUE( w.OpenSystem( true ) );
            const CString path = w.ServerPath( "bigpack/original/huge.bin" );
            w.system->NoteFile( path, hugeSize, hugeDigest );
            CString message;
            UInt64 start = MT::PrecisionTickCount();
            const UInt64 resolution = MT::PrecisionTimerResolution();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == w.system->FetchWholeFile( path, false, GUCEF_NULL, GUCEF_NULL, message ) );
            const UInt64 wholeMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            CRemoteAssetFileSystem::SStatistics statistics;
            w.system->GetStatistics( statistics );
            const UInt64 calls = statistics.fetchCalls;
            start = MT::PrecisionTickCount();
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/huge.bin" ), content, 65536 ) && content.GetDataSize() == hugeSize && Sha256Of( content ) == hugeDigest );
            const UInt64 cachedMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            w.system->GetStatistics( statistics );
            ASSERT_TRUE( statistics.fetchCalls == calls );
            // the same through the VFS with no cache entry yet: sequential reads fetch ahead
            w.system->GetCache().Clear();
            w.system->GetStatistics( statistics );
            const UInt64 callsBefore = statistics.fetchCalls;
            start = MT::PrecisionTickCount();
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/huge.bin" ), content, 65536 ) && Sha256Of( content ) == hugeDigest );
            const UInt64 streamMs = ( MT::PrecisionTickCount() - start ) * 1000 / resolution;
            w.system->GetStatistics( statistics );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "RemoteFiles: 64 MiB over the loopback link: whole file by the transfer " << wholeMs << " ms (" << ( wholeMs > 0 ? 64000 / wholeMs : 0 ) << " MiB/s, " << calls
                       << " calls), streamed through the VFS in 64 KiB reads " << streamMs << " ms (" << ( streamMs > 0 ? 64000 / streamMs : 0 ) << " MiB/s including the hash, " << ( statistics.fetchCalls - callsBefore )
                       << " calls), the same from the cache " << cachedMs << " ms" );
            ASSERT_TRUE( statistics.fetchCalls - callsBefore <= 40 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: keep offline pins the files in the cache, and the cache keeps them under a size limit that removes the rest" )
        try
        {
            ASSERT_TRUE( w.started );
            w.settings.maxCacheBytes = 16ULL * BlockBytes;
            ASSERT_TRUE( w.OpenSystem( true ) );
            TRemoteFileTransferPtr pin( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_KEEP_OFFLINE, "keep" ) );
            pin->AddFolder( w.ServerPath( "bigpack/intermediate" ), "" );
            CFutureResult future = w.system->StartTransfer( pin );
            ASSERT_TRUE( PumpUntilTransferEnded( pin, 30000 ) && CRemoteFileTransfer::STATE_DONE == pin->GetState() );
            ASSERT_TRUE( w.system->GetCache().IsPinned( w.ServerPath( "bigpack/intermediate/a/b.txt" ) ) );
            // a read of the big file (14 blocks) and then of other data pushes out unpinned entries but never the pin
            CDynamicBuffer content;
            ASSERT_TRUE( ReadViaVfs( w.ClientPath( "bigpack/original/big.bin" ), content, 262144 ) && SameBytes( content, w.big ) );
            w.system->GetCache().SetMaxBytes( 2ULL * BlockBytes );
            CAssetBlockCache::SEntryInfo info;
            ASSERT_TRUE( w.system->GetCache().GetEntryInfo( w.ServerPath( "bigpack/intermediate/a/b.txt" ), info ) && info.complete && info.pinned );
            ASSERT_TRUE( !w.system->GetCache().GetEntryInfo( w.ServerPath( "bigpack/original/big.bin" ), info ) );
            // released, then it goes with the limit
            TRemoteFileTransferPtr release( GUCEF_NEW CRemoteFileTransfer( CRemoteFileTransfer::MODE_RELEASE_OFFLINE, "release" ) );
            release->AddFolder( w.ServerPath( "bigpack/intermediate" ), "" );
            future = w.system->StartTransfer( release );
            ASSERT_TRUE( PumpUntilTransferEnded( release, 30000 ) && !w.system->GetCache().IsPinned( w.ServerPath( "bigpack/intermediate/a/b.txt" ) ) );
            w.settings.maxCacheBytes = 20ULL * 1024 * 1024 * 1024;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files: the stored thumbnails are put by a device, found by the digest of the content and the size, and read through the files of the service" )
        try
        {
            ASSERT_TRUE( w.started && w.OpenSystem( true ) );
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
            ASSERT_TRUE( ReadViaVfs( w.system->ToClientPath( page.entries[ 0 ].vfsPath ), fetched, 4096 ) && SameBytes( fetched, png64 ) );
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

    w.CloseSystem();
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

void
PerformAssetStoreRemoteFilesGateTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetStoreRemoteFilesGate" );

    // the pictures of this world are kept in a folder of the settings: the gate and the permissions work the same
    const CString thumbnailsFolder = TestDirectory( "rf_thumbnails_gate" );
    DeleteDir( thumbnailsFolder, true );
    CFilesWorld g( "gate", true, thumbnailsFolder );
    CString packRestricted, assetRestricted, pathRestricted;
    CString packOpen, assetOpen, pathOpen;
    CString packNml, assetNml, pathNml;
    TAssetStoreServicePtr viewer;
    TAssetStoreServicePtr machine;
    TAssetStoreServicePtr learner;
    bool ready = false;
    if ( g.started )
    {
        CString error;
        ready = IngestTexturePack( g, *g.client, "Vault", "\"noRedistribution\"", 2, packRestricted, assetRestricted, pathRestricted ) &&
                IngestTexturePack( g, *g.client, "Fields", "\"attributionRequired\"", 3, packOpen, assetOpen, pathOpen ) &&
                IngestTexturePack( g, *g.client, "Corpus", "\"noMachineLearning\"", 4, packNml, assetNml, pathNml );
        CString::StringSet viewerRoles;
        viewerRoles.insert( "viewer" );
        viewer = g.world.MakeClient( "gate-viewer", error, &viewerRoles, false );
        CString::StringSet machineRoles;
        machineRoles.insert( "mcp" );
        machine = g.world.MakeClient( "gate-mcp", error, &machineRoles, false );
        CString::StringSet learnerRoles;
        learnerRoles.insert( "mlexport" );
        learner = g.world.MakeClient( "gate-learner", error, &learnerRoles, true );
        if ( viewer.IsNULL() || machine.IsNULL() )
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "RemoteFilesGate: a client was not made: " << error );
    }

    GUCEF_TESTFW_TESTCASE( "Remote files licence gate: the owner device reads every pack; a viewer is refused the files of a noRedistribution pack in plain words and reads the others" )
        try
        {
            ASSERT_TRUE( g.started && ready && !viewer.IsNULL() );
            // the service knows the flags of the pack a path is in
            CString::StringSet flags;
            const bool inPack = g.slow->GetRestrictionsOfPath( pathRestricted, flags );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "FilesGate: " << pathRestricted << " in a pack: " << inPack << ", flags " << flags.size() << ", folder " );
            ASSERT_TRUE( inPack && 1 == flags.count( "noRedistribution" ) );
            ASSERT_TRUE( g.slow->GetRestrictionsOfPath( pathOpen, flags ) && 0 == flags.count( "noRedistribution" ) && 1 == flags.count( "attributionRequired" ) );
            ASSERT_TRUE( !g.slow->GetRestrictionsOfPath( g.mainRoot + "/thumbnails/ab/x.png", flags ) );
            CAssetFileBlock block;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->ReadFileBlock( pathRestricted, 0, 16, block ) && 16 == block.data.GetDataSize() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->Open() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->ReadFileBlock( pathOpen, 0, 16, block ) && 16 == block.data.GetDataSize() );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->ReadFileBlock( pathRestricted, 0, 16, block ) );
            // the metadata of the pack stays visible
            CAssetFileSet files;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->GetAssetFileSet( assetRestricted, files ) && !files.mainVfsPath.IsNULLOrEmpty() );
            CAssetFileStatPage stats;
            CString::StringVector paths;
            paths.push_back( pathRestricted );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->StatFiles( paths, stats ) && 1 == stats.entries.size() );
            // through the file system: the refusal is said in words
            CRemoteFilesSettings settings;
            settings.cacheFolder = TestDirectory( "rf_cache_gate" );
            DeleteDir( settings.cacheFolder, true );
            CRemoteAssetFileSystem system( *viewer, settings );
            ASSERT_TRUE( system.Open() && 1 == system.MountLocations( "viewer" ) );
            CRemoteFileAccess access( system.GetId(), pathRestricted );
            ASSERT_TRUE( access.IsValid() );
            UInt8 probe[ 8 ];
            ASSERT_TRUE( 0 == access.Read( probe, 1, 8 ) && access.GetLastMessage().HasSubstr( "does not give the file" ) >= 0 );
            system.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files licence gate: the machine role (mcp) sees the metadata of every pack, the raw content stays withheld: no block, no thumbnail" )
        try
        {
            ASSERT_TRUE( g.started && ready && !machine.IsNULL() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->Open() );
            CAssetFileSet files;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->GetAssetFileSet( assetOpen, files ) && ASSETSTORE_STATUS_OK == machine->GetAssetFileSet( assetRestricted, files ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->GetPack( packRestricted, pack ) && ASSETSTORE_STATUS_OK == machine->GetPack( packNml, pack ) );
            CAssetFileStatPage listing;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->ListDirectory( g.mainRoot, CString::Empty, 0, listing ) && listing.entries.size() >= 3 );
            CAssetFileBlock block;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->ReadFileBlock( pathOpen, 0, 16, block ) );
            CString::StringVector ids;
            ids.push_back( assetOpen );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->GetThumbnails( ids, 64, page ) );
            // a search sees every pack, whatever its licence
            CAssetSearchQuery query;
            CAssetSearchResultPage results;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->Search( query, results ) && results.hits.size() >= 3 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Remote files licence gate: the thumbnails of a pack that the device may not have are refused, those it was given can be read; a learning role is told that a noMachineLearning pack has no such file" )
        try
        {
            ASSERT_TRUE( g.started && ready && !viewer.IsNULL() && !learner.IsNULL() );
            CDynamicBuffer png;
            ASSERT_TRUE( MakePng( 32, 32, false, png ) );
            const CString assets[ 2 ] = { assetRestricted, assetOpen };
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
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->PutThumbnail( "gate-put-" + ToString( i ), thumbnail, put ) );
            }
            CString::StringVector ids;
            ids.push_back( assetRestricted );
            ids.push_back( assetOpen );
            CAssetThumbnailPage mine;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->GetThumbnails( ids, 32, mine ) && ASSETSTORE_STATUS_OK == mine.entries[ 0 ].status && ASSETSTORE_STATUS_OK == mine.entries[ 1 ].status );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->GetThumbnails( ids, 32, page ) && 2 == page.entries.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == page.entries[ 0 ].status && page.entries[ 0 ].vfsPath.IsNULLOrEmpty() && page.entries[ 0 ].message.HasSubstr( "owner" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == page.entries[ 1 ].status );
            CAssetFileBlock block;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->ReadFileBlock( page.entries[ 1 ].vfsPath, 0, 16, block ) );
            // the file of the picture that the device was not given
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->ReadFileBlock( mine.entries[ 0 ].vfsPath, 0, 16, block ) );

            // the learning role: the pack with noMachineLearning is not there for it
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == learner->Open() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == learner->ReadFileBlock( pathOpen, 0, 16, block ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == learner->ReadFileBlock( pathNml, 0, 16, block ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->ReadFileBlock( pathNml, 0, 16, block ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Thumbnail upload permission: the owner device puts and replaces a thumbnail, a viewer is refused and keeps its read access" )
        try
        {
            ASSERT_TRUE( g.started && ready && !viewer.IsNULL() );
            CDynamicBuffer png;
            ASSERT_TRUE( MakePng( 48, 48, false, png ) );
            CAssetThumbnail thumbnail;
            thumbnail.assetId = assetOpen;
            thumbnail.kind = "thumbnail";
            thumbnail.size = 48;
            thumbnail.key.size = 48;
            thumbnail.key.rendererId = "test";
            thumbnail.encoded = png;
            // the owner puts it, and replaces it
            CAssetServiceResult put;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->PutThumbnail( "perm-put-owner-1", thumbnail, put ) );
            thumbnail.key.rendererId = "test-replaced";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->PutThumbnail( "perm-put-owner-2", thumbnail, put ) );
            // the viewer is refused, with or without a command id
            CAssetServiceResult refused;
            thumbnail.key.rendererId = "test-viewer";
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->PutThumbnail( "perm-put-viewer-1", thumbnail, refused ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->PutThumbnail( CString::Empty, thumbnail, refused ) );
            // the viewer reads what the owner put
            CString::StringVector ids;
            ids.push_back( assetOpen );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->GetThumbnails( ids, 48, page ) && 1 == page.entries.size() && ASSETSTORE_STATUS_OK == page.entries[ 0 ].status );
            CAssetFileBlock block;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->ReadFileBlock( page.entries[ 0 ].vfsPath, 0, 16, block ) );
            // the picture is still the one of the owner
            CAssetThumbnailPage ownerPage;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == g.client->GetThumbnails( ids, 48, ownerPage ) && 1 == ownerPage.entries.size() && ASSETSTORE_STATUS_OK == ownerPage.entries[ 0 ].status );
            ASSERT_TRUE( ownerPage.entries[ 0 ].key.rendererId == "test-replaced" );
            // the folder of the settings holds the pictures, not the storage location
            ASSERT_TRUE( ownerPage.entries[ 0 ].vfsPath.HasSubstr( CAssetStoreServiceConfig::ThumbnailsVfsRoot + "/" ) == 0 );
            ASSERT_TRUE( FileExists( CombinePath( thumbnailsFolder, ownerPage.entries[ 0 ].vfsPath.CutChars( CAssetStoreServiceConfig::ThumbnailsVfsRoot.Length() + 1, true ) ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    viewer.Unlink();
    machine.Unlink();
    learner.Unlink();
}

/*-------------------------------------------------------------------------*/
