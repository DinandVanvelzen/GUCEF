/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#include "gucefWEB_CIMcpHttpAuthenticator.h"
#define GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#endif /* GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H ? */

#ifndef GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H
#include "gucefWEB_CMcpHttpTransportSettings.h"
#define GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H
#endif /* GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H ? */

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

class CMcpHttpExchangeChannel;

/**
 *  The Streamable HTTP transport of MCP (factory name "streamable-http"): one endpoint path that takes a JSON-RPC
 *  request or notification per POST and answers each request with a JSON body or with an SSE stream scoped to it.
 *  There are no sessions: every POST is a channel of its own (CMcpHttpExchangeChannel), closed with its answer.
 *  Closing the stream of a modern request cancels it.
 *
 *  The transport owns a CHTTPServer on the pulse generator of the MCP server, with a request takeover route for its
 *  path. Before the body of a POST is read it checks, in this order: Host (403), Origin when present (403),
 *  the method (405 for anything but POST, with Allow: POST), authentication (401), Content-Type application/json
 *  (415), Content-Length (411; 413 when larger than allowed), Accept (406 when it lists neither JSON nor SSE).
 *
 *  It binds to 127.0.0.1 by default. Another address needs TLS (a certificate and key in the settings: HTTPS) or
 *  allowNonLoopback, which serves plain HTTP on the network. Remote clients name the host in Host, which has to be
 *  one of allowedHosts.
 *  Binding to loopback is no authentication: every local process and every web page can reach it, hence Host and
 *  Origin validation (DNS rebinding) and the authenticator ("static-bearer" by default).
 */
class GUCEF_WEB_PUBLIC_CPP CMcpStreamableHttpTransport : public COM::CIMcpTransport      ,
                                                         private CIHttpRequestTakeoverHandler
{
    public:

    CMcpStreamableHttpTransport( void );

    virtual ~CMcpStreamableHttpTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** Before Start() */
    void SetSettings( const CMcpHttpTransportSettings& settings );

    const CMcpHttpTransportSettings& GetSettings( void ) const;

    /** An authenticator made by the application, instead of the one the settings name. Before Start() */
    void SetAuthenticator( const CIMcpHttpAuthenticatorPtr& authenticator );

    /** The authenticator in use, or the one that will be */
    CIMcpHttpAuthenticatorPtr GetAuthenticator( void ) const;

    /** The port in use: that of the settings, or the free one picked for port 0. 0 while not active */
    UInt16 GetBoundPort( void ) const;

    /** The HTTP server while the transport is active, GUCEF_NULL otherwise */
    CHTTPServer* GetHttpServer( void ) const;

    /** The POSTs that are not answered yet */
    UInt32 GetOpenExchangeCount( void ) const;

    COM::CMcpServer* GetServer( void ) const;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( COM::CMcpServer& server ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( bool graceful ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetEndpointDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Whether the Host field of a request names this endpoint: one of the allowed host names (without regard to
     *  case, IPv6 in brackets) with the bound port. An empty list allows localhost, 127.0.0.1 and [::1].
     */
    static bool IsHostAllowed( const CORE::CString& hostField, const CORE::CString::StringVector& allowedHosts, UInt16 boundPort );

    /** Whether the Origin field of a request is one of the allowed origins (RFC 6454 same origin); "null" never is */
    static bool IsOriginAllowed( const CORE::CString& originField, const CORE::CString::StringVector& allowedOrigins );

    private:

    friend class CMcpHttpExchangeChannel;

    typedef GUCEF::map< UInt64, COM::CMcpChannelPtr > TChannelMap;
    typedef GUCEF::vector< COM::CMcpChannelPtr > TChannelVector;

    CMcpStreamableHttpTransport( const CMcpStreamableHttpTransport& src );
    CMcpStreamableHttpTransport& operator=( const CMcpStreamableHttpTransport& src );

    // CIHttpRequestTakeoverHandler
    virtual EHeaderDecision OnHttpRequestHeaders( CHTTPServer& server                 ,
                                                  const CHttpRequestData& request     ,
                                                  const CHttpServerExchange& exchange ,
                                                  CHttpResponseData& response         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpRequest( CHTTPServer& server             ,
                                const CHttpRequestData& request ,
                                CHttpServerExchange& exchange   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpExchangeClosed( CHTTPServer& server           ,
                                       CHttpServerExchange& exchange ) GUCEF_VIRTUAL_OVERRIDE;

    /** From a channel: its answer went out, the channel closes at the server on the next pulse */
    void OnChannelDone( CMcpHttpExchangeChannel& channel );

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void CloseDoneChannels( void );

    void Release( void );

    void RaiseError( const CORE::CString& problem );

    static void SetJsonRpcErrorBody( CHttpResponseData& response, Int32 code, const CORE::CString& message );

    /** Audits a refusal before dispatch at the server: "HTTP <status>: <why>", the principal as not authenticated */
    void AuditRefusal( int status, const CORE::CString& why, Int32 errorCode, const COM::CMcpPrincipal* principal = GUCEF_NULL );

    /** Audits a refusal of the authenticator, with what its challenge tells of the cause */
    void AuditAuthenticationRefusal( const COM::CMcpPrincipal& principal, const CHttpResponseData& challenge );

    COM::CMcpServer* m_server;
    CHTTPServer* m_httpServer;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CMcpHttpTransportSettings m_settings;
    CIMcpHttpAuthenticatorPtr m_authenticator;
    bool m_authenticatorGiven;
    CORE::CDataNode m_authenticatorConfig;  /**< the configuration entry, for an authenticator made at Start() */
    TChannelMap m_channels;                 /**< by exchange id: the POSTs that are not answered yet */
    TChannelVector m_doneChannels;          /**< answered, to be closed at the server on the next pulse */
    bool m_active;
    UInt32 m_keepAliveMs;
    UInt64 m_nextKeepAliveInMs;
};

typedef CORE::CTFactory< COM::CIMcpTransport, CMcpStreamableHttpTransport, MT::CMutex > TMcpStreamableHttpTransportFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */
