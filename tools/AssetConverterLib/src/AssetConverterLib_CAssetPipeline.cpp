/*
 *  AssetConverterLib: converts and cooks a pack: licence check, import, normalisation, post-processing, validation, export, cooking
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#include "AssetConverterLib_CAssetConversionTaskConsumer.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H ? */

#ifndef ASSETCONVERTERLIB_CASSETNORMALISER_H
#include "AssetConverterLib_CAssetNormaliser.h"
#define ASSETCONVERTERLIB_CASSETNORMALISER_H
#endif /* ASSETCONVERTERLIB_CASSETNORMALISER_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#include "AssetConverterLib_CCookSettings.h"
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H
#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#include "AssetConverterLib_CImportSettingsBuilder.h"
#define ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H
#endif /* ASSETCONVERTERLIB_CIMPORTSETTINGSBUILDER_H ? */

#ifndef ASSETCONVERTERLIB_CLICENCERECORD_H
#include "AssetConverterLib_CLicenceRecord.h"
#define ASSETCONVERTERLIB_CLICENCERECORD_H
#endif /* ASSETCONVERTERLIB_CLICENCERECORD_H ? */

#ifndef ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#include "AssetConverterLib_CReferenceGraph.h"
#define ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#endif /* ASSETCONVERTERLIB_CREFERENCEGRAPH_H ? */

#ifndef ASSETCONVERTERLIB_CSIZEREPORT_H
#include "AssetConverterLib_CSizeReport.h"
#define ASSETCONVERTERLIB_CSIZEREPORT_H
#endif /* ASSETCONVERTERLIB_CSIZEREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CTEXTUREBAKER_H
#include "AssetConverterLib_CTextureBaker.h"
#define ASSETCONVERTERLIB_CTEXTUREBAKER_H
#endif /* ASSETCONVERTERLIB_CTEXTUREBAKER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#include "AssetConverterLib_CIntermediateLock.h"
#define ASSETCONVERTERLIB_CINTERMEDIATELOCK_H
#endif /* ASSETCONVERTERLIB_CINTERMEDIATELOCK_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#include "AssetConverterLib_CPackIndexUpdater.h"
#define ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#include "AssetConverterLib_CPackIndexLocator.h"
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

#include "AssetConverterLib_CAssetPipeline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const CORE::CString PipelineSource = "Pipeline";
const CORE::CString ConverterPluginKey = "converter:AssetConverterLib";

CAssetPluginRegistry&
Plugins( void )
{
    return CAssetConverterGlobal::Instance()->GetPluginRegistry();
}

/** Asks the tasks of the assets from index 'first' on that are not done to stop; the ones that have not started then do nothing */
void
RequestCancellationOfRest( GUCEF::vector< CORE::CFutureResult >& futures, size_t first )
{
    for ( size_t f=first; f<futures.size(); ++f )
        if ( futures[ f ].HasAFuture() && !futures[ f ].IsReady() )
            futures[ f ].RequestCancellation();
}

/** The hashes of the files, by VFS path; a file that cannot be read has an empty hash */
void
HashFiles( const CORE::CString::StringVector& paths, CAssetCacheEntry::THashMap& hashes )
{
    hashes.clear();
    for ( size_t i=0; i<paths.size(); ++i )
        hashes[ paths[ i ] ] = CContentHash::HashFile( paths[ i ] );
}

UInt32
ElapsedMs( UInt64 startTicks )
{
    return (UInt32) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
}

void
Finish( CAssetConversionJob& job, TActivityAction action, TActivityReason reason, const CORE::CString& detail, UInt64 startTicks )
{
    job.activity.assetPath = job.assetPath;
    job.activity.action = action;
    job.activity.reason = reason;
    job.activity.detail = detail;
    job.activity.durationInMs = ElapsedMs( startTicks );
    CConversionReportEntry* entry = job.report.GetCurrentEntry();
    if ( GUCEF_NULL != entry )
        entry->durationInMs = job.activity.durationInMs;
}


/*-------------------------------------------------------------------------*/

/**
 *  The asset was cancelled while it was converted: nothing of it is stored (no cache entry is made for it, as it did not convert), the
 *  report lists it as skipped with the reason "cancelled", and the run is marked as cancelled
 */
bool
FinishCancelled( CAssetConversionJob& job, CConversionReportEntry& entry, UInt64 startTicks )
{
    entry.skipped = true;
    job.report.AddSkipped( PipelineSource, job.assetPath, "the conversion was cancelled" );
    job.report.SetCancelled();
    Finish( job, ACTIVITYACTION_SKIPPED, ACTIVITYREASON_CANCELLED, CORE::CString::Empty, startTicks );
    return false;
}

/*-------------------------------------------------------------------------*/

/** What a run needs of the plugins and the manifest before it looks at the assets */
struct SRunSetup
{
    GUCEF::vector< TAssetImporterPtr > importers;
    TAssetExporterPtr exporter;
    GUCEF::vector< TAssetCookerPtr > cookers;
    CAssetCacheEntry::TPluginVersionMap cookerVersions;
    UInt32 maxBones;
    CORE::CString::StringVector targets;
    CAssetPackManifest::TProfileVector profiles;
    CORE::CString processingHash;
    CORE::CString problem;                  /**< what keeps the run from starting */
    CORE::CString problemSubject;

    SRunSetup( void ) : importers(), exporter(), cookers(), cookerVersions(), maxBones( 0 ), targets(), profiles(), processingHash(), problem(), problemSubject() {}
};

/** An asset of the run: its path in the pack and its importer; a clash names the asset whose intermediate it would overwrite */
struct SSelectedAsset
{
    CORE::CString assetPath;
    TAssetImporterPtr importer;
    CORE::CString clashWith;
};
typedef GUCEF::vector< SSelectedAsset > TSelectedAssetVector;

/** The importers the manifest names: the one by name, or all of them for "auto" */
void
ResolveImporters( const CAssetPackManifest& manifest, SRunSetup& setup )
{
    setup.importers.clear();
    if ( CAssetPackManifest::AutoImporter == manifest.importer.Lowercase() )
    {
        Plugins().GetImporters( setup.importers );
    }
    else
    {
        TAssetImporterPtr importer;
        if ( Plugins().TryGetImporter( manifest.importer, importer ) )
            setup.importers.push_back( importer );
    }
}

/** The exporter, the cookers and the processing settings of the run; false with the problem when a plugin is missing */
bool
ResolvePlugins( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, SRunSetup& setup )
{
    CAssetPluginRegistry& plugins = Plugins();
    if ( setup.importers.empty() )
    {
        setup.problemSubject = manifest.importer;
        if ( CAssetPackManifest::AutoImporter == manifest.importer.Lowercase() )
            setup.problem = "no importer is registered (are the importer plugins loaded?)";
        else
            setup.problem = "the importer is not registered (is its plugin loaded?)";
        return false;
    }
    if ( !plugins.TryGetExporter( manifest.exporter, setup.exporter ) && PIPELINEMODE_FULL == options.mode )
    {
        setup.problemSubject = manifest.exporter;
        setup.problem = "the exporter of the intermediate is not registered (is its plugin loaded?)";
        return false;
    }
    if ( !options.noCook )
        setup.targets = options.cookTargets.empty() ? manifest.cookTargets : options.cookTargets;
    for ( size_t i=0; i<setup.targets.size(); ++i )
    {
        TAssetCookerPtr cooker;
        if ( !plugins.TryGetCooker( setup.targets[ i ], cooker ) )
        {
            setup.problemSubject = setup.targets[ i ];
            setup.problem = "there is no cooker for the target (is its plugin loaded?)";
            return false;
        }
        // a target that was asked for by an alias is the cooker's own name from here on
        setup.targets[ i ] = cooker->GetTargetName();
        setup.cookers.push_back( cooker );
        setup.cookerVersions[ "cooker:" + cooker->GetTargetName() ] = cooker->GetVersion();
        const UInt32 limit = cooker->GetMaxBonesPerSubMesh();
        if ( limit > 0 && ( 0 == setup.maxBones || limit < setup.maxBones ) )
            setup.maxBones = limit;
    }
    setup.profiles = manifest.cookProfiles;
    if ( setup.profiles.empty() )
        setup.profiles.push_back( CCookProfile() );

    CORE::CDataNode processing;
    manifest.GetProcessingSettings( processing );
    processing.DelChild( "cookTargets" );
    processing.SetValuesOfChildByName( "cookTargets", setup.targets );
    processing.SetAttribute( "maxBones", setup.maxBones );
    setup.processingHash = CCanonicalJson::Hash( processing );
    return true;
}

