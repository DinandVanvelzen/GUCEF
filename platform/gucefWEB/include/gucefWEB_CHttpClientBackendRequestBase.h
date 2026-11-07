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

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#define GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#include "gucefWEB_CIHttpClientBackend.h"
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */

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
 *  Common implementation for requests of HTTP client backends.
 *  It takes care of everything that does not depend on how the bytes travel, so that all backends behave alike:
 *  - what is sent: the method, the header fields and the request body (streamed from a source or from memory)
 *  - what is received: the status, mapping the header fields onto the response, discarding the body of a redirect
 *    that will be followed and writing the body to the sink or the response content
 *  - the transfer info and the observer calls that go with each of these
 *
 *  A backend derives from this, moves the bytes and calls the protected members as things happen.
 *  Like the interface it is not thread-safe, it is used from the thread of the pulse generator of the request.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpClientBackendRequestBase : public CIHttpClientBackendRequest
{
    public:

    CHttpClientBackendRequestBase( const CHttpRequestData& request          ,
                                   const CHttpClientRequestOptions& options ,
                                   CHttpResponseData& response              ,
                                   CHttpClientTransferInfo& transferInfo    ,
                                   CIHttpClientBackendObserver& observer    );

    virtual ~CHttpClientBackendRequestBase();

    virtual bool IsFinished( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** marks the request as cancelled and finishes it, OnFinish() lets the backend stop the transfer */
    virtual void Cancel( void ) GUCEF_VIRTUAL_OVERRIDE;

    static bool IsRedirectStatusCode( int statusCode );

    protected:

    /** the method in upper case, GET when none is given */
    const CString& GetRequestMethod( void ) const;

    bool IsHeadRequest( void ) const;

    /** whether a request body is sent: when there is one, or when the method normally has one (POST, PUT, PATCH) */
    bool HasRequestBody( void ) const;

    /** whether the body is read from options.requestBody rather than taken from the request content */
    bool IsRequestBodyStreamed( void ) const;

    /** whether the size of the body is known up front, otherwise it has to be sent chunked */
    bool IsRequestBodySizeKnown( void ) const;

    /** the size of the body, valid when IsRequestBodySizeKnown() */
    UInt64 GetRequestBodySize( void ) const;

    /**
     *  Reads the next part of the request body
     *  @return the number of bytes read, 0 at the end of the body
     */
    UInt32 ReadRequestBody( void* dest, UInt32 maxBytes );

    /**
     *  Goes back to the start of the request body so that it can be sent again, for example when a connection that
     *  was reused turned out to be closed already.
     *  @return false if the body source does not allow going back
     */
    bool RewindRequestBody( void );

    /** how much of the request body was read so far */
    UInt64 GetRequestBodyBytesRead( void ) const;

    /**
     *  Provides the header fields of the request as "Name: value" lines without line ends: the fields given in the
     *  request except Host, Connection, Content-Length and Transfer-Encoding, which belong to the transport, plus
     *  User-Agent, Accept, Accept-Encoding and Content-Type derived from the request and options.
     */
    void GetRequestHeaderFields( CString::StringVector& fields ) const;

    /** the connection is established */
    void OnConnected( void );

    /** a number of bytes of the request was sent */
    void OnDataSent( UInt32 bytesSent );

    /** a number of bytes was received, counting everything including headers and framing */
    void OnDataReceived( UInt32 bytesReceived );

    /**
     *  A status line such as "HTTP/1.1 200 OK" starts a new response, any earlier one (an interim 1xx response)
     *  is forgotten. Line ends are ignored.
     *  @return false if the line is not a status line
     */
    bool OnResponseStatusLine( const char* line, UInt32 length );

    /**
     *  A header line "Name: value" of the response. Line ends are ignored.
     *  A line starting with white space continues the previous field (obsolete line folding).
     */
    void OnResponseHeaderLine( const char* line, UInt32 length );

    /** a header field of the response, sets the matching response member and the observer is told */
    void OnResponseHeader( const CString& name, const CString& value );

    /**
     *  All header fields of the final response are received. Interim (1xx) responses have to be skipped by the
     *  backend, the next status line replaces them.
     *  A Content-Length over options.maxResponseBodySize finishes the request here: the backend checks IsFinished() after it.
     */
    void OnResponseHeadersComplete( void );

    /**
     *  A block of the response body: written to the sink or appended to the response content, unless the body
     *  belongs to a redirect that will be followed. A block that would make the body larger than
     *  options.maxResponseBodySize is not taken: the request is finished as too large then.
     *  @return false if writing failed or the body is too large, the request is finished then
     */
    bool OnResponseBody( const void* data, UInt32 dataSize );

    /**
     *  Ends the request. Calls OnFinish() so the backend can stop its transfer, only the first call counts.
     *  @param errorCode backend specific code for the error, ignored on success
     */
    void Finish( bool success, const CString& errorDescription, Int32 errorCode = -1 );

    /** called once when the request finishes, before Finish() returns */
    virtual void OnFinish( void );

    bool HaveResponseHeadersCompleted( void ) const;

    /** whether the response said how long its body is, see GetResponseContentLength() */
    bool HasResponseContentLength( void ) const;

    UInt64 GetResponseContentLength( void ) const;

    bool IsResponseChunked( void ) const;

    /** whether the response body is discarded because the response is a redirect that will be followed */
    bool IsResponseBodyDiscarded( void ) const;

    protected:

    const CHttpRequestData& m_request;
    const CHttpClientRequestOptions& m_options;
    CHttpResponseData& m_response;
    CHttpClientTransferInfo& m_transferInfo;
    CIHttpClientBackendObserver& m_observer;

    private:

    CHttpClientBackendRequestBase( void );                                                   /**< not implemented */
    CHttpClientBackendRequestBase( const CHttpClientBackendRequestBase& src );               /**< not implemented */
    CHttpClientBackendRequestBase& operator=( const CHttpClientBackendRequestBase& src );    /**< not implemented */

    /** finishes the request because its body is larger than options.maxResponseBodySize */
    void FinishResponseBodyTooLarge( UInt64 bodySize );

    CString m_method;
    bool m_isHeadRequest;
    bool m_connected;
    bool m_finished;
    bool m_headersComplete;
    bool m_discardBody;
    bool m_hasContentLength;
    bool m_chunked;
    UInt64 m_contentLength;
    UInt64 m_responseBodySize;
    CString m_lastHeaderName;
    CORE::IOAccessPtr m_bodySource;
    bool m_bodySizeKnown;
    UInt64 m_bodySize;
    UInt64 m_bodyOffset;
    UInt64 m_bodyStartPos;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H ? */
