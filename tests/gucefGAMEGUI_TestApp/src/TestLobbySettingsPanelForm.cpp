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

/** Records the changes the player makes in a settings panel */
class CSettingRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CSettingRecorder > TEventCallback;

    UInt32 count;
    CORE::CString key;
    CORE::CString value;
    bool proposed;

    CSettingRecorder( CORE::CNotifier* panel )
        : CORE::CObserver(), count( 0 ), key(), value(), proposed( false )
    {
        TEventCallback callback( this, &CSettingRecorder::OnSettingChanged );
        SubscribeTo( panel, GAMEGUI::CLobbySettingsPanelForm::SettingChangedEvent, callback );
    }

    virtual ~CSettingRecorder() { UnsubscribeAllFromObserver(); }

    void OnSettingChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        const GAMEGUI::CSettingChangedEventData* change = static_cast< const GAMEGUI::CSettingChangedEventData* >( eventData );
        ++count;
        key = change->key;
        value = change->value;
        proposed = change->proposed;
    }
};

/*-------------------------------------------------------------------------*/

/** A visible widget of the context for a panel to fill */
static GUI::CWidget*
MakeHolder( GUI::CGUIContext* context, const char* name, UInt32 slot = 0 )
{
    GUI::CWidget* holder = context->CreateWidget( "Panel", name, GUCEF_NULL );
    holder->SetProperty( "Size", "0,640 0,430" );
    holder->SetProperty( "Position", "0," + CORE::UInt32ToString( 640 * ( slot % 2 ) ) + " 0," + CORE::UInt32ToString( 440 * ( slot / 2 ) ) );
    context->ShowRoot( holder, false );
    context->FlushChanges();
    return holder;
}

/*-------------------------------------------------------------------------*/

void
PerformLobbySettingsPanelFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.SettingsPanel" );

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: A control per setting of the schema, of the kind the type of the setting asks for" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        GAMEGUI::CLobbySettingsPanelForm panel;
        panel.SetSchema( fixture.params.schema );
        ASSERT_TRUE( 6 == panel.GetSettingCount() && GUCEF_NULL == panel.GetControl( "map" ) );     // not built yet
        ASSERT_TRUE( panel.Build( MakeHolder( context, "Holder" ) ) );

        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CCombobox >( panel.GetControl( "map" ), "Combobox" ) );
        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CSpinner >( panel.GetControl( "speed" ), "Spinner" ) );
        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CSpinner >( panel.GetControl( "handicap" ), "Spinner" ) );
        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CCheckbox >( panel.GetControl( "fog" ), "Checkbox" ) );
        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CSpinner >( panel.GetControl( "gravity" ), "Spinner" ) );
        ASSERT_TRUE( GUCEF_NULL != GAMEGUI::WidgetAs< GUI::CEditbox >( panel.GetControl( "motd" ), "Editbox" ) );
        ASSERT_TRUE( GUCEF_NULL == panel.GetControl( "nothing" ) );

        // the definitions arrive in the controls: items, range, step, decimals
        GUI::CCombobox::TStringVector items;
        GAMEGUI::WidgetAs< GUI::CCombobox >( panel.GetControl( "map" ), "Combobox" )->GetListItems( items );
        ASSERT_TRUE( 2 == items.size() && "dust" == items[ 0 ] && "ice" == items[ 1 ] );
        GUI::CWidget* speed = panel.GetControl( "speed" );
        ASSERT_TRUE( 1 == speed->GetIntProperty( "Min" ) && 5 == speed->GetIntProperty( "Max" ) && 0 == speed->GetIntProperty( "Decimals" ) );
        GUI::CWidget* gravity = panel.GetControl( "gravity" );
        ASSERT_TRUE( 1 == gravity->GetIntProperty( "Decimals" ) && gravity->GetFloatProperty( "Step" ) > 0.09f && gravity->GetFloatProperty( "Step" ) < 0.11f );

        // labels, and the group box of the two settings of a group
        ASSERT_TRUE( "Map" == panel.GetLabelText( "map" ) && "Game speed" == panel.GetLabelText( "speed" ) && "Fog of war" == panel.GetLabelText( "fog" ) );
        ASSERT_TRUE( GUCEF_NULL != panel.GetWidget( "Group0" ) && "Physics" == panel.GetWidget( "Group0" )->GetProperty( "Caption" ) );
        ASSERT_TRUE( GUCEF_NULL == panel.GetWidget( "Group1" ) );
        ASSERT_TRUE( panel.GetControl( "gravity" )->GetParentWidget()->GetParentWidget() == panel.GetWidget( "Group0" ) );

        // the defaults of the schema
        ASSERT_TRUE( "dust" == panel.GetValue( "map" ) && "3" == panel.GetValue( "speed" ) && "0" == panel.GetValue( "handicap" ) );
        ASSERT_TRUE( "false" == panel.GetValue( "fog" ) && "1.0" == panel.GetValue( "gravity" ) && "hello" == panel.GetValue( "motd" ) );
        CORE::CValueList values;
        panel.GetValues( values );
        ASSERT_TRUE( 6 == values.GetKeyCount() && "3" == values.GetValueAlways( "speed" ).AsString() && "hello" == values.GetValueAlways( "motd" ).AsString() );
        ASSERT_TRUE( panel.IsSettingEditable( "map" ) && panel.IsSettingEditable( "motd" ) && !panel.IsProposalOnly( "speed" ) );

        // values given from outside
        CORE::CValueList given;
        given.SetAllowMultipleValues( false );
        given.Set( "map", "ice" );
        given.Set( "speed", "5" );
        given.Set( "fog", "true" );
        given.Set( "gravity", "1.5" );
        given.Set( "motd", "welcome" );
        panel.SetValues( given );
        ASSERT_TRUE( "ice" == panel.GetValue( "map" ) && "5" == panel.GetValue( "speed" ) && "true" == panel.GetValue( "fog" ) );
        ASSERT_TRUE( "1.5" == panel.GetValue( "gravity" ) && "welcome" == panel.GetValue( "motd" ) && "0" == panel.GetValue( "handicap" ) );
        ASSERT_TRUE( 1 == GAMEGUI::WidgetAs< GUI::CCombobox >( panel.GetControl( "map" ), "Combobox" )->GetSelectedIndex() && panel.GetControl( "fog" )->GetBoolProperty( "Checked" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: The changes of the player are notified and kept, without a lobby" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        GAMEGUI::CLobbySettingsPanelForm panel;
        panel.SetSchema( fixture.params.schema );
        ASSERT_TRUE( panel.Build( MakeHolder( context, "Holder" ) ) );
        CSettingRecorder recorder( &panel );

        // a spinner
        panel.GetControl( "speed" )->ChangeValueByUser( "Value", "4" );
        ASSERT_TRUE( 1 == recorder.count && "speed" == recorder.key && "4" == recorder.value && !recorder.proposed );
        ASSERT_TRUE( "4" == panel.GetValue( "speed" ) );
        ASSERT_TRUE( GAMEGUI::WidgetAs< GUI::CSpinner >( panel.GetControl( "speed" ), "Spinner" )->StepValue( 1 ) );
        ASSERT_TRUE( "5" == panel.GetValue( "speed" ) );

        // the same value again is no change
        UInt32 count = recorder.count;
        panel.GetControl( "speed" )->ChangeValueByUser( "Value", "5" );
        ASSERT_TRUE( count == recorder.count );

        // a float
        panel.GetControl( "gravity" )->ChangeValueByUser( "Value", "1.7" );
        ASSERT_TRUE( "gravity" == recorder.key && "1.7" == recorder.value && "1.7" == panel.GetValue( "gravity" ) );

        // a choice from the list of the combo box, picked with the mouse
        ASSERT_TRUE( PickItem( context, panel.GetControl( "map" ), 1 ) );
        ASSERT_TRUE( "map" == recorder.key && "ice" == recorder.value && "ice" == panel.GetValue( "map" ) );

        // a check box, clicked
        ClickWidget( context, panel.GetControl( "fog" ) );
        ASSERT_TRUE( "fog" == recorder.key && "true" == recorder.value && "true" == panel.GetValue( "fog" ) );

        // text: the change is made with Enter
        count = recorder.count;
        panel.GetControl( "motd" )->SetProperty( "Text", "typed" );
        ASSERT_TRUE( count == recorder.count );
        TypeInto( context, panel.GetControl( "motd" ), "Have fun", true );
        ASSERT_TRUE( "motd" == recorder.key && "Have fun" == recorder.value && "Have fun" == panel.GetValue( "motd" ) );

        CORE::CValueList values;
        panel.GetValues( values );
        ASSERT_TRUE( "5" == values.GetValueAlways( "speed" ).AsString() && "ice" == values.GetValueAlways( "map" ).AsString() && "true" == values.GetValueAlways( "fog" ).AsString() );

        // a schema of another game replaces the controls
        MULTIPLAY::CLobbySettingsSchema other;
        MULTIPLAY::CLobbySettingDef lives;
        lives.key = "lives";
        lives.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
        lives.defaultValue = "3";
        other.AddSetting( lives );
        panel.SetSchema( other );
        ASSERT_TRUE( 1 == panel.GetSettingCount() && GUCEF_NULL == panel.GetControl( "map" ) && GUCEF_NULL != panel.GetControl( "lives" ) );
        ASSERT_TRUE( "lives" == panel.GetLabelText( "lives" ) && "3" == panel.GetValue( "lives" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: In a lobby a control is enabled when the member may change it, and a vote setting says it is a proposal" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        GAMEGUI::CLobbySettingsPanelForm bossPanel;
        GAMEGUI::CLobbySettingsPanelForm playerPanel;
        GAMEGUI::CLobbySettingsPanelForm spectatorPanel;
        ASSERT_TRUE( bossPanel.Build( MakeHolder( context, "BossHolder" ) ) && playerPanel.Build( MakeHolder( context, "PlayerHolder", 1 ) ) && spectatorPanel.Build( MakeHolder( context, "SpectatorHolder", 2 ) ) );
        bossPanel.SetLobby( fixture.alice );
        playerPanel.SetLobby( fixture.bob );
        spectatorPanel.SetLobby( fixture.dave );
        ASSERT_TRUE( fixture.alice.GetPointerAlways() == bossPanel.GetLobby().GetPointerAlways() && 6 == bossPanel.GetSettingCount() );

        // the boss changes everything, nothing is a proposal
        const char* keys[] = { "map", "speed", "handicap", "fog", "gravity", "motd" };
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_TRUE( bossPanel.IsSettingEditable( keys[ i ] ) && !bossPanel.IsProposalOnly( keys[ i ] ) );
        ASSERT_TRUE( "Game speed" == bossPanel.GetLabelText( "speed" ) );

        // a player changes what is not the boss's alone; the vote setting is a proposal
        ASSERT_FALSE( playerPanel.IsSettingEditable( "map" ) || playerPanel.IsSettingEditable( "gravity" ) || playerPanel.IsSettingEditable( "motd" ) );
        ASSERT_TRUE( playerPanel.IsSettingEditable( "speed" ) && playerPanel.IsProposalOnly( "speed" ) && playerPanel.IsSettingEditable( "handicap" ) && playerPanel.IsSettingEditable( "fog" ) );
        ASSERT_TRUE( "Game speed (propose)" == playerPanel.GetLabelText( "speed" ) && "Map" == playerPanel.GetLabelText( "map" ) );
        ASSERT_TRUE( !playerPanel.GetControl( "map" )->GetProperty( "Tooltip" ).IsNULLOrEmpty() && !playerPanel.GetControl( "speed" )->GetProperty( "Tooltip" ).IsNULLOrEmpty() );

        // a spectator changes nothing
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_FALSE( spectatorPanel.IsSettingEditable( keys[ i ] ) );

        // the values are the lobby's
        ASSERT_TRUE( "dust" == playerPanel.GetValue( "map" ) && "3" == playerPanel.GetValue( "speed" ) && "hello" == spectatorPanel.GetValue( "motd" ) );

        // when the lobby is not open any more nobody changes anything
        ASSERT_TRUE( fixture.AllReady() );
        ASSERT_TRUE( fixture.alice->RequestStart() );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == fixture.alice->GetState().phase );
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_FALSE( bossPanel.IsSettingEditable( keys[ i ] ) || playerPanel.IsSettingEditable( keys[ i ] ) );
        PumpFor( 300 );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_IN_GAME == fixture.alice->GetState().phase );
        ASSERT_FALSE( bossPanel.IsSettingEditable( "map" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: Changes go to the lobby, a proposal becomes a vote that changes the setting for everyone when it passes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        GAMEGUI::CLobbySettingsPanelForm bossPanel;
        GAMEGUI::CLobbySettingsPanelForm playerPanel;
        ASSERT_TRUE( bossPanel.Build( MakeHolder( context, "BossHolder" ) ) && playerPanel.Build( MakeHolder( context, "PlayerHolder", 1 ) ) );
        bossPanel.SetLobby( fixture.alice );
        playerPanel.SetLobby( fixture.bob );
        CSettingRecorder bossRecorder( &bossPanel );
        CSettingRecorder playerRecorder( &playerPanel );

        // the boss changes the map: the lobby has it, the panel of a player shows it
        ASSERT_TRUE( PickItem( context, bossPanel.GetControl( "map" ), 1 ) );
        ASSERT_TRUE( "ice" == fixture.alice->GetState().GetSetting( "map" ) && 1 == bossRecorder.count && "map" == bossRecorder.key && !bossRecorder.proposed );
        ASSERT_TRUE( "ice" == playerPanel.GetValue( "map" ) && 1 == GAMEGUI::WidgetAs< GUI::CCombobox >( playerPanel.GetControl( "map" ), "Combobox" )->GetSelectedIndex() );

        // the boss sets the vote setting directly
        bossPanel.GetControl( "speed" )->ChangeValueByUser( "Value", "2" );
        ASSERT_TRUE( "2" == fixture.alice->GetState().GetSetting( "speed" ) && !bossRecorder.proposed && !fixture.alice->GetState().activeVote.IsActive() );
        ASSERT_TRUE( "2" == playerPanel.GetValue( "speed" ) );

        // a player proposes: a vote starts, the setting and the control stay as they were
        playerPanel.GetControl( "speed" )->ChangeValueByUser( "Value", "4" );
        ASSERT_TRUE( 1 == playerRecorder.count && "speed" == playerRecorder.key && "4" == playerRecorder.value && playerRecorder.proposed );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && "speed" == fixture.alice->GetState().activeVote.key && "4" == fixture.alice->GetState().activeVote.value );
        ASSERT_TRUE( "2" == fixture.alice->GetState().GetSetting( "speed" ) && "2" == playerPanel.GetValue( "speed" ) && "2" == bossPanel.GetValue( "speed" ) );

        // the other player agrees: it passes and every panel shows it
        ASSERT_TRUE( fixture.carol->Vote( fixture.alice->GetState().activeVote.id, true ) );
        ASSERT_TRUE( !fixture.alice->GetState().activeVote.IsActive() && "4" == fixture.alice->GetState().GetSetting( "speed" ) );
        ASSERT_TRUE( "4" == playerPanel.GetValue( "speed" ) && "4" == bossPanel.GetValue( "speed" ) );

        // a flag anyone may set
        ClickWidget( context, playerPanel.GetControl( "fog" ) );
        ASSERT_TRUE( "true" == fixture.alice->GetState().GetSetting( "fog" ) && !playerRecorder.proposed && "fog" == playerRecorder.key );
        ASSERT_TRUE( "true" == bossPanel.GetValue( "fog" ) && bossPanel.GetControl( "fog" )->GetBoolProperty( "Checked" ) );

        // a value that belongs to the member is the member's alone
        playerPanel.GetControl( "handicap" )->ChangeValueByUser( "Value", "20" );
        ASSERT_TRUE( "20" == fixture.alice->GetState().GetSetting( "bob-id/handicap" ) && "handicap" == playerRecorder.key );
        ASSERT_TRUE( "20" == playerPanel.GetValue( "handicap" ) && "0" == bossPanel.GetValue( "handicap" ) );
        bossPanel.GetControl( "handicap" )->ChangeValueByUser( "Value", "5" );
        ASSERT_TRUE( "5" == fixture.alice->GetState().GetSetting( "alice-id/handicap" ) && "20" == playerPanel.GetValue( "handicap" ) );

        // the text of the boss, changed with Enter
        TypeInto( context, bossPanel.GetControl( "motd" ), "Welcome all", true );
        ASSERT_TRUE( "Welcome all" == fixture.alice->GetState().GetSetting( "motd" ) && "Welcome all" == playerPanel.GetValue( "motd" ) );

        // a change the lobby refuses is not kept
        fixture.alice->SetSetting( "map", "dust" );
        ASSERT_TRUE( "dust" == bossPanel.GetValue( "map" ) && "dust" == playerPanel.GetValue( "map" ) );

        // taking the lobby away leaves a plain form
        playerPanel.SetLobby( MULTIPLAY::CILobbyPtr() );
        ASSERT_TRUE( playerPanel.GetLobby().IsNULL() && 6 == playerPanel.GetSettingCount() && playerPanel.IsSettingEditable( "map" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: A unit is shown after the control, and a setting that depends on another is enabled while that one has the value, without a lobby" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        MULTIPLAY::CLobbySettingsSchema schema;
        MULTIPLAY::CLobbySettingDef timer;
        timer.key = "timer";
        timer.label = "Turn timer";
        timer.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
        timer.defaultValue = "true";
        schema.AddSetting( timer );
        MULTIPLAY::CLobbySettingDef seconds;
        seconds.key = "seconds";
        seconds.label = "Seconds per turn";
        seconds.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
        seconds.hasRange = true;
        seconds.minValue = 15;
        seconds.maxValue = 3600;
        seconds.step = 5;
        seconds.defaultValue = "180";
        seconds.unit = "s";
        seconds.enabledByKey = "timer";
        seconds.enabledByValue = "true";
        schema.AddSetting( seconds );
        MULTIPLAY::CLobbySettingDef mode;
        mode.key = "mode";
        mode.label = "Mode";
        mode.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
        mode.options.push_back( "plain" );
        mode.options.push_back( "bonus" );
        mode.defaultValue = "plain";
        mode.unit = "x";
        schema.AddSetting( mode );
        MULTIPLAY::CLobbySettingDef bonus;
        bonus.key = "bonus";
        bonus.label = "Bonus";
        bonus.type = MULTIPLAY::CLobbySettingDef::TYPE_STRING;
        bonus.defaultValue = "gold";
        bonus.unit = "pts";
        bonus.enabledByKey = "mode";
        bonus.enabledByValue = "bonus";
        schema.AddSetting( bonus );
        MULTIPLAY::CLobbySettingDef stray;
        stray.key = "stray";
        stray.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
        stray.defaultValue = "1";
        stray.enabledByKey = "nosuch";
        stray.enabledByValue = "true";
        schema.AddSetting( stray );

        GAMEGUI::CLobbySettingsPanelForm panel;
        panel.SetSchema( schema );
        ASSERT_TRUE( panel.Build( MakeHolder( context, "Holder" ) ) );
        CSettingRecorder recorder( &panel );

        // units are labels after the controls of settings that have one, in the room left by the control
        ASSERT_TRUE( "s" == panel.GetUnitText( "seconds" ) && "x" == panel.GetUnitText( "mode" ) && "pts" == panel.GetUnitText( "bonus" ) );
        ASSERT_TRUE( panel.GetUnitText( "timer" ).IsNULLOrEmpty() && panel.GetUnitText( "stray" ).IsNULLOrEmpty() && panel.GetUnitText( "nosuch" ).IsNULLOrEmpty() );
        ASSERT_TRUE( GUCEF_NULL != panel.GetWidget( "Unit1" ) && GUCEF_NULL == panel.GetWidget( "Unit0" ) && GUCEF_NULL == panel.GetWidget( "Unit4" ) );
        context->FlushChanges();
        GUI::CWidget* secondsControl = panel.GetControl( "seconds" );
        GUI::CWidget* secondsUnit = panel.GetWidget( "Unit1" );
        ASSERT_TRUE( secondsControl->GetRect().left + secondsControl->GetRect().width <= secondsUnit->GetRect().left + 1 );
        ASSERT_TRUE( secondsUnit->GetRect().width > 0 && secondsUnit->GetRect().left + secondsUnit->GetRect().width <= secondsControl->GetParentWidget()->GetRect().left + secondsControl->GetParentWidget()->GetRect().width + 1 );
        ASSERT_TRUE( secondsControl->GetRect().width > 0 && secondsControl->GetRect().top == secondsUnit->GetRect().top );
        // a setting without a unit keeps the whole width
        GUI::CWidget* strayControl = panel.GetControl( "stray" );
        ASSERT_TRUE( strayControl->GetRect().width > secondsControl->GetRect().width );

        // the timer is on: seconds can be edited; the gate on a key the schema lacks is no gate
        ASSERT_TRUE( panel.IsGateOpen( "seconds" ) && panel.IsSettingEditable( "seconds" ) && panel.IsGateOpen( "timer" ) );
        ASSERT_TRUE( panel.IsGateOpen( "stray" ) && panel.IsSettingEditable( "stray" ) );
        ASSERT_TRUE( panel.GetControl( "seconds" )->GetProperty( "Tooltip" ).IsNULLOrEmpty() );

        // changing the controlling setting closes the gate, at once; the value stays
        panel.GetControl( "seconds" )->ChangeValueByUser( "Value", "90" );
        ASSERT_TRUE( "seconds" == recorder.key && "90" == recorder.value && "90" == panel.GetValue( "seconds" ) );
        ClickWidget( context, panel.GetControl( "timer" ) );
        ASSERT_TRUE( "timer" == recorder.key && "false" == recorder.value );
        ASSERT_FALSE( panel.IsGateOpen( "seconds" ) || panel.IsSettingEditable( "seconds" ) );
        ASSERT_TRUE( panel.IsSettingEditable( "timer" ) && panel.IsSettingEditable( "mode" ) );
        ASSERT_TRUE( "90" == panel.GetValue( "seconds" ) && "s" == panel.GetUnitText( "seconds" ) );
        ASSERT_TRUE( 0 <= panel.GetControl( "seconds" )->GetProperty( "Tooltip" ).HasSubstr( "Turn timer" ) );

        // a change of a gated setting does not get through
        UInt32 count = recorder.count;
        panel.GetControl( "seconds" )->ChangeValueByUser( "Value", "120" );
        ASSERT_TRUE( count == recorder.count && "90" == panel.GetValue( "seconds" ) );
        CORE::CValueList values;
        panel.GetValues( values );
        ASSERT_TRUE( "90" == values.GetValueAlways( "seconds" ).AsString() && "false" == values.GetValueAlways( "timer" ).AsString() );

        // and open again
        ClickWidget( context, panel.GetControl( "timer" ) );
        ASSERT_TRUE( panel.IsGateOpen( "seconds" ) && panel.IsSettingEditable( "seconds" ) && "90" == panel.GetValue( "seconds" ) );
        ASSERT_TRUE( panel.GetControl( "seconds" )->GetProperty( "Tooltip" ).IsNULLOrEmpty() );
        panel.GetControl( "seconds" )->ChangeValueByUser( "Value", "120" );
        ASSERT_TRUE( "seconds" == recorder.key && "120" == recorder.value );

        // the value that opens it can be a choice: bonus is editable in the mode bonus only
        ASSERT_FALSE( panel.IsSettingEditable( "bonus" ) );
        ASSERT_TRUE( PickItem( context, panel.GetControl( "mode" ), 1 ) );
        ASSERT_TRUE( "mode" == recorder.key && "bonus" == recorder.value );
        ASSERT_TRUE( panel.IsSettingEditable( "bonus" ) && "gold" == panel.GetValue( "bonus" ) );
        ASSERT_TRUE( PickItem( context, panel.GetControl( "mode" ), 0 ) );
        ASSERT_FALSE( panel.IsSettingEditable( "bonus" ) );
        ASSERT_TRUE( "gold" == panel.GetValue( "bonus" ) );

        // values given from the outside count as well
        CORE::CValueList given;
        given.SetAllowMultipleValues( false );
        given.Set( "timer", "false" );
        given.Set( "mode", "bonus" );
        panel.SetValues( given );
        ASSERT_FALSE( panel.IsSettingEditable( "seconds" ) );
        ASSERT_TRUE( panel.IsSettingEditable( "bonus" ) );
        given.Set( "timer", "true" );
        panel.SetValues( given );
        ASSERT_TRUE( panel.IsSettingEditable( "seconds" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SettingsPanel: In a lobby a gated setting follows the setting it depends on, also when the change comes from another member or from a vote" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        MULTIPLAY::CLobbySettingDef timer;
        timer.key = "timer";
        timer.label = "Turn timer";
        timer.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
        timer.defaultValue = "true";
        timer.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
        fixture.params.schema.AddSetting( timer );
        MULTIPLAY::CLobbySettingDef seconds;
        seconds.key = "seconds";
        seconds.label = "Seconds per turn";
        seconds.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
        seconds.hasRange = true;
        seconds.minValue = 15;
        seconds.maxValue = 3600;
        seconds.step = 5;
        seconds.defaultValue = "180";
        seconds.unit = "s";
        seconds.enabledByKey = "timer";
        seconds.enabledByValue = "true";
        seconds.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_ANY;
        fixture.params.schema.AddSetting( seconds );
        ASSERT_TRUE( fixture.Setup() );

        GAMEGUI::CLobbySettingsPanelForm bossPanel;
        GAMEGUI::CLobbySettingsPanelForm playerPanel;
        ASSERT_TRUE( bossPanel.Build( MakeHolder( context, "BossHolder" ) ) && playerPanel.Build( MakeHolder( context, "PlayerHolder", 1 ) ) );
        bossPanel.SetLobby( fixture.alice );
        playerPanel.SetLobby( fixture.bob );
        ASSERT_TRUE( 8 == bossPanel.GetSettingCount() && "s" == playerPanel.GetUnitText( "seconds" ) );
        ASSERT_TRUE( bossPanel.IsSettingEditable( "seconds" ) && playerPanel.IsSettingEditable( "seconds" ) );

        // a player changes the seconds: the lobby has it
        playerPanel.GetControl( "seconds" )->ChangeValueByUser( "Value", "90" );
        ASSERT_TRUE( "90" == fixture.alice->GetState().GetSetting( "seconds" ) && "90" == bossPanel.GetValue( "seconds" ) );

        // the boss turns the timer off: the panel of the player, which did nothing, sees the setting go away
        ClickWidget( context, bossPanel.GetControl( "timer" ) );
        ASSERT_TRUE( "false" == fixture.alice->GetState().GetSetting( "timer" ) );
        ASSERT_FALSE( bossPanel.IsSettingEditable( "seconds" ) || playerPanel.IsSettingEditable( "seconds" ) );
        ASSERT_FALSE( bossPanel.IsGateOpen( "seconds" ) || playerPanel.IsGateOpen( "seconds" ) );
        ASSERT_TRUE( "90" == fixture.alice->GetState().GetSetting( "seconds" ) && "90" == playerPanel.GetValue( "seconds" ) && "90" == bossPanel.GetValue( "seconds" ) );

        // the lobby still takes a value for it (the server keeps checking values), a panel that is asked to change it does not
        ASSERT_TRUE( fixture.alice->SetSetting( "seconds", "60" ) );
        ASSERT_TRUE( "60" == fixture.alice->GetState().GetSetting( "seconds" ) && "60" == playerPanel.GetValue( "seconds" ) );
        ASSERT_FALSE( playerPanel.IsSettingEditable( "seconds" ) );

        // the timer back on, and the player proposes it off: nothing changes until the vote passes
        ClickWidget( context, bossPanel.GetControl( "timer" ) );
        ASSERT_TRUE( "true" == fixture.alice->GetState().GetSetting( "timer" ) );
        ASSERT_TRUE( bossPanel.IsSettingEditable( "seconds" ) && playerPanel.IsSettingEditable( "seconds" ) );
        ClickWidget( context, playerPanel.GetControl( "timer" ) );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && "timer" == fixture.alice->GetState().activeVote.key );
        ASSERT_TRUE( bossPanel.IsSettingEditable( "seconds" ) && playerPanel.IsSettingEditable( "seconds" ) );
        ASSERT_TRUE( fixture.carol->Vote( fixture.alice->GetState().activeVote.id, true ) );
        ASSERT_TRUE( "false" == fixture.alice->GetState().GetSetting( "timer" ) );
        ASSERT_FALSE( bossPanel.IsSettingEditable( "seconds" ) || playerPanel.IsSettingEditable( "seconds" ) );
        ASSERT_TRUE( "60" == bossPanel.GetValue( "seconds" ) );

        // without the lobby the panel is a plain form again, with the gate evaluated on its own values
        playerPanel.SetLobby( MULTIPLAY::CILobbyPtr() );
        ASSERT_TRUE( playerPanel.IsGateOpen( "seconds" ) && playerPanel.IsSettingEditable( "seconds" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
