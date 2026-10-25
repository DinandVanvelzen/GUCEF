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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_AUTH_NAMES_H
#include "gucefAUTH_Names.h"
#define GUCEF_AUTH_NAMES_H
#endif /* GUCEF_AUTH_NAMES_H ? */

#include "gucefAUTHGUI_CAccountsPanel.h"

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

const CORE::CEvent CAccountsPanel::ChangedEvent = "GUCEF::AUTHGUI::CAccountsPanel::ChangedEvent";
const CORE::CEvent CAccountsPanel::ErrorChangedEvent = "GUCEF::AUTHGUI::CAccountsPanel::ErrorChangedEvent";
const CORE::CEvent CAccountsPanel::AutoLoginChangedEvent = "GUCEF::AUTHGUI::CAccountsPanel::AutoLoginChangedEvent";
const CORE::CEvent CAccountsPanel::LoginRequestedEvent = "GUCEF::AUTHGUI::CAccountsPanel::LoginRequestedEvent";

const UInt32 CAccountsPanel::MaxShownTextLength;
const UInt32 CAccountsPanel::MaxHintNameLength;

/** The measures of the content, in pixels */
static const Int32 ControlHeight = 38;
static const Int32 EditHeight = 34;
static const Int32 LabelHeight = 22;
static const Int32 CancelWidth = 84;
static const Int32 OpenWidth = 100;
static const Int32 HintHeight = 26;
static const char* const HintTextColour = "150,160,180,255";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAccountsPanel::RegisterEvents( void )
{GUCEF_TRACE;

    ChangedEvent.Initialize();
    ErrorChangedEvent.Initialize();
    AutoLoginChangedEvent.Initialize();
    LoginRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAccountsPanel::CAccountsPanel( void )
    : CORE::CObservingNotifier()
    , m_manager( GUCEF_NULL )
    , m_form( GUCEF_NULL )
    , m_noProvidersText( "No login is set up for this game." )
    , m_progress()
    , m_bindings()
    , m_shownProviders()
    , m_shownUnavailable()
    , m_unavailable()
    , m_error()
    , m_preferDeviceCode( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAccountsPanel::~CAccountsPanel()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

static CORE::CString
CutOff( const CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    if ( text.Length() <= maxLength )
        return text;
    return text.SubstrToIndex( maxLength - 3, true ) + "...";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::MakeAccountText( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    return CutOff( AUTH::GetAccountProviderDisplayName( account.GetProvider() ) + ": " + account.GetSubject(), MaxShownTextLength );
}

/*-------------------------------------------------------------------------*/

Int32
CAccountsPanel::GetRowHeight( TLoginProgress progress, bool hasHint )
{GUCEF_TRACE;

    Int32 height = ControlHeight;
    if ( LOGINPROGRESS_SHOWING_DEVICE_CODE == progress )
        height = ControlHeight + 8 + LabelHeight + 2 + EditHeight + 8 + LabelHeight + 2 + EditHeight;
    return hasHint ? height + 2 + HintHeight : height;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetLoginButtonText( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    CORE::CString text = "Log in with " + AUTH::GetProviderDisplayName( providerName );
    AUTH::CIAuthProvider* provider = GUCEF_NULL != m_manager ? m_manager->GetProvider( providerName ) : GUCEF_NULL;
    CORE::CString name;
    bool running = false;
    if ( GUCEF_NULL != provider && provider->GetDetectedAccountHint( name, running ) && !name.IsNULLOrEmpty() )
        text += " (" + CutOff( name, MaxHintNameLength ) + ")";
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetHintText( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    AUTH::CIAuthProvider* provider = GUCEF_NULL != m_manager ? m_manager->GetProvider( providerName ) : GUCEF_NULL;
    CORE::CString name;
    bool running = false;
    if ( GUCEF_NULL == provider || !provider->GetDetectedAccountHint( name, running ) )
        return CORE::CString::Empty;
    return AUTH::GetProviderDisplayName( providerName ) + ( running ? " is installed and running" : " is installed" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetUnavailableText( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_unavailable.size(); ++i )
    {
        if ( m_unavailable[ i ].providerName == providerName )
            return m_unavailable[ i ].message;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetUnavailableTooltip( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_unavailable.size(); ++i )
    {
        if ( m_unavailable[ i ].providerName == providerName )
            return m_unavailable[ i ].GetTooltipText();
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::GetShownUnavailableProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    providerNames = m_shownUnavailable;
}

/*-------------------------------------------------------------------------*/

CAccountsPanel::SProgress&
CAccountsPanel::GetOrMakeProgress( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_progress[ providerName ];
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::SetNoProvidersText( const CORE::CString& text )
{GUCEF_TRACE;

    m_noProvidersText = text;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccountsPanel::GetNoProvidersText( void ) const
{GUCEF_TRACE;

    return m_noProvidersText;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::AddProviderRow( CORE::CDataNode& list, const CORE::CString& providerName, UInt32 index )
{GUCEF_TRACE;

    const CORE::CString number = CORE::UInt32ToString( index );
    TProgressMap::const_iterator p = m_progress.find( providerName );
    const TLoginProgress progress = p != m_progress.end() ? (*p).second.progress : LOGINPROGRESS_IDLE;
    const CORE::CString hintText = GetHintText( providerName );
    const CORE::CString height = CORE::Int32ToString( GetRowHeight( progress, !hintText.IsNULLOrEmpty() ) );
    const bool cancellable = LOGINPROGRESS_WAITING_FOR_BROWSER == progress || LOGINPROGRESS_SHOWING_DEVICE_CODE == progress;
    const CORE::CString controlHeight = CORE::Int32ToString( ControlHeight );

    CORE::CDataNode* row = GUI::CFormLayout::AddWidgetNode( list, "Panel", "Row" + number, "0,0 0,0", "1,0 0," + height );

    // the button has the whole row, until there is a status to say next to it
    const bool hasStatus = LOGINPROGRESS_IDLE != progress;
    CORE::CDataNode* login = GUI::CFormLayout::AddWidgetNode( *row, "Button", "Login" + number, "0,0 0,0", ( hasStatus ? "0.58,-4 0," : "1,0 0," ) + controlHeight );
    GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_Text, GetLoginButtonText( providerName ) );
    if ( LOGINPROGRESS_WAITING_FOR_BROWSER == progress || LOGINPROGRESS_SHOWING_DEVICE_CODE == progress )
        GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_Enabled, "false" );
    else
        GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_StyleClass, "Primary" );

    CORE::CString status;
    CORE::CString statusColour = "200,205,220,255";
    switch ( progress )
    {
        case LOGINPROGRESS_WAITING_FOR_BROWSER: status = GetWaitingText( providerName ); break;
        case LOGINPROGRESS_SHOWING_DEVICE_CODE: status = "Enter code"; break;
        case LOGINPROGRESS_DONE: status = "Logged in"; statusColour = "140,220,140,255"; break;
        case LOGINPROGRESS_FAILED: status = "Failed"; statusColour = "255,110,110,255"; break;
        default: break;
    }
    if ( hasStatus )
    {
        CORE::CDataNode* statusLabel = GUI::CFormLayout::AddWidgetNode( *row, "Label", "Status" + number, "0.58,6 0,0", ( cancellable ? "0.42," + CORE::Int32ToString( -( CancelWidth + 12 ) ) : CORE::CString( "0.42,-6" ) ) + " 0," + controlHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *statusLabel, GUI::EWidgetProperty_Text, status );
        GUI::CFormLayout::SetWidgetNodeProperty( *statusLabel, GUI::EWidgetProperty_TextColour, statusColour );
        GUI::CFormLayout::SetWidgetNodeProperty( *statusLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
    }
    {
        SBinding binding;
        binding.widgetName = "Login" + number;
        binding.action = ACTION_LOGIN;
        binding.providerName = providerName;
        m_bindings.push_back( binding );
    }

    if ( cancellable )
    {
        CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *row, "Button", "Cancel" + number, "1," + CORE::Int32ToString( -CancelWidth ) + " 0,0", "0," + CORE::Int32ToString( CancelWidth ) + " 0," + controlHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
        SBinding binding;
        binding.widgetName = "Cancel" + number;
        binding.action = ACTION_CANCEL;
        binding.providerName = providerName;
        m_bindings.push_back( binding );
    }

    if ( LOGINPROGRESS_SHOWING_DEVICE_CODE == progress )
    {
        const AUTH::CAuthActionRequired& action = (*p).second.action;
        Int32 y = ControlHeight + 8;
        CORE::CDataNode* urlLabel = GUI::CFormLayout::AddWidgetNode( *row, "Label", "UrlLabel" + number, "0,0 0," + CORE::Int32ToString( y ), "1,0 0," + CORE::Int32ToString( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *urlLabel, GUI::EWidgetProperty_Text, "Go to this page on any device" );
        y += LabelHeight + 2;
        CORE::CDataNode* url = GUI::CFormLayout::AddWidgetNode( *row, "Editbox", "Url" + number, "0,0 0," + CORE::Int32ToString( y ), "1," + CORE::Int32ToString( -( OpenWidth + 8 ) ) + " 0," + CORE::Int32ToString( EditHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_ReadOnly, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_SelectAllOnFocus, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *url, GUI::EWidgetProperty_Text, action.url );
        CORE::CDataNode* open = GUI::CFormLayout::AddWidgetNode( *row, "Button", "Open" + number, "1," + CORE::Int32ToString( -OpenWidth ) + " 0," + CORE::Int32ToString( y ), "0," + CORE::Int32ToString( OpenWidth ) + " 0," + CORE::Int32ToString( EditHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *open, GUI::EWidgetProperty_Text, "Open" );
        y += EditHeight + 8;
        CORE::CDataNode* codeLabel = GUI::CFormLayout::AddWidgetNode( *row, "Label", "CodeLabel" + number, "0,0 0," + CORE::Int32ToString( y ), "1,0 0," + CORE::Int32ToString( LabelHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *codeLabel, GUI::EWidgetProperty_Text, "and enter this code" );
        y += LabelHeight + 2;
        CORE::CDataNode* code = GUI::CFormLayout::AddWidgetNode( *row, "Editbox", "Code" + number, "0,0 0," + CORE::Int32ToString( y ), "1,0 0," + CORE::Int32ToString( EditHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *code, GUI::EWidgetProperty_ReadOnly, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *code, GUI::EWidgetProperty_SelectAllOnFocus, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *code, GUI::EWidgetProperty_Text, action.userCode );
        SBinding binding;
        binding.widgetName = "Open" + number;
        binding.action = ACTION_OPEN_PAGE;
        binding.providerName = providerName;
        m_bindings.push_back( binding );
    }

    // what this machine says about the provider, below everything else of the row
    if ( !hintText.IsNULLOrEmpty() )
    {
        const Int32 hintY = GetRowHeight( progress, false ) + 2;
        CORE::CDataNode* hint = GUI::CFormLayout::AddWidgetNode( *row, "Label", "Hint" + number, "0,0 0," + CORE::Int32ToString( hintY ), "1,0 0," + CORE::Int32ToString( HintHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *hint, GUI::EWidgetProperty_Text, hintText );
        GUI::CFormLayout::SetWidgetNodeProperty( *hint, GUI::EWidgetProperty_TextColour, HintTextColour );
    }
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::AddUnavailableRow( CORE::CDataNode& list, const AUTH::CAuthUnavailableProvider& unavailable, UInt32 index )
{GUCEF_TRACE;

    const CORE::CString number = CORE::UInt32ToString( index );
    const CORE::CString tooltip = unavailable.GetTooltipText();
    const Int32 height = ControlHeight + 2 + HintHeight;
    CORE::CDataNode* row = GUI::CFormLayout::AddWidgetNode( list, "Panel", "UnavailableRow" + number, "0,0 0,0", "1,0 0," + CORE::Int32ToString( height ) );

    // a button that cannot be pressed, and why, and where to put what is missing
    CORE::CDataNode* login = GUI::CFormLayout::AddWidgetNode( *row, "Button", "UnavailableLogin" + number, "0,0 0,0", "1,0 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_Text, "Log in with " + AUTH::GetProviderDisplayName( unavailable.providerName ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_Enabled, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *login, GUI::EWidgetProperty_Tooltip, tooltip );

    CORE::CDataNode* reason = GUI::CFormLayout::AddWidgetNode( *row, "Label", "UnavailableReason" + number, "0,0 0," + CORE::Int32ToString( ControlHeight + 2 ), "1,0 0," + CORE::Int32ToString( HintHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *reason, GUI::EWidgetProperty_Text, unavailable.message );
    GUI::CFormLayout::SetWidgetNodeProperty( *reason, GUI::EWidgetProperty_TextColour, HintTextColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *reason, GUI::EWidgetProperty_Tooltip, tooltip );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::AddAccountRow( CORE::CDataNode& list, const AUTH::CAccountRef& account, UInt32 index )
{GUCEF_TRACE;

    const CORE::CString number = CORE::UInt32ToString( index );
    const CORE::CString controlHeight = CORE::Int32ToString( ControlHeight );
    CORE::CDataNode* row = GUI::CFormLayout::AddWidgetNode( list, "Panel", "AccountRow" + number, "0,0 0,0", "1,0 0," + controlHeight );
    CORE::CDataNode* text = GUI::CFormLayout::AddWidgetNode( *row, "Label", "AccountText" + number, "0,0 0,0", "1," + CORE::Int32ToString( -( OpenWidth + 8 ) ) + " 0," + controlHeight );
    GUI::CFormLayout::SetWidgetNodeProperty( *text, GUI::EWidgetProperty_Text, MakeAccountText( account ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *text, GUI::EWidgetProperty_VTextAlign, "Center" );
    CORE::CDataNode* unlink = GUI::CFormLayout::AddWidgetNode( *row, "Button", "Unlink" + number, "1," + CORE::Int32ToString( -OpenWidth ) + " 0,0", "0," + CORE::Int32ToString( OpenWidth ) + " 0," + controlHeight );
    GUI::CFormLayout::SetWidgetNodeProperty( *unlink, GUI::EWidgetProperty_Text, "Unlink" );
    SBinding binding;
    binding.widgetName = "Unlink" + number;
    binding.action = ACTION_UNLINK;
    binding.account = account;
    m_bindings.push_back( binding );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::AddContent( CORE::CDataNode& list )
{GUCEF_TRACE;

    m_bindings.clear();
    m_shownProviders.clear();
    m_shownUnavailable.clear();
    m_unavailable.clear();
    const CORE::CString headerColour = "150,172,214,255";

    CORE::CDataNode* providersHeader = GUI::CFormLayout::AddWidgetNode( list, "Label", "ProvidersHeader", "0,0 0,0", "1,0 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *providersHeader, GUI::EWidgetProperty_Text, "Log in with" );
    GUI::CFormLayout::SetWidgetNodeProperty( *providersHeader, GUI::EWidgetProperty_TextColour, headerColour );

    CORE::CString::StringVector providers;
    GetOfferedProviders( providers );
    m_shownProviders = providers;
    if ( providers.empty() )
    {
        CORE::CDataNode* none = GUI::CFormLayout::AddWidgetNode( list, "Label", "NoProviders", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *none, GUI::EWidgetProperty_Text, m_noProvidersText );
    }
    for ( size_t i=0; i<providers.size(); ++i )
        AddProviderRow( list, providers[ i ], (UInt32) i );

    // what the manager knows and does not offer: disabled, with the reason
    if ( GUCEF_NULL != m_manager )
        m_manager->GetUnavailableProviders( m_unavailable );
    for ( size_t i=0; i<m_unavailable.size(); ++i )
    {
        AddUnavailableRow( list, m_unavailable[ i ], (UInt32) i );
        m_shownUnavailable.push_back( m_unavailable[ i ].providerName );
    }

    CORE::CDataNode* accountsHeader = GUI::CFormLayout::AddWidgetNode( list, "Label", "AccountsHeader", "0,0 0,0", "1,0 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *accountsHeader, GUI::EWidgetProperty_Text, "Linked accounts" );
    GUI::CFormLayout::SetWidgetNodeProperty( *accountsHeader, GUI::EWidgetProperty_TextColour, headerColour );
    AUTH::TLinkRecordVector links;
    if ( GUCEF_NULL != m_manager )
        m_manager->GetLinkedAccounts( links );
    if ( links.empty() )
    {
        CORE::CDataNode* none = GUI::CFormLayout::AddWidgetNode( list, "Label", "NoAccounts", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *none, GUI::EWidgetProperty_Text, "None. You play as a guest." );
    }
    for ( size_t i=0; i<links.size(); ++i )
        AddAccountRow( list, links[ i ].accountRef, (UInt32) i );

    // what the manager remembers of the names of accounts is the person's to forget: the button is there only while there is something
    if ( HasRememberedLoginHints() )
    {
        CORE::CDataNode* forget = GUI::CFormLayout::AddWidgetNode( list, "Button", "ForgetHints", "0,0 0,0", "1,0 0," + CORE::Int32ToString( ControlHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *forget, GUI::EWidgetProperty_Text, "Forget remembered account names" );
        GUI::CFormLayout::SetWidgetNodeProperty( *forget, GUI::EWidgetProperty_Tooltip, "The names of the accounts you logged in with are kept on this computer to fill in the login page next time" );
        SBinding binding;
        binding.widgetName = "ForgetHints";
        binding.action = ACTION_FORGET_HINTS;
        m_bindings.push_back( binding );
    }
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::Bind( GUI::CForm& form )
{GUCEF_TRACE;

    m_form = &form;
    for ( size_t i=0; i<m_bindings.size(); ++i )
    {
        GUI::CWidget* widget = form.GetWidget( m_bindings[ i ].widgetName );
        if ( GUCEF_NULL != widget )
        {
            TEventCallback activatedCallback( this, &CAccountsPanel::OnWidgetActivated );
            SubscribeTo( widget, GUI::CWidget::ActivatedEvent, activatedCallback );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    if ( manager == m_manager )
        return;
    if ( GUCEF_NULL != m_manager )
        UnsubscribeFrom( m_manager );
    m_manager = manager;
    m_progress.clear();
    if ( GUCEF_NULL != m_manager )
    {
        TEventCallback actionRequiredCallback( this, &CAccountsPanel::OnManagerActionRequired );
        SubscribeTo( m_manager, AUTH::CAuthManager::ActionRequiredEvent, actionRequiredCallback );
        TEventCallback succeededCallback( this, &CAccountsPanel::OnManagerLoginSucceeded );
        SubscribeTo( m_manager, AUTH::CAuthManager::LoginSucceededEvent, succeededCallback );
        TEventCallback failedCallback( this, &CAccountsPanel::OnManagerLoginFailed );
        SubscribeTo( m_manager, AUTH::CAuthManager::LoginFailedEvent, failedCallback );
        TEventCallback stateChangedCallback( this, &CAccountsPanel::OnManagerStateChanged );
        SubscribeTo( m_manager, AUTH::CAuthManager::LoginStateChangedEvent, stateChangedCallback );
        SubscribeTo( m_manager, AUTH::CAuthManager::AccountLinkedEvent, stateChangedCallback );
        SubscribeTo( m_manager, AUTH::CAuthManager::AccountUnlinkedEvent, stateChangedCallback );
        SubscribeTo( m_manager, AUTH::CAuthManager::AccountLoggedOutEvent, stateChangedCallback );
        SubscribeTo( m_manager, AUTH::CAuthManager::AutoLoginFinishedEvent, stateChangedCallback );
        SubscribeTo( m_manager, AUTH::CAuthManager::SettingsChangedEvent, stateChangedCallback );
        // a login that is going on already is waited for
        CORE::CString::StringVector providers;
        m_manager->GetOfferedProviders( providers );
        for ( size_t i=0; i<providers.size(); ++i )
        {
            if ( m_manager->IsLoginPending( providers[ i ] ) )
                GetOrMakeProgress( providers[ i ] ).progress = LOGINPROGRESS_WAITING_FOR_BROWSER;
        }
    }
    NotifyObservers( ChangedEvent );
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthManager*
CAccountsPanel::GetAuthManager( void ) const
{GUCEF_TRACE;

    return m_manager;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::GetOfferedProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    providerNames.clear();
    if ( GUCEF_NULL != m_manager )
        m_manager->GetOfferedProviders( providerNames );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::ShowError( const CORE::CString& text )
{GUCEF_TRACE;

    m_error = text;
    NotifyObservers( ErrorChangedEvent );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetErrorText( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::ClearError( void )
{GUCEF_TRACE;

    if ( !m_error.IsNULLOrEmpty() )
        ShowError( CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::SetPreferDeviceCode( bool preferDeviceCode )
{GUCEF_TRACE;

    m_preferDeviceCode = preferDeviceCode;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::GetPreferDeviceCode( void ) const
{GUCEF_TRACE;

    return m_preferDeviceCode;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::Login( const CORE::CString& providerName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager )
        return false;
    {
        const SProgress& current = GetOrMakeProgress( providerName );
        if ( LOGINPROGRESS_WAITING_FOR_BROWSER == current.progress || LOGINPROGRESS_SHOWING_DEVICE_CODE == current.progress )
            return false;
    }

    ShowError( CORE::CString::Empty );

    // a host that has to prepare for the login does that now, the manager is asked after
    {
        CORE::TCloneableString requested( providerName );
        NotifyObservers( LoginRequestedEvent, &requested );
    }
    if ( GUCEF_NULL == m_manager )
        return false;

    AUTH::CAuthOptions options;
    options.linkToProfile = true;
    options.preferDeviceCode = m_preferDeviceCode;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    SProgress& progress = GetOrMakeProgress( providerName );
    progress.message.Clear();
    progress.action = AUTH::CAuthActionRequired();
    // waiting until the provider says what the player has to do
    progress.progress = LOGINPROGRESS_WAITING_FOR_BROWSER;
    NotifyObservers( ChangedEvent );
    if ( !m_manager->Login( providerName, options, future ) )
    {
        // the manager tells why in its LoginFailedEvent
        SProgress& failed = GetOrMakeProgress( providerName );
        if ( LOGINPROGRESS_FAILED != failed.progress )
        {
            failed.progress = LOGINPROGRESS_FAILED;
            failed.message = "The login could not be started";
            ShowError( AUTH::GetProviderDisplayName( providerName ) + ": " + failed.message );
            NotifyObservers( ChangedEvent );
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::CancelLogin( const CORE::CString& providerName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager )
        return false;
    bool cancelled = m_manager->CancelLogin( providerName );
    SProgress& progress = GetOrMakeProgress( providerName );
    progress.progress = LOGINPROGRESS_IDLE;
    progress.message.Clear();
    progress.action = AUTH::CAuthActionRequired();
    NotifyObservers( ChangedEvent );
    return cancelled;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::Unlink( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager )
        return false;
    bool unlinked = m_manager->Unlink( account );
    NotifyObservers( ChangedEvent );
    return unlinked;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetWaitingText( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    if ( p == m_progress.end() || 0 == (*p).second.minutesToWait )
        return "Waiting...";
    return "Waiting, up to " + CORE::UInt32ToString( (*p).second.minutesToWait ) + " min";
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::HasRememberedLoginHints( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_manager && m_manager->HasLoginHints();
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::ForgetLoginHints( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager )
        return false;
    const bool forgotten = m_manager->ClearLoginHints();
    NotifyObservers( ChangedEvent );
    return forgotten;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::OpenDeviceCodePage( const CORE::CString& providerName )
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    if ( p == m_progress.end() || LOGINPROGRESS_SHOWING_DEVICE_CODE != (*p).second.progress )
        return false;
    const AUTH::CAuthActionRequired& action = (*p).second.action;
    const CORE::CString& url = action.urlWithCode.IsNULLOrEmpty() ? action.url : action.urlWithCode;
    if ( !CORE::OpenUrlInSystemBrowser( url ) )
    {
        ShowError( "The browser could not be opened, go to the page by hand" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::GetAutoLogin( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_manager && m_manager->GetSettings().autoLogin;
}

/*-------------------------------------------------------------------------*/

bool
CAccountsPanel::SetAutoLogin( bool autoLogin )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager || m_manager->GetSettings().autoLogin == autoLogin )
        return false;
    AUTH::CAuthSettings settings( m_manager->GetSettings() );
    settings.autoLogin = autoLogin;
    m_manager->SetSettings( settings );
    NotifyObservers( AutoLoginChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::GetShownProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    providerNames = m_shownProviders;
}

/*-------------------------------------------------------------------------*/

CAccountsPanel::TLoginProgress
CAccountsPanel::GetProgress( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    return p != m_progress.end() ? (*p).second.progress : LOGINPROGRESS_IDLE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetProgressMessage( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    return p != m_progress.end() ? (*p).second.message : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetDeviceCode( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    return p != m_progress.end() && LOGINPROGRESS_SHOWING_DEVICE_CODE == (*p).second.progress ? (*p).second.action.userCode : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetDeviceCodeUrl( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProgressMap::const_iterator p = m_progress.find( providerName );
    return p != m_progress.end() && LOGINPROGRESS_SHOWING_DEVICE_CODE == (*p).second.progress ? (*p).second.action.url : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountsPanel::GetStateText( void ) const
{GUCEF_TRACE;

    AUTH::TLinkRecordVector links;
    if ( GUCEF_NULL == m_manager || !m_manager->GetLinkedAccounts( links ) || links.empty() )
        return "Guest";

    // the providers once each, in the order of the links
    CORE::CString::StringVector names;
    for ( size_t i=0; i<links.size(); ++i )
    {
        CORE::CString name = AUTH::GetAccountProviderDisplayName( links[ i ].accountRef.GetProvider() );
        bool known = false;
        for ( size_t n=0; n<names.size(); ++n )
            known = known || names[ n ] == name;
        if ( !known )
            names.push_back( name );
    }
    CORE::CString text = "Linked: ";
    for ( size_t n=0; n<names.size() && n<3; ++n )
        text += ( 0 == n ? "" : ", " ) + names[ n ];
    if ( names.size() > 3 )
        text += " +" + CORE::UInt32ToString( (UInt32)( names.size() - 3 ) );
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnWidgetActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the widgets of the content come and go with the layout of the host: a binding is found by the widget that notifies
    if ( GUCEF_NULL == m_form )
        return;
    for ( size_t i=0; i<m_bindings.size(); ++i )
    {
        if ( notifier != m_form->GetWidget( m_bindings[ i ].widgetName ) )
            continue;
        // copies: the action can change the bindings
        const SBinding binding = m_bindings[ i ];
        switch ( binding.action )
        {
            case ACTION_LOGIN:      Login( binding.providerName ); break;
            case ACTION_CANCEL:     CancelLogin( binding.providerName ); break;
            case ACTION_OPEN_PAGE:  OpenDeviceCodePage( binding.providerName ); break;
            case ACTION_UNLINK:     Unlink( binding.account ); break;
            case ACTION_FORGET_HINTS: ForgetLoginHints(); break;
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnManagerActionRequired( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
    {
        const AUTH::CAuthActionRequired* action = static_cast< const AUTH::CAuthActionRequired* >( eventData );
        SProgress& progress = GetOrMakeProgress( action->providerName );
        progress.action = *action;
        progress.progress = AUTH::AUTHACTION_SHOW_DEVICE_CODE == action->kind ? LOGINPROGRESS_SHOWING_DEVICE_CODE : LOGINPROGRESS_WAITING_FOR_BROWSER;
        // the time the login waits for the person, said once when the browser is asked for (the label is not made again each minute)
        const UInt64 nowMs = AUTH::GetEpochTimeInMs();
        progress.minutesToWait = action->expiresAtMs > nowMs ? (UInt32) ( ( action->expiresAtMs - nowMs + 59999 ) / 60000 ) : 0;
        progress.message.Clear();
    }
    NotifyObservers( ChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnManagerLoginSucceeded( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
    {
        const AUTH::CAuthResult* result = static_cast< const AUTH::CAuthResult* >( eventData );
        SProgress& progress = GetOrMakeProgress( result->providerName );
        progress.progress = LOGINPROGRESS_DONE;
        progress.action = AUTH::CAuthActionRequired();
        progress.message = result->displayName.IsNULLOrEmpty() ? MakeAccountText( result->accountRef ) : result->displayName;
        ShowError( CORE::CString::Empty );
    }
    NotifyObservers( ChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnManagerLoginFailed( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
    {
        const AUTH::CAuthFailure* failure = static_cast< const AUTH::CAuthFailure* >( eventData );
        SProgress& progress = GetOrMakeProgress( failure->providerName );
        progress.action = AUTH::CAuthActionRequired();
        if ( AUTH::AUTHFAIL_CANCELLED == failure->reason )
        {
            progress.progress = LOGINPROGRESS_IDLE;
            progress.message.Clear();
        }
        else
        {
            progress.progress = LOGINPROGRESS_FAILED;
            progress.message = failure->message.IsNULLOrEmpty() ? CORE::CString( "The login did not work" ) : failure->message;
            ShowError( AUTH::GetProviderDisplayName( failure->providerName ) + ": " + progress.message );
        }
    }
    NotifyObservers( ChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnManagerStateChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( ChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAccountsPanel::OnNotify( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events are the only ones that come here: the events of the manager have a handler each
    if ( CORE::CINotifier::DestructionEvent == eventId && GUCEF_NULL != m_manager && notifier == static_cast< CORE::CNotifier* >( m_manager ) )
    {
        m_manager = GUCEF_NULL;
        NotifyObservers( ChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccountsPanel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::CAccountsPanel";
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
