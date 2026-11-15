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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTQUERYREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTQUERYREFERENCE_H

/*
 *  Test helpers of the query tests of the gucefGEOMETRY test application: a seeded random source over the stream of
 *  gucefMATH, random shapes of every kind, the widening of shapes to Float64, and references for the queries that
 *  share no code with the functions under test: marching with bisection on Contains for ray casts, alternating
 *  projections between the nearest-point members of two convex shapes for distances and overlap, and dense sampling.
 *  Specific to gucefGEOMETRY, so it lives with its test application.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_GEOMETRY_QUERIES_H
#include "gucefGEOMETRY_queries.h"
#define GUCEF_GEOMETRY_QUERIES_H
#endif /* GUCEF_GEOMETRY_QUERIES_H ? */

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace QueryReference {

typedef GUCEF::MATH::CTPoint3D< GUCEF::Float64 > P3;
typedef GUCEF::MATH::CTVector3D< GUCEF::Float64 > V3;
typedef GUCEF::MATH::CTDirectionVector3D< GUCEF::Float64 > D3;
typedef GUCEF::MATH::CTQuaternion< GUCEF::Float64 > Q4;

/*-------------------------------------------------------------------------*/

/**
 *  A seeded random source over the random stream of gucefMATH. Every draw goes into a local variable before use, so that
 *  the order of evaluation of arguments (it differs between GCC and Clang) cannot change a sequence.
 */
class CRandom
{
    public:

    explicit CRandom( GUCEF::UInt64 seed )
        : m_seed( seed )
        , m_stream( GUCEF::MATH::CSplitMix64( seed ) )
    {
    }

    GUCEF::UInt64 GetSeed( void ) const { return m_seed; }

    GUCEF::Float64 Uniform( GUCEF::Float64 low, GUCEF::Float64 high ) { return m_stream.Range( low, high ); }

    /** Uniform integer in [ 0, count ) */
    GUCEF::Int32 Index( GUCEF::Int32 count ) { return (GUCEF::Int32) m_stream.Below( (GUCEF::UInt32) count ); }

    bool Chance( GUCEF::Float64 probability ) { return m_stream.NextFloat64() < probability; }

    P3 Point( GUCEF::Float64 range )
    {
        const GUCEF::Float64 x = Uniform( -range, range );
        const GUCEF::Float64 y = Uniform( -range, range );
        const GUCEF::Float64 z = Uniform( -range, range );
        return P3( x, y, z );
    }

    V3 Vector( GUCEF::Float64 range )
    {
        const GUCEF::Float64 x = Uniform( -range, range );
        const GUCEF::Float64 y = Uniform( -range, range );
        const GUCEF::Float64 z = Uniform( -range, range );
        return V3( x, y, z );
    }

    D3 Direction( void )
    {
        for ( ;; )
        {
            const V3 v = Vector( 1.0 );
            D3 d;
            if ( v.GetSquaredLength() > 0.04 && D3::TryCreate( v, d ) )
                return d;
        }
    }

    Q4 Rotation( void )
    {
        const D3 axis = Direction();
        const GUCEF::Float64 angle = Uniform( 0.0, 6.283185307179586 );
        return Q4::FromAxisAngle( axis, GUCEF::MATH::CTRadian< GUCEF::Float64 >( angle ) );
    }

    private:

    GUCEF::UInt64 m_seed;
    GUCEF::MATH::CTRandomStream< GUCEF::MATH::CSplitMix64 > m_stream;
};

/*-------------------------------------------------------------------------*/

/** A point, vector, direction and quaternion of the kind T from Float64 values */
template < typename T > GUCEF::MATH::CTPoint3D< T > ToP( const P3& p ) { return GUCEF::MATH::CTPoint3D< T >( (T) p.x, (T) p.y, (T) p.z ); }
template < typename T > GUCEF::MATH::CTVector3D< T > ToV( const V3& v ) { return GUCEF::MATH::CTVector3D< T >( (T) v.x, (T) v.y, (T) v.z ); }
template < typename T > GUCEF::MATH::CTDirectionVector3D< T > ToD( const D3& d ) { return GUCEF::MATH::CTDirectionVector3D< T >( ToV< T >( d.AsVector() ) ); }
template < typename T > GUCEF::MATH::CTQuaternion< T > ToQ( const Q4& q ) { return GUCEF::MATH::CTQuaternion< T >::FromXYZW( (T) q.x, (T) q.y, (T) q.z, (T) q.w ).Normalized(); }

/** The exact Float64 values of a point, vector, direction and quaternion of the kind T */
template < typename T > P3 Widen( const GUCEF::MATH::CTPoint3D< T >& p ) { return P3( (GUCEF::Float64) p.x, (GUCEF::Float64) p.y, (GUCEF::Float64) p.z ); }
template < typename T > V3 Widen( const GUCEF::MATH::CTVector3D< T >& v ) { return V3( (GUCEF::Float64) v.x, (GUCEF::Float64) v.y, (GUCEF::Float64) v.z ); }
template < typename T > Q4 Widen( const GUCEF::MATH::CTQuaternion< T >& q ) { return Q4::FromXYZW( (GUCEF::Float64) q.x, (GUCEF::Float64) q.y, (GUCEF::Float64) q.z, (GUCEF::Float64) q.w ); }
template < typename T > D3 WidenD( const GUCEF::MATH::CTDirectionVector3D< T >& d )
{
    D3 result;
    D3::TryCreate( Widen( d.AsVector() ), result );
    return result;
}

/*-------------------------------------------------------------------------*/

/** The shapes in Float64 with the exact values of the shape in the kind T */
template < typename T > GUCEF::GEOMETRY::CTSphere< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTSphere< T >& s ) { return GUCEF::GEOMETRY::CTSphere< GUCEF::Float64 >( Widen( s.centre ), (GUCEF::Float64) s.radius ); }
template < typename T > GUCEF::GEOMETRY::CTAxisAlignedBox3D< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTAxisAlignedBox3D< T >& b ) { return GUCEF::GEOMETRY::CTAxisAlignedBox3D< GUCEF::Float64 >( Widen( b.min ), Widen( b.max ) ); }
template < typename T > GUCEF::GEOMETRY::CTOrientedBox3D< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTOrientedBox3D< T >& b ) { return GUCEF::GEOMETRY::CTOrientedBox3D< GUCEF::Float64 >( Widen( b.centre ), Widen( b.rotation ).Normalized(), Widen( b.halfExtents ) ); }
template < typename T > GUCEF::GEOMETRY::CTCapsule< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTCapsule< T >& c ) { return GUCEF::GEOMETRY::CTCapsule< GUCEF::Float64 >( Widen( c.start ), Widen( c.end ), (GUCEF::Float64) c.radius ); }
template < typename T > GUCEF::GEOMETRY::CTCylinder< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTCylinder< T >& c ) { return GUCEF::GEOMETRY::CTCylinder< GUCEF::Float64 >( Widen( c.start ), Widen( c.end ), (GUCEF::Float64) c.radius ); }
template < typename T > GUCEF::GEOMETRY::CTCone< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTCone< T >& c ) { return GUCEF::GEOMETRY::CTCone< GUCEF::Float64 >( Widen( c.apex ), WidenD( c.axis ), (GUCEF::Float64) c.height, (GUCEF::Float64) c.baseRadius ); }
template < typename T > GUCEF::GEOMETRY::CTTriangle3D< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTTriangle3D< T >& t ) { return GUCEF::GEOMETRY::CTTriangle3D< GUCEF::Float64 >( Widen( t.a ), Widen( t.b ), Widen( t.c ) ); }
template < typename T > GUCEF::GEOMETRY::CTSegment3D< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTSegment3D< T >& s ) { return GUCEF::GEOMETRY::CTSegment3D< GUCEF::Float64 >( Widen( s.start ), Widen( s.end ) ); }
template < typename T > GUCEF::GEOMETRY::CTRay3D< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTRay3D< T >& r ) { return GUCEF::GEOMETRY::CTRay3D< GUCEF::Float64 >( Widen( r.origin ), WidenD( r.direction ) ); }
template < typename T > GUCEF::GEOMETRY::CTPlane< GUCEF::Float64 > Widen( const GUCEF::GEOMETRY::CTPlane< T >& p ) { return GUCEF::GEOMETRY::CTPlane< GUCEF::Float64 >( WidenD( p.normal ), (GUCEF::Float64) p.offset ); }

