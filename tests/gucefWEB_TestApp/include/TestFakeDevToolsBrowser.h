/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_WEB_TESTAPP_TESTFAKEDEVTOOLSBROWSER_H
#define GUCEF_WEB_TESTAPP_TESTFAKEDEVTOOLSBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_WEB_CJSONRPCSERVER_H
#include "gucefWEB_CJsonRpcServer.h"
#define GUCEF_WEB_CJSONRPCSERVER_H
#endif /* GUCEF_WEB_CJSONRPCSERVER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A browser on the loopback interface that speaks enough of the Chrome DevTools protocol for site discovery: the
 *  discovery pages (/json/version, /json/list) and a JSON-RPC web socket at /devtools/browser/B1 on the same port, which
 *  answers the Target, Network, Page and Runtime commands the recorder sends and sends the events a test scripts, with
 *  the session id of a tab in the envelope as Chrome does. Runs on the application's pulse generator.
 *
 *  Answers can be scripted per command (lasting, or once), and so can the events a command makes the browser send,
 *  before its answer or after it. Page.getNavigationHistory reports the address of the tab, which follows the
 *  Page.frameNavigated and Page.navigatedWithinDocument events of the main frame it sends.
 */
class CFakeDevToolsBrowser : public GUCEF::COM::CIJsonRpcRequestHandler
{
    public:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TStringMap;

    CFakeDevToolsBrowser( void );

    virtual ~CFakeDevToolsBrowser();

    /** Listens on a free port of the loopback interface */
    bool Start( void );

    GUCEF::UInt16 GetPort( void ) const;

    /** A target Target.getTargets lists; attaching to it gives the session "S-" + id */
    void AddTarget( const GUCEF::CORE::CString& targetId, const GUCEF::CORE::CString& type, const GUCEF::CORE::CString& url );

    void SetResponseBody( const GUCEF::CORE::CString& requestId, const GUCEF::CORE::CString& body );

    void SetRequestPostData( const GUCEF::CORE::CString& requestId, const GUCEF::CORE::CString& postData );

    /** Has a client sent a command over a connection, through which events can be sent */
    bool HasConnection( void ) const;

    /** Forgets the connection of an earlier client */
    void ForgetConnection( void );

    /** Sends an event, for a session (empty: the browser) */
    bool Emit( const GUCEF::CORE::CString& sessionId, const GUCEF::CORE::CString& method, const GUCEF::CORE::CDataNode& params );

    bool EmitRequest( const GUCEF::CORE::CString& sessionId, const GUCEF::CORE::CString& requestId, const GUCEF::CORE::CString& method,
                      const GUCEF::CORE::CString& url, const GUCEF::CORE::CString& type, const GUCEF::CORE::CString& frameId,
                      const TStringMap& headers, const GUCEF::CORE::CString& postData, bool hasPostData, GUCEF::Float64 timestamp );

    bool EmitResponse( const GUCEF::CORE::CString& sessionId, const GUCEF::CORE::CString& requestId, GUCEF::Int32 status,
                       const GUCEF::CORE::CString& mimeType, const TStringMap& headers, const GUCEF::CORE::CString& type );

    bool EmitFinished( const GUCEF::CORE::CString& sessionId, const GUCEF::CORE::CString& requestId, GUCEF::UInt32 size );

    /** "method@session" of every command received, in order */
    GUCEF::CORE::CString::StringVector commands;

    /** The parameters of every command received, in the order of commands */
    GUCEF::vector< GUCEF::CORE::CDataNode > commandParams;

    /**
     *  The result of a command, for the commands without a built-in answer. The key is the method, or the method and
     *  the value of its "role", "selector" or "accessibleName" parameter as "method#value", which is looked for first
     */
    void SetCommandResult( const GUCEF::CORE::CString& key, const GUCEF::CORE::CDataNode& result );

    /** Answers the command with this key (as SetCommandResult()) with a protocol error */
    void SetCommandError( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& message );

    /**
     *  An answer for the next command with this key only (as SetCommandResult()), looked at before the lasting answers
     *  and errors. Several queue up in order
     */
    void QueueCommandResult( const GUCEF::CORE::CString& key, const GUCEF::CORE::CDataNode& result );

    /** Answers only the next command with this key with a protocol error, before the lasting answers */
    void QueueCommandError( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& message );

