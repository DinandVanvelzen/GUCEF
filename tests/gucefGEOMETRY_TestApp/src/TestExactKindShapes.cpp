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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_FRACTIONSHAPES_H
#include "gucefGEOMETRY_fractionshapes.h"
#define GUCEF_GEOMETRY_FRACTIONSHAPES_H
#endif /* GUCEF_GEOMETRY_FRACTIONSHAPES_H ? */

#include "TestExactKindShapes.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The checks that every exact kind has to pass, for kind T with the values written as a / b through 'make' */
template < typename T >
static UInt32
CheckExactShapes( const T& zero, const T& one, const T& two, const T& three, const T& four )
{
    typedef CTPoint2D< T > P2;
    typedef CTPoint3D< T > P3;
    typedef CTVector3D< T > V3;
    typedef CTSegment2D< T > Seg2;
    UInt32 ok = 0;
    UInt32 line = 0;
#define CHECK( cond ) { ++line; if ( 0 == ok && !( cond ) ) ok = line; }

    // segments: crossing, touching, collinear overlap and apart, and a point on a segment
    const Seg2 a( P2( zero, zero ), P2( four, four ) );
    CHECK( a.Intersects( Seg2( P2( zero, four ), P2( four, zero ) ) ) );
    CHECK( a.Intersects( Seg2( P2( four, four ), P2( four + two, two ) ) ) );
    CHECK( a.Intersects( Seg2( P2( three, three ), P2( four + three, four + three ) ) ) );
    CHECK( !a.Intersects( Seg2( P2( four + one, four + one ), P2( four + three, four + three ) ) ) );
    CHECK( !a.Intersects( Seg2( P2( zero, one ), P2( four, four + one ) ) ) );
    CHECK( a.Intersects( Seg2( P2( two, two ), P2( two, two ) ) ) && !a.Intersects( Seg2( P2( two, three ), P2( two, three ) ) ) );
    CHECK( a.GetVector() == CTVector2D< T >( four, four ) && a.GetBounds() == CTRectangle< T >( P2( zero, zero ), P2( four, four ) ) );

    // the closed rectangle: containment, merging, intersection, the empty rectangle as identity
    const CTRectangle< T > r( P2( zero, one ), P2( three, four ) );
    CHECK( r.Contains( P2( three, four ) ) && r.Contains( P2( one, two ) ) && !r.Contains( P2( four, two ) ) && !r.Contains( P2( two, zero ) ) );
    CHECK( CTRectangle< T >().IsEmpty() && CTRectangle< T >().Merged( r ) == r && r.Merged( CTRectangle< T >() ) == r );
    CHECK( r.Intersection( CTRectangle< T >( P2( two, two ), P2( four + one, four + one ) ) ) == CTRectangle< T >( P2( two, two ), P2( three, four ) ) );
    CHECK( r.Intersection( CTRectangle< T >( P2( four, zero ), P2( four + one, zero + one ) ) ).IsEmpty() );
    CHECK( r.ClosestPoint( P2( four + one, zero ) ) == P2( three, one ) && r.GetArea() == three * three );
    // an empty rectangle that is not the default one is also the identity of Merged
    const CTRectangle< T > shrunk = CTRectangle< T >( P2( zero, zero ), P2( one, one ) ).Expanded( zero - two );
    CHECK( shrunk.IsEmpty() && shrunk.Merged( r ) == r && r.Merged( shrunk ) == r && !shrunk.Contains( P2( zero, zero ) ) );

    // the closed box
    const CTAxisAlignedBox3D< T > b( P3( zero, zero, zero ), P3( one, two, three ) );
    CHECK( b.Contains( P3( one, one, three ) ) && !b.Contains( P3( one, one, four ) ) && b.GetVolume() == one * two * three );
    CHECK( b.Merged( CTAxisAlignedBox3D< T >() ) == b && CTAxisAlignedBox3D< T >().Merged( b ) == b );
    CHECK( b.Intersection( CTAxisAlignedBox3D< T >( P3( one, one, one ), P3( four, four, four ) ) ) == CTAxisAlignedBox3D< T >( P3( one, one, one ), P3( one, two, three ) ) );
    CHECK( b.ClosestPoint( P3( four, one, zero - one ) ) == P3( one, one, zero ) );
    CHECK( b.GetCorner( 7 ) == b.max && b.GetExtents() == V3( one, two, three ) );

    // a polygon: the unit square scaled by two, and an L shape
    CTPolygon2D< T > square;
    square.AddPoint( P2( zero, zero ) );
    square.AddPoint( P2( two, zero ) );
    square.AddPoint( P2( two, two ) );
    square.AddPoint( P2( zero, two ) );
    CHECK( square.IsConvex() && square.IsSimple() && square.Contains( P2( one, one ) ) && square.Contains( P2( two, one ) ) && !square.Contains( P2( three, one ) ) );
    CTPolygon2D< T > lShape;
    lShape.AddPoint( P2( zero, zero ) );
    lShape.AddPoint( P2( four, zero ) );
    lShape.AddPoint( P2( four, two ) );
    lShape.AddPoint( P2( two, two ) );
    lShape.AddPoint( P2( two, four ) );
    lShape.AddPoint( P2( zero, four ) );
    CHECK( !lShape.IsConvex() && lShape.IsSimple() && lShape.Contains( P2( one, three ) ) && !lShape.Contains( P2( three, three ) ) );
    CTPolygon2D< T > bow;
    bow.AddPoint( P2( zero, zero ) );
    bow.AddPoint( P2( two, two ) );
    bow.AddPoint( P2( two, zero ) );
    bow.AddPoint( P2( zero, two ) );
    CHECK( !bow.IsSimple() && !bow.IsConvex() );

    // text forms
    CTRectangle< T > parsed;
    CHECK( parsed.FromString( r.ToString() ) && parsed == r );
    CTSegment3D< T > parsedSegment;
    const CTSegment3D< T > segment3( P3( zero, one, two ), P3( three, four, zero ) );
    CHECK( parsedSegment.FromString( segment3.ToString() ) && parsedSegment == segment3 );
#undef CHECK
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformExactKindShapesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ExactKindShapes" );

    GUCEF_TESTFW_TESTCASE( "ExactKindShapes: Int32 straight-line shapes" )
    try
    {
        { UInt32 failed = CheckExactShapes< Int32 >( 0, 1, 2, 3, 4 ); if ( 0 != failed ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ExactKindShapes: Int32 check " << failed << " failed" ); ASSERT_TRUE( 0 == failed ); }
        // the extremes of the kind: a rectangle around the whole range and a segment crossing the origin
        const TInt32Rectangle all( TInt32Point2D( -2147483647 - 1, -2147483647 - 1 ), TInt32Point2D( 2147483647, 2147483647 ) );
        ASSERT_TRUE( all.Contains( TInt32Point2D( 0, 0 ) ) && all.Contains( all.min ) && all.Contains( all.max ) && all.Merged( all ) == all );
        ASSERT_TRUE( TInt32Segment2D( TInt32Point2D( -2147483647 - 1, -2147483647 - 1 ), TInt32Point2D( 2147483647, 2147483647 ) ).Intersects( TInt32Segment2D( TInt32Point2D( -2147483647 - 1, 2147483647 ), TInt32Point2D( 2147483647, -2147483647 - 1 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ExactKindShapes: 64 bit fraction straight-line shapes" )
    try
    {
        typedef CORE::TInt64Fraction F;
        { UInt32 failed = CheckExactShapes< F >( F( 0 ), F( 1 ), F( 2 ), F( 3 ), F( 4 ) ); if ( 0 != failed ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ExactKindShapes: fraction check " << failed << " failed" ); ASSERT_TRUE( 0 == failed ); }
        { UInt32 failed = CheckExactShapes< F >( F( 0 ), F( 1, 3 ), F( 2, 3 ), F( 1 ), F( 4, 3 ) ); if ( 0 != failed ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ExactKindShapes: scaled fraction check " << failed << " failed" ); ASSERT_TRUE( 0 == failed ); }
        // a point exactly on a segment at 1/3, and one a tiny bit off it
        const TInt64FractionSegment2D diagonal( TInt64FractionPoint2D( F( 0 ), F( 0 ) ), TInt64FractionPoint2D( F( 1 ), F( 1 ) ) );
        ASSERT_TRUE( diagonal.Intersects( TInt64FractionSegment2D( TInt64FractionPoint2D( F( 1, 3 ), F( 1, 3 ) ), TInt64FractionPoint2D( F( 1, 3 ), F( 1, 3 ) ) ) ) );
        ASSERT_FALSE( diagonal.Intersects( TInt64FractionSegment2D( TInt64FractionPoint2D( F( 1, 3 ), F( 1, 3 ) + F( 1, 1000000000 ) ), TInt64FractionPoint2D( F( 1, 3 ), F( 1, 3 ) + F( 1, 1000000000 ) ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ExactKindShapes: big fraction straight-line shapes" )
    try
    {
        typedef TMathBigFraction F;
        // the big fraction has no largest value, so the closed rectangle and box are not offered; segments and polygons are
        typedef CTPoint2D< F > P2;
        typedef CTSegment2D< F > Seg2;
        const Seg2 diagonal( P2( F( 0 ), F( 0 ) ), P2( F( 1 ), F( 1 ) ) );
        ASSERT_TRUE( diagonal.Intersects( Seg2( P2( F( 1, 3 ), F( 1, 3 ) ), P2( F( 1, 3 ), F( 1, 3 ) ) ) ) );
        // far beyond the precision of a Float64: 1 / 10^30 off the line
        F tiny( 1 );
        for ( UInt32 i=0; i<30; ++i )
            tiny = tiny / F( 10 );
        ASSERT_FALSE( diagonal.Intersects( Seg2( P2( F( 1, 3 ), F( 1, 3 ) + tiny ), P2( F( 1, 3 ), F( 1, 3 ) + tiny ) ) ) );
        ASSERT_TRUE( diagonal.Intersects( Seg2( P2( F( 1, 2 ), F( 0 ) ), P2( F( 1, 2 ), F( 1 ) ) ) ) );
        ASSERT_TRUE( diagonal.Intersects( Seg2( P2( F( 1 ) - tiny, F( 1 ) - tiny ), P2( F( 2 ), F( 2 ) ) ) ) );
        TBigFractionPolygon2D ring;
        ring.AddPoint( P2( F( 0 ), F( 0 ) ) );
        ring.AddPoint( P2( F( 1 ), F( 0 ) ) );
        ring.AddPoint( P2( F( 1 ), F( 1 ) ) );
        ring.AddPoint( P2( F( 0 ), F( 1 ) ) );
        ASSERT_TRUE( ring.IsConvex() && ring.IsSimple() && ring.Contains( P2( F( 1, 7 ), F( 6, 7 ) ) ) && !ring.Contains( P2( F( 1 ) + tiny, F( 1, 2 ) ) ) && ring.Contains( P2( F( 1 ), F( 1, 2 ) ) ) );
        TBigFractionSegment3D parsed;
        const TBigFractionSegment3D segment( CTPoint3D< F >( F( 1, 3 ), F( -2 ), F( 5, 7 ) ), CTPoint3D< F >( F( 0 ), F( 1, 9 ), F( 3 ) ) );
        ASSERT_TRUE( parsed.FromString( segment.ToString() ) && parsed == segment );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
