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

#ifndef GUCEF_MCPCI_CDIRECTCISERVICE_H
#define GUCEF_MCPCI_CDIRECTCISERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "gucefMcpCi_ICiService.h"
#include "gucefMcpCi_ICiBackend.h"
#include "gucefMcpCi_CCiRegistry.h"
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
 *  The CI service on this machine: the registry says which repositories exist, the snapshot builder takes the work
 *  of a repository, and the backend of each CI id does the rest. Run ids are "<CI id>:<run id of the backend>".
 *
 *  Only repositories of the registry are built: a submit for a repository whose remotes are not those of the CI id
 *  is refused. Submitting also removes, at most every few hours per CI id, the scratch branches and reports older than
 *  the retention of the CI id.
 */
class CDirectCiService : public ICiService
{
    public:

    /** The registry and the snapshot builder must outlive the service */
    CDirectCiService( const CCiRegistry& registry, const CSnapshotBuilder& snapshotBuilder );
    virtual ~CDirectCiService() GUCEF_VIRTUAL_OVERRIDE;

    /** Adds a backend for the CI ids of its name; the service deletes it */
    void AddBackend( ICiBackend* backend );

    /** The name of this machine, part of the names of the scratch branches */
    void SetMachineName( const GUCEF::CORE::CString& machineName );

    /** How often a waiting Status() asks the backend */
    void SetPollInterval( GUCEF::UInt32 pollIntervalInMs );

    virtual bool Resolve( const GUCEF::CORE::CString& repoDir                     ,
                          const GUCEF::CORE::CString::StringVector& remotes       ,
                          GUCEF::CORE::CString& ciId                              ,
                          GUCEF::CORE::CString& description                       ,
                          GUCEF::CORE::CString& matchedRemote                     ,
                          GUCEF::CORE::CString::StringVector& examinedRemotes     ,
                          GUCEF::CORE::CString& error                             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Describe( const GUCEF::CORE::CString& ciId     ,
                           const GUCEF::CORE::CString& repoDir  ,
                           CCiDescription& description          ,
                           GUCEF::CORE::CString& error          ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Submit( const GUCEF::CORE::CString& ciId       ,
                         const GUCEF::CORE::CString& repoDir    ,
                         const CCiSubmitOptions& options        ,
                         CCiSnapshot& snapshot                  ,
                         GUCEF::CORE::CString& runId            ,
                         GUCEF::CORE::CString& error            ,
                         ICiProgress* progress = GUCEF_NULL     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Status( const GUCEF::CORE::CString& runId      ,
                         GUCEF::UInt32 waitSeconds              ,
                         CCiRunStatus& status                   ,
                         GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Report( const GUCEF::CORE::CString& runId      ,
                         CCiReport& report                      ,
                         GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool JobDetails( const GUCEF::CORE::CString& runId      ,
                             const GUCEF::CORE::CString& jobId      ,
                             const GUCEF::CORE::CString& product    ,
                             CCiJobDetail& detail                   ,
                             GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Cancel( const GUCEF::CORE::CString& runId      ,
                         GUCEF::CORE::CString& message          ,
                         GUCEF::CORE::CString& error            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Cleanup( const GUCEF::CORE::CString& ciId      ,
                          GUCEF::UInt32 olderThanDays           ,
                          GUCEF::CORE::CString& summary         ,
                          GUCEF::CORE::CString& error           ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString ListCiIds( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  What a gateway needs besides the calls of ICiService: snapshots that arrive as bundles instead of as a working
     *  tree of this machine
     */

    /** The registry the service works with */
    const CCiRegistry& GetRegistry( void ) const;

    /** The commits of the backend of the CI id that a snapshot can be sent against as a delta, newest first */
    bool ListBaseCommits( const GUCEF::CORE::CString& ciId, GUCEF::CORE::CString::StringVector& commits, GUCEF::CORE::CString& error );

    /** Fetches what the backend of the CI id has published into a bare repository of this machine */
    bool FetchPublished( const GUCEF::CORE::CString& ciId, const GUCEF::CORE::CString& bareRepoDir, GUCEF::CORE::CString& error );

    /** Sends a snapshot whose commit is in the repository snapshot.repoDir (a bare one) to the backend of the CI id */
    bool SendSnapshot( const GUCEF::CORE::CString& ciId, const CCiSnapshot& snapshot, GUCEF::CORE::CString& error );

    /**
     *  Starts a run on a snapshot that was sent with SendSnapshot(): the defaults of the CI id fill in what the options
     *  leave out, the mode and the filters are checked against the options of the registry
     */
    bool StartRun( const GUCEF::CORE::CString& ciId       ,
                   const CCiSnapshot& snapshot            ,
                   const CCiSubmitOptions& options        ,
                   GUCEF::CORE::CString& runId            ,
                   GUCEF::CORE::CString& error            );

    /** The URLs of the remotes of a repository (git remote -v) */
    static bool GetRemotes( const GUCEF::CORE::CString& repoDir, GUCEF::CORE::CString::StringVector& remotes, GUCEF::CORE::CString& error );

    private:

    typedef GUCEF::vector< ICiBackend* > TBackendVector;
    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::Int64 > TSweepMap;

    CDirectCiService( const CDirectCiService& src );                /**< not implemented */
    CDirectCiService& operator=( const CDirectCiService& src );     /**< not implemented */

    const CCiDefinition* FindCi( const GUCEF::CORE::CString& ciId, GUCEF::CORE::CString& error ) const;
    ICiBackend* FindBackend( const CCiDefinition& ci, GUCEF::CORE::CString& error ) const;

    /** The CI and the backend of a run id and the run id of the backend */
    bool SplitRunId( const GUCEF::CORE::CString& runId      ,
                     const CCiDefinition*& ci               ,
                     ICiBackend*& backend                   ,
                     GUCEF::CORE::CString& backendRunId     ,
                     GUCEF::CORE::CString& error            ) const;

    /** Checks the filters against the options of the CI, when it has them */
    bool CheckFilters( const CCiDefinition& ci, ICiBackend& backend, const GUCEF::CORE::CString& topLevel, const CCiSubmitOptions& options, GUCEF::CORE::CString& error ) const;

    void SweepIfDue( const CCiDefinition& ci, ICiBackend& backend );

    /** The options with the defaults of the CI id filled in */
    static CCiSubmitOptions WithDefaults( const CCiDefinition& ci, const CCiSubmitOptions& options );

    const CCiRegistry& m_registry;
    const CSnapshotBuilder& m_snapshotBuilder;
    TBackendVector m_backends;
    GUCEF::CORE::CString m_machineName;
    GUCEF::UInt32 m_pollIntervalInMs;
    GUCEF::MT::CMutex m_lock;
    TSweepMap m_lastSweeps;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CDIRECTCISERVICE_H ? */
