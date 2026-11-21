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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#include "gucefGUI_CInputSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CInputSettingsForm::PageTitle = "Input";

/*-------------------------------------------------------------------------*/

CInputSettingsForm::CInputSettingsForm( void )
    : CSettingsPageForm()
    , m_shortcuts( GUCEF_NULL )
    , m_actions()
    , m_rebindingActionId()
    , m_status()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputSettingsForm::~CInputSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::SetShortcutMap( CKeyboardShortcutMap* shortcuts )
{GUCEF_TRACE;

    m_shortcuts = shortcuts;
}

/*-------------------------------------------------------------------------*/

CKeyboardShortcutMap*
CInputSettingsForm::GetShortcutMap( void ) const
{GUCEF_TRACE;

    return m_shortcuts;
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::AddAction( const CString& actionId, const CString& title, INPUT::KeyCode defaultKey, UInt32 defaultModifiers )
{GUCEF_TRACE;

    if ( actionId.IsNULLOrEmpty() || GUCEF_NULL != FindAction( actionId ) )
        return false;
    TAction action;
    action.actionId = actionId;
    action.title = title.IsNULLOrEmpty() ? actionId : title;
    action.defaultKey = defaultKey;
    action.defaultModifiers = CKeyboardShortcutMap::NormalizeModifiers( defaultModifiers );
    action.key = defaultKey;
    action.modifiers = action.defaultModifiers;
    m_actions.push_back( action );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CInputSettingsForm::GetActionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_actions.size();
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::MakeLine( const TAction& action ) const
{GUCEF_TRACE;

    const CString chord = INPUT::KEYCODE_UNASSIGNED == action.key ? CString( "(none)" ) : CKeyboardShortcutMap::DescribeChord( action.key, action.modifiers );
    return action.title + "    " + chord;
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::GetActionLine( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_actions.size() ? MakeLine( m_actions[ index ] ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CInputSettingsForm::TAction*
CInputSettingsForm::FindAction( const CString& actionId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        if ( m_actions[ i ].actionId == actionId )
            return &m_actions[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CInputSettingsForm::TAction*
CInputSettingsForm::FindAction( const CString& actionId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        if ( m_actions[ i ].actionId == actionId )
            return &m_actions[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::FindOwner( const CString& exceptActionId, INPUT::KeyCode key, UInt32 modifiers ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        if ( m_actions[ i ].actionId != exceptActionId && m_actions[ i ].key == key && m_actions[ i ].modifiers == modifiers )
            return m_actions[ i ].title;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::GetPendingChord( const CString& actionId, INPUT::KeyCode& key, UInt32& modifiers ) const
{GUCEF_TRACE;

    const TAction* action = FindAction( actionId );
    if ( GUCEF_NULL == action )
        return false;
    key = action->key;
    modifiers = action->modifiers;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::GetSelectedActionId( void )
{GUCEF_TRACE;

    CListbox* list = GetWidgetAs< CListbox >( "Actions" );
    const Int32 selected = GUCEF_NULL != list ? list->GetSelectedIndex() : -1;
    return selected >= 0 && selected < (Int32) m_actions.size() ? m_actions[ (UInt32) selected ].actionId : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::SelectAction( const CString& actionId )
{GUCEF_TRACE;

    CListbox* list = GetWidgetAs< CListbox >( "Actions" );
    if ( GUCEF_NULL == list )
        return false;
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        if ( m_actions[ i ].actionId == actionId )
            return list->SetSelectedIndex( (Int32) i );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::ShowStatus( const CString& text )
{GUCEF_TRACE;

    m_status = text;
    SetWidgetProperty( "Status", EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::GetStatusText( void )
{GUCEF_TRACE;

    return m_status;
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::ShowActions( void )
{GUCEF_TRACE;

    CListbox* list = GetWidgetAs< CListbox >( "Actions" );
    if ( GUCEF_NULL == list )
        return;
    const Int32 selected = list->GetSelectedIndex();
    CListbox::TStringVector lines;
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        lines.push_back( MakeLine( m_actions[ i ] ) );
    list->SetItems( lines );
    if ( selected >= 0 && selected < (Int32) lines.size() )
        list->SetSelectedIndex( selected );
    else
    if ( !lines.empty() )
        list->SetSelectedIndex( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::BeginRebind( void )
{GUCEF_TRACE;

    const CString actionId = GetSelectedActionId();
    const TAction* action = FindAction( actionId );
    if ( GUCEF_NULL == action )
        return false;
    m_rebindingActionId = actionId;
    // the keys that follow come through this page: its button holds the focus
    CWidget* change = GetWidget( "Change" );
    if ( GUCEF_NULL != change && !change->HasFocus() )
        change->SetFocus();
    ShowStatus( "Press the new key for " + action->title + ", with Ctrl, Alt or Shift if you want. Escape cancels." );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::IsRebinding( void ) const
{GUCEF_TRACE;

    return !m_rebindingActionId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::Rebind( const CString& actionId, INPUT::KeyCode key, UInt32 modifiers )
{GUCEF_TRACE;

    TAction* action = FindAction( actionId );
    if ( GUCEF_NULL == action || INPUT::KEYCODE_UNASSIGNED == key )
        return false;
    const UInt32 normalized = CKeyboardShortcutMap::NormalizeModifiers( modifiers );
    const CString owner = FindOwner( actionId, key, normalized );
    const CString chord = CKeyboardShortcutMap::DescribeChord( key, normalized );
    if ( !owner.IsNULLOrEmpty() )
    {
        ShowStatus( chord + " is the key of " + owner + " already: choose another key, or change that one first." );
        return false;
    }
    action->key = key;
    action->modifiers = normalized;
    ShowActions();
    SetHasChanges( true );
    NotifyObservers( PageChangedEvent );
    ShowStatus( action->title + " is on " + chord + ". Apply keeps it." );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::ResetAction( const CString& actionId )
{GUCEF_TRACE;

    const TAction* action = FindAction( actionId );
    return GUCEF_NULL != action && Rebind( actionId, action->defaultKey, action->defaultModifiers );
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::ResetAll( void )
{GUCEF_TRACE;

    // the shipped chords are free of conflicts among themselves: all of them go back together
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
    {
        m_actions[ i ].key = m_actions[ i ].defaultKey;
        m_actions[ i ].modifiers = m_actions[ i ].defaultModifiers;
    }
    ShowActions();
    SetHasChanges( true );
    NotifyObservers( PageChangedEvent );
    ShowStatus( "Every key is back on the one that came with the program. Apply keeps them." );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CInputSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return PageTitle;
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = CFormLayout::AddWidgetNode( layout, "Panel", "InputSettings", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* hint = CFormLayout::AddWidgetNode( *page, "Label", "Hint", "0,24 0,10", "1,-48 0,48" );
    CFormLayout::SetWidgetNodeProperty( *hint, EWidgetProperty_Text, "Select an action, press Change and then the key it gets" );
    CFormLayout::AddWidgetNode( *page, "Listbox", "Actions", "0,24 0,64", "1,-48 1,-188" );
    CORE::CDataNode* change = CFormLayout::AddWidgetNode( *page, "Button", "Change", "0,24 1,-116", "0.22,-12 0,48" );
    CFormLayout::SetWidgetNodeProperty( *change, EWidgetProperty_Text, "Change" );
    CORE::CDataNode* reset = CFormLayout::AddWidgetNode( *page, "Button", "Reset", "0.22,24 1,-116", "0.22,-12 0,48" );
    CFormLayout::SetWidgetNodeProperty( *reset, EWidgetProperty_Text, "Reset" );
    CORE::CDataNode* resetAll = CFormLayout::AddWidgetNode( *page, "Button", "ResetAll", "0.44,24 1,-116", "0.26,-12 0,48" );
    CFormLayout::SetWidgetNodeProperty( *resetAll, EWidgetProperty_Text, "Reset all" );
    CORE::CDataNode* status = CFormLayout::AddWidgetNode( *page, "Label", "Status", "0,24 1,-60", "1,-48 0,52" );
    CFormLayout::SetWidgetNodeProperty( *status, EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CSettingsPageForm::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Change" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CInputSettingsForm::OnChangeActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Reset" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CInputSettingsForm::OnResetActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "ResetAll" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CInputSettingsForm::OnResetAllActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CInputSettingsForm::OnRootKeyDown );
        SubscribeTo( root, CWidget::KeyDownEvent, callback );
    }
    ShowActions();
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    m_rebindingActionId.Clear();
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
    {
        TAction& action = m_actions[ i ];
        INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
        UInt32 modifiers = 0;
        if ( GUCEF_NULL != m_shortcuts && m_shortcuts->GetChord( action.actionId, key, modifiers ) )
        {
            action.key = key;
            action.modifiers = modifiers;
        }
        else
        {
            action.key = action.defaultKey;
            action.modifiers = action.defaultModifiers;
        }
    }
    if ( GUCEF_NULL != GetRootWidget() )
    {
        ShowActions();
        ShowStatus( CString::Empty );
    }
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CInputSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shortcuts )
        return false;
    // unbound first, then bound: actions may have taken each other's chords
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        m_shortcuts->Unbind( m_actions[ i ].actionId );
    for ( UInt32 i=0; i<(UInt32) m_actions.size(); ++i )
        if ( INPUT::KEYCODE_UNASSIGNED != m_actions[ i ].key )
            m_shortcuts->Bind( m_actions[ i ].actionId, m_actions[ i ].key, m_actions[ i ].modifiers );
    SetHasChanges( false );
    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::OnChangeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    BeginRebind();
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::OnResetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_rebindingActionId.Clear();
    ResetAction( GetSelectedActionId() );
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::OnResetAllActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_rebindingActionId.Clear();
    ResetAll();
}

/*-------------------------------------------------------------------------*/

void
CInputSettingsForm::OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( eventData );
    if ( GUCEF_NULL == key || key->handled || EVENTPHASE_BUBBLE == key->phase || m_rebindingActionId.IsNULLOrEmpty() )
        return;

    // the keys that are only a modifier wait for the key they go with
    switch ( key->keyCode )
    {
        case INPUT::KEYCODE_LSHIFT: case INPUT::KEYCODE_RSHIFT: case INPUT::KEYCODE_LCONTROL: case INPUT::KEYCODE_RCONTROL:
        case INPUT::KEYCODE_LMENU: case INPUT::KEYCODE_RMENU: case INPUT::KEYCODE_LWIN: case INPUT::KEYCODE_RWIN: case INPUT::KEYCODE_CAPITAL:
            return;
        default:
            break;
    }
    key->handled = true;
    const CString actionId = m_rebindingActionId;
    const UInt32 modifiers = CKeyboardShortcutMap::NormalizeModifiers( key->modifiers );
    m_rebindingActionId.Clear();
    if ( INPUT::KEYCODE_ESCAPE == key->keyCode && 0 == modifiers )
    {
        ShowStatus( "The key was not changed." );
        return;
    }
    Rebind( actionId, key->keyCode, modifiers );
}

/*-------------------------------------------------------------------------*/

const CString&
CInputSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CInputSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
