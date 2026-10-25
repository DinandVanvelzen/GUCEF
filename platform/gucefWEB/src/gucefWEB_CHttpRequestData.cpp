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

#include "gucefWEB_CHttpRequestData.h"

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

CHttpRequestData::CHttpRequestData( void )
    : CORE::CICloneable()
    , resourceRepresentations()
    , encodingRepresentations()
    , resourceVersions()
    , requestProtocol()
    , requestProtocolVersion()
    , requestType()
    , requestHost()
    , requestUri()
    , requestUriParams()
    , content()
    , contentRepresentation()
    , transactionID()
    , keepConnectionsAlive( true )
    , isWebSocketUpgrade( false )
    , m_requestUriWithAuthority()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpRequestData::CHttpRequestData( const CHttpRequestData& src )
    : CORE::CICloneable( src )
    , resourceRepresentations( src.resourceRepresentations )
    , encodingRepresentations( src.encodingRepresentations )
    , resourceVersions( src.resourceVersions )
    , requestProtocol( src.requestProtocol )
    , requestProtocolVersion( src.requestProtocolVersion )
    , requestType( src.requestType )
    , requestHost( src.requestHost )
    , requestUri( src.requestUri )
    , requestUriParams( src.requestUriParams )
    , content( src.content )
    , contentRepresentation( src.contentRepresentation )
    , transactionID( src.transactionID )
    , keepConnectionsAlive( src.keepConnectionsAlive )
    , isWebSocketUpgrade( src.isWebSocketUpgrade )
    , otherHeaders( src.otherHeaders )
    , m_requestUriWithAuthority( src.m_requestUriWithAuthority )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpRequestData::~CHttpRequestData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CHttpRequestData::Clone( void ) const 
{GUCEF_TRACE;

    return GUCEF_NEW CHttpRequestData( *this );
}

/*-------------------------------------------------------------------------*/

CString 
CHttpRequestData::ConstructUriWithAuthority( void ) const
{GUCEF_TRACE;
    
    if ( m_requestUriWithAuthority.IsNULLOrEmpty() )
    {
        m_requestUriWithAuthority = requestProtocol.Lowercase() + "://" + requestHost + requestUri;        
    }
    return m_requestUriWithAuthority;
}

/*-------------------------------------------------------------------------*/

