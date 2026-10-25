/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "authpluginANDROID_CAndroidAuthUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAndroidAuthUtils::IsAcceptableUrl( const CORE::CString& url, bool allowInsecureHttp )
{GUCEF_TRACE;

    // No user information, no control characters, no spaces: what is in the url is what is asked for
    UInt32 length = AUTH::GetByteLength( url );
    if ( 0 == length || length > 2048 )
        return false;
    const char* chars = url.C_String();
    for ( UInt32 i=0; i<length; ++i )
        if ( (unsigned char) chars[ i ] <= 0x20 || (unsigned char) chars[ i ] >= 0x7F )
            return false;

    CORE::CString lower = url.Lowercase();
    bool https = lower.StartsWith( "https://" );
    bool http = lower.StartsWith( "http://" );
    if ( !https && !( http && allowInsecureHttp ) )
        return false;

    // The text between the scheme and the first '/' is the authority; an '@' in it is user information
    CORE::CString rest = url.SubstrFromRange( https ? 8 : 7, url.Length() );
    CORE::CString authority = rest.SubstrToChar( '/', true, false );
    return authority.HasChar( '@' ) < 0 && !authority.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthUtils::Execute( const CORE::CString& method      ,
                            const CORE::CString& url         ,
                            const CORE::CString& contentType ,
                            const CORE::CString& body        ,
                            UInt32 timeoutInMs               ,
                            Int32& statusCode                ,
                            CORE::CString& responseBody      )
{GUCEF_TRACE;

    statusCode = 0;
    responseBody.Clear();

    WEB::CHttpRequestData request;
    request.requestType = method;
    request.resourceRepresentations.push_back( "application/json" );
    if ( !contentType.IsNULLOrEmpty() )
    {
        request.contentRepresentation = contentType;
        request.content.Clear( true );
        request.content.CopyFrom( AUTH::GetByteLength( body ), body.C_String() );
    }
    if ( !request.SetUrl( url ) )
        return false;

    WEB::CHttpClientRequestOptions options;
    options.totalTimeoutInMs = timeoutInMs;
    options.connectTimeoutInMs = timeoutInMs;
    options.followRedirects = false;
    options.verifyPeer = true;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transfer;
    if ( !client.Execute( request, options, response, &transfer ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: The request did not get an answer: " << transfer.errorDescription );
        return false;
    }

    statusCode = response.statusCode;
    if ( response.content.GetDataSize() > MaxResponseSize )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: The answer is larger than the limit and is dropped" );
        return false;
    }
    if ( response.content.GetDataSize() > 0 )
        responseBody = CORE::CString( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthUtils::ParseJson( const CORE::CString& text, CORE::CDataNode& root )
{GUCEF_TRACE;

    if ( 0 == AUTH::GetByteLength( text ) || AUTH::GetByteLength( text ) > MaxResponseSize )
        return false;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: There is no json codec, load dstorepluginJSONPARSER" );
        return false;
    }
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( text, false );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &root, &access );
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthUtils::DecodeJwtClaims( const CORE::CString& jwt, CORE::CDataNode& claims )
{GUCEF_TRACE;

    UInt32 length = AUTH::GetByteLength( jwt );
    if ( 0 == length || length > MaxJwtSize )
        return false;

    // header.payload.signature
    Int32 firstDot = jwt.HasChar( '.' );
    Int32 secondDot = firstDot >= 0 ? jwt.HasChar( '.', (UInt32) firstDot + 1, true ) : -1;
    if ( firstDot < 1 || secondDot < 0 || secondDot <= firstDot + 1 || (UInt32) secondDot + 1 >= length )
        return false;

    CORE::CString payload = jwt.SubstrFromRange( (UInt32) firstDot + 1, (UInt32) secondDot );
    CORE::CDynamicBuffer decoded;
    if ( !CORE::Base64UrlDecode( payload, decoded ) || 0 == decoded.GetDataSize() )
        return false;

    CORE::CString json( decoded.AsConstTypePtr< char >(), decoded.GetDataSize() );
    return ParseJson( json, claims );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAndroidAuthUtils::GetString( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    if ( GUCEF_NULL == attribute || attribute->second.IsNULL() )
        return CORE::CString::Empty;
    return attribute->second.AsString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAndroidAuthUtils::MakeNonce( void )
{GUCEF_TRACE;

    UInt8 random[ 16 ];
    if ( !AUTH::CAuthCrypto::RandomBytes( random, sizeof( random ) ) )
        return CORE::CString::Empty;
    CORE::CString nonce;
    CORE::Base64UrlEncode( random, sizeof( random ), nonce, false );
    return nonce;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthUtils::VerifyWithEngine( const CORE::CString& descriptorName         ,
                                     const AUTH::CAuthProviderSettings& settings ,
                                     const AUTH::CAuthProof& proof               ,
                                     AUTH::CAuthResult& result                   )
{GUCEF_TRACE;

    AUTH::IAuthFlowEngine* engine = AUTH::CAuthGlobal::Instance()->GetFlowEngine();
    if ( GUCEF_NULL == engine )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: A token cannot be verified, there is no OIDC engine" );
        return false;
    }

    AUTH::CAuthProviderDescriptor descriptor;
    if ( !AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( descriptorName, descriptor ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: A token cannot be verified, there is no descriptor " << descriptorName );
        return false;
    }

    if ( !engine->IsSupported( descriptor, settings ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthUtils: The OIDC engine cannot verify tokens of " << descriptorName << " with these settings" );
        return false;
    }
    return engine->Verify( descriptor, settings, proof, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
