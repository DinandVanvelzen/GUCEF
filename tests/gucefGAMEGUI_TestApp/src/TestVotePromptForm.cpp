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
#include "TestLobbyFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the votes cast and the closing of a vote prompt */
class CVoteRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CVoteRecorder > TEventCallback;

    UInt32 casts;
    UInt32 closes;
    CORE::CString voteId;
    bool yes;

    CVoteRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), casts( 0 ), closes( 0 ), voteId(), yes( false )
    {
        TEventCallback castCallback( this, &CVoteRecorder::OnVoteCast );
        SubscribeTo( form, GAMEGUI::CVotePromptForm::VoteCastEvent, castCallback );
        TEventCallback closedCallback( this, &CVoteRecorder::OnClosed );
        SubscribeTo( form, GAMEGUI::CVotePromptForm::ClosedEvent, closedCallback );
    }

    virtual ~CVoteRecorder() { UnsubscribeAllFromObserver(); }

    void OnVoteCast( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            const GAMEGUI::CVoteCastEventData* cast = static_cast< const GAMEGUI::CVoteCastEventData* >( eventData );
            ++casts;
            voteId = cast->voteId;
            yes = cast->yes;
        }
    }

    void OnClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++closes;
    }
};

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CLobbyVote
MakeSettingVote( void )
{
    MULTIPLAY::CLobbyVote vote;
    vote.id = "vote-7";
    vote.what = "setting";
    vote.proposer = "bob-id";
    vote.key = "speed";
    vote.value = "4";
    vote.yes = 1;
    vote.no = 0;
    vote.needed = 2;
    return vote;
}

/*-------------------------------------------------------------------------*/

