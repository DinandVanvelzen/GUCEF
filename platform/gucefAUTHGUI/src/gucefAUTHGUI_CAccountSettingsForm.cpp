/*
 *  gucefAUTHGUI: GUCEF module providing the user interface for logging in with external identities and the account page of settings
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

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#include "gucefCORE_CTScopedValue.h"
#define GUCEF_CORE_CTSCOPEDVALUE_H
#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#include "gucefAUTHGUI_CAccountSettingsForm.h"

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

const CORE::CEvent CAccountSettingsForm::AutoLoginChangedEvent = "GUCEF::AUTHGUI::CAccountSettingsForm::AutoLoginChangedEvent";
const CORE::CEvent CAccountSettingsForm::LoginRequestedEvent = "GUCEF::AUTHGUI::CAccountSettingsForm::LoginRequestedEvent";

const char* const CAccountSettingsForm::NoProvidersText = "No sign-in services are set up in this build. You play as a guest.";

const UInt32 CAccountSettingsForm::MaxDisplayNameLength;

/** The measures of the page, in pixels: the content has the whole tab, up to what is still comfortable to read on a very wide screen */
static const Int32 MaxContentWidth = 1384;
static const Int32 ScrollBarWidth = 16;
static const Int32 NarrowBelow = 768;
static const Int32 ControlHeight = 38;
static const Int32 EditHeight = 34;
static const Int32 LabelHeight = 24;
static const Int32 ErrorHeight = 32;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAccountSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    AutoLoginChangedEvent.Initialize();
    LoginRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAccountSettingsForm::CAccountSettingsForm( void )
    : GUI::CSettingsPageForm()
    , m_panel()
    , m_profile( GUCEF_NULL )
    , m_profileNotifier( GUCEF_NULL )
    , m_errorText()
    , m_dirty( true )
    , m_rebuilding( false )
    , m_narrow( false )
    , m_subscribedToFrames( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_panel.SetNoProvidersText( NoProvidersText );
    TEventCallback errorChangedCallback( this, &CAccountSettingsForm::OnPanelErrorChanged );
    SubscribeTo( &m_panel, CAccountsPanel::ErrorChangedEvent, errorChangedCallback );
    TEventCallback loginRequestedCallback( this, &CAccountSettingsForm::OnPanelLoginRequested );
    SubscribeTo( &m_panel, CAccountsPanel::LoginRequestedEvent, loginRequestedCallback );
    TEventCallback changedCallback( this, &CAccountSettingsForm::OnPanelChanged );
    SubscribeTo( &m_panel, CAccountsPanel::ChangedEvent, changedCallback );
}

/*-------------------------------------------------------------------------*/

