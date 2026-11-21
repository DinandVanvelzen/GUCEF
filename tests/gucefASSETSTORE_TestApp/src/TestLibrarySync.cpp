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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#include "gucefASSETSTORE_CAssetStoreRemoteCodec.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#include "gucefASSETSTORE_CAssetItemRevision.h"
#define GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#endif /* GUCEF_ASSETSTORE_CASSETITEMREVISION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H
#include "gucefASSETSTORE_CAssetFormatPolicy.h"
#define GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H
#endif /* GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H
#include "gucefASSETSTORE_CAssetLibrarySyncItem.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlanner.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#include "gucefASSETSTORE_CAssetLibrarySyncAdmission.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H
#include "gucefASSETSTORE_CAssetLibrarySyncScheduleTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef GUCEF_CORE_CJOBDEFINITION_H
#include "gucefCORE_CJobDefinition.h"
#define GUCEF_CORE_CJOBDEFINITION_H
#endif /* GUCEF_CORE_CJOBDEFINITION_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTMODELANALYZER_H
#include "TestModelAnalyzer.h"
#define TESTMODELANALYZER_H
#endif /* TESTMODELANALYZER_H ? */

#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestLibrarySync.h"

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

/** The database of a run of the suites */
struct SSyncFixture
{
    CString label;
    CString base;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;
    SSyncFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeSyncStoreSettings( const SSyncFixture& fixture )
{
    CPrimaryAssetStoreSettings settings;
    settings.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    settings.locations.push_back( main );
    settings.defaultLocationId = "main";
    settings.database = fixture.database;
    settings.analysisConcurrency = 2;
    if ( GUCEF_NULL != fixture.credentials )
    {
        settings.credentialProvider = "gucefASSETSTORE.test";
        settings.credentialAccount = "catalogue";
    }
    return settings;
}

UInt64
CountSyncRows( CPrimaryAssetStore& store, const CString& table )
{
    CAssetCatalogue::TRowVector rows;
    if ( ASSETSTORE_STATUS_OK != store.GetCatalogue().SelectAll( table, rows ) )
        return 0;
    return rows.size();
}

/** A listed item of the fake library: one format per id given, each with one file whose content is the text given */
CAssetStoreItem
MakeLibraryItem( const CString& itemId, const CString& title, const CString::StringVector& formatIds, UInt64 fileSize )
{
    CAssetStoreItem item;
    item.providerId = "libstore";
    item.itemId = itemId;
    item.title = title;
    item.seller = "Library Seller";
    item.listingType = "3d-model";
    item.licenceName = "Standard License";
    for ( size_t f=0; f<formatIds.size(); ++f )
    {
        CAssetStoreFormat format;
        format.formatId = formatIds[ f ];
        format.name = formatIds[ f ].Uppercase();
        CAssetStoreFile file;
        file.fileId = itemId + "-" + formatIds[ f ];
        file.name = "SM_" + itemId + "." + formatIds[ f ];
        file.size = fileSize;
        format.files.push_back( file );
        item.formats.push_back( format );
    }
    return item;
}

/** Writes a file into a new staging folder of the store, as a finished download leaves it; answers the folder */
CString
StageDownload( CPrimaryAssetStore& store, const CString& fileName, const CString& content )
{
    const CString folder = store.MakeDownloadStagingPath( "main" );
    WriteVfsFile( folder + "/" + fileName, content );
    return folder;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLibrarySyncUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "LibrarySyncUnit" );

