/*
 *  gucefCOM: GUCEF module providing communication functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CJsonRpcEndpoint::ResponseEvent = "GUCEF::COM::CJsonRpcEndpoint::ResponseEvent";
const CORE::CEvent CJsonRpcEndpoint::NotificationEvent = "GUCEF::COM::CJsonRpcEndpoint::NotificationEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{GUCEF_TRACE;

    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL() )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: The json dstore codec is not available, is dstorepluginJSONPARSER loaded?" );
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  True when the JSON text has an integer outside the 64 bit range (no fraction, no exponent) after a number with a
 *  fraction, outside of strings. The json parser behind the json codec reads integers into 64 bits and builds a double
 *  of one that is larger. It remembers having seen a fraction for the rest of the text, so such an integer comes out
 *  ten times too small. Without a fraction before it the double is right, so it is only refused when it would be wrong
 */
static bool
HasMisreadableIntegerBeyond64Bit( const CORE::CString& text )
{GUCEF_TRACE;

    const char* data = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    bool inString = false;
    bool seenFraction = false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = data[ i ];
        if ( inString )
        {
            if ( '\\' == c )
                ++i;                    // the escaped character, which can be a quote
            else
            if ( '"' == c )
                inString = false;
            continue;
        }
        if ( '"' == c )
        {
            inString = true;
            continue;
        }
        if ( '-' != c && ( c < '0' || c > '9' ) )
            continue;

        // a number: the characters that can make up one
        UInt32 end = i;
        bool isInteger = true;
        bool hasFraction = false;
        while ( end < size && ( ( data[ end ] >= '0' && data[ end ] <= '9' ) || '-' == data[ end ] || '+' == data[ end ] ||
                                '.' == data[ end ] || 'e' == data[ end ] || 'E' == data[ end ] ) )
        {
            if ( '.' == data[ end ] || 'e' == data[ end ] || 'E' == data[ end ] )
                isInteger = false;
            if ( '.' == data[ end ] )
                hasFraction = true;
            ++end;
        }
        if ( isInteger && seenFraction )
        {
            const bool negative = '-' == c;
            UInt32 digitsStart = i + ( negative ? 1 : 0 );
            while ( digitsStart + 1 < end && '0' == data[ digitsStart ] )
                ++digitsStart;          // leading zeros are not valid JSON, they do not make a number larger
            const UInt32 digits = end - digitsStart;
            if ( digits > 19 )
                return true;
            if ( 19 == digits )
            {
                const char* limit = negative ? "9223372036854775808" : "9223372036854775807";
                if ( 0 < memcmp( data + digitsStart, limit, 19 ) )
                    return true;
            }
        }
        if ( hasFraction )
            seenFraction = true;
        i = end - 1;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
ParseJson( const CORE::CString& text, CORE::CDataNode& root )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( text.ByteSize() < 2 || !GetJsonCodec( codec ) )
        return false;
    if ( HasMisreadableIntegerBeyond64Bit( text ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: The JSON text has an integer outside the 64 bit range after a number with a fraction, which the JSON parser cannot read correctly" );
        return false;
    }

    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );    // ByteSize includes the null terminator
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &root, &access );
}

/*-------------------------------------------------------------------------*/

static bool
IsNullType( Int32 nodeType )
{
    return GUCEF_DATATYPE_NIL == nodeType || GUCEF_DATATYPE_NULL == nodeType;
}

/**
 *  Writes a node tree as JSON text following the same conventions the json codec uses when reading:
 *  an object has its scalar members as attributes and its other members as children, an array has its items as
 *  children and a node of the nil type is null.
 *  The codec is only used for reading: its writer turns an object that has exactly one member and nothing else
 *  into a bare value, which would break messages such as { "url": "http://example.org/" }
 */
static void
Put( CORE::CDynamicBuffer& out, char c )
{
    out.Append( &c, 1 );
}

static void
Put( CORE::CDynamicBuffer& out, const char* text )
{
    out.Append( text, (UInt32) ::strlen( text ) );
}

/**
 *  The length of the well-formed UTF-8 sequence that starts at 'data' for a lead byte of 0x80 or more, or 0 when it is ill-formed
 *  (a stray continuation byte, an overlong form, a surrogate, a code point above U+10FFFF, a truncated sequence).
 *  'illFormedLength' is then the length of the maximal ill-formed subpart, the number of bytes to replace by one U+FFFD
 */
static UInt32
Utf8SequenceLength( const unsigned char* data, UInt32 available, UInt32& illFormedLength )
{
    UInt32 continuationBytes = 0;
    unsigned char lower = 0x80;
    unsigned char upper = 0xBF;
    const unsigned char lead = data[ 0 ];
    if ( lead >= 0xC2 && lead <= 0xDF )
    {
        continuationBytes = 1;
    }
    else
    if ( lead >= 0xE0 && lead <= 0xEF )
    {
        continuationBytes = 2;
        if ( 0xE0 == lead ) lower = 0xA0;           // not overlong
        if ( 0xED == lead ) upper = 0x9F;           // not a surrogate
    }
    else
    if ( lead >= 0xF0 && lead <= 0xF4 )
    {
        continuationBytes = 3;
        if ( 0xF0 == lead ) lower = 0x90;           // not overlong
        if ( 0xF4 == lead ) upper = 0x8F;           // not above U+10FFFF
    }
    else
    {
        illFormedLength = 1;                        // a continuation byte without a lead, or a byte that is never used
        return 0;
    }

    for ( UInt32 i=1; i<=continuationBytes; ++i )
    {
        if ( i >= available || data[ i ] < lower || data[ i ] > upper )
        {
            illFormedLength = i;
            return 0;
        }
        lower = 0x80;
        upper = 0xBF;
    }
    return continuationBytes + 1;
}

/**
 *  Writes a string as a JSON string. JSON text has to be valid UTF-8: every ill-formed sequence in the string is
 *  written as one U+FFFD replacement character
 */
static void
AppendJsonString( CORE::CDynamicBuffer& out, const CORE::CString& str )
{
    static const char* hexDigits = "0123456789abcdef";
    Put( out, '"' );
    const char* data = str.C_String();
    UInt32 size = str.ByteSize() > 0 ? str.ByteSize() - 1 : 0;      // ByteSize includes the null terminator
    for ( UInt32 i=0; i<size; ++i )
    {
        unsigned char c = (unsigned char) data[ i ];
        if ( c >= 0x80 )
        {
            UInt32 illFormedLength = 0;
            UInt32 length = Utf8SequenceLength( (const unsigned char*) data + i, size - i, illFormedLength );
            if ( 0 == length )
            {
                Put( out, "\xEF\xBF\xBD" );              // U+FFFD
                i += illFormedLength - 1;
            }
            else
            {
                out.Append( data + i, length );
                i += length - 1;
            }
            continue;
        }
        switch ( c )
        {
            case '"':  Put( out, "\\\"" ); break;
            case '\\': Put( out, "\\\\" ); break;
            case '\b': Put( out, "\\b" ); break;
            case '\f': Put( out, "\\f" ); break;
            case '\n': Put( out, "\\n" ); break;
            case '\r': Put( out, "\\r" ); break;
            case '\t': Put( out, "\\t" ); break;
            default:
            {
                if ( c < 0x20 )
                {
                    Put( out, "\\u00" );
                    Put( out, hexDigits[ c >> 4 ] );
                    Put( out, hexDigits[ c & 0xF ] );
                }
                else
                {
                    Put( out, (char) c );
                }
            }
        }
    }
    Put( out, '"' );
}

static void
AppendJsonScalar( CORE::CDynamicBuffer& out, const CORE::CVariant& value )
{
    if ( !value.IsInitialized() || value.IsNIL() || value.IsNULL() )
    {
        Put( out, "null" );
    }
    else
    if ( value.IsBoolean() )
    {
        Put( out, value.AsBool() ? "true" : "false" );
    }
    else
    if ( value.IsUnsignedInteger() )
    {
        Put( out, CORE::UInt64ToString( value.AsUInt64() ).C_String() );
    }
    else
    if ( value.IsInteger() )
    {
        Put( out, CORE::Int64ToString( value.AsInt64() ).C_String() );
    }
    else
    if ( value.IsFloat() )
    {
        Float64 number = value.AsFloat64();
        if ( number != number || number > 1.7976931348623157e308 || number < -1.7976931348623157e308 )
        {
            Put( out, "null" );      // JSON has no representation for NaN or infinity
        }
        else
        {
            // the shortest text that reads back as the same double, with a decimal point whatever the locale of the C library is
            Put( out, CORE::Float64ToRoundTripString( number ).C_String() );
        }
    }
    else
    {
        AppendJsonString( out, value.AsString() );
    }
}

static void AppendJsonNode( CORE::CDynamicBuffer& out, const CORE::CDataNode& node );

