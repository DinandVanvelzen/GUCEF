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

#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */

#include "gucefASSETSTORE_CAssetSyncMarkStore.h"

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

void
PackSourceToRow( const CAssetPackSourceRecord& record, CAssetCatalogue::TRow& row )
{
    row.clear();
    row[ "pack_id" ] = CORE::CVariant( record.packId );
    row[ "source_id" ] = CORE::CVariant( record.sourceId );
    row[ "source_item_id" ] = CORE::CVariant( record.sourceItemId );
    row[ "source_format_id" ] = CORE::CVariant( record.sourceFormatId );
    row[ "revision" ] = CORE::CVariant( record.revision );
    row[ "changed_ms" ] = CORE::CVariant( record.changedMs );
    row[ "content_digest" ] = CORE::CVariant( record.contentDigest );
    row[ "superseded_by" ] = CORE::CVariant( record.supersededBy );
    row[ "superseded_ms" ] = CORE::CVariant( record.supersededMs );
    row[ "synced_ms" ] = CORE::CVariant( record.syncedMs );
    row[ "sync_job_id" ] = CORE::CVariant( record.syncJobId );
}

void
PackSourceFromRow( const CAssetCatalogue::TRow& row, CAssetPackSourceRecord& record )
{
    record.packId = CAssetCatalogue::GetText( row, "pack_id" );
    record.sourceId = CAssetCatalogue::GetText( row, "source_id" );
    record.sourceItemId = CAssetCatalogue::GetText( row, "source_item_id" );
    record.sourceFormatId = CAssetCatalogue::GetText( row, "source_format_id" );
    record.revision = CAssetCatalogue::GetText( row, "revision" );
    record.changedMs = CAssetCatalogue::GetInt( row, "changed_ms" );
    record.contentDigest = CAssetCatalogue::GetText( row, "content_digest" );
    record.supersededBy = CAssetCatalogue::GetText( row, "superseded_by" );
    record.supersededMs = CAssetCatalogue::GetInt( row, "superseded_ms" );
    record.syncedMs = CAssetCatalogue::GetInt( row, "synced_ms" );
    record.syncJobId = CAssetCatalogue::GetText( row, "sync_job_id" );
}

void
SyncItemToRow( const CAssetSyncItemRecord& record, CAssetCatalogue::TRow& row )
{
    row.clear();
    row[ "source_id" ] = CORE::CVariant( record.sourceId );
    row[ "item_id" ] = CORE::CVariant( record.itemId );
    row[ "last_seen_ms" ] = CORE::CVariant( record.lastSeenMs );
    row[ "listed_revision" ] = CORE::CVariant( record.listedRevision );
    row[ "listed_changed_ms" ] = CORE::CVariant( record.listedChangedMs );
    row[ "listing_fingerprint" ] = CORE::CVariant( record.listingFingerprint );
    row[ "format_fingerprint" ] = CORE::CVariant( record.formatFingerprint );
    row[ "enriched_ms" ] = CORE::CVariant( record.enrichedMs );
    row[ "size_estimate" ] = CORE::CVariant( record.sizeEstimate );
    row[ "available" ] = CORE::CVariant( record.available );
    row[ "unavailable_reason" ] = CORE::CVariant( record.unavailableReason );
    row[ "attempts" ] = CORE::CVariant( (Int64) record.attempts );
    row[ "last_attempt_ms" ] = CORE::CVariant( record.lastAttemptMs );
    row[ "next_attempt_ms" ] = CORE::CVariant( record.nextAttemptMs );
    row[ "last_status" ] = CORE::CVariant( record.lastStatus );
    row[ "last_error" ] = CORE::CVariant( record.lastError );
}

void
SyncItemFromRow( const CAssetCatalogue::TRow& row, CAssetSyncItemRecord& record )
{
    record.sourceId = CAssetCatalogue::GetText( row, "source_id" );
    record.itemId = CAssetCatalogue::GetText( row, "item_id" );
    record.lastSeenMs = CAssetCatalogue::GetInt( row, "last_seen_ms" );
    record.listedRevision = CAssetCatalogue::GetText( row, "listed_revision" );
    record.listedChangedMs = CAssetCatalogue::GetInt( row, "listed_changed_ms" );
    record.listingFingerprint = CAssetCatalogue::GetText( row, "listing_fingerprint" );
    record.formatFingerprint = CAssetCatalogue::GetText( row, "format_fingerprint" );
    record.enrichedMs = CAssetCatalogue::GetInt( row, "enriched_ms" );
    record.sizeEstimate = CAssetCatalogue::GetInt( row, "size_estimate" );
    record.available = CAssetCatalogue::GetBool( row, "available", true );
    record.unavailableReason = CAssetCatalogue::GetText( row, "unavailable_reason" );
    record.attempts = (UInt32) CAssetCatalogue::GetInt( row, "attempts" );
    record.lastAttemptMs = CAssetCatalogue::GetInt( row, "last_attempt_ms" );
    record.nextAttemptMs = CAssetCatalogue::GetInt( row, "next_attempt_ms" );
    record.lastStatus = CAssetCatalogue::GetText( row, "last_status" );
    record.lastError = CAssetCatalogue::GetText( row, "last_error" );
}

}

/*-------------------------------------------------------------------------*/

