/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#ifndef GUCEF_COM_CMAILSETUPVALIDATION_H
#include "gucefCOM_CMailSetupValidation.h"
#define GUCEF_COM_CMAILSETUPVALIDATION_H
#endif /* GUCEF_COM_CMAILSETUPVALIDATION_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#include "gucefGAMEGUI_CMailAccountForm.h"

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

const CORE::CEvent CMailAccountForm::SaveRequestedEvent = "GUCEF::GAMEGUI::CMailAccountForm::SaveRequestedEvent";
const CORE::CEvent CMailAccountForm::TestRequestedEvent = "GUCEF::GAMEGUI::CMailAccountForm::TestRequestedEvent";
const CORE::CEvent CMailAccountForm::CancelledEvent = "GUCEF::GAMEGUI::CMailAccountForm::CancelledEvent";
const CORE::CEvent CMailAccountForm::SignInRequestedEvent = "GUCEF::GAMEGUI::CMailAccountForm::SignInRequestedEvent";
const CORE::CEvent CMailAccountForm::SignOutRequestedEvent = "GUCEF::GAMEGUI::CMailAccountForm::SignOutRequestedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 LabelHeight = 25;
const Int32 ControlHeight = 30;
const Int32 RowHeight = 60;
const Int32 PageButtonsTop = 44;                    /**< the buttons that switch between the two pages */
const Int32 AccountTop = 88;                        /**< the first row of the account page */
const Int32 ServersTop = 140;                       /**< the first row of the servers page, below its note */
const Int32 PasswordStripBottom = 226;              /**< the password row sits this far above the bottom of the window and shows on both pages */
const Int32 StatusBottom = 164;
const Int32 PresetListLimit = 64;

/** A label above a control: 'x' and 'width' are the first part of a position and of a size ("0.5,10"), the row counts from 'top' */
CORE::CDataNode*
AddField( CORE::CDataNode& parent, const char* type, const char* name, const char* label, const char* x, Int32 top, Int32 row, const char* width )
{
    const CORE::CString labelName = CORE::CString( name ) + "Label";
    const CORE::CString y = "0," + CORE::Int32ToString( top + row * RowHeight );
    const CORE::CString controlY = "0," + CORE::Int32ToString( top + row * RowHeight + LabelHeight + 2 );
    CORE::CDataNode* labelNode = GUI::CFormLayout::AddWidgetNode( parent, "Label", labelName, CORE::CString( x ) + " " + y, CORE::CString( width ) + " 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *labelNode, GUI::EWidgetProperty_Text, label );
    return GUI::CFormLayout::AddWidgetNode( parent, type, name, CORE::CString( x ) + " " + controlY, CORE::CString( width ) + " 0," + CORE::Int32ToString( ControlHeight ) );
}

/** The widgets of the account page and of the servers page: the password row and the status are on both */
const char* const AccountPageWidgets[] = { "AddressLabel", "Address", "DisplayNameLabel", "DisplayName", "ProviderLabel", "Provider", "UserNameLabel", "UserName", "PlayerCodeLabel", "PlayerCode", "AutoAccept", "GuidanceTitle", "Guidance" };
const char* const ServerPageWidgets[] = {
    "ServersNote", "OutgoingHostLabel", "OutgoingHost", "OutgoingPortLabel", "OutgoingPort", "OutgoingSecurityLabel", "OutgoingSecurity", "ProtocolLabel", "Protocol",
    "IncomingHostLabel", "IncomingHost", "IncomingPortLabel", "IncomingPort", "IncomingSecurityLabel", "IncomingSecurity",
    "MailboxLabel", "Mailbox", "PollSecondsLabel", "PollSeconds", "UseIdle", "ProcessedActionLabel", "ProcessedAction", "ProcessedFolderLabel", "ProcessedFolder", "ProcessedNote", "Pop3Delete"
};

Int32
SecurityToIndex( GUCEF::COM::EMailTlsMode mode )
{
    switch ( mode )
    {
        case GUCEF::COM::MAILTLS_IMPLICIT: return 0;
        case GUCEF::COM::MAILTLS_NONE: return 2;
        default: return 1;
    }
}

/** The text of a field */
CORE::CString
TextOf( GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? TrimText( widget->GetProperty( GUI::EWidgetProperty_Text ) ) : CORE::CString::Empty;
}

UInt16
PortOf( GUI::CWidget* widget )
{
    const CORE::CString text = TextOf( widget );
    if ( text.IsNULLOrEmpty() )
        return 0;
    const UInt32 value = CORE::StringToUInt32( text, 0 );
    return value > 65535 ? 0 : (UInt16) value;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailAccountFormData::CMailAccountFormData( void )
    : CORE::CICloneable()
    , address()
    , displayName()
    , userName()
    , password()
    , useImap( true )
    , incomingHost()
    , incomingPort( 0 )
    , incomingSecurity( 0 )
    , outgoingHost()
    , outgoingPort( 0 )
    , outgoingSecurity( 1 )
    , mailbox( "INBOX" )
    , pollSeconds( 60 )
    , useIdle( true )
    , processedAction( 1 )
    , processedFolder()
    , pop3Delete( false )
    , autoAccept( true )
    , providerId()
    , authMode( 0 )
    , oauthProvider()
{GUCEF_TRACE;

}

CMailAccountFormData::CMailAccountFormData( const CMailAccountFormData& src )
    : CORE::CICloneable( src )
    , address( src.address )
    , displayName( src.displayName )
    , userName( src.userName )
    , password( src.password )
    , useImap( src.useImap )
    , incomingHost( src.incomingHost )
    , incomingPort( src.incomingPort )
    , incomingSecurity( src.incomingSecurity )
    , outgoingHost( src.outgoingHost )
    , outgoingPort( src.outgoingPort )
    , outgoingSecurity( src.outgoingSecurity )
    , mailbox( src.mailbox )
    , pollSeconds( src.pollSeconds )
    , useIdle( src.useIdle )
    , processedAction( src.processedAction )
    , processedFolder( src.processedFolder )
    , pop3Delete( src.pop3Delete )
    , autoAccept( src.autoAccept )
    , providerId( src.providerId )
    , authMode( src.authMode )
    , oauthProvider( src.oauthProvider )
{GUCEF_TRACE;

}

CMailAccountFormData::~CMailAccountFormData()
{GUCEF_TRACE;

    CORE::SecureWipeString( password );
}

CMailAccountFormData&
CMailAccountFormData::operator=( const CMailAccountFormData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        address = src.address;
        displayName = src.displayName;
        userName = src.userName;
        CORE::SecureWipeString( password );
        password = src.password;
        useImap = src.useImap;
        incomingHost = src.incomingHost;
        incomingPort = src.incomingPort;
        incomingSecurity = src.incomingSecurity;
        outgoingHost = src.outgoingHost;
        outgoingPort = src.outgoingPort;
        outgoingSecurity = src.outgoingSecurity;
        mailbox = src.mailbox;
        pollSeconds = src.pollSeconds;
        useIdle = src.useIdle;
        processedAction = src.processedAction;
        processedFolder = src.processedFolder;
        pop3Delete = src.pop3Delete;
        autoAccept = src.autoAccept;
        providerId = src.providerId;
        authMode = src.authMode;
        oauthProvider = src.oauthProvider;
    }
    return *this;
}

CORE::CICloneable*
CMailAccountFormData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailAccountFormData( *this );
}

