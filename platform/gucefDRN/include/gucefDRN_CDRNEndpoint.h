/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#define GUCEF_DRN_CDRNENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where a channel connects to or listens on.
 *
 *  The URI form is <scheme>://<address>[?key=value&key=value]: tcp://host:port, udp://host:port,
 *  ws://host:port/path (channel type "websocket"), relay://relayhost:port/session/peerId.
 *  The address is everything between "://" and the parameters. Unknown schemes are channel types of their own
 *  (plugins, test decorators).
 */
class GUCEF_DRN_EXPORT_CPP CDRNEndpoint
{
    public:

    static const CORE::CString ChannelTypeTcp;
    static const CORE::CString ChannelTypeUdp;
    static const CORE::CString ChannelTypeWebSocket;
    static const CORE::CString ChannelTypeRelay;

    CDRNEndpoint( void );

    CDRNEndpoint( const CORE::CString& channelType, const CORE::CString& address );

    /** Parses the URI form, see IsValid() for the outcome */
    explicit CDRNEndpoint( const CORE::CString& uri );

    CDRNEndpoint( const CDRNEndpoint& src );

    ~CDRNEndpoint();

    CDRNEndpoint& operator=( const CDRNEndpoint& src );

    bool operator==( const CDRNEndpoint& other ) const;
    bool operator!=( const CDRNEndpoint& other ) const;
    bool operator<( const CDRNEndpoint& other ) const;

    bool FromString( const CORE::CString& uri );

    CORE::CString ToString( void ) const;

    bool IsValid( void ) const;

    void Clear( void );

    void SetChannelType( const CORE::CString& channelType );
    const CORE::CString& GetChannelType( void ) const;

    void SetAddress( const CORE::CString& address );
    const CORE::CString& GetAddress( void ) const;

    CORE::CValueList& GetParams( void );
    const CORE::CValueList& GetParams( void ) const;

    /** The host part of a host:port[/path] address */
    CORE::CString GetHost( void ) const;

    /** The port of a host:port[/path] address, 0 if there is none */
    UInt16 GetPort( void ) const;

    /** The path of a host:port/path address including the leading '/', empty if there is none */
    CORE::CString GetPath( void ) const;

    /** The URI scheme used for a channel type: "ws" for "websocket", the type itself otherwise */
    static CORE::CString SchemeForChannelType( const CORE::CString& channelType );

    /** The channel type for a URI scheme: "websocket" for "ws", the scheme itself otherwise */
    static CORE::CString ChannelTypeForScheme( const CORE::CString& scheme );

    private:

    CORE::CString m_channelType;
    CORE::CString m_address;
    CORE::CValueList m_params;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */
