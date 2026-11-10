/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H
#include "gucefWEB_CHttpClientRequestOptions.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H ? */

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
 *  Receives the progress of a request carried out by a HTTP client backend.
 *  All calls are made from the thread of the pulse generator the request runs on.
 *  This is a plain interface rather than events so that no event data is created for each block of data.
 */
class GUCEF_WEB_PUBLIC_CPP CIHttpClientBackendObserver
{
    public:

    virtual ~CIHttpClientBackendObserver();

    /** the connection to the server, or the proxy, is established */
    virtual void OnHttpClientConnected( void );

    /** a response header line, called for each header including repeated ones such as Set-Cookie */
    virtual void OnHttpClientResponseHeader( const CString& name, const CString& value );

    /** all headers of the response are received, the response object holds the status and headers */
    virtual void OnHttpClientResponseHeadersComplete( const CHttpResponseData& response );

    /** a block of the response body, after it was written to the sink or the response content */
    virtual void OnHttpClientBodyData( const void* data, UInt32 dataSize );

    /** a number of bytes of the request was sent */
    virtual void OnHttpClientDataSent( UInt32 bytesSent );

    /** the request is redirected to the given URL. Reported by CHTTPClient which follows redirects, never by backends */
    virtual void OnHttpClientRedirect( const CString& newUrl );
};

/*-------------------------------------------------------------------------*/

/**
 *  A request being carried out by a backend. It makes progress on the pulses of the pulse generator it was
 *  started with, the owner polls IsFinished(). Not thread-safe: use it from the thread of that pulse generator.
 */
class GUCEF_WEB_PUBLIC_CPP CIHttpClientBackendRequest
{
    public:

    virtual ~CIHttpClientBackendRequest();

    virtual bool IsFinished( void ) const = 0;

    /** whether a connection to the server was established yet */
    virtual bool IsConnected( void ) const = 0;

    /** stops the request, it is finished afterwards and the transfer info reports the cancellation */
    virtual void Cancel( void ) = 0;
};

typedef CORE::CTSharedPtr< CIHttpClientBackendRequest, MT::CMutex > CIHttpClientBackendRequestPtr;

/*-------------------------------------------------------------------------*/

/**
 *  A way of carrying out HTTP requests, provided by the platform or by plugins, for example using curl.
 *  Backends are created via the factory of CWebGlobal and used by CHTTPClient.
 *
 *  A backend carries out exactly one HTTP exchange per request:
 *  - Redirects are not followed, cookies are not handled and the request headers are sent as given, including any
 *    Cookie or Range header. CHTTPClient takes care of these for all backends alike.
 *  - The response body is written to options.responseBodySink when set, otherwise it is appended to the response
 *    content. The body of a redirect response (3xx with a Location header) is discarded when options.followRedirects
 *    is set, since it is not the content that was asked for.
 *  - The request body is options.requestBody when set, otherwise the request content.
 *  - The transfer info gets success, the error details and the byte counters. Success means a response was
 *    received, regardless of its status code.
 *
 *  StartRequest() can be called from multiple threads at the same time, each request runs on the thread of the
 *  pulse generator it is given.
 */
class GUCEF_WEB_PUBLIC_CPP CIHttpClientBackend
{
    public:

    virtual ~CIHttpClientBackend();

    virtual CString GetBackendName( void ) const = 0;

    /** whether URLs with the given lowercase scheme, for example "https", can be requested */
    virtual bool SupportsScheme( const CString& scheme ) const = 0;

    /**
     *  Starts the request. The request, options, response, transfer info and observer have to remain available
     *  until the returned request is finished.
     *
     *  @return the request in progress, or a NULL pointer if it could not be started (the transfer info says why)
     */
    virtual CIHttpClientBackendRequestPtr StartRequest( const CHttpRequestData& request          ,
                                                        const CHttpClientRequestOptions& options ,
                                                        CHttpResponseData& response              ,
                                                        CHttpClientTransferInfo& transferInfo    ,
                                                        CORE::PulseGeneratorPtr pulseGenerator   ,
                                                        CIHttpClientBackendObserver& observer    ) = 0;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CString, CIHttpClientBackend, MT::CMutex > THttpClientBackendFactory;
typedef THttpClientBackendFactory::TProductPtr                                CIHttpClientBackendPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */
