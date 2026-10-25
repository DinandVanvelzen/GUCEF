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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the events of the form that hands over to the game */
class CConnectingRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CConnectingRecorder > TEventCallback;

    UInt32 cancels;
    UInt32 connects;
    UInt32 failures;
    CORE::CString failure;

    CConnectingRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), cancels( 0 ), connects( 0 ), failures( 0 ), failure()
    {
        TEventCallback cancelCallback( this, &CConnectingRecorder::OnCancelRequested );
        SubscribeTo( form, GAMEGUI::CConnectingForm::CancelRequestedEvent, cancelCallback );
        TEventCallback connectedCallback( this, &CConnectingRecorder::OnConnected );
        SubscribeTo( form, GAMEGUI::CConnectingForm::ConnectedEvent, connectedCallback );
        TEventCallback failedCallback( this, &CConnectingRecorder::OnFailed );
        SubscribeTo( form, GAMEGUI::CConnectingForm::FailedEvent, failedCallback );
    }

    virtual ~CConnectingRecorder() { UnsubscribeAllFromObserver(); }

    void OnCancelRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++cancels;
    }

    void OnConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++connects;
    }

    void OnFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++failures;
            failure = static_cast< GAMEGUI::CConnectingForm::TFailedEventData* >( eventData )->GetData();
        }
    }
};

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CLaunchDescription
MakeLaunch( bool withRelay )
{
    MULTIPLAY::CLaunchDescription launch;
    launch.gameId = "testgame";
    launch.topology = MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER;
    launch.serverMemberId = "alice-id";
    launch.localMemberId = "bob-id";

    MULTIPLAY::CLaunchMember server;
    server.id = "alice-id";
    server.name = "Alice";
    server.endpoints.push_back( "udp://192.168.1.5:7777" );
    server.endpoints.push_back( "tcp://192.168.1.5:7778" );
    launch.members.push_back( server );
    MULTIPLAY::CLaunchMember client;
    client.id = "bob-id";
    client.name = "Bob";
    client.endpoints.push_back( "udp://192.168.1.6:7777" );
    launch.members.push_back( client );

    if ( withRelay )
        launch.relay = "tcp://relay.example.com:9000";
    return launch;
}

/*-------------------------------------------------------------------------*/