template < typename T >
GUCEF::GEOMETRY::CTFrustum< GUCEF::Float64 >
Widen( const GUCEF::GEOMETRY::CTFrustum< T >& f )
{
    GUCEF::GEOMETRY::CTPlane< GUCEF::Float64 > planes[ 6 ];
    for ( GUCEF::UInt32 i=0; i<6; ++i )
        planes[ i ] = Widen( f.GetPlane( i ) );
    GUCEF::GEOMETRY::CTFrustum< GUCEF::Float64 > result;
    GUCEF::GEOMETRY::CTFrustum< GUCEF::Float64 >::TryFromPlanes( planes, result );
    return result;
}

template < typename T >
GUCEF::GEOMETRY::CTPlaneBoundedVolume< GUCEF::Float64 >
Widen( const GUCEF::GEOMETRY::CTPlaneBoundedVolume< T >& v )
{
    GUCEF::GEOMETRY::CTPlaneBoundedVolume< GUCEF::Float64 > result;
    for ( GUCEF::UInt32 i=0; i<v.GetPlaneCount(); ++i )
        result.AddPlane( Widen( v.GetPlane( i ) ) );
    return result;
}

/*-------------------------------------------------------------------------*/

/** The Float64 type of a shape of any kind: SWidened< CTSphere< Float32 > >::Type is CTSphere< Float64 > */
template < typename TShape > struct SWidened;
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTSphere< T > > { typedef GUCEF::GEOMETRY::CTSphere< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTAxisAlignedBox3D< T > > { typedef GUCEF::GEOMETRY::CTAxisAlignedBox3D< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTOrientedBox3D< T > > { typedef GUCEF::GEOMETRY::CTOrientedBox3D< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTCapsule< T > > { typedef GUCEF::GEOMETRY::CTCapsule< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTCylinder< T > > { typedef GUCEF::GEOMETRY::CTCylinder< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTCone< T > > { typedef GUCEF::GEOMETRY::CTCone< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTTriangle3D< T > > { typedef GUCEF::GEOMETRY::CTTriangle3D< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTSegment3D< T > > { typedef GUCEF::GEOMETRY::CTSegment3D< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTPlane< T > > { typedef GUCEF::GEOMETRY::CTPlane< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTFrustum< T > > { typedef GUCEF::GEOMETRY::CTFrustum< GUCEF::Float64 > Type; };
template < typename T > struct SWidened< GUCEF::GEOMETRY::CTPlaneBoundedVolume< T > > { typedef GUCEF::GEOMETRY::CTPlaneBoundedVolume< GUCEF::Float64 > Type; };

/*-------------------------------------------------------------------------*/

/** Random shapes of the kind T of a size and position that make about a third of the pairs overlap */
template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTSphere< T >& s )
{
    const P3 c = r.Point( 1.5 );
    const GUCEF::Float64 radius = r.Uniform( 0.2, 1.2 );
    s = GUCEF::GEOMETRY::CTSphere< T >( ToP< T >( c ), (T) radius );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTAxisAlignedBox3D< T >& b )
{
    const P3 c = r.Point( 1.5 );
    const GUCEF::Float64 hx = r.Uniform( 0.2, 1.0 );
    const GUCEF::Float64 hy = r.Uniform( 0.2, 1.0 );
    const GUCEF::Float64 hz = r.Uniform( 0.2, 1.0 );
    b = GUCEF::GEOMETRY::CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( c ), ToV< T >( V3( hx, hy, hz ) ) );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTOrientedBox3D< T >& b )
{
    const P3 c = r.Point( 1.5 );
    const Q4 q = r.Rotation();
    const GUCEF::Float64 hx = r.Uniform( 0.2, 1.0 );
    const GUCEF::Float64 hy = r.Uniform( 0.2, 1.0 );
    const GUCEF::Float64 hz = r.Uniform( 0.2, 1.0 );
    b = GUCEF::GEOMETRY::CTOrientedBox3D< T >( ToP< T >( c ), ToQ< T >( q ), ToV< T >( V3( hx, hy, hz ) ) );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTCapsule< T >& c )
{
    const P3 start = r.Point( 1.5 );
    const V3 delta = r.Vector( 0.8 );
    const GUCEF::Float64 radius = r.Uniform( 0.1, 0.8 );
    c = GUCEF::GEOMETRY::CTCapsule< T >( ToP< T >( start ), ToP< T >( start + delta ), (T) radius );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTCylinder< T >& c )
{
    const P3 start = r.Point( 1.5 );
    const V3 delta = r.Vector( 0.8 );
    const GUCEF::Float64 radius = r.Uniform( 0.1, 0.8 );
    c = GUCEF::GEOMETRY::CTCylinder< T >( ToP< T >( start ), ToP< T >( start + delta ), (T) radius );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTCone< T >& c )
{
    const P3 apex = r.Point( 1.5 );
    const D3 axis = r.Direction();
    const GUCEF::Float64 height = r.Uniform( 0.5, 2.0 );
    const GUCEF::Float64 radius = r.Uniform( 0.2, 1.2 );
    c = GUCEF::GEOMETRY::CTCone< T >( ToP< T >( apex ), ToD< T >( axis ), (T) height, (T) radius );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTTriangle3D< T >& t )
{
    const P3 a = r.Point( 1.5 );
    const P3 b = r.Point( 1.5 );
    const P3 c = r.Point( 1.5 );
    t = GUCEF::GEOMETRY::CTTriangle3D< T >( ToP< T >( a ), ToP< T >( b ), ToP< T >( c ) );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTSegment3D< T >& s )
{
    const P3 a = r.Point( 1.5 );
    const P3 b = r.Chance( 0.03 ) ? a : r.Point( 1.5 );
    s = GUCEF::GEOMETRY::CTSegment3D< T >( ToP< T >( a ), ToP< T >( b ) );
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTPlane< T >& p )
{
    const P3 point = r.Point( 1.5 );
    const D3 normal = r.Direction();
    p = GUCEF::GEOMETRY::CTPlane< T >::FromPointNormal( ToP< T >( point ), ToD< T >( normal ) );
}

/** A random projection convention: the four named ones and two others */
inline GUCEF::MATH::SProjectionConvention
RandomConvention( CRandom& r )
{
    switch ( r.Index( 6 ) )
    {
        case 0: return GUCEF::MATH::SProjectionConvention::OpenGL();
        case 1: return GUCEF::MATH::SProjectionConvention::Vulkan();
        case 2: return GUCEF::MATH::SProjectionConvention::Direct3D();
        case 3: return GUCEF::MATH::SProjectionConvention::ReversedZInfinite( GUCEF::MATH::HANDEDNESS_RIGHT );
        case 4: return GUCEF::MATH::SProjectionConvention::ReversedZInfinite( GUCEF::MATH::HANDEDNESS_LEFT );
        default: return GUCEF::MATH::SProjectionConvention::Make( GUCEF::MATH::HANDEDNESS_RIGHT, GUCEF::MATH::DEPTHRANGE_MINUS_ONE_TO_ONE, true, false, false );
    }
}

/** A perspective camera in a random pose: the frustum of its combined matrix, with the parameters that define it */
template < typename T >
struct SCamera
{
    GUCEF::MATH::SProjectionConvention convention;
    P3 eye;
    D3 forward;
    D3 up;
    GUCEF::Float64 tanHalfY;
    GUCEF::Float64 tanHalfX;
    GUCEF::Float64 nearDistance;
    GUCEF::Float64 farDistance;
    GUCEF::GEOMETRY::CTFrustum< T > frustum;

    SCamera( void ) : convention( GUCEF::MATH::SProjectionConvention::OpenGL() ), eye(), forward(), up(), tanHalfY( 1 ), tanHalfX( 1 ), nearDistance( 1 ), farDistance( 2 ), frustum() {}
};

template < typename T >
void
RandomCamera( CRandom& r, SCamera< T >& camera )
{
    typedef GUCEF::MATH::CTMatrix4D< T > TMatrix;
    camera.convention = RandomConvention( r );
    camera.eye = r.Point( 3.0 );
    camera.forward = r.Direction();
    const D3 sideways = r.Direction();
    V3 upVector = sideways.AsVector() - camera.forward.AsVector() * sideways.Dot( camera.forward.AsVector() );
    if ( !D3::TryCreate( upVector, camera.up ) )
        D3::TryCreate( camera.forward.AsVector().GetAnyPerpendicular(), camera.up );
    const GUCEF::Float64 fovY = r.Uniform( 0.5, 1.7 );
    const GUCEF::Float64 aspect = r.Uniform( 0.7, 2.0 );
    camera.nearDistance = r.Uniform( 0.1, 1.0 );
    camera.farDistance = r.Uniform( 3.0, 10.0 );
    camera.tanHalfY = tan( fovY * 0.5 );
    camera.tanHalfX = camera.tanHalfY * aspect;
    const P3 target = camera.eye + camera.forward.AsVector();
    const TMatrix view = TMatrix::LookAt( ToP< T >( camera.eye ), ToP< T >( target ), ToV< T >( camera.up.AsVector() ), camera.convention.handedness );
    const TMatrix projection = TMatrix::Perspective( GUCEF::MATH::CTRadian< T >( (T) fovY ), (T) aspect, (T) camera.nearDistance, (T) camera.farDistance, camera.convention );
    camera.frustum = GUCEF::GEOMETRY::CTFrustum< T >::FromViewProjection( projection * view, camera.convention );
}

/** The frustum of a camera pose and parameters by geometry alone: is the world point inside the view volume? Returns the signed margin ( positive inside, relative to the extent at that depth ) */
template < typename T >
GUCEF::Float64
CameraMargin( const SCamera< T >& camera, const P3& p )
{
    const V3 d = p - camera.eye;
    const GUCEF::Float64 depth = d.Dot( camera.forward.AsVector() );
    const V3 right = camera.forward.AsVector().Cross( camera.up.AsVector() );
    const GUCEF::Float64 x = d.Dot( right );
    const GUCEF::Float64 y = d.Dot( camera.up.AsVector() );
    GUCEF::Float64 margin = depth - camera.nearDistance;
    if ( !camera.convention.infiniteFar )
        margin = fmin( margin, camera.farDistance - depth );
    margin = fmin( margin, camera.tanHalfX * depth - fabs( x ) );
    margin = fmin( margin, camera.tanHalfY * depth - fabs( y ) );
    return margin;
}

/** A random bounded convex volume: planes tangent to a sphere of about unit size, plus four that make it bounded */
template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTPlaneBoundedVolume< T >& v )
{
    typedef GUCEF::GEOMETRY::CTPlane< T > TPlane;
    const P3 centre = r.Point( 1.0 );
    const GUCEF::Float64 radius = r.Uniform( 0.5, 1.2 );
    v.Clear();
    const GUCEF::Float64 s = 0.5773502691896258;
    const V3 fixedNormals[ 4 ] = { V3( s, s, s ), V3( s, -s, -s ), V3( -s, s, -s ), V3( -s, -s, s ) };
    for ( GUCEF::UInt32 i=0; i<4; ++i )
    {
        D3 n;
        D3::TryCreate( fixedNormals[ i ], n );
        const V3 inward = n.AsVector() * -1.0;
        v.AddPlane( TPlane::FromPointNormal( ToP< T >( centre + n.AsVector() * radius ), ToD< T >( D3( inward ) ) ) );
    }
    const GUCEF::Int32 extra = 2 + r.Index( 5 );
    for ( GUCEF::Int32 i=0; i<extra; ++i )
    {
        const D3 n = r.Direction();
        const GUCEF::Float64 d = radius * r.Uniform( 1.0, 1.5 );
        v.AddPlane( TPlane::FromPointNormal( ToP< T >( centre + n.AsVector() * d ), ToD< T >( D3( n.AsVector() * -1.0 ) ) ) );
    }
}

template < typename T >
void
RandomShape( CRandom& r, GUCEF::GEOMETRY::CTFrustum< T >& f )
{
    SCamera< T > camera;
    RandomCamera( r, camera );
    f = camera.frustum;
}

/*-------------------------------------------------------------------------*/

/** The first distance along the ray at which the solid contains the point, by marching and bisection on Contains only */
template < typename TShape >
bool
MarchToSolid( const TShape& shape, const GUCEF::GEOMETRY::CTRay3D< GUCEF::Float64 >& ray, GUCEF::Float64 maxDistance, GUCEF::Float64 step, GUCEF::Float64& distance )
{
    if ( shape.Contains( ray.origin ) )
    {
        distance = 0;
        return true;
    }
    GUCEF::Float64 previous = 0;
    for ( GUCEF::Float64 t=step; t<=maxDistance + step; t+=step )
    {
        if ( shape.Contains( ray.GetPoint( t ) ) )
        {
            GUCEF::Float64 lo = previous;
            GUCEF::Float64 hi = t;
            for ( GUCEF::UInt32 i=0; i<60; ++i )
            {
                const GUCEF::Float64 mid = 0.5 * ( lo + hi );
                if ( shape.Contains( ray.GetPoint( mid ) ) )
                    hi = mid;
                else
                    lo = mid;
            }
            distance = hi;
            return true;
        }
        previous = t;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Alternating projections between two convex shapes with a ClosestPoint member (Cheney and Goldstein): the points
 *  converge to a nearest pair, or into the common part of overlapping shapes. Returns the final distance ( an upper
 *  bound of the true distance that is tight for separated shapes ).
 */
template < typename TA, typename TB >
GUCEF::Float64
AlternatingProjectionDistance( const TA& a, const TB& b, const P3& start, P3& onA, P3& onB, GUCEF::UInt32 iterations )
{
    P3 x = start;
    P3 y = b.ClosestPoint( x );
    GUCEF::Float64 previous = -1;
    for ( GUCEF::UInt32 i=0; i<iterations; ++i )
    {
        x = a.ClosestPoint( y );
        y = b.ClosestPoint( x );
        const GUCEF::Float64 d = ( y - x ).GetLength();
        if ( fabs( previous - d ) < 1.0e-15 && d > 0 )
            break;
        previous = d;
        if ( d < 1.0e-13 )
            break;
    }
    onA = x;
    onB = y;
    return ( y - x ).GetLength();
}

/** A point of the shape to start from: the centre of its bounds */
template < typename TShape >
P3
Representative( const TShape& shape )
{
    return shape.GetBounds().GetCentre();
}

/** The foot of the origin on the plane */
inline P3
Representative( const GUCEF::GEOMETRY::CTPlane< GUCEF::Float64 >& plane )
{
    return plane.Project( P3() );
}

/** Alternating projections starting from a point of the first shape */
template < typename TA, typename TB >
GUCEF::Float64
AlternatingProjectionDistance( const TA& a, const TB& b, P3& onA, P3& onB, GUCEF::UInt32 iterations )
{
    return AlternatingProjectionDistance( a, b, Representative( a ), onA, onB, iterations );
}

/**
 *  The distance of two convex shapes by alternating projections, run again for longer when the first run ended at a
 *  distance that might still be falling (the convergence is slow for shapes that touch at a grazing angle).
 */
template < typename TA, typename TB >
GUCEF::Float64
ConvergedDistance( const TA& a, const TB& b, P3& onA, P3& onB )
{
    GUCEF::Float64 d = AlternatingProjectionDistance( a, b, onA, onB, 3000 );
    if ( d > 1.0e-9 && d < 0.05 )
        d = AlternatingProjectionDistance( a, b, onA, onB, 400000 );
    return d;
}

/*-------------------------------------------------------------------------*/

/** 'count' points on and in the shape (Float64), as a dense sampling for upper bounds of distances */
inline void
SampleShape( const GUCEF::GEOMETRY::CTSphere< GUCEF::Float64 >& s, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        const D3 d = r.Direction();
        const GUCEF::Float64 f = 0 == i % 2 ? 1.0 : r.Uniform( 0.0, 1.0 );
        out.push_back( s.centre + d.AsVector() * ( s.radius * f ) );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTAxisAlignedBox3D< GUCEF::Float64 >& b, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    for ( GUCEF::UInt32 i=0; i<8; ++i )
        out.push_back( b.GetCorner( i ) );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        const GUCEF::Float64 x = r.Uniform( b.min.x, b.max.x );
        const GUCEF::Float64 y = r.Uniform( b.min.y, b.max.y );
        const GUCEF::Float64 z = r.Uniform( b.min.z, b.max.z );
        P3 p( x, y, z );
        if ( 0 != i % 2 )
        {
            const GUCEF::UInt32 axis = (GUCEF::UInt32) r.Index( 3 );
            p.SetComponent( axis, r.Chance( 0.5 ) ? b.min.GetComponent( axis ) : b.max.GetComponent( axis ) );
        }
        out.push_back( p );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTOrientedBox3D< GUCEF::Float64 >& b, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    for ( GUCEF::UInt32 i=0; i<8; ++i )
        out.push_back( b.GetCorner( i ) );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        const GUCEF::Float64 x = r.Uniform( -b.halfExtents.x, b.halfExtents.x );
        const GUCEF::Float64 y = r.Uniform( -b.halfExtents.y, b.halfExtents.y );
        const GUCEF::Float64 z = r.Uniform( -b.halfExtents.z, b.halfExtents.z );
        V3 local( x, y, z );
        if ( 0 != i % 2 )
        {
            const GUCEF::UInt32 axis = (GUCEF::UInt32) r.Index( 3 );
            local.SetComponent( axis, r.Chance( 0.5 ) ? -b.halfExtents.GetComponent( axis ) : b.halfExtents.GetComponent( axis ) );
        }
        out.push_back( b.FromLocal( local ) );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTCapsule< GUCEF::Float64 >& c, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        const GUCEF::Float64 t = r.Uniform( 0.0, 1.0 );
        const D3 d = r.Direction();
        const GUCEF::Float64 f = 0 == i % 2 ? 1.0 : r.Uniform( 0.0, 1.0 );
        out.push_back( c.start + ( c.end - c.start ) * t + d.AsVector() * ( c.radius * f ) );
    }
}

/** A random unit vector perpendicular to the unit vector u */
inline V3
RandomPerpendicular( CRandom& r, const V3& u )
{
    for ( ;; )
    {
        const D3 d = r.Direction();
        const V3 perpendicular = d.AsVector() - u * d.AsVector().Dot( u );
        if ( perpendicular.GetSquaredLength() > 0.01 )
            return perpendicular.Normalized();
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTCylinder< GUCEF::Float64 >& c, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    const V3 axis = c.end - c.start;
    const V3 u = axis.Normalized();
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        GUCEF::Float64 t = r.Uniform( 0.0, 1.0 );
        GUCEF::Float64 rho = r.Uniform( 0.0, 1.0 );
        if ( 1 == i % 3 )
            rho = 1.0;
        else if ( 2 == i % 3 )
            t = r.Chance( 0.5 ) ? 0.0 : 1.0;
        out.push_back( c.start + axis * t + RandomPerpendicular( r, u ) * ( c.radius * rho ) );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTCone< GUCEF::Float64 >& c, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    const V3 u = c.axis.AsVector();
    out.push_back( c.apex );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        GUCEF::Float64 s = r.Uniform( 0.0, 1.0 );
        GUCEF::Float64 rho = r.Uniform( 0.0, 1.0 );
        if ( 1 == i % 3 )
            rho = 1.0;
        else if ( 2 == i % 3 )
            s = 1.0;
        out.push_back( c.apex + u * ( c.height * s ) + RandomPerpendicular( r, u ) * ( c.baseRadius * s * rho ) );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTTriangle3D< GUCEF::Float64 >& t, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    out.push_back( t.a );
    out.push_back( t.b );
    out.push_back( t.c );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        GUCEF::Float64 u = r.Uniform( 0.0, 1.0 );
        GUCEF::Float64 v = r.Uniform( 0.0, 1.0 );
        if ( u + v > 1.0 )
        {
            u = 1.0 - u;
            v = 1.0 - v;
        }
        if ( 0 != i % 3 )
        {
            /* on the edges as well */
            if ( 1 == i % 3 )
                v = 0;
            else
                u = 1.0 - v;
        }
        out.push_back( t.a + ( t.b - t.a ) * u + ( t.c - t.a ) * v );
    }
}

inline void
SampleShape( const GUCEF::GEOMETRY::CTSegment3D< GUCEF::Float64 >& s, CRandom& r, GUCEF::UInt32 count, GUCEF::vector< P3 >& out )
{
    out.push_back( s.start );
    out.push_back( s.end );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
        out.push_back( s.GetPoint( r.Uniform( 0.0, 1.0 ) ) );
}

/*-------------------------------------------------------------------------*/

/** The smallest distance between two sampled point sets */
inline GUCEF::Float64
SampledDistance( const GUCEF::vector< P3 >& a, const GUCEF::vector< P3 >& b )
{
    GUCEF::Float64 best = 1.0e300;
    for ( size_t i=0; i<a.size(); ++i )
    {
        for ( size_t j=0; j<b.size(); ++j )
        {
            const GUCEF::Float64 d = ( b[ j ] - a[ i ] ).GetSquaredLength();
            if ( d < best )
                best = d;
        }
    }
    return sqrt( best );
}

/*-------------------------------------------------------------------------*/

/** |a - b| <= tolerance, false for NaN */
inline bool
NearTo( GUCEF::Float64 a, GUCEF::Float64 b, GUCEF::Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

}; /* namespace QueryReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTQUERYREFERENCE_H ? */
