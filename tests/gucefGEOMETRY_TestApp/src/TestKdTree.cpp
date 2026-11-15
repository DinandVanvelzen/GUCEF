/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTKDTREE_H
#include "gucefGEOMETRY_CTKdTree.h"
#define GUCEF_GEOMETRY_CTKDTREE_H
#endif /* GUCEF_GEOMETRY_CTKDTREE_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestKdTree.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace SpatialReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim > struct SKdPoint;

template < typename T >
struct SKdPoint< T, 2 >
{
    typedef CTPoint2D< T > TPoint;
    static TPoint Make( const Float64* c ) { return TPoint( (T) c[ 0 ], (T) c[ 1 ] ); }
    /** The squared distance by its own sum, in the order of the axes in the kind T */
    static T SquaredDistance( const TPoint& a, const TPoint& b )
    {
        const T dx = a.x - b.x;
        const T dy = a.y - b.y;
        return dx * dx + dy * dy;
    }
};

template < typename T >
struct SKdPoint< T, 3 >
{
    typedef CTPoint3D< T > TPoint;
    static TPoint Make( const Float64* c ) { return TPoint( (T) c[ 0 ], (T) c[ 1 ], (T) c[ 2 ] ); }
    static T SquaredDistance( const TPoint& a, const TPoint& b )
    {
        const T dx = a.x - b.x;
        const T dy = a.y - b.y;
        const T dz = a.z - b.z;
        return ( dx * dx + dy * dy ) + dz * dz;
    }
};

/** The pairs ( squared distance, index ) of all points, sorted: the scan */
template < typename T, UInt32 Dim >
static void
ScanDistances( const GUCEF::vector< typename SKdPoint< T, Dim >::TPoint >& points, const typename SKdPoint< T, Dim >::TPoint& q, GUCEF::vector< std::pair< T, UInt32 > >& pairs )
{
    pairs.clear();
    for ( UInt32 i=0; i<points.size(); ++i )
        pairs.push_back( std::make_pair( SKdPoint< T, Dim >::SquaredDistance( q, points[ i ] ), i ) );
    // the eight best in order are all the queries need besides the scan for the radius
    const size_t best = pairs.size() < 8 ? pairs.size() : 8;
    std::partial_sort( pairs.begin(), pairs.begin() + best, pairs.end() );
}

class CRadiusCollector
{
    public:

    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< Float64 > distances;

    template < typename T >
    bool Visit( UInt32 index, const T& squaredDistance )
    {
        indices.push_back( index );
        distances.push_back( (Float64) squaredDistance );
        return true;
    }
};

template < typename T, UInt32 Dim >
static bool
KdSweep( const char* name, UInt64 seed, UInt32 pointCount, UInt32 queryCount, bool duplicates, bool lattice )
{
    typedef SKdPoint< T, Dim > TKind;
    typedef typename TKind::TPoint TPoint;
    CRandom random( seed );
    GUCEF::vector< TPoint > points;
    for ( UInt32 i=0; i<pointCount; ++i )
    {
        Float64 c[ 3 ];
        for ( UInt32 a=0; a<Dim; ++a )
            c[ a ] = lattice ? (Float64) random.Index( 12 ) : random.Uniform( -100.0, 100.0 );
        if ( duplicates && i > 0 && random.Chance( 0.2 ) )
            points.push_back( points[ (size_t) random.Index( (Int32) i ) ] );
        else
            points.push_back( TKind::Make( c ) );
    }
    CTKdTree< T, Dim > tree;
    tree.Build( &points[ 0 ], pointCount, 1 + (UInt32) random.Index( 12 ) );
    if ( tree.GetPointCount() != pointCount )
        return false;
    GUCEF::vector< std::pair< T, UInt32 > > pairs;
    for ( UInt32 n=0; n<queryCount; ++n )
    {
        Float64 c[ 3 ];
        for ( UInt32 a=0; a<Dim; ++a )
            c[ a ] = lattice ? random.Uniform( -1.0, 12.0 ) * ( random.Chance( 0.5 ) ? 1.0 : 0.0 ) + (Float64) random.Index( 12 ) * ( random.Chance( 0.5 ) ? 1.0 : 0.0 ) : random.Uniform( -110.0, 110.0 );
        const TPoint q = TKind::Make( c );
        ScanDistances< T, Dim >( points, q, pairs );

        UInt32 nearest = 0;
        T nearestDistance = (T) 0;
        if ( !tree.FindNearest( q, nearest, nearestDistance ) || nearest != pairs[ 0 ].second || nearestDistance != pairs[ 0 ].first )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": nearest " << nearest << " at " << nearestDistance << " instead of " << pairs[ 0 ].second << " at " << pairs[ 0 ].first << ", seed " << seed << " query " << n );
            return false;
        }

        UInt32 indices[ 8 ];
        T distances[ 8 ];
        const UInt32 k = 8;
        const UInt32 found = tree.FindKNearest( q, k, indices, distances );
        const UInt32 expectedCount = pairs.size() < k ? (UInt32) pairs.size() : k;
        if ( found != expectedCount )
            return false;
        for ( UInt32 i=0; i<found; ++i )
            if ( indices[ i ] != pairs[ i ].second || distances[ i ] != pairs[ i ].first )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the " << i << "th nearest is " << indices[ i ] << " instead of " << pairs[ i ].second << ", seed " << seed << " query " << n );
                return false;
            }

        // a radius that holds about a dozen points, and sometimes none or all
        const UInt32 rank = (UInt32) random.Index( 8 );
        T radius = rank < pairs.size() ? (T) sqrt( (Float64) pairs[ rank ].first ) * (T) 3 : (T) 1000.0;
        if ( random.Chance( 0.05 ) )
            radius = (T) 0;
        CRadiusCollector collector;
        tree.FindWithinRadius( q, radius, collector );
        GUCEF::vector< UInt32 > expected;
        for ( size_t i=0; i<pairs.size(); ++i )
            if ( pairs[ i ].first <= radius * radius )
                expected.push_back( pairs[ i ].second );
        if ( !SameSet( collector.indices, expected ) || !NoDuplicates( collector.indices ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": radius query returns " << collector.indices.size() << " points instead of " << expected.size() << ", seed " << seed << " query " << n );
            return false;
        }
        for ( size_t i=0; i<collector.indices.size(); ++i )
            if ( collector.distances[ i ] != (Float64) TKind::SquaredDistance( q, points[ collector.indices[ i ] ] ) )
                return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "KdTree: " << name << ": " << queryCount << " nearest, 8 nearest and radius queries over " << pointCount << " points equal the scan (" << tree.GetNodeCount() << " nodes)" );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
