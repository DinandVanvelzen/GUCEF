/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */
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
#ifndef GUCEF_ASSETSTORE_H
#include "gucefASSETSTORE.h"
#define GUCEF_ASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define ASSETCONVERTERLIB_H
#endif /* ASSETCONVERTERLIB_H ? */
#ifndef ASSETSTORECONVERTERHOOK_H
#include "AssetStoreConverterHook.h"
#define ASSETSTORECONVERTERHOOK_H
#endif /* ASSETSTORECONVERTERHOOK_H ? */
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
#ifndef TESTHOOKSUPPORT_H
#include "TestHookSupport.h"
#define TESTHOOKSUPPORT_H
#endif /* TESTHOOKSUPPORT_H ? */
#include "TestHookConversion.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace GUCEF::ASSETSTORECONVERTER;
using namespace HOOKTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The observer of a conversion of the tests: counts the progress, and asks to stop at once or after some assets */
class CTestObserver : public CIAssetConversionObserver
{
    public:

    UInt32 progress;
    Int32 cancelAfter;                  /**< -1 never, 0 from the start, n after n assets are done */

    explicit CTestObserver( Int32 stopAfter ) : CIAssetConversionObserver(), progress( 0 ), cancelAfter( stopAfter ) {}

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return cancelAfter >= 0 && progress >= (UInt32) cancelAfter;
    }

    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++progress;
    }
};

CAssetRecord
MakeModelAsset( const CString& name, const CString& path )
{
    CAssetRecord asset;
    asset.assetId = "as" + name;
    asset.packId = "pkDirect";
    asset.name = name;
    asset.kind = "static-model";
    asset.sourceFormat = "gltf";
    asset.originalPath = path;
    return asset;
}

CAssetPackRecord
MakePackWithLicence( const CString& licenceClass, const CString& licenceName, const CString& restriction )
{
    CAssetPackRecord pack;
    pack.packId = "pkDirect";
    pack.sourceId = "disk";
    pack.title = "Direct pack";
    pack.licence.licenceClass = licenceClass;
    pack.licence.name = licenceName;
    if ( !restriction.IsNULLOrEmpty() )
        pack.licence.restrictions.insert( restriction );
    return pack;
}

bool
HasConverter( const CAssetConversionRecord& record, const CString& key )
{
    for ( CString::StringSet::const_iterator e=record.converters.begin(); e!=record.converters.end(); ++e )
        if ( 0 == (*e).HasSubstr( key + "/", true ) )
            return true;
    return false;
}

bool
HasProblemWith( const CString::StringVector& problems, const CString& part )
{
    for ( size_t p=0; p<problems.size(); ++p )
        if ( problems[ p ].HasSubstr( part ) >= 0 )
            return true;
    return false;
}

const CAssetRecord*
FindAsset( const TAssetRecordVector& assets, const CString& name )
{
    for ( size_t a=0; a<assets.size(); ++a )
        if ( assets[ a ].name == name )
            return &assets[ a ];
    return GUCEF_NULL;
}

/** What one run of the suite works with */
struct SHookFixture
{
    CString label;
    CString base;                               /**< VFS root of the folders of the run */
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;

    SHookFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

CPrimaryAssetStoreSettings
MakeSettings( const SHookFixture& fixture )
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

TAssetStoreStatus
IngestPack( CPrimaryAssetStore& store, const CString& folder, const CString& keep, CAssetIngestResult& result )
{
    CAssetIngestRequest request;
    request.sourceId = "disk";
    request.contentPath = folder;
    request.keep = keep;
    return store.Ingest( request, result );
}

/** The suite, against the database of the fixture */
void
RunHookSuite( SHookFixture& fixture )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString& base = fixture.base;
    MountFreshDirectory( fixture.label + "_hk_main", base + "/main" );
    MountFreshDirectory( fixture.label + "_hk_source", base + "/source" );

    bool sourcesWritten = WriteGltfPack( base + "/source/Hook_Both", "Hook Both", "{ \"name\": \"CC BY 4.0\" }", 2 );
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Only", "Hook Only", "{ \"name\": \"CC BY 4.0\" }", 2 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_NoLicence", "Hook No Licence", CString::Empty, 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Custom", "Hook Custom", "{ \"name\": \"Studio Custom Terms\" }", 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Reviewed", "Hook Reviewed", "{ \"name\": \"Studio Custom Terms\", \"restrictions\": [ \"reviewed\" ] }", 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_UeOnly", "Hook Ue Only", "{ \"name\": \"Standard Licence\", \"restrictions\": [ \"ueOnly\" ] }", 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Reference", "Hook Reference", "{ \"name\": \"Standard Licence\", \"restrictions\": [ \"referenceOnly\" ] }", 1 ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Broken", "Hook Broken", "{ \"name\": \"CC BY 4.0\" }", 2 ) && sourcesWritten;
    sourcesWritten = WriteVfsFile( base + "/source/Hook_Broken/Models/SM_Tri_02.gltf", CString( "this is not JSON at all" ) ) && sourcesWritten;
    sourcesWritten = WriteGltfPack( base + "/source/Hook_Later", "Hook Later", "{ \"name\": \"CC BY 4.0\" }", 2 ) && sourcesWritten;

    CPrimaryAssetStore store;
    store.SetSettings( MakeSettings( fixture ) );
    store.SetCredentialStore( fixture.credentials );
    CPipelineConverterHook hook;
    store.SetConverterHook( &hook );
    CString bothId;
    CString laterId;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the store opens with the hook on the pipeline" ).C_String() )
        try
        {
            ASSERT_TRUE( sourcesWritten );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.Open() );
            ASSERT_TRUE( "desktop" == hook.GetProfileName() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep both: a pack of glTF files is converted by the pipeline during the ingest, the originals stay" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Hook_Both", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == result.state && 3 == result.assetCount );
            bothId = result.packId;
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( bothId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && pack.stateReason.IsNULLOrEmpty() );
            ASSERT_TRUE( pack.formatsConverted.end() != pack.formatsConverted.find( "gltf" ) );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome && "desktop" == pack.conversion.profile && 2 == pack.conversion.convertedCount );
            ASSERT_TRUE( HasConverter( pack.conversion, "importer:gltf" ) && HasConverter( pack.conversion, "exporter:gltf" ) && HasConverter( pack.conversion, "converter:AssetConverterLib" ) );
            ASSERT_TRUE( pack.conversion.reportPath == "converted/desktop/report.json" );

            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Models/SM_Tri_01.gltf" ) && vfs.FileExists( root + "/original/Models/tex.png" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/desktop/report.json" ) && vfs.FileExists( root + "/converted/desktop/report.txt" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/desktop/pack.assetpack.json" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/desktop/licence/" + CLicenceRecordBuilder::GetRecordId( pack ) + ".licence.json" ) );
            const CString licenceText = ReadVfsText( root + "/converted/desktop/licence/" + CLicenceRecordBuilder::GetRecordId( pack ) + ".licence.json" );
            ASSERT_TRUE( licenceText.HasSubstr( "\"store\": \"disk\"" ) >= 0 && licenceText.HasSubstr( "CC BY 4.0" ) >= 0 );

            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( bothId, assets ) && 3 == assets.size() );
            const CAssetRecord* model = FindAsset( assets, "SM_Tri_01" );
            const CAssetRecord* texture = FindAsset( assets, "tex" );
            ASSERT_TRUE( GUCEF_NULL != model && GUCEF_NULL != texture );
            if ( GUCEF_NULL != model && GUCEF_NULL != texture )
            {
                ASSERT_TRUE( "gltf" == model->convertedFormat && model->convertedPath.HasSubstr( "converted/desktop/" ) == 0 && model->convertedPath.HasSubstr( ".gltf" ) > 0 );
                ASSERT_TRUE( texture->convertedPath.IsNULLOrEmpty() );

                // the converted file resolves from the catalogue and is a glTF file
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetConvertedVfsPath( model->assetId, vfsPath, message ) );
                ASSERT_TRUE( vfs.FileExists( vfsPath ) && vfsPath == root + "/" + model->convertedPath );
                const CString gltf = ReadVfsText( vfsPath );
                ASSERT_TRUE( gltf.HasSubstr( "\"asset\"" ) >= 0 && gltf.HasSubstr( "\"2.0\"" ) >= 0 && gltf.HasSubstr( "SM_Tri_01" ) >= 0 );
            }

            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, bothId ) && 1 == history.size() && "ingested" == history[ 0 ].outcome );
            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) && report.IsIntact() );

            bool stale = true;
            CString reason;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( bothId, stale, reason ) && !stale );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": keep converted: the glTF originals go after a clean report, the picture stays" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Hook_Only", "converted", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == result.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_FALSE( vfs.FileExists( root + "/original/Models/SM_Tri_01.gltf" ) );
            ASSERT_FALSE( vfs.FileExists( root + "/original/Models/SM_Tri_02.gltf" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/original/Models/tex.png" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( result.packId, assets ) );
            const CAssetRecord* model = FindAsset( assets, "SM_Tri_02" );
            ASSERT_TRUE( GUCEF_NULL != model && model->files.empty() && !model->convertedPath.IsNULLOrEmpty() );
            if ( GUCEF_NULL != model )
            {
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetConvertedVfsPath( model->assetId, vfsPath, message ) && vfs.FileExists( vfsPath ) );
            }
            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.VerifyLocation( "main", report ) && report.IsIntact() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack without a licence record is refused by the converter: ingested as original, never a failed ingest" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Hook_NoLicence", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_REFUSED == result.state );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( pack.stateReason == "conversion refused: the pack has no licence record in the catalogue" );
            ASSERT_TRUE( HasProblemWith( result.problems, "conversion refused: the pack has no licence record in the catalogue" ) );
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Models/SM_Tri_01.gltf" ) && !vfs.DirExists( root + "/converted" ) );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeRefused == pack.conversion.outcome && pack.formatsConverted.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the licence flags decide: non-standard without review, ueOnly and referenceOnly refuse; reviewed converts" ).C_String() )
        try
        {
            struct SCase { const char* folder; bool refused; const char* part; };
            const SCase cases[] =
            {
                { "Hook_Custom", true, "restricted or non-standard and it is not reviewed" },
                { "Hook_UeOnly", true, "marks the pack ueOnly" },
                { "Hook_Reference", true, "marks the pack referenceOnly" },
                { "Hook_Reviewed", false, "" }
            };
            for ( UInt32 c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
            {
                CAssetIngestResult result;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/" + cases[ c ].folder, "both", result ) );
                CAssetPackRecord pack;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
                if ( cases[ c ].refused )
                {
                    ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_REFUSED == pack.state && ASSETSTORE_STATUS_OK == result.status );
                    ASSERT_TRUE( pack.stateReason.HasSubstr( "conversion refused: " ) == 0 && pack.stateReason.HasSubstr( cases[ c ].part ) > 0 );
                    ASSERT_TRUE( pack.formatsConverted.empty() );
                }
                else
                {
                    ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && !pack.formatsConverted.empty() );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": an asset that fails to convert fails the conversion: the pack stays as original, pending with the reason, and nothing of the run stays" ).C_String() )
        try
        {
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Hook_Broken", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state && ASSETSTORE_STATUS_OK == result.status );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( result.packId, pack ) );
            ASSERT_TRUE( pack.stateReason.HasSubstr( "conversion failed: " ) == 0 && pack.stateReason.HasSubstr( "error(s) in the conversion report" ) > 0 );
            ASSERT_TRUE( CAssetConversionRecord::OutcomeFailed == pack.conversion.outcome && pack.formatsConverted.empty() && pack.conversion.failedCount >= 1 );
            ASSERT_TRUE( HasProblemWith( result.problems, "SM_Tri_02" ) && HasProblemWith( result.problems, "kept as original" ) );
            const CString root = base + "/main/" + pack.rootPath;
            ASSERT_TRUE( vfs.FileExists( root + "/original/Models/SM_Tri_01.gltf" ) && vfs.FileExists( root + "/original/Models/SM_Tri_02.gltf" ) );
            ASSERT_TRUE( vfs.FileExists( root + "/converted/desktop/report.json" ) );
            ASSERT_FALSE( vfs.DirExists( root + "/converted/desktop/intermediate" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( result.packId, assets ) );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a pack ingested without the converter is converted later, as the keep policy says" ).C_String() )
        try
        {
            store.SetConverterHook( GUCEF_NULL );
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == IngestPack( store, base + "/source/Hook_Later", "both", result ) );
            ASSERT_TRUE( ASSETPACK_STATE_CONVERSION_PENDING == result.state );
            laterId = result.packId;
            store.SetConverterHook( &hook );

            CFutureResult future = store.ConvertPackAsync( laterId );
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( GetFutureResult< CAssetConversionOutcome >( future, outcome ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == outcome.status && ASSETPACK_STATE_INGESTED == outcome.state && 2 == outcome.record.convertedCount );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && !pack.formatsConverted.empty() && HasConverter( pack.conversion, "importer:gltf" ) );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( laterId, assets ) );
            const CAssetRecord* model = FindAsset( assets, "SM_Tri_01" );
            ASSERT_TRUE( GUCEF_NULL != model );
            if ( GUCEF_NULL != model )
            {
                CString vfsPath, message;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssetConvertedVfsPath( model->assetId, vfsPath, message ) && vfs.FileExists( vfsPath ) );
            }
            GUCEF::vector< CAssetIngestHistoryEntry > history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetIngestHistory( history, laterId ) && 2 == history.size() && "converted" == history[ 1 ].outcome );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": the pending packs of a store are converted in one run: the packs the converter refuses are counted" ).C_String() )
        try
        {
            CAssetConversionOutcome batch;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.ConvertPendingPacks( batch ) );
            // the broken pack is the one pending pack; it fails again, nothing else is pending
            ASSERT_TRUE( 0 == batch.packsConverted && 1 == batch.packsFailed && 0 == batch.packsRefused );
            CString::StringVector pending;
            store.GetPacksPendingConversion( pending );
            ASSERT_TRUE( 1 == pending.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": a conversion with other converter settings than the recorded ones is stale; a failed new conversion leaves the earlier one" ).C_String() )
        try
        {
            bool stale = true;
            CString reason;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && !stale );
            CConverterHookSettings settings = hook.GetSettings();
            settings.cookTarget = "ogre";
            hook.SetSettings( settings );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && stale );
            ASSERT_TRUE( reason.HasSubstr( "cook target ogre" ) >= 0 );
            CString::StringVector stalePacks;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetStaleConversions( stalePacks ) && stalePacks.size() >= 3 );

            // the cooker is not loaded here: the new conversion fails, the earlier one stays as it was
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_FAILED == store.ConvertPack( laterId, outcome ) );
            ASSERT_TRUE( HasProblemWith( outcome.problems, "no cooker" ) || outcome.message.HasSubstr( "error(s)" ) >= 0 );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetPack( laterId, pack ) );
            ASSERT_TRUE( ASSETPACK_STATE_INGESTED == pack.state && CAssetConversionRecord::OutcomeConverted == pack.conversion.outcome && !pack.formatsConverted.empty() );
            TAssetRecordVector assets;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.GetAssets( laterId, assets ) );
            for ( size_t a=0; a<assets.size(); ++a )
                if ( assets[ a ].kind == "static-model" )
                    ASSERT_TRUE( !assets[ a ].convertedPath.IsNULLOrEmpty() );

            settings.cookTarget.Clear();
            hook.SetSettings( settings );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == store.IsConversionStale( laterId, stale, reason ) && !stale );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    store.SetConverterHook( GUCEF_NULL );
    store.Close();
}

CString
Text( UInt32 value )
{
    return CORE::ToString( value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformHookUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConverterHook.Unit" );

    GUCEF_TESTFW_TESTCASE( "Settings: defaults, a round trip through a data tree, and a tree that holds nothing" )
        try
        {
            CConverterHookSettings defaults;
            ASSERT_TRUE( "desktop" == defaults.cookProfile.name && "auto" == defaults.importer && "gltf" == defaults.exporter && defaults.cookTarget.IsNULLOrEmpty() );
            ASSERT_TRUE( defaults.convertibleKinds.end() != defaults.convertibleKinds.find( "static-model" ) && defaults.nonStandardLicenceClasses.end() != defaults.nonStandardLicenceClasses.find( "unknown" ) );

            CConverterHookSettings custom;
            custom.cookProfile.name = "android";
            custom.cookProfile.maxTextureSize = 1024;
            custom.cookTarget = "ogre";
            custom.threads = 3;
            custom.mainExtensions.clear();
            custom.mainExtensions.push_back( "mesh" );
            custom.nonStandardLicenceClasses.clear();
            CDataNode node( "conversion", GUCEF_DATATYPE_OBJECT );
            CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( custom.Serialize( node, serializerSettings ) );
            CConverterHookSettings back;
            ASSERT_TRUE( back.Deserialize( node, serializerSettings ) );
            ASSERT_TRUE( "android" == back.cookProfile.name && 1024 == back.cookProfile.maxTextureSize && "ogre" == back.cookTarget && 3 == back.threads );
            ASSERT_TRUE( 1 == back.mainExtensions.size() && "mesh" == back.mainExtensions[ 0 ] );

            CDataNode empty( "conversion", GUCEF_DATATYPE_OBJECT );
            CConverterHookSettings kept;
            ASSERT_TRUE( kept.Deserialize( empty, serializerSettings ) );
            ASSERT_TRUE( "desktop" == kept.cookProfile.name && "auto" == kept.importer && kept.mainExtensions.size() == defaults.mainExtensions.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Licence record: made from the licence of the pack, the flags map, the converter applies its rules" )
        try
        {
            CConverterHookSettings settings;
            ASSETCONVERTER::CLicenceRecord record;
            CString problem;

            // no licence at all
            ASSERT_TRUE( !CLicenceRecordBuilder::Build( MakePackWithLicence( "unknown", CString::Empty, CString::Empty ), settings, record, problem ) );
            ASSERT_TRUE( "the pack has no licence record in the catalogue" == problem );

            // a licence with a standard class
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", CString::Empty ), settings, record, problem ) );
            ASSERT_TRUE( !record.ueOnly && !record.referenceOnly && !record.restrictedOrNonStandard && !record.reviewed && 1 == record.seats );
            ASSERT_TRUE( "CC BY 4.0" == record.licenceName && "disk" == record.store && "Direct pack" == record.product && !record.id.IsNULLOrEmpty() );
            CString reason;
            ASSERT_TRUE( record.AllowsConversion( reason ) );

            // the flags
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", "ueOnly" ), settings, record, problem ) && record.ueOnly && !record.AllowsConversion( reason ) );
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", "referenceOnly" ), settings, record, problem ) && record.referenceOnly && !record.AllowsConversion( reason ) );
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", "restrictedOrNonStandard" ), settings, record, problem ) && record.restrictedOrNonStandard && !record.AllowsConversion( reason ) );
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", "humbleOneSeat" ), settings, record, problem ) && record.humbleOneSeat && record.AllowsConversion( reason ) );
            ASSERT_TRUE( record.restrictions.empty() );

            // the other flags of the store's licence reach the record by name, without changing the conversion
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "attribution", "CC BY 4.0", "noMachineLearning" ), settings, record, problem ) );
            ASSERT_TRUE( record.HasRestriction( ASSETCONVERTER::CLicenceRecord::NoMachineLearning ) && 1 == record.restrictions.size() && record.AllowsConversion( reason ) );

            // a class that is non-standard until it is reviewed
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "unknown", "Studio Custom Terms", CString::Empty ), settings, record, problem ) );
            ASSERT_TRUE( record.restrictedOrNonStandard && !record.reviewed && !record.AllowsConversion( reason ) );
            CAssetPackRecord reviewed = MakePackWithLicence( "unknown", "Studio Custom Terms", "reviewed" );
            ASSERT_TRUE( CLicenceRecordBuilder::Build( reviewed, settings, record, problem ) && record.restrictedOrNonStandard && record.reviewed && record.AllowsConversion( reason ) );

            // the classes that count as non-standard are a setting
            settings.nonStandardLicenceClasses.clear();
            ASSERT_TRUE( CLicenceRecordBuilder::Build( MakePackWithLicence( "unknown", "Studio Custom Terms", CString::Empty ), settings, record, problem ) && !record.restrictedOrNonStandard && record.AllowsConversion( reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Manifest: the folders of the conversion, the profile as the one cook profile, the cook target when there is one" )
        try
        {
            CPipelineConverterHook hook;
            CAssetConversionRequest request;
            request.pack = MakePackWithLicence( "attribution", "CC BY 4.0", CString::Empty );
            request.sourceVfsPath = "hooktest/pack/original";
            request.convertedVfsFolder = "hooktest/pack/converted/desktop";
            request.convertedPathPrefix = "converted/desktop";
            ASSETCONVERTER::CAssetPackManifest manifest;
            CString problem;
            ASSERT_TRUE( hook.BuildManifest( request, manifest, problem ) );
            ASSERT_TRUE( "pkDirect" == manifest.id && "disk" == manifest.store && "auto" == manifest.importer && "gltf" == manifest.exporter );
            ASSERT_TRUE( "hooktest/pack/original" == manifest.GetSourcePath() );
            ASSERT_TRUE( "hooktest/pack/converted/desktop/intermediate" == manifest.GetIntermediatePath() );
            ASSERT_TRUE( "hooktest/pack/converted" == manifest.GetOutputPath() );
            ASSERT_TRUE( manifest.GetLicenceRecordPath() == "hooktest/pack/converted/desktop/licence/" + CLicenceRecordBuilder::GetRecordId( request.pack ) + ".licence.json" );
            ASSERT_TRUE( manifest.cookTargets.empty() && 1 == manifest.cookProfiles.size() && "desktop" == manifest.cookProfiles[ 0 ].name );

            CConverterHookSettings settings = hook.GetSettings();
            settings.cookTarget = "ogre";
            settings.cookProfile.name = "web";
            hook.SetSettings( settings );
            ASSERT_TRUE( hook.BuildManifest( request, manifest, problem ) );
            ASSERT_TRUE( 1 == manifest.cookTargets.size() && "ogre" == manifest.cookTargets[ 0 ] && "web" == manifest.cookProfiles[ 0 ].name );
            // the folder is not named after the profile: the output is the folder itself
            ASSERT_TRUE( "hooktest/pack/converted/desktop" == manifest.GetOutputPath() );
            request.convertedVfsFolder.Clear();
            ASSERT_FALSE( hook.BuildManifest( request, manifest, problem ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Versions: a part with another version, a part that is gone, a cook target that came or went make a conversion stale" )
        try
        {
            CPipelineConverterHook hook;
            UInt32 gltfVersion = 0;
            UInt32 converterVersion = 0;
            ASSERT_TRUE( ASSETCONVERTER::CAssetPipeline::GetCurrentPluginVersion( "importer:gltf", gltfVersion ) );
            ASSERT_TRUE( ASSETCONVERTER::CAssetPipeline::GetCurrentPluginVersion( "converter:AssetConverterLib", converterVersion ) );
            CAssetConversionRecord record;
            record.outcome = CAssetConversionRecord::OutcomeConverted;
            record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "importer:gltf", gltfVersion ) );
            record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "converter:AssetConverterLib", converterVersion ) );
            CString reason;
            ASSERT_TRUE( !hook.IsConversionStale( record, reason ) );

            record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "importer:gltf", gltfVersion + 1 ) );
            record.converters.erase( CAssetConversionRecord::MakeConverterEntry( "importer:gltf", gltfVersion ) );
            ASSERT_TRUE( hook.IsConversionStale( record, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "importer:gltf" ) >= 0 && reason.HasSubstr( "version " + Text( gltfVersion ) ) >= 0 );

            record.converters.clear();
            record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "importer:nosuchformat", 1 ) );
            ASSERT_TRUE( hook.IsConversionStale( record, reason ) && reason.HasSubstr( "is not loaded" ) >= 0 );

            // the cooker of the settings
            record.converters.clear();
            record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "converter:AssetConverterLib", converterVersion ) );
            CConverterHookSettings settings = hook.GetSettings();
            settings.cookTarget = "ogre";
            hook.SetSettings( settings );
            ASSERT_TRUE( hook.IsConversionStale( record, reason ) && reason.HasSubstr( "cook target ogre" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Hook: refused packs write nothing, cancel stops the run before and during, a run reports its progress" )
        try
        {
            MountFreshDirectory( "hk_direct", "hooktest/direct" );
            ASSERT_TRUE( WriteGltfPack( "hooktest/direct/src", "Direct", "{ \"name\": \"CC BY 4.0\" }", 4 ) );
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CPipelineConverterHook hook;
            ASSERT_TRUE( hook.CanConvert( MakeModelAsset( "SM_Tri_01", "Models/SM_Tri_01.gltf" ) ) );
            CAssetRecord texture;
            texture.kind = "texture";
            ASSERT_FALSE( hook.CanConvert( texture ) );

            TAssetRecordVector assets;
            for ( UInt32 m=1; m<=4; ++m )
                assets.push_back( MakeModelAsset( "SM_Tri_0" + Text( m ), "Models/SM_Tri_0" + Text( m ) + ".gltf" ) );

            CAssetConversionRequest request;
            request.sourceVfsPath = "hooktest/direct/src";
            request.convertedPathPrefix = "converted/desktop";

            // refused: no licence, nothing is written
            request.pack = MakePackWithLicence( "unknown", CString::Empty, CString::Empty );
            request.convertedVfsFolder = "hooktest/direct/refused/converted/desktop";
            CAssetConversionOutcome outcome;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == hook.ConvertPack( request, assets, outcome ) );
            ASSERT_TRUE( outcome.message == "the pack has no licence record in the catalogue" );
            ASSERT_FALSE( vfs.DirExists( "hooktest/direct/refused" ) );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );

            // asked to stop before it starts
            request.pack = MakePackWithLicence( "attribution", "CC BY 4.0", CString::Empty );
            request.convertedVfsFolder = "hooktest/direct/cancelled/converted/desktop";
            CTestObserver stopped( 0 );
            request.observer = &stopped;
            outcome = CAssetConversionOutcome();
            ASSERT_TRUE( ASSETSTORE_STATUS_CANCELLED == hook.ConvertPack( request, assets, outcome ) );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );
            ASSERT_TRUE( vfs.FileExists( "hooktest/direct/cancelled/converted/desktop/report.json" ) );
            ASSERT_TRUE( ReadVfsText( "hooktest/direct/cancelled/converted/desktop/report.json" ).HasSubstr( "\"cancelled\": true" ) >= 0 );

            // asked to stop after the first asset
            request.convertedVfsFolder = "hooktest/direct/midway/converted/desktop";
            CTestObserver midway( 1 );
            request.observer = &midway;
            outcome = CAssetConversionOutcome();
            ASSERT_TRUE( ASSETSTORE_STATUS_CANCELLED == hook.ConvertPack( request, assets, outcome ) );
            ASSERT_TRUE( midway.progress >= 1 && midway.progress <= 4 );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( assets[ a ].convertedPath.IsNULLOrEmpty() );

            // not stopped: every asset reports and gets its converted file
            request.convertedVfsFolder = "hooktest/direct/done/converted/desktop";
            CTestObserver watching( -1 );
            request.observer = &watching;
            outcome = CAssetConversionOutcome();
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == hook.ConvertPack( request, assets, outcome ) );
            ASSERT_TRUE( 4 == watching.progress && 4 == outcome.record.convertedCount && 0 == outcome.record.failedCount );
            for ( size_t a=0; a<assets.size(); ++a )
            {
                ASSERT_TRUE( assets[ a ].convertedPath.HasSubstr( "converted/desktop/" ) == 0 && "gltf" == assets[ a ].convertedFormat );
                ASSERT_TRUE( vfs.FileExists( "hooktest/direct/done/" + assets[ a ].convertedPath ) );
            }
            ASSERT_TRUE( outcome.record.reportPath == "converted/desktop/report.json" && HasConverter( outcome.record, "importer:gltf" ) );

            // the same again: the cache of the converter answers
            CAssetConversionOutcome again;
            CTestObserver watchingAgain( -1 );
            request.observer = &watchingAgain;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == hook.ConvertPack( request, assets, again ) );
            ASSERT_TRUE( 4 == again.record.cachedCount && 0 == again.record.convertedCount );
            for ( size_t a=0; a<assets.size(); ++a )
                ASSERT_TRUE( !assets[ a ].convertedPath.IsNULLOrEmpty() );
            UnmountDirectory( "hk_direct" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformHookPrimaryStoreVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConverterHook.VfsTables" );

    SHookFixture fixture;
    fixture.label = "vfstables";
    fixture.base = "hooktest/vfstables";
    MountFreshDirectory( "vfstables_catalogue", "hooktest/vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "hooktest/vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunHookSuite( fixture );
    UnmountDirectory( "vfstables_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformHookPrimaryStorePostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ConverterHook.PostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "ConverterHook.PostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the converter hook against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }

    SHookFixture fixture;
    fixture.label = "postgresql";
    fixture.base = "hooktest/postgresql";
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
    fixture.database.backendOptions.Set( "applicationName", CVariant( "AssetStoreConverterHook_TestApp" ) );
    fixture.database.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
    fixture.database.backendOptions.Set( "maxRowsPerStatement", CVariant( "500" ) );

    // the password of the environment goes through a credential store, as the settings of an application would have it
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
    {
        AUTH::CSecretString secret( ( CString( password ) ) );
        credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
        fixture.credentials = &credentials;
    }
    GUCEF_LOG( LOGLEVEL_NORMAL, "ConverterHook.PostgreSql: testing in schema \"" << schema << "\"" );
    RunHookSuite( fixture );

    // the schema of this run goes
    GUCEF_TESTFW_TESTCASE( "PostgreSQL: the schema of the converter hook run is dropped" )
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
