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

#ifndef GUCEF_MCPCI_CCIDEFINITION_H
#define GUCEF_MCPCI_CCIDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/**
 *  One CI set up for a repository, an entry of the registry (ci.json):
 *
 *      {
 *        "id": "GUCEF",                          the CI id the model uses
 *        "description": "GUCEF platform",        optional, shown by ci_resolve_repo
 *        "backend": "forgejo",
 *        "url": "https://forgejo.lab:32001",      the web root of the backend
 *        "owner": "GUCEF", "repo": "GUCEF",      the published repository (informational)
 *        "scratchOwner": "ci", "scratchRepo": "GUCEF",   the writable repository the snapshots go to
 *        "scratchUrl": "...",                     optional git URL of the scratch repository; default <url>/<scratchOwner>/<scratchRepo>.git
 *        "workflow": "gucef-ci.yml",              the workflow file that is dispatched
 *        "matrixDir": "projects/CI/matrix",       optional, where matrix.json and products.json are in the repository
 *        "reportsOwner": "ci-bot",                optional, the user or organisation that owns the report packages
 *                                                 (ci-reports); default: --reports-owner, else ci-bot. Not the scratch owner
 *        "retentionDays": 14,                     optional, scratch branches and reports older than this are removed; 0: never
 *        "remotes": [ "https://github.com/org/GUCEF.git", "git@github.com:org/GUCEF.git" ],
 *        "defaults": { "mode": "build", "toolchains": [], "cppStandards": [], "products": [] },
 *        "options": { "toolchains": [], "cppStandards": [], "products": [] }   optional, used by ci_describe without a repository
 *      }
 */
class CCiDefinition
{
    public:

    GUCEF::CORE::CString id;
    GUCEF::CORE::CString description;
    GUCEF::CORE::CString backend;
    GUCEF::CORE::CString url;
    GUCEF::CORE::CString owner;
    GUCEF::CORE::CString repo;
    GUCEF::CORE::CString scratchOwner;
    GUCEF::CORE::CString scratchRepo;
    GUCEF::CORE::CString scratchUrl;
    GUCEF::CORE::CString workflow;
    GUCEF::CORE::CString matrixDir;
    GUCEF::CORE::CString reportsOwner;
    GUCEF::UInt32 retentionDays;
    GUCEF::CORE::CString::StringVector remotes;
    GUCEF::CORE::CString::StringVector normalizedRemotes;

    GUCEF::CORE::CString defaultMode;
    GUCEF::CORE::CString::StringVector defaultToolchains;
    GUCEF::CORE::CString::StringVector defaultCppStandards;
    GUCEF::CORE::CString::StringVector defaultProducts;

    GUCEF::CORE::CString::StringVector optionToolchains;
    GUCEF::CORE::CString::StringVector optionCppStandards;
    GUCEF::CORE::CString::StringVector optionProducts;

    CCiDefinition( void );

    /** Reads an entry of the registry. @return false with the reason when a required field is missing */
    bool LoadConfig( const GUCEF::CORE::CDataNode& node, GUCEF::CORE::CString& problem );

    /** The git URL of the scratch repository */
    GUCEF::CORE::CString GetScratchGitUrl( void ) const;


    /** The web root without a trailing / */
    GUCEF::CORE::CString GetBaseUrl( void ) const;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIDEFINITION_H ? */