    GUCEF_TESTFW_TESTCASE( "Library sync: the new members of a listed item are optional on the wire, an older shape reads as the defaults, the answer of the remote service carries them" )
        try
        {
            CDataNodeSerializableSettings settings;
            CString::StringVector formats;
            formats.push_back( "fbx" );
            CAssetStoreItem plain = MakeLibraryItem( "i1", "Plain", formats, 10 );
            CDataNode plainNode( "item", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( plain.Serialize( plainNode, settings ) );
            // a record with the defaults has none of the new members: an older reader sees what it knows
            ASSERT_TRUE( GUCEF_NULL == plainNode.GetAttribute( "changedAtMs" ) && GUCEF_NULL == plainNode.GetAttribute( "downloadable" ) && GUCEF_NULL == plainNode.GetAttribute( "formatsLevel" ) );
            CAssetStoreItem plainBack;
            plainBack.downloadable = false;
            plainBack.changedAtMs = 5;
            ASSERT_TRUE( plainBack.Deserialize( plainNode, settings ) );
            ASSERT_TRUE( 0 == plainBack.changedAtMs && plainBack.downloadable && plainBack.revision.IsNULLOrEmpty() && 0 == plainBack.formatsLevel && 0 == plainBack.acquiredAtMs );
            ASSERT_TRUE( 2 == plainBack.GetEffectiveFormatsLevel() );

            CAssetStoreItem full = plain;
            full.changedAtMs = 1731000000123LL;
            full.revision = "etag-7";
            full.acquiredAtMs = 1600000000000LL;
            full.downloadable = false;
            full.unavailableReason = "key only";
            full.formatsLevel = 1;
            CDataNode fullNode( "item", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( full.Serialize( fullNode, settings ) );
            CAssetStoreItem fullBack;
            ASSERT_TRUE( fullBack.Deserialize( fullNode, settings ) );
            ASSERT_TRUE( 1731000000123LL == fullBack.changedAtMs && "etag-7" == fullBack.revision && 1600000000000LL == fullBack.acquiredAtMs );
            ASSERT_TRUE( !fullBack.downloadable && "key only" == fullBack.unavailableReason && 1 == fullBack.formatsLevel && 1 == fullBack.GetEffectiveFormatsLevel() );

            // through the codec of the remote service as the answer of Stores/GetItem
            COMCORE::CRpcContractPtr control = CAssetStoreRemoteContracts::GetControlContract();
            ASSERT_FALSE( control.IsNULL() );
            if ( !control.IsNULL() )
            {
                COMCORE::CRpcMessage answer( control, control->FindType( "CallAnswer" ) );
                CAssetStoreRemoteCodec::FillAnswer( answer, ASSETSTORE_STATUS_OK, CString::Empty, &full, 3, false );
                CIDataNodeSerializable* record = GUCEF_NULL;
                CString message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == CAssetStoreRemoteCodec::ReadAnswer( CAssetServiceOperations::GetStoreItem, answer, record, message ) );
                const CAssetStoreItem* remote = CICloneable::CloneableAs< CAssetStoreItem >( record );
                ASSERT_TRUE( GUCEF_NULL != remote && 1731000000123LL == remote->changedAtMs && "etag-7" == remote->revision && !remote->downloadable && 1 == remote->formatsLevel );
                GUCEF_DELETE record;
            }
            // the level of formats without files and without sizes
            CAssetStoreItem noFormats;
            ASSERT_TRUE( 0 == noFormats.GetEffectiveFormatsLevel() );
            CAssetStoreItem noSizes = MakeLibraryItem( "i2", "No sizes", formats, 0 );
            ASSERT_TRUE( 1 == noSizes.GetEffectiveFormatsLevel() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: the revision ladder picks the first basis with a value and tags it; values of other bases are not compared; a time is stale only when newer" )
        try
        {
            CString::StringVector formats;
            formats.push_back( "fbx" );
            CAssetStoreItem item = MakeLibraryItem( "i1", "Ladder", formats, 100 );
            // formats with sizes: the formats fingerprint
            CString tagged = CAssetItemRevision::GetEffectiveRevision( item );
            ASSERT_TRUE( CAssetItemRevision::FormatBasis == CAssetItemRevision::GetBasis( tagged ) && 64 == CAssetItemRevision::GetValue( tagged ).Length() );
            item.revision = "r1";
            ASSERT_TRUE( "rev:r1" == CAssetItemRevision::GetEffectiveRevision( item ) );
            item.version = "2.1";
            ASSERT_TRUE( "ver:2.1" == CAssetItemRevision::GetEffectiveRevision( item ) );
            item.changedAtMs = 1000;
            ASSERT_TRUE( "ts:1000" == CAssetItemRevision::GetEffectiveRevision( item ) && 1000 == CAssetItemRevision::GetChangedMs( "ts:1000" ) );
            // no sizes, no version, no time: the listing fingerprint; no id at all: unknown
            CAssetStoreItem bare = MakeLibraryItem( "i3", "Bare", formats, 0 );
            ASSERT_TRUE( CAssetItemRevision::ListingBasis == CAssetItemRevision::GetBasis( CAssetItemRevision::GetEffectiveRevision( bare ) ) );
            CAssetStoreItem nothing;
            ASSERT_TRUE( "unk:" == CAssetItemRevision::GetEffectiveRevision( nothing ) );

            ASSERT_TRUE( CAssetItemRevision::COMPARISON_SAME == CAssetItemRevision::Compare( "ts:1000", "ts:1000" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_STALE == CAssetItemRevision::Compare( "ts:1000", "ts:2000" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_SAME == CAssetItemRevision::Compare( "ts:2000", "ts:1000" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_STALE == CAssetItemRevision::Compare( "ver:1", "ver:2" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_SAME == CAssetItemRevision::Compare( "fmt:ab", "fmt:ab" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_CHECK_FURTHER == CAssetItemRevision::Compare( "lst:ab", "lst:cd" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_OTHER_BASIS == CAssetItemRevision::Compare( "ver:1", "ts:1" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_UNKNOWN == CAssetItemRevision::Compare( "unk:", "unk:" ) );
            ASSERT_TRUE( CAssetItemRevision::COMPARISON_UNKNOWN == CAssetItemRevision::Compare( CString::Empty, "ts:1" ) );
            ASSERT_TRUE( CString( "stale" ) == CAssetItemRevision::ComparisonToString( CAssetItemRevision::COMPARISON_STALE ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: the fingerprints are stable against what they leave out and change with what they hold" )
        try
        {
            CString::StringVector formats;
            formats.push_back( "fbx" );
            formats.push_back( "obj" );
            CAssetStoreItem item = MakeLibraryItem( "i1", "Print", formats, 100 );
            const CString listing = CAssetItemRevision::ComputeListingFingerprint( item );
            const CString files = CAssetItemRevision::ComputeFormatFingerprint( item );
            // images, descriptions, addresses and the order of the formats are not in either
            CAssetStoreItem cosmetic = item;
            cosmetic.description = "a new description";
            cosmetic.thumbnailUrl = "https://example.invalid/new.png";
            cosmetic.imageUrls.push_back( "https://example.invalid/other.png" );
            cosmetic.url = "https://example.invalid/item?session=1";
            CAssetStoreFormat first = cosmetic.formats[ 0 ];
            cosmetic.formats[ 0 ] = cosmetic.formats[ 1 ];
            cosmetic.formats[ 1 ] = first;
            ASSERT_TRUE( listing == CAssetItemRevision::ComputeListingFingerprint( cosmetic ) && files == CAssetItemRevision::ComputeFormatFingerprint( cosmetic ) );
            // the title changes the listing print, not the files print; a new file size changes the files print
            CAssetStoreItem retitled = item;
            retitled.title = "Print 2";
            ASSERT_TRUE( listing != CAssetItemRevision::ComputeListingFingerprint( retitled ) && files == CAssetItemRevision::ComputeFormatFingerprint( retitled ) );
            CAssetStoreItem reuploaded = item;
            reuploaded.formats[ 1 ].files[ 0 ].size = 101;
            ASSERT_TRUE( files != CAssetItemRevision::ComputeFormatFingerprint( reuploaded ) );
            // no sizes: no files print
            ASSERT_TRUE( CAssetItemRevision::ComputeFormatFingerprint( MakeLibraryItem( "i2", "x", formats, 0 ) ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: every format of an item is wanted, the preferred ones first, engine exports last; only the preferred ones when asked" )
        try
        {
            CString::StringVector formats;
            formats.push_back( "ue:5.3" );
            formats.push_back( "blend" );
            formats.push_back( "obj" );
            formats.push_back( "gltf" );
            formats.push_back( "obj" );
            CAssetStoreItem item = MakeLibraryItem( "i1", "Formats", formats, 10 );
            CAssetFormatPolicy policy;
            ASSERT_TRUE( 5 == policy.preference.size() && "gltf" == policy.preference[ 0 ] && !policy.onlyPreferred );
            CString::StringVector ranked;
            policy.RankFormats( item, ranked );
            ASSERT_TRUE( 4 == ranked.size() && "gltf" == ranked[ 0 ] && "obj" == ranked[ 1 ] && "blend" == ranked[ 2 ] && "ue:5.3" == ranked[ 3 ] );
            policy.onlyPreferred = true;
            policy.RankFormats( item, ranked );
            ASSERT_TRUE( 2 == ranked.size() && "gltf" == ranked[ 0 ] && "obj" == ranked[ 1 ] );
            CDataNode node( "formatPolicy", GUCEF_DATATYPE_OBJECT );
            CDataNodeSerializableSettings settings;
            ASSERT_TRUE( policy.Serialize( node, settings ) );
            CAssetFormatPolicy back;
            ASSERT_TRUE( back.Deserialize( node, settings ) && back.onlyPreferred && back.preference == policy.preference );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: the spec travels as the arguments of the pipeline request; defaults of the profile; what is not valid says why" )
        try
        {
            CAssetLibrarySyncSpec spec;
            ASSERT_TRUE( CAssetLibrarySyncSpec::ModeRun == spec.mode && !spec.retryFailed && 0 == spec.maxItems );
            ASSERT_TRUE( !spec.profile.enabled && CAssetLibrarySyncProfile::OnStaleKeep == spec.profile.onStale && CAssetLibrarySyncProfile::OrderNewest == spec.profile.order );
            ASSERT_TRUE( 5 == spec.profile.reservePercent && 20ULL * 1024 * 1024 * 1024 == spec.profile.reserveBytes && 2.0 == spec.profile.diskFactor && 5 == spec.profile.maxSyncAttempts );
            CString reason;
            ASSERT_FALSE( spec.IsValid( reason ) );
            ASSERT_FALSE( reason.IsNULLOrEmpty() );
            spec.storeId = "SomeStore";
            spec.mode = CAssetLibrarySyncSpec::ModePlan;
            spec.retryFailed = true;
            spec.itemIds.push_back( "a" );
            spec.profile.reserveBytes = 7;
            spec.profile.order = CAssetLibrarySyncProfile::OrderSmallest;
            spec.profile.formats.onlyPreferred = true;
            ASSERT_TRUE( spec.IsValid( reason ) );
            CAssetPipelineRequest request;
            spec.ToRequest( request );
            ASSERT_TRUE( CAssetPipelineOperations::SyncLibrary == request.operation && CAssetPipelineOperations::IsJob( request.operation ) && CAssetPipelineOperations::IsServiceJob( request.operation ) );
            ASSERT_TRUE( CAssetPipelineOperations::IsServiceJob( CAssetPipelineOperations::IndexPack ) && !CAssetPipelineOperations::IsServiceJob( CAssetPipelineOperations::ConvertPack ) );
            // the request crosses the wire as its DataNode form
            CDataNodeSerializableSettings settings;
            CDataNode node( "request", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( request.Serialize( node, settings ) );
            CAssetPipelineRequest received;
            ASSERT_TRUE( received.Deserialize( node, settings ) );
            CAssetLibrarySyncSpec back = CAssetLibrarySyncSpec::FromRequest( received );
            ASSERT_TRUE( "somestore" == back.storeId && back.IsPlanOnly() && back.retryFailed && 1 == back.itemIds.size() && 7 == back.profile.reserveBytes );
            ASSERT_TRUE( CAssetLibrarySyncProfile::OrderSmallest == back.profile.order && back.profile.formats.onlyPreferred );
            // the store id of the request when the arguments have none
            CAssetPipelineRequest bare;
            bare.operation = CAssetPipelineOperations::SyncLibrary;
            bare.storeId = "OtherStore";
            ASSERT_TRUE( "otherstore" == CAssetLibrarySyncSpec::FromRequest( bare ).storeId && CAssetLibrarySyncSpec::ModeRun == CAssetLibrarySyncSpec::FromRequest( bare ).mode );
            back.profile.onStale = "replace";
            ASSERT_FALSE( back.IsValid( reason ) );
            ASSERT_TRUE( reason.HasSubstr( "replace" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: a listing of 20000 items in pages of 100, with a repeated page, is reduced and planned in bounded time; the plan is the same for the same inputs" )
        try
        {
            CAssetLibrarySyncSpec spec;
            spec.storeId = "scale";
            spec.mode = CAssetLibrarySyncSpec::ModePlan;
            CString::StringVector formats;
            formats.push_back( "fbx" );
            formats.push_back( "obj" );
            const UInt32 itemCount = 20000;
            const UInt64 started = MT::PrecisionTickCount();
            CAssetLibrarySyncPlanner planner;
            planner.Reset( spec );
            for ( UInt32 first=0; first<itemCount; first+=100 )
            {
                CAssetStoreLibraryPage page;
                for ( UInt32 i=first; i<first+100; ++i )
                {
                    CAssetStoreItem item = MakeLibraryItem( "s" + ToString( i ), "Scale " + ToString( i ), formats, 1000 + i );
                    item.changedAtMs = 1600000000000LL + ( i % 977 ) * 1000;
                    item.description = "a description of some length that the reduction drops at once, as it drops the pictures";
                    page.items.push_back( item );
                }
                planner.AddListingPage( page );
                if ( 0 == first )
                    planner.AddListingPage( page );       // a store that repeats a page: the items count once
            }
            planner.SetListingComplete( true );
            CAssetLibrarySyncPlan plan;
            CAssetLibrarySyncPlanner::TWorkItemVector worklist;
            planner.BuildPlan( 1800000000000LL, plan, worklist );
            const UInt64 ticks = MT::PrecisionTickCount() - started;
            const UInt64 ms = ( ticks * 1000 ) / MT::PrecisionTimerResolution();
            GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySync: reduced and planned " << itemCount << " listed items (" << worklist.size() << " downloads) in " << ms << " ms" );
            ASSERT_TRUE( itemCount == plan.listed && itemCount == plan.GetCount( CAssetLibrarySyncItem::Missing ) && 2 * itemCount == worklist.size() && 2 * itemCount == plan.queuedDownloads );
            ASSERT_TRUE( ms < 60000 );
            // newest first, the formats of an item in their order
            ASSERT_TRUE( worklist[ 0 ].changedAtMs >= worklist[ worklist.size() - 1 ].changedAtMs && "fbx" == worklist[ 0 ].formatId && "obj" == worklist[ 1 ].formatId && worklist[ 0 ].itemId == worklist[ 1 ].itemId );
            CAssetLibrarySyncPlan again;
            CAssetLibrarySyncPlanner::TWorkItemVector worklistAgain;
            planner.BuildPlan( 1800000000000LL, again, worklistAgain );
            bool same = worklistAgain.size() == worklist.size();
            for ( size_t w=0; w<worklist.size() && same; ++w )
                same = worklist[ w ].itemId == worklistAgain[ w ].itemId && worklist[ w ].formatId == worklistAgain[ w ].formatId;
            ASSERT_TRUE( same );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: the free space of the library mount named by GUCEF_TEST_LIBRARY_MOUNT (a network share such as CIFS) is read, read-only" )
        const char* mount = getenv( "GUCEF_TEST_LIBRARY_MOUNT" );
        if ( GUCEF_NULL == mount || '\0' == mount[ 0 ] )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_LIBRARY_MOUNT is not set" );
        try
        {
            UInt64 freeBytes = 0;
            UInt64 totalBytes = 0;
            const bool read = CAssetLibrarySyncAdmission::ReadVolume( mount, freeBytes, totalBytes );
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "LibrarySync: the volume of " << mount << ": read " << read << ", " << freeBytes << " bytes free of " << totalBytes );
            ASSERT_TRUE( read && totalBytes > 0 && freeBytes <= totalBytes );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Library sync: admission: the window, the ingest backlog, the caps from the ledger, the disk rule and the free space of a real volume" )
        try
        {
            CAssetLibrarySyncProfile profile;
            profile.reservePercent = 0;
            profile.reserveBytes = 0;
            profile.maxInFlightPerStore = 0;
            profile.maxPendingIngests = 2;
            profile.maxStagedBytes = 1000;
            profile.maxItemsPerHour = 3;
            CAssetLibrarySyncAdmission admission;
            admission.Configure( profile, 2 );
            ASSERT_TRUE( 4 == admission.GetWindow() && 4 == admission.GetConfiguredWindow() );
            admission.Configure( profile, 3 );
            ASSERT_TRUE( 6 == admission.GetWindow() );
            // the rate limit halves the window, 20 successes grow it by one
            admission.NoteRateLimited();
            ASSERT_TRUE( 3 == admission.GetWindow() );
            admission.NoteRateLimited();
            admission.NoteRateLimited();
            admission.NoteRateLimited();
            ASSERT_TRUE( 1 == admission.GetWindow() );
            for ( UInt32 i=0; i<20; ++i )
                admission.NoteSuccess();
            ASSERT_TRUE( 2 == admission.GetWindow() );
            admission.Configure( profile, 2 );

            const Int64 now = 1800000000000LL;
            CAssetLibrarySyncAdmission::SLoad load;
            CString reason;
            bool waiting = true;
            // no volume: the disk rule is skipped
            ASSERT_TRUE( admission.Admit( 10, load, now, reason, waiting ) && reason.IsNULLOrEmpty() && !waiting );
            load.inFlightItems = 4;
            ASSERT_FALSE( admission.Admit( 10, load, now, reason, waiting ) );
            ASSERT_TRUE( !waiting && reason.HasSubstr( "window" ) >= 0 );
            load.inFlightItems = 2;
            load.pendingIngests = 2;
            ASSERT_FALSE( admission.Admit( 10, load, now, reason, waiting ) );
            ASSERT_TRUE( reason.HasSubstr( "ingest" ) >= 0 );
            load.pendingIngests = 1;
            load.stagedBytes = 995;
            ASSERT_FALSE( admission.Admit( 10, load, now, reason, waiting ) );
            load.stagedBytes = 0;
            for ( UInt32 i=0; i<3; ++i )
                admission.NoteQueued( now - 1000, 10 );
            ASSERT_FALSE( admission.Admit( 10, load, now, reason, waiting ) );
            ASSERT_TRUE( waiting && reason.HasSubstr( "per hour" ) >= 0 );
            // an hour later the cap allows it again
            ASSERT_TRUE( admission.Admit( 10, load, now + 60 * 60 * 1000, reason, waiting ) );
            ASSERT_TRUE( 3 == admission.GetLedger().size() );

            // the disk rule: free - ( inFlight + item ) x factor >= reserve
            ASSERT_TRUE( CAssetLibrarySyncAdmission::FitsOnDisk( 1000, 100, 100, 2.0, 600 ) );
            ASSERT_FALSE( CAssetLibrarySyncAdmission::FitsOnDisk( 1000, 100, 101, 2.0, 600 ) );
            profile.reservePercent = 5;
            profile.reserveBytes = 10;
            admission.Configure( profile, 2 );
            ASSERT_TRUE( 50 == admission.GetReserveBytes( 1000 ) && 10 == admission.GetReserveBytes( 100 ) );

            // the free space of the volume of the folder of the tests, through gucefCORE
            const CString folder = TestDirectory( "ls_volume" );
            CreateDirs( folder );
            UInt64 freeBytes = 0;
            UInt64 totalBytes = 0;
            ASSERT_TRUE( CAssetLibrarySyncAdmission::ReadVolume( folder, freeBytes, totalBytes ) && freeBytes > 0 && totalBytes >= freeBytes );
            GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySync: the volume of " << folder << " has " << freeBytes << " of " << totalBytes << " bytes free" );
            CStorageVolumeInformation info;
            ASSERT_TRUE( GetFileSystemStorageVolumeInformationByDirPath( info, folder ) && info.hasFreeBytesAvailableToCaller && info.freeBytesAvailableToCaller > 0 );
            ASSERT_FALSE( CAssetLibrarySyncAdmission::ReadVolume( CString::Empty, freeBytes, totalBytes ) );
            // a reserve above the free space refuses with the reason, as a waiting state
            profile.reservePercent = 0;
            profile.reserveBytes = info.freeBytesAvailableToCaller + 1024ULL * 1024 * 1024;
            profile.maxItemsPerHour = 0;
            admission.Configure( profile, 2 );
            admission.SetVolumePath( folder );
            CAssetLibrarySyncAdmission::SLoad idle;
            ASSERT_FALSE( admission.Admit( 1, idle, now, reason, waiting ) );
            ASSERT_TRUE( waiting && reason.HasSubstr( "disk space" ) >= 0 );
            // an item larger than the free space, with no reserve
            profile.reserveBytes = 0;
            admission.Configure( profile, 2 );
            admission.SetVolumePath( folder );
            ASSERT_FALSE( admission.Admit( info.freeBytesAvailableToCaller, idle, now, reason, waiting ) );
            ASSERT_TRUE( admission.Admit( 1, idle, now, reason, waiting ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Migration 5, the sync tables and the facts and dedupe of the ingest, on the database of the fixture */
/*-------------------------------------------------------------------------*/


/** Writes one row of store facts on a thread of its own */
class CFactsWriter : public MT::CActiveObject
{
    public:
    CAssetSyncMarkStore& syncStore;
    CAssetPackSourceRecord record;
    MT::CTAtomic< UInt32 > done;
    TAssetStoreStatus status;
    mutable MT::CMutex lock;
    CFactsWriter( CAssetSyncMarkStore& store, const CAssetPackSourceRecord& facts )
        : MT::CActiveObject(), syncStore( store ), record( facts ), done( 0 ), status( ASSETSTORE_STATUS_FAILED ), lock() {}
    virtual ~CFactsWriter() { Deactivate( false, true ); }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return this; }
    virtual MT::TLockStatus Lock( UInt32 timeoutInMs ) const GUCEF_VIRTUAL_OVERRIDE { return lock.Lock( timeoutInMs ); }
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE { return lock.Unlock(); }
    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            status = syncStore.WritePackSource( record );
        }
        catch ( ... )
        {
            status = ASSETSTORE_STATUS_FAILED;
        }
        done.Store( 1 );
        return true;
    }
};

/**
 *  In an event of the pulse generator (its thread holds the lock of the generator's notifications) a write of the same table
 *  runs on another thread and is waited for: the writer must not need the lock of the pulse generator under the write lock of
 *  the table (the lock order fault found by the restart tests: a write that reads a partition made a scan that subscribed to
 *  the pulse generator, while the thread of that generator waited for the write lock in a write of its own)
 */
class CWriteInEvent : public CTSGNotifier
{
    public:
    typedef CTEventHandlerFunctor< CWriteInEvent > TEventCallback;
    CAssetSyncMarkStore& syncStore;
    CAssetPackSourceRecord record;
    bool ran;
    bool finishedInTime;
    TAssetStoreStatus status;
    CWriteInEvent( CAssetSyncMarkStore& store, const CAssetPackSourceRecord& facts, CTimer& timer )
        : CTSGNotifier( CCoreGlobal::Instance()->GetPulseGenerator() ), syncStore( store ), record( facts ), ran( false ), finishedInTime( false ), status( ASSETSTORE_STATUS_FAILED )
    {
        TEventCallback callback( this, &CWriteInEvent::OnTimer );
        SubscribeTo( &timer, CTimer::TimerUpdateEvent, callback );
    }
    virtual ~CWriteInEvent() { SignalUpcomingDestruction(); UnsubscribeAllFromObserver(); }
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CString name = "CWriteInEvent"; return name; }
    void OnTimer( CNotifier*, const CEvent&, CICloneable* )
    {
        if ( ran )
            return;
        ran = true;
        CFactsWriter writer( syncStore, record );
        writer.Activate();
        for ( UInt32 waited=0; waited<15000 && 0 == writer.done.Load(); waited+=5 )
            MT::PrecisionDelay( 5 );
        finishedInTime = 0 != writer.done.Load();
        status = writer.status;
        // the writer is given the time it needs before it goes (without the fix it ends after the lock timeout)
        while ( 0 == writer.done.Load() )
            MT::PrecisionDelay( 5 );
    }
};

/*-------------------------------------------------------------------------*/

void
RunLibrarySyncStoreSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_source", base + "/source" );

    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    const CString diskPack = base + "/source/Barrels";
    bool written = WriteVfsFile( diskPack + "/SM_Barrel_01.fbx", CString( "triangles=400;vertices=300" ) );
    written = WriteVfsFile( diskPack + "/SM_Barrel_02.fbx", CString( "triangles=450;vertices=320" ) ) && written;

    CPrimaryAssetStore store;
    store.SetSettings( MakeSyncStoreSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    CString diskPackId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": migration 4 to 5 on a populated catalogue adds the tables of the library sync, empty, and keeps the rows" ).C_String() )
        try
        {
            ASSERT_TRUE( written );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open( 4 ) );
            ASSERT_TRUE( 4 == store.GetCatalogue().GetSchemaVersion() && !store.GetSyncStore().IsAvailable() );
            CAssetCatalogue::TRowVector probe;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != store.GetCatalogue().SelectAll( "pack_sources", probe ) );
            TAssetPackSourceRecordVector none;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == store.GetSyncStore().GetPackSourcesOfStore( "libstore", none ) );

            CAssetIngestRequest request;
            request.sourceId = "disk";
            request.contentPath = diskPack;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( request, result ) && 2 == result.assetCount );
            diskPackId = result.packId;

            store.Close();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( 5 == store.GetCatalogue().GetSchemaVersion() && 5 == CAssetCatalogue::CurrentSchemaVersion && 5 == CountSyncRows( store, "schema_version" ) );
            ASSERT_TRUE( store.GetSyncStore().IsAvailable() );
            ASSERT_TRUE( 0 == CountSyncRows( store, "pack_sources" ) && 0 == CountSyncRows( store, "sync_marks" ) && 0 == CountSyncRows( store, "sync_items" ) );
            CAssetPackRecord pack;
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( diskPackId, pack ) && ASSETSTORE_STATUS_OK == store.GetAssets( diskPackId, assets ) && 2 == assets.size() );
            // a pack from disk has no store facts
            CAssetPackSourceRecord source;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == store.GetSyncStore().GetPackSource( diskPackId, source ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the sync tables: store facts by store and item, supersede within a format, marks, observations; a removed pack takes its facts" ).C_String() )
        try
        {
            CAssetSyncMarkStore& sync = store.GetSyncStore();
            CAssetPackSourceRecord a;
            a.packId = "pk-a";
            a.sourceId = "libstore";
            a.sourceItemId = "item-1";
            a.sourceFormatId = "fbx";
            a.revision = "ts:1000";
            a.changedMs = 1000;
            a.syncedMs = 1;
            CAssetPackSourceRecord b = a;
            b.packId = "pk-b";
            b.sourceFormatId = "obj";
            CAssetPackSourceRecord c = a;
            c.packId = "pk-c";
            c.sourceItemId = "item-2";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.WritePackSource( a ) && ASSETSTORE_STATUS_OK == sync.WritePackSource( b ) && ASSETSTORE_STATUS_OK == sync.WritePackSource( c ) );
            TAssetPackSourceRecordVector ofStore;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetPackSourcesOfStore( "LibStore", ofStore ) && 3 == ofStore.size() );
            TAssetPackSourceRecordVector ofItem;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetPackSourcesOfItem( "libstore", "item-1", ofItem ) && 2 == ofItem.size() );
            // a new fbx pack of item-1 supersedes the fbx pack only
            CAssetPackSourceRecord d = a;
            d.packId = "pk-d";
            d.revision = "ts:2000";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.WritePackSource( d ) );
            UInt32 superseded = 0;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.SupersedeOlderPacks( d, 5000, superseded ) && 1 == superseded );
            CAssetPackSourceRecord back;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetPackSource( "pk-a", back ) && "pk-d" == back.supersededBy && 5000 == back.supersededMs && !back.IsCurrent() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetPackSource( "pk-b", back ) && back.IsCurrent() && "ts:1000" == back.revision );
            ASSERT_TRUE( back.MatchesFormat( "obj" ) && !back.MatchesFormat( "fbx" ) );

            CAssetSyncMark mark;
            mark.sourceId = "libstore";
            mark.itemId = "item-9";
            mark.mark = CAssetSyncMark::Ignore;
            mark.reason = "not wanted";
            mark.markedMs = 7;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.SetMark( mark ) );
            mark.mark = "something";
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == sync.SetMark( mark ) );
            TAssetSyncMarkVector marks;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetMarks( "libstore", marks ) && 1 == marks.size() && CAssetSyncMark::Ignore == marks[ 0 ].mark && "not wanted" == marks[ 0 ].reason );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.RemoveMark( "libstore", "item-9" ) && ASSETSTORE_STATUS_OK == sync.GetMarks( "libstore", marks ) && marks.empty() );

            TAssetSyncItemRecordVector observed( 2 );
            observed[ 0 ].sourceId = "libstore";
            observed[ 0 ].itemId = "item-1";
            observed[ 0 ].attempts = 3;
            observed[ 0 ].nextAttemptMs = 99;
            observed[ 0 ].lastStatus = "FAILED";
            observed[ 1 ].sourceId = "libstore";
            observed[ 1 ].itemId = "item-2";
            observed[ 1 ].available = false;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.WriteSyncItems( observed ) );
            TAssetSyncItemRecordMap items;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.GetSyncItems( "libstore", items ) && 2 == items.size() );
            ASSERT_TRUE( 3 == items[ "item-1" ].attempts && 99 == items[ "item-1" ].nextAttemptMs && "FAILED" == items[ "item-1" ].lastStatus && !items[ "item-2" ].available );

            // the facts of a pack go with its rows when the pack is removed
            CAssetPackSourceRecord ofDisk = a;
            ofDisk.packId = diskPackId;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == sync.WritePackSource( ofDisk ) && ASSETSTORE_STATUS_OK == sync.GetPackSource( diskPackId, back ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.RemovePack( diskPackId, false ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == sync.GetPackSource( diskPackId, back ) );
            // clean up the made up rows
            store.GetCatalogue().DeleteWhereEquals( "pack_sources", "source_id", CVariant( "libstore" ) );
            store.GetCatalogue().DeleteWhereEquals( "sync_items", "source_id", CVariant( "libstore" ) );
            ASSERT_TRUE( 0 == CountSyncRows( store, "pack_sources" ) && 0 == CountSyncRows( store, "sync_items" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the dedupe of the ingest by item knows formats and revisions: a second format and a newer revision are new packs, the old one superseded; a manual download of the same version stays a duplicate" ).C_String() )
        try
        {
            // before the store facts (the characterisation of the earlier behaviour, kept as the case of a manual download):
            // a download of an item without a format or an expected revision that has the version of a pack is a duplicate
            CString::StringVector fbx;
            fbx.push_back( "fbx" );
            CAssetIngestRequest first;
            first.sourceId = "libstore";
            first.itemId = "char-1";
            first.item = MakeLibraryItem( "char-1", "Characterised Crate", fbx, 24 );
            first.item.changedAtMs = 1000;
            first.keep = "original";
            first.formatId = "fbx";
            first.expectedRevision = "ts:1000";
            first.syncJobId = "pipeline:test";
            first.contentPath = StageDownload( store, "SM_Char.fbx", "triangles=96;vertices=64" );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( first, result ) && !result.packId.IsNULLOrEmpty() );
            CAssetPackSourceRecord facts;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( result.packId, facts ) );
            ASSERT_TRUE( "libstore" == facts.sourceId && "char-1" == facts.sourceItemId && "fbx" == facts.sourceFormatId && "ts:1000" == facts.revision );
            ASSERT_TRUE( 1000 == facts.changedMs && 64 == facts.contentDigest.Length() && "pipeline:test" == facts.syncJobId && facts.IsCurrent() && facts.syncedMs > 0 );

            CAssetIngestRequest manual = first;
            manual.formatId.Clear();
            manual.expectedRevision.Clear();
            manual.syncJobId.Clear();
            manual.contentPath = StageDownload( store, "SM_Char.fbx", "triangles=99;vertices=67" );
            CAssetIngestResult manualResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == store.Ingest( manual, manualResult ) && result.packId == manualResult.duplicateOfPackId );

            // another format of the same item, other files: a pack of its own
            CAssetIngestRequest second = first;
            second.formatId = "obj";
            second.contentPath = StageDownload( store, "SM_Char.obj", "triangles=97;vertices=65" );
            CAssetIngestResult secondResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( second, secondResult ) && secondResult.packId != result.packId );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( result.packId, facts ) && facts.IsCurrent() );

            // the item changed at the store (a newer time, no version), other files: a new pack, the old fbx pack superseded and kept
            CAssetIngestRequest changed = first;
            changed.item.changedAtMs = 2000;
            changed.expectedRevision = "ts:2000";
            changed.contentPath = StageDownload( store, "SM_Char.fbx", "triangles=98;vertices=66" );
            CAssetIngestResult changedResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Ingest( changed, changedResult ) && !changedResult.packId.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( result.packId, facts ) && changedResult.packId == facts.supersededBy && facts.supersededMs > 0 );
            CAssetPackRecord oldPack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, oldPack ) );
            CAssetPackSourceRecord objFacts;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( secondResult.packId, objFacts ) && objFacts.IsCurrent() );

            // the same revision again from a sync: a duplicate of the current pack of the format
            CAssetIngestRequest again = changed;
            again.contentPath = StageDownload( store, "SM_Char.fbx", "triangles=100;vertices=68" );
            CAssetIngestResult againResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == store.Ingest( again, againResult ) && changedResult.packId == againResult.duplicateOfPackId );

