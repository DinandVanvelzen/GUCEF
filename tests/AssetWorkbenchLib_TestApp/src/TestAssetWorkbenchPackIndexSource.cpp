/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include "gucef_test_framework.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#include "AssetWorkbenchLib.h"
#include "AssetConverterLib.h"
#include "AssetConverterLib_CPackIndexWriter.h"
#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchPackIndexSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

void
Pump( UInt32 milliseconds )
{
    for ( UInt32 waited=0; waited<milliseconds; waited+=5 )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::PrecisionDelay( 5 );
    }
}

bool
WriteVfs( const CORE::CString& path, const CORE::CString& content )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const Int32 slash = path.HasChar( '/', false );
    if ( slash > 0 )
        vfs.CreateDir( path.SubstrToIndex( (UInt32) slash, true ) );
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( content, 0, false );
    return vfs.StoreAsFile( path, buffer, 0, true );
}

/** A folder of the disk mounted into the VFS at the mount point */
CORE::CString
MountFolder( const CORE::CString& name, const CORE::CString& mountPoint )
{
    const CORE::CString dir = MakeCleanTestDir( name );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.UnmountArchiveByName( "wbpi_" + name );
    vfs.AddRoot( dir, "wbpi_" + name, true, false, mountPoint );
    return dir;
}

/** The index of a pack of the files: structure level, sizes given (more than the files have, so the index never lists fewer bytes than the pack) */
bool
BuildDocument( const CORE::CString::StringVector& files, ASSETCONVERTER::CPackIndexDocument& doc )
{
    ASSETCONVERTER::CAssetPackListing listing;
    ASSETCONVERTER::CPackIndexBuilder builder( ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds() );
    for ( size_t i=0; i<files.size(); ++i )
    {
        listing.AddFile( files[ i ] );
        builder.SetFileSize( files[ i ], 100 + (UInt64) i );
    }
    ASSETCONVERTER::CPackIndexBuildOptions options;
    options.useFileSystemSizes = false;
    options.packName = "synthetic";
    return builder.BuildStructure( listing, ASSETCONVERTER::CAssetPackProfile(), options, doc, GUCEF_NULL );
}

void
MakeFiles( UInt32 models, UInt32 textures, CORE::CString::StringVector& files )
{
    files.clear();
    for ( UInt32 m=0; m<models; ++m )
        files.push_back( "Models/Props/SM_Prop_" + CORE::ToString( m + 10 ) + ".fbx" );
    for ( UInt32 t=0; t<textures; ++t )
        files.push_back( "Textures/T_Tex_" + CORE::ToString( t + 10 ) + ".png" );
    files.push_back( "Audio/hit.wav" );
}

bool
SamePage( const ASSETCONVERTER::CPackIndexResource::TResourceVector& a, const ASSETCONVERTER::CPackIndexResource::TResourceVector& b )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
        if ( a[ i ].path != b[ i ].path || a[ i ].name != b[ i ].name || a[ i ].kind != b[ i ].kind || a[ i ].group != b[ i ].group ||
             a[ i ].subGroup != b[ i ].subGroup || a[ i ].format != b[ i ].format || a[ i ].size != b[ i ].size )
            return false;
    return true;
}

/** Reads every page of a source into one list, the pages of the size given */
bool
ReadAll( IPackIndexSource& source, const CORE::CString& packId, const CORE::CString& group, const CORE::CString& subGroup, UInt32 pageSize,
         ASSETCONVERTER::CPackIndexResource::TResourceVector& all, UInt32& pages, UInt64& total )
{
    all.clear();
    pages = 0;
    CORE::CString token;
    do
    {
        ASSETCONVERTER::CPackIndexResource::TResourceVector page;
        CORE::CString next;
        if ( IPackIndexSource::RESULT_OK != source.GetResources( packId, group, subGroup, token, pageSize, page, next, total ) )
            return false;
        all.insert( all.end(), page.begin(), page.end() );
        token = next;
        ++pages;
    }
    while ( !token.IsNULLOrEmpty() && pages < 1000 );
    return true;
}

