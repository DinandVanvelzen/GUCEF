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

#ifndef GUCEF_MCPCI_CCIDEPLOYRESULT_H
#define GUCEF_MCPCI_CCIDEPLOYRESULT_H

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
 *  What a deployment or a rollback did, or why it was refused, as far as it is known; also the line of the audit.
 *
 *  Outcomes: deployed, unchanged (the target already ran the digest), rolled_back, refused (a guard said no: see code),
 *  rollout_failed, rollout_timeout, rollout_unknown (the rollout could not be followed), cancelled, failed (kubectl or
 *  the registry failed before anything changed).
 *
 *  Codes of refusals: not_listed, scope, confirm, bad_request, unknown_run, not_green, no_images, image_failed,
 *  unpublished, registry, pull_policy, busy, kubectl, unknown_ci.
 */
class CCiDeployResult
{
    public:

    GUCEF::CORE::CString action;            /**< deploy or rollback */
    GUCEF::CORE::CString ciId;
    GUCEF::CORE::CString image;
    GUCEF::CORE::CString target;
    GUCEF::CORE::CString nameSpace;
    GUCEF::CORE::CString deployment;
    GUCEF::CORE::CString container;
    GUCEF::CORE::CString runId;
    GUCEF::CORE::CString commit;
    GUCEF::CORE::CString digest;
    GUCEF::CORE::CString imageReference;    /**< what the container was set to */
    GUCEF::CORE::CString previousImage;     /**< what the container ran before */
    GUCEF::CORE::CString previousDigest;
    GUCEF::CORE::CString revision;          /**< of the Deployment afterwards, when known */
    GUCEF::CORE::CString replicas;          /**< the replicas afterwards for a person, when known */
    GUCEF::CORE::CString strategy;
    GUCEF::CORE::CString outcome;
    GUCEF::CORE::CString code;
    GUCEF::CORE::CString message;           /**< what happened and what the state is, for the reader */

    CCiDeployResult( void );

    bool IsRefused( void ) const;

    /** Whether the Deployment runs what was asked for */
    bool IsSuccess( void ) const;

    void SaveToJson( GUCEF::CORE::CDataNode& node ) const;

    void LoadFromJson( const GUCEF::CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIDEPLOYRESULT_H ? */
