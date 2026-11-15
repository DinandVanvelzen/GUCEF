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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#include "gucefWEB_CWebViewEngineBase.h"
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#include "gucefWEB_CWebViewHostAllowList.h"
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWLOADERROR_H
#include "gucefWEB_CWebViewLoadError.h"
#define GUCEF_WEB_CWEBVIEWLOADERROR_H
#endif /* GUCEF_WEB_CWEBVIEWLOADERROR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#include "gucefWEB_CWebViewPopupRequest.h"
#define GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#include "gucefWEB_CWebViewDownloadRequest.h"
#define GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#include "gucefWEB_CWebViewPermissionRequest.h"
#define GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#include "gucefWEB_CWebViewCertificateError.h"
#define GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#endif /* GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#include "gucefWEB_CWebViewScriptDialog.h"
#define GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#endif /* GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CWEBPAGEVIEW_H
#include "gucefGUI_CWebPageView.h"
#define GUCEF_GUI_CWEBPAGEVIEW_H
#endif /* GUCEF_GUI_CWEBPAGEVIEW_H ? */

#include "gucefGUI_CWebViewForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWebViewForm::TitleChangedEvent = "GUCEF::GUI::CWebViewForm::TitleChangedEvent";
const CORE::CEvent CWebViewForm::UrlChangedEvent = "GUCEF::GUI::CWebViewForm::UrlChangedEvent";
const CORE::CEvent CWebViewForm::LoadStateChangedEvent = "GUCEF::GUI::CWebViewForm::LoadStateChangedEvent";
const CORE::CEvent CWebViewForm::LoadFailedEvent = "GUCEF::GUI::CWebViewForm::LoadFailedEvent";
const CORE::CEvent CWebViewForm::PopupRequestedEvent = "GUCEF::GUI::CWebViewForm::PopupRequestedEvent";
const CORE::CEvent CWebViewForm::DownloadRequestedEvent = "GUCEF::GUI::CWebViewForm::DownloadRequestedEvent";
const CORE::CEvent CWebViewForm::PermissionRequestedEvent = "GUCEF::GUI::CWebViewForm::PermissionRequestedEvent";
const CORE::CEvent CWebViewForm::CertificateErrorEvent = "GUCEF::GUI::CWebViewForm::CertificateErrorEvent";
const CORE::CEvent CWebViewForm::ScriptDialogEvent = "GUCEF::GUI::CWebViewForm::ScriptDialogEvent";
const CORE::CEvent CWebViewForm::ConsoleMessageEvent = "GUCEF::GUI::CWebViewForm::ConsoleMessageEvent";
const CORE::CEvent CWebViewForm::CloseRequestedEvent = "GUCEF::GUI::CWebViewForm::CloseRequestedEvent";
const CORE::CEvent CWebViewForm::CancelledEvent = "GUCEF::GUI::CWebViewForm::CancelledEvent";

const Int32 CWebViewForm::NavigationBarHeight;
const Int32 CWebViewForm::StatusBarHeight;
const Int32 CWebViewForm::DialogButtonsHeight;
const Int32 CWebViewForm::WindowTitleHeight;

