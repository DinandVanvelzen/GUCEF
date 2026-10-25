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

#include "gucefWEB_CHttpClientRequestOptions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CHttpClientRequestOptions::DefaultUserAgent = "GUCEF-HTTP/1.0";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpClientRequestOptions::CHttpClientRequestOptions( void )
    : CORE::CICloneable()
    , requestBody()
    , responseBodySink()
    , hasRange( false )
    , rangeStart( 0 )
    , rangeLength( -1 )
    , connectTimeoutInMs( 30000 )
    , totalTimeoutInMs( 0 )
    , followRedirects( true )
    , maxRedirects( 10 )
    , verifyPeer( true )
    , caBundlePath()
    , cookieJar()
    , proxyHost()
    , proxyPort( 0 )
    , useEnvironmentProxy( true )
    , userAgent( DefaultUserAgent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientRequestOptions::CHttpClientRequestOptions( const CHttpClientRequestOptions& src )
    : CORE::CICloneable( src )
    , requestBody( src.requestBody )
    , responseBodySink( src.responseBodySink )
    , hasRange( src.hasRange )
    , rangeStart( src.rangeStart )
    , rangeLength( src.rangeLength )
    , connectTimeoutInMs( src.connectTimeoutInMs )
    , totalTimeoutInMs( src.totalTimeoutInMs )
    , followRedirects( src.followRedirects )
    , maxRedirects( src.maxRedirects )
    , verifyPeer( src.verifyPeer )
    , caBundlePath( src.caBundlePath )
    , cookieJar( src.cookieJar )
    , proxyHost( src.proxyHost )
    , proxyPort( src.proxyPort )
    , useEnvironmentProxy( src.useEnvironmentProxy )
    , userAgent( src.userAgent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientRequestOptions::~CHttpClientRequestOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientRequestOptions&
CHttpClientRequestOptions::operator=( const CHttpClientRequestOptions& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        requestBody = src.requestBody;
        responseBodySink = src.responseBodySink;
        hasRange = src.hasRange;
        rangeStart = src.rangeStart;
        rangeLength = src.rangeLength;
        connectTimeoutInMs = src.connectTimeoutInMs;
        totalTimeoutInMs = src.totalTimeoutInMs;
        followRedirects = src.followRedirects;
        maxRedirects = src.maxRedirects;
        verifyPeer = src.verifyPeer;
        caBundlePath = src.caBundlePath;
        cookieJar = src.cookieJar;
        proxyHost = src.proxyHost;
        proxyPort = src.proxyPort;
        useEnvironmentProxy = src.useEnvironmentProxy;
        userAgent = src.userAgent;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CHttpClientRequestOptions::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHttpClientRequestOptions( *this );
}

/*-------------------------------------------------------------------------*/

const char*
HttpTransferErrorClassToString( THttpTransferErrorClass errorClass )
{
    switch ( errorClass )
    {
        case HTTPERRORCLASS_NO_BACKEND :    return "no-backend";
        case HTTPERRORCLASS_DNS :           return "dns";
        case HTTPERRORCLASS_CONNECT :       return "connect";
        case HTTPERRORCLASS_TLS :           return "tls";
        case HTTPERRORCLASS_TIMEOUT :       return "timeout";
        case HTTPERRORCLASS_OTHER :         return "other";
        default :                           return "none";
    }
}

/*-------------------------------------------------------------------------*/

CHttpClientTransferInfo::CHttpClientTransferInfo( void )
    : CORE::CICloneable()
    , success( false )
    , errorCode( 0 )
    , errorDescription()
    , errorClass( HTTPERRORCLASS_NONE )
    , wasCancelled( false )
    , timedOut( false )
    , effectiveUrl()
    , redirectCount( 0 )
    , bytesSent( 0 )
    , bytesReceived( 0 )
    , durationInMs( 0 )
    , backendName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientTransferInfo::~CHttpClientTransferInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHttpClientTransferInfo::Clear( void )
{GUCEF_TRACE;

    *this = CHttpClientTransferInfo();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CHttpClientTransferInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHttpClientTransferInfo( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