const char*
CMailAccountFormData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

const char*
CMailAccountFormData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CMailAccountFormData";
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::RegisterEvents( void )
{GUCEF_TRACE;

    SaveRequestedEvent.Initialize();
    TestRequestedEvent.Initialize();
    CancelledEvent.Initialize();
    SignInRequestedEvent.Initialize();
    SignOutRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMailAccountForm::CMailAccountForm( void )
    : GUI::CFormEx()
    , m_passwordSettings()
    , m_open( false )
    , m_testing( false )
    , m_presets( GUCEF_NULL )
    , m_defaultPresets()
    , m_oauthProviders()
    , m_providerId( COM::CMailProviderPresets::CustomId )
    , m_providerPicked( false )
    , m_signedIn( false )
    , m_serversPage( false )
    , m_applying( false )
{GUCEF_TRACE;

    RegisterEvents();
}

CMailAccountForm::CMailAccountForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_passwordSettings()
    , m_open( false )
    , m_testing( false )
    , m_presets( GUCEF_NULL )
    , m_defaultPresets()
    , m_oauthProviders()
    , m_providerId( COM::CMailProviderPresets::CustomId )
    , m_providerPicked( false )
    , m_signedIn( false )
    , m_serversPage( false )
    , m_applying( false )
{GUCEF_TRACE;

    RegisterEvents();
}

CMailAccountForm::~CMailAccountForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "MailAccountForm", "0.5,0 0.5,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Mailbox settings" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,640 0,700" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,1000 0,760" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    // the two pages
    CORE::CDataNode* accountPage = GUI::CFormLayout::AddWidgetNode( *window, "Button", "PageAccount", "0,20 0," + CORE::Int32ToString( PageButtonsTop ), "0,200 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *accountPage, GUI::EWidgetProperty_Text, "Account" );
    CORE::CDataNode* serversPage = GUI::CFormLayout::AddWidgetNode( *window, "Button", "PageServers", "0,230 0," + CORE::Int32ToString( PageButtonsTop ), "0,260 0,34" );
    GUI::CFormLayout::SetWidgetNodeProperty( *serversPage, GUI::EWidgetProperty_Text, "Servers and options" );

    // the account page, left: who the player is
    const char* const leftX = "0,20";
    const char* const leftWidth = "0.5,-30";
    AddField( *window, "Editbox", "Address", "Your mail address (type this first)", leftX, AccountTop, 0, leftWidth );
    AddField( *window, "Editbox", "DisplayName", "Your name in the games", leftX, AccountTop, 1, leftWidth );
    AddField( *window, "Combobox", "Provider", "Mail provider (found from the address)", leftX, AccountTop, 2, leftWidth );
    AddField( *window, "Editbox", "UserName", "User name (empty: the address)", leftX, AccountTop, 3, leftWidth );
    AddField( *window, "Editbox", "PlayerCode", "Your player code: give it to those who invite you", leftX, AccountTop, 4, leftWidth );
    CORE::CDataNode* autoAccept = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "AutoAccept", CORE::CString( leftX ) + " 0," + CORE::Int32ToString( AccountTop + 5 * RowHeight + 4 ), CORE::CString( leftWidth ) + " 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *autoAccept, GUI::EWidgetProperty_Text, "Accept invitations automatically" );

    // the account page, right: how this provider signs in
    const char* const rightX = "0.5,10";
    CORE::CDataNode* guidanceTitle = GUI::CFormLayout::AddWidgetNode( *window, "Label", "GuidanceTitle", CORE::CString( rightX ) + " 0," + CORE::Int32ToString( AccountTop ), "0.5,-30 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *guidanceTitle, GUI::EWidgetProperty_Text, "Signing in to your mailbox" );
    CORE::CDataNode* guidance = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Guidance", CORE::CString( rightX ) + " 0," + CORE::Int32ToString( AccountTop + LabelHeight + 4 ), "0.5,-30 0," + CORE::Int32ToString( 5 * RowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *guidance, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* signIn = GUI::CFormLayout::AddWidgetNode( *window, "Button", "SignIn", CORE::CString( rightX ) + " 0," + CORE::Int32ToString( AccountTop + LabelHeight + 4 + 5 * RowHeight + 8 ), "0.5,-30 0,36" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signIn, GUI::EWidgetProperty_StyleClass, "Primary" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signIn, GUI::EWidgetProperty_Visible, "false" );

    // on both pages: the password, or the sign-in that replaces it
    const CORE::CString passwordTop = "1,-" + CORE::Int32ToString( PasswordStripBottom );
    const CORE::CString passwordControlTop = "1,-" + CORE::Int32ToString( PasswordStripBottom - LabelHeight - 2 );
    CORE::CDataNode* passwordLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "PasswordLabel", CORE::CString( leftX ) + " " + passwordTop, CORE::CString( leftWidth ) + " 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *passwordLabel, GUI::EWidgetProperty_Text, "Password or app password (kept encrypted)" );
    GUI::CFormLayout::AddWidgetNode( *window, "SecretInput", "Password", CORE::CString( leftX ) + " " + passwordControlTop, CORE::CString( leftWidth ) + " 0," + CORE::Int32ToString( ControlHeight ) );
    CORE::CDataNode* reveal = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "PasswordReveal", "0.5,-105 " + passwordControlTop, "0,95 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *reveal, GUI::EWidgetProperty_Text, "Show" );
    GUI::CFormLayout::SetWidgetNodeProperty( *reveal, GUI::EWidgetProperty_Visible, "false" );
    // a press on the toggle must leave the focus in the password field: the field hides what it shows when it loses the focus
    GUI::CFormLayout::SetWidgetNodeProperty( *reveal, GUI::EWidgetProperty_Focusable, "false" );
    CORE::CDataNode* signedState = GUI::CFormLayout::AddWidgetNode( *window, "Label", "SignInState", CORE::CString( leftX ) + " " + passwordTop, "0.5,-30 0," + CORE::Int32ToString( LabelHeight + ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *signedState, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signedState, GUI::EWidgetProperty_Visible, "false" );
    CORE::CDataNode* signOut = GUI::CFormLayout::AddWidgetNode( *window, "Button", "SignOut", CORE::CString( rightX ) + " " + passwordControlTop, "0.5,-30 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *signOut, GUI::EWidgetProperty_Text, "Use a password instead" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signOut, GUI::EWidgetProperty_Visible, "false" );

    // the servers page
    CORE::CDataNode* serversNote = GUI::CFormLayout::AddWidgetNode( *window, "Label", "ServersNote", CORE::CString( leftX ) + " 0," + CORE::Int32ToString( AccountTop ), "1,-40 0,48" );
    GUI::CFormLayout::SetWidgetNodeProperty( *serversNote, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *serversNote, GUI::EWidgetProperty_Visible, "false" );
    AddField( *window, "Editbox", "OutgoingHost", "Outgoing server (SMTP)", leftX, ServersTop, 0, leftWidth );
    AddField( *window, "Editbox", "OutgoingPort", "Port (empty: usual)", leftX, ServersTop, 1, "0.22,-10" );
    CORE::CDataNode* outSecurity = AddField( *window, "Combobox", "OutgoingSecurity", "Security", "0.24,10", ServersTop, 1, "0.26,-30" );
    CORE::CDataNode* protocol = AddField( *window, "Combobox", "Protocol", "Receive mail with", leftX, ServersTop, 2, leftWidth );
    AddField( *window, "Editbox", "IncomingHost", "Incoming server", leftX, ServersTop, 3, leftWidth );
    AddField( *window, "Editbox", "IncomingPort", "Port (empty: usual)", leftX, ServersTop, 4, "0.22,-10" );
    CORE::CDataNode* inSecurity = AddField( *window, "Combobox", "IncomingSecurity", "Security", "0.24,10", ServersTop, 4, "0.26,-30" );
    if ( GUCEF_NULL != outSecurity )
    {
        GUI::CFormLayout::SetWidgetNodeProperty( *outSecurity, GUI::EWidgetProperty_Items, "SSL/TLS\nSTARTTLS\nNone (not encrypted)" );
        GUI::CFormLayout::SetWidgetNodeProperty( *outSecurity, GUI::EWidgetProperty_SelectedIndex, "1" );
    }
    if ( GUCEF_NULL != inSecurity )
    {
        GUI::CFormLayout::SetWidgetNodeProperty( *inSecurity, GUI::EWidgetProperty_Items, "SSL/TLS\nSTARTTLS\nNone (not encrypted)" );
        GUI::CFormLayout::SetWidgetNodeProperty( *inSecurity, GUI::EWidgetProperty_SelectedIndex, "0" );
    }
    if ( GUCEF_NULL != protocol )
    {
        GUI::CFormLayout::SetWidgetNodeProperty( *protocol, GUI::EWidgetProperty_Items, "IMAP\nPOP3" );
        GUI::CFormLayout::SetWidgetNodeProperty( *protocol, GUI::EWidgetProperty_SelectedIndex, "0" );
    }
    AddField( *window, "Editbox", "Mailbox", "Mail folder the games read", rightX, ServersTop, 0, "0.5,-30" );
    AddField( *window, "Editbox", "PollSeconds", "Check mail every (s)", rightX, ServersTop, 1, "0.2,-10" );
    CORE::CDataNode* idle = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "UseIdle", "0.72,0 0," + CORE::Int32ToString( ServersTop + RowHeight + LabelHeight + 2 ), "0.28,-30 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *idle, GUI::EWidgetProperty_Text, "Server tells me (IMAP)" );

    // what happens to a game mail once it was handled
    CORE::CDataNode* processed = AddField( *window, "Combobox", "ProcessedAction", "When a game mail has been handled", rightX, ServersTop, 2, "0.5,-30" );
    if ( GUCEF_NULL != processed )
    {
        GUI::CFormLayout::SetWidgetNodeProperty( *processed, GUI::EWidgetProperty_Items, "Leave it in the inbox\nCopy it to the folder below\nMove it to the folder below" );
        GUI::CFormLayout::SetWidgetNodeProperty( *processed, GUI::EWidgetProperty_SelectedIndex, "1" );
    }
    AddField( *window, "Editbox", "ProcessedFolder", "Folder for handled game mail (empty: none)", rightX, ServersTop, 3, "0.5,-30" );
    CORE::CDataNode* note = GUI::CFormLayout::AddWidgetNode( *window, "Label", "ProcessedNote", CORE::CString( rightX ) + " 0," + CORE::Int32ToString( ServersTop + 4 * RowHeight ), "0.5,-30 0,96" );
    GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *note, GUI::EWidgetProperty_Text, "Only mail of Redux games is touched. Move removes it from your inbox into the folder; Copy keeps it in your inbox too. If your server cannot move mail safely, the game copies it and says so." );
    CORE::CDataNode* pop3Delete = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "Pop3Delete", CORE::CString( leftX ) + " 0," + CORE::Int32ToString( ServersTop + 5 * RowHeight + 6 ), "0.5,-30 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *pop3Delete, GUI::EWidgetProperty_Text, "POP3: delete handled game mail from the server" );

    // the strip along the bottom edge
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,20 1,-" + CORE::Int32ToString( StatusBottom ), "1,-40 0,94" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1,-64", "1,-2 0,62" );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* test = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Test", "0,20 0,12", "0,160 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *test, GUI::EWidgetProperty_Text, "Test connection" );
    CORE::CDataNode* save = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Save", "1,-252 0,12", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *save, GUI::EWidgetProperty_Text, "Save" );
    GUI::CFormLayout::SetWidgetNodeProperty( *save, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Cancel", "1,-128 0,12", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CMailAccountForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    FillProviderList();
    const COM::CMailProviderPreset& custom = GetProviderPresets().GetCustomPreset();
    ApplyPreset( custom, false );
    RefreshGuidance();
    ApplyPage();
    Hide();
    m_open = false;
    return true;
}

void
CMailAccountForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* button = GetWidget( "Save" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMailAccountForm::OnSaveActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Test" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMailAccountForm::OnTestActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Cancel" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMailAccountForm::OnCancelActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CMailAccountForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
    button = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback checked( this, &CMailAccountForm::OnRevealChecked );
        SubscribeTo( button, GUI::CCheckbox::BoxCheckedEvent, checked );
        TEventCallback unchecked( this, &CMailAccountForm::OnRevealUnchecked );
        SubscribeTo( button, GUI::CCheckbox::BoxUncheckedEvent, unchecked );
    }
    button = GetWidget( "Password" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback revealChanged( this, &CMailAccountForm::OnPasswordRevealStateChanged );
        SubscribeTo( button, GUI::CSecretInput::RevealStateChangedEvent, revealChanged );
    }
    button = GetWidget( "Address" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback changed( this, &CMailAccountForm::OnAddressChanged );
        SubscribeTo( button, GUI::CEditbox::TextChangedEvent, changed );
    }
    button = GetWidget( "Provider" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback picked( this, &CMailAccountForm::OnProviderPicked );
        SubscribeTo( button, GUI::CWidget::ValueChangedEvent, picked );
    }
    button = GetWidget( "SignIn" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback activated( this, &CMailAccountForm::OnSignInActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, activated );
    }
    button = GetWidget( "SignOut" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback activated( this, &CMailAccountForm::OnSignOutActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, activated );
    }
    button = GetWidget( "PageAccount" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback activated( this, &CMailAccountForm::OnAccountPageActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, activated );
    }
    button = GetWidget( "PageServers" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback activated( this, &CMailAccountForm::OnServersPageActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, activated );
    }
    ApplyPasswordSettings();
    GUI::CWidget* code = GetWidget( "PlayerCode" );
    if ( GUCEF_NULL != code )
        code->SetBoolProperty( GUI::EWidgetProperty_ReadOnly, true );
    GUI::CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL != status )
        status->SetProperty( GUI::EWidgetProperty_TextColour, "200,205,220,255" );
}