/** Why the licence record refuses the pack; empty when it allows the conversion. 'record' is the record when one was read */
CORE::CString
CheckLicence( const CAssetPackManifest& manifest, CLicenceRecord& record, bool& recordRead )
{
    recordRead = false;
    const CORE::CString recordPath = manifest.GetLicenceRecordPath();
    if ( recordPath.IsNULLOrEmpty() )
        return "the manifest names no licence record";
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( recordPath ) || !record.Load( recordPath ) )
        return "there is no readable licence record at " + recordPath;
    recordRead = true;
    if ( record.id != manifest.licenceRecord )
        return "the licence record at " + recordPath + " has the identifier " + record.id + ", not " + manifest.licenceRecord;
    CORE::CString refusal;
    record.AllowsConversion( refusal );
    return refusal;
}

/** The assets of the run: the files an importer can import, selected by the manifest and the asset filter of the options */
void
SelectAssets( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CAssetPackContext& context,
              const GUCEF::vector< TAssetImporterPtr >& importers, TSelectedAssetVector& selected, CORE::CString::StringVector& filteredOut )
{
    selected.clear();
    filteredOut.clear();
    GUCEF::map< CORE::CString, CORE::CString > intermediateNames;
    const CORE::CString::StringVector& files = context.GetFiles();
    for ( size_t f=0; f<files.size(); ++f )
    {
        if ( !manifest.IsAssetSelected( files[ f ] ) )
            continue;
        for ( size_t i=0; i<importers.size(); ++i )
        {
            if ( !importers[ i ]->CanImport( context.ToVfsPath( files[ f ] ), context ) )
                continue;
            if ( !options.IsInAssetFilter( files[ f ] ) )
            {
                filteredOut.push_back( files[ f ] );
                break;
            }
            SSelectedAsset asset;
            asset.assetPath = files[ f ];
            asset.importer = importers[ i ];
            const CORE::CString stem = CVfsPaths::WithoutExtension( files[ f ] ).Lowercase();
            GUCEF::map< CORE::CString, CORE::CString >::const_iterator taken = intermediateNames.find( stem );
            if ( taken != intermediateNames.end() )
                asset.clashWith = (*taken).second;
            else
                intermediateNames[ stem ] = files[ f ];
            selected.push_back( asset );
            break;
        }
    }
}