CAssetSyncMarkStore::CAssetSyncMarkStore( CAssetCatalogue& catalogue )
    : m_catalogue( catalogue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSyncMarkStore::~CAssetSyncMarkStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncMarkStore::IsAvailable( void ) const
{GUCEF_TRACE;

    return m_catalogue.IsOpen() && m_catalogue.GetSchemaVersion() >= 5;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::WritePackSource( const CAssetPackSourceRecord& record )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    if ( record.packId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CAssetCatalogue::TRowVector rows( 1 );
    PackSourceToRow( record, rows[ 0 ] );
    return m_catalogue.Write( "pack_sources", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetPackSource( const CORE::CString& packId, CAssetPackSourceRecord& record ) const
{GUCEF_TRACE;

    record.Clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_sources", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    PackSourceFromRow( rows[ 0 ], record );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetPackSourcesOfStore( const CORE::CString& sourceId, TAssetPackSourceRecordVector& records ) const
{GUCEF_TRACE;

    records.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_sources", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    records.resize( rows.size() );
    for ( size_t r=0; r<rows.size(); ++r )
        PackSourceFromRow( rows[ r ], records[ r ] );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetPackSourcesOfItem( const CORE::CString& sourceId, const CORE::CString& itemId, TAssetPackSourceRecordVector& records ) const
{GUCEF_TRACE;

    records.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    query.WhereEquals( "source_item_id", CORE::CVariant( itemId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_sources", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    records.resize( rows.size() );
    for ( size_t r=0; r<rows.size(); ++r )
        PackSourceFromRow( rows[ r ], records[ r ] );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::SupersedeOlderPacks( const CAssetPackSourceRecord& newPack, Int64 nowMs, UInt32& superseded )
{GUCEF_TRACE;

    superseded = 0;
    TAssetPackSourceRecordVector others;
    TAssetStoreStatus status = GetPackSourcesOfItem( newPack.sourceId, newPack.sourceItemId, others );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CAssetCatalogue::TRowVector rows;
    for ( size_t i=0; i<others.size(); ++i )
    {
        CAssetPackSourceRecord& other = others[ i ];
        if ( other.packId == newPack.packId || !other.IsCurrent() )
            continue;
        // a pack of another format stays current; one whose format is not known is replaced by the first new format
        if ( !other.sourceFormatId.IsNULLOrEmpty() && !newPack.sourceFormatId.IsNULLOrEmpty() && other.sourceFormatId != newPack.sourceFormatId )
            continue;
        other.supersededBy = newPack.packId;
        other.supersededMs = nowMs;
        CAssetCatalogue::TRow row;
        PackSourceToRow( other, row );
        rows.push_back( row );
    }
    if ( rows.empty() )
        return ASSETSTORE_STATUS_OK;
    status = m_catalogue.Write( "pack_sources", rows );
    if ( ASSETSTORE_STATUS_OK == status )
        superseded = (UInt32) rows.size();
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::SetMark( const CAssetSyncMark& mark )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    if ( mark.sourceId.IsNULLOrEmpty() || mark.itemId.IsNULLOrEmpty() || ( CAssetSyncMark::Ignore != mark.mark && CAssetSyncMark::Pin != mark.mark ) )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "source_id" ] = CORE::CVariant( mark.sourceId.Lowercase() );
    rows[ 0 ][ "item_id" ] = CORE::CVariant( mark.itemId );
    rows[ 0 ][ "mark" ] = CORE::CVariant( mark.mark );
    rows[ 0 ][ "reason" ] = CORE::CVariant( mark.reason );
    rows[ 0 ][ "marked_ms" ] = CORE::CVariant( mark.markedMs );
    return m_catalogue.Write( "sync_marks", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::RemoveMark( const CORE::CString& sourceId, const CORE::CString& itemId )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    query.WhereEquals( "item_id", CORE::CVariant( itemId ) );
    return m_catalogue.Delete( "sync_marks", query );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetMarks( const CORE::CString& sourceId, TAssetSyncMarkVector& marks ) const
{GUCEF_TRACE;

    marks.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "sync_marks", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    marks.resize( rows.size() );
    for ( size_t r=0; r<rows.size(); ++r )
    {
        marks[ r ].sourceId = CAssetCatalogue::GetText( rows[ r ], "source_id" );
        marks[ r ].itemId = CAssetCatalogue::GetText( rows[ r ], "item_id" );
        marks[ r ].mark = CAssetCatalogue::GetText( rows[ r ], "mark" );
        marks[ r ].reason = CAssetCatalogue::GetText( rows[ r ], "reason" );
        marks[ r ].markedMs = CAssetCatalogue::GetInt( rows[ r ], "marked_ms" );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::WriteSyncItems( const TAssetSyncItemRecordVector& records )
{GUCEF_TRACE;

    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    if ( records.empty() )
        return ASSETSTORE_STATUS_OK;
    CAssetCatalogue::TRowVector rows( records.size() );
    for ( size_t r=0; r<records.size(); ++r )
    {
        SyncItemToRow( records[ r ], rows[ r ] );
        rows[ r ][ "source_id" ] = CORE::CVariant( records[ r ].sourceId.Lowercase() );
    }
    return m_catalogue.Write( "sync_items", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetSyncItem( const CORE::CString& sourceId, const CORE::CString& itemId, CAssetSyncItemRecord& record ) const
{GUCEF_TRACE;

    record = CAssetSyncItemRecord();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    query.WhereEquals( "item_id", CORE::CVariant( itemId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "sync_items", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    SyncItemFromRow( rows[ 0 ], record );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetSyncMarkStore::GetSyncItems( const CORE::CString& sourceId, TAssetSyncItemRecordMap& records ) const
{GUCEF_TRACE;

    records.clear();
    if ( !IsAvailable() )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "source_id", CORE::CVariant( sourceId.Lowercase() ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "sync_items", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        CAssetSyncItemRecord record;
        SyncItemFromRow( rows[ r ], record );
        records[ record.itemId ] = record;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
