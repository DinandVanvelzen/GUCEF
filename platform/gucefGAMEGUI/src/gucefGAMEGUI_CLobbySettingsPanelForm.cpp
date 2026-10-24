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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CSPINNER_H
#include "gucefGUI_CSpinner.h"
#define GUCEF_GUI_CSPINNER_H
#endif /* GUCEF_GUI_CSPINNER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"

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

const CORE::CEvent CLobbySettingsPanelForm::SettingChangedEvent = "GUCEF::GAMEGUI::CLobbySettingsPanelForm::SettingChangedEvent";
const CORE::CString CLobbySettingsPanelForm::ProposeSuffix = " (propose)";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 RowHeight = 32;

/** The digits after the point a spinner needs to show multiples of the step */
UInt32
CountDecimals( Float64 step )
{
    if ( step <= 0.0 )
        return 1;
    UInt32 decimals = 0;
    Float64 scaled = step;
    while ( decimals < 4 && fabs( scaled - floor( scaled + 0.5 ) ) > 1e-6 )
    {
        scaled *= 10.0;
        ++decimals;
    }
    return decimals;
}

/** Whether two values of a setting are the same value, "4" and "4.0" are for a number */
bool
SameValue( const MULTIPLAY::CLobbySettingDef& def, const CORE::CString& a, const CORE::CString& b )
{
    switch ( def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_BOOL:
            return CORE::StringToBool( a, false ) == CORE::StringToBool( b, false );
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
            return fabs( CORE::StringToDouble( a, 0.0 ) - CORE::StringToDouble( b, 0.0 ) ) < 1e-9;
        default:
            return a == b;
    }
}

const CORE::CString&
LabelOf( const MULTIPLAY::CLobbySettingDef& def )
{
    return def.label.IsNULLOrEmpty() ? def.key : def.label;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSettingChangedEventData::CSettingChangedEventData( void )
    : CORE::CICloneable()
    , key()
    , value()
    , proposed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSettingChangedEventData::CSettingChangedEventData( const CORE::CString& key_, const CORE::CString& value_, bool proposed_ )
    : CORE::CICloneable()
    , key( key_ )
    , value( value_ )
    , proposed( proposed_ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSettingChangedEventData::~CSettingChangedEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CSettingChangedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CSettingChangedEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::RegisterEvents( void )
{GUCEF_TRACE;

    SettingChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm::CLobbySettingsPanelForm( void )
    : GUI::CFormEx()
    , m_placeholder( GUCEF_NULL )
    , m_schema()
    , m_values()
    , m_lobby()
    , m_controls()
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm::~CLobbySettingsPanelForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::AddControlNodes( CORE::CDataNode& parent, UInt32 index )
{GUCEF_TRACE;

    const TControl& control = m_controls[ index ];
    const MULTIPLAY::CLobbySettingDef& def = control.def;

    CORE::CDataNode* row = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "Row" + CORE::UInt32ToString( index ), "0,0 0,0", "1,0 0," + CORE::Int32ToString( RowHeight ) );

    if ( MULTIPLAY::CLobbySettingDef::TYPE_BOOL == def.type )
    {
        CORE::CDataNode* box = GUI::CFormLayout::AddWidgetNode( *row, "Checkbox", control.widgetName, "0,0 0,0", "1,0 1,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *box, "Text", LabelOf( def ) );
        return;
    }

    CORE::CDataNode* label = GUI::CFormLayout::AddWidgetNode( *row, "Label", control.labelName, "0,0 0,0", "0.45,-8 1,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *label, "Text", LabelOf( def ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *label, "VTextAlign", "Center" );

    switch ( def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            CORE::CDataNode* spinner = GUI::CFormLayout::AddWidgetNode( *row, "Spinner", control.widgetName, "0.45,0 0,0", "0.55,0 1,0" );
            bool isInt = MULTIPLAY::CLobbySettingDef::TYPE_INT == def.type;
            Float64 step = def.step > 0.0 ? def.step : ( isInt ? 1.0 : 0.1 );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, "Min", CORE::DoubleToString( def.hasRange ? def.minValue : -1000000.0 ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, "Max", CORE::DoubleToString( def.hasRange ? def.maxValue : 1000000.0 ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, "Step", CORE::DoubleToString( step ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, "Decimals", CORE::UInt32ToString( isInt ? 0 : CountDecimals( step ) ) );
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            CORE::CDataNode* combo = GUI::CFormLayout::AddWidgetNode( *row, "Combobox", control.widgetName, "0.45,0 0,0", "0.55,0 1,0" );
            GUI::CFormLayout::SetWidgetNodeProperty( *combo, "Items", JoinLines( def.options ) );
            break;
        }
        default:
        {
            GUI::CFormLayout::AddWidgetNode( *row, "Editbox", control.widgetName, "0.45,0 0,0", "0.55,0 1,0" );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* panel = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "LobbySettingsPanel", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *panel, "VBox", "Settings", "0,0 0,0", "1,0 1,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, "Spacing", "6" );

    // consecutive settings of one group share a group box
    CORE::CString currentGroup;
    CORE::CDataNode* groupNode = GUCEF_NULL;
    UInt32 groupCount = 0;
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const CORE::CString& group = m_controls[ i ].def.group;
        if ( group.IsNULLOrEmpty() )
        {
            groupNode = GUCEF_NULL;
            currentGroup.Clear();
            AddControlNodes( *list, i );
            continue;
        }
        if ( GUCEF_NULL == groupNode || group != currentGroup )
        {
            groupNode = GUI::CFormLayout::AddWidgetNode( *list, "GroupBox", "Group" + CORE::UInt32ToString( groupCount++ ), "0,0 0,0", "1,0 0,0" );
            GUI::CFormLayout::SetWidgetNodeProperty( *groupNode, "Caption", group );
            currentGroup = group;
        }
        AddControlNodes( *groupNode, i );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::Rebuild( void )
{GUCEF_TRACE;

    // one control per setting, named by place: the keys of settings can be anything
    m_controls.clear();
    for ( UInt32 i=0; i<(UInt32)m_schema.settings.size(); ++i )
    {
        TControl control;
        control.def = m_schema.settings[ i ];
        control.widgetName = "Setting" + CORE::UInt32ToString( i );
        if ( MULTIPLAY::CLobbySettingDef::TYPE_BOOL != control.def.type )
            control.labelName = "Label" + CORE::UInt32ToString( i );
        m_controls.push_back( control );
    }

    if ( GUCEF_NULL == m_placeholder )
        return true;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::Build( GUI::CWidget* placeholder )
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
CLobbySettingsPanelForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        GUI::CWidget* widget = GetWidget( m_controls[ i ].widgetName );
        if ( GUCEF_NULL == widget )
            continue;
        if ( MULTIPLAY::CLobbySettingDef::TYPE_STRING == m_controls[ i ].def.type )
        {
            SubscribeTo( widget, GUI::CEditbox::TextEnteredEvent );
            SubscribeTo( widget, GUI::CWidget::KeyboardFocusLostEvent );
        }
        else
            SubscribeTo( widget, GUI::CWidget::ValueChangedEvent );
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetSchema( const MULTIPLAY::CLobbySettingsSchema& schema )
{GUCEF_TRACE;

    m_schema = schema;
    m_values.clear();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetValues( const CORE::CValueList& values )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const CORE::CString& key = m_controls[ i ].def.key;
        if ( values.HasKey( key ) )
            m_values[ key ] = values.GetValueAlways( key ).AsString();
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::GetValues( CORE::CValueList& values )
{GUCEF_TRACE;

    values.SetAllowMultipleValues( false );
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        values.Set( m_controls[ i ].def.key, GetValue( m_controls[ i ].def.key ) );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetLobby( MULTIPLAY::CILobbyPtr lobby )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
        UnsubscribeFrom( m_lobby.GetPointerAlways() );

    m_lobby = lobby;
    if ( !m_lobby.IsNULL() )
    {
        SubscribeTo( m_lobby.GetPointerAlways() );
        m_schema = m_lobby->GetSchema();
        m_values.clear();
    }
    Rebuild();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CILobbyPtr&
CLobbySettingsPanelForm::GetLobby( void ) const
{GUCEF_TRACE;

    return m_lobby;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbySettingsSchema&
CLobbySettingsPanelForm::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbySettingsPanelForm::GetSettingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_controls.size();
}

/*-------------------------------------------------------------------------*/

Int32
CLobbySettingsPanelForm::FindControlIndex( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        if ( m_controls[ i ].def.key == key )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CLobbySettingsPanelForm::FindControlIndexOfWidget( CORE::CNotifier* widget )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        if ( widget == GetWidget( m_controls[ i ].widgetName ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CLobbySettingsPanelForm::GetControl( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    return index < 0 ? GUCEF_NULL : GetWidget( m_controls[ index ].widgetName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetLabelText( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return CORE::CString::Empty;
    const TControl& control = m_controls[ index ];
    GUI::CWidget* label = GetWidget( control.labelName.IsNULLOrEmpty() ? control.widgetName : control.labelName );
    return GUCEF_NULL != label ? label->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetValueKey( const TControl& control ) const
{GUCEF_TRACE;

    // a value that belongs to the member is kept under the id of the member
    if ( !m_lobby.IsNULL() && MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_OWNER == control.def.changePolicy )
        return m_lobby->GetLocalMemberId() + "/" + control.def.key;
    return control.def.key;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetKnownValue( const TControl& control ) const
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        CORE::CString valueKey = GetValueKey( control );
        if ( state.settings.HasKey( valueKey ) )
            return state.GetSetting( valueKey );
        return control.def.defaultValue;
    }
    TValueMap::const_iterator i = m_values.find( control.def.key );
    if ( i != m_values.end() )
        return (*i).second;
    return control.def.defaultValue;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::ReadControl( const TControl& control )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( control.widgetName );
    if ( GUCEF_NULL == widget )
        return GetKnownValue( control );

    switch ( control.def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_BOOL:
        {
            return widget->GetBoolProperty( "Checked" ) ? "true" : "false";
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            GUI::CSpinner* spinner = dynamic_cast< GUI::CSpinner* >( widget );
            return GUCEF_NULL != spinner ? spinner->GetValueText() : widget->GetProperty( "Value" );
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            GUI::CCombobox* combo = dynamic_cast< GUI::CCombobox* >( widget );
            if ( GUCEF_NULL == combo )
                return CORE::CString::Empty;
            Int32 selected = combo->GetSelectedIndex();
            if ( selected >= 0 && selected < (Int32) control.def.options.size() )
                return control.def.options[ selected ];
            return combo->GetText();
        }
        default:
        {
            return widget->GetProperty( "Text" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::ShowControl( const TControl& control, const CORE::CString& value )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( control.widgetName );
    if ( GUCEF_NULL == widget )
        return;

    switch ( control.def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_BOOL:
        {
            widget->SetProperty( "Checked", CORE::StringToBool( value, false ) ? "true" : "false" );
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            widget->SetProperty( "Value", value );
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            GUI::CCombobox* combo = dynamic_cast< GUI::CCombobox* >( widget );
            if ( GUCEF_NULL == combo )
                break;
            Int32 found = -1;
            for ( UInt32 i=0; i<(UInt32)control.def.options.size(); ++i )
                if ( control.def.options[ i ] == value )
                    found = (Int32) i;
            if ( found >= 0 )
                combo->SetSelectedIndex( found );
            else
            {
                combo->SetSelectedIndex( -1 );
                combo->SetText( value );
            }
            break;
        }
        default:
        {
            widget->SetProperty( "Text", value );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetValue( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return CORE::CString::Empty;
    return ReadControl( m_controls[ index ] );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsProposal( const TControl& control ) const
{GUCEF_TRACE;

    return !m_lobby.IsNULL() &&
           MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE == control.def.changePolicy &&
           !m_lobby->IsBoss();
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::CanChange( const TControl& control ) const
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return true;

    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    if ( MULTIPLAY::LOBBYPHASE_OPEN != state.phase )
        return false;
    const MULTIPLAY::CLobbyMember* member = state.FindMember( m_lobby->GetLocalMemberId() );
    if ( GUCEF_NULL == member || member->IsSpectator() )
        return false;
    if ( MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS == control.def.changePolicy )
        return m_lobby->IsBoss();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsSettingEditable( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return false;
    GUI::CWidget* widget = GetWidget( m_controls[ index ].widgetName );
    return GUCEF_NULL != widget && widget->GetBoolProperty( "Enabled" );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsProposalOnly( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    return index >= 0 && IsProposal( m_controls[ index ] );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::Refresh( void )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    m_updating = true;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const TControl& control = m_controls[ i ];
        GUI::CWidget* widget = GetWidget( control.widgetName );
        if ( GUCEF_NULL == widget )
            continue;

        // text being typed is not taken away from the player
        bool typing = MULTIPLAY::CLobbySettingDef::TYPE_STRING == control.def.type && widget->HasFocus();
        if ( !typing )
            ShowControl( control, GetKnownValue( control ) );

        bool canChange = CanChange( control );
        bool proposal = canChange && IsProposal( control );
        widget->SetEnabled( canChange );

        CORE::CString label = LabelOf( control.def );
        if ( proposal )
            label += ProposeSuffix;
        GUI::CWidget* labelWidget = GetWidget( control.labelName.IsNULLOrEmpty() ? control.widgetName : control.labelName );
        if ( GUCEF_NULL != labelWidget )
            labelWidget->SetProperty( "Text", label );

        CORE::CString tip;
        if ( proposal )
            tip = "A change is a proposal that the players vote on";
        else
        if ( !canChange && !m_lobby.IsNULL() && MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS == control.def.changePolicy )
            tip = "Only the boss changes this";
        widget->SetProperty( "Tooltip", tip );
    }

    m_updating = false;
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::Commit( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_controls.size() )
        return;
    const TControl& control = m_controls[ index ];

    CORE::CString value = ReadControl( control );
    CORE::CString known = GetKnownValue( control );
    if ( SameValue( control.def, value, known ) )
        return;

    if ( m_lobby.IsNULL() )
    {
        m_values[ control.def.key ] = value;
        CSettingChangedEventData eventData( control.def.key, value, false );
        NotifyObservers( SettingChangedEvent, &eventData );
        return;
    }

    if ( !CanChange( control ) )
    {
        Refresh();
        return;
    }

    bool proposal = IsProposal( control );
    CSettingChangedEventData eventData( control.def.key, value, proposal );
    m_lobby->SetSetting( GetValueKey( control ), value );
    if ( proposal )
        Refresh();          // a proposal changes nothing until the players agree
    NotifyObservers( SettingChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::OnNotify( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() && notifier == m_lobby.GetPointerAlways() )
    {
        if ( MULTIPLAY::CILobby::LobbyStateChangedEvent == eventId )
            Refresh();
        return;
    }

    if ( GUI::CWidget::ValueChangedEvent == eventId || GUI::CEditbox::TextEnteredEvent == eventId || GUI::CWidget::KeyboardFocusLostEvent == eventId )
    {
        if ( m_updating )
            return;
        Int32 index = FindControlIndexOfWidget( notifier );
        if ( index >= 0 )
            Commit( (UInt32) index );
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbySettingsPanelForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CLobbySettingsPanelForm";
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
