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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#include "gucefASSETSTORE_CAssetItemRevision.h"
#define GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#endif /* GUCEF_ASSETSTORE_CASSETITEMREVISION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#include "gucefASSETSTORE_CAssetSyncMarkStore.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncPlanner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int64 DayMs = 24 * 60 * 60 * 1000;

/** The order of the work list: the items of the plan by the order of the profile, ties by the item id */
class CPlanOrder
{
    public:

    CPlanOrder( const TAssetLibrarySyncItemVector& items, const CORE::CString& order ) : m_items( items ), m_order( order ) {}

    bool operator()( size_t left, size_t right ) const
    {
        const CAssetLibrarySyncItem& a = m_items[ left ];
        const CAssetLibrarySyncItem& b = m_items[ right ];
        if ( CAssetLibrarySyncProfile::OrderNewest == m_order || CAssetLibrarySyncProfile::OrderOldest == m_order )
        {
            const bool newest = CAssetLibrarySyncProfile::OrderNewest == m_order;
            if ( a.changedAtMs != b.changedAtMs )
                return newest ? a.changedAtMs > b.changedAtMs : a.changedAtMs < b.changedAtMs;
            if ( a.acquiredAtMs != b.acquiredAtMs )
                return newest ? a.acquiredAtMs > b.acquiredAtMs : a.acquiredAtMs < b.acquiredAtMs;
        }
        else
        if ( CAssetLibrarySyncProfile::OrderSmallest == m_order )
        {
            if ( a.estimatedBytes != b.estimatedBytes )
                return a.estimatedBytes < b.estimatedBytes;
        }
        if ( a.listIndex != b.listIndex )
            return a.listIndex < b.listIndex;
        return a.itemId < b.itemId;
    }

    private:

