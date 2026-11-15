/*
 *  gucefAUTHGUI: GUCEF module providing prebaked account and login user interface forms
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

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_AUTHGUI_UTILS_H
#include "gucefAUTHGUI_Utils.h"
#define GUCEF_AUTHGUI_UTILS_H
#endif /* GUCEF_AUTHGUI_UTILS_H ? */

#include "gucefAUTHGUI_CSignInSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CSignInSettingsForm::SignInRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::SignInRequestedEvent";
const CORE::CEvent CSignInSettingsForm::SignOutRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::SignOutRequestedEvent";
const CORE::CEvent CSignInSettingsForm::SaveCredentialsRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::SaveCredentialsRequestedEvent";
const CORE::CEvent CSignInSettingsForm::ForgetCredentialsRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::ForgetCredentialsRequestedEvent";
const CORE::CEvent CSignInSettingsForm::SetUpCodeRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::SetUpCodeRequestedEvent";
const CORE::CEvent CSignInSettingsForm::ChangeCodeRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::ChangeCodeRequestedEvent";
const CORE::CEvent CSignInSettingsForm::LockNowRequestedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::LockNowRequestedEvent";
const CORE::CEvent CSignInSettingsForm::ClosedEvent = "GUCEF::AUTHGUI::CSignInSettingsForm::ClosedEvent";

/** The measures of the content, in pixels */
static const Int32 MaxWindowWidth = 560;
static const Int32 MinWindowWidth = 480;
static const Int32 LabelHeight = 24;
static const Int32 FieldHeight = 36;
static const Int32 ControlHeight = 38;
static const Int32 ContentBottomPadding = 12;
static const Int32 MaxSlots = 8;

