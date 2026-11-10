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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#include "gucefWEB_CMcpStaticBearerAuthenticator.h"

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

/** token68 of RFC 9110 11.2: the syntax of a bearer token (RFC 6750 2.1) */
static bool
IsToken68( const char* text, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return false;
    UInt32 i = 0;
    for ( ; i<size; ++i )
    {
        const char c = text[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
             '-' == c || '.' == c || '_' == c || '~' == c || '+' == c || '/' == c )
            continue;
        break;
    }
    if ( 0 == i )
        return false;
    for ( ; i<size; ++i )
        if ( '=' != text[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpStaticBearerAuthenticator::CMcpStaticBearerAuthenticator( void )
    : CIMcpHttpAuthenticator()
    , m_token()
    , m_tokenFile()
    , m_tokenLabel( "default" )
    , m_scopes()
    , m_prepared( false )
{GUCEF_TRACE;

    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
}

/*-------------------------------------------------------------------------*/

CMcpStaticBearerAuthenticator::~CMcpStaticBearerAuthenticator()
{GUCEF_TRACE;

    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
}

/*-------------------------------------------------------------------------*/

void CMcpStaticBearerAuthenticator::SetToken( const CORE::CString& token ) {GUCEF_TRACE; m_token = token; m_prepared = false; }
CORE::CString CMcpStaticBearerAuthenticator::GetToken( void ) const {GUCEF_TRACE; return m_token; }
void CMcpStaticBearerAuthenticator::SetTokenFile( const CORE::CString& tokenFile ) {GUCEF_TRACE; m_tokenFile = tokenFile; }
const CORE::CString& CMcpStaticBearerAuthenticator::GetTokenFile( void ) const {GUCEF_TRACE; return m_tokenFile; }
void CMcpStaticBearerAuthenticator::SetTokenLabel( const CORE::CString& label ) {GUCEF_TRACE; m_tokenLabel = label; }
void CMcpStaticBearerAuthenticator::SetScopes( const CORE::CString::StringVector& scopes ) {GUCEF_TRACE; m_scopes = scopes; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStaticBearerAuthenticator::GetAuthenticatorName( void ) const
{GUCEF_TRACE;

    return StaticBearerAuthenticatorName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStaticBearerAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CMcpStaticBearerAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::Prepare( CORE::CString& problem )
{GUCEF_TRACE;

    m_prepared = false;
    if ( m_token.IsNULLOrEmpty() && !m_tokenFile.IsNULLOrEmpty() )
    {
        // read only when nobody else can have put it there or read it; never through a symbolic link
        CORE::CDynamicBuffer content;
        CORE::CString fileProblem;
        bool missing = false;
        if ( CORE::ReadOwnerOnlyFile( m_tokenFile, content, fileProblem, &missing ) )
        {
            if ( content.GetDataSize() > 0 )
            {
                CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
                m_token = text.Trim( true ).Trim( false );
            }
        }
        else
        if ( !missing )
        {
            problem = "the bearer token file is refused: " + fileProblem;
            return false;
        }
        else
        {
            // a first start: a random token, in a new file that is the owner's only from its creation on
            UInt8 random[ 32 ];
            if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
            {
                problem = "no random bytes are available for a token";
                return false;
            }
            CORE::CString generated;
            CORE::Base64UrlEncode( random, sizeof( random ), generated, false );
            ::memset( random, 0, sizeof( random ) );
            if ( !CORE::CreateOwnerOnlyFile( m_tokenFile, generated.C_String(), ( generated.ByteSize() > 0 ? generated.ByteSize() - 1 : 0 ), fileProblem ) )
            {
                problem = "the bearer token file cannot be made: " + fileProblem;
                return false;
            }
            m_token = generated;
        }
    }
    if ( m_token.IsNULLOrEmpty() )
    {
        problem = "a bearer token is required and there is neither a token nor a token file";
        return false;
    }
    if ( !IsToken68( m_token.C_String(), ( m_token.ByteSize() > 0 ? m_token.ByteSize() - 1 : 0 ) ) )
    {
        problem = "the bearer token holds characters a bearer token cannot have";
        return false;
    }
    CORE::CSha256::Hash( m_token.C_String(), ( m_token.ByteSize() > 0 ? m_token.ByteSize() - 1 : 0 ), m_tokenDigest );
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::IsTokenValid( const CORE::CString& given ) const
{GUCEF_TRACE;

    // both sides are hashed first, so that the comparison takes the same time whatever the length of the given token
    UInt8 givenDigest[ 32 ];
    CORE::CSha256::Hash( given.C_String(), ( given.ByteSize() > 0 ? given.ByteSize() - 1 : 0 ), givenDigest );
    const bool equal = CORE::CSha256::ConstantTimeEquals( givenDigest, m_tokenDigest, sizeof( givenDigest ) );
    ::memset( givenDigest, 0, sizeof( givenDigest ) );
    return equal && m_prepared && !given.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::HasTokenInQuery( const CORE::CString& queryText )
{GUCEF_TRACE;

    if ( queryText.IsNULLOrEmpty() )
        return false;
    CORE::CString::StringVector parameters = queryText.ParseElements( '&', false );
    for ( size_t i=0; i<parameters.size(); ++i )
    {
        CORE::CString name = parameters[ i ].SubstrToChar( '=', true );
        if ( name.Equals( "access_token", false ) || name.Equals( "token", false ) || name.Equals( "bearer", false ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::Authenticate( const CHttpRequestData& request ,
                                             COM::CMcpPrincipal& principal   ,
                                             CHttpResponseData& challenge    )
{GUCEF_TRACE;

    if ( HasTokenInQuery( request.requestUriParams ) )
    {
        challenge.statusCode = 400;
        challenge.otherHeaders[ "WWW-Authenticate" ] = "Bearer error=\"invalid_request\", error_description=\"the token must not be in the query\"";
        return false;
    }

    CHttpRequestData::TStringMap::const_iterator header = request.otherHeaders.find( "authorization" );
    if ( header == request.otherHeaders.end() )
    {
        // no credentials at all: the challenge has no error code (RFC 6750 3.1)
        challenge.statusCode = 401;
        challenge.otherHeaders[ "WWW-Authenticate" ] = "Bearer";
        return false;
    }

    // "Bearer" SP token68, the scheme without regard to case. Two Authorization fields were joined with a comma,
    // which no token68 holds, so they are refused
    const CORE::CString& value = (*header).second;
    const char* text = value.C_String();
    const UInt32 size = ( value.ByteSize() > 0 ? value.ByteSize() - 1 : 0 );
    bool accepted = false;
    if ( size > 7 && ' ' == text[ 6 ] && value.SubstrToIndex( 6, true ).Equals( "bearer", false ) )
    {
        UInt32 start = 7;
        while ( start < size && ' ' == text[ start ] )
            ++start;
        if ( IsToken68( text + start, size - start ) )
            accepted = IsTokenValid( CORE::CString( text + start, size - start ) );
    }
    if ( !accepted )
    {
        challenge.statusCode = 401;
        challenge.otherHeaders[ "WWW-Authenticate" ] = "Bearer error=\"invalid_token\"";
        return false;
    }

    principal.subject = "bearer:" + m_tokenLabel;
    principal.isAuthenticated = true;
    principal.scopes = m_scopes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "authenticator", CORE::CVariant( StaticBearerAuthenticatorName ) );
    cfg.SetAttribute( "bearerTokenFile", CORE::CVariant( m_tokenFile ) );
    cfg.SetAttribute( "tokenLabel", CORE::CVariant( m_tokenLabel ) );
    CORE::CString scopes;
    for ( size_t i=0; i<m_scopes.size(); ++i )
    {
        if ( i > 0 )
            scopes += ',';
        scopes += m_scopes[ i ];
    }
    cfg.SetAttribute( "scopes", CORE::CVariant( scopes ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStaticBearerAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_tokenFile = cfg.GetAttributeValueOrChildValueByName( "bearerTokenFile" ).AsString( m_tokenFile, true );
    m_tokenLabel = cfg.GetAttributeValueOrChildValueByName( "tokenLabel" ).AsString( m_tokenLabel, true );
    CORE::CString::StringVector scopes;
    if ( cfg.GetStringListByName( "scopes", scopes ) )
        m_scopes = scopes;
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpNoHttpAuthenticator::CMcpNoHttpAuthenticator( void )
    : CIMcpHttpAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpNoHttpAuthenticator::~CMcpNoHttpAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpNoHttpAuthenticator::GetAuthenticatorName( void ) const
{GUCEF_TRACE;

    return NoAuthenticatorName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpNoHttpAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CMcpNoHttpAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMcpNoHttpAuthenticator::Prepare( CORE::CString& problem )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttpTransport: No authentication is required. Every process of this machine, and every web page whose origin is allowed, can use the server" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpNoHttpAuthenticator::Authenticate( const CHttpRequestData& request ,
                                       COM::CMcpPrincipal& principal   ,
                                       CHttpResponseData& challenge    )
{GUCEF_TRACE;

    principal.subject = "anonymous";
    principal.isAuthenticated = false;
    principal.scopes.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpNoHttpAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "authenticator", CORE::CVariant( NoAuthenticatorName ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpNoHttpAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
