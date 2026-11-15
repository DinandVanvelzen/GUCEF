/*
 *  AssetConverterLib: conversion of asset packs into an engine-neutral intermediate and its cooking
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

#ifndef ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H
#define ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H

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

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

class CConversionReportEntry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a run of the pipeline (CAssetPipeline::RunPack()) tells the one that started it, and what it asks: how many
 *  assets there are to convert and how many are done, and whether the run should stop. The calls are made on the thread
 *  that called RunPack(), between the assets that its tasks finish, never from a task of the pool.
 *
 *  A run that is asked to stop lets the assets that are being converted finish, does not start the ones that are
 *  waiting, and skips the cooking: the report says that it was cancelled and lists the assets that did not run as
 *  skipped (reason "cancelled"). The cache entries of the assets that finished are written as each finishes, so a
 *  next run of the pack converts only the rest (and cooks what the cancelled run did not cook).
 *
 *  IsCancelRequested() and OnAssetFinished() must be implemented; the other calls are optional (they do nothing by
 *  default): the number of assets once the pack is indexed, the phase of the run, and the report entry of every
 *  asset that finished (for counts per outcome).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetPipelineObserver
{
    public:

    CIAssetPipelineObserver( void );
    virtual ~CIAssetPipelineObserver();

    /** Whether the run should stop; asked often, so keep it cheap */
    virtual bool IsCancelRequested( void ) const = 0;

    /** The asset (a path in the pack) finished, 'done' of 'total' */
    virtual void OnAssetFinished( UInt32 done, UInt32 total, const CORE::CString& assetPath ) = 0;

    /** The pack is indexed: the number of assets this run converts (after the asset filter) */
    virtual void OnPackIndexed( UInt32 assetCount );

    /** The run enters a phase */
    virtual void OnPhase( TPipelinePhase phase );

    /** Right after OnAssetFinished(): the report entry of the asset, with its status and findings */
    virtual void OnAssetReport( const CConversionReportEntry& entry, UInt32 done, UInt32 total );

    private:

    CIAssetPipelineObserver( const CIAssetPipelineObserver& src );              /**< not implemented */
    CIAssetPipelineObserver& operator=( const CIAssetPipelineObserver& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETPIPELINEOBSERVER_H ? */
