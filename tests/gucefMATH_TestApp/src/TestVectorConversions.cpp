/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CORESTRUCTINTEROP_H
#include "gucefMATH_CoreStructInterop.h"
#define GUCEF_MATH_CORESTRUCTINTEROP_H
#endif /* GUCEF_MATH_CORESTRUCTINTEROP_H ? */

#include "TestMathReference.h"
#include "TestVectorConversions.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The CORE structures and the gucefMATH types they convert to have the same size (the array forms rely on it) */
GUCEF_MATH_STATIC_ASSERT( sizeof( CORE::TPoint2DF ) == sizeof( TFloat32Point2D ), TPoint2DF_matches_TFloat32Point2D );
GUCEF_MATH_STATIC_ASSERT( sizeof( CORE::TPoint3DF ) == sizeof( TFloat32Point3D ), TPoint3DF_matches_TFloat32Point3D );
GUCEF_MATH_STATIC_ASSERT( sizeof( CORE::TPoint2DI ) == sizeof( TInt32Point2D ), TPoint2DI_matches_TInt32Point2D );
GUCEF_MATH_STATIC_ASSERT( sizeof( CORE::TPoint3DI ) == sizeof( TInt32Point3D ), TPoint3DI_matches_TInt32Point3D );
GUCEF_MATH_STATIC_ASSERT( sizeof( CORE::TSize3D ) == sizeof( TFloat32Vector3D ), TSize3D_matches_TFloat32Vector3D );

/*-------------------------------------------------------------------------*/

static Float32 Float32FromBits( UInt32 bits ) { Float32 value; memcpy( &value, &bits, 4 ); return value; }

