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

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTBACKEND_H
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#include "gucefWEB_CIHttpClientBackend.h"
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */

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
 *  HTTP client backend which uses libcurl, adds support for https.
 *  Requests are carried out with the curl multi interface, driven by the pulses of the pulse generator
 *  the request is started with.
 */
class HTTPCLIENTPLUGIN_CURL_PRIVATE_CPP CCurlHttpClientBackend : public WEB::CIHttpClientBackend
{
    public:

    static const CORE::CString BackendName;

    CCurlHttpClientBackend( void );

    virtual ~CCurlHttpClientBackend() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SupportsScheme( const CORE::CString& scheme ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual WEB::CIHttpClientBackendRequestPtr StartRequest( const WEB::CHttpRequestData& request          ,
                                                             const WEB::CHttpClientRequestOptions& options ,
                                                             WEB::CHttpResponseData& response              ,
                                                             WEB::CHttpClientTransferInfo& transferInfo    ,
                                                             CORE::PulseGeneratorPtr pulseGenerator        ,
                                                             WEB::CIHttpClientBackendObserver& observer    ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCurlHttpClientBackend( const CCurlHttpClientBackend& src );                /**< not implemented */
    CCurlHttpClientBackend& operator=( const CCurlHttpClientBackend& src );     /**< not implemented */

    private:

    CCurlHttpClientContextPtr m_context;
    bool m_supportsHttps;
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

#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTBACKEND_H ? */
