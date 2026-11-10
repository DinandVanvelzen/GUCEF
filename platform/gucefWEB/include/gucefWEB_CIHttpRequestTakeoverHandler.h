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

#ifndef GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#define GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

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

class CHTTPServer;
class CHttpServerExchange;

/**
 *  Takes over the requests to one exact path of a CHTTPServer, before the router and before the web socket
 *  upgrade: see CHTTPServer::RegisterRequestTakeoverHandler(). The server reads the request, applies its limits
 *  and hands the request with an exchange to the handler, which answers through the exchange whenever it likes:
 *  at once, later from a timer or a task completion, or as a stream (an SSE stream for example).
 *
 *  Every callback is made on the thread that drives the pulse generator of the server, in order, and never from
 *  within a call the handler makes to an exchange or to the server: a handler can answer from inside a callback.
 */
class GUCEF_WEB_PUBLIC_CPP CIHttpRequestTakeoverHandler
{
    public:

    enum EHeaderDecision
    {
        TAKEOVER_CONTINUE   ,   /**< read the body and call OnHttpRequest() */
        TAKEOVER_REJECT         /**< send the response that was filled in, which must have a status of 400 or above, and close */
    };

    CIHttpRequestTakeoverHandler( void ) {}

    virtual ~CIHttpRequestTakeoverHandler() {}

    /**
     *  The header block of a request is in and its body is not. Checks that refuse a request before its body is read
     *  belong here (origin, authentication, content type), so that a client that is refused cannot make the server
     *  buffer a body. The server applies the body limit of the route after this returns CONTINUE.
     *  'request' has no content yet. 'response' is only sent when REJECT is returned.
     */
    virtual EHeaderDecision OnHttpRequestHeaders( CHTTPServer& server                 ,
                                                  const CHttpRequestData& request     ,
                                                  const CHttpServerExchange& exchange ,
                                                  CHttpResponseData& response         ) = 0;

    /**
     *  The whole request is in. The handler now owns the exchange until it ends it (SendResponse(), EndStream()) or
     *  the client goes away (OnHttpExchangeClosed()). The request, its content included, is only valid during this
     *  call: copy what is needed later. Keep CHttpServerExchange::GetSharedPtr() to answer after the call returned.
     */
    virtual void OnHttpRequest( CHTTPServer& server             ,
                                const CHttpRequestData& request ,
                                CHttpServerExchange& exchange   ) = 0;

    /**
     *  The connection closed before the handler ended the exchange: the client went away, the server closed a stream
     *  that fell too far behind, or the server stopped. Called once per such exchange. Sends on the exchange fail
     *  from now on; the shared pointers to it stay valid.
     */
    virtual void OnHttpExchangeClosed( CHTTPServer& server           ,
                                       CHttpServerExchange& exchange ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H ? */
