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

#ifndef GUCEF_IRCHUB_LLMVENDORCLIENT_H
#define GUCEF_IRCHUB_LLMVENDORCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** One message of a conversation */
struct SChatTurn
{
    GUCEF::CORE::CString role;          /**< "user" or "assistant" */
    GUCEF::CORE::CString content;
};
typedef std::vector< SChatTurn > TChatTurnVector;

/*-------------------------------------------------------------------------*/

/**
 *  How to reach a model. The vendors:
 *    "anthropic"          the Anthropic Messages API: baseUrl https://api.anthropic.com/v1
 *    "openai"             the OpenAI chat completions API (max_completion_tokens): baseUrl https://api.openai.com/v1
 *    "openai-compatible"  a chat completions API with max_tokens: Google Gemini
 *                         (https://generativelanguage.googleapis.com/v1beta/openai), xAI Grok (https://api.x.ai/v1)
 *    "llamacpp"           llama.cpp llama-server, an openai-compatible API that also takes enable_thinking
 */
struct SLlmSettings
{
    GUCEF::CORE::CString vendor;
    GUCEF::CORE::CString baseUrl;       /**< up to and including the API version, without the last slash */
    GUCEF::CORE::CString model;         /**< empty: not sent, the server's default model (llama-server) */
    GUCEF::CORE::CString apiKey;        /**< empty: no authentication */
    GUCEF::CORE::CString effort;        /**< anthropic: low, medium, high, xhigh or max; empty: the model's default */
    GUCEF::UInt32 maxTokens;
    bool thinking;                      /**< llamacpp: let a reasoning model think first */
    GUCEF::UInt32 timeoutInMs;

    SLlmSettings( void );
};

/*-------------------------------------------------------------------------*/

struct SLlmResult
{
    GUCEF::CORE::CString answer;
    GUCEF::CORE::CString model;         /**< the model that answered, as the API reports it */
    GUCEF::CORE::CString error;         /**< why there is no answer, for the log and for the person who asked */
    bool refused;                       /**< the model declined to answer */
    GUCEF::UInt32 inputTokens;
    GUCEF::UInt32 outputTokens;

    SLlmResult( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whether the vendor is one Chat() knows */
bool IsKnownVendor( const GUCEF::CORE::CString& vendor );

/**
 *  Sends the conversation to the model and waits for the answer. Blocks for as long as the model takes: call it from a
 *  worker thread. Needs the JSON codec plugin and, for https, the TLS support of gucefWEB.
 *  @return false with the reason in result.error when there is no answer
 */
bool Chat( const SLlmSettings& settings            ,
           const GUCEF::CORE::CString& system      ,
           const TChatTurnVector& turns            ,
           SLlmResult& result                      );

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IRCHUB_LLMVENDORCLIENT_H ? */
