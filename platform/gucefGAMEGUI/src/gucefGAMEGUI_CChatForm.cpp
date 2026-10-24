/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CTEXTBOX_H
#include "gucefGUI_CTextbox.h"
#define GUCEF_GUI_CTEXTBOX_H
#endif /* GUCEF_GUI_CTEXTBOX_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CChatForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CChatForm::MessageSentEvent = "GUCEF::GAMEGUI::CChatForm::MessageSentEvent";

namespace {

const char* const DirectMessageItem = "Direct message";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CChatForm::RegisterEvents( void )
{GUCEF_TRACE;

    MessageSentEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CChatForm::CChatForm( void )
    : GUI::CFormEx()
    , m_placeholder( GUCEF_NULL )
    , m_service( GUCEF_NULL )
    , m_singleChannel()
    , m_tabs()
    , m_active( -1 )
    , m_memberMenu( GUCEF_NULL )
    , m_menuUser()
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CChatForm::~CChatForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_memberMenu && GUCEF_NULL != context )
        context->DestroyWidget( m_memberMenu );
    m_memberMenu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const bool single = !m_singleChannel.IsNULLOrEmpty();
    const Int32 tabsHeight = single ? 0 : 32;
    const Int32 topicHeight = 24;
    const Int32 top = tabsHeight + topicHeight + 4;
    const Int32 bottom = 40;

