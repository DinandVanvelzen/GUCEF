/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#include "TestFakeAssetStoreService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeAssetStoreService::CFakeAssetStoreService( CORE::PulseGeneratorPtr pulseGenerator )
    : CIAssetStoreService( pulseGenerator )
    , m_answers()
    , m_calls()
    , m_pending()
    , m_providers()
    , m_images()
    , m_imageRequests( 0 )
    , m_inProcess( true )
    , m_open( true )
    , m_catalogue( true )
    , m_defer( false )
    , m_statusMessage()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeAssetStoreService::~CFakeAssetStoreService()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    CompletePendingCalls();
    ClearAnswers();
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::SetAnswer( const CORE::CString& operation, TAssetStoreStatus status, const CORE::CICloneable* result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TAnswerMap::iterator existing = m_answers.find( operation );
    if ( existing != m_answers.end() )
        GUCEF_DELETE (*existing).second.result;
    SAnswer& answer = m_answers[ operation ];
    answer.status = status;
    answer.result = GUCEF_NULL != result ? result->Clone() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::ClearAnswers( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TAnswerMap::iterator i = m_answers.begin(); i != m_answers.end(); ++i )
        GUCEF_DELETE (*i).second.result;
    m_answers.clear();
}

/*-------------------------------------------------------------------------*/

void CFakeAssetStoreService::SetInProcess( bool inProcess ) { m_inProcess = inProcess; }
void CFakeAssetStoreService::SetOpen( bool open ) { m_open = open; }
void CFakeAssetStoreService::SetCatalogueAvailable( bool available ) { m_catalogue = available; }
void CFakeAssetStoreService::SetStatusMessage( const CORE::CString& message ) { m_statusMessage = message; }
void CFakeAssetStoreService::SetDeferAsync( bool defer ) { m_defer = defer; }
CORE::CString CFakeAssetStoreService::GetType( void ) const { return "fake"; }
bool CFakeAssetStoreService::IsInProcess( void ) const { return m_inProcess; }
TAssetStoreStatus CFakeAssetStoreService::Open( void ) { m_open = true; return m_catalogue ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_UNAVAILABLE; }
void CFakeAssetStoreService::Close( void ) { m_open = false; }
bool CFakeAssetStoreService::IsOpen( void ) const { return m_open; }
bool CFakeAssetStoreService::IsCatalogueAvailable( void ) const { return m_open && m_catalogue; }
CORE::CString CFakeAssetStoreService::GetStatusMessage( void ) const { return m_statusMessage; }

/*-------------------------------------------------------------------------*/

UInt32
CFakeAssetStoreService::CompletePendingCalls( void )
{GUCEF_TRACE;

    TPendingVector pending;
    {
        MT::CScopeMutex lock( m_lock );
        pending = m_pending;
        m_pending.clear();
    }
    for ( size_t i=0; i<pending.size(); ++i )
    {
        CORE::CICloneable* result = GUCEF_NULL;
        CORE::CString message;
        const TAssetStoreStatus status = Dispatch( pending[ i ].call, result, message );
        CompleteCall( pending[ i ].source, pending[ i ].call, status, result, message );
    }
    return (UInt32) pending.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeAssetStoreService::GetPendingCallCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CFakeAssetStoreService::ExecuteAsync( const CAssetServiceCall& call )
{GUCEF_TRACE;

    SPending pending;
    pending.call = call;
    if ( m_defer )
    {
        MT::CScopeMutex lock( m_lock );
        m_pending.push_back( pending );
        return pending.source.GetFuture();
    }
    CORE::CICloneable* result = GUCEF_NULL;
    CORE::CString message;
    const TAssetStoreStatus status = Dispatch( call, result, message );
    CompleteCall( pending.source, call, status, result, message );
    return pending.source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::SetStoreProvider( const CORE::CString& storeId, const TAssetStoreProviderPtr& provider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_providers[ storeId ] = provider;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAssetStoreService::GetStoreProvider( const CORE::CString& storeId, TAssetStoreProviderPtr& provider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< CORE::CString, TAssetStoreProviderPtr >::const_iterator i = m_providers.find( storeId );
    if ( i == m_providers.end() )
        return false;
    provider = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::SetListingImage( const CAssetThumbnail& image )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_images.push_back( image );
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeAssetStoreService::GetListingImageRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_imageRequests;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAssetStoreService::TryGetListingImage( const CORE::CString& url, UInt32 size, CAssetThumbnail& image )
{GUCEF_TRACE;

    // the fake has its pictures only after they were asked for once, as a cache would
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAssetStoreService::RequestListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

    CAssetThumbnail answer;
    bool found = false;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_imageRequests;
        for ( size_t i=0; i<m_images.size(); ++i )
            if ( m_images[ i ].url == url )
            {
                answer = m_images[ i ];
                found = true;
            }
    }
    if ( !found )
    {
        answer.url = url;
        answer.status = ASSETSTORE_STATUS_NOT_FOUND;
    }
    answer.size = size;
    Raise( ListingImageReadyEvent, answer );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::CancelListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::Raise( const CORE::CEvent& eventId )
{GUCEF_TRACE;

    NotifyObservers( eventId );
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::Raise( const CORE::CEvent& eventId, const CORE::CICloneable& eventData )
{GUCEF_TRACE;

    CORE::CICloneable* copy = eventData.Clone();
    NotifyObservers( eventId, copy );
    GUCEF_DELETE copy;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeAssetStoreService::GetCallCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_calls.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeAssetStoreService::CountCalls( const CORE::CString& operation ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( size_t i=0; i<m_calls.size(); ++i )
        if ( m_calls[ i ].operation == operation )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAssetStoreService::GetLastCall( const CORE::CString& operation, CAssetServiceCall& call ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=m_calls.size(); i>0; --i )
        if ( m_calls[ i-1 ].operation == operation )
        {
            call = m_calls[ i-1 ];
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeAssetStoreService::ClearCalls( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_calls.clear();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetServiceInfo( CAssetServiceInfo& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetServiceInfo );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetCatalogueChanges );
    call.SetUInt64( "sinceRevision", sinceRevision );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::Search( const CAssetSearchQuery& query, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Search );
    call.SetRecord( "query", query );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::Suggest( const CORE::CString& prefix, UInt32 maxCount, CAssetServiceStringList& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Suggest );
    call.SetText( "prefix", prefix );
    call.SetUInt32( "maxCount", maxCount );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetVocabulary( CAssetVocabularyInfo& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetVocabulary );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetPack( const CORE::CString& packId, CAssetPackRecord& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetPack );
    call.SetText( "packId", packId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetAssets( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRecordPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAssets );
    call.SetText( "packId", packId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetAsset( const CORE::CString& assetId, CAssetRecord& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAsset );
    call.SetText( "assetId", assetId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetRelations( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRelationPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetRelations );
    call.SetText( "assetId", assetId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetIngestHistory( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetIngestHistoryPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetIngestHistory );
    call.SetText( "packId", packId );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetSavedSearches( const CORE::CString& pageToken, UInt32 maxEntries, CAssetSavedSearchPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetSavedSearches );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SaveSearch( const CORE::CString& commandId, const CAssetSavedSearch& search, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SaveSearch, commandId );
    call.SetRecord( "search", search );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::DeleteSavedSearch( const CORE::CString& commandId, const CORE::CString& searchId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::DeleteSavedSearch, commandId );
    call.SetText( "searchId", searchId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RunSavedSearch );
    call.SetText( "searchId", searchId );
    call.SetUInt32( "pageIndex", pageIndex );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetAssetFileSet( const CORE::CString& assetId, CAssetFileSet& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAssetFileSet );
    call.SetText( "assetId", assetId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetLicence );
    call.SetText( "licenceOrPackId", licenceOrPackId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::UpdatePackMetadata( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::UpdatePackMetadata, commandId );
    call.SetRecord( "pack", pack );
    call.SetUInt64( "expectedRevision", expectedRevision );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetClassification( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetClassification, commandId );
    call.SetText( "objectId", objectId );
    call.SetRecord( "classification", classification );
    call.SetText( "source", source );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetCategoryMapping, commandId );
    call.SetText( "sourceId", sourceId );
    call.SetText( "storeCategory", storeCategory );
    call.SetText( "categoryId", categoryId );
    call.SetText( "theme", theme );
    call.SetText( "kind", kind );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RemoveCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemoveCategoryMapping, commandId );
    call.SetText( "sourceId", sourceId );
    call.SetText( "storeCategory", storeCategory );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetSynonym( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetSynonym, commandId );
    call.SetText( "word", word );
    call.SetText( "replacement", replacement );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetCategoryRule( const CORE::CString& commandId, const CAssetCategoryRule& rule, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetCategoryRule, commandId );
    call.SetRecord( "rule", rule );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RemoveCategoryRule( const CORE::CString& commandId, const CORE::CString& ruleId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemoveCategoryRule, commandId );
    call.SetText( "ruleId", ruleId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::CountRuleMatches( const CAssetCategoryRule& rule, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CountRuleMatches );
    call.SetRecord( "rule", rule );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::Reclassify( const CORE::CString& commandId, const CORE::CString::StringVector& packIds, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Reclassify, commandId );
    call.SetTexts( "packIds", packIds );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RemovePack( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RemovePack, commandId );
    call.SetText( "packId", packId );
    call.SetBool( "deleteFiles", deleteFiles );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetThumbnails );
    call.SetTexts( "assetIds", assetIds );
    call.SetUInt32( "size", size );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::PutThumbnail, commandId );
    call.SetRecord( "thumbnail", thumbnail );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetMissingThumbnails );
    call.SetText( "rendererId", rendererId );
    call.SetText( "rendererVersion", rendererVersion );
    call.SetUInt32( "size", size );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::BeginUpload, commandId );
    call.SetText( "name", name );
    call.SetUInt64( "totalBytes", totalBytes );
    call.SetUInt32( "fileCount", fileCount );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::QueryUpload );
    call.SetText( "uploadId", uploadId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CommitUpload, commandId );
    call.SetText( "uploadId", uploadId );
    call.SetRecord( "ingest", ingest );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::AbortUpload, commandId );
    call.SetText( "uploadId", uploadId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetLocations( CAssetLocationPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetLocations );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RefreshLocationStates( CAssetLocationPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RefreshLocationStates );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RelocateLocation, commandId );
    call.SetText( "locationId", locationId );
    call.SetText( "newVfsRoot", newVfsRoot );
    call.SetText( "newFileSystemPath", newFileSystemPath );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::VerifyLocation, commandId );
    call.SetText( "locationId", locationId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::Ingest, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ResumeIngest, commandId );
    call.SetText( "ingestId", ingestId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetPendingIngests( CAssetServiceStringList& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetPendingIngests );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ListImportRoots( CAssetServiceStringList& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListImportRoots );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetSourcePolicy, commandId );
    call.SetRecord( "policy", policy );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListJobs );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetJob( const CORE::CString& jobId, CAssetJobEntry& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetJob );
    call.SetText( "jobId", jobId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ControlJob, commandId );
    call.SetText( "jobId", jobId );
    call.SetText( "action", AssetJobActionToString( action ) );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetJobResult );
    call.SetText( "jobId", jobId );
    call.SetText( "section", section );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ListStores( CAssetStoreInfoPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ListStores );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::QueryLibrary );
    call.SetText( "storeId", storeId );
    call.SetRecord( "query", query );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetStoreItem );
    call.SetText( "storeId", storeId );
    call.SetText( "itemId", itemId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::EnqueueDownload, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::BeginSignIn, commandId );
    call.SetText( "storeId", storeId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::CancelSignIn, commandId );
    call.SetText( "storeId", storeId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SignOut, commandId );
    call.SetText( "storeId", storeId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetSignInPrompts( CAssetSignInPromptPage& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetSignInPrompts );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestConversion, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestExport, commandId );
    call.SetTexts( "assetIds", assetIds );
    call.SetText( "profile", profile );
    call.SetText( "format", format );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RunPipelineQuery );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ApplyPipelineChange, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::StartPipelineJob, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RenderSheet );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::RequestLocalCopy, commandId );
    call.SetRecord( "request", request );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::SetOfflinePin, commandId );
    call.SetText( "packId", packId );
    call.SetBool( "pinned", pinned );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::LockKeyring, commandId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetAuditRecords );
    call.SetText( "pageToken", pageToken );
    call.SetUInt32( "maxEntries", maxEntries );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::ReindexFromStorage, commandId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::GetResourceStatus( CAssetServiceDocument& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::GetResourceStatus );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result )
{GUCEF_TRACE;

    // the answer is a secret: it is not recorded
    CAssetServiceCall call( CAssetServiceOperations::AnswerPrompt, commandId );
    call.SetText( "promptId", promptId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CFakeAssetStoreService::AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = AnswerPrompt( commandId, promptId, answer, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result )
{GUCEF_TRACE;

    CAssetServiceCall call( CAssetServiceOperations::WriteUploadBlock );
    call.SetText( "uploadId", uploadId );
    call.SetText( "relativePath", relativePath );
    call.SetUInt64( "offset", offset );
    call.SetUInt32( "bytes", data.GetDataSize() );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CFakeAssetStoreService::WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = WriteUploadBlock( uploadId, relativePath, offset, data, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeAssetStoreService::UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result )
{GUCEF_TRACE;

    // the code is a secret: it is not recorded
    CAssetServiceCall call( CAssetServiceOperations::UnlockKeyring, commandId );
    return Answer( call, result );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CFakeAssetStoreService::UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = UnlockKeyring( commandId, code, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