CAccountSettingsForm::~CAccountSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return "Account";
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    m_panel.SetAuthManager( manager );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthManager*
CAccountSettingsForm::GetAuthManager( void ) const
{GUCEF_TRACE;

    return m_panel.GetAuthManager();
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::SetProfile( AUTH::IAccountProfile* profile )
{GUCEF_TRACE;

    if ( profile == m_profile )
        return;
    if ( GUCEF_NULL != m_profileNotifier )
        UnsubscribeFrom( m_profileNotifier );
    m_profile = profile;
    m_profileNotifier = GUCEF_NULL != m_profile ? m_profile->GetProfileNotifier() : GUCEF_NULL;
    if ( GUCEF_NULL != m_profileNotifier )
    {
        TEventCallback profileChangedCallback( this, &CAccountSettingsForm::OnProfileChanged );
        SubscribeTo( m_profileNotifier, m_profile->GetProfileChangedEvent(), profileChangedCallback );
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

AUTH::IAccountProfile*
CAccountSettingsForm::GetProfile( void ) const
{GUCEF_TRACE;

    return m_profile;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::SetPreferDeviceCode( bool preferDeviceCode )
{GUCEF_TRACE;

    m_panel.SetPreferDeviceCode( preferDeviceCode );
}

/*-------------------------------------------------------------------------*/

CAccountsPanel&
CAccountSettingsForm::GetPanel( void )
{GUCEF_TRACE;

    return m_panel;
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::IsNarrow( void )
{GUCEF_TRACE;

    // the page is as wide as the settings window lets it be: before it is laid out it is taken to be wide
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return m_narrow;
    const Int32 width = root->GetRect().width;
    return width > 0 ? width < NarrowBelow : m_narrow;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    m_dirty = false;
    m_narrow = IsNarrow();
    const CORE::CString headerColour = "150,172,214,255";
    const CORE::CString editHeight = CORE::Int32ToString( EditHeight );
    const CORE::CString labelHeight = CORE::Int32ToString( LabelHeight );

    CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "AccountSettings", "0,0 0,0", "1,0 1,0" );

    // the content in a column that is as wide as the tab (a very wide screen has it as wide as is useful, in the middle), with the scroll bar at its edge; the scroll panel is the
    // whole page, so the wheel and the keys scroll it also over the margins at both sides of the column
    CORE::CDataNode* body = GUI::CFormLayout::AddWidgetNode( *page, "ScrollPanel", "Body", "0,0 0,0", "1,0 1," + CORE::Int32ToString( -ErrorHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_BottomPadding, "12" );
    GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_ScrollBarWidth, CORE::Int32ToString( ScrollBarWidth ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_MaxColumnWidth, CORE::Int32ToString( MaxContentWidth + ScrollBarWidth ) );
    CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *body, "VBox", "BodyList", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, m_narrow ? "12" : "16" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "12" );

    if ( GUCEF_NULL != m_profile )
    {
        CORE::CDataNode* profileHeader = GUI::CFormLayout::AddWidgetNode( *list, "Label", "ProfileHeader", "0,0 0,0", "1,0 0," + labelHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *profileHeader, GUI::EWidgetProperty_Text, "Profile" );
        GUI::CFormLayout::SetWidgetNodeProperty( *profileHeader, GUI::EWidgetProperty_TextColour, headerColour );
        CORE::CDataNode* nameLabel = GUI::CFormLayout::AddWidgetNode( *list, "Label", "NameLabel", "0,0 0,0", "1,0 0," + labelHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *nameLabel, GUI::EWidgetProperty_Text, "The name shown for you" );
        CORE::CDataNode* name = GUI::CFormLayout::AddWidgetNode( *list, "Editbox", "DisplayName", "0,0 0,0", "1,0 0," + editHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *name, GUI::EWidgetProperty_MaxLength, CORE::UInt32ToString( MaxDisplayNameLength ) );
        CORE::CDataNode* idLabel = GUI::CFormLayout::AddWidgetNode( *list, "Label", "ProfileIdLabel", "0,0 0,0", "1,0 0," + labelHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *idLabel, GUI::EWidgetProperty_Text, "The id of this profile" );
        CORE::CDataNode* id = GUI::CFormLayout::AddWidgetNode( *list, "Editbox", "ProfileId", "0,0 0,0", "1,0 0," + editHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *id, GUI::EWidgetProperty_ReadOnly, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *id, GUI::EWidgetProperty_SelectAllOnFocus, "true" );
    }

    CORE::CDataNode* stateHeader = GUI::CFormLayout::AddWidgetNode( *list, "Label", "StateHeader", "0,0 0,0", "1,0 0," + labelHeight );
    GUI::CFormLayout::SetWidgetNodeProperty( *stateHeader, GUI::EWidgetProperty_Text, "Status" );
    GUI::CFormLayout::SetWidgetNodeProperty( *stateHeader, GUI::EWidgetProperty_TextColour, headerColour );
    CORE::CDataNode* state = GUI::CFormLayout::AddWidgetNode( *list, "Label", "State", "0,0 0,0", "1,0 0," + labelHeight );
    GUI::CFormLayout::SetWidgetNodeProperty( *state, GUI::EWidgetProperty_Text, m_panel.GetStateText() );

    m_panel.AddContent( *list );

    CORE::CString::StringVector offered;
    m_panel.GetOfferedProviders( offered );
    if ( !offered.empty() )
    {
        CORE::CDataNode* autoLogin = GUI::CFormLayout::AddWidgetNode( *list, "Checkbox", "AutoLogin", "0,0 0,0", "1,0 0," + CORE::Int32ToString( ControlHeight ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *autoLogin, GUI::EWidgetProperty_Text, "Log in when the game starts" );
    }

    // what went wrong, in the strip along the bottom edge
    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *page, "Label", "Error", "0,24 1," + CORE::Int32ToString( -ErrorHeight ), "1,-48 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_TextColour, "255,110,110,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_Text, m_errorText );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CSettingsPageForm::OnPostLayoutLoad();
    m_panel.Bind( *this );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::ShowValues( void )
{GUCEF_TRACE;

    GUI::CWidget* name = GetWidget( "DisplayName" );
    if ( GUCEF_NULL != name && GUCEF_NULL != m_profile )
        name->SetProperty( GUI::EWidgetProperty_Text, m_profile->GetDisplayName() );
    GUI::CWidget* id = GetWidget( "ProfileId" );
    if ( GUCEF_NULL != id && GUCEF_NULL != m_profile )
        id->SetProperty( GUI::EWidgetProperty_Text, m_profile->GetProfileId() );
    GUI::CWidget* autoLogin = GetWidget( "AutoLogin" );
    if ( GUCEF_NULL != autoLogin )
        autoLogin->SetBoolProperty( GUI::EWidgetProperty_Checked, m_panel.GetAutoLogin() );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::Rebuild( bool keepChanges )
{GUCEF_TRACE;

    if ( m_rebuilding || GUCEF_NULL == GetRootWidget() )
        return false;
    CORE::CTScopedValue< bool > rebuilding( m_rebuilding, true );

    // what the person typed and ticked stays, and where the page was scrolled to
    const bool keep = keepChanges && HasChanges();
    CORE::CString typedName;
    bool typedAutoLogin = false;
    const bool hadName = keep && GUCEF_NULL != GetWidget( "DisplayName" );
    const bool hadAutoLogin = keep && GUCEF_NULL != GetWidget( "AutoLogin" );
    if ( hadName )
        typedName = GetWidget( "DisplayName" )->GetProperty( GUI::EWidgetProperty_Text );
    if ( hadAutoLogin )
        typedAutoLogin = GetWidget( "AutoLogin" )->GetBoolProperty( GUI::EWidgetProperty_Checked );
    Int32 scrolled = 0;
    GUI::CScrollPanel* oldBody = GUI::CWidget::ToScrollPanel( GetWidget( "Body" ) );
    if ( GUCEF_NULL != oldBody )
        scrolled = oldBody->GetScrollPosition();
    const bool changes = HasChanges();

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    const bool loaded = LoadLayout( layout );
    if ( loaded )
    {
        {
            CReadingSettingsScope readingSettings( *this );
            ShowValues();
            GUI::CWidget* name = GetWidget( "DisplayName" );
            if ( hadName && GUCEF_NULL != name )
                name->SetProperty( GUI::EWidgetProperty_Text, typedName );
            GUI::CWidget* autoLogin = GetWidget( "AutoLogin" );
            if ( hadAutoLogin && GUCEF_NULL != autoLogin )
                autoLogin->SetBoolProperty( GUI::EWidgetProperty_Checked, typedAutoLogin );
        }
        SetHasChanges( keep ? changes : false );
        GUI::CGUIContext* context = GetGUIContext();
        if ( GUCEF_NULL != context )
            context->FlushChanges();
        GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( GetWidget( "Body" ) );
        if ( GUCEF_NULL != body && scrolled > 0 )
            body->SetScrollPosition( scrolled );
    }
    return loaded;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::Refresh( bool keepChanges )
{GUCEF_TRACE;

    Rebuild( keepChanges );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::BuildPage( GUI::CWidget* parent )
{GUCEF_TRACE;

    if ( !GUI::CSettingsPageForm::BuildPage( parent ) )
        return false;

    // the page follows the manager, the profile and the width it has, a frame after they changed
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback frameStartedCallback( this, &CAccountSettingsForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, frameStartedCallback );
        m_subscribedToFrames = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    // the manager has offered providers since the page was made, the settings are what they were: the page is made again
    m_errorText.Clear();
    m_panel.ClearError();
    Rebuild( false );
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::ShowError( const CORE::CString& text )
{GUCEF_TRACE;

    m_errorText = text;
    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    ShowError( CORE::CString::Empty );
    if ( GUCEF_NULL != m_profile && GUCEF_NULL != GetWidget( "DisplayName" ) )
    {
        const CORE::CString name = GetDisplayNameText().Trim( true ).Trim( false );
        if ( name.IsNULLOrEmpty() )
        {
            ShowError( "Enter a name" );
            return false;
        }
        if ( name != m_profile->GetDisplayName() && !m_profile->SetDisplayName( name ) )
        {
            ShowError( "The name could not be saved" );
            return false;
        }
    }
    GUI::CWidget* autoLogin = GetWidget( "AutoLogin" );
    if ( GUCEF_NULL != autoLogin && m_panel.SetAutoLogin( autoLogin->GetBoolProperty( GUI::EWidgetProperty_Checked ) ) )
        NotifyObservers( AutoLoginChangedEvent );

    // the profile tells that it changed: what the page shows is made again at the next frame, the name now
    {
        CReadingSettingsScope readingSettings( *this );
        ShowValues();
    }
    SetHasChanges( false );
    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountSettingsForm::GetDisplayNameText( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "DisplayName", GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::SetDisplayNameText( const CORE::CString& name )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "DisplayName" );
    return GUCEF_NULL != field && field->ChangeValueByUser( GUI::EWidgetProperty_Text, name );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountSettingsForm::GetProfileIdText( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "ProfileId", GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::HasProfileFields( void )
{GUCEF_TRACE;

    return GUCEF_NULL != GetWidget( "DisplayName" ) && GUCEF_NULL != GetWidget( "ProfileId" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountSettingsForm::GetStateText( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "State", GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAccountSettingsForm::GetErrorText( void ) const
{GUCEF_TRACE;

    return m_errorText;
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::HasAutoLoginBox( void )
{GUCEF_TRACE;

    return GUCEF_NULL != GetWidget( "AutoLogin" );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::GetAutoLogin( void )
{GUCEF_TRACE;

    GUI::CWidget* box = GetWidget( "AutoLogin" );
    return GUCEF_NULL != box && box->GetBoolProperty( GUI::EWidgetProperty_Checked );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::SetAutoLogin( bool autoLogin )
{GUCEF_TRACE;

    GUI::CWidget* box = GetWidget( "AutoLogin" );
    return GUCEF_NULL != box && box->ChangeValueByUser( GUI::EWidgetProperty_Checked, autoLogin ? "true" : "false" );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::Login( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.Login( providerName );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::CancelLogin( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.CancelLogin( providerName );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::Unlink( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    return m_panel.Unlink( account );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::HasForgetLoginHintsButton( void )
{GUCEF_TRACE;

    return GUCEF_NULL != GetWidget( "ForgetHints" );
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::ForgetLoginHints( void )
{GUCEF_TRACE;

    return m_panel.ForgetLoginHints();
}

/*-------------------------------------------------------------------------*/

bool
CAccountSettingsForm::OpenDeviceCodePage( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.OpenDeviceCodePage( providerName );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::GetShownProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    m_panel.GetShownProviders( providerNames );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::GetShownUnavailableProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    m_panel.GetShownUnavailableProviders( providerNames );
}

/*-------------------------------------------------------------------------*/

CAccountsPanel::TLoginProgress
CAccountSettingsForm::GetProgress( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_panel.GetProgress( providerName );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnPanelErrorChanged( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowError( m_panel.GetErrorText() );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnPanelLoginRequested( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( LoginRequestedEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnPanelChanged( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnProfileChanged( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // not while the player holds the thumb of the scroll bar: the next frame
    GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( GetWidget( "Body" ) );
    if ( !m_rebuilding && GUCEF_NULL != GetRootWidget() && ( m_dirty || IsNarrow() != m_narrow ) && ( GUCEF_NULL == body || !body->IsThumbDragged() ) )
        Rebuild( true );
}

/*-------------------------------------------------------------------------*/

void
CAccountSettingsForm::OnNotify( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events are the only ones that come here: every other event has a handler of its own
    if ( CORE::CINotifier::DestructionEvent == eventId && GUCEF_NULL != m_profileNotifier && notifier == m_profileNotifier )
    {
        m_profile = GUCEF_NULL;
        m_profileNotifier = GUCEF_NULL;
        m_dirty = true;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccountSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::CAccountSettingsForm";
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
