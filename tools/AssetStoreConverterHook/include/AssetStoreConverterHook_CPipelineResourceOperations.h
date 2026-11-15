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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H

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
 *  The operation read_resource: the resources of section 8 of the asset tools design that the converter serves, by
 *  their URI (argument "uri"): assettools://docs/manifest-reference, docs/import-settings/{importer},
 *  packs/{packId}/manifest, licence, report, report-summary, activity (the last 5 runs), plan (of the last convert_plan),
 *  packs/{packId}/assets/{assetPath}/report, sidecar and override, and packs/{packId}/cook/{profile}/references and
 *  sizes. The answer is { uri, mimeType, text }; a resource above maxResourceBytes is refused with too-large, an unknown
 *  one with not-found. Only the manifest of a pack that is not runnable is served: its other files may lie outside the
 *  roots. The licence record holds purchase details only with exposePurchaseDetails.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineResourceOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus ReadResource( CPipelineOperationCall& call, const CPipelinePlanStore& plans );

    /** The text of the manifest reference */
    static CORE::CString GetManifestReference( void );

    private:

    CPipelineResourceOperations( void );    /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H ? */
