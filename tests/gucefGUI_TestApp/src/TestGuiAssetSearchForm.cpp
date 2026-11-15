/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestAssetFormSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

using namespace GUITEST;

typedef GUI::CISearchModel TModel;

/** A catalogue in memory: 25 items of three kinds, keyword search, a choice group, a range group, facets, paging and saved searches */
class CTestSearchModel : public TModel
{
    public:

    struct SItem
    {
        CORE::CString key;
        CORE::CString title;
        CORE::CString kind;
        Int32 sizeKb;
        bool available;
    };

    GUCEF::vector< SItem > items;
    GUI::TSavedSearchVector saved;
    GUCEF::vector< GUI::CSearchQuery > requests;
    GUCEF::vector< TSearchCompletion > pending;
    bool deferred;
    bool failNext;
    bool savedSupported;
    UInt32 nextRequestId;
    UInt32 nextSavedId;

    CTestSearchModel( void ) : TModel(), items(), saved(), requests(), pending(), deferred( false ), failNext( false ), savedSupported( true ), nextRequestId( 1 ), nextSavedId( 1 )
    {
        static const char* const models[] = { "Sword", "Axe", "Barrel", "Crate" };
        for ( UInt32 m=0; m<4; ++m )
            for ( UInt32 n=1; n<=3; ++n )
                Add( CORE::CString( "model-" ) + CORE::UInt32ToString( m * 3 + n ), CORE::CString( models[ m ] ) + " " + CORE::UInt32ToString( n ), "model", (Int32) ( ( m * 3 + n ) * 100 ) );
        static const char* const textures[] = { "Rock Albedo", "Wood Albedo" };
        for ( UInt32 t=0; t<2; ++t )
            for ( UInt32 n=1; n<=4; ++n )
                Add( CORE::CString( "texture-" ) + CORE::UInt32ToString( t * 4 + n ), CORE::CString( textures[ t ] ) + " " + CORE::UInt32ToString( n ), "texture", (Int32) ( 2000 + n * 500 ) );
        static const char* const sounds[] = { "Sword Hit", "Axe Hit", "Ambient Wind", "Ambient Rain", "Music Calm" };
        for ( UInt32 s=0; s<5; ++s )
            Add( CORE::CString( "audio-" ) + CORE::UInt32ToString( s + 1 ), sounds[ s ], "audio", (Int32) ( 50 + s * 10 ) );
        items[ 3 ].available = false;
        GUI::CSavedSearch builtIn;
        builtIn.id = "s1";
        builtIn.name = "Textures";
        builtIn.builtIn = true;
        builtIn.query.AddChoice( "kind", "texture" );
        saved.push_back( builtIn );
        nextSavedId = 2;
    }

    void Add( const CORE::CString& key, const CORE::CString& title, const CORE::CString& kind, Int32 sizeKb )
    {
        SItem item;
        item.key = key;
        item.title = title;
        item.kind = kind;
        item.sizeKb = sizeKb;
        item.available = true;
        items.push_back( item );
    }

    virtual void GetFilterDescription( GUI::CSearchFilterDescription& description ) const GUCEF_VIRTUAL_OVERRIDE
    {
        description.Clear();
        description.AddChoiceGroup( "kind", "Kind" );
        description.AddChoice( "kind", "model", "Model" );
        description.AddChoice( "kind", "texture", "Texture" );
        description.AddChoice( "kind", "audio", "Audio" );
        description.AddRangeGroup( "size", "Size", "KB" );
    }

    virtual void GetResultColumns( TColumnVector& columns ) const GUCEF_VIRTUAL_OVERRIDE
    {
        columns.clear();
        columns.push_back( TColumn( "title", "Name", 50 ) );
        columns.push_back( TColumn( "subtitle", "Kind", 25 ) );
        columns.push_back( TColumn( "size", "Size", 25 ) );
    }

    virtual void GetSortChoices( TSortChoiceVector& choices ) const GUCEF_VIRTUAL_OVERRIDE
    {
        choices.clear();
        choices.push_back( TSortChoice( "name", "Name" ) );
        choices.push_back( TSortChoice( "size", "Size" ) );
    }

    /** Tells the form that the suggestions changed, as a model does whose answer for a prefix arrived later */
    void AnnounceSuggestions( void )
    {
        NotifyObservers( SuggestionsChangedEvent );
    }

    virtual void GetSuggestions( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& suggestions ) const GUCEF_VIRTUAL_OVERRIDE
    {
        suggestions.clear();
        GUCEF::set< CORE::CString > words;
        const CORE::CString lower = prefix.Lowercase();
        for ( size_t i=0; i<items.size(); ++i )
        {
            CORE::CString::StringVector parts = items[ i ].title.Lowercase().ParseElements( ' ', false );
            for ( size_t p=0; p<parts.size(); ++p )
                if ( parts[ p ].Length() > lower.Length() && 0 == parts[ p ].HasSubstr( lower ) )
                    words.insert( parts[ p ] );
        }
        for ( GUCEF::set< CORE::CString >::const_iterator w = words.begin(); w != words.end() && suggestions.size() < maxCount; ++w )
            suggestions.push_back( *w );
    }

    virtual bool SupportsSavedSearches( void ) const GUCEF_VIRTUAL_OVERRIDE { return savedSupported; }

    virtual void GetSavedSearches( GUI::TSavedSearchVector& searches ) const GUCEF_VIRTUAL_OVERRIDE { searches = saved; }

    virtual bool SaveSearch( const GUI::CSavedSearch& search ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUI::CSavedSearch copy = search;
        if ( copy.id.IsNULLOrEmpty() )
        {
            copy.id = "u" + CORE::UInt32ToString( nextSavedId++ );
            saved.push_back( copy );
        }
        else
        {
            for ( size_t i=0; i<saved.size(); ++i )
                if ( saved[ i ].id == copy.id )
                    saved[ i ] = copy;
        }
        NotifyObservers( SavedSearchesChangedEvent );
        return true;
    }

    virtual bool DeleteSavedSearch( const CORE::CString& searchId ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t i=0; i<saved.size(); ++i )
        {
            if ( saved[ i ].id == searchId && !saved[ i ].builtIn )
            {
                saved.erase( saved.begin() + i );
                NotifyObservers( SavedSearchesChangedEvent );
                return true;
            }
        }
        return false;
    }

    bool Matches( const SItem& item, const GUI::CSearchQuery& query ) const
    {
        const CORE::CString title = item.title.Lowercase();
        CORE::CString::StringVector words = query.text.Lowercase().ParseElements( ' ', false );
        for ( size_t w=0; w<words.size(); ++w )
            if ( !words[ w ].IsNULLOrEmpty() && title.HasSubstr( words[ w ] ) < 0 )
                return false;
        GUI::CSearchQuery::TChoiceMap::const_iterator kinds = query.choices.find( "kind" );
        if ( kinds != query.choices.end() && (*kinds).second.find( item.kind ) == (*kinds).second.end() )
            return false;
        const GUI::CSearchQuery::TRange range = query.GetRange( "size" );
        if ( range.hasMinimum && item.sizeKb < range.minimum )
            return false;
        if ( range.hasMaximum && item.sizeKb > range.maximum )
            return false;
        return true;
    }

    TSearchCompletion Search( const GUI::CSearchQuery& query, UInt32 requestId )
    {
        TSearchCompletion completion;
        completion.requestId = requestId;
        completion.succeeded = true;
        GUCEF::vector< const SItem* > found;
        for ( size_t i=0; i<items.size(); ++i )
            if ( Matches( items[ i ], query ) )
                found.push_back( &items[ i ] );
        if ( "size" == query.sort )
            for ( size_t a=0; a<found.size(); ++a )
                for ( size_t b=a+1; b<found.size(); ++b )
                    if ( found[ b ]->sizeKb > found[ a ]->sizeKb )
                        std::swap( found[ a ], found[ b ] );
        const UInt32 pageSize = query.pageSize > 0 ? query.pageSize : 10;
        completion.page.total = found.size();
        completion.page.pageIndex = query.pageIndex;
        completion.page.pageSize = pageSize;
        for ( size_t i=query.pageIndex * pageSize; i<found.size() && i<(size_t) ( query.pageIndex + 1 ) * pageSize; ++i )
        {
            GUI::CSearchResultPage::TRow row;
            row.key = found[ i ]->key;
            row.title = found[ i ]->title;
            row.subtitle = found[ i ]->kind;
            row.thumbnailKey = "thumb:" + found[ i ]->key;
            row.typeName = found[ i ]->kind;
            row.available = found[ i ]->available;
            row.values[ "size" ] = CORE::Int32ToString( found[ i ]->sizeKb ) + " KB";
            completion.page.rows.push_back( row );
        }
        for ( size_t i=0; i<found.size(); ++i )
            completion.page.facets[ "kind" ][ found[ i ]->kind ]++;
        return completion;
    }

    virtual UInt32 RequestSearch( const GUI::CSearchQuery& query ) GUCEF_VIRTUAL_OVERRIDE
    {
        requests.push_back( query );
        const UInt32 id = nextRequestId++;
        TSearchCompletion completion = Search( query, id );
        if ( failNext )
        {
            failNext = false;
            completion.succeeded = false;
            completion.message = "The catalogue is not reachable";
            completion.page.Clear();
        }
        if ( deferred )
            pending.push_back( completion );
        else
            Raise( completion );
        return id;
    }

    void ChangeDescription( void )
    {
        NotifyObservers( FilterDescriptionChangedEvent );
    }

    void ChangeSaved( void )
    {
        NotifyObservers( SavedSearchesChangedEvent );
    }

    void Raise( const TSearchCompletion& completion )
    {
        TSearchCompletedEventData data( completion );
        NotifyObservers( SearchCompletedEvent, &data );
    }

    void RaiseAll( bool reversed )
    {
        GUCEF::vector< TSearchCompletion > list = pending;
        pending.clear();
        if ( reversed )
            for ( size_t i=list.size(); i>0; --i )
                Raise( list[ i - 1 ] );
        else
            for ( size_t i=0; i<list.size(); ++i )
                Raise( list[ i ] );
    }
};

/** Makes a context and a form with the model, built in the place of a widget of a screen */
struct SFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUI::CWidget* screen;
    GUI::CWidget* placeholder;
    CTestSearchModel model;
    GUI::CAssetSearchForm* form;

    SFixture( bool top = false, bool attachModel = true ) : contextPtr( CreateFormContext() ), context( GUCEF_NULL ), screen( GUCEF_NULL ), placeholder( GUCEF_NULL ), model(), form( GUCEF_NULL )
    {
        context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        if ( GUCEF_NULL == context )
            return;
        screen = MakeScreen( context );
        placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,640 0,480" );
        form = new GUI::CAssetSearchForm( context );
        if ( top )
            form->SetFilterPlacement( GUI::CAssetSearchForm::FILTERPLACEMENT_TOP );
        form->Build( placeholder );
        if ( attachModel )
            form->SetModel( &model );
        context->RenderFrame( 16 );
    }

    ~SFixture()
    {
        delete form;
    }
};

class CSearchScene : public CFormScene
{
    public:

    CTestSearchModel model;
    GUI::CAssetSearchForm form;

    explicit CSearchScene( GUI::CGUIContext* context ) : CFormScene(), model(), form( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        form.SetModel( &model );
        form.SetPageSize( 8 );
        form.SetChoice( "kind", "model", true );
        GUI::CSearchQuery::TRange range;
        range.hasMinimum = true;
        range.minimum = 200;
        form.SetRange( "size", range );
        form.SelectResult( "model-4" );
        context->RenderFrame( 16 );
    }
};

CFormScene*
BuildSearchScene( GUI::CGUIContext* context )
{
    return new CSearchScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiAssetSearchFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.AssetSearchForm" );

    GUCEF_TESTFW_TESTCASE( "Search: the query, the filter description and the result page keep their values, compare and round trip through a node" )
    try
    {
        GUI::CSearchQuery query;
        ASSERT_TRUE( query.IsEmpty() );
        query.text = "sword";
        query.AddChoice( "kind", "model" );
        query.AddChoice( "kind", "texture" );
        GUI::CSearchQuery::TRange range;
        range.hasMaximum = true;
        range.maximum = 2500.5;
        query.SetRange( "size", range );
        query.sort = "size";
        query.pageIndex = 2;
        query.pageSize = 25;
        query.extra.SetName( "extra" );
        query.extra.SetAttribute( "native", "x=1" );
        ASSERT_TRUE( !query.IsEmpty() && query.HasChoice( "kind", "model" ) && !query.HasChoice( "kind", "audio" ) );
        ASSERT_TRUE( query.GetRange( "size" ).hasMaximum && !query.GetRange( "size" ).hasMinimum && 2500.5 == query.GetRange( "size" ).maximum );
        ASSERT_TRUE( !query.GetRange( "other" ).IsSet() );

        CORE::CDataNode node;
        query.Save( node );
        GUI::CSearchQuery loaded;
        ASSERT_TRUE( loaded.Load( node ) );
        ASSERT_TRUE( loaded.IsSameSearch( query ) && loaded.pageSize == 25 && "sword" == loaded.text && "x=1" == loaded.extra.GetAttributeValue( "native" ).AsString() );
        ASSERT_TRUE( 0 == loaded.pageIndex && loaded != query );
        loaded.pageIndex = 2;
        ASSERT_TRUE( loaded == query );
        GUI::CSearchQuery copy( query );
        ASSERT_TRUE( copy == query );
        copy.extra.SetAttribute( "native", "x=2" );
        ASSERT_TRUE( !copy.IsSameSearch( query ) );
        query.RemoveChoice( "kind", "model" );
        query.RemoveChoice( "kind", "texture" );
        ASSERT_TRUE( query.choices.empty() );
        query.SetRange( "size", GUI::CSearchQuery::TRange() );
        ASSERT_TRUE( query.ranges.empty() );
        CORE::CDataNode wrong;
        wrong.SetName( "Other" );
        ASSERT_FALSE( loaded.Load( wrong ) );

        GUI::CSearchFilterDescription description;
        description.AddChoiceGroup( "kind", "Kind" );
        ASSERT_TRUE( description.AddChoice( "kind", "model" ) && !description.AddChoice( "nothing", "x" ) );
        description.AddRangeGroup( "size", "Size", "KB" );
        ASSERT_TRUE( !description.AddChoice( "size", "x" ) && 2 == description.GetGroups().size() );
        ASSERT_TRUE( GUCEF_NULL != description.FindGroup( "size" ) && GUCEF_NULL == description.FindGroup( "x" ) && "model" == description.GetGroups()[ 0 ].choices[ 0 ].label );
        GUI::CSearchFilterDescription same = description;
        ASSERT_TRUE( same == description );
        same.AddChoice( "kind", "texture" );
        ASSERT_FALSE( same == description );

        GUI::CSearchResultPage page;
        page.total = 101;
        page.pageSize = 50;
        page.facets[ "kind" ][ "model" ] = 7;
        UInt64 count = 0;
        ASSERT_TRUE( 3 == page.GetPageCount() && page.GetFacetCount( "kind", "model", count ) && 7 == count && !page.GetFacetCount( "kind", "x", count ) );
        page.total = 0;
        ASSERT_TRUE( 0 == page.GetPageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the form builds as a window and in the place of a widget, with the model it searches at once and shows the first page" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        {
            GUI::CAssetSearchForm window( context );
            ASSERT_TRUE( window.Build( "Find" ) && window.IsVisible() );
            ASSERT_TRUE( "Find" == window.GetRootWidget()->GetProperty( "Title" ) );
        }
        SFixture fixture;
        ASSERT_TRUE( GUCEF_NULL != fixture.form );
        const char* names[] = { "Query", "Saved", "Sort", "FiltersToggle", "TilesToggle", "SaveSearch", "DeleteSearch", "Filters", "Results", "Grid", "Tiles", "Status", "PreviousPage", "PageLabel", "NextPage", "Suggestions" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( *fixture.form, names[ i ] ) );
        ASSERT_TRUE( 1 == fixture.form->GetRequestCount() && 1 == fixture.model.requests.size() );
        ASSERT_TRUE( fixture.model.requests[ 0 ].IsEmpty() && 0 == fixture.model.requests[ 0 ].pageIndex && 50 == fixture.model.requests[ 0 ].pageSize );
        ASSERT_TRUE( 25 == fixture.form->GetPage().total && 25 == fixture.form->GetShownRowCount() );
        ASSERT_TRUE( "25 results" == fixture.form->GetStatusText() && "25 results" == fixture.form->GetWidget( "Status" )->GetProperty( "Text" ) );
        GUI::CGridView* grid = fixture.form->GetWidgetAs< GUI::CGridView >( "Grid" );
        ASSERT_TRUE( 25 == grid->GetRowCount() && 3 == grid->GetColumnCount() );
        ASSERT_TRUE( "Name" == grid->GetColumnTitle( 0 ) );
        ASSERT_TRUE( "Sword 1" == grid->GetText( 0, 0 ) );
        ASSERT_TRUE( "model" == grid->GetText( 1, 0 ) );
        ASSERT_TRUE( "100 KB" == grid->GetText( 2, 0 ) );
        ASSERT_TRUE( "1 / 1" == fixture.form->GetWidget( "PageLabel" )->GetProperty( "Text" ) );
        // saved searches and sort choices of the model are shown, the sort and saved controls too
        ASSERT_TRUE( fixture.form->GetWidget( "Saved" )->GetBoolProperty( "Visible" ) && fixture.form->GetWidget( "Sort" )->GetBoolProperty( "Visible" ) );
        ASSERT_TRUE( 1 == fixture.form->GetSavedSearchNames().size() && "Textures" == fixture.form->GetSavedSearchNames()[ 0 ] );
        // without a model there is nothing to search
        SFixture empty( false, false );
        ASSERT_TRUE( "No catalogue" == empty.form->GetStatusText() && 0 == empty.form->GetRequestCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the filter panel is made from the description, with the counts of the facets next to the choices" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        ASSERT_TRUE( GUCEF_NULL != form.GetChoiceWidget( "kind", "model" ) && GUCEF_NULL != form.GetChoiceWidget( "kind", "audio" ) && GUCEF_NULL == form.GetChoiceWidget( "kind", "x" ) );
        ASSERT_TRUE( "Model (12)" == form.GetChoiceText( "kind", "model" ) && "Texture (8)" == form.GetChoiceText( "kind", "texture" ) && "Audio (5)" == form.GetChoiceText( "kind", "audio" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetRangeMinimumWidget( "size" ) && GUCEF_NULL != form.GetRangeMaximumWidget( "size" ) && GUCEF_NULL == form.GetRangeMinimumWidget( "kind" ) );

        // a search narrows the counts, a value without results says 0
        form.SetText( "sword" );
        ASSERT_TRUE( "Model (3)" == form.GetChoiceText( "kind", "model" ) && "Audio (1)" == form.GetChoiceText( "kind", "audio" ) && "Texture (0)" == form.GetChoiceText( "kind", "texture" ) );

        // the panel follows a description that changes: a new group, the old choices go
        GUI::CWidget* before = form.GetChoiceWidget( "kind", "model" );
        ASSERT_TRUE( GUCEF_NULL != before );
        fixture.model.ChangeDescription();
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != form.GetChoiceWidget( "kind", "model" ) );
        // the panel has the room of its groups in a scroll panel
        GUI::CScrollPanel* panel = GUI::CWidget::ToScrollPanel( form.GetWidget( "Filters" ) );
        ASSERT_TRUE( GUCEF_NULL != panel && panel->GetRect().height > 100 );
        const GUI::CRect box = form.GetChoiceWidget( "kind", "audio" )->GetRect();
        ASSERT_TRUE( box.width > 40 && box.height > 10 && box.top >= panel->GetRect().top );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: typing commits after the delay of frame time or at once with Enter, and every committed query is notified and asked of the model" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUITEST::CEventRecorder changed( &form, GUI::CAssetSearchForm::QueryChangedEvent );
        GUI::CWidget* query = form.GetWidget( "Query" );
        fixture.context->FlushChanges();
        fixture.context->SetFocus( query );
        fixture.context->InjectText( "axe" );
        // typed, not committed: no request yet
        ASSERT_TRUE( 1 == form.GetRequestCount() && 0 == changed.count );
        form.Update( 100 );
        ASSERT_TRUE( 1 == form.GetRequestCount() );
        form.Update( 250 );
        ASSERT_TRUE( 2 == form.GetRequestCount() && 1 == changed.count && "axe" == form.GetQuery().text );
        ASSERT_TRUE( 4 == form.GetPage().total && "4 results" == form.GetStatusText() );
        ASSERT_TRUE( "axe" == changed.Last< GUI::CAssetSearchForm::TQueryChangedEventData >()->GetData().text );

        // nothing more is asked while nothing changes
        form.Update( 1000 );
        ASSERT_TRUE( 2 == form.GetRequestCount() );

        // Enter commits at once
        TypeInto( fixture.context, query, "sword", true );
        ASSERT_TRUE( 3 == form.GetRequestCount() && "sword" == form.GetQuery().text && 4 == form.GetPage().total );
        // the same text again is the same search: it is not asked again
        TypeInto( fixture.context, query, "sword", true );
        ASSERT_TRUE( 3 == form.GetRequestCount() );
        // setting the text from the code
        form.SetText( "" );
        ASSERT_TRUE( 4 == form.GetRequestCount() );
        ASSERT_TRUE( 25 == form.GetPage().total );
        ASSERT_TRUE( query->GetProperty( "Text" ).IsNULLOrEmpty() );
        form.SetText( "wood" );
        ASSERT_TRUE( "wood" == query->GetProperty( "Text" ) && 4 == form.GetPage().total );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the word being typed asks the model for suggestions, shown in a list, and a choice replaces the word" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUI::CWidget* query = form.GetWidget( "Query" );
        GUI::CWidget* list = form.GetWidget( "Suggestions" );
        ASSERT_FALSE( list->GetBoolProperty( "Visible" ) );
        fixture.context->FlushChanges();
        fixture.context->SetFocus( query );
        fixture.context->InjectText( "a" );
        CORE::CString::StringVector suggestions = form.GetSuggestionList();
        ASSERT_TRUE( list->GetBoolProperty( "Visible" ) && 3 == suggestions.size() && "albedo" == suggestions[ 0 ] && "ambient" == suggestions[ 1 ] && "axe" == suggestions[ 2 ] );
        fixture.context->InjectText( "m" );
        suggestions = form.GetSuggestionList();
        ASSERT_TRUE( 1 == suggestions.size() && "ambient" == suggestions[ 0 ] );
        // the suggestions are of the last word
        TypeInto( fixture.context, query, "wood am", false );
        suggestions = form.GetSuggestionList();
        ASSERT_TRUE( 1 == suggestions.size() && "ambient" == suggestions[ 0 ] );
        ASSERT_TRUE( form.ChooseSuggestion( 0 ) );
        ASSERT_TRUE( "wood ambient " == query->GetProperty( "Text" ) && "wood ambient" == form.GetQuery().text && !list->GetBoolProperty( "Visible" ) );
        ASSERT_FALSE( form.ChooseSuggestion( 3 ) );
        // a word that nothing continues, and a text that ends in a space, have none
        TypeInto( fixture.context, query, "zzz", false );
        ASSERT_TRUE( form.GetSuggestionList().empty() && !list->GetBoolProperty( "Visible" ) );
        // Enter closes the list
        TypeInto( fixture.context, query, "sw", false );
        ASSERT_TRUE( list->GetBoolProperty( "Visible" ) );
        PressKey( fixture.context, INPUT::KEYCODE_RETURN );
        ASSERT_FALSE( list->GetBoolProperty( "Visible" ) );
        // a click on a suggestion of the list
        TypeInto( fixture.context, query, "ro", false );
        ASSERT_TRUE( 1 == form.GetSuggestionList().size() );
        fixture.context->RenderFrame( 16 );
        GUI::CListbox* box = form.GetWidgetAs< GUI::CListbox >( "Suggestions" );
        box->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( "ro" != query->GetProperty( "Text" ) );
        ASSERT_TRUE( "rock" == query->GetProperty( "Text" ).Trim( true ).Trim( false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: suggestions that the model gets later are shown when it says they changed, only while the person types in the query" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUI::CWidget* query = form.GetWidget( "Query" );
        GUI::CWidget* list = form.GetWidget( "Suggestions" );
        fixture.context->FlushChanges();
        fixture.context->SetFocus( query );
        TypeInto( fixture.context, query, "qua", false );
        ASSERT_TRUE( form.GetSuggestionList().empty() && !list->GetBoolProperty( "Visible" ) );
        // the answer for the prefix arrives: the form asks again and shows it
        CTestSearchModel::SItem item;
        item.key = "late";
        item.title = "Quartz Floor";
        item.kind = "texture";
        item.sizeKb = 10;
        item.available = true;
        fixture.model.items.push_back( item );
        fixture.model.AnnounceSuggestions();
        CORE::CString::StringVector suggestions = form.GetSuggestionList();
        ASSERT_TRUE( list->GetBoolProperty( "Visible" ) && 1 == suggestions.size() && "quartz" == suggestions[ 0 ] );
        // without the person typing in the query nothing pops up
        PressKey( fixture.context, INPUT::KEYCODE_RETURN );
        ASSERT_FALSE( list->GetBoolProperty( "Visible" ) );
        fixture.context->SetFocus( GUCEF_NULL );
        fixture.model.AnnounceSuggestions();
        ASSERT_FALSE( list->GetBoolProperty( "Visible" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the choices of a group commit at once, a click on a check box checks it, and the facets follow the results" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUITEST::CEventRecorder changed( &form, GUI::CAssetSearchForm::QueryChangedEvent );
        ClickWidget( fixture.context, form.GetChoiceWidget( "kind", "texture" ) );
        ASSERT_TRUE( form.GetQuery().HasChoice( "kind", "texture" ) && 8 == form.GetPage().total && 1 == changed.count );
        ASSERT_TRUE( 2 == form.GetRequestCount() );
        // any of the chosen values
        ASSERT_TRUE( form.SetChoice( "kind", "audio", true ) );
        ASSERT_TRUE( 13 == form.GetPage().total && form.GetChoiceWidget( "kind", "audio" )->GetBoolProperty( "Checked" ) );
        ASSERT_TRUE( form.SetChoice( "kind", "texture", false ) );
        ASSERT_TRUE( 5 == form.GetPage().total && !form.GetChoiceWidget( "kind", "texture" )->GetBoolProperty( "Checked" ) );
        ClickWidget( fixture.context, form.GetChoiceWidget( "kind", "audio" ) );
        ASSERT_TRUE( form.GetQuery().choices.empty() && 25 == form.GetPage().total );
        // a choice that is not in the description is refused, as is a range for a choice group
        ASSERT_FALSE( form.SetChoice( "kind2", "x", true ) );
        ASSERT_FALSE( form.SetRange( "kind", GUI::CSearchQuery::TRange() ) );
        // clearing the filters
        form.SetChoice( "kind", "model", true );
        form.SetText( "axe" );
        form.ClearFilters();
        ASSERT_TRUE( form.GetQuery().IsEmpty() );
        ASSERT_TRUE( 25 == form.GetPage().total );
        ASSERT_TRUE( form.GetWidget( "Query" )->GetProperty( "Text" ).IsNULLOrEmpty() );
        ASSERT_FALSE( form.GetChoiceWidget( "kind", "model" )->GetBoolProperty( "Checked" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: a range is typed in its boxes (commit with Enter or after the delay), a number that is not one is left out, and the query keeps the bounds" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUI::CWidget* minimum = form.GetRangeMinimumWidget( "size" );
        GUI::CWidget* maximum = form.GetRangeMaximumWidget( "size" );
        TypeInto( fixture.context, minimum, "1000", true );
        ASSERT_TRUE( form.GetQuery().GetRange( "size" ).hasMinimum && 1000.0 == form.GetQuery().GetRange( "size" ).minimum && !form.GetQuery().GetRange( "size" ).hasMaximum );
        ASSERT_TRUE( 11 == form.GetPage().total );
        const UInt64 above = form.GetPage().total;
        TypeInto( fixture.context, maximum, "2500", false );
        form.Update( 400 );
        ASSERT_TRUE( form.GetQuery().GetRange( "size" ).hasMaximum && 2500.0 == form.GetQuery().GetRange( "size" ).maximum && form.GetPage().total < above );
        const UInt32 requests = form.GetRequestCount();
        // text that is no number counts as no bound
        TypeInto( fixture.context, maximum, "many", true );
        ASSERT_TRUE( !form.GetQuery().GetRange( "size" ).hasMaximum && form.GetRequestCount() == requests + 1 );
        // from the code, the boxes show it
        GUI::CSearchQuery::TRange range;
        range.hasMinimum = true;
        range.minimum = 100;
        range.hasMaximum = true;
        range.maximum = 300;
        ASSERT_TRUE( form.SetRange( "size", range ) );
        ASSERT_TRUE( 100.0 == CORE::StringToDouble( minimum->GetProperty( "Text" ) ) && 300.0 == CORE::StringToDouble( maximum->GetProperty( "Text" ) ) && 3 == form.GetPage().total );
        ASSERT_TRUE( form.SetRange( "size", GUI::CSearchQuery::TRange() ) );
        ASSERT_TRUE( minimum->GetProperty( "Text" ).IsNULLOrEmpty() );
        ASSERT_TRUE( 25 == form.GetPage().total );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: results come in pages with the buttons of the paging bar, a change of the search goes back to the first page, and the sort is a choice of the model" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        form.SetPageSize( 10 );
        form.Search();
        ASSERT_TRUE( 10 == form.GetShownRowCount() && "1 / 3" == form.GetWidget( "PageLabel" )->GetProperty( "Text" ) && 25 == form.GetPage().total );
        GUI::CWidget* previous = form.GetWidget( "PreviousPage" );
        GUI::CWidget* next = form.GetWidget( "NextPage" );
        ASSERT_TRUE( !previous->GetBoolProperty( "Enabled" ) && next->GetBoolProperty( "Enabled" ) );
        GUITEST::CEventRecorder changed( &form, GUI::CAssetSearchForm::QueryChangedEvent );
        ClickWidget( fixture.context, next );
        ASSERT_TRUE( 1 == form.GetPage().pageIndex && "Sword 1" != form.GetShownRow( 0 ).title && "2 / 3" == form.GetWidget( "PageLabel" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 1 == changed.Last< GUI::CAssetSearchForm::TQueryChangedEventData >()->GetData().pageIndex );
        ASSERT_TRUE( form.NextPage() && 5 == form.GetShownRowCount() && !form.NextPage() && !next->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( form.GetWidget( "PageLabel" )->GetProperty( "Text" ) == "3 / 3" );
        ClickWidget( fixture.context, previous );
        ASSERT_TRUE( 1 == form.GetPage().pageIndex && form.PreviousPage() && !form.PreviousPage() && 0 == form.GetPage().pageIndex );
        ASSERT_FALSE( form.GoToPage( 7 ) );
        ASSERT_TRUE( form.GoToPage( 2 ) );
        // a filter goes back to the first page
        form.SetChoice( "kind", "model", true );
        ASSERT_TRUE( 0 == form.GetPage().pageIndex && 0 == form.GetQuery().pageIndex && 12 == form.GetPage().total && "1 / 2" == form.GetWidget( "PageLabel" )->GetProperty( "Text" ) );
        // the sort
        GUI::CCombobox* sort = form.GetWidgetAs< GUI::CCombobox >( "Sort" );
        ASSERT_TRUE( sort->GetSelectedIndex() == 0 && "Sword 1" == form.GetShownRow( 0 ).title );
        sort->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        ASSERT_TRUE( "size" == form.GetQuery().sort );
        ASSERT_TRUE( "Crate 3" == form.GetShownRow( 0 ).title );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: a result is selected with a click and opened with a double click or Enter, and a selection that is not on the new page ends" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Grid" );
        GUITEST::CEventRecorder selected( &form, GUI::CAssetSearchForm::ResultSelectedEvent );
        GUITEST::CEventRecorder opened( &form, GUI::CAssetSearchForm::ResultOpenedEvent );
        ClickRow( fixture.context, grid, 2 );
        ASSERT_TRUE( 1 == selected.count && "model-3" == selected.Last< CORE::TCloneableString >()->GetData() && "model-3" == form.GetSelectedKey() );
        DoubleClickRow( fixture.context, grid, 5 );
        ASSERT_TRUE( 1 == opened.count && "model-6" == opened.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_TRUE( form.SelectResult( "audio-2" ) && "audio-2" == form.GetSelectedKey() );
        ASSERT_FALSE( form.SelectResult( "nothing" ) );
        ASSERT_TRUE( form.OpenResult( "texture-1" ) && !form.OpenResult( "nothing" ) && 2 == opened.count );
        // the selection goes when the result is not there any more
        selected.Reset();
        form.SetText( "rock" );
        ASSERT_TRUE( form.GetSelectedKey().IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == selected.count );
        ASSERT_TRUE( selected.Last< CORE::TCloneableString >()->GetData().IsNULLOrEmpty() );
        // and stays when it is
        form.SetText( "" );
        form.SelectResult( "model-1" );
        selected.Reset();
        form.SetText( "sword" );
        ASSERT_TRUE( "model-1" == form.GetSelectedKey() && 0 == selected.count );
        ASSERT_TRUE( 0 == grid->GetSelectedRow() );
        // results that cannot be opened now are drawn dimmed in the grid
        form.SetText( "" );
        GUI::CColour dim, normal;
        ASSERT_TRUE( grid->GetRowColour( 3, dim ) && grid->GetRowColour( 2, normal ) );
        ASSERT_TRUE( dim != normal && !form.GetShownRow( 3 ).available );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: only the answer of the last request is shown, whether it comes in order, late or inside the call, and a failure keeps the results with its reason" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        fixture.model.deferred = true;
        form.SetText( "sword" );
        form.SetText( "axe" );
        ASSERT_TRUE( form.IsSearching() && "Searching..." == form.GetStatusText() && 2 == fixture.model.pending.size() );
        // the answers come the wrong way round: the first one is old
        fixture.model.RaiseAll( true );
        ASSERT_TRUE( !form.IsSearching() && 4 == form.GetPage().total && "axe" == form.GetQuery().text );
        // an answer for a request that is not the last is ignored
        form.SetText( "sword" );
        form.SetText( "crate" );
        TModel::TSearchCompletion stale = fixture.model.pending[ 0 ];
        fixture.model.pending.erase( fixture.model.pending.begin() );
        fixture.model.RaiseAll( false );
        const UInt64 total = form.GetPage().total;
        fixture.model.Raise( stale );
        ASSERT_TRUE( total == form.GetPage().total && 3 == total );
        // answered inside the call
        fixture.model.deferred = false;
        form.SetText( "crate" );
        ASSERT_TRUE( !form.IsSearching() && 3 == form.GetPage().total && "3 results" == form.GetStatusText() );
        // a failure keeps the results, says why and is replaced by the next answer
        fixture.model.failNext = true;
        form.SetText( "barrel" );
        ASSERT_TRUE( !form.IsSearching() );
        ASSERT_TRUE( 3 == form.GetPage().total );
        ASSERT_TRUE( "The catalogue is not reachable" == form.GetStatusText() );
        form.Search();
        ASSERT_TRUE( "3 results" == form.GetStatusText() );
        form.SetText( "nothing at all" );
        ASSERT_TRUE( "No results" == form.GetStatusText() );
        ASSERT_TRUE( 0 == form.GetShownRowCount() );
        ASSERT_TRUE( form.GetWidget( "PageLabel" )->GetProperty( "Text" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the saved searches are chosen from a list and put in the filters, a search is kept under a name and a built-in one is not deleted" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUITEST::CEventRecorder chosen( &form, GUI::CAssetSearchForm::SavedSearchChosenEvent );
        GUI::CCombobox* list = form.GetWidgetAs< GUI::CCombobox >( "Saved" );
        {
            GUI::CCombobox::TStringVector items;
            list->GetListItems( items );
            ASSERT_TRUE( 2 == items.size() && "Saved searches" == items[ 0 ] && "Textures" == items[ 1 ] );
        }
        list->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        ASSERT_TRUE( 1 == chosen.count );
        ASSERT_TRUE( "s1" == chosen.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_TRUE( "s1" == form.GetChosenSavedSearchId() );
        ASSERT_TRUE( form.GetQuery().HasChoice( "kind", "texture" ) );
        ASSERT_TRUE( form.GetChoiceWidget( "kind", "texture" )->GetBoolProperty( "Checked" ) );
        ASSERT_TRUE( 8 == form.GetPage().total );
        // the saved search is not the query any more when the person changes it
        form.SetText( "rock" );
        ASSERT_TRUE( form.GetChosenSavedSearchId().IsNULLOrEmpty() );
        ASSERT_TRUE( form.ChooseSavedSearch( "s1" ) && !form.ChooseSavedSearch( "nothing" ) );

        // the built-in one cannot be deleted, the button is disabled
        ASSERT_FALSE( form.DeleteChosenSavedSearch() );
        ASSERT_FALSE( form.GetWidget( "DeleteSearch" )->GetBoolProperty( "Enabled" ) );

        // keeping a search
        form.SetText( "wood" );
        ASSERT_TRUE( form.SaveCurrentSearch( "Wooden things" ) );
        ASSERT_TRUE( 2 == form.GetSavedSearchNames().size() && "Wooden things" == form.GetSavedSearchNames()[ 1 ] && "u2" == form.GetChosenSavedSearchId() );
        ASSERT_TRUE( form.GetWidget( "DeleteSearch" )->GetBoolProperty( "Enabled" ) );
        form.ClearFilters();
        ASSERT_TRUE( form.ChooseSavedSearch( "u2" ) && "wood" == form.GetQuery().text && "wood" == form.GetWidget( "Query" )->GetProperty( "Text" ) && 4 == form.GetPage().total );
        // the same name replaces it
        form.SetChoice( "kind", "texture", true );
        ASSERT_TRUE( form.SaveCurrentSearch( "Wooden things" ) && 2 == fixture.model.saved.size() && fixture.model.saved[ 1 ].query.HasChoice( "kind", "texture" ) );
        ASSERT_FALSE( form.SaveCurrentSearch( "" ) );
        // the buttons: Save opens the box for the name
        ClickWidget( fixture.context, form.GetWidget( "SaveSearch" ) );
        fixture.context->RenderFrame( 16 );
        // deleting
        ASSERT_TRUE( form.DeleteChosenSavedSearch() && 1 == form.GetSavedSearchNames().size() && form.GetChosenSavedSearchId().IsNULLOrEmpty() );

        // a model that keeps no searches: no list and no buttons
        fixture.model.savedSupported = false;
        fixture.model.ChangeSaved();
        form.SetModel( GUCEF_NULL );
        form.SetModel( &fixture.model );
        ASSERT_TRUE( !form.GetWidget( "Saved" )->GetBoolProperty( "Visible" ) && !form.GetWidget( "SaveSearch" )->GetBoolProperty( "Visible" ) && !form.SaveCurrentSearch( "x" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the thumbnail view makes a tile per result, asks the provider for the pictures once and shows them when they are ready" )
    try
    {
        // a provider that answers when it is told to
        class CTestProvider : public GUI::CIThumbnailProvider
        {
            public:
            GUCEF::vector< CORE::CString > requested;
            virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE { requested.push_back( path ); }
            void Answer( const CORE::CString& path, UInt32 size )
            {
                TThumbnail thumbnail;
                thumbnail.path = path;
                thumbnail.size = size;
                UInt8 pixels[ 4 * 4 * 4 ];
                for ( UInt32 i=0; i<sizeof( pixels ); ++i )
                    pixels[ i ] = 200;
                thumbnail.pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( pixels, 4, 4, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
                TThumbnailReadyEventData data( thumbnail );
                NotifyObservers( ThumbnailReadyEvent, &data );
            }
        };
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        CTestProvider provider;
        form.SetThumbnailProvider( &provider );
        ASSERT_FALSE( form.IsThumbnailView() );
        ASSERT_TRUE( 0 == provider.requested.size() && 0 == form.GetRealizedTileCount() );
        ClickWidget( fixture.context, form.GetWidget( "TilesToggle" ) );
        fixture.context->RenderFrame( 16 );
        form.Update( 16 );
        ASSERT_TRUE( form.IsThumbnailView() && form.GetWidget( "Tiles" )->GetBoolProperty( "Visible" ) && !form.GetWidget( "Grid" )->GetBoolProperty( "Visible" ) );
        ASSERT_TRUE( 25 == form.GetRealizedTileCount() && 25 == provider.requested.size() && "thumb:model-1" == provider.requested[ 0 ] );
        // asked once
        form.Update( 16 );
        form.SetThumbnailSize( 96 );
        ASSERT_TRUE( 25 == provider.requested.size() );
        ASSERT_FALSE( form.HasThumbnail( "thumb:model-1" ) );
        provider.Answer( "thumb:model-1", 96 );
        ASSERT_TRUE( form.HasThumbnail( "thumb:model-1" ) );
        GUI::CWidget* tiles = form.GetWidget( "Tiles" );
        GUI::CWidget* tile = tiles->GetChildWidget( "Tile_0" );
        ASSERT_TRUE( GUCEF_NULL != tile && tile->GetChildWidget( "Image" )->GetBoolProperty( "Visible" ) && !tile->GetChildWidget( "Kind" )->GetBoolProperty( "Visible" ) );
        GUI::CWidget* other = tiles->GetChildWidget( "Tile_1" );
        ASSERT_TRUE( !other->GetChildWidget( "Image" )->GetBoolProperty( "Visible" ) && "model" == other->GetChildWidget( "Kind" )->GetProperty( "Text" ) && "Sword 2" == other->GetChildWidget( "Name" )->GetProperty( "Text" ) );
        // an answer of another size is not for these tiles
        provider.Answer( "thumb:model-2", 48 );
        ASSERT_FALSE( form.HasThumbnail( "thumb:model-2" ) );
        // tiles do not overlap and lie inside the panel
        const GUI::CRect first = tile->GetRect();
        const GUI::CRect second = other->GetRect();
        ASSERT_TRUE( first.width > 90 && first.height > 100 && ( second.left >= first.left + first.width || second.top >= first.top + first.height ) );

        // click and double click on tiles
        GUITEST::CEventRecorder selected( &form, GUI::CAssetSearchForm::ResultSelectedEvent );
        GUITEST::CEventRecorder opened( &form, GUI::CAssetSearchForm::ResultOpenedEvent );
        ClickWidget( fixture.context, other );
        ASSERT_TRUE( 1 == selected.count && "model-2" == form.GetSelectedKey() );
        fixture.context->RenderFrame( 500 );
        ClickWidget( fixture.context, tile );
        ClickWidget( fixture.context, tile );
        ASSERT_TRUE( 1 == opened.count && "model-1" == opened.Last< CORE::TCloneableString >()->GetData() );

        // a new page rebinds the tiles, fewer results hide the spare ones without making widgets
        form.SetText( "axe" );
        form.Update( 16 );
        ASSERT_TRUE( 4 == form.GetRealizedTileCount() && !tiles->GetChildWidget( "Tile_7" )->GetBoolProperty( "Visible" ) );
        form.SetThumbnailView( false );
        ASSERT_TRUE( form.GetWidget( "Grid" )->GetBoolProperty( "Visible" ) && !form.GetWidget( "Tiles" )->GetBoolProperty( "Visible" ) );
        // a provider that goes away is let go of
        form.SetThumbnailProvider( GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL == form.GetThumbnailProvider() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: the filters fold away with their check box and come above the results in a narrow place, and the results get the room" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        GUI::CWidget* filters = form.GetWidget( "Filters" );
        GUI::CWidget* results = form.GetWidget( "Results" );
        const GUI::CRect narrow = results->GetRect();
        ASSERT_TRUE( form.AreFiltersVisible() && filters->GetBoolProperty( "Visible" ) );
        ClickWidget( fixture.context, form.GetWidget( "FiltersToggle" ) );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( !form.AreFiltersVisible() && !filters->GetBoolProperty( "Visible" ) && results->GetRect().width > narrow.width + 100 );
        form.SetFiltersVisible( true );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( filters->GetBoolProperty( "Visible" ) && results->GetRect().width == narrow.width );
        ASSERT_TRUE( filters->GetRect().left + filters->GetRect().width <= results->GetRect().left + 2 );

        SFixture top( true );
        GUI::CWidget* topFilters = top.form->GetWidget( "Filters" );
        GUI::CWidget* topResults = top.form->GetWidget( "Results" );
        const GUI::CRect a = topFilters->GetRect();
        const GUI::CRect b = topResults->GetRect();
        ASSERT_TRUE( a.top + a.height <= b.top + 2 && a.width > 400 && b.width > 400 && b.height > 100 );
        top.form->SetFiltersVisible( false );
        top.context->RenderFrame( 16 );
        ASSERT_TRUE( topResults->GetRect().height > b.height + 50 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Search: a model that is replaced or goes away is let go of, and the form searches the new one at once" )
    try
    {
        SFixture fixture;
        GUI::CAssetSearchForm& form = *fixture.form;
        form.SetText( "sword" );
        {
            CTestSearchModel other;
            other.items.clear();
            other.Add( "a", "Sword A", "model", 1 );
            other.Add( "b", "Sword B", "model", 2 );
            other.Add( "c", "Sword C", "model", 3 );
            form.SetModel( &other );
            ASSERT_TRUE( &other == form.GetModel() && 3 == form.GetPage().total && form.GetQuery().text == "sword" );
        }
        // the model above was destroyed: the form knows
        ASSERT_TRUE( GUCEF_NULL == form.GetModel() );
        form.SetText( "x" );
        ASSERT_TRUE( "No catalogue" == form.GetStatusText() );
        form.SetModel( &fixture.model );
        ASSERT_TRUE( 4 == form.GetPage().total && "x" == form.GetQuery().text );
        form.SetModel( GUCEF_NULL );
        ASSERT_TRUE( 0 == form.GetShownRowCount() );
        form.SetModel( &fixture.model );
        ASSERT_TRUE( form.GetShownRowCount() > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "asset search", &BuildSearchScene );
    RunFormLintBaseline( "AssetSearch", &BuildSearchScene );
}
