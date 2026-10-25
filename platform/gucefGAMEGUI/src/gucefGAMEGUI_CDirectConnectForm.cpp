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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CDirectConnectForm.h"

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

const CORE::CEvent CDirectConnectForm::ConnectRequestedEvent = "GUCEF::GAMEGUI::CDirectConnectForm::ConnectRequestedEvent";
const CORE::CEvent CDirectConnectForm::CancelledEvent = "GUCEF::GAMEGUI::CDirectConnectForm::CancelledEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDirectConnectRequest::CDirectConnectRequest( void )
    : CORE::CICloneable()
    , address()
    , endpoint()
    , password()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDirectConnectRequest::CDirectConnectRequest( const CORE::CString& address_  ,
                                              const CORE::CString& endpoint_ ,
                                              const CORE::CString& password_ )
    : CORE::CICloneable()
    , address( address_ )
    , endpoint( endpoint_ )
    , password( password_ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDirectConnectRequest::~CDirectConnectRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDirectConnectRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDirectConnectRequest( *this );
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectRequestedEvent.Initialize();
    CancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDirectConnectForm::CDirectConnectForm( void )
    : GUI::CFormEx()
    , m_defaultScheme( "udp" )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDirectConnectForm::CDirectConnectForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_defaultScheme( "udp" )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDirectConnectForm::~CDirectConnectForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "DirectConnectForm", "0.5,0 0.5,0", "0.9,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Direct connect" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,480 0,300" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* addressLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "AddressLabel", "0,20 0,56", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *addressLabel, GUI::EWidgetProperty_Text, "Address (host:port or tcp://host:port)" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Address", "0,20 0,84", "1,-40 0,36" );

    CORE::CDataNode* passwordLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "PasswordLabel", "0,20 0,132", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *passwordLabel, GUI::EWidgetProperty_Text, "Password, if the game has one" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Password", "0,20 0,160", "1,-40 0,36" );

    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Error", "0,20 0,204", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_TextColour, "255,110,110,255" );

    CORE::CDataNode* connect = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Connect", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, GUI::EWidgetProperty_Text, "Connect" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* button = GetWidget( "Connect" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CDirectConnectForm::OnConnectActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Cancel" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CDirectConnectForm::OnCancelActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    TEventCallback fieldCallback( this, &CDirectConnectForm::OnFieldTextEntered );
    GUI::CWidget* field = GetWidget( "Address" );
    if ( GUCEF_NULL != field )
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, fieldCallback );
    field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, fieldCallback );
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CDirectConnectForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* address = GetWidget( "Address" );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != address )
        context->SetFocus( address );
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::SetAddress( const CORE::CString& address )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Address" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, address );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectConnectForm::GetAddress( void )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Address" );
    return GUCEF_NULL != field ? field->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::SetPassword( const CORE::CString& password )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, password );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectConnectForm::GetPassword( void )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Password" );
    return GUCEF_NULL != field ? field->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::SetDefaultScheme( const CORE::CString& scheme )
{GUCEF_TRACE;

    m_defaultScheme = scheme;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDirectConnectForm::GetDefaultScheme( void ) const
{GUCEF_TRACE;

    return m_defaultScheme;
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::ParseAddress( const CORE::CString& address       ,
                                  const CORE::CString& defaultScheme ,
                                  CORE::CString& endpoint            )
{GUCEF_TRACE;

    endpoint.Clear();

    CORE::CString trimmed = TrimText( address );
    CORE::CString scheme, host, path;
    UInt16 port = 0;
    if ( !MULTIPLAY::CGameListingEntry::ParseEndpoint( trimmed, scheme, host, port, path ) )
        return false;

    if ( scheme.IsNULLOrEmpty() )
        scheme = defaultScheme;
    endpoint = MULTIPLAY::CGameListingEntry::MakeEndpoint( scheme, host, port ) + path;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::Connect( void )
{GUCEF_TRACE;

    CORE::CString address = TrimText( GetAddress() );
    CORE::CString endpoint;
    GUI::CWidget* error = GetWidget( "Error" );

    if ( address.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( GUI::EWidgetProperty_Text, "Enter the address of the game" );
        return false;
    }
    if ( !ParseAddress( address, m_defaultScheme, endpoint ) )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( GUI::EWidgetProperty_Text, "The address needs a host and a port, such as 192.168.1.5:7777" );
        return false;
    }
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

    CDirectConnectRequest request( address, endpoint, GetPassword() );
    Close();
    NotifyObservers( ConnectRequestedEvent, &request );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::Cancel( void )
{GUCEF_TRACE;

    Close();
    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectConnectForm::GetErrorText( void )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    return GUCEF_NULL != error ? error->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnConnectActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Connect();
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnFieldTextEntered( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        Connect();
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnNavigation( CORE::CNotifier* notifier    ,
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
CDirectConnectForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CDirectConnectForm";
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
