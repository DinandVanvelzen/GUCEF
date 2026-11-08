/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#ifndef GUCEF_MCPLOCALLLM_TOOLS_H
#define GUCEF_MCPLOCALLLM_TOOLS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefMcpLocalLlm_LlmClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The settings of the tools */
struct SToolSettings
{
    GUCEF::CORE::CString agentScript;   /**< the coding agent client: local-agent.sh, or local-agent.ps1 on Windows */
    GUCEF::CORE::CString model;         /**< the model preferred when a call names none; empty: any */
    GUCEF::UInt32 agentTimeoutInSecs;   /**< a delegated task running longer is stopped */
    GUCEF::UInt32 maxDiffLines;         /**< a diff up to this size is part of a delegation result, 0: never */

    GUCEF::CORE::CString::StringVector allowedCommands;     /**< run_and_digest: patterns, * and ? as wildcards */
    GUCEF::UInt32 commandTimeoutInSecs; /**< run_and_digest: a command running longer is stopped */
    GUCEF::CORE::CString logDir;        /**< run_and_digest: where the full output of commands is kept */

    SToolSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Registers the tools of the server:
 *
 *    delegate_to_local_llm   runs the coding agent on a directory with the model of an endpoint of the pool;
 *                            only with an agent script and endpoints
 *    delegate_batch          runs several such tasks, at the same time on as many endpoints as are free; only with
 *                            an agent script and endpoints
 *    run_and_digest          runs an allowed command and returns its errors, failed targets and failed tests;
 *                            only when commands are allowed
 *    ask_local_llm           answers a question about files with the model of an endpoint; only with endpoints
 *    local_llm_status        reports the endpoints of the pool and the models they run
 *
 *  The tools that use a model take an optional "model": only endpoints running it are used. Without it the
 *  configured model (settings.model) is preferred, and any endpoint used when none runs it.
 *
 *  The pool must outlive the server.
 */
bool RegisterTools( GUCEF::COM::CMcpServer& server, CLlmEndpointPool& pool, const SToolSettings& settings );

/** Matches text against a pattern with * (any characters) and ? (one character) */
bool WildcardMatch( const char* pattern, const char* text );

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPLOCALLLM_TOOLS_H ? */
