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

#ifndef GUCEF_MCPCI_CCIREPORT_H
#define GUCEF_MCPCI_CCIREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMcpCi_CCiRunStatus.h"
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
 *  The outcome of a run per job. Each job of the matrix (one cell) publishes report.json, written by
 *  projects/CI/scripts/ci_cell.sh (schema 1; the python writer, or the plain bash writer when the runner has no
 *  python3, which leaves out the test counts, resultsFile, the logTail of tests and the names of failed cases):
 *
 *      {
 *        "schema": 1,
 *        "cell": "gcc-linux-x64-cxx17-Release",          the id of the matrix cell, also the name of the job
 *        "platform": "linux64", "compiler": "gcc", "cppStandard": "17", "buildType": "Release",
 *        "mode": "build" | "test",
 *        "runId": "...", "commit": "<sha>", "ref": "refs/heads/...",
 *        "status": "ok" | "failed",
 *        "started": "2026-11-23T02:42:09Z", "seconds": 312,
 *        "products": [
 *          { "name": "GUCEF_tag_platform",
 *            "status": "ok" | "failed",
 *            "phase": "configure" | "build" | "test",       the phase it got to: where it failed when it failed
 *            "exitCode": 1, "seconds": 120,
 *            "logFile": "GUCEF_tag_platform.log.gz",         the full log of configure, build and the test runs
 *            "logTail": "the last 200 lines of the log",
 *            "tests": [                                      only in test mode
 *              { "name": "gucefCORE_TestApp",               the executable, with _d in a debug build
 *                "status": "ok" | "failed" | "timeout" | "skipped",
 *                "exitCode": 1, "seconds": 3,
 *                "tests": 40, "failures": 1, "skipped": 0,   python writer only
 *                "resultsFile": "gucefCORE_TestApp_Results.xml",
 *                "logTail": "the last 50 lines of its output",   python writer, only when it did not succeed
 *                "failedCases": [ { "suite": "...", "name": "Test: ...", "message": "... at most 500 characters" } ] } ] } ]
 *      }
 *
 *  Test applications have no log file of their own: their output is part of the log of their product, and the
 *  report holds the end of it.
 *
 *  The images job publishes no report.json but images.json (CCiImageReport); its cell is made from that: mode
 *  "images", and per image a product of phase ImagePhase with the status, tag, digest, seconds and the end of the log
 *  of the image build as the report gives them, and "<name>.log.gz" as its log. Fields that are not known are ignored; a "workspace" (the checkout on the runner) is
 *  used when a report has one, otherwise it is derived from the build directory in the logs.
 */
class CCiReport
{
    public:

    /** A failed test case as the results file of the test framework names it */
    class CTestCase
    {
        public:

        GUCEF::CORE::CString suite;
        GUCEF::CORE::CString name;
        GUCEF::CORE::CString message;

        CTestCase( void );

        /** "suite: name - message", at most maxChars characters */
        GUCEF::CORE::CString Describe( GUCEF::UInt32 maxChars ) const;
    };

    typedef GUCEF::vector< CTestCase > TTestCaseVector;

    /** One test application that ran in the build of a product */
    class CTest
    {
        public:

        GUCEF::CORE::CString name;
        GUCEF::CORE::CString status;            /**< as the report gives it: ok, failed, timeout, skipped */
        GUCEF::Int32 exitCode;
        GUCEF::UInt32 seconds;
        GUCEF::Int32 testCount;                 /**< -1 when the report does not say */
        GUCEF::Int32 failureCount;              /**< -1 when the report does not say */
        GUCEF::Int32 skippedCount;              /**< -1 when the report does not say */
        GUCEF::CORE::CString resultsFile;
        GUCEF::CORE::CString logTail;
        TTestCaseVector failedCases;

        CTest( void );
        bool IsFailed( void ) const;
        bool IsSkipped( void ) const;
    };

    typedef GUCEF::vector< CTest > TTestVector;

    /** One product (CMake target) of the cell */
    class CProduct
    {
        public:

        GUCEF::CORE::CString name;
        GUCEF::CORE::CString status;            /**< as the report gives it: ok, failed */
        GUCEF::CORE::CString phase;             /**< configure, build or test */
        GUCEF::Int32 exitCode;
        GUCEF::UInt32 seconds;
        GUCEF::CORE::CString logFile;
        GUCEF::CORE::CString logTail;
        TTestVector tests;
        GUCEF::CORE::CString tag;               /**< an image: its tag */
        GUCEF::CORE::CString digest;            /**< an image: its digest, when it was pushed */

        CProduct( void );

        /** Whether it is an image of the images job */
        bool IsImage( void ) const;
        bool IsFailed( void ) const;
        GUCEF::UInt32 CountFailedTests( void ) const;
    };

    typedef GUCEF::vector< CProduct > TProductVector;

    /** One job of the run with its report, when it published one */
    class CCell
    {
        public:

        GUCEF::CORE::CString jobId;
        CCiRunStatus::EState state;             /**< of the job */
        bool hasReport;                         /**< false: the job published no report (not run, or it failed before) */
        GUCEF::Int32 schema;
        GUCEF::CORE::CString cell;
        GUCEF::CORE::CString platform;
        GUCEF::CORE::CString compiler;
        GUCEF::CORE::CString cppStandard;
        GUCEF::CORE::CString buildType;
        GUCEF::CORE::CString mode;
        GUCEF::CORE::CString status;            /**< as the report gives it: ok, failed */
        GUCEF::CORE::CString runId;
        GUCEF::CORE::CString commit;
        GUCEF::CORE::CString started;
        GUCEF::UInt32 seconds;
        GUCEF::CORE::CString workspace;         /**< the checkout on the runner, empty when not known */
        GUCEF::CORE::CString problem;           /**< why the report could not be read, when it could not */
        TProductVector products;

        CCell( void );

        /** Reads report.json */
        bool LoadFromReport( const GUCEF::CORE::CDataNode& report );

        /** The cell of the images job, from images.json */
        void LoadFromImageReport( const CCiImageReport& report );

        bool IsFailed( void ) const;
        GUCEF::UInt32 CountFailedProducts( void ) const;
        GUCEF::UInt32 CountFailedTests( void ) const;

        /** The test application with the name, GUCEF_NULL when there is none; 'product' gets its product */
        const CTest* FindTest( const GUCEF::CORE::CString& name, const CProduct** product ) const;
    };

    typedef GUCEF::vector< CCell > TCellVector;

    /** The phase of the products of the images job: "image" */
    static const char* ImagePhase;

    CCiRunStatus status;
    TCellVector cells;

    CCiReport( void );

    /** A status word of the report as the tools write it: ok becomes succeeded, timeout becomes timed out */
    static GUCEF::CORE::CString DescribeStatus( const GUCEF::CORE::CString& status );

    /** Whether a status word of the report means a failure: failed, timeout (and failure, error) */
    static bool IsFailedStatus( const GUCEF::CORE::CString& status );

    /**
     *  The checkout on the runner, derived from a log: the part before /common/bin/<cell>/ of the build directory
     *  that CMake names. Empty when the log does not show it.
     */
    static GUCEF::CORE::CString GuessWorkspace( const GUCEF::CORE::CString& log, const GUCEF::CORE::CString& cell );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIREPORT_H ? */
