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

#ifndef GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#define GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#include "gucefASSETSTORE_CAssetConversionOutcome.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H
#include "gucefASSETSTORE_CAssetConversionRequest.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H ? */

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

class CPrimaryAssetStore;

/**
 *  The conversion of a pack that is in the store already (section 7.4: "conversion can also run later for packs marked
 *  conversion pending"), run on the calling thread, and the rules that it shares with the convert step of the ingest:
 *  what a hook's outcome does to the state of the pack.
 *
 *  Convert() takes the pack as it is placed, "<pack root>/original", has the converter hook convert it into
 *  the intermediate folder of the pack (CPackStorageLayout) and then, in one transaction where the backend has them, writes the converted
 *  paths and formats of the assets, the format facets, the conversion record, the state of the pack and an entry of
 *  the ingest history. A keep policy of "converted" removes the originals of the converted assets after that (and their
 *  file rows), and only when the conversion ended without errors. A pack that was converted before and now fails or is
 *  refused keeps its earlier conversion and state; the outcome and the history say what happened.
 *
 *  A cancel request on the future that the job belongs to ends the conversion with CANCELLED; nothing of the pack
 *  changes then.
 */
class GUCEF_ASSETSTORE_PRIVATE_CPP CAssetPackConversion
{
    public:

    explicit CAssetPackConversion( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel = GUCEF_NULL );

    virtual ~CAssetPackConversion();

    /** Converts a pack of the store; the outcome says how it went, the status is the one of the outcome */
    TAssetStoreStatus Convert( const CORE::CString& packId, CAssetConversionOutcome& outcome );

    /** Converts the packs that are "conversion pending", one after the other; the outcome holds the counts */
    TAssetStoreStatus ConvertPending( CAssetConversionOutcome& outcome );

    /**
     *  What the outcome of a hook does to a pack that is being converted: OK takes the converted assets over and records
     *  the conversion, REFUSED and FAILED leave the assets and put the pack in the state "conversion refused" or
     *  "conversion pending" with a reason, and say so in the problems. Returns OK when the outcome is taken in (the
     *  ingest goes on with the pack as original), and the status of the hook when the step has to fail (CANCELLED, or a
     *  conversion that could not run).
     */
    static TAssetStoreStatus ApplyOutcome( TAssetStoreStatus hookStatus, const CAssetConversionOutcome& hookOutcome ,
                                           const TAssetRecordVector& converted, CAssetPackRecord& pack              ,
                                           TAssetRecordVector& assets, CORE::CString::StringVector& problems         ,
                                           CORE::CString& message                                                    );

    /** The converted formats of the assets */
    static void CollectConvertedFormats( const TAssetRecordVector& assets, CORE::CString::StringSet& formats );

    /** Makes an id for a conversion job: "cv" and 19 hexadecimal characters */
    static CORE::CString MakeJobId( void );

    protected:

    /**
     *  The steps a conversion of a stored pack goes through, for a class that converts in another way (the regeneration of
     *  the outputs of a pack, CAssetPackRegeneration): OnPrepare() may change the request before the hook gets it (other
     *  folders, a scope, a cook only run, another observer), OnConverted() sees what the hook answered before the outcome
     *  is applied (and may change it), OnCommit() runs after the outcome is applied and before the catalogue is written
     *  (an error leaves the catalogue as it was), OnCommitted() after the catalogue write (persisted false: it failed).
     *  The defaults do nothing.
     */
    virtual TAssetStoreStatus OnPrepare( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CString& locationRoot,
                                         CAssetConversionRequest& request, CORE::CString& message );
    virtual TAssetStoreStatus OnConverted( const CAssetConversionRequest& request, TAssetStoreStatus& hookStatus, CAssetConversionOutcome& hookOutcome,
                                           const TAssetRecordVector& assets, TAssetRecordVector& converted, CORE::CString& message );
    virtual TAssetStoreStatus OnCommit( const CAssetConversionRequest& request, TAssetStoreStatus hookStatus, const CAssetPackRecord& before,
                                        const TAssetRecordVector& beforeAssets, CORE::CString& message );
    virtual void OnCommitted( bool persisted );

    /**
     *  Writes the state of the conversion that the records hold (converted paths and formats of the assets, the conversion
     *  record, the converted formats and the state of the pack) to the catalogue in one transaction, with an entry of the
     *  ingest history; for a rollback to an earlier generation of the outputs
     */
    TAssetStoreStatus RestoreState( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CAssetConversionOutcome& outcome,
                                    Int64 startedAtMs, CORE::CString& message );

    CPrimaryAssetStore& GetStore( void ) const;
    const CORE::CFutureResultSource* GetCancelSource( void ) const;

    CORE::CString m_jobId;                      /**< the id of the job of the history entries; empty: one is made */
    CORE::CString m_historyOrigin;              /**< the origin of the history entry, "conversion" by default */
    CORE::CString m_historyPolicy;              /**< added to the policy of the history entry */
    CIAssetConversionObserver* m_observer;      /**< the observer of the hook instead of the events of the store; not owned */
    bool m_notifyEvents;                        /**< the conversion events of the store are raised, true by default */
    bool m_removeOriginals;                     /**< a keep policy of "converted" removes the originals of what converted, true by default */

    private:

    TAssetStoreStatus Persist( const CAssetPackRecord& pack, const TAssetRecordVector& assets, bool assetsChanged   ,
                               const CORE::CString::StringSet& removedOriginals, const CAssetConversionOutcome& outcome ,
                               Int64 startedAtMs, CORE::CString& message                                              );
    bool IsCancelRequested( void ) const;

    CAssetPackConversion( const CAssetPackConversion& src );              /**< not implemented */
    CAssetPackConversion& operator=( const CAssetPackConversion& src );   /**< not implemented */

    private:

    CPrimaryAssetStore& m_store;
    const CORE::CFutureResultSource* m_cancel;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H ? */
