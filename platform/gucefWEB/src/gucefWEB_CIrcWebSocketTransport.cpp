/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CIrcWebSocketTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CIrcWebSocketTransport::SubProtocol = "text.ircv3.net";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcWebSocketTransport::CIrcWebSocketTransport( void )
    : COM::CIIrcTransport()
    , m_webSocket( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_isConnected( false )
{GUCEF_TRACE;

    RegisterWebSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

CIrcWebSocketTransport::CIrcWebSocketTransport( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COM::CIIrcTransport()
    , m_webSocket( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_isConnected( false )
{GUCEF_TRACE;

    RegisterWebSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::RegisterWebSocketEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback connectedCallback( this, &CIrcWebSocketTransport::OnWebSocketConnected );
    SubscribeTo( &m_webSocket                  ,
                 CWebSocketClient::ConnectedEvent ,
                 connectedCallback             );

    TEventCallback textCallback( this, &CIrcWebSocketTransport::OnWebSocketTextMessage );
    SubscribeTo( &m_webSocket                  ,
                 CWebSocketClient::TextMessageEvent ,
                 textCallback                  );

    TEventCallback closedCallback( this, &CIrcWebSocketTransport::OnWebSocketClosed );
    SubscribeTo( &m_webSocket                  ,
                 CWebSocketClient::ClosedEvent ,
                 closedCallback                );

    TEventCallback errorCallback( this, &CIrcWebSocketTransport::OnWebSocketError );
    SubscribeTo( &m_webSocket                  ,
                 CWebSocketClient::ErrorEvent  ,
                 errorCallback                 );
}

/*-------------------------------------------------------------------------*/

CIrcWebSocketTransport::~CIrcWebSocketTransport()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_webSocket.Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CIrcWebSocketTransport::Connect( const COM::CIrcClientSettings& settings )
{GUCEF_TRACE;

    if ( CWebSocketClient::STATE_DISCONNECTED != m_webSocket.GetState() )
        m_webSocket.Disconnect();
    m_isConnected = false;

    CORE::CString path = settings.webSocketPath.IsNULLOrEmpty() ? CORE::CString( "/" ) : settings.webSocketPath;
    if ( !path.StartsWith( "/" ) )
        path = "/" + path;
    CORE::CString url = ( settings.useTls ? "wss://" : "ws://" ) + settings.server + ":" + CORE::UInt16ToString( settings.port ) + path;
    if ( settings.useTls )
        m_webSocket.SetTlsSettings( settings.tls );

    CWebSocketClient::TStringVector subProtocols;
    subProtocols.push_back( SubProtocol );
    return m_webSocket.Connect( url, subProtocols );
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::Disconnect( void )
{GUCEF_TRACE;

    if ( CWebSocketClient::STATE_OPEN == m_webSocket.GetState() )
        m_webSocket.Close();
    else
        m_webSocket.Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CIrcWebSocketTransport::SendLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( !m_isConnected )
        return false;
    return m_webSocket.SendText( line );
}

/*-------------------------------------------------------------------------*/

bool
CIrcWebSocketTransport::IsConnected( void ) const
{GUCEF_TRACE;

    return m_isConnected;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcWebSocketTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return WebSocketTransportName;
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::OnWebSocketConnected( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // A server that picked another sub protocol speaks something else than IRC lines in text messages
    if ( !m_webSocket.GetSubProtocol().IsNULLOrEmpty() && m_webSocket.GetSubProtocol() != SubProtocol )
    {
        TTransportErrorEventData eData( "The server did not accept the " + SubProtocol + " sub protocol" );
        if ( !NotifyObservers( TransportErrorEvent, &eData ) ) return;
        m_webSocket.Close();
        return;
    }
    m_isConnected = true;
    NotifyObservers( ConnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::OnWebSocketTextMessage( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    const CORE::CString& text = static_cast< TWebSocketTextMessageEventData* >( eventData )->GetData().text;

    // One line per message, a sender may still end it with CR LF
    const char* str = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    while ( size > 0 && ( '\r' == str[ size-1 ] || '\n' == str[ size-1 ] ) )
        --size;
    if ( 0 == size )
        return;

    TLineReceivedEventData eData( size == text.ByteSize() - 1 ? text : CORE::CString( str, size ) );
    NotifyObservers( LineReceivedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::OnWebSocketClosed( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_isConnected = false;
    NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CIrcWebSocketTransport::OnWebSocketError( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString description = GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CORE::CString( "WebSocket error" );
    TTransportErrorEventData eData( description );
    NotifyObservers( TransportErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcWebSocketTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CIrcWebSocketTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
