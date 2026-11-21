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

#ifndef GUCEF_MCPCI_CFORGEJOBACKEND_H
#define GUCEF_MCPCI_CFORGEJOBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefMcpCi_ICiBackend.h"
#include "gucefMcpCi_CSnapshotBuilder.h"

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
 *  Forgejo Actions (Forgejo 15, Gitea 1.22 API) as a CI backend. Everything happens in the scratch repository of the
 *  CI id:
 *
 *    submit       git push of the snapshot to its scratch branch (SendSnapshot), then (StartRun)
 *                 POST /api/v1/repos/{o}/{r}/actions/workflows/{file}/dispatches { ref, inputs, return_run_info };
 *                 when the answer holds no run, the run is found in GET .../actions/runs (event workflow_dispatch,
 *                 head_sha, the ref, newest first) with a short bounded retry
 *    status       GET .../actions/runs/{id}, and the jobs from GET .../actions/tasks (those with the run number and
 *                 commit of the run); GET .../commits/{sha}/statuses when the tasks hold none
 *    report       per job report.json from the generic package registry, see CForgejoReportLayout
 *    job details  the report and the logs of the job from the registry; gzip through the "gzip" codec of the codec
 *                 registry (codecspluginZLIB), the uncompressed log otherwise
 *    cancel       POST .../actions/runs/{id}/cancel; versions of Forgejo without it say so
 *    cleanup      GET and DELETE .../branches for scratch/ branches; GET /api/v1/packages/{owner}?type=generic and
 *                 DELETE /api/packages/{owner}/generic/{name}/{version} for reports
 *    base commits GET /api/v1/repos/{o}/{r} for the default branch and GET .../branches: its head and those of the
 *                 newest scratch branches
 *    fetch        git fetch of the branches of the scratch repository, the token in the environment of git as for the push
 *
 *  Requests carry "Authorization: token <token>", verify the certificate of an https server (with the CA file when
 *  given) and always have a time limit and a size limit for the response.
 */
class CForgejoBackend : public ICiBackend
{
    public:

    CForgejoBackend( void );
    virtual ~CForgejoBackend() GUCEF_VIRTUAL_OVERRIDE;

    /** The API token; also registered as a secret for the log redaction of the HTTP stack */
    void SetToken( const GUCEF::CORE::CString& token );

    /** PEM file with the authorities to trust for https, empty for the trust store of the system */
    void SetCaBundlePath( const GUCEF::CORE::CString& caBundlePath );

    /** What pushes the snapshots; must outlive the backend */
    void SetSnapshotPusher( const CSnapshotBuilder* pusher );

    /** The owner of the report packages of CI ids whose registry entry names none; default "ci-bot" */
    void SetDefaultReportsOwner( const GUCEF::CORE::CString& owner );

    /** The owner of the report packages of a CI id */
    GUCEF::CORE::CString GetReportsOwner( const CCiDefinition& ci ) const;

    /** How often and how long apart the run of a dispatch is looked for when the dispatch does not name it */
    void SetRunDiscovery( GUCEF::UInt32 attempts, GUCEF::UInt32 delayInMs );

    virtual GUCEF::CORE::CString GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Describe( const CCiDefinition& ci                ,
                           const GUCEF::CORE::CString& repoDir    ,
                           CCiDescription& description            ,
                           GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendSnapshot( const CCiDefinition& ci         ,
                               const CCiSnapshot& snapshot     ,
                               GUCEF::CORE::CString& error     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartRun( const CCiDefinition& ci             ,
                           const CCiSnapshot& snapshot         ,
                           const CCiSubmitOptions& options     ,
                           GUCEF::CORE::CString& runId         ,
                           GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ListBaseCommits( const CCiDefinition& ci                      ,
                                  GUCEF::CORE::CString::StringVector& commits  ,
                                  GUCEF::CORE::CString& error                  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FetchPublished( const CCiDefinition& ci                ,
                                 const GUCEF::CORE::CString& bareRepoDir ,
                                 GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetRun( const CCiDefinition& ci             ,
                         const GUCEF::CORE::CString& runId   ,
                         CCiRunStatus& status                ,
                         bool& isUnknown                     ,
                         GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetReport( const CCiDefinition& ci             ,
                            const GUCEF::CORE::CString& runId   ,
                            CCiReport& report                   ,
                            bool& isUnknown                     ,
                            GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetJobDetail( const CCiDefinition& ci             ,
                               const GUCEF::CORE::CString& runId   ,
                               const GUCEF::CORE::CString& jobId   ,
                               const GUCEF::CORE::CString& product ,
                               CCiJobDetail& detail                ,
                               bool& isUnknown                     ,
                               GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Cancel( const CCiDefinition& ci             ,
                         const GUCEF::CORE::CString& runId   ,
                         GUCEF::CORE::CString& message       ,
                         bool& isUnknown                     ,
                         GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Cleanup( const CCiDefinition& ci             ,
                          GUCEF::UInt32 olderThanDays         ,
                          GUCEF::CORE::CString& summary       ,
                          GUCEF::CORE::CString& error         ) GUCEF_VIRTUAL_OVERRIDE;

    /** Forgejo states as the states of the interface */
    static CCiRunStatus::EState MapState( const GUCEF::CORE::CString& forgejoStatus );

    /**
     *  Expands gzip data with the "gzip" codec of the codec registry, giving up past maxSize bytes of output.
     *  @return false when there is no such codec or the data is not valid gzip
     */
    static bool Gunzip( const GUCEF::CORE::CDynamicBuffer& compressed, GUCEF::UInt64 maxSize, GUCEF::CORE::CDynamicBuffer& raw, GUCEF::CORE::CString& problem );

    private:

    /** One request. @return false when no response came; the status is in 'status' otherwise */
    bool Execute( const GUCEF::CORE::CString& method       ,
                  const GUCEF::CORE::CString& url          ,
                  const GUCEF::CORE::CString& jsonBody     ,
                  GUCEF::UInt64 maxResponseBytes           ,
                  GUCEF::UInt32 timeoutInMs                ,
                  GUCEF::Int32& status                     ,
                  GUCEF::CORE::CDynamicBuffer& body        ,
                  GUCEF::CORE::CString& error              ) const;

    /** A request with a JSON answer; 2xx with a body that is not JSON is an error, other statuses are returned */
    bool ExecuteJson( const GUCEF::CORE::CString& method       ,
                      const GUCEF::CORE::CString& url          ,
                      const GUCEF::CORE::CString& jsonBody     ,
                      GUCEF::Int32& status                     ,
                      GUCEF::CORE::CDataNode& json             ,
                      GUCEF::CORE::CString& error              ) const;

    /** <url>/api/v1/repos/<scratchOwner>/<scratchRepo> */
    GUCEF::CORE::CString ScratchApi( const CCiDefinition& ci ) const;

    /** <url>/api/packages/<owner>/generic/<package>/<version>/<file> */
    GUCEF::CORE::CString PackageFileUrl( const CCiDefinition& ci, const GUCEF::CORE::CString& version, const GUCEF::CORE::CString& fileName ) const;

    bool LoadRun( const CCiDefinition& ci, const GUCEF::CORE::CString& runId, CCiRunStatus& status, bool& isUnknown, GUCEF::CORE::CString& error ) const;
    bool LoadJobs( const CCiDefinition& ci, GUCEF::Int64 runNumber, CCiRunStatus& status, GUCEF::CORE::CString& error ) const;
    bool LoadJobsFromStatuses( const CCiDefinition& ci, CCiRunStatus& status ) const;
    bool DiscoverRun( const CCiDefinition& ci, const CCiSnapshot& snapshot, GUCEF::CORE::CString& runId, GUCEF::CORE::CString& error ) const;

    /** report.json of a job. @return false with 'missing' set when the job published none */
    bool LoadCellReport( const CCiDefinition& ci, const GUCEF::CORE::CString& runId, const GUCEF::CORE::CString& jobId, CCiReport::CCell& cell, bool& missing, GUCEF::CORE::CString& error ) const;

    /** A log of a job, uncompressed. @return false with the reason in 'problem' */
    bool LoadLog( const CCiDefinition& ci, const GUCEF::CORE::CString& version, const GUCEF::CORE::CString& fileName, GUCEF::CORE::CString& content, GUCEF::CORE::CString& note, GUCEF::CORE::CString& problem ) const;

    GUCEF::CORE::CString m_token;
    GUCEF::CORE::CString m_caBundlePath;
    const CSnapshotBuilder* m_pusher;
    GUCEF::CORE::CString m_defaultReportsOwner;
    GUCEF::UInt32 m_discoveryAttempts;
    GUCEF::UInt32 m_discoveryDelayInMs;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CFORGEJOBACKEND_H ? */
