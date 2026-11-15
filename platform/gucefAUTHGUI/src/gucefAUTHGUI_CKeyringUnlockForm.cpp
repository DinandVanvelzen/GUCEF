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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

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

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTHGUI_UTILS_H
#include "gucefAUTHGUI_Utils.h"
#define GUCEF_AUTHGUI_UTILS_H
#endif /* GUCEF_AUTHGUI_UTILS_H ? */

#include "gucefAUTHGUI_CKeyringUnlockForm.h"

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

const CORE::CEvent CKeyringUnlockForm::UnlockRequestedEvent = "GUCEF::AUTHGUI::CKeyringUnlockForm::UnlockRequestedEvent";
const CORE::CEvent CKeyringUnlockForm::CancelledEvent = "GUCEF::AUTHGUI::CKeyringUnlockForm::CancelledEvent";
const CORE::CEvent CKeyringUnlockForm::DoneEvent = "GUCEF::AUTHGUI::CKeyringUnlockForm::DoneEvent";
const UInt32 CKeyringUnlockForm::DefaultMinimumCodeLength;

/** The measures of the content, in pixels */
static const Int32 MaxWindowWidth = 520;
static const Int32 MinWindowWidth = 360;
static const Int32 LabelHeight = 24;
static const Int32 FieldHeight = 36;
static const Int32 ControlHeight = 38;
static const Int32 ContentBottomPadding = 16;