            // a newer time whose files are those of the current pack: the duplicate confirms the revision of that pack (no second pack)
            CAssetIngestRequest touched = changed;
            touched.item.changedAtMs = 3000;
            touched.expectedRevision = "ts:3000";
            touched.contentPath = StageDownload( store, "SM_Char.fbx", "triangles=98;vertices=66" );
            CAssetIngestResult touchedResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_DUPLICATE == store.Ingest( touched, touchedResult ) && changedResult.packId == touchedResult.duplicateOfPackId );
            CAssetPackSourceRecord confirmed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSource( changedResult.packId, confirmed ) && "ts:3000" == confirmed.revision && 3000 == confirmed.changedMs && confirmed.IsCurrent() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a write of the store facts on another thread, waited for in an event of the pulse generator, does not wait for the pulse generator" ).C_String() )
        try
        {
            ASSERT_TRUE( store.IsOpen() );
            if ( store.IsOpen() )
            {
                // a partition of the store exists, so the next write of it reads it first
                CAssetPackSourceRecord first;
                first.packId = "lockorder-1";
                first.sourceId = "lockorder";
                first.sourceItemId = "item-1";
                first.revision = "ver:1";
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().WritePackSource( first ) );
                CAssetPackSourceRecord second = first;
                second.packId = "lockorder-2";
                second.sourceItemId = "item-2";
                CTimer timer( CCoreGlobal::Instance()->GetPulseGenerator(), 10 );
                CWriteInEvent inEvent( store.GetSyncStore(), second, timer );
                timer.SetEnabled( true );
                for ( UInt32 waited=0; waited<5000 && !inEvent.ran; waited+=10 )
                    PumpFor( 10 );
                timer.SetEnabled( false );
                ASSERT_TRUE( inEvent.ran && inEvent.finishedInTime && ASSETSTORE_STATUS_OK == inEvent.status );
                TAssetPackSourceRecordVector facts;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetSyncStore().GetPackSourcesOfStore( "lockorder", facts ) && 2 == facts.size() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.Close();
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

}

/*-------------------------------------------------------------------------*/

namespace {

const char* const SyncStoreId = "synclib";

/** A listed item of the fake library with the content of its files on the server: one file per format, content of the seed */
CAssetStoreItem
AddSyncItem( CFakeStoreServer& server, UInt32 index, UInt32 formatCount, Int64 changedAtMs, UInt32 seed, bool add )
{
    const CString itemId = "lib-" + CString( index < 10 ? "0" : "" ) + ToString( index );
    CAssetStoreItem item;
    item.providerId = SyncStoreId;
    item.itemId = itemId;
    item.title = "Library Item " + ToString( index );
    item.seller = index < 10 ? CString( "Library Seller" ) : CString( "Library Seller, added later" );
    item.listingType = "3d-model";
    item.licenceName = 5 == index ? CString::Empty : CString( "Standard License" );
    item.changedAtMs = changedAtMs;
    static const char* const formatIds[] = { "fbx", "obj" };
    for ( UInt32 f=0; f<formatCount && f<2; ++f )
    {
        CDynamicBuffer content;
        const CString text = "triangles=" + ToString( 1000 + index * 10 + f + seed * 1000 ) + ";vertices=" + ToString( 500 + index + seed );
        content.Append( text.C_String(), text.Length() );
        const CString fileId = itemId + "-" + formatIds[ f ];
        server.SetFileContent( fileId, content );
        CAssetStoreFormat format;
        format.formatId = formatIds[ f ];
        format.name = CString( formatIds[ f ] ).Uppercase();
        CAssetStoreFile file;
        file.fileId = fileId;
        file.name = "SM_Lib_" + ToString( index ) + "." + formatIds[ f ];
        file.size = content.GetDataSize();
        file.digestAlgorithm = "sha256";
        file.digest = Sha256Of( content );
        format.files.push_back( file );
        item.formats.push_back( format );
    }
    if ( add )
        server.AddItem( item );
    else
        server.ReplaceItem( item );
    return item;
}

CAssetLibrarySyncSpec
MakeSyncSpec( bool plan )
{
    CAssetLibrarySyncSpec spec;
    spec.storeId = SyncStoreId;
    spec.mode = plan ? CAssetLibrarySyncSpec::ModePlan : CAssetLibrarySyncSpec::ModeRun;
    spec.pageSize = 3;
    // the folder of the tests is small: no reserve, the window of three items
    spec.profile.reservePercent = 0;
    spec.profile.reserveBytes = 0;
    spec.profile.maxInFlightPerStore = 3;
    spec.profile.indexAfter = false;
    return spec;
}

TAssetStoreStatus
StartSync( CIAssetStoreService& service, const CAssetLibrarySyncSpec& spec, CAssetServiceResult& started )
{
    CAssetPipelineRequest request;
    spec.ToRequest( request );
    return service.StartPipelineJob( CString::Empty, request, started );
}

bool
WaitForSync( CIAssetStoreService& service, const CString& jobId, CAssetJobEntry& job, UInt32 timeoutInMs = 120000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        if ( ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) && job.IsEnded() )
            return true;
        PumpFor( 10 );
    }
    return false;
}

/** A count of the plan ("counts" or "skipped") or of the run of the result of a sync job */
UInt32
SyncCount( const CAssetServiceDocument& result, const char* group, const char* name )
{
    const CDataNode* node = GUCEF_NULL;
    if ( CString( "run" ) == group )
        node = result.document.FindChild( "run" );
    else
    {
        const CDataNode* plan = result.document.FindChild( "plan" );
        node = GUCEF_NULL != plan ? plan->FindChild( group ) : GUCEF_NULL;
    }
    return GUCEF_NULL == node ? 0 : node->GetAttributeValueOrChildValueByName( name ).AsUInt32( 0, true );
}

UInt32
PlanValue( const CAssetServiceDocument& result, const char* name )
{
    const CDataNode* plan = result.document.FindChild( "plan" );
    return GUCEF_NULL == plan ? 0 : plan->GetAttributeValueOrChildValueByName( name ).AsUInt32( 0, true );
}

/** Runs a sync to its end and gives its result; false when it did not end in time */
bool
RunSync( CIAssetStoreService& service, const CAssetLibrarySyncSpec& spec, CAssetJobEntry& job, CAssetServiceDocument& result )
{
    CAssetServiceResult started;
    if ( ASSETSTORE_STATUS_OK != StartSync( service, spec, started ) )
        return false;
    if ( !WaitForSync( service, started.id, job ) )
        return false;
    return ASSETSTORE_STATUS_OK == service.GetJobResult( started.id, "plan", CString::Empty, 0, result );
}

/** The state of an item in the lines of the plan of a sync job */
CString
ItemState( CIAssetStoreService& service, const CString& jobId, const CString& itemId )
{
    CString pageToken;
    do
    {
        CAssetServiceDocument page;
        if ( ASSETSTORE_STATUS_OK != service.GetJobResult( jobId, "items", pageToken, 4, page ) )
            return CString::Empty;
        const CDataNode* items = page.document.FindChild( "items" );
        if ( GUCEF_NULL != items )
        {
            for ( CDataNode::const_iterator i = items->ConstBegin(); i != items->ConstEnd(); ++i )
                if ( itemId == (*i)->GetAttributeValueOrChildValueByName( "itemId" ).AsString() )
                    return (*i)->GetAttributeValueOrChildValueByName( "state" ).AsString();
        }
        pageToken = page.nextPageToken;
    }
    while ( !pageToken.IsNULLOrEmpty() );
    return CString::Empty;
}

UInt32
CountStorePacks( CPrimaryAssetStore& store )
{
    CDataTableQuery query;
    query.WhereEquals( "source_id", CVariant( SyncStoreId ) );
    CAssetCatalogue::TRowVector rows;
    store.GetCatalogue().Select( "packs", query, rows );
    UInt32 ingested = 0;
    for ( size_t r=0; r<rows.size(); ++r )
        if ( AssetPackStateIsIngested( CAssetCatalogue::GetText( rows[ r ], "state" ) ) )
            ++ingested;
    return ingested;
}

/** The library sync of the service against the fake library store, end to end */
void
RunLibrarySyncServiceSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    const CString mainFolder = MountFreshDirectory( fixture.label + "_main", base + "/main" );

    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    // the library: eight items to download (odd ones with two formats), lib-05 without a licence, lib-07 and lib-08 without a time of change, lib-09 not downloadable
    CFakeStoreServer server;
    CFakeStoreProviderFactory storeFactory( SyncStoreId );
    for ( UInt32 i=1; i<=8; ++i )
        AddSyncItem( server, i, 1 == i % 2 ? 2 : 1, 7 == i || 8 == i ? 0 : 1700000000000LL + i * 1000, 0, true );
    {
        CAssetStoreItem locked = AddSyncItem( server, 9, 1, 1700000009000LL, 0, true );
        locked.downloadable = false;
        locked.unavailableReason = "key only";
        server.ReplaceItem( locked );
    }
    const bool serverStarted = server.Start();
    CFakeStoreProvider::SetServer( &server );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( SyncStoreId, &storeFactory );
    CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();

    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 2;
    config.primaryStore.database = fixture.database;
    config.credentialStore = fixture.credentials;
    if ( GUCEF_NULL != fixture.credentials )
    {
        config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
        config.primaryStore.credentialAccount = "catalogue";
    }
    CAssetSourcePolicy policy;
    policy.sourceId = SyncStoreId;
    policy.mode = ASSETINGEST_MODE_MANUAL;
    policy.keep = ASSETKEEP_ORIGINAL;
    config.primaryStore.SetPolicy( policy );
    config.loadDownloadQueue = false;
    config.downloads.retryBaseDelayInMs = 20;
    config.downloads.retryMaxDelayInMs = 6000;
    config.downloads.maxAttempts = 2;
    config.downloads.progressIntervalInMs = 10;
    config.downloads.connectTimeoutInMs = 5000;
    CString error;
    TAssetStoreServicePtr servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
    CLocalAssetStoreService* local = servicePtr.IsNULL() ? GUCEF_NULL : static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
    const bool opened = GUCEF_NULL != local && ASSETSTORE_STATUS_OK == local->Open();
    CPrimaryAssetStore* store = opened ? local->GetPrimaryStore() : GUCEF_NULL;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": sync plan (dry run): every page listed, the states and the estimate, nothing queued, only the observations written" ).C_String() )
        try
        {
            ASSERT_TRUE( serverStarted && opened && GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                CIAssetStoreService& service = *local;
                server.ResetStats();
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( service, MakeSyncSpec( true ), job, result ) );
                ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && "pipeline" == job.kind && "sync" == job.subKind && SyncStoreId == job.providerId );
                ASSERT_TRUE( 9 == PlanValue( result, "listed" ) && 3 == PlanValue( result, "pages" ) && 1 == PlanValue( result, "listingComplete" ) && 3 == server.GetStats().libraryRequests );
                ASSERT_TRUE( 8 == SyncCount( result, "counts", "missing" ) && 1 == SyncCount( result, "counts", "unavailable" ) && 0 == SyncCount( result, "counts", "current" ) );
                ASSERT_TRUE( 8 == PlanValue( result, "queuedItems" ) && 12 == PlanValue( result, "queuedDownloads" ) && PlanValue( result, "estimatedBytes" ) > 0 );
                ASSERT_TRUE( SyncCount( result, "counts", "needs-licence-review" ) >= 1 && 0 == SyncCount( result, "run", "queued" ) );
                const CDataNode* storage = GUCEF_NULL != result.document.FindChild( "plan" ) ? result.document.FindChild( "plan" )->FindChild( "storage" ) : GUCEF_NULL;
                ASSERT_TRUE( GUCEF_NULL != storage && "fits" == storage->GetAttributeValueOrChildValueByName( "verdict" ).AsString() && storage->GetAttributeValueOrChildValueByName( "freeBytes" ).AsUInt64( 0, true ) > 0 );
                ASSERT_TRUE( 0 == server.GetStats().fileRequests && 0 == CountStorePacks( *store ) );
                CAssetJobPage jobs;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ListJobs( CString::Empty, CIAssetStoreService::MaxPageEntries, jobs ) );
                for ( size_t j=0; j<jobs.entries.size(); ++j )
                    ASSERT_TRUE( "download" != jobs.entries[ j ].kind );
                // the lines of the plan in pages
                ASSERT_TRUE( CAssetLibrarySyncItem::Missing == ItemState( service, job.jobId, "lib-01" ) && CAssetLibrarySyncItem::Unavailable == ItemState( service, job.jobId, "lib-09" ) );
                CAssetServiceDocument firstPage;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetJobResult( job.jobId, "items", CString::Empty, 4, firstPage ) && "4" == firstPage.nextPageToken );
                TAssetSyncItemRecordMap observed;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetSyncItems( SyncStoreId, observed ) && 9 == observed.size() && !observed[ "lib-09" ].available );
                ASSERT_TRUE( "ts:1700000001000" == observed[ "lib-01" ].listedRevision && CAssetItemRevision::FormatBasis == CAssetItemRevision::GetBasis( observed[ "lib-07" ].listedRevision ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": sync run: every format of every missing item is downloaded through the queue in a window and ingested with its store facts" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                CIAssetStoreService& service = *local;
                CAssetLibrarySyncSpec spec = MakeSyncSpec( false );
                spec.profile.indexAfter = true;
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( service, spec, job, result ) );
                ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && ASSETSTORE_STATUS_OK == job.status && 12 == job.itemsTotal && 12 == job.itemsDone );
                ASSERT_TRUE( 12 == SyncCount( result, "run", "queued" ) && 12 == SyncCount( result, "run", "downloaded" ) && 12 == SyncCount( result, "run", "ingested" ) && 0 == SyncCount( result, "run", "failed" ) );
                ASSERT_TRUE( SyncCount( result, "run", "peakInFlight" ) >= 1 && SyncCount( result, "run", "peakInFlight" ) <= 3 && 3 == SyncCount( result, "run", "window" ) );
                const CDataNode* run = result.document.FindChild( "run" );
                ASSERT_TRUE( GUCEF_NULL != run && !run->GetAttributeValueOrChildValueByName( "indexJobId" ).AsString().IsNULLOrEmpty() );
                ASSERT_TRUE( 12 == CountStorePacks( *store ) && 12 == server.GetStats().fileRequests );
                // an ingested download leaves the queue of the manager: the queue holds no more than the window
                PumpFor( 100 );
                TAssetStoreDownloadJobVector downloads;
                local->GetDownloadManager()->GetJobs( downloads );
                UInt32 leftOfRun = 0;
                for ( size_t d=0; d<downloads.size(); ++d )
                    if ( job.jobId == downloads[ d ].syncJobId )
                        ++leftOfRun;
                ASSERT_TRUE( 0 == leftOfRun );
                TAssetPackSourceRecordVector facts;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfStore( SyncStoreId, facts ) && 12 == facts.size() );
                UInt32 objPacks = 0;
                for ( size_t f=0; f<facts.size(); ++f )
                {
                    ASSERT_TRUE( facts[ f ].IsCurrent() && job.jobId == facts[ f ].syncJobId && !facts[ f ].contentDigest.IsNULLOrEmpty() );
                    if ( "obj" == facts[ f ].sourceFormatId )
                        ++objPacks;
                    if ( "lib-01" == facts[ f ].sourceItemId )
                        ASSERT_TRUE( "ts:1700000001000" == facts[ f ].revision && 1700000001000LL == facts[ f ].changedMs );
                }
                ASSERT_TRUE( 4 == objPacks );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": sync run again: idempotent, everything current, nothing downloaded" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                server.ResetStats();
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( false ), job, result ) );
                ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && 0 == job.itemsTotal );
                ASSERT_TRUE( 8 == SyncCount( result, "counts", "current" ) && 0 == SyncCount( result, "counts", "missing" ) && 0 == SyncCount( result, "counts", "stale" ) );
                ASSERT_TRUE( 0 == PlanValue( result, "queuedDownloads" ) && 0 == SyncCount( result, "run", "queued" ) && 0 == server.GetStats().fileRequests && 12 == CountStorePacks( *store ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a changed item is downloaded again as new packs, the old ones superseded and kept; a newer time without new files only confirms the revision" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                TAssetPackSourceRecordVector before;
                store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-01", before );
                ASSERT_TRUE( 2 == before.size() );
                AddSyncItem( server, 1, 2, 1700000100000LL, 1, false );        // new files, a newer time
                AddSyncItem( server, 2, 1, 1700000200000LL, 0, false );        // the same files, a newer time
                server.ResetStats();
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( false ), job, result ) );
                ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && 2 == SyncCount( result, "counts", "stale" ) && 6 == SyncCount( result, "counts", "current" ) );
                ASSERT_TRUE( 3 == PlanValue( result, "queuedDownloads" ) && 2 == SyncCount( result, "run", "ingested" ) && 1 == SyncCount( result, "run", "revalidated" ) );
                ASSERT_TRUE( CAssetLibrarySyncItem::Stale == ItemState( *local, job.jobId, "lib-01" ) && 14 == CountStorePacks( *store ) );
                TAssetPackSourceRecordVector after;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-01", after ) && 4 == after.size() );
                UInt32 superseded = 0;
                for ( size_t a=0; a<after.size(); ++a )
                {
                    if ( after[ a ].IsCurrent() )
                    {
                        ASSERT_TRUE( "ts:1700000100000" == after[ a ].revision );
                        continue;
                    }
                    ++superseded;
                    // the old pack is still there; its successor is the new pack of the same format
                    CAssetPackRecord old;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetPack( after[ a ].packId, old ) );
                    CAssetPackSourceRecord successor;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSource( after[ a ].supersededBy, successor ) && successor.sourceFormatId == after[ a ].sourceFormatId );
                }
                ASSERT_TRUE( 2 == superseded );
                TAssetPackSourceRecordVector touched;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-02", touched ) && 1 == touched.size() && "ts:1700000200000" == touched[ 0 ].revision );

                // and the next run has nothing to do
                CAssetServiceDocument again;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( false ), job, again ) );
                ASSERT_TRUE( 8 == SyncCount( again, "counts", "current" ) && 0 == PlanValue( again, "queuedDownloads" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a download of an item that is in flight already makes it in flight in the plan, not queued again" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                AddSyncItem( server, 10, 1, 1700000010000LL, 0, true );
                CFakeFileBehaviour slow;
                slow.fail429 = 1;
                slow.retryAfterSeconds = 5;
                server.SetFileBehaviour( "lib-10-fbx", slow );
                CAssetDownloadRequest download;
                download.providerId = SyncStoreId;
                download.itemId = "lib-10";
                download.formatId = "fbx";
                download.forIngest = true;
                CAssetServiceResult queued;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->EnqueueDownload( CString::Empty, download, queued ) );
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( true ), job, result ) );
                ASSERT_TRUE( 1 == SyncCount( result, "counts", "in-flight" ) && CAssetLibrarySyncItem::InFlight == ItemState( *local, job.jobId, "lib-10" ) && 0 == PlanValue( result, "queuedDownloads" ) );
                CAssetServiceResult cancelled;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ControlJob( CString::Empty, queued.id, ASSETJOB_ACTION_CANCEL, cancelled ) );
                server.SetFileBehaviour( "lib-10-fbx", CFakeFileBehaviour() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a download that fails counts against the item: failed before, left out, back with a retry of the failed ones; the others go on" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                AddSyncItem( server, 11, 1, 1700000011000LL, 0, true );
                CFakeFileBehaviour broken;
                broken.fail503 = 100000;
                server.SetFileBehaviour( "lib-11-fbx", broken );
                CAssetLibrarySyncSpec spec = MakeSyncSpec( false );
                spec.profile.maxSyncAttempts = 1;
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( *local, spec, job, result ) );
                ASSERT_TRUE( ASSETJOB_STATE_DONE == job.state && 1 == SyncCount( result, "run", "failed" ) && 1 == SyncCount( result, "run", "ingested" ) );
                ASSERT_TRUE( "partly failed" == job.stateText && job.IsActionAvailable( ASSETJOB_ACTION_RETRY ) );
                const CDataNode* failures = result.document.FindChild( "failures" );
                ASSERT_TRUE( GUCEF_NULL != failures && 1 == failures->GetNrOfChildNodes() );
                CAssetSyncItemRecord record;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetSyncItem( SyncStoreId, "lib-11", record ) && 1 == record.attempts && record.nextAttemptMs > 0 && !record.lastStatus.IsNULLOrEmpty() );

                CAssetServiceDocument plan;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( true ), job, plan ) );
                ASSERT_TRUE( 1 == SyncCount( plan, "counts", "failed-before" ) && CAssetLibrarySyncItem::FailedBefore == ItemState( *local, job.jobId, "lib-11" ) && 0 == PlanValue( plan, "queuedDownloads" ) );
                CAssetLibrarySyncSpec retry = MakeSyncSpec( true );
                retry.retryFailed = true;
                CAssetServiceDocument retried;
                ASSERT_TRUE( RunSync( *local, retry, job, retried ) );
                ASSERT_TRUE( CAssetLibrarySyncItem::Missing == ItemState( *local, job.jobId, "lib-11" ) && 1 == PlanValue( retried, "queuedDownloads" ) );
                server.SetFileBehaviour( "lib-11-fbx", CFakeFileBehaviour() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a second sync of the store is refused while one runs; a cancel stops queueing and cancels the downloads that did not finish" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                AddSyncItem( server, 12, 1, 1700000012000LL, 0, true );
                CFakeFileBehaviour slow;
                slow.fail429 = 1;
                slow.retryAfterSeconds = 5;
                server.SetFileBehaviour( "lib-12-fbx", slow );
                CAssetLibrarySyncSpec spec = MakeSyncSpec( false );
                spec.retryFailed = true;
                CAssetServiceResult started;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *local, spec, started ) );
                CAssetServiceResult second;
                ASSERT_TRUE( ASSETSTORE_STATUS_ALREADY_EXISTS == StartSync( *local, spec, second ) && started.id == second.id );
                // until the downloads are queued
                CAssetJobEntry job;
                bool queuedAll = false;
                for ( UInt32 waited=0; waited<30000 && !queuedAll; waited+=10 )
                {
                    CAssetServiceDocument progress;
                    queuedAll = ASSETSTORE_STATUS_OK == local->GetJobResult( started.id, "plan", CString::Empty, 0, progress ) && SyncCount( progress, "run", "queued" ) >= 1;
                    if ( !queuedAll )
                        PumpFor( 10 );
                }
                ASSERT_TRUE( queuedAll );
                CAssetServiceResult cancelled;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_CANCEL, cancelled ) );
                ASSERT_TRUE( WaitForSync( *local, started.id, job, 30000 ) && ASSETJOB_STATE_CANCELLED == job.state );
                CAssetServiceDocument result;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->GetJobResult( started.id, "plan", CString::Empty, 0, result ) && SyncCount( result, "run", "cancelled" ) >= 1 );
                // the download of lib-12 that waited for the store is cancelled; no pack of it
                // a running download stops at its next step: the manager says so a little later
                bool cancelledDownload = false;
                for ( UInt32 waited=0; waited<10000 && !cancelledDownload; waited+=10 )
                {
                    TAssetStoreDownloadJobVector downloads;
                    local->GetDownloadManager()->GetJobs( downloads );
                    for ( size_t d=0; d<downloads.size(); ++d )
                        if ( "lib-12" == downloads[ d ].itemId && started.id == downloads[ d ].syncJobId )
                            cancelledDownload = ASSETDOWNLOAD_STATE_CANCELLED == downloads[ d ].state;
                    if ( !cancelledDownload )
                        PumpFor( 10 );
                }
                ASSERT_TRUE( cancelledDownload );
                TAssetPackSourceRecordVector none;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-12", none ) && none.empty() );
                // an ended sync is removed from the list of jobs
                PumpFor( 200 );
                CAssetServiceResult removed;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_REMOVE, removed ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == local->GetJob( started.id, job ) );
                server.SetFileBehaviour( "lib-12-fbx", CFakeFileBehaviour() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the disk admission: with less free space than the reserve the run waits with the reason and queues nothing" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                UInt64 freeBytes = 0;
                UInt64 totalBytes = 0;
                ASSERT_TRUE( CAssetLibrarySyncAdmission::ReadVolume( mainFolder, freeBytes, totalBytes ) && freeBytes > 0 );
                CAssetLibrarySyncSpec spec = MakeSyncSpec( false );
                spec.profile.reserveBytes = freeBytes + 1024ULL * 1024 * 1024 * 1024;
                CAssetServiceResult started;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *local, spec, started ) );
                CAssetJobEntry job;
                bool waiting = false;
                for ( UInt32 waited=0; waited<30000 && !waiting; waited+=10 )
                {
                    waiting = ASSETSTORE_STATUS_OK == local->GetJob( started.id, job ) && job.stateText.HasSubstr( "disk space" ) >= 0;
                    if ( !waiting )
                        PumpFor( 10 );
                }
                ASSERT_TRUE( waiting && !job.IsEnded() );
                CAssetServiceDocument result;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->GetJobResult( started.id, "plan", CString::Empty, 0, result ) && 0 == SyncCount( result, "run", "queued" ) );
                const CDataNode* storage = GUCEF_NULL != result.document.FindChild( "plan" ) ? result.document.FindChild( "plan" )->FindChild( "storage" ) : GUCEF_NULL;
                ASSERT_TRUE( GUCEF_NULL != storage && "does-not-fit" == storage->GetAttributeValueOrChildValueByName( "verdict" ).AsString() );
                CAssetServiceResult cancelled;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_CANCEL, cancelled ) );
                ASSERT_TRUE( WaitForSync( *local, started.id, job, 30000 ) && ASSETJOB_STATE_CANCELLED == job.state );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": profile filters and a pin leave items out with their reason; a pack without store facts is adopted; a listing without formats is completed by the details of the items" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                CAssetJobEntry job;
                CAssetLibrarySyncSpec kinds = MakeSyncSpec( true );
                kinds.profile.listingTypes.insert( "material" );
                CAssetServiceDocument filtered;
                ASSERT_TRUE( RunSync( *local, kinds, job, filtered ) );
                ASSERT_TRUE( PlanValue( filtered, "listed" ) == SyncCount( filtered, "counts", "filtered" ) && PlanValue( filtered, "listed" ) == SyncCount( filtered, "skipped", "filter:kind" ) );
                CAssetLibrarySyncSpec licences = MakeSyncSpec( true );
                licences.profile.licenceClasses.insert( "public-domain" );
                CAssetServiceDocument byLicence;
                ASSERT_TRUE( RunSync( *local, licences, job, byLicence ) && SyncCount( byLicence, "skipped", "filter:licence" ) > 0 );
                CAssetLibrarySyncSpec sizes = MakeSyncSpec( true );
                sizes.profile.maxItemBytes = 1;
                CAssetServiceDocument bySize;
                ASSERT_TRUE( RunSync( *local, sizes, job, bySize ) && SyncCount( bySize, "skipped", "filter:size" ) > 0 && 0 == PlanValue( bySize, "queuedDownloads" ) );

                CAssetSyncMark pin;
                pin.sourceId = SyncStoreId;
                pin.itemId = "lib-04";
                pin.mark = CAssetSyncMark::Pin;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().SetMark( pin ) );
                CAssetServiceDocument pinned;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( true ), job, pinned ) && 1 == SyncCount( pinned, "counts", "pinned" ) && CAssetLibrarySyncItem::Pinned == ItemState( *local, job.jobId, "lib-04" ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().RemoveMark( SyncStoreId, "lib-04" ) );

                // a pack of the store from before the facts were kept: unknown in freshness, adopted by a run, current after
                TAssetPackSourceRecordVector sixFacts;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-06", sixFacts ) && 1 == sixFacts.size() );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetCatalogue().DeleteWhereEquals( "pack_sources", "pack_id", CVariant( sixFacts[ 0 ].packId ) ) );
                CAssetLibrarySyncSpec onlySix = MakeSyncSpec( false );
                onlySix.itemIds.push_back( "lib-06" );
                CAssetServiceDocument adopted;
                ASSERT_TRUE( RunSync( *local, onlySix, job, adopted ) && 1 == SyncCount( adopted, "counts", "unknown-freshness" ) );
                ASSERT_TRUE( 0 == PlanValue( adopted, "queuedDownloads" ) && 0 == SyncCount( adopted, "counts", "missing" ) );
                CAssetPackSourceRecord adoptedFacts;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSource( sixFacts[ 0 ].packId, adoptedFacts ) && adoptedFacts.sourceFormatId.IsNULLOrEmpty() && sixFacts[ 0 ].revision == adoptedFacts.revision );
                CAssetServiceDocument afterAdoption;
                ASSERT_TRUE( RunSync( *local, onlySix, job, afterAdoption ) && 1 == SyncCount( afterAdoption, "counts", "current" ) );

                // the listing has no formats: the details of every downloadable item are asked, a new item is downloaded in both formats
                AddSyncItem( server, 13, 2, 1700000013000LL, 0, true );
                server.SetListingWithoutFormats( true );
                server.ResetStats();
                CAssetLibrarySyncSpec enrich = MakeSyncSpec( false );
                enrich.itemIds.push_back( "lib-13" );
                enrich.itemIds.push_back( "lib-07" );
                CAssetServiceDocument enriched;
                ASSERT_TRUE( RunSync( *local, enrich, job, enriched ) );
                // the details of the two selected items for the plan, then per download one for its plan and one for its ingest
                ASSERT_TRUE( 6 == server.GetStats().itemRequests );
                ASSERT_TRUE( 2 == PlanValue( enriched, "queuedDownloads" ) && 2 == SyncCount( enriched, "run", "ingested" ) && 1 == SyncCount( enriched, "counts", "current" ) );
                server.SetListingWithoutFormats( false );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": marks and the listing: an ignored item is left out, an item that left a complete listing is reported and its packs kept, a listing page that fails is asked again" ).C_String() )
        try
        {
            ASSERT_TRUE( GUCEF_NULL != store );
            if ( GUCEF_NULL != store )
            {
                CAssetSyncMark mark;
                mark.sourceId = SyncStoreId;
                mark.itemId = "lib-12";
                mark.mark = CAssetSyncMark::Ignore;
                mark.reason = "not wanted";
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().SetMark( mark ) );
                TAssetPackSourceRecordVector packsOfThree;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetPackSourcesOfItem( SyncStoreId, "lib-03", packsOfThree ) && !packsOfThree.empty() );
                ASSERT_TRUE( server.RemoveItem( "lib-03" ) );
                server.SetLibraryFailures( 1 );
                CAssetJobEntry job;
                CAssetServiceDocument result;
                ASSERT_TRUE( RunSync( *local, MakeSyncSpec( true ), job, result ) );
                ASSERT_TRUE( 1 == SyncCount( result, "counts", "ignored" ) && CAssetLibrarySyncItem::Ignored == ItemState( *local, job.jobId, "lib-12" ) );
                ASSERT_TRUE( 1 == SyncCount( result, "counts", "removed-upstream" ) && 1 == PlanValue( result, "listingComplete" ) );
                CAssetPackRecord kept;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetPack( packsOfThree[ 0 ].packId, kept ) );
                CAssetSyncItemRecord removed;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store->GetSyncStore().GetSyncItem( SyncStoreId, "lib-03", removed ) && !removed.available && "removed-upstream" == removed.unavailableReason );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( GUCEF_NULL != local )
        local->Close();
    servicePtr.Unlink();
    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SyncStoreId );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( SyncStoreId );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    server.Stop();
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

}

/*-------------------------------------------------------------------------*/

namespace {

/** Counts the job events of one job of a service: how many, and whether one showed it running and one showed it ended */
class CSyncJobEventRecorder : public CObservingNotifier
{
    public:

    typedef CTEventHandlerFunctor< CSyncJobEventRecorder > TEventCallback;

    CString jobId;
    UInt32 events;
    bool sawRunning;
    bool sawEnded;
    CAssetJobEntry last;

    explicit CSyncJobEventRecorder( CIAssetStoreService& service )
        : CObservingNotifier(), jobId(), events( 0 ), sawRunning( false ), sawEnded( false ), last()
    {
        TEventCallback changed( this, &CSyncJobEventRecorder::OnJobChanged );
        SubscribeTo( &service, CIAssetStoreService::JobChangedEvent, changed );
    }

    virtual ~CSyncJobEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    void Follow( const CString& id )
    {
        jobId = id;
        events = 0;
        sawRunning = false;
        sawEnded = false;
    }

    private:

    void OnJobChanged( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetJobEntry* job = CICloneable::CloneableAs< CAssetJobEntry >( eventData );
        if ( GUCEF_NULL == job || job->jobId != jobId )
            return;
        ++events;
        last = *job;
        if ( ASSETJOB_STATE_RUNNING == job->state && "sync" == job->subKind )
            sawRunning = true;
        if ( job->IsEnded() )
            sawEnded = true;
    }
};

/** The fake library store of the sync suites that need a service of their own: items 1 to 8 (odd ones with two formats), lib-09 not downloadable */
class CSyncLibraryWorld
{
    public:

    CFakeStoreServer server;
    CFakeStoreProviderFactory factory;
    bool started;

    CSyncLibraryWorld( void )
        : server(), factory( SyncStoreId ), started( false )
    {
        for ( UInt32 i=1; i<=8; ++i )
            AddSyncItem( server, i, 1 == i % 2 ? 2 : 1, 7 == i || 8 == i ? 0 : 1700000000000LL + i * 1000, 0, true );
        CAssetStoreItem locked = AddSyncItem( server, 9, 1, 1700000009000LL, 0, true );
        locked.downloadable = false;
        locked.unavailableReason = "key only";
        server.ReplaceItem( locked );
        started = server.Start();
        CFakeStoreProvider::SetServer( &server );
        CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( SyncStoreId, &factory );
        CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
    }

    ~CSyncLibraryWorld()
    {
        CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SyncStoreId );
        CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( SyncStoreId );
        CFakeStoreProvider::SetServer( GUCEF_NULL );
        server.Stop();
    }
};

/** The configuration of a service of the sync suites: one location under the base, the catalogue of the fixture, the state folder given */
CAssetStoreServiceConfig
MakeSyncServiceConfig( const SSyncFixture& fixture, const CString& stateDirectory )
{
    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 2;
    config.primaryStore.database = fixture.database;
    config.credentialStore = fixture.credentials;
    if ( GUCEF_NULL != fixture.credentials )
    {
        config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
        config.primaryStore.credentialAccount = "catalogue";
    }
    CAssetSourcePolicy policy;
    policy.sourceId = SyncStoreId;
    policy.mode = ASSETINGEST_MODE_MANUAL;
    policy.keep = ASSETKEEP_ORIGINAL;
    config.primaryStore.SetPolicy( policy );
    config.stateDirectory = stateDirectory;
    config.loadDownloadQueue = !stateDirectory.IsNULLOrEmpty();
    config.downloads.retryBaseDelayInMs = 20;
    config.downloads.retryMaxDelayInMs = 6000;
    config.downloads.maxAttempts = 2;
    config.downloads.progressIntervalInMs = 10;
    config.downloads.connectTimeoutInMs = 5000;
    return config;
}

/** The profile the sync suites keep for the store: no reserve on the small test folder, a window of three, no index job */
CAssetLibrarySyncProfile
MakeSyncTestProfile( void )
{
    CAssetLibrarySyncProfile profile;
    profile.enabled = true;
    profile.reservePercent = 0;
    profile.reserveBytes = 0;
    profile.maxInFlightPerStore = 3;
    profile.indexAfter = false;
    profile.order = CAssetLibrarySyncProfile::OrderStore;
    return profile;
}

/** A spec that runs with the profile the service keeps for the store */
CAssetLibrarySyncSpec
MakeStoreProfileSpec( bool plan )
{
    CAssetLibrarySyncSpec spec;
    spec.storeId = SyncStoreId;
    spec.mode = plan ? CAssetLibrarySyncSpec::ModePlan : CAssetLibrarySyncSpec::ModeRun;
    spec.pageSize = 3;
    spec.useStoreProfile = true;
    return spec;
}

/** The lines of the plan of a sync job as one text, "item=state/reason;" in their order, read in pages of four */
CString
PlanLines( CIAssetStoreService& service, const CString& jobId )
{
    CString lines;
    CString pageToken;
    UInt32 pages = 0;
    do
    {
        CAssetServiceDocument page;
        if ( ASSETSTORE_STATUS_OK != service.GetJobResult( jobId, "items", pageToken, 4, page ) || ++pages > 100 )
            return "failed";
        const CDataNode* items = page.document.FindChild( "items" );
        if ( GUCEF_NULL != items )
        {
            for ( CDataNode::const_iterator i = items->ConstBegin(); i != items->ConstEnd(); ++i )
                lines += (*i)->GetAttributeValueOrChildValueByName( "itemId" ).AsString() + "=" + (*i)->GetAttributeValueOrChildValueByName( "state" ).AsString() + "/" +
                         (*i)->GetAttributeValueOrChildValueByName( "reason" ).AsString() + ";";
        }
        pageToken = page.nextPageToken;
    }
    while ( !pageToken.IsNULLOrEmpty() );
    return lines;
}

/** The counts of a sync result that do not depend on when or where it ran, as one text */
CString
PlanCounts( const CAssetServiceDocument& result )
{
    static const char* const counts[] = { "missing", "stale", "current", "unknown-freshness", "in-flight", "unavailable", "ignored", "pinned", "needs-licence-review" };
    CString text;
    for ( size_t c=0; c<sizeof( counts ) / sizeof( counts[ 0 ] ); ++c )
        text += CString( counts[ c ] ) + "=" + ToString( SyncCount( result, "counts", counts[ c ] ) ) + ";";
    static const char* const values[] = { "listed", "pages", "listingComplete", "queuedItems", "queuedDownloads", "estimatedBytes" };
    for ( size_t v=0; v<sizeof( values ) / sizeof( values[ 0 ] ); ++v )
        text += CString( values[ v ] ) + "=" + ToString( PlanValue( result, values[ v ] ) ) + ";";
    return text;
}

