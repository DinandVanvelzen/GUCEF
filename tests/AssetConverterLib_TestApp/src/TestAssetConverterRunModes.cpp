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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterRunModes.h"

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
LoadManifest( const CORE::CString& manifestPath, CAssetPackManifest& manifest )
{
    return manifest.Load( manifestPath );
}

bool
Run( const CORE::CString& manifestPath, CAssetPipeline& pipeline, CConversionReport& report, const CAssetPipelineOptions& options = CAssetPipelineOptions() )
{
    CAssetPackManifest manifest;
    if ( !LoadManifest( manifestPath, manifest ) )
        return false;
    return pipeline.RunPack( manifest, options, report );
}

bool
Plan( const CORE::CString& manifestPath, CAssetPackPlan& plan, const CAssetPipelineOptions& options = CAssetPipelineOptions() )
{
    CAssetPackManifest manifest;
    if ( !LoadManifest( manifestPath, manifest ) )
        return false;
    CAssetPipeline pipeline;
    return pipeline.PlanPack( manifest, options, plan );
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

/** Whether the run did for every asset of the plan what the plan said, with the same reason */
bool
RunMatchesPlan( const CAssetPackPlan& plan, const CAssetPipeline& pipeline )
{
    if ( plan.entries.size() != pipeline.GetLastRun().entries.size() )
        return false;
    for ( size_t i=0; i<plan.entries.size(); ++i )
    {
        const CImportActivityEntry* activity = FindActivity( pipeline, plan.entries[ i ].assetPath );
        if ( GUCEF_NULL == activity )
            return false;
        if ( activity->action != plan.entries[ i ].GetExpectedActivityAction() || activity->reason != plan.entries[ i ].reason )
            return false;
    }
    return true;
}

bool
VfsFileExists( const CORE::CString& vfsPath )
{
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath );
}

/** Every file below the folder of the work folder, as paths on the disk */
CORE::CString::StringVector
ListDiskFiles( const CORE::CString& relativeFolder )
{
    CORE::CString::StringVector files;
    VFS::CVFS::TStringVector found;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( found, "work/" + relativeFolder, true, true, CORE::CString::Empty, 100000 );
    for ( size_t i=0; i<found.size(); ++i )
        files.push_back( found[ i ] );
    return files;
}

bool
LoadCacheEntry( const CORE::CString& pack, const CORE::CString& assetPath, CAssetCacheEntry& entry )
{
    const CORE::CString path = CAssetCacheEntry::GetEntryPath( "work/" + pack + "/intermediate", assetPath );
    return VfsFileExists( path ) && entry.Load( path );
}

/** The standard pack with four more assets: props/extra1.stub .. extra4.stub */
CORE::CString
WriteLargerPack( const CORE::CString& name )
{
    const CORE::CString manifest = WriteStandardPack( name, CORE::CString::Empty );
    for ( UInt32 i=1; i<=4; ++i )
        WriteWorkFile( name + "/src/props/extra" + CORE::ToString( i ) + ".stub", MakeStubSource( "Extra" + CORE::ToString( i ), "crate.png" ) );
    return manifest;
}

/** Writes a lock file of the intermediate of the pack as another process would */
bool
WriteForeignLock( const CORE::CString& pack, UInt32 processId, const CORE::CString& host )
{
    return WriteWorkFile( pack + "/intermediate/" + CIntermediateLock::LockFileName,
                          "pid=" + CORE::ToString( processId ) + "\nhost=" + host + "\nstarted=2026-01-01T00:00:00.000Z\ntoken=foreign-token\n" );
}

/** An observer of the tests: stops after some assets, records the phases, the indexed count and the reports */
class CRunModeObserver : public CIAssetPipelineObserver
{
    public:

    UInt32 finished;
    Int32 stopAfter;
    UInt32 indexed;
    GUCEF::vector< TPipelinePhase > phases;
    UInt32 reports;

    explicit CRunModeObserver( Int32 stopWhenDone = -1 ) : CIAssetPipelineObserver(), finished( 0 ), stopAfter( stopWhenDone ), indexed( 0 ), phases(), reports( 0 ) {}

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return stopAfter >= 0 && finished >= (UInt32) stopAfter;
    }

    virtual void OnAssetFinished( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++finished;
    }

    virtual void OnPackIndexed( UInt32 assetCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        indexed = assetCount;
    }

    virtual void OnPhase( TPipelinePhase phase ) GUCEF_VIRTUAL_OVERRIDE
    {
        phases.push_back( phase );
    }

    virtual void OnAssetReport( const CConversionReportEntry& entry, UInt32 done, UInt32 total ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !entry.assetPath.IsNULLOrEmpty() )
            ++reports;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterRunModeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.RunModes" );

    GUCEF_TESTFW_TESTCASE( "Plan: a dry run writes nothing and says per asset what the run then does, with the same reasons, the importer and the effective settings" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "plan", CORE::CString::Empty );
        const CORE::CString::StringVector before = ListDiskFiles( "plan" );
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan ) );
        ASSERT_TRUE( before == ListDiskFiles( "plan" ) );
        ASSERT_TRUE( plan.IsRunnable() && plan.conversionAllowed && "rec" == plan.licenceRecordId && plan.lockHolder.IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == plan.cookTargets.size() && "stubcook" == plan.cookTargets[ 0 ] );
        ASSERT_TRUE( 2 == plan.entries.size() && 2 == plan.GetCount( PLANACTION_IMPORT ) );
        const CAssetPlanEntry* rock = plan.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != rock && "stub" == rock->importer && ACTIVITYREASON_NEW == rock->reason );
        ASSERT_TRUE( GUCEF_NULL != rock && "default" == rock->settings.GetAttributeValueOrChildValueByName( "stubDefault" ).AsString() && !rock->layers.empty() );

        // the run does what the plan said
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( RunMatchesPlan( plan, pipeline ) );

        // unchanged: both cached; then a changed source: only that asset, with the reason
        ASSERT_TRUE( Plan( manifest, plan ) );
        ASSERT_TRUE( 2 == plan.GetCount( PLANACTION_CACHED ) );
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( RunMatchesPlan( plan, pipeline ) );
        WriteWorkFile( "plan/src/props/rock.stub", "{ \"nodes\": [ { \"name\": \"Rock\", \"mesh\": true, \"size\": 3, \"material\": 0 } ], "
                       "\"materials\": [ { \"name\": \"RockMaterial\", \"baseColor\": \"rock.png\" } ], \"dependencies\": [ \"rock.notes.txt\" ] }" );
        WriteWorkFile( "plan/overrides/props/crate.stub.import.json", "{ \"stubScale\": 4 }" );
        const CORE::CString::StringVector beforeChangedPlan = ListDiskFiles( "plan" );
        ASSERT_TRUE( Plan( manifest, plan ) );
        ASSERT_TRUE( beforeChangedPlan == ListDiskFiles( "plan" ) );
        rock = plan.FindEntry( "props/rock.stub" );
        const CAssetPlanEntry* crate = plan.FindEntry( "props/crate.stub" );
        ASSERT_TRUE( GUCEF_NULL != rock && PLANACTION_IMPORT == rock->action && ACTIVITYREASON_SOURCE_CHANGED == rock->reason );
        ASSERT_TRUE( GUCEF_NULL != crate && PLANACTION_IMPORT == crate->action && ACTIVITYREASON_SETTINGS_CHANGED == crate->reason );
        ASSERT_TRUE( GUCEF_NULL != crate && 4 == crate->settings.GetAttributeValueOrChildValueByName( "stubScale" ).AsInt32() );
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( RunMatchesPlan( plan, pipeline ) );

        // a forced plan, and the plan as JSON
        CAssetPipelineOptions forced;
        forced.reimport = true;
        ASSERT_TRUE( Plan( manifest, plan, forced ) );
        ASSERT_TRUE( 2 == plan.GetCount( PLANACTION_IMPORT ) && ACTIVITYREASON_FORCED == plan.entries[ 0 ].reason );
        ASSERT_TRUE( plan.SaveJson( "work/plan/plans/forced.json" ) );
        CAssetPackPlan loaded;
        ASSERT_TRUE( loaded.LoadJson( "work/plan/plans/forced.json" ) );
        ASSERT_TRUE( 2 == loaded.GetCount( PLANACTION_IMPORT ) && "plan" == loaded.packId && GUCEF_NULL != loaded.FindEntry( "props/crate.stub" ) );
        ASSERT_TRUE( plan.ToSummaryText().HasSubstr( "props/rock.stub" ) >= 0 );

        // a refused pack: every asset refused, with the reason of the record
        const CORE::CString refused = WriteStandardPack( "planrefused", CORE::CString::Empty, false );
        ASSERT_TRUE( Plan( refused, plan ) );
        ASSERT_TRUE( !plan.conversionAllowed && !plan.IsRunnable() && 2 == plan.GetCount( PLANACTION_REFUSED ) && !plan.licenceReason.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resume: a cancelled run keeps the cache entries of what it finished (cook pending) and lists the rest as skipped; the next run converts only the rest and cooks all" )
    try
    {
        const CORE::CString manifest = WriteLargerPack( "resume" );
        CStubCooker& cooker = GetStubCooker();
        cooker.cookedAssets.clear();
        CRunModeObserver stopping( 1 );
        CAssetPipelineOptions options;
        options.observer = &stopping;
        options.threads = 1;
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_FALSE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( report.IsCancelled() && cooker.cookedAssets.empty() );

        const char* assets[] = { "props/rock.stub", "props/crate.stub", "props/extra1.stub", "props/extra2.stub", "props/extra3.stub", "props/extra4.stub" };
        UInt32 withEntry = 0;
        bool allPending = true;
        for ( UInt32 i=0; i<6; ++i )
        {
            CAssetCacheEntry entry;
            if ( LoadCacheEntry( "resume", assets[ i ], entry ) )
            {
                ++withEntry;
                allPending = allPending && entry.cookPending;
            }
        }
        ASSERT_TRUE( withEntry >= 1 && withEntry < 6 && withEntry == stopping.finished && allPending );
        ASSERT_TRUE( 6 - withEntry == report.GetAssetCount( ASSETSTATUS_SKIPPED ) );
        UInt32 skippedActivities = 0;
        for ( size_t i=0; i<pipeline.GetLastRun().entries.size(); ++i )
        {
            const CImportActivityEntry& activity = pipeline.GetLastRun().entries[ i ];
            if ( ACTIVITYACTION_SKIPPED == activity.action && ACTIVITYREASON_CANCELLED == activity.reason && !activity.assetPath.IsNULLOrEmpty() )
                ++skippedActivities;
        }
        ASSERT_TRUE( 6 - withEntry == skippedActivities );
        CConversionReport written;
        ASSERT_TRUE( written.LoadJson( "work/resume/intermediate/report.json" ) && written.IsCancelled() );

        // the plan of the resume: cook what finished, import the rest
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan ) );
        ASSERT_TRUE( withEntry == plan.GetCount( PLANACTION_COOK ) && 6 - withEntry == plan.GetCount( PLANACTION_IMPORT ) );

        // the resume: only the rest is imported, everything is cooked
        const UInt32 importsBefore = GetStubImporter().importCount;
        CConversionReport resumed;
        CAssetPipeline resumePipeline;
        ASSERT_TRUE( Run( manifest, resumePipeline, resumed ) );
        ASSERT_TRUE( importsBefore + ( 6 - withEntry ) == GetStubImporter().importCount );
        ASSERT_TRUE( 6 == cooker.cookedAssets.size() && !resumed.IsCancelled() );
        ASSERT_TRUE( RunMatchesPlan( plan, resumePipeline ) );
        bool nonePending = true;
        for ( UInt32 i=0; i<6; ++i )
        {
            CAssetCacheEntry entry;
            nonePending = nonePending && LoadCacheEntry( "resume", assets[ i ], entry ) && !entry.cookPending;
        }
        ASSERT_TRUE( nonePending && VfsFileExists( "work/resume/out/desktop/resume/extra4.mesh" ) );

        // and then everything comes from the cache
        cooker.cookedAssets.clear();
        const UInt32 importsAfter = GetStubImporter().importCount;
        CConversionReport third;
        ASSERT_TRUE( Run( manifest, resumePipeline, third ) );
        ASSERT_TRUE( 6 == third.GetAssetCount( ASSETSTATUS_CACHED ) && importsAfter == GetStubImporter().importCount && cooker.cookedAssets.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lock: a run while the intermediate is locked writes nothing and says who holds it; a stale lock is taken over; a lock of another host is not" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "locked", CORE::CString::Empty );
        const UInt32 importsBefore = GetStubImporter().importCount;
        CAssetPipeline pipeline;
        CConversionReport report;
        {
            CIntermediateLock lock( "work/locked/intermediate" );
            ASSERT_TRUE( lock.IsHeld() );
            ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "locked/intermediate/.lock" ) ) );
            CIntermediateLock second( "work/locked/intermediate" );
            ASSERT_FALSE( second.IsHeld() );
            ASSERT_TRUE( second.GetHolder().HasSubstr( "process " + CORE::ToString( MT::GetProcessID() ) ) >= 0 );
            CORE::CString holder;
            ASSERT_TRUE( CIntermediateLock::GetCurrentHolder( "work/locked/intermediate", holder ) && !holder.IsNULLOrEmpty() );

            ASSERT_FALSE( Run( manifest, pipeline, report ) );
            ASSERT_TRUE( report.IsLocked() && report.HasFailures() && report.GetLockHolder().HasSubstr( "process" ) >= 0 );
            ASSERT_TRUE( importsBefore == GetStubImporter().importCount && !VfsFileExists( "work/locked/intermediate/report.json" ) );
            CAssetPackPlan plan;
            ASSERT_TRUE( Plan( manifest, plan ) );
            ASSERT_TRUE( !plan.lockHolder.IsNULLOrEmpty() );

            // validate-only writes nothing and needs no lock
            CAssetPipelineOptions validate;
            validate.mode = PIPELINEMODE_VALIDATE_ONLY;
            ASSERT_TRUE( Run( manifest, pipeline, report, validate ) );
        }
        ASSERT_FALSE( CORE::FileExists( WorkDiskPath( "locked/intermediate/.lock" ) ) );
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_FALSE( CORE::FileExists( WorkDiskPath( "locked/intermediate/.lock" ) ) );

        // a lock of a process that is gone on this host: taken over
        UInt32 gone = 0x3FFFFF00;
        bool alive = true;
        while ( gone < 0x3FFFFFFF && CORE::CheckOnProcessAliveStatus( (CORE::TProcessId) gone, alive ) && alive )
            ++gone;
        ASSERT_TRUE( WriteForeignLock( "locked", gone, CORE::GetHostname() ) );
        CAssetPipelineOptions forced;
        forced.reimport = true;
        ASSERT_TRUE( Run( manifest, pipeline, report, forced ) );
        ASSERT_TRUE( !report.IsLocked() && 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        ASSERT_FALSE( CORE::FileExists( WorkDiskPath( "locked/intermediate/.lock" ) ) );

        // a lock of another host cannot be judged: busy
        ASSERT_TRUE( WriteForeignLock( "locked", 1, "another-host-of-the-tests" ) );
        ASSERT_FALSE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( report.IsLocked() && report.GetLockHolder().HasSubstr( "another-host-of-the-tests" ) >= 0 );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "locked/intermediate/.lock" ) ) );
        CORE::DeleteFile( WorkDiskPath( "locked/intermediate/.lock" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Filter: asset globs restrict a run to their assets; the others keep their cache entries and their place in the references and sizes" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "filtered", CORE::CString::Empty );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        CAssetCacheEntry crateBefore;
        ASSERT_TRUE( LoadCacheEntry( "filtered", "props/crate.stub", crateBefore ) );

        WriteWorkFile( "filtered/src/props/rock.stub", "{ \"nodes\": [ { \"name\": \"Rock\", \"mesh\": true, \"size\": 4, \"material\": 0 } ], "
                       "\"materials\": [ { \"name\": \"RockMaterial\", \"baseColor\": \"rock.png\" } ] }" );
        WriteWorkFile( "filtered/src/props/crate.stub", MakeStubSource( "CrateChanged", "crate.png" ) );
        CAssetPipelineOptions options;
        options.assetGlobs.push_back( "props/rock*" );
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan, options ) );
        ASSERT_TRUE( 1 == plan.entries.size() && 1 == plan.filteredOut && GUCEF_NULL != plan.FindEntry( "props/rock.stub" ) );

        GetStubCooker().cookedAssets.clear();
        ASSERT_TRUE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( GUCEF_NULL != report.FindEntry( "props/rock.stub" ) && GUCEF_NULL == report.FindEntry( "props/crate.stub" ) );
        ASSERT_TRUE( 1 == report.GetAssetFilter().size() && 1 == GetStubCooker().cookedAssets.size() );
        ASSERT_TRUE( RunMatchesPlan( plan, pipeline ) );
        CAssetCacheEntry crateAfter;
        ASSERT_TRUE( LoadCacheEntry( "filtered", "props/crate.stub", crateAfter ) && crateAfter.key == crateBefore.key );
        const CORE::CString references = ReadVfsText( "work/filtered/out/desktop/references.json" );
        ASSERT_TRUE( references.HasSubstr( "crate" ) >= 0 && references.HasSubstr( "rock" ) >= 0 );
        CConversionReport written;
        ASSERT_TRUE( written.LoadJson( "work/filtered/intermediate/report.json" ) && 1 == written.GetAssetFilter().size() );

        // an exact path is a filter too; the crate then sees its change
        options.assetGlobs.clear();
        options.assetGlobs.push_back( "props/crate.stub" );
        ASSERT_TRUE( Run( manifest, pipeline, report, options ) );
        const CImportActivityEntry* crate = FindActivity( pipeline, "props/crate.stub" );
        ASSERT_TRUE( GUCEF_NULL != crate && ACTIVITYACTION_IMPORTED == crate->action && ACTIVITYREASON_SOURCE_CHANGED == crate->reason );
        ASSERT_TRUE( GUCEF_NULL == FindActivity( pipeline, "props/rock.stub" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Validate-only: the assets are imported and validated, nothing is written but the report that is asked for" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "validate", "\"validators\": [ { \"name\": \"TextureSize\", \"settings\": { \"maxSize\": 2 } }, \"MissingTextures\" ]" );
        const CORE::CString::StringVector before = ListDiskFiles( "validate" );
        const UInt32 importsBefore = GetStubImporter().importCount;
        GetStubCooker().cookedAssets.clear();
        CAssetPipelineOptions options;
        options.mode = PIPELINEMODE_VALIDATE_ONLY;
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_FALSE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( PIPELINEMODE_VALIDATE_ONLY == report.GetMode() && 2 == report.GetAssetCount( ASSETSTATUS_FAILED ) );
        const CConversionReportEntry* rock = report.FindEntry( "props/rock.stub" );
        ASSERT_TRUE( GUCEF_NULL != rock && rock->files.empty() && rock->GetItemCount( REPORTITEM_ERROR ) >= 1 );
        ASSERT_TRUE( importsBefore + 2 == GetStubImporter().importCount && GetStubCooker().cookedAssets.empty() );
        ASSERT_TRUE( before == ListDiskFiles( "validate" ) );
        ASSERT_FALSE( CORE::DirExists( WorkDiskPath( "validate/intermediate" ) ) );

        // the plan of validate-only
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan, options ) );
        ASSERT_TRUE( 2 == plan.GetCount( PLANACTION_VALIDATE ) );

        // with a report path: that report only
        options.reportPath = "work/validate/reports/validation.json";
        ASSERT_FALSE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( VfsFileExists( "work/validate/reports/validation.json" ) && VfsFileExists( "work/validate/reports/validation.txt" ) );
        ASSERT_FALSE( CORE::DirExists( WorkDiskPath( "validate/intermediate" ) ) );
        CConversionReport written;
        ASSERT_TRUE( written.LoadJson( "work/validate/reports/validation.json" ) && PIPELINEMODE_VALIDATE_ONLY == written.GetMode() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cook-only: refused without an intermediate; after a conversion it cooks the intermediate again without importing" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "cookonly", CORE::CString::Empty );
        CStubCooker& cooker = GetStubCooker();
        cooker.cookedAssets.clear();
        CAssetPipelineOptions options;
        options.mode = PIPELINEMODE_COOK_ONLY;
        CAssetPipeline pipeline;
        CConversionReport report;
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan, options ) );
        ASSERT_TRUE( 2 == plan.GetCount( PLANACTION_NO_INTERMEDIATE ) );
        ASSERT_FALSE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( cooker.cookedAssets.empty() && report.HasFailures() && 2 == report.GetAssetCount( ASSETSTATUS_SKIPPED ) );

        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == cooker.cookedAssets.size() );
        CORE::DeleteDir( WorkDiskPath( "cookonly/out" ), true );
        ASSERT_FALSE( VfsFileExists( "work/cookonly/out/desktop/cookonly/rock.mesh" ) );

        cooker.cookedAssets.clear();
        const UInt32 importsBefore = GetStubImporter().importCount;
        ASSERT_TRUE( Plan( manifest, plan, options ) );
        ASSERT_TRUE( 2 == plan.GetCount( PLANACTION_COOK ) );
        ASSERT_TRUE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( importsBefore == GetStubImporter().importCount && 2 == cooker.cookedAssets.size() );
        ASSERT_TRUE( PIPELINEMODE_COOK_ONLY == report.GetMode() && 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
        ASSERT_TRUE( VfsFileExists( "work/cookonly/out/desktop/cookonly/rock.mesh" ) && VfsFileExists( "work/cookonly/out/desktop/references.json" ) );

        // the cache entries know the cooked files again: a full run keeps everything
        cooker.cookedAssets.clear();
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) && importsBefore == GetStubImporter().importCount && cooker.cookedAssets.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Licence restrictions: the record keeps its restriction flags with the names of the store; they do not change the conversion; the plan shows them" )
    try
    {
        CLicenceRecord record;
        record.id = "restricted";
        record.restrictions.insert( CLicenceRecord::NoMachineLearning );
        record.restrictions.insert( CLicenceRecord::AttributionRequired );
        CORE::CDataNode node( "licence", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( record.Serialize( node ) );
        CLicenceRecord loaded;
        ASSERT_TRUE( loaded.Deserialize( node ) );
        ASSERT_TRUE( loaded.HasRestriction( "noMachineLearning" ) && loaded.HasRestriction( "attributionRequired" ) && !loaded.HasRestriction( "noCommercialUse" ) );
        CORE::CString reason;
        ASSERT_TRUE( loaded.AllowsConversion( reason ) );

        // a record without flags is written as before: no "restrictions" member
        CLicenceRecord plain;
        plain.id = "plain";
        CORE::CDataNode plainNode( "licence", GUCEF_DATATYPE_OBJECT );
        plain.Serialize( plainNode );
        ASSERT_TRUE( GUCEF_NULL == plainNode.FindChild( "restrictions" ) );

        // through the JSON of a record file, in a pack
        const CORE::CString manifest = WriteStandardPack( "restrictions", CORE::CString::Empty );
        WriteWorkFile( "restrictions/licences/rec.licence.json", "{ \"id\": \"rec\", \"store\": \"Test store\", \"licence\": \"Test licence\", "
                       "\"restrictions\": [ \"noMachineLearning\", \"editorialOnly\" ] }" );
        CAssetPackPlan plan;
        ASSERT_TRUE( Plan( manifest, plan ) );
        ASSERT_TRUE( plan.conversionAllowed && 2 == plan.restrictions.size() && plan.restrictions.end() != plan.restrictions.find( "noMachineLearning" ) );
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( Run( manifest, pipeline, report ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Observer: the number of assets once indexed, the phases in order and the report entry of every asset" )
    try
    {
        const CORE::CString manifest = WriteStandardPack( "phases", CORE::CString::Empty );
        CRunModeObserver observer;
        CAssetPipelineOptions options;
        options.observer = &observer;
        CAssetPipeline pipeline;
        CConversionReport report;
        ASSERT_TRUE( Run( manifest, pipeline, report, options ) );
        ASSERT_TRUE( 2 == observer.indexed && 2 == observer.finished && 2 == observer.reports );
        ASSERT_TRUE( 5 == observer.phases.size() );
        if ( 5 == observer.phases.size() )
        {
            ASSERT_TRUE( PIPELINEPHASE_LICENCE == observer.phases[ 0 ] && PIPELINEPHASE_INDEX == observer.phases[ 1 ] && PIPELINEPHASE_CONVERT == observer.phases[ 2 ] );
            ASSERT_TRUE( PIPELINEPHASE_COOK == observer.phases[ 3 ] && PIPELINEPHASE_REPORT == observer.phases[ 4 ] );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
