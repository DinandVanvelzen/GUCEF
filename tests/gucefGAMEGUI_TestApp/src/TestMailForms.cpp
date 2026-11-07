/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGAMEGUI_CEMailAccountForm.h"
#include "gucefGAMEGUI_CMatchTableForm.h"
#include "gucefGAMEGUI_TestCommon.h"
#include "TestMatchLayoutChecks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether a widget is entirely inside a rectangle */
bool
IsInside( GUI::CWidget* widget, const GUI::CRect& outer )
{
    if ( GUCEF_NULL == widget )
        return false;
    const GUI::CRect& r = widget->GetRect();
    return r.left >= outer.left && r.top >= outer.top && r.Right() <= outer.Right() && r.Bottom() <= outer.Bottom();
}

/** Whether two widgets overlap */
bool
Overlap( GUI::CWidget* a, GUI::CWidget* b )
{
    const GUI::CRect& x = a->GetRect();
    const GUI::CRect& y = b->GetRect();
    return x.left < y.Right() && y.left < x.Right() && x.top < y.Bottom() && y.top < x.Bottom();
}

/** Whether a widget is the ancestor or the widget itself */
bool
IsWithin( GUI::CWidget* widget, GUI::CWidget* ancestor )
{
    for ( GUI::CWidget* p=widget; GUCEF_NULL != p; p=p->GetParentWidget() )
        if ( p == ancestor )
            return true;
    return false;
}