KdSpecialCases( void )
{
    typedef CTPoint3D< Float64 > P;
    CTKdTree< Float64, 3 > tree;
    UInt32 index = 0;
    Float64 d = 0.0;
    // empty tree
    tree.Build( static_cast< const P* >( GUCEF_NULL ), 0 );
    if ( tree.FindNearest( P( 0, 0, 0 ), index, d ) || 0 != tree.GetPointCount() )
        return false;
    UInt32 indices[ 8 ];
    Float64 distances[ 8 ];
    if ( 0 != tree.FindKNearest( P( 0, 0, 0 ), 8, indices, distances ) )
        return false;
    // fewer points than k, and the tie of equal points goes to the lowest index
    const P few[ 5 ] = { P( 1, 1, 1 ), P( 1, 1, 1 ), P( 1, 1, 1 ), P( 5, 5, 5 ), P( 1, 1, 1 ) };
    tree.Build( few, 5, 1 );
    if ( 5 != tree.FindKNearest( P( 0, 0, 0 ), 8, indices, distances ) )
        return false;
    if ( indices[ 0 ] != 0 || indices[ 1 ] != 1 || indices[ 2 ] != 2 || indices[ 3 ] != 4 || indices[ 4 ] != 3 )
        return false;
    if ( !tree.FindNearest( P( 0, 0, 0 ), index, d ) || 0 != index )
        return false;
    // a point that is not finite is skipped in the build and refused as a query
    const Float64 zero = 0.0;
    const P bad[ 3 ] = { P( 0, 0, 0 ), P( zero / zero, 0, 0 ), P( 3, 0, 0 ) };
    tree.Build( bad, 3 );
    if ( 2 != tree.GetPointCount() || !tree.FindNearest( P( 2, 0, 0 ), index, d ) || 2 != index )
        return false;
    if ( tree.FindNearest( P( zero / zero, 0, 0 ), index, d ) )
        return false;
    // k = 1 and k = 0
    if ( 0 != tree.FindKNearest( P( 0, 0, 0 ), 0, indices, distances ) || 1 != tree.FindKNearest( P( 0, 0, 0 ), 1, indices, distances ) )
        return false;
    // the radius 0 finds the points at the query position
    CRadiusCollector collector;
    tree.FindWithinRadius( P( 3, 0, 0 ), 0.0, collector );
    if ( 1 != collector.indices.size() || 2 != collector.indices[ 0 ] )
        return false;
    // a negative radius finds nothing
    CRadiusCollector none;
    tree.FindWithinRadius( P( 3, 0, 0 ), -1.0, none );
    return none.indices.empty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformKdTreeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "KdTree" );

    GUCEF_TESTFW_TESTCASE( "KdTree: nearest, 8 nearest and radius queries over 10^5 points equal the scan, 3D and 2D, Float64 and Float32" )
    try
    {
        ASSERT_TRUE( ( KdSweep< Float64, 3 >( "3D Float64", 10101, ScaledCount( 100000 ), ScaledCount( 10000 ), false, false ) ) );
        ASSERT_TRUE( ( KdSweep< Float32, 3 >( "3D Float32", 10102, ScaledCount( 100000 ), ScaledCount( 3000 ), false, false ) ) );
        ASSERT_TRUE( ( KdSweep< Float64, 2 >( "2D Float64", 10103, ScaledCount( 100000 ), ScaledCount( 5000 ), false, false ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KdTree: duplicate points and lattice points with many equal distances are handled, ties go to the lowest index" )
    try
    {
        ASSERT_TRUE( ( KdSweep< Float64, 3 >( "3D duplicates", 10104, ScaledCount( 5000 ), ScaledCount( 2000 ), true, false ) ) );
        ASSERT_TRUE( ( KdSweep< Float64, 3 >( "3D lattice", 10105, ScaledCount( 3000 ), ScaledCount( 2000 ), true, true ) ) );
        ASSERT_TRUE( ( KdSweep< Float32, 2 >( "2D lattice", 10106, ScaledCount( 2000 ), ScaledCount( 2000 ), true, true ) ) );
        ASSERT_TRUE( ( KdSweep< Float64, 3 >( "3D few points", 10107, 5, 200, true, false ) ) );
        ASSERT_TRUE( KdSpecialCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
