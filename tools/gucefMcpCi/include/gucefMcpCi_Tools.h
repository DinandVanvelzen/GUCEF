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

#ifndef GUCEF_MCPCI_TOOLS_H
#define GUCEF_MCPCI_TOOLS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefMcpCi_ICiService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The settings of the tools */
struct SToolSettings
{
    GUCEF::CORE::CString logDir;            /**< where the full logs of jobs are kept, absolute */
    GUCEF::UInt32 maxWaitSeconds;           /**< the longest wait_seconds of ci_status */
    GUCEF::CORE::CRedactor redactor;        /**< what never reaches the output of a tool: the credentials */

    SToolSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Registers the eight CI tools, written against the service so that they are the same in every mode:
 *
 *    ci_resolve_repo     the CI id of a repository, from its remotes; "no CI" when the CI does not know it
 *    ci_describe         the toolchains, C++ standards, products and modes of a CI id
 *    ci_submit           sends the unpublished work of a repository as a snapshot and starts a run on it
 *    ci_status           the state of a run, with a bounded wait for it to finish
 *    ci_report           per job and product what succeeded and failed, the first errors, failed tests
 *    ci_job_details      one job: the digest of its logs, the end of a log or its tests; the full logs are saved
 *    ci_cancel           stops a run
 *    ci_cleanup          removes old scratch branches and reports
 *
 *  The service must outlive the server.
 */
bool RegisterTools( GUCEF::COM::CMcpServer& server, ICiService& service, const SToolSettings& settings );

/**
 *  The digest of a build log as ci_job_details shows it: the distinct errors with file:line, failed targets and
 *  tests; the end of the log when no known error format is in it
 */
GUCEF::CORE::CString FormatLogDigest( const GUCEF::CORE::CString& log, const GUCEF::CORE::CString& workspace );

/** The names of the tools, in the order of RegisterTools() */
GUCEF::CORE::CString::StringVector GetToolNames( void );

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TOOLS_H ? */
