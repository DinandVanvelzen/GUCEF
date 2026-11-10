/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_WEB_CSSEEVENTWRITER_H
#include "gucefWEB_CSseEventWriter.h"
#define GUCEF_WEB_CSSEEVENTWRITER_H
#endif /* GUCEF_WEB_CSSEEVENTWRITER_H ? */

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#include "gucefWEB_CMcpStreamableHttpTransport.h"
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */

#include "gucefWEB_CMcpHttpExchangeChannel.h"

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

static const char* McpParamHeaderPrefix = "mcp-param-";

/*-------------------------------------------------------------------------*/

/** A header value as RFC 9110 has it: visible ASCII, space and horizontal tab */
static bool
IsHeaderValueText( const CORE::CString& value )
{GUCEF_TRACE;

    const char* text = value.C_String();
    const UInt32 size = value.ByteSize() > 0 ? value.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( !( ( c >= 0x21 && c <= 0x7E ) || ' ' == c || '\t' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Client text as UTF-8: a byte that does not belong to a valid UTF-8 sequence becomes U+FFFD. A header value decoded
 *  from base64 can be any bytes, and a message or a log line must stay text
 */
static CORE::CString
ToValidUtf8( const CORE::CString& text )
{GUCEF_TRACE;

    const unsigned char* bytes = reinterpret_cast< const unsigned char* >( text.C_String() );
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    CORE::CString result;
    UInt32 i = 0;
    while ( i < size )
    {
        const unsigned char c = bytes[ i ];
        UInt32 length = 0;
        UInt32 codePoint = 0;
        if ( c < 0x80 ) { length = 1; codePoint = c; }
        else
        if ( c >= 0xC2 && c <= 0xDF ) { length = 2; codePoint = c & 0x1F; }
        else
        if ( c >= 0xE0 && c <= 0xEF ) { length = 3; codePoint = c & 0x0F; }
        else
        if ( c >= 0xF0 && c <= 0xF4 ) { length = 4; codePoint = c & 0x07; }
        bool valid = length > 0 && i + length <= size;
        for ( UInt32 n=1; valid && n<length; ++n )
        {
            valid = 0x80 == ( bytes[ i+n ] & 0xC0 );
            codePoint = ( codePoint << 6 ) | ( bytes[ i+n ] & 0x3F );
        }
        if ( valid && ( ( 3 == length && codePoint < 0x800 ) || ( 4 == length && ( codePoint < 0x10000 || codePoint > 0x10FFFF ) ) ||
                        ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) ) )
            valid = false;
        if ( !valid )
        {
            result += "\xEF\xBF\xBD";
            ++i;
            continue;
        }
        result.Append( text.C_String() + i, length );
        i += length;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** Client text for a message or a log line: UTF-8, redacted, and at most 128 bytes, cut between characters */
static CORE::CString
ForMessage( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString redacted = CHttpLogRedaction::RedactText( ToValidUtf8( text ) );
    const UInt32 size = redacted.ByteSize() > 0 ? redacted.ByteSize() - 1 : 0;
    if ( size > 128 )
    {
        // back to the first byte of the character at the cut: a continuation byte is 10xxxxxx
        UInt32 cut = 128;
        const char* bytes = redacted.C_String();
        while ( cut > 0 && 0x80 == ( ( (unsigned char) bytes[ cut ] ) & 0xC0 ) )
            --cut;
        redacted = CORE::CString( bytes, cut ) + "...";
    }
    return redacted;
}

/*-------------------------------------------------------------------------*/

/**
 *  The value of an integer header parameter: a decimal integer, an optional '-' and digits, as the transport page asks.
 *  A fraction of zeros (42.0) is taken as the integer it is, as decided earlier; hexadecimal, exponents, a '+',
 *  infinity and whitespace are no decimal integer. At most 20 digits, without overflow
 */
static bool
ParseDecimalIntegerHeader( const CORE::CString& text, bool& negative, UInt64& magnitude )
{GUCEF_TRACE;

    const char* chars = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 i = 0;
    negative = false;
    magnitude = 0;
    if ( i < size && '-' == chars[ i ] )
    {
        negative = true;
        ++i;
    }
    const UInt32 digitsStart = i;
    while ( i < size && chars[ i ] >= '0' && chars[ i ] <= '9' )
    {
        const UInt64 digit = (UInt64) ( chars[ i ] - '0' );
        if ( magnitude > ( 18446744073709551615ULL - digit ) / 10 )
            return false;
        magnitude = magnitude * 10 + digit;
        ++i;
    }
    if ( i == digitsStart )
        return false;
    if ( i < size && '.' == chars[ i ] )
    {
        ++i;
        const UInt32 fractionStart = i;
        while ( i < size && '0' == chars[ i ] )
            ++i;
        if ( i == fractionStart )
            return false;
    }
    if ( 0 == magnitude )
        negative = false;
    return i == size;
}

/*-------------------------------------------------------------------------*/

/** A JSON number of the body as sign and magnitude, when it is a whole number within 64 bits */
static bool
GetWholeNumber( const CORE::CVariant& value, bool& negative, UInt64& magnitude )
{GUCEF_TRACE;

    if ( value.IsInteger() )
    {
        if ( value.IsUnsignedInteger() )
        {
            negative = false;
            magnitude = value.AsUInt64();
            return true;
        }
        const Int64 signedValue = value.AsInt64();
        negative = signedValue < 0;
        magnitude = signedValue < 0 ? ( (UInt64) ( -( signedValue + 1 ) ) ) + 1 : (UInt64) signedValue;
        return true;
    }
    if ( value.IsFloat() )
    {
        const Float64 real = value.AsFloat64();
        if ( real != real || real - real != 0.0 || floor( real ) != real || fabs( real ) >= 18446744073709551616.0 )
            return false;
        magnitude = (UInt64) fabs( real );
        negative = real < 0.0 && 0 != magnitude;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** A media range of Accept, without its parameters, against what it covers */
static void
GetAcceptedKinds( const CHttpRequestData& request, bool& json, bool& sse )
{GUCEF_TRACE;

    json = false;
    sse = false;
    if ( request.resourceRepresentations.empty() )
    {
        // no Accept field: any media type is acceptable (RFC 9110 12.5.1)
        json = true;
        sse = true;
        return;
    }
    for ( size_t i=0; i<request.resourceRepresentations.size(); ++i )
    {
        const CORE::CString range = request.resourceRepresentations[ i ].Lowercase();
        if ( "*/*" == range )
        {
            json = true;
            sse = true;
        }
        else
        if ( "application/json" == range || "application/*" == range )
            json = true;
        else
        if ( "text/event-stream" == range || "text/*" == range )
            sse = true;
    }
}

/*-------------------------------------------------------------------------*/

/** The member of the arguments at a path of property names; GUCEF_NULL when there is none or it is null */
static bool
FindArgumentValue( const CORE::CDataNode* arguments, const CORE::CString::StringVector& path, CORE::CVariant& value, bool& isContainer )
{GUCEF_TRACE;

    isContainer = false;
    const CORE::CDataNode* node = arguments;
    for ( size_t i=0; i<path.size() && GUCEF_NULL != node; ++i )
    {
        if ( GUCEF_DATATYPE_OBJECT != node->GetNodeType() )
            return false;
        if ( i+1 == path.size() )
        {
            const CORE::CDataNode::TKeyValuePair* attribute = node->GetAttribute( path[ i ] );
            if ( GUCEF_NULL != attribute )
            {
                if ( (*attribute).second.IsNIL() )
                    return false;
                value = (*attribute).second;
                return true;
            }
            const CORE::CDataNode* child = node->FindChild( path[ i ] );
            if ( GUCEF_NULL == child || GUCEF_DATATYPE_NIL == child->GetNodeType() )
                return false;
            isContainer = true;
            return true;
        }
        node = node->FindChild( path[ i ] );
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpHttpExchangeChannel::CMcpHttpExchangeChannel( const COM::CMcpPrincipal& principal          ,
                                                  CMcpStreamableHttpTransport& transport       ,
                                                  const CHttpServerExchangePtr& exchange       ,
                                                  const CHttpRequestData& request              )
    : COM::CMcpChannel( CHANNEL_EXCHANGE, principal )
    , m_transport( &transport )
    , m_exchange( exchange )
    , m_exchangeId( exchange.IsNULL() ? 0 : exchange->GetExchangeId() )
    , m_hasProtocolVersionHeader( false )
    , m_protocolVersionHeader()
    , m_hasMethodHeader( false )
    , m_methodHeader()
    , m_hasNameHeader( false )
    , m_nameHeader()
    , m_paramHeaders()
    , m_acceptsJson( false )
    , m_acceptsSse( false )
    , m_requestReceived( false )
    , m_headerChecksDone( false )
    , m_legacy( false )
    , m_responded( false )
    , m_streaming( false )
    , m_errorStatus( 200 )
    , m_forcedStatus( 0 )
    , m_eventBuffer()
{GUCEF_TRACE;

    // header names were lowercased by the HTTP server; a field that came more than once has its values joined by
    // commas, so that a second Mcp-Method cannot hide behind a first one
    const CHttpRequestData::TStringMap& headers = request.otherHeaders;
    CHttpRequestData::TStringMap::const_iterator i = headers.find( "mcp-protocol-version" );
    if ( i != headers.end() )
    {
        m_hasProtocolVersionHeader = true;
        m_protocolVersionHeader = (*i).second;
    }
    i = headers.find( "mcp-method" );
    if ( i != headers.end() )
    {
        m_hasMethodHeader = true;
        m_methodHeader = (*i).second;
    }
    i = headers.find( "mcp-name" );
    if ( i != headers.end() )
    {
        m_hasNameHeader = true;
        m_nameHeader = (*i).second;
    }
    const UInt32 prefixLength = (UInt32) strlen( McpParamHeaderPrefix );
    for ( i = headers.begin(); i != headers.end(); ++i )
    {
        if ( (*i).first.Length() > prefixLength && 0 == (*i).first.HasSubstr( McpParamHeaderPrefix, 0, true ) )
            m_paramHeaders[ (*i).first.CutChars( prefixLength, true ) ] = (*i).second;
    }
    GetAcceptedKinds( request, m_acceptsJson, m_acceptsSse );
}

/*-------------------------------------------------------------------------*/

CMcpHttpExchangeChannel::~CMcpHttpExchangeChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpHttpExchangeChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CMcpHttpExchangeChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpHttpExchangeChannel::GetExchangeId( void ) const
{GUCEF_TRACE;

    return m_exchangeId;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::HasResponded( void ) const
{GUCEF_TRACE;

    return m_responded;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::IsStreaming( void ) const
{GUCEF_TRACE;

    return m_streaming && !m_responded && !m_exchange.IsNULL();
}

/*-------------------------------------------------------------------------*/

int
CMcpHttpExchangeChannel::StatusForErrorCode( Int32 code, bool afterHeaderChecks, bool legacy )
{GUCEF_TRACE;

    switch ( code )
    {
        case COM::MCP_ERROR_PARSE:
        case COM::MCP_ERROR_INVALID_REQUEST:
        case COM::MCP_ERROR_HEADER_MISMATCH:
        case COM::MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY:
        case COM::MCP_ERROR_UNSUPPORTED_PROTOCOL_VERSION:
            return 400;
        case COM::MCP_ERROR_INVALID_PARAMS:
            return afterHeaderChecks ? 200 : 400;
        case COM::MCP_ERROR_METHOD_NOT_FOUND:
            return legacy ? 200 : 404;
        case COM::MCP_ERROR_GUCEF_ACCESS_DENIED:
            return 403;
        case COM::MCP_ERROR_GUCEF_RATE_LIMITED:
            return 429;
        case COM::MCP_ERROR_GUCEF_BUSY:
            return 503;
        default:
            return 200;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::DecodeHeaderValue( const CORE::CString& headerValue, CORE::CString& decoded )
{GUCEF_TRACE;

    if ( !IsHeaderValueText( headerValue ) )
        return false;

    static const char* sentinelStart = "=?base64?";
    static const char* sentinelEnd = "?=";
    const UInt32 size = headerValue.Length();
    if ( size >= 11 && 0 == headerValue.HasSubstr( sentinelStart, 0, true ) && headerValue.SubstrToIndex( size-2, false ) == sentinelEnd )
    {
        CORE::CString base64 = headerValue.SubstrFromRange( 9, size-2 );
        if ( base64.IsNULLOrEmpty() )
        {
            decoded.Clear();
            return true;
        }
        CORE::CDynamicBuffer bytes;
        bytes.SetBufferSize( base64.Length() + 4, false );
        UInt32 used = 0;
        if ( !CORE::Base64Decode( base64, bytes.GetBufferPtr(), bytes.GetBufferSize(), used ) )
            return false;
        decoded = CORE::CString( static_cast< const char* >( bytes.GetConstBufferPtr() ), used );
        return true;
    }
    decoded = headerValue;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::OnRequestReceived( const COM::SJsonRpcRequest& request )
{GUCEF_TRACE;

    m_requestReceived = true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::IsLegacyRequest( const COM::SJsonRpcRequest& request ) const
{GUCEF_TRACE;

    // without sessions the header says which revision a request without modern _meta is of
    return m_hasProtocolVersionHeader && COM::CMcpProtocolVersions::IsLegacyVersion( m_protocolVersionHeader );
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::OnErrorCode( Int32 code )
{GUCEF_TRACE;

    m_errorStatus = StatusForErrorCode( code, m_headerChecksDone, m_legacy );
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::ValidateRequestHeaders( const COM::SJsonRpcRequest& request ,
                                                 const COM::CMcpRequestMeta* meta    ,
                                                 COM::EMcpProtocolEra era            ,
                                                 Int32& errorCode                    ,
                                                 CORE::CString& errorMessage         )
{GUCEF_TRACE;

    m_headerChecksDone = true;
    m_legacy = ( COM::MCP_ERA_LEGACY == era );

    // a stream of change notifications needs a client that takes a stream
    if ( request.method == COM::CMcpMethods::SubscriptionsListen && !m_acceptsSse )
    {
        m_forcedStatus = 406;
        errorCode = COM::MCP_ERROR_INVALID_REQUEST;
        errorMessage = "Not acceptable: subscriptions/listen answers with text/event-stream, which the Accept header does not list";
        return false;
    }

    // 2025-11-25 did not define the Mcp-* headers
    if ( m_legacy || GUCEF_NULL == meta )
        return true;

    errorCode = COM::MCP_ERROR_HEADER_MISMATCH;
    if ( !m_hasProtocolVersionHeader )
    {
        errorMessage = "Header mismatch: the MCP-Protocol-Version header is required";
        return false;
    }
    if ( m_protocolVersionHeader != meta->GetProtocolVersion() )
    {
        errorMessage = "Header mismatch: MCP-Protocol-Version header value '" + ForMessage( m_protocolVersionHeader ) + "' does not match body value '" + ForMessage( meta->GetProtocolVersion() ) + "'";
        return false;
    }
    if ( !m_hasMethodHeader )
    {
        errorMessage = "Header mismatch: the Mcp-Method header is required";
        return false;
    }
    if ( m_methodHeader != request.method )
    {
        errorMessage = "Header mismatch: Mcp-Method header value '" + ForMessage( m_methodHeader ) + "' does not match body value '" + ForMessage( request.method ) + "'";
        return false;
    }

    // the name or URI of the target, for the methods that have one
    const char* nameMember = GUCEF_NULL;
    if ( request.method == COM::CMcpMethods::ToolsCall || request.method == COM::CMcpMethods::PromptsGet )
        nameMember = "name";
    else
    if ( request.method == COM::CMcpMethods::ResourcesRead )
        nameMember = "uri";
    if ( GUCEF_NULL != nameMember )
    {
        if ( !m_hasNameHeader )
        {
            errorMessage = "Header mismatch: the Mcp-Name header is required for " + request.method;
            return false;
        }
        CORE::CString decoded;
        if ( !DecodeHeaderValue( m_nameHeader, decoded ) )
        {
            errorMessage = "Header mismatch: the Mcp-Name header value is malformed";
            return false;
        }
        const CORE::CDataNode::TKeyValuePair* member = request.hasParams ? request.params.GetAttribute( nameMember ) : GUCEF_NULL;
        const CORE::CString bodyValue = ( GUCEF_NULL != member && (*member).second.IsString() ) ? (*member).second.AsString() : CORE::CString::Empty;
        if ( GUCEF_NULL == member || !(*member).second.IsString() || decoded != bodyValue )
        {
            errorMessage = "Header mismatch: Mcp-Name header value '" + ForMessage( decoded ) + "' does not match body value '" + ForMessage( bodyValue ) + "'";
            return false;
        }
    }

    if ( request.method == COM::CMcpMethods::ToolsCall && !CheckParamHeaders( request, errorCode, errorMessage ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::CheckParamHeaders( const COM::SJsonRpcRequest& request, Int32& errorCode, CORE::CString& errorMessage )
{GUCEF_TRACE;

    COM::CMcpServer* server = GUCEF_NULL != m_transport ? m_transport->GetServer() : GUCEF_NULL;
    if ( GUCEF_NULL == server || !request.hasParams )
        return true;
    const CORE::CDataNode::TKeyValuePair* name = request.params.GetAttribute( "name" );
    if ( GUCEF_NULL == name || !(*name).second.IsString() )
        return true;

    // a tool that is unknown or that the principal cannot see is answered by the server as if it did not exist:
    // its header parameters are not looked at, so that a mismatch cannot tell it exists
    COM::CMcpToolEntryPtr entry;
    if ( !server->GetToolRegistry().TryGet( (*name).second.AsString(), entry ) || entry.IsNULL() )
        return true;
    COM::CIMcpAccessPolicyPtr policy = server->GetAccessPolicy();
    if ( !policy.IsNULL() && !policy->IsToolVisible( GetPrincipal(), *entry ) )
        return true;

    const CORE::CDataNode* arguments = request.params.FindChild( "arguments" );
    errorCode = COM::MCP_ERROR_HEADER_MISMATCH;
    for ( size_t p=0; p<entry->headerParameters.size(); ++p )
    {
        const COM::CMcpToolEntry::SHeaderParameter& parameter = entry->headerParameters[ p ];
        const CORE::CString headerName = "Mcp-Param-" + parameter.headerName;
        TStringMap::const_iterator header = m_paramHeaders.find( parameter.headerName.Lowercase() );

        CORE::CVariant value;
        bool isContainer = false;
        const bool present = FindArgumentValue( arguments, parameter.propertyPath, value, isContainer );
        if ( !present )
        {
            if ( header != m_paramHeaders.end() )
            {
                errorMessage = "Header mismatch: the " + headerName + " header is present but the argument it mirrors is not";
                return false;
            }
            continue;
        }
        if ( header == m_paramHeaders.end() )
        {
            errorMessage = "Header mismatch: the " + headerName + " header is required, the argument it mirrors is present";
            return false;
        }
        CORE::CString decoded;
        if ( !DecodeHeaderValue( (*header).second, decoded ) )
        {
            errorMessage = "Header mismatch: the " + headerName + " header value is malformed";
            return false;
        }
        bool equal = false;
        CORE::CString bodyText;
        if ( isContainer )
        {
            bodyText = "<not a primitive value>";
        }
        else
        if ( "integer" == parameter.type )
        {
            // a decimal integer, compared exactly as sign and magnitude: 42.0 is 42, 0x2A and 4.2e1 are no decimal integer
            bodyText = value.AsString();
            bool headerNegative = false;
            UInt64 headerMagnitude = 0;
            bool bodyNegative = false;
            UInt64 bodyMagnitude = 0;
            equal = ParseDecimalIntegerHeader( decoded, headerNegative, headerMagnitude ) && GetWholeNumber( value, bodyNegative, bodyMagnitude ) &&
                    headerNegative == bodyNegative && headerMagnitude == bodyMagnitude;
        }
        else
        if ( "boolean" == parameter.type )
        {
            bodyText = value.IsBoolean() ? ( value.AsBool() ? "true" : "false" ) : value.AsString();
            equal = value.IsBoolean() && decoded == bodyText;
        }
        else
        {
            bodyText = value.AsString();
            equal = value.IsString() && decoded == bodyText;
        }
        if ( !equal )
        {
            errorMessage = "Header mismatch: " + headerName + " header value '" + ForMessage( decoded ) + "' does not match body value '" + ForMessage( bodyText ) + "'";
            return false;
        }
    }

    // a recognized header must hold valid characters even when nothing in the body asks for it
    for ( TStringMap::const_iterator h = m_paramHeaders.begin(); h != m_paramHeaders.end(); ++h )
    {
        if ( !IsHeaderValueText( (*h).second ) )
        {
            errorMessage = "Header mismatch: the Mcp-Param-" + (*h).first + " header value holds characters a header value cannot have";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::SendWholeResponse( int status, const CORE::CString& body )
{GUCEF_TRACE;

    if ( m_exchange.IsNULL() || !m_exchange->IsOpen() )
        return false;
    CHttpResponseData response;
    response.statusCode = status;
    if ( !body.IsNULLOrEmpty() )
    {
        response.contentType = "application/json";
        // bytes, not characters: a UTF-8 text is longer in bytes than in characters
        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
    }
    return m_exchange->SendResponse( response );
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::BeginStream( void )
{GUCEF_TRACE;

    if ( m_streaming )
        return true;
    if ( m_exchange.IsNULL() || !m_exchange->IsOpen() )
        return false;
    CHttpResponseData head;
    head.statusCode = 200;
    head.contentType = "text/event-stream";
    head.cacheability = "no-cache";
    head.otherHeaders[ "X-Accel-Buffering" ] = "no";
    if ( !m_exchange->BeginStream( head ) )
        return false;
    m_streaming = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::Done( void )
{GUCEF_TRACE;

    m_exchange.Unlink();
    if ( GUCEF_NULL != m_transport )
        m_transport->OnChannelDone( *this );
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::SendMessageText( const CORE::CString& messageText )
{GUCEF_TRACE;

    if ( m_responded || m_exchange.IsNULL() || !m_exchange->IsOpen() )
        return false;

    if ( OUTGOING_NOTIFICATION == GetOutgoingKind() )
    {
        // a client that only takes JSON gets the response without what would have to come before it
        if ( !m_acceptsSse )
            return true;
        if ( !BeginStream() )
            return false;
        m_eventBuffer.SetDataSize( 0 );
        CSseEventWriter::AppendEvent( m_eventBuffer, messageText );
        return m_exchange->SendStreamData( m_eventBuffer );
    }

    // the response: the last message of this exchange
    m_responded = true;
    bool sent = false;
    if ( m_streaming )
    {
        m_eventBuffer.SetDataSize( 0 );
        CSseEventWriter::AppendEvent( m_eventBuffer, messageText );
        sent = m_exchange->SendStreamData( m_eventBuffer );
        m_exchange->EndStream();
    }
    else
    {
        int status = 200;
        const EOutgoingKind kind = GetOutgoingKind();
        if ( OUTGOING_ENDPOINT_ERROR == kind )
            status = 400;           // not a JSON-RPC request at all: a parse error, a batch, a null id
        else
        if ( OUTGOING_ERROR == kind )
            status = m_errorStatus;
        if ( 0 != m_forcedStatus )
            status = m_forcedStatus;
        sent = SendWholeResponse( status, messageText );
    }
    Done();
    return sent;
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::FinishIncoming( void )
{GUCEF_TRACE;

    if ( m_responded || m_requestReceived || m_exchange.IsNULL() )
        return;

    // a notification, or a response a legacy client sent: accepted, nothing to answer with
    m_responded = true;
    SendWholeResponse( 202, CORE::CString::Empty );
    Done();
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpExchangeChannel::SendKeepAlive( void )
{GUCEF_TRACE;

    if ( !IsStreaming() )
        return false;
    m_eventBuffer.SetDataSize( 0 );
    CSseEventWriter::AppendComment( m_eventBuffer );
    return m_exchange->SendStreamData( m_eventBuffer );
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::DropExchange( void )
{GUCEF_TRACE;

    m_exchange.Unlink();
    m_transport = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::OnRequestCancelled( const CORE::CVariant& id )
{GUCEF_TRACE;

    // a cancelled request gets no JSON-RPC response; the POST is still completed: a stream ends, a JSON reply that
    // never started is 204 No Content
    if ( m_responded || m_exchange.IsNULL() )
        return;
    m_responded = true;
    if ( m_streaming )
        m_exchange->EndStream();
    else
        SendWholeResponse( 204, CORE::CString::Empty );
    Done();
}

/*-------------------------------------------------------------------------*/

void
CMcpHttpExchangeChannel::OnChannelClosed( void )
{GUCEF_TRACE;

    // the server closed the channel while the exchange is still open, without a response: the stream ends, or the
    // client hears that the server cannot answer
    if ( m_responded || m_exchange.IsNULL() )
        return;
    m_responded = true;
    if ( m_streaming )
        m_exchange->EndStream();
    else
        SendWholeResponse( 503, CORE::CString::Empty );
    Done();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
