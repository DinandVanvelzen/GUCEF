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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCiReport::ImagePhase = "image";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A text attribute, trimmed */
static CORE::CString
Text( const CORE::CDataNode& node, const char* name )
{
    return CCiUtils::Trimmed( node.GetAttributeValue( name ).AsString() );
}

/*-------------------------------------------------------------------------*/

/** A status word, lower case */
static CORE::CString
Word( const CORE::CDataNode& node, const char* name )
{
    return Text( node, name ).Lowercase();
}

/*-------------------------------------------------------------------------*/

/** The first of two attributes that is there, for the names of older or other writers */
static CORE::CString
Either( const CORE::CDataNode& node, const char* name, const char* otherName )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    if ( GUCEF_NULL == attribute )
        attribute = node.GetAttribute( otherName );
    return GUCEF_NULL != attribute ? (*attribute).second.AsString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/** A count that may be missing or null: -1 then */
static Int32
Count( const CORE::CDataNode& node, const char* name )
{
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    if ( GUCEF_NULL == attribute || CCiUtils::Trimmed( (*attribute).second.AsString() ).IsNULLOrEmpty() )
        return -1;
    return (*attribute).second.AsInt32( -1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiReport::CTestCase::CTestCase( void )
    : suite()
    , name()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiReport::CTestCase::Describe( UInt32 maxChars ) const
{GUCEF_TRACE;

    CORE::CString text = suite.IsNULLOrEmpty() ? name : suite + ": " + name;
    if ( !message.IsNULLOrEmpty() )
        text += " - " + message.ReplaceSubstr( "\r", "" ).ReplaceChar( '\n', ' ' );
    if ( text.Length() > maxChars && maxChars > 3 )
        text = text.SubstrToIndex( maxChars - 3, true ) + "...";
    return text;
}

/*-------------------------------------------------------------------------*/

CCiReport::CTest::CTest( void )
    : name()
    , status()
    , exitCode( 0 )
    , seconds( 0 )
    , testCount( -1 )
    , failureCount( -1 )
    , skippedCount( -1 )
    , resultsFile()
    , logTail()
    , failedCases()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CTest::IsFailed( void ) const
{GUCEF_TRACE;

    return IsFailedStatus( status ) || !failedCases.empty();
}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CTest::IsSkipped( void ) const
{GUCEF_TRACE;

    return status == "skipped";
}

/*-------------------------------------------------------------------------*/

CCiReport::CProduct::CProduct( void )
    : name()
    , status()
    , phase()
    , exitCode( 0 )
    , seconds( 0 )
    , logFile()
    , logTail()
    , tests()
    , tag()
    , digest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CProduct::IsImage( void ) const
{GUCEF_TRACE;

    return phase == ImagePhase;
}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CProduct::IsFailed( void ) const
{GUCEF_TRACE;

    return IsFailedStatus( status ) || CountFailedTests() > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCiReport::CProduct::CountFailedTests( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<tests.size(); ++i )
        if ( tests[ i ].IsFailed() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CCiReport::CCell::CCell( void )
    : jobId()
    , state( CCiRunStatus::STATE_UNKNOWN )
    , hasReport( false )
    , schema( 0 )
    , cell()
    , platform()
    , compiler()
    , cppStandard()
    , buildType()
    , mode()
    , status()
    , runId()
    , commit()
    , started()
    , seconds( 0 )
    , workspace()
    , problem()
    , products()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CCiReport::CCell::LoadFromImageReport( const CCiImageReport& report )
{GUCEF_TRACE;

    schema = (Int32) report.schema;
    cell = jobId;
    mode = "images";
    runId = report.run;
    commit = report.commit;
    products.clear();
    seconds = 0;
    bool anyFailed = false;
    for ( size_t i=0; i<report.images.size(); ++i )
    {
        const CCiImageReport::CImage& image = report.images[ i ];
        CProduct product;
        product.name = image.name;
        // the status word as the report gives it; an image without a digest was not pushed
        product.status = image.status.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : image.status;
        if ( product.status == "ok" && !CCiImageReport::IsDigest( image.digest ) )
            product.status = "failed";
        product.phase = ImagePhase;
        product.exitCode = product.status == "ok" ? 0 : 1;
        product.seconds = (UInt32) image.seconds;
        product.logFile = image.name + ".log.gz";
        product.logTail = image.logTail;
        product.tag = image.tag;
        product.digest = image.digest;
        anyFailed = anyFailed || product.status != "ok";
        seconds += product.seconds;
        products.push_back( product );
    }
    status = report.status.IsNULLOrEmpty() ? ( anyFailed ? CORE::CString( "failed" ) : CORE::CString( "ok" ) ) : report.status;
    hasReport = true;
}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CCell::LoadFromReport( const CORE::CDataNode& report )
{GUCEF_TRACE;

    schema = report.GetAttributeValue( "schema" ).AsInt32( 0 );
    cell = Text( report, "cell" );
    platform = Text( report, "platform" );
    compiler = Text( report, "compiler" );
    cppStandard = Text( report, "cppStandard" );
    buildType = Text( report, "buildType" );
    mode = Word( report, "mode" );
    status = CCiUtils::Trimmed( Either( report, "status", "result" ) ).Lowercase();
    runId = Text( report, "runId" );
    commit = Text( report, "commit" );
    started = Text( report, "started" );
    seconds = report.GetAttributeValue( "seconds" ).AsUInt32( 0 );
    workspace = Text( report, "workspace" );
    products.clear();

    const CORE::CDataNode* productList = report.FindChild( "products" );
    if ( GUCEF_NULL != productList )
    {
        for ( CORE::CDataNode::const_iterator i = productList->ConstBegin(); i != productList->ConstEnd(); ++i )
        {
            const CORE::CDataNode& entry = *(*i);
            CProduct product;
            product.name = Text( entry, "name" );
            product.status = Word( entry, "status" );
            product.phase = Word( entry, "phase" );
            product.exitCode = entry.GetAttributeValue( "exitCode" ).AsInt32( 0 );
            product.seconds = entry.GetAttributeValue( "seconds" ).AsUInt32( 0 );
            product.logFile = CCiUtils::Trimmed( Either( entry, "logFile", "log" ) );
            product.logTail = Either( entry, "logTail", "tail" );

            const CORE::CDataNode* testList = entry.FindChild( "tests" );
            if ( GUCEF_NULL != testList )
            {
                for ( CORE::CDataNode::const_iterator t = testList->ConstBegin(); t != testList->ConstEnd(); ++t )
                {
                    const CORE::CDataNode& testEntry = *(*t);
                    CTest test;
                    test.name = Text( testEntry, "name" );
                    test.status = Word( testEntry, "status" );
                    test.exitCode = testEntry.GetAttributeValue( "exitCode" ).AsInt32( 0 );
                    test.seconds = testEntry.GetAttributeValue( "seconds" ).AsUInt32( 0 );
                    test.testCount = Count( testEntry, "tests" );
                    test.failureCount = Count( testEntry, "failures" );
                    test.skippedCount = Count( testEntry, "skipped" );
                    test.resultsFile = Text( testEntry, "resultsFile" );
                    test.logTail = testEntry.GetAttributeValue( "logTail" ).AsString();
                    const CORE::CDataNode* cases = testEntry.FindChild( "failedCases" );
                    if ( GUCEF_NULL != cases )
                    {
                        for ( CORE::CDataNode::const_iterator c = cases->ConstBegin(); c != cases->ConstEnd(); ++c )
                        {
                            CTestCase testCase;
                            testCase.suite = Text( *(*c), "suite" );
                            testCase.name = Text( *(*c), "name" );
                            testCase.message = Text( *(*c), "message" );
                            if ( testCase.name.IsNULLOrEmpty() )
                                testCase.name = CCiUtils::Trimmed( (*c)->GetValue().AsString() );   // a plain name
                            if ( !testCase.name.IsNULLOrEmpty() )
                                test.failedCases.push_back( testCase );
                        }
                    }
                    if ( !test.name.IsNULLOrEmpty() )
                        product.tests.push_back( test );
                }
            }
            if ( !product.name.IsNULLOrEmpty() )
                products.push_back( product );
        }
    }

    // the checkout on the runner, for paths relative to it
    for ( size_t i=0; i<products.size() && workspace.IsNULLOrEmpty(); ++i )
        workspace = GuessWorkspace( products[ i ].logTail, cell.IsNULLOrEmpty() ? jobId : cell );
    hasReport = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiReport::CCell::IsFailed( void ) const
{GUCEF_TRACE;

    return IsFailedStatus( status ) || CountFailedProducts() > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCiReport::CCell::CountFailedProducts( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<products.size(); ++i )
        if ( products[ i ].IsFailed() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CCiReport::CCell::CountFailedTests( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<products.size(); ++i )
        count += products[ i ].CountFailedTests();
    return count;
}

/*-------------------------------------------------------------------------*/

const CCiReport::CTest*
CCiReport::CCell::FindTest( const CORE::CString& name, const CProduct** product ) const
{GUCEF_TRACE;

    for ( size_t p=0; p<products.size(); ++p )
    {
        for ( size_t t=0; t<products[ p ].tests.size(); ++t )
        {
            const CTest& test = products[ p ].tests[ t ];
            if ( test.name == name || test.name == name + "_d" )
            {
                if ( GUCEF_NULL != product )
                    *product = &products[ p ];
                return &test;
            }
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CCiReport::CCiReport( void )
    : status()
    , cells()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiReport::DescribeStatus( const CORE::CString& status )
{GUCEF_TRACE;

    const CORE::CString word = status.Lowercase();
    if ( word == "ok" || word == "success" || word == "succeeded" || word == "passed" )
        return "succeeded";
    if ( word == "failed" || word == "failure" || word == "error" )
        return "failed";
    if ( word == "timeout" || word == "timedout" )
        return "timed out";
    if ( word.IsNULLOrEmpty() )
        return "unknown";
    return word;
}

/*-------------------------------------------------------------------------*/

bool
CCiReport::IsFailedStatus( const CORE::CString& status )
{GUCEF_TRACE;

    const CORE::CString word = status.Lowercase();
    return word == "failed" || word == "failure" || word == "error" || word == "timeout" || word == "timedout";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiReport::GuessWorkspace( const CORE::CString& log, const CORE::CString& cell )
{GUCEF_TRACE;

    if ( log.IsNULLOrEmpty() || cell.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString marker = "/common/bin/" + cell + "/";
    const char* c = log.C_String();
    const char* found = ::strstr( c, marker.C_String() );          // bytes, not characters: logs hold UTF-8
    if ( GUCEF_NULL == found || found == c )
        return CORE::CString::Empty;
    const Int32 at = (Int32) ( found - c );

    // back to the start of the path: white space, a quote or the start of the text
    Int32 start = at;
    while ( start > 0 && ' ' != c[ start-1 ] && '\t' != c[ start-1 ] && '\n' != c[ start-1 ] && '"' != c[ start-1 ] && '\'' != c[ start-1 ] && '=' != c[ start-1 ] )
        --start;
    if ( '/' != c[ start ] )
        return CORE::CString::Empty;
    return CORE::CString( c + start, (UInt32) ( at - start ) );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