/*-------------------------------------------------------------------------*/

bool
CMailAccountForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    m_open = Show();
    if ( m_open && GUCEF_NULL != GetRootWidget() )
        GetRootWidget()->BringToFront();
    return m_open;
}

void
CMailAccountForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
    ClearSecrets();
}

bool
CMailAccountForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::SetData( const CMailAccountFormData& data )
{GUCEF_TRACE;

    {
        CORE::CTScopedValue< bool > applying( m_applying, true );
        FillFromData( data );
    }
    RefreshGuidance();
    ApplyPage();
}

void
CMailAccountForm::FillFromData( const CMailAccountFormData& data )
{GUCEF_TRACE;

    const struct { const char* name; const CORE::CString* text; } fields[] =
    {
        { "Address", &data.address }, { "DisplayName", &data.displayName }, { "UserName", &data.userName },
        { "IncomingHost", &data.incomingHost }, { "OutgoingHost", &data.outgoingHost }, { "Mailbox", &data.mailbox }, { "ProcessedFolder", &data.processedFolder }
    };
    for ( size_t i=0; i<sizeof( fields ) / sizeof( fields[ 0 ] ); ++i )
    {
        GUI::CWidget* field = GetWidget( fields[ i ].name );
        if ( GUCEF_NULL != field )
            field->SetProperty( GUI::EWidgetProperty_Text, *fields[ i ].text );
    }
    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password && !password->SetValue( data.password ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMailAccountForm: The password does not fit the password field (too many characters or not valid text) and was not put in it" );
    GUI::CWidget* field = GetWidget( "IncomingPort" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, 0 == data.incomingPort ? CORE::CString::Empty : CORE::UInt32ToString( data.incomingPort ) );
    field = GetWidget( "OutgoingPort" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, 0 == data.outgoingPort ? CORE::CString::Empty : CORE::UInt32ToString( data.outgoingPort ) );
    field = GetWidget( "PollSeconds" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, CORE::UInt32ToString( data.pollSeconds ) );
    GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Protocol" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( data.useImap ? 0 : 1 );
    choice = GetWidgetAs< GUI::CCombobox >( "IncomingSecurity" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( data.incomingSecurity < 0 || data.incomingSecurity > 2 ? 0 : data.incomingSecurity );
    choice = GetWidgetAs< GUI::CCombobox >( "OutgoingSecurity" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( data.outgoingSecurity < 0 || data.outgoingSecurity > 2 ? 1 : data.outgoingSecurity );
    choice = GetWidgetAs< GUI::CCombobox >( "ProcessedAction" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( data.processedAction < 0 || data.processedAction > 2 ? 1 : data.processedAction );
    field = GetWidget( "Pop3Delete" );
    if ( GUCEF_NULL != field )
        field->SetBoolProperty( GUI::EWidgetProperty_Checked, data.pop3Delete );
    field = GetWidget( "UseIdle" );
    if ( GUCEF_NULL != field )
        field->SetBoolProperty( GUI::EWidgetProperty_Checked, data.useIdle );
    field = GetWidget( "AutoAccept" );
    if ( GUCEF_NULL != field )
        field->SetBoolProperty( GUI::EWidgetProperty_Checked, data.autoAccept );

    // the provider of the data, else the one the address says; the servers of the data stay as they are
    const COM::CMailProviderPreset* preset = GUCEF_NULL;
    if ( !data.providerId.IsNULLOrEmpty() )
        preset = GetProviderPresets().FindById( data.providerId );
    if ( GUCEF_NULL == preset )
        GetProviderPresets().Recognize( data.address, preset );
    m_providerId = preset->id;
    m_providerPicked = false;
    choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( GetProviderIndex( m_providerId ) );
    m_signedIn = 1 == data.authMode;
    GUI::CWidget* state = GetWidget( "SignInState" );
    if ( GUCEF_NULL != state )
        state->SetProperty( GUI::EWidgetProperty_Text, m_signedIn ? CORE::CString( "Signed in with " ) + COM::CMailProviderPresets::GetOAuthDisplayName( data.oauthProvider ) + ". The password is not used for this mailbox." : CORE::CString::Empty );
}

CMailAccountFormData
CMailAccountForm::GetData( void )
{GUCEF_TRACE;

    CMailAccountFormData data;
    data.address = TextOf( GetWidget( "Address" ) );
    data.displayName = TextOf( GetWidget( "DisplayName" ) );
    data.userName = TextOf( GetWidget( "UserName" ) );
    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password )
    {
        // the scoped copy is wiped at the end of this block; the copy in the data is the receiver's
        CORE::CSecureBuffer value = password->GetValue();
        value.CopyToString( data.password );
    }
    data.incomingHost = TextOf( GetWidget( "IncomingHost" ) );
    data.outgoingHost = TextOf( GetWidget( "OutgoingHost" ) );
    data.mailbox = TextOf( GetWidget( "Mailbox" ) );
    data.incomingPort = PortOf( GetWidget( "IncomingPort" ) );
    data.outgoingPort = PortOf( GetWidget( "OutgoingPort" ) );
    const CORE::CString poll = TextOf( GetWidget( "PollSeconds" ) );
    data.pollSeconds = poll.IsNULLOrEmpty() ? 60 : CORE::StringToUInt32( poll, 60 );
    GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Protocol" );
    data.useImap = GUCEF_NULL == choice || choice->GetSelectedIndex() <= 0;
    choice = GetWidgetAs< GUI::CCombobox >( "IncomingSecurity" );
    data.incomingSecurity = GUCEF_NULL != choice && choice->GetSelectedIndex() >= 0 ? choice->GetSelectedIndex() : 0;
    choice = GetWidgetAs< GUI::CCombobox >( "OutgoingSecurity" );
    data.outgoingSecurity = GUCEF_NULL != choice && choice->GetSelectedIndex() >= 0 ? choice->GetSelectedIndex() : 1;
    GUI::CWidget* idle = GetWidget( "UseIdle" );
    data.useIdle = GUCEF_NULL != idle && idle->GetBoolProperty( GUI::EWidgetProperty_Checked );
    choice = GetWidgetAs< GUI::CCombobox >( "ProcessedAction" );
    data.processedAction = GUCEF_NULL != choice && choice->GetSelectedIndex() >= 0 && choice->GetSelectedIndex() <= 2 ? choice->GetSelectedIndex() : 1;
    data.processedFolder = TextOf( GetWidget( "ProcessedFolder" ) );
    GUI::CWidget* pop3Delete = GetWidget( "Pop3Delete" );
    data.pop3Delete = GUCEF_NULL != pop3Delete && pop3Delete->GetBoolProperty( GUI::EWidgetProperty_Checked );
    GUI::CWidget* autoAccept = GetWidget( "AutoAccept" );
    data.autoAccept = GUCEF_NULL == autoAccept || autoAccept->GetBoolProperty( GUI::EWidgetProperty_Checked );
    data.providerId = m_providerId;
    const COM::CMailProviderPreset* preset = GetSelectedPreset();
    data.authMode = m_signedIn ? 1 : 0;
    if ( m_signedIn && GUCEF_NULL != preset )
        data.oauthProvider = preset->oauthProvider;
    return data;
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::SetPlayerCode( const CORE::CString& code )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "PlayerCode" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, code );
}

CORE::CString
CMailAccountForm::GetPlayerCode( void )
{GUCEF_TRACE;

    return TextOf( GetWidget( "PlayerCode" ) );
}

void
CMailAccountForm::SetStatus( const CORE::CString& text, bool problem )
{GUCEF_TRACE;

    GUI::CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL == status )
        return;
    status->SetProperty( GUI::EWidgetProperty_Text, text );
    status->SetProperty( GUI::EWidgetProperty_TextColour, problem ? "255,110,110,255" : "170,225,170,255" );
}

CORE::CString
CMailAccountForm::GetStatus( void )
{GUCEF_TRACE;

    GUI::CWidget* status = GetWidget( "Status" );
    return GUCEF_NULL != status ? status->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMailAccountForm::SetTesting( bool testing )
{GUCEF_TRACE;

    m_testing = testing;
    GUI::CWidget* button = GetWidget( "Test" );
    if ( GUCEF_NULL != button )
    {
        button->SetBoolProperty( GUI::EWidgetProperty_Enabled, !testing );
        button->SetProperty( GUI::EWidgetProperty_Text, testing ? "Testing..." : "Test connection" );
    }
}

bool
CMailAccountForm::IsTesting( void ) const
{GUCEF_TRACE;

    return m_testing;
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::SetPasswordSettings( const GUI::CSecretInputSettings& settings )
{GUCEF_TRACE;

    m_passwordSettings = settings;
    m_passwordSettings.Sanitize();
    ApplyPasswordSettings();
}

const GUI::CSecretInputSettings&
CMailAccountForm::GetPasswordSettings( void ) const
{GUCEF_TRACE;

    return m_passwordSettings;
}

void
CMailAccountForm::ApplyPasswordSettings( void )
{GUCEF_TRACE;

    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password )
    {
        password->SetSettings( m_passwordSettings );
        // the field makes room for the toggle when there is one
        password->SetProperty( GUI::EWidgetProperty_Size, ( m_passwordSettings.allowReveal ? CORE::CString( "0.5,-130 0," ) : CORE::CString( "0.5,-30 0," ) ) + CORE::Int32ToString( ControlHeight ) );
    }
    GUI::CWidget* reveal = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != reveal )
    {
        if ( !m_passwordSettings.allowReveal )
            reveal->SetBoolProperty( GUI::EWidgetProperty_Checked, false );
        reveal->SetVisibility( m_passwordSettings.allowReveal );
    }
    ApplyPage();
}

void
CMailAccountForm::ClearSecrets( void )
{GUCEF_TRACE;

    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password )
    {
        password->SetRevealed( false );
        password->ClearValue();
    }
    GUI::CWidget* reveal = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != reveal )
        reveal->SetBoolProperty( GUI::EWidgetProperty_Checked, false );
}

void
CMailAccountForm::OnRevealChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password && password->SetRevealed( true ) )
    {
        // what is shown is shown while the field has the focus
        password->SetFocus();
        return;
    }
    GUI::CWidget* reveal = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != reveal )
        reveal->SetBoolProperty( GUI::EWidgetProperty_Checked, false );
}

void
CMailAccountForm::OnRevealUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    if ( GUCEF_NULL != password )
        password->SetRevealed( false );
}

void
CMailAccountForm::OnPasswordRevealStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the field hides the password by itself (after a while, when it loses the focus): the toggle follows
    GUI::CSecretInput* password = GetWidgetAs< GUI::CSecretInput >( "Password" );
    GUI::CWidget* reveal = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != password && GUCEF_NULL != reveal )
        reveal->SetBoolProperty( GUI::EWidgetProperty_Checked, password->IsRevealed() );
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::SetProviderPresets( const COM::CMailProviderPresets* presets )
{GUCEF_TRACE;

    m_presets = presets;
    if ( GUCEF_NULL != GetRootWidget() )
    {
        FillProviderList();
        const COM::CMailProviderPreset* preset = GetProviderPresets().FindById( m_providerId );
        if ( GUCEF_NULL == preset )
            preset = &GetProviderPresets().GetCustomPreset();
        m_providerId = preset->id;
        GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
        if ( GUCEF_NULL != choice )
        {
            CORE::CTScopedValue< bool > applying( m_applying, true );
            choice->SetSelectedIndex( GetProviderIndex( m_providerId ) );
        }
        RefreshGuidance();
        ApplyPage();
    }
}