static void
AppendJsonNode( CORE::CDynamicBuffer& out, const CORE::CDataNode& node )
{
    Int32 nodeType = node.GetNodeType();
    if ( GUCEF_DATATYPE_ARRAY == nodeType )
    {
        Put( out, '[' );
        bool first = true;
        for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        {
            if ( !first )
                Put( out, ',' );
            first = false;
            AppendJsonNode( out, *(*i) );
        }
        Put( out, ']' );
    }
    else
    if ( GUCEF_DATATYPE_OBJECT == nodeType && ( node.GetAttCount() > 0 || node.HasChildren() || !node.HasValue() ) )
    {
        Put( out, '{' );
        bool first = true;
        for ( UInt32 i=0; i<node.GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( i );
            if ( !first )
                Put( out, ',' );
            first = false;
            AppendJsonString( out, (*attribute).first );
            Put( out, ':' );
            AppendJsonScalar( out, (*attribute).second );
        }
        for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        {
            if ( !first )
                Put( out, ',' );
            first = false;
            AppendJsonString( out, (*i)->GetName() );
            Put( out, ':' );
            AppendJsonNode( out, *(*i) );
        }
        Put( out, '}' );
    }
    else
    if ( IsNullType( nodeType ) )
    {
        Put( out, "null" );
    }
    else
    {
        AppendJsonScalar( out, node.GetValue() );
    }
}

static bool
StoreJson( const CORE::CDataNode& root, CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDynamicBuffer json;
    AppendJsonNode( json, root );
    Put( json, '\0' );
    text = CORE::CString( static_cast< const char* >( json.GetConstBufferPtr() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
IsContainerType( Int32 nodeType )
{
    return GUCEF_DATATYPE_ARRAY == nodeType || GUCEF_DATATYPE_OBJECT == nodeType;
}

/*-------------------------------------------------------------------------*/


/**
 *  Adds a member to a JSON object. Per the JSON codec scalars become attributes of the object,
 *  objects and arrays child nodes and null a child of the nil type
 */
static void
AddMember( CORE::CDataNode& parent, const CORE::CString& name, const CORE::CDataNode& value )
{GUCEF_TRACE;

    Int32 nodeType = value.GetNodeType();
    if ( IsContainerType( nodeType ) )
    {
        CORE::CDataNode copy( value );
        copy.SetName( name );
        parent.AddChild( copy );
    }
    else
    if ( IsNullType( nodeType ) )
    {
        parent.AddChild( name, GUCEF_DATATYPE_NIL );
    }
    else
    {
        parent.SetAttribute( name, value.GetValue() );
    }
}

/*-------------------------------------------------------------------------*/

static bool
HasMember( const CORE::CDataNode& object, const CORE::CString& name )
{GUCEF_TRACE;

    // FindChild(): CDataNode::Find() also searches the siblings and every descendant of the node
    return GUCEF_NULL != object.GetAttribute( name ) || GUCEF_NULL != object.FindChild( name );
}

/**
 *  The reverse of AddMember: gets a member of a JSON object as a node. For a scalar the value is in the node
 */
static bool
GetMember( const CORE::CDataNode& object, const CORE::CString& name, CORE::CDataNode& value )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        value = CORE::CDataNode();
        value.SetValue( (*attribute).second );
        value.SetNodeType( (*attribute).second.GetTypeId() );
        return true;
    }
    const CORE::CDataNode* child = object.FindChild( name );
    if ( GUCEF_NULL != child )
    {
        value = *child;
        value.SetName( CORE::CString::Empty );
        return true;
    }
    return false;
}

static bool
IsNullId( const CORE::CVariant& id )
{GUCEF_TRACE;

    return !id.IsInitialized() || id.IsNULL() || id.IsNIL();
}

static void
AddId( CORE::CDataNode& message, const CORE::CVariant& id )
{GUCEF_TRACE;

    if ( IsNullId( id ) )
        message.AddChild( "id", GUCEF_DATATYPE_NIL );
    else
        message.SetAttribute( "id", id );
}

/**
 *  The id of a request has to be a string, a number or null (or absent for a notification). The codec reads a boolean
 *  as an attribute and an object or array as a child node: none of those is an id
 */
static bool
HasValidId( const CORE::CDataNode& message )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = message.GetAttribute( "id" );
    if ( GUCEF_NULL != attribute )
    {
        const CORE::CVariant& id = (*attribute).second;
        if ( IsNullId( id ) )
            return true;
        if ( id.IsBoolean() )
            return false;
        return id.IsString() || id.IsInteger() || id.IsFloat();
    }
    const CORE::CDataNode* child = message.FindChild( "id" );
    if ( GUCEF_NULL != child )
        return IsNullType( child->GetNodeType() );
    return true;
}

static CORE::CVariant
GetId( const CORE::CDataNode& message )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = message.GetAttribute( "id" );
    if ( GUCEF_NULL != attribute )
        return (*attribute).second;
    return CORE::CVariant();    // absent or null
}

static CORE::CString
CallKey( const CORE::CVariant& id )
{
    return id.AsString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::ParseJsonText( const CORE::CString& jsonText, CORE::CDataNode& root )
{GUCEF_TRACE;

    root.Clear();
    return ParseJson( jsonText, root );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::ToJsonText( const CORE::CDataNode& root, CORE::CString& jsonText )
{GUCEF_TRACE;

    return StoreJson( root, jsonText );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::HasJsonMember( const CORE::CDataNode& object, const CORE::CString& name )
{GUCEF_TRACE;

    return HasMember( object, name );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CJsonRpcEndpoint::FindJsonMemberNode( const CORE::CDataNode& object, const CORE::CString& name )
{GUCEF_TRACE;

    return object.FindChild( name );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::GetJsonMember( const CORE::CDataNode& object, const CORE::CString& name, CORE::CDataNode& value )
{GUCEF_TRACE;

    return GetMember( object, name, value );
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::AddJsonMember( CORE::CDataNode& object, const CORE::CString& name, const CORE::CDataNode& value )
{GUCEF_TRACE;

    AddMember( object, name, value );
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::RegisterEvents( void )
{GUCEF_TRACE;

    ResponseEvent.Initialize();
    NotificationEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint::CJsonRpcEndpoint( CIJsonRpcTransport* transport )
    : CORE::CNotifier()
    , m_transport( transport )
    , m_requestHandler( GUCEF_NULL )
    , m_dialect( JSONRPC_DIALECT_STRICT_2_0 )
    , m_allowBatches( true )
    , m_allowNullRequestId( true )
    , m_defaultCallTimeoutInMs( DefaultCallTimeoutInMs )
    , m_nextCallId( 1 )
    , m_pendingCalls()
    , m_outgoingCallFields()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_isWaiting( false )
    , m_waitDestroyedFlag( GUCEF_NULL )
    , m_waitDone( false )
    , m_waitKey()
    , m_waitResponse()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint::~CJsonRpcEndpoint()
{GUCEF_TRACE;

    // tells the CallAndWait() that is in progress on this endpoint, if any, that there is nothing left to wait on
    if ( GUCEF_NULL != m_waitDestroyedFlag )
        *m_waitDestroyedFlag = true;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetTransport( CIJsonRpcTransport* transport )
{GUCEF_TRACE;

    m_transport = transport;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetDialect( EJsonRpcDialect dialect )
{GUCEF_TRACE;

    m_dialect = dialect;
}

/*-------------------------------------------------------------------------*/

EJsonRpcDialect
CJsonRpcEndpoint::GetDialect( void ) const
{GUCEF_TRACE;

    return m_dialect;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetAllowBatches( bool allowBatches )
{GUCEF_TRACE;

    m_allowBatches = allowBatches;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::GetAllowBatches( void ) const
{GUCEF_TRACE;

    return m_allowBatches;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetAllowNullRequestId( bool allowNullRequestId )
{GUCEF_TRACE;

    m_allowNullRequestId = allowNullRequestId;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::GetAllowNullRequestId( void ) const
{GUCEF_TRACE;

    return m_allowNullRequestId;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetRequestHandler( CIJsonRpcRequestHandler* handler )
{GUCEF_TRACE;

    m_requestHandler = handler;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetOutgoingCallField( const CORE::CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        m_outgoingCallFields.clear();
    else
        m_outgoingCallFields[ name ] = value;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetDefaultCallTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_defaultCallTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CJsonRpcEndpoint::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

UInt32
CJsonRpcEndpoint::GetPendingCallCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_pendingCalls.size();
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::InitMessage( CORE::CDataNode& message ) const
{GUCEF_TRACE;

    message = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    if ( JSONRPC_DIALECT_STRICT_2_0 == m_dialect )
        message.SetAttribute( "jsonrpc", "2.0" );
}

/*-------------------------------------------------------------------------*/

void
CJsonRpcEndpoint::AddCallFields( CORE::CDataNode& message ) const
{GUCEF_TRACE;

    for ( GUCEF::map< CORE::CString, CORE::CVariant >::const_iterator i = m_outgoingCallFields.begin(); i != m_outgoingCallFields.end(); ++i )
        message.SetAttribute( (*i).first, (*i).second );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::SendNode( const CORE::CDataNode& message )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_transport )
        return false;

    CORE::CString text;
    if ( !StoreJson( message, text ) )
        return false;
    return m_transport->SendMessageText( text );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::BuildCall( const CORE::CString& method       ,
                             const CORE::CDataNode* params     ,
                             bool isNotification               ,
                             UInt64& callId                    ,
                             UInt32 timeoutInMs                ,
                             CORE::CDataNode& message          )
{GUCEF_TRACE;

    if ( method.IsNULLOrEmpty() )
        return false;
    if ( GUCEF_NULL != params && !IsContainerType( params->GetNodeType() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: The parameters of a call must be an array or an object" );
        return false;
    }

    InitMessage( message );
    message.SetAttribute( "method", method );
    if ( !isNotification )
    {
        callId = m_nextCallId++;
        message.SetAttribute( "id", callId );

        SPendingCall pending;
        pending.callId = CORE::CVariant( callId );
        pending.deadlineInMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() + ( 0 == timeoutInMs ? m_defaultCallTimeoutInMs : timeoutInMs );
        m_pendingCalls[ CallKey( pending.callId ) ] = pending;
    }
    if ( GUCEF_NULL != params )
        AddMember( message, "params", *params );
    AddCallFields( message );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::Call( const CORE::CString& method       ,
                        const CORE::CDataNode* params     ,
                        UInt64& callId                    ,
                        UInt32 timeoutInMs                )
{GUCEF_TRACE;

    CORE::CDataNode message;
    callId = 0;

    // The call is registered before it is sent as an answer can arrive before the send even returns
    if ( !BuildCall( method, params, false, callId, timeoutInMs, message ) )
        return false;

    if ( !SendNode( message ) )
    {
        m_pendingCalls.erase( CallKey( CORE::CVariant( callId ) ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::Notify( const CORE::CString& method, const CORE::CDataNode* params )
{GUCEF_TRACE;

    CORE::CDataNode message;
    UInt64 unused = 0;
    if ( !BuildCall( method, params, true, unused, 0, message ) )
        return false;
    return SendNode( message );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::CallBatch( TBatchItemVector& items, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( items.empty() )
        return false;

    CORE::CDataNode batch( GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<items.size(); ++i )
    {
        CORE::CDataNode message;
        if ( !BuildCall( items[ i ].method, items[ i ].hasParams ? &items[ i ].params : GUCEF_NULL, items[ i ].isNotification, items[ i ].callId, timeoutInMs, message ) )
        {
            // forget the calls of this batch which were already registered
            for ( size_t n=0; n<i; ++n )
                if ( !items[ n ].isNotification )
                    m_pendingCalls.erase( CallKey( CORE::CVariant( items[ n ].callId ) ) );
            return false;
        }
        batch.AddChild( message );
    }

    if ( !SendNode( batch ) )
    {
        for ( size_t n=0; n<items.size(); ++n )
            if ( !items[ n ].isNotification )
                m_pendingCalls.erase( CallKey( CORE::CVariant( items[ n ].callId ) ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The state of a CallAndWait() in progress: set up when the wait starts, undone when it ends unless the endpoint
 *  was deleted in the meantime in which case there is nothing left to undo
 */
class CJsonRpcEndpoint::CWaitScope
{
    public:

    explicit CWaitScope( CJsonRpcEndpoint& endpoint )
        : m_endpoint( &endpoint )
        , m_endpointDestroyed( false )
    {
        endpoint.m_isWaiting = true;
        endpoint.m_waitDone = false;
        endpoint.m_waitDestroyedFlag = &m_endpointDestroyed;
    }

    ~CWaitScope()
    {
        if ( !m_endpointDestroyed )
        {
            m_endpoint->m_isWaiting = false;
            m_endpoint->m_waitDestroyedFlag = GUCEF_NULL;
        }
    }

    bool IsEndpointDestroyed( void ) const { return m_endpointDestroyed; }

    private:

    CWaitScope( const CWaitScope& src );
    CWaitScope& operator=( const CWaitScope& src );

    CJsonRpcEndpoint* m_endpoint;
    bool m_endpointDestroyed;
};

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::CallAndWait( const CORE::CString& method   ,
                               const CORE::CDataNode* params ,
                               SJsonRpcResponse& response    ,
                               UInt32 timeoutInMs            )
{GUCEF_TRACE;

    // The wait runs on the calling thread and pumps the pulse generator of the transport, which only the
    // thread that drives it may do. Refuse before anything is sent so a refused call leaves no trace.
    CORE::PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    if ( pulseGenerator.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: CallAndWait() refused for method \"" << method << "\": the endpoint has no pulse generator to wait with, see SetPulseGenerator()" );
        return false;
    }
    UInt32 callingThreadId = MT::GetCurrentTaskID();
    UInt32 drivingThreadId = pulseGenerator->GetPulseDriverThreadId();
    if ( callingThreadId != drivingThreadId )
    {
        if ( 0 == drivingThreadId )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: CallAndWait() refused for method \"" << method << "\" on thread " << callingThreadId <<
                ": it is not known which thread drives the pulse generator of the endpoint. Give the pulse generator a driver or a driver thread id, or use Call()" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: CallAndWait() refused for method \"" << method << "\" on thread " << callingThreadId <<
                ": the pulse generator of the endpoint is driven by thread " << drivingThreadId << ". Wait on that thread or use Call()" );
        }
        return false;
    }

    if ( m_isWaiting )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: CallAndWait() is not supported while another CallAndWait() is in progress" );
        return false;
    }

    // Whatever is pumped can delete this endpoint, for example the owner of a transport that drops a failed connection.
    // The scope sees to it that no member is touched after that, the state is reset at the end of the wait otherwise
    CWaitScope scope( *this );

    UInt64 callId = 0;
    m_waitKey = CallKey( CORE::CVariant( (UInt64) m_nextCallId ) );     // the id the call is about to get
    if ( !Call( method, params, callId, timeoutInMs ) )
        return false;

    // Time passes through the pulse generator which also drives the transport
    while ( !scope.IsEndpointDestroyed() && !m_waitDone )
    {
        pulseGenerator->ForceDirectPulse();
        if ( scope.IsEndpointDestroyed() )
            return false;       // we were deleted by a pulse handler
        if ( !ExpireTimedOutCalls() )
            return false;       // we were deleted by a response handler
        if ( !m_waitDone )
            MT::ThreadDelay( 1 );
    }
    if ( scope.IsEndpointDestroyed() )
        return false;

    response = m_waitResponse;
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::DeliverResponse( SJsonRpcResponse& response )
{GUCEF_TRACE;

    if ( m_isWaiting && !m_waitDone && CallKey( response.id ) == m_waitKey )
    {
        m_waitResponse = response;
        m_waitDone = true;
        return true;
    }

    TJsonRpcResponseEventData eData( response );
    return NotifyObservers( ResponseEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::ExpireTimedOutCalls( UInt32* expiredCount )
{GUCEF_TRACE;

    if ( GUCEF_NULL != expiredCount )
        *expiredCount = 0;
    if ( m_pendingCalls.empty() )
        return true;

    UInt64 now = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    GUCEF::vector< CORE::CVariant > expired;
    for ( TPendingCallMap::iterator i = m_pendingCalls.begin(); i != m_pendingCalls.end(); ++i )
        if ( (*i).second.deadlineInMs <= now )
            expired.push_back( (*i).second.callId );

    for ( size_t n=0; n<expired.size(); ++n )
    {
        m_pendingCalls.erase( CallKey( expired[ n ] ) );

        SJsonRpcResponse response;
        response.id = expired[ n ];
        response.isError = true;
        response.isLocalError = true;
        response.error.code = JSONRPC_ERROR_CALL_TIMEOUT;
        response.error.message = "The call was not answered in time";
        if ( GUCEF_NULL != expiredCount )
            ++(*expiredCount);
        if ( !DeliverResponse( response ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::FailAllPendingCalls( Int32 errorCode, const CORE::CString& errorMessage )
{GUCEF_TRACE;

    TPendingCallMap pending;
    pending.swap( m_pendingCalls );

    for ( TPendingCallMap::iterator i = pending.begin(); i != pending.end(); ++i )
    {
        SJsonRpcResponse response;
        response.id = (*i).second.callId;
        response.isError = true;
        response.isLocalError = true;
        response.error.code = errorCode;
        response.error.message = errorMessage;
        if ( !DeliverResponse( response ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::SendResponse( const CORE::CVariant& callId, const CORE::CDataNode& result )
{GUCEF_TRACE;

    CORE::CDataNode message;
    InitMessage( message );
    AddId( message, callId );
    AddMember( message, "result", result );
    return SendNode( message );
}

/*-------------------------------------------------------------------------*/

bool
CJsonRpcEndpoint::SendErrorResponse( const CORE::CVariant& callId       ,
                                     Int32 errorCode                    ,
                                     const CORE::CString& errorMessage  ,
                                     const CORE::CDataNode* errorData   )
{GUCEF_TRACE;

    CORE::CDataNode message;
    InitMessage( message );
    AddId( message, callId );
    CORE::CDataNode* error = message.AddChild( "error", GUCEF_DATATYPE_OBJECT );
    error->SetAttribute( "code", errorCode );
    error->SetAttribute( "message", errorMessage );
    if ( GUCEF_NULL != errorData )
        AddMember( *error, "data", *errorData );
    return SendNode( message );
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint::EProcessResult
CJsonRpcEndpoint::ProcessIncoming( const CORE::CString& messageText )
{GUCEF_TRACE;

    CORE::CDataNode root;
    if ( !ParseJson( messageText, root ) )
    {
        // the id of the message is not known: null
        SendErrorResponse( CORE::CVariant(), JSONRPC_ERROR_PARSE, "Parse error" );
        return PROCESS_INVALID;
    }

    if ( GUCEF_DATATYPE_ARRAY != root.GetNodeType() )
        return ProcessMessageNode( root, GUCEF_NULL );

    if ( !m_allowBatches )
    {
        SendErrorResponse( CORE::CVariant(), JSONRPC_ERROR_INVALID_REQUEST, "Invalid request: batches are not supported" );
        return PROCESS_INVALID;
    }

    // A batch: the responses to its calls are sent together as one batch
    if ( 0 == root.GetNrOfChildNodes() )
    {
        SendErrorResponse( CORE::CVariant(), JSONRPC_ERROR_INVALID_REQUEST, "Invalid request: empty batch" );
        return PROCESS_INVALID;
    }

    CORE::CDataNode batchResponses( GUCEF_DATATYPE_ARRAY );
    EProcessResult overall = PROCESS_OK;
    for ( CORE::CDataNode::const_iterator i = root.ConstBegin(); i != root.ConstEnd(); ++i )
    {
        EProcessResult result = ProcessMessageNode( *(*i), &batchResponses );
        if ( PROCESS_DELETED == result )
            return PROCESS_DELETED;
        if ( PROCESS_INVALID == result )
            overall = PROCESS_INVALID;
    }
    if ( batchResponses.HasChildren() )
        SendNode( batchResponses );
    return overall;
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint::EProcessResult
CJsonRpcEndpoint::ProcessMessageNode( const CORE::CDataNode& node, CORE::CDataNode* batchResponses )
{GUCEF_TRACE;

    // Responses we generate go into the batch if there is one, otherwise they are sent right away
    CORE::CDataNode response;
    bool haveResponse = false;
    EProcessResult result = PROCESS_OK;

    InitMessage( response );

    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
    {
        AddId( response, CORE::CVariant() );
        CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
        error->SetAttribute( "code", (Int32) JSONRPC_ERROR_INVALID_REQUEST );
        error->SetAttribute( "message", "Invalid request: a message must be an object" );
        haveResponse = true;
        result = PROCESS_INVALID;
    }
    else
    {
        const CORE::CDataNode::TKeyValuePair* version = node.GetAttribute( "jsonrpc" );
        bool versionOk = JSONRPC_DIALECT_LENIENT == m_dialect || ( GUCEF_NULL != version && (*version).second.AsString() == "2.0" );
        bool hasMethod = GUCEF_NULL != node.GetAttribute( "method" ) && node.GetAttribute( "method" )->second.IsString();
        bool hasId = HasMember( node, "id" );

        const bool hasValidId = HasValidId( node );

        if ( hasMethod && versionOk && hasId && !hasValidId )
        {
            // the handler is not run for a request that cannot be identified, the peer cannot be told which request it was
            AddId( response, CORE::CVariant() );
            CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
            error->SetAttribute( "code", (Int32) JSONRPC_ERROR_INVALID_REQUEST );
            error->SetAttribute( "message", "Invalid request: the id must be a string, a number or null" );
            haveResponse = true;
            result = PROCESS_INVALID;
        }
        else
        if ( hasMethod && versionOk && hasId && !m_allowNullRequestId && IsNullId( GetId( node ) ) )
        {
            AddId( response, CORE::CVariant() );
            CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
            error->SetAttribute( "code", (Int32) JSONRPC_ERROR_INVALID_REQUEST );
            error->SetAttribute( "message", "Invalid request: the id must not be null" );
            haveResponse = true;
            result = PROCESS_INVALID;
        }
        else
        if ( hasMethod && versionOk )
        {
            CORE::CString method = node.GetAttribute( "method" )->second.AsString();

            CORE::CDataNode params;
            bool hasParams = GetMember( node, "params", params );
            if ( hasParams && !IsContainerType( params.GetNodeType() ) )
            {
                // the parameters have to be structured
                if ( hasId )
                {
                    AddId( response, GetId( node ) );
                    CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
                    error->SetAttribute( "code", (Int32) JSONRPC_ERROR_INVALID_PARAMS );
                    error->SetAttribute( "message", "Invalid params: must be an array or an object" );
                    haveResponse = true;
                }
                result = PROCESS_INVALID;
            }
            else
            if ( !hasId )
            {
                SJsonRpcNotification notification;
                notification.method = method;
                notification.params = params;
                notification.hasParams = hasParams;
                notification.envelope = node;
                TJsonRpcNotificationEventData eData( notification );
                if ( !NotifyObservers( NotificationEvent, &eData ) )
                    return PROCESS_DELETED;
            }
            else
            {
                SJsonRpcRequest request;
                request.id = GetId( node );
                request.method = method;
                request.params = params;
                request.hasParams = hasParams;
                request.envelope = node;

                SJsonRpcResponse handlerResponse;
                EJsonRpcRequestResult handled = JSONRPC_REQUEST_HANDLED;
                if ( GUCEF_NULL == m_requestHandler )
                {
                    handlerResponse.isError = true;
                    handlerResponse.error.code = JSONRPC_ERROR_METHOD_NOT_FOUND;
                    handlerResponse.error.message = "Method not found";
                }
                else
                {
                    try
                    {
                        handled = m_requestHandler->OnJsonRpcRequest( *this, request, handlerResponse );
                    }
                    catch ( ... )
                    {
                        handled = JSONRPC_REQUEST_HANDLED;
                        handlerResponse = SJsonRpcResponse();
                        handlerResponse.isError = true;
                        handlerResponse.error.code = JSONRPC_ERROR_INTERNAL;
                        handlerResponse.error.message = "Internal error";
                    }
                }

                if ( JSONRPC_REQUEST_HANDLED == handled )
                {
                    AddId( response, request.id );
                    if ( handlerResponse.isError )
                    {
                        CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
                        error->SetAttribute( "code", handlerResponse.error.code );
                        error->SetAttribute( "message", handlerResponse.error.message );
                        if ( handlerResponse.error.hasData )
                            AddMember( *error, "data", handlerResponse.error.data );
                    }
                    else
                    {
                        AddMember( response, "result", handlerResponse.result );
                    }
                    haveResponse = true;
                }
            }
        }
        else
        if ( !hasMethod && ( HasMember( node, "result" ) || HasMember( node, "error" ) ) )
        {
            if ( !versionOk )
                return PROCESS_INVALID;
            return HandleResponseNode( node );
        }
        else
        {
            // neither a call nor an answer
            AddId( response, hasId && hasValidId ? GetId( node ) : CORE::CVariant() );
            CORE::CDataNode* error = response.AddChild( "error", GUCEF_DATATYPE_OBJECT );
            error->SetAttribute( "code", (Int32) JSONRPC_ERROR_INVALID_REQUEST );
            error->SetAttribute( "message", "Invalid request" );
            haveResponse = true;
            result = PROCESS_INVALID;
        }
    }

    if ( haveResponse )
    {
        if ( GUCEF_NULL != batchResponses )
            batchResponses->AddChild( response );
        else
            SendNode( response );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CJsonRpcEndpoint::EProcessResult
CJsonRpcEndpoint::HandleResponseNode( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CVariant id = GetId( node );
    TPendingCallMap::iterator pending = m_pendingCalls.find( CallKey( id ) );
    if ( pending == m_pendingCalls.end() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "JsonRpc: Received a response to a call which is not pending (late or unknown id): " + id.AsString() );
        return PROCESS_OK;
    }

    SJsonRpcResponse response;
    response.id = (*pending).second.callId;
    response.envelope = node;
    m_pendingCalls.erase( pending );

    CORE::CDataNode error;
    if ( GetMember( node, "error", error ) )
    {
        response.isError = true;
        const CORE::CDataNode::TKeyValuePair* code = error.GetAttribute( "code" );
        const CORE::CDataNode::TKeyValuePair* message = error.GetAttribute( "message" );
        response.error.code = GUCEF_NULL != code ? (Int32) (*code).second.AsInt32() : 0;
        response.error.message = GUCEF_NULL != message ? (*message).second.AsString() : CORE::CString();
        response.error.hasData = GetMember( error, "data", response.error.data );
    }
    else
    {
        GetMember( node, "result", response.result );
    }

    if ( !DeliverResponse( response ) )
        return PROCESS_DELETED;
    return PROCESS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
