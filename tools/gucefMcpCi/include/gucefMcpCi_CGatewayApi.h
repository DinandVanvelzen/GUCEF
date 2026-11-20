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

#ifndef GUCEF_MCPCI_CGATEWAYAPI_H
#define GUCEF_MCPCI_CGATEWAYAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefMcpCi_CDirectCiService.h"
#include "gucefMcpCi_CUploadStore.h"
#include "gucefMcpCi_CBundleReceiver.h"

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
 *  What the gateway API does for a request whose client is known, as synchronous code: CCiGateway runs Execute() as
 *  a task of its thread pool and answers the exchange when the task is done. The HTTP server only takes over exact
 *  paths, so the ids are in the query:
 *
 *    GET    /v1/ci                                 the CI ids and the limits of the gateway
 *    GET    /v1/ci?id=ID                           options of a CI id, its defaults and "have": the commits a snapshot
 *                                                  can be sent against as a delta, newest first
 *    POST   /v1/resolve                            { "remotes": [ ... ] } to the CI id
 *    POST   /v1/uploads?ci=ID&size=N&sha256=H      starts an upload of a git bundle: its id and the chunk size
 *    PUT    /v1/uploads?id=U&offset=N              a chunk (at most 8 MiB), strictly in order
 *    GET    /v1/uploads?id=U                       how far an upload is: where to go on after a chunk failed
 *    DELETE /v1/uploads?id=U                       gives an upload up
 *    POST   /v1/uploads/commit?id=U                checks size and SHA-256, takes the bundle into the bare repository
 *                                                  of the CI id and sends the snapshot to the backend: the snapshot
 *    POST   /v1/runs                               { "snapshot": U, "options": { ... } } starts a run: its run id
 *    GET    /v1/runs?id=R&wait=N                   the state of a run; with wait the gateway answers when the run is
 *                                                  finished or after N seconds, without holding a thread meanwhile
 *    GET    /v1/runs/report?id=R                   the report of a run
 *    GET    /v1/runs/jobs?id=R&job=J&product=P&part=digest|log_tail|tests&limit=L&offset=O&full=true
 *                                                  one job: its report, and per log its digest and the log within
 *                                                  bounds; with full=true the whole log up to the download limit
 *    POST   /v1/runs/cancel?id=R                   cancels a run
 *    POST   /v1/cleanup                            { "ci": ID, "olderThanDays": N } the retention sweep
 *
 *  Answers are JSON; errors are { "error": "...", "code": "..." } with a fitting HTTP status.
 *
 *  Thread-safe.
 */
class CGatewayApi
{
    public:

    /** One request, copied out of the exchange */
    class CRequest
    {
        public:

        typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TParameterMap;

        GUCEF::CORE::CString method;
        GUCEF::CORE::CString path;
        TParameterMap parameters;               /**< of the query, decoded */
        GUCEF::CORE::CDynamicBuffer body;
        GUCEF::CORE::CString machineId;         /**< the machine of the token */
        GUCEF::UInt64 pollDeadlineInMs;         /**< a waiting status: until when (CCiUtils::MonotonicMs()), 0 for no wait */

        CRequest( void );

        GUCEF::CORE::CString Parameter( const char* name ) const;
    };

    class CResponse
    {
        public:

        GUCEF::Int32 status;
        GUCEF::CORE::CString body;              /**< JSON */
        bool pollAgain;                         /**< a waiting status of a run that is not finished: ask again later */

        CResponse( void );
    };

    /** The routes: whether there is one for the method and the path, and the largest body it takes */
    static bool FindRoute( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& path, GUCEF::UInt32& maxBodyBytes, bool& pathKnown );

    /** The exact paths of the API */
    static GUCEF::CORE::CString::StringVector GetPaths( void );

    /** The largest body of a request other than a chunk */
    static const GUCEF::UInt32 MaxJsonBodyBytes = 65536;

    /** The service, the uploads and the receiver must outlive the API */
    CGatewayApi( CDirectCiService& service, CUploadStore& uploads, CBundleReceiver& receiver );

    /** The longest wait of a status; default 600 */
    void SetMaxWaitSeconds( GUCEF::UInt32 maxWaitSeconds );

    GUCEF::UInt32 GetMaxWaitSeconds( void ) const;

    /** The most log bytes one answer with full=true carries; default 24 MiB */
    void SetMaxFullLogBytes( GUCEF::UInt32 maxBytes );

    /** Carries out a request */
    void Execute( const CRequest& request, CResponse& response );

    /** Forgets the snapshots that were sent more than an hour ago */
    void RemoveExpiredSnapshots( GUCEF::UInt64 nowInMs );

    /** An error answer */
    static void SetError( CResponse& response, GUCEF::Int32 status, const char* code, const GUCEF::CORE::CString& message );

    /** The query of a request URI as parameters, decoded */
    static void ParseQuery( const GUCEF::CORE::CString& query, CRequest::TParameterMap& parameters );

    private:

    /** A snapshot that was sent, waiting for its run */
    class CSentSnapshot
    {
        public:

        GUCEF::CORE::CString ciId;
        GUCEF::CORE::CString machineId;
        CCiSnapshot snapshot;
        GUCEF::CORE::CString runId;             /**< of the run started on it, empty while there is none */
        bool starting;                          /**< a run is being started on it */
        GUCEF::UInt64 sentInMs;

        CSentSnapshot( void );
    };
    typedef GUCEF::map< GUCEF::CORE::CString, CSentSnapshot > TSentSnapshotMap;

    CGatewayApi( const CGatewayApi& src );                  /**< not implemented */
    CGatewayApi& operator=( const CGatewayApi& src );       /**< not implemented */

    void GetCi( const CRequest& request, CResponse& response );
    void Resolve( const CRequest& request, CResponse& response );
    void CreateUpload( const CRequest& request, CResponse& response );
    void WriteChunk( const CRequest& request, CResponse& response );
    void GetUpload( const CRequest& request, CResponse& response );
    void RemoveUpload( const CRequest& request, CResponse& response );
    void CommitUpload( const CRequest& request, CResponse& response );
    void StartRun( const CRequest& request, CResponse& response );
    void GetRun( const CRequest& request, CResponse& response );
    void GetReport( const CRequest& request, CResponse& response );
    void GetJob( const CRequest& request, CResponse& response );
    void CancelRun( const CRequest& request, CResponse& response );
    void Cleanup( const CRequest& request, CResponse& response );

    /** A failed call of the service as an answer */
    static void SetServiceError( CResponse& response, const GUCEF::CORE::CString& error );

    /** An upload result as an answer, false when it is not UPLOAD_OK */
    static bool SetUploadError( CResponse& response, CUploadStore::EResult result, const GUCEF::CORE::CString& problem );

    static void SetJson( CResponse& response, GUCEF::Int32 status, const GUCEF::CORE::CDataNode& json );

    static bool ReadJsonBody( const CRequest& request, CResponse& response, GUCEF::CORE::CDataNode& json );

    void AddLimits( GUCEF::CORE::CDataNode& json ) const;

    CDirectCiService& m_service;
    CUploadStore& m_uploads;
    CBundleReceiver& m_receiver;
    GUCEF::UInt32 m_maxWaitSeconds;
    GUCEF::UInt32 m_maxFullLogBytes;
    TSentSnapshotMap m_sent;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CGATEWAYAPI_H ? */
