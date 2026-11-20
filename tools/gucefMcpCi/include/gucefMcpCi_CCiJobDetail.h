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

#ifndef GUCEF_MCPCI_CCIJOBDETAIL_H
#define GUCEF_MCPCI_CCIJOBDETAIL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiReport.h"

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
 *  One job of a run: its report and the full logs of the products that were asked for. A test application has no
 *  log of its own: its output is in the log of its product, and the report has the end of it and its failed cases.
 */
class CCiJobDetail
{
    public:

    class CLog
    {
        public:

        GUCEF::CORE::CString name;          /**< the product */
        GUCEF::CORE::CString fileName;      /**< the file it was published as */
        GUCEF::CORE::CString content;       /**< the full log; empty with a problem */
        GUCEF::CORE::CString problem;       /**< why the log is not there */
        GUCEF::CORE::CString digest;        /**< the digest of the full log made where the log was whole, empty when not made */
        GUCEF::UInt64 totalBytes;           /**< the size of the full log, 0 when not known */
        bool truncated;                     /**< content holds only the end of the log: a gateway sent at most its limit */

        CLog( void );
    };

    typedef GUCEF::vector< CLog > TLogVector;

    GUCEF::CORE::CString runId;
    GUCEF::CORE::CString jobId;
    CCiRunStatus::EState state;
    CCiReport::CCell cell;
    TLogVector logs;
    GUCEF::CORE::CString note;              /**< what the backend wants the reader to know, such as a missing codec */

    CCiJobDetail( void );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIJOBDETAIL_H ? */
