/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterPipeline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

bool
RunPack( const CORE::CString& manifestPath, CAssetPipeline& pipeline, CConversionReport& report, const CAssetPipelineOptions& options = CAssetPipelineOptions() )
{
    CAssetPackManifest manifest;
    if ( !manifest.Load( manifestPath ) )
        return false;
    return pipeline.RunPack( manifest, options, report );
}

const CImportActivityEntry*
FindActivity( const CAssetPipeline& pipeline, const CORE::CString& assetPath )
{
    const CImportActivityLog::TEntryVector& entries = pipeline.GetLastRun().entries;
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].assetPath == assetPath )
            return &entries[ i ];
    return GUCEF_NULL;
}

bool
HasActivity( const CAssetPipeline& pipeline, const CORE::CString& assetPath, TActivityAction action, TActivityReason reason )
{
    const CImportActivityEntry* entry = FindActivity( pipeline, assetPath );
    return GUCEF_NULL != entry && entry->action == action && entry->reason == reason;
}

TAssetStatus
StatusOf( const CConversionReport& report, const CORE::CString& assetPath )
{
    const CConversionReportEntry* entry = report.FindEntry( assetPath );
    return GUCEF_NULL == entry ? ASSETSTATUS_COUNT : entry->GetStatus();
}

bool
VfsFileExists( const CORE::CString& vfsPath )
{
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath );
}

/** The observer of a run of the tests: counts the assets that finish and asks to stop after some of them */
class CCountingObserver : public CIAssetPipelineObserver
{
    public:

    UInt32 finished;
    UInt32 lastTotal;
    Int32 stopAfter;                    /**< -1: never */

