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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#include "gucefGAMEGUI_CConnectingForm.h"

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

const CORE::CEvent CConnectingForm::CancelRequestedEvent = "GUCEF::GAMEGUI::CConnectingForm::CancelRequestedEvent";
const CORE::CEvent CConnectingForm::ConnectedEvent = "GUCEF::GAMEGUI::CConnectingForm::ConnectedEvent";
const CORE::CEvent CConnectingForm::FailedEvent = "GUCEF::GAMEGUI::CConnectingForm::FailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CConnectingForm::RegisterEvents( void )
{GUCEF_TRACE;

    CancelRequestedEvent.Initialize();
    ConnectedEvent.Initialize();
    FailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CConnectingForm::CConnectingForm( void )
    : GUI::CFormEx()
    , m_endpoints()
    , m_hasRelay( false )
    , m_current( 0 )
    , m_progress( 0.0f )
    , m_failed( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CConnectingForm::CConnectingForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_endpoints()
    , m_hasRelay( false )
    , m_current( 0 )
    , m_progress( 0.0f )
    , m_failed( false )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CConnectingForm::~CConnectingForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "ConnectingForm", "0.5,0 0.5,0", "0.9,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Connecting" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,520 0,300" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,20 0,56", "1,-40 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Endpoint", "0,20 0,92", "1,-40 0,28" );
    CORE::CDataNode* relay = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Relay", "0,20 0,124", "1,-40 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *relay, GUI::EWidgetProperty_TextColour, "255,200,90,255" );

    GUI::CFormLayout::AddWidgetNode( *window, "ProgressBar", "Progress", "0,20 0,168", "1,-40 0,22" );
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "ProgressText", "0,20 0,196", "1,-40 0,24" );

    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::Build( void )
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
CConnectingForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
    {
        TEventCallback callback( this, &CConnectingForm::OnCancelActivated );
        SubscribeTo( cancel, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CConnectingForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::Begin( const MULTIPLAY::CLaunchDescription& launch )
{GUCEF_TRACE;

    CORE::CString::StringVector endpoints;
    launch.GetHostEndpoints( endpoints );
    return Begin( endpoints, launch.relay );
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::Begin( const CORE::CString::StringVector& endpoints, const CORE::CString& relay )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    m_endpoints = endpoints;
    m_hasRelay = !relay.IsNULLOrEmpty();
    if ( m_hasRelay )
        m_endpoints.push_back( relay );
    m_current = 0;
    m_failed = false;
    SetProgress( 0.0f, CORE::CString::Empty );

    GUI::CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
        cancel->SetProperty( GUI::EWidgetProperty_Text, "Cancel" );

    m_open = true;
    ShowCurrentEndpoint();
    return ShowModal();
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::ShowCurrentEndpoint( void )
{GUCEF_TRACE;

    if ( m_endpoints.empty() )
    {
        SetLabelText( "Status", "There is no endpoint to connect to" );
        SetLabelText( "Endpoint", CORE::CString::Empty );
        SetLabelText( "Relay", CORE::CString::Empty );
        return;
    }
    if ( m_current >= (UInt32) m_endpoints.size() )
        return;

    UInt32 direct = (UInt32) m_endpoints.size() - ( m_hasRelay ? 1 : 0 );
    if ( IsUsingRelay() )
    {
        SetLabelText( "Status", "Connecting through the relay" );
        SetLabelText( "Relay", "The game could not be reached directly, the relay of the lobby server is the fallback" );
    }
    else
    {
        SetLabelText( "Status", "Connecting to the game" );
        SetLabelText( "Relay", CORE::CString::Empty );
    }
    CORE::CString text = "Trying " + m_endpoints[ m_current ];
    if ( IsUsingRelay() )
        text += " (relay)";
    else
        text += " (" + CORE::UInt32ToString( m_current + 1 ) + " of " + CORE::UInt32ToString( direct ) + ")";
    SetLabelText( "Endpoint", text );
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::TryNextEndpoint( void )
{GUCEF_TRACE;

    if ( m_current + 1 >= (UInt32) m_endpoints.size() )
    {
        m_current = (UInt32) m_endpoints.size();
        ReportFailed( "No endpoint of the game could be reached" );
        return false;
    }
    ++m_current;
    ShowCurrentEndpoint();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CConnectingForm::GetCurrentEndpoint( void ) const
{GUCEF_TRACE;

    if ( m_current < (UInt32) m_endpoints.size() )
        return m_endpoints[ m_current ];
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::IsUsingRelay( void ) const
{GUCEF_TRACE;

    return m_hasRelay && !m_endpoints.empty() && m_current + 1 == (UInt32) m_endpoints.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CConnectingForm::GetEndpointCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_endpoints.size();
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::SetProgress( Float32 fraction, const CORE::CString& caption )
{GUCEF_TRACE;

    if ( fraction < 0.0f )
        fraction = 0.0f;
    if ( fraction > 1.0f )
        fraction = 1.0f;
    m_progress = fraction;

    GUI::CWidget* bar = GetWidget( "Progress" );
    if ( GUCEF_NULL != bar )
        bar->SetProperty( GUI::EWidgetProperty_Value, CORE::FloatToString( fraction * 100.0f ) );
    SetLabelText( "ProgressText", caption );
}

/*-------------------------------------------------------------------------*/

Float32
CConnectingForm::GetProgress( void ) const
{GUCEF_TRACE;

    return m_progress;
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::SetStatusText( const CORE::CString& text )
{GUCEF_TRACE;

    SetLabelText( "Status", text );
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::ReportConnected( void )
{GUCEF_TRACE;

    m_failed = false;
    Close();
    NotifyObservers( ConnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::ReportFailed( const CORE::CString& reason )
{GUCEF_TRACE;

    m_failed = true;
    SetLabelText( "Status", "Could not connect: " + reason );
    GUI::CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
        cancel->SetProperty( GUI::EWidgetProperty_Text, "Close" );
    TFailedEventData eventData( reason );
    NotifyObservers( FailedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::Cancel( void )
{GUCEF_TRACE;

    Close();
    NotifyObservers( CancelRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CConnectingForm::HasFailed( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        Cancel();
}

/*-------------------------------------------------------------------------*/

void
CConnectingForm::OnNavigation( CORE::CNotifier* notifier    ,
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
CConnectingForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CConnectingForm";
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
