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

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#include "gucefCORE_CTScopedValue.h"
#define GUCEF_CORE_CTSCOPEDVALUE_H
#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

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

#ifndef GUCEF_GUI_CWINDOW_H
#include "gucefGUI_CWindow.h"
#define GUCEF_GUI_CWINDOW_H
#endif /* GUCEF_GUI_CWINDOW_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_AUTHGUI_UTILS_H
#include "gucefAUTHGUI_Utils.h"
#define GUCEF_AUTHGUI_UTILS_H
#endif /* GUCEF_AUTHGUI_UTILS_H ? */

#include "gucefAUTHGUI_CLoginForm.h"

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

const CORE::CEvent CLoginForm::ClosedEvent = "GUCEF::AUTHGUI::CLoginForm::ClosedEvent";
const CORE::CEvent CLoginForm::AutoLoginChangedEvent = "GUCEF::AUTHGUI::CLoginForm::AutoLoginChangedEvent";

const UInt32 CLoginForm::MaxShownTextLength;

/** The measures of the content, in pixels */
static const Int32 MinWindowWidth = 400;
static const Int32 MinWindowHeight = 380;
static const Int32 MaxWindowWidth = 640;
static const Int32 MaxWindowHeight = 820;
static const Int32 ControlHeight = 38;
static const Int32 StateHeight = 28;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLoginForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
    AutoLoginChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLoginForm::CLoginForm( void )
    : GUI::CFormEx()
    , m_panel()
    , m_movedPosition()
    , m_open( false )
    , m_dirty( true )
    , m_subscribedToFrames( false )
    , m_rebuilding( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeToPanel();
}

/*-------------------------------------------------------------------------*/