    CORE::CDataNode* panel = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "ChatPanel", "0,0 0,0", "1,0 1,0" );

    if ( !single )
    {
        CORE::CDataNode* tabs = GUI::CFormLayout::AddWidgetNode( *panel, "TabControl", "ChatTabs", "0,0 0,0", "1,0 0," + CORE::Int32ToString( tabsHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *tabs, "TabHeight", CORE::Int32ToString( tabsHeight ) );
    }

    CORE::CDataNode* topic = GUI::CFormLayout::AddWidgetNode( *panel, "Label", "Topic", "0,0 0," + CORE::Int32ToString( tabsHeight + 2 ), "1,0 0," + CORE::Int32ToString( topicHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *topic, "TextColour", "170,175,190,255" );

    GUI::CFormLayout::AddWidgetNode( *panel, "Textbox", "Messages", "0,0 0," + CORE::Int32ToString( top ),
                                     "1,-168 1," + CORE::Int32ToString( -( top + bottom ) ) );
    GUI::CFormLayout::AddWidgetNode( *panel, "Listbox", "Members", "1,-160 0," + CORE::Int32ToString( top ),
                                     "0,160 1," + CORE::Int32ToString( -( top + bottom ) ) );

    const Int32 buttonsWidth = single ? 88 : 176;
    GUI::CFormLayout::AddWidgetNode( *panel, "Editbox", "Input", "0,0 1,-32", "1," + CORE::Int32ToString( -( buttonsWidth + 8 ) ) + " 0,32" );
    CORE::CDataNode* send = GUI::CFormLayout::AddWidgetNode( *panel, "Button", "Send", "1," + CORE::Int32ToString( -buttonsWidth ) + " 1,-32", "0,80 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *send, "Text", "Send" );
    GUI::CFormLayout::SetWidgetNodeProperty( *send, "StyleClass", "Primary" );
    if ( !single )
    {
        CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *panel, "Button", "Close", "1,-88 1,-32", "0,80 0,32" );
        GUI::CFormLayout::SetWidgetNodeProperty( *close, "Text", "Close" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::Rebuild( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_placeholder )
        return true;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::Build( GUI::CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    m_placeholder = placeholder;
    return Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CChatForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* tabs = GetWidget( "ChatTabs" );
    if ( GUCEF_NULL != tabs )
        SubscribeTo( tabs, GUI::CTabControl::ActiveTabChangedEvent );
    GUI::CWidget* members = GetWidget( "Members" );
    if ( GUCEF_NULL != members )
    {
        SubscribeTo( members, GUI::CWidget::ItemActivatedEvent );
        SubscribeTo( members, GUI::CWidget::MouseClickEvent );
    }
    GUI::CWidget* input = GetWidget( "Input" );
    if ( GUCEF_NULL != input )
        SubscribeTo( input, GUI::CEditbox::TextEnteredEvent );
    GUI::CWidget* send = GetWidget( "Send" );
    if ( GUCEF_NULL != send )
        SubscribeTo( send, GUI::CWidget::ActivatedEvent );
    GUI::CWidget* close = GetWidget( "Close" );
    if ( GUCEF_NULL != close )
        SubscribeTo( close, GUI::CWidget::ActivatedEvent );

    ShowTabs();
    ShowActiveTab();
}

/*-------------------------------------------------------------------------*/

void
CChatForm::SetService( MULTIPLAY::CIChatService* service )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service )
        UnsubscribeFrom( m_service );
    m_service = service;
    if ( GUCEF_NULL == m_service )
        return;

    SubscribeTo( m_service );

    CORE::CString::StringVector channels;
    m_service->GetChannels( channels );
    for ( UInt32 i=0; i<(UInt32)channels.size(); ++i )
        if ( m_singleChannel.IsNULLOrEmpty() || channels[ i ] == m_singleChannel )
            AddTab( channels[ i ], false );
    if ( !m_singleChannel.IsNULLOrEmpty() && m_service->IsConnected() && GUCEF_NULL == m_service->GetChannel( m_singleChannel ) )
        m_service->JoinChannel( m_singleChannel );

    ShowTabs();
    ShowActiveTab();
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CIChatService*
CChatForm::GetService( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::SetSingleChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( channel == m_singleChannel )
        return;
    m_singleChannel = channel;

    m_tabs.clear();
    m_active = -1;
    if ( !m_singleChannel.IsNULLOrEmpty() )
    {
        AddTab( m_singleChannel, false );
        if ( GUCEF_NULL != m_service && m_service->IsConnected() && GUCEF_NULL == m_service->GetChannel( m_singleChannel ) )
            m_service->JoinChannel( m_singleChannel );
    }
    else
    if ( GUCEF_NULL != m_service )
    {
        CORE::CString::StringVector channels;
        m_service->GetChannels( channels );
        for ( UInt32 i=0; i<(UInt32)channels.size(); ++i )
            AddTab( channels[ i ], false );
    }

    if ( GUCEF_NULL != m_placeholder )
        Rebuild();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChatForm::GetSingleChannel( void ) const
{GUCEF_TRACE;

    return m_singleChannel;
}

/*-------------------------------------------------------------------------*/

Int32
CChatForm::FindTab( const CORE::CString& name, bool direct ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_tabs.size(); ++i )
        if ( m_tabs[ i ].direct == direct && m_tabs[ i ].name == name )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CChatForm::AddTab( const CORE::CString& name, bool direct )
{GUCEF_TRACE;

    Int32 existing = FindTab( name, direct );
    if ( existing >= 0 )
        return existing;

    TTab tab;
    tab.name = name;
    tab.direct = direct;
    tab.unread = 0;
    m_tabs.push_back( tab );
    if ( m_active < 0 )
        m_active = 0;
    ShowTabs();
    if ( 0 == m_active && 1 == m_tabs.size() )
        ShowActiveTab();
    return (Int32) m_tabs.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::RemoveTab( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_tabs.size() )
        return;
    m_tabs.erase( m_tabs.begin() + index );
    if ( m_tabs.empty() )
        m_active = -1;
    else
    if ( m_active > (Int32) index )
        --m_active;
    else
    if ( m_active >= (Int32) m_tabs.size() )
        m_active = (Int32) m_tabs.size() - 1;
    ShowTabs();
    ShowActiveTab();
}

/*-------------------------------------------------------------------------*/

UInt32
CChatForm::GetTabCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_tabs.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChatForm::GetTabName( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_tabs.size() ? m_tabs[ index ].name : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::IsTabDirect( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_tabs.size() && m_tabs[ index ].direct;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChatForm::GetTabTitle( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= (UInt32) m_tabs.size() )
        return CORE::CString::Empty;
    const TTab& tab = m_tabs[ index ];
    CORE::CString title = tab.direct ? "@" + tab.name : tab.name;
    if ( tab.unread > 0 )
        title += " (" + CORE::UInt32ToString( tab.unread ) + ")";
    return title;
}

/*-------------------------------------------------------------------------*/

Int32
CChatForm::GetActiveTabIndex( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::SetActiveTab( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_tabs.size() )
        return false;
    m_active = (Int32) index;
    ShowTabs();
    ShowActiveTab();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CChatForm::GetUnreadCount( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_tabs.size() ? m_tabs[ index ].unread : 0;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CChatForm::GetTabLines( UInt32 index ) const
{GUCEF_TRACE;

    static const CORE::CString::StringVector noLines;
    return index < (UInt32) m_tabs.size() ? m_tabs[ index ].lines : noLines;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CChatForm::GetShownLines( void )
{GUCEF_TRACE;

    GUI::CTextbox* messages = GetWidgetAs< GUI::CTextbox >( "Messages" );
    if ( GUCEF_NULL == messages )
        return CORE::CString::StringVector();
    return messages->GetText();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CChatForm::GetShownMembers( void )
{GUCEF_TRACE;

    GUI::CListbox* members = GetWidgetAs< GUI::CListbox >( "Members" );
    if ( GUCEF_NULL == members )
        return CORE::CString::StringVector();
    return members->GetItems();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChatForm::GetShownTopic( void )
{GUCEF_TRACE;

    GUI::CWidget* topic = GetWidget( "Topic" );
    return GUCEF_NULL != topic ? topic->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChatForm::FormatMessage( const MULTIPLAY::CChatMessage& message )
{GUCEF_TRACE;

    CORE::CString text = message.from + ": " + message.text;
    if ( message.time.GetYear() >= 1971 )
        text = "[" + FormatClock( message.time ) + "] " + text;
    return text;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::ShowTabs( void )
{GUCEF_TRACE;

    GUI::CWidget* tabs = GetWidget( "ChatTabs" );
    if ( GUCEF_NULL == tabs )
        return;

    CORE::CString::StringVector titles;
    for ( UInt32 i=0; i<(UInt32)m_tabs.size(); ++i )
        titles.push_back( GetTabTitle( i ) );

    bool wasUpdating = m_updating;
    m_updating = true;
    tabs->SetStringListProperty( "Tabs", titles );
    tabs->SetIntProperty( "ActiveTab", m_active < 0 ? 0 : m_active );
    m_updating = wasUpdating;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::ScrollMessagesToEnd( void )
{GUCEF_TRACE;

    GUI::CTextbox* messages = GetWidgetAs< GUI::CTextbox >( "Messages" );
    if ( GUCEF_NULL == messages )
        return;
    Int32 hidden = (Int32) messages->GetDisplayLines().size() - messages->GetVisibleLineCount();
    messages->SetIntProperty( "ScrollPosition", hidden > 0 ? hidden : 0 );
}

/*-------------------------------------------------------------------------*/

void
CChatForm::ShowMembers( void )
{GUCEF_TRACE;

    GUI::CListbox* members = GetWidgetAs< GUI::CListbox >( "Members" );
    if ( GUCEF_NULL == members )
        return;

    GUI::CListbox::TStringVector names;
    if ( m_active >= 0 && m_active < (Int32) m_tabs.size() )
    {
        const TTab& tab = m_tabs[ m_active ];
        if ( tab.direct )
        {
            if ( GUCEF_NULL != m_service )
                names.push_back( m_service->GetLocalUserName() );
            names.push_back( tab.name );
        }
        else
        if ( GUCEF_NULL != m_service )
        {
            const MULTIPLAY::CIChatChannel* channel = m_service->GetChannel( tab.name );
            if ( GUCEF_NULL != channel )
            {
                const CORE::CString::StringSet& set = channel->GetMembers();
                for ( CORE::CString::StringSet::const_iterator i = set.begin(); i != set.end(); ++i )
                    names.push_back( (*i) );
            }
        }
    }
    members->SetItems( names );
}

/*-------------------------------------------------------------------------*/

void
CChatForm::ShowActiveTab( void )
{GUCEF_TRACE;

    GUI::CWidget* messages = GetWidget( "Messages" );
    GUI::CWidget* topic = GetWidget( "Topic" );

    if ( m_active < 0 || m_active >= (Int32) m_tabs.size() )
    {
        if ( GUCEF_NULL != messages )
            messages->SetProperty( "Text", CORE::CString::Empty );
        if ( GUCEF_NULL != topic )
            topic->SetProperty( "Text", CORE::CString::Empty );
        ShowMembers();
        return;
    }

    TTab& tab = m_tabs[ m_active ];
    bool hadUnread = tab.unread > 0;
    tab.unread = 0;
    if ( GUCEF_NULL != messages )
        messages->SetProperty( "Text", JoinLines( tab.lines ) );
    if ( GUCEF_NULL != topic )
    {
        CORE::CString text;
        if ( !tab.direct && GUCEF_NULL != m_service )
        {
            const MULTIPLAY::CIChatChannel* channel = m_service->GetChannel( tab.name );
            if ( GUCEF_NULL != channel )
                text = channel->GetTopic();
        }
        topic->SetProperty( "Text", text );
    }
    ScrollMessagesToEnd();
    ShowMembers();
    if ( hadUnread )
        ShowTabs();
}

/*-------------------------------------------------------------------------*/

void
CChatForm::AppendLine( UInt32 tabIndex, const CORE::CString& line )
{GUCEF_TRACE;

    if ( tabIndex >= (UInt32) m_tabs.size() )
        return;
    TTab& tab = m_tabs[ tabIndex ];
    tab.lines.push_back( line );

    bool trimmed = false;
    if ( tab.lines.size() > MaxLinesPerTab )
    {
        tab.lines.erase( tab.lines.begin(), tab.lines.begin() + ( tab.lines.size() - MaxLinesPerTab ) );
        trimmed = true;
    }

    if ( (Int32) tabIndex == m_active )
    {
        GUI::CTextbox* messages = GetWidgetAs< GUI::CTextbox >( "Messages" );
        if ( GUCEF_NULL != messages )
        {
            if ( trimmed )
                messages->SetProperty( "Text", JoinLines( tab.lines ) );
            else
                messages->AppendLine( line );
            ScrollMessagesToEnd();
        }
    }
    else
    {
        ++tab.unread;
        ShowTabs();
    }
}

/*-------------------------------------------------------------------------*/

void
CChatForm::OnChatMessage( const MULTIPLAY::CChatMessage& message )
{GUCEF_TRACE;

    Int32 index = -1;
    if ( message.isDirect )
    {
        if ( !m_singleChannel.IsNULLOrEmpty() )
            return;
        const CORE::CString& local = m_service->GetLocalUserName();
        const CORE::CString& other = ( message.from == local && !message.to.IsNULLOrEmpty() ) ? message.to : message.from;
        index = AddTab( other, true );
    }
    else
    {
        if ( !m_singleChannel.IsNULLOrEmpty() && message.channel != m_singleChannel )
            return;
        index = AddTab( message.channel, false );
    }
    AppendLine( (UInt32) index, FormatMessage( message ) );
}

/*-------------------------------------------------------------------------*/

void
CChatForm::OnChatPresence( const MULTIPLAY::CChatPresenceEventData& presence )
{GUCEF_TRACE;

    const CORE::CString& local = m_service->GetLocalUserName();
    bool isLocal = presence.user == local;

    if ( presence.channel.IsNULLOrEmpty() )
    {
        // a user left all channels
        ShowMembers();
        return;
    }
    if ( !m_singleChannel.IsNULLOrEmpty() && presence.channel != m_singleChannel )
        return;

    Int32 index = FindTab( presence.channel, false );
    if ( MULTIPLAY::CChatPresenceEventData::PRESENCE_JOINED == presence.presence )
    {
        if ( isLocal && index < 0 )
            index = AddTab( presence.channel, false );
        if ( index >= 0 )
        {
            if ( !isLocal )
                AppendLine( (UInt32) index, "* " + presence.user + " joined" );
            if ( index == m_active )
                ShowMembers();
        }
    }
    else
    if ( MULTIPLAY::CChatPresenceEventData::PRESENCE_LEFT == presence.presence )
    {
        if ( isLocal )
        {
            if ( index >= 0 && m_singleChannel.IsNULLOrEmpty() )
                RemoveTab( (UInt32) index );
            return;
        }
        if ( index >= 0 )
        {
            AppendLine( (UInt32) index, "* " + presence.user + " left" + ( presence.reason.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + presence.reason + ")" ) );
            if ( index == m_active )
                ShowMembers();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::JoinChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( channel.IsNULLOrEmpty() || !m_singleChannel.IsNULLOrEmpty() || GUCEF_NULL == m_service )
        return false;
    if ( !m_service->JoinChannel( channel ) )
        return false;
    Int32 index = AddTab( channel, false );
    return SetActiveTab( (UInt32) index );
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::OpenDirectConversation( const CORE::CString& user )
{GUCEF_TRACE;

    if ( user.IsNULLOrEmpty() || !m_singleChannel.IsNULLOrEmpty() || GUCEF_NULL == m_service )
        return false;
    if ( user == m_service->GetLocalUserName() )
        return false;
    Int32 index = AddTab( user, true );
    return SetActiveTab( (UInt32) index );
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::CloseTab( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_tabs.size() || !m_singleChannel.IsNULLOrEmpty() )
        return false;
    // copies: the tab can go while the service leaves the channel
    const CORE::CString name = m_tabs[ index ].name;
    const bool direct = m_tabs[ index ].direct;
    if ( !direct && GUCEF_NULL != m_service )
        m_service->LeaveChannel( name );

    // the service may have told us already, with the presence of the local user
    Int32 still = FindTab( name, direct );
    if ( still >= 0 )
        RemoveTab( (UInt32) still );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChatForm::SendText( const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service || m_active < 0 || m_active >= (Int32) m_tabs.size() || TrimText( text ).IsNULLOrEmpty() )
        return false;

    const TTab& tab = m_tabs[ m_active ];
    MULTIPLAY::CChatMessage message;
    message.isDirect = tab.direct;
    message.from = m_service->GetLocalUserName();
    message.text = text;
    if ( tab.direct )
    {
        message.to = tab.name;
        if ( !m_service->SendDirect( tab.name, text ) )
            return false;
    }
    else
    {
        message.channel = tab.name;
        if ( !m_service->SendToChannel( tab.name, text ) )
            return false;
    }
    NotifyObservers( MessageSentEvent, &message );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChatForm::SendInput( void )
{GUCEF_TRACE;

    GUI::CWidget* input = GetWidget( "Input" );
    if ( GUCEF_NULL == input )
        return;
    CORE::CString text = input->GetProperty( "Text" );
    if ( SendText( text ) )
        input->SetProperty( "Text", CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CChatForm::OpenMemberMenu( Int32 x, Int32 y, Int32 row )
{GUCEF_TRACE;

    GUI::CListbox* members = GetWidgetAs< GUI::CListbox >( "Members" );
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == members || GUCEF_NULL == context || !m_singleChannel.IsNULLOrEmpty() )
        return;

    CORE::CString user;
    if ( row < 0 || !members->GetLineAtIndex( (UInt32) row, user ) )
        return;
    if ( GUCEF_NULL != m_service && user == m_service->GetLocalUserName() )
        return;

    if ( GUCEF_NULL == m_memberMenu )
    {
        m_memberMenu = static_cast< GUI::CPopupMenu* >( context->CreateWidget( "PopupMenu", "ChatForm#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_memberMenu )
            return;
        SubscribeTo( m_memberMenu, GUI::CWidget::ItemActivatedEvent );
    }
    m_menuUser = user;
    GUI::CPopupMenu::TStringVector items;
    items.push_back( DirectMessageItem );
    m_memberMenu->SetItems( items );
    m_memberMenu->ShowAt( x, y );
}

/*-------------------------------------------------------------------------*/

void
CChatForm::OnNotify( CORE::CNotifier* notifier    ,
                     const CORE::CEvent& eventId  ,
                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service && notifier == m_service )
    {
        if ( CORE::CINotifier::DestructionEvent == eventId )
        {
            m_service = GUCEF_NULL;
            return;
        }
        if ( MULTIPLAY::CIChatService::ChatMessageEvent == eventId && GUCEF_NULL != eventData )
        {
            OnChatMessage( *static_cast< const MULTIPLAY::CChatMessage* >( eventData ) );
            return;
        }
        if ( MULTIPLAY::CIChatService::ChatPresenceEvent == eventId && GUCEF_NULL != eventData )
        {
            OnChatPresence( *static_cast< const MULTIPLAY::CChatPresenceEventData* >( eventData ) );
            return;
        }
        if ( MULTIPLAY::CIChatService::ChatTopicEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CChatTopicEventData* topic = static_cast< const MULTIPLAY::CChatTopicEventData* >( eventData );
            if ( m_active >= 0 && m_active < (Int32) m_tabs.size() && !m_tabs[ m_active ].direct && m_tabs[ m_active ].name == topic->channel )
            {
                GUI::CWidget* label = GetWidget( "Topic" );
                if ( GUCEF_NULL != label )
                    label->SetProperty( "Text", topic->topic );
            }
            return;
        }
        if ( MULTIPLAY::CIChatService::ChatErrorEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CIChatService::TChatErrorEventData* error = static_cast< const MULTIPLAY::CIChatService::TChatErrorEventData* >( eventData );
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ChatForm: The chat service reports: " << error->GetData() );
            if ( m_active >= 0 )
                AppendLine( (UInt32) m_active, "* " + error->GetData() );
            return;
        }
        return;
    }

    if ( GUI::CTabControl::ActiveTabChangedEvent == eventId )
    {
        if ( m_updating || GUCEF_NULL == eventData )
            return;
        const GUI::CTabControl::TActiveTabChangedEventData* tab = static_cast< const GUI::CTabControl::TActiveTabChangedEventData* >( eventData );
        if ( tab->GetData() < (UInt32) m_tabs.size() && (Int32) tab->GetData() != m_active )
        {
            m_active = (Int32) tab->GetData();
            ShowActiveTab();
        }
        return;
    }
    if ( GUI::CWidget::ItemActivatedEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
        if ( notifier == m_memberMenu && GUCEF_NULL != m_memberMenu )
        {
            GUI::CGUIContext* context = GetGUIContext();
            if ( GUCEF_NULL != context && context->GetPopup() == m_memberMenu )
                context->ClosePopup();
            if ( DirectMessageItem == item->item )
                OpenDirectConversation( m_menuUser );
            return;
        }
        if ( notifier == GetWidget( "Members" ) && GUI::EVENTPHASE_TUNNEL != item->phase )
            OpenDirectConversation( item->item );
        return;
    }
    if ( GUI::CWidget::MouseClickEvent == eventId && GUCEF_NULL != eventData && notifier == GetWidget( "Members" ) )
    {
        GUI::CPointerEventData* pointer = static_cast< GUI::CPointerEventData* >( eventData );
        if ( GUI::MOUSEBUTTON_RIGHT == pointer->button && GUI::EVENTPHASE_TUNNEL != pointer->phase )
        {
            GUI::CListbox* members = GetWidgetAs< GUI::CListbox >( "Members" );
            OpenMemberMenu( pointer->x, pointer->y, GUCEF_NULL != members ? members->GetRowAt( pointer->localY ) : -1 );
        }
        return;
    }
    if ( GUI::CEditbox::TextEnteredEvent == eventId )
    {
        SendInput();
        return;
    }
    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Send" ) )
            SendInput();
        else
        if ( notifier == GetWidget( "Close" ) && m_active >= 0 )
            CloseTab( (UInt32) m_active );
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChatForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CChatForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
