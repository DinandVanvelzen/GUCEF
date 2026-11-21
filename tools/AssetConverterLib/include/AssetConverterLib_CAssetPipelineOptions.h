/*
 *  AssetConverterLib: the options of a run of the pipeline
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

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H
#include "AssetConverterLib_CIAssetPipelineObserver.h"
#define ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H
#endif /* ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H ? */


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
 *  The options of a run, which the command line sets: whether validation errors still let assets be cooked (force),
 *  whether the cache is ignored (reimport), the thread pool and its number of workers (0: one per logical CPU), cook
 *  targets and an output folder instead of those of the manifest, the VFS path of the report (default
 *  <intermediate>/report.json, with the readable summary next to it as .txt) and whether the activity log is written.
 *  An observer (not owned, may be GUCEF_NULL) is told how far the run is and can stop it.
 *
 *  The mode (default full) makes a run validate only (nothing written but the report when reportPath is given) or cook
 *  only (from the intermediate of an earlier run). The asset globs restrict this run to the assets whose path in the
 *  pack matches one of them, without changing the manifest; the cache entries, references and sizes of the other
 *  assets stay as they were.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPipelineOptions
{
    public:

    bool force;
    bool reimport;
    UInt32 threads;
    CORE::CString threadPoolName;
    CORE::CString::StringVector cookTargets;
    CORE::CString outputDir;
    CORE::CString reportPath;
    bool writeActivityLog;
    CIAssetPipelineObserver* observer;
    TPipelineMode mode;
    CORE::CString::StringVector assetGlobs;

    // the pack index (design sections 4 and 5): done once, redone on request
    bool noIndex;           /**< do not write the pack index */
    bool indexOnly;         /**< build the pack index and stop: no conversion, no cooking */
    bool reindex;           /**< build the structure and the grouping of the pack index again */
    bool analyse;           /**< add the analysis level (the headers of the pictures) to the pack index; reading each file of an archive can be slow, so a run does not do it unless asked */
    bool reanalyse;         /**< read the files again for the statistics of the pack index */
    bool rethumbnail;       /**< render the thumbnails of the pack index again (the thumbnail step belongs to the viewer: the flag is carried) */
    bool retryFailed;       /**< analyse again the resources that failed before */
    bool noCook;            /**< convert to the intermediate and stop there: no cooker runs, whatever the manifest names as cook targets */
    CORE::CString indexFolder;  /**< where the pack index is written; the intermediate folder of the pack when empty */

    CAssetPipelineOptions( void );

    /** Whether the asset is part of this run: no globs, or one of them matches its path in the pack */
    bool IsInAssetFilter( const CORE::CString& assetPath ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */
