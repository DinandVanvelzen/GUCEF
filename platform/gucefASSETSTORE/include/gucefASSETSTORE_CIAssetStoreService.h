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

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#include "CTAbstractFactoryWithParam.h"
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCATEGORYRULE_H
#include "gucefASSETSTORE_CAssetCategoryRule.h"
#define GUCEF_ASSETSTORE_CASSETCATEGORYRULE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATEGORYRULE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H
#include "gucefASSETSTORE_CAssetClassification.h"
#define GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H
#endif /* GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONJOBREQUEST_H
#include "gucefASSETSTORE_CAssetConversionJobRequest.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONJOBREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONJOBREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H
#include "gucefASSETSTORE_CAssetDownloadRequest.h"
#define GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETFILESET_H
#include "gucefASSETSTORE_CAssetFileSet.h"
#define GUCEF_ASSETSTORE_CASSETFILESET_H
#endif /* GUCEF_ASSETSTORE_CASSETFILESET_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#include "gucefASSETSTORE_CAssetIngestResult.h"
#define GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTRESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#include "gucefASSETSTORE_CAssetJobEntry.h"
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLOCALCOPYREQUEST_H
#include "gucefASSETSTORE_CAssetLocalCopyRequest.h"
#define GUCEF_ASSETSTORE_CASSETLOCALCOPYREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETLOCALCOPYREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#include "gucefASSETSTORE_CAssetSavedSearch.h"
#define GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H
#endif /* GUCEF_ASSETSTORE_CASSETSAVEDSEARCH_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#include "gucefASSETSTORE_CAssetSearchQuery.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H
#include "gucefASSETSTORE_CAssetSearchResultPage.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHRESULTPAGE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#include "gucefASSETSTORE_CAssetServiceCall.h"
#define GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICECALL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEINFO_H
#include "gucefASSETSTORE_CAssetServiceInfo.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEINFO_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#include "gucefASSETSTORE_CAssetServiceResult.h"
#define GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICERESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICESTRINGLIST_H
#include "gucefASSETSTORE_CAssetServiceStringList.h"
#define GUCEF_ASSETSTORE_CASSETSERVICESTRINGLIST_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICESTRINGLIST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H
#include "gucefASSETSTORE_CAssetSourcePolicy.h"
#define GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H
#endif /* GUCEF_ASSETSTORE_CASSETSOURCEPOLICY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#include "gucefASSETSTORE_CAssetStoreQuery.h"
#define GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#include "gucefASSETSTORE_CAssetThumbnail.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H
#include "gucefASSETSTORE_CAssetVerificationReport.h"
#define GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H
#endif /* GUCEF_ASSETSTORE_CASSETVERIFICATIONREPORT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H
#include "gucefASSETSTORE_CAssetVocabularyInfo.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H
#include "gucefASSETSTORE_AssetServicePages.h"
#define GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H
#endif /* GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H
#include "gucefASSETSTORE_CAssetPackIndexInfo.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#include "gucefASSETSTORE_CAssetLibrarySyncProfile.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H
#include "gucefASSETSTORE_CAssetSettingsRequest.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H
#include "gucefASSETSTORE_CAssetSettingsDescription.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H
#include "gucefASSETSTORE_CAssetSettingsChangeResult.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H ? */

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
 *  The asset store as one service (section 4 of the service design): every function of the store that a viewer, a tool
 *  or the MCP tool set uses, so they depend on this interface only and work the same with the in-process implementation
 *  (CLocalAssetStoreService, the store of this process) and with an implementation that reaches the store of a server.
 *
 *  Method groups (section 4.2): Info, Catalogue (search, facets, suggestions, vocabulary, packs, assets, relations, ingest
 *  history, saved searches, file sets, licences), Curation, Thumbnails, Files, Locations, Ingest, Jobs, Stores (online
 *  stores, their library and items, downloads, listing pictures), SignIn, Conversion, Pipeline (the operations of the
 *  converter pipeline and the render path for the tools, carried out by CIAssetConverterHook and CIAssetRenderHook),
 *  LocalCopy and Admin. An implementation that cannot carry out an operation answers ASSETSTORE_STATUS_UNSUPPORTED with
 *  a message that says why; GetServiceInfo() names the capabilities it has.
 *
 *  Shape (section 4.1):
 *  - Every operation has a synchronous form, which runs on the calling thread and blocks until it is done, and an
 *    asynchronous form (<name>Async()) that returns a CORE::CFutureResult. The asynchronous forms make a CAssetServiceCall
 *    and hand it to ExecuteAsync(): an in-process implementation runs Dispatch() of the call as a pool task, a remote one
 *    sends it. The completed future carries the result object of the synchronous form (TryGetFutureResult()); a failure
 *    ends the future in an error state with "statusName: message" as its extra information (GetStatusOfFuture(),
 *    GetMessageOfFuture()). Nobody pumps a pulse generator that is not their own.
 *  - The synchronous form of an implementation that is not in process (IsInProcess() false) waits for the network: it
 *    must not be used on the thread of a GUI.
 *  - Mutations carry a command id (commandId, empty for none): a repeat with the same id answers the first outcome
 *    (result.replayed) instead of acting twice; the same id used for another operation is INVALID_ARGUMENT.
 *  - Lists are paged with a page token (empty: the first page) and a maximum of entries (0: DefaultPageEntries, at most
 *    MaxPageEntries); every page carries the catalogue revision it was read at.
 *  - Values are the record types of the module and the value types of the service, all of them serialisable to a
 *    DataNode; no raw pointer crosses the interface. The exceptions are handles that are objects themselves: the provider
 *    of an online store (GetStoreProvider(), for the browsing of its library through CAssetStoreLibraryFileSystemAccess)
 *    and the pixels of pictures.
 *  - Secrets never go into a call: AnswerPrompt(), ImportWebSession() and UnlockKeyring() take a AUTH::CSecretString and
 *    have asynchronous forms of their own, as has WriteUploadBlock() for its bytes.
 *
 *  Events (section 4.3) are raised on the thread of the pulse generator given at construction (the GUI thread of a
 *  viewer): CatalogueChangedEvent (CAssetCatalogueChange), JobChangedEvent and JobRemovedEvent (CAssetJobEntry),
 *  LocationStateChangedEvent (CAssetLocationInfo), StoreStateChangedEvent (CAssetStoreInfo), StoresChangedEvent (no data:
 *  stores came or went), SignInPromptEvent and SignInPromptClosedEvent (CAssetSignInPrompt), ThumbnailUpdatedEvent and
 *  ListingImageReadyEvent (CAssetThumbnail), NoticeEvent (CAssetServiceNotice), SettingsChangedEvent (CAssetSettingsChangeResult:
 *  the run-time settings changed) and ConnectionStateChangedEvent (CAssetServiceNotice, its code the state; implementations that
 *  are not in process only).
 *
 *  Implementations are made by the factories registered in CAssetStoreGlobal::GetServiceFactory() under their id
 *  (GetType()), with a CAssetStoreServiceConfig; "local" is registered by the module.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStoreService : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent CatalogueChangedEvent;
    static const CORE::CEvent JobChangedEvent;
    static const CORE::CEvent JobRemovedEvent;
    static const CORE::CEvent LocationStateChangedEvent;
    static const CORE::CEvent StoreStateChangedEvent;
    static const CORE::CEvent StoresChangedEvent;
    static const CORE::CEvent SignInPromptEvent;
    static const CORE::CEvent SignInPromptClosedEvent;
    static const CORE::CEvent ThumbnailUpdatedEvent;
    static const CORE::CEvent ListingImageReadyEvent;
    static const CORE::CEvent NoticeEvent;
    static const CORE::CEvent SettingsChangedEvent;
    static const CORE::CEvent ConnectionStateChangedEvent;

    static void RegisterEvents( void );

    static const UInt32 DefaultPageEntries = 100;
    static const UInt32 MaxPageEntries = 1000;

    /** Events are raised on the thread of the pulse generator, of the global one when it is null */
    explicit CIAssetStoreService( CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CIAssetStoreService() GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  The implementation
     */

    /** The id of the implementation, the key of its factory: "local", "remote" */
    virtual CORE::CString GetType( void ) const = 0;

    /** Whether the calls run in this process without a network: the synchronous forms are then cheap enough for a GUI thread */
    virtual bool IsInProcess( void ) const = 0;

    /** Makes the service usable: opens the store, or connects. An implementation may serve part of the operations when this fails (GetStatusMessage() says what) */
    virtual TAssetStoreStatus Open( void ) = 0;

    virtual void Close( void ) = 0;

    virtual bool IsOpen( void ) const = 0;

    /** Whether the catalogue of the primary store can be used now */
    virtual bool IsCatalogueAvailable( void ) const = 0;

    /** What is wrong when the service or its catalogue is not available, for people; empty otherwise */
    virtual CORE::CString GetStatusMessage( void ) const = 0;

    /*
     *  The asynchronous forms, and the carrying out of calls
     */

    /** Carries out a call asynchronously: the asynchronous forms of the operations come here */
    virtual CORE::CFutureResult ExecuteAsync( const CAssetServiceCall& call ) = 0;

    /**
     *  Carries out a call on the calling thread through the synchronous forms: what an in-process implementation runs on
     *  its pool thread, and what a host of the service runs for a remote caller. 'result' receives the result object of
     *  the operation (owned by the caller, GUCEF_NULL for an unknown operation); 'message' what went wrong
     */
    TAssetStoreStatus Dispatch( const CAssetServiceCall& call, CORE::CICloneable*& result, CORE::CString& message );

    /** Whether the status ends the call with its record (also DUPLICATE of an ingest, and every status of Settings/Set and Settings/Reset: the record has the status and the reasons per key) */
    static bool IsOutcomeStatus( const CORE::CString& operation, TAssetStoreStatus status );

    /** Ends the future of a call with the result of Dispatch(); takes ownership of the result */
    static void CompleteCall( CORE::CFutureResultSource& source, const CAssetServiceCall& call, TAssetStoreStatus status,
                              CORE::CICloneable* result, const CORE::CString& message );

    /** A future that ended already, for answers that need no work: completed with the result (owned) or failed with the status */
    static CORE::CFutureResult MakeEndedFuture( TAssetStoreStatus status, CORE::CICloneable* result, const CORE::CString& message );

    /** The status of a future of the service that ended: OK when it finished, otherwise the status of its extra information */
    static TAssetStoreStatus GetStatusOfFuture( const CORE::CFutureResult& future );

    /** The message of a future of the service that failed, empty otherwise */
    static CORE::CString GetMessageOfFuture( const CORE::CFutureResult& future );

    /** The result object of a future that finished; false when it is not ready, failed or holds no T */
    template < class T >
    static bool TryGetFutureResult( const CORE::CFutureResult& future, T& result )
    {
        if ( !future.IsReady() )
            return false;
        CORE::CTaskPtr task = future.GetResult( 0 );
        if ( task.IsNULL() || CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() )
            return false;
        const T* data = CORE::CICloneable::CloneableAs< T >( task->GetTaskData() );
        if ( GUCEF_NULL == data )
            return false;
        result = *data;
        return true;
    }

    /** The maximum of entries of a page: DefaultPageEntries for 0, at most MaxPageEntries */
    static UInt32 ClampPageEntries( UInt32 maxEntries );

    /** The page tokens of the lists that page by position: the position as text */
    static UInt32 ParsePageToken( const CORE::CString& pageToken );
    static CORE::CString MakePageToken( UInt32 offset );

    /** A page of a list that is held whole: the entries from the position of the token, the total and the next token */
    template < class TPage, class TEntryVector >
    static void FillPage( const TEntryVector& all, const CORE::CString& pageToken, UInt32 maxEntries, TPage& page )
    {
        page.entries.clear();
        const UInt32 first = ParsePageToken( pageToken );
        const UInt32 count = ClampPageEntries( maxEntries );
        for ( UInt32 i=first; i<(UInt32) all.size() && i<first+count; ++i )
            page.entries.push_back( all[ i ] );
        page.total = all.size();
        page.nextPageToken = first + count < (UInt32) all.size() ? MakePageToken( first + count ) : CORE::CString::Empty;
    }

    /*
     *  Handles and pictures that are not calls
     */

    /**
     *  The licence restriction flags of the pack that holds a VFS path of the storage locations of this service, for the gate of the files
     *  that a host serves; false when the path is in no pack. Not an operation of the wire: the host asks the service it serves. The
     *  default has no pack for any path
     */
    virtual bool GetRestrictionsOfPath( const CORE::CString& vfsPath, CORE::CString::StringSet& flags ) const;

    /**
     *  What GetRestrictionsOfPath() answers, when the service holds that answer in memory now (current for the catalogue it is
     *  asked of): true with 'flags' and 'inPack' as GetRestrictionsOfPath() gives them, and GetRestrictionsOfPath() answers the
     *  same path without asking the catalogue. False, nothing set, when the answer is not held: it would take a query of the
     *  catalogue. Never asks the catalogue and never waits for one: for callers that must not wait (a thread of a network node); the
     *  answer is made held by one call of GetRestrictionsOfPath() on a thread that may wait. The default holds nothing
     */
    virtual bool TryGetRestrictionsOfPathKept( const CORE::CString& vfsPath, CORE::CString::StringSet& flags, bool& inPack ) const;

    /**
     *  The size and the SHA-256 (lower case hexadecimal) that the catalogue has of the file at a VFS path of the storage locations;
     *  false when the catalogue knows no such file. The catalogue is asked once per pack and catalogue revision (the files of a pack are
     *  kept), never once per file: for a thread that may wait (a pool thread). The default knows none
     */
    virtual bool GetCatalogueDigestOfPath( const CORE::CString& vfsPath, UInt64& sizeBytes, CORE::CString& sha256Hex ) const;

    /** The provider of an online store for the browsing of its library (CAssetStoreLibraryFileSystemAccess); false for an unknown store */
    virtual bool GetStoreProvider( const CORE::CString& storeId, TAssetStoreProviderPtr& provider ) = 0;

    /** A listing picture of a store item that the service holds already, at once and without waiting; false when it has none */
    virtual bool TryGetListingImage( const CORE::CString& url, UInt32 size, CAssetThumbnail& image ) = 0;

    /** Asks for a listing picture: ListingImageReadyEvent follows, also for a failure (the status of the event data says). False when it cannot be asked */
    virtual bool RequestListingImage( const CORE::CString& url, UInt32 size ) = 0;

    /** Takes a request for a listing picture back: no event follows */
    virtual void CancelListingImage( const CORE::CString& url, UInt32 size ) = 0;

    /*
     *  The operations, by method group
     */

    // Info

    /** What the service is and offers: implementation, version, capabilities, roles of the caller, limits, the storage locations with their state, the online stores, the source policies, the catalogue revision, the keyring state and the converter */
    virtual TAssetStoreStatus GetServiceInfo( CAssetServiceInfo& result ) = 0;
    CORE::CFutureResult GetServiceInfoAsync(  );

    /** The changes of the catalogue after the revision, oldest first; a single RESYNC entry when the journal no longer holds them. page.revision is the revision now */
    virtual TAssetStoreStatus GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result ) = 0;
    CORE::CFutureResult GetCatalogueChangesAsync( UInt64 sinceRevision, UInt32 maxEntries );

    // Catalogue

    /** A page of results with the facets; the page carries the catalogue revision it was read at */
    virtual TAssetStoreStatus Search( const CAssetSearchQuery& query, CAssetSearchResultPage& result ) = 0;
    CORE::CFutureResult SearchAsync( const CAssetSearchQuery& query );

    /** Keywords of the index that start with the prefix, at most maxCount (at most 64) */
    virtual TAssetStoreStatus Suggest( const CORE::CString& prefix, UInt32 maxCount, CAssetServiceStringList& result ) = 0;
    CORE::CFutureResult SuggestAsync( const CORE::CString& prefix, UInt32 maxCount );

    /** The terms of the vocabulary: kinds, categories, themes, styles, licence classes and restriction flags, with the mappings per source */
    virtual TAssetStoreStatus GetVocabulary( CAssetVocabularyInfo& result ) = 0;
    CORE::CFutureResult GetVocabularyAsync(  );

    /** The record of a pack, with its licence */
    virtual TAssetStoreStatus GetPack( const CORE::CString& packId, CAssetPackRecord& result ) = 0;
    CORE::CFutureResult GetPackAsync( const CORE::CString& packId );

    /** A page of the assets of a pack */
    virtual TAssetStoreStatus GetAssets( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRecordPage& result ) = 0;
    CORE::CFutureResult GetAssetsAsync( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries );

    /** The record of an asset with its typed properties and files */
    virtual TAssetStoreStatus GetAsset( const CORE::CString& assetId, CAssetRecord& result ) = 0;
    CORE::CFutureResult GetAssetAsync( const CORE::CString& assetId );

    /** A page of the relations of an asset, from it and to it */
    virtual TAssetStoreStatus GetRelations( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRelationPage& result ) = 0;
    CORE::CFutureResult GetRelationsAsync( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries );

    /** A page of the ingest history of a pack, of every pack for an empty id */
    virtual TAssetStoreStatus GetIngestHistory( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetIngestHistoryPage& result ) = 0;
    CORE::CFutureResult GetIngestHistoryAsync( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries );

    /** A page of the saved searches, the precanned ones included */
    virtual TAssetStoreStatus GetSavedSearches( const CORE::CString& pageToken, UInt32 maxEntries, CAssetSavedSearchPage& result ) = 0;
    CORE::CFutureResult GetSavedSearchesAsync( const CORE::CString& pageToken, UInt32 maxEntries );

    /** Adds or replaces a saved search; result.id is its id */
    virtual TAssetStoreStatus SaveSearch( const CORE::CString& commandId, const CAssetSavedSearch& search, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SaveSearchAsync( const CORE::CString& commandId, const CAssetSavedSearch& search );

    /** Removes a saved search */
    virtual TAssetStoreStatus DeleteSavedSearch( const CORE::CString& commandId, const CORE::CString& searchId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult DeleteSavedSearchAsync( const CORE::CString& commandId, const CORE::CString& searchId );

    /** A page of the results of a saved search */
    virtual TAssetStoreStatus RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& result ) = 0;
    CORE::CFutureResult RunSavedSearchAsync( const CORE::CString& searchId, UInt32 pageIndex );

    /** The files an asset needs to be opened: its own files, the files of the assets it refers to, the VFS paths of its original and its converted file with their availability */
    virtual TAssetStoreStatus GetAssetFileSet( const CORE::CString& assetId, CAssetFileSet& result ) = 0;
    CORE::CFutureResult GetAssetFileSetAsync( const CORE::CString& assetId );

    /** A licence by its id, or the licence of the pack with the id */
    virtual TAssetStoreStatus GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result ) = 0;
    CORE::CFutureResult GetLicenceAsync( const CORE::CString& licenceOrPackId );

    /**
     *  The pack index of a pack (section 6 of the pack index design): the row of its index in the catalogue with the summaries of
     *  its groups and, when the document can be read, its manifest (identity, producer, profile, licence flags, counts, shard
     *  list) without the records. NOT_FOUND for a pack that has no index (or that the caller may not see). 'forLearning' says the
     *  caller is a machine learning consumer: a pack whose licence carries noMachineLearning is then not there for it (NOT_FOUND),
     *  here and in the two operations below; a remote host sets it itself for the devices of the learning roles, whatever the
     *  client says. Implementations without a pack index answer UNSUPPORTED (the default).
     */
    virtual TAssetStoreStatus GetPackIndex( const CORE::CString& packId, bool forLearning, CAssetPackIndexInfo& result );
    CORE::CFutureResult GetPackIndexAsync( const CORE::CString& packId, bool forLearning );

    /** The group summaries of the pack index of a pack: the function groups in their fixed order, each followed by its sub-groups; the revision is that of the catalogue */
    virtual TAssetStoreStatus GetResourceGroups( const CORE::CString& packId, bool forLearning, CAssetPackGroupPage& result );
    CORE::CFutureResult GetResourceGroupsAsync( const CORE::CString& packId, bool forLearning );

    /**
     *  A page of the resources of the pack index of a pack in the canonical order (group in the fixed order, sub-group, path); with
     *  a group id only those of the group, with a sub-group as well only those of it. Paged like every list; every page carries the
     *  catalogue revision it was read at.
     */
    virtual TAssetStoreStatus GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries, bool forLearning, CAssetPackResourcePage& result );
    CORE::CFutureResult GetResourcesAsync( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, const CORE::CString& pageToken, UInt32 maxEntries, bool forLearning );

    // Curation

    /** Changes what a person may change of a pack (title, categories, themes, styles, keywords, licence class and flags), refused when the catalogue is at another revision than expected (0: no check) */
    virtual TAssetStoreStatus UpdatePackMetadata( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult UpdatePackMetadataAsync( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision );

    /** The classification of one pack or asset, with the source of the decision ("person", "llm-assisted") */
    virtual TAssetStoreStatus SetClassification( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetClassificationAsync( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source );

    /** Adds or replaces the mapping of a store category */
    virtual TAssetStoreStatus SetCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetCategoryMappingAsync( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind );

    /** Removes the mapping of a store category */
    virtual TAssetStoreStatus RemoveCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RemoveCategoryMappingAsync( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory );

    /** Adds or replaces a synonym; an empty replacement makes the word a stop word */
    virtual TAssetStoreStatus SetSynonym( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetSynonymAsync( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement );

    /** Adds or replaces a category rule */
    virtual TAssetStoreStatus SetCategoryRule( const CORE::CString& commandId, const CAssetCategoryRule& rule, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetCategoryRuleAsync( const CORE::CString& commandId, const CAssetCategoryRule& rule );

    /** Removes a category rule */
    virtual TAssetStoreStatus RemoveCategoryRule( const CORE::CString& commandId, const CORE::CString& ruleId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RemoveCategoryRuleAsync( const CORE::CString& commandId, const CORE::CString& ruleId );

    /** How many packs and assets the rule would classify now (document attributes "packs" and "assets"), for a preview */
    virtual TAssetStoreStatus CountRuleMatches( const CAssetCategoryRule& rule, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult CountRuleMatchesAsync( const CAssetCategoryRule& rule );

    /** Applies the rules and mappings again to the packs (every pack when none are given), as a job: result.id is the job */
    virtual TAssetStoreStatus Reclassify( const CORE::CString& commandId, const CORE::CString::StringVector& packIds, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult ReclassifyAsync( const CORE::CString& commandId, const CORE::CString::StringVector& packIds );

    /** Takes a pack out of the catalogue; with deleteFiles its placed files go as well */
    virtual TAssetStoreStatus RemovePack( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RemovePackAsync( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles );

    // Thumbnails

    /** The thumbnails of the assets at the size (128, 256 or 512), at most 64 per call; an asset without one is an entry with the status NOT_FOUND */
    virtual TAssetStoreStatus GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result ) = 0;
    CORE::CFutureResult GetThumbnailsAsync( const CORE::CString::StringVector& assetIds, UInt32 size );

    /** Stores a thumbnail or contact sheet a renderer made */
    virtual TAssetStoreStatus PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult PutThumbnailAsync( const CORE::CString& commandId, const CAssetThumbnail& thumbnail );

    /** The work list of a renderer: assets that have no thumbnail of the renderer version at the size (entries without pixels) */
    virtual TAssetStoreStatus GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result ) = 0;
    CORE::CFutureResult GetMissingThumbnailsAsync( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries );

    // Files

    /** Starts an upload of a pack from the caller's disk; result.id is the upload */
    virtual TAssetStoreStatus BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult BeginUploadAsync( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount );

    /**
     *  Starts an upload with its file list: each entry the relative path in the upload (path), the size (sizeBytes) and
     *  the SHA-256 digest (digest, 64 lower case hexadecimal characters). The service checks the digests when the upload
     *  is committed; result.id is the upload. Implementations without uploads answer UNSUPPORTED (the default)
     */
    virtual TAssetStoreStatus BeginUploadFiles( const CORE::CString& commandId, const CORE::CString& name, const CAssetFileStatPage& files, CAssetServiceResult& result );
    CORE::CFutureResult BeginUploadFilesAsync( const CORE::CString& commandId, const CORE::CString& name, const CAssetFileStatPage& files );

    /** The files of an upload with the bytes received of each */
    virtual TAssetStoreStatus QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result ) = 0;
    CORE::CFutureResult QueryUploadAsync( const CORE::CString& uploadId );

    /** Bytes of a file of an upload, at the offset in the file with the relative path */
    virtual TAssetStoreStatus WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result ) = 0;
    virtual CORE::CFutureResult WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data ) = 0;

    /** Ends an upload: the digests are checked and the ingest starts; result.id is the ingest */
    virtual TAssetStoreStatus CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult CommitUploadAsync( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest );

    /** Drops an upload and what it received */
    virtual TAssetStoreStatus AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult AbortUploadAsync( const CORE::CString& commandId, const CORE::CString& uploadId );

    // Locations

    /** The storage locations with their state */
    virtual TAssetStoreStatus GetLocations( CAssetLocationPage& result ) = 0;
    CORE::CFutureResult GetLocationsAsync(  );

    /** Checks which locations are reachable (LocationStateChangedEvent for those that changed) and answers them */
    virtual TAssetStoreStatus RefreshLocationStates( CAssetLocationPage& result ) = 0;
    CORE::CFutureResult RefreshLocationStatesAsync(  );

    /** Moves a location to another root */
    virtual TAssetStoreStatus RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RelocateLocationAsync( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath );

    /** Checks every file of every pack of the location, its existence and its digest */
    virtual TAssetStoreStatus VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result ) = 0;
    CORE::CFutureResult VerifyLocationAsync( const CORE::CString& commandId, const CORE::CString& locationId );

    // Ingest

    /** Ingests a pack (a VFS folder or archive of the service); DUPLICATE is an outcome, not a failure: the asynchronous form completes with it */
    virtual TAssetStoreStatus Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result ) = 0;
    CORE::CFutureResult IngestAsync( const CORE::CString& commandId, const CAssetIngestRequest& request );

    /** Continues an ingest from the step that failed */
    virtual TAssetStoreStatus ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result ) = 0;
    CORE::CFutureResult ResumeIngestAsync( const CORE::CString& commandId, const CORE::CString& ingestId );

    /** The ids of the ingests that are staged and did not finish */
    virtual TAssetStoreStatus GetPendingIngests( CAssetServiceStringList& result ) = 0;
    CORE::CFutureResult GetPendingIngestsAsync(  );

    /** The folders of the service that are offered for ingest by path, as "<rootId>=<VFS path>" */
    virtual TAssetStoreStatus ListImportRoots( CAssetServiceStringList& result ) = 0;
    CORE::CFutureResult ListImportRootsAsync(  );

    /** Adds or replaces the ingest policy of a source */
    virtual TAssetStoreStatus SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetSourcePolicyAsync( const CORE::CString& commandId, const CAssetSourcePolicy& policy );

    // Jobs

    /** A page of the jobs in the order they are shown: the downloads in the order of the queue, then the ingests, the conversions and the pipeline jobs in the order they started */
    virtual TAssetStoreStatus ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result ) = 0;
    CORE::CFutureResult ListJobsAsync( const CORE::CString& pageToken, UInt32 maxEntries );

    /** One job */
    virtual TAssetStoreStatus GetJob( const CORE::CString& jobId, CAssetJobEntry& result ) = 0;
    CORE::CFutureResult GetJobAsync( const CORE::CString& jobId );

    /** Pauses, resumes, cancels, retries or removes a job; the state that follows comes with JobChangedEvent */
    virtual TAssetStoreStatus ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult ControlJobAsync( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action );

    /** The result of a job that ended, bounded: the summary, or a page of a section ("failures", "approximations", "assets") */
    virtual TAssetStoreStatus GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult GetJobResultAsync( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries );

    // Stores

    /** The online stores with their sign in state */
    virtual TAssetStoreStatus ListStores( CAssetStoreInfoPage& result ) = 0;
    CORE::CFutureResult ListStoresAsync(  );

    /** A page of the library of the signed in account of an online store */
    virtual TAssetStoreStatus QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result ) = 0;
    CORE::CFutureResult QueryLibraryAsync( const CORE::CString& storeId, const CAssetStoreQuery& query );

    /** The details of an item of an online store, with its formats and files */
    virtual TAssetStoreStatus GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result ) = 0;
    CORE::CFutureResult GetStoreItemAsync( const CORE::CString& storeId, const CORE::CString& itemId );

    /** Queues a download; for the primary store the ingest follows when it is done (by the service, or by the store for a source with the automatic policy); result.id is the job */
    virtual TAssetStoreStatus EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult EnqueueDownloadAsync( const CORE::CString& commandId, const CAssetDownloadRequest& request );

    // SignIn

    /** Starts the sign in of a store on the service; the steps of the person come as SignInPromptEvent */
    virtual TAssetStoreStatus BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult BeginSignInAsync( const CORE::CString& commandId, const CORE::CString& storeId );

    /** Stops a sign in that runs */
    virtual TAssetStoreStatus CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult CancelSignInAsync( const CORE::CString& commandId, const CORE::CString& storeId );

    /** Forgets the session of a store */
    virtual TAssetStoreStatus SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SignOutAsync( const CORE::CString& commandId, const CORE::CString& storeId );

    /** The prompts that wait for the person */
    virtual TAssetStoreStatus GetSignInPrompts( CAssetSignInPromptPage& result ) = 0;
    CORE::CFutureResult GetSignInPromptsAsync(  );

    /** Answers a prompt of a sign in; the answer is a secret and never logged */
    virtual TAssetStoreStatus AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result ) = 0;
    virtual CORE::CFutureResult AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer ) = 0;

    /**
     *  Takes over the web session of a store that the account holder signed in to in their own browser on another machine (a store
     *  whose sign in has a step only a person can do, such as a captcha): 'session' is the session as one secret
     *  (AUTH::CWebSession::ToSecret(): the cookies with their domain, path, expiry and flags, the user agent and the time
     *  it was taken). Only the cookies of the store's own domains are kept, sealed in the session store of the service; the
     *  session is never logged, never part of an event or an answer, and no read gives it back. The store may check it
     *  with a request of its own and refuse it (NOT_SIGNED_IN). result.id is the store id, result.message says how many
     *  cookies were taken and what the store said. A sync that waits for a sign in of the store goes on. On a remote
     *  service only a device with the owner role may call it. Implementations without it answer UNSUPPORTED (the default)
     */
    virtual TAssetStoreStatus ImportWebSession( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session, CAssetServiceResult& result );
    virtual CORE::CFutureResult ImportWebSessionAsync( const CORE::CString& commandId, const CORE::CString& storeId, const AUTH::CSecretString& session );

    // Conversion

    /** Converts a pack (or every pack that is conversion pending) as a job: result.id is the job, its progress and end come with JobChangedEvent */
    virtual TAssetStoreStatus RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RequestConversionAsync( const CORE::CString& commandId, const CAssetConversionJobRequest& request );

    /** Makes a self-contained folder of the assets for use outside the catalogue, as a job */
    virtual TAssetStoreStatus RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RequestExportAsync( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format );

    // Pipeline

    /** A read operation of the converter pipeline (CAssetPipelineOperations), or the preview of a write (request.apply false): carried out by the converter hook */
    virtual TAssetStoreStatus RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult RunPipelineQueryAsync( const CAssetPipelineRequest& request );

    /** Applies a write of the pipeline (a manifest or an override) with the change token of its preview */
    virtual TAssetStoreStatus ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult ApplyPipelineChangeAsync( const CORE::CString& commandId, const CAssetPipelineRequest& request );

    /** Starts a job of the pipeline (convert, cook, validate): result.id is the job; GetJobResult() gives its result */
    virtual TAssetStoreStatus StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult StartPipelineJobAsync( const CORE::CString& commandId, const CAssetPipelineRequest& request );

    /** Renders contact sheets of the targets of the request through the render hook */
    virtual TAssetStoreStatus RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult RenderSheetAsync( const CAssetPipelineRequest& request );

    // LocalCopy

    /** Copies a pack, the file set of assets or an export to a folder of the caller, as a job */
    virtual TAssetStoreStatus RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult RequestLocalCopyAsync( const CORE::CString& commandId, const CAssetLocalCopyRequest& request );

    /** Keeps the files of a pack available without the service (or no longer) */
    virtual TAssetStoreStatus SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult SetOfflinePinAsync( const CORE::CString& commandId, const CORE::CString& packId, bool pinned );

    // Admin

    /** Locks the keyring of the saved store passwords */
    virtual TAssetStoreStatus LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult LockKeyringAsync( const CORE::CString& commandId );

    /** Unlocks the keyring of the saved store passwords with its code, which is never logged */
    virtual TAssetStoreStatus UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result ) = 0;
    virtual CORE::CFutureResult UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code ) = 0;

    /** A page of the audit records */
    virtual TAssetStoreStatus GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult GetAuditRecordsAsync( const CORE::CString& pageToken, UInt32 maxEntries );

    /** Rebuilds the catalogue from the pack records kept with the packs, as a job */
    virtual TAssetStoreStatus ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result ) = 0;
    CORE::CFutureResult ReindexFromStorageAsync( const CORE::CString& commandId );

    /** Counts of what the service holds and does: operations under way, jobs, caches */
    virtual TAssetStoreStatus GetResourceStatus( CAssetServiceDocument& result ) = 0;
    CORE::CFutureResult GetResourceStatusAsync(  );

    /*
     *  Sync: the persistent state of the library sync (library sync design, section 7.1). A run or a dry run of the sync of
     *  a store is the pipeline job "sync-library" (StartPipelineJob() with a CAssetLibrarySyncSpec as the arguments); its
     *  progress is that of every job (JobChangedEvent, GetJob(), ListJobs()), its plan and its lines are the sections "plan"
     *  and "items" of GetJobResult(). Implementations without the library sync answer UNSUPPORTED (the default).
     */

    /** A page of the ignore and pin marks of the items of a store */
    virtual TAssetStoreStatus GetSyncMarks( const CORE::CString& storeId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetSyncMarkPage& result );
    CORE::CFutureResult GetSyncMarksAsync( const CORE::CString& storeId, const CORE::CString& pageToken, UInt32 maxEntries );

    /**
     *  Sets the marks of items of a store: an entry with the mark "ignore" or "pin" sets it (with its reason), an entry
     *  without a mark removes the mark of its item. The store id of an entry is that of the call when the entry has none
     */
    virtual TAssetStoreStatus SetSyncMarks( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetSyncMarkPage& marks, CAssetServiceResult& result );
    CORE::CFutureResult SetSyncMarksAsync( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetSyncMarkPage& marks );

    /** The sync profile the service keeps for a store: the defaults when none was set */
    virtual TAssetStoreStatus GetSyncProfile( const CORE::CString& storeId, CAssetLibrarySyncProfile& result );
    CORE::CFutureResult GetSyncProfileAsync( const CORE::CString& storeId );

    /** Replaces the sync profile of a store; kept over a restart of the service. A sync whose spec says useStoreProfile runs with it */
    virtual TAssetStoreStatus SetSyncProfile( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetLibrarySyncProfile& profile, CAssetServiceResult& result );
    CORE::CFutureResult SetSyncProfileAsync( const CORE::CString& commandId, const CORE::CString& storeId, const CAssetLibrarySyncProfile& profile );

    /*
     *  Settings: the run-time settings of the service (gucefASSETSTORE_RemoteService_Design.md 18). The request says who asks; the host of
     *  the remote service sets that from the device, an in-process caller without a host is the site administrator. Implementations
     *  without settings answer UNSUPPORTED (the default).
     */

    /** The settings the caller may read with their values, where the values come from and whether the caller may change them */
    virtual TAssetStoreStatus DescribeSettings( const CAssetSettingsRequest& request, CAssetSettingsDescription& result );
    CORE::CFutureResult DescribeSettingsAsync( const CAssetSettingsRequest& request );

    /**
     *  Changes the settings of request.values: every key and value is checked first, and one that is not valid changes nothing
     *  (INVALID_ARGUMENT with the reason per key in result.errors); a call that changes nothing answers OK with result.noChange.
     *  SettingsChangedEvent follows a change
     */
    virtual TAssetStoreStatus SetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result );
    CORE::CFutureResult SetSettingsAsync( const CORE::CString& commandId, const CAssetSettingsRequest& request );

    /** The settings of request.keys go back to the value of the site settings; checked and applied as SetSettings() */
    virtual TAssetStoreStatus ResetSettings( const CORE::CString& commandId, const CAssetSettingsRequest& request, CAssetSettingsChangeResult& result );
    CORE::CFutureResult ResetSettingsAsync( const CORE::CString& commandId, const CAssetSettingsRequest& request );

    /**
     *  Writes the refusal of a change of the settings that a host refused itself (a device without the role) to the audit of the service.
     *  Not an operation of the wire: the host tells the service it serves. The default keeps nothing
     */
    virtual void AuditSettingsRefusal( const CAssetSettingsRequest& request, const CORE::CString& operation, const CORE::CString& reason );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIAssetStoreService( const CIAssetStoreService& src );              /**< not implemented */
    CIAssetStoreService& operator=( const CIAssetStoreService& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactoryWithParam< CORE::CString, CIAssetStoreService, CAssetStoreServiceConfig, MT::CMutex >   CAssetStoreServiceFactory;
typedef CAssetStoreServiceFactory::TProductPtr                                                                       TAssetStoreServicePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */
