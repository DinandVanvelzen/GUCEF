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


#ifndef ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H
#include "gucefASSETSTORE_CAssetPackIndexRequest.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#include "gucefASSETSTORE_CAssetPackIndexOutcome.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

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

class CPipelineConverterHook;

/**
 *  The hook operation BuildPackIndex of the pack index design (section 2.11): the pack index of a pack of the store, made
 *  with AssetConverterLib, which the store cannot link.
 *
 *  Build() runs the pipeline of the converter with "index only" (CAssetPipeline::RunPack() with indexOnly) on the original
 *  files of the pack and the intermediate folder of the pack: the gate of the converter decides whether the stored document is
 *  fresh (nothing is built, the outcome says reused), stale or missing (the structure level is built, with the analysis level
 *  when the request asks), under the lock of the index so that two writers never write one index. The licence record of the
 *  pack goes into the intermediate folder first (as a conversion does), so the flags of the licence are in the document. The
 *  document is metadata: a pack whose licence the converter would refuse is indexed all the same.
 *
 *  Derive() makes the document from what the catalogue knows of the pack (the assets of the request: names, paths, formats,
 *  sizes, digests, typed properties, conversion) and reads no file of the pack, for packs that were ingested before the index
 *  existed and for packs whose originals are gone. Grouping comes from the paths only; the analysis level is marked as the
 *  catalogue has it, and the producer says "derivedFromCatalogue". A later build replaces it.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPackIndexOperations
{
    public:

    /** Builds, or finds fresh, the document of the request, or derives it when the request says so; the outcome says which */
    static ASSETSTORE::TAssetStoreStatus Build( const CPipelineConverterHook& hook, const ASSETSTORE::CAssetPackIndexRequest& request, ASSETSTORE::CAssetPackIndexOutcome& outcome );

    /** Makes the document from the assets of the request */
    static ASSETSTORE::TAssetStoreStatus Derive( const ASSETSTORE::CAssetPackIndexRequest& request, ASSETSTORE::CAssetPackIndexOutcome& outcome );

    private:

    CPackIndexOperations( void );     /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPACKINDEXOPERATIONS_H ? */
