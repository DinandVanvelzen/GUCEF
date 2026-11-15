/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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
#ifndef GUCEF_GUI_CASSETSEARCHFORM_H
#define GUCEF_GUI_CASSETSEARCHFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CISEARCHMODEL_H
#include "gucefGUI_CISearchModel.h"
#define GUCEF_GUI_CISEARCHMODEL_H
#endif /* GUCEF_GUI_CISEARCHMODEL_H ? */

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/**
 *  A search of a catalogue: a search field with suggestions, a panel of filters built from what the model describes (choices
 *  with the counts of the results, ranges of numbers), saved searches, the results as a grid or as thumbnails, and paging.
 *  It works on a CISearchModel, so any catalogue can use it; the form knows nothing of what the results are.
 *
 *  The text is committed to the query after TypingDelayInMs of frame time without typing, or at once with Enter; the filters
 *  that are choices commit at once, the ranges like the text. A change of the search (not of the page) goes back to the first
 *  page. Every committed query is notified with QueryChangedEvent (a CSearchQuery) and asked of the model; the form shows the
 *  answer of the last request only. The words of the text that the person is typing ask the model for suggestions, shown in
 *  a list below the field; a click on one replaces the word.
 *
 *  The results come with the columns of the model; a double click or Enter on a result notifies ResultOpenedEvent with its key,
 *  a selection ResultSelectedEvent (the key, empty when the selection ended). The thumbnail view shows the pictures that a
 *  CIThumbnailProvider (SetThumbnailProvider()) makes of the thumbnail keys of the rows. A saved search that is chosen is
 *  put in the filters and run, and notified with SavedSearchChosenEvent (its id). The form builds its layout in code, as a
 *  window (Build()) or in the place of a widget of another form (Build( placeholder )), with the filters beside the results or,
 *  for a narrow place, above them (SetFilterPlacement() before Build()).
 */
class GUCEF_GUI_PUBLIC_CPP CAssetSearchForm : public CFormEx
{
    public:

    typedef CORE::CTCloneableObj< CSearchQuery > TQueryChangedEventData;

    static const CORE::CEvent QueryChangedEvent;            /**< TQueryChangedEventData: a search was asked of the model */
    static const CORE::CEvent ResultOpenedEvent;            /**< TCloneableString: the key of the result that was opened (double click, Enter) */
    static const CORE::CEvent ResultSelectedEvent;          /**< TCloneableString: the key of the result that was selected; empty when no result is selected any more */
    static const CORE::CEvent SavedSearchChosenEvent;       /**< TCloneableString: the id of the saved search that the person chose */

    static void RegisterEvents( void );

    enum EFilterPlacement
    {
        FILTERPLACEMENT_LEFT = 0    ,   /**< beside the results */
        FILTERPLACEMENT_TOP             /**< above the results, for a narrow place */
    };

    enum EDefaults
    {
        TypingDelayInMs = 300       ,   /**< how long typing may rest before the text is searched, in milliseconds of frame time */
        DefaultPageSize = 50        ,
        DefaultThumbnailSize = 96   ,
        MaxSuggestions = 8
    };

    CAssetSearchForm( void );

    explicit CAssetSearchForm( CIGUIContext* context );

    virtual ~CAssetSearchForm();

    /** Where the filters are; takes effect when the form is built */
    void SetFilterPlacement( EFilterPlacement placement );

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Search" );

    /** Makes the form in the place of 'placeholder', which it fills */
    bool Build( CWidget* placeholder );

    /*
     *  Model
     */

    /** The catalogue to search (not owned, it has to outlive the form or be taken away with GUCEF_NULL); the form reads the filters and runs the search of the empty query */
    void SetModel( CISearchModel* model );

    CISearchModel* GetModel( void ) const;

    /** The makers of the pictures of the thumbnail view (not owned) */
    void SetThumbnailProvider( CIThumbnailProvider* provider );

    CIThumbnailProvider* GetThumbnailProvider( void ) const;

    /*
     *  The search
     */

    /** Puts the query in the widgets and searches; the text is committed at once */
    void SetQuery( const CSearchQuery& query );

    const CSearchQuery& GetQuery( void ) const;

    /** Sets the text of the search field and searches at once */
    void SetText( const CString& text );

    /** Checks or unchecks a choice of a filter group and searches */
    bool SetChoice( const CString& groupId, const CString& value, bool chosen );

    /** Sets the range of a filter group (the text boxes) and searches at once */
    bool SetRange( const CString& groupId, const CSearchQuery::TRange& range );

    /** Takes the text, the choices and the ranges away and searches */
    void ClearFilters( void );

    void SetPageSize( UInt32 pageSize );

    UInt32 GetPageSize( void ) const;

    bool GoToPage( UInt32 pageIndex );

    bool NextPage( void );

    bool PreviousPage( void );

    /** Asks the model again for the current query */
    void Search( void );

    /** The number of searches the form asked of the model */
    UInt32 GetRequestCount( void ) const;

    bool IsSearching( void ) const;

    /** What the status line says: the number of results, a note while it searches or the reason a search failed */
    CString GetStatusText( void ) const;

    /** Lets frame time pass: commits the typing and the range boxes that rested, and lays out the thumbnails */
    void Update( UInt32 deltaInMs );

    /** Commits what is typed at once */
    void CommitEdits( void );

    /*
     *  Results
     */

    const CSearchResultPage& GetPage( void ) const;

    UInt32 GetShownRowCount( void ) const;

    const CSearchResultPage::TRow& GetShownRow( UInt32 index ) const;

    /** Selects the result with the key; false when it is not on the page */
    bool SelectResult( const CString& key );

    const CString& GetSelectedKey( void ) const;

    /** Notifies ResultOpenedEvent for a result of the page; false when it is not there */
    bool OpenResult( const CString& key );

    void SetThumbnailView( bool thumbnails );

    bool IsThumbnailView( void ) const;

    void SetThumbnailSize( UInt32 size );

    UInt32 GetThumbnailSize( void ) const;

    UInt32 GetRealizedTileCount( void ) const;

    bool HasThumbnail( const CString& thumbnailKey ) const;

    void SetFiltersVisible( bool visible );

    bool AreFiltersVisible( void ) const;

    /** The text a choice of the filter panel shows now, with its count when the results have one */
    CString GetChoiceText( const CString& groupId, const CString& value ) const;

    /** The widget of a choice or GUCEF_NULL */
    CWidget* GetChoiceWidget( const CString& groupId, const CString& value ) const;

    /** The widgets of the range boxes of a group, GUCEF_NULL when it has none */
    CWidget* GetRangeMinimumWidget( const CString& groupId ) const;

    CWidget* GetRangeMaximumWidget( const CString& groupId ) const;

    /** The suggestions that are shown now */
    CString::StringVector GetSuggestionList( void ) const;

    /** Takes the suggestion at the index as the word that is being typed */
    bool ChooseSuggestion( UInt32 index );

    /*
     *  Saved searches
     */

    /** The names in the list of saved searches, after the first line that tells what the list is */
    CString::StringVector GetSavedSearchNames( void ) const;

    /** Puts the saved search in the filters and runs it, as when it is chosen in the list; false for an id the model does not have */
    bool ChooseSavedSearch( const CString& searchId );

    /** The id of the saved search that was chosen last and is still the query, empty otherwise */
    const CString& GetChosenSavedSearchId( void ) const;

    /** Keeps the current query under a name with the model; false when the model does not */
    bool SaveCurrentSearch( const CString& name );

    /** Deletes the chosen saved search when it is not built in; false when it cannot be */
    bool DeleteChosenSavedSearch( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAssetSearchForm > TEventCallback;

    struct SChoiceWidget
    {
        CWidget* widget;
        UInt32 group;
        UInt32 choice;
    };

    struct SRangeWidget
    {
        CWidget* minimum;
        CWidget* maximum;
        UInt32 group;
    };

    struct STile
    {
        CWidget* panel;
        Int32 row;
    };

    void OnQueryTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnQueryTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSavedValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSortValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFiltersChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFiltersUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTilesChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTilesUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDeleteActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPreviousActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSuggestionValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnChoiceChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnChoiceUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRangeTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRangeTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTilesMouseClick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSearchCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterDescriptionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSavedSearchesChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSuggestionsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnThumbnailReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSaveNameClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void ArrangeAreas( void );
    void ReadModel( void );
    void ShowColumns( void );
    void ShowSortChoices( void );
    void ShowSavedSearches( void );
    void BuildFilterPanel( void );
    void ClearFilterPanel( void );
    void ShowFacetCounts( void );
    void ShowQueryInWidgets( void );
    void ShowResults( void );
    void ShowGridRows( void );
    void ShowPaging( void );
    void ShowStatus( void );
    void ShowSuggestions( const CString& text );
    void HideSuggestions( void );
    void LayoutTiles( void );
    void BindTile( UInt32 slot, Int32 row, Int32 cellWidth, Int32 cellHeight, Int32 columns );
    void RequestTileThumbnails( void );
    void SelectRow( Int32 row, bool notify );
    void NotifyQueryChanged( void );
    void ChangeSearch( void );
    void ChangePage( UInt32 pageIndex );
    void RunSearch( void );
    bool ReadEdits( void );
    void MarkEditsPending( void );
    bool ChangeChoice( CORE::CNotifier* box, bool chosen );
    bool IsValidNumber( const CString& text, Float64& value ) const;
    Int32 FindRowOfKey( const CString& key ) const;
    Int32 FindSavedSearchIndex( const CString& searchId ) const;
    CString GetLastWord( const CString& text ) const;

    CAssetSearchForm( const CAssetSearchForm& src );
    CAssetSearchForm& operator=( const CAssetSearchForm& src );

    private:

    CISearchModel* m_model;
    CIThumbnailProvider* m_thumbnailProvider;
    EFilterPlacement m_placement;
    CSearchQuery m_query;
    CSearchResultPage m_page;
    CSearchFilterDescription m_description;
    CISearchModel::TColumnVector m_columns;
    CISearchModel::TSortChoiceVector m_sortChoices;
    TSavedSearchVector m_savedSearches;
    CString m_chosenSavedSearchId;
    GUCEF::vector< SChoiceWidget > m_choiceWidgets;
    GUCEF::vector< SRangeWidget > m_rangeWidgets;
    GUCEF::vector< CWidget* > m_filterWidgets;
    GUCEF::vector< STile > m_tiles;
    GUCEF::map< CString, IMAGE::TPixelMapPtr > m_thumbnails;
    GUCEF::set< CString > m_requestedThumbnails;
    CString::StringVector m_suggestions;
    CString m_selectedKey;
    CString m_statusNote;
    CMessageBoxForm* m_nameBox;
    UInt32 m_requestId;
    UInt32 m_requestCount;
    UInt32 m_pageSize;
    UInt32 m_thumbnailSize;
    UInt32 m_filterGeneration;
    UInt32 m_typedWaitMs;
    UInt32 m_tilesPerRow;
    Int32 m_lastViewportWidth;
    Int32 m_lastViewportHeight;
    Int32 m_lastScroll;
    bool m_inRequest;
    bool m_completedInRequest;
    bool m_searching;
    bool m_editsPending;
    bool m_thumbnailView;
    bool m_filtersVisible;
    bool m_tilesDirty;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
    bool m_haveResults;
    bool m_searchFailed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CASSETSEARCHFORM_H ? */
