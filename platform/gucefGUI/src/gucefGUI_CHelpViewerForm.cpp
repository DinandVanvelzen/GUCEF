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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#include "gucefGUI_CHelpViewerForm.h"

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

const CORE::CEvent CHelpViewerForm::TopicShownEvent = "GUCEF::GUI::CHelpViewerForm::TopicShownEvent";
const CORE::CEvent CHelpViewerForm::ClosedEvent = "GUCEF::GUI::CHelpViewerForm::ClosedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether a routed event is at the widget it is for (a button or a list), not passing it on the way to another */
bool
IsAtTarget( CORE::CICloneable* eventData )
{
    const CRoutedEventData* routed = CRoutedEventData::FromCloneable( eventData );
    return GUCEF_NULL == routed || EVENTPHASE_TARGET == routed->phase;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CHelpViewerForm::RegisterEvents( void )
{GUCEF_TRACE;

    TopicShownEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CHelpViewerForm::CHelpViewerForm( void )
    : CFormEx()
    , m_source()
    , m_history()
    , m_document()
    , m_currentTopic()
    , m_found( false )
    , m_text()
    , m_links()
    , m_index()
    , m_open( false )
    , m_asWindow( false )
    , m_updatingWidgets( false )
    , m_linkClickPending( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CHelpViewerForm::CHelpViewerForm( CIGUIContext* context )
    : CFormEx( context )
    , m_source()
    , m_history()
    , m_document()
    , m_currentTopic()
    , m_found( false )
    , m_text()
    , m_links()
    , m_index()
    , m_open( false )
    , m_asWindow( false )
    , m_updatingWidgets( false )
    , m_linkClickPending( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CHelpViewerForm::~CHelpViewerForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CString&
CHelpViewerForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CHelpViewerForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "HelpViewerForm", "0.12,0 0.08,0", "0.76,0 0.84,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "HelpViewerForm", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* back = CFormLayout::AddWidgetNode( *content, "Button", "Back", "0,8 0,6", "0,90 0,30" );
    CFormLayout::SetWidgetNodeProperty( *back, EWidgetProperty_Text, "Back" );
    CFormLayout::SetWidgetNodeProperty( *back, EWidgetProperty_Tooltip, "The topic read before this one (Alt+Left)" );
    CORE::CDataNode* forward = CFormLayout::AddWidgetNode( *content, "Button", "Forward", "0,104 0,6", "0,90 0,30" );
    CFormLayout::SetWidgetNodeProperty( *forward, EWidgetProperty_Text, "Forward" );
    CFormLayout::SetWidgetNodeProperty( *forward, EWidgetProperty_Tooltip, "The topic read after this one (Alt+Right)" );
    CORE::CDataNode* topicTitle = CFormLayout::AddWidgetNode( *content, "Label", "TopicTitle", "0,206 0,6", "1,-214 0,30" );
    CFormLayout::SetWidgetNodeProperty( *topicTitle, EWidgetProperty_StyleClass, "Heading" );
    CFormLayout::SetWidgetNodeProperty( *topicTitle, EWidgetProperty_VTextAlign, "Center" );

    CORE::CDataNode* topics = CFormLayout::AddWidgetNode( *content, "Listbox", "Topics", "0,8 0,44", "0.28,-12 1,-96" );
    CFormLayout::SetWidgetNodeProperty( *topics, EWidgetProperty_TextOverflow, "Ellipsis" );
    CORE::CDataNode* document = CFormLayout::AddWidgetNode( *content, "Textbox", "Document", "0.28,4 0,44", "0.72,-12 0.72,-52" );
    CFormLayout::SetWidgetNodeProperty( *document, EWidgetProperty_ReadOnly, "true" );
    CFormLayout::SetWidgetNodeProperty( *document, EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* linksTitle = CFormLayout::AddWidgetNode( *content, "Label", "LinksTitle", "0.28,4 0.72,-4", "0.72,-12 0,24" );
    CFormLayout::SetWidgetNodeProperty( *linksTitle, EWidgetProperty_Text, "See also" );
    CORE::CDataNode* links = CFormLayout::AddWidgetNode( *content, "Listbox", "Links", "0.28,4 0.72,22", "0.72,-12 0.28,-74" );
    CFormLayout::SetWidgetNodeProperty( *links, EWidgetProperty_TextOverflow, "Ellipsis" );

    CORE::CDataNode* reviewed = CFormLayout::AddWidgetNode( *content, "Label", "Reviewed", "0,8 1,-44", "1,-140 0,28" );
    CFormLayout::SetWidgetNodeProperty( *reviewed, EWidgetProperty_StyleClass, "Dim" );
    CFormLayout::SetWidgetNodeProperty( *reviewed, EWidgetProperty_VTextAlign, "Center" );
    CORE::CDataNode* close = CFormLayout::AddWidgetNode( *content, "Button", "Close", "1,-120 1,-44", "0,110 0,34" );
    CFormLayout::SetWidgetNodeProperty( *close, EWidgetProperty_Text, "Close" );
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    m_asWindow = true;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    m_asWindow = false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Back" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CHelpViewerForm::OnBackActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Forward" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CHelpViewerForm::OnForwardActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Close" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CHelpViewerForm::OnCloseActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Topics" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CHelpViewerForm::OnTopicsSelectionChanged );
        SubscribeTo( widget, CListbox::SelectionChangedEvent, callback );
    }
    widget = GetWidget( "Links" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback clicked( this, &CHelpViewerForm::OnLinksClicked );
        SubscribeTo( widget, CListbox::ItemClickedEvent, clicked );
        TEventCallback selection( this, &CHelpViewerForm::OnLinksSelectionChanged );
        SubscribeTo( widget, CListbox::SelectionChangedEvent, selection );
        TEventCallback activated( this, &CHelpViewerForm::OnLinksActivated );
        SubscribeTo( widget, CWidget::ItemActivatedEvent, activated );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigation( this, &CHelpViewerForm::OnNavigation );
        SubscribeTo( root, CWidget::NavigationEvent, navigation );
        TEventCallback keys( this, &CHelpViewerForm::OnKeyDown );
        SubscribeTo( root, CWidget::KeyDownEvent, keys );
    }
    FillIndex();
    ShowCurrent();
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::SetHelpSource( const THelpSourcePtr& source )
{GUCEF_TRACE;

    m_source = source;
    m_history.Clear();
    m_document.Clear();
    m_currentTopic.Clear();
    m_found = false;
    m_text.Clear();
    m_links.clear();
    FillIndex();
    ShowCurrent();
}

/*-------------------------------------------------------------------------*/

const THelpSourcePtr&
CHelpViewerForm::GetHelpSource( void ) const
{GUCEF_TRACE;

    return m_source;
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::LoadTopic( const CString& topicId )
{GUCEF_TRACE;

    m_currentTopic = topicId;
    m_document.Clear();
    m_links.clear();
    m_found = !m_source.IsNULL() && m_source->GetTopic( topicId, m_document );
    if ( !m_found )
    {
        m_document.Clear();
        m_document.id = topicId;
        m_document.title = "Not found";
        m_text = "There is no help topic \"" + topicId + "\".";
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "HelpViewerForm: no help topic " << topicId );
        return;
    }
    m_text = m_document.ToPlainText( m_source.GetPointerAlways() );
    m_document.CollectLinks( m_links );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::FillIndex( void )
{GUCEF_TRACE;

    m_index.clear();
    if ( !m_source.IsNULL() )
        m_source->ListTopics( m_index );
    CListbox* topics = GetWidgetAs< CListbox >( "Topics" );
    if ( GUCEF_NULL == topics )
        return;
    CListbox::TStringVector items;
    for ( UInt32 i=0; i<(UInt32)m_index.size(); ++i )
        items.push_back( m_index[ i ].group.IsNULLOrEmpty() ? m_index[ i ].title : m_index[ i ].group + " - " + m_index[ i ].title );
    m_updatingWidgets = true;
    topics->SetItems( items );
    topics->SetSelectedIndex( -1 );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::ShowCurrent( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    m_updatingWidgets = true;
    CWidget* widget = GetWidget( "TopicTitle" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( EWidgetProperty_Text, m_document.title );
    widget = GetWidget( "Document" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( EWidgetProperty_Text, m_text );

    CListbox* links = GetWidgetAs< CListbox >( "Links" );
    if ( GUCEF_NULL != links )
    {
        CListbox::TStringVector items;
        for ( UInt32 i=0; i<(UInt32)m_links.size(); ++i )
            items.push_back( m_source.IsNULL() ? m_links[ i ] : m_source->GetTopicTitle( m_links[ i ] ) );
        links->SetItems( items );
        links->SetSelectedIndex( -1 );
    }
    widget = GetWidget( "LinksTitle" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( !m_links.empty() );

    CString reviewed;
    if ( !m_document.version.IsNULLOrEmpty() )
        reviewed = "Version " + m_document.version;
    if ( !m_document.reviewed.IsNULLOrEmpty() )
        reviewed += ( reviewed.IsNULLOrEmpty() ? CString( "Reviewed " ) : CString( ", reviewed " ) ) + m_document.reviewed;
    widget = GetWidget( "Reviewed" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( EWidgetProperty_Text, reviewed );

    widget = GetWidget( "Back" );
    if ( GUCEF_NULL != widget )
        widget->SetEnabled( m_history.CanGoBack() );
    widget = GetWidget( "Forward" );
    if ( GUCEF_NULL != widget )
        widget->SetEnabled( m_history.CanGoForward() );

    // the index shows where the reader is
    CListbox* topics = GetWidgetAs< CListbox >( "Topics" );
    if ( GUCEF_NULL != topics )
    {
        Int32 selected = -1;
        for ( UInt32 i=0; i<(UInt32)m_index.size() && m_found; ++i )
            if ( m_index[ i ].id == m_currentTopic )
                selected = (Int32) i;
        topics->SetSelectedIndex( selected );
        if ( selected >= 0 )
            topics->ScrollToRow( selected );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::Open( const CString& topicId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    FillIndex();
    ShowTopic( topicId );
    if ( m_asWindow && !m_open && !ShowModal() )
        return false;
    m_open = true;
    CGUIContext* context = GetGUIContext();
    CWidget* document = GetWidget( "Document" );
    if ( GUCEF_NULL != context && GUCEF_NULL != document && document->IsFocusable() )
        context->SetFocus( document );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::ShowTopic( const CString& topicId )
{GUCEF_TRACE;

    LoadTopic( topicId );
    if ( m_found )
        m_history.Visit( topicId );
    ShowCurrent();
    CORE::TCloneableString eventData( topicId );
    NotifyObservers( TopicShownEvent, &eventData );
    return m_found;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::GoBack( void )
{GUCEF_TRACE;

    if ( !m_history.Back() )
        return false;
    LoadTopic( m_history.GetCurrent() );
    ShowCurrent();
    CORE::TCloneableString eventData( m_currentTopic );
    NotifyObservers( TopicShownEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::GoForward( void )
{GUCEF_TRACE;

    if ( !m_history.Forward() )
        return false;
    LoadTopic( m_history.GetCurrent() );
    ShowCurrent();
    CORE::TCloneableString eventData( m_currentTopic );
    NotifyObservers( TopicShownEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::OpenLink( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_links.size() )
        return false;
    const CString topic = m_links[ index ];
    return ShowTopic( topic );
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::OpenIndexEntry( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_index.size() )
        return false;
    const CString topic = m_index[ index ].id;
    return ShowTopic( topic );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::Close( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_open = false;
    if ( m_asWindow )
        Hide();
    NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

const CString&
CHelpViewerForm::GetCurrentTopic( void ) const
{GUCEF_TRACE;

    return m_currentTopic;
}

/*-------------------------------------------------------------------------*/

bool
CHelpViewerForm::IsCurrentTopicFound( void ) const
{GUCEF_TRACE;

    return m_found;
}

/*-------------------------------------------------------------------------*/

const CHelpDocument&
CHelpViewerForm::GetCurrentDocument( void ) const
{GUCEF_TRACE;

    return m_document;
}

/*-------------------------------------------------------------------------*/

const CString&
CHelpViewerForm::GetDocumentText( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------*/

const CHelpViewerForm::TStringVector&
CHelpViewerForm::GetLinkTopics( void ) const
{GUCEF_TRACE;

    return m_links;
}

/*-------------------------------------------------------------------------*/

const CIHelpSource::TTopicEntryVector&
CHelpViewerForm::GetIndex( void ) const
{GUCEF_TRACE;

    return m_index;
}

/*-------------------------------------------------------------------------*/

CHelpHistory&
CHelpViewerForm::GetHistory( void )
{GUCEF_TRACE;

    return m_history;
}

/*-------------------------------------------------------------------------*/

Int32
CHelpViewerForm::GetListSelection( const CString& listName )
{GUCEF_TRACE;

    CListbox* list = GetWidgetAs< CListbox >( listName );
    return GUCEF_NULL != list ? list->GetSelectedIndex() : -1;
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnBackActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsAtTarget( eventData ) )
        GoBack();
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnForwardActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsAtTarget( eventData ) )
        GoForward();
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsAtTarget( eventData ) )
        Close();
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnTopicsSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    const Int32 selected = GetListSelection( "Topics" );
    if ( selected >= 0 && (size_t) selected < m_index.size() && m_index[ (size_t) selected ].id != m_currentTopic )
        OpenIndexEntry( (UInt32) selected );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnLinksClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the selection follows the click: the link opens when it is selected
    m_linkClickPending = true;
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnLinksSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets || !m_linkClickPending )
        return;
    m_linkClickPending = false;
    const Int32 selected = GetListSelection( "Links" );
    if ( selected >= 0 )
        OpenLink( (UInt32) selected );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnLinksActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !IsAtTarget( eventData ) )
        return;
    m_linkClickPending = false;
    const CItemEventData* item = CORE::CICloneable::CloneableAs< CItemEventData >( eventData );
    const Int32 index = GUCEF_NULL != item && item->index >= 0 ? item->index : GetListSelection( "Links" );
    if ( index >= 0 )
        OpenLink( (UInt32) index );
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CNavigationEventData* navigation = CORE::CICloneable::CloneableAs< CNavigationEventData >( eventData );
    if ( GUCEF_NULL == navigation || EVENTPHASE_TUNNEL == navigation->phase || navigation->handled || !m_open )
        return;
    if ( NAVIGATION_CANCEL == navigation->direction )
    {
        navigation->handled = true;
        Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CHelpViewerForm::OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( eventData );
    if ( GUCEF_NULL == key || EVENTPHASE_TUNNEL == key->phase || key->handled )
        return;
    if ( 0 == ( key->modifiers & INPUT::KEYMOD_ALT ) )
        return;
    if ( INPUT::KEYCODE_LEFT == key->keyCode )
    {
        key->handled = true;
        GoBack();
    }
    else
    if ( INPUT::KEYCODE_RIGHT == key->keyCode )
    {
        key->handled = true;
        GoForward();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
