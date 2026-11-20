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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiJson.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Text( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString();
}

/*-------------------------------------------------------------------------*/

static Int32
Int( const CORE::CDataNode& node, const char* name, Int32 defaultValue )
{
    return node.GetAttributeValue( name ).AsInt32( defaultValue );
}

/*-------------------------------------------------------------------------*/

static UInt32
Count( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsUInt32( 0 );
}

/*-------------------------------------------------------------------------*/

static UInt64
Big( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsUInt64( 0 );
}

/*-------------------------------------------------------------------------*/

static bool
Flag( const CORE::CDataNode& node, const char* name, bool defaultValue )
{
    return node.GetAttributeValue( name ).AsBool( defaultValue );
}

/*-------------------------------------------------------------------------*/

/** The strings of an array child, empty ones included, which GetStringArray() leaves out */
static CORE::CString::StringVector
Strings( const CORE::CDataNode& node, const char* name )
{
    CORE::CString::StringVector values;
    const CORE::CDataNode* array = node.FindChild( name );
    if ( GUCEF_NULL == array )
        return values;
    CORE::CDataNode::TVariantVector items = array->GetChildrenValues();
    for ( size_t i=0; i<items.size(); ++i )
        values.push_back( items[ i ].AsString() );
    return values;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CCiJson::WriteStrings( const CORE::CString::StringVector& values, const char* name, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDataNode* array = node.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<values.size(); ++i )
        array->AddValueAsChild( CORE::CVariant( values[ i ] ) );
}

/*-------------------------------------------------------------------------*/

