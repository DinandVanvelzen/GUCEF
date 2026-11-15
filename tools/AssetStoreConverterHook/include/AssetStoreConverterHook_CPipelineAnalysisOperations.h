/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H
#include "AssetStoreConverterHook_CPipelinePlanStore.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The read-only operations of the pipeline that work out something about packs (sections 7.2 and 7.4 of the asset
 *  tools design): convert_plan (a dry run: what a conversion would do per asset and why, kept for the plan resource),
 *  compare_routes (the summaries of the same asset converted through other routes, from the sidecars; converts
 *  nothing), material_map_suggest (the source materials of a shader and a proposal of a property mapping) and
 *  manifest_propose (a manifest for a source that has none yet). None of them writes a file.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineAnalysisOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus ConvertPlan( CPipelineOperationCall& call, CPipelinePlanStore& plans );
    static ASSETSTORE::TAssetStoreStatus CompareRoutes( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus MaterialMapSuggest( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus ManifestPropose( CPipelineOperationCall& call );

    private:

    CPipelineAnalysisOperations( void );    /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H ? */
