/*
 *  gucefMcpLocalLlm_TestApp: tests of the gucefMcpLocalLlm MCP server
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

#ifndef GUCEF_MCPLOCALLLM_TESTAPP_TESTMCPLOCALLLM_H
#define GUCEF_MCPLOCALLLM_TESTAPP_TESTMCPLOCALLLM_H

/**
 *  Tests of the gucefMcpLocalLlm server, run as a child process over stdio.
 *
 *  Without an LLM server: the tool list, run_and_digest with fixture logs, refused commands and invalid arguments.
 *  With an LLM server (LOCAL_LLM_URL, default http://localhost:8080) and the coding agent script (LOCAL_AGENT_SCRIPT,
 *  default local-agent.sh next to the test application): local_llm_status, ask_local_llm, delegate_to_local_llm,
 *  delegate_batch and the explanation of failures in an unknown format. These are skipped when either is missing.
 */
void PerformMcpLocalLlmTests( void );

#endif /* GUCEF_MCPLOCALLLM_TESTAPP_TESTMCPLOCALLLM_H ? */
