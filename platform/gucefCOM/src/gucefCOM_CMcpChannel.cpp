/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#include <string.h>

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpChannel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpChannel::CMcpChannel( EKind kind, const CMcpPrincipal& principal )
    : CORE::CObservingNotifier()
    , CIJsonRpcTransport()
    , CIJsonRpcRequestHandler()
    , m_kind( kind )
    , m_principal( principal )
    , m_endpoint( GUCEF_NULL )
    , m_server( GUCEF_NULL )
    , m_channelId( 0 )
    , m_closed( true )
    , m_maxMessageBytes( DefaultMaxMessageBytes )
    , m_maxJsonDepth( DefaultMaxJsonDepth )
    , m_legacySession( GUCEF_NULL )
    , m_outgoingKind( OUTGOING_ENDPOINT_ERROR )
    , m_incomingCheck()
{GUCEF_TRACE;

    m_endpoint.SetTransport( this );
    m_endpoint.SetRequestHandler( this );
    m_endpoint.SetDialect( JSONRPC_DIALECT_STRICT_2_0 );
    m_endpoint.SetAllowBatches( false );
    m_endpoint.SetAllowNullRequestId( false );

    typedef CORE::CTEventHandlerFunctor< CMcpChannel > TEventCallback;
    TEventCallback callback( this, &CMcpChannel::OnEndpointNotification );
    SubscribeTo( &m_endpoint, CJsonRpcEndpoint::NotificationEvent, callback );
}

/*-------------------------------------------------------------------------*/

CMcpChannel::~CMcpChannel()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
    m_endpoint.SetRequestHandler( GUCEF_NULL );
    m_endpoint.SetTransport( GUCEF_NULL );
    GUCEF_DELETE m_legacySession;
    m_legacySession = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMcpChannel::EKind
CMcpChannel::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

