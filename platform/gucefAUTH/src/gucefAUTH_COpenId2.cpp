/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_COpenId2.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 COpenId2NonceCache::MaxEntries;
const UInt32 COpenId2Assertion::MaxFieldBytes;
const UInt32 COpenId2Assertion::MaxExtraFields;
const UInt32 COpenId2Relay::MinPort;
const UInt32 COpenId2Relay::MinStateBytes;
const UInt32 COpenId2Relay::MaxStateBytes;
const UInt32 COpenId2Relay::MaxUrlBytes;

const CORE::CString COpenId2Relay::PortParameter = "port";
const CORE::CString COpenId2Relay::StateParameter = "s";
const CORE::CString COpenId2Relay::CallbackPath = "/callback";

const CORE::CString COpenId2Assertion::Namespace = "http://specs.openid.net/auth/2.0";
const CORE::CString COpenId2Assertion::IdentifierSelect = "http://specs.openid.net/auth/2.0/identifier_select";

/** The fields of a positive assertion this module knows and needs. Other openid.* fields (extensions) are kept as well, see COpenId2Assertion::Load() */
static const char* const g_knownFields[] =
{
    "openid.ns", "openid.mode", "openid.op_endpoint", "openid.claimed_id", "openid.identity",
    "openid.return_to", "openid.response_nonce", "openid.assoc_handle", "openid.signed", "openid.sig"
};

/** What the provider has to have signed */
static const char* const g_requiredSigned[] =
{
    "op_endpoint", "claimed_id", "identity", "return_to", "response_nonce", "assoc_handle"
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
OpenId2StatusToString( TOpenId2Status status )
{GUCEF_TRACE;

    switch ( status )
    {
        case OPENID2_OK :                   return "ok";
        case OPENID2_MALFORMED :            return "malformed";
        case OPENID2_WRONG_ENDPOINT :       return "wrong-endpoint";
        case OPENID2_BAD_CLAIMED_ID :       return "bad-claimed-id";
        case OPENID2_BAD_RETURN_TO :        return "bad-return-to";
        case OPENID2_UNSIGNED_FIELD :       return "unsigned-field";
        case OPENID2_BAD_NONCE :            return "bad-nonce";
        case OPENID2_STALE_NONCE :          return "stale-nonce";
        case OPENID2_REPLAYED_NONCE :       return "replayed-nonce";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------*/

COpenId2NonceCache::COpenId2NonceCache( void )
    : m_lock()
    , m_nonces()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpenId2NonceCache::~COpenId2NonceCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
COpenId2NonceCache::Reserve( const CORE::CString& nonce, UInt64 nowSeconds, UInt32 keepForSeconds )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    // Forget the ones that cannot matter any more
    TNonceMap::iterator i = m_nonces.begin();
    while ( i != m_nonces.end() )
    {
        if ( (*i).second <= nowSeconds )
            m_nonces.erase( i++ );
        else
            ++i;
    }

    if ( m_nonces.find( nonce ) != m_nonces.end() )
        return false;
    if ( m_nonces.size() >= MaxEntries )
    {
        // Full of fresh nonces: refuse rather than forget one that could be replayed
        return false;
    }
    m_nonces[ nonce ] = nowSeconds + keepForSeconds;
    return true;
}

/*-------------------------------------------------------------------------*/

void
COpenId2NonceCache::Release( const CORE::CString& nonce )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_nonces.erase( nonce );
}

/*-------------------------------------------------------------------------*/

void
COpenId2NonceCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_nonces.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
COpenId2NonceCache::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_nonces.size();
}

/*-------------------------------------------------------------------------*/

COpenId2Assertion::COpenId2Assertion( void )
    : fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