/** A node with its attributes, values and children as one text, to compare two */
CString
NodeText( const CDataNode& node )
{
    CString text = node.GetName() + "(" + node.GetValue().AsString() + ")[";
    for ( CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
        text += (*a).first + "=" + (*a).second.AsString() + ";";
    text += "]{";
    for ( CDataNode::const_iterator c = node.ConstBegin(); c != node.ConstEnd(); ++c )
        text += NodeText( *(*c) );
    return text + "}";
}

/** A serialised profile as text, to compare two */
CString
ProfileText( const CAssetLibrarySyncProfile& profile )
{
    CDataNode node( "profile", GUCEF_DATATYPE_OBJECT );
    CDataNodeSerializableSettings settings;
    profile.Serialize( node, settings );
    return NodeText( node );
}

/** The marks of the store as one text, "item=mark/reason;" */
CString
MarksText( CIAssetStoreService& service )
{
    CAssetSyncMarkPage page;
    if ( ASSETSTORE_STATUS_OK != service.GetSyncMarks( SyncStoreId, CString::Empty, 0, page ) )
        return "failed";
    CString text;
    for ( size_t m=0; m<page.entries.size(); ++m )
        text += page.entries[ m ].itemId + "=" + page.entries[ m ].mark + "/" + page.entries[ m ].reason + ";";
    return text;
}

/**
 *  The library sync over the remote path (library sync design, test group 12): the four Sync operations, the plan and the run
 *  as the pipeline job "sync-library", through a remote client of the owner, compared with the local service of the server
 *  side; the roles of the host; the profile and the marks over a restart of the service
 */
void
RunLibrarySyncRemoteSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    {
        CSyncLibraryWorld library;
        CRemoteAssetStoreWorld world( fixture.label );
        const CString stateDirectory = CombinePath( world.directory, "service_state" );
        const CAssetStoreServiceConfig config = MakeSyncServiceConfig( fixture, stateDirectory );
        world.bindingSettings.roles.insert( "viewer" );
        world.bindingSettings.roles.insert( "mcp" );
        CString error;
        const bool serverStarted = library.started && world.StartServer( config, error );
        TAssetStoreServicePtr owner = serverStarted ? world.MakeClient( "owner", error ) : TAssetStoreServicePtr();
        CString::StringSet viewerRoles;
        viewerRoles.insert( "viewer" );
        TAssetStoreServicePtr viewer = serverStarted ? world.MakeClient( "viewer", error, &viewerRoles, false ) : TAssetStoreServicePtr();
        CString::StringSet machineRoles;
        machineRoles.insert( "mcp" );
        TAssetStoreServicePtr machine = serverStarted ? world.MakeClient( "machine", error, &machineRoles, false ) : TAssetStoreServicePtr();
        const bool clientsOpen = !owner.IsNULL() && !viewer.IsNULL() && !machine.IsNULL() &&
                                 ASSETSTORE_STATUS_OK == owner->Open() && ASSETSTORE_STATUS_OK == viewer->Open() && ASSETSTORE_STATUS_OK == machine->Open();
        if ( !clientsOpen )
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "LibrarySyncRemote: the world did not start: " << error );
        CLocalAssetStoreService* local = serverStarted ? static_cast< CLocalAssetStoreService* >( world.serverService.GetPointerAlways() ) : GUCEF_NULL;
        CPrimaryAssetStore* store = GUCEF_NULL != local ? local->GetPrimaryStore() : GUCEF_NULL;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: the four Sync operations give what the local service gives; a mutation repeated with its command id is answered from the first" ).C_String() )
            try
            {
                ASSERT_TRUE( clientsOpen && GUCEF_NULL != store );
                if ( clientsOpen && GUCEF_NULL != store )
                {
                    CAssetServiceInfo info;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetServiceInfo( info ) && 1 == info.capabilities.count( "librarySync" ) );

                    // the profile: the defaults, then the one set, the same on both sides
                    CAssetLibrarySyncProfile remoteProfile;
                    CAssetLibrarySyncProfile localProfile;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetSyncProfile( SyncStoreId, remoteProfile ) && !remoteProfile.enabled );
                    ASSERT_TRUE( ProfileText( CAssetLibrarySyncProfile() ) == ProfileText( remoteProfile ) );
                    CAssetServiceResult set;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSyncProfile( "sync-profile-1", SyncStoreId, MakeSyncTestProfile(), set ) && !set.replayed );
                    CAssetServiceResult again;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSyncProfile( "sync-profile-1", SyncStoreId, MakeSyncTestProfile(), again ) && again.replayed );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetSyncProfile( SyncStoreId, remoteProfile ) && remoteProfile.enabled && 3 == remoteProfile.maxInFlightPerStore );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->GetSyncProfile( SyncStoreId, localProfile ) && ProfileText( localProfile ) == ProfileText( remoteProfile ) );
                    CAssetLibrarySyncProfile broken = MakeSyncTestProfile();
                    broken.order = "sideways";
                    ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == owner->SetSyncProfile( CString::Empty, SyncStoreId, broken, set ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == owner->SetSyncProfile( CString::Empty, "no-such-store", MakeSyncTestProfile(), set ) );

                    // the marks: set two, remove one; a repeat is answered from the first; a wrong mark changes nothing
                    CAssetSyncMarkPage marks;
                    CAssetSyncMark ignore;
                    ignore.itemId = "lib-02";
                    ignore.mark = CAssetSyncMark::Ignore;
                    ignore.reason = "not wanted";
                    marks.entries.push_back( ignore );
                    CAssetSyncMark pin;
                    pin.itemId = "lib-03";
                    pin.mark = CAssetSyncMark::Pin;
                    pin.reason = "keep this version";
                    marks.entries.push_back( pin );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSyncMarks( "sync-marks-1", SyncStoreId, marks, set ) && !set.replayed );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSyncMarks( "sync-marks-1", SyncStoreId, marks, again ) && again.replayed );
                    ASSERT_TRUE( "lib-02=ignore/not wanted;lib-03=pin/keep this version;" == MarksText( *owner ) && MarksText( *owner ) == MarksText( *local ) );
                    CAssetSyncMarkPage removal;
                    CAssetSyncMark unpin;
                    unpin.itemId = "lib-03";
                    removal.entries.push_back( unpin );
                    CAssetSyncMark wrong;
                    wrong.itemId = "lib-04";
                    wrong.mark = "maybe";
                    removal.entries.push_back( wrong );
                    ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == owner->SetSyncMarks( CString::Empty, SyncStoreId, removal, set ) );
                    ASSERT_TRUE( "lib-02=ignore/not wanted;lib-03=pin/keep this version;" == MarksText( *owner ) );
                    removal.entries.pop_back();
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSyncMarks( CString::Empty, SyncStoreId, removal, set ) );
                    ASSERT_TRUE( "lib-02=ignore/not wanted;" == MarksText( *owner ) && MarksText( *owner ) == MarksText( *local ) );
                    CAssetSyncMarkPage firstPage;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetSyncMarks( SyncStoreId, CString::Empty, 1, firstPage ) && 1 == firstPage.entries.size() && 1 == firstPage.total && firstPage.nextPageToken.IsNULLOrEmpty() );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: the roles: a viewer reads the marks, the profile and the plan, and is refused the start, the marks, the profile and the control of a sync; the machine role may plan and not run" ).C_String() )
            try
            {
                ASSERT_TRUE( clientsOpen && GUCEF_NULL != store );
                if ( clientsOpen && GUCEF_NULL != store )
                {
                    ASSERT_TRUE( MarksText( *owner ) == MarksText( *viewer ) );
                    CAssetLibrarySyncProfile seen;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->GetSyncProfile( SyncStoreId, seen ) && seen.enabled );
                    CAssetServiceResult refused;
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->SetSyncProfile( CString::Empty, SyncStoreId, MakeSyncTestProfile(), refused ) );
                    CAssetSyncMarkPage marks;
                    CAssetSyncMark ignore;
                    ignore.itemId = "lib-04";
                    ignore.mark = CAssetSyncMark::Ignore;
                    marks.entries.push_back( ignore );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->SetSyncMarks( CString::Empty, SyncStoreId, marks, refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->SetSyncMarks( CString::Empty, SyncStoreId, marks, refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == StartSync( *viewer, MakeStoreProfileSpec( true ), refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == StartSync( *viewer, MakeStoreProfileSpec( false ), refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == StartSync( *machine, MakeStoreProfileSpec( false ), refused ) );
                    ASSERT_TRUE( "lib-02=ignore/not wanted;" == MarksText( *owner ) );

                    // the machine role plans; the viewer reads the plan and may not control the job
                    CAssetServiceResult started;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *machine, MakeStoreProfileSpec( true ), started ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( WaitForSync( *owner, started.id, job ) && ASSETJOB_STATE_DONE == job.state );
                    CAssetServiceDocument plan;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->GetJobResult( started.id, "plan", CString::Empty, 0, plan ) && 7 == SyncCount( plan, "counts", "missing" ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_RETRY, refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_RETRY, refused ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_REMOVE, refused ) );
                    ASSERT_TRUE( 0 == library.server.GetStats().fileRequests );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: a plan started through the client is the plan of the local service, line for line; its progress reaches the client as job events" ).C_String() )
            try
            {
                ASSERT_TRUE( clientsOpen && GUCEF_NULL != store );
                if ( clientsOpen && GUCEF_NULL != store )
                {
                    CSyncJobEventRecorder events( *owner );
                    CAssetServiceResult started;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *owner, MakeStoreProfileSpec( true ), started ) );
                    events.Follow( started.id );
                    CAssetJobEntry job;
                    ASSERT_TRUE( WaitForSync( *owner, started.id, job ) && ASSETJOB_STATE_DONE == job.state && "sync" == job.subKind );
                    PumpFor( 300 );
                    ASSERT_TRUE( events.events >= 1 && events.sawEnded );
                    CAssetServiceDocument remotePlan;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetJobResult( started.id, "plan", CString::Empty, 0, remotePlan ) );
                    const CString remoteLines = PlanLines( *owner, started.id );

                    CAssetJobEntry localJob;
                    CAssetServiceDocument localPlan;
                    ASSERT_TRUE( RunSync( *local, MakeStoreProfileSpec( true ), localJob, localPlan ) );
                    const CString localLines = PlanLines( *local, localJob.jobId );
                    ASSERT_TRUE( PlanCounts( remotePlan ) == PlanCounts( localPlan ) && remoteLines == localLines );
                    ASSERT_TRUE( 7 == SyncCount( remotePlan, "counts", "missing" ) && 1 == SyncCount( remotePlan, "counts", "ignored" ) && 1 == SyncCount( remotePlan, "counts", "unavailable" ) );
                    ASSERT_TRUE( 11 == PlanValue( remotePlan, "queuedDownloads" ) && remoteLines.HasSubstr( "lib-02=ignored" ) >= 0 );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: a run started through the client with the profile of the store downloads what is missing; a second run through the client and one of the local service have nothing to do" ).C_String() )
            try
            {
                ASSERT_TRUE( clientsOpen && GUCEF_NULL != store );
                if ( clientsOpen && GUCEF_NULL != store )
                {
                    CSyncJobEventRecorder events( *owner );
                    CAssetServiceResult started;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *owner, MakeStoreProfileSpec( false ), started ) );
                    events.Follow( started.id );
                    CAssetJobEntry job;
                    ASSERT_TRUE( WaitForSync( *owner, started.id, job ) && ASSETJOB_STATE_DONE == job.state && 11 == job.itemsTotal && 11 == job.itemsDone );
                    PumpFor( 300 );
                    ASSERT_TRUE( events.sawRunning && events.sawEnded && events.events >= 3 );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetJobResult( started.id, "plan", CString::Empty, 0, result ) );
                    ASSERT_TRUE( 11 == SyncCount( result, "run", "ingested" ) && 0 == SyncCount( result, "run", "failed" ) && SyncCount( result, "run", "peakInFlight" ) <= 3 );
                    ASSERT_TRUE( 11 == CountStorePacks( *store ) );

                    CAssetJobEntry second;
                    CAssetServiceDocument remoteAgain;
                    ASSERT_TRUE( RunSync( *owner, MakeStoreProfileSpec( false ), second, remoteAgain ) );
                    CAssetJobEntry third;
                    CAssetServiceDocument localAgain;
                    ASSERT_TRUE( RunSync( *local, MakeStoreProfileSpec( false ), third, localAgain ) );
                    ASSERT_TRUE( 7 == SyncCount( remoteAgain, "counts", "current" ) && 0 == SyncCount( remoteAgain, "run", "queued" ) );
                    ASSERT_TRUE( PlanCounts( remoteAgain ) == PlanCounts( localAgain ) && PlanLines( *owner, second.jobId ) == PlanLines( *local, third.jobId ) );
                    ASSERT_TRUE( 11 == CountStorePacks( *store ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": remote: the profile and the marks of the store survive a restart of the service" ).C_String() )
            try
            {
                ASSERT_TRUE( clientsOpen && GUCEF_NULL != store );
                if ( clientsOpen && GUCEF_NULL != store )
                {
                    const CString profileBefore = ProfileText( MakeSyncTestProfile() );
                    owner->Close();
                    viewer->Close();
                    machine->Close();
                    owner.Unlink();
                    viewer.Unlink();
                    machine.Unlink();
                    store = GUCEF_NULL;
                    local = GUCEF_NULL;
                    world.StopServer();
                    ASSERT_TRUE( world.StartServer( config, error ) );
                    TAssetStoreServicePtr reopened = world.MakeClient( "owner", error );
                    ASSERT_TRUE( !reopened.IsNULL() && ASSETSTORE_STATUS_OK == reopened->Open() );
                    if ( !reopened.IsNULL() && reopened->IsOpen() )
                    {
                        CAssetLibrarySyncProfile profile;
                        ASSERT_TRUE( ASSETSTORE_STATUS_OK == reopened->GetSyncProfile( SyncStoreId, profile ) && profileBefore == ProfileText( profile ) );
                        ASSERT_TRUE( "lib-02=ignore/not wanted;" == MarksText( *reopened ) );
                        reopened->Close();
                    }
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        if ( !owner.IsNULL() )
            owner->Close();
        if ( !viewer.IsNULL() )
            viewer->Close();
        if ( !machine.IsNULL() )
            machine->Close();
        world.StopServer();
    }
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

}

/*-------------------------------------------------------------------------*/

namespace {

/** A service of the restart suite: made, opened, and ended (Close() and the object gone) as a stopped process would */
class CSyncServiceProcess
{
    public:

    TAssetStoreServicePtr servicePtr;
    CLocalAssetStoreService* local;
    CPrimaryAssetStore* store;

    CSyncServiceProcess( void ) : servicePtr(), local( GUCEF_NULL ), store( GUCEF_NULL ) {}
    ~CSyncServiceProcess() { Stop(); }

    bool Start( const CAssetStoreServiceConfig& config )
    {
        Stop();
        CString error;
        servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
        local = servicePtr.IsNULL() ? GUCEF_NULL : static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
        if ( GUCEF_NULL == local || ASSETSTORE_STATUS_OK != local->Open() )
            return false;
        store = local->GetPrimaryStore();
        return GUCEF_NULL != store;
    }

    void Stop( void )
    {
        if ( GUCEF_NULL != local )
            local->Close();
        servicePtr.Unlink();
        local = GUCEF_NULL;
        store = GUCEF_NULL;
        PumpFor( 20 );
    }
};

/** Pumps until the job has ended at least 'done' of its downloads (ingested, failed or skipped) or ended; false when it ended or the time ran out */
bool
PumpUntilDone( CIAssetStoreService& service, const CString& jobId, UInt32 done, CAssetJobEntry& job, UInt32 timeoutInMs = 60000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) )
        {
            if ( job.IsEnded() )
                return false;
            if ( job.itemsDone >= done && job.itemsTotal > 0 )
                return true;
        }
        PumpFor( 5 );
    }
    return false;
}

/** Pumps until the job is in the state with the downloads planned (its plan is there); false when the time ran out */
bool
PumpUntilState( CIAssetStoreService& service, const CString& jobId, TAssetJobState state, UInt32 itemsTotal, CAssetJobEntry& job, UInt32 timeoutInMs = 60000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) && state == job.state && itemsTotal == job.itemsTotal )
            return true;
        PumpFor( 5 );
    }
    return false;
}

/** Whether no item and format of the store has more than one current pack, and how many current packs there are */
bool
OneCurrentPackEach( CPrimaryAssetStore& store, UInt32& current )
{
    TAssetPackSourceRecordVector facts;
    current = 0;
    if ( ASSETSTORE_STATUS_OK != store.GetSyncStore().GetPackSourcesOfStore( SyncStoreId, facts ) )
        return false;
    CString::StringSet seen;
    for ( size_t f=0; f<facts.size(); ++f )
    {
        if ( !facts[ f ].IsCurrent() )
            continue;
        ++current;
        const CString key = facts[ f ].sourceItemId + "/" + facts[ f ].sourceFormatId;
        if ( seen.end() != seen.find( key ) )
            return false;
        seen.insert( key );
    }
    return true;
}

/** The ids of a set of items as the itemIds of a spec */
CString::StringVector
SyncItemIds( UInt32 first, UInt32 last )
{
    CString::StringVector ids;
    for ( UInt32 i=first; i<=last; ++i )
        ids.push_back( "lib-" + CString( i < 10 ? "0" : "" ) + ToString( i ) );
    return ids;
}

/**
 *  Restart in the middle of a sync (library sync design, test group 8), the control of a sync (pause, resume) and the wait for
 *  a sign in: the service is ended while a run lists, while downloads are in flight, without its work list file and while it
 *  is paused; a new service on the same state folder and catalogue takes the job up under its id and ends it with the same
 *  packs as an uninterrupted run, nothing downloaded twice
 */
