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

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#define ASSETCONVERTERLIB_CASSETPIPELINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#include "AssetConverterLib_CAssetConversionJob.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONJOB_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#include "AssetConverterLib_CAssetPackPlan.h"
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#include "AssetConverterLib_CImportActivityLog.h"
#define ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The pipeline of a pack (RunPack()):
 *
 *  1. The licence record of the manifest is read and checked (CLicenceRecord): without a record, or with one that does
 *     not allow the conversion, the pack is refused and nothing is converted.
 *  2. The source is opened (CAssetPackContext); the importers index the pack (IndexPack()); every file an importer
 *     can import is an asset (with "auto" the first importer by name that can).
 *  3. Each asset is a task on the thread pool "AssetConversion" (ConvertAssetAsync()); the calling thread blocks on
 *     their futures, and does tasks itself when the pool has no threads. Per asset (ConvertAsset()): the effective
 *     settings, the cache check (the key of CAssetCacheEntry; an unchanged asset keeps the files of the last run),
 *     import, normalisation, the post-processor stack, the baking of the textures, the validators, the export and
 *     the sidecar.
 *  4. The cookers of the run cook the converted assets whose validation has no errors (or all with force), once per
 *     cook profile, into <output>/<profile> (<output>/<target>/<profile> with more than one target), and the
 *     reference graph and size report of each profile are written there (references.json, sizes.json).
 *  5. The cache entries, the activity log (<intermediate>/activity.json) and the report are written. The cache entry of
 *     an asset is first written as soon as its export finished (marked cook pending when cookers run), so a run that
 *     is cancelled or interrupted leaves what it finished for the next run, which then converts only the rest.
 *
 *  A run that writes holds the lock of the intermediate (CIntermediateLock) from start to end: a second run of the
 *  same intermediate, in this process or another, is refused with the holder in the report (IsLocked()), and writes
 *  nothing. The options choose the mode (full, validate-only, cook-only), restrict the run to some assets (asset
 *  globs) and give the observer that follows the run and can stop it.
 *
 *  PlanPack() is the dry run: it decides per asset what RunPack() with the same manifest and options would do and
 *  why, with the effective settings and the importer, and writes nothing.
 *
 *  Everything is read and written through the VFS. RunPack() and PlanPack() are synchronous: they return when the pack
 *  is done; RunPack() uses the thread pool of the options for the assets.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPipeline
{
    public:

    CAssetPipeline( void );

    ~CAssetPipeline();

    /** True when every asset converted (and cooked) without errors; the report says what happened */
    bool RunPack( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CConversionReport& report );

    /**
     *  What RunPack() would do with the same manifest and options, per asset and why, without writing anything.
     *  @return false when the source of the pack cannot be opened (the plan says so); a refused pack or missing
     *          plugins still give a plan, with the decision and the problems
     */
    bool PlanPack( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CAssetPackPlan& plan );

    /** The activity of the last run */
    const CImportActivityLog::SRun& GetLastRun( void ) const;

    /** Converts the asset of the job on the calling thread; the results go into the job */
    static bool ConvertAsset( CAssetConversionJob& job );

    /** Converts the asset of the job as a task on the pool; the future gives the job with its results (TryGetResult()) */
    static CORE::CFutureResult ConvertAssetAsync( const CAssetConversionJob& job, const CORE::CString& threadPoolName );

    /** The job of a finished task of ConvertAssetAsync(); it lives as long as the future */
    static bool TryGetResult( const CORE::CFutureResult& future, CAssetConversionJob*& job );

    /** The current version of a plugin of a cache key ("importer:fbx", "cooker:ogre", "converter:AssetConverterLib", ...) */
    static bool GetCurrentPluginVersion( const CORE::CString& pluginKey, UInt32& version );

    /**
     *  The cache comparison of an asset without importing it: the effective settings (with their layers), the parts of
     *  the key known before the import, and the stored entry (when there is one and the job is no reimport) compared
     *  with the current plugin versions and dependency hashes, and the existence of the files it lists.
     *  @param reason   ACTIVITYREASON_UNCHANGED when the stored entry stands, else the first part that differs (as the
     *                  activity log of a run says it), with 'detail'
     *  @param sourceProblem    set when the source cannot be read (the asset cannot be converted)
     *  @return true when the stored entry stands
     */
    static bool EvaluateCache( const CAssetConversionJob& job        ,
                               CORE::CDataNode& settings             ,
                               CORE::CString::StringVector& layers   ,
                               CAssetCacheEntry& current             ,
                               CAssetCacheEntry& stored              ,
                               bool& hasStored                       ,
                               TActivityReason& reason               ,
                               CORE::CString& detail                 ,
                               CORE::CString& sourceProblem          );

    private:

    CAssetPipeline( const CAssetPipeline& src );            /**< not implemented */
    CAssetPipeline& operator=( const CAssetPipeline& src ); /**< not implemented */

    void Refuse( const CORE::CString& reason, const CAssetPipelineOptions& options, CAssetPackContext& context,
                 const GUCEF::vector< TAssetImporterPtr >& importers, CConversionReport& report );

    /** RunPack() once the lock of the intermediate is held (or not needed) */
    bool RunPackImpl( const CAssetPackManifest& manifest, const CAssetPipelineOptions& options, CConversionReport& report );

    private:

    CImportActivityLog::SRun m_lastRun;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */
