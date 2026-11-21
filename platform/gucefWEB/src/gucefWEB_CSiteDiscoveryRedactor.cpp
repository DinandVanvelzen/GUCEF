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

#include <string.h>
#include <math.h>
#include <algorithm>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefWEB_CSiteDiscoveryRedactor.h"

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

static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

static bool
StartsWith( const CORE::CString& text, const CORE::CString& prefix )
{
    const UInt32 n = ByteLength( prefix );
    return n <= ByteLength( text ) && 0 == memcmp( text.C_String(), prefix.C_String(), n );
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    if ( part.IsNULLOrEmpty() )
        return false;
    if ( text.IsNULLOrEmpty() )
        return false;
    return GUCEF_NULL != strstr( text.C_String(), part.C_String() );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
PercentEncode( const CORE::CString& text )
{
    static const char* const hex = "0123456789ABCDEF";
    const unsigned char* t = (const unsigned char*) text.C_String();
    const UInt32 len = ByteLength( text );
    CORE::CString result;
    for ( UInt32 i=0; i<len; ++i )
    {
        const unsigned char c = t[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '-' || c == '_' || c == '.' || c == '~' )
            result += (char) c;
        else
        {
            result += '%';
            result += hex[ c >> 4 ];
            result += hex[ c & 15 ];
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
LongerFirst( const CORE::CString& a, const CORE::CString& b )
{
    return a.ByteSize() > b.ByteSize();
}

/*-------------------------------------------------------------------------*/

static bool
IsAlnumChar( unsigned char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' );
}

/*-------------------------------------------------------------------------*/

static bool
IsHexChar( unsigned char c )
{
    return ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) || ( c >= '0' && c <= '9' );
}

/*-------------------------------------------------------------------------*/

/** Does a token start at pos: nothing alphanumeric before it, or a percent escape or a backslash escape (\n) */
static bool
IsBoundaryBefore( const char* text, UInt32 pos )
{
    if ( 0 == pos )
        return true;
    const unsigned char before = (unsigned char) text[ pos-1 ];
    if ( !IsAlnumChar( before ) )
        return true;
    if ( pos >= 3 && text[ pos-3 ] == '%' && IsHexChar( (unsigned char) text[ pos-2 ] ) && IsHexChar( before ) )
        return true;
    if ( pos >= 2 && text[ pos-2 ] == '\\' )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
IsBoundaryAfter( const char* text, UInt32 textLength, UInt32 end )
{
    return end >= textLength || !IsAlnumChar( (unsigned char) text[ end ] );
}

/*-------------------------------------------------------------------------*/

/** The first place the part occurs in the text, as a whole token when wholeToken is set; false when it does not */
static bool
FindPart( const CORE::CString& text, const CORE::CString& part, bool wholeToken, UInt32 startAt, UInt32& position )
{
    const UInt32 textLength = ByteLength( text );
    const UInt32 partLength = ByteLength( part );
    if ( 0 == partLength || textLength < partLength || startAt > textLength - partLength )
        return false;
    const char* t = text.C_String();
    const char* p = part.C_String();
    const char* at = t + startAt;
    while ( GUCEF_NULL != ( at = strstr( at, p ) ) )
    {
        const UInt32 pos = (UInt32) ( at - t );
        if ( !wholeToken || ( IsBoundaryBefore( t, pos ) && IsBoundaryAfter( t, textLength, pos + partLength ) ) )
        {
            position = pos;
            return true;
        }
        ++at;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Shannon entropy of the bytes of the text, in bits over the whole text (bits per byte times the length) */
static UInt32
EntropyBits( const CORE::CString& text )
{
    const unsigned char* t = (const unsigned char*) text.C_String();
    const UInt32 len = ByteLength( text );
    if ( 0 == len )
        return 0;
    UInt32 counts[ 256 ];
    memset( counts, 0, sizeof( counts ) );
    for ( UInt32 i=0; i<len; ++i )
        ++counts[ t[ i ] ];
    double bits = 0.0;
    for ( UInt32 c=0; c<256; ++c )
        if ( counts[ c ] > 0 )
            bits -= (double) counts[ c ] * ( log( (double) counts[ c ] / (double) len ) / log( 2.0 ) );
    return (UInt32) ( bits + 0.5 );
}

/*-------------------------------------------------------------------------*/

/** A value that is made like a token: a run of letters and digits of 8 or more that mixes both, 16 or more digits, or 32 or more letters */
static bool
HasTokenRun( const CORE::CString& value )
{
    const unsigned char* t = (const unsigned char*) value.C_String();
    const UInt32 len = ByteLength( value );
    UInt32 i = 0;
    while ( i < len )
    {
        if ( !IsAlnumChar( t[ i ] ) )
        {
            ++i;
            continue;
        }
        UInt32 digits = 0;
        UInt32 letters = 0;
        UInt32 end = i;
        while ( end < len && IsAlnumChar( t[ end ] ) )
        {
            if ( t[ end ] >= '0' && t[ end ] <= '9' )
                ++digits;
            else
                ++letters;
            ++end;
        }
        const UInt32 run = end - i;
        if ( run >= 8 && digits > 0 && letters > 0 )
            return true;
        if ( run >= 16 && letters == 0 )
            return true;
        if ( run >= 32 )
            return true;
        i = end;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Values shorter than this only count as a whole token, see the class description */
static const UInt32 WholeTokenBelowLength = 24;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CSiteDiscoveryRedactor::MinSecretLength;
const CORE::CString CSiteDiscoveryRedactor::NotRecordedMarker = "<not recorded>";
const CORE::CString CSiteDiscoveryRedactor::SecretMarker = "{secret}";

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor::CSiteDiscoveryRedactor( void )
    : m_settings()
    , m_names()
    , m_text()
    , m_neverRecordHosts()
    , m_secrets()
    , m_ignoredCount( 0 )
{GUCEF_TRACE;

    NewHashKey();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor::CSiteDiscoveryRedactor( const CSiteDiscoverySettings& settings )
    : m_settings( settings )
    , m_names()
    , m_text()
    , m_neverRecordHosts()
    , m_secrets()
    , m_ignoredCount( 0 )
{GUCEF_TRACE;

    NewHashKey();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::NewHashKey( void )
{GUCEF_TRACE;

    if ( !CORE::CRandom::GetRandomBytes( m_hashKey, (UInt32) sizeof( m_hashKey ) ) )
    {
        // never a fixed key: the time and the address of this object, which differ per run
        memset( m_hashKey, 0, sizeof( m_hashKey ) );
        const UInt64 ticks = MT::PrecisionTickCount();
        const void* self = this;
        memcpy( m_hashKey, &ticks, sizeof( ticks ) );
        memcpy( m_hashKey + sizeof( ticks ), &self, sizeof( self ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::SetSettings( const CSiteDiscoverySettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    Rebuild();
}

/*-------------------------------------------------------------------------*/

const CSiteDiscoverySettings&
CSiteDiscoveryRedactor::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::Rebuild( void )
{GUCEF_TRACE;

    m_names = CORE::CRedactor( CORE::REDACT_NAME_WORDS );
    m_text = CORE::CRedactor( CORE::REDACT_NAME_WORDS | CORE::REDACT_TEXT_KEY_VALUES | CORE::REDACT_TEXT_AUTH_SCHEMES );
    m_text.SetMarker( SecretMarker );
    for ( size_t i=0; i<m_settings.secretNamePatterns.size(); ++i )
    {
        m_names.AddSensitiveName( m_settings.secretNamePatterns[ i ] );
        m_text.AddSensitiveName( m_settings.secretNamePatterns[ i ] );
    }

    m_neverRecordHosts.clear();
    for ( size_t i=0; i<m_settings.neverRecordHosts.size(); ++i )
    {
        CORE::CGlobPattern pattern;
        if ( pattern.SetPattern( m_settings.neverRecordHosts[ i ].Lowercase(), false ) )
            m_neverRecordHosts.push_back( pattern );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::MatchesHostFilter( const CORE::CString::StringVector& terms, const CORE::CString& host )
{GUCEF_TRACE;

    bool anyTerm = false;
    const CORE::CString lowerHost = host.Lowercase();
    for ( size_t i=0; i<terms.size(); ++i )
    {
        if ( terms[ i ].IsNULLOrEmpty() )
            continue;
        anyTerm = true;
        if ( Contains( lowerHost, terms[ i ].Lowercase() ) )
            return true;
    }
    return !anyTerm;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsInScope( const CUrlPathTemplate& parsed ) const
{GUCEF_TRACE;

    if ( !MatchesHostFilter( m_settings.hostFilter, parsed.host ) )
        return false;
    if ( m_settings.allowPathPrefixes.empty() )
        return true;
    const CORE::CString lowerPath = parsed.path.Lowercase();
    for ( size_t i=0; i<m_settings.allowPathPrefixes.size(); ++i )
        if ( !m_settings.allowPathPrefixes[ i ].IsNULLOrEmpty() && StartsWith( lowerPath, m_settings.allowPathPrefixes[ i ].Lowercase() ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsNeverRecord( const CUrlPathTemplate& parsed ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_neverRecordHosts.size(); ++i )
        if ( m_neverRecordHosts[ i ].IsMatch( parsed.host ) )
            return true;
    const CORE::CString lowerPath = parsed.path.Lowercase();
    for ( size_t i=0; i<m_settings.neverRecordPathPatterns.size(); ++i )
        if ( Contains( lowerPath, m_settings.neverRecordPathPatterns[ i ].Lowercase() ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsNeverRecordUrl( const CORE::CString& url ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) )
        parsed.path = url;
    return IsNeverRecord( parsed );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsSecretName( const CORE::CString& name ) const
{GUCEF_TRACE;

    return m_names.IsSensitiveName( name );
}

/*-------------------------------------------------------------------------*/

const CORE::CRedactor&
CSiteDiscoveryRedactor::GetNameRedactor( void ) const
{GUCEF_TRACE;

    return m_names;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsPlausibleSecret( const CORE::CString& value ) const
{GUCEF_TRACE;

    const UInt32 length = ByteLength( value );
    const UInt32 minLength = m_settings.knownSecretMinLength > MinSecretLength ? m_settings.knownSecretMinLength : MinSecretLength;
    if ( length < minLength || value == NotRecordedMarker || value == SecretMarker )
        return false;
    if ( !HasTokenRun( value ) )
        return false;       // words, identifiers, ordinary numbers, URLs and paths of words: structure, not a secret
    if ( m_settings.knownSecretMinEntropyBits > 0 && EntropyBits( value ) < m_settings.knownSecretMinEntropyBits )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterSecret( const CORE::CString& value, ESecretClass secretClass )
{GUCEF_TRACE;

    const CORE::CString trimmed = value.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return;
    if ( !IsPlausibleSecret( trimmed ) )
    {
        ++m_ignoredCount;
        return;
    }
    SKnownSecret info;
    info.secretClass = secretClass;
    info.wholeToken = ByteLength( trimmed ) < WholeTokenBelowLength;
    m_secrets.insert( TSecretMap::value_type( trimmed, info ) );
    const CORE::CString escaped = CUrlPathTemplate::EscapeJson( trimmed );
    if ( escaped != trimmed )
        m_secrets.insert( TSecretMap::value_type( escaped, info ) );
    const CORE::CString encoded = PercentEncode( trimmed );
    if ( encoded != trimmed )
        m_secrets.insert( TSecretMap::value_type( encoded, info ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::SecretClassToString( ESecretClass secretClass )
{GUCEF_TRACE;

    switch ( secretClass )
    {
        case SECRET_CLASS_COOKIE:                return "cookie";
        case SECRET_CLASS_CREDENTIAL_HEADER:     return "credential-header";
        case SECRET_CLASS_QUERY_VALUE:           return "query-value";
        case SECRET_CLASS_REQUEST_BODY_FIELD:    return "request-body-field";
        case SECRET_CLASS_RESPONSE_BODY_FIELD:   return "response-body-field";
        default:                                 return "other";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::FormatClassCounts( const TClassCountMap& counts )
{GUCEF_TRACE;

    CORE::CString result;
    for ( TClassCountMap::const_iterator i = counts.begin(); i != counts.end(); ++i )
    {
        if ( !result.IsNULLOrEmpty() )
            result += ", ";
        result += i->first + ": " + CORE::UInt32ToString( i->second );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::CaptureHeaderValue( const CORE::CString& lowerCaseName, const CORE::CString& value )
{GUCEF_TRACE;

    if ( lowerCaseName == "cookie" )
    {
        // every cookie value on its own: one of them may be repeated elsewhere
        CORE::CString::StringVector cookies = value.ParseElements( ';', false );
        for ( size_t i=0; i<cookies.size(); ++i )
        {
            const Int32 eq = cookies[ i ].HasChar( '=' );
            RegisterSecret( eq >= 0 ? cookies[ i ].CutChars( (UInt32) eq + 1, true ) : cookies[ i ], SECRET_CLASS_COOKIE );
        }
        RegisterSecret( value, SECRET_CLASS_COOKIE );
        return NotRecordedMarker;
    }
    if ( lowerCaseName == "set-cookie" )
    {
        RegisterSetCookie( value );
        return NotRecordedMarker;
    }
    if ( IsSecretName( lowerCaseName ) )
    {
        RegisterSecret( value, SECRET_CLASS_CREDENTIAL_HEADER );
        const Int32 space = value.HasChar( ' ' );
        if ( space > 0 )
            RegisterSecret( value.CutChars( (UInt32) space + 1, true ), SECRET_CLASS_CREDENTIAL_HEADER );      // the credential after Bearer, Basic, ...
        return NotRecordedMarker;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterSetCookie( const CORE::CString& headerValue )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = headerValue.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const Int32 eq = lines[ i ].HasChar( '=' );
        if ( eq < 0 )
            continue;
        CORE::CString rest = lines[ i ].CutChars( (UInt32) eq + 1, true );
        const Int32 semicolon = rest.HasChar( ';' );
        RegisterSecret( semicolon >= 0 ? rest.SubstrToIndex( (UInt32) semicolon, true ) : rest, SECRET_CLASS_COOKIE );
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterQuerySecrets( const CORE::CString& url )
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) )
        return;
    for ( size_t i=0; i<parsed.queryParams.size(); ++i )
    {
        if ( IsSecretName( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].first ) ) )
        {
            RegisterSecret( parsed.queryParams[ i ].second, SECRET_CLASS_QUERY_VALUE );
            RegisterSecret( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].second ), SECRET_CLASS_QUERY_VALUE );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterJsonSecrets( const CORE::CDataNode& node, bool parentSecret, ESecretClass secretClass )
{GUCEF_TRACE;

    if ( parentSecret && node.GetAttCount() == 0 && !node.HasChildren() )
        RegisterSecret( node.GetValue().AsString(), secretClass );
    for ( UInt32 i=0; i<node.GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = node.GetAttribute( i );
        if ( GUCEF_NULL != att && ( parentSecret || IsSecretName( att->first ) ) )
            RegisterSecret( att->second.AsString(), secretClass );
    }
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CString& name = (*i)->GetName();
        RegisterJsonSecrets( *(*i), parentSecret || ( !name.IsNULLOrEmpty() && IsSecretName( name ) ), secretClass );
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterBodySecrets( const CORE::CString& contentType, const CORE::CString& body, ESecretClass secretClass )
{GUCEF_TRACE;

    if ( body.IsNULLOrEmpty() )
        return;
    const CORE::CString type = contentType.Lowercase();
    const char first = body.C_String()[ 0 ];
    if ( Contains( type, "json" ) || first == '{' || first == '[' )
    {
        CORE::CDataNode document;
        if ( COM::CJsonRpcEndpoint::ParseJsonText( body, document ) )
            RegisterJsonSecrets( document, false, secretClass );
        return;
    }
    if ( Contains( type, "x-www-form-urlencoded" ) || type.IsNULLOrEmpty() )
    {
        CORE::CString::StringVector pairs = body.ParseElements( '&', false );
        for ( size_t i=0; i<pairs.size(); ++i )
        {
            const Int32 eq = pairs[ i ].HasChar( '=' );
            if ( eq <= 0 )
                continue;
            if ( IsSecretName( CUrlPathTemplate::PercentDecode( pairs[ i ].SubstrToIndex( (UInt32) eq, true ) ) ) )
            {
                const CORE::CString value = pairs[ i ].CutChars( (UInt32) eq + 1, true );
                RegisterSecret( value, secretClass );
                RegisterSecret( CUrlPathTemplate::PercentDecode( value ), secretClass );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryRedactor::GetKnownSecretCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_secrets.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryRedactor::GetIgnoredSecretCount( void ) const
{GUCEF_TRACE;

    return m_ignoredCount;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::ClearKnownSecrets( void )
{GUCEF_TRACE;

    m_secrets.clear();
    m_ignoredCount = 0;
    NewHashKey();
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::FindKnownSecret( const CORE::CString& text, ESecretClass& secretClass ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return false;
    UInt32 position = 0;
    for ( TSecretMap::const_iterator i = m_secrets.begin(); i != m_secrets.end(); ++i )
    {
        if ( FindPart( text, i->first, i->second.wholeToken, 0, position ) )
        {
            secretClass = i->second.secretClass;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::ContainsKnownSecret( const CORE::CString& text ) const
{GUCEF_TRACE;

    ESecretClass ignored = SECRET_CLASS_OTHER;
    return FindKnownSecret( text, ignored );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::ScrubKnownSecrets( const CORE::CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() || m_secrets.empty() || !ContainsKnownSecret( text ) )
        return text;
    CORE::CString::StringVector ordered;
    for ( TSecretMap::const_iterator i = m_secrets.begin(); i != m_secrets.end(); ++i )
        ordered.push_back( i->first );
    std::sort( ordered.begin(), ordered.end(), LongerFirst );
    CORE::CString result = text;
    for ( size_t i=0; i<ordered.size(); ++i )
    {
        const bool wholeToken = m_secrets.find( ordered[ i ] )->second.wholeToken;
        UInt32 position = 0;
        UInt32 startAt = 0;
        CORE::CString rebuilt;
        bool found = false;
        while ( FindPart( result, ordered[ i ], wholeToken, startAt, position ) )
        {
            found = true;
            rebuilt += CORE::CString( result.C_String() + startAt, position - startAt );
            rebuilt += SecretMarker;
            startAt = position + ByteLength( ordered[ i ] );
        }
        if ( found )
        {
            rebuilt += CORE::CString( result.C_String() + startAt, ByteLength( result ) - startAt );
            result = rebuilt;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactSecretValue( const CORE::CString& value ) const
{GUCEF_TRACE;

    if ( CSiteDiscoverySettings::SECRET_VALUE_STRIP == m_settings.secretValueMode || value.IsNULLOrEmpty() )
        return SecretMarker;

    UInt8 mac[ CORE::CHmacSha256::MacSize ];
    CORE::CHmacSha256::Mac( m_hashKey, (UInt32) sizeof( m_hashKey ), value.C_String(), ByteLength( value ), mac );
    static const char* const hex = "0123456789abcdef";
    char text[ 13 ];
    for ( UInt32 i=0; i<6; ++i )
    {
        text[ i*2 ] = hex[ mac[ i ] >> 4 ];
        text[ i*2 + 1 ] = hex[ mac[ i ] & 15 ];
    }
    text[ 12 ] = '\0';
    return CORE::CString( "{secret:" ) + text + "}";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactQueryValue( const CORE::CString& decodedName, const CORE::CString& value ) const
{GUCEF_TRACE;

    if ( IsSecretName( decodedName ) )
        return RedactSecretValue( CUrlPathTemplate::PercentDecode( value ) );
    return ScrubKnownSecrets( value );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactUrl( const CORE::CString& url, bool neverRecorded ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) )
    {
        // data:, blob: and the like are not recorded; should one come here, only its scheme is kept
        const Int32 colon = url.HasChar( ':' );
        return colon > 0 ? url.SubstrToIndex( (UInt32) colon + 1, true ) : CORE::CString( "{url}" );
    }

    const bool examples = m_settings.recordExampleValues && !neverRecorded;
    CORE::CString result = parsed.scheme + "://" + parsed.host + ( examples ? ScrubKnownSecrets( parsed.path ) : parsed.pathTemplate );
    if ( neverRecorded )
        return result;
    for ( size_t i=0; i<parsed.queryParams.size(); ++i )
    {
        result += ( i == 0 ? '?' : '&' );
        result += ScrubKnownSecrets( parsed.queryParams[ i ].first );
        if ( examples )
            result += "=" + RedactQueryValue( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].first ), parsed.queryParams[ i ].second );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactHeaderValue( const CORE::CString& lowerCaseName, const CORE::CString& value ) const
{GUCEF_TRACE;

    if ( value == NotRecordedMarker || lowerCaseName == "cookie" || lowerCaseName == "set-cookie" || IsSecretName( lowerCaseName ) )
        return NotRecordedMarker;
    if ( !m_settings.recordExampleValues )
    {
        if ( lowerCaseName == "content-type" || lowerCaseName == "accept" || lowerCaseName == "x-requested-with" )
            return ScrubKnownSecrets( value );
        return CORE::CString::Empty;
    }
    if ( lowerCaseName == "referer" || lowerCaseName == "origin" || lowerCaseName == "location" )
        return RedactUrl( value, IsNeverRecordUrl( value ) );
    return ScrubKnownSecrets( m_text.RedactText( value ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactBody( const CORE::CString& body ) const
{GUCEF_TRACE;

    if ( body.IsNULLOrEmpty() )
        return body;
    return ScrubKnownSecrets( m_text.RedactText( body ) );
}

/*-------------------------------------------------------------------------*/

static bool
IsEnumChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
           c == '_' || c == '.' || c == ',' || c == ':' || c == '+' || c == '-';
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsEnumLikeQueryValue( const CORE::CString& decodedName, const CORE::CString& decodedValue ) const
{GUCEF_TRACE;

    const UInt32 length = ByteLength( decodedValue );
    if ( 0 == length || length > 32 || decodedName.IsNULLOrEmpty() )
        return false;
    const char* t = decodedValue.C_String();
    bool allDigits = true;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( !IsEnumChar( t[ i ] ) )
            return false;
        if ( t[ i ] < '0' || t[ i ] > '9' )
            allDigits = false;
    }
    if ( IsSecretName( decodedName ) )
        return false;
    if ( allDigits )
    {
        if ( length > 4 )
            return false;       // a number of that size is an identifier or a timestamp: {n}
    }
    else
    if ( CUrlPathTemplate::LooksLikeId( decodedValue ) )
        return false;
    if ( HasTokenRun( decodedValue ) || IsPlausibleSecret( decodedValue ) || ContainsKnownSecret( decodedValue ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::ExtractEnumQueryValues( const CUrlPathTemplate& parsed, GUCEF::vector< std::pair< CORE::CString, CORE::CString > >& result ) const
{GUCEF_TRACE;

    result.clear();
    if ( !m_settings.recordEnumQueryValues )
        return;
    for ( size_t i=0; i<parsed.queryParams.size(); ++i )
    {
        const CORE::CString name = CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].first );
        const CORE::CString value = CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].second );
        if ( IsEnumLikeQueryValue( name, value ) )
            result.push_back( std::make_pair( name, value ) );
    }
}

/*-------------------------------------------------------------------------*/

static void
AddUniqueName( CORE::CString::StringVector& names, const CORE::CString& name )
{
    if ( !name.IsNULLOrEmpty() && std::find( names.begin(), names.end(), name ) == names.end() )
        names.push_back( name );
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::ParseCookieNames( const CORE::CString& cookieHeader, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    CORE::CString::StringVector cookies = cookieHeader.ParseElements( ';', false );
    for ( size_t i=0; i<cookies.size(); ++i )
    {
        const CORE::CString cookie = cookies[ i ].Trim( true ).Trim( false );
        const Int32 eq = cookie.HasChar( '=' );
        AddUniqueName( names, eq >= 0 ? cookie.SubstrToIndex( (UInt32) eq, true ).Trim( true ).Trim( false ) : cookie );
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::ParseSetCookieNames( const CORE::CString& setCookieHeader, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = setCookieHeader.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const Int32 eq = lines[ i ].HasChar( '=' );
        if ( eq <= 0 )
            continue;
        AddUniqueName( names, lines[ i ].SubstrToIndex( (UInt32) eq, true ).Trim( true ).Trim( false ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactCookieName( const CORE::CString& name ) const
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return name;
    if ( IsPlausibleSecret( name ) || ContainsKnownSecret( name ) )
        return "{token-like name}";
    CORE::CString result = ScrubKnownSecrets( CUrlPathTemplate::TemplateSegment( name ) );
    if ( result.Length() > 64 )
        result = result.SubstrToIndex( 64, true );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactLabel( const CORE::CString& label ) const
{GUCEF_TRACE;

    CORE::CRedactor redactor( m_text );
    redactor.AddFlags( CORE::REDACT_TEXT_TOKEN_RUNS | CORE::REDACT_TEXT_EMAILS | CORE::REDACT_TEXT_CARD_NUMBERS | CORE::REDACT_SANITIZE_CONTROL );
    redactor.SetTokenMinLength( 16 );
    const CORE::CString redacted = ScrubKnownSecrets( redactor.RedactText( label ) );

    // runs of digits are what differs between accounts, orders and dates: {n}
    const char* t = redacted.C_String();
    const UInt32 len = ByteLength( redacted );
    CORE::CString result;
    UInt32 i = 0;
    while ( i < len )
    {
        if ( t[ i ] >= '0' && t[ i ] <= '9' )
        {
            UInt32 end = i;
            while ( end < len && ( ( t[ end ] >= '0' && t[ end ] <= '9' ) || ( end > i && ( t[ end ] == ',' || t[ end ] == '.' || t[ end ] == '-' || t[ end ] == '/' ) && end + 1 < len && t[ end+1 ] >= '0' && t[ end+1 ] <= '9' ) ) )
                ++end;
            result += "{n}";
            i = end;
        }
        else
        {
            result += t[ i ];
            ++i;
        }
    }
    result = result.Trim( true ).Trim( false );
    if ( result.Length() > 60 )
        result = result.SubstrToIndex( 60, true );
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
