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

/** Records what the form that creates a lobby asks for */
class CCreateRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCreateRecorder > TEventCallback;

    UInt32 creates;
    UInt32 cancels;
    GAMEGUI::CCreateLobbyRequest last;

    CCreateRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), creates( 0 ), cancels( 0 ), last()
    {
        TEventCallback createCallback( this, &CCreateRecorder::OnCreateRequested );
        SubscribeTo( form, GAMEGUI::CCreateLobbyForm::CreateRequestedEvent, createCallback );
        TEventCallback cancelCallback( this, &CCreateRecorder::OnCancelled );
        SubscribeTo( form, GAMEGUI::CCreateLobbyForm::CancelledEvent, cancelCallback );
    }

    virtual ~CCreateRecorder() { UnsubscribeAllFromObserver(); }

    void OnCreateRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++creates;
            last = *static_cast< GAMEGUI::CCreateLobbyRequest* >( eventData );
        }
    }

    void OnCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++cancels;
    }
};

/*-------------------------------------------------------------------------*/

void
PerformCreateLobbyFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.CreateLobby" );

    GUCEF_TESTFW_TESTCASE( "CreateLobby: The fields show the template, the settings of the schema and the lobby services" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        fixture.params.name = "My game";
        fixture.params.password = "secret";
        fixture.params.maxPlayers = 6;
        fixture.params.allowSpectators = false;
        fixture.params.isPublic = true;
        fixture.params.initialSettings.SetAllowMultipleValues( false );
        fixture.params.initialSettings.Set( "map", "ice" );

        GAMEGUI::CCreateLobbyForm form( context );
        CORE::CString::StringVector services;
        services.push_back( "test" );
        services.push_back( "gucef" );
        form.SetServiceNames( services );
        form.SetTemplate( fixture.params );
        ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( context->GetFocus() == form.GetWidget( "Name" ) );

        ASSERT_TRUE( "My game" == form.GetWidget( "Name" )->GetProperty( "Text" ) && "secret" == form.GetWidget( "Password" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 6 == form.GetWidget( "MaxPlayers" )->GetIntProperty( "Value" ) && !form.GetWidget( "Spectators" )->GetBoolProperty( "Checked" ) );
        ASSERT_TRUE( 1 == form.GetWidgetAs< GUI::CCombobox >( "Where" )->GetSelectedIndex() );

        // the lobby services: the ones given, the first chosen
        ASSERT_TRUE( "test" == form.GetServiceName() );
        ASSERT_TRUE( form.SelectService( "gucef" ) && "gucef" == form.GetServiceName() && !form.SelectService( "nothing" ) );

        // the settings of the schema, with the values of the template over the defaults
        GAMEGUI::CLobbySettingsPanelForm* panel = form.GetSettingsPanel();
        ASSERT_TRUE( GUCEF_NULL != panel && 6 == panel->GetSettingCount() );
        if ( GUCEF_NULL == panel )
            return;
        ASSERT_TRUE( "ice" == panel->GetValue( "map" ) && "3" == panel->GetValue( "speed" ) && "hello" == panel->GetValue( "motd" ) );
        ASSERT_TRUE( panel->GetRootWidget()->GetParentWidget() == form.GetWidget( "SettingsHolder" ) );

        // without services given, the lobby services of gucefMULTIPLAY are offered: the built-in "gucef" and the registered ones
        TTestLobbyServiceFactory testServiceFactory;
        MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().RegisterConcreteFactory( CTestLobbyService::ServiceName, &testServiceFactory );
        form.SetServiceNames( CORE::CString::StringVector() );
        ASSERT_TRUE( "gucef" == form.GetServiceName() && form.SelectService( "test" ) && "test" == form.GetServiceName() );
        MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().UnregisterConcreteFactory( CTestLobbyService::ServiceName );
        form.SetServiceNames( CORE::CString::StringVector() );
        ASSERT_TRUE( "gucef" == form.GetServiceName() && !form.SelectService( "test" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CreateLobby: Create wants a name and notifies the params with what the player filled in" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        fixture.params.name = "";
        GAMEGUI::CCreateLobbyForm form( context );
        CCreateRecorder recorder( &form );
        CORE::CString::StringVector services;
        services.push_back( "test" );
        form.SetServiceNames( services );
        form.SetTemplate( fixture.params );
        ASSERT_TRUE( form.Open() );

        // no name, no lobby
        ClickWidget( context, form.GetWidget( "Create" ) );
        ASSERT_TRUE( 0 == recorder.creates && form.IsOpen() && !form.GetErrorText().IsNULLOrEmpty() );
        TypeInto( context, form.GetWidget( "Name" ), "   ", true );
        ASSERT_TRUE( 0 == recorder.creates && form.IsOpen() );

        // fill in the fields
        form.GetWidget( "Password" )->SetProperty( "Text", "pw" );
        form.GetWidget( "MaxPlayers" )->ChangeValueByUser( "Value", "3" );
        ClickWidget( context, form.GetWidget( "Spectators" ) );                 // the template had them
        ASSERT_TRUE( PickItem( context, form.GetWidget( "Where" ), 1 ) );
        form.GetSettingsPanel()->GetControl( "speed" )->ChangeValueByUser( "Value", "4" );
        form.GetSettingsPanel()->GetControl( "fog" )->ChangeValueByUser( "Checked", "true" );
        TypeInto( context, form.GetWidget( "Name" ), " Brawl ", true );

        ASSERT_TRUE( 1 == recorder.creates && !form.IsOpen() && !form.IsVisible() && form.GetErrorText().IsNULLOrEmpty() );
        const MULTIPLAY::CLobbyCreateParams& params = recorder.last.params;
        ASSERT_TRUE( "Brawl" == params.name && "pw" == params.password && 3 == params.maxPlayers && !params.allowSpectators && params.isPublic );
        ASSERT_TRUE( "testgame" == params.gameId && "1.0" == params.gameVersion && MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER == params.topology );
        ASSERT_TRUE( 6 == params.schema.settings.size() && 2 == params.schema.factions.size() && params.schema.uniqueColours );
        ASSERT_TRUE( "test" == recorder.last.serviceName );
        CORE::CValueList start;
        params.GetStartSettings( start );
        ASSERT_TRUE( "4" == start.GetValueAlways( "speed" ).AsString() && "true" == start.GetValueAlways( "fog" ).AsString() && "dust" == start.GetValueAlways( "map" ).AsString() );

        // a lobby made with them has them
        CTestLobbyService service;
        MULTIPLAY::CPlayerIdentity identity;
        identity.userId = "alice-id";
        identity.displayName = "Alice";
        CORE::CFutureResult connected( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( service.Connect( identity, connected ) );
        CORE::CFutureResult created( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CILobbyPtr lobby;
        ASSERT_TRUE( service.CreateLobby( params, created ) && MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( created, lobby ) );
        ASSERT_TRUE( !lobby.IsNULL() && "Brawl" == lobby->GetState().name && "4" == lobby->GetState().GetSetting( "speed" ) && 3 == lobby->GetState().slots.size() );
        lobby = MULTIPLAY::CILobbyPtr();
        CTestLobbyService::Reset();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CreateLobby: Cancel and Escape close the form, a template with slots gets as many as there are players" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        for ( UInt32 i=0; i<2; ++i )
        {
            MULTIPLAY::CLobbySlot slot;
            slot.index = i;
            slot.team = i;
            fixture.params.slots.push_back( slot );
        }
        fixture.params.maxPlayers = 2;

        GAMEGUI::CCreateLobbyForm form( context );
        CCreateRecorder recorder( &form );
        form.SetTemplate( fixture.params );
        ASSERT_TRUE( form.Open() );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 1 == recorder.cancels && !form.IsOpen() && !form.IsVisible() );
        ASSERT_TRUE( form.Open() );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 2 == recorder.cancels && !form.IsVisible() && 0 == recorder.creates );

        // more players: more slots
        ASSERT_TRUE( form.Open() );
        form.GetWidget( "MaxPlayers" )->ChangeValueByUser( "Value", "4" );
        TypeInto( context, form.GetWidget( "Name" ), "Four", true );
        ASSERT_TRUE( 1 == recorder.creates && 4 == recorder.last.params.slots.size() && 3 == recorder.last.params.slots[ 3 ].index && 3 == recorder.last.params.slots[ 3 ].team );

        // fewer: fewer
        ASSERT_TRUE( form.Open() );
        form.GetWidget( "MaxPlayers" )->ChangeValueByUser( "Value", "1" );
        TypeInto( context, form.GetWidget( "Name" ), "Alone", true );
        ASSERT_TRUE( 2 == recorder.creates && 1 == recorder.last.params.slots.size() );

        // no service given: the built-in one of gucefMULTIPLAY
        ASSERT_TRUE( "gucef" == recorder.last.serviceName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
