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

#ifndef GUCEF_CORE_CVERSION_H
#include "gucefCORE_CVersion.h"
#define GUCEF_CORE_CVERSION_H
#endif /* GUCEF_CORE_CVERSION_H ? */

#ifndef GUCEF_CORE_CVERSIONRANGE_H
#include "gucefCORE_CVersionRange.h"
#define GUCEF_CORE_CVERSIONRANGE_H
#endif /* GUCEF_CORE_CVERSIONRANGE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVersion.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformVersionTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CVersion TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CVersion" );

    // Test 1: Default construction
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
            CORE::CVersion v;
            ASSERT_TRUE( v.GetMajor() == 0 );
            ASSERT_TRUE( v.GetMinor() == 0 );
            ASSERT_TRUE( v.GetPatch() == 0 );
            ASSERT_TRUE( v.GetRelease() == 0 );
            ASSERT_TRUE( v.IsAllZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Parameterized construction
    GUCEF_TESTFW_TESTCASE( "Test 2: Parameterized construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Parameterized construction" );
            CORE::CVersion v( 1, 2, 3, 4 );
            ASSERT_TRUE( v.GetMajor() == 1 );
            ASSERT_TRUE( v.GetMinor() == 2 );
            ASSERT_TRUE( v.GetPatch() == 3 );
            ASSERT_TRUE( v.GetRelease() == 4 );
            ASSERT_FALSE( v.IsAllZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Partial parameterized construction (defaults)
    GUCEF_TESTFW_TESTCASE( "Test 3: Partial parameterized construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Partial parameterized construction" );
            CORE::CVersion v( 5 );
            ASSERT_TRUE( v.GetMajor() == 5 );
            ASSERT_TRUE( v.GetMinor() == 0 );
            ASSERT_TRUE( v.GetPatch() == 0 );
            ASSERT_TRUE( v.GetRelease() == 0 );

            CORE::CVersion v2( 2, 7 );
            ASSERT_TRUE( v2.GetMajor() == 2 );
            ASSERT_TRUE( v2.GetMinor() == 7 );
            ASSERT_TRUE( v2.GetPatch() == 0 );
            ASSERT_TRUE( v2.GetRelease() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Copy construction
    GUCEF_TESTFW_TESTCASE( "Test 4: Copy construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Copy construction" );
            CORE::CVersion src( 3, 1, 4, 1 );
            CORE::CVersion copy( src );
            ASSERT_TRUE( copy.GetMajor() == 3 );
            ASSERT_TRUE( copy.GetMinor() == 1 );
            ASSERT_TRUE( copy.GetPatch() == 4 );
            ASSERT_TRUE( copy.GetRelease() == 1 );
            ASSERT_TRUE( copy == src );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Construction from TVersion
    GUCEF_TESTFW_TESTCASE( "Test 5: Construction from TVersion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Construction from TVersion" );
            CORE::TVersion tv;
            tv.major = 10;
            tv.minor = 20;
            tv.patch = 30;
            tv.release = 40;
            CORE::CVersion v( tv );
            ASSERT_TRUE( v.GetMajor() == 10 );
            ASSERT_TRUE( v.GetMinor() == 20 );
            ASSERT_TRUE( v.GetPatch() == 30 );
            ASSERT_TRUE( v.GetRelease() == 40 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Setters
    GUCEF_TESTFW_TESTCASE( "Test 6: Setters" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Setters" );
            CORE::CVersion v;
            v.SetMajor( 7 );
            v.SetMinor( 8 );
            v.SetPatch( 9 );
            v.SetRelease( 10 );
            ASSERT_TRUE( v.GetMajor() == 7 );
            ASSERT_TRUE( v.GetMinor() == 8 );
            ASSERT_TRUE( v.GetPatch() == 9 );
            ASSERT_TRUE( v.GetRelease() == 10 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Clear
    GUCEF_TESTFW_TESTCASE( "Test 7: Clear" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Clear" );
            CORE::CVersion v( 1, 2, 3, 4 );
            ASSERT_FALSE( v.IsAllZero() );
            v.Clear();
            ASSERT_TRUE( v.GetMajor() == 0 );
            ASSERT_TRUE( v.GetMinor() == 0 );
            ASSERT_TRUE( v.GetPatch() == 0 );
            ASSERT_TRUE( v.GetRelease() == 0 );
            ASSERT_TRUE( v.IsAllZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Assignment from CVersion
    GUCEF_TESTFW_TESTCASE( "Test 8: Assignment from CVersion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Assignment from CVersion" );
            CORE::CVersion src( 2, 4, 6, 8 );
            CORE::CVersion dst;
            dst = src;
            ASSERT_TRUE( dst == src );
            ASSERT_TRUE( dst.GetMajor() == 2 );
            ASSERT_TRUE( dst.GetMinor() == 4 );
            ASSERT_TRUE( dst.GetPatch() == 6 );
            ASSERT_TRUE( dst.GetRelease() == 8 );

            // Self-assignment
            dst = dst;
            ASSERT_TRUE( dst == src );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Assignment from TVersion
    GUCEF_TESTFW_TESTCASE( "Test 9: Assignment from TVersion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Assignment from TVersion" );
            CORE::TVersion tv;
            tv.major = 11;
            tv.minor = 22;
            tv.patch = 33;
            tv.release = 44;
            CORE::CVersion v;
            v = tv;
            ASSERT_TRUE( v.GetMajor() == 11 );
            ASSERT_TRUE( v.GetMinor() == 22 );
            ASSERT_TRUE( v.GetPatch() == 33 );
            ASSERT_TRUE( v.GetRelease() == 44 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: operator== equality
    GUCEF_TESTFW_TESTCASE( "Test 10: operator== equality" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: operator== equality" );
            CORE::CVersion a( 1, 2, 3, 4 );
            CORE::CVersion b( 1, 2, 3, 4 );
            CORE::CVersion c( 1, 2, 3, 5 );
            ASSERT_TRUE( a == b );
            ASSERT_FALSE( a == c );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: operator< less-than
    GUCEF_TESTFW_TESTCASE( "Test 11: operator< less-than" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: operator< less-than" );
            CORE::CVersion v100( 1, 0, 0, 0 );
            CORE::CVersion v110( 1, 1, 0, 0 );
            CORE::CVersion v111( 1, 1, 1, 0 );
            CORE::CVersion v200( 2, 0, 0, 0 );

            ASSERT_TRUE( v100 < v110 );
            ASSERT_TRUE( v100 < v111 );
            ASSERT_TRUE( v100 < v200 );
            ASSERT_TRUE( v110 < v111 );
            ASSERT_TRUE( v110 < v200 );
            ASSERT_TRUE( v111 < v200 );

            ASSERT_FALSE( v200 < v100 );
            ASSERT_FALSE( v110 < v100 );
            ASSERT_FALSE( v100 < v100 );  // equal is not less than
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: operator> greater-than
    GUCEF_TESTFW_TESTCASE( "Test 12: operator> greater-than" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: operator> greater-than" );
            CORE::CVersion v100( 1, 0, 0, 0 );
            CORE::CVersion v200( 2, 0, 0, 0 );
            CORE::CVersion v210( 2, 1, 0, 0 );

            ASSERT_TRUE( v200 > v100 );
            ASSERT_TRUE( v210 > v200 );
            ASSERT_TRUE( v210 > v100 );

            ASSERT_FALSE( v100 > v200 );
            ASSERT_FALSE( v100 > v100 );  // equal is not greater than
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: Comparison only differs in release component
    GUCEF_TESTFW_TESTCASE( "Test 13: Comparison on release component" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: Comparison on release component" );
            CORE::CVersion a( 1, 0, 0, 0 );
            CORE::CVersion b( 1, 0, 0, 1 );
            ASSERT_TRUE( a < b );
            ASSERT_TRUE( b > a );
            ASSERT_FALSE( a == b );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: GetCStyleVersion and SetCStyleVersion
    GUCEF_TESTFW_TESTCASE( "Test 14: GetCStyleVersion and SetCStyleVersion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: GetCStyleVersion and SetCStyleVersion" );
            CORE::CVersion v( 5, 6, 7, 8 );
            const CORE::TVersion& tv = v.GetCStyleVersion();
            ASSERT_TRUE( tv.major == 5 );
            ASSERT_TRUE( tv.minor == 6 );
            ASSERT_TRUE( tv.patch == 7 );
            ASSERT_TRUE( tv.release == 8 );

            CORE::TVersion tv2;
            tv2.major = 9;
            tv2.minor = 10;
            tv2.patch = 11;
            tv2.release = 12;
            v.SetCStyleVersion( tv2 );
            ASSERT_TRUE( v.GetMajor() == 9 );
            ASSERT_TRUE( v.GetMinor() == 10 );
            ASSERT_TRUE( v.GetPatch() == 11 );
            ASSERT_TRUE( v.GetRelease() == 12 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: ToString with all fields
    GUCEF_TESTFW_TESTCASE( "Test 15: ToString with all fields" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: ToString with all fields" );
            CORE::CVersion v( 1, 2, 3, 4 );
            CORE::CString s = v.ToString( true, true, true );
            ASSERT_TRUE( s == "1.2.3.4" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: ToString with partial fields
    GUCEF_TESTFW_TESTCASE( "Test 16: ToString with partial fields" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: ToString with partial fields" );
            CORE::CVersion v( 1, 2, 3, 4 );
            ASSERT_TRUE( v.ToString( false, false, false ) == "1" );
            ASSERT_TRUE( v.ToString( true, false, false ) == "1.2" );
            ASSERT_TRUE( v.ToString( true, true, false ) == "1.2.3" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: operator CString
    GUCEF_TESTFW_TESTCASE( "Test 17: operator CString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: operator CString" );
            CORE::CVersion v( 2, 0, 1, 0 );
            CORE::CString s = (CORE::CString) v;
            ASSERT_TRUE( s == "2.0.1.0" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: FromString - full 4-component version
    GUCEF_TESTFW_TESTCASE( "Test 18: FromString - 4 components" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: FromString - 4 components" );
            CORE::CVersion v;
            ASSERT_TRUE( v.FromString( "1.2.3.4" ) );
            ASSERT_TRUE( v.GetMajor() == 1 );
            ASSERT_TRUE( v.GetMinor() == 2 );
            ASSERT_TRUE( v.GetPatch() == 3 );
            ASSERT_TRUE( v.GetRelease() == 4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: FromString - fewer components
    GUCEF_TESTFW_TESTCASE( "Test 19: FromString - fewer components" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: FromString - fewer components" );
            CORE::CVersion v;

            ASSERT_TRUE( v.FromString( "3" ) );
            ASSERT_TRUE( v.GetMajor() == 3 );
            ASSERT_TRUE( v.GetMinor() == 0 );
            ASSERT_TRUE( v.GetPatch() == 0 );
            ASSERT_TRUE( v.GetRelease() == 0 );

            ASSERT_TRUE( v.FromString( "5.6" ) );
            ASSERT_TRUE( v.GetMajor() == 5 );
            ASSERT_TRUE( v.GetMinor() == 6 );
            ASSERT_TRUE( v.GetPatch() == 0 );
            ASSERT_TRUE( v.GetRelease() == 0 );

            ASSERT_TRUE( v.FromString( "7.8.9" ) );
            ASSERT_TRUE( v.GetMajor() == 7 );
            ASSERT_TRUE( v.GetMinor() == 8 );
            ASSERT_TRUE( v.GetPatch() == 9 );
            ASSERT_TRUE( v.GetRelease() == 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: FromString round-trip
    GUCEF_TESTFW_TESTCASE( "Test 20: FromString round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: FromString round-trip" );
            CORE::CVersion original( 10, 20, 30, 40 );
            CORE::CString str = original.ToString( true, true, true );

            CORE::CVersion parsed;
            ASSERT_TRUE( parsed.FromString( str ) );
            ASSERT_TRUE( parsed == original );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: IsAllZero with partial components
    GUCEF_TESTFW_TESTCASE( "Test 21: IsAllZero with partial components" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: IsAllZero with partial components" );
            CORE::CVersion v( 0, 0, 0, 1 );
            ASSERT_FALSE( v.IsAllZero() );

            CORE::CVersion v2( 0, 0, 1, 0 );
            ASSERT_FALSE( v2.IsAllZero() );

            CORE::CVersion v3( 0, 1, 0, 0 );
            ASSERT_FALSE( v3.IsAllZero() );

            CORE::CVersion v4( 1, 0, 0, 0 );
            ASSERT_FALSE( v4.IsAllZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVersion TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/

void
PerformVersionRangeTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CVersionRange TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CVersionRange" );

    // Test 1: Default construction
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
            CORE::CVersionRange range;
            ASSERT_TRUE( range.GetMinimum().IsAllZero() );
            ASSERT_TRUE( range.GetMaximum().IsAllZero() );
            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_TRUE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Parameterized construction - inclusive range
    GUCEF_TESTFW_TESTCASE( "Test 2: Parameterized construction - inclusive range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Parameterized construction - inclusive range" );
            CORE::CVersion minV( 1, 0, 0, 0 );
            CORE::CVersion maxV( 2, 0, 0, 0 );
            CORE::CVersionRange range( minV, true, maxV, true );

            ASSERT_TRUE( range.GetMinimum() == minV );
            ASSERT_TRUE( range.GetMaximum() == maxV );
            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_TRUE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Parameterized construction - exclusive range
    GUCEF_TESTFW_TESTCASE( "Test 3: Parameterized construction - exclusive range" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Parameterized construction - exclusive range" );
            CORE::CVersion minV( 1, 0, 0, 0 );
            CORE::CVersion maxV( 3, 0, 0, 0 );
            CORE::CVersionRange range( minV, false, maxV, false );

            ASSERT_FALSE( range.GetIsMinimumIncluded() );
            ASSERT_FALSE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Parameterized construction - mixed inclusion
    GUCEF_TESTFW_TESTCASE( "Test 4: Parameterized construction - mixed inclusion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Parameterized construction - mixed inclusion" );
            CORE::CVersion minV( 1, 5, 0, 0 );
            CORE::CVersion maxV( 2, 5, 0, 0 );
            CORE::CVersionRange range( minV, true, maxV, false );

            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_FALSE( range.GetIsMaximumIncluded() );
            ASSERT_TRUE( range.GetMinimum() == minV );
            ASSERT_TRUE( range.GetMaximum() == maxV );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Copy construction
    GUCEF_TESTFW_TESTCASE( "Test 5: Copy construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Copy construction" );
            CORE::CVersion minV( 1, 0, 0, 0 );
            CORE::CVersion maxV( 5, 0, 0, 0 );
            CORE::CVersionRange original( minV, true, maxV, false );
            CORE::CVersionRange copy( original );

            ASSERT_TRUE( copy == original );
            ASSERT_TRUE( copy.GetMinimum() == minV );
            ASSERT_TRUE( copy.GetMaximum() == maxV );
            ASSERT_TRUE( copy.GetIsMinimumIncluded() );
            ASSERT_FALSE( copy.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Assignment from CVersionRange
    GUCEF_TESTFW_TESTCASE( "Test 6: Assignment from CVersionRange" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Assignment from CVersionRange" );
            CORE::CVersion minV( 2, 0, 0, 0 );
            CORE::CVersion maxV( 4, 0, 0, 0 );
            CORE::CVersionRange src( minV, false, maxV, true );
            CORE::CVersionRange dst;
            dst = src;

            ASSERT_TRUE( dst == src );
            ASSERT_FALSE( dst.GetIsMinimumIncluded() );
            ASSERT_TRUE( dst.GetIsMaximumIncluded() );

            // Self-assignment
            dst = dst;
            ASSERT_TRUE( dst == src );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Assignment from CVersion (sets exact version range)
    GUCEF_TESTFW_TESTCASE( "Test 7: Assignment from CVersion" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Assignment from CVersion" );
            CORE::CVersion v( 3, 1, 4, 1 );
            CORE::CVersionRange range;
            range = v;

            ASSERT_TRUE( range.GetMinimum() == v );
            ASSERT_TRUE( range.GetMaximum() == v );
            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_TRUE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: operator== equality and inequality
    GUCEF_TESTFW_TESTCASE( "Test 8: operator== equality and inequality" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: operator== equality and inequality" );
            CORE::CVersion minV( 1, 0, 0, 0 );
            CORE::CVersion maxV( 2, 0, 0, 0 );
            CORE::CVersionRange a( minV, true, maxV, true );
            CORE::CVersionRange b( minV, true, maxV, true );
            CORE::CVersionRange c( minV, false, maxV, true );  // different minIncluded
            CORE::CVersionRange d( minV, true, maxV, false );  // different maxIncluded

            ASSERT_TRUE( a == b );
            ASSERT_FALSE( a == c );
            ASSERT_FALSE( a == d );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: GetCStyle and SetCStyle round-trip
    GUCEF_TESTFW_TESTCASE( "Test 9: GetCStyle and SetCStyle" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: GetCStyle and SetCStyle" );
            CORE::CVersion minV( 1, 2, 3, 4 );
            CORE::CVersion maxV( 5, 6, 7, 8 );
            CORE::CVersionRange original( minV, true, maxV, false );

            CORE::TVersionRange cs = original.GetCStyle();
            ASSERT_TRUE( cs.min.major == 1 );
            ASSERT_TRUE( cs.min.minor == 2 );
            ASSERT_TRUE( cs.min.patch == 3 );
            ASSERT_TRUE( cs.min.release == 4 );
            ASSERT_TRUE( cs.max.major == 5 );
            ASSERT_TRUE( cs.max.minor == 6 );
            ASSERT_TRUE( cs.max.patch == 7 );
            ASSERT_TRUE( cs.max.release == 8 );
            ASSERT_TRUE( cs.includeMin != 0 );
            ASSERT_TRUE( cs.includeMax == 0 );

            CORE::CVersionRange restored;
            restored.SetCStyle( cs );
            ASSERT_TRUE( restored == original );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: FromString - exact version bracket notation [x.y.z.w]
    GUCEF_TESTFW_TESTCASE( "Test 10: FromString - exact version [x.y]" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: FromString - exact version [x.y]" );
            CORE::CVersionRange range;
            bool result = range.FromString( "[1.2]" );
            ASSERT_TRUE( result );
            ASSERT_TRUE( range.GetMinimum().GetMajor() == 1 );
            ASSERT_TRUE( range.GetMinimum().GetMinor() == 2 );
            ASSERT_TRUE( range.GetMaximum().GetMajor() == 1 );
            ASSERT_TRUE( range.GetMaximum().GetMinor() == 2 );
            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_TRUE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: FromString - plain version (minimum bound)
    GUCEF_TESTFW_TESTCASE( "Test 11: FromString - plain version minimum bound" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: FromString - plain version minimum bound" );
            CORE::CVersionRange range;
            bool result = range.FromString( "2.0" );
            ASSERT_TRUE( result );
            ASSERT_TRUE( range.GetMinimum().GetMajor() == 2 );
            ASSERT_TRUE( range.GetMinimum().GetMinor() == 0 );
            ASSERT_TRUE( range.GetIsMinimumIncluded() );
            ASSERT_FALSE( range.GetIsMaximumIncluded() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: FromString - empty string returns false
    GUCEF_TESTFW_TESTCASE( "Test 12: FromString - empty string" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: FromString - empty string" );
            CORE::CVersionRange range;
            bool result = range.FromString( "" );
            ASSERT_FALSE( result );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CVersionRange TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
