/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTHTTP_H
#define GUCEF_AUTH_TESTAPP_TESTHTTP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#include "gucefWEB_CHttpClientRequestTask.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestPump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A request that is carried out on the thread pool while the calling thread pumps the global pulse generator, so that a
 *  server which runs on that pulse generator (the loopback listener) can answer. True when a response came.
 */
inline bool
PumpedExecute( const GUCEF::WEB::CHttpRequestData& request       ,
               GUCEF::WEB::CHttpResponseData& response           ,
               GUCEF::WEB::CHttpClientTransferInfo& info         ,
               const GUCEF::CORE::CString& proxyHost = GUCEF::CORE::CString::Empty ,
               GUCEF::UInt16 proxyPort = 0                       ,
               GUCEF::UInt32 timeoutInMs = 10000                 )
{GUCEF_TRACE;

    GUCEF::WEB::CHTTPClient client;
    GUCEF::WEB::CHttpClientRequestOptions options;
    options.followRedirects = false;
    options.totalTimeoutInMs = timeoutInMs;
    options.proxyHost = proxyHost;
    options.proxyPort = proxyPort;
    GUCEF::CORE::CFutureResult future = client.ExecuteAsync( request, options );
    if ( future.HasNoFuture() )
        return false;
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs + 2000 && !future.IsReady(); ++waited )
        PumpFor( 1 );
    if ( !future.IsReady() )
        return false;
    GUCEF::CORE::CTaskPtr task = future.GetResult( 0 );
    GUCEF::WEB::CHttpClientRequestTaskData* data = GUCEF::WEB::CHTTPClient::GetRequestTaskData( task );
    if ( GUCEF_NULL == data )
        return false;
    response = data->response;
    info = data->transferInfo;
    return info.success;
}

/*-------------------------------------------------------------------------*/

inline bool
PumpedGet( const GUCEF::CORE::CString& url, GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& method = "GET" )
{GUCEF_TRACE;

    GUCEF::WEB::CHttpRequestData request;
    GUCEF::WEB::CHttpClientTransferInfo info;
    request.requestType = method;
    if ( !request.SetUrl( url ) )
        return false;
    if ( "POST" == method )
        request.contentRepresentation = "text/plain";
    return PumpedExecute( request, response, info );
}

/*-------------------------------------------------------------------------*/

/** The value of a header of a response, ignoring the case of the name */
inline GUCEF::CORE::CString
HeaderOf( const GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& name )
{GUCEF_TRACE;

    for ( GUCEF::WEB::CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin(); i != response.otherHeaders.end(); ++i )
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
    return GUCEF::CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
BodyOf( const GUCEF::WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    return GUCEF::AUTH::MakeString( static_cast< const char* >( response.content.GetConstBufferPtr() ), response.content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTHTTP_H ? */