const CMcpPrincipal&
CMcpChannel::GetPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpChannel::GetChannelId( void ) const
{GUCEF_TRACE;

    return m_channelId;
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::IsClosed( void ) const
{GUCEF_TRACE;

    return m_closed;
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::IsOpen( void ) const
{GUCEF_TRACE;

    return !m_closed && GUCEF_NULL != m_server;
}

/*-------------------------------------------------------------------------*/

CMcpLegacySession*
CMcpChannel::GetLegacySession( void )
{GUCEF_TRACE;

    return m_legacySession;
}

/*-------------------------------------------------------------------------*/

const CMcpLegacySession*
CMcpChannel::GetLegacySession( void ) const
{GUCEF_TRACE;

    return m_legacySession;
}

/*-------------------------------------------------------------------------*/

CMcpLegacySession&
CMcpChannel::OpenLegacySession( void )
{GUCEF_TRACE;

    GUCEF_DELETE m_legacySession;
    m_legacySession = GUCEF_NEW CMcpLegacySession();
    return *m_legacySession;
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint&
CMcpChannel::GetEndpoint( void )
{GUCEF_TRACE;

    return m_endpoint;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpChannel::GetMaxMessageBytes( void ) const
{GUCEF_TRACE;

    return m_maxMessageBytes;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::AttachToServer( CMcpServer* server, UInt64 channelId, UInt32 maxMessageBytes, UInt32 maxJsonDepth )
{GUCEF_TRACE;

    m_server = server;
    m_channelId = channelId;
    m_maxMessageBytes = maxMessageBytes;
    m_maxJsonDepth = maxJsonDepth;
    m_closed = false;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::DetachFromServer( void )
{GUCEF_TRACE;

    bool wasOpen = !m_closed;
    m_server = GUCEF_NULL;
    m_closed = true;
    if ( wasOpen )
        OnChannelClosed();
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpChannel::GetJsonNestingDepth( const char* text, UInt32 size, UInt32 maxDepth )
{GUCEF_TRACE;

    UInt32 depth = 0;
    UInt32 deepest = 0;
    bool inString = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = text[ i ];
        if ( inString )
        {
            if ( '\\' == c )
                ++i;                    // the escaped character cannot end the string
            else
            if ( '"' == c )
                inString = false;
            continue;
        }
        switch ( c )
        {
            case '"':
                inString = true;
                break;
            case '{':
            case '[':
                ++depth;
                if ( depth > deepest )
                {
                    deepest = depth;
                    if ( deepest > maxDepth )
                        return deepest;
                }
                break;
            case '}':
            case ']':
                if ( depth > 0 )
                    --depth;
                break;
            default:
                break;
        }
    }
    return deepest;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes are UTF-8 (RFC 3629): no overlong forms, no surrogates, nothing above U+10FFFF */
static bool
IsUtf8Bytes( const unsigned char* bytes, UInt32 size )
{GUCEF_TRACE;

    UInt32 i = 0;
    while ( i < size )
    {
        const unsigned char c = bytes[ i ];
        if ( c < 0x80 )
        {
            ++i;
            continue;
        }
        UInt32 length = 0;
        UInt32 codePoint = 0;
        if ( c >= 0xC2 && c <= 0xDF ) { length = 2; codePoint = c & 0x1F; }
        else
        if ( c >= 0xE0 && c <= 0xEF ) { length = 3; codePoint = c & 0x0F; }
        else
        if ( c >= 0xF0 && c <= 0xF4 ) { length = 4; codePoint = c & 0x07; }
        else
            return false;
        if ( i + length > size )
            return false;
        for ( UInt32 n=1; n<length; ++n )
        {
            if ( 0x80 != ( bytes[ i+n ] & 0xC0 ) )
                return false;
            codePoint = ( codePoint << 6 ) | ( bytes[ i+n ] & 0x3F );
        }
        if ( ( 3 == length && codePoint < 0x800 ) || ( 4 == length && ( codePoint < 0x10000 || codePoint > 0x10FFFF ) ) ||
             ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += length;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static int
JsonHexValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The four hexadecimal digits of a \u escape at text[ i ], -1 when they are not there */
static Int32
JsonEscapeCodeUnit( const char* text, UInt32 size, UInt32 i )
{GUCEF_TRACE;

    if ( i + 4 > size )
        return -1;
    Int32 value = 0;
    for ( UInt32 n=0; n<4; ++n )
    {
        const int digit = JsonHexValue( text[ i+n ] );
        if ( digit < 0 )
            return -1;
        value = value * 16 + digit;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

/** Appends a code point as UTF-8 */
static void
AppendUtf8( CORE::CString& out, UInt32 codePoint )
{GUCEF_TRACE;

    if ( codePoint < 0x80 )
        out += (char) codePoint;
    else
    if ( codePoint < 0x800 )
    {
        out += (char) ( 0xC0 | ( codePoint >> 6 ) );
        out += (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else
    if ( codePoint < 0x10000 )
    {
        out += (char) ( 0xE0 | ( codePoint >> 12 ) );
        out += (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        out += (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else
    {
        out += (char) ( 0xF0 | ( codePoint >> 18 ) );
        out += (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
        out += (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        out += (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
}

/*-------------------------------------------------------------------------*/

CMcpChannel::SJsonTextCheck::SJsonTextCheck( void )
    : problem( JSONTEXT_OK )
    , problemAtTopLevel( false )
    , detail()
    , idIsInt64Min( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

namespace {

/**
 *  A validator of JSON text as RFC 8259 defines it, which also gathers what the vendored parser loses: repeated member
 *  names, U+0000 and unpaired surrogates in strings, and the raw text of the id. json-parser is lenient: it takes
 *  trailing commas, comments, single quotes and more, so what it parses is no proof that the text is JSON. The
 *  recursion is bounded by the nesting depth, which is checked before.
 */
class CJsonTextValidator
{
    public:

    CJsonTextValidator( const char* text, UInt32 size, CMcpChannel::SJsonTextCheck& result )
        : m_text( text ), m_size( size ), m_pos( 0 ), m_result( result )
    {
    }

    /** False when the text is no JSON */
    bool Validate( void )
    {
        SkipWhitespace();
        if ( !Value( 0, false ) )
            return false;
        SkipWhitespace();
        return m_pos == m_size;
    }

    private:

    void SkipWhitespace( void )
    {
        while ( m_pos < m_size && ( ' ' == m_text[ m_pos ] || '\t' == m_text[ m_pos ] || '\n' == m_text[ m_pos ] || '\r' == m_text[ m_pos ] ) )
            ++m_pos;
    }

    void NoteProblem( CMcpChannel::EJsonTextProblem problem, bool topLevel, const CORE::CString& detail )
    {
        // the first problem found is the one reported
        if ( CMcpChannel::JSONTEXT_OK != m_result.problem )
            return;
        m_result.problem = problem;
        m_result.problemAtTopLevel = topLevel;
        m_result.detail = detail;
    }

    bool Value( UInt32 depth, bool isTopLevelId )
    {
        if ( m_pos >= m_size )
            return false;
        const char c = m_text[ m_pos ];
        if ( '{' == c )
            return Object( depth+1 );
        if ( '[' == c )
            return Array( depth+1 );
        if ( '"' == c )
            return String( GUCEF_NULL, depth <= 1 );
        if ( '-' == c || ( c >= '0' && c <= '9' ) )
        {
            const UInt32 start = m_pos;
            if ( !Number() )
                return false;
            if ( isTopLevelId )
                m_result.idIsInt64Min = CORE::CString( m_text + start, m_pos - start ) == "-9223372036854775808";
            return true;
        }
        return Literal( "true", 4 ) || Literal( "false", 5 ) || Literal( "null", 4 );
    }

    bool Literal( const char* literal, UInt32 length )
    {
        if ( m_pos + length > m_size || 0 != ::memcmp( m_text + m_pos, literal, length ) )
            return false;
        m_pos += length;
        return true;
    }

    /** -? ( 0 | [1-9][0-9]* ) ( . [0-9]+ )? ( [eE] [+-]? [0-9]+ )? */
    bool Number( void )
    {
        if ( '-' == m_text[ m_pos ] )
            ++m_pos;
        if ( m_pos >= m_size )
            return false;
        if ( '0' == m_text[ m_pos ] )
            ++m_pos;
        else
        if ( m_text[ m_pos ] >= '1' && m_text[ m_pos ] <= '9' )
        {
            while ( m_pos < m_size && m_text[ m_pos ] >= '0' && m_text[ m_pos ] <= '9' )
                ++m_pos;
        }
        else
            return false;
        if ( m_pos < m_size && '.' == m_text[ m_pos ] )
        {
            ++m_pos;
            if ( !Digits() )
                return false;
        }
        if ( m_pos < m_size && ( 'e' == m_text[ m_pos ] || 'E' == m_text[ m_pos ] ) )
        {
            ++m_pos;
            if ( m_pos < m_size && ( '+' == m_text[ m_pos ] || '-' == m_text[ m_pos ] ) )
                ++m_pos;
            if ( !Digits() )
                return false;
        }
        return true;
    }

    bool Digits( void )
    {
        const UInt32 start = m_pos;
        while ( m_pos < m_size && m_text[ m_pos ] >= '0' && m_text[ m_pos ] <= '9' )
            ++m_pos;
        return m_pos > start;
    }

    /** A string; its decoded text in 'decoded' when given (member names) */
    bool String( CORE::CString* decoded, bool topLevel )
    {
        ++m_pos;     // the opening quote
        while ( m_pos < m_size )
        {
            const unsigned char c = (unsigned char) m_text[ m_pos ];
            if ( '"' == c )
            {
                ++m_pos;
                return true;
            }
            if ( c < 0x20 )
                return false;           // a raw control character: escaped or no JSON
            if ( '\\' != c )
            {
                if ( GUCEF_NULL != decoded )
                    *decoded += (char) c;
                ++m_pos;
                continue;
            }
            if ( m_pos + 1 >= m_size )
                return false;
            const char escaped = m_text[ m_pos+1 ];
            if ( 'u' != escaped )
            {
                char plain = 0;
                switch ( escaped )
                {
                    case '"':  plain = '"'; break;
                    case '\\': plain = '\\'; break;
                    case '/':  plain = '/'; break;
                    case 'b':  plain = '\b'; break;
                    case 'f':  plain = '\f'; break;
                    case 'n':  plain = '\n'; break;
                    case 'r':  plain = '\r'; break;
                    case 't':  plain = '\t'; break;
                    default:   return false;
                }
                if ( GUCEF_NULL != decoded )
                    *decoded += plain;
                m_pos += 2;
                continue;
            }
            const Int32 unit = JsonEscapeCodeUnit( m_text, m_size, m_pos+2 );
            if ( unit < 0 )
                return false;
            m_pos += 6;
            UInt32 codePoint = (UInt32) unit;
            if ( 0 == unit )
                NoteProblem( CMcpChannel::JSONTEXT_NUL_CHARACTER, topLevel, CORE::CString::Empty );
            if ( unit >= 0xD800 && unit <= 0xDBFF )
            {
                const Int32 low = ( m_pos + 1 < m_size && '\\' == m_text[ m_pos ] && 'u' == m_text[ m_pos+1 ] ) ? JsonEscapeCodeUnit( m_text, m_size, m_pos+2 ) : -1;
                if ( low >= 0xDC00 && low <= 0xDFFF )
                {
                    m_pos += 6;
                    codePoint = 0x10000 + ( ( (UInt32) unit - 0xD800 ) << 10 ) + ( (UInt32) low - 0xDC00 );
                }
                else
                {
                    NoteProblem( CMcpChannel::JSONTEXT_UNPAIRED_SURROGATE, topLevel, CORE::CString::Empty );
                }
            }
            else
            if ( unit >= 0xDC00 && unit <= 0xDFFF )
            {
                NoteProblem( CMcpChannel::JSONTEXT_UNPAIRED_SURROGATE, topLevel, CORE::CString::Empty );
            }
            if ( GUCEF_NULL != decoded )
                AppendUtf8( *decoded, codePoint );
        }
        return false;      // unterminated
    }

    bool Object( UInt32 depth )
    {
        ++m_pos;     // '{'
        SkipWhitespace();
        if ( m_pos < m_size && '}' == m_text[ m_pos ] )
        {
            ++m_pos;
            return true;
        }
        GUCEF::set< CORE::CString > names;
        for ( ;; )
        {
            if ( m_pos >= m_size || '"' != m_text[ m_pos ] )
                return false;
            CORE::CString name;
            if ( !String( &name, 1 == depth ) )
                return false;
            if ( !names.insert( name ).second )
                NoteProblem( CMcpChannel::JSONTEXT_REPEATED_NAME, 1 == depth, name );
            SkipWhitespace();
            if ( m_pos >= m_size || ':' != m_text[ m_pos ] )
                return false;
            ++m_pos;
            SkipWhitespace();
            if ( !Value( depth, 1 == depth && "id" == name ) )
                return false;
            SkipWhitespace();
            if ( m_pos >= m_size )
                return false;
            if ( '}' == m_text[ m_pos ] )
            {
                ++m_pos;
                return true;
            }
            if ( ',' != m_text[ m_pos ] )
                return false;
            ++m_pos;
            SkipWhitespace();     // a '}' right after the comma is a trailing comma: refused by the name check above
        }
    }

    bool Array( UInt32 depth )
    {
        ++m_pos;     // '['
        SkipWhitespace();
        if ( m_pos < m_size && ']' == m_text[ m_pos ] )
        {
            ++m_pos;
            return true;
        }
        for ( ;; )
        {
            if ( !Value( depth, false ) )
                return false;
            SkipWhitespace();
            if ( m_pos >= m_size )
                return false;
            if ( ']' == m_text[ m_pos ] )
            {
                ++m_pos;
                return true;
            }
            if ( ',' != m_text[ m_pos ] )
                return false;
            ++m_pos;
            SkipWhitespace();
        }
    }

    const char* m_text;
    UInt32 m_size;
    UInt32 m_pos;
    CMcpChannel::SJsonTextCheck& m_result;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
CMcpChannel::CheckJsonText( const char* text, UInt32 size, UInt32 maxDepth, SJsonTextCheck& result )
{GUCEF_TRACE;

    result = SJsonTextCheck();
    if ( GUCEF_NULL == text || 0 == size )
        return;
    if ( GetJsonNestingDepth( text, size, maxDepth ) > maxDepth )
    {
        result.problem = JSONTEXT_TOO_DEEP;
        return;
    }
    if ( !IsUtf8Bytes( reinterpret_cast< const unsigned char* >( text ), size ) )
    {
        result.problem = JSONTEXT_NOT_UTF8;
        return;
    }
    CJsonTextValidator validator( text, size, result );
    if ( !validator.Validate() )
    {
        result = SJsonTextCheck();
        result.problem = JSONTEXT_NOT_JSON;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::ProcessIncomingMessage( const CORE::CString& messageText )
{GUCEF_TRACE;

    if ( m_closed )
        return false;

    const UInt32 size = messageText.ByteSize() > 0 ? messageText.ByteSize() - 1 : 0;
    if ( size > m_maxMessageBytes )
    {
        ProcessMessageTooLarge( size );
        return false;
    }
    SJsonTextCheck check;
    CheckJsonText( messageText.C_String(), size, m_maxJsonDepth, check );
    if ( JSONTEXT_TOO_DEEP == check.problem )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A message nested deeper than " << m_maxJsonDepth << " levels was refused" );
        SendError( CORE::CVariant(), MCP_ERROR_PARSE, "Parse error: nested deeper than " + CORE::ToString( m_maxJsonDepth ) + " levels" );
        return false;
    }
    if ( JSONTEXT_NOT_JSON == check.problem )
    {
        // json-parser takes trailing commas, comments and more: what is no JSON is refused before it
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A message that is no JSON text was refused" );
        SendError( CORE::CVariant(), MCP_ERROR_PARSE, "Parse error" );
        return false;
    }
    if ( JSONTEXT_NOT_UTF8 == check.problem )
    {
        // JSON text is UTF-8 (RFC 8259 8.1): what is not is no JSON, and is never echoed or handed on
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A message that is not UTF-8 was refused" );
        SendError( CORE::CVariant(), MCP_ERROR_PARSE, "Parse error: the message is not UTF-8" );
        return false;
    }

    // the message is parsed for its id, and the request is refused with it (OnJsonRpcRequest())
    m_incomingCheck = check;
    m_endpoint.ProcessIncoming( messageText );
    m_incomingCheck = SJsonTextCheck();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::ProcessMessageTooLarge( UInt32 messageBytes )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A message of " << messageBytes << " bytes was refused, the limit is " << m_maxMessageBytes );
    SendError( CORE::CVariant(), MCP_ERROR_INVALID_REQUEST, "Invalid request: message too large" );
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::SendResult( const CORE::CVariant& id, const CORE::CDataNode& result )
{GUCEF_TRACE;

    if ( m_closed )
        return false;
    const EOutgoingKind previousKind = m_outgoingKind;
    m_outgoingKind = OUTGOING_RESULT;
    const bool sent = m_endpoint.SendResponse( id, result );
    m_outgoingKind = previousKind;
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::SendError( const CORE::CVariant& id, Int32 code, const CORE::CString& message, const CORE::CDataNode* data )
{GUCEF_TRACE;

    if ( m_closed )
        return false;
    OnErrorCode( code );
    const EOutgoingKind previousKind = m_outgoingKind;
    m_outgoingKind = OUTGOING_ERROR;
    const bool sent = m_endpoint.SendErrorResponse( id, code, message, data );
    m_outgoingKind = previousKind;
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::SendNotification( const CORE::CString& method, const CORE::CDataNode* params )
{GUCEF_TRACE;

    if ( m_closed )
        return false;
    const EOutgoingKind previousKind = m_outgoingKind;
    m_outgoingKind = OUTGOING_NOTIFICATION;
    const bool sent = m_endpoint.Notify( method, params );
    m_outgoingKind = previousKind;
    return sent;
}

/*-------------------------------------------------------------------------*/

CMcpChannel::EOutgoingKind
CMcpChannel::GetOutgoingKind( void ) const
{GUCEF_TRACE;

    return m_outgoingKind;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::OnRequestReceived( const SJsonRpcRequest& request )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::OnErrorCode( Int32 code )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::ValidateRequestHeaders( const SJsonRpcRequest& request ,
                                     const CMcpRequestMeta* meta    ,
                                     EMcpProtocolEra era            ,
                                     Int32& errorCode               ,
                                     CORE::CString& errorMessage    )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::CompleteRequestPrincipal( CMcpPrincipal& principal )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::OnRequestCancelled( const CORE::CVariant& id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::OnChannelClosed( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpChannel::IsLegacyRequest( const SJsonRpcRequest& request ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_legacySession;
}

/*-------------------------------------------------------------------------*/

EJsonRpcRequestResult
CMcpChannel::OnJsonRpcRequest( CJsonRpcEndpoint& endpoint    ,
                               const SJsonRpcRequest& request ,
                               SJsonRpcResponse& response     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server || m_closed )
    {
        response.isError = true;
        response.error.code = MCP_ERROR_INTERNAL;
        response.error.message = "Internal error: the channel is not open";
        return JSONRPC_REQUEST_HANDLED;
    }
    // the server answers every request itself, through this channel
    OnRequestReceived( request );

    // what the parser cannot carry faithfully is refused with the id of the request: a member name that occurs twice
    // (which of the two would be judged and which used?), U+0000 (the parser cuts a string there), an unpaired surrogate
    if ( JSONTEXT_OK != m_incomingCheck.problem )
    {
        const bool envelope = m_incomingCheck.problemAtTopLevel;
        const Int32 code = envelope ? (Int32) MCP_ERROR_INVALID_REQUEST : (Int32) MCP_ERROR_INVALID_PARAMS;
        CORE::CString message = envelope ? "Invalid request: " : "Invalid params: ";
        if ( JSONTEXT_REPEATED_NAME == m_incomingCheck.problem )
        {
            // a long name is not repeated back: cutting it could split a character
            const CORE::CString& name = m_incomingCheck.detail;
            if ( name.ByteSize() <= 65 )
                message += "the member name \"" + CORE::CRedactor::ForLogLine().RedactText( name ) + "\" occurs more than once in an object";
            else
                message += "a member name occurs more than once in an object";
        }
        else
        if ( JSONTEXT_NUL_CHARACTER == m_incomingCheck.problem )
            message += "strings with the character U+0000 are not supported";
        else
            message += "a string holds an unpaired surrogate, which is no Unicode text";
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A request was refused: " << message );
        SendError( request.id, code, message );
        return JSONRPC_REQUEST_DEFERRED;
    }

    // the smallest Int64 reaches us as a double: its text said it is that integer
    if ( m_incomingCheck.idIsInt64Min && request.id.IsFloat() && -9223372036854775808.0 == request.id.AsFloat64() )
    {
        SJsonRpcRequest corrected( request );
        corrected.id = CORE::CVariant( (Int64) ( -9223372036854775807LL - 1 ) );
        m_server->OnChannelRequest( *this, corrected );
        return JSONRPC_REQUEST_DEFERRED;
    }

    m_server->OnChannelRequest( *this, request );
    return JSONRPC_REQUEST_DEFERRED;
}

/*-------------------------------------------------------------------------*/

void
CMcpChannel::OnEndpointNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const TJsonRpcNotificationEventData* data = static_cast< const TJsonRpcNotificationEventData* >( eventData );
    if ( GUCEF_NULL == data || GUCEF_NULL == m_server || m_closed )
        return;
    if ( JSONTEXT_OK != m_incomingCheck.problem )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpChannel(" << m_channelId << "): A notification that cannot be carried faithfully was dropped" );
        return;
    }
    m_server->OnChannelNotification( *this, data->GetData() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