/** Records the events of a form with the text they carry */
class CFormRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CFormRecorder > TEventCallback;

    CORE::CString::StringVector names;
    CORE::CString::StringVector texts;
    GAMEGUI::CEMailAccountFormData account;

    CFormRecorder( void ) : CORE::CObserver(), names(), texts(), account() {}
    virtual ~CFormRecorder() { UnsubscribeAllFromObserver(); }

    void Watch( CORE::CNotifier* form, const CORE::CEvent& eventId, const char* name )
    {
        TEventCallback callback( this, &CFormRecorder::OnEvent );
        SubscribeTo( form, eventId, callback );
        m_names[ eventId.GetName() ] = name;
    }

    UInt32 Count( const char* name ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<names.size(); ++i )
            if ( names[ i ] == name )
                ++count;
        return count;
    }

    CORE::CString LastText( const char* name ) const
    {
        for ( size_t i=names.size(); i>0; --i )
            if ( names[ i - 1 ] == name )
                return texts[ i - 1 ];
        return CORE::CString::Empty;
    }

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        names.push_back( m_names[ eventId.GetName() ] );
        CORE::CString text;
        if ( GUCEF_NULL != eventData )
        {
            if ( GAMEGUI::CMatchTableAction* action = CORE::CICloneable::CloneableAs< GAMEGUI::CMatchTableAction >( eventData ) )
                text = action->text;
            if ( GAMEGUI::CEMailAccountFormData* data = CORE::CICloneable::CloneableAs< GAMEGUI::CEMailAccountFormData >( eventData ) )
                account = *data;
        }
        texts.push_back( text );
    }

    private:

    GUCEF::map< CORE::CString, CORE::CString > m_names;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.MailForms" );

    GUCEF_TESTFW_TESTCASE( "MailAccount: The fields are filled and read back, Save and Test hand on what was typed, the player code cannot be changed, Cancel closes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        CFormRecorder recorder;
        recorder.Watch( &form, GAMEGUI::CEMailAccountForm::SaveRequestedEvent, "save" );
        recorder.Watch( &form, GAMEGUI::CEMailAccountForm::TestRequestedEvent, "test" );
        recorder.Watch( &form, GAMEGUI::CEMailAccountForm::CancelledEvent, "cancel" );
        ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() && !context->IsModalRoot( form.GetRootWidget() ) );

        GAMEGUI::CEMailAccountFormData data;
        data.address = "alice@mail.example";
        data.displayName = "Alice";
        data.userName = "alice.login";
        data.password = "pw-for-the-test";
        data.useImap = false;
        data.incomingHost = "pop.mail.example";
        data.incomingPort = 995;
        data.incomingSecurity = 0;
        data.outgoingHost = "smtp.mail.example";
        data.outgoingPort = 587;
        data.outgoingSecurity = 1;
        data.mailbox = "Games";
        data.pollSeconds = 90;
        data.useIdle = false;
        data.processedAction = 2;
        data.processedFolder = "Redux Games";
        data.pop3Delete = true;
        data.autoAccept = false;
        form.SetData( data );
        form.SetPlayerCode( "ab12" );
        GAMEGUI::CEMailAccountFormData read = form.GetData();
        ASSERT_TRUE( read.address == data.address && read.displayName == data.displayName && read.userName == data.userName && read.password == data.password );
        ASSERT_TRUE( !read.useImap && read.incomingHost == data.incomingHost && 995 == read.incomingPort && 0 == read.incomingSecurity );
        ASSERT_TRUE( read.outgoingHost == data.outgoingHost && 587 == read.outgoingPort && 1 == read.outgoingSecurity && read.mailbox == "Games" && 90 == read.pollSeconds && !read.useIdle );
        ASSERT_TRUE( 2 == read.processedAction && read.processedFolder == "Redux Games" && read.pop3Delete && !read.autoAccept );
        ASSERT_TRUE( "ab12" == form.GetPlayerCode() && form.GetWidget( "PlayerCode" )->GetBoolProperty( GUI::EWidgetProperty_ReadOnly ) );

        // the defaults of a mailbox: IMAP, the folder INBOX, every minute, the usual ports
        GAMEGUI::CEMailAccountFormData fresh;
        form.SetData( fresh );
        read = form.GetData();
        ASSERT_TRUE( read.useImap && "INBOX" == read.mailbox && 60 == read.pollSeconds && 0 == read.incomingPort && 0 == read.outgoingPort && read.useIdle );
        // processed game mail: copied by default, no folder, nothing deleted from a POP3 server
        ASSERT_TRUE( 1 == read.processedAction && read.processedFolder.IsNULLOrEmpty() && !read.pop3Delete );
        // invitations of people the player agreed to be introduced to are accepted by themselves until the player says otherwise
        ASSERT_TRUE( read.autoAccept );

        // a port that is no port is the usual one
        form.SetData( data );
        form.GetWidget( "IncomingPort" )->SetProperty( GUI::EWidgetProperty_Text, "99999" );
        form.GetWidget( "OutgoingPort" )->SetProperty( GUI::EWidgetProperty_Text, "abc" );
        read = form.GetData();
        ASSERT_TRUE( 0 == read.incomingPort && 0 == read.outgoingPort );
        form.SetData( data );

        ClickWidget( context, form.GetWidget( "Save" ) );
        ASSERT_TRUE( 1 == recorder.Count( "save" ) && recorder.account.address == "alice@mail.example" && recorder.account.password == "pw-for-the-test" && form.IsOpen() );
        ClickWidget( context, form.GetWidget( "Test" ) );
        ASSERT_TRUE( 1 == recorder.Count( "test" ) && recorder.account.outgoingHost == "smtp.mail.example" );
        form.SetTesting( true );
        ASSERT_TRUE( form.IsTesting() && !IsWidgetEnabled( form, "Test" ) );
        form.RequestTest();
        ASSERT_TRUE( 1 == recorder.Count( "test" ) );
        form.SetTesting( false );
        ASSERT_TRUE( !form.IsTesting() && IsWidgetEnabled( form, "Test" ) );

        form.SetStatus( "The outgoing server accepted the login.", false );
        ASSERT_TRUE( "The outgoing server accepted the login." == form.GetStatus() );
        form.SetStatus( "The password is wrong.", true );
        ASSERT_TRUE( "The password is wrong." == form.GetStatus() );

        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 1 == recorder.Count( "cancel" ) && !form.IsOpen() && !form.IsVisible() );
        ASSERT_TRUE( form.Open() );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 2 == recorder.Count( "cancel" ) && !form.IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: The choice for processed game mail has three plain choices, the note says that Move takes the mail out of the inbox, and the choice is read back" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        ASSERT_TRUE( form.Open() );
        GUI::CCombobox* choice = form.GetWidgetAs< GUI::CCombobox >( "ProcessedAction" );
        ASSERT_TRUE( GUCEF_NULL != choice );
        if ( GUCEF_NULL == choice )
            return;
        const CORE::CString items = choice->GetProperty( GUI::EWidgetProperty_Items );
        ASSERT_TRUE( 3 == items.ParseElements( '\n', false ).size() );
        ASSERT_TRUE( items.HasSubstr( "Leave" ) >= 0 && items.HasSubstr( "Copy" ) >= 0 && items.HasSubstr( "Move" ) >= 0 );
        const CORE::CString note = form.GetWidget( "ProcessedNote" )->GetProperty( GUI::EWidgetProperty_Text );
        ASSERT_TRUE( note.HasSubstr( "Move" ) >= 0 && note.HasSubstr( "removes" ) >= 0 && note.HasSubstr( "inbox" ) >= 0 );
        for ( Int32 i=0; i<3; ++i )
        {
            GAMEGUI::CEMailAccountFormData data;
            data.processedAction = i;
            form.SetData( data );
            ASSERT_TRUE( i == form.GetData().processedAction );
        }
        // a value outside the choices is the default
        GAMEGUI::CEMailAccountFormData wrong;
        wrong.processedAction = 77;
        form.SetData( wrong );
        ASSERT_TRUE( 1 == form.GetData().processedAction );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: The window and every control stay inside the screen and apart from each other from 1242 x 952 to the ultrawide sizes" )
    try
    {
        struct SSize { UInt32 width; UInt32 height; };
        const SSize sizes[] = { { 1242, 952 }, { 3440, 1440 }, { 3840, 1080 }, { 1024, 768 } };
        for ( int s=0; s<4; ++s )
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, false );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CEMailAccountForm form( context );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();
            context->RenderFrame( 16 );
            GUI::CWidget* window = form.GetRootWidget();
            const GUI::CRect screen( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height );
            ASSERT_TRUE( IsInside( window, screen ) );
            const char* const names[] = { "Address", "DisplayName", "UserName", "Password", "PlayerCode", "Mailbox", "PollSeconds", "UseIdle", "AutoAccept", "OutgoingHost", "OutgoingPort", "OutgoingSecurity", "Protocol",
                                          "IncomingHost", "IncomingPort", "IncomingSecurity", "ProcessedAction", "ProcessedFolder", "ProcessedNote", "Pop3Delete", "Status", "Test", "Save", "Cancel" };
            const size_t count = sizeof( names ) / sizeof( names[ 0 ] );
            // the form has two pages: the controls of the page that is shown stay apart, the others are not seen
            for ( int page=0; page<2; ++page )
            {
                form.ShowServersPage( 1 == page );
                context->RenderFrame( 16 );
                context->FlushChanges();
                for ( size_t i=0; i<count; ++i )
                {
                    GUI::CWidget* one = form.GetWidget( names[ i ] );
                    ASSERT_TRUE( GUCEF_NULL != one && IsInside( one, window->GetRect() ) );
                    for ( size_t j=i+1; j<count && GUCEF_NULL != one && one->IsEffectivelyVisible(); ++j )
                    {
                        GUI::CWidget* other = form.GetWidget( names[ j ] );
                        if ( GUCEF_NULL == other || !other->IsEffectivelyVisible() )
                            continue;
                        if ( Overlap( one, other ) )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layout: " << names[ i ] << " overlaps " << names[ j ] << " at " << sizes[ s ].width << " x " << sizes[ s ].height );
                        ASSERT_TRUE( !Overlap( one, other ) );
                    }
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchTable: A game shows the table and the rest; every request carries what the player picked or typed" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CMatchTableForm form( context );
        CFormRecorder recorder;
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ProposeRequestedEvent, "propose" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::VoteYesRequestedEvent, "yes" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::VoteNoRequestedEvent, "no" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ApplyRequestedEvent, "apply" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ResendRequestedEvent, "resend" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::AskMissingRequestedEvent, "ask" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ExportFileRequestedEvent, "exportfile" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ExportTextRequestedEvent, "exporttext" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ImportFileRequestedEvent, "importfile" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::ImportTextRequestedEvent, "importtext" );
        recorder.Watch( &form, GAMEGUI::CMatchTableForm::CloseRequestedEvent, "close" );
        ASSERT_TRUE( form.Open() && form.IsOpen() && !context->IsModalRoot( form.GetRootWidget() ) );

        CORE::CString::StringVector lines;
        lines.push_back( "Alice (made the game), alice@mail.example, playing" );
        lines.push_back( "Bob (you), bob@mail.example, invited" );
        lines.push_back( "Carol, carol@mail.example, invited" );
        form.SetTable( "Friday game", lines );
        form.SetStatement( "Everybody sees the mail addresses of all the players." );

        // the settings of the game are told in plain lines next to the players, in an invitation, in a game and while the players are introduced
        ASSERT_TRUE( form.GetSummary().IsNULLOrEmpty() );
        CORE::CString::StringVector summary;
        summary.push_back( "Scenario: Custom" );
        summary.push_back( "Galaxy: 125 systems, star density low" );
        summary.push_back( "Time for a turn: 7 days" );
        form.SetSummary( summary );
        ASSERT_TRUE( form.GetSummary().HasSubstr( "Scenario: Custom" ) >= 0 && form.GetSummary().HasSubstr( "125 systems" ) >= 0 && form.GetSummary().HasSubstr( "7 days" ) >= 0 );
        ASSERT_TRUE( form.GetWidget( "Summary" )->IsVisible() && form.GetWidget( "SummaryLabel" )->IsVisible() );
        ASSERT_TRUE( form.GetWidget( "Summary" )->IsEffectivelyVisible() );
        form.SetSummary( CORE::CString::StringVector() );
        ASSERT_TRUE( form.GetSummary().IsNULLOrEmpty() );
        form.SetSummary( summary );

        GUI::CWidget* tableBox = form.GetWidget( "Table" );
        ASSERT_TRUE( GUCEF_NULL != tableBox && tableBox->GetProperty( GUI::EWidgetProperty_Text ).HasSubstr( "carol@mail.example" ) >= 0 && form.GetWidget( "Statement" )->IsVisible() );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Accept" ) && GUCEF_NULL == form.GetWidget( "Decline" ) && GUCEF_NULL == form.GetWidget( "IntroStart" ) );

        // a game
        ASSERT_TRUE( form.GetWidget( "Propose" )->IsVisible() && form.GetWidget( "ExportFile" )->IsVisible() );
        form.SetDelivery( "Your turn arrived." );
        ASSERT_TRUE( "Your turn arrived." == form.GetDelivery() );
        ClickWidget( context, form.GetWidget( "Resend" ) );
        ClickWidget( context, form.GetWidget( "AskMissing" ) );
        ClickWidget( context, form.GetWidget( "ExportText" ) );
        ClickWidget( context, form.GetWidget( "Apply" ) );
        ASSERT_TRUE( 1 == recorder.Count( "resend" ) && 1 == recorder.Count( "ask" ) && 1 == recorder.Count( "exporttext" ) && 0 == recorder.Count( "apply" ) );
        form.SetApplyEnabled( true );
        ClickWidget( context, form.GetWidget( "Apply" ) );
        ASSERT_TRUE( 1 == recorder.Count( "apply" ) );

        // a player to hand to the computer is picked from the list of players
        CORE::CString::StringVector ids, names;
        ids.push_back( "id-carol" ); names.push_back( "Carol" );
        ids.push_back( "id-dave" ); names.push_back( "Dave" );
        form.SetTargets( ids, names );
        ASSERT_TRUE( PickItem( context, form.GetWidget( "Target" ), 1 ) );
        context->FlushChanges();
        ClickWidget( context, form.GetWidget( "Propose" ) );
        ASSERT_TRUE( 1 == recorder.Count( "propose" ) && "id-dave" == recorder.LastText( "propose" ) );

        // votes are about the proposal that is picked in the list
        form.RequestVote( true );
        ASSERT_TRUE( 0 == recorder.Count( "yes" ) );
        CORE::CString::StringVector proposalIds, proposalLines;
        proposalIds.push_back( "p-1" ); proposalLines.push_back( "Hand Carol to the computer: 1 yes of 2 needed" );
        proposalIds.push_back( "p-2" ); proposalLines.push_back( "Hand Dave to the computer: 2 yes, passed" );
        form.SetProposals( proposalIds, proposalLines );
        GUI::CListbox* proposals = GAMEGUI::WidgetAs< GUI::CListbox >( form.GetWidget( "Proposals" ), "Listbox" );
        ASSERT_TRUE( GUCEF_NULL != proposals );
        if ( GUCEF_NULL != proposals )
            proposals->SetSelectedIndex( 1 );
        ClickWidget( context, form.GetWidget( "VoteYes" ) );
        ClickWidget( context, form.GetWidget( "VoteNo" ) );
        ASSERT_TRUE( 1 == recorder.Count( "yes" ) && "p-2" == recorder.LastText( "yes" ) && 1 == recorder.Count( "no" ) && "p-2" == recorder.LastText( "no" ) );

        // files and text
        form.SetPath( "/tmp/turn.rdxturn" );
        ASSERT_TRUE( "/tmp/turn.rdxturn" == form.GetPath() );
        ClickWidget( context, form.GetWidget( "ExportFile" ) );
        ClickWidget( context, form.GetWidget( "ImportFile" ) );
        ASSERT_TRUE( 1 == recorder.Count( "exportfile" ) && "/tmp/turn.rdxturn" == recorder.LastText( "exportfile" ) && 1 == recorder.Count( "importfile" ) && "/tmp/turn.rdxturn" == recorder.LastText( "importfile" ) );
        form.GetWidget( "ImportTextField" )->SetProperty( GUI::EWidgetProperty_Text, "-----BEGIN TURN-----" );
        ClickWidget( context, form.GetWidget( "ImportText" ) );
        ASSERT_TRUE( 1 == recorder.Count( "importtext" ) && "-----BEGIN TURN-----" == recorder.LastText( "importtext" ) );
        form.SetOutput( "-----BEGIN TURN-----\nabc\n-----END TURN-----" );
        ASSERT_TRUE( form.GetOutput().HasSubstr( "abc" ) >= 0 );
        form.SetMessage( "The turn was taken.", false );
        ASSERT_TRUE( "The turn was taken." == form.GetMessage() );

        ClickWidget( context, form.GetWidget( "Close" ) );
        ASSERT_TRUE( 1 == recorder.Count( "close" ) && !form.IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MatchTable: The window and its controls stay inside the screen and apart from each other, as a game, from 1242 x 952 to the ultrawide sizes" )
    try
    {
        struct SSize { UInt32 width; UInt32 height; };
        const SSize sizes[] = { { 1242, 952 }, { 3440, 1440 }, { 3840, 1080 } };
        for ( int s=0; s<3; ++s )
        
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, false );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CMatchTableForm form( context );
            ASSERT_TRUE( form.Open() );
            CORE::CString::StringVector lines;
            for ( int i=0; i<6; ++i )
                lines.push_back( "Player " + CORE::Int32ToString( i ) + ", player" + CORE::Int32ToString( i ) + "@mail.example, playing" );
            form.SetTable( "A game with a name that is quite long to see how it fits in the title line of the window", lines );
            CORE::CString::StringVector summary;
            summary.push_back( "Scenario: Custom" );
            summary.push_back( "Galaxy: 125 systems, star density low" );
            summary.push_back( "Rules: Gold CD release" );
            summary.push_back( "Time for a turn: 7 days" );
            form.SetSummary( summary );
            form.SetStatement( "Everybody who plays this game can see the mail addresses of all the players: they are part of the game, and the turns are mailed between the players directly." );
            form.SetMessage( "The turn was taken.", false );
            form.SetDelivery( "Your turn was sent. There is no sign yet that it arrived." );
            context->RenderFrame( 16 );
            context->FlushChanges();
            context->RenderFrame( 16 );
            GUI::CWidget* window = form.GetRootWidget();
            ASSERT_TRUE( IsInside( window, GUI::CRect( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height ) ) );
            CORE::CString::StringVector names;
            const char* const common[] = { "Table", "Summary", "Statement", "Message", "Close" };
            const char* const game[] = { "Delivery", "Resend", "AskMissing", "Target", "Propose", "Proposals", "VoteYes", "VoteNo", "Apply", "Path", "ExportFile", "ExportText", "ImportFile", "ImportTextField", "ImportText", "Output" };
            for ( size_t i=0; i<sizeof( common ) / sizeof( common[ 0 ] ); ++i ) names.push_back( common[ i ] );
            for ( size_t i=0; i<sizeof( game ) / sizeof( game[ 0 ] ); ++i ) names.push_back( game[ i ] );
            for ( size_t i=0; i<names.size(); ++i )
            {
                GUI::CWidget* one = form.GetWidget( names[ i ] );
                ASSERT_TRUE( GUCEF_NULL != one && IsInside( one, window->GetRect() ) );
                for ( size_t j=i+1; j<names.size() && GUCEF_NULL != one; ++j )
                {
                    GUI::CWidget* other = form.GetWidget( names[ j ] );
                    if ( GUCEF_NULL != other && Overlap( one, other ) )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layout: " << names[ i ] << " overlaps " << names[ j ] << " at " << sizes[ s ].width << " x " << sizes[ s ].height << " " );
                    ASSERT_TRUE( GUCEF_NULL == other || !Overlap( one, other ) );
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
