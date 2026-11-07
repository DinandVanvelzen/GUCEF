/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CJSONSYNTAXVALIDATOR_H
#include "gucefCORE_CJsonSyntaxValidator.h"
#define GUCEF_CORE_CJSONSYNTAXVALIDATOR_H
#endif /* GUCEF_CORE_CJSONSYNTAXVALIDATOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonSyntaxValidator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Checks a copy of exactly the bytes in a block of their own size: under ASan a read past them is reported */
bool
IsValidExact( const char* text, UInt32 length, UInt32 maxDepth = 64 )
{
    char* copy = static_cast< char* >( malloc( length > 0 ? length : 1 ) );
    if ( GUCEF_NULL == copy )
        return false;
    if ( length > 0 )
        memcpy( copy, text, length );
    const bool valid = CORE::CJsonSyntaxValidator::IsValid( copy, length, maxDepth );
    free( copy );
    return valid;
}

bool
IsValidText( const char* text, UInt32 maxDepth = 64 )
{
    return IsValidExact( text, (UInt32) strlen( text ), maxDepth );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformJsonSyntaxValidatorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JSON syntax validator TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonSyntaxValidator" );

    GUCEF_TESTFW_TESTCASE( "JSON syntax: strict documents are taken" )
        try
        {
            const char* valid[] = { "{}", "[]", "0", "-0", "1", "-12.5e+3", "0.5E-7", "1e9", "true", "false", "null", "\"\"",
                                    " \t\r\n[ 1 , 2 ]\n ", "{\"a\":{\"b\":[1,{\"c\":null}]},\"d\":\"x\"}", "[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u00e9\\uD83D\\uDE00\"]",
                                    "[\"\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\"]", "{\"\":[]}", "[[[[[]]]]]", "[true,false,null,0,\"s\",{},[]]" };
            for ( UInt32 i=0; i<sizeof( valid ) / sizeof( valid[ 0 ] ); ++i )
                ASSERT_TRUE( IsValidText( valid[ i ] ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON syntax: lenient forms, truncated literals, bad numbers, strings and UTF-8 are refused" )
        try
        {
            const char* invalid[] = { "", " ", "[fals", "[tru", "[nul", "tru", "nul", "fals", "[1,]", "{\"a\":1,}", "[,1]", "{,}", "[1 2]",
                                      "{\"a\" 1}", "{a:1}", "{'a':1}", "['a']", "[01]", "[-]", "[1.]", "[.5]", "[1e]", "[1e+]", "[+1]",
                                      "[0x10]", "[NaN]", "[Infinity]", "[TRUE]", "[\"\\x\"]", "[\"\\u12\"]", "[\"\\u12G4\"]", "[\"a\tb\"]",
                                      "[\"a\nb\"]", "[\"open]", "[1]]", "[1] x", "[1][2]", "{\"a\":1}}", "[}", "{]", "\xEF\xBB\xBF[]",
                                      "[\"\xC0\x80\"]", "[\"\xED\xA0\x80\"]", "[\"\xF4\x90\x80\x80\"]", "[\"\xE2\x82\"]", "[\"\x80\"]",
                                      "[\"\xF5\x80\x80\x80\"]", "/* c */ []", "[1]//", "{\"a\":}", "{\"a\"}", "{\"a\":1 \"b\":2}" };
            for ( UInt32 i=0; i<sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
                ASSERT_FALSE( IsValidText( invalid[ i ] ) );

            // a zero byte is not white space
            ASSERT_FALSE( IsValidExact( "[1]\0", 4 ) );
            ASSERT_FALSE( IsValidExact( "[\"a\0b\"]", 7 ) );

            CORE::CString error;
            ASSERT_FALSE( CORE::CJsonSyntaxValidator::IsValid( "[1,]", 4, 8, &error ) );
            ASSERT_TRUE( error.HasSubstr( "byte 3", true ) >= 0 );
            ASSERT_FALSE( CORE::CJsonSyntaxValidator::IsValid( GUCEF_NULL, 3, 8, &error ) );
            ASSERT_FALSE( CORE::CJsonSyntaxValidator::IsValid( GUCEF_NULL, 0, 8, &error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON syntax: no strict prefix of an object or array document is taken, and none is read past" )
        try
        {
            const char* documents[] = { "{\"name\":\"t\",\"columns\":[{\"n\":\"a\",\"nullable\":false,\"v\":null,\"w\":-1.25e-3}],\"pk\":[\"a\",\"b\"]}",
                                        "[true,false,null,\"\\u00e9\xC3\xA9\",[{}],12345678901234567890]" };
            for ( UInt32 d=0; d<2; ++d )
            {
                const UInt32 length = (UInt32) strlen( documents[ d ] );
                ASSERT_TRUE( IsValidExact( documents[ d ], length ) );
                UInt32 taken = 0;
                for ( UInt32 cut=0; cut<length; ++cut )
                {
                    if ( IsValidExact( documents[ d ], cut ) )
                        ++taken;
                }
                ASSERT_TRUE( 0 == taken );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JSON syntax: the nesting of objects and arrays is bounded" )
        try
        {
            ASSERT_TRUE( IsValidText( "0", 0 ) && !IsValidText( "[]", 0 ) );
            ASSERT_TRUE( IsValidText( "[[{\"a\":[]}]]", 4 ) && !IsValidText( "[[{\"a\":[]}]]", 3 ) );

            // deeper than the bit stack tracks: refused at the tracked maximum, whatever larger bound is asked for
            const UInt32 depth = CORE::CJsonSyntaxValidator::MaxTrackedNestingDepth + 1;
            char* deep = static_cast< char* >( malloc( depth * 2 ) );
            ASSERT_TRUE( GUCEF_NULL != deep );
            if ( GUCEF_NULL != deep )
            {
                memset( deep, '[', depth );
                memset( deep + depth, ']', depth );
                ASSERT_FALSE( CORE::CJsonSyntaxValidator::IsValid( deep, depth * 2, GUCEF_UINT32MAX ) );
                ASSERT_TRUE( CORE::CJsonSyntaxValidator::IsValid( deep + 1, ( depth - 1 ) * 2, GUCEF_UINT32MAX ) );
                free( deep );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
