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

#ifndef GUCEF_MCPCI_CCIRUNSTATUS_H
#define GUCEF_MCPCI_CCIRUNSTATUS_H

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

/** The state of a run and its jobs, in the small set every backend maps to */
class CCiRunStatus
{
    public:

    enum EState
    {
        STATE_QUEUED = 0,
        STATE_RUNNING,
        STATE_SUCCEEDED,
        STATE_FAILED,
        STATE_CANCELLED,
        STATE_UNKNOWN
    };

    /** One job of a run: a cell of the matrix, or a step such as the planning */
    class CJob
    {
        public:

        GUCEF::CORE::CString id;        /**< the name of the job, unique within the run: what ci_job_details takes */
        EState state;
        GUCEF::UInt64 elapsedSecs;

        CJob( void );
    };

    typedef GUCEF::vector< CJob > TJobVector;

    GUCEF::CORE::CString runId;
    GUCEF::CORE::CString ciId;
    GUCEF::CORE::CString commit;
    GUCEF::CORE::CString title;
    EState state;
    GUCEF::UInt64 elapsedSecs;
    TJobVector jobs;

    CCiRunStatus( void );

    bool IsFinished( void ) const;

    GUCEF::UInt32 CountJobs( EState state ) const;

    /** The ids of the jobs in the state, comma separated, at most maxIds of them and "and N more" */
    GUCEF::CORE::CString ListJobs( EState state, GUCEF::UInt32 maxIds ) const;

    static const char* StateToString( EState state );

    static bool IsFinal( EState state );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIRUNSTATUS_H ? */
