/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiDeployImage.h"
#include "gucefMcpCi_CCiImageReport.h"
#include "gucefMcpCi_CRegistryClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 ConnectTimeoutInMs = 10000;
static const UInt32 TotalTimeoutInMs = 30000;
static const UInt64 MaxManifestBytes = (UInt64) 4 * 1024 * 1024;
static const UInt64 MaxTokenAnswerBytes = (UInt64) 64 * 1024;
static const UInt32 MaxTagLength = 128;
static const char* UserAgent = "gucefMcpCi/1.0";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
HexOf( const UInt8* bytes, UInt32 size )
{
    static const char* digits = "0123456789abcdef";
    CORE::CString text;
    for ( UInt32 i=0; i<size; ++i )
    {
        text += digits[ bytes[ i ] >> 4 ];
        text += digits[ bytes[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRegistryClient::CRegistryClient( void )
    : m_token()
    , m_caBundlePath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRegistryClient::SetToken( const CORE::CString& token )
{GUCEF_TRACE;

    m_token = token;
    if ( !m_token.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( m_token );
}

/*-------------------------------------------------------------------------*/

void
CRegistryClient::SetCaBundlePath( const CORE::CString& caBundlePath )
{GUCEF_TRACE;

    m_caBundlePath = caBundlePath;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryClient::IsTag( const CORE::CString& text )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( 0 == length || length > MaxTagLength )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char ch = c[ i ];
        const bool word = ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || '_' == ch;
        if ( !word && !( i > 0 && ( '.' == ch || '-' == ch ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRegistryClient::Header( const TStringMap& headers, const char* name )
{GUCEF_TRACE;

    for ( TStringMap::const_iterator i = headers.begin(); i != headers.end(); ++i )
        if ( (*i).first.Equals( name, false ) )
            return CCiUtils::Trimmed( (*i).second );
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryClient::ParseChallenge( const CORE::CString& challenge, CORE::CString& scheme, TStringMap& parameters )
{GUCEF_TRACE;

    parameters.clear();
    const CORE::CString text = CCiUtils::Trimmed( challenge );
    const Int32 space = text.HasChar( ' ', true );
    scheme = ( space > 0 ? text.SubstrToIndex( (UInt32) space, true ) : text ).Lowercase();
    if ( space <= 0 )
        return !scheme.IsNULLOrEmpty();

    // name="value" pairs separated by commas; a value may hold commas inside its quotes
    const CORE::CString rest = text.CutChars( (UInt32) space + 1, true );
    const char* c = rest.C_String();
    const UInt32 size = rest.ByteSize() > 0 ? rest.ByteSize() - 1 : 0;
    UInt32 i = 0;
    while ( i < size )
    {
        while ( i < size && ( ' ' == c[ i ] || ',' == c[ i ] ) )
            ++i;
        UInt32 nameStart = i;
        while ( i < size && '=' != c[ i ] && ',' != c[ i ] )
            ++i;
        const CORE::CString name = CCiUtils::Trimmed( CORE::CString( c + nameStart, i - nameStart ) ).Lowercase();
        if ( i >= size || '=' != c[ i ] )
            continue;
        ++i;
        CORE::CString value;
        if ( i < size && '"' == c[ i ] )
        {
            ++i;
            while ( i < size && '"' != c[ i ] )
            {
                if ( '\\' == c[ i ] && i+1 < size )
                    ++i;
                value += c[ i ];
                ++i;
            }
            ++i;
        }
        else
        {
            UInt32 valueStart = i;
            while ( i < size && ',' != c[ i ] )
                ++i;
            value = CCiUtils::Trimmed( CORE::CString( c + valueStart, i - valueStart ) );
        }
        if ( !name.IsNULLOrEmpty() )
            parameters[ name ] = value;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRegistryClient::Origin( const CORE::CString& url )
{GUCEF_TRACE;

    const CORE::CString lower = CCiUtils::Trimmed( url ).Lowercase();
    const Int32 schemeEnd = lower.HasSubstr( "://", true );
    if ( schemeEnd <= 0 )
        return CORE::CString::Empty;
    const CORE::CString scheme = lower.SubstrToIndex( (UInt32) schemeEnd, true );
    CORE::CString authority = lower.CutChars( (UInt32) schemeEnd + 3, true );
    const Int32 slash = authority.HasChar( '/', true );
    if ( slash >= 0 )
        authority = authority.SubstrToIndex( (UInt32) slash, true );
    const Int32 query = authority.HasChar( '?', true );
    if ( query >= 0 )
        authority = authority.SubstrToIndex( (UInt32) query, true );
    if ( authority.HasChar( '@', true ) >= 0 || authority.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const Int32 close = authority.HasChar( ']', false );
    const Int32 colon = authority.HasChar( ':', false );
    if ( colon < 0 || colon < close )
        authority += scheme == "https" ? ":443" : ":80";
    return scheme + "://" + authority;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRegistryClient::BasicAuthorization( const CORE::CString& user ) const
{GUCEF_TRACE;

    if ( m_token.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString pair = user + ":" + m_token;
    return "Basic " + CORE::Base64Encode( pair.C_String(), pair.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

bool
CRegistryClient::Execute( const CORE::CString& method          ,
                          const CORE::CString& url             ,
                          const CORE::CString& authorization   ,
                          bool acceptManifests                 ,
                          UInt64 maxBytes                      ,
                          Int32& status                        ,
                          TStringMap& headers                  ,
                          CORE::CDynamicBuffer& body           ,
                          CORE::CString& error                 ) const
{GUCEF_TRACE;

    status = 0;
    headers.clear();
    body.Clear();
    WEB::CHttpRequestData request;
    request.requestType = method;
    if ( !request.SetUrl( url ) )
    {
        error = "the address of the registry is not a valid URL: " + WEB::CHttpLogRedaction::RedactUrl( url );
        return false;
    }
    if ( !authorization.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = authorization;
    if ( acceptManifests )
    {
        request.resourceRepresentations.push_back( "application/vnd.oci.image.index.v1+json" );
        request.resourceRepresentations.push_back( "application/vnd.oci.image.manifest.v1+json" );
        request.resourceRepresentations.push_back( "application/vnd.docker.distribution.manifest.list.v2+json" );
        request.resourceRepresentations.push_back( "application/vnd.docker.distribution.manifest.v2+json" );
    }
    else
        request.resourceRepresentations.push_back( "application/json" );

    WEB::CHttpClientRequestOptions options;
    options.maxResponseBodySize = maxBytes;
    options.connectTimeoutInMs = ConnectTimeoutInMs;
    options.totalTimeoutInMs = TotalTimeoutInMs;
    options.verifyPeer = true;
    options.caBundlePath = m_caBundlePath;
    options.followRedirects = false;            // the credentials are never sent elsewhere
    options.useEnvironmentProxy = false;
    options.userAgent = UserAgent;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transferInfo;
    if ( !client.Execute( request, options, response, &transferInfo ) )
    {
        if ( transferInfo.responseBodyTooLarge )
            error = "the answer of the registry is larger than " + CCiUtils::FormatBytes( maxBytes );
        else
            error = "the registry did not answer (" + CORE::CString( WEB::HttpTransferErrorClassToString( transferInfo.errorClass ) ) + ": " +
                    WEB::CHttpLogRedaction::RedactText( transferInfo.errorDescription ) + ")";
        return false;
    }
    status = response.statusCode;
    for ( WEB::CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin(); i != response.otherHeaders.end(); ++i )
        headers[ (*i).first ] = (*i).second;
    body = response.content;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryClient::FetchBearerToken( const CORE::CString& registryUrl   ,
                                   const CORE::CString& user          ,
                                   const CORE::CString& challenge     ,
                                   const CORE::CString& repository    ,
                                   CORE::CString& bearer              ,
                                   CORE::CString& error               ) const
{GUCEF_TRACE;

    CORE::CString scheme;
    TStringMap parameters;
    if ( !ParseChallenge( challenge, scheme, parameters ) || scheme != "bearer" )
    {
        error = "the registry asked for another kind of authentication";
        return false;
    }
    TStringMap::const_iterator realm = parameters.find( "realm" );
    if ( realm == parameters.end() || (*realm).second.IsNULLOrEmpty() )
    {
        error = "the registry asked for a bearer token without naming where to get it";
        return false;
    }
    // the credentials go to the registry itself only
    const CORE::CString realmOrigin = Origin( (*realm).second );
    if ( realmOrigin.IsNULLOrEmpty() || realmOrigin != Origin( registryUrl ) )
    {
        error = "the registry asked for a bearer token from another server (" + WEB::CHttpLogRedaction::RedactUrl( (*realm).second ) +
                "); the credentials are only sent to the registry itself";
        return false;
    }

    CORE::CString url = (*realm).second;
    url += url.HasChar( '?', true ) >= 0 ? "&" : "?";
    TStringMap::const_iterator service = parameters.find( "service" );
    if ( service != parameters.end() && !(*service).second.IsNULLOrEmpty() )
        url += "service=" + CCiUtils::PercentEncode( (*service).second ) + "&";
    url += "scope=" + CCiUtils::PercentEncode( "repository:" + repository + ":pull" );

    Int32 status = 0;
    TStringMap headers;
    CORE::CDynamicBuffer body;
    if ( !Execute( "GET", url, BasicAuthorization( user ), false, MaxTokenAnswerBytes, status, headers, body, error ) )
        return false;
    if ( 200 != status )
    {
        error = "the registry gave no token (HTTP " + CORE::Int32ToString( status ) + ")" +
                ( 401 == status || 403 == status ? CORE::CString( ": it refused the credentials (registryUser of ci.json, --registry-token-file)" ) : CORE::CString::Empty );
        return false;
    }
    CORE::CDataNode json;
    if ( !CCiUtils::ParseJson( body, json ) )
    {
        error = "the token answer of the registry is not JSON";
        return false;
    }
    bearer = CCiUtils::Trimmed( json.GetAttributeValue( "token" ).AsString() );
    if ( bearer.IsNULLOrEmpty() )
        bearer = CCiUtils::Trimmed( json.GetAttributeValue( "access_token" ).AsString() );
    if ( bearer.IsNULLOrEmpty() )
    {
        error = "the token answer of the registry holds no token";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryClient::ResolveDigest( const CORE::CString& registryUrl  ,
                                const CORE::CString& user         ,
                                const CORE::CString& repository   ,
                                const CORE::CString& tag          ,
                                CORE::CString& digest             ,
                                bool& notFound                    ,
                                CORE::CString& error              ) const
{GUCEF_TRACE;

    digest.Clear();
    notFound = false;
    if ( !CCiDeployImage::IsImageName( repository ) || !IsTag( tag ) )
    {
        error = "\"" + repository + ":" + tag + "\" is not an image name and tag";
        return false;
    }
    CORE::CString base = registryUrl;
    while ( base.EndsWith( "/" ) )
        base = base.CutChars( 1, false );
    const CORE::CString url = base + "/v2/" + repository + "/manifests/" + tag;
    const CORE::CString what = "the registry " + base + " for " + repository + ":" + tag;

    // without credentials first: the registry says how it wants them
    Int32 status = 0;
    TStringMap headers;
    CORE::CDynamicBuffer body;
    CORE::CString authorization;
    if ( !Execute( "HEAD", url, authorization, true, MaxManifestBytes, status, headers, body, error ) )
    {
        error = "Asking " + what + " failed: " + error;
        return false;
    }
    if ( 401 == status && !m_token.IsNULLOrEmpty() )
    {
        const CORE::CString challenge = Header( headers, "www-authenticate" );
        CORE::CString bearer;
        CORE::CString handshakeProblem;
        if ( challenge.Lowercase().StartsWith( "bearer" ) && FetchBearerToken( base, user, challenge, repository, bearer, handshakeProblem ) )
            authorization = "Bearer " + bearer;
        else
        {
            if ( !handshakeProblem.IsNULLOrEmpty() )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Registry: The token handshake failed (" << handshakeProblem << "); trying Basic credentials" );
            authorization = BasicAuthorization( user );
        }
        if ( !Execute( "HEAD", url, authorization, true, MaxManifestBytes, status, headers, body, error ) )
        {
            error = "Asking " + what + " failed: " + error;
            return false;
        }
        if ( 401 == status && authorization.StartsWith( "Bearer" ) )
        {
            // a bearer the registry does not take: Basic as the last try
            authorization = BasicAuthorization( user );
            if ( !Execute( "HEAD", url, authorization, true, MaxManifestBytes, status, headers, body, error ) )
            {
                error = "Asking " + what + " failed: " + error;
                return false;
            }
        }
    }
    if ( 401 == status || 403 == status )
    {
        error = "The registry refused to give " + repository + ":" + tag + " (HTTP " + CORE::Int32ToString( status ) + "); the registry token needs read:package " +
                "(registryUser of ci.json, --registry-token-file)";
        return false;
    }
    if ( 404 == status )
    {
        notFound = true;
        error = "The registry has no image " + repository + ":" + tag;
        return false;
    }
    if ( 200 != status )
    {
        error = "The registry answered HTTP " + CORE::Int32ToString( status ) + " for " + repository + ":" + tag;
        return false;
    }

    digest = Header( headers, "docker-content-digest" ).Lowercase();
    if ( digest.IsNULLOrEmpty() )
    {
        // the digest of the manifest itself
        if ( !Execute( "GET", url, authorization, true, MaxManifestBytes, status, headers, body, error ) || 200 != status )
        {
            error = "The manifest of " + repository + ":" + tag + " could not be read: " + ( error.IsNULLOrEmpty() ? "HTTP " + CORE::Int32ToString( status ) : error );
            return false;
        }
        digest = Header( headers, "docker-content-digest" ).Lowercase();
        if ( digest.IsNULLOrEmpty() )
        {
            UInt8 hash[ CORE::CSha256::DigestSize ];
            CORE::CSha256::Hash( body.GetConstBufferPtr(), body.GetDataSize(), hash );
            digest = "sha256:" + HexOf( hash, CORE::CSha256::DigestSize );
        }
    }
    if ( !CCiImageReport::IsDigest( digest ) )
    {
        error = "The registry gave no SHA-256 digest for " + repository + ":" + tag;
        digest.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
