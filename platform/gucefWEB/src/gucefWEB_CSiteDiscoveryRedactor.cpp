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

/** Lower case of the ASCII letters only, so that positions stay the same */
static CORE::CString
AsciiLower( const CORE::CString& text )
{
    const UInt32 len = ByteLength( text );
    if ( 0 == len )
        return CORE::CString();
    GUCEF::vector< char > buffer( text.C_String(), text.C_String() + len );
    for ( UInt32 i=0; i<len; ++i )
        if ( buffer[ i ] >= 'A' && buffer[ i ] <= 'Z' )
            buffer[ i ] = (char) ( buffer[ i ] - 'A' + 'a' );
    return CORE::CString( &buffer[ 0 ], len );
}

/*-------------------------------------------------------------------------*/

/** Does a placeholder of ours start at pos ({n}, {value}, {secret:ab12}, {token-like name}): small letters, digits, ':', '-', '_' and spaces in braces */
static bool
IsPlaceholderAt( const char* text, UInt32 length, UInt32 pos, UInt32& end )
{
    if ( pos >= length || text[ pos ] != '{' )
        return false;
    UInt32 j = pos + 1;
    while ( j < length && j - pos <= 40 )
    {
        const char c = text[ j ];
        if ( c == '}' )
        {
            if ( j == pos + 1 )
                return false;
            end = j + 1;
            return true;
        }
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || c == ':' || c == '-' || c == '_' || c == ' ' ) )
            return false;
        ++j;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
IsPlaceholderText( const CORE::CString& text )
{
    const UInt32 len = ByteLength( text );
    UInt32 end = 0;
    return IsPlaceholderAt( text.C_String(), len, 0, end ) && end == len;
}

/*-------------------------------------------------------------------------*/

static bool
LongerTextFirst( const CORE::CString& a, const CORE::CString& b )
{
    return a.ByteSize() > b.ByteSize();
}

/*-------------------------------------------------------------------------*/

namespace {

/** A part of a file name: kind 0 a separator, 1 a word of letters and digits, 2 a placeholder */
struct SFileNameItem
{
    CORE::CString text;
    int kind;
};

typedef GUCEF::vector< SFileNameItem > TFileNameItemVector;

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

/** Values shorter than this only count as a whole token, see the class description */
static const UInt32 WholeTokenBelowLength = 24;

/** The most public URL parts kept, so a long session does not grow the set without end */
static const UInt32 MaxPublicParts = 100000;

/** A registered value that appears in more than this percentage of the journal lines is named as probably not a secret */
static const UInt32 WarnLineSharePercent = 5;

/*-------------------------------------------------------------------------*/

/** A URL ("https://...", "//host/...") or a path ("/a/b", "./a"): its parts are looked at one by one, never the whole */
static bool
LooksLikeUrlOrPath( const CORE::CString& value )
{
    const char* t = value.C_String();
    const UInt32 len = ByteLength( value );
    if ( 0 == len )
        return false;
    for ( UInt32 i=0; i<len; ++i )
        if ( t[ i ] == ' ' || t[ i ] == '\t' || t[ i ] == '\n' )
            return false;
    if ( t[ 0 ] == '/' || ( len > 1 && t[ 0 ] == '.' && t[ 1 ] == '/' ) )
        return true;
    return GUCEF_NULL != strstr( t, "://" );
}

/*-------------------------------------------------------------------------*/

/** A host name or cookie domain: labels of small letters, digits and '-' separated by dots, the last of 2 to 24 letters */
static bool
LooksLikeHostName( const CORE::CString& value )
{
    const char* t = value.C_String();
    UInt32 len = ByteLength( value );
    UInt32 start = 0;
    if ( len > 0 && t[ 0 ] == '.' )
        start = 1;
    if ( len - start < 4 )
        return false;
    UInt32 dots = 0;
    UInt32 labelLength = 0;
    UInt32 lastLabelStart = start;
    for ( UInt32 i=start; i<len; ++i )
    {
        const char c = t[ i ];
        if ( c == '.' )
        {
            if ( 0 == labelLength )
                return false;
            ++dots;
            labelLength = 0;
            lastLabelStart = i + 1;
            continue;
        }
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || c == '-' ) )
            return false;
        if ( ++labelLength > 63 )
            return false;
    }
    const UInt32 lastLength = len - lastLabelStart;
    if ( 0 == dots || lastLength < 2 || lastLength > 24 )
        return false;
    for ( UInt32 i=lastLabelStart; i<len; ++i )
        if ( t[ i ] < 'a' || t[ i ] > 'z' )
            return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CSiteDiscoveryRedactor::MinSecretLength;
