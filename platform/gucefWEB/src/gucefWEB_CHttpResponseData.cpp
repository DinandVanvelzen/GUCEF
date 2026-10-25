/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CHttpResponseData.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpResponseData::CHttpResponseData( void )
    : CORE::CICloneable()         
    , allowedMethods()
    , cacheability()
    , eTag()
    , lastModified()
    , content()
    , contentEncoding()
    , statusCode()
    , contentType()
    , location()
    , acceptedTypes()
    , keepConnectionsAlive( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpResponseData::CHttpResponseData( const CHttpResponseData& src )
    : CORE::CICloneable( src )         
    , allowedMethods( src.allowedMethods )
    , cacheability( src.cacheability )
    , eTag( src.eTag )
    , lastModified( src.lastModified )
    , content( src.content )
    , contentEncoding( src.contentEncoding )
    , statusCode( src.statusCode )
    , contentType( src.contentType )
    , location( src.location )
    , acceptedTypes( src.acceptedTypes )
    , keepConnectionsAlive( src.keepConnectionsAlive )
    , otherHeaders( src.otherHeaders )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpResponseData::~CHttpResponseData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHttpResponseData::Clear( void )
{GUCEF_TRACE;

    allowedMethods.clear();
    cacheability.Clear();
    eTag.Clear();
    lastModified.Clear();
    content.SetDataSize( 0 );
    contentEncoding.Clear();
    statusCode = 0;
    contentType.Clear();
    location.Clear();
    acceptedTypes.clear();
    keepConnectionsAlive = true;
    otherHeaders.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CHttpResponseData::Clone( void ) const 
{GUCEF_TRACE;

    return GUCEF_NEW CHttpResponseData( *this );
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::Serialize( CORE::CDynamicBuffer& outputBuffer ) const
{GUCEF_TRACE;

    // @TODO: Optimize this
    
    // Protocol switching responses (web socket) need the reason phrase and an explicit Connection: Upgrade
    CString response = "HTTP/1.1 " + CORE::Int32ToString( statusCode ) + ( 101 == statusCode ? " Switching Protocols" : "" ) + "\r\nServer: gucefWEB/" + CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( false, true ) + "\r\n";

    // A Connection header explicitly provided by the caller takes precedence over the one we derive
    if ( otherHeaders.find( "Connection" ) == otherHeaders.end() )
    {
        if ( keepConnectionsAlive )
            response += "Connection: keep-alive\r\n";
        else
            response += "Connection: close\r\n";
    }
    if ( !location.IsNULLOrEmpty() )
    {
        response += "Content-Location: " + location + "\r\n";
    }
    if ( !eTag.IsNULLOrEmpty() )
    {
        response += "ETag: " + eTag + "\r\n";
    }
    if ( !cacheability.IsNULLOrEmpty() )
    {
        response += "Cache-Control: " + cacheability + "\r\n";
    }
    if ( !lastModified.IsNULLOrEmpty() )
    {
        response += "Last-Modified: " + lastModified + "\r\n";
    }
    if ( content.GetDataSize() > 0 )
    {
        if ( !contentEncoding.IsNULLOrEmpty() )
            response += "Content-Encoding: " + contentEncoding + "\r\n";
        response += "Content-Type: " + contentType + "\r\n";
    }
    if ( 101 != statusCode )
        response += "Content-Length: " + CORE::UInt32ToString( content.GetDataSize() ) + "\r\n";

    // You cannot send back accept types to the client using a standard HTTP header since
    // it is a HTTP request only header. We don't want the client to aimlessly retry different
    // representations either on PUT/POST so we do want to inform the client of the accepted types.
    // The client will need to have support for this operation to take advantage of it since it is a custom HTTP header
    if ( statusCode == 415 )
    {
        response += "Accept: ";
        TStringVector::const_iterator i = acceptedTypes.begin();
        while ( i != acceptedTypes.end() )
        {
            response += (*i) + ',';
            ++i;
        }
        response += "\r\n";
    }

    // If the operation was and a subset of allowed operations where specified
    // we will send those to the client.
    if ( statusCode == 405 && allowedMethods.size() > 0 )
    {
        response += "Allow: ";
        TStringVector::const_iterator i = allowedMethods.begin();
        while ( i != allowedMethods.end() )
        {
            response += (*i) + ',';
            ++i;
        }
        response += "\r\n";
    }

    // Add any other headers
    TStringMap::const_iterator i = otherHeaders.begin();
    while ( i != otherHeaders.end() )
    {
        response += (*i).first + ": " + (*i).second + "\r\n";
        ++i;
    }

    // Add the end of HTTP header delimiter
    response += "\r\n";

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "HttpResponseData(" << CORE::PointerToString( this ) << "): Finished building response header: " << CORE::CRedactor::ForHeaders().RedactHeaders( response ) );

    // Copy the HTTP header into the buffer
    outputBuffer.CopyFrom( response.Length(), response.C_String() );

    // Copy the HTTP message body into the buffer
    if ( content.GetDataSize() > 0 )
    {
        outputBuffer.CopyFrom( response.Length(), content.GetDataSize(), content.GetConstBufferPtr() );
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