const COM::CMailProviderPresets&
CMailAccountForm::GetProviderPresets( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_presets ? *m_presets : m_defaultPresets;
}

const COM::CMailProviderPreset*
CMailAccountForm::GetSelectedPreset( void ) const
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetProviderPresets().FindById( m_providerId );
    return GUCEF_NULL != preset ? preset : &GetProviderPresets().GetCustomPreset();
}

CORE::CString
CMailAccountForm::GetProviderId( void ) const
{GUCEF_TRACE;

    return m_providerId;
}

void
CMailAccountForm::SetOAuthProviders( const CORE::CString::StringVector& providers )
{GUCEF_TRACE;

    m_oauthProviders = providers;
    RefreshGuidance();
    ApplyPage();
}

bool
CMailAccountForm::IsOAuthAvailable( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    if ( providerName.IsNULLOrEmpty() )
        return false;
    for ( size_t i=0; i<m_oauthProviders.size(); ++i )
        if ( m_oauthProviders[ i ] == providerName )
            return true;
    return false;
}

void
CMailAccountForm::FillProviderList( void )
{GUCEF_TRACE;

    GUI::CWidget* list = GetWidget( "Provider" );
    if ( GUCEF_NULL == list )
        return;
    const COM::CMailProviderPresets& presets = GetProviderPresets();
    CORE::CString items;
    for ( UInt32 i=0; i<presets.GetCount() && (Int32) i < PresetListLimit; ++i )
    {
        if ( i > 0 )
            items += "\n";
        items += presets.GetPreset( i )->displayName;
    }
    if ( GUCEF_NULL == presets.FindById( COM::CMailProviderPresets::CustomId ) )
    {
        if ( !items.IsNULLOrEmpty() )
            items += "\n";
        items += presets.GetCustomPreset().displayName;
    }
    CORE::CTScopedValue< bool > applying( m_applying, true );
    list->SetProperty( GUI::EWidgetProperty_Items, items );
}

