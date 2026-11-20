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

#ifndef GUCEF_MCPCI_CCISNAPSHOT_H
#define GUCEF_MCPCI_CCISNAPSHOT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  The unpublished work of a repository as one synthetic commit on top of HEAD, made without touching the working
 *  tree, the index, the branches or the refs of the repository: only objects are added to its object store.
 */
class CCiSnapshot
{
    public:

    GUCEF::CORE::CString repoDir;           /**< the top level directory of the repository */
    GUCEF::CORE::CString baseCommit;        /**< HEAD */
    GUCEF::CORE::CString commit;            /**< the synthetic commit, HEAD itself without uncommitted work */
    GUCEF::CORE::CString branchName;        /**< scratch/<machine>-<utc>-<shortsha>, the name it is sent under */
    bool includesUncommitted;
    GUCEF::UInt32 filesInSnapshot;          /**< files in the tree of the commit */
    GUCEF::UInt32 modifiedFiles;            /**< tracked files that differ from HEAD */
    GUCEF::UInt32 addedFiles;               /**< files that are not in HEAD, untracked ones included */
    GUCEF::UInt32 untrackedFiles;           /**< of the added files those that are not in the index either */
    GUCEF::UInt32 deletedFiles;
    GUCEF::UInt64 changedBytes;             /**< the size of the added and modified files */
    bool hasSendEstimate;
    GUCEF::UInt64 sendBytesEstimate;        /**< the objects no remote tracking branch has, compressed, as git counts them */
    GUCEF::UInt32 unpublishedCommits;       /**< commits of HEAD that no remote tracking branch has */
    GUCEF::CORE::CString::StringVector changedPaths;   /**< "M path", "A path", "D path": the first ones */
    bool changedPathsTruncated;

    CCiSnapshot( void );

    /** The changes in a few lines for a person */
    GUCEF::CORE::CString Describe( void ) const;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCISNAPSHOT_H ? */