void
PerformConnectingFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.Connecting" );

    GUCEF_TESTFW_TESTCASE( "Connecting: The endpoints of the launch description are tried in turn, the relay last" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CConnectingForm form( context );
        CConnectingRecorder recorder( &form );

        ASSERT_TRUE( form.Begin( MakeLaunch( true ) ) );
        ASSERT_TRUE( form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) && !form.HasFailed() );
        ASSERT_TRUE( 3 == form.GetEndpointCount() && "udp://192.168.1.5:7777" == form.GetCurrentEndpoint() && !form.IsUsingRelay() );
        ASSERT_TRUE( "Connecting to the game" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Trying udp://192.168.1.5:7777 (1 of 2)" == form.GetWidget( "Endpoint" )->GetProperty( "Text" ) );
        ASSERT_TRUE( form.GetWidget( "Relay" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        ASSERT_TRUE( form.TryNextEndpoint() );
        ASSERT_TRUE( "tcp://192.168.1.5:7778" == form.GetCurrentEndpoint() && "Trying tcp://192.168.1.5:7778 (2 of 2)" == form.GetWidget( "Endpoint" )->GetProperty( "Text" ) );

        // the relay is the fallback
        ASSERT_TRUE( form.TryNextEndpoint() );
        ASSERT_TRUE( form.IsUsingRelay() && "tcp://relay.example.com:9000" == form.GetCurrentEndpoint() );
        ASSERT_TRUE( "Trying tcp://relay.example.com:9000 (relay)" == form.GetWidget( "Endpoint" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Connecting through the relay" == form.GetWidget( "Status" )->GetProperty( "Text" ) && !form.GetWidget( "Relay" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // nothing is left
        ASSERT_FALSE( form.TryNextEndpoint() );
        ASSERT_TRUE( form.HasFailed() && 1 == recorder.failures && "No endpoint of the game could be reached" == recorder.failure );
        ASSERT_TRUE( form.GetCurrentEndpoint().IsNULLOrEmpty() && form.IsOpen() );
        ASSERT_TRUE( "Could not connect: No endpoint of the game could be reached" == form.GetWidget( "Status" )->GetProperty( "Text" ) && "Close" == form.GetWidget( "Cancel" )->GetProperty( "Text" ) );

        // begin again: as new
        ASSERT_TRUE( form.Begin( MakeLaunch( false ) ) );
        ASSERT_TRUE( 2 == form.GetEndpointCount() && !form.HasFailed() && "Cancel" == form.GetWidget( "Cancel" )->GetProperty( "Text" ) );
        ASSERT_TRUE( form.TryNextEndpoint() && !form.IsUsingRelay() && !form.TryNextEndpoint() && 2 == recorder.failures );

        // no endpoint at all
        ASSERT_TRUE( form.Begin( CORE::CString::StringVector(), CORE::CString::Empty ) );
        ASSERT_TRUE( 0 == form.GetEndpointCount() && form.GetCurrentEndpoint().IsNULLOrEmpty() && "There is no endpoint to connect to" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        ASSERT_FALSE( form.TryNextEndpoint() );

        // only the relay
        CORE::CString::StringVector none;
        ASSERT_TRUE( form.Begin( none, "tcp://relay:1" ) );
        ASSERT_TRUE( form.IsUsingRelay() && "tcp://relay:1" == form.GetCurrentEndpoint() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Connecting: The transfer shows, the hand-over ends in success or failure, Cancel and Escape cancel" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CConnectingForm form( context );
        CConnectingRecorder recorder( &form );

        ASSERT_TRUE( form.Begin( MakeLaunch( true ) ) );
        ASSERT_TRUE( 0.0f == form.GetProgress() && 0.0f == form.GetWidget( "Progress" )->GetFloatProperty( "Value" ) );
        form.SetProgress( 0.5f, "170 of 340 KB" );
        ASSERT_TRUE( 0.5f == form.GetProgress() && 50.0f == form.GetWidget( "Progress" )->GetFloatProperty( "Value" ) && "170 of 340 KB" == form.GetWidget( "ProgressText" )->GetProperty( "Text" ) );
        form.SetProgress( 1.5f, "" );
        ASSERT_TRUE( 1.0f == form.GetProgress() && 100.0f == form.GetWidget( "Progress" )->GetFloatProperty( "Value" ) );
        form.SetProgress( -1.0f, "" );
        ASSERT_TRUE( 0.0f == form.GetProgress() );
        form.SetStatusText( "Receiving the game state" );
        ASSERT_TRUE( "Receiving the game state" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        // success
        form.ReportConnected();
        ASSERT_TRUE( 1 == recorder.connects && !form.IsOpen() && !form.IsVisible() && 0 == recorder.cancels );

        // a failure the player reads, the button then says Close
        ASSERT_TRUE( form.Begin( MakeLaunch( true ) ) );
        form.ReportFailed( "The token was refused" );
        ASSERT_TRUE( 1 == recorder.failures && "The token was refused" == recorder.failure && form.IsOpen() && form.HasFailed() );
        ASSERT_TRUE( "Could not connect: The token was refused" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 1 == recorder.cancels && !form.IsOpen() && !form.IsVisible() );

        // cancel with the button and with Escape
        ASSERT_TRUE( form.Begin( MakeLaunch( true ) ) );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 2 == recorder.cancels && !form.IsVisible() );
        ASSERT_TRUE( form.Begin( MakeLaunch( true ) ) );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 3 == recorder.cancels && !form.IsVisible() && !form.IsOpen() );

        // a cancel when nothing is open is nothing
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 3 == recorder.cancels );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
