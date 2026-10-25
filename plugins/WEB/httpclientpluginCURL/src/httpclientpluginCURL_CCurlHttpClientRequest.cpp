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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "httpclientpluginCURL_CCurlHttpClientRequest.h"

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

/** a larger buffer than curl's default means large downloads take fewer pulses */
static const long ReceiveBufferSize = 128 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** sets a flag while a curl callback runs so that finishing the request from inside it does not touch the multi handle */
class CCallbackScope
{
    public:

    explicit CCallbackScope( bool& flag ) : m_flag( flag ) { m_flag = true; }
    ~CCallbackScope() { m_flag = false; }

    private:

    CCallbackScope( const CCallbackScope& src );
    CCallbackScope& operator=( const CCallbackScope& src );

    bool& m_flag;
};

/*-------------------------------------------------------------------------*/

static bool
IsLineEndOnly( const char* line, size_t length )
{
    for ( size_t i=0; i<length; ++i )
    {
        if ( '\r' != line[ i ] && '\n' != line[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCurlHttpClientRequest::CCurlHttpClientRequest( CCurlHttpClientContextPtr context             ,
                                                const WEB::CHttpRequestData& request          ,
                                                const WEB::CHttpClientRequestOptions& options ,
                                                WEB::CHttpResponseData& response              ,
                                                WEB::CHttpClientTransferInfo& transferInfo    ,
                                                WEB::CIHttpClientBackendObserver& observer    )
    : WEB::CHttpClientBackendRequestBase( request, options, response, transferInfo, observer )
    , m_context( context )
    , m_driver( GUCEF_NULL )
    , m_easy( GUCEF_NULL )
    , m_headers( GUCEF_NULL )
    , m_lastUploaded( 0 )
    , m_isInMulti( false )
    , m_inCallback( false )
{GUCEF_TRACE;

    m_errorBuffer[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCurlHttpClientRequest::ResolveCaBundle( const CORE::CString& configuredPath )
{GUCEF_TRACE;

    // What the caller asks for is used as is: a file that cannot be read makes https requests fail, it never makes them unverified
    if ( !configuredPath.IsNULLOrEmpty() )
        return configuredPath;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // curl uses the certificate store of Windows
    return CORE::CString::Empty;

    #else

    // The standard variable of OpenSSL and of tools built on it
    CORE::CString fromEnvironment = CORE::GetEnv( "SSL_CERT_FILE" );
    if ( !fromEnvironment.IsNULLOrEmpty() )
        return fromEnvironment;

    // The bundle of the system: Debian and Ubuntu, RHEL and Fedora, Alpine and macOS, openSUSE, RHEL (extracted store).
    // None of them: curl falls back on the default locations of the TLS library it was built with
    static const char* const systemBundles[] =
    {
        "/etc/ssl/certs/ca-certificates.crt"            ,
        "/etc/pki/tls/certs/ca-bundle.crt"              ,
        "/etc/ssl/cert.pem"                             ,
        "/etc/ssl/ca-bundle.pem"                        ,
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem"
    };
    for ( size_t i=0; i<sizeof( systemBundles ) / sizeof( systemBundles[ 0 ] ); ++i )
    {
        CORE::CString bundle = systemBundles[ i ];
        if ( CORE::FileExists( bundle ) )
            return bundle;
    }
    return CORE::CString::Empty;

    #endif
}

/*-------------------------------------------------------------------------*/

WEB::THttpTransferErrorClass
CCurlHttpClientRequest::ClassifyCurlResult( CURLcode result )
{GUCEF_TRACE;

    switch ( result )
    {
        case CURLE_COULDNT_RESOLVE_HOST :
        case CURLE_COULDNT_RESOLVE_PROXY :
        {
            return WEB::HTTPERRORCLASS_DNS;
        }
        case CURLE_COULDNT_CONNECT :
        case CURLE_SEND_ERROR :
        case CURLE_RECV_ERROR :
        case CURLE_GOT_NOTHING :
        case CURLE_PARTIAL_FILE :
        {
            return WEB::HTTPERRORCLASS_CONNECT;
        }
        case CURLE_SSL_CONNECT_ERROR :
        case CURLE_PEER_FAILED_VERIFICATION :
        case CURLE_SSL_CERTPROBLEM :
        case CURLE_SSL_CIPHER :
        case CURLE_SSL_CACERT_BADFILE :
        case CURLE_SSL_ISSUER_ERROR :
        case CURLE_SSL_CRL_BADFILE :
        case CURLE_SSL_ENGINE_NOTFOUND :
        case CURLE_SSL_ENGINE_SETFAILED :
        case CURLE_SSL_ENGINE_INITFAILED :
        case CURLE_SSL_INVALIDCERTSTATUS :
        case CURLE_SSL_PINNEDPUBKEYNOTMATCH :
        case CURLE_USE_SSL_FAILED :
        {
            return WEB::HTTPERRORCLASS_TLS;
        }
        case CURLE_OPERATION_TIMEDOUT :
        {
            return WEB::HTTPERRORCLASS_TIMEOUT;
        }
        default :
        {
            return WEB::HTTPERRORCLASS_OTHER;
        }
    }
}

/*-------------------------------------------------------------------------*/

CCurlHttpClientRequest::~CCurlHttpClientRequest()
{GUCEF_TRACE;

    RemoveFromDriver();

    if ( GUCEF_NULL != m_easy )
        curl_easy_cleanup( m_easy );
    m_easy = GUCEF_NULL;

    if ( GUCEF_NULL != m_headers )
        curl_slist_free_all( m_headers );
    m_headers = GUCEF_NULL;

    if ( GUCEF_NULL != m_driver )
    {
        CCurlMultiDriver* driver = m_driver;
        m_driver = GUCEF_NULL;
        m_context->ReleaseDriverIfIdle( driver );
    }
}

/*-------------------------------------------------------------------------*/

CURL*
CCurlHttpClientRequest::GetEasyHandle( void ) const
{GUCEF_TRACE;

    return m_easy;
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientRequest::SetIsInMulti( bool isInMulti )
{GUCEF_TRACE;

    m_isInMulti = isInMulti;
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientRequest::RemoveFromDriver( void )
{GUCEF_TRACE;

    if ( m_isInMulti && GUCEF_NULL != m_driver )
        m_driver->RemoveRequest( this );
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientRequest::OnFinish( void )
{GUCEF_TRACE;

    // Inside a curl callback the handle stays where it is, curl is stopped by the callback return value
    // and the driver removes the handle when curl reports the transfer as done
    if ( !m_inCallback )
        RemoveFromDriver();
}

/*-------------------------------------------------------------------------*/

bool
CCurlHttpClientRequest::SetupEasyHandle( void )
{GUCEF_TRACE;

    m_easy = curl_easy_init();
    if ( GUCEF_NULL == m_easy )
        return false;

    // The basics. Redirects are followed by the caller of the backend, never by curl
    CORE::CString url = m_request.GetUrl();
    curl_easy_setopt( m_easy, CURLOPT_URL, url.C_String() );
    curl_easy_setopt( m_easy, CURLOPT_PRIVATE, this );
    curl_easy_setopt( m_easy, CURLOPT_SHARE, m_context->GetShareHandle() );
    curl_easy_setopt( m_easy, CURLOPT_NOSIGNAL, 1L );
    curl_easy_setopt( m_easy, CURLOPT_ERRORBUFFER, m_errorBuffer );
    curl_easy_setopt( m_easy, CURLOPT_FOLLOWLOCATION, 0L );
    curl_easy_setopt( m_easy, CURLOPT_BUFFERSIZE, ReceiveBufferSize );
    curl_easy_setopt( m_easy, CURLOPT_CONNECTTIMEOUT_MS, (long) m_options.connectTimeoutInMs );

    // Same as the other backends: the headers are given as is, no decoding of the content
    curl_easy_setopt( m_easy, CURLOPT_SSL_VERIFYPEER, m_options.verifyPeer ? 1L : 0L );
    curl_easy_setopt( m_easy, CURLOPT_SSL_VERIFYHOST, m_options.verifyPeer ? 2L : 0L );

    // The authorities that are trusted: the build of curl has no bundle of its own, so one is given here
    if ( m_options.verifyPeer )
    {
        CORE::CString caBundle = ResolveCaBundle( m_options.caBundlePath );
        if ( !caBundle.IsNULLOrEmpty() )
            curl_easy_setopt( m_easy, CURLOPT_CAINFO, caBundle.C_String() );
    }

    // An empty proxy stops curl from looking at the proxy environment variables so that the proxy
    // options are the only thing deciding, same as for the other backends
    if ( !m_options.proxyHost.IsNULLOrEmpty() )
    {
        CORE::CString proxy = m_options.proxyHost + ":" + CORE::ToString( m_options.proxyPort );
        curl_easy_setopt( m_easy, CURLOPT_PROXY, proxy.C_String() );
        curl_easy_setopt( m_easy, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L );
    }
    else
    {
        curl_easy_setopt( m_easy, CURLOPT_PROXY, "" );
    }

    // Request header fields. An empty Expect field disables the wait for 100 Continue
    CORE::CString::StringVector fields;
    GetRequestHeaderFields( fields );
    for ( size_t i=0; i<fields.size(); ++i )
    {
        struct curl_slist* appended = curl_slist_append( m_headers, fields[ i ].C_String() );
        if ( GUCEF_NULL == appended )
            return false;
        m_headers = appended;
    }
    struct curl_slist* appended = curl_slist_append( m_headers, "Expect:" );
    if ( GUCEF_NULL == appended )
        return false;
    m_headers = appended;
    curl_easy_setopt( m_easy, CURLOPT_HTTPHEADER, m_headers );

    // The method and the body
    const CORE::CString& method = GetRequestMethod();
    if ( HasRequestBody() )
    {
        curl_easy_setopt( m_easy, CURLOPT_UPLOAD, 1L );
        curl_easy_setopt( m_easy, CURLOPT_CUSTOMREQUEST, method.C_String() );
        curl_easy_setopt( m_easy, CURLOPT_READFUNCTION, &CCurlHttpClientRequest::CurlReadCallback );
        curl_easy_setopt( m_easy, CURLOPT_READDATA, this );
        curl_easy_setopt( m_easy, CURLOPT_SEEKFUNCTION, &CCurlHttpClientRequest::CurlSeekCallback );
        curl_easy_setopt( m_easy, CURLOPT_SEEKDATA, this );

        // A body of unknown size comes from a stream which cannot be sent again if a reused connection turns out
        // to be closed already, so such a request is given a connection of its own
        if ( !IsRequestBodySizeKnown() )
            curl_easy_setopt( m_easy, CURLOPT_FRESH_CONNECT, 1L );

        // Without a known size curl sends the body chunked
        curl_easy_setopt( m_easy, CURLOPT_INFILESIZE_LARGE, IsRequestBodySizeKnown() ? (curl_off_t) GetRequestBodySize() : (curl_off_t) -1 );
    }
    else
    if ( IsHeadRequest() )
    {
        curl_easy_setopt( m_easy, CURLOPT_NOBODY, 1L );
    }
    else
    if ( method == "GET" )
    {
        curl_easy_setopt( m_easy, CURLOPT_HTTPGET, 1L );
    }
    else
    {
        curl_easy_setopt( m_easy, CURLOPT_CUSTOMREQUEST, method.C_String() );
    }

    // The callbacks through which the response and the progress arrive
    curl_easy_setopt( m_easy, CURLOPT_WRITEFUNCTION, &CCurlHttpClientRequest::CurlWriteCallback );
    curl_easy_setopt( m_easy, CURLOPT_WRITEDATA, this );
    curl_easy_setopt( m_easy, CURLOPT_HEADERFUNCTION, &CCurlHttpClientRequest::CurlHeaderCallback );
    curl_easy_setopt( m_easy, CURLOPT_HEADERDATA, this );
    curl_easy_setopt( m_easy, CURLOPT_PREREQFUNCTION, &CCurlHttpClientRequest::CurlPrereqCallback );
    curl_easy_setopt( m_easy, CURLOPT_PREREQDATA, this );
    curl_easy_setopt( m_easy, CURLOPT_XFERINFOFUNCTION, &CCurlHttpClientRequest::CurlTransferInfoCallback );
    curl_easy_setopt( m_easy, CURLOPT_XFERINFODATA, this );
    curl_easy_setopt( m_easy, CURLOPT_NOPROGRESS, 0L );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCurlHttpClientRequest::Start( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    if ( !SetupEasyHandle() )
    {
        Finish( false, "Unable to set up the curl request" );
        return false;
    }

    m_driver = m_context->GetOrCreateDriver( pulseGenerator );
    if ( GUCEF_NULL == m_driver || !m_driver->AddRequest( this ) )
    {
        Finish( false, "Unable to start the curl transfer" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCurlHttpClientRequest::OnCurlDone( CURLcode result )
{GUCEF_TRACE;

    // The request can already be finished, for example when a callback aborted the transfer. The result is
    // an error then which is not interesting since we know why we aborted
    if ( IsFinished() )
    {
        RemoveFromDriver();
        return;
    }

    // The transfer progress only counts the body that was uploaded, the request line and header fields
    // (the size of the issued requests according to curl) are part of what was sent as well
    long requestSize = 0;
    if ( CURLE_OK == curl_easy_getinfo( m_easy, CURLINFO_REQUEST_SIZE, &requestSize ) && requestSize > 0 )
        OnDataSent( (UInt32) requestSize );

    if ( CURLE_OK == result )
    {
        if ( HaveResponseHeadersCompleted() )
            Finish( true, CORE::CString::Empty );
        else
            Finish( false, "The server did not reply with a HTTP response" );
    }
    else
    {
        CORE::CString description = '\0' != m_errorBuffer[ 0 ] ? CORE::CString( m_errorBuffer ) : CORE::CString( curl_easy_strerror( result ) );
        m_transferInfo.timedOut = CURLE_OPERATION_TIMEDOUT == result;
        m_transferInfo.errorClass = ClassifyCurlResult( result );
        Finish( false, description, (CORE::Int32) result );
    }
}

/*-------------------------------------------------------------------------*/

size_t
CCurlHttpClientRequest::CurlWriteCallback( char* data, size_t size, size_t count, void* userPtr )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    size_t byteCount = size * count;
    if ( request->IsFinished() )
        return 0;

    request->OnDataReceived( (UInt32) byteCount );
    return request->OnResponseBody( data, (UInt32) byteCount ) ? byteCount : 0;
}

/*-------------------------------------------------------------------------*/

size_t
CCurlHttpClientRequest::CurlHeaderCallback( char* data, size_t size, size_t count, void* userPtr )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    size_t byteCount = size * count;
    if ( request->IsFinished() )
        return 0;

    request->OnDataReceived( (UInt32) byteCount );

    if ( byteCount >= 5 && 0 == memcmp( data, "HTTP/", 5 ) )
    {
        // The status line, a new response starts. An interim response such as 100 Continue is replaced by the next one
        return request->OnResponseStatusLine( data, (UInt32) byteCount ) ? byteCount : 0;
    }

    if ( IsLineEndOnly( data, byteCount ) )
    {
        // The empty line ends the header fields. Interim (1xx) responses are followed by a new status line
        // and trailers after the body end with an empty line as well
        if ( !request->HaveResponseHeadersCompleted() && request->m_response.statusCode >= 200 )
            request->OnResponseHeadersComplete();
        return byteCount;
    }

    request->OnResponseHeaderLine( data, (UInt32) byteCount );
    return byteCount;
}

/*-------------------------------------------------------------------------*/

size_t
CCurlHttpClientRequest::CurlReadCallback( char* buffer, size_t size, size_t count, void* userPtr )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    if ( request->IsFinished() )
        return CURL_READFUNC_ABORT;

    // Zero means the end of the body
    return (size_t) request->ReadRequestBody( buffer, (UInt32) ( size * count ) );
}

/*-------------------------------------------------------------------------*/

int
CCurlHttpClientRequest::CurlSeekCallback( void* userPtr, curl_off_t offset, int origin )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    // curl only needs to start over from the beginning
    if ( SEEK_SET != origin || 0 != offset )
        return CURL_SEEKFUNC_CANTSEEK;
    return request->RewindRequestBody() ? CURL_SEEKFUNC_OK : CURL_SEEKFUNC_CANTSEEK;
}

/*-------------------------------------------------------------------------*/

int
CCurlHttpClientRequest::CurlTransferInfoCallback( void* userPtr, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    if ( request->IsFinished() )
        return 1;

    if ( ulNow > request->m_lastUploaded )
    {
        request->OnDataSent( (UInt32) ( ulNow - request->m_lastUploaded ) );
        request->m_lastUploaded = ulNow;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

int
CCurlHttpClientRequest::CurlPrereqCallback( void* userPtr, char* primaryIp, char* localIp, int primaryPort, int localPort )
{
    CCurlHttpClientRequest* request = static_cast< CCurlHttpClientRequest* >( userPtr );
    CCallbackScope scope( request->m_inCallback );

    if ( request->IsFinished() )
        return CURL_PREREQFUNC_ABORT;

    request->OnConnected();
    return CURL_PREREQFUNC_OK;
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
