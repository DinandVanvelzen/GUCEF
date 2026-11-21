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

#ifndef GUCEF_MCPCI_CCIDEPLOYIMAGE_H
#define GUCEF_MCPCI_CCIDEPLOYIMAGE_H

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
 *  One image of the "deploy" block of a CI id in the registry (ci.json), with the Deployments it may be rolled out to.
 *  Only what is listed here can ever be touched by a deployment:
 *
 *      "deploy": {
 *        "images": [
 *          {
 *            "name": "asset-store-service",            the name CI builds the image under (images.json, ci_submit images)
 *            "registryName": "asset-store-service",    optional, the repository name in the registry; default: name
 *            "registryOwner": "ci",                    optional, the owner in the registry; default ci
 *            "targets": {
 *              "live": { "namespace": "assetstore", "deployment": "assetstore", "container": "assetstore",
 *                        "allowScratch": false }       optional; default false: only images of published commits
 *            }
 *          }
 *        ]
 *      }
 *
 *  The image reference of a deployment is <registry host>/<registryOwner>/<registryName>@sha256:<digest>.
 */
class CCiDeployImage
{
    public:

    /** A Deployment the image may be rolled out to, under a name the model uses ("live", "dev", ...) */
    class CTarget
    {
        public:

        GUCEF::CORE::CString name;
        GUCEF::CORE::CString nameSpace;
        GUCEF::CORE::CString deployment;
        GUCEF::CORE::CString container;
        bool allowScratch;                  /**< whether images of scratch snapshots may be rolled out here */

        CTarget( void );

        /** "<namespace>/<deployment>", the key of the lock of the Deployment */
        GUCEF::CORE::CString GetKey( void ) const;
    };

    typedef GUCEF::vector< CTarget > TTargetVector;

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString registryName;
    GUCEF::CORE::CString registryOwner;
    TTargetVector targets;

    CCiDeployImage( void );

    /**
     *  Reads an image of the deploy block. Strict: a missing or malformed name, namespace, deployment or container, or
     *  an image without targets refuses the whole entry with the reason in 'problem'. Target names are free.
     */
    bool LoadConfig( const GUCEF::CORE::CDataNode& node, const GUCEF::CORE::CString& ciId, GUCEF::CORE::CString& problem );

    const CTarget* FindTarget( const GUCEF::CORE::CString& targetName ) const;

    /** The names of the targets, comma separated */
    GUCEF::CORE::CString ListTargets( void ) const;

    /**
     *  Whether the text is a name Kubernetes takes for a namespace, a Deployment or a container (RFC 1123: lower case
     *  letters, digits, '-' and '.', starting and ending with a letter or digit, at most 253 characters), which is also
     *  what makes it safe in the arguments of kubectl
     */
    static bool IsKubernetesName( const GUCEF::CORE::CString& text );

    /** Whether the text is safe as an image or registry name: lower case letters, digits, '-', '_', '.', '/' */
    static bool IsImageName( const GUCEF::CORE::CString& text );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIDEPLOYIMAGE_H ? */