void
PerformVotePromptFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.VotePrompt" );

    GUCEF_TESTFW_TESTCASE( "VotePrompt: A vote is described, with the tally and the time left, and answered with the buttons and Escape" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CVotePromptForm form( context );
        CVoteRecorder recorder( &form );
        ASSERT_FALSE( form.Cast( true ) );

        MULTIPLAY::CLobbyVote vote = MakeSettingVote();
        vote.deadline = CORE::CDateTime::NowUTCDateTime();
        vote.deadline.AddSeconds( 30 );
        ASSERT_TRUE( form.Open( vote, "Bob" ) && form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( "Bob proposes to change speed to 4" == form.GetWidget( "What" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Yes 1  No 0  (2 needed)" == form.GetWidget( "Tally" )->GetProperty( "Text" ) );
        CORE::CString timeLeft = form.GetWidget( "TimeLeft" )->GetProperty( "Text" );
        ASSERT_TRUE( "Time left: 30 s" == timeLeft || "Time left: 29 s" == timeLeft );
        ASSERT_TRUE( "vote-7" == form.GetVote().id );

        // yes
        ClickWidget( context, form.GetWidget( "Yes" ) );
        ASSERT_TRUE( 1 == recorder.casts && "vote-7" == recorder.voteId && recorder.yes && 1 == recorder.closes && !form.IsOpen() && !form.IsVisible() );

        // no
        vote.id = "vote-8";
        ASSERT_TRUE( form.Open( vote, "Bob" ) );
        ClickWidget( context, form.GetWidget( "No" ) );
        ASSERT_TRUE( 2 == recorder.casts && "vote-8" == recorder.voteId && !recorder.yes && !form.IsOpen() );

        // Escape is no
        vote.id = "vote-9";
        ASSERT_TRUE( form.Open( vote, "Bob" ) );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 3 == recorder.casts && "vote-9" == recorder.voteId && !recorder.yes && !form.IsOpen() && 3 == recorder.closes );

        // no deadline, no time left shown; a deadline that came: nought
        vote.deadline = CORE::CDateTime();
        ASSERT_TRUE( form.Open( vote, "Bob" ) && form.GetWidget( "TimeLeft" )->GetProperty( "Text" ).IsNULLOrEmpty() );
        vote.deadline = CORE::CDateTime::NowUTCDateTime();
        ASSERT_TRUE( form.Open( vote, "Bob" ) && "Time left: 0 s" == form.GetWidget( "TimeLeft" )->GetProperty( "Text" ) );

        // the time moves on with the frames of the context
        vote.deadline = CORE::CDateTime::NowUTCDateTime();
        vote.deadline.AddSeconds( 5 );
        ASSERT_TRUE( form.Open( vote, "Bob" ) );
        timeLeft = form.GetWidget( "TimeLeft" )->GetProperty( "Text" );
        vote.deadline = CORE::CDateTime::NowUTCDateTime();
        vote.deadline.AddSeconds( 90 );
        form.Open( vote, "Bob" );
        context->RenderFrame( 16 );
        ASSERT_TRUE( "Time left: 90 s" == form.GetWidget( "TimeLeft" )->GetProperty( "Text" ) || "Time left: 89 s" == form.GetWidget( "TimeLeft" )->GetProperty( "Text" ) );
        form.Close();
        ASSERT_FALSE( form.IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VotePrompt: The question names the setting by its label and the proposer, for a start too" )
    try
    {
        MULTIPLAY::CLobbyVote vote = MakeSettingVote();
        MULTIPLAY::CLobbySettingsSchema schema;
        MULTIPLAY::CLobbySettingDef speed;
        speed.key = "speed";
        speed.label = "Game speed";
        schema.AddSetting( speed );
        ASSERT_TRUE( "Bob proposes to change speed to 4" == GAMEGUI::CVotePromptForm::DescribeVote( vote, "Bob", GUCEF_NULL ) );
        ASSERT_TRUE( "Bob proposes to change Game speed to 4" == GAMEGUI::CVotePromptForm::DescribeVote( vote, "Bob", &schema ) );
        vote.key = "unknown";
        ASSERT_TRUE( "Bob proposes to change unknown to 4" == GAMEGUI::CVotePromptForm::DescribeVote( vote, "Bob", &schema ) );
        vote.what = "start";
        ASSERT_TRUE( "Bob proposes to start the game" == GAMEGUI::CVotePromptForm::DescribeVote( vote, "Bob", &schema ) );
        vote.what = "kick";
        ASSERT_TRUE( "Bob proposes: kick" == GAMEGUI::CVotePromptForm::DescribeVote( vote, "Bob", &schema ) );
        vote.yes = 3;
        vote.no = 2;
        vote.needed = 4;
        ASSERT_TRUE( "Yes 3  No 2  (4 needed)" == GAMEGUI::CVotePromptForm::DescribeTally( vote ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VotePrompt: Bound to a lobby the prompt opens for the members that can vote, keeps the tally and casts" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        GAMEGUI::CVotePromptForm bobPrompt( context );
        GAMEGUI::CVotePromptForm carolPrompt( context );
        GAMEGUI::CVotePromptForm davePrompt( context );
        CVoteRecorder carolRecorder( &carolPrompt );
        ASSERT_TRUE( bobPrompt.Build() && carolPrompt.Build() && davePrompt.Build() );
        bobPrompt.SetLobby( fixture.bob );
        carolPrompt.SetLobby( fixture.carol );
        davePrompt.SetLobby( fixture.dave );
        ASSERT_TRUE( fixture.carol.GetPointerAlways() == carolPrompt.GetLobby().GetPointerAlways() );

        // Bob asks for a faster game
        ASSERT_TRUE( fixture.bob->SetSetting( "speed", "4" ) );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() );
        ASSERT_FALSE( bobPrompt.IsOpen() );                 // the one who proposes agrees already
        ASSERT_FALSE( davePrompt.IsOpen() );                // a spectator does not vote
        ASSERT_TRUE( carolPrompt.IsOpen() && carolPrompt.IsVisible() );
        ASSERT_TRUE( "Bob proposes to change Game speed to 4" == carolPrompt.GetWidget( "What" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Yes 1  No 0  (2 needed)" == carolPrompt.GetWidget( "Tally" )->GetProperty( "Text" ) );
        ASSERT_TRUE( !carolPrompt.GetWidget( "TimeLeft" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // another vote moves the tally of the prompt
        ASSERT_TRUE( fixture.alice->Vote( fixture.alice->GetState().activeVote.id, false ) );
        ASSERT_TRUE( carolPrompt.IsOpen() && "Yes 1  No 1  (2 needed)" == carolPrompt.GetWidget( "Tally" )->GetProperty( "Text" ) );

        // Carol says yes with the button: the lobby has the vote, it passes, the setting changes
        ClickWidget( context, carolPrompt.GetWidget( "Yes" ) );
        ASSERT_TRUE( 1 == carolRecorder.casts && carolRecorder.yes && 1 == carolRecorder.closes && !carolPrompt.IsOpen() );
        ASSERT_TRUE( !fixture.alice->GetState().activeVote.IsActive() && "4" == fixture.alice->GetState().GetSetting( "speed" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VotePrompt: A vote that ends, or that the member answered another way, takes the prompt away" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        GAMEGUI::CVotePromptForm carolPrompt( context );
        CVoteRecorder carolRecorder( &carolPrompt );
        carolPrompt.SetLobby( fixture.carol );

        // answered through the lobby: the prompt goes, the vote runs on until the last player answers
        ASSERT_TRUE( fixture.bob->SetSetting( "speed", "4" ) );
        ASSERT_TRUE( carolPrompt.IsOpen() );
        ASSERT_TRUE( fixture.carol->Vote( fixture.carol->GetState().activeVote.id, false ) );
        ASSERT_TRUE( !carolPrompt.IsOpen() && 1 == carolRecorder.closes && 0 == carolRecorder.casts );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() );
        ASSERT_TRUE( fixture.alice->Vote( fixture.alice->GetState().activeVote.id, false ) );
        ASSERT_TRUE( !fixture.alice->GetState().activeVote.IsActive() && "3" == fixture.alice->GetState().GetSetting( "speed" ) );

        // a lobby with a vote that runs: the prompt opens when it gets the lobby
        ASSERT_TRUE( fixture.bob->SetSetting( "speed", "5" ) );
        ASSERT_TRUE( carolPrompt.IsOpen() );
        GAMEGUI::CVotePromptForm lateComer( context );
        lateComer.SetLobby( fixture.alice );
        ASSERT_TRUE( lateComer.IsOpen() && "Bob proposes to change Game speed to 5" == lateComer.GetWidget( "What" )->GetProperty( "Text" ) );

        // the lobby goes away from the prompt: closed
        lateComer.SetLobby( MULTIPLAY::CILobbyPtr() );
        ASSERT_TRUE( !lateComer.IsOpen() && lateComer.GetLobby().IsNULL() );

        // the vote ends by running out of time: the prompt goes
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && carolPrompt.IsOpen() );
        PumpFor( 900 );
        ASSERT_TRUE( !fixture.alice->GetState().activeVote.IsActive() && "3" == fixture.alice->GetState().GetSetting( "speed" ) );
        ASSERT_TRUE( !carolPrompt.IsOpen() && 2 == carolRecorder.closes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