const CORE::CString CSiteDiscoveryRedactor::NotRecordedMarker = "<not recorded>";
const CORE::CString CSiteDiscoveryRedactor::SecretMarker = "{secret}";
const CORE::CString CSiteDiscoveryRedactor::ValueMarker = "{value}";
const CORE::CString CSiteDiscoveryRedactor::KeyMarker = "{key}";
const CORE::CString CSiteDiscoveryRedactor::TokenLikeNameMarker = "{token-like name}";
const CORE::CString CSiteDiscoveryRedactor::TextMarker = "{text}";

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor::CSiteDiscoveryRedactor( void )
    : m_settings()
    , m_names()
    , m_text()
    , m_neverRecordHosts()
    , m_valuePatterns()
    , m_dictionary()
    , m_secrets()
    , m_secretInfos()
    , m_ignoredFields()
    , m_publicParts()
    , m_publicHosts()
    , m_ignoredCount( 0 )
    , m_assessedLines( 0 )
    , m_journalMode( false )
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
    , m_valuePatterns()
    , m_dictionary()
    , m_secrets()
    , m_secretInfos()
    , m_ignoredFields()
    , m_publicParts()
    , m_publicHosts()
    , m_ignoredCount( 0 )
    , m_assessedLines( 0 )
    , m_journalMode( false )
{GUCEF_TRACE;

    NewHashKey();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor::SSecretInfo::SSecretInfo( void )
    : secretClass( SECRET_CLASS_OTHER )
    , fieldName()
    , length( 0 )
    , entropyBits( 0 )
    , credential( false )
    , publicValue( false )
    , suspectedPublic( false )
    , journalLines( 0 )
    , firstJournalLines()
{GUCEF_TRACE;

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

    m_valuePatterns.clear();
    for ( size_t i=0; i<m_settings.sensitiveValuePatterns.size(); ++i )
    {
        CORE::CGlobPattern pattern;
        if ( !m_settings.sensitiveValuePatterns[ i ].IsNULLOrEmpty() && pattern.SetPattern( AsciiLower( m_settings.sensitiveValuePatterns[ i ] ), false ) )
            m_valuePatterns.push_back( pattern );
    }
    m_dictionary.clear();
    for ( size_t i=0; i<m_settings.sensitiveValues.size(); ++i )
    {
        const CORE::CString value = AsciiLower( m_settings.sensitiveValues[ i ].Trim( true ).Trim( false ) );
        if ( !value.IsNULLOrEmpty() )
            m_dictionary.push_back( value );
    }
    std::sort( m_dictionary.begin(), m_dictionary.end(), LongerTextFirst );

    m_ignoredFields.clear();
    for ( size_t i=0; i<m_settings.ignoredSecretFields.size(); ++i )
    {
        CORE::CGlobPattern pattern;
        if ( !m_settings.ignoredSecretFields[ i ].IsNULLOrEmpty() && pattern.SetPattern( AsciiLower( m_settings.ignoredSecretFields[ i ] ), false ) )
            m_ignoredFields.push_back( pattern );
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

bool
CSiteDiscoveryRedactor::IsPasswordFieldName( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lower = AsciiLower( name );
    return Contains( lower, "passw" ) || Contains( lower, "pwd" ) || Contains( lower, "passphrase" ) || Contains( lower, "passcode" );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsIgnoredSecretField( const CORE::CString& name ) const
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || m_ignoredFields.empty() )
        return false;
    const CORE::CString lower = AsciiLower( name );
    for ( size_t i=0; i<m_ignoredFields.size(); ++i )
        if ( m_ignoredFields[ i ].IsMatch( lower ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterUrlParts( const CORE::CString& value, ESecretClass secretClass, const CORE::CString& fieldName )
{GUCEF_TRACE;

    // the whole is an address; a token in it is in one of its path segments or query values
    CORE::CString path = value;
    CORE::CString query;
    const Int32 question = value.HasChar( '?' );
    if ( question >= 0 )
    {
        path = value.SubstrToIndex( (UInt32) question, true );
        query = value.CutChars( (UInt32) question + 1, true );
    }
    const Int32 hash = query.HasChar( '#' );
    if ( hash >= 0 )
        query = query.SubstrToIndex( (UInt32) hash, true );
    const Int32 scheme = path.HasSubstr( "://" );
    if ( scheme >= 0 )
        path = path.CutChars( (UInt32) scheme + 3, true );
    CORE::CString::StringVector segments = path.ParseElements( '/', false );
    for ( size_t i=0; i<segments.size(); ++i )
    {
        // the host of a URL is public, path segments are looked at on their own
        if ( scheme >= 0 && i == 0 )
            continue;
        const CORE::CString segment = CUrlPathTemplate::PercentDecode( segments[ i ] );
        if ( !LooksLikeUrlOrPath( segment ) )
            RegisterSecret( segment, secretClass, fieldName );
    }
    CORE::CString::StringVector items = query.ParseElements( '&', false );
    for ( size_t i=0; i<items.size(); ++i )
    {
        const Int32 eq = items[ i ].HasChar( '=' );
        if ( eq <= 0 )
            continue;
        const CORE::CString name = CUrlPathTemplate::PercentDecode( items[ i ].SubstrToIndex( (UInt32) eq, true ) );
        const CORE::CString itemValue = CUrlPathTemplate::PercentDecode( items[ i ].CutChars( (UInt32) eq + 1, true ) );
        if ( !LooksLikeUrlOrPath( itemValue ) )
            RegisterSecret( itemValue, secretClass, name );
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterSecret( const CORE::CString& value, ESecretClass secretClass, const CORE::CString& fieldName )
{GUCEF_TRACE;

    const CORE::CString trimmed = value.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return;
    const bool credential = SECRET_CLASS_COOKIE == secretClass || SECRET_CLASS_CREDENTIAL_HEADER == secretClass ||
                            ( !fieldName.IsNULLOrEmpty() && IsPasswordFieldName( fieldName ) );
    if ( !credential && IsIgnoredSecretField( fieldName ) )
    {
        ++m_ignoredCount;
        return;
    }
    if ( LooksLikeUrlOrPath( trimmed ) )
    {
        ++m_ignoredCount;
        RegisterUrlParts( trimmed, secretClass, fieldName );
        return;
    }
    if ( LooksLikeHostName( trimmed ) || !IsPlausibleSecret( trimmed ) )
    {
        ++m_ignoredCount;
        return;
    }

    TSecretMap::iterator known = m_secrets.find( trimmed );
    if ( known != m_secrets.end() )
    {
        // a value seen before: a credential stays one, and one that turns out to be a credential becomes one
        SSecretInfo& info = m_secretInfos[ known->second ];
        if ( credential && !info.credential )
        {
            info.credential = true;
            info.secretClass = secretClass;
            info.fieldName = fieldName;
            info.publicValue = false;
            info.suspectedPublic = false;
        }
        return;
    }

    SSecretInfo info;
    info.secretClass = secretClass;
    info.fieldName = fieldName;
    info.length = ByteLength( trimmed );
    info.entropyBits = EntropyBits( trimmed );
    info.credential = credential;
    info.publicValue = !credential && IsPublicText( trimmed );
    const UInt32 index = (UInt32) m_secretInfos.size();
    m_secretInfos.push_back( info );
    m_secrets.insert( TSecretMap::value_type( trimmed, index ) );
    const CORE::CString escaped = CUrlPathTemplate::EscapeJson( trimmed );
    if ( escaped != trimmed )
        m_secrets.insert( TSecretMap::value_type( escaped, index ) );
    const CORE::CString encoded = PercentEncode( trimmed );
    if ( encoded != trimmed )
        m_secrets.insert( TSecretMap::value_type( encoded, index ) );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsPublicText( const CORE::CString& value ) const
{GUCEF_TRACE;

    if ( m_publicParts.find( value ) != m_publicParts.end() )
        return true;
    UInt32 position = 0;
    for ( TStringSet::const_iterator h = m_publicHosts.begin(); h != m_publicHosts.end(); ++h )
        if ( FindPart( *h, value, true, 0, position ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::NotePublicPart( const CORE::CString& part )
{GUCEF_TRACE;

    if ( part.IsNULLOrEmpty() || m_publicParts.size() >= MaxPublicParts || !m_publicParts.insert( part ).second )
        return;
    TSecretMap::const_iterator known = m_secrets.find( part );
    if ( known != m_secrets.end() && !m_secretInfos[ known->second ].credential )
        m_secretInfos[ known->second ].publicValue = true;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::NotePublicHost( const CORE::CString& host )
{GUCEF_TRACE;

    const CORE::CString lower = AsciiLower( host );
    if ( lower.IsNULLOrEmpty() || m_publicHosts.size() >= MaxPublicParts || !m_publicHosts.insert( lower ).second )
        return;
    UInt32 position = 0;
    for ( TSecretMap::const_iterator i = m_secrets.begin(); i != m_secrets.end(); ++i )
    {
        SSecretInfo& info = m_secretInfos[ i->second ];
        if ( !info.credential && !info.publicValue && FindPart( lower, i->first, true, 0, position ) )
            info.publicValue = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::NotePublicUrl( const CORE::CString& url )
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) || IsNeverRecord( parsed ) )
        return;
    NotePublicHost( parsed.host );
    CORE::CString::StringVector segments = parsed.path.ParseElements( '/', false );
    for ( size_t i=0; i<segments.size(); ++i )
        NotePublicPart( CUrlPathTemplate::PercentDecode( segments[ i ] ) );
    for ( size_t i=0; i<parsed.queryParams.size(); ++i )
        if ( !IsSecretName( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].first ) ) )
            NotePublicPart( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].second ) );
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::SetJournalMode( bool journalMode )
{GUCEF_TRACE;

    m_journalMode = journalMode;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsJournalMode( void ) const
{GUCEF_TRACE;

    return m_journalMode;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsLookedFor( UInt32 index ) const
{GUCEF_TRACE;

    const SSecretInfo& info = m_secretInfos[ index ];
    if ( info.credential )
        return true;
    return !m_journalMode && !info.publicValue && !info.suspectedPublic;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryRedactor::AssessJournal( const CORE::CString& journalText )
{GUCEF_TRACE;

    m_assessedLines = 0;
    for ( size_t i=0; i<m_secretInfos.size(); ++i )
    {
        m_secretInfos[ i ].journalLines = 0;
        m_secretInfos[ i ].firstJournalLines.clear();
    }
    if ( m_secrets.empty() )
        return 0;

    // which values does each line hold: a value counts once per line, whichever of its forms is there
    const char* t = journalText.IsNULLOrEmpty() ? "" : journalText.C_String();
    const UInt32 len = ByteLength( journalText );
    GUCEF::vector< UInt32 > seenOnLine( m_secretInfos.size(), 0 );
    UInt32 start = 0;
    UInt32 lineNumber = 0;
    while ( start < len )
    {
        UInt32 end = start;
        while ( end < len && t[ end ] != '\n' )
            ++end;
        const CORE::CString line( t + start, end - start );
        start = end + 1;
        ++lineNumber;
        if ( line.IsNULLOrEmpty() )
            continue;
        ++m_assessedLines;
        UInt32 position = 0;
        for ( TSecretMap::const_iterator i = m_secrets.begin(); i != m_secrets.end(); ++i )
        {
            SSecretInfo& info = m_secretInfos[ i->second ];
            if ( seenOnLine[ i->second ] == lineNumber )
                continue;
            const bool wholeToken = info.length < WholeTokenBelowLength || ( !info.credential &&
                                    ( SECRET_CLASS_REQUEST_BODY_FIELD == info.secretClass || SECRET_CLASS_RESPONSE_BODY_FIELD == info.secretClass ) );
            if ( !FindPart( line, i->first, wholeToken, 0, position ) )
                continue;
            seenOnLine[ i->second ] = lineNumber;
            ++info.journalLines;
            if ( info.firstJournalLines.size() < MaxListedJournalLines )
                info.firstJournalLines.push_back( lineNumber );
        }
    }

    // a value in a large share of the lines is a public string that a field with a secret looking name held
    UInt32 suspected = 0;
    if ( m_settings.tripwireMaxLineShare > 0 && m_assessedLines > 0 )
    {
        for ( size_t i=0; i<m_secretInfos.size(); ++i )
        {
            SSecretInfo& info = m_secretInfos[ i ];
            if ( info.credential || info.publicValue || info.suspectedPublic )
                continue;
            if ( (UInt64) info.journalLines * 100 > (UInt64) m_settings.tripwireMaxLineShare * m_assessedLines )
            {
                info.suspectedPublic = true;
                ++suspected;
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoveryRedactor: Not looked for in the report, suspected public: " << DescribeSecretUse( (UInt32) i ) );
            }
        }
    }
    return suspected;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryRedactor::GetAssessedJournalLineCount( void ) const
{GUCEF_TRACE;

    return m_assessedLines;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryRedactor::GetSecretInfoCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_secretInfos.size();
}

/*-------------------------------------------------------------------------*/

const CSiteDiscoveryRedactor::SSecretInfo&
CSiteDiscoveryRedactor::GetSecretInfo( UInt32 index ) const
{GUCEF_TRACE;

    return m_secretInfos[ index ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::DescribeSecret( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= m_secretInfos.size() )
        return CORE::CString::Empty;
    const SSecretInfo& info = m_secretInfos[ index ];
    CORE::CString field = "(unknown)";
    if ( !info.fieldName.IsNULLOrEmpty() )
        field = "`" + RedactMemberName( info.fieldName ) + "`";
    return "class " + SecretClassToString( info.secretClass ) + ", field " + field + ", length " + CORE::UInt32ToString( info.length ) +
           ", entropy " + CORE::UInt32ToString( info.entropyBits ) + " bits";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::DescribeSecretUse( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= m_secretInfos.size() )
        return CORE::CString::Empty;
    const SSecretInfo& info = m_secretInfos[ index ];
    CORE::CString text = DescribeSecret( index );
    if ( 0 == m_assessedLines )
        return text;
    text += "; in " + CORE::UInt32ToString( info.journalLines ) + " of " + CORE::UInt32ToString( m_assessedLines ) + " journal lines";
    if ( !info.firstJournalLines.empty() )
    {
        text += " (lines ";
        for ( size_t i=0; i<info.firstJournalLines.size(); ++i )
            text += ( i > 0 ? ", " : "" ) + CORE::UInt32ToString( info.firstJournalLines[ i ] );
        text += info.journalLines > info.firstJournalLines.size() ? ", ...)" : ")";
    }
    if ( (UInt64) info.journalLines * 100 > (UInt64) WarnLineSharePercent * m_assessedLines )
    {
        text += ". One value matched " + CORE::UInt32ToString( info.journalLines ) + " of " + CORE::UInt32ToString( m_assessedLines ) +
                " lines: probably not a secret: field " + ( info.fieldName.IsNULLOrEmpty() ? CORE::CString( "(unknown)" ) : RedactMemberName( info.fieldName ) ) +
                ", length " + CORE::UInt32ToString( info.length ) + ", entropy " + CORE::UInt32ToString( info.entropyBits ) +
                ": add the field name to ignoredSecretFields or raise knownSecretMinEntropyBits";
    }
    return text;
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
            const CORE::CString cookieName = eq >= 0 ? cookies[ i ].SubstrToIndex( (UInt32) eq, true ).Trim( true ).Trim( false ) : CORE::CString( "cookie" );
            RegisterSecret( eq >= 0 ? cookies[ i ].CutChars( (UInt32) eq + 1, true ) : cookies[ i ], SECRET_CLASS_COOKIE, cookieName );
        }
        RegisterSecret( value, SECRET_CLASS_COOKIE, "cookie" );
        return NotRecordedMarker;
    }
    if ( lowerCaseName == "set-cookie" )
    {
        RegisterSetCookie( value );
        return NotRecordedMarker;
    }
    if ( IsSecretName( lowerCaseName ) )
    {
        if ( lowerCaseName != "authorization" && lowerCaseName != "proxy-authorization" && IsIgnoredSecretField( lowerCaseName ) )
            return NotRecordedMarker;
        RegisterSecret( value, SECRET_CLASS_CREDENTIAL_HEADER, lowerCaseName );
        const Int32 space = value.HasChar( ' ' );
        if ( space > 0 )
            RegisterSecret( value.CutChars( (UInt32) space + 1, true ), SECRET_CLASS_CREDENTIAL_HEADER, lowerCaseName );      // the credential after Bearer, Basic, ...
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
        RegisterSecret( semicolon >= 0 ? rest.SubstrToIndex( (UInt32) semicolon, true ) : rest, SECRET_CLASS_COOKIE, lines[ i ].SubstrToIndex( (UInt32) eq, true ).Trim( true ).Trim( false ) );
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
        const CORE::CString name = CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].first );
        if ( IsSecretName( name ) )
        {
            RegisterSecret( parsed.queryParams[ i ].second, SECRET_CLASS_QUERY_VALUE, name );
            RegisterSecret( CUrlPathTemplate::PercentDecode( parsed.queryParams[ i ].second ), SECRET_CLASS_QUERY_VALUE, name );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryRedactor::RegisterJsonSecrets( const CORE::CDataNode& node, bool parentSecret, const CORE::CString& secretFieldName, ESecretClass secretClass )
{GUCEF_TRACE;

    // secretFieldName: the nearest name that made this part secret, for the diagnostics and the password and ignore rules
    if ( parentSecret && node.GetAttCount() == 0 && !node.HasChildren() )
        RegisterSecret( node.GetValue().AsString(), secretClass, secretFieldName );
    for ( UInt32 i=0; i<node.GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = node.GetAttribute( i );
        if ( GUCEF_NULL == att )
            continue;
        if ( IsSecretName( att->first ) )
            RegisterSecret( att->second.AsString(), secretClass, att->first );
        else
        if ( parentSecret )
            RegisterSecret( att->second.AsString(), secretClass, secretFieldName );
    }
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CString& name = (*i)->GetName();
        const bool secretName = !name.IsNULLOrEmpty() && IsSecretName( name );
        RegisterJsonSecrets( *(*i), parentSecret || secretName, secretName ? name : secretFieldName, secretClass );
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
            RegisterJsonSecrets( document, false, CORE::CString::Empty, secretClass );
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
            const CORE::CString name = CUrlPathTemplate::PercentDecode( pairs[ i ].SubstrToIndex( (UInt32) eq, true ) );
            if ( IsSecretName( name ) )
            {
                const CORE::CString value = pairs[ i ].CutChars( (UInt32) eq + 1, true );
                RegisterSecret( value, secretClass, name );
                RegisterSecret( CUrlPathTemplate::PercentDecode( value ), secretClass, name );
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
    m_secretInfos.clear();
    m_publicParts.clear();
    m_publicHosts.clear();
    m_ignoredCount = 0;
    m_assessedLines = 0;
    NewHashKey();
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::FindKnownSecretIndex( const CORE::CString& text, UInt32& index ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return false;
    UInt32 position = 0;
    for ( TSecretMap::const_iterator i = m_secrets.begin(); i != m_secrets.end(); ++i )
    {
        if ( !IsLookedFor( i->second ) )
            continue;
        const SSecretInfo& info = m_secretInfos[ i->second ];
        const bool wholeToken = info.length < WholeTokenBelowLength || ( !info.credential &&
                                ( SECRET_CLASS_REQUEST_BODY_FIELD == info.secretClass || SECRET_CLASS_RESPONSE_BODY_FIELD == info.secretClass ) );
        if ( FindPart( text, i->first, wholeToken, 0, position ) )
        {
            index = i->second;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::FindKnownSecret( const CORE::CString& text, ESecretClass& secretClass ) const
{GUCEF_TRACE;

    UInt32 index = 0;
    if ( !FindKnownSecretIndex( text, index ) )
        return false;
    secretClass = m_secretInfos[ index ].secretClass;
    return true;
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
        if ( IsLookedFor( i->second ) )
            ordered.push_back( i->first );
    std::sort( ordered.begin(), ordered.end(), LongerFirst );
    CORE::CString result = text;
    for ( size_t i=0; i<ordered.size(); ++i )
    {
        const SSecretInfo& info = m_secretInfos[ m_secrets.find( ordered[ i ] )->second ];
        const bool wholeToken = info.length < WholeTokenBelowLength || ( !info.credential &&
                                ( SECRET_CLASS_REQUEST_BODY_FIELD == info.secretClass || SECRET_CLASS_RESPONSE_BODY_FIELD == info.secretClass ) );
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
    if ( StartsWith( value, "{secret" ) && IsPlaceholderText( value ) )
        return value;       // redacted before: a journal read back

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
    if ( !m_journalMode && !value.IsNULLOrEmpty() && ( IsValueQueryName( decodedName ) || IsSensitiveValue( CUrlPathTemplate::PercentDecode( value ) ) ) )
        return ValueMarker;
    return ScrubKnownSecrets( value );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactUrl( const CORE::CString& url, bool neverRecorded ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( MaskUrl( url ), parsed ) )
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
        result += ScrubKnownSecrets( parsed.queryParams[ i ].first );     // MaskUrl() replaced the token-like names
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
    return MaskValues( ScrubKnownSecrets( m_text.RedactText( body ) ) );
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
        if ( IsEnumQueryName( name ) && IsEnumLikeQueryValue( name, value ) )
            result.push_back( std::make_pair( name, value ) );
    }
    RedactEnumValues( result );
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
    const CORE::CString redacted = MaskValues( ScrubKnownSecrets( redactor.RedactText( label ) ) );

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

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsInList( const CORE::CString::StringVector& list, const CORE::CString& value ) const
{GUCEF_TRACE;

    const CORE::CString lower = AsciiLower( value );
    for ( size_t i=0; i<list.size(); ++i )
        if ( AsciiLower( list[ i ] ) == lower )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsSensitiveToken( const CORE::CString& token ) const
{GUCEF_TRACE;

    const UInt32 len = ByteLength( token );
    if ( 0 == len )
        return false;
    const CORE::CString lower = AsciiLower( token );
    for ( size_t i=0; i<m_dictionary.size(); ++i )
        if ( m_dictionary[ i ] == lower )
            return true;
    for ( size_t i=0; i<m_valuePatterns.size(); ++i )
        if ( m_valuePatterns[ i ].IsMatch( lower ) )
            return true;
    if ( m_settings.maskUppercaseTokens && len <= m_settings.uppercaseTokenMaxLength )
    {
        const char* t = token.C_String();
        for ( UInt32 i=0; i<len; ++i )
            if ( t[ i ] < 'A' || t[ i ] > 'Z' )
                return false;
        return !IsInList( m_settings.nonSensitiveValues, token );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::MaskDictionary( const CORE::CString& text, const CORE::CString& marker ) const
{GUCEF_TRACE;

    if ( m_dictionary.empty() || text.IsNULLOrEmpty() )
        return text;
    CORE::CString result = text;
    for ( size_t d=0; d<m_dictionary.size(); ++d )
    {
        const CORE::CString& word = m_dictionary[ d ];
        const UInt32 wordLength = ByteLength( word );
        CORE::CString lower = AsciiLower( result );
        const char* l = lower.C_String();
        const UInt32 len = ByteLength( lower );
        CORE::CString rebuilt;
        UInt32 copied = 0;
        UInt32 pos = 0;
        bool found = false;
        while ( pos + wordLength <= len )
        {
            const char* at = strstr( l + pos, word.C_String() );
            if ( GUCEF_NULL == at )
                break;
            const UInt32 start = (UInt32) ( at - l );
            const UInt32 end = start + wordLength;
            const bool before = 0 == start || !IsAlnumChar( (unsigned char) l[ start-1 ] );
            const bool after = end >= len || !IsAlnumChar( (unsigned char) l[ end ] );
            if ( before && after )
            {
                rebuilt += CORE::CString( result.C_String() + copied, start - copied );
                rebuilt += marker;
                copied = end;
                found = true;
                pos = end;
            }
            else
                pos = start + 1;
        }
        if ( found )
        {
            rebuilt += CORE::CString( result.C_String() + copied, ByteLength( result ) - copied );
            result = rebuilt;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::MaskValues( const CORE::CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() || m_journalMode )
        return text;
    const CORE::CString dictionaryMasked = MaskDictionary( text, ValueMarker );
    const char* t = dictionaryMasked.C_String();
    const UInt32 len = ByteLength( dictionaryMasked );
    CORE::CString result;
    UInt32 i = 0;
    while ( i < len )
    {
        UInt32 placeholderEnd = 0;
        if ( IsPlaceholderAt( t, len, i, placeholderEnd ) )
        {
            // a placeholder of ours stays as it is
            result += CORE::CString( t + i, placeholderEnd - i );
            i = placeholderEnd;
            continue;
        }
        if ( IsAlnumChar( (unsigned char) t[ i ] ) )
        {
            UInt32 end = i;
            while ( end < len && IsAlnumChar( (unsigned char) t[ end ] ) )
                ++end;
            const CORE::CString token( t + i, end - i );
            result += IsSensitiveToken( token ) ? ValueMarker : token;
            i = end;
            continue;
        }
        result += t[ i ];
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsSensitiveValue( const CORE::CString& value ) const
{GUCEF_TRACE;

    return !value.IsNULLOrEmpty() && MaskValues( value ) != value;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactMemberName( const CORE::CString& name ) const
{GUCEF_TRACE;

    const UInt32 len = ByteLength( name );
    if ( 0 == len || IsPlaceholderText( name ) )
        return name;
    if ( IsPlausibleSecret( name ) || ContainsKnownSecret( name ) )
        return KeyMarker;
    if ( m_settings.maskIdLikeMemberNames )
    {
        bool allDigits = true;
        for ( UInt32 i=0; i<len && allDigits; ++i )
            allDigits = name.C_String()[ i ] >= '0' && name.C_String()[ i ] <= '9';
        const CORE::CString templated = CUrlPathTemplate::TemplateSegment( name );
        if ( allDigits || templated == "{n}" || templated == "{guid}" || templated == "{hex}" )
            return KeyMarker;
    }
    if ( IsSensitiveValue( name ) )
        return KeyMarker;
    return name;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactShapePath( const CORE::CString& path ) const
{GUCEF_TRACE;

    // dictionary entries that hold a dot span segments
    const CORE::CString whole = MaskDictionary( path, KeyMarker );
    CORE::CString::StringVector segments = whole.ParseElements( '.', true );
    CORE::CString result;
    for ( size_t i=0; i<segments.size(); ++i )
    {
        CORE::CString name = segments[ i ];
        CORE::CString suffix;
        while ( name.Length() >= 2 && name.EndsWith( "[]" ) )
        {
            suffix += "[]";
            name = name.SubstrToIndex( name.Length() - 2, true );
        }
        if ( i > 0 )
            result += '.';
        result += RedactMemberName( name ) + suffix;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactShapeLine( const CORE::CString& line ) const
{GUCEF_TRACE;

    // path: type [= sample], path [= sample], path = sample, path -> template
    const char* t = line.C_String();
    const UInt32 len = ByteLength( line );
    if ( 0 == len )
        return line;
    const char* colon = strstr( t, ": " );
    const char* equals = strstr( t, " = " );
    const char* arrow = strstr( t, " -> " );
    const char* split = GUCEF_NULL;
    if ( GUCEF_NULL != colon )
        split = colon;
    if ( GUCEF_NULL != equals && ( GUCEF_NULL == split || equals < split ) )
        split = equals;
    if ( GUCEF_NULL != arrow && ( GUCEF_NULL == split || arrow < split ) )
        split = arrow;
    if ( GUCEF_NULL == split )
        return RedactShapePath( line );
    const UInt32 pos = (UInt32) ( split - t );
    const CORE::CString path( t, pos );
    const CORE::CString rest( t + pos, len - pos );
    if ( split == arrow )
        return RedactShapePath( path ) + " -> " + RedactAddressTemplate( CORE::CString( t + pos + 4, len - pos - 4 ) );
    return RedactShapePath( path ) + MaskValues( rest );
}

/*-------------------------------------------------------------------------*/

static bool
RedactLines( const CSiteDiscoveryRedactor& redactor, CORE::CString::StringVector& lines )
{
    bool changed = false;
    CORE::CString::StringVector out;
    CORE::CString::StringSet seen;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = redactor.RedactShapeLine( lines[ i ] );
        if ( line != lines[ i ] )
            changed = true;
        if ( seen.find( line ) != seen.end() )
        {
            changed = true;     // two values became the same placeholder
            continue;
        }
        seen.insert( line );
        out.push_back( line );
    }
    if ( changed )
        lines.swap( out );
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::RedactShape( CWebBodyShape& shape ) const
{GUCEF_TRACE;

    bool changed = RedactLines( *this, shape.lines );
    changed = RedactLines( *this, shape.paginationHints ) || changed;
    changed = RedactLines( *this, shape.urlFields ) || changed;
    return changed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    const char* p = path.C_String();
    const UInt32 len = ByteLength( path );
    if ( 0 == len )
        return path;
    CORE::CString result;
    CORE::CString previous;
    UInt32 start = 0;
    while ( true )
    {
        UInt32 end = start;
        while ( end < len && p[ end ] != '/' )
            ++end;
        const CORE::CString segment( p + start, end - start );
        CORE::CString masked = segment;
        if ( !segment.IsNULLOrEmpty() && !IsPlaceholderText( segment ) )
        {
            if ( !previous.IsNULLOrEmpty() && IsInList( m_settings.valuePathSegmentsAfter, CUrlPathTemplate::PercentDecode( previous ) ) )
                masked = ValueMarker;
            else
            if ( IsSensitiveValue( CUrlPathTemplate::PercentDecode( segment ) ) )
                masked = ValueMarker;
        }
        result += masked;
        previous = segment;
        if ( end >= len )
            break;
        result += '/';
        start = end + 1;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactAddressTemplate( const CORE::CString& address ) const
{GUCEF_TRACE;

    const char* a = address.C_String();
    const UInt32 len = ByteLength( address );
    if ( 0 == len )
        return address;
    UInt32 hostStart = 0;
    const char* scheme = strstr( a, "://" );
    if ( GUCEF_NULL != scheme )
        hostStart = (UInt32) ( scheme - a ) + 3;
    const char* slash = strchr( a + hostStart, '/' );
    if ( GUCEF_NULL == slash )
        return address;
    const UInt32 pathStart = (UInt32) ( slash - a );
    UInt32 pathEnd = pathStart;
    while ( pathEnd < len && a[ pathEnd ] != '?' && a[ pathEnd ] != '#' )
        ++pathEnd;
    return CORE::CString( a, pathStart ) + RedactPath( CORE::CString( a + pathStart, pathEnd - pathStart ) ) + CORE::CString( a + pathEnd, len - pathEnd );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsTokenLikeParameterName( const CORE::CString& decodedName ) const
{GUCEF_TRACE;

    const UInt32 len = ByteLength( decodedName );
    if ( 0 == len || IsPlaceholderText( decodedName ) )
        return false;
    if ( IsPlausibleSecret( decodedName ) || ContainsKnownSecret( decodedName ) )
        return true;
    if ( !m_settings.maskTokenLikeParameterNames )
        return false;
    if ( len > m_settings.maxParameterNameLength )
        return true;

    // a run of base64 characters (also the URL safe ones) that mixes capitals, small letters and digits, or ends in padding
    const char* t = decodedName.C_String();
    UInt32 i = 0;
    while ( i < len )
    {
        UInt32 end = i;
        bool upper = false, lower = false, digit = false;
        while ( end < len && ( IsAlnumChar( (unsigned char) t[ end ] ) || t[ end ] == '+' || t[ end ] == '/' || t[ end ] == '_' || t[ end ] == '-' || t[ end ] == '=' ) )
        {
            const char c = t[ end ];
            if ( c >= 'A' && c <= 'Z' ) upper = true;
            else if ( c >= 'a' && c <= 'z' ) lower = true;
            else if ( c >= '0' && c <= '9' ) digit = true;
            ++end;
        }
        const UInt32 run = end - i;
        if ( run >= 16 && upper && lower && digit )
            return true;
        if ( run >= 8 && t[ end-1 ] == '=' && ( upper || digit ) )
            return true;
        i = end > i ? end : i + 1;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactQueryName( const CORE::CString& decodedName ) const
{GUCEF_TRACE;

    return IsTokenLikeParameterName( decodedName ) ? TokenLikeNameMarker : decodedName;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsValueQueryName( const CORE::CString& decodedName ) const
{GUCEF_TRACE;

    return IsInList( m_settings.valueQueryNames, decodedName );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::IsEnumQueryName( const CORE::CString& decodedName ) const
{GUCEF_TRACE;

    if ( decodedName.IsNULLOrEmpty() || IsValueQueryName( decodedName ) || IsSecretName( decodedName ) || IsTokenLikeParameterName( decodedName ) )
        return false;
    return m_settings.enumQueryNames.empty() || IsInList( m_settings.enumQueryNames, decodedName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::MaskUrl( const CORE::CString& url ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( m_journalMode || !CUrlPathTemplate::Parse( url, parsed ) )
        return url;
    CORE::CString result = parsed.scheme + "://" + parsed.host + RedactPath( parsed.path );
    for ( size_t i=0; i<parsed.queryParams.size(); ++i )
    {
        const CORE::CString& rawName = parsed.queryParams[ i ].first;
        const CORE::CString& rawValue = parsed.queryParams[ i ].second;
        const CORE::CString decodedName = CUrlPathTemplate::PercentDecode( rawName );
        result += ( i == 0 ? '?' : '&' );
        result += IsTokenLikeParameterName( decodedName ) ? TokenLikeNameMarker : rawName;
        if ( rawValue.IsNULLOrEmpty() )
            continue;
        result += '=';
        if ( IsPlaceholderText( rawValue ) )
            result += rawValue;
        else
        if ( IsValueQueryName( decodedName ) || IsSensitiveValue( CUrlPathTemplate::PercentDecode( rawValue ) ) )
            result += ValueMarker;
        else
            result += rawValue;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryRedactor::RedactFileName( const CORE::CString& fileName ) const
{GUCEF_TRACE;

    // a name holding a registered secret is left whole, for the tripwire to replace its line: templating would leave part of the secret
    if ( fileName.IsNULLOrEmpty() || ContainsKnownSecret( fileName ) )
        return fileName;

    // values of the user, then digit runs, outside placeholders
    const CORE::CString masked = MaskValues( fileName );
    const char* t = masked.C_String();
    const UInt32 len = ByteLength( masked );
    CORE::CString templated;
    UInt32 i = 0;
    while ( i < len )
    {
        UInt32 placeholderEnd = 0;
        if ( IsPlaceholderAt( t, len, i, placeholderEnd ) )
        {
            templated += CORE::CString( t + i, placeholderEnd - i );
            i = placeholderEnd;
            continue;
        }
        if ( t[ i ] >= '0' && t[ i ] <= '9' )
        {
            while ( i < len && t[ i ] >= '0' && t[ i ] <= '9' )
                ++i;
            templated += "{n}";
            continue;
        }
        templated += t[ i ];
        ++i;
    }
    if ( !m_settings.maskFileNameWords )
        return templated;

    // the extension stays: a dot followed by 1 to 5 letters and digits at the end
    const char* n = templated.C_String();
    const UInt32 nlen = ByteLength( templated );
    UInt32 baseLength = nlen;
    for ( UInt32 e=nlen; e>0; --e )
    {
        if ( n[ e-1 ] == '.' )
        {
            const UInt32 extLength = nlen - e;
            bool ok = extLength >= 1 && extLength <= 5 && e > 1;
            for ( UInt32 c=e; ok && c<nlen; ++c )
                ok = IsAlnumChar( (unsigned char) n[ c ] );
            if ( ok )
                baseLength = e - 1;
            break;
        }
        if ( !IsAlnumChar( (unsigned char) n[ e-1 ] ) )
            break;
    }

    // items: placeholders, words (letters and digits) and separators
    TFileNameItemVector items;
    UInt32 k = 0;
    while ( k < baseLength )
    {
        SFileNameItem item;
        UInt32 end = k + 1;
        UInt32 placeholderEnd = 0;
        if ( IsPlaceholderAt( n, baseLength, k, placeholderEnd ) )
        {
            end = placeholderEnd;
            item.kind = 2;
        }
        else
        if ( IsAlnumChar( (unsigned char) n[ k ] ) )
        {
            while ( end < baseLength && IsAlnumChar( (unsigned char) n[ end ] ) )
                ++end;
            item.kind = 1;
        }
        else
            item.kind = 0;
        item.text = CORE::CString( n + k, end - k );
        items.push_back( item );
        k = end;
    }
    CORE::CString result;
    size_t previousPlaceholder = items.size();
    for ( size_t m=0; m<items.size(); ++m )
    {
        if ( items[ m ].kind != 2 )
            continue;
        if ( previousPlaceholder == items.size() )
        {
            for ( size_t c=0; c<=m; ++c )
                result += items[ c ].text;
        }
        else
        {
            // between two masked parts: the separators at either end stay, any word in between makes it {text}
            size_t first = previousPlaceholder + 1;
            size_t last = m;      // exclusive
            CORE::CString lead, trail;
            while ( first < last && items[ first ].kind == 0 )
                lead += items[ first++ ].text;
            CORE::CString::StringVector trailing;
            while ( last > first && items[ last-1 ].kind == 0 )
                trailing.insert( trailing.begin(), items[ --last ].text );
            for ( size_t c=0; c<trailing.size(); ++c )
                trail += trailing[ c ];
            bool anyWord = false;
            CORE::CString middle;
            for ( size_t c=first; c<last; ++c )
            {
                middle += items[ c ].text;
                if ( items[ c ].kind == 1 )
                    anyWord = true;
            }
            result += lead + ( anyWord ? TextMarker : middle ) + trail + items[ m ].text;
        }
        previousPlaceholder = m;
    }
    if ( previousPlaceholder == items.size() )
        return templated;
    for ( size_t c=previousPlaceholder+1; c<items.size(); ++c )
        result += items[ c ].text;
    return result + CORE::CString( n + baseLength, nlen - baseLength );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::RedactEnumValues( CObservedWebRequest::TStringPairVector& values ) const
{GUCEF_TRACE;

    bool changed = false;
    CObservedWebRequest::TStringPairVector kept;
    for ( size_t i=0; i<values.size(); ++i )
    {
        if ( !IsEnumQueryName( values[ i ].first ) )
        {
            changed = true;
            continue;
        }
        const CORE::CString value = IsSensitiveValue( values[ i ].second ) ? ValueMarker : values[ i ].second;
        if ( value != values[ i ].second )
            changed = true;
        kept.push_back( std::make_pair( values[ i ].first, value ) );
    }
    if ( changed )
        values.swap( kept );
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::RedactObservation( CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    bool changed = false;
    const CORE::CString url = MaskUrl( observation.url );
    if ( url != observation.url )
    {
        observation.url = url;
        changed = true;
    }
    for ( CObservedWebRequest::TStringMap::iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
    {
        if ( h->second.IsNULLOrEmpty() || h->second == NotRecordedMarker )
            continue;
        if ( h->first == "referer" || h->first == "origin" || h->first == "location" )
        {
            const CORE::CString masked = MaskUrl( h->second );
            if ( masked != h->second )
            {
                h->second = masked;
                changed = true;
            }
        }
    }
    changed = RedactShape( observation.requestBodyShape ) || changed;
    if ( observation.hasResponseShape )
        changed = RedactShape( observation.responseShape ) || changed;
    const CORE::CString fileName = RedactFileName( observation.downloadFileName );
    if ( fileName != observation.downloadFileName )
    {
        observation.downloadFileName = fileName;
        changed = true;
    }
    const CORE::CString redirect = RedactAddressTemplate( observation.redirectTarget );
    if ( redirect != observation.redirectTarget )
    {
        observation.redirectTarget = redirect;
        changed = true;
    }
    changed = RedactEnumValues( observation.queryEnumValues ) || changed;
    if ( !observation.postData.IsNULLOrEmpty() )
    {
        const CORE::CString postData = MaskValues( observation.postData );
        if ( postData != observation.postData )
        {
            observation.postData = postData;
            changed = true;
        }
    }
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryRedactor::RedactAction( CObservedUserAction& action ) const
{GUCEF_TRACE;

    bool changed = false;
    const CORE::CString label = RedactLabel( action.label );
    if ( label != action.label )
    {
        action.label = label;
        changed = true;
    }
    const CORE::CString page = RedactAddressTemplate( action.page );
    if ( page != action.page )
    {
        action.page = page;
        changed = true;
    }
    return changed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