CCiRunStatus::EState
CCiJson::StateFromString( const CORE::CString& text )
{GUCEF_TRACE;

    const CCiRunStatus::EState states[] = { CCiRunStatus::STATE_QUEUED, CCiRunStatus::STATE_RUNNING, CCiRunStatus::STATE_SUCCEEDED,
                                            CCiRunStatus::STATE_FAILED, CCiRunStatus::STATE_CANCELLED, CCiRunStatus::STATE_UNKNOWN };
    for ( size_t i=0; i<sizeof( states ) / sizeof( states[ 0 ] ); ++i )
        if ( text == CCiRunStatus::StateToString( states[ i ] ) )
            return states[ i ];
    return CCiRunStatus::STATE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiSubmitOptions& options, CORE::CDataNode& node )
{GUCEF_TRACE;

    WriteStrings( options.toolchains, "toolchains", node );
    WriteStrings( options.cppStandards, "cppStandards", node );
    WriteStrings( options.products, "products", node );
    node.SetAttribute( "mode", options.mode );
    node.SetAttribute( "includeUncommitted", options.includeUncommitted );
    node.SetAttribute( "dryRun", options.dryRun );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiSubmitOptions& options )
{GUCEF_TRACE;

    options = CCiSubmitOptions();
    options.toolchains = CCiUtils::GetStringArray( node, "toolchains" );
    options.cppStandards = CCiUtils::GetStringArray( node, "cppStandards" );
    options.products = CCiUtils::GetStringArray( node, "products" );
    options.mode = CCiUtils::Trimmed( Text( node, "mode" ) );
    options.includeUncommitted = Flag( node, "includeUncommitted", true );
    options.dryRun = Flag( node, "dryRun", false );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiSnapshot& snapshot, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "baseCommit", snapshot.baseCommit );
    node.SetAttribute( "commit", snapshot.commit );
    node.SetAttribute( "branchName", snapshot.branchName );
    node.SetAttribute( "includesUncommitted", snapshot.includesUncommitted );
    node.SetAttribute( "filesInSnapshot", snapshot.filesInSnapshot );
    node.SetAttribute( "modifiedFiles", snapshot.modifiedFiles );
    node.SetAttribute( "addedFiles", snapshot.addedFiles );
    node.SetAttribute( "untrackedFiles", snapshot.untrackedFiles );
    node.SetAttribute( "deletedFiles", snapshot.deletedFiles );
    node.SetAttribute( "changedBytes", snapshot.changedBytes );
    node.SetAttribute( "hasSendEstimate", snapshot.hasSendEstimate );
    node.SetAttribute( "sendBytesEstimate", snapshot.sendBytesEstimate );
    node.SetAttribute( "unpublishedCommits", snapshot.unpublishedCommits );
    WriteStrings( snapshot.changedPaths, "changedPaths", node );
    node.SetAttribute( "changedPathsTruncated", snapshot.changedPathsTruncated );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiSnapshot& snapshot )
{GUCEF_TRACE;

    snapshot = CCiSnapshot();
    snapshot.baseCommit = Text( node, "baseCommit" );
    snapshot.commit = Text( node, "commit" );
    snapshot.branchName = Text( node, "branchName" );
    snapshot.includesUncommitted = Flag( node, "includesUncommitted", false );
    snapshot.filesInSnapshot = Count( node, "filesInSnapshot" );
    snapshot.modifiedFiles = Count( node, "modifiedFiles" );
    snapshot.addedFiles = Count( node, "addedFiles" );
    snapshot.untrackedFiles = Count( node, "untrackedFiles" );
    snapshot.deletedFiles = Count( node, "deletedFiles" );
    snapshot.changedBytes = Big( node, "changedBytes" );
    snapshot.hasSendEstimate = Flag( node, "hasSendEstimate", false );
    snapshot.sendBytesEstimate = Big( node, "sendBytesEstimate" );
    snapshot.unpublishedCommits = Count( node, "unpublishedCommits" );
    snapshot.changedPaths = Strings( node, "changedPaths" );
    snapshot.changedPathsTruncated = Flag( node, "changedPathsTruncated", false );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiDescription& description, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "ciId", description.ciId );
    node.SetAttribute( "source", description.source );
    CORE::CDataNode* toolchains = node.AddChild( "toolchains", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<description.toolchains.size(); ++i )
    {
        const CCiDescription::CToolchain& toolchain = description.toolchains[ i ];
        CORE::CDataNode* entry = toolchains->AddChild( "", GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "id", toolchain.id );
        entry->SetAttribute( "platform", toolchain.platform );
        entry->SetAttribute( "compiler", toolchain.compiler );
        entry->SetAttribute( "arch", toolchain.arch );
        entry->SetAttribute( "enabled", toolchain.enabled );
    }
    WriteStrings( description.cppStandards, "cppStandards", node );
    WriteStrings( description.buildTypes, "buildTypes", node );
    WriteStrings( description.products, "products", node );
    WriteStrings( description.modes, "modes", node );
    node.SetAttribute( "defaultCellCount", description.defaultCellCount );
    node.SetAttribute( "hasOptions", description.hasOptions );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiDescription& description )
{GUCEF_TRACE;

    description = CCiDescription();
    description.ciId = Text( node, "ciId" );
    description.source = Text( node, "source" );
    const CORE::CDataNode* toolchains = node.FindChild( "toolchains" );
    if ( GUCEF_NULL != toolchains )
    {
        for ( CORE::CDataNode::const_iterator i = toolchains->ConstBegin(); i != toolchains->ConstEnd(); ++i )
        {
            CCiDescription::CToolchain toolchain;
            toolchain.id = Text( *(*i), "id" );
            toolchain.platform = Text( *(*i), "platform" );
            toolchain.compiler = Text( *(*i), "compiler" );
            toolchain.arch = Text( *(*i), "arch" );
            toolchain.enabled = Flag( *(*i), "enabled", true );
            if ( !toolchain.id.IsNULLOrEmpty() )
                description.toolchains.push_back( toolchain );
        }
    }
    description.cppStandards = CCiUtils::GetStringArray( node, "cppStandards" );
    description.buildTypes = CCiUtils::GetStringArray( node, "buildTypes" );
    description.products = CCiUtils::GetStringArray( node, "products" );
    const CORE::CString::StringVector modes = CCiUtils::GetStringArray( node, "modes" );
    if ( !modes.empty() )
        description.modes = modes;
    description.defaultCellCount = Count( node, "defaultCellCount" );
    description.hasOptions = Flag( node, "hasOptions", false );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiRunStatus& status, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "runId", status.runId );
    node.SetAttribute( "ciId", status.ciId );
    node.SetAttribute( "commit", status.commit );
    node.SetAttribute( "title", status.title );
    node.SetAttribute( "state", CCiRunStatus::StateToString( status.state ) );
    node.SetAttribute( "elapsedSecs", status.elapsedSecs );
    CORE::CDataNode* jobs = node.AddChild( "jobs", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<status.jobs.size(); ++i )
    {
        CORE::CDataNode* job = jobs->AddChild( "", GUCEF_DATATYPE_OBJECT );
        job->SetAttribute( "id", status.jobs[ i ].id );
        job->SetAttribute( "state", CCiRunStatus::StateToString( status.jobs[ i ].state ) );
        job->SetAttribute( "elapsedSecs", status.jobs[ i ].elapsedSecs );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiRunStatus& status )
{GUCEF_TRACE;

    status = CCiRunStatus();
    status.runId = Text( node, "runId" );
    status.ciId = Text( node, "ciId" );
    status.commit = Text( node, "commit" );
    status.title = Text( node, "title" );
    status.state = StateFromString( Text( node, "state" ) );
    status.elapsedSecs = Big( node, "elapsedSecs" );
    const CORE::CDataNode* jobs = node.FindChild( "jobs" );
    if ( GUCEF_NULL == jobs )
        return;
    for ( CORE::CDataNode::const_iterator i = jobs->ConstBegin(); i != jobs->ConstEnd(); ++i )
    {
        CCiRunStatus::CJob job;
        job.id = Text( *(*i), "id" );
        job.state = StateFromString( Text( *(*i), "state" ) );
        job.elapsedSecs = Big( *(*i), "elapsedSecs" );
        status.jobs.push_back( job );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiReport::CCell& cell, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "jobId", cell.jobId );
    node.SetAttribute( "state", CCiRunStatus::StateToString( cell.state ) );
    node.SetAttribute( "hasReport", cell.hasReport );
    node.SetAttribute( "schema", cell.schema );
    node.SetAttribute( "cell", cell.cell );
    node.SetAttribute( "platform", cell.platform );
    node.SetAttribute( "compiler", cell.compiler );
    node.SetAttribute( "cppStandard", cell.cppStandard );
    node.SetAttribute( "buildType", cell.buildType );
    node.SetAttribute( "mode", cell.mode );
    node.SetAttribute( "status", cell.status );
    node.SetAttribute( "runId", cell.runId );
    node.SetAttribute( "commit", cell.commit );
    node.SetAttribute( "started", cell.started );
    node.SetAttribute( "seconds", cell.seconds );
    node.SetAttribute( "workspace", cell.workspace );
    node.SetAttribute( "problem", cell.problem );
    CORE::CDataNode* products = node.AddChild( "products", GUCEF_DATATYPE_ARRAY );
    for ( size_t p=0; p<cell.products.size(); ++p )
    {
        const CCiReport::CProduct& product = cell.products[ p ];
        CORE::CDataNode* productNode = products->AddChild( "", GUCEF_DATATYPE_OBJECT );
        productNode->SetAttribute( "name", product.name );
        productNode->SetAttribute( "status", product.status );
        productNode->SetAttribute( "phase", product.phase );
        productNode->SetAttribute( "exitCode", product.exitCode );
        productNode->SetAttribute( "seconds", product.seconds );
        productNode->SetAttribute( "logFile", product.logFile );
        productNode->SetAttribute( "logTail", product.logTail );
        CORE::CDataNode* tests = productNode->AddChild( "tests", GUCEF_DATATYPE_ARRAY );
        for ( size_t t=0; t<product.tests.size(); ++t )
        {
            const CCiReport::CTest& test = product.tests[ t ];
            CORE::CDataNode* testNode = tests->AddChild( "", GUCEF_DATATYPE_OBJECT );
            testNode->SetAttribute( "name", test.name );
            testNode->SetAttribute( "status", test.status );
            testNode->SetAttribute( "exitCode", test.exitCode );
            testNode->SetAttribute( "seconds", test.seconds );
            testNode->SetAttribute( "testCount", test.testCount );
            testNode->SetAttribute( "failureCount", test.failureCount );
            testNode->SetAttribute( "skippedCount", test.skippedCount );
            testNode->SetAttribute( "resultsFile", test.resultsFile );
            testNode->SetAttribute( "logTail", test.logTail );
            CORE::CDataNode* cases = testNode->AddChild( "failedCases", GUCEF_DATATYPE_ARRAY );
            for ( size_t c=0; c<test.failedCases.size(); ++c )
            {
                CORE::CDataNode* caseNode = cases->AddChild( "", GUCEF_DATATYPE_OBJECT );
                caseNode->SetAttribute( "suite", test.failedCases[ c ].suite );
                caseNode->SetAttribute( "name", test.failedCases[ c ].name );
                caseNode->SetAttribute( "message", test.failedCases[ c ].message );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiReport::CCell& cell )
{GUCEF_TRACE;

    cell = CCiReport::CCell();
    cell.jobId = Text( node, "jobId" );
    cell.state = StateFromString( Text( node, "state" ) );
    cell.hasReport = Flag( node, "hasReport", false );
    cell.schema = Int( node, "schema", cell.schema );
    cell.cell = Text( node, "cell" );
    cell.platform = Text( node, "platform" );
    cell.compiler = Text( node, "compiler" );
    cell.cppStandard = Text( node, "cppStandard" );
    cell.buildType = Text( node, "buildType" );
    cell.mode = Text( node, "mode" );
    cell.status = Text( node, "status" );
    cell.runId = Text( node, "runId" );
    cell.commit = Text( node, "commit" );
    cell.started = Text( node, "started" );
    cell.seconds = Count( node, "seconds" );
    cell.workspace = Text( node, "workspace" );
    cell.problem = Text( node, "problem" );
    const CORE::CDataNode* products = node.FindChild( "products" );
    if ( GUCEF_NULL == products )
        return;
    for ( CORE::CDataNode::const_iterator p = products->ConstBegin(); p != products->ConstEnd(); ++p )
    {
        const CORE::CDataNode& productNode = *(*p);
        CCiReport::CProduct product;
        product.name = Text( productNode, "name" );
        product.status = Text( productNode, "status" );
        product.phase = Text( productNode, "phase" );
        product.exitCode = Int( productNode, "exitCode", product.exitCode );
        product.seconds = Count( productNode, "seconds" );
        product.logFile = Text( productNode, "logFile" );
        product.logTail = Text( productNode, "logTail" );
        const CORE::CDataNode* tests = productNode.FindChild( "tests" );
        if ( GUCEF_NULL != tests )
        {
            for ( CORE::CDataNode::const_iterator t = tests->ConstBegin(); t != tests->ConstEnd(); ++t )
            {
                const CORE::CDataNode& testNode = *(*t);
                CCiReport::CTest test;
                test.name = Text( testNode, "name" );
                test.status = Text( testNode, "status" );
                test.exitCode = Int( testNode, "exitCode", test.exitCode );
                test.seconds = Count( testNode, "seconds" );
                test.testCount = Int( testNode, "testCount", -1 );
                test.failureCount = Int( testNode, "failureCount", -1 );
                test.skippedCount = Int( testNode, "skippedCount", -1 );
                test.resultsFile = Text( testNode, "resultsFile" );
                test.logTail = Text( testNode, "logTail" );
                const CORE::CDataNode* cases = testNode.FindChild( "failedCases" );
                if ( GUCEF_NULL != cases )
                {
                    for ( CORE::CDataNode::const_iterator c = cases->ConstBegin(); c != cases->ConstEnd(); ++c )
                    {
                        CCiReport::CTestCase testCase;
                        testCase.suite = Text( *(*c), "suite" );
                        testCase.name = Text( *(*c), "name" );
                        testCase.message = Text( *(*c), "message" );
                        test.failedCases.push_back( testCase );
                    }
                }
                product.tests.push_back( test );
            }
        }
        cell.products.push_back( product );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiReport& report, CORE::CDataNode& node )
{GUCEF_TRACE;

    Write( report.status, *node.AddChild( "status", GUCEF_DATATYPE_OBJECT ) );
    CORE::CDataNode* cells = node.AddChild( "cells", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<report.cells.size(); ++i )
        Write( report.cells[ i ], *cells->AddChild( "", GUCEF_DATATYPE_OBJECT ) );
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiReport& report )
{GUCEF_TRACE;

    report = CCiReport();
    const CORE::CDataNode* status = node.FindChild( "status" );
    if ( GUCEF_NULL != status )
        Read( *status, report.status );
    const CORE::CDataNode* cells = node.FindChild( "cells" );
    if ( GUCEF_NULL == cells )
        return;
    for ( CORE::CDataNode::const_iterator i = cells->ConstBegin(); i != cells->ConstEnd(); ++i )
    {
        CCiReport::CCell cell;
        Read( *(*i), cell );
        report.cells.push_back( cell );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Write( const CCiJobDetail& detail, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "runId", detail.runId );
    node.SetAttribute( "jobId", detail.jobId );
    node.SetAttribute( "state", CCiRunStatus::StateToString( detail.state ) );
    node.SetAttribute( "note", detail.note );
    Write( detail.cell, *node.AddChild( "cell", GUCEF_DATATYPE_OBJECT ) );
    CORE::CDataNode* logs = node.AddChild( "logs", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<detail.logs.size(); ++i )
    {
        const CCiJobDetail::CLog& log = detail.logs[ i ];
        CORE::CDataNode* entry = logs->AddChild( "", GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "name", log.name );
        entry->SetAttribute( "fileName", log.fileName );
        entry->SetAttribute( "content", log.content );
        entry->SetAttribute( "problem", log.problem );
        entry->SetAttribute( "digest", log.digest );
        entry->SetAttribute( "totalBytes", log.totalBytes );
        entry->SetAttribute( "truncated", log.truncated );
    }
}

/*-------------------------------------------------------------------------*/

void
CCiJson::Read( const CORE::CDataNode& node, CCiJobDetail& detail )
{GUCEF_TRACE;

    detail = CCiJobDetail();
    detail.runId = Text( node, "runId" );
    detail.jobId = Text( node, "jobId" );
    detail.state = StateFromString( Text( node, "state" ) );
    detail.note = Text( node, "note" );
    const CORE::CDataNode* cell = node.FindChild( "cell" );
    if ( GUCEF_NULL != cell )
        Read( *cell, detail.cell );
    const CORE::CDataNode* logs = node.FindChild( "logs" );
    if ( GUCEF_NULL == logs )
        return;
    for ( CORE::CDataNode::const_iterator i = logs->ConstBegin(); i != logs->ConstEnd(); ++i )
    {
        CCiJobDetail::CLog log;
        log.name = Text( *(*i), "name" );
        log.fileName = Text( *(*i), "fileName" );
        log.content = Text( *(*i), "content" );
        log.problem = Text( *(*i), "problem" );
        log.digest = Text( *(*i), "digest" );
        log.totalBytes = Big( *(*i), "totalBytes" );
        log.truncated = Flag( *(*i), "truncated", false );
        detail.logs.push_back( log );
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