namespace {

const char* SecurityImageAliases[ 4 ] = { "WebViewForm.Security.None", "WebViewForm.Security.Secure", "WebViewForm.Security.Insecure", "WebViewForm.Security.CertificateError" };
const char* SecurityTooltips[ 4 ] = { "", "Secure connection", "Not secure", "Certificate error: the connection is not trusted" };
const UInt8 SecurityColours[ 4 ][ 3 ] = { { 120, 120, 128 }, { 40, 170, 70 }, { 230, 150, 30 }, { 210, 40, 40 } };
const Int32 SecurityMarkSize = 20;

CString
Px( Int32 value )
{
    return CORE::Int32ToString( value );
}

/** The address without its query and fragment: what may be shown where codes and states could be read */
CString
WithoutQuery( const CString& url )
{
    Int32 end = url.HasChar( '?' );
    const Int32 fragment = url.HasChar( '#' );
    if ( fragment >= 0 && ( end < 0 || fragment < end ) )
        end = fragment;
    return end >= 0 ? url.SubstrToIndex( (UInt32) end, true ) : url;
}

CString
HostOf( const CString& url )
{
    CString scheme, host;
    UInt16 port = 0;
    bool userInfo = false;
    if ( WEB::CWebViewHostAllowList::SplitUrl( url, scheme, host, port, userInfo ) )
        return host;
    return url;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebViewForm::RegisterEvents( void )
{GUCEF_TRACE;

    TitleChangedEvent.Initialize();
    UrlChangedEvent.Initialize();
    LoadStateChangedEvent.Initialize();
    LoadFailedEvent.Initialize();
    PopupRequestedEvent.Initialize();
    DownloadRequestedEvent.Initialize();
    PermissionRequestedEvent.Initialize();
    CertificateErrorEvent.Initialize();
    ScriptDialogEvent.Initialize();
    ConsoleMessageEvent.Initialize();
    CloseRequestedEvent.Initialize();
    CancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebViewForm::CWebViewForm( void )
    : CFormEx()
    , m_relay()
    , m_view()
    , m_showNavigationBar( true )
    , m_showStatusBar( true )
    , m_showDialogButtons( false )
    , m_addressEditable( true )
    , m_focusPageOnShow( true )
    , m_followTextScale( true )
    , m_prompt()
    , m_statusText()
    , m_failedUrl()
    , m_loading( false )
    , m_frameSinceLoad( false )
    , m_errorShown( false )
    , m_frameSubscribed( false )
    , m_laidOutWidth( -1 )
    , m_answered()
    , m_prompts()
    , m_promptForm( GUCEF_NULL )
    , m_promptOpen( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeRelay();
}

/*-------------------------------------------------------------------------*/

CWebViewForm::CWebViewForm( CIGUIContext* context )
    : CFormEx( context )
    , m_relay()
    , m_view()
    , m_showNavigationBar( true )
    , m_showStatusBar( true )
    , m_showDialogButtons( false )
    , m_addressEditable( true )
    , m_focusPageOnShow( true )
    , m_followTextScale( true )
    , m_prompt()
    , m_statusText()
    , m_failedUrl()
    , m_loading( false )
    , m_frameSinceLoad( false )
    , m_errorShown( false )
    , m_frameSubscribed( false )
    , m_laidOutWidth( -1 )
    , m_answered()
    , m_prompts()
    , m_promptForm( GUCEF_NULL )
    , m_promptOpen( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeRelay();
}

/*-------------------------------------------------------------------------*/

CWebViewForm::~CWebViewForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Detach();
    GUCEF_DELETE m_promptForm;
    m_promptForm = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::SubscribeRelay( void )
{GUCEF_TRACE;

    WEB::IWebView::RegisterEvents();
    TEventCallback started( this, &CWebViewForm::OnViewLoadStarted );
    SubscribeTo( &m_relay, WEB::IWebView::LoadStartedEvent, started );
    TEventCallback committed( this, &CWebViewForm::OnViewLoadCommitted );
    SubscribeTo( &m_relay, WEB::IWebView::LoadCommittedEvent, committed );
    TEventCallback finished( this, &CWebViewForm::OnViewLoadFinished );
    SubscribeTo( &m_relay, WEB::IWebView::LoadFinishedEvent, finished );
    TEventCallback failed( this, &CWebViewForm::OnViewLoadFailed );
    SubscribeTo( &m_relay, WEB::IWebView::LoadFailedEvent, failed );
    TEventCallback title( this, &CWebViewForm::OnViewTitleChanged );
    SubscribeTo( &m_relay, WEB::IWebView::TitleChangedEvent, title );
    TEventCallback url( this, &CWebViewForm::OnViewUrlChanged );
    SubscribeTo( &m_relay, WEB::IWebView::UrlChangedEvent, url );
    TEventCallback progress( this, &CWebViewForm::OnViewProgressChanged );
    SubscribeTo( &m_relay, WEB::IWebView::ProgressChangedEvent, progress );
    TEventCallback security( this, &CWebViewForm::OnViewSecurityChanged );
    SubscribeTo( &m_relay, WEB::IWebView::SecurityStateChangedEvent, security );
    TEventCallback frame( this, &CWebViewForm::OnViewFrameAvailable );
    SubscribeTo( &m_relay, WEB::IWebView::FrameAvailableEvent, frame );
    TEventCallback popup( this, &CWebViewForm::OnViewDecisionEvent );
    SubscribeTo( &m_relay, WEB::IWebView::PopupRequestedEvent, popup );
    TEventCallback download( this, &CWebViewForm::OnViewDecisionEvent );
    SubscribeTo( &m_relay, WEB::IWebView::DownloadRequestedEvent, download );
    TEventCallback permission( this, &CWebViewForm::OnViewDecisionEvent );
    SubscribeTo( &m_relay, WEB::IWebView::PermissionRequestedEvent, permission );
    TEventCallback certificate( this, &CWebViewForm::OnViewDecisionEvent );
    SubscribeTo( &m_relay, WEB::IWebView::CertificateErrorEvent, certificate );
    TEventCallback dialog( this, &CWebViewForm::OnViewDecisionEvent );
    SubscribeTo( &m_relay, WEB::IWebView::ScriptDialogEvent, dialog );
    TEventCallback console( this, &CWebViewForm::OnViewConsoleMessage );
    SubscribeTo( &m_relay, WEB::IWebView::ConsoleMessageEvent, console );
    TEventCallback close( this, &CWebViewForm::OnViewCloseRequested );
    SubscribeTo( &m_relay, WEB::IWebView::CloseRequestedEvent, close );
    TEventCallback focusLeave( this, &CWebViewForm::OnViewFocusLeave );
    SubscribeTo( &m_relay, WEB::IWebView::FocusLeaveRequestedEvent, focusLeave );
    TEventCallback stalled( this, &CWebViewForm::OnViewEnded );
    SubscribeTo( &m_relay, WEB::IWebView::EngineStalledEvent, stalled );
    TEventCallback terminated( this, &CWebViewForm::OnViewEnded );
    SubscribeTo( &m_relay, WEB::IWebView::ProcessTerminatedEvent, terminated );
    TEventCallback closed( this, &CWebViewForm::OnViewEnded );
    SubscribeTo( &m_relay, WEB::IWebView::ClosedEvent, closed );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "WebViewForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, Px( WindowTitleHeight ) );
        // never smaller than the page needs and the chrome around it
        const Int32 minHeight = CWebPageView::MinimumHeight + WindowTitleHeight + NavigationBarHeight + StatusBarHeight + DialogButtonsHeight + 48;
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_MinSize, "0," + Px( CWebPageView::MinimumWidth + 16 ) + " 0," + Px( minHeight ) );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0," + Px( WindowTitleHeight ), "1,0 1,-" + Px( WindowTitleHeight ) );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "WebViewForm", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* bar = CFormLayout::AddWidgetNode( *content, "Panel", "NavigationBar", "0,0 0,0", "1,0 0," + Px( NavigationBarHeight ) );
    CORE::CDataNode* back = CFormLayout::AddWidgetNode( *bar, "Button", "Back", "0,4 0,3", "0,32 0,28" );
    CFormLayout::SetWidgetNodeProperty( *back, EWidgetProperty_Text, "<" );
    CFormLayout::SetWidgetNodeProperty( *back, EWidgetProperty_Tooltip, "Back" );
    CORE::CDataNode* forward = CFormLayout::AddWidgetNode( *bar, "Button", "Forward", "0,40 0,3", "0,32 0,28" );
    CFormLayout::SetWidgetNodeProperty( *forward, EWidgetProperty_Text, ">" );
    CFormLayout::SetWidgetNodeProperty( *forward, EWidgetProperty_Tooltip, "Forward" );
    CORE::CDataNode* reload = CFormLayout::AddWidgetNode( *bar, "Button", "ReloadStop", "0,76 0,3", "0,64 0,28" );
    CFormLayout::SetWidgetNodeProperty( *reload, EWidgetProperty_Text, "Reload" );
    CFormLayout::SetWidgetNodeProperty( *reload, EWidgetProperty_Tooltip, "Load the page again" );
    CORE::CDataNode* address = CFormLayout::AddWidgetNode( *bar, "Editbox", "Address", "0,146 0,3", "1,-182 0,28" );
    CFormLayout::SetWidgetNodeProperty( *address, EWidgetProperty_Tooltip, "The address of the page" );
    CORE::CDataNode* security = CFormLayout::AddWidgetNode( *bar, "ImageFrame", "Security", "1,-30 0,7", "0,20 0,20" );     // SecurityMarkSize
    CFormLayout::SetWidgetNodeProperty( *security, EWidgetProperty_ScaleMode, "Stretch" );

    CORE::CDataNode* banner = CFormLayout::AddWidgetNode( *content, "Label", "Banner", "0,8 0,0", "1,-16 0,0" );
    CFormLayout::SetWidgetNodeProperty( *banner, EWidgetProperty_WordWrap, "true" );
    CFormLayout::SetWidgetNodeProperty( *banner, EWidgetProperty_Visible, "false" );

    CORE::CDataNode* page = CFormLayout::AddWidgetNode( *content, "WebPageView", "Page", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *page, EWidgetProperty_Focusable, "true" );

    CORE::CDataNode* overlay = CFormLayout::AddWidgetNode( *content, "Label", "LoadingOverlay", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *overlay, EWidgetProperty_Text, "Loading..." );
    CFormLayout::SetWidgetNodeProperty( *overlay, EWidgetProperty_HTextAlign, "Center" );
    CFormLayout::SetWidgetNodeProperty( *overlay, EWidgetProperty_VTextAlign, "Center" );
    CFormLayout::SetWidgetNodeProperty( *overlay, EWidgetProperty_InputTransparent, "true" );
    CFormLayout::SetWidgetNodeProperty( *overlay, EWidgetProperty_Visible, "false" );

    CORE::CDataNode* error = CFormLayout::AddWidgetNode( *content, "Panel", "ErrorPanel", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *error, EWidgetProperty_Visible, "false" );
    CORE::CDataNode* errorText = CFormLayout::AddWidgetNode( *error, "Textbox", "ErrorText", "0,16 0,16", "1,-32 1,-72" );
    CFormLayout::SetWidgetNodeProperty( *errorText, EWidgetProperty_ReadOnly, "true" );
    CORE::CDataNode* retry = CFormLayout::AddWidgetNode( *error, "Button", "Retry", "1,-232 1,-46", "0,100 0,32" );
    CFormLayout::SetWidgetNodeProperty( *retry, EWidgetProperty_Text, "Retry" );
    CORE::CDataNode* errorBack = CFormLayout::AddWidgetNode( *error, "Button", "ErrorBack", "1,-116 1,-46", "0,100 0,32" );
    CFormLayout::SetWidgetNodeProperty( *errorBack, EWidgetProperty_Text, "Back" );

    CORE::CDataNode* status = CFormLayout::AddWidgetNode( *content, "Panel", "StatusBar", "0,0 1,-" + Px( StatusBarHeight ), "1,0 0," + Px( StatusBarHeight ) );
    CORE::CDataNode* statusText = CFormLayout::AddWidgetNode( *status, "Label", "Status", "0,6 0,4", "1,-130 0,18" );
    CFormLayout::SetWidgetNodeProperty( *statusText, EWidgetProperty_Text, "" );
    CORE::CDataNode* progress = CFormLayout::AddWidgetNode( *status, "ProgressBar", "Progress", "1,-118 0,6", "0,110 0,12" );
    CFormLayout::SetWidgetNodeProperty( *progress, EWidgetProperty_Max, "100" );
    CFormLayout::SetWidgetNodeProperty( *progress, EWidgetProperty_Value, "0" );

    CORE::CDataNode* buttons = CFormLayout::AddWidgetNode( *content, "Panel", "DialogButtons", "0,0 1,-" + Px( DialogButtonsHeight ), "1,0 0," + Px( DialogButtonsHeight ) );
    CORE::CDataNode* cancel = CFormLayout::AddWidgetNode( *buttons, "Button", "Cancel", "1,-116 0,6", "0,108 0,32" );
    CFormLayout::SetWidgetNodeProperty( *cancel, EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    struct SButton { const char* name; void ( CWebViewForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] = { { "Back", &CWebViewForm::OnBack }, { "ErrorBack", &CWebViewForm::OnBack }, { "Forward", &CWebViewForm::OnForward },
                                { "ReloadStop", &CWebViewForm::OnReloadStop }, { "Retry", &CWebViewForm::OnRetry }, { "Cancel", &CWebViewForm::OnCancelButton } };
    for ( UInt32 i=0; i<(UInt32)( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL == button )
            continue;
        TEventCallback callback( this, buttons[ i ].handler );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    CWidget* address = GetWidget( "Address" );
    if ( GUCEF_NULL != address )
    {
        TEventCallback callback( this, &CWebViewForm::OnAddressEntered );
        SubscribeTo( address, CEditbox::TextEnteredEvent, callback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CWebViewForm::OnRootKeyDown );
        SubscribeTo( root, CWidget::KeyDownEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CWebViewForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }
    EnsureSecurityImages();
    ShowSecurity( WEB::WEBVIEWSECURITY_NONE );
    m_laidOutWidth = -1;
    Relayout();
    // a view attached before the layout was loaded gets the new page widget
    if ( !m_view.IsNULL() )
    {
        WEB::TWebViewPtr view = m_view;
        Attach( view );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::SetWidgetVisible( const CString& name, bool visible )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( name );
    if ( GUCEF_NULL != widget )
        widget->SetVisibility( visible );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::Relayout( void )
{GUCEF_TRACE;

    CWidget* page = GetWidget( "Page" );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == page || GUCEF_NULL == context )
        return;

    SetWidgetVisible( "NavigationBar", m_showNavigationBar );
    SetWidgetVisible( "StatusBar", m_showStatusBar );
    SetWidgetVisible( "DialogButtons", m_showDialogButtons );
    CWidget* address = GetWidget( "Address" );
    if ( GUCEF_NULL != address )
        address->SetBoolProperty( EWidgetProperty_ReadOnly, !m_addressEditable );

    Int32 top = m_showNavigationBar ? NavigationBarHeight : 0;
    CWidget* banner = GetWidget( "Banner" );
    if ( GUCEF_NULL != banner )
    {
        banner->SetVisibility( !m_prompt.IsNULLOrEmpty() );
        banner->SetProperty( EWidgetProperty_Text, m_prompt );
        if ( !m_prompt.IsNULLOrEmpty() )
        {
            // as high as the wrapped prompt at the width it has
            CWidget* content = GUCEF_NULL != banner->GetParentWidget() ? banner->GetParentWidget() : banner;
            const Int32 width = content->GetRect().width > 32 ? content->GetRect().width - 16 : 600;
            Int32 textWidth = 0, textHeight = 0;
            context->MeasureText( banner->GetProperty( EWidgetProperty_Font ), m_prompt, width, textWidth, textHeight );
            if ( textHeight <= 0 )
                textHeight = context->GetLineHeight( banner->GetProperty( EWidgetProperty_Font ) );
            banner->SetProperty( EWidgetProperty_Position, "0,8 0," + Px( top + 6 ) );
            banner->SetProperty( EWidgetProperty_Size, "1,-16 0," + Px( textHeight ) );
            top += textHeight + 12;
            m_laidOutWidth = content->GetRect().width;
        }
    }
    const Int32 bottom = ( m_showStatusBar ? StatusBarHeight : 0 ) + ( m_showDialogButtons ? DialogButtonsHeight : 0 );
    const CString position = "0,0 0," + Px( top );
    const CString size = "1,0 1,-" + Px( top + bottom );
    const char* pageArea[] = { "Page", "LoadingOverlay", "ErrorPanel" };
    for ( UInt32 i=0; i<3; ++i )
    {
        CWidget* widget = GetWidget( pageArea[ i ] );
        if ( GUCEF_NULL == widget )
            continue;
        widget->SetProperty( EWidgetProperty_Position, position );
        widget->SetProperty( EWidgetProperty_Size, size );
    }
    CWidget* status = GetWidget( "StatusBar" );
    if ( GUCEF_NULL != status )
        status->SetProperty( EWidgetProperty_Position, "0,0 1,-" + Px( bottom ) );
    page->SetVisibility( !m_errorShown );
    FitStatusText();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::Attach( const WEB::TWebViewPtr& view )
{GUCEF_TRACE;

    if ( !m_view.IsNULL() && m_view != view )
        Detach();
    if ( view.IsNULL() )
        return false;
    m_view = view;
    m_relay.Attach( view );
    m_frameSinceLoad = m_view->GetFrameExchange().HasFrame();
    m_errorShown = false;
    SetWidgetVisible( "ErrorPanel", false );
    SetLoading( m_view->IsLoading() );
    ShowAddress( m_view->GetUrl() );
    ShowSecurity( m_view->GetSecurityState() );
    CWebPageView* page = GetPageView();
    if ( GUCEF_NULL == page )
        return true;            // the layout is not loaded yet: done in OnPostLayoutLoad()
    page->Attach( view );
    const CString title = m_view->GetTitle();
    page->SetProperty( EWidgetProperty_Tooltip, title );
    Relayout();
    CGUIContext* context = GetGUIContext();
    if ( m_focusPageOnShow && GUCEF_NULL != context && IsVisible() )
        context->SetFocus( page );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::Detach( void )
{GUCEF_TRACE;

    CWebPageView* page = GetPageView();
    if ( GUCEF_NULL != page )
        page->Detach();
    m_relay.Detach();
    m_view.Unlink();
    m_prompts.clear();
    m_answered.clear();
    if ( GUCEF_NULL != m_promptForm && m_promptOpen )
    {
        m_promptOpen = false;
        m_promptForm->Hide();
    }
    SetLoading( false );
}

/*-------------------------------------------------------------------------*/

WEB::TWebViewPtr
CWebViewForm::GetView( void ) const
{GUCEF_TRACE;

    return m_view;
}

/*-------------------------------------------------------------------------*/

CWebPageView*
CWebViewForm::GetPageView( void )
{GUCEF_TRACE;

    return GetWidgetAs< CWebPageView >( "Page" );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewForm::Navigate( const CString& url )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return WEB::CWebViewEngineBase::MakeEndedFuture( WEB::WEBVIEWERROR_VIEW_CLOSED, "The form shows no view" );
    HideError();
    return m_view->Navigate( url );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::SetPrompt( const CString& prompt )
{GUCEF_TRACE;

    m_prompt = prompt;
    Relayout();
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewForm::GetPrompt( void ) const
{GUCEF_TRACE;

    return m_prompt;
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::ShowError( const CString& message, bool canRetry )
{GUCEF_TRACE;

    m_errorShown = true;
    CWidget* text = GetWidget( "ErrorText" );
    if ( GUCEF_NULL != text )
        text->SetProperty( EWidgetProperty_Text, message );
    SetWidgetVisible( "Retry", canRetry );
    SetWidgetVisible( "ErrorPanel", true );
    SetWidgetVisible( "LoadingOverlay", false );
    SetWidgetVisible( "Page", false );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::HideError( void )
{GUCEF_TRACE;

    if ( !m_errorShown )
        return;
    m_errorShown = false;
    SetWidgetVisible( "ErrorPanel", false );
    SetWidgetVisible( "Page", true );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::IsErrorShown( void ) const
{GUCEF_TRACE;

    return m_errorShown;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::IsLoadingOverlayShown( void ) const
{GUCEF_TRACE;

    CWidget* overlay = const_cast< CWebViewForm* >( this )->GetWidget( "LoadingOverlay" );
    return GUCEF_NULL != overlay && overlay->IsVisible();
}

/*-------------------------------------------------------------------------*/

void CWebViewForm::SetShowNavigationBar( bool show ) {GUCEF_TRACE; m_showNavigationBar = show; Relayout(); }
bool CWebViewForm::GetShowNavigationBar( void ) const {GUCEF_TRACE; return m_showNavigationBar; }
void CWebViewForm::SetShowStatusBar( bool show ) {GUCEF_TRACE; m_showStatusBar = show; Relayout(); }
bool CWebViewForm::GetShowStatusBar( void ) const {GUCEF_TRACE; return m_showStatusBar; }
void CWebViewForm::SetShowDialogButtons( bool show ) {GUCEF_TRACE; m_showDialogButtons = show; Relayout(); }
bool CWebViewForm::GetShowDialogButtons( void ) const {GUCEF_TRACE; return m_showDialogButtons; }
void CWebViewForm::SetAddressEditable( bool editable ) {GUCEF_TRACE; m_addressEditable = editable; Relayout(); if ( !m_view.IsNULL() ) ShowAddress( m_view->GetUrl() ); }
bool CWebViewForm::GetAddressEditable( void ) const {GUCEF_TRACE; return m_addressEditable; }
void CWebViewForm::SetFocusPageOnShow( bool focusPage ) {GUCEF_TRACE; m_focusPageOnShow = focusPage; }
bool CWebViewForm::GetFocusPageOnShow( void ) const {GUCEF_TRACE; return m_focusPageOnShow; }
void CWebViewForm::SetFollowTextScale( bool follow ) {GUCEF_TRACE; m_followTextScale = follow; }
bool CWebViewForm::GetFollowTextScale( void ) const {GUCEF_TRACE; return m_followTextScale; }

/*-------------------------------------------------------------------------*/

void
CWebViewForm::ApplyPresentation( const WEB::CWebViewPresentation& presentation )
{GUCEF_TRACE;

    m_showNavigationBar = presentation.showNavigationBar;
    m_showStatusBar = presentation.showStatusBar;
    m_showDialogButtons = presentation.showDialogButtons;
    m_addressEditable = presentation.addressEditable;
    m_prompt = presentation.prompt;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root && GUCEF_NULL != root->AsWindow() && !presentation.title.IsNULLOrEmpty() )
        root->SetProperty( EWidgetProperty_Title, presentation.title );
    SetWidgetVisible( "Cancel", presentation.cancelAllowed );
    Relayout();
    if ( !m_view.IsNULL() )
        ShowAddress( m_view->GetUrl() );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::Cancel( void )
{GUCEF_TRACE;

    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::ResolveDecision( UInt32 decisionId, bool allow, const CString& text )
{GUCEF_TRACE;

    if ( m_view.IsNULL() || 0 == decisionId )
        return false;
    m_answered.insert( decisionId );
    m_view->ResolveDecision( decisionId, allow, text );
    return true;
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm*
CWebViewForm::GetOpenPrompt( void )
{GUCEF_TRACE;

    return m_promptOpen ? m_promptForm : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewForm::GetPendingPromptCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_prompts.size() + ( m_promptOpen ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewForm::GetStatusText( void ) const
{GUCEF_TRACE;

    return m_statusText;
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::Update( void )
{GUCEF_TRACE;

    m_relay.Relay();
    ShowNextPrompt();
    // the banner wraps again when the width changed
    CWidget* banner = GetWidget( "Banner" );
    if ( GUCEF_NULL != banner && !m_prompt.IsNULLOrEmpty() && GUCEF_NULL != banner->GetParentWidget() && banner->GetParentWidget()->GetRect().width != m_laidOutWidth )
        Relayout();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::SetLoading( bool loading )
{GUCEF_TRACE;

    m_loading = loading;
    CWidget* reload = GetWidget( "ReloadStop" );
    if ( GUCEF_NULL != reload )
    {
        reload->SetProperty( EWidgetProperty_Text, loading ? "Stop" : "Reload" );
        reload->SetProperty( EWidgetProperty_Tooltip, loading ? "Stop loading the page" : "Load the page again" );
    }
    // the loading line shows until the first frame of the navigation was drawn
    SetWidgetVisible( "LoadingOverlay", loading && !m_frameSinceLoad && !m_errorShown );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::SetStatusText( const CString& text )
{GUCEF_TRACE;

    m_statusText = text;
    FitStatusText();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::FitStatusText( void )
{GUCEF_TRACE;

    CWidget* label = GetWidget( "Status" );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == label || GUCEF_NULL == context )
        return;
    // cut at the end with "..." when it does not fit; the tooltip has all of it
    const Int32 available = label->GetRect().width;
    CString shown = m_statusText;
    Int32 width = 0, height = 0;
    const CString font = label->GetProperty( EWidgetProperty_Font );
    context->MeasureText( font, shown, 0, width, height );
    if ( available > 0 && width > available )
    {
        UInt32 keep = shown.Length();
        while ( keep > 0 )
        {
            --keep;
            shown = m_statusText.SubstrToIndex( keep, true ) + "...";
            context->MeasureText( font, shown, 0, width, height );
            if ( width <= available )
                break;
        }
    }
    label->SetProperty( EWidgetProperty_Text, shown );
    label->SetProperty( EWidgetProperty_Tooltip, shown == m_statusText ? CString::Empty : m_statusText );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::ShowAddress( const CString& url )
{GUCEF_TRACE;

    CWidget* address = GetWidget( "Address" );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == address || ( GUCEF_NULL != context && context->GetFocus() == address ) )
        return;
    address->SetProperty( EWidgetProperty_Text, m_addressEditable ? url : WithoutQuery( url ) );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::EnsureSecurityImages( void )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return;
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( context->HasImage( SecurityImageAliases[ i ] ) )
            continue;
        // a round mark in the colour of the state, at the size of the indicator so no driver has to scale it
        const Int32 size = SecurityMarkSize;
        IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, size, size, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
        for ( Int32 y=0; y<size; ++y )
        {
            for ( Int32 x=0; x<size; ++x )
            {
                UInt8* p = data + ( y * size + x ) * 4;
                const Int32 dx = 2 * x - ( size - 1 ), dy = 2 * y - ( size - 1 );
                const bool inside = dx * dx + dy * dy <= ( size - 1 ) * ( size - 1 );
                p[ 0 ] = SecurityColours[ i ][ 0 ];
                p[ 1 ] = SecurityColours[ i ][ 1 ];
                p[ 2 ] = SecurityColours[ i ][ 2 ];
                p[ 3 ] = inside ? 255 : 0;
            }
        }
        context->RegisterImage( SecurityImageAliases[ i ], pixels );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::ShowSecurity( WEB::TWebViewSecurityState state )
{GUCEF_TRACE;

    CWidget* indicator = GetWidget( "Security" );
    if ( GUCEF_NULL == indicator )
        return;
    const UInt32 index = (UInt32) state < 4 ? (UInt32) state : 0;
    indicator->SetProperty( EWidgetProperty_Image, SecurityImageAliases[ index ] );
    indicator->SetProperty( EWidgetProperty_Tooltip, SecurityTooltips[ index ] );
    indicator->SetVisibility( WEB::WEBVIEWSECURITY_NONE != state );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewLoadStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* url = static_cast< const CORE::TCloneableString* >( eventData );
    m_frameSinceLoad = false;
    HideError();
    SetLoading( true );
    CWidget* progress = GetWidget( "Progress" );
    if ( GUCEF_NULL != progress )
        progress->SetProperty( EWidgetProperty_Value, "0" );
    if ( GUCEF_NULL != url )
        SetStatusText( "Loading " + HostOf( url->GetData() ) );
    CORE::TCloneableBool loading( true );
    NotifyObservers( LoadStateChangedEvent, &loading );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewLoadCommitted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* url = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != url )
        ShowAddress( url->GetData() );
    if ( !m_view.IsNULL() )
        ShowSecurity( m_view->GetSecurityState() );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewLoadFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SetLoading( false );
    SetStatusText( CString::Empty );
    CWidget* progress = GetWidget( "Progress" );
    if ( GUCEF_NULL != progress )
        progress->SetProperty( EWidgetProperty_Value, "100" );
    CORE::TCloneableBool loading( false );
    NotifyObservers( LoadStateChangedEvent, &loading );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewLoadFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::TWebViewLoadErrorEventData* failure = static_cast< const WEB::TWebViewLoadErrorEventData* >( eventData );
    SetLoading( false );
    if ( GUCEF_NULL != failure && WEB::WEBVIEWERROR_CANCELLED != failure->GetData().error )
    {
        const WEB::CWebViewLoadError& error = failure->GetData();
        m_failedUrl = error.url;
        CString reason;
        switch ( error.error )
        {
            case WEB::WEBVIEWERROR_REFUSED_BY_POLICY: reason = "This page may not be opened here: " + HostOf( error.url ) + " is not one of the allowed sites."; break;
            case WEB::WEBVIEWERROR_CERTIFICATE: reason = "The connection to " + HostOf( error.url ) + " is not trusted: its certificate was refused."; break;
            case WEB::WEBVIEWERROR_NETWORK: reason = HostOf( error.url ) + " could not be reached."; break;
            case WEB::WEBVIEWERROR_PROCESS_ENDED: reason = "The page stopped working."; break;
            default: reason = "The page could not be loaded."; break;
        }
        if ( !error.message.IsNULLOrEmpty() )
            reason += "\n\n" + error.message;
        ShowError( reason, WEB::WEBVIEWERROR_REFUSED_BY_POLICY != error.error );
    }
    NotifyObservers( LoadFailedEvent, eventData );
    CORE::TCloneableBool loading( false );
    NotifyObservers( LoadStateChangedEvent, &loading );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewTitleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* title = static_cast< const CORE::TCloneableString* >( eventData );
    CWebPageView* page = GetPageView();
    if ( GUCEF_NULL != page && GUCEF_NULL != title )
        page->SetProperty( EWidgetProperty_Tooltip, title->GetData() );
    NotifyObservers( TitleChangedEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewUrlChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* url = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != url )
        ShowAddress( url->GetData() );
    NotifyObservers( UrlChangedEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewProgressChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::IWebView::TProgressEventData* progress = static_cast< const WEB::IWebView::TProgressEventData* >( eventData );
    CWidget* bar = GetWidget( "Progress" );
    if ( GUCEF_NULL != bar && GUCEF_NULL != progress )
        bar->SetProperty( EWidgetProperty_Value, CORE::Int32ToString( (Int32) ( progress->GetData() * 100.0f + 0.5f ) ) );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewSecurityChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::IWebView::TStateEventData* state = static_cast< const WEB::IWebView::TStateEventData* >( eventData );
    if ( GUCEF_NULL != state )
        ShowSecurity( (WEB::TWebViewSecurityState) state->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewFrameAvailable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_frameSinceLoad = true;
    SetWidgetVisible( "LoadingOverlay", false );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewDecisionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    SPrompt prompt;
    prompt.decisionId = 0;
    prompt.dialogKind = WEB::WEBVIEWDIALOG_ALERT;
    WEB::TWebViewDecision decision = WEB::WEBVIEWDECISION_REFUSE;
    const CORE::CEvent* formEvent = &PopupRequestedEvent;

    if ( WEB::IWebView::PopupRequestedEvent == eventId )
    {
        const WEB::CWebViewPopupRequest& request = static_cast< WEB::TWebViewPopupRequestEventData* >( eventData )->GetData();
        prompt.kind = WEB::WEBVIEWDECISIONKIND_POPUP;
        prompt.decisionId = request.decisionId;
        prompt.title = "Open a window";
        prompt.text = "The page wants to open " + WithoutQuery( request.url ) + ". Open it here?";
        decision = request.decision;
        formEvent = &PopupRequestedEvent;
    }
    else
    if ( WEB::IWebView::DownloadRequestedEvent == eventId )
    {
        const WEB::CWebViewDownloadRequest& request = static_cast< WEB::TWebViewDownloadRequestEventData* >( eventData )->GetData();
        prompt.kind = WEB::WEBVIEWDECISIONKIND_DOWNLOAD;
        prompt.decisionId = request.decisionId;
        prompt.title = "Download";
        prompt.text = "The page wants to download " + ( request.suggestedName.IsNULLOrEmpty() ? CString( "a file" ) : request.suggestedName ) + ". Allow it?";
        decision = request.decision;
        formEvent = &DownloadRequestedEvent;
    }
    else
    if ( WEB::IWebView::PermissionRequestedEvent == eventId )
    {
        const WEB::CWebViewPermissionRequest& request = static_cast< WEB::TWebViewPermissionRequestEventData* >( eventData )->GetData();
        prompt.kind = WEB::WEBVIEWDECISIONKIND_PERMISSION;
        prompt.decisionId = request.decisionId;
        prompt.title = "Permission";
        prompt.text = HostOf( request.origin ) + " asks for " + WEB::WebViewPermissionKindToString( request.kind ) + ". Allow it?";
        decision = request.decision;
        formEvent = &PermissionRequestedEvent;
    }
    else
    if ( WEB::IWebView::CertificateErrorEvent == eventId )
    {
        const WEB::CWebViewCertificateError& report = static_cast< WEB::TWebViewCertificateErrorEventData* >( eventData )->GetData();
        prompt.kind = WEB::WEBVIEWDECISIONKIND_CERTIFICATE_ERROR;
        prompt.decisionId = report.decisionId;
        prompt.title = "Certificate error";
        prompt.text = "The certificate of " + report.host + " is not trusted (" + report.errorDescription + ", issued by " + report.issuer + "). Continue anyway?";
        decision = report.decision;
        formEvent = &CertificateErrorEvent;
        ShowSecurity( WEB::WEBVIEWSECURITY_CERTIFICATE_ERROR );
    }
    else
    if ( WEB::IWebView::ScriptDialogEvent == eventId )
    {
        const WEB::CWebViewScriptDialog& dialog = static_cast< WEB::TWebViewScriptDialogEventData* >( eventData )->GetData();
        prompt.kind = WEB::WEBVIEWDECISIONKIND_SCRIPT_DIALOG;
        prompt.dialogKind = dialog.kind;
        prompt.decisionId = dialog.decisionId;
        prompt.title = WEB::WEBVIEWDIALOG_BEFOREUNLOAD == dialog.kind ? "Leave the page" : "The page says";
        prompt.text = dialog.message;
        prompt.defaultText = dialog.defaultPromptText;
        decision = dialog.decision;
        formEvent = &ScriptDialogEvent;
    }
    else
        return;

    // the owner answers while it is told; what nobody answered the form asks the person
    NotifyObservers( *formEvent, eventData );
    if ( WEB::WEBVIEWDECISION_ASK != decision || 0 == prompt.decisionId )
        return;
    if ( m_answered.find( prompt.decisionId ) != m_answered.end() )
    {
        m_answered.erase( prompt.decisionId );
        return;
    }
    m_prompts.push_back( prompt );
    ShowNextPrompt();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::ShowNextPrompt( void )
{GUCEF_TRACE;

    if ( m_promptOpen || m_prompts.empty() || m_view.IsNULL() )
        return;
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return;
    if ( GUCEF_NULL == m_promptForm )
    {
        m_promptForm = GUCEF_NEW CMessageBoxForm( context );
        TEventCallback callback( this, &CWebViewForm::OnPromptClosed );
        SubscribeTo( m_promptForm, CMessageBoxForm::ClosedEvent, callback );
    }
    const SPrompt& prompt = m_prompts.front();
    bool opened = false;
    if ( WEB::WEBVIEWDECISIONKIND_SCRIPT_DIALOG == prompt.kind )
    {
        switch ( prompt.dialogKind )
        {
            case WEB::WEBVIEWDIALOG_ALERT: opened = m_promptForm->Open( prompt.title, prompt.text, CMessageBoxForm::MESSAGEBOXBUTTONS_OK ); break;
            case WEB::WEBVIEWDIALOG_CONFIRM: opened = m_promptForm->Open( prompt.title, prompt.text, CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL ); break;
            case WEB::WEBVIEWDIALOG_PROMPT: opened = m_promptForm->OpenInput( prompt.title, prompt.text, prompt.defaultText, CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL ); break;
            default: opened = m_promptForm->Open( prompt.title, prompt.text, CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO ); break;
        }
    }
    else
        opened = m_promptForm->Open( prompt.title, prompt.text, CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO );
    m_promptOpen = opened;
    if ( !opened )
    {
        // nobody can be asked: refused, as an unanswered decision would be
        m_view->ResolveDecision( prompt.decisionId, false );
        m_prompts.erase( m_prompts.begin() );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnPromptClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_promptOpen || m_prompts.empty() || GUCEF_NULL == m_promptForm )
        return;
    m_promptOpen = false;
    const SPrompt prompt = m_prompts.front();
    m_prompts.erase( m_prompts.begin() );
    const CMessageBoxForm::EMessageBoxResult result = m_promptForm->GetResult();
    const bool allow = CMessageBoxForm::MESSAGEBOXRESULT_OK == result || CMessageBoxForm::MESSAGEBOXRESULT_YES == result;
    const CString text = WEB::WEBVIEWDIALOG_PROMPT == prompt.dialogKind && WEB::WEBVIEWDECISIONKIND_SCRIPT_DIALOG == prompt.kind ? m_promptForm->GetInputText() : CString::Empty;
    if ( !m_view.IsNULL() )
        m_view->ResolveDecision( prompt.decisionId, allow, text );
    // the next one opens at the next frame, not inside the notification of this one
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewConsoleMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( ConsoleMessageEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewCloseRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( CloseRequestedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewForm::FocusChrome( bool forward )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return false;
    const char* forwardOrder[] = { "Cancel", "Back", "Address" };
    const char* backwardOrder[] = { "Address", "Back", "Cancel" };
    const char** order = forward ? forwardOrder : backwardOrder;
    for ( UInt32 i=0; i<3; ++i )
    {
        CWidget* widget = GetWidget( order[ i ] );
        if ( GUCEF_NULL != widget && widget->IsEffectivelyVisible() && widget->IsFocusable() && context->SetFocus( widget ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewFocusLeave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::IWebView::TFlagEventData* forward = static_cast< const WEB::IWebView::TFlagEventData* >( eventData );
    CWebPageView* page = GetPageView();
    CGUIContext* context = GetGUIContext();
    // only when the page still has the focus: the person may have moved on meanwhile
    if ( GUCEF_NULL != page && GUCEF_NULL != context && context->GetFocus() == page )
        FocusChrome( GUCEF_NULL == forward || forward->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnViewEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SetLoading( false );
    if ( WEB::IWebView::EngineStalledEvent == eventId )
        ShowError( "The web page stopped answering. Close this window and try again.", false );
    else
    if ( WEB::IWebView::ProcessTerminatedEvent == eventId )
        ShowError( "The page stopped working.", true );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnBack( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_view.IsNULL() )
    {
        HideError();
        m_view->GoBack();
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnForward( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_view.IsNULL() )
        m_view->GoForward();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnReloadStop( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return;
    if ( m_loading )
        m_view->Stop();
    else
        m_view->Reload();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnAddressEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWidget* address = GetWidget( "Address" );
    if ( !m_addressEditable || GUCEF_NULL == address || m_view.IsNULL() )
        return;
    CString url = address->GetProperty( EWidgetProperty_Text ).Trim( true ).Trim( false );
    if ( url.IsNULLOrEmpty() )
        return;
    // an address without a scheme is a web site
    if ( url.HasSubstr( "://" ) < 0 && !url.Lowercase().StartsWith( "about:" ) && !url.Lowercase().StartsWith( "data:" ) )
        url = "https://" + url;
    Navigate( url );
    CWebPageView* page = GetPageView();
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != page && GUCEF_NULL != context )
        context->SetFocus( page );
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnRetry( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_view.IsNULL() )
        return;
    HideError();
    if ( !m_failedUrl.IsNULLOrEmpty() )
        m_view->Navigate( m_failedUrl );
    else
        m_view->Reload();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnCancelButton( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CWebViewForm::OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( eventData );
    CWebPageView* page = GetPageView();
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == key || GUCEF_NULL == context || key->handled )
        return;
    const bool inPage = GUCEF_NULL != page && key->source == page;

    // F6 (and Ctrl+F6 backwards) moves between the page and the chrome, taken before the page sees it
    if ( INPUT::KEYCODE_F6 == key->keyCode && EVENTPHASE_TUNNEL == key->phase )
    {
        if ( inPage )
            FocusChrome( 0 == ( key->modifiers & INPUT::KEYMOD_CTRL ) );
        else
        if ( GUCEF_NULL != page && page->IsEffectivelyVisible() )
            context->SetFocus( page );
        key->handled = true;
        return;
    }
    // Escape on the chrome is Cancel; in the page it is the page's
    if ( INPUT::KEYCODE_ESCAPE == key->keyCode && !inPage && EVENTPHASE_TUNNEL != key->phase )
    {
        key->handled = true;
        Cancel();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWebViewForm";
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