bool
SameGroups( const ASSETCONVERTER::CPackIndexGroup::TGroupVector& a, const ASSETCONVERTER::CPackIndexGroup::TGroupVector& b )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t g=0; g<a.size(); ++g )
    {
        if ( a[ g ].id != b[ g ].id || a[ g ].count != b[ g ].count || a[ g ].bytes != b[ g ].bytes || a[ g ].subGroups.size() != b[ g ].subGroups.size() )
            return false;
        for ( size_t s=0; s<a[ g ].subGroups.size(); ++s )
            if ( a[ g ].subGroups[ s ].id != b[ g ].subGroups[ s ].id || a[ g ].subGroups[ s ].count != b[ g ].subGroups[ s ].count )
                return false;
    }
    return true;
}

/** Counts what a service says of the catalogue */
class CChangeRecorder : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CChangeRecorder > TEventCallback;

    UInt32 indexUpdates;
    UInt32 removals;

    explicit CChangeRecorder( ASSETSTORE::CIAssetStoreService& service )
        : CORE::CObservingNotifier()
        , indexUpdates( 0 )
        , removals( 0 )
    {
        TEventCallback callback( this, &CChangeRecorder::OnChange );
        SubscribeTo( &service, ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent, callback );
    }

    virtual ~CChangeRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    private:

    void OnChange( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const ASSETSTORE::CAssetCatalogueChange* change = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetCatalogueChange >( eventData );
        if ( GUCEF_NULL == change )
            return;
        if ( ASSETSTORE::ASSETCHANGE_PACK_INDEX_UPDATED == change->kind )
            ++indexUpdates;
        if ( ASSETSTORE::ASSETCHANGE_PACK_REMOVED == change->kind )
            ++removals;
    }
};

