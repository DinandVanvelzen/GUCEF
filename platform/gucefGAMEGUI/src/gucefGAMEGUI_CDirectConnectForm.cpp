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

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "DirectConnectForm", "0.5,-240 0.5,-150", "0,480 0,300" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, "Title", "Direct connect" );

    CORE::CDataNode* addressLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "AddressLabel", "0,20 0,56", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *addressLabel, "Text", "Address (host:port or tcp://host:port)" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Address", "0,20 0,84", "1,-40 0,36" );

    CORE::CDataNode* passwordLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "PasswordLabel", "0,20 0,132", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *passwordLabel, "Text", "Password, if the game has one" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Password", "0,20 0,160", "1,-40 0,36" );

    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Error", "0,20 0,204", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, "TextColour", "255,110,110,255" );

    CORE::CDataNode* connect = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Connect", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, "Text", "Connect" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, "StyleClass", "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, "Text", "Cancel" );
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

    const char* buttons[] = { "Connect", "Cancel" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL != button )
            SubscribeTo( button, GUI::CWidget::ActivatedEvent );
    }
    const char* fields[] = { "Address", "Password" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* field = GetWidget( fields[ i ] );
        if ( GUCEF_NULL != field )
            SubscribeTo( field, GUI::CEditbox::TextEnteredEvent );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        SubscribeTo( root, GUI::CWidget::NavigationEvent );
}

/*-------------------------------------------------------------------------*/

bool
CDirectConnectForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

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
        field->SetProperty( "Text", address );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectConnectForm::GetAddress( void )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Address" );
    return GUCEF_NULL != field ? field->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::SetPassword( const CORE::CString& password )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        field->SetProperty( "Text", password );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectConnectForm::GetPassword( void )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "Password" );
    return GUCEF_NULL != field ? field->GetProperty( "Text" ) : CORE::CString::Empty;
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
            error->SetProperty( "Text", "Enter the address of the game" );
        return false;
    }
    if ( !ParseAddress( address, m_defaultScheme, endpoint ) )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( "Text", "The address needs a host and a port, such as 192.168.1.5:7777" );
        return false;
    }
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

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
    return GUCEF_NULL != error ? error->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CDirectConnectForm::OnNotify( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Connect" ) )
            Connect();
        else
        if ( notifier == GetWidget( "Cancel" ) )
            Cancel();
        return;
    }
    if ( GUI::CEditbox::TextEnteredEvent == eventId )
    {
        if ( m_open )
            Connect();
        return;
    }
    if ( GUI::CWidget::NavigationEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
        if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
        {
            navigation->handled = true;
            Cancel();
        }
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
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
