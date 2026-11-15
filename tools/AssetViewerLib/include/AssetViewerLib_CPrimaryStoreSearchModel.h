/*
 *  AssetViewerLib: the asset sources of the viewer: online stores, the primary store and the transfers
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
#ifndef ASSETVIEWERLIB_CPRIMARYSTORESEARCHMODEL_H
#define ASSETVIEWERLIB_CPRIMARYSTORESEARCHMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_GUI_CISEARCHMODEL_H
#include "gucefGUI_CISearchModel.h"
#define GUCEF_GUI_CISEARCHMODEL_H
#endif /* GUCEF_GUI_CISEARCHMODEL_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  The search of the primary store for a CAssetSearchForm: it describes the filters the catalogue has (kind, category, source,
 *  licence class, format, rigged / animated / LODs / collision, what the licence allows, and the ranges of the size in MB, the
 *  triangles and the texture size), asks the store for results as asynchronous searches and answers with the page and the
 *  facets, suggests the keywords of the index and keeps the saved searches of the store.
 *
 *  The search form's generic query is mapped onto ASSETSTORE::CAssetSearchQuery. What the form does not edit (themes, art styles, pack states, the
 *  roles of textures) rides along in the 'extra' of the query: a saved search of the store comes out as a generic query whose
 *  extra holds the whole store query, and a search starts from the extra and applies what the form holds on top of it.
 *
 *  Update() has to be called between two frames: it takes the answers of the searches that finished and raises SearchCompletedEvent
 *  on the calling thread. A result row is keyed by the id of its asset, or "pack:" and the id of its pack for a result that is a
 *  pack; its thumbnail key is the VFS path of its file.
 *
 *  Everything goes through the asset store service (ASSETSTORE::CIAssetStoreService), which is not owned and has to outlive the
 *  model. With a service in this process the vocabulary, the suggestions and the saved searches are asked of it when they are
 *  needed, as the store itself answers them. With a service that is not in process nothing waits on the GUI thread (section 5.1
 *  of the service design): the vocabulary and the saved searches are cached, asked for at the start and again when the
 *  catalogue says they changed (FilterDescriptionChangedEvent and SavedSearchesChangedEvent follow); suggestions come from a
 *  cache by prefix (512 prefixes, the least recently used goes first, cleared by a change of the vocabulary), a prefix that is
 *  not there is asked for in the background and SuggestionsChangedEvent tells the form when its answer arrived; a saved search
 *  is changed in the cache at once and the list of the service replaces it when the service refused.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CPrimaryStoreSearchModel : public GUI::CISearchModel
{
    public:

    static const CORE::CString PackKeyPrefix;       /**< "pack:" */

    typedef CORE::CTEventHandlerFunctor< CPrimaryStoreSearchModel > TEventCallback;

    static const UInt32 SuggestionCacheSize = 512;

    explicit CPrimaryStoreSearchModel( ASSETSTORE::CIAssetStoreService& service );

    virtual ~CPrimaryStoreSearchModel() GUCEF_VIRTUAL_OVERRIDE;

    /** The ids of the sources the source filter offers besides "disk" (the online stores that are registered); changing them changes the description */
    void SetSourceIds( const CORE::CString::StringVector& sourceIds );

    /** Takes the answers of finished searches (and of the vocabulary, suggestions and saved searches of a service that is not in process); the number of searches that was answered */
    UInt32 Update( void );

    /** The number of searches that have not been answered */
    UInt32 GetPendingCount( void ) const;

    /** The number of prefixes whose suggestions are cached, and of those asked for that did not arrive yet */
    UInt32 GetCachedSuggestionCount( void ) const;
    UInt32 GetPendingSuggestionCount( void ) const;

    /** The hit behind a key of a result row of the last answer, false when the key is not known */
    bool FindHit( const CORE::CString& key, ASSETSTORE::CAssetSearchHit& hit ) const;

    /** The store query of a generic query, and the generic query of a store query */
    static void ToStoreQuery( const GUI::CSearchQuery& query, ASSETSTORE::CAssetSearchQuery& storeQuery );
    static void FromStoreQuery( const ASSETSTORE::CAssetSearchQuery& storeQuery, GUI::CSearchQuery& query );

    virtual void GetFilterDescription( GUI::CSearchFilterDescription& description ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 RequestSearch( const GUI::CSearchQuery& query ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetResultColumns( TColumnVector& columns ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSortChoices( TSortChoiceVector& choices ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSuggestions( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& suggestions ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SupportsSavedSearches( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSavedSearches( GUI::TSavedSearchVector& searches ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveSearch( const GUI::CSavedSearch& search ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool DeleteSavedSearch( const CORE::CString& searchId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SPending
    {
        UInt32 requestId;
        CORE::CFutureResult future;

        SPending( UInt32 id, const CORE::CFutureResult& result ) : requestId( id ), future( result ) {}
    };
    typedef GUCEF::vector< SPending > TPendingVector;
    typedef GUCEF::map< CORE::CString, ASSETSTORE::CAssetSearchHit > THitMap;

    struct SSuggestRequest
    {
        CORE::CString prefix;
        CORE::CFutureResult future;

        SSuggestRequest( const CORE::CString& text, const CORE::CFutureResult& result ) : prefix( text ), future( result ) {}
    };
    typedef GUCEF::vector< SSuggestRequest > TSuggestRequestVector;
    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector > TSuggestionMap;

    CPrimaryStoreSearchModel( const CPrimaryStoreSearchModel& src );
    CPrimaryStoreSearchModel& operator=( const CPrimaryStoreSearchModel& src );

    void Complete( UInt32 requestId, const CORE::CFutureResult& future );
    void OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void RequestVocabulary( void );
    void RequestSavedSearches( void );
    UInt32 TakeBackgroundAnswers( void );
    void ToGenericSavedSearches( const ASSETSTORE::TAssetSavedSearchVector& stored, GUI::TSavedSearchVector& searches ) const;

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    CORE::CString::StringVector m_sourceIds;
    ASSETSTORE::CAssetVocabularyInfo m_vocabulary;
    bool m_haveVocabulary;
    CORE::CFutureResult m_vocabularyFuture;
    ASSETSTORE::TAssetSavedSearchVector m_savedSearches;
    CORE::CFutureResult m_savedSearchesFuture;
    GUCEF::vector< CORE::CFutureResult > m_savedSearchChanges;
    mutable TSuggestionMap m_suggestions;
    mutable CORE::CString::StringVector m_suggestionOrder;
    mutable TSuggestRequestVector m_suggestRequests;
    TPendingVector m_pending;
    THitMap m_hits;
    UInt32 m_nextRequestId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CPRIMARYSTORESEARCHMODEL_H ? */