/** A random finite Float32 from its bit pattern (every exponent, subnormals included) */
static Float32
RandomFiniteFloat32( MathReference::CSeededSequence& generator )
{
    for ( ;; )
    {
        Float32 value = Float32FromBits( generator.NextUInt32() );
        if ( CTNumericTraits< Float32 >::IsFinite( value ) )
            return value;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVectorConversionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VectorConversions" );

    GUCEF_TESTFW_TESTCASE( "VectorConversions: Float32 to Float64 is exact, Float64 to Float32 rounds to nearest" )
    try
    {
        const UInt64 seed = 1234567;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat32Vector3D v( RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ) );
            TFloat64Vector3D wide( v );
            if ( wide.x != (Float64) v.x || wide.y != (Float64) v.y || wide.z != (Float64) v.z )
                ++failures;
            if ( TFloat32Vector3D( wide ) != v )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VectorConversions: float widening seed " << seed << " failures " << failures );

        const Float64 tie = 1.0 + ldexp( 1.0, -24 );
        ASSERT_TRUE( TFloat32Vector2D( TFloat64Vector2D( tie, -tie ) ) == TFloat32Vector2D( 1.0f, -1.0f ) );
        ASSERT_TRUE( TFloat32Vector4D( TFloat64Vector4D( 0.1, 0.2, 0.3, 0.4 ) ) == TFloat32Vector4D( 0.1f, 0.2f, 0.3f, 0.4f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorConversions: floats to integers and fixed point, half to even and saturating" )
    try
    {
        ASSERT_TRUE( TInt32Vector3D( TFloat32Vector3D( 1.5f, 2.5f, -2.5f ) ) == TInt32Vector3D( 2, 2, -2 ) );
        ASSERT_TRUE( TInt32Vector3D( TFloat64Vector3D( 3e9, -3e9, 0.49 ) ) == TInt32Vector3D( 2147483647, (Int32) 0x80000000u, 0 ) );
        ASSERT_TRUE( TUInt8Vector3D( TFloat32Vector3D( -1.0f, 255.6f, 127.5f ) ) == TUInt8Vector3D( 0, 255, 128 ) );
        ASSERT_TRUE( TInt16Point2D( TFloat64Point2D( 0.5, 1.5 ) ) == TInt16Point2D( 0, 2 ) );

        TFixed32Vector3D fixed( TFloat64Vector3D( 0.1, -1.5, 1e6 ) );
        ASSERT_TRUE( 6554 == fixed.x.GetRaw() );
        ASSERT_TRUE( -98304 == fixed.y.GetRaw() );
        ASSERT_TRUE( TFixed32::Highest() == fixed.z );
        TFixed64Vector2D fixed64( TFloat32Vector2D( 0.75f, -0.125f ) );
        ASSERT_TRUE( TFixed64( 3 ) / TFixed64( 4 ) == fixed64.x && -( (Int64) 1 << 29 ) == fixed64.y.GetRaw() );

        #ifndef GUCEF_MATH_DEBUG_MODE
        // NaN into a kind without NaN gives 0 (debug builds assert on it)
        const Float32 nan = (Float32) MathQuietNaN64();
        ASSERT_TRUE( TInt32Vector3D( TFloat32Vector3D( nan, 1.0f, nan ) ) == TInt32Vector3D( 0, 1, 0 ) );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorConversions: fixed point, integers and fractions to other kinds" )
    try
    {
        TFixed32Vector3D fixed( TFixed32::FromRaw( 1 ), TFixed32( -3 ), TFixed32::FromRaw( 98304 ) );
        TFloat64Vector3D asFloat( fixed );
        ASSERT_TRUE( asFloat == TFloat64Vector3D( 1.0 / 65536.0, -3.0, 1.5 ) );
        ASSERT_TRUE( TFixed32Vector3D( asFloat ) == fixed );
        TFixed64Vector3D widened( fixed );
        ASSERT_TRUE( TFixed64Vector3D( TFixed64::FromRaw( (Int64) 1 << 16 ), TFixed64( -3 ), TFixed64( 3 ) / TFixed64( 2 ) ) == widened );

        typedef CORE::TInt64Fraction F;
        TInt64FractionVector3D exact( fixed );
        ASSERT_TRUE( exact == TInt64FractionVector3D( F( 1, 65536 ), F( -3 ), F( 3, 2 ) ) );
        ASSERT_TRUE( TFixed32Vector3D( exact ) == fixed );
        ASSERT_TRUE( TFloat32Vector3D( TInt64FractionVector3D( F( 1, 3 ), F( 2 ), F( -1, 4 ) ) ) == TFloat32Vector3D( 0.33333334f, 2.0f, -0.25f ) );
        ASSERT_TRUE( TInt32Vector3D( TInt64FractionVector3D( F( 5, 2 ), F( 7, 2 ), F( -5, 2 ) ) ) == TInt32Vector3D( 2, 4, -2 ) );
        ASSERT_TRUE( TBigFractionVector2D( TInt32Vector2D( 7, -9 ) ) == TBigFractionVector2D( TMathBigFraction( 7 ), TMathBigFraction( -9 ) ) );
        ASSERT_TRUE( TFloat64Point3D( TInt64Point3D( 1, 2, 3 ) ) == TFloat64Point3D( 1, 2, 3 ) );
        ASSERT_TRUE( TInt64FractionPoint2D( TFloat32Point2D( 0.5f, -0.25f ) ) == TInt64FractionPoint2D( F( 1, 2 ), F( -1, 4 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorConversions: CORE struct interop round trips, single values and arrays" )
    try
    {
        CORE::TPoint2DF p2f; p2f.x = 1.5f; p2f.y = -2.25f;
        CORE::TPoint3DF p3f; p3f.x = 1.5f; p3f.y = -2.25f; p3f.z = 1e-30f;
        CORE::TPoint2DI p2i; p2i.x = -7; p2i.y = 2147483647;
        CORE::TPoint3DI p3i; p3i.x = -7; p3i.y = 2147483647; p3i.z = (Int32) 0x80000000u;
        CORE::TSize3D size; size.width = 640.0f; size.height = 480.0f; size.depth = 0.5f;

        ASSERT_TRUE( ToMath( p2f ) == TFloat32Point2D( 1.5f, -2.25f ) );
        ASSERT_TRUE( ToMath( p3f ) == TFloat32Point3D( 1.5f, -2.25f, 1e-30f ) );
        ASSERT_TRUE( ToMath( p2i ) == TInt32Point2D( -7, 2147483647 ) );
        ASSERT_TRUE( ToMath( p3i ) == TInt32Point3D( -7, 2147483647, (Int32) 0x80000000u ) );
        ASSERT_TRUE( ToMathVector( size ) == TFloat32Vector3D( 640.0f, 480.0f, 0.5f ) );

        CORE::TPoint2DF b2f = ToCore( ToMath( p2f ) );
        CORE::TPoint3DF b3f = ToCore( ToMath( p3f ) );
        CORE::TPoint2DI b2i = ToCore( ToMath( p2i ) );
        CORE::TPoint3DI b3i = ToCore( ToMath( p3i ) );
        CORE::TSize3D bSize = ToCoreSize( ToMathVector( size ) );
        ASSERT_TRUE( 0 == memcmp( &b2f, &p2f, sizeof( p2f ) ) );
        ASSERT_TRUE( 0 == memcmp( &b3f, &p3f, sizeof( p3f ) ) );
        ASSERT_TRUE( 0 == memcmp( &b2i, &p2i, sizeof( p2i ) ) );
        ASSERT_TRUE( 0 == memcmp( &b3i, &p3i, sizeof( p3i ) ) );
        ASSERT_TRUE( 0 == memcmp( &bSize, &size, sizeof( size ) ) );

        // arrays by memcpy of identical layouts, in both directions
        CORE::TPoint3DF coreArray[ 64 ];
        TFloat32Point3D mathArray[ 64 ];
        CORE::TPoint3DF backArray[ 64 ];
        for ( UInt32 i=0; i<64; ++i )
        {
            coreArray[ i ].x = (Float32) i;
            coreArray[ i ].y = (Float32) i * 0.5f;
            coreArray[ i ].z = -(Float32) i;
        }
        ToMath( coreArray, mathArray, 64 );
        ToCore( mathArray, backArray, 64 );
        bool same = 0 == memcmp( coreArray, backArray, sizeof( coreArray ) );
        for ( UInt32 i=0; i<64; ++i )
            same = same && mathArray[ i ] == ToMath( coreArray[ i ] );
        ASSERT_TRUE( same );

        CORE::TPoint2DF core2f[ 3 ] = { p2f, p2f, p2f };
        TFloat32Point2D math2f[ 3 ];
        CORE::TPoint2DF back2f[ 3 ];
        ToMath( core2f, math2f, 3 );
        ToCore( math2f, back2f, 3 );
        ASSERT_TRUE( 0 == memcmp( core2f, back2f, sizeof( core2f ) ) && math2f[ 2 ] == ToMath( p2f ) );

        CORE::TPoint2DI core2i[ 2 ] = { p2i, p2i };
        TInt32Point2D math2i[ 2 ];
        CORE::TPoint2DI back2i[ 2 ];
        ToMath( core2i, math2i, 2 );
        ToCore( math2i, back2i, 2 );
        ASSERT_TRUE( 0 == memcmp( core2i, back2i, sizeof( core2i ) ) && math2i[ 1 ] == ToMath( p2i ) );

        CORE::TPoint3DI core3i[ 2 ] = { p3i, p3i };
        TInt32Point3D math3i[ 2 ];
        CORE::TPoint3DI back3i[ 2 ];
        ToMath( core3i, math3i, 2 );
        ToCore( math3i, back3i, 2 );
        ASSERT_TRUE( 0 == memcmp( core3i, back3i, sizeof( core3i ) ) && math3i[ 1 ] == ToMath( p3i ) );

        CORE::TSize3D coreSizes[ 2 ] = { size, size };
        TFloat32Vector3D mathSizes[ 2 ];
        CORE::TSize3D backSizes[ 2 ];
        ToMathVector( coreSizes, mathSizes, 2 );
        ToCoreSize( mathSizes, backSizes, 2 );
        ASSERT_TRUE( 0 == memcmp( coreSizes, backSizes, sizeof( coreSizes ) ) && mathSizes[ 1 ] == ToMathVector( size ) );
        ToMath( coreArray, mathArray, 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
