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

#ifndef GUCEF_AUTH_COAUTH2HTTP_H
#define GUCEF_AUTH_COAUTH2HTTP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_AUTH_CAUTHOPTIONS_H
#include "gucefAUTH_CAuthOptions.h"
#define GUCEF_AUTH_CAUTHOPTIONS_H
#endif /* GUCEF_AUTH_CAUTHOPTIONS_H ? */

#ifndef GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#include "gucefAUTH_COAuth2EngineSettings.h"
#define GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#endif /* GUCEF_AUTH_COAUTH2ENGINESETTINGS_H ? */

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

/** One request of the engine to a provider */
class GUCEF_AUTH_EXPORT_CPP CAuthHttpRequest
{
    public:

    WEB::CHttpRequestData request;
    WEB::CHttpClientRequestOptions options;

    CAuthHttpRequest( void );

    /** A GET of the url. Redirects are not followed: a provider that sends a token request somewhere else is not obeyed */
    bool Get( const CORE::CString& url, const COAuth2EngineSettings& settings );

    /** A POST of the body */
    bool Post( const CORE::CString& url, const CORE::CString& contentType, const CORE::CString& body, const COAuth2EngineSettings& settings );

    void SetHeader( const CORE::CString& name, const CORE::CString& value );

    /** Authorization: Bearer token */
    void SetBearer( const CSecretString& token );

    /** Authorization with a ready made header value, for example Basic */
    void SetAuthorization( const CSecretString& headerValue );
};

/*-------------------------------------------------------------------------*/

/** Why a request got no answer, the same for every HTTP client backend */
enum EAuthHttpFailure
{
    AUTHHTTP_OK = 0,
    AUTHHTTP_NO_HTTPS,          /**< the build has no https: the plugin of a https capable backend (httpclientpluginCURL) is not loaded */
    AUTHHTTP_NO_BACKEND,        /**< no HTTP client backend for the scheme of the address */
    AUTHHTTP_DNS,               /**< the name of the host could not be resolved */
    AUTHHTTP_CONNECT,           /**< no connection to the host could be made, or it was lost */
    AUTHHTTP_TLS,               /**< the secure connection failed: certificate not trusted, not for this host, or the handshake failed */
    AUTHHTTP_TIMEOUT,           /**< no answer in time */
    AUTHHTTP_CANCELLED,         /**< the request was cancelled */
    AUTHHTTP_OTHER              /**< any other failure to get an answer */
};

typedef enum EAuthHttpFailure TAuthHttpFailure;

/** A stable lower case name of a failure, for logs */
GUCEF_AUTH_EXPORT_CPP const char*
AuthHttpFailureToString( TAuthHttpFailure failure );

/*-------------------------------------------------------------------------*/

/** What came back */
class GUCEF_AUTH_EXPORT_CPP CAuthHttpResponse
{
    public:

    bool transportOk;                       /**< a response was received, whatever its status */
    bool wasCancelled;
    bool timedOut;
    bool tooLarge;                          /**< the body is longer than the limit; the body is empty then */
    int statusCode;
    CORE::CString contentType;
    CORE::CString cacheControl;
    CORE::CDynamicBuffer body;

    /** when no answer came: why, with the host the request went to (no user information, path or query) and what the backend reported */
    TAuthHttpFailure failure;
    CORE::CString scheme;
    CORE::CString host;
    CORE::CString redactedUrl;              /**< the address as CORE::CRedactor::ForUrl() makes it: no user information, no values of the query, no fragment */
    CORE::CString backendName;
    Int32 backendErrorCode;

    CAuthHttpResponse( void );

    bool IsSuccessStatus( void ) const;

    /**
     *  The reason and the sentence for a person for a request that did not work out: no answer (the failure above), an answer that is
     *  too large, or a status that is not a success. The sentence names the host, never more of the address. False when the request worked.
     */
    bool DescribeFailure( TAuthFailureReason& reason, CORE::CString& message ) const;

    /** For a log: the class of the failure, the host, the address as CORE::CRedactor::ForUrl() makes it, the backend and its error code or the status; nothing of the request or the answer */
    CORE::CString DescribeForLog( void ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The HTTP client of the engine: one request at a time on the calling thread (Execute()) or as a task of the thread
 *  pool (Start() and Collect()). No redirects, timeouts from the engine settings, answers over the size limit are
 *  dropped. Nothing of a request or an answer is written to a log here.
 *
 *  Whether a request to a https address can be carried out at all depends on the HTTP client backends that are registered when it
 *  is made, see IsHttpsSupported().
 */
class GUCEF_AUTH_EXPORT_CPP COAuth2Http
{
    public:

    explicit COAuth2Http( const COAuth2EngineSettings& settings );
    ~COAuth2Http();

    bool Execute( const CAuthHttpRequest& request, CAuthHttpResponse& response );

    /**
     *  Lets go of the HTTP client backend objects this client kept from earlier requests (a plugin such as httpclientpluginCURL made
     *  them): call it before such a plugin is unloaded, when no request is under way. The next request makes them again.
     */
    void ReleaseBackends( void );

    /**
     *  Whether a HTTP client backend that carries https is available right now: the native backend with a TLS provider (the OpenSSL of the
     *  system on Linux, loaded when first asked for), WinHTTP on Windows, or a backend of a plugin such as httpclientpluginCURL
     */
    static bool IsHttpsSupported( void );

    /** Why https is not supported, as a sentence for the user: "No HTTPS support: libssl.so.3: cannot open shared object file ...". Meaningful when IsHttpsSupported() is false */
    static CORE::CString GetHttpsUnavailableMessage( void );

    /** Starts the request as a task; the future has no future when it cannot be queued */
    CORE::CFutureResult Start( const CAuthHttpRequest& request );

    /** Reads the outcome of a future that is ready */
    bool Collect( CORE::CFutureResult& future, CAuthHttpResponse& response ) const;

    private:

    COAuth2Http( const COAuth2Http& src );              /**< not implemented */
    COAuth2Http& operator=( const COAuth2Http& src );   /**< not implemented */

    void Fill( const WEB::CHttpRequestData& request, const WEB::CHttpResponseData& data, const WEB::CHttpClientTransferInfo& info, bool sent, CAuthHttpResponse& response ) const;

    private:

    const COAuth2EngineSettings& m_settings;
    WEB::CHTTPClient m_client;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COAUTH2HTTP_H ? */