Int32
CMailAccountForm::GetProviderIndex( const CORE::CString& id ) const
{GUCEF_TRACE;

    const COM::CMailProviderPresets& presets = GetProviderPresets();
    for ( UInt32 i=0; i<presets.GetCount(); ++i )
        if ( presets.GetPreset( i )->id == id )
            return (Int32) i;
    return (Int32) presets.GetCount();      // the custom entry that the list adds when the table has none
}

bool
CMailAccountForm::SelectProvider( const CORE::CString& id )
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetProviderPresets().FindById( id );
    if ( GUCEF_NULL == preset )
        return false;
    m_providerId = preset->id;
    m_providerPicked = true;
    GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
    if ( GUCEF_NULL != choice )
    {
        CORE::CTScopedValue< bool > applying( m_applying, true );
        choice->SetSelectedIndex( GetProviderIndex( m_providerId ) );
    }
    ApplyPreset( *preset, true );
    RefreshGuidance();
    ApplyPage();
    return true;
}

void
CMailAccountForm::ApplyPreset( const COM::CMailProviderPreset& preset, bool fillServers )
{GUCEF_TRACE;

    GUI::CWidget* note = GetWidget( "ServersNote" );
    if ( GUCEF_NULL != note )
        note->SetProperty( GUI::EWidgetProperty_Text, preset.serverFieldsEditable || !preset.imap.IsSet() ? CORE::CString( "The servers of your mail provider are not known to the game: fill them in as your provider tells you. The security should be SSL/TLS or STARTTLS." )
                                                                                                         : CORE::CString( "Servers filled in for " ) + preset.displayName + ". You only need to change them if your provider told you something else." );
    if ( !fillServers || preset.serverFieldsEditable )
        return;

    CORE::CTScopedValue< bool > applying( m_applying, true );
    const bool imap = preset.imap.IsSet();
    const COM::CMailServerPreset& incoming = imap ? preset.imap : preset.pop3;
    GUI::CWidget* field = GetWidget( "OutgoingHost" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, preset.smtp.host );
    field = GetWidget( "OutgoingPort" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, CORE::UInt32ToString( preset.smtp.GetEffectivePort( COM::MAILPROTOCOL_SMTP ) ) );
    field = GetWidget( "IncomingHost" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, incoming.host );
    field = GetWidget( "IncomingPort" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, CORE::UInt32ToString( incoming.GetEffectivePort( imap ? COM::MAILPROTOCOL_IMAP : COM::MAILPROTOCOL_POP3 ) ) );
    GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "OutgoingSecurity" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( SecurityToIndex( preset.smtp.security ) );
    choice = GetWidgetAs< GUI::CCombobox >( "IncomingSecurity" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( SecurityToIndex( incoming.security ) );
    choice = GetWidgetAs< GUI::CCombobox >( "Protocol" );
    if ( GUCEF_NULL != choice )
        choice->SetSelectedIndex( imap ? 0 : 1 );
}

CORE::CString
CMailAccountForm::GetGuidance( void )
{GUCEF_TRACE;

    GUI::CWidget* guidance = GetWidget( "Guidance" );
    return GUCEF_NULL != guidance ? guidance->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMailAccountForm::RefreshGuidance( void )
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetSelectedPreset();
    const CORE::CString address = TextOf( GetWidget( "Address" ) );
    CORE::CString text;
    if ( address.IsNULLOrEmpty() && !m_providerPicked && COM::CMailProviderPresets::CustomId == preset->id )
        text = "Start with your mail address, for example name@gmail.com. The game then finds your mail provider, fills in its servers and tells you how to sign in to it.";
    else
        text = COM::CMailProviderPresets::DescribeSignIn( *preset, preset->HasOAuth() && IsOAuthAvailable( preset->oauthProvider ) );
    ApplyPreset( *preset, false );
    GUI::CWidget* guidance = GetWidget( "Guidance" );
    if ( GUCEF_NULL != guidance )
        guidance->SetProperty( GUI::EWidgetProperty_Text, text );
    GUI::CWidget* title = GetWidget( "GuidanceTitle" );
    if ( GUCEF_NULL != title )
        title->SetProperty( GUI::EWidgetProperty_Text, CORE::CString( "Signing in to " ) + preset->displayName );
    GUI::CWidget* signIn = GetWidget( "SignIn" );
    if ( GUCEF_NULL != signIn )
        signIn->SetProperty( GUI::EWidgetProperty_Text, GetSignInText() );
}

bool
CMailAccountForm::IsSignInShown( void )
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetSelectedPreset();
    return preset->HasOAuth() && IsOAuthAvailable( preset->oauthProvider );
}

