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

#ifndef GUCEF_MCPCI_CSNAPSHOTBUILDER_H
#define GUCEF_MCPCI_CSNAPSHOTBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiSnapshot.h"

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
 *  Takes everything of a repository that is not published yet (commits that are not pushed, modified, deleted and
 *  untracked files) as one synthetic commit, and sends it to a scratch repository. Git does the work:
 *
 *    1. a temporary index (GIT_INDEX_FILE in a temporary directory) gets the tree of HEAD (git read-tree, keeping
 *       the file information of the index of the user so that unchanged files are not read again), then the working
 *       tree (git add -A, which honours .gitignore); git write-tree and git commit-tree -p HEAD make the commit, with
 *       the identity given on the command line so that no git configuration is needed
 *    2. the index, the working tree, the branches and the refs of the repository are never changed: only objects
 *       are added to its object store
 *    3. guards: a size cap; untracked paths that look like secrets (*.env, secrets.*, *.pem, *.key, id_rsa*, .netrc,
 *       *.token and the like) are refused unless the user tracks them
 *    4. the commit is pushed as scratch/<machine>-<utc>-<shortsha>. The token for the push goes to git through the
 *       environment (GIT_CONFIG_COUNT, git 2.31 and later): never in the URL, the arguments or a message
 *
 *  Thread-safe: several snapshots can be made at the same time.
 */
class CSnapshotBuilder
{
    public:

    static const GUCEF::UInt64 DefaultMaxBytes;     /**< 500 MiB */

    CSnapshotBuilder( void );

    /** The largest snapshot that is sent: the changed files, and the objects to send when git can estimate them */
    void SetMaxBytes( GUCEF::UInt64 maxBytes );
    GUCEF::UInt64 GetMaxBytes( void ) const;

    /** Where the temporary indexes are made, a directory of this process; the system temporary directory by default */
    void SetWorkDir( const GUCEF::CORE::CString& workDir );

    /** The author and committer of the synthetic commits */
    void SetCommitIdentity( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& email );

    /**
     *  Makes the snapshot of the repository that holds 'repoDir'.
     *  @param includeUncommitted   false: HEAD as it is, the working tree is left out
     *  @param machineName          part of the branch name
     *  @return false with the reason in 'error'
     */
    bool Build( const GUCEF::CORE::CString& repoDir      ,
                bool includeUncommitted                  ,
                const GUCEF::CORE::CString& machineName  ,
                CCiSnapshot& snapshot                    ,
                GUCEF::CORE::CString& error              ) const;

    /**
     *  Pushes the commit of the snapshot to its branch in the repository at 'remoteUrl'.
     *  @param authorizationHeader  the value of the Authorization header git sends over HTTP, empty for none; never
     *                              part of the arguments or of 'error'
     */
    bool Push( const CCiSnapshot& snapshot                       ,
               const GUCEF::CORE::CString& remoteUrl             ,
               const GUCEF::CORE::CString& authorizationHeader   ,
               GUCEF::CORE::CString& error                       ) const;

    /**
     *  Fetches from the repository at 'remoteUrl' into the repository 'repoDir' (a bare one on a gateway) with the
     *  refspec, pruning what the remote no longer has. The credential reaches git as for Push().
     */
    bool Fetch( const GUCEF::CORE::CString& repoDir                 ,
                const GUCEF::CORE::CString& remoteUrl               ,
                const GUCEF::CORE::CString& authorizationHeader     ,
                const GUCEF::CORE::CString& refspec                 ,
                GUCEF::CORE::CString& error                         ) const;

    /**
     *  Writes the commit of the snapshot as a git bundle to 'bundlePath', without the history that the commits in
     *  'excludedCommits' already have: an incremental bundle, or the whole history when the list is empty. The
     *  repository of the user is not changed: the bundle is made in a temporary repository of this process that
     *  borrows the objects of the repository (objects/info/alternates) and names the commit as BundleRef.
     *  @param excludedCommits  commits the receiver has; only those that exist in the repository may be given
     */
    bool CreateBundle( const CCiSnapshot& snapshot                         ,
                       const GUCEF::CORE::CString::StringVector& excludedCommits ,
                       const GUCEF::CORE::CString& bundlePath              ,
                       GUCEF::CORE::CString& error                         ) const;

    /** Whether the repository 'repoDir' has the commit 'commit' */
    static bool HasCommit( const GUCEF::CORE::CString& repoDir, const GUCEF::CORE::CString& commit );

    /** The ref under which a bundle of CreateBundle() carries the commit: "refs/heads/snapshot" */
    static const char* BundleRef;

    /** Whether the file name of the path looks like it holds a secret */
    static bool IsSecretLookingPath( const GUCEF::CORE::CString& path );

    /** scratch/<machine>-<utc>-<first 10 of the commit> */
    static GUCEF::CORE::CString MakeBranchName( const GUCEF::CORE::CString& machineName, const GUCEF::CORE::CString& commit );

    /** The prefix of the branches of snapshots: "scratch/" */
    static const char* BranchPrefix;

    /** Whether the git of this machine takes configuration from the environment (2.31 and later) */
    static bool GitTakesConfigFromEnvironment( GUCEF::CORE::CString& version );

    private:

    bool BuildWithWorkingTree( const GUCEF::CORE::CString& topLevel ,
                               CCiSnapshot& snapshot               ,
                               GUCEF::CORE::CString& error         ) const;

    void EstimateSendSize( const GUCEF::CORE::CString& topLevel, CCiSnapshot& snapshot ) const;

    /** The environment that hands the Authorization header to git. @return false when this git cannot take it */
    static bool CredentialEnvironment( const GUCEF::CORE::CString& authorizationHeader, GUCEF::CORE::CProcess::TStringMap& env, GUCEF::CORE::CString& error );

    GUCEF::CORE::CString MakeTempDir( void ) const;

    GUCEF::UInt64 m_maxBytes;
    GUCEF::CORE::CString m_workDir;
    GUCEF::CORE::CString m_commitName;
    GUCEF::CORE::CString m_commitEmail;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CSNAPSHOTBUILDER_H ? */
