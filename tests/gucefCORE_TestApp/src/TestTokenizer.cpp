/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTOKENIZER_H
#include "gucefCORE_CTokenizer.h"
#define GUCEF_CORE_CTOKENIZER_H
#endif /* GUCEF_CORE_CTOKENIZER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestTokenizer.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// common hardcoded constants for testing
static const CORE::CString::StringVector cppPredefinedTokens = { "!", "!=", "=", "&&", "&", ">", ">=", "<", "<=" };
static const CORE::CString cppGroupOpenToken = "(";
static const CORE::CString cppGroupCloseToken = ")";
static const CORE::CString::StringVector noDefinedVars;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
TestCase_NoOpParsing( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 1: NoOp Parsing - empty string" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: NoOp Parsing - empty string" );
            // hardcoded test input strings
            const CORE::CString testString0 = CORE::CString::Empty;
            const CORE::CString::StringVector testString0Vars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testString0Seq0Tokens = { };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString0, cppPredefinedTokens, testString0Vars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testString0Seq0Tokens );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_NoGroupsBasicParsingWithWhitespace( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 2: No groups basic parsing with whitespace" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: No groups basic parsing with whitespace" );
            // hardcoded test input strings
            const CORE::CString testString1 = "var1 > var2 && var2 != 3";
            const CORE::CString::StringVector testString1Vars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testString1Seq0Tokens = { "var1", ">", "var2", "&&", "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString1, cppPredefinedTokens, testString1Vars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testString1Seq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 7 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].second == 3 );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].second == 4 );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].second == 5 );
            ASSERT_TRUE( rootSequence->ordering[ 6 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 6 ].second == 6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_NoGroupsBasicParsingNoWhitespaceWithDefinedVars( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 3: No groups basic parsing, no whitespace, with defined vars" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: No groups basic parsing, no whitespace, with defined vars" );
            // hardcoded test input strings
            const CORE::CString testString2 = "var1>var2&&var2!=3";
            const CORE::CString::StringVector testString2Vars = { "var1", "var2" };

            // hardcoded expected results
            const CORE::CString::StringVector testString2Seq0Tokens = { "var1", ">", "var2", "&&", "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString2, cppPredefinedTokens, testString2Vars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testString2Seq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 7 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].second == 3 );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].second == 4 );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].second == 5 );
            ASSERT_TRUE( rootSequence->ordering[ 6 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 6 ].second == 6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_NoGroupsBasicParsingNoWhitespaceWithDefinedVars2( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 4: No groups basic parsing, no whitespace, with defined vars (adjacent vars)" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: No groups basic parsing, no whitespace, with defined vars (adjacent vars)" );
            // hardcoded test input strings
            const CORE::CString testString3 = "var1var2&&var2!=3";
            const CORE::CString::StringVector testString3Vars = { "var1", "var2" };

            // hardcoded expected results
            const CORE::CString::StringVector testString3Seq0Tokens = { "var1", "var2", "&&", "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString3, cppPredefinedTokens, testString3Vars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testString3Seq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 6 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].second == 3 );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].second == 4 );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 5 ].second == 5 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_NoGroupsBasicParsingNoWhitespaceNoDefinedVars( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 5: No groups basic parsing, no whitespace, no defined vars" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: No groups basic parsing, no whitespace, no defined vars" );
            // hardcoded test input strings
            const CORE::CString testString4 = "var1var2&&var2!=3";
            const CORE::CString::StringVector testString4Vars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testString4Seq0Tokens = { "var1var2", "&&", "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString4, cppPredefinedTokens, testString4Vars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testString4Seq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 5 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
        ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
        ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].second == 3 );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].second == 4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_SingleGroupParsingWithWhitespaceNoDefinedVars( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 6: Single group parsing with whitespace, no defined vars" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Single group parsing with whitespace, no defined vars" );
            // hardcoded test input strings
            const CORE::CString testString = "( var1 > var2 )";
            const CORE::CString::StringVector testStringVars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testStringSeq0Tokens = { "var1", ">", "var2" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString, cppPredefinedTokens, testStringVars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );
            ASSERT_TRUE( rootSequence->sequences.empty() );
            ASSERT_TRUE( rootSequence->sequenceOpenToken == cppGroupOpenToken );
            ASSERT_TRUE( rootSequence->sequenceCloseToken == cppGroupCloseToken );
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testStringSeq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 3 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_SameLevelGroupsParsingWithWhitespaceNoDefinedVars( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 7: Same level groups parsing with whitespace, no defined vars" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Same level groups parsing with whitespace, no defined vars" );
            // hardcoded test input strings
            const CORE::CString testString = "( var1 > var2 ) && ( var2 != 3 )";
            const CORE::CString::StringVector testStringVars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testStringSeq0Tokens = { "&&" };
            const CORE::CString::StringVector testStringSeq1Tokens = { "var1", ">", "var2" };
            const CORE::CString::StringVector testStringSeq2Tokens = { "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString, cppPredefinedTokens, testStringVars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );        
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testStringSeq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
        ASSERT_TRUE( rootSequence->ordering.size() == 3 );
        ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE );
        ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
        ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
        ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 1 );  
            ASSERT_TRUE( rootSequence->sequences.size() == 2 );
            ASSERT_TRUE( rootSequence->sequenceOpenToken.IsNULLOrEmpty() );
            ASSERT_TRUE( rootSequence->sequenceCloseToken.IsNULLOrEmpty() );
            ASSERT_TRUE( !rootSequence->sequences[ 0 ].IsNULL() );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->sequenceOpenToken == cppGroupOpenToken );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->sequenceCloseToken == cppGroupCloseToken );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->tokens == testStringSeq1Tokens );
            ASSERT_TRUE( !rootSequence->sequences[ 0 ]->ordering.empty() );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering.size() == 3 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 2 ].second == 2 );
            ASSERT_TRUE( !rootSequence->sequences[ 1 ].IsNULL() );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->sequenceOpenToken == cppGroupOpenToken );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->sequenceCloseToken == cppGroupCloseToken );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->tokens == testStringSeq2Tokens ); 
            ASSERT_TRUE( !rootSequence->sequences[ 1 ]->ordering.empty() );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering.size() == 3 );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 1 ]->ordering[ 2 ].second == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestCase_NestedGroupsParsingWithWhitespaceNoDefinedVars( void )
{
    GUCEF_TESTFW_TESTCASE( "Test 8: Nested groups parsing with whitespace, no defined vars" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Nested groups parsing with whitespace, no defined vars" );
            // hardcoded test input strings
            const CORE::CString testString = "( var1 > var2 && ( var2 != 3 ) )";
            const CORE::CString::StringVector testStringVars = noDefinedVars;

            // hardcoded expected results
            const CORE::CString::StringVector testStringSeq0Tokens = { "var1", ">", "var2", "&&" };
            const CORE::CString::StringVector testStringSeq1Tokens = { "var2", "!=", "3" };

            // run the test
            CORE::CString::StringVector errors;        
            CORE::CTokenizer::CTokenSequencePtr rootSequence;
            ASSERT_TRUE( CORE::CTokenizer::Tokenize( testString, cppPredefinedTokens, testStringVars, cppGroupOpenToken, cppGroupCloseToken, rootSequence, errors ) );
            ASSERT_TRUE( !rootSequence.IsNULL() );
            ASSERT_TRUE( errors.empty() );        
            ASSERT_TRUE( !rootSequence->tokens.empty() );
            ASSERT_TRUE( rootSequence->tokens == testStringSeq0Tokens );
            ASSERT_TRUE( !rootSequence->ordering.empty() );
            ASSERT_TRUE( rootSequence->ordering.size() == 5 );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 2 ].second == 2 );  
            ASSERT_TRUE( rootSequence->ordering[ 3 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->ordering[ 3 ].second == 3 ); 
            ASSERT_TRUE( rootSequence->ordering[ 4 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE );
            ASSERT_TRUE( rootSequence->ordering[ 4 ].second == 0 );        
            ASSERT_TRUE( rootSequence->sequences.size() == 1 );
            ASSERT_TRUE( rootSequence->sequenceOpenToken == cppGroupOpenToken );
            ASSERT_TRUE( rootSequence->sequenceCloseToken == cppGroupCloseToken );
            ASSERT_TRUE( !rootSequence->sequences[ 0 ].IsNULL() );
        ASSERT_TRUE( rootSequence->sequences[ 0 ]->sequenceOpenToken == cppGroupOpenToken );
        ASSERT_TRUE( rootSequence->sequences[ 0 ]->sequenceCloseToken == cppGroupCloseToken );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->tokens == testStringSeq1Tokens );
            ASSERT_TRUE( !rootSequence->sequences[ 0 ]->ordering.empty() );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering.size() == 3 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 0 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 0 ].second == 0 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 1 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 1 ].second == 1 );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 2 ].first == CORE::CTokenizer::CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN );
            ASSERT_TRUE( rootSequence->sequences[ 0 ]->ordering[ 2 ].second == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformParsingValidations( void )
{
    TestCase_NoOpParsing();
    TestCase_NoGroupsBasicParsingWithWhitespace();
    TestCase_NoGroupsBasicParsingNoWhitespaceWithDefinedVars();
    TestCase_NoGroupsBasicParsingNoWhitespaceWithDefinedVars2();
    TestCase_NoGroupsBasicParsingNoWhitespaceNoDefinedVars();
    
    TestCase_SingleGroupParsingWithWhitespaceNoDefinedVars();
    TestCase_SameLevelGroupsParsingWithWhitespaceNoDefinedVars();
    TestCase_NestedGroupsParsingWithWhitespaceNoDefinedVars();
}

/*-------------------------------------------------------------------------*/

void
PerformTokenizerTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CTokenizer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CTokenizer" );

    PerformParsingValidations();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CTokenizer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