CORE::CString
CMailAccountForm::GetSignInText( void )
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetSelectedPreset();
    if ( !preset->HasOAuth() )
        return CORE::CString::Empty;
    return CORE::CString( m_signedIn ? "Sign in with " : "Sign in with " ) + COM::CMailProviderPresets::GetOAuthDisplayName( preset->oauthProvider ) + ( m_signedIn ? " again" : "" );
}

void
CMailAccountForm::SetSignedIn( bool signedIn, const CORE::CString& text )
{GUCEF_TRACE;

    m_signedIn = signedIn;
    GUI::CWidget* state = GetWidget( "SignInState" );
    if ( GUCEF_NULL != state )
        state->SetProperty( GUI::EWidgetProperty_Text, signedIn ? text : CORE::CString::Empty );
    if ( signedIn )
        ClearSecrets();
    RefreshGuidance();
    ApplyPage();
}

bool
CMailAccountForm::IsSignedIn( void ) const
{GUCEF_TRACE;

    return m_signedIn;
}

void
CMailAccountForm::ShowServersPage( bool servers )
{GUCEF_TRACE;

    m_serversPage = servers;
    ApplyPage();
}

bool
CMailAccountForm::IsServersPageShown( void ) const
{GUCEF_TRACE;

    return m_serversPage;
}

