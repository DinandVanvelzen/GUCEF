/*
 *  httpclientpluginCURL: GUCEF plugin providing a HTTP client backend using libcurl
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

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <curl/curl.h>

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#include "gucefWEB_CHttpClientBackendRequestBase.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H ? */

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H
#include "httpclientpluginCURL_CCurlHttpClientContext.h"
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H
#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBPLUGIN {
namespace HTTPCLIENTCURL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A single HTTP exchange carried out by curl: one easy handle which is driven by the multi driver
 *  of the pulse generator the request was started with.
 */
class HTTPCLIENTPLUGIN_CURL_PRIVATE_CPP CCurlHttpClientRequest : public WEB::CHttpClientBackendRequestBase
{
    public:

    CCurlHttpClientRequest( CCurlHttpClientContextPtr context             ,
                            const WEB::CHttpRequestData& request          ,
                            const WEB::CHttpClientRequestOptions& options ,
                            WEB::CHttpResponseData& response              ,
                            WEB::CHttpClientTransferInfo& transferInfo    ,
                            WEB::CIHttpClientBackendObserver& observer    );

    virtual ~CCurlHttpClientRequest() GUCEF_VIRTUAL_OVERRIDE;

    /** sets up the easy handle and adds it to the driver of the pulse generator. On failure the request is finished */
    bool Start( CORE::PulseGeneratorPtr pulseGenerator );

    /** called by the driver when curl is done with the transfer */
    void OnCurlDone( CURLcode result );

    CURL* GetEasyHandle( void ) const;

    /** called by the driver for bookkeeping of whether the easy handle is part of its multi handle */
    void SetIsInMulti( bool isInMulti );

    /**
     *  The file with the trusted certificate authorities for a https request: the configured path when given, otherwise
     *  the environment variable SSL_CERT_FILE, otherwise the first bundle of the system that exists (Linux: Debian and Ubuntu,
     *  RHEL, /etc/ssl/cert.pem). Empty when there is none and curl is to use the defaults of its TLS library; on Windows curl
     *  uses the certificate store of the system and only a configured path is returned.
     */
    static CORE::CString ResolveCaBundle( const CORE::CString& configuredPath );

    /** the kind of failure a curl result code stands for */
    static WEB::THttpTransferErrorClass ClassifyCurlResult( CURLcode result );

    protected:

    virtual void OnFinish( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCurlHttpClientRequest( void );                                             /**< not implemented */
    CCurlHttpClientRequest( const CCurlHttpClientRequest& src );                /**< not implemented */
    CCurlHttpClientRequest& operator=( const CCurlHttpClientRequest& src );     /**< not implemented */

    bool SetupEasyHandle( void );

    void RemoveFromDriver( void );

    static size_t CurlWriteCallback( char* data, size_t size, size_t count, void* userPtr );
    static size_t CurlHeaderCallback( char* data, size_t size, size_t count, void* userPtr );
    static size_t CurlReadCallback( char* buffer, size_t size, size_t count, void* userPtr );
    static int CurlSeekCallback( void* userPtr, curl_off_t offset, int origin );
    static int CurlTransferInfoCallback( void* userPtr, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow );
    static int CurlPrereqCallback( void* userPtr, char* primaryIp, char* localIp, int primaryPort, int localPort );

    private:

    CCurlHttpClientContextPtr m_context;
    CCurlMultiDriver* m_driver;
    CURL* m_easy;
    struct curl_slist* m_headers;
    char m_errorBuffer[ CURL_ERROR_SIZE ];
    curl_off_t m_lastUploaded;
    bool m_isInMulti;
    bool m_inCallback;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H ? */