    /**
     *  An event the browser sends on the session of every command with this key (as SetCommandResult()): before the
     *  answer, or after it on a later pulse (at least delayInMs later), as Chrome reports the commit and the load of a
     *  navigation after its answer. Events after the answer go out in the order they are due
     */
    void AddCommandEvent( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& method, const GUCEF::CORE::CDataNode& params,
                          bool afterAnswer, GUCEF::UInt32 delayInMs = 0 );

    /** Forgets the scripted answers, errors and events of the key */
    void ClearCommandScript( const GUCEF::CORE::CString& key );

    /**
     *  With this on, a Runtime.evaluate whose expression fetches an address of the write request guard's probe host makes
     *  the browser report that request as paused (Fetch.requestPaused on the browser target, a POST of resource type
     *  Fetch) before it answers "refused", as a browser with the guard's interception would
     */
    void SetEmulateProbeInterception( bool emulate );

    /** The parameters of the latest command of the method on the session (any session when '*') */
    bool GetLastCommandParams( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& sessionId, GUCEF::CORE::CDataNode& params ) const;

    /** How many commands of the method were sent on the session (any session when '*') */
    GUCEF::UInt32 CountCommands( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& sessionId ) const;

    bool WasCommandSent( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& sessionId ) const;

    bool WasAttachedTo( const GUCEF::CORE::CString& targetId ) const;

    virtual GUCEF::COM::EJsonRpcRequestResult OnJsonRpcRequest( GUCEF::COM::CJsonRpcEndpoint& endpoint     ,
                                                                const GUCEF::COM::SJsonRpcRequest& request ,
                                                                GUCEF::COM::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFakeDevToolsBrowser( const CFakeDevToolsBrowser& src );
    CFakeDevToolsBrowser& operator=( const CFakeDevToolsBrowser& src );

    struct STarget
    {
        GUCEF::CORE::CString id;
        GUCEF::CORE::CString type;
        GUCEF::CORE::CString url;
    };

    struct SScriptedAnswer
    {
        bool isError;
        GUCEF::CORE::CString message;
        GUCEF::CORE::CDataNode result;
    };

    struct SScriptedEvent
    {
        GUCEF::CORE::CString sessionId;
        GUCEF::CORE::CString method;
        GUCEF::CORE::CDataNode params;
        bool afterAnswer;
        GUCEF::UInt32 delayInMs;
        GUCEF::UInt64 dueInMs;
    };

    typedef GUCEF::vector< SScriptedAnswer > TScriptedAnswerVector;
    typedef GUCEF::vector< SScriptedEvent > TScriptedEventVector;

    /** Sends the events that come after an answer, on the pulse after it */
    class CPulseSink : public GUCEF::CORE::CObserver
    {
        public:

        explicit CPulseSink( CFakeDevToolsBrowser& owner );

        virtual ~CPulseSink();

        private:

        typedef GUCEF::CORE::CTEventHandlerFunctor< CPulseSink > TEventCallback;

        void OnPulse( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );

        CFakeDevToolsBrowser& m_owner;
    };

    void EmitScripted( const SScriptedEvent& event );
    void SendPendingEvents( void );

    GUCEF::WEB::THttpServerRequestHandlerFactory* m_handlerFactory;
    GUCEF::WEB::CHttpServerSettings m_httpSettings;
    GUCEF::WEB::CHTTPServer* m_httpServer;
    GUCEF::WEB::CWebSocketServer* m_webSocketServer;
    GUCEF::WEB::CJsonRpcServer* m_rpcServer;
    GUCEF::COM::CJsonRpcEndpoint* m_connection;
    GUCEF::vector< STarget > m_targets;
    TStringMap m_bodies;
    TStringMap m_postData;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CDataNode > m_results;
    TStringMap m_errors;
    GUCEF::map< GUCEF::CORE::CString, TScriptedAnswerVector > m_queued;
    GUCEF::map< GUCEF::CORE::CString, TScriptedEventVector > m_commandEvents;
    TScriptedEventVector m_pendingEvents;
    TStringMap m_tabUrls;
    CPulseSink* m_pulseSink;
    GUCEF::CORE::CString::StringSet m_attached;
    bool m_emulateProbe;
    GUCEF::UInt32 m_probeCount;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_TESTAPP_TESTFAKEDEVTOOLSBROWSER_H ? */