    explicit CCountingObserver( Int32 stopWhenDone ) : CIAssetPipelineObserver(), finished( 0 ), lastTotal( 0 ), stopAfter( stopWhenDone ) {}

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return stopAfter >= 0 && finished >= (UInt32) stopAfter;
    }

    virtual void OnAssetFinished( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++finished;
        lastTotal = total;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterPipelineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Pipeline" );

    GUCEF_TESTFW_TESTCASE( "Pipeline: a pack converts and cooks; a second run is a cache hit that keeps every file and the reports" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "cachehit", CORE::CString::Empty );
        CStubCooker& cooker = GetStubCooker();
        cooker.cookedAssets.clear();
        const UInt32 importsBefore = GetStubImporter().importCount;

        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && !report.HasFailures() && "rec" == report.GetLicenceRecordId() );
        ASSERT_TRUE( GUCEF_NULL == report.FindEntry( "readme.txt" ) && GUCEF_NULL != report.FindEntry( "props/rock.stub" ) );
        ASSERT_TRUE( importsBefore + 2 == GetStubImporter().importCount && 2 == cooker.cookedAssets.size() );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_NEW ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_NEW ) );

        // the intermediate: the export, the sidecar, the baked textures; the cache entries; the cooked files
        const CORE::CString inter = "work/cachehit/intermediate";
        ASSERT_TRUE( VfsFileExists( inter + "/props/rock.gltf" ) && VfsFileExists( inter + "/props/rock.conversion.json" ) );
        ASSERT_TRUE( VfsFileExists( inter + "/.cache/props/rock.stub.cache.json" ) && VfsFileExists( inter + "/.cache/props/crate.stub.cache.json" ) );
        ASSERT_TRUE( VfsFileExists( inter + "/report.json" ) && VfsFileExists( inter + "/report.txt" ) && VfsFileExists( inter + "/activity.json" ) );
        const CORE::CString out = "work/cachehit/out/desktop";
        ASSERT_TRUE( VfsFileExists( out + "/cachehit/rock.mesh" ) && VfsFileExists( out + "/cachehit/textures/shared.tex" ) );
        ASSERT_TRUE( VfsFileExists( out + "/references.json" ) && VfsFileExists( out + "/sizes.json" ) );

        CORE::CDataNode exported;
        ASSERT_TRUE( CCanonicalJson::Load( inter + "/props/rock.gltf", exported ) );
        CORE::CString::StringVector images;
        exported.GetValuesOfChildByName( "images", images );
        ASSERT_TRUE( 1 == images.size() && VfsFileExists( images[ 0 ] ) && "work/cachehit/intermediate/textures" == CVfsPaths::FolderOf( images[ 0 ] ) );

        // the sidecar: provenance, licence, settings, normalisation, materials with their source, summary, dependencies
        CORE::CDataNode sidecar;
        ASSERT_TRUE( CCanonicalJson::Load( inter + "/props/rock.conversion.json", sidecar ) );
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        ASSERT_TRUE( GUCEF_NULL != provenance && "props/rock.stub" == provenance->GetAttributeValueOrChildValueByName( "assetPath" ).AsString() && "stub" == provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != provenance && "cachehit" == provenance->GetAttributeValueOrChildValueByName( "pack" ).AsString() && "stub" == provenance->GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString() );
        ASSERT_TRUE( "rec" == sidecar.GetAttributeValueOrChildValueByName( "licenceRecord" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != sidecar.FindChild( "settings" ) && GUCEF_NULL != sidecar.FindChild( "normalisation" ) );
        const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
        ASSERT_TRUE( GUCEF_NULL != summary && 12 == summary->GetAttributeValueOrChildValueByName( "triangles" ).AsUInt32() && 1 == summary->GetAttributeValueOrChildValueByName( "materials" ).AsUInt32() );
        CORE::CString::StringVector dependencies;
        sidecar.GetValuesOfChildByName( "dependencies", dependencies );
        ASSERT_TRUE( 1 == dependencies.size() && "props/rock.notes.txt" == dependencies[ 0 ] );
        const CORE::CDataNode* materials = sidecar.FindChild( "materials" );
        ASSERT_TRUE( GUCEF_NULL != materials && 1 == materials->GetNrOfDirectChildNodes() );
        if ( GUCEF_NULL != materials && 1 == materials->GetNrOfDirectChildNodes() )
        {
            const CORE::CDataNode* source = materials->GetFirstChild()->FindChild( "source" );
            ASSERT_TRUE( GUCEF_NULL != source && "Stub/Shader" == source->GetAttributeValueOrChildValueByName( "shader" ).AsString() );
        }
        const CORE::CDataNode* textures = sidecar.FindChild( "textures" );
        ASSERT_TRUE( GUCEF_NULL != textures && 1 == textures->GetNrOfDirectChildNodes() );
        if ( GUCEF_NULL != textures && 1 == textures->GetNrOfDirectChildNodes() )
        {
            CORE::CString::StringVector sources;
            textures->GetFirstChild()->GetValuesOfChildByName( "sources", sources );
            ASSERT_TRUE( 1 == sources.size() && "props/rock.png" == sources[ 0 ] );
            ASSERT_TRUE( textures->GetFirstChild()->GetAttributeValueOrChildValueByName( "baked" ).AsString().HasSubstr( "textures/rock.", 0 ) == 0 );
        }

        // the second run: everything from the cache, nothing imported or cooked again
        const UInt32 importsAfterFirst = GetStubImporter().importCount;
        const UInt32 cookCalls = cooker.cookCalls;
        CConversionReport second;
        ASSERT_TRUE( RunPack( manifest, pipeline, second ) );
        ASSERT_TRUE( 2 == second.GetAssetCount( ASSETSTATUS_CACHED ) && importsAfterFirst == GetStubImporter().importCount && cookCalls == cooker.cookCalls );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_CACHED, ACTIVITYREASON_UNCHANGED ) );
        const CConversionReportEntry* cached = second.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != cached && cached->files.size() >= 6 );

        // the reference graph and the size report of the profile still have both assets
        CReferenceGraph graph;
        ASSERT_TRUE( graph.Load( out + "/references.json" ) );
        ASSERT_TRUE( "desktop" == graph.GetProfile() && graph.GetFiles().find( "cachehit/rock.mesh" ) != graph.GetFiles().end() );
        CReferenceGraph::TFileMap::const_iterator material = graph.GetFiles().find( "cachehit/rock.material" );
        ASSERT_TRUE( material != graph.GetFiles().end() && 2 == (*material).second.references.size() && "props/rock.stub" == (*material).second.asset );
        CSizeReport sizes;
        ASSERT_TRUE( sizes.Load( out + "/sizes.json" ) && 2 == sizes.GetAssets().size() );
        CSizeReport::TAssetMap::const_iterator rock = sizes.GetAssets().find( "props/rock.stub" );
        ASSERT_TRUE( rock != sizes.GetAssets().end() );
        if ( rock != sizes.GetAssets().end() )
        {
            ASSERT_TRUE( CStubCooker::MeshSize + CStubCooker::MaterialSize + CStubCooker::OwnTextureSize == (*rock).second.exclusiveSize );
            ASSERT_TRUE( CStubCooker::SharedTextureSize == (*rock).second.sharedSize && 4 == (*rock).second.fileCount );
        }

        // the activity log has both runs
        CImportActivityLog log;
        ASSERT_TRUE( log.Load( inter + "/activity.json" ) && 2 == log.GetRuns().size() && "cachehit" == log.GetRuns()[ 1 ].packId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: a changed source, setting, plugin version or dependency, a missing output or a forced run invalidates the cache, with the reason" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "invalidate", CORE::CString::Empty );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );

        // the source
        WriteWorkFile( "invalidate/src/props/rock.stub", "{ \"nodes\": [ { \"name\": \"Rock\", \"mesh\": true, \"size\": 3, \"material\": 0 } ], "
                       "\"materials\": [ { \"name\": \"RockMaterial\", \"baseColor\": \"rock.png\" } ], \"dependencies\": [ \"rock.notes.txt\" ], \"approximation\": \"rounded corners\" }" );
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_SOURCE_CHANGED ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_CACHED, ACTIVITYREASON_UNCHANGED ) );
        ASSERT_TRUE( ASSETSTATUS_APPROXIMATED == StatusOf( report, "props/rock.stub" ) );

        // a setting: a preset for the crate only
        WriteWorkFile( "invalidate/pack.assetpack.json", ReadVfsText( manifest ).ReplaceSubstr( "\"cookTargets\"", "\"presets\": [ { \"match\": \"**/crate*\", \"settings\": { \"stubScale\": 2 } } ], \"cookTargets\"" ) );
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_SETTINGS_CHANGED ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_CACHED, ACTIVITYREASON_UNCHANGED ) );

        // the version of the importer, then of the cooker
        GetStubImporter().version = 2;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        const CImportActivityEntry* entry = FindActivity( pipeline, "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && ACTIVITYREASON_VERSION_CHANGED == entry->reason && "importer:stub 1 -> 2" == entry->detail );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_VERSION_CHANGED ) );
        GetStubCooker().version = 3;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        entry = FindActivity( pipeline, "props/crate.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && ACTIVITYREASON_VERSION_CHANGED == entry->reason && "cooker:stubcook 1 -> 3" == entry->detail );
        GetStubImporter().version = 1;
        GetStubCooker().version = 1;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );

        // a dependency
        WriteWorkFile( "invalidate/src/props/rock.notes.txt", "other notes" );
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        entry = FindActivity( pipeline, "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && ACTIVITYREASON_DEPENDENCY_CHANGED == entry->reason && "work/invalidate/src/props/rock.notes.txt" == entry->detail );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_CACHED, ACTIVITYREASON_UNCHANGED ) );

        // a cooked file that is gone
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( "work/invalidate/out/desktop/invalidate/crate.mesh", false ) );
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        entry = FindActivity( pipeline, "props/crate.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && ACTIVITYREASON_OUTPUT_MISSING == entry->reason && "work/invalidate/out/desktop/invalidate/crate.mesh" == entry->detail );
        ASSERT_TRUE( VfsFileExists( "work/invalidate/out/desktop/invalidate/crate.mesh" ) );

        // a forced import
        CAssetPipelineOptions options;
        options.reimport = true;
        ASSERT_TRUE( RunPack( manifest, pipeline, report, options ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_FORCED ) );

        // the log has every run
        CImportActivityLog log;
        ASSERT_TRUE( log.Load( "work/invalidate/intermediate/activity.json" ) && 9 == log.GetRuns().size() );
    }
    catch ( ... )
    {
        GetStubImporter().version = 1;
        GetStubCooker().version = 1;
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: presets by glob and the per-asset override reach the importer, the post-processors and the sidecar" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "presets",
            "\"rules\": { \"group\": \"pack\", \"OrderA\": { \"tag\": \"fromRules\" } }, "
            "\"presets\": [ { \"match\": \"props/**\", \"settings\": { \"stubScale\": 2, \"group\": \"props\" } }, { \"match\": \"**/crate*\", \"settings\": { \"stubScale\": 3 } } ], "
            "\"postProcessors\": { \"stub\": [ \"OrderB\", { \"name\": \"OrderA\", \"settings\": { \"tag\": \"x\" } } ] }" );
        WriteWorkFile( "presets/overrides/props/rock.stub.import.json", "{ \"stubScale\": 5, \"OrderA\": { \"tag\": \"fromOverride\" } }" );

        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );

        CORE::CDataNode rock;
        ASSERT_TRUE( CCanonicalJson::Load( "work/presets/intermediate/props/rock.conversion.json", rock ) );
        const CORE::CDataNode* settings = rock.FindChild( "settings" );
        ASSERT_TRUE( GUCEF_NULL != settings && 5 == settings->GetAttributeValueOrChildValueByName( "stubScale" ).AsInt32() );
        ASSERT_TRUE( GUCEF_NULL != settings && "props" == settings->GetAttributeValueOrChildValueByName( "group" ).AsString() && "default" == settings->GetAttributeValueOrChildValueByName( "stubDefault" ).AsString() );
        CORE::CString::StringVector layers;
        rock.GetValuesOfChildByName( "settingsLayers", layers );
        ASSERT_TRUE( 4 == layers.size() && "override work/presets/overrides/props/rock.stub.import.json" == layers[ 3 ] );
        const CORE::CDataNode* extras = rock.FindChild( "extras" );
        ASSERT_TRUE( GUCEF_NULL != extras && "OrderB,OrderA(fromOverride)" == extras->GetAttributeValueOrChildValueByName( "order" ).AsString() );
        const CORE::CDataNode* seen = GUCEF_NULL != extras ? extras->FindChild( "settings" ) : GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL != seen && 5 == seen->GetAttributeValueOrChildValueByName( "stubScale" ).AsInt32() );

        CORE::CDataNode crate;
        ASSERT_TRUE( CCanonicalJson::Load( "work/presets/intermediate/props/crate.conversion.json", crate ) );
        settings = crate.FindChild( "settings" );
        ASSERT_TRUE( GUCEF_NULL != settings && 3 == settings->GetAttributeValueOrChildValueByName( "stubScale" ).AsInt32() && "props" == settings->GetAttributeValueOrChildValueByName( "group" ).AsString() );
        extras = crate.FindChild( "extras" );
        ASSERT_TRUE( GUCEF_NULL != extras && "OrderB,OrderA(fromRules)" == extras->GetAttributeValueOrChildValueByName( "order" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: the post-processor stack runs in the order of the manifest per source format, \"*\" for the others" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "stacks", "\"postProcessors\": { \"fbx\": [ \"OrderA\" ], \"*\": [ \"OrderA\", \"OrderB\", { \"name\": \"OrderA\", \"settings\": { \"tag\": \"again\" } } ] }" );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( RunPack( manifest, pipeline, report ) );
        CORE::CDataNode sidecar;
        ASSERT_TRUE( CCanonicalJson::Load( "work/stacks/intermediate/props/crate.conversion.json", sidecar ) );
        const CORE::CDataNode* extras = sidecar.FindChild( "extras" );
        ASSERT_TRUE( GUCEF_NULL != extras && "OrderA,OrderB,OrderA(again)" == extras->GetAttributeValueOrChildValueByName( "order" ).AsString() );

        // a post-processor that is not registered fails the asset
        const CORE::CString broken = WriteStandardPack( "stackbroken", "\"postProcessors\": { \"*\": [ \"NoSuchProcessor\" ] }" );
        ASSERT_FALSE( RunPack( broken, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_FAILED ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_FAILED, ACTIVITYREASON_NEW ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: validation errors stop the cooking of an asset unless forced; the intermediate is still written" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "validation", "\"validators\": [ { \"name\": \"TextureSize\", \"settings\": { \"maxSize\": 2 } }, \"MissingTextures\" ]" );
        CStubCooker& cooker = GetStubCooker();
        cooker.cookedAssets.clear();
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_FALSE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_FAILED ) && cooker.cookedAssets.empty() );
        const CConversionReportEntry* entry = report.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && entry->cookBlocked );
        ASSERT_TRUE( VfsFileExists( "work/validation/intermediate/props/rock.gltf" ) && !VfsFileExists( "work/validation/intermediate/.cache/props/rock.stub.cache.json" ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_FAILED, ACTIVITYREASON_NEW ) );

        CAssetPipelineOptions options;
        options.force = true;
        ASSERT_FALSE( RunPack( manifest, pipeline, report, options ) );
        ASSERT_TRUE( 2 == cooker.cookedAssets.size() && VfsFileExists( "work/validation/out/desktop/validation/rock.mesh" ) );
        entry = report.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && !entry->cookBlocked );

        // the limit of the hardware skinning of the cooker reaches the BoneLimit validator
        const CORE::CString bones = WriteStandardPack( "bones", "\"validators\": [ \"BoneLimit\" ]" );
        WriteWorkFile( "bones/src/props/rock.stub", "{ \"nodes\": [ { \"name\": \"Rock\", \"mesh\": true, \"skinned\": true } ] }" );
        ASSERT_TRUE( RunPack( bones, pipeline, report ) );
        cooker.maxBones = 2;
        ASSERT_FALSE( RunPack( bones, pipeline, report ) );
        ASSERT_TRUE( ASSETSTATUS_FAILED == StatusOf( report, "props/rock.stub" ) && ASSETSTATUS_CONVERTED == StatusOf( report, "props/crate.stub" ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/crate.stub", ACTIVITYACTION_IMPORTED, ACTIVITYREASON_SETTINGS_CHANGED ) );
        entry = report.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && 1 == entry->GetItemCount( REPORTITEM_ERROR ) && "BoneLimit" == entry->items.back().source );
        cooker.maxBones = 0;
    }
    catch ( ... )
    {
        GetStubCooker().maxBones = 0;
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: the licence record decides; refused packs convert nothing and say why" )
    try
    {
        CAssetPipeline pipeline;
        CConversionReport report;

        // no record
        const CORE::CString missing = WriteStandardPack( "nolicence", CORE::CString::Empty, false );
        ASSERT_FALSE( RunPack( missing, pipeline, report ) );
        ASSERT_TRUE( report.IsRefused() && report.GetRefusalReason().HasSubstr( "no readable licence record", 0 ) >= 0 );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_REFUSED ) && !VfsFileExists( "work/nolicence/intermediate/props/rock.gltf" ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/rock.stub", ACTIVITYACTION_REFUSED, ACTIVITYREASON_LICENCE ) );
        ASSERT_TRUE( VfsFileExists( "work/nolicence/intermediate/report.json" ) && VfsFileExists( "work/nolicence/intermediate/activity.json" ) );

        // ueOnly, referenceOnly, restricted without review
        const CORE::CString ueOnly = WriteStandardPack( "ueonly", CORE::CString::Empty, false );
        WriteLicence( "ueonly/licences", "rec", true, false, false, false );
        ASSERT_FALSE( RunPack( ueOnly, pipeline, report ) );
        ASSERT_TRUE( report.IsRefused() && report.GetRefusalReason().HasSubstr( "ueOnly", 0 ) >= 0 );
        WriteLicence( "ueonly/licences", "rec", false, true, false, false );
        ASSERT_TRUE( ReadVfsText( "work/ueonly/licences/rec.licence.json" ).HasSubstr( "\"referenceOnly\": true", 0 ) > 0 );
        CLicenceRecord probe;
        ASSERT_TRUE( probe.Load( "work/ueonly/licences/rec.licence.json" ) && probe.referenceOnly && !probe.ueOnly );
        ASSERT_FALSE( RunPack( ueOnly, pipeline, report ) );
        ASSERT_TRUE( report.IsRefused() && report.GetRefusalReason().HasSubstr( "referenceOnly", 0 ) >= 0 );
        WriteLicence( "ueonly/licences", "rec", false, false, true, false );
        ASSERT_FALSE( RunPack( ueOnly, pipeline, report ) );
        ASSERT_TRUE( report.IsRefused() && report.GetRefusalReason().HasSubstr( "reviewed", 0 ) >= 0 );

        // reviewed: converted
        WriteLicence( "ueonly/licences", "rec", false, false, true, true );
        ASSERT_TRUE( RunPack( ueOnly, pipeline, report ) );
        ASSERT_TRUE( !report.IsRefused() && 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );

        // a record of another pack
        const CORE::CString other = WriteStandardPack( "otherlicence", CORE::CString::Empty, false );
        WriteLicence( "otherlicence/licences", "rec", false, false, false, false );
        WriteWorkFile( "otherlicence/licences/rec.licence.json", ReadVfsText( "work/otherlicence/licences/rec.licence.json" ).ReplaceSubstr( "\"id\": \"rec\"", "\"id\": \"another\"" ) );
        ASSERT_FALSE( RunPack( other, pipeline, report ) );
        ASSERT_TRUE( report.IsRefused() && report.GetRefusalReason().HasSubstr( "another", 0 ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: the intermediate and the cooked files are deterministic: the same bytes on every run" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "determinism", CORE::CString::Empty );
        CAssetPipeline pipeline;
        CConversionReport report;
        CAssetPipelineOptions options;
        options.reimport = true;
        const char* const files[] = { "intermediate/props/rock.gltf", "intermediate/props/rock.conversion.json", "intermediate/props/crate.conversion.json",
                                      "intermediate/.cache/props/rock.stub.cache.json", "intermediate/report.json", "out/desktop/references.json", "out/desktop/sizes.json" };
        const UInt32 count = sizeof( files ) / sizeof( files[ 0 ] );
        CORE::CString::StringVector first;
        ASSERT_TRUE( RunPack( manifest, pipeline, report, options ) );
        for ( UInt32 i=0; i<count; ++i )
            first.push_back( CContentHash::HashFile( CORE::CString( "work/determinism/" ) + files[ i ] ) );
        CORE::CDataNode exported;
        ASSERT_TRUE( CCanonicalJson::Load( "work/determinism/intermediate/props/rock.gltf", exported ) );
        CORE::CString::StringVector images;
        exported.GetValuesOfChildByName( "images", images );
        ASSERT_TRUE( 1 == images.size() );
        const CORE::CString bakedHash = images.empty() ? CORE::CString::Empty : CContentHash::HashFile( images[ 0 ] );

        // the baked texture is written anew too: it is removed first
        if ( !images.empty() )
            VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( images[ 0 ], true );
        options.threads = 1;
        ASSERT_TRUE( RunPack( manifest, pipeline, report, options ) );
        for ( UInt32 i=0; i<count; ++i )
        {
            const CORE::CString hash = CContentHash::HashFile( CORE::CString( "work/determinism/" ) + files[ i ] );
            ASSERT_TRUE( !hash.IsNULLOrEmpty() && hash == first[ i ] );
        }
        ASSERT_TRUE( !images.empty() && !bakedHash.IsNULLOrEmpty() && bakedHash == CContentHash::HashFile( images[ 0 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: a pack whose source is an archive is mounted, read in place and unmounted" )
    try
    {
        MakeWorkFolder( "archive" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
        ASSERT_TRUE( !writer.IsNULL() );
        if ( !writer.IsNULL() )
        {
            ASSERT_TRUE( WriteWorkFile( "archive/stage/rock.stub", MakeStubSource( "Rock", "rock.png" ) ) );
            ASSERT_TRUE( WriteTestPng( "work/archive/stage/rock.png", 2, 2, 1, 2, 3, 255 ) );
            {
                CORE::CFileAccess output( WorkDiskPath( "archive/pack.zip" ), "wb" );
                ASSERT_TRUE( writer->Open( output ) );
                VFS::CArchiveWriterEntrySettings entrySettings;
                ASSERT_TRUE( writer->AddFileFromPath( "models/rock.stub", WorkDiskPath( "archive/stage/rock.stub" ), entrySettings ) );
                ASSERT_TRUE( writer->AddFileFromPath( "models/rock.png", WorkDiskPath( "archive/stage/rock.png" ), entrySettings ) );
                ASSERT_TRUE( writer->Close() );
            }
            WriteLicence( "archive/licences", "rec", false, false, false, false );
            WriteWorkFile( "archive/pack.assetpack.json", "{ \"id\": \"archive\", \"licenceRecord\": \"rec\", \"licenceFolder\": \"licences\", \"source\": \"pack.zip\", "
                                                          "\"importer\": \"stub\", \"cookTargets\": [ \"stubcook\" ] }" );
            CAssetPipeline pipeline;
            CConversionReport report;
            ASSERT_TRUE( RunPack( "work/archive/pack.assetpack.json", pipeline, report ) );
            ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && GUCEF_NULL != report.FindEntry( "models/rock.stub" ) );
            ASSERT_TRUE( VfsFileExists( "work/archive/intermediate/models/rock.gltf" ) && VfsFileExists( "work/archive/out/desktop/archive/rock.mesh" ) );
            ASSERT_FALSE( vfs.IsMountedArchive( "assetpacks/archive" ) );
            ASSERT_FALSE( VfsFileExists( "assetpacks/archive/models/rock.stub" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: an asset the importer finds nothing to convert in is skipped with its reason, not failed; a declined import without the mark stays failed" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "skipping", CORE::CString::Empty );
        WriteWorkFile( "skipping/src/props/blueprint.stub", "{ \"skip\": \"Class Blueprint: not converted\" }" );
        WriteWorkFile( "skipping/src/props/unmarked.stub", "{ \"skip\": \"Class Blueprint: not converted\", \"unmarked\": true }" );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_FALSE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && 1 == report.GetAssetCount( ASSETSTATUS_SKIPPED ) && 1 == report.GetAssetCount( ASSETSTATUS_FAILED ) );
        ASSERT_TRUE( ASSETSTATUS_SKIPPED == StatusOf( report, "props/blueprint.stub" ) && ASSETSTATUS_FAILED == StatusOf( report, "props/unmarked.stub" ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/blueprint.stub", ACTIVITYACTION_SKIPPED, ACTIVITYREASON_NEW ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/unmarked.stub", ACTIVITYACTION_FAILED, ACTIVITYREASON_NEW ) );
        const CConversionReportEntry* skipped = report.FindEntry( "props/blueprint.stub" );
        ASSERT_TRUE( GUCEF_NULL != skipped && 0 == skipped->GetItemCount( REPORTITEM_ERROR ) && 1 == skipped->GetItemCount( REPORTITEM_SKIPPED ) );
        const CConversionReportEntry* failed = report.FindEntry( "props/unmarked.stub" );
        ASSERT_TRUE( GUCEF_NULL != failed && 1 == failed->GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_FALSE( VfsFileExists( "work/skipping/out/desktop/skipping/blueprint.mesh" ) );
        // without a current asset the mark changes nothing
        CConversionReport loose;
        loose.SetCurrentAssetSkipped();
        ASSERT_TRUE( loose.GetEntries().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: a failing import fails its asset only; an asynchronous conversion gives its job back through the future" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "partial", CORE::CString::Empty );
        WriteWorkFile( "partial/src/props/broken.stub", "{ \"fail\": true }" );
        WriteWorkFile( "partial/src/props/garbage.stub", "this is no JSON" );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_FALSE( RunPack( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && 2 == report.GetAssetCount( ASSETSTATUS_FAILED ) );
        ASSERT_TRUE( HasActivity( pipeline, "props/broken.stub", ACTIVITYACTION_FAILED, ACTIVITYREASON_NEW ) );
        ASSERT_TRUE( VfsFileExists( "work/partial/out/desktop/partial/rock.mesh" ) && !VfsFileExists( "work/partial/out/desktop/partial/broken.mesh" ) );

        // the asynchronous form on its own
        CAssetPackManifest packManifest;
        ASSERT_TRUE( packManifest.Load( manifest ) );
        CAssetPackContext context;
        ASSERT_TRUE( context.Open( packManifest ) && 8 == context.GetFiles().size() );
        ASSERT_TRUE( GetStubImporter().IndexPack( context ) && 4 == context.GetIndexSize( "name" ) );
        CORE::CString resolved;
        ASSERT_TRUE( context.TryResolve( "name", "rock", resolved ) && "props/rock.stub" == resolved && !context.TryResolve( "name", "none", resolved ) );
        CTextureBaker baker( "work/partial/intermediate/textures" );
        CAssetConversionJob job;
        job.manifest = &packManifest;
        job.context = &context;
        job.baker = &baker;
        CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "stub", job.importer );
        CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", job.exporter );
        job.assetPath = "props/crate.stub";
        job.reimport = true;
        CORE::CFutureResult future = CAssetPipeline::ConvertAssetAsync( job, "AssetConversion" );
        ASSERT_TRUE( future.HasAFuture() );
        future.Await();
        CAssetConversionJob* result = GUCEF_NULL;
        ASSERT_TRUE( CAssetPipeline::TryGetResult( future, result ) && GUCEF_NULL != result );
        if ( GUCEF_NULL != result )
        {
            ASSERT_TRUE( result->converted && !result->cached && ACTIVITYREASON_FORCED == result->activity.reason );
            ASSERT_TRUE( ASSETSTATUS_CONVERTED == StatusOf( result->report, "props/crate.stub" ) && 64 == result->cacheEntry.key.Length() );
        }

        // and synchronous, on this thread, with the cache entry of the asynchronous one: unchanged, so from the cache
        if ( GUCEF_NULL != result )
            ASSERT_TRUE( result->cacheEntry.Save( CAssetCacheEntry::GetEntryPath( packManifest.GetIntermediatePath(), job.assetPath ) ) );
        CAssetConversionJob cachedJob = job;
        cachedJob.reimport = false;
        ASSERT_TRUE( CAssetPipeline::ConvertAsset( cachedJob ) && cachedJob.cached && ACTIVITYACTION_CACHED == cachedJob.activity.action );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pipeline: an observer gets the progress of the assets, and a request to stop ends the run before the cooking, with the cache entries of what finished only, with a report that says so" )
    try
    {
        CStubCooker& cooker = GetStubCooker();

        // not asked to stop: every asset is reported once, with the total
        const CORE::CString manifest = WriteStandardPack( "observed", CORE::CString::Empty );
        cooker.cookedAssets.clear();
        CCountingObserver watching( -1 );
        CAssetPipelineOptions options;
        options.observer = &watching;
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( RunPack( manifest, pipeline, report, options ) );
        ASSERT_TRUE( 2 == watching.finished && 2 == watching.lastTotal && !report.IsCancelled() && 2 == cooker.cookedAssets.size() );

        // asked to stop after the first asset
        const CORE::CString stoppedManifest = WriteStandardPack( "stopped", CORE::CString::Empty );
        cooker.cookedAssets.clear();
        CCountingObserver stopping( 1 );
        options.observer = &stopping;
        options.threads = 1;                                      // one asset at a time, in a pool of its own that has no other threads,
        options.threadPoolName = "AssetConverterStopTest";        // so the stop after the first is seen before the second starts
        CAssetPipeline stoppedPipeline;
        CConversionReport stopped;
        ASSERT_FALSE( RunPack( stoppedManifest, stoppedPipeline, stopped, options ) );
        ASSERT_TRUE( stopped.IsCancelled() && stopped.HasFailures() && 1 == stopping.finished && cooker.cookedAssets.empty() );
        // the asset that finished has its cache entry (cook pending, the next run resumes after it), the other none
        const UInt32 entries = ( VfsFileExists( "work/stopped/intermediate/.cache/props/rock.stub.cache.json" ) ? 1 : 0 ) +
                               ( VfsFileExists( "work/stopped/intermediate/.cache/props/crate.stub.cache.json" ) ? 1 : 0 );
        ASSERT_TRUE( stopping.finished == entries && 2 - entries == stopped.GetAssetCount( ASSETSTATUS_SKIPPED ) );
        ASSERT_TRUE( VfsFileExists( "work/stopped/intermediate/report.json" ) && stopped.ToSummaryText().HasSubstr( "CANCELLED" ) >= 0 );
        CConversionReport written;
        ASSERT_TRUE( written.LoadJson( "work/stopped/intermediate/report.json" ) && written.IsCancelled() );

        // asked to stop before the first asset: nothing is converted
        const CORE::CString nothingManifest = WriteStandardPack( "stoppedatonce", CORE::CString::Empty );
        CCountingObserver atOnce( 0 );
        options.observer = &atOnce;
        CAssetPipeline atOncePipeline;
        CConversionReport atOnceReport;
        ASSERT_FALSE( RunPack( nothingManifest, atOncePipeline, atOnceReport, options ) );
        ASSERT_TRUE( atOnceReport.IsCancelled() && 0 == atOnce.finished && 0 == atOnceReport.GetAssetCount( ASSETSTATUS_CONVERTED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
