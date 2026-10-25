/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H
#define GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CLoopbackHandlerFactory;

/**
 *  The place a browser is sent back to after a login (RFC 8252, section 7.3): a small HTTP server on 127.0.0.1 only that
 *  takes exactly one valid callback and then stays quiet.
 *
 *  A request is valid when
 *  - the method is GET, the path is the configured one and the Host is 127.0.0.1:port or localhost:port (a page on
 *    another origin cannot make the browser deliver a callback by rebinding a name to the loopback address),
 *  - the query has the state of the login, compared in constant time, and a code, an error or an OpenID 2.0 response.
 *  Anything else is answered with an error page that says nothing about state or code and leaves the listener waiting.
 *  The valid callback is answered with a page that says the application can be returned to; after that every request,
 *  valid or not, is answered with 404. The listener closes itself a moment after the callback, at the deadline, and
 *  when it is closed or destroyed. Nothing of a request is written to a log.
 *
 *  The listener runs on the global pulse generator: Start(), Poll() and Close() are called on the thread that pumps it.
 */
class GUCEF_AUTH_EXPORT_CPP CLoopbackRedirectListener
{
    public:

    enum EState
    {
        STATE_IDLE = 0,             /**< not started */
        STATE_LISTENING,
        STATE_RECEIVED,             /**< the callback arrived */
        STATE_TIMED_OUT,            /**< the deadline passed without one */
        STATE_CLOSED                /**< closed by the owner */
    };

    static const UInt32 CloseDelayAfterCallbackInMs = 100;

    CLoopbackRedirectListener( void );
    ~CLoopbackRedirectListener();

    /**
     *  Starts listening on 127.0.0.1. Port 0 takes a free port. The path starts with a slash. The state is what the query of the callback has to carry
     *  as the parameter stateParameter. The listener gives up after timeoutInMs (0 is never). False when it cannot listen, for example when the fixed port is taken.
     */
    bool Start( UInt16 port                             ,
                const CORE::CString& path               ,
                const CSecretString& expectedState      ,
                UInt32 timeoutInMs                      ,
                const CORE::CString& stateParameter = "state" );

    /** Looks at the clock: closes the listener at the deadline and a moment after the callback. Returns the state */
    EState Poll( void );

    EState GetState( void ) const;

    /** The port it listens on, 0 when it does not */
    UInt16 GetPort( void ) const;

    /** http://127.0.0.1:port/path */
    CORE::CString GetRedirectUri( void ) const;

    /** http://127.0.0.1:port */
    CORE::CString GetOrigin( void ) const;

    /** Hands out the parameters of the callback, decoded; after this the listener holds none. False when there is no callback */
    bool TakeCallback( CORE::CString::StringMap& parameters );

    /** Stops listening. Not from inside a request */
    void Close( void );

    /** How many requests were turned away */
    UInt32 GetRejectedCount( void ) const;

    /** How many requests arrived at all */
    UInt32 GetRequestCount( void ) const;

    /** The handler's entry point: answers one request. Public for the handler class only */
    void HandleRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response );

    private:

    CLoopbackRedirectListener( const CLoopbackRedirectListener& src );              /**< not implemented */
    CLoopbackRedirectListener& operator=( const CLoopbackRedirectListener& src );   /**< not implemented */

    void Reject( WEB::CHttpResponseData& response, int statusCode, const char* text );
    void FillPage( WEB::CHttpResponseData& response, int statusCode, const CORE::CString& message ) const;

    private:

    WEB::CHTTPServer* m_server;
    CLoopbackHandlerFactory* m_factory;
    EState m_state;
    UInt16 m_port;
    CORE::CString m_path;
    CORE::CString m_stateParameter;
    CSecretString m_expectedState;
    UInt64 m_deadlineMs;
    UInt64 m_receivedAtMs;
    bool m_serverClosed;
    CORE::CString::StringMap m_parameters;
    UInt32 m_rejectedCount;
    UInt32 m_requestCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H ? */
