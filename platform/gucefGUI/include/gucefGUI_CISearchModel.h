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
#ifndef GUCEF_GUI_CISEARCHMODEL_H
#define GUCEF_GUI_CISEARCHMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_CSEARCHQUERY_H
#include "gucefGUI_CSearchQuery.h"
#define GUCEF_GUI_CSEARCHQUERY_H
#endif /* GUCEF_GUI_CSEARCHQUERY_H ? */

#ifndef GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#include "gucefGUI_CSearchFilterDescription.h"
#define GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#endif /* GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H ? */

#ifndef GUCEF_GUI_CSEARCHRESULTPAGE_H
#include "gucefGUI_CSearchResultPage.h"
#define GUCEF_GUI_CSEARCHRESULTPAGE_H
#endif /* GUCEF_GUI_CSEARCHRESULTPAGE_H ? */

#ifndef GUCEF_GUI_CSAVEDSEARCH_H
#include "gucefGUI_CSavedSearch.h"
#define GUCEF_GUI_CSAVEDSEARCH_H
#endif /* GUCEF_GUI_CSAVEDSEARCH_H ? */

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
 *  A catalogue that a CAssetSearchForm searches: it describes the filters that make sense for it, runs a search for a
 *  CSearchQuery and answers with a page of results with the facet counts, gives suggestions for the text being typed and
 *  keeps saved searches. The application implements it on its database, index or file listing; the form knows nothing of
 *  what is in it.
 *
 *  A search is asked for with RequestSearch(), which returns the number of the request; the model answers with
 *  SearchCompletedEvent (the request number, whether it worked, a message and the page), raised on the thread the GUI runs
 *  on, either before RequestSearch() returns (a model that searches at once) or later (a search on a pool thread whose
 *  future the model checks every frame). The form shows only the answer of the request it asked for last: an answer of an
 *  earlier request is ignored, so the model need not cancel what it started.
 */
class GUCEF_GUI_PUBLIC_CPP CISearchModel : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent SearchCompletedEvent;             /**< TSearchCompletedEventData */
    static const CORE::CEvent FilterDescriptionChangedEvent;    /**< no data: GetFilterDescription() answers something else now */
    static const CORE::CEvent SavedSearchesChangedEvent;        /**< no data: the saved searches changed */
    static const CORE::CEvent SuggestionsChangedEvent;          /**< no data: GetSuggestions() may answer more now (an answer it waited for arrived); the form asks again for the word being typed */

    static void RegisterEvents( void );

    /** What SearchCompletedEvent carries */
    struct GUCEF_GUI_PUBLIC_CPP SSearchCompletion
    {
        UInt32 requestId;
        bool succeeded;
        CString message;                // why a search failed; may hold a note for a search that worked
        CSearchResultPage page;

        SSearchCompletion( void );
    };
    typedef struct SSearchCompletion TSearchCompletion;
    typedef CORE::CTCloneableObj< TSearchCompletion > TSearchCompletedEventData;

    /** A column of the result grid */
    struct GUCEF_GUI_PUBLIC_CPP SColumn
    {
        CString key;                    /**< the key of the values of a result row */
        CString title;
        Int32 width;                    /**< relative to the widths of the other columns */

        SColumn( void );
        SColumn( const CString& columnKey, const CString& columnTitle, Int32 columnWidth );
    };
    typedef struct SColumn TColumn;
    typedef GUCEF::vector< TColumn > TColumnVector;

    /** One way of ordering the results */
    struct GUCEF_GUI_PUBLIC_CPP SSortChoice
    {
        CString id;
        CString title;

        SSortChoice( void );
        SSortChoice( const CString& sortId, const CString& sortTitle );
    };
    typedef struct SSortChoice TSortChoice;
    typedef GUCEF::vector< TSortChoice > TSortChoiceVector;

    CISearchModel( void );

    virtual ~CISearchModel() GUCEF_VIRTUAL_OVERRIDE;

    /** The filters that make sense for this catalogue */
    virtual void GetFilterDescription( CSearchFilterDescription& description ) const = 0;

    /** Asks for a search; the number of the request, which the answer carries */
    virtual UInt32 RequestSearch( const CSearchQuery& query ) = 0;

    /** The columns of the result grid; by default the title and the subtitle of the rows */
    virtual void GetResultColumns( TColumnVector& columns ) const;

    /** The ways of ordering the results; by default none, and the form shows no sort choice */
    virtual void GetSortChoices( TSortChoiceVector& choices ) const;

    /**
     *  At most 'maxCount' words that continue the word the person is typing, shortest first; by default none. Quick: it is
     *  called while a person types. A model that has to ask elsewhere for a prefix (a remote service) answers what it has
     *  at once, asks in the background and raises SuggestionsChangedEvent when the answer arrived
     */
    virtual void GetSuggestions( const CString& prefix, UInt32 maxCount, CString::StringVector& suggestions ) const;

    /** Whether the model keeps saved searches (the form shows the list of them when it does); false by default */
    virtual bool SupportsSavedSearches( void ) const;

    virtual void GetSavedSearches( TSavedSearchVector& searches ) const;

    /** Keeps a search under its name: a new one when its id is empty, else it replaces the one with the id; false when the model did not */
    virtual bool SaveSearch( const CSavedSearch& search );

    /** Deletes a saved search that is not built in; false when the model did not */
    virtual bool DeleteSavedSearch( const CString& searchId );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CISearchModel( const CISearchModel& src );
    CISearchModel& operator=( const CISearchModel& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CISEARCHMODEL_H ? */
