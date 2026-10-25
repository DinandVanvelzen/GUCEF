/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGAMEGUI_TestCommon.h"
#include "TestChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the lines the player sends with the chat form */
class CSentRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CSentRecorder > TEventCallback;

    UInt32 count;
    MULTIPLAY::CChatMessage last;

    CSentRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), count( 0 ), last()
    {
        TEventCallback callback( this, &CSentRecorder::OnMessageSent );
        SubscribeTo( form, GAMEGUI::CChatForm::MessageSentEvent, callback );
    }

    virtual ~CSentRecorder() { UnsubscribeAllFromObserver(); }

    void OnMessageSent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++count;
            last = *static_cast< MULTIPLAY::CChatMessage* >( eventData );
        }
    }
};

/** Records the messages a chat service of the test hears */
class CHeardRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CHeardRecorder > TEventCallback;

    UInt32 count;
    MULTIPLAY::CChatMessage last;

    CHeardRecorder( CORE::CNotifier* service )
        : CORE::CObserver(), count( 0 ), last()
    {
        TEventCallback callback( this, &CHeardRecorder::OnMessageHeard );
        SubscribeTo( service, MULTIPLAY::CIChatService::ChatMessageEvent, callback );
    }

    virtual ~CHeardRecorder() { UnsubscribeAllFromObserver(); }

    void OnMessageHeard( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++count;
            last = *static_cast< MULTIPLAY::CChatMessage* >( eventData );
        }
    }
};

/*-------------------------------------------------------------------------*/

static bool
ConnectUser( CTestChatService& service, const char* name )
{
    MULTIPLAY::CPlayerIdentity identity;
    identity.userId = CORE::CString( name ) + "-id";
    identity.displayName = name;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    return service.Connect( identity, future ) && service.IsConnected();
}

/*-------------------------------------------------------------------------*/

static GUI::CWidget*
MakeChatHolder( GUI::CGUIContext* context )
{
    GUI::CWidget* holder = context->CreateWidget( "Panel", "ChatHolder", GUCEF_NULL );
    holder->SetProperty( "Size", "0,800 0,500" );
    context->ShowRoot( holder, false );
    context->FlushChanges();
    return holder;
}

/*-------------------------------------------------------------------------*/

/** A click on the header of a tab */
static void
ClickTab( GUI::CGUIContext* context, GUI::CWidget* tabs, UInt32 index )
{
    context->FlushChanges();
    GUI::CTabControl* control = GAMEGUI::WidgetAs< GUI::CTabControl >( tabs, "TabControl" );
    GUI::CRect tab = control->GetTabRect( index );
    ClickAt( context, tabs->GetRect().left + tab.left + tab.width / 2, tabs->GetRect().top + tab.top + tab.height / 2 );
    context->RenderFrame( 500 );
}

/*-------------------------------------------------------------------------*/

static void
ClickMember( GUI::CGUIContext* context, GUI::CListbox* list, Int32 row, GUI::TMouseButton button, UInt32 times )
{
    context->FlushChanges();
    const GUI::CRect& rect = list->GetRect();
    Int32 rowHeight = list->GetRowHeight();
    for ( UInt32 i=0; i<times; ++i )
        ClickAt( context, rect.left + 10, rect.top + row * rowHeight + rowHeight / 2, button );
    context->RenderFrame( 300 );
}

/*-------------------------------------------------------------------------*/

