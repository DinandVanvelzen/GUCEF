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

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

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

#include "gucefAUTHGUI_COAuthCodeEntryForm.h"

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

const CORE::CEvent COAuthCodeEntryForm::CodeSubmittedEvent = "GUCEF::AUTHGUI::COAuthCodeEntryForm::CodeSubmittedEvent";
const CORE::CEvent COAuthCodeEntryForm::CancelledEvent = "GUCEF::AUTHGUI::COAuthCodeEntryForm::CancelledEvent";
const CORE::CEvent COAuthCodeEntryForm::DoneEvent = "GUCEF::AUTHGUI::COAuthCodeEntryForm::DoneEvent";

/** The measures of the content, in pixels */
static const Int32 MaxWindowWidth = 640;
static const Int32 MinWindowWidth = 400;
static const Int32 LabelHeight = 24;
static const Int32 FieldHeight = 36;
static const Int32 ControlHeight = 38;
static const Int32 SideButtonWidth = 112;
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
COAuthCodeEntryForm::RegisterEvents( void )
{GUCEF_TRACE;

    CodeSubmittedEvent.Initialize();
    CancelledEvent.Initialize();
    DoneEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

COAuthCodeEntryForm::COAuthCodeEntryForm( void )
    : GUI::CFormEx()
    , m_engine( GUCEF_NULL )
    , m_codeSettings()
    , m_providerName()
    , m_url()
    , m_messageText()
    , m_messageIsError( false )
    , m_built( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_codeSettings.allowPaste = true;
    m_codeSettings.allowReveal = true;
    m_codeSettings.allowCopy = false;
    m_codeSettings.hideLength = false;
    m_codeSettings.maxLength = GUI::CSecretInputSettings::AbsoluteMaxLength;
    m_codeSettings.Sanitize();
}

/*-------------------------------------------------------------------------*/

COAuthCodeEntryForm::COAuthCodeEntryForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_engine( GUCEF_NULL )
    , m_codeSettings()
    , m_providerName()
    , m_url()
    , m_messageText()
    , m_messageIsError( false )
    , m_built( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_codeSettings.allowPaste = true;
    m_codeSettings.allowReveal = true;
    m_codeSettings.allowCopy = false;
    m_codeSettings.hideLength = false;
    m_codeSettings.maxLength = GUI::CSecretInputSettings::AbsoluteMaxLength;
    m_codeSettings.Sanitize();
}

/*-------------------------------------------------------------------------*/

COAuthCodeEntryForm::~COAuthCodeEntryForm()
{GUCEF_TRACE;

    ClearSecrets();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // from the top: what to do, the address with its buttons, the code with its button; the strip of the buttons at the bottom
    Int32 y = WindowContentTop;
    Int32 contentHeight = 2 * ( LabelHeight + 4 ) + 8 + ( LabelHeight + 4 + FieldHeight + 8 + ControlHeight + 14 ) + ( LabelHeight + 4 + FieldHeight );
    Int32 windowHeight = WindowContentTop + contentHeight + ContentBottomPadding + WindowFooterHeight;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "OAuthCodeEntryForm", "0.5,0 0.5,0", "0.9,0 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Sign in with a code" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0," + Px( MinWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + Px( MaxWindowWidth ) + " 0," + Px( windowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* prompt = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Prompt", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *prompt, GUI::EWidgetProperty_Text, "Sign in to " + ( m_providerName.IsNULLOrEmpty() ? CORE::CString( "the store" ) : m_providerName ) + " on its page." );
    y += LabelHeight + 4;
    CORE::CDataNode* note = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Note", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_Text, "Allow access there, then paste the code it shows." );
    GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_TextColour, NoteColour );
    y += LabelHeight + 4 + 8;

    CORE::CDataNode* urlLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "UrlLabel", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *urlLabel, GUI::EWidgetProperty_Text, "Page to sign in on" );
    y += LabelHeight + 4;
    CORE::CDataNode* url = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Url", "0,20 0," + Px( y ), "1,-40 0," + Px( FieldHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_ReadOnly, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_SelectAllOnFocus, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_Text, m_url );
    y += FieldHeight + 8;
    CORE::CDataNode* copy = GUI::CFormLayout::AddWidgetNode( *window, "Button", "CopyUrl", "0,20 0," + Px( y ), "0,160 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *copy, GUI::EWidgetProperty_Text, "Copy address" );
    CORE::CDataNode* open = GUI::CFormLayout::AddWidgetNode( *window, "Button", "OpenUrl", "0,192 0," + Px( y ), "0,180 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *open, GUI::EWidgetProperty_Text, "Open in browser" );
    y += ControlHeight + 14;

    CORE::CDataNode* codeLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "CodeLabel", "0,20 0," + Px( y ), "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *codeLabel, GUI::EWidgetProperty_Text, "Code from the page" );
    y += LabelHeight + 4;
    GUI::CFormLayout::AddWidgetNode( *window, "SecretInput", "Code", "0,20 0," + Px( y ), "1," + Px( -( 40 + SideButtonWidth + 8 ) ) + " 0," + Px( FieldHeight ) );
    CORE::CDataNode* paste = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Paste", "1," + Px( -( 20 + SideButtonWidth ) ) + " 0," + Px( y ), "0," + Px( SideButtonWidth ) + " 0," + Px( FieldHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *paste, GUI::EWidgetProperty_Text, "Paste" );

    // the message and the buttons: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + Px( -( WindowFooterHeight + 1 ) ), "1,-2 0," + Px( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* message = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Message", "0,20 0,8", "1,-40 0," + Px( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_TextColour, m_messageIsError ? ErrorColour : StatusColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_Text, m_messageText );
    CORE::CDataNode* submit = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Submit", "1,-276 1,-52", "0,136 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *submit, GUI::EWidgetProperty_Text, "Submit code" );
    GUI::CFormLayout::SetWidgetNodeProperty( *submit, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Cancel", "1,-132 1,-52", "0,112 0," + Px( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::Rebuild( void )
{GUCEF_TRACE;

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
    }
    return loaded;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::Build( void )
{GUCEF_TRACE;

    if ( !Rebuild() )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    const char* buttons[ 5 ] = { "Submit", "Cancel", "CopyUrl", "OpenUrl", "Paste" };
    for ( UInt32 i=0; i<5; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL == button )
            continue;
        TEventCallback callback( this, 0 == i ? &COAuthCodeEntryForm::OnSubmitActivated : 1 == i ? &COAuthCodeEntryForm::OnCancelActivated :
                                       2 == i ? &COAuthCodeEntryForm::OnCopyActivated : 3 == i ? &COAuthCodeEntryForm::OnOpenActivated : &COAuthCodeEntryForm::OnPasteActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* field = GetWidget( "Code" );
    if ( GUCEF_NULL != field )
    {
        TEventCallback enteredCallback( this, &COAuthCodeEntryForm::OnCodeTextEntered );
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, enteredCallback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigationCallback( this, &COAuthCodeEntryForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, navigationCallback );
    }
    ApplyCodeSettings();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::ApplyCodeSettings( void )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "Code" );
    if ( GUCEF_NULL != field )
        field->SetSettings( m_codeSettings );
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::Open( const CORE::CString& providerDisplayName, const CORE::CString& authorizationUrl )
{GUCEF_TRACE;

    m_providerName = providerDisplayName;
    m_url = authorizationUrl;
    m_messageText.Clear();
    m_messageIsError = false;
    // the prompt and the address are part of the layout: it is made again for each login
    if ( !Rebuild() )
        return false;
    Hide();
    ClearSecrets();

    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* field = GetWidget( "Code" );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != field )
        context->SetFocus( field );
    UpdateState();
    return shown;
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::Close( void )
{GUCEF_TRACE;

    ClearSecrets();
    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COAuthCodeEntryForm::GetProviderDisplayName( void ) const
{GUCEF_TRACE;

    return m_providerName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COAuthCodeEntryForm::GetAuthorizationUrl( void ) const
{GUCEF_TRACE;

    return m_url;
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::SetEngine( AUTH::IAuthFlowEngine* engine )
{GUCEF_TRACE;

    m_engine = engine;
}

/*-------------------------------------------------------------------------*/

AUTH::IAuthFlowEngine*
COAuthCodeEntryForm::GetEngine( void ) const
{GUCEF_TRACE;

    return m_engine;
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::SetCodeSettings( const GUI::CSecretInputSettings& settings )
{GUCEF_TRACE;

    m_codeSettings = settings;
    m_codeSettings.Sanitize();
    ApplyCodeSettings();
}

/*-------------------------------------------------------------------------*/

const GUI::CSecretInputSettings&
COAuthCodeEntryForm::GetCodeSettings( void ) const
{GUCEF_TRACE;

    return m_codeSettings;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::ReadCode( AUTH::CSecretString& value ) const
{GUCEF_TRACE;

    value.Clear();
    GUI::CSecretInput* field = const_cast< COAuthCodeEntryForm* >( this )->GetWidgetAs< GUI::CSecretInput >( "Code" );
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

bool
COAuthCodeEntryForm::GetCode( AUTH::CSecretString& code ) const
{GUCEF_TRACE;

    return ReadCode( code );
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::ClearSecrets( void )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "Code" );
    if ( GUCEF_NULL != field )
    {
        field->SetRevealed( false );
        field->ClearValue();
    }
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::ShowMessage( const CORE::CString& text, bool isError )
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
COAuthCodeEntryForm::GetMessageText( void ) const
{GUCEF_TRACE;

    return m_messageText;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::CanOpenUrl( void ) const
{GUCEF_TRACE;

    return CORE::IsOpenableBrowserUrl( m_url );
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::UpdateState( void )
{GUCEF_TRACE;

    GUI::CWidget* open = GetWidget( "OpenUrl" );
    if ( GUCEF_NULL != open )
    {
        const bool canOpen = CanOpenUrl();
        open->SetBoolProperty( GUI::EWidgetProperty_Enabled, canOpen );
        open->SetProperty( GUI::EWidgetProperty_Tooltip, canOpen ? CORE::CString::Empty : CORE::CString( "This address cannot be opened from here: copy it into a browser" ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
COAuthCodeEntryForm::CheckToText( AUTH::TCodeCheck check )
{GUCEF_TRACE;

    switch ( check )
    {
        case AUTH::CODECHECK_ACCEPTED: return CORE::CString::Empty;
        case AUTH::CODECHECK_NO_LOGIN_WAITING: return "No sign in is waiting for a code: start it again.";
        case AUTH::CODECHECK_AMBIGUOUS: return "Several sign ins wait for a code: finish the others.";
        case AUTH::CODECHECK_EMPTY: return "The code is empty.";
        case AUTH::CODECHECK_MALFORMED: return "That is not a code. Copy it as the page shows it.";
        case AUTH::CODECHECK_NO_CODE: return "The text has no authorization code in it.";
        default: return "The code was not accepted: check it and try again.";
    }
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::CopyUrl( void )
{GUCEF_TRACE;

    if ( m_url.IsNULLOrEmpty() || !GUI::CSecretInput::WriteClipboardText( m_url ) )
    {
        ShowMessage( "Could not copy: select the address and copy it.", true );
        return false;
    }
    ShowMessage( "Address copied.", false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::OpenUrl( void )
{GUCEF_TRACE;

    if ( !CanOpenUrl() )
    {
        ShowMessage( "Cannot open this address here: copy it into a browser.", true );
        return false;
    }
    if ( !CORE::OpenUrlInSystemBrowser( m_url ) )
    {
        ShowMessage( "The browser did not open: copy the address instead.", true );
        return false;
    }
    ShowMessage( "The page is opening in your browser.", false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::PasteCode( void )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "Code" );
    if ( GUCEF_NULL == field || !m_codeSettings.allowPaste )
        return false;
    CORE::CString text;
    CORE::CString trimmed;
    bool done = GUI::CSecretInput::ReadClipboardText( text );
    if ( done )
    {
        // a code copied from a page comes with the white space and the line end around it
        UInt32 first = 0;
        UInt32 end = text.Length();
        while ( first < end && (UInt8) text[ first ] <= ' ' )
            ++first;
        while ( end > first && (UInt8) text[ end - 1 ] <= ' ' )
            --end;
        trimmed = text.SubstrFromRange( first, end );
        done = field->SetValue( trimmed );
    }
    CORE::SecureWipeString( text );
    CORE::SecureWipeString( trimmed );
    if ( !done )
    {
        field->ClearValue();
        ShowMessage( "Nothing to paste, or the text is too long for a code.", true );
        return false;
    }
    ShowMessage( CORE::CString::Empty, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COAuthCodeEntryForm::Submit( void )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == GetRootWidget() )
        return false;
    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "Code" );
    if ( GUCEF_NULL == field || !field->HasValue() )
    {
        ShowMessage( CheckToText( AUTH::CODECHECK_EMPTY ), true );
        return false;
    }

    // an application without an engine takes the code here
    if ( !NotifyObservers( CodeSubmittedEvent ) )
        return false;      // the form went away in an observer
    if ( !m_open )
        return true;       // the application dealt with it and closed the form

    if ( GUCEF_NULL == m_engine )
    {
        ClearSecrets();
        return true;
    }

    AUTH::CSecretString code;
    ReadCode( code );
    ClearSecrets();
    AUTH::TCodeCheck check = AUTH::CODECHECK_REFUSED;
    m_engine->SubmitAuthorizationCode( code, check );
    code.Clear();
    ShowCheck( check );
    return AUTH::CODECHECK_ACCEPTED == check;
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::Cancel( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    Close();
    if ( wasOpen )
        NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::FinishDone( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    Close();
    if ( wasOpen )
        NotifyObservers( DoneEvent );
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::ShowCheck( AUTH::TCodeCheck check )
{GUCEF_TRACE;

    if ( AUTH::CODECHECK_ACCEPTED == check )
    {
        FinishDone();
        return;
    }
    ShowMessage( CheckToText( check ), true );
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* field = GetWidget( "Code" );
    if ( m_open && GUCEF_NULL != context && GUCEF_NULL != field )
        context->SetFocus( field );
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnSubmitActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Submit();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnCopyActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CopyUrl();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnOpenActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OpenUrl();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnPasteActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    PasteCode();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnCodeTextEntered( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Submit();
}

/*-------------------------------------------------------------------------*/

void
COAuthCodeEntryForm::OnNavigation( CORE::CNotifier* notifier    ,
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

const CORE::CString&
COAuthCodeEntryForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::COAuthCodeEntryForm";
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
