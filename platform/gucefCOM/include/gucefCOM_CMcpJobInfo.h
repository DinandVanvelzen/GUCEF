/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPJOBINFO_H
#define GUCEF_COM_CMCPJOBINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What an MCP client sees of a job (CMcpJobBridge): an opaque handle, the kind, the state, the times, the progress and
 *  the outcome message of the scheduler run behind it. The run itself (its ids, its record) stays on the server side:
 *  ToDataNode() writes the job object a tool returns, without the run ids and without the result document reference.
 *
 *  States: queued (QUEUED, WAITING), working (RUNNING), completed (SUCCEEDED), failed (FAILED, TIMED_OUT, SKIPPED, or
 *  a run the scheduler no longer knows), cancelled (CANCELLED), interrupted (the run belonged to an earlier process of
 *  the server and did not end). The names that the tasks extension of MCP uses mean the same there.
 */
class GUCEF_COM_PUBLIC_CPP CMcpJobInfo
{
    public:

    static const CORE::CString StateQueued;         /**< "queued" */
    static const CORE::CString StateWorking;        /**< "working" */
    static const CORE::CString StateCompleted;      /**< "completed" */
    static const CORE::CString StateFailed;         /**< "failed" */
    static const CORE::CString StateCancelled;      /**< "cancelled" */
    static const CORE::CString StateInterrupted;    /**< "interrupted" */

    CORE::CString jobId;                /**< the handle: 22 characters of base64url over 128 random bits */
    CORE::CString kind;                 /**< what the job does, as the tool that started it names it */
    CORE::CString subject;              /**< the principal that started it: the only one that may see it (besides operators) */
    CORE::CString state;
    bool hasRun;
    CORE::SJobRunRecord run;            /**< the run as last seen */
    Int64 createdUtcMs;
    Int64 expiresUtcMs;                 /**< 0 while the job is not final; after it the handle is expired */

    CMcpJobInfo( void );

    /** Completed, failed, cancelled or interrupted */
    bool IsFinal( void ) const;

    static bool IsFinalState( const CORE::CString& state );

    /** The state of a job whose run is in the given state */
    static const CORE::CString& StateOfRun( CORE::TJobRunState runState );

    /**
     *  The job object: { "jobId", "kind", "state", "createdUtc", "startedUtc", "endedUtc", "progress": { "done",
     *  "total", "message" }, "message", "expiresUtc" }; times are ISO 8601 UTC text or null, "total" is null when
     *  unknown, "message" is the outcome message of the run (or null)
     */
    void ToDataNode( CORE::CDataNode& node ) const;

    /** The JSON Schema of the job object, for output schemas (CMcpSchemaBuilder) */
    static CORE::CDataNode GetSchema( void );

    /** A UTC time in ms since the Unix epoch as ISO 8601 text; 0 gives an empty text */
    static CORE::CString UtcMsToText( Int64 utcMs );
};

typedef GUCEF::vector< CMcpJobInfo > TMcpJobInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPJOBINFO_H ? */
