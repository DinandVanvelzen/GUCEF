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

#ifndef GUCEF_MCPCI_ICIBACKEND_H
#define GUCEF_MCPCI_ICIBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiDefinition.h"
#include "gucefMcpCi_CCiSubmitOptions.h"
#include "gucefMcpCi_CCiSnapshot.h"
#include "gucefMcpCi_CCiDescription.h"
#include "gucefMcpCi_CCiRunStatus.h"
#include "gucefMcpCi_CCiReport.h"
#include "gucefMcpCi_CCiJobDetail.h"
#include "gucefMcpCi_CCiImageReport.h"

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
 *  A CI product behind the backend-neutral interface: Forgejo Actions first, others later. The run ids it hands
 *  out are its own, opaque to everybody else. Which repositories exist is decided by the registry, not by the
 *  backend; the backend is told the entry of the registry with every call.
 *
 *  The calls are made from several threads at the same time.
 */
class ICiBackend
{
    public:

    virtual ~ICiBackend();

    /** The name the registry uses for the backend: "forgejo" */
    virtual GUCEF::CORE::CString GetBackendName( void ) const = 0;

    /**
     *  The options of the CI. With a repository directory the backend may read them from the files of the working
     *  tree, since that is what a snapshot of it builds.
     */
    virtual bool Describe( const CCiDefinition& ci                ,
                           const GUCEF::CORE::CString& repoDir    ,
                           CCiDescription& description            ,
                           GUCEF::CORE::CString& error            ) = 0;

    /** Sends the snapshot in the way of the backend and starts a run with the options: SendSnapshot(), then StartRun() */
    virtual bool Submit( const CCiDefinition& ci             ,
                         const CCiSnapshot& snapshot         ,
                         const CCiSubmitOptions& options     ,
                         GUCEF::CORE::CString& runId         ,
                         GUCEF::CORE::CString& error         );

    /** Sends the commit of the snapshot, which is in the repository snapshot.repoDir, to the backend as snapshot.branchName */
    virtual bool SendSnapshot( const CCiDefinition& ci         ,
                               const CCiSnapshot& snapshot     ,
                               GUCEF::CORE::CString& error     ) = 0;

    /** Starts a run with the options on a snapshot that was sent */
    virtual bool StartRun( const CCiDefinition& ci             ,
                           const CCiSnapshot& snapshot         ,
                           const CCiSubmitOptions& options     ,
                           GUCEF::CORE::CString& runId         ,
                           GUCEF::CORE::CString& error         ) = 0;

    /**
     *  The commits the backend has that a snapshot can be sent against as a delta, newest first: the head of the
     *  default branch of the scratch repository and those of its newest scratch branches
     */
    virtual bool ListBaseCommits( const CCiDefinition& ci                      ,
                                  GUCEF::CORE::CString::StringVector& commits  ,
                                  GUCEF::CORE::CString& error                  ) = 0;

    /**
     *  Fetches the branches of the scratch repository into the bare repository 'bareRepoDir', under refs/remotes/scratch/,
     *  so that a delta against the commits of ListBaseCommits() can be applied there
     */
    virtual bool FetchPublished( const CCiDefinition& ci                ,
                                 const GUCEF::CORE::CString& bareRepoDir ,
                                 GUCEF::CORE::CString& error            ) = 0;

    /** @param isUnknown    set when the run does not exist */
    virtual bool GetRun( const CCiDefinition& ci             ,
                         const GUCEF::CORE::CString& runId   ,
                         CCiRunStatus& status                ,
                         bool& isUnknown                     ,
                         GUCEF::CORE::CString& error         ) = 0;

    virtual bool GetReport( const CCiDefinition& ci             ,
                            const GUCEF::CORE::CString& runId   ,
                            CCiReport& report                   ,
                            bool& isUnknown                     ,
                            GUCEF::CORE::CString& error         ) = 0;

    /**
     *  The report of one job and the full logs of its failed products and test applications, or of the one named
     *  @param product      a product or test application to fetch the log of; empty: those that failed
     */
    virtual bool GetJobDetail( const CCiDefinition& ci             ,
                               const GUCEF::CORE::CString& runId   ,
                               const GUCEF::CORE::CString& jobId   ,
                               const GUCEF::CORE::CString& product ,
                               CCiJobDetail& detail                ,
                               bool& isUnknown                     ,
                               GUCEF::CORE::CString& error         ) = 0;

    /** @param message  what happened, for the reader */
    virtual bool Cancel( const CCiDefinition& ci             ,
                         const GUCEF::CORE::CString& runId   ,
                         GUCEF::CORE::CString& message       ,
                         bool& isUnknown                     ,
                         GUCEF::CORE::CString& error         ) = 0;

    /** Removes the scratch branches and reports older than the given number of days */
    virtual bool Cleanup( const CCiDefinition& ci             ,
                          GUCEF::UInt32 olderThanDays         ,
                          GUCEF::CORE::CString& summary       ,
                          GUCEF::CORE::CString& error         ) = 0;

    /**
     *  The images a run built (images.json of its images job).
     *  @param missing  set when the run published none: it built no images, or the images job did not get to it
     */
    virtual bool GetImageReport( const CCiDefinition& ci             ,
                                 const GUCEF::CORE::CString& runId   ,
                                 CCiImageReport& report              ,
                                 bool& missing                       ,
                                 GUCEF::CORE::CString& error         ) = 0;

    /**
     *  Fetches the published repository of the CI id (owner/repo, the pull mirror) into the bare repository
     *  'bareRepoDir': its branches under PublishedRefPrefix, its default branch (HEAD) as PublishedHeadRef
     */
    virtual bool FetchPublishedRepository( const CCiDefinition& ci                 ,
                                           const GUCEF::CORE::CString& bareRepoDir ,
                                           GUCEF::CORE::CString& error             ) = 0;

    /** The newest run of the scratch repository whose head is the commit; false with 'error' when there is none */
    virtual bool FindRunOfCommit( const CCiDefinition& ci             ,
                                  const GUCEF::CORE::CString& commit  ,
                                  GUCEF::CORE::CString& runId         ,
                                  GUCEF::CORE::CString& error         ) = 0;

    /** Where FetchPublishedRepository() puts the branches of the published repository: "refs/remotes/published/" */
    static const char* PublishedRefPrefix;

    /** Where FetchPublishedRepository() puts the default branch of the published repository: "refs/ci/published-head" */
    static const char* PublishedHeadRef;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_ICIBACKEND_H ? */
