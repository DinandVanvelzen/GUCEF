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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#include "gucefWEB_CHttpClientRequestTask.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_COAuth2Http.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthHttpRequest::CAuthHttpRequest( void )
    : request()
    , options()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static void
PrepareOptions( WEB::CHttpClientRequestOptions& options, const COAuth2EngineSettings& settings )
{GUCEF_TRACE;

    options.followRedirects = false;
    options.maxRedirects = 0;
    options.verifyPeer = true;                      // the certificate of a provider is always checked
    options.caBundlePath = settings.caBundlePath;
    options.connectTimeoutInMs = settings.httpConnectTimeoutInMs;
    options.totalTimeoutInMs = settings.httpTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CAuthHttpRequest::Get( const CORE::CString& url, const COAuth2EngineSettings& settings )
{GUCEF_TRACE;

    PrepareOptions( options, settings );
    request.requestType = "GET";
    request.resourceRepresentations.clear();
    request.resourceRepresentations.push_back( "application/json" );
    return request.SetUrl( url );
}

/*-------------------------------------------------------------------------*/

bool
CAuthHttpRequest::Post( const CORE::CString& url, const CORE::CString& contentType, const CORE::CString& body, const COAuth2EngineSettings& settings )
{GUCEF_TRACE;

    PrepareOptions( options, settings );
    request.requestType = "POST";
    request.resourceRepresentations.clear();
    request.resourceRepresentations.push_back( "application/json" );
    request.contentRepresentation = contentType;
    request.content.Clear( true );
    request.content.CopyFrom( GetByteLength( body ), body.C_String() );
    return request.SetUrl( url );
}

/*-------------------------------------------------------------------------*/

void
CAuthHttpRequest::SetHeader( const CORE::CString& name, const CORE::CString& value )
{GUCEF_TRACE;

    request.otherHeaders[ name ] = value;
}

/*-------------------------------------------------------------------------*/

void
CAuthHttpRequest::SetBearer( const CSecretString& token )
{GUCEF_TRACE;

    request.otherHeaders[ "Authorization" ] = "Bearer " + token.Reveal();
}

/*-------------------------------------------------------------------------*/

void
CAuthHttpRequest::SetAuthorization( const CSecretString& headerValue )
{GUCEF_TRACE;

    request.otherHeaders[ "Authorization" ] = headerValue.Reveal();
}

/*-------------------------------------------------------------------------*/

const char*
AuthHttpFailureToString( TAuthHttpFailure failure )
{
    switch ( failure )
    {
        case AUTHHTTP_NO_HTTPS :    return "no-https";
        case AUTHHTTP_NO_BACKEND :  return "no-backend";
        case AUTHHTTP_DNS :         return "dns";
        case AUTHHTTP_CONNECT :     return "connect";
        case AUTHHTTP_TLS :         return "tls";
        case AUTHHTTP_TIMEOUT :     return "timeout";
        case AUTHHTTP_CANCELLED :   return "cancelled";
        case AUTHHTTP_OTHER :       return "other";
        default :                   return "ok";
    }
}

/*-------------------------------------------------------------------------*/

CAuthHttpResponse::CAuthHttpResponse( void )
    : transportOk( false )
    , wasCancelled( false )
    , timedOut( false )
    , tooLarge( false )
    , statusCode( 0 )
    , contentType()
    , cacheControl()
    , body()
    , failure( AUTHHTTP_OK )
    , scheme()
    , host()
    , redactedUrl()
    , backendName()
    , backendErrorCode( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAuthHttpResponse::IsSuccessStatus( void ) const
{GUCEF_TRACE;

    return transportOk && statusCode >= 200 && statusCode < 300;
}

/*-------------------------------------------------------------------------*/

bool
CAuthHttpResponse::DescribeFailure( TAuthFailureReason& reason, CORE::CString& message ) const
{GUCEF_TRACE;

    const CORE::CString where = host.IsNULLOrEmpty() ? CORE::CString( "the provider" ) : host;
    if ( !transportOk )
    {
        switch ( failure )
        {
            case AUTHHTTP_NO_HTTPS :
            {
                reason = AUTHFAIL_NOT_AVAILABLE;
                message = COAuth2Http::GetHttpsUnavailableMessage();
                return true;
            }
            case AUTHHTTP_NO_BACKEND :
            {
                reason = AUTHFAIL_NOT_AVAILABLE;
                message = "No HTTP client for \"" + scheme + "\" addresses in this build";
                return true;
            }
            case AUTHHTTP_DNS :
            {
                reason = AUTHFAIL_NETWORK;
                message = "The address of " + where + " could not be found (DNS lookup failed)";
                return true;
            }
            case AUTHHTTP_CONNECT :
            {
                reason = AUTHFAIL_NETWORK;
                message = "No connection to " + where + " could be made";
                return true;
            }
            case AUTHHTTP_TLS :
            {
                reason = AUTHFAIL_NETWORK;
                message = "The secure connection to " + where + " failed: its certificate is not trusted or does not match, or the TLS handshake failed";
                return true;
            }
            case AUTHHTTP_TIMEOUT :
            {
                reason = AUTHFAIL_NETWORK;
                message = where + " did not answer in time";
                return true;
            }
            case AUTHHTTP_CANCELLED :
            {
                reason = AUTHFAIL_CANCELLED;
                message = "The request was cancelled";
                return true;
            }
            default :
            {
                reason = AUTHFAIL_NETWORK;
                message = "The provider could not be reached";
                return true;
            }
        }
    }
    if ( tooLarge )
    {
        reason = AUTHFAIL_INVALID_RESPONSE;
        message = "The answer of " + where + " is too large";
        return true;
    }
    if ( !IsSuccessStatus() )
    {
        // The provider did not deliver what was asked for: trying again later can work
        reason = AUTHFAIL_NETWORK;
        message = where + " answered with HTTP status " + CORE::ToString( statusCode );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthHttpResponse::DescribeForLog( void ) const
{GUCEF_TRACE;

    CORE::CString text = transportOk ? CORE::CString( "answered" ) : CORE::CString( AuthHttpFailureToString( failure ) );
    text += " host=" + ( host.IsNULLOrEmpty() ? CORE::CString( "?" ) : host );
    if ( !redactedUrl.IsNULLOrEmpty() )
        text += " url=" + redactedUrl;
    if ( !backendName.IsNULLOrEmpty() )
        text += " backend=" + backendName;
    if ( !transportOk && 0 != backendErrorCode )
        text += " backend-error=" + CORE::ToString( backendErrorCode );
    if ( transportOk )
        text += " status=" + CORE::ToString( statusCode );
    return text;
}

/*-------------------------------------------------------------------------*/

COAuth2Http::COAuth2Http( const COAuth2EngineSettings& settings )
    : m_settings( settings )
    , m_client()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COAuth2Http::~COAuth2Http()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static TAuthHttpFailure
ClassifyFailure( const WEB::CHttpClientTransferInfo& info, const CORE::CString& scheme )
{
    if ( info.wasCancelled )
        return AUTHHTTP_CANCELLED;
    if ( info.timedOut )
        return AUTHHTTP_TIMEOUT;
    switch ( info.errorClass )
    {
        case WEB::HTTPERRORCLASS_NO_BACKEND :   return scheme.Equals( "https", false ) ? AUTHHTTP_NO_HTTPS : AUTHHTTP_NO_BACKEND;
        case WEB::HTTPERRORCLASS_DNS :          return AUTHHTTP_DNS;
        case WEB::HTTPERRORCLASS_CONNECT :      return AUTHHTTP_CONNECT;
        case WEB::HTTPERRORCLASS_TLS :          return AUTHHTTP_TLS;
        case WEB::HTTPERRORCLASS_TIMEOUT :      return AUTHHTTP_TIMEOUT;
        default :                               return AUTHHTTP_OTHER;
    }
}

/*-------------------------------------------------------------------------*/

void
COAuth2Http::Fill( const WEB::CHttpRequestData& request, const WEB::CHttpResponseData& data, const WEB::CHttpClientTransferInfo& info, bool sent, CAuthHttpResponse& response ) const
{GUCEF_TRACE;

    response = CAuthHttpResponse();
    response.wasCancelled = info.wasCancelled;
    response.timedOut = info.timedOut;
    response.transportOk = sent && info.success;
    response.scheme = request.requestProtocol.Lowercase();
    response.host = request.GetHostName();
    response.redactedUrl = CORE::CRedactor::ForUrl().RedactUrl( request.GetUrl() );
    response.backendName = info.backendName;
    if ( !response.transportOk )
    {
        response.failure = ClassifyFailure( info, response.scheme );
        response.backendErrorCode = info.errorCode;
        return;
    }
    response.statusCode = data.statusCode;
    response.contentType = data.contentType;
    response.cacheControl = data.cacheability;
    if ( data.content.GetDataSize() > m_settings.maxResponseBytes )
    {
        response.tooLarge = true;
        return;
    }
    response.body = data.content;
}

/*-------------------------------------------------------------------------*/

void
COAuth2Http::ReleaseBackends( void )
{GUCEF_TRACE;

    // Setting the preference makes the client let go of the backend objects it made
    m_client.SetPreferredBackends( m_client.GetPreferredBackends() );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Http::IsHttpsSupported( void )
{GUCEF_TRACE;

    return WEB::CHTTPClient::IsSchemeSupported( "https" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COAuth2Http::GetHttpsUnavailableMessage( void )
{GUCEF_TRACE;

    // "no HTTPS support: <what the TLS provider says>", as a sentence
    CORE::CString reason = WEB::CHTTPClient::DescribeUnsupportedScheme( "https" );
    if ( reason.IsNULLOrEmpty() )
        return "No HTTPS support";
    return reason.SubstrToIndex( 1, true ).Uppercase() + reason.CutChars( 1, true );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Http::Execute( const CAuthHttpRequest& request, CAuthHttpResponse& response )
{GUCEF_TRACE;

    WEB::CHttpResponseData data;
    WEB::CHttpClientTransferInfo info;
    bool sent = m_client.Execute( request.request, request.options, data, &info );
    Fill( request.request, data, info, sent, response );
    return response.transportOk && !response.tooLarge;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
COAuth2Http::Start( const CAuthHttpRequest& request )
{GUCEF_TRACE;

    return m_client.ExecuteAsync( request.request, request.options );
}

/*-------------------------------------------------------------------------*/

bool
COAuth2Http::Collect( CORE::CFutureResult& future, CAuthHttpResponse& response ) const
{GUCEF_TRACE;

    response = CAuthHttpResponse();
    if ( future.HasNoFuture() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
    if ( GUCEF_NULL == data )
        return false;
    Fill( data->request, data->response, data->transferInfo, true, response );
    return response.transportOk && !response.tooLarge;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