/** Runs the job index-pack of a pack on the service and waits for it */
bool
RunIndexJob( ASSETSTORE::CIAssetStoreService& service, const CORE::CString& packId, bool force )
{
    ASSETSTORE::CAssetPipelineRequest request;
    request.operation = ASSETSTORE::CAssetPipelineOperations::IndexPack;
    request.packId = packId;
    request.force = force;
    ASSETSTORE::CAssetServiceResult started;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != service.StartPipelineJob( CORE::CString::Empty, request, started ) )
        return false;
    for ( UInt32 waited=0; waited<10000; waited+=5 )
    {
        ASSETSTORE::CAssetJobEntry job;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK == service.GetJob( started.id, job ) && job.IsEnded() )
            return ASSETSTORE::ASSETJOB_STATE_DONE == job.state;
        Pump( 5 );
    }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchPackIndexSourceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbench.PackIndexSource" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    MountFolder( "pisrc", "wbpisrc" );

    GUCEF_TESTFW_TESTCASE( "Local source: the manifest, the groups and the pages of the records of a document on the disk; the records are read once; a pack the source does not know, or that has no document, is not found" )
    try
    {
        CORE::CString::StringVector files;
        MakeFiles( 14, 10, files );
        ASSETCONVERTER::CPackIndexDocument doc;
        ASSERT_TRUE( BuildDocument( files, doc ) && 25 == doc.resources.size() );
        ASSETCONVERTER::CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( doc, "wbpisrc/local/a" ) );

        CLocalPackIndexSource source;
        ASSERT_TRUE( source.IsCheap() && !source.GetName().IsNULLOrEmpty() );
        source.SetPackFolder( "a", "wbpisrc/local/a" );
        source.SetPackFolder( "empty", "wbpisrc/local/empty" );
        ASSETCONVERTER::CPackIndexDocument manifest;
        bool stale = true;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( "a", manifest, stale ) && !stale );
        ASSERT_TRUE( manifest.identity.packKey == doc.identity.packKey && 25 == manifest.counts.resources && manifest.resources.empty() && !manifest.groups.empty() );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == source.GetManifest( "nothing", manifest, stale ) && !source.GetLastMessage().IsNULLOrEmpty() );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == source.GetManifest( "empty", manifest, stale ) );

        ASSETCONVERTER::CPackIndexGroup::TGroupVector groups;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetGroups( "a", groups ) && SameGroups( groups, doc.groups ) );

        // the pages: ten at a time, in the order of the index, with the total and the token of the next
        ASSETCONVERTER::CPackIndexResource::TResourceVector page;
        CORE::CString next;
        UInt64 total = 0;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetResources( "a", CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty, 10, page, next, total ) );
        ASSERT_TRUE( 10 == page.size() && 25 == total && !next.IsNULLOrEmpty() );
        ASSETCONVERTER::CPackIndexResource::TResourceVector all;
        UInt32 pages = 0;
        ASSERT_TRUE( ReadAll( source, "a", CORE::CString::Empty, CORE::CString::Empty, 10, all, pages, total ) && 3 == pages && 25 == all.size() );
        ASSERT_TRUE( 1 == source.GetDocumentReadCount() );
        for ( size_t i=1; i<all.size(); ++i )
            ASSERT_TRUE( !( all[ i ] < all[ i - 1 ] ) );

        // a group, and a sub-group of it
        ASSERT_TRUE( ReadAll( source, "a", "textures", CORE::CString::Empty, 4, all, pages, total ) && 10 == all.size() && 3 == pages && 10 == total );
        ASSERT_TRUE( ReadAll( source, "a", "static-models", "props", 100, all, pages, total ) && 14 == all.size() && 1 == pages );
        ASSERT_TRUE( ReadAll( source, "a", "static-models", "nothing", 100, all, pages, total ) && all.empty() && 0 == total );

        // the next document replaces what was read
        source.Invalidate( "a" );
        ASSERT_TRUE( ReadAll( source, "a", CORE::CString::Empty, CORE::CString::Empty, 100, all, pages, total ) && 2 == source.GetDocumentReadCount() );
        source.RemovePack( "a" );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == source.GetManifest( "a", manifest, stale ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // a service of this process with a catalogue, and two packs ingested from the disk
    vfs.CreateDir( "wbpisrc/main" );
    vfs.CreateDir( "wbpisrc/db" );
    vfs.CreateDir( "wbpisrc/source" );
    ASSETSTORE::CAssetStoreServiceConfig config;
    config.loadDownloadQueue = false;
    config.startDownloads = false;
    config.primaryStore.locations.clear();
    ASSETSTORE::CAssetStorageLocation location;
    location.locationId = "main";
    location.name = "Main";
    location.vfsRoot = "wbpisrc/main";
    config.primaryStore.locations.push_back( location );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.database.backendName = "vfstables";
    config.primaryStore.database.database = "wbpisrc/db";
    config.primaryStore.database.backendOptions.SetAllowMultipleValues( false );
    config.primaryStore.database.backendOptions.Set( "syncWrites", CORE::CVariant( "false" ) );
    ASSETSTORE::CLocalAssetStoreService service( config );
    ASSETSTORE::TAssetStoreStatus opened = service.Open();
    CORE::CString packX, packY;
    CORE::CString::StringVector filesX;
    MakeFiles( 14, 10, filesX );
    CORE::CString::StringVector filesY;
    MakeFiles( 3, 1, filesY );

    GUCEF_TESTFW_TESTCASE( "Service source: the same answers as the document on the disk: the manifest, the groups and the pages, for the pack that the catalogue has an index of" )
    try
    {
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == opened && service.IsCatalogueAvailable() );
        // the packs on the disk: one flagged noMachineLearning, one without
        ASSERT_TRUE( WriteVfs( "wbpisrc/source/PackX/pack.json", "{ \"title\": \"Pack X\", \"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" ) );
        ASSERT_TRUE( WriteVfs( "wbpisrc/source/PackY/pack.json", "{ \"title\": \"Pack Y\", \"licence\": { \"name\": \"CC BY 4.0\" } }" ) );
        for ( size_t f=0; f<filesX.size(); ++f )
            ASSERT_TRUE( WriteVfs( "wbpisrc/source/PackX/" + filesX[ f ], "x" ) );
        for ( size_t f=0; f<filesY.size(); ++f )
            ASSERT_TRUE( WriteVfs( "wbpisrc/source/PackY/" + filesY[ f ], "y" ) );
        ASSETSTORE::CAssetIngestRequest request;
        request.sourceId = "disk";
        request.keep = "original";
        ASSETSTORE::CAssetIngestResult result;
        request.contentPath = "wbpisrc/source/PackX";
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service.Ingest( CORE::CString::Empty, request, result ) );
        packX = result.packId;
        request.contentPath = "wbpisrc/source/PackY";
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service.Ingest( CORE::CString::Empty, request, result ) );
        packY = result.packId;

        // the documents the converter would have written, in the folders of the packs
        ASSETCONVERTER::CPackIndexDocument docX;
        ASSETCONVERTER::CPackIndexDocument docY;
        CORE::CString::StringVector listedX = filesX;
        listedX.push_back( "pack.json" );
        CORE::CString::StringVector listedY = filesY;
        listedY.push_back( "pack.json" );
        ASSERT_TRUE( BuildDocument( listedX, docX ) && BuildDocument( listedY, docY ) );
        ASSETCONVERTER::CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( docX, "wbpisrc/main/intermediate/disk/PackX" ) && writer.Write( docY, "wbpisrc/main/intermediate/disk/PackY" ) );
        ASSERT_TRUE( RunIndexJob( service, packX, false ) && RunIndexJob( service, packY, false ) );

        CLocalPackIndexSource local;
        local.SetPackFolder( packX, "wbpisrc/main/intermediate/disk/PackX" );
        local.SetPackFolder( packY, "wbpisrc/main/intermediate/disk/PackY" );
        CServicePackIndexSource remote( service );
        ASSERT_TRUE( remote.IsCheap() == service.IsInProcess() && !remote.GetName().IsNULLOrEmpty() );

        ASSETCONVERTER::CPackIndexDocument fromDisk;
        ASSETCONVERTER::CPackIndexDocument fromService;
        bool staleDisk = true;
        bool staleService = true;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == local.GetManifest( packX, fromDisk, staleDisk ) && IPackIndexSource::RESULT_OK == remote.GetManifest( packX, fromService, staleService ) );
        ASSERT_TRUE( !staleDisk && !staleService && fromDisk.identity.packKey == fromService.identity.packKey && fromDisk.producer.indexKey == fromService.producer.indexKey );
        ASSERT_TRUE( fromDisk.documentHash == fromService.documentHash && fromDisk.counts.resources == fromService.counts.resources && 26 == fromService.counts.resources );
        ASSERT_TRUE( fromService.resources.empty() && SameGroups( fromDisk.groups, fromService.groups ) );

        ASSETCONVERTER::CPackIndexGroup::TGroupVector groupsDisk;
        ASSETCONVERTER::CPackIndexGroup::TGroupVector groupsService;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == local.GetGroups( packX, groupsDisk ) && IPackIndexSource::RESULT_OK == remote.GetGroups( packX, groupsService ) );
        ASSERT_TRUE( SameGroups( groupsDisk, groupsService ) );

        ASSETCONVERTER::CPackIndexResource::TResourceVector allDisk;
        ASSETCONVERTER::CPackIndexResource::TResourceVector allService;
        UInt32 pagesDisk = 0;
        UInt32 pagesService = 0;
        UInt64 totalDisk = 0;
        UInt64 totalService = 0;
        ASSERT_TRUE( ReadAll( local, packX, CORE::CString::Empty, CORE::CString::Empty, 10, allDisk, pagesDisk, totalDisk ) );
        ASSERT_TRUE( ReadAll( remote, packX, CORE::CString::Empty, CORE::CString::Empty, 10, allService, pagesService, totalService ) );
        ASSERT_TRUE( 26 == allDisk.size() && pagesDisk == pagesService && totalDisk == totalService && SamePage( allDisk, allService ) );
        ASSERT_TRUE( ReadAll( local, packX, "textures", CORE::CString::Empty, 7, allDisk, pagesDisk, totalDisk ) && ReadAll( remote, packX, "textures", CORE::CString::Empty, 7, allService, pagesService, totalService ) );
        ASSERT_TRUE( 10 == allService.size() && SamePage( allDisk, allService ) && 2 == pagesService );
        ASSERT_TRUE( ReadAll( local, packX, "static-models", "props", 100, allDisk, pagesDisk, totalDisk ) && ReadAll( remote, packX, "static-models", "props", 100, allService, pagesService, totalService ) );
        ASSERT_TRUE( 14 == allService.size() && SamePage( allDisk, allService ) );

        // the record of the service says what the catalogue holds: the id of the store, and the asset the file is
        ASSERT_TRUE( !allService.empty() && !allService[ 0 ].id.IsNULLOrEmpty() && allService[ 0 ].id == ASSETSTORE::CAssetPackIndexLoader::MakeResourceId( packX, allService[ 0 ].path ) );

        // a pack the service has no index of, and one it does not know
        ASSETCONVERTER::CPackIndexDocument none;
        bool stale = false;
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == remote.GetManifest( "nopack", none, stale ) && !remote.GetLastMessage().IsNULLOrEmpty() );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == remote.GetResources( "nopack", CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty, 10, allService, none.documentHash, totalService ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Service source: the caches: the manifest is asked once, the pages are kept, the events of the service make them ask again and a document with the same index key stands; at most 32 pages; the manifest on the disk" )
    try
    {
        ASSERT_TRUE( !packX.IsNULLOrEmpty() );
        CChangeRecorder changes( service );
        CServicePackIndexSource source( service );
        source.SetDiskCacheFolder( "wbpisrc/cache" );
        ASSETCONVERTER::CPackIndexDocument manifest;
        bool stale = false;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( packX, manifest, stale ) && 1 == source.GetServiceCallCount() );
        const CORE::CString firstKey = manifest.producer.indexKey;
        ASSERT_TRUE( vfs.FileExists( "wbpisrc/cache/" + CORE::CString( "local" ) + "." + packX + ".manifest.json" ) );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( packX, manifest, stale ) && 1 == source.GetServiceCallCount() && 1 == source.GetCache().GetMemoryManifestHitCount() + 0 );

        // a page asked twice is asked of the service once
        ASSETCONVERTER::CPackIndexResource::TResourceVector page;
        CORE::CString next;
        UInt64 total = 0;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetResources( packX, "textures", CORE::CString::Empty, CORE::CString::Empty, 5, page, next, total ) && 2 == source.GetServiceCallCount() );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetResources( packX, "textures", CORE::CString::Empty, CORE::CString::Empty, 5, page, next, total ) && 2 == source.GetServiceCallCount() );
        ASSERT_TRUE( 5 == page.size() && 10 == total && 1 == source.GetCache().GetPageHitCount() );

        // the index of the pack is loaded again from the same document: the event comes, the pages go, the manifest is asked and stands
        const UInt32 updatesBefore = changes.indexUpdates;
        ASSERT_TRUE( RunIndexJob( service, packX, true ) );
        Pump( 100 );
        ASSERT_TRUE( changes.indexUpdates > updatesBefore );
        ASSERT_TRUE( 0 == source.GetCache().GetPageCount() );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( packX, manifest, stale ) && 3 == source.GetServiceCallCount() && firstKey == manifest.producer.indexKey );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( packX, manifest, stale ) && 3 == source.GetServiceCallCount() );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetResources( packX, "textures", CORE::CString::Empty, CORE::CString::Empty, 5, page, next, total ) && 4 == source.GetServiceCallCount() );

        // another document of the pack: the index key changes, the manifest follows
        CORE::CString::StringVector more = filesX;
        more.push_back( "Textures/T_Extra.png" );
        more.push_back( "pack.json" );
        ASSETCONVERTER::CPackIndexDocument bigger;
        ASSERT_TRUE( BuildDocument( more, bigger ) );
        ASSETCONVERTER::CPackIndexWriter writer;
        ASSERT_TRUE( writer.Write( bigger, "wbpisrc/main/intermediate/disk/PackX" ) );
        ASSERT_TRUE( RunIndexJob( service, packX, false ) );
        Pump( 100 );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetManifest( packX, manifest, stale ) && 27 == manifest.counts.resources && manifest.producer.indexKey != firstKey );

        // at most 32 pages are kept, the one used the longest ago goes
        for ( UInt32 size=1; size<=40; ++size )
            ASSERT_TRUE( IPackIndexSource::RESULT_OK == source.GetResources( packX, CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty, size, page, next, total ) );
        ASSERT_TRUE( CPackIndexClientCache::MaxPages == source.GetCache().GetPageCount() && source.GetCache().GetEvictionCount() >= 8 );

        // the manifest of the disk shows the pack when the service cannot be reached
        CServicePackIndexSource offline( service );
        offline.SetDiskCacheFolder( "wbpisrc/cache" );
        service.Close();
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == offline.GetManifest( packX, manifest, stale ) && offline.IsServingFromCache() && stale );
        ASSERT_TRUE( manifest.producer.indexKey != firstKey && 27 == manifest.counts.resources && 1 == offline.GetCache().GetDiskManifestHitCount() );
        ASSETCONVERTER::CPackIndexDocument unknown;
        ASSERT_TRUE( IPackIndexSource::RESULT_UNAVAILABLE == offline.GetManifest( packY, unknown, stale ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Service source: the licence gate: for a machine learning consumer a pack flagged noMachineLearning is not there, a pack without the flag is; a removed pack leaves the cache" )
    try
    {
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service.Open() && service.IsCatalogueAvailable() );
        CChangeRecorder changes( service );
        CServicePackIndexSource normal( service );
        CServicePackIndexSource learning( service );
        learning.SetLearning( true );
        ASSERT_TRUE( learning.IsLearning() && !normal.IsLearning() );
        ASSETCONVERTER::CPackIndexDocument manifest;
        bool stale = false;
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == normal.GetManifest( packX, manifest, stale ) );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == learning.GetManifest( packX, manifest, stale ) );
        ASSETCONVERTER::CPackIndexGroup::TGroupVector groups;
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == learning.GetGroups( packX, groups ) );
        ASSETCONVERTER::CPackIndexResource::TResourceVector page;
        CORE::CString next;
        UInt64 total = 0;
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == learning.GetResources( packX, CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty, 10, page, next, total ) );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == learning.GetManifest( packY, manifest, stale ) && 6 == manifest.counts.resources );
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == learning.GetResources( packY, CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty, 10, page, next, total ) && 6 == page.size() );

        // a pack that is removed leaves the cache of the source that had it
        ASSERT_TRUE( IPackIndexSource::RESULT_OK == normal.GetManifest( packY, manifest, stale ) );
        CPackIndexClientCache::SManifest cached;
        ASSERT_TRUE( normal.GetCache().FindManifest( packY, cached ) );
        ASSETSTORE::CAssetServiceResult removed;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service.RemovePack( CORE::CString::Empty, packY, false, removed ) );
        Pump( 100 );
        ASSERT_TRUE( 1 == changes.removals && !normal.GetCache().FindManifest( packY, cached ) );
        ASSERT_TRUE( IPackIndexSource::RESULT_NOT_FOUND == normal.GetManifest( packY, manifest, stale ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    service.Close();
    vfs.UnmountArchiveByName( "wbpi_pisrc" );
}

/*-------------------------------------------------------------------------*/
