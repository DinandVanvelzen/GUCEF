/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_SAMPLEWARPING_H
#define GUCEF_MATH_SAMPLEWARPING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Warps of the unit square [0, 1) x [0, 1) onto shapes, section 4.9 of the design. The input is a pair of uniform
 *  numbers (from a random stream or a low discrepancy sequence); the warps keep the area measure, so uniform input
 *  gives uniform output on the shape, with the stated density where it is not uniform. Float32 and Float64 only (the
 *  predefined kinds are exported instantiations compiled without floating-point contraction), results use sqrt, sin
 *  and cos and are therefore not bit identical across platforms. Inputs outside [0, 1] are clamped. Realtime-safe.
 *  Hemispheres have their pole on +Z; BuildOrthonormalBasis and OrientAroundNormal turn them towards any direction.
 */

/** Uniform on the unit disk by the polar map ( r = sqrt( u ), angle = 2 pi v ) */
template < typename T >
CTVector2D< T >
UniformDisk( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform on the unit disk by the concentric map of Shirley and Chiu (1997): keeps neighbourhoods and strata of the square */
template < typename T >
CTVector2D< T >
ConcentricDisk( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform on the unit circle: ( cos( 2 pi u ), sin( 2 pi u ) ) */
template < typename T >
CTVector2D< T >
UniformCircle( const T& u ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform on the unit sphere: z = 1 - 2u, angle = 2 pi v */
template < typename T >
CTVector3D< T >
UniformSphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform on the hemisphere z >= 0: z = u, angle = 2 pi v */
template < typename T >
CTVector3D< T >
UniformHemisphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/** Cosine weighted on the hemisphere z >= 0 (density cos( theta ) / pi): the concentric disk lifted onto the hemisphere (Malley's method) */
template < typename T >
CTVector3D< T >
CosineHemisphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform in the cone around +Z that has the given cosine of its half angle ( 1 = a line, -1 = the sphere ): z = 1 - u ( 1 - cosThetaMax ) */
template < typename T >
CTVector3D< T >
UniformCone( const T& u, const T& v, const T& cosThetaMax ) GUCEF_MATH_REALTIME_CONTEXT;

/** Uniform on a triangle, as barycentric coordinates ( b0, b1, b2 ) that sum to 1: ( 1 - sqrt( u ), sqrt( u ) ( 1 - v ), sqrt( u ) v ) */
template < typename T >
CTVector3D< T >
UniformTriangle( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT;

/**
 *  Two unit vectors that together with the unit vector 'n' form a right handed orthonormal basis ( tangent x bitangent
 *  = n ), without a singularity (Duff et al. 2017). 'n' must have unit length; the result for others is unspecified.
 */
template < typename T >
void
BuildOrthonormalBasis( const CTVector3D< T >& n, CTVector3D< T >& tangent, CTVector3D< T >& bitangent ) GUCEF_MATH_REALTIME_CONTEXT;

/** 'local' ( expressed with +Z as the pole ) turned so that the pole is the unit vector 'n': local.x tangent + local.y bitangent + local.z n */
template < typename T >
CTVector3D< T >
OrientAroundNormal( const CTVector3D< T >& local, const CTVector3D< T >& n ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace SampleWarpingDetail {

template < typename T > GUCEF_MATH_FORCE_INLINE T Unit( const T& v ) { return v > T( 0 ) ? ( v < T( 1 ) ? v : T( 1 ) ) : T( 0 ); }
template < typename T > GUCEF_MATH_FORCE_INLINE T TwoPi( void ) { return T( 6.28318530717958647692528676655900577 ); }
template < typename T > GUCEF_MATH_FORCE_INLINE T SqrtNonNegative( const T& v ) { return v > T( 0 ) ? (T) sqrt( (Float64) v ) : T( 0 ); }

}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
UniformDisk( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T radius = SampleWarpingDetail::SqrtNonNegative( SampleWarpingDetail::Unit( u ) );
    const T angle = SampleWarpingDetail::TwoPi< T >() * SampleWarpingDetail::Unit( v );
    return CTVector2D< T >( radius * (T) cos( (Float64) angle ), radius * (T) sin( (Float64) angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
ConcentricDisk( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T a = T( 2 ) * SampleWarpingDetail::Unit( u ) - T( 1 );
    const T b = T( 2 ) * SampleWarpingDetail::Unit( v ) - T( 1 );
    if ( a == T( 0 ) && b == T( 0 ) )
        return CTVector2D< T >( T( 0 ), T( 0 ) );

    const T quarterPi = T( 0.78539816339744830961566084581987572 );
    T radius;
    T angle;
    if ( a * a > b * b )
    {
        radius = a;
        angle = quarterPi * ( b / a );
    }
    else
    {
        radius = b;
        angle = T( 1.57079632679489661923132169163975144 ) - quarterPi * ( a / b );
    }
    return CTVector2D< T >( radius * (T) cos( (Float64) angle ), radius * (T) sin( (Float64) angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
UniformCircle( const T& u ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T angle = SampleWarpingDetail::TwoPi< T >() * SampleWarpingDetail::Unit( u );
    return CTVector2D< T >( (T) cos( (Float64) angle ), (T) sin( (Float64) angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
UniformSphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T z = T( 1 ) - T( 2 ) * SampleWarpingDetail::Unit( u );
    const T radius = SampleWarpingDetail::SqrtNonNegative( T( 1 ) - z * z );
    const T angle = SampleWarpingDetail::TwoPi< T >() * SampleWarpingDetail::Unit( v );
    return CTVector3D< T >( radius * (T) cos( (Float64) angle ), radius * (T) sin( (Float64) angle ), z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
UniformHemisphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T z = SampleWarpingDetail::Unit( u );
    const T radius = SampleWarpingDetail::SqrtNonNegative( T( 1 ) - z * z );
    const T angle = SampleWarpingDetail::TwoPi< T >() * SampleWarpingDetail::Unit( v );
    return CTVector3D< T >( radius * (T) cos( (Float64) angle ), radius * (T) sin( (Float64) angle ), z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CosineHemisphere( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTVector2D< T > disk = ConcentricDisk( u, v );
    const T z = SampleWarpingDetail::SqrtNonNegative( T( 1 ) - disk.x * disk.x - disk.y * disk.y );
    return CTVector3D< T >( disk.x, disk.y, z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
UniformCone( const T& u, const T& v, const T& cosThetaMax ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T cosMax = cosThetaMax < T( -1 ) ? T( -1 ) : ( cosThetaMax > T( 1 ) ? T( 1 ) : cosThetaMax );
    const T z = T( 1 ) - SampleWarpingDetail::Unit( u ) * ( T( 1 ) - cosMax );
    const T radius = SampleWarpingDetail::SqrtNonNegative( T( 1 ) - z * z );
    const T angle = SampleWarpingDetail::TwoPi< T >() * SampleWarpingDetail::Unit( v );
    return CTVector3D< T >( radius * (T) cos( (Float64) angle ), radius * (T) sin( (Float64) angle ), z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
UniformTriangle( const T& u, const T& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T root = SampleWarpingDetail::SqrtNonNegative( SampleWarpingDetail::Unit( u ) );
    const T unitV = SampleWarpingDetail::Unit( v );
    return CTVector3D< T >( T( 1 ) - root, root * ( T( 1 ) - unitV ), root * unitV );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
BuildOrthonormalBasis( const CTVector3D< T >& n, CTVector3D< T >& tangent, CTVector3D< T >& bitangent ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T sign = n.z >= T( 0 ) ? T( 1 ) : T( -1 );
    const T a = T( -1 ) / ( sign + n.z );
    const T b = n.x * n.y * a;
    tangent = CTVector3D< T >( T( 1 ) + sign * n.x * n.x * a, sign * b, -sign * n.x );
    bitangent = CTVector3D< T >( b, sign + n.y * n.y * a, -n.y );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
OrientAroundNormal( const CTVector3D< T >& local, const CTVector3D< T >& n ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector3D< T > tangent, bitangent;
    BuildOrthonormalBasis( n, tangent, bitangent );
    return CTVector3D< T >( tangent.x * local.x + bitangent.x * local.y + n.x * local.z,
                            tangent.y * local.x + bitangent.y * local.y + n.y * local.z,
                            tangent.z * local.x + bitangent.z * local.y + n.z * local.z );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The predefined kinds of the sampling warps. X( kind, vector2, vector3 ) is invoked per kind; used by the extern
 *  declarations here and by the instantiations in the module source ( gucefMATH_RandomInstantiations.cpp ).
 */
#define GUCEF_MATH_SAMPLEWARPING_PREDEFINED( X )  \
    X( Float32, TFloat32Vector2D, TFloat32Vector3D ) X( Float64, TFloat64Vector2D, TFloat64Vector3D )

#define GUCEF_MATH_SAMPLEWARPING_FUNCTIONS( prefix, kind, vector2, vector3 )                                                                           \
    prefix vector2 UniformDisk< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                        \
    prefix vector2 ConcentricDisk< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                     \
    prefix vector2 UniformCircle< kind >( const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                                   \
    prefix vector3 UniformSphere< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                      \
    prefix vector3 UniformHemisphere< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                  \
    prefix vector3 CosineHemisphere< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                   \
    prefix vector3 UniformCone< kind >( const kind&, const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                           \
    prefix vector3 UniformTriangle< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                                    \
    prefix void BuildOrthonormalBasis< kind >( const vector3&, vector3&, vector3& ) GUCEF_MATH_REALTIME_CONTEXT;                                       \
    prefix vector3 OrientAroundNormal< kind >( const vector3&, const vector3& ) GUCEF_MATH_REALTIME_CONTEXT;

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
#define GUCEF_MATH_SAMPLEWARPING_EXTERN( kind, vector2, vector3 ) GUCEF_MATH_SAMPLEWARPING_FUNCTIONS( GUCEF_MATH_EXTERN_TEMPLATE, kind, vector2, vector3 )
GUCEF_MATH_SAMPLEWARPING_PREDEFINED( GUCEF_MATH_SAMPLEWARPING_EXTERN )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */
