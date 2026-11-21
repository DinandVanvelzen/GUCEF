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

#ifndef GUCEF_MCPCI_CCIRELEASESTATUS_H
#define GUCEF_MCPCI_CCIRELEASESTATUS_H

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

/** What the Deployments of a target of a CI id run now: one entry per image of the deploy block with that target */
class CCiReleaseStatus
{
    public:

    class CEntry
    {
        public:

        GUCEF::CORE::CString image;
        GUCEF::CORE::CString nameSpace;
        GUCEF::CORE::CString deployment;
        GUCEF::CORE::CString container;
        bool allowScratch;
        GUCEF::CORE::CString imageReference;    /**< what the container runs */
        GUCEF::CORE::CString digest;            /**< of the reference, empty when it names a tag */
        GUCEF::CORE::CString imagePullPolicy;
        GUCEF::CORE::CString revision;
        GUCEF::CORE::CString changeCause;       /**< who deployed what: commit, run, machine and time when the CI did */
        GUCEF::CORE::CString strategy;
        GUCEF::CORE::CString replicas;          /**< for a person */
        GUCEF::CORE::CString history;           /**< the revision history, bounded */
        GUCEF::CORE::CString problem;           /**< why the state could not be read, empty when it was */

        CEntry( void );
    };

    typedef GUCEF::vector< CEntry > TEntryVector;

    GUCEF::CORE::CString ciId;
    GUCEF::CORE::CString target;
    TEntryVector entries;

    CCiReleaseStatus( void );

    void SaveToJson( GUCEF::CORE::CDataNode& node ) const;

    void LoadFromJson( const GUCEF::CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIRELEASESTATUS_H ? */