void
CMailAccountForm::ApplyPage( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    for ( size_t i=0; i<sizeof( AccountPageWidgets ) / sizeof( AccountPageWidgets[ 0 ] ); ++i )
    {
        GUI::CWidget* widget = GetWidget( AccountPageWidgets[ i ] );
        if ( GUCEF_NULL != widget )
            widget->SetVisibility( !m_serversPage );
    }
    for ( size_t i=0; i<sizeof( ServerPageWidgets ) / sizeof( ServerPageWidgets[ 0 ] ); ++i )
    {
        GUI::CWidget* widget = GetWidget( ServerPageWidgets[ i ] );
        if ( GUCEF_NULL != widget )
            widget->SetVisibility( m_serversPage );
    }
    GUI::CWidget* widget = GetWidget( "SignIn" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( !m_serversPage && IsSignInShown() );

    // the password row shows on both pages; a mailbox that is signed in to has none
    widget = GetWidget( "PasswordLabel" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( !m_signedIn );
    widget = GetWidget( "Password" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( !m_signedIn );
    widget = GetWidget( "PasswordReveal" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( !m_signedIn && m_passwordSettings.allowReveal );
    widget = GetWidget( "SignInState" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( m_signedIn );
    widget = GetWidget( "SignOut" );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( m_signedIn );

    widget = GetWidget( "PageAccount" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_StyleClass, m_serversPage ? "" : "Primary" );
    widget = GetWidget( "PageServers" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_StyleClass, m_serversPage ? "Primary" : "" );
}

bool
CMailAccountForm::Validate( CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    struct SCheck { const char* name; int kind; const char* what; bool serversPage; };
    static const SCheck checks[] =
    {
        { "Address", 0, "", false }, { "DisplayName", 1, "", false }, { "UserName", 2, "", false },
        { "OutgoingHost", 3, "Outgoing server: ", true }, { "OutgoingPort", 4, "Outgoing server: ", true },
        { "IncomingHost", 3, "Incoming server: ", true }, { "IncomingPort", 4, "Incoming server: ", true },
        { "Mailbox", 5, "", true }, { "ProcessedFolder", 5, "", true }
    };
    for ( size_t i=0; i<sizeof( checks ) / sizeof( checks[ 0 ] ); ++i )
    {
        const CORE::CString text = TextOf( GetWidget( checks[ i ].name ) );
        CORE::CString found;
        bool ok = true;
        UInt16 port = 0;
        switch ( checks[ i ].kind )
        {
            case 0: ok = text.IsNULLOrEmpty() || COM::CMailSetupValidation::CheckAddress( text, found ); break;
            case 1: ok = COM::CMailSetupValidation::CheckDisplayName( text, found ); break;
            case 2: ok = COM::CMailSetupValidation::CheckUserName( text, found ); break;
            case 3: ok = text.IsNULLOrEmpty() || COM::CMailSetupValidation::CheckHost( text, found ); break;
            case 4: ok = COM::CMailSetupValidation::CheckPortText( text, port, found ); break;
            default: ok = COM::CMailSetupValidation::CheckFolderName( text, found ); break;
        }
        if ( !ok )
        {
            problem = CORE::CString( checks[ i ].what ) + found;
            m_serversPage = checks[ i ].serversPage;
            ApplyPage();
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::RequestSignIn( void )
{GUCEF_TRACE;

    const COM::CMailProviderPreset* preset = GetSelectedPreset();
    if ( !IsSignInShown() )
        return;
    CORE::CString problem;
    if ( !Validate( problem ) )
    {
        SetStatus( problem, true );
        return;
    }
    CORE::TCloneableString name( preset->oauthProvider );
    NotifyObservers( SignInRequestedEvent, &name );
}

void
CMailAccountForm::OnAddressChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_applying )
        return;
    const COM::CMailProviderPreset* found = GUCEF_NULL;
    const COM::CMailProviderPresets::ERecognition recognition = GetProviderPresets().Recognize( TextOf( GetWidget( "Address" ) ), found );
    if ( COM::CMailProviderPresets::RECOGNIZE_KNOWN == recognition )
    {
        if ( found->id != m_providerId )
        {
            m_providerId = found->id;
            m_providerPicked = false;
            GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
            if ( GUCEF_NULL != choice )
            {
                CORE::CTScopedValue< bool > applying( m_applying, true );
                choice->SetSelectedIndex( GetProviderIndex( m_providerId ) );
            }
            ApplyPreset( *found, true );
            if ( m_signedIn )
                SetSignedIn( false, CORE::CString::Empty );
        }
    }
    else
    if ( !m_providerPicked && COM::CMailProviderPresets::CustomId != m_providerId )
    {
        m_providerId = COM::CMailProviderPresets::CustomId;
        GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
        if ( GUCEF_NULL != choice )
        {
            CORE::CTScopedValue< bool > applying( m_applying, true );
            choice->SetSelectedIndex( GetProviderIndex( m_providerId ) );
        }
        ApplyPreset( GetProviderPresets().GetCustomPreset(), false );
        if ( m_signedIn )
            SetSignedIn( false, CORE::CString::Empty );
    }
    RefreshGuidance();
    ApplyPage();
}

void
CMailAccountForm::OnProviderPicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_applying )
        return;
    GUI::CCombobox* choice = GetWidgetAs< GUI::CCombobox >( "Provider" );
    if ( GUCEF_NULL == choice )
        return;
    const Int32 index = choice->GetSelectedIndex();
    const COM::CMailProviderPresets& presets = GetProviderPresets();
    const COM::CMailProviderPreset* preset = index >= 0 ? presets.GetPreset( (UInt32) index ) : GUCEF_NULL;
    if ( GUCEF_NULL == preset )
        preset = &presets.GetCustomPreset();
    if ( preset->id != m_providerId && m_signedIn )
        SetSignedIn( false, CORE::CString::Empty );
    SelectProvider( preset->id );
}

void
CMailAccountForm::OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestSignIn();
}

void
CMailAccountForm::OnSignOutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( SignOutRequestedEvent );
}

void
CMailAccountForm::OnAccountPageActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowServersPage( false );
}

void
CMailAccountForm::OnServersPageActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowServersPage( true );
}

/*-------------------------------------------------------------------------*/

void
CMailAccountForm::RequestSave( void )
{GUCEF_TRACE;

    CORE::CString problem;
    if ( !Validate( problem ) )
    {
        SetStatus( problem, true );
        return;
    }
    CMailAccountFormData data = GetData();
    NotifyObservers( SaveRequestedEvent, &data );
}

void
CMailAccountForm::RequestTest( void )
{GUCEF_TRACE;

    if ( m_testing )
        return;
    CORE::CString problem;
    if ( !Validate( problem ) )
    {
        SetStatus( problem, true );
        return;
    }
    CMailAccountFormData data = GetData();
    NotifyObservers( TestRequestedEvent, &data );
}

void
CMailAccountForm::Cancel( void )
{GUCEF_TRACE;

    Close();
    NotifyObservers( CancelledEvent );
}

void
CMailAccountForm::OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestSave();
}

void
CMailAccountForm::OnTestActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestTest();
}

void
CMailAccountForm::OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

void
CMailAccountForm::OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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

const CORE::CString&
CMailAccountForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CMailAccountForm";
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