/** Writes the cache entry of an asset that converted without errors, with the files its report entry lists */
void
SaveCacheEntry( CAssetConversionJob& job, const CConversionReportEntry* entry, bool cookPending, const CORE::CString& intermediateDir )
{
    if ( GUCEF_NULL == entry || entry->HasErrors() )
        return;
    job.cacheEntry.files = entry->files;
    job.cacheEntry.cookPending = cookPending;
    if ( !job.cacheEntry.Save( CAssetCacheEntry::GetEntryPath( intermediateDir, job.assetPath ) ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: The cache entry of " << job.assetPath << " cannot be written" );
}

/** The cooking of the jobs that are to be cooked, per profile, with the reference graph and the size report of each */
void
CookAssets( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CAssetPackContext& context, const SRunSetup& setup,
            GUCEF::vector< CAssetConversionJob >& jobs, const CORE::CString::StringVector& filteredOut, CConversionReport& report )
{
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();
    const CORE::CString outputDir = options.outputDir.IsNULLOrEmpty() ? manifest.GetOutputPath() : options.outputDir;

    // the assets the filter left out keep their files in the references and sizes of the profiles
    GUCEF::vector< CAssetCacheEntry > otherEntries;
    for ( size_t i=0; i<filteredOut.size(); ++i )
    {
        CAssetCacheEntry stored;
        const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( intermediateDir, filteredOut[ i ] );
        if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( entryPath ) && stored.Load( entryPath ) )
            otherEntries.push_back( stored );
    }

    for ( size_t c=0; c<setup.cookers.size(); ++c )
    {
        for ( size_t p=0; p<setup.profiles.size(); ++p )
        {
            const CORE::CString targetDir = setup.cookers.size() > 1 ? CVfsPaths::Join( outputDir, setup.cookers[ c ]->GetTargetName() ) : outputDir;
            const CORE::CString profileDir = CVfsPaths::Join( targetDir, setup.profiles[ p ].name );
            CCookSettings cookSettings;
            cookSettings.target = setup.cookers[ c ]->GetTargetName();
            cookSettings.profile = setup.profiles[ p ];
            cookSettings.packId = manifest.id;
            cookSettings.sourceRoot = context.GetSourceRoot();
            cookSettings.lodPolicy = manifest.GetLodPolicy();
            const CORE::CDataNode* lod = manifest.rules.FindChild( "lod" );
            if ( GUCEF_NULL != lod )
                cookSettings.lodSettings = *lod;
            cookSettings.colliderPolicy = manifest.GetColliderPolicy();
            cookSettings.force = options.force;
            manifest.GetPackSettings( cookSettings.settings );
            for ( size_t j=0; j<jobs.size(); ++j )
            {
                if ( !( jobs[ j ].converted || jobs[ j ].cookPending ) || !jobs[ j ].cookAllowed )
                    continue;
                CCookSettings::SAsset asset;
                asset.assetPath = jobs[ j ].assetPath;
                asset.sidecarFile = CConversionSidecar::GetPath( intermediateDir, jobs[ j ].assetPath );
                for ( size_t f=0; f<jobs[ j ].cacheEntry.files.size(); ++f )
                    if ( !jobs[ j ].cacheEntry.files[ f ].IsCooked() )
                        asset.files.push_back( jobs[ j ].cacheEntry.files[ f ].path );
                cookSettings.assets.push_back( asset );
            }
            TAssetCookerPtr cooker = setup.cookers[ c ];
            if ( !cookSettings.assets.empty() && !cooker->Cook( intermediateDir, profileDir, cookSettings, report ) )
            {
                report.SelectAsset( CORE::CString::Empty );
                report.AddError( setup.cookers[ c ]->GetTargetName(), setup.profiles[ p ].name, "the cooking failed" );
            }

            // the reference graph and the size report of the profile, of every asset that has cooked files in it
            CConversionReport graphReport;
            const CConversionReport* graphSource = &report;
            if ( !otherEntries.empty() )
            {
                graphReport = report;
                for ( size_t o=0; o<otherEntries.size(); ++o )
                {
                    if ( GUCEF_NULL != graphReport.FindEntry( otherEntries[ o ].assetPath ) )
                        continue;
                    CConversionReportEntry& other = graphReport.SelectAsset( otherEntries[ o ].assetPath );
                    other.cached = true;
                    for ( size_t f=0; f<otherEntries[ o ].files.size(); ++f )
                        other.AddFile( otherEntries[ o ].files[ f ] );
                }
                graphSource = &graphReport;
            }
            CReferenceGraph graph;
            graph.Build( *graphSource, setup.profiles[ p ].name, profileDir );
            // the normalized relations of every asset with cooked files in the profile, from its sidecar (converter design 5.8.5)
            const CConversionReport::TEntryVector& graphEntries = graphSource->GetEntries();
            for ( size_t e=0; e<graphEntries.size(); ++e )
            {
                CORE::CString::StringVector cookedFiles;
                graph.GetFilesOfAsset( graphEntries[ e ].assetPath, cookedFiles );
                const CORE::CString sidecarPath = CConversionSidecar::GetPath( intermediateDir, graphEntries[ e ].assetPath );
                if ( cookedFiles.empty() || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( sidecarPath ) )
                    continue;
                CORE::CDataNode sidecar;
                CAssetReference::TReferenceVector relations;
                if ( CCanonicalJson::Load( sidecarPath, sidecar ) && CConversionSidecar::ReadRelations( sidecar, relations ) )
                    graph.SetRelations( graphEntries[ e ].assetPath, relations );
            }
            CSizeReport sizes;
            sizes.Build( graph, profileDir );
            graph.Save( CVfsPaths::Join( profileDir, "references.json" ) );
            sizes.Save( CVfsPaths::Join( profileDir, "sizes.json" ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The run of the cook-only mode: cooks what the intermediate holds for the assets of the run */
bool
CookOnlyRun( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CAssetPackContext& context,
                          const SRunSetup& setup, GUCEF::vector< CAssetConversionJob >& jobs, const CORE::CString::StringVector& filteredOut,
                          CConversionReport& report, const CORE::CString& reportPath )
{
    // the assets with an intermediate: their cache entries name the files to cook
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    UInt32 withIntermediate = 0;
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        CAssetConversionJob& job = jobs[ j ];
        CConversionReportEntry& entry = report.SelectAsset( job.assetPath );
        entry.importer = job.importer->GetName();
        CAssetCacheEntry stored;
        const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( intermediateDir, job.assetPath );
        CORE::CString missing;
        if ( !vfs.FileExists( entryPath ) || !stored.Load( entryPath ) )
            missing = "there is no cache entry of the asset in the intermediate";
        for ( size_t f=0; missing.IsNULLOrEmpty() && f<stored.files.size(); ++f )
            if ( !stored.files[ f ].IsCooked() && !vfs.FileExists( stored.files[ f ].path ) )
                missing = "the intermediate file " + stored.files[ f ].path + " is gone";
        if ( !missing.IsNULLOrEmpty() )
        {
            entry.skipped = true;
            report.AddSkipped( PipelineSource, job.assetPath, missing + ": convert the asset first" );
            continue;
        }
        entry.cached = true;
        for ( size_t f=0; f<stored.files.size(); ++f )
            if ( !stored.files[ f ].IsCooked() )
                entry.AddFile( stored.files[ f ] );
        job.cacheEntry = stored;
        job.cached = true;
        job.cookPending = true;
        job.cookAllowed = true;
        ++withIntermediate;
    }
    if ( 0 == withIntermediate )
    {
        report.SelectAsset( CORE::CString::Empty );
        report.AddError( PipelineSource, intermediateDir, "there is no intermediate to cook from: convert the pack first" );
        report.SaveJson( reportPath );
        report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
        return false;
    }

    CIAssetPipelineObserver* observer = options.observer;
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_COOK );
    CookAssets( manifest, options, context, setup, jobs, filteredOut, report );

    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_REPORT );
    UInt32 done = 0;
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        if ( !jobs[ j ].cookPending )
            continue;
        const CConversionReportEntry* entry = report.FindEntry( jobs[ j ].assetPath );
        SaveCacheEntry( jobs[ j ], entry, false, intermediateDir );
        ++done;
        if ( GUCEF_NULL != observer )
        {
            observer->OnAssetFinished( done, withIntermediate, jobs[ j ].assetPath );
            if ( GUCEF_NULL != entry )
                observer->OnAssetReport( *entry, done, withIntermediate );
        }
    }
    report.SaveJson( reportPath );
    report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << " cooked from its intermediate: " << withIntermediate << " of " << (UInt32) jobs.size() << " assets" );
    return !report.HasFailures();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPipeline::CAssetPipeline( void )
    : m_lastRun()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPipeline::~CAssetPipeline()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CImportActivityLog::SRun&
CAssetPipeline::GetLastRun( void ) const
{GUCEF_TRACE;

    return m_lastRun;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::GetCurrentPluginVersion( const CORE::CString& pluginKey, UInt32& version )
{GUCEF_TRACE;

    if ( ConverterPluginKey == pluginKey )
    {
        version = CAssetConverterGlobal::GetConverterVersion();
        return true;
    }
    const Int32 colon = pluginKey.HasChar( ':', true );
    if ( colon <= 0 )
        return false;
    const CORE::CString role = pluginKey.SubstrFromRange( 0, (UInt32) colon );
    const CORE::CString name = pluginKey.CutChars( (UInt32) colon+1, true );
    CAssetPluginRegistry& plugins = Plugins();
    if ( "importer" == role )
    {
        TAssetImporterPtr plugin;
        if ( !plugins.TryGetImporter( name, plugin ) )
            return false;
        version = plugin->GetVersion();
        return true;
    }
    if ( "exporter" == role )
    {
        TAssetExporterPtr plugin;
        if ( !plugins.TryGetExporter( name, plugin ) )
            return false;
        version = plugin->GetVersion();
        return true;
    }
    if ( "cooker" == role )
    {
        TAssetCookerPtr plugin;
        if ( !plugins.TryGetCooker( name, plugin ) )
            return false;
        version = plugin->GetVersion();
        return true;
    }
    if ( "postProcessor" == role )
    {
        TAssetPostProcessorPtr plugin;
        if ( !plugins.TryGetPostProcessor( name, plugin ) )
            return false;
        version = plugin->GetVersion();
        return true;
    }
    if ( "validator" == role )
    {
        TAssetValidatorPtr plugin;
        if ( !plugins.TryGetValidator( name, plugin ) )
            return false;
        version = plugin->GetVersion();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TActivityReason
CAssetPipeline::CheckStoredEntry( const CAssetCacheEntry& stored, const CORE::CString& sourceVfsPath, const CORE::CString& expectedSettingsHash, CORE::CString& detail, bool& sourceVerified )
{GUCEF_TRACE;

    detail.Clear();
    CAssetCacheEntry now = stored;

    // the source
    const CORE::CString sourceHash = sourceVfsPath.IsNULLOrEmpty() ? CORE::CString::Empty : CContentHash::HashFile( sourceVfsPath );
    sourceVerified = !sourceHash.IsNULLOrEmpty();
    if ( sourceVerified )
        now.sourceHash = sourceHash;

    // the settings, when the caller knows them
    if ( !expectedSettingsHash.IsNULLOrEmpty() )
        now.settingsHash = expectedSettingsHash;

    // the plugins that are there now; the others cannot be judged
    CAssetCacheEntry::TPluginVersionMap::const_iterator p = stored.plugins.begin();
    while ( p != stored.plugins.end() )
    {
        UInt32 version = 0;
        if ( GetCurrentPluginVersion( (*p).first, version ) )
            now.plugins[ (*p).first ] = version;
        ++p;
    }

    // the files the import read besides the source
    CAssetCacheEntry::THashMap::const_iterator d = stored.dependencies.begin();
    while ( d != stored.dependencies.end() )
    {
        const CORE::CString hash = CContentHash::HashFile( (*d).first );
        if ( !hash.IsNULLOrEmpty() )
            now.dependencies[ (*d).first ] = hash;
        ++d;
    }
    return stored.CompareWith( now, detail );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::EvaluateCache( const CAssetConversionJob& job        ,
                               CORE::CDataNode& settings             ,
                               CORE::CString::StringVector& layers   ,
                               CAssetCacheEntry& current             ,
                               CAssetCacheEntry& stored              ,
                               bool& hasStored                       ,
                               TActivityReason& reason               ,
                               CORE::CString& detail                 ,
                               CORE::CString& sourceProblem          )
{GUCEF_TRACE;

    hasStored = false;
    reason = job.reimport ? ACTIVITYREASON_FORCED : ACTIVITYREASON_NEW;
    detail.Clear();
    sourceProblem.Clear();
    if ( GUCEF_NULL == job.manifest || GUCEF_NULL == job.context || job.importer.IsNULL() )
    {
        sourceProblem = "the conversion job is incomplete";
        return false;
    }
    const CAssetPackManifest& manifest = *job.manifest;
    const CORE::CString vfsPath = job.context->ToVfsPath( job.assetPath );
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();

    // the effective settings
    CORE::CDataNode defaults( "settings", GUCEF_DATATYPE_OBJECT );
    job.importer->GetDefaultSettings( defaults );
    layers.clear();
    CImportSettingsBuilder::Build( defaults, manifest, job.assetPath, job.importer->GetName(), settings, layers );

    // the parts of the key that are known before the import
    current = CAssetCacheEntry();
    current.assetPath = job.assetPath;
    current.sourceHash = CContentHash::HashFile( vfsPath );
    if ( current.sourceHash.IsNULLOrEmpty() )
    {
        sourceProblem = "the source " + vfsPath + " cannot be read";
        return false;
    }
    {
        CORE::CDataNode keySettings( "settings", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* effective = keySettings.AddChild( settings );
        effective->SetName( "effective" );
        keySettings.SetAttribute( "processing", job.processingHash );
        current.settingsHash = CCanonicalJson::Hash( keySettings );
    }

    // the stored entry with the current versions of its plugins and the current hashes of its dependencies
    const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( intermediateDir, job.assetPath );
    if ( job.reimport || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( entryPath ) || !stored.Load( entryPath ) )
        return false;
    hasStored = true;
    CAssetCacheEntry now = current;
    CAssetCacheEntry::TPluginVersionMap::const_iterator p = stored.plugins.begin();
    while ( p != stored.plugins.end() )
    {
        UInt32 version = 0;
        if ( GetCurrentPluginVersion( (*p).first, version ) )
            now.plugins[ (*p).first ] = version;
        ++p;
    }
    CAssetCacheEntry::TPluginVersionMap::const_iterator c = job.cookerVersions.begin();
    while ( c != job.cookerVersions.end() )
    {
        now.plugins[ (*c).first ] = (*c).second;
        ++c;
    }
    CORE::CString::StringVector dependencyPaths;
    CAssetCacheEntry::THashMap::const_iterator d = stored.dependencies.begin();
    while ( d != stored.dependencies.end() )
    {
        dependencyPaths.push_back( (*d).first );
        ++d;
    }
    HashFiles( dependencyPaths, now.dependencies );
    reason = stored.CompareWith( now, detail );
    if ( ACTIVITYREASON_UNCHANGED == reason )
    {
        for ( size_t f=0; f<stored.files.size(); ++f )
        {
            if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( stored.files[ f ].path ) )
            {
                reason = ACTIVITYREASON_OUTPUT_MISSING;
                detail = stored.files[ f ].path;
                break;
            }
        }
    }
    return ACTIVITYREASON_UNCHANGED == reason;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::ConvertAsset( CAssetConversionJob& job )
{GUCEF_TRACE;

    const UInt64 startTicks = MT::PrecisionTickCount();
    job.cached = false;
    job.converted = false;
    job.cookAllowed = false;
    job.cookPending = false;
    if ( GUCEF_NULL == job.manifest || GUCEF_NULL == job.context || GUCEF_NULL == job.baker || job.importer.IsNULL() )
        return false;

    const CAssetPackManifest& manifest = *job.manifest;
    CAssetPackContext& context = *job.context;
    CConversionReport& report = job.report;
    report.SetPackId( manifest.id );
    report.SetLicenceRecordId( manifest.licenceRecord );
    CConversionReportEntry& entry = report.SelectAsset( job.assetPath );
    entry.importer = job.importer->GetName();
    const CORE::CString vfsPath = context.ToVfsPath( job.assetPath );
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();

    // the effective settings and the cache: an unchanged asset keeps the files of the last run
    CORE::CDataNode settings;
    CORE::CString::StringVector layers;
    CAssetCacheEntry current;
    CAssetCacheEntry stored;
    bool hasStored = false;
    TActivityReason reason = ACTIVITYREASON_NEW;
    CORE::CString detail;
    CORE::CString sourceProblem;
    const bool unchanged = EvaluateCache( job, settings, layers, current, stored, hasStored, reason, detail, sourceProblem );
    if ( !sourceProblem.IsNULLOrEmpty() )
    {
        report.AddError( PipelineSource, job.assetPath, sourceProblem );
        Finish( job, ACTIVITYACTION_FAILED, ACTIVITYREASON_NEW, "source not readable", startTicks );
        return false;
    }
    if ( unchanged && !job.validateOnly )
    {
        entry.cached = true;
        for ( size_t f=0; f<stored.files.size(); ++f )
            entry.AddFile( stored.files[ f ] );
        job.cacheEntry = stored;
        job.cached = true;
        if ( stored.cookPending )
        {
            // the intermediate stands, an earlier run did not get to the cooking: this run cooks it
            job.cookPending = true;
            job.cookAllowed = true;
            Finish( job, ACTIVITYACTION_CACHED, ACTIVITYREASON_COOK_PENDING, CORE::CString::Empty, startTicks );
        }
        else
        {
            Finish( job, ACTIVITYACTION_CACHED, ACTIVITYREASON_UNCHANGED, CORE::CString::Empty, startTicks );
        }
        return true;
    }

    // the import
    CAssetScene scene;
    scene.name = CVfsPaths::StemOf( job.assetPath );
    scene.settings = settings;
    scene.provenance.packId = manifest.id;
    scene.provenance.assetPath = job.assetPath;
    scene.provenance.sourceId = job.assetPath;
    scene.provenance.sourceFormat = CVfsPaths::ExtensionOf( job.assetPath );
    scene.provenance.importerName = job.importer->GetName();
    scene.provenance.importerVersion = job.importer->GetVersion();
    scene.provenance.converterVersion = CAssetConverterGlobal::GetConverterVersion();
    const bool imported = job.importer->Import( vfsPath, context, scene, report );
    if ( report.IsCancellationRequested() )
        return FinishCancelled( job, entry, startTicks );
    if ( !imported )
    {
        if ( entry.skipped && !entry.HasErrors() )
        {
            // the importer found nothing it converts in the asset and said why (CConversionReport::SetCurrentAssetSkipped())
            Finish( job, ACTIVITYACTION_SKIPPED, reason, "nothing the importer converts", startTicks );
            return false;
        }
        if ( !entry.HasErrors() )
            report.AddError( job.importer->GetName(), job.assetPath, "the import failed" );
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }
    if ( scene.provenance.sourceFormat.IsNULLOrEmpty() )
        scene.provenance.sourceFormat = CVfsPaths::ExtensionOf( job.assetPath );
    current.plugins[ "importer:" + job.importer->GetName() ] = job.importer->GetVersion();
    current.plugins[ ConverterPluginKey ] = CAssetConverterGlobal::GetConverterVersion();

    // the normalisation
    if ( !CAssetNormaliser::Normalise( scene, report ) )
    {
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }

    // the post-processors: the stack of the manifest for the source format, else the default of the importer
    CPluginStackEntry::TEntryVector stack;
    if ( !manifest.GetPostProcessorStack( scene.provenance.sourceFormat, stack ) )
    {
        CORE::CString::StringVector names;
        job.importer->GetDefaultPostProcessors( names );
        for ( size_t i=0; i<names.size(); ++i )
            stack.push_back( CPluginStackEntry( names[ i ] ) );
    }
    for ( size_t i=0; i<stack.size(); ++i )
    {
        TAssetPostProcessorPtr processor;
        if ( !Plugins().TryGetPostProcessor( stack[ i ].name, processor ) )
        {
            report.AddError( PipelineSource, stack[ i ].name, "the post-processor is not registered" );
            continue;
        }
        if ( !processor->AppliesTo( scene.provenance.sourceFormat ) )
            continue;
        CORE::CDataNode pluginSettings;
        CImportSettingsBuilder::GetPluginSettings( scene.settings, stack[ i ].settings, processor->GetName(), pluginSettings );
        if ( !processor->Process( scene, pluginSettings, report ) )
            report.AddError( processor->GetName(), job.assetPath, "the post-processor failed" );
        current.plugins[ "postProcessor:" + processor->GetName() ] = processor->GetVersion();
    }
    if ( entry.HasErrors() )
    {
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }

    // the textures
    for ( size_t t=0; t<scene.textures.size(); ++t )
    {
        if ( report.IsCancellationRequested() )
            return FinishCancelled( job, entry, startTicks );
        job.baker->Bake( scene.textures[ t ], report );
    }
    if ( report.IsCancellationRequested() )
        return FinishCancelled( job, entry, startTicks );

    // the validators; their errors keep the asset from being cooked unless forced
    const UInt32 errorsBefore = entry.GetItemCount( REPORTITEM_ERROR );
    CPluginStackEntry::TEntryVector validators;
    manifest.GetValidators( validators );
    for ( size_t i=0; i<validators.size(); ++i )
    {
        TAssetValidatorPtr validator;
        if ( !Plugins().TryGetValidator( validators[ i ].name, validator ) )
        {
            report.AddWarning( PipelineSource, validators[ i ].name, "the validator is not registered" );
            continue;
        }
        CORE::CDataNode pluginSettings;
        CImportSettingsBuilder::GetPluginSettings( scene.settings, validators[ i ].settings, validator->GetName(), pluginSettings );
        if ( job.maxBones > 0 && pluginSettings.GetAttributeValueOrChildValueByName( "maxBones" ).IsNULLOrEmpty() )
            pluginSettings.SetAttribute( "maxBones", job.maxBones );
        if ( !validator->Validate( scene, pluginSettings, report ) )
            report.AddWarning( validator->GetName(), job.assetPath, "the validator could not run" );
        current.plugins[ "validator:" + validator->GetName() ] = validator->GetVersion();
    }
    const bool validationErrors = entry.GetItemCount( REPORTITEM_ERROR ) > errorsBefore;

    // validate-only: nothing is exported, nothing is written
    if ( job.validateOnly )
    {
        Finish( job, validationErrors ? ACTIVITYACTION_FAILED : ACTIVITYACTION_IMPORTED, reason, detail, startTicks );
        return !validationErrors;
    }

    // the export and the sidecar
    if ( job.exporter.IsNULL() )
    {
        report.AddError( PipelineSource, job.assetPath, "there is no exporter for the intermediate" );
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }
    if ( report.IsCancellationRequested() )
        return FinishCancelled( job, entry, startTicks );
    const CORE::CString exportDir = CVfsPaths::Join( intermediateDir, CVfsPaths::FolderOf( job.assetPath ) );
    if ( !job.exporter->Export( scene, exportDir, report ) )
    {
        report.AddError( job.exporter->GetName(), job.assetPath, "the export failed" );
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }
    current.plugins[ "exporter:" + job.exporter->GetName() ] = job.exporter->GetVersion();
    CAssetCacheEntry::TPluginVersionMap::const_iterator c = job.cookerVersions.begin();
    while ( c != job.cookerVersions.end() )
    {
        current.plugins[ (*c).first ] = (*c).second;
        ++c;
    }

    const CORE::CString sidecarPath = CConversionSidecar::GetPath( intermediateDir, job.assetPath );
    CORE::CDataNode sidecar;
    CConversionSidecar::Build( scene, entry, layers, manifest.licenceRecord, context, intermediateDir, sidecar );
    if ( !CCanonicalJson::Store( sidecarPath, sidecar ) )
    {
        report.AddError( PipelineSource, sidecarPath, "the sidecar cannot be written" );
        Finish( job, ACTIVITYACTION_FAILED, reason, detail, startTicks );
        return false;
    }
    report.AddOutputFile( sidecarPath, CORE::CString::StringVector() );

    // the cache entry as this run makes it; the pipeline adds the cooked files and stores it
    HashFiles( scene.dependencies, current.dependencies );
    current.files = entry.files;
    current.ComputeKey();
    job.cacheEntry = current;
    job.converted = true;
    if ( validationErrors )
        entry.cookBlocked = !job.force;
    job.cookAllowed = !validationErrors || job.force;
    Finish( job, validationErrors ? ACTIVITYACTION_FAILED : ACTIVITYACTION_IMPORTED, reason, detail, startTicks );
    return !validationErrors;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::StoreCacheEntry( CAssetConversionJob& job, bool cookPending )
{GUCEF_TRACE;

    if ( GUCEF_NULL == job.manifest || !job.converted || job.report.IsCancelled() )
        return false;
    const CConversionReportEntry* entry = job.report.FindEntry( job.assetPath );
    if ( GUCEF_NULL == entry || entry->HasErrors() )
        return false;
    const CORE::CString intermediateDir = job.manifest->GetIntermediatePath();
    SaveCacheEntry( job, entry, cookPending, intermediateDir );
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( CAssetCacheEntry::GetEntryPath( intermediateDir, job.assetPath ) );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAssetPipeline::ConvertAssetAsync( const CAssetConversionJob& job, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName );
    if ( pool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    CAssetConversionJob copy( job );
    return pool->QueueTask( CAssetConversionTaskConsumer::TaskType, &copy );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::TryGetResult( const CORE::CFutureResult& future, CAssetConversionJob*& job )
{GUCEF_TRACE;

    job = GUCEF_NULL;
    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return false;
    job = CORE::CICloneable::CloneableAs< CAssetConversionJob >( task->GetTaskData() );
    return GUCEF_NULL != job;
}

/*-------------------------------------------------------------------------*/

void
CAssetPipeline::Refuse( const CORE::CString& reason, const CAssetPipelineOptions& options, CAssetPackContext& context,
                        const GUCEF::vector< TAssetImporterPtr >& importers, CConversionReport& report )
{GUCEF_TRACE;

    report.SetRefused( reason );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPipeline: Pack " << context.GetManifest().id << " is refused: " << reason );
    const CORE::CString::StringVector& files = context.GetFiles();
    for ( size_t f=0; f<files.size(); ++f )
    {
        if ( !context.GetManifest().IsAssetSelected( files[ f ] ) || !options.IsInAssetFilter( files[ f ] ) )
            continue;
        for ( size_t i=0; i<importers.size(); ++i )
        {
            if ( !importers[ i ]->CanImport( context.ToVfsPath( files[ f ] ), context ) )
                continue;
            CConversionReportEntry& entry = report.SelectAsset( files[ f ] );
            entry.refused = true;
            entry.importer = importers[ i ]->GetName();
            CImportActivityEntry activity;
            activity.assetPath = files[ f ];
            activity.action = ACTIVITYACTION_REFUSED;
            activity.reason = ACTIVITYREASON_LICENCE;
            activity.detail = reason;
            m_lastRun.entries.push_back( activity );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

namespace {

/** The inputs of the pack index of a pack the pipeline holds open: one listing (the files the context already knows), the profile the recognisers give and the options */
void
MakeIndexInputs( const CAssetPackManifest& manifest, const CAssetPackContext& context, const CLicenceRecord* record, CAssetPackListing& listing, CAssetPackProfile& profile, CPackIndexBuildOptions& buildOptions )
{GUCEF_TRACE;

    listing.SetVfsRoot( context.GetSourceRoot() );
    listing.SetPackName( manifest.id );
    const CORE::CString::StringVector& files = context.GetFiles();
    for ( size_t i=0; i<files.size(); ++i )
        listing.AddFile( files[ i ] );

    CAssetPluginRegistry& registry = Plugins();
    registry.RecognisePack( listing, profile );
    buildOptions.packName = manifest.id;
    buildOptions.packTitle = manifest.title;
    buildOptions.tool = "AssetConverterLib";
    buildOptions.toolVersion = CORE::UInt32ToString( CAssetConverterGlobal::GetConverterVersion() );
    buildOptions.outputRoot = "intermediate";
    GUCEF::vector< TAssetImporterPtr > importers;
    registry.GetImporters( importers );
    for ( size_t i=0; i<importers.size(); ++i )
        buildOptions.plugins[ "recogniser:" + importers[ i ]->GetName() ] = importers[ i ]->GetVersion();
    buildOptions.licence.recordId = manifest.licenceRecord;
    if ( GUCEF_NULL != record )
        for ( CORE::CString::StringSet::const_iterator r = record->restrictions.begin(); r != record->restrictions.end(); ++r )
            buildOptions.licence.restrictions.push_back( *r );
}

}

/*-------------------------------------------------------------------------*/

const CPackIndexRunResult&
CAssetPipeline::GetLastIndexResult( void ) const
{GUCEF_TRACE;

    return m_lastIndexResult;
}

/*-------------------------------------------------------------------------*/

void
CAssetPipeline::UpdateIndex( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, const CAssetPackContext& context, const CConversionReport* report, bool licenceRefused )
{GUCEF_TRACE;

    if ( options.noIndex )
        return;
    CLicenceRecord record;
    bool recordRead = false;
    CheckLicence( manifest, record, recordRead );

    CAssetPackListing listing;
    CAssetPackProfile profile;
    CPackIndexBuildOptions buildOptions;
    MakeIndexInputs( manifest, context, recordRead ? &record : GUCEF_NULL, listing, profile, buildOptions );

    CPackIndexUpdateOptions updateOptions;
    updateOptions.level = ( options.analyse || options.reanalyse || options.retryFailed ) ? CPackIndexUpdateOptions::LEVEL_ANALYSIS : CPackIndexUpdateOptions::LEVEL_STRUCTURE;
    updateOptions.redo.reindex = options.reindex;
    updateOptions.redo.reanalyse = options.reanalyse;
    updateOptions.redo.rethumbnail = options.rethumbnail;
    updateOptions.redo.reimport = options.reimport;
    updateOptions.redo.retryFailed = options.retryFailed;
    updateOptions.intermediateDir = manifest.GetIntermediatePath();
    updateOptions.report = report;
    updateOptions.licenceRefused = licenceRefused;
    updateOptions.packVfsRoot = context.GetSourceRoot();

    const CORE::CString indexFolder = options.indexFolder.IsNULLOrEmpty() ? manifest.GetIntermediatePath() : options.indexFolder;
    CPackIndexUpdater updater( Plugins().GetKinds() );
    CPackIndexDocument document;
    updater.EnsureIndex( listing, profile, buildOptions, updateOptions, indexFolder, document, m_lastIndexResult );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << ": " << m_lastIndexResult.ToSummaryText() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::PlanPack( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CAssetPackPlan& plan )
{GUCEF_TRACE;

    plan.Clear();
    plan.packId = manifest.id;
    plan.mode = options.mode;
    plan.licenceRecordId = manifest.licenceRecord;
    plan.assetFilter = options.assetGlobs;
    plan.intermediateDir = manifest.GetIntermediatePath();
    CIntermediateLock::GetCurrentHolder( plan.intermediateDir, plan.lockHolder );

    SRunSetup setup;
    ResolveImporters( manifest, setup );
    plan.cookTargets = options.cookTargets.empty() ? manifest.cookTargets : options.cookTargets;

    CAssetPackContext context;
    if ( !context.Open( manifest ) )
    {
        plan.problems.push_back( "the source of the pack " + manifest.GetSourcePath() + " cannot be opened" );
        return false;
    }

    // the licence decision; a refused pack lists its assets as refused, as a run does
    CLicenceRecord record;
    bool recordRead = false;
    plan.licenceReason = CheckLicence( manifest, record, recordRead );
    if ( recordRead )
        plan.restrictions = record.restrictions;
    plan.conversionAllowed = plan.licenceReason.IsNULLOrEmpty();
    if ( !options.noIndex )
    {
        CAssetPackListing indexListing;
        CAssetPackProfile indexProfile;
        CPackIndexBuildOptions indexBuildOptions;
        MakeIndexInputs( manifest, context, recordRead ? &record : GUCEF_NULL, indexListing, indexProfile, indexBuildOptions );
        CPackIndexUpdateOptions indexUpdateOptions;
        indexUpdateOptions.redo.reindex = options.reindex;
        indexUpdateOptions.redo.reanalyse = options.reanalyse;
        indexUpdateOptions.redo.retryFailed = options.retryFailed;
        CPackIndexUpdater indexUpdater( Plugins().GetKinds() );
        const CORE::CString indexFolder = options.indexFolder.IsNULLOrEmpty() ? plan.intermediateDir : options.indexFolder;
        plan.indexNote = indexUpdater.PlanIndex( indexListing, indexProfile, indexBuildOptions, indexUpdateOptions, indexFolder ).ToString();
    }
    if ( !plan.conversionAllowed )
    {
        const CORE::CString::StringVector& files = context.GetFiles();
        for ( size_t f=0; f<files.size(); ++f )
        {
            if ( !manifest.IsAssetSelected( files[ f ] ) )
                continue;
            for ( size_t i=0; i<setup.importers.size(); ++i )
            {
                if ( !setup.importers[ i ]->CanImport( context.ToVfsPath( files[ f ] ), context ) )
                    continue;
                if ( !options.IsInAssetFilter( files[ f ] ) )
                {
                    ++plan.filteredOut;
                    break;
                }
                CAssetPlanEntry entry;
                entry.assetPath = files[ f ];
                entry.importer = setup.importers[ i ]->GetName();
                entry.action = PLANACTION_REFUSED;
                entry.reason = ACTIVITYREASON_LICENCE;
                entry.detail = plan.licenceReason;
                plan.entries.push_back( entry );
                break;
            }
        }
        return true;
    }

    if ( !ResolvePlugins( manifest, options, setup ) )
    {
        plan.problems.push_back( setup.problemSubject + ": " + setup.problem );
        return true;
    }
    plan.cookTargets = setup.targets;

    for ( size_t i=0; i<setup.importers.size(); ++i )
        setup.importers[ i ]->IndexPack( context );
    TSelectedAssetVector selected;
    CORE::CString::StringVector filteredOut;
    SelectAssets( manifest, options, context, setup.importers, selected, filteredOut );
    plan.filteredOut = (UInt32) filteredOut.size();

    const CORE::CString intermediateDir = manifest.GetIntermediatePath();
    for ( size_t s=0; s<selected.size(); ++s )
    {
        CAssetPlanEntry entry;
        entry.assetPath = selected[ s ].assetPath;
        entry.importer = selected[ s ].importer->GetName();
        if ( !selected[ s ].clashWith.IsNULLOrEmpty() )
        {
            entry.action = PLANACTION_FAILED;
            entry.reason = ACTIVITYREASON_NEW;
            entry.detail = "its intermediate would have the name of that of " + selected[ s ].clashWith;
            plan.entries.push_back( entry );
            continue;
        }

        CAssetConversionJob job;
        job.manifest = &manifest;
        job.context = &context;
        job.importer = selected[ s ].importer;
        job.exporter = setup.exporter;
        job.cookerVersions = setup.cookerVersions;
        job.processingHash = setup.processingHash;
        job.maxBones = setup.maxBones;
        job.force = options.force;
        job.reimport = options.reimport && PIPELINEMODE_COOK_ONLY != options.mode;
        job.validateOnly = PIPELINEMODE_VALIDATE_ONLY == options.mode;
        job.assetPath = selected[ s ].assetPath;

        CAssetCacheEntry current;
        CAssetCacheEntry stored;
        bool hasStored = false;
        CORE::CString sourceProblem;
        const bool unchanged = EvaluateCache( job, entry.settings, entry.layers, current, stored, hasStored, entry.reason, entry.detail, sourceProblem );
        if ( !sourceProblem.IsNULLOrEmpty() )
        {
            entry.action = PLANACTION_FAILED;
            entry.reason = ACTIVITYREASON_NEW;
            entry.detail = sourceProblem;
        }
        else
        if ( PIPELINEMODE_COOK_ONLY == options.mode )
        {
            CAssetCacheEntry intermediate;
            const CORE::CString entryPath = CAssetCacheEntry::GetEntryPath( intermediateDir, job.assetPath );
            if ( hasStored || ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( entryPath ) && intermediate.Load( entryPath ) ) )
            {
                entry.action = PLANACTION_COOK;
            }
            else
            {
                entry.action = PLANACTION_NO_INTERMEDIATE;
                entry.detail = "there is no cache entry of the asset in the intermediate";
            }
        }
        else
        if ( PIPELINEMODE_VALIDATE_ONLY == options.mode )
        {
            entry.action = PLANACTION_VALIDATE;
        }
        else
        if ( unchanged )
        {
            if ( stored.cookPending )
            {
                entry.action = PLANACTION_COOK;
                entry.reason = ACTIVITYREASON_COOK_PENDING;
            }
            else
            {
                entry.action = PLANACTION_CACHED;
            }
        }
        else
        {
            entry.action = PLANACTION_IMPORT;
        }
        plan.entries.push_back( entry );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::RunPack( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CConversionReport& report )
{GUCEF_TRACE;

    report.Clear();
    report.SetPackId( manifest.id );
    report.SetLicenceRecordId( manifest.licenceRecord );
    report.SetMode( options.mode );
    report.SetAssetFilter( options.assetGlobs );
    m_lastRun = CImportActivityLog::SRun();
    m_lastRun.time = CImportActivityLog::GetUtcTimeText();
    m_lastRun.packId = manifest.id;

    // a run that writes holds the lock of the intermediate: two runs never write the same intermediate at once
    if ( PIPELINEMODE_VALIDATE_ONLY == options.mode )
        return RunPackImpl( manifest, options, report );
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();
    CIntermediateLock lock( intermediateDir );
    if ( !lock.IsHeld() )
    {
        report.SetLocked( lock.GetHolder() );
        report.AddError( PipelineSource, intermediateDir, "the intermediate is in use by " + lock.GetHolder() );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPipeline: Pack " << manifest.id << " is not run: its intermediate " << intermediateDir << " is in use by " << lock.GetHolder() );
        return false;
    }
    return RunPackImpl( manifest, options, report );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipeline::RunPackImpl( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CConversionReport& report )
{GUCEF_TRACE;

    const bool validateOnly = PIPELINEMODE_VALIDATE_ONLY == options.mode;
    const bool cookOnly = PIPELINEMODE_COOK_ONLY == options.mode;
    const bool writesActivity = options.writeActivityLog && PIPELINEMODE_FULL == options.mode;
    const CORE::CString intermediateDir = manifest.GetIntermediatePath();
    const CORE::CString reportPath = options.reportPath.IsNULLOrEmpty() ? CVfsPaths::Join( intermediateDir, "report.json" ) : options.reportPath;
    const bool writesReport = !validateOnly || !options.reportPath.IsNULLOrEmpty();
    CIAssetPipelineObserver* observer = options.observer;

    SRunSetup setup;
    ResolveImporters( manifest, setup );

    // the source
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
    {
        report.AddError( PipelineSource, manifest.GetSourcePath(), "the source of the pack cannot be opened" );
        if ( writesReport )
            report.SaveJson( reportPath );
        return false;
    }

    // the pack index alone: no licence check (the index is metadata), no conversion, no cooking
    if ( options.indexOnly )
    {
        UpdateIndex( manifest, options, context, GUCEF_NULL, false );
        const bool indexed = m_lastIndexResult.reused || m_lastIndexResult.written || m_lastIndexResult.locked;
        if ( !indexed )
            report.AddError( PipelineSource, manifest.id, "the pack index could not be built: " + m_lastIndexResult.message );
        return indexed && !report.HasFailures();
    }

    // the licence record decides whether anything is converted
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_LICENCE );
    CLicenceRecord record;
    bool recordRead = false;
    const CORE::CString refusal = CheckLicence( manifest, record, recordRead );
    if ( !refusal.IsNULLOrEmpty() )
    {
        Refuse( refusal, options, context, setup.importers, report );
        UpdateIndex( manifest, options, context, &report, true );
        if ( writesActivity )
            CImportActivityLog::AppendRun( CImportActivityLog::GetLogPath( intermediateDir ), m_lastRun );
        if ( writesReport )
        {
            report.SaveJson( reportPath );
            report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
        }
        return false;
    }

    // the plugins of the run
    if ( !ResolvePlugins( manifest, options, setup ) )
    {
        report.AddError( PipelineSource, setup.problemSubject, setup.problem );
        if ( writesReport )
            report.SaveJson( reportPath );
        return false;
    }

    // the index of the pack, then the assets: the files an importer can import that the run is for
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_INDEX );
    for ( size_t i=0; i<setup.importers.size(); ++i )
    {
        if ( !setup.importers[ i ]->IndexPack( context ) )
            report.AddWarning( setup.importers[ i ]->GetName(), manifest.id, "the index of the pack could not be built completely" );
    }
    CTextureBaker baker( CVfsPaths::Join( intermediateDir, "textures" ) );
    baker.SetWritesFiles( !validateOnly );
    TSelectedAssetVector selected;
    CORE::CString::StringVector filteredOut;
    SelectAssets( manifest, options, context, setup.importers, selected, filteredOut );
    GUCEF::vector< CAssetConversionJob > jobs;
    for ( size_t s=0; s<selected.size(); ++s )
    {
        if ( !selected[ s ].clashWith.IsNULLOrEmpty() )
        {
            CConversionReportEntry& entry = report.SelectAsset( selected[ s ].assetPath );
            entry.importer = selected[ s ].importer->GetName();
            report.AddError( PipelineSource, selected[ s ].assetPath, "its intermediate would have the name of that of " + selected[ s ].clashWith );
            continue;
        }
        CAssetConversionJob job;
        job.manifest = &manifest;
        job.context = &context;
        job.baker = &baker;
        job.importer = selected[ s ].importer;
        job.exporter = setup.exporter;
        job.cookerVersions = setup.cookerVersions;
        job.processingHash = setup.processingHash;
        job.maxBones = setup.maxBones;
        job.force = options.force;
        job.reimport = options.reimport;
        job.validateOnly = validateOnly;
        job.assetPath = selected[ s ].assetPath;
        jobs.push_back( job );
    }
    const CORE::CString::StringVector& files = context.GetFiles();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << ": " << (UInt32) jobs.size() << " assets of " << (UInt32) files.size() << " files"
               << ( filteredOut.empty() ? CORE::CString::Empty : ", " + CORE::ToString( (UInt32) filteredOut.size() ) + " left out by the asset filter" )
               << ( PIPELINEMODE_FULL == options.mode ? CORE::CString::Empty : CORE::CString( ", mode " ) + ToString( options.mode ) ) );
    const UInt32 totalAssets = (UInt32) jobs.size();
    if ( GUCEF_NULL != observer )
        observer->OnPackIndexed( totalAssets );

    if ( cookOnly )
        {
            const bool cooked = CookOnlyRun( manifest, options, context, setup, jobs, filteredOut, report, reportPath );
            UpdateIndex( manifest, options, context, &report, false );
            return cooked;
        }

    // the assets as tasks; this thread waits for them, and works on them itself when the pool has no threads
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_CONVERT );
    const CORE::CString poolName = options.threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : options.threadPoolName;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
    if ( !pool.IsNULL() )
    {
        if ( options.threads > 0 )
        {
            pool->SetDesiredMaxTotalNrOfThreads( (Int32) options.threads );
            pool->SetDesiredMinNrOfWorkerThreads( options.threads );
        }
        else
        {
            pool->SetNrOfWorkerThreadsToLogicalCPUs( 1 );
        }
    }
    GUCEF::vector< CORE::CFutureResult > futures;
    futures.reserve( jobs.size() );
    for ( size_t j=0; j<jobs.size(); ++j )
        futures.push_back( ConvertAssetAsync( jobs[ j ], poolName ) );
    UInt32 finishedAssets = 0;
    bool cancelled = false;
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        if ( !cancelled && GUCEF_NULL != observer && observer->IsCancelRequested() )
        {
            cancelled = true;
            RequestCancellationOfRest( futures, j );
        }
        if ( futures[ j ].HasNoFuture() )
        {
            // the task could not be queued: the asset is converted here
            if ( !cancelled )
                ConvertAsset( jobs[ j ] );
        }
        else
        {
            while ( !futures[ j ].IsReady() )
            {
                if ( !pool.IsNULL() && 0 == pool->GetActiveNrOfWorkerThreads() && pool->CarryOutQueuedTasksIfAny( 1 ) > 0 )
                    continue;
                MT::PrecisionDelay( 2 );
                if ( !cancelled && GUCEF_NULL != observer && observer->IsCancelRequested() )
                {
                    cancelled = true;
                    RequestCancellationOfRest( futures, j + 1 );
                }
            }
            CAssetConversionJob* result = GUCEF_NULL;
            if ( TryGetResult( futures[ j ], result ) )
            {
                jobs[ j ] = *result;
            }
            else
            if ( !cancelled )
            {
                jobs[ j ].report.SelectAsset( jobs[ j ].assetPath );
                jobs[ j ].report.AddError( PipelineSource, jobs[ j ].assetPath, "the task of the asset did not finish" );
            }
        }

        CAssetConversionJob& job = jobs[ j ];
        const bool ran = job.converted || job.cached || !job.report.GetEntries().empty();
        if ( !ran )
        {
            // not started before the run was asked to stop: skipped, the next run converts it
            CConversionReportEntry& entry = job.report.SelectAsset( job.assetPath );
            entry.importer = job.importer->GetName();
            entry.skipped = true;
            job.report.AddSkipped( PipelineSource, job.assetPath, "the run was cancelled before the asset was converted" );
            job.activity.assetPath = job.assetPath;
            job.activity.action = ACTIVITYACTION_SKIPPED;
            job.activity.reason = ACTIVITYREASON_CANCELLED;
            job.activity.detail.Clear();
            job.activity.durationInMs = 0;
            continue;
        }

        // the cache entry as soon as the asset is exported: a run that stops later resumes after it
        if ( !validateOnly && job.converted )
            SaveCacheEntry( job, job.report.FindEntry( job.assetPath ), job.cookAllowed && !setup.cookers.empty(), intermediateDir );

        ++finishedAssets;
        if ( GUCEF_NULL != observer )
        {
            observer->OnAssetFinished( finishedAssets, totalAssets, job.assetPath );
            const CConversionReportEntry* entry = job.report.FindEntry( job.assetPath );
            if ( GUCEF_NULL != entry )
                observer->OnAssetReport( *entry, finishedAssets, totalAssets );
        }
    }
    futures.clear();
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        report.Merge( jobs[ j ].report );
        if ( !jobs[ j ].activity.assetPath.IsNULLOrEmpty() )
            m_lastRun.entries.push_back( jobs[ j ].activity );
    }

    // asked to stop: no cooking; the assets that finished have their cache entries, the report says what did not run
    if ( cancelled )
    {
        if ( GUCEF_NULL != observer )
            observer->OnPhase( PIPELINEPHASE_REPORT );
        report.SetCancelled();
        if ( writesActivity )
            CImportActivityLog::AppendRun( CImportActivityLog::GetLogPath( intermediateDir ), m_lastRun );
        if ( writesReport )
        {
            report.SaveJson( reportPath );
            report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << " was cancelled after " << finishedAssets << " of " << totalAssets << " assets" );
        return false;
    }

    // validate-only: the findings are the result
    if ( validateOnly )
    {
        if ( GUCEF_NULL != observer )
            observer->OnPhase( PIPELINEPHASE_REPORT );
        if ( writesReport )
        {
            report.SaveJson( reportPath );
            report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << " validated: " << report.GetItemCount( REPORTITEM_ERROR ) << " errors, "
                   << report.GetItemCount( REPORTITEM_WARNING ) << " warnings in " << totalAssets << " assets" );
        return !report.HasFailures();
    }

    // the cooking, per profile; with more than one target each has its own folder
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_COOK );
    CookAssets( manifest, options, context, setup, jobs, filteredOut, report );

    // the cache entries of the assets that were converted or cooked without errors, now with their cooked files
    if ( GUCEF_NULL != observer )
        observer->OnPhase( PIPELINEPHASE_REPORT );
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        if ( jobs[ j ].converted || jobs[ j ].cookPending )
            SaveCacheEntry( jobs[ j ], report.FindEntry( jobs[ j ].assetPath ), false, intermediateDir );
    }

    if ( writesActivity )
        CImportActivityLog::AppendRun( CImportActivityLog::GetLogPath( intermediateDir ), m_lastRun );
    report.SaveJson( reportPath );
    report.SaveSummary( CVfsPaths::WithoutExtension( reportPath ) + ".txt" );
    UpdateIndex( manifest, options, context, &report, false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPipeline: Pack " << manifest.id << " done: " << report.GetAssetCount( ASSETSTATUS_CONVERTED ) << " converted, "
               << report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) << " approximated, " << report.GetAssetCount( ASSETSTATUS_CACHED ) << " cached, "
               << report.GetAssetCount( ASSETSTATUS_FAILED ) << " failed" );
    return !report.HasFailures();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