void
PerformChatFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.Chat" );

    GUCEF_TESTFW_TESTCASE( "Chat: A tab per channel, the messages show in the tab of their channel and the other tabs count what is new" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService alice;
        CTestChatService bob;
        ASSERT_TRUE( ConnectUser( alice, "Alice" ) && ConnectUser( bob, "Bob" ) );
        ASSERT_TRUE( alice.JoinChannel( "general" ) && alice.JoinChannel( "lobby" ) );

        GAMEGUI::CChatForm chat;
        CSentRecorder sent( &chat );
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );
        chat.SetService( &alice );
        ASSERT_TRUE( &alice == chat.GetService() );

        // the channels of the service have a tab each
        ASSERT_TRUE( 2 == chat.GetTabCount() && "general" == chat.GetTabName( 0 ) && "lobby" == chat.GetTabName( 1 ) && 0 == chat.GetActiveTabIndex() );
        ASSERT_TRUE( !chat.IsTabDirect( 0 ) && "general" == chat.GetTabTitle( 0 ) );
        GUI::CTabControl* tabs = chat.GetWidgetAs< GUI::CTabControl >( "ChatTabs" );
        ASSERT_TRUE( GUCEF_NULL != tabs && 2 == tabs->GetTabCount() && 0 == tabs->GetActiveTab() );
        ASSERT_TRUE( "general" == tabs->GetStringListProperty( "Tabs" )[ 0 ] );

        // the other user joins and talks
        ASSERT_TRUE( bob.JoinChannel( "general" ) && bob.JoinChannel( "lobby" ) );
        ASSERT_TRUE( bob.SendToChannel( "general", "Hello general" ) );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Bob: Hello general" ) && AnyLineHas( chat.GetShownLines(), "* Bob joined" ) );

        // a message of the tab that is not shown is counted, not shown
        ASSERT_TRUE( bob.SendToChannel( "lobby", "Hello lobby" ) && bob.SendToChannel( "lobby", "Anybody?" ) );
        ASSERT_FALSE( AnyLineHas( chat.GetShownLines(), "Hello lobby" ) );
        ASSERT_TRUE( 3 == chat.GetUnreadCount( 1 ) && "lobby (3)" == chat.GetTabTitle( 1 ) && "lobby (3)" == tabs->GetStringListProperty( "Tabs" )[ 1 ] );
        ASSERT_TRUE( 0 == chat.GetUnreadCount( 0 ) );

        // the tab is shown with a click on its title: its lines, no unread
        ClickTab( context, tabs, 1 );
        ASSERT_TRUE( 1 == chat.GetActiveTabIndex() && 0 == chat.GetUnreadCount( 1 ) && "lobby" == chat.GetTabTitle( 1 ) );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Bob: Hello lobby" ) && AnyLineHas( chat.GetShownLines(), "Bob: Anybody?" ) && !AnyLineHas( chat.GetShownLines(), "Hello general" ) );
        ASSERT_TRUE( 3 == chat.GetTabLines( 1 ).size() );
        ASSERT_TRUE( 1 == tabs->GetActiveTab() );

        // typing a line sends it to the channel of the shown tab
        TypeInto( context, chat.GetWidget( "Input" ), "Hi Bob", true );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Alice: Hi Bob" ) && chat.GetWidget( "Input" )->GetProperty( "Text" ).IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == sent.count && "lobby" == sent.last.channel && !sent.last.isDirect && "Hi Bob" == sent.last.text && "Alice" == sent.last.from );
        ClickTab( context, tabs, 0 );
        chat.GetWidget( "Input" )->SetProperty( "Text", "Back in general" );
        ClickWidget( context, chat.GetWidget( "Send" ) );
        ASSERT_TRUE( 2 == sent.count && "general" == sent.last.channel && AnyLineHas( chat.GetShownLines(), "Alice: Back in general" ) );
        ASSERT_FALSE( AnyLineHas( chat.GetShownLines(), "Hi Bob" ) );

        // empty lines are not sent
        ASSERT_TRUE( !chat.SendText( "   " ) && !chat.SendText( "" ) && 2 == sent.count );

        // the form joins a channel for the player and shows it
        ASSERT_TRUE( chat.JoinChannel( "trade" ) && 3 == chat.GetTabCount() && 2 == chat.GetActiveTabIndex() && GUCEF_NULL != alice.GetChannel( "trade" ) );
        ASSERT_FALSE( chat.JoinChannel( "" ) );

        // a message with the time in it
        CORE::CDateTime time = CORE::CDateTime::NowUTCDateTime();
        MULTIPLAY::CChatMessage message;
        message.from = "Bob";
        message.text = "timed";
        message.time = time;
        ASSERT_TRUE( GAMEGUI::CChatForm::FormatMessage( message ) == "[" + GAMEGUI::FormatClock( time ) + "] Bob: timed" );
        message.time = CORE::CDateTime();
        ASSERT_TRUE( "Bob: timed" == GAMEGUI::CChatForm::FormatMessage( message ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chat: The handlers are subscribed again when the layout is rebuilt, and the form lets go of a service that goes away" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService* service = GUCEF_NEW CTestChatService();
        ASSERT_TRUE( ConnectUser( *service, "Alice" ) && service->JoinChannel( "general" ) );

        GAMEGUI::CChatForm chat;
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );

        // the tab control, the members (two events), the input, Send and Close
        const UInt32 withTabs = GetEventSubscriptionCount( chat );
        ASSERT_TRUE( 6 == withTabs );
        chat.SetService( service );
        ASSERT_TRUE( withTabs + 4 == GetEventSubscriptionCount( chat ) );

        // the layout of a single channel has no tabs and no Close button, and the service stays subscribed
        chat.SetSingleChannel( "general" );
        ASSERT_TRUE( 4 + 4 == GetEventSubscriptionCount( chat ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() && chat.SendText( "still heard" ) );

        // the service goes away under the form
        GUCEF_DELETE service;
        service = GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL == chat.GetService() && 4 == GetEventSubscriptionCount( chat ) );
        ASSERT_FALSE( chat.SendText( "nobody to hear it" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chat: The members list and the topic follow the channel shown" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService alice;
        CTestChatService bob;
        CTestChatService carol;
        ASSERT_TRUE( ConnectUser( alice, "Alice" ) && ConnectUser( bob, "Bob" ) && ConnectUser( carol, "Carol" ) );

        GAMEGUI::CChatForm chat;
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );
        chat.SetService( &alice );
        ASSERT_TRUE( 0 == chat.GetTabCount() && chat.GetShownMembers().empty() );

        // joining makes the tab, with the members
        ASSERT_TRUE( alice.JoinChannel( "general" ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() && 1 == chat.GetShownMembers().size() && "Alice" == chat.GetShownMembers()[ 0 ] );
        ASSERT_TRUE( bob.JoinChannel( "general" ) && carol.JoinChannel( "general" ) && carol.JoinChannel( "other" ) );
        CORE::CString::StringVector members = chat.GetShownMembers();
        ASSERT_TRUE( 3 == members.size() && "Alice" == members[ 0 ] && "Bob" == members[ 1 ] && "Carol" == members[ 2 ] );

        // someone leaves
        ASSERT_TRUE( bob.LeaveChannel( "general" ) );
        ASSERT_TRUE( 2 == chat.GetShownMembers().size() && AnyLineHas( chat.GetTabLines( 0 ), "* Bob left" ) );

        // the topic of the channel
        ASSERT_TRUE( chat.GetShownTopic().IsNULLOrEmpty() );
        ASSERT_TRUE( carol.SetTopic( "general", "Be nice" ) );
        ASSERT_TRUE( "Be nice" == chat.GetShownTopic() );

        // a topic of a channel that is not shown does not show
        ASSERT_TRUE( alice.JoinChannel( "other" ) && 2 == chat.GetTabCount() );
        ASSERT_TRUE( carol.SetTopic( "other", "Elsewhere" ) );
        ASSERT_TRUE( "Be nice" == chat.GetShownTopic() );
        ASSERT_TRUE( chat.SetActiveTab( 1 ) && "Elsewhere" == chat.GetShownTopic() && 2 == chat.GetShownMembers().size() );
        ASSERT_FALSE( chat.SetActiveTab( 5 ) );
        ASSERT_TRUE( chat.SetActiveTab( 0 ) && "Be nice" == chat.GetShownTopic() );

        // an error of the service shows in the chat
        alice.ReportError( "The network went away" );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "* The network went away" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chat: Direct messages have a tab of their own, opened by a message, a double click on a member or the menu" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService alice;
        CTestChatService bob;
        CTestChatService carol;
        ASSERT_TRUE( ConnectUser( alice, "Alice" ) && ConnectUser( bob, "Bob" ) && ConnectUser( carol, "Carol" ) );
        CHeardRecorder bobHears( &bob );
        CHeardRecorder carolHears( &carol );
        ASSERT_TRUE( alice.JoinChannel( "general" ) && bob.JoinChannel( "general" ) && carol.JoinChannel( "general" ) );

        GAMEGUI::CChatForm chat;
        CSentRecorder sent( &chat );
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );
        chat.SetService( &alice );
        ASSERT_TRUE( 1 == chat.GetTabCount() );

        // a direct message makes a tab, which is not shown yet
        ASSERT_TRUE( bob.SendDirect( "Alice", "Psst" ) );
        ASSERT_TRUE( 2 == chat.GetTabCount() && chat.IsTabDirect( 1 ) && "Bob" == chat.GetTabName( 1 ) && "@Bob (1)" == chat.GetTabTitle( 1 ) );
        ASSERT_TRUE( 0 == chat.GetActiveTabIndex() && !AnyLineHas( chat.GetShownLines(), "Psst" ) );
        ASSERT_TRUE( -1 == chat.FindTab( "Bob", false ) && 1 == chat.FindTab( "Bob", true ) );

        // shown, and answered
        ASSERT_TRUE( chat.SetActiveTab( 1 ) );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Bob: Psst" ) && "@Bob" == chat.GetTabTitle( 1 ) );
        ASSERT_TRUE( 2 == chat.GetShownMembers().size() );
        TypeInto( context, chat.GetWidget( "Input" ), "Yes?", true );
        ASSERT_TRUE( 2 == bobHears.count && bobHears.last.isDirect && "Alice" == bobHears.last.from && "Yes?" == bobHears.last.text );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Alice: Yes?" ) && 2 == chat.GetTabCount() );
        ASSERT_TRUE( 1 == sent.count && sent.last.isDirect && "Bob" == sent.last.to && sent.last.channel.IsNULLOrEmpty() );

        // a double click on a member of a channel
        ASSERT_TRUE( chat.SetActiveTab( 0 ) );
        GUI::CListbox* members = chat.GetWidgetAs< GUI::CListbox >( "Members" );
        ASSERT_TRUE( GUCEF_NULL != members && 3 == members->GetItems().size() && "Carol" == members->GetItems()[ 2 ] );
        ClickMember( context, members, 2, GUI::MOUSEBUTTON_LEFT, 2 );
        ASSERT_TRUE( 3 == chat.GetTabCount() && 2 == chat.GetActiveTabIndex() && "Carol" == chat.GetTabName( 2 ) && chat.IsTabDirect( 2 ) );
        ASSERT_TRUE( chat.SendText( "Hello Carol" ) && 1 == carolHears.count && "Hello Carol" == carolHears.last.text );

        // you do not talk to yourself
        ASSERT_TRUE( chat.SetActiveTab( 0 ) );
        ClickMember( context, members, 0, GUI::MOUSEBUTTON_LEFT, 2 );
        ASSERT_TRUE( 3 == chat.GetTabCount() && 0 == chat.GetActiveTabIndex() );
        ASSERT_FALSE( chat.OpenDirectConversation( "Alice" ) || chat.OpenDirectConversation( "" ) );

        // the menu of the right mouse button
        ASSERT_TRUE( chat.CloseTab( 2 ) && 2 == chat.GetTabCount() );
        ClickMember( context, members, 2, GUI::MOUSEBUTTON_RIGHT, 1 );
        GUI::CPopupMenu* menu = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
        ASSERT_TRUE( GUCEF_NULL != menu && 1 == menu->GetItems().size() && "Direct message" == menu->GetItems()[ 0 ] );
        if ( GUCEF_NULL != menu )
        {
            context->FlushChanges();
            const GUI::CRect& rect = menu->GetRect();
            ClickAt( context, rect.left + 10, rect.top + menu->GetRowHeight() / 2 );
            ASSERT_TRUE( 3 == chat.GetTabCount() && 2 == chat.GetActiveTabIndex() && "Carol" == chat.GetTabName( 2 ) && GUCEF_NULL == context->GetPopup() );
        }

        // no menu for yourself
        ASSERT_TRUE( chat.SetActiveTab( 0 ) );
        ClickMember( context, members, 0, GUI::MOUSEBUTTON_RIGHT, 1 );
        ASSERT_TRUE( GUCEF_NULL == context->GetPopup() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chat: Close leaves a channel or drops a conversation, and a channel the user left loses its tab" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService alice;
        CTestChatService bob;
        ASSERT_TRUE( ConnectUser( alice, "Alice" ) && ConnectUser( bob, "Bob" ) );

        GAMEGUI::CChatForm chat;
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );
        chat.SetService( &alice );
        ASSERT_TRUE( chat.JoinChannel( "one" ) && chat.JoinChannel( "two" ) && chat.JoinChannel( "three" ) && chat.OpenDirectConversation( "Bob" ) );
        ASSERT_TRUE( 4 == chat.GetTabCount() && 3 == chat.GetActiveTabIndex() );

        // the conversation goes
        ClickWidget( context, chat.GetWidget( "Close" ) );
        ASSERT_TRUE( 3 == chat.GetTabCount() && 2 == chat.GetActiveTabIndex() && "three" == chat.GetTabName( 2 ) );

        // the channel is left, with the service
        ClickWidget( context, chat.GetWidget( "Close" ) );
        ASSERT_TRUE( 2 == chat.GetTabCount() && GUCEF_NULL == alice.GetChannel( "three" ) && 1 == chat.GetActiveTabIndex() );

        // a tab in the middle goes: the shown one stays
        ASSERT_TRUE( chat.SetActiveTab( 1 ) && chat.CloseTab( 0 ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() && 0 == chat.GetActiveTabIndex() && "two" == chat.GetTabName( 0 ) );

        // leaving with the service takes the tab too
        ASSERT_TRUE( alice.LeaveChannel( "two" ) );
        ASSERT_TRUE( 0 == chat.GetTabCount() && -1 == chat.GetActiveTabIndex() && chat.GetShownLines().empty() );
        ASSERT_FALSE( chat.SendText( "to nobody" ) );
        ASSERT_FALSE( chat.CloseTab( 0 ) );

        // the history of a tab is kept to a limit
        ASSERT_TRUE( alice.JoinChannel( "busy" ) );
        for ( UInt32 i=0; i<GAMEGUI::CChatForm::MaxLinesPerTab + 20; ++i )
            alice.SendToChannel( "busy", "line " + CORE::UInt32ToString( i ) );
        ASSERT_TRUE( GAMEGUI::CChatForm::MaxLinesPerTab == chat.GetTabLines( 0 ).size() );
        ASSERT_TRUE( AnyLineHas( chat.GetTabLines( 0 ), "line 519" ) && !AnyLineHas( chat.GetTabLines( 0 ), "line 5:" ) && !Contains( chat.GetTabLines( 0 ), "Alice: line 0" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chat: Bound to a single channel there is that channel only" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestChatService alice;
        CTestChatService bob;
        ASSERT_TRUE( ConnectUser( alice, "Alice" ) && ConnectUser( bob, "Bob" ) );
        ASSERT_TRUE( alice.JoinChannel( "general" ) && bob.JoinChannel( "lobby-7" ) );

        GAMEGUI::CChatForm chat;
        chat.SetSingleChannel( "lobby-7" );
        ASSERT_TRUE( chat.Build( MakeChatHolder( context ) ) );
        chat.SetService( &alice );

        // the channel is joined for the player; the others are not there
        ASSERT_TRUE( "lobby-7" == chat.GetSingleChannel() && 1 == chat.GetTabCount() && "lobby-7" == chat.GetTabName( 0 ) && GUCEF_NULL != alice.GetChannel( "lobby-7" ) );
        ASSERT_TRUE( GUCEF_NULL == chat.GetWidget( "ChatTabs" ) && GUCEF_NULL == chat.GetWidget( "Close" ) && GUCEF_NULL != chat.GetWidget( "Send" ) );
        ASSERT_TRUE( 2 == chat.GetShownMembers().size() );

        // only its messages show
        ASSERT_TRUE( bob.SendToChannel( "lobby-7", "In the lobby" ) );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Bob: In the lobby" ) );
        ASSERT_TRUE( alice.SendToChannel( "general", "Elsewhere" ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() && !AnyLineHas( chat.GetShownLines(), "Elsewhere" ) );

        // direct messages are not for this form, and the player cannot go elsewhere
        ASSERT_TRUE( bob.SendDirect( "Alice", "Psst" ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() && !AnyLineHas( chat.GetShownLines(), "Psst" ) );
        ASSERT_FALSE( chat.OpenDirectConversation( "Bob" ) || chat.JoinChannel( "another" ) || chat.CloseTab( 0 ) );
        ASSERT_TRUE( 1 == chat.GetTabCount() );

        // sending
        TypeInto( context, chat.GetWidget( "Input" ), "Hello", true );
        ASSERT_TRUE( AnyLineHas( chat.GetShownLines(), "Alice: Hello" ) );

        // not single any more: all the channels of the service
        chat.SetSingleChannel( "" );
        ASSERT_TRUE( chat.GetSingleChannel().IsNULLOrEmpty() && 2 == chat.GetTabCount() && GUCEF_NULL != chat.GetWidget( "ChatTabs" ) && GUCEF_NULL != chat.GetWidget( "Close" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