CLoginForm::CLoginForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_panel()
    , m_movedPosition()
    , m_open( false )
    , m_dirty( true )
    , m_subscribedToFrames( false )
    , m_rebuilding( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeToPanel();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::SubscribeToPanel( void )
{GUCEF_TRACE;

    TEventCallback errorChangedCallback( this, &CLoginForm::OnPanelErrorChanged );
    SubscribeTo( &m_panel, CAccountsPanel::ErrorChangedEvent, errorChangedCallback );
    TEventCallback changedCallback( this, &CLoginForm::OnPanelChanged );
    SubscribeTo( &m_panel, CAccountsPanel::ChangedEvent, changedCallback );
}

/*-------------------------------------------------------------------------*/

CLoginForm::~CLoginForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::MakeAccountText( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    return CAccountsPanel::MakeAccountText( account );
}

/*-------------------------------------------------------------------------*/

CAccountsPanel&
CLoginForm::GetPanel( void )
{GUCEF_TRACE;

    return m_panel;
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const CORE::CString headerColour = "150,172,214,255";
    const CORE::CString contentTop = CORE::Int32ToString( WindowContentTop );
    const CORE::CString bodyTop = CORE::Int32ToString( WindowContentTop + StateHeight + 8 );

    // nine tenths of the screen up to a size that is plenty, in the middle of it
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "LoginForm", "0.5,0 0.5,0", "0.9,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Log in / accounts" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0," + CORE::Int32ToString( MinWindowWidth ) + " 0," + CORE::Int32ToString( MinWindowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + CORE::Int32ToString( MaxWindowWidth ) + " 0," + CORE::Int32ToString( MaxWindowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* state = GUI::CFormLayout::AddWidgetNode( *window, "Label", "State", "0,20 0," + contentTop, "1,-40 0," + CORE::Int32ToString( StateHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *state, GUI::EWidgetProperty_Text, GetStateText() );

    // everything else scrolls
    CORE::CDataNode* body = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "Body", "0,0 0," + bodyTop, "1,0 1," + CORE::Int32ToString( -( WindowFooterHeight + WindowContentTop + StateHeight + 8 + 8 ) ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_BottomPadding, "12" );
    CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *body, "VBox", "BodyList", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, "16" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "12" );

    m_panel.AddContent( *list );

    CORE::CDataNode* autoLogin = GUI::CFormLayout::AddWidgetNode( *list, "Checkbox", "AutoLogin", "0,0 0,0", "1,0 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *autoLogin, GUI::EWidgetProperty_Text, "Log in when the game starts" );
    GUI::CFormLayout::SetWidgetNodeProperty( *autoLogin, GUI::EWidgetProperty_Checked, GetAutoLogin() ? "true" : "false" );

    // the error and the button: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1," + CORE::Int32ToString( -( WindowFooterHeight + 1 ) ), "1,-2 0," + CORE::Int32ToString( WindowFooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Error", "0,20 0,8", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_TextColour, "255,110,110,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_Text, m_panel.GetErrorText() );
    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Close", "1,-132 1,-52", "0,112 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_StyleClass, "Primary" );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::Rebuild( void )
{GUCEF_TRACE;

    if ( m_rebuilding )
        return true;
    CORE::CTScopedValue< bool > rebuilding( m_rebuilding, true );

    // the window stays where the player put it, folded when it was, scrolled where it was
    bool collapsed = false;
    Int32 scrolled = 0;
    GUI::CWindow* oldWindow = GUI::CWidget::ToWindow( GetRootWidget() );
    if ( GUCEF_NULL != oldWindow )
    {
        if ( oldWindow->IsMovedByUser() )
            m_movedPosition = oldWindow->GetProperty( GUI::EWidgetProperty_Position );
        collapsed = oldWindow->IsCollapsed();
    }
    GUI::CScrollPanel* oldBody = GUI::CWidget::ToScrollPanel( GetWidget( "Body" ) );
    if ( GUCEF_NULL != oldBody )
        scrolled = oldBody->GetScrollPosition();

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    bool loaded = LoadLayout( layout );
    m_dirty = false;

    if ( loaded )
    {
        GUI::CWindow* window = GUI::CWidget::ToWindow( GetRootWidget() );
        if ( GUCEF_NULL != window )
        {
            if ( !m_movedPosition.IsNULLOrEmpty() )
                window->SetProperty( GUI::EWidgetProperty_Position, m_movedPosition );
            if ( collapsed )
                window->SetCollapsed( true );
        }
        if ( m_open )
            Show();
        else
            Hide();
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
CLoginForm::MarkDirty( void )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::Build( void )
{GUCEF_TRACE;

    if ( !Rebuild() )
        return false;
    Hide();
    m_open = false;

    // the content follows the manager and the logins, a frame after it changed
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback frameStartedCallback( this, &CLoginForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, frameStartedCallback );
        m_subscribedToFrames = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
    {
        Build();
        return;
    }
    Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    m_panel.Bind( *this );
    GUI::CWidget* close = GetWidget( "Close" );
    if ( GUCEF_NULL != close )
    {
        TEventCallback activatedCallback( this, &CLoginForm::OnCloseActivated );
        SubscribeTo( close, GUI::CWidget::ActivatedEvent, activatedCallback );
    }
    GUI::CWidget* autoLogin = GetWidget( "AutoLogin" );
    if ( GUCEF_NULL != autoLogin )
    {
        TEventCallback valueChangedCallback( this, &CLoginForm::OnAutoLoginValueChanged );
        SubscribeTo( autoLogin, GUI::CWidget::ValueChangedEvent, valueChangedCallback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback navigationCallback( this, &CLoginForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, navigationCallback );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    m_open = true;
    m_dirty = true;
    return Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::Close( void )
{GUCEF_TRACE;

    bool wasOpen = m_open;
    m_open = false;
    Hide();
    if ( wasOpen )
        NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    m_panel.SetAuthManager( manager );
    MarkDirty();
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthManager*
CLoginForm::GetAuthManager( void ) const
{GUCEF_TRACE;

    return m_panel.GetAuthManager();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::ShowError( const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::GetErrorText( void ) const
{GUCEF_TRACE;

    return m_panel.GetErrorText();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::SetPreferDeviceCode( bool preferDeviceCode )
{GUCEF_TRACE;

    m_panel.SetPreferDeviceCode( preferDeviceCode );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::GetPreferDeviceCode( void ) const
{GUCEF_TRACE;

    return m_panel.GetPreferDeviceCode();
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::Login( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.Login( providerName );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::CancelLogin( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.CancelLogin( providerName );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::Unlink( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    return m_panel.Unlink( account );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::OpenDeviceCodePage( const CORE::CString& providerName )
{GUCEF_TRACE;

    return m_panel.OpenDeviceCodePage( providerName );
}

/*-------------------------------------------------------------------------*/

bool
CLoginForm::GetAutoLogin( void ) const
{GUCEF_TRACE;

    return m_panel.GetAutoLogin();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::SetAutoLogin( bool autoLogin )
{GUCEF_TRACE;

    if ( !m_panel.SetAutoLogin( autoLogin ) )
        return;
    GUI::CWidget* box = GetWidget( "AutoLogin" );
    if ( GUCEF_NULL != box && box->GetBoolProperty( GUI::EWidgetProperty_Checked ) != autoLogin )
        box->SetBoolProperty( GUI::EWidgetProperty_Checked, autoLogin );
    NotifyObservers( AutoLoginChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::GetShownProviders( CORE::CString::StringVector& providerNames ) const
{GUCEF_TRACE;

    m_panel.GetShownProviders( providerNames );
}

/*-------------------------------------------------------------------------*/

CLoginForm::TLoginProgress
CLoginForm::GetProgress( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_panel.GetProgress( providerName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::GetProgressMessage( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_panel.GetProgressMessage( providerName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::GetDeviceCode( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_panel.GetDeviceCode( providerName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::GetDeviceCodeUrl( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_panel.GetDeviceCodeUrl( providerName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoginForm::GetStateText( void ) const
{GUCEF_TRACE;

    return m_panel.GetStateText();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnPanelErrorChanged( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowError( m_panel.GetErrorText() );
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnPanelChanged( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_dirty && m_open && !m_rebuilding )
    {
        // not while the player holds the title bar or the thumb of the scroll bar: the next frame
        GUI::CWindow* window = GUI::CWidget::ToWindow( GetRootWidget() );
        GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( GetWidget( "Body" ) );
        if ( ( GUCEF_NULL == window || !window->IsDragging() ) && ( GUCEF_NULL == body || !body->IsThumbDragged() ) )
            Rebuild();
    }
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnCloseActivated( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnAutoLoginValueChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CWidget* autoLogin = GetWidget( "AutoLogin" );
    if ( !m_rebuilding && GUCEF_NULL != autoLogin )
        SetAutoLogin( autoLogin->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
}

/*-------------------------------------------------------------------------*/

void
CLoginForm::OnNavigation( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Close();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLoginForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHGUI::CLoginForm";
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
