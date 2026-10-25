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

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H
#define GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */

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
 *  Client side settings for performing a single HTTP request, complementing the CHttpRequestData describing
 *  the request message itself.
 *
 *  The body of the request and the response can be streamed via CIOAccess objects so that uploads and downloads
 *  of any size do not have to be held in memory. These are shared pointers since an asynchronous request can
 *  outlive the code that started it.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpClientRequestOptions : public CORE::CICloneable
{
    public:

    /** body of the request, read from its current position till its end. Used instead of the request content when set */
    CORE::IOAccessPtr requestBody;

    /** the response body is written here instead of into the response content when set */
    CORE::IOAccessPtr responseBodySink;

    /** request only part of the resource: rangeStart and rangeLength bytes, -1 for everything after rangeStart */
    bool hasRange;
    UInt64 rangeStart;
    Int64 rangeLength;

    /** maximum time for establishing the connection, 0 for no limit */
    UInt32 connectTimeoutInMs;

    /** maximum time for the whole request including redirects, 0 for no limit */
    UInt32 totalTimeoutInMs;

    bool followRedirects;
    UInt32 maxRedirects;

    /**
     *  whether the certificate of a https server is verified, only to be switched off for testing.
     *  With false the native backend uses CTlsSettings::DisableVerificationInsecurely(), which logs a warning for every connection
     */
    bool verifyPeer;

    /**
     *  File with the certificates of the authorities that are trusted to vouch for the certificate of a https server (PEM).
     *  Empty: the backend uses the trust store it finds itself, which for the curl backend and the native backend is the environment
     *  variable SSL_CERT_FILE when set and otherwise the bundle of the operating system (/etc/ssl/certs/ca-certificates.crt,
     *  /etc/pki/tls/certs/ca-bundle.crt or /etc/ssl/cert.pem). WinHTTP uses the certificate store of Windows and ignores this.
     *  When given, only these authorities are trusted: the trust store of the system is not used then.
     *  A path that cannot be read makes https requests fail: the verification is never skipped because of it.
     */
    CString caBundlePath;

    /** cookies to send and to store cookies received in, optional */
    CHttpCookieJarPtr cookieJar;

    /** proxy server to use, empty for none. An https URL is reached through it with a CONNECT tunnel */
    CString proxyHost;
    UInt16 proxyPort;

    /**
     *  Whether the native backend takes the proxy from the environment (HTTPS_PROXY, HTTP_PROXY, NO_PROXY, see
     *  CHttpProxyEnvironment) when proxyHost is empty. True by default; a host that is this computer is never proxied.
     *  The other backends decide for themselves: libcurl reads the same variables and WinHTTP the settings of the system
     */
    bool useEnvironmentProxy;

    CString userAgent;

    CHttpClientRequestOptions( void );
    CHttpClientRequestOptions( const CHttpClientRequestOptions& src );
    virtual ~CHttpClientRequestOptions() GUCEF_VIRTUAL_OVERRIDE;
    CHttpClientRequestOptions& operator=( const CHttpClientRequestOptions& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const CString DefaultUserAgent;
};

/*-------------------------------------------------------------------------*/

/**
 *  What kind of failure ended a request that got no response, the same for every backend
 */
enum EHttpTransferErrorClass
{
    HTTPERRORCLASS_NONE = 0,    /**< no failure, or the request was cancelled */
    HTTPERRORCLASS_NO_BACKEND,  /**< no backend is available for the scheme of the URL, for https a plugin such as httpclientpluginCURL is not loaded */
    HTTPERRORCLASS_DNS,         /**< the name of the host could not be resolved */
    HTTPERRORCLASS_CONNECT,     /**< the connection to the host could not be made, or was lost */
    HTTPERRORCLASS_TLS,         /**< the secure connection failed: the certificate is not trusted or does not match the host, or the handshake failed */
    HTTPERRORCLASS_TIMEOUT,     /**< the host did not answer in time */
    HTTPERRORCLASS_OTHER        /**< anything else */
};

typedef enum EHttpTransferErrorClass THttpTransferErrorClass;

/** A stable lower case name of an error class, for logs */
GUCEF_WEB_PUBLIC_CPP const char*
HttpTransferErrorClassToString( THttpTransferErrorClass errorClass );

/*-------------------------------------------------------------------------*/

/**
 *  Information about how a HTTP request was carried out, as a complement to the response
 */
class GUCEF_WEB_PUBLIC_CPP CHttpClientTransferInfo : public CORE::CICloneable
{
    public:

    /** whether a response was received, also true for responses with an error status code */
    bool success;
    Int32 errorCode;
    CString errorDescription;

    /** the kind of failure, filled by the backends and the client for every request that got no response */
    THttpTransferErrorClass errorClass;
    bool wasCancelled;
    bool timedOut;

    /** the URL of the final response, after redirects */
    CString effectiveUrl;
    UInt32 redirectCount;
    UInt64 bytesSent;
    UInt64 bytesReceived;
    UInt32 durationInMs;
    CString backendName;

    CHttpClientTransferInfo( void );
    virtual ~CHttpClientTransferInfo() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTOPTIONS_H ? */