static const char ErrorColour[] = "255,110,110,255";
static const char StatusColour[] = "200,208,224,255";
static const char NoteColour[] = "150,160,180,255";
static const char HeaderColour[] = "230,235,245,255";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Px( Int32 value )
{GUCEF_TRACE;

    return CORE::Int32ToString( value );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
SlotName( UInt32 index )
{GUCEF_TRACE;

    return "Slot" + CORE::UInt32ToString( index );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
SlotLabelName( UInt32 index )
{GUCEF_TRACE;

    return "SlotLabel" + CORE::UInt32ToString( index );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSignInSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    SignInRequestedEvent.Initialize();
    SignOutRequestedEvent.Initialize();
    SaveCredentialsRequestedEvent.Initialize();
    ForgetCredentialsRequestedEvent.Initialize();
    SetUpCodeRequestedEvent.Initialize();
    ChangeCodeRequestedEvent.Initialize();
    LockNowRequestedEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSignInSettingsForm::CSignInSettingsForm( void )
    : GUI::CFormEx()
    , m_state()
    , m_credentialSettings()
    , m_messageText()
    , m_messageIsError( false )
    , m_built( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_credentialSettings.allowReveal = true;
}

/*-------------------------------------------------------------------------*/

CSignInSettingsForm::CSignInSettingsForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_state()
    , m_credentialSettings()
    , m_messageText()
    , m_messageIsError( false )
    , m_built( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_credentialSettings.allowReveal = true;
}

/*-------------------------------------------------------------------------*/

CSignInSettingsForm::~CSignInSettingsForm()
{GUCEF_TRACE;

    ClearSecrets();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSignInSettingsForm::MakeSlotLabel( const CORE::CString& slot )
{GUCEF_TRACE;

    if ( slot.IsNULLOrEmpty() )
        return slot;
    // the name of a slot with a capital letter first: "username" is "Username"
    return slot.SubstrFromRange( 0, 1 ).Uppercase() + slot.SubstrFromRange( 1, slot.Length() );
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // from the top: the state with Sign in and Sign out, the saved credentials with a field for each slot, the keyring code; the strip along the bottom edge
    const UInt32 slotCount = (UInt32) m_state.credentialSlots.size() > (UInt32) MaxSlots ? (UInt32) MaxSlots : (UInt32) m_state.credentialSlots.size();
    Int32 contentHeight = LabelHeight + 8 + ControlHeight + 14                                              // state and its buttons
                        + ( LabelHeight + 4 ) * 2 + (Int32) slotCount * ( LabelHeight + 2 + FieldHeight + 8 ) + ControlHeight + 14   // the credentials
                        + ( LabelHeight + 4 ) * 2 + ControlHeight;                                          // the keyring code
    Int32 windowHeight = WindowContentTop + contentHeight + ContentBottomPadding + WindowFooterHeight;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "SignInSettingsForm", "0.5,0 0.5,0", "0.9,0 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Sign in: " + ( m_state.serviceName.IsNULLOrEmpty() ? CORE::CString( "service" ) : m_state.serviceName ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0," + Px( MinWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + Px( MaxWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    Int32 y = WindowContentTop;
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    y += LabelHeight + 8;
    CORE::CDataNode* signIn = GUI::CFormLayout::AddWidgetNode( *window, "Button", "SignIn", "0,20 0," + Px( y ), "0,150 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *signIn, GUI::EWidgetProperty_Text, "Sign in" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signIn, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* signOut = GUI::CFormLayout::AddWidgetNode( *window, "Button", "SignOut", "0,182 0," + Px( y ), "0,150 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *signOut, GUI::EWidgetProperty_Text, "Sign out" );
    y += ControlHeight + 14;

    CORE::CDataNode* credentialsHeader = GUI::CFormLayout::AddWidgetNode( *window, "Label", "CredentialsHeader", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *credentialsHeader, GUI::EWidgetProperty_Text, "Saved credentials" );
    GUI::CFormLayout::SetWidgetNodeProperty( *credentialsHeader, GUI::EWidgetProperty_TextColour, HeaderColour );
    y += LabelHeight + 4;
    CORE::CDataNode* credentialsNote = GUI::CFormLayout::AddWidgetNode( *window, "Label", "CredentialsNote", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *credentialsNote, GUI::EWidgetProperty_TextColour, NoteColour );
    y += LabelHeight + 4;
    for ( UInt32 i=0; i<slotCount; ++i )
    {
        CORE::CDataNode* label = GUI::CFormLayout::AddWidgetNode( *window, "Label", SlotLabelName( i ), "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *label, GUI::EWidgetProperty_Text, MakeSlotLabel( m_state.credentialSlots[ i ] ) );
        y += LabelHeight + 2;
        GUI::CFormLayout::AddWidgetNode( *window, "SecretInput", SlotName( i ), "0,20 0," + Px( y ), "1,-40 0," + Px( FieldHeight ) );
        y += FieldHeight + 8;
    }
    CORE::CDataNode* save = GUI::CFormLayout::AddWidgetNode( *window, "Button", "SaveCredentials", "0,20 0," + Px( y ), "0,200 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *save, GUI::EWidgetProperty_Text, "Save credentials..." );
    CORE::CDataNode* forget = GUI::CFormLayout::AddWidgetNode( *window, "Button", "ForgetCredentials", "0,232 0," + Px( y ), "0,240 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *forget, GUI::EWidgetProperty_Text, "Forget saved credentials" );
    y += ControlHeight + 14;

    CORE::CDataNode* keyringHeader = GUI::CFormLayout::AddWidgetNode( *window, "Label", "KeyringHeader", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *keyringHeader, GUI::EWidgetProperty_Text, "Keyring code" );
    GUI::CFormLayout::SetWidgetNodeProperty( *keyringHeader, GUI::EWidgetProperty_TextColour, HeaderColour );
    y += LabelHeight + 4;
    CORE::CDataNode* keyringNote = GUI::CFormLayout::AddWidgetNode( *window, "Label", "KeyringNote", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *keyringNote, GUI::EWidgetProperty_TextColour, NoteColour );
    y += LabelHeight + 4;
    CORE::CDataNode* keyringCode = GUI::CFormLayout::AddWidgetNode( *window, "Button", "KeyringCode", "0,20 0," + Px( y ), "0,240 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *keyringCode, GUI::EWidgetProperty_Text, "Change keyring code..." );
    CORE::CDataNode* lockNow = GUI::CFormLayout::AddWidgetNode( *window, "Button", "LockNow", "0,272 0," + Px( y ), "0,140 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *lockNow, GUI::EWidgetProperty_Text, "Lock now" );

    // the message and the button: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + Px( -( WindowFooterHeight + 1 ) ), "1,-2 0," + Px( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* message = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Message", "0,20 0,8", "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_TextColour, m_messageIsError ? ErrorColour : StatusColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_Text, m_messageText );
    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Close", "1,-132 1,-52", "0,112 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::Rebuild( void )
{GUCEF_TRACE;

    // the old fields go with their values (a secret input wipes itself when it goes)
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    bool loaded = LoadLayout( layout );
    if ( loaded )
    {
        m_built = true;
        GUI::CGUIContext* context = GetGUIContext();
        if ( GUCEF_NULL != context )
            context->FlushChanges();
        UpdateState();
    }
    return loaded;
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::Build( void )
{GUCEF_TRACE;

    if ( !Rebuild() )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    const char* buttons[ 7 ] = { "SignIn", "SignOut", "SaveCredentials", "ForgetCredentials", "KeyringCode", "LockNow", "Close" };
    for ( UInt32 i=0; i<7; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL == button )
            continue;
        TEventCallback callback( this, 0 == i ? &CSignInSettingsForm::OnSignInActivated : 1 == i ? &CSignInSettingsForm::OnSignOutActivated :
                                       2 == i ? &CSignInSettingsForm::OnSaveActivated : 3 == i ? &CSignInSettingsForm::OnForgetActivated :
                                       4 == i ? &CSignInSettingsForm::OnKeyringActivated : 5 == i ? &CSignInSettingsForm::OnLockActivated : &CSignInSettingsForm::OnCloseActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    for ( UInt32 i=0; i<(UInt32) MaxSlots; ++i )
    {
        GUI::CWidget* field = GetWidget( SlotName( i ) );
        if ( GUCEF_NULL == field )
            break;
        TEventCallback enteredCallback( this, &CSignInSettingsForm::OnSlotTextEntered );
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, enteredCallback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigationCallback( this, &CSignInSettingsForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, navigationCallback );
    }
    ApplyCredentialSettings();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::ApplyCredentialSettings( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) MaxSlots; ++i )
    {
        GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( SlotName( i ) );
        if ( GUCEF_NULL == field )
            break;
        field->SetSettings( m_credentialSettings );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::Open( void )
{GUCEF_TRACE;

    if ( ( !m_built || GUCEF_NULL == GetRootWidget() ) && !Build() )
        return false;
    ClearSecrets();
    ShowMessage( CORE::CString::Empty, false );
    UpdateState();
    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* first = GetWidget( SlotName( 0 ) );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != first )
        context->SetFocus( first );
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::Close( void )
{GUCEF_TRACE;

    ClearSecrets();
    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::SetState( const CSignInSettingsState& state )
{GUCEF_TRACE;

    const bool layoutChanges = !m_built || m_state.credentialSlots != state.credentialSlots || m_state.serviceName != state.serviceName;
    m_state = state;
    if ( layoutChanges )
    {
        if ( m_built )
        {
            const bool wasOpen = m_open;
            Rebuild();
            if ( wasOpen )
            {
                Hide();
                ShowModal();
            }
            else
            {
                Hide();
            }
        }
        return;
    }
    UpdateState();
}

/*-------------------------------------------------------------------------*/

const CSignInSettingsState&
CSignInSettingsForm::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::SetCredentialSettings( const GUI::CSecretInputSettings& settings )
{GUCEF_TRACE;

    m_credentialSettings = settings;
    m_credentialSettings.Sanitize();
    ApplyCredentialSettings();
}

/*-------------------------------------------------------------------------*/

const GUI::CSecretInputSettings&
CSignInSettingsForm::GetCredentialSettings( void ) const
{GUCEF_TRACE;

    return m_credentialSettings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSignInSettingsForm::GetStatusText( void ) const
{GUCEF_TRACE;

    CORE::CString text = m_state.signedIn ? CORE::CString( "Signed in" ) : m_state.signInRunning ? CORE::CString( "Signing in..." ) : CORE::CString( "Not signed in" );
    if ( !m_state.statusNote.IsNULLOrEmpty() )
        text += ( m_state.signInRunning && !m_state.signedIn ? " " : ". " ) + m_state.statusNote;
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSignInSettingsForm::GetCredentialsNoteText( void ) const
{GUCEF_TRACE;

    if ( m_state.credentialSlots.empty() )
        return "This service needs no credentials.";
    switch ( m_state.keyringState )
    {
        case CSignInSettingsState::KEYRING_UNAVAILABLE: return "This build cannot keep saved credentials.";
        case CSignInSettingsState::KEYRING_NOT_SET_UP: return "No credentials are saved.";
        case CSignInSettingsState::KEYRING_LOCKED: return "Unlock the keyring code to see what is saved.";
        default: return m_state.hasSavedCredentials ? CORE::CString( "Credentials are saved for an automatic sign in." ) : CORE::CString( "No credentials are saved." );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSignInSettingsForm::GetKeyringNoteText( void ) const
{GUCEF_TRACE;

    switch ( m_state.keyringState )
    {
        case CSignInSettingsState::KEYRING_UNAVAILABLE: return "Not available in this build.";
        case CSignInSettingsState::KEYRING_NOT_SET_UP: return "No keyring code yet. Set one up to save credentials.";
        case CSignInSettingsState::KEYRING_LOCKED: return "Locked. Give the code to use the saved credentials.";
        default: return "Unlocked. It locks again when idle.";
    }
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::UpdateState( void )
{GUCEF_TRACE;

    const bool idle = !m_state.signInRunning;
    const bool keyringKept = CSignInSettingsState::KEYRING_UNAVAILABLE != m_state.keyringState;
    const bool setUp = CSignInSettingsState::KEYRING_LOCKED == m_state.keyringState || CSignInSettingsState::KEYRING_UNLOCKED == m_state.keyringState;

    GUI::CWidget* widget = GetWidget( "Status" );
    if ( GUCEF_NULL != widget )
    {
        widget->SetProperty( GUI::EWidgetProperty_Text, GetStatusText() );
        widget->SetProperty( GUI::EWidgetProperty_TextColour, m_state.signedIn ? "140,220,160,255" : StatusColour );
    }
    widget = GetWidget( "CredentialsNote" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, GetCredentialsNoteText() );
    widget = GetWidget( "KeyringNote" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, GetKeyringNoteText() );

    widget = GetWidget( "SignIn" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, !m_state.signedIn && idle );
    widget = GetWidget( "SignOut" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, m_state.signedIn && idle );
    widget = GetWidget( "SaveCredentials" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, keyringKept && !m_state.credentialSlots.empty() );
    widget = GetWidget( "ForgetCredentials" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, setUp && !m_state.credentialSlots.empty() );
    widget = GetWidget( "KeyringCode" );
    if ( GUCEF_NULL != widget )
    {
        widget->SetProperty( GUI::EWidgetProperty_Text, CSignInSettingsState::KEYRING_NOT_SET_UP == m_state.keyringState ? "Set up keyring code..." : "Change keyring code..." );
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, keyringKept );
    }
    widget = GetWidget( "LockNow" );
    if ( GUCEF_NULL != widget )
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, CSignInSettingsState::KEYRING_UNLOCKED == m_state.keyringState );
    for ( UInt32 i=0; i<(UInt32) MaxSlots; ++i )
    {
        widget = GetWidget( SlotName( i ) );
        if ( GUCEF_NULL == widget )
            break;
        widget->SetBoolProperty( GUI::EWidgetProperty_Enabled, keyringKept );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::IsActionEnabled( const CORE::CString& widgetName ) const
{GUCEF_TRACE;

    GUI::CWidget* widget = const_cast< CSignInSettingsForm* >( this )->GetWidget( widgetName );
    return GUCEF_NULL != widget && widget->GetBoolProperty( GUI::EWidgetProperty_Enabled );
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::ShowMessage( const CORE::CString& text, bool isError )
{GUCEF_TRACE;

    m_messageText = text;
    m_messageIsError = isError;
    GUI::CWidget* message = GetWidget( "Message" );
    if ( GUCEF_NULL != message )
    {
        message->SetProperty( GUI::EWidgetProperty_TextColour, isError ? ErrorColour : StatusColour );
        message->SetProperty( GUI::EWidgetProperty_Text, text );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSignInSettingsForm::GetMessageText( void ) const
{GUCEF_TRACE;

    return m_messageText;
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::GetCredential( const CORE::CString& slot, AUTH::CSecretString& value ) const
{GUCEF_TRACE;

    value.Clear();
    for ( UInt32 i=0; i<(UInt32) m_state.credentialSlots.size() && i<(UInt32) MaxSlots; ++i )
    {
        if ( m_state.credentialSlots[ i ] != slot )
            continue;
        GUI::CSecretInput* field = const_cast< CSignInSettingsForm* >( this )->GetWidgetAs< GUI::CSecretInput >( SlotName( i ) );
        if ( GUCEF_NULL == field || !field->HasValue() )
            return false;
        // the copy in a string is wiped as soon as the secret string has it
        CORE::CSecureBuffer buffer = field->GetValue();
        CORE::CString text;
        buffer.CopyToString( text );
        value.Set( text );
        CORE::SecureWipeString( text );
        return !value.IsEmpty();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::ClearSecrets( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) MaxSlots; ++i )
    {
        GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( SlotName( i ) );
        if ( GUCEF_NULL == field )
            break;
        field->SetRevealed( false );
        field->ClearValue();
    }
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::SignIn( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "SignIn" ) )
        return false;
    return NotifyObservers( SignInRequestedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::SignOut( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "SignOut" ) )
        return false;
    return NotifyObservers( SignOutRequestedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::SaveCredentials( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "SaveCredentials" ) )
        return false;
    const UInt32 slotCount = (UInt32) m_state.credentialSlots.size() > (UInt32) MaxSlots ? (UInt32) MaxSlots : (UInt32) m_state.credentialSlots.size();
    for ( UInt32 i=0; i<slotCount; ++i )
    {
        GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( SlotName( i ) );
        if ( GUCEF_NULL == field || 0 == field->GetValueLength() )
        {
            ShowMessage( "Fill in the " + MakeSlotLabel( m_state.credentialSlots[ i ] ).Lowercase() + " first.", true );
            if ( GUCEF_NULL != field && GUCEF_NULL != GetGUIContext() )
                GetGUIContext()->SetFocus( field );
            return false;
        }
    }
    ShowMessage( CORE::CString::Empty, false );
    // the handler takes the values with GetCredential(); the fields are wiped when it is done
    const bool notified = NotifyObservers( SaveCredentialsRequestedEvent );
    if ( notified )
        ClearSecrets();
    return notified;
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::ForgetCredentials( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "ForgetCredentials" ) )
        return false;
    return NotifyObservers( ForgetCredentialsRequestedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::KeyringCode( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "KeyringCode" ) )
        return false;
    return NotifyObservers( CSignInSettingsState::KEYRING_NOT_SET_UP == m_state.keyringState ? SetUpCodeRequestedEvent : ChangeCodeRequestedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CSignInSettingsForm::LockNow( void )
{GUCEF_TRACE;

    if ( !m_open || !IsActionEnabled( "LockNow" ) )
        return false;
    return NotifyObservers( LockNowRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::CloseWindow( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    Close();
    if ( wasOpen )
        NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SignIn();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnSignOutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SignOut();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SaveCredentials();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnForgetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ForgetCredentials();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnKeyringActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    KeyringCode();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnLockActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    LockNow();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CloseWindow();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnSlotTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SaveCredentials();
}

/*-------------------------------------------------------------------------*/

void
CSignInSettingsForm::OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        CloseWindow();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSignInSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::CSignInSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
