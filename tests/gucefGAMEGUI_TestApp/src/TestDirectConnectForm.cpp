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

/** Records what the form to connect directly asks for */
class CDirectRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CDirectRecorder > TEventCallback;

    UInt32 connects;
    UInt32 cancels;
    GAMEGUI::CDirectConnectRequest last;

    CDirectRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), connects( 0 ), cancels( 0 ), last()
    {
        TEventCallback connectCallback( this, &CDirectRecorder::OnConnectRequested );
        SubscribeTo( form, GAMEGUI::CDirectConnectForm::ConnectRequestedEvent, connectCallback );
        TEventCallback cancelCallback( this, &CDirectRecorder::OnCancelled );
        SubscribeTo( form, GAMEGUI::CDirectConnectForm::CancelledEvent, cancelCallback );
    }

    virtual ~CDirectRecorder() { UnsubscribeAllFromObserver(); }

    void OnConnectRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++connects;
            last = *static_cast< GAMEGUI::CDirectConnectRequest* >( eventData );
        }
    }

    void OnCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++cancels;
    }
};

/*-------------------------------------------------------------------------*/

void
PerformDirectConnectFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.DirectConnect" );

    GUCEF_TESTFW_TESTCASE( "DirectConnect: An address is host:port or an endpoint URI" )
    try
    {
        CORE::CString endpoint;
        ASSERT_TRUE( GAMEGUI::CDirectConnectForm::ParseAddress( "10.0.0.5:7777", "udp", endpoint ) && "udp://10.0.0.5:7777" == endpoint );
        ASSERT_TRUE( GAMEGUI::CDirectConnectForm::ParseAddress( "  game.example.com:80  ", "ws", endpoint ) && "ws://game.example.com:80" == endpoint );
        ASSERT_TRUE( GAMEGUI::CDirectConnectForm::ParseAddress( "tcp://host:1234", "udp", endpoint ) && "tcp://host:1234" == endpoint );
        ASSERT_TRUE( GAMEGUI::CDirectConnectForm::ParseAddress( "WS://host:8080/play", "udp", endpoint ) && "ws://host:8080/play" == endpoint );
        ASSERT_FALSE( GAMEGUI::CDirectConnectForm::ParseAddress( "host", "udp", endpoint ) );
        ASSERT_TRUE( endpoint.IsNULLOrEmpty() );
        ASSERT_FALSE( GAMEGUI::CDirectConnectForm::ParseAddress( "host:0", "udp", endpoint ) );
        ASSERT_FALSE( GAMEGUI::CDirectConnectForm::ParseAddress( ":80", "udp", endpoint ) );
        ASSERT_FALSE( GAMEGUI::CDirectConnectForm::ParseAddress( "", "udp", endpoint ) );
        ASSERT_FALSE( GAMEGUI::CDirectConnectForm::ParseAddress( "host:abc", "udp", endpoint ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DirectConnect: A good address is notified with the password, a bad one says what is wrong" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CDirectConnectForm form( context );
        CDirectRecorder recorder( &form );
        ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( context->GetFocus() == form.GetWidget( "Address" ) && "udp" == form.GetDefaultScheme() );

        // nothing typed
        ClickWidget( context, form.GetWidget( "Connect" ) );
        ASSERT_TRUE( 0 == recorder.connects && form.IsOpen() && "Enter the address of the game" == form.GetErrorText() );

        // not an address
        form.SetAddress( "somewhere" );
        ASSERT_TRUE( !form.Connect() && 0 == recorder.connects && form.GetErrorText().HasSubstr( "host and a port", true ) >= 0 );

        // a good one, with a password
        form.SetAddress( "  192.168.1.5:7777 " );
        form.SetPassword( "secret" );
        ASSERT_TRUE( "  192.168.1.5:7777 " == form.GetAddress() && "secret" == form.GetPassword() );
        ClickWidget( context, form.GetWidget( "Connect" ) );
        ASSERT_TRUE( 1 == recorder.connects && !form.IsOpen() && !form.IsVisible() && form.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( "192.168.1.5:7777" == recorder.last.address && "udp://192.168.1.5:7777" == recorder.last.endpoint && "secret" == recorder.last.password );

        // Enter in the address connects too, with the scheme the application wants
        form.SetDefaultScheme( "tcp" );
        ASSERT_TRUE( form.Open() );
        form.SetPassword( "" );
        TypeInto( context, form.GetWidget( "Address" ), "game.example.com:4000", true );
        ASSERT_TRUE( 2 == recorder.connects && "tcp://game.example.com:4000" == recorder.last.endpoint && recorder.last.password.IsNULLOrEmpty() && !form.IsOpen() );

        // a URI keeps its scheme
        ASSERT_TRUE( form.Open() );
        form.SetAddress( "ws://game.example.com:8080/play" );
        TypeInto( context, form.GetWidget( "Password" ), "pw", true );
        ASSERT_TRUE( 3 == recorder.connects && "ws://game.example.com:8080/play" == recorder.last.endpoint && "pw" == recorder.last.password );

        // a failed try does not leave its error behind when the form opens again
        ASSERT_TRUE( form.Open() );
        form.SetAddress( "" );
        ASSERT_FALSE( form.Connect() );
        ASSERT_TRUE( form.Open() && form.GetErrorText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DirectConnect: Cancel and Escape close the form" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CDirectConnectForm form( context );
        CDirectRecorder recorder( &form );
        ASSERT_TRUE( form.Open() );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 1 == recorder.cancels && !form.IsOpen() && !form.IsVisible() );
        ASSERT_TRUE( form.Open() );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 2 == recorder.cancels && !form.IsVisible() && 0 == recorder.connects );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
