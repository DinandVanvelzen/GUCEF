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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#include "gucefASSETSTORE_CAssetPackSourceRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#include "gucefASSETSTORE_CAssetSyncMark.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H
#include "gucefASSETSTORE_CAssetSyncItemRecord.h"
#define GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlan.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H ? */

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
class CAssetVocabulary;
class CAssetSyncMarkStore;

/**
 *  The diff of the library sync (library sync design, section 4): the listing of a store, reduced item by item to a small
 *  record as the pages come in, against the packs of the store, their store facts, the marks of the owner, the observations
 *  of earlier runs and the downloads in flight. BuildPlan() gives every listed item one state (section 4.1, first match
 *  wins: ignored, no id, filtered, unavailable, in flight, pinned, current, unknown freshness, stale, failed before,
 *  missing), the counts and the ordered work list of the downloads: one per format of every missing or stale item (the
 *  decision of the owner: every format is downloaded).
 *
 *  The planner reads the catalogue with one query per table and store (LoadStoreState()) and joins in memory; it writes
 *  nothing but the observations and, for a run, the facts of adopted packs (WriteObservations()). It knows no store: the
 *  same code serves every plugin. The plan is a pure function of its inputs, so the same inputs give the same plan in
 *  the same order. Not thread safe: one job uses it on its thread.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncPlanner
{
    public:

    /** One download of the work list: a format of an item with the revision the plan saw */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SWorkItem
    {
        CORE::CString itemId;
        CORE::CString title;
        CORE::CString formatId;
        CORE::CString expectedRevision;
        UInt64 estimatedBytes;
        bool stale;
        Int64 changedAtMs;
        Int64 acquiredAtMs;
        UInt32 listIndex;

        SWorkItem( void );
    };
    typedef GUCEF::vector< SWorkItem > TWorkItemVector;

    CAssetLibrarySyncPlanner( void );
    ~CAssetLibrarySyncPlanner();

    /** Forgets everything and takes the spec of a new plan */
    void Reset( const CAssetLibrarySyncSpec& spec );

    /** The vocabulary that classifies the licences of the items (may be GUCEF_NULL: every licence is "unknown"); not owned */
    void SetVocabulary( const CAssetVocabulary* vocabulary );

    /** Reduces the items of a page to the records of the plan; an item seen before is skipped. Answers the number of new items */
    UInt32 AddListingPage( const CAssetStoreLibraryPage& page );

    /** Whether every page of the listing was read and the last one ended it */
    void SetListingComplete( bool complete );

    UInt32 GetListedCount( void ) const;
    UInt32 GetPageCount( void ) const;

    /** Reads the packs of the store, their store facts, the marks and the observations: one query per table */
    TAssetStoreStatus LoadStoreState( CPrimaryAssetStore& store );

    /** The downloads of the manager; those of the store that have not ended are in flight */
    void SetDownloadJobs( const TAssetStoreDownloadJobVector& jobs );

    /** The items whose listing is not enough to plan them: no formats, or a listing fingerprint that changed (GetItem decides) */
    void GetEnrichmentCandidates( CORE::CString::StringVector& itemIds ) const;

    /** The details of an item from GetItem(): its formats, sizes and revision replace those of the listing */
    void ApplyEnrichment( const CAssetStoreItem& item );

    /** The plan of the listing and the downloads of a run in the order of the profile */
    void BuildPlan( Int64 nowMs, CAssetLibrarySyncPlan& plan, TWorkItemVector& worklist );

    /**
     *  Writes what this listing showed of every item (sync_items). 'adopt' is set for a run (not for a plan): the counters
     *  of the items a retry of the failed ones takes up again restart, and when the profile adopts, the packs from before
     *  the facts were kept that the plan found unknown in freshness get their store facts. Call after BuildPlan()
     */
    TAssetStoreStatus WriteObservations( CAssetSyncMarkStore& syncStore, Int64 nowMs, bool adopt );

    /** The bookkeeping of a failed download of an item: attempts, the time of the next one (doubling from the profile's backoff, at most a day), the status */
    static void NoteFailedAttempt( CAssetSyncItemRecord& record, TAssetStoreStatus status, const CORE::CString& message, Int64 nowMs, const CAssetLibrarySyncProfile& profile );

    /** The bookkeeping of a download of an item that ended in a pack (or a duplicate of one): the counters restart */
    static void NoteSucceededAttempt( CAssetSyncItemRecord& record, TAssetStoreStatus status, Int64 nowMs );

    private:

    struct SFormat
    {
        CORE::CString formatId;
        UInt64 bytes;
        bool sizeKnown;
    };
    typedef GUCEF::vector< SFormat > TFormatVector;

    struct SListed
    {
        CORE::CString itemId;
        CORE::CString title;
        CORE::CString listingType;
        CORE::CString licenceClass;
        bool downloadable;
        CORE::CString unavailableReason;
        CORE::CString revision;
        Int64 changedAtMs;
        Int64 acquiredAtMs;
        CORE::CString listingFingerprint;
        CORE::CString formatFingerprint;
        UInt8 formatsLevel;
        UInt32 listIndex;
        UInt32 allFormatCount;
        bool enriched;
        TFormatVector formats;
    };
    typedef GUCEF::vector< SListed > TListedVector;

    struct SPack
    {
        CORE::CString packId;
        CORE::CString itemId;
        TAssetPackState state;
        Int64 ingestedMs;
    };
    typedef GUCEF::vector< SPack > TPackVector;

    void Reduce( const CAssetStoreItem& item, SListed& listed ) const;
    CORE::CString ClassifyLicence( const CAssetStoreItem& item ) const;
    void PlanItem( const SListed& listed, Int64 nowMs, UInt64 medianBytes, CAssetLibrarySyncItem& line, CORE::CString& skipReason );
    static CORE::CString Key( const CORE::CString& itemId, const CORE::CString& formatId );

    CAssetLibrarySyncPlanner( const CAssetLibrarySyncPlanner& src );              /**< not implemented */
    CAssetLibrarySyncPlanner& operator=( const CAssetLibrarySyncPlanner& src );   /**< not implemented */

    private:

    CAssetLibrarySyncSpec m_spec;
    const CAssetVocabulary* m_vocabulary;
    TListedVector m_listed;
    GUCEF::map< CORE::CString, size_t > m_listedIndex;
    UInt32 m_pages;
    bool m_listingComplete;
    TPackVector m_packs;
    GUCEF::map< CORE::CString, CAssetPackSourceRecord > m_facts;    /**< by pack id */
    GUCEF::map< CORE::CString, CAssetSyncMark > m_marks;             /**< by item id */
    TAssetSyncItemRecordMap m_observations;                          /**< by item id */
    CORE::CString::StringSet m_inFlight;                             /**< item and format keys, and item ids with a pack that is being ingested */
    CORE::CString::StringSet m_adoptable;                            /**< pack ids without facts that the plan found unknown in freshness */
    CORE::CString::StringSet m_removedUpstream;                      /**< item ids */
    CORE::CString::StringSet m_retried;                              /**< item ids whose failures a retry resets */
    CORE::CString::StringSet m_selected;                             /**< the item ids of the spec, empty for all */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLANNER_H ? */
