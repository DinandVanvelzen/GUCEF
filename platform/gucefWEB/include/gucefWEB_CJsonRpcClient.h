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

#ifndef GUCEF_WEB_CJSONRPCCLIENT_H
#define GUCEF_WEB_CJSONRPCCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The calling side of a JSON-RPC conversation: an endpoint which times out calls on its own and can
 *  be attached to a web socket client as its transport. Any other transport can be used through SetTransport()
 *  in which case the owner of that transport gives incoming messages to ProcessIncoming().
 */
class GUCEF_WEB_PUBLIC_CPP CJsonRpcClient : public COM::CJsonRpcEndpoint
{
    public:

    /** Uses the application's (global) pulse generator */
    CJsonRpcClient( void );

    /** Uses the given pulse generator, which should be the one that drives the transport, for timing out calls and for CallAndWait() */
    explicit CJsonRpcClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CJsonRpcClient();

    /** Also moves the timer which times out calls to the given pulse generator */
    virtual void SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Uses the given web socket client, which must outlive this object or be detached, as the transport:
     *  messages are sent as text messages and the text messages received are processed. When the web socket
     *  closes the calls still waiting for an answer are failed.
     */
    void AttachWebSocketClient( CWebSocketClient& webSocketClient );

    void DetachWebSocketClient( void );

    private:

    CJsonRpcClient( const CJsonRpcClient& src );
    CJsonRpcClient& operator=( const CJsonRpcClient& src );

    class CWebSocketTransport : public COM::CIJsonRpcTransport
    {
        public:
        CWebSocketTransport( void ) : m_webSocketClient( GUCEF_NULL ) {}
        virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
            { return GUCEF_NULL != m_webSocketClient && m_webSocketClient->SendText( messageText ); }
        CWebSocketClient* m_webSocketClient;
    };

    class CEventSink : public CORE::CObserver
    {
        public:
        explicit CEventSink( CJsonRpcClient& client ) : CORE::CObserver(), m_client( client ) {}
        virtual void OnNotify( CORE::CNotifier* notifier                 ,
                               const CORE::CEvent& eventid               ,
                               CORE::CICloneable* eventdata /* = NULL */ ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CJsonRpcClient& m_client;
    };

    friend class CEventSink;

    private:

    CORE::CTimer m_timeoutTimer;
    CWebSocketTransport m_webSocketTransport;
    CEventSink m_eventSink;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CJSONRPCCLIENT_H ? */