static const char ErrorColour[] = "255,110,110,255";
static const char StatusColour[] = "200,208,224,255";
static const char NoteColour[] = "150,160,180,255";

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::RegisterEvents( void )
{GUCEF_TRACE;

    UnlockRequestedEvent.Initialize();
    CancelledEvent.Initialize();
    DoneEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::CKeyringUnlockForm( void )
    : GUI::CFormEx()
    , m_session( GUCEF_NULL )
    , m_codeSettings()
    , m_minimumCodeLength( DefaultMinimumCodeLength )
    , m_mode( MODE_UNLOCK )
    , m_builtMode( MODE_UNLOCK )
    , m_built( false )
    , m_open( false )
    , m_busy( false )
    , m_waiting( false )
    , m_subscribedToFrames( false )
    , m_messageText()
    , m_messageIsError( false )
    , m_strength( STRENGTH_NONE )
    , m_shownWaitSeconds( 0 )
    , m_waitUntilMs( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_codeSettings.notifyCapsLock = true;
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::CKeyringUnlockForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_session( GUCEF_NULL )
    , m_codeSettings()
    , m_minimumCodeLength( DefaultMinimumCodeLength )
    , m_mode( MODE_UNLOCK )
    , m_builtMode( MODE_UNLOCK )
    , m_built( false )
    , m_open( false )
    , m_busy( false )
    , m_waiting( false )
    , m_subscribedToFrames( false )
    , m_messageText()
    , m_messageIsError( false )
    , m_strength( STRENGTH_NONE )
    , m_shownWaitSeconds( 0 )
    , m_waitUntilMs( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_codeSettings.notifyCapsLock = true;
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::~CKeyringUnlockForm()
{GUCEF_TRACE;

    ClearSecrets();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // from the top: the prompt, the note of a new code, the fields with their labels, the strength; the strip of the buttons at the bottom
    const bool newCode = MODE_UNLOCK != m_mode;
    Int32 y = WindowContentTop;
    struct SRow { const char* labelName; const char* label; const char* fieldName; };
    SRow rows[ 3 ];
    UInt32 rowCount = 0;
    const char* title = "Unlock saved credentials";
    const char* prompt = "Enter the keyring code to use the saved credentials.";
    const char* submit = "Unlock";
    if ( MODE_SET_UP == m_mode )
    {
        title = "Set up a keyring code";
        prompt = "Choose a keyring code for the saved credentials.";
        submit = "Set up";
        SRow code = { "CodeLabel", "New code", "Code" };
        SRow confirm = { "ConfirmLabel", "New code again", "Confirm" };
        rows[ rowCount++ ] = code;
        rows[ rowCount++ ] = confirm;
    }
    else
    if ( MODE_CHANGE_CODE == m_mode )
    {
        title = "Change the keyring code";
        prompt = "Change the keyring code of the saved credentials.";
        submit = "Change code";
        SRow current = { "CurrentLabel", "Current code", "Current" };
        SRow code = { "CodeLabel", "New code", "Code" };
        SRow confirm = { "ConfirmLabel", "New code again", "Confirm" };
        rows[ rowCount++ ] = current;
        rows[ rowCount++ ] = code;
        rows[ rowCount++ ] = confirm;
    }
    else
    {
        SRow code = { "CodeLabel", "Keyring code", "Code" };
        rows[ rowCount++ ] = code;
    }

    // the height of the content decides the height of the window
    Int32 contentHeight = LabelHeight + 12 + ( newCode ? LabelHeight + 12 : 0 ) + (Int32) rowCount * ( LabelHeight + 4 + FieldHeight + 12 ) + ( newCode ? LabelHeight : 0 );
    Int32 windowHeight = WindowContentTop + contentHeight + ContentBottomPadding + WindowFooterHeight;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "KeyringUnlockForm", "0.5,0 0.5,0", "0.9,0 0,"+ Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, title );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0," + Px( MinWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + Px( MaxWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* promptNode = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Prompt", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *promptNode, GUI::EWidgetProperty_Text, prompt );
    y += LabelHeight + 12;
    if ( newCode )
    {
        CORE::CDataNode* note = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Note", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_Text, "If the code is forgotten, the saved credentials are lost." );
        GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_TextColour, NoteColour );
        y += LabelHeight + 12;
    }
    for ( UInt32 i=0; i<rowCount; ++i )
    {
        CORE::CDataNode* label = GUI::CFormLayout::AddWidgetNode( *window, "Label", rows[ i ].labelName, "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *label, GUI::EWidgetProperty_Text, rows[ i ].label );
        y += LabelHeight + 4;
        GUI::CFormLayout::AddWidgetNode( *window, "SecretInput", rows[ i ].fieldName, "0,20 0," + Px( y ), "1,-40 0," + Px( FieldHeight ) );
        y += FieldHeight + 12;
    }
    if ( newCode )
    {
        CORE::CDataNode* strength = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Strength", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *strength, GUI::EWidgetProperty_Text, CORE::CString::Empty );
        GUI::CFormLayout::SetWidgetNodeProperty( *strength, GUI::EWidgetProperty_TextColour, NoteColour );
    }

    // the message and the buttons: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + Px( -( WindowFooterHeight + 1 ) ), "1,-2 0," + Px( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* message = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Message", "0,20 0,8", "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_TextColour, m_messageIsError ? ErrorColour : StatusColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_Text, m_messageText );
    CORE::CDataNode* submitNode = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Submit", "1,-276 1,-52", "0,136 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *submitNode, GUI::EWidgetProperty_Text, submit );
    GUI::CFormLayout::SetWidgetNodeProperty( *submitNode, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Cancel", "1,-132 1,-52", "0,112 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::Rebuild( void )
{GUCEF_TRACE;

    // the old fields go with their values (a secret input wipes itself when it goes)
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    bool loaded = LoadLayout( layout );
    if ( loaded )
    {
        m_built = true;
        m_builtMode = m_mode;
        GUI::CGUIContext* context = GetGUIContext();
        if ( GUCEF_NULL != context )
            context->FlushChanges();
    }
    return loaded;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::Build( void )
{GUCEF_TRACE;

    if ( !Rebuild() )
        return false;
    Hide();
    m_open = false;
    // the wait after wrong codes is counted down at every frame
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback frameStartedCallback( this, &CKeyringUnlockForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, frameStartedCallback );
        m_subscribedToFrames = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* button = GetWidget( "Submit" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CKeyringUnlockForm::OnSubmitActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Cancel" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CKeyringUnlockForm::OnCancelActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    const char* fields[ 3 ] = { "Current", "Code", "Confirm" };
    for ( UInt32 i=0; i<3; ++i )
    {
        GUI::CWidget* field = GetWidget( fields[ i ] );
        if ( GUCEF_NULL == field )
            continue;
        TEventCallback enteredCallback( this, &CKeyringUnlockForm::OnFieldTextEntered );
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, enteredCallback );
        TEventCallback changedCallback( this, &CKeyringUnlockForm::OnCodeChanged );
        SubscribeTo( field, GUI::CSecretInput::SecretChangedEvent, changedCallback );
        TEventCallback capsLockCallback( this, &CKeyringUnlockForm::OnCapsLockChanged );
        SubscribeTo( field, GUI::CSecretInput::CapsLockStateChangedEvent, capsLockCallback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigationCallback( this, &CKeyringUnlockForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, navigationCallback );
    }
    ApplyCodeSettings();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::ApplyCodeSettings( void )
{GUCEF_TRACE;

    const char* fields[ 3 ] = { "Current", "Code", "Confirm" };
    for ( UInt32 i=0; i<3; ++i )
    {
        GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( fields[ i ] );
        if ( GUCEF_NULL != field )
            field->SetSettings( m_codeSettings );
    }
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::Open( EMode mode )
{GUCEF_TRACE;

    if ( MODE_UNLOCK == mode && GUCEF_NULL != m_session && !m_session->IsSetUp() )
        mode = MODE_SET_UP;
    m_mode = mode;
    m_busy = false;
    m_waiting = false;
    m_messageText.Clear();
    m_messageIsError = false;
    m_strength = STRENGTH_NONE;
    if ( ( !m_built || m_builtMode != m_mode || GUCEF_NULL == GetRootWidget() ) && !Build() )
        return false;
    ClearSecrets();
    SetMessage( CORE::CString::Empty, false );
    UpdateStrength();

    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* first = GetWidget( MODE_CHANGE_CODE == m_mode ? "Current" : "Code" );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != first )
        context->SetFocus( first );

    // a wait after wrong codes that is not over yet
    if ( MODE_SET_UP != m_mode && GUCEF_NULL != m_session && m_session->GetRetryWaitInMs() > 0 )
        ShowResult( AUTH::KEYRING_THROTTLED, m_session->GetRetryWaitInMs() );
    UpdateState();
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::Close( void )
{GUCEF_TRACE;

    ClearSecrets();
    m_open = false;
    m_busy = false;         // what still runs is no longer the business of the form
    m_waiting = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::EMode
CKeyringUnlockForm::GetMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::SetSession( AUTH::CKeyringSession* session )
{GUCEF_TRACE;

    if ( session == m_session )
        return;
    if ( GUCEF_NULL != m_session )
        UnsubscribeFrom( m_session );
    m_session = session;
    m_busy = false;
    SubscribeToSession();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::SubscribeToSession( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_session )
        return;
    TEventCallback doneCallback( this, &CKeyringUnlockForm::OnSessionDone );
    SubscribeTo( m_session, AUTH::CKeyringSession::UnlockedEvent, doneCallback );
    TEventCallback setUpCallback( this, &CKeyringUnlockForm::OnSessionDone );
    SubscribeTo( m_session, AUTH::CKeyringSession::CodeSetUpEvent, setUpCallback );
    TEventCallback changedCallback( this, &CKeyringUnlockForm::OnSessionDone );
    SubscribeTo( m_session, AUTH::CKeyringSession::CodeChangedEvent, changedCallback );
    TEventCallback wrongCodeCallback( this, &CKeyringUnlockForm::OnSessionWrongCode );
    SubscribeTo( m_session, AUTH::CKeyringSession::WrongCodeEvent, wrongCodeCallback );
    TEventCallback failedCallback( this, &CKeyringUnlockForm::OnSessionFailed );
    SubscribeTo( m_session, AUTH::CKeyringSession::FailedEvent, failedCallback );
    TEventCallback retryCallback( this, &CKeyringUnlockForm::OnSessionRetryAllowed );
    SubscribeTo( m_session, AUTH::CKeyringSession::RetryAllowedEvent, retryCallback );
}

/*-------------------------------------------------------------------------*/

AUTH::CKeyringSession*
CKeyringUnlockForm::GetSession( void ) const
{GUCEF_TRACE;

    return m_session;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::SetCodeSettings( const GUI::CSecretInputSettings& settings )
{GUCEF_TRACE;

    m_codeSettings = settings;
    m_codeSettings.Sanitize();
    ApplyCodeSettings();
}

/*-------------------------------------------------------------------------*/

const GUI::CSecretInputSettings&
CKeyringUnlockForm::GetCodeSettings( void ) const
{GUCEF_TRACE;

    return m_codeSettings;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::SetMinimumCodeLength( UInt32 minimumCodeLength )
{GUCEF_TRACE;

    m_minimumCodeLength = minimumCodeLength < 1 ? 1 : minimumCodeLength;
    UpdateStrength();
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyringUnlockForm::GetMinimumCodeLength( void ) const
{GUCEF_TRACE;

    return m_minimumCodeLength;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::ReadField( const CORE::CString& name, AUTH::CSecretString& value ) const
{GUCEF_TRACE;

    value.Clear();
    GUI::CSecretInput* field = const_cast< CKeyringUnlockForm* >( this )->GetWidgetAs< GUI::CSecretInput >( name );
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

/*-------------------------------------------------------------------------*/

UInt32
CKeyringUnlockForm::GetFieldLength( const CORE::CString& name ) const
{GUCEF_TRACE;

    GUI::CSecretInput* field = const_cast< CKeyringUnlockForm* >( this )->GetWidgetAs< GUI::CSecretInput >( name );
    return GUCEF_NULL == field ? 0 : field->GetValueLength();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::GetCode( AUTH::CSecretString& code ) const
{GUCEF_TRACE;

    return ReadField( MODE_CHANGE_CODE == m_mode ? "Current" : "Code", code );
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::GetNewCode( AUTH::CSecretString& newCode ) const
{GUCEF_TRACE;

    if ( MODE_UNLOCK == m_mode )
    {
        newCode.Clear();
        return false;
    }
    return ReadField( "Code", newCode );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::ClearSecrets( void )
{GUCEF_TRACE;

    const char* fields[ 3 ] = { "Current", "Code", "Confirm" };
    for ( UInt32 i=0; i<3; ++i )
    {
        GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( fields[ i ] );
        if ( GUCEF_NULL != field )
        {
            field->SetRevealed( false );
            field->ClearValue();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::SetMessage( const CORE::CString& text, bool isError )
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

void
CKeyringUnlockForm::UpdateState( void )
{GUCEF_TRACE;

    GUI::CWidget* submit = GetWidget( "Submit" );
    if ( GUCEF_NULL != submit )
        submit->SetBoolProperty( GUI::EWidgetProperty_Enabled, !m_busy && !m_waiting );
    const char* fields[ 3 ] = { "Current", "Code", "Confirm" };
    for ( UInt32 i=0; i<3; ++i )
    {
        GUI::CWidget* field = GetWidget( fields[ i ] );
        if ( GUCEF_NULL != field )
            field->SetBoolProperty( GUI::EWidgetProperty_Enabled, !m_busy );
    }
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::EStrength
CKeyringUnlockForm::RateCode( const CORE::CSecureBuffer& code, UInt32 minimumCodeLength )
{GUCEF_TRACE;

    // code points and the kinds of characters, counted in place: nothing of the code is copied
    const UInt8* bytes = code.GetData();
    UInt32 size = code.GetSize();
    UInt32 length = 0;
    bool lower = false, upper = false, digit = false, other = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt8 c = bytes[ i ];
        if ( 0x80 == ( c & 0xC0 ) )
            continue;           // a continuation byte of UTF-8
        ++length;
        if ( c >= 'a' && c <= 'z' )
            lower = true;
        else
        if ( c >= 'A' && c <= 'Z' )
            upper = true;
        else
        if ( c >= '0' && c <= '9' )
            digit = true;
        else
            other = true;
    }
    if ( 0 == length )
        return STRENGTH_NONE;
    if ( length < minimumCodeLength )
        return STRENGTH_TOO_SHORT;
    UInt32 kinds = ( lower ? 1 : 0 ) + ( upper ? 1 : 0 ) + ( digit ? 1 : 0 ) + ( other ? 1 : 0 );
    if ( length >= 20 || ( length >= 14 && kinds >= 2 ) || ( length >= 12 && kinds >= 3 ) )
        return STRENGTH_STRONG;
    if ( length >= 12 || ( length >= 10 && kinds >= 3 ) )
        return STRENGTH_GOOD;
    if ( kinds >= 2 )
        return STRENGTH_FAIR;
    return STRENGTH_WEAK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKeyringUnlockForm::StrengthToText( EStrength strength )
{GUCEF_TRACE;

    switch ( strength )
    {
        case STRENGTH_TOO_SHORT: return "Too short";
        case STRENGTH_WEAK: return "Weak";
        case STRENGTH_FAIR: return "Fair";
        case STRENGTH_GOOD: return "Good";
        case STRENGTH_STRONG: return "Strong";
        default: return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::UpdateStrength( void )
{GUCEF_TRACE;

    m_strength = STRENGTH_NONE;
    GUI::CSecretInput* code = GetWidgetAs< GUI::CSecretInput >( "Code" );
    if ( MODE_UNLOCK != m_mode && GUCEF_NULL != code )
        m_strength = RateCode( code->GetValue(), m_minimumCodeLength );
    GUI::CWidget* strength = GetWidget( "Strength" );
    if ( GUCEF_NULL != strength )
    {
        CORE::CString text = GetStrengthText();
        if ( STRENGTH_TOO_SHORT == m_strength )
            text = "Too short: at least " + CORE::UInt32ToString( m_minimumCodeLength ) + " characters";
        strength->SetProperty( GUI::EWidgetProperty_Text, text.IsNULLOrEmpty() ? text : CORE::CString( "Strength: " ) + text );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKeyringUnlockForm::GetStrengthText( void ) const
{GUCEF_TRACE;

    if ( MODE_UNLOCK == m_mode )
        return CORE::CString::Empty;
    return StrengthToText( m_strength );
}

/*-------------------------------------------------------------------------*/

CKeyringUnlockForm::EStrength
CKeyringUnlockForm::GetStrength( void ) const
{GUCEF_TRACE;

    return m_strength;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKeyringUnlockForm::GetMessageText( void ) const
{GUCEF_TRACE;

    return m_messageText;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::IsBusy( void ) const
{GUCEF_TRACE;

    return m_busy;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::IsWaiting( void ) const
{GUCEF_TRACE;

    return m_waiting;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKeyringUnlockForm::MakeWaitText( UInt32 retryWaitInMs ) const
{GUCEF_TRACE;

    UInt32 seconds = ( retryWaitInMs + 999 ) / 1000;
    return "Wrong code. Wait " + CORE::UInt32ToString( seconds ) + ( 1 == seconds ? " second" : " seconds" ) + " before the next try.";
}

/*-------------------------------------------------------------------------*/

bool
CKeyringUnlockForm::Submit( void )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == GetRootWidget() )
        return false;
    if ( m_busy )
        return false;
    if ( m_waiting || ( MODE_SET_UP != m_mode && GUCEF_NULL != m_session && m_session->GetRetryWaitInMs() > 0 ) )
    {
        ShowResult( AUTH::KEYRING_THROTTLED, GUCEF_NULL != m_session ? m_session->GetRetryWaitInMs() : (UInt32) ( m_waitUntilMs > AUTH::GetMonotonicTimeInMs() ? m_waitUntilMs - AUTH::GetMonotonicTimeInMs() : 0 ) );
        return false;
    }

    // the checks of the form
    if ( MODE_CHANGE_CODE == m_mode && 0 == GetFieldLength( "Current" ) )
    {
        SetMessage( "Enter the current code.", true );
        return false;
    }
    if ( 0 == GetFieldLength( "Code" ) )
    {
        SetMessage( MODE_UNLOCK == m_mode ? "Enter the keyring code." : "Enter a new code.", true );
        return false;
    }
    if ( MODE_UNLOCK != m_mode )
    {
        if ( GetFieldLength( "Code" ) < m_minimumCodeLength )
        {
            SetMessage( "Use at least " + CORE::UInt32ToString( m_minimumCodeLength ) + " characters.", true );
            return false;
        }
        GUI::CSecretInput* code = GetWidgetAs< GUI::CSecretInput >( "Code" );
        GUI::CSecretInput* confirm = GetWidgetAs< GUI::CSecretInput >( "Confirm" );
        CORE::CSecureBuffer codeValue = code->GetValue();
        CORE::CSecureBuffer confirmValue = GUCEF_NULL != confirm ? confirm->GetValue() : CORE::CSecureBuffer();
        if ( !codeValue.Equals( confirmValue.GetData(), confirmValue.GetSize() ) )
        {
            if ( GUCEF_NULL != confirm )
                confirm->ClearValue();
            SetMessage( "The two new codes are not the same.", true );
            return false;
        }
    }

    // an application without a session takes the codes here
    if ( !NotifyObservers( UnlockRequestedEvent ) )
        return false;      // the form went away in an observer
    if ( !m_open )
        return true;       // the application dealt with it and closed the form

    if ( GUCEF_NULL == m_session )
    {
        ClearSecrets();
        return true;
    }

    AUTH::CSecretString code;
    AUTH::CSecretString newCode;
    GetCode( code );
    GetNewCode( newCode );
    ClearSecrets();
    CORE::CFutureResult future( CORE::TASKSTATUS_UNDEFINED );
    const char* runningText = "Checking the code...";
    switch ( m_mode )
    {
        case MODE_SET_UP:
        {
            future = m_session->SetUpCodeAsync( newCode );
            runningText = "Setting up the keyring code...";
            break;
        }
        case MODE_CHANGE_CODE:
        {
            future = m_session->ChangeCodeAsync( code, newCode );
            runningText = "Changing the keyring code...";
            break;
        }
        default:
        {
            future = m_session->UnlockWithCodeAsync( code );
            break;
        }
    }
    code.Clear();
    newCode.Clear();
    if ( future.HasNoFuture() )
    {
        // refused without starting (busy, waiting, no store) or not queued: the future says why
        AUTH::TKeyringResult result = AUTH::CKeyringSession::GetResultOfFuture( future );
        ShowResult( AUTH::KEYRING_OK == result ? AUTH::KEYRING_FAILED : result, m_session->GetRetryWaitInMs() );
        return false;
    }
    // it runs, or ran already on the pool: the outcome comes as the events of the session at its next tick
    m_busy = true;
    SetMessage( runningText, false );
    UpdateState();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::Cancel( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    Close();
    if ( wasOpen )
        NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::FinishDone( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    Close();
    if ( wasOpen )
        NotifyObservers( DoneEvent );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::ShowResult( AUTH::TKeyringResult result, UInt32 retryWaitInMs )
{GUCEF_TRACE;

    m_busy = false;
    GUI::CWidget* focus = GUCEF_NULL;
    switch ( result )
    {
        case AUTH::KEYRING_OK:
        {
            FinishDone();
            return;
        }
        case AUTH::KEYRING_WRONG_CODE:
        case AUTH::KEYRING_THROTTLED:
        {
            if ( retryWaitInMs > 0 )
            {
                m_waiting = true;
                m_waitUntilMs = AUTH::GetMonotonicTimeInMs() + retryWaitInMs;
                m_shownWaitSeconds = ( retryWaitInMs + 999 ) / 1000;
                SetMessage( MakeWaitText( retryWaitInMs ), true );
            }
            else
            {
                m_waiting = false;
                SetMessage( "Wrong code.", true );
            }
            focus = GetWidget( MODE_CHANGE_CODE == m_mode ? "Current" : "Code" );
            break;
        }
        case AUTH::KEYRING_BUSY: SetMessage( "The keyring is busy, try again in a moment.", true ); break;
        case AUTH::KEYRING_NOT_SET_UP: SetMessage( "There is no keyring code yet: set one up.", true ); break;
        case AUTH::KEYRING_ALREADY_SET_UP: SetMessage( "A keyring code is set up already.", true ); break;
        case AUTH::KEYRING_INVALID_CODE: SetMessage( "That code cannot be used.", true ); break;
        case AUTH::KEYRING_TAMPERED: SetMessage( "The saved credentials were changed and cannot be read.", true ); break;
        case AUTH::KEYRING_NEWER_VERSION: SetMessage( "The saved credentials are of a newer version of the application.", true ); break;
        case AUTH::KEYRING_UNAVAILABLE: SetMessage( "This build cannot open saved credentials.", true ); break;
        default: SetMessage( "The saved credentials could not be opened.", true ); break;
    }
    UpdateState();
    GUI::CGUIContext* context = GetGUIContext();
    if ( m_open && GUCEF_NULL != focus && GUCEF_NULL != context )
        context->SetFocus( focus );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnSubmitActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Submit();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnFieldTextEntered( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Submit();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnCodeChanged( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateStrength();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnCapsLockChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a hint that does not push away what went wrong or what runs
    static const char capsLockText[] = "Caps Lock is on.";
    const CORE::TCloneableBool* on = static_cast< const CORE::TCloneableBool* >( eventData );
    if ( GUCEF_NULL == on || m_busy || m_waiting )
        return;
    if ( on->GetData() && ( m_messageText.IsNULLOrEmpty() || !m_messageIsError ) )
        SetMessage( capsLockText, false );
    else
    if ( !on->GetData() && capsLockText == m_messageText )
        SetMessage( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnNavigation( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Cancel();
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_open || !m_waiting )
        return;
    UInt64 now = AUTH::GetMonotonicTimeInMs();
    UInt32 remaining = 0;
    if ( GUCEF_NULL != m_session )
        remaining = m_session->GetRetryWaitInMs();
    else
    if ( m_waitUntilMs > now )
        remaining = (UInt32) ( m_waitUntilMs - now );
    if ( 0 == remaining )
    {
        m_waiting = false;
        m_shownWaitSeconds = 0;
        SetMessage( "Wrong code. Try again.", true );
        UpdateState();
        return;
    }
    UInt32 seconds = ( remaining + 999 ) / 1000;
    if ( seconds != m_shownWaitSeconds )
    {
        m_shownWaitSeconds = seconds;
        SetMessage( MakeWaitText( remaining ), true );
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnSessionDone( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the operation of the form, or an unlock elsewhere while the form asks for the code
    if ( m_busy || ( m_open && MODE_UNLOCK == m_mode && AUTH::CKeyringSession::UnlockedEvent == eventId ) )
        FinishDone();
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnSessionWrongCode( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_busy )
        return;
    const AUTH::CKeyringEventData* data = static_cast< const AUTH::CKeyringEventData* >( eventData );
    ShowResult( AUTH::KEYRING_WRONG_CODE, GUCEF_NULL != data ? data->retryWaitInMs : 0 );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnSessionFailed( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_busy )
        return;
    const AUTH::CKeyringEventData* data = static_cast< const AUTH::CKeyringEventData* >( eventData );
    ShowResult( GUCEF_NULL != data ? data->result : AUTH::KEYRING_FAILED, 0 );
}

/*-------------------------------------------------------------------------*/

void
CKeyringUnlockForm::OnSessionRetryAllowed( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_waiting )
        return;
    m_waiting = false;
    m_shownWaitSeconds = 0;
    if ( m_open )
        SetMessage( "Wrong code. Try again.", true );
    UpdateState();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKeyringUnlockForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::CKeyringUnlockForm";
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
