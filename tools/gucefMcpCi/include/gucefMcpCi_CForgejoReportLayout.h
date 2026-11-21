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

#ifndef GUCEF_MCPCI_CFORGEJOREPORTLAYOUT_H
#define GUCEF_MCPCI_CFORGEJOREPORTLAYOUT_H

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

/**
 *  Where the jobs of a Forgejo run publish their reports, the agreement between the cell script of the workflow and
 *  this server, kept in this one place:
 *
 *    the generic package registry of the reports owner (DefaultReportsOwner, the bot user whose token the cells use;
 *    CI_REPORT_UPLOAD_URL = <server>/api/packages/ci-bot/generic in the workflow), package PackageName, version
 *    MakeVersion( run, job ): CI_RUN_ID of the job (github.run_id), a '-', and CELL_ID, the id of the matrix cell,
 *    which is also the name of the job. The files:
 *      report.json             see CCiReport for its content
 *      <product>.log.gz        the full log of a product (configure, build and the runs of its test applications),
 *                              gzip compressed; test applications have no log file of their own
 *      <product>.log           the same uncompressed, used when the compressed one is missing or cannot be read
 *
 *    URL of a file: <url>/api/packages/<owner>/generic/<PackageName>/<version>/<file>
 *
 *    The images job publishes ImagesFileName (see CCiImageReport) in the same package, version MakeImagesVersion( run ):
 *    "<run>-images".
 */
class CForgejoReportLayout
{
    public:

    static const char* DefaultReportsOwner;     /**< "ci-bot" */
    static const char* PackageName;             /**< "ci-reports" */
    static const char* PackageType;             /**< "generic" */
    static const char* ReportFileName;          /**< "report.json" */
    static const char* CompressedLogSuffix;     /**< ".log.gz" */
    static const char* LogSuffix;               /**< ".log" */
    static const char* ImagesFileName;          /**< "images.json" */
    static const char* ImagesVersionSuffix;     /**< "images" */

    /** The package version of the image report of a run: "<runId>-images" */
    static GUCEF::CORE::CString MakeImagesVersion( const GUCEF::CORE::CString& runId );

    /** The package version of the report of a job: "<runId>-<jobId>" */
    static GUCEF::CORE::CString MakeVersion( const GUCEF::CORE::CString& runId, const GUCEF::CORE::CString& jobId );

    /** <url>/api/packages/<owner>/generic/<PackageName>/<version>/<file> */
    static GUCEF::CORE::CString FileUrl( const GUCEF::CORE::CString& baseUrl, const GUCEF::CORE::CString& owner,
                                         const GUCEF::CORE::CString& version, const GUCEF::CORE::CString& fileName );

    /** <url>/api/packages/<owner>/generic/<PackageName>/<version>, for DELETE */
    static GUCEF::CORE::CString VersionUrl( const GUCEF::CORE::CString& baseUrl, const GUCEF::CORE::CString& owner, const GUCEF::CORE::CString& version );

    /** <url>/api/v1/packages/<owner>?type=generic&q=<PackageName>&page=<page>&limit=<limit>: the versions */
    static GUCEF::CORE::CString ListUrl( const GUCEF::CORE::CString& baseUrl, const GUCEF::CORE::CString& owner, GUCEF::UInt32 page, GUCEF::UInt32 limit );

    /** The log file of a product when its report names none: "<name>.log.gz" */
    static GUCEF::CORE::CString DefaultLogFileName( const GUCEF::CORE::CString& name );

    /** The uncompressed variant of a log file name: "x.log.gz" becomes "x.log", others stay as they are */
    static GUCEF::CORE::CString PlainLogFileName( const GUCEF::CORE::CString& fileName );

    /** Whether the file name is that of a gzip compressed file */
    static bool IsCompressed( const GUCEF::CORE::CString& fileName );

    private:

    CForgejoReportLayout( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CFORGEJOREPORTLAYOUT_H ? */
