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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTGEOSPACEREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTGEOSPACEREFERENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#include "TestConvexReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Helpers of the transform tree tests: seeded TRS transforms, the reference of a chain of local transforms (the
 *  widened TRS values composed in long double by code written out here, not the library's FromTRS and product) and
 *  matrix comparisons.
 */
namespace GeoSpaceReference {

typedef QueryReference::P3 P3;
typedef QueryReference::V3 V3;
typedef QueryReference::D3 D3;
typedef QueryReference::Q4 Q4;
typedef GUCEF::MATH::CTMatrix4D< GUCEF::Float64 > M4;
typedef GUCEF::MATH::CTTransform3D< GUCEF::Float64 > TRS64;

/*-------------------------------------------------------------------------*/

/**
 *  A seeded TRS of the kind T: position in [ -range, range ], a random rotation, scale magnitudes in [ low, high ]
 *  (uniform when 'uniform'), each component negative with probability 'mirror'
 */
template < typename T >
GUCEF::MATH::CTTransform3D< T >
RandomTransform( QueryReference::CRandom& r, GUCEF::Float64 range, bool uniform, GUCEF::Float64 low, GUCEF::Float64 high, GUCEF::Float64 mirror )
{
    const P3 p = r.Point( range );
    const Q4 q = r.Rotation();
    const GUCEF::Float64 s0 = r.Uniform( low, high );
    const GUCEF::Float64 s1 = r.Uniform( low, high );
    const GUCEF::Float64 s2 = r.Uniform( low, high );
    const bool m0 = r.Chance( mirror );
    const bool m1 = r.Chance( mirror );
    const bool m2 = r.Chance( mirror );
    V3 s = uniform ? V3( s0, s0, s0 ) : V3( s0, s1, s2 );
    s = V3( m0 ? -s.x : s.x, m1 ? -s.y : s.y, m2 ? -s.z : s.z );
    return GUCEF::MATH::CTTransform3D< T >( QueryReference::ToV< T >( p.ToPositionVector() ), QueryReference::ToQ< T >( q ), QueryReference::ToV< T >( s ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  An affine matrix in long double, rows 0 .. 2 of [ linear | translation ]: the arithmetic of the reference chain.
 *  Written out here (quaternion to matrix, scale, product) so the reference shares no code with the implementation.
 */
struct SReferenceAffine
{
    long double m[ 3 ][ 4 ];
};

/** The long double affine matrix T * R * S of a TRS of any kind, from the widened values (the rotation renormalised) */
template < typename T >
SReferenceAffine
ReferenceTRS( const GUCEF::MATH::CTTransform3D< T >& t )
{
    const Q4 q = QueryReference::Widen( t.rotation );
    const V3 p = QueryReference::Widen( t.position );
    const V3 s = QueryReference::Widen( t.scale );
    long double x = q.x;
    long double y = q.y;
    long double z = q.z;
    long double w = q.w;
    const long double length = sqrtl( x * x + y * y + z * z + w * w );
    x /= length;
    y /= length;
    z /= length;
    w /= length;
    /* the rotation matrix of a unit quaternion, acting on column vectors */
    const long double r[ 3 ][ 3 ] = { { 1.0L - 2.0L * ( y * y + z * z ), 2.0L * ( x * y - z * w ), 2.0L * ( x * z + y * w ) },
                                      { 2.0L * ( x * y + z * w ), 1.0L - 2.0L * ( x * x + z * z ), 2.0L * ( y * z - x * w ) },
                                      { 2.0L * ( x * z - y * w ), 2.0L * ( y * z + x * w ), 1.0L - 2.0L * ( x * x + y * y ) } };
    const long double scale[ 3 ] = { s.x, s.y, s.z };
    const long double position[ 3 ] = { p.x, p.y, p.z };
    SReferenceAffine a;
    for ( GUCEF::UInt32 row=0; row<3; ++row )
    {
        for ( GUCEF::UInt32 c=0; c<3; ++c )
            a.m[ row ][ c ] = r[ row ][ c ] * scale[ c ];
        a.m[ row ][ 3 ] = position[ row ];
    }
    return a;
}

/** The product a * b of two affine matrices in long double */
inline SReferenceAffine
ReferenceProduct( const SReferenceAffine& a, const SReferenceAffine& b )
{
    SReferenceAffine out;
    for ( GUCEF::UInt32 row=0; row<3; ++row )
    {
        for ( GUCEF::UInt32 c=0; c<4; ++c )
        {
            long double sum = 3 == c ? a.m[ row ][ 3 ] : 0.0L;
            for ( GUCEF::UInt32 k=0; k<3; ++k )
                sum += a.m[ row ][ k ] * b.m[ k ][ c ];
            out.m[ row ][ c ] = sum;
        }
    }
    return out;
}

/** A long double affine matrix rounded to a Float64 matrix (last row 0, 0, 0, 1) */
inline M4
ReferenceToM4( const SReferenceAffine& a )
{
    M4 out;
    for ( GUCEF::UInt32 row=0; row<3; ++row )
        for ( GUCEF::UInt32 c=0; c<4; ++c )
            out( row, c ) = (GUCEF::Float64) a.m[ row ][ c ];
    out( 3, 0 ) = 0.0;
    out( 3, 1 ) = 0.0;
    out( 3, 2 ) = 0.0;
    out( 3, 3 ) = 1.0;
    return out;
}

/** The Float64 matrix of a TRS of any kind (the long double reference, rounded) */
template < typename T >
M4
WideMatrix( const GUCEF::MATH::CTTransform3D< T >& t )
{
    return ReferenceToM4( ReferenceTRS( t ) );
}

/** The reference world matrix of a node: the long double product of the local matrices up the chain, rounded to Float64 */
template < typename T >
M4
ReferenceWorld( const GUCEF::GEOMETRY::CTGeoSpace< T >& node )
{
    SReferenceAffine world = ReferenceTRS( node.GetLocalTransform() );
    for ( const GUCEF::GEOMETRY::CTGeoSpace< T >* p = node.GetParent(); GUCEF_NULL != p; p = p->GetParent() )
        world = ReferenceProduct( ReferenceTRS( p->GetLocalTransform() ), world );
    return ReferenceToM4( world );
}

/** A matrix of any kind widened to Float64 */
template < typename T >
M4
Widen( const GUCEF::MATH::CTMatrix4D< T >& m )
{
    M4 out;
    for ( GUCEF::UInt32 r=0; r<4; ++r )
        for ( GUCEF::UInt32 c=0; c<4; ++c )
            out( r, c ) = (GUCEF::Float64) m( r, c );
    return out;
}

inline M4
Widen( const GUCEF::MATH::CTMatrix4D< GUCEF::MATH::TFixed32_32 >& m )
{
    M4 out;
    for ( GUCEF::UInt32 r=0; r<4; ++r )
        for ( GUCEF::UInt32 c=0; c<4; ++c )
            out( r, c ) = ConvexReference::FixedToFloat64( m( r, c ) );
    return out;
}

/** The largest element difference relative to max( 1, the largest element of the reference ) */
inline GUCEF::Float64
RelativeDifference( const M4& value, const M4& reference )
{
    GUCEF::Float64 largest = 1.0;
    GUCEF::Float64 difference = 0.0;
    for ( GUCEF::UInt32 r=0; r<4; ++r )
    {
        for ( GUCEF::UInt32 c=0; c<4; ++c )
        {
            largest = fabs( reference( r, c ) ) > largest ? fabs( reference( r, c ) ) : largest;
            const GUCEF::Float64 d = fabs( value( r, c ) - reference( r, c ) );
            difference = d > difference ? d : difference;
        }
    }
    return difference / largest;
}

/** True when two matrices are equal bit for bit (every element compared as raw bits) */
template < typename T >
bool
SameMatrixBits( const GUCEF::MATH::CTMatrix4D< T >& a, const GUCEF::MATH::CTMatrix4D< T >& b )
{
    for ( GUCEF::UInt32 i=0; i<16; ++i )
        if ( 0 != memcmp( &a.GetData()[ i ], &b.GetData()[ i ], sizeof( T ) ) )
            return false;
    return true;
}

/** Adds every element of a fixed-point matrix to a hash */
inline void
HashMatrix( ConvexReference::CRawHash& hash, const GUCEF::MATH::CTMatrix4D< GUCEF::MATH::TFixed32_32 >& m )
{
    for ( GUCEF::UInt32 i=0; i<16; ++i )
        hash.Add( m.GetData()[ i ] );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GeoSpaceReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTGEOSPACEREFERENCE_H ? */
