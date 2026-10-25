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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H
#include "httpclientpluginCURL_CCurlHttpClientRequest.h"
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H
#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H ? */

#include "httpclientpluginCURL_CCurlHttpClientBackend.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CCurlHttpClientBackend::BackendName = "curl";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCurlHttpClientBackend::CCurlHttpClientBackend( void )
    : WEB::CIHttpClientBackend()
    , m_context( GUCEF_NEW CCurlHttpClientContext() )
    , m_supportsHttps( false )
{GUCEF_TRACE;

    curl_version_info_data* info = curl_version_info( CURLVERSION_NOW );
    m_supportsHttps = GUCEF_NULL != info && 0 != ( info->features & CURL_VERSION_SSL );
}

/*-------------------------------------------------------------------------*/

CCurlHttpClientBackend::~CCurlHttpClientBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCurlHttpClientBackend::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

bool
CCurlHttpClientBackend::SupportsScheme( const CORE::CString& scheme ) const
{GUCEF_TRACE;

    return scheme.Equals( "http", false ) || ( m_supportsHttps && scheme.Equals( "https", false ) );
}

/*-------------------------------------------------------------------------*/

WEB::CIHttpClientBackendRequestPtr
CCurlHttpClientBackend::StartRequest( const WEB::CHttpRequestData& request          ,
                                      const WEB::CHttpClientRequestOptions& options ,
                                      WEB::CHttpResponseData& response              ,
                                      WEB::CHttpClientTransferInfo& transferInfo    ,
                                      CORE::PulseGeneratorPtr pulseGenerator        ,
                                      WEB::CIHttpClientBackendObserver& observer    )
{GUCEF_TRACE;

    if ( !SupportsScheme( request.requestProtocol ) )
    {
        transferInfo.success = false;
        transferInfo.errorCode = -1;
        transferInfo.errorClass = WEB::HTTPERRORCLASS_NO_BACKEND;
        transferInfo.errorDescription = "The curl HTTP client backend does not support \"" + request.requestProtocol + "\"";
        return WEB::CIHttpClientBackendRequestPtr();
    }

    CCurlHttpClientRequest* curlRequest = GUCEF_NEW CCurlHttpClientRequest( m_context, request, options, response, transferInfo, observer );
    WEB::CIHttpClientBackendRequestPtr requestPtr( curlRequest );
    if ( !curlRequest->Start( pulseGenerator ) )
        return WEB::CIHttpClientBackendRequestPtr();
    return requestPtr;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
