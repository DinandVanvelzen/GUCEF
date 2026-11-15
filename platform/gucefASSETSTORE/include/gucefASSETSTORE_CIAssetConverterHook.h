/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H
#include "gucefASSETSTORE_CAssetConversionRequest.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#include "gucefASSETSTORE_CAssetConversionOutcome.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The conversion step of the ingest pipeline (sections 7.4 and 8, step 5) and of the conversion of packs that are in
 *  the store already, implemented by the converter pipeline (AssetConverterLib) with the cook profile of the primary
 *  store in a module beside it (the store does not depend on the converter). Without a hook, or for packs without
 *  assets it can convert, a pack whose keep policy asks for converted content is ingested as original and marked
 *  "conversion pending".
 *
 *  ConvertPack() runs on the thread of the job (the ingest, or the pool task of ConvertPackAsync()) and blocks until
 *  the conversion is done; the observer of the request tells it when to stop and takes its progress. It reads the
 *  original files below request.sourceVfsPath, writes below request.convertedVfsFolder and sets convertedPath (see
 *  CAssetConversionRequest) and convertedFormat of the assets it converted; it leaves the other assets alone. What it
 *  returns:
 *
 *  - OK: the conversion ran and its report has no errors; 'outcome.record' says what it did. A keep policy of
 *    "converted" then lets the originals of the converted assets go.
 *  - REFUSED: the converter will not convert this pack (no licence record, a licence that does not allow it, ...);
 *    'outcome.message' says why. No asset is touched. Not an error: the pack stays as original.
 *  - FAILED: the conversion ran and its report has errors; no asset is touched, what it wrote is taken away. The pack
 *    stays as original.
 *  - CANCELLED: the observer asked to stop.
 *  - another status: the conversion could not run (a folder cannot be made, the storage is gone); the step fails and
 *    can be resumed.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetConverterHook
{
    public:

    CIAssetConverterHook( void );
    virtual ~CIAssetConverterHook();

    virtual CORE::CString GetName( void ) const = 0;

    /** The folder name of the cook profile below "converted/" */
    virtual CORE::CString GetProfileName( void ) const = 0;

    /** Whether the converter has something to do with the asset (a model, a scene or a prefab it has an importer for) */
    virtual bool CanConvert( const CAssetRecord& asset ) const = 0;

    virtual TAssetStoreStatus ConvertPack( const CAssetConversionRequest& request ,
                                           TAssetRecordVector& assets             ,
                                           CAssetConversionOutcome& outcome       ) = 0;

    /**
     *  Whether the conversion that the record describes is out of date: a part of the converter that made it has
     *  another version now, or is gone. 'reason' says which.
     */
    virtual bool IsConversionStale( const CAssetConversionRecord& recorded, CORE::CString& reason ) const = 0;

    /**
     *  Whether the hook carries out the operation of the converter pipeline (a name of CAssetPipelineOperations). False by
     *  default: a hook that only converts the packs of the store has none of them
     */
    virtual bool SupportsPipelineOperation( const CORE::CString& operation ) const;

    /**
     *  An operation of the converter pipeline for the tools of the asset store service (CIAssetStoreService::RunPipelineQuery(),
     *  ApplyPipelineChange(), StartPipelineJob()): the inspection of converter packs, their reports, sidecars and activity, a
     *  plan, a conversion, a cook or a validation run, the comparison of routes, and the authoring of manifests and overrides
     *  with a preview (request.apply false: the change token and the diff in the result) and an apply. What an operation
     *  takes beyond the members of the request is in request.arguments, what it answers is result.document; the members of
     *  both are those of the tool of the operation.
     *
     *  Runs on the calling thread and blocks until it is done: a job (CAssetPipelineOperations::IsJob()) runs on the pool
     *  task of the service, takes cancellation from the observer and reports its progress to it (the observer may be
     *  GUCEF_NULL for an operation that is not a job). ASSETSTORE_STATUS_UNSUPPORTED by default, with a message that says
     *  the converter has no such operation
     */
    virtual TAssetStoreStatus RunPipelineOperation( const CAssetPipelineRequest& request    ,
                                                    CIAssetConversionObserver* observer     ,
                                                    CAssetServiceDocument& result           );

    private:

    CIAssetConverterHook( const CIAssetConverterHook& src );              /**< not implemented */
    CIAssetConverterHook& operator=( const CIAssetConverterHook& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */
