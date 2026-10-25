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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CHttpClientRequestRunner.h"

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsRedirectStatusCode( int statusCode )
{
    return 301 == statusCode || 302 == statusCode || 303 == statusCode || 307 == statusCode || 308 == statusCode;
}

/*-------------------------------------------------------------------------*/

/** removes the header with the given name, regardless of the case it is written in */
static void
EraseHeader( CHttpRequestData::TStringMap& headers, const CString& name )
{
    CHttpRequestData::TStringMap::iterator i = headers.begin();
    while ( i != headers.end() )
    {
        if ( (*i).first.Equals( name, false ) )
            headers.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

/** whether the header holds a credential, or is otherwise not for other eyes than those of the server it was meant for */
static bool
IsCredentialHeaderName( const CString& name )
{
    return name.Equals( "Authorization", false )       ||
           name.Equals( "Proxy-Authorization", false ) ||
           name.Equals( "Cookie", false )              ||
           CORE::CRedactor::ForHeaders().IsSensitiveName( name );
}

/*-------------------------------------------------------------------------*/

static void
EraseCredentialHeaders( CHttpRequestData::TStringMap& headers )
{
    CHttpRequestData::TStringMap::iterator i = headers.begin();
    while ( i != headers.end() )
    {
        if ( IsCredentialHeaderName( (*i).first ) )
            headers.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

static CString
FindHeader( const CHttpRequestData::TStringMap& headers, const CString& name )
{
    for ( CHttpRequestData::TStringMap::const_iterator i = headers.begin(); i != headers.end(); ++i )
    {
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

/** resolves the target of a redirect against the URL of the request that was redirected, RFC 3986 section 5.2 */
static CString
ResolveRedirectUrl( const CHttpRequestData& request, const CString& location )
{
    if ( location.HasSubstr( "://", true ) > 0 )
        return location;
    if ( location.Length() >= 2 && '/' == location[ 0 ] && '/' == location[ 1 ] )
        return request.requestProtocol + ":" + location;
    CString base = request.requestProtocol + "://" + request.requestHost;
    if ( location.Length() > 0 && '/' == location[ 0 ] )
        return base + location;
    if ( location.Length() > 0 && '?' == location[ 0 ] )
        return base + request.requestUri + location;
    Int32 lastSlash = request.requestUri.HasChar( '/', false );
    CString directory = lastSlash >= 0 ? request.requestUri.SubstrToIndex( (UInt32) lastSlash + 1, true ) : CString( "/" );
    return base + directory + location;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpClientRequestRunner::CHttpClientRequestRunner( CHttpClientBackendSelectionPtr backends          ,
                                                    const CHttpRequestData& request                  ,
                                                    const CHttpClientRequestOptions& options         ,
                                                    CHttpResponseData& response                      ,
                                                    CHttpClientTransferInfo& transferInfo            ,
                                                    CIHttpClientBackendObserver* progressObserver    )
    : CIHttpClientBackendObserver()
    , m_backends( backends )
    , m_originalRequest( request )
    , m_options( options )
    , m_response( response )
    , m_transferInfo( transferInfo )
    , m_progressObserver( progressObserver )
    , m_hopRequest()
    , m_hopOptions( options )
    , m_backendRequest()
    , m_pulseGenerator()
    , m_requestStartTicks( 0 )
    , m_hopStartTicks( 0 )
    , m_requestBodyStartPos( 0 )
    , m_credentialsStripped( false )
    , m_finished( false )
{GUCEF_TRACE;

    // The hop request starts as the original, its body is linked rather than copied
    m_hopRequest.resourceRepresentations = request.resourceRepresentations;
    m_hopRequest.encodingRepresentations = request.encodingRepresentations;
    m_hopRequest.resourceVersions = request.resourceVersions;
    m_hopRequest.requestProtocol = request.requestProtocol.Lowercase();
    m_hopRequest.requestProtocolVersion = request.requestProtocolVersion;
    m_hopRequest.requestType = request.requestType.IsNULLOrEmpty() ? CString( "GET" ) : request.requestType.Uppercase();
    m_hopRequest.requestHost = request.requestHost;
    m_hopRequest.requestUri = request.requestUri;
    m_hopRequest.requestUriParams = request.requestUriParams;
    m_hopRequest.contentRepresentation = request.contentRepresentation;
    m_hopRequest.otherHeaders = request.otherHeaders;
    if ( request.content.GetDataSize() > 0 )
        m_hopRequest.content.LinkTo( request.content.GetConstBufferPtr(), request.content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestRunner::~CHttpClientRequestRunner()
{GUCEF_TRACE;

    if ( !m_finished )
        Cancel();
}

/*-------------------------------------------------------------------------*/

UInt32
CHttpClientRequestRunner::GetElapsedMs( UInt64 sinceTicks ) const
{GUCEF_TRACE;

    UInt64 ticksPerSecond = MT::PrecisionTimerResolution();
    if ( 0 == ticksPerSecond )
        return 0;
    return (UInt32) ( ( MT::PrecisionTickCount() - sinceTicks ) * 1000 / ticksPerSecond );
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::Start( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
    m_transferInfo.Clear();
    m_requestStartTicks = MT::PrecisionTickCount();
    if ( !m_options.requestBody.IsNULL() )
        m_requestBodyStartPos = m_options.requestBody->Tell();
    return StartHop();
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::StartHop( void )
{GUCEF_TRACE;

    CIHttpClientBackendPtr backend = m_backends.IsNULL() ? CIHttpClientBackendPtr() : m_backends->GetBackendForScheme( m_hopRequest.requestProtocol );
    if ( backend.IsNULL() )
    {
        m_transferInfo.errorClass = HTTPERRORCLASS_NO_BACKEND;
        Fail( "No HTTP client backend is available for \"" + m_hopRequest.requestProtocol + "\" URLs: " + CHttpClientBackendSelection::DescribeUnsupportedScheme( m_hopRequest.requestProtocol ) );
        return false;
    }
    m_transferInfo.backendName = backend->GetBackendName();

    // Cookies: any given by the caller plus those from the jar that apply to this hop
    CString cookies = m_credentialsStripped ? CString::Empty : FindHeader( m_originalRequest.otherHeaders, "Cookie" );
    if ( !m_options.cookieJar.IsNULL() )
    {
        CHttpCookieJarPtr cookieJar = m_options.cookieJar;
        CString jarCookies = cookieJar->GetCookieHeaderValue( m_hopRequest.GetHostName(), m_hopRequest.requestUri,
                                                                       m_hopRequest.requestProtocol == "https" );
        if ( !jarCookies.IsNULLOrEmpty() )
            cookies = cookies.IsNULLOrEmpty() ? jarCookies : cookies + "; " + jarCookies;
    }
    EraseHeader( m_hopRequest.otherHeaders, "Cookie" );
    if ( !cookies.IsNULLOrEmpty() )
        m_hopRequest.otherHeaders[ "Cookie" ] = cookies;

    if ( m_options.hasRange )
    {
        CString range = "bytes=" + CORE::ToString( m_options.rangeStart ) + "-";
        if ( m_options.rangeLength > 0 )
            range += CORE::ToString( m_options.rangeStart + (UInt64) m_options.rangeLength - 1 );
        EraseHeader( m_hopRequest.otherHeaders, "Range" );
        m_hopRequest.otherHeaders[ "Range" ] = range;
    }

    // every hop starts with an empty response, which also makes a reused response object safe
    m_response.Clear();

    m_hopStartTicks = MT::PrecisionTickCount();
    m_backendRequest = backend->StartRequest( m_hopRequest, m_hopOptions, m_response, m_transferInfo, m_pulseGenerator, *this );
    if ( m_backendRequest.IsNULL() )
    {
        // The backend describes why in the transfer info
        if ( m_transferInfo.errorDescription.IsNULLOrEmpty() )
            m_transferInfo.errorDescription = "The " + m_transferInfo.backendName + " HTTP client backend could not start the request";
        m_transferInfo.success = false;
        Finish();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::FollowRedirect( void )
{GUCEF_TRACE;

    if ( m_transferInfo.redirectCount >= m_options.maxRedirects )
    {
        Fail( "Too many redirects, the maximum is " + CORE::ToString( m_options.maxRedirects ) );
        return false;
    }

    CString newUrl = ResolveRedirectUrl( m_hopRequest, m_response.location );
    CString previousProtocol = m_hopRequest.requestProtocol;
    CString previousHost = m_hopRequest.requestHost;
    int status = m_response.statusCode;

    // 303, and for historic reasons 301 and 302 after a POST, continue as GET without a body
    bool becomesGet = ( 303 == status && m_hopRequest.requestType != "HEAD" ) ||
                      ( ( 301 == status || 302 == status ) && m_hopRequest.requestType == "POST" );
    if ( becomesGet )
    {
        m_hopRequest.requestType = "GET";
        m_hopRequest.content.Clear();
        m_hopRequest.contentRepresentation.Clear();
        m_hopOptions.requestBody = CORE::IOAccessPtr();
    }
    else
    if ( !m_hopOptions.requestBody.IsNULL() )
    {
        // 307 and 308 repeat the request including its body
        if ( 0 != m_hopOptions.requestBody->Setpos( m_requestBodyStartPos ) )
        {
            Fail( "The request is redirected but its body cannot be sent again since its source cannot seek" );
            return false;
        }
    }

    if ( !m_hopRequest.SetUrl( newUrl ) )
    {
        Fail( "The request is redirected to an invalid URL: " + CORE::CRedactor::ForUrl().RedactUrl( newUrl ) );
        return false;
    }

    // Credentials are only for the origin they were meant for: not for another scheme (https to http), host or port.
    // What is taken away is not given back when a later redirect leads to the first origin again.
    if ( !CHttpRequestData::IsSameOrigin( previousProtocol, previousHost, m_hopRequest.requestProtocol, m_hopRequest.requestHost ) )
    {
        EraseCredentialHeaders( m_hopRequest.otherHeaders );
        m_credentialsStripped = true;
    }

    ++m_transferInfo.redirectCount;
    if ( GUCEF_NULL != m_progressObserver )
        m_progressObserver->OnHttpClientRedirect( newUrl );

    m_backendRequest = CIHttpClientBackendRequestPtr();
    return StartHop();
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::Update( void )
{GUCEF_TRACE;

    if ( m_finished )
        return true;
    if ( m_backendRequest.IsNULL() )
    {
        Finish();
        return true;
    }

    if ( m_backendRequest->IsFinished() )
    {
        if ( m_transferInfo.success && m_options.followRedirects && IsRedirectStatusCode( m_response.statusCode ) && !m_response.location.IsNULLOrEmpty() )
        {
            FollowRedirect();
            return m_finished;
        }
        Finish();
        return true;
    }

    if ( m_options.totalTimeoutInMs > 0 && GetElapsedMs( m_requestStartTicks ) > m_options.totalTimeoutInMs )
    {
        m_transferInfo.timedOut = true;
        m_backendRequest->Cancel();
        Fail( "The request timed out after " + CORE::ToString( m_options.totalTimeoutInMs ) + " ms" );
        return true;
    }
    if ( m_options.connectTimeoutInMs > 0 && !m_backendRequest->IsConnected() && GetElapsedMs( m_hopStartTicks ) > m_options.connectTimeoutInMs )
    {
        m_transferInfo.timedOut = true;
        m_backendRequest->Cancel();
        Fail( "Connecting timed out after " + CORE::ToString( m_options.connectTimeoutInMs ) + " ms" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::Cancel( void )
{GUCEF_TRACE;

    if ( m_finished )
        return;
    m_transferInfo.wasCancelled = true;
    if ( !m_backendRequest.IsNULL() )
        m_backendRequest->Cancel();
    Fail( "The request was cancelled" );
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::IsFinished( void ) const
{GUCEF_TRACE;

    return m_finished;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::Fail( const CString& errorDescription )
{GUCEF_TRACE;

    m_transferInfo.success = false;
    m_transferInfo.errorDescription = errorDescription;
    if ( 0 == m_transferInfo.errorCode )
        m_transferInfo.errorCode = -1;
    Finish();
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::Finish( void )
{GUCEF_TRACE;

    if ( m_finished )
        return;
    m_finished = true;
    m_transferInfo.effectiveUrl = m_hopRequest.GetUrl();
    if ( !m_transferInfo.success && HTTPERRORCLASS_NONE == m_transferInfo.errorClass && !m_transferInfo.wasCancelled )
        m_transferInfo.errorClass = m_transferInfo.timedOut ? HTTPERRORCLASS_TIMEOUT : HTTPERRORCLASS_OTHER;
    if ( !m_transferInfo.success )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpClientRequestRunner: Request to " << CORE::CRedactor::ForUrl().RedactUrl( m_transferInfo.effectiveUrl ) << " failed: " << m_transferInfo.errorDescription );
    m_transferInfo.durationInMs = GetElapsedMs( m_requestStartTicks );

    // Releases the connection
    m_backendRequest = CIHttpClientBackendRequestPtr();
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestRunner::Run( CHttpClientBackendSelectionPtr backends          ,
                               const CHttpRequestData& request                  ,
                               const CHttpClientRequestOptions& options         ,
                               CHttpResponseData& response                      ,
                               CHttpClientTransferInfo& transferInfo            ,
                               CIHttpClientBackendObserver* progressObserver    )
{GUCEF_TRACE;

    // A pulse generator of our own, driven by the calling thread: everything happens on this thread
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    CHttpClientRequestRunner runner( backends, request, options, response, transferInfo, progressObserver );
    if ( runner.Start( pulseGenerator ) )
    {
        while ( !runner.Update() )
        {
            pulseGenerator->ForceDirectPulse();
            pulseGenerator->WaitTillNextPulseWindow( 1, 10 );
        }
    }
    return transferInfo.success;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::OnHttpClientConnected( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_progressObserver )
        m_progressObserver->OnHttpClientConnected();
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::OnHttpClientResponseHeader( const CString& name, const CString& value )
{GUCEF_TRACE;

    if ( !m_options.cookieJar.IsNULL() && name.Equals( "Set-Cookie", false ) )
    {
        CHttpCookieJarPtr cookieJar = m_options.cookieJar;
        cookieJar->SetCookieFromHeader( value, m_hopRequest.GetHostName(), m_hopRequest.requestUri,
                                                  m_hopRequest.requestProtocol == "https" );
    }
    if ( GUCEF_NULL != m_progressObserver )
        m_progressObserver->OnHttpClientResponseHeader( name, value );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::OnHttpClientResponseHeadersComplete( const CHttpResponseData& response )
{GUCEF_TRACE;

    // Headers of a redirect that will be followed are not of interest to the progress observer
    if ( GUCEF_NULL != m_progressObserver &&
         !( m_options.followRedirects && IsRedirectStatusCode( response.statusCode ) && !response.location.IsNULLOrEmpty() ) )
        m_progressObserver->OnHttpClientResponseHeadersComplete( response );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::OnHttpClientBodyData( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_progressObserver )
        m_progressObserver->OnHttpClientBodyData( data, dataSize );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestRunner::OnHttpClientDataSent( UInt32 bytesSent )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_progressObserver )
        m_progressObserver->OnHttpClientDataSent( bytesSent );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