bool
CHttpRequestData::SetUrl( const CString& url )
{GUCEF_TRACE;

    Int32 schemeEnd = url.HasSubstr( "://", true );
    if ( schemeEnd <= 0 )
        return false;

    CString remainder = url.CutChars( (UInt32) schemeEnd + 3, true );

    // drop the fragment, it is not sent to the server
    Int32 fragmentStart = remainder.HasChar( '#', true );
    if ( fragmentStart >= 0 )
        remainder = remainder.SubstrToIndex( (UInt32) fragmentStart, true );

    Int32 pathStart = remainder.HasChar( '/', true );
    Int32 queryStart = remainder.HasChar( '?', true );
    Int32 authorityEnd = pathStart;
    if ( queryStart >= 0 && ( authorityEnd < 0 || queryStart < authorityEnd ) )
        authorityEnd = queryStart;

    CString authority = authorityEnd >= 0 ? remainder.SubstrToIndex( (UInt32) authorityEnd, true ) : remainder;
    if ( authority.IsNULLOrEmpty() )
        return false;

    CString pathAndQuery = authorityEnd >= 0 ? remainder.CutChars( (UInt32) authorityEnd, true ) : CString();
    CString path = pathAndQuery;
    CString query;
    queryStart = pathAndQuery.HasChar( '?', true );
    if ( queryStart >= 0 )
    {
        path = pathAndQuery.SubstrToIndex( (UInt32) queryStart, true );
        query = pathAndQuery.CutChars( (UInt32) queryStart + 1, true );
    }
    if ( path.IsNULLOrEmpty() )
        path = "/";

    requestProtocol = url.SubstrToIndex( (UInt32) schemeEnd, true ).Lowercase();
    requestHost = authority;
    requestUri = path;
    requestUriParams = query;
    m_requestUriWithAuthority.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CHttpRequestData::GetUrl( void ) const
{GUCEF_TRACE;

    CString url = ConstructUriWithAuthority();
    if ( !requestUriParams.IsNULLOrEmpty() )
        url += "?" + requestUriParams;
    return url;
}

/*-------------------------------------------------------------------------*/

CString
CHttpRequestData::GetHostName( void ) const
{GUCEF_TRACE;

    if ( requestHost.Length() > 0 && '[' == requestHost[ 0 ] )
    {
        // IPv6 address literal: [address]:port
        Int32 closingBracket = requestHost.HasChar( ']', true );
        if ( closingBracket > 0 )
            return requestHost.SubstrFromRange( 1, (UInt32) closingBracket );
        return requestHost;
    }

    Int32 portSeparator = requestHost.HasChar( ':', false );
    if ( portSeparator >= 0 )
        return requestHost.SubstrToIndex( (UInt32) portSeparator, true );
    return requestHost;
}

/*-------------------------------------------------------------------------*/

UInt16
CHttpRequestData::GetPort( void ) const
{GUCEF_TRACE;

    Int32 portSeparator = requestHost.HasChar( ':', false );
    Int32 closingBracket = requestHost.HasChar( ']', false );
    if ( portSeparator >= 0 && portSeparator > closingBracket )
    {
        UInt16 port = CORE::StringToUInt16( requestHost.CutChars( (UInt32) portSeparator + 1, true ) );
        if ( 0 != port )
            return port;
    }
    return requestProtocol.Equals( "https", false ) ? 443 : 80;
}

/**
 *  Splits the authority of a request target in the lowercase host and the port, the user info is dropped
 *  @return false if there is no scheme or host, or if the port is not a number from 1 to 65535
 */
static bool
ParseOrigin( const CString& protocol, const CString& authority, CString& host, UInt16& port )
{GUCEF_TRACE;

    if ( protocol.IsNULLOrEmpty() )
        return false;

    // what looks like host:port in front of the last @ is user info, the host is what follows it
    CString hostAndPort = authority;
    Int32 userInfoEnd = authority.HasChar( '@', false );
    if ( userInfoEnd >= 0 )
        hostAndPort = authority.CutChars( (UInt32) userInfoEnd + 1, true );

    CString portText;
    Int32 portSeparator = hostAndPort.HasChar( ':', false );
    Int32 closingBracket = hostAndPort.HasChar( ']', false );
    if ( portSeparator >= 0 && portSeparator > closingBracket )
    {
        host = hostAndPort.SubstrToIndex( (UInt32) portSeparator, true );
        portText = hostAndPort.CutChars( (UInt32) portSeparator + 1, true );
    }
    else
        host = hostAndPort;

    host = host.Lowercase();
    if ( host.IsNULLOrEmpty() )
        return false;

    if ( portText.IsNULLOrEmpty() )
    {
        port = protocol.Equals( "https", false ) ? 443 : 80;
        return true;
    }

    if ( portText.Length() > 5 )
        return false;
    UInt32 portNumber = 0;
    for ( UInt32 i=0; i<portText.Length(); ++i )
    {
        if ( portText[ i ] < '0' || portText[ i ] > '9' )
            return false;
        portNumber = portNumber * 10 + (UInt32) ( portText[ i ] - '0' );
    }
    if ( 0 == portNumber || portNumber > 65535 )
        return false;
    port = (UInt16) portNumber;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpRequestData::IsSameOrigin( const CString& protocolA, const CString& hostA ,
                                const CString& protocolB, const CString& hostB )
{GUCEF_TRACE;

    CString nameA, nameB;
    UInt16 portA = 0, portB = 0;
    if ( !ParseOrigin( protocolA, hostA, nameA, portA ) || !ParseOrigin( protocolB, hostB, nameB, portB ) )
        return false;

    return protocolA.Equals( protocolB, false ) && portA == portB && nameA == nameB;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace WEB */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