COpenId2Assertion::Load( const CORE::CString::StringMap& parameters )
{GUCEF_TRACE;

    fields.clear();
    for ( size_t i=0; i<sizeof( g_knownFields ) / sizeof( g_knownFields[ 0 ] ); ++i )
    {
        CORE::CString::StringMap::const_iterator f = parameters.find( g_knownFields[ i ] );
        if ( f == parameters.end() )
        {
            fields.clear();
            return false;
        }
        if ( GetByteLength( (*f).second ) > MaxFieldBytes || (*f).second.IsNULLOrEmpty() )
        {
            fields.clear();
            return false;
        }
        fields[ g_knownFields[ i ] ] = (*f).second;
    }

    // check_authentication asks for exact copies of all fields of the response (OpenID 2.0, 11.4.2.1): the signature
    // covers the fields the provider listed in openid.signed, which can include fields of extensions. Nothing but openid.*
    // fields of a sane form and size is kept
    UInt32 extra = 0;
    for ( CORE::CString::StringMap::const_iterator p = parameters.begin(); p != parameters.end(); ++p )
    {
        const CORE::CString& name = (*p).first;
        if ( fields.find( name ) != fields.end() || !name.StartsWith( "openid." ) )
            continue;
        UInt32 size = GetByteLength( name );
        if ( size > 64 || ++extra > MaxExtraFields || GetByteLength( (*p).second ) > MaxFieldBytes )
        {
            fields.clear();
            return false;
        }
        const char* chars = name.C_String();
        for ( UInt32 i=0; i<size; ++i )
        {
            char c = chars[ i ];
            if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c ) )
            {
                fields.clear();
                return false;
            }
        }
        fields[ name ] = (*p).second;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenId2Assertion::ToQueryString( void ) const
{GUCEF_TRACE;

    return CAuthUrl::EncodeForm( fields );
}

/*-------------------------------------------------------------------------*/

bool
COpenId2Assertion::LoadFromQueryString( const CORE::CString& query )
{GUCEF_TRACE;

    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !CAuthUrl::ParseQuery( query, parameters, duplicate ) || duplicate )
    {
        fields.clear();
        return false;
    }
    return Load( parameters );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenId2Assertion::Get( const CORE::CString& field ) const
{GUCEF_TRACE;

    CORE::CString::StringMap::const_iterator f = fields.find( field );
    return f == fields.end() ? CORE::CString::Empty : (*f).second;
}

/*-------------------------------------------------------------------------*/

static bool
IsDigits( const char* p, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( p[ i ] < '0' || p[ i ] > '9' )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static UInt32
ToNumber( const char* p, UInt32 count )
{
    UInt32 value = 0;
    for ( UInt32 i=0; i<count; ++i )
        value = value * 10 + (UInt32) ( p[ i ] - '0' );
    return value;
}

/*-------------------------------------------------------------------------*/

/** The characters an address of a relay page is made of: no escapes, no user information, no query, no fragment */
static bool
IsRelayUrlChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
           '-' == c || '.' == c || '_' == c || '~' == c || ':' == c || '/' == c;
}

/*-------------------------------------------------------------------------*/

/** A host name for a relay: labels of a-z 0-9 - at least two, the last one not only digits (so no IPv4 address) */
static bool
IsRelayHostName( const CORE::CString& host )
{
    UInt32 size = GetByteLength( host );
    if ( 0 == size || size > 253 )
        return false;
    const char* p = host.C_String();
    UInt32 labels = 0;
    UInt32 labelStart = 0;
    bool lastAllDigits = true;
    for ( UInt32 i=0; i<=size; ++i )
    {
        if ( i == size || '.' == p[ i ] )
        {
            UInt32 length = i - labelStart;
            if ( 0 == length || length > 63 || '-' == p[ labelStart ] || '-' == p[ i - 1 ] )
                return false;
            ++labels;
            if ( i == size )
                break;
            labelStart = i + 1;
            lastAllDigits = true;
            continue;
        }
        char c = p[ i ];
        bool digit = c >= '0' && c <= '9';
        if ( !( digit || ( c >= 'a' && c <= 'z' ) || '-' == c ) )
            return false;
        if ( !digit )
            lastAllDigits = false;
    }
    return labels >= 2 && !lastAllDigits;
}

/*-------------------------------------------------------------------------*/