    const TAssetLibrarySyncItemVector& m_items;
    CORE::CString m_order;
};

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncPlanner::SWorkItem::SWorkItem( void )
    : itemId()
    , title()
    , formatId()
    , expectedRevision()
    , estimatedBytes( 0 )
    , stale( false )
    , changedAtMs( 0 )
    , acquiredAtMs( 0 )
    , listIndex( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncPlanner::CAssetLibrarySyncPlanner( void )
    : m_spec()
    , m_vocabulary( GUCEF_NULL )
    , m_listed()
    , m_listedIndex()
    , m_pages( 0 )
    , m_listingComplete( false )
    , m_packs()
    , m_facts()
    , m_marks()
    , m_observations()
    , m_inFlight()
    , m_adoptable()
    , m_removedUpstream()
    , m_retried()
    , m_selected()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncPlanner::~CAssetLibrarySyncPlanner()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::Reset( const CAssetLibrarySyncSpec& spec )
{GUCEF_TRACE;

    m_spec = spec;
    m_listed.clear();
    m_listedIndex.clear();
    m_pages = 0;
    m_listingComplete = false;
    m_packs.clear();
    m_facts.clear();
    m_marks.clear();
    m_observations.clear();
    m_inFlight.clear();
    m_adoptable.clear();
    m_removedUpstream.clear();
    m_retried.clear();
    m_selected.clear();
    for ( size_t i=0; i<spec.itemIds.size(); ++i )
        m_selected.insert( spec.itemIds[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::SetVocabulary( const CAssetVocabulary* vocabulary )
{GUCEF_TRACE;

    m_vocabulary = vocabulary;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncPlanner::Key( const CORE::CString& itemId, const CORE::CString& formatId )
{GUCEF_TRACE;

    return itemId + "\n" + formatId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncPlanner::ClassifyLicence( const CAssetStoreItem& item ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_vocabulary )
        return "unknown";
    CAssetLicence licence;
    licence.name = item.licenceName;
    licence.url = item.licenceUrl;
    licence.text = item.licenceText;
    licence.restrictions = item.licenceRestrictions;
    m_vocabulary->ClassifyLicence( licence );
    return licence.licenceClass.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : licence.licenceClass;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::Reduce( const CAssetStoreItem& item, SListed& listed ) const
{GUCEF_TRACE;

    listed.itemId = item.itemId;
    listed.title = item.title;
    listed.listingType = item.listingType;
    listed.licenceClass = ClassifyLicence( item );
    listed.downloadable = item.downloadable;
    listed.unavailableReason = item.unavailableReason;
    listed.revision = CAssetItemRevision::GetEffectiveRevision( item );
    listed.changedAtMs = item.changedAtMs;
    listed.acquiredAtMs = item.acquiredAtMs;
    listed.listingFingerprint = CAssetItemRevision::ComputeListingFingerprint( item );
    listed.formatFingerprint = CAssetItemRevision::ComputeFormatFingerprint( item );
    listed.formatsLevel = item.GetEffectiveFormatsLevel();
    listed.allFormatCount = (UInt32) item.formats.size();
    listed.formats.clear();
    CORE::CString::StringVector ranked;
    m_spec.profile.formats.RankFormats( item, ranked );
    for ( size_t r=0; r<ranked.size(); ++r )
    {
        const CAssetStoreFormat* format = item.FindFormat( ranked[ r ] );
        if ( GUCEF_NULL == format )
            continue;
        SFormat entry;
        entry.formatId = format->formatId;
        entry.bytes = 0;
        entry.sizeKnown = !format->files.empty();
        for ( size_t f=0; f<format->files.size(); ++f )
        {
            entry.bytes += format->files[ f ].size;
            if ( 0 == format->files[ f ].size )
                entry.sizeKnown = false;
        }
        listed.formats.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncPlanner::AddListingPage( const CAssetStoreLibraryPage& page )
{GUCEF_TRACE;

    ++m_pages;
    UInt32 added = 0;
    for ( size_t i=0; i<page.items.size(); ++i )
    {
        const CAssetStoreItem& item = page.items[ i ];
        if ( !item.itemId.IsNULLOrEmpty() && m_listedIndex.end() != m_listedIndex.find( item.itemId ) )
            continue;
        SListed listed;
        Reduce( item, listed );
        listed.listIndex = (UInt32) m_listed.size();
        listed.enriched = false;
        if ( !item.itemId.IsNULLOrEmpty() )
            m_listedIndex[ item.itemId ] = m_listed.size();
        m_listed.push_back( listed );
        ++added;
    }
    return added;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::SetListingComplete( bool complete )
{GUCEF_TRACE;

    m_listingComplete = complete;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncPlanner::GetListedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_listed.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncPlanner::GetPageCount( void ) const
{GUCEF_TRACE;

    return m_pages;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncPlanner::LoadStoreState( CPrimaryAssetStore& store )
{GUCEF_TRACE;

    const CORE::CString storeId = m_spec.storeId.Lowercase();
    m_packs.clear();
    m_facts.clear();
    m_marks.clear();
    m_observations.clear();

    // the packs of the store, projected to what the diff needs
    CORE::CDataTableQuery query;
    query.Select( "pack_id" ).Select( "source_item_id" ).Select( "state" ).Select( "ingested_ms" );
    query.WhereEquals( "source_id", CORE::CVariant( storeId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = store.GetCatalogue().Select( "packs", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        SPack pack;
        pack.packId = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
        pack.itemId = CAssetCatalogue::GetText( rows[ r ], "source_item_id" );
        pack.state = AssetPackStateFromString( CAssetCatalogue::GetText( rows[ r ], "state" ) );
        pack.ingestedMs = CAssetCatalogue::GetInt( rows[ r ], "ingested_ms" );
        if ( !pack.itemId.IsNULLOrEmpty() )
            m_packs.push_back( pack );
    }

    CAssetSyncMarkStore& syncStore = store.GetSyncStore();
    if ( !syncStore.IsAvailable() )
        return ASSETSTORE_STATUS_OK;
    TAssetPackSourceRecordVector facts;
    status = syncStore.GetPackSourcesOfStore( storeId, facts );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t f=0; f<facts.size(); ++f )
        m_facts[ facts[ f ].packId ] = facts[ f ];
    TAssetSyncMarkVector marks;
    status = syncStore.GetMarks( storeId, marks );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t m=0; m<marks.size(); ++m )
        m_marks[ marks[ m ].itemId ] = marks[ m ];
    return syncStore.GetSyncItems( storeId, m_observations );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::SetDownloadJobs( const TAssetStoreDownloadJobVector& jobs )
{GUCEF_TRACE;

    const CORE::CString storeId = m_spec.storeId.Lowercase();
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        const CAssetStoreDownloadJob& job = jobs[ j ];
        if ( job.providerId != storeId || !job.forIngest )
            continue;
        if ( ASSETDOWNLOAD_STATE_DONE == job.state || ASSETDOWNLOAD_STATE_FAILED == job.state || ASSETDOWNLOAD_STATE_CANCELLED == job.state )
            continue;
        m_inFlight.insert( Key( job.itemId, job.formatId ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::GetEnrichmentCandidates( CORE::CString::StringVector& itemIds ) const
{GUCEF_TRACE;

    itemIds.clear();
    for ( size_t i=0; i<m_listed.size(); ++i )
    {
        const SListed& listed = m_listed[ i ];
        if ( listed.itemId.IsNULLOrEmpty() || listed.enriched || !listed.downloadable )
            continue;
        if ( !m_selected.empty() && m_selected.end() == m_selected.find( listed.itemId ) )
            continue;
        if ( listed.formats.empty() && 0 == listed.allFormatCount )
        {
            // the listing has no formats: only the details say what to download
            itemIds.push_back( listed.itemId );
            continue;
        }
        const CORE::CString basis = CAssetItemRevision::GetBasis( listed.revision );
        if ( ( CAssetItemRevision::FormatBasis == basis || CAssetItemRevision::ListingBasis == basis ) && listed.formatsLevel < 2 )
        {
            // the listing names the formats without their files (formatsLevel 1) and gives no time, version or revision: a
            // fingerprint of it would not change when the files do; the files, their sizes and the revision come from the
            // details (design 2.5)
            itemIds.push_back( listed.itemId );
            continue;
        }
        if ( CAssetItemRevision::ListingBasis != basis )
            continue;
        TAssetSyncItemRecordMap::const_iterator seen = m_observations.find( listed.itemId );
        if ( seen != m_observations.end() && !(*seen).second.listingFingerprint.IsNULLOrEmpty() && (*seen).second.listingFingerprint != listed.listingFingerprint )
            itemIds.push_back( listed.itemId );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::ApplyEnrichment( const CAssetStoreItem& item )
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, size_t >::const_iterator at = m_listedIndex.find( item.itemId );
    if ( at == m_listedIndex.end() )
        return;
    SListed& listed = m_listed[ (*at).second ];
    SListed details;
    Reduce( item, details );
    // a format fingerprint of a listing without files is of the format ids only: the details give the better revision
    const CORE::CString basis = CAssetItemRevision::GetBasis( listed.revision );
    const bool weakRevision = CAssetItemRevision::ListingBasis == basis || CAssetItemRevision::UnknownBasis == basis ||
                              ( CAssetItemRevision::FormatBasis == basis && listed.formatsLevel < 2 );
    listed.formats = details.formats;
    listed.allFormatCount = details.allFormatCount;
    listed.formatsLevel = details.formatsLevel;
    listed.formatFingerprint = details.formatFingerprint;
    if ( weakRevision )
        listed.revision = details.revision;
    if ( listed.changedAtMs <= 0 && details.changedAtMs > 0 )
        listed.changedAtMs = details.changedAtMs;
    listed.enriched = true;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::PlanItem( const SListed& listed, Int64 nowMs, UInt64 medianBytes, CAssetLibrarySyncItem& line, CORE::CString& skipReason )
{GUCEF_TRACE;

    const CAssetLibrarySyncProfile& profile = m_spec.profile;
    line.itemId = listed.itemId;
    line.title = listed.title;
    line.listedRevision = listed.revision;
    line.licenceClass = listed.licenceClass;
    line.changedAtMs = listed.changedAtMs;
    line.acquiredAtMs = listed.acquiredAtMs;
    line.listIndex = listed.listIndex;
    UInt64 allBytes = 0;
    bool allKnown = !listed.formats.empty();
    for ( size_t f=0; f<listed.formats.size(); ++f )
    {
        line.formats.push_back( listed.formats[ f ].formatId );
        allBytes += listed.formats[ f ].sizeKnown ? listed.formats[ f ].bytes : medianBytes;
        allKnown = allKnown && listed.formats[ f ].sizeKnown;
    }
    line.estimatedBytes = allBytes;
    line.sizeKnown = allKnown;

    // 1 ignored, 2 no id
    GUCEF::map< CORE::CString, CAssetSyncMark >::const_iterator mark = m_marks.find( listed.itemId );
    const bool hasMark = mark != m_marks.end();
    if ( hasMark && CAssetSyncMark::Ignore == (*mark).second.mark )
    {
        line.state = CAssetLibrarySyncItem::Ignored;
        line.reason = (*mark).second.reason.IsNULLOrEmpty() ? CORE::CString( "ignored" ) : (*mark).second.reason;
        skipReason = "ignored";
        return;
    }
    if ( listed.itemId.IsNULLOrEmpty() )
    {
        line.state = CAssetLibrarySyncItem::NoId;
        line.reason = "the listing gave no item id";
        skipReason = "no-item-id";
        return;
    }

    // 3 filtered
    CORE::CString filter;
    if ( !m_selected.empty() && m_selected.end() == m_selected.find( listed.itemId ) )
        filter = "not-selected";
    else
    if ( !profile.listingTypes.empty() && profile.listingTypes.end() == profile.listingTypes.find( listed.listingType ) )
        filter = "filter:kind";
    else
    if ( !profile.licenceClasses.empty() && profile.licenceClasses.end() == profile.licenceClasses.find( listed.licenceClass ) )
        filter = "filter:licence";
    else
    if ( listed.formats.empty() && listed.allFormatCount > 0 )
        filter = "no-preferred-format";
    else
    if ( !allKnown && profile.skipUnknownSize )
        filter = "filter:size";
    else
    if ( allKnown && ( ( profile.maxItemBytes > 0 && allBytes > profile.maxItemBytes ) || ( profile.minItemBytes > 0 && allBytes < profile.minItemBytes ) ) )
        filter = "filter:size";
    if ( !filter.IsNULLOrEmpty() )
    {
        line.state = CAssetLibrarySyncItem::Filtered;
        line.reason = filter;
        skipReason = filter;
        return;
    }

    // 4 unavailable
    if ( !listed.downloadable )
    {
        line.state = CAssetLibrarySyncItem::Unavailable;
        line.reason = listed.unavailableReason.IsNULLOrEmpty() ? CORE::CString( "the account cannot download it now" ) : listed.unavailableReason;
        skipReason = "unavailable";
        return;
    }
    if ( listed.formats.empty() )
    {
        line.state = CAssetLibrarySyncItem::Unavailable;
        line.reason = "the store lists no format of the item";
        skipReason = "no-format";
        return;
    }

    // the packs of the item that stand for it now
    GUCEF::vector< const SPack* > current;
    bool beingIngested = false;
    for ( size_t p=0; p<m_packs.size(); ++p )
    {
        const SPack& pack = m_packs[ p ];
        if ( pack.itemId != listed.itemId )
            continue;
        if ( ASSETPACK_STATE_STAGED == pack.state || ASSETPACK_STATE_INDEXING == pack.state )
            beingIngested = true;
        if ( !AssetPackStateIsIngested( pack.state ) )
            continue;
        GUCEF::map< CORE::CString, CAssetPackSourceRecord >::const_iterator fact = m_facts.find( pack.packId );
        if ( fact != m_facts.end() && !(*fact).second.IsCurrent() )
            continue;
        current.push_back( &pack );
        line.packIds.push_back( pack.packId );
    }

    // 5 in flight
    bool inFlight = beingIngested;
    for ( size_t f=0; f<listed.formats.size() && !inFlight; ++f )
        inFlight = m_inFlight.end() != m_inFlight.find( Key( listed.itemId, listed.formats[ f ].formatId ) );
    if ( inFlight )
    {
        line.state = CAssetLibrarySyncItem::InFlight;
        line.reason = beingIngested ? "being ingested" : "queued or downloading";
        return;
    }

    // 6 pinned
    if ( hasMark && CAssetSyncMark::Pin == (*mark).second.mark && !current.empty() )
    {
        line.state = CAssetLibrarySyncItem::Pinned;
        line.reason = (*mark).second.reason.IsNULLOrEmpty() ? CORE::CString( "pinned" ) : (*mark).second.reason;
        skipReason = "pinned";
        return;
    }

    // 7 to 9 per format: current, unknown freshness, stale; none is missing
    TAssetSyncItemRecordMap::const_iterator seen = m_observations.find( listed.itemId );
    const bool hasSeen = seen != m_observations.end();
    CORE::CString::StringVector missing;
    CORE::CString::StringVector stale;
    UInt32 unknown = 0;
    CORE::CString::StringSet adoptable;
    for ( size_t f=0; f<listed.formats.size(); ++f )
    {
        const CORE::CString& formatId = listed.formats[ f ].formatId;
        bool same = false;
        bool newer = false;
        bool notKnown = false;
        bool candidate = false;
        for ( size_t c=0; c<current.size(); ++c )
        {
            GUCEF::map< CORE::CString, CAssetPackSourceRecord >::const_iterator fact = m_facts.find( current[ c ]->packId );
            const bool hasFact = fact != m_facts.end();
            if ( hasFact && !(*fact).second.MatchesFormat( formatId ) )
                continue;
            candidate = true;
            CAssetItemRevision::TComparison comparison = CAssetItemRevision::COMPARISON_UNKNOWN;
            if ( hasFact )
            {
                line.storedRevision = (*fact).second.revision;
                comparison = CAssetItemRevision::Compare( (*fact).second.revision, listed.revision );
                if ( CAssetItemRevision::COMPARISON_CHECK_FURTHER == comparison )
                {
                    // the listing changed: only a change of the files makes it stale
                    comparison = CAssetItemRevision::COMPARISON_UNKNOWN;
                    if ( listed.enriched && hasSeen && !(*seen).second.formatFingerprint.IsNULLOrEmpty() )
                        comparison = (*seen).second.formatFingerprint == listed.formatFingerprint ? CAssetItemRevision::COMPARISON_SAME : CAssetItemRevision::COMPARISON_STALE;
                }
            }
            if ( CAssetItemRevision::COMPARISON_SAME == comparison )
                same = true;
            else
            if ( CAssetItemRevision::COMPARISON_STALE == comparison )
                newer = true;
            else
            {
                // adoption: a pack of another basis or without facts is taken as it is unless the store changed the item after its ingest
                if ( listed.changedAtMs > 0 && current[ c ]->ingestedMs > 0 && current[ c ]->ingestedMs < listed.changedAtMs )
                    newer = true;
                else
                {
                    notKnown = true;
                    if ( !hasFact )
                        adoptable.insert( current[ c ]->packId );
                }
            }
        }
        if ( !candidate )
            missing.push_back( formatId );
        else
        if ( same )
            continue;
        else
        if ( newer )
            stale.push_back( formatId );
        else
        if ( notKnown )
            ++unknown;
    }

    const bool reportStale = CAssetLibrarySyncProfile::OnStaleReport == profile.onStale;
    if ( !stale.empty() )
    {
        line.state = CAssetLibrarySyncItem::Stale;
        line.reason = "the store has a newer version";
        if ( !reportStale )
            line.queuedFormats = stale;
        for ( size_t m=0; m<missing.size(); ++m )
            line.queuedFormats.push_back( missing[ m ] );
        if ( reportStale )
            skipReason = "stale-reported";
    }
    else
    if ( !missing.empty() )
    {
        line.state = CAssetLibrarySyncItem::Missing;
        line.reason = current.empty() ? "no pack of the item" : "no pack of a format of the item";
        line.queuedFormats = missing;
    }
    else
    if ( unknown > 0 )
    {
        line.state = CAssetLibrarySyncItem::UnknownFreshness;
        line.reason = "a pack exists; whether it is the listed version cannot be told";
        for ( CORE::CString::StringSet::const_iterator a = adoptable.begin(); a != adoptable.end(); ++a )
            m_adoptable.insert( *a );
        return;
    }
    else
    {
        line.state = CAssetLibrarySyncItem::Current;
        return;
    }

    // a download that ended as a duplicate of another item's files for this revision is not repeated
    if ( CAssetLibrarySyncItem::Missing == line.state && hasSeen && CORE::CString( AssetStoreStatusToString( ASSETSTORE_STATUS_DUPLICATE ) ) == (*seen).second.lastStatus && (*seen).second.listedRevision == listed.revision )
    {
        line.state = CAssetLibrarySyncItem::Current;
        line.reason = "content-duplicate";
        line.queuedFormats.clear();
        return;
    }

    // 10 failed before
    if ( !line.queuedFormats.empty() && hasSeen && (*seen).second.attempts > 0 )
    {
        if ( m_spec.retryFailed )
        {
            m_retried.insert( listed.itemId );
        }
        else
        if ( (*seen).second.attempts >= profile.maxSyncAttempts )
        {
            line.state = CAssetLibrarySyncItem::FailedBefore;
            line.reason = "failed " + CORE::ToString( (*seen).second.attempts ) + " times: " + (*seen).second.lastError;
            line.queuedFormats.clear();
            skipReason = "failed-before";
            return;
        }
        else
        if ( (*seen).second.nextAttemptMs > nowMs )
        {
            line.state = CAssetLibrarySyncItem::FailedBefore;
            line.reason = "the next attempt is after a wait: " + (*seen).second.lastError;
            line.queuedFormats.clear();
            skipReason = "backoff";
            return;
        }
    }

    // the estimate of what is downloaded
    UInt64 queuedBytes = 0;
    bool queuedKnown = true;
    for ( size_t q=0; q<line.queuedFormats.size(); ++q )
        for ( size_t f=0; f<listed.formats.size(); ++f )
            if ( listed.formats[ f ].formatId == line.queuedFormats[ q ] )
            {
                queuedBytes += listed.formats[ f ].sizeKnown ? listed.formats[ f ].bytes : medianBytes;
                queuedKnown = queuedKnown && listed.formats[ f ].sizeKnown;
            }
    if ( !line.queuedFormats.empty() )
    {
        line.estimatedBytes = queuedBytes;
        line.sizeKnown = queuedKnown;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::BuildPlan( Int64 nowMs, CAssetLibrarySyncPlan& plan, TWorkItemVector& worklist )
{GUCEF_TRACE;

    plan.Clear();
    worklist.clear();
    m_adoptable.clear();
    m_removedUpstream.clear();
    m_retried.clear();
    plan.storeId = m_spec.storeId;
    plan.mode = m_spec.mode;
    plan.listed = (UInt32) m_listed.size();
    plan.pages = m_pages;

    // the median of the known sizes of a format download: the estimate of one whose size is not known
    GUCEF::vector< UInt64 > known;
    for ( size_t i=0; i<m_listed.size(); ++i )
        for ( size_t f=0; f<m_listed[ i ].formats.size(); ++f )
            if ( m_listed[ i ].formats[ f ].sizeKnown )
                known.push_back( m_listed[ i ].formats[ f ].bytes );
    UInt64 medianBytes = m_spec.profile.unknownSizeEstimateBytes;
    if ( !known.empty() )
    {
        std::sort( known.begin(), known.end() );
        medianBytes = known[ known.size() / 2 ];
    }

    // a listing is complete when every page was read and it is not much shorter than the last complete one
    UInt32 previouslySeen = 0;
    for ( TAssetSyncItemRecordMap::const_iterator o = m_observations.begin(); o != m_observations.end(); ++o )
        if ( (*o).second.lastSeenMs > 0 && (*o).second.unavailableReason != "removed-upstream" )
            ++previouslySeen;
    bool complete = m_listingComplete && m_selected.empty();
    if ( complete && previouslySeen >= 20 && (UInt64) m_listed.size() * 2 < (UInt64) previouslySeen )
        complete = false;
    plan.listingComplete = m_listingComplete;

    plan.items.resize( m_listed.size() );
    for ( size_t i=0; i<m_listed.size(); ++i )
    {
        CAssetLibrarySyncItem& line = plan.items[ i ];
        CORE::CString skipReason;
        PlanItem( m_listed[ i ], nowMs, medianBytes, line, skipReason );
        if ( "not-selected" == line.reason )
            continue;
        plan.AddCount( line.state );
        if ( !skipReason.IsNULLOrEmpty() )
            plan.AddSkipped( skipReason );
        if ( line.IsQueued() && "unknown" == line.licenceClass )
            plan.AddCount( "needs-licence-review" );
    }

    // the order of the work list, and the limit of the spec
    GUCEF::vector< size_t > order;
    for ( size_t i=0; i<plan.items.size(); ++i )
        if ( plan.items[ i ].IsQueued() )
            order.push_back( i );
    std::sort( order.begin(), order.end(), CPlanOrder( plan.items, m_spec.profile.order ) );
    for ( size_t o=0; o<order.size(); ++o )
    {
        CAssetLibrarySyncItem& line = plan.items[ order[ o ] ];
        if ( m_spec.maxItems > 0 && plan.queuedItems >= m_spec.maxItems )
        {
            line.queuedFormats.clear();
            line.reason = "max-items";
            plan.AddSkipped( "max-items" );
            continue;
        }
        const SListed& listed = m_listed[ m_listedIndex[ line.itemId ] ];
        ++plan.queuedItems;
        plan.estimatedBytes += line.estimatedBytes;
        if ( !line.sizeKnown )
            ++plan.sizeUnknownItems;
        for ( size_t q=0; q<line.queuedFormats.size(); ++q )
        {
            SWorkItem work;
            work.itemId = line.itemId;
            work.title = line.title;
            work.formatId = line.queuedFormats[ q ];
            work.expectedRevision = line.listedRevision;
            work.stale = CAssetLibrarySyncItem::Stale == line.state;
            work.changedAtMs = line.changedAtMs;
            work.acquiredAtMs = line.acquiredAtMs;
            work.listIndex = line.listIndex;
            for ( size_t f=0; f<listed.formats.size(); ++f )
                if ( listed.formats[ f ].formatId == work.formatId )
                    work.estimatedBytes = listed.formats[ f ].sizeKnown ? listed.formats[ f ].bytes : medianBytes;
            worklist.push_back( work );
            ++plan.queuedDownloads;
        }
    }

    // removed upstream: current packs of items a complete listing does not have; never deleted, only reported
    if ( complete )
    {
        for ( size_t p=0; p<m_packs.size(); ++p )
        {
            const SPack& pack = m_packs[ p ];
            if ( !AssetPackStateIsIngested( pack.state ) || m_listedIndex.end() != m_listedIndex.find( pack.itemId ) )
                continue;
            GUCEF::map< CORE::CString, CAssetPackSourceRecord >::const_iterator fact = m_facts.find( pack.packId );
            if ( fact != m_facts.end() && !(*fact).second.IsCurrent() )
                continue;
            m_removedUpstream.insert( pack.itemId );
        }
        if ( !m_removedUpstream.empty() )
            plan.AddCount( "removed-upstream", (UInt32) m_removedUpstream.size() );
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncPlanner::WriteObservations( CAssetSyncMarkStore& syncStore, Int64 nowMs, bool adopt )
{GUCEF_TRACE;

    if ( !syncStore.IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    const CORE::CString storeId = m_spec.storeId.Lowercase();
    TAssetSyncItemRecordVector records;
    for ( size_t i=0; i<m_listed.size(); ++i )
    {
        const SListed& listed = m_listed[ i ];
        if ( listed.itemId.IsNULLOrEmpty() )
            continue;
        if ( !m_selected.empty() && m_selected.end() == m_selected.find( listed.itemId ) )
            continue;
        CAssetSyncItemRecord record;
        TAssetSyncItemRecordMap::const_iterator seen = m_observations.find( listed.itemId );
        if ( seen != m_observations.end() )
            record = (*seen).second;
        record.sourceId = storeId;
        record.itemId = listed.itemId;
        record.lastSeenMs = nowMs;
        record.listedRevision = listed.revision;
        record.listedChangedMs = listed.changedAtMs;
        record.listingFingerprint = listed.listingFingerprint;
        if ( !listed.formatFingerprint.IsNULLOrEmpty() )
            record.formatFingerprint = listed.formatFingerprint;
        if ( listed.enriched )
            record.enrichedMs = nowMs;
        Int64 size = 0;
        for ( size_t f=0; f<listed.formats.size(); ++f )
            size += (Int64) listed.formats[ f ].bytes;
        record.sizeEstimate = size;
        record.available = listed.downloadable;
        record.unavailableReason = listed.downloadable ? CORE::CString::Empty : listed.unavailableReason;
        if ( adopt && m_retried.end() != m_retried.find( listed.itemId ) )
        {
            record.attempts = 0;
            record.nextAttemptMs = 0;
        }
        records.push_back( record );
    }
    for ( CORE::CString::StringSet::const_iterator r = m_removedUpstream.begin(); r != m_removedUpstream.end(); ++r )
    {
        CAssetSyncItemRecord record;
        TAssetSyncItemRecordMap::const_iterator seen = m_observations.find( *r );
        if ( seen != m_observations.end() )
            record = (*seen).second;
        record.sourceId = storeId;
        record.itemId = *r;
        record.available = false;
        record.unavailableReason = "removed-upstream";
        records.push_back( record );
    }
    TAssetStoreStatus status = syncStore.WriteSyncItems( records );
    if ( ASSETSTORE_STATUS_OK != status || !adopt || !m_spec.profile.adoptExisting )
        return status;

    // adoption: the packs from before the facts were kept get them from this listing, so the next plan can compare
    for ( CORE::CString::StringSet::const_iterator a = m_adoptable.begin(); a != m_adoptable.end(); ++a )
    {
        for ( size_t p=0; p<m_packs.size(); ++p )
        {
            if ( m_packs[ p ].packId != *a )
                continue;
            GUCEF::map< CORE::CString, size_t >::const_iterator at = m_listedIndex.find( m_packs[ p ].itemId );
            if ( at == m_listedIndex.end() )
                break;
            const SListed& listed = m_listed[ (*at).second ];
            CAssetPackSourceRecord facts;
            facts.packId = *a;
            facts.sourceId = storeId;
            facts.sourceItemId = listed.itemId;
            facts.revision = listed.revision;
            facts.changedMs = listed.changedAtMs;
            facts.syncedMs = nowMs;
            facts.syncJobId = CORE::CString::Empty;
            const TAssetStoreStatus written = syncStore.WritePackSource( facts );
            if ( ASSETSTORE_STATUS_OK != written )
                status = written;
            break;
        }
    }
    return status;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::NoteFailedAttempt( CAssetSyncItemRecord& record, TAssetStoreStatus status, const CORE::CString& message, Int64 nowMs, const CAssetLibrarySyncProfile& profile )
{GUCEF_TRACE;

    ++record.attempts;
    record.lastAttemptMs = nowMs;
    Int64 wait = (Int64) profile.retryBackoffMinutes * 60 * 1000;
    for ( UInt32 i=1; i<record.attempts && wait < DayMs; ++i )
        wait *= 2;
    if ( wait > DayMs )
        wait = DayMs;
    record.nextAttemptMs = nowMs + wait;
    record.lastStatus = AssetStoreStatusToString( status );
    // the text of a status, never an address: the query of an address that slipped into a message is cut
    CORE::CString text = message;
    const Int32 query = text.HasChar( '?' );
    if ( query >= 0 )
        text = text.SubstrToIndex( (UInt32) query, true );
    record.lastError = text;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlanner::NoteSucceededAttempt( CAssetSyncItemRecord& record, TAssetStoreStatus status, Int64 nowMs )
{GUCEF_TRACE;

    record.attempts = 0;
    record.nextAttemptMs = 0;
    record.lastAttemptMs = nowMs;
    record.lastStatus = AssetStoreStatusToString( status );
    record.lastError.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
