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

#ifndef GUCEF_MCPCI_CREMOTECISERVICE_H
#define GUCEF_MCPCI_CREMOTECISERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMcpCi_ICiService.h"
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
 *  The CI service of proxy mode: every call goes to a gateway (CGatewayApi) over HTTPS with the bearer token of this
 *  machine; this machine holds no CI credentials and needs no route to the CI, only to the gateway.
 *
 *  A submit makes the snapshot here with CSnapshotBuilder, as direct mode does (the repository of the user is not
 *  changed), and sends it as a git bundle: against the commits the gateway names as "have" that this repository has
 *  (an incremental bundle), else with its whole history. The bundle goes in chunks; a chunk that fails is resumed from
 *  where the gateway says the upload is, a few times. Then the gateway takes the bundle in and the run is started.
 *
 *  Requests verify the certificate of the gateway (with the CA file when given), have a time limit for the
 *  connection and for the whole request, and a size limit for the answer. A gateway that cannot be reached, or that
 *  does not answer with its API, gives a clear error and nothing waits longer than its time limit.
 *
 *  Thread-safe.
 */
class CRemoteCiService : public ICiService
{
    public:

    /** The snapshot builder must outlive the service */
    explicit CRemoteCiService( const CSnapshotBuilder& snapshotBuilder );

    virtual ~CRemoteCiService() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  @param url      the root of the gateway, https://host:port (http only to this machine)
     *  @param token    the bearer token of this machine; also registered with the redaction of the HTTP logs
     *  @param caFile   PEM file of the authorities that vouch for the gateway, empty for the system trust store
     */
    void SetGateway( const GUCEF::CORE::CString& url, const GUCEF::CORE::CString& token, const GUCEF::CORE::CString& caFile );

    /** The name of this machine, part of the scratch branches made by the snapshot builder */
    void SetMachineName( const GUCEF::CORE::CString& machineName );

    /** Where the bundles are made before they are sent */
    void SetWorkDir( const GUCEF::CORE::CString& workDir );

    /** The time limit for making the connection; default 10000 */
    void SetConnectTimeout( GUCEF::UInt32 timeoutInMs );

    /**
     *  Asks the gateway for its CI ids, for the instructions of the server.
     *  @return false with the reason when the gateway cannot be reached or does not answer with its API
     */
    bool Probe( GUCEF::CORE::CString& ciIds, GUCEF::CORE::CString& problem );

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

    /** The CI ids of the gateway as of the last answer that named them */
    virtual GUCEF::CORE::CString ListCiIds( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRemoteCiService( const CRemoteCiService& src );                /**< not implemented */
    CRemoteCiService& operator=( const CRemoteCiService& src );     /**< not implemented */

    /**
     *  One request to the gateway.
     *  @param body     the body, GUCEF_NULL for none
     *  @return false when no answer came; 'status' and 'answer' hold it otherwise
     */
    bool Execute( const GUCEF::CORE::CString& method             ,
                  const GUCEF::CORE::CString& pathAndQuery       ,
                  const GUCEF::CORE::CDynamicBuffer* body        ,
                  const char* contentType                        ,
                  GUCEF::UInt32 timeoutInMs                      ,
                  GUCEF::UInt64 maxAnswerBytes                   ,
                  GUCEF::Int32& status                           ,
                  GUCEF::CORE::CDynamicBuffer& answer            ,
                  GUCEF::CORE::CString& error                    ) const;

    /**
     *  A request with a JSON answer; an answer other than 2xx becomes 'error', with the message of the gateway.
     *  @param statusOut    optional: the HTTP status, 0 when no answer came
     */
    bool CallJson( const GUCEF::CORE::CString& method             ,
                   const GUCEF::CORE::CString& pathAndQuery       ,
                   const GUCEF::CORE::CString& jsonBody           ,
                   GUCEF::UInt32 timeoutInMs                      ,
                   GUCEF::UInt64 maxAnswerBytes                   ,
                   GUCEF::CORE::CDataNode& json                   ,
                   GUCEF::CORE::CString& error                    ,
                   GUCEF::Int32* statusOut = GUCEF_NULL           ) const;

    /** Sends the bundle in chunks, resuming after failed chunks. @return the id of the upload */
    bool Upload( const GUCEF::CORE::CString& ciId          ,
                 const GUCEF::CORE::CString& bundlePath    ,
                 ICiProgress* progress                     ,
                 GUCEF::CORE::CString& uploadId            ,
                 GUCEF::CORE::CString& error               ) const;

    /** The commits of 'have' to make the bundle against: those of the repository that the snapshot builds on */
    static GUCEF::CORE::CString::StringVector BundleBase( const CCiSnapshot& snapshot, const GUCEF::CORE::CString::StringVector& have );

    GUCEF::CORE::CString MakeBundlePath( void ) const;

    void RememberIds( const GUCEF::CORE::CDataNode& json ) const;

    GUCEF::CORE::CString GetUrl( void ) const;

    const CSnapshotBuilder& m_snapshotBuilder;
    GUCEF::CORE::CString m_url;
    GUCEF::CORE::CString m_token;
    GUCEF::CORE::CString m_caFile;
    GUCEF::CORE::CString m_machineName;
    GUCEF::CORE::CString m_workDir;
    GUCEF::UInt32 m_connectTimeoutInMs;
    mutable GUCEF::CORE::CString m_ciIds;
    mutable GUCEF::UInt32 m_bundleCounter;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CREMOTECISERVICE_H ? */
