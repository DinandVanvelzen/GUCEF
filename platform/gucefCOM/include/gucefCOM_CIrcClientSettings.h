/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CIRCCLIENTSETTINGS_H
#define GUCEF_COM_CIRCCLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a CIrcClient connects and who it is on the server.
 *  In config: the members by the same name as attributes or child values, lists comma separated.
 */
class GUCEF_COM_PUBLIC_CPP CIrcClientSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString server;
    UInt16 port;                                    /**< 6667 by default, with useTls from a configuration 6697 unless the configuration gives a port */
    bool useTls;                                    /**< over TLS: IRC over TCP with TLS (port 6697), or wss:// for the WebSocket. The certificate of the server is verified */
    COMCORE::CTlsSettings tls;                      /**< how TLS is done when useTls: the CA to trust and the like. In config the child "Tls" */
    bool useWebSocket;                              /**< over a WebSocket with the text.ircv3.net sub protocol, when the transport is available */
    CORE::CString webSocketPath;                    /**< the resource of the WebSocket url, "/" by default */
    CORE::CString nick;
    CORE::CString::StringVector alternativeNicks;   /**< tried in order when the nick is in use */
    CORE::CString userName;                         /**< the nick when empty */
    CORE::CString realName;                         /**< the nick when empty */
    CORE::CString saslAccount;                      /**< SASL PLAIN authentication when set */
    CORE::CString saslPassword;
    CORE::CString::StringVector wantedCapabilities; /**< requested when the server offers them */
    CORE::CString::StringVector autoJoinChannels;   /**< joined once registered */
    UInt32 pingTimeoutInSecs;                       /**< silence after which the server is pinged, and after another the connection is given up */

    CIrcClientSettings( void );

    CIrcClientSettings( const CIrcClientSettings& src );

    virtual ~CIrcClientSettings();

    CIrcClientSettings& operator=( const CIrcClientSettings& src );

    /** The capabilities asked for by default: sasl, message-tags, server-time, echo-message, account-tag, batch, chathistory, multi-prefix */
    static void GetDefaultCapabilities( CORE::CString::StringVector& capabilities );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIRCCLIENTSETTINGS_H ? */
