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

#ifndef GUCEF_WEB_CHTTPSERVER_H
#define GUCEF_WEB_CHTTPSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#include "gucefWEB_CIWebSocketUpgradeHandler.h"
#define GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#endif /* GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H
#include "gucefWEB_CIHTTPServerRouterController.h"
#define GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#include "gucefWEB_CIHttpRequestTakeoverHandler.h"
#define GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#endif /* GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#include "gucefWEB_CHttpServerExchange.h"
#define GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#endif /* GUCEF_WEB_CHTTPSERVEREXCHANGE_H ? */

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

class GUCEF_WEB_PUBLIC_CPP CHttpServerSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    UInt32 maxClientConnections;
    bool keepAliveConnections;
    bool processRequestsAsync;
    bool allowWebSocketUpgrades;

    /**
     *  IPv4 address to listen on as dotted text, for example "127.0.0.1" to serve this machine only.
     *  Empty (the default) and "0.0.0.0" listen on every NIC. An address that is not a dotted IPv4 address
     *  makes Listen() fail rather than listening wider than was asked for.
     */
    CORE::CString bindAddress;

    /**
     *  Limits of a request that is coming in. A header block larger than maxHeaderBytes, or with more than
     *  maxHeaderCount fields, is answered with 431. A body larger than maxBodyBytes (or than the limit of the takeover
     *  route of its path) is answered with 413 before any of it is read. A header block that is not complete within
     *  headerTimeoutMs of its first byte, or a body that is not complete within bodyTimeoutMs of the end of the header
     *  block, is answered with 408. Each of these closes the connection.
     *  A connection on which no request is in progress and nothing arrived for idleTimeoutMs is closed without a
     *  response; 0 keeps such connections. Connections that were upgraded to web sockets, and requests a handler is
     *  working on, have no time limit here.
     */
    UInt32 maxHeaderBytes;
    UInt32 maxHeaderCount;
    UInt32 maxBodyBytes;
    UInt32 headerTimeoutMs;
    UInt32 bodyTimeoutMs;
    UInt32 idleTimeoutMs;

    /**
     *  The bytes a streamed response (CHttpServerExchange::SendStreamData()) may have queued that the socket did not
     *  take yet. A client that reads slower than that is closed
     */
    UInt32 maxStreamBacklogBytes;

    static const UInt32 DefaultMaxHeaderBytes = 16384;
    static const UInt32 DefaultMaxHeaderCount = 100;
    static const UInt32 DefaultMaxBodyBytes = 1048576;
    static const UInt32 DefaultHeaderTimeoutMs = 10000;
    static const UInt32 DefaultBodyTimeoutMs = 30000;
    static const UInt32 DefaultIdleTimeoutMs = 60000;
    static const UInt32 DefaultMaxStreamBacklogBytes = 8388608;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CHttpServerSettings( void );
    CHttpServerSettings( const CHttpServerSettings& src );
    virtual ~CHttpServerSettings();
    CHttpServerSettings& operator=( const CHttpServerSettings& src );
};

/*-------------------------------------------------------------------------*/

/** The state of one connection while HTTP is spoken on it: private to the module */
class CHttpServerConnectionState;

/*-------------------------------------------------------------------------*/

/**
 *  HTTP/1.1 server on a CTCPServerSocket.
 *
 *  A request is taken in over as many reads as it takes: the header block is collected up to maxHeaderBytes, the
 *  body by its Content-Length up to maxBodyBytes, and requests that follow each other on a connection (pipelined)
 *  are answered in order, the next one only after the response to the one before it went out. Request bodies with
 *  a Transfer-Encoding are not supported and get 501. Requests go to, in this order: the takeover handler of their
 *  exact path (RegisterRequestTakeoverHandler()), the web socket upgrade, the router of the request handler.
 *
 *  Everything happens on the thread that drives the pulse generator of the server, except router requests of a
 *  server with processRequestsAsync, which are handled as tasks. With processRequestsAsync the responses to
 *  pipelined router requests go out in the order the tasks finish.
 */
class GUCEF_WEB_PUBLIC_CPP CHTTPServer : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent WebSocketClientConnectedEvent;

    typedef CORE::CTLinkedCloneableObj< CHttpRequestData > TWebSocketClientConnectedEventData;

    static void RegisterEvents( void );

    public:

    typedef CString::StringVector TStringVector;

    CHTTPServer( THttpServerRequestHandlerFactory* requestHandlerFactory = GUCEF_NULL ,
                 const CHttpServerSettings* settings = GUCEF_NULL                     );

    CHTTPServer( const CORE::PulseGeneratorPtr& pulsGenerator                         ,
                 THttpServerRequestHandlerFactory* requestHandlerFactory = GUCEF_NULL ,
                 const CHttpServerSettings* settings = GUCEF_NULL                     );

    virtual ~CHTTPServer();

    void SetSettings( const CHttpServerSettings& settings );

    void GetSettings( CHttpServerSettings& settings );

    /**
     *  listen on default NIC with port given
     */
    bool ListenOnPort( const UInt16 port );

    bool Listen( void );

    /**
     *  The port the server is listening on, 0 when it is not. For a server that was told to listen on port 0
     *  this is the free port the system picked
     */
    UInt16 GetBoundPort( void ) const;

    bool IsActive( void ) const;

    /**
     *  stop listening for clients, close server socket
     */
    void Close( void );

    bool SetPort( UInt16 port );

    UInt16 GetPort( void ) const;

    CIHTTPServerRouterController* GetRouterController( void ) const;

    THttpServerRequestHandlerFactory::TProductPtr GetRequestHandler( void ) const;

    THttpServerRequestHandlerFactory* GetRequestHandlerFactory( void ) const;

    /**
     *  Sets the handler that takes over connections which successfully upgrade to the web socket protocol.
     *  Without a handler the upgrade is still accepted (when allowed by the settings) and the
     *  WebSocketClientConnectedEvent is sent but nobody will process the data of the connection.
     *  With a handler the connection is handed over once the upgrade is done: its data goes to the
     *  CIWebSocketConnectionHandler the handler linked to the connection.
     *  The handler must outlive the server or be unset (GUCEF_NULL) before it is destroyed.
     */
    void SetWebSocketUpgradeHandler( CIWebSocketUpgradeHandler* handler );

    /**
     *  Takes over the requests to one exact path: the request URI without its query. Such requests are checked before
     *  the router and before the web socket upgrade, so the handler also gets the upgrade requests for its path.
     *  @param maxBodyBytes the largest body the route takes, 0 for the maxBodyBytes of the settings
     *  The handler must stay valid while it is registered. Call on the thread of the server's pulse generator.
     *  @return false when the path is empty or has a handler already
     */
    bool RegisterRequestTakeoverHandler( const CORE::CString& exactPath, CIHttpRequestTakeoverHandler* handler, UInt32 maxBodyBytes );

    /**
     *  Ends a takeover route. The exchanges of the route that are still open are closed without telling the handler,
     *  which asked for this. Call on the thread of the server's pulse generator.
     */
    bool UnregisterRequestTakeoverHandler( const CORE::CString& exactPath );

    /** The pulse generator the server and its socket run on: its thread is the thread of every callback of the server */
    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    /** The exchanges of takeover routes that are not ended yet */
    UInt32 GetOpenExchangeCount( void ) const;

    /** The connections the server keeps HTTP state for: those that are connected and not upgraded to a web socket */
    UInt32 GetTrackedConnectionCount( void ) const;

    /** The bytes the server holds of requests that are coming in or wait behind the one being answered */
    UInt32 GetBufferedRequestBytes( void ) const;

    private:

    friend class CAsyncHttpServerRequestHandler;
    friend class CAsyncHttpServerResponseHandler;
    friend class CHttpServerExchange;

    /**
     *  Event callback member function.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ ) GUCEF_VIRTUAL_OVERRIDE;

    void ExtractCommaSeparatedValues( const CString& stringToExtractFrom ,
                                      TStringVector& list                ) const;

    void StripItems( TStringVector& list ,
                     char stripDelim     ) const;

    struct SRequestHeadInfo;
    struct SLimits;

    void ProcessConnectionData( CHttpServerConnectionState& state, const char* data, UInt32 dataSize );

    bool ProcessRequestHead( CHttpServerConnectionState& state, const char* head, UInt32 headSize, const SLimits& limits );

    void DispatchRequest( CHttpServerConnectionState& state, const char* rest, UInt32 restSize );

    void SyncProcessRouterRequest( CHttpServerConnectionState& state );

    void ASyncProcessRouterRequest( CHttpServerConnectionState& state );

    bool ParseRequestHead( const char* head, UInt32 headSize, CHttpRequestData& request, SRequestHeadInfo& info ) const;

    /** Answers with an error status and closes the connection once the answer is out */
    void RefuseRequest( CHttpServerConnectionState& state, int statusCode, const CORE::CString& reason );

    void SendRefusal( CHttpServerConnectionState& state, CHttpResponseData& response );

    void CloseAfterFlush( CHttpServerConnectionState& state );

    void ResetConnectionState( CHttpServerConnectionState& state );

    CHttpServerConnectionState* GetConnectionState( UInt32 connectionIndex ) const;

    CHttpServerConnectionState* GetExchangeState( const CHttpServerExchange& exchange ) const;

    void GetLimits( SLimits& limits ) const;

    void RequestPulseWork( void );

    UInt64 NowInMs( void ) const;

    // for CHttpServerExchange
    bool OnExchangeSendResponse( CHttpServerExchange& exchange, const CHttpResponseData& response );
    bool OnExchangeBeginStream( CHttpServerExchange& exchange, const CHttpResponseData& head );
    bool OnExchangeStreamData( CHttpServerExchange& exchange, const void* data, UInt32 size );
    bool OnExchangeEndStream( CHttpServerExchange& exchange );
    UInt32 GetExchangeBacklogBytes( const CHttpServerExchange& exchange ) const;

    UInt32 ParseHeaderFields( const char* bufferPtr       ,
                              const UInt32 bufferSize     ,
                              TStringVector& headerFields ) const;

    void RegisterEventHandlers( void );

    void ApplyBindAddress( void );

    void
    OnClientConnectedEvent( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void
    OnClientDataReceivedEvent( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    bool SendResponseASync( UInt32 connectionId                  ,
                            const CORE::CDynamicBuffer& response ,
                            const COMCORE::CIPv4Address& remoteIP  );

    void OnWebSocketUpgrade( COMCORE::CTCPServerConnection* connection ,
                             const CHttpRequestData& requestData       );

    void
    OnClientDisconnectedEvent( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void
    OnPulse( CORE::CNotifier* notifier    ,
             const CORE::CEvent& eventId  ,
             CORE::CICloneable* eventData );

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CHTTPServer( const CHTTPServer& src );             /**< not implemented */
    CHTTPServer& operator=( const CHTTPServer& src );  /**< not implemented */

    private:

    typedef CORE::CTEventHandlerFunctor< CHTTPServer > TEventCallback;

    struct STakeoverRoute
    {
        CIHttpRequestTakeoverHandler* handler;
        UInt32 maxBodyBytes;
    };
    typedef GUCEF::map< CORE::CString, STakeoverRoute > TTakeoverRouteMap;
    typedef GUCEF::vector< CHttpServerConnectionState* > TConnectionStateVector;

    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CTCPServerSocket m_tcpServerSocket;
    THttpServerRequestHandlerFactory::TProductPtr m_requestHandler;
    THttpServerRequestHandlerFactory* m_requestHandlerFactory;
    CHttpServerSettings m_settings;
    CIWebSocketUpgradeHandler* m_webSocketHandler;
    TTakeoverRouteMap m_takeoverRoutes;
    TConnectionStateVector m_connectionStates;     /**< by connection index: connection objects are reused, the states with them */
    bool m_hasPulseWork;                            /**< a connection waits for a resume or a close on the next pulse */
    UInt64 m_nextDeadlineCheckInMs;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