void
RunLibrarySyncRestartSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    {
        CSyncLibraryWorld library;
        const CString stateDirectory = TestDirectory( fixture.label + "_state" );
        DeleteDir( stateDirectory, true );
        CAssetStoreServiceConfig config = MakeSyncServiceConfig( fixture, stateDirectory );
        config.downloads.maxAttempts = 4;
        CSyncServiceProcess process;
        const bool started = library.started && process.Start( config );
        CAssetServiceResult set;
        CAssetLibrarySyncProfile profile = MakeSyncTestProfile();
        const bool profiled = started && ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, profile, set );

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": restart while the run lists: the new service takes the job up under its id and downloads everything once" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    CAssetServiceResult run;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, MakeStoreProfileSpec( false ), run ) );
                    // ended before the first page was asked
                    process.Stop();
                    ASSERT_TRUE( process.Start( config ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( WaitForSync( *process.local, run.id, job ) && ASSETJOB_STATE_DONE == job.state && "sync" == job.subKind );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, result ) );
                    ASSERT_TRUE( 12 == SyncCount( result, "run", "queued" ) && 12 == SyncCount( result, "run", "ingested" ) && 0 == SyncCount( result, "run", "revalidated" ) );
                    UInt32 current = 0;
                    ASSERT_TRUE( OneCurrentPackEach( *process.store, current ) && 12 == current && 12 == CountStorePacks( *process.store ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": restart with downloads in flight: the job follows the downloads it queued, queues the rest, and nothing is downloaded twice" ).C_String() )
            try
            {
                ASSERT_TRUE( GUCEF_NULL != process.store );
                if ( GUCEF_NULL != process.store )
                {
                    // six new items; two of them are slow (the store asks to come back after two seconds)
                    for ( UInt32 i=10; i<=15; ++i )
                        AddSyncItem( library.server, i, 1, 1700000000000LL + i * 1000, 0, true );
                    CFakeFileBehaviour slow;
                    slow.fail429 = 1;
                    slow.retryAfterSeconds = 2;
                    library.server.SetFileBehaviour( "lib-12-fbx", slow );
                    library.server.SetFileBehaviour( "lib-13-fbx", slow );
                    CAssetServiceResult run;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, MakeStoreProfileSpec( false ), run ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( PumpUntilDone( *process.local, run.id, 2, job ) );
                    CAssetServiceDocument before;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, before ) );
                    const UInt32 queuedBefore = SyncCount( before, "run", "queued" );
                    const UInt32 endedBefore = SyncCount( before, "run", "ingested" );
                    ASSERT_TRUE( 6 == PlanValue( before, "queuedDownloads" ) && queuedBefore > endedBefore );
                    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncRestart: ended the service with " << queuedBefore << " queued and " << endedBefore << " ingested" );

                    process.Stop();
                    ASSERT_TRUE( process.Start( config ) );
                    ASSERT_TRUE( WaitForSync( *process.local, run.id, job ) && ASSETJOB_STATE_DONE == job.state );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, result ) );
                    ASSERT_TRUE( 6 == SyncCount( result, "run", "queued" ) && 6 == SyncCount( result, "run", "ingested" ) && 0 == SyncCount( result, "run", "revalidated" ) && 0 == SyncCount( result, "run", "failed" ) );
                    ASSERT_TRUE( 6 == PlanValue( result, "queuedDownloads" ) && CAssetLibrarySyncItem::Missing == ItemState( *process.local, run.id, "lib-12" ) );
                    UInt32 current = 0;
                    ASSERT_TRUE( OneCurrentPackEach( *process.store, current ) && 18 == current && 18 == CountStorePacks( *process.store ) );
                    library.server.SetFileBehaviour( "lib-12-fbx", CFakeFileBehaviour() );
                    library.server.SetFileBehaviour( "lib-13-fbx", CFakeFileBehaviour() );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": restart without the work list file: the job plans again, its downloads in flight are followed and not queued again" ).C_String() )
            try
            {
                ASSERT_TRUE( GUCEF_NULL != process.store );
                if ( GUCEF_NULL != process.store )
                {
                    for ( UInt32 i=16; i<=19; ++i )
                        AddSyncItem( library.server, i, 1, 1700000000000LL + i * 1000, 0, true );
                    CFakeFileBehaviour slow;
                    slow.fail429 = 1;
                    slow.retryAfterSeconds = 2;
                    library.server.SetFileBehaviour( "lib-17-fbx", slow );
                    CAssetServiceResult run;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, MakeStoreProfileSpec( false ), run ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( PumpUntilDone( *process.local, run.id, 1, job ) );
                    process.Stop();
                    const CString worklistFile = CombinePath( stateDirectory, "syncjob_" + run.id.CutChars( CLocalAssetStoreService::PipelineJobPrefix.Length(), true ) + ".json" );
                    ASSERT_TRUE( FileExists( worklistFile ) && DeleteFile( worklistFile ) );
                    ASSERT_TRUE( process.Start( config ) );
                    ASSERT_TRUE( WaitForSync( *process.local, run.id, job ) && ASSETJOB_STATE_DONE == job.state );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, result ) );
                    ASSERT_TRUE( 0 == SyncCount( result, "run", "revalidated" ) && 0 == SyncCount( result, "run", "failed" ) && SyncCount( result, "run", "queued" ) <= 4 );
                    UInt32 current = 0;
                    ASSERT_TRUE( OneCurrentPackEach( *process.store, current ) && 22 == current && 22 == CountStorePacks( *process.store ) );
                    ASSERT_TRUE( !FileExists( worklistFile ) );
                    library.server.SetFileBehaviour( "lib-17-fbx", CFakeFileBehaviour() );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": pause and resume: a paused run queues nothing, stays paused over a restart, and ends after the resume" ).C_String() )
            try
            {
                ASSERT_TRUE( GUCEF_NULL != process.store );
                if ( GUCEF_NULL != process.store )
                {
                    for ( UInt32 i=20; i<=23; ++i )
                        AddSyncItem( library.server, i, 1, 1700000000000LL + i * 1000, 0, true );
                    CAssetLibrarySyncSpec spec = MakeStoreProfileSpec( false );
                    spec.itemIds = SyncItemIds( 20, 23 );
                    CAssetServiceResult run;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, spec, run ) );
                    CAssetServiceResult control;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->ControlJob( CString::Empty, run.id, ASSETJOB_ACTION_PAUSE, control ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( PumpUntilState( *process.local, run.id, ASSETJOB_STATE_PAUSED, 4, job ) && 0 == job.itemsDone );
                    ASSERT_TRUE( job.stateText.HasSubstr( "paused" ) >= 0 );
                    CAssetServiceDocument paused;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, paused ) && 0 == SyncCount( paused, "run", "queued" ) );

                    process.Stop();
                    ASSERT_TRUE( process.Start( config ) );
                    ASSERT_TRUE( PumpUntilState( *process.local, run.id, ASSETJOB_STATE_PAUSED, 4, job ) && 0 == job.itemsDone );
                    ASSERT_TRUE( CAssetLibrarySyncItem::Missing == ItemState( *process.local, run.id, "lib-21" ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->ControlJob( CString::Empty, run.id, ASSETJOB_ACTION_RESUME, control ) );
                    ASSERT_TRUE( WaitForSync( *process.local, run.id, job ) && ASSETJOB_STATE_DONE == job.state );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( run.id, "plan", CString::Empty, 0, result ) && 4 == SyncCount( result, "run", "ingested" ) );
                    UInt32 current = 0;
                    ASSERT_TRUE( OneCurrentPackEach( *process.store, current ) && 26 == current );
                    // the state file keeps no checkpoint of a run that ended
                    process.Stop();
                    ASSERT_TRUE( process.Start( config ) );
                    PumpFor( 200 );
                    ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == process.local->GetJob( run.id, job ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        process.Stop();
    }

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a listing refused for a sign in waits with the reason and goes on when the store is signed in" ).C_String() )
        try
        {
            CFakeSignInStoreProviderFactory signInFactory( "syncsign", false );
            CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "syncsign", &signInFactory );
            CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
            CSyncServiceProcess process;
            const bool started = process.Start( MakeSyncServiceConfig( fixture, CString::Empty ) );
            ASSERT_TRUE( started );
            TAssetStoreProviderPtr provider;
            ASSERT_TRUE( started && process.local->GetStoreProvider( "syncsign", provider ) && !provider.IsNULL() );
            if ( started && !provider.IsNULL() )
            {
                CFakeSignInStoreProvider* signIn = static_cast< CFakeSignInStoreProvider* >( provider.GetPointerAlways() );
                signIn->SetSignedIn( false );
                CAssetLibrarySyncSpec spec = MakeSyncSpec( true );
                spec.storeId = "syncsign";
                CAssetServiceResult run;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, spec, run ) );
                PumpFor( 500 );
                CAssetJobEntry job;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJob( run.id, job ) && !job.IsEnded() );
                ASSERT_TRUE( "waiting for sign in" == job.stateText && ASSETSTORE_STATUS_OWNER_ACTION_NEEDED == job.status );
                signIn->SetSignedIn( true );
                ASSERT_TRUE( WaitForSync( *process.local, run.id, job, 10000 ) && ASSETJOB_STATE_DONE == job.state && ASSETSTORE_STATUS_OK == job.status );
            }
            process.Stop();
            CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "syncsign" );
            CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "syncsign" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

}

/*-------------------------------------------------------------------------*/

namespace {

/** What the scripted volume reader answers for a folder: the real volume, nothing (unknown), a full volume or a small one */
enum EVolumeScript
{
    VOLUME_REAL = 0     ,
    VOLUME_UNKNOWN      ,
    VOLUME_FULL         ,
    VOLUME_SMALL
};

struct SVolumeScript
{
    CString scratchFolder;
    CString libraryFolder;
    EVolumeScript scratch;
    EVolumeScript library;
    UInt32 libraryReads;
    SVolumeScript( void ) : scratchFolder(), libraryFolder(), scratch( VOLUME_REAL ), library( VOLUME_REAL ), libraryReads( 0 ) {}
};

bool
ScriptedVolumeReader( const CString& path, UInt64& freeBytes, UInt64& totalBytes, void* context )
{
    SVolumeScript* script = static_cast< SVolumeScript* >( context );
    const CString normal = RelativePath( path ).ReplaceChar( '\\', '/' );
    EVolumeScript mode = VOLUME_REAL;
    if ( !script->scratchFolder.IsNULLOrEmpty() && 0 == normal.HasSubstr( script->scratchFolder ) )
        mode = script->scratch;
    else
    if ( !script->libraryFolder.IsNULLOrEmpty() && 0 == normal.HasSubstr( script->libraryFolder ) )
    {
        mode = script->library;
        ++script->libraryReads;
    }
    switch ( mode )
    {
        case VOLUME_UNKNOWN: return false;
        case VOLUME_FULL: freeBytes = 0; totalBytes = 1024ULL * 1024ULL * 1024ULL * 1024ULL; return true;
        case VOLUME_SMALL: freeBytes = 10; totalBytes = 1024ULL * 1024ULL * 1024ULL * 1024ULL; return true;
        case VOLUME_REAL:
        default:
        {
            CStorageVolumeInformation info;
            if ( !GetFileSystemStorageVolumeInformationByDirPath( info, path ) || !info.hasFreeBytesAvailableToCaller )
                return false;
            freeBytes = info.freeBytesAvailableToCaller;
            totalBytes = info.hasTotalNumberOfBytes ? info.totalNumberOfBytes : 0;
            return true;
        }
    }
}

/** The staging folders of the downloads the manager is given */
class CStagingRecorder : public CObserver
{
    public:
    CString::StringVector destinations;
    explicit CStagingRecorder( CAssetStoreDownloadManager& manager ) : CObserver(), destinations() { SubscribeTo( &manager, CAssetStoreDownloadManager::JobAddedEvent ); }
    virtual ~CStagingRecorder() { UnsubscribeAllFromObserver(); }
    virtual void OnNotify( CNotifier*, const CEvent& eventId, CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CAssetStoreDownloadJobEventData* data = CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
        if ( GUCEF_NULL != data )
            destinations.push_back( data->job.destination );
    }
    UInt32 CountUnder( const CString& prefix ) const
    {
        UInt32 count = 0;
        for ( size_t d=0; d<destinations.size(); ++d )
            if ( 0 == destinations[ d ].HasSubstr( prefix ) )
                ++count;
        return count;
    }
};

/** Pumps until the state text of the job holds the text; false when the job ended or the time ran out */
bool
PumpUntilStateText( CIAssetStoreService& service, const CString& jobId, const CString& text, CAssetJobEntry& job, UInt32 timeoutInMs = 30000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) )
        {
            if ( job.IsEnded() )
                return false;
            if ( job.stateText.HasSubstr( text ) >= 0 )
                return true;
        }
        PumpFor( 5 );
    }
    return false;
}

/**
 *  Where downloads are staged and the space rules (owner decision: the library location is where everything ends up; a local
 *  scratch staging area is a preference only; the space that counts is that of the library volume; an unreadable volume admits):
 *  the scratch area used when it has room, a scratch area that cannot be made or is too small sends the download to the library
 *  staging, an unknown library volume admits, a full library volume makes a sync wait and refuses a manual download
 */
void
RunLibrarySyncDiskSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    const CString mainFolder = RelativePath( MountFreshDirectory( fixture.label + "_main", base + "/main" ) ).ReplaceChar( '\\', '/' );
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    {
        CSyncLibraryWorld library;
        SVolumeScript script;
        script.libraryFolder = mainFolder;
        CAssetLibrarySyncAdmission::SetVolumeReader( &ScriptedVolumeReader, &script );
        const CString scratchFolder = RelativePath( TestDirectory( fixture.label + "_scratch" ) ).ReplaceChar( '\\', '/' );
        DeleteDir( scratchFolder, true );
        script.scratchFolder = scratchFolder;
        CAssetStoreServiceConfig config = MakeSyncServiceConfig( fixture, CString::Empty );
        config.primaryStore.scratchStagingPath = scratchFolder;
        CSyncServiceProcess process;
        bool started = library.started && process.Start( config );
        CAssetServiceResult set;
        const bool profiled = started && ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, MakeSyncTestProfile(), set );

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a scratch staging area with room takes the downloads; the packs end up in the library location and the scratch area is left empty" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    CStagingRecorder staging( *process.local->GetDownloadManager() );
                    CAssetJobEntry job;
                    CAssetServiceDocument result;
                    ASSERT_TRUE( RunSync( *process.local, MakeStoreProfileSpec( false ), job, result ) && ASSETJOB_STATE_DONE == job.state );
                    ASSERT_TRUE( 12 == SyncCount( result, "run", "ingested" ) && 12 == CountStorePacks( *process.store ) );
                    ASSERT_TRUE( 12 == staging.destinations.size() && 12 == staging.CountUnder( CPrimaryAssetStore::ScratchStagingVfsRoot ) );
                    CString::StringVector left;
                    CString::StringVector folders;
                    VFS::CVfsGlobal::Instance()->GetVfs().GetDirList( folders, CPrimaryAssetStore::ScratchStagingVfsRoot, false, false, CString::Empty );
                    ASSERT_TRUE( folders.empty() );
                    TAssetPackSourceRecordVector facts;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.store->GetSyncStore().GetPackSourcesOfStore( SyncStoreId, facts ) && 12 == facts.size() );
                    CAssetPackRecord pack;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.store->GetPack( facts[ 0 ].packId, pack ) && "main" == pack.locationId );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a scratch staging area that is too small, or that cannot be made, is passed by: the downloads are staged on the library without an error or a wait" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    for ( UInt32 i=30; i<=31; ++i )
                        AddSyncItem( library.server, i, 1, 1700000000000LL + i * 1000, 0, true );
                    script.scratch = VOLUME_SMALL;
                    {
                        CStagingRecorder staging( *process.local->GetDownloadManager() );
                        CAssetJobEntry job;
                        CAssetServiceDocument result;
                        ASSERT_TRUE( RunSync( *process.local, MakeStoreProfileSpec( false ), job, result ) && ASSETJOB_STATE_DONE == job.state );
                        ASSERT_TRUE( 2 == SyncCount( result, "run", "ingested" ) && 0 == SyncCount( result, "run", "failed" ) );
                        ASSERT_TRUE( 2 == staging.destinations.size() && 0 == staging.CountUnder( CPrimaryAssetStore::ScratchStagingVfsRoot ) && 2 == staging.CountUnder( base + "/main/staging" ) );
                    }
                    script.scratch = VOLUME_REAL;

                    // a scratch path below a file cannot be made
                    process.Stop();
                    const CString blocker = CombinePath( TestDirectory( fixture.label + "_blocker" ), "file" );
                    CreateDirs( StripFilename( blocker ) );
                    CDynamicBuffer content;
                    content.Append( "x", 1 );
                    ASSERT_TRUE( content.WriteContentToFile( blocker, 0, true ) && FileExists( blocker ) );
                    CAssetStoreServiceConfig blocked = config;
                    blocked.primaryStore.scratchStagingPath = CombinePath( blocker, "scratch" );
                    ASSERT_TRUE( process.Start( blocked ) );
                    AddSyncItem( library.server, 32, 1, 1700000032000LL, 0, true );
                    CStagingRecorder staging( *process.local->GetDownloadManager() );
                    CAssetJobEntry job;
                    CAssetServiceDocument result;
                    ASSERT_TRUE( RunSync( *process.local, MakeStoreProfileSpec( false ), job, result ) && ASSETJOB_STATE_DONE == job.state );
                    ASSERT_TRUE( 1 == SyncCount( result, "run", "ingested" ) && 1 == staging.destinations.size() && 1 == staging.CountUnder( base + "/main/staging" ) );
                    ASSERT_TRUE( 15 == CountStorePacks( *process.store ) );
                    process.Stop();
                    started = process.Start( config );
                    ASSERT_TRUE( started );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the free space of the library volume cannot be read: the sync is admitted (a warning is logged once) and ends" ).C_String() )
            try
            {
                ASSERT_TRUE( started );
                if ( started )
                {
                    AddSyncItem( library.server, 33, 1, 1700000033000LL, 0, true );
                    script.library = VOLUME_UNKNOWN;
                    script.libraryReads = 0;
                    CAssetJobEntry job;
                    CAssetServiceDocument result;
                    ASSERT_TRUE( RunSync( *process.local, MakeStoreProfileSpec( false ), job, result ) && ASSETJOB_STATE_DONE == job.state );
                    ASSERT_TRUE( 1 == SyncCount( result, "run", "ingested" ) && script.libraryReads > 0 && 16 == CountStorePacks( *process.store ) );
                    script.library = VOLUME_REAL;
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a full library volume makes a sync wait with the reason; a manual download of items that do not fit is refused per item, not waited for" ).C_String() )
            try
            {
                ASSERT_TRUE( started );
                if ( started )
                {
                    for ( UInt32 i=34; i<=35; ++i )
                        AddSyncItem( library.server, i, 1, 1700000000000LL + i * 1000, 0, true );
                    script.library = VOLUME_FULL;
                    CAssetServiceResult run;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == StartSync( *process.local, MakeStoreProfileSpec( false ), run ) );
                    CAssetJobEntry job;
                    ASSERT_TRUE( PumpUntilStateText( *process.local, run.id, "waiting for disk space", job ) && 0 == job.itemsDone );
                    CAssetServiceResult control;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->ControlJob( CString::Empty, run.id, ASSETJOB_ACTION_CANCEL, control ) );
                    ASSERT_TRUE( WaitForSync( *process.local, run.id, job ) && ASSETJOB_STATE_CANCELLED == job.state );

                    CAssetLibrarySyncSpec manual = MakeStoreProfileSpec( false );
                    manual.manual = true;
                    manual.itemIds = SyncItemIds( 34, 35 );
                    CAssetServiceDocument result;
                    ASSERT_TRUE( RunSync( *process.local, manual, job, result ) && ASSETJOB_STATE_DONE == job.state );
                    ASSERT_TRUE( 2 == SyncCount( result, "run", "failed" ) && 0 == SyncCount( result, "run", "queued" ) && 16 == CountStorePacks( *process.store ) );
                    const CDataNode* failures = result.document.FindChild( "failures" );
                    ASSERT_TRUE( GUCEF_NULL != failures && 2 == failures->GetNrOfChildNodes() );
                    ASSERT_TRUE( GUCEF_NULL != failures && ( *failures->ConstBegin() )->GetAttributeValueOrChildValueByName( "message" ).AsString().HasSubstr( "not enough free space" ) == 0 );

                    // with room again the manual download goes through (the failed items are retried with the retry of the failed ones)
                    script.library = VOLUME_REAL;
                    manual.retryFailed = true;
                    ASSERT_TRUE( RunSync( *process.local, manual, job, result ) && ASSETJOB_STATE_DONE == job.state && 2 == SyncCount( result, "run", "ingested" ) );
                    ASSERT_TRUE( 18 == CountStorePacks( *process.store ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        process.Stop();
        CAssetLibrarySyncAdmission::SetVolumeReader( GUCEF_NULL, GUCEF_NULL );
    }
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}


/*-------------------------------------------------------------------------*/

/** The notices of a service and the first sync job of a store that it reports */
class CSyncNoticeRecorder : public CObservingNotifier
{
    public:

    typedef CTEventHandlerFunctor< CSyncNoticeRecorder > TEventCallback;

    CString storeId;
    CString firstSyncJobId;
    CString::StringVector codes;
    CString::StringVector texts;

    CSyncNoticeRecorder( CIAssetStoreService& service, const CString& store )
        : CObservingNotifier(), storeId( store ), firstSyncJobId(), codes(), texts()
    {
        TEventCallback notice( this, &CSyncNoticeRecorder::OnNotice );
        SubscribeTo( &service, CIAssetStoreService::NoticeEvent, notice );
        TEventCallback changed( this, &CSyncNoticeRecorder::OnJobChanged );
        SubscribeTo( &service, CIAssetStoreService::JobChangedEvent, changed );
    }

    virtual ~CSyncNoticeRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    UInt32 Count( const char* code ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<codes.size(); ++i )
            if ( codes[ i ] == code )
                ++count;
        return count;
    }

    private:

    void OnNotice( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetServiceNotice* notice = CICloneable::CloneableAs< CAssetServiceNotice >( eventData );
        if ( GUCEF_NULL == notice )
            return;
        codes.push_back( notice->code );
        texts.push_back( notice->text );
    }

    void OnJobChanged( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetJobEntry* job = CICloneable::CloneableAs< CAssetJobEntry >( eventData );
        if ( GUCEF_NULL != job && firstSyncJobId.IsNULLOrEmpty() && "sync" == job->subKind && storeId == job->providerId )
            firstSyncJobId = job->jobId;
    }
};

/** Pumps until the recorder saw a sync job of its store; false when the time ran out */
bool
PumpUntilSyncJobSeen( CSyncNoticeRecorder& recorder, UInt32 timeoutInMs = 60000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs && recorder.firstSyncJobId.IsNULLOrEmpty(); waited+=10 )
        PumpFor( 10 );
    return !recorder.firstSyncJobId.IsNULLOrEmpty();
}

/** Pumps until the recorder saw a notice with the code; false when the time ran out */
bool
PumpUntilNotice( CSyncNoticeRecorder& recorder, const char* code, UInt32 timeoutInMs = 60000 )
{
    for ( UInt32 waited=0; waited<timeoutInMs && 0 == recorder.Count( code ); waited+=10 )
        PumpFor( 10 );
    return 0 != recorder.Count( code );
}

/** Whether the scheduler of the service holds the schedule job of the store */
bool
HasScheduleJob( CLocalAssetStoreService& service, const CString& storeId )
{
    JobSchedulerPtr scheduler = service.GetSyncScheduler();
    if ( scheduler.IsNULL() )
        return false;
    CString::StringSet ids;
    scheduler->GetJobIds( ids );
    return ids.end() != ids.find( CLocalAssetStoreService::MakeScheduleJobId( storeId ) );
}

/**
 *  The scheduled sync (library sync design, step S10): a profile with a schedule registers the job sync-library.<store> with
 *  the job scheduler of the service, a run of that job starts a sync with the profile of the store, the end of the sync is a
 *  notice of the service; without a schedule there is no scheduler, and the schedule comes back after a restart
 */
void
RunLibrarySyncScheduleSuite( SSyncFixture& fixture )
{
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_main", base + "/main" );
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    {
        CSyncLibraryWorld library;
        const CString stateDirectory = TestDirectory( fixture.label + "_state" );
        DeleteDir( stateDirectory, true );
        CAssetStoreServiceConfig config = MakeSyncServiceConfig( fixture, stateDirectory );
        CSyncServiceProcess process;
        const bool started = library.started && process.Start( config );
        CAssetServiceResult set;
        const bool profiled = started && ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, MakeSyncTestProfile(), set );

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": schedule: off by default, an enabled profile without a schedule makes no scheduler; a schedule that is not a cron expression is refused" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    ASSERT_TRUE( CAssetLibrarySyncProfile().schedule.IsNULLOrEmpty() && !CAssetLibrarySyncProfile().enabled );
                    ASSERT_TRUE( process.local->GetSyncScheduler().IsNULL() );
                    CAssetLibrarySyncProfile broken = MakeSyncTestProfile();
                    broken.schedule = "every night";
                    CAssetServiceResult refused;
                    ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == process.local->SetSyncProfile( CString::Empty, SyncStoreId, broken, refused ) );
                    ASSERT_TRUE( process.local->GetSyncScheduler().IsNULL() );
                    ASSERT_TRUE( "sync-library.fab-store-x_1" == CLocalAssetStoreService::MakeScheduleJobId( "Fab Store/X_1" ) );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": schedule: a profile with a schedule registers sync-library.<store>; a run of the job starts a sync with the profile that ends with a notice" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    CSyncNoticeRecorder recorder( *process.local, SyncStoreId );
                    CAssetLibrarySyncProfile scheduled = MakeSyncTestProfile();
                    scheduled.schedule = "0 3 * * *";
                    CAssetServiceResult setScheduled;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, scheduled, setScheduled ) );
                    ASSERT_TRUE( HasScheduleJob( *process.local, SyncStoreId ) );
                    JobSchedulerPtr scheduler = process.local->GetSyncScheduler();
                    ASSERT_TRUE( !scheduler.IsNULL() && "assetstore-sync-" + process.local->GetScheduleKey() == scheduler->GetName() );
                    if ( !scheduler.IsNULL() )
                    {
                        CJobDefinition job;
                        ASSERT_TRUE( scheduler->GetJobDefinition( CLocalAssetStoreService::MakeScheduleJobId( SyncStoreId ), job ) );
                        ASSERT_TRUE( CAssetLibrarySyncScheduleTaskConsumer::TaskType == job.GetTaskTypeName() );
                        scheduler->RunNowAsync( CLocalAssetStoreService::MakeScheduleJobId( SyncStoreId ), "test" );
                        ASSERT_TRUE( PumpUntilSyncJobSeen( recorder ) );
                        CAssetJobEntry entry;
                        ASSERT_TRUE( WaitForSync( *process.local, recorder.firstSyncJobId, entry ) && ASSETJOB_STATE_DONE == entry.state );
                        CAssetServiceDocument result;
                        ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->GetJobResult( recorder.firstSyncJobId, "plan", CString::Empty, 0, result ) );
                        ASSERT_TRUE( 12 == SyncCount( result, "run", "ingested" ) );
                        ASSERT_TRUE( PumpUntilNotice( recorder, "SYNC_DONE" ) );
                        PumpFor( 50 );
                        ASSERT_TRUE( 1 == recorder.Count( "SYNC_DONE" ) && 0 == recorder.Count( "SYNC_FAILED" ) );
                    }
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( ( fixture.label + ": schedule: the schedule comes back after a restart; a profile without it, or disabled, takes the job away; the scheduler goes with the service" ).C_String() )
            try
            {
                ASSERT_TRUE( started && profiled );
                if ( started && profiled )
                {
                    process.Stop();
                    ASSERT_TRUE( process.Start( config ) );
                    ASSERT_TRUE( HasScheduleJob( *process.local, SyncStoreId ) );
                    CAssetLibrarySyncProfile disabled = MakeSyncTestProfile();
                    disabled.schedule = "0 3 * * *";
                    disabled.enabled = false;
                    CAssetServiceResult setDisabled;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, disabled, setDisabled ) );
                    ASSERT_TRUE( !HasScheduleJob( *process.local, SyncStoreId ) );
                    CAssetLibrarySyncProfile again = MakeSyncTestProfile();
                    again.schedule = "30 2 * * 1";
                    CAssetServiceResult setAgain;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, again, setAgain ) );
                    ASSERT_TRUE( HasScheduleJob( *process.local, SyncStoreId ) );
                    CAssetServiceResult setNone;
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, MakeSyncTestProfile(), setNone ) );
                    ASSERT_TRUE( !HasScheduleJob( *process.local, SyncStoreId ) );
                    ASSERT_TRUE( ASSETSTORE_STATUS_OK == process.local->SetSyncProfile( CString::Empty, SyncStoreId, again, setAgain ) );
                    const CString hostName = "assetstore-sync-" + process.local->GetScheduleKey();
                    ASSERT_TRUE( !CCoreGlobal::Instance()->GetTaskManager().GetJobScheduler( hostName ).IsNULL() );
                    process.Stop();
                    ASSERT_TRUE( CCoreGlobal::Instance()->GetTaskManager().GetJobScheduler( hostName ).IsNULL() );
                }
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        process.Stop();
    }
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}
}

/*-------------------------------------------------------------------------*/

void
PerformLibrarySyncVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "LibrarySyncVfsTables" );

    SSyncFixture fixture;
    fixture.label = "ls_vfstables";
    fixture.base = "astest/ls_vfstables";
    MountFreshDirectory( "ls_vfstables_catalogue", "astest/ls_vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/ls_vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunLibrarySyncStoreSuite( fixture );
    UnmountDirectory( "ls_vfstables_catalogue" );

    SSyncFixture service;
    service.label = "lss_vfstables";
    service.base = "astest/lss_vfstables";
    MountFreshDirectory( "lss_vfstables_catalogue", "astest/lss_vfstables/catalogue" );
    service.database = fixture.database;
    service.database.database = "astest/lss_vfstables/catalogue/db";
    RunLibrarySyncServiceSuite( service );
    UnmountDirectory( "lss_vfstables_catalogue" );

    SSyncFixture remote;
    remote.label = "lsr_vfstables";
    remote.base = "astest/lsr_vfstables";
    MountFreshDirectory( "lsr_vfstables_catalogue", "astest/lsr_vfstables/catalogue" );
    remote.database = fixture.database;
    remote.database.database = "astest/lsr_vfstables/catalogue/db";
    RunLibrarySyncRemoteSuite( remote );
    UnmountDirectory( "lsr_vfstables_catalogue" );

    SSyncFixture restart;
    restart.label = "lsk_vfstables";
    restart.base = "astest/lsk_vfstables";
    MountFreshDirectory( "lsk_vfstables_catalogue", "astest/lsk_vfstables/catalogue" );
    restart.database = fixture.database;
    restart.database.database = "astest/lsk_vfstables/catalogue/db";
    RunLibrarySyncRestartSuite( restart );
    UnmountDirectory( "lsk_vfstables_catalogue" );

    SSyncFixture disk;
    disk.label = "lsd_vfstables";
    disk.base = "astest/lsd_vfstables";
    MountFreshDirectory( "lsd_vfstables_catalogue", "astest/lsd_vfstables/catalogue" );
    disk.database = fixture.database;
    disk.database.database = "astest/lsd_vfstables/catalogue/db";
    RunLibrarySyncDiskSuite( disk );
    UnmountDirectory( "lsd_vfstables_catalogue" );

    SSyncFixture schedule;
    schedule.label = "lsj_vfstables";
    schedule.base = "astest/lsj_vfstables";
    MountFreshDirectory( "lsj_vfstables_catalogue", "astest/lsj_vfstables/catalogue" );
    schedule.database = fixture.database;
    schedule.database.database = "astest/lsj_vfstables/catalogue/db";
    RunLibrarySyncScheduleSuite( schedule );
    UnmountDirectory( "lsj_vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformLibrarySyncPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "LibrarySyncPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "LibrarySyncPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the library sync against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    {
        CString reachReason;
        if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "LibrarySyncPostgreSql: the test database cannot be reached: " << reachReason );
            GUCEF_TESTFW_TESTCASE( "PostgreSQL: the test database can be reached" )
                try
                {
                    ASSERT_TRUE( reachReason.IsNULLOrEmpty() );
                }
                catch ( ... )
                {
                    ERRORHERE;
                }
            GUCEF_TESTFW_TESTCASE_END;
            return;
        }
    }

    SSyncFixture fixture;
    fixture.label = "ls_postgresql";
    fixture.base = "astest/ls_postgresql";
    const CString schema = MakeTestSchemaName();
    fixture.database.backendName = "postgresql";
    CValueList parsed;
    CString error;
    if ( DBPLUGIN::POSTGRESQL::CPostgreSqlSettings::ParseConnectionString( connectionString, parsed, error ) )
        fixture.database.database = parsed.GetValueAlways( CString( "dbname" ) ).AsString();
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "connectionString", CVariant( connectionString ) );
    fixture.database.backendOptions.Set( "schema", CVariant( schema ) );
    fixture.database.backendOptions.Set( "createSchema", CVariant( "true" ) );
    fixture.database.backendOptions.Set( "applicationName", CVariant( "gucefASSETSTORE_TestApp" ) );
    fixture.database.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
    fixture.database.backendOptions.Set( "maxRowsPerStatement", CVariant( "500" ) );

    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        fixture.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing in schema \"" << schema << "\"" );
    RunLibrarySyncStoreSuite( fixture );

    SSyncFixture service = fixture;
    service.label = "lss_postgresql";
    service.base = "astest/lss_postgresql";
    const CString serviceSchema = MakeTestSchemaName() + "s";
    service.database.backendOptions.Set( "schema", CVariant( serviceSchema ) );
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing the service in schema \"" << serviceSchema << "\"" );
    RunLibrarySyncServiceSuite( service );

    SSyncFixture remote = fixture;
    remote.label = "lsr_postgresql";
    remote.base = "astest/lsr_postgresql";
    const CString remoteSchema = MakeTestSchemaName() + "r";
    remote.database.backendOptions.Set( "schema", CVariant( remoteSchema ) );
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing the remote path in schema \"" << remoteSchema << "\"" );
    RunLibrarySyncRemoteSuite( remote );

    SSyncFixture restart = fixture;
    restart.label = "lsk_postgresql";
    restart.base = "astest/lsk_postgresql";
    const CString restartSchema = MakeTestSchemaName() + "k";
    restart.database.backendOptions.Set( "schema", CVariant( restartSchema ) );
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing the restart in schema \"" << restartSchema << "\"" );
    RunLibrarySyncRestartSuite( restart );

    SSyncFixture disk = fixture;
    disk.label = "lsd_postgresql";
    disk.base = "astest/lsd_postgresql";
    const CString diskSchema = MakeTestSchemaName() + "d";
    disk.database.backendOptions.Set( "schema", CVariant( diskSchema ) );
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing the staging and the space rules in schema \"" << diskSchema << "\"" );
    RunLibrarySyncDiskSuite( disk );

    SSyncFixture schedule = fixture;
    schedule.label = "lsj_postgresql";
    schedule.base = "astest/lsj_postgresql";
    const CString scheduleSchema = MakeTestSchemaName() + "j";
    schedule.database.backendOptions.Set( "schema", CVariant( scheduleSchema ) );
    GUCEF_LOG( LOGLEVEL_NORMAL, "LibrarySyncPostgreSql: testing the schedule in schema \"" << scheduleSchema << "\"" );
    RunLibrarySyncScheduleSuite( schedule );

    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schemas of the run of the library sync suites are dropped" )
        try
        {
            CPrimaryAssetStoreSettings settings;
            settings.database = fixture.database;
            if ( GUCEF_NULL != fixture.credentials )
            {
                settings.credentialProvider = "gucefASSETSTORE.test";
                settings.credentialAccount = "catalogue";
            }
            COMCORE::CDatabaseClientSettings databaseSettings;
            ASSERT_TRUE( settings.GetDatabaseSettingsWithCredentials( fixture.credentials, databaseSettings, error ) );
            COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
            ASSERT_TRUE( !client.IsNULL() && client->Open( databaseSettings ) );
            if ( !client.IsNULL() && client->IsOpen() )
            {
                DBPLUGIN::POSTGRESQL::CPostgreSqlConnectionPtr connection = static_cast< DBPLUGIN::POSTGRESQL::CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
                ASSERT_TRUE( !connection.IsNULL() );
                if ( !connection.IsNULL() )
                {
                    MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                    CString dropError;
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", dropError ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( serviceSchema ) + " CASCADE", dropError ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( remoteSchema ) + " CASCADE", dropError ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( restartSchema ) + " CASCADE", dropError ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( diskSchema ) + " CASCADE", dropError ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( scheduleSchema ) + " CASCADE", dropError ) );
                }
                client->Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
