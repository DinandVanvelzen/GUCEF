/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CJSONRPCCLIENT_H
#include "gucefWEB_CJsonRpcClient.h"
#define GUCEF_WEB_CJSONRPCCLIENT_H
#endif /* GUCEF_WEB_CJSONRPCCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CJsonRpcClient::CEventSink::OnNotify( CORE::CNotifier* notifier                 ,
                                      const CORE::CEvent& eventid               ,
                                      CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( eventid == CORE::CTimer::TimerUpdateEvent )
    {
        m_client.ExpireTimedOutCalls();
    }
    else
    if ( eventid == CWebSocketClient::TextMessageEvent )
    {
        TWebSocketTextMessageEventData* eData = static_cast< TWebSocketTextMessageEventData* >( eventdata );
        m_client.ProcessIncoming( eData->GetData().text );
    }
    else
    if ( eventid == CWebSocketClient::ClosedEvent )
    {
        m_client.FailAllPendingCalls( COM::JSONRPC_ERROR_TRANSPORT_CLOSED, "The connection was closed" );
    }
}

/*-------------------------------------------------------------------------*/

CJsonRpcClient::CJsonRpcClient( void )
    : COM::CJsonRpcEndpoint()
    , m_timeoutTimer( 250 )
    , m_webSocketTransport()
    , m_eventSink( *this )
{GUCEF_TRACE;

    m_eventSink.SubscribeTo( &m_timeoutTimer );
    m_timeoutTimer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

CJsonRpcClient::CJsonRpcClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COM::CJsonRpcEndpoint()
    , m_timeoutTimer( pulseGenerator, 250 )
    , m_webSocketTransport()
    , m_eventSink( *this )
{GUCEF_TRACE;

    COM::CJsonRpcEndpoint::SetPulseGenerator( pulseGenerator );
    m_eventSink.SubscribeTo( &m_timeoutTimer );
    m_timeoutTimer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

CJsonRpcClient::~CJsonRpcClient()
{GUCEF_TRACE;

    DetachWebSocketClient();
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcClient::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    COM::CJsonRpcEndpoint::SetPulseGenerator( pulseGenerator );
    m_timeoutTimer.SetPulseGenerator( pulseGenerator );
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcClient::AttachWebSocketClient( CWebSocketClient& webSocketClient )
{GUCEF_TRACE;

    DetachWebSocketClient();

    m_webSocketTransport.m_webSocketClient = &webSocketClient;
    SetTransport( &m_webSocketTransport );
    m_eventSink.SubscribeTo( &webSocketClient );
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcClient::DetachWebSocketClient( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_webSocketTransport.m_webSocketClient )
    {
        m_eventSink.UnsubscribeFrom( *m_webSocketTransport.m_webSocketClient );
        m_webSocketTransport.m_webSocketClient = GUCEF_NULL;
        SetTransport( GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