bool
COpenId2Relay::ParseRelayUrl( const CORE::CString& url, bool allowInsecure, CORE::CString& baseUrl, CORE::CString& realm, CORE::CString& reason )
{GUCEF_TRACE;

    baseUrl.Clear();
    realm.Clear();
    reason.Clear();

    UInt32 size = GetByteLength( url );
    if ( 0 == size )
    {
        reason = "The relay address is empty";
        return false;
    }
    if ( size > MaxUrlBytes )
    {
        reason = "The relay address is too long";
        return false;
    }
    const char* p = url.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( !IsRelayUrlChar( p[ i ] ) )
        {
            reason = "The relay address has a character that is not allowed: only letters, digits and - . _ ~ : / are (no user information, query, fragment, escapes or non ASCII text)";
            return false;
        }
    }

    // The scheme
    CORE::CString lower = url.Lowercase();
    bool secure = lower.StartsWith( "https://" );
    if ( !secure && !lower.StartsWith( "http://" ) )
    {
        reason = "The relay address has to start with https://";
        return false;
    }
    UInt32 schemeSize = secure ? 8 : 7;

    // The authority and the path
    UInt32 authorityEnd = schemeSize;
    while ( authorityEnd < size && '/' != p[ authorityEnd ] )
        ++authorityEnd;
    CORE::CString authority = lower.SubstrFromRange( schemeSize, authorityEnd );
    CORE::CString path = authorityEnd < size ? MakeString( p + authorityEnd, size - authorityEnd ) : CORE::CString( "/" );

    // The host and the port
    CORE::CString host = authority;
    UInt32 port = 0;
    Int32 colon = authority.HasChar( ':' );
    if ( colon >= 0 )
    {
        host = authority.SubstrToIndex( (UInt32) colon, true );
        CORE::CString portText = authority.CutChars( (UInt32) colon + 1, true );
        UInt32 portSize = GetByteLength( portText );
        if ( 0 == portSize || portSize > 5 || portText.HasChar( ':' ) >= 0 || '0' == portText.C_String()[ 0 ] || !IsDigits( portText.C_String(), portSize ) )
        {
            reason = "The port of the relay address is not a number";
            return false;
        }
        port = ToNumber( portText.C_String(), portSize );
        if ( port > 65535 )
        {
            reason = "The port of the relay address is not a number";
            return false;
        }
    }
    if ( host.IsNULLOrEmpty() )
    {
        reason = "The relay address has no host";
        return false;
    }

    bool loopback = allowInsecure && CAuthUrl::IsLoopbackHost( host );
    if ( !secure && !loopback )
    {
        reason = "The relay address has to be https";
        return false;
    }
    if ( !loopback && !IsRelayHostName( host ) )
    {
        reason = "The host of the relay address has to be a domain name";
        return false;
    }
    if ( secure && 0 != port && 443 != port && !allowInsecure )
    {
        reason = "The relay address can only use port 443";
        return false;
    }

    // The path: plain segments only
    if ( path.HasChar( ':' ) >= 0 || path.HasSubstr( ".." ) >= 0 || path.HasSubstr( "//" ) >= 0 || path.HasSubstr( "/./" ) >= 0 || path.EndsWith( "/." ) )
    {
        reason = "The path of the relay address is not plain";
        return false;
    }

    CORE::CString origin = ( secure ? CORE::CString( "https://" ) : CORE::CString( "http://" ) ) + host;
    if ( 0 != port && !( secure && 443 == port ) && !( !secure && 80 == port ) )
        origin += ":" + CORE::ToString( port );
    baseUrl = origin + path;
    realm = origin + "/";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenId2Relay::IsValidState( const CORE::CString& state )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( state );
    if ( size < MinStateBytes || size > MaxStateBytes )
        return false;
    const char* p = state.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = p[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '_' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenId2Relay::BuildReturnTo( const CORE::CString& baseUrl, UInt16 port, const CORE::CString& state )
{GUCEF_TRACE;

    CORE::CString::StringMap parameters;
    parameters[ PortParameter ] = CORE::ToString( port );
    parameters[ StateParameter ] = state;
    return CAuthUrl::AppendQuery( baseUrl, parameters );
}

/*-------------------------------------------------------------------------*/

bool
COpenId2Relay::IsRelayReturnTo( const CORE::CString& returnTo, const CORE::CString& baseUrl )
{GUCEF_TRACE;

    UInt32 baseSize = GetByteLength( baseUrl );
    UInt32 size = GetByteLength( returnTo );
    if ( 0 == baseSize || size <= baseSize + 1 || size > CAuthUrl::MaxUrlBytes )
        return false;
    if ( 0 != memcmp( returnTo.C_String(), baseUrl.C_String(), baseSize ) || '?' != returnTo.C_String()[ baseSize ] )
        return false;

    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !CAuthUrl::ParseQuery( MakeString( returnTo.C_String() + baseSize + 1, size - baseSize - 1 ), parameters, duplicate ) || duplicate || 2 != parameters.size() )
        return false;
    CORE::CString::StringMap::const_iterator port = parameters.find( PortParameter );
    CORE::CString::StringMap::const_iterator state = parameters.find( StateParameter );
    if ( port == parameters.end() || state == parameters.end() || !IsValidState( (*state).second ) )
        return false;
    UInt32 portSize = GetByteLength( (*port).second );
    const char* digits = (*port).second.C_String();
    if ( portSize < 4 || portSize > 5 || '0' == digits[ 0 ] || !IsDigits( digits, portSize ) )
        return false;
    UInt32 value = ToNumber( digits, portSize );
    if ( value < MinPort || value > 65535 )
        return false;

    // Only the form the application makes itself: no other spelling of the same query
    return returnTo == BuildReturnTo( baseUrl, (UInt16) value, (*state).second );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenId2::BuildAuthenticationUrl( const CORE::CString& endpoint, const CORE::CString& returnTo, const CORE::CString& realm )
{GUCEF_TRACE;

    CORE::CString::StringMap parameters;
    parameters[ "openid.ns" ] = COpenId2Assertion::Namespace;
    parameters[ "openid.mode" ] = "checkid_setup";
    parameters[ "openid.return_to" ] = returnTo;
    parameters[ "openid.realm" ] = realm;
    parameters[ "openid.identity" ] = COpenId2Assertion::IdentifierSelect;
    parameters[ "openid.claimed_id" ] = COpenId2Assertion::IdentifierSelect;
    return CAuthUrl::AppendQuery( endpoint, parameters );
}

/*-------------------------------------------------------------------------*/

bool
COpenId2::ParseNonceTime( const CORE::CString& nonce, UInt64& epochSeconds )
{GUCEF_TRACE;

    // 2010-05-12T20:31:54Z followed by any text
    UInt32 size = GetByteLength( nonce );
    const char* p = nonce.C_String();
    if ( size < 20 || size > 255 || '-' != p[ 4 ] || '-' != p[ 7 ] || 'T' != p[ 10 ] || ':' != p[ 13 ] || ':' != p[ 16 ] || 'Z' != p[ 19 ] )
        return false;
    if ( !IsDigits( p, 4 ) || !IsDigits( p + 5, 2 ) || !IsDigits( p + 8, 2 ) || !IsDigits( p + 11, 2 ) || !IsDigits( p + 14, 2 ) || !IsDigits( p + 17, 2 ) )
        return false;
    UInt32 year = ToNumber( p, 4 ), month = ToNumber( p + 5, 2 ), day = ToNumber( p + 8, 2 );
    UInt32 hour = ToNumber( p + 11, 2 ), minute = ToNumber( p + 14, 2 ), second = ToNumber( p + 17, 2 );
    if ( year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 60 )
        return false;

    // Days since 1970-01-01 of a proleptic Gregorian date
    Int64 y = (Int64) year - ( month <= 2 ? 1 : 0 );
    Int64 era = ( y >= 0 ? y : y - 399 ) / 400;
    Int64 yoe = y - era * 400;
    Int64 mp = ( (Int64) month + 9 ) % 12;
    Int64 doy = ( 153 * mp + 2 ) / 5 + (Int64) day - 1;
    Int64 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    Int64 days = era * 146097 + doe - 719468;
    epochSeconds = (UInt64) ( days * 86400 + (Int64) hour * 3600 + (Int64) minute * 60 + (Int64) second );
    return true;
}

/*-------------------------------------------------------------------------*/

TOpenId2Status
COpenId2::Validate( const COpenId2Assertion& assertion  ,
                    const CORE::CString& endpoint       ,
                    const IOpenId2ProviderProfile& profile,
                    const CORE::CString& expectedReturnTo,
                    const CORE::CString& loopbackPath   ,
                    UInt64 nowSeconds                   ,
                    UInt32 maxAgeSeconds                ,
                    COpenId2NonceCache& nonces          ,
                    const CORE::CString& relayBase      )
{GUCEF_TRACE;

    // The fields this module needs are there (Load() takes nothing less); the fields of extensions are there as well, unsigned ones are not looked at
    if ( assertion.fields.size() < sizeof( g_knownFields ) / sizeof( g_knownFields[ 0 ] ) )
        return OPENID2_MALFORMED;
    if ( assertion.Get( "openid.ns" ) != COpenId2Assertion::Namespace || assertion.Get( "openid.mode" ) != "id_res" )
        return OPENID2_MALFORMED;
    if ( assertion.Get( "openid.op_endpoint" ) != endpoint )
        return OPENID2_WRONG_ENDPOINT;

    CORE::CString subject;
    if ( !profile.ExtractSubject( endpoint, assertion.Get( "openid.claimed_id" ), assertion.Get( "openid.identity" ), subject ) || subject.IsNULLOrEmpty() )
        return OPENID2_BAD_CLAIMED_ID;

    // The return_to: the one that was sent, or on the loopback address with the path of the application. With a relay it is
    // always a page of the relay with the query of the relay, and the relay is not trusted with anything else
    CORE::CString returnTo = assertion.Get( "openid.return_to" );
    if ( !expectedReturnTo.IsNULLOrEmpty() )
    {
        if ( returnTo != expectedReturnTo )
            return OPENID2_BAD_RETURN_TO;
        if ( !relayBase.IsNULLOrEmpty() && !COpenId2Relay::IsRelayReturnTo( returnTo, relayBase ) )
            return OPENID2_BAD_RETURN_TO;
    }
    else
    if ( !relayBase.IsNULLOrEmpty() )
    {
        if ( !COpenId2Relay::IsRelayReturnTo( returnTo, relayBase ) )
            return OPENID2_BAD_RETURN_TO;
    }
    else
    {
        CORE::CString scheme, host, path, query;
        UInt16 port = 0;
        if ( !CAuthUrl::Split( returnTo, scheme, host, port, path, query ) || scheme != "http" || !CAuthUrl::IsLoopbackHost( host ) || path != loopbackPath )
            return OPENID2_BAD_RETURN_TO;
    }

    // What has to be signed
    CORE::CString::StringVector signedFields = assertion.Get( "openid.signed" ).ParseElements( ',', false );
    for ( size_t i=0; i<sizeof( g_requiredSigned ) / sizeof( g_requiredSigned[ 0 ] ); ++i )
        if ( !ContainsString( signedFields, g_requiredSigned[ i ] ) )
            return OPENID2_UNSIGNED_FIELD;

    // The nonce: a time, fresh, and not seen before
    CORE::CString nonce = assertion.Get( "openid.response_nonce" );
    UInt64 nonceTime = 0;
    if ( !ParseNonceTime( nonce, nonceTime ) )
        return OPENID2_BAD_NONCE;
    if ( nonceTime + maxAgeSeconds < nowSeconds || nonceTime > nowSeconds + 120 )
        return OPENID2_STALE_NONCE;
    if ( !nonces.Reserve( nonce, nowSeconds, maxAgeSeconds + 240 ) )
        return OPENID2_REPLAYED_NONCE;
    return OPENID2_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenId2::BuildCheckAuthenticationBody( const COpenId2Assertion& assertion )
{GUCEF_TRACE;

    CORE::CString::StringMap fields = assertion.fields;
    fields[ "openid.mode" ] = "check_authentication";
    return CAuthUrl::EncodeForm( fields );
}

/*-------------------------------------------------------------------------*/

bool
COpenId2::IsValidAnswer( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || 0 == size || size > 4096 )
        return false;
    CORE::CString text = MakeString( static_cast< const char* >( data ), size );
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    bool valid = false;
    bool invalid = false;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ].RemoveChar( '\r' );
        if ( line == "is_valid:true" )
            valid = true;
        else
        if ( line.StartsWith( "is_valid:" ) )
            invalid = true;
    }
    return valid && !invalid;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
