/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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

#include "gucef_test_framework.h"

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#include "AssetWorkbenchLib_CPrimaryStoreSearchModel.h"
#include "AssetWorkbenchLib_CAssetStoreTransferModel.h"
#include "AssetWorkbenchLib_CAssetStoreThumbnailProvider.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestFakeSourceStore.h"
#include "TestFakeAssetStoreService.h"
#include "TestAssetWorkbenchServiceAdapters.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Counts the events of one id of a notifier */
class CCountingObserver : public CORE::CObserver
{
    public:

    UInt32 count;

    CCountingObserver( CORE::CNotifier* notifier, const CORE::CEvent& id ) : CORE::CObserver(), count( 0 ), m_id( id )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CCountingObserver() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == m_id )
            ++count;
    }

    private:

    CORE::CEvent m_id;
};

/** Keeps the last search answer of a search model */
class CSearchAnswerRecorder : public CORE::CObserver
{
    public:

    UInt32 count;
    GUI::CISearchModel::TSearchCompletion last;

    explicit CSearchAnswerRecorder( CORE::CNotifier* notifier ) : CORE::CObserver(), count( 0 ), last()
    {
        SubscribeTo( notifier, GUI::CISearchModel::SearchCompletedEvent );
    }

    virtual ~CSearchAnswerRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != GUI::CISearchModel::SearchCompletedEvent || GUCEF_NULL == eventData )
            return;
        ++count;
        last = static_cast< GUI::CISearchModel::TSearchCompletedEventData* >( eventData )->GetData();
    }
};

ASSETSTORE::CAssetVocabularyInfo
MakeVocabulary( void )
{
    ASSETSTORE::CAssetVocabularyInfo vocabulary;
    vocabulary.kinds.push_back( "static-model" );
    vocabulary.kinds.push_back( "texture" );
    vocabulary.categories.push_back( "weapons" );
    vocabulary.categories.push_back( "weapons/melee" );
    vocabulary.licenceClasses.push_back( "royalty-free" );
    return vocabulary;
}

UInt32
CountChoices( const GUI::CSearchFilterDescription& description, const char* group )
{
    const GUI::CSearchFilterDescription::TGroup* found = description.FindGroup( group );
    return GUCEF_NULL != found ? (UInt32) found->choices.size() : 0;
}

ASSETSTORE::CAssetJobEntry
MakeJob( const CORE::CString& jobId, const CORE::CString& kind, ASSETSTORE::TAssetJobState state )
{
    ASSETSTORE::CAssetJobEntry job;
    job.jobId = jobId;
    job.kind = kind;
    job.name = "Job " + jobId;
    job.state = state;
    return job;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestSearchModelOnService( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Service adapters: the search model asks a service in this process when the form needs it, and searches through the asynchronous form" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        const ASSETSTORE::CAssetVocabularyInfo vocabulary = MakeVocabulary();
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::GetVocabulary, ASSETSTORE::ASSETSTORE_STATUS_OK, &vocabulary );
        ASSETSTORE::CAssetServiceStringList keywords;
        keywords.values.push_back( "sword" );
        keywords.values.push_back( "swords" );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::Suggest, ASSETSTORE::ASSETSTORE_STATUS_OK, &keywords );
        ASSETSTORE::CAssetSearchResultPage page;
        ASSETSTORE::CAssetSearchHit hit;
        hit.assetId = "as1";
        hit.packId = "pk1";
        hit.name = "SM_Sword";
        hit.kind = "static-model";
        hit.packTitle = "Weapons";
        hit.vfsPath = "assetstore/main/weapons/SM_Sword.fbx";
        page.hits.push_back( hit );
        page.total = 1;
        page.pageSize = 50;
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::Search, ASSETSTORE::ASSETSTORE_STATUS_OK, &page );

        CPrimaryStoreSearchModel model( service );
        ASSERT_TRUE( 0 == service.GetCallCount() );
        GUI::CSearchFilterDescription description;
        model.GetFilterDescription( description );
        ASSERT_TRUE( 2 == CountChoices( description, "kind" ) && 1 == CountChoices( description, "category" ) && 1 == CountChoices( description, "licence" ) );
        CORE::CString::StringVector suggestions;
        model.GetSuggestions( "swo", 5, suggestions );
        ASSERT_TRUE( 2 == suggestions.size() && "sword" == suggestions[ 0 ] && 0 == model.GetPendingSuggestionCount() );
        ASSETSTORE::CAssetServiceCall call;
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::Suggest, call ) && "swo" == call.GetText( "prefix" ) && 5 == call.GetUInt32( "maxCount" ) );

        CSearchAnswerRecorder answers( &model );
        GUI::CSearchQuery query;
        query.text = "sword";
        query.AddChoice( "kind", "static-model" );
        model.RequestSearch( query );
        ASSERT_TRUE( 1 == model.Update() && 1 == answers.count && answers.last.succeeded && 1 == answers.last.page.rows.size() );
        ASSERT_TRUE( "as1" == answers.last.page.rows[ 0 ].key && hit.vfsPath == answers.last.page.rows[ 0 ].thumbnailKey );
        ASSETSTORE::CAssetSearchHit found;
        ASSERT_TRUE( model.FindHit( "as1", found ) && "pk1" == found.packId );
        ASSETSTORE::CAssetSearchQuery sent;
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::Search, call ) && call.GetRecord( "query", sent ) && "sword" == sent.text && 1 == sent.kinds.count( "static-model" ) );

        // without a catalogue the search fails at once with the reason
        service.SetCatalogueAvailable( false );
        model.RequestSearch( query );
        ASSERT_TRUE( 2 == answers.count && !answers.last.succeeded && !answers.last.message.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Service adapters: with a service that is not in process the search model never waits: the vocabulary, saved searches and suggestions come in the background and the form is told" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( false );
        service.SetDeferAsync( true );
        const ASSETSTORE::CAssetVocabularyInfo vocabulary = MakeVocabulary();
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::GetVocabulary, ASSETSTORE::ASSETSTORE_STATUS_OK, &vocabulary );
        ASSETSTORE::CAssetSavedSearchPage saved;
        ASSETSTORE::CAssetSavedSearch seed;
        seed.searchId = "seed-1";
        seed.name = "Textures";
        seed.precanned = true;
        saved.entries.push_back( seed );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::GetSavedSearches, ASSETSTORE::ASSETSTORE_STATUS_OK, &saved );
        ASSETSTORE::CAssetServiceStringList keywords;
        keywords.values.push_back( "sword" );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::Suggest, ASSETSTORE::ASSETSTORE_STATUS_OK, &keywords );

        CPrimaryStoreSearchModel model( service );
        CCountingObserver descriptionChanged( &model, GUI::CISearchModel::FilterDescriptionChangedEvent );
        CCountingObserver savedChanged( &model, GUI::CISearchModel::SavedSearchesChangedEvent );
        CCountingObserver suggestionsChanged( &model, GUI::CISearchModel::SuggestionsChangedEvent );
        ASSERT_TRUE( 2 == service.GetPendingCallCount() );
        GUI::CSearchFilterDescription description;
        model.GetFilterDescription( description );
        ASSERT_TRUE( 0 == CountChoices( description, "kind" ) );
        GUI::TSavedSearchVector searches;
        model.GetSavedSearches( searches );
        ASSERT_TRUE( searches.empty() );
        ASSERT_TRUE( 2 == service.CompletePendingCalls() );
        model.Update();
        model.GetFilterDescription( description );
        model.GetSavedSearches( searches );
        ASSERT_TRUE( 1 == descriptionChanged.count && 2 == CountChoices( description, "kind" ) && 1 == savedChanged.count && 1 == searches.size() && searches[ 0 ].builtIn );

        // a prefix that is not cached: no answer yet, one question for any number of keystrokes, then the event and the answer
        CORE::CString::StringVector suggestions;
        model.GetSuggestions( "swo", 8, suggestions );
        model.GetSuggestions( "swo", 8, suggestions );
        ASSERT_TRUE( suggestions.empty() && 1 == model.GetPendingSuggestionCount() && 1 == service.GetPendingCallCount() );
        service.CompletePendingCalls();
        model.Update();
        ASSERT_TRUE( 1 == suggestionsChanged.count && 1 == model.GetCachedSuggestionCount() );
        model.GetSuggestions( "swo", 8, suggestions );
        ASSERT_TRUE( 1 == suggestions.size() && "sword" == suggestions[ 0 ] && 0 == service.GetPendingCallCount() );

        // a change of the vocabulary empties the cache and reads the vocabulary again
        ASSETSTORE::CAssetCatalogueChange change;
        change.kind = ASSETSTORE::ASSETCHANGE_VOCABULARY;
        change.revision = 7;
        service.Raise( ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent, change );
        ASSERT_TRUE( 0 == model.GetCachedSuggestionCount() && 1 == service.GetPendingCallCount() );
        service.CompletePendingCalls();
        model.Update();
        ASSERT_TRUE( 2 == descriptionChanged.count );

        // a saved search is shown at once; a refusal of the service brings its own list back
        GUI::CSavedSearch mine;
        mine.name = "Mine";
        ASSERT_TRUE( model.SaveSearch( mine ) );
        model.GetSavedSearches( searches );
        ASSERT_TRUE( 2 == searches.size() && 1 == service.GetPendingCallCount() );
        ASSETSTORE::CAssetServiceResult refused;
        refused.message = "read only";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::SaveSearch, ASSETSTORE::ASSETSTORE_STATUS_REFUSED, &refused );
        service.CompletePendingCalls();
        model.Update();
        ASSERT_TRUE( 1 == service.GetPendingCallCount() && 1 == service.CountCalls( ASSETSTORE::CAssetServiceOperations::SaveSearch ) );
        service.CompletePendingCalls();
        model.Update();
        model.GetSavedSearches( searches );
        ASSERT_TRUE( 1 == searches.size() && "seed-1" == searches[ 0 ].id );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestTransferModelOnService( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Service adapters: the transfer model shows the jobs of the service with their words, progress and actions, and asks the service to act" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        ASSETSTORE::CAssetJobPage jobs;
        ASSETSTORE::CAssetJobEntry download = MakeJob( "dl1", "download", ASSETSTORE::ASSETJOB_STATE_RUNNING );
        download.totalBytes = 1000;
        download.doneBytes = 250;
        download.bytesPerSecond = 100;
        download.secondsLeft = 8;
        download.folder = "downloads/store/item";
        download.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_PAUSE, true );
        download.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL, true );
        jobs.entries.push_back( download );
        ASSETSTORE::CAssetJobEntry ingest = MakeJob( "ingest:i1", "ingest", ASSETSTORE::ASSETJOB_STATE_FAILED );
        ingest.fraction = 0.5f;
        ingest.message = "the analyse step failed";
        ingest.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY, true );
        ingest.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_REMOVE, true );
        jobs.entries.push_back( ingest );
        jobs.entries.push_back( MakeJob( "convert:cv1", "conversion", ASSETSTORE::ASSETJOB_STATE_QUEUED ) );
        jobs.total = 3;
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &jobs );
        ASSETSTORE::CAssetServiceResult accepted;
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ControlJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &accepted );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::RequestConversion, ASSETSTORE::ASSETSTORE_STATUS_OK, &accepted );

        CAssetStoreTransferModel model( service );
        GUI::CITransferQueueModel::TEntryVector entries;
        model.GetEntries( entries );
        ASSERT_TRUE( 3 == entries.size() && "Download" == entries[ 0 ].kind && "Ingest" == entries[ 1 ].kind && "Conversion" == entries[ 2 ].kind );
        ASSERT_TRUE( GUI::CITransferQueueModel::TRANSFERSTATE_RUNNING == entries[ 0 ].state && 100 == entries[ 0 ].bytesPerSecond && 8 == entries[ 0 ].secondsLeft );
        ASSERT_TRUE( entries[ 0 ].GetProgress() > 0.24f && entries[ 0 ].GetProgress() < 0.26f && "downloads/store/item" == entries[ 0 ].folder );
        ASSERT_TRUE( GUI::CITransferQueueModel::TRANSFERSTATE_FAILED == entries[ 1 ].state && 0.5f == entries[ 1 ].fraction && "the analyse step failed" == entries[ 1 ].message );
        ASSERT_TRUE( model.IsActionAvailable( entries[ 0 ], GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) && !model.IsActionAvailable( entries[ 0 ], GUI::CITransferQueueModel::TRANSFERACTION_RESUME ) );
        ASSERT_TRUE( model.IsActionAvailable( entries[ 0 ], GUI::CITransferQueueModel::TRANSFERACTION_OPEN_FOLDER ) );
        ASSERT_TRUE( model.IsActionAvailable( entries[ 1 ], GUI::CITransferQueueModel::TRANSFERACTION_RETRY ) && !model.IsActionAvailable( entries[ 1 ], GUI::CITransferQueueModel::TRANSFERACTION_CANCEL ) );
        ASSERT_TRUE( 1 == model.GetIngestCount() && 1 == model.GetConversionCount() );

        ASSERT_TRUE( model.PerformAction( "dl1", GUI::CITransferQueueModel::TRANSFERACTION_PAUSE ) );
        ASSETSTORE::CAssetServiceCall call;
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "dl1" == call.GetText( "jobId" ) && "pause" == call.GetText( "action" ) );
        ASSERT_FALSE( model.PerformAction( "dl1", GUI::CITransferQueueModel::TRANSFERACTION_OPEN_FOLDER ) );
        ASSERT_TRUE( model.StartConversion( "pk9" ) && service.GetLastCall( ASSETSTORE::CAssetServiceOperations::RequestConversion, call ) );
        ASSETSTORE::CAssetConversionJobRequest request;
        ASSERT_TRUE( call.GetRecord( "request", request ) && "pk9" == request.packId && !request.allPending );
        ASSERT_TRUE( model.StartPendingConversion() && service.GetLastCall( ASSETSTORE::CAssetServiceOperations::RequestConversion, call ) && call.GetRecord( "request", request ) && request.allPending );

        // the events of the service tell the form
        CCountingObserver changed( &model, GUI::CITransferQueueModel::EntriesChangedEvent );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, download );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobRemovedEvent, ingest );
        ASSERT_TRUE( 2 == changed.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Service adapters: with a service that is not in process the transfer model mirrors the jobs: a read in the background, the events update it, actions are sent" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( false );
        service.SetDeferAsync( true );
        ASSETSTORE::CAssetJobPage jobs;
        ASSETSTORE::CAssetJobEntry download = MakeJob( "dl1", "download", ASSETSTORE::ASSETJOB_STATE_QUEUED );
        jobs.entries.push_back( download );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &jobs );

        CAssetStoreTransferModel model( service );
        GUI::CITransferQueueModel::TEntryVector entries;
        model.GetEntries( entries );
        ASSERT_TRUE( entries.empty() && 1 == service.GetPendingCallCount() );
        service.CompletePendingCalls();
        model.Update();
        model.GetEntries( entries );
        ASSERT_TRUE( 1 == entries.size() && GUI::CITransferQueueModel::TRANSFERSTATE_QUEUED == entries[ 0 ].state );

        // a job that changed is taken from the event, one that is new reads the list again
        download.state = ASSETSTORE::ASSETJOB_STATE_RUNNING;
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, download );
        model.GetEntries( entries );
        ASSERT_TRUE( GUI::CITransferQueueModel::TRANSFERSTATE_RUNNING == entries[ 0 ].state && 0 == service.GetPendingCallCount() );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, MakeJob( "ingest:new", "ingest", ASSETSTORE::ASSETJOB_STATE_RUNNING ) );
        ASSERT_TRUE( 1 == service.GetPendingCallCount() );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobRemovedEvent, download );
        model.GetEntries( entries );
        ASSERT_TRUE( entries.empty() );

        ASSERT_TRUE( model.PerformAction( "dl1", GUI::CITransferQueueModel::TRANSFERACTION_CANCEL ) && 2 == service.GetPendingCallCount() );
        service.CompletePendingCalls();
        ASSERT_TRUE( 1 == service.CountCalls( ASSETSTORE::CAssetServiceOperations::ControlJob ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestListingThumbnailsOnService( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Service adapters: the listing thumbnails of a store library are asked of the service once per picture and answered from its event" )
    try
    {
        SOURCESTEST::TFakeSourceFactory factory;
        ASSETSTORE::TAssetStoreProviderPtr provider = factory.Create();
        SOURCESTEST::CFakeSourceProvider* fake = static_cast< SOURCESTEST::CFakeSourceProvider* >( provider.GetPointerAlways() );
        GUCEF::vector< SOURCESTEST::CFakeSourceProvider::SFormatSpec > none;
        fake->AddItem( "pic-1", "With picture", "Seller", "3d-model", none, CORE::CString::Empty, "http://127.0.0.1:1/listing.png" );
        ASSETSTORE::CAssetStoreLibraryFileSystemAccess library( provider );
        library.Refresh();
        for ( UInt32 i=0; i<300 && library.GetItemCount() < 1; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( 1 == library.GetItemCount() );
        const CORE::CString path = library.GetPathOfItem( "pic-1" );

        ASSETSTORETEST::CFakeAssetStoreService service;
        ASSETSTORE::CAssetThumbnail picture;
        picture.url = "http://127.0.0.1:1/listing.png";
        picture.kind = "listing";
        service.SetListingImage( picture );
        CAssetStoreThumbnailProvider thumbnails( service, library );
        CThumbnailRecorder recorder( &thumbnails, GUI::CIThumbnailProvider::ThumbnailReadyEvent );
        thumbnails.RequestThumbnail( path, 64 );
        ASSERT_TRUE( 1 == service.GetListingImageRequestCount() && 1 == recorder.count && recorder.lastPath == path && 64 == recorder.lastSize && 0 == thumbnails.GetWaitingCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchServiceAdapterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchServiceAdapters" );

    TestSearchModelOnService();
    TestTransferModelOnService();
    TestListingThumbnailsOnService();
}

/*-------------------------------------------------------------------------*/
