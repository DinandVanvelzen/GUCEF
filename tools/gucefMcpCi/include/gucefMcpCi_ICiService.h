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

#ifndef GUCEF_MCPCI_ICISERVICE_H
#define GUCEF_MCPCI_ICISERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiSubmitOptions.h"
#include "gucefMcpCi_CCiSnapshot.h"
#include "gucefMcpCi_CCiDescription.h"
#include "gucefMcpCi_CCiRunStatus.h"
#include "gucefMcpCi_CCiReport.h"
#include "gucefMcpCi_CCiJobDetail.h"
#include "gucefMcpCi_ICiProgress.h"

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
 *  The CI as the tools see it, the same in every mode of the server: CDirectCiService works with a backend itself,
 *  CRemoteCiService forwards the calls to a gateway. Run ids are opaque strings. Every call returns false
 *  with a message for the model in 'error' when it cannot do what was asked.
 *
 *  The calls are made from several threads at the same time.
 */
class ICiService
{
    public:

    virtual ~ICiService();

    /**
     *  The CI id of a repository: the remotes of 'repoDir' (git remote -v) and the given remotes are compared with
     *  the registry. False when no CI is set up for any of them: nothing is guessed.
     */
    virtual bool Resolve( const GUCEF::CORE::CString& repoDir                     ,
                          const GUCEF::CORE::CString::StringVector& remotes       ,
                          GUCEF::CORE::CString& ciId                              ,
                          GUCEF::CORE::CString& description                       ,
                          GUCEF::CORE::CString& matchedRemote                     ,
                          GUCEF::CORE::CString::StringVector& examinedRemotes     ,
                          GUCEF::CORE::CString& error                             ) = 0;

    virtual bool Describe( const GUCEF::CORE::CString& ciId     ,
                           const GUCEF::CORE::CString& repoDir  ,
                           CCiDescription& description          ,
                           GUCEF::CORE::CString& error          ) = 0;

    /**
     *  Makes the snapshot of the repository and starts a run on it; with options.dryRun only the snapshot.
     *  The repository must be one of the CI id.
     *  @param progress     optional: told how far sending the snapshot is, and asked whether to stop
     */
    virtual bool Submit( const GUCEF::CORE::CString& ciId       ,
                         const GUCEF::CORE::CString& repoDir    ,
                         const CCiSubmitOptions& options        ,
                         CCiSnapshot& snapshot                  ,
                         GUCEF::CORE::CString& runId            ,
                         GUCEF::CORE::CString& error            ,
                         ICiProgress* progress = GUCEF_NULL     ) = 0;

    /** The state of the run; with waitSeconds it waits until the run is finished or the time is up */
    virtual bool Status( const GUCEF::CORE::CString& runId      ,
                         GUCEF::UInt32 waitSeconds              ,
                         CCiRunStatus& status                   ,
                         GUCEF::CORE::CString& error            ) = 0;

    virtual bool Report( const GUCEF::CORE::CString& runId      ,
                         CCiReport& report                      ,
                         GUCEF::CORE::CString& error            ) = 0;

    virtual bool JobDetails( const GUCEF::CORE::CString& runId      ,
                             const GUCEF::CORE::CString& jobId      ,
                             const GUCEF::CORE::CString& product    ,
                             CCiJobDetail& detail                   ,
                             GUCEF::CORE::CString& error            ) = 0;

    virtual bool Cancel( const GUCEF::CORE::CString& runId      ,
                         GUCEF::CORE::CString& message          ,
                         GUCEF::CORE::CString& error            ) = 0;

    virtual bool Cleanup( const GUCEF::CORE::CString& ciId      ,
                          GUCEF::UInt32 olderThanDays           ,
                          GUCEF::CORE::CString& summary         ,
                          GUCEF::CORE::CString& error           ) = 0;

    /** The CI ids that can be used, comma separated */
    virtual GUCEF::CORE::CString ListCiIds( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_ICISERVICE_H ? */
