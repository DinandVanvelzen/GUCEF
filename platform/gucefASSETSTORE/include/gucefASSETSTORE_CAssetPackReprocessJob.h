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

#ifndef GUCEF_ASSETSTORE_CASSETPACKREPROCESSJOB_H
#define GUCEF_ASSETSTORE_CASSETPACKREPROCESSJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#include "gucefASSETSTORE_CAssetReprocessRequest.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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

class CLocalAssetStoreService;

/**
 *  A reprocessing job of a pack of the primary store (the pipeline jobs of CAssetPipelineOperations::IsReprocess()), run by the
 *  in-process service on the pool task of the job, and the plan of one (reprocess_plan, read only). The steps of the request
 *  (CAssetReprocessRequest) run in their order:
 *
 *  - redownload: the original again from the store the pack came from (its store facts name the item and the format): the store
 *    has to be signed in (else the job ends with the status NOT_SIGNED_IN and nothing done), the size guard of the store plugin
 *    applies unless allowLarge, the download counts against the caps of the sync profile of the store and its space reserve, and a
 *    machine caller against maxMachineRedownloadsPerDay. The download goes to a staging folder of the location and is ingested
 *    with CAssetIngestRequest::verifyPackId: the same files are "verified identical" (nothing changes; missing or damaged original
 *    files are put back when repairOriginals), other files become a new pack and the old pack is kept as superseded (its store
 *    facts say by which). A pack that came from an upload or a folder has no store source and is not downloaded. With thenConvert
 *    a new pack gets its outputs (its ingest converts it as its keep policy says, else the intermediate step does).
 *  - intermediate, target: CAssetPackRegeneration, with the scope of the request resolved to paths of the pack (assets, asset ids,
 *    the statuses of the last report, the kinds of the catalogue).
 *  - rollback: the previous generation of the outputs becomes current again.
 *
 *  After a step that changed outputs the pack index of the pack is loaded again and the change journal says the pack changed.
 *  Every step writes an entry of the ingest history of the pack (origin "reprocess:<step>", the caller and the arguments in its
 *  policy): the audit. The result document has a summary for people, the counts before and after, the assets that improved or
 *  regressed, the path of the report and whether a rollback is possible.
 */
class GUCEF_ASSETSTORE_PRIVATE_CPP CAssetPackReprocessJob
{
    public:

    CAssetPackReprocessJob( CLocalAssetStoreService& service, const CORE::CString& jobId, const CAssetReprocessRequest& request,
                            CIAssetConversionObserver* observer );

    ~CAssetPackReprocessJob();

    /** Runs the steps; the status of the job and its result document */
    TAssetStoreStatus Run( CAssetServiceDocument& result );

    /** What the request would do now, without doing anything */
    TAssetStoreStatus Plan( CAssetServiceDocument& result );

    private:

    TAssetStoreStatus ResolveScope( const CORE::CString& packId, CORE::CString::StringVector& globs, CORE::CDataNode& scope, CORE::CString& message );
    TAssetStoreStatus Redownload( const CAssetPackRecord& pack, CORE::CDataNode& node, CORE::CString& newPackId, bool& converted );
    TAssetStoreStatus FindStoreSource( const CAssetPackRecord& pack, CORE::CString& storeId, CORE::CString& itemId, CORE::CString& formatId,
                                       CORE::CString& outcome, CORE::CString& message );
    TAssetStoreStatus Regenerate( const CORE::CString& packId, bool target, const CORE::CString::StringVector& globs, bool cook, CORE::CDataNode& node );
    TAssetStoreStatus RollbackOutputs( const CORE::CString& packId, CORE::CDataNode& node );
    void ReloadIndex( const CORE::CString& packId );
    void WriteHistory( const CORE::CString& packId, const CORE::CString& step, const CORE::CString& outcome, const CORE::CString& problems, Int64 startedAtMs );
    void Progress( UInt32 done, UInt32 total, const CORE::CString& text );
    bool IsCancelled( void ) const;
    bool IsStoreSyncRunning( const CORE::CString& storeId ) const;
    CORE::CDataNode MakeMade( const CORE::CString& step ) const;
    UInt64 ReadFreeBytes( const CAssetPackRecord& pack, bool& known ) const;

    CAssetPackReprocessJob( const CAssetPackReprocessJob& src );              /**< not implemented */
    CAssetPackReprocessJob& operator=( const CAssetPackReprocessJob& src );   /**< not implemented */

    CLocalAssetStoreService& m_service;
    CORE::CString m_jobId;
    CAssetReprocessRequest m_request;
    CIAssetConversionObserver* m_observer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKREPROCESSJOB_H ? */
