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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CASSETSEARCHFORM_H
#include "gucefGUI_CAssetSearchForm.h"
#define GUCEF_GUI_CASSETSEARCHFORM_H
#endif /* GUCEF_GUI_CASSETSEARCHFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetSearchForm::QueryChangedEvent = "GUCEF::GUI::CAssetSearchForm::QueryChangedEvent";
const CORE::CEvent CAssetSearchForm::ResultOpenedEvent = "GUCEF::GUI::CAssetSearchForm::ResultOpenedEvent";
const CORE::CEvent CAssetSearchForm::ResultSelectedEvent = "GUCEF::GUI::CAssetSearchForm::ResultSelectedEvent";
const CORE::CEvent CAssetSearchForm::SavedSearchChosenEvent = "GUCEF::GUI::CAssetSearchForm::SavedSearchChosenEvent";

namespace {

const Int32 TopBarHeight = 100;
const Int32 BottomBarHeight = 30;
const Int32 Margin = 6;
const Int32 GroupTitleHeight = 22;
const Int32 ChoiceHeight = 24;
const Int32 RangeHeight = 28;
const Int32 TilePadding = 6;
const Int32 TileLabelHeight = 34;
const Int32 SuggestionRowHeight = 22;
const char* const SavedSearchesTitle = "Saved searches";

const CColour TileColour( 45, 48, 56, 255 );
const CColour TileSelectedColour( 52, 98, 168, 255 );
const CColour NormalRowColour( 224, 224, 224, 255 );
const CColour UnavailableRowColour( 128, 128, 134, 255 );

CString
Int( Int32 value )
{
    return CORE::Int32ToString( value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


void
CAssetSearchForm::RegisterEvents( void )
{GUCEF_TRACE;

    QueryChangedEvent.Initialize();
    ResultOpenedEvent.Initialize();
    ResultSelectedEvent.Initialize();
    SavedSearchChosenEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetSearchForm::CAssetSearchForm( void )
    : CFormEx()
    , m_model( GUCEF_NULL )
    , m_thumbnailProvider( GUCEF_NULL )
    , m_placement( FILTERPLACEMENT_LEFT )
    , m_query()
    , m_page()
    , m_description()
    , m_columns()
    , m_sortChoices()
    , m_savedSearches()
    , m_chosenSavedSearchId()
    , m_choiceWidgets()
    , m_rangeWidgets()
    , m_filterWidgets()
    , m_tiles()
    , m_thumbnails()
    , m_requestedThumbnails()
    , m_suggestions()
    , m_selectedKey()
    , m_statusNote()
    , m_nameBox( GUCEF_NULL )
    , m_requestId( 0 )
    , m_requestCount( 0 )
    , m_pageSize( DefaultPageSize )
    , m_thumbnailSize( DefaultThumbnailSize )
    , m_filterGeneration( 0 )
    , m_typedWaitMs( 0 )
    , m_tilesPerRow( 1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_lastScroll( 0 )
    , m_inRequest( false )
    , m_completedInRequest( false )
    , m_searching( false )
    , m_editsPending( false )
    , m_thumbnailView( false )
    , m_filtersVisible( true )
    , m_tilesDirty( true )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_haveResults( false )
    , m_searchFailed( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_query.pageSize = m_pageSize;
}

/*-------------------------------------------------------------------------*/

CAssetSearchForm::CAssetSearchForm( CIGUIContext* context )
    : CFormEx( context )
    , m_model( GUCEF_NULL )
    , m_thumbnailProvider( GUCEF_NULL )
    , m_placement( FILTERPLACEMENT_LEFT )
    , m_query()
    , m_page()
    , m_description()
    , m_columns()
    , m_sortChoices()
    , m_savedSearches()
    , m_chosenSavedSearchId()
    , m_choiceWidgets()
    , m_rangeWidgets()
    , m_filterWidgets()
    , m_tiles()
    , m_thumbnails()
    , m_requestedThumbnails()
    , m_suggestions()
    , m_selectedKey()
    , m_statusNote()
    , m_nameBox( GUCEF_NULL )
    , m_requestId( 0 )
    , m_requestCount( 0 )
    , m_pageSize( DefaultPageSize )
    , m_thumbnailSize( DefaultThumbnailSize )
    , m_filterGeneration( 0 )
    , m_typedWaitMs( 0 )
    , m_tilesPerRow( 1 )
    , m_lastViewportWidth( 0 )
    , m_lastViewportHeight( 0 )
    , m_lastScroll( 0 )
    , m_inRequest( false )
    , m_completedInRequest( false )
    , m_searching( false )
    , m_editsPending( false )
    , m_thumbnailView( false )
    , m_filtersVisible( true )
    , m_tilesDirty( true )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_haveResults( false )
    , m_searchFailed( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_query.pageSize = m_pageSize;
}

/*-------------------------------------------------------------------------*/

CAssetSearchForm::~CAssetSearchForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_nameBox;
    m_nameBox = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetFilterPlacement( EFilterPlacement placement )
{GUCEF_TRACE;

    m_placement = placement;
    ArrangeAreas();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "AssetSearchForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "AssetSearchForm", "0,0 0,0", "1,0 1,0" );

    CFormLayout::AddWidgetNode( *content, "Editbox", "Query", "0,6 0,4", "1,-12 0,28" );
    CFormLayout::AddWidgetNode( *content, "Combobox", "Saved", "0,6 0,36", "0.5,-9 0,28" );
    CFormLayout::AddWidgetNode( *content, "Combobox", "Sort", "0.5,3 0,36", "0.5,-9 0,28" );
    CORE::CDataNode* filters = CFormLayout::AddWidgetNode( *content, "Checkbox", "FiltersToggle", "0,6 0,70", "0,70 0,26" );
    CFormLayout::SetWidgetNodeProperty( *filters, EWidgetProperty_Text, "Filters" );
    CFormLayout::SetWidgetNodeProperty( *filters, EWidgetProperty_Checked, "true" );
    CORE::CDataNode* tiles = CFormLayout::AddWidgetNode( *content, "Checkbox", "TilesToggle", "0,80 0,70", "0,58 0,26" );
    CFormLayout::SetWidgetNodeProperty( *tiles, EWidgetProperty_Text, "Tiles" );
    CORE::CDataNode* save = CFormLayout::AddWidgetNode( *content, "Button", "SaveSearch", "1,-146 0,70", "0,68 0,26" );
    CFormLayout::SetWidgetNodeProperty( *save, EWidgetProperty_Text, "Save" );
    CORE::CDataNode* remove = CFormLayout::AddWidgetNode( *content, "Button", "DeleteSearch", "1,-74 0,70", "0,68 0,26" );
    CFormLayout::SetWidgetNodeProperty( *remove, EWidgetProperty_Text, "Delete" );

    CORE::CDataNode* filterPanel = CFormLayout::AddWidgetNode( *content, "ScrollPanel", "Filters", "0,6 0,100", "0.3,-9 1,-132" );
    CFormLayout::SetWidgetNodeProperty( *filterPanel, EWidgetProperty_BackgroundColour, "34,36,44,255" );
    CORE::CDataNode* results = CFormLayout::AddWidgetNode( *content, "Panel", "Results", "0.3,3 0,100", "0.7,-9 1,-132" );
    CFormLayout::SetWidgetNodeProperty( *results, EWidgetProperty_BackgroundColour, "0,0,0,0" );
    CORE::CDataNode* grid = CFormLayout::AddWidgetNode( *results, "GridView", "Grid", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_FitColumns, "true" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_TextOverflow, "Ellipsis" );
    CORE::CDataNode* thumbs = CFormLayout::AddWidgetNode( *results, "ScrollPanel", "Tiles", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *thumbs, EWidgetProperty_BackgroundColour, "30,32,38,255" );
    CFormLayout::SetWidgetNodeProperty( *thumbs, EWidgetProperty_Focusable, "true" );
    CFormLayout::SetWidgetNodeProperty( *thumbs, EWidgetProperty_Visible, "false" );

    CFormLayout::AddWidgetNode( *content, "Label", "Status", "0,6 1,-28", "1,-206 0,24" );
    CORE::CDataNode* previous = CFormLayout::AddWidgetNode( *content, "Button", "PreviousPage", "1,-192 1,-28", "0,56 0,24" );
    CFormLayout::SetWidgetNodeProperty( *previous, EWidgetProperty_Text, "Prev" );
    CORE::CDataNode* pageLabel = CFormLayout::AddWidgetNode( *content, "Label", "PageLabel", "1,-132 1,-28", "0,72 0,24" );
    CFormLayout::SetWidgetNodeProperty( *pageLabel, EWidgetProperty_HTextAlign, "Center" );
    CORE::CDataNode* next = CFormLayout::AddWidgetNode( *content, "Button", "NextPage", "1,-56 1,-28", "0,50 0,24" );
    CFormLayout::SetWidgetNodeProperty( *next, EWidgetProperty_Text, "Next" );

    // last, so that it is drawn over the filters and the results
    CORE::CDataNode* suggestions = CFormLayout::AddWidgetNode( *content, "Listbox", "Suggestions", "0,6 0,32", "1,-12 0,22" );
    CFormLayout::SetWidgetNodeProperty( *suggestions, EWidgetProperty_Visible, "false" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Query" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback changedCallback( this, &CAssetSearchForm::OnQueryTextChanged );
        SubscribeTo( widget, CEditbox::TextChangedEvent, changedCallback );
        TEventCallback enteredCallback( this, &CAssetSearchForm::OnQueryTextEntered );
        SubscribeTo( widget, CEditbox::TextEnteredEvent, enteredCallback );
    }
    widget = GetWidget( "Saved" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnSavedValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Sort" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnSortValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "FiltersToggle" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CAssetSearchForm::OnFiltersChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CAssetSearchForm::OnFiltersUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "TilesToggle" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CAssetSearchForm::OnTilesChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CAssetSearchForm::OnTilesUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "SaveSearch" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnSaveActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "DeleteSearch" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnDeleteActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "PreviousPage" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnPreviousActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "NextPage" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnNextActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Suggestions" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnSuggestionValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Grid" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback selectionCallback( this, &CAssetSearchForm::OnGridSelectionChanged );
        SubscribeTo( widget, CGridView::SelectionChangedEvent, selectionCallback );
        TEventCallback itemCallback( this, &CAssetSearchForm::OnGridItemActivated );
        SubscribeTo( widget, CWidget::ItemActivatedEvent, itemCallback );
    }
    widget = GetWidget( "Tiles" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnTilesMouseClick );
        SubscribeTo( widget, CWidget::MouseClickEvent, callback );
    }
    widget = GetRootWidget();
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnRootKeyDown );
        SubscribeTo( widget, CWidget::KeyDownEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CAssetSearchForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }

    // the widgets are new: whatever the form knows goes into them
    m_choiceWidgets.clear();
    m_rangeWidgets.clear();
    m_filterWidgets.clear();
    m_tiles.clear();
    m_tilesDirty = true;
    ArrangeAreas();
    ShowColumns();
    ShowSortChoices();
    ShowSavedSearches();
    BuildFilterPanel();
    ShowQueryInWidgets();
    ShowResults();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ArrangeAreas( void )
{GUCEF_TRACE;

    CWidget* filters = GetWidget( "Filters" );
    CWidget* results = GetWidget( "Results" );
    if ( GUCEF_NULL == filters || GUCEF_NULL == results )
        return;
    const CString top = Int( TopBarHeight );
    const CString bottom = Int( BottomBarHeight + 2 );
    filters->SetBoolProperty( EWidgetProperty_Visible, m_filtersVisible );
    if ( FILTERPLACEMENT_LEFT == m_placement )
    {
        filters->SetProperty( EWidgetProperty_Position, "0,6 0," + top );
        filters->SetProperty( EWidgetProperty_Size, "0.3,-9 1,-" + Int( TopBarHeight + BottomBarHeight + 2 ) );
        results->SetProperty( EWidgetProperty_Position, m_filtersVisible ? "0.3,3 0," + top : "0,6 0," + top );
        results->SetProperty( EWidgetProperty_Size, m_filtersVisible ? "0.7,-9 1,-" + Int( TopBarHeight + BottomBarHeight + 2 ) : "1,-12 1,-" + Int( TopBarHeight + BottomBarHeight + 2 ) );
    }
    else
    {
        // above the results: a third of the height that is left
        filters->SetProperty( EWidgetProperty_Position, "0,6 0," + top );
        filters->SetProperty( EWidgetProperty_Size, "1,-12 0.34,-" + Int( TopBarHeight / 3 ) );
        results->SetProperty( EWidgetProperty_Position, m_filtersVisible ? "0,6 0.34," + Int( TopBarHeight - TopBarHeight / 3 + 4 ) : "0,6 0," + top );
        results->SetProperty( EWidgetProperty_Size, m_filtersVisible ? "1,-12 0.66,-" + Int( BottomBarHeight + 6 + ( TopBarHeight - TopBarHeight / 3 ) ) : "1,-12 1,-" + Int( TopBarHeight + BottomBarHeight + 2 ) );
    }
    m_tilesDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetModel( CISearchModel* model )
{GUCEF_TRACE;

    if ( model == m_model )
        return;
    if ( GUCEF_NULL != m_model )
        UnsubscribeFrom( m_model );
    m_model = model;
    m_requestId = 0;
    m_searching = false;
    m_page.Clear();
    m_haveResults = false;
    m_selectedKey.Clear();
    m_chosenSavedSearchId.Clear();
    if ( GUCEF_NULL != m_model )
    {
        TEventCallback completedCallback( this, &CAssetSearchForm::OnSearchCompleted );
        SubscribeTo( m_model, CISearchModel::SearchCompletedEvent, completedCallback );
        TEventCallback descriptionCallback( this, &CAssetSearchForm::OnFilterDescriptionChanged );
        SubscribeTo( m_model, CISearchModel::FilterDescriptionChangedEvent, descriptionCallback );
        TEventCallback savedCallback( this, &CAssetSearchForm::OnSavedSearchesChanged );
        SubscribeTo( m_model, CISearchModel::SavedSearchesChangedEvent, savedCallback );
        TEventCallback suggestionsCallback( this, &CAssetSearchForm::OnSuggestionsChanged );
        SubscribeTo( m_model, CISearchModel::SuggestionsChangedEvent, suggestionsCallback );
    }
    ReadModel();
    ShowResults();
    if ( GUCEF_NULL != m_model )
    {
        m_query.pageIndex = 0;
        NotifyQueryChanged();
        RunSearch();
    }
}

/*-------------------------------------------------------------------------*/

CISearchModel*
CAssetSearchForm::GetModel( void ) const
{GUCEF_TRACE;

    return m_model;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetThumbnailProvider( CIThumbnailProvider* provider )
{GUCEF_TRACE;

    if ( provider == m_thumbnailProvider )
        return;
    if ( GUCEF_NULL != m_thumbnailProvider )
        UnsubscribeFrom( m_thumbnailProvider );
    m_thumbnailProvider = provider;
    m_requestedThumbnails.clear();
    if ( GUCEF_NULL != m_thumbnailProvider )
    {
        TEventCallback callback( this, &CAssetSearchForm::OnThumbnailReady );
        SubscribeTo( m_thumbnailProvider, CIThumbnailProvider::ThumbnailReadyEvent, callback );
    }
    m_tilesDirty = true;
}

/*-------------------------------------------------------------------------*/

CIThumbnailProvider*
CAssetSearchForm::GetThumbnailProvider( void ) const
{GUCEF_TRACE;

    return m_thumbnailProvider;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ReadModel( void )
{GUCEF_TRACE;

    m_description.Clear();
    m_columns.clear();
    m_sortChoices.clear();
    m_savedSearches.clear();
    if ( GUCEF_NULL != m_model )
    {
        m_model->GetFilterDescription( m_description );
        m_model->GetResultColumns( m_columns );
        m_model->GetSortChoices( m_sortChoices );
        if ( m_model->SupportsSavedSearches() )
            m_model->GetSavedSearches( m_savedSearches );
    }
    ShowColumns();
    ShowSortChoices();
    ShowSavedSearches();
    BuildFilterPanel();
    ShowQueryInWidgets();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowColumns( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Grid" );
    if ( GUCEF_NULL == grid )
        return;
    CString titles;
    GUCEF::vector< Int32 > widths;
    for ( UInt32 i=0; i<(UInt32)m_columns.size(); ++i )
    {
        titles += ( i > 0 ? "\n" : "" ) + m_columns[ i ].title;
        widths.push_back( m_columns[ i ].width > 0 ? m_columns[ i ].width : 1 );
    }
    m_updatingWidgets = true;
    grid->SetProperty( EWidgetProperty_Columns, titles );
    grid->SetIntListProperty( EWidgetProperty_ColumnWidths, widths );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowSortChoices( void )
{GUCEF_TRACE;

    CCombobox* sort = GetWidgetAs< CCombobox >( "Sort" );
    if ( GUCEF_NULL == sort )
        return;
    CCombobox::TStringVector items;
    Int32 selected = 0;
    for ( UInt32 i=0; i<(UInt32)m_sortChoices.size(); ++i )
    {
        items.push_back( m_sortChoices[ i ].title );
        if ( m_sortChoices[ i ].id == m_query.sort )
            selected = (Int32) i;
    }
    m_updatingWidgets = true;
    sort->SetListItems( items );
    if ( !items.empty() )
        sort->SetSelectedIndex( selected );
    m_updatingWidgets = false;
    sort->SetBoolProperty( EWidgetProperty_Visible, !m_sortChoices.empty() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowSavedSearches( void )
{GUCEF_TRACE;

    CCombobox* saved = GetWidgetAs< CCombobox >( "Saved" );
    if ( GUCEF_NULL == saved )
        return;
    const bool supported = GUCEF_NULL != m_model && m_model->SupportsSavedSearches();
    CCombobox::TStringVector items;
    items.push_back( SavedSearchesTitle );
    Int32 selected = 0;
    for ( UInt32 i=0; i<(UInt32)m_savedSearches.size(); ++i )
    {
        items.push_back( m_savedSearches[ i ].name );
        if ( !m_chosenSavedSearchId.IsNULLOrEmpty() && m_savedSearches[ i ].id == m_chosenSavedSearchId )
            selected = (Int32) i + 1;
    }
    m_updatingWidgets = true;
    saved->SetListItems( items );
    saved->SetSelectedIndex( selected );
    m_updatingWidgets = false;
    saved->SetBoolProperty( EWidgetProperty_Visible, supported );
    CWidget* save = GetWidget( "SaveSearch" );
    CWidget* remove = GetWidget( "DeleteSearch" );
    if ( GUCEF_NULL != save )
        save->SetBoolProperty( EWidgetProperty_Visible, supported );
    if ( GUCEF_NULL != remove )
    {
        remove->SetBoolProperty( EWidgetProperty_Visible, supported );
        const Int32 index = FindSavedSearchIndex( m_chosenSavedSearchId );
        remove->SetBoolProperty( EWidgetProperty_Enabled, index >= 0 && !m_savedSearches[ index ].builtIn );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ClearFilterPanel( void )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
        for ( UInt32 i=0; i<(UInt32)m_filterWidgets.size(); ++i )
            context->DestroyWidget( m_filterWidgets[ i ] );
    m_filterWidgets.clear();
    m_choiceWidgets.clear();
    m_rangeWidgets.clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::BuildFilterPanel( void )
{GUCEF_TRACE;

    ClearFilterPanel();
    CGUIContext* context = GetGUIContext();
    CWidget* panel = GetWidget( "Filters" );
    if ( GUCEF_NULL == context || GUCEF_NULL == panel )
        return;
    ++m_filterGeneration;
    const CString tag = "_" + CORE::UInt32ToString( m_filterGeneration ) + "_";
    Int32 y = Margin;
    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 g=0; g<(UInt32)groups.size(); ++g )
    {
        const CSearchFilterDescription::TGroup& group = groups[ g ];
        CWidget* title = context->CreateWidget( "Label", "GroupTitle" + tag + CORE::UInt32ToString( g ), panel );
        if ( GUCEF_NULL == title )
            continue;
        m_filterWidgets.push_back( title );
        title->SetProperty( EWidgetProperty_Position, "0," + Int( Margin ) + " 0," + Int( y ) );
        title->SetProperty( EWidgetProperty_Size, "1,-" + Int( 2 * Margin ) + " 0," + Int( GroupTitleHeight ) );
        title->SetProperty( EWidgetProperty_Text, group.unit.IsNULLOrEmpty() || CSearchFilterDescription::FILTERKIND_RANGE != group.kind ? group.title : group.title + " (" + group.unit + ")" );
        title->SetColourProperty( EWidgetProperty_TextColour, CColour( 150, 170, 210, 255 ) );
        title->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        if ( !group.tooltip.IsNULLOrEmpty() )
            title->SetProperty( EWidgetProperty_Tooltip, group.tooltip );
        y += GroupTitleHeight;
        if ( CSearchFilterDescription::FILTERKIND_CHOICES == group.kind )
        {
            for ( UInt32 c=0; c<(UInt32)group.choices.size(); ++c )
            {
                CWidget* box = context->CreateWidget( "Checkbox", "Choice" + tag + CORE::UInt32ToString( g ) + "_" + CORE::UInt32ToString( c ), panel );
                if ( GUCEF_NULL == box )
                    continue;
                m_filterWidgets.push_back( box );
                box->SetProperty( EWidgetProperty_Position, "0," + Int( Margin + 8 ) + " 0," + Int( y ) );
                box->SetProperty( EWidgetProperty_Size, "1,-" + Int( 2 * Margin + 8 ) + " 0," + Int( ChoiceHeight - 2 ) );
                box->SetProperty( EWidgetProperty_Text, group.choices[ c ].label.IsNULLOrEmpty() ? group.choices[ c ].value : group.choices[ c ].label );
                SChoiceWidget entry;
                entry.widget = box;
                entry.group = g;
                entry.choice = c;
                m_choiceWidgets.push_back( entry );
                TEventCallback checkedCallback( this, &CAssetSearchForm::OnChoiceChecked );
                SubscribeTo( box, CCheckbox::BoxCheckedEvent, checkedCallback );
                TEventCallback uncheckedCallback( this, &CAssetSearchForm::OnChoiceUnchecked );
                SubscribeTo( box, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
                y += ChoiceHeight;
            }
        }
        else
        {
            SRangeWidget range;
            range.group = g;
            range.minimum = context->CreateWidget( "Editbox", "RangeMin" + tag + CORE::UInt32ToString( g ), panel );
            range.maximum = context->CreateWidget( "Editbox", "RangeMax" + tag + CORE::UInt32ToString( g ), panel );
            if ( GUCEF_NULL != range.minimum && GUCEF_NULL != range.maximum )
            {
                m_filterWidgets.push_back( range.minimum );
                m_filterWidgets.push_back( range.maximum );
                range.minimum->SetProperty( EWidgetProperty_Position, "0," + Int( Margin + 8 ) + " 0," + Int( y ) );
                range.minimum->SetProperty( EWidgetProperty_Size, "0.5,-" + Int( Margin + 12 ) + " 0," + Int( RangeHeight - 2 ) );
                range.minimum->SetProperty( EWidgetProperty_Tooltip, "From" );
                range.maximum->SetProperty( EWidgetProperty_Position, "0.5,4 0," + Int( y ) );
                range.maximum->SetProperty( EWidgetProperty_Size, "0.5,-" + Int( Margin + 8 ) + " 0," + Int( RangeHeight - 2 ) );
                range.maximum->SetProperty( EWidgetProperty_Tooltip, "To" );
                m_rangeWidgets.push_back( range );
                TEventCallback changedMin( this, &CAssetSearchForm::OnRangeTextChanged );
                SubscribeTo( range.minimum, CEditbox::TextChangedEvent, changedMin );
                SubscribeTo( range.maximum, CEditbox::TextChangedEvent, changedMin );
                TEventCallback enteredMin( this, &CAssetSearchForm::OnRangeTextEntered );
                SubscribeTo( range.minimum, CEditbox::TextEnteredEvent, enteredMin );
                SubscribeTo( range.maximum, CEditbox::TextEnteredEvent, enteredMin );
            }
            y += RangeHeight;
        }
        y += 4;
    }
    // the room of everything: a spacer at the bottom makes the scroll panel as high as the groups need
    CWidget* spacer = context->CreateWidget( "Panel", "FilterSpacer" + tag, panel );
    if ( GUCEF_NULL != spacer )
    {
        m_filterWidgets.push_back( spacer );
        spacer->SetColourProperty( EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetColourProperty( EWidgetProperty_BorderColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        spacer->SetProperty( EWidgetProperty_Position, "0,0 0," + Int( y > 0 ? y : 0 ) );
        spacer->SetProperty( EWidgetProperty_Size, "0,1 0,1" );
    }
    ShowFacetCounts();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowFacetCounts( void )
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    m_updatingWidgets = true;
    for ( UInt32 i=0; i<(UInt32)m_choiceWidgets.size(); ++i )
    {
        const SChoiceWidget& entry = m_choiceWidgets[ i ];
        if ( entry.group >= (UInt32)groups.size() || entry.choice >= (UInt32)groups[ entry.group ].choices.size() )
            continue;
        const CSearchFilterDescription::TGroup& group = groups[ entry.group ];
        const CSearchFilterDescription::TChoice& choice = group.choices[ entry.choice ];
        CString text = choice.label.IsNULLOrEmpty() ? choice.value : choice.label;
        if ( m_haveResults && m_page.facets.find( group.id ) != m_page.facets.end() )
        {
            UInt64 count = 0;
            m_page.GetFacetCount( group.id, choice.value, count );
            text += " (" + CORE::UInt64ToString( count ) + ")";
        }
        entry.widget->SetProperty( EWidgetProperty_Text, text );
        entry.widget->SetBoolProperty( EWidgetProperty_Checked, m_query.HasChoice( group.id, choice.value ) );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowQueryInWidgets( void )
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    m_updatingWidgets = true;
    CWidget* query = GetWidget( "Query" );
    if ( GUCEF_NULL != query && query->GetProperty( EWidgetProperty_Text ) != m_query.text )
        query->SetProperty( EWidgetProperty_Text, m_query.text );
    for ( UInt32 i=0; i<(UInt32)m_choiceWidgets.size(); ++i )
    {
        const SChoiceWidget& entry = m_choiceWidgets[ i ];
        if ( entry.group < (UInt32)groups.size() && entry.choice < (UInt32)groups[ entry.group ].choices.size() )
            entry.widget->SetBoolProperty( EWidgetProperty_Checked, m_query.HasChoice( groups[ entry.group ].id, groups[ entry.group ].choices[ entry.choice ].value ) );
    }
    for ( UInt32 i=0; i<(UInt32)m_rangeWidgets.size(); ++i )
    {
        const SRangeWidget& entry = m_rangeWidgets[ i ];
        if ( entry.group >= (UInt32)groups.size() )
            continue;
        const CSearchQuery::TRange range = m_query.GetRange( groups[ entry.group ].id );
        entry.minimum->SetProperty( EWidgetProperty_Text, range.hasMinimum ? CORE::ToString( range.minimum ) : CString::Empty );
        entry.maximum->SetProperty( EWidgetProperty_Text, range.hasMaximum ? CORE::ToString( range.maximum ) : CString::Empty );
    }
    CCombobox* sort = GetWidgetAs< CCombobox >( "Sort" );
    if ( GUCEF_NULL != sort && !m_sortChoices.empty() )
    {
        Int32 selected = 0;
        for ( UInt32 i=0; i<(UInt32)m_sortChoices.size(); ++i )
            if ( m_sortChoices[ i ].id == m_query.sort )
                selected = (Int32) i;
        sort->SetSelectedIndex( selected );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

CString
CAssetSearchForm::GetLastWord( const CString& text ) const
{GUCEF_TRACE;

    const Int32 space = text.HasChar( ' ', false );
    return space >= 0 ? text.CutChars( (UInt32) space + 1, true ) : text;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowSuggestions( const CString& text )
{GUCEF_TRACE;

    m_suggestions.clear();
    const CString word = GetLastWord( text ).Trim( true ).Trim( false );
    if ( GUCEF_NULL != m_model && !word.IsNULLOrEmpty() && !text.IsNULLOrEmpty() && ' ' != text[ text.Length() - 1 ] )
        m_model->GetSuggestions( word, (UInt32) MaxSuggestions, m_suggestions );
    CListbox* list = GetWidgetAs< CListbox >( "Suggestions" );
    if ( GUCEF_NULL == list )
        return;
    if ( m_suggestions.empty() )
    {
        HideSuggestions();
        return;
    }
    m_updatingWidgets = true;
    list->SetItems( m_suggestions );
    list->SetSelectedIndex( -1 );
    m_updatingWidgets = false;
    list->SetProperty( EWidgetProperty_Size, "1,-" + Int( 2 * Margin ) + " 0," + Int( (Int32) m_suggestions.size() * SuggestionRowHeight + 4 ) );
    list->SetBoolProperty( EWidgetProperty_Visible, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::HideSuggestions( void )
{GUCEF_TRACE;

    m_suggestions.clear();
    CWidget* list = GetWidget( "Suggestions" );
    if ( GUCEF_NULL != list )
        list->SetBoolProperty( EWidgetProperty_Visible, false );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CAssetSearchForm::GetSuggestionList( void ) const
{GUCEF_TRACE;

    return m_suggestions;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::ChooseSuggestion( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32)m_suggestions.size() )
        return false;
    CWidget* edit = GetWidget( "Query" );
    CString text = GUCEF_NULL != edit ? edit->GetProperty( EWidgetProperty_Text ) : m_query.text;
    const Int32 space = text.HasChar( ' ', false );
    const CString head = space >= 0 ? text.SubstrToIndex( (UInt32) space + 1, true ) : CString::Empty;
    const CString replaced = head + m_suggestions[ index ] + " ";
    HideSuggestions();
    if ( GUCEF_NULL != edit )
    {
        m_updatingWidgets = true;
        edit->SetProperty( EWidgetProperty_Text, replaced );
        m_updatingWidgets = false;
    }
    m_editsPending = true;
    CommitEdits();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::IsValidNumber( const CString& text, Float64& value ) const
{GUCEF_TRACE;

    const CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    char* end = GUCEF_NULL;
    const Float64 parsed = ::strtod( trimmed.C_String(), &end );
    if ( GUCEF_NULL == end || '\0' != *end )
        return false;
    value = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::ReadEdits( void )
{GUCEF_TRACE;

    CSearchQuery next( m_query );
    CWidget* edit = GetWidget( "Query" );
    if ( GUCEF_NULL != edit )
        next.text = edit->GetProperty( EWidgetProperty_Text ).Trim( true ).Trim( false );
    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 i=0; i<(UInt32)m_rangeWidgets.size(); ++i )
    {
        const SRangeWidget& entry = m_rangeWidgets[ i ];
        if ( entry.group >= (UInt32)groups.size() )
            continue;
        CSearchQuery::TRange range;
        Float64 value = 0.0;
        if ( IsValidNumber( entry.minimum->GetProperty( EWidgetProperty_Text ), value ) )
        {
            range.hasMinimum = true;
            range.minimum = value;
        }
        if ( IsValidNumber( entry.maximum->GetProperty( EWidgetProperty_Text ), value ) )
        {
            range.hasMaximum = true;
            range.maximum = value;
        }
        next.SetRange( groups[ entry.group ].id, range );
    }
    if ( next.IsSameSearch( m_query ) )
        return false;
    m_query = next;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::MarkEditsPending( void )
{GUCEF_TRACE;

    m_editsPending = true;
    m_typedWaitMs = 0;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::CommitEdits( void )
{GUCEF_TRACE;

    if ( !m_editsPending )
        return;
    m_editsPending = false;
    m_typedWaitMs = 0;
    if ( ReadEdits() )
    {
        m_chosenSavedSearchId.Clear();
        ChangeSearch();
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ChangeSearch( void )
{GUCEF_TRACE;

    m_query.pageIndex = 0;
    ShowSavedSearches();
    NotifyQueryChanged();
    RunSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ChangePage( UInt32 pageIndex )
{GUCEF_TRACE;

    m_query.pageIndex = pageIndex;
    NotifyQueryChanged();
    RunSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::NotifyQueryChanged( void )
{GUCEF_TRACE;

    m_query.pageSize = m_pageSize;
    TQueryChangedEventData data( m_query );
    NotifyObservers( QueryChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::RunSearch( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_model )
        return;
    m_query.pageSize = m_pageSize;
    m_statusNote = "Searching...";
    m_searching = true;
    m_completedInRequest = false;
    m_inRequest = true;
    ++m_requestCount;
    const UInt32 id = m_model->RequestSearch( m_query );
    m_inRequest = false;
    m_requestId = id;
    if ( m_completedInRequest )
        m_searching = false;
    ShowStatus();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::Search( void )
{GUCEF_TRACE;

    RunSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSearchCompleted( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_model )
        return;
    CISearchModel::TSearchCompletedEventData* data = static_cast< CISearchModel::TSearchCompletedEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const CISearchModel::TSearchCompletion& completion = data->GetData();
    if ( !m_inRequest && completion.requestId != m_requestId )
        return;
    if ( m_inRequest )
        m_completedInRequest = true;
    m_searching = false;
    m_searchFailed = !completion.succeeded;
    if ( !completion.succeeded )
    {
        m_statusNote = completion.message.IsNULLOrEmpty() ? CString( "The search failed" ) : completion.message;
        ShowStatus();
        return;
    }
    m_statusNote = completion.message;
    m_page = completion.page;
    m_haveResults = true;
    if ( m_page.pageSize > 0 )
        m_query.pageIndex = m_page.pageIndex;
    // a selection stays when its result is still there
    if ( !m_selectedKey.IsNULLOrEmpty() && FindRowOfKey( m_selectedKey ) < 0 )
    {
        m_selectedKey.Clear();
        CORE::TCloneableString cleared;
        NotifyObservers( ResultSelectedEvent, &cleared );
    }
    ShowResults();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnFilterDescriptionChanged( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_model || GUCEF_NULL == m_model )
        return;
    m_model->GetFilterDescription( m_description );
    BuildFilterPanel();
    ShowQueryInWidgets();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSuggestionsChanged( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the model has more for the word being typed: asked again only while the person types in the query
    if ( notifier != m_model || GUCEF_NULL == m_model )
        return;
    CWidget* edit = GetWidget( "Query" );
    if ( GUCEF_NULL != edit && edit->HasFocus() )
        ShowSuggestions( edit->GetProperty( EWidgetProperty_Text ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSavedSearchesChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_model || GUCEF_NULL == m_model )
        return;
    m_savedSearches.clear();
    if ( m_model->SupportsSavedSearches() )
        m_model->GetSavedSearches( m_savedSearches );
    if ( FindSavedSearchIndex( m_chosenSavedSearchId ) < 0 )
        m_chosenSavedSearchId.Clear();
    ShowSavedSearches();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnNotify( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventId && GUCEF_NULL != notifier )
    {
        if ( notifier == m_model )
        {
            m_model = GUCEF_NULL;
            m_searching = false;
            return;
        }
        if ( notifier == m_thumbnailProvider )
        {
            m_thumbnailProvider = GUCEF_NULL;
            return;
        }
    }
    CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowResults( void )
{GUCEF_TRACE;

    ShowGridRows();
    m_tilesDirty = true;
    ShowFacetCounts();
    ShowPaging();
    ShowStatus();
    LayoutTiles();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowGridRows( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Grid" );
    if ( GUCEF_NULL == grid )
        return;
    CGridView::TRowVector rows;
    CString colours;
    bool anyUnavailable = false;
    Int32 selectedRow = -1;
    for ( UInt32 r=0; r<(UInt32)m_page.rows.size(); ++r )
    {
        const CSearchResultPage::TRow& row = m_page.rows[ r ];
        CGridView::TStringVector cells;
        for ( UInt32 c=0; c<(UInt32)m_columns.size(); ++c )
        {
            CString text;
            const CString& key = m_columns[ c ].key;
            if ( "title" == key )
                text = row.title;
            else
            if ( "subtitle" == key )
                text = row.subtitle;
            else
            {
                GUCEF::map< CString, CString >::const_iterator v = row.values.find( key );
                if ( v != row.values.end() )
                    text = (*v).second;
            }
            cells.push_back( text.ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' ).ReplaceChar( '\t', ' ' ) );
        }
        rows.push_back( cells );
        colours += ( r > 0 ? "\n" : "" ) + ( row.available ? NormalRowColour : UnavailableRowColour ).ToString();
        anyUnavailable = anyUnavailable || !row.available;
        if ( row.key == m_selectedKey )
            selectedRow = (Int32) r;
    }
    m_updatingWidgets = true;
    grid->SetRows( rows );
    grid->SetProperty( EWidgetProperty_ItemColours, anyUnavailable ? colours : CString::Empty );
    if ( selectedRow >= 0 )
        grid->SetSelectedRow( selectedRow );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowPaging( void )
{GUCEF_TRACE;

    const UInt32 pages = m_page.GetPageCount();
    const UInt32 index = m_page.pageIndex;
    CWidget* previous = GetWidget( "PreviousPage" );
    CWidget* next = GetWidget( "NextPage" );
    CWidget* label = GetWidget( "PageLabel" );
    if ( GUCEF_NULL != previous )
        previous->SetBoolProperty( EWidgetProperty_Enabled, index > 0 );
    if ( GUCEF_NULL != next )
        next->SetBoolProperty( EWidgetProperty_Enabled, index + 1 < pages );
    if ( GUCEF_NULL != label )
        label->SetProperty( EWidgetProperty_Text, pages > 0 ? CORE::UInt32ToString( index + 1 ) + " / " + CORE::UInt32ToString( pages ) : CString::Empty );
}

/*-------------------------------------------------------------------------*/

CString
CAssetSearchForm::GetStatusText( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_model )
        return "No catalogue";
    if ( m_searching )
        return "Searching...";
    if ( m_searchFailed || !m_haveResults )
        return m_statusNote;
    CString text = 0 == m_page.total ? CString( "No results" ) : CORE::UInt64ToString( m_page.total ) + ( 1 == m_page.total ? " result" : " results" );
    if ( !m_statusNote.IsNULLOrEmpty() )
        text += ": " + m_statusNote;
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ShowStatus( void )
{GUCEF_TRACE;

    CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
        status->SetProperty( EWidgetProperty_Text, GetStatusText() );
}

/*-------------------------------------------------------------------------*/

Int32
CAssetSearchForm::FindRowOfKey( const CString& key ) const
{GUCEF_TRACE;

    for ( UInt32 r=0; r<(UInt32)m_page.rows.size(); ++r )
        if ( m_page.rows[ r ].key == key )
            return (Int32) r;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetSearchForm::FindSavedSearchIndex( const CString& searchId ) const
{GUCEF_TRACE;

    if ( searchId.IsNULLOrEmpty() )
        return -1;
    for ( UInt32 i=0; i<(UInt32)m_savedSearches.size(); ++i )
        if ( m_savedSearches[ i ].id == searchId )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SelectRow( Int32 row, bool notify )
{GUCEF_TRACE;

    const CString key = row >= 0 && row < (Int32) m_page.rows.size() ? m_page.rows[ row ].key : CString::Empty;
    if ( key == m_selectedKey )
        return;
    m_selectedKey = key;
    // the grid and the tiles show the selection
    CGridView* grid = GetWidgetAs< CGridView >( "Grid" );
    if ( GUCEF_NULL != grid && grid->GetSelectedRow() != row )
    {
        m_updatingWidgets = true;
        grid->SetSelectedRow( row );
        if ( row >= 0 )
            grid->ScrollToRow( row );
        m_updatingWidgets = false;
    }
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
        if ( m_tiles[ s ].row >= 0 )
            m_tiles[ s ].panel->SetColourProperty( EWidgetProperty_BackgroundColour, m_tiles[ s ].row == row ? TileSelectedColour : TileColour );
    if ( notify )
    {
        CORE::TCloneableString data( m_selectedKey );
        NotifyObservers( ResultSelectedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::SelectResult( const CString& key )
{GUCEF_TRACE;

    const Int32 row = FindRowOfKey( key );
    if ( row < 0 )
        return false;
    SelectRow( row, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAssetSearchForm::GetSelectedKey( void ) const
{GUCEF_TRACE;

    return m_selectedKey;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::OpenResult( const CString& key )
{GUCEF_TRACE;

    if ( FindRowOfKey( key ) < 0 )
        return false;
    CORE::TCloneableString data( key );
    NotifyObservers( ResultOpenedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetQuery( const CSearchQuery& query )
{GUCEF_TRACE;

    m_query = query;
    m_editsPending = false;
    ShowQueryInWidgets();
    ShowSortChoices();
    ShowSavedSearches();
    ChangeSearch();
}

/*-------------------------------------------------------------------------*/

const CSearchQuery&
CAssetSearchForm::GetQuery( void ) const
{GUCEF_TRACE;

    return m_query;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetText( const CString& text )
{GUCEF_TRACE;

    CWidget* edit = GetWidget( "Query" );
    if ( GUCEF_NULL != edit )
    {
        m_updatingWidgets = true;
        edit->SetProperty( EWidgetProperty_Text, text );
        m_updatingWidgets = false;
    }
    m_editsPending = true;
    if ( GUCEF_NULL == edit && m_query.text != text )
        m_query.text = text;
    CommitEdits();
    if ( GUCEF_NULL == edit )
    {
        m_chosenSavedSearchId.Clear();
        ChangeSearch();
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::SetChoice( const CString& groupId, const CString& value, bool chosen )
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroup* group = m_description.FindGroup( groupId );
    if ( GUCEF_NULL == group || CSearchFilterDescription::FILTERKIND_CHOICES != group->kind )
        return false;
    if ( m_query.HasChoice( groupId, value ) == chosen )
        return true;
    if ( chosen )
        m_query.AddChoice( groupId, value );
    else
        m_query.RemoveChoice( groupId, value );
    m_chosenSavedSearchId.Clear();
    ShowQueryInWidgets();
    ChangeSearch();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::SetRange( const CString& groupId, const CSearchQuery::TRange& range )
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroup* group = m_description.FindGroup( groupId );
    if ( GUCEF_NULL == group || CSearchFilterDescription::FILTERKIND_RANGE != group->kind )
        return false;
    m_query.SetRange( groupId, range );
    m_chosenSavedSearchId.Clear();
    ShowQueryInWidgets();
    ChangeSearch();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::ClearFilters( void )
{GUCEF_TRACE;

    m_query.text.Clear();
    m_query.choices.clear();
    m_query.ranges.clear();
    m_chosenSavedSearchId.Clear();
    m_editsPending = false;
    ShowQueryInWidgets();
    ChangeSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetPageSize( UInt32 pageSize )
{GUCEF_TRACE;

    m_pageSize = pageSize;
    m_query.pageSize = pageSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSearchForm::GetPageSize( void ) const
{GUCEF_TRACE;

    return m_pageSize;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::GoToPage( UInt32 pageIndex )
{GUCEF_TRACE;

    if ( !m_haveResults || pageIndex >= m_page.GetPageCount() || pageIndex == m_page.pageIndex )
        return false;
    ChangePage( pageIndex );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::NextPage( void )
{GUCEF_TRACE;

    return GoToPage( m_page.pageIndex + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::PreviousPage( void )
{GUCEF_TRACE;

    return m_page.pageIndex > 0 && GoToPage( m_page.pageIndex - 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSearchForm::GetRequestCount( void ) const
{GUCEF_TRACE;

    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::IsSearching( void ) const
{GUCEF_TRACE;

    return m_searching;
}

/*-------------------------------------------------------------------------*/

const CSearchResultPage&
CAssetSearchForm::GetPage( void ) const
{GUCEF_TRACE;

    return m_page;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSearchForm::GetShownRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_page.rows.size();
}

/*-------------------------------------------------------------------------*/

const CSearchResultPage::TRow&
CAssetSearchForm::GetShownRow( UInt32 index ) const
{GUCEF_TRACE;

    static const CSearchResultPage::TRow none;
    return index < (UInt32) m_page.rows.size() ? m_page.rows[ index ] : none;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetThumbnailView( bool thumbnails )
{GUCEF_TRACE;

    m_thumbnailView = thumbnails;
    CWidget* grid = GetWidget( "Grid" );
    CWidget* tiles = GetWidget( "Tiles" );
    if ( GUCEF_NULL != grid )
        grid->SetBoolProperty( EWidgetProperty_Visible, !thumbnails );
    if ( GUCEF_NULL != tiles )
        tiles->SetBoolProperty( EWidgetProperty_Visible, thumbnails );
    CWidget* box = GetWidget( "TilesToggle" );
    if ( GUCEF_NULL != box && box->GetBoolProperty( EWidgetProperty_Checked ) != thumbnails )
    {
        m_updatingWidgets = true;
        box->SetBoolProperty( EWidgetProperty_Checked, thumbnails );
        m_updatingWidgets = false;
    }
    m_tilesDirty = true;
    LayoutTiles();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::IsThumbnailView( void ) const
{GUCEF_TRACE;

    return m_thumbnailView;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetThumbnailSize( UInt32 size )
{GUCEF_TRACE;

    if ( size < 32 )
        size = 32;
    if ( size == m_thumbnailSize )
        return;
    m_thumbnailSize = size;
    m_requestedThumbnails.clear();
    m_tilesDirty = true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSearchForm::GetThumbnailSize( void ) const
{GUCEF_TRACE;

    return m_thumbnailSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSearchForm::GetRealizedTileCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)m_tiles.size(); ++i )
        if ( m_tiles[ i ].row >= 0 )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::HasThumbnail( const CString& thumbnailKey ) const
{GUCEF_TRACE;

    GUCEF::map< CString, IMAGE::TPixelMapPtr >::const_iterator i = m_thumbnails.find( thumbnailKey );
    return i != m_thumbnails.end() && !(*i).second.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::SetFiltersVisible( bool visible )
{GUCEF_TRACE;

    m_filtersVisible = visible;
    CWidget* box = GetWidget( "FiltersToggle" );
    if ( GUCEF_NULL != box && box->GetBoolProperty( EWidgetProperty_Checked ) != visible )
    {
        m_updatingWidgets = true;
        box->SetBoolProperty( EWidgetProperty_Checked, visible );
        m_updatingWidgets = false;
    }
    ArrangeAreas();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::AreFiltersVisible( void ) const
{GUCEF_TRACE;

    return m_filtersVisible;
}

/*-------------------------------------------------------------------------*/

CWidget*
CAssetSearchForm::GetChoiceWidget( const CString& groupId, const CString& value ) const
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 i=0; i<(UInt32)m_choiceWidgets.size(); ++i )
    {
        const SChoiceWidget& entry = m_choiceWidgets[ i ];
        if ( entry.group < (UInt32)groups.size() && groups[ entry.group ].id == groupId && entry.choice < (UInt32)groups[ entry.group ].choices.size() &&
             groups[ entry.group ].choices[ entry.choice ].value == value )
            return entry.widget;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString
CAssetSearchForm::GetChoiceText( const CString& groupId, const CString& value ) const
{GUCEF_TRACE;

    CWidget* widget = GetChoiceWidget( groupId, value );
    return GUCEF_NULL != widget ? widget->GetProperty( EWidgetProperty_Text ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CWidget*
CAssetSearchForm::GetRangeMinimumWidget( const CString& groupId ) const
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 i=0; i<(UInt32)m_rangeWidgets.size(); ++i )
        if ( m_rangeWidgets[ i ].group < (UInt32)groups.size() && groups[ m_rangeWidgets[ i ].group ].id == groupId )
            return m_rangeWidgets[ i ].minimum;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CAssetSearchForm::GetRangeMaximumWidget( const CString& groupId ) const
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 i=0; i<(UInt32)m_rangeWidgets.size(); ++i )
        if ( m_rangeWidgets[ i ].group < (UInt32)groups.size() && groups[ m_rangeWidgets[ i ].group ].id == groupId )
            return m_rangeWidgets[ i ].maximum;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CAssetSearchForm::GetSavedSearchNames( void ) const
{GUCEF_TRACE;

    CString::StringVector names;
    for ( UInt32 i=0; i<(UInt32)m_savedSearches.size(); ++i )
        names.push_back( m_savedSearches[ i ].name );
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::ChooseSavedSearch( const CString& searchId )
{GUCEF_TRACE;

    const Int32 index = FindSavedSearchIndex( searchId );
    if ( index < 0 )
        return false;
    const CSearchQuery chosen = m_savedSearches[ index ].query;
    m_query = chosen;
    m_query.pageIndex = 0;
    m_editsPending = false;
    m_chosenSavedSearchId = searchId;
    ShowQueryInWidgets();
    ShowSortChoices();
    ShowSavedSearches();
    CORE::TCloneableString data( searchId );
    NotifyObservers( SavedSearchChosenEvent, &data );
    NotifyQueryChanged();
    RunSearch();
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAssetSearchForm::GetChosenSavedSearchId( void ) const
{GUCEF_TRACE;

    return m_chosenSavedSearchId;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::SaveCurrentSearch( const CString& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_model || !m_model->SupportsSavedSearches() || name.IsNULLOrEmpty() )
        return false;
    CommitEdits();
    CSavedSearch search;
    search.name = name;
    search.query = m_query;
    search.query.pageIndex = 0;
    // a name that is there is replaced when it is not built in
    for ( UInt32 i=0; i<(UInt32)m_savedSearches.size(); ++i )
        if ( m_savedSearches[ i ].name == name && !m_savedSearches[ i ].builtIn )
            search.id = m_savedSearches[ i ].id;
    if ( !m_model->SaveSearch( search ) )
        return false;
    m_savedSearches.clear();
    m_model->GetSavedSearches( m_savedSearches );
    for ( UInt32 i=0; i<(UInt32)m_savedSearches.size(); ++i )
        if ( m_savedSearches[ i ].name == name && !m_savedSearches[ i ].builtIn )
            m_chosenSavedSearchId = m_savedSearches[ i ].id;
    ShowSavedSearches();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::DeleteChosenSavedSearch( void )
{GUCEF_TRACE;

    const Int32 index = FindSavedSearchIndex( m_chosenSavedSearchId );
    if ( GUCEF_NULL == m_model || index < 0 || m_savedSearches[ index ].builtIn )
        return false;
    if ( !m_model->DeleteSavedSearch( m_chosenSavedSearchId ) )
        return false;
    m_chosenSavedSearchId.Clear();
    m_savedSearches.clear();
    m_model->GetSavedSearches( m_savedSearches );
    ShowSavedSearches();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::LayoutTiles( void )
{GUCEF_TRACE;

    CScrollPanel* panel = CWidget::ToScrollPanel( GetWidget( "Tiles" ) );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == panel || GUCEF_NULL == context || !m_thumbnailView )
        return;
    const CRect viewport = panel->GetViewportRect();
    if ( viewport.width <= 0 || viewport.height <= 0 )
        return;
    const Int32 size = (Int32) m_thumbnailSize;
    const Int32 cellWidth = size + 2 * TilePadding;
    const Int32 cellHeight = size + TileLabelHeight + 2 * TilePadding;
    const Int32 columns = viewport.width / cellWidth > 0 ? viewport.width / cellWidth : 1;
    const Int32 count = (Int32) m_page.rows.size();
    const Int32 rows = ( count + columns - 1 ) / columns;
    m_tilesPerRow = (UInt32) columns;

    CWidget* spacer = panel->GetChildWidget( "TileSpacer" );
    if ( GUCEF_NULL == spacer )
    {
        spacer = context->CreateWidget( "Panel", "TileSpacer", panel );
        spacer->SetColourProperty( EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetColourProperty( EWidgetProperty_BorderColour, CColour( 0, 0, 0, 0 ) );
        spacer->SetBoolProperty( EWidgetProperty_InputTransparent, true );
    }
    spacer->SetProperty( EWidgetProperty_Position, "0,0 0," + Int( rows > 0 ? rows * cellHeight - 1 : 0 ) );
    spacer->SetProperty( EWidgetProperty_Size, "0,1 0,1" );
    panel->SetIntProperty( EWidgetProperty_ScrollStep, cellHeight / 2 );

    while ( (Int32) m_tiles.size() < count )
    {
        CWidget* tile = context->CreateWidget( "Panel", "Tile_" + CORE::UInt32ToString( (UInt32) m_tiles.size() ), panel );
        if ( GUCEF_NULL == tile )
            break;
        tile->SetBoolProperty( EWidgetProperty_Clip, true );
        CWidget* image = context->CreateWidget( "ImageFrame", "Image", tile );
        image->SetProperty( EWidgetProperty_ScaleMode, "Fit" );
        image->SetBoolProperty( EWidgetProperty_Smooth, true );
        image->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        CWidget* kind = context->CreateWidget( "Label", "Kind", tile );
        kind->SetProperty( EWidgetProperty_HTextAlign, "Center" );
        kind->SetProperty( EWidgetProperty_VTextAlign, "Center" );
        kind->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        CWidget* name = context->CreateWidget( "Label", "Name", tile );
        name->SetProperty( EWidgetProperty_HTextAlign, "Center" );
        name->SetBoolProperty( EWidgetProperty_WordWrap, true );
        name->SetBoolProperty( EWidgetProperty_InputTransparent, true );
        STile entry;
        entry.panel = tile;
        entry.row = -1;
        m_tiles.push_back( entry );
    }
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( (Int32) s < count )
            BindTile( s, (Int32) s, cellWidth, cellHeight, columns );
        else
        {
            m_tiles[ s ].row = -1;
            m_tiles[ s ].panel->SetBoolProperty( EWidgetProperty_Visible, false );
        }
    }
    m_lastViewportWidth = viewport.width;
    m_lastViewportHeight = viewport.height;
    m_lastScroll = panel->GetScrollPosition();
    m_tilesDirty = false;
    RequestTileThumbnails();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::BindTile( UInt32 slot, Int32 row, Int32 cellWidth, Int32 cellHeight, Int32 columns )
{GUCEF_TRACE;

    STile& tile = m_tiles[ slot ];
    const CSearchResultPage::TRow& entry = m_page.rows[ row ];
    const Int32 size = (Int32) m_thumbnailSize;
    tile.row = row;
    CWidget* panel = tile.panel;
    panel->SetProperty( EWidgetProperty_Position, "0," + Int( ( row % columns ) * cellWidth ) + " 0," + Int( ( row / columns ) * cellHeight ) );
    panel->SetProperty( EWidgetProperty_Size, "0," + Int( cellWidth - 2 ) + " 0," + Int( cellHeight - 2 ) );
    panel->SetProperty( EWidgetProperty_Tooltip, entry.tooltip.IsNULLOrEmpty() ? entry.title : entry.tooltip );
    panel->SetBoolProperty( EWidgetProperty_Visible, true );
    CWidget* image = panel->GetChildWidget( "Image" );
    CWidget* kind = panel->GetChildWidget( "Kind" );
    CWidget* name = panel->GetChildWidget( "Name" );
    image->SetProperty( EWidgetProperty_Position, "0," + Int( TilePadding ) + " 0," + Int( TilePadding ) );
    image->SetProperty( EWidgetProperty_Size, "0," + Int( size ) + " 0," + Int( size ) );
    kind->SetProperty( EWidgetProperty_Position, "0," + Int( TilePadding ) + " 0," + Int( TilePadding + size / 2 - 10 ) );
    kind->SetProperty( EWidgetProperty_Size, "0," + Int( size ) + " 0,20" );
    name->SetProperty( EWidgetProperty_Position, "0," + Int( TilePadding ) + " 0," + Int( TilePadding + size + 2 ) );
    name->SetProperty( EWidgetProperty_Size, "0," + Int( size ) + " 0," + Int( TileLabelHeight ) );
    name->SetProperty( EWidgetProperty_Text, entry.title );
    CImageFrame* frame = image->IsRegisteredAs( "ImageFrame" ) ? static_cast< CImageFrame* >( image ) : GUCEF_NULL;
    GUCEF::map< CString, IMAGE::TPixelMapPtr >::const_iterator thumbnail = m_thumbnails.find( entry.thumbnailKey );
    bool pictured = false;
    if ( thumbnail != m_thumbnails.end() && !(*thumbnail).second.IsNULL() && GUCEF_NULL != frame )
    {
        frame->SetImage( (*thumbnail).second );
        pictured = true;
    }
    else
    if ( GUCEF_NULL != frame )
        frame->SetImageName( CString::Empty );
    image->SetBoolProperty( EWidgetProperty_Visible, pictured );
    kind->SetBoolProperty( EWidgetProperty_Visible, !pictured );
    kind->SetProperty( EWidgetProperty_Text, entry.typeName );
    panel->SetColourProperty( EWidgetProperty_BackgroundColour, entry.key == m_selectedKey ? TileSelectedColour : TileColour );
    panel->SetFloatProperty( EWidgetProperty_Opacity, entry.available ? 1.0f : 0.5f );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::RequestTileThumbnails( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_thumbnailProvider )
        return;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].row < 0 )
            continue;
        const CString& key = m_page.rows[ m_tiles[ s ].row ].thumbnailKey;
        if ( key.IsNULLOrEmpty() || m_thumbnails.find( key ) != m_thumbnails.end() || m_requestedThumbnails.find( key ) != m_requestedThumbnails.end() )
            continue;
        m_requestedThumbnails.insert( key );
        m_thumbnailProvider->RequestThumbnail( key, m_thumbnailSize );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( m_editsPending )
    {
        m_typedWaitMs += deltaInMs;
        if ( m_typedWaitMs >= (UInt32) TypingDelayInMs )
            CommitEdits();
    }
    if ( m_thumbnailView )
    {
        CScrollPanel* panel = CWidget::ToScrollPanel( GetWidget( "Tiles" ) );
        if ( GUCEF_NULL != panel )
        {
            const CRect viewport = panel->GetViewportRect();
            if ( m_tilesDirty || viewport.width != m_lastViewportWidth || viewport.height != m_lastViewportHeight )
                LayoutTiles();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnThumbnailReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIThumbnailProvider::TThumbnailReadyEventData* ready = static_cast< CIThumbnailProvider::TThumbnailReadyEventData* >( eventData );
    if ( GUCEF_NULL == ready || ready->GetData().size != m_thumbnailSize )
        return;
    const CString& key = ready->GetData().path;
    m_thumbnails[ key ] = ready->GetData().pixels;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].row >= 0 && m_page.rows[ m_tiles[ s ].row ].thumbnailKey == key )
        {
            const Int32 size = (Int32) m_thumbnailSize;
            BindTile( s, m_tiles[ s ].row, size + 2 * TilePadding, size + TileLabelHeight + 2 * TilePadding, (Int32) m_tilesPerRow );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnQueryTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CWidget* edit = GetWidget( "Query" );
    MarkEditsPending();
    if ( GUCEF_NULL != edit )
        ShowSuggestions( edit->GetProperty( EWidgetProperty_Text ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnQueryTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    HideSuggestions();
    m_editsPending = true;
    CommitEdits();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnRangeTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        MarkEditsPending();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnRangeTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_editsPending = true;
    CommitEdits();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSavedValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CCombobox* saved = GetWidgetAs< CCombobox >( "Saved" );
    if ( GUCEF_NULL == saved )
        return;
    const Int32 selected = saved->GetSelectedIndex();
    if ( selected >= 1 && selected <= (Int32) m_savedSearches.size() )
        ChooseSavedSearch( m_savedSearches[ selected - 1 ].id );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSortValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CCombobox* sort = GetWidgetAs< CCombobox >( "Sort" );
    if ( GUCEF_NULL == sort )
        return;
    const Int32 selected = sort->GetSelectedIndex();
    if ( selected >= 0 && selected < (Int32) m_sortChoices.size() && m_sortChoices[ selected ].id != m_query.sort )
    {
        m_query.sort = m_sortChoices[ selected ].id;
        ChangeSearch();
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnFiltersChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetFiltersVisible( true );
}

void
CAssetSearchForm::OnFiltersUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetFiltersVisible( false );
}

void
CAssetSearchForm::OnTilesChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetThumbnailView( true );
}

void
CAssetSearchForm::OnTilesUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        SetThumbnailView( false );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return;
    if ( GUCEF_NULL == m_nameBox )
    {
        m_nameBox = GUCEF_NEW CMessageBoxForm( GetContext() );
        if ( !m_nameBox->Build() )
            return;
        TEventCallback callback( this, &CAssetSearchForm::OnSaveNameClosed );
        SubscribeTo( m_nameBox, CMessageBoxForm::ClosedEvent, callback );
    }
    m_nameBox->OpenInput( "Save search", "Name of the search", m_query.text, CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSaveNameClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_nameBox && CMessageBoxForm::MESSAGEBOXRESULT_OK == m_nameBox->GetResult() )
        SaveCurrentSearch( m_nameBox->GetInputText().Trim( true ).Trim( false ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnDeleteActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    DeleteChosenSavedSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnPreviousActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    PreviousPage();
}

void
CAssetSearchForm::OnNextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NextPage();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnSuggestionValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CListbox* list = GetWidgetAs< CListbox >( "Suggestions" );
    if ( GUCEF_NULL != list && list->GetSelectedIndex() >= 0 )
        ChooseSuggestion( (UInt32) list->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchForm::ChangeChoice( CORE::CNotifier* box, bool chosen )
{GUCEF_TRACE;

    const CSearchFilterDescription::TGroupVector& groups = m_description.GetGroups();
    for ( UInt32 i=0; i<(UInt32)m_choiceWidgets.size(); ++i )
    {
        if ( box == m_choiceWidgets[ i ].widget )
        {
            const SChoiceWidget& entry = m_choiceWidgets[ i ];
            if ( entry.group >= (UInt32)groups.size() )
                return false;
            const CSearchFilterDescription::TGroup& group = groups[ entry.group ];
            if ( entry.choice >= (UInt32)group.choices.size() )
                return false;
            if ( chosen )
                m_query.AddChoice( group.id, group.choices[ entry.choice ].value );
            else
                m_query.RemoveChoice( group.id, group.choices[ entry.choice ].value );
            m_chosenSavedSearchId.Clear();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnChoiceChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets && ChangeChoice( notifier, true ) )
        ChangeSearch();
}

void
CAssetSearchForm::OnChoiceUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets && ChangeChoice( notifier, false ) )
        ChangeSearch();
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnGridSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Grid" );
    if ( GUCEF_NULL != grid )
        SelectRow( grid->GetSelectedRow(), true );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnGridItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRoutedEventData* routed = static_cast< CRoutedEventData* >( eventData );
    if ( GUCEF_NULL != routed && EVENTPHASE_TUNNEL == routed->phase )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Grid" );
    if ( GUCEF_NULL == grid )
        return;
    const Int32 row = grid->GetSelectedRow();
    if ( row >= 0 && row < (Int32) m_page.rows.size() )
        OpenResult( m_page.rows[ row ].key );
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnTilesMouseClick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL == pointer || MOUSEBUTTON_LEFT != pointer->button || EVENTPHASE_TUNNEL == pointer->phase )
        return;
    CWidget* tiles = GetWidget( "Tiles" );
    CWidget* hit = pointer->source;
    while ( GUCEF_NULL != hit && hit->GetParentWidget() != tiles )
        hit = hit->GetParentWidget();
    if ( GUCEF_NULL == hit )
        return;
    for ( UInt32 s=0; s<(UInt32)m_tiles.size(); ++s )
    {
        if ( m_tiles[ s ].panel == hit && m_tiles[ s ].row >= 0 )
        {
            if ( GUCEF_NULL != tiles )
                tiles->SetFocus();
            SelectRow( m_tiles[ s ].row, true );
            if ( pointer->clickCount >= 2 )
                OpenResult( m_page.rows[ m_tiles[ s ].row ].key );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetSearchForm::OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( CRoutedEventData::FromCloneable( eventData ) );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_BUBBLE == key->phase )
        return;
    if ( INPUT::KEYCODE_ESCAPE == key->keyCode )
        HideSuggestions();
    else
    if ( INPUT::KEYCODE_RETURN == key->keyCode && !m_selectedKey.IsNULLOrEmpty() && !m_editsPending )
        OpenResult( m_selectedKey );
}

/*-------------------------------------------------------------------------*/

const CString&
CAssetSearchForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CAssetSearchForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
