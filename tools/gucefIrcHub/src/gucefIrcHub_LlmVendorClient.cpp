/*
 *  gucefIrcHub: IRC hub with LLM bots for a family IRC server
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

#include "gucefIrcHub_LlmVendorClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 ConnectTimeoutInMs = 10000;
static const char* AnthropicApiVersion = "2026-10-21";
/** server-side fallback to another model when a safety classifier declines a request */
static const char* AnthropicFallbackBeta = "server-side-fallback-2026-07-01";
static const UInt32 MaxErrorBodyLength = 300;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

SLlmSettings::SLlmSettings( void )
    : vendor()
    , baseUrl()
    , model()
    , apiKey()
    , effort()
    , maxTokens( 4096 )
    , thinking( false )
    , timeoutInMs( 300000 )
{
}

/*-------------------------------------------------------------------------*/

SLlmResult::SLlmResult( void )
    : answer()
    , model()
    , error()
    , refused( false )
    , inputTokens( 0 )
    , outputTokens( 0 )
{
}

/*-------------------------------------------------------------------------*/

bool
IsKnownVendor( const CORE::CString& vendor )
{
    return "anthropic" == vendor || "openai" == vendor || "openai-compatible" == vendor || "llamacpp" == vendor;
}

/*-------------------------------------------------------------------------*/

static bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
BufferAsText( const CORE::CDynamicBuffer& buffer, UInt32 maxLength )
{
    UInt32 length = buffer.GetDataSize();
    if ( 0 == length )
        return CORE::CString::Empty;
    if ( length > maxLength )
        length = maxLength;
    return CORE::CString( std::string( (const char*) buffer.GetConstBufferPtr(), length ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  POSTs body as JSON and parses the response as JSON. An error response of the API is described by the message
 *  in its JSON ({"error": {"message": ...}}) when it has one, else by the start of its text.
 */
static bool
PostJson( const CORE::CString& url                ,
          const CORE::CString::StringMap& headers ,
          const CORE::CDataNode& body             ,
          UInt32 timeoutInMs                      ,
          CORE::CDataNode& root                   ,
          CORE::CString& error                    )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
    {
        error = "the JSON codec is not available";
        return false;
    }

    WEB::CHttpRequestData request;
    request.requestType = "POST";
    if ( !request.SetUrl( url ) )
    {
        error = "invalid URL " + url;
        return false;
    }
    CORE::CDynamicBufferAccess bodyAccess( request.content );
    if ( !codec->StoreDataTree( &body, &bodyAccess ) )
    {
        error = "the request could not be written as JSON";
        return false;
    }
    request.contentRepresentation = "application/json";
    CORE::CString::StringMap::const_iterator h = headers.begin();
    while ( h != headers.end() )
    {
        request.otherHeaders[ (*h).first ] = (*h).second;
        ++h;
    }

    WEB::CHttpClientRequestOptions options;
    options.connectTimeoutInMs = ConnectTimeoutInMs;
    options.totalTimeoutInMs = timeoutInMs;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transferInfo;
    const bool executed = client.Execute( request, options, response, &transferInfo );
    if ( !executed && 0 == response.statusCode )
    {
        error = transferInfo.errorDescription.IsNULLOrEmpty() ? CORE::CString( "no response" ) : transferInfo.errorDescription;
        return false;
    }

    CORE::CDynamicBufferAccess access( response.content );
    const bool isJson = response.content.GetDataSize() > 0 && codec->BuildDataTree( &root, &access );
    if ( 200 != response.statusCode )
    {
        error = "HTTP status " + CORE::Int32ToString( response.statusCode );
        const CORE::CDataNode* errorNode = isJson ? root.FindChild( "error" ) : GUCEF_NULL;
        CORE::CString message;
        if ( GUCEF_NULL != errorNode )
            message = errorNode->GetAttributeValue( "message" ).AsString();
        if ( message.IsNULLOrEmpty() )
            message = BufferAsText( response.content, MaxErrorBodyLength );
        if ( !message.IsNULLOrEmpty() )
            error += ": " + message;
        return false;
    }
    if ( !isJson )
    {
        error = "the response is not JSON";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
AddMessage( CORE::CDataNode& messages, const CORE::CString& role, const CORE::CString& content )
{
    CORE::CDataNode* message = messages.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    message->SetAttribute( "role", role );
    message->SetAttribute( "content", content );
}

/*-------------------------------------------------------------------------*/

static bool
ChatAnthropic( const SLlmSettings& settings ,
               const CORE::CString& system  ,
               const TChatTurnVector& turns ,
               SLlmResult& result           )
{
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "model", settings.model );
    body.SetAttribute( "max_tokens", settings.maxTokens );
    if ( !system.IsNULLOrEmpty() )
        body.SetAttribute( "system", system );
    CORE::CDataNode* messages = body.AddChild( "messages", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<turns.size(); ++i )
        AddMessage( *messages, turns[ i ].role, turns[ i ].content );
    if ( !settings.effort.IsNULLOrEmpty() )
    {
        CORE::CDataNode* outputConfig = body.AddChild( "output_config", GUCEF_DATATYPE_OBJECT );
        outputConfig->SetAttribute( "effort", settings.effort );
    }
    // a request a safety classifier declines is answered by a fallback model the API picks
    body.SetAttribute( "fallbacks", CORE::CString( "default" ) );

    CORE::CString::StringMap headers;
    headers[ "x-api-key" ] = settings.apiKey;
    headers[ "anthropic-version" ] = AnthropicApiVersion;
    headers[ "anthropic-beta" ] = AnthropicFallbackBeta;

    CORE::CDataNode response;
    if ( !PostJson( settings.baseUrl + "/messages", headers, body, settings.timeoutInMs, response, result.error ) )
        return false;

    result.model = response.GetAttributeValue( "model" ).AsString();
    const CORE::CDataNode* usage = response.FindChild( "usage" );
    if ( GUCEF_NULL != usage )
    {
        result.inputTokens = usage->GetAttributeValue( "input_tokens" ).AsUInt32();
        result.outputTokens = usage->GetAttributeValue( "output_tokens" ).AsUInt32();
    }

    // the answer is the text blocks of the content; thinking blocks are left out
    CORE::CString answer;
    const CORE::CDataNode* content = response.FindChild( "content" );
    const CORE::CDataNode* block = GUCEF_NULL != content ? content->GetFirstChild() : GUCEF_NULL;
    while ( GUCEF_NULL != block )
    {
        if ( "text" == block->GetAttributeValue( "type" ).AsString() )
            answer += block->GetAttributeValue( "text" ).AsString();
        block = block->GetNext();
    }

    const CORE::CString stopReason = response.GetAttributeValue( "stop_reason" ).AsString();
    if ( "refusal" == stopReason )
    {
        result.refused = true;
        result.error = "the model declined to answer";
        return false;
    }
    if ( answer.IsNULLOrEmpty() )
    {
        result.error = "the answer is empty (stop reason " + stopReason + ")";
        return false;
    }
    result.answer = answer;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ChatOpenAiStyle( const SLlmSettings& settings ,
                 const CORE::CString& system  ,
                 const TChatTurnVector& turns ,
                 SLlmResult& result           )
{
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    if ( !settings.model.IsNULLOrEmpty() )
        body.SetAttribute( "model", settings.model );
    // OpenAI's own API takes max_completion_tokens, the compatible APIs max_tokens
    body.SetAttribute( "openai" == settings.vendor ? "max_completion_tokens" : "max_tokens", settings.maxTokens );
    CORE::CDataNode* messages = body.AddChild( "messages", GUCEF_DATATYPE_ARRAY );
    if ( !system.IsNULLOrEmpty() )
        AddMessage( *messages, "system", system );
    for ( size_t i=0; i<turns.size(); ++i )
        AddMessage( *messages, turns[ i ].role, turns[ i ].content );
    if ( "llamacpp" == settings.vendor )
    {
        CORE::CDataNode* templateArgs = body.AddChild( "chat_template_kwargs", GUCEF_DATATYPE_OBJECT );
        templateArgs->SetAttribute( "enable_thinking", CORE::CVariant( settings.thinking ) );
    }

    CORE::CString::StringMap headers;
    if ( !settings.apiKey.IsNULLOrEmpty() )
        headers[ "Authorization" ] = "Bearer " + settings.apiKey;

    CORE::CDataNode response;
    if ( !PostJson( settings.baseUrl + "/chat/completions", headers, body, settings.timeoutInMs, response, result.error ) )
        return false;

    result.model = response.GetAttributeValue( "model" ).AsString();
    const CORE::CDataNode* usage = response.FindChild( "usage" );
    if ( GUCEF_NULL != usage )
    {
        result.inputTokens = usage->GetAttributeValue( "prompt_tokens" ).AsUInt32();
        result.outputTokens = usage->GetAttributeValue( "completion_tokens" ).AsUInt32();
    }

    const CORE::CDataNode* choices = response.FindChild( "choices" );
    const CORE::CDataNode* choice = GUCEF_NULL != choices ? choices->GetFirstChild() : GUCEF_NULL;
    const CORE::CDataNode* reply = GUCEF_NULL != choice ? choice->FindChild( "message" ) : GUCEF_NULL;
    if ( GUCEF_NULL == reply )
    {
        result.error = "the response has no message";
        return false;
    }
    const CORE::CString refusal = reply->GetAttributeValue( "refusal" ).AsString();
    if ( !refusal.IsNULLOrEmpty() )
    {
        result.refused = true;
        result.error = "the model declined to answer: " + refusal;
        return false;
    }
    result.answer = reply->GetAttributeValue( "content" ).AsString();
    if ( result.answer.IsNULLOrEmpty() )
    {
        result.error = "the answer is empty";
        if ( choice->GetAttributeValue( "finish_reason" ).AsString() == "length" )
            result.error += " (the model used up its tokens, probably while thinking)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Chat( const SLlmSettings& settings ,
      const CORE::CString& system  ,
      const TChatTurnVector& turns ,
      SLlmResult& result           )
{GUCEF_TRACE;

    result = SLlmResult();
    if ( "anthropic" == settings.vendor )
        return ChatAnthropic( settings, system, turns, result );
    if ( IsKnownVendor( settings.vendor ) )
        return ChatOpenAiStyle( settings, system, turns, result );
    result.error = "unknown vendor " + settings.vendor;
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/
