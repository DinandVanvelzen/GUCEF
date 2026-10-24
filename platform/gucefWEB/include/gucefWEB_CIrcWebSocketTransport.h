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

#ifndef GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H
#define GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIIRCTRANSPORT_H
#include "gucefCOM_CIIrcTransport.h"
#define GUCEF_COM_CIIRCTRANSPORT_H
#endif /* GUCEF_COM_CIIRCTRANSPORT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

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
 *  IRC over a WebSocket (the IRCv3 WebSocket spec): the text.ircv3.net sub protocol, one line per text message
 *  without CR LF. The url is ws://<server>:<port><webSocketPath> from the settings.
 *  gucefWEB registers it with the IRC transport factory of COM::CComGlobal as "websocket".
 */
class GUCEF_WEB_PUBLIC_CPP CIrcWebSocketTransport : public COM::CIIrcTransport
{
    public:

    static const CORE::CString SubProtocol;     /**< "text.ircv3.net" */

    CIrcWebSocketTransport( void );

    explicit CIrcWebSocketTransport( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CIrcWebSocketTransport();

    virtual bool Connect( const COM::CIrcClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIrcWebSocketTransport( const CIrcWebSocketTransport& src );
    CIrcWebSocketTransport& operator=( const CIrcWebSocketTransport& src );

    private:

    CWebSocketClient m_webSocket;
    bool m_isConnected;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COM::CIIrcTransport, CIrcWebSocketTransport, MT::CMutex > TIrcWebSocketTransportFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H ? */
