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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

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
 *  The operations of the pipeline that run it, the jobs of the tools (section 6.3 of the asset tools design):
 *  convert_pack, convert_file (a scratch pack below the first work root), cook_pack (cook only, from the intermediate)
 *  and validate_pack (validate only: nothing is written). Each runs CAssetPipeline::RunPack() on the calling thread, the
 *  thread of the job, with the observer of the call for progress and cancellation, after the checks that refuse a run
 *  before it starts: the folders of the manifest (path-refused), the licence record (licence-refused) and the lock of
 *  the intermediate (busy, with its holder). A cancelled run answers CANCELLED; the cache entries of the assets that
 *  were done make the next run resume. A run whose assets failed is not a failed job: its document groups the failures.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineRunOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus ConvertPack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads );
    static ASSETSTORE::TAssetStoreStatus ConvertFile( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads );
    static ASSETSTORE::TAssetStoreStatus CookPack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads );
    static ASSETSTORE::TAssetStoreStatus ValidatePack( CPipelineOperationCall& call, const CORE::CString& threadPoolName, UInt32 threads );

    /** The message of a finding with the paths, numbers and quoted names replaced by placeholders, to group findings */
    static CORE::CString MakePattern( const CORE::CString& message );

    private:

    CPipelineRunOperations( void );     /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H ? */
