/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#include "gucefCORE_CCommandIdempotencyCache.h"
#define GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCommandIdempotencyCache.h"

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

/** An outcome type of a caller: a status and a text */
struct SCiTestOutcome
{
    Int32 status;
    CORE::CString text;

    SCiTestOutcome( void ) : status( 0 ), text() {}
    SCiTestOutcome( Int32 s, const char* t ) : status( s ), text( t ) {}
};

CORE::CDataNode
MakeOutcome( const char* handle )
{
    CORE::CDataNode outcome( "outcome", GUCEF_DATATYPE_OBJECT );
    outcome.SetAttribute( "handle", handle );
    return outcome;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCommandIdempotencyCacheTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CommandIdempotencyCache" );

    GUCEF_TESTFW_TESTCASE( "CommandIdempotencyCache: a repeated command gets the stored outcome, other arguments are a conflict, an empty digest is not compared" )
    try
    {
        CORE::CCommandIdempotencyCache cache;
        CORE::CDataNode found;
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "alice", "k1", "d1", 1000, found ) );
        cache.Store( "alice", "k1", "d1", 1000, MakeOutcome( "h1" ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "alice", "k1", "d1", 1001, found ) );
        ASSERT_TRUE( "h1" == found.GetAttributeValueOrChildValueByName( "handle" ).AsString() );
        found.Clear();
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_CONFLICT == cache.Find( "alice", "k1", "d2", 1002, found ) );
        ASSERT_TRUE( found.GetAttributeValueOrChildValueByName( "handle" ).AsString().IsNULLOrEmpty() );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "alice", "k1", CORE::CString::Empty, 1002, found ) );

        // keys are per subject
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "bob", "k1", "d1", 1003, found ) );
        ASSERT_TRUE( 1 == cache.GetCount() && 1 == cache.GetCountOfSubject( "alice" ) && 0 == cache.GetCountOfSubject( "bob" ) );

        // storing again replaces
        cache.Store( "alice", "k1", "d3", 1004, MakeOutcome( "h2" ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "alice", "k1", "d3", 1005, found ) );
        ASSERT_TRUE( "h2" == found.GetAttributeValueOrChildValueByName( "handle" ).AsString() && 1 == cache.GetCount() );

        ASSERT_TRUE( cache.Remove( "alice", "k1" ) );
        ASSERT_FALSE( cache.Remove( "alice", "k1" ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "alice", "k1", "d3", 1006, found ) && 0 == cache.GetCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CommandIdempotencyCache: records expire after the time to live; the per subject and total bounds push out the oldest" )
    try
    {
        CORE::CCommandIdempotencyCache cache;
        ASSERT_TRUE( CORE::CCommandIdempotencyCache::DefaultTtlMs == cache.GetTtlMs() && 256 == cache.GetMaxPerSubject() && 4096 == cache.GetMaxTotal() );
        cache.SetBounds( 100, 2, 3 );
        CORE::CDataNode found;
        cache.Store( "a", "1", CORE::CString::Empty, 0, MakeOutcome( "a1" ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "a", "1", CORE::CString::Empty, 99, found ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "a", "1", CORE::CString::Empty, 100, found ) );
        ASSERT_TRUE( 0 == cache.GetCount() );

        // per subject: the third of a subject pushes out its oldest
        cache.Store( "a", "1", CORE::CString::Empty, 200, MakeOutcome( "a1" ) );
        cache.Store( "a", "2", CORE::CString::Empty, 201, MakeOutcome( "a2" ) );
        cache.Store( "a", "3", CORE::CString::Empty, 202, MakeOutcome( "a3" ) );
        ASSERT_TRUE( 2 == cache.GetCountOfSubject( "a" ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "a", "1", CORE::CString::Empty, 203, found ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "a", "2", CORE::CString::Empty, 203, found ) );

        // in total: the fourth pushes out the oldest of anyone
        cache.Store( "b", "1", CORE::CString::Empty, 204, MakeOutcome( "b1" ) );
        cache.Store( "c", "1", CORE::CString::Empty, 205, MakeOutcome( "c1" ) );
        ASSERT_TRUE( 3 == cache.GetCount() );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_MISS == cache.Find( "a", "2", CORE::CString::Empty, 206, found ) );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "c", "1", CORE::CString::Empty, 206, found ) );

        // no bounds
        cache.SetBounds( 0, 0, 0 );
        for ( UInt32 i=0; i<50; ++i )
            cache.Store( "d", CORE::ToString( i ), CORE::CString::Empty, 1000000 + i, MakeOutcome( "d" ) );
        ASSERT_TRUE( 53 == cache.GetCount() );
        ASSERT_TRUE( CORE::COMMANDIDEMPOTENCY_HIT == cache.Find( "b", "1", CORE::CString::Empty, 99999999, found ) );
        cache.Clear();
        ASSERT_TRUE( 0 == cache.GetCount() && 0 == cache.GetCountOfSubject( "d" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CommandIdempotencyCache: the template keeps an outcome type of the caller" )
    try
    {
        CORE::CTCommandIdempotencyCache< SCiTestOutcome > cache;
        SCiTestOutcome found;
        ASSERT_FALSE( cache.Find( "s", "c", 10, found ) );
        cache.Store( "s", "c", 10, SCiTestOutcome( 7, "applied" ) );
        ASSERT_TRUE( cache.Find( "s", "c", 11, found ) );
        ASSERT_TRUE( 7 == found.status && "applied" == found.text );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
