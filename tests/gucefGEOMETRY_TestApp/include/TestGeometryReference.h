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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYREFERENCE_H

/*
 *  Test helpers of the gucefGEOMETRY test application: a seeded generator for reproducible sweeps and small Float64
 *  reference routines that are independent of the code under test. Specific to gucefGEOMETRY, so it lives with its
 *  test application.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GeometryReference {

/**
 *  Seeded generator for test sweeps (SplitMix64, Steele, Lea and Flood 2014): small, fast and identical on every
 *  platform. Deliberately independent of the generators under test.
 */
class CSeededSequence
{
    public:

    explicit CSeededSequence( GUCEF::UInt64 seed )
        : m_state( seed )
    {
    }

    GUCEF::UInt64 NextUInt64( void )
    {
        GUCEF::UInt64 z = ( m_state += 0x9E3779B97F4A7C15ULL );
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        return z ^ ( z >> 31 );
    }

    GUCEF::UInt32 NextUInt32( void )
    {
        return (GUCEF::UInt32) ( NextUInt64() >> 32 );
    }

    /** Uniform in [ low, high ) */
    GUCEF::Float64 NextFloat64( GUCEF::Float64 low, GUCEF::Float64 high )
    {
        GUCEF::Float64 unit = (GUCEF::Float64) ( NextUInt64() >> 11 ) * ( 1.0 / 9007199254740992.0 );
        return low + ( high - low ) * unit;
    }

    /** Uniform integer in [ low, high ] */
    GUCEF::Int32 NextInt32( GUCEF::Int32 low, GUCEF::Int32 high )
    {
        GUCEF::UInt64 range = (GUCEF::UInt64) ( (GUCEF::Int64) high - (GUCEF::Int64) low ) + 1;
        return (GUCEF::Int32) ( (GUCEF::Int64) low + (GUCEF::Int64) ( NextUInt64() % range ) );
    }

    private:

    GUCEF::UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

/** A plain Float64 3-vector for the reference computations */
struct SVec3
{
    GUCEF::Float64 x, y, z;
    SVec3( void ) : x( 0 ), y( 0 ), z( 0 ) {}
    SVec3( GUCEF::Float64 ax, GUCEF::Float64 ay, GUCEF::Float64 az ) : x( ax ), y( ay ), z( az ) {}
};

inline SVec3 operator+( const SVec3& a, const SVec3& b ) { return SVec3( a.x + b.x, a.y + b.y, a.z + b.z ); }
inline SVec3 operator-( const SVec3& a, const SVec3& b ) { return SVec3( a.x - b.x, a.y - b.y, a.z - b.z ); }
inline SVec3 operator*( const SVec3& a, GUCEF::Float64 s ) { return SVec3( a.x * s, a.y * s, a.z * s ); }
inline GUCEF::Float64 Dot( const SVec3& a, const SVec3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline GUCEF::Float64 Length( const SVec3& a ) { return sqrt( Dot( a, a ) ); }

/** The distance from p to the segment ab, with the point on the segment: a Float64 textbook projection */
inline GUCEF::Float64
SegmentDistance( const SVec3& p, const SVec3& a, const SVec3& b, SVec3& closest )
{
    SVec3 ab = b - a;
    GUCEF::Float64 denominator = Dot( ab, ab );
    GUCEF::Float64 t = 0 == denominator ? 0 : Dot( p - a, ab ) / denominator;
    t = t < 0 ? 0 : ( t > 1 ? 1 : t );
    closest = a + ab * t;
    return Length( p - closest );
}

/**
 *  The distance from p to the triangle abc in Float64: the distance to the plane when the projection falls inside
 *  (barycentric coordinates by the area ratios), otherwise the smallest distance to the three edges.
 */
inline GUCEF::Float64
TriangleDistance( const SVec3& p, const SVec3& a, const SVec3& b, const SVec3& c )
{
    SVec3 n( ( b.y - a.y ) * ( c.z - a.z ) - ( b.z - a.z ) * ( c.y - a.y ),
             ( b.z - a.z ) * ( c.x - a.x ) - ( b.x - a.x ) * ( c.z - a.z ),
             ( b.x - a.x ) * ( c.y - a.y ) - ( b.y - a.y ) * ( c.x - a.x ) );
    GUCEF::Float64 nn = Dot( n, n );
    SVec3 e1;
    SVec3 e2;
    SVec3 e3;
    GUCEF::Float64 best = SegmentDistance( p, a, b, e1 );
    GUCEF::Float64 d = SegmentDistance( p, b, c, e2 );
    best = d < best ? d : best;
    d = SegmentDistance( p, c, a, e3 );
    best = d < best ? d : best;
    if ( nn > 0 )
    {
        GUCEF::Float64 h = Dot( p - a, n ) / sqrt( nn );
        SVec3 q = p - n * ( Dot( p - a, n ) / nn );
        /* areas of the sub triangles with the signs of their normals along n */
        SVec3 qa = a - q;
        SVec3 qb = b - q;
        SVec3 qc = c - q;
        SVec3 n1( qb.y * qc.z - qb.z * qc.y, qb.z * qc.x - qb.x * qc.z, qb.x * qc.y - qb.y * qc.x );
        SVec3 n2( qc.y * qa.z - qc.z * qa.y, qc.z * qa.x - qc.x * qa.z, qc.x * qa.y - qc.y * qa.x );
        SVec3 n3( qa.y * qb.z - qa.z * qb.y, qa.z * qb.x - qa.x * qb.z, qa.x * qb.y - qa.y * qb.x );
        if ( Dot( n1, n ) >= 0 && Dot( n2, n ) >= 0 && Dot( n3, n ) >= 0 )
            return fabs( h ) < best ? fabs( h ) : best;
    }
    return best;
}

/**
 *  The distance from p to the triangle abc by dense sampling of the barycentric coordinates ( steps x steps ), an upper
 *  bound of the true distance that no correct closest point can exceed.
 */
inline GUCEF::Float64
TriangleSampledDistance( const SVec3& p, const SVec3& a, const SVec3& b, const SVec3& c, GUCEF::UInt32 steps )
{
    GUCEF::Float64 best = Length( p - a );
    for ( GUCEF::UInt32 i=0; i<=steps; ++i )
    {
        for ( GUCEF::UInt32 j=0; i+j<=steps; ++j )
        {
            GUCEF::Float64 v = (GUCEF::Float64) i / (GUCEF::Float64) steps;
            GUCEF::Float64 w = (GUCEF::Float64) j / (GUCEF::Float64) steps;
            SVec3 q = a * ( 1.0 - v - w ) + b * v + c * w;
            GUCEF::Float64 d = Length( p - q );
            best = d < best ? d : best;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline bool
SameBits( const T& a, const T& b )
{
    return 0 == memcmp( &a, &b, sizeof( T ) );
}

/** |a - b| <= tolerance, false for NaN */
inline bool
Near( GUCEF::Float64 a, GUCEF::Float64 b, GUCEF::Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

}; /* namespace GeometryReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYREFERENCE_H ? */
